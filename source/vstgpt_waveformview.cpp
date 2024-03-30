//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------
#include "vstgpt_waveformview.h"
#include "mam/wave-draw/wave-draw.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cgraphicspath.h"
#include <algorithm>

using namespace VSTGUI;

namespace mam {

//------------------------------------------------------------------------
static auto create_rect(const wave_draw::DrawData& data) -> const CRect
{
    const CPoint origin(data.x, data.y);
    const CPoint size(data.width, data.height);
    const CRect rect = CRect(origin, size);
    return rect;
}
//------------------------------------------------------------------------
static auto draw_data(VSTGUI::CDrawContext& context,
                      const wave_draw::DrawData& data)
{
    constexpr CCoord ROUND_CORNER_RADIUS = 1.;

    VSTGUI::SharedPointer<VSTGUI::CGraphicsPath> obj =
        VSTGUI::owned(context.createRoundRectGraphicsPath(create_rect(data),
                                                          ROUND_CORNER_RADIUS));
    auto* graphics_path = context.createRoundRectGraphicsPath(
        create_rect(data), ROUND_CORNER_RADIUS);
    context.drawGraphicsPath(graphics_path);
}

//------------------------------------------------------------------------
// WaveformView
//------------------------------------------------------------------------
WaveformView::WaveformView(const VSTGUI::CRect& size,
                           FnGetAudioBuffer&& fn_get_audio_buffer)
: CView(size)
, fn_get_audio_buffer(fn_get_audio_buffer)
{
}

//------------------------------------------------------------------------
void WaveformView::draw_like_spotify(VSTGUI::CDrawContext* pContext,
                                     const VSTGUI::CRect& viewSize)
{
    constexpr auto spacing    = 1.;
    constexpr auto line_width = 2.;

    // Since we have a fixed view_width, we need to compute the zoom_factor
    // beforehand.
    const auto zoom_factor = wave_draw::compute_zoom_factor(
        fn_get_audio_buffer(), viewSize.getWidth(), line_width, spacing);

    auto drawFunc = [&](const wave_draw::DrawData& data) {
        draw_data(*pContext, data);
    };
    wave_draw::Drawer()
        .init(fn_get_audio_buffer(), zoom_factor)
        .setup_wave(line_width, spacing)
        .setup_dimensions(viewSize.getWidth(), viewSize.getHeight())
        .draw(drawFunc);
}

//------------------------------------------------------------------------
void WaveformView::drawFull(VSTGUI::CDrawContext* pContext,
                            const VSTGUI::CRect& viewSize)
{
    pContext->setLineWidth(1.0);

    const auto amplitude    = viewSize.getHeight() * 0.5;
    const auto waveFormData = fn_get_audio_buffer();
    const auto numSamples   = fn_get_audio_buffer().size();
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
void WaveformView::drawSimplified(VSTGUI::CDrawContext* pContext,
                                  const VSTGUI::CRect& viewSize)
{
    pContext->setLineWidth(4.0);

    const auto amplitude    = viewSize.getHeight() * 0.5;
    const auto waveFormData = fn_get_audio_buffer();
    const auto numSamples   = fn_get_audio_buffer().size();
    if (numSamples > 1)
    {
        // Calculate the horizontal scale factor
        const auto xScale =
            viewSize.getWidth() / static_cast<CCoord>(numSamples - 1);

        size_t stylized = 200; // from 1 up to x to have it more stylized
        int oldBased    = 0;
        int offset      = 20;

        // Draw the waveform lines
        for (size_t i = 0; i < size_t(numSamples) - 1; i += stylized)
        {
            if (!waveFormData[i + offset])
                return;

            const auto x1 = CCoord(i + offset) * xScale;
            const auto y1 = amplitude * CCoord(waveFormData[i + offset]);

            int based = floor(x1);
            if (based != oldBased && based % 5 == 0)
            {
                pContext->drawLine(CPoint(x1, amplitude + y1 + offset / 2),
                                   CPoint(x1, amplitude - y1 + offset / 2));
                oldBased = based;
            }
        }
    }
}

//------------------------------------------------------------------------
// https://steinbergmedia.github.io/vst3_doc/vstgui/html/the_view_system.html#inherit_from_cview
//------------------------------------------------------------------------
void WaveformView::draw(CDrawContext* pContext)
{
    const auto viewSize = getViewSize();
    CDrawContext::Transform t(
        *pContext, CGraphicsTransform().translate(viewSize.getTopLeft()));

    // Draw the waveform with black lines
    pContext->setFrameColor(waveformColor);

    pContext->setFillColor({75, 75, 75});
    // pContext->drawGraphicsPath(
    //   pContext->createRoundRectGraphicsPath(viewSize, 15));

    // drawSimplified(pContext, viewSize);
    // drawFull(pContext, viewSize);
    draw_like_spotify(pContext, viewSize);
}

//--------------------------------------------------------------------
void WaveformView::setColor(VSTGUI::CColor color)
{
    waveformColor = color;
}

} // namespace mam
