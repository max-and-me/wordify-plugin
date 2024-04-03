//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ara_document_controller.h"
#include "base/source/fobject.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/uidescription/icontroller.h"
#include <functional>

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
    //--------------------------------------------------------------------
    VstGPTListController(
        ARADocumentController& controller,
        ARADocumentController::FnGetSampleRate&& fn_get_playback_sample_rate);
    virtual ~VstGPTListController();

    void PLUGIN_API update(FUnknown* changedUnknown,
                           Steinberg::int32 message) override {};
    VSTGUI::CView*
    verifyView(VSTGUI::CView* view,
               const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;
    // IControlListener
    void valueChanged(VSTGUI::CControl* pControl) override {};
    void controlBeginEdit(VSTGUI::CControl* pControl) override {};
    void controlEndEdit(VSTGUI::CControl* pControl) override {};
    VSTGUI::IController* createSubController(VSTGUI::UTF8StringPtr name, const VSTGUI::IUIDescription* description) override;

    OBJ_METHODS(VstGPTListController, FObject)

    //--------------------------------------------------------------------
private:
    void onDataChanged();

    VSTGUI::CListControl* listControl = nullptr;
    VSTGUI::CTextLabel* label         = nullptr;
    
    VSTGUI::CRowColumnView* rowColView = nullptr;
    
    using SubControllerList = std::vector<VSTGUI::IController*>;
    SubControllerList subControllerList;

    ARADocumentController& controller;
    ARADocumentController::MetaWordsDataList cached_meta_words_data_list;
    tiny_observer_pattern::ObserverID observer_id = 0;
    ARADocumentController::FnGetSampleRate fn_get_playback_sample_rate;
};

//------------------------------------------------------------------------
} // namespace mam
