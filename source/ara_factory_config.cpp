//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#include "ara_factory_config.h"

#define TEST_PLUGIN_NAME "ARATestPlugIn"
#define TEST_MANUFACTURER_NAME "ARA SDK Examples"
#define TEST_INFORMATION_URL "https://www.ara-audio.org/examples"
#define TEST_MAILTO_URL "mailto:info@ara-audio.org?subject=ARATestPlugIn"
#define TEST_VERSION_STRING "0.9.0"

#define TEST_FACTORY_ID "org.ara-audio.examples.testplugin.arafactory"
#define TEST_DOCUMENT_ARCHIVE_ID                                               \
    "org.ara-audio.examples.testplugin.aradocumentarchive.version1"
#define TEST_FILECHUNK_ARCHIVE_ID                                              \
    "org.ara-audio.examples.testplugin.arafilechunkarchive.version1"

namespace mam {

//------------------------------------------------------------------------
const char* ARAFactoryConfig::getFactoryID() const noexcept
{
    return TEST_FACTORY_ID;
}

//--------------------------------------------------------------------
const char* ARAFactoryConfig::getPlugInName() const noexcept
{
    return TEST_PLUGIN_NAME;
}

//--------------------------------------------------------------------
const char* ARAFactoryConfig::getManufacturerName() const noexcept
{
    return TEST_MANUFACTURER_NAME;
}

//--------------------------------------------------------------------
const char* ARAFactoryConfig::getInformationURL() const noexcept
{
    return TEST_INFORMATION_URL;
}

//--------------------------------------------------------------------
const char* ARAFactoryConfig::getVersion() const noexcept
{
    return TEST_VERSION_STRING;
}

//--------------------------------------------------------------------
const char* ARAFactoryConfig::getDocumentArchiveID() const noexcept
{
    return TEST_DOCUMENT_ARCHIVE_ID;
}

//--------------------------------------------------------------------
} // namespace mam