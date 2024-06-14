// Copyright(c) 2024 Max And Me.

#include "meta_words_audio_source.h"
#include "little_helpers.h"
#include "mam/meta_words/runner.h"
#include "supress_warnings.h"
#include "task_manager.h"
#include "wordify_defines.h"
#include <cctype>
#include <cmath>
#include <filesystem>
#include <functional>
#include <iostream>
#include <thread>
BEGIN_SUPRESS_WARNINGS
#include "samplerate.h"
#include "sndfile.h"
END_SUPRESS_WARNINGS

namespace mam::meta_words {
namespace {

//------------------------------------------------------------------------
using PathType                       = const std::string;
const double WHISPER_CPP_SAMPLE_RATE = 16000.;

//------------------------------------------------------------------------
class AudioBlockReader
{
public:
    //--------------------------------------------------------------------
    AudioBlockReader(size_t samples_total)
    : samples_total(samples_total)
    {
    }

    auto set_size(size_t value) -> AudioBlockReader&
    {
        block_size   = value;
        samples_read = 0;
        return *this;
    }

    template <typename Func>
    auto read(Func&& func) -> size_t
    {
        if (sample_pos > samples_total)
            return 0;

        const auto sample_begin = sample_pos;
        auto sample_end         = sample_begin + block_size;
        sample_end              = std::min(sample_end, samples_total);

        samples_read = sample_end - sample_begin;

        func(sample_pos, samples_read);

        sample_pos += samples_read;
        return samples_read;
    }

    //--------------------------------------------------------------------
private:
    size_t block_size    = 0;
    size_t sample_pos    = 0;
    size_t samples_read  = 0;
    size_t samples_total = 0;
};

//------------------------------------------------------------------------
auto read_audio_from_host(AudioSource& audio_src) -> void
{
    const auto sample_count = static_cast<size_t>(audio_src.getSampleCount());
    auto block_reader       = AudioBlockReader(sample_count).set_size(4096);

    while (true)
    {
        auto num_read = block_reader.read([&](auto pos, auto count) {
            auto data_pointers = mam::audio_buffer_management::to_channel_data(
                audio_src.get_audio_buffers(), pos);

            ARA::PlugIn::HostAudioReader audioReader{&audio_src};
            audioReader.readAudioSamples(
                static_cast<ARA::ARASamplePosition>(pos),
                static_cast<ARA::ARASampleCount>(count), data_pointers.data());
        });

        if (num_read == 0)
            break;
    }
}

//------------------------------------------------------------------------
auto compute_new_buffer_size(size_t buf_size, double ratio) -> size_t
{
    const auto new_size = static_cast<double>(buf_size) * ratio;
    return static_cast<size_t>(new_size + 0.5);
}

//------------------------------------------------------------------------
auto resample_to_16kHz(
    const AudioSource& audio_src,
    const audio_buffer_management::AudioBuffer<AudioSource::SampleType>& buf)
    -> const audio_buffer_management::AudioBuffer<AudioSource::SampleType>
{
    const auto ratio = WHISPER_CPP_SAMPLE_RATE / audio_src.getSampleRate();

    const auto new_size = compute_new_buffer_size(buf.size(), ratio);
    audio_buffer_management::AudioBuffer<AudioSource::SampleType> resampled_buf;
    resampled_buf.resize(new_size);

    const auto channel_count = static_cast<size_t>(audio_src.getChannelCount());
    SRC_DATA data{};
    data.data_in      = buf.data();
    data.data_out     = resampled_buf.data();
    data.input_frames = static_cast<long>(buf.size() / channel_count);
    data.output_frames =
        static_cast<long>(resampled_buf.size() / channel_count);
    data.src_ratio = ratio;

    const auto error =
        src_simple(&data, SRC_LINEAR, audio_src.getChannelCount());
    if (error != 0)
        resampled_buf.resize(0);

    return resampled_buf;
}

//------------------------------------------------------------------------
auto write_audio_to_file(AudioSource& audio_src,
                         const PathType& file_path) -> int
{
    const auto buffers   = audio_src.get_audio_buffers();
    auto interleaved_buf = audio_buffer_management::to_interleaved(buffers);

    bool needs_resampling =
        audio_src.getSampleRate() != WHISPER_CPP_SAMPLE_RATE;
    if (needs_resampling)
        interleaved_buf = resample_to_16kHz(audio_src, interleaved_buf);

    std::filesystem::remove(file_path);

    SF_INFO sfinfo{
        /* sf_count_t	frames = */ 0,
        /* int			samplerate = */
        static_cast<int>(WHISPER_CPP_SAMPLE_RATE),
        /* int			channels = */ audio_src.getChannelCount(),
        /* int			format = */ (SF_FORMAT_WAV | SF_FORMAT_PCM_16),
        /* int          sections = */ 0,
        /* int          seekable = */ 0};

    SNDFILE* file = sf_open(file_path.data(), SFM_RDWR, &sfinfo);
    if (!file)
    {
        printf("Error : Not able to open output file.\n");
        return 1;
    };

    const auto write_count =
        sf_write_float(file, interleaved_buf.data(),
                       static_cast<sf_count_t>(interleaved_buf.size()));
    if (static_cast<size_t>(write_count) != interleaved_buf.size())
    {
        puts(sf_strerror(file));
    }

    sf_close(file);
    return 0;
}

//------------------------------------------------------------------------
auto transform_to_seconds(MetaWords& meta_words) -> void
{
    for (auto& meta_word : meta_words)
    {
        meta_word.begin *= 0.001;
        meta_word.duration *= 0.001;
    }
}

//------------------------------------------------------------------------
auto trim_meta_words(MetaWords& meta_words) -> void
{
    for (auto& word : meta_words)
    {
        auto& str = word.word;
        str.erase(std::remove_if(str.begin(), str.end(),
                                 [](char c) {
                                     return std::isspace(
                                         static_cast<unsigned char>(c));
                                 }),
                  str.end());
    }
}

//------------------------------------------------------------------------
} // namespace

//------------------------------------------------------------------------
// AudioSource
//------------------------------------------------------------------------
AudioSource::AudioSource(ARA::PlugIn::Document* document,
                         ARA::ARAAudioSourceHostRef hostRef,
                         FuncAnalyseProgress&& analyse_progress_func,
                         Id id)
: ARA::PlugIn::AudioSource{document, hostRef}
, analyse_progress_func(analyse_progress_func)
, id(id)
{
}

//------------------------------------------------------------------------

AudioSource::~AudioSource()
{
    if (task_id.has_value())
    {
        task_managing::cancel_task(task_id.value());
        task_id.reset();
    }
};

//------------------------------------------------------------------------
void AudioSource::updateRenderSampleCache()
{
    ARA_INTERNAL_ASSERT(isSampleAccessEnabled());

    if (audio_buffers.size() > 0)
        return;

    // Read audio buffers from host
    const auto channel_count = static_cast<size_t>(getChannelCount());
    const auto sample_count  = static_cast<size_t>(getSampleCount());
    audio_buffers =
        audio_buffer_management::create_multi_channel_buffers<SampleType>(
            channel_count, sample_count);

    read_audio_from_host(*this);

    // Write audio buffers to temporary audio file
    const auto tmp_dir =
        std::filesystem::temp_directory_path() / PLUGIN_IDENTIFIER;
    std::filesystem::create_directories(tmp_dir);

    const auto tmp_file = tmp_dir / PathType(getName());
    const auto path     = PathType{tmp_file.generic_u8string()};
    write_audio_to_file(*this, path);

    // Start analyse task on temporary audio file
    task_id = task_managing::append_task(path, [&](auto meta_words_) {
        meta_words = meta_words_;
        end_analysis();
    });

    begin_analysis();
}

//------------------------------------------------------------------------
void AudioSource::begin_analysis()
{
    const AnalyseProgressData& data = {
        /*.id*/ get_id(),
        /*.state*/ AnalyseProgressData::State::BeginAnalyse,
    };

    analyse_progress_func(data);
}

//------------------------------------------------------------------------
void AudioSource::perform_analysis()
{
    const AnalyseProgressData& data = {
        /*.id*/ get_id(),
        /*.state*/ AnalyseProgressData::State::PerformAnalyse,
    };

    analyse_progress_func(data);
}

//------------------------------------------------------------------------
void AudioSource::end_analysis()
{
    transform_to_seconds(meta_words);
    trim_meta_words(meta_words);

    const AnalyseProgressData& data = {
        /*.id*/ get_id(),
        /*.state*/ AnalyseProgressData::State::EndAnalyse,
    };

    analyse_progress_func(data);

    task_id.reset();
}

//------------------------------------------------------------------------
const float*
AudioSource::getRenderSampleCache(ARA::ARAChannelCount channel) const
{
    return static_cast<const float*>(audio_buffers[channel].data());
}

//------------------------------------------------------------------------
void AudioSource::destroyRenderSampleCache()
{
    audio_buffers.clear();
}

//------------------------------------------------------------------------
auto AudioSource::get_meta_words() const -> const MetaWords&
{
    return meta_words;
}

//------------------------------------------------------------------------
auto AudioSource::set_meta_words(const MetaWords& meta_words_) -> void
{
    if (task_id.has_value())
        task_managing::cancel_task(task_id.value());

    meta_words = meta_words_;
    trim_meta_words(meta_words);
}

//------------------------------------------------------------------------
auto AudioSource::get_audio_buffers() -> MultiChannelBufferType&
{
    return audio_buffers;
}

//------------------------------------------------------------------------
} // namespace mam::meta_words
