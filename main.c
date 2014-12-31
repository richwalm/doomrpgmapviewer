/*
	Doom RPG Map Viewer
	Written by Richard Walmsley. <richwalm@gmail.com>
	Thanks to Simon "Fraggle" Howard for the specs. (http://www.soulsphere.org/random/doom-rpg-bnf.txt)

	Copyright (C) 2012 Richard Walmsley <richwalm@gmail.com>

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#define	_WIN32_WINNT	0x0500
#include <windows.h>
#include <Windowsx.h>
#include <stdio.h>

#include "main.h"
#include "resource.h"
#include "map.h"
#include "draw.h"

static HWND hWnd;
static HDC hDC;
static HGLRC hRC;
static HACCEL hAccelerators;
static HMENU hMenu;

static BOOL MapLoaded = FALSE;
Map GameMap;
State PState;

static INT Redraw()
{
	if (!Draw(PState.ShowGrid & 1, PState.ShowNode & 1, PState.ShowVertex & 1, PState.ShowThings & 1, PState.ShowBlockmap & 1))
		return 0;
	SwapBuffers(hDC);

	return 1;
}

/*
static INT HandleClick(INT X, INT Y, BOOL Loop)
{
	RECT WindowSize;
	UINT MapX, MapY;
	INT Return;

	if (!GetClientRect(hWnd, &WindowSize))
		return 0;

	MapX = X * MAP_SIZE / WindowSize.right;
	MapY = Y * MAP_SIZE / WindowSize.bottom;

	if (Loop)
		PState.Node = 0;

	while (TRUE) {

		Return = GetNode(&GameMap, PState.Node, MapX, MapY);
		if (Return < 0)
			break;
		PState.Node = Return;

		if (!Loop)
			break;
	}

	Redraw();
	return 1;
}
*/

static VOID SetTitle(CONST TCHAR *Filename)
{
	TCHAR WindowTitle[BUFSIZ];
	INT Return;

	Return =
	#ifdef UNICODE
	_snwprintf
	#else
	_snprintf
	#endif
	(WindowTitle, BUFSIZ - 1, TEXT("%s - %s"), TEXT(PROGRAM_TITLE), Filename);

	if (Return < 0)
		WindowTitle[BUFSIZ - 1] = '\0';

	SetWindowText(hWnd, WindowTitle);

	return;
}

static INT ResizeGL()
{
	RECT WindowSize;

	if (!GetClientRect(hWnd, &WindowSize))
		return 0;

	GLSetup(WindowSize.right, WindowSize.bottom);

	return 1;
}

static INT LoadDialog()
{
	OPENFILENAME OpenFilename;
	TCHAR Path[MAX_PATH];
	Path[0] = '\0';
	#ifdef UNICODE
	CHAR UTF8Path[MAX_PATH];
	#endif

	ZeroMemory(&OpenFilename, sizeof(OpenFilename));
	OpenFilename.lStructSize = sizeof(OpenFilename);
	OpenFilename.hwndOwner = hWnd;
	OpenFilename.lpstrFilter = TEXT("Doom RPG Map Files (*.bsp)\0*.bsp\0");
	OpenFilename.lpstrFile = Path;
	OpenFilename.nMaxFile = MAX_PATH;
	OpenFilename.lpstrFileTitle = PState.Filename;
	OpenFilename.nMaxFileTitle = MAX_PATH;
	OpenFilename.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
	OpenFilename.lpstrDefExt = TEXT("bsp");

	if (!GetOpenFileName(&OpenFilename)) {
		if (CommDlgExtendedError() != 0)
			return 0;
		return 2;
	}

	#ifdef UNICODE
	if (!WideCharToMultiByte(CP_UTF8, 0, OpenFilename.lpstrFile, -1, UTF8Path, sizeof(UTF8Path), NULL, NULL))
		return 0;
	#endif

	if (MapLoaded) {
		FreeMap(&GameMap);
		MapLoaded = FALSE;
	}

	if (!LoadMap(
		#ifdef UNICODE
		UTF8Path,
		#else
		OpenFilename.lpstrFile,
		#endif
		&GameMap))
		return 0;

	MapLoaded = TRUE;
	PState.Node = 0;

	SetTitle(PState.Filename);

	return 1;
}

static INT UpdateMenu(UINT ID, BOOL Checked)
{
	MENUITEMINFO MenuItemInfo;

	ZeroMemory(&MenuItemInfo, sizeof(MenuItemInfo));
	MenuItemInfo.cbSize = sizeof(MenuItemInfo);
	MenuItemInfo.fMask = MIIM_STATE;
	MenuItemInfo.fState = (Checked ? MFS_CHECKED : MFS_UNCHECKED);

	if (!SetMenuItemInfo(hMenu, ID, FALSE, &MenuItemInfo))
		return 0;

	return 1;
}

static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MINMAXINFO *MinMaxInfo;

	switch (uMsg) {

        case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case ID_LOAD:
					if (!LoadDialog()) {
						MessageBox(hWnd, TEXT("Unable to load the map file.\n"), NULL, MB_ICONERROR);
						PostQuitMessage(0);
					}
					else
						Redraw();
					break;
				case ID_QUIT:
					PostQuitMessage(0);
					break;
				case ID_VIEW_NODE:
					PState.ShowNode++;
					UpdateMenu(LOWORD(wParam), PState.ShowNode & 1);
					Redraw();
					break;
				case ID_VIEW_BLOCKMAP:
					PState.ShowBlockmap++;
					UpdateMenu(LOWORD(wParam), PState.ShowBlockmap & 1);
					Redraw();
					break;
				case ID_VIEW_THINGS:
					PState.ShowThings++;
					UpdateMenu(LOWORD(wParam), PState.ShowThings & 1);
					Redraw();
					break;
				case ID_VIEW_GRID:
					PState.ShowGrid++;
					UpdateMenu(LOWORD(wParam), PState.ShowGrid & 1);
					Redraw();
					break;
				case ID_VIEW_VERTICES:
					PState.ShowVertex++;
					UpdateMenu(LOWORD(wParam), PState.ShowVertex & 1);
					Redraw();
					break;

				case ID_ABOUT:
					MessageBox(hWnd, TEXT(
						"Doom RPG Map Viewer v0.8\r\n\r\n" \
						"Written by Richard Walmsley <richwalm@gmail.com>\r\n\r\n" \
						"Thanks to Simon \"Fraggle\" Howard for the specs.\r\n" \
						"(http://www.soulsphere.org/random/doom-rpg-bnf.txt)"),
						TEXT("About"), 0);
					break;
			}
/*
		case WM_LBUTTONDOWN:
			HandleClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), FALSE);
			break;

		case WM_RBUTTONDOWN:
			HandleClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), TRUE);
			break;
*/
		case WM_SIZE:
			ResizeGL();
			Redraw();
			break;

		case WM_MOVE:
			Redraw();
			break;

		case WM_GETMINMAXINFO:
			MinMaxInfo = (MINMAXINFO *)lParam;
			MinMaxInfo->ptMinTrackSize.x = MinMaxInfo->ptMinTrackSize.y = MIN_WINDOW_SIZE;
			break;

		case WM_CLOSE:
			PostQuitMessage(0);
			break;

		case WM_DESTROY:
			return 0;

		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

static INT EnableGL(HWND hWnd, HDC* hDC, HGLRC* hRC)
{
	PIXELFORMATDESCRIPTOR PFD;
	INT Format;

	*hDC = GetDC(hWnd);
	if (!*hDC)
		return 0;

	ZeroMemory(&PFD, sizeof(PFD));
	PFD.nSize = sizeof(PFD);
	PFD.nVersion = 1;
	PFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	PFD.iPixelType = PFD_TYPE_RGBA;
	PFD.cColorBits = 24;
	PFD.cDepthBits = 16;
	PFD.iLayerType = PFD_MAIN_PLANE;

	Format = ChoosePixelFormat(*hDC, &PFD);
	if (!Format)
		return 0;
	if (!SetPixelFormat(*hDC, Format, &PFD))
		return 0;

	*hRC = wglCreateContext(*hDC);
	if (!*hRC)
		return 0;
	if (!wglMakeCurrent(*hDC, *hRC))
		return 0;

	return 1;
}

static VOID DisableGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hWnd, hDC);

    return;
}

static INT MakeWindow(HINSTANCE hInstance, int nCmdShow)
{
	CONST TCHAR *ClassName = TEXT("DoomRPGMapViewer");
	WNDCLASSEX WinClass;
	RECT WindowSize;

	WinClass.cbSize = sizeof(WinClass);
	WinClass.style = CS_OWNDC;
	WinClass.lpfnWndProc = WindowProc;
	WinClass.cbClsExtra = 0;
	WinClass.cbWndExtra = 0;
	WinClass.hInstance = hInstance;
	WinClass.hIcon = LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
	WinClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WinClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	WinClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
	WinClass.lpszClassName = ClassName;
	WinClass.hIconSm = NULL;

	if (!RegisterClassEx(&WinClass))
		return 0;

	WindowSize.left = WindowSize.top = 0;
	WindowSize.right = WindowSize.bottom = WINDOW_SIZE;
	if (!AdjustWindowRectEx(&WindowSize, WS_OVERLAPPEDWINDOW, TRUE, 0))
		return 0;

	hWnd = CreateWindowEx(
		0,
		ClassName,
		TEXT(PROGRAM_TITLE),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		WindowSize.right - WindowSize.left,
		WindowSize.bottom - WindowSize.top,
		NULL,
		NULL,
		hInstance,
		NULL);
	if (!hWnd)
		return 0;

	hMenu = GetMenu(hWnd);

	return 1;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG Msg;

	if (!MakeWindow(hInstance, nCmdShow)) {
		MessageBox(NULL, TEXT("Couldn't create the window.\n"), NULL, MB_ICONERROR);
		return 0;
	}

	hAccelerators = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATORS));
	if (!hAccelerators) {
		MessageBox(NULL, TEXT("Couldn't load the accelerators.\n"), NULL, MB_ICONERROR);
		return 0;
	}

	if (!EnableGL(hWnd, &hDC, &hRC)) {
		MessageBox(hWnd, TEXT("Couldn't enable OpenGL.\n"), NULL, MB_ICONERROR);
		return 0;
	}

	switch (LoadDialog()) {
		case 0:
			MessageBox(hWnd, TEXT("Unable to load the map file.\n"), NULL, MB_ICONERROR);
		case 2:
			return 0;
	}

	ShowWindow(hWnd, nCmdShow);

	/* Defaults. If changing, adjust the menu resource as well. */
	PState.ShowGrid = TRUE;
	PState.ShowNode = TRUE;
	PState.ShowVertex = TRUE;
	PState.ShowThings = TRUE;
	PState.ShowBlockmap = TRUE;

	if (!ResizeGL()) {
		FreeMap(&GameMap);
		MessageBox(hWnd, TEXT("Unable to do the inital drawing.\n"), NULL, MB_ICONERROR);
		return 0;
	}
	Redraw();

	while (GetMessage(&Msg, NULL, 0, 0) > 0) {

		if (!TranslateAccelerator(hWnd, hAccelerators, &Msg)) {
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}

	}

	DisableGL(hWnd, hDC, hRC);
	DestroyWindow(hWnd);

	return Msg.wParam;
}
