//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#include "ara_factory_config.h"
#include "vstgpt_defines.h"

#define VSTGPT_VERSION_STRING "0.9.0"

#define VSTGPT_FACTORY_ID "org.maxandme.vstgptplugin.arafactory"
#define VSTGPT_DOCUMENT_ARCHIVE_ID                                             \
    "org.maxandme.vstgptplugin.aradocumentarchive.version1"
#define VSTGPT_FILECHUNK_ARCHIVE_ID                                            \
    "org.maxandme.vstgptplugin.arafilechunkarchive.version1"

namespace mam {

//------------------------------------------------------------------------
const char* ARAFactoryConfig::getFactoryID() const noexcept
{
    return VSTGPT_FACTORY_ID;
}

//--------------------------------------------------------------------
const char* ARAFactoryConfig::getPlugInName() const noexcept
{
    return PLUGIN_NAME_STR;
}

//--------------------------------------------------------------------
const char* ARAFactoryConfig::getManufacturerName() const noexcept
{
    return COMPANY_NAME_STR;
}

//--------------------------------------------------------------------
const char* ARAFactoryConfig::getInformationURL() const noexcept
{
    return COMPANY_URL_STR;
}

//--------------------------------------------------------------------
const char* ARAFactoryConfig::getVersion() const noexcept
{
    return VSTGPT_VERSION_STRING;
}

//--------------------------------------------------------------------
const char* ARAFactoryConfig::getDocumentArchiveID() const noexcept
{
    return VSTGPT_DOCUMENT_ARCHIVE_ID;
}

//--------------------------------------------------------------------
} // namespace mam