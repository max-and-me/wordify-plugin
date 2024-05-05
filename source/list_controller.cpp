//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "list_controller.h"
#include "hilite_text_button.h"
#include "list_entry_controller.h"
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
constexpr auto LIST_ENTRY_VIEW_TEMPLATE  = "ListEntryTemplate";

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
// ListController
//------------------------------------------------------------------------
ListController::ListController(ARADocumentController* controller,
                               const IUIDescription* uidesc)
: controller(controller)
, uidesc(uidesc)
{
    if (controller)
    {
        lifetime_observer = tiny_observer_pattern::make_observer(
            controller->get_playback_region_lifetimes_subject(),
            [&](const auto& data) {
                this->on_add_remove_playback_region(data);
            });

        order_observer = tiny_observer_pattern::make_observer(
            controller->get_playback_region_order_subject(),
            [&](const auto&) { this->on_playback_regions_reordered(); });

        word_selected_observer_id = controller->register_word_selected_observer(
            [this](const auto& data) { this->checkSelectWord(data); });
    }
}

//------------------------------------------------------------------------
ListController::~ListController()
{
    if (controller)
        controller->unregister_word_selected_observer(
            word_selected_observer_id);
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

    if (UTF8StringView(name) == "ListEntryController")
    {
        return new ListEntryController(controller, playback_region_id.value(),
                                       description);
    }

    return nullptr;
}

//------------------------------------------------------------------------
void ListController::checkSelectWord(const WordSelectData& data)
{
    // word data selection
    if (data.indices.empty() == false)
    {
        controller->onRequestSelectWord(data.indices.at(0), data.region_id);

        // waveform selection
        controller->get_region_selection_model().select(
            {data.region_id, static_cast<size_t>(data.indices.at(0))});
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
            if (region_id == data.region_id)
                toFind = child;
        }
    });

    if (toFind == nullptr)
        return;

    if (auto* container = toFind->asViewContainer())
    {
        std::vector<HiliteTextButton*> btns;
        container->getChildViewsOfType<HiliteTextButton>(btns, true);
        for (auto& btn : btns)
        {
            btn->setHilite(false);

            auto it = std::find(data.indices.begin(), data.indices.end(),
                                btn->getTag());
            if (it != data.indices.end())
            {
                auto rect = btn->translateToGlobal(rowColView->getViewSize());
                CScrollView* scroll = dynamic_cast<CScrollView*>(
                    rowColView->getParentView()->getParentView());
                if (scroll)
                    scroll->makeRectVisible(rect);
                auto* grandParent =
                    rowColView->getParentView()->getParentView();
                if (grandParent)
                {
                    CPoint btnRectGlobal = btn->getViewSize().getTopLeft();
                    btn->localToFrame(btnRectGlobal);

                    CRect btnRect = btn->getViewSize();
                    btnRect.offset(btnRectGlobal.x - btnRect.getWidth(),
                                   btnRectGlobal.y);
                    btn->setHilite(true);
                }
            }
            btn->setDirty();
        }
    }
}

//------------------------------------------------------------------------
} // namespace mam
