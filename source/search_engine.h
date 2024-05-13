// Copyright(c) 2024 Max And Me.

#pragma once

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace mam::meta_words {
class PlaybackRegion;
}

namespace mam::search_engine {
//------------------------------------------------------------------------
using StringType  = std::string;
using WordIndex   = size_t;
using WordIndices = std::vector<WordIndex>;
using RegionID    = size_t;
using OptWord     = std::optional<WordIndex>;
using Regions     = std::map<RegionID, meta_words::PlaybackRegion*>;

struct Result
{
    RegionID regio_id = 0;
    WordIndices indices;
    OptWord selected_word;
};

using SearchResults = std::vector<Result>;
using MatchFunc =
    std::function<bool(const StringType& s0, const StringType& s1)>;

auto search(const StringType& search_string,
            const Regions& regions,
            const MatchFunc& match_func) -> const SearchResults&;
auto next_occurence() -> void;
auto previous_occurence() -> void;
auto clear_results() -> void;

//------------------------------------------------------------------------
} // namespace mam::search_engine