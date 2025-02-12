//------------------------------------------------------------------------
// Copyright(c) 2025 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "wordify_types.h"

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

bool isMatch(StringType toMatch, StringType string, MatchMethod method);

}; // namespace StringMatcher

//------------------------------------------------------------------------
} // namespace mam
