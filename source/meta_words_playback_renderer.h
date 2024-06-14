// Copyright(c) 2024 Max And Me.

#pragma once

#include "supress_warnings.h"
BEGIN_SUPRESS_WARNINGS
#include "ARA_Library/PlugIn/ARAPlug.h"
END_SUPRESS_WARNINGS

namespace mam::meta_words {

//------------------------------------------------------------------------
//  ARAPlaybackRenderer
//------------------------------------------------------------------------
class PlaybackRenderer : public ARA::PlugIn::PlaybackRenderer
{
public:
    //------------------------------------------------------------------------
    using ARA::PlugIn::PlaybackRenderer::PlaybackRenderer;

    void renderPlaybackRegions(float* const* ppOutput,
                               ARA::ARASamplePosition samplePosition,
                               ARA::ARASampleCount samplesToRender,
                               bool isPlayingBack);

    void enableRendering(ARA::ARASampleRate sampleRate,
                         ARA::ARAChannelCount channelCount,
                         ARA::ARASampleCount maxSamplesToRender) noexcept;
    void disableRendering() noexcept;

protected:
    //------------------------------------------------------------------------
#if ARA_VALIDATE_API_CALLS
    void willAddPlaybackRegion(
        ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void willRemovePlaybackRegion(
        ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
#endif

private:
    ARA::ARASampleRate _sampleRate{44100.0f};
    ARA::ARASampleCount _maxSamplesToRender{4096};
    ARA::ARAChannelCount _channelCount{1};
#if ARA_VALIDATE_API_CALLS
    bool _isRenderingEnabled{false};
#endif
};

//------------------------------------------------------------------------
} // namespace mam::meta_words
