#include <iostream>
#include <thread>
#include <chrono>

#include "Hotswapper.h"
#include "FileWatcher.h"

int main() 
{
	hscpp::Hotswapper swapper;

	hscpp::FileWatcher watcher;
	watcher.AddWatch("C:\\Users\\jheru\\Documents", false);
	watcher.AddWatch("./src", true);

	std::vector<hscpp::FileWatcher::Event> files;

	while (true)
	{
		watcher.PollChanges(files);

		for (auto file : files)
		{
			std::cout << file.FullPath() << (int)file.type << std::endl;
		}
	}

	watcher.ClearAllWatches();

	return 0;
}