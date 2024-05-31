//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "spinner_view.h"
#include "vstgui/lib/cdrawcontext.h"
#include <chrono>

using namespace VSTGUI;

namespace mam {

#ifndef kPI
#define kPI 3.14159265358979323846
#endif

//------------------------------------------------------------------------
SpinnerView::SpinnerView(const CRect& size)
: CView(size)
{
}

//------------------------------------------------------------------------
auto SpinnerView::set_dregree(Degree value) -> void
{
    rotationAngle = value;
    invalid();
}

//------------------------------------------------------------------------
void SpinnerView::draw(CDrawContext* context)
{
    CView::draw(context);

    const CRect bounds     = getViewSize();
    const CPoint center    = bounds.getCenter();
    const auto width_half  = bounds.getWidth() * 0.5;
    const auto height_half = bounds.getHeight() * 0.5;

    constexpr auto numLines  = 12;
    constexpr auto angleStep = 360. / numLines;

    constexpr CCoord kLineLength = 0.5; // normalized from 0. to 1.
    constexpr CCoord kLineWidth  = 2.0; // in pixel
    constexpr CColor lineColor(150, 150, 150);

    context->setDrawMode(kAntiAliasing);
    context->setLineWidth(kLineWidth);
    context->setFrameColor(lineColor);
    context->setFillColor(lineColor);

    LineList lines;
    for (int i = 0; i < numLines; ++i)
    {
        const CCoord angle =
            (CCoord(i) * angleStep + rotationAngle) * (kPI / 180.0);

        const auto width  = cos(angle) * width_half;
        const auto height = sin(angle) * height_half;
        const CPoint start(center.x + width, center.y + height);
        const CPoint end(center.x + width * (1. - kLineLength),
                         center.y + height * (1. - kLineLength));

        lines.push_back({start, end});
    }

    context->drawLines(lines);
}

//------------------------------------------------------------------------
// SpinAnimation
//------------------------------------------------------------------------
const char* SpinAnimation::ANIMATION_ID = "SpinAnimation";
SpinAnimation::SpinAnimation() {}

//------------------------------------------------------------------------
void SpinAnimation::animationStart(CView* view, IdStringPtr name)
{
    SpinnerView* spinner_view = dynamic_cast<SpinnerView*>(view);
    if (!spinner_view)
        return;

    if (UTF8String(name) == ANIMATION_ID)
    {
        spinner_view->set_dregree(start_value);
    }
}

//------------------------------------------------------------------------
void SpinAnimation::animationTick(CView* view, IdStringPtr name, float pos)
{
    SpinnerView* spinner_view = dynamic_cast<SpinnerView*>(view);
    if (!spinner_view)
        return;

    if (UTF8String(name) == ANIMATION_ID)
    {
        const auto current_degree = (end_value - start_value) * pos;
        spinner_view->set_dregree(current_degree);
    }
}

//------------------------------------------------------------------------
void SpinAnimation::animationFinished(CView* view,
                                      IdStringPtr name,
                                      bool wasCanceled)
{
    SpinnerView* spinner_view = dynamic_cast<SpinnerView*>(view);
    if (!spinner_view)
        return;

    if (UTF8String(name) == ANIMATION_ID)
    {
        spinner_view->set_dregree(end_value);
    }
}

//------------------------------------------------------------------------
} // namespace mam
