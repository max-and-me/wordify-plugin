//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "preferences_serde.h"
#include "hao/special_folders/special_folders.h"
#include "nlohmann/json.hpp"
#include <filesystem>
#include <fstream>

namespace mam::meta_words::serde {
//------------------------------------------------------------------------
using json                                  = nlohmann::json;
static constexpr auto PREFERENCES_FILE_NAME = "preferences.json";

static constexpr auto PREFS_VERSION_KEY       = "preferences_version";
static constexpr auto COLOR_SCHEME_KEY        = "color_scheme";
static constexpr auto COLOR_SCHEME_LITE_VALUE = "light";
static constexpr auto COLOR_SCHEME_DARK_VALUE = "dark";
//------------------------------------------------------------------------
NLOHMANN_JSON_SERIALIZE_ENUM(ColorScheme,
                             {
                                 {Light, COLOR_SCHEME_LITE_VALUE},
                                 {Dark, COLOR_SCHEME_DARK_VALUE},
                             })

//------------------------------------------------------------------------
void to_json(json& j, const Preferences& prefs)
{
    j = json{{PREFS_VERSION_KEY, prefs.version},
             {COLOR_SCHEME_KEY, prefs.color_scheme}};
}

//------------------------------------------------------------------------
void from_json(const json& j, Preferences& prefs)
{
    j.at(PREFS_VERSION_KEY).get_to(prefs.version);
    j.at(COLOR_SCHEME_KEY).get_to(prefs.color_scheme);
}

//------------------------------------------------------------------------
auto serialize(const Preferences& prefs, String& s) -> bool
{
    json j = prefs;
    s      = j.dump();

    return true;
}
//------------------------------------------------------------------------
auto deserialize(const String& s, Preferences& prefs) -> bool
{
    if (s.empty())
        return false;

    json j = json::parse(s);
    prefs  = j.template get<Preferences>();

    return true;
}

//------------------------------------------------------------------------
auto write_to(const Preferences& prefs, const PathType& directory) -> bool
{
    const auto prefs_folder = hao::special_folders::get_preferences_folder();
    auto prefs_folder_path  = std::filesystem::path(prefs_folder);
    prefs_folder_path /= directory;

    if (!std::filesystem::exists(prefs_folder_path))
    {
        if (!std::filesystem::create_directories(prefs_folder_path))
            return false;
    }

    const auto prefs_file_path = prefs_folder_path / PREFERENCES_FILE_NAME;

    json j = prefs;
    std::ofstream file(prefs_file_path);
    file << j;

    return true;
}

//------------------------------------------------------------------------
auto read_from(const PathType& directory, Preferences& prefs) -> bool
{
    const auto prefs_folder = hao::special_folders::get_preferences_folder();
    auto prefs_folder_path  = std::filesystem::path(prefs_folder);
    prefs_folder_path /= directory;

    const auto prefs_file_path = prefs_folder_path / PREFERENCES_FILE_NAME;

    if (!std::filesystem::exists(prefs_file_path))
        return false;

    std::ifstream file(prefs_file_path);
    prefs = json::parse(file);

    return true;
}

//------------------------------------------------------------------------
} // namespace mam::meta_words::serde
