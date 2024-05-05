//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------
#include "vstgui/lib/controls/cbuttons.h"

#pragma once

namespace VSTGUI {
} // namespace VSTGUI

namespace mam {

//------------------------------------------------------------------------
class HiliteTextButton : public VSTGUI::CTextButton
{
public:
    //--------------------------------------------------------------------

    HiliteTextButton(const VSTGUI::CRect& size,
                     VSTGUI::IControlListener* listener = nullptr,
                     int32_t tag                        = -1,
                     VSTGUI::UTF8StringPtr title        = nullptr,
                     VSTGUI::CTextButton::Style         = kKickStyle);

    void draw(VSTGUI::CDrawContext* context) override;
    void setHilite(bool state) { hilite = state; }

    //--------------------------------------------------------------------
private:
    bool hilite = false;
};

//------------------------------------------------------------------------
} // namespace mam
