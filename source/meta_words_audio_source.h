// Copyright(c) 2025 Max And Me.

#pragma once

#include "audio_buffer_management.h"
#include "mam/meta_words/meta_word.h"
#include "warn_cpp/suppress_warnings.h"
#include "wordify_types.h"
#include <future>
#include <optional>
BEGIN_SUPPRESS_WARNINGS
#include "ARA_Library/PlugIn/ARAPlug.h"
#include "base/source/timer.h"
END_SUPPRESS_WARNINGS

namespace mam::meta_words {

//------------------------------------------------------------------------
//  AnalyseProgressData
//------------------------------------------------------------------------
struct AnalyseProgressData
{
    enum class State
    {
        BeginAnalyse,
        PerformAnalyse,
        EndAnalyse,
    };

    Id audio_source_id{0};
    State state{State::BeginAnalyse};
};

//------------------------------------------------------------------------
// AudioSource
//------------------------------------------------------------------------
class AudioSource : public ARA::PlugIn::AudioSource
{
public:
    //--------------------------------------------------------------------
    using SampleType = float;
    using MultiChannelBufferType =
        mam::audio_buffer_management::MultiChannelBuffers<SampleType>;
    using MetaWords           = mam::meta_words::MetaWords;
    using FnChanged           = std::function<void(AudioSource*)>;
    using FuncAnalyseProgress = std::function<void(const AnalyseProgressData&)>;

    AudioSource(ARA::PlugIn::Document* document,
                ARA::ARAAudioSourceHostRef hostRef,
                Id id);
    ~AudioSource() override;

    auto updateRenderSampleCache() -> void;
    auto destroyRenderSampleCache() -> void;
    auto getRenderSampleCache(ARA::ARAChannelCount channel) const -> const
        float*;
    auto get_audio_buffers() -> MultiChannelBufferType&;
    auto get_meta_words() const -> const MetaWords&;
    auto set_meta_words(const MetaWords& meta_words) -> void;
    auto get_id() const -> Id { return id; }

    FuncAnalyseProgress analyse_progress_func;

    //--------------------------------------------------------------------
protected:
    void begin_analysis();
    void perform_analysis();
    void end_analysis();

    Id id{0};
    OptionalId task_id;
    MultiChannelBufferType audio_buffers;
    MetaWords meta_words;
};

//------------------------------------------------------------------------
} // namespace mam::meta_words
