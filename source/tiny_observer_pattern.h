//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include <functional>
#include <stdint.h>
#include <unordered_map>

namespace mam::tiny_observer_pattern {

//------------------------------------------------------------------------
using ObserverID = std::uint64_t;

//------------------------------------------------------------------------
// Subject
//------------------------------------------------------------------------
template <typename Func>
class Subject
{
public:
    //--------------------------------------------------------------------
    using Callback = Func;

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
using SimpleCallback = std::function<void()>;
using SimpleSubject  = Subject<SimpleCallback>;
//------------------------------------------------------------------------

} // namespace mam::tiny_observer_pattern