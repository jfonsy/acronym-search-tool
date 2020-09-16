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
#include "procs.h"

static void freeAcroDB( void )
{
    uint32_t r = 0;

    Acronym * currAcro = entry_list->acronyms[ r ], * tmpAcro;
    wchar_t * head, * currDefn;

    while( r <= entry_list->entries )
    {
        head = entry_list->acronyms[ r ]->defn;
        currDefn = head;

        if ( NULL != currDefn )
        {
            free( currDefn );       currDefn = NULL;
        }

        tmpAcro = entry_list->acronyms[ ++r ];

        free( currAcro->acronym );  currAcro->acronym = NULL;
        free( currAcro );           currAcro = NULL;
        currAcro = tmpAcro;
    }

    free( entry_list->acronyms );   entry_list->acronyms = NULL;
    free( entry_list );             entry_list = NULL;
}

extern void reloadACR( HWND hwnd )
{
    //locate the child window at top right displaying # of acros
    HWND acrloaded = FindWindowExW( GetParent( hwnd ), NULL, L"STATIC", NULL );
    //free the current list, and re-initialise with the updated file
    freeAcroDB();
    initialise();
    //set new text containing updated # of acros loaded
    wchar_t * acronyms_loaded = acrLoadedText( entry_list );
    SetWindowTextW( acrloaded, acronyms_loaded );
}

static void protectMe( HWND hwnd, CHARRANGE current_text,
                       uint32_t * oldcpmax, CHARFORMAT2W cf )
{
    uint32_t last_char_posn = 0;

    //get range of existing text in rich edit control
    //the current max will the new minimum for the next line
    last_char_posn = lastCharIndex( hwnd, false );
    current_text.cpMin = *oldcpmax;
    current_text.cpMax = LOWORD( last_char_posn );
    *oldcpmax = current_text.cpMax;

    //set parameters to enable protection
    //and format change to show older entries
    cf.dwMask      = CFM_PROTECTED;
    cf.dwEffects   = CFE_PROTECTED;

    //apply protection to the existing text
    SendMessageW( hwnd, EM_EXSETSEL, 0, (LPARAM)&current_text );
    SendMessageW( hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf );
    //unselect
    SendMessageW( hwnd, WM_KEYDOWN, VK_RIGHT, 0 );

    cf.dwMask = CFM_COLOR;
    cf.crTextColor = FRGD_GENERAL;
    SendMessageW( hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf );
}

static bool chkInput( HWND hwnd,  wchar_t * retstring,
                      CHARFORMAT2W cf, uint8_t caller )
{
    size_t len = wcslen( retstring );
    uint8_t i = 0, j = 0;

    cf.crTextColor = FRGD_ERROR;
    SendMessageW( hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf );

    switch( caller )
    {

    case 0:

    case 1:

    /*TRIM OUT CERTAIN PUNCTUATION, AND SPACES */
        remPunct( retstring );

    /*GET THE NEW LENGTH, CONV TO UCASE, AND CHECK IF > LIMIT */
        len = wcslen( retstring );
        for( i = 0; i < len; i++ )
            retstring[ i ] = towupper( retstring[ i ] );

#ifdef DEBUG
        printf( "%d\n", wcslen( retstring ) );
#endif // DEBUG

        if( len > MAX_ACRO_LENGTH )
        {
            SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                (LPARAM)L"Maximum acronym length is 15 characters\n\n" );
            return false;
        }

        if( 1 == len && ( L'.' == retstring[ 0 ] ))
        {
            SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                        (LPARAM)L"Invalid Input\n\n" );
            return false;
        }

        break;

    case 2:

        if( 0 == wcscmp( retstring, L"err_inv" ) )
        {
            SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
            (LPARAM)L"Invalid Input\n\n" );

            return false;
        }

        len = wcslen( retstring );

        for( i = 0; i < len; i++ )
        {

            if( MAX_DEFN_LENGTH == i )
            {
                SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                    (LPARAM)L"Maximum definition word length is 250 characters\n\n" );
                return false;
            }
            if ( L'#' != retstring[ i ] &&
                 L';' != retstring[ i ] &&
                 L'$' != retstring[ i ] ) retstring[ j++ ] = retstring[ i ];
      //disregard '#' and ';' or it will crash the program later, '$' is used to mark deletions
        }

        retstring[ j ] = L'\0';

        if( L' ' == retstring[ 0 ] || L' ' == retstring[ j - 1 ] )
        {
            SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                (LPARAM)L"Leading/trailing spaces, or more than one space"
                         L" between words is not permitted\n\n" );

            return false;
        }
        if( j < 3 )
        {
            SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                (LPARAM)L"A minimum of 3 characters is required\n\n" );

            return false;
        }

        bool space = false;
        for( uint8_t x = 1; x < i; x++ )
        {
            if     ( L' ' == retstring [ x ] &&  true == space )
            {
                SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                    (LPARAM)L"Leading/trailing spaces, or more than one space"
                             L" between words is not permitted\n\n" );

                return false;
            }
            else if( L' ' != retstring [ x ] &&  true == space ) space = false;
            else if( L' ' == retstring [ x ] && false == space ) space = true;
        }


        break;
    }

    return true;
}

COORD getCarPos( HWND hwnd )
{
    COORD pos;
    CHARRANGE cr;

    pos.Y = (LONG)SendMessageW( hwnd, EM_LINEFROMCHAR, (WPARAM)-1, 0 ) + 1;

    cr.cpMin = 0;
    cr.cpMax = 0;
    SendMessage( hwnd, EM_EXGETSEL, 0, (LPARAM)&cr );

    LONG firstcharacter = SendMessageW( hwnd, EM_LINEINDEX, (WPARAM)-1, 0 );

    pos.X = cr.cpMin - firstcharacter;

    return pos;
}

LRESULT CALLBACK MultiLineProc( HWND hwnd, UINT msg,
                                WPARAM wParam, LPARAM lParam )
{
    static bool refresh = false;
    static      CHARFORMAT2W cf = { 0 };
    static      CHARRANGE current_text;
    static uint32_t oldcpmax = 0;
    static uint8_t  cmd_flag = 0, caller = 0;

    int32_t  index;
    uint32_t last_char_posn = 0;
    static wchar_t   * retstring     = NULL,
                     * addAcroString = NULL,
                     * addDefnString = NULL;
    HMENU cmenu;
    POINT point_cmenu;
    static COORD carpos_cr = { 0 };
    COORD carpos_new       = { 0 }, tmp_carpos = { 0 };

    cf.cbSize = sizeof( CHARFORMAT2W );


    switch( msg )
    {
        case WM_CHAR:
        {
            switch( wParam )
            {
                case 0x08:
                    //backspace
                    break;
                case 0x09:
                    //tab
                    break;
                case 0x0A:
                    //line feed
                    break;
                case 0x1B:
                    //escape
                    break;
                case 0x0D:
                    //carriage return

                    tmp_carpos = carpos_cr;
                    carpos_cr = getCarPos( hwnd );

                    // Expecting the new caret position to always be old + 2
                    if( !(tmp_carpos.Y <= carpos_cr.Y - 2) )
                    {
                        index = GetWindowTextLengthW( hwnd );
                        SendMessageW( hwnd, EM_SETSEL, (WPARAM)index, (LPARAM)index );
                        return CallWindowProcW( OldMultiLineIO, hwnd, msg, wParam, lParam );
                    }

                    //process what the user has entered before doing a search
                    if( NULL != retstring )
                    {
                        free( retstring ); retstring = NULL;
                    }
                    retstring = getInput( hwnd );

                    //only search if there's no input error reported
                    if( 0 == caller &&
                        false == chkRawInput( hwnd, retstring ) )
                    {
                        if( 0 == wcscmp( retstring, L"." ))
                        {
                            SendMessageW( GetParent( hwnd ), WM_CLOSE, 0, 0 );
                        }
                        else if( L'<' == retstring[ 0 ] && L'<' == retstring[ 1 ] )
                        {
                            cmd_flag = 1;
                        }
                        else
                        {
                            if( false == chkInput( hwnd, retstring, cf, caller ) )
                            {
                                protectMe( hwnd, current_text, &oldcpmax, cf );
                                break;
                            }

                            lookUpAcro( hwnd, entry_list, retstring, NULL, 0, cf );
                        }
                    }
                    else if( 1 == caller &&
                             false == chkRawInput( hwnd, retstring ) &&
                              true == chkInput( hwnd, retstring, cf, caller ) )
                    {
                        addAcroString = wcsdup( retstring );

                        cf.crTextColor = FRGD_OTH;
                        SendMessageW( hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf );

                        SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                            (LPARAM)L"Enter Definition\n" );
                        protectMe( hwnd, current_text, &oldcpmax, cf );

                        caller = 2;

                        break;
                    }
                    else if( 2 == caller &&
                             true == chkInput( hwnd, retstring, cf, caller ) )
                    {
                        addDefnString = wcsdup( retstring );
                        protectMe( hwnd, current_text, &oldcpmax, cf );
                        refresh = cmdAddAcro( hwnd, entry_list, addAcroString, addDefnString );
                        protectMe( hwnd, current_text, &oldcpmax, cf );

                        caller = 0;

                        if( true == refresh )
                        {
                            reloadACR( hwnd );
                            refresh = false;
                        }
                        disable_commands = false;

                        break;
                    }
                    else
                    {
                        disable_commands = false;
                        caller = 0;
                    }

                    //apply overwrite protection to all text
                    protectMe( hwnd, current_text, &oldcpmax, cf );

                    break;

                default:
                {
                    carpos_new = getCarPos( hwnd );
                    index = GetWindowTextLengthW( hwnd );

                    if( carpos_new.Y <= carpos_cr.Y )
                    {
                        index = GetWindowTextLengthW( hwnd );
                        SendMessageW( hwnd, EM_SETSEL, (WPARAM)index, (LPARAM)index );
                    }

                    last_char_posn = lastCharIndex( hwnd, false );

#ifdef DEBUG
                    printf( "%d %ld\n", LOWORD( last_char_posn ),
                                             current_text.cpMax );
#endif // DEBUG
                    if( last_char_posn >= MAX_SCREEN_BUF )
                    {
                        MessageBoxW( NULL,
                        L"Maximum displayable characters have been exceeded (64,000)\n"
                        L"The screen will be cleared to continue.",
                        L"Warning",
                        MB_OK | MB_ICONEXCLAMATION );

                        //set the event mask so the ENM_PROTECTED mask is disabled
                        int mask = SendMessageW( hwnd, EM_GETEVENTMASK, 0, 0 );
                        SendMessageW( hwnd, EM_SETEVENTMASK, 0, (LPARAM)mask & ~ENM_PROTECTED );
                        //clear the screen
                        SetWindowTextW( hwnd, 0 );
                        oldcpmax = 0;
                        //re-enable protection
                        SendMessageW( hwnd, EM_SETEVENTMASK, 0, (LPARAM)ENM_PROTECTED );

                    }

                    cf.dwMask = CFM_COLOR;
                    cf.crTextColor = FRGD_GENERAL;
                    SendMessageW( hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf );

                    break;
                }

            }

            if( 1 == cmd_flag ) SendMessageW( hwnd, WM_CMDRECV, 0, 0 );

            break;
        }

        case WM_CMDRECV:

            switch( prcsCmd( hwnd, retstring, cf ) )
            {
                case 1: //add acro

                    if( true == disable_commands ) break;

                    caller   = 1;
                    cmd_flag = 0;
                    protectMe( hwnd, current_text, &oldcpmax, cf );
                    disable_commands = true;

                    break;

                case 2: // update

                    if( true == disable_commands ) break;

                    cmd_flag = 0;
                    SendMessageW( hwnd, WM_COMMAND, IDM_COM_UPD, 0 );

                    break;

                case 3: //resolve duplicates

                    if( true == disable_commands ) break;

                    reloadACR( hwnd );
                    cmd_flag = 0;
                    protectMe( hwnd, current_text, &oldcpmax, cf );

                    break;

                case 4: //no dupl to resolve

                    protectMe( hwnd, current_text, &oldcpmax, cf );
                    cmd_flag = 0;

                    break;

                case 5: //delete last entry

                    if( true == disable_commands ) break;

                    SendMessageW( hwnd, WM_COMMAND, IDM_COM_DEL, 0 );

                    break;

                default:

                    cmd_flag = 0;
                    protectMe( hwnd, current_text, &oldcpmax, cf );

                    break;
            }

            break;

        case WM_COMMAND:
        {
            int8_t updateresult;

            switch( LOWORD( wParam ) )
            {
            case IDC_CONTADD:

                index = GetWindowTextLengthW( hwnd );
                SendMessageW( hwnd, EM_SETSEL, (WPARAM)index, (LPARAM)index );

                if( true == disable_commands )
                {
                    disabledCmdMsg( hwnd );
                    break;
                }

                //disable_commands = true;
                caller = 1;
                prcsCmd( hwnd, L"<<A", cf );
                protectMe( hwnd, current_text, &oldcpmax, cf );
                disable_commands = true;

                break;

            case IDM_COM_DEL:

                if( true == disable_commands )
                {
                    disabledCmdMsg( hwnd );
                    break;
                }

                if( cmdDelAcro( hwnd, entry_list ))
                    reloadACR( hwnd );

                cmd_flag = 0;
                protectMe( hwnd, current_text, &oldcpmax, cf );
                SetFocus( hwnd );

                break;

            case IDM_COM_RES:

                if( true == disable_commands )
                {
                    disabledCmdMsg( hwnd );
                    break;
                }

                if( 3 == prcsCmd( hwnd, L"<<R", cf ) )
                {
                    reloadACR( hwnd );
                    protectMe( hwnd, current_text, &oldcpmax, cf );
                }
                else
                {
                    protectMe( hwnd, current_text, &oldcpmax, cf );
                }

                break;

            case IDM_COM_UPD:

                if( true == disable_commands )
                {
                    disabledCmdMsg( hwnd );
                    break;
                }

                //begin = clock();

                colourMe( hwnd, FRGD_INFO );
                SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                    (LPARAM)L"Checking for updates...\n" );
                UpdateWindow( hwnd );

                updateresult = initUpdate( hwnd );

                //end = clock();
                //printf( "%f seconds\n", (double)( end - begin ) / CLOCKS_PER_SEC );

                switch ( updateresult )
                {
                case 1:

                    colourMe( hwnd, FRGD_INFO );
                    SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                        (LPARAM)L"Successfully added!\n\n" );

                    reloadACR( hwnd );

                    break;

                case 0:

                    colourMe( hwnd, FRGD_INFO );
                    SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                        (LPARAM)L"There's nothing new to add!\n\n" );

                    break;

                case -1:

                    colourMe( hwnd, FRGD_ERROR );
                    SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                        (LPARAM)L"An error occurred, unable to access the network file\n\n" );

                    break;

                case -2:

                    colourMe( hwnd, FRGD_ERROR );
                    SendMessageW( hwnd, EM_REPLACESEL, (WPARAM)FALSE,
                        (LPARAM)L"An error occurred whilst comparing the local and network files\n\n" );

                    break;
                }
                protectMe( hwnd, current_text, &oldcpmax, cf );
            }

            break;
        }

        case WM_RBUTTONUP:

            point_cmenu.x = LOWORD( lParam );
            point_cmenu.y = HIWORD( lParam );

            cmenu = CreatePopupMenu();
            ClientToScreen( hwnd, &point_cmenu );

            AppendMenuW( cmenu, MF_STRING, IDC_CONTADD,   L"Add Acronym" );

            TrackPopupMenu( cmenu, TPM_RIGHTBUTTON,
                            point_cmenu.x, point_cmenu.y, 0, hwnd, NULL );

            DestroyMenu( cmenu );

            break;

        default:

            break;
    }

    return CallWindowProcW( OldMultiLineIO, hwnd, msg, wParam, lParam);
}
