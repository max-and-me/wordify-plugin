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
// WaveFormView
//------------------------------------------------------------------------
class WaveFormView : public VSTGUI::CView
{
public:
    //--------------------------------------------------------------------
    struct Data
    {
        using Color       = std::tuple<double, double, double>;
        using AudioBuffer = gsl::span<const float>;

        Color color;
        AudioBuffer audio_buffer;
    };

    using FuncWaveFormData = std::function<Data()>;

    WaveFormView(const VSTGUI::CRect& size);
    bool initialize(FuncWaveFormData&& audio_buffer_func);
    void draw(VSTGUI::CDrawContext* pContext) override;

    //--------------------------------------------------------------------
private:
    void drawFull(VSTGUI::CDrawContext* pContext,
                  const VSTGUI::CRect& viewSize);
    void draw_like_spotify(VSTGUI::CDrawContext& pContext,
                           const VSTGUI::CRect& viewSize);

    FuncWaveFormData waveform_data_func;
};

} // namespace mam
