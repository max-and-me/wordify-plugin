//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include <stdint.h>
#include <unordered_map>
#include <functional>

namespace mam::tiny_observer_pattern {

//------------------------------------------------------------------------
// Subject
//------------------------------------------------------------------------
class Subject
{
public:
    //--------------------------------------------------------------------
    using ObserverID = std::uint64_t;
    using Callback   = std::function<void()>;

    auto add_listener(Callback&& cb) -> ObserverID;
    auto remove_listener(ObserverID observer_id) -> bool;

    //--------------------------------------------------------------------
protected:
    static ObserverID observer_id;
    using Observers = std::unordered_map<ObserverID, Callback>;
    Observers observers;

    auto notify_all_observers() const -> void;
};
//------------------------------------------------------------------------

} // namespace mam::tiny_observer_pattern