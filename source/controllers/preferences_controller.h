// Copyright(c) 2024 Max And Me.

#pragma once

#include "warn_cpp/suppress_warnings.h"
BEGIN_SUPPRESS_WARNINGS
#include "base/source/fobject.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/uidescription/icontroller.h"
END_SUPPRESS_WARNINGS

namespace VSTGUI {
class COptionMenu;
}

namespace mam {
class ARADocumentController;

//------------------------------------------------------------------------
// PreferencesController
//------------------------------------------------------------------------
class PreferencesController : public Steinberg::FObject,
                              public VSTGUI::IController,
                              public VSTGUI::ViewListenerAdapter
{
public:
    //--------------------------------------------------------------------
    using SchemeToggle = std::function<void(bool)>;

    PreferencesController(ARADocumentController* controller);
    ~PreferencesController() override;

    // IController
    void PLUGIN_API update(FUnknown* changedUnknown,
                           Steinberg::int32 message) override;
    VSTGUI::CView*
    verifyView(VSTGUI::CView* view,
               const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;

    // IControlListener
    void valueChanged(VSTGUI::CControl* pControl) override;

    // IViewListener
    void viewWillDelete(VSTGUI::CView* view) override;

    OBJ_METHODS(PreferencesController, FObject)

    //--------------------------------------------------------------------
private:
    ARADocumentController* controller = nullptr;
    VSTGUI::COptionMenu* options_menu = nullptr;
};

//------------------------------------------------------------------------
} // namespace mam
