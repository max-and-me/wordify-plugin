//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "meta_words_clip_controller.h"
#include "hstack_layout.h"
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

//------------------------------------------------------------------------
static auto update_label_control(CTextLabel& listTitle,
                                 const MetaWordsData& meta_words_data) -> void
{
    auto [r, g, b]             = meta_words_data.color;
    const VSTGUI::CColor color = make_color<float>(r, g, b, std::nullopt);
    listTitle.setFontColor(color);
    listTitle.setText(VSTGUI::UTF8String(meta_words_data.name));
    listTitle.sizeToFit();
}

//------------------------------------------------------------------------
static auto
update_time_display_control(CTextLabel& timeDisplay,
                            const MetaWordsData& meta_words_data) -> void
{
    const auto str = to_time_display_string(meta_words_data.project_time_start);
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
            const auto word_data   = word_dataset[row];
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
using CRects          = std::vector<CRect>;
using StringType      = std::string;
using FuncStringWidth = std::function<size_t(const StringType&)>;
auto collect_string_size_rects(const MetaWordsData& meta_words_data,
                               FuncStringWidth&& string_width_func) -> CRects
{
    CRects rects;
    for (const auto& meta_word_data : meta_words_data.words)
    {
        constexpr CCoord height = 24.;
        if (!meta_word_data.is_audible)
        {
            rects.push_back({0, 0, 0, height});
            continue;
        }

        const CCoord width = string_width_func(meta_word_data.word.word);

        rects.push_back({0, 0, width, height});
    }

    return rects;
}

//------------------------------------------------------------------------
auto find_view_after(CViewContainer* container, size_t tag) -> CView*
{
    for (auto i = 0; i < container->getNbViews(); i++)
    {
        auto* view = container->getView(i);
        if (auto control = dynamic_cast<CControl*>(view))
        {
            if (tag < control->getTag())
                return control;
        }
    }

    return nullptr;
}

//------------------------------------------------------------------------
auto find_view_with_tag(CViewContainer* container, size_t tag) -> CView*
{
    for (auto i = 0; i < container->getNbViews(); i++)
    {
        auto* view = container->getView(i);
        if (auto control = dynamic_cast<CControl*>(view))
        {
            if (control->getTag() == tag)
                return control;
        }
    }

    return nullptr;
}

//------------------------------------------------------------------------
static auto update_text_document(const VSTGUI::IUIDescription* description,
                                 const VSTGUI::UIAttributes& attributes,
                                 IControlListener* listener,
                                 CViewContainer* text_document,
                                 const MetaWordsData& meta_words_data) -> void
{
    if (!text_document)
        return;

    const auto font_desc    = description->getFont("ListEntryFont");
    const auto font_painter = font_desc->getPlatformFont()->getPainter();
    auto string_width_func  = [&](const StringType& text) {
        const auto t = UTF8String(text);
        return font_painter->getStringWidth(nullptr, t.getPlatformString(),
                                             true);
    };
    const auto rects =
        collect_string_size_rects(meta_words_data, string_width_func);

    // Remove views
    std::vector<CControl*> views_to_remove;
    text_document->forEachChild([&](CView* child) {
        if (auto* control = dynamic_cast<CControl*>(child))
        {
            if (!meta_words_data.words[control->getTag()].is_audible)
                views_to_remove.push_back(control);
        }
    });
    for (auto* child : views_to_remove)
        text_document->removeView(child);

    // Add new views
    for (auto i = 0; i < meta_words_data.words.size(); ++i)
    {
        const auto word_data = meta_words_data.words[i];
        if (!word_data.is_audible)
            continue;

        if (find_view_with_tag(text_document, i))
            continue;

        auto* view_after = find_view_after(text_document, i);

        auto view =
            description->getViewFactory()->createView(attributes, description);
        auto but = dynamic_cast<CTextButton*>(view);
        but->setViewSize(rects[i]);
        but->setTitle(UTF8String(meta_words_data.words[i].word.word));
        but->setTag(i);
        but->setListener(listener);

        // Set gradients to nullptr. This drastically speeds up the
        // performance!!!
        but->setGradient(nullptr);
        but->setGradientHighlighted(nullptr);

        text_document->addView(but, view_after);
    }

    fit_content(text_document->getParentView());
}

//------------------------------------------------------------------------
// Helper class to call sizeToFit to all parents up the hierarchy
// AFTER a view has been attached or resized itself.
//------------------------------------------------------------------------
class FitContent : public VSTGUI::ViewListenerAdapter
{
public:
    //--------------------------------------------------------------------
    FitContent(CViewContainer* container)
    : container(container)
    {
        if (container)
            container->registerViewListener(this);
    }

    ~FitContent() override
    {
        if (container)
            container->unregisterViewListener(this);
    }

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
    CViewContainer* container = nullptr;
};

//------------------------------------------------------------------------
// VstGPTWaveClipListController
//------------------------------------------------------------------------
MetaWordsClipController::MetaWordsClipController(
    const VSTGUI::IUIDescription* description)
: description(description)
{
}

//------------------------------------------------------------------------
MetaWordsClipController::~MetaWordsClipController()
{
    if (listControl)
        listControl->unregisterViewListener(view_listener.get());

    if (text_document)
        text_document->unregisterViewListener(view_listener.get());

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
        text_document->invalid();
    }
}

//------------------------------------------------------------------------
VSTGUI::CView*
MetaWordsClipController::verifyView(VSTGUI::CView* view,
                                    const VSTGUI::UIAttributes& attributes,
                                    const VSTGUI::IUIDescription* description)
{

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
                stack_layout  = std::make_unique<HStackLayout>(text_document);
                stack_layout->setup(4., 0., 0.);
                update_text_document(description, meta_word_button_attributes,
                                     this, text_document,
                                     meta_words_data_func());

                view_listener = std::make_unique<FitContent>(text_document);
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
