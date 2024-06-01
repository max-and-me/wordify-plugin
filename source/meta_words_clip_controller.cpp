//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "meta_words_clip_controller.h"

#include "hilite_text_button.h"
#include "hstack_layout.h"
#include "little_helpers.h"
#include "meta_words_data.h"
#include "vstgui/lib/animation/ianimationtarget.h"
#include "vstgui/lib/animation/timingfunctions.h"
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
#include <cmath>
#include <optional>

using namespace VSTGUI;

namespace mam {
//------------------------------------------------------------------------
// Normally these are not available from outside, but we need them here.
static CViewAttributeID kTemplateNameAttributeID = 'uitl';

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
                         CViewContainer* region_transcript,
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

        // Little workaround for sizeToFit function of the button. sizeToFit
        // takes the round radius into account. That's not what we want here! So
        // we put it to zero, sizeToFit and put it back after.
        auto size_to_fit = [](CTextButton* button) {
            const auto radius = button->getRoundRadius();
            button->setRoundRadius(0.);
            button->sizeToFit();
            button->setRoundRadius(radius);
        };

        auto button   = opt_button.value();
        auto but_size = button->getViewSize();
        button->setViewSize(but_size.setWidth(but_width));
        button->setTitle(but_title);
        size_to_fit(button);
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
// LoadingIndicatorAnimationHandler
//------------------------------------------------------------------------
class LoadingIndicatorAnimationHandler : public ViewListenerAdapter
{
    struct WaveAnimation : public Animation::IAnimationTarget,
                           public NonAtomicReferenceCounted
    {
        static const char* ANIMATION_ID;
        using Rects = std::vector<CRect>;
        Rects rects;

        void animationStart(CView* view, IdStringPtr name) override
        {
            if (const auto* container = view->asViewContainer())
            {
                container->forEachChild([&](CView* child) {
                    rects.push_back(child->getViewSize());
                });
            }
        }

        void animationTick(CView* view, IdStringPtr name, float pos) override
        {
            if (!view)
                return;

            if (auto* container = view->asViewContainer())
            {
                const auto OFFSET = M_PI / double(container->getNbViews());
                size_t index      = 0;
                container->forEachChild([&](CView* child) {
                    constexpr auto TWO_PI = 2. * M_PI;
                    const auto phase      = TWO_PI * pos;
                    const auto phase_shift =
                        static_cast<double>(index) * OFFSET;
                    const auto value        = std::sin(phase - phase_shift);
                    const auto val_positive = std::max(0., value);

                    // Only fade dots out half
                    child->setAlphaValue(0.5 + val_positive * 0.5);

                    // Move them up by delta in pixel
                    constexpr auto DELTA_PX = .5;
                    auto r                  = rects.at(index);
                    const auto extend_delta = DELTA_PX * val_positive;
                    r.extend(extend_delta, extend_delta);
                    child->setViewSize(r);

                    if (auto* label = dynamic_cast<CTextLabel*>(child))
                        label->setRoundRectRadius(r.getWidth() * 0.5);

                    index++;
                });
            }
        }

        void animationFinished(CView* view,
                               IdStringPtr name,
                               bool wasCanceled) override
        {
        }
    };

    void viewAttached(CView* view) override
    {
        constexpr auto PERIOD_DURATION = 1200.;
        constexpr auto REPEAT_FOREVER  = std::numeric_limits<int32_t>().max();

        auto* timing_function =
            new Animation::LinearTimingFunction(PERIOD_DURATION);
        auto* repeater = new Animation::RepeatTimingFunction(
            timing_function, REPEAT_FOREVER, false);

        if (view)
            view->addAnimation(WaveAnimation::ANIMATION_ID, new WaveAnimation,
                               repeater);
    }

    void viewRemoved(CView* view) override
    {
        if (view)
            view->removeAnimation(WaveAnimation::ANIMATION_ID);
    }

    void viewWillDelete(CView* view) override
    {
        if (view)
        {
            view->unregisterViewListener(this);
            delete this;
        }
    }
};

const char* LoadingIndicatorAnimationHandler::WaveAnimation::ANIMATION_ID =
    "WaveAnimation";

//------------------------------------------------------------------------
static auto add_loading_indicator(CViewContainer* region_transcript,
                                  const IUIDescription* description) -> void
{
    if (region_transcript->getNbViews() > 0)
        return;

    if (auto indicator_template =
            description->createView("LoadingIndicatorTemplate", nullptr))
    {
        region_transcript->addView(indicator_template);

        if (auto* container = dynamic_cast<CViewContainer*>(indicator_template))
        {
            // TODO: Needs to be improved!!!!
            constexpr auto DOT_GROUP_VIEW_INDEX = 1;
            if (auto* dot_group = container->getView(DOT_GROUP_VIEW_INDEX))
            {
                dot_group->registerViewListener(
                    new LoadingIndicatorAnimationHandler);
            }
        }
    }
}

//------------------------------------------------------------------------
static auto remove_loading_indicator(CViewContainer* region_transcript) -> void
{
    if (region_transcript->getNbViews() == 1)
    {
        if (CView* view = region_transcript->getView(0))
        {
            uint32_t attrSize = 0;
            if (view->getAttributeSize(kTemplateNameAttributeID, attrSize))
            {
                char* str = new char[attrSize];
                if (view->getAttribute(kTemplateNameAttributeID, attrSize, str,
                                       attrSize))
                {
                    if (UTF8String("LoadingIndicatorTemplate") == str)
                    {
                        region_transcript->removeView(view);
                    }
                }
                delete[] str;
            }
        }
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

        if (const auto* view_name =
                attributes.getAttributeValue("uidesc-label"))
        {
            if (*view_name == "MetaWordButton")
            {
                auto* btn = new HiliteTextButton(CRect(), listener);
                but_factory->applyCustomViewAttributeValues(
                    btn, IdStringPtr("CTextButton"), attributes, description);

                btn->verifyTextButtonView(description);

                return std::make_optional(btn);
            }
        }

        return std::nullopt;
    };

    remove_word_buttons(*region_transcript, meta_words_data);
    insert_word_buttons(cache, meta_words_data, region_transcript, but_creator);
}

//------------------------------------------------------------------------
// Helper class to call sizeToFit to all parents up the hierarchy
// AFTER a view has been attached or resized itself.
//------------------------------------------------------------------------
struct FitContentHandler : public ViewListenerAdapter
{
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

    void viewAttached(CView* view) override
    {
        if (view)
            fit_content(view->getParentView());
    }

    void viewSizeChanged(CView* view, const CRect& oldSize) override
    {
        if (view)
            fit_content(view->getParentView());
    }

    void viewWillDelete(CView* view) override
    {
        if (auto c = view->asViewContainer())
        {
            c->unregisterViewListener(this);
            delete this;
        }
    }
};

//------------------------------------------------------------------------
// MetaWordsClipController
//------------------------------------------------------------------------
MetaWordsClipController::MetaWordsClipController(
    const IUIDescription* description)
: description(description)
{
}

//------------------------------------------------------------------------
MetaWordsClipController::~MetaWordsClipController()
{
    if (subject)
    {
        subject->remove(observer_handle);
    }

    if (region_title)
    {
        region_title->unregisterViewListener(this);
        region_title = nullptr;
    }

    if (region_start_time)
    {
        region_start_time->unregisterViewListener(this);
        region_start_time = nullptr;
    }

    if (region_duration_time)
    {
        region_duration_time->unregisterViewListener(this);
        region_duration_time = nullptr;
    }

    if (region_transcript)
    {
        region_transcript->unregisterViewListener(this);
        region_transcript = nullptr;
    }
}

//------------------------------------------------------------------------
bool MetaWordsClipController::initialize(Subject* subject)
{
    if (!subject)
        return false;

    this->subject = subject;

    observer_handle =
        subject->append([&]() { this->on_meta_words_data_changed(); });

    auto view = description->createView("TextWordTemplate", this);
    if (view)
        view->forget();

    if (!meta_words_data_func)
        return false;

    return true;
}

//------------------------------------------------------------------------
void MetaWordsClipController::on_meta_words_data_changed()
{
    const auto& data = meta_words_data_func();

    if (region_start_time)
        update_region_start_time(*region_start_time, data);

    if (region_duration_time)
        update_region_duration_time(*region_duration_time, data);

    if (region_title)
        update_region_title(*region_title, data);

    if (region_transcript)
    {
        init_words_width_cache(data);

        if (!data.words.empty())
            remove_loading_indicator(region_transcript);

        update_region_transcript(region_transcript, data, description,
                                 meta_word_button_attributes, this, cache);
        region_transcript->invalid();
    }
}

//------------------------------------------------------------------------
void MetaWordsClipController::init_words_width_cache(const MetaWordsData& data)
{
    if (cache.word_widths.empty())
    {
        cache.word_widths = compute_word_widths(data, [&](const Word& word) {
            return compute_word_width(description, word);
        });
    }
}

//------------------------------------------------------------------------
CView*
MetaWordsClipController::verifyView(CView* view,
                                    const UIAttributes& attributes,
                                    const IUIDescription* /*description*/)
{
    if (!view)
        return view;

    if (auto viewLabel =
            attributes.getAttributeValue(UIViewCreator::kAttrUIDescLabel))
    {
        if (*viewLabel == "RegionStartTime")
        {
            region_start_time = dynamic_cast<CTextLabel*>(view);
            region_start_time->registerViewListener(this);
        }
        else if (*viewLabel == "RegionDurationTime")
        {
            region_duration_time = dynamic_cast<CTextLabel*>(view);
            region_duration_time->registerViewListener(this);
        }
        else if (*viewLabel == "RegionTitle")
        {
            region_title = dynamic_cast<CTextLabel*>(view);
            region_title->registerViewListener(this);
        }
        else if (*viewLabel == "RegionTranscript")
        {
            const auto data = meta_words_data_func();

            init_words_width_cache(data);
            region_transcript = view->asViewContainer();
            region_transcript->registerViewListener(this);
            region_transcript->registerViewListener(new FitContentHandler);

            stack_layout = std::make_unique<HStackLayout>(region_transcript);
            stack_layout->setup({0., 0.}, {0., 0., 0., 0.});

            if (data.words.empty())
                add_loading_indicator(region_transcript, description);
        }
        else if (*viewLabel == "MetaWordButton")
        {
            meta_word_button_attributes = attributes;
        }
    }

    return view;
};

//------------------------------------------------------------------------
void MetaWordsClipController::valueChanged(CControl* pControl)
{
    if (pControl && on_select_word_func)
    {
        on_select_word_func(pControl->getTag());
    }
}

//------------------------------------------------------------------------
void MetaWordsClipController::viewAttached(CView* view)
{
    const auto& data = meta_words_data_func();
    if (view == region_start_time)
    {
        update_region_start_time(*region_start_time, data);
    }
    else if (view == region_duration_time)
    {
        update_region_duration_time(*region_duration_time, data);
    }
    else if (view == region_title)
    {
        update_region_title(*region_title, data);
    }
    else if (view == region_transcript)
    {
        update_region_transcript(region_transcript, data, description,
                                 meta_word_button_attributes, this, cache);
    }
}

//------------------------------------------------------------------------
void MetaWordsClipController::viewRemoved(CView* view) {}

//------------------------------------------------------------------------
void MetaWordsClipController::viewWillDelete(CView* view)
{
    if (view == region_title)
    {
        region_title->unregisterViewListener(this);
        region_title = nullptr;
    }
    else if (view == region_start_time)
    {
        region_start_time->unregisterViewListener(this);
        region_start_time = nullptr;
    }
    else if (view == region_duration_time)
    {
        region_duration_time->unregisterViewListener(this);
        region_duration_time = nullptr;
    }
    else if (view == region_transcript)
    {
        region_transcript->unregisterViewListener(this);
        region_transcript = nullptr;
    }
}

//------------------------------------------------------------------------
} // namespace mam
