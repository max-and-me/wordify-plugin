//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#include "ara_audio_source.h"

#include <cmath>

//------------------------------------------------------------------------
void ARATestAudioSource::updateRenderSampleCache()
{
    ARA_INTERNAL_ASSERT(isSampleAccessEnabled());

    // set up cache (this is a hack, so we're ignoring potential overflow of 32
    // bit with long files here...)
    const auto channelCount{static_cast<size_t>(getChannelCount())};
    const auto sampleCount{static_cast<size_t>(getSampleCount())};
    _sampleCache.resize(channelCount * sampleCount);

    // create temporary host audio reader and let it fill the cache
    // (we can safely ignore any errors while reading since host must clear
    // buffers in that case, as well as report the error to the user)
    ARA::PlugIn::HostAudioReader audioReader{this};
    std::vector<void*> dataPointers{channelCount};
    for (auto c{0U}; c < channelCount; ++c)
        dataPointers[c] = _sampleCache.data() + c * sampleCount;
    audioReader.readAudioSamples(
        0, static_cast<ARA::ARASampleCount>(sampleCount), dataPointers.data());

    std::vector<float*> dataPointers_f{channelCount};

    for (int i = 0; i < dataPointers.size(); i++)
    {
        dataPointers_f[i] = static_cast<float*>(dataPointers[i]);
    }
}

const float* ARATestAudioSource::getRenderSampleCacheForChannel(
    ARA::ARAChannelCount channel) const
{
    return _sampleCache.data() +
           static_cast<size_t>(channel * getSampleCount());
}

void ARATestAudioSource::destroyRenderSampleCache()
{
    _sampleCache.clear();
    _sampleCache.resize(0);
}
