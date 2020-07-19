#pragma once

#include <Windows.h>
#include <string>

namespace hscpp
{
	std::string GetErrorString(DWORD error);
	std::string GetLastErrorString();
}