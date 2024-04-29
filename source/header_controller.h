//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ara_document_controller.h"
#include "base/source/fobject.h"
#include "vstgui/uidescription/icontroller.h"

namespace VSTGUI {
class CRowColLayout;
class CSearchTextEdit;
} // namespace VSTGUI

namespace Steinberg::Vst {
class Parameter;
}

namespace mam {
class SpinnerView;
//------------------------------------------------------------------------
// HeaderController
//------------------------------------------------------------------------
class HeaderController : public Steinberg::FObject, public VSTGUI::IController
{
public:
    //--------------------------------------------------------------------
    HeaderController(ARADocumentController* controller,
                     Steinberg::Vst::Parameter* param);
    ~HeaderController() override;

    void PLUGIN_API update(FUnknown* changedUnknown,
                           Steinberg::int32 message) override;

    VSTGUI::CView*
    createView(const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;

    VSTGUI::CView*
    verifyView(VSTGUI::CView* view,
               const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;

    // IControlListener
    void valueChanged(VSTGUI::CControl* pControl) override;
    VSTGUI::IController*
    createSubController(VSTGUI::UTF8StringPtr name,
                        const VSTGUI::IUIDescription* description);

    OBJ_METHODS(HeaderController, FObject)

    //--------------------------------------------------------------------
private:
    using StringType = std::string;

    void on_task_count_changed();
    void on_task_count_changed(size_t value, const StringType& value_str);
    void
    on_word_analysis_progress(const meta_words::WordAnalysisProgressData& data);
    void updateSearchResults();

    enum
    {
        kSearchFieldTag = 1000,
    };

    ARADocumentController* controller    = nullptr;
    VSTGUI::CViewContainer* container    = nullptr;
    VSTGUI::CTextLabel* task_count_view  = nullptr;
    SpinnerView* spinner_view            = nullptr;
    VSTGUI::CRowColLayout* rowcol_parent = nullptr;
    tiny_observer_pattern::ObserverID word_analysis_progress_observer_id = 0;
    Steinberg::IPtr<Steinberg::Vst::Parameter> task_count_param;
    VSTGUI::CSearchTextEdit* searchField = nullptr;
    std::string filterString;
};

//------------------------------------------------------------------------
} // namespace mam
