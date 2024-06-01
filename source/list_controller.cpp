//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "list_controller.h"
#include "hilite_text_button.h"
#include "meta_words_clip_controller.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/controls/cbuttons.h"
#include "vstgui/lib/crowcolumnview.h"
#include "vstgui/lib/cscrollview.h"
#include "vstgui/uidescription/iuidescription.h"
#include <string>

using namespace VSTGUI;

namespace mam {

//------------------------------------------------------------------------
constexpr size_t PLAYBACK_REGION_ID_ATTR = 123456789;
constexpr auto LIST_ENTRY_VIEW_TEMPLATE  = "ClipTemplate";

//------------------------------------------------------------------------
static auto find_view_by_id(const CRowColumnView& rowColView,
                            ListController::PlaybackRegion::Id pbr_id) -> CView*
{
    CView* viewToFind = nullptr;
    rowColView.forEachChild([&, pbr_id](CView* view) {
        if (viewToFind)
            return;

        meta_words::PlaybackRegion::Id id = 0;
        if (view->getAttribute(PLAYBACK_REGION_ID_ATTR, id))
        {
            if (pbr_id == size_t(id))
                viewToFind = view;
        }
    });

    return viewToFind;
}

//------------------------------------------------------------------------
auto get_button_state(const search_engine::SearchResult& search_results,
                      int32_t control_tag) -> HiliteTextButton::HiliteState
{
    using State    = HiliteTextButton::HiliteState;
    auto new_state = State::kNone;

    const auto& indices    = search_results.indices;
    const auto opt_focused = search_results.focused_word;

    auto iter = std::find(indices.begin(), indices.end(), control_tag);
    if (iter != indices.end())
    {
        const auto i = std::distance(indices.begin(), iter);
        new_state    = State::kSearchHilite;
        if (opt_focused)
        {
            new_state = (i == opt_focused.value()) ? State::kSearchSelectHilite
                                                   : State::kSearchHilite;
        }
    }

    return new_state;
}

//------------------------------------------------------------------------
auto scroll_to_view(CRowColumnView* rowColView, const CView* view)
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
// ListController
//------------------------------------------------------------------------
ListController::ListController(ARADocumentController* controller,
                               const IUIDescription* uidesc)
: controller(controller)
, uidesc(uidesc)
{
    if (controller)
    {
        lifetime_observer_handle =
            controller->get_playback_region_lifetimes_subject()->append(
                [&](const auto& data) {
                    this->on_add_remove_playback_region(data);
                });

        order_observer_handle =
            controller->get_playback_region_order_subject()->append(
                [&](const auto&) { this->on_playback_regions_reordered(); });

        word_selected_observer_handle =
            controller->get_selected_word_subject()->append(
                [this](const auto& data) {
                    for (const auto& result : data)
                        this->checkSelectWord(result);
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

    if (controller)
    {
        controller->get_playback_region_order_subject()->remove(
            order_observer_handle);

        controller->get_selected_word_subject()->remove(
            word_selected_observer_handle);

        controller->get_playback_region_lifetimes_subject()->remove(
            lifetime_observer_handle);
    }
}

//------------------------------------------------------------------------
CView* ListController::verifyView(CView* view,
                                  const UIAttributes& /*attributes*/,
                                  const IUIDescription* description)
{
    if (!rowColView)
    {
        if (rowColView = dynamic_cast<CRowColumnView*>(view))
        {
            rowColView->registerViewListener(this);
            if (controller)
            {
                controller->for_each_playback_region_id(
                    [&](const PlaybackRegion::Id id) {
                        auto* newView = create_list_item_view(id);
                        if (newView)
                        {
                            rowColView->addView(newView);
                            rowColView->sizeToFit();
                            newView->setAttribute('prid', id);
                        }
                    });
            }
        }
    }

    return view;
}

//------------------------------------------------------------------------
auto ListController::create_list_item_view(const PlaybackRegion::Id id)
    -> CView*
{
    if (!uidesc)
        return nullptr;

    playback_region_id = id;

    auto* newView = uidesc->createView(LIST_ENTRY_VIEW_TEMPLATE, this);
    newView->setAttribute(PLAYBACK_REGION_ID_ATTR, id);

    playback_region_id.reset();

    return newView;
}

//------------------------------------------------------------------------
void ListController::on_playback_regions_reordered()
{
    if (!rowColView)
        return;

    if (!controller)
        return;

    auto func = [&](size_t index, meta_words::PlaybackRegion::Id id) {
        auto* viewToMove = find_view_by_id(*rowColView, id);
        rowColView->changeViewZOrder(viewToMove, static_cast<uint32_t>(index));
    };

    controller->for_each_playback_region_id_enumerated(func);

    rowColView->invalid();
}

//------------------------------------------------------------------------
void ListController::on_add_remove_playback_region(
    const PlaybackRegionLifetimeData& data)
{
    if (!uidesc)
        return;

    if (!rowColView)
        return;

    switch (data.event)
    {
        case PlaybackRegionLifetimeData::Event::HasBeenAdded: {
            auto* viewToAdd = create_list_item_view(data.id);
            if (viewToAdd)
            {
                rowColView->addView(viewToAdd);
                rowColView->sizeToFit();
                on_playback_regions_reordered();
            }

            break;
        }
        case PlaybackRegionLifetimeData::Event::WillBeRemoved: {
            auto* viewToRemove = find_view_by_id(*rowColView, data.id);
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

    if (UTF8StringView(name) == "MetaWordsClipController")
    {
        if (!controller)
            return nullptr;

        auto pbr_id = playback_region_id.value();
        auto ctler  = this->controller;

        auto* subctrl = new MetaWordsClipController(description);
        if (!subctrl)
            return nullptr;

        auto& subject = controller->get_playback_region_changed_subject(pbr_id);

        subctrl->meta_words_data_func = [=]() {
            return build_meta_words_data(ctler, pbr_id);
        };

        subctrl->on_select_word_func = [=](int index) {
            controller->get_region_selection_model().select(
                {pbr_id, static_cast<size_t>(index)});
            controller->onRequestSelectWord(index, pbr_id);
        };

        return subctrl->initialize(&subject) ? subctrl : nullptr;
    }

    return nullptr;
}

//------------------------------------------------------------------------
void ListController::checkSelectWord(
    const search_engine::SearchResult& search_result)
{
    // word search_result selection
    if (!search_result.indices.empty() &&
        search_result.focused_word.has_value())
    {
        controller->onRequestSelectWord(
            search_result.indices.at(search_result.focused_word.value()),
            search_result.region_id);

        // waveform selection
        controller->get_region_selection_model().select(
            {search_result.region_id,
             static_cast<size_t>(search_result.indices.at(
                 search_result.focused_word.value()))});
    }

    // ui update
    if (!rowColView)
        return;

    CView* toFind = nullptr;
    rowColView->forEachChild([&](CView* child) {
        if (toFind)
            return;

        meta_words::PlaybackRegion::Id region_id = 0;

        if (child->getAttribute('prid', region_id))
        {
            if (region_id == search_result.region_id)
                toFind = child;
        }
    });

    if (toFind == nullptr)
        return;

    using State   = HiliteTextButton::HiliteState;
    using Buttons = std::vector<HiliteTextButton*>;
    if (const auto* container = toFind->asViewContainer())
    {
        Buttons btns;
        container->getChildViewsOfType<HiliteTextButton>(btns, true);
        for (auto& btn : btns)
        {
            const auto new_state =
                get_button_state(search_result, btn->getTag());
            if (new_state == HiliteTextButton::HiliteState::kSearchSelectHilite)
                scroll_to_view(rowColView, btn);

            if (btn->setHilite(new_state))
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
