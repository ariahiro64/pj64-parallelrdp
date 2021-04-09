#ifndef MAIN_H
#define MAIN_H

#include	<stdio.h>							// Standard Input/Output
#include	<stdlib.h>							// Standard General Utilities Library
#include	<tchar.h>							// For Using TCHAR Characters
#include	<string>							// Easy to use text with this

#define	WIN32_LEAN_AND_MEAN					// Cut-down version of windows.h
#include	<windows.h>							// Windows
#include	<Shellapi.h>
#include	"KickstartResource.h"				// Dialog window

#include <Commctrl.h>
#pragma comment (lib, "Comctl32.lib")

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>


using std::string;
using std::ifstream;
using std::istringstream;

//	Gives us new-style buttons and such (not just win95-style)
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

VOID CALLBACK NotificationCompletion(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);

HINSTANCE hInstance;


struct settingkey {
	char name[255];
	int val;
	
};

struct RES
{
	int w,h;
};

struct Data
{
	string file;
	string title;
};

int OpenSetupDialog();

class Load
{
public:
	Load(string filename);
	Data load;
};
#endif