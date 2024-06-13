//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ARA_Library/PlugIn/ARAPlug.h"
#include "audio_buffer_management.h"
#include "base/source/timer.h"
#include "mam/meta_words/meta_word.h"
#include "wordify_types.h"
#include <future>
#include <optional>

namespace mam::meta_words {

//------------------------------------------------------------------------
//  WordAnalysisProgressData
//------------------------------------------------------------------------
struct WordAnalysisProgressData
{
    enum class State
    {
        BeginAnalyse,
        PerformAnalyse,
        EndAnalyse,
    };

    size_t audio_source_id = 0;
    double progress_val    = 0.;
    State state;
};

//------------------------------------------------------------------------
// AudioSource
//------------------------------------------------------------------------
class AudioSource : public ARA::PlugIn::AudioSource
{
public:
    using SampleType = float;
    using Identifier = std::size_t;
    using MultiChannelBufferType =
        mam::audio_buffer_management::MultiChannelBuffers<SampleType>;
    using MetaWords = mam::meta_words::MetaWords;
    using FnChanged = std::function<void(AudioSource*)>;
    using FuncAnalyzeProgress =
        std::function<void(const WordAnalysisProgressData&)>;

    AudioSource(ARA::PlugIn::Document* document,
                ARA::ARAAudioSourceHostRef hostRef,
                FuncAnalyzeProgress&& analyze_progress_func,
                Identifier identifier);
    ~AudioSource() override;

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

    auto get_meta_words() const -> const MetaWords&;
    auto set_meta_words(const MetaWords& meta_words) -> void;
    FnChanged changed_func;

    const Identifier getIdentifier() const { return identifier; }

protected:
    void begin_analysis();
    void perform_analysis();
    void end_analysis();
    std::atomic<double> analysis_progress = 0.;

    MultiChannelBufferType audio_buffers;
    MetaWords meta_words;
    FuncAnalyzeProgress analyze_progress_func;

    Id identifier{0};

    using OptTaskId = std::optional<size_t>;
    OptTaskId task_id;
};

//------------------------------------------------------------------------
} // namespace mam::meta_words
