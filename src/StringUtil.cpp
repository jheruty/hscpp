#include <windows.h>

#include "StringUtil.h"

namespace hscpp
{

	std::string GetErrorString(DWORD error)
	{
		if (error == 0)
		{
			return ""; // No error.
		}

		LPSTR buffer = nullptr;
		size_t size = FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<LPSTR>(&buffer),
			0,
			NULL);

		std::string message(buffer, size);
		LocalFree(buffer);

		// Remove trailing '\r\n'. 
		if (message.size() >= 2
			&& message.at(message.size() - 1) == '\n'
			&& message.at(message.size() - 2) == '\r')
		{
			message = message.substr(0, message.size() - 2);
		}

		return message;
	}

	std::string GetLastErrorString()
	{
		return GetErrorString(GetLastError());
	}

}