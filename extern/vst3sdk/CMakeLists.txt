
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

option(VSTGUI_STANDALONE "VSTGUI Standalone library" OFF)
option(VSTGUI_STANDALONE_EXAMPLES "VSTGUI Standalone examples" OFF)
option(VSTGUI_TOOLS "Build VSTGUI Tools" OFF)
set(SMTG_ENABLE_VSTGUI_SUPPORT OFF)

FetchContent_Declare(
	vstgui
	GIT_REPOSITORY https://github.com/steinbergmedia/vstgui.git
	GIT_TAG fe9370d5b7136d763a90987e8f97cca1964d1e4b
)

FetchContent_GetProperties(vstgui)
if(NOT vstgui_POPULATED)
  FetchContent_Populate(vstgui)
endif()

option(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES "Enable VST 3 Plug-in Examples" OFF)
option(SMTG_ENABLE_VST3_HOSTING_EXAMPLES "Enable VST 3 Hosting Examples" OFF)
option(SMTG_CREATE_PLUGIN_LINK $<IF:$<CONFIG:Debug>,ON,OFF>)
if(WIN32)
    option(SMTG_CREATE_PLUGIN_LINK OFF)
endif()

FetchContent_Declare(
    vst3sdk
    GIT_REPOSITORY https://github.com/steinbergmedia/vst3sdk.git
    GIT_TAG v3.7.11_build_10
)

#set(VSTGUI_SOURCE_DIR "${vstgui_SOURCE_DIR}")
FetchContent_MakeAvailable(vst3sdk)

set(public_sdk_SOURCE_DIR "${vst3sdk_SOURCE_DIR}/public.sdk")
smtg_enable_vstgui_support(VSTGUI_SOURCE_DIR ${vstgui_SOURCE_DIR})