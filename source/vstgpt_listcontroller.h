//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "vstgui/uidescription/icontroller.h"
#include "base/source/fobject.h"

namespace mam {

class VstGPTListController : public Steinberg::FObject, public VSTGUI::IController
{
public:
//------------------------------------------------------------------------
    VstGPTListController ();
	virtual ~VstGPTListController ();

    void PLUGIN_API update (FUnknown* changedUnknown, Steinberg::int32 message) override {};
	VSTGUI::CView* verifyView (VSTGUI::CView* view, const VSTGUI::UIAttributes& attributes,
	                           const VSTGUI::IUIDescription* description) override;
    void valueChanged (VSTGUI::CControl* pControl) override {};
    void controlBeginEdit (VSTGUI::CControl* pControl) override {};
    void controlEndEdit (VSTGUI::CControl* pControl) override {};

	OBJ_METHODS (VstGPTListController, FObject)
//------------------------------------------------------------------------
private:
	VSTGUI::CControl* sizeControl = nullptr;
};

//------------------------------------------------------------------------
} // mam
