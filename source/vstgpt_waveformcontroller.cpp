//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "vstgpt_waveformcontroller.h"
#include "mam/meta_words/meta_word.h"
#include "meta_words_playback_region.h"
#include "vstgpt_waveformview.h"
#include "vstgui/lib/ccolor.h"
#include "vstgui/lib/cgradientview.h"
#include "vstgui/lib/controls/icontrollistener.h"
#include "vstgui/lib/cstring.h"
#include "vstgui/lib/events.h"
#include "vstgui/lib/platform/platformfactory.h"
#include "vstgui/uidescription/iuidescription.h"
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
static auto update_waveform_view(WaveformView* view,
                                 const VstGPTWaveFormController::Data& data)
    -> void
{
    if (!view)
        return;

    const auto [r, g, b] = data.color;
    const VSTGUI::CColor color(r, g, b);
    view->setColor(color);
    view->setDirty();
}

//------------------------------------------------------------------------
static auto update_background_view(CGradientView* view,
                                   const VstGPTWaveFormController::Data& data)
    -> void
{
    if (!view)
        return;

    // TODO: Create a new gradient and set it to the view!
    // view->setGradient(new VSTGUI::CGradient(...));
    // view->setDirty();
}

//------------------------------------------------------------------------
// VstGPTWaveFormController
//------------------------------------------------------------------------
VstGPTWaveFormController::VstGPTWaveFormController(
    tiny_observer_pattern::SimpleSubject* subject)
: subject(subject)
{
    if (subject)
        observer_id =
            subject->add_listener([this](const auto&) { this->onDataChanged(); });
}

//------------------------------------------------------------------------
VstGPTWaveFormController::~VstGPTWaveFormController()
{
    if (subject)
        subject->remove_listener(observer_id);
}

//------------------------------------------------------------------------
void VstGPTWaveFormController::onDataChanged()
{
    const auto data = this->waveform_data_func();

    update_waveform_view(waveform_view, data);
    update_background_view(background_view, data);
}

//------------------------------------------------------------------------
CView*
VstGPTWaveFormController::createView(const VSTGUI::UIAttributes& attributes,
                                     const VSTGUI::IUIDescription* description)
{
    if (const auto* view_name =
            attributes.getAttributeValue("custom-view-name"))
    {
        if (*view_name == "WaveForm")
        {
            const auto view_size =
                read_view_size(attributes).value_or<CPoint>({320., 240.});

            return new WaveformView(CRect{0, 0, view_size.x, view_size.y});
        }
    }

    return nullptr;
}

//------------------------------------------------------------------------
VSTGUI::CView*
VstGPTWaveFormController::verifyView(VSTGUI::CView* view,
                                     const VSTGUI::UIAttributes& attributes,
                                     const VSTGUI::IUIDescription* description)
{
    if (const auto* view_name =
            attributes.getAttributeValue("custom-view-name"))
    {
        if (*view_name == "WaveForm")
        {
            const auto func = this->waveform_data_func;
            waveform_view   = dynamic_cast<WaveformView*>(view);
            waveform_view->setAudioBufferFunc(
                [func]() { return func().audio_buffer; });
            onDataChanged();
        }
        else if (*view_name == "Background")
        {
            background_view = dynamic_cast<CGradientView*>(view);
        }
    }
    return view;
}

//------------------------------------------------------------------------
void VstGPTWaveFormController::set_waveform_data_func(
    const FuncWaveFormData&& waveform_data_func)
{
    this->waveform_data_func = waveform_data_func;
    onDataChanged();
}

//------------------------------------------------------------------------
} // namespace mam
