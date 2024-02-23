//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "vstgpt_context.h"
#include "ara_document_controller.h"
#include "meta_words_audio_source.h"
#include "meta_words_editor_view.h"
#include "meta_words_playback_region.h"

namespace mam {

//------------------------------------------------------------------------
VstGPTContext::VstGPTContext(ARADocumentController* document_controller)
: document_controller(document_controller)
{
}

//------------------------------------------------------------------------
void VstGPTContext::registerContextListener(IContextListener* listener)
{
    if (!listeners)
        listeners =
            std::unique_ptr<ContextListenerList>(new ContextListenerList());
    listeners->add(listener);
}

//------------------------------------------------------------------------
void VstGPTContext::unregisterContextListener(IContextListener* listener)
{
    if (listeners)
        listeners->remove(listener);
}

//------------------------------------------------------------------------
void VstGPTContext::onRequestSelectWord(int index)
{
    const auto& meta_words_data = getData();
    const auto& words           = meta_words_data.words;
    const auto& selected_word   = words.at(index);

    if (document_controller)
    {
        document_controller->onRequestLocatorPosChanged(
            selected_word.begin + meta_words_data.project_offset);
    }
}

//------------------------------------------------------------------------
void VstGPTContext::updateListeners()
{
    if (listeners)
    {
        listeners->forEach(
            [this](IContextListener* listener) { listener->onDataChanged(); });
    }
}

//------------------------------------------------------------------------
const VstGPTContext::MetaWordsDataList VstGPTContext::getDataList() const
{
    if (!document_controller)
        return MetaWordsDataList();

    return document_controller->collect_meta_data_words();
}

//------------------------------------------------------------------------
const MetaWordsData VstGPTContext::getData() const
{
    const auto& data_list = getDataList();
    if (data_list.empty())
        return {};

    return data_list.at(0);
}

//------------------------------------------------------------------------
} // namespace mam
