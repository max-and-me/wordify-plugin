// Copyright(c) 2024 Max And Me.

#include "warn_cpp/suppress_warnings.h"
#include "word_button.h"
BEGIN_SUPPRESS_WARNINGS
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/uidescription/uidescription.h"
END_SUPPRESS_WARNINGS

using namespace VSTGUI;

namespace mam {
namespace {

//------------------------------------------------------------------------
auto drawBackground(CDrawContext* context,
                    const CRect& rect,
                    const CCoord radius,
                    const CColor background) -> void
{
    context->setFillColor(background);
    if (auto path = owned(context->createRoundRectGraphicsPath(rect, radius)))
        context->drawGraphicsPath(path);
}

//------------------------------------------------------------------------
} // namespace

//------------------------------------------------------------------------
// WordButton
//------------------------------------------------------------------------
WordButton::WordButton(const CRect& size,
                       IControlListener* listener,
                       int32_t tag,
                       UTF8StringPtr title,
                       Style style)
: CTextButton(size, listener, tag, title, style)
{
}

//------------------------------------------------------------------------
void WordButton::draw(CDrawContext* context)
{
    if (state != State::kNone)
    {
        drawBackground(context, getViewSize(), getRoundRadius(),
                       currentBgrColor);
    }

    CTextButton::draw(context);
}

//------------------------------------------------------------------------
bool WordButton::setState(State new_state)
{
    const bool changed = state != new_state;
    state              = new_state;
    if (!changed)
        return false;

    switch (state)
    {
        case State::kNone: {
            setTextColor(normalTextColor);
            currentBgrColor = VSTGUI::kTransparentCColor;
            break;
        }

        case State::kSearched: {
            setTextColor(searchedTextColor);
            currentBgrColor = searchedBgrColor;
            break;
        }

        case State::kFocused: {
            setTextColor(focusedTextColor);
            currentBgrColor = focusedBgrColor;
            break;
        }

        default: {
            setTextColor(normalTextColor);
            currentBgrColor = VSTGUI::kTransparentCColor;
            break;
        }
    }

    return changed;
}

//------------------------------------------------------------------------
void WordButton::verifyTextButtonView(const VSTGUI::IUIDescription* description)
{
    description->getColor("search_hilite_bgr_color", searchedBgrColor);
    description->getColor("search_hilite_text_color", searchedTextColor);
    description->getColor("search_select_hilite_bgr_color", focusedBgrColor);
    description->getColor("search_select_hilite_text_color", focusedTextColor);
    description->getColor("transcript_text_color", normalTextColor);
}

//------------------------------------------------------------------------
} // namespace mam
