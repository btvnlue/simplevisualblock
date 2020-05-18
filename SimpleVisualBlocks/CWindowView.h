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

typedef struct UIAction
{
	enum ACODE {
		UI_NONE
		, UI_ADD_NODE
		, UI_ADD_CHILD
		, UI_DEL_CHILD
		, UI_COLLS_NODE
		, UI_EXPAND_NODE
		, UI_OPEN_FILENODE
		, UI_UPDATE_FILENODES
	} action;
	FileNode* node;
	HTREEITEM hItem;
} UIACTION, *PUIACTION;

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

	std::list<UIAction> actions;
	HANDLE hAction;
public:
	static std::map<HWND, CWindowView*> views;
	static CWindowView* GetViewInstance(HINSTANCE hinst);
	static int WaitAllWindows();
	static DWORD WINAPI ThDisplayTree(_In_ LPVOID lpParameter);
	static DWORD WINAPI ThNewWindow(_In_ LPVOID lpParameter);
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
	int UpdateFileNodes_(FileNode* node);
	int InsertNodeInList(FileNode* node, int idx);
	int UpdateFileNodes(FileNode* node);
	int DispListNodeItem(int itemindex, FileNode* node, DWORD cat);
	LRESULT ProcNotifyTree(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static int CALLBACK CompFileNodeList(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	HTREEITEM AddNodeItem(HTREEITEM hpnt, FileNode* node);
	int AddNodeListItem(HTREEITEM hpnt, std::list<FileNode*>& nls);
//	int GetTreeNodeChildren(HTREEITEM hnode, std::list<HTREEITEM>& clist);
//	int UpdateNodeData(FileNode* node);
//	int DisplayNode(HTREEITEM hpnt, FileNode* node, BOOL dochild);
	int UpdateNodeColor(FileNode* node);
	int UpdateDisplayNode(FileNode* cnd);
	static DWORD WINAPI ThDisplayBlock(_In_ LPVOID lpParameter);
	int UpdateTaskbar(FileNode* und);
	int ProcActions();
	int ProcCommandStopTh();
	int ChangeWindowTitle(const std::wstring& procpath);
	int ProcCommandStart(const WCHAR* path);
	BOOL ProcCommandDrives(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT ProcSelectDirectory(HWND hPnt);
	LRESULT ProcCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	int DispListNode(FileNode* node, DWORD cat);
	int PutAction(UIAction::ACODE act, FileNode* cnd);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	int ProcContextMenu(int xx, int yy);
	int ProcContextMenuTree(int xx, int yy);
	static ATOM MyRegisterClass(HINSTANCE hInstance);
	int ProcCommandStop();
	int SelectTreeNode(FileNode* node);
};

