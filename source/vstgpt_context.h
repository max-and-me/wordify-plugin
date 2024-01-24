//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "mam/meta_words/meta_word.h"

namespace mam {

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
private:
    VstGPTContext (){};
    static VstGPTContext* instance;
    Data data;
};

//------------------------------------------------------------------------
} // namespace mam
