//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ARA_Library/PlugIn/ARAPlug.h"

namespace mam {

//------------------------------------------------------------------------
class ARATestAudioSource : public ARA::PlugIn::AudioSource
{
public:
    using SampleType             = float;
    using ChannelBufferType      = std::vector<SampleType>;
    using MultiChannelBufferType = std::vector<ChannelBufferType>;

    ARATestAudioSource(ARA::PlugIn::Document* document,
                       ARA::ARAAudioSourceHostRef hostRef)
    : AudioSource{document, hostRef}
    {
    }

    // render thread sample access:
    // in order to keep this test code as simple as possible, our test audio
    // source uses brute force and caches all samples in-memory so that
    // renderers can access it without threading issues the document controller
    // triggers filling this cache on the main thread, immediately after access
    // is enabled. actual plug-ins will use a multi-threaded setup to only cache
    // sections of the audio source on demand - a sophisticated file I/O
    // threading implementation is needed for file-based processing regardless
    // of ARA.
    void updateRenderSampleCache();
    const float*
    getRenderSampleCacheForChannel(ARA::ARAChannelCount channel) const;
    void destroyRenderSampleCache();
    MultiChannelBufferType& get_audio_buffers();

protected:
    MultiChannelBufferType audio_buffers;
    std::vector<float> _sampleCache;
};

//------------------------------------------------------------------------
}