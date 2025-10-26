// Copyright(c) 2025 Max And Me.

#include "exporter.h"
#include "warn_cpp/suppress_warnings.h"
#include <chrono>
#include <fstream>
#include <sstream>
BEGIN_SUPPRESS_WARNINGS
#include "fmt/format.h"
END_SUPPRESS_WARNINGS

namespace mam::exporter {
namespace subrip {

template <typename T>
auto to_time_display_string(T seconds) -> StringType
{
    namespace chrono = std::chrono;

    chrono::seconds total(static_cast<size_t>(seconds));
    const auto h = chrono::duration_cast<chrono::hours>(total);
    const auto m = chrono::duration_cast<chrono::minutes>(total - h);
    const auto s = chrono::duration_cast<chrono::seconds>(total - h - m);
    const auto ms =
        chrono::duration_cast<chrono::milliseconds>(total - h - m - s);

    auto output = fmt::format("{:02}:{:02}:{:02},{:03}", h.count(), m.count(),
                              s.count(), ms.count());

    return output;
}

struct SubTitle
{
    StringType start_time;
    StringType end_time;
    StringType text;
};

using SubTitles = std::vector<SubTitle>;

// https://en.wikipedia.org/wiki/SubRip
constexpr auto kArrow = " --> ";

static auto do_export(const PathType& output_path, const SubTitles& sub_titles)
    -> bool
{
    using OFStream = std::ofstream;
    OFStream out_stream(output_path);

    size_t counter = 1;
    for (const auto& el : sub_titles)
    {
        out_stream << counter << std::endl;
        out_stream << el.start_time << kArrow << el.end_time << std::endl;
        out_stream << el.text << std::endl;
        out_stream << std::endl;

        counter++;
    }

    return true;
}
} // namespace subrip

static auto trim(StringType& str) -> StringType
{
    if (str.empty())
        return str;

    if (std::isspace(str.front()) != 0)
        str.erase(str.begin());
    if (std::isspace(str.back()))
        str.pop_back();

    return str;
}

static auto convert(const RegionDataList& regions,
                    subrip::SubTitles& sub_titles) -> void
{
    for (const auto& region : regions)
    {
        subrip::SubTitle sub_title;
        sub_title.start_time =
            subrip::to_time_display_string(region.project_time_start);
        sub_title.end_time = subrip::to_time_display_string(
            region.project_time_start + region.duration);
        for (const auto& word : region.words)
        {
            if (word.is_clipped_by_region)
                continue;

            sub_title.text += word.word.value + ' ';
        }
        sub_title.text = trim(sub_title.text);

        sub_titles.emplace_back(sub_title);
    }
}

auto do_export(const PathType& output_path,
               const RegionDataList& regions,
               Format format) -> bool
{
    switch (format)
    {
        case Format::SRT: {
            subrip::SubTitles sub_titles;
            sub_titles.reserve(regions.size());
            convert(regions, sub_titles);
            return subrip::do_export(output_path, sub_titles);
        }

        case Format::JSON: {
            // TODO: Put JSON stuff here
            return false;
        }

        default: {
            return false;
        }
    }
}

auto getFormatInfo(Format format) -> FormatInfo
{
    switch (format)
    {
        case Format::SRT:
            return {"SubRip", "srt"};
        case Format::JSON:
            return {"JSON", "json"};
        default:
            return {};
    }
}

} // namespace mam::exporter