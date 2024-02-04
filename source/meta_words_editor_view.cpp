//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "meta_words_editor_view.h"

namespace mam::meta_words {

//------------------------------------------------------------------------
EditorView::EditorView(
    ARA::PlugIn::DocumentController* document_controller) noexcept
: ARA::PlugIn::EditorView(document_controller)
{
}

//--------------------------------------------------------------------
void EditorView::doNotifySelection(
    const ARA::PlugIn::ViewSelection* selection) noexcept
{
    const auto& playback_regions = selection->getPlaybackRegions();
}

//------------------------------------------------------------------------
} // namespace mam::meta_words