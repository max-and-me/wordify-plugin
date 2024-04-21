//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "base/source/fobject.h"
#include "gsl/span"
#include "tiny_observer_pattern.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/uidescription/icontroller.h"
#include "waveform_view.h"

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
    using Subject     = tiny_observer_pattern::SimpleSubject;
    using Observer    = tiny_observer_pattern::Observer<Subject>;
    using ObserverPtr = std::unique_ptr<Observer>;
    using Data        = const WaveFormView::Data;

    using FuncWaveFormData = std::function<Data()>;

    WaveFormController();
    ~WaveFormController() override;

    bool initialize(Subject* subject, FuncWaveFormData&& waveform_data_func);

    void PLUGIN_API update(FUnknown* changedUnknown,
                           Steinberg::int32 message) override {};

    VSTGUI::CView*
    createView(const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;

    VSTGUI::CView*
    verifyView(VSTGUI::CView* view,
               const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;

    // IControlListener
    void valueChanged(VSTGUI::CControl* pControl) override {};

    OBJ_METHODS(WaveFormController, FObject)

    //--------------------------------------------------------------------
private:
    void on_meta_words_data_changed();

    ObserverPtr observer;
    FuncWaveFormData waveform_data_func;
    WaveFormView* waveform_view            = nullptr;
    VSTGUI::CGradientView* background_view = nullptr;
};

//------------------------------------------------------------------------
} // namespace mam
