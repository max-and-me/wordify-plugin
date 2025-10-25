// Copyright(c) 2025 Max And Me.

#pragma once

#include "region_data.h"
#include <string>
#include <vector>

namespace mam::exporter {

enum class Format
{
    JSON,
    SRT, // SubRip
};

using StringType     = std::string;
using PathType       = StringType;
using RegionDataList = std::vector<RegionData>;

auto do_export(const PathType& output_path,
               const RegionDataList& regions,
               Format format) -> bool;

} // namespace mam::exporter