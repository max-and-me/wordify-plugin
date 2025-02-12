//------------------------------------------------------------------------
// Copyright(c) 2025 Max And Me.
//------------------------------------------------------------------------

#include "meta_words_audio_modification.h"
#include "ARA_Library/PlugIn/ARAPlug.h"

namespace mam::meta_words {
//------------------------------------------------------------------------
AudioModification::AudioModification(
    ARA::PlugIn::AudioSource* audioSource,
    ARA::ARAAudioModificationHostRef hostRef,
    const ARA::PlugIn::AudioModification* optionalModificationToClone) noexcept
: ARA::PlugIn::AudioModification(
      audioSource, hostRef, optionalModificationToClone)
{
}
//------------------------------------------------------------------------
} // namespace mam::meta_words