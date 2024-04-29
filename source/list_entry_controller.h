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
    ListEntryController(ARADocumentController* controller,
                        const meta_words::PlaybackRegion::Id id,
                        const VSTGUI::IUIDescription* description);

    ~ListEntryController() override;

    using WordSelectObserver = tiny_observer_pattern::Observer<
        ARADocumentController::WordSelectSubject>;
    using WordSelectObserverPtr = std::unique_ptr<WordSelectObserver>;

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
    void checkSelectWord(const WordSelectData& data);

    const VSTGUI::IUIDescription* description               = nullptr;
    ARADocumentController* controller                       = nullptr;
    const meta_words::PlaybackRegion::Id playback_region_id = 0;

    WordSelectObserverPtr word_selected_observer;
    ARADocumentController::ObserverID word_selected_observer_id = 0;
};

//------------------------------------------------------------------------
} // namespace mam
