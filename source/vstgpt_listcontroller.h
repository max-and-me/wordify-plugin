//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ara_document_controller.h"
#include "base/source/fobject.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/uidescription/icontroller.h"

namespace VSTGUI {
class CListControl;
}
namespace mam {

//------------------------------------------------------------------------
// VstGPTListController
//------------------------------------------------------------------------
class VstGPTListController : public Steinberg::FObject,
                             public VSTGUI::IController
{
public:
    //------------------------------------------------------------------------
    VstGPTListController(ARADocumentController& controller);
    virtual ~VstGPTListController();

    void PLUGIN_API update(FUnknown* changedUnknown,
                           Steinberg::int32 message) override{};
    VSTGUI::CView*
    verifyView(VSTGUI::CView* view,
               const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;
    // IControlListener
    void valueChanged(VSTGUI::CControl* pControl) override;
    void controlBeginEdit(VSTGUI::CControl* pControl) override{};
    void controlEndEdit(VSTGUI::CControl* pControl) override{};

    void onDataChanged();

    OBJ_METHODS(VstGPTListController, FObject)
    //------------------------------------------------------------------------
private:
    VSTGUI::CListControl* listControl = nullptr;
    ARADocumentController& controller;
    ARADocumentController::MetaWordsDataList cached_meta_words_data_list;
    tiny_observer_pattern::ObserverID observer_id = 0;
};

//------------------------------------------------------------------------
} // namespace mam
