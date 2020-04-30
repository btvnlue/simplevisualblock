#pragma once

#include <string>

#include "CNodes.h"

class CPathAnalyzer
{
public :
	FileNode* root;
	CThreadPool* pool;

	//int StartPath(const std::wstring& path);
	int StartPathDist(const std::wstring& path);
	//int LoadDirectory(FileNode* nodedir);
	int Stop();
	int GetTasksCount();
};

