//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "vstgui/lib/cview.h"
#include "vstgui/lib/ccolor.h"
#include "ara_document_controller.h"

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
    void setColor (VSTGUI::CColor);
    
private:
    void drawFull(VSTGUI::CDrawContext *pContext, const VSTGUI::CRect &viewSize);
    void drawSimplified(VSTGUI::CDrawContext *pContext, const VSTGUI::CRect &viewSize);
    
    float* waveFormData{nullptr};
    int numSamples;
    VSTGUI::CColor waveformColor {0,0,0};
   
};

} // namespace mam
