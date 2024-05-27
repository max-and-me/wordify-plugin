//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "search_controller.h"
#include "public.sdk/source/vst/utility/stringconvert.h"
#include "public.sdk/source/vst/vstparameters.h"
#include "spinner_view.h"
#include "vstgui/lib/animation/timingfunctions.h"
#include "vstgui/lib/controls/csearchtextedit.h"
#include "vstgui/lib/controls/ctextlabel.h"
#include "vstgui/uidescription/iuidescription.h"
#include "vstgui/uidescription/uiattributes.h"
#include <limits>

namespace mam {
using namespace ::VSTGUI;

//------------------------------------------------------------------------
// SearchController
//------------------------------------------------------------------------
SearchController::SearchController(ARADocumentController* controller)
: controller(controller)
{
    if (!controller)
        return;
}

//------------------------------------------------------------------------
SearchController::~SearchController()
{
    changed(kWillDestroy);
}

//------------------------------------------------------------------------
void PLUGIN_API SearchController::update(FUnknown* changedUnknown,
                                         Steinberg::int32 message)
{
    if (auto* param =
            Steinberg::FCast<Steinberg::Vst::Parameter>(changedUnknown))
    {
    }
}

//------------------------------------------------------------------------
CView* SearchController::createView(const VSTGUI::UIAttributes& attributes,
                                    const VSTGUI::IUIDescription* description)
{
    return nullptr;
}

//------------------------------------------------------------------------
VSTGUI::CView*
SearchController::verifyView(VSTGUI::CView* view,
                             const VSTGUI::UIAttributes& attributes,
                             const VSTGUI::IUIDescription* description)
{
    if (const auto* view_name = attributes.getAttributeValue("uidesc-label"))
    {
        if (*view_name == "Search")
        {
            if (auto c = dynamic_cast<CSearchTextEdit*>(view))
            {
                c->setTag(kSearchFieldTag);
                c->setListener(this);
            }
        }
        else if (*view_name == "Next")
        {
            if (auto c = dynamic_cast<CControl*>(view))
            {
                c->setTag(kSearchNextTag);
                c->setListener(this);
            }
        }
        else if (*view_name == "Previous")
        {
            if (auto c = dynamic_cast<CControl*>(view))
            {
                c->setTag(kSearchPreviousTag);
                c->setListener(this);
            }
        }
    }

    return view;
}

//------------------------------------------------------------------------
VSTGUI::IController*
SearchController::createSubController(VSTGUI::UTF8StringPtr name,
                                      const VSTGUI::IUIDescription* description)
{
    return nullptr;
}

//------------------------------------------------------------------------
void SearchController::valueChanged(CControl* control)
{
    switch (control->getTag())
    {
        case kSearchFieldTag: {
            if (auto sf = dynamic_cast<CSearchTextEdit*>(control))
                controller->search_word(sf->getText().getString());

            break;
        }
        case kSearchNextTag: {
            if (control->getValue() == control->getMax())
                controller->focus_next_occurence();

            break;
        }
        case kSearchPreviousTag: {
            if (control->getValue() == control->getMax())
                controller->focus_prev_occurence();

            break;
        }
    }
}

//------------------------------------------------------------------------

} // namespace mam
