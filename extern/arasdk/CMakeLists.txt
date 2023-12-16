
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    arasdk
    GIT_REPOSITORY https://github.com/Celemony/ARA_SDK.git
    GIT_TAG 53fc2b887abed5ddb6047b1ad7acee60604c6489
    SOURCE_SUBDIR ARA_Library
)

FetchContent_MakeAvailable(arasdk)
