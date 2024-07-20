//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace mam {
//------------------------------------------------------------------------
static const Steinberg::FUID
    kWordifyProcessorUID(0x76FA7B01, 0x4D2757B4, 0x9BA55204, 0x681B0F2C);
static const Steinberg::FUID
    kWordifyControllerUID(0xB7046436, 0x1AFA58A3, 0x99C5BA83, 0x7ED9EC1F);

#define WordifyVST3Category "OnlyARA"

//------------------------------------------------------------------------
} // namespace mam
