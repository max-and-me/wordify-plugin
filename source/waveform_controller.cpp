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
WaveFormController::WaveFormController(ARADocumentController* controller)
: controller(controller)
{
}

//------------------------------------------------------------------------
WaveFormController::~WaveFormController()
{
    if (subject)
        subject->remove(observer_handle);

    unregister_current_region_props_observer();

    if (waveform_view)
        waveform_view->unregisterViewListener(this);

    if (background_view)
        background_view->unregisterViewListener(this);
}

//------------------------------------------------------------------------
bool WaveFormController::initialize(Subject* _subject, FuncWaveFormData&& func)
{
    if (!_subject)
        return false;

    subject            = _subject;
    waveform_data_func = std::move(func);
    observer_handle    = subject->append(
        [&](const auto& data) { this->on_selected_region_word(data); });

    return true;
}

//------------------------------------------------------------------------
void WaveFormController::on_selected_region_word(
    const SelectedWordEventData& new_selected_word)
{
    if (selected_word.region_id != new_selected_word.region_id)
    {
        unregister_current_region_props_observer();
        register_region_props_observer(new_selected_word);
    }

    update_waveform();
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
void WaveFormController::update_waveform()
{
    if (waveform_view)
        waveform_view->invalid();

    const auto data = this->waveform_data_func();
    update_background_view(background_view, data);
}

//------------------------------------------------------------------------
void WaveFormController::register_region_props_observer(
    const SelectedWordEventData& new_selected_word)
{
    if (!controller)
        return;

    selected_word = new_selected_word;

    region_props_observer_handle =
        controller->get_playback_region_changed_subject(selected_word.region_id)
            .append([&]() { update_waveform(); });
}

//------------------------------------------------------------------------
void WaveFormController::unregister_current_region_props_observer()
{
    if (!controller)
        return;

    controller->get_playback_region_changed_subject(selected_word.region_id)
        .remove(region_props_observer_handle);
}

//------------------------------------------------------------------------
} // namespace mam
