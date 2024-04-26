
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    meta-words
    GIT_REPOSITORY "https://github.com/max-and-me/meta-words.git"
    GIT_TAG 79bc1dfaa323aa0c258449479cbd9697dd23be84
)

FetchContent_MakeAvailable(meta-words)
