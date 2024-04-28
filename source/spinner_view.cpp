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
static auto systme_time_milliseconds() -> size_t
{
    std::chrono::time_point<std::chrono::system_clock> now =
        std::chrono::system_clock::now();
    const auto duration = now.time_since_epoch();
    const auto millis =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    return millis;
}

//------------------------------------------------------------------------
template <typename Real>
static auto system_time_to_angle() -> Real
{
    constexpr auto FULL_ROTATION_TIME   = 4000; // 4000ms (4s)
    constexpr auto FULL_ROTATION_RATE   = Real(1.) / Real(FULL_ROTATION_TIME);
    constexpr auto FULL_ROTATION_DEGREE = Real(360.);

    const auto system_time_ms = systme_time_milliseconds();
    const auto normalized =
        float(system_time_ms % FULL_ROTATION_TIME) * FULL_ROTATION_RATE;

    return FULL_ROTATION_DEGREE * normalized;
}

//------------------------------------------------------------------------
SpinnerView::SpinnerView(const CRect& size)
: CView(size)
{
    setWantsIdle(true);
}

//------------------------------------------------------------------------
void SpinnerView::onIdle()
{
    rotationAngle = system_time_to_angle<const float>();
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

    constexpr int numLines     = 12;
    constexpr float lineLength = 5.0;
    constexpr CColor lineColor(150, 150, 150);
    constexpr float factor = 0.2f;

    context->setFrameColor(lineColor);
    context->setFillColor(lineColor);

    for (int i = 0; i < numLines; ++i)
    {
        const float angle = (i * 30.0f + rotationAngle) * (kPI / 180.0f);
        const CPoint start(center.x + cos(angle) * (bounds.getWidth() * factor),
                           center.y +
                               sin(angle) * (bounds.getHeight() * factor));
        const CPoint end(start.x + cos(angle) * lineLength,
                         start.y + sin(angle) * lineLength);
        context->drawLine(start, end);
    }
}

//------------------------------------------------------------------------
} // namespace mam
