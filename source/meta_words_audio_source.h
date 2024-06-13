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
                FuncAnalyseProgress&& analyse_progress_func,
                Id id);
    ~AudioSource() override;

    auto updateRenderSampleCache() -> void;
    auto destroyRenderSampleCache() -> void;
    auto getRenderSampleCache(ARA::ARAChannelCount channel) const -> const
        float*;
    auto get_audio_buffers() -> MultiChannelBufferType&;
    auto get_meta_words() const -> const MetaWords&;
    auto set_meta_words(const MetaWords& meta_words) -> void;
    auto get_id() const -> const Id { return id; }

    //--------------------------------------------------------------------
protected:
    void begin_analysis();
    void perform_analysis();
    void end_analysis();

    Id id{0};
    OptionalId task_id;
    MultiChannelBufferType audio_buffers;
    MetaWords meta_words;
    FuncAnalyseProgress analyse_progress_func;
};

//------------------------------------------------------------------------
} // namespace mam::meta_words
