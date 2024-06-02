// Copyright(c) 2024 Max And Me.

#include "ara_factory_config.h"
#include "projectversion.h"
#include "wordify_defines.h"

#define WORDIFY_FACTORY_ID "org.wordify.plugin.arafactory"
#define WORDIFY_DOCUMENT_ARCHIVE_ID                                            \
    "org.wordify.plugin.aradocumentarchive.version1"
#define WORDIFY_FILECHUNK_ARCHIVE_ID                                           \
    "org.wordify.plugin.arafilechunkarchive.version1"

namespace mam {

//------------------------------------------------------------------------
// ARAFactoryConfig
//------------------------------------------------------------------------
const char* ARAFactoryConfig::getFactoryID() const noexcept
{
    return WORDIFY_FACTORY_ID;
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
    return VERSION_STR;
}

//--------------------------------------------------------------------
const char* ARAFactoryConfig::getDocumentArchiveID() const noexcept
{
    return WORDIFY_DOCUMENT_ARCHIVE_ID;
}

//--------------------------------------------------------------------
} // namespace mam
