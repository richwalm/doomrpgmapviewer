#ifndef MAIN_H
#define MAIN_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define PROGRAM_TITLE	"Doom RPG Map Viewer"
#define WINDOW_SIZE		600
#define MIN_WINDOW_SIZE		320

typedef struct State {
	BOOL ShowGrid;
	BOOL ShowNode;
	BOOL ShowVertex;
	BOOL ShowThings;
	BOOL ShowBlockmap;
	UINT Node;
	TCHAR Filename[MAX_PATH];
} State;

#endif
