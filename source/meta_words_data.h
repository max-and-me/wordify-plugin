//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "mam/meta_words/meta_word.h"
#include <utility>
#include <tuple>

namespace mam {

//------------------------------------------------------------------------
struct MetaWordsData
{
    using Seconds = double;
    using String  = std::string;
    using Color = std::tuple<uint8_t, uint8_t, uint8_t>;

    String name;
    Color color;
    Seconds project_offset;
    Seconds project_time_start;
    meta_words::MetaWords words;
};

//------------------------------------------------------------------------
} // namespace mam