// Copyright (c) 2023-present, WordifyOrg.

#include "spinner_view.h"
#include "warn_cpp/suppress_warnings.h"
#include <chrono>
BEGIN_SUPPRESS_WARNINGS
#include "vstgui/lib/cdrawcontext.h"
END_SUPPRESS_WARNINGS

using namespace VSTGUI;

namespace mam {

#ifndef kPI
#define kPI 3.14159265358979323846
#endif

//------------------------------------------------------------------------
auto inset_for_round_line_caps(CRect& size, const CCoord line_width) -> CRect&
{
    size.left += std::floor(line_width / 2.);
    size.right -= std::ceil(line_width / 2.);
    size.top += std::floor(line_width / 2.);
    size.bottom -= std::ceil(line_width / 2.);

    return size;
}

//------------------------------------------------------------------------
auto draw_spinner_spinning(CDrawContext* context,
                           const CRect& bounds_,
                           double position) -> void
{
    constexpr auto use_round_caps = true;
    constexpr auto numLines       = 12;
    constexpr auto angleStep      = 360. / numLines;

    constexpr CCoord kNormLineLength = 0.4; // normalized from 0. to 1.
    constexpr CCoord kNormLineWidth  = 0.1; // normalized from 0. to 1.
    constexpr CColor lineColor(150, 150, 150);

    const double rotationAngle = 360. * position;

    CRect bounds = bounds_;
    if (use_round_caps)
    {
        // When using round caps, we need to have an inset to avoid clipping
        bounds = inset_for_round_line_caps(bounds,
                                           kNormLineWidth * bounds.getWidth());
        context->setLineStyle(
            CLineStyle(CLineStyle::kLineCapRound, CLineStyle::kLineJoinRound));
    }

    const CPoint center    = bounds.getCenter();
    const auto width_half  = bounds.getWidth() * 0.5;
    const auto height_half = bounds.getHeight() * 0.5;

    context->setDrawMode(kAntiAliasing);
    context->setLineWidth(kNormLineWidth * bounds.getWidth());
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
        const CPoint end(center.x + width * (1. - kNormLineLength),
                         center.y + height * (1. - kNormLineLength));

        lines.push_back({start, end});
    }

    context->drawLines(lines);
}

//------------------------------------------------------------------------
auto draw_spinner_like_ios(CDrawContext* context,
                           const CRect& bounds_,
                           double position) -> void
{
    // position: animation position from 0..1

    constexpr auto USE_ROUND_CAPS    = true;
    constexpr auto NUM_LINES         = 10; // at least 2 lines!!!
    constexpr auto ANGLE_STEP        = 360. / NUM_LINES;
    constexpr CCoord MAX_ANGLE       = (NUM_LINES * ANGLE_STEP) * (kPI / 180.0);
    constexpr CCoord NORM_LINE_LEN   = 0.4; // normalized from 0. to 1.
    constexpr CCoord NORM_LINE_WIDTH = 0.1; // normalized from 0. to 1.
    constexpr CColor LINE_COLOR(150, 150, 150);

    CRect bounds = bounds_;
    if (USE_ROUND_CAPS)
    {
        // When using round caps, we need to have an inset to avoid clipping
        bounds = inset_for_round_line_caps(bounds,
                                           NORM_LINE_WIDTH * bounds.getWidth());
        context->setLineStyle(
            CLineStyle(CLineStyle::kLineCapRound, CLineStyle::kLineJoinRound));
    }

    const CPoint center    = bounds.getCenter();
    const auto width_half  = bounds.getWidth() * 0.5;
    const auto height_half = bounds.getHeight() * 0.5;

    context->setDrawMode(kAntiAliasing);
    context->setLineWidth(NORM_LINE_WIDTH * bounds.getWidth());

    for (int i = 0; i < NUM_LINES; ++i)
    {
        const CCoord angle = (CCoord(i) * ANGLE_STEP) * (kPI / 180.0);
        const auto width   = sin(angle - kPI) * width_half;
        const auto height  = cos(angle - kPI) * height_half;
        const CPoint start(center.x + width, center.y + height);
        const CPoint end(center.x + width * (1. - NORM_LINE_LEN),
                         center.y + height * (1. - NORM_LINE_LEN));

        const auto delta    = angle / MAX_ANGLE;
        const auto newAlpha = 1. - fmod(position + delta, 1.);

        auto newLineColor = LINE_COLOR;
        newLineColor.setNormAlpha(newAlpha);
        context->setFrameColor(newLineColor);
        context->setFillColor(newLineColor);
        context->drawLine({start, end});
    }
}

//------------------------------------------------------------------------
SpinnerView::SpinnerView(const CRect& size)
: CView(size)
{
}

//------------------------------------------------------------------------
auto SpinnerView::set_animation_position(float position) -> void
{
    animation_position = position;
    invalid();
}

//------------------------------------------------------------------------
void SpinnerView::draw(CDrawContext* context)
{
    CView::draw(context);

    draw_spinner_like_ios(context, getViewSize(), animation_position);
    // draw_spinner_spinning(context, getViewSize(), animation_position);
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
        spinner_view->set_animation_position(0.);
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
        spinner_view->set_animation_position(pos);
    }
}

//------------------------------------------------------------------------
void SpinAnimation::animationFinished(CView* view,
                                      IdStringPtr name,
                                      bool /*wasCanceled*/)
{
    SpinnerView* spinner_view = dynamic_cast<SpinnerView*>(view);
    if (!spinner_view)
        return;

    if (UTF8String(name) == ANIMATION_ID)
    {
        spinner_view->set_animation_position(1.);
    }
}

//------------------------------------------------------------------------
} // namespace mam
