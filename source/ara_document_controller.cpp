//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#include "ara_document_controller.h"
#include "ara_factory_config.h"

namespace mam {

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
}

//------------------------------------------------------------------------
} // namespace mam