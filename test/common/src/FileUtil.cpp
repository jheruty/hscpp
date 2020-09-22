
#include "common/FileUtil.h"

namespace hscpp { namespace test {

    fs::path test::RootTestDirectory()
    {
        return fs::path(__FILE__).parent_path() / ".." / "..";
    }

}}