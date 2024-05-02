//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ara_document_controller.h"
#include "base/source/fobject.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/uidescription/icontroller.h"

namespace VSTGUI {
} // namespace VSTGUI

namespace Steinberg::Vst {
class Parameter;
}

namespace mam {
class SpinnerView;

//------------------------------------------------------------------------
// HeaderController
//------------------------------------------------------------------------
class SpinnerController : public Steinberg::FObject,
                          public VSTGUI::IController,
                          public VSTGUI::ViewListenerAdapter
{
public:
    //--------------------------------------------------------------------
    using TextLabel = VSTGUI::CTextLabel;

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
    void valueChanged(VSTGUI::CControl* pControl) override {}
    VSTGUI::IController*
    createSubController(VSTGUI::UTF8StringPtr name,
                        const VSTGUI::IUIDescription* description);

    // IViewListener
    void viewWillDelete(VSTGUI::CView* view) override;

    OBJ_METHODS(SpinnerController, FObject)

    //--------------------------------------------------------------------
private:
    using StringType             = std::string;
    using SpinnerViewListenerPtr = std::unique_ptr<struct SpinnerViewListener>;
    SpinnerViewListenerPtr view_listener;

    void on_task_count_changed();
    void on_task_count_changed(size_t value, const StringType& value_str);

    ARADocumentController* controller = nullptr;
    TextLabel* spinner_badge          = nullptr;
    SpinnerView* spinner_view         = nullptr;
    Steinberg::IPtr<Steinberg::Vst::Parameter> task_counter;
};

//------------------------------------------------------------------------
} // namespace mam
