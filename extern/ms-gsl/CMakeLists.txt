
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    ms-gsl
    GIT_REPOSITORY https://github.com/microsoft/GSL.git
    GIT_TAG b39e7e4b0987859f5b19ff7686b149c916588658
)

FetchContent_MakeAvailable(ms-gsl)
