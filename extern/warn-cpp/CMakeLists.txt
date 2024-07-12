cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    warn-cpp
    GIT_REPOSITORY "https://github.com/max-and-me/warn-cpp.git"
    GIT_TAG main
)

FetchContent_MakeAvailable(warn-cpp)
