// Copyright(c) 2023 Max And Me.

#include "vstgui/lib/cviewcontainer.h"
#include "vstgui/lib/iviewlistener.h"

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

    HStackLayout(ViewContainer* container);
    ~HStackLayout() override;

    void viewContainerViewAdded(ViewContainer* container, View* view) override;
    void viewContainerViewRemoved(ViewContainer* container,
                                  View* view) override;
    void viewSizeChanged(View* view, const Rect& oldSize) override;

    //--------------------------------------------------------------------
private:
    ViewContainer* container = nullptr;
};
//------------------------------------------------------------------------

} // namespace mam