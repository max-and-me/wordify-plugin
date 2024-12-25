// Copyright(c) 2024 Max And Me.

#include "preferences_controller.h"
#include "ara_document_controller.h"
#include "version.h"
#include "warn_cpp/suppress_warnings.h"
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
                options_menu->addEntry("Export as Text ...");
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
        if (options_menu->getLastResult() == 0)
        {
            open_url(URL{"https://www.wordify.org"});
        }
        else if (options_menu->getLastResult() == 1)
        {
            export_text();
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
