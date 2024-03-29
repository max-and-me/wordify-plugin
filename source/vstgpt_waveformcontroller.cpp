//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "vstgpt_waveformcontroller.h"
#include "mam/meta_words/meta_word.h"
#include "vstgui/lib/ccolor.h"
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
static auto update_view(WaveformView& view, const MetaWordsData& data) -> void
{
    const VSTGUI::CColor color(data.color.r, data.color.g, data.color.b);
    view.setColor(color);
}

//------------------------------------------------------------------------
// VstGPTWaveFormController
//------------------------------------------------------------------------
VstGPTWaveFormController::VstGPTWaveFormController(
    ARADocumentController& controller,
    ARADocumentController::FnGetSampleRate&& fn_get_playback_sample_rate)
: controller(controller)
, fn_get_playback_sample_rate(fn_get_playback_sample_rate)
{
    observer_id = controller.add_listener([this]() { this->onDataChanged(); });
}

//------------------------------------------------------------------------
VstGPTWaveFormController::~VstGPTWaveFormController()
{
    controller.remove_listener(observer_id);
}

//------------------------------------------------------------------------
void VstGPTWaveFormController::onDataChanged()
{
    cached_meta_words_data_list =
        controller.collect_meta_data_words(fn_get_playback_sample_rate());
    if (cached_meta_words_data_list.empty())
        return;

    const auto& data = cached_meta_words_data_list.at(0);

    if (view)
    {
        update_view(*view, data);
        view->setDirty();
    }
}

//------------------------------------------------------------------------
CView*
VstGPTWaveFormController::createView(const VSTGUI::UIAttributes& attributes,
                                     const VSTGUI::IUIDescription* description)
{
    const auto view_size_optional = read_view_size(attributes);
    const auto view_size = view_size_optional.value_or<CPoint>({320., 240.});

    /*
    view = new WaveformView(CRect{0, 0, view_size.x, view_size.y}, [&]() {
        return
    controller.collect_region_channel_buffer(fn_get_playback_sample_rate());
    });

    onDataChanged();*/
    return view;
}

//------------------------------------------------------------------------
} // namespace mam
