//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "mam/meta_words/meta_word.h"
#include "public.sdk/source/vst/vstparameters.h"
#include <functional>

namespace mam::analysing {

//------------------------------------------------------------------------
using FuncFinished    = std::function<void(meta_words::MetaWords)>;
using FuncProgress    = std::function<void(double)>;
using PathType        = std::string;
using TaskId          = size_t;
using VstParameterPtr = Steinberg::IPtr<Steinberg::Vst::Parameter>;

auto push_task(const PathType file,
               FuncFinished&& finished_func,
               FuncProgress&& progress_func) -> TaskId;
auto cancel_task(TaskId task_id) -> bool;
auto task_count_param() -> VstParameterPtr;

//------------------------------------------------------------------------
} // namespace mam::analysing
