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
    struct Color
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    String name;
    Color color;
    Seconds project_offset;
    meta_words::MetaWords words;
};

//------------------------------------------------------------------------
} // namespace mam