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
    if (hilite)
    {
        if (getTextColor() != hiliteColor)
            setTextColor(hiliteColor);

        CRect rect     = getViewSize();
        CPoint topLeft = rect.getTopLeft();
        rect.setTopLeft(topLeft);

        CPoint bottomRight = rect.getBottomRight();
        bottomRight.y      = bottomRight.y;
        rect.setBottomRight(bottomRight);

        context->setFillColor(CColor(200, 200, 200, 125));
        context->drawGraphicsPath(
            context->createRoundRectGraphicsPath(rect, 4));
    }
    else
    {
        if (getTextColor() != normalColor)
            setTextColor(normalColor);
    }
    CTextButton::draw(context);
}

//------------------------------------------------------------------------
void HiliteTextButton::setTextColor(const CColor& color)
{
    CTextButton::setTextColor(color);
    if (!hilite)
        normalColor = getTextColor();
}

//------------------------------------------------------------------------
} // namespace mam
