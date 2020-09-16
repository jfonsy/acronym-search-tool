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

static void prtInputErr( HWND hwnd, wchar_t * retstring )
{
    if( 0 == wcscmp( retstring, L"err_max" ) )
    {
        SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
            (LPARAM)L"Maximum input is 255 characters\n" );
    }
    else
    {
        SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                    (LPARAM)L"Invalid Input\n" );
    }
}

extern bool chkRawInput( HWND hwnd, wchar_t * retstring )
{
    if( NULL != retstring )
    {
        if( 0 == wcscmp( retstring, L"err_max" ) ||
            0 == wcscmp( retstring, L"err_inv" ) )
        {
            colourMe( hwnd, FRGD_ERROR );
            prtInputErr( hwnd, retstring );

            return true;
        }
    }

    return false;
}

extern wchar_t * getInput( HWND hwnd )
{
    wchar_t raw_input[ RAWINPUTMAX ];
    uint32_t charindex, linelength, lineindex, n_chars;

    //char index of first char in current line(bRed on caret posn)
    charindex  = SendMessageW( hwnd, EM_LINEINDEX, (WPARAM)          - 1, 0 );
    linelength = SendMessageW( hwnd, EM_LINELENGTH,(WPARAM)charindex - 1, 0 );

    if( linelength > RAWINPUTMAX ) return wcsdup( L"err_max" );

    lineindex  = SendMessageW( hwnd, EM_EXLINEFROMCHAR, 0, (LPARAM)charindex );
    //n_chars is linelength + 1 due to CR
    *( (uint16_t*)raw_input ) = sizeof( raw_input ) / sizeof( wchar_t );
    //lineindex - 1 as EM_GETLINE asks for a zero-based index for WPARAM
    n_chars    = SendMessageW( hwnd, EM_GETLINE, (WPARAM)lineindex - 1, (LPARAM)raw_input );
    raw_input[ --n_chars ] = L'\0';

    // if no input is given (only '\n) return invalid -- BREAK THIS OUT LATER TO ANOTHER FUNCTION AND CLEAN UP
    if( 0 == n_chars ||
      ( 1 == n_chars && ( L'.' != raw_input[ 0 ] )))
        //only single character program end input valid '.'  called from main
    {
        return( wcsdup( L"err_inv" ) );
    }

#ifdef DEBUG
    printf( "Character Index: %u - Line Length: %u - n_chars: %u - Line Number: %u\n",
             charindex,             linelength,        n_chars,       lineindex );
#endif // DEBUG

    return wcsdup( raw_input );
}

extern bool lookUpAcro( HWND hwnd, AcronymDB * entry_list,
                                   wchar_t const * retstring,
                                   wchar_t const * addDefnString,
                                   uint8_t const caller, CHARFORMAT2W cf )
{
    uint32_t b = 0;
    bool acro_found = false; //only used with main caller
    bool defn_found = false; //only used with cmdAddAcro caller

    Acronym * currAcro = entry_list->acronyms[ b ];

    while ( b <= entry_list->entries )
    {
        if ( 0 == _wcsicmp( currAcro->acronym, retstring ) )
        {
            wchar_t * currDefn = entry_list->acronyms[ b ]->defn;

            if( NULL != currDefn )
            {
                //have to add wcslen() as an argument, as the defn in the file contains CR LF which
                //results in _wcsicmp > 0 - this issue didn't exist in the narrow string version
                //using strcmpi, hmm...
                if( ( 1 == caller ) &&
                    ( 0 == _wcsnicmp( currDefn, addDefnString, wcslen( addDefnString ) ) ) )
                {
                    //printf("\ndefn match\n");
                    defn_found = true;
                }

                else if ( 0 == caller )
                {
                    colourMe( hwnd, FRGD_DEFNTRUE );

                    SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                            (LPARAM)L"        " );
                    SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                            (LPARAM)currDefn );

                    //last entry doesn't have CR LF, so send it
                    if( b == entry_list->entries )
                    {
                        SendMessageW( hwnd, WM_KEYDOWN, VK_END, 0 );
                        SendMessageW( hwnd, WM_KEYDOWN, VK_RETURN, 0 );
                    }
                }
            }
            if     ( ( 1 == caller ) && ( true  == defn_found ) )  return (  true );
            acro_found = true;
            //in case there are duplicate acronym entries with different definitions
            currAcro = entry_list->acronyms[ ++b ];
        }
        else
        {
            currAcro = entry_list->acronyms[ ++b ];
        }

    }
    //a matching defn was never found in the loop above
    if( 1 == caller ) return false;

    if( false == acro_found && 0 == caller )
    {
        cf.crTextColor = FRGD_ERROR;
        SendMessageW( hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf );

        SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                (LPARAM)L"        Acronym not found\n" );
#ifdef DEBUG
    if ( IsWindowUnicode( hwnd ) )
        printf( "Yes the Window is Unicode\n" );
    else
        printf( "No the Window is not Unicode\n" );
#endif // DEBUG
    }
    return ( acro_found );
}
