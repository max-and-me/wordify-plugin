cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    libtinyfiledialogs
    GIT_REPOSITORY "https://github.com/native-toolkit/libtinyfiledialogs.git"
    GIT_TAG 2.9.3
)

FetchContent_MakeAvailable(libtinyfiledialogs)

add_library(libtinyfiledialogs
    ${libtinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.c
    ${libtinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.h
)

target_include_directories(libtinyfiledialogs
    PUBLIC
        ${libtinyfiledialogs_SOURCE_DIR}
)
