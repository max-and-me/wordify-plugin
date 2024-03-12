//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "vstgpt_waveformcontroller.h"
#include "mam/meta_words/meta_word.h"
#include "vstgpt_waveformview.h"
#include "vstgui/lib/controls/icontrollistener.h"
#include "vstgui/lib/cstring.h"
#include "vstgui/lib/events.h"
#include "vstgui/lib/platform/platformfactory.h"
#include "vstgui/uidescription/uiattributes.h"
#include <optional>

//------------------------------------------------------------------------
namespace mam {
using namespace ::VSTGUI;

//------------------------------------------------------------------------
using CPointOptional = std::optional<CPoint>;
static auto read_view_size(const VSTGUI::UIAttributes& attributes)
    -> CPointOptional
{
    if (!attributes.hasAttribute("size"))
        return std::nullopt;

    VSTGUI::CPoint view_size;
    const auto* view_size_str = attributes.getAttributeValue("size");
    if (view_size_str)
    {
        if (attributes.stringToPoint(*view_size_str, view_size) == false)
            return std::nullopt;
    }

    return view_size;
}

//------------------------------------------------------------------------
// VstGPTWaveFormController
//------------------------------------------------------------------------
VstGPTWaveFormController::VstGPTWaveFormController(
    ARADocumentController& _controller)
: controller(_controller)
{
}

//------------------------------------------------------------------------
VstGPTWaveFormController::~VstGPTWaveFormController() {}

//------------------------------------------------------------------------
CView*
VstGPTWaveFormController::createView(const VSTGUI::UIAttributes& attributes,
                                     const VSTGUI::IUIDescription* description)
{
    const auto view_size_optional = read_view_size(attributes);
    const auto view_size = view_size_optional.value_or<CPoint>({320., 240.});

    int num_samples;
    float* waveform_data = const_cast<float*>(
        controller.collect_region_channel_buffer(num_samples));

    view = new WaveformView(CRect{0, 0, view_size.x, view_size.y},
                            waveform_data, num_samples);
    return view;
}

//------------------------------------------------------------------------
} // namespace mam
