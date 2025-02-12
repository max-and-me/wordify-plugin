// Copyright(c) 2025 Max And Me.

#pragma once

#include "meta_words_playback_region.h"
#include "warn_cpp/suppress_warnings.h"
#include "wordify_types.h"
#include <functional>
#include <map>
#include <optional>
#include <vector>
BEGIN_SUPPRESS_WARNINGS
#include "eventpp/callbacklist.h"
END_SUPPRESS_WARNINGS

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
