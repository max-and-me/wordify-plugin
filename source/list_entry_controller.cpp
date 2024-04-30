//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "list_entry_controller.h"
#include "meta_words_clip_controller.h"

namespace mam {

//------------------------------------------------------------------------
static auto build_meta_words_data(const ARADocumentController* controller,
                                  const meta_words::PlaybackRegion::Id id)
    -> const MetaWordsData
{
    if (!controller)
        return {};

    auto opt_region = controller->find_playback_region(id);
    if (!opt_region)
        return {};

    return opt_region.value()->get_meta_words_data();
}

//------------------------------------------------------------------------
// ListEntryController
//------------------------------------------------------------------------
ListEntryController::ListEntryController(
    ARADocumentController* controller,
    const meta_words::PlaybackRegion::Id id,
    const VSTGUI::IUIDescription* description)
: controller(controller)
, playback_region_id(id)
, description(description)
{
    if (controller)
        word_selected_observer_id = controller->register_word_selected_observer(
            [this](const auto& data) { this->checkSelectWord(data); });
}

//------------------------------------------------------------------------
ListEntryController::~ListEntryController()
{
    if (controller)
        controller->unregister_word_selected_observer(
            word_selected_observer_id);
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

    if (!controller)
        return nullptr;

    auto pbr_id = playback_region_id;
    auto ctler  = this->controller;

    if (VSTGUI::UTF8StringView(name) == "MetaWordsClipController")
    {
        auto* subctrl = new MetaWordsClipController(description);
        if (!subctrl)
            return nullptr;

        auto& subject = controller->get_playback_region_changed_subject(pbr_id);
        subctrl->initialize(
            &subject, [=]() { return build_meta_words_data(ctler, pbr_id); });

        subctrl->list_value_changed_func = [=](int index) {
            controller->get_region_selection_model().select(
                {pbr_id, static_cast<size_t>(index)});
            controller->onRequestSelectWord(index, pbr_id);
        };
        return subctrl;
    }

    return nullptr;
}

//------------------------------------------------------------------------
void ListEntryController::checkSelectWord(const WordSelectData& data)
{
    if (data.region_id != playback_region_id)
        return;

    controller->onRequestSelectWord(data.index, data.region_id);

    controller->get_region_selection_model().select(
        {playback_region_id, static_cast<size_t>(data.index)});
}

//------------------------------------------------------------------------

} // namespace mam
