//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#include "vstgpt_controller.h"
#include "vstgpt_cids.h"
#include "vstgpt_listcontroller.h"
#include "vstgui/plugin-bindings/vst3editor.h"

using namespace Steinberg;

namespace mam {

//------------------------------------------------------------------------
// VstGPTController Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API VstGPTController::initialize(FUnknown* context)
{
    // Here the Plug-in will be instantiated

    //---do not forget to call parent ------
    tresult result = EditControllerEx1::initialize(context);
    if (result != kResultOk)
    {
        return result;
    }

    // Here you could register some parameters

    return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstGPTController::terminate()
{
    // Here the Plug-in will be de-instantiated, last possibility to remove some
    // memory!

    //---do not forget to call parent ------
    return EditControllerEx1::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstGPTController::setComponentState(IBStream* state)
{
    // Here you get the state of the component (Processor part)
    if (!state)
        return kResultFalse;

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstGPTController::setState(IBStream* state)
{
    // Here you get the state of the controller

    return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstGPTController::getState(IBStream* state)
{
    // Here you are asked to deliver the state of the controller (if needed)
    // Note: the real state of your plug-in is saved in the processor

    return kResultTrue;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API VstGPTController::createView(FIDString name)
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

//------------------------------------------------------------------------
tresult PLUGIN_API VstGPTController::setParamNormalized(Vst::ParamID tag,
                                                        Vst::ParamValue value)
{
    // called by host to update your parameters
    tresult result = EditControllerEx1::setParamNormalized(tag, value);
    return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstGPTController::getParamStringByValue(
    Vst::ParamID tag, Vst::ParamValue valueNormalized, Vst::String128 string)
{
    // called by host to get a string for given normalized value of a specific
    // parameter (without having to set the value!)
    return EditControllerEx1::getParamStringByValue(tag, valueNormalized,
                                                    string);
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstGPTController::getParamValueByString(
    Vst::ParamID tag, Vst::TChar* string, Vst::ParamValue& valueNormalized)
{
    // called by host to get a normalized value from a string representation of
    // a specific parameter (without having to set the value!)
    return EditControllerEx1::getParamValueByString(tag, string,
                                                    valueNormalized);
}

//------------------------------------------------------------------------
VSTGUI::IController* VstGPTController::createSubController (
                                                            VSTGUI::UTF8StringPtr name, const VSTGUI::IUIDescription* /*description*/, VSTGUI::VST3Editor* /*editor*/)
{
    if (VSTGUI::UTF8StringView (name) == "MetaWordsListController")
        return new VstGPTListController ();
    
    return nullptr;
}
} // namespace mam
