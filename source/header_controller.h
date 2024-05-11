//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ara_document_controller.h"
#include "base/source/fobject.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/uidescription/icontroller.h"

namespace VSTGUI {
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
    HeaderController(ARADocumentController* controller);
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
                        const VSTGUI::IUIDescription* description) override;

    OBJ_METHODS(HeaderController, FObject)

    //--------------------------------------------------------------------
private:
    using StringType = std::string;
    int updateSearchResults(std::string search, int selectIndex);
    int selectPreviousNextSearch(std::string search, int selectIndex);

    enum
    {
        kSearchFieldTag = 1000,
        kSearchNextTag,
        kSearchPreviousTag
    };

    ARADocumentController* controller = nullptr;
    std::string filterString;
    int searchSelectIndex = 0;
};

//------------------------------------------------------------------------
} // namespace mam
