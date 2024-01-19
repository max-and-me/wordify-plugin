//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

// https://en.cppreference.com/w/cpp/filesystem/temp_directory_path

#include <filesystem>
#include <iostream>
namespace fs = std::filesystem;

int main()
{
    std::cout << "Temp directory is " << fs::temp_directory_path() << '\n';
}