//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "meta_words_playback_region.h"
#include "ARA_Library/PlugIn/ARAPlug.h"
#include "meta_words_audio_source.h"

namespace mam::meta_words {

//------------------------------------------------------------------------
using Seconds = const MetaWordsData::Seconds;

static auto calculate_project_offset(const PlaybackRegion& region) -> Seconds
{
    return region.getStartInPlaybackTime() -
           region.getStartInAudioModificationTime();
}

//------------------------------------------------------------------------
static auto filter_audible_meta_words(const MetaWords& words,
                                      const PlaybackRegion& region) -> MetaWords
{
    MetaWords filtered_words;

    const auto startInAudioModificationTime =
        region.getStartInAudioModificationTime();

    const auto endInAudioModificationTime =
        (region.getStartInAudioModificationTime() +
         region.getDurationInAudioModificationTime());

    auto is_in_range =
        [startInAudioModificationTime,
         endInAudioModificationTime](const meta_words::MetaWord& word) {
            return word.begin >= startInAudioModificationTime &&
                   word.begin < endInAudioModificationTime;
        };

    std::copy_if(words.begin(), words.end(), std::back_inserter(filtered_words),
                 is_in_range);

    return filtered_words;
}

//------------------------------------------------------------------------
static auto collect_meta_words(const PlaybackRegion& region) -> const MetaWords
{
    MetaWords words;

    if (const auto* modification = region.getAudioModification())
    {
        if (const auto* source = modification->getAudioSource<AudioSource>())
        {
            words = source->get_meta_words();
        }
    }

    return words;
}

//------------------------------------------------------------------------
// Playback speed can differ from the real speed. Imagine the sample being
// played back in 44.1Khz but the original sample is in 16kHz. We need to
// modify the timestamps then.
//------------------------------------------------------------------------
static auto
modify_meta_words_time_stamps(const MetaWords& words,
                              const PlaybackRegion& region,
                              ARA::ARASampleRate playback_sample_rate)
    -> const MetaWords
{
    MetaWords modified_words;

    double speed_factor = 1.;
    if (const auto* modification = region.getAudioModification())
    {
        if (const auto* source = modification->getAudioSource<AudioSource>())
        {
            speed_factor = source->getSampleRate() / playback_sample_rate;
        }
    }

    for (const auto& el : words)
    {
        auto modified_el = el;
        modified_el.begin *= speed_factor;
        modified_el.duration *= speed_factor;

        modified_words.push_back(modified_el);
    }

    return modified_words;
}

//------------------------------------------------------------------------
PlaybackRegion::PlaybackRegion(
    ARA::PlugIn::AudioModification* audioModification,
    ARA::ARAPlaybackRegionHostRef hostRef) noexcept
: ARA::PlugIn::PlaybackRegion(audioModification, hostRef)
{
}

//------------------------------------------------------------------------
const MetaWordsData PlaybackRegion::get_meta_words_data(
    ARA::ARASampleRate playback_sample_rate) const
{
    MetaWordsData data;

    data.words = collect_meta_words(*this);
    data.words =
        modify_meta_words_time_stamps(data.words, *this, playback_sample_rate);
    data.words          = filter_audible_meta_words(data.words, *this);
    data.project_offset = calculate_project_offset(*this);
    data.name           = getEffectiveName();

    if (const auto& tmp_color = getEffectiveColor())
    {
        static const float RGB_MAX_FLOAT = 255.f;
        data.color.r = static_cast<uint8_t>(tmp_color->r * RGB_MAX_FLOAT);
        data.color.g = static_cast<uint8_t>(tmp_color->g * RGB_MAX_FLOAT);
        data.color.b = static_cast<uint8_t>(tmp_color->b * RGB_MAX_FLOAT);
    }

    return data;
}
//------------------------------------------------------------------------
} // namespace mam::meta_words