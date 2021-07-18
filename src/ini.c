#include "ini.h"
#include <windows.h>
#include <stdbool.h>
#include <unistd.h>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

char ini_file[MAX_PATH];

// extracts directory from full file path
static void file_to_dir(char* file_path, char* directory, size_t size)
{
	char dir_buf[MAX_PATH] = { 0 };

	char* str = strtok(file_path, "\\");
	char* str2 = NULL;
	while (str != NULL)
	{
		str2 = strtok(NULL, "\\");
		if (str2 == NULL)
		{ // no more '\' found
			break;
		}

		strcat(dir_buf, str);
		strcat(dir_buf, "\\");

		str = str2;
	}

	strncpy(directory, dir_buf, size);
}

void ini_init()
{
	char buf[MAX_PATH] = { 0 };
	GetModuleFileNameA((HINSTANCE)&__ImageBase, buf, MAX_PATH);

	file_to_dir(buf, ini_file, MAX_PATH);
	strcat(ini_file, "\\pj64-parallelrdp.ini");
}

bool ini_set_value(const char* key, int value)
{
	char num_str[10];
	itoa(value, num_str, 10);
    return WritePrivateProfileStringA("Settings", key, num_str, ini_file);
}

bool ini_get_value(const char* key, int* value)
{
    char buf[MAX_PATH];
    GetPrivateProfileStringA("Settings", key, NULL, buf, MAX_PATH, ini_file);
    if (buf == NULL)
    {
    	*value = 0;
        return false;
    }

    *value = atoi(buf);
    return true;
}

