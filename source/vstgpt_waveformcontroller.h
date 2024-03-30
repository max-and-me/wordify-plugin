//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ara_document_controller.h"
#include "base/source/fobject.h"
#include "vstgpt_waveformview.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/uidescription/icontroller.h"
#include <functional>

namespace VSTGUI {
class CListControl;
}
namespace mam {

//------------------------------------------------------------------------
// VstGPTWaveFormController
//------------------------------------------------------------------------
class VstGPTWaveFormController : public Steinberg::FObject,
                                 public VSTGUI::IController
{
public:
    //--------------------------------------------------------------------
    VstGPTWaveFormController(
        ARADocumentController* controller,
        ARADocumentController::FnGetSampleRate&& fn_get_playback_sample_rate);
    virtual ~VstGPTWaveFormController();

    void PLUGIN_API update(FUnknown* changedUnknown,
                           Steinberg::int32 message) override {};
    VSTGUI::CView*
    verifyView(VSTGUI::CView* view,
               const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override
    {
        return view;
    }

    VSTGUI::CView*
    createView(const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;

    // IControlListener
    void valueChanged(VSTGUI::CControl* pControl) override {};

    void onDataChanged();

    OBJ_METHODS(VstGPTWaveFormController, FObject)

    //--------------------------------------------------------------------
private:
    ARADocumentController* controller = nullptr;
    ARADocumentController::MetaWordsDataList cached_meta_words_data_list;
    tiny_observer_pattern::ObserverID observer_id = 0;
    ARADocumentController::FnGetSampleRate func_playback_sample_rate;

    WaveformView* view = nullptr;
};

//------------------------------------------------------------------------
} // namespace mam
