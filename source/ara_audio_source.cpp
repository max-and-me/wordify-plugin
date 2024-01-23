//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#include "ara_audio_source.h"

#include "sndfile.h"
#include "vstgpt_defines.h"
#include <cmath>
#include <filesystem>
#include <functional>
#include "mam/meta_words/runner.h"
#include <iostream>


namespace mam {
namespace {

//------------------------------------------------------------------------
using PathType     = const std::string;
using DataPointers = std::vector<void*>;
enum class ChannelIndex : int
{
    kLeft,
    kRight
};
using FnSampleValue =
    std::function<ARATestAudioSource::SampleType(ChannelIndex, int)>;

//------------------------------------------------------------------------
DataPointers
create_data_pointers(ARATestAudioSource::MultiChannelBufferType& audio_buffers)
{
    DataPointers data_pointers;
    data_pointers.reserve(audio_buffers.size());
    for (auto ch = 0; ch < audio_buffers.size(); ++ch)
        data_pointers.push_back(static_cast<void*>(audio_buffers[ch].data()));

    return data_pointers;
}

//------------------------------------------------------------------------
void read_audio_from_host(ARATestAudioSource& audio_src)
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
    using SampleType  = ARATestAudioSource::SampleType;
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
void write_audio_to_file(ARATestAudioSource& audio_src,
                         const PathType& file_path)
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
meta_words::MetaWords run_sync(mam::meta_words::Command& cmd)
{
    std::cout << "Run sync..." << '\n';

    mam::meta_words::FnProgress fn_progress = [](double val) { double tmp = val; };

    return (run(cmd, fn_progress));
}

//------------------------------------------------------------------------
mam::meta_words::MetaWords process_audio_with_meta_words (const PathType& file_path)
{
    using namespace mam::meta_words;
    
    // whisper.cpp executable
    const PathType executable =
        "/Users/max/Documents/Code/vst-gpt/build_whisper/bin/"
        "main";
    //  The whisper.cpp library takes the audio file and writes the result
    //  of its analysis into a CSV file. The file is named like the audio
    //  file and by prepending ".csv" e.g. my_speech.wav ->
    //  my_speech.wav.csv
    const Options options         = {"-ocsv"};
    const OneValArgs one_val_args = {
        // model file resp. binary
        {"-m", "/Users/max/Documents/Code/vst-gpt/whisper.cpp/models/ggml-base.en.bin"},
        // audio file to analyse
        {"-f", file_path},
        // maximum segment length in characters: "1" mains one word
        {"-ml", "1"}};

    Command cmd{executable, options, one_val_args};

    return run_sync(cmd);
};

//------------------------------------------------------------------------
} // namespace

//------------------------------------------------------------------------
void ARATestAudioSource::updateRenderSampleCache()
{
#if 1
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
   
    auto meta_words = process_audio_with_meta_words (path);
    for (auto& word : meta_words)
        std::cout << word.word;
    
#else
    ARA_INTERNAL_ASSERT(isSampleAccessEnabled());

    // set up cache (this is a hack, so we're ignoring potential overflow of
    // 32 bit with long files here...)
    const auto channelCount{static_cast<size_t>(getChannelCount())};
    const auto sampleCount{static_cast<size_t>(getSampleCount())};
    _sampleCache.resize(channelCount * sampleCount);

    // create temporary host audio reader and let it fill the cache
    // (we can safely ignore any errors while reading since host must clear
    // buffers in that case, as well as report the error to the user)
    ARA::PlugIn::HostAudioReader audioReader{this};
    std::vector<void*> dataPointers{channelCount};
    for (auto c{0U}; c < channelCount; ++c)
        dataPointers[c] = _sampleCache.data() + c * sampleCount;
    audioReader.readAudioSamples(
        0, static_cast<ARA::ARASampleCount>(sampleCount), dataPointers.data());

    std::vector<float*> dataPointers_f{channelCount};

    for (int i = 0; i < dataPointers.size(); i++)
    {
        dataPointers_f[i] = static_cast<float*>(dataPointers[i]);
    }
#endif
}

//------------------------------------------------------------------------
const float* ARATestAudioSource::getRenderSampleCacheForChannel(
    ARA::ARAChannelCount channel) const
{
    return _sampleCache.data() +
           static_cast<size_t>(channel * getSampleCount());
}

//------------------------------------------------------------------------
void ARATestAudioSource::destroyRenderSampleCache()
{
    _sampleCache.clear();
    _sampleCache.resize(0);
}

//------------------------------------------------------------------------
} // namespace mam
