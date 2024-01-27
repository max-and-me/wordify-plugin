//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ARA_Library/PlugIn/ARAPlug.h"
#include "vstgpt_context.h"

namespace mam {
//------------------------------------------------------------------------
// ARADocumentController
//------------------------------------------------------------------------
class ARADocumentController : public ARA::PlugIn::DocumentController, public mam::ContextListenerAdapter
{
public:
    //--------------------------------------------------------------------
    // publish inherited constructor
    using ARA::PlugIn::DocumentController::DocumentController;
    using Super = ARA::PlugIn::DocumentController;
    
    virtual ~ARADocumentController ()
    {
        VstGPTContext* context = VstGPTContext::getInstance ();
        context->unregisterContextListener (this);
    };

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
    
    void onRequestLocatorPosChanged (double pos) override;

    //--------------------------------------------------------------------
protected:
};
//------------------------------------------------------------------------
} // namespace mam
