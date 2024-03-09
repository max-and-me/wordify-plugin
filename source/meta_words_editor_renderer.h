//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ARA_Library/PlugIn/ARAPlug.h"

namespace mam::meta_words {

//------------------------------------------------------------------------
class EditorRenderer : public ARA::PlugIn::EditorRenderer
{
public:
    //--------------------------------------------------------------------
    explicit EditorRenderer(
        ARA::PlugIn::DocumentController* document_controller) noexcept;

    //--------------------------------------------------------------------
private:
};

//------------------------------------------------------------------------
} // namespace mam::meta_words