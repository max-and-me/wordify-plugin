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

struct MetaWordsSerde
{
    PersistentId persistent_id;
    MetaWords words;
};

using MetaWordsSerdeDataset = std::vector<MetaWordsSerde>;

//------------------------------------------------------------------------
auto serialize(const MetaWordsSerdeDataset& meta_words_serde_dataset,
               String& s) -> bool;
auto deserialize(const String& s,
                 MetaWordsSerdeDataset& meta_words_serde_dataset) -> bool;

//------------------------------------------------------------------------
} // namespace mam::meta_words::serde