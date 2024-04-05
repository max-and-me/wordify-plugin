//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "tiny_observer_pattern.h"
#include "base/source/fobject.h"
#include "gsl/span"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/uidescription/icontroller.h"

namespace VSTGUI {
class CListControl;
class CGradientView;
} // namespace VSTGUI
namespace mam {
class WaveformView;

//------------------------------------------------------------------------
// VstGPTWaveFormController
//------------------------------------------------------------------------
class VstGPTWaveFormController : public Steinberg::FObject,
                                 public VSTGUI::IController
{
public:
    //--------------------------------------------------------------------
    struct Data
    {
        using Color           = std::tuple<double, double, double>;
        using AudioBufferSpan = gsl::span<const float>;

        Color color;
        AudioBufferSpan audio_buffer;
    };

    using FuncWaveFormData = std::function<const Data()>;

    VstGPTWaveFormController(tiny_observer_pattern::SimpleSubject* subject);
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

    void set_waveform_data_func(const FuncWaveFormData&& waveform_data_func);

    OBJ_METHODS(VstGPTWaveFormController, FObject)

    //--------------------------------------------------------------------
private:
    void onDataChanged();

    tiny_observer_pattern::SimpleSubject* subject = nullptr;
    tiny_observer_pattern::ObserverID observer_id = 0;
    FuncWaveFormData waveform_data_func;
    WaveformView* waveform_view            = nullptr;
    VSTGUI::CGradientView* background_view = nullptr;
};

//------------------------------------------------------------------------
} // namespace mam
