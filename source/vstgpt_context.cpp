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
using Seconds      = const double;
using MilliSeconds = const double;
static MilliSeconds to_milliseconds(Seconds val)
{
    return val / 1000.;
}

//------------------------------------------------------------------------
void VstGPTContext::onRequestSelectWord(int index)
{
    if (listeners)
    {
        meta_words::MetaWord word = data.words.at(index);
        listeners->forEach([this, word](IContextListener* listener) {
            MilliSeconds pos = to_milliseconds(word.begin);
            listener->onRequestLocatorPosChanged(pos);
        });
    }
}

} // namespace mam
