// Copyright(c) 2024 Max And Me.

#pragma once

#include "region_data.h"
#include "supress_warnings.h"
BEGIN_SUPRESS_WARNINGS
#include "base/source/fobject.h"
#include "eventpp/callbacklist.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/uidescription/icontroller.h"
#include "vstgui/uidescription/uiattributes.h"
END_SUPRESS_WARNINGS

namespace VSTGUI {
class IUIDescription;
class CTextLabel;
class CTextLabel;
class CViewContainer;
class CView;
}; // namespace VSTGUI

namespace mam {

class HStackLayout;

//------------------------------------------------------------------------
// RegionController
//------------------------------------------------------------------------
class RegionController : public Steinberg::FObject,
                         public VSTGUI::IController,
                         public VSTGUI::ViewListenerAdapter
{
public:
    //--------------------------------------------------------------------
    using FuncRegionData     = std::function<const RegionData()>;
    using FuncOnSelectedWord = std::function<void(int)>;
    using Subject            = eventpp::CallbackList<void(void)>;
    using ObserverHandle     = Subject::Handle;
    using Width              = VSTGUI::CCoord;

    struct Cache
    {
        using Widths = std::vector<Width>;
        Widths word_widths;
    };

    RegionController(const VSTGUI::IUIDescription* description);
    ~RegionController() override;

    bool initialize(Subject* subject);

    VSTGUI::CView*
    verifyView(VSTGUI::CView* view,
               const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;

    // IControlListener
    void valueChanged(VSTGUI::CControl* pControl) override;

    // ViewListenerAdapter
    void viewAttached(VSTGUI::CView* view) override;
    void viewRemoved(VSTGUI::CView* view) override;
    void viewWillDelete(VSTGUI::CView* view) override;

    FuncOnSelectedWord on_select_word_func;
    FuncRegionData region_data_func;

    OBJ_METHODS(RegionController, FObject)
    //--------------------------------------------------------------------
private:
    void on_region_changed();
    void init_words_width_cache(const RegionData& data);

    const VSTGUI::IUIDescription* description = nullptr;
    VSTGUI::CTextLabel* region_title          = nullptr;
    VSTGUI::CTextLabel* region_start_time     = nullptr;
    VSTGUI::CTextLabel* region_duration_time  = nullptr;
    VSTGUI::CViewContainer* region_transcript = nullptr;

    std::unique_ptr<HStackLayout> stack_layout;
    VSTGUI::UIAttributes meta_word_button_attributes;

    Subject* subject = nullptr;
    ObserverHandle observer_handle;
    Cache cache;
};
//------------------------------------------------------------------------
} // namespace mam
