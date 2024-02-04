
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    meta-words
    GIT_REPOSITORY "https://github.com/max-and-me/meta-words.git"
    GIT_TAG 855f35950cdb70693d949a5e5bbb8b481fed3f67
)

FetchContent_MakeAvailable(meta-words)
