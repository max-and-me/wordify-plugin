//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "base/source/fobject.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/uidescription/icontroller.h"

namespace VSTGUI {
class CSearchTextEdit;
} // namespace VSTGUI

namespace mam {
class ARADocumentController;

//------------------------------------------------------------------------
// SearchController
//------------------------------------------------------------------------
class SearchController : public Steinberg::FObject, public VSTGUI::IController
{
public:
    //--------------------------------------------------------------------
    SearchController(ARADocumentController* controller);
    ~SearchController() override;

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
    void valueChanged(VSTGUI::CControl* pControl) override;
    VSTGUI::IController*
    createSubController(VSTGUI::UTF8StringPtr name,
                        const VSTGUI::IUIDescription* description) override;

    OBJ_METHODS(SearchController, FObject)

    //--------------------------------------------------------------------
private:
    ARADocumentController* controller = nullptr;
};

//------------------------------------------------------------------------
} // namespace mam
