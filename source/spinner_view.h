//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "vstgui/lib/cview.h"

namespace mam {

//------------------------------------------------------------------------
// SpinnerView
//------------------------------------------------------------------------
class SpinnerView : public VSTGUI::CView
{
public:
    SpinnerView(const VSTGUI::CRect& size);

    void onIdle() override;

    void draw(VSTGUI::CDrawContext* context) override;

private:
    float rotationAngle = 0.0f;
};
} // namespace mam
