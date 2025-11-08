// Copyright (c) 2023-present, WordifyOrg.

#include "preferences_controller.h"
#include "ara_document_controller.h"
#include "exporter.h"
#include "version.h"
#include "warn_cpp/suppress_warnings.h"
#include <string>
BEGIN_SUPPRESS_WARNINGS
#include "public.sdk/source/vst/vstparameters.h"
#include "vstgui/lib/cfileselector.h"
#include "vstgui/lib/controls/coptionmenu.h"
#include "vstgui/uidescription/uiattributes.h"
END_SUPPRESS_WARNINGS

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
static auto collect_playback_regions(ARADocumentController& controller)
    -> const exporter::RegionDataList
{
    exporter::RegionDataList regions;
    controller.for_each_region_id([&](const Id id) {
        auto opt_region = controller.find_playback_region(id);
        auto region     = opt_region.value_or(nullptr);
        if (!region)
            return;

        regions.push_back(region->get_region_data());
    });

    return regions;
}

//------------------------------------------------------------------------
static auto export_file(const exporter::RegionDataList& list,
                        VSTGUI::CFrame& frame,
                        exporter::Format format,
                        const StringType& last_file_path) -> const StringType
{
    using StringType = std::string;
    StringType output_file_path;

    CNewFileSelector* selector =
        CNewFileSelector::create(&frame, CNewFileSelector::kSelectSaveFile);
    if (selector)
    {
        const auto format_info = exporter::getFormatInfo(format);
        const auto extension =
            CFileExtension(UTF8String(format_info.description),
                           UTF8String(format_info.extension));
        const UTF8String title = "Export " + format_info.description;
        selector->setTitle(title);
        selector->setAllowMultiFileSelection(false);
        selector->setDefaultExtension(extension);
        selector->setInitialDirectory(UTF8String(last_file_path));
        selector->run([&](CNewFileSelector* control) {
            if (control == nullptr)
                return;

            const auto canceled = control->getNumSelectedFiles() == 0;
            if (canceled)
                return;

            output_file_path = control->getSelectedFile(0);
            exporter::do_export(output_file_path, list, format);
        });
        selector->forget();
    }

    return output_file_path;
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
PreferencesController::~PreferencesController() {}

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
                auto item = new CCommandMenuItem({"Visit wordify.org..."});
                item->setActions(
                    [this](auto) { open_url(URL{"https://www.wordify.org"}); });
                options_menu->addEntry(item);

                auto export_submenu = new COptionMenu();
                options_menu->addEntry(export_submenu, "Export");

                item = new CCommandMenuItem({"JSON..."});
                item->setActions([this](auto) {
                    last_file_path =
                        export_file(collect_playback_regions(*controller),
                                    *(options_menu->getFrame()),
                                    exporter::Format::JSON, last_file_path);
                });
                export_submenu->addEntry(item);

                item = new CCommandMenuItem({"SubRip..."});
                item->setActions([this](auto) {
                    last_file_path =
                        export_file(collect_playback_regions(*controller),
                                    *(options_menu->getFrame()),
                                    exporter::Format::SRT, last_file_path);
                });
                export_submenu->addEntry(item);

                options_menu->addEntry(UTF8String("v") + VERSION_STR, -1,
                                       CMenuItem::kDisabled);
            }
        }
    }

    return view;
}

//------------------------------------------------------------------------
void PreferencesController::valueChanged(CControl* /* pControl */) {}

//------------------------------------------------------------------------
void PLUGIN_API PreferencesController::update(FUnknown* /*changedUnknown*/,
                                              Steinberg::int32 /*tag*/)
{
}

//------------------------------------------------------------------------
void PreferencesController::viewWillDelete(VSTGUI::CView* view) {}

//------------------------------------------------------------------------
} // namespace mam
