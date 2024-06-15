// Copyright(c) 2024 Max And Me.

#pragma once

#include "ara_document_controller.h"
#include "nonstd.h"
#include "supress_warnings.h"
#include "views/waveform_view.h"
BEGIN_SUPRESS_WARNINGS
#include "base/source/fobject.h"
#include "eventpp/callbacklist.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/uidescription/icontroller.h"
END_SUPRESS_WARNINGS

namespace VSTGUI {
class CListControl;
class CGradientView;
} // namespace VSTGUI

namespace mam {

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

    WaveFormController(ARADocumentController* controller);
    ~WaveFormController() override;

    bool initialize(Subject* subject, FuncWaveFormData&& waveform_data_func);

    void PLUGIN_API update(FUnknown* /*changedUnknown*/,
                           Steinberg::int32 /*message*/) override {};

    VSTGUI::CView*
    createView(const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;

    VSTGUI::CView*
    verifyView(VSTGUI::CView* view,
               const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;

    // IControlListener
    void valueChanged(VSTGUI::CControl* /*pControl*/) override {};

    // ViewListenerAdapter
    void viewAttached(VSTGUI::CView* view) override;
    void viewWillDelete(VSTGUI::CView* view) override;

    OBJ_METHODS(WaveFormController, FObject)

    //--------------------------------------------------------------------
private:
    void on_selected_region_word(const SelectedWordEventData& selected_word);
    void update_waveform();
    void
    register_region_props_observer(const SelectedWordEventData& selected_word);
    void unregister_current_region_props_observer();

    ARADocumentController* controller = nullptr;
    Subject* subject                  = nullptr;
    ObserverHandle observer_handle;
    FuncWaveFormData waveform_data_func;
    WaveFormView* waveform_view            = nullptr;
    VSTGUI::CGradientView* background_view = nullptr;

    RegionPropsChangedCallback::Handle region_props_observer_handle;
    SelectedWordEventData selected_word;
};

//------------------------------------------------------------------------
} // namespace mam
