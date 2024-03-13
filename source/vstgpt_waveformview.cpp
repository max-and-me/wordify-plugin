//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------
#include "vstgpt_waveformview.h"
#include "vstgui/lib/cdrawcontext.h"

using namespace VSTGUI;

namespace mam {

//------------------------------------------------------------------------
// WaveformView
//------------------------------------------------------------------------
WaveformView::WaveformView(const VSTGUI::CRect& size,
                           float* _waveFormData,
                           int _numSamples)
: CView(size)
, waveFormData(_waveFormData)
, numSamples(_numSamples)
{
    setTransparency(false);
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
    pContext->setLineWidth(1.0);

    const auto amplitude = viewSize.getHeight() * 0.5;
    if (waveFormData && numSamples > 1)
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

//--------------------------------------------------------------------
void WaveformView::setColor (VSTGUI::CColor color)
{
    waveformColor = color;
}

} // namespace mam
