//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "list_entry_controller.h"
#include "meta_words_clip_controller.h"
#include "waveform_controller.h"

namespace mam {

//------------------------------------------------------------------------
static auto onRequestSelectWord(int index,
                                const mam::MetaWordsData& data,
                                ARADocumentController& document_controller)
    -> void
{
    const auto& meta_words_data = data;
    const auto& words           = meta_words_data.words;
    const auto& selected_word   = words.at(index);

    const auto new_position =
        selected_word.begin + meta_words_data.project_offset;
    document_controller.onRequestLocatorPosChanged(new_position);
}

//------------------------------------------------------------------------
static auto onRequestSelectWord(int index,
                                ARADocumentController& controller,
                                double sample_rate,
                                meta_words::PlaybackRegion::Id id) -> void
{
    auto opt_region = controller.find_playback_region(id);
    if (!opt_region)
        return;

    onRequestSelectWord(index,
                        opt_region.value()->get_meta_words_data(sample_rate),
                        controller);
}

//------------------------------------------------------------------------
// ListEntryController
//------------------------------------------------------------------------
ListEntryController::ListEntryController(
    tiny_observer_pattern::SimpleSubject* subject,
    ARADocumentController& controller,
    ARADocumentController::FnGetSampleRate& fn_get_playback_sample_rate,
    const meta_words::PlaybackRegion::Id playback_region_id)
: subject(subject)
, controller(controller)
, fn_get_playback_sample_rate(fn_get_playback_sample_rate)
, playback_region_id(playback_region_id)
{
    if (subject)
        observer_id = subject->add_listener([this](const auto&) {});
}

//------------------------------------------------------------------------
ListEntryController::~ListEntryController()
{
    if (subject)
        subject->remove_listener(observer_id);
};

//------------------------------------------------------------------------
VSTGUI::CView*
ListEntryController::verifyView(VSTGUI::CView* view,
                                const VSTGUI::UIAttributes& attributes,
                                const VSTGUI::IUIDescription* description)
{
    return view;
};

//------------------------------------------------------------------------
VSTGUI::IController* ListEntryController::createSubController(
    VSTGUI::UTF8StringPtr name, const VSTGUI::IUIDescription* description)
{
    if (playback_region_id == meta_words::PlaybackRegion::INVALID_ID)
        return nullptr;

    auto& subject = controller.get_playback_region_subject(playback_region_id);
    auto sample_rate_func = fn_get_playback_sample_rate;
    auto pbr_id           = playback_region_id;

    if (VSTGUI::UTF8StringView(name) == "MetaWordsClipController")
    {
        auto* subctrl = new MetaWordsClipController(&controller);
        subctrl->set_meta_words_data_func(
            [this, pbr_id, sample_rate_func]() -> const MetaWordsData {
                auto opt_region = controller.find_playback_region(pbr_id);
                if (!opt_region)
                    return {};

                return opt_region.value()->get_meta_words_data(
                    sample_rate_func());
            });
        subctrl->set_list_clicked_func(
            [this, pbr_id, sample_rate_func](int index) {
                onRequestSelectWord(index, controller, sample_rate_func(), pbr_id);
            });
        return subctrl;
    }
    else if (VSTGUI::UTF8StringView(name) == "WaveFormController")
    {
        auto* tmp_controller = new WaveFormController(&subject);
        tmp_controller->set_waveform_data_func(
            [pbr_id, this, sample_rate_func]() -> WaveFormController::Data {
                auto opt_region = controller.find_playback_region(pbr_id);
                if (!opt_region)
                    return {};

                WaveFormController::Data data;
                data.audio_buffer =
                    opt_region.value()->get_audio_buffer(sample_rate_func());

                const auto color = opt_region.value()
                                       ->get_meta_words_data(sample_rate_func())
                                       .color;
                data.color = std::make_tuple(color.r, color.g, color.b);
                return data;
            });
        return tmp_controller;
    }

    return nullptr;
}

//------------------------------------------------------------------------
} // namespace mam