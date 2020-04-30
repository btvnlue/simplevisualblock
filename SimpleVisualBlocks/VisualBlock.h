#pragma once

#include <list>
#include <Windows.h>

class FileNode;

class VisualBlock {
public:
	virtual ~VisualBlock();
	RECT rect;
	std::list<VisualBlock*> blocks;
	std::list<FileNode*> nodes;
};
