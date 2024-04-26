//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ARA_Library/PlugIn/ARAPlug.h"
#include "meta_words_data.h"
#include "meta_words_playback_region.h"
#include "region_order_manager.h"
#include "tiny_observer_pattern.h"
#include "tiny_selection_model.h"

namespace mam {
namespace meta_words {
class AudioSource;
class AudioModification;
class PlaybackRegion;
class PlaybackRenderer;
} // namespace meta_words

//------------------------------------------------------------------------
// ARADocumentController
//------------------------------------------------------------------------
struct PlaybackRegionLifetimeData
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
//  RegionData
//------------------------------------------------------------------------
struct RegionData
{
    using RegionId  = size_t;
    using WordIndex = size_t;

    RegionId region_id;
    WordIndex word_index;
};

//------------------------------------------------------------------------

using RegionSelectionModel = SelectionModel<RegionData>;

//------------------------------------------------------------------------
//  WordAnalysisProgressData
//------------------------------------------------------------------------
struct WordAnalysisProgressData
{
    enum class State
    {
        kAnalysisStarted,
        kAnalysisRunning,
        kAnalysisStopped,
    };

    meta_words::AudioSource::Identifier identifier;
    double progress_val = 0.;
    State state;
};

//------------------------------------------------------------------------
// ARADocumentController
//------------------------------------------------------------------------
class ARADocumentController : public ARA::PlugIn::DocumentController,
                              public tiny_observer_pattern::SimpleSubject
{
public:
    //--------------------------------------------------------------------
    using AudioModification    = meta_words::AudioModification;
    using AudioSource          = meta_words::AudioSource;
    using MetaWordsDataList    = std::vector<MetaWordsData>;
    using OptPlaybackRegionPtr = meta_words::OptPlaybackRegionPtr;
    using PlaybackRegion       = meta_words::PlaybackRegion;
    using PlaybackRenderer     = meta_words::PlaybackRenderer;
    using Subject              = tiny_observer_pattern::SimpleSubject;
    using ObserverID           = tiny_observer_pattern::ObserverID;

    using SampleRate     = double;
    using FuncSampleRate = std::function<SampleRate()>;

    // Containers
    using PlaybackRegionObservers =
        std::unordered_map<PlaybackRegion::Id, Subject>;
    using PlaybackRegions =
        std::unordered_map<PlaybackRegion::Id, PlaybackRegion*>;

    // Subjects
    using PlaybackRegionLifetimesSubject =
        tiny_observer_pattern::Subject<PlaybackRegionLifetimeData>;
    using PlaybackRegionsOrderSubject = RegionOrderManager::OrderSubject;
    using AnalysisProgressSubject =
        tiny_observer_pattern::Subject<WordAnalysisProgressData>;
    using ColorSchemeSubject = tiny_observer_pattern::SimpleSubject;

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

    template <typename Func>
    void for_each_playback_region_id(Func&& func)
    {
        auto tmp_func = [func](size_t /*index*/, const PlaybackRegion::Id id) {
            func(id);
        };
        region_order_manager.for_each_playback_region_id_enumerated(tmp_func);
    }

    auto get_playback_region_changed_subject(
        const PlaybackRegion::Id playback_region_id) -> Subject&
    {
        return playback_region_observers[playback_region_id];
    }

    auto get_playback_region_order_subject() -> PlaybackRegionsOrderSubject*
    {
        return region_order_manager.get_order_subject();
    }

    auto
    get_playback_region_lifetimes_subject() -> PlaybackRegionLifetimesSubject*
    {
        return &playback_region_lifetimes_subject;
    }

    auto get_color_scheme_subject() -> ColorSchemeSubject*
    {
        return &color_scheme_subject;
    }

    auto register_word_analysis_progress_observer(
        AnalysisProgressSubject::Callback&& callback) -> ObserverID;

    auto unregister_word_analysis_progress_observer(ObserverID id) -> bool;

    template <typename Func>
    auto for_each_playback_region_id_enumerated(Func& func) const -> void
    {
        region_order_manager.for_each_playback_region_id_enumerated(func);
    }

    auto get_region_selection_model() -> RegionSelectionModel&;
    auto is_dark_scheme() const -> bool { return dark_scheme; }
    auto set_dark_scheme(bool state) -> void
    {
        dark_scheme = state;
        color_scheme_subject.notify_listeners({});
    }

    //--------------------------------------------------------------------
protected:
    PlaybackRegionObservers playback_region_observers;
    PlaybackRegions playback_regions;
    PlaybackRegionLifetimesSubject playback_region_lifetimes_subject;
    AnalysisProgressSubject word_analysis_progress_subject;
    ColorSchemeSubject color_scheme_subject;
    RegionOrderManager region_order_manager;
    RegionSelectionModel region_selection_model;

    std::atomic<bool> _renderersCanAccessModelGraph{true};
    std::atomic<int> _countOfRenderersCurrentlyAccessingModelGraph{0};

private:
    void on_add_playback_region(PlaybackRegion* region);
    void on_remove_playback_region(PlaybackRegion::Id id);
    void on_word_analysis_progress(const AudioSource& source, bool state);
    void on_word_analysis_progress(const AudioSource& source, double progress);

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

    auto register_playback_region_changed_observer(
        const PlaybackRegion::Id playback_region_id,
        Subject::Callback&& callback) -> ObserverID;

    auto unregister_playback_region_changed_observer(
        const PlaybackRegion::Id playback_region_id, ObserverID id);

    bool dark_scheme = true;
};

//------------------------------------------------------------------------
} // namespace mam
