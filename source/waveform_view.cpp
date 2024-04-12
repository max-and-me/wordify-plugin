//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#include "waveform_view.h"
#include "mam/wave-draw/wave-draw.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cgraphicspath.h"

using namespace VSTGUI;

namespace mam {

//------------------------------------------------------------------------
// WaveFormView
//------------------------------------------------------------------------
template <typename T>
class BoundsCheck
{
public:
    //--------------------------------------------------------------------
    using Range = std::pair<T, T>;
    BoundsCheck(const Range& range)
    : lo(range.first)
    , hi(range.first + range.second)
    {
    }

    bool is_in(const T& value) const { return is_in(value, lo, hi); }

    //--------------------------------------------------------------------

private:
    const T lo{0};
    const T hi{0};

    bool is_in(const T& value, const T& lo, const T& hi) const
    {
        return !(value < lo) && !(hi < value);
    }
};
//------------------------------------------------------------------------
WaveFormView::WaveFormView(const CRect& size)
: CView(size)
{
}

//------------------------------------------------------------------------
auto WaveFormView::initialize(FuncWaveFormData&& waveform_data_func) -> bool
{
    this->waveform_data_func = std::move(waveform_data_func);
    return true;
}

//------------------------------------------------------------------------
auto WaveFormView::draw_like_spotify(CDrawContext& pContext,
                                     const CRect& viewSize) -> void
{
    using Drawer   = wave_draw::Drawer;
    using DrawData = wave_draw::DrawData;

    constexpr auto SPACING             = 1.;
    constexpr auto LINE_WIDTH          = 2.;
    constexpr auto ROUND_CORNER_RADIUS = 1.;

    auto waveform_data = waveform_data_func();
    // Since we have a fixed view_width, we need to compute the zoom_factor
    // beforehand.
    const auto zoom_factor = wave_draw::compute_zoom_factor(
        waveform_data.audio_buffer, viewSize.getWidth(), LINE_WIDTH, SPACING);

    const auto samples_per_bucket = static_cast<size_t>(zoom_factor);
    const BoundsCheck<size_t> bounds_check(
        {waveform_data.highlight_range.first / samples_per_bucket,
         waveform_data.highlight_range.second / samples_per_bucket});

    // TODO: Get rid of warning!
    const auto [r, g, b] = waveform_data.color;
    const VSTGUI::CColor color_normal(r, g, b);
    const VSTGUI::CColor color_highlight(255, 255, 255);

    Drawer()
        .init(waveform_data.audio_buffer, zoom_factor)
        .setup_wave(LINE_WIDTH, SPACING)
        .setup_dimensions(viewSize.getWidth(), viewSize.getHeight())
        .draw([&](const DrawData& data, size_t count) {
            const auto rect =
                CRect({data.x, data.y}, {data.width, data.height});
            auto graphics_path = owned(pContext.createRoundRectGraphicsPath(
                rect, ROUND_CORNER_RADIUS));

            pContext.setFillColor(bounds_check.is_in(count) ? color_highlight
                                                            : color_normal);
            pContext.drawGraphicsPath(graphics_path);
        });
}

//------------------------------------------------------------------------
auto WaveFormView::drawFull(CDrawContext* pContext,
                            const CRect& viewSize) -> void
{
    pContext->setLineWidth(1.0);

    const auto amplitude    = viewSize.getHeight() * 0.5;
    const auto waveFormData = waveform_data_func().audio_buffer;
    const auto numSamples   = waveform_data_func().audio_buffer.size();
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
// https://steinbergmedia.github.io/vst3_doc/vstgui/html/the_view_system.html#inherit_from_cview
//------------------------------------------------------------------------
auto WaveFormView::draw(CDrawContext* pContext) -> void
{
    if (!pContext)
        return;

    const auto viewSize = getViewSize();
    CDrawContext::Transform t(
        *pContext, CGraphicsTransform().translate(viewSize.getTopLeft()));

    draw_like_spotify(*pContext, viewSize);
}

//------------------------------------------------------------------------
} // namespace mam
