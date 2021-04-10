#include "Main.h"
#define INI_IMPLEMENTATION
#define INI_STRNICMP(s1, s2, cnt) (strcmp(s1, s2))
#include "ini.h"
#include "KickstartResource.h"

#define MAXNUMRES 4096
string selectedAspect = "";
string currentIndexAspect = "";
string  listItemAspect = "";
int currentIndexResolution = 0;
int nRes = 0;
RES ress[MAXNUMRES];

HBRUSH g_hbrBackground = CreateSolidBrush(RGB(0, 0, 0));		// Black background color
HBRUSH g_hbrForeground = CreateSolidBrush(RGB(255, 255, 255));	// White foreground color
HBRUSH hBrush = NULL;

/*
struct Settings {
	int windowWidth;
	int windowHeight;
	int windowBPP;
	int Fullscreen;
	int Vsync;
	int vk_overscan;
	int vk_downscaling_steps;
	int vk_native_texture_lod;
	int vk_native_tex_rect;
	int vk_divot_filter;
	int vk_gamma_dither;
	int vk_vi_aa;
	int vk_vi_scale;
	int vk_dither_filter;
	int vk_interlacing;
};*/


/*
   ConfigOpenSection("Video-Parallel", &configVideoParallel);
	ConfigSetDefaultBool(configVideoParallel, KEY_FULLSCREEN, 0, "Use fullscreen mode if True, or windowed mode if False");
	ConfigSetDefaultInt(configVideoParallel, KEY_UPSCALING, 0, "Amount of rescaling: 0=None, 2=2x, 4=4x, 8=8x");
	ConfigSetDefaultInt(configVideoParallel, KEY_SCREEN_WIDTH, 1024, "Screen width");
	ConfigSetDefaultInt(configVideoParallel, KEY_SCREEN_HEIGHT, 768, "Screen width");
	ConfigSetDefaultBool(configVideoParallel, KEY_SSREADBACKS, 0, "Enable superscaling of readbacks when upsampling");
	ConfigSetDefaultBool(configVideoParallel, KEY_SSDITHER, 0, "Enable superscaling of dithering when upsampling");

	ConfigSetDefaultBool(configVideoParallel, KEY_DEINTERLACE, 0, "Deinterlacing method. Weave should only be used with 1x scaling factor. False=Bob, True=Weave");
	ConfigSetDefaultBool(configVideoParallel, KEY_INTEGER, 0, "Enable integer scaling");
	ConfigSetDefaultInt(configVideoParallel, KEY_OVERSCANCROP, 0, "Amount of overscan pixels to crop");
	ConfigSetDefaultBool(configVideoParallel, KEY_AA, 1, "VI anti-aliasing, smooths polygon edges.");
	ConfigSetDefaultBool(configVideoParallel, KEY_DIVOT, 1, "Allow VI divot filter, cleans up stray black pixels.");
	ConfigSetDefaultBool(configVideoParallel, KEY_GAMMADITHER, 1, "Allow VI gamma dither.");
	ConfigSetDefaultBool(configVideoParallel, KEY_VIBILERP, 1, "Allow VI bilinear scaling.");
	ConfigSetDefaultBool(configVideoParallel, KEY_VIDITHER, 1, "Allow VI dedither.");
	ConfigSetDefaultInt(configVideoParallel, KEY_DOWNSCALE, 0, "Downsampling factor, Downscales output after VI, equivalent to SSAA. 0=disabled, 1=1/2, 2=1/4, 3=1/8");
	ConfigSetDefaultBool(configVideoParallel, KEY_NATIVETEXTLOD, 0, "Use native texture LOD computation when upscaling, effectively a LOD bias.");
	ConfigSetDefaultBool(configVideoParallel, KEY_NATIVETEXTRECT, 1, "Native resolution TEX_RECT. TEX_RECT primitives should generally be TEX_RECT primitives should generally be rendered at native resolution to avoid seams.");
	ConfigSaveSection("Video-Parallel");
*/

#define KEY_FULLSCREEN 0
#define KEY_UPSCALING 1
#define KEY_SCREEN_WIDTH 2
#define KEY_SCREEN_HEIGHT 3
#define KEY_SSREADBACKS 4 
#define KEY_SSDITHER 5
#define KEY_DEINTERLACE 6
#define KEY_INTEGER 7
#define KEY_OVERSCANCROP 8
#define KEY_AA 9
#define KEY_DIVOT 10
#define KEY_GAMMADITHER 11 
#define KEY_VIBILERP 12
#define KEY_VIDITHER 13 
#define KEY_NATIVETEXTLOD 14
#define KEY_NATIVETEXTRECT 15
#define KEY_VSYNC 16
#define KEY_DOWNSCALING 17
#define NUM_CONFIGVARS 18
settingkey setting_defaults[NUM_CONFIGVARS]
{
	{"KEY_FULLSCREEN", 0},
	{"KEY_UPSCALING", 0},
	{"KEY_SCREEN_WIDTH", 640},
	{"KEY_SCREEN_HEIGHT",480},
	{"KEY_SSREADBACKS", 0},
	{"KEY_SSDITHER", 0},
	{"KEY_DEINTERLACE", 0},
	{"KEY_INTEGER", 0},
	{"KEY_OVERSCANCROP", 0},
	{"KEY_AA", 0},
	{"KEY_DIVOT", 1},
	{"KEY_GAMMADITHER", 1},
	{"KEY_VIBILERP", 1},
	{"KEY_VIDITHER", 1},
	{"KEY_NATIVETEXTLOD", 0},
	{"KEY_NATIVETEXTRECT", 1},
	{"KEY_VSYNC", 1},
	{"KEY_DOWNSCALE", 1}
};

void save_coresettings()
{
	FILE* fp = _wfopen(L"parasettings.ini", L"r");
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* data = (char*)malloc(size + 1);
	memset(data, 0, size + 1);
	fread(data, 1, size, fp);
	fclose(fp);
	ini_t* ini = ini_load(data, NULL);
	free(data);
	
	int section =
		ini_find_section(ini, "Settings", strlen("Settings"));
	int vars_infile = ini_property_count(ini, section);

	if (vars_infile != NUM_CONFIGVARS) {
		fclose(fp);
	}
	for (int i = 0; i < NUM_CONFIGVARS; i++) {
		char snum[10];
		int num = setting_defaults[i].val;
		itoa(num, snum, 10);
		int idx = ini_find_property(ini, section, (char*)setting_defaults[i].name, strlen(setting_defaults[i].name));
		ini_property_value_set(ini, section, idx, snum, strlen(snum));
	}
	size = ini_save(ini, NULL, 0); // Find the size needed
	char* data2 = (char*)malloc(size);
	size = ini_save(ini, data2, size); // Actually save the file
	fp = _wfopen(L"parasettings.ini", L"w");
	fwrite(data, 1, size, fp);
	fclose(fp);
	free(data2);
	ini_destroy(ini);
}

void init_coresettings() {
	FILE* fp = _wfopen(L"parasettings.ini", L"r");
	if (!fp) {
		// create a new file with defaults
		ini_t* ini = ini_create(NULL);
		int section =
			ini_section_add(ini, "Settings", strlen("Settings"));
		for (int i = 0; i < NUM_CONFIGVARS; i++) {
			char snum[10];
			int num = setting_defaults[i].val;
			itoa(num, snum, 10);
			ini_property_add(ini, section, (char*)setting_defaults[i].name, strlen(setting_defaults[i].name), (char*)snum, strlen(snum));
		}
		int size = ini_save(ini, NULL, 0); // Find the size needed
		char* data = (char*)malloc(size);
		size = ini_save(ini, data, size); // Actually save the file
		ini_destroy(ini);
		fp = _wfopen(L"parasettings.ini", L"w");
		fwrite(data, 1, size, fp);
		fclose(fp);
		free(data);
	}
	else {
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char* data = (char*)malloc(size + 1);
		memset(data, 0, size + 1);
		fread(data, 1, size, fp);
		fclose(fp);
		ini_t* ini = ini_load(data, NULL);
		free(data);
		
		int section =
			ini_find_section(ini, "Settings", strlen("Settings"));
		int vars_infile = ini_property_count(ini, section);

		if (vars_infile != NUM_CONFIGVARS) {
			fclose(fp);
		}
		bool save = false;
		for (int i = 0; i < NUM_CONFIGVARS; i++) {
			int idx =
				ini_find_property(ini, section, (char*)setting_defaults[i].name,
					strlen(setting_defaults[i].name));
			if (idx != INI_NOT_FOUND) {
				const char* variable_val = ini_property_value(ini, section, idx);
				setting_defaults[i].val = atoi(variable_val);
			}
			else {
				char snum[10];
				int num = setting_defaults[i].val;
				itoa(num, snum, 10);
				ini_property_add(ini, section, (char*)setting_defaults[i].name, strlen(setting_defaults[i].name), (char*)snum, strlen(snum));
				save = true;
			}
		}
		if (save) {
			int size = ini_save(ini, NULL, 0); // Find the size needed
			char* data = (char*)malloc(size);
			size = ini_save(ini, data, size); // Actually save the file
			fp = _wfopen(L"parasettings.ini", L"w");
			fwrite(data, 1, size, fp);
			fclose(fp);
			free(data);
		}
		ini_destroy(ini);
		fclose(fp);
	}
}

int WINAPI WinMain(HINSTANCE	hInstance,								// Instance
	HINSTANCE	hPrevInstance,							// Previous Instance
	LPSTR		lpCmdLine,								// Command Line Parameters
	int			nCmdShow)								// Window Show State

{

	HANDLE hMutex = OpenMutex(
		MUTEX_ALL_ACCESS, 0, "parallel-pj64.0");
	if (!hMutex)
		hMutex = CreateMutex(0, 0, "parallel-pj64.0");
	else
		return 0;


	hInstance = hInstance;

	init_coresettings();

	if (!OpenSetupDialog())
	{
		ReleaseMutex(hMutex);
		return -1;
	}

	ReleaseMutex(hMutex);

	return 0;
}

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

// Get the horizontal and vertical screen sizes in pixel
void GetDesktopResolution(int& horizontal, int& vertical)
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
		SetWindowText(hWnd, (LPCSTR)"Parallel-RDP config");

		if (setting_defaults[KEY_SCREEN_WIDTH].val == 0 || setting_defaults[KEY_SCREEN_HEIGHT].val == 0)
		{
			GetDesktopResolution(setting_defaults[KEY_SCREEN_WIDTH].val, setting_defaults[KEY_SCREEN_HEIGHT].val);
		}

		char s[500];
		int i = 0;

		DEVMODE d;
		EnumDisplaySettings(NULL, 0, &d);
		while (1)
		{
			BOOL h = EnumDisplaySettings(NULL, i++, &d);
			if (!h || nRes > MAXNUMRES)
				break;




			if ((setting_defaults[KEY_SCREEN_WIDTH].val * 9) / 16 == setting_defaults[KEY_SCREEN_HEIGHT].val)
			{
				selectedAspect = "16:9";
				if ((d.dmPelsWidth * 9) / 16 != d.dmPelsHeight)
					continue;
			}

			else if ((setting_defaults[KEY_SCREEN_WIDTH].val * 10) / 16 == setting_defaults[KEY_SCREEN_HEIGHT].val)
			{
				selectedAspect = "16:10";
				if ((d.dmPelsWidth * 10) / 16 != d.dmPelsHeight)
					continue;
			}

			else if ((setting_defaults[KEY_SCREEN_WIDTH].val * 3) / 4 == setting_defaults[KEY_SCREEN_HEIGHT].val)
			{
				selectedAspect = "4:3";
				if ((d.dmPelsWidth * 3) / 4 != d.dmPelsHeight)
					continue;
			}

			else
			{
				selectedAspect = "Others";
				if (!((d.dmPelsWidth * 9) / 16 != d.dmPelsHeight && (d.dmPelsWidth * 10) / 16 != d.dmPelsHeight && (d.dmPelsWidth * 3) / 4 != d.dmPelsHeight))
					continue;
			}

			if (d.dmBitsPerPel != 32)
				continue;

			if (!nRes || ress[nRes - 1].w != d.dmPelsWidth || ress[nRes - 1].h != d.dmPelsHeight)
			{
				ress[nRes].w = d.dmPelsWidth;
				ress[nRes].h = d.dmPelsHeight;

				nRes++;
				_snprintf_s(s, 500, "%d X %d", d.dmPelsWidth, d.dmPelsHeight);
				SendDlgItemMessage(hWnd, ComboResolution, CB_ADDSTRING, 0, (LPARAM)s);
			}
		}

		SendDlgItemMessage(hWnd, ComboAspect, CB_ADDSTRING, 0, (LPARAM)"16:9");
		SendDlgItemMessage(hWnd, ComboAspect, CB_ADDSTRING, 0, (LPARAM)"16:10");
		SendDlgItemMessage(hWnd, ComboAspect, CB_ADDSTRING, 0, (LPARAM)"4:3");
		SendDlgItemMessage(hWnd, ComboAspect, CB_ADDSTRING, 0, (LPARAM)"Others");



		for (i = 0; i < nRes; i++)
			if (ress[i].w == setting_defaults[KEY_SCREEN_WIDTH].val && ress[i].h == setting_defaults[KEY_SCREEN_HEIGHT].val)
				SendDlgItemMessage(hWnd, ComboResolution, CB_SETCURSEL, i, 0);
		currentIndexResolution = SendDlgItemMessage(hWnd, ComboResolution, CB_GETCURSEL, 0, 0);

		if (setting_defaults[KEY_FULLSCREEN].val)
			SendDlgItemMessage(hWnd, CheckFullscreen, BM_SETCHECK, 1, 1);

		///////////////////////// get the aspect ratio of current set resolution and set the selection /////////////////////////////
		int	AspectCount = SendDlgItemMessage(hWnd, ComboAspect, CB_GETCOUNT, 0, 0);

		for (int itemIndex = 0; itemIndex < AspectCount; itemIndex++)
		{
			SendDlgItemMessage(hWnd, ComboAspect, CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)listItemAspect.c_str());

			if (!strcmp(listItemAspect.c_str(), selectedAspect.c_str()))
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

	SendDlgItemMessage(hWnd, ComboUpscaler2, CB_SETCURSEL, setting_defaults[KEY_DOWNSCALING].val, 0);


	switch (setting_defaults[KEY_UPSCALING].val)
	{
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

	SendDlgItemMessage(hWnd, ComboDeinterlace, CB_SETCURSEL, setting_defaults[KEY_DEINTERLACE].val, 0);

	SendDlgItemMessage(hWnd, SSREADS, BM_SETCHECK, setting_defaults[KEY_SSREADBACKS].val, 0);
	SendDlgItemMessage(hWnd, CheckFullscreen, BM_SETCHECK, setting_defaults[KEY_FULLSCREEN].val, 0);
	SendDlgItemMessage(hWnd, SSDITHER, BM_SETCHECK, setting_defaults[KEY_SSDITHER].val, 0);
	SendDlgItemMessage(hWnd, INTEGERSCALE, BM_SETCHECK, setting_defaults[KEY_INTEGER].val, 0);
	SendDlgItemMessage(hWnd, VIBilinear, BM_SETCHECK, setting_defaults[KEY_VIBILERP].val, 0);
	SendDlgItemMessage(hWnd, VIDIVOT, BM_SETCHECK, setting_defaults[KEY_DIVOT].val, 0);
	SendDlgItemMessage(hWnd, VIAA, BM_SETCHECK, setting_defaults[KEY_AA].val, 0);
	SendDlgItemMessage(hWnd, VDEDITHER, BM_SETCHECK, setting_defaults[KEY_VIDITHER].val, 0);
	SendDlgItemMessage(hWnd, NATIVETEXLOD, BM_SETCHECK, setting_defaults[KEY_NATIVETEXTLOD].val, 0);
	SendDlgItemMessage(hWnd, NATIVETEXRECT, BM_SETCHECK, setting_defaults[KEY_NATIVETEXTRECT].val, 0);
	SendDlgItemMessage(hWnd, CheckVerticalSync, BM_SETCHECK, setting_defaults[KEY_VSYNC].val, 0);

	break;


	case WM_COMMAND:

		if (HIWORD(wParam) == CBN_SELCHANGE)
		{
			int itemIndexAspect = SendDlgItemMessage(hWnd, ComboAspect, CB_GETCURSEL, 0, 0);
			SendDlgItemMessage(hWnd, ComboAspect, CB_GETLBTEXT, (WPARAM)itemIndexAspect, (LPARAM)listItemAspect.c_str());

			int itemIndexResolution = SendDlgItemMessage(hWnd, ComboResolution, CB_GETCURSEL, 0, 0);

			if (!strcmp(listItemAspect.c_str(), "16:10"))
			{
				if (itemIndexResolution != currentIndexResolution)		// If we have a selection change in resolution
				{
					currentIndexResolution = SendDlgItemMessage(hWnd, ComboResolution, CB_GETCURSEL, 0, 0);
				}

				else if (strcmp(currentIndexAspect.c_str(), "16:10"))
				{
					currentIndexAspect = listItemAspect.c_str();

					SendDlgItemMessage(hWnd, ComboResolution, CB_RESETCONTENT, 0, 0);
					memset(&ress, 0, sizeof(ress));	//	Clear the width and height data in struct
					nRes = 0;
					char s[500];
					int i = 0;

					DEVMODE d;
					EnumDisplaySettings(NULL, 0, &d);
					while (1)
					{
						BOOL h = EnumDisplaySettings(NULL, i++, &d);
						if (!h || nRes > MAXNUMRES)
							break;

						if ((d.dmPelsWidth * 10) / 16 != d.dmPelsHeight)
							continue;

						if (d.dmBitsPerPel != 32)
							continue;

						if (!nRes || ress[nRes - 1].w != d.dmPelsWidth || ress[nRes - 1].h != d.dmPelsHeight)
						{
							ress[nRes].w = d.dmPelsWidth;
							ress[nRes].h = d.dmPelsHeight;

							nRes++;
							_snprintf_s(s, 500, "%d X %d", d.dmPelsWidth, d.dmPelsHeight);
							SendDlgItemMessage(hWnd, ComboResolution, CB_ADDSTRING, 0, (LPARAM)s);
						}
					}

					SendDlgItemMessage(hWnd, ComboResolution, CB_SETCURSEL, (nRes - 1), 0);
					currentIndexResolution = SendDlgItemMessage(hWnd, ComboResolution, CB_GETCURSEL, 0, 0);
				}
			}

			if (!strcmp(listItemAspect.c_str(), "16:9"))
			{
				if (itemIndexResolution != currentIndexResolution)		// If we have a selection change in resolution
				{
					currentIndexResolution = SendDlgItemMessage(hWnd, ComboResolution, CB_GETCURSEL, 0, 0);
				}

				else if (strcmp(currentIndexAspect.c_str(), "16:9"))
				{
					currentIndexAspect = listItemAspect.c_str();

					SendDlgItemMessage(hWnd, ComboResolution, CB_RESETCONTENT, 0, 0);
					memset(&ress, 0, sizeof(ress));	//	Clear the width and height data in struct
					nRes = 0;
					char s[500];
					int i = 0;

					DEVMODE d;
					EnumDisplaySettings(NULL, 0, &d);
					while (1)
					{
						BOOL h = EnumDisplaySettings(NULL, i++, &d);
						if (!h || nRes > MAXNUMRES)
							break;

						if ((d.dmPelsWidth * 9) / 16 != d.dmPelsHeight)
							continue;

						if (d.dmBitsPerPel != 32)
							continue;

						if (!nRes || ress[nRes - 1].w != d.dmPelsWidth || ress[nRes - 1].h != d.dmPelsHeight)
						{
							ress[nRes].w = d.dmPelsWidth;
							ress[nRes].h = d.dmPelsHeight;

							nRes++;
							_snprintf_s(s, 500, "%d X %d", d.dmPelsWidth, d.dmPelsHeight);
							SendDlgItemMessage(hWnd, ComboResolution, CB_ADDSTRING, 0, (LPARAM)s);
						}
					}

					SendDlgItemMessage(hWnd, ComboResolution, CB_SETCURSEL, (nRes - 1), 0);
					currentIndexResolution = SendDlgItemMessage(hWnd, ComboResolution, CB_GETCURSEL, 0, 0);
				}
			}
			if (!strcmp(listItemAspect.c_str(), "4:3"))
			{
				if (itemIndexResolution != currentIndexResolution)		// If we have a selection change in resolution
				{
					currentIndexResolution = SendDlgItemMessage(hWnd, ComboResolution, CB_GETCURSEL, 0, 0);
				}

				else if (strcmp(currentIndexAspect.c_str(), "4:3"))
				{
					currentIndexAspect = listItemAspect.c_str();

					SendDlgItemMessage(hWnd, ComboResolution, CB_RESETCONTENT, 0, 0);
					memset(&ress, 0, sizeof(ress));	//	Clear the width and height data in struct
					nRes = 0;
					char s[500];
					int i = 0;

					DEVMODE d;
					EnumDisplaySettings(NULL, 0, &d);
					while (1)
					{
						BOOL h = EnumDisplaySettings(NULL, i++, &d);
						if (!h || nRes > MAXNUMRES)
							break;

						if ((d.dmPelsWidth * 3) / 4 != d.dmPelsHeight)
							continue;

						if (d.dmBitsPerPel != 32)
							continue;

						if (!nRes || ress[nRes - 1].w != d.dmPelsWidth || ress[nRes - 1].h != d.dmPelsHeight)
						{
							ress[nRes].w = d.dmPelsWidth;
							ress[nRes].h = d.dmPelsHeight;

							nRes++;
							_snprintf_s(s, 500, "%d X %d", d.dmPelsWidth, d.dmPelsHeight);
							SendDlgItemMessage(hWnd, ComboResolution, CB_ADDSTRING, 0, (LPARAM)s);
						}
					}

					SendDlgItemMessage(hWnd, ComboResolution, CB_SETCURSEL, (nRes - 1), 0);
					currentIndexResolution = SendDlgItemMessage(hWnd, ComboResolution, CB_GETCURSEL, 0, 0);
				}
			}

			if (!strcmp(listItemAspect.c_str(), "Others"))
			{
				if (itemIndexResolution != currentIndexResolution)		// If we have a selection change in resolution
				{
					currentIndexResolution = SendDlgItemMessage(hWnd, ComboResolution, CB_GETCURSEL, 0, 0);
				}

				else if (strcmp(currentIndexAspect.c_str(), "Others"))
				{
					currentIndexAspect = listItemAspect.c_str();

					SendDlgItemMessage(hWnd, ComboResolution, CB_RESETCONTENT, 0, 0);
					memset(&ress, 0, sizeof(ress));	//	Clear the width and height data in struct
					nRes = 0;
					char s[500];
					int i = 0;

					DEVMODE d;
					EnumDisplaySettings(NULL, 0, &d);
					while (1)
					{
						BOOL h = EnumDisplaySettings(NULL, i++, &d);
						if (!h || nRes > MAXNUMRES)
							break;

						if (!((d.dmPelsWidth * 9) / 16 != d.dmPelsHeight && (d.dmPelsWidth * 10) / 16 != d.dmPelsHeight && (d.dmPelsWidth * 3) / 4 != d.dmPelsHeight))
							continue;

						if (d.dmBitsPerPel != 32)
							continue;

						if (!nRes || ress[nRes - 1].w != d.dmPelsWidth || ress[nRes - 1].h != d.dmPelsHeight)
						{
							ress[nRes].w = d.dmPelsWidth;
							ress[nRes].h = d.dmPelsHeight;

							nRes++;
							_snprintf_s(s, 500, "%d X %d", d.dmPelsWidth, d.dmPelsHeight);
							SendDlgItemMessage(hWnd, ComboResolution, CB_ADDSTRING, 0, (LPARAM)s);
						}
					}

					SendDlgItemMessage(hWnd, ComboResolution, CB_SETCURSEL, (nRes - 1), 0);
					currentIndexResolution = SendDlgItemMessage(hWnd, ComboResolution, CB_GETCURSEL, 0, 0);
				}
			}
		}
		switch (LOWORD(wParam))
		{
		case ButtonRun:
		{
			int lut[4] = { 1,2,4,8 };

			int ups = SendDlgItemMessage(hWnd, ComboUpscaler, CB_GETCURSEL, 0, 0);
			setting_defaults[KEY_UPSCALING].val = lut[ups];
	

			setting_defaults[KEY_DEINTERLACE].val = SendDlgItemMessage(hWnd, ComboDeinterlace, CB_GETCURSEL, 0, 0);
			setting_defaults[KEY_SSREADBACKS].val = SendDlgItemMessage(hWnd, SSREADS, BM_GETCHECK, 0, 0);
			setting_defaults[KEY_FULLSCREEN].val = SendDlgItemMessage(hWnd, CheckFullscreen, BM_GETCHECK, 0, 0);
			setting_defaults[KEY_SSDITHER].val = SendDlgItemMessage(hWnd, SSDITHER, BM_GETCHECK, 0, 0);
			setting_defaults[KEY_INTEGER].val = SendDlgItemMessage(hWnd, INTEGERSCALE, BM_GETCHECK, 0, 0);
			setting_defaults[KEY_VIBILERP].val = SendDlgItemMessage(hWnd, VIBilinear, BM_GETCHECK, 0, 0);
			setting_defaults[KEY_DIVOT].val = SendDlgItemMessage(hWnd, VIDIVOT, BM_GETCHECK, 0, 0);
			setting_defaults[KEY_AA].val = SendDlgItemMessage(hWnd, VIAA, BM_GETCHECK, 0, 0);
			setting_defaults[KEY_VIDITHER].val = SendDlgItemMessage(hWnd, VDEDITHER, BM_GETCHECK, 0, 0);
			setting_defaults[KEY_NATIVETEXTLOD].val = SendDlgItemMessage(hWnd, NATIVETEXLOD, BM_GETCHECK, 0, 0);
			setting_defaults[KEY_NATIVETEXTRECT].val = SendDlgItemMessage(hWnd, NATIVETEXRECT, BM_GETCHECK, 0, 0);
			setting_defaults[KEY_SCREEN_WIDTH].val = ress[SendDlgItemMessage(hWnd, ComboResolution, CB_GETCURSEL, 0, 0)].w;
			setting_defaults[KEY_SCREEN_HEIGHT].val = ress[SendDlgItemMessage(hWnd, ComboResolution, CB_GETCURSEL, 0, 0)].h;
			setting_defaults[KEY_FULLSCREEN].val = SendDlgItemMessage(hWnd, CheckFullscreen, BM_GETCHECK, 0, 0);
			setting_defaults[KEY_VSYNC].val = SendDlgItemMessage(hWnd, CheckVerticalSync, BM_GETCHECK, 0, 0);
			setting_defaults[KEY_DOWNSCALING].val = SendDlgItemMessage(hWnd, ComboUpscaler2, CB_GETCURSEL, 0, 0);
			save_coresettings();
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

int OpenSetupDialog()
{
	InitCommonControls();		// Used for visual styles
	return DialogBox(hInstance, MAKEINTRESOURCE(FormKickstart), GetForegroundWindow(), DlgFunc);
}