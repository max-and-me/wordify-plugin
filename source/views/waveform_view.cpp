// Copyright (c) 2023-present, WordifyOrg.

#include "waveform_view.h"
#include "little_helpers.h"
#include "mam/wave-draw/wave-draw.h"
#include "warn_cpp/suppress_warnings.h"
#include <optional>
BEGIN_SUPPRESS_WARNINGS
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cgraphicspath.h"
END_SUPPRESS_WARNINGS

using namespace VSTGUI;

namespace mam {

//------------------------------------------------------------------------
constexpr auto HITLITE_TINT_FACTOR = 2.f / 3.f;
constexpr auto SPACING             = 1.;
constexpr auto LINE_WIDTH          = 2.;
constexpr auto ROUND_CORNER_RADIUS = 1.;

//------------------------------------------------------------------------
WaveFormView::WaveFormView(const CRect& size)
: CView(size)
{
}

//------------------------------------------------------------------------
auto WaveFormView::draw_like_spotify(CDrawContext& pContext,
                                     const CRect& viewSize) -> void
{
    if (!waveform_data_func)
        return;

    using Drawer   = wave_draw::Drawer;
    using DrawData = wave_draw::DrawData;
    using Color    = VSTGUI::CColor;

    auto waveform_data = waveform_data_func();
    // Since we have a fixed view_width, we need to compute the zoom_factor
    // beforehand.
    const auto zoom_factor = wave_draw::compute_zoom_factor(
        waveform_data.audio_buffer, viewSize.getWidth(), LINE_WIDTH, SPACING);

    const auto samples_per_bucket = static_cast<size_t>(zoom_factor);
    const BoundsCheck<size_t> bounds_check(
        {waveform_data.hilite_range.first / samples_per_bucket,
         waveform_data.hilite_range.second / samples_per_bucket});

    // TODO: Get rid of warning!
    const auto& [r, g, b]    = waveform_data.color;
    const Color color_normal = make_color<float>(r, g, b, std::nullopt);
    const Color color_hilite = make_color<float>(r, g, b, HITLITE_TINT_FACTOR);

    auto normal_graphics_path = owned(pContext.createGraphicsPath());
    auto hilite_graphics_path = owned(pContext.createGraphicsPath());

    Drawer(waveform_data.audio_buffer, zoom_factor)
        .setup_wave(LINE_WIDTH, SPACING)
        .setup_dimensions(viewSize.getWidth(), viewSize.getHeight())
        .draw([&](const DrawData& data, size_t count) {
            const auto rect =
                CRect({data.x, data.y}, {data.width, data.height});

            if (bounds_check.is_in(count))
                hilite_graphics_path->addRoundRect(rect, ROUND_CORNER_RADIUS);
            else
                normal_graphics_path->addRoundRect(rect, ROUND_CORNER_RADIUS);
        });

    pContext.setFillColor(color_normal);
    pContext.drawGraphicsPath(normal_graphics_path);
    pContext.setFillColor(color_hilite);
    pContext.drawGraphicsPath(hilite_graphics_path);
}

//------------------------------------------------------------------------
auto WaveFormView::drawFull(CDrawContext* pContext,
                            const CRect& viewSize) -> void
{
    if (!waveform_data_func)
        return;

    pContext->setLineWidth(1.0);

    const auto amplitude    = viewSize.getHeight() * 0.5;
    const auto waveFormData = waveform_data_func().audio_buffer;
    const auto numSamples   = waveform_data_func().audio_buffer.size();
    if (numSamples > 1)
    {
        // Calculate the horizontal scale factor
        const auto xScale =
            viewSize.getWidth() / static_cast<CCoord>(numSamples - 1);

        size_t stylized = 1; // from 1 up to x to have it more stylized
        // Draw the waveform lines
        for (size_t i = 0; i < size_t(numSamples) - 1; i += stylized)
        {
            const auto x1 = CCoord(i) * xScale;
            const auto y1 = amplitude * CCoord(waveFormData[i]);
            const auto x2 = CCoord(i + stylized) * xScale;
            const auto y2 = amplitude * CCoord(waveFormData[i + 1]);

            pContext->drawLine(CPoint(x1, amplitude + y1),
                               CPoint(x2, amplitude + y2));
        }
    }
}

//------------------------------------------------------------------------
// https://steinbergmedia.github.io/vst3_doc/vstgui/html/the_view_system.html#inherit_from_cview
//------------------------------------------------------------------------
auto WaveFormView::draw(CDrawContext* pContext) -> void
{
    if (!pContext)
        return;

    const auto viewSize = getViewSize();
    CDrawContext::Transform t(
        *pContext, CGraphicsTransform().translate(viewSize.getTopLeft()));

    draw_like_spotify(*pContext, viewSize);
}

//------------------------------------------------------------------------
} // namespace mam
