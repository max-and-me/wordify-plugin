//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ARA_API/ARAVST3.h"
#include "ARA_Library/PlugIn/ARAPlug.h"
#include "public.sdk/source/vst/vstsinglecomponenteffect.h"
#include "vstgui/plugin-bindings/vst3editor.h"
#include <memory>

namespace mam {

//------------------------------------------------------------------------
//  VstGPTProcessor
//------------------------------------------------------------------------
class VstGPTSingleComponent : public Steinberg::Vst::SingleComponentEffect,
                              public ARA::IPlugInEntryPoint,
                              public ARA::IPlugInEntryPoint2,
                              public VSTGUI::VST3EditorDelegate
{
public:
    VstGPTSingleComponent();
    ~VstGPTSingleComponent() SMTG_OVERRIDE;

    // Create function
    static Steinberg::FUnknown* createInstance(void* /*context*/)
    {
        return (Steinberg::Vst::IAudioProcessor*)new VstGPTSingleComponent;
    }

    //-------------------------------------------------------------------------
    // AudioEffect overrides:
    //-------------------------------------------------------------------------
    /** Called at first after constructor */
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context)
        SMTG_OVERRIDE;

    /** Called at the end before destructor */
    Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;

    /** Switch the Plug-in on/off */
    Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool state)
        SMTG_OVERRIDE;

    /** Will be called before any process call */
    Steinberg::tresult PLUGIN_API
    setupProcessing(Steinberg::Vst::ProcessSetup& newSetup) SMTG_OVERRIDE;

    /** Asks if a given sample size is supported see SymbolicSampleSizes. */
    Steinberg::tresult PLUGIN_API
    canProcessSampleSize(Steinberg::int32 symbolicSampleSize) SMTG_OVERRIDE;

    /** Here we go...the process call */
    Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data)
        SMTG_OVERRIDE;

    /** For persistence */
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state)
        SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state)
        SMTG_OVERRIDE;

    //------------------------------------------------------------------------
    // ARA::IPlugInEntryPoint2 overrides:
    //------------------------------------------------------------------------
    /** Get associated ARA factory */
    const ARA::ARAFactory* PLUGIN_API getFactory() SMTG_OVERRIDE;

    /** Bind to ARA document controller instance */
    const ARA::ARAPlugInExtensionInstance* PLUGIN_API bindToDocumentController(
        ARA::ARADocumentControllerRef documentControllerRef) SMTG_OVERRIDE;
    const ARA::ARAPlugInExtensionInstance* PLUGIN_API
    bindToDocumentControllerWithRoles(
        ARA::ARADocumentControllerRef documentControllerRef,
        ARA::ARAPlugInInstanceRoleFlags knownRoles,
        ARA::ARAPlugInInstanceRoleFlags assignedRoles) SMTG_OVERRIDE;

    // VSTGUI::VST3EditorDelegate
    VSTGUI::IController*
    createSubController(VSTGUI::UTF8StringPtr name,
                        const VSTGUI::IUIDescription* description,
                        VSTGUI::VST3Editor* editor) SMTG_OVERRIDE;
    void didOpen(VSTGUI::VST3Editor* editor) override;
    void willClose(VSTGUI::VST3Editor* editor) override;

    // Edit Controller
    Steinberg::IPlugView* PLUGIN_API createView(Steinberg::FIDString name)
        SMTG_OVERRIDE;

    OBJ_METHODS(VstGPTSingleComponent, Steinberg::Vst::SingleComponentEffect)
    DEFINE_INTERFACES
    DEF_INTERFACE(IPlugInEntryPoint)
    DEF_INTERFACE(IPlugInEntryPoint2)
    END_DEFINE_INTERFACES(Steinberg::Vst::SingleComponentEffect)
    REFCOUNT_METHODS(Steinberg::Vst::SingleComponentEffect)
    //------------------------------------------------------------------------
protected:
    ARA::PlugIn::PlugInExtension _araPlugInExtension;
    using VstGPTContextPtr = std::shared_ptr<class VstGPTContext>;
    VstGPTContextPtr context;
};

//------------------------------------------------------------------------
} // namespace mam
