
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    meta-words
    GIT_REPOSITORY "https://github.com/max-and-me/meta-words.git"
    GIT_TAG cfdbd41d85634da962503bf14d60c601fa664a88
)

FetchContent_MakeAvailable(meta-words)
