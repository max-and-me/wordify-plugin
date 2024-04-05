//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "gsl/span"
#include "vstgui/lib/ccolor.h"
#include "vstgui/lib/cview.h"

namespace mam {
//------------------------------------------------------------------------
using SampleType      = float;
using AudioBufferSpan = gsl::span<const SampleType>;

//------------------------------------------------------------------------
// WaveformView
//------------------------------------------------------------------------
class WaveformView : public VSTGUI::CView
{
public:
    //--------------------------------------------------------------------
    using FuncAudioBuffer = std::function<AudioBufferSpan()>;

    WaveformView(const VSTGUI::CRect& size);

    void draw(VSTGUI::CDrawContext* pContext) override;
    void setColor(VSTGUI::CColor);
    void setAudioBufferFunc(FuncAudioBuffer&& func_audio_buffer);

    //--------------------------------------------------------------------
private:
    void drawFull(VSTGUI::CDrawContext* pContext,
                  const VSTGUI::CRect& viewSize);
    void drawSimplified(VSTGUI::CDrawContext* pContext,
                        const VSTGUI::CRect& viewSize);
    void draw_like_spotify(VSTGUI::CDrawContext& pContext,
                           const VSTGUI::CRect& viewSize);

    FuncAudioBuffer func_audio_buffer;
    VSTGUI::CColor waveformColor{0, 0, 0};
};

} // namespace mam
