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

#ifndef ACRO_H_INCLUDED
#define ACRO_H_INCLUDED

#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#define WIN32_WINNT        0x0601
#define __MSVCRT_VERSION__ 0x0800
// Exact MSVCRT version unclear.
// Required for I/O constants defined in fcntl.h
// LINK 6

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>
#include <wchar.h>
#include <tchar.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <malloc.h>
#include <richedit.h>
#include "resources.h"

/************************ OTHER DEFS ******************************/
#define MAX_ACRO_LENGTH    15
#define MAX_DEFN_LENGTH    250
#define RAWINPUTMAX        MAX_DEFN_LENGTH + 5
#define RELOAD_DELAY       1000
#define MAX_ACR_SIZE       5242880  //( 5MiB )
#define BUF                MAX_ACRO_LENGTH + MAX_DEFN_LENGTH

//throwError defs
#define DIR_GEN_PERM  1
#define FIL_LOAD_SIZE 5
#define FIL_LOAD_DESC 6
#define FIL_LOAD_ENC  10
#define FIL_EXT_FORM  12
#define FIL_LOAD_MISS 40
#define FIL_GEN_ACC   51
#define FIL_DUP_CRT   52
#define FIL_EXT_WRT   67

//#define DEBUG            1
#define d_printf( fmt, ... ) \
    do { if ( DEBUG ) printf( fmt, __VA_ARGS__ ); } while( 0 )
#define d_wprintf( fmt, ... ) \
    do { if ( DEBUG ) wprintf( fmt, __VA_ARGS__ ); } while( 0 )

// O_TEXT files have <cr><lf> sequences translated to <lf> on read()'s,
//    and <lf> sequences translated to <cr><lf> on write()'s
//#define _O_TEXT         0x4000  /* file mode is text (translated)
//#define _O_BINARY       0x8000  /* file mode is binary (untranslated)
//#define _O_WTEXT        0x10000 /* file mode is UTF16 (translated)
//#define _O_U16TEXT      0x20000 /* file mode is UTF16 no BOM (translated)
//#define _O_U8TEXT       0x40000 /* file mode is UTF8  no BOM (translated)
/*******************************************************************/
//clock_t begin, end;

bool disable_commands;

/* CHANGES BASED ON DARK/LIGHT MODE */
unsigned char TRANS_GLOBAL;
COLORREF BKGD_GLOBAL;
COLORREF FRGD_ACROLOADED;
COLORREF FRGD_GENERAL;
COLORREF FRGD_OTH;
COLORREF FRGD_ERROR;
COLORREF FRGD_DEFNTRUE;
COLORREF FRGD_INFO;

/*************************** STRUCTS *********************************/

//acronym entry
typedef struct
{
	wchar_t * acronym;
	wchar_t * defn;      //a pointer to the defn
} Acronym;

//database of acronyms
typedef struct
{
	uint32_t entries;
	Acronym ** acronyms;  //array of pointers to acronym entries
} AcronymDB;

/********************************************************************/


/*************************** PROTOTYPES ******************************/
//called from prcsCmd
extern bool cmdResDup( HWND hwnd );
extern bool cmdDelAcro( HWND hwnd, AcronymDB * entry_list );
extern bool cmdAddAcro( HWND hwnd, AcronymDB * entry_list,
                                   wchar_t * addAcroString,
                                   wchar_t * addDefnString );

//called from main
extern uint8_t prcsCmd( HWND hwnd, wchar_t * retstring, CHARFORMAT2W cf );


extern bool chkRawInput( HWND hwnd, wchar_t * retstring );
extern bool isNotPunct( wchar_t const c );
extern bool lookUpAcro( HWND hwnd, AcronymDB * entry_list,
                                   wchar_t const * retstring,
                                   wchar_t const * addDefnString,
                                   uint8_t const caller, CHARFORMAT2W cf );


extern FILE * retFileHandleNETLOC( wchar_t ** lckfile_path, wchar_t * option );
extern FILE * retFileHandleACR   ( wchar_t * option,        wchar_t ** filename,
                                   uint8_t const caller );

extern FILE * tmp_wopen( void * buf, size_t len );
extern FILE * sort_listAcro( FILE * stream, wchar_t * filecont );


extern uint32_t countRows( wchar_t const * filecont );
extern uint32_t getFileSize( FILE * fptr, bool initial_load );

extern void prcsDel( HWND hwnd, wchar_t * del_resp );
extern void chkMalloc( void * chk_me, uint8_t const err_code );
extern void throwError( uint8_t const err_code, uint32_t const r );
extern void remPunct( wchar_t * delimbuff );
extern void colourMe( HWND hwnd, COLORREF rgbvalue );
extern void disabledCmdMsg( HANDLE hwnd );
extern void disabledCmdMsg2( void );
extern void reloadACR( HWND hwnd );

extern wchar_t * getInput( HWND hwnd );
extern wchar_t * getInputDefn( HWND hwnd );
extern wchar_t * loadACR( FILE * fptr, uint32_t const file_size );

extern uint32_t lastCharIndex( HWND hwnd, BOOL want_coord );
extern uint32_t firstCharIndex( HWND hwnd );
/**************************************************************************/

#endif // ACRO_H_INCLUDED
