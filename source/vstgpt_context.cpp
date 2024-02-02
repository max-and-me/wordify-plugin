//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "vstgpt_context.h"

namespace mam {

VstGPTContext* VstGPTContext::instance = nullptr;

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
    if (listeners)
    {
        meta_words::MetaWord word = data.words.at(index);
        listeners->forEach([this, word](IContextListener* listener) {
            listener->onRequestLocatorPosChanged(word.begin / 10000);
        });
    }
}

} // namespace mam
