// Copyright(c) 2024 Max And Me.

#pragma once

#include "ara_document_controller.h"
#include "base/source/fobject.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/uidescription/icontroller.h"

namespace VSTGUI {
class CSearchTextEdit;
} // namespace VSTGUI

namespace Steinberg::Vst {
class Parameter;
}; // namespace Steinberg::Vst

namespace mam {

//------------------------------------------------------------------------
// SearchController
//------------------------------------------------------------------------
class SearchController : public Steinberg::FObject,
                         public VSTGUI::IController,
                         public VSTGUI::ViewListenerAdapter
{
public:
    //--------------------------------------------------------------------
    SearchController(ARADocumentController* controller,
                     Steinberg::Vst::Parameter* smart_search_param);
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

    // ViewListenerAdapter
    void viewWillDelete(VSTGUI::CView* view) override;

    OBJ_METHODS(SearchController, FObject)

    //--------------------------------------------------------------------
private:
    ARADocumentController* controller             = nullptr;
    Steinberg::Vst::Parameter* smart_search_param = nullptr;
    VSTGUI::CSearchTextEdit* search_field         = nullptr;

    RegionChangedCallback::Handle region_props_changed_observer_handle;
    RegionLifetimeCallback::Handle region_lifetime_observer_handle;

    void clear_search();
};

//------------------------------------------------------------------------
} // namespace mam
