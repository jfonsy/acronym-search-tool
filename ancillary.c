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

static inline uint8_t digitCount( uint16_t acronyms_loaded )
{
    uint8_t digits;
    char    buf[ sizeof( int ) * 8 + 1 ];

    itoa( acronyms_loaded, buf, 10 );
    for( digits = 0; '\0' != buf[ digits ]; digits++ );

    return digits;
}

extern wchar_t * acrLoadedText( AcronymDB * entry_list )
{
    wchar_t acronyms_loaded[ 30 ]; //" acronyms" is 10 characters including \0
                                //max of 15 digit number for proper display

    swprintf( acronyms_loaded , L"%*d acronyms",
             20 - digitCount( entry_list->entries ) + 1,
             entry_list->entries + 1 );

    return( wcsdup( acronyms_loaded ) );
}

extern void colourMe( HWND hwnd, COLORREF rgbvalue )
{
    CHARFORMAT2W cf = { 0 };
    cf.cbSize = sizeof( CHARFORMAT2W );

    cf.dwMask = CFM_COLOR;
    cf.crTextColor = rgbvalue;
    SendMessageW( hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf );
}

extern uint32_t firstCharIndex( HWND hwnd )
{
    RECT r;
    POINT pt;

    GetClientRect( hwnd, &r );

    pt.x = 0;
    pt.y = 0;

    uint32_t n = SendMessageW( hwnd, EM_CHARFROMPOS, 0, (LPARAM)&pt );
    uint32_t c_first = SendMessageW( hwnd, EM_POSFROMCHAR, n, 0 );

    return c_first;
}

extern uint32_t lastCharIndex( HWND hwnd, BOOL want_coord )
{
    RECT r;
    POINT pt;

    GetClientRect( hwnd, &r );

    pt.x = ( r.right - r.left ) -1;
    pt.y = ( r.bottom - r.top ) -1;

    //get index of last char within window
    //then retrieve coordinates of that char
    uint32_t n = SendMessageW( hwnd, EM_CHARFROMPOS, 0, (LPARAM)&pt );
    uint32_t c_last = SendMessageW( hwnd, EM_POSFROMCHAR, n, 0 );

    //last char co-ordinates
    if( true == want_coord ) return c_last;
    return n;
}

extern uint32_t countRows( wchar_t const * filecont )
{
    uint32_t rows = 0, i, f = wcslen( filecont );

    for ( i = 0; i < f; i++ )
    {
        //disregard blank lines at the top or somewhere in the middle
        if (( L'\n' == filecont[ i ] ) &&
                  ( i != 0 )           &&
            ( L'\n' != filecont[ i + 1 ] ))
        {
            rows++;
        }
    }
    //if there's a new line at the end, disregard
    if (( L'\0' == filecont[ i ] ) &&
        ( L'\n' == filecont[ i - 1 ] )) rows--;

    return ( rows );
}

extern inline void chkMalloc( void * chk_me, uint8_t const err_code )
{
    if ( !chk_me )
    {
        throwError( err_code, 0 );
    }
}

extern void remPunct( wchar_t * delimbuff )
{
    wchar_t * src, * dst;

    for ( src = dst = delimbuff; *src != L'\0'; src++ )
    {
        *dst = *src;
        if ( isNotPunct( * dst ) )
        {
            dst++;
        }
    }

    * dst = L'\0';
}

extern bool isNotPunct( const wchar_t c )
{
    if ( c != L'-'  &&
         c != L'('  &&
         c != L')'  &&
         c != L'&'  &&
         c != L'/'  &&
         c != L'\\' &&
         c != L' '  &&
         c != L'#' )
    {
        return true;
    }
    return false;
}

extern inline void disabledCmdMsg( HANDLE hwnd )
{
    SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                  (LPARAM)L"Another operation is being performed\n\n" );
}

extern inline void disabledCmdMsg2( void )
{
    MessageBoxW( NULL,
    L"Another operation is being performed",
    L"Acronym Search Tool",
    MB_OK | MB_ICONERROR );
}

/* Any functions calling this will result in a hard exit of the program */
extern void throwError( uint8_t const err_code, uint32_t const r )
{
    wchar_t buf[ BUF ];

    switch ( err_code )
    {
        //retFileHandle error
        case 1:

            MessageBoxW( NULL,
            L"Program directory path error - check access rights",
            L"Acronym Search Tool",
            MB_OK | MB_ICONERROR );

            break;
        //getFileSize errors
        case 5:

            MessageBoxW( NULL,
            L"The '.acr' file is greater than 5MiB (5.24MB), reduce the file size.",
            L"Acronym Search Tool",
            MB_OK | MB_ICONERROR );

            break;
        case 6:

            MessageBoxW( NULL,
            L"Unable to load the '.acr' file",
            L"Acronym Search Tool",
            MB_OK | MB_ICONERROR );

            break;
        //loadACR error
        case 10:

            MessageBoxW( NULL,
            L"The '.acr' file needs to be encoded in UTF-16/UCS-2 (LE) format.\n"
            L"The file maybe corrupted, or it is encoded differently.",
            L"Acronym Search Tool",
            MB_OK | MB_ICONERROR );

            break;

        case 12:

            MessageBoxW( NULL,
            L"Unable to extract from the file\n"
            L"Try saving it in UTF-16/UCS-2 (LE) format and try again.",
            L"Acronym Search Tool",
            MB_OK | MB_ICONERROR );

            break;
        //load errors
        case 35:
        case 36:
        case 37:
            _snwprintf( buf, BUF,
                 L"Potential error(s) found at lines %u to %u of acronym list\n"
                 L"Code: %u\n", r - 1, r + 1, err_code );
            MessageBoxW( NULL, buf,
            L"Acronym Search Tool",
            MB_OK | MB_ICONERROR );

            break;
        case 40:

            MessageBoxW( NULL,
            L"An acronyms '.acr' file was not found in the program folder.",
            L"Acronym Search Tool",
            MB_OK | MB_ICONERROR );

            break;
        case 51:

            MessageBoxW( NULL,
            L"File error - unable to update the >>LOCAL<< '.acr' file\r\n"
            L"Check that the file exists, and write access is allowed",
            L"Acronym Search Tool",
            MB_OK | MB_ICONERROR );

            break;
        case 52:

            MessageBoxW( NULL,
            L"File error - unable to create a new '.acr' file\r\n"
            L"Check that write access is allowed in your program directory",
            L"Acronym Search Tool",
            MB_OK | MB_ICONERROR );

            break;

        case 67:

            MessageBoxW( NULL,
            L"An unknown error occurred whilst writing extracted acronyms"
            L" to the local file. However, some entries may have been added.",
            L"Acronym Search Tool",
            MB_OK | MB_ICONERROR );

            break;
        //malloc errors
        default:
            snwprintf( buf, BUF, L"Memory Allocation Error - Code: %u\n", err_code );
            MessageBoxW( NULL, buf,
            L"Acronym Search Tool",
            MB_OK | MB_ICONERROR );
    }

    exit( EXIT_FAILURE );
}

/* Return the master acr file in a network folder by reading its location in a local text file */
extern FILE * retFileHandleNETLOC( wchar_t ** lckfile_path, wchar_t * option )
{
    _WDIR           * dptr;
    struct _wdirent * directory;
    FILE            * net_acr_loc, * net_acr;
    wchar_t           net_loc[ MAX_PATH ], c = 0;

    dptr = _wopendir( L"." ); //parent directory of exe
    if ( NULL != dptr )
    {
        while( NULL != ( directory = _wreaddir( dptr ) ))
        {
            //check whether the file name contains net_acr_loc
            if( wcsstr( directory->d_name, L"net_acr_loc" ))
            {
                //open the .txt file to read the network location
                net_acr_loc = _wfopen( directory->d_name , L"r" );

                //copy the address specified in the file into a temporary buffer
                uint16_t i = 0;
                while(( WEOF != (wint_t)c ) && ( i < MAX_PATH ))
                {
                    c = fgetwc( net_acr_loc );
                    net_loc[ i++ ] = c;
                }
                //remove the EOF char from the stream before terminating
                net_loc[ i - 1 ] = L'\0';
                fclose( net_acr_loc );
            }
        }

        _wclosedir( dptr );
    }

    //first check whether the lock file exists
    if( NULL != ( dptr = _wopendir( net_loc ) ) &&
           0 == wcscmp( option, L"a" ) )
    {
        while( NULL != ( directory = _wreaddir( dptr ) ))
        {
            if( wcsstr( directory->d_name, L".lckacr" ))
            {
                _wclosedir( dptr );
                return NULL;                 //it exists, return early
            }
        }
        _wclosedir( dptr );
    }
    //if the lock file doesn't exist, then...
    if( NULL != ( dptr = _wopendir( net_loc )))
    {
        while( NULL != ( directory = _wreaddir( dptr )))
        {
            if( wcsstr( directory->d_name, L".acr" ))
            {
                //buffer to hold path of lockfile to be created
                wchar_t inuse_path[ MAX_PATH ];
                if( MAX_PATH > wcslen( net_loc ) + 9 ) //9 chars for _.inuseckacr\0
                {
                    if( 0 == wcscmp( option, L"a" ))
                    {
                        wcscpy( inuse_path, net_loc );
                        inuse_path[ wcslen( net_loc ) + 1 ] = L'\0';

                        wcscat( inuse_path, L"_.lckacr" );
                        //create the lock file
                        FILE * in_use = _wfopen( inuse_path, L"w" );

                        fclose( in_use );
                        //freed by addToFile
                        *lckfile_path = wcsdup( inuse_path );
                    }

                    wcscat( net_loc, directory->d_name );
                    net_acr = _wfopen( net_loc , option );

                    _wclosedir( dptr );

                    return( net_acr );
                }
                else
                {
                    _wclosedir( dptr );
                    return NULL;
                }
            }
        }
    }
    _wclosedir( dptr );
    return NULL;
}

extern FILE * retFileHandleACR( wchar_t * option, wchar_t ** filename, uint8_t caller ) //errcodes 1 - 4
{
    _WDIR           * dptr;
    struct _wdirent * directory;
    FILE            * acr_file;
    wchar_t           buffer[ MAX_PATH ] = L"";
    wchar_t        ** lppPart = { NULL };
    // LINK 1


    dptr = _wopendir( L"." ); //local acr file
    if ( dptr == NULL ) throwError( DIR_GEN_PERM, 0 );

    while(( directory = _wreaddir( dptr )) != NULL )
    {
        // check whether the file name contains .acr
        // n.b: only the first found .acr occurrence will be used
        if( wcsstr( directory->d_name, L".acr" ))
        {
            acr_file = _wfopen( directory->d_name , option );
            if ( 1 == caller )
            {
                GetFullPathNameW( directory->d_name, MAX_PATH, buffer, lppPart );
                *filename = ( wchar_t * ) malloc(( wcslen( buffer ) + 1 ) * ( sizeof( wchar_t )));
                wmemcpy( *filename, buffer, wcslen( buffer ) + 1 );
            }

            if( NULL != dptr ) _wclosedir( dptr );
            return ( acr_file );
        }
    }

    return NULL;
}
