// Copyright(c) 2024 Max And Me.

#pragma once

#include "search_engine.h"
#include "supress_warnings.h"
#include "wordify_types.h"
BEGIN_SUPRESS_WARNINGS
#include "ara_document_controller.h"
#include "base/source/fobject.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/uidescription/icontroller.h"
END_SUPRESS_WARNINGS

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
                           Steinberg::int32 /*message*/) override {};
    View* verifyView(View* view,
                     const UIAttributes& attributes,
                     const IUIDescription* description) override;

    // IControlListener
    void valueChanged(Control* /*pControl*/) override {};
    void controlBeginEdit(Control* /*pControl*/) override {};
    void controlEndEdit(Control* /*pControl*/) override {};
    IController*
    createSubController(UTF8StringPtr name,
                        const IUIDescription* description) override;

    void viewWillDelete(VSTGUI::CView* view) override;

    OBJ_METHODS(ListController, FObject)

    //--------------------------------------------------------------------
private:
    void checkSelectWord(const SearchEngine::SearchResult& search_result);
    void on_add_remove_playback_region(const RegionLifetimeEventData& data);
    void on_playback_regions_reordered();
    auto create_list_item_view(const Id id) -> VSTGUI::CView*;

    RowColumnView* rowColView    = nullptr;
    const IUIDescription* uidesc = nullptr;

    ARADocumentController* controller = nullptr;
    OptPlaybackRegionId playback_region_id;

    RegionLifetimeCallback::Handle lifetime_observer_handle;
    RegionsOrderCallback::Handle order_observer_handle;
    SearchEngine::SearchEngineCallback::Handle word_selected_observer_handle;
};

//------------------------------------------------------------------------
} // namespace mam
