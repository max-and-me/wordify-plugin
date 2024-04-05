//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "base/source/fobject.h"
#include "gsl/span"
#include "tiny_observer_pattern.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/uidescription/icontroller.h"

namespace VSTGUI {
class CListControl;
class CGradientView;
} // namespace VSTGUI
namespace mam {
class WaveFormView;

//------------------------------------------------------------------------
// WaveFormController
//------------------------------------------------------------------------
class WaveFormController : public Steinberg::FObject, public VSTGUI::IController
{
public:
    //--------------------------------------------------------------------
    struct Data
    {
        using Color       = std::tuple<double, double, double>;
        using AudioBuffer = gsl::span<const float>;

        Color color;
        AudioBuffer audio_buffer;
    };

    using FuncWaveFormData = std::function<const Data()>;

    WaveFormController(tiny_observer_pattern::SimpleSubject* subject);
    virtual ~WaveFormController();

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

    OBJ_METHODS(WaveFormController, FObject)

    //--------------------------------------------------------------------
private:
    void onDataChanged();

    tiny_observer_pattern::SimpleSubject* subject = nullptr;
    tiny_observer_pattern::ObserverID observer_id = 0;
    FuncWaveFormData waveform_data_func;
    WaveFormView* waveform_view            = nullptr;
    VSTGUI::CGradientView* background_view = nullptr;
};

//------------------------------------------------------------------------
} // namespace mam
