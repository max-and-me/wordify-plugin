cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    wave-draw
    GIT_REPOSITORY "https://github.com/max-and-me/wave-draw.git"
    GIT_TAG main
)

FetchContent_MakeAvailable(wave-draw)
