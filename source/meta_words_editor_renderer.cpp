//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "meta_words_editor_renderer.h"

namespace mam::meta_words {

//------------------------------------------------------------------------
EditorRenderer::EditorRenderer(
    ARA::PlugIn::DocumentController* document_controller) noexcept
: ARA::PlugIn::EditorRenderer(document_controller)
{
}

//------------------------------------------------------------------------
} // namespace mam::meta_words