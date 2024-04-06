//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "meta_words_clip_controller.h"
#include "ara_document_controller.h"
#include "list_entry_controller.h"
#include "meta_words_data.h"
#include "vstgui/lib/controls/cstringlist.h"
#include "vstgui/lib/controls/ctextlabel.h"
#include "vstgui/lib/platform/platformfactory.h"

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
static auto update_label_control(CTextLabel& label, const MetaWordsData& data)
    -> void
{
    auto [r, g, b] = data.color;
    const VSTGUI::CColor color(r, g, b);
    label.setFontColor(color);
    label.setText(VSTGUI::UTF8String(data.name));
}

//------------------------------------------------------------------------
// VstGPTWaveClipListController
//------------------------------------------------------------------------
MetaWordsClipController::MetaWordsClipController() {}

//------------------------------------------------------------------------
MetaWordsClipController::~MetaWordsClipController()
{
    if (subject)
        subject->remove_listener(observer_id);
}

//------------------------------------------------------------------------
bool MetaWordsClipController::initialize(
    Subject* subject, FuncMetaWordsData&& meta_words_data_func)
{
    if (!subject)
        return false;

    if (this->subject)
    {
        this->subject->remove_listener(observer_id);
    }

    this->subject = subject;
    this->meta_words_data_func = std::move(meta_words_data_func);

    observer_id = this->subject->add_listener(
        [this](const auto&) { this->onDataChanged(); });

    onDataChanged();

    return true;
}

//------------------------------------------------------------------------
void MetaWordsClipController::onDataChanged()
{
    const auto& data = meta_words_data_func();
    if (listControl)
    {
        update_list_control_content(*listControl, data.words);
        listControl->setDirty();
    }

    if (label)
    {
        update_label_control(*label, data);
        label->setDirty();
    }
}

//------------------------------------------------------------------------
VSTGUI::CView*
MetaWordsClipController::verifyView(VSTGUI::CView* view,
                                    const VSTGUI::UIAttributes& attributes,
                                    const VSTGUI::IUIDescription* description)
{

    if (!listControl)
    {
        if (listControl = dynamic_cast<CListControl*>(view))
        {
            listControl->registerControlListener(this);
            update_list_control_content(*listControl,
                                        meta_words_data_func().words);
        }
    }
    if (!label)
    {
        if (label = dynamic_cast<CTextLabel*>(view))
            update_label_control(*label, meta_words_data_func());
    }

    return view;
};

//------------------------------------------------------------------------
void MetaWordsClipController::valueChanged(VSTGUI::CControl* pControl)
{
    if (pControl && pControl == listControl)
    {
        if (list_value_changed_func)
            list_value_changed_func(listControl->getValue());
    }
}

//------------------------------------------------------------------------
auto MetaWordsClipController::set_list_clicked_func(
    const FuncListValueChanged&& list_value_changed_func) -> void
{
    this->list_value_changed_func = list_value_changed_func;
}

//------------------------------------------------------------------------
} // namespace mam