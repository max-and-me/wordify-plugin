//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "tiny_observer_pattern.h"

namespace mam::tiny_observer_pattern {

//------------------------------------------------------------------------
Subject::ObserverID Subject::observer_id = 0;

//------------------------------------------------------------------------
auto Subject::add_listener(Callback&& cb) -> ObserverID
{
    observers.emplace(++observer_id, std::move(cb));
    return observer_id;
}

//------------------------------------------------------------------------
auto Subject::remove_listener(ObserverID observer_id) -> bool
{
    const auto it = observers.find(observer_id);
    if (it == observers.end())
        return false;

    observers.erase(it);
    return true;
}

//------------------------------------------------------------------------
auto Subject::notify_all_observers() const -> void
{
    for (const auto& observer : observers)
    {
        observer.second();
    }
}

//------------------------------------------------------------------------
} // namespace mam::tiny_observer_pattern