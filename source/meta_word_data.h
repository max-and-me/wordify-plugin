//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#include "mam/meta_words/meta_word.h"
#include <vector>

namespace mam {
//------------------------------------------------------------------------
struct MetaWordData
{
    bool is_audible = true;
    meta_words::MetaWord word;
};

//------------------------------------------------------------------------
using MetaWordData = std::vector<MetaWordData>;
//------------------------------------------------------------------------

} // namespace mam