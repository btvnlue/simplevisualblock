#pragma once

#include <string>

#include "CNodes.h"

class CPathAnalyzer
{
public :
	CPathAnalyzer();
	virtual ~CPathAnalyzer();

	FileNode* root;
	CThreadPool* pool;
	BOOL keeprun;
	HANDLE hThUpdate;

	static DWORD WINAPI ThUpdateTree(LPVOID lpParameter);

	//int StartPath(const std::wstring& path);
	int StartPathDist(const std::wstring& path);
	//int LoadDirectory(FileNode* nodedir);
	int Stop();
	int GetTasksCount();
	int Pause();
	int Resume();
};

