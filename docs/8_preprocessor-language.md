# Preprocessor configuration language

## if/else statements

As noted in the [previous](./7_preprocessor-requires.md) section, hscpp_require macros do not respect `#ifdef` blocks, because they are completely separate from C++ compilation. If different hscpp_require macros should be selected depending on the project configuration, one can use the hscpp preprocessor's configuration language.

This is accomplished by if/else statements, similar to many programming languages:

```cpp
hscpp_if(stringVar == "Value");
    hscpp_require_source("source1");
hscpp_elif(numberVar > 1.0);
    hscpp_require_source("source2");
hscpp_elif(boolVar);
    hscpp_require_source("source3");
hscpp_else();
    hscpp_message("error"); // Print a debug message to the console.
hscpp_end(); // Needed to end a scope.
```

Expressions within `hscpp_if`, and `hscpp_elif` are allowed to reference variables. These variables are provided to the `Hotswapper`, and may be of type String, Number, or Bool.

For example:
```cpp
swapper.SetVar("stringVar", "Value");
swapper.SetVar("numberVar", 0.95);
swapper.SetVar("boolVar", true);
```

The following operations are supported:
- Comparison *(valid on all types)*
    - ==, !=
- Comparison *(only valid on Number)*
    - <
    - <=
    - \>
    - \>=
- Negation *(valid on all types)*
    - !
- Arithmetic *(only valid on Number)*
    - \+
    - \- *(both binary and unary)*
    - \*
    - /
- Logical
    - &&, ||

Note that it is impossible to cast from one type to another, and all comparisons must be between two variables of the same type.

For truthiness:
- Empty String is false, anything else is true.
- Number 0 is false, anything else is true.
- Bool is either true or false.

## Return statement

The `hscpp_return` statement can be used to immediately stop the Preprocessor. Statements that have already been evaluated will be processed as normal.

For example:

```cpp
hscpp_require_source("source1")

hscpp_if (var)
    hscpp_return()
hscpp_end()

hscpp_require_source("source2")
```

In the above, if `var` is truthy, only source1 will be added to the compilation list.

## String interpolation

Variables can also be interpolated within hscpp_require macros, using `${VarName}`.

For example:
> Main.cpp
```cpp
swapper.AddVar("rootDirectory", "some/path/");
```
> HotSwapObject.cpp
```cpp
hscpp_require_source("${rootDirectory}/to/file.cpp");
```

[Next lets look at dependent compilation.](./9_dependent-compilation.md)