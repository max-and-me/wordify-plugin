//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#include "ara_audio_source.h"

#include <cmath>

namespace mam {
namespace {

//------------------------------------------------------------------------
using PathType = const std::string;

//------------------------------------------------------------------------
using DataPointers = std::vector<void*>;
DataPointers
create_data_pointers(ARATestAudioSource::MultiChannelBufferType& audio_buffers)
{
    DataPointers data_pointers;
    data_pointers.reserve(audio_buffers.size());
    for (auto ch = 0; ch < audio_buffers.size(); ++ch)
        data_pointers.push_back(static_cast<void*>(audio_buffers[ch].data()));

    return data_pointers;
}

//------------------------------------------------------------------------
void read_audio_from_host(ARATestAudioSource& audio_src)
{
    // create temporary host audio reader and let it fill the buffers
    // (we can safely ignore any errors while reading since host must clear
    // buffers in that case, as well as report the error to the user)
    DataPointers data_pointers =
        create_data_pointers(audio_src.get_audio_buffers());

    ARA::PlugIn::HostAudioReader audioReader{&audio_src};
    audioReader.readAudioSamples(
        0, static_cast<ARA::ARASampleCount>(audio_src.getChannelCount()),
        data_pointers.data());
}

//------------------------------------------------------------------------
void write_audio_to_file(ARATestAudioSource& audio_src,
                         const PathType& file_path)
{
    // TODO
}

//------------------------------------------------------------------------
} // namespace

//------------------------------------------------------------------------
void ARATestAudioSource::updateRenderSampleCache()
{
#if 1
    this->audio_buffers.resize(this->getChannelCount());
    for (auto& ch : audio_buffers)
        ch.resize(this->getSampleCount());

    read_audio_from_host(*this);
    write_audio_to_file(*this, PathType{"C:\\sine.wav"});

#else
    ARA_INTERNAL_ASSERT(isSampleAccessEnabled());

    // set up cache (this is a hack, so we're ignoring potential overflow of
    // 32 bit with long files here...)
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
#endif
}

//------------------------------------------------------------------------
const float* ARATestAudioSource::getRenderSampleCacheForChannel(
    ARA::ARAChannelCount channel) const
{
    return _sampleCache.data() +
           static_cast<size_t>(channel * getSampleCount());
}

//------------------------------------------------------------------------
void ARATestAudioSource::destroyRenderSampleCache()
{
    _sampleCache.clear();
    _sampleCache.resize(0);
}

//------------------------------------------------------------------------
} // namespace mam