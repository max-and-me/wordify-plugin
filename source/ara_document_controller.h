//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ARA_Library/PlugIn/ARAPlug.h"
#include "meta_words_data.h"
#include "meta_words_playback_region.h"
#include "meta_words_playback_renderer.h"
#include "tiny_observer_pattern.h"

namespace mam {

//------------------------------------------------------------------------
// ARADocumentController
//------------------------------------------------------------------------
class ARADocumentController : public ARA::PlugIn::DocumentController,
                              public tiny_observer_pattern::SimpleSubject
{
public:
    //--------------------------------------------------------------------
    using MetaWordsDataList = std::vector<MetaWordsData>;

    using SampleRate      = double;
    using FnGetSampleRate = std::function<SampleRate()>;

    // publish inherited constructor
    using ARA::PlugIn::DocumentController::DocumentController;
    using Super = ARA::PlugIn::DocumentController;

    // getter for the companion API implementations
    static const ARA::ARAFactory* getARAFactory() noexcept;
    bool doRestoreObjectsFromArchive(
        ARA::PlugIn::HostArchiveReader* archiveReader,
        const ARA::PlugIn::RestoreObjectsFilter* filter) noexcept override;
    bool doStoreObjectsToArchive(
        ARA::PlugIn::HostArchiveWriter* archiveWriter,
        const ARA::PlugIn::StoreObjectsFilter* filter) noexcept override;
    void doUpdateMusicalContextContent(
        ARA::PlugIn::MusicalContext* musicalContext,
        const ARA::ARAContentTimeRange* range,
        ARA::ContentUpdateScopes scopeFlags) noexcept override;
    void doUpdateAudioSourceContent(
        ARA::PlugIn::AudioSource* audioSource,
        const ARA::ARAContentTimeRange* range,
        ARA::ContentUpdateScopes scopeFlags) noexcept override;
    void
    willEnableAudioSourceSamplesAccess(ARA::PlugIn::AudioSource* audioSource,
                                       bool enable) noexcept override;
    void
    didEnableAudioSourceSamplesAccess(ARA::PlugIn::AudioSource* audioSource,
                                      bool enable) noexcept override;
    ARA::PlugIn::AudioSource*
    doCreateAudioSource(ARA::PlugIn::Document* document,
                        ARA::ARAAudioSourceHostRef hostRef) noexcept override;
    void didUpdateAudioSourceProperties(
        ARA::PlugIn::AudioSource* audioSource) noexcept override;

    ARA::PlugIn::AudioModification* doCreateAudioModification(
        ARA::PlugIn::AudioSource* audioSource,
        ARA::ARAAudioModificationHostRef hostRef,
        const ARA::PlugIn::AudioModification*
            optionalModificationToClone) noexcept override;

    void didUpdateAudioModificationProperties(
        ARA::PlugIn::AudioModification* audioModification) noexcept override;

    ARA::PlugIn::PlaybackRegion* doCreatePlaybackRegion(
        ARA::PlugIn::AudioModification* modification,
        ARA::ARAPlaybackRegionHostRef hostRef) noexcept override;

    void didUpdatePlaybackRegionProperties(
        ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;

    ARA::PlugIn::EditorView* doCreateEditorView() noexcept override;
    ARA::PlugIn::EditorRenderer* doCreateEditorRenderer() noexcept override;

    const MetaWordsDataList
    collect_meta_data_words(ARA::ARASampleRate playback_sample_rate) const;

    const meta_words::PlaybackRegion::AudioBufferSpan
    collect_region_channel_buffer(
        ARA::ARASampleRate playback_sample_rate) const;

    void onRequestLocatorPosChanged(double pos);

    ARA::PlugIn::PlaybackRenderer* doCreatePlaybackRenderer() noexcept override;

    // Render thread synchronization:
    // This is just a test code implementation of handling the threading -
    // proper code will use a more sophisticated threading implementation, which
    // is needed regardless of ARA. The test code simply blocks renderer access
    // to the model while it is being modified. This includes waiting until
    // concurrent renderer model access has completed before starting
    // modifications.
    bool rendererWillAccessModelGraph(
        meta_words::PlaybackRenderer* playbackRenderer) noexcept;
    void rendererDidAccessModelGraph(
        meta_words::PlaybackRenderer* playbackRenderer) noexcept;

    template <typename Func>
    void for_each_playback_region(Func& func)
    {
        ARADocumentController::MetaWordsDataList meta_words_data_list;
        if (auto* document = getDocument())
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
                    for (const auto* playback_region : playback_regions)
                    {
                        func(playback_region);
                    }
                }
            }
        }
    }

    tiny_observer_pattern::SimpleSubject&
    get_subject(const meta_words::PlaybackRegion* playback_region);
    //--------------------------------------------------------------------
protected:
    using PlaybackRegionObservers =
        std::unordered_map<const meta_words::PlaybackRegion*,
                           tiny_observer_pattern::SimpleSubject>;
    PlaybackRegionObservers playback_region_observers;

    std::atomic<bool> _renderersCanAccessModelGraph{true};
    std::atomic<int> _countOfRenderersCurrentlyAccessingModelGraph{0};
};

//------------------------------------------------------------------------
} // namespace mam
