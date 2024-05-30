// Copyright(c) 2024 Max And Me.

#include "preferences_controller.h"
#include "ara_document_controller.h"
#include "public.sdk/source/vst/vstparameters.h"
#include "version.h"
#include "vstgui/lib/controls/coptionmenu.h"
#include "vstgui/uidescription/uiattributes.h"

namespace mam {
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
PreferencesController::~PreferencesController()
{
    if (options_menu)
    {
        options_menu->unregisterControlListener(this);
        options_menu->unregisterViewListener(this);
        options_menu = nullptr;
    }
}

//------------------------------------------------------------------------
CView* PreferencesController::verifyView(CView* view,
                                         const UIAttributes& attributes,
                                         const IUIDescription* description)
{

    if (const auto* view_name = attributes.getAttributeValue("uidesc-label"))
    {
        if (*view_name == "PreferencesMenu")
        {
            if (options_menu = dynamic_cast<COptionMenu*>(view))
            {
                options_menu->addEntry("Visit wordify.org ...");
                options_menu->addEntry("Check for updates ...");
                options_menu->addEntry(UTF8String("v") + VERSION_STR, -1,
                                       CMenuItem::kDisabled);
                options_menu->registerControlListener(this);
                options_menu->registerViewListener(this);
            }
        }
    }

    return view;
}

//------------------------------------------------------------------------
void PreferencesController::valueChanged(CControl* /*pControl*/) {}

//------------------------------------------------------------------------
void PLUGIN_API PreferencesController::update(FUnknown* /*changedUnknown*/,
                                              Steinberg::int32 /*tag*/)
{
}

//------------------------------------------------------------------------
void PreferencesController::viewWillDelete(VSTGUI::CView* view)
{
    if (view == options_menu)
    {
        options_menu->unregisterControlListener(this);
        options_menu->unregisterViewListener(this);
        options_menu = nullptr;
    }
}

//------------------------------------------------------------------------
} // namespace mam
