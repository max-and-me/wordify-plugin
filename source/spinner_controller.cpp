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
#include <limits>

namespace mam {
using namespace ::VSTGUI;

//------------------------------------------------------------------------
struct SpinnerViewListener : VSTGUI::ViewListenerAdapter
{
    void viewAttached(CView* view) override
    {
        if (dynamic_cast<SpinnerView*>(view))
        {
            constexpr auto SPIN_PERIOD_DURATION = 3000.;
            constexpr auto SPIN_FOREVER = std::numeric_limits<int32_t>().max();

            auto* timing_function = new VSTGUI::Animation::LinearTimingFunction(
                SPIN_PERIOD_DURATION);
            auto* repeater = new VSTGUI::Animation::RepeatTimingFunction(
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
, task_count_param(param)
{
    if (!controller)
        return;

    if (task_count_param)
        param->addDependent(this);

    view_listener = std::make_unique<SpinnerViewListener>();
}

//------------------------------------------------------------------------
SpinnerController::~SpinnerController()
{
    if (task_count_param)
        task_count_param->removeDependent(this);

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
    if (!task_count_param)
        return;

    const auto norm  = task_count_param->getNormalized();
    const auto count = task_count_param->toPlain(norm);

    StringType value_str;
    if (spinner_badge)
    {
        Steinberg::Vst::String128 text;
        task_count_param->toString(norm, text);
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
        if (param->getInfo().id == task_count_param->getInfo().id)
        {
            on_task_count_changed();
        }
    }
}

//------------------------------------------------------------------------
CView* SpinnerController::createView(const VSTGUI::UIAttributes& attributes,
                                     const VSTGUI::IUIDescription* description)
{
    if (const auto* view_name = attributes.getAttributeValue("uidesc-label"))
    {
        if (*view_name == "SpinnerView")
        {
            VSTGUI::CPoint origin;
            VSTGUI::CPoint size;
            const auto* size_str = attributes.getAttributeValue("size");
            if (size_str)
                attributes.stringToPoint(*size_str, size);
            const auto* origin_str = attributes.getAttributeValue("origin");
            if (origin_str)
                attributes.stringToPoint(*origin_str, origin);

            const CRect rect{origin, size};
            return new SpinnerView(rect);
        }
    }
    return nullptr;
}

//------------------------------------------------------------------------
VSTGUI::CView*
SpinnerController::verifyView(VSTGUI::CView* view,
                              const VSTGUI::UIAttributes& attributes,
                              const VSTGUI::IUIDescription* description)
{

    if (const auto* view_name = attributes.getAttributeValue("uidesc-label"))
    {
        if (*view_name == "SpinnerView")
        {
            spinner_view = dynamic_cast<SpinnerView*>(view);

            spinner_view->registerViewListener(this); // for lifetime
            if (view_listener)                        // for animations
                spinner_view->registerViewListener(view_listener.get());

            if (task_count_param)
                on_task_count_changed();
        }
        else if (*view_name == "TaskCount")
        {
            spinner_badge = dynamic_cast<VSTGUI::CTextLabel*>(view);
            if (spinner_badge)
                on_task_count_changed();
        }
    }

    return view;
}

//------------------------------------------------------------------------
VSTGUI::IController* SpinnerController::createSubController(
    VSTGUI::UTF8StringPtr name, const VSTGUI::IUIDescription* description)
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
