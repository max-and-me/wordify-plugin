//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "vstgpt_listcontroller.h"
#include "mam/meta_words/meta_word.h"
#include "vstgui/lib/controls/clistcontrol.h"
#include "vstgui/lib/controls/cstringlist.h"
#include "vstgui/lib/controls/ctextlabel.h"
#include "vstgui/lib/controls/icontrollistener.h"
#include "vstgui/lib/crowcolumnview.h"
#include "vstgui/lib/cstring.h"
#include "vstgui/lib/events.h"
#include "vstgui/lib/platform/platformfactory.h"
#include "vstgui/uidescription/iuidescription.h"
#include "waveform_controller.h"
#include <iterator>

//------------------------------------------------------------------------
namespace mam {
using namespace ::VSTGUI;

//------------------------------------------------------------------------
static auto update_list_control_content(CListControl& listControl,
                                        const meta_words::MetaWords& words)
    -> void
{
    listControl.setMax(words.size() - 1);
    listControl.recalculateLayout();

    if (auto stringListDrawer =
            dynamic_cast<StringListControlDrawer*>(listControl.getDrawer()))
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
static auto update_label_control(CTextLabel& label, const MetaWordsData& data)
    -> void
{
    const VSTGUI::CColor color(data.color.r, data.color.g, data.color.b);
    label.setFontColor(color);
    label.setText(VSTGUI::UTF8String(data.name));
}

//------------------------------------------------------------------------
static auto onRequestSelectWord(int index,
                                const mam::MetaWordsData& data,
                                ARADocumentController& document_controller)
    -> void
{
    const auto& meta_words_data = data;
    const auto& words           = meta_words_data.words;
    const auto& selected_word   = words.at(index);

    const auto new_position =
        selected_word.begin + meta_words_data.project_offset;
    document_controller.onRequestLocatorPosChanged(new_position);
}

//------------------------------------------------------------------------
// VstGPTWaveClipListController
//------------------------------------------------------------------------
class MetaWordsClipController : public Steinberg::FObject,
                                public VSTGUI::IController
{
public:
    //--------------------------------------------------------------------
    using FuncMetaWordsData    = std::function<const MetaWordsData()>;
    using FuncListValueChanged = std::function<void(int)>;

    MetaWordsClipController(tiny_observer_pattern::SimpleSubject* subject)
    : subject(subject)
    {
        if (subject)
            observer_id = subject->add_listener(
                [this](const auto&) { this->onDataChanged(); });
    }

    ~MetaWordsClipController() override
    {
        if (subject)
            subject->remove_listener(observer_id);
    };

    void onDataChanged()
    {
        const auto& data = meta_words_data_func();
        if (listControl)
        {
            update_list_control_content(*listControl, data.words);
            listControl->setDirty();
        }

        if (label)
        {
            update_label_control(*label, data);
            label->setDirty();
        }
    }

    VSTGUI::CView*
    verifyView(VSTGUI::CView* view,
               const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override
    {

        if (!listControl)
        {
            if (listControl = dynamic_cast<CListControl*>(view))
            {
                listControl->registerControlListener(this);
                update_list_control_content(*listControl,
                                            meta_words_data_func().words);
            }
        }
        if (!label)
        {
            if (label = dynamic_cast<CTextLabel*>(view))
                update_label_control(*label, meta_words_data_func());
        }

        return view;
    };

    // IControlListener
    void valueChanged(VSTGUI::CControl* pControl) override
    {
        if (pControl && pControl == listControl)
        {
            list_value_changed_func(listControl->getValue());
        }
    }

    auto
    set_meta_words_data_func(const FuncMetaWordsData&& meta_words_data_func)
        -> void
    {
        this->meta_words_data_func = meta_words_data_func;
    }

    auto
    set_list_clicked_func(const FuncListValueChanged&& list_value_changed_func)
        -> void
    {
        this->list_value_changed_func = list_value_changed_func;
    }

    OBJ_METHODS(MetaWordsClipController, FObject)
    //--------------------------------------------------------------------
private:
    VSTGUI::CListControl* listControl = nullptr;
    VSTGUI::CTextLabel* label         = nullptr;

    FuncMetaWordsData meta_words_data_func;
    FuncListValueChanged list_value_changed_func;
    tiny_observer_pattern::SimpleSubject* subject = nullptr;
    tiny_observer_pattern::ObserverID observer_id = 0;
};

//------------------------------------------------------------------------
// VstGPTListController
//------------------------------------------------------------------------
VstGPTListController::VstGPTListController(
    ARADocumentController& controller,
    ARADocumentController::FnGetSampleRate&& fn_get_playback_sample_rate)
: controller(controller)
, fn_get_playback_sample_rate(fn_get_playback_sample_rate)
{
    observer_id =
        controller.add_listener([this](const auto&) { this->onDataChanged(); });
}

//------------------------------------------------------------------------
VstGPTListController::~VstGPTListController()
{
    controller.remove_listener(observer_id);
}

//------------------------------------------------------------------------
CView* VstGPTListController::verifyView(CView* view,
                                        const UIAttributes& /*attributes*/,
                                        const IUIDescription* description)
{
    if (!rowColView)
    {
        if (rowColView = dynamic_cast<CRowColumnView*>(view))
        {
            controller.for_each_playback_region(
                [&](const meta_words::PlaybackRegion* playbackRegion) {
                    if (!playbackRegion)
                        return;

                    const IUIDescription* uidescription = description;
                    if (!uidescription)
                        return;

                    this->tmp_playback_region = playbackRegion;
                    auto* newView =
                        uidescription->createView("ListEntryTemplate", this);
                    this->tmp_playback_region = nullptr;

                    if (newView)
                        rowColView->addView(newView);
                });
        }
    }

    return view;
}

//------------------------------------------------------------------------
void VstGPTListController::onDataChanged()
{
    // TODO: Do things when PlaybackRegions are added or removed
}

//------------------------------------------------------------------------
VSTGUI::IController* VstGPTListController::createSubController(
    VSTGUI::UTF8StringPtr name, const VSTGUI::IUIDescription* description)
{
    if (!this->tmp_playback_region)
        return nullptr;

    auto& subject         = controller.get_subject(this->tmp_playback_region);
    auto sample_rate_func = fn_get_playback_sample_rate;

    if (VSTGUI::UTF8StringView(name) == "MetaWordsClipController")
    {
        auto* subctrl = new MetaWordsClipController(&controller);
        subctrl->set_meta_words_data_func(
            [sample_rate_func, region = this->tmp_playback_region]() {
                return region->get_meta_words_data(sample_rate_func());
            });
        subctrl->set_list_clicked_func([&, sample_rate_func,
                                        region = this->tmp_playback_region](
                                           int index) {
            onRequestSelectWord(index,
                                region->get_meta_words_data(sample_rate_func()),
                                controller);
        });
        return subctrl;
    }
    else if (VSTGUI::UTF8StringView(name) == "WaveFormController")
    {
        auto* tmp_controller = new WaveFormController(&subject);
        tmp_controller->set_waveform_data_func(
            [sample_rate_func, region = this->tmp_playback_region]() {
                WaveFormController::Data data;
                data.audio_buffer =
                    region->get_audio_buffer(sample_rate_func());

                const auto color =
                    region->get_meta_words_data(sample_rate_func()).color;
                data.color = std::make_tuple(color.r, color.g, color.b);
                return data;
            });
        return tmp_controller;
    }

    return nullptr;
}

//------------------------------------------------------------------------
} // namespace mam
