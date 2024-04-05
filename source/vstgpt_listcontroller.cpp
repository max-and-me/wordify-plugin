//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "vstgpt_listcontroller.h"
#include "mam/meta_words/meta_word.h"
#include "vstgpt_waveformcontroller.h"
#include "vstgui/lib/controls/clistcontrol.h"
#include "vstgui/lib/controls/cstringlist.h"
#include "vstgui/lib/controls/ctextlabel.h"
#include "vstgui/lib/controls/icontrollistener.h"
#include "vstgui/lib/crowcolumnview.h"
#include "vstgui/lib/cstring.h"
#include "vstgui/lib/events.h"
#include "vstgui/lib/platform/platformfactory.h"
#include "vstgui/uidescription/iuidescription.h"
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
class VstGPTListEntryController : public Steinberg::FObject,
                                  public VSTGUI::IController
{
public:
    //--------------------------------------------------------------------
    VstGPTListEntryController(MetaWordsData& data,
                              ARADocumentController& controller)
    : data(data)
    , controller(controller)
    {
    }
    virtual ~VstGPTListEntryController(){};

    void updateData(MetaWordsData& _data)
    {
        data = _data;
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

    void PLUGIN_API update(FUnknown* changedUnknown,
                           Steinberg::int32 message) override{};
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
                update_list_control_content(*listControl, data.words);
            }
        }
        if (!label)
        {
            if (label = dynamic_cast<CTextLabel*>(view))
                update_label_control(*label, data);
        }

        return view;
    };
    // IControlListener
    void valueChanged(VSTGUI::CControl* pControl) override
    {
        if (pControl && pControl == listControl)
        {
            onRequestSelectWord(listControl->getValue(), data, controller);
        }
    }
    void controlBeginEdit(VSTGUI::CControl* pControl) override{};
    void controlEndEdit(VSTGUI::CControl* pControl) override{};

    VSTGUI::IController*
    createSubController(VSTGUI::UTF8StringPtr name,
                        const VSTGUI::IUIDescription* description) override
    {
        return nullptr;
    }

    OBJ_METHODS(VstGPTListEntryController, FObject)
    //--------------------------------------------------------------------
private:
    VSTGUI::CListControl* listControl = nullptr;
    VSTGUI::CTextLabel* label         = nullptr;

    MetaWordsData& data;
    ARADocumentController& controller;
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
    observer_id = controller.add_listener([this]() { this->onDataChanged(); });
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
            cached_meta_words_data_list = controller.collect_meta_data_words(
                fn_get_playback_sample_rate());

            controller.for_each_playback_region(
                [&](const meta_words::PlaybackRegion* playbackRegion) {
                    if (!playbackRegion)
                        return;

                    const IUIDescription* uidescription = description;
                    if (!uidescription)
                        return;

                    auto* newView =
                        uidescription->createView("ListEntryTemplate", this);

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
    cached_meta_words_data_list =
        controller.collect_meta_data_words(fn_get_playback_sample_rate());
    if (cached_meta_words_data_list.empty())
    {
        // TODO: Clear all controls!!!
        return;
    }

    for (int i = 0; i < subControllerList.size(); i++)
    {
        auto subCtrl =
            dynamic_cast<VstGPTListEntryController*>(subControllerList.at(i));

        subCtrl->updateData(cached_meta_words_data_list.at(i));
    }
}

//------------------------------------------------------------------------
VSTGUI::IController* VstGPTListController::createSubController(
    VSTGUI::UTF8StringPtr name, const VSTGUI::IUIDescription* description)
{
    if (cached_meta_words_data_list.empty())
        return nullptr;

    if (VSTGUI::UTF8StringView(name) == "MetaWordsClipController")
    {
        auto* subctrl = new VstGPTListEntryController(
            cached_meta_words_data_list.at(0), controller);
        subControllerList.push_back(subctrl);
        return subctrl;
    }
    else if (VSTGUI::UTF8StringView(name) == "WaveFormController")
    {
        return new VstGPTWaveFormController(&controller,
                                            fn_get_playback_sample_rate);
    }

    return nullptr;
}

//------------------------------------------------------------------------
} // namespace mam
