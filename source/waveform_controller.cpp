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
static auto
read_view_size(const VSTGUI::UIAttributes& attributes) -> VSTGUI::CPoint
{
    VSTGUI::CPoint size;
    attributes.getPointAttribute("size", size);
    return size;
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
    if (waveform_view)
        waveform_view->unregisterViewListener(this);

    if (background_view)
        background_view->unregisterViewListener(this);
}

//------------------------------------------------------------------------
bool WaveFormController::initialize(Subject* subject, FuncWaveFormData&& func)
{
    if (!subject)
        return false;

    this->waveform_data_func = std::move(func);
    this->observer           = tiny_observer_pattern::make_observer(
        subject, [&](const auto&) { this->on_meta_words_data_changed(); });

    return true;
}

//------------------------------------------------------------------------
void WaveFormController::on_meta_words_data_changed()
{
    if (waveform_view)
        waveform_view->invalid();

    const auto data = this->waveform_data_func();
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
            const auto view_size = read_view_size(attributes);
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
            waveform_view->waveform_data_func = this->waveform_data_func;
            waveform_view->registerViewListener(this);
        }
        else if (*view_name == "Background")
        {
            background_view = dynamic_cast<CGradientView*>(view);
            background_view->registerViewListener(this);
        }
    }
    return view;
}

//------------------------------------------------------------------------
void WaveFormController::viewAttached(CView* view)
{
    if (view == waveform_view)
    {
        waveform_view->invalid();
    }
    else if (view == background_view)
    {
        update_background_view(background_view, waveform_data_func());
    }
}

//------------------------------------------------------------------------

void WaveFormController::viewWillDelete(CView* view)
{
    if (view == waveform_view)
    {
        waveform_view->unregisterViewListener(this);
        waveform_view = nullptr;
    }
    else if (view == background_view)
    {
        background_view->unregisterViewListener(this);
        background_view = nullptr;
    }
}

//------------------------------------------------------------------------
} // namespace mam
