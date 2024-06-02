// Copyright(c) 2024 Max And Me.

#pragma once

#include "eventpp/callbacklist.h"
#include "meta_words_playback_region.h"
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace mam::meta_words {
class PlaybackRegion;
}

namespace mam {

//------------------------------------------------------------------------
// SearchEngine
//------------------------------------------------------------------------
class SearchEngine
{
public:
    //--------------------------------------------------------------------
    using StringType  = std::string;
    using WordIndex   = size_t;
    using WordIndices = std::vector<WordIndex>;
    using RegionID    = size_t;
    using OptWord     = std::optional<WordIndex>;
    using Regions     = std::map<Id, meta_words::PlaybackRegion*>;

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
    using SearchEngineCallback =
        eventpp::CallbackList<void(const SearchResults&)>;

    static SearchEngine& instance()
    {
        static SearchEngine cache;
        return cache;
    }

    auto search(const StringType& search_word, MatchFunc&& match_func) -> void;
    auto research(MatchFunc&& match_func) -> void;
    auto next_occurence() -> void;
    auto prev_occurence() -> void;
    auto clear_results() -> void;
    auto current_search_word() -> StringType;
    auto get_callback() -> SearchEngineCallback&;

    using FuncRegions = std::function<const Regions()>;
    FuncRegions get_regions;

    //--------------------------------------------------------------------
private:
    SearchEngineCallback callback;
};

//------------------------------------------------------------------------
} // namespace mam
