//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "header_controller.h"
#include "public.sdk/source/vst/utility/stringconvert.h"
#include "public.sdk/source/vst/vstparameters.h"
#include "spinner_view.h"
#include "vstgui/lib/controls/csearchtextedit.h"
#include "vstgui/lib/controls/ctextlabel.h"
#include "vstgui/uidescription/iuidescription.h"
#include "vstgui/uidescription/uiattributes.h"

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

    /* word_analysis_progress_observer_id =
         controller->register_word_analysis_progress_observer(
             [this](const auto& data) {
                 this->on_word_analysis_progress(data);
             });*/

    if (task_count_param)
        param->addDependent(this);
}

//------------------------------------------------------------------------
HeaderController::~HeaderController()
{
    if (task_count_param)
        task_count_param->removeDependent(this);

    /* controller->unregister_word_analysis_progress_observer(
         word_analysis_progress_observer_id);*/
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
        if (container)
        {
            if (!spinner_view)
            {
                const auto view_size = CPoint({40., 40.});
                spinner_view =
                    new SpinnerView(CRect{0, 0, view_size.x, view_size.y});
                container->addView(spinner_view);
            }
        }
        if (task_count_view)
        {
            task_count_view->setVisible(true);
            task_count_view->setText(VSTGUI::UTF8String(value_str));
        }
    }
    else
    {
        if (container && spinner_view)
            container->removeView(spinner_view);

        if (task_count_view)
        {
            task_count_view->setVisible(false);
            task_count_view->setText(VSTGUI::UTF8String(value_str));
        }
    }
}

//------------------------------------------------------------------------
void HeaderController::on_word_analysis_progress(
    const meta_words::WordAnalysisProgressData& data)
{
    using State = meta_words::WordAnalysisProgressData::State;

    switch (data.state)
    {
        case State::kAnalysisStarted: {
            if (container)
            {
                if (!spinner_view)
                {
                    const auto view_size = CPoint({40., 40.});
                    spinner_view =
                        new SpinnerView(CRect{0, 0, view_size.x, view_size.y});
                    container->addView(spinner_view);
                }
            }
            break;
        }

        case State::kAnalysisRunning: {
            // TODO: Do something here!
            break;
        }

        case State::kAnalysisStopped: {
            if (container && spinner_view)
                container->removeView(spinner_view);
            break;
        }

        default:
            break;
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
        if (*view_name == "SpinnerHLayout")
        {
            if (auto cnt = view->asViewContainer())
                container = cnt;

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

} // namespace mam
