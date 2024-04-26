
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    meta-words
    GIT_REPOSITORY "https://github.com/max-and-me/meta-words.git"
    GIT_TAG 4f95b5442cc840838da16363314254db7f040a6f
)

FetchContent_MakeAvailable(meta-words)
