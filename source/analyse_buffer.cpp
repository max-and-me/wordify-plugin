//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#include "analyse_buffer.h"
#include "process.hpp"
#include <string>
#include <vector>

using namespace TinyProcessLib;
using namespace std;

namespace mam {

//------------------------------------------------------------------------
bool analyse_buffer(float** buffer)
{
#ifdef _WIN32
    const auto arguments = std::vector<TinyProcessLib::Process::string_type>{
        L"/bin/echo", L"Hello", L"World"};
    const auto path = L"";
#else
    const auto arguments = std::vector<TinyProcessLib::Process::string_type>{
        "/bin/echo", "Hello", "World"};
    const auto path = "";
#endif
    TinyProcessLib::Process process1b(arguments, path,
                                      [](const char* bytes, size_t n) {
                                          // cout << "Output from stdout: " <<
                                          // string(bytes, n);
                                      });
    auto exit_status = process1b.get_exit_status();

    return false;
}

//------------------------------------------------------------------------
} // namespace mam
