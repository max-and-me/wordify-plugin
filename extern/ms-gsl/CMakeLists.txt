
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    ms-gsl
    GIT_REPOSITORY https://github.com/microsoft/GSL.git
    GIT_TAG v4.2.0
)

FetchContent_MakeAvailable(ms-gsl)
