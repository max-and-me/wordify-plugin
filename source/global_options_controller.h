//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ara_document_controller.h"
#include "base/source/fobject.h"
#include "vstgui/uidescription/icontroller.h"

namespace VSTGUI {
class COptionMenu;
}

//------------------------------------------------------------------------
namespace mam {

//------------------------------------------------------------------------
// GlobalOptionsController
//------------------------------------------------------------------------
class GlobalOptionsController : public Steinberg::FObject,
                                public VSTGUI::IController
{
public:
    //--------------------------------------------------------------------
    GlobalOptionsController(ARADocumentController* controller);
    ~GlobalOptionsController() override;

    void PLUGIN_API update(FUnknown* changedUnknown,
                           Steinberg::int32 message) override {};

    VSTGUI::CView*
    verifyView(VSTGUI::CView* view,
               const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;

    // IControlListener
    void valueChanged(VSTGUI::CControl* pControl) override {};

    OBJ_METHODS(GlobalOptionsController, FObject)

    //--------------------------------------------------------------------
private:
    ARADocumentController* controller = nullptr;
    VSTGUI::COptionMenu* options_menu = nullptr;
};

//------------------------------------------------------------------------
} // namespace mam
