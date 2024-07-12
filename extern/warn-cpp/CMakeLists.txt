cmake_minimum_required(VERSION 3.20.0)

project("Warn CPP"
    VERSION 2024.07
    LANGUAGES CXX
)

# Include files here in order to bring functions/macros into scope
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(MAM_TargetCompileWarnings)

add_library(warn-cpp INTERFACE)

target_include_directories(warn-cpp
    INTERFACE
        include
)

target_compile_features(warn-cpp 
    INTERFACE 
        cxx_std_17
)
