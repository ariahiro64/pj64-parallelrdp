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

#define KEY_FULLSCREEN "Fullscreen"
#define KEY_SCREEN_WIDTH "ScreenWidth"
#define KEY_SCREEN_HEIGHT "ScreenHeight"
#define KEY_UPSCALING "Upscaling"
#define KEY_SSDITHER "SuperscaledDither"
#define KEY_SSREADBACKS "SuperscaledReads"
#define KEY_OVERSCANCROP "CropOverscan"
#define KEY_DIVOT "Divot"
#define KEY_GAMMADITHER "GammaDither"
#define KEY_AA "VIAA"
#define KEY_VIBILERP "VIBilerp"
#define KEY_VIDITHER "VIDither"
#define KEY_DOWNSCALE "DownScale"
#define KEY_NATIVETEXTRECT "NativeTextRECT"
#define KEY_NATIVETEXTLOD "NativeTextLOD"
#define KEY_DEINTERLACE "Deinterlace"
#define KEY_INTEGER "IntegerScale"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "gfx_1.3.h"
#include "glguts.h"
#include "parallel_imp.h"



static bool warn_hle;
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

EXPORT void CALL GetDllInfo(PLUGIN_INFO* PluginInfo)
{
    PluginInfo->Version = 0x0103;
    PluginInfo->Type  = PLUGIN_TYPE_GFX;
    sprintf(PluginInfo->Name, "parallel");

    PluginInfo->NormalMemory = TRUE;
    PluginInfo->MemoryBswaped = TRUE;
}

EXPORT int CALL InitiateGFX(GFX_INFO Gfx_Info)
{
    gfx = Gfx_Info;

    return 1;
}

EXPORT void CALL MoveScreen(int xpos, int ypos)
{
}

EXPORT void CALL ProcessDList(void)
{
}

EXPORT void CALL ProcessRDPList(void)
{
    vk_process_commands();
}

EXPORT void CALL RomOpen(void)
{
    window_fullscreen = false;
    window_width = 1024;
    window_height = 768;
    vk_rescaling = 0;
    vk_ssreadbacks = 0;
    window_integerscale = 0;
    vk_ssdither = 0;


    plugin_init();
    vk_init();
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
