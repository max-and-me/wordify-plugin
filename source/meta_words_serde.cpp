//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "meta_words_serde.h"
#include "nlohmann/json.hpp"

namespace mam::meta_words {

//------------------------------------------------------------------------
using JsonString = std::string;
using json       = nlohmann::json;
void to_json(json& j, const MetaWord& m)
{
    j = json{{"word", m.word}, {"begin", m.begin}, {"duration", m.duration}};
}

//------------------------------------------------------------------------
void from_json(const json& j, MetaWord& m)
{
    j.at("word").get_to(m.word);
    j.at("begin").get_to(m.begin);
    j.at("duration").get_to(m.duration);
}

//------------------------------------------------------------------------

namespace serde {

//------------------------------------------------------------------------
void to_json(json& j, const MetaWordsSerde& mws)
{
    j = json{{"persistent_id", mws.persistent_id}, {"words", mws.words}};
}

//------------------------------------------------------------------------
void from_json(const json& j, MetaWordsSerde& mws)
{
    j.at("persistent_id").get_to(mws.persistent_id);
    j.at("words").get_to(mws.words);
}

//------------------------------------------------------------------------
auto serialize(const MetaWordsSerdeDataset& meta_words_serde_dataset,
               JsonString& s) -> bool
{
    json j;

    for (const auto& el : meta_words_serde_dataset)
        j.push_back(el);

    s = j.dump();

    return true;
}
//------------------------------------------------------------------------
auto deserialize(const JsonString& s,
                 MetaWordsSerdeDataset& meta_words_serde_dataset) -> bool
{
    json j = json::parse(s);

    for (auto& el : j)
    {
        auto ds = el.template get<MetaWordsSerde>();
        meta_words_serde_dataset.emplace_back(ds);
    }

    return true;
}
//------------------------------------------------------------------------
} // namespace serde
} // namespace mam::meta_words