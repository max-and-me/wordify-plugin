
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

option(VSTGUI_STANDALONE "VSTGUI Standalone library" OFF)
option(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES "Enable VST 3 Plug-in Examples" OFF)
option(VSTGUI_STANDALONE_EXAMPLES "VSTGUI Standalone examples" OFF)
option(VSTGUI_TOOLS "Build VSTGUI Tools" OFF)
set(SMTG_ENABLE_VSTGUI_SUPPORT OFF)

FetchContent_Declare(
    vstgui
    GIT_REPOSITORY https://github.com/steinbergmedia/vstgui.git
    GIT_TAG 772e5e141b6da97e66976cf41b83681d2bc86d90
)

FetchContent_GetProperties(vstgui)
if(NOT vstgui_POPULATED)
    FetchContent_Populate(vstgui)
endif()

# Patch for this issue: https://github.com/steinbergmedia/vstgui/issues/249
# I have no other solution for this atm but patching
execute_process(COMMAND ${GIT_EXECUTABLE} apply --ignore-whitespace "${CMAKE_CURRENT_LIST_DIR}/vstgui_cairo_finish.patch"
    WORKING_DIRECTORY ${vstgui_SOURCE_DIR}
    RESULT_VARIABLE ret
    ERROR_VARIABLE err
    TIMEOUT 5
)
# 
# option(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES "Enable VST 3 Plug-in Examples" OFF)
# option(SMTG_ENABLE_VST3_HOSTING_EXAMPLES "Enable VST 3 Hosting Examples" OFF)
# option(SMTG_CREATE_PLUGIN_LINK $<IF:$<CONFIG:Debug>,ON,OFF>)
# if(WIN32)
#     option(SMTG_CREATE_PLUGIN_LINK OFF)
# endif()

# Disable symbolic link creation on all platforms because of a bug in v3.7.14_build_55
option(SMTG_CREATE_PLUGIN_LINK OFF)

FetchContent_Declare(
    vst3sdk
    GIT_REPOSITORY https://github.com/steinbergmedia/vst3sdk.git
    GIT_TAG v3.8.0_build_66
)

#set(VSTGUI_SOURCE_DIR "${vstgui_SOURCE_DIR}")
FetchContent_MakeAvailable(vst3sdk)

set(public_sdk_SOURCE_DIR "${vst3sdk_SOURCE_DIR}/public.sdk")
smtg_enable_vstgui_support(VSTGUI_SOURCE_DIR ${vstgui_SOURCE_DIR})
