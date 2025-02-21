//------------------------------------------------------------------------
// Copyright (c) 2023-present, WordifyOrg.
//------------------------------------------------------------------------

#pragma once

#include "warn_cpp/suppress_warnings.h"
#include "task_manager.h"
BEGIN_SUPPRESS_WARNINGS
#include "ARA_API/ARAVST3.h"
#include "ARA_Library/PlugIn/ARAPlug.h"
#include "ipslviewembedding.h"
#include "public.sdk/source/vst/vstsinglecomponenteffect.h"
#include "vstgui/plugin-bindings/vst3editor.h"
END_SUPPRESS_WARNINGS

namespace mam {

//------------------------------------------------------------------------
//  WordifySingleComponent
//------------------------------------------------------------------------
class WordifySingleComponent : public Steinberg::Vst::SingleComponentEffect,
                               public ARA::IPlugInEntryPoint,
                               public ARA::IPlugInEntryPoint2,
                               public VSTGUI::VST3EditorDelegate,
                               public Presonus::IPlugInViewEmbedding
{
public:
    //--------------------------------------------------------------------

    using Editors = std::vector<Steinberg::Vst::EditorView*>;

    WordifySingleComponent();
    ~WordifySingleComponent() override;

    // Create function
    static Steinberg::FUnknown* createInstance(void* /*context*/)
    {
        return (Steinberg::Vst::IAudioProcessor*)new WordifySingleComponent;
    }

    // AudioEffect overrides:
    /** Called at first after constructor */
    Steinberg::tresult PLUGIN_API
    initialize(Steinberg::FUnknown* context) override;

    /** Called at the end before destructor */
    Steinberg::tresult PLUGIN_API terminate() override;

    /** Switch the Plug-in on/off */
    Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool state) override;

    /** Will be called before any process call */
    Steinberg::tresult PLUGIN_API
    setupProcessing(Steinberg::Vst::ProcessSetup& newSetup) override;

    /** Asks if a given sample size is supported see SymbolicSampleSizes. */
    Steinberg::tresult PLUGIN_API
    canProcessSampleSize(Steinberg::int32 symbolicSampleSize) override;

    /** Here we go...the process call */
    Steinberg::tresult PLUGIN_API
    process(Steinberg::Vst::ProcessData& data) override;

    /** For persistence */
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) override;
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) override;

    // ARA::IPlugInEntryPoint2 overrides:
    /** Get associated ARA factory */
    const ARA::ARAFactory* PLUGIN_API getFactory() override;

    /** Bind to ARA document controller instance */
    const ARA::ARAPlugInExtensionInstance* PLUGIN_API bindToDocumentController(
        ARA::ARADocumentControllerRef documentControllerRef) override;
    const ARA::ARAPlugInExtensionInstance* PLUGIN_API
    bindToDocumentControllerWithRoles(
        ARA::ARADocumentControllerRef documentControllerRef,
        ARA::ARAPlugInInstanceRoleFlags knownRoles,
        ARA::ARAPlugInInstanceRoleFlags assignedRoles) override;

    // VSTGUI::VST3EditorDelegate
    VSTGUI::IController*
    createSubController(VSTGUI::UTF8StringPtr name,
                        const VSTGUI::IUIDescription* description,
                        VSTGUI::VST3Editor* editor) override;
    void didOpen(VSTGUI::VST3Editor* editor) override;
    void willClose(VSTGUI::VST3Editor* editor) override;

    // Edit Controller
    Steinberg::IPlugView* PLUGIN_API
    createView(Steinberg::FIDString name) override;
    void PLUGIN_API editorAttached(Steinberg::Vst::EditorView* editor) override;
    void PLUGIN_API editorRemoved(Steinberg::Vst::EditorView* editor) override;
    void PLUGIN_API update(Steinberg::FUnknown* changedUnknown,
                           Steinberg::int32 tag) override;

    // Presonus
    Steinberg::TBool PLUGIN_API isViewEmbeddingSupported() override;
    Steinberg::tresult PLUGIN_API setViewIsEmbedded(
        Steinberg::IPlugView* view, Steinberg::TBool embedded) override;

    OBJ_METHODS(WordifySingleComponent, Steinberg::Vst::SingleComponentEffect)
    DEFINE_INTERFACES
    DEF_INTERFACE(IPlugInEntryPoint)
    DEF_INTERFACE(IPlugInEntryPoint2)
    DEF_INTERFACE(IPlugInViewEmbedding)
    END_DEFINE_INTERFACES(Steinberg::Vst::SingleComponentEffect)
    REFCOUNT_METHODS(Steinberg::Vst::SingleComponentEffect)

    //--------------------------------------------------------------------
protected:
    ARA::PlugIn::PlugInExtension araPlugInExtension;
    Editors editors;

    auto restore_parameters() -> void;
    auto store_parameters() -> void;

    bool dark_scheme = false;
    task_managing::TaskCountCallback::Handle task_count_handle;
};

//------------------------------------------------------------------------
} // namespace mam
