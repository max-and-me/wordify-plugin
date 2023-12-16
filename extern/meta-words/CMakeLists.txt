
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    meta-words
    GIT_REPOSITORY "https://github.com/max-and-me/meta-words.git"
    GIT_TAG 319f0761b1dea59cd4e1f440bc2491375a06f3e1
)

FetchContent_MakeAvailable(meta-words)
