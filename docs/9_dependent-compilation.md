# Dependent compilation

## Concept

Earlier, we saw how only classes that have `HSCPP_TRACK` can be hot-swapped. However, it is common for software to have components that do not fit neatly into this paradigm. For example, consider a helper math library, which consists of many free floating functions. How could such a library be modified at runtime?

The answer is to keep track of all *dependents* of this math library, and force them to recompile whenever the math library changes.

For example, consider two objects that use `HSCPP_TRACK`: HotSwap1 and HotSwap2. Both libraries depend on our hypothetical HelperMath library. If HelperMath's code is changed, hscpp could force a recompilation of all of HelperMath's code, as well as HotSwap1 and HotSwap2.

Since all references to HelperMath will have been updated to use the new code, the hot-swap will be successful.

## Enabling dependent compilation

Like the preprocessor, dependent compilation must be explicitly enabled. Enabling dependent compilation will implicitly enable the preprocessor.

```cpp
...
#include "hscpp/Feature.h"

int main()
{
    hscpp::Hotswapper swapper;
    swapper.EnableFeature(hscpp::Feature::DependentCompilation);
}
```

## hscpp modules

To use dependent compilation, non-tracked code must be added to an `hscpp_module`. For example, assume the helper math library consists of Math.h, Math.cpp, and MathDefs.cpp. Within each file, one should add:
```cpp
hscpp_module("HelperMath");
```

The name of the module is arbitrary, as long as all files in the module use the same name. This tells hscpp several things:
- Compiling any file from Math.h, Math.cpp, or MathDefs.cpp should trigger a compilation of all three files in the HelperMath module.
- Any file that `#include "Math.h"` is considered a module dependent, and thus any change to Math.h, Math.cpp, or MathDefs.cpp should also compile that file.
- Because any file that `#include "Math.h"` is a HelperMath module dependent, recompiling that file will also add Math.cpp and MathDefs.cpp to the build.

It is possible for a file to be part of multiple modules, and for modules to depend on other modules.

## Interaction with hscpp_if statements.

Both hscpp_modules and `#include` statements respect `hscpp_if` statements. For example, if a certain header file is not valid in a particular configuration, one can wrap it in an `hscpp_if` to conditionally exclude it from the dependency graph.

## Experimental status

Dependent compilation is currently very experimental. However, a working proof-of-concept can be found in the [dependent-compilation-demo.](../examples/dependent-compilation-demo)