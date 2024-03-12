//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "vstgui/lib/cview.h"

#pragma once

namespace mam {

//------------------------------------------------------------------------
// WaveformView
//------------------------------------------------------------------------
class WaveformView : public VSTGUI::CView
{

public:
    WaveformView(const VSTGUI::CRect& size,
                 float* waveFormData,
                 int numSamples);
    void draw(VSTGUI::CDrawContext* pContext) override;

private:
    float* waveFormData{nullptr};
    int numSamples;
};

} // namespace mam
