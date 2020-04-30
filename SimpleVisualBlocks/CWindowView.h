#pragma once

#include <Windows.h>
#include <map>
#include <list>
#include <string>
#include <CommCtrl.h>
#include <objbase.h>

#include "CVisualBlockView.h"

class FileNode;
class CPathAnalyzer;
class CBlockAnalyzer;
class CSplitWnd;

class CWindowView
{
	HWND hMain;
	HINSTANCE hInst;

	BOOL InitInstance(HINSTANCE hInstance, HWND hMain, int nCmdShow);

	CWindowView();
	int CreateWindowView(HINSTANCE hInst);
	ATOM viewClass;
	HBRUSH hbWhite;
	HBRUSH hbGreen;
	HBRUSH hbRed;
	WCHAR wbuf[1024];
	CVisualBlockView* blockView;
	int iUpdateForBlock;
	
	CSplitWnd* splitStatus;
	CSplitWnd* splitLog;
	CSplitWnd* splitVisual;
	HWND hLogList = NULL;
	HWND hNodeTree = NULL;
	HWND hFileList = NULL;
	std::map<FileNode*, HTREEITEM> mapNode;
	std::map<unsigned long, std::wstring> mapDrive;
	FileNode* selectednode = NULL;
	FileNode* viewnode = NULL;
	static std::list<HANDLE> windowThreads;
	std::wstring prevPath;
	IUnknown* pTaskbar;
	std::map<std::wstring, int> mapExtIndex;

public:
	static std::map<HWND, CWindowView*> views;
	static CWindowView* GetViewInstance(HINSTANCE hinst);
	static int WaitAllWindows();
	static DWORD WINAPI ThDisplayTree(_In_ LPVOID lpParameter);
	static DWORD WINAPI ThNewWindow(LPVOID lpParameter);
	int keeprun_;
	virtual ~CWindowView();

	CPathAnalyzer* ana;
	CBlockAnalyzer* bna;
	CSplitWnd* splitContent = NULL;
	HWND hStatus;
	HANDLE hDispTreeTh;
	HANDLE hDispBlockTh;

	int ProcessMessage();
	int Log(const WCHAR* msg);
	LRESULT ProcNotifyList(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT ProcNotifyVisual(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT ProcNotify(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	int InitialStatusBar(HWND hsts);
	int LoadLogicalDrives();
	int InitialMenu(HWND hwnd);
	int InitialLog();
	BOOL InitiateNodeTree(HWND hTree);
	BOOL InitialTaskbar(HWND hpnt);
	BOOL InitContentWindow(HINSTANCE hinst, HWND hpnt);
	int InsertNodeInList(FileNode* node);
	int UpdateFileNodes(FileNode* node);
	int DispListNodeItem(int itemindex, FileNode* node, DWORD cat);
	LRESULT ProcNotifyTree(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static int CALLBACK CompFileNodeList(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	HTREEITEM AddItemNode(HTREEITEM hpnt, FileNode* node);
	int GetTreeNodeChildren(HTREEITEM hnode, std::list<HTREEITEM>& clist);
	int UpdateNodeData(FileNode* node);
	int DisplayNode(HTREEITEM hpnt, FileNode* node, BOOL dochild);
	int UpdateNodeColor(FileNode* node);
	static DWORD WINAPI ThDisplayBlock(LPVOID lpParameter);
	int UpdateTaskbar(FileNode* und);
	int ProcCommandStopTh();
	int ChangeWindowTitle(const std::wstring& procpath);
	int ProcCommandStart(const WCHAR* path);
	BOOL ProcCommandDrives(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT ProcSelectDirectory(HWND hPnt);
	LRESULT ProcCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	int DispListNode(FileNode* node, DWORD cat);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static ATOM MyRegisterClass(HINSTANCE hInstance);
	int ProcCommandStop();
	int SelectTreeNode(FileNode* node);
};

