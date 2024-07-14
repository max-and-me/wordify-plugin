cmake_minimum_required(VERSION 3.19.0)

option(BUILD_TESTING "" OFF)

add_subdirectory(arasdk)
add_subdirectory(eventpp)
add_subdirectory(fmt)
add_subdirectory(json)
add_subdirectory(libsamplerate)
add_subdirectory(libsndfile)
add_subdirectory(meta-words)
add_subdirectory(ms-gsl)
add_subdirectory(presonus-plugin-extensions)
add_subdirectory(vst3sdk)
add_subdirectory(wave-draw)
add_subdirectory(special-folders)
add_subdirectory(vst3-cpack)
add_subdirectory(warn-cpp)
add_subdirectory(whereami)


