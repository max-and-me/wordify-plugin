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
                             double vspacing,
                             double padding) -> const CCoord
{
    if (rects.empty())
        return {};

    CPoint offset(padding, padding);
    const CPoint origin(padding, padding);

    // The first rect needs to be treated in a special way. No matter if it fits
    // into the first line or not, it needs to stay at origin! Even if the
    // parent container is very narrow.
    auto& first_rect          = rects[0];
    const auto default_height = first_rect.getHeight();
    first_rect.moveTo(offset);
    offset.x += first_rect.getWidth() + hspacing;

    // Start from the 2nd element (if there is one of course)
    std::for_each(std::next(rects.begin(), 1), rects.end(), [&](auto& rect) {
        rect.setHeight(default_height);
        if (!((offset.x + rect.getWidth() + padding) < parent.x))
        {
            offset.x = padding;
            offset.y += default_height + vspacing;
        }

        rect.moveTo(offset);
        offset.x += rect.getWidth() + hspacing;
    });

    return offset.y + default_height + padding;
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
        rect      = rects[count++];
        child->setViewSize(rect);
    });
}

//------------------------------------------------------------------------
static auto do_layout(HStackLayout::ViewContainer* container,
                      HStackLayout::Coord hspacing,
                      HStackLayout::Coord vspacing,
                      HStackLayout::Coord padding) -> void
{
    auto parent_size = container->getViewSize();
    CRects sizes     = collect_view_size(container);
    auto new_height =
        layout_row_stack({parent_size.getWidth(), parent_size.getHeight()},
                         sizes, hspacing, vspacing, padding);

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
        container->unregisterViewListener(this);
    }
}

//------------------------------------------------------------------------
void HStackLayout::viewContainerViewAdded(ViewContainer* container, View* view)
{
    do_layout(container, hspacing, vspacing, padding);
}

//------------------------------------------------------------------------
void HStackLayout::viewContainerViewRemoved(ViewContainer* container,
                                            View* view)
{
    do_layout(container, hspacing, vspacing, padding);
}

//------------------------------------------------------------------------
void HStackLayout::viewSizeChanged(View* view, const Rect& oldSize)
{
    do_layout(container, hspacing, vspacing, padding);
}

//------------------------------------------------------------------------
void HStackLayout::setup(Coord hspacing, Coord vspacing, Coord padding)
{
    this->hspacing = hspacing;
    this->vspacing = vspacing;
    this->padding  = padding;
}

//------------------------------------------------------------------------

} // namespace mam