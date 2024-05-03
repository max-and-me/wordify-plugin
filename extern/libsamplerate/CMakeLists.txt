
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    libsamplerate
    GIT_REPOSITORY https://github.com/libsndfile/libsamplerate.git
    GIT_TAG 0.2.2
)

option(BUILD_TESTING OFF)
FetchContent_MakeAvailable(libsamplerate)
