#pragma once

#include <iostream>

#define LOG_FAIL(msg) std::cout << "[FAIL]: " << msg << std::endl
#define LOG_LOADED(msg) std::cout << "[LOADED]: " << msg << std::endl;
#define LOG_DONE(msg) std::cout << "[DONE]: " << msg << std::endl
#define LOG_INFO(msg) std::cout << "[INFO]: " << msg << std::endl
#define LOG_RESULT(msg) std::cout << "[RESULT]: " << msg << std::endl