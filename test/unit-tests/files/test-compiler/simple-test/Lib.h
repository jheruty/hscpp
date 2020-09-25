#ifdef _WIN32

#define HSCPP_API __declspec(dllexport)

#else

#define HSCPP_API

#endif

extern "C"
{
    HSCPP_API void SetValueTo12(int &val);
}