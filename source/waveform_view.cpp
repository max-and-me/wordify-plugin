//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------
#include "waveform_view.h"
#include "mam/wave-draw/wave-draw.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cgraphicspath.h"
#include <algorithm>

using namespace VSTGUI;

namespace mam {

//------------------------------------------------------------------------
// WaveFormView
//------------------------------------------------------------------------
WaveFormView::WaveFormView(const CRect& size)
: CView(size)
{
}

//------------------------------------------------------------------------
bool WaveFormView::initialize(FuncAudioBuffer&& audio_buffer_func)
{
    this->audio_buffer_func = std::move(audio_buffer_func);
    return true;
}

//------------------------------------------------------------------------
void WaveFormView::draw_like_spotify(CDrawContext& pContext,
                                     const CRect& viewSize)
{
    using Drawer   = wave_draw::Drawer;
    using DrawData = wave_draw::DrawData;

    constexpr auto SPACING             = 1.;
    constexpr auto LINE_WIDTH          = 2.;
    constexpr auto ROUND_CORNER_RADIUS = 1.;

    // Since we have a fixed view_width, we need to compute the zoom_factor
    // beforehand.
    const auto zoom_factor = wave_draw::compute_zoom_factor(
        audio_buffer_func(), viewSize.getWidth(), LINE_WIDTH, SPACING);

    Drawer()
        .init(audio_buffer_func(), zoom_factor)
        .setup_wave(LINE_WIDTH, SPACING)
        .setup_dimensions(viewSize.getWidth(), viewSize.getHeight())
        .draw([&](const DrawData& data) {
            const auto rect =
                CRect({data.x, data.y}, {data.width, data.height});
            auto graphics_path = owned(pContext.createRoundRectGraphicsPath(
                rect, ROUND_CORNER_RADIUS));

            pContext.setFillColor(waveformColor);
            pContext.drawGraphicsPath(graphics_path);
        });
}

//------------------------------------------------------------------------
void WaveFormView::drawFull(CDrawContext* pContext, const CRect& viewSize)
{
    pContext->setLineWidth(1.0);

    const auto amplitude    = viewSize.getHeight() * 0.5;
    const auto waveFormData = audio_buffer_func();
    const auto numSamples   = audio_buffer_func().size();
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
void WaveFormView::drawSimplified(CDrawContext* pContext, const CRect& viewSize)
{
    pContext->setLineWidth(4.0);

    const auto amplitude    = viewSize.getHeight() * 0.5;
    const auto waveFormData = audio_buffer_func();
    const auto numSamples   = audio_buffer_func().size();
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
void WaveFormView::draw(CDrawContext* pContext)
{
    if (!pContext)
        return;

    const auto viewSize = getViewSize();
    CDrawContext::Transform t(
        *pContext, CGraphicsTransform().translate(viewSize.getTopLeft()));

    pContext->setFillColor({75, 75, 75});

    // drawSimplified(pContext, viewSize);
    // drawFull(pContext, viewSize);
    draw_like_spotify(*pContext, viewSize);
}

//--------------------------------------------------------------------
void WaveFormView::setColor(CColor color)
{
    waveformColor = color;
}

//------------------------------------------------------------------------
} // namespace mam
