// Copyright (c) 2023-present, WordifyOrg.

#include "preferences_controller.h"
#include "ara_document_controller.h"
#include "exporter.h"
#include "version.h"
#include "warn_cpp/suppress_warnings.h"
#include <map>
#include <string>
BEGIN_SUPPRESS_WARNINGS
#include "public.sdk/source/vst/vstparameters.h"
#include "vstgui/lib/cfileselector.h"
#include "vstgui/lib/controls/coptionmenu.h"
#include "vstgui/uidescription/uiattributes.h"
END_SUPPRESS_WARNINGS

#include <fstream>
#include <iostream>

#ifdef WIN32
#include <windows.h>

#include "utf_8_everywhere/convert.h"
#include <shellapi.h>
#endif

namespace mam {
using namespace VSTGUI;

//------------------------------------------------------------------------
enum MenuEntryIndex
{
    VISIT = 0,
    EXPORT_TEXT,
    EXPORT_SRT
};

using MenuEntries                     = std::map<MenuEntryIndex, StringType>;
static const MenuEntries kMenuEntries = {
    {{VISIT, "Visit wordify.org ..."},
     {EXPORT_TEXT, "Export as Text ..."},
     {EXPORT_SRT, "Export as SubRip ..."}}};

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
static auto collectPlaybackRegions(ARADocumentController& controller)
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
static auto selectFilePath(VSTGUI::CFrame& frame) -> const StringType
{
    using StringType = std::string;
    StringType output_file_path;

    CNewFileSelector* selector =
        CNewFileSelector::create(&frame, CNewFileSelector::kSelectSaveFile);
    if (selector)
    {
        selector->setTitle("Save SubRip File");
        selector->setAllowMultiFileSelection(false);
        selector->setDefaultExtension(CFileExtension("SubRip", "srt"));
        selector->run([&](CNewFileSelector* control) {
            if (control == nullptr)
                return;

            const auto canceled = control->getNumSelectedFiles() == 0;
            if (canceled)
                return;

            output_file_path = control->getSelectedFile(0);
        });
        selector->forget();
    }

    return output_file_path;
}

//------------------------------------------------------------------------
static auto export_subrip(ARADocumentController& controller,
                          VSTGUI::CFrame& frame)
{
    using StringType            = std::string;
    StringType output_file_path = selectFilePath(frame);
    if (output_file_path.empty())
        return;

    exporter::do_export(output_file_path, collectPlaybackRegions(controller),
                        exporter::Format::SRT);
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
                for (const auto& el : kMenuEntries)
                {
                    options_menu->addEntry(UTF8String(el.second), el.first);
                }

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
        if (options_menu->getLastResult() == VISIT)
        {
            open_url(URL{"https://www.wordify.org"});
        }
        else if (options_menu->getLastResult() == EXPORT_TEXT)
        {
            export_text();
        }
        else if (options_menu->getLastResult() == EXPORT_SRT)
        {
            if (!controller || !options_menu->getFrame())
                return;

            export_subrip(*controller, *(options_menu->getFrame()));
        }
    }
}

//------------------------------------------------------------------------
void PreferencesController::export_text()
{
    nlohmann::json transcript;

    CNewFileSelector* selector = CNewFileSelector::create(
        options_menu->getFrame(), CNewFileSelector::kSelectSaveFile);
    if (selector)
    {
        selector->setTitle("Save JSON File");
        selector->setAllowMultiFileSelection(false);
        selector->setDefaultExtension(CFileExtension("JSON", "json"));
        selector->run([&](CNewFileSelector* control) {
            if (control == nullptr)
                return;

            auto size = control->getNumSelectedFiles();
            if (size == 0)
            {
                std::cerr << "No file selected!" << std::endl;
                return;
            }

            std::string filePath = control->getSelectedFile(0);
            if (!filePath.empty())
            {
                controller->for_each_region_id([&](const Id id) {
                    add_transcript_to_json(id, transcript);
                });

                std::ofstream file(filePath);
                if (file.is_open())
                {
                    file << std::setw(4) << transcript << std::endl;
                    file.close();
                    std::cout << "JSON file saved to: " << filePath
                              << std::endl;
                }
                else
                {
                    std::cerr << "Unable to open file for writing!"
                              << std::endl;
                }
            }
        });
        selector->forget();
    }
}

//------------------------------------------------------------------------
void PreferencesController::add_transcript_to_json(const Id region_id,
                                                   nlohmann::json& transcript)
{
    if (!controller)
        return;

    auto opt_region = controller->find_playback_region(region_id);
    auto region     = opt_region.value_or(nullptr);
    if (!region)
        return;

    const auto words_data = region->get_region_data();

    StringType speaker    = words_data.name;
    StringType start_time = std::to_string(words_data.project_time_start);
    StringType duration   = std::to_string(words_data.duration);

    const auto& words = words_data.words;
    StringType spoken_text;
    for (const auto& word : words)
        spoken_text += word.word.value + " ";

    nlohmann::json speaker_data = {{"speaker", speaker},
                                   {"start_time", start_time},
                                   {"duration", duration},
                                   {"spoken_text", spoken_text}};

    transcript["transcript"].push_back(speaker_data);
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
