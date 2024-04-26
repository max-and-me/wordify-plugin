
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    special-folders
    GIT_REPOSITORY "https://github.com/hansen-audio/special-folders.git"
    GIT_TAG main
)

FetchContent_MakeAvailable(special-folders)
