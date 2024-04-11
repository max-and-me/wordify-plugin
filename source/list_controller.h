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
    using Control        = VSTGUI::CControl;
    using View           = VSTGUI::CView;
    using RowColumnView  = VSTGUI::CRowColumnView;
    using UIAttributes   = VSTGUI::UIAttributes;
    using IUIDescription = VSTGUI::IUIDescription;
    using UTF8StringPtr  = VSTGUI::UTF8StringPtr;
    using IController    = VSTGUI::IController;

    ListController(
        ARADocumentController* controller,
        ARADocumentController::FuncSampleRate&& playback_sample_rate_func,
        const IUIDescription* ui_description);
    ~ListController() override;

    void PLUGIN_API update(FUnknown* changedUnknown,
                           Steinberg::int32 message) override {};
    View* verifyView(View* view,
                     const UIAttributes& attributes,
                     const IUIDescription* description) override;
    // IControlListener
    void valueChanged(Control* pControl) override {};
    void controlBeginEdit(Control* pControl) override {};
    void controlEndEdit(Control* pControl) override {};
    IController*
    createSubController(UTF8StringPtr name,
                        const IUIDescription* description) override;

    OBJ_METHODS(ListController, FObject)

    //--------------------------------------------------------------------
private:
    void on_add_remove_playback_region(const PlaybackRegionLifetimeData& data);
    void on_playback_regions_reordered();
    auto create_list_item_view(const PlaybackRegion::Id id) -> VSTGUI::CView*;

    RowColumnView* rowColView            = nullptr;
    const IUIDescription* ui_description = nullptr;

    ARADocumentController* controller                      = nullptr;
    tiny_observer_pattern::ObserverID lifetime_observer_id = 0;
    tiny_observer_pattern::ObserverID order_observer_id    = 0;
    ARADocumentController::FuncSampleRate playback_sample_rate_func;
    PlaybackRegion::Id tmp_playback_region_id = PlaybackRegion::INVALID_ID;
};

//------------------------------------------------------------------------
} // namespace mam
