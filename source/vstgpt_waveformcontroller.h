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
// VstGPTWaveFormController
//------------------------------------------------------------------------
class VstGPTWaveFormController : public Steinberg::FObject,
                                 public VSTGUI::IController
{
public:
    //--------------------------------------------------------------------

    VstGPTWaveFormController(ARADocumentController& controller);
    virtual ~VstGPTWaveFormController();

    void PLUGIN_API update(FUnknown* changedUnknown,
                           Steinberg::int32 message) override{};
    VSTGUI::CView*
    verifyView(VSTGUI::CView* view,
               const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override
    {
        return view;
    }

    VSTGUI::CView*
    createView(const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;

    // IControlListener
    void valueChanged(VSTGUI::CControl* pControl) override{};

    OBJ_METHODS(VstGPTWaveFormController, FObject)

    //--------------------------------------------------------------------
private:
    ARADocumentController& controller;
    VSTGUI::CView* view;
};

//------------------------------------------------------------------------
} // namespace mam
