// Copyright(c) 2024 Max And Me.

#include "meta_words_playback_region.h"
#include "meta_words_audio_source.h"
#include <array>
BEGIN_SUPRESS_WARNINGS
#include "ARA_Library/PlugIn/ARAPlug.h"
END_SUPRESS_WARNINGS

namespace mam::meta_words {

//------------------------------------------------------------------------
using Word                                      = std::string;
using PunctuationMarks                          = std::array<Word, 15>;
static const PunctuationMarks PUNCTUATION_MARKS = {".", "?", "!", ",",  ";",
                                                   "-", "(", ")", "[",  "]",
                                                   "{", "}", "'", "\"", "..."};
static auto is_puntuation_mark(const Word& word) -> bool
{
    for (auto& el : PUNCTUATION_MARKS)
    {
        if (el == word)
            return true;
    }
    return false;
}

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
static auto mark_clipped_words(MetaWordDataset& words,
                               const PlaybackRegion& region) -> MetaWordDataset
{
    for (auto& word_data : words)
    {
        word_data.is_clipped_by_region =
            !is_in_playback_region(region, word_data.word);
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
                word_data.is_punctuation_mark =
                    is_puntuation_mark(meta_word.word);
                word_dataset.push_back(word_data);
            }
        }
    }

    return word_dataset;
}

//------------------------------------------------------------------------
Id PlaybackRegion::new_id = PlaybackRegion::INVALID_ID;
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
    Color color = std::make_tuple(0.f, 0.f, 0.f);
    if (const auto& tmp_color = getEffectiveColor())
    {
        color = std::make_tuple(tmp_color->r, tmp_color->g, tmp_color->b);
    }

    return color;
}
//------------------------------------------------------------------------
const MetaWordsData PlaybackRegion::get_meta_words_data() const
{
    MetaWordsData data;

    data.words = collect_meta_words(*this);
    // Since we calculate everything in seconds, we dont need modify timestamps
    // to the sample rate
    // data.words = modify_time_stamps(data.words, *this, playback_sample_rate);
    data.words              = mark_clipped_words(data.words, *this);
    data.project_offset     = calculate_project_offset(*this);
    data.project_time_start = getStartInPlaybackTime();
    data.duration           = getDurationInPlaybackTime();

    if (getRegionSequence())
    {
        data.name  = getEffectiveName();
        data.color = get_effective_color();
    }

    return data;
}

//------------------------------------------------------------------------
auto PlaybackRegion::get_audio_buffer() const -> const AudioBufferSpanData
{
    const auto& audioSrc = this->getAudioModification()
                               ->getAudioSource<mam::meta_words::AudioSource>();

    const auto& audio_buffers = audioSrc->get_audio_buffers();
    const auto& left_channel  = audio_buffers.at(0);
    AudioBufferSpan buffer_span{left_channel};

    const auto start_samples = size_t(this->getStartInAudioModificationTime() *
                                      audioSrc->getSampleRate());
    auto duration_samples =
        size_t(this->getDurationInPlaybackTime() * audioSrc->getSampleRate());

    duration_samples = std::min(duration_samples, left_channel.size());

    const AudioBufferSpanData data{
        start_samples,
        AudioBufferSpan(left_channel).subspan(start_samples, duration_samples)};

    return data;
}
//------------------------------------------------------------------------
} // namespace mam::meta_words
