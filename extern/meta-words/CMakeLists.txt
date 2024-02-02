
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    meta-words
    GIT_REPOSITORY "https://github.com/max-and-me/meta-words.git"
    GIT_TAG 90d3168c363ac17a28ac290c3065167a9a3616e4
)

FetchContent_MakeAvailable(meta-words)
