//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "vstgpt_waveformcontroller.h"
#include "mam/meta_words/meta_word.h"
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
static auto update_waveform_view(WaveformView* view, const MetaWordsData& data)
    -> void
{
    if (!view)
        return;

    const VSTGUI::CColor color(data.color.r, data.color.g, data.color.b);
    view->setColor(color);
    view->setDirty();
}

//------------------------------------------------------------------------
static auto update_background_view(CGradientView* view,
                                   const MetaWordsData& data) -> void
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
    ARADocumentController* controller,
    ARADocumentController::FnGetSampleRate& func_playback_sample_rate)
: controller(controller)
, func_playback_sample_rate(func_playback_sample_rate)
{
    if (controller)
        observer_id =
            controller->add_listener([this]() { this->onDataChanged(); });
}

//------------------------------------------------------------------------
VstGPTWaveFormController::~VstGPTWaveFormController()
{
    if (controller)
        controller->remove_listener(observer_id);
}

//------------------------------------------------------------------------
void VstGPTWaveFormController::onDataChanged()
{
    if (!controller)
        return;

    cached_meta_words_data_list =
        controller->collect_meta_data_words(func_playback_sample_rate());
    if (cached_meta_words_data_list.empty())
        return;

    const auto& data = cached_meta_words_data_list.at(0);

    update_waveform_view(waveform_view, data);
    update_background_view(background_view, data);
}

//------------------------------------------------------------------------
CView*
VstGPTWaveFormController::createView(const VSTGUI::UIAttributes& attributes,
                                     const VSTGUI::IUIDescription* description)
{
    if (!controller)
        return nullptr;

    CView* new_view = nullptr;
    if (const auto* view_name =
            attributes.getAttributeValue("custom-view-name"))
    {
        if (*view_name == "WaveForm")
        {
            const auto view_size_optional = read_view_size(attributes);
            const auto view_size =
                view_size_optional.value_or<CPoint>({320., 240.});

            const auto c    = this->controller;
            const auto func = this->func_playback_sample_rate;
            new_view        = new WaveformView(
                CRect{0, 0, view_size.x, view_size.y}, [c, func]() {
                    const auto sr = func();
                    return c->collect_region_channel_buffer(sr);
                });
        }
    }

    return new_view;
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
            waveform_view = dynamic_cast<WaveformView*>(view);
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
} // namespace mam
