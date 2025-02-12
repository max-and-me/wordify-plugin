//------------------------------------------------------------------------
// Copyright(c) 2025 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "mam/meta_words/meta_word.h"
#include "wordify_types.h"

namespace mam::meta_words::serde {
//------------------------------------------------------------------------
using PersistentId = StringType;

struct AudioSource
{
    PersistentId persistent_id;
    MetaWords words;
};

struct Archive
{
    using AudioSources = std::vector<AudioSource>;

    size_t version = 1;
    AudioSources audio_sources;
};

//------------------------------------------------------------------------
auto serialize(const Archive& archive, StringType& s) -> bool;
auto deserialize(const StringType& s, Archive& archive) -> bool;

//------------------------------------------------------------------------
} // namespace mam::meta_words::serde
