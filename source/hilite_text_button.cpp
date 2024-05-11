//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------
#include "hilite_text_button.h"
#include "vstgui/lib/cdrawcontext.h"

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
        if (getTextColor() != normalTextColor)
            setTextColor(normalTextColor);
        CTextButton::draw(context);
        return;
    }

    if (hilite == HiliteState::kSearchHilite)
    {
        if (getTextColor() != searchHiliteTextColor)
            setTextColor(searchHiliteTextColor);
        context->setFillColor(searchHiliteBgrColor);
    }
    else
    {
        if (getTextColor() != searchSelectHiliteTextColor)
            setTextColor(searchSelectHiliteTextColor);
        context->setFillColor(searchSelectHiliteBgrColor);
    }

    CRect rect     = getViewSize();
    CPoint topLeft = rect.getTopLeft();
    rect.setTopLeft(topLeft);

    CPoint bottomRight = rect.getBottomRight();
    bottomRight.y      = bottomRight.y;
    rect.setBottomRight(bottomRight);

    constexpr CCoord RADIUS = 4.;
    if (auto path = owned(context->createRoundRectGraphicsPath(rect, RADIUS)))
        context->drawGraphicsPath(path);

    CTextButton::draw(context);
}

//------------------------------------------------------------------------
void HiliteTextButton::setTextColor(const CColor& color)
{
    CTextButton::setTextColor(color);
    if (hilite == HiliteState::kNone)
        normalTextColor = getTextColor();
}

//------------------------------------------------------------------------
void HiliteTextButton::setSchemeHiliteColors(const VSTGUI::CColor& shbc,
                                             const VSTGUI::CColor& shtc,
                                             const VSTGUI::CColor& ssbc,
                                             const VSTGUI::CColor& sstc)
{
    searchHiliteBgrColor        = shbc;
    searchHiliteTextColor       = shtc;
    searchSelectHiliteBgrColor  = ssbc;
    searchSelectHiliteTextColor = sstc;
}

} // namespace mam
