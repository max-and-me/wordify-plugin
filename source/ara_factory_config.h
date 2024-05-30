// Copyright(c) 2023 Max And Me.

#pragma once

#include "ARA_Library/PlugIn/ARAPlug.h"

namespace mam {

//--------------------------------------------------------------------
// ARAFactoryConfig
//--------------------------------------------------------------------
class ARAFactoryConfig : public ARA::PlugIn::FactoryConfig
{
public:
    //----------------------------------------------------------------
    const char* getFactoryID() const noexcept override;
    const char* getPlugInName() const noexcept override;
    const char* getManufacturerName() const noexcept override;
    const char* getInformationURL() const noexcept override;
    const char* getVersion() const noexcept override;
    const char* getDocumentArchiveID() const noexcept override;
    //----------------------------------------------------------------
};

//--------------------------------------------------------------------
} // namespace mam