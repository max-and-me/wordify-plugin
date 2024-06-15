// Copyright(c) 2024 Max And Me.

#include "word_button.h"
BEGIN_SUPRESS_WARNINGS
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/uidescription/uidescription.h"
END_SUPRESS_WARNINGS

using namespace VSTGUI;

namespace mam {
namespace {

//------------------------------------------------------------------------
auto drawHiliteBackground(CDrawContext* context,
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
        drawHiliteBackground(context, getViewSize(), getRoundRadius(),
                             currentBackgroundColor);

    CTextButton::draw(context);
}

//------------------------------------------------------------------------
bool WordButton::setState(State state_)
{
    const bool changed = state != state_;
    state              = state_;
    if (!changed)
        return false;

    switch (state)
    {
        case mam::WordButton::State::kNone: {
            setTextColor(normalTextColor);
            currentBackgroundColor = VSTGUI::kTransparentCColor;
            break;
        }

        case mam::WordButton::State::kSearched: {
            setTextColor(searchedTextColor);
            currentBackgroundColor = searchedBgrColor;
            break;
        }

        case mam::WordButton::State::kFocused: {
            setTextColor(focusedTextColor);
            currentBackgroundColor = focusedBgrColor;
            break;
        }

        default: {
            setTextColor(normalTextColor);
            currentBackgroundColor = VSTGUI::kTransparentCColor;
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
