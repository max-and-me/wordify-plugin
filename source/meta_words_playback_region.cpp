//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "meta_words_playback_region.h"
#include "ARA_Library/PlugIn/ARAPlug.h"
#include "ara_audio_source.h"

namespace mam::meta_words {

//------------------------------------------------------------------------
using Seconds = MetaWordsData::Seconds;

Seconds calculate_project_offset(const PlaybackRegion& region)
{
    return region.getStartInPlaybackTime() -
           region.getStartInAudioModificationTime();
}

//------------------------------------------------------------------------
mam::meta_words::MetaWords filter_meta_words(const MetaWords& words,
                                             const PlaybackRegion& region)
{
    MetaWords filtered_words;

    const auto startInAudioModificationTime =
        region.getStartInAudioModificationTime() * 1000.;

    const auto endInAudioModificationTime =
        (region.getStartInAudioModificationTime() +
         region.getDurationInAudioModificationTime()) *
        1000.;

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
mam::meta_words::MetaWords collect_meta_words(const PlaybackRegion& region)
{
    mam::meta_words::MetaWords words;

    if (const auto* modification = region.getAudioModification())
    {
        if (const auto* source =
                modification->getAudioSource<ARATestAudioSource>())
        {
            words = source->get_meta_words();
        }
    }

    return filter_meta_words(words, region);
}

//------------------------------------------------------------------------
PlaybackRegion::PlaybackRegion(
    ARA::PlugIn::AudioModification* audioModification,
    ARA::ARAPlaybackRegionHostRef hostRef) noexcept
: ARA::PlugIn::PlaybackRegion(audioModification, hostRef)
{
}

//------------------------------------------------------------------------
const MetaWordsData PlaybackRegion::get_meta_words_data() const
{
    MetaWordsData data;

    data.words          = collect_meta_words(*this);
    data.project_offset = calculate_project_offset(*this);

    return data;
}
//------------------------------------------------------------------------
} // namespace mam::meta_words