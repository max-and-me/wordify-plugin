// Copyright(c) 2024 Max And Me.

#include "whipser_cpp_wrapper.h"
#include "base/source/fdebug.h"
#include "hao/special_folders/special_folders.h"
#include "whereami.h"
#include "wordify_defines.h"
#include <filesystem>
#include <string>

namespace mam::whisper_cpp {

//------------------------------------------------------------------------
namespace {
using StringType = std::string;

//------------------------------------------------------------------------
auto get_ggml_file_path(const StringType& company_name,
                        const StringType& plugin_name) -> PathType
{
#if 0
    (void)company_name;
    (void)plugin_name;
    std::filesystem::path file_path(MAM_WHISPER_CPP_MODEL_DOWNLOAD_DIR);
    file_path /= "ggml-base.en.bin";
#else
    const auto model_path_str = hao::special_folders::get_application_data(
        hao::special_folders::Domain::kLocal);

    std::filesystem::path file_path(model_path_str);
    file_path /= company_name;
    file_path /= plugin_name;
    file_path /= MAM_GGML_DIRECTORY_NAME;
    file_path /= "ggml-medium.bin";
#endif
    return file_path.string();
}

//------------------------------------------------------------------------
} // namespace

//------------------------------------------------------------------------
auto get_worker_executable_path() -> PathType
{
#if 0
    return MAM_WHISPER_CPP_EXECUTABLE;
#else
    const int length = wai_getModulePath(NULL, 0, NULL);
    const auto cpath = (char*)malloc(length + 1);
    wai_getModulePath(cpath, length, NULL);
    cpath[length] = '\0';

    std::filesystem::path fs_path(cpath);
    free(cpath);

    fs_path.replace_filename(MAM_WHISPER_CPP_EXECUTABLE_NAME);
    return fs_path.string();
#endif
}

//------------------------------------------------------------------------
auto get_ggml_file_path() -> PathType
{
    return get_ggml_file_path(COMPANY_NAME_STR, PLUGIN_NAME_STR);
}

//------------------------------------------------------------------------

} // namespace mam::whisper_cpp
