#pragma once

#include <vector>
#include <string>

#include "hscpp/CmdShell.h"

namespace hscpp
{
	class Compiler
	{
	public:
		Compiler();
		
		void Compile(const std::vector<std::string>& files, const std::vector<std::string>& includeDirectories);
		void Update();

	private:
		std::string GetVisualStudioPath();

		CmdShell m_CmdShell;
	};
}