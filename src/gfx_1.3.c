/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-video-angrylionplus - plugin.c                            *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2014 Bobby Smiles                                       *
 *   Copyright (C) 2009 Richard Goedeken                                   *
 *   Copyright (C) 2002 Hacktarux                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define M64P_PLUGIN_PROTOTYPES 1

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "gfx_1.3.h"
#include "glguts.h"
#include "parallel_imp.h"
#define INI_IMPLEMENTATION
#define INI_STRNICMP(s1, s2, cnt) (strcmp(s1, s2))
#include "ini.h"

typedef struct settingkey {
	char name[255];
	int val;
	
};

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
#define NUM_CONFIGVARS

struct settingkey setting_defaults[NUM_CONFIGVARS]=
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
	{"KEY_DOWNSCALE", 0}
};

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



static bool warn_hle = false;
static bool plugin_initialized;

GFX_INFO gfx;
uint32_t rdram_size;


static bool is_valid_ptr(void *ptr, uint32_t bytes)
{
    SIZE_T dwSize;
    MEMORY_BASIC_INFORMATION meminfo;
    if (!ptr) {
        return false;
    }
    memset(&meminfo, 0x00, sizeof(meminfo));
    dwSize = VirtualQuery(ptr, &meminfo, sizeof(meminfo));
    if (!dwSize) {
        return false;
    }
    if (MEM_COMMIT != meminfo.State) {
        return false;
    }
    if (!(meminfo.Protect & (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY))) {
        return false;
    }
    if (bytes > meminfo.RegionSize) {
        return false;
    }
    if ((uint64_t)((char*)ptr - (char*)meminfo.BaseAddress) > (uint64_t)(meminfo.RegionSize - bytes)) {
        return false;
    }
    return true;
}

void plugin_init(void)
{
    rdram_size = 0x800000;
    // Zilmar's API doesn't provide a way to check the amount of RDRAM available.
    // It can only be 4 MiB or 8 MiB, so check if the last 16 bytes of the provided
    // buffer in the 8 MiB range are valid. If not, it must be 4 MiB.
    if (!is_valid_ptr(&gfx.RDRAM[0x7f0000], 16)) {
        rdram_size /= 2;
    }

}

void plugin_close(void)
{
}

EXPORT void CALL CaptureScreen(char* directory)
{

}

EXPORT void CALL GetDllInfo(PLUGIN_INFO* PluginInfo)
{
    PluginInfo->Version = 0x0103;
    PluginInfo->Type  = PLUGIN_TYPE_GFX;
    sprintf(PluginInfo->Name, "parallel");

    PluginInfo->NormalMemory = TRUE;
    PluginInfo->MemoryBswaped = TRUE;
}

EXPORT BOOL CALL InitiateGFX(GFX_INFO Gfx_Info)
{
    
    gfx = Gfx_Info;
    plugin_init();
    return TRUE;
}

EXPORT void CALL CloseDLL(void)
{
}

EXPORT void CALL MoveScreen(int xpos, int ypos)
{
}

EXPORT void CALL ProcessDList(void)
{
     if (!warn_hle) {
        msg_warning("Please disable 'Graphic HLE' in the plugin settings.");
        warn_hle = true;
    }
}

EXPORT void CALL ProcessRDPList(void)
{
    vk_process_commands();
}

EXPORT void CALL RomOpen(void)
{
    init_coresettings();
    window_fullscreen = setting_defaults[KEY_FULLSCREEN].val;
    window_width = setting_defaults[KEY_SCREEN_WIDTH].val;
    window_height = setting_defaults[KEY_SCREEN_HEIGHT].val;
    vk_rescaling = setting_defaults[KEY_UPSCALING].val;
    vk_ssreadbacks = setting_defaults[KEY_SSREADBACKS].val;
    window_integerscale = setting_defaults[KEY_INTEGER].val;
    vk_ssdither = setting_defaults[KEY_SSDITHER].val;
    vk_interlacing = setting_defaults[KEY_DEINTERLACE].val;
    vk_overscan =setting_defaults[KEY_OVERSCANCROP].val;
    vk_native_texture_lod =setting_defaults[KEY_NATIVETEXTLOD].val;
    vk_native_tex_rect =setting_defaults[KEY_NATIVETEXTRECT].val;
    vk_divot_filter=setting_defaults[KEY_DIVOT].val;
    vk_gamma_dither=setting_defaults[KEY_GAMMADITHER].val;
    vk_vi_aa=setting_defaults[KEY_AA].val;
    vk_vi_scale=setting_defaults[KEY_VIBILERP].val;
    vk_dither_filter=setting_defaults[KEY_VIDITHER].val;
    vk_interlacing=setting_defaults[KEY_DEINTERLACE].val;
    vk_interlacing=setting_defaults[KEY_DEINTERLACE].val;
	vk_downscaling_steps=setting_defaults[KEY_DOWNSCALING].val;
	window_vsync=setting_defaults[KEY_VSYNC].val;
    vk_init();
}


EXPORT void CALL DrawScreen(void)
{
}

EXPORT void CALL ReadScreen(void **dest, long *width, long *height)
{
}



EXPORT void CALL RomClosed(void)
{
    vk_destroy();
}

EXPORT void CALL ShowCFB(void)
{
  //  vk_rasterize();
}

EXPORT void CALL UpdateScreen(void)
{
   // vk_rasterize();
}

EXPORT void CALL ViStatusChanged(void)
{
}

EXPORT void CALL ViWidthChanged(void)
{
}

EXPORT void CALL ChangeWindow(void)
{
    screen_toggle_fullscreen();
}

EXPORT void CALL FBWrite(DWORD addr, DWORD size)
{
}

EXPORT void CALL FBRead(DWORD addr)
{
}

EXPORT void CALL FBGetFrameBufferInfo(void *pinfo)
{
}
