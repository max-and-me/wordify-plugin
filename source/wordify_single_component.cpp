// Copyright (c) 2023-present, WordifyOrg.

#include "wordify_single_component.h"
#include "controllers/list_controller.h"
#include "controllers/preferences_controller.h"
#include "controllers/search_controller.h"
#include "controllers/spinner_controller.h"
#include "controllers/waveform_controller.h"
#include "meta_words_editor_renderer.h"
#include "meta_words_editor_view.h"
#include "meta_words_playback_renderer.h"
#include "parameter_ids.h"
#include "preferences_serde.h"
#include "task_manager.h"
#include "warn_cpp/suppress_warnings.h"
#include "wordify_cids.h"
#include "wordify_defines.h"
#include <optional>
BEGIN_SUPPRESS_WARNINGS
#include "ara_document_controller.h"
#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "vstgui/uidescription/uidescription.h"
END_SUPPRESS_WARNINGS

using namespace Steinberg;

namespace Presonus {
DEF_CLASS_IID(IPlugInViewEmbedding)
}

namespace mam {

//------------------------------------------------------------------------
static const auto DEFAULT_COLOR_SCHEME_UIDESC =
    "editor_res_signal_lite_scheme.uidesc";

//------------------------------------------------------------------------
//  // helper to improve readability
static auto getAudioBusChannelCount(const IPtr<Vst::Bus>& bus) -> int32
{
    return Vst::SpeakerArr::getChannelCount(
        FCast<Vst::AudioBus>(bus.get())->getArrangement());
}

//------------------------------------------------------------------------
static auto on_playback_renderer(meta_words::PlaybackRenderer& playbackRenderer,
                                 Vst::ProcessData& data) -> void
{
    // if we're an ARA playback renderer, calculate ARA playback output
    playbackRenderer.renderPlaybackRegions(
        data.outputs[0].channelBuffers32,
        data.processContext->projectTimeSamples, data.numSamples,
        (data.processContext->state & Vst::ProcessContext::kPlaying) != 0);
}

//------------------------------------------------------------------------
static auto on_editor_renderer(meta_words::EditorRenderer& editorRenderer,
                               Vst::ProcessData& data) -> void
{
    if (data.processContext)
    {
        const auto time = data.processContext->projectTimeSamples /
                          data.processContext->sampleRate;
        editorRenderer.update_project_time(time);
    }
}

//------------------------------------------------------------------------
static auto on_editor_view(meta_words::EditorView& /*editorView*/,
                           Vst::ProcessData& /*data*/) -> void
{
}

//------------------------------------------------------------------------
using Range = std::pair<size_t, size_t>;
static auto to_sample_range(const meta_words::MetaWord& word,
                            double sample_rate) -> const Range
{
    const auto begin_sample = static_cast<size_t>(word.begin * sample_rate);
    const auto duration_sample =
        static_cast<size_t>(word.duration * sample_rate);
    return {begin_sample, duration_sample};
}

//------------------------------------------------------------------------
static auto build_waveform_data(ARADocumentController* controller)
    -> WaveFormController::Data
{
    if (!controller)
        return {};

    auto selection  = controller->get_region_selection_model().get_data();
    auto opt_region = controller->find_playback_region(selection.region_id);
    auto region     = opt_region.value_or(nullptr);
    if (!region)
        return {};

    const auto& audioSrc = region->getAudioModification()
                               ->getAudioSource<mam::meta_words::AudioSource>();
    if (!audioSrc)
        return {};

    const auto sample_rate = audioSrc->getSampleRate();

    const auto span_data   = region->get_audio_buffer();
    const auto region_data = region->get_region_data().words;
    size_t a               = 0;
    size_t b               = 0;
    if ((selection.word_index < region_data.size()))
    {
        const auto word_sample_range = to_sample_range(
            region_data.at(selection.word_index).word, sample_rate);

        // Wow, wild calculations here. But it seems to work for now :)
        const auto span_begin_samples = span_data.offset_samples;
        const auto span_end_samples =
            span_data.offset_samples + span_data.audio_buffer_span.size();
        const auto word_begin_samples = word_sample_range.first;
        const auto word_end_samples =
            word_sample_range.first + word_sample_range.second;

        a = std::clamp(word_begin_samples, span_begin_samples,
                       span_end_samples);
        b = std::clamp(word_end_samples, span_begin_samples, span_end_samples);

        a -= span_data.offset_samples;
        b -= span_data.offset_samples;
        b -= a;
    }

    return {region->get_effective_color(), span_data.audio_buffer_span, {a, b}};
}

//------------------------------------------------------------------------
using ColorSchemeDesc = StringType;
static auto get_color_scheme(bool is_dark) -> const ColorSchemeDesc
{
    return is_dark ? "editor_res_signal_dark_scheme.uidesc"
                   : "editor_res_signal_lite_scheme.uidesc";
}

//------------------------------------------------------------------------
using OptColorSchemeDesc = std::optional<ColorSchemeDesc>;
static auto get_color_scheme(WordifySingleComponent& controller,
                             mam::ParamIds id) -> OptColorSchemeDesc
{
    const auto* color_scheme_param = controller.getParameterObject(id);
    if (!color_scheme_param)
        return std::nullopt;

    const auto is_dark      = color_scheme_param->getNormalized() > 0.;
    const auto color_scheme = get_color_scheme(is_dark);
    return {color_scheme};
}

//------------------------------------------------------------------------
static auto set_dark_scheme_on_editors(WordifySingleComponent::Editors& editors,
                                       bool is_dark)
{
    for (auto& editor : editors)
    {
        auto* view        = dynamic_cast<VSTGUI::VST3Editor*>(editor);
        const auto uidesc = view->getUIDescription();
        if (uidesc)
        {
            const auto color_scheme = get_color_scheme(is_dark);
            const auto dark_scheme_resources =
                VSTGUI::makeOwned<VSTGUI::UIDescription>(color_scheme.c_str());
            if (!dark_scheme_resources->parse())
                return;

            uidesc->setSharedResources(dark_scheme_resources);
            view->exchangeView("view");
        }
    }
}

//------------------------------------------------------------------------
static auto update_task_count_param(size_t count,
                                    WordifySingleComponent* component)
{
    if (auto p =
            component->getParameterObject(ParamIds::kParamIdAnalyzeTaskCount))
    {
        const auto norm = p->toNormalized(static_cast<Vst::ParamValue>(count));
        p->setNormalized(norm);
    }
}

//------------------------------------------------------------------------
// WordifySingleComponent
//------------------------------------------------------------------------
WordifySingleComponent::WordifySingleComponent() {}

//------------------------------------------------------------------------
WordifySingleComponent::~WordifySingleComponent() {}

//------------------------------------------------------------------------
tresult PLUGIN_API WordifySingleComponent::initialize(FUnknown* context)
{
    // Here the Plug-in will be instantiated

    //---always initialize the parent-------
    tresult result = SingleComponentEffect::initialize(context);
    // if everything Ok, continue
    if (result != kResultOk)
    {
        return result;
    }

    //--- create Audio IO ------
    addAudioInput(STR16("Mono In"), Vst::SpeakerArr::kMono);
    addAudioOutput(STR16("Mono Out"), Vst::SpeakerArr::kMono);

    restore_parameters();

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API WordifySingleComponent::terminate()
{
    // Here the Plug-in will be de-instantiated, last possibility to remove
    // some memory!

    store_parameters();
    task_managing::get_task_count_callback()->remove(task_count_handle);

    for (auto i = 0; i < parameters.getParameterCount(); i++)
    {
        if (auto* p = parameters.getParameterByIndex(i))
            p->removeDependent(this);
    }

    //---do not forget to call parent ------
    return SingleComponentEffect::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API WordifySingleComponent::setActive(TBool state)
{
    //--- called when the Plug-in is enable/disable (On/Off) -----

    if (meta_words::PlaybackRenderer* playbackRenderer =
            araPlugInExtension
                .getPlaybackRenderer<meta_words::PlaybackRenderer>())
    {
        if (state)
            playbackRenderer->enableRendering(
                processSetup.sampleRate,
                getAudioBusChannelCount(audioOutputs[0]),
                processSetup.maxSamplesPerBlock);
        else
            playbackRenderer->disableRendering();
    }

    return SingleComponentEffect::setActive(state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API WordifySingleComponent::process(Vst::ProcessData& data)
{
    if (data.numOutputs == 0)
        return kResultOk;

    ARA_VALIDATE_API_CONDITION(data.outputs[0].numChannels ==
                               getAudioBusChannelCount(audioOutputs[0]));
    ARA_VALIDATE_API_CONDITION(data.numSamples <=
                               processSetup.maxSamplesPerBlock);

    if (auto playbackRenderer =
            araPlugInExtension
                .getPlaybackRenderer<meta_words::PlaybackRenderer>())
    {
        on_playback_renderer(*playbackRenderer, data);
    }
    else if (auto editorRenderer =
                 araPlugInExtension
                     .getEditorRenderer<meta_words::EditorRenderer>())
    {
        on_editor_renderer(*editorRenderer, data);
    }
    else if (auto editorView =
                 araPlugInExtension.getEditorView<meta_words::EditorView>())
    {
        on_editor_view(*editorView, data);
    }
    else
    {
        // No support for non ARA
    }

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API
WordifySingleComponent::setupProcessing(Vst::ProcessSetup& newSetup)
{
    return SingleComponentEffect::setupProcessing(newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API
WordifySingleComponent::canProcessSampleSize(int32 symbolicSampleSize)
{
    // by default kSample32 is supported
    if (symbolicSampleSize == Vst::kSample32)
        return kResultTrue;

    // disable the following comment if your processing support kSample64
    /* if (symbolicSampleSize == Vst::kSample64)
        return kResultTrue; */

    return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API WordifySingleComponent::setState(IBStream* /*state*/)
{
    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API WordifySingleComponent::getState(IBStream* /*state*/)
{
    return kResultOk;
}

//------------------------------------------------------------------------
VSTGUI::IController* WordifySingleComponent::createSubController(
    VSTGUI::UTF8StringPtr name,
    const VSTGUI::IUIDescription* description,
    VSTGUI::VST3Editor* /*editor*/)
{
    auto* document_controller =
        araPlugInExtension.getDocumentController<ARADocumentController>();
    // auto* editorView = araPlugInExtension.getEditorView();

    if (!document_controller)
        return nullptr;

    if (VSTGUI::UTF8StringView(name) == "MetaWordsListController")
    {
        return new ListController(document_controller, description);
    }
    else if (VSTGUI::UTF8StringView(name) == "WaveFormController")
    {
        auto* subctrl = new WaveFormController(document_controller);
        if (!subctrl)
            return nullptr;

        subctrl->initialize(
            document_controller->get_region_selection_subject(),
            [=]() { return build_waveform_data(document_controller); });

        return subctrl;
    }
    else if (VSTGUI::UTF8StringView(name) == "SearchController")
    {
        return new SearchController(
            document_controller,
            getParameterObject(ParamIds::kParamIdSmartSearchMode));
    }
    else if (VSTGUI::UTF8StringView(name) == "SpinnerController")
    {
        return new SpinnerController(
            document_controller,
            getParameterObject(ParamIds::kParamIdAnalyzeTaskCount));
    }
    else if (VSTGUI::UTF8StringView(name) == "PreferencesController")
    {
        return new PreferencesController(document_controller);
    }

    return nullptr;
}

//------------------------------------------------------------------------
void WordifySingleComponent::didOpen(VSTGUI::VST3Editor* /*editor*/) {}

//------------------------------------------------------------------------
void WordifySingleComponent::willClose(VSTGUI::VST3Editor* /*editor*/)
{
    if (araPlugInExtension.getEditorView())
        araPlugInExtension.getEditorView()->setEditorOpen(false);
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API WordifySingleComponent::createView(FIDString name)
{
    // Here the Host wants to open your editor (if you have one)
    if (FIDStringsEqual(name, Vst::ViewType::kEditor))
    {
        // create your editor here and return a IPlugView ptr of it
        auto* view =
            new VSTGUI::VST3Editor(this, "view", "wordify_editor.uidesc");

        const auto uidesc = view->getUIDescription();
        if (uidesc)
        {
            const auto opt_color_scheme =
                get_color_scheme(*this, kParamIdColorScheme);

            const auto color_scheme =
                opt_color_scheme.value_or(DEFAULT_COLOR_SCHEME_UIDESC);

            auto scheme_res =
                VSTGUI::makeOwned<VSTGUI::UIDescription>(color_scheme.c_str());

            if (!scheme_res->parse())
                return nullptr;

            uidesc->setSharedResources(scheme_res);
        }

        // Must be set to 'true' to get notified by a host selection change
        if (araPlugInExtension.getEditorView())
            araPlugInExtension.getEditorView()->setEditorOpen(true);

        return view;
    }
    return nullptr;
}

//-----------------------------------------------------------------------------
const ARA::ARAPlugInExtensionInstance* PLUGIN_API
WordifySingleComponent::bindToDocumentController(
    ARA::ARADocumentControllerRef /*documentControllerRef*/)
{
    ARA_VALIDATE_API_STATE(
        false && "call is deprecated in ARA 2, host must not call this");
    return nullptr;
}

//-----------------------------------------------------------------------------
const ARA::ARAPlugInExtensionInstance* PLUGIN_API
WordifySingleComponent::bindToDocumentControllerWithRoles(
    ARA::ARADocumentControllerRef documentControllerRef,
    ARA::ARAPlugInInstanceRoleFlags knownRoles,
    ARA::ARAPlugInInstanceRoleFlags assignedRoles)
{
    return araPlugInExtension.bindToARA(documentControllerRef, knownRoles,
                                        assignedRoles);
}

//-----------------------------------------------------------------------------
const ARA::ARAFactory* PLUGIN_API WordifySingleComponent::getFactory()
{
    return ARADocumentController::getARAFactory();
}

//------------------------------------------------------------------------
TBool PLUGIN_API WordifySingleComponent::isViewEmbeddingSupported()
{
    return TBool(true);
}

//------------------------------------------------------------------------
tresult PLUGIN_API WordifySingleComponent::setViewIsEmbedded(
    IPlugView* /*view*/, TBool /*embedded*/)
{
    return kResultOk;
}

//------------------------------------------------------------------------
void PLUGIN_API WordifySingleComponent::editorAttached(Vst::EditorView* editor)
{
    editors.push_back(editor);
}

//------------------------------------------------------------------------
void PLUGIN_API WordifySingleComponent::editorRemoved(Vst::EditorView* editor)
{

    editors.erase(std::find(editors.begin(), editors.end(), editor));
}

//------------------------------------------------------------------------
auto WordifySingleComponent::restore_parameters() -> void
{
    // Restore preferences from disc
    meta_words::serde::Preferences prefs;
    meta_words::serde::read_from(COMPANY_NAME_STR, PLUGIN_NAME_STR, prefs);

    if (auto* p = new Vst::StringListParameter(STR("ColorScheme"),
                                               ParamIds::kParamIdColorScheme))
    {
        p->appendString(STR("Lite"));
        p->appendString(STR("Dark"));
        p->setNormalized(
            prefs.color_scheme == meta_words::serde::ColorScheme::Dark ? 1.
                                                                       : 0.);
        dark_scheme = p->getNormalized() > 0.;
        parameters.addParameter(p);
        p->addDependent(this);
    }
    if (auto* p = new Vst::Parameter(STR("SmartSearchMode"),
                                     ParamIds::kParamIdSmartSearchMode))
    {
        const auto val =
            prefs.smart_search == meta_words::serde::SmartSearch::On ? 1. : 0.;
        p->setNormalized(val);
        parameters.addParameter(p);
        p->addDependent(this);
    }
    if (auto* p = new Vst::Parameter(STR("SmartSearchNext"),
                                     ParamIds::kParamIdSmartSearchNext))
    {
        parameters.addParameter(p);
        p->addDependent(this);
    }
    if (auto* p = new Vst::Parameter(STR("SmartSearchPrev"),
                                     ParamIds::kParamIdSmartSearchPrev))
    {
        parameters.addParameter(p);
        p->addDependent(this);
    }
    if (auto* p = new Vst::RangeParameter(STR("TaskCount"),
                                          ParamIds::kParamIdAnalyzeTaskCount,
                                          STR(""), 0., 100., 0., 100))
    {
        parameters.addParameter(p);
        p->addDependent(this);

        const auto num_tasks =
            static_cast<Vst::ParamValue>(task_managing::count_tasks());
        const auto norm = p->toNormalized(num_tasks);
        p->setNormalized(norm);

        task_count_handle = task_managing::get_task_count_callback()->append(
            [&](size_t count) { update_task_count_param(count, this); });
    }
}

//------------------------------------------------------------------------
auto WordifySingleComponent::store_parameters() -> void
{
    // Store preferences to disc
    meta_words::serde::Preferences prefs{};

    if (auto* color_scheme_param =
            getParameterObject(ParamIds::kParamIdColorScheme))
    {
        prefs.color_scheme = color_scheme_param->getNormalized() > 0.
                                 ? meta_words::serde::ColorScheme::Dark
                                 : meta_words::serde::ColorScheme::Light;
    }

    if (auto* smart_search_param =
            getParameterObject(ParamIds::kParamIdSmartSearchMode))
    {
        prefs.smart_search = smart_search_param->getNormalized() > 0.
                                 ? meta_words::serde::SmartSearch::On
                                 : meta_words::serde::SmartSearch::Off;
    }

    meta_words::serde::write_to(prefs, COMPANY_NAME_STR, PLUGIN_NAME_STR);
}

//------------------------------------------------------------------------
void PLUGIN_API WordifySingleComponent::update(FUnknown* changedUnknown,
                                               int32 /*tag*/)
{
    if (auto* param = FCast<Vst::Parameter>(changedUnknown))
    {
        switch (param->getInfo().id)
        {
            case ParamIds::kParamIdColorScheme: {
                if (dark_scheme == param->getNormalized() > 0.)
                    return;

                dark_scheme = param->getNormalized() > 0.;
                set_dark_scheme_on_editors(editors, dark_scheme);
                break;
            }
            case ParamIds::kParamIdSmartSearchMode:
                // Nothing here
                break;
            case ParamIds::kParamIdSmartSearchNext:
                if (param->getNormalized() > 0.)
                    SearchEngine::instance().next_occurence();
                break;
            case ParamIds::kParamIdSmartSearchPrev:
                if (param->getNormalized() > 0.)
                    SearchEngine::instance().prev_occurence();
                break;
        }
    }
}

//------------------------------------------------------------------------
} // namespace mam
