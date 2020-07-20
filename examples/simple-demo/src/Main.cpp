#include <iostream>
#include <thread>
#include <chrono>

#include "hscpp/Hotswapper.h"
#include "hscpp/FileWatcher.h"

int main() 
{
	hscpp::Hotswapper swapper;

	swapper.AddIncludeDirectory("./include");
	swapper.AddIncludeDirectory("../../include");
	swapper.AddSourceDirectory("./src", true);

	while (true)
	{
		swapper.Update();
	}

	return 0;
}