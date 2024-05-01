//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "header_controller.h"
#include "public.sdk/source/vst/utility/stringconvert.h"
#include "public.sdk/source/vst/vstparameters.h"
#include "spinner_view.h"
#include "vstgui/lib/animation/timingfunctions.h"
#include "vstgui/lib/controls/csearchtextedit.h"
#include "vstgui/lib/controls/ctextlabel.h"
#include "vstgui/uidescription/iuidescription.h"
#include "vstgui/uidescription/uiattributes.h"
#include <limits>

namespace mam {
using namespace ::VSTGUI;

//------------------------------------------------------------------------
// HeaderController
//------------------------------------------------------------------------
HeaderController::HeaderController(ARADocumentController* controller,
                                   Steinberg::Vst::Parameter* param)
: controller(controller)
, task_count_param(param)
{
    if (!controller)
        return;

    if (task_count_param)
        param->addDependent(this);
}

//------------------------------------------------------------------------
HeaderController::~HeaderController()
{
    if (task_count_param)
        task_count_param->removeDependent(this);

    if (spinner_view)
        spinner_view->unregisterViewListener(this);
}

//------------------------------------------------------------------------
void HeaderController::on_task_count_changed()
{
    if (!task_count_param)
        return;

    const auto norm  = task_count_param->getNormalized();
    const auto count = task_count_param->toPlain(norm);

    StringType value_str;
    if (task_count_view)
    {
        Steinberg::Vst::String128 text;
        task_count_param->toString(norm, text);
        value_str = VST3::StringConvert::convert(text);
    }

    on_task_count_changed(size_t(count), value_str);
}

//------------------------------------------------------------------------
void HeaderController::on_task_count_changed(size_t value,
                                             const StringType& value_str)
{
    if (value > 0)
    {
        if (spinner_view)
        {
            spinner_view->setVisible(true);
        }

        if (task_count_view)
        {
            task_count_view->setVisible(true);
            task_count_view->setText(VSTGUI::UTF8String(value_str));
        }
    }
    else
    {
        if (spinner_view)
        {
            spinner_view->setVisible(false);
        }

        if (task_count_view)
        {
            task_count_view->setVisible(false);
            task_count_view->setText(VSTGUI::UTF8String(value_str));
        }
    }
}

//------------------------------------------------------------------------
void PLUGIN_API HeaderController::update(FUnknown* changedUnknown,
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
CView* HeaderController::createView(const VSTGUI::UIAttributes& attributes,
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
HeaderController::verifyView(VSTGUI::CView* view,
                             const VSTGUI::UIAttributes& attributes,
                             const VSTGUI::IUIDescription* description)
{

    if (const auto* view_name = attributes.getAttributeValue("uidesc-label"))
    {
        if (*view_name == "SpinnerView")
        {
            spinner_view = dynamic_cast<SpinnerView*>(view);
            spinner_view->registerViewListener(this);
            if (task_count_param)
                on_task_count_changed();
        }
        else if (*view_name == "TaskCount")
        {
            task_count_view = dynamic_cast<VSTGUI::CTextLabel*>(view);
            if (task_count_view)
                on_task_count_changed();
        }
        else if (*view_name == "search")
        {
            if (!searchField)
            {
                if (auto sf = dynamic_cast<CSearchTextEdit*>(view))
                {
                    searchField = sf;
                    searchField->setTag(kSearchFieldTag);
                    searchField->setListener(this);
                }
            }
        }
    }

    return view;
}

//------------------------------------------------------------------------
VSTGUI::IController*
HeaderController::createSubController(VSTGUI::UTF8StringPtr name,
                                      const VSTGUI::IUIDescription* description)
{
    return nullptr;
}

//------------------------------------------------------------------------
void HeaderController::valueChanged(CControl* control)
{
    switch (control->getTag())
    {
        case kSearchFieldTag: {
            if (auto sf = dynamic_cast<CSearchTextEdit*>(control))
            {
                filterString = sf->getText();
                updateSearchResults();
            }
            break;
        }
    }
}

//------------------------------------------------------------------------
void HeaderController::updateSearchResults()
{
    controller->find_word_in_region(filterString);
}

//------------------------------------------------------------------------
void HeaderController::viewAttached(CView* view)
{
    if (view == spinner_view)
    {
        constexpr auto SPIN_PERIOD_DURATION = 4000.;
        constexpr auto SPIN_FOREVER = std::numeric_limits<int32_t>().max();

        auto* timing_function =
            new VSTGUI::Animation::LinearTimingFunction(SPIN_PERIOD_DURATION);
        auto* repeater = new VSTGUI::Animation::RepeatTimingFunction(
            timing_function, SPIN_FOREVER, false);
        spinner_view->addAnimation(SpinAnimation::ANIMATION_ID,
                                   new SpinAnimation, repeater);
    }
}

//------------------------------------------------------------------------
void HeaderController::viewRemoved(CView* view)
{
    if (view == spinner_view)
    {
        spinner_view->removeAnimation(SpinAnimation::ANIMATION_ID);
    }
}

//------------------------------------------------------------------------
void HeaderController::viewWillDelete(VSTGUI::CView* view)
{
    if (view == spinner_view)
    {
        spinner_view->unregisterViewListener(this);
        spinner_view = nullptr;
    }
}

//------------------------------------------------------------------------

} // namespace mam
