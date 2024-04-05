//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#include "vstgpt_single_component.h"
#include "ara_document_controller.h"
#include "base/source/fstreamer.h"
#include "meta_words_editor_renderer.h"
#include "meta_words_editor_view.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "vstgpt_cids.h"
#include "vstgpt_listcontroller.h"
#include "waveform_controller.h"

using namespace Steinberg;

namespace mam {
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

    editorRenderer;
    data;
}

//------------------------------------------------------------------------
static auto on_editor_view(meta_words::EditorView& editorView,
                           Vst::ProcessData& data) -> void
{
    editorView;
    data;
}

//------------------------------------------------------------------------
// VstGPTProcessor
//------------------------------------------------------------------------
VstGPTSingleComponent::VstGPTSingleComponent() {}

//------------------------------------------------------------------------
VstGPTSingleComponent::~VstGPTSingleComponent() {}

//------------------------------------------------------------------------
tresult PLUGIN_API VstGPTSingleComponent::initialize(FUnknown* context)
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
    addAudioInput(STR16("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
    addAudioOutput(STR16("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstGPTSingleComponent::terminate()
{
    // Here the Plug-in will be de-instantiated, last possibility to remove some
    // memory!

    //---do not forget to call parent ------
    return SingleComponentEffect::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstGPTSingleComponent::setActive(TBool state)
{
    //--- called when the Plug-in is enable/disable (On/Off) -----

    if (meta_words::PlaybackRenderer* playbackRenderer =
            _araPlugInExtension
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
tresult PLUGIN_API VstGPTSingleComponent::process(Vst::ProcessData& data)
{
    ARA_VALIDATE_API_CONDITION(data.outputs[0].numChannels ==
                               getAudioBusChannelCount(audioOutputs[0]));
    ARA_VALIDATE_API_CONDITION(data.numSamples <=
                               processSetup.maxSamplesPerBlock);

    if (auto playbackRenderer =
            _araPlugInExtension
                .getPlaybackRenderer<meta_words::PlaybackRenderer>())
    {
        on_playback_renderer(*playbackRenderer, data);
    }
    else if (auto editorRenderer =
                 _araPlugInExtension
                     .getEditorRenderer<meta_words::EditorRenderer>())
    {
        on_editor_renderer(*editorRenderer, data);
    }
    else if (auto editorView =
                 _araPlugInExtension.getEditorView<meta_words::EditorView>())
    {
        on_editor_view(*editorView, data);
    }
    else
    {
        // if we're no ARA playback renderer, we're just copying the inputs to
        // the outputs, which is appropriate both when being only an ARA editor
        // renderer, or when being used in non-ARA mode.
        for (int32 c = 0; c < data.outputs[0].numChannels; ++c)
            std::memcpy(data.outputs[0].channelBuffers32[c],
                        data.inputs[0].channelBuffers32[c],
                        sizeof(float) * static_cast<size_t>(data.numSamples));
    }

    // if we are an ARA editor renderer, we now would add out preview signal to
    // the output, but our test implementation does not support editing and thus
    // never generates any preview signal.
    //  if (auto editorRenderer =
    //  _araPlugInExtension.getEditorRenderer<ARATestEditorRenderer*> ())
    //      editorRenderer->addEditorSignal (...);

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API
VstGPTSingleComponent::setupProcessing(Vst::ProcessSetup& newSetup)
{
    //--- called before any processing ----
    return SingleComponentEffect::setupProcessing(newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API
VstGPTSingleComponent::canProcessSampleSize(int32 symbolicSampleSize)
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
tresult PLUGIN_API VstGPTSingleComponent::setState(IBStream* state)
{
    // called when we load a preset, the model has to be reloaded
    IBStreamer streamer(state, kLittleEndian);

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstGPTSingleComponent::getState(IBStream* state)
{
    // here we need to save the model
    IBStreamer streamer(state, kLittleEndian);

    return kResultOk;
}

//------------------------------------------------------------------------
VSTGUI::IController* VstGPTSingleComponent::createSubController(
    VSTGUI::UTF8StringPtr name,
    const VSTGUI::IUIDescription* /*description*/,
    VSTGUI::VST3Editor* /*editor*/)
{
    auto* document_controller =
        _araPlugInExtension.getDocumentController<ARADocumentController>();
    // auto* editorView = _araPlugInExtension.getEditorView();

    if (!document_controller)
        return nullptr;

    if (VSTGUI::UTF8StringView(name) == "MetaWordsListController")
        return new VstGPTListController(
            *document_controller,
            std::move([this]() { return this->processSetup.sampleRate; }));

    return nullptr;
}

//------------------------------------------------------------------------
void VstGPTSingleComponent::didOpen(VSTGUI::VST3Editor* editor) {}

//------------------------------------------------------------------------
void VstGPTSingleComponent::willClose(VSTGUI::VST3Editor* editor)
{
    if (_araPlugInExtension.getEditorView())
        _araPlugInExtension.getEditorView()->setEditorOpen(false);
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API VstGPTSingleComponent::createView(FIDString name)
{
    // Here the Host wants to open your editor (if you have one)
    if (FIDStringsEqual(name, Vst::ViewType::kEditor))
    {
        // create your editor here and return a IPlugView ptr of it
        auto* view =
            new VSTGUI::VST3Editor(this, "view", "vstgpt_editor.uidesc");

        // Must be set to 'true' to get notified by a host selection change
        if (_araPlugInExtension.getEditorView())
            _araPlugInExtension.getEditorView()->setEditorOpen(true);

        return view;
    }
    return nullptr;
}

//-----------------------------------------------------------------------------
const ARA::ARAPlugInExtensionInstance* PLUGIN_API
VstGPTSingleComponent::bindToDocumentController(
    ARA::ARADocumentControllerRef /*documentControllerRef*/)
{
    ARA_VALIDATE_API_STATE(
        false && "call is deprecated in ARA 2, host must not call this");
    return nullptr;
}

//-----------------------------------------------------------------------------
const ARA::ARAPlugInExtensionInstance* PLUGIN_API
VstGPTSingleComponent::bindToDocumentControllerWithRoles(
    ARA::ARADocumentControllerRef documentControllerRef,
    ARA::ARAPlugInInstanceRoleFlags knownRoles,
    ARA::ARAPlugInInstanceRoleFlags assignedRoles)
{
    return _araPlugInExtension.bindToARA(documentControllerRef, knownRoles,
                                         assignedRoles);
}

//-----------------------------------------------------------------------------
const ARA::ARAFactory* PLUGIN_API VstGPTSingleComponent::getFactory()
{
    return ARADocumentController::getARAFactory();
}

//------------------------------------------------------------------------
} // namespace mam
