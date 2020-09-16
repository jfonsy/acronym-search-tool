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

static bool deleteLastLine(void)
{
    FILE * fptr = NULL;
    wchar_t * filename = NULL;
    uint32_t file_size = 0;
    uint16_t trunc_bytes = 0;
    wint_t wc = 0;

    if( NULL == ( fptr = retFileHandleACR( L"r+", &filename, 1 ) ) )
        throwError( FIL_GEN_ACC, 0 );

    file_size = getFileSize( fptr, false );

    fseek( fptr, 0, SEEK_END );
    fseek( fptr, 0, SEEK_CUR );

    uint32_t end_pos = ftell( fptr ) - 2;
    uint32_t last_cr = 0;

    /* assuming a line is at most 500 chars,
       go back a 1000 bytes, and search for the
       last occurrence of a CR */
    fseek( fptr, -1000, SEEK_CUR );

    for( uint16_t m = 0; m < 1000; m++ )
    {
        wc = fgetwc( fptr );
        if( 0x0D == wc )
            last_cr = ftell( fptr ) - 2;
    }
    // some error
    if( 0 == last_cr ) return false;

    trunc_bytes = end_pos - last_cr;

    int fd = fileno( fptr );
    int truncate_success = _chsize( fd, file_size - trunc_bytes );

    fclose( fptr );

    if( 0 == truncate_success ) return true;

    return false;
}

extern bool cmdDelAcro( HWND hwnd, AcronymDB * entry_list )
{
    wchar_t buf[ BUF ];
    wchar_t * last_acro;
    wchar_t * last_defn;

    colourMe( hwnd, FRGD_OTH );
    if( 0 == entry_list->entries )
    {
        SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                      (LPARAM)L"The last entry cannot be deleted\n\n" );
        return false;
    }

    last_acro = entry_list->acronyms[ entry_list->entries ]->acronym;
    last_defn = entry_list->acronyms[ entry_list->entries ]->defn;

   _snwprintf( buf, BUF, L"Delete the entry \"%s # %s\" ?",
               last_acro, last_defn );

    int resp = 0;
    resp = MessageBoxW( hwnd, buf, L"Confirm deletion", MB_YESNO |
                                                        MB_ICONEXCLAMATION |
                                                        MB_DEFBUTTON2 );

    wmemset( buf, 0x0, BUF );
    _snwprintf( buf, BUF, L"\n%s # %s was deleted\n\n", last_acro, last_defn );

    if( IDYES == resp )
    {
        if( true == deleteLastLine())
        {
            colourMe( hwnd, FRGD_INFO );
            SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)buf );
            return true;
        }
        else
            SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                          (LPARAM)L"The delete operation could not be performed\n\n" );
    }
    else
        SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                      (LPARAM)L"The delete operation was cancelled\n\n" );

    return false;
}
