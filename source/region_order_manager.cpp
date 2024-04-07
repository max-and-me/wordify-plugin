//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#include "region_order_manager.h"

namespace mam {

//------------------------------------------------------------------------
bool sort(RegionOrderManager::FuncStartInPlaybackTime& start_in_playback_time,
          RegionOrderManager::OrderedIds& ids)
{
    using Id = meta_words::PlaybackRegion::Id;

    bool has_been_reorderd = false;
    auto sorter            = [&](const Id id0, const Id id1) {
        const auto time0 = start_in_playback_time(id0);
        const auto time1 = start_in_playback_time(id1);

        bool result = time0 < time1;

        has_been_reorderd |= result;

        return result;
    };

    std::sort(ids.begin(), ids.end(), sorter);
    return has_been_reorderd;
}

//------------------------------------------------------------------------
// RegionOrderManager
//------------------------------------------------------------------------
auto RegionOrderManager::initialize(
    FuncStartInPlaybackTime&& start_in_playback_time_func) -> bool
{
    this->start_in_playback_time_func = start_in_playback_time_func;
    return true;
}

//------------------------------------------------------------------------
auto RegionOrderManager::register_observer(OrderSubject::Callback&& callback)
    -> ObserverID
{
    return this->playback_region_order_subject.add_listener(
        std::move(callback));
}

//------------------------------------------------------------------------
auto RegionOrderManager::unregister_observer(ObserverID id) -> void
{
    playback_region_order_subject.remove_listener(id);
}

//------------------------------------------------------------------------
auto RegionOrderManager::push_back(PlaybackRegion::Id id) -> void
{
    playback_region_ids_ordered.push_back(id);
    reorder();
}

//------------------------------------------------------------------------
auto RegionOrderManager::remove(PlaybackRegion::Id id) -> void
{
    auto iter = std::remove(playback_region_ids_ordered.begin(),
                            playback_region_ids_ordered.end(), id);
    playback_region_ids_ordered.erase(iter, playback_region_ids_ordered.end());
}

//------------------------------------------------------------------------
auto RegionOrderManager::reorder() -> void
{
    bool should_notify_listeners = sort(this->start_in_playback_time_func,
                                        this->playback_region_ids_ordered);

    if (should_notify_listeners)
        playback_region_order_subject.notify_listeners({});
}

//------------------------------------------------------------------------
} // namespace mam
