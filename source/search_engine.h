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

struct RegionWord
{
    RegionID region_id   = 0;
    WordIndex word_index = 0;
};

struct SearchResult
{
    RegionID region_id = 0;
    WordIndices indices;
    OptWord focused_word;
};

using SearchResults = std::vector<SearchResult>;
using MatchFunc =
    std::function<bool(const StringType& s0, const StringType& s1)>;

auto search(const StringType& search_word,
            const Regions& regions,
            MatchFunc&& match_func) -> const SearchResults&;

auto research(const Regions& regions,
              MatchFunc&& match_func) -> const SearchResults&;

auto next_occurence() -> SearchResults;
auto prev_occurence() -> SearchResults;
auto clear_results() -> SearchResults;

//------------------------------------------------------------------------
} // namespace mam::search_engine