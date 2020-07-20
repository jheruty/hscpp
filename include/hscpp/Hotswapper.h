#pragma once

#include <string>
#include <vector>

#include "hscpp/FileWatcher.h"
#include "hscpp/Compiler.h"

namespace hscpp
{
	class Hotswapper
	{
	public:
		void AddIncludeDirectory(const std::string& directory);
		void AddSourceDirectory(const std::string& directory, bool bRecursive);

		void Update();
	private:
		FileWatcher m_FileWatcher;
		std::vector<FileWatcher::Event> m_FileEvents;

		Compiler m_Compiler;

		std::vector<std::string> m_IncludeDirectories;

		std::vector<std::string> GetChangedFiles();
	};
}