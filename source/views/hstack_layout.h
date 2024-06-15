// Copyright(c) 2024 Max And Me.

#pragma once

#include "supress_warnings.h"
BEGIN_SUPRESS_WARNINGS
#include "vstgui/lib/cviewcontainer.h"
#include "vstgui/lib/iviewlistener.h"
END_SUPRESS_WARNINGS

namespace VSTGUI {
class CViewContainer;
class CView;
struct CRect;
} // namespace VSTGUI

namespace mam {
//------------------------------------------------------------------------
// HStackLayout
//------------------------------------------------------------------------
class HStackLayout : public VSTGUI::ViewListenerAdapter,
                     public VSTGUI::ViewContainerListenerAdapter
{
public:
    //--------------------------------------------------------------------
    using ViewContainer = VSTGUI::CViewContainer;
    using View          = VSTGUI::CView;
    using Rect          = VSTGUI::CRect;
    using Coord         = VSTGUI::CCoord;

    struct Padding
    {
        Coord top    = 0.;
        Coord right  = 0.;
        Coord bottom = 0.;
        Coord left   = 0.;
    };

    struct Spacing
    {
        Coord horiz = 0.;
        Coord verti = 0.;
    };

    HStackLayout(ViewContainer* container);
    ~HStackLayout() override;

    void setup(const Spacing& spacing, const Padding& padding);

    void viewContainerViewAdded(ViewContainer* container, View* view) override;
    void viewContainerViewRemoved(ViewContainer* container,
                                  View* view) override;
    void viewSizeChanged(View* view, const Rect& oldSize) override;
    void viewWillDelete(View* view) override;

    //--------------------------------------------------------------------
private:
    ViewContainer* observed_container = nullptr;
    Spacing spacing{0., 0.};
    Padding padding{0., 0., 0., 0.};
};
//------------------------------------------------------------------------

} // namespace mam
