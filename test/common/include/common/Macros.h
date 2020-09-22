#pragma once

#define CALL(func, ...)\
{ /* Create scope for INFO */ \
    INFO("Calling " << #func << " on line " << __LINE__ << ".");\
    func(__VA_ARGS__);\
}\
