#include "framework.h"
#include "topmostifier.h"


BOOL CALLBACK fn_bEnumWndProc( HWND hWnd, LPARAM lParam )
{
	char szTitle[256];

	HWND hList = (HWND)lParam;

	GetWindowText(hWnd, szTitle, sizeof(szTitle));

	if ( IsWindowVisible(hWnd) && *szTitle )
	{
		int idx = ListBox_AddString(hList, szTitle);
		ListBox_SetItemData(hList, idx, hWnd);
	}

	return TRUE;
}

void fn_vGetAllWindows( HWND hList )
{
	ListBox_ResetContent(hList);
	EnumDesktopWindows(NULL, fn_bEnumWndProc, (LPARAM)hList);
}


unsigned char fn_ucGetAlpha( HWND hWnd )
{
	unsigned char ucAlphaTmp;
	BOOL bIsAlpha = GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_LAYERED;

	if ( bIsAlpha && GetLayeredWindowAttributes(hWnd, NULL, &ucAlphaTmp, NULL) )
		return ucAlphaTmp;

	return C_MaxAlpha;
}

void fn_vSetAlpha( HWND hWnd, unsigned char ucAlpha )
{
	long lNewLong = (M_IsAlpha(ucAlpha))
		? GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED
		: GetWindowLong(hWnd, GWL_EXSTYLE) & ~WS_EX_LAYERED;

	SetWindowLong( hWnd, GWL_EXSTYLE, lNewLong);
	SetLayeredWindowAttributes(hWnd, 0, ucAlpha, LWA_ALPHA);
}


BOOL fn_bGetTopmost( HWND hWnd )
{
	return GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST;
}

void fn_vSetTopmost( HWND hWnd, BOOL bTopmost )
{
	SetWindowPos(hWnd, (bTopmost) ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}
