// Copyright (c) 2023-present, WordifyOrg.

#pragma once

namespace VSTGUI {
class CView;
}

namespace mam::animations {

//------------------------------------------------------------------------
using View = VSTGUI::CView;

auto add_simple_fade_in(View* view) -> void;
auto add_simple_fade_out(View* view) -> void;

//------------------------------------------------------------------------
} // namespace mam::animations