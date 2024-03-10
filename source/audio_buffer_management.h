//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include <vector>

namespace mam::audio_buffer_management {

//------------------------------------------------------------------------
template <class T>
using AudioBuffer = typename std::vector<T>;

template <class T>
using MultiChannelBuffers = std::vector<AudioBuffer<T>>;

using MultiChannelData = std::vector<void*>;

//------------------------------------------------------------------------
template <class T>
auto create_channel_buffer(size_t num_samples) -> AudioBuffer<T>
{
    AudioBuffer<T> audio_buf;
    audio_buf.resize(num_samples);
    return audio_buf;
}

//------------------------------------------------------------------------
template <class T>
auto create_multi_channel_buffers(size_t num_channels, size_t num_samples)
    -> MultiChannelBuffers<T>
{
    MultiChannelBuffers<T> multi_channel_bufs;
    multi_channel_bufs.resize(num_channels);
    for (auto& channel : multi_channel_bufs)
    {
        channel.resize(num_samples);
    }

    return multi_channel_bufs;
}

//------------------------------------------------------------------------
template <class T>
auto to_channel_data(MultiChannelBuffers<T>& multi_channel_bufs) -> MultiChannelData
{
    MultiChannelData channel_data;
    channel_data.reserve(multi_channel_bufs.size());
    for (auto ch = 0; ch < multi_channel_bufs.size(); ++ch)
        channel_data.push_back(static_cast<void*>(multi_channel_bufs[ch].data()));

    return channel_data;
}

//------------------------------------------------------------------------
template <class T>
auto to_interleaved(const MultiChannelBuffers<T>& multi_channel_bufs) -> AudioBuffer<T>
{
    AudioBuffer<T> interleaved_buf;
    if (multi_channel_bufs.empty())
        return interleaved_buf;

    const auto num_channels = multi_channel_bufs.size();
    const auto num_samples = multi_channel_bufs[0].size();

    for (auto sample = 0; sample < num_samples; sample++)
        for (auto channel = 0; channel < num_channels; channel++)
            interleaved_buf.push_back(multi_channel_bufs[channel][sample]);

    return interleaved_buf;
}

//------------------------------------------------------------------------
} // namespace mam::audio_buffer_management