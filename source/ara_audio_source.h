
//------------------------------------------------------------------------------
//! \file       ARATestAudioSource.h
//!             audio source implementation for the ARA test plug-in,
//!             customizing the audio source base class of the ARA library
//! \project    ARA SDK Examples
//! \copyright  Copyright (c) 2012-2023, Celemony Software GmbH, All Rights Reserved.
//! \license    Licensed under the Apache License, Version 2.0 (the "License");
//!             you may not use this file except in compliance with the License.
//!             You may obtain a copy of the License at
//!
//!               http://www.apache.org/licenses/LICENSE-2.0
//!
//!             Unless required by applicable law or agreed to in writing, software
//!             distributed under the License is distributed on an "AS IS" BASIS,
//!             WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//!             See the License for the specific language governing permissions and
//!             limitations under the License.
//------------------------------------------------------------------------------

#pragma once

#include "ARA_Library/PlugIn/ARAPlug.h"

/*******************************************************************************/
class ARATestAudioSource : public ARA::PlugIn::AudioSource
{
public:
    ARATestAudioSource (ARA::PlugIn::Document* document, ARA::ARAAudioSourceHostRef hostRef)
    : AudioSource { document, hostRef }
    {}

   
    // render thread sample access:
    // in order to keep this test code as simple as possible, our test audio source uses brute
    // force and caches all samples in-memory so that renderers can access it without threading issues
    // the document controller triggers filling this cache on the main thread, immediately after access is enabled.
    // actual plug-ins will use a multi-threaded setup to only cache sections of the audio source on demand -
    // a sophisticated file I/O threading implementation is needed for file-based processing regardless of ARA.
    void updateRenderSampleCache ();
    const float* getRenderSampleCacheForChannel (ARA::ARAChannelCount channel) const;
    void destroyRenderSampleCache ();

protected:
    std::vector<float> _sampleCache;
};
