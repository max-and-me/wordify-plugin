//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "spinner_controller.h"
#include "public.sdk/source/vst/utility/stringconvert.h"
#include "public.sdk/source/vst/vstparameters.h"
#include "spinner_view.h"
#include "vstgui/lib/animation/timingfunctions.h"
#include "vstgui/lib/controls/ctextlabel.h"
#include "vstgui/uidescription/iuidescription.h"
#include "vstgui/uidescription/uiattributes.h"
#include <cassert>
#include <limits>

using namespace VSTGUI;

namespace mam {

//------------------------------------------------------------------------
auto create_spinner_view(const UIAttributes& attributes,
                         const IUIDescription* description) -> SpinnerView*
{
    CPoint origin;
    CPoint size;
    bool res = attributes.getPointAttribute("origin", origin);
    res      = res && attributes.getPointAttribute("size", size);

    assert(res && "origin and size must be available!");

    const CRect rect{origin, size};
    return new SpinnerView(rect);
}

//------------------------------------------------------------------------
// SpinnerViewListener
//------------------------------------------------------------------------
struct SpinnerViewListener : ViewListenerAdapter
{
    void viewAttached(CView* view) override
    {
        if (dynamic_cast<SpinnerView*>(view))
        {
            constexpr auto SPIN_PERIOD_DURATION = 3000.;
            constexpr auto SPIN_FOREVER = std::numeric_limits<int32_t>().max();

            auto* timing_function =
                new Animation::LinearTimingFunction(SPIN_PERIOD_DURATION);
            auto* repeater = new Animation::RepeatTimingFunction(
                timing_function, SPIN_FOREVER, false);
            view->addAnimation(SpinAnimation::ANIMATION_ID, new SpinAnimation,
                               repeater);
        }
    }

    void viewRemoved(CView* view) override
    {
        if (dynamic_cast<SpinnerView*>(view))
        {
            view->removeAnimation(SpinAnimation::ANIMATION_ID);
        }
    }
};

//------------------------------------------------------------------------
// SpinnerController
//------------------------------------------------------------------------
SpinnerController::SpinnerController(ARADocumentController* controller,
                                     Steinberg::Vst::Parameter* param)
: controller(controller)
, task_counter(param)
{
    if (!controller)
        return;

    if (task_counter)
        param->addDependent(this);

    view_listener = std::make_unique<SpinnerViewListener>();
}

//------------------------------------------------------------------------
SpinnerController::~SpinnerController()
{
    if (task_counter)
        task_counter->removeDependent(this);

    if (spinner_view)
    {
        spinner_view->unregisterViewListener(this);
        if (view_listener)
            spinner_view->unregisterViewListener(view_listener.get());
    }
}

//------------------------------------------------------------------------
void SpinnerController::on_task_count_changed()
{
    if (!task_counter)
        return;

    const auto norm  = task_counter->getNormalized();
    const auto count = task_counter->toPlain(norm);

    StringType value_str;
    if (spinner_badge)
    {
        Steinberg::Vst::String128 text;
        task_counter->toString(norm, text);
        value_str = VST3::StringConvert::convert(text);
    }

    on_task_count_changed(size_t(count), value_str);
}

//------------------------------------------------------------------------
void SpinnerController::on_task_count_changed(size_t value,
                                              const StringType& value_str)
{
    if (value > 0)
    {
        if (spinner_view)
        {
            spinner_view->setVisible(true);
        }

        if (spinner_badge)
        {
            spinner_badge->setVisible(true);
            spinner_badge->setText(VSTGUI::UTF8String(value_str));
        }
    }
    else
    {
        if (spinner_view)
        {
            spinner_view->setVisible(false);
        }

        if (spinner_badge)
        {
            spinner_badge->setVisible(false);
            spinner_badge->setText(VSTGUI::UTF8String(value_str));
        }
    }
}

//------------------------------------------------------------------------
void PLUGIN_API SpinnerController::update(FUnknown* changedUnknown,
                                          Steinberg::int32 message)
{
    if (auto* param =
            Steinberg::FCast<Steinberg::Vst::Parameter>(changedUnknown))
    {
        if (param->getInfo().id == task_counter->getInfo().id)
        {
            on_task_count_changed();
        }
    }
}

//------------------------------------------------------------------------
CView* SpinnerController::createView(const UIAttributes& attributes,
                                     const IUIDescription* description)
{
    if (const auto* view_name = attributes.getAttributeValue("uidesc-label"))
    {
        if (*view_name == "SpinnerView")
        {
            return create_spinner_view(attributes, description);
        }
    }
    return nullptr;
}

//------------------------------------------------------------------------
VSTGUI::CView* SpinnerController::verifyView(VSTGUI::CView* view,
                                             const UIAttributes& attributes,
                                             const IUIDescription* description)
{

    if (const auto* view_name = attributes.getAttributeValue("uidesc-label"))
    {
        if (*view_name == "SpinnerView")
        {
            spinner_view = dynamic_cast<SpinnerView*>(view);

            // Lifetime handling
            spinner_view->registerViewListener(this);

            // Animations handling
            if (view_listener)
                spinner_view->registerViewListener(view_listener.get());

            if (task_counter)
                on_task_count_changed();
        }
        else if (*view_name == "SpinnerBadge")
        {
            spinner_badge = dynamic_cast<VSTGUI::CTextLabel*>(view);
            if (spinner_badge)
                on_task_count_changed();
        }
    }

    return view;
}

//------------------------------------------------------------------------
VSTGUI::IController*
SpinnerController::createSubController(UTF8StringPtr name,
                                       const IUIDescription* description)
{
    return nullptr;
}

//------------------------------------------------------------------------
void SpinnerController::viewWillDelete(VSTGUI::CView* view)
{
    if (spinner_view == view)
    {
        if (view_listener)
            spinner_view->unregisterViewListener(view_listener.get());

        spinner_view->unregisterViewListener(this);
        spinner_view = nullptr;
    }
    else if (spinner_badge == view)
    {
        spinner_badge = nullptr;
    }
}

//------------------------------------------------------------------------

} // namespace mam
