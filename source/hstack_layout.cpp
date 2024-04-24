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
static auto
layout_row_stack(const CPoint parent,
                 CRects& rects,
                 const HStackLayout::Spacing& spacing,
                 const HStackLayout::Padding& padding) -> const CCoord
{
    if (rects.empty())
        return {};

    CPoint offset(padding.left, padding.top);
    const CPoint origin(padding.left, padding.top);

    const auto default_height = rects.begin()->getHeight();

    for (auto& rect : rects)
    {
        rect.setHeight(default_height);
        rect.moveTo(offset);

        // The first rect never gets a word wrap, even is parent rect is very
        // narrow
        const bool is_first_rect = offset == origin;
        const bool is_line_end =
            (offset.x + rect.getWidth() + padding.right) > parent.x;
        const bool needs_word_wrap = is_line_end && !is_first_rect;
        if (needs_word_wrap)
        {
            offset.x = padding.left;                    // Carriage Return
            offset.y += default_height + spacing.verti; // Line Feed
            rect.moveTo(offset);
        }

        offset.x += rect.getWidth() + spacing.horiz; // Next word horiz pos
    }

    return offset.y + default_height + padding.bottom;
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

        const auto rect     = child->getViewSize();
        const auto new_rect = rects[count++];
        if (rect != new_rect)
            child->setViewSize(new_rect);
    });
}

//------------------------------------------------------------------------
static auto do_layout(HStackLayout::ViewContainer* container,
                      const HStackLayout::Spacing& spacing,
                      const HStackLayout::Padding& padding) -> void
{
    // Layout children
    auto parent_size = container->getViewSize();
    CRects sizes     = collect_view_size(container);
    const auto new_height =
        layout_row_stack({parent_size.getWidth(), parent_size.getHeight()},
                         sizes, spacing, padding);

    // Adjust parent container height
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
void HStackLayout::viewWillDelete(View* view)
{
    if (view == container)
    {
        container->unregisterViewContainerListener(this);
        container->unregisterViewListener(this);
        container = nullptr;
    }
}

//------------------------------------------------------------------------
void HStackLayout::viewContainerViewAdded(ViewContainer* container, View* view)
{
    do_layout(container, spacing, padding);
}

//------------------------------------------------------------------------
void HStackLayout::viewContainerViewRemoved(ViewContainer* container,
                                            View* view)
{
    do_layout(container, spacing, padding);
}

//------------------------------------------------------------------------
void HStackLayout::viewSizeChanged(View* view, const Rect& oldSize)
{
    do_layout(container, spacing, padding);
}

//------------------------------------------------------------------------
void HStackLayout::setup(const Spacing& spacing, const Padding& padding)
{
    this->spacing = spacing;
    this->padding = padding;
}

//------------------------------------------------------------------------

} // namespace mam