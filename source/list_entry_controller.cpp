//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "list_entry_controller.h"
#include "meta_words_clip_controller.h"

namespace mam {

//------------------------------------------------------------------------
static auto onRequestSelectWord(int index,
                                const mam::MetaWordsData& data,
                                ARADocumentController* controller) -> void
{
    if (!controller)
        return;

    const auto& meta_words_data = data;
    const auto& words           = meta_words_data.words;
    const auto& selected_word   = words.at(index);

    const auto new_position =
        selected_word.begin + meta_words_data.project_offset;
    controller->onRequestLocatorPosChanged(new_position);
}

//------------------------------------------------------------------------
static auto onRequestSelectWord(int index,
                                ARADocumentController* controller,
                                double sample_rate,
                                const meta_words::PlaybackRegion::Id id) -> void
{
    if (!controller)
        return;

    auto opt_region = controller->find_playback_region(id);
    if (!opt_region)
        return;

    onRequestSelectWord(index,
                        opt_region.value()->get_meta_words_data(sample_rate),
                        controller);
}

//------------------------------------------------------------------------
static auto build_meta_words_data(const ARADocumentController* controller,
                                  const meta_words::PlaybackRegion::Id id,
                                  double sample_rate) -> const MetaWordsData
{
    if (!controller)
        return {};

    auto opt_region = controller->find_playback_region(id);
    if (!opt_region)
        return {};

    return opt_region.value()->get_meta_words_data(sample_rate);
}

//------------------------------------------------------------------------
// ListEntryController
//------------------------------------------------------------------------
ListEntryController::ListEntryController(
    ARADocumentController* controller,
    ARADocumentController::FnGetSampleRate& playback_sample_rate_func,
    const meta_words::PlaybackRegion::Id playback_region_id)
: controller(controller)
, playback_sample_rate_func(playback_sample_rate_func)
, playback_region_id(playback_region_id)
{
}

//------------------------------------------------------------------------
ListEntryController::~ListEntryController(){};

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

    if (!controller)
        return nullptr;

    auto& subject =
        controller->get_playback_region_changed_subject(playback_region_id);
    auto sample_rate_func = playback_sample_rate_func;
    auto pbr_id           = playback_region_id;
    auto ctler            = this->controller;

    if (VSTGUI::UTF8StringView(name) == "MetaWordsClipController")
    {
        auto* subctrl = new MetaWordsClipController();
        if (!subctrl)
            return nullptr;

        subctrl->initialize(controller, [ctler, pbr_id, sample_rate_func]() {
            return build_meta_words_data(ctler, pbr_id, sample_rate_func());
        });

        subctrl->set_list_clicked_func(
            [ctler, pbr_id, sample_rate_func](int index) {
                onRequestSelectWord(index, ctler, sample_rate_func(), pbr_id);
            });
        return subctrl;
    }

    return nullptr;
}

//------------------------------------------------------------------------
} // namespace mam