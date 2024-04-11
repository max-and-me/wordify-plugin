//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "meta_words_clip_controller.h"
#include "fmt/format.h"
#include "list_entry_controller.h"
#include "meta_words_data.h"
#include "vstgui/lib/ccolor.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cfont.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/controls/cstringlist.h"
#include "vstgui/lib/controls/ctextlabel.h"
#include "vstgui/lib/cpoint.h"
#include "vstgui/lib/crect.h"
#include "vstgui/lib/crowcolumnview.h"
#include "vstgui/lib/cview.h"
#include "vstgui/lib/platform/platformfactory.h"
#include "vstgui/uidescription/detail/uiviewcreatorattributes.h"
#include "vstgui/uidescription/uiattributes.h"
#include <chrono>

namespace mam {
using namespace ::VSTGUI;

#ifndef kPI
#define kPI 3.14159265358979323846
#endif

//------------------------------------------------------------------------
auto to_time_display_string(MetaWordsData::Seconds seconds)
    -> MetaWordsData::String
{
    namespace chrono = std::chrono;

    chrono::seconds total(static_cast<int>(seconds));
    const auto h = chrono::duration_cast<chrono::hours>(total);
    const auto m = chrono::duration_cast<chrono::minutes>(total - h);
    const auto s = chrono::duration_cast<chrono::seconds>(total - h - m);

    auto output =
        fmt::format("{:02}:{:02}:{:02}", h.count(), m.count(), s.count());

    return output;
}

//------------------------------------------------------------------------
class SpinningLoadingView : public CView
{
public:
    SpinningLoadingView(const CRect& size)
    : CView(size)
    {
        setWantsIdle(true);
    }

    void onIdle() override
    {
        rotationAngle += 5.0f;
        if (rotationAngle >= 360.0f)
            rotationAngle -= 360.0f;
        invalid();
    }

    void draw(CDrawContext* context) override
    {
        CView::draw(context);

        CRect bounds        = getViewSize();
        const CPoint center = bounds.getCenter();

        context->setDrawMode(kAntiAliasing);
        context->setLineWidth(2.0);

        constexpr int numLines        = 12;
        constexpr float lineLength    = 10.0;
        constexpr float lineThickness = 2.0;
        constexpr CColor lineColor(255, 255, 255);

        context->setFrameColor(lineColor);
        context->setFillColor(lineColor);

        for (int i = 0; i < numLines; ++i)
        {
            const float angle = (i * 30.0f + rotationAngle) * (kPI / 180.0f);
            const CPoint start(
                center.x + cos(angle) * (bounds.getWidth() * 0.3f),
                center.y + sin(angle) * (bounds.getHeight() * 0.3f));
            const CPoint end(start.x + cos(angle) * lineLength,
                             start.y + sin(angle) * lineLength);
            context->drawLine(start, end);
        }
    }

private:
    float rotationAngle = 0.0f;
};

//------------------------------------------------------------------------
static auto update_label_control(CTextLabel& listTitle,
                                 const MetaWordsData& data) -> void
{
    auto [r, g, b] = data.color;
    const VSTGUI::CColor color(r, g, b);
    listTitle.setFontColor(color);
    listTitle.setText(VSTGUI::UTF8String(data.name));
    listTitle.sizeToFit();
}

//------------------------------------------------------------------------
static auto update_time_display_control(CTextLabel& timeDisplay,
                                        const MetaWordsData& data) -> void
{
    const auto str = to_time_display_string(data.project_time_start);
    timeDisplay.setText(VSTGUI::UTF8String(str));
}

//------------------------------------------------------------------------
static auto
update_list_control_content(CListControl* listControl,
                            const meta_words::MetaWords& words) -> void
{
    listControl->setMax(words.size() - 1);
    listControl->recalculateLayout();

    if (auto stringListDrawer =
            dynamic_cast<StringListControlDrawer*>(listControl->getDrawer()))
    {
        stringListDrawer->setStringProvider([words](int32_t row) {
            const meta_words::MetaWord word = words.at(row);
            const std::string name          = word.word;

            const UTF8String string(name.data());
            return getPlatformFactory().createString(string);
        });
    }
}

//------------------------------------------------------------------------
// Helper class to call sizeToFit to all parents up the hierarchy
// AFTER a view has been attached or resized itself.
//------------------------------------------------------------------------
class FitContent : public VSTGUI::ViewListenerAdapter
{
public:
    //--------------------------------------------------------------------
    void viewAttached(CView* view) override
    {
        fit_content(view->getParentView());
    }

    void viewSizeChanged(CView* view, const CRect& oldSize) override
    {
        fit_content(view->getParentView());
    }

    //--------------------------------------------------------------------
private:
    static auto fit_content(CView* view) -> void
    {
        // Only call sizeToFit, when view is a CRowColumnView
        if (auto* rc_view = dynamic_cast<CRowColumnView*>(view))
        {
            // Return immediately, if there is nothing more to resize
            if (!rc_view->sizeToFit())
                return;

            fit_content(rc_view->getParentView());
        }
    }
};

//------------------------------------------------------------------------
// VstGPTWaveClipListController
//------------------------------------------------------------------------
MetaWordsClipController::MetaWordsClipController()
{
    view_listener = std::make_unique<FitContent>();
}

//------------------------------------------------------------------------
MetaWordsClipController::~MetaWordsClipController()
{
    if (listControl)
        listControl->unregisterViewListener(view_listener.get());

    if (subject)
        subject->remove_listener(observer_id);
}

//------------------------------------------------------------------------
bool MetaWordsClipController::initialize(
    Subject* subject, FuncMetaWordsData&& meta_words_data_func)
{
    if (!subject)
        return false;

    if (this->subject)
    {
        this->subject->remove_listener(observer_id);
    }

    this->subject              = subject;
    this->meta_words_data_func = std::move(meta_words_data_func);

    observer_id = this->subject->add_listener(
        [this](const auto&) { this->on_meta_words_data_changed(); });

    on_meta_words_data_changed();

    return true;
}

//------------------------------------------------------------------------
void MetaWordsClipController::on_meta_words_data_changed()
{
    const auto& data = meta_words_data_func();
    if (listControl)
    {
        update_list_control_content(listControl, data.words);
        listControl->invalid();
    }

    if (listTitle)
    {
        update_label_control(*listTitle, data);
        listTitle->setDirty();
    }

    if (timeDisplay)
    {
        update_time_display_control(*timeDisplay, data);
        timeDisplay->invalid();
    }
}

//------------------------------------------------------------------------
VSTGUI::CView*
MetaWordsClipController::verifyView(VSTGUI::CView* view,
                                    const VSTGUI::UIAttributes& attributes,
                                    const VSTGUI::IUIDescription* description)
{
    if (auto container = view->asViewContainer())
    {
        if (!spinner)
        {
            const auto view_size = CPoint({65., 65.});
            spinner =
                new SpinningLoadingView(CRect{0, 0, view_size.x, view_size.y});
            // container->addView(spinner);
        }
    }

    if (!listControl)
    {
        if (listControl = dynamic_cast<CListControl*>(view))
        {
            listControl->registerViewListener(view_listener.get());
            listControl->registerControlListener(this);
            update_list_control_content(listControl,
                                        meta_words_data_func().words);
        }
    }
    if (!listTitle)
    {
        if (auto viewLabel =
                attributes.getAttributeValue(UIViewCreator::kAttrUIDescLabel))
        {
            if (*viewLabel == "ListTitle")
            {
                if (listTitle = dynamic_cast<CTextLabel*>(view))
                    update_label_control(*listTitle, meta_words_data_func());
            }
        }
    }
    if (!timeDisplay)
    {
        if (auto viewLabel =
                attributes.getAttributeValue(UIViewCreator::kAttrUIDescLabel))
        {
            if (*viewLabel == "TimeDisplay")
            {
                if (timeDisplay = dynamic_cast<CTextLabel*>(view))
                    update_time_display_control(*timeDisplay,
                                                meta_words_data_func());
            }
        }
    }

    if (!root_view)
    {
        if (auto viewLabel =
                attributes.getAttributeValue(UIViewCreator::kAttrUIDescLabel))
        {
            if (*viewLabel == "MetaWordsClipTemplate")
            {
                root_view = dynamic_cast<CViewContainer*>(view);
            }
        }
    }

    return view;
};

//------------------------------------------------------------------------
void MetaWordsClipController::valueChanged(VSTGUI::CControl* pControl)
{
    if (pControl && pControl == listControl)
    {
        if (list_value_changed_func)
            list_value_changed_func(listControl->getValue());
    }
}

//------------------------------------------------------------------------
} // namespace mam
