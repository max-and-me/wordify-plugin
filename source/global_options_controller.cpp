//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "global_options_controller.h"
#include "vstgui/lib/controls/coptionmenu.h"
#include "vstgui/uidescription/uiattributes.h"

namespace mam {

//------------------------------------------------------------------------
using namespace VSTGUI;

//------------------------------------------------------------------------
// GlobalOptionsController
//------------------------------------------------------------------------
GlobalOptionsController::GlobalOptionsController(
    ARADocumentController* controller)
: controller(controller)
{
    if (!controller)
        return;
}

//------------------------------------------------------------------------
GlobalOptionsController::~GlobalOptionsController() {}

//------------------------------------------------------------------------
VSTGUI::CView*
GlobalOptionsController::verifyView(VSTGUI::CView* view,
                                    const VSTGUI::UIAttributes& attributes,
                                    const VSTGUI::IUIDescription* description)
{

    if (const auto* view_name = attributes.getAttributeValue("uidesc-label"))
    {
        if (*view_name == "GlobalOptionsMenu")
        {
            this->options_menu = dynamic_cast<VSTGUI::COptionMenu*>(view);
            this->options_menu->addEntry("Dark");
            this->options_menu->addEntry("Lite");
        }
    }

    return view;
}

//------------------------------------------------------------------------
} // namespace mam
