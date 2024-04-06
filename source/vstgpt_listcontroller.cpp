//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "vstgpt_listcontroller.h"
#include "list_entry_controller.h"
#include "vstgui/lib/crowcolumnview.h"
#include "vstgui/uidescription/iuidescription.h"
#include <string>

using namespace VSTGUI;

namespace mam {

//------------------------------------------------------------------------
// VstGPTListController
//------------------------------------------------------------------------
constexpr size_t PLAYBACK_REGION_ID_ATTR = 123456789;
VstGPTListController::VstGPTListController(
    ARADocumentController& controller,
    ARADocumentController::FnGetSampleRate&& fn_get_playback_sample_rate,
    const VSTGUI::IUIDescription* ui_description)
: controller(controller)
, fn_get_playback_sample_rate(fn_get_playback_sample_rate)
, ui_description(ui_description)
{
    observer_id = controller.register_playback_region_lifetimes_observer(
        [this](const auto& data) { this->onDataChanged(data); });
}

//------------------------------------------------------------------------
VstGPTListController::~VstGPTListController()
{
    controller.unregister_playback_region_lifetimes_observer(observer_id);
}

//------------------------------------------------------------------------
CView* VstGPTListController::verifyView(CView* view,
                                        const UIAttributes& /*attributes*/,
                                        const IUIDescription* description)
{
    if (!rowColView)
    {
        if (rowColView = dynamic_cast<CRowColumnView*>(view))
        {
            controller.for_each_playback_region(
                [&](const PlaybackRegion* playbackRegion) {
                    if (!playbackRegion)
                        return;

                    const IUIDescription* uidescription = description;
                    if (!uidescription)
                        return;

                    tmp_playback_region_id = playbackRegion->get_id();
                    auto* newView =
                        uidescription->createView("ListEntryTemplate", this);
                    newView->setAttribute(PLAYBACK_REGION_ID_ATTR,
                                          tmp_playback_region_id);
                    tmp_playback_region_id = PlaybackRegion::INVALID_ID;

                    if (newView)
                        rowColView->addView(newView);
                });
        }
    }

    return view;
}

//------------------------------------------------------------------------
void VstGPTListController::onDataChanged(const PlaybackRegionLifetimeData& data)
{
    if (!ui_description)
        return;

    switch (data.event)
    {
        case PlaybackRegionLifetimeData::Event::HasBeenAdded: {
            tmp_playback_region_id = data.id;
            auto* newView =
                ui_description->createView("ListEntryTemplate", this);
            newView->setAttribute(PLAYBACK_REGION_ID_ATTR,
                                  tmp_playback_region_id);
            tmp_playback_region_id = PlaybackRegion::INVALID_ID;

            if (newView)
                rowColView->addView(newView);

            break;
        }

        case PlaybackRegionLifetimeData::Event::WillBeRemoved: {
            if (rowColView)
            {
                CView* viewToRemove = nullptr;
                rowColView->forEachChild([&, pbr_id = data.id](CView* view) {
                    if (viewToRemove)
                        return;

                    auto id = PlaybackRegion::INVALID_ID;
                    if (view->getAttribute(PLAYBACK_REGION_ID_ATTR, id))
                    {
                        if (pbr_id == id)
                            viewToRemove = view;
                    }
                });

                if (viewToRemove)
                    rowColView->removeView(viewToRemove);
            }
            break;
        }
    }
}

//------------------------------------------------------------------------
VSTGUI::IController* VstGPTListController::createSubController(
    VSTGUI::UTF8StringPtr name, const VSTGUI::IUIDescription* description)
{
    if (tmp_playback_region_id == PlaybackRegion::INVALID_ID)
        return nullptr;

    if (VSTGUI::UTF8StringView(name) == "ListEntryController")
    {
        return new ListEntryController(controller,
                                       this->fn_get_playback_sample_rate,
                                       tmp_playback_region_id);
    }

    return nullptr;
}

//------------------------------------------------------------------------
} // namespace mam
