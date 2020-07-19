#include <iostream>
#include <thread>
#include <chrono>

#include "Hotswapper.h"
#include "FileWatcher.h"

int main() 
{
	hscpp::Hotswapper swapper;

	hscpp::FileWatcher watcher;
	watcher.AddDirectory("C:\\Users\\jheru\\Documents", false);
	watcher.AddDirectory("./src", true);

	std::vector<std::string> files;
	while (true)
	{
		watcher.PollChanges(files);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	return 0;
}