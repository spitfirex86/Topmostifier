#include "framework.h"
#include "resource.h"
#include "topmostifier.h"


HWND g_hQuickWnd = NULL;
HWND g_hOther = NULL;

HWND g_hQAlpha;
HWND g_hQTopmost;


void fn_vUpdateMainControls( void )
{
	if ( g_hWnd )
		fn_vUpdateControls();
}

BOOL fn_bProcessCmdsQuick( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
	switch ( LOWORD(wParam) )
	{
		case IDC_TOPMOST:
			fn_vSetTopmost(g_hOther, Button_GetCheck(g_hQTopmost));
			SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			fn_vUpdateMainControls();
			return TRUE;

		case IDOK:
		case IDCANCEL:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			return TRUE;
	}

	return FALSE;
}

BOOL CALLBACK fn_bQuickDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			g_hOther = (HWND)lParam;
			g_hQAlpha = GetDlgItem(hWnd, IDC_ALPHASLIDER);
			g_hQTopmost = GetDlgItem(hWnd, IDC_TOPMOST);

			RECT stRect = { 0 }, stOwnRect = { 0 }, stBorders = { 0 };
			GetWindowRect(g_hOther, &stRect);
			GetWindowRect(hWnd, &stOwnRect);
			AdjustWindowRect(&stBorders, GetWindowLong(g_hOther, GWL_STYLE), FALSE);
			stRect.right -= stOwnRect.right - stOwnRect.left + stBorders.right;
			SetWindowPos(hWnd, HWND_TOPMOST, stRect.right, stRect.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);

			char szTitle[256];
			GetWindowText(g_hOther, szTitle, sizeof(szTitle));
			SetDlgItemText(hWnd, IDC_APPNAME, szTitle);

			SendMessage(g_hQAlpha, TBM_SETRANGE, TRUE, MAKELPARAM(C_MinAlpha, C_MaxAlpha));
			SendMessage(g_hQAlpha, TBM_SETPAGESIZE, 0, C_AlphaStep);

			Button_SetCheck(g_hQTopmost, fn_bGetTopmost(g_hOther));
			SendMessage(g_hQAlpha, TBM_SETPOS, TRUE, fn_ucGetAlpha(g_hOther));

			return TRUE;
		}

		case WM_ACTIVATE:
			if ( wParam == WA_INACTIVE )
			{
				SendMessage(hWnd, WM_CLOSE, 0, 0);
			}
			return TRUE;

		case WM_HSCROLL:
			if ( (HWND)lParam == g_hQAlpha )
			{
				int pos = SendMessage(g_hQAlpha, TBM_GETPOS, 0, 0);
				fn_vSetAlpha(g_hOther, pos);
				fn_vUpdateMainControls();
				return TRUE;
			}
			break;

		case WM_COMMAND:
			return fn_bProcessCmdsQuick(hWnd, wParam, lParam);

		case WM_CLOSE:
			DestroyWindow(hWnd);
			return TRUE;

		case WM_DESTROY:
			g_hOther = NULL;
			g_hQAlpha = NULL;
			g_hQTopmost = NULL;
			g_hQuickWnd = NULL;
			return TRUE;
	}

	return FALSE;
}

void fn_vShowQuickWnd( void )
{
	HWND hOther = GetForegroundWindow();
	if ( !hOther || g_hQuickWnd ) 
		return;

	HWND hDlg = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_QUICK), NULL, fn_bQuickDlgProc, (LPARAM)hOther);
	if ( !hDlg )
		return;

	g_hQuickWnd = hDlg;
	ShowWindow(hDlg, SW_SHOW);
	SetForegroundWindow(hDlg);
}

void fn_vDestroyQuickWnd( void )
{
	if ( g_hQuickWnd )
		DestroyWindow(g_hQuickWnd);
}

