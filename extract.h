/*
 * Copyright (©) 2020 J. Fonseka
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

#include "acro.h"
#include <commdlg.h>

#define MIN_ACRO_LENGTH 3

#if MIN_ACRO_LENGTH >= MAX_ACRO_LENGTH
    #error Defined Min/Max Acronym Length Violation
#endif // MIN_ACRO_LENGTH

typedef struct
{
    HWND      h_prog;
    wchar_t * file_cont;
} ThreadData;

static AcronymDB * initExtractDB( uint32_t entries );

static bool addExtrToFile( HWND h_prog, AcronymDB * filt_list );
static inline bool isPunct( const wchar_t c );
static inline bool minMatchCheckAcro( wchar_t * file_cont, uint32_t curr_posn );

static DWORD WINAPI processContents( LPVOID lpParam );
static wchar_t * loadFileContents( FILE * fptr, uint32_t file_size );

static void startParsing( HWND hwnd, FILE * fptr_e );
static void addUnique( AcronymDB * extr_list, wchar_t * acro, uint32_t * filt_count );
static void lookForDefns( HWND h_prog, AcronymDB * filt_list, wchar_t const * file_cont );
static inline void insertDefn( AcronymDB * filt_list, uint32_t entry_no, wchar_t * extr_defn );

static void getAcro( AcronymDB * extr_list, wchar_t * file_cont,
                                            uint32_t * posn,
                                            uint32_t * filt_count );

static inline void acroCheck( wchar_t * file_cont,
                              uint32_t * posn,
                              uint32_t * count_all );


static AcronymDB * getFilteredList( AcronymDB * extr_list, uint32_t filt_count );


