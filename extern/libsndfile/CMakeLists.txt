
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    libsndfile
    GIT_REPOSITORY https://github.com/libsndfile/libsndfile.git
    GIT_TAG d9a35ea0d5c64c19dd635ae578e0028df8f66d6a
)

option (ENABLE_EXTERNAL_LIBS "Enable FLAC, Vorbis, and Opus codecs" OFF)
option (ENABLE_MPEG "Enable MPEG codecs" OFF)
option (BUILD_PROGRAMS "Build programs" OFF)
option (BUILD_EXAMPLES "Build examples" OFF)
option (ENABLE_CPACK "Enable CPack support" OFF)
set(BUILD_TESTING OFF)
FetchContent_MakeAvailable(libsndfile)
