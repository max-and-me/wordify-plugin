// Copyright(c) 2024 Max And Me.


#pragma once

#include "mam/meta_words/meta_word.h"
#include <tuple>
#include <utility>
#include <vector>

namespace mam {

//------------------------------------------------------------------------
struct RegionWordData
{
    bool is_clipped_by_region = true;
    bool is_punctuation_mark  = false;
    meta_words::MetaWord word;
};

using RegionWordDataset = std::vector<RegionWordData>;

//------------------------------------------------------------------------
struct RegionData
{
    using Seconds = double;
    using String  = std::string;
    using Color   = std::tuple<float, float, float>;

    String name;
    Color color;
    Seconds project_offset{0.};
    Seconds project_time_start{0.};
    Seconds duration{0.};
    RegionWordDataset words;
};

//------------------------------------------------------------------------
} // namespace mam
