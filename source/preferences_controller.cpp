//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "preferences_controller.h"
#include "vstgui/lib/controls/coptionmenu.h"
#include "vstgui/uidescription/uiattributes.h"

namespace mam {

//------------------------------------------------------------------------
using namespace VSTGUI;

//------------------------------------------------------------------------
// PreferencesController
//------------------------------------------------------------------------
PreferencesController::PreferencesController(ARADocumentController* controller)
: controller(controller)
{
    if (!controller)
        return;
}

//------------------------------------------------------------------------
PreferencesController::~PreferencesController() {}

//------------------------------------------------------------------------
VSTGUI::CView*
PreferencesController::verifyView(VSTGUI::CView* view,
                                  const VSTGUI::UIAttributes& attributes,
                                  const VSTGUI::IUIDescription* description)
{

    if (const auto* view_name = attributes.getAttributeValue("uidesc-label"))
    {
        if (*view_name == "PreferencesMenu")
        {
            this->options_menu = dynamic_cast<VSTGUI::COptionMenu*>(view);
            this->options_menu->addEntry("Dark");
            this->options_menu->addEntry("Lite");
            this->options_menu->registerControlListener(this);
        }
        else if (*view_name == "SchemeSwitch")
        {
            scheme_switch = dynamic_cast<VSTGUI::CControl*>(view);
        }
    }

    return view;
}

//------------------------------------------------------------------------
void PreferencesController::valueChanged(VSTGUI::CControl* pControl)
{
    if (pControl == this->options_menu)
    {
    }
    else if (pControl == scheme_switch)
    {
    }
}
//------------------------------------------------------------------------
} // namespace mam
