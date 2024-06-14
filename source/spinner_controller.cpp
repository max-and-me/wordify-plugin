// Copyright(c) 2024 Max And Me.

#include "spinner_controller.h"
#include "spinner_view.h"
#include <cassert>
#include <limits>
BEGIN_SUPRESS_WARNINGS
#include "public.sdk/source/vst/utility/stringconvert.h"
#include "public.sdk/source/vst/vstparameters.h"
#include "view_animations.h"
#include "vstgui/lib/animation/timingfunctions.h"
#include "vstgui/lib/controls/ctextlabel.h"
#include "vstgui/uidescription/iuidescription.h"
#include "vstgui/uidescription/uiattributes.h"
END_SUPRESS_WARNINGS

using namespace VSTGUI;

namespace mam {
namespace {
//------------------------------------------------------------------------
auto create_spinner_view(const UIAttributes& attributes,
                         const IUIDescription* /*description*/) -> SpinnerView*
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
auto count_num_tasks(Steinberg::IPtr<Steinberg::Vst::Parameter> task_counter,
                     SpinnerController::StringType& value_str) -> size_t
{
    const auto norm  = task_counter->getNormalized();
    const auto count = task_counter->toPlain(norm);

    Steinberg::Vst::String128 text;
    task_counter->toString(norm, text);
    value_str = VST3::StringConvert::convert(text);

    return size_t(count);
}

//------------------------------------------------------------------------
} // namespace

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

    void viewWillDelete(CView* view) override
    {
        if (dynamic_cast<SpinnerView*>(view))
        {
            view->unregisterViewListener(this);
            delete this;
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
}

//------------------------------------------------------------------------
SpinnerController::~SpinnerController()
{
    if (task_counter)
        task_counter->removeDependent(this);

    if (spinner_view)
        spinner_view->unregisterViewListener(this);

    if (spinner_badge)
        spinner_badge->unregisterViewListener(this);

    if (spinner_layout)
        spinner_layout->unregisterViewListener(this);
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
    if (spinner_layout)
    {
        const auto fade_in = value > 0;
        spinner_layout->forEachChild([&](CView* child) {
            fade_in ? animations::add_simple_fade_in(child)
                    : animations::add_simple_fade_out(child);
        });
    }

    if (spinner_badge)
        spinner_badge->setText(VSTGUI::UTF8String(value_str));
}

//------------------------------------------------------------------------
auto SpinnerController::count_tasks() const -> size_t
{
    StringType str;
    return count_num_tasks(task_counter, str);
}

//------------------------------------------------------------------------
void PLUGIN_API SpinnerController::update(FUnknown* changedUnknown,
                                          Steinberg::int32 /*message*/)
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
                                             const IUIDescription* /*description*/)
{
    if (!view)
        return view;

    if (const auto* view_name = attributes.getAttributeValue("uidesc-label"))
    {
        if (*view_name == "SpinnerView")
        {
            spinner_view = dynamic_cast<SpinnerView*>(view);

            // Lifetime handling
            spinner_view->registerViewListener(this);

            // Animations handling
            spinner_view->registerViewListener(new SpinnerViewListener);
        }
        else if (*view_name == "SpinnerBadge")
        {
            spinner_badge = dynamic_cast<VSTGUI::CTextLabel*>(view);

            // Lifetime handling
            spinner_badge->registerViewListener(this);
        }
        else if (*view_name == "SpinnerLayout")
        {
            spinner_layout = view->asViewContainer();

            // Lifetime handling
            spinner_layout->registerViewListener(this);
        }
    }

    return view;
}

//------------------------------------------------------------------------
VSTGUI::IController*
SpinnerController::createSubController(UTF8StringPtr /*name*/,
                                       const IUIDescription* /*description*/)
{
    return nullptr;
}

//------------------------------------------------------------------------
void SpinnerController::viewAttached(CView* view)
{
    if (view == spinner_badge)
    {
        StringType str;
        const auto num_tasks = count_num_tasks(task_counter, str);
        spinner_badge->setText(str.c_str());

        view->setAlphaValue(num_tasks > 0 ? 1. : 0.);
    }

    if (view == spinner_view)
        view->setAlphaValue(count_tasks() > 0 ? 1. : 0.);
}

//------------------------------------------------------------------------
void SpinnerController::viewWillDelete(VSTGUI::CView* view)
{
    if (spinner_view == view)
    {
        spinner_view->unregisterViewListener(this);
        spinner_view = nullptr;
    }
    else if (spinner_badge == view)
    {
        spinner_badge->unregisterViewListener(this);
        spinner_badge = nullptr;
    }
    else if (spinner_layout == view)
    {
        spinner_layout->unregisterViewListener(this);
        spinner_layout = nullptr;
    }
}

//------------------------------------------------------------------------

} // namespace mam
