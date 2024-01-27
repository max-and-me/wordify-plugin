//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "mam/meta_words/meta_word.h"
#include "vstgui/lib/dispatchlist.h"

#include <memory>
namespace mam {

class IContextListener
{
public:
    /** the context has changed  */
    virtual void onRequestLocatorPosChanged (double pos) = 0;
};

//------------------------------------------------------------------------
class ContextListenerAdapter : public IContextListener
{
public:
    void onRequestLocatorPosChanged (double pos) override {}
};

class VstGPTContext
{
public:
    VstGPTContext(const VstGPTContext& obj) = delete;
    static VstGPTContext* getInstance ()
    {
        if (instance == nullptr)
            instance = new VstGPTContext;
        return instance;
    }
    struct Data
    {
        meta_words::MetaWords words;
    };
    void setData(Data _data){data = _data;}
    const Data& getData() {return data;}
    void onRequestSelectWord (int index);
    void registerContextListener (IContextListener* listener);
    void unregisterContextListener (IContextListener* listener);
    
private:
    VstGPTContext (){};
    using ContextListenerList = VSTGUI::DispatchList<IContextListener*>;
    std::unique_ptr<ContextListenerList> listeners;
    static VstGPTContext* instance;
    Data data;
};

//------------------------------------------------------------------------
} // namespace mam
