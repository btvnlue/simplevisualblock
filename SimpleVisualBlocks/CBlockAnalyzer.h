#pragma once
#include "CNodes.h"

#include <list>

class VisualBlock;

class CBlockAnalyzer
{
	FileNode* procnode;
	int SmallSizeLimit;
public:
	CBlockAnalyzer();
	BOOL CanSplit(VisualBlock* block);
	int SplitRect(RECT& rct, double ratio, RECT& lrct, RECT& rrct);
	int SplitNodesRect(VisualBlock* ori, VisualBlock* left, VisualBlock* right);
	int AnalyzeBlock(VisualBlock* blk);
	VisualBlock* AnalyzeFileNode(FileNode* node, RECT rct);
	int GetEdgeList(VisualBlock* block, std::list<VisualBlock*>& edglist);
	int GetNodeList(VisualBlock* block, std::list<VisualBlock*>& edglist);
	BOOL NeedRedraw(int nodeupd, FileNode* nsel, BOOL resized);
};

