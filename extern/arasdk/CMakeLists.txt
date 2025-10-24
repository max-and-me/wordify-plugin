
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    arasdk
    GIT_REPOSITORY https://github.com/Celemony/ARA_SDK.git
    GIT_TAG f39ed547148ba9823d55589d642f42c42f46a3f8
    SOURCE_SUBDIR ARA_Library
)

FetchContent_MakeAvailable(arasdk)
