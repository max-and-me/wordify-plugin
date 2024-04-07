//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "list_controller.h"
#include "list_entry_controller.h"
#include "vstgui/lib/crowcolumnview.h"
#include "vstgui/uidescription/iuidescription.h"
#include <string>

using namespace VSTGUI;

namespace mam {

//------------------------------------------------------------------------
constexpr size_t PLAYBACK_REGION_ID_ATTR = 123456789;

//------------------------------------------------------------------------
static auto find_view_by_id(const VSTGUI::CRowColumnView* rowColView,
                            ListController::PlaybackRegion::Id id)
    -> VSTGUI::CView*
{
    CView* viewToFind = nullptr;
    rowColView->forEachChild([&, pbr_id = id](CView* view) {
        if (viewToFind)
            return;

        auto id = ListController::PlaybackRegion::INVALID_ID;
        if (view->getAttribute(PLAYBACK_REGION_ID_ATTR, id))
        {
            if (pbr_id == id)
                viewToFind = view;
        }
    });

    return viewToFind;
}

//------------------------------------------------------------------------
// ListController
//------------------------------------------------------------------------
ListController::ListController(
    ARADocumentController& controller,
    ARADocumentController::FnGetSampleRate&& playback_sample_rate_func,
    const VSTGUI::IUIDescription* ui_description)
: controller(controller)
, playback_sample_rate_func(playback_sample_rate_func)
, ui_description(ui_description)
{
    lifetime_observer_id =
        controller.register_playback_region_lifetimes_observer(
            [this](const auto& data) {
                this->on_add_remove_playback_region(data);
            });

    order_observer_id = controller.register_playback_region_order_observer(
        [this](const auto&) { this->on_playback_regions_reordered(); });
}

//------------------------------------------------------------------------
ListController::~ListController()
{
    controller.unregister_playback_region_lifetimes_observer(
        lifetime_observer_id);

    controller.unregister_playback_region_order_observer(order_observer_id);
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
            controller.for_each_playback_region_id(
                [&](const PlaybackRegion::Id id) {
                    auto* newView = create_list_item_view(id);
                    if (newView)
                        rowColView->addView(newView);
                });
        }
    }

    return view;
}

//------------------------------------------------------------------------
auto ListController::create_list_item_view(const PlaybackRegion::Id id)
    -> CView*
{
    if (!ui_description)
        return nullptr;

    tmp_playback_region_id = id;

    auto* newView = ui_description->createView("ListEntryTemplate", this);
    newView->setAttribute(PLAYBACK_REGION_ID_ATTR, id);

    tmp_playback_region_id = PlaybackRegion::INVALID_ID;

    return newView;
}

//------------------------------------------------------------------------
void ListController::on_playback_regions_reordered()
{
    if (!rowColView)
        return;

    controller.for_each_playback_region_id_enumerated(
        [&](size_t index, meta_words::PlaybackRegion::Id id) {
            auto* viewToMove = find_view_by_id(rowColView, id);
            rowColView->changeViewZOrder(viewToMove,
                                         static_cast<uint32_t>(index));
        });

    rowColView->invalid();
}

//------------------------------------------------------------------------
void ListController::on_add_remove_playback_region(
    const PlaybackRegionLifetimeData& data)
{
    if (!ui_description)
        return;

    switch (data.event)
    {
        case PlaybackRegionLifetimeData::Event::HasBeenAdded: {
            auto* viewToAdd = create_list_item_view(data.id);
            if (viewToAdd)
            {
                rowColView->addView(viewToAdd);
                rowColView->invalid();
            }

            break;
        }
        case PlaybackRegionLifetimeData::Event::WillBeRemoved: {
            auto* viewToRemove = find_view_by_id(rowColView, data.id);
            if (viewToRemove)
            {
                rowColView->removeView(viewToRemove);
                rowColView->invalid();
            }
            break;
        }
    }
}

//------------------------------------------------------------------------
VSTGUI::IController*
ListController::createSubController(VSTGUI::UTF8StringPtr name,
                                    const VSTGUI::IUIDescription* description)
{
    if (tmp_playback_region_id == PlaybackRegion::INVALID_ID)
        return nullptr;

    if (VSTGUI::UTF8StringView(name) == "ListEntryController")
    {
        return new ListEntryController(controller,
                                       this->playback_sample_rate_func,
                                       tmp_playback_region_id);
    }

    return nullptr;
}

//------------------------------------------------------------------------
} // namespace mam
