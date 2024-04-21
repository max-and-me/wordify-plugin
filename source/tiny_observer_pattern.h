//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
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
using None = struct
{
};
using SimpleSubject = Subject<None>;

//------------------------------------------------------------------------
template <typename T>
class Observer
{
public:
    //------------------------------------------------------------------------
    Observer(T* subject)
    : subject(subject)
    {
        if (this->subject)
        {
            observer_id = subject->add_listener([&](const auto& data) {
                if (on_notify)
                    on_notify(data);
            });
        }
    }

    ~Observer()
    {
        if (subject)
        {
            subject->remove_listener(observer_id);
        }
    }

    typename T::Callback on_notify;

    //------------------------------------------------------------------------
private:
    T* subject             = nullptr;
    ObserverID observer_id = 0;
};

//------------------------------------------------------------------------
template <typename T, typename F>
auto make_observer(T* subject, F callback) -> std::unique_ptr<Observer<T>>
{
    auto observer =
        std::make_unique<tiny_observer_pattern::Observer<T>>(subject);
    observer->on_notify = callback;
    return observer;
}
//------------------------------------------------------------------------

} // namespace mam::tiny_observer_pattern