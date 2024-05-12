//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include <string>

namespace mam {

//------------------------------------------------------------------------
// StringMatcher
//------------------------------------------------------------------------
namespace StringMatcher {
enum class MatchMethod
{
    directMatch,
    subMatch,
    nearbyFuzzyMatch,
    intermediateFuzzyMatch

};

bool isMatch(std::string toMatch, std::string string, MatchMethod method);

}; // namespace StringMatcher

//------------------------------------------------------------------------
} // namespace mam
