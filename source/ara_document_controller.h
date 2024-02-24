//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ARA_Library/PlugIn/ARAPlug.h"
#include "vstgpt_context.h"
#include <unordered_map>
#include <functional>

namespace mam {
//------------------------------------------------------------------------
// ARADocumentController
//------------------------------------------------------------------------
class ARADocumentController : public ARA::PlugIn::DocumentController
{
public:
    //--------------------------------------------------------------------
    using MetaWordsDataList = std::vector<MetaWordsData>;

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

    const MetaWordsDataList collect_meta_data_words() const;

    // Observer pattern
    using ObserverID = std::uint64_t;
    using Callback = std::function<void()>;
    auto add_listener(Callback&& cb) -> ObserverID;
    auto remove_listener(ObserverID observer_id) -> bool;

    void onRequestLocatorPosChanged(double pos);

    //--------------------------------------------------------------------
protected:
    void notify_all() const;

    ObserverID observer_id = 0;
    using Observers = std::unordered_map<ObserverID, Callback>;
    Observers observers;
};
//------------------------------------------------------------------------
} // namespace mam
