//------------------------------------------------------------------------
// Copyright (c) 2023-present, WordifyOrg.
//------------------------------------------------------------------------

#pragma once

#include "warn_cpp/suppress_warnings.h"
BEGIN_SUPPRESS_WARNINGS
#include "ARA_Library/PlugIn/ARAPlug.h"
END_SUPPRESS_WARNINGS

namespace mam::meta_words {

//------------------------------------------------------------------------
class EditorRenderer : public ARA::PlugIn::EditorRenderer
{
public:
    //--------------------------------------------------------------------
    using Seconds = double;

    explicit EditorRenderer(
        ARA::PlugIn::DocumentController* document_controller) noexcept;

    auto update_project_time(Seconds time) -> void;
    //--------------------------------------------------------------------
private:
    Seconds time = 0.;
};

//------------------------------------------------------------------------
} // namespace mam::meta_words