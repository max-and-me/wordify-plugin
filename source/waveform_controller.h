//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ara_document_controller.h"
#include "base/source/fobject.h"
#include "eventpp/callbacklist.h"
#include "nonstd.h"
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
class WaveFormController : public Steinberg::FObject,
                           public VSTGUI::IController,
                           public VSTGUI::ViewListenerAdapter
{
public:
    //--------------------------------------------------------------------
    using Subject = eventpp::CallbackList<void(const SelectedWordEventData&)>;
    using ObserverHandle = Subject::Handle;
    using Data           = const WaveFormView::Data;

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

    // ViewListenerAdapter
    void viewAttached(VSTGUI::CView* view) override;
    void viewWillDelete(VSTGUI::CView* view) override;

    OBJ_METHODS(WaveFormController, FObject)

    //--------------------------------------------------------------------
private:
    void on_meta_words_data_changed();

    Subject* subject = nullptr;
    ObserverHandle observer_handle;
    FuncWaveFormData waveform_data_func;
    WaveFormView* waveform_view            = nullptr;
    VSTGUI::CGradientView* background_view = nullptr;
};

//------------------------------------------------------------------------
} // namespace mam
