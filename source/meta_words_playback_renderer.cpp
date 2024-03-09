//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#include "meta_words_playback_renderer.h"
#include "ARA_Library/Utilities/ARASamplePositionConversion.h"
#include "ara_document_controller.h"
#include "meta_words_audio_source.h"
#include <algorithm>

namespace mam::meta_words {

//------------------------------------------------------------------------
// PlaybackRenderer
//------------------------------------------------------------------------
void PlaybackRenderer::renderPlaybackRegions(
    float* const* ppOutput,
    ARA::ARASamplePosition samplePosition,
    ARA::ARASampleCount samplesToRender,
    bool isPlayingBack)
{
    // initialize output buffers with silence, in case no viable playback
    // region intersects with the current buffer, or if the model is
    // currently not accessible due to being edited.
    for (auto c{0}; c < _channelCount; ++c)
        std::memset(ppOutput[c], 0,
                    sizeof(float) * static_cast<size_t>(samplesToRender));

    // only output samples while host is playing back
    if (!isPlayingBack)
        return;

    // flag that we've started rendering to prevent the document from being
    // edited while in this callback see TestDocumentController for details.
    auto docController{getDocumentController<ARADocumentController>()};
    if (docController->rendererWillAccessModelGraph(this))
    {
        const auto sampleEnd{samplePosition + samplesToRender};
        for (const auto& playbackRegion : getPlaybackRegions())
        {
            const auto audioSource{playbackRegion->getAudioModification()
                                       ->getAudioSource<const AudioSource>()};

            // render silence if access is currently disabled
            // (this is done here only to ease host debugging - actual
            // plug-ins would have at least some samples cached for realtime
            // access and would continue unless there's a cache miss.)
            if (!audioSource->isSampleAccessEnabled())
                continue;

            // this simplified test code "rendering" only produces audio if
            // the sample rate matches
            // if (audioSource->getSampleRate() != _sampleRate)
            //    continue;

            // evaluate region borders in song time, calculate sample range
            // to copy in song time (if a plug-in uses playback region
            // head/tail time, it will also need to reflect these values
            // here)
            const auto regionStartSample =
                playbackRegion->getStartInPlaybackSamples(_sampleRate);
            if (sampleEnd <= regionStartSample)
                continue;

            const auto regionEndSample{
                playbackRegion->getEndInPlaybackSamples(_sampleRate)};
            if (regionEndSample <= samplePosition)
                continue;

            auto startSongSample = std::max(regionStartSample, samplePosition);
            auto endSongSample   = std::min(regionEndSample, sampleEnd);

            // calculate offset between song and audio source samples, clip
            // at region borders in audio source samples (if a plug-in
            // supports time stretching, it will also need to reflect the
            // stretch factor here)
            // const auto offsetToPlaybackRegion =
            // playbackRegion->getStartInAudioModificationSamples() -
            // regionStartSample;
            const auto offsetToPlaybackRegion =
                ARA::samplePositionAtTime(
                    playbackRegion->getStartInAudioModificationTime(),
                    _sampleRate) -
                regionStartSample;

            const auto startAvailableSourceSamples =
                std::max(ARA::ARASamplePosition{0},
                         ARA::samplePositionAtTime(
                             playbackRegion->getStartInAudioModificationTime(),
                             _sampleRate));

            const auto endAvailableSourceSamples =
                std::min(audioSource->getSampleCount(),
                         ARA::samplePositionAtTime(
                             playbackRegion->getEndInAudioModificationTime(),
                             _sampleRate));
            /*const auto endAvailableSourceSamples =
                std::min(audioSource->getSampleCount(),
                         playbackRegion->getEndInAudioModificationSamples());*/

            startSongSample =
                std::max(startSongSample,
                         startAvailableSourceSamples - offsetToPlaybackRegion);
            endSongSample = std::min(endSongSample, endAvailableSourceSamples -
                                                        offsetToPlaybackRegion);
            if (endSongSample <= startSongSample)
                 continue;

            // add samples from audio source
            const auto sourceChannelCount{audioSource->getChannelCount()};
            for (auto posInSong{startSongSample}; posInSong < endSongSample;
                 ++posInSong)
            {
                const auto posInBuffer{posInSong - samplePosition};
                const auto posInSource{posInSong + offsetToPlaybackRegion};
                if (sourceChannelCount == _channelCount)
                {
                    for (auto c{0}; c < sourceChannelCount; ++c)
                    {
                        const auto* channel_samples =
                            audioSource->getRenderSampleCacheForChannel(c);
                        ppOutput[c][posInBuffer] +=
                            channel_samples[posInSource];
                    }
                }
                else
                {
                    // crude channel format conversion:
                    // mix down to mono, then distribute the mono signal
                    // evenly to all channels. note that when down-mixing to
                    // mono, the result is scaled by channel count, whereas
                    // upon up-mixing it is just copied to all channels.
                    // \todo ambisonic formats should just stick with the
                    // mono sum on channel 0,
                    //       but in this simple test code we currently do
                    //       not distinguish ambisonics
                    float monoSum{0.0f};
                    for (auto c{0}; c < sourceChannelCount; ++c)
                        monoSum += audioSource->getRenderSampleCacheForChannel(
                            c)[posInSource];
                    if (sourceChannelCount > 1)
                        monoSum /= static_cast<float>(sourceChannelCount);
                    for (auto c{0}; c < _channelCount; ++c)
                        ppOutput[c][posInBuffer] = monoSum;
                }
            }
        }

        // let the document controller know we're done
        docController->rendererDidAccessModelGraph(this);
    }
}

//------------------------------------------------------------------------
void PlaybackRenderer::enableRendering(
    ARA::ARASampleRate sampleRate,
    ARA::ARAChannelCount channelCount,
    ARA::ARASampleCount maxSamplesToRender) noexcept
{
    // proper plug-ins would use this call to manage the resources which
    // they need for rendering, but our test plug-in caches everything it
    // needs in-memory anyways, so this method is near-empty
    _sampleRate         = sampleRate;
    _channelCount       = channelCount;
    _maxSamplesToRender = maxSamplesToRender;
#if ARA_VALIDATE_API_CALLS
    _isRenderingEnabled = true;
#endif
}

//------------------------------------------------------------------------
void PlaybackRenderer::disableRendering() noexcept
{
#if ARA_VALIDATE_API_CALLS
    _isRenderingEnabled = false;
#endif
}

//------------------------------------------------------------------------
#if ARA_VALIDATE_API_CALLS
void PlaybackRenderer::willAddPlaybackRegion(
    ARA::PlugIn::PlaybackRegion* /*playbackRegion*/) noexcept
{
    ARA_VALIDATE_API_STATE(!_isRenderingEnabled);
}

//------------------------------------------------------------------------
void PlaybackRenderer::willRemovePlaybackRegion(
    ARA::PlugIn::PlaybackRegion* /*playbackRegion*/) noexcept
{
    ARA_VALIDATE_API_STATE(!_isRenderingEnabled);
}
#endif

//------------------------------------------------------------------------

} // namespace mam::meta_words
