//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "spinner_view.h"
#include "vstgui/lib/cdrawcontext.h"

using namespace VSTGUI;

namespace mam {

#ifndef kPI
#define kPI 3.14159265358979323846
#endif

//------------------------------------------------------------------------
SpinnerView::SpinnerView(const CRect& size)
: CView(size)
{
    setWantsIdle(true);
}

//------------------------------------------------------------------------
void SpinnerView::onIdle()
{
    rotationAngle += 5.0f;
    if (rotationAngle >= 360.0f)
        rotationAngle -= 360.0f;
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

    constexpr int numLines        = 12;
    constexpr float lineLength    = 10.0;
    constexpr float lineThickness = 2.0;
    constexpr CColor lineColor(255, 255, 255);

    context->setFrameColor(lineColor);
    context->setFillColor(lineColor);

    for (int i = 0; i < numLines; ++i)
    {
        const float angle = (i * 30.0f + rotationAngle) * (kPI / 180.0f);
        const CPoint start(
            center.x + cos(angle) * (bounds.getWidth() * 0.3f),
            center.y + sin(angle) * (bounds.getHeight() * 0.3f));
        const CPoint end(start.x + cos(angle) * lineLength,
                            start.y + sin(angle) * lineLength);
        context->drawLine(start, end);
    }
}

//------------------------------------------------------------------------
} // namespace mam
