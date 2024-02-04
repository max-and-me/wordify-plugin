//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ARA_Library/PlugIn/ARAPlug.h"
#include "mam/meta_words/meta_word.h"

namespace mam::meta_words {

//------------------------------------------------------------------------
class AudioModification : public ARA::PlugIn::AudioModification
{
public:
    //--------------------------------------------------------------------
    AudioModification(ARA::PlugIn::AudioSource* audioSource,
                      ARA::ARAAudioModificationHostRef hostRef,
                      const ARA::PlugIn::AudioModification*
                          optionalModificationToClone) noexcept;
    //--------------------------------------------------------------------
private:
};

//------------------------------------------------------------------------
} // namespace mam::meta_words