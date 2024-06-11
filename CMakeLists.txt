cmake_minimum_required(VERSION 3.25.0)

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15 CACHE STRING "")
string(TIMESTAMP VERSION_NOW "%Y.%m") # VERSION_NOW e.g. 2023.08

# Only turn on validator in Release configuration
option(SMTG_RUN_VST_VALIDATOR $<$<CONFIG:Debug>:OFF>$<$<CONFIG:Release>:ON>)

project("Wordify"
    # This is your plug-in version number. Change it here only.
    # Version number symbols usable in C++ can be found in
    # source/version.h and ${PROJECT_BINARY_DIR}/projectversion.h.
    VERSION       ${VERSION_NOW}
    DESCRIPTION   "Wordify VST 3 Plug-in"
    HOMEPAGE_URL  "https://www.wordify.org"
)

string(TIMESTAMP YEAR_NOW     "%Y") # Current year
set(CPACK_PACKAGE_VENDOR      "WordifyOrg")
set(PROJECT_CONTACT_EMAIL     "hey@wordify.org")
set(PROJECT_BUNDLE_IDENTIFIER "org.wordify.plugin.vst3")
set(PROJECT_LEGAL_COPYRIGHT   "Copyright(c) ${YEAR_NOW}")
set(MAM_GGML_DIRECTORY_NAME   "ModelData")

message(STATUS "[MAM] CMAKE_SYSTEM_NAME          : ${CMAKE_SYSTEM_NAME}")
message(STATUS "[MAM] CPACK_PACKAGE_VENDOR       : ${CPACK_PACKAGE_VENDOR}")
message(STATUS "[MAM] PROJECT_HOMEPAGE_URL       : ${PROJECT_HOMEPAGE_URL}")
message(STATUS "[MAM] PROJECT_CONTACT_EMAIL      : ${PROJECT_CONTACT_EMAIL}")
message(STATUS "[MAM] PROJECT_NAME               : ${PROJECT_NAME}")
message(STATUS "[MAM] PROJECT_BUNDLE_IDENTIFIER  : ${PROJECT_BUNDLE_IDENTIFIER}")
message(STATUS "[MAM] PROJECT_LEGAL_COPYRIGHT    : ${PROJECT_LEGAL_COPYRIGHT}")

add_subdirectory(extern)
smtg_enable_vst3_sdk()

smtg_add_vst3plugin(Wordify
    ${public_sdk_SOURCE_DIR}/source/vst/vstsinglecomponenteffect.cpp
    ${public_sdk_SOURCE_DIR}/source/vst/vstsinglecomponenteffect.h
    source/ara_document_controller.cpp
    source/ara_document_controller.h
    source/ara_factory_config.cpp
    source/ara_factory_config.h
    source/audio_buffer_management.h
    source/audio_source_analyze_worker.cpp
    source/audio_source_analyze_worker.h
    source/hilite_text_button.cpp
    source/hilite_text_button.h
    source/hstack_layout.cpp
    source/hstack_layout.h
    source/list_controller.cpp
    source/list_controller.h
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
    source/nonstd.h
    source/parameter_ids.h
    source/preferences_controller.cpp
    source/preferences_controller.h
    source/preferences_serde.cpp
    source/preferences_serde.h
    source/region_order_manager.cpp
    source/region_order_manager.h
    source/search_controller.cpp
    source/search_controller.h
    source/search_engine.cpp
    source/search_engine.h
    source/spinner_controller.cpp
    source/spinner_controller.h
    source/spinner_view.cpp
    source/spinner_view.h
    source/string_matcher.cpp
    source/string_matcher.h
    source/task_manager.cpp
    source/task_manager.h
    source/tiny_selection_model.h
    source/version.h
    source/view_animations.cpp
    source/view_animations.h
    source/waveform_controller.cpp
    source/waveform_controller.h
    source/waveform_view.cpp
    source/waveform_view.h
    source/whipser_cpp_wrapper.cpp
    source/whipser_cpp_wrapper.h
    source/wordify_cids.h
    source/wordify_defines.h
    source/wordify_entry.cpp
    source/wordify_single_component.cpp
    source/wordify_single_component.h
    source/wordify_types.h
)

smtg_target_add_plugin_resources(Wordify
    RESOURCES
        "resource/bulb-solid_dark.png"
        "resource/bulb-solid_dark_2x.png"
        "resource/bulb-solid_dark_3x.png"
        "resource/bulb-solid_dark_4x.png"
        "resource/bulb-solid_lite.png"
        "resource/bulb-solid_lite_2x.png"
        "resource/bulb-solid_lite_3x.png"
        "resource/bulb-solid_lite_4x.png"
        "resource/chevron-down-solid_dark.png"
        "resource/chevron-down-solid_dark_2x.png"
        "resource/chevron-down-solid_dark_3x.png"
        "resource/chevron-down-solid_dark_4x.png"
        "resource/chevron-down-solid_lite.png"
        "resource/chevron-down-solid_lite_2x.png"
        "resource/chevron-down-solid_lite_3x.png"
        "resource/chevron-down-solid_lite_4x.png"
        "resource/chevron-up-solid_dark.png"
        "resource/chevron-up-solid_dark_2x.png"
        "resource/chevron-up-solid_dark_3x.png"
        "resource/chevron-up-solid_dark_4x.png"
        "resource/chevron-up-solid_lite.png"
        "resource/chevron-up-solid_lite_2x.png"
        "resource/chevron-up-solid_lite_3x.png"
        "resource/chevron-up-solid_lite_4x.png"
        "resource/editor_res_dark_scheme.uidesc"
        "resource/editor_res_signal_dark_scheme.uidesc"
        "resource/editor_res_signal_lite_scheme.uidesc"
        "resource/magnifying-glass-solid_dark.png"
        "resource/magnifying-glass-solid_dark_2x.png"
        "resource/magnifying-glass-solid_dark_3x.png"
        "resource/magnifying-glass-solid_dark_4x.png"
        "resource/magnifying-glass-solid_lite.png"
        "resource/magnifying-glass-solid_lite_2x.png"
        "resource/magnifying-glass-solid_lite_3x.png"
        "resource/magnifying-glass-solid_lite_4x.png"
        "resource/moon-solid.png"
        "resource/moon-solid_2x.png"
        "resource/moon-solid_3x.png"
        "resource/moon-solid_4x.png"
        "resource/sun-solid.png"
        "resource/sun-solid_2x.png"
        "resource/sun-solid_3x.png"
        "resource/sun-solid_4x.png"
        "resource/wordify_editor.uidesc"
)

smtg_target_add_plugin_snapshots (Wordify
    RESOURCES
        resource/76FA7B014D2757B49BA55204681B0F2C_snapshot.png
        resource/76FA7B014D2757B49BA55204681B0F2C_snapshot_2.0x.png
)

target_link_libraries(Wordify
    PRIVATE
        ARA_PlugIn_Library
        eventpp::eventpp
        fmt
        meta-words
        nlohmann_json
        presonus-plugin-extensions
        samplerate
        sdk
        sndfile
        special-folders
        wave-draw
        vstgui_support
        whereami
)

smtg_target_configure_version_file(Wordify)

# VSTGUI support
target_sources(Wordify
    PRIVATE
        resource/wordify_editor.uidesc
)

target_link_libraries(Wordify
    PRIVATE
        vstgui_support
)

target_compile_features(Wordify 
    PRIVATE
        cxx_std_17
)

set_property(
    TARGET
        ARA_PlugIn_Library
        common
        fmt
        main
        meta-words
        nlohmann_json
        presonus-plugin-extensions
        samplerate
        sndfile
        special-folders
        tiny-process-library
        vstgui_support
        wave-draw
        whereami
        whisper
    PROPERTY
        POSITION_INDEPENDENT_CODE ON
)

target_compile_definitions(Wordify
    PRIVATE
        MAM_WHISPER_CPP_EXECUTABLE="$<TARGET_FILE:main>"
        MAM_WHISPER_CPP_EXECUTABLE_NAME="$<TARGET_FILE_NAME:main>"
        MAM_GGML_DIRECTORY_NAME="${MAM_GGML_DIRECTORY_NAME}"
        MAM_WHISPER_CPP_MODEL_DOWNLOAD_DIR="${MAM_WHISPER_CPP_MODEL_DOWNLOAD_DIR}"
        CPACK_PACKAGE_VENDOR="${CPACK_PACKAGE_VENDOR}"
        PROJECT_HOMEPAGE_URL="${PROJECT_HOMEPAGE_URL}"
        PROJECT_CONTACT_EMAIL="${PROJECT_CONTACT_EMAIL}"
        PROJECT_NAME="${PROJECT_NAME}"
        PROJECT_BUNDLE_IDENTIFIER="${PROJECT_BUNDLE_IDENTIFIER}"
        PROJECT_LEGAL_COPYRIGHT="${PROJECT_LEGAL_COPYRIGHT}"
)

if(SMTG_MAC)
    smtg_target_set_bundle(Wordify
        BUNDLE_IDENTIFIER ${PROJECT_BUNDLE_IDENTIFIER}
        COMPANY_NAME "${CPACK_PACKAGE_VENDOR}"
    )

    # How ugly, just to add ggml-common.h and ggml-metal.metal to the bundle :-/
    get_target_property(whisper_SOURCE_DIR whisper whisper_SOURCE_DIR)
    message(STATUS "[MAM] whisper_SOURCE_DIR: ${whisper_SOURCE_DIR}")
    get_target_property(PLUGIN_PACKAGE_PATH Wordify SMTG_PLUGIN_PACKAGE_PATH)
    get_target_property(PLUGIN_PACKAGE_RESOURCES Wordify SMTG_PLUGIN_PACKAGE_RESOURCES)
    set(resources_folder "${PLUGIN_PACKAGE_PATH}/${PLUGIN_PACKAGE_RESOURCES}")
    add_custom_command(
            TARGET Wordify POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${whisper_SOURCE_DIR}/ggml-common.h
                ${resources_folder}/ggml-common.h
    )
    add_custom_command(
        TARGET Wordify POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${whisper_SOURCE_DIR}/ggml-metal.metal
            ${resources_folder}/ggml-metal.metal
    )   
elseif(SMTG_WIN)
    target_sources(Wordify PRIVATE 
        resource/win32resource.rc
    )

    target_link_libraries(Wordify 
        PRIVATE
            utf-8-everywhere
            shell32
    )

    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Wordify)

    target_compile_definitions(Wordify
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

# This needs to be fixed in vst3sdk.
# https://github.com/steinbergmedia/vst3_cmake/blob/master/modules/SMTG_AddSMTGLibrary.cmake#L292C21-L292C54
# If this is not set, "$<TARGET_FILE_DIR:Wordify>" returns 'Wordify.bundle' instead of 'Wordify.vst3'
set_target_properties(Wordify PROPERTIES
    BUNDLE_EXTENSION vst3
)

# whispercpp's command line executable is called 'main'. We need
# some more meaningful name and simple rename the executable
# 'WordifyWorker'
set_target_properties(main PROPERTIES 
    OUTPUT_NAME "WordifyWorker"
)

smtg_target_setup_universal_binary(fmt)
smtg_target_setup_universal_binary(nlohmann_json)
smtg_target_setup_universal_binary(presonus-plugin-extensions)
smtg_target_setup_universal_binary(samplerate)
smtg_target_setup_universal_binary(special-folders)
smtg_target_setup_universal_binary(wave-draw)
smtg_target_setup_universal_binary(vstgui_support)
smtg_target_setup_universal_binary(ARA_PlugIn_Library)
smtg_target_setup_universal_binary(tiny-process-library)
smtg_target_setup_universal_binary(sndfile)
smtg_target_setup_universal_binary(meta-words)
smtg_target_setup_universal_binary(whereami)
smtg_target_setup_universal_binary(main)
smtg_target_setup_universal_binary(whisper)
smtg_target_setup_universal_binary(common)

# Copy WordifyWorker next to our Wordify executable
add_custom_command(
    TARGET Wordify POST_BUILD
    COMMENT "[MAM] Copy WordifyWorker."
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:main>"
        "$<TARGET_FILE_DIR:Wordify>"
)

# Dependency Graph
add_custom_target(${PROJECT_NAME}-dependency-graph
    COMMAND ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} --graphviz=${CMAKE_BINARY_DIR}/graphviz/${PROJECT_NAME}.dot
)

# Installer
set(MAM_WORDIFY_MODEL_FILE "${MAM_WHISPER_CPP_MODEL_DOWNLOAD_DIR}/ggml-medium.bin")
message(STATUS "[MAM] MAM_WORDIFY_MODEL_FILE     : ${MAM_WORDIFY_MODEL_FILE}")

add_subdirectory(installer)
