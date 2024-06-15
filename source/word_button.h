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
class WordButton : public VSTGUI::CTextButton
{
public:
    //--------------------------------------------------------------------

    enum class State
    {
        kNone = 0,
        kSearched,
        kFocused
    };
    WordButton(const VSTGUI::CRect& size,
               VSTGUI::IControlListener* listener = nullptr,
               int32_t tag                        = -1,
               VSTGUI::UTF8StringPtr title        = nullptr,
               VSTGUI::CTextButton::Style         = kKickStyle);

    void draw(VSTGUI::CDrawContext* context) override;
    bool setState(State state);
    void verifyTextButtonView(const VSTGUI::IUIDescription* description);

    //--------------------------------------------------------------------
private:
    VSTGUI::CColor searchedBgrColor       = VSTGUI::kGreyCColor;
    VSTGUI::CColor searchedTextColor      = VSTGUI::kWhiteCColor;
    VSTGUI::CColor focusedBgrColor        = VSTGUI::kYellowCColor;
    VSTGUI::CColor focusedTextColor       = VSTGUI::kBlackCColor;
    VSTGUI::CColor normalTextColor        = VSTGUI::kBlackCColor;
    VSTGUI::CColor currentBackgroundColor = VSTGUI::kTransparentCColor;
    State state                           = State::kNone;
};

//------------------------------------------------------------------------
} // namespace mam
