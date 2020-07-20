#include <stdio.h>
#include <stdarg.h>

#include "hscpp/Log.h"

namespace hscpp
{
	hscpp::LogLevel Log::s_Level = LogLevel::Debug;

	void Log::SetLogLevel(LogLevel level)
	{
		s_Level = level;
	}

	void Log::Write(LogLevel level, const char* fmt, ...)
	{
		if (static_cast<int32_t>(level) >= static_cast<int32_t>(s_Level))
		{
			va_list args;
			va_start(args, fmt);
			vfprintf(stdout, fmt, args);
			va_end(args);
		}
	}
}

