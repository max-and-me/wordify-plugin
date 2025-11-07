
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    arasdk
    GIT_REPOSITORY https://github.com/Celemony/ARA_SDK.git
    GIT_TAG releases/2.3.0
    SOURCE_SUBDIR ARA_Library
)

FetchContent_MakeAvailable(arasdk)
