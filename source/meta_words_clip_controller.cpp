//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "meta_words_clip_controller.h"
#include "hstack_layout.h"
#include "list_entry_controller.h"
#include "little_helpers.h"
#include "meta_words_data.h"
#include "vstgui/lib/ccolor.h"
#include "vstgui/lib/cfont.h"
#include "vstgui/lib/controls/cbuttons.h"
#include "vstgui/lib/controls/ctextlabel.h"
#include "vstgui/lib/cpoint.h"
#include "vstgui/lib/crect.h"
#include "vstgui/lib/crowcolumnview.h"
#include "vstgui/lib/platform/iplatformfont.h"
#include "vstgui/lib/platform/platformfactory.h"
#include "vstgui/uidescription/detail/uiviewcreatorattributes.h"
#include "vstgui/uidescription/iviewfactory.h"
#include "vstgui/uidescription/uiattributes.h"
#include <optional>

using namespace VSTGUI;

namespace mam {

//------------------------------------------------------------------------
static auto update_region_title(CTextLabel& region_title,
                                const MetaWordsData& meta_words_data) -> void
{
    auto [r, g, b]     = meta_words_data.color;
    const CColor color = make_color<float>(r, g, b, std::nullopt);
    region_title.setFontColor(color);
    region_title.setText(UTF8String(meta_words_data.name));
    region_title.sizeToFit();
}

//------------------------------------------------------------------------
static auto
update_region_start_time(CTextLabel& region_start_time,
                         const MetaWordsData& meta_words_data) -> void
{
    const auto str = to_time_display_string(meta_words_data.project_time_start);
    region_start_time.setText(UTF8String(str));
}

//------------------------------------------------------------------------
static auto
update_region_duration_time(CTextLabel& region_duration_time,
                            const MetaWordsData& meta_words_data) -> void
{
    const auto str = to_time_display_string(meta_words_data.duration);
    region_duration_time.setText(UTF8String(str));
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
                                const Func& width_func) -> WordWidths
{
    WordWidths widths;
    for (const auto& meta_word_data : meta_words_data.words)
    {
        const auto width = width_func(meta_word_data.word.word);
        widths.push_back(width);
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
    static constexpr auto FONT_ID = "region_transcript_font";
    return description->getFont(FONT_ID)
        ->getPlatformFont()
        ->getPainter()
        ->getStringWidth(nullptr, UTF8String(word).getPlatformString(), true);
}

//------------------------------------------------------------------------
static auto remove_word_buttons(CViewContainer& region_transcript,
                                const MetaWordsData& meta_words_data)
{
    using Controls = std::vector<CControl*>;

    // Remove views
    Controls buttons_to_remove;
    region_transcript.forEachChild([&](CView* child) {
        if (auto* control = dynamic_cast<CControl*>(child))
        {
            const auto word_index = control->getTag();
            bool to_be_removed    = true;
            if (size_t(word_index) < meta_words_data.words.size())
            {
                to_be_removed =
                    meta_words_data.words[word_index].is_clipped_by_region;
            }

            if (to_be_removed)
                buttons_to_remove.push_back(control);
        }
    });

    for (auto* child : buttons_to_remove)
        region_transcript.removeView(child);
}

//------------------------------------------------------------------------
using OptTextButton = std::optional<CTextButton*>;
template <typename Func>
void insert_word_buttons(const mam::MetaWordsClipController::Cache& cache,
                         const mam::MetaWordsData& meta_words_data,
                         VSTGUI::CViewContainer* region_transcript,
                         Func&& but_create_func)
{
    const auto& word_widths = cache.word_widths;
    for (size_t word_index = 0; word_index < meta_words_data.words.size();
         ++word_index)
    {
        // Don't add a button for words which are clipped
        const auto word_data = meta_words_data.words[word_index];
        if (word_data.is_clipped_by_region)
            continue;

        // Continue if button already exists
        if (find_view_with_tag(region_transcript, word_index))
            continue;

        // Setting gradients to nullptr improves performance quite a lot when
        // redrawing
        const auto but_gradient = nullptr;
        const auto but_title    = UTF8String(word_data.word.word);
        const auto but_width    = word_widths[word_index];
        const auto but_enabled  = !word_data.is_punctuation_mark;
        const auto but_tag      = int32_t(word_index);

        OptTextButton opt_button = but_create_func();
        if (!opt_button.has_value())
            continue;

        auto button   = opt_button.value();
        auto but_size = button->getViewSize();
        button->setViewSize(but_size.setWidth(but_width));
        button->setTitle(but_title);
        button->setMouseEnabled(but_enabled);
        button->setGradient(but_gradient);
        button->setGradientHighlighted(but_gradient);
        button->setTag(but_tag);

        // Insert the button at position
        auto* view_after = find_view_after(region_transcript, but_tag);
        region_transcript->addView(button, view_after);
    }
}

//------------------------------------------------------------------------
static auto
update_region_transcript(CViewContainer* region_transcript,
                         const MetaWordsData& meta_words_data,
                         const IUIDescription* description,
                         const UIAttributes& attributes,
                         IControlListener* listener,

                         const MetaWordsClipController::Cache& cache) -> void
{
    if (!region_transcript)
        return;

    auto but_creator = [&]() -> OptTextButton {
        const auto but_factory = description->getViewFactory();
        if (!but_factory)
            return std::nullopt;

        auto view = but_factory->createView(attributes, description);
        if (auto button = dynamic_cast<CTextButton*>(view))
        {
            button->setListener(listener);
            return std::make_optional(button);
        }
        return std::nullopt;
    };

    remove_word_buttons(*region_transcript, meta_words_data);
    insert_word_buttons(cache, meta_words_data, region_transcript, but_creator);

    fit_content(region_transcript->getParentView());
}

//------------------------------------------------------------------------
template <typename C, typename Func>
static auto update_control(C* c, const MetaWordsData& data, Func& func) -> void
{
    if (!c)
        return;

    if (!func)
        return;

    func(*c, data);
    c->invalid();
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
    if (region_transcript)
        region_transcript->unregisterViewListener(view_listener.get());
}

//------------------------------------------------------------------------
bool MetaWordsClipController::initialize(
    Subject* subject, FuncMetaWordsData&& meta_words_data_func)
{
    observer = tiny_observer_pattern::make_observer(
        subject, [&](const auto&) { on_meta_words_data_changed(); });

    this->meta_words_data_func = std::move(meta_words_data_func);

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

    update_control(region_start_time, data, update_region_start_time);
    update_control(region_duration_time, data, update_region_duration_time);
    update_control(region_title, data, update_region_title);

    if (region_transcript)
    {
        if (cache.word_widths.empty())
        {
            cache.word_widths =
                compute_word_widths(data, [&](const Word& word) {
                    return compute_word_width(description, word);
                });
        }
        update_region_transcript(region_transcript, data, description,
                                 meta_word_button_attributes, this, cache);
        region_transcript->invalid();
    }
}

//------------------------------------------------------------------------
CView* MetaWordsClipController::verifyView(CView* view,
                                           const UIAttributes& attributes,
                                           const IUIDescription* description)
{
    if (!region_start_time)
    {
        if (auto viewLabel =
                attributes.getAttributeValue(UIViewCreator::kAttrUIDescLabel))
        {
            if (*viewLabel == "RegionStartTime")
            {
                if (region_start_time = dynamic_cast<CTextLabel*>(view))
                    update_region_start_time(*region_start_time,
                                             meta_words_data_func());
            }
        }
    }

    if (!region_duration_time)
    {
        if (auto viewLabel =
                attributes.getAttributeValue(UIViewCreator::kAttrUIDescLabel))
        {
            if (*viewLabel == "RegionDurationTime")
            {
                if (region_duration_time = dynamic_cast<CTextLabel*>(view))
                    update_region_duration_time(*region_duration_time,
                                                meta_words_data_func());
            }
        }
    }

    if (!region_title)
    {
        if (auto viewLabel =
                attributes.getAttributeValue(UIViewCreator::kAttrUIDescLabel))
        {
            if (*viewLabel == "RegionTitle")
            {
                if (region_title = dynamic_cast<CTextLabel*>(view))
                    update_region_title(*region_title, meta_words_data_func());
            }
        }
    }

    if (!region_transcript)
    {
        if (auto viewLabel =
                attributes.getAttributeValue(UIViewCreator::kAttrUIDescLabel))
        {
            if (*viewLabel == "RegionTranscript")
            {
                if (cache.word_widths.empty())
                {
                    cache.word_widths = compute_word_widths(
                        meta_words_data_func(), [&](const Word& word) {
                            return compute_word_width(description, word);
                        });
                }

                region_transcript = dynamic_cast<CViewContainer*>(view);
                stack_layout =
                    std::make_unique<HStackLayout>(region_transcript);
                stack_layout->setup({0., 0.}, {0., 0., 0., -4});
                update_region_transcript(
                    region_transcript, meta_words_data_func(), description,
                    meta_word_button_attributes, this, cache);

                view_listener = std::make_unique<FitContent>(region_transcript);
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
