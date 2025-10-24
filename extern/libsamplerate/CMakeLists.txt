
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    libsamplerate
    GIT_REPOSITORY https://github.com/libsndfile/libsamplerate.git
    GIT_TAG 2ccde9568cca73c7b32c97fefca2e418c16ae5e3
)

option(BUILD_TESTING "" OFF)

FetchContent_MakeAvailable(libsamplerate)
