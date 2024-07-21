// Copyright(c) 2024 Max And Me.

#pragma once

#include "eventpp/callbacklist.h"
#include "mam/meta_words/runner.h"
#include "wordify_types.h"
#include <functional>

namespace mam::task_managing {

//------------------------------------------------------------------------
using ResultData = meta_words::ExpectedMetaWords;
struct Expected
{
    bool was_canceled = false;
    ResultData data;
};

using PathType          = StringType;
using InputData         = PathType;
using FuncFinished      = std::function<void(const Expected&)>;
using TaskCountCallback = eventpp::CallbackList<void(const size_t)>;

auto append_task(const InputData& input_data, FuncFinished&& finished_callback)
    -> Id;
auto cancel_task(Id task_id) -> bool;
auto count_tasks() -> size_t;
auto get_task_count_callback() -> TaskCountCallback*;

//------------------------------------------------------------------------
} // namespace mam::task_managing
