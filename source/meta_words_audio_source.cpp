//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------
#include "meta_words_audio_source.h"
#include "audio_source_analyze_worker.h"
#include "little_helpers.h"
#include "mam/meta_words/runner.h"
#include "samplerate.h"
#include "sndfile.h"
#include "wordify_defines.h"
#include <cmath>
#include <filesystem>
#include <functional>
#include <iostream>
#include <thread>

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
        this->block_size   = value;
        this->samples_read = 0;
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
    auto block_reader =
        AudioBlockReader(audio_src.getSampleCount()).set_size(4096);

    while (true)
    {
        auto num_read = block_reader.read([&](auto pos, auto count) {
            auto data_pointers = mam::audio_buffer_management::to_channel_data(
                audio_src.get_audio_buffers(), pos);

            ARA::PlugIn::HostAudioReader audioReader{&audio_src};
            audioReader.readAudioSamples(
                pos, static_cast<ARA::ARASampleCount>(count),
                data_pointers.data());
        });

        if (num_read == 0)
            break;
    }

    /*
    // create temporary host audio reader and let it fill the buffers
    // (we can safely ignore any errors while reading since host must clear
    // buffers in that case, as well as report the error to the user)

    auto data_pointers = mam::audio_buffer_management::to_channel_data(
        audio_src.get_audio_buffers());

    ARA::PlugIn::HostAudioReader audioReader{&audio_src};
    audioReader.readAudioSamples(
        0, static_cast<ARA::ARASampleCount>(audio_src.getSampleCount()),
        data_pointers.data());
        */
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

    SRC_DATA data{0};
    data.data_in  = buf.data();
    data.data_out = resampled_buf.data();
    data.input_frames =
        static_cast<long>(buf.size() / audio_src.getChannelCount());
    data.output_frames =
        static_cast<long>(resampled_buf.size() / audio_src.getChannelCount());
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
        /* int			format = */ (SF_FORMAT_WAV | SF_FORMAT_PCM_16)};

    SNDFILE* file = nullptr;
    if (!(file = sf_open(file_path.data(), SFM_RDWR, &sfinfo)))
    {
        printf("Error : Not able to open output file.\n");
        return 1;
    };

    if (sf_write_float(file, interleaved_buf.data(), interleaved_buf.size()) !=
        interleaved_buf.size())
    {
        puts(sf_strerror(file));
    }

    sf_close(file);
    return 0;
}

//------------------------------------------------------------------------
auto transform_to_seconds(MetaWords& meta_words) -> void
{
    std::transform(meta_words.begin(), meta_words.end(), meta_words.begin(),
                   [](MetaWord word) {
                       // ms to s
                       word.begin *= 0.001;
                       word.duration *= 0.001;
                       return word;
                   });
}

//------------------------------------------------------------------------
auto trim_first_meta_word(MetaWords& meta_words) -> MetaWords
{
    // Check the first entry, which can be empty. If so, remove it.
    auto iter = meta_words.begin();
    if (iter != meta_words.end())
    {
        if (iter->word.empty())
            iter = meta_words.erase(iter);
    }

    return meta_words;
}

//------------------------------------------------------------------------
auto trim_meta_words(MetaWords& meta_words) -> void
{
    for (auto& word : meta_words)
    {
        auto& str = word.word;
        str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end());
    }
}

//------------------------------------------------------------------------
} // namespace

//------------------------------------------------------------------------
// AudioSource
//------------------------------------------------------------------------
AudioSource::~AudioSource()
{
    if (task_id.has_value())
        analysing::cancel_task(task_id.value());
};

//------------------------------------------------------------------------
void AudioSource::updateRenderSampleCache()
{
    ARA_INTERNAL_ASSERT(isSampleAccessEnabled());

    if (this->audio_buffers.size() > 0)
        return;

    this->audio_buffers =
        audio_buffer_management::create_multi_channel_buffers<SampleType>(
            this->getChannelCount(), this->getSampleCount());

    read_audio_from_host(*this);

    const auto tmp_dir =
        std::filesystem::temp_directory_path() / PLUGIN_IDENTIFIER;
    std::filesystem::create_directories(tmp_dir);

    const auto tmp_file = tmp_dir / PathType(this->getName());
    const auto path     = PathType{tmp_file.generic_u8string()};
    write_audio_to_file(*this, path);

    task_id = analysing::push_task(
        path,
        [&](auto meta_words_) {
            // TODO
            this->meta_words = meta_words_;
            this->end_analysis();
        },
        [&](auto value) { // TODO
            this->analysis_progress = value;
            this->perform_analysis();
        });

    this->begin_analysis();
}

//------------------------------------------------------------------------
void AudioSource::begin_analysis()
{
    const WordAnalysisProgressData& data = {
        /*.id*/ this->getIdentifier(),
        /*.progress*/ 0.,
        /*.state*/ WordAnalysisProgressData::State::kAnalysisStarted,
    };

    analyze_progress_func(data);
}

//------------------------------------------------------------------------
void AudioSource::perform_analysis()
{
    double progress_val = analysis_progress;
    // fn_progress_changed(*this, progress_val);

    const WordAnalysisProgressData& data = {
        /*.id*/ this->getIdentifier(),
        /*.progress*/ progress_val,
        /*.state*/ WordAnalysisProgressData::State::kAnalysisRunning,
    };

    analyze_progress_func(data);
}

//------------------------------------------------------------------------
void AudioSource::end_analysis()
{
    transform_to_seconds(this->meta_words);
    trim_meta_words(this->meta_words);

    const WordAnalysisProgressData& data = {
        /*.id*/ this->getIdentifier(),
        /*.progress*/ 1.,
        /*.state*/ WordAnalysisProgressData::State::kAnalysisStopped,
    };

    analyze_progress_func(data);

    task_id.reset();
}

//------------------------------------------------------------------------
const float*
AudioSource::getRenderSampleCacheForChannel(ARA::ARAChannelCount channel) const
{
    return audio_buffers[channel].data();
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
        analysing::cancel_task(task_id.value());

    this->meta_words = meta_words_;
    trim_meta_words(this->meta_words);
}

//------------------------------------------------------------------------
} // namespace mam::meta_words
