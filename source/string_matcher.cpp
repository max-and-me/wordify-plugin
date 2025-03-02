//------------------------------------------------------------------------
// Copyright (c) 2023-present, WordifyOrg.
//------------------------------------------------------------------------

#include "string_matcher.h"
#include <algorithm>
#include <cctype>
#include <vector>

namespace mam {

namespace StringMatcher {

enum class FuzzyMatchStyle
{
    none,
    nearbyFuzzy,
    intermediateFuzzy
};

//------------------------------------------------------------------------
void prepareStrings(StringType& toMatch, StringType& string)
{
    std::transform(toMatch.begin(), toMatch.end(), toMatch.begin(), ::tolower);
    std::transform(string.begin(), string.end(), string.begin(), ::tolower);

    toMatch.erase(
        std::remove_if(
            toMatch.begin(), toMatch.end(),
            [](char c) { return std::ispunct(static_cast<unsigned char>(c)); }),
        toMatch.end());
}

//------------------------------------------------------------------------
bool isDirectMatch(StringType toMatch, StringType string)
{
    if (toMatch.length() != string.length())
        return false;

    return (toMatch == string);
}

//------------------------------------------------------------------------
bool isSubMatch(StringType toMatch, StringType string)
{
    return std::search(toMatch.begin(), toMatch.end(), string.begin(),
                       string.end()) != toMatch.end();
}

//------------------------------------------------------------------------
bool isFuzzyMatch(StringType toMatch, StringType string, FuzzyMatchStyle style)
{
    size_t x = toMatch.length();
    size_t y = string.length();


    std::vector<std::vector<int>> distanceVector(x + 1,
                                                 std::vector<int>(y + 1, 0));

    for (size_t i = 0; i <= x; ++i)
        distanceVector[i][0] = static_cast<int>(i);
    for (size_t j = 0; j <= y; ++j)
        distanceVector[0][j] =  static_cast<int>(j);

    for (size_t i = 1; i <= x; ++i)
    {
        for (size_t j = 1; j <= y; ++j)
        {
            if (toMatch[i - 1] == string[j - 1])
                distanceVector[i][j] = distanceVector[i - 1][j - 1];
            else
                distanceVector[i][j] =
                    std::min({distanceVector[i - 1][j],
                              distanceVector[i][j - 1],
                              distanceVector[i - 1][j - 1]}) +
                    1;
        }
    }

    int result = distanceVector[x][y];

    if (style == FuzzyMatchStyle::nearbyFuzzy)
        return result <= 1;
    else if (style == FuzzyMatchStyle::intermediateFuzzy)
        return result <= 2;

    return false;
}

//------------------------------------------------------------------------
bool isMatch(StringType toMatch, StringType string, MatchMethod method)
{
    prepareStrings(toMatch, string);
    if (method == MatchMethod::directMatch)
        return isDirectMatch(toMatch, string);
    else if (method == MatchMethod::subMatch)
        return isSubMatch(toMatch, string);
    else if (method == MatchMethod::nearbyFuzzyMatch)
        return isFuzzyMatch(toMatch, string, FuzzyMatchStyle::nearbyFuzzy);
    else if (method == MatchMethod::intermediateFuzzyMatch)
        return isFuzzyMatch(toMatch, string,
                            FuzzyMatchStyle::intermediateFuzzy);

    return false;
}

} // namespace StringMatcher
} // namespace mam
