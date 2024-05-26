// Copyright(c) 2024 Max And Me.

#include "search_engine.h"
#include "meta_words_playback_region.h"

namespace mam::search_engine {
namespace {
//------------------------------------------------------------------------
auto collect_search_results(const StringType& search_word,
                            const Regions& regions,
                            MatchFunc&& match_func) -> const SearchResults
{
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

    return results;
}

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
    if (SearchEngineCache::instance().search_word == search_word)
        return SearchEngineCache::instance().search_results;

    SearchEngineCache::instance().search_word = search_word;
    SearchEngineCache::instance().search_results =
        collect_search_results(search_word, regions, std::move(match_func));

    // Focus the first word
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
    auto& focused_word   = SearchEngineCache::instance().focused_word;
    auto& search_results = SearchEngineCache::instance().search_results;

    SearchResults results;

    auto iter =
        std::find_if(search_results.rbegin(), search_results.rend(),
                     [&](const auto& result) {
                         return result.region_id == focused_word.region_id;
                     });

    if (iter == search_results.rend())
        return results; // Should not happen, would be a serious error!

    if (focused_word.word_index == 0)
    {
        // Advance from one region to the next
        iter->focused_word.reset();
        results.push_back(*iter);

        iter = std::next(iter);
        if (iter == search_results.rend())
            iter = search_results.rbegin();

        focused_word.word_index = iter->indices.size() - 1;
        focused_word.region_id  = iter->region_id;
        iter->focused_word      = focused_word.word_index;
        results.push_back(*iter);
    }
    else
    {
        focused_word.word_index--;

        // Still in the same region
        iter->focused_word = focused_word.word_index;
        results.push_back(*iter);
    }

    return results;
}

//------------------------------------------------------------------------
auto clear_results() -> SearchResults
{
    auto results = SearchEngineCache::instance().search_results;
    for (auto& result : results)
    {
        result.focused_word = std::nullopt;
        result.indices.clear();
    }

    SearchEngineCache::instance().search_results.clear();
    SearchEngineCache::instance().search_word.clear();

    return results;
}
//------------------------------------------------------------------------
} // namespace mam::search_engine