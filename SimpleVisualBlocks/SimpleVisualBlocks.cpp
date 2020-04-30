// SimpleVisualBlocks.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "SimpleVisualBlocks.h"

#include "CWindowView.h"



// Global Variables:
//HINSTANCE hInst;                                // current instance

//// Forward declarations of functions included in this code module:
//ATOM                MyRegisterClass(HINSTANCE hInstance);
//BOOL                InitInstance(HINSTANCE, int);
//LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
//INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	CWindowView::GetViewInstance(hInstance);
	CWindowView::WaitAllWindows();
}

int Log(const wchar_t* msg)
{
	return 0;

	WCHAR logsbuf[2048];
	SYSTEMTIME stm;
	::GetLocalTime(&stm);
	wsprintf(logsbuf, L"[%4d-%02d-%02d %02d:%02d:%02d.%03d] %s\n", stm.wYear, stm.wMonth, stm.wDay, stm.wHour, stm.wMinute, stm.wSecond, stm.wMilliseconds, msg);
	::OutputDebugString(logsbuf);

	return 0;
}


