//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "header_controller.h"
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

    word_analysis_start_stop_observer_id =
        controller->register_word_analysis_start_stop_observer(
            [this](const auto& data) {
                this->on_word_analysis_start_stop(data);
            });
}

//------------------------------------------------------------------------
HeaderController::~HeaderController()
{
    controller->unregister_word_analysis_start_stop_observer(
        word_analysis_start_stop_observer_id);
}

//------------------------------------------------------------------------
void HeaderController::on_word_analysis_start_stop(
    const WordAnalysisStartStopData& data)
{
    bool test = true;
    test      = false;
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

        if (auto container = view->asViewContainer())
        {
            if (!spinner_view)
            {
                /*const auto view_size = CPoint({65., 65.});
                spinner_view =
                    new SpinnerView(CRect{0, 0, view_size.x, view_size.y});
                container->addView(spinner_view);*/
            }
        }
    }

    return view;
}

//------------------------------------------------------------------------
} // namespace mam
