#include "FileWatcher.h"
#include "Log.h"
#include "StringUtil.h"

namespace hscpp
{

	bool FileWatcher::AddDirectory(const std::string& directory, bool recursive)
	{
		auto pWatch = std::make_unique<DirectoryWatch>();

		// FILE_FLAG_BACKUP_SEMANTICS is necessary to open a directory.
		HANDLE hDirectory = CreateFileA(
			directory.c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			NULL);

		if (hDirectory == INVALID_HANDLE_VALUE)
		{
			Log::Write(LogLevel::Error, "%s: Failed to add directory '%s' to watch. [%s]",
				__func__, directory.c_str(), GetLastErrorString().c_str());
			return false;
		}

		pWatch->hDirectory = hDirectory;
		pWatch->recursive = recursive;

		if (!ReadDirectoryChangesAsync(pWatch.get()))
		{
			Log::Write(LogLevel::Error, "%s: Failed initial call to ReadDirectoryChangesW. [%s]",
				__func__, GetLastErrorString().c_str());
			return false;
		}

		m_DirHandles.push_back(hDirectory);
		m_Watchers.push_back(std::move(pWatch));

		return true;
	}

	void FileWatcher::PollChanges(std::vector<std::string>& changedFiles)
	{
		WaitForMultipleObjectsEx(m_DirHandles.size(), m_DirHandles.data(), false, 0, true);

		changedFiles = m_ChangedFiles;
		m_ChangedFiles.clear();
	}

	void WINAPI FileWatcher::WatchCallback(DWORD dwErrorCode,
		DWORD dwNumberOfBytesTransferred, LPOVERLAPPED lpOverlapped)
	{
		if (dwErrorCode != ERROR_SUCCESS)
		{
			Log::Write(LogLevel::Error, "%s: ReadDirectoryChangesW failed. [%s]",
				__func__, GetErrorString(dwErrorCode).c_str());
			return;
		}

		DirectoryWatch* pWatch = reinterpret_cast<DirectoryWatch*>(lpOverlapped);

		if (!ReadDirectoryChangesAsync(pWatch))
		{
			Log::Write(LogLevel::Error, "%s: Failed refresh call to ReadDirectoryChangesW. [%s]",
				__func__, GetLastErrorString().c_str());
			return;
		}
	}

	bool FileWatcher::ReadDirectoryChangesAsync(DirectoryWatch* pWatch)
	{
		return ReadDirectoryChangesW(
			pWatch->hDirectory,
			pWatch->buffer,
			sizeof(pWatch->buffer),
			pWatch->recursive,
			FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SIZE,
			NULL,
			&pWatch->overlapped,
			WatchCallback) != 0;
	}

}
