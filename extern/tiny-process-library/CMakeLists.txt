
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    tiny-process-library
    GIT_REPOSITORY https://gitlab.com/eidheim/tiny-process-library.git
    GIT_TAG 93edd94959a2cc8aec1ea4ec5df82835b2b05bac
)

option(BUILD_TESTING "" OFF)

FetchContent_MakeAvailable(tiny-process-library)
