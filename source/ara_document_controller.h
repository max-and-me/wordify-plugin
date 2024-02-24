//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ARA_Library/PlugIn/ARAPlug.h"
#include "vstgpt_context.h"
#include "tiny_observer_pattern.h"

namespace mam {

//------------------------------------------------------------------------
// ARADocumentController
//------------------------------------------------------------------------
class ARADocumentController : public ARA::PlugIn::DocumentController,
                              public tiny_observer_pattern::Subject
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

    void onRequestLocatorPosChanged(double pos);

    //--------------------------------------------------------------------
protected:
};

//------------------------------------------------------------------------
} // namespace mam
