//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "gsl/span"
#include "vstgui/lib/ccolor.h"
#include "vstgui/lib/cview.h"

namespace mam {

//------------------------------------------------------------------------
// WaveFormView
//------------------------------------------------------------------------
class WaveFormView : public VSTGUI::CView
{
public:
    //--------------------------------------------------------------------
    struct Data
    {
        using Color       = std::tuple<float, float, float>;
        using AudioBuffer = gsl::span<const float>;
        using Range       = std::pair<size_t, size_t>; // start and duration

        Color color;
        AudioBuffer audio_buffer;
        Range highlight_range;
    };

    using FuncWaveFormData = std::function<const Data()>;

    WaveFormView(const VSTGUI::CRect& size);
    auto initialize(FuncWaveFormData&& waveform_data_func) -> bool;
    auto draw(VSTGUI::CDrawContext* pContext) -> void override;

    //--------------------------------------------------------------------
private:
    auto drawFull(VSTGUI::CDrawContext* pContext,
                  const VSTGUI::CRect& viewSize) -> void;
    auto draw_like_spotify(VSTGUI::CDrawContext& pContext,
                           const VSTGUI::CRect& viewSize) -> void;

    FuncWaveFormData waveform_data_func;
};

} // namespace mam
