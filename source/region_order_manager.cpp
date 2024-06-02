//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "region_order_manager.h"

namespace mam {

//------------------------------------------------------------------------
bool sort(RegionOrderManager::FuncStartInPlaybackTime& start_in_playback_time,
          RegionOrderManager::OrderedIds& ids)
{
    using Id = Id;

    auto sorter = [&](const Id id0, const Id id1) {
        const auto time0 = start_in_playback_time(id0);
        const auto time1 = start_in_playback_time(id1);

        bool result = time0 < time1;

        return result;
    };

    if (std::is_sorted(ids.begin(), ids.end(), sorter))
        return false;

    std::sort(ids.begin(), ids.end(), sorter);
    return true;
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
auto RegionOrderManager::push_back(Id id) -> void
{
    playback_region_ids_ordered.push_back(id);
    reorder();
}

//------------------------------------------------------------------------
auto RegionOrderManager::remove(Id id) -> void
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
        playback_region_order_subject({});
}

//------------------------------------------------------------------------
} // namespace mam
