
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    nlohmann-json
    GIT_REPOSITORY "https://github.com/nlohmann/json.git"
    GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(nlohmann-json)
