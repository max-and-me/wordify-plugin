//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ara_document_controller.h"
#include "base/source/fobject.h"
#include "tiny_observer_pattern.h"
#include "vstgui/uidescription/icontroller.h"

namespace mam {

//------------------------------------------------------------------------
// ListEntryController
//------------------------------------------------------------------------
class ListEntryController : public Steinberg::FObject,
                            public VSTGUI::IController
{
public:
    //--------------------------------------------------------------------
    ListEntryController(
        const VSTGUI::IUIDescription* description,
        ARADocumentController* controller,
        ARADocumentController::FuncSampleRate& playback_sample_rate_func,
        const meta_words::PlaybackRegion::Id playback_region_id);
    ~ListEntryController() override;

    VSTGUI::CView*
    verifyView(VSTGUI::CView* view,
               const VSTGUI::UIAttributes& attributes,
               const VSTGUI::IUIDescription* description) override;
    VSTGUI::IController*
    createSubController(VSTGUI::UTF8StringPtr name,
                        const VSTGUI::IUIDescription* description) override;

    void valueChanged(VSTGUI::CControl* pControl) override {}

    OBJ_METHODS(ListEntryController, FObject)
    //--------------------------------------------------------------------
private:
    const VSTGUI::IUIDescription* description = nullptr;
    ARADocumentController* controller         = nullptr;
    ARADocumentController::FuncSampleRate playback_sample_rate_func;
    const meta_words::PlaybackRegion::Id playback_region_id =
        meta_words::PlaybackRegion::INVALID_ID;
};

//------------------------------------------------------------------------
} // namespace mam
