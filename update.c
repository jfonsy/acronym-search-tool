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
#include "update.h"

static void upd_addToFile( HWND hwnd, FILE * fptr_loc, wchar_t * to_add )
{
    wchar_t * stringpos = to_add; // point to beginning position of string
    wchar_t widebuf[ BUF ];
    wint_t wc;
    uint16_t i = 0;

    // Write CRLF
    fputwc( L'\r', fptr_loc );
    fputwc( L'\n', fptr_loc );

    to_add = stringpos;
    while( L'\0' != ( wc = *( to_add++ )))
    {
        fputwc( wc, fptr_loc );
        widebuf[ i++ ] = wc;
    }

    widebuf[ i ] = L'\0';

    colourMe( hwnd, FRGD_INFO );
    SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
            (LPARAM)L"  >>   ");
    SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
            (LPARAM)widebuf );
    SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
            (LPARAM)L"\n");

    UpdateWindow( hwnd );
    fflush( fptr_loc );
    add_occurred = true;
}

static wchar_t * getLastEntry( FILE * stream, uint32_t file_size )
{
    wint_t   wc;
    uint16_t x = 0, bytes_rem;
    uint32_t streampos;
    wchar_t  entry[ BUF ];

    streampos = ftell( stream );
    bytes_rem = file_size - streampos;

   // Skip control characters whilst getting data
   do {

       wc = fgetwc( stream );
       if( !( wc >    0 && wc <=  31 ) &&
           !( wc >= 127 && wc <= 159 ))
       {
            entry[ x++ ] = wc;
       }
       bytes_rem -= 2;

   } while( bytes_rem > 0 );

    entry[ x ] = L'\0';

    return wcsdup( entry );
}

// read chars until carriage return
static wchar_t * getEntry( FILE * stream )
{
    wint_t   wc;
    uint16_t x = 0;
    wchar_t  entry[ BUF ];

    while( L'\n' != ( wc = fgetwc( stream )))
    {
        if( L'\r' != wc )
            entry[ x++ ] = wc;
    }
    entry[ x ] = L'\0';

    return wcsdup( entry );
}

// Compare entries in the local file against the network file
// if there is no match found, add the entry to the local file
// TODO: Look into a better method that avoids the excessive comparisons
// happening with this crude way of checking
static bool startCompare( HWND hwnd, FILE * stream_loc, FILE * stream_net,
                          uint32_t const stream_loc_entries,
                          uint32_t const stream_net_entries )
{
    FILE    * fptr_loc;
    wchar_t * filename = NULL;
    bool      match    = true;

    if( NULL == ( fptr_loc = retFileHandleACR( L"a", &filename, 0 )))
    {
        throwError( FIL_GEN_ACC, 0 );
    }

    _setmode( _fileno( fptr_loc ), _O_BINARY );

    wchar_t  * entry_net = NULL, * entry_loc = NULL;
    uint32_t line_net  = 0, line_loc = 0;
    uint32_t file_size_loc = getFileSize( stream_loc, false );
    uint32_t file_size_net = getFileSize( stream_net, false );

    // depending on whether it's the last line or not
    // entries will be obtained and compared slightly differently
    for( ; line_net <= stream_net_entries; line_net++ )
    {
        if( stream_net_entries == line_net )
        {
            entry_net = getLastEntry( stream_net, file_size_net );
        }
        else
        {
            entry_net = getEntry( stream_net );
        }

        for( ; line_loc <= stream_loc_entries; line_loc++ )
        {
            if( stream_loc_entries == line_loc )
            {
                entry_loc = getLastEntry( stream_loc, file_size_loc );
            }
            else
            {
                entry_loc = getEntry( stream_loc );
            }

            if( 0 == wcscmp( entry_net, entry_loc ) )
            {
                match = true;
                break;
            }
            else
            {
                match = false;
                free( entry_loc ); entry_loc = NULL;
            }
        }

        if( false == match )
        {
            //min len == 2 for acro, a space separated hash,
            //and len == 3 for desc to avoid garbage being written
            //crude check, but better than nothing
            if(     8 < wcslen( entry_net ) &&
                NULL != wcsstr( entry_net, L" # " ) )
            {
                upd_addToFile( hwnd, fptr_loc, entry_net );
                match = true;
            }
        }
        //go back to start of the file
        line_loc = 0;
        fseek( stream_loc, 0, SEEK_SET );
        fseek( stream_loc, 0, SEEK_CUR );

        free( entry_loc ); entry_loc = NULL;
        free( entry_net ); entry_net = NULL;
    }

    fclose( fptr_loc );
    fclose( stream_loc );
    fclose( stream_net );

    return true;
}

static bool beginTFOSC( HWND hwnd, FILE * fptr_loc, FILE * fptr_net,
                        wchar_t * filecont_loc, wchar_t * filecont_net )
{
    FILE * stream_loc = NULL, * stream_net = NULL;
    uint32_t stream_loc_entries, stream_net_entries;

    stream_loc_entries = countRows( filecont_loc );
    stream_net_entries = countRows( filecont_net );

    // TMP FILE OPEN AND SORT

    stream_loc = tmp_wopen( filecont_loc, wcslen( filecont_loc ));
    stream_net = tmp_wopen( filecont_net, wcslen( filecont_net ));
    stream_loc = sort_listAcro( stream_loc, filecont_loc );
    stream_net = sort_listAcro( stream_net, filecont_net );

    fclose( fptr_net ); //we won't be writing to the network file
    fclose( fptr_loc ); //we will write to the local file later if needed

    free( filecont_loc ); filecont_loc = NULL;
    free( filecont_net ); filecont_net = NULL;

    if( NULL == stream_loc ||
        NULL == stream_net )
    {
        if( NULL != stream_loc ) fclose( stream_loc );
        if( NULL != stream_net ) fclose( stream_net );
        return false;
    }

    if( true == startCompare( hwnd, stream_loc, stream_net,
                             stream_loc_entries, stream_net_entries ))
    {
        return true;
    }

    return false;
}

static inline int32_t chkSum( uint32_t file_size, wchar_t * filecont )
{
    int32_t sum = 0;

    while( file_size-- > 0 )
        sum += *( filecont++ );

    return sum;
}

/* Perform a checksum comparison of the local and network files
   to see if there's a need to update before progressing */
extern int8_t initUpdate( HWND hwnd )
{
    FILE    * fptr_loc, * fptr_net;
    wchar_t * filecont_loc = NULL, * filecont_net = NULL;
    wchar_t * filename = NULL;
    uint32_t file_size_loc, file_size_net;

    if( NULL == ( fptr_loc = retFileHandleACR( L"r", &filename, 1 )))
    {
        throwError( FIL_GEN_ACC, 0 );
    }

    file_size_loc = getFileSize( fptr_loc, false );
    filecont_loc = loadACR( fptr_loc, file_size_loc );

    // checksum of local file
    int32_t sum_loc = chkSum( file_size_loc, filecont_loc );

    if( NULL == ( fptr_net = retFileHandleNETLOC( NULL, L"r" )))
    {
        if( NULL != filecont_loc )
        {
            free( filecont_loc ); filecont_loc = NULL;
        }
        return -1;
    }

    file_size_net = getFileSize( fptr_net, false );
    filecont_net = loadACR( fptr_net, file_size_net );

    // checksum of network file
    int32_t sum_net = chkSum( file_size_net, filecont_net );

    // update not required
    if( sum_loc == sum_net )
    {
        fclose( fptr_loc );
        fclose( fptr_net );
        free( filecont_loc ); filecont_loc = NULL;
        free( filecont_net ); filecont_net = NULL;

        return 0;
    }
    else
    {
        // Begin the process by opening a temporary file
        // then sorting and comparing
        if( false == beginTFOSC( hwnd, fptr_loc, fptr_net,
                                filecont_loc, filecont_net ))
        {
            return -2;
        }
        if( add_occurred )
        {
            add_occurred = false;
            return 1;
        }
        else
            return 0;

    }

    // Unknown error if we get here
    return -1;
}
