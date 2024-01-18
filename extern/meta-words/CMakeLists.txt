
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    meta-words
    GIT_REPOSITORY "https://github.com/max-and-me/meta-words.git"
    GIT_TAG 07ff7c5adfe84247f2038ed352190ad191a38782
)

FetchContent_MakeAvailable(meta-words)
