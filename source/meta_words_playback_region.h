//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ARA_Library/PlugIn/ARAPlug.h"
#include "meta_words_data.h"
#include "gsl/span"
#include "meta_words_audio_source.h"

namespace mam::meta_words {

//------------------------------------------------------------------------
class PlaybackRegion : public ARA::PlugIn::PlaybackRegion
{
public:
    //--------------------------------------------------------------------
    using AudioBuf = mam::audio_buffer_management::AudioBuffer<AudioSource::SampleType>;
    using AudioBufferSpan = gsl::span<const AudioSource::SampleType>;
    ;
    explicit PlaybackRegion(ARA::PlugIn::AudioModification* audioModification,
                            ARA::ARAPlaybackRegionHostRef hostRef) noexcept;

    auto get_meta_words_data(ARA::ARASampleRate playback_sample_rate) const
        -> const MetaWordsData;

    auto get_audio_buffer(ARA::ARASampleRate playback_sample_rate) const -> const AudioBufferSpan;
    //--------------------------------------------------------------------
private:
};

//------------------------------------------------------------------------
} // namespace mam::meta_words