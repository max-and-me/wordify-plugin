//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "vstgui/lib/animation/ianimationtarget.h"
#include "vstgui/lib/cview.h"

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

    auto set_dregree(Degree value) -> void;
    void draw(VSTGUI::CDrawContext* context) override;

    //--------------------------------------------------------------------
private:
    Degree rotationAngle = 0.;
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

protected:
    //--------------------------------------------------------------------

    using Degree             = SpinnerView::Degree;
    const Degree start_value = 0.;
    const Degree end_value   = 360.;
};

//------------------------------------------------------------------------
} // namespace mam
