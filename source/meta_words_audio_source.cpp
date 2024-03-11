//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------
#include "meta_words_audio_source.h"
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
auto read_audio_from_host(AudioSource& audio_src) -> void
{
    // create temporary host audio reader and let it fill the buffers
    // (we can safely ignore any errors while reading since host must clear
    // buffers in that case, as well as report the error to the user)

    auto data_pointers = mam::audio_buffer_management::to_channel_data(
        audio_src.get_audio_buffers());

    ARA::PlugIn::HostAudioReader audioReader{&audio_src};
    audioReader.readAudioSamples(
        0, static_cast<ARA::ARASampleCount>(audio_src.getSampleCount()),
        data_pointers.data());
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
        src_simple(&data, SRC_SINC_BEST_QUALITY, audio_src.getChannelCount());
    if (error != 0)
        resampled_buf.resize(0);

    return resampled_buf;
}

//------------------------------------------------------------------------
auto write_audio_to_file(AudioSource& audio_src, const PathType& file_path)
    -> int
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
        // audio file to analyse
        {"-f", file_path},
        // maximum segment length in characters: "1" mains one word
        {"-ml", "1"}};

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

        if (fn_changed)
            fn_changed();
    }
    else
    {
        perform_analysis();
    }
}

//------------------------------------------------------------------------
void AudioSource::begin_analysis()
{
    timer = Steinberg::owned(Steinberg::Timer::create(
        Steinberg::newTimerCallback(
            [this](Steinberg::Timer* timer) { this->idle(); }),
        1.));
}

//------------------------------------------------------------------------
void AudioSource::perform_analysis()
{
    double progress_val = analysis_progress;
    // TODO: update progress bar
}

//------------------------------------------------------------------------
void AudioSource::end_analysis()
{
    this->meta_words = future_meta_words.get();
    transform_to_seconds(this->meta_words);

    if (timer)
        timer->stop();
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
