// Copyright(c) 2024 Max And Me.

#pragma once

#include "meta_words_audio_source.h"
#include "nonstd.h"
#include "region_data.h"
#include "supress_warnings.h"
#include "wordify_types.h"
#include <optional>
BEGIN_SUPRESS_WARNINGS
#include "ARA_Library/PlugIn/ARAPlug.h"
END_SUPRESS_WARNINGS

namespace mam::meta_words {

//------------------------------------------------------------------------
class PlaybackRegion : public ARA::PlugIn::PlaybackRegion
{
public:
    //--------------------------------------------------------------------
    static constexpr Id INVALID_ID = 0;
    using Color                    = RegionData::Color;

    using AudioBuf =
        mam::audio_buffer_management::AudioBuffer<AudioSource::SampleType>;
    using AudioBufferSpan = nonstd::span<const AudioSource::SampleType>;

    struct AudioBufferSpanData
    {
        size_t offset_samples = 0;
        AudioBufferSpan audio_buffer_span;
    };

    explicit PlaybackRegion(ARA::PlugIn::AudioModification* audioModification,
                            ARA::ARAPlaybackRegionHostRef hostRef) noexcept;

    auto get_region_data() const -> const RegionData;
    auto get_audio_buffer() const -> const AudioBufferSpanData;
    auto get_id() const -> Id { return id; }
    auto get_effective_color() const -> Color;

    //--------------------------------------------------------------------
private:
    static Id new_id;
    Id id = INVALID_ID;
};

//------------------------------------------------------------------------
using OptPlaybackRegionPtr = std::optional<const PlaybackRegion*>;

//------------------------------------------------------------------------
} // namespace mam::meta_words
