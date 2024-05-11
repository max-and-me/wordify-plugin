//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "list_controller.h"
#include "hilite_text_button.h"
#include "meta_words_clip_controller.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/controls/cbuttons.h"
#include "vstgui/lib/crowcolumnview.h"
#include "vstgui/lib/cscrollview.h"
#include "vstgui/uidescription/iuidescription.h"
#include <string>

using namespace VSTGUI;

namespace mam {

//------------------------------------------------------------------------
constexpr size_t PLAYBACK_REGION_ID_ATTR = 123456789;
constexpr auto LIST_ENTRY_VIEW_TEMPLATE  = "MetaWordsClipTemplateNG";

//------------------------------------------------------------------------
static auto find_view_by_id(const CRowColumnView& rowColView,
                            ListController::PlaybackRegion::Id pbr_id) -> CView*
{
    CView* viewToFind = nullptr;
    rowColView.forEachChild([&, pbr_id](CView* view) {
        if (viewToFind)
            return;

        meta_words::PlaybackRegion::Id id = 0;
        if (view->getAttribute(PLAYBACK_REGION_ID_ATTR, id))
        {
            if (pbr_id == size_t(id))
                viewToFind = view;
        }
    });

    return viewToFind;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
static auto build_meta_words_data(const ARADocumentController* controller,
                                  const meta_words::PlaybackRegion::Id id)
    -> const MetaWordsData
{
    if (!controller)
        return {};

    auto opt_region = controller->find_playback_region(id);
    if (!opt_region)
        return {};

    return opt_region.value()->get_meta_words_data();
}

//------------------------------------------------------------------------
// ListController
//------------------------------------------------------------------------
ListController::ListController(ARADocumentController* controller,
                               const IUIDescription* uidesc)
: controller(controller)
, uidesc(uidesc)
{
    if (controller)
    {
        lifetime_observer = tiny_observer_pattern::make_observer(
            controller->get_playback_region_lifetimes_subject(),
            [&](const auto& data) {
                this->on_add_remove_playback_region(data);
            });

        order_observer = tiny_observer_pattern::make_observer(
            controller->get_playback_region_order_subject(),
            [&](const auto&) { this->on_playback_regions_reordered(); });

        word_selected_observer_id = controller->register_word_selected_observer(
            [this](const auto& data) { this->checkSelectWord(data); });
    }
}

//------------------------------------------------------------------------
ListController::~ListController()
{
    if (rowColView)
    {
        rowColView->unregisterViewListener(this);
        rowColView = nullptr;
    }

    if (controller)
        controller->unregister_word_selected_observer(
            word_selected_observer_id);
}

//------------------------------------------------------------------------
CView* ListController::verifyView(CView* view,
                                  const UIAttributes& /*attributes*/,
                                  const IUIDescription* description)
{
    if (!rowColView)
    {
        if (rowColView = dynamic_cast<CRowColumnView*>(view))
        {
            rowColView->registerViewListener(this);
            if (controller)
            {
                controller->for_each_playback_region_id(
                    [&](const PlaybackRegion::Id id) {
                        auto* newView = create_list_item_view(id);
                        if (newView)
                        {
                            rowColView->addView(newView);
                            rowColView->sizeToFit();
                            newView->setAttribute('prid', id);
                        }
                    });
            }
        }
    }

    return view;
}

//------------------------------------------------------------------------
auto ListController::create_list_item_view(const PlaybackRegion::Id id)
    -> CView*
{
    if (!uidesc)
        return nullptr;

    playback_region_id = id;

    auto* newView = uidesc->createView(LIST_ENTRY_VIEW_TEMPLATE, this);
    newView->setAttribute(PLAYBACK_REGION_ID_ATTR, id);

    playback_region_id.reset();

    return newView;
}

//------------------------------------------------------------------------
void ListController::on_playback_regions_reordered()
{
    if (!rowColView)
        return;

    if (!controller)
        return;

    auto func = [&](size_t index, meta_words::PlaybackRegion::Id id) {
        auto* viewToMove = find_view_by_id(*rowColView, id);
        rowColView->changeViewZOrder(viewToMove, static_cast<uint32_t>(index));
    };

    controller->for_each_playback_region_id_enumerated(func);

    rowColView->invalid();
}

//------------------------------------------------------------------------
void ListController::on_add_remove_playback_region(
    const PlaybackRegionLifetimeData& data)
{
    if (!uidesc)
        return;

    if (!rowColView)
        return;

    switch (data.event)
    {
        case PlaybackRegionLifetimeData::Event::HasBeenAdded: {
            auto* viewToAdd = create_list_item_view(data.id);
            if (viewToAdd)
            {
                rowColView->addView(viewToAdd);
                rowColView->sizeToFit();
                on_playback_regions_reordered();
            }

            break;
        }
        case PlaybackRegionLifetimeData::Event::WillBeRemoved: {
            auto* viewToRemove = find_view_by_id(*rowColView, data.id);
            if (viewToRemove)
            {
                rowColView->removeView(viewToRemove);
                rowColView->sizeToFit();
                rowColView->invalid();
            }
            break;
        }
    }
}

//------------------------------------------------------------------------
IController*
ListController::createSubController(UTF8StringPtr name,
                                    const IUIDescription* description)
{
    if (!playback_region_id.has_value())
        return nullptr;

    if (UTF8StringView(name) == "MetaWordsClipController")
    {
        if (!controller)
            return nullptr;

        auto pbr_id = playback_region_id.value();
        auto ctler  = this->controller;

        auto* subctrl = new MetaWordsClipController(description);
        if (!subctrl)
            return nullptr;

        auto& subject = controller->get_playback_region_changed_subject(pbr_id);

        subctrl->meta_words_data_func = [=]() {
            return build_meta_words_data(ctler, pbr_id);
        };

        subctrl->on_select_word_func = [=](int index) {
            controller->get_region_selection_model().select(
                {pbr_id, static_cast<size_t>(index)});
            controller->onRequestSelectWord(index, pbr_id);
        };

        return subctrl->initialize(&subject) ? subctrl : nullptr;
    }

    return nullptr;
}

//------------------------------------------------------------------------
void ListController::checkSelectWord(const WordSelectData& data)
{
    // word data selection
    if (data.indices.empty() == false && data.hiliteSelectIndex != -1)
    {
        controller->onRequestSelectWord(data.indices.at(data.hiliteSelectIndex),
                                        data.region_id);

        // waveform selection
        controller->get_region_selection_model().select(
            {data.region_id,
             static_cast<size_t>(data.indices.at(data.hiliteSelectIndex))});
    }

    // ui update
    if (!rowColView)
        return;

    CView* toFind = nullptr;
    rowColView->forEachChild([&](CView* child) {
        if (toFind)
            return;

        meta_words::PlaybackRegion::Id region_id = 0;

        if (child->getAttribute('prid', region_id))
        {
            if (region_id == data.region_id)
                toFind = child;
        }
    });

    if (toFind == nullptr)
        return;

    if (auto* container = toFind->asViewContainer())
    {
        std::vector<HiliteTextButton*> btns;
        container->getChildViewsOfType<HiliteTextButton>(btns, true);
        for (auto& btn : btns)
        {
            btn->setHilite(HiliteTextButton::HiliteState::kNone);

            auto it = std::find(data.indices.begin(), data.indices.end(),
                                btn->getTag());
            if (it != data.indices.end())
            {
                CPoint btnRectGlobal = btn->getViewSize().getTopLeft();
                btn->localToFrame(btnRectGlobal);

                CRect btnRect = btn->getViewSize();
                btnRect.offset(btnRectGlobal.x - btnRect.getWidth(),
                               btnRectGlobal.y);
                if (data.hiliteSelectIndex != -1 &&
                    btn->getTag() == data.indices.at(data.hiliteSelectIndex))
                {
                    btn->setHilite(
                        HiliteTextButton::HiliteState::kSearchSelectHilite);
                    auto rect =
                        btn->translateToGlobal(rowColView->getViewSize());
                    CScrollView* scroll = dynamic_cast<CScrollView*>(
                        rowColView->getParentView()->getParentView());
                    if (scroll)
                        scroll->makeRectVisible(rect);
                }
                else
                {
                    btn->setHilite(
                        HiliteTextButton::HiliteState::kSearchHilite);
                }
            }
            btn->setDirty();
        }
    }
}

//------------------------------------------------------------------------
void ListController::viewWillDelete(VSTGUI::CView* view)
{
    if (view == rowColView)
    {
        rowColView->unregisterViewListener(this);
        rowColView = nullptr;
    }
}

//------------------------------------------------------------------------
} // namespace mam
