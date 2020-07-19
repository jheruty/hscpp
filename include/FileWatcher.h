#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <memory>

namespace hscpp
{
	enum class FileEventType
	{
		New,
		Delete,
		Rename,
	};

	struct FileEvent
	{
		FileEventType type;
		std::string filename;
	};

	class FileWatcher
	{
	public:
		bool AddDirectory(const std::string& directory, bool recursive);
		void PollChanges(std::vector<std::string>& changedFiles);

	private:
		struct DirectoryWatch
		{
			// Allows casting of an OVERLAPPED struct to a DirectoryWatch safely.
			OVERLAPPED overlapped = {0};
			alignas(sizeof(DWORD)) uint8_t buffer[32 * 1024] = { 0 };

			HANDLE hDirectory = INVALID_HANDLE_VALUE;
			bool recursive = false;
		};

		static void WINAPI WatchCallback(DWORD dwErrorCode, DWORD dwNumberOfBytesTransferred, LPOVERLAPPED lpOverlapped);
		static bool ReadDirectoryChangesAsync(DirectoryWatch* pWatch);

		std::vector<std::unique_ptr<DirectoryWatch>> m_Watchers;
		std::vector<HANDLE> m_DirHandles;

		std::vector<std::string> m_ChangedFiles;
	};
}