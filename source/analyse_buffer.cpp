#include "analyse_buffer.h"
#include "process.hpp"
#include <vector>
#include <string>


using namespace TinyProcessLib;
using namespace std;

namespace mam {
bool analyse_buffer (float** buffer)
{
    Process process1b(vector<string>{"/bin/echo", "Hello", "World"}, "", [](const char *bytes, size_t n) {
        //cout << "Output from stdout: " << string(bytes, n);
    });
    auto exit_status = process1b.get_exit_status();
    
    return false;
}
}//mam
