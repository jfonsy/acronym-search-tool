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
#include "cmddup.h"

// LINK 2
ssize_t getwdelim( wchar_t ** restrict lineptr, size_t * restrict n,
                   wint_t delim, FILE * restrict stream )
{
    wchar_t * cur_pos, * new_lineptr;
    size_t new_lineptr_len;
    wint_t wc;

    if ( lineptr == NULL || n == NULL || stream == NULL )
    {
        errno = EINVAL;
        return -1;
    }

    if ( *lineptr == NULL )
    {
        *n = 560; /* init len */
        if (( *lineptr = (wchar_t *)malloc( *n )) == NULL )
        {
            errno = ENOMEM;
            return -1;
        }
    }

    cur_pos = *lineptr;
    for (;;)
    {
        wc = getwc( stream );

        if ( ferror( stream ) || ( wc == WEOF && cur_pos == *lineptr ))
            return -1;

        if ( wc == WEOF )
            break;

        if (( *lineptr + *n - cur_pos ) < 2 )
        {
            if ( SSIZE_MAX / 2 < *n )
            {
#ifdef EOVERFLOW
                errno = EOVERFLOW; //mingw headers don't have this
#else
                errno = ERANGE; /* no EOVERFLOW defined */
#endif
                return -1;
            }
            new_lineptr_len = *n * 2;

            if (( new_lineptr = (wchar_t *)realloc( *lineptr, new_lineptr_len )) == NULL )
            {
                errno = ENOMEM;
                return -1;
            }
            cur_pos = new_lineptr + ( cur_pos - *lineptr );
            *lineptr = new_lineptr;
            *n = new_lineptr_len;
        }

        *cur_pos++ = (wchar_t)wc;

        if ( wc == delim )
            break;
    }

    *cur_pos = L'\0';
    return (ssize_t)( cur_pos - *lineptr );
}

ssize_t getwline( wchar_t ** restrict lineptr, size_t * restrict n, FILE * stream )
{
    return getwdelim( lineptr, n, L'\n', stream );
}

static void fullDelDuplAcr( FILE * stream, wchar_t const * const dup,
                                           wchar_t const * const desc_dup,
                                           uint32_t const desc_stream_pos )
{
    wint_t   wc;
    uint32_t prev_stream_pos = ftell( stream );  //store original position of fptr

    //go to position of the acronym description
    fseek( stream, 0, SEEK_SET );
    fseek( stream, desc_stream_pos, SEEK_CUR );

    //go to the end of the line of the first description
    while( L'\n' != ( wc = fgetwc( stream )));

    //we are now at the position of the duplicate
    uint32_t dup_pos = ftell( stream );
    uint32_t chkWEOF = getFileSize( stream, false ) - dup_pos;

    //restore
    fseek( stream, 0, SEEK_SET );
    fseek( stream, dup_pos, SEEK_CUR );

    // fseek to assert new position before read/write calls
    fseek( stream, 0, SEEK_CUR );

    uint16_t j = wcslen( desc_dup ) + wcslen( dup ) + 2;
    for( uint16_t i = 0; i < j; i++ )
    {
        fputwc( L'$', stream );
        if( 0 == (chkWEOF -= 2) )  //we are at the end of the file now
        {
            //go back 2 positions from dup_pos, and overwrite CR LF
            fseek( stream, dup_pos - 4, SEEK_SET );
            fseek( stream, 0, SEEK_CUR );
            fputwc( L'$', stream );
            fputwc( L'$', stream );
            break;
        }

    }

    fseek( stream, 0, SEEK_SET );     //restore original position of stream
    fseek( stream, prev_stream_pos, SEEK_CUR );

}

static bool
compDesc( wchar_t const * const desc_acro, wchar_t const * const desc_dup )
{
    wchar_t  cpy_desc_acro[ 256 ] = { 0 };
    wchar_t  cpy_desc_dup[ 256 ]  = { 0 };
    uint16_t len_desc_acro = wcslen( desc_acro );
    uint16_t len_desc_dup  = wcslen( desc_dup );
    uint16_t x;

    wmemcpy( cpy_desc_acro, desc_acro, len_desc_acro + 1 );
    cpy_desc_acro[ len_desc_acro - 2 ] = L'\0'; //terminate at CR LF

    // the second description must be handled differently
    // if it's at the end of the file, it won't have CR LF
    for( x = 0; x < len_desc_dup; x++ )
    {
        if( 0x0D != desc_dup[ x ] && 0x0A != desc_dup[ x + 1 ] )
            cpy_desc_dup[ x ] = desc_dup[ x ];
    }
    cpy_desc_dup[ x ] = L'\0';

    if ( 0 == wcscmp( cpy_desc_acro, cpy_desc_dup ) )return true; //same definition

    return false;
}

//take a look at the definition entry and see if there are multiple entries
static int8_t
lookAtDef( wchar_t const * const desc_dup, wchar_t const * const desc_acro )
{

    if( true == compDesc( desc_acro, desc_dup ) )
    {
        return 0;                         //direct match of descriptions
    }

    return -1; //some kind of error, we shouldn't get here
}

//this function should search the acronym list for a match of the acronym it was passed by resdup_listAcro
static bool
resdup_findAcroOcc( FILE * stream, wchar_t const * const acro,
                                   wchar_t const * const desc_acro,
                                   uint32_t const desc_stream_pos )
{

    wchar_t * dup = NULL, * desc_dup = NULL, * hashptr = NULL;    //duplicate acro, and duplicate description
    size_t   len1 = 0, len2 = 0;
    uint32_t prev_stream_pos = ftell( stream );   //store original position of stream upon function entry
    int8_t   res_lookAtDef;
    bool     flag = false;

    while( -1 != getwdelim( &dup, &len1, L'#', stream ) )
    {
        //The getline function below will move the stream one line forward after getting
        //the duplicate description. If we need to call delete subsequently,
        //then we need to give the position of the description obtained earlier as well
        if( -1 != getwline( &desc_dup, &len2, stream ) )
        {
            hashptr = wcschr( dup, L'#' );
            dup[ hashptr - dup - 1 ] = L'\0';

            if ( 0 == wcscmp( dup, acro ) )   //acronyms match, now check the descriptions
            {
                res_lookAtDef = lookAtDef( desc_dup, desc_acro );
                switch( res_lookAtDef )
                {
                    case -1:
                                  //error
                        break;

                    //descriptions match
                    //mark the entire line of dupl for deletion
                    default:
                        fullDelDuplAcr( stream, dup, desc_dup, desc_stream_pos );
                        flag = true;
                        break;
                }

            }
        }
    }
        //as stream was being moved around a lot, we need to go back to the start of the file
        //and then to the location where stream was originally pointing
    fseek( stream, 0, SEEK_SET );
    fseek( stream, prev_stream_pos, SEEK_CUR );
    free( dup );      dup      = NULL;
    free( desc_dup ); desc_dup = NULL;

    if( true == flag ) return true;

    return false;
}

static bool resdup_listAcro( HWND hwnd, FILE * stream )
{
    wchar_t   * acro = NULL, * desc_acro = NULL, * hashptr = NULL;
    wchar_t     buf[ BUF ];
    size_t      len = 0;
    static bool flag = false;
    uint32_t    desc_stream_pos;

    while( -1 != getwdelim( &acro, &len, '#', stream ) )
    {
        desc_stream_pos = ftell( stream );
        if( -1 != getwline( &desc_acro, &len, stream ) ) //go to the next line
        {
            hashptr = wcschr( acro, L'#' );          //point to occurence of hash
            acro[ hashptr - acro - 1 ] = L'\0';
        //hashptr - acro - 1, terminate string at location of space before the hash (mem address subtraction)

                                    //pass the acronym, description, and position of description
            if( true == resdup_findAcroOcc( stream, acro, desc_acro, desc_stream_pos ) )
            {
                if( false == flag )
                {
                    colourMe( hwnd, FRGD_INFO );
                    SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                            (LPARAM)L"The following duplicates were found:\n" );

                    flag = true;
                }

                _snwprintf( buf, BUF, L"    %s\n", acro );
                colourMe( hwnd, FRGD_INFO );
                SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)buf );

                free( acro );      acro = NULL;
                free( desc_acro ); desc_acro = NULL;
                UpdateWindow( hwnd );
            }
        }
    }

    if ( false == flag )
    {
        colourMe( hwnd, FRGD_INFO );
        SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                (LPARAM)L"No duplicates were found\n\n" );
        return false;
    }
    else
    {
        flag = false;
        return true;
    }
    //reset the flag and return true; yes one or more duplicates were found
}

static bool resdup_newACR( FILE * stream )
{
    wint_t   wc;
    uint32_t file_size, bytes_rem;
    time_t   rawtime;
    struct   tm * timeinfo;
    wchar_t  newacr_filename[ 75 ];
    FILE *   fptr2;

    time( &rawtime );
    timeinfo = localtime( &rawtime );
    swprintf( newacr_filename, L"acronyms_[%02d-%02d-%02d_%02d.%02d.%02d].acr",
                                     timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year - 100,
                                            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec );

    if( NULL == ( fptr2 = _wfopen( newacr_filename, L"w" )))
        throwError( FIL_DUP_CRT, 0 );

    file_size = getFileSize( stream, false );

    _setmode( _fileno( stream ), _O_BINARY );
    _setmode( _fileno( fptr2 ), _O_BINARY );
    // set to binary mode to avoid unwanted character conversion or CR-LF issues
    // LINK 3

    fseek( stream, 0, SEEK_SET );
    fseek( fptr2, 0, SEEK_SET );

    fputc( 0xFF, fptr2 ); //enter the BOM for UTF-16LE
    fputc( 0xFE, fptr2 ); //they are entered as narrow chars

    bytes_rem = file_size;

    do {
        wc = fgetwc( stream );
        if( L'$' != wc )
        {
            fputwc( wc, fptr2 );
            fseek( fptr2, 0, SEEK_CUR );
        }
        bytes_rem -= 2;
    } while( bytes_rem > 0 );

    fclose( fptr2 );

    return( true );
}

static void renameFile( HWND hwnd, wchar_t * filename )
{
    wchar_t * s = &filename[ 0 ];
    wchar_t renamed_file[ MAX_PATH ];
    uint8_t i = 0;

    while( L'\0' != *s )
    {
        if( ( L'.' == *s       )  &&
            ( L'a' == *( s + 1 )) &&
            ( L'c' == *( s + 2 )) &&
            ( L'r' == *( s + 3 )))
        {
            renamed_file[ i++ ] = L'.';
            renamed_file[ i++ ] = L'o';
            renamed_file[ i++ ] = L'l';
            renamed_file[ i++ ] = L'd';
            renamed_file[ i++ ] = L'a';
            renamed_file[ i++ ] = L'c';
            renamed_file[ i++ ] = L'r';
            renamed_file[ i ] = L'\0';
            break;
        }
        renamed_file[ i++ ] = *s++;
    }

    _wrename( filename, renamed_file );
    free( filename ); filename = NULL;

    colourMe( hwnd, FRGD_INFO );
    SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
            (LPARAM)L"Done\n" );
    colourMe( hwnd, FRGD_GENERAL );
    SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
            (LPARAM)L"You may have to run the duplicate check again "
                 "if there were excessive copies!\n\n" );

}


static int32_t cmpr( const void * a, const void * b )
{
    return( wcscmp( *( wchar_t ** )a, *( wchar_t ** )b ));
}

// LINK 4
// temporary file creation as windows doesn't have a direct fmemopen equivalent
extern FILE * tmp_wopen( void * buf, size_t len )
{
    int fd;
    FILE * fp;
    char tp[ MAX_PATH - 13 ];
    char fn[ MAX_PATH + 1 ];

    if ( !GetTempPathA( sizeof( tp ), tp ))
        return NULL;

    if ( !GetTempFileNameA( tp, "confuse", 0, fn ))
        return NULL;

    fd = _open( fn,
        _O_CREAT | _O_RDWR | _O_SHORT_LIVED | _O_TEMPORARY | _O_BINARY,
        _S_IREAD | _S_IWRITE );
    if ( -1 == fd )
        return NULL;

    fp = _fdopen( fd, "w+" );
    if ( !fp )
    {
        _close( fd );
        return NULL;
    }

    fwrite( buf, sizeof(wchar_t), len, fp );
    rewind( fp );

    return fp;
}

extern FILE * sort_listAcro( FILE * stream, wchar_t * filecont )
{
    size_t size_strings = 0;
    uint32_t rows = countRows( filecont ), r = 0;
    wchar_t line[ MAX_DEFN_LENGTH + MAX_ACRO_LENGTH + 5 ];
    wchar_t * line_array[ rows ];
    FILE * s_stream = NULL;

    while( NULL != fgetws( line, sizeof( line ), stream ))
    {
        line[ wcscspn( line, L"\r" ) ] = L'\0';

        if( r <= rows )
        {
            line_array[ r++ ] = wcsdup( line );
            size_strings += wcslen( line );
        }
        else
            break;
    }

    qsort( line_array, r, sizeof( const wchar_t * ), cmpr );
    fclose( stream ); //no longer needed, we have sorted strings

      //open a temp file with contents of the sorted strings
    s_stream = tmp_wopen( &line_array, size_strings );

    for( uint32_t y = 0; y < rows; y++ )
    {
    //write to the file and free earlier mallocs from strdup
        fwprintf( s_stream, L"%s\r\n", line_array[ y ] );
        free( line_array[ y ] );
        line_array[ y ] = NULL;
    }
    //don't add a newline to the last line
    fwprintf( s_stream, L"%s", line_array[ rows ] );

    rewind( s_stream );
    return( s_stream );
}

extern bool cmdResDup( HWND hwnd )
{
    FILE * fptr, * stream = NULL; //stream is for the temp file
    bool resolved_dupl = false, created_newACR = false;
    wchar_t * filename = NULL;
    wchar_t * filecont = NULL;
    uint32_t file_size;

    if( NULL == ( fptr = retFileHandleACR( L"r", &filename, 1 )))
    {
        throwError( FIL_GEN_ACC, 0 );
    }

    else
    {
        file_size = getFileSize( fptr, false );
        filecont = loadACR( fptr, file_size );

        if( NULL == ( stream = tmp_wopen( filecont, wcslen( filecont ))))
        {
          //handle properly if tmp_wopen doesn't work
            free( filecont ); filecont = NULL;
            return false;
        }

        // return a sorted file
        stream = sort_listAcro( stream, filecont );
        free( filecont ); filecont = NULL;
        // now attempt to resolve duplicates
        resolved_dupl = resdup_listAcro( hwnd, stream );
    }

    //if duplicates were resolved, we need to clean up and write to a new file
    if( true == resolved_dupl )
    {
        colourMe( hwnd, FRGD_INFO );
        SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                (LPARAM)L"Resolving duplicates..." );
        UpdateWindow( hwnd );

        if( true == resdup_newACR( stream ) ) created_newACR = true;
    }

    fclose( fptr );
    fclose( stream );

    if ( true == created_newACR )
    {
        renameFile( hwnd, filename );
        return true;
    }

    free( filename ); filename = NULL;
    return false;
}
