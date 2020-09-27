#include "hscpp/module/ModuleInterface.h"

extern "C"
{

    // Most module files are header-only, to avoid needing to force include them during compilation.
    // Unfortunately, to export a function successfully from a shared object, we need to create a
    // definition in a translation unit (inline exports only works on Windows).
    //
    // A future trick might be able to remove this.
    hscpp::ModuleInterface *Hscpp_GetModuleInterface()
    {
        static hscpp::ModuleInterface moduleInterface;
        return &moduleInterface;
    }

}
