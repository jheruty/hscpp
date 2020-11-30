# Preprocessor requires

## What is the hscpp preprocessor?

The hscpp preprocessor runs before the hscpp compiler, and is used to gather additional dependencies that are needed to build individual source files.

When a file is saved, the hscpp preprocessor will parse the file, and look for `hscpp_require` macros. The contents of these macros determine which dependencies get added.

## Enabling the preprocessor

The hscpp preprocessor is disabled by default, and must be manually turned on:
```cpp
#include "hscpp/Hotswapper.h"

int main()
{
    hscpp::Hotswapper swapper;
    swapper.EnableFeature(hscpp::Feature::Preprocessor);
}
```

## Require macros
- `hscpp_require_source`
    - Add additional source files to compilation list. The preprocessor will recursively preprocess any additional sources added here.
- `hscpp_require_include_dir`
    - Add additional include directories.
- `hscpp_require_library`
    - Add additional libraries.
- `hscpp_require_library_dir`
    - Add additional library directories.
- `hscpp_require_preprocessor_def`
    - Add additional preprocessor definitions.

All macros accept a comma separated list of strings. `hscpp_require_preprocessor_def` additionally accepts C++ identifiers. For example:
```cpp
hscpp_require_source("./relative_path/to/file.cpp", "/absolute/path/to/file.cpp");
hscpp_require_preprocessor_def("DEFINITION_1", DEFINITION_2);
```

hscpp_require macros can be placed anywhere in a source files. It is important to note that hscpp_macros are completely separate from the C++ preprocessor, and so one cannot `#ifdef` them out.

See the [runtime-require-demo](../examples/runtime-require-demo) for a usage example.

[Next we will see how to conditionally choose hscpp_require macros based on project configuration.](./8_preprocessor-language.md)