#ifdef _WIN32

#define HSCPP_API __declspec(dllexport)

#else

#define HSCPP_API __attribute__ ((visibility ("default")))

#endif

extern "C"
{
    HSCPP_API void SetValueTo12(int &val);
}