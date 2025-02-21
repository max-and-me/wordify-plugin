//------------------------------------------------------------------------
// Copyright (c) 2023-present, WordifyOrg.
//------------------------------------------------------------------------

#include "meta_words_editor_view.h"
#include "ara_document_controller.h"
#include "meta_words_playback_region.h"

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
    const auto& playback_regions =
        selection->getPlaybackRegions<PlaybackRegion>();
    if (playback_regions.empty())
        return;

    const auto& first_region = playback_regions.front();
    const auto region_id     = first_region->get_id();
    auto* controller         = getDocumentController<ARADocumentController>();
    if (controller)
        controller->on_region_selected_by_host(region_id);
}

//------------------------------------------------------------------------
} // namespace mam::meta_words