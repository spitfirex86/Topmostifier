#include "framework.h"
#include "resource.h"
#include "topmostifier.h"


#define WM_APP_TRAY		(WM_APP + 1)
#define WM_HK_QUICK		(WM_APP + 2)

char const g_szAppName[] = "Spitfire's Topmostifier(tm)";
char const g_szShortName[] = "Topmostifier";


HINSTANCE g_hInst = NULL;
HWND g_hWnd = NULL;

HWND g_hListWnd;
HWND g_hAlpha;
HWND g_hTopmost;

HICON g_hIcon;
HICON g_hIconSmall;
HMENU g_hMenu;

int g_lSelIdx = LB_ERR;
HWND g_hSelWnd = NULL;


BOOL fn_bCreateTrayIcon( HWND hWnd )
{
	NOTIFYICONDATA nid = { 0 };
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = IDI_APP;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_APP_TRAY;
	nid.hIcon = g_hIconSmall;
	strcpy(nid.szTip, g_szShortName);

	return Shell_NotifyIcon(NIM_ADD, &nid);
}

BOOL fn_vDeleteTrayIcon( HWND hWnd )
{
	NOTIFYICONDATA nid = { 0 };
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = IDI_APP;

	return Shell_NotifyIcon(NIM_DELETE, &nid);
}

BOOL fn_bProcessTrayMsg( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
	switch ( lParam )
	{
		case WM_LBUTTONUP:
			ShowWindow(hWnd, SW_SHOW);
			SetForegroundWindow(hWnd);
			return TRUE;

		case WM_RBUTTONUP:
		{
			POINT stClick = { 0 };
			GetCursorPos(&stClick);

			HMENU hTrayMenu = GetSubMenu(g_hMenu, 0);
			SetMenuDefaultItem(hTrayMenu, 0, TRUE);

			/* hack to correctly handle focus on the popup menu.
			   as described in the "remarks" section of the TrackPopupMenu docs */
			SetForegroundWindow(hWnd);
			TrackPopupMenu(hTrayMenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN, stClick.x, stClick.y, 0, hWnd, NULL);
			PostMessage(hWnd, WM_NULL, 0, 0);
		}
			return TRUE;
	}

	return FALSE;
}

void fn_vGetSelectionInfo( void )
{
	g_lSelIdx = ListBox_GetCurSel(g_hListWnd);
	g_hSelWnd = (g_lSelIdx != LB_ERR)
		? (HWND)ListBox_GetItemData(g_hListWnd, g_lSelIdx)
		: NULL;
}

void fn_vUpdateControls( void )
{
	fn_vGetSelectionInfo();
	if ( g_hSelWnd )
	{
		EnableWindow(g_hAlpha, TRUE);
		EnableWindow(g_hTopmost, TRUE);

		Button_SetCheck(g_hTopmost, fn_bGetTopmost(g_hSelWnd));
		SendMessage(g_hAlpha, TBM_SETPOS, TRUE, fn_ucGetAlpha(g_hSelWnd));
	}
	else
	{
		EnableWindow(g_hAlpha, FALSE);
		EnableWindow(g_hTopmost, FALSE);

		Button_SetCheck(g_hTopmost, FALSE);
		SendMessage(g_hAlpha, TBM_SETPOS, TRUE, 0);
	}
}

BOOL fn_bProcessCmds( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
	switch ( LOWORD(wParam) )
	{
		case IDC_TOPMOST:
			if ( g_hSelWnd )
				fn_vSetTopmost(g_hSelWnd, Button_GetCheck(g_hTopmost));
			return TRUE;

		case IDC_REFRESH:
			fn_vGetAllWindows(g_hListWnd);
			fn_vUpdateControls();
			return TRUE;

		case IDC_WNDLIST:
			if ( HIWORD(wParam) == LBN_SELCHANGE )
			{
				fn_vUpdateControls();
				return TRUE;
			}
			break;

		case IDM_SHOW:
			ShowWindow(hWnd, SW_SHOW);
			return TRUE;

		case IDC_EXIT:
		case IDM_EXIT:
			DestroyWindow(hWnd);
			return TRUE;

		/*case IDOK:*/
		case IDCANCEL:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			return TRUE;
	}

	return FALSE;
}

BOOL CALLBACK fn_bTopmostifierDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static UINT s_uTaskbarMsg;

	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)g_hIcon);
			SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hIconSmall);

			fn_bCreateTrayIcon(hWnd);
			/* needed to handle re-creating tray icon if explorer restarts */
			s_uTaskbarMsg = RegisterWindowMessage("TaskbarCreated");
				
			g_hListWnd = GetDlgItem(hWnd, IDC_WNDLIST);
			g_hAlpha = GetDlgItem(hWnd, IDC_ALPHASLIDER);
			g_hTopmost = GetDlgItem(hWnd, IDC_TOPMOST);

			SendMessage(g_hAlpha, TBM_SETRANGE, TRUE, MAKELPARAM(C_MinAlpha, C_MaxAlpha));
			SendMessage(g_hAlpha, TBM_SETPAGESIZE, 0, C_AlphaStep);
			SendMessage(g_hAlpha, TBM_SETPOS, TRUE, C_MaxAlpha);

			/* TODO: make this customizable */
			RegisterHotKey(hWnd, WM_HK_QUICK, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, 'W');

			return TRUE;
		}

		case WM_SHOWWINDOW:
			if ( wParam == TRUE && lParam == 0 )
			{
				fn_vGetAllWindows(g_hListWnd);
				SetFocus(g_hListWnd);
				fn_vUpdateControls();
			}
			return TRUE;

		case WM_HSCROLL:
			if ( (HWND)lParam == g_hAlpha )
			{
				if ( g_hSelWnd )
				{
					int pos = SendMessage(g_hAlpha, TBM_GETPOS, 0, 0);
					fn_vSetAlpha(g_hSelWnd, pos);
					//fn_vUpdateControls();
				}
				return TRUE;
			}
			break;

		case WM_HOTKEY:
			if ( wParam == WM_HK_QUICK )
			{
				fn_vShowQuickWnd();
				return TRUE;
			}
			break;

		case WM_APP_TRAY:
			return fn_bProcessTrayMsg(hWnd, wParam, lParam);

		case WM_COMMAND:
			return fn_bProcessCmds(hWnd, wParam, lParam);

		case WM_CLOSE:
			ShowWindow(hWnd, SW_HIDE);
			return TRUE;

		case WM_DESTROY:
			fn_vDestroyQuickWnd();
			UnregisterHotKey(hWnd, WM_HK_QUICK);
			fn_vDeleteTrayIcon(hWnd);
			PostQuitMessage(0);
			return TRUE;

		default:
			if ( uMsg == s_uTaskbarMsg )
			{
				/* explorer died and/or was restarted, create the icon again */
				fn_bCreateTrayIcon(hWnd);
				return TRUE;
			}
			break;
	}

	return FALSE;
}

void fn_vLoadRes( void )
{
	g_hIcon = LoadImage(g_hInst, MAKEINTRESOURCE(IDI_APP), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
	g_hIconSmall = LoadImage(g_hInst, MAKEINTRESOURCE(IDI_APP), IMAGE_ICON, 16, 16, 0);
	g_hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_TRAYMENU));
}

void fn_vDestroyRes( void )
{
	DestroyIcon(g_hIcon);
	DestroyIcon(g_hIconSmall);
	DestroyMenu(g_hMenu);
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow )
{
	HWND hDlg;
	MSG msg;

	HANDLE hMutex = CreateMutex(NULL, TRUE, "SpitfiresTopmostifierSI");
	if ( !hMutex || GetLastError() == ERROR_ALREADY_EXISTS )
	{
		HWND hWndExisting = FindWindow(NULL, g_szAppName);
		if ( hWndExisting )
		{
			ShowWindow(hWndExisting, SW_SHOW);
			SetForegroundWindow(hWndExisting);
		}
		return 0;
	}

	g_hInst = hInstance;
	fn_vLoadRes();

	/* TODO: more cmdline switches could be useful */
	if ( strstr(lpCmdLine, "-q") )
		nCmdShow = SW_HIDE;

	hDlg = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, fn_bTopmostifierDlgProc);
	if ( hDlg == NULL )
		return 1;

	g_hWnd = hDlg;

	SetWindowText(hDlg, g_szAppName);
	ShowWindow(hDlg, nCmdShow);

	while ( GetMessage(&msg, NULL, 0, 0) > 0 )
	{
		HWND hActiveDlg = g_hQuickWnd ? g_hQuickWnd : hDlg;
		if ( !IsDialogMessage(hActiveDlg, &msg) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	fn_vDestroyRes();
	return 0;
}
