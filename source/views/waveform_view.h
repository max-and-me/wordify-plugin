// Copyright(c) 2024 Max And Me.

#pragma once

#include "nonstd.h"
#include "warn_cpp/suppress_warnings.h"
BEGIN_SUPPRESS_WARNINGS
#include "vstgui/lib/ccolor.h"
#include "vstgui/lib/cview.h"
END_SUPPRESS_WARNINGS

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
        using AudioBuffer = nonstd::span<const float>;
        using Range       = std::pair<size_t, size_t>; // start and duration

        Color color;
        AudioBuffer audio_buffer;
        Range hilite_range;
    };

    using FuncWaveFormData = std::function<const Data()>;
    FuncWaveFormData waveform_data_func;

    WaveFormView(const VSTGUI::CRect& size);
    auto draw(VSTGUI::CDrawContext* pContext) -> void override;

    //--------------------------------------------------------------------
private:
    auto drawFull(VSTGUI::CDrawContext* pContext, const VSTGUI::CRect& viewSize)
        -> void;
    auto draw_like_spotify(VSTGUI::CDrawContext& pContext,
                           const VSTGUI::CRect& viewSize) -> void;
};

} // namespace mam
