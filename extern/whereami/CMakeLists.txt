cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    whereami
    GIT_REPOSITORY "https://github.com/gpakosz/whereami.git"
    GIT_TAG dcb52a058dc14530ba9ae05e4339bd3ddfae0e0e
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