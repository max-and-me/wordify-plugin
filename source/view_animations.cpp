// Copyright(c) 2024 Max And Me.

#include "view_animations.h"
#include "supress_warnings.h"
BEGIN_SUPRESS_WARNINGS
#include "vstgui/lib/animation/animations.h"
#include "vstgui/lib/animation/timingfunctions.h"
#include "vstgui/lib/cview.h"
END_SUPRESS_WARNINGS

namespace mam::animations {

//------------------------------------------------------------------------
constexpr auto DUR = 300;

//------------------------------------------------------------------------
auto add_simple_fade_in(View* view) -> void
{
    namespace Anim = VSTGUI::Animation;

    if (view)
    {
        view->addAnimation("AlphaAnimation",
                           new Anim::AlphaValueAnimation(1.f, true),
                           new Anim::CubicBezierTimingFunction(
                               Anim::CubicBezierTimingFunction::easyIn(DUR)));
    }
}

//------------------------------------------------------------------------

auto add_simple_fade_out(View* view) -> void
{
    namespace Anim = VSTGUI::Animation;

    if (view)
    {
        view->addAnimation(
            "AlphaAnimation", new Anim::AlphaValueAnimation(0.f, true),
            new Anim::CubicBezierTimingFunction(
                Anim::CubicBezierTimingFunction::easyOut(DUR * 2)));
    }
}

//------------------------------------------------------------------------
} // namespace mam::animations