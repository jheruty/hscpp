#include "hscpp/Compiler.h"
#include "hscpp/Log.h"

namespace hscpp
{

	void Compiler::Compile(const std::vector<std::string>& files)
	{
		for (auto file : files)
		{
			Log::Write(LogLevel::Debug, "%s\n", file.c_str());
		}
	}

}