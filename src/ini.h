#ifndef INI_H
#define INI_H

#include <stdbool.h>
#include <windows.h>

extern char ini_file[MAX_PATH];

extern void ini_init();
extern bool ini_set_value(const char* key, int value);
extern bool ini_get_value(const char* key, int* value);

#endif // INI_H
