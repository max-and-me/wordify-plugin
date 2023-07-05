//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#include "ara_main_factory.h"
#include "version.h"
#include "vstgpt_cids.h"
#include "vstgpt_controller.h"
#include "vstgpt_processor.h"

#include "public.sdk/source/main/pluginfactory.h"

#define stringPluginName "VstGPT"

using namespace Steinberg::Vst;
using namespace mam;

//------------------------------------------------------------------------
//  VST Plug-in Entry
//------------------------------------------------------------------------
// Windows: do not forget to include a .def file in your project to export
// GetPluginFactory function!
//------------------------------------------------------------------------

DEF_CLASS_IID(ARA::IMainFactory)
DEF_CLASS_IID(ARA::IPlugInEntryPoint)
DEF_CLASS_IID(ARA::IPlugInEntryPoint2)

// clang-format off

BEGIN_FACTORY_DEF("Max And Me",
				  "https://www.maxandme.org",
				  "mailto:info@maxandme.org")

//---First Plug-in included in this factory-------
// its kVstAudioEffectClass component
DEF_CLASS2(INLINE_UID_FROM_FUID(kVstGPTProcessorUID),
		   PClassInfo::kManyInstances,		// cardinality
		   kVstAudioEffectClass,			// the component category (do not changed this)
		   stringPluginName,				// here the Plug-in name (to be changed)
		   Vst::kDistributable,				// means that component and controller could be distributed on different computers
		   VstGPTVST3Category,				// Subcategory for this Plug-in (to be changed)
		   FULL_VERSION_STR,				// Plug-in version (to be changed)
		   kVstVersionString,				// the VST 3 SDK version (do not changed this, use always this define)
		   VstGPTProcessor::createInstance) // function pointer called when this component should be instantiated

// its kVstComponentControllerClass component
DEF_CLASS2(INLINE_UID_FROM_FUID(kVstGPTControllerUID),
		   PClassInfo::kManyInstances,		 // cardinality
		   kVstComponentControllerClass,	 // the Controller category (do not changed this)
		   stringPluginName "Controller",	 // controller name (could be the same than component name)
		   0,								 // not used here
		   "",								 // not used here
		   FULL_VERSION_STR,				 // Plug-in version (to be changed)
		   kVstVersionString,				 // the VST 3 SDK version (do not changed this, use always this define)
		   VstGPTController::createInstance) // function pointer called when this component should be instantiated

// its kARAMainFactoryClass component
DEF_CLASS2(INLINE_UID_FROM_FUID(ARAMainFactory::getClassFUID()),
		   PClassInfo::kManyInstances,		   // cardinality
		   kARAMainFactoryClass,			   // the ARA Main Factory category (do not changed this)
		   stringPluginName "ARA Factory",				   // here the Plug-in name (MUST be the same as component name if multiple kVstAudioEffectClass components are used!)
		   0,								   // not used here
		   "",								   // not used here
		   FULL_VERSION_STR,				   // Plug-in version (to be changed)
		   kVstVersionString,				   // the VST 3 SDK version (do not changed this, use always this define)
		   ARAMainFactory::createInstance) // function pointer called when this component should be instantiated

//----for others Plug-ins contained in this factory, put like for the first Plug-in different DEF_CLASS2---

END_FACTORY

// clang-format on