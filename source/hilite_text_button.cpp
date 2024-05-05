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
        CRect rect     = getViewSize();
        CPoint topLeft = rect.getTopLeft();
        topLeft.x      = topLeft.x + 4;
        topLeft.y      = topLeft.y + 4;
        rect.setTopLeft(topLeft);

        CPoint bottomRight = rect.getBottomRight();
        bottomRight.y      = bottomRight.y - 2;
        rect.setBottomRight(bottomRight);

        context->setFillColor(CColor(255, 255, 0, 125));
        context->drawGraphicsPath(
            context->createRoundRectGraphicsPath(rect, 0));
    }
    CTextButton::draw(context);
}

//------------------------------------------------------------------------
} // namespace mam
