
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    presonus-plugin-extensions
    GIT_REPOSITORY "https://github.com/fenderdigital/presonus-plugin-extensions.git"
    GIT_TAG 04-2024 
)

FetchContent_MakeAvailable(presonus-plugin-extensions)

add_library(presonus-plugin-extensions INTERFACE)

target_include_directories(presonus-plugin-extensions
    INTERFACE
        ${presonus-plugin-extensions_SOURCE_DIR}
)
