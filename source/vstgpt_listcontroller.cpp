//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "vstgpt_listcontroller.h"
#include "vstgui/lib/controls/clistcontrol.h"
#include "vstgui/lib/controls/cstringlist.h"
#include "vstgui/lib/controls/icontrollistener.h"
#include "vstgui/lib/cstring.h"
#include "vstgui/lib/events.h"
#include "vstgui/lib/platform/platformfactory.h"
#include "mam/meta_words/meta_word.h"

//------------------------------------------------------------------------
namespace mam {
using namespace ::VSTGUI;

//------------------------------------------------------------------------
static auto update_list_control_content(CListControl& listControl,
                                        const meta_words::MetaWords& words)
    -> void
{
    listControl.setMax(words.size() - 1);
    listControl.recalculateLayout();

    if (auto stringListDrawer =
            dynamic_cast<StringListControlDrawer*>(listControl.getDrawer()))
    {
        stringListDrawer->setStringProvider([words](int32_t row) {
            const meta_words::MetaWord word = words.at(row);
            const std::string name          = word.word;

            const UTF8String string(name.data());
            return getPlatformFactory().createString(string);
        });
    }
}

//------------------------------------------------------------------------
static void onRequestSelectWord(int index,
                                const mam::MetaWordsData& data,
                                ARADocumentController& document_controller)
{
    const auto& meta_words_data = data;
    const auto& words           = meta_words_data.words;
    const auto& selected_word   = words.at(index);

    const auto new_position =
        selected_word.begin + meta_words_data.project_offset;
    document_controller.onRequestLocatorPosChanged(new_position);
}

//------------------------------------------------------------------------
// VstGPTListController
//------------------------------------------------------------------------
VstGPTListController::VstGPTListController(ARADocumentController& controller)
: controller(controller)
{
    observer_id = controller.add_listener([this]() { this->onDataChanged(); });
}

//------------------------------------------------------------------------
VstGPTListController::~VstGPTListController()
{
    controller.remove_listener(observer_id);
}

//------------------------------------------------------------------------
void VstGPTListController::valueChanged(CControl* pControl)
{
    if (pControl && pControl == listControl)
    {
        if (cached_meta_words_data_list.empty())
            return;

        onRequestSelectWord(listControl->getValue(),
                            cached_meta_words_data_list.at(0), controller);
    }
}

//------------------------------------------------------------------------
CView* VstGPTListController::verifyView(CView* view,
                                        const UIAttributes& /*attributes*/,
                                        const IUIDescription* /*description*/)
{
    if (!listControl)
        listControl = dynamic_cast<CListControl*>(view);

    if (!listControl)
        return view;

    listControl->registerControlListener(this);
    cached_meta_words_data_list = controller.collect_meta_data_words();
    if (!cached_meta_words_data_list.empty())
    {
        update_list_control_content(*listControl,
                                    cached_meta_words_data_list.at(0).words);
    }

    return view;
}

//------------------------------------------------------------------------
void VstGPTListController::onDataChanged()
{
    if (!listControl)
        return;

    cached_meta_words_data_list = controller.collect_meta_data_words();
    if (!cached_meta_words_data_list.empty())
    {
        update_list_control_content(*listControl,
                                    cached_meta_words_data_list.at(0).words);
        listControl->setDirty();
    }
}
//------------------------------------------------------------------------

} // namespace mam
