// Copyright(c) 2023 Max And Me.

#include "hstack_layout.h"
#include "vstgui/lib/controls/ccontrol.h"
#include <vector>

namespace mam {

//------------------------------------------------------------------------
using CCoord   = VSTGUI::CCoord;
using CRect    = VSTGUI::CRect;
using CPoint   = VSTGUI::CPoint;
using CControl = VSTGUI::CControl;
using CRects   = std::vector<CRect>;

//------------------------------------------------------------------------
static auto layout_row_stack(const CPoint parent,
                             CRects& rects,
                             double hspacing,
                             double vspacing) -> const CCoord
{
    if (rects.empty())
        return {};

    CPoint offset(0, 0);
    const CPoint origin(0, 0);
    const auto default_height = rects.at(0).getHeight();
    for (auto& rect : rects)
    {
        rect.setHeight(default_height);
        rect.moveTo(origin);
        if (!(offset.x + rect.getWidth() < parent.x))
        {
            offset.x = 0.;
            offset.y += default_height + vspacing;
        }

        rect.moveTo(offset);
        offset.x += rect.getWidth() + hspacing;
    }

    return offset.y + default_height;
}

//------------------------------------------------------------------------
static auto collect_view_size(HStackLayout::ViewContainer* container) -> CRects
{
    CRects rects;
    container->forEachChild([&](HStackLayout::View* child) {
        if (child->isSubview())
            rects.push_back(child->getViewSize());
    });

    return rects;
}

//------------------------------------------------------------------------
static auto apply_view_size(HStackLayout::ViewContainer* container,
                            const CRects& rects) -> void
{
    size_t count = 0;
    container->forEachChild([&](HStackLayout::View* child) {
        if (!child->isSubview())
            return;

        auto rect = child->getViewSize();
        rect      = rects.at(count++);
        child->setViewSize(rect);
    });
}

//------------------------------------------------------------------------
static auto do_layout(HStackLayout::ViewContainer* container) -> void
{
    auto parent_size = container->getViewSize();
    CRects sizes     = collect_view_size(container);
    auto new_height  = layout_row_stack(
        {parent_size.getWidth(), parent_size.getHeight()}, sizes, 0, 0);

    parent_size.setHeight(new_height);
    container->setViewSize(parent_size);
    apply_view_size(container, sizes);
}

//------------------------------------------------------------------------
// HStackLayout
//------------------------------------------------------------------------
HStackLayout::HStackLayout(ViewContainer* container)
: container(container)
{
    if (container)
    {
        container->registerViewListener(this);
        container->registerViewContainerListener(this);
    }
}

//------------------------------------------------------------------------
HStackLayout::~HStackLayout()
{
    if (container)
    {
        container->unregisterViewContainerListener(this);
        container->registerViewListener(this);
    }
}

//------------------------------------------------------------------------
void HStackLayout::viewContainerViewAdded(ViewContainer* container, View* view)
{
    do_layout(container);
}

//------------------------------------------------------------------------
void HStackLayout::viewContainerViewRemoved(ViewContainer* container,
                                            View* view)
{
    do_layout(container);
}

//------------------------------------------------------------------------
void HStackLayout::viewSizeChanged(View* view, const Rect& oldSize)
{
    do_layout(container);
}

//------------------------------------------------------------------------

} // namespace mam