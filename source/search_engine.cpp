// Copyright(c) 2024 Max And Me.

#include "search_engine.h"
#include "meta_words_playback_region.h"
#include "wordify_types.h"

namespace mam {
namespace {
//------------------------------------------------------------------------
auto collect_search_results(const StringType& search_word,
                            const SearchEngine::Regions& regions,
                            SearchEngine::MatchFunc&& match_func)
    -> const SearchEngine::SearchResults
{
    SearchEngine::SearchResults results;

    for (const auto& region : regions)
    {
        auto region_data = region.second->get_region_data();

        SearchEngine::WordIndices indices;
        for (size_t i = 0; i < region_data.words.size(); i++)
        {
            const auto& word_data = region_data.words[i];
            if (word_data.is_clipped_by_region)
                continue;

            auto word = word_data.word.value;
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
        SearchEngine::RegionID region_id   = 0;
        SearchEngine::WordIndex word_index = 0;
    };

    static SearchEngineCache& instance()
    {
        static SearchEngineCache cache;
        return cache;
    }

    SearchEngine::SearchResults search_results;
    StringType search_word;
    RegionWord focused_word;
};

namespace detail {
//------------------------------------------------------------------------
auto search(const StringType& search_word,
            const SearchEngine::Regions& regions,
            SearchEngine::MatchFunc&& match_func)
    -> const SearchEngine::SearchResults&
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
auto next_occurence() -> SearchEngine::SearchResults
{
    auto& focused_word   = SearchEngineCache::instance().focused_word;
    auto& search_results = SearchEngineCache::instance().search_results;

    SearchEngine::SearchResults results;

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
auto prev_occurence() -> SearchEngine::SearchResults
{
    auto& focused_word   = SearchEngineCache::instance().focused_word;
    auto& search_results = SearchEngineCache::instance().search_results;

    SearchEngine::SearchResults results;

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
auto clear_results() -> SearchEngine::SearchResults
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
} // namespace detail

//------------------------------------------------------------------------
// SearchEngine
//------------------------------------------------------------------------
auto SearchEngine::search(const StringType& search_word,
                          MatchFunc&& match_func) -> void
{
    if (!get_regions)
        return;
    
    this->clear_results();

    const auto& results =
        detail::search(search_word, get_regions(), std::move(match_func));

    callback(results);
}

//------------------------------------------------------------------------
auto SearchEngine::research(MatchFunc&& match_func) -> void
{
    const auto w = SearchEngineCache::instance().search_word;
    clear_results();
    return search(w, std::move(match_func));
}

//------------------------------------------------------------------------
auto SearchEngine::next_occurence() -> void
{
    const auto results = mam::detail::next_occurence();
    callback(results);
}

//------------------------------------------------------------------------
auto SearchEngine::prev_occurence() -> void
{
    const auto results = mam::detail::prev_occurence();
    callback(results);
}

//------------------------------------------------------------------------
auto SearchEngine::clear_results() -> void
{
    const auto results = mam::detail::clear_results();
    callback(results);
}

//------------------------------------------------------------------------
auto SearchEngine::current_search_word() -> StringType
{
    return SearchEngineCache::instance().search_word;
}

//------------------------------------------------------------------------
auto SearchEngine::get_callback() -> SearchEngineCallback&
{
    return callback;
}

//------------------------------------------------------------------------
} // namespace mam
