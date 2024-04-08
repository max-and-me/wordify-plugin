//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ara_document_controller.h"
#include "base/source/fobject.h"
#include "vstgui/uidescription/icontroller.h"

namespace VSTGUI {
class CRowColumnView;
class IUIDescription;
} // namespace VSTGUI
namespace mam {
namespace meta_words {
class PlaybackRegion;
}
//------------------------------------------------------------------------
// ListController
//------------------------------------------------------------------------
class ListController : public Steinberg::FObject, public VSTGUI::IController
{
public:
    //--------------------------------------------------------------------
    using PlaybackRegion = meta_words::PlaybackRegion;

    ListController(
        ARADocumentController* controller,
        ARADocumentController::FnGetSampleRate&& playback_sample_rate_func,
        const VSTGUI::IUIDescription* ui_description);
    ~ListController() override;

    void PLUGIN_API update(FUnknown* changedUnknown,
                           Steinberg::int32 message) override{};
    VSTGUI::CView*
    verifyView(VSTGUI::CView* view,
               const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;
    // IControlListener
    void valueChanged(VSTGUI::CControl* pControl) override{};
    void controlBeginEdit(VSTGUI::CControl* pControl) override{};
    void controlEndEdit(VSTGUI::CControl* pControl) override{};
    VSTGUI::IController*
    createSubController(VSTGUI::UTF8StringPtr name,
                        const VSTGUI::IUIDescription* description) override;

    OBJ_METHODS(ListController, FObject)

    //--------------------------------------------------------------------
private:
    void on_add_remove_playback_region(const PlaybackRegionLifetimeData& data);
    void on_playback_regions_reordered();
    auto create_list_item_view(const PlaybackRegion::Id id) -> VSTGUI::CView*;

    VSTGUI::CRowColumnView* rowColView           = nullptr;
    const VSTGUI::IUIDescription* ui_description = nullptr;

    ARADocumentController* controller = nullptr;
    tiny_observer_pattern::ObserverID lifetime_observer_id = 0;
    tiny_observer_pattern::ObserverID order_observer_id    = 0;
    ARADocumentController::FnGetSampleRate playback_sample_rate_func;
    PlaybackRegion::Id tmp_playback_region_id = PlaybackRegion::INVALID_ID;
};

//------------------------------------------------------------------------
} // namespace mam
