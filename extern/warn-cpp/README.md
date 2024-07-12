# Warn CPP

## Introduction

This library contains a ```CMake``` function and a C++ header to better deal with wanings in your projects.

## Getting Started

### Include into your project

You can include the library by simple using ```FetchContent``` as follows:

```cmake
...
FetchContent_Declare(
    warn-cpp
    GIT_REPOSITORY "https://github.com/max-and-me/warn-cpp.git"
    GIT_TAG main
)

FetchContent_MakeAvailable(warn-cpp)
...
```

Afterwards you can apply the warnings to any of your targets:

```cmake
...
target_link_libraries(MyTarget
    PRIVATE
        warn-cpp
)

mam_target_compile_warnings(MyTarget)
...
```

> Pass multiple targets like this: ```mam_target_compile_warnings(MyTarget MyLib1 MyLib2)```

### Suppress warnings

Whenever an external library causes warnings by including one of its headers you can simply disable them:

```cpp
#include "my_header.h"
#include "warn_cpp/suppress_warnings.h"
BEGIN_SUPPRESS_WARNINGS
#include "extern/lib1/header.h"
#include "extern/lib2/header.h"
#include "extern/lib3/header.h"
END_SUPPRESS_WARNINGS
```

## Gettings Help

### Further links

* https://github.com/cpp-best-practices/cppbestpractices/blob/master/02-Use_the_Tools_Available.md
* https://github.com/holoplot/shield.git
 * https://www.fluentcpp.com/2019/08/30/how-to-disable-a-warning-in-cpp/
 * https://learn.microsoft.com/de-de/cpp/build/reference/external-external-headers-diagnostics?view=msvc-170
 * https://stackoverflow.com/questions/28166565/detect-gcc-as-opposed-to-msvc-clang-with-macro
 * https://stackoverflow.com/questions/13826722/how-do-i-define-a-macro-with-multiple-pragmas-for-clang
 