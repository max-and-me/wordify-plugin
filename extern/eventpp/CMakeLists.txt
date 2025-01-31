
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    eventpp
    GIT_REPOSITORY https://github.com/wqking/eventpp.git
    GIT_TAG 1224dd6c9bd4577d686ac42334fc545997f5ece1
)

FetchContent_MakeAvailable(eventpp)
