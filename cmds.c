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

static inline void prntDefaultCmds( HWND hwnd, CHARFORMAT2W cf )
{
    cf.crTextColor = FRGD_OTH;
    SendMessageW( hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf );

    SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
            (LPARAM)L"Available Commands:\n"
                    L" 1) <<a - Add Acronym\n"
                    L" 2) <<u - Update\n"
                    L" 3) <<r - Resolve Duplicates\n"
                    L" 4) <<d - Delete Last Entry\n\n" );

    cf.crTextColor = FRGD_GENERAL;
    SendMessageW( hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf );
 }

static inline void cmdErr( HWND hwnd, CHARFORMAT2W cf )
{
    cf.crTextColor = FRGD_ERROR;
    SendMessageW( hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf );

    SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
            (LPARAM)L"Unrecognised Command\n\n" );

    prntDefaultCmds( hwnd, cf );
}

extern uint8_t prcsCmd( HWND hwnd, wchar_t * retstring, CHARFORMAT2W cf )
{

    cf.dwMask = CFM_COLOR;
    cf.crTextColor = FRGD_OTH;
    SendMessageW( hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf );

    switch ( wcslen( retstring ))
    {
        case 2:

            prntDefaultCmds( hwnd, cf );
            return 0;

            break;

        case 3:

            if(     L'A' == towupper( retstring[ 2 ] ))
            {
                if( disable_commands ) { disabledCmdMsg( hwnd ); return 0; }

                SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                    (LPARAM)L"Enter Acronym\n" );

                return 1;

            }
            else if( L'R' == towupper( retstring[ 2 ] ))
            {

                if( disable_commands ) { disabledCmdMsg( hwnd ); return 0; }

                colourMe( hwnd, FRGD_INFO );
                SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                        (LPARAM)L"Checking for duplicates, this can take some time...\n" );
                UpdateWindow( hwnd );

                if( true == cmdResDup( hwnd ) )
                    return 3;
                else
                    return 4;

            }
            else if( L'U' == towupper( retstring[ 2 ] ))
            {

                if( disable_commands ) { disabledCmdMsg( hwnd ); return 0; }

                return 2;
            }
            else if( L'D' == towupper( retstring[ 2 ] ))
            {

                if( disable_commands ) { disabledCmdMsg( hwnd ); return 0; }

                return 5;
            }
            else
            {
                cmdErr( hwnd, cf );
                return 0;
            }
            break;

        default:

            cmdErr( hwnd, cf );
            return 0;

            break;
    }

    return -1;
}
