//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "vstgpt_listcontroller.h"
#include "vstgpt_context.h"

#include "vstgui/lib/controls/clistcontrol.h"
#include "vstgui/lib/controls/cstringlist.h"
#include "vstgui/lib/controls/icontrollistener.h"
#include "vstgui/lib/platform/platformfactory.h"
#include "vstgui/lib/events.h"
#include "vstgui/lib/cstring.h"

//------------------------------------------------------------------------
namespace mam {
using namespace::VSTGUI;

//------------------------------------------------------------------------
VstGPTListController::VstGPTListController()
{
}

//------------------------------------------------------------------------
VstGPTListController::~VstGPTListController()
{
}

//------------------------------------------------------------------------
void VstGPTListController::valueChanged(CControl* pControl)
{
    if (pControl && pControl == listControl)
    {
        VstGPTContext* context = VstGPTContext::getInstance();
        context->onRequestSelectWord(listControl->getValue());
    }
}

//------------------------------------------------------------------------
CView* VstGPTListController::verifyView(CView* view,
                                                 const UIAttributes& /*attributes*/,
                                                 const IUIDescription* /*description*/)
{
    
    if (!listControl)
        listControl = dynamic_cast<CListControl*>(view);
    
    if (listControl)
    {
        VstGPTContext* context = VstGPTContext::getInstance();
        
        listControl->setMax(context->getData().words.size()-1);
        listControl->recalculateLayout();
        listControl->registerControlListener (this);
        
        if (auto stringListDrawer =  dynamic_cast<StringListControlDrawer*> (listControl->getDrawer ()))
        {
            stringListDrawer->setStringProvider([context] (int32_t row) {
                meta_words::MetaWords words = context->getData().words;
                meta_words::MetaWord word = words.at (row);
                std::string name = word.word;
                
                UTF8String string (name.data());
                return getPlatformFactory().createString (string);
            });
        }
    }
	return view;
}
//------------------------------------------------------------------------

} // mam
