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
// ListEntryController
//------------------------------------------------------------------------
ListEntryController::ListEntryController(
    tiny_observer_pattern::SimpleSubject* subject,
    ARADocumentController& controller,
    ARADocumentController::FnGetSampleRate& fn_get_playback_sample_rate,
    const meta_words::PlaybackRegion* playback_region)
: subject(subject)
, controller(controller)
, fn_get_playback_sample_rate(fn_get_playback_sample_rate)
, playback_region(playback_region)
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
    if (!this->playback_region)
        return nullptr;

    auto& subject         = controller.get_subject(this->playback_region);
    auto sample_rate_func = fn_get_playback_sample_rate;

    if (VSTGUI::UTF8StringView(name) == "MetaWordsClipController")
    {
        auto* subctrl = new MetaWordsClipController(&controller);
        subctrl->set_meta_words_data_func(
            [sample_rate_func, region = this->playback_region]() {
                return region->get_meta_words_data(sample_rate_func());
            });
        subctrl->set_list_clicked_func(
            [&, sample_rate_func, region = this->playback_region](int index) {
                onRequestSelectWord(
                    index, region->get_meta_words_data(sample_rate_func()),
                    controller);
            });
        return subctrl;
    }
    else if (VSTGUI::UTF8StringView(name) == "WaveFormController")
    {
        auto* tmp_controller = new WaveFormController(&subject);
        tmp_controller->set_waveform_data_func(
            [sample_rate_func, region = this->playback_region]() {
                WaveFormController::Data data;
                data.audio_buffer =
                    region->get_audio_buffer(sample_rate_func());

                const auto color =
                    region->get_meta_words_data(sample_rate_func()).color;
                data.color = std::make_tuple(color.r, color.g, color.b);
                return data;
            });
        return tmp_controller;
    }

    return nullptr;
}

//------------------------------------------------------------------------
} // namespace mam