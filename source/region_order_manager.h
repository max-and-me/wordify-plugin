//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "eventpp/callbacklist.h"
#include "meta_words_playback_region.h"

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
    using OrderedIds     = std::vector<PlaybackRegion::Id>;
    using OrderSubject =
        eventpp::CallbackList<void(const PlaybackRegionOrderChangeData&)>;

    using FuncStartInPlaybackTime = std::function<double(PlaybackRegion::Id)>;

    auto
    initialize(FuncStartInPlaybackTime&& start_in_playback_time_func) -> bool;
    auto get_order_subject() -> OrderSubject*
    {
        return &playback_region_order_subject;
    }
    auto push_back(PlaybackRegion::Id id) -> void;
    auto remove(PlaybackRegion::Id id) -> void;
    auto reorder() -> void;
    template <typename Func>
    auto for_each_playback_region_id_enumerated(Func& func) const
    {
        for (OrderedIds::size_type i = 0;
             i < playback_region_ids_ordered.size(); i++)
        {
            func(i, playback_region_ids_ordered.at(i));
        }
    }

    //--------------------------------------------------------------------
private:
    OrderSubject playback_region_order_subject;
    OrderedIds playback_region_ids_ordered;
    FuncStartInPlaybackTime start_in_playback_time_func;
};

//------------------------------------------------------------------------
} // namespace mam
