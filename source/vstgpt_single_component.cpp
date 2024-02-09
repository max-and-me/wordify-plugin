//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#include "vstgpt_single_component.h"
#include "ara_document_controller.h"
#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "vstgpt_cids.h"
#include "vstgpt_listcontroller.h"

using namespace Steinberg;

namespace mam {
//------------------------------------------------------------------------
// VstGPTProcessor
//------------------------------------------------------------------------
VstGPTSingleComponent::VstGPTSingleComponent() {}

//------------------------------------------------------------------------
VstGPTSingleComponent::~VstGPTSingleComponent() {}

//------------------------------------------------------------------------
tresult PLUGIN_API VstGPTSingleComponent::initialize(FUnknown* context)
{
    // Here the Plug-in will be instantiated

    //---always initialize the parent-------
    tresult result = SingleComponentEffect::initialize(context);
    // if everything Ok, continue
    if (result != kResultOk)
    {
        return result;
    }

    //--- create Audio IO ------
    addAudioInput(STR16("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
    addAudioOutput(STR16("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

    /* If you don't need an event bus, you can remove the next line */
    addEventInput(STR16("Event In"), 1);

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstGPTSingleComponent::terminate()
{
    // Here the Plug-in will be de-instantiated, last possibility to remove some
    // memory!

    //---do not forget to call parent ------
    return SingleComponentEffect::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstGPTSingleComponent::setActive(TBool state)
{
    //--- called when the Plug-in is enable/disable (On/Off) -----
    return SingleComponentEffect::setActive(state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstGPTSingleComponent::process(Vst::ProcessData& data)
{
    //--- First : Read inputs parameter changes-----------

    /*if (data.inputParameterChanges)
    {
        int32 numParamsChanged = data.inputParameterChanges->getParameterCount
    (); for (int32 index = 0; index < numParamsChanged; index++)
        {
            if (auto* paramQueue = data.inputParameterChanges->getParameterData
    (index))
            {
                Vst::ParamValue value;
                int32 sampleOffset;
                int32 numPoints = paramQueue->getPointCount ();
                switch (paramQueue->getParameterId ())
                {
                }
            }
        }
    }*/

    //--- Here you have to implement your processing

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API
VstGPTSingleComponent::setupProcessing(Vst::ProcessSetup& newSetup)
{
    //--- called before any processing ----
    return SingleComponentEffect::setupProcessing(newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API
VstGPTSingleComponent::canProcessSampleSize(int32 symbolicSampleSize)
{
    // by default kSample32 is supported
    if (symbolicSampleSize == Vst::kSample32)
        return kResultTrue;

    // disable the following comment if your processing support kSample64
    /* if (symbolicSampleSize == Vst::kSample64)
        return kResultTrue; */

    return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstGPTSingleComponent::setState(IBStream* state)
{
    // called when we load a preset, the model has to be reloaded
    IBStreamer streamer(state, kLittleEndian);

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstGPTSingleComponent::getState(IBStream* state)
{
    // here we need to save the model
    IBStreamer streamer(state, kLittleEndian);

    return kResultOk;
}

//------------------------------------------------------------------------
VSTGUI::IController* VstGPTSingleComponent::createSubController(
    VSTGUI::UTF8StringPtr name,
    const VSTGUI::IUIDescription* /*description*/,
    VSTGUI::VST3Editor* /*editor*/)
{
    if (VSTGUI::UTF8StringView(name) == "MetaWordsListController")
        return new VstGPTListController(VstGPTContext::getInstance());

    return nullptr;
}

//------------------------------------------------------------------------
void VstGPTSingleComponent::didOpen(VSTGUI::VST3Editor* editor)
{
    // Must be set to 'true' to get notified by a host selection change
    if (_araPlugInExtension.getEditorView())
        _araPlugInExtension.getEditorView()->setEditorOpen(true);
}

//------------------------------------------------------------------------
void VstGPTSingleComponent::willClose(VSTGUI::VST3Editor* editor)
{
    if (_araPlugInExtension.getEditorView())
        _araPlugInExtension.getEditorView()->setEditorOpen(false);
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API VstGPTSingleComponent::createView(FIDString name)
{
    // Here the Host wants to open your editor (if you have one)
    if (FIDStringsEqual(name, Vst::ViewType::kEditor))
    {
        // create your editor here and return a IPlugView ptr of it
        auto* view =
            new VSTGUI::VST3Editor(this, "view", "vstgpt_editor.uidesc");
        return view;
    }
    return nullptr;
}

//-----------------------------------------------------------------------------
const ARA::ARAPlugInExtensionInstance* PLUGIN_API
VstGPTSingleComponent::bindToDocumentController(
    ARA::ARADocumentControllerRef /*documentControllerRef*/)
{
    ARA_VALIDATE_API_STATE(
        false && "call is deprecated in ARA 2, host must not call this");
    return nullptr;
}

//-----------------------------------------------------------------------------
const ARA::ARAPlugInExtensionInstance* PLUGIN_API
VstGPTSingleComponent::bindToDocumentControllerWithRoles(
    ARA::ARADocumentControllerRef documentControllerRef,
    ARA::ARAPlugInInstanceRoleFlags knownRoles,
    ARA::ARAPlugInInstanceRoleFlags assignedRoles)
{
    return _araPlugInExtension.bindToARA(documentControllerRef, knownRoles,
                                         assignedRoles);
}

//-----------------------------------------------------------------------------
const ARA::ARAFactory* PLUGIN_API VstGPTSingleComponent::getFactory()
{
    return ARADocumentController::getARAFactory();
}

//------------------------------------------------------------------------
} // namespace mam
