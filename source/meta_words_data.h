//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "mam/meta_words/meta_word.h"
#include <utility>

namespace mam {

//------------------------------------------------------------------------
struct MetaWordsData
{
    using Seconds = double;
    using String = std::string;

    String name;
    Seconds project_offset;
    meta_words::MetaWords words;
};

//------------------------------------------------------------------------
} // namespace mam