
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    meta-words
    GIT_REPOSITORY "https://github.com/max-and-me/meta-words.git"
    GIT_TAG e5c0b9bae05a504a5e4fe791890992d3cc27fd3d
)

FetchContent_MakeAvailable(meta-words)
