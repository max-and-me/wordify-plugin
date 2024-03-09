//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ARA_Library/PlugIn/ARAPlug.h"
#include "meta_words_data.h"

namespace mam::meta_words {

//------------------------------------------------------------------------
class PlaybackRegion : public ARA::PlugIn::PlaybackRegion
{
public:
    //--------------------------------------------------------------------
    explicit PlaybackRegion(ARA::PlugIn::AudioModification* audioModification,
                            ARA::ARAPlaybackRegionHostRef hostRef) noexcept;

    const MetaWordsData
    get_meta_words_data(ARA::ARASampleRate playback_sample_rate) const;
    //--------------------------------------------------------------------
private:
};

//------------------------------------------------------------------------
} // namespace mam::meta_words