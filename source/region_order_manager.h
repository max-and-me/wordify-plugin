// Copyright (c) 2023-present, WordifyOrg.

#pragma once

#include "meta_words_playback_region.h"
#include "warn_cpp/suppress_warnings.h"
BEGIN_SUPPRESS_WARNINGS
#include "eventpp/callbacklist.h"
END_SUPPRESS_WARNINGS

namespace mam {

//------------------------------------------------------------------------
struct PlaybackRegionOrderChangeData
{
    // TODO: Nothing to do here
};

//------------------------------------------------------------------------
// RegionOrderManager
//
// Keeps a sorted list of all playback regions IDs. The regions are always
// sorted to the playback time (no matter in which sequence they are). When IDs
// have been sorted, all observers will be notified.
//------------------------------------------------------------------------
class RegionOrderManager
{
public:
    //--------------------------------------------------------------------
    using PlaybackRegion = meta_words::PlaybackRegion;
    using OrderedIds     = std::vector<Id>;
    using OrderSubject =
        eventpp::CallbackList<void(const PlaybackRegionOrderChangeData&)>;
    using FuncStartInPlaybackTime = std::function<double(Id)>;

    auto get_order_subject() -> OrderSubject*;
    auto push_back(Id id) -> void;
    auto remove(Id id) -> void;
    auto reorder() -> void;

    template <typename Func>
    auto for_each_region_id_enumerated(Func& func) const;

    FuncStartInPlaybackTime start_in_playback_time_func;

    //--------------------------------------------------------------------
private:
    OrderSubject playback_region_order_subject;
    OrderedIds playback_region_ids_ordered;
};

//------------------------------------------------------------------------
template <typename Func>
inline auto RegionOrderManager::for_each_region_id_enumerated(Func& func) const
{
    for (OrderedIds::size_type i = 0; i < playback_region_ids_ordered.size();
         i++)
    {
        func(i, playback_region_ids_ordered.at(i));
    }
}

//------------------------------------------------------------------------
} // namespace mam
