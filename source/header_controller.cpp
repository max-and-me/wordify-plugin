//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "header_controller.h"
#include "global_options_controller.h"
#include "spinner_view.h"
#include "vstgui/lib/crowcolumnview.h"
#include "vstgui/uidescription/iuidescription.h"
#include "vstgui/uidescription/uiattributes.h"
//------------------------------------------------------------------------
namespace mam {
using namespace ::VSTGUI;

//------------------------------------------------------------------------
// WaveFormController
//------------------------------------------------------------------------
HeaderController::HeaderController(ARADocumentController* controller)
: controller(controller)
{
    if (!controller)
        return;

    word_analysis_progress_observer_id =
        controller->register_word_analysis_progress_observer(
            [this](const auto& data) {
                this->on_word_analysis_progress(data);
            });
}

//------------------------------------------------------------------------
HeaderController::~HeaderController()
{
    controller->unregister_word_analysis_progress_observer(
        word_analysis_progress_observer_id);
}

//------------------------------------------------------------------------
void HeaderController::on_word_analysis_progress(
    const WordAnalysisProgressData& data)
{

    if (data.state != WordAnalysisProgressData::State::kAnalysisStopped)
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
    }
    else
    {
        if (container && spinner_view)
            container->removeView(spinner_view);
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

    if (const auto* view_name =
            attributes.getAttributeValue("custom-view-name"))
    {
        if (*view_name != "HeaderHLayout")
            return view;

        if (auto cnt = view->asViewContainer())
            container = cnt;
    }

    return view;
}

//------------------------------------------------------------------------
VSTGUI::IController*
HeaderController::createSubController(VSTGUI::UTF8StringPtr name,
                                      const VSTGUI::IUIDescription* description)
{
    if (VSTGUI::UTF8StringView(name) == "GlobalOptionsController")
    {
        return new GlobalOptionsController(this->controller);
    }

    return nullptr;
}

//------------------------------------------------------------------------
} // namespace mam
