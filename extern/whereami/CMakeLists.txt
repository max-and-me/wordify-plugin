cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    whereami
    GIT_REPOSITORY "https://github.com/gpakosz/whereami.git"
    GIT_TAG c5f6b015c36964790493841b9b4623184c0f7c3f
)

FetchContent_MakeAvailable(whereami)

add_library(whereami
    ${whereami_SOURCE_DIR}/src/whereami.c
    ${whereami_SOURCE_DIR}/src/whereami.h
)

target_include_directories(whereami
    PUBLIC
        ${whereami_SOURCE_DIR}/src
)