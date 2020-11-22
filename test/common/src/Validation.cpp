#include "common/Validation.h"

namespace hscpp { namespace test
{

        void ValidateError(const LangError& result,
            const LangError::Code& expectedCode, size_t expectedLine, const std::vector<std::string>& expectedArgs)
        {
            REQUIRE(result.ErrorCode() == expectedCode);
            REQUIRE(result.Line() == expectedLine);
            REQUIRE(result.NumArgs() == expectedArgs.size());

            // $1, $2... etc are interpolated arguments. Validate they are fully replaced.
            REQUIRE(result.ToString().find("$") == std::string::npos);

            for (size_t i = 0; i < result.NumArgs(); ++i)
            {
                REQUIRE(result.GetArg(i) == expectedArgs.at(i));
            }
        }

}}