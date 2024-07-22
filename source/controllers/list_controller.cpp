//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "list_controller.h"
#include "region_controller.h"
#include "search_engine.h"
#include "views/word_button.h"
#include "warn_cpp/suppress_warnings.h"
BEGIN_SUPPRESS_WARNINGS
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/controls/cbuttons.h"
#include "vstgui/lib/crowcolumnview.h"
#include "vstgui/lib/cscrollview.h"
#include "vstgui/uidescription/iuidescription.h"
END_SUPPRESS_WARNINGS

using namespace VSTGUI;

namespace mam {

//------------------------------------------------------------------------
constexpr size_t PLAYBACK_REGION_ID_ATTR = 'prid';
constexpr auto REGION_VIEW_TEMPLATE      = "RegionTemplate";

//------------------------------------------------------------------------
static auto find_region_view_by_id(const CRowColumnView& rowColView,
                                   Id pbr_id) -> CView*
{
    CView* viewToFind = nullptr;
    rowColView.forEachChild([&, pbr_id](CView* view) {
        if (viewToFind)
            return;

        Id id = 0;
        if (view->getAttribute(PLAYBACK_REGION_ID_ATTR, id))
        {
            if (pbr_id == size_t(id))
                viewToFind = view;
        }
    });

    return viewToFind;
}

//------------------------------------------------------------------------
static auto get_button_state(const SearchEngine::SearchResult& search_results,
                             int32_t control_tag) -> WordButton::State
{
    using State    = WordButton::State;
    auto new_state = State::kNone;

    const auto& indices    = search_results.indices;
    const auto opt_focused = search_results.focused_word;

    auto iter = std::find(indices.begin(), indices.end(), control_tag);
    if (iter != indices.end())
    {
        const auto i = std::distance(indices.begin(), iter);
        new_state    = State::kSearched;
        if (opt_focused)
        {
            new_state = (static_cast<Id>(i) == opt_focused.value())
                            ? State::kFocused
                            : State::kSearched;
        }
    }

    return new_state;
}

//------------------------------------------------------------------------
static auto scroll_to_view(CRowColumnView* rowColView, const CView* view)
{
    if (!rowColView)
        return;

    auto parent = rowColView->getParentView();
    if (!parent)
        return;

    parent = parent->getParentView();
    if (!parent)
        return;

    // Copied from CScrollView::notify (CBaseObject* sender, IdStringPtr
    // message)
    if (auto scroll_view = dynamic_cast<CScrollView*>(parent))
    {
        CRect r = view->getViewSize();
        CPoint p;
        view->localToFrame(p);
        scroll_view->frameToLocal(p);
        r.offset(p.x, p.y);
        scroll_view->makeRectVisible(r);
    }
}

//------------------------------------------------------------------------
static auto find_region_data(const ARADocumentController* controller,
                             const Id id) -> const RegionData
{
    if (!controller)
        return {};

    auto opt_region = controller->find_playback_region(id);
    if (!opt_region)
        return {};

    return opt_region.value()->get_region_data();
}

//------------------------------------------------------------------------
static auto on_request_select_word(const Id region_id,
                                   const Index word_index,
                                   ARADocumentController* controller) -> void
{
    if (!controller)
        return;

    // Region should be normally always valid (but who knows ;))
    auto opt_region = controller->find_playback_region(region_id);
    auto region     = opt_region.value_or(nullptr);
    if (!region)
        return;

    // Get the selected word
    const auto words_data = region->get_region_data();
    const auto& words     = words_data.words;
    const auto& word      = words.at(word_index);

    // Compute its time position, BUT limit it to the region start time
    // so the locator will always jump to the beginning of the region
    // no matter if the word start position is already partly outside
    auto pos = word.word.begin + words_data.project_offset;
    pos      = std::max(pos, region->getStartInPlaybackTime());
    controller->onRequestLocatorPosChanged(pos);
}

//------------------------------------------------------------------------
// ListController
//------------------------------------------------------------------------
ListController::ListController(ARADocumentController* document_controller,
                               const IUIDescription* uidesc)
: document_controller(document_controller)
, uidesc(uidesc)
{
    if (document_controller)
    {
        lifetime_observer_handle =
            document_controller->get_playback_region_lifetimes_subject()
                ->append([&](const auto& data) {
                    on_add_remove_playback_region(data);
                });

        order_observer_handle =
            document_controller->get_playback_region_order_subject()->append(
                [&](const auto&) { on_playback_regions_reordered(); });

        focus_word_observer_handle =
            SearchEngine::instance().get_callback().append(
                [this](const auto& data) {
                    for (const auto& result : data)
                        on_focus_word(result);
                });

        region_selected_by_host_handle =
            document_controller->get_region_selected_by_host_subject()->append(
                [this](const auto& region_id_data) {
                    on_region_selected_by_host(region_id_data.id);
                });
    }
}

//------------------------------------------------------------------------
ListController::~ListController()
{
    if (rowColView)
    {
        rowColView->unregisterViewListener(this);
        rowColView = nullptr;
    }

    if (document_controller)
    {
        document_controller->get_playback_region_order_subject()->remove(
            order_observer_handle);

        SearchEngine::instance().get_callback().remove(
            focus_word_observer_handle);

        document_controller->get_playback_region_lifetimes_subject()->remove(
            lifetime_observer_handle);

        document_controller->get_region_selected_by_host_subject()->remove(
            region_selected_by_host_handle);
    }
}

//------------------------------------------------------------------------
CView* ListController::verifyView(CView* view,
                                  const UIAttributes& /*attributes*/,
                                  const IUIDescription* /*description*/)
{
    if (!rowColView)
    {
        rowColView = dynamic_cast<CRowColumnView*>(view);
        if (rowColView)
        {
            rowColView->registerViewListener(this);
            if (document_controller)
            {
                document_controller->for_each_region_id([&](const Id id) {
                    auto* newView = create_list_item_view(id);
                    if (newView)
                    {
                        rowColView->addView(newView);
                        rowColView->sizeToFit();
                        newView->setAttribute(PLAYBACK_REGION_ID_ATTR, id);
                    }
                });
            }
        }
    }

    return view;
}

//------------------------------------------------------------------------
auto ListController::create_list_item_view(const Id id) -> CView*
{
    if (!uidesc)
        return nullptr;

    playback_region_id = id;

    auto* newView = uidesc->createView(REGION_VIEW_TEMPLATE, this);
    newView->setAttribute(PLAYBACK_REGION_ID_ATTR, id);

    playback_region_id.reset();

    return newView;
}

//------------------------------------------------------------------------
void ListController::on_playback_regions_reordered()
{
    if (!rowColView)
        return;

    if (!document_controller)
        return;

    auto func = [&](size_t index, Id id) {
        auto* viewToMove = find_region_view_by_id(*rowColView, id);
        rowColView->changeViewZOrder(viewToMove, static_cast<uint32_t>(index));
    };

    document_controller->for_each_region_id_enumerated(func);

    rowColView->invalid();
}

//------------------------------------------------------------------------
void ListController::on_region_selected_by_host(Id region_id)
{
    if (!rowColView)
        return;

    CView* toFind = find_region_view_by_id(*rowColView, region_id);
    if (toFind == nullptr)
        return;

    // Somehow only scrolling to the first child seems to work, no idea why!?
    auto* vc = toFind->asViewContainer();
    toFind   = vc->getView(0);

    scroll_to_view(rowColView, toFind);
}

//------------------------------------------------------------------------
void ListController::on_add_remove_playback_region(
    const RegionLifetimeEventData& data)
{
    if (!uidesc)
        return;

    if (!rowColView)
        return;

    switch (data.event)
    {
        case RegionLifetimeEventData::Event::HasBeenAdded: {
            auto* viewToAdd = create_list_item_view(data.id);
            if (viewToAdd)
            {
                rowColView->addView(viewToAdd);
                rowColView->sizeToFit();
                on_playback_regions_reordered();
            }

            break;
        }
        case RegionLifetimeEventData::Event::WillBeRemoved: {
            auto* viewToRemove = find_region_view_by_id(*rowColView, data.id);
            if (viewToRemove)
            {
                rowColView->removeView(viewToRemove);
                rowColView->sizeToFit();
                rowColView->invalid();
            }
            break;
        }
    }
}

//------------------------------------------------------------------------
IController*
ListController::createSubController(UTF8StringPtr name,
                                    const IUIDescription* description)
{
    if (!playback_region_id.has_value())
        return nullptr;

    if (UTF8StringView(name) == "RegionController")
    {
        if (!document_controller)
            return nullptr;

        auto pbr_id = playback_region_id.value();
        auto ctler  = document_controller;

        auto* subctrl = new RegionController(description);
        if (!subctrl)
            return nullptr;

        auto& subject =
            document_controller->get_playback_region_changed_subject(pbr_id);

        subctrl->region_data_func = [=]() {
            return find_region_data(ctler, pbr_id);
        };

        subctrl->on_select_word_func = [=](Index index) {
            document_controller->get_region_selection_model().select(
                {pbr_id, static_cast<size_t>(index)});

            on_request_select_word(pbr_id, index, document_controller);
        };

        return subctrl->initialize(&subject) ? subctrl : nullptr;
    }

    return nullptr;
}

//------------------------------------------------------------------------
void ListController::on_focus_word(
    const SearchEngine::SearchResult& search_result)
{
    // word search_result selection
    if (!search_result.indices.empty() &&
        search_result.focused_word.has_value())
    {
        const auto index =
            search_result.indices.at(search_result.focused_word.value());
        on_request_select_word(search_result.region_id, index,
                               document_controller);

        // waveform selection
        document_controller->get_region_selection_model().select(
            {search_result.region_id,
             static_cast<size_t>(search_result.indices.at(
                 search_result.focused_word.value()))});
    }

    // ui update
    if (!rowColView)
        return;

    CView* toFind =
        find_region_view_by_id(*rowColView, search_result.region_id);
    if (toFind == nullptr)
        return;

    using Buttons = std::vector<WordButton*>;
    if (const auto* container = toFind->asViewContainer())
    {
        Buttons btns;
        container->getChildViewsOfType<WordButton>(btns, true);
        for (auto& btn : btns)
        {
            const auto new_state =
                get_button_state(search_result, btn->getTag());
            if (new_state == WordButton::State::kFocused)
                scroll_to_view(rowColView, btn);

            if (btn->setState(new_state))
                btn->invalid();
        }
    }
}

//------------------------------------------------------------------------
void ListController::viewWillDelete(VSTGUI::CView* view)
{
    if (view == rowColView)
    {
        rowColView->unregisterViewListener(this);
        rowColView = nullptr;
    }
}

//------------------------------------------------------------------------
} // namespace mam
