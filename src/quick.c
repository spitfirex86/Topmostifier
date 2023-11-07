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

void fn_vRepositionQuickWnd( HWND hWnd, HWND hOther )
{
	RECT stRect = { 0 }, stOwnRect = { 0 }, stBorders = { 0 };
	GetWindowRect(hOther, &stRect);
	GetWindowRect(hWnd, &stOwnRect);
	AdjustWindowRect(&stBorders, GetWindowLong(hOther, GWL_STYLE), FALSE);

	long lX = stRect.right - (stOwnRect.right - stOwnRect.left) - stBorders.right;
	long lY = (stRect.top >= 0) ? stRect.top : 0; /* necessary for maximized windows */

	SetWindowPos(hWnd, HWND_TOPMOST, lX, lY, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
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

			fn_vRepositionQuickWnd(hWnd, g_hOther);

			char szTitle[256];
			GetWindowText(g_hOther, szTitle, sizeof(szTitle));
			SetDlgItemText(hWnd, IDC_APPNAME, szTitle);

			Trackbar_SetRange(g_hQAlpha, C_MinAlpha, C_MaxAlpha);
			Trackbar_SetPageSize(g_hQAlpha, C_AlphaStep);

			Button_SetCheck(g_hQTopmost, fn_bGetTopmost(g_hOther));
			Trackbar_SetPos(g_hQAlpha, fn_ucGetAlpha(g_hOther));

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
				int pos = Trackbar_GetPos(g_hQAlpha);
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

