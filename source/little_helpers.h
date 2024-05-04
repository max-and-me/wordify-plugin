//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "fmt/format.h"
#include "vstgui/lib/ccolor.h"
#include <optional>
#include <string>

namespace mam {

//------------------------------------------------------------------------
// WaveFormView
//------------------------------------------------------------------------
template <typename T>
class BoundsCheck
{
public:
    //--------------------------------------------------------------------
    using Range = std::pair<T, T>;
    BoundsCheck(const Range& range)
    : lo(range.first)
    , hi(range.first + range.second)
    {
    }

    bool is_in(const T& value) const { return is_in(value, lo, hi); }

    //--------------------------------------------------------------------

private:
    const T lo{0};
    const T hi{0};

    bool is_in(const T& value, const T& lo_, const T& hi_) const
    {
        return !(value < lo_) && !(hi_ < value);
    }
};

//------------------------------------------------------------------------
template <typename T>
static auto make_color(const T& norm_r,
                       const T& norm_g,
                       const T& norm_b,
                       std::optional<T> opt_tint) -> const VSTGUI::CColor
{
    const auto tint = opt_tint.value_or(0.f);
    VSTGUI::CColor color_normal;
    color_normal.setNormRed(norm_r + (T(1.) - norm_r) * tint);
    color_normal.setNormGreen(norm_g + (T(1.) - norm_g) * tint);
    color_normal.setNormBlue(norm_b + (T(1.) - norm_b) * tint);
    return color_normal;
}

//------------------------------------------------------------------------
inline std::string trim(const std::string& source)
{
    std::string s(source);
    s.erase(0, s.find_first_not_of(" \n\r\t"));
    s.erase(s.find_last_not_of(" \n\r\t") + 1);
    return s;
}

//------------------------------------------------------------------------
using TimeDisplayString = std::string;
template <typename T>
auto to_time_display_string(T seconds) -> TimeDisplayString
{
    namespace chrono = std::chrono;

    chrono::seconds total(static_cast<size_t>(seconds));
    const auto h = chrono::duration_cast<chrono::hours>(total);
    const auto m = chrono::duration_cast<chrono::minutes>(total - h);
    const auto s = chrono::duration_cast<chrono::seconds>(total - h - m);

    // auto output =
    //     fmt::format("{:02}:{:02}:{:02}", h.count(), m.count(), s.count());

    auto output = fmt::format("{:02}:{:02}", m.count(), s.count());

    // Only if there are some hours we display them
    if (h.count() > 0.)
        output = fmt::format("{:02}:{}", h.count(), output);

    return output;
}

//------------------------------------------------------------------------

} // namespace mam