/*
 * Copyright (©) 2019 J. Fonseka
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PROCS_H_INCLUDED
#define PROCS_H_INCLUDED

#include <shellapi.h>
#include <commctrl.h>

#define PROG_VER L"Acronym Search v2.7"

#define MAX_SCREEN_BUF   64000
// max height/width of logo
#define LOGOHMAX         65
#define LOGOWMAX         250

#define IDM_COM_ADD      1
#define IDM_COM_DEL      2
#define IDM_COM_UPD      3
#define IDM_COM_RES      4
#define IDM_COM_EXTR     5
#define IDM_COM_EXIT     6
#define IDM_VW_LM        7
#define IDM_VW_DM        9
#define IDM_HLP_HLP      10

#define IDC_CONTADD      11
#define IDC_MULTILINEIO  100

#define WM_CMDRECV (WM_USER + 0x100)

// dimensions and start posn of main window
#define STARTPOS_HORZ    200
#define STARTPOS_VERT    100
#define WMAIN_HEIGHT     600
#define WMAIN_WIDTH      1000


WNDPROC OldMultiLineIO;
AcronymDB * entry_list;

LRESULT CALLBACK WndProc      ( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK MultiLineProc( HWND, UINT, WPARAM, LPARAM );

extern AcronymDB * loadToMemory( FILE * fptr );
extern void initialise();
extern bool loadFileToExtract( HWND hwnd );
extern wchar_t * acrLoadedText( AcronymDB * entry_list );
extern int8_t initUpdate( HWND hwnd );

#endif // PROCS_H_INCLUDED
