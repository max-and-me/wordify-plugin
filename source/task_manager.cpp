// Copyright(c) 2024 Max And Me.

#include "task_manager.h"
#include "base/source/timer.h"
#include <array>
#include <future>
#include <optional>

namespace mam::task_managing {
namespace {

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
    auto is_busy() -> bool;
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
        auto& task      = optional_task.value();
        task.finished_callback(meta_words);
    }
}

//------------------------------------------------------------------------
auto Worker::is_busy() -> bool
{
    return optional_task.has_value();
}

//------------------------------------------------------------------------
// TaskManager
//------------------------------------------------------------------------
struct TaskManager
{
    using TaskList = std::vector<Task>;
    using Workers  = std::array<Worker, 2>;

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

    auto work() -> void;
    auto assign_next_tasks() -> void;
    auto count_tasks() -> size_t;

    FuncTaskCount task_count_callback;
    Workers workers;
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
    tasks.clear();
    for (auto& worker : workers)
    {
        worker.is_canceled = true;
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
        task_count_callback(count_tasks());
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

    auto task = tasks.front();
    for (auto& worker : workers)
    {
        if (worker.is_busy())
            continue;

        worker.optional_task = task;
        tasks.erase(tasks.begin());

        if (tasks.empty())
            return;
    }
}

//------------------------------------------------------------------------
auto TaskManager::count_tasks() -> size_t
{
    auto count = tasks.size();
    for (auto& worker : workers)
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
auto append_task(const InputData input_data, FuncFinished&& finished_func) -> Id
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
} // namespace mam::task_managing
