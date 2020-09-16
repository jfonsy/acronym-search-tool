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

HFONT font_all = NULL;
bool LMODE;

static void addMenus( HWND hwnd, HINSTANCE hInstance )
{
    HMENU hMenubar, hMenuCmd, hMenuView, hMenuHelp;

    hMenubar  = CreateMenu();
    hMenuCmd  = CreateMenu();
    hMenuView = CreateMenu();
    hMenuHelp = CreateMenu();

    HBITMAP hbmpAdd  = LoadBitmapW( hInstance, MAKEINTRESOURCE(Add)  );
    HBITMAP hbmpDel  = LoadBitmapW( hInstance, MAKEINTRESOURCE(Del)  );
    HBITMAP hbmpUpd  = LoadBitmapW( hInstance, MAKEINTRESOURCE(Upd)  );
    HBITMAP hbmpRes  = LoadBitmapW( hInstance, MAKEINTRESOURCE(Res)  );
    HBITMAP hbmpExtr = LoadBitmapW( hInstance, MAKEINTRESOURCE(Extr) );
    HBITMAP hbmpExit = LoadBitmapW( hInstance, MAKEINTRESOURCE(Exit) );
    HBITMAP hbmpHlp  = LoadBitmapW( hInstance, MAKEINTRESOURCE(Hlp)  );
    HBITMAP hbmpLm   = LoadBitmapW( hInstance, MAKEINTRESOURCE(Lm)   );
    HBITMAP hbmpDm   = LoadBitmapW( hInstance, MAKEINTRESOURCE(Dm)   );

    AppendMenuW( hMenuCmd,  MF_STRING,    IDM_COM_ADD,         L"&Add"                );
    AppendMenuW( hMenuCmd,  MF_STRING,    IDM_COM_DEL,         L"&Delete"             );
    AppendMenuW( hMenuCmd,  MF_STRING,    IDM_COM_UPD,         L"&Update"             );
    AppendMenuW( hMenuCmd,  MF_STRING,    IDM_COM_RES,         L"&Resolve Duplicates" );
    AppendMenuW( hMenuCmd,  MF_STRING,    IDM_COM_EXTR,        L"&Extract From File"  );
    AppendMenuW( hMenuCmd,  MF_SEPARATOR,            0,         NULL                  );
    AppendMenuW( hMenuCmd,  MF_STRING,    IDM_COM_EXIT,        L"&Exit"               );

    AppendMenuW( hMenuView, MF_STRING,    IDM_VW_LM,           L"&Light Mode"         );
    AppendMenuW( hMenuView, MF_STRING,    IDM_VW_DM,           L"&Dark Mode"          );

    AppendMenuW( hMenuHelp, MF_STRING,    IDM_HLP_HLP,         L"&Help"               );

    AppendMenuW( hMenubar,  MF_POPUP,     (UINT_PTR)hMenuCmd,  L"&Command"            );
    AppendMenuW( hMenubar,  MF_POPUP,     (UINT_PTR)hMenuView, L"&View"               );
    AppendMenuW( hMenubar,  MF_POPUP,     (UINT_PTR)hMenuHelp, L"&Help"               );

    SetMenuItemBitmaps( hMenuCmd,  0, MF_BYPOSITION, hbmpAdd,  hbmpAdd  );
    SetMenuItemBitmaps( hMenuCmd,  1, MF_BYPOSITION, hbmpDel,  hbmpDel  );
    SetMenuItemBitmaps( hMenuCmd,  2, MF_BYPOSITION, hbmpUpd,  hbmpUpd  );
    SetMenuItemBitmaps( hMenuCmd,  3, MF_BYPOSITION, hbmpRes,  hbmpRes  );
    SetMenuItemBitmaps( hMenuCmd,  4, MF_BYPOSITION, hbmpExtr, hbmpExtr );
    SetMenuItemBitmaps( hMenuCmd,  6, MF_BYPOSITION, hbmpExit, hbmpExit );
    SetMenuItemBitmaps( hMenuView, 0, MF_BYPOSITION, hbmpLm,   hbmpLm   );
    SetMenuItemBitmaps( hMenuView, 1, MF_BYPOSITION, hbmpDm,   hbmpDm   );
    SetMenuItemBitmaps( hMenuHelp, 0, MF_BYPOSITION, hbmpHlp,  hbmpHlp  );

    SetMenu( hwnd, hMenubar );
}

static FILE * retFileHandlePREF( wchar_t * option )
{
    _WDIR * dptr;
    FILE  * ini_file;

    dptr = _wopendir( L"." );
    if ( dptr == NULL ) throwError( DIR_GEN_PERM, 0 );

    ini_file = _wfopen( L"pref.ini", option );

    if( NULL != dptr )     _wclosedir( dptr );
    if( NULL != ini_file ) return ( ini_file );

    return NULL;
}

static void setMode()
{
    FILE * fptr_pref;
    char * mode = NULL;
    size_t linelen = 0;
    LMODE = false;

    if( NULL != ( fptr_pref = retFileHandlePREF( L"r" ) ) )
        if( -1 != getline( &mode, &linelen, fptr_pref ) )
            if( 0 == strcmp( mode, "LMODE = TRUE" ) )
            {
                LMODE = true;
                /* LIGHT MODE */
                TRANS_GLOBAL     = 230;
                BKGD_GLOBAL      = RGB( 255, 255, 255 );
                FRGD_ACROLOADED  = RGB( 0, 0, 0 );
                FRGD_GENERAL     = RGB( 0, 0, 0 );
                FRGD_OTH         = RGB( 75, 75, 75 );
                FRGD_ERROR       = RGB( 255, 25, 30 );
                FRGD_DEFNTRUE    = RGB( 0, 0, 153 );
                FRGD_INFO        = RGB( 102, 0, 102 );
            }

    if( !LMODE )
    {
        /* DARK MODE */
        TRANS_GLOBAL     = 220;
        BKGD_GLOBAL      = RGB( 0, 0, 0 );
        FRGD_ACROLOADED  = RGB( 1, 175, 235 );
        FRGD_GENERAL     = RGB( 242, 242, 242 );
        FRGD_OTH         = RGB( 1, 150, 235 );
        FRGD_ERROR       = RGB( 255, 50, 60 );
        FRGD_DEFNTRUE    = RGB( 138, 235, 0 );
        FRGD_INFO        = RGB( 255, 215, 0 );
    }

    free( mode ); mode = NULL;
    fclose( fptr_pref );
}

extern void initialise()
{
    FILE      * fptr = NULL;
    entry_list = loadToMemory( fptr );
}

HWND wMultiLine( HWND hwnd, HFONT font_all )
{
    // Check in C:\Windows\System32 or \SysWow64
    // It should be a Rich Edit v3.1 control
    LoadLibrary( TEXT( "Riched20.dll" ) );
    HWND multiline_io = CreateWindowExW(
                            WS_EX_CLIENTEDGE, RICHEDIT_CLASSW, 0,
                            WS_CHILD | WS_VISIBLE   | WS_VSCROLL |
                            ES_LEFT  | ES_MULTILINE | ES_AUTOVSCROLL,
                            0, 0, 0, 0, hwnd,     //set in WM_SIZE
                            (HMENU)IDC_MULTILINEIO,
                            (HINSTANCE)GetWindowLongW( hwnd, GWL_HINSTANCE ),
                            NULL );

    SendMessageW( multiline_io, EM_SETBKGNDCOLOR, 0, BKGD_GLOBAL );
    SendMessageW( multiline_io, WM_SETFONT, (WPARAM)font_all, 0 );

    return multiline_io;
}

HWND wAcroLoaded( HWND hwnd, RECT hwnd_rect,
                  POINT point_diff_acrloaded,
                  HFONT font_all, LPARAM lParam )
{
    CREATESTRUCT * ptr_entry_list = (CREATESTRUCT *)lParam;
    entry_list     = (AcronymDB *)ptr_entry_list -> lpCreateParams;
    SetWindowLongPtrW( hwnd, GWLP_USERDATA, (LONG_PTR)entry_list );
    // As per MSDN to retrieve user specific data
    // store data in a struct, and then set it with GWLP_USERDATA flag
    // and then retrieve later with GetWindowLongPtr
    LONG_PTR ret_entry_list = GetWindowLongPtrW( hwnd, GWLP_USERDATA );
    entry_list = (AcronymDB *)ret_entry_list;

    wchar_t * acronyms_loaded;
    acronyms_loaded = acrLoadedText( entry_list );

    // place child window at upper right region of parent window
    HWND txt_acrloaded = CreateWindowW(
                        L"STATIC", acronyms_loaded,
                        WS_CHILD | WS_VISIBLE,
                        (hwnd_rect.right - hwnd_rect.left) - point_diff_acrloaded.x,
                        0,
                        point_diff_acrloaded.x,
                        point_diff_acrloaded.y,
                        hwnd, NULL, NULL, NULL );

    SendMessageW( txt_acrloaded, WM_SETFONT, (WPARAM)font_all, TRUE );

    free( acronyms_loaded ); acronyms_loaded = NULL;

    return txt_acrloaded;
}

HBITMAP wLogo( HWND hwnd, RECT hwnd_rect, POINT point_diff_acrloaded )
{
    HBITMAP           logo_hBitmap;
    BITMAP            bitmap_info;
    _WDIR           * dptr;
    struct _wdirent * directory;
    wchar_t         * filename = NULL;
    wchar_t           buffer[ MAX_PATH ] = L"";
    wchar_t         ** lppPart = { NULL };

    CreateWindowW( 0, L"Draw Bitmap",
                   WS_CHILD | WS_VISIBLE,
                   ( hwnd_rect.right - hwnd_rect.left ) - point_diff_acrloaded.x, 0,
                   point_diff_acrloaded.x, point_diff_acrloaded.y,
                   hwnd, NULL, NULL, NULL );

    dptr = _wopendir( L"." ); //parent directory of exe
    if ( NULL != dptr )
    {
        while( NULL != ( directory = _wreaddir( dptr ) ) )
        {
            if( wcsstr( directory->d_name, L"LogoL" ) && true == LMODE )
            {
                GetFullPathNameW( directory->d_name, MAX_PATH, buffer, lppPart );
                filename = (wchar_t *) malloc(( wcslen( buffer ) + 1 ) * ( sizeof( wchar_t )));
                wmemcpy( filename, buffer, wcslen( buffer ) + 1 );

                break;
            }
            else if( wcsstr( directory->d_name, L"Logo" ) && false == LMODE )
            {
                GetFullPathNameW( directory->d_name, MAX_PATH, buffer, lppPart );
                filename = (wchar_t *) malloc(( wcslen( buffer ) + 1 ) * ( sizeof( wchar_t )));
                wmemcpy( filename, buffer, wcslen( buffer ) + 1 );

                break;
            }
        }
    }
    logo_hBitmap = (HBITMAP)LoadImageW( NULL, filename,
                                        IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE );

    GetObject( logo_hBitmap, sizeof( BITMAP ), &bitmap_info );

    if( bitmap_info.bmWidth  > LOGOWMAX ||
        bitmap_info.bmHeight > LOGOHMAX )
        {
            //probably not the correct bitmap!
            logo_hBitmap = NULL;
        }

    if( NULL == logo_hBitmap )
        MessageBoxW( hwnd, L"Logo file missing or corrupted", L"Warning",
                    MB_OK | MB_ICONEXCLAMATION );

    free( filename ); filename = NULL;
    if( NULL != dptr ) _wclosedir( dptr );

    return logo_hBitmap;
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR pCmdLine, int nCmdShow )
{
    // suppress unused variable warnings
    (void)hPrevInstance;
    (void)pCmdLine;

    MSG  msg;
    HWND hwnd;
    WNDCLASSW wc;

    /* SET TO EITHER DARK OR LIGHT MODE BASED ON PREF.INI */
    setMode();

    HBRUSH parent_hBrush = CreateSolidBrush( BKGD_GLOBAL );
    initialise();

    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.lpszClassName = L"Window";
    wc.hInstance     = hInstance;
    wc.hbrBackground = parent_hBrush;
    wc.lpszMenuName  = NULL;
    wc.lpfnWndProc   = WndProc;
    wc.hCursor       = LoadCursorW( NULL, IDC_ARROW );
    wc.hIcon         = LoadIconW( NULL, IDI_APPLICATION );

    RegisterClassW( &wc );
    hwnd = CreateWindowExW(

              WS_EX_LAYERED,
              wc.lpszClassName,
              PROG_VER,

              WS_OVERLAPPEDWINDOW | ES_AUTOVSCROLL | WS_VISIBLE,
              STARTPOS_HORZ, STARTPOS_VERT, WMAIN_WIDTH, WMAIN_HEIGHT,
              NULL, NULL, hInstance,

              entry_list ); //Additional application data

    SetLayeredWindowAttributes( hwnd, 0, TRANS_GLOBAL, LWA_ALPHA );

    ShowWindow( hwnd, nCmdShow );
    UpdateWindow( hwnd );

    while ( GetMessageW( &msg, NULL, 0, 0 ) )
    {
        TranslateMessage( &msg );
        DispatchMessageW( &msg );
    }

    DeleteObject( parent_hBrush );

    return (int) msg.wParam;
}

LRESULT CALLBACK WndProc( HWND hwnd, UINT msg,
                          WPARAM wParam, LPARAM lParam )
{
    FILE * fptr_pref;
    static HBITMAP logo_hBitmap;
    static HBRUSH txt_hBrush = NULL;
    static HWND txt_acrloaded, multiline_io;
    static RECT hwnd_rect;
    static POINT point_diff_acrloaded;
    static CHARFORMAT2W cf = { 0 };
    int32_t index;

    point_diff_acrloaded.x = 230;
    point_diff_acrloaded.y = 25;

    // a font that supports as many unicode characters as possible is best
    if ( NULL == font_all ) font_all =

        CreateFontW( 24, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Arial Unicode MS Regular" );

    // don't use GetWindowRect, we need the client area, which is different
    GetClientRect( hwnd, &hwnd_rect );

    switch( msg )
    {
        case WM_CREATE:
        {
             cf.cbSize = sizeof( CHARFORMAT2W );
             cf.dwMask = CFM_COLOR;
             cf.crTextColor = FRGD_GENERAL;

            /* SET APP ICON */

             HINSTANCE hInstance = ( ( LPCREATESTRUCT )lParam ) -> hInstance;
             HICON hIcon = LoadIcon ( hInstance, MAKEINTRESOURCE ( AppIcon ) );
             SendMessageW( hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon );

            /* ADD MENU OPTIONS */

            addMenus( hwnd, hInstance );

            /* # of ACRONYMS LOADED WINDOW */

            txt_acrloaded = wAcroLoaded( hwnd, hwnd_rect,
                                         point_diff_acrloaded,
                                         font_all, lParam );

            /* COMPANY LOGO WINDOW */

            logo_hBitmap = wLogo( hwnd, hwnd_rect,
                                  point_diff_acrloaded );


            /* MULTILINE USER I/O WINDOW */

            multiline_io = wMultiLine( hwnd, font_all );
            SendMessageW( multiline_io, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf );
            SendMessageW( multiline_io, EM_SETLIMITTEXT, MAX_SCREEN_BUF, 0 );
            //max of 64,000 characters available in the screen

            TEXTMETRIC tm;
            HDC hdc = GetDC( multiline_io );

            SelectObject( hdc, (HGDIOBJ)font_all );
            GetTextMetrics( hdc, &tm );
            ReleaseDC( multiline_io, hdc );

            SetFocus( multiline_io );

            OldMultiLineIO = (WNDPROC) SetWindowLongW( multiline_io,
                                            GWL_WNDPROC, (LONG) MultiLineProc );

            //as per MSDN to receive EN_PROTECTED notifications, set the mask
            SendMessageW( multiline_io, EM_SETEVENTMASK, 0, (LPARAM)ENM_PROTECTED );

            break;
        }

        case WM_COMMAND:

            switch( LOWORD( wParam ) )
            {
            case IDM_COM_ADD:

                if( disable_commands ) { disabledCmdMsg2(); break; }

                // LINK 5
                index = GetWindowTextLengthW( multiline_io );
                SendMessageW( multiline_io, EM_SETSEL, (WPARAM)index, (LPARAM)index );

                SendMessageW( multiline_io, WM_COMMAND, IDC_CONTADD, 0 );

                break;

            case IDM_COM_DEL:

                if( disable_commands ) { disabledCmdMsg2(); break; }

                index = GetWindowTextLengthW( multiline_io );
                SendMessageW( multiline_io, EM_SETSEL, (WPARAM)index, (LPARAM)index );

                SendMessageW( multiline_io, WM_COMMAND, IDM_COM_DEL, 0 );

                break;

            case IDM_VW_DM:

                if( NULL != ( fptr_pref = retFileHandlePREF( L"wb" ) ) )
                {
                    fprintf( fptr_pref, "LMODE = FALSE" );
                    fclose( fptr_pref );
                }

                    MessageBoxW( NULL,
                    L"Preferences updated.\nRestart for changes to take effect.",
                    L"Information",
                    MB_OK | MB_ICONINFORMATION );

                break;

            case IDM_VW_LM:

                if( NULL != ( fptr_pref = retFileHandlePREF( L"wb" ) ) )
                {
                    fprintf( fptr_pref, "LMODE = TRUE" );
                    fclose( fptr_pref );
                }

                    MessageBoxW( NULL,
                    L"Preferences updated.\nRestart for changes to take effect.",
                    L"Information",
                    MB_OK | MB_ICONINFORMATION );

                break;

            case IDM_COM_EXIT:

                SendMessageW( hwnd, WM_CLOSE, 0, 0 );

                break;

            case IDM_COM_UPD:

                if( disable_commands ) { disabledCmdMsg2(); break; }

                index = GetWindowTextLengthW( multiline_io );
                SendMessageW( multiline_io, EM_SETSEL, (WPARAM)index, (LPARAM)index );

                SendMessageW( multiline_io, WM_COMMAND, IDM_COM_UPD, 0 );

                break;

            case IDM_COM_RES:

                if( disable_commands ) { disabledCmdMsg2(); break; }

                index = GetWindowTextLengthW( multiline_io );
                SendMessageW( multiline_io, EM_SETSEL, (WPARAM)index, (LPARAM)index );

                SendMessageW( multiline_io, WM_COMMAND, IDM_COM_RES, 0 );

                break;

            case IDM_HLP_HLP:

                ShellExecuteW( NULL, 0, L"User Manual.chm", 0, 0, SW_SHOWNORMAL );

                break;

            case IDM_COM_EXTR:

                if( disable_commands ) { disabledCmdMsg2(); break; }

                disable_commands = true;
                loadFileToExtract( hwnd );

                break;
            }
            break;

        case WM_SIZE: //resize the child window based on an update of WM_SIZE
        {
            //move child window as parent is resized
            MoveWindow( txt_acrloaded,
                       ( hwnd_rect.right - hwnd_rect.left ) - point_diff_acrloaded.x,
                       0,
                       point_diff_acrloaded.x,
                       point_diff_acrloaded.y,
                       TRUE );

            MoveWindow( multiline_io, 0, 70,
                        LOWORD(lParam),
                        HIWORD(lParam) - 70, TRUE );

            SetFocus( multiline_io );

            break;
        }

        case WM_CTLCOLORSTATIC:
        {
            HDC hdc_acroload = (HDC)wParam;
            SetTextColor( hdc_acroload, FRGD_ACROLOADED );
            SetBkColor  ( hdc_acroload, BKGD_GLOBAL );

            if( NULL == txt_hBrush ) txt_hBrush = CreateSolidBrush( BKGD_GLOBAL );

            return (INT_PTR)txt_hBrush;
        }

        case WM_PAINT:
        {
            BITMAP bitmap;
            HGDIOBJ old_bitmap;
            HDC logo_hdc, logo_hdcmem;
            PAINTSTRUCT ps;

            logo_hdc    = BeginPaint( hwnd, &ps );
            logo_hdcmem = CreateCompatibleDC( logo_hdc );

            old_bitmap = SelectObject( logo_hdcmem, logo_hBitmap );

            GetObject( logo_hBitmap, sizeof( bitmap ), &bitmap );
            BitBlt( logo_hdc, 5, 5, bitmap.bmWidth, bitmap.bmHeight,
                 logo_hdcmem, 0, 0, SRCCOPY );

            SelectObject( logo_hdcmem, old_bitmap );
            DeleteDC( logo_hdcmem );

            EndPaint( hwnd, &ps );
            DeleteDC( logo_hdc );
            DeleteObject( old_bitmap );

            SetFocus( multiline_io );

            break;
        }

        case WM_NOTIFY:

            if ( EN_PROTECTED == ( (LPNMHDR)lParam )->code )
            {
#ifdef DEBUG
                printf( "@ EN_PROTECTED CASE\n" );
#endif // DEBUG
                return 1;
            }
            break;

        case WM_DESTROY:

            PostQuitMessage( 0 );

            break;

        default:

            return DefWindowProcW( hwnd, msg, wParam, lParam );
    }

    return 0;
}
