// Copyright(c) 2024 Max And Me.

/* https://github.com/holoplot/shield.git
 * https://www.fluentcpp.com/2019/08/30/how-to-disable-a-warning-in-cpp/
 * https://learn.microsoft.com/de-de/cpp/build/reference/external-external-headers-diagnostics?view=msvc-170
 * https://stackoverflow.com/questions/28166565/detect-gcc-as-opposed-to-msvc-clang-with-macro
 * https://stackoverflow.com/questions/13826722/how-do-i-define-a-macro-with-multiple-pragmas-for-clang
 */

#pragma once

#if defined(_MSC_VER)
// the global warning level is now 0 here
#define BEGIN_SUPRESS_WARNINGS __pragma(warning(push, 0))
#define END_SUPRESS_WARNINGS __pragma(warning(pop))
#elif defined(__clang__)
#define BEGIN_SUPRESS_WARNINGS                                                 \
    _Pragma("clang diagnostic push")                                           \
        _Pragma("clang diagnostic ignored \"-Weverything\"")
#define END_SUPRESS_WARNINGS _Pragma("clang diagnostic push")
#elif defined(__GNUC__) || defined(__GNUG__)
#define BEGIN_SUPRESS_WARNINGS
#define END_SUPRESS_WARNINGS
#endif
