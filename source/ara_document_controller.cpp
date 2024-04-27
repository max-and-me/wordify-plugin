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
#include "meta_words_serde.h"
#include "preferences_serde.h"

namespace mam {

//------------------------------------------------------------------------
static auto collect_meta_words_serde_dataset(
    const ARA::PlugIn::StoreObjectsFilter* filter,
    meta_words::serde::Archive& archive) -> meta_words::serde::Archive&
{
    using AudioSource = ARADocumentController::AudioSource;

    meta_words::serde::Archive meta_words_serde_dataset;
    auto audio_sources = filter->getAudioSourcesToStore<AudioSource>();
    for (const auto& as : audio_sources)
    {
        const auto persistent_id = as->getPersistentID();
        const auto mdw           = as->get_meta_words();

        archive.audio_sources.push_back({persistent_id, mdw});
    }

    return archive;
}

//------------------------------------------------------------------------
static auto
apply_meta_words_serde_dataset(const ARA::PlugIn::RestoreObjectsFilter* filter,
                               meta_words::serde::Archive& archive) -> void
{
    using AudioSource = ARADocumentController::AudioSource;

    for (const auto& el : archive.audio_sources)
    {
        auto audio_sources = filter->getAudioSourceToRestoreStateWithID(
            (ARA::ARAPersistentID)el.persistent_id.data());

        if (auto as = dynamic_cast<AudioSource*>(audio_sources))
        {
            as->set_meta_words(el.words);
        }
    }
}

//------------------------------------------------------------------------
template <typename Func>
static auto
for_each_playback_region_(const ARADocumentController::AudioSource& source,
                          Func&& func) -> void
{
    const auto& sources = source.getAudioModifications();
    for (const auto& source : sources)
    {
        const auto& regions =
            source->getPlaybackRegions<ARADocumentController::PlaybackRegion>();
        for (const auto& region : regions)
        {
            if (!func(*region))
                return;
        }
    }
}

//------------------------------------------------------------------------
// ARADocumentController
//------------------------------------------------------------------------
const ARA::ARAFactory* ARADocumentController::getARAFactory() noexcept
{
    return ARA::PlugIn::PlugInEntry::getPlugInEntry<ARAFactoryConfig,
                                                    ARADocumentController>()
        ->getFactory();
}

//------------------------------------------------------------------------
ARADocumentController::ARADocumentController(
    const ARA::PlugIn::PlugInEntry* entry,
    const ARA::ARADocumentControllerHostInstance* instance) noexcept
: ARA::PlugIn::DocumentController(entry, instance)
{
    region_order_manager.initialize([this](PlaybackRegion::Id id) {
        const auto opt_region = find_playback_region(id);
        if (opt_region)
            return opt_region.value()->getStartInPlaybackTime();

        return 0.;
    });
}

//------------------------------------------------------------------------
bool ARADocumentController::doRestoreObjectsFromArchive(
    ARA::PlugIn::HostArchiveReader* archiveReader,
    const ARA::PlugIn::RestoreObjectsFilter* filter) noexcept
{
    // Retore archive
    const auto archive_size = archiveReader->getArchiveSize();
    std::string deserialized;
    deserialized.resize(archive_size);
    bool result = archiveReader->readBytesFromArchive(
        0, archive_size, (ARA::ARAByte*)deserialized.data());

    meta_words::serde::Archive archive;
    meta_words::serde::deserialize(deserialized, archive);
    apply_meta_words_serde_dataset(filter, archive);
    return result;
}

//------------------------------------------------------------------------
bool ARADocumentController::doStoreObjectsToArchive(
    ARA::PlugIn::HostArchiveWriter* archiveWriter,
    const ARA::PlugIn::StoreObjectsFilter* filter) noexcept
{
    // Store archive
    meta_words::serde::Archive archive;
    archive = collect_meta_words_serde_dataset(filter, archive);

    std::string serialized;
    meta_words::serde::serialize(archive, serialized);
    return archiveWriter->writeBytesToArchive(0, serialized.length(),
                                              (ARA::ARAByte*)serialized.data());
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
    if (auto as = dynamic_cast<AudioSource*>(audioSource))
    {
        if (scopeFlags.affectSamples() && as->isSampleAccessEnabled())
            as->updateRenderSampleCache();
    }
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
    auto as = dynamic_cast<meta_words::AudioSource*>(audioSource);

    if (as == nullptr)
        return;

    if (enable)
        as->updateRenderSampleCache();
}

//------------------------------------------------------------------------
ARA::PlugIn::AudioSource* ARADocumentController::doCreateAudioSource(
    ARA::PlugIn::Document* document,
    ARA::ARAAudioSourceHostRef hostRef) noexcept
{

    static AudioSource::Identifier id = 0;

    auto* new_audio_source = new AudioSource(
        document, hostRef,
        [this](const AudioSource& source, bool state) {
            this->on_word_analysis_progress(source, state);
        },
        [this](const AudioSource& source, double progress) {
            this->on_word_analysis_progress(source, progress);
        },
        id);
    id++;

    if (!new_audio_source)
        return nullptr;

    new_audio_source->changed_func = [this](AudioSource* audio_source) {
        if (!audio_source)
            return;

        auto audio_modifications = audio_source->getAudioModifications();
        for (auto& am : audio_modifications)
        {
            auto regions = am->getPlaybackRegions<PlaybackRegion>();
            for (auto& region : regions)
            {
                auto obj = playback_region_observers.find(region->get_id());
                if (obj != playback_region_observers.end())
                    obj->second.notify_listeners({});
            }
        }
        this->notify_listeners({});
    };

    return new_audio_source;
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
    auto* region = new PlaybackRegion(modification, hostRef);

    // Do stuff!
    this->on_add_playback_region(region);

    return region;
}

//------------------------------------------------------------------------

void ARADocumentController::doDestroyPlaybackRegion(
    ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    // Do stuff
    auto* pbr = dynamic_cast<PlaybackRegion*>(playbackRegion);
    if (pbr)
        this->on_remove_playback_region(pbr->get_id());

    ARA::PlugIn::DocumentController::doDestroyPlaybackRegion(playbackRegion);
}

//------------------------------------------------------------------------
void ARADocumentController::didUpdatePlaybackRegionProperties(
    ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    ARA::PlugIn::DocumentController::didUpdatePlaybackRegionProperties(
        playbackRegion);

    region_order_manager.reorder();

    this->notify_listeners({});
    if (auto* pbr = dynamic_cast<PlaybackRegion*>(playbackRegion))
    {
        auto obj = playback_region_observers.find(pbr->get_id());
        if (obj != playback_region_observers.end())
            obj->second.notify_listeners({});
    }
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

    // on_add_playback_region(pbr);
}

//------------------------------------------------------------------------
void ARADocumentController::willRemovePlaybackRegionFromRegionSequence(
    ARA::PlugIn::RegionSequence* regionSequence,
    ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    auto* pbr = dynamic_cast<meta_words::PlaybackRegion*>(playbackRegion);
    if (!pbr)
        return;

    // on_remove_playback_region(pbr->get_id());
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
auto ARADocumentController::register_word_analysis_progress_observer(
    AnalysisProgressSubject::Callback&& callback) -> ObserverID
{
    return word_analysis_progress_subject.add_listener(std::move(callback));
}

//------------------------------------------------------------------------
auto ARADocumentController::unregister_word_analysis_progress_observer(
    ObserverID id) -> bool
{
    return word_analysis_progress_subject.remove_listener(id);
}

//------------------------------------------------------------------------
auto ARADocumentController::register_playback_region_changed_observer(
    const PlaybackRegion::Id playback_region_id,
    Subject::Callback&& callback) -> ObserverID
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
void ARADocumentController::on_add_playback_region(PlaybackRegion* region)
{
    playback_regions.insert({region->get_id(), region});
    region_order_manager.push_back(region->get_id());

    playback_region_lifetimes_subject.notify_listeners(
        {PlaybackRegionLifetimeData::Event::HasBeenAdded, region->get_id()});
}

//------------------------------------------------------------------------
void ARADocumentController::on_remove_playback_region(PlaybackRegion::Id id)
{
    playback_region_lifetimes_subject.notify_listeners(
        {PlaybackRegionLifetimeData::Event::WillBeRemoved, id});

    region_order_manager.remove(id);
    playback_regions.erase(id);
}

//------------------------------------------------------------------------
void ARADocumentController::on_word_analysis_progress(const AudioSource& source,
                                                      bool state)
{
    WordAnalysisProgressData data{
        source.getIdentifier(), -1.,
        state ? WordAnalysisProgressData::State::kAnalysisStarted
              : WordAnalysisProgressData::State::kAnalysisStopped};

    word_analysis_progress_subject.notify_listeners(data);

    const auto func = [&](const PlaybackRegion& region) -> bool {
        auto obj = playback_region_observers.find(region.get_id());
        if (obj != playback_region_observers.end())
            obj->second.notify_listeners({});

        return true;
    };
    for_each_playback_region_(source, std::move(func));
}

//------------------------------------------------------------------------
void ARADocumentController::on_word_analysis_progress(const AudioSource& source,
                                                      double progress)
{
    WordAnalysisProgressData data{
        source.getIdentifier(), progress,
        WordAnalysisProgressData::State::kAnalysisRunning};

    word_analysis_progress_subject.notify_listeners(data);
}

//------------------------------------------------------------------------
auto ARADocumentController::get_region_selection_model()
    -> RegionSelectionModel&
{
    if (!region_selection_model.on_select_func)
    {
        region_selection_model.on_select_func =
            [&](const RegionSelectionModel::DataType&) {
                // TODO: a bit overkill here to notify all observers without
                // context. But hey, it s trial and error ;) At least observers
                // should know that it is the 'selection' which changed.
                this->notify_listeners({});
            };
    }

    return region_selection_model;
}

//------------------------------------------------------------------------
} // namespace mam
