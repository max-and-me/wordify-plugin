
cmake_minimum_required(VERSION 3.14.0)

include(FetchContent)

FetchContent_Declare(
    meta-words
    GIT_REPOSITORY "https://github.com/max-and-me/meta-words.git"
    GIT_TAG 3037f71a4799c47edb834a8a88ee89a9d1427ca4
)

FetchContent_MakeAvailable(meta-words)

# Only download if not exist already
set(MAM_WHISPER_CPP_MODEL_FILE_NAME "ggml-medium.bin")
if(EXISTS "${MAM_WHISPER_CPP_MODEL_DOWNLOAD_DIR}/${MAM_WHISPER_CPP_MODEL_FILE_NAME}")
    return()
endif()
message(STATUS "[MAM] Downloading model file ${MAM_WHISPER_CPP_MODEL_FILE_NAME}...")
file(DOWNLOAD 
    https://huggingface.co/ggerganov/whisper.cpp/resolve/main/${MAM_WHISPER_CPP_MODEL_FILE_NAME}
    ${MAM_WHISPER_CPP_MODEL_DOWNLOAD_DIR}/${MAM_WHISPER_CPP_MODEL_FILE_NAME}
    SHOW_PROGRESS
)
