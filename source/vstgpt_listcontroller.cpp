//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "vstgpt_listcontroller.h"

#include "vstgui/lib/controls/clistcontrol.h"
#include "vstgui/lib/controls/cstringlist.h"
#include "vstgui/lib/platform/platformfactory.h"
#include "vstgui/lib/cstring.h"

//------------------------------------------------------------------------
namespace mam {
using namespace::VSTGUI;
//------------------------------------------------------------------------
VstGPTListController::VstGPTListController ()
{
}

//------------------------------------------------------------------------
VstGPTListController::~VstGPTListController ()
{
	
}

//------------------------------------------------------------------------
CView* VstGPTListController::verifyView (CView* view,
                                                 const UIAttributes& /*attributes*/,
                                                 const IUIDescription* /*description*/)
{

    auto* listControl = dynamic_cast<CListControl*> (view);
    
    if (listControl)
    {
        listControl->setMax(12);
        listControl->recalculateLayout();
        
        if (auto stringListDrawer =  dynamic_cast<StringListControlDrawer*> (listControl->getDrawer ()))
        {
            stringListDrawer->setStringProvider([] (int32_t row) {
                UTF8String string ("FooBarBaz");
                return getPlatformFactory ().createString (string);
            });
        }
        
    }
	return view;
}
//------------------------------------------------------------------------

} // mam
