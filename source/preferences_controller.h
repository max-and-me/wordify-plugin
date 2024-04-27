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

namespace Steinberg::Vst {
class Parameter;
}

//------------------------------------------------------------------------
namespace mam {

//------------------------------------------------------------------------
// PreferencesController
//------------------------------------------------------------------------
class PreferencesController : public Steinberg::FObject,
                              public VSTGUI::IController
{
public:
    //--------------------------------------------------------------------
    using SchemeToggle = std::function<void(bool)>;

    PreferencesController(ARADocumentController* controller,
                          Steinberg::Vst::Parameter* param);
    ~PreferencesController() override;

    void PLUGIN_API update(FUnknown* changedUnknown,
                           Steinberg::int32 message) override;

    VSTGUI::CView*
    verifyView(VSTGUI::CView* view,
               const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;

    // IControlListener
    void valueChanged(VSTGUI::CControl* pControl) override;

    OBJ_METHODS(PreferencesController, FObject)

    //--------------------------------------------------------------------
private:
    ARADocumentController* controller = nullptr;
    VSTGUI::COptionMenu* options_menu = nullptr;
    VSTGUI::CControl* scheme_switch   = nullptr;
    Steinberg::Vst::Parameter* param  = nullptr;
};

//------------------------------------------------------------------------
} // namespace mam
