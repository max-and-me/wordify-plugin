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
static auto is_in_playback_region(const PlaybackRegion& region,
                                  const meta_words::MetaWord& word) -> bool
{
    const auto start_time = region.getStartInAudioModificationTime();
    const auto end_time   = (region.getStartInAudioModificationTime() +
                           region.getDurationInAudioModificationTime());

    const auto word_end = word.begin + word.duration;

    const bool is_in = (word.begin >= start_time && word.begin < end_time) ||
                       (word_end >= start_time && word_end < end_time);

    return is_in;
}

//------------------------------------------------------------------------
static auto
filter_audible_words(MetaWordDataset& words,
                     const PlaybackRegion& region) -> MetaWordDataset
{
    for (auto& word_data : words)
    {
        word_data.is_clipped_by_region =
            is_in_playback_region(region, word_data.word);
    }

    return words;
}

//------------------------------------------------------------------------
static auto
collect_meta_words(const PlaybackRegion& region) -> const MetaWordDataset
{
    MetaWordDataset word_dataset;

    if (const auto* modification = region.getAudioModification())
    {
        if (const auto* source = modification->getAudioSource<AudioSource>())
        {
            const auto meta_words = source->get_meta_words();
            for (const auto& meta_word : meta_words)
            {
                MetaWordData word_data;
                word_data.word                 = meta_word;
                word_data.is_clipped_by_region = true;
                word_dataset.push_back(word_data);
            }
        }
    }

    return word_dataset;
}

//------------------------------------------------------------------------
static auto
compute_speed_factor(const PlaybackRegion& region,
                     ARA::ARASampleRate playback_sample_rate) -> double
{
    double speed_factor = 1.;
    if (const auto* modification = region.getAudioModification())
    {
        if (const auto* source = modification->getAudioSource<AudioSource>())
        {
            speed_factor = source->getSampleRate() / playback_sample_rate;
        }
    }

    return speed_factor;
}

//------------------------------------------------------------------------
static auto modify_time_stamps(MetaWordData& word,
                               double speed_factor) -> MetaWordData&
{
    word.word.begin *= speed_factor;
    word.word.duration *= speed_factor;
    return word;
}

//------------------------------------------------------------------------
// Playback speed can differ from the real speed. Imagine the sample being
// played back in 44.1Khz but the original sample is in 16kHz. We need to
// modify the timestamps then.
//------------------------------------------------------------------------
static auto modify_time_stamps(MetaWordDataset& word_dataset,
                               const PlaybackRegion& region,
                               ARA::ARASampleRate playback_sample_rate)
    -> const MetaWordDataset
{
    const auto speed_factor =
        compute_speed_factor(region, playback_sample_rate);

    for (auto& word : word_dataset)
        word = modify_time_stamps(word, speed_factor);

    return word_dataset;
}

//------------------------------------------------------------------------
PlaybackRegion::Id PlaybackRegion::new_id = PlaybackRegion::INVALID_ID;
PlaybackRegion::PlaybackRegion(
    ARA::PlugIn::AudioModification* audioModification,
    ARA::ARAPlaybackRegionHostRef hostRef) noexcept
: ARA::PlugIn::PlaybackRegion(audioModification, hostRef)
, id(++new_id)
{
}

//------------------------------------------------------------------------
auto PlaybackRegion::get_effective_color() const -> Color
{
    Color color = std::make_tuple(0, 0, 0);
    if (const auto& tmp_color = getEffectiveColor())
    {
        color = std::make_tuple(tmp_color->r, tmp_color->g, tmp_color->b);
    }

    return color;
}
//------------------------------------------------------------------------
const MetaWordsData PlaybackRegion::get_meta_words_data(
    ARA::ARASampleRate playback_sample_rate) const
{
    MetaWordsData data;

    data.words = collect_meta_words(*this);
    data.words = modify_time_stamps(data.words, *this, playback_sample_rate);
    data.words = filter_audible_words(data.words, *this);
    data.project_offset     = calculate_project_offset(*this);
    data.project_time_start = this->getStartInPlaybackTime();

    if (getRegionSequence())
    {
        data.name  = getEffectiveName();
        data.color = get_effective_color();
    }

    return data;
}

//------------------------------------------------------------------------
auto PlaybackRegion::get_audio_buffer(
    ARA::ARASampleRate playback_sample_rate) const -> const AudioBufferSpanData
{
    const auto& audioSrc = this->getAudioModification()
                               ->getAudioSource<mam::meta_words::AudioSource>();

    const auto& audio_buffers = audioSrc->get_audio_buffers();
    const auto& left_channel  = audio_buffers.at(0);
    AudioBufferSpan buffer_span{left_channel};

    const auto start_samples =
        size_t(this->getStartInAudioModificationTime() * playback_sample_rate);
    const auto duration_samples =
        size_t(this->getDurationInPlaybackTime() * playback_sample_rate);

    const AudioBufferSpanData data{
        start_samples,
        AudioBufferSpan(left_channel).subspan(start_samples, duration_samples)};

    return data;
}
//------------------------------------------------------------------------
} // namespace mam::meta_words