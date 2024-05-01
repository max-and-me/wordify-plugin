//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "list_entry_controller.h"
#include "meta_words_clip_controller.h"
#include "meta_words_data.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/uidescription/uiattributes.h"
#include <memory>

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
                                public VSTGUI::IController
{
public:
    //--------------------------------------------------------------------
    using FuncMetaWordsData    = std::function<const MetaWordsData()>;
    using FuncListValueChanged = std::function<void(int)>;
    using Subject              = tiny_observer_pattern::SimpleSubject;
    using Width                = VSTGUI::CCoord;
    using ObserverPtr =
        std::unique_ptr<tiny_observer_pattern::Observer<Subject>>;

    struct Cache
    {
        using Widths = std::vector<Width>;
        Widths word_widths;
    };

    MetaWordsClipController(const VSTGUI::IUIDescription* description);

    bool initialize(Subject* subject, FuncMetaWordsData&& meta_words_data_func);

    VSTGUI::CView*
    verifyView(VSTGUI::CView* view,
               const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;

    // IControlListener
    void valueChanged(VSTGUI::CControl* pControl) override;

    FuncListValueChanged list_value_changed_func;

    OBJ_METHODS(MetaWordsClipController, FObject)
    //--------------------------------------------------------------------
private:
    void on_meta_words_data_changed();

    const VSTGUI::IUIDescription* description = nullptr;
    VSTGUI::CTextLabel* region_title          = nullptr;
    VSTGUI::CTextLabel* region_start_time     = nullptr;
    VSTGUI::CTextLabel* region_duration_time  = nullptr;
    VSTGUI::CViewContainer* region_transcript = nullptr;

    std::unique_ptr<VSTGUI::ViewListenerAdapter> view_listener;
    std::unique_ptr<HStackLayout> stack_layout;
    VSTGUI::UIAttributes meta_word_button_attributes;

    FuncMetaWordsData meta_words_data_func;
    ObserverPtr observer;

    Cache cache;
};
//------------------------------------------------------------------------
} // namespace mam
