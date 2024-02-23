//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "meta_words_data.h"
#include "vstgui/lib/dispatchlist.h"

#include <memory>
namespace mam {

class ARADocumentController;

//------------------------------------------------------------------------
class IContextListener
{
public:
    /** the context has changed  */
    virtual void onDataChanged() = 0;
};

//------------------------------------------------------------------------
class VstGPTContext
{
public:
    VstGPTContext(const VstGPTContext& obj) = delete;
    VstGPTContext(ARADocumentController* document_controller);

    using MetaWordsDataList = std::vector<MetaWordsData>;

    const MetaWordsDataList getDataList() const;
    const MetaWordsData getData() const;
    void onRequestSelectWord(int index);
    void registerContextListener(IContextListener* listener);
    void unregisterContextListener(IContextListener* listener);

private:
    VstGPTContext(){};

    using ContextListenerList = VSTGUI::DispatchList<IContextListener*>;
    std::unique_ptr<ContextListenerList> listeners;
    void updateListeners();

    ARADocumentController* document_controller = nullptr;
};

//------------------------------------------------------------------------
} // namespace mam
