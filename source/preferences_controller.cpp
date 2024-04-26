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
            this->options_menu->addEntry("Entry 1");
            this->options_menu->addEntry("Entry 2");
            this->options_menu->registerControlListener(this);
        }
        else if (*view_name == "SchemeSwitch")
        {
            const bool is_dark = controller->is_dark_scheme();
            scheme_switch      = dynamic_cast<VSTGUI::CControl*>(view);
            scheme_switch->setValueNormalized(is_dark ? 1. : 0.);
            scheme_switch->registerControlListener(this);
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
        const auto val = pControl->getValueNormalized();
        controller->set_dark_scheme(val > 0.);
    }
}

//------------------------------------------------------------------------
} // namespace mam
