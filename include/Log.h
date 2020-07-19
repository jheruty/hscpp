#pragma once

#include <string>

namespace hscpp
{
	enum class LogLevel
	{
		Trace,
		Debug,
		Info,
		Error,
	};

	class Log
	{
	public:
		static void SetLogLevel(LogLevel level);
		static void Write(LogLevel level, const char* fmt, ...);

	private:
		static LogLevel s_Level;
	};
}

