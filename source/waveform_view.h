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
        using Color       = std::tuple<uint8_t, uint8_t, uint8_t>;
        using AudioBuffer = gsl::span<const float>;

        Color color;
        AudioBuffer audio_buffer;
    };

    using FuncWaveFormData = std::function<Data()>;

    WaveFormView(const VSTGUI::CRect& size);
    auto initialize(FuncWaveFormData&& audio_buffer_func) -> bool;
    auto draw(VSTGUI::CDrawContext* pContext) -> void override;

    //--------------------------------------------------------------------
private:
    auto drawFull(VSTGUI::CDrawContext* pContext, const VSTGUI::CRect& viewSize)
        -> void;
    auto draw_like_spotify(VSTGUI::CDrawContext& pContext,
                           const VSTGUI::CRect& viewSize) -> void;

    FuncWaveFormData waveform_data_func;
};

} // namespace mam
