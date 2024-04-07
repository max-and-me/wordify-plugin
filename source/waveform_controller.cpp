//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "waveform_controller.h"
#include "vstgui/lib/cgradientview.h"
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
static auto update_background_view(CGradientView* view,
                                   const WaveFormController::Data& data) -> void
{
    if (!view)
        return;

    // TODO: Create a new gradient and set it to the view!
    // view->setGradient(new VSTGUI::CGradient(...));
    // view->invalid();
}

//------------------------------------------------------------------------
// WaveFormController
//------------------------------------------------------------------------
WaveFormController::WaveFormController() {}

//------------------------------------------------------------------------
WaveFormController::~WaveFormController()
{
    if (subject)
        subject->remove_listener(observer_id);
}

//------------------------------------------------------------------------
bool WaveFormController::initialize(Subject* subject,
                                    FuncWaveFormData&& waveform_data_func)
{
    if (!subject)
        return false;

    if (this->subject)
    {
        if (subject)
            subject->remove_listener(observer_id);
    }

    this->subject            = subject;
    this->waveform_data_func = std::move(waveform_data_func);

    observer_id = subject->add_listener(
        [this](const auto&) { this->on_waveform_data_changed(); });

    on_waveform_data_changed();

    return true;
}

//------------------------------------------------------------------------
void WaveFormController::on_waveform_data_changed()
{
    const auto data = this->waveform_data_func();

    if (waveform_view)
        waveform_view->invalid();

    update_background_view(background_view, data);
}

//------------------------------------------------------------------------
CView* WaveFormController::createView(const VSTGUI::UIAttributes& attributes,
                                      const VSTGUI::IUIDescription* description)
{
    if (const auto* view_name =
            attributes.getAttributeValue("custom-view-name"))
    {
        if (*view_name == "WaveForm")
        {
            const auto view_size =
                read_view_size(attributes).value_or<CPoint>({320., 240.});

            return new WaveFormView(CRect{0, 0, view_size.x, view_size.y});
        }
    }

    return nullptr;
}

//------------------------------------------------------------------------
VSTGUI::CView*
WaveFormController::verifyView(VSTGUI::CView* view,
                               const VSTGUI::UIAttributes& attributes,
                               const VSTGUI::IUIDescription* description)
{
    if (const auto* view_name =
            attributes.getAttributeValue("custom-view-name"))
    {
        if (*view_name == "WaveForm")
        {
            waveform_view = dynamic_cast<WaveFormView*>(view);
            waveform_view->initialize(
                [func = this->waveform_data_func]() -> WaveFormView::Data {
                    return func();
                });
            waveform_view->invalid();
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
