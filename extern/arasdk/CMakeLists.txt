
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    arasdk
    GIT_REPOSITORY https://github.com/Celemony/ARA_SDK.git
    GIT_TAG 6eb5c48063a8fecfdf2ff5dce015db654d3bb7d5
    SOURCE_SUBDIR ARA_Library
)

FetchContent_MakeAvailable(arasdk)
