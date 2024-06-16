// Copyright(c) 2024 Max And Me.

#pragma once

#if __cplusplus >= 202002L
#include <span>
#elif defined(__GNUC__) || defined(__GNUG__)
#include <span>
#else
#include "gsl/span"
#endif

namespace nonstd {

//------------------------------------------------------------------------
#if __cplusplus >= 202002L
template <class T>
using span = std::span<T>;
#else
template <class T>
using span = gsl::span<T>;
#endif

//------------------------------------------------------------------------
} // namespace nonstd