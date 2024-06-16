// Copyright(c) 2024 Max And Me.

#pragma once

#include "mam/meta_words/meta_word.h"
#include "wordify_types.h"
#include <functional>

namespace mam::task_managing {

//------------------------------------------------------------------------
using PathType      = StringType;
using ResultData    = meta_words::MetaWords;
using InputData     = PathType;
using FuncFinished  = std::function<void(const ResultData&)>;
using FuncTaskCount = std::function<void(size_t)>;

auto initialise(FuncTaskCount&& task_count_callback) -> bool;
auto terminate() -> void;
auto append_task(const InputData& input_data,
                 FuncFinished&& finished_callback) -> Id;
auto cancel_task(Id task_id) -> bool;
auto count_tasks() -> size_t;

//------------------------------------------------------------------------
} // namespace mam::task_managing
