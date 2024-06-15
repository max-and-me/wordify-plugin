//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
//------------------------------------------------------------------------

#include "meta_words_serde.h"
#include "nlohmann/json.hpp"

namespace mam::meta_words {

//------------------------------------------------------------------------
using JsonString = std::string;
using json       = nlohmann::json;
void to_json(json& j, const MetaWord& m)
{
    j = json{{"word", m.value}, {"begin", m.begin}, {"duration", m.duration}};
}

//------------------------------------------------------------------------
void from_json(const json& j, MetaWord& m)
{
    j.at("word").get_to(m.value);
    j.at("begin").get_to(m.begin);
    j.at("duration").get_to(m.duration);
}

//------------------------------------------------------------------------

namespace serde {

//------------------------------------------------------------------------
void to_json(json& j, const AudioSource& mws)
{
    j = json{{"persistent_id", mws.persistent_id}, {"words", mws.words}};
}

//------------------------------------------------------------------------
void from_json(const json& j, AudioSource& mws)
{
    j.at("persistent_id").get_to(mws.persistent_id);
    j.at("words").get_to(mws.words);
}

//------------------------------------------------------------------------
void to_json(json& j, const Archive& archive)
{
    j = json{{"archive_version", archive.version},
             {"audio_sources", archive.audio_sources}};
}

//------------------------------------------------------------------------
void from_json(const json& j, Archive& archive)
{
    j.at("archive_version").get_to(archive.version);
    j.at("audio_sources").get_to(archive.audio_sources);
}

//------------------------------------------------------------------------
auto serialize(const Archive& archive, JsonString& s) -> bool
{
    json j = archive;
    s      = j.dump();

    return true;
}
//------------------------------------------------------------------------
auto deserialize(const JsonString& s, Archive& archive) -> bool
{
    if (s.empty())
        return false;

    json j  = json::parse(s);
    archive = j.template get<Archive>();

    return true;
}
//------------------------------------------------------------------------
} // namespace serde
} // namespace mam::meta_words
