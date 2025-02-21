// Copyright (c) 2023-present, WordifyOrg.

#include "region_order_manager.h"
#include <cassert>

namespace mam {

//------------------------------------------------------------------------
bool sort(RegionOrderManager::FuncStartInPlaybackTime& start_in_playback_time,
          RegionOrderManager::OrderedIds& ids)
{
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
    assert(start_in_playback_time_func &&
           "start_in_playback_time_func must be set from outside!");
    if (!start_in_playback_time_func)
        return;

    bool is_notify_listeners =
        sort(start_in_playback_time_func, playback_region_ids_ordered);

    if (is_notify_listeners)
        playback_region_order_subject({});
}

//------------------------------------------------------------------------
auto RegionOrderManager::get_order_subject() -> OrderSubject*
{
    return &playback_region_order_subject;
}

//------------------------------------------------------------------------
} // namespace mam
