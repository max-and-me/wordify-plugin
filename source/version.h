//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/fplatform.h"

// Plain project version file generated by cmake
#include "projectversion.h"

#define stringOriginalFilename PROJECT_NAME ".vst3"
#if SMTG_PLATFORM_64
#define stringFileDescription PROJECT_NAME " VST3 (64Bit)"
#else
#define stringFileDescription PROJECT_NAME " VST3"
#endif
#define stringCompanyName CPACK_PACKAGE_VENDOR "\0"
#define stringLegalCopyright PROJECT_LEGAL_COPYRIGHT
#define stringLegalTrademarks                                                  \
    "VST is a trademark of Steinberg Media Technologies GmbH"
