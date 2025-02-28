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
    size.left += std::ceil(line_width / 2.);
    size.right -= std::ceil(line_width / 2.);
    size.top += std::ceil(line_width / 2.);
    size.bottom -= std::ceil(line_width / 2.);

    return size;
}

//------------------------------------------------------------------------
struct Point
{
    double x = 0.;
    double y = 0.;
};

struct Rect
{
    double width  = 0.;
    double height = 0.;
};

using Line   = std::pair<Point, Point>;
using FnLine = std::function<void(const Line&, double delta)>;
/**
 * @brief Computes the lines to be drawn
 * @param rect The rect inside which the lines shall be drawn
 * @param centerPoint The center point coordinates inside the rect
 * @param angleDeltaNormalized Rotate the lines around the center, range 0..1
 * (e.g. for animation)
 * @param fnLine Function callback for each line
 */
auto compute_lines(const Rect& rect,
                   const Point& center,
                   double angleDeltaNormalized,
                   FnLine&& fnLine) -> void
{
    constexpr auto MAX_CIRCLE_ANGLE  = 360.;
    constexpr auto HALF_CIRCLE_ANGLE = 180.;
    constexpr auto NUM_LINES         = std::max(1, 12); // at least one line
    constexpr auto ANGLE_STEP        = MAX_CIRCLE_ANGLE / NUM_LINES;
    constexpr double MAX_ANGLE =
        (NUM_LINES * ANGLE_STEP) * (kPI / HALF_CIRCLE_ANGLE);
    constexpr auto NORM_LINE_LEN = 0.3; // normalized from 0. to 1.

    const auto angleDeltaDegree = angleDeltaNormalized * MAX_CIRCLE_ANGLE;
    const auto width_half       = rect.width * 0.5;
    const auto height_half      = rect.height * 0.5;

    for (int i = 0; i < NUM_LINES; ++i)
    {
        const auto angle = (double(i) * ANGLE_STEP + angleDeltaDegree) *
                           (kPI / HALF_CIRCLE_ANGLE);
        const auto width  = sin(angle - kPI) * width_half;
        const auto height = cos(angle - kPI) * height_half;
        const Point start{center.x + width, center.y + height};
        const Point end{center.x + width * (1. - NORM_LINE_LEN),
                        center.y + height * (1. - NORM_LINE_LEN)};

        const auto delta = angle / MAX_ANGLE;
        const Line line{start, end};
        if (fnLine)
            fnLine(line, delta);
    }
}
//------------------------------------------------------------------------
auto draw_spinner_like_ios(CDrawContext* context,
                           const CRect& bounds_,
                           double animPos) -> void
{
    constexpr auto USE_ROUND_CAPS    = true;
    constexpr CCoord NORM_LINE_WIDTH = 0.1; // normalized from 0. to 1.

    CRect bounds = bounds_;
    if (USE_ROUND_CAPS)
    {
        // When using round caps, we need to have an inset to avoid clipping
        bounds = inset_for_round_line_caps(bounds,
                                           NORM_LINE_WIDTH * bounds.getWidth());
        context->setLineStyle(
            CLineStyle(CLineStyle::kLineCapRound, CLineStyle::kLineJoinRound));
    }

    context->setDrawMode(kAntiAliasing);
    context->setLineWidth(NORM_LINE_WIDTH * bounds.getWidth());

    const Point centerPoint{bounds.getCenter().x, bounds.getCenter().y};
    const Rect boundingBox{bounds.getWidth(), bounds.getHeight()};

    compute_lines(boundingBox, centerPoint, 0.,
                  [context, animPos](const Line& line, double delta) {
                      constexpr CColor LINE_COLOR(150, 150, 150);
                      const auto newAlpha = 1. - fmod(animPos + delta, 1.);

                      auto newLineColor = LINE_COLOR;
                      newLineColor.setNormAlpha(newAlpha);
                      context->setFrameColor(newLineColor);
                      context->setFillColor(newLineColor);

                      const CPoint start{line.first.x, line.first.y};
                      const CPoint end{line.second.x, line.second.y};
                      context->drawLine({start, end});
                  });
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
