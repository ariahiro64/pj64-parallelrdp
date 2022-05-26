#include <stdio.h>
#include "config.h"
#include "ini.h"

struct settingkey_t settings[NUM_CONFIGVARS] =
{
    {"KEY_FULLSCREEN", 0},
    {"KEY_UPSCALING", 0},
    {"KEY_SCREEN_WIDTH", 640},
    {"KEY_SCREEN_HEIGHT", 480},
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
    {"KEY_DOWNSCALE", 1},
    {"KEY_WIDESCREEN", 0},
    {"KEY_SYNCHRONOUS", 1}
};

void config_init()
{
	// initialize ini parser
	ini_init();

	FILE* file = fopen(ini_file, "r");
    if (file == NULL)
    { // file doesn't exist, create it
        config_save();
    }
    else
    { // file exists, read it
        config_load();
        fclose(file);
    }
}

void config_save()
{
	for (int i = 0; i < NUM_CONFIGVARS; i++) 
    {
        ini_set_value(settings[i].name, settings[i].val);
    }
}

void config_load()
{
	for (int i = 0; i < NUM_CONFIGVARS; i++)
    {
        int value = -1;
        bool ret = ini_get_value(settings[i].name, &value);
        if (ret)
        {
            settings[i].val = value;
        }
    }
}

