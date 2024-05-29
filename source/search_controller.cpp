// Copyright(c) 2024 Max And Me.

#include "search_controller.h"
#include "ara_document_controller.h"
#include "public.sdk/source/vst/vstparameters.h"
#include "vstgui/lib/controls/csearchtextedit.h"
#include "vstgui/uidescription/iuidescription.h"
#include "vstgui/uidescription/uiattributes.h"

using namespace VSTGUI;

namespace mam {

//------------------------------------------------------------------------
enum
{
    kSearchFieldTag = 1000,
    kSearchNextTag,
    kSearchPreviousTag,
    kSearchSmartSearchTag
};

//------------------------------------------------------------------------
// SearchController
//------------------------------------------------------------------------
SearchController::SearchController(
    ARADocumentController* controller,
    Steinberg::Vst::Parameter* smart_search_param)
: controller(controller)
, smart_search_param(smart_search_param)
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
}

//------------------------------------------------------------------------
CView* SearchController::createView(const UIAttributes& attributes,
                                    const IUIDescription* description)
{
    return nullptr;
}

//------------------------------------------------------------------------
CView* SearchController::verifyView(CView* view,
                                    const UIAttributes& attributes,
                                    const IUIDescription* description)
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
        else if (*view_name == "SmartSearch")
        {
            if (auto c = dynamic_cast<CControl*>(view))
            {
                c->setTag(kSearchSmartSearchTag);
                c->setListener(this);
                if (smart_search_param)
                    c->setValueNormalized(smart_search_param->getNormalized());
            }
        }
    }

    return view;
}

//------------------------------------------------------------------------
IController*
SearchController::createSubController(UTF8StringPtr name,
                                      const IUIDescription* description)
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
        case kSearchSmartSearchTag: {
            controller->activate_smart_search(control->getValue() > 0);
            smart_search_param->setNormalized(control->getValueNormalized());
            break;
        }
    }
}

//------------------------------------------------------------------------
} // namespace mam
