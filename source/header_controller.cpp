//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "header_controller.h"
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
// HeaderController
//------------------------------------------------------------------------
HeaderController::HeaderController(ARADocumentController* controller)
: controller(controller)
{
    if (!controller)
        return;
}

//------------------------------------------------------------------------
HeaderController::~HeaderController() {}

//------------------------------------------------------------------------
void PLUGIN_API HeaderController::update(FUnknown* changedUnknown,
                                         Steinberg::int32 message)
{
    if (auto* param =
            Steinberg::FCast<Steinberg::Vst::Parameter>(changedUnknown))
    {
    }
}

//------------------------------------------------------------------------
CView* HeaderController::createView(const VSTGUI::UIAttributes& attributes,
                                    const VSTGUI::IUIDescription* description)
{
    return nullptr;
}

//------------------------------------------------------------------------
VSTGUI::CView*
HeaderController::verifyView(VSTGUI::CView* view,
                             const VSTGUI::UIAttributes& attributes,
                             const VSTGUI::IUIDescription* description)
{
    if (const auto* view_name = attributes.getAttributeValue("uidesc-label"))
    {
        if (*view_name == "search")
        {
            if (!searchField)
            {
                if (auto sf = dynamic_cast<CSearchTextEdit*>(view))
                {
                    searchField = sf;
                    searchField->setTag(kSearchFieldTag);
                    searchField->setListener(this);
                }
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
HeaderController::createSubController(VSTGUI::UTF8StringPtr name,
                                      const VSTGUI::IUIDescription* description)
{
    return nullptr;
}

//------------------------------------------------------------------------
void HeaderController::valueChanged(CControl* control)
{
    switch (control->getTag())
    {
        case kSearchFieldTag: {
            if (auto sf = dynamic_cast<CSearchTextEdit*>(control))
            {
                filterString = sf->getText();
                updateSearchResults();
            }
            break;
        }
        case kSearchNextTag: {
            if (control->getValue() == control->getMax())
                selectNextSearch ();
            break;
        }
        case kSearchPreviousTag: {
            if (control->getValue() == control->getMax())
                selectPreviousSearch ();
            break;
        }
    }
}

//------------------------------------------------------------------------
void HeaderController::updateSearchResults()
{
    controller->find_word_in_region(filterString);
}

//------------------------------------------------------------------------
void HeaderController::selectNextSearch()
{
}

//------------------------------------------------------------------------
void HeaderController::selectPreviousSearch()
{
}

//------------------------------------------------------------------------
void HeaderController::viewWillDelete(VSTGUI::CView* view)
{
    if (searchField == view)
    {
        searchField = nullptr;
    }
}

//------------------------------------------------------------------------

} // namespace mam
