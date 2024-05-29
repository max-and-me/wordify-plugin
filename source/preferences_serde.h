//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "mam/meta_words/meta_word.h"
#include <string>

namespace mam::meta_words::serde {
//------------------------------------------------------------------------
using String   = std::string;
using PathType = std::string;

enum ColorScheme
{
    Light,
    Dark
};

enum SmartSearch
{
    Off,
    On
};

struct Preferences
{
    size_t version = 1;
    ColorScheme color_scheme{Dark};
    SmartSearch smart_search{Off};
};

//------------------------------------------------------------------------
auto serialize(const Preferences& prefs, String& s) -> bool;
auto deserialize(const String& s, Preferences& prefs) -> bool;

auto write_to(const Preferences& prefs,
              const String& company_name,
              const String& plugin_name) -> bool;
auto read_from(const String& company_name,
               const String& plugin_name,
               Preferences& prefs) -> bool;

//------------------------------------------------------------------------
} // namespace mam::meta_words::serde