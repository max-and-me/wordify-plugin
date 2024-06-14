// Copyright(c) 2024 Max And Me.

#pragma once

#include "supress_warnings.h"
BEGIN_SUPRESS_WARNINGS
#include "vstgui/lib/controls/cbuttons.h"
END_SUPRESS_WARNINGS

namespace VSTGUI {
class IUIDescription;
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
    bool setHilite(HiliteState state);
    void verifyTextButtonView(const VSTGUI::IUIDescription* description);

    //--------------------------------------------------------------------
private:
    VSTGUI::CColor searchHiliteBgrColor        = VSTGUI::kGreyCColor;
    VSTGUI::CColor searchHiliteTextColor       = VSTGUI::kWhiteCColor;
    VSTGUI::CColor searchSelectHiliteBgrColor  = VSTGUI::kYellowCColor;
    VSTGUI::CColor searchSelectHiliteTextColor = VSTGUI::kBlackCColor;
    VSTGUI::CColor normalTextColor             = VSTGUI::kBlackCColor;
    VSTGUI::CColor currentBackgroundColor      = VSTGUI::kTransparentCColor;
    HiliteState hilite                         = HiliteState::kNone;
};

//------------------------------------------------------------------------
} // namespace mam
