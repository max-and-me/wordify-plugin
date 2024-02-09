//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "vstgpt_listcontroller.h"
#include "vstgpt_context.h"

#include "vstgui/lib/controls/clistcontrol.h"
#include "vstgui/lib/controls/cstringlist.h"
#include "vstgui/lib/controls/icontrollistener.h"
#include "vstgui/lib/cstring.h"
#include "vstgui/lib/events.h"
#include "vstgui/lib/platform/platformfactory.h"

//------------------------------------------------------------------------
namespace mam {
using namespace ::VSTGUI;

//------------------------------------------------------------------------
VstGPTListController::VstGPTListController(VstGPTContext* context)
: context(context)
{
    if (context)
        context->registerContextListener(this);
}

//------------------------------------------------------------------------
VstGPTListController::~VstGPTListController()
{
    if (context)
        context->unregisterContextListener(this);
}

//------------------------------------------------------------------------
void VstGPTListController::valueChanged(CControl* pControl)
{
    if (pControl && pControl == listControl)
    {
        if (context)
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

    if (!context)
        return view;

    return view;
    if (listControl)
    {
        listControl->setMax(context->getData().words.size() - 1);
        listControl->recalculateLayout();
        listControl->registerControlListener(this);

        if (auto stringListDrawer = dynamic_cast<StringListControlDrawer*>(
                listControl->getDrawer()))
        {
            stringListDrawer->setStringProvider(
                [context = this->context](int32_t row) {
                    meta_words::MetaWords words = context->getData().words;
                    meta_words::MetaWord word   = words.at(row);
                    std::string name            = word.word;

                    UTF8String string(name.data());
                    return getPlatformFactory().createString(string);
                });
        }
    }
    return view;
}

//------------------------------------------------------------------------
void VstGPTListController::onDataChanged()
{
    if (listControl)
    {
        listControl->setMax(context->getData().words.size() - 1);
        listControl->recalculateLayout();
        listControl->registerControlListener(this);

        if (auto stringListDrawer = dynamic_cast<StringListControlDrawer*>(
                listControl->getDrawer()))
        {
            stringListDrawer->setStringProvider(
                [context = this->context](int32_t row) {
                    meta_words::MetaWords words = context->getData().words;
                    meta_words::MetaWord word   = words.at(row);
                    std::string name            = word.word;

                    UTF8String string(name.data());
                    return getPlatformFactory().createString(string);
                });
        }
    }
}
//------------------------------------------------------------------------

} // namespace mam
