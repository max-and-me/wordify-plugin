//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------
#include "vstgui/lib/controls/cbuttons.h"

#pragma once

namespace mam {
class HiliteTextButton : public VSTGUI::CTextButton
{

public:
    HiliteTextButton (const VSTGUI::CRect& size);
    
    void draw (VSTGUI::CDrawContext* context) override;
    void setHilite (bool state){hilite = state;}
    
private:
    bool hilite = false;
};

}// namespace mam 
