
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    special-folders
    GIT_REPOSITORY "https://github.com/hansen-audio/special-folders.git"
    GIT_TAG b209cffd230a0d2c807837c4192f1cc72a943c58
)

FetchContent_MakeAvailable(special-folders)
