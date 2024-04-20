//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "mam/meta_words/meta_word.h"
#include <string>

namespace mam::meta_words::serde {
//------------------------------------------------------------------------
using PersistentId = std::string;
using String       = std::string;

struct AudioSource
{
    PersistentId persistent_id;
    MetaWords words;
};

struct Archive
{
    using MetaWordsAudioSources = std::vector<AudioSource>;
    size_t version              = 1;
    MetaWordsAudioSources audio_sources;
};

//------------------------------------------------------------------------
auto serialize(const Archive& archive, String& s) -> bool;
auto deserialize(const String& s, Archive& archive) -> bool;

//------------------------------------------------------------------------
} // namespace mam::meta_words::serde