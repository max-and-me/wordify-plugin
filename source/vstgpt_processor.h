//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#pragma once

#include "ARA_API/ARAVST3.h"
#include "ARA_Library/PlugIn/ARAPlug.h"
#include "public.sdk/source/vst/vstaudioeffect.h"

namespace mam {

//------------------------------------------------------------------------
//  VstGPTProcessor
//------------------------------------------------------------------------
class VstGPTProcessor : public Steinberg::Vst::AudioEffect,
                        public ARA::IPlugInEntryPoint,
                        public ARA::IPlugInEntryPoint2
{
public:
    VstGPTProcessor();
    ~VstGPTProcessor() SMTG_OVERRIDE;

    // Create function
    static Steinberg::FUnknown* createInstance(void* /*context*/)
    {
        return (Steinberg::Vst::IAudioProcessor*)new VstGPTProcessor;
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

    OBJ_METHODS(VstGPTProcessor, AudioEffect)
    DEFINE_INTERFACES
    DEF_INTERFACE(IPlugInEntryPoint)
    DEF_INTERFACE(IPlugInEntryPoint2)
    END_DEFINE_INTERFACES(AudioEffect)
    REFCOUNT_METHODS(AudioEffect)
    //------------------------------------------------------------------------
protected:
    ARA::PlugIn::PlugInExtension _araPlugInExtension;
};

//------------------------------------------------------------------------
} // namespace mam
