#include "CWindowView.h"
#include "Resource.h"
#include "CSplitWnd.h"
#include "CPathAnalyzer.h"
#include "CBlockAnalyzer.h"
#include "VisualBlock.h"

#include <windowsx.h>
#include <ShlObj.h>
#include <set>

#pragma comment(lib, "ComCtl32.lib")

#define MAX_LOADSTRING 100
#define WM_U_UPDATEFILENODES WM_USER + 0x11021
#define WM_U_DISPFILENODE WM_USER + 0x11022
#define WM_U_DISPFILENODE_TIMELY WM_USER + 0x11033
#define WM_U_HOVERNODE WM_USER + 0x11012
#define WM_U_SELECTNODE WM_USER + 0x2011
#define WM_U_DISPTASKCOUNT WM_USER + 0x1011
#define WM_U_SELECTTREENODE WM_USER + 0x1012
#define WM_U_OPENTREENODEINSHELL WM_USER + 0x987
#define WM_U_ZOOM_TO_TREENODE WM_USER + 0x989
#define WM_U_OPENNODEINSHELL WM_USER + 0x988
#define WM_U_OPENNODEFOLDERINSHELL WM_USER + 0x986
#define WM_U_DESTROY WM_USER + 0x1022
#define COMMAND_MENU_DRIVE_BASE 0x1234

WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

std::map<HWND, CWindowView*> CWindowView::views;

HTREEITEM CWindowView::AddNodeItem(HTREEITEM hpnt, FileNode* node)
{
	HTREEITEM hti = NULL;
	if (mapNode.find(node) == mapNode.end()) {
		TVINSERTSTRUCT tvi = { 0 };
		tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvi.item.cchTextMax = (int)wcslen(node->name);
		tvi.item.pszText = (LPWSTR)node->name;
		tvi.item.lParam = (LPARAM)node;
		tvi.item.iImage = node->type == FileNode::NT_FILE ? 4 : (node->type == FileNode::NT_DIR ? 0 : 2);
		tvi.item.iSelectedImage = tvi.item.iImage;
		tvi.hParent = hpnt;

		//HTREEITEM hti = (HTREEITEM)::SendMessage(hNodeTree, TVM_INSERTITEM, 0, (LPARAM)& tvi);
		hti = TreeView_InsertItem(hNodeTree, &tvi);
		if (hti) {
			mapNode[node] = hti;
		}
	}
	return hti;
}

int CWindowView::AddNodeListItem(HTREEITEM hpnt, std::list<FileNode*>& nls)
{
	FileNode* cnd;
	int icnt = 0;
	HTREEITEM hir;

	for (std::list<FileNode*>::iterator itnl = nls.begin()
		; itnl!=nls.end()
		; itnl++) {
		cnd = *itnl;
		hir = AddNodeItem(hpnt, cnd);
		icnt += hir ? 1 : 0;
	}

	if (icnt > 0) {
		RECT rct;
		*(HTREEITEM*)& rct = hpnt;

		SendMessage(hNodeTree, TVM_GETITEMRECT, FALSE, (LPARAM)& rct);
		::InvalidateRect(hNodeTree, &rct, TRUE);
	}
	return icnt;
}

//int CWindowView::GetTreeNodeChildren(HTREEITEM hnode, std::list<HTREEITEM>& clist)
//{
//	HTREEITEM hci = (HTREEITEM)::SendMessage(hNodeTree, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hnode);
//	while (hci)
//	{
//		clist.push_back(hci);
//
//		hci = (HTREEITEM)::SendMessage(hNodeTree, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hci);
//	}
//	return 0;
//}

//int CWindowView::UpdateNodeData(FileNode* node)
//{
//	//DWORD udc_ = FileNodeHelper::UpdateNode(node);
//	if (0) {
//		if (selectednode == node) {
//			::PostMessage(hMain, WM_U_DISPFILENODE, (WPARAM)selectednode, 0);
//			FileNodeList* cns = selectednode->nodes.CopyList();
//			FileNode* ndd;
//			for (int ii = 0; ii < cns->size; ii++) {
//				ndd = (*cns)[ii];
//				::PostMessage(hMain, WM_U_DISPFILENODE, (WPARAM)ndd, 0);
//			}
//			delete cns;
//		}
//	}
//	return 0?1:0;
//}

//int CWindowView::DisplayNode(HTREEITEM hpnt, FileNode* node, BOOL dochild)
//{
//	HTREEITEM htn;
//	if (mapNode.find(node) == mapNode.end())
//	{
//		htn = AddItemNode(hpnt, node);
//	}
//	else
//	{
//		htn = mapNode[node];
//	}
//
//	int rtn = UpdateNodeData(node);
//
//	if (dochild)
//	{
//		if (htn != INVALID_HANDLE_VALUE)
//		{
//			TVITEM tim = { 0 };
//			tim.hItem = htn;
//			tim.mask = TVIF_STATE;
//			tim.stateMask = TVIS_EXPANDED;
//			::SendMessage(hNodeTree, TVM_GETITEM, 0, (LPARAM)&tim);
//			BOOL bss = (tim.state & TVIS_EXPANDED) ? TRUE : FALSE;
//
//			FileNodeList* fnl = node->nodes.CopyList();
//			std::map<FileNode*, HTREEITEM>::iterator itmn;
//			for (int ii=0; ii<fnl->size; ii++)
//			{
//				DisplayNode(htn, (*fnl)[ii], bss);
//			}
//			delete fnl;
//		}
//	}
//
//	return rtn;
//}

class LessCnt : public std::less<std::pair<int, int>>
{
public:
	constexpr bool operator()(const std::pair<int, int>& _Left, const std::pair<int, int>& _Right) const { // apply operator< to operands
		if (_Left.second > _Right.second) return true;
		if (_Left.second < _Right.second) return false;
		if (_Left.first > _Right.first) return true;
		return false;
	}
};

int CWindowView::UpdateNodeColor(FileNode* node)
{
	int* extcnt = NULL;
	int ecs = mapExtIndex.size();
	if (ecs > 0) {
		extcnt = new int[ecs];
		ZeroMemory(extcnt, ecs * sizeof(int));
	}
	FileNodeHelper::UpdateNodeExtIndex(node, mapExtIndex, extcnt, ecs);
	if (ecs > 0) {
		std::set<std::pair<int, int>, LessCnt> ens;
		for (int ii = 0; ii < ecs; ii++) {
			ens.insert({ ii, extcnt[ii] });
		}

		int* sortextcnt = new int[ecs+1];
		int* sortextidx = new int[ecs+1];
		int ii = 0;
		for (std::set<std::pair<int, int>>::iterator ites = ens.begin()
			; ites != ens.end()
			; ites++) {
			sortextidx[ii] = (*ites).first;
			sortextcnt[ii] = (*ites).second;
			ii++;
		}

		DWORD* extcolors = new DWORD[ecs];
		blockView->UpdateColors(sortextidx, sortextcnt, extcolors, ecs);
		FileNodeHelper::UpdateNodeColor(node, extcolors, ecs);

		delete[] sortextidx;
		delete[] sortextcnt;
		delete[] extcolors;
		delete[] extcnt;
	}
	return 0;
}

int CWindowView::UpdateDisplayNode(FileNode* cnd)
{
	DWORD cat = cnd->display_cat;
	if (cat > 0) {
		cnd->display_cat = 0;
		::SendMessage(hMain, WM_U_DISPFILENODE, (WPARAM)cnd, cat);
	}
	return 0;
}

DWORD CWindowView::ThDisplayTree(_In_ LPVOID lpParameter)
{
	CWindowView* self = reinterpret_cast<CWindowView*>(lpParameter);
	int udn = 0;
	while (self->keeprun_ == 1)
	{
		self->ProcActions();

		if (self->selectednode) {
			self->PutAction(UIACTION::UI_ADD_CHILD, self->selectednode);
			if (self->viewnode) {
				self->iUpdateForBlock = (self->viewnode->updated_cat | self->viewnode->display_cat) > 0 ? 1 : 0;
			}
			self->UpdateDisplayNode(self->selectednode);
			for (int ii = 0; ii < self->selectednode->nodes.size; ii++) {
				self->UpdateDisplayNode(self->selectednode->nodes[ii]);
			}
			self->UpdateTaskbar(self->selectednode);
		}
		udn = self->ana->GetTasksCount();
		::SendMessage(self->hMain, WM_U_DISPTASKCOUNT, udn, 0);

		::Sleep(100);
	}
	::PostMessage(self->hMain, WM_COMMAND, MAKEWPARAM(IDM_STOPTH, 0), 0);
	return 0;
}

DWORD CWindowView::ThDisplayBlock(_In_ LPVOID lpParameter)
{
	CWindowView* self = reinterpret_cast<CWindowView*>(lpParameter);
	VisualBlock* vblk;
	std::list<VisualBlock*> elst;
	std::list<VisualBlock*> ndlst;
	FileNode* psn = NULL;
	BOOL brd;
	int udn;

	while (self->keeprun_ == 1)
	{
		if (self->blockView) {
			brd = FALSE;
			udn = self->iUpdateForBlock;
			self->iUpdateForBlock = 0;
			if (self->bna->NeedRedraw(udn, self->viewnode, self->blockView->ResetResized())) {
				self->UpdateNodeColor(self->viewnode);

				vblk = self->bna->AnalyzeFileNode(self->viewnode, self->blockView->GetRect());
				self->bna->GetEdgeList(vblk, elst);
				self->bna->GetNodeList(vblk, ndlst);
				self->blockView->SetViewList(elst);
				self->blockView->SetNodeList(ndlst);
				brd = TRUE;

				delete vblk;
				elst.clear();
				ndlst.clear();
			}
			if (self->selectednode != psn) {
				self->blockView->SetSelectNode(self->selectednode);
				brd = TRUE;
				psn = self->selectednode;
			}
			if (brd) {
				self->blockView->Redraw();
			}

			Sleep(1000);
		}
	}

	return 0x2234;
}

int CWindowView::UpdateTaskbar(FileNode* und) {
	if (pTaskbar) {
		ITaskbarList3* ptsk3;
		pTaskbar->QueryInterface(IID_ITaskbarList3, (void**)&ptsk3);
		if (ptsk3) {
			ptsk3->SetProgressValue(hMain, (ULONGLONG)(und->progress * 100), 100);
			ptsk3->Release();
		}
	}
	return 0;
}

int CWindowView::ProcActions()
{
	UIACTION act = { UIACTION::UI_NONE, 0 };
	BOOL keepact = TRUE;

	while (keepact) {
		act.action = UIACTION::UI_NONE;

		if (::WaitForSingleObject(hAction, 100) == WAIT_OBJECT_0) {
			if (actions.size() > 0) {
				act = *actions.begin();
				actions.pop_front();
			}
			else {
				keepact = FALSE;
			}
			::SetEvent(hAction);
		}

		switch (act.action) {
		case UIACTION::UI_ADD_NODE:
			AddNodeItem(act.hItem, act.node);
			break;
		case UIACTION::UI_ADD_CHILD:
		{
			std::map<FileNode*, HTREEITEM>::iterator itfm = mapNode.find(act.node);
			if (itfm != mapNode.end()) {
				std::list<FileNode*> nls;
				act.node->nodes.GetNodesList(nls);
				AddNodeListItem(itfm->second, nls);
			}
		}
		break;
		case UIACTION::UI_DEL_CHILD:
			break;
		case UIACTION::UI_EXPAND_NODE:
		{
			for (int ii = 0; ii < act.node->nodes.size; ii++) {
				PutAction(UIACTION::UI_ADD_CHILD, act.node->nodes[ii]);
			}
		}
		break;
		case UIACTION::UI_COLLS_NODE:
		{
			for (int ii = 0; ii < act.node->nodes.size; ii++) {
				PutAction(UIACTION::UI_DEL_CHILD, act.node->nodes[ii]);
			}
		}
		break;
		case UIACTION::UI_OPEN_FILENODE:
			if (act.node->type == FileNode::NT_FILE) {
				std::wstring pth = FileNodeHelper::GetFullPath(act.node);
				HINSTANCE hinst = ShellExecute(NULL, L"open", act.node->name, NULL, pth.c_str(), SW_SHOW);
				if ((long)hinst > 32) {
					wsprintf(wbuf, L"Open file [%s]", FileNodeHelper::GetNodeFullPathName(act.node).c_str());
					Log(wbuf);
				}
				else {
					wsprintf(wbuf, L"Failed open file [%s]", FileNodeHelper::GetNodeFullPathName(act.node).c_str());
					Log(wbuf);
				}
			}
			else if (act.node->type == FileNode::NT_DIR) {
				std::wstring pth = FileNodeHelper::GetNodeFullPathName(act.node);
				HINSTANCE hinst = ShellExecute(NULL, L"open", L".", NULL, pth.c_str(), SW_SHOW);
				if ((long)hinst > 32) {
					wsprintf(wbuf, L"Open directory [%s]", FileNodeHelper::GetNodeFullPathName(act.node).c_str());
					Log(wbuf);
				}
				else {
					wsprintf(wbuf, L"Failed open directory [%s]", FileNodeHelper::GetNodeFullPathName(act.node).c_str());
					Log(wbuf);
				}
			}
			break;
		case UIACTION::UI_UPDATE_FILENODES:
			UpdateFileNodes(act.node);
			break;
		default:
			break;
		}
	}
	return 0;
}

int CWindowView::ProcCommandStop()
{
	keeprun_ = 2;
	return 0;
}

int CWindowView::SelectTreeNode(FileNode* node)
{
	std::map<FileNode*, HTREEITEM>::iterator itmn = mapNode.find(node);
	if (itmn == mapNode.end()) {
		if (node->parent) {
			SelectTreeNode(node->parent);
		}
	} else {
		HTREEITEM hnd = itmn->second;
		TreeView_Select(hNodeTree, hnd, TVGN_CARET);
	}
	return 0;
}

int CWindowView::ProcCommandStopTh()
{
	DWORD dwex;
	if (hDispBlockTh) {
		::GetExitCodeThread(hDispBlockTh, &dwex);
		while (dwex == STILL_ACTIVE)
		{
			::Sleep(100);
			::GetExitCodeThread(hDispBlockTh, &dwex);
		}
		CloseHandle(hDispBlockTh);
		hDispBlockTh = NULL;
	}
	if (hDispTreeTh)
	{
		::GetExitCodeThread(hDispTreeTh, &dwex);
		while (dwex == STILL_ACTIVE)
		{
			::Sleep(100);
			::GetExitCodeThread(hDispTreeTh, &dwex);
		}
		CloseHandle(hDispTreeTh);
		hDispTreeTh = NULL;
	}
	selectednode = NULL;
	blockView->Clear();
	SendMessage(hFileList, LVM_DELETEALLITEMS, 0, 0);
	SendMessage(hNodeTree, TVM_DELETEITEM, 0, 0);
	mapNode.clear();
	mapExtIndex.clear();
	if (bna)
	{
		delete bna;
		bna = NULL;
	}
	if (ana)
	{
		ana->Stop();
		delete ana;
		ana = NULL;
	}
	if (keeprun_ == 3) {
		::DestroyWindow(hMain);
	}

	return 0;
}

int CWindowView::ChangeWindowTitle(const std::wstring& procpath)
{
	wchar_t otitle[MAX_LOADSTRING];
	int tln = GetWindowText(hMain, otitle, MAX_LOADSTRING);
	if (tln) {
		wsprintf(wbuf, L"%s - %s", procpath.c_str(), otitle);
		SetWindowText(hMain, wbuf);
	}
	return 0;
}

int CWindowView::ProcCommandStart(const WCHAR* path)
{
	DWORD thId;

	int pln = wcslen(path);
	if (pln > 0) {
		WCHAR* ptc = new WCHAR[pln + 1];
		wcscpy_s(ptc, pln + 1, path);
		BOOL keepseek = ptc[pln - 1] == L'\\';
		while (keepseek) {
			ptc[pln - 1] = 0;
			pln--;
			keepseek = pln > 0 && ptc[pln - 1] == L'\\';
		}
		if (pln > 0) {
			if (hNodeTree != INVALID_HANDLE_VALUE) {
				if (ana) {
					CWindowView* view = CWindowView::GetViewInstance(::GetModuleHandle(NULL));
					if (view) {
						view->prevPath = this->prevPath;
						view->ProcCommandStart(ptc);
					}
				}
				else {
					wsprintf(wbuf, L"Start path [%s]", ptc);
					Log(wbuf);
					ChangeWindowTitle(ptc);

					ana = new CPathAnalyzer();
					ana->StartPathDist(ptc);
					bna = new CBlockAnalyzer();

					PutAction(UIACTION::UI_ADD_NODE, ana->root);
					selectednode = ana->root;
					viewnode = ana->root;

					keeprun_ = 1;
					hDispTreeTh = ::CreateThread(NULL, 0, CWindowView::ThDisplayTree, this, 0, &thId);
					hDispBlockTh = ::CreateThread(NULL, 0, CWindowView::ThDisplayBlock, this, 0, &thId);
				}
			}
		}
		delete[] ptc;
	}
	return 0;
}

BOOL CWindowView::ProcCommandDrives(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL btn = FALSE;

	unsigned long mid = wParam;
	if (mapDrive.find(mid) != mapDrive.end()) {
		btn = TRUE;
		ProcCommandStart(mapDrive[mid].c_str());
	}

	return btn;
}

#define SYS_DIR_SEL_DIALOG_FOLDER 0x0
#define SYS_DIR_SEL_DIALOG_TREEID 0x64

INT CALLBACK CallbackBrowseFoldersProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
	switch (uMsg) {
	case BFFM_INITIALIZED:
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
		break;
	case BFFM_SELCHANGED:
		if (hwnd) {
			HWND hfd = GetDlgItem(hwnd, SYS_DIR_SEL_DIALOG_FOLDER);
			if (hfd) {
				HWND htr = GetDlgItem(hfd, SYS_DIR_SEL_DIALOG_TREEID);
				if (htr) {
					HTREEITEM hti = (HTREEITEM)SendMessage(htr, TVM_GETNEXTITEM, TVGN_CARET, 0);
					if (hti) {
						::SendMessage(htr, TVM_ENSUREVISIBLE, 0, (LPARAM)hti);
					}
				}
			}
		}
		break;
	default:
		break;
	}
	return 0;
}

LRESULT CWindowView::ProcSelectDirectory(HWND hPnt)
{
	HRESULT hr = CoInitialize(NULL);
	BROWSEINFO bsi = { 0 };
	bsi.lpfn = CallbackBrowseFoldersProc;
	bsi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bsi.lpszTitle = L"Select Path";
	bsi.lParam = (LPARAM)prevPath.c_str();
	bsi.hwndOwner = hPnt;
	bsi.ulFlags = BIF_EDITBOX | BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON | BIF_USENEWUI;
	LPITEMIDLIST pidl = SHBrowseForFolder(&bsi);
	if (pidl) {
		wchar_t buffer[MAX_PATH];
		if (SHGetPathFromIDList(pidl, buffer)) {
			prevPath = buffer;
			ProcCommandStart(buffer);
		}
	}
	return 0;
}
LRESULT CWindowView::ProcCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	// Parse the menu selections:
	switch (wmId)
	{
	case IDM_ABOUT:
		DialogBox(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
		break;
	case IDM_EXIT:
		keeprun_ = 3;
		::PostMessage(hMain, WM_CLOSE, 0, 0);
		break;
	case IDM_REFRESH_DRIVES:
		InitialMenu(hWnd);
		break;
	case IDM_STOP:
		ProcCommandStop();
		break;
	case IDM_STOPTH:
		ProcCommandStopTh();
		break;
	case IDM_PATH_SELECTDIRECTORY:
		ProcSelectDirectory(hWnd);
		break;
	case IDM_OPEN_NODE_IN_SHELL:
		PostMessage(hMain, WM_U_OPENTREENODEINSHELL, NULL, NULL);
		break;
	case IDM_ZOOM_NODE:
		::PostMessage(hMain, WM_U_ZOOM_TO_TREENODE, NULL, NULL);
		break;
	case IDM_TREENODE_PAUSE:
		if (ana) {
			ana->Pause();
		}
		break;
	case IDM_TREENODE_RESUME:
		if (ana) {
			ana->Resume();
		}
	case IDM_OPEN_NODEFOLDER_IN_SHELL:
		if (ana) {
			PostMessage(hMain, WM_U_OPENNODEFOLDERINSHELL, NULL, NULL);
			break;
		}
	default:
		if (!ProcCommandDrives(hWnd, message, wParam, lParam)) {
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}

	return 0;
}

LRESULT CWindowView::ProcNotifyTree(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lrst = 0;
	NMTREEVIEW* pntree = (NMTREEVIEW*)lParam;

	switch (pntree->hdr.code)
	{
	case TVN_SELCHANGED:
	{
		if (pntree->itemNew.hItem) 
		{
			TVITEM tgi = { 0 };
			tgi.mask = TVIF_PARAM;
			tgi.hItem = pntree->itemNew.hItem;
			::SendMessage(hNodeTree, TVM_GETITEM, 0, (LPARAM)&tgi);
			if (tgi.lParam)
			{
				selectednode = (FileNode*)tgi.lParam;
				//::PostMessage(hMain, WM_U_UPDATEFILENODES, 0, tgi.lParam);
				PutAction(UIACTION::UI_UPDATE_FILENODES, selectednode);
			}
		}
	}
	break;
	case TVN_ITEMEXPANDED:
		if (pntree->itemNew.hItem) {
			TVITEM tgi = { 0 };
			FileNode* nde = (FileNode*)pntree->itemNew.lParam;
			tgi.hItem = pntree->itemNew.hItem;
			tgi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			tgi.iImage = 4;
			if (pntree->action == TVE_EXPAND) {
				tgi.iImage = nde->type == FileNode::NT_DIR ? 1 : (nde->type == FileNode::NT_VDIR ? 3 : tgi.iImage);
				PutAction(UIACTION::UI_EXPAND_NODE, nde);
			}
			else if (pntree->action == TVE_COLLAPSE) {
				tgi.iImage = nde->type == FileNode::NT_DIR ? 0 : (nde->type == FileNode::NT_VDIR ? 2 : tgi.iImage);
				PutAction(UIACTION::UI_COLLS_NODE, nde);
			}
			tgi.iSelectedImage = tgi.iImage;
			TreeView_SetItem(hNodeTree, &tgi);
		}
		break;
	case NM_DBLCLK:
		::PostMessage(hMain, WM_U_OPENTREENODEINSHELL, NULL, NULL);
		break;
	}
	return lrst;
}

int CWindowView::CompFileNodeList(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	FileNode* fn1 = (FileNode*)lParam1;
	FileNode* fn2 = (FileNode*)lParam2;
	int64_t cpi = fn1->compsize - fn2->compsize;
	return cpi < 0 ? 1 : (cpi > 0 ? -1 : 0);
}

LRESULT CWindowView::ProcNotifyList(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lrst = 0;
	LPNMHDR pnmh = (LPNMHDR)lParam;

	switch (pnmh->code)
	{
	case LVN_COLUMNCLICK:
	{
		LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
		if (pnmv->iSubItem == 5) {
			ListView_SortItems(hFileList, &CompFileNodeList, NULL);
		}
	}
		break;
	case NM_CUSTOMDRAW:
	{
		LPNMLVCUSTOMDRAW plcd = (LPNMLVCUSTOMDRAW)lParam;
		switch (plcd->nmcd.dwDrawStage)
		{
		case CDDS_PREPAINT:
			lrst = CDRF_NOTIFYITEMDRAW;
			break;
		case CDDS_ITEMPREPAINT:
			lrst = CDRF_NOTIFYSUBITEMDRAW;
			break;
		case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
			if (plcd->iSubItem == 1)
			{
				RECT src;
				ListView_GetSubItemRect(hFileList, plcd->nmcd.dwItemSpec, 1, LVIR_BOUNDS, &src);
				InflateRect(&src, -1, -1);
				FillRect(plcd->nmcd.hdc, &src, hbWhite);
				if (plcd->nmcd.lItemlParam) {
					FileNode* fnd = (FileNode*)plcd->nmcd.lItemlParam;
					src.right = (long)((src.right - src.left) * fnd->progress) + src.left;
					HBRUSH hbd = hbGreen;
					if (fnd->progress < 0.8) {
						hbd = hbRed;
					}
					FillRect(plcd->nmcd.hdc, &src, hbd);
					LVITEM lvi = { 0 };
					lvi.iSubItem = 1;
					lvi.pszText = wbuf;
					lvi.cchTextMax = 1024;
					int tls = ::SendMessage(hFileList, LVM_GETITEMTEXT, plcd->nmcd.dwItemSpec, (LPARAM)&lvi);
					DrawText(plcd->nmcd.hdc, wbuf, tls, &src, DT_RIGHT);
				}
				lrst = CDRF_SKIPDEFAULT;
			}
			else
			{
				lrst = CDRF_DODEFAULT;
			}
			break;
		default:
			break;
		}
	}
	break;
	case NM_DBLCLK:
	{
		LPNMITEMACTIVATE lpiac = (LPNMITEMACTIVATE)lParam;
		if (lpiac->iItem >= 0) {
			LVITEM tgi = { 0 };
			tgi.mask = LVIF_PARAM;
			tgi.iItem = lpiac->iItem;
			::SendMessage(hFileList, LVM_GETITEM, 0, (LPARAM)& tgi);

			if (tgi.lParam) {
				FileNode* cnd = (FileNode*)tgi.lParam;
				if (cnd->type == FileNode::NT_FILE) {
					::PostMessage(hMain, WM_U_OPENNODEINSHELL, 0, tgi.lParam);
				}
				else {
					::PostMessage(hMain, WM_U_SELECTTREENODE, 0, tgi.lParam);
				}
			}
		}
	}
	break;
	default:
		break;
	}

	return lrst;
}

LRESULT CWindowView::ProcNotifyVisual(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lrst = 0;
	NMVISUALBLOCK* pnmvb = (NMVISUALBLOCK*)lParam;
	switch (pnmvb->nmhdr.code) {
	case NMC_VISUALBLOCK_HOVERNODE:
		::PostMessage(hMain, WM_U_HOVERNODE, (WPARAM)pnmvb->node, 0);
		break;
	case NMC_VISUALBLOCK_SELECTNODE:
		::PostMessage(hMain, WM_U_SELECTTREENODE, 0, (LPARAM)pnmvb->node);
		break;
	case NMC_VISUALBLOCK_LOGCLICKNODE:
	{
		FileNode* cnd = pnmvb->node;
		if (cnd) {
			wsprintf(wbuf, L"Copy [%s] to clip board", FileNodeHelper::GetNodeFullPathName(cnd).c_str());
			Log(wbuf);
		}
	}
		break;
	}

	return lrst;
}

LRESULT CWindowView::ProcNotify(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lrst = 0;
	LPNMHDR pnmh = (LPNMHDR)lParam;

	if (pnmh->hwndFrom == hNodeTree)
	{
		lrst = ProcNotifyTree(hWnd, message, wParam, lParam);
	}
	else if (pnmh->hwndFrom == hFileList)
	{
		lrst = ProcNotifyList(hWnd, message, wParam, lParam);
	}
	else if (pnmh->hwndFrom == *blockView)
	{
		lrst = ProcNotifyVisual(hWnd, message, wParam, lParam);
	}
	return lrst;
}

int FormatNumberView(long long num, WCHAR* wbuf, int bsz)
{
	WCHAR dpb[1024];

	long long dpn = num;
	long mun;
	wsprintf(wbuf, L"");
	wsprintf(dpb, L"");
	while (dpn > 0)
	{
		mun = dpn % 1000;
		dpn /= 1000;
		if (dpn > 0)
		{
			wsprintf(dpb, L",%03d%s", mun, wbuf);
		}
		else
		{
			wsprintf(dpb, L"%d%s", mun, wbuf);
		}
		wsprintf(wbuf, L"%s", dpb);
	}

	if (wcslen(wbuf) <= 0)
	{
		wsprintf(wbuf, L"0");
	}

	return 0;
}

int CWindowView::InsertNodeInList(FileNode* node, int idx)
{
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
	lvi.pszText = (LPWSTR)node->name;
	lvi.iItem = idx;
	lvi.cchTextMax = (int)wcslen(node->name);
	lvi.lParam = (LPARAM)node;
	lvi.iImage = node->type == FileNode::NT_DIR ? 0 : (node->type == FileNode::NT_VDIR ? 2 : 4);
	ListView_InsertItem(hFileList, &lvi);

	DispListNodeItem(lvi.iItem, node, nuc_all);

	return 0;
}

int CWindowView::UpdateFileNodes(FileNode* node)
{
	if (node)
	{
		ListView_DeleteAllItems(hFileList);
		//FileNodeList* dcl;
		//dcl = node->nodes.CopyList();
		for (int ii = 0; ii < node->nodes.size; ii++)
		{
			if (node==selectednode)	{
				InsertNodeInList(node->nodes[ii], ii);
			}
		}
		//delete dcl;
		InsertNodeInList(node, 0);
	}
	return 0;
}

int CWindowView::UpdateFileNodes_(FileNode* node)
{
	int ii;
	if (node)
	{
		ListView_DeleteAllItems(hFileList);
		//FileNodeList* dcl;
		//dcl = node->nodes.CopyList();
		ii = 0;
		while (ii<node->nodes.size)
		{
			ii = SendMessage(hMain, WM_U_DISPFILENODE_TIMELY, (WPARAM)node, ii);
		}
		//delete dcl;
		InsertNodeInList(node, 0);
	}
	return 0;
}

int CWindowView::DispListNodeItem(int itemindex, FileNode* node, DWORD cat)
{
	wchar_t dsbf[1024];
	if (cat & nuc_progress) {
		swprintf_s(dsbf, L"%.02f%%", node->progress * 100);
		ListView_SetItemText(hFileList, itemindex, 1, dsbf);
	}
	if (cat & nuc_fcnt) {
		FormatNumberView(node->filecnt, dsbf, 1024);
		ListView_SetItemText(hFileList, itemindex, 2, dsbf);
	}
	if (cat & nuc_dcnt) {
		FormatNumberView(node->dircnt, dsbf, 1024);
		ListView_SetItemText(hFileList, itemindex, 3, dsbf);
	}
	if (cat & nuc_size) {
		FormatNumberView(node->size, dsbf, 1024);
		ListView_SetItemText(hFileList, itemindex, 4, dsbf);
	}
	if (cat & nuc_compsize) {
		FormatNumberView(node->compsize, dsbf, 1024);
		ListView_SetItemText(hFileList, itemindex, 5, dsbf);
	}

	return 0;
}

int CWindowView::DispListNode(FileNode* node, DWORD cat)
{
	LVFINDINFO lfi = { 0 };
	lfi.flags = LVFI_PARAM;
	lfi.lParam = (LPARAM)node;
	
	int itm = ListView_FindItem(hFileList, -1, &lfi);
	if (itm >= 0)
	{
		DispListNodeItem(itm, node, cat);
	}
	return 0;
}

int CWindowView::PutAction(UIAction::ACODE act, FileNode* cnd)
{
	if (::WaitForSingleObject(hAction, INFINITE) == WAIT_OBJECT_0) {
		actions.push_back({ act, cnd, NULL});
		::SetEvent(hAction);
	}
	return 0;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK CWindowView::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lrst = 0;
	CWindowView* view = NULL;
	std::map<HWND, CWindowView*>::iterator itwv = CWindowView::views.find(hWnd);
	if (itwv != CWindowView::views.end())
	{
		view = itwv->second;
	}

	switch (message)
	{
	case WM_COMMAND:
		if (view) {
			lrst = view->ProcCommand(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_NOTIFY:
		if (view) {
			lrst = view->ProcNotify(hWnd, message, wParam, lParam);
		}
		break;
	case WM_DESTROY:
		if (view) {
			if (view->hDispTreeTh) {
				view->keeprun_ = 3;
			}
			else {
				PostQuitMessage(0);
			}
		}
		else {
			PostQuitMessage(0);
		}
		break;
	case WM_SIZE:
		if ((view) && (view->splitContent))
		{
			RECT rct;
			::GetClientRect(hWnd, &rct);
			::MoveWindow(*view->splitContent, rct.left, rct.top, rct.right, rct.bottom, FALSE);
		}
		break;
	case WM_U_UPDATEFILENODES:
		if (view) {
			FileNode* fnd = (FileNode*)lParam;
			view->UpdateFileNodes(fnd);
		}

		break;
	case WM_U_HOVERNODE:
		if (view)
		{
			FileNode* fnd = (FileNode*)wParam;
			std::wstring pth = FileNodeHelper::GetNodeFullPathName(fnd);
			::SendMessage(view->hStatus, SB_SETTEXT, MAKEWPARAM(MAKEWORD(2, 0), 0), (LPARAM)pth.c_str());
			WCHAR wbs[1024];
			FormatNumberView(fnd->size, wbs, 1024);
			::SendMessage(view->hStatus, SB_SETTEXT, MAKEWPARAM(MAKEWORD(1, 0), 0), (LPARAM)wbs);
		}
		break;
	case WM_U_DISPTASKCOUNT:
		if (view)
		{
			WCHAR wbs[1024];
			WCHAR sbs[1024];
			int ttc = wParam;
			FormatNumberView(ttc, wbs, 1024);
			wsprintf(sbs, L"Tasks: %s", wbs);
			::SendMessage(view->hStatus, SB_SETTEXT, MAKEWPARAM(MAKEWORD(0, 0), 0), (LPARAM)sbs);
		}
		break;
	case WM_U_DISPFILENODE_TIMELY:
		if (view) {
			DWORD ttk = GetTickCount();
			FileNode* cnd = (FileNode*)wParam;
			int icd = lParam;
			DWORD ctk = ttk;
			BOOL kupd = TRUE;
			while (kupd) {
				view->InsertNodeInList(cnd->nodes[icd], 0);
				icd++;
				kupd = icd < cnd->nodes.size ? kupd : FALSE;
				ctk = GetTickCount();
				kupd = ctk - ttk < 0 ? kupd : FALSE;
			}
			lrst = icd;
		}
		break;
	case WM_U_DISPFILENODE:
		if (view) {
			FileNode* dnd = (FileNode*)wParam;
			DWORD updcat = lParam;
			view->DispListNode(dnd, updcat);
		}
		break;
	case WM_U_SELECTTREENODE:
		if (view) {
			FileNode* scn = (FileNode*)lParam;
			view->SelectTreeNode(scn);
		}
		break;
	case WM_U_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CLOSE:
		if (view) {
			::ShowWindow(view->hMain, SW_HIDE);
			if (view->hDispTreeTh) {
				view->keeprun_ = 3;
			}
			else {
				PostMessage(hWnd, WM_U_DESTROY, 0, 0);
			}
		}
		else {
			PostMessage(hWnd, WM_U_DESTROY, 0, 0);
		}
		break;
	case WM_CONTEXTMENU:
		if (view) {
			view->ProcContextMenu(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		}
		break;
	case WM_U_OPENNODEINSHELL:
		if (view) {
			FileNode* cnd = (FileNode*)lParam;
			view->PutAction(UIACTION::UI_OPEN_FILENODE, cnd);
		}
		break;
	case WM_U_OPENNODEFOLDERINSHELL:
		if (view) {
			HTREEITEM hti = (HTREEITEM)SendMessage(view->hNodeTree, TVM_GETNEXTITEM, TVGN_CARET, 0);
			TVITEM tgi = { 0 };
			tgi.mask = TVIF_PARAM;
			tgi.hItem = hti;
			::SendMessage(view->hNodeTree, TVM_GETITEM, 0, (LPARAM)& tgi);
			if (tgi.lParam)
			{
				FileNode* cnd = (FileNode*)tgi.lParam;
				cnd = FileNodeHelper::GetFolderNode(cnd);
				::PostMessage(hWnd, WM_U_OPENNODEINSHELL, 0, (LPARAM)cnd);
			}

		}
		break;
	case WM_U_OPENTREENODEINSHELL:
		if (view) {
			HTREEITEM hti = (HTREEITEM)SendMessage(view->hNodeTree, TVM_GETNEXTITEM, TVGN_CARET, 0);
			TVITEM tgi = { 0 };
			tgi.mask = TVIF_PARAM;
			tgi.hItem = hti;
			::SendMessage(view->hNodeTree, TVM_GETITEM, 0, (LPARAM)& tgi);
			if (tgi.lParam)
			{
				FileNode* cnd = (FileNode*)tgi.lParam;
				::PostMessage(hWnd, WM_U_OPENNODEINSHELL, 0, (LPARAM)cnd);
			}
		}
		break;
	case WM_U_ZOOM_TO_TREENODE:
		if (view) {
			HTREEITEM hti = (HTREEITEM)SendMessage(view->hNodeTree, TVM_GETNEXTITEM, TVGN_CARET, 0);
			TVITEM tgi = { 0 };
			tgi.mask = TVIF_PARAM;
			tgi.hItem = hti;
			::SendMessage(view->hNodeTree, TVM_GETITEM, 0, (LPARAM)& tgi);
			if (tgi.lParam)
			{
				FileNode* cnd = (FileNode*)tgi.lParam;
				view->viewnode = cnd;
			}
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return lrst;
}

int CWindowView::ProcContextMenu(int xx, int yy)
{
	POINT pnt = { xx, yy };
	POINT wpt;
	RECT rct;

	wpt = pnt;
	::ScreenToClient(hNodeTree, &wpt);

	::GetClientRect(hNodeTree, &rct);
	if (PtInRect(&rct, wpt)) {
		ProcContextMenuTree(wpt.x, wpt.y);
	}
	return 0;
}

int CWindowView::ProcContextMenuTree(int xx, int yy)
{
	RECT rct;
	POINT pnt = { xx, yy };
	HTREEITEM hti = (HTREEITEM)SendMessage(hNodeTree, TVM_GETNEXTITEM, TVGN_CARET, 0);
	if (hti) {
		*(HTREEITEM*)& rct = hti;
		BOOL bst = (BOOL)SendMessage(hNodeTree, TVM_GETITEMRECT, TRUE, (LPARAM)& rct);
		if (bst) {
			if (PtInRect(&rct, pnt)) {
				HINSTANCE hinst = ::GetModuleHandle(NULL);
				HMENU hcm = LoadMenu(hinst, MAKEINTRESOURCEW(IDC_NODECONTEXT));
				if (hcm) {
					HMENU hsm = GetSubMenu(hcm, 0);
					ClientToScreen(hNodeTree, &pnt);
					TrackPopupMenu(hsm, TPM_LEFTALIGN | TPM_LEFTBUTTON, pnt.x, pnt.y, 0, hMain, NULL);
					DestroyMenu(hsm);
				}
			}
		}
	}
	return 0;
}
//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM CWindowView::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SIMPLEVISUALBLOCKS));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SIMPLEVISUALBLOCKS);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

int InitialFileList(HWND hlist)
{
	int idx;

	LVCOLUMN lvc = { 0 };

	idx = 0;
	lvc.mask = LVCF_TEXT | LVCF_WIDTH;
	lvc.cx = 120;
	lvc.pszText = (LPWSTR)L"FileName";
	lvc.cchTextMax = (int)wcslen(lvc.pszText);
	::SendMessage(hlist, LVM_INSERTCOLUMN, idx, (LPARAM)&lvc);
	//::SendMessage(hlist, LVM_SETCOLUMNWIDTH, idx, LVSCW_AUTOSIZE_USEHEADER);

	idx = 1;
	lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
	lvc.fmt = LVCFMT_RIGHT;
	lvc.cx = 120;
	lvc.pszText = (LPWSTR)L"Progress";
	lvc.cchTextMax = (int)wcslen(lvc.pszText);
	::SendMessage(hlist, LVM_INSERTCOLUMN, idx, (LPARAM)& lvc);
	//::SendMessage(hlist, LVM_SETCOLUMNWIDTH, idx, LVSCW_AUTOSIZE_USEHEADER);

	idx = 2;
	lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
	lvc.fmt = LVCFMT_RIGHT;
	lvc.cx = 100;
	lvc.pszText = (LPWSTR)L"Files";
	lvc.cchTextMax = (int)wcslen(lvc.pszText);
	::SendMessage(hlist, LVM_INSERTCOLUMN, idx, (LPARAM)& lvc);
	
	idx = 3;
	lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
	lvc.fmt = LVCFMT_RIGHT;
	lvc.cx = 80;
	lvc.pszText = (LPWSTR)L"Dirs";
	lvc.cchTextMax = (int)wcslen(lvc.pszText);
	::SendMessage(hlist, LVM_INSERTCOLUMN, idx, (LPARAM)& lvc);

	idx = 4;
	lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
	lvc.fmt = LVCFMT_RIGHT;
	lvc.cx = 120;
	lvc.pszText = (LPWSTR)L"Size";
	lvc.cchTextMax = (int)wcslen(lvc.pszText);
	::SendMessage(hlist, LVM_INSERTCOLUMN, idx, (LPARAM)& lvc);

	idx = 5;
	lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
	lvc.fmt = LVCFMT_RIGHT;
	lvc.cx = 120;
	lvc.pszText = (LPWSTR)L"Size on Disk";
	lvc.cchTextMax = (int)wcslen(lvc.pszText);
	::SendMessage(hlist, LVM_INSERTCOLUMN, idx, (LPARAM)& lvc);
	//::SendMessage(hlist, LVM_SETCOLUMNWIDTH, idx, LVSCW_AUTOSIZE_USEHEADER);

	HIMAGELIST hImages = ImageList_Create(16, 16, ILC_COLOR32, 5, 10);
	HINSTANCE hInst = ::GetModuleHandle(NULL);
	HBITMAP hBitMap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BM_NODETREE));
	ImageList_Add(hImages, hBitMap, NULL);
	DeleteObject(hBitMap);

	ListView_SetImageList(hlist, hImages, LVSIL_SMALL);
	return 0;
}

int CWindowView::InitialStatusBar(HWND hsts)
{
	int pars[3] = { 100, 300, -1 };
	::SendMessage(hsts, SB_SETPARTS, 3, (LPARAM)&pars);

	return 0;
}

int CWindowView::LoadLogicalDrives()
{
	DWORD szz = ::GetLogicalDriveStrings(NULL, NULL);
	if (szz) {
		wchar_t* dbf = new wchar_t[szz];
		szz = ::GetLogicalDriveStrings(szz, dbf);
		if (szz) {
			unsigned long ipos = 0;
			int ii = 0;
			while (ipos < szz) {
				mapDrive[COMMAND_MENU_DRIVE_BASE + ii] = dbf + ipos;
				ipos += wcslen(dbf + ipos) + 1;
				ii++;
			}
		}
		delete[] dbf;
	}
	return 0;
}

int CWindowView::InitialMenu(HWND hwnd)
{
	HMENU hmn = ::GetMenu(hwnd);

	if (hmn) {
//		HMENU hsub;
		MENUITEMINFO mii = { 0 };
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_ID | MIIM_STRING;

		if (mapDrive.size() > 0) {
			for (std::map<unsigned long, std::wstring>::iterator itmd = mapDrive.begin()
				; itmd != mapDrive.end()
				; itmd++) {
				::DeleteMenu(hmn, itmd->first, MF_BYCOMMAND);
			}
			mapDrive.clear();
		}

		LoadLogicalDrives();

		for (std::map<unsigned long, std::wstring>::iterator itdm = mapDrive.begin()
			; itdm != mapDrive.end()
			; itdm++) {
			mii.wID = itdm->first;
			mii.dwTypeData = (LPWSTR)itdm->second.c_str();
			mii.cch = itdm->second.length();
			InsertMenuItem(hmn, IDM_REFRESH_DRIVES, FALSE, &mii);
		}
	}
	return 0;
}

int CWindowView::InitialLog()
{
	Log(L"HELP: Shift+CLICK block copy path to clipboard");
	Log(L"HELP: Click 'Size on Disk' to sort");
	Log(L"HELP: Double click node or block to focus in tree");
	Log(L"HELP: Analysis of 'Size on Disk' takes time, please be petiant");

	return 0;
}

BOOL CWindowView::InitiateNodeTree(HWND hTree)
{
	HIMAGELIST hImages = ::ImageList_Create(16, 16, ILC_COLOR32, 5, 10);
	HBITMAP hBitMap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BM_NODETREE));
	ImageList_Add(hImages, hBitMap, NULL);
	DeleteObject(hBitMap);

	TreeView_SetImageList(hTree, hImages, TVSIL_NORMAL);

	return TRUE;
}

BOOL CWindowView::InitialTaskbar(HWND hpnt)
{
	HRESULT hrt = CoInitialize(NULL);
	if (hrt == S_OK) {
		hrt = ::CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL, IID_ITaskbarList3, (LPVOID*)& pTaskbar);
	}
	return TRUE;
}

BOOL CWindowView::InitContentWindow(HINSTANCE hinst, HWND hpnt)
{
	if (splitContent == NULL)
	{
		InitCommonControls();

		//////////////////// Create status split
		splitContent = CSplitWnd::CreateSplitWindow(hpnt);
		::ShowWindow(*splitContent, SW_SHOW);
		splitContent->style = CSplitWnd::DOCKDOWN;

		splitStatus = CSplitWnd::CreateSplitWindow(*splitContent);
		hStatus = CreateWindow(STATUSCLASSNAME, L"", WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 200, 10, *splitContent, NULL, hInst, NULL);

		splitContent->SetWindow(*splitStatus);
		splitContent->SetWindow(hStatus);

		/////////////////// Create log split

		splitStatus->style = CSplitWnd::TOPDOWN;
		splitStatus->SetRatio(0.8);

		splitLog = CSplitWnd::CreateSplitWindow(*splitStatus);
		hLogList = CreateWindowEx(LVS_EX_FULLROWSELECT, WC_LISTBOX, L""
			, WS_CHILD | LBS_NOINTEGRALHEIGHT | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL, 0, 0, 0, 0
			, *splitStatus, NULL, hinst, NULL);

		splitStatus->SetWindow(*splitLog);
		splitStatus->SetWindow(hLogList);

		//////////////////// Create tree split
		splitLog->style = CSplitWnd::LEFTRIGHT;
		hNodeTree = CreateWindowEx(0, WC_TREEVIEW, L"", WS_CHILD | WS_VSCROLL | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_SHOWSELALWAYS, 0, 0, 0, 0, *splitLog, NULL, hinst, NULL);
		splitVisual = CSplitWnd::CreateSplitWindow(*splitLog);
		splitLog->SetRatio(0.3);

		splitLog->SetWindow(hNodeTree);
		splitLog->SetWindow(*splitVisual);

		//////////////////// Create list split
		splitVisual->style = CSplitWnd::TOPDOWN;
		hFileList = CreateWindowEx(0, WC_LISTVIEW, L"", WS_CHILD | WS_VSCROLL | WS_HSCROLL | LVS_REPORT | LVS_EDITLABELS | WS_VISIBLE, 0, 0, 0, 0, *splitVisual, NULL, hinst, NULL);
		blockView = CVisualBlockView::CreateVisualBlockView(*splitVisual);

		splitVisual->SetWindow(hFileList);
		splitVisual->SetWindow(*blockView);

		//////////////////// Setup tree view
		InitiateNodeTree(hNodeTree);

		//////////////////// Setup content windows
		ListView_SetExtendedListViewStyle(hFileList, LVS_EX_FULLROWSELECT);
		InitialFileList(hFileList);

		//////////////////// Setup status bar
		InitialStatusBar(hStatus);

		/////////////////// Setup system font
		HFONT hsysfont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
		::SendMessage(hFileList, WM_SETFONT, (WPARAM)hsysfont, MAKELPARAM(TRUE, 0));
		::SendMessage(hLogList, WM_SETFONT, (WPARAM)hsysfont, MAKELPARAM(TRUE, 0));
		::SendMessage(hNodeTree, WM_SETFONT, (WPARAM)hsysfont, MAKELPARAM(TRUE, 0));

		/////////////////// Setup Menu
		InitialMenu(hpnt);

		/////////////////// Setup output help log
		InitialLog();

		/////////////////// Setup taskbar progress
		InitialTaskbar(hpnt);
	}
	return TRUE;
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL CWindowView::InitInstance(HINSTANCE hInstance, HWND hMain, int nCmdShow)
{
	if (!hMain)
	{
		return FALSE;
	}

	InitContentWindow(hInstance, hMain);

	ShowWindow(hMain, nCmdShow);
	UpdateWindow(hMain);

	return TRUE;
}

CWindowView::CWindowView()
	: hMain(NULL)
	, wbuf(L"")
	, hInst(NULL)
	, hStatus(NULL)
	, splitContent(NULL)
	, splitLog(NULL)
	, splitStatus(NULL)
	, splitVisual(NULL)
	, viewClass(0)
	, ana(NULL)
	, bna(NULL)
	, blockView(NULL)
	, keeprun_(0)
	, hDispTreeTh(NULL)
	, hDispBlockTh(NULL)
	, selectednode(NULL)
	, viewnode(NULL)
	, iUpdateForBlock(0)
{
	hbWhite = ::CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF));
	hbGreen = ::CreateSolidBrush(RGB(0xF0, 0xFF, 0xF0));
	hbRed = ::CreateSolidBrush(RGB(0xFF, 0xF0, 0xF0));

	hAction = ::CreateEvent(NULL, FALSE, TRUE, NULL);
}

CWindowView::~CWindowView()
{
	::DeleteObject(hbWhite);
	::DeleteObject(hbGreen);
	::DeleteObject(hbRed);
	if (pTaskbar) {
		pTaskbar->Release();
		pTaskbar = NULL;
	}
	if (hAction) {
		CloseHandle(hAction);
		hAction = NULL;
	}
}

struct ViewParameter{
	BOOL started;
	HINSTANCE hInstance;
	WCHAR* path;
	HWND hMain;
	CWindowView* view;
};

DWORD CWindowView::ThNewWindow(_In_ LPVOID lpParameter)
{
	ViewParameter* vparm = (ViewParameter*)lpParameter;
	CWindowView* nwv = new CWindowView();
	nwv->CreateWindowView(vparm->hInstance);
	vparm->hMain = nwv->hMain;
	vparm->view = nwv;
	vparm->started = TRUE;
	nwv->ProcessMessage();

	return 0x4321;
}

std::list<HANDLE> CWindowView::windowThreads;
CWindowView* CWindowView::GetViewInstance(HINSTANCE hinst)
{
	DWORD thId;
	ViewParameter vparm;
	vparm.hInstance = hinst;
	vparm.started = FALSE;
	HANDLE hThWnd = ::CreateThread(NULL, 0, CWindowView::ThNewWindow, &vparm, 0, &thId);
	if (hThWnd) {
		windowThreads.push_back(hThWnd);
	}
	while (vparm.started == FALSE) {
		::Sleep(10);
	}
	return vparm.view;
}

int CWindowView::WaitAllWindows()
{
	HANDLE hth;
	while (windowThreads.size() > 0) {
		hth = *windowThreads.begin();
		if (WaitForSingleObject(hth, 100) == WAIT_OBJECT_0) {
			CloseHandle(hth);
			windowThreads.pop_front();
		}
	}
	return 0;
}

int CWindowView::CreateWindowView(HINSTANCE hinst)
{
	hInst = hinst;
	if (viewClass == 0)
	{
		LoadStringW(hInst, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
		LoadStringW(hInst, IDC_SIMPLEVISUALBLOCKS, szWindowClass, MAX_LOADSTRING);
		viewClass = MyRegisterClass(hInst);
	}

	hMain = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hinst, nullptr);
	views[hMain] = this;

	// Perform application initialization:
	if (!InitInstance(hInst, hMain, TRUE))
	{
		return FALSE;
	}

	return TRUE;
}

int CWindowView::ProcessMessage()
{
	HACCEL hAccelTable = LoadAccelerators(hInst, MAKEINTRESOURCE(IDC_SIMPLEVISUALBLOCKS));
	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

int CWindowView::Log(const WCHAR* msg)
{
	if (hLogList) {
		SYSTEMTIME stm;
		WCHAR lbuf[1024];
		::GetLocalTime(&stm);
		wsprintf(lbuf, L"[%04d-%02d-%02d %02d:%02d:%02d.%03d] : %s"
			, stm.wYear, stm.wMonth, stm.wDay
			, stm.wHour, stm.wMinute, stm.wSecond, stm.wMilliseconds
			, msg);
		ListBox_InsertString(hLogList, 0, lbuf);
	}

	return 0;
}