//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#include "ara_document_controller.h"

namespace mam {

//------------------------------------------------------------------------
const ARA::ARAFactory* ARADocumentController::getARAFactory() noexcept
{
    return ARA::PlugIn::PlugInEntry::getPlugInEntry<ARATestFactoryConfig,
                                                    ARADocumentController>()
        ->getFactory();
}

//------------------------------------------------------------------------
} // namespace mam