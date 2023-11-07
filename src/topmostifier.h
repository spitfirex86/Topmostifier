#pragma once


#define C_MinAlpha		0xF
#define C_MaxAlpha		0xFF
#define C_AlphaStep		0xF

#define M_IsAlpha(ucAlpha) ((ucAlpha)<C_MaxAlpha)


#define Trackbar_GetPos(hwndCtl) ((int)SNDMSG((hwndCtl), TBM_GETPOS, 0, 0))
#define Trackbar_SetPos(hwndCtl, pos) ((void)SNDMSG((hwndCtl), TBM_SETPOS, TRUE, (LPARAM)(pos)))
#define Trackbar_SetRange(hwndCtl, uwMin, uwMax) ((void)SNDMSG((hwndCtl), TBM_SETRANGE, TRUE, MAKELPARAM((uwMin), (uwMax))))
#define Trackbar_SetPageSize(hwndCtl, size) ((int)SNDMSG((hwndCtl), TBM_SETPAGESIZE, 0, (LPARAM)(size)))


/*
 * main.c
 */

extern HINSTANCE g_hInst;
extern HWND g_hWnd;

void fn_vUpdateControls( void );


/*
 * tricks.c
 */

void fn_vGetAllWindows( HWND hList );

unsigned char fn_ucGetAlpha( HWND hWnd );
void fn_vSetAlpha( HWND hWnd, unsigned char ucAlpha );

BOOL fn_bGetTopmost( HWND hWnd );
void fn_vSetTopmost( HWND hWnd, BOOL bTopmost );


/*
 * quick.c
 */

extern HWND g_hQuickWnd;

void fn_vShowQuickWnd( void );
void fn_vDestroyQuickWnd( void );
