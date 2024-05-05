//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "list_entry_controller.h"
#include "meta_words_data.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/uidescription/uiattributes.h"

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
// VstGPTWaveClipListController
//------------------------------------------------------------------------
class MetaWordsClipController : public Steinberg::FObject,
                                public VSTGUI::IController,
                                public VSTGUI::ViewListenerAdapter
{
public:
    //--------------------------------------------------------------------
    using FuncMetaWordsData  = std::function<const MetaWordsData()>;
    using FuncOnSelectedWord = std::function<void(int)>;
    using Subject            = tiny_observer_pattern::SimpleSubject;
    using Width              = VSTGUI::CCoord;
    using ObserverPtr =
        std::unique_ptr<tiny_observer_pattern::Observer<Subject>>;

    struct Cache
    {
        using Widths = std::vector<Width>;
        Widths word_widths;
    };

    MetaWordsClipController(const VSTGUI::IUIDescription* description);
    ~MetaWordsClipController() override;

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
    FuncMetaWordsData meta_words_data_func;

    OBJ_METHODS(MetaWordsClipController, FObject)
    //--------------------------------------------------------------------
private:
    void on_meta_words_data_changed();
    void init_words_width_cache(const MetaWordsData& data);

    const VSTGUI::IUIDescription* description = nullptr;
    VSTGUI::CTextLabel* region_title          = nullptr;
    VSTGUI::CTextLabel* region_start_time     = nullptr;
    VSTGUI::CTextLabel* region_duration_time  = nullptr;
    VSTGUI::CViewContainer* region_transcript = nullptr;

    std::unique_ptr<HStackLayout> stack_layout;
    VSTGUI::UIAttributes meta_word_button_attributes;

    ObserverPtr observer;
    Cache cache;
};
//------------------------------------------------------------------------
} // namespace mam
