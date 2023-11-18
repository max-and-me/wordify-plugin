
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    arasdk
    GIT_REPOSITORY https://github.com/Celemony/ARA_SDK.git
    GIT_TAG 8e893bfc7d6847c2100a47ad4314599994533458
    SOURCE_SUBDIR ARA_Library
)

FetchContent_MakeAvailable(arasdk)
