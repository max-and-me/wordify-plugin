// Copyright (c) 2023-present, WordifyOrg.

#pragma once

#include "ara_document_controller.h"
#include "warn_cpp/suppress_warnings.h"
#include "wordify_types.h"
BEGIN_SUPPRESS_WARNINGS
#include "base/source/fobject.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/uidescription/icontroller.h"
END_SUPPRESS_WARNINGS

namespace VSTGUI {
class CViewContainer;
} // namespace VSTGUI

namespace Steinberg::Vst {
class Parameter;
}

namespace mam {
class SpinnerView;

//------------------------------------------------------------------------
// SpinnerController
//------------------------------------------------------------------------
class SpinnerController : public Steinberg::FObject,
                          public VSTGUI::IController,
                          public VSTGUI::ViewListenerAdapter
{
public:
    //--------------------------------------------------------------------
    using TextLabel     = VSTGUI::CTextLabel;
    using ViewContainer = VSTGUI::CViewContainer;

    SpinnerController(ARADocumentController* controller,
                      Steinberg::Vst::Parameter* param);
    ~SpinnerController() override;

    void PLUGIN_API update(FUnknown* changedUnknown,
                           Steinberg::int32 message) override;

    VSTGUI::CView*
    createView(const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;

    VSTGUI::CView*
    verifyView(VSTGUI::CView* view,
               const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;

    // IControlListener
    void valueChanged(VSTGUI::CControl* /*pControl*/) override {}
    VSTGUI::IController*
    createSubController(VSTGUI::UTF8StringPtr name,
                        const VSTGUI::IUIDescription* description) override;

    // IViewListener
    void viewWillDelete(VSTGUI::CView* view) override;
    void viewAttached(VSTGUI::CView* view) override;

    OBJ_METHODS(SpinnerController, FObject)

    //--------------------------------------------------------------------
private:
    size_t count_tasks() const;
    void on_task_count_changed();
    void on_task_count_changed(size_t value, const StringType& value_str);

    ARADocumentController* controller = nullptr;
    ViewContainer* spinner_layout     = nullptr;
    TextLabel* spinner_badge          = nullptr;
    SpinnerView* spinner_view         = nullptr;
    Steinberg::IPtr<Steinberg::Vst::Parameter> task_counter;
};

//------------------------------------------------------------------------
} // namespace mam
