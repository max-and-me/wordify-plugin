// Copyright(c) 2024 Max And Me.

#include "task_manager.h"
#include "base/source/timer.h"
#include "mam/meta_words/runner.h"
#include "whipser_cpp_wrapper.h"
#include <array>
#include <future>
#include <optional>

namespace mam::task_managing {
namespace {

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
using OptionalId   = std::optional<Id>;
using AtomicBool   = std::atomic_bool;
using FutureResult = std::future<ResultData>;

//------------------------------------------------------------------------
// Task
//------------------------------------------------------------------------
struct Task
{
    Id task_id = 0;
    InputData input_data;
    FuncFinished finished_callback;
};

using OptionalTask = std::optional<Task>;

//------------------------------------------------------------------------
// Worker
//------------------------------------------------------------------------
struct Worker
{
    OptionalTask optional_task;
    AtomicBool is_canceled = false;
    FutureResult future_result;

    auto do_work() -> void;
    auto is_busy() const -> bool;
    auto cancel() -> void;
};

//------------------------------------------------------------------------
auto Worker::do_work() -> void
{
    if (!is_busy())
        return;

    if (future_result.wait_for(std::chrono::seconds(0)) ==
        std::future_status::ready)
    {
        is_canceled     = false;
        auto meta_words = future_result.get();

        if (optional_task.has_value())
        {
            auto& task = optional_task.value();
            task.finished_callback(meta_words);
            optional_task.reset();
        }
    }
}

//------------------------------------------------------------------------
auto Worker::is_busy() const -> bool
{
    return optional_task.has_value();
}

//------------------------------------------------------------------------
auto Worker::cancel() -> void
{
    is_canceled = true;
    if (is_busy())
    {
        optional_task.reset();
        future_result.wait_for(std::chrono::seconds(5));
    }
}

//------------------------------------------------------------------------
// TaskManager
/* Has a list of tasks and a list of workers. A worker can work on one task
 * at a time. The task will be removed from the list and transfered to a worker.
 *
 */
//------------------------------------------------------------------------
struct TaskManager
{
    static constexpr auto kNumWorkers = 1;
    using TaskList                    = std::vector<Task>;
    using WorkerList                  = std::array<Worker, kNumWorkers>;

    static auto instance() -> TaskManager&
    {
        static TaskManager inst;
        return inst;
    }

    auto initialise(FuncTaskCount&& task_count_func) -> bool;
    auto terminate() -> void;
    auto append_task(const InputData& input_data,
                     FuncFinished&& finished_func) -> Id;
    auto cancel_task(Id task_id) -> bool;
    auto count_tasks() const -> size_t;

    // private:
    auto work() -> void;
    auto assign_next_tasks() -> void;

    FuncTaskCount task_count_callback;
    WorkerList workers;
    TaskList tasks;

    Steinberg::IPtr<Steinberg::Timer> timer;
    static Id next_task_id;
};

//------------------------------------------------------------------------
Id TaskManager::next_task_id = 0;
auto TaskManager::initialise(FuncTaskCount&& task_count_callback_) -> bool
{
    task_count_callback = task_count_callback_;
    return true;
}

//------------------------------------------------------------------------
auto TaskManager::terminate() -> void
{
    task_count_callback = nullptr;
    tasks.clear();
    for (auto& worker : workers)
    {
        worker.cancel();
    }
}

//------------------------------------------------------------------------
auto TaskManager::append_task(const InputData& input_data,
                              FuncFinished&& finished_func) -> Id
{
    next_task_id++;

    Task new_task{next_task_id, input_data, std::move(finished_func)};
    tasks.emplace_back(new_task);

    if (!timer)
    {
        timer = Steinberg::owned(Steinberg::Timer::create(
            Steinberg::newTimerCallback(
                [this](Steinberg::Timer* timer) { this->work(); }),
            1.));

        assign_next_tasks();
    }

    if (task_count_callback)
        task_count_callback(count_tasks());

    return next_task_id;
}

//------------------------------------------------------------------------
auto TaskManager::cancel_task(Id task_id) -> bool
{
    auto iter =
        std::remove_if(tasks.begin(), tasks.end(), [task_id](const auto& task) {
            return task.task_id == task_id;
        });

    if (iter != tasks.end())
    {
        tasks.erase(iter, tasks.end());
        return true;
    }

    for (auto& worker : workers)
    {
        if (!worker.is_busy())
            continue;

        if (worker.optional_task.value().task_id == task_id)
        {
            worker.is_canceled = true;
            return true;
        }
    }

    return false;
}

//------------------------------------------------------------------------
auto TaskManager::work() -> void
{
    bool stop_work_timer = true;
    for (auto& worker : workers)
    {
        worker.do_work();
        if (worker.is_busy())
        {
            stop_work_timer = false;
            continue;
        }

        if (task_count_callback)
            task_count_callback(count_tasks());

        assign_next_tasks();
    }

    if (stop_work_timer)
        timer = nullptr;
}

//------------------------------------------------------------------------
auto TaskManager::assign_next_tasks() -> void
{
    if (tasks.empty())
        return;

    for (auto& worker : workers)
    {
        if (worker.is_busy())
            continue;

        if (tasks.empty())
            break;

        auto task = tasks.front();
        tasks.erase(tasks.begin());

        worker.optional_task = task;
        worker.is_canceled   = false;
        worker.future_result = std::async([&]() {
            auto progress_func = [&](auto) {};
            auto cancel_func   = [&]() { return worker.is_canceled.load(); };

            const auto cmd =
                create_whisper_cmd(worker.optional_task.value().input_data);
            return meta_words::run(cmd, std::move(progress_func),
                                   std::move(cancel_func));
        });
    }
}

//------------------------------------------------------------------------
auto TaskManager::count_tasks() const -> size_t
{
    auto count = tasks.size();
    for (const auto& worker : workers)
    {
        if (worker.is_busy())
            count++;
    }

    return count;
}

//------------------------------------------------------------------------
} // namespace

//------------------------------------------------------------------------
auto initialise(FuncTaskCount&& task_count_func) -> bool
{
    return TaskManager::instance().initialise(std::move(task_count_func));
}

//------------------------------------------------------------------------
auto terminate() -> void
{
    TaskManager::instance().terminate();
}

//------------------------------------------------------------------------
auto append_task(const InputData& input_data,
                 FuncFinished&& finished_func) -> Id
{
    return TaskManager::instance().append_task(input_data,
                                               std::move(finished_func));
}

//------------------------------------------------------------------------
auto cancel_task(Id task_id) -> bool
{
    return TaskManager::instance().cancel_task(task_id);
}

//------------------------------------------------------------------------
auto count_tasks() -> size_t
{
    return TaskManager::instance().count_tasks();
}

//------------------------------------------------------------------------
} // namespace mam::task_managing
