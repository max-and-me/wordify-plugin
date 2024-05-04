cmake_minimum_required(VERSION 3.25.0)

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15 CACHE STRING "")
string(TIMESTAMP VERSION_NOW "%Y.%m") # VERSION_NOW e.g. 2023.08

# Only turn on validator in Release configuration
option(SMTG_RUN_VST_VALIDATOR $<$<CONFIG:Debug>:OFF>$<$<CONFIG:Release>:ON>)

project(VstGPT
    # This is your plug-in version number. Change it here only.
    # Version number symbols usable in C++ can be found in
    # source/version.h and ${PROJECT_BINARY_DIR}/projectversion.h.
    VERSION       ${VERSION_NOW}
    DESCRIPTION   "Wordify VST 3 Plug-in"
    HOMEPAGE_URL  "https://www.wordify.org"
)

string(TIMESTAMP YEAR_NOW     "%Y") # Current year
set(CPACK_PACKAGE_VENDOR      "WordifyOrg")
set(PROJECT_CONTACT_EMAIL     "info@wordify.org")
set(PROJECT_BUNDLE_IDENTIFIER "org.wordify.plugin.vst3")
set(PROJECT_LEGAL_COPYRIGHT   "Copyright(c) ${YEAR_NOW}")

add_subdirectory(extern)
smtg_enable_vst3_sdk()

smtg_add_vst3plugin(VstGPT
    ${public_sdk_SOURCE_DIR}/source/vst/vstsinglecomponenteffect.cpp
    ${public_sdk_SOURCE_DIR}/source/vst/vstsinglecomponenteffect.h
    source/ara_document_controller.cpp
    source/ara_document_controller.h
    source/ara_factory_config.cpp
    source/ara_factory_config.h
    source/audio_source_analyze_worker.cpp
    source/audio_source_analyze_worker.h
    source/audio_buffer_management.h
    source/header_controller.cpp
    source/header_controller.h
    source/hstack_layout.cpp
    source/hstack_layout.h
    source/list_controller.cpp
    source/list_controller.h
    source/list_entry_controller.cpp
    source/list_entry_controller.h
    source/little_helpers.h
    source/meta_words_audio_modification.cpp
    source/meta_words_audio_modification.h
    source/meta_words_audio_source.cpp
    source/meta_words_audio_source.h
    source/meta_words_clip_controller.cpp
    source/meta_words_clip_controller.h
    source/meta_words_data.h
    source/meta_words_editor_renderer.cpp
    source/meta_words_editor_renderer.h
    source/meta_words_editor_view.cpp
    source/meta_words_editor_view.h
    source/meta_words_playback_region.cpp
    source/meta_words_playback_region.h
    source/meta_words_playback_renderer.cpp
    source/meta_words_playback_renderer.h
    source/meta_words_serde.cpp
    source/meta_words_serde.h
    source/parameter_ids.h
    source/preferences_serde.cpp
    source/preferences_serde.h
    source/preferences_controller.cpp
    source/preferences_controller.h
    source/region_order_manager.cpp
    source/region_order_manager.h
    source/spinner_controller.cpp
    source/spinner_controller.h
    source/spinner_view.cpp
    source/spinner_view.h
    source/tiny_observer_pattern.h
    source/tiny_selection_model.h
    source/version.h
    source/view_animations.cpp
    source/view_animations.h
    source/vstgpt_cids.h
    source/vstgpt_entry.cpp
    source/vstgpt_single_component.cpp
    source/vstgpt_single_component.h
    source/waveform_controller.cpp
    source/waveform_controller.h
    source/waveform_view.cpp
    source/waveform_view.h
)

smtg_target_add_plugin_resources(VstGPT
    RESOURCES
        "resource/vstgpt_editor.uidesc"
        "resource/editor_res_dark_scheme.uidesc"
        "resource/editor_res_signal_dark_scheme.uidesc"
        "resource/editor_res_signal_lite_scheme.uidesc"
        "resource/moon-solid.png"
        "resource/moon-solid_2x.png"
        "resource/moon-solid_3x.png"
        "resource/moon-solid_4x.png"
        "resource/sun-solid.png"
        "resource/sun-solid_2x.png"
        "resource/sun-solid_3x.png"
        "resource/sun-solid_4x.png"
)

smtg_target_add_plugin_snapshots (VstGPT
    RESOURCES
        resource/76FA7B014D2757B49BA55204681B0F2C_snapshot.png
        resource/76FA7B014D2757B49BA55204681B0F2C_snapshot_2.0x.png
)

target_link_libraries(VstGPT
    PRIVATE
        ARA_PlugIn_Library
        fmt
        meta-words
        nlohmann_json
        presonus-plugin-extensions
        samplerate
        sdk
        sndfile
        special-folders
        tiny-process-library
        wave-draw
        vstgui_support
)

smtg_target_configure_version_file(VstGPT)

if(SMTG_MAC)
    smtg_target_set_bundle(VstGPT
        BUNDLE_IDENTIFIER ${PROJECT_BUNDLE_IDENTIFIER}
        COMPANY_NAME "${CPACK_PACKAGE_VENDOR}"
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
set_target_properties(main PROPERTIES 
    OUTPUT_NAME "WordifyWorker"
)

target_compile_definitions(VstGPT
    PRIVATE
        MAM_WHISPER_CPP_EXECUTABLE="$<TARGET_FILE:main>"
        MAM_WHISPER_CPP_MODEL_DOWNLOAD_DIR="${MAM_WHISPER_CPP_MODEL_DOWNLOAD_DIR}"
        CPACK_PACKAGE_VENDOR="${CPACK_PACKAGE_VENDOR}"
        PROJECT_HOMEPAGE_URL="${PROJECT_HOMEPAGE_URL}"
        PROJECT_CONTACT_EMAIL="${PROJECT_CONTACT_EMAIL}"
        PROJECT_NAME="${PROJECT_NAME}"
        PROJECT_BUNDLE_IDENTIFIER="${PROJECT_BUNDLE_IDENTIFIER}"
        PROJECT_LEGAL_COPYRIGHT="${PROJECT_LEGAL_COPYRIGHT}"
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

# Installer
set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE.txt)
set(MAM_WORDIFY_MODEL_FILE "${MAM_WHISPER_CPP_MODEL_DOWNLOAD_DIR}/ggml-medium.bin")
message(STATUS "[MAM] CMAKE_SYSTEM_NAME          : ${CMAKE_SYSTEM_NAME}")
message(STATUS "[MAM] CPACK_RESOURCE_FILE_LICENSE: ${CPACK_RESOURCE_FILE_LICENSE}")
message(STATUS "[MAM] MAM_WORDIFY_MODEL_FILE     : ${MAM_WORDIFY_MODEL_FILE}")
message(STATUS "[MAM] CPACK_PACKAGE_VENDOR       : ${CPACK_PACKAGE_VENDOR}")
message(STATUS "[MAM] PROJECT_HOMEPAGE_URL       : ${PROJECT_HOMEPAGE_URL}")
message(STATUS "[MAM] PROJECT_CONTACT_EMAIL      : ${PROJECT_CONTACT_EMAIL}")
message(STATUS "[MAM] PROJECT_NAME               : ${PROJECT_NAME}")
message(STATUS "[MAM] BUNDLE_IDENTIFIER          : ${BUNDLE_IDENTIFIER}")

get_target_property(PLUGIN_PACKAGE_PATH VstGPT SMTG_PLUGIN_PACKAGE_PATH)

vst3_cpack_configure(
    PLUGIN_PACKAGE_PATH "${PLUGIN_PACKAGE_PATH}"
)

# AI Models
cpack_add_component(
    modeldata
    DISPLAY_NAME "Model Data"
    INSTALL_TYPES full
    REQUIRED
)

# Installation path for ai model data
# Windows: C:\Program Data\...
# macOS  : /Library/Application Support/...
set(CPACK_INNOSETUP_modeldata_INSTALL_DIRECTORY "{commonappdata}")    # C:\Program Data\
set(MAM_MODEL_DATA_FOLDER "${CPACK_PACKAGE_VENDOR}/${PROJECT_NAME}/ModelData")
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(MAM_MODEL_DATA_FOLDER "Library/Application Support/${MAM_MODEL_DATA_FOLDER}")
endif()

message(STATUS "[MAM] MAM_MODEL_DATA_FOLDER: ${MAM_MODEL_DATA_FOLDER}")

install(
    FILES       "${MAM_WORDIFY_MODEL_FILE}"
    DESTINATION "${MAM_MODEL_DATA_FOLDER}"
    COMPONENT   modeldata
)

list(APPEND CPACK_COMPONENTS_ALL modeldata)

include(CPack)