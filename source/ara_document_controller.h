//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ARA_Library/PlugIn/ARAPlug.h"

namespace mam {
//------------------------------------------------------------------------
// ARADocumentController
//------------------------------------------------------------------------
class ARADocumentController : public ARA::PlugIn::DocumentController
{
public:
    //--------------------------------------------------------------------
    // publish inherited constructor
    using ARA::PlugIn::DocumentController::DocumentController;

    // getter for the companion API implementations
    static const ARA::ARAFactory* getARAFactory() noexcept;
    //--------------------------------------------------------------------
protected:
};
//------------------------------------------------------------------------
} // namespace mam