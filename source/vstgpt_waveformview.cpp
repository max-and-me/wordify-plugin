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
void WaveformView::draw(CDrawContext* pContext)
{
    // Draw the waveform with black lines
    pContext->setFrameColor(CColor(0, 0, 0));
    pContext->setLineWidth(1.0);

    CRect viewSize = getViewSize();
    viewSize.inset(5, 5);

    const float amplitude = viewSize.getHeight() * 0.7;
    if (waveFormData && numSamples > 1)
    {
        // Calculate the horizontal scale factor
        float xScale = viewSize.getWidth() / static_cast<float>(numSamples - 1);

        int stylized = 1; // from 1 up to x to have it more stylized
        // Draw the waveform lines
        for (size_t i = 0; i < numSamples - 1; i += stylized)
        {
            float x1 = i * xScale;
            float y1 = amplitude * waveFormData[i];
            float x2 = (i + stylized) * xScale;
            float y2 = amplitude * waveFormData[i + 1];

            pContext->drawLine(CPoint(x1, amplitude + y1),
                               CPoint(x2, amplitude + y2));
        }
    }
}

} // namespace mam
