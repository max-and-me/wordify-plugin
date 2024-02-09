//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "vstgpt_context.h"
#include "ara_audio_source.h"
#include "ara_document_controller.h"
#include "meta_words_editor_view.h"
#include "meta_words_playback_region.h"

namespace mam {

//------------------------------------------------------------------------
using Seconds      = const double;
using MilliSeconds = const double;
static MilliSeconds to_milliseconds(Seconds val)
{
    return val / 1000.;
}

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
            to_milliseconds(selected_word.begin) +
            meta_words_data.project_offset);
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
const MetaWordsData VstGPTContext::getData() const
{
    MetaWordsData meta_words_data;

    if (!document_controller)
        return meta_words_data;

    auto views = document_controller->getEditorViews();
    if (views.empty())
        return meta_words_data;

    const auto& view_selection = views.at(0)->getViewSelection();
    const auto& playback_regions =
        view_selection.getPlaybackRegions<meta_words::PlaybackRegion>();

    if (playback_regions.empty())
        return meta_words_data;

    meta_words_data = playback_regions.at(0)->get_meta_words_data();

    return meta_words_data;
}

//------------------------------------------------------------------------
} // namespace mam
