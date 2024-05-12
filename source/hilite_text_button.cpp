//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------
#include "hilite_text_button.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/uidescription/uidescription.h"

using namespace VSTGUI;

namespace mam {
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
    if (hilite == HiliteState::kNone)
    {
        CTextButton::draw(context);
        return;
    }

    const CRect& rect   = getViewSize();
    const CCoord radius = getRoundRadius();
    context->setFillColor(currentBackgroundColor);
    if (auto path = owned(context->createRoundRectGraphicsPath(rect, radius)))
        context->drawGraphicsPath(path);

    CTextButton::draw(context);
}

//------------------------------------------------------------------------
bool HiliteTextButton::setHilite(HiliteState state)
{
    const bool changed = hilite != state;
    hilite             = state;

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
