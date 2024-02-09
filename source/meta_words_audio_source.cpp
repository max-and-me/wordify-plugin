//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------
#include "meta_words_audio_source.h"

#include "sndfile.h"
#include "vstgpt_context.h"
#include "vstgpt_defines.h"

#include "mam/meta_words/runner.h"

#include <cmath>
#include <filesystem>
#include <functional>
#include <iostream>

namespace mam::meta_words {
namespace {

//------------------------------------------------------------------------
using PathType     = const std::string;
using DataPointers = std::vector<void*>;
enum class ChannelIndex : int
{
    kLeft,
    kRight
};
using FnSampleValue = std::function<AudioSource::SampleType(ChannelIndex, int)>;

//------------------------------------------------------------------------
DataPointers
create_data_pointers(AudioSource::MultiChannelBufferType& audio_buffers)
{
    DataPointers data_pointers;
    data_pointers.reserve(audio_buffers.size());
    for (auto ch = 0; ch < audio_buffers.size(); ++ch)
        data_pointers.push_back(static_cast<void*>(audio_buffers[ch].data()));

    return data_pointers;
}

//------------------------------------------------------------------------
void read_audio_from_host(AudioSource& audio_src)
{
    // create temporary host audio reader and let it fill the buffers
    // (we can safely ignore any errors while reading since host must clear
    // buffers in that case, as well as report the error to the user)
    DataPointers data_pointers =
        create_data_pointers(audio_src.get_audio_buffers());

    ARA::PlugIn::HostAudioReader audioReader{&audio_src};
    audioReader.readAudioSamples(
        0, static_cast<ARA::ARASampleCount>(audio_src.getSampleCount()),
        data_pointers.data());
}

//------------------------------------------------------------------------
int write_audio_file(const PathType& file_path,
                     float sample_rate,
                     int channel_count,
                     int sample_count,
                     FnSampleValue& func)
{
    std::filesystem::remove(file_path);

    using SampleType  = AudioSource::SampleType;
    auto SAMPLE_RATE  = sample_rate;
    auto SAMPLE_COUNT = sample_count;

    SNDFILE* file;
    SF_INFO sfinfo;
    int k;
    SampleType* buffer;

    if (!(buffer = static_cast<SampleType*>(
              malloc(2 * SAMPLE_COUNT * sizeof(SampleType)))))
    {
        printf("Error : Malloc failed.\n");
        return 1;
    };

    memset(&sfinfo, 0, sizeof(sfinfo));

    sfinfo.samplerate = SAMPLE_RATE;
    sfinfo.frames     = SAMPLE_COUNT;
    sfinfo.channels   = channel_count;
    sfinfo.format     = (SF_FORMAT_WAV | SF_FORMAT_PCM_16);

    if (!(file = sf_open(file_path.data(), SFM_RDWR, &sfinfo)))
    {
        printf("Error : Not able to open output file.\n");
        free(buffer);
        return 1;
    };

    if (sfinfo.channels == 1)
    {
        for (k = 0; k < SAMPLE_COUNT; k++)
            buffer[k] = func(ChannelIndex::kLeft, k);
    }
    else if (sfinfo.channels == 2)
    {
        for (k = 0; k < SAMPLE_COUNT; k++)
        {
            buffer[2 * k]     = func(ChannelIndex::kLeft, k);
            buffer[2 * k + 1] = func(ChannelIndex::kRight, k);
        };
    }
    else
    {
        printf("Error : make_sine can only generate mono or stereo files.\n");
        sf_close(file);
        free(buffer);
        return 1;
    };

    if (sf_write_float(file, buffer, sfinfo.channels * SAMPLE_COUNT) !=
        sfinfo.channels * SAMPLE_COUNT)
        puts(sf_strerror(file));

    sf_close(file);
    free(buffer);
    return 0;
}

//------------------------------------------------------------------------
void write_audio_to_file(AudioSource& audio_src, const PathType& file_path)
{
    auto buffers       = audio_src.get_audio_buffers();
    FnSampleValue func = [&](ChannelIndex ch, int sample) {
        return buffers[static_cast<int>(ch)][sample];
    };

    write_audio_file(file_path, audio_src.getSampleRate(),
                     audio_src.getChannelCount(), audio_src.getSampleCount(),
                     func);
}

//-----------------------------------------------------------------------------
meta_words::MetaWords run_sync(Command& cmd)
{
    std::cout << "Run sync..." << '\n';

    FnProgress fn_progress = [](double val) { double tmp = val; };

    return (run(cmd, fn_progress));
}

//------------------------------------------------------------------------
MetaWords process_audio_with_meta_words(const PathType& file_path)
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

    return run_sync(cmd);
};

//------------------------------------------------------------------------
void transform_to_seconds(MetaWords& meta_words)
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
    if (this->audio_buffers.size() > 0)
        return;

    this->audio_buffers.resize(this->getChannelCount());
    for (auto& ch : audio_buffers)
        ch.resize(this->getSampleCount());

    auto tmp_dir = std::filesystem::temp_directory_path();
    tmp_dir /= PLUGIN_IDENTIFIER;
    std::filesystem::create_directories(tmp_dir);
    tmp_dir /= PathType(this->getName());

    read_audio_from_host(*this);

    auto path = PathType{tmp_dir};
    write_audio_to_file(*this, path);

    meta_words = process_audio_with_meta_words(path);
    transform_to_seconds(meta_words);
}

//------------------------------------------------------------------------
const float*
AudioSource::getRenderSampleCacheForChannel(ARA::ARAChannelCount channel) const
{
    return _sampleCache.data() +
           static_cast<size_t>(channel * getSampleCount());
}

//------------------------------------------------------------------------
void AudioSource::destroyRenderSampleCache()
{
    _sampleCache.clear();
    _sampleCache.resize(0);
}

//------------------------------------------------------------------------
const AudioSource::MetaWords& AudioSource::get_meta_words() const
{
    return meta_words;
}

//------------------------------------------------------------------------
} // namespace mam::meta_words
