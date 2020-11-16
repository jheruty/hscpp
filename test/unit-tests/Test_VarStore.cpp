#include "catch/catch.hpp"

#include "hscpp/preprocessor/VarStore.h"

namespace hscpp { namespace test
{

    TEST_CASE("VarStore can store and interpolate variables.")
    {
        VarStore store;

        store.SetVar("Var1", Variant("Var1_Value"));
        store.SetVar("TheSecond Variable", Variant("Var2's !@$Value"));

        std::string str = "Our values are ${Var1} and ${   TheSecond Variable}, ${Third   } does not exist.";
        std::string origStr = str;
        std::string interpolatedStr = store.Interpolate(str);

        REQUIRE(str == origStr);
        REQUIRE(interpolatedStr == "Our values are Var1_Value and Var2's !@$Value, ${Third   } does not exist.");

        store.RemoveVar("TheSecond Variable");
        interpolatedStr = store.Interpolate(str);

        REQUIRE(interpolatedStr == "Our values are Var1_Value and ${   TheSecond Variable}, ${Third   } does not exist.");

        store.SetVar("Var2", Variant("Var2_Value"));
        str = "Interpolate multiple times ${ Var1} and ${ Var1 } and ${Var2 } and ${   Var1   }";

        interpolatedStr = store.Interpolate(str);

        REQUIRE(interpolatedStr == "Interpolate multiple times Var1_Value and Var1_Value and Var2_Value and Var1_Value");

        store.SetVar(" SpacesAreTrimmed ", Variant("217"));
        str = "Padded variables are trimmed ${SpacesAreTrimmed} ${ SpacesAreTrimmed }";

        interpolatedStr = store.Interpolate(str);

        REQUIRE(interpolatedStr == "Padded variables are trimmed 217 217");
    }

    TEST_CASE("VarStore can handle edge cases.")
    {
        VarStore store;

        std::string str = "{}{}{}{}{}";
        REQUIRE(store.Interpolate(str) == "{}{}{}{}{}");

        store.SetVar("Var", Variant("Val"));
        str = "${                     Var";
        REQUIRE(store.Interpolate(str) == "${                     Var");

        str = "                 Var            }";
        REQUIRE(store.Interpolate(str) == "                 Var            }");

        str = "${                         Var                                 }";
        REQUIRE(store.Interpolate(str) == "Val");

        str = "${ ${Var}  }";
        REQUIRE(store.Interpolate(str) == "${ Val  }");
    }

    TEST_CASE("VarStore can interpolate Number and Bool variants.")
    {
        VarStore store;

        std::string str = "Bool: ${ Bool }, Number: ${ Number }";
        store.SetVar("Bool", Variant(true));
        store.SetVar("Number", Variant(100.0));

        REQUIRE(store.Interpolate(str) == "Bool: true, Number: 100");

        store.SetVar("Number", Variant(0.5));
        REQUIRE(store.Interpolate(str) == "Bool: true, Number: 0.5");
    }

}}