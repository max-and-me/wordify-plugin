//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ARA_Library/PlugIn/ARAPlug.h"
#include "mam/meta_words/meta_word.h"

namespace mam::meta_words {

//------------------------------------------------------------------------
class AudioSource : public ARA::PlugIn::AudioSource
{
public:
    using SampleType             = float;
    using ChannelBufferType      = std::vector<SampleType>;
    using MultiChannelBufferType = std::vector<ChannelBufferType>;
    using MetaWords              = mam::meta_words::MetaWords;

    AudioSource(ARA::PlugIn::Document* document,
                ARA::ARAAudioSourceHostRef hostRef)
    : ARA::PlugIn::AudioSource{document, hostRef}
    {
    }
    virtual ~AudioSource(){};

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
    MultiChannelBufferType& get_audio_buffers() { return audio_buffers; }

    const MetaWords& get_meta_words() const;

protected:
    MultiChannelBufferType audio_buffers;
    std::vector<float> _sampleCache;
    MetaWords meta_words;
};

//------------------------------------------------------------------------
} // namespace mam::meta_words
