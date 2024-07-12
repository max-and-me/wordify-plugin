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
#define BEGIN_SUPPRESS_WARNINGS __pragma(warning(push, 0))
#define END_SUPPRESS_WARNINGS __pragma(warning(pop))
#elif defined(__clang__)
#define BEGIN_SUPPRESS_WARNINGS                                                 \
    _Pragma("clang diagnostic push")                                           \
        _Pragma("clang diagnostic ignored \"-Weverything\"")
#define END_SUPPRESS_WARNINGS _Pragma("clang diagnostic push")
#elif defined(__GNUC__) || defined(__GNUG__)
// clang-format off
#define BEGIN_SUPPRESS_WARNINGS
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wconversion-null\"") \
    _Pragma("GCC diagnostic ignored \"-Wcast-align\"") \
    _Pragma("GCC diagnostic ignored \"-Wconversion\"") \
    _Pragma("GCC diagnostic ignored \"-Wdeprecated\"") \
    _Pragma("GCC diagnostic ignored \"-Wdouble-promotion\"") \
    _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"") \
    _Pragma("GCC diagnostic ignored \"-Wempty-body\"") \
    _Pragma("GCC diagnostic ignored \"-Wextra\"") \
    _Pragma("GCC diagnostic ignored \"-Wformat\"") \
    _Pragma("GCC diagnostic ignored \"-Winvalid-offsetof\"") \
    _Pragma("GCC diagnostic ignored \"-Wignored-qualifiers\"") \
    _Pragma("GCC diagnostic ignored \"-Wmissing-braces\"") \
    _Pragma("GCC diagnostic ignored \"-Wmissing-field-initializers\"") \
    _Pragma("GCC diagnostic ignored \"-Wnarrowing\"") \
    _Pragma("GCC diagnostic ignored \"-Wnon-virtual-dtor\"") \
    _Pragma("GCC diagnostic ignored \"-Wold-style-cast\"") \
    _Pragma("GCC diagnostic ignored \"-Woverloaded-virtual\"") \
    _Pragma("GCC diagnostic ignored \"-Wpadded\"") \
    _Pragma("GCC diagnostic ignored \"-Wparentheses\"") \
    _Pragma("GCC diagnostic ignored \"-Wpedantic\"") \
    _Pragma("GCC diagnostic ignored \"-Wreturn-type\"") \
    _Pragma("GCC diagnostic ignored \"-Wrestrict\"") \
    _Pragma("GCC diagnostic ignored \"-Wshadow\"") \
    _Pragma("GCC diagnostic ignored \"-Wsign-compare\"") \
    _Pragma("GCC diagnostic ignored \"-Wsign-conversion\"") \
    _Pragma("GCC diagnostic ignored \"-Wswitch\"") \
    _Pragma("GCC diagnostic ignored \"-Wswitch-enum\"") \
    _Pragma("GCC diagnostic ignored \"-Wundef\"") \
    _Pragma("GCC diagnostic ignored \"-Wuninitialized\"") \
    _Pragma("GCC diagnostic ignored \"-Wunknown-pragmas\"") \
    _Pragma("GCC diagnostic ignored \"-Wunreachable-code\"") \
    _Pragma("GCC diagnostic ignored \"-Wunused-but-set-variable\"") \
    _Pragma("GCC diagnostic ignored \"-Wunused-function\"") \
    _Pragma("GCC diagnostic ignored \"-Wunused-label\"") \
    _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"") \
    _Pragma("GCC diagnostic ignored \"-Wunused-value\"") \
    _Pragma("GCC diagnostic ignored \"-Wunused-variable\"")
// clang-format on
#define END_SUPPRESS_WARNINGS _Pragma("gcc diagnostic push")
#endif
