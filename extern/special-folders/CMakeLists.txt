
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    special-folders
    GIT_REPOSITORY "https://github.com/hansen-audio/special-folders.git"
    GIT_TAG c056bf053a2c7de183ce2c9146e1575b144ed1d1
)

FetchContent_MakeAvailable(special-folders)
