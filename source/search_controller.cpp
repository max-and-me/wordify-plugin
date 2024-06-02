// Copyright(c) 2024 Max And Me.

#include "search_controller.h"
#include "ara_document_controller.h"
#include "public.sdk/source/vst/vstparameters.h"
#include "search_engine.h"
#include "string_matcher.h"
#include "vstgui/lib/controls/csearchtextedit.h"
#include "vstgui/uidescription/iuidescription.h"
#include "vstgui/uidescription/uiattributes.h"

using namespace VSTGUI;
using namespace Steinberg;

namespace mam {

//------------------------------------------------------------------------
enum
{
    kSearchFieldTag = 1000,
};

//------------------------------------------------------------------------
auto string_match_method(Steinberg::Vst::Parameter* smart_search_param)
{
    bool mode = smart_search_param->getNormalized() > 0.;
    return mode ? StringMatcher::MatchMethod::nearbyFuzzyMatch
                : StringMatcher::MatchMethod::directMatch;
}

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

    SearchEngine::instance().get_regions = [controller]() {
        return controller->get_playback_regions();
    };

    if (smart_search_param)
        smart_search_param->addDependent(this);
}

//------------------------------------------------------------------------
SearchController::~SearchController()
{
    if (smart_search_param)
        smart_search_param->removeDependent(this);

    if (SearchEngine::instance().get_regions)
        SearchEngine::instance().get_regions = nullptr;
}

//------------------------------------------------------------------------
void PLUGIN_API SearchController::update(FUnknown* changedUnknown,
                                         Steinberg::int32 message)
{
    if (auto* param = FCast<Vst::Parameter>(changedUnknown))
    {
        if (param == smart_search_param)
        {
            const auto smm = string_match_method(smart_search_param);
            SearchEngine::instance().research(
                [&](const auto& s0, const auto& s1) -> bool {
                    return StringMatcher::isMatch(s0, s1, smm);
                });
        }
    }
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
                c->setText(SearchEngine::instance().current_search_word());
                c->setTag(kSearchFieldTag);
                c->setListener(this);
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
            {
                const auto search_word = sf->getText().getString();
                if (search_word.empty())
                    SearchEngine::instance().clear_results();
                else
                {
                    const auto smm = string_match_method(smart_search_param);
                    SearchEngine::instance().search(
                        search_word,
                        [&](const auto& s0, const auto& s1) -> bool {
                            return StringMatcher::isMatch(s0, s1, smm);
                        });
                }
            }

            break;
        }
    }
}

//------------------------------------------------------------------------
} // namespace mam
