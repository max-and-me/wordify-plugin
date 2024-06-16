// Copyright(c) 2024 Max And Me.

#include "preferences_controller.h"
#include "ara_document_controller.h"
#include "version.h"
#include "wordify_types.h"
#include <string>
BEGIN_SUPRESS_WARNINGS
#include "public.sdk/source/vst/vstparameters.h"
#include "vstgui/lib/controls/coptionmenu.h"
#include "vstgui/uidescription/uiattributes.h"
END_SUPRESS_WARNINGS

#ifdef WIN32
#include <windows.h>

#include "utf_8_everywhere/convert.h"
#include <shellapi.h>
#endif

namespace mam {
using namespace VSTGUI;

//------------------------------------------------------------------------
using URL = const struct
{
    StringType value;
};

auto open_url(URL& url) -> void
{
#if defined(WIN32)
    std::wstring wurl = utf_8_everywhere::convert(url.value);
    ShellExecuteW(0, L"open", wurl.c_str(), NULL, NULL, SW_SHOWDEFAULT);
#elif __APPLE__
    system((std::string("open ") + url.value).c_str());
#elif __linux__
    system((std::string("xdg-open ") + url.value).c_str());
#endif
}

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
                                         const IUIDescription* /*description*/)
{

    if (const auto* view_name = attributes.getAttributeValue("uidesc-label"))
    {
        if (*view_name == "PreferencesMenu")
        {
            options_menu = dynamic_cast<COptionMenu*>(view);
            if (options_menu)
            {
                options_menu->addEntry("Visit wordify.org ...");
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
void PreferencesController::valueChanged(CControl* pControl)
{
    if (pControl == options_menu)
    {
        open_url(URL{"https://www.wordify.org"});
    }
}

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
