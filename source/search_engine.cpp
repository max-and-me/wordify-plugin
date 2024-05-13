// Copyright(c) 2024 Max And Me.

#include "search_engine.h"
#include "meta_words_playback_region.h"

namespace mam::search_engine {

//------------------------------------------------------------------------
struct SearchEngineCache
{
    using RegionWord = std::pair<RegionID, WordIndex>;

    static SearchEngineCache& instance()
    {
        static SearchEngineCache cache;
        return cache;
    }

    SearchResults search_results;
    StringType search_term;
    RegionWord selected_word;
};

//------------------------------------------------------------------------
auto search(const StringType& search_string,
            const Regions& regions,
            const MatchFunc& match_func) -> const SearchResults&
{
    if (search_string == SearchEngineCache::instance().search_term)
        return SearchEngineCache::instance().search_results;

    SearchEngineCache::instance().search_term = search_string;
    SearchResults results;
    for (const auto& region : regions)
    {
        auto regionPtr          = region.second;
        auto meta_words_data    = regionPtr->get_meta_words_data();
        auto meta_words_dataSet = meta_words_data.words;
        WordIndices indices;
        for (size_t i = 0; i < meta_words_dataSet.size(); i++)
        {
            const auto& word_data = meta_words_dataSet[i];
            if (word_data.is_clipped_by_region)
                continue;

            auto word = word_data.word.word;
            if (match_func(word, search_string))
                indices.push_back(i);
        }

        results.push_back({region.first, indices, std::nullopt});
    }

    SearchEngineCache::instance().search_results = std::move(results);
    // TODO
    // SearchEngineCache::instance().selected_word

    return SearchEngineCache::instance().search_results;
}

//------------------------------------------------------------------------
auto next_occurence() -> void {}

//------------------------------------------------------------------------
auto previous_occurence() -> void {}

//------------------------------------------------------------------------
auto clear_results() -> void
{
    SearchEngineCache::instance().search_results.clear();
    SearchEngineCache::instance().search_term.clear();
}
//------------------------------------------------------------------------
} // namespace mam::search_engine