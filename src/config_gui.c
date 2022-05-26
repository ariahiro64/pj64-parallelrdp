#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <stdio.h>

#include "config_gui_resources.h"
#include "config_gui.h"
#include "config.h"

struct res_t
{
	int w;
	int h;
};

HMODULE config_gui_hInstance;

#define MAXNUMRES 4096
char* selectedAspect     = "";
char* currentIndexAspect = "";
char* listItemAspect     = "";
int currentIndexResolution = 0;
int nRes = 0;
struct res_t ress[MAXNUMRES];

// Get the horizontal and vertical screen sizes in pixel
void GetDesktopResolution(int horizontal, int vertical)
{
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	horizontal = desktop.right;
	vertical = desktop.bottom;
}

BOOL CALLBACK DlgFunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{



	case WM_INITDIALOG:
	{
		SetWindowText(hWnd, (LPCSTR)"paraLLEl-RDP config");

		char s[500];
		int i = 0;

		DEVMODE d;
		EnumDisplaySettings(NULL, 0, &d);
		while (1)
		{
			BOOL h = EnumDisplaySettings(NULL, i++, &d);
			if (!h || nRes > MAXNUMRES)
				break;
			if (d.dmBitsPerPel != 32)
				continue;

			if (!nRes || ress[nRes - 1].w != d.dmPelsWidth || ress[nRes - 1].h != d.dmPelsHeight)
			{
				ress[nRes].w = d.dmPelsWidth;
				ress[nRes].h = d.dmPelsHeight;

				nRes++;
				sprintf(s, "%ld X %ld", d.dmPelsWidth, d.dmPelsHeight);
				SendDlgItemMessage(hWnd, ComboResolution, CB_ADDSTRING, 0, (LPARAM)s);
			}
		}



		for (i = 0; i < nRes; i++)
			if (ress[i].w == settings[KEY_SCREEN_WIDTH].val && ress[i].h == settings[KEY_SCREEN_HEIGHT].val)
				SendDlgItemMessage(hWnd, ComboResolution, CB_SETCURSEL, i, 0);
		currentIndexResolution = SendDlgItemMessage(hWnd, ComboResolution, CB_GETCURSEL, 0, 0);

		if (settings[KEY_FULLSCREEN].val)
			SendDlgItemMessage(hWnd, CheckFullscreen, BM_SETCHECK, 1, 1);

		///////////////////////// get the aspect ratio of current set resolution and set the selection /////////////////////////////
		int	AspectCount = SendDlgItemMessage(hWnd, ComboAspect, CB_GETCOUNT, 0, 0);

		for (int itemIndex = 0; itemIndex < AspectCount; itemIndex++)
		{
			SendDlgItemMessage(hWnd, ComboAspect, CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)listItemAspect);

			if (!strcmp(listItemAspect, selectedAspect))
			{
				SendDlgItemMessage(hWnd, ComboAspect, CB_SETCURSEL, itemIndex, 0);
			}
		}
	}

	SendDlgItemMessage(hWnd, ComboUpscaler, CB_ADDSTRING, 0, (LPARAM)"None");
	SendDlgItemMessage(hWnd, ComboUpscaler, CB_ADDSTRING, 0, (LPARAM)"2x");
	SendDlgItemMessage(hWnd, ComboUpscaler, CB_ADDSTRING, 0, (LPARAM)"4x");
	SendDlgItemMessage(hWnd, ComboUpscaler, CB_ADDSTRING, 0, (LPARAM)"8x (hi-end GPUs only");

	SendDlgItemMessage(hWnd, ComboUpscaler2, CB_ADDSTRING, 0, (LPARAM)"None");
	SendDlgItemMessage(hWnd, ComboUpscaler2, CB_ADDSTRING, 0, (LPARAM)"1/2");
	SendDlgItemMessage(hWnd, ComboUpscaler2, CB_ADDSTRING, 0, (LPARAM)"1/4");
	SendDlgItemMessage(hWnd, ComboUpscaler2, CB_ADDSTRING, 0, (LPARAM)"1/8");

	SendDlgItemMessage(hWnd, ComboDeinterlace, CB_ADDSTRING, 0, (LPARAM)"Bob");
	SendDlgItemMessage(hWnd, ComboDeinterlace, CB_ADDSTRING, 0, (LPARAM)"Weave");

	SendDlgItemMessage(hWnd, ComboUpscaler2, CB_SETCURSEL, settings[KEY_DOWNSCALING].val, 0);


	switch (settings[KEY_UPSCALING].val)
	{
	case 0:
	case 1:
		SendDlgItemMessage(hWnd, ComboUpscaler, CB_SETCURSEL, 0, 0);
		break;
	case 2:
		SendDlgItemMessage(hWnd, ComboUpscaler, CB_SETCURSEL, 1, 0);
		break;
	case 4:
		SendDlgItemMessage(hWnd, ComboUpscaler, CB_SETCURSEL, 2, 0);
		break;
	case 8:
		SendDlgItemMessage(hWnd, ComboUpscaler, CB_SETCURSEL, 3, 0);
		break;
	}

	SendDlgItemMessage(hWnd, ComboDeinterlace, CB_SETCURSEL, settings[KEY_DEINTERLACE].val, 0);

	SendDlgItemMessage(hWnd, SSREADS, BM_SETCHECK, settings[KEY_SSREADBACKS].val, 0);
	SendDlgItemMessage(hWnd, CheckFullscreen, BM_SETCHECK, settings[KEY_FULLSCREEN].val, 0);
	SendDlgItemMessage(hWnd, SSDITHER, BM_SETCHECK, settings[KEY_SSDITHER].val, 0);
	SendDlgItemMessage(hWnd, INTEGERSCALE, BM_SETCHECK, settings[KEY_INTEGER].val, 0);
	SendDlgItemMessage(hWnd, VIBilinear, BM_SETCHECK, settings[KEY_VIBILERP].val, 0);
	SendDlgItemMessage(hWnd, VIDIVOT, BM_SETCHECK, settings[KEY_DIVOT].val, 0);
	SendDlgItemMessage(hWnd, VIAA, BM_SETCHECK, settings[KEY_AA].val, 0);
	SendDlgItemMessage(hWnd, VDEDITHER, BM_SETCHECK, settings[KEY_VIDITHER].val, 0);
	SendDlgItemMessage(hWnd, NATIVETEXLOD, BM_SETCHECK, settings[KEY_NATIVETEXTLOD].val, 0);
	SendDlgItemMessage(hWnd, NATIVETEXRECT, BM_SETCHECK, settings[KEY_NATIVETEXTRECT].val, 0);
	SendDlgItemMessage(hWnd, CheckVerticalSync, BM_SETCHECK, settings[KEY_VSYNC].val, 0);
	SendDlgItemMessage(hWnd, CheckSynchronous, BM_SETCHECK, settings[KEY_SYNCHRONOUS].val, 0);
	SendDlgItemMessage(hWnd, CheckWidescreen, BM_SETCHECK, settings[KEY_WIDESCREEN].val, 0);

	break;


	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ButtonRun:
		{
			int lut[4] = { 1,2,4,8 };

			int ups = SendDlgItemMessage(hWnd, ComboUpscaler, CB_GETCURSEL, 0, 0);
			settings[KEY_UPSCALING].val = lut[ups];
			settings[KEY_DEINTERLACE].val = SendDlgItemMessage(hWnd, ComboDeinterlace, CB_GETCURSEL, 0, 0);
			settings[KEY_SSREADBACKS].val = SendDlgItemMessage(hWnd, SSREADS, BM_GETCHECK, 0, 0);
			settings[KEY_FULLSCREEN].val = SendDlgItemMessage(hWnd, CheckFullscreen, BM_GETCHECK, 0, 0);
			settings[KEY_SSDITHER].val = SendDlgItemMessage(hWnd, SSDITHER, BM_GETCHECK, 0, 0);
			settings[KEY_INTEGER].val = SendDlgItemMessage(hWnd, INTEGERSCALE, BM_GETCHECK, 0, 0);
			settings[KEY_VIBILERP].val = SendDlgItemMessage(hWnd, VIBilinear, BM_GETCHECK, 0, 0);
			settings[KEY_DIVOT].val = SendDlgItemMessage(hWnd, VIDIVOT, BM_GETCHECK, 0, 0);
			settings[KEY_AA].val = SendDlgItemMessage(hWnd, VIAA, BM_GETCHECK, 0, 0);
			settings[KEY_VIDITHER].val = SendDlgItemMessage(hWnd, VDEDITHER, BM_GETCHECK, 0, 0);
			settings[KEY_NATIVETEXTLOD].val = SendDlgItemMessage(hWnd, NATIVETEXLOD, BM_GETCHECK, 0, 0);
			settings[KEY_NATIVETEXTRECT].val = SendDlgItemMessage(hWnd, NATIVETEXRECT, BM_GETCHECK, 0, 0);
			settings[KEY_SCREEN_WIDTH].val = ress[SendDlgItemMessage(hWnd, ComboResolution, CB_GETCURSEL, 0, 0)].w;
			settings[KEY_SCREEN_HEIGHT].val = ress[SendDlgItemMessage(hWnd, ComboResolution, CB_GETCURSEL, 0, 0)].h;
			settings[KEY_FULLSCREEN].val = SendDlgItemMessage(hWnd, CheckFullscreen, BM_GETCHECK, 0, 0);
			settings[KEY_VSYNC].val = SendDlgItemMessage(hWnd, CheckVerticalSync, BM_GETCHECK, 0, 0);
			settings[KEY_SYNCHRONOUS].val = SendDlgItemMessage(hWnd, CheckSynchronous, BM_GETCHECK, 0, 0);
			settings[KEY_DOWNSCALING].val = SendDlgItemMessage(hWnd, ComboUpscaler2, CB_GETCURSEL, 0, 0);
			settings[KEY_WIDESCREEN].val = SendDlgItemMessage(hWnd, CheckWidescreen, BM_GETCHECK, 0, 0);
			config_save();
			EndDialog(hWnd, TRUE);
		} break;
		case ButtonQuit:
		{
			EndDialog(hWnd, FALSE);
		} break;

		} break;
	}
	return (WM_INITDIALOG == uMsg) ? TRUE : FALSE;
}

void config_gui_open(HWND hParent)
{
	InitCommonControls(); // Used for visual styles
	DialogBox(config_gui_hInstance, MAKEINTRESOURCE(FormKickstart), hParent, DlgFunc);
}
