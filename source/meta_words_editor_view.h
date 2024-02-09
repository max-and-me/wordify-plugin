//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ARA_Library/PlugIn/ARAPlug.h"

namespace mam::meta_words {

//------------------------------------------------------------------------
class EditorView : public ARA::PlugIn::EditorView
{
public:
    //--------------------------------------------------------------------
    explicit EditorView(
        ARA::PlugIn::DocumentController* document_controller) noexcept;
    //--------------------------------------------------------------------
private:
    void doNotifySelection(
        const ARA::PlugIn::ViewSelection* selection) noexcept override;
};

//------------------------------------------------------------------------
} // namespace mam::meta_words