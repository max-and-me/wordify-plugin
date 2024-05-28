//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#include "ara_main_factory.h"
#include "public.sdk/source/main/pluginfactory.h"
#include "version.h"
#include "wordify_cids.h"
#include "wordify_defines.h"
#include "wordify_single_component.h"

using namespace Steinberg::Vst;
using namespace mam;

//------------------------------------------------------------------------
//  VST Plug-in Entry
//------------------------------------------------------------------------
DEF_CLASS_IID(ARA::IMainFactory)
DEF_CLASS_IID(ARA::IPlugInEntryPoint)
DEF_CLASS_IID(ARA::IPlugInEntryPoint2)

// clang-format off

BEGIN_FACTORY_DEF(COMPANY_NAME_STR,
				  COMPANY_URL_STR,
				  COMPANY_EMAIL_STR)

//---First Plug-in included in this factory-------
DEF_CLASS2(INLINE_UID_FROM_FUID(kWordifyProcessorUID),
		   PClassInfo::kManyInstances,		// cardinality
		   kVstAudioEffectClass,			// the component category (do not changed this)
		   PLUGIN_NAME_STR,				// here the Plug-in name (to be changed)
		   Vst::kDistributable,				// means that component and controller could be distributed on different computers
		   WordifyVST3Category,				// Subcategory for this Plug-in (to be changed)
		   FULL_VERSION_STR,				// Plug-in version (to be changed)
		   kVstVersionString,				// the VST 3 SDK version (do not changed this, use always this define)
		   WordifySingleComponent::createInstance) // function pointer called when this component should be instantiated

// its kARAMainFactoryClass component
DEF_CLASS2(INLINE_UID_FROM_FUID(ARAMainFactory::getClassFUID()),
		   PClassInfo::kManyInstances,		   // cardinality
		   kARAMainFactoryClass,			   // the ARA Main Factory category (do not changed this)
		   PLUGIN_NAME_STR "ARA Factory",				   // here the Plug-in name (MUST be the same as component name if multiple kVstAudioEffectClass components are used!)
		   0,								   // not used here
		   "",								   // not used here
		   FULL_VERSION_STR,				   // Plug-in version (to be changed)
		   kVstVersionString,				   // the VST 3 SDK version (do not changed this, use always this define)
		   ARAMainFactory::createInstance) // function pointer called when this component should be instantiated

//----for others Plug-ins contained in this factory, put like for the first Plug-in different DEF_CLASS2---

END_FACTORY

// clang-format on