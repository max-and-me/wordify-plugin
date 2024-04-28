//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ara_document_controller.h"
#include "base/source/fobject.h"
#include "vstgui/uidescription/icontroller.h"

namespace VSTGUI {
class CRowColLayout;
}
namespace mam {
class SpinnerView;
//------------------------------------------------------------------------
// HeaderController
//------------------------------------------------------------------------
class HeaderController : public Steinberg::FObject, public VSTGUI::IController
{
public:
    //--------------------------------------------------------------------
    HeaderController(ARADocumentController* controller);
    ~HeaderController() override;

    void PLUGIN_API update(FUnknown* changedUnknown,
                           Steinberg::int32 message) override {};

    VSTGUI::CView*
    createView(const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;

    VSTGUI::CView*
    verifyView(VSTGUI::CView* view,
               const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;

    // IControlListener
    void valueChanged(VSTGUI::CControl* pControl) override {};
    VSTGUI::IController*
    createSubController(VSTGUI::UTF8StringPtr name,
                        const VSTGUI::IUIDescription* description);

    OBJ_METHODS(HeaderController, FObject)

    //--------------------------------------------------------------------
private:
    void on_word_analysis_progress(const meta_words::WordAnalysisProgressData& data);

    ARADocumentController* controller    = nullptr;
    VSTGUI::CViewContainer* container    = nullptr;
    SpinnerView* spinner_view            = nullptr;
    VSTGUI::CRowColLayout* rowcol_parent = nullptr;
    tiny_observer_pattern::ObserverID word_analysis_progress_observer_id = 0;
};

//------------------------------------------------------------------------
} // namespace mam
