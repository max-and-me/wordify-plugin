//------------------------------------------------------------------------
// Copyright(c) 2025 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "warn_cpp/suppress_warnings.h"
BEGIN_SUPPRESS_WARNINGS
#include "ARA_Library/PlugIn/ARAPlug.h"
END_SUPPRESS_WARNINGS

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
