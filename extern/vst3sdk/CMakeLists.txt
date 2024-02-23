
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

option(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES "Enable VST 3 Plug-in Examples" OFF)
option(SMTG_ENABLE_VST3_HOSTING_EXAMPLES "Enable VST 3 Hosting Examples" OFF)
if(WIN32)
    option(SMTG_CREATE_PLUGIN_LINK OFF)
endif()

FetchContent_Declare(
    vst3sdk
    GIT_REPOSITORY https://github.com/steinbergmedia/vst3sdk.git
    GIT_TAG v3.7.8_build_34
)

FetchContent_MakeAvailable(vst3sdk)
