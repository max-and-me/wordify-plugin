cmake_minimum_required(VERSION 3.14.0)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15 CACHE STRING "")

project(VstGPT
    # This is your plug-in version number. Change it here only.
    # Version number symbols usable in C++ can be found in
    # source/version.h and ${PROJECT_BINARY_DIR}/projectversion.h.
    VERSION 1.0.0.0 
    DESCRIPTION "VstGPT VST 3 Plug-in"
)

add_subdirectory(extern)
smtg_enable_vst3_sdk()

smtg_add_vst3plugin(VstGPT
   source/ara_audio_source.cpp
   source/ara_audio_source.h
   source/ara_document_controller.cpp
   source/ara_document_controller.h
   source/ara_factory_config.cpp
   source/ara_factory_config.h
   source/meta_words_audio_modification.cpp
   source/meta_words_audio_modification.h
   source/version.h
   source/vstgpt_cids.h
   source/vstgpt_context.h
   source/vstgpt_context.cpp
   source/vstgpt_controller.cpp
   source/vstgpt_controller.h
   source/vstgpt_entry.cpp
   source/vstgpt_listcontroller.h
   source/vstgpt_listcontroller.cpp
   source/vstgpt_processor.cpp
   source/vstgpt_processor.h
)

smtg_target_add_plugin_resources(VstGPT
    RESOURCES
        "resource/vstgpt_editor.uidesc"
)

smtg_target_add_plugin_snapshots (VstGPT
    RESOURCES
        resource/76FA7B014D2757B49BA55204681B0F2C_snapshot.png
        resource/76FA7B014D2757B49BA55204681B0F2C_snapshot_2.0x.png
)

target_link_libraries(VstGPT
    PRIVATE
        sdk
        ARA_PlugIn_Library
        tiny-process-library
        sndfile
        meta-words
)

smtg_target_configure_version_file(VstGPT)

if(SMTG_MAC)
    smtg_target_set_bundle(VstGPT
        BUNDLE_IDENTIFIER org.maxandme.vst3.vstgpt
        COMPANY_NAME "Max And Me"
    )
    smtg_target_set_debug_executable(VstGPT
        "/Applications/VST3PluginTestHost.app"
        "--pluginfolder;$(BUILT_PRODUCTS_DIR)"
    )
elseif(SMTG_WIN)
    target_sources(VstGPT PRIVATE 
        resource/win32resource.rc
    )
    if(MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VstGPT)

        smtg_target_set_debug_executable(VstGPT
            "$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
            "--pluginfolder \"$(OutDir)/\""
        )
    endif()

    if(WIN32)
        target_compile_definitions(VstGPT
            PUBLIC
                -D_UNICODE
                -DUNICODE
        )
    
        target_compile_definitions(tiny-process-library
            PUBLIC
                -D_UNICODE
                -DUNICODE
        )
    endif()
endif(SMTG_MAC)

# VSTGUI support
target_sources(VstGPT
    PRIVATE
        resource/vstgpt_editor.uidesc
)

target_link_libraries(VstGPT
    PRIVATE
        vstgui_support
)

target_compile_features(VstGPT 
    PRIVATE
        cxx_std_17
)

set_property(
    TARGET
        ARA_PlugIn_Library
        tiny-process-library
        sndfile
        meta-words
    PROPERTY
        POSITION_INDEPENDENT_CODE ON
)

# whisper.cpp exe and model
target_compile_definitions(VstGPT
    PRIVATE
        MAM_WHISPER_CPP_EXECUTABLE="$<TARGET_FILE:main>"
        MAM_WHISPER_CPP_MODEL_DOWNLOAD_DIR="${MAM_WHISPER_CPP_MODEL_DOWNLOAD_DIR}"
)

# test
add_executable(temp-dir-test
    test/test_temp_dir.cpp
)

target_compile_features(temp-dir-test 
    PRIVATE
        cxx_std_17
)

# Dependency Graph
add_custom_target(${PROJECT_NAME}-dependency-graph
    COMMAND ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} --graphviz=${CMAKE_BINARY_DIR}/graphviz/${PROJECT_NAME}.dot
)