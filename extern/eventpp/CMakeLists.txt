
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    eventpp
    GIT_REPOSITORY https://github.com/wqking/eventpp.git
    GIT_TAG c472fb22e71ead0e58ff7df89e12c66b0bdfb533
)

FetchContent_MakeAvailable(eventpp)
