//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "vstgpt_listcontroller.h"
#include "mam/meta_words/meta_word.h"
#include "meta_words_clip_controller.h"
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

using namespace VSTGUI;

namespace mam {

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
