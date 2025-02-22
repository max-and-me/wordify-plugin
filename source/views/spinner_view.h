// Copyright (c) 2023-present, WordifyOrg.

#pragma once

#include "warn_cpp/suppress_warnings.h"
BEGIN_SUPPRESS_WARNINGS
#include "vstgui/lib/animation/ianimationtarget.h"
#include "vstgui/lib/cview.h"
END_SUPPRESS_WARNINGS

namespace mam {

//------------------------------------------------------------------------
// SpinnerView
//------------------------------------------------------------------------
class SpinnerView : public VSTGUI::CView
{
public:
    //--------------------------------------------------------------------
    using Degree = double;

    SpinnerView(const VSTGUI::CRect& size);

    auto set_animation_position(float position) -> void;
    void draw(VSTGUI::CDrawContext* context) override;

    //--------------------------------------------------------------------
private:
    float animation_position = 0.;
};

//------------------------------------------------------------------------
// SpinAnimation
//------------------------------------------------------------------------
class SpinAnimation : public VSTGUI::Animation::IAnimationTarget,
                      public VSTGUI::NonAtomicReferenceCounted
{
public:
    //--------------------------------------------------------------------
    static const char* ANIMATION_ID;

    SpinAnimation();

    void animationStart(VSTGUI::CView* view, VSTGUI::IdStringPtr name) override;
    void animationTick(VSTGUI::CView* view,
                       VSTGUI::IdStringPtr name,
                       float pos) override;
    void animationFinished(VSTGUI::CView* view,
                           VSTGUI::IdStringPtr name,
                           bool wasCanceled) override;
    //--------------------------------------------------------------------
private:
};

//------------------------------------------------------------------------
} // namespace mam
