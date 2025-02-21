//------------------------------------------------------------------------
// Copyright (c) 2023-present, WordifyOrg.
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
auto EditorRenderer::update_project_time(Seconds time_) -> void
{
    this->time = time_;
}

//------------------------------------------------------------------------
} // namespace mam::meta_words