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

using namespace VSTGUI;

namespace mam {

//------------------------------------------------------------------------
static auto update_label_control(CTextLabel& listTitle,
                                 const MetaWordsData& meta_words_data) -> void
{
    auto [r, g, b]     = meta_words_data.color;
    const CColor color = make_color<float>(r, g, b, std::nullopt);
    listTitle.setFontColor(color);
    listTitle.setText(UTF8String(meta_words_data.name));
    listTitle.sizeToFit();
}

//------------------------------------------------------------------------
static auto
update_time_display_control(CTextLabel& timeDisplay,
                            const MetaWordsData& meta_words_data) -> void
{
    const auto str = to_time_display_string(meta_words_data.project_time_start);
    timeDisplay.setText(UTF8String(str));
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
using WordWidths = std::vector<CCoord>;
template <typename Func>
static auto compute_word_widths(const MetaWordsData& meta_words_data,
                                const Func& width_func)
{
    WordWidths widths;
    for (const auto& meta_word_data : meta_words_data.words)
    {
        if (!meta_word_data.is_clipped_by_region)
        {
            widths.push_back(0.);
            continue;
        }

        widths.push_back(width_func(meta_word_data.word.word));
    }

    return widths;
}

//------------------------------------------------------------------------
static auto find_view_after(CViewContainer* container, size_t tag) -> CView*
{
    for (uint32_t i = 0; i < container->getNbViews(); i++)
    {
        auto* view = container->getView(i);
        if (auto control = dynamic_cast<CControl*>(view))
        {
            if (int32_t(tag) < control->getTag())
                return control;
        }
    }

    return nullptr;
}

//------------------------------------------------------------------------
static auto find_view_with_tag(CViewContainer* container, size_t tag) -> CView*
{
    for (uint32_t i = 0; i < container->getNbViews(); i++)
    {
        auto* view = container->getView(i);
        if (auto control = dynamic_cast<CControl*>(view))
        {
            if (control->getTag() == uint32_t(tag))
                return control;
        }
    }

    return nullptr;
}

//------------------------------------------------------------------------
using Word = std::string;
static auto compute_word_width(const IUIDescription* description,
                               Word word) -> CCoord
{
    static constexpr auto FONT_ID = "ListEntryFont";
    return description->getFont(FONT_ID)
        ->getPlatformFont()
        ->getPainter()
        ->getStringWidth(nullptr, UTF8String(word).getPlatformString(), true);
}

//------------------------------------------------------------------------
static auto update_text_document(const IUIDescription* description,
                                 const UIAttributes& attributes,
                                 IControlListener* listener,
                                 CViewContainer* text_document,
                                 const MetaWordsData& meta_words_data) -> void
{
    if (!text_document)
        return;

    const auto word_widths =
        compute_word_widths(meta_words_data, [&](const Word& word) {
            return compute_word_width(description, word);
        });

    // Remove views
    std::vector<CControl*> views_to_remove;
    text_document->forEachChild([&](CView* child) {
        if (auto* control = dynamic_cast<CControl*>(child))
        {
            if (!meta_words_data.words[control->getTag()].is_clipped_by_region)
                views_to_remove.push_back(control);
        }
    });
    for (auto* child : views_to_remove)
        text_document->removeView(child);

    // Add new views
    for (size_t i = 0; i < meta_words_data.words.size(); ++i)
    {
        const auto word_data = meta_words_data.words[i];
        if (!word_data.is_clipped_by_region)
            continue;

        if (find_view_with_tag(text_document, i))
            continue;

        auto view =
            description->getViewFactory()->createView(attributes, description);
        auto but      = dynamic_cast<CTextButton*>(view);
        auto but_size = but->getViewSize();
        but_size.setWidth(word_widths[i]);
        but->setViewSize(but_size);
        but->setTitle(UTF8String(meta_words_data.words[i].word.word));
        // Set gradients to nullptr. This drastically speeds up the
        // performance!!!
        but->setGradient(nullptr);
        but->setGradientHighlighted(nullptr);

        but->setTag(int32_t(i));
        but->setListener(listener);
        auto* view_after = find_view_after(text_document, i);
        text_document->addView(but, view_after);
    }

    fit_content(text_document->getParentView());
}

//------------------------------------------------------------------------
// Helper class to call sizeToFit to all parents up the hierarchy
// AFTER a view has been attached or resized itself.
//------------------------------------------------------------------------
class FitContent : public ViewListenerAdapter
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
    const IUIDescription* description)
: description(description)
{
}

//------------------------------------------------------------------------
MetaWordsClipController::~MetaWordsClipController()
{
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

    if (timeDisplay)
    {
        update_time_display_control(*timeDisplay, data);
        timeDisplay->invalid();
    }

    if (listTitle)
    {
        update_label_control(*listTitle, data);
        listTitle->invalid();
    }

    if (text_document)
    {
        update_text_document(description, meta_word_button_attributes, this,
                             text_document, data);
        text_document->invalid();
    }
}

//------------------------------------------------------------------------
CView* MetaWordsClipController::verifyView(CView* view,
                                           const UIAttributes& attributes,
                                           const IUIDescription* description)
{
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
void MetaWordsClipController::valueChanged(CControl* pControl)
{
    if (pControl)
    {
        list_value_changed_func(pControl->getTag());
    }
}

//------------------------------------------------------------------------
} // namespace mam
