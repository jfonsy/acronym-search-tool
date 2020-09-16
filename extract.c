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

#include "extract.h"

static wchar_t * loadFileContents( FILE * fptr, uint32_t file_size )
{
    wchar_t * read_buffer;
    uint8_t   readBOM[ 2 ];

    // BOM bytes are 2 narrow chars
    fread( readBOM, 1, 2, fptr );

    // BOM for for UTF-16(LE) is FF FE
    if (( 0xFF == readBOM[ 0 ] ) && ( 0xFE == readBOM[ 1 ] ))
        fseek( fptr, 2, SEEK_SET ); //skip the BOM characters

    /* If it's something else, then attempt to read as UTF-8 and then convert
       to wide chars */
    else
    {
        char * read_buffer_oth = (char *) malloc(( file_size * sizeof(char)) +
                                                               sizeof(char));
        chkMalloc( read_buffer_oth, 11 );

        read_buffer_oth[fread( read_buffer_oth, sizeof(char), file_size, fptr )] = '\0';

        /* Call MultiBytetoWideChar once with last param == 0 to receive required size
           Then call it again with the last param set to the required size to receive
           the converted string */
        int bytes_to_write = 0;
        bytes_to_write = MultiByteToWideChar( CP_UTF8,
                                             0,
                                             read_buffer_oth,
                                             -1, //our string is null terminated
                                             NULL,
                                             0); // set to 0 to receive required size

        if( 0 == bytes_to_write )
            throwError( FIL_EXT_FORM, 0 );

        read_buffer = (wchar_t *) malloc(( bytes_to_write * sizeof(wchar_t)) +
                                                            sizeof(wchar_t));

        bytes_to_write = MultiByteToWideChar( CP_UTF8,
                                             0,
                                             read_buffer_oth,
                                             -1, //our string is null terminated
                                             read_buffer,
                                             bytes_to_write);

        free( read_buffer_oth ); read_buffer_oth = NULL;

        return( read_buffer );
    }

    read_buffer = (wchar_t *) malloc(( file_size * sizeof(wchar_t)) +
                                                   sizeof(wchar_t));
    chkMalloc( read_buffer, 11 );

    read_buffer[fread( read_buffer, sizeof(wchar_t), file_size, fptr )] = L'\0';

    return ( read_buffer );
}

/* Allocate a new database and free the old one */
static AcronymDB * getFilteredList( AcronymDB * extr_list, uint32_t filt_count )
{
    AcronymDB * filt_list = initExtractDB( filt_count );

    for( uint32_t r = 0; r < filt_count; r++ )
    {
        filt_list->acronyms[ r ] = malloc( sizeof( Acronym ));
        chkMalloc( filt_list->acronyms[ r ], 64 );

        filt_list->acronyms[ r ]->acronym =
                                  wcsdup( extr_list->acronyms[ r ]->acronym );
        filt_list->acronyms[ r ]->defn = NULL; // we don't have any definitions yet

        free( extr_list->acronyms[ r ]->acronym );
        free( extr_list->acronyms[ r ] );

        chkMalloc( filt_list->acronyms[ r ]-> acronym, 65 );
    }

    free( extr_list->acronyms ); extr_list->acronyms = NULL;
    free( extr_list );           extr_list = NULL;

    filt_list->entries = filt_count;

    return( filt_list );
}

/* Add unique entries and keep track of the filtered number */
static void
addUnique( AcronymDB * extr_list, wchar_t * acro, uint32_t * filt_count )
{
    uint32_t i = 0;

    /* first iteration evaluates to false, and the acronym will be added
    as there's nothing entered yet */
    while( i < *filt_count )
    {
        if( 0 == wcscmp( extr_list->acronyms[ i++ ]->acronym, acro ))
            return;
    }

    extr_list->acronyms[ *filt_count ] = malloc( sizeof( Acronym ));
    chkMalloc( extr_list->acronyms[ *filt_count ], 62 );

    extr_list->acronyms[ *filt_count ]->acronym = wcsdup( acro );
    chkMalloc( extr_list->acronyms[ (*filt_count)++ ]->acronym, 63 );

}

static AcronymDB * initExtractDB( uint32_t entries )
{
    AcronymDB * list = NULL;

    list = malloc( sizeof( AcronymDB ));
    chkMalloc( list, 60 );

    list->acronyms = malloc(( sizeof( Acronym ) * ( entries )));
    chkMalloc( list->acronyms, 61 );
    list->entries = entries;

    return( list );
}

/* Punctuation that we will consider as part of the acronym.
  More can be added but the matching will not be effective
  without further checks */
static inline bool isPunct( const wchar_t wc )
{
    if ( wc == L'-' || wc == L'&' )
        return true;

    return false;
}

static void getAcro( AcronymDB * extr_list, wchar_t * file_cont,
                                            uint32_t * posn,
                                            uint32_t * filt_count )
{
    wchar_t acro[ MAX_ACRO_LENGTH ];
    wchar_t * ptr = acro;
    uint32_t start_posn = *posn;

    while( iswupper( file_cont[ *posn ] ) ||
            isPunct( file_cont[ *posn ] ))
    {
        *ptr++ = file_cont[ *posn ];

        if( (*posn)++ > (start_posn + MAX_ACRO_LENGTH) ) return;
    }
    *ptr = L'\0';

    addUnique( extr_list, acro, filt_count );
}

/* Check for consecutive upper chars or
   an acronym with certain punctuation  */
static inline void
acroCheck( wchar_t * file_cont, uint32_t * posn, uint32_t * count_all )
{
    uint32_t start_posn = *posn;

    while( iswupper( file_cont[ *posn ] ) ||
            isPunct( file_cont[ *posn ] ))
    {
        if( (*posn)++ > (start_posn + MAX_ACRO_LENGTH) ) return;
    }
    (*count_all)++;
}

/* Minimum requirement before parsing can continue to further checks */
static inline bool
minMatchCheckAcro( wchar_t * file_cont, uint32_t curr_posn )
{
    for( uint8_t x = 0; x < MIN_ACRO_LENGTH; x++ )
    {
        if( !iswupper( file_cont[curr_posn + x] ) &&
             !isPunct( file_cont[curr_posn + x] ))
            return false;
    }
    return true;
}

static inline void
insertDefn( AcronymDB * filt_list, uint32_t entry_no, wchar_t * extr_defn )
{
    filt_list->acronyms[ entry_no ]->defn = wcsdup( extr_defn );
    chkMalloc( filt_list->acronyms[ entry_no ]->defn, 66 );
}

/* Copy the characters between each space and concatenate the words together
   to get the extracted definition */
static void
extrDefn( AcronymDB * filt_list, wchar_t const * file_cont,
            uint32_t entry_no, uint32_t * pos_index, uint8_t acro_len )
{
    /* Array of strings, with # of words == acro_len
       So if the acronym is ABC, we expect to find 3 corresponding words
    */
    wchar_t words[acro_len][MAX_DEFN_LENGTH];
    uint32_t start_pos = pos_index[ 0 ];

    for( uint8_t x = 0; x < acro_len; x++ )
    {
        uint32_t cp = pos_index[ x ]; //current position
        wchar_t word[ MAX_DEFN_LENGTH ];
        wchar_t * ptr = word;

        while( file_cont[ cp ] != L' ' )
        {
            if( cp >= ( start_pos + MAX_DEFN_LENGTH )) break;

            *ptr++ = file_cont[ cp++ ];

            // break if control characters are encountered
            if( file_cont[ cp ] <= 0x1F ||
                (( file_cont[ cp ] >= 0x7F ) &&
                 ( file_cont[ cp ] <= 0x9F ))) break;
        }

        *ptr = L'\0';
        wcscpy( words[x], word );
    }

    // concatenate all the strings
    wchar_t extr_defn[ MAX_DEFN_LENGTH ] = { 0 };
    for( uint8_t x = 0; x < acro_len; x++ )
    {
        wcscat( extr_defn, words[x] );
        // don't copy a space if it's the last word
        if( x == ( acro_len - 1 )) break;
        wcscat( extr_defn, L" " );
    }

    insertDefn( filt_list, entry_no, extr_defn );
}

/* Iterate through the filtered list. Look for consective words whose first
letters match the acronym letters. When all matched letters == the length
of the acronym, then the positions of those matches are used to extract the words
*/
static void
lookForDefns( HWND h_prog, AcronymDB * filt_list, wchar_t const * file_cont )
{
    uint32_t const file_cont_len = wcslen( file_cont );
    // increment amount for our progress from 35% to 85%
    float pct_incr = 50.0 / filt_list->entries;
    float pct = 0.0, old_pct = 0.0;
    wchar_t progress_info[ 30 ];

    for( uint32_t p = 0; p < filt_list->entries; p++ )
    {
        wchar_t const * acro = filt_list->acronyms[ p ]->acronym;
        uint8_t const acro_len = wcslen( acro );

        uint32_t cp = 0; //current position
        uint8_t matches = 0;

        // display every 1% increase in progress
        if( pct >= old_pct + 1.0f )
        {
            _snwprintf( progress_info, 30, L"            Extracting...%d%%",
                                                   (uint8_t)( pct + 35.0 ));

            SetWindowTextW( h_prog, progress_info );
            old_pct = pct;
        }
        pct = pct_incr * (float)p;

        for( uint8_t q = 0; q < acro_len; q++ )
        {
            uint32_t pos_ind[ acro_len ];

            while( cp < file_cont_len )
            {
    //wprintf( L"Index: %d ; comparing %c with %c\n", q, acro[q], file_cont[cp] );
                if( acro[ q ] == file_cont[ cp ] )
                {
                    matches += 1;
                    pos_ind[ q++ ] = cp;
                    if( matches == acro_len )
                    {
                        extrDefn( filt_list, file_cont, p, pos_ind, acro_len );
                        break;
                    }
                }
                else
                {
                    /* if there are no consecutive matches
                    then we reset the search back to the start of the acronym
                    and keep looking through the document */
                    matches = 0;
                    q = 0;
                }
                // locate each new space
                while( file_cont[ cp++ ] != L' ')
                    if( file_cont_len == cp ) break;
            }
        }
    }
}

static bool addExtrToFile( HWND h_prog, AcronymDB * filt_list )
{
    FILE    * fptr;
    int16_t result = 0;

    if( NULL == ( fptr = retFileHandleACR( L"a", NULL, 0 )))
    {
        throwError( FIL_GEN_ACC, 0 );
    }

    _setmode( _fileno( fptr ), _O_WTEXT );

    for( uint32_t m = 0; m < filt_list->entries; m++ )
    {
        /* Not all acronyms in the filtered list have an associated definition.
           We need to skip over these. When the filtered database was created,
           all the defns were set to NULL*/
        if( NULL != filt_list->acronyms[ m ]->defn )
        {
            result = fwprintf( fptr, L"\n%s # %s",
                               filt_list->acronyms[ m ]->acronym,
                               filt_list->acronyms[ m ]->defn );

            // some error, hard exit
            if( result < 0 )
                throwError( FIL_EXT_WRT, 0 );
        }
    }

    fclose( fptr );

    // free the database
    for( uint32_t n = 0; n < filt_list->entries; n++ )
    {
        free( filt_list->acronyms[ n ]->acronym );

        if( NULL != filt_list->acronyms[ n ]->defn )
            free( filt_list->acronyms[ n ]->defn );

        free( filt_list->acronyms[ n ] );
    }

    free( filt_list->acronyms );
    free( filt_list );

    SetWindowTextW( h_prog, L"    Written to file...100%" );



    MessageBoxW( GetParent(h_prog),
                 L"Extracted data has been written to the local file\n"
                 L"The database will now be reloaded",
                 L"Extraction Complete", MB_ICONINFORMATION );

    return true;
}

/* Matching requirement is a minimum of 3 consecutive upper case letters
 This will not get every acronym due to the variety out there */
static DWORD WINAPI processContents( LPVOID lpParam )
{
    ThreadData * data      = lpParam;
    HWND h_prog            = data->h_prog;
    wchar_t    * file_cont = data->file_cont;

    uint32_t cp = 0; //current position
    uint32_t const file_cont_len = wcslen( file_cont );
    uint32_t count_all = 0;

    SetWindowTextW( h_prog, L"             Extracting...5%" );

    // perform a count of all acronyms
    for( ; cp < file_cont_len; cp++ )
    {
        if( minMatchCheckAcro( file_cont, cp ))
        {
            acroCheck( file_cont, &cp, &count_all );
        }
    }

    if( 0 == count_all ) return 0;

    SetWindowTextW( h_prog, L"            Extracting...10%" );

    // initialise a database to store them
    cp = 0;
    AcronymDB * extr_list = initExtractDB( count_all - 1 );
    uint32_t filt_count = 0;

    // now enter those acronyms into a database if they are unique
    for( ; cp < file_cont_len; cp++ )
    {
        if( minMatchCheckAcro( file_cont, cp ))
        {
            // get a filtered count back
            getAcro( extr_list, file_cont, &cp, &filt_count );
        }
    }

    SetWindowTextW( h_prog, L"            Extracting...20%" );

    // create a new database with the filtered results, and delete the previous
    AcronymDB * filt_list = getFilteredList( extr_list, filt_count );

    SetWindowTextW( h_prog, L"            Extracting...35%" );

    lookForDefns( h_prog, filt_list, file_cont );

    SetWindowTextW( h_prog, L"     Writing to file...85%" );
    bool isAddFinished = addExtrToFile( h_prog, filt_list );

    disable_commands = false;
    free( file_cont );
    free( data );

    if( isAddFinished )
        reloadACR( h_prog );

    return 0;
}

static void startParsing( HWND h_prog, FILE * fptr_e )
{
    ThreadData * data = malloc( sizeof( ThreadData ));
    uint32_t const file_size = getFileSize( fptr_e, false );

    data->h_prog = h_prog;
    data->file_cont = loadFileContents( fptr_e, file_size );

    HANDLE hThr_parser = CreateThread(
        NULL,             // default security attributes
        0,                // default stack size
        processContents,  // function to be called
        data,             // thread data
        0,                // default thread creation flags
        NULL);            // get thread id

    // Since fclose is used, we don't need to close the handle later
    fclose( fptr_e );
    // As per MSDN, this won't terminate the thread
    // but we should free the handle
    CloseHandle( hThr_parser );
}

extern bool loadFileToExtract( HWND hwnd )
{
    OPENFILENAMEW ofn;
    HANDLE hf;
    FILE * fptr_e;
    wchar_t file_to_extract[ MAX_PATH ] = { 0 };
    wchar_t cwd[ MAX_PATH ];

    // we need to reset back to the original directory later
    GetCurrentDirectoryW( MAX_PATH, cwd );

    ZeroMemory( &ofn, sizeof( ofn ));
    ofn.lStructSize = sizeof( ofn );
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = file_to_extract;
    ofn.nMaxFile = sizeof( file_to_extract );
    ofn.lpstrFilter = L"Text\0*.TXT\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if( true == GetOpenFileNameW( &ofn ))
    {
        hf = CreateFileW( ofn.lpstrFile,
                         GENERIC_READ,
                         0, // disable shared access until completion
                         (LPSECURITY_ATTRIBUTES)NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         (HANDLE)NULL );

        if( hf == INVALID_HANDLE_VALUE )
        {
            disable_commands = false;
            return false;
        }

        // associates a CRT file descriptor with an existing handle
        int n_handle = _open_osfhandle((long)hf, _O_RDONLY );
        if( -1 == n_handle )
        {
            CloseHandle( hf );
            disable_commands = false;
            return false;
        }

        fptr_e = _wfdopen( n_handle, L"r" );
        if( !fptr_e )
        {
            CloseHandle( hf );
            disable_commands = false;
            return false;
        }

    HWND h_prog =
         FindWindowExW( hwnd, NULL, L"STATIC", NULL );

    startParsing( h_prog, fptr_e );
    // reset the directory back to the program folder
    SetCurrentDirectoryW( cwd );

    }
    else
        disable_commands = false;

    return false;
}
