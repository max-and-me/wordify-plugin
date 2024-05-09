//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ara_document_controller.h"
#include "base/source/fobject.h"
#include "vstgui/lib/iviewlistener.h"
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
class ListController : public Steinberg::FObject,
                       public VSTGUI::IController,
                       public VSTGUI::ViewListenerAdapter
{
public:
    //--------------------------------------------------------------------
    using PlaybackRegion   = meta_words::PlaybackRegion;
    using Control          = VSTGUI::CControl;
    using View             = VSTGUI::CView;
    using RowColumnView    = VSTGUI::CRowColumnView;
    using UIAttributes     = VSTGUI::UIAttributes;
    using IUIDescription   = VSTGUI::IUIDescription;
    using UTF8StringPtr    = VSTGUI::UTF8StringPtr;
    using IController      = VSTGUI::IController;
    using LifetimeObserver = tiny_observer_pattern::Observer<
        ARADocumentController::PlaybackRegionLifetimesSubject>;
    using LifetimeObserverPtr = std::unique_ptr<LifetimeObserver>;
    using OrderObserver       = tiny_observer_pattern::Observer<
              ARADocumentController::PlaybackRegionsOrderSubject>;
    using OrderObserverPtr    = std::unique_ptr<OrderObserver>;
    using OptPlaybackRegionId = std::optional<PlaybackRegion::Id>;

    ListController(ARADocumentController* controller,
                   const IUIDescription* uidesc);
    ~ListController();

    void PLUGIN_API update(FUnknown* changedUnknown,
                           Steinberg::int32 message) override {};
    View* verifyView(View* view,
                     const UIAttributes& attributes,
                     const IUIDescription* description) override;

    using WordSelectObserver = tiny_observer_pattern::Observer<
        ARADocumentController::WordSelectSubject>;
    using WordSelectObserverPtr = std::unique_ptr<WordSelectObserver>;

    // IControlListener
    void valueChanged(Control* pControl) override {};
    void controlBeginEdit(Control* pControl) override {};
    void controlEndEdit(Control* pControl) override {};
    IController*
    createSubController(UTF8StringPtr name,
                        const IUIDescription* description) override;

    void viewWillDelete(VSTGUI::CView* view) override;

    OBJ_METHODS(ListController, FObject)

    //--------------------------------------------------------------------
private:
    void checkSelectWord(const WordSelectData& data);
    void on_add_remove_playback_region(const PlaybackRegionLifetimeData& data);
    void on_playback_regions_reordered();
    auto create_list_item_view(const PlaybackRegion::Id id) -> VSTGUI::CView*;

    RowColumnView* rowColView    = nullptr;
    const IUIDescription* uidesc = nullptr;

    ARADocumentController* controller = nullptr;
    LifetimeObserverPtr lifetime_observer;
    OrderObserverPtr order_observer;
    OptPlaybackRegionId playback_region_id;
    WordSelectObserverPtr word_selected_observer;
    ARADocumentController::ObserverID word_selected_observer_id = 0;
};

//------------------------------------------------------------------------
} // namespace mam
