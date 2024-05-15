// Copyright(c) 2024 Max And Me.

#include "search_engine.h"
#include "meta_words_playback_region.h"

namespace mam::search_engine {
namespace {
//------------------------------------------------------------------------

//------------------------------------------------------------------------
} // namespace
//------------------------------------------------------------------------
struct SearchEngineCache
{
    struct RegionWord
    {
        RegionID region_id   = 0;
        WordIndex word_index = 0;
    };

    static SearchEngineCache& instance()
    {
        static SearchEngineCache cache;
        return cache;
    }

    SearchResults search_results;
    StringType search_word;
    RegionWord focused_word;
};

//------------------------------------------------------------------------
auto search(const StringType& search_word,
            const Regions& regions,
            MatchFunc&& match_func) -> const SearchResults&
{
    if (search_word == SearchEngineCache::instance().search_word)
        return SearchEngineCache::instance().search_results;

    SearchEngineCache::instance().search_word = search_word;
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
            if (match_func(word, search_word))
                indices.push_back(i);
        }

        if (!indices.empty())
            results.push_back({region.first, indices, std::nullopt});
    }

    SearchEngineCache::instance().search_results = std::move(results);

    if (!SearchEngineCache::instance().search_results.empty())
    {
        auto& first_result = SearchEngineCache::instance().search_results.at(0);

        constexpr auto DEFAULT_WORD_INDEX          = 0;
        first_result.focused_word                  = DEFAULT_WORD_INDEX;
        SearchEngineCache::instance().focused_word = {
            first_result.region_id, first_result.focused_word.value()};
    }

    return SearchEngineCache::instance().search_results;
}

//------------------------------------------------------------------------
auto next_occurence() -> SearchResults
{
    auto& focused_word   = SearchEngineCache::instance().focused_word;
    auto& search_results = SearchEngineCache::instance().search_results;

    SearchResults results;

    auto iter = std::find_if(
        search_results.begin(), search_results.end(), [&](const auto& result) {
            return result.region_id == focused_word.region_id;
        });

    if (iter == search_results.end())
        return results; // Should not happen, would be a serious error!

    focused_word.word_index++;
    if (focused_word.word_index < iter->indices.size())
    {
        // Still in the same region
        iter->focused_word = focused_word.word_index;
        results.push_back(*iter);
    }
    else
    {
        // Advance from one region to the next
        iter->focused_word.reset();
        results.push_back(*iter);

        iter = std::next(iter);
        if (iter == search_results.end())
            iter = search_results.begin();

        focused_word.word_index = 0;
        focused_word.region_id  = iter->region_id;
        iter->focused_word      = focused_word.word_index;
        results.push_back(*iter);
    }

    return results;
}

//------------------------------------------------------------------------
auto prev_occurence() -> SearchResults
{
    return {};
}

//------------------------------------------------------------------------
auto clear_results() -> void
{
    SearchEngineCache::instance().search_results.clear();
    SearchEngineCache::instance().search_word.clear();
}
//------------------------------------------------------------------------
} // namespace mam::search_engine