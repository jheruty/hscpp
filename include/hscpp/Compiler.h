#pragma once

#include <vector>
#include <string>

namespace hscpp
{
	class Compiler
	{
	public:
		void Compile(const std::vector<std::string>& files);
	};
}