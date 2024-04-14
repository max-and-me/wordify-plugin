//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "list_entry_controller.h"
#include "meta_words_clip_controller.h"
#include "meta_words_data.h"
#include "vstgui/lib/controls/cstringlist.h"
#include "vstgui/lib/controls/ctextlabel.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/lib/platform/platformfactory.h"
#include "vstgui/uidescription/uiattributes.h"
#include <memory>

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

    MetaWordsClipController(const VSTGUI::IUIDescription* description);
    ~MetaWordsClipController() override;

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
    VSTGUI::CListControl* listControl         = nullptr;
    VSTGUI::CTextLabel* listTitle             = nullptr;
    VSTGUI::CTextLabel* timeDisplay           = nullptr;
    VSTGUI::CView* spinner                    = nullptr;
    VSTGUI::CViewContainer* root_view         = nullptr;
    VSTGUI::CViewContainer* text_document     = nullptr;
    std::unique_ptr<VSTGUI::ViewListenerAdapter> view_listener;
    std::unique_ptr<HStackLayout> stack_layout;
    VSTGUI::UIAttributes meta_word_button_attributes;

    FuncMetaWordsData meta_words_data_func;
    tiny_observer_pattern::SimpleSubject* subject = nullptr;
    tiny_observer_pattern::ObserverID observer_id = 0;
};
//------------------------------------------------------------------------
} // namespace mam
