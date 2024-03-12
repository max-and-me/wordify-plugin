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

//------------------------------------------------------------------------
namespace mam {
using namespace ::VSTGUI;

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
    int num_samples;
    float* waveform_data = const_cast<float*>(
        controller.collect_region_channel_buffer(num_samples));

    view = new WaveformView(CRect{0, 0, 400, 200}, waveform_data, num_samples);
    return view;
}

//------------------------------------------------------------------------
} // namespace mam
