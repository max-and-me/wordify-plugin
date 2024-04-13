//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "meta_words_clip_controller.h"
#include "fmt/format.h"
#include "list_entry_controller.h"
#include "little_helpers.h"
#include "meta_words_data.h"
#include "vstgui/lib/ccolor.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cfont.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/controls/cbuttons.h"
#include "vstgui/lib/controls/cstringlist.h"
#include "vstgui/lib/controls/ctextlabel.h"
#include "vstgui/lib/cpoint.h"
#include "vstgui/lib/crect.h"
#include "vstgui/lib/crowcolumnview.h"
#include "vstgui/lib/cview.h"
#include "vstgui/lib/platform/iplatformfont.h"
#include "vstgui/lib/platform/platformfactory.h"
#include "vstgui/uidescription/detail/uiviewcreatorattributes.h"
#include "vstgui/uidescription/iviewfactory.h"
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
    auto [r, g, b]             = data.color;
    const VSTGUI::CColor color = make_color<float>(r, g, b, std::nullopt);
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
                            const MetaWordDataset& word_dataset) -> void
{
    listControl->setMax(word_dataset.size() - 1);
    listControl->recalculateLayout();

    if (auto stringListDrawer =
            dynamic_cast<StringListControlDrawer*>(listControl->getDrawer()))
    {
        stringListDrawer->setStringProvider([word_dataset](int32_t row) {
            const auto word_data   = word_dataset.at(row);
            const std::string name = word_data.word.word;

            const UTF8String string(name.data());
            return getPlatformFactory().createString(string);
        });
    }
}

//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
using CRects = std::vector<CRect>;
static auto layout_row_stack(const CPoint parent,
                             CRects& rects,
                             double hspacing,
                             double vspacing) -> const CPoint
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

    return {parent.x, offset.y + default_height};
}

//------------------------------------------------------------------------
static auto update_text_document(const VSTGUI::IUIDescription* description,
                                 const VSTGUI::UIAttributes& attributes,
                                 IControlListener* listener,
                                 CViewContainer* text_document,
                                 const MetaWordsData& data) -> void
{
    if (!text_document)
        return;

    text_document->removeAll();

    CRects rects;
    auto font_desc    = description->getFont("ListEntryFont");
    auto font_painter = font_desc->getPlatformFont()->getPainter();

    for (const auto& word : data.words)
    {
        constexpr CCoord height = 24;
        if (!word.is_audible)
        {
            rects.push_back({0, 0, 0, height});
            continue;
        }

        auto text          = UTF8String(word.word.word);
        const CCoord width = font_painter->getStringWidth(
                                 nullptr, text.getPlatformString(), true) +
                             4.;

        rects.push_back({0, 0, width, height});
    }

    const auto parent_size =
        layout_row_stack(text_document->getViewSize().getSize(), rects, 0, 0);

    for (size_t i = 0; i < data.words.size(); ++i)
    {
        if (!data.words.at(i).is_audible)
            continue;

        auto new_view =
            description->getViewFactory()->createView(attributes, description);
        auto new_word = dynamic_cast<CTextButton*>(new_view);
        new_word->setViewSize(rects.at(i));

        // auto* new_word =
        //     new CTextButton(rects.at(i), getViewController(text_document));

        new_word->setTitle(UTF8String(data.words.at(i).word.word));
        // new_word->setTransparency(true);
        new_word->setListener(getViewController(text_document));
        // new_word->setFont(font_desc);
        new_word->setTag(i);
        new_word->setListener(listener);
        text_document->addView(new_word);
    }

    auto s = text_document->getViewSize();
    s.setHeight(parent_size.y);
    text_document->setViewSize(s);
    fit_content(text_document->getParentView());
    text_document->invalid();
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
};

//------------------------------------------------------------------------
// VstGPTWaveClipListController
//------------------------------------------------------------------------
MetaWordsClipController::MetaWordsClipController(
    const VSTGUI::IUIDescription* description)
: description(description)
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

    auto view = description->createView("TextWordTemplate", this);
    if (view)
        view->forget();

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

    if (text_document)
    {
        update_text_document(description, meta_word_button_attributes, this,
                             text_document, data);
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

    if (!text_document)
    {
        if (auto viewLabel =
                attributes.getAttributeValue(UIViewCreator::kAttrUIDescLabel))
        {
            if (*viewLabel == "TextDocument")
            {
                text_document = dynamic_cast<CViewContainer*>(view);
                update_text_document(description, meta_word_button_attributes,
                                     this, text_document,
                                     meta_words_data_func());
            }
        }
    }

    if (auto viewLabel =
            attributes.getAttributeValue(UIViewCreator::kAttrUIDescLabel))
    {
        if (*viewLabel == "MetaWordButton")
        {
            meta_word_button_attributes = attributes;
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
    else
    {
        if (pControl)
            list_value_changed_func(pControl->getTag());
    }
}

//------------------------------------------------------------------------
} // namespace mam
