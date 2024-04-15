//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>

namespace mam::tiny_observer_pattern {

//------------------------------------------------------------------------
using ObserverID = std::uint64_t;

//------------------------------------------------------------------------
// Subject
//------------------------------------------------------------------------
template <typename CallbackData>
class Subject
{
public:
    //--------------------------------------------------------------------
    using Callback = std::function<void(const CallbackData)>;

    auto add_listener(Callback&& cb) -> ObserverID
    {
        auto observer_id = new_id();
        observers.emplace(observer_id, std::move(cb));
        return observer_id;
    }

    auto remove_listener(ObserverID observer_id) -> bool
    {
        const auto it = observers.find(observer_id);
        if (it == observers.end())
            return false;

        observers.erase(it);
        return true;
    }

    void notify_listeners(const CallbackData& data)
    {
        for (const auto& observer : observers)
        {
            observer.second(data);
        }
    }

    //--------------------------------------------------------------------
protected:
    using Observers = std::unordered_map<ObserverID, Callback>;
    Observers observers;

    auto new_id() -> ObserverID
    {
        static ObserverID id;
        return ++id;
    }
};

//------------------------------------------------------------------------
using CallbackData = struct
{
};
using SimpleSubject = Subject<CallbackData>;
//------------------------------------------------------------------------

} // namespace mam::tiny_observer_pattern