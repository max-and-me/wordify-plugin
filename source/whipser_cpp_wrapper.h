// Copyright(c) 2024 Max And Me.

#pragma once

#include "wordify_types.h"

namespace mam::whisper_cpp {
//------------------------------------------------------------------------
using PathType = StringType;

auto get_worker_executable_path() -> PathType;
auto get_ggml_file_path() -> PathType;

//------------------------------------------------------------------------
} // namespace mam::whisper_cpp