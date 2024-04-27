//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "preferences_controller.h"
#include "public.sdk/source/vst/vstparameters.h"
#include "vstgui/lib/controls/coptionmenu.h"
#include "vstgui/uidescription/uiattributes.h"

namespace mam {

//------------------------------------------------------------------------
using namespace VSTGUI;

//------------------------------------------------------------------------
// PreferencesController
//------------------------------------------------------------------------
PreferencesController::PreferencesController(ARADocumentController* controller,
                                             Steinberg::Vst::Parameter* param)
: controller(controller)
, param(param)
{
    if (!controller)
        return;

    if (param)
        param->addDependent(this);
}

//------------------------------------------------------------------------
PreferencesController::~PreferencesController()
{
    if (param)
        param->removeDependent(this);
}

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
            scheme_switch = dynamic_cast<VSTGUI::CControl*>(view);
            scheme_switch->setValueNormalized(param->getNormalized());
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
        param->setNormalized(val);
    }
}

//------------------------------------------------------------------------
void PLUGIN_API PreferencesController::update(FUnknown* changedUnknown,
                                              Steinberg::int32 tag)
{
    if (auto* tmp_param =
            Steinberg::FCast<Steinberg::Vst::Parameter>(changedUnknown))
    {
        if (param->getInfo().id == tmp_param->getInfo().id)
        {
            scheme_switch->setValueNormalized(param->getNormalized());
        }
    }
}

//------------------------------------------------------------------------
} // namespace mam
