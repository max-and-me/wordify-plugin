//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ARA_Library/PlugIn/ARAPlug.h"
#include "eventpp/callbacklist.h"
#include "meta_words_data.h"
#include "meta_words_playback_region.h"
#include "region_order_manager.h"
#include "search_engine.h"
#include "string_matcher.h"
#include "tiny_selection_model.h"

namespace mam {
namespace meta_words {
class AudioSource;
class AudioModification;
class PlaybackRegion;
class PlaybackRenderer;
} // namespace meta_words

//------------------------------------------------------------------------
// RegionLifetimeEventData
//------------------------------------------------------------------------
struct RegionLifetimeEventData
{
    using PlaybackRegion = meta_words::PlaybackRegion;

    enum class Event
    {
        HasBeenAdded,
        WillBeRemoved,
    };
    Event event;
    PlaybackRegion::Id id{0};
};

//------------------------------------------------------------------------
// SelectedWordEventData
//------------------------------------------------------------------------
struct SelectedWordEventData
{
    using RegionId  = size_t;
    using WordIndex = size_t;

    RegionId region_id;
    WordIndex word_index;
};

using RegionSelectionModel = SelectionModel<SelectedWordEventData>;

//------------------------------------------------------------------------
using SelectedWordCallback =
    eventpp::CallbackList<void(const SelectedWordEventData&)>;
using RegionPropertiesChangedCallback = eventpp::CallbackList<void(void)>;
using SearchEngineCallback =
    eventpp::CallbackList<void(const search_engine::SearchResults&)>;
using RegionLifetimeCallback =
    eventpp::CallbackList<void(const RegionLifetimeEventData&)>;
using RegionsOrderCallback = RegionOrderManager::OrderSubject;

//------------------------------------------------------------------------
// ARADocumentController
//------------------------------------------------------------------------
class ARADocumentController : public ARA::PlugIn::DocumentController
{
public:
    //--------------------------------------------------------------------
    using AudioModification    = meta_words::AudioModification;
    using AudioSource          = meta_words::AudioSource;
    using MetaWordsDataList    = std::vector<MetaWordsData>;
    using OptPlaybackRegionPtr = meta_words::OptPlaybackRegionPtr;
    using PlaybackRegion       = meta_words::PlaybackRegion;
    using PlaybackRenderer     = meta_words::PlaybackRenderer;

    using SampleRate     = double;
    using FuncSampleRate = std::function<SampleRate()>;

    // Containers
    using PlaybackRegionObservers =
        std::unordered_map<PlaybackRegion::Id, RegionPropertiesChangedCallback>;
    using PlaybackRegions = std::map<PlaybackRegion::Id, PlaybackRegion*>;

    // publish inherited constructor
    using ARA::PlugIn::DocumentController::DocumentController;
    using Super = ARA::PlugIn::DocumentController;

    // getter for the companion API implementations
    static const ARA::ARAFactory* getARAFactory() noexcept;

    explicit ARADocumentController(
        const ARA::PlugIn::PlugInEntry* entry,
        const ARA::ARADocumentControllerHostInstance* instance) noexcept;

    bool doRestoreObjectsFromArchive(
        ARA::PlugIn::HostArchiveReader* archiveReader,
        const ARA::PlugIn::RestoreObjectsFilter* filter) noexcept override;
    bool doStoreObjectsToArchive(
        ARA::PlugIn::HostArchiveWriter* archiveWriter,
        const ARA::PlugIn::StoreObjectsFilter* filter) noexcept override;
    void doUpdateMusicalContextContent(
        ARA::PlugIn::MusicalContext* musicalContext,
        const ARA::ARAContentTimeRange* range,
        ARA::ContentUpdateScopes scopeFlags) noexcept override;
    void doUpdateAudioSourceContent(
        ARA::PlugIn::AudioSource* audioSource,
        const ARA::ARAContentTimeRange* range,
        ARA::ContentUpdateScopes scopeFlags) noexcept override;
    void
    willEnableAudioSourceSamplesAccess(ARA::PlugIn::AudioSource* audioSource,
                                       bool enable) noexcept override;
    void
    didEnableAudioSourceSamplesAccess(ARA::PlugIn::AudioSource* audioSource,
                                      bool enable) noexcept override;
    ARA::PlugIn::AudioSource*
    doCreateAudioSource(ARA::PlugIn::Document* document,
                        ARA::ARAAudioSourceHostRef hostRef) noexcept override;
    void didUpdateAudioSourceProperties(
        ARA::PlugIn::AudioSource* audioSource) noexcept override;

    ARA::PlugIn::AudioModification* doCreateAudioModification(
        ARA::PlugIn::AudioSource* audioSource,
        ARA::ARAAudioModificationHostRef hostRef,
        const ARA::PlugIn::AudioModification*
            optionalModificationToClone) noexcept override;

    void didUpdateAudioModificationProperties(
        ARA::PlugIn::AudioModification* audioModification) noexcept override;

    ARA::PlugIn::PlaybackRegion* doCreatePlaybackRegion(
        ARA::PlugIn::AudioModification* modification,
        ARA::ARAPlaybackRegionHostRef hostRef) noexcept override;

    void doDestroyPlaybackRegion(
        ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;

    void didUpdatePlaybackRegionProperties(
        ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;

    ARA::PlugIn::EditorView* doCreateEditorView() noexcept override;
    ARA::PlugIn::EditorRenderer* doCreateEditorRenderer() noexcept override;

    void onRequestLocatorPosChanged(double pos);

    ARA::PlugIn::PlaybackRenderer* doCreatePlaybackRenderer() noexcept override;

    void didAddPlaybackRegionToRegionSequence(
        ARA::PlugIn::RegionSequence* regionSequence,
        ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;

    void willRemovePlaybackRegionFromRegionSequence(
        ARA::PlugIn::RegionSequence* regionSequence,
        ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;

    // Render thread synchronization:
    // This is just a test code implementation of handling the threading -
    // proper code will use a more sophisticated threading implementation, which
    // is needed regardless of ARA. The test code simply blocks renderer access
    // to the model while it is being modified. This includes waiting until
    // concurrent renderer model access has completed before starting
    // modifications.
    bool
    rendererWillAccessModelGraph(PlaybackRenderer* playbackRenderer) noexcept;
    void
    rendererDidAccessModelGraph(PlaybackRenderer* playbackRenderer) noexcept;

    auto
    find_playback_region(PlaybackRegion::Id id) const -> OptPlaybackRegionPtr;

    // Search Engine
    auto search_word(std::string search) -> void;
    auto clear_search_results() -> void;
    auto focus_next_occurence() -> void;
    auto focus_prev_occurence() -> void;

    auto get_selected_word_subject() -> SearchEngineCallback*
    {
        return &search_engine_subject;
    }

    auto activate_smart_search(bool activate) -> void;
    // Search Engine

    template <typename Func>
    void for_each_playback_region_id(Func&& func)
    {
        auto tmp_func = [func](size_t /*index*/, const PlaybackRegion::Id id) {
            func(id);
        };
        region_order_manager.for_each_playback_region_id_enumerated(tmp_func);
    }

    auto get_playback_region_changed_subject(
        const PlaybackRegion::Id playback_region_id)
        -> RegionPropertiesChangedCallback&
    {
        return playback_region_observers[playback_region_id];
    }

    auto get_playback_region_order_subject() -> RegionsOrderCallback*
    {
        return region_order_manager.get_order_subject();
    }

    auto get_playback_region_lifetimes_subject() -> RegionLifetimeCallback*
    {
        return &playback_region_lifetimes_subject;
    }

    auto get_region_selection_subject() -> SelectedWordCallback*
    {
        return &selected_word_callback;
    }

    template <typename Func>
    auto for_each_playback_region_id_enumerated(Func& func) const -> void
    {
        region_order_manager.for_each_playback_region_id_enumerated(func);
    }

    auto get_region_selection_model() -> RegionSelectionModel&;

    auto onRequestSelectWord(int index,
                             const meta_words::PlaybackRegion::Id id) -> void;

    auto selectStringMatchMethod(StringMatcher::MatchMethod matchMethod) -> void
    {
        string_match_method = matchMethod;
    }

    //--------------------------------------------------------------------
private:
    PlaybackRegionObservers playback_region_observers;
    RegionLifetimeCallback playback_region_lifetimes_subject;
    SearchEngineCallback search_engine_subject;
    SelectedWordCallback selected_word_callback;
    RegionOrderManager region_order_manager;
    RegionSelectionModel region_selection_model;

    PlaybackRegions playback_regions;

    std::atomic<bool> _renderersCanAccessModelGraph{true};
    std::atomic<int> _countOfRenderersCurrentlyAccessingModelGraph{0};

    void on_add_playback_region(PlaybackRegion* region);
    void on_remove_playback_region(PlaybackRegion::Id id);
    void on_analyze_audio_source_progress(
        const meta_words::WordAnalysisProgressData& data);

    template <typename Func>
    void for_each_playback_region(Func&& func)
    {
        ARADocumentController::MetaWordsDataList meta_words_data_list;
        if (auto* document = getDocument())
        {
            const auto& sequences = document->getRegionSequences();
            for (const auto sequence : sequences)
            {
                const auto& regions =
                    sequence->getPlaybackRegions<PlaybackRegion>();
                for (const auto* region : regions)
                {
                    func(region);
                }
            }
        }
    }

    StringMatcher::MatchMethod string_match_method =
        StringMatcher::MatchMethod::nearbyFuzzyMatch;
};

//------------------------------------------------------------------------
} // namespace mam
