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
#include "ini.h"
#include "config_gui.h"
#include "config.h"

static bool warn_hle = false;

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
    sprintf(PluginInfo->Name, "ParaLLEl");

    PluginInfo->NormalMemory = TRUE;
    PluginInfo->MemoryBswaped = TRUE;
}

EXPORT BOOL CALL InitiateGFX(GFX_INFO Gfx_Info)
{
    // initialize config
    config_init();

    gfx = Gfx_Info;
    plugin_init();
    return TRUE;
}

EXPORT void CALL DllConfig(HWND hParent)
{
    config_gui_open(hParent);
    // reload settings
    config_load();
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
    window_fullscreen = settings[KEY_FULLSCREEN].val;
    window_width = settings[KEY_SCREEN_WIDTH].val;
    window_height = settings[KEY_SCREEN_HEIGHT].val;
    window_widescreen = settings[KEY_WIDESCREEN].val;
    window_vsync = settings[KEY_VSYNC].val;
    window_integerscale = settings[KEY_INTEGER].val;

    vk_rescaling = settings[KEY_UPSCALING].val;
    vk_ssreadbacks = settings[KEY_SSREADBACKS].val;
    vk_ssdither = settings[KEY_SSDITHER].val;

    vk_interlacing = settings[KEY_DEINTERLACE].val;
    vk_overscan = settings[KEY_OVERSCANCROP].val;
    vk_native_texture_lod = settings[KEY_NATIVETEXTLOD].val;
    vk_native_tex_rect = settings[KEY_NATIVETEXTRECT].val;
    vk_divot_filter = settings[KEY_DIVOT].val;
    vk_gamma_dither = settings[KEY_GAMMADITHER].val;
    vk_dither_filter = settings[KEY_VIDITHER].val;
    vk_interlacing = settings[KEY_DEINTERLACE].val;
    vk_vi_aa = settings[KEY_AA].val;
    vk_vi_scale = settings[KEY_VIBILERP].val;
    vk_downscaling_steps = settings[KEY_DOWNSCALING].val;
    vk_synchronous  = settings[KEY_SYNCHRONOUS].val;

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
    vk_rasterize();
}

EXPORT void CALL UpdateScreen(void)
{
    vk_rasterize();
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

EXPORT BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    // set hInstance for the config GUI
    config_gui_hInstance = hModule;
    return TRUE;
}
