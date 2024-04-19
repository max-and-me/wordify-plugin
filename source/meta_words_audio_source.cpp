//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------
#include "meta_words_audio_source.h"
#include "little_helpers.h"
#include "mam/meta_words/runner.h"
#include "samplerate.h"
#include "sndfile.h"
#include "vstgpt_defines.h"
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

    auto set_size(size_t block_size) -> AudioBlockReader&
    {
        this->block_size   = block_size;
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

//-----------------------------------------------------------------------------
auto run_sync(const Command& cmd, FnProgress&& fn_progress) -> MetaWords
{
    std::cout << "Run sync..." << '\n';

    // FnProgress fn_progress = [](double val) { double tmp = val; };

    return (run(cmd, fn_progress));
}

//------------------------------------------------------------------------
auto create_whisper_cmd(const PathType& file_path) -> const Command
{
    //  The whisper.cpp library takes the audio file and writes the result
    //  of its analysis into a CSV file. The file is named like the audio
    //  file and by prepending ".csv" e.g. my_speech.wav ->
    //  my_speech.wav.csv
    const Options options         = {"-ocsv"};
    const OneValArgs one_val_args = {
        // model file resp. binary
        {"-m", MAM_WHISPER_CPP_MODEL_DOWNLOAD_DIR "/ggml-base.en.bin"},
        //{"-m", MAM_WHISPER_CPP_MODEL_DOWNLOAD_DIR "/ggml-medium.bin"},
        // audio file to analyse
        {"-f", file_path},
        // maximum segment length in characters: "1" mains one word
        {"-ml", "1"}};

    // static constexpr auto EXE_PATH =
    // "Z:\\Private\\mam\\vst-gpt_build\\bin\\Release\\main.exe"; Command cmd{
    // EXE_PATH, options, one_val_args};
    Command cmd{MAM_WHISPER_CPP_EXECUTABLE, options, one_val_args};
    return cmd;
}

//------------------------------------------------------------------------
auto process_audio_with_meta_words(const PathType& file_path,
                                   FnProgress&& fn_progress) -> MetaWords
{
    const auto cmd = create_whisper_cmd(file_path);
    return run_sync(cmd, std::move(fn_progress));
};

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
auto trim_meta_words(MetaWords& meta_words) -> MetaWords
{
    // Check the first entry, which can be empty. If so, remove it.
    auto iter = meta_words.begin();
    if (iter != meta_words.end())
    {
        if (iter->word.empty())
            iter = meta_words.erase(iter);
    }

    for (auto& meta_word : meta_words)
    {
        meta_word.word = trim(meta_word.word);
    }

    return meta_words;
}

//------------------------------------------------------------------------
} // namespace

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

    future_meta_words = std::async([&, path]() {
        FnProgress fn_progress = [&](ProgressValue val) {
            this->analysis_progress = val;
        };

        return process_audio_with_meta_words(path, std::move(fn_progress));
    });

    this->begin_analysis();
}

//------------------------------------------------------------------------
auto AudioSource::idle() -> void
{
    if (future_meta_words.wait_for(std::chrono::seconds(0)) ==
        std::future_status::ready)
    {
        end_analysis();

        if (changed_func)
            changed_func(this);
    }
    else
    {
        perform_analysis();
    }
}

//------------------------------------------------------------------------
void AudioSource::begin_analysis()
{
    fn_start_stop_changed(*this, true);
    timer = Steinberg::owned(Steinberg::Timer::create(
        Steinberg::newTimerCallback(
            [this](Steinberg::Timer* timer) { this->idle(); }),
        1.));
}

//------------------------------------------------------------------------
void AudioSource::perform_analysis()
{
    double progress_val = analysis_progress;
    fn_progress_changed(*this, progress_val);
}

//------------------------------------------------------------------------
void AudioSource::end_analysis()
{
    this->meta_words = future_meta_words.get();
    this->meta_words = trim_meta_words(this->meta_words);
    transform_to_seconds(this->meta_words);

    if (timer)
        timer->stop();

    fn_start_stop_changed(*this, false);
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
const AudioSource::MetaWords& AudioSource::get_meta_words() const
{
    return meta_words;
}

//------------------------------------------------------------------------
} // namespace mam::meta_words
