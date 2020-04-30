#pragma once

#include <Windows.h>
#include <map>
#include <list>

#define NMC_VISUALBLOCK_HOVERNODE 0x5546
#define NMC_VISUALBLOCK_SELECTNODE 0x5547
#define NMC_VISUALBLOCK_LOGCLICKNODE 0x5548

class VisualBlock;
class FileNode;

struct NMVISUALBLOCK {
	NMHDR nmhdr;
	FileNode* node;
};

class CVisualBlockView
{
	static ATOM classBlock;
	static std::map<HWND, CVisualBlockView*> views;
	std::list<VisualBlock*> viewlist;
	std::map<FileNode*, VisualBlock*> viewnodeblockmap;
	std::map<FileNode*, VisualBlock*> allnodeblockmap;

	CVisualBlockView();
	int BuildColorIndex();
	static LRESULT CALLBACK WndBlockProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static ATOM RegisterBlockClass(HINSTANCE hInstance);

	int SendNotifyLogClickNode(FileNode* node);

	int SendNotifySelectNode(FileNode* node);
	int SendNotifyHoverNode(FileNode* node);
	HWND hWnd;
	HWND hToolTip;
	HANDLE hEventView;
	BOOL resized;
	VisualBlock* inBlock;
	VisualBlock* selectBlock;
	FileNode* selectNode;
	HBRUSH hBlackBrush;
	HBRUSH hGrayBrush;
	HBRUSH hBackBrush;
	HBRUSH hSelectBrush;

	DWORD *colors;
	int colorsCount;
public:
	virtual ~CVisualBlockView();
	HWND InitiateToolTips(HWND hPnt);
	static CVisualBlockView* CreateVisualBlockView(HWND parent);
	operator HWND() const;
	RECT GetRect();
	int Redraw();
	int SetViewList(std::list<VisualBlock*>& vlst);
	int CleanUpViewBlockMap();
	int SetNodeList(std::list<VisualBlock*>& nlst);
	int SetSelectNode(FileNode* node);
	int Clear();
	int UpdateColors(int* extidx, int* extcnt, DWORD* extcolors, int cnt);
	BOOL ResetResized();
};

