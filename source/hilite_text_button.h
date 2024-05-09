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

    enum class HiliteState
    {
        kNone = 0,
        kSearchHilite,
        kSearchSelectHilite
    };
    HiliteTextButton(const VSTGUI::CRect& size,
                     VSTGUI::IControlListener* listener = nullptr,
                     int32_t tag                        = -1,
                     VSTGUI::UTF8StringPtr title        = nullptr,
                     VSTGUI::CTextButton::Style         = kKickStyle);

    void draw(VSTGUI::CDrawContext* context) override;
    void setHilite(HiliteState state) { hilite = state; }
    void setTextColor(const VSTGUI::CColor& color) override;
    void setSchemeHiliteColors(const VSTGUI::CColor& shbc,
                               const VSTGUI::CColor& shtc,
                               const VSTGUI::CColor& ssbc,
                               const VSTGUI::CColor& sstc);

    //--------------------------------------------------------------------
private:
    VSTGUI::CColor searchHiliteBgrColor        = VSTGUI::kGreyCColor;
    VSTGUI::CColor searchHiliteTextColor       = VSTGUI::kWhiteCColor;
    VSTGUI::CColor searchSelectHiliteBgrColor  = VSTGUI::kYellowCColor;
    VSTGUI::CColor searchSelectHiliteTextColor = VSTGUI::kBlackCColor;
    VSTGUI::CColor normalTextColor             = VSTGUI::kBlackCColor;
    HiliteState hilite                         = HiliteState::kNone;
};

//------------------------------------------------------------------------
} // namespace mam
