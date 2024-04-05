//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#include "ara_document_controller.h"
#include "ara_factory_config.h"
#include "meta_words_audio_modification.h"
#include "meta_words_audio_source.h"
#include "meta_words_editor_renderer.h"
#include "meta_words_editor_view.h"
#include "meta_words_playback_region.h"
#include "meta_words_playback_renderer.h"

namespace mam {

//------------------------------------------------------------------------
static ARADocumentController::MetaWordsDataList
collect_meta_data_words(const mam::ARADocumentController& document_controller,
                        ARA::ARASampleRate playback_sample_rate)
{
    ARADocumentController::MetaWordsDataList meta_words_data_list;
    if (auto* document = document_controller.getDocument())
    {
        const auto& audio_sources =
            document->getAudioSources<meta_words::AudioSource>();
        for (const auto& audio_source : audio_sources)
        {
            const auto& audio_modifications =
                audio_source->getAudioModifications();
            for (const auto audio_modification : audio_modifications)
            {
                const auto& playback_regions =
                    audio_modification
                        ->getPlaybackRegions<meta_words::PlaybackRegion>();
                for (const auto& playback_region : playback_regions)
                {
                    const auto data = playback_region->get_meta_words_data(
                        playback_sample_rate);
                    meta_words_data_list.emplace_back(data);
                }
            }
        }
    }

    return meta_words_data_list;
}

//------------------------------------------------------------------------
static auto
collect_audio_buffer(const mam::ARADocumentController& document_controller,
                     ARA::ARASampleRate playback_sample_rate)
    -> meta_words::PlaybackRegion::AudioBufferSpan
{
    ARADocumentController::MetaWordsDataList meta_words_data_list;
    if (auto* document = document_controller.getDocument())
    {
        const auto& audio_sources =
            document->getAudioSources<meta_words::AudioSource>();
        for (const auto& audio_source : audio_sources)
        {
            const auto& audio_modifications =
                audio_source->getAudioModifications();
            for (const auto audio_modification : audio_modifications)
            {
                const auto& playback_regions =
                    audio_modification
                        ->getPlaybackRegions<meta_words::PlaybackRegion>();
                for (const auto& playback_region : playback_regions)
                {
                    return playback_region->get_audio_buffer(
                        playback_sample_rate);
                }
            }
        }
    }

    return {};
}

//------------------------------------------------------------------------
const ARA::ARAFactory* ARADocumentController::getARAFactory() noexcept
{
    return ARA::PlugIn::PlugInEntry::getPlugInEntry<ARAFactoryConfig,
                                                    ARADocumentController>()
        ->getFactory();
}

//------------------------------------------------------------------------
bool ARADocumentController::doRestoreObjectsFromArchive(
    ARA::PlugIn::HostArchiveReader* archiveReader,
    const ARA::PlugIn::RestoreObjectsFilter* filter) noexcept
{
    return true;
}

//------------------------------------------------------------------------
bool ARADocumentController::doStoreObjectsToArchive(
    ARA::PlugIn::HostArchiveWriter* archiveWriter,
    const ARA::PlugIn::StoreObjectsFilter* filter) noexcept
{
    return true;
}

//------------------------------------------------------------------------
void ARADocumentController::doUpdateMusicalContextContent(
    ARA::PlugIn::MusicalContext* musicalContext,
    const ARA::ARAContentTimeRange* range,
    ARA::ContentUpdateScopes scopeFlags) noexcept
{
}

//------------------------------------------------------------------------
void ARADocumentController::doUpdateAudioSourceContent(
    ARA::PlugIn::AudioSource* audioSource,
    const ARA::ARAContentTimeRange* range,
    ARA::ContentUpdateScopes scopeFlags) noexcept
{
    // TODO
}

//------------------------------------------------------------------------
void ARADocumentController::willEnableAudioSourceSamplesAccess(
    ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept
{
}

//------------------------------------------------------------------------
void ARADocumentController::didEnableAudioSourceSamplesAccess(
    ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept
{
    auto testAudioSource = dynamic_cast<meta_words::AudioSource*>(audioSource);

    if (testAudioSource == nullptr)
        return;

    if (enable)
        testAudioSource->updateRenderSampleCache();
}

//------------------------------------------------------------------------
ARA::PlugIn::AudioSource* ARADocumentController::doCreateAudioSource(
    ARA::PlugIn::Document* document,
    ARA::ARAAudioSourceHostRef hostRef) noexcept
{
    return new meta_words::AudioSource(
        document, hostRef, [this]() { this->notify_listeners({}); });
}

//------------------------------------------------------------------------
ARA::PlugIn::AudioModification*
ARADocumentController::doCreateAudioModification(
    ARA::PlugIn::AudioSource* audioSource,
    ARA::ARAAudioModificationHostRef hostRef,
    const ARA::PlugIn::AudioModification* optionalModificationToClone) noexcept
{
    return new meta_words::AudioModification(audioSource, hostRef,
                                             optionalModificationToClone);
}

//------------------------------------------------------------------------
void ARADocumentController::didUpdateAudioModificationProperties(
    ARA::PlugIn::AudioModification* audioModification) noexcept
{
    ARA::PlugIn::DocumentController::didUpdateAudioModificationProperties(
        audioModification);

    this->notify_listeners({});
}

//------------------------------------------------------------------------
void ARADocumentController::didUpdateAudioSourceProperties(
    ARA::PlugIn::AudioSource* audioSource) noexcept
{
    // TODO: Trigger or schedule analysis here!
    /* From the ARA doc
        // create temporary host audio reader and let it fill the cache
        ARA::PlugIn::HostAudioReader audioReader { audio_src };
        std::vector<void*> dataPointers { channelCount };
        for (auto c { 0U }; c < channelCount; ++c)
            dataPointers[c] = _sampleCache.data () + c * sampleCount;
        audioReader.readAudioSamples (0, static_cast<ARA::ARASampleCount>
        (sampleCount), dataPointers.data ());
    */

    this->notify_listeners({});
}

//------------------------------------------------------------------------
ARA::PlugIn::PlaybackRegion* ARADocumentController::doCreatePlaybackRegion(
    ARA::PlugIn::AudioModification* modification,
    ARA::ARAPlaybackRegionHostRef hostRef) noexcept
{
    return new meta_words::PlaybackRegion(modification, hostRef);
}

//------------------------------------------------------------------------
void ARADocumentController::didUpdatePlaybackRegionProperties(
    ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    ARA::PlugIn::DocumentController::didUpdatePlaybackRegionProperties(
        playbackRegion);

    this->notify_listeners({});
    for (auto& o : playback_region_observers)
        o.second.notify_listeners({});
}

//------------------------------------------------------------------------
ARA::PlugIn::EditorView* ARADocumentController::doCreateEditorView() noexcept
{
    return new meta_words::EditorView(this);
}

//------------------------------------------------------------------------
ARA::PlugIn::EditorRenderer*
ARADocumentController::doCreateEditorRenderer() noexcept
{
    return new meta_words::EditorRenderer(this);
}

//------------------------------------------------------------------------
ARA::PlugIn::PlaybackRenderer*
ARADocumentController::doCreatePlaybackRenderer() noexcept
{
    return new meta_words::PlaybackRenderer(this);
}

//------------------------------------------------------------------------
bool ARADocumentController::rendererWillAccessModelGraph(
    meta_words::PlaybackRenderer* /*playbackRenderer*/) noexcept
{
    ++_countOfRenderersCurrentlyAccessingModelGraph;
    return _renderersCanAccessModelGraph;
}

//------------------------------------------------------------------------
void ARADocumentController::rendererDidAccessModelGraph(
    meta_words::PlaybackRenderer* /*playbackRenderer*/) noexcept
{
    ARA_INTERNAL_ASSERT(_countOfRenderersCurrentlyAccessingModelGraph > 0);
    --_countOfRenderersCurrentlyAccessingModelGraph;
}

//------------------------------------------------------------------------
const ARADocumentController::MetaWordsDataList
ARADocumentController::collect_meta_data_words(
    ARA::ARASampleRate playback_sample_rate) const
{
    return mam::collect_meta_data_words(*this, playback_sample_rate);
}

//------------------------------------------------------------------------
auto ARADocumentController::collect_region_channel_buffer(
    ARA::ARASampleRate playback_sample_rate) const
    -> const meta_words::PlaybackRegion::AudioBufferSpan
{
    return mam::collect_audio_buffer(*this, playback_sample_rate);
}

//------------------------------------------------------------------------
void ARADocumentController::onRequestLocatorPosChanged(double pos)
{
    auto hostPBCtrl = getHostPlaybackController();
    if (hostPBCtrl)
        hostPBCtrl->requestSetPlaybackPosition(ARA::ARATimePosition{pos});
}

//------------------------------------------------------------------------
tiny_observer_pattern::SimpleSubject& ARADocumentController::get_subject(
    const meta_words::PlaybackRegion* playback_region)
{
    return playback_region_observers[playback_region];
}

//------------------------------------------------------------------------
} // namespace mam
