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
#include "load.h"

extern uint32_t getFileSize( FILE * fptr, bool initial_load ) //errcodes 5 - 9
{
    int fd = fileno( fptr );
    if( -1 == fd )
        throwError( FIL_LOAD_DESC, 0 );

    int32_t file_size = _filelength( fd );
    if(( file_size >= MAX_ACR_SIZE ) && ( initial_load ))
        throwError( FIL_LOAD_SIZE, 0 );

    return( file_size );
}

extern wchar_t * loadACR( FILE * fptr, uint32_t const file_size ) //errcodes 10 - 14
{
    wchar_t * read_buffer;
    uint8_t   readBOM[ 2 ];

    // BOM bytes are 2 narrow chars
    fread( readBOM, 1, 2, fptr );

    // BOM for for UTF-16(LE) is FF FE
    if (( 0xFF == readBOM[ 0 ] ) && ( 0xFE == readBOM[ 1 ] ))
        fseek( fptr, 2, SEEK_SET ); //skip the BOM characters
    else
       throwError( FIL_LOAD_ENC, 0 );

    read_buffer = (wchar_t *) malloc(( file_size * sizeof( wchar_t ))
                                                 + sizeof( wchar_t ));
    chkMalloc( read_buffer, 11 );

    HANDLE h_file =  (HANDLE)_get_osfhandle(_fileno( fptr ));
    DWORD bytes_read;

    /* Using ReadFile seems to produce better results with wide chars
       fread seems to have issues with certain characters, such as Cyrillic */
    if( false == ReadFile( h_file, read_buffer, file_size, &bytes_read, NULL ))
        throwError( FIL_LOAD_DESC, 0 );

    read_buffer[ bytes_read ] = L'\0';

    return ( read_buffer );
}

static void addAcronym( AcronymDB * entry_list,
                          wchar_t * delimbuff,
                              uint32_t const r ) //errcodes 15 - 19
{
    wchar_t * ptr_delim;
    //convert the characters to uppercase to make matching easier later
    for ( ptr_delim = delimbuff; *ptr_delim != L'\0'; ptr_delim++ )
    {
        *ptr_delim = towupper( *ptr_delim );
    }

    entry_list->acronyms[ r ] = malloc( sizeof( Acronym ));
    chkMalloc( entry_list->acronyms[ r ], 15 );

    entry_list->acronyms[ r ]->acronym = wcsdup( delimbuff );
    chkMalloc( entry_list->acronyms[ r ]->acronym, 16 );
}

static inline void addDefn( AcronymDB * entry_list, wchar_t * delimbuff,
                            uint32_t const r ) //errcodes 25 - 29
{
    entry_list->acronyms[ r ]->defn = wcsdup( delimbuff );
    chkMalloc( entry_list->acronyms[ r ]->defn, 25 );
}

// create space for the entry list based on rows counted earlier
static AcronymDB * initAcroDB( uint32_t rows ) //errcodes 30 - 34
{
    AcronymDB * entry_list = malloc( sizeof( AcronymDB ));
    chkMalloc( entry_list, 30 );

    entry_list->acronyms = malloc(( sizeof( Acronym ) * ( rows + 1 )));
    chkMalloc( entry_list->acronyms, 31 );
    entry_list->entries = rows;

    return ( entry_list );
}

extern AcronymDB * loadToMemory( FILE * fptr )
{
    AcronymDB * entry_list = NULL;
    wchar_t   * file_cont = NULL;
    uint32_t    file_size = 0;

    if(( fptr = retFileHandleACR( L"r", NULL, 0 )) == NULL )
    {
        throwError( FIL_LOAD_MISS, 0 );
    }

    file_size = getFileSize( fptr, true );
    file_cont  = loadACR( fptr, file_size );
    fclose( fptr );

    // create linked lists of acronyms and defs from the file contents
    entry_list = mapAcroDefs( file_cont );

    return( entry_list );
}

static AcronymDB * mapAcroDefs( wchar_t * file_cont ) //errcodes 35 - 39
{
    uint32_t  rows = countRows( file_cont ), r = 0;
    wchar_t * delimbuff;

    AcronymDB * entry_list = initAcroDB( rows );
    // create space for database of acronyms based on entries counted

    // tokenise the file contents, the first split will be an acronym entry
    //including a space before the hash
    delimbuff = wcstok( file_cont, L"#\n" );

    while ( r <= rows )
    {   //catch potential errors in the file (not tokenised according to # or \n
        if ( NULL == delimbuff )
            throwError( 35, r + 1 );

        //return location of space before the hash
        wchar_t * spcptr = wcschr( delimbuff, L' ' );
        if ( NULL == spcptr )
            throwError( 36, r + 1 );

        //terminate the word correctly, hopefully we have a valid acronym entry now
        *spcptr = L'\0';

        remPunct( delimbuff );
        addAcronym( entry_list, delimbuff, r );

        while ( NULL != delimbuff )
        {
            delimbuff = wcstok( NULL, L"#\n" );

            // get rid of the space that will be present
            // otherwise there's a formatting error present; missing hash etc.
            if ( L' ' == delimbuff[ 0 ] )
                wmemmove( delimbuff, delimbuff + 1, wcslen( delimbuff ));

            delimbuff[ wcslen( delimbuff ) ] = L'\0';

            addDefn( entry_list, delimbuff, r );

            if ( NULL == wcsstr( delimbuff, L"\n" ) )
            {
                r++;
                delimbuff = wcstok( NULL, L"#\n" );
                break;
            }

        }
    }

    free( file_cont ); file_cont = NULL;
    return ( entry_list );
}
