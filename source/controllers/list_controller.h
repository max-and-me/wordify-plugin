// Copyright(c) 2024 Max And Me.

#pragma once

#include "search_engine.h"
#include "warn_cpp/suppress_warnings.h"
#include "wordify_types.h"
BEGIN_SUPPRESS_WARNINGS
#include "ara_document_controller.h"
#include "base/source/fobject.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/uidescription/icontroller.h"
END_SUPPRESS_WARNINGS

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
    using PlaybackRegion      = meta_words::PlaybackRegion;
    using Control             = VSTGUI::CControl;
    using View                = VSTGUI::CView;
    using RowColumnView       = VSTGUI::CRowColumnView;
    using UIAttributes        = VSTGUI::UIAttributes;
    using IUIDescription      = VSTGUI::IUIDescription;
    using UTF8StringPtr       = VSTGUI::UTF8StringPtr;
    using IController         = VSTGUI::IController;
    using OptPlaybackRegionId = std::optional<Id>;

    ListController(ARADocumentController* controller,
                   const IUIDescription* uidesc);
    ~ListController();

    void PLUGIN_API update(FUnknown* /*changedUnknown*/,
                           Steinberg::int32 /*message*/) override{};
    View* verifyView(View* view,
                     const UIAttributes& attributes,
                     const IUIDescription* description) override;

    // IControlListener
    void valueChanged(Control* /*pControl*/) override{};
    void controlBeginEdit(Control* /*pControl*/) override{};
    void controlEndEdit(Control* /*pControl*/) override{};
    IController*
    createSubController(UTF8StringPtr name,
                        const IUIDescription* description) override;

    void viewWillDelete(VSTGUI::CView* view) override;

    OBJ_METHODS(ListController, FObject)

    //--------------------------------------------------------------------
private:
    void on_focus_word(const SearchEngine::SearchResult& search_result);
    void on_add_remove_playback_region(const RegionLifetimeEventData& data);
    void on_playback_regions_reordered();
    void on_region_selected_by_host(Id region_id);
    auto create_list_item_view(const Id id) -> VSTGUI::CView*;

    RowColumnView* rowColView         = nullptr;
    ARADocumentController* controller = nullptr;
    const IUIDescription* uidesc      = nullptr;
    OptPlaybackRegionId playback_region_id;

    RegionLifetimeCallback::Handle lifetime_observer_handle;
    RegionsOrderCallback::Handle order_observer_handle;
    RegionSelectedByHostCallback::Handle region_selected_by_host_handle;
    SearchEngine::SearchEngineCallback::Handle focus_word_observer_handle;
};

//------------------------------------------------------------------------
} // namespace mam
