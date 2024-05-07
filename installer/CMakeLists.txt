cmake_minimum_required(VERSION 3.20.0)

set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE.txt)

message(STATUS "[MAM] CPACK_RESOURCE_FILE_LICENSE: ${CPACK_RESOURCE_FILE_LICENSE}")

# Add Wordify plugin
get_target_property(PLUGIN_PACKAGE_PATH Wordify SMTG_PLUGIN_PACKAGE_PATH)
vst3_cpack_configure(
    PLUGIN_PACKAGE_PATH "${PLUGIN_PACKAGE_PATH}"
)

# Add another install component for the model data
cpack_add_component(
    modeldata
    DISPLAY_NAME "Model Data"
    INSTALL_TYPES full
    REQUIRED
)

# Installation path for the model data:
# Windows: C:\Program Data\WordifyOrg\Wordify\ModelData\...
# macOS  : /Library/Application Support/WordifyOrg/Wordify/ModelData/...
set(CPACK_INNOSETUP_modeldata_INSTALL_DIRECTORY "{commonappdata}")    # C:\Program Data\
set(MAM_MODEL_DATA_FOLDER "${CPACK_PACKAGE_VENDOR}/${PROJECT_NAME}/ModelData")
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # On macos we need to add 'Library/Application Support'
    set(MAM_MODEL_DATA_FOLDER "Library/Application Support/${MAM_MODEL_DATA_FOLDER}")
endif()

install(
    FILES       "${MAM_WORDIFY_MODEL_FILE}"
    DESTINATION "${MAM_MODEL_DATA_FOLDER}"
    COMPONENT   modeldata
)

list(APPEND CPACK_COMPONENTS_ALL modeldata)

message(STATUS "[MAM] MAM_MODEL_DATA_FOLDER: ${MAM_MODEL_DATA_FOLDER}")

include(CPack)