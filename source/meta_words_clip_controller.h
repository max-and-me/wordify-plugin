//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ara_document_controller.h"
#include "list_entry_controller.h"
#include "meta_words_clip_controller.h"
#include "meta_words_data.h"
#include "vstgui/lib/controls/cstringlist.h"
#include "vstgui/lib/controls/ctextlabel.h"
#include "vstgui/lib/platform/platformfactory.h"

namespace mam {

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

    MetaWordsClipController(tiny_observer_pattern::SimpleSubject* subject);
    ~MetaWordsClipController() override;

    void onDataChanged();

    VSTGUI::CView*
    verifyView(VSTGUI::CView* view,
               const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;

    // IControlListener
    void valueChanged(VSTGUI::CControl* pControl) override;

    auto
    set_meta_words_data_func(const FuncMetaWordsData&& meta_words_data_func)
        -> void;
    auto
    set_list_clicked_func(const FuncListValueChanged&& list_value_changed_func)
        -> void;

    OBJ_METHODS(MetaWordsClipController, FObject)
    //--------------------------------------------------------------------
private:
    VSTGUI::CListControl* listControl = nullptr;
    VSTGUI::CTextLabel* label         = nullptr;

    FuncMetaWordsData meta_words_data_func;
    FuncListValueChanged list_value_changed_func;
    tiny_observer_pattern::SimpleSubject* subject = nullptr;
    tiny_observer_pattern::ObserverID observer_id = 0;
};
//------------------------------------------------------------------------
} // namespace mam