// Copyright(c) 2024 Max And Me.

#include "hilite_text_button.h"
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
HiliteTextButton::HiliteTextButton(const CRect& size,
                                   IControlListener* listener,
                                   int32_t tag,
                                   UTF8StringPtr title,
                                   Style style)
: CTextButton(size, listener, tag, title, style)
{
}

//------------------------------------------------------------------------
void HiliteTextButton::draw(CDrawContext* context)
{
    if (hilite != HiliteState::kNone)
        drawHiliteBackground(context, getViewSize(), getRoundRadius(),
                             currentBackgroundColor);

    CTextButton::draw(context);
}

//------------------------------------------------------------------------
bool HiliteTextButton::setHilite(HiliteState state)
{
    const bool changed = hilite != state;
    hilite             = state;
    if (!changed)
        return false;

    switch (hilite)
    {
        case mam::HiliteTextButton::HiliteState::kNone: {
            setTextColor(normalTextColor);
            currentBackgroundColor = VSTGUI::kTransparentCColor;
            break;
        }

        case mam::HiliteTextButton::HiliteState::kSearchHilite: {
            setTextColor(searchHiliteTextColor);
            currentBackgroundColor = searchHiliteBgrColor;
            break;
        }

        case mam::HiliteTextButton::HiliteState::kSearchSelectHilite: {
            setTextColor(searchSelectHiliteTextColor);
            currentBackgroundColor = searchSelectHiliteBgrColor;
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
void HiliteTextButton::verifyTextButtonView(
    const VSTGUI::IUIDescription* description)
{
    description->getColor("search_hilite_bgr_color", searchHiliteBgrColor);

    description->getColor("search_hilite_text_color", searchHiliteTextColor);

    description->getColor("search_select_hilite_bgr_color",
                          searchSelectHiliteBgrColor);

    description->getColor("search_select_hilite_text_color",
                          searchSelectHiliteTextColor);

    description->getColor("transcript_text_color", normalTextColor);
}

//------------------------------------------------------------------------
} // namespace mam
