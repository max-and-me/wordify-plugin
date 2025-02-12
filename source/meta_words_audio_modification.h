//------------------------------------------------------------------------
// Copyright(c) 2025 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "mam/meta_words/meta_word.h"
#include "warn_cpp/suppress_warnings.h"
BEGIN_SUPPRESS_WARNINGS
#include "ARA_Library/PlugIn/ARAPlug.h"
END_SUPPRESS_WARNINGS

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