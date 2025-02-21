//------------------------------------------------------------------------
// Copyright (c) 2023-present, WordifyOrg.
//------------------------------------------------------------------------

#pragma once

#include "mam/meta_words/meta_word.h"
#include "wordify_types.h"

namespace mam::meta_words::serde {

//------------------------------------------------------------------------
using PathType = StringType;

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
auto serialize(const Preferences& prefs, StringType& s) -> bool;
auto deserialize(const StringType& s, Preferences& prefs) -> bool;

auto write_to(const Preferences& prefs,
              const StringType& company_name,
              const StringType& plugin_name) -> bool;
auto read_from(const StringType& company_name,
               const StringType& plugin_name,
               Preferences& prefs) -> bool;

//------------------------------------------------------------------------
} // namespace mam::meta_words::serde