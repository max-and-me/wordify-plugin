
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    libsamplerate
    GIT_REPOSITORY https://github.com/libsndfile/libsamplerate.git
    GIT_TAG 0844c208f683527c08ea8a80acc13b398aa9c8bf
)

option(BUILD_TESTING "" OFF)

FetchContent_MakeAvailable(libsamplerate)
