//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ARA_Library/PlugIn/ARAPlug.h"
#include "audio_buffer_management.h"
#include "base/source/timer.h"
#include "mam/meta_words/meta_word.h"
#include <future>

namespace mam::meta_words {

//------------------------------------------------------------------------
class AudioSource : public ARA::PlugIn::AudioSource
{
public:
    using SampleType = float;
    using Identifier = int;
    using MultiChannelBufferType =
        mam::audio_buffer_management::MultiChannelBuffers<SampleType>;
    using MetaWords = mam::meta_words::MetaWords;
    using FnChanged = std::function<void(AudioSource*)>;
    using FnStartStopChanged =
        std::function<void(const AudioSource& source, bool status)>;
    using FnProgressChanged =
        std::function<void(const AudioSource& source, double progress)>;

    AudioSource(ARA::PlugIn::Document* document,
                ARA::ARAAudioSourceHostRef hostRef,
                FnStartStopChanged&& fn_start_stop_changed,
                FnProgressChanged&& fn_progress_changed,
                Identifier identifier)
    : ARA::PlugIn::AudioSource{document, hostRef}
    , fn_changed(fn_changed)
    , fn_start_stop_changed(fn_start_stop_changed)
    , fn_progress_changed(fn_progress_changed)
    , identifier(identifier)
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
    FnChanged changed_func;

    const Identifier getIdentifier() const { return identifier; }

protected:
    void idle();
    void begin_analysis();
    void perform_analysis();
    void end_analysis();
    std::atomic<double> analysis_progress = 0.;

    MultiChannelBufferType audio_buffers;
    MetaWords meta_words;
    FnChanged fn_changed;
    FnStartStopChanged fn_start_stop_changed;
    FnProgressChanged fn_progress_changed;

    using MetaWordsFuture = std::future<MetaWords>;
    Steinberg::IPtr<Steinberg::Timer> timer;
    MetaWordsFuture future_meta_words;
    Identifier identifier = -1;
};

//------------------------------------------------------------------------
} // namespace mam::meta_words
