//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ara_document_controller.h"
#include "base/source/fobject.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/uidescription/icontroller.h"

namespace VSTGUI {
class CListControl;
class CGradientView;
} // namespace VSTGUI
namespace mam {
class WaveformView;
namespace meta_words {
class PlaybackRegion;
}

//------------------------------------------------------------------------
// VstGPTWaveFormController
//------------------------------------------------------------------------
class VstGPTWaveFormController : public Steinberg::FObject,
                                 public VSTGUI::IController
{
public:
    //--------------------------------------------------------------------
    using SampleRate        = double;
    using FuncGetSampleRate = std::function<SampleRate()>;

    VstGPTWaveFormController(ARADocumentController* controller,
                             FuncGetSampleRate func_playback_sample_rate);
    virtual ~VstGPTWaveFormController();

    void PLUGIN_API update(FUnknown* changedUnknown,
                           Steinberg::int32 message) override{};

    VSTGUI::CView*
    createView(const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;

    VSTGUI::CView*
    verifyView(VSTGUI::CView* view,
               const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;

    // IControlListener
    void valueChanged(VSTGUI::CControl* pControl) override{};

    void set_playback_region(const meta_words::PlaybackRegion* playback_region);

    OBJ_METHODS(VstGPTWaveFormController, FObject)

    //--------------------------------------------------------------------
private:
    void onDataChanged();

    ARADocumentController* controller = nullptr;
    tiny_observer_pattern::ObserverID observer_id = 0;
    ARADocumentController::FnGetSampleRate func_playback_sample_rate;
    WaveformView* waveform_view            = nullptr;
    const meta_words::PlaybackRegion* playback_region  = nullptr;
    VSTGUI::CGradientView* background_view = nullptr;
};

//------------------------------------------------------------------------
} // namespace mam
