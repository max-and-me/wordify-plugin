//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ARA_Library/PlugIn/ARAPlug.h"
#include "nonstd.h"
#include "meta_words_audio_source.h"
#include "meta_words_data.h"
#include <optional>

namespace mam::meta_words {

//------------------------------------------------------------------------
class PlaybackRegion : public ARA::PlugIn::PlaybackRegion
{
public:
    //--------------------------------------------------------------------
    using Id                       = uint64_t;
    static constexpr Id INVALID_ID = -1;
    using Color                    = MetaWordsData::Color;

    using AudioBuf =
        mam::audio_buffer_management::AudioBuffer<AudioSource::SampleType>;
    using AudioBufferSpan = gsl::span<const AudioSource::SampleType>;

    struct AudioBufferSpanData
    {
        size_t offset_samples = 0;
        AudioBufferSpan audio_buffer_span;
    };

    explicit PlaybackRegion(ARA::PlugIn::AudioModification* audioModification,
                            ARA::ARAPlaybackRegionHostRef hostRef) noexcept;

    auto get_meta_words_data() const -> const MetaWordsData;
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