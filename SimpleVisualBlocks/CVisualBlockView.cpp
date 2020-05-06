#include "CVisualBlockView.h"
#include "Resource.h"

#include "VisualBlock.h"
#include "CNodes.h"

#include <set>
#include <CommCtrl.h>

#define MAX_LOADSTRING 100

ATOM CVisualBlockView::classBlock = 0;
std::map<HWND, CVisualBlockView*> CVisualBlockView::views;

WCHAR szBlockTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szBlockClass[MAX_LOADSTRING];            // the main window class name

#define COLOR_SEG_DIST 60

CVisualBlockView::CVisualBlockView()
	:hWnd(NULL)
	,resized(FALSE)
	,inBlock(NULL)
	,selectNode(NULL)
	,selectBlock(NULL)
	,hToolTip(NULL)
	,colorsCount(0)
	,colors(NULL)
{
	hEventView = ::CreateEvent(NULL, FALSE, TRUE, NULL);		
	hBlackBrush = ::CreateSolidBrush(RGB(0x00, 0x00, 0x7F));
	hGrayBrush = ::CreateSolidBrush(RGB(0xFF, 0x0, 0x0));
	hBackBrush = ::CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF));
	hSelectBrush = ::CreateSolidBrush(RGB(0x00, 0x00, 0xFF));

	BuildColorIndex();
}

int CVisualBlockView::BuildColorIndex()
{
	if (colors) {
		delete[] colors;
		colors = NULL;
	}

	colorsCount = 6 * COLOR_SEG_DIST;
	colors = new DWORD[colorsCount];
	DWORD* icolors = new DWORD[colorsCount];

	int iidx = 0;
	DWORD csl = 0xFF - COLOR_SEG_DIST + 1;
	DWORD rr = 0xFF;
	DWORD gg = csl;
	DWORD bb = csl;

	for (int ii = 0; ii < COLOR_SEG_DIST; ii++) {
		icolors[ii] = RGB(0xFF, csl + ii, csl);
		icolors[COLOR_SEG_DIST + ii] = RGB(0xFF - ii - 1, 0xFF, csl);
		icolors[2 * COLOR_SEG_DIST + ii] = RGB(csl, 0xFF, csl + ii);
		icolors[3 * COLOR_SEG_DIST + ii] = RGB(csl, 0xFF - ii - 1, 0xFF);
		icolors[4 * COLOR_SEG_DIST + ii] = RGB(csl + ii, csl, 0xFF);
		icolors[5 * COLOR_SEG_DIST + ii] = RGB(0xFF, csl, 0xFF - ii - 1);
	}

	int div = 1;
	for (int ii = 0; ii < colorsCount; ii++) {
		while (icolors[iidx] == 0) {
			iidx++;
			iidx %= colorsCount;
		}
		colors[ii] = icolors[iidx];
		icolors[iidx] = 0;

		iidx += colorsCount / div;
		if (iidx >= colorsCount) {
			div++;
			iidx += colorsCount / div > 0 ? colorsCount / div : 1;
			iidx %= colorsCount;
		}
	}

	delete[] icolors;
	return 0;
}

int CVisualBlockView::UpdateColors(int* extidx, int* extcnt, DWORD* extcolors, int cnt)
{
	int icnt = colorsCount - 1;
	int ii;
	int cdiv = 0;
	int pdiv = 0;
	int idx;
	int totalcnt = 0;

	for (ii = 0; ii < cnt; ii++) {
		totalcnt += extcnt[ii];
	}
	if (totalcnt > 0) {
		for (ii = 0; ii < cnt; ii++) {
			// find the most less ext index -> idx
			idx = extidx[cnt - ii - 1];
			cdiv += extcnt[cnt - ii - 1];
			pdiv = cdiv * icnt / totalcnt;
			extcolors[idx] = colors[colorsCount - pdiv - 1];
		}
	}
	return 0;
}

LRESULT CALLBACK CVisualBlockView::WndBlockProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lrst = 0;
	CVisualBlockView* view = NULL;
	std::map<HWND, CVisualBlockView*>::iterator itwv = CVisualBlockView::views.find(hWnd);
	if (itwv != CVisualBlockView::views.end())
	{
		view = itwv->second;
	}

	switch (message)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		RECT rct;
		FileNode* cnd;
		HBRUSH hbr;
		HGDIOBJ oob;
		::GetClientRect(hWnd, &rct);
		HDC hdc = BeginPaint(hWnd, &ps);
		//FillRect(hdc, &rct, view->hBackBrush);
		HBRUSH hob = (HBRUSH)SelectObject(hdc, view->hBackBrush);
		if (view) {
			if (::WaitForSingleObject(view->hEventView, 0) == WAIT_OBJECT_0) {
				if (view->viewlist.size() > 0) {
					for (std::list<VisualBlock*>::iterator itvb = view->viewlist.begin()
						; itvb != view->viewlist.end()
						; itvb++) {
						rct = (*itvb)->rect;
						rct.right++;
						rct.bottom++;
						//FrameRect(hdc, &rct, view->hBlackBrush);
						cnd = NULL;
						if ((*itvb)->nodes.size() > 0) {
							cnd = *(*itvb)->nodes.begin();
							hbr = ::CreateSolidBrush(cnd->color);
							oob = ::SelectObject(hdc, hbr);
							Rectangle(hdc, rct.left, rct.top, rct.right, rct.bottom);
							::SelectObject(hdc, oob);
							DeleteObject(hbr);
						}
						else {
							Rectangle(hdc, rct.left, rct.top, rct.right, rct.bottom);
						}
					}
				}
				else {
					FillRect(hdc, &rct, view->hBackBrush);
				}
				if (view->inBlock) {
					rct = view->inBlock->rect;
					rct.right++;
					rct.bottom++;
					FrameRect(hdc, &rct, view->hGrayBrush);
				}
				if (view->selectBlock) {
					rct = view->selectBlock->rect;
					rct.right++;
					rct.bottom++;
					FrameRect(hdc, &rct, view->hSelectBrush);
					InflateRect(&rct, -1, -1);
					FrameRect(hdc, &rct, view->hSelectBrush);
				}
				::SetEvent(view->hEventView);
			}
		}
		SelectObject(hdc, hob);
		//FillRect(hdc, &rct, hgb);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_SIZE:
		if (view) {
			view->resized = TRUE;

			if (view->hToolTip) {
				RECT rct = { 0 };
				rct.right = LOWORD(lParam);
				rct.bottom = HIWORD(lParam);

				SendMessage(view->hToolTip, TTM_ADJUSTRECT, FALSE, (LPARAM)& rct);
			}

		}
		break;
	case WM_ERASEBKGND:
		break;
	case WM_MOUSEMOVE:
		if (view) {
			VisualBlock* pblock = view->inBlock;
			RECT trct;
			if (::WaitForSingleObject(view->hEventView, 0) == WAIT_OBJECT_0) {
				int xPos = (int)(short)LOWORD(lParam);
				int yPos = (int)(short)HIWORD(lParam);

				if (view->inBlock) {
					trct = view->inBlock->rect;
					trct.right++;
					trct.bottom++;
					if (!PtInRect(&trct, { (LONG)xPos, (LONG)yPos })) {
						view->inBlock = NULL;
					}
				}
				if (view->inBlock == NULL) {
					std::list<VisualBlock*>::iterator itvb = view->viewlist.begin();
					BOOL keepseek = itvb != view->viewlist.end();
					while (keepseek) {
						trct = (*itvb)->rect;
						trct.right++;
						trct.bottom++;
						if (PtInRect(&trct, { xPos, yPos })) {
							view->inBlock = *itvb;
							if (view->inBlock->nodes.size() > 0) {
								view->SendNotifyHoverNode(*view->inBlock->nodes.begin());
							}
							keepseek = FALSE;
						}
						itvb++;
						keepseek = itvb == view->viewlist.end() ? FALSE : keepseek;
					}
				}
				if (view->inBlock) {
					RECT rct = view->inBlock->rect;
					rct.bottom++;
					rct.right++;
					InvalidateRect(hWnd, &rct, TRUE);
				}
				if (pblock && (view->inBlock != pblock)) {
					RECT rct = pblock->rect;
					rct.bottom++;
					rct.right++;
					InvalidateRect(hWnd, &rct, TRUE);
				}
				SetEvent(view->hEventView);
			}
		}
		break;
	case WM_LBUTTONDOWN:
		break;
	case WM_LBUTTONUP:
		if (view) {
			if (wParam & MK_SHIFT) {
				if (view->inBlock) {
					if (view->inBlock->nodes.size() > 0) {
						FileNode* fnd = *view->inBlock->nodes.begin();
						BOOL btn = OpenClipboard(hWnd);
						if (btn) {
							btn = EmptyClipboard();
							std::wstring pnm = FileNodeHelper::GetNodeFullPathName(fnd);
							int cch = pnm.size();
							HGLOBAL hgb = GlobalAlloc(GMEM_MOVEABLE, (cch + 1) * sizeof(WCHAR));
							if (hgb) {
								LPVOID lmm = GlobalLock(hgb);
								memcpy_s(lmm, (cch + 1)*sizeof(WCHAR), pnm.c_str(), (cch+1)*sizeof(WCHAR));
								GlobalUnlock(hgb);
								::SetClipboardData(CF_UNICODETEXT, hgb);
								HWND hpnt = ::GetParent(hWnd);
							}
							btn = CloseClipboard();
							if (hgb) {
								view->SendNotifyLogClickNode(fnd);
							}
						}
					}
				}
			}
		}
		break;
	case WM_LBUTTONDBLCLK:
		if (view) {
			if (::WaitForSingleObject(view->hEventView, INFINITE) == WAIT_OBJECT_0) {
				if (view->inBlock) {
					FileNode* fnd = *view->inBlock->nodes.begin();
					view->SendNotifySelectNode(fnd);
				}
				::SetEvent(view->hEventView);
			}
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return lrst;
}

ATOM CVisualBlockView::RegisterBlockClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc = WndBlockProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SIMPLEVISUALBLOCKS));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SIMPLEVISUALBLOCKS);
	wcex.lpszClassName = szBlockClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

int CVisualBlockView::SendNotifyLogClickNode(FileNode* node)
{
	HWND hpnt = ::GetParent(hWnd);
	NMVISUALBLOCK nmvb;

	nmvb.nmhdr.code = NMC_VISUALBLOCK_LOGCLICKNODE;
	nmvb.nmhdr.hwndFrom = hWnd;
	nmvb.node = node;

	SendNotifyMessage(hpnt, WM_NOTIFY, 0, (LPARAM)& nmvb);
	return 0;
}


int CVisualBlockView::SendNotifySelectNode(FileNode* node)
{
	HWND hpnt = ::GetParent(hWnd);
	NMVISUALBLOCK nmvb;

	nmvb.nmhdr.code = NMC_VISUALBLOCK_SELECTNODE;
	nmvb.nmhdr.hwndFrom = hWnd;
	nmvb.node = node;

	SendNotifyMessage(hpnt, WM_NOTIFY, 0, (LPARAM)& nmvb);
	return 0;
}

int CVisualBlockView::SendNotifyHoverNode(FileNode* node)
{
	HWND hpnt = ::GetParent(hWnd);
	NMVISUALBLOCK nmvb;

	nmvb.nmhdr.code = NMC_VISUALBLOCK_HOVERNODE;
	nmvb.nmhdr.hwndFrom = hWnd;
	nmvb.node = node;

	SendNotifyMessage(hpnt, WM_NOTIFY, 0, (LPARAM)& nmvb);
	return 0;
}

int CVisualBlockView::Clear()
{
	if (::WaitForSingleObject(hEventView, INFINITE) == WAIT_OBJECT_0) {
		inBlock = NULL;
		selectBlock = NULL;
		selectNode = NULL;

		CleanUpViewBlockMap();

		for (std::list<VisualBlock*>::iterator itvb = viewlist.begin()
			; itvb != viewlist.end()
			; itvb++)
		{
			delete* itvb;
		}
		viewlist.clear();
		viewnodeblockmap.clear();
		::SetEvent(hEventView);
		InvalidateRect(hWnd, NULL, TRUE);
	}
	return 0;
}

CVisualBlockView::~CVisualBlockView()
{
	::CloseHandle(hEventView);
	DeleteObject(hBlackBrush);
	DeleteObject(hGrayBrush);
	DeleteObject(hBackBrush);
	DeleteObject(hSelectBrush);
	if (colors) {
		delete[] colors;
		colors = NULL;
	}
}

HWND CVisualBlockView::InitiateToolTips(HWND hPnt)
{
	HINSTANCE hinst = ::GetModuleHandle(NULL);
	HWND hwndTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
		WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		hPnt, NULL,
		hinst, NULL);

	if (hwndTip)
	{
		SetWindowPos(hwndTip, HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		// Associate the tooltip with the tool.
		TOOLINFO toolInfo = { 0 };
//		toolInfo.cbSize = sizeof(toolInfo);
		toolInfo.cbSize = TTTOOLINFOA_V1_SIZE;
		toolInfo.hwnd = hPnt;
		toolInfo.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
		toolInfo.uId = (UINT_PTR)hPnt;
		toolInfo.hinst = hinst;
		toolInfo.lpszText = (wchar_t*)TEXT("Shift+CLICK to copy file path");
		GetClientRect(hPnt, &toolInfo.rect);
		SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)& toolInfo);
	}
	return hwndTip;
}

CVisualBlockView* CVisualBlockView::CreateVisualBlockView(HWND hParent)
{
	HINSTANCE hInst = ::GetModuleHandle(NULL);
	if (classBlock == 0)
	{
		LoadStringW(hInst, IDS_APP_TITLE, szBlockTitle, MAX_LOADSTRING);
		LoadStringW(hInst, IDC_VISUALBLOCKVIEW, szBlockClass, MAX_LOADSTRING);
		classBlock = RegisterBlockClass(hInst);
	}

	CVisualBlockView* view = new CVisualBlockView();
	view->hWnd = CreateWindow(szBlockClass, szBlockTitle, WS_CHILD | WS_VISIBLE,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, hParent, nullptr, hInst, nullptr);

	if (view->hWnd) {
		CVisualBlockView::views[view->hWnd] = view;
		view->hToolTip = view->InitiateToolTips(view->hWnd);
		ShowWindow(view->hWnd, TRUE);
		UpdateWindow(view->hWnd);
	}

	return view;
}

CVisualBlockView::operator HWND() const
{
	return hWnd;
}

RECT CVisualBlockView::GetRect()
{
	RECT rct = { 0 };
	if (hWnd && (hWnd != INVALID_HANDLE_VALUE)) {
		::GetClientRect(hWnd, &rct);
		rct.right--;
		rct.bottom--;
	}
	return rct;
}

int CVisualBlockView::Redraw()
{
	if (hWnd) {
		InvalidateRect(hWnd, NULL, TRUE);
	}
	return 0;
}

int CVisualBlockView::SetViewList(std::list<VisualBlock*>& vlst)
{
	FileNode* innode = NULL;
	VisualBlock* vblk;
	if (::WaitForSingleObject(hEventView, INFINITE) == WAIT_OBJECT_0) {
		if (inBlock) {
			if (inBlock->nodes.size() > 0) {
				innode = *inBlock->nodes.begin();
			}
		}
		inBlock = NULL;
		viewnodeblockmap.clear();
		for (std::list<VisualBlock*>::iterator itvb = viewlist.begin()
			; itvb != viewlist.end()
			; itvb++)
		{
			delete* itvb;
		}
		viewlist.clear();
		for (std::list<VisualBlock*>::iterator itvb = vlst.begin()
			; itvb != vlst.end()
			; itvb++)
		{
			vblk = new VisualBlock(**itvb);
			if (vblk->nodes.size() > 0) {
				viewnodeblockmap[*vblk->nodes.begin()] = vblk;
			}
			viewlist.push_back(vblk);
		}
		::SetEvent(hEventView);
		if (innode) {
			std::map<FileNode*, VisualBlock*>::iterator itnb = viewnodeblockmap.find(innode);
			if (itnb != viewnodeblockmap.end()) {
				inBlock = itnb->second;
			}
		}
	}
	return 0;
}

int CVisualBlockView::CleanUpViewBlockMap()
{
	VisualBlock* nvb;
	std::set<VisualBlock*> avb;

	for (std::map<FileNode*, VisualBlock*>::iterator itfv = allnodeblockmap.begin()
		; itfv != allnodeblockmap.end()
		; itfv++) {
		nvb = itfv->second;
		if (avb.find(nvb) == avb.end()) {
			avb.insert(nvb);
			delete nvb;
		}
	}
	avb.clear();
	allnodeblockmap.clear();

	return 0;
}

int CVisualBlockView::SetNodeList(std::list<VisualBlock*>& nlst)
{
	VisualBlock* nvb;
	if (::WaitForSingleObject(hEventView, INFINITE) == WAIT_OBJECT_0) {
		CleanUpViewBlockMap();

		//build view block map
		for (std::list<VisualBlock*>::iterator itvb = nlst.begin()
			; itvb != nlst.end()
			; itvb++) {
			if ((*itvb)->nodes.size() > 0) {
				nvb = new VisualBlock(**itvb);
				nvb->blocks.clear();
				for (std::list<FileNode*>::iterator itfn = (*itvb)->nodes.begin()
					; itfn != (*itvb)->nodes.end()
					; itfn++) {
					allnodeblockmap[*itfn] = nvb;
				}
			}
		}

		selectBlock = NULL;
		::SetEvent(hEventView);
	}

	if (selectNode) {
		SetSelectNode(selectNode);
	}
	return 0;
}

int CVisualBlockView::SetSelectNode(FileNode* node)
{
	selectNode = node;
	if (::WaitForSingleObject(hEventView, INFINITE) == WAIT_OBJECT_0) {
		if (allnodeblockmap.find(node) != allnodeblockmap.end()) {
			selectBlock = allnodeblockmap[node];
		}
		else {
			selectBlock = NULL;
		}
		::SetEvent(hEventView);
	}
	if (selectBlock == NULL) {
		if (node->parent) {
			SetSelectNode(node->parent);
		}
	}
	return 0;
}

BOOL CVisualBlockView::ResetResized()
{
	BOOL rtn = resized;
	resized = FALSE;
	return rtn;
}
