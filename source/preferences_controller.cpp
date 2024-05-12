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
PreferencesController::PreferencesController(
    ARADocumentController* controller,
    Steinberg::Vst::Parameter* color_scheme_param)
: controller(controller)
, color_scheme_param(color_scheme_param)
{
    if (!controller)
        return;

    if (color_scheme_param)
        color_scheme_param->addDependent(this);
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

    if (scheme_switch)
    {
        scheme_switch->unregisterControlListener(this);
        scheme_switch->unregisterViewListener(this);
        scheme_switch = nullptr;
    }

    if (color_scheme_param)
        color_scheme_param->removeDependent(this);
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
                auto* sub_menu = new COptionMenu();
                sub_menu->addEntry("Show direct matches");
                sub_menu->addEntry("Show sub matches");
                sub_menu->addEntry("Show nearby fuzzy matches");
                sub_menu->addEntry("Show intermediate fuzzy matches");
                options_menu->addEntry(sub_menu, "Search Settings");

                options_menu->addEntry(UTF8String("v") + VERSION_STR, -1,
                                       CMenuItem::kDisabled);
                options_menu->registerControlListener(this);
                options_menu->registerViewListener(this);
            }
        }
        else if (*view_name == "SchemeSwitch")
        {
            if (scheme_switch = dynamic_cast<CControl*>(view))
            {
                if (color_scheme_param)
                {
                    const auto val = color_scheme_param->getNormalized();
                    scheme_switch->setValueNormalized(val);
                    scheme_switch->registerControlListener(this);
                    scheme_switch->registerViewListener(this);
                }
            }
        }
    }

    return view;
}

//------------------------------------------------------------------------
void PreferencesController::valueChanged(CControl* pControl)
{
    if (pControl == options_menu)
    {
        int result;
        auto sub = options_menu->getLastItemMenu(result);
        if (result == 0)
            controller->selectStringMatchMethod(
                StringMatcher::MatchMethod::directMatch);
        else if (result == 1)
            controller->selectStringMatchMethod(
                StringMatcher::MatchMethod::subMatch);
        else if (result == 2)
            controller->selectStringMatchMethod(
                StringMatcher::MatchMethod::nearbyFuzzyMatch);
        else if (result == 3)
            controller->selectStringMatchMethod(
                StringMatcher::MatchMethod::intermediateFuzzyMatch);
    }
    else if (pControl == scheme_switch)
    {
        if (!color_scheme_param)
            return;

        const auto val = pControl->getValueNormalized();
        color_scheme_param->setNormalized(val);
    }
}

//------------------------------------------------------------------------
void PLUGIN_API PreferencesController::update(FUnknown* changedUnknown,
                                              Steinberg::int32 tag)
{
    if (!color_scheme_param)
        return;

    if (!scheme_switch)
        return;

    if (const auto* tmp_param =
            Steinberg::FCast<Steinberg::Vst::Parameter>(changedUnknown))
    {
        if (color_scheme_param->getInfo().id == tmp_param->getInfo().id)
        {
            scheme_switch->setValueNormalized(
                color_scheme_param->getNormalized());
        }
    }
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
    else if (view == scheme_switch)
    {
        scheme_switch->unregisterControlListener(this);
        scheme_switch->unregisterViewListener(this);
        scheme_switch = nullptr;
    }
}

//------------------------------------------------------------------------
} // namespace mam
