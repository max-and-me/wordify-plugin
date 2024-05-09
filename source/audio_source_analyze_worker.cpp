//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "audio_source_analyze_worker.h"
#include "base/source/timer.h"
#include "hao/special_folders/special_folders.h"
#include "mam/meta_words/runner.h"
#include "meta_words_data.h"
#include "parameter_ids.h"
#include "vstgpt_defines.h"
#include "whipser_cpp_wrapper.h"
#include <algorithm>
#include <atomic>
#include <filesystem>
#include <functional>
#include <future>
#include <string>
#include <utility>
#include <vector>

namespace mam::analysing {
namespace {

//------------------------------------------------------------------------
using FutureResult = std::future<meta_words::MetaWords>;
using FuncCancel   = std::function<bool()>;
using FuncProgress = std::function<void(double)>;

//------------------------------------------------------------------------
auto run_sync(const meta_words::Command& cmd,
              FuncProgress&& progress_func,
              FuncCancel&& cancel_func) -> meta_words::MetaWords
{
    return (run(cmd, std::move(progress_func), std::move(cancel_func)));
}

//------------------------------------------------------------------------
auto create_whisper_cmd(const meta_words::PathType& file_path)
    -> const meta_words::Command
{
    using Options    = const meta_words::Options;
    using OneValArgs = const meta_words::OneValArgs;
    using Command    = const meta_words::Command;

    //  The whisper.cpp library takes the audio file and writes the result
    //  of its analysis into a CSV file. The file is named like the audio
    //  file and by prepending ".csv" e.g. my_speech.wav ->
    //  my_speech.wav.csv
    Options options = {"-ocsv" /* output result in a CSV file */,
                       "-sow" /* split on word rather than on token */};

    OneValArgs one_val_args = {
        // model file resp. binary
        {"-m", whisper_cpp::get_ggml_file_path()},
        // audio file to analyse
        {"-f", file_path},
        // maximum segment length in characters: "1" mains one word
        {"-ml", "1"},
        // auto language detection
        {"-l", "auto"}};

    Command cmd{whisper_cpp::get_worker_executable_path(), options,
                one_val_args};

    return cmd;
}

//------------------------------------------------------------------------
auto process_audio_with_meta_words(const meta_words::PathType& file_path,
                                   FuncProgress&& progress_func,
                                   FuncCancel&& cancel_func)
    -> meta_words::MetaWords
{
    const auto cmd = create_whisper_cmd(file_path);
    return run_sync(cmd, std::move(progress_func), std::move(cancel_func));
};

//------------------------------------------------------------------------
} // namespace

//------------------------------------------------------------------------
// Analyzer
//------------------------------------------------------------------------
template <typename TInput, typename TOutput>
class AnalyzeWorker
{
public:
    //--------------------------------------------------------------------
    using Self = AnalyzeWorker;

    using FuncDoTask =
        std::function<TOutput(const TInput&, FuncProgress&&, FuncCancel&&)>;
    using FuncCancel = std::function<bool()>;

    struct Task
    {
        TInput input_data;
        FuncFinished finished_func;
        FuncProgress progress_func;
        FuncDoTask do_task_func;
    };

    using TaskId        = size_t;
    using FutureResult  = std::future<TOutput>;
    using ScheduledTask = std::pair<TaskId, Task>;
    using TaskList      = std::vector<ScheduledTask>;

    static Self& instance()
    {
        static Self the_worker;
        return the_worker;
    }

    auto push_task(const Task&& task) -> TaskId
    {
        ScheduledTask waiting_task{++task_id, task};
        task_list.push_back(waiting_task);
        notify_observers();

        if (!timer)
        {
            timer = Steinberg::owned(Steinberg::Timer::create(
                Steinberg::newTimerCallback(
                    [this](Steinberg::Timer* timer) { this->work(); }),
                1.));

            next(future_result);
        }

        return task_id;
    }

    auto cancel_task(TaskId task_id) -> bool
    {
        if (task_list.empty())
            return false;

        auto& first_task = task_list.at(0);
        if (first_task.first == task_id)
            is_canceled = true;

        auto iter = std::remove_if(task_list.begin(), task_list.end(),
                                   [](auto task) -> bool { return true; });

        task_list.erase(iter, task_list.end());
        notify_observers();

        return true;
    }

    auto task_count() const -> size_t { return task_list.size(); }
    auto get_task_count_parmeter() -> VstParameterPtr
    {
        return task_count_param;
    }

    //--------------------------------------------------------------------
private:
    static size_t task_id;
    std::atomic_bool is_canceled       = false;
    std::atomic<double> progress_value = 0.;
    FutureResult future_result;
    TaskList task_list;
    Steinberg::IPtr<Steinberg::Timer> timer;
    Steinberg::IPtr<Steinberg::Vst::Parameter> task_count_param;

    AnalyzeWorker()
    {

        Steinberg::Vst::ParameterInfo paramInfo;
        paramInfo.id        = ParamIdAnalyzeTaskCount;
        paramInfo.stepCount = 100;
        task_count_param    = Steinberg::owned(
            new Steinberg::Vst::RangeParameter(paramInfo, 0, 100));
    }

    void notify_observers()
    {
        if (task_count_param)
        {
            const auto norm = task_count_param->toNormalized(task_list.size());
            task_count_param->setNormalized(norm);
        }
    }

    void work()
    {
        if (future_result.wait_for(std::chrono::seconds(0)) ==
            std::future_status::ready)
        {
            is_canceled = false;
            if (!task_list.empty())
            {
                auto& task      = task_list.at(0);
                auto meta_words = future_result.get();
                if (task.second.finished_func)
                    task.second.finished_func(meta_words);

                task_list.erase(task_list.begin());
                notify_observers();
            }

            if (task_list.empty())
            {
                timer = nullptr;
            }
            else
            {
                if (!next(future_result))
                {
                    timer = nullptr;
                }
            }
        }

        if (!task_list.empty())
        {
            task_list.at(0).second.progress_func(progress_value.load());
        }
    }

    auto next(FutureResult& result_future) -> bool
    {
        if (task_list.empty())
            return false;

        auto task = task_list.at(0);

        result_future = std::async([this, task]() {
            FuncProgress progress_func = [&](auto val) {
                this->progress_value = val;
            };

            FuncCancel cancel_func = [&]() { return is_canceled.load(); };

            return task.second.do_task_func(task.second.input_data,
                                            std::move(progress_func),
                                            std::move(cancel_func));
        });

        return true;
    }
};

//------------------------------------------------------------------------
using WorkAnalyzeWorker = AnalyzeWorker<PathType, meta_words::MetaWords>;
template <>
size_t WorkAnalyzeWorker::task_id = 0;

//------------------------------------------------------------------------
auto push_task(const PathType file,
               FuncFinished&& finished_func,
               FuncProgress&& progress_func) -> TaskId
{
    WorkAnalyzeWorker::Task task;
    task.input_data    = file;
    task.finished_func = std::move(finished_func);
    task.progress_func = std::move(progress_func);
    task.do_task_func  = [](const meta_words::PathType& file_path,
                           FuncProgress&& progress_func,
                           FuncCancel&& cancel_func) {
        return process_audio_with_meta_words(
            file_path, std::move(progress_func), std::move(cancel_func));
    };
    return WorkAnalyzeWorker::instance().push_task(std::move(task));
}

//------------------------------------------------------------------------
auto cancel_task(TaskId task_id) -> bool
{
    return WorkAnalyzeWorker::instance().cancel_task(task_id);
}

//------------------------------------------------------------------------
auto count_tasks() -> size_t
{
    return WorkAnalyzeWorker::instance().task_count();
}

//------------------------------------------------------------------------
auto task_count_param() -> VstParameterPtr
{
    return WorkAnalyzeWorker::instance().get_task_count_parmeter();
}
//------------------------------------------------------------------------

} // namespace mam::analysing
