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
bool sort(ARADocumentController& controller,
          ARADocumentController::PlaybackRegionOrderedIds& ids)
{
    using Id = meta_words::PlaybackRegion::Id;

    bool has_been_reorderd = false;
    auto sorter = [&](const Id id0, const Id id1) {
        const auto pbr0 = controller.find_playback_region(id0);
        const auto pbr1 = controller.find_playback_region(id1);
        if (!pbr0 || !pbr1)
            return false;

        bool result = pbr0.value()->getStartInPlaybackTime() <
            pbr1.value()->getStartInPlaybackTime();

        has_been_reorderd |= result;

        return result;
    };

    std::sort(ids.begin(), ids.end(), sorter);
    return has_been_reorderd;
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
    return new AudioSource(document, hostRef,
                           [this]() { this->notify_listeners({}); });
}

//------------------------------------------------------------------------
ARA::PlugIn::AudioModification*
ARADocumentController::doCreateAudioModification(
    ARA::PlugIn::AudioSource* audioSource,
    ARA::ARAAudioModificationHostRef hostRef,
    const ARA::PlugIn::AudioModification* optionalModificationToClone) noexcept
{
    return new AudioModification(audioSource, hostRef,
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
    return new PlaybackRegion(modification, hostRef);
}

//------------------------------------------------------------------------
void ARADocumentController::didUpdatePlaybackRegionProperties(
    ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    ARA::PlugIn::DocumentController::didUpdatePlaybackRegionProperties(
        playbackRegion);

    if (sort(*this, playback_region_ids_ordered))
        playback_region_order_subject.notify_listeners({});

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
    return new PlaybackRenderer(this);
}

//------------------------------------------------------------------------
bool ARADocumentController::rendererWillAccessModelGraph(
    PlaybackRenderer* /*playbackRenderer*/) noexcept
{
    ++_countOfRenderersCurrentlyAccessingModelGraph;
    return _renderersCanAccessModelGraph;
}

//------------------------------------------------------------------------
void ARADocumentController::rendererDidAccessModelGraph(
    PlaybackRenderer* /*playbackRenderer*/) noexcept
{
    ARA_INTERNAL_ASSERT(_countOfRenderersCurrentlyAccessingModelGraph > 0);
    --_countOfRenderersCurrentlyAccessingModelGraph;
}

//------------------------------------------------------------------------
void ARADocumentController::didAddPlaybackRegionToRegionSequence(
    ARA::PlugIn::RegionSequence* regionSequence,
    ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    auto* pbr = dynamic_cast<PlaybackRegion*>(playbackRegion);
    if (!pbr)
        return;

    playback_regions.insert({pbr->get_id(), pbr});
    playback_region_ids_ordered.push_back(pbr->get_id());
    sort(*this, playback_region_ids_ordered);

    playback_region_lifetimes_subject.notify_listeners(
        {PlaybackRegionLifetimeData::Event::HasBeenAdded, pbr->get_id()});
}

//------------------------------------------------------------------------
void ARADocumentController::willRemovePlaybackRegionFromRegionSequence(
    ARA::PlugIn::RegionSequence* regionSequence,
    ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    auto* pbr = dynamic_cast<meta_words::PlaybackRegion*>(playbackRegion);
    if (!pbr)
        return;

    playback_region_lifetimes_subject.notify_listeners(
        {PlaybackRegionLifetimeData::Event::WillBeRemoved, pbr->get_id()});

    auto iter = std::remove(playback_region_ids_ordered.begin(),
                            playback_region_ids_ordered.end(), pbr->get_id());
    playback_region_ids_ordered.erase(iter, playback_region_ids_ordered.end());
    playback_regions.erase(pbr->get_id());
}

//------------------------------------------------------------------------
auto ARADocumentController::find_playback_region(PlaybackRegion::Id id) const
    -> OptPlaybackRegionPtr
{
    auto iter = playback_regions.find(id);
    if (iter == playback_regions.end())
        return std::nullopt;

    return (*iter).second;
}

//------------------------------------------------------------------------
void ARADocumentController::onRequestLocatorPosChanged(double pos)
{
    auto hostPBCtrl = getHostPlaybackController();
    if (hostPBCtrl)
        hostPBCtrl->requestSetPlaybackPosition(ARA::ARATimePosition{pos});
}

//------------------------------------------------------------------------
auto ARADocumentController::register_playback_region_lifetimes_observer(
    PlaybackRegionLifetimesSubject::Callback&& callback) -> ObserverID
{
    return playback_region_lifetimes_subject.add_listener(std::move(callback));
}

//------------------------------------------------------------------------
auto ARADocumentController::unregister_playback_region_lifetimes_observer(
    ObserverID id) -> bool
{
    return playback_region_lifetimes_subject.remove_listener(id);
}

//------------------------------------------------------------------------
auto ARADocumentController::register_playback_region_changed_observer(
    const PlaybackRegion::Id playback_region_id, Subject::Callback&& callback)
    -> ObserverID
{
    auto& subject = playback_region_observers[playback_region_id];
    return subject.add_listener(std::move(callback));
}

//------------------------------------------------------------------------
auto ARADocumentController::unregister_playback_region_changed_observer(
    const PlaybackRegion::Id playback_region_id, ObserverID id)
{
    auto& subject = playback_region_observers[playback_region_id];
    return subject.remove_listener(id);
}

//------------------------------------------------------------------------
auto ARADocumentController::register_playback_region_order_observer(
    PlaybackRegionOrderSuject::Callback&& callback) -> ObserverID
{
    return this->playback_region_order_subject.add_listener(
        std::move(callback));
}

//------------------------------------------------------------------------
auto ARADocumentController::unregister_playback_region_order_observer(
    ObserverID id) -> void
{
    playback_region_order_subject.remove_listener(id);
}

//------------------------------------------------------------------------
} // namespace mam
