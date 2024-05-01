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

    CRect bounds        = getViewSize();
    const CPoint center = bounds.getCenter();

    context->setDrawMode(kAntiAliasing);
    context->setLineWidth(2.0);

    constexpr int numLines              = 12;
    constexpr VSTGUI::CCoord lineLength = 5.0;
    constexpr CColor lineColor(150, 150, 150);
    constexpr VSTGUI::CCoord factor = 0.2;

    context->setFrameColor(lineColor);
    context->setFillColor(lineColor);

    for (int i = 0; i < numLines; ++i)
    {
        const VSTGUI::CCoord angle =
            (VSTGUI::CCoord(i) * 30.0 + rotationAngle) * (kPI / 180.0);
        const CPoint start(center.x + cos(angle) * (bounds.getWidth() * factor),
                           center.y +
                               sin(angle) * (bounds.getHeight() * factor));
        const CPoint end(start.x + cos(angle) * lineLength,
                         start.y + sin(angle) * lineLength);

        context->drawLine(start, end);
    }
}

//------------------------------------------------------------------------
// SpinAnimation
//------------------------------------------------------------------------
const char* SpinAnimation::ANIMATION_ID = "SpinAnimation";
SpinAnimation::SpinAnimation() {}

//------------------------------------------------------------------------
void SpinAnimation::animationStart(VSTGUI::CView* view,
                                   VSTGUI::IdStringPtr name)
{
    SpinnerView* spinner_view = dynamic_cast<SpinnerView*>(view);
    if (!spinner_view)
        return;

    if (VSTGUI::UTF8String(name) == ANIMATION_ID)
    {
        spinner_view->set_dregree(start_value);
    }
}

//------------------------------------------------------------------------
void SpinAnimation::animationTick(VSTGUI::CView* view,
                                  VSTGUI::IdStringPtr name,
                                  float pos)
{
    SpinnerView* spinner_view = dynamic_cast<SpinnerView*>(view);
    if (!spinner_view)
        return;

    if (VSTGUI::UTF8String(name) == ANIMATION_ID)
    {
        const auto current_degree = (end_value - start_value) * pos;
        spinner_view->set_dregree(current_degree);
    }
}

//------------------------------------------------------------------------
void SpinAnimation::animationFinished(VSTGUI::CView* view,
                                      VSTGUI::IdStringPtr name,
                                      bool wasCanceled)
{
    SpinnerView* spinner_view = dynamic_cast<SpinnerView*>(view);
    if (!spinner_view)
        return;

    if (VSTGUI::UTF8String(name) == ANIMATION_ID)
    {
        spinner_view->set_dregree(end_value);
    }
}

//------------------------------------------------------------------------
} // namespace mam
