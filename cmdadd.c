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

#include "acro.h"
#include "cmdadd.h"

static void cmd_addToFile( HWND hwnd, wchar_t const * addAcroString,
                                      wchar_t const * addDefnString )
{
    wchar_t   buf[ BUF ];
    wchar_t * lckfile_path = NULL;
    int16_t   loc_res = 0, net_res = 0;
    FILE    * fptr;

    if( NULL == ( fptr = retFileHandleACR( L"a", NULL, 0 )))
    {
        throwError( FIL_GEN_ACC, 0 );
    }

    _setmode( _fileno( fptr ), _O_WTEXT );

    loc_res = fwprintf( fptr, L"\n%s # %s", addAcroString, addDefnString );
    fclose( fptr );

    // Pass a pointer to get lckfile_path so we can remove it after writing to the file
    if( NULL == ( fptr = retFileHandleNETLOC( &lckfile_path, L"a" ) ) )
    {
        if( NULL != lckfile_path )
        {
            _wremove( lckfile_path );
            free( lckfile_path ); lckfile_path = NULL;
        }
        net_res = -1;
    }
    else
    {
        _setmode( _fileno( fptr ), _O_WTEXT );
        net_res = fwprintf( fptr, L"\n%s # %s", addAcroString, addDefnString );

        fclose( fptr );
        _wremove( lckfile_path );
        free( lckfile_path ); lckfile_path = NULL;
    }

    if( ( loc_res < 0 ) &&
        ( net_res < 0 ) )
    {
        colourMe( hwnd, FRGD_ERROR );
        SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                (LPARAM)L"    Could not write to >>LOCAL<< file\n" );
        colourMe( hwnd, FRGD_ERROR );
        SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                (LPARAM)L"    Could not write to >>NETWORK<< file\n\n" );

    }
    else if( ( loc_res > 0 ) &&
             ( net_res < 0 ) )
    {
        colourMe( hwnd, FRGD_DEFNTRUE );
        _snwprintf( buf, BUF, L"\n%ls # %ls\n",
                    addAcroString, addDefnString );
        SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                (LPARAM)buf );

        colourMe( hwnd, FRGD_INFO );
        SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                (LPARAM)L"    Added to >>LOCAL<< file\n" );
        colourMe( hwnd, FRGD_ERROR );
        SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                (LPARAM)L"    Could not write to >>NETWORK<< file\n\n" );

    }
    else if( ( loc_res < 0 ) &&
             ( net_res > 0 ) )
    {
        colourMe( hwnd, FRGD_DEFNTRUE );
        _snwprintf( buf, BUF, L"\n%ls # %ls\n",
                    addAcroString, addDefnString );
        SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                (LPARAM)buf );

        colourMe( hwnd, FRGD_INFO );
        SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                (LPARAM)L"    Added to >>NETWORK<< file\n" );
        colourMe( hwnd, FRGD_ERROR );
        SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                (LPARAM)L"    Could not write to >>LOCAL<< file\n\n" );
        colourMe( hwnd, FRGD_ERROR );

    }
    else if( ( loc_res > 0 ) &&
             ( net_res > 0 ) )
    {
        colourMe( hwnd, FRGD_DEFNTRUE );
        _snwprintf( buf, BUF, L"\n%ls # %ls\n",
                    addAcroString, addDefnString );
        SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                (LPARAM)buf );

        colourMe( hwnd, FRGD_INFO );
        SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                (LPARAM)L"    Added to >>LOCAL<< file\n" );
        colourMe( hwnd, FRGD_INFO );
        SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                (LPARAM)L"    Added to >>NETWORK<< file\n\n" );

    }
}

extern bool cmdAddAcro( HWND hwnd, AcronymDB * entry_list,
                        wchar_t * addAcroString, wchar_t * addDefnString )
{
    wchar_t buf[ BUF ];
    CHARFORMAT2W cf = { 0 };
    cf.cbSize = sizeof( CHARFORMAT2W );
    cf.dwMask = CFM_COLOR;

#ifdef DEBUG
    wprintf( L"%s - %s", addAcroString, addDefnString );
#endif // DEBUG

    if ( true != lookUpAcro( hwnd, entry_list, addAcroString, addDefnString, 1, cf ) )
    {
        cmd_addToFile( hwnd, addAcroString, addDefnString );
    }
    else
    {
        colourMe( hwnd, FRGD_INFO );

        _snwprintf( buf, BUF, L"\n%s # %s already exists in the acronym list\n\n",
                    addAcroString, addDefnString );
        SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                (LPARAM)buf );

        free( addDefnString ); addDefnString = NULL;
        free( addAcroString ); addAcroString = NULL;

        return false;
    }

    free( addDefnString ); addDefnString = NULL;
    free( addAcroString ); addAcroString = NULL;

    return true;
}
