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
        selected_word.word.begin + meta_words_data.project_offset;
    controller->onRequestLocatorPosChanged(new_position);
}

//------------------------------------------------------------------------
static auto onRequestSelectWord(int index,
                                ARADocumentController* controller,
                                const meta_words::PlaybackRegion::Id id) -> void
{
    if (!controller)
        return;

    // Region should be normally always valid (but who knows ;))
    auto opt_region = controller->find_playback_region(id);
    auto region     = opt_region.value_or(nullptr);
    if (!region)
        return;

    // Get the selected word
    const auto words_data = region->get_meta_words_data();
    const auto& words     = words_data.words;
    const auto& word      = words.at(index);

    // Compute its time position, BUT limit it to the region start time
    // so the locator will always jump to the beginning of the region
    // no matter if the word start position is already partly outside
    auto pos = word.word.begin + words_data.project_offset;
    pos      = std::max(pos, region->getStartInPlaybackTime());
    controller->onRequestLocatorPosChanged(pos);
}

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
            onRequestSelectWord(index, ctler, pbr_id);
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

    onRequestSelectWord(data.index, data.meta_word_data, controller);

    controller->get_region_selection_model().select(
        {playback_region_id, static_cast<size_t>(data.index)});
}

//------------------------------------------------------------------------

} // namespace mam
