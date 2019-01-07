#include "QSearch.h"
#include "QSearchDlg.h"
#include "QSearchLng.h"


#define  QSEARCH_FIRST       0x0001
#define  QSEARCH_NEXT        0x0002
#define  QSEARCH_SEL         0x0100
#define  QSEARCH_SEL_FINDUP  0x0200
#define  QSEARCH_NOFINDUP    0x1000
#define  QSEARCH_NOFINDBEGIN 0x2000
#define  QSEARCH_FINDUP      0x4000
#define  QSEARCH_FINDBEGIN   0x8000

#define  QSEARCH_EOF_DOWN    0x0001
#define  QSEARCH_EOF_UP      0x0002

#define  VK_QS_FINDBEGIN     VK_MENU     // Alt
#define  VK_QS_FINDUP        VK_SHIFT    // Shift
#define  VK_QS_PICKUPTEXT    VK_CONTROL  // Ctrl

/* >>>>>>>>>>>>>>>>>>>>>>>> highlight plugin >>>>>>>>>>>>>>>>>>>>>>>> */
#define DLLA_HIGHLIGHT_MARK                2
#define DLLA_HIGHLIGHT_UNMARK              3
#define DLLA_HIGHLIGHT_FINDMARK            4

#define MARKFLAG_MATCHCASE 0x01
#define MARKFLAG_REGEXP    0x02
#define MARKFLAG_WHOLEWORD 0x04

typedef struct sDLLECHIGHLIGHT_MARK {
    UINT_PTR dwStructSize;
    INT_PTR nAction;
    unsigned char *pColorText;
    unsigned char *pColorBk;
    UINT_PTR dwMarkFlags;
    UINT_PTR dwFontStyle;
    UINT_PTR dwMarkID;
    wchar_t *wszMarkText;
} DLLECHIGHLIGHT_MARK;

typedef struct sDLLECHIGHLIGHT_UNMARK {
    UINT_PTR dwStructSize;
    INT_PTR nAction;
    UINT_PTR dwMarkID;
} DLLECHIGHLIGHT_UNMARK;

const char*    cszHighlightMainA = "Coder::HighLight";
const wchar_t* cszHighlightMainW = L"Coder::HighLight";
/* <<<<<<<<<<<<<<<<<<<<<<<< highlight plugin <<<<<<<<<<<<<<<<<<<<<<<< */


/* >>>>>>>>>>>>>>>>>>>>>>>> qsearchdlg state >>>>>>>>>>>>>>>>>>>>>>>> */
    void initializeQSearchDlgState(QSearchDlgState* pQSearchDlg)
    {
        pQSearchDlg->hDlg = NULL;
        pQSearchDlg->hFindEdit = NULL;
        pQSearchDlg->hBtnFindNext = NULL;
        pQSearchDlg->hBtnFindPrev = NULL;
        pQSearchDlg->bMatchCase = FALSE;
        //pQSearchDlg->bOnDlgStart = FALSE;
        pQSearchDlg->bQSearching = FALSE;
        pQSearchDlg->bIsQSearchingRightNow = FALSE;
        pQSearchDlg->bMouseJustLeavedFindEdit = FALSE;
        pQSearchDlg->pDockData = NULL;
        pQSearchDlg->szFindTextW[0] = 0;
        pQSearchDlg->uSearchOrigin = QS_SO_UNKNOWN;
        pQSearchDlg->uWmShowFlags = 0;
    }
/* <<<<<<<<<<<<<<<<<<<<<<<< qsearchdlg state <<<<<<<<<<<<<<<<<<<<<<<< */


// extern vars
extern PluginState     g_Plugin;
extern QSearchDlgState g_QSearchDlg;
extern QSearchOpt      g_Options;
extern wchar_t         g_szFunctionQSearchW[128];
extern BOOL            g_bHighlightPlugin;


// static (local) vars
static WNDPROC prev_editWndProc = NULL;
static WNDPROC prev_btnFindNextWndProc = NULL;
static WNDPROC prev_btnFindPrevWndProc = NULL;
/*static WNDPROC prev_chWndProc = NULL;*/
static BOOL    qs_bEditIsActive = TRUE;
static BOOL    qs_bEditCanBeNonActive = TRUE;
static BOOL    qs_bEditSelJustChanged = FALSE;
static BOOL    qs_bEditNotFound = FALSE;
static BOOL    qs_bEditNotRegExp = FALSE;
static BOOL    qs_bEditIsEOF = FALSE;
static int     qs_nEditEOF = 0;
static BOOL    qs_bEditTextChanged = TRUE;
static BOOL    qs_bHotKeyPressedOnShow = FALSE;
static BOOL    qs_bForceFindFirst = FALSE;
static BOOL    qs_bBtnFindIsFocused = FALSE;
static DWORD   qs_dwHotKey = 0;


// funcs
HWND qsearchDoInitToolTip(HWND hDlg, HWND hEdit);
void qsearchDoQuit(HWND hEdit, HWND hToolTip, HMENU hPopupMenuLoaded, HBRUSH hBrush1, HBRUSH hBrush2, HBRUSH hBrush3);
void qsearchDoSearchText(HWND hEdit, DWORD dwParams);
void qsearchDoSelFind(HWND hEdit, BOOL bFindPrev);
void qsearchDoSetNotFound(HWND hEdit, BOOL bNotFound, BOOL bNotRegExp, BOOL bEOF);
void qsearchDoShowHide(HWND hDlg, BOOL bShow, UINT uShowFlags);
void qsearchDoTryHighlightAll(HWND hDlg);
void qsearchDoTryUnhighlightAll(void);
HWND qsearchGetFindEdit(HWND hDlg);

#define UFHF_MOVE_TO_TOP_IF_EXISTS 0x0001
#define UFHF_KEEP_EDIT_TEXT        0x0002
BOOL qsearchFindHistoryAdd(HWND hEdit, const wchar_t* szTextAW, UINT uUpdFlags);

BOOL qsearchIsFindHistoryEnabled(void)
{
    return (g_Options.dwFindHistoryItems > 1);
}

BOOL qsearchIsFindHistoryBeingSaved(void)
{
    return ((g_Options.dwHistorySave & 0x01) != 0);
}

BOOL qsearchIsSearchFlagsBeingSaved(void)
{
    return ((g_Options.dwHistorySave & 0x02) != 0);
}

BOOL qsearchIsSavingHistoryToStdLocation(void)
{
    return ((g_Options.dwHistorySave & 0x04) != 0);
}

/*static void editSetTrailSel(HWND hEdit)
{
    if ( g_Plugin.bOldWindows )
    {
        int len = GetWindowTextLengthA(hEdit);
        SendMessageA(hEdit, EM_SETSEL, len, len);
    }
    else
    {
        int len = GetWindowTextLengthW(hEdit);
        SendMessageW(hEdit, EM_SETSEL, len, len);
    }
}*/

static void qsdlgShowHideWholeWordCheckBox(HWND hDlg)
{
    HWND    hChWholeWord;
    wchar_t wszText[64];

    hChWholeWord = GetDlgItem(hDlg, IDC_CH_WHOLEWORD);

    if ( g_Plugin.bOldWindows )
    {
        lstrcpyA( (char *) wszText, qsearchGetHintA(IDC_CH_WHOLEWORD) );
        if ( g_Options.dwFlags[OPTF_SRCH_USE_SPECIALCHARS] )
            lstrcatA( (char *) wszText, "*" );
        else if ( g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] )
            lstrcatA( (char *) wszText, "^" );
        SetWindowTextA(hChWholeWord, (const char *) wszText);
    }
    else
    {
        lstrcpyW( wszText, qsearchGetHintW(IDC_CH_WHOLEWORD) );
        if ( g_Options.dwFlags[OPTF_SRCH_USE_SPECIALCHARS] )
            lstrcatW( wszText, L"*" );
        else if ( g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] )
            lstrcatW( wszText, L"^" );
        SetWindowTextW(hChWholeWord, wszText);
    }

    /*static int nChHighlightAllLeft0 = 0;
    HWND hChWholeWord;
    HWND hChHighlightAll;
    RECT rectDlg;
    RECT rectChWholeWord;
    RECT rectChHighlightAll;

    hChWholeWord = GetDlgItem(hDlg, IDC_CH_WHOLEWORD);
    hChHighlightAll = GetDlgItem(hDlg, IDC_CH_HIGHLIGHTALL);
    GetWindowRect(hDlg, &rectDlg);
    GetWindowRect(hChWholeWord, &rectChWholeWord);
    GetWindowRect(hChHighlightAll, &rectChHighlightAll);

    if ( nChHighlightAllLeft0 == 0 )
        nChHighlightAllLeft0 = rectChHighlightAll.left - rectDlg.left;

    ShowWindow( 
        hChWholeWord,
        g_Options.dwFlags[OPTF_SRCH_USE_SPECIALCHARS] ? SW_HIDE : SW_SHOWNORMAL
    );

    MoveWindow(
        hChHighlightAll,
        g_Options.dwFlags[OPTF_SRCH_USE_SPECIALCHARS] ? 
          (rectChWholeWord.left - rectDlg.left) : nChHighlightAllLeft0,
        rectChHighlightAll.top - rectDlg.top,
        rectChHighlightAll.right - rectChHighlightAll.left,
        rectChHighlightAll.bottom - rectChHighlightAll.top,
        TRUE
    );*/
}

static DWORD getQSearchHotKey(void)
{
    if ( g_szFunctionQSearchW[0] )
    {
        if ( g_Plugin.bOldWindows )
        {
            PLUGINFUNCTION* pfA;

            pfA = (PLUGINFUNCTION *) SendMessage( g_Plugin.hMainWnd, 
              AKD_DLLFINDA, (WPARAM) g_szFunctionQSearchW, 0 );
            if ( pfA )
            {
                return pfA->wHotkey;
            }
        }
        else
        {
            PLUGINFUNCTION* pfW;

            pfW = (PLUGINFUNCTION *) SendMessage( g_Plugin.hMainWnd, 
              AKD_DLLFINDW, (WPARAM) g_szFunctionQSearchW, 0 );
            if ( pfW )
            {
                return pfW->wHotkey;
            }
        }
    }
    return 0;
}

static BOOL isQSearchHotKeyPressed(void)
{
    if ( qs_dwHotKey )
    {
        if ( GetKeyState(LOBYTE(qs_dwHotKey)) & 0x80 )
        {
            if ( GetKeyState(VK_MENU) & 0x80 )
            {
                if ( (HIBYTE(qs_dwHotKey) & HOTKEYF_ALT) != HOTKEYF_ALT )
                    return FALSE;
            }
            else
            {
                if ( HIBYTE(qs_dwHotKey) & HOTKEYF_ALT )
                    return FALSE;
            }

            if ( GetKeyState(VK_CONTROL) & 0x80 )
            {
                if ( (HIBYTE(qs_dwHotKey) & HOTKEYF_CONTROL) != HOTKEYF_CONTROL )
                    return FALSE;
            }
            else
            {
                if ( HIBYTE(qs_dwHotKey) & HOTKEYF_CONTROL )
                    return FALSE;
            }

            if ( GetKeyState(VK_SHIFT) & 0x80 )
            {
                if ( (HIBYTE(qs_dwHotKey) & HOTKEYF_SHIFT) != HOTKEYF_SHIFT )
                    return FALSE;
            }
            else
            {
                if ( HIBYTE(qs_dwHotKey) & HOTKEYF_SHIFT )
                    return FALSE;
            }

            return TRUE;
        }
    }
    return FALSE;
}

static void getEditFindText(HWND hEdit, wchar_t szTextAW[MAX_TEXT_SIZE])
{
    szTextAW[0] = 0;
    if ( g_Plugin.bOldWindows )
        GetWindowTextA( hEdit, (LPSTR) szTextAW, MAX_TEXT_SIZE - 1 );
    else
        GetWindowTextW( hEdit, (LPWSTR) szTextAW, MAX_TEXT_SIZE - 1 );
}

static void setEditFindText(HWND hEdit, const wchar_t* pszTextAW)
{
    if ( g_Plugin.bOldWindows )
    {
        SetWindowTextA( hEdit, (LPCSTR) pszTextAW );
    }
    else
    {
        SetWindowTextW( hEdit, (LPCWSTR) pszTextAW );
    }
}

static void cutEditText(HWND hEdit, BOOL bCutAfterCaret)
{
    DWORD   dwSelPos1 = 0;
    DWORD   dwSelPos2 = 0;
    UINT    len = 0;

    SendMessage( hEdit, EM_GETSEL, (WPARAM) &dwSelPos1, (LPARAM) &dwSelPos2 );

    if ( g_Plugin.bOldWindows )
        len = (UINT) GetWindowTextLengthA(hEdit);
    else
        len = (UINT) GetWindowTextLengthW(hEdit);

    if ( bCutAfterCaret )
    {
        if ( dwSelPos1 < len )
        {
            /*if ( dwSelPos1 < 0 )
                dwSelPos1 = 0;*/

            //SendMessage( hEdit, WM_SETREDRAW, FALSE, 0 );
            SendMessage( hEdit, EM_SETSEL, dwSelPos1, -1 );
            //SendMessage( hEdit, WM_SETREDRAW, TRUE, 0 );

            if ( g_Plugin.bOldWindows )
            {
                SendMessageA( hEdit, EM_REPLACESEL, TRUE, (LPARAM) "" );
            }
            else
            {
                SendMessageW( hEdit, EM_REPLACESEL, TRUE, (LPARAM) L"" );
            }

            SendMessage( hEdit, EM_SETSEL, dwSelPos1, dwSelPos1 );
        }
    }
    else
    {
        if ( dwSelPos2 > 0 )
        {
            if ( dwSelPos2 > len )
                dwSelPos2 = len;

            //SendMessage( hEdit, WM_SETREDRAW, FALSE, 0 );
            SendMessage( hEdit, EM_SETSEL, 0, dwSelPos2 );
            //SendMessage( hEdit, WM_SETREDRAW, TRUE, 0 );

            if ( g_Plugin.bOldWindows )
            {
                SendMessageA( hEdit, EM_REPLACESEL, TRUE, (LPARAM) "" );
            }
            else
            {
                SendMessageW( hEdit, EM_REPLACESEL, TRUE, (LPARAM) L"" );
            }

            SendMessage( hEdit, EM_SETSEL, 0, 0 );
        }
    }
}

static BOOL getAkelPadSelectedText(wchar_t szTextAW[MAX_TEXT_SIZE])
{
    EDITINFO  ei;

    ei.hWndEdit = NULL;
    SendMessage( g_Plugin.hMainWnd, 
      AKD_GETEDITINFO, (WPARAM) NULL, (LPARAM) &ei );
    if ( ei.hWndEdit )
    {
        CHARRANGE_X cr = { 0, 0 };

        SendMessage( ei.hWndEdit, EM_EXGETSEL_X, 0, (LPARAM) &cr );
        if ( cr.cpMin != cr.cpMax )
        {
            if ( cr.cpMax >= cr.cpMin + MAX_TEXT_SIZE )
                cr.cpMax = cr.cpMin + MAX_TEXT_SIZE - 1;

            if ( g_Plugin.bOldWindows )
            {
                TEXTRANGEA_X trA;

                trA.chrg.cpMin = cr.cpMin;
                trA.chrg.cpMax = cr.cpMax;
                trA.lpstrText = (LPSTR) szTextAW;
                SendMessageA( ei.hWndEdit, EM_GETTEXTRANGE_X, 0, (LPARAM) &trA );
                if ( g_Options.dwFlags[OPTF_SRCH_USE_SPECIALCHARS] ||
                     g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] )
                {
                    int  i;
                    int  k;
                    char szTextA[2*MAX_TEXT_SIZE];

                    i = 0;
                    k = 0;
                    while ( trA.lpstrText[i] )
                    {
                        switch ( trA.lpstrText[i] )
                        {
                            case '\t':
                                szTextA[k++] = '\\';
                                szTextA[k++] = 't';
                                break;
                            case '\r':
                                szTextA[k++] = '\\';
                                szTextA[k++] = 'n';
                                break;
                            case '\\':
                            case '*':
                            case '?':
                                szTextA[k++] = '\\';
                            default:
                                if ( g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] )
                                {
                                    switch ( trA.lpstrText[i] )
                                    {
                                        case '(':
                                        case ')':
                                        case '[':
                                        case ']':
                                        case '{':
                                        case '}':
                                        case '.':
                                        case '!':
                                        case '|':
                                        case '+':
                                        case '^':
                                        case '$':
                                            szTextA[k++] = '\\';
                                            break;
                                    }
                                }
                                szTextA[k++] = trA.lpstrText[i];
                                break;
                        }
                        ++i;
                    }
                    if ( k > i )
                    {
                        if ( k > MAX_TEXT_SIZE - 1 )
                        {
                            k = MAX_TEXT_SIZE - 1;
                        }
                        szTextA[k] = 0;
                        i = 0;
                        while ( (trA.lpstrText[i] = szTextA[i]) != 0 )
                        {
                            ++i;
                        }
                    }
                }
            }
            else
            {
                TEXTRANGEW_X trW;

                trW.chrg.cpMin = cr.cpMin;
                trW.chrg.cpMax = cr.cpMax;
                trW.lpstrText = (LPWSTR) szTextAW;
                SendMessageW( ei.hWndEdit, EM_GETTEXTRANGE_X, 0, (LPARAM) &trW );
                if ( g_Options.dwFlags[OPTF_SRCH_USE_SPECIALCHARS] ||
                     g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] )
                {
                    int     i;
                    int     k;
                    wchar_t szTextW[2*MAX_TEXT_SIZE];

                    i = 0;
                    k = 0;
                    while ( trW.lpstrText[i] )
                    {
                        switch ( trW.lpstrText[i] )
                        {
                            case L'\t':
                                szTextW[k++] = L'\\';
                                szTextW[k++] = L't';
                                break;
                            case L'\r':
                                szTextW[k++] = L'\\';
                                szTextW[k++] = L'n';
                                break;
                            case L'\\':
                            case L'*':
                            case L'?':
                                szTextW[k++] = L'\\';
                            default:
                                if ( g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] )
                                {
                                    switch ( trW.lpstrText[i] )
                                    {
                                        case L'(':
                                        case L')':
                                        case L'[':
                                        case L']':
                                        case L'{':
                                        case L'}':
                                        case L'.':
                                        case L'!':
                                        case L'|':
                                        case L'+':
                                        case L'^':
                                        case L'$':
                                            szTextW[k++] = L'\\';
                                            break;
                                    }
                                }
                                szTextW[k++] = trW.lpstrText[i];
                                break;
                        }
                        ++i;
                    }
                    if ( k > i )
                    {
                        if ( k > MAX_TEXT_SIZE - 1 )
                        {
                            k = MAX_TEXT_SIZE - 1;
                        }
                        szTextW[k] = 0;
                        i = 0;
                        while ( (trW.lpstrText[i] = szTextW[i]) != 0 )
                        {
                            ++i;
                        }
                    }
                }
            }
            qs_bEditTextChanged = TRUE;
            return TRUE;
        }
    }
    return FALSE;
}

static BOOL EnableDlgItem(HWND hDlg, int itemID, BOOL bEnable)
{
    HWND hDlgItem = GetDlgItem(hDlg, itemID);
    return ( hDlgItem ? EnableWindow(hDlgItem, bEnable) : FALSE );
}

static WNDPROC setWndProc(HWND hWnd, WNDPROC newWndProc)
{
    if ( g_Plugin.bOldWindows )
        return (WNDPROC) SetWindowLongPtrA( hWnd, GWLP_WNDPROC, (LONG_PTR) newWndProc );
    else
        return (WNDPROC) SetWindowLongPtrW( hWnd, GWLP_WNDPROC, (LONG_PTR) newWndProc );
}

static LRESULT callWndProc(WNDPROC prevWndProc, 
  HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if ( g_Plugin.bOldWindows )
        return CallWindowProcA(prevWndProc, hWnd, uMsg, wParam, lParam);
    else
        return CallWindowProcW(prevWndProc, hWnd, uMsg, wParam, lParam);
}

static LRESULT CALLBACK btnFindWndProc(HWND hBtn,
  UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static BOOL bTrackingMouse = FALSE;
    WNDPROC prevWndProc;

    switch ( uMsg )
    {
        case WM_SETFOCUS:
        {
            qs_bBtnFindIsFocused = TRUE;
            break;
        }
        case WM_KILLFOCUS:
        {
            HWND hWndToFocus;

            hWndToFocus = (HWND) wParam;
            if ( (hWndToFocus != g_QSearchDlg.hBtnFindNext) &&
                 (hWndToFocus != g_QSearchDlg.hBtnFindPrev) )
            {
                qs_bBtnFindIsFocused = FALSE;
                if ( hWndToFocus != g_QSearchDlg.hFindEdit )
                {
                    qs_nEditEOF = 0;
                    PostMessage( g_QSearchDlg.hDlg, QSM_SETNOTFOUND, FALSE, 0 );
                }
            }
            break;
        }
        case WM_MOUSEMOVE:
        {
            if ( !bTrackingMouse )
            {
                TRACKMOUSEEVENT tme;

                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hBtn;
                tme.dwHoverTime = HOVER_DEFAULT;
                if ( _TrackMouseEvent(&tme) )
                {
                    bTrackingMouse = TRUE;
                }
            }
            break;
        }
        case WM_MOUSELEAVE:
        {
            bTrackingMouse = FALSE;
            if ( qs_bBtnFindIsFocused )
            {
                g_QSearchDlg.bMouseJustLeavedFindEdit = TRUE;
            }
            break;
        }
        default:
            break;
    }

    if ( hBtn == g_QSearchDlg.hBtnFindNext )
        prevWndProc = prev_btnFindNextWndProc;
    else
        prevWndProc = prev_btnFindPrevWndProc;
    return callWndProc(prevWndProc, hBtn, uMsg, wParam, lParam);
}

/*
static LRESULT CALLBACK btnWndProc(HWND hBtn,
  UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static BOOL qs_bBtnTrackMouse = FALSE;

    switch ( uMsg )
    {
        case WM_MOUSEMOVE:
        {
            if ( !qs_bBtnTrackMouse )
            {
                TRACKMOUSEEVENT tme;
                LONG_PTR        dwWndStyle;

                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hBtn;
                tme.dwHoverTime = HOVER_DEFAULT;
                if ( _TrackMouseEvent(&tme) )
                {
                    qs_bBtnTrackMouse = TRUE;
                }
                if ( g_Plugin.bOldWindows )
                    dwWndStyle = GetWindowLongPtrA(hBtn, GWL_STYLE);
                else
                    dwWndStyle = GetWindowLongPtrW(hBtn, GWL_STYLE);
                if ( dwWndStyle & BS_FLAT )
                {
                    dwWndStyle -= BS_FLAT;
                    dwWndStyle |= BS_PUSHLIKE;
                    if ( g_Plugin.bOldWindows )
                        SetWindowLongPtrA(hBtn, GWL_STYLE, dwWndStyle);
                    else
                        SetWindowLongPtrW(hBtn, GWL_STYLE, dwWndStyle);
                    SendMessage( 
                      hBtn, 
                      BM_SETSTYLE, 
                      BS_PUSHLIKE | BS_BITMAP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,
                      TRUE );
                }
            }
            break;
        }
        case WM_MOUSELEAVE:
        {
            LONG_PTR dwWndStyle;
            
            if ( g_Plugin.bOldWindows )
                dwWndStyle = GetWindowLongPtrA(hBtn, GWL_STYLE);
            else
                dwWndStyle = GetWindowLongPtrW(hBtn, GWL_STYLE);
            if ( dwWndStyle & BS_PUSHLIKE )
            {
                dwWndStyle -= BS_PUSHLIKE;
                dwWndStyle |= BS_FLAT;
                if ( g_Plugin.bOldWindows )
                    SetWindowLongPtrA(hBtn, GWL_STYLE, dwWndStyle);
                else
                    SetWindowLongPtrW(hBtn, GWL_STYLE, dwWndStyle);
                SendMessage( 
                  hBtn, 
                  BM_SETSTYLE, 
                  BS_FLAT | BS_BITMAP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,
                  TRUE );
            }
            qs_bBtnTrackMouse = FALSE;
            break;
        }
        default:
            break;
    }

    return callWndProc(prev_btnWndProc, hBtn, uMsg, wParam, lParam);
}
*/

/*static LRESULT CALLBACK chWndProc(HWND hCh,
  UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch ( uMsg )
    {
        case WM_LBUTTONDOWN:
            {
                LRESULT lResult = callWndProc(prev_chWndProc, 
                                    hCh, uMsg, wParam, lParam);
                SetFocus( g_QSearchDlg.hFindEdit );
                return lResult;
            }
            break;
    }

    return callWndProc(prev_chWndProc, hCh, uMsg, wParam, lParam);
}*/

BOOL qsPickUpSelection(HWND hEdit)
{
    if ( getAkelPadSelectedText(g_QSearchDlg.szFindTextW) )
    {
        setEditFindText(hEdit, g_QSearchDlg.szFindTextW);

        SendMessage(hEdit, EM_SETSEL, 0, -1);
        qs_bEditSelJustChanged = TRUE;

        if ( g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE] )
        {
            qs_bForceFindFirst = FALSE;
            qsearchDoTryHighlightAll(g_QSearchDlg.hDlg);
        }
        else
            qs_bForceFindFirst = TRUE;

        return TRUE;
    }

    return FALSE;
}

void qsChangeCkeckBoxState(WORD idCheckBox)
{
    const WPARAM wp = (BN_CLICKED << 16) & 0xFFFF0000L;
    WPARAM wState;
    HWND   hCh;

    hCh = GetDlgItem(g_QSearchDlg.hDlg, idCheckBox);

    if ( idCheckBox == IDC_CH_WHOLEWORD )
    {
        if ( !IsWindowVisible(hCh) )
            return;
    }

    if ( SendMessage(hCh, BM_GETCHECK, 0, 0) == BST_CHECKED )
        wState = BST_UNCHECKED;
    else
        wState = BST_CHECKED;

    SendMessage( hCh, BM_SETCHECK, wState, 0 );
    SendMessage( g_QSearchDlg.hDlg, WM_COMMAND, wp | idCheckBox, (LPARAM) hCh );
}

static LRESULT OnEditKeyDown_Enter_or_F3(HWND hEdit, WPARAM wParam)
{
    if ( GetKeyState(VK_QS_PICKUPTEXT) & 0x80 )
    {
        if ( getAkelPadSelectedText(g_QSearchDlg.szFindTextW) )
        {
            setEditFindText(hEdit, g_QSearchDlg.szFindTextW);
            SendMessage( g_QSearchDlg.hDlg, QSM_SETNOTFOUND, FALSE, 0 );
        }
        SendMessage(hEdit, EM_SETSEL, 0, -1);
#ifdef _DEBUG
        Debug_Output("editWndProc, WM_KEYDOWN, (RETURN||F3)&&PickUp, SETSEL(0, -1)\n");
#endif
        qs_bEditTextChanged = TRUE;
        if ( g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE] )
        {
            qs_bForceFindFirst = FALSE;
            qsearchDoTryHighlightAll(g_QSearchDlg.hDlg);
        }
        else
            qs_bForceFindFirst = TRUE;
    }
    else
    {
        if ( qsearchIsFindHistoryEnabled() && (wParam == VK_RETURN) )
        {
            HWND hCombo = GetDlgItem(g_QSearchDlg.hDlg, IDC_CB_FINDTEXT);
            if ( SendMessage(hCombo, CB_GETDROPPEDSTATE, 0, 0) )
            {
                // drop-down list box is shown
                int iItem = (int) SendMessage(hCombo, CB_GETCURSEL, 0, 0);
                if ( iItem != CB_ERR )
                {
                        //g_QSearchDlg.szFindTextW[0] = 0;
                        //SendMessageA(hCombo, CB_GETLBTEXT, iItem, (LPARAM) g_QSearchDlg.szFindTextW);
                        //SendMessageA(hCombo, CB_SETCURSEL, iItem, 0);
                    SendMessage(hCombo, CB_SHOWDROPDOWN, FALSE, 0);
                        //SetWindowTextA(hEdit, (LPCSTR) g_QSearchDlg.szFindTextW);
                    SendMessage( g_QSearchDlg.hDlg, QSM_SETNOTFOUND, FALSE, 0 );
                        //SendMessageA(hEdit, EM_SETSEL, 0, -1);
                    SetFocus(hEdit);
                    qs_bForceFindFirst = TRUE;
                    qs_bEditTextChanged = TRUE;
                    g_QSearchDlg.uSearchOrigin = QS_SO_QSEARCH;
                }
                else
                {
                    SendMessage(hCombo, CB_SHOWDROPDOWN, FALSE, 0);
                    SetFocus(hEdit);
                }
                return 0;
            }
        }

        SendMessage( g_QSearchDlg.hDlg, WM_COMMAND, IDOK, 0 );
    }

    return 0;
}

LRESULT CALLBACK editWndProc(HWND hEdit,
  UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static DWORD dwSelPos1 = 0;
    static DWORD dwSelPos2 = (DWORD)(-1L);
    static BOOL  bHotKeyPressed = FALSE;
    static BOOL  bEditTrackingMouse = FALSE;

#ifdef _DEBUG
    Debug_Output("Edit Msg 0x%X:  0x0%X  0x0%X\n", uMsg, wParam, lParam);
#endif

    switch ( uMsg )
    {
        case WM_KEYDOWN:
        {
            bHotKeyPressed = FALSE;

            if ( (wParam == VK_DOWN) || (wParam == VK_UP) ||
                 (wParam == VK_PRIOR) || (wParam == VK_NEXT) )
            {
                if ( qsearchIsFindHistoryEnabled() )
                {
                    HWND hCombo = GetDlgItem(g_QSearchDlg.hDlg, IDC_CB_FINDTEXT);
                    if ( !SendMessage(hCombo, CB_GETDROPPEDSTATE, 0, 0) )
                    {
                        int iItem = (int) SendMessage(hCombo, CB_GETCURSEL, 0, 0);

                        getEditFindText(hEdit, g_QSearchDlg.szFindTextW);
                        if ( qsearchFindHistoryAdd(hEdit, g_QSearchDlg.szFindTextW, 0) )
                        {
                            if ( wParam == VK_DOWN ) // Arrow Down
                            {
                                if ( SendMessage(hCombo, CB_GETCOUNT, 0, 0) > (iItem + 1) )
                                    ++iItem; // next item
                            }
                            else if ( wParam == VK_NEXT ) // Page Down
                            {
                                iItem = (int) SendMessage(hCombo, CB_GETCOUNT, 0, 0);
                                if ( iItem > 0 )
                                    --iItem; // last item
                            }
                            SendMessage(hCombo, CB_SETCURSEL, iItem, 0);
                            g_QSearchDlg.uSearchOrigin = QS_SO_QSEARCH;
                        }
                    }
                }
            }

            if ( (wParam == VK_RETURN) || (wParam == VK_F3) )
            {
                return OnEditKeyDown_Enter_or_F3(hEdit, wParam);
            }
            else if ( (wParam == VK_DELETE) || (wParam == VK_BACK) )
            {
                qs_bEditTextChanged = TRUE;
                g_QSearchDlg.uSearchOrigin = QS_SO_QSEARCH;

                if ( GetKeyState(VK_CONTROL) & 0x80 ) // Ctrl+Del, Ctrl+BS
                {
                    cutEditText( hEdit, (wParam == VK_DELETE) );
                    if ( g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE] )
                    {
                        SendMessage( g_QSearchDlg.hDlg, QSM_FINDFIRST, QSEARCH_NOFINDUP | QSEARCH_NOFINDBEGIN, 0 );
                    }
                    else
                    {
                        SendMessage( g_QSearchDlg.hDlg, QSM_SETNOTFOUND, FALSE, 0 );
                    }
                    return 0;
                }
                else if ( wParam == VK_DELETE ) // Del
                {
                    if ( g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE] )
                    {
                        LRESULT lResult = callWndProc(prev_editWndProc, 
                                            hEdit, uMsg, wParam, lParam);
                        SendMessage( g_QSearchDlg.hDlg, QSM_FINDFIRST, QSEARCH_NOFINDUP | QSEARCH_NOFINDBEGIN, 0 );
                        return lResult;
                    }
                    else
                    {
                        LRESULT lResult = callWndProc(prev_editWndProc, 
                                            hEdit, uMsg, wParam, lParam);
                        SendMessage( g_QSearchDlg.hDlg, QSM_SETNOTFOUND, FALSE, 0 );
                        return lResult;
                    }
                }
                // BS is processed in WM_CHAR
            }
            else if ( wParam == VK_ESCAPE )
            {
                HWND hCombo = NULL;

                if ( qsearchIsFindHistoryEnabled() )
                {
                    hCombo = GetDlgItem(g_QSearchDlg.hDlg, IDC_CB_FINDTEXT);
                    if ( !SendMessage(hCombo, CB_GETDROPPEDSTATE, 0, 0) )
                        hCombo = NULL;
                }

                if ( hCombo )
                {
                    // drop-down list box is shown - hide it
                    int iItem = (int) SendMessage(hCombo, CB_GETCURSEL, 0, 0);
                    if ( iItem != CB_ERR )
                    {
                        SendMessage(hCombo, CB_SHOWDROPDOWN, FALSE, 0);
                        SendMessage( g_QSearchDlg.hDlg, QSM_SETNOTFOUND, FALSE, 0 );
                        SetFocus(hEdit);
                        qs_bForceFindFirst = TRUE;
                        qs_bEditTextChanged = TRUE;
                        g_QSearchDlg.uSearchOrigin = QS_SO_QSEARCH;
                    }
                    else
                    {
                        SendMessage(hCombo, CB_SHOWDROPDOWN, FALSE, 0);
                        SetFocus(hEdit);
                    }
                    return 0;
                }
                else
                {
                    // drop-down list box is hidden
                    if ( g_Options.dwFlags[OPTF_CATCH_MAIN_ESC] == ESC_THROW_TO_MAIN )
                    {
                        PostMessage( g_Plugin.hMainWnd, WM_COMMAND, 0x10000 | IDM_FILE_EXIT, 0 );
                        return 0;
                    }

                    SendMessage( g_QSearchDlg.hDlg, QSM_SHOWHIDE, FALSE, 0 );
                    return 0;
                }
            }
            else if ( wParam == VK_TAB )
            {
                return 0;
            }
            else if ( wParam == 0x41 ) // 'A'
            {
                if ( GetKeyState(VK_CONTROL) & 0x80 ) // Ctrl+A
                {
                    // we can have Grey Alt here (e.g. Polish keyboard layout)
                    // even though Alt usually comes under WM_SYSKEYDOWN
                    // Grey Alt comes under WM_KEYDOWN as VK_CONTROL + VK_MENU
                    if ( (GetKeyState(VK_MENU) & 0x80) != 0x80 ) // no Grey Alt
                    {
                        SendMessage( hEdit, EM_SETSEL, 0, -1 );
#ifdef _DEBUG
                        Debug_Output("editWndProc, WM_KEYDOWN, Ctrl+A, SETSEL(0, -1)\n");
#endif
                        return 0;
                    }
                }
            }
            else if ( LOBYTE(wParam) == LOBYTE(qs_dwHotKey) )
            {
                bHotKeyPressed = isQSearchHotKeyPressed();
                return 0;
            }
            break;
        }
        case WM_SYSKEYDOWN:
        {
            // it's required when qs_dwHotKey contains Alt
            // (Alt+key do not come as WM_KEYDOWN)
            if ( LOBYTE(wParam) == LOBYTE(qs_dwHotKey) )
            {
                bHotKeyPressed = isQSearchHotKeyPressed();
                return 0;
            }
            if ( (wParam == VK_RETURN) || (wParam == VK_F3) )
            {
                // (if Alt+Enter or Alt+F3 pressed)
                OnEditKeyDown_Enter_or_F3(hEdit, wParam);
                return 0;
            }
            /*
            if ( g_Options.dwUseAltHotkeys )
            {
                if ( wParam == g_Options.dwAltMatchCase )
                {
                    qsChangeCkeckBoxState(IDC_CH_MATCHCASE);
                    return 1;
                }
                if ( wParam == g_Options.dwAltHighlightAll )
                {
                    qsChangeCkeckBoxState(IDC_CH_HIGHLIGHTALL);
                    return 1;
                }
                if ( wParam == g_Options.dwAltWholeWord )
                {
                    qsChangeCkeckBoxState(IDC_CH_WHOLEWORD);
                    return 1;
                }
            }
            */
            break;
        }
        case WM_KEYUP:
        {
            if ( bHotKeyPressed )
            {
                bHotKeyPressed = FALSE;
                if ( !qs_bHotKeyPressedOnShow )
                {
                    if ( g_Options.dwFlags[OPTF_HOTKEY_HIDES_PANEL] )
                    {
                        SendMessage( g_QSearchDlg.hDlg, QSM_SHOWHIDE, FALSE, 0 );
                    }
                    else
                    {
                        SetFocus( g_Plugin.hMainWnd );
                        /*if ( (g_Options.dwFlags[OPTF_EDITOR_AUTOFOCUS] & AUTOFOCUS_EDITOR_ALWAYS) == AUTOFOCUS_EDITOR_ALWAYS )
                        {
                            qs_bEditIsActive = FALSE;
                            qs_bEditTrackMouse = FALSE;
                        }*/
                    }
                    return 0;
                }
            }

            qs_bHotKeyPressedOnShow = FALSE;

            if ( (wParam == VK_RETURN) || 
                 (wParam == VK_ESCAPE) || 
                 (wParam == VK_TAB) )
            {
                return 0;
            }
            break;
        }
        case WM_SYSKEYUP:
        {
            if ( bHotKeyPressed )
            {
                SendMessage(hEdit, WM_KEYUP, wParam, lParam);
                return 0;
            }
            break;
        }
        case WM_CHAR:
        {
            if ( bHotKeyPressed ||
                 (wParam == VK_RETURN) ||
                 (wParam == VK_ESCAPE) ||
                 (wParam == VK_TAB) ||
                 (wParam == 0x0A) ||      /* 0x0A is Ctrl+Enter. Why? Ask M$ ;) */
                 (wParam == 0x7F) )       /* 0x7F is Ctrl+Backspace. Why? Ask M$ */
            {
                return 0;
            }
            else
            {
                qs_bEditTextChanged = TRUE;
                g_QSearchDlg.uSearchOrigin = QS_SO_QSEARCH;
                if ( g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE] )
                {
                    LRESULT lResult = callWndProc(prev_editWndProc, 
                                        hEdit, uMsg, wParam, lParam);
                    SendMessage( g_QSearchDlg.hDlg, QSM_FINDFIRST, QSEARCH_NOFINDUP | QSEARCH_NOFINDBEGIN, 0 );
                    return lResult;
                }
                else
                {
                    LRESULT lResult = callWndProc(prev_editWndProc, 
                                        hEdit, uMsg, wParam, lParam);
                    SendMessage( g_QSearchDlg.hDlg, QSM_SETNOTFOUND, FALSE, 0 );
                    return lResult;
                }
            }
            break;
        }
        case WM_SETFOCUS:
        {
            qs_bEditIsActive = TRUE;
            InvalidateRect(hEdit, NULL, TRUE);
            UpdateWindow(hEdit);
            if ( qs_bEditCanBeNonActive )
            {
                if ( g_Options.dwFlags[OPTF_EDIT_FOCUS_SELECTALL] )
                {
                    SendMessage(hEdit, EM_SETSEL, 0, -1);
#ifdef _DEBUG
                    Debug_Output("editWndProc, WM_SETFOCUS, if (SelectAll), SETSEL(0, -1)\n");
#endif
                }
                else
                {
                    if ( !qs_bEditSelJustChanged )
                    {
                        SendMessage(hEdit, EM_SETSEL, dwSelPos1, dwSelPos2);
#ifdef _DEBUG
                        Debug_Output("editWndProc, WM_SETFOCUS, if (!SelJustChanged), SETSEL(%d, %d)\n", dwSelPos1, dwSelPos2);
#endif
                    }
                    else
                    {
                        qs_bEditSelJustChanged = FALSE;
                        SendMessage( hEdit, EM_GETSEL, 
                          (WPARAM) &dwSelPos1, (LPARAM) &dwSelPos2 );
#ifdef _DEBUG
                        Debug_Output("editWndProc, WM_GETFOCUS, if (SelJustChanged), GETSEL(%d, %d)\n", dwSelPos1, dwSelPos2);
#endif
                    }
                }
            }
            g_QSearchDlg.bMouseJustLeavedFindEdit = FALSE;
//#ifdef _DEBUG
//            Debug_Output("QSearchDlg.c, editWndProc, WM_SETFOCUS, g_QSearchDlg.MouseJustLeavedFindEdit = FALSE;\n");
//#endif
            break;
        }
        case WM_KILLFOCUS:
        {
            if ( qs_bEditCanBeNonActive )
            {
                HWND hWndToFocus;

                qs_bEditIsActive = FALSE;
                if ( !g_Options.dwFlags[OPTF_EDIT_FOCUS_SELECTALL] )
                {
                    SendMessage( hEdit, EM_GETSEL, 
                      (WPARAM) &dwSelPos1, (LPARAM) &dwSelPos2 );
#ifdef _DEBUG
                    Debug_Output("editWndProc, WM_KILLFOCUS, GETSEL(%d, %d)\n", dwSelPos1, dwSelPos2);
#endif
                }

                hWndToFocus = (HWND) wParam;
                if ( (hWndToFocus != g_QSearchDlg.hBtnFindNext) &&
                     (hWndToFocus != g_QSearchDlg.hBtnFindPrev) )
                {
                    qs_nEditEOF = 0;
                    SendMessage( g_QSearchDlg.hDlg, QSM_SETNOTFOUND, FALSE, 0 );
                    g_QSearchDlg.bMouseJustLeavedFindEdit = FALSE;
//#ifdef _DEBUG
//                    Debug_Output("QSearchDlg.c, editWndProc, WM_KILLFOCUS, g_QSearchDlg.MouseJustLeavedFindEdit = FALSE;\n");
//#endif
                }
                /*
                InvalidateRect(hEdit, NULL, TRUE);
                UpdateWindow(hEdit);
                */
            }
            break;
        }
        case WM_MOUSEMOVE:
        {
            if ( (!qs_bEditIsActive) && (!bEditTrackingMouse) )
            {
                if ( (g_Options.dwFlags[OPTF_QSEARCH_AUTOFOCUS] & 0x01) ||
                     (g_Options.dwFlags[OPTF_EDITOR_AUTOFOCUS] & 0x01) )
                {
                    HWND hFocusedWnd = GetFocus();
                    // When AkelPad's window is not active, GetFocus() returns NULL
                    if ( hFocusedWnd && hFocusedWnd != hEdit )
                    {
                        SetFocus(hEdit);
                    }
                }
            }
            if ( !bEditTrackingMouse )
            {
                TRACKMOUSEEVENT tme;

                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hEdit;
                tme.dwHoverTime = HOVER_DEFAULT;
                if ( _TrackMouseEvent(&tme) )
                {
                    bEditTrackingMouse = TRUE;
                }

                g_QSearchDlg.bMouseJustLeavedFindEdit = FALSE;
//#ifdef _DEBUG
//                Debug_Output("QSearchDlg.c, editWndProc, WM_MOUSEMOVE, g_QSearchDlg.MouseJustLeavedFindEdit = FALSE;\n");
//#endif
            }
            break;
        }
        case WM_MOUSELEAVE:
        {
            bEditTrackingMouse = FALSE;
            g_QSearchDlg.bMouseJustLeavedFindEdit = TRUE;
//#ifdef _DEBUG
//            Debug_Output("QSearchDlg.c, editWndProc, WM_MOUSELEAVE, g_QSearchDlg.MouseJustLeavedFindEdit = TRUE;\n");
//#endif
            break;
        }
        case WM_GETDLGCODE:
        {
            if ( !g_Options.dwFlags[OPTF_EDIT_FOCUS_SELECTALL] )
            {
                LRESULT lResult = callWndProc(prev_editWndProc, hEdit, uMsg, wParam, lParam);
                if ( lResult & DLGC_HASSETSEL )
                {
                    lResult -= DLGC_HASSETSEL;
#ifdef _DEBUG
                    {
                        DWORD pos1, pos2;
                        SendMessage(hEdit, EM_GETSEL, (WPARAM)&pos1, (LPARAM)&pos2);
                        Debug_Output("editWndProc, WM_GETDLGCODE, GETSEL(%d, %d)\n", pos1, pos2);
                    }
#endif
                }
                return lResult;
            }
            break;
        }
        case WM_NOTIFY:
        {
            if ( g_Plugin.bOldWindows )
            {
                if ( ((LPNMHDR) lParam)->code == TTN_GETDISPINFOA )
                {
                    LPNMTTDISPINFOA lpnmdiA = (LPNMTTDISPINFOA) lParam;
                    SendMessage(lpnmdiA->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);
                    lpnmdiA->lpszText = (LPSTR) qsearchGetHintA(IDC_ED_FINDTEXT);
                    return 0;
                }
            }
            else
            {
                if ( ((LPNMHDR) lParam)->code == TTN_GETDISPINFOW )
                {
                    LPNMTTDISPINFOW lpnmdiW = (LPNMTTDISPINFOW) lParam;
                    SendMessage(lpnmdiW->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);
                    lpnmdiW->lpszText = (LPWSTR) qsearchGetHintW(IDC_ED_FINDTEXT);
                    return 0;
                }
            }
            break;
        }
        default:
            break;
    }

    return callWndProc(prev_editWndProc, hEdit, uMsg, wParam, lParam);
}

static BOOL isHighlightMainActive(void)
{
    if ( g_Plugin.bOldWindows )
    {
        PLUGINFUNCTION *pfA = (PLUGINFUNCTION *) SendMessageA( g_Plugin.hMainWnd,
            AKD_DLLFINDA, (WPARAM) cszHighlightMainA, 0 );

        if ( pfA && pfA->bRunning )
        {
            return TRUE;
        }
    }
    else
    {
        PLUGINFUNCTION *pfW = (PLUGINFUNCTION *) SendMessageW( g_Plugin.hMainWnd,
            AKD_DLLFINDW, (WPARAM) cszHighlightMainW, 0 );

        if ( pfW && pfW->bRunning )
        {
            return TRUE;
        }
    }

    return FALSE;
}

static void qsUpdateHighlight(HWND hDlg, HWND hEdit)
{
    wchar_t szSelectedTextW[MAX_TEXT_SIZE];

    getEditFindText(hEdit, g_QSearchDlg.szFindTextW);

    if ( getAkelPadSelectedText(szSelectedTextW) )
    {
        BOOL bEqual = FALSE;

        if ( g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] )
        {
            bEqual = TRUE;
        }
        else
        {
            HWND    hChMatchCase;
            wchar_t szText1[MAX_TEXT_SIZE];
            wchar_t szText2[MAX_TEXT_SIZE];
            const wchar_t* psz1;
            const wchar_t* psz2;

            hChMatchCase = GetDlgItem(hDlg, IDC_CH_MATCHCASE);
            if ( SendMessage(hChMatchCase, BM_GETCHECK, 0, 0) == BST_CHECKED )
            {
                // match case
                psz1 = g_QSearchDlg.szFindTextW;
                psz2 = szSelectedTextW;
            }
            else
            {
                // case-insensitive
                if ( g_Plugin.bOldWindows )
                {
                    lstrcpyA( (LPSTR) szText1, (LPCSTR) g_QSearchDlg.szFindTextW );
                    CharUpperA( (LPSTR) szText1 );
                    lstrcpyA( (LPSTR) szText2, (LPCSTR) szSelectedTextW );
                    CharUpperA( (LPSTR) szText2 );
                }
                else
                {
                    lstrcpyW( szText1, g_QSearchDlg.szFindTextW );
                    CharUpperW( szText1 );
                    lstrcpyW( szText2, szSelectedTextW );
                    CharUpperW( szText2 );
                }
                psz1 = szText1;
                psz2 = szText2;
            }

            if ( g_Options.dwFlags[OPTF_SRCH_USE_SPECIALCHARS] )
            {
                if ( g_Plugin.bOldWindows )
                    bEqual = (match_mask((LPCSTR)psz1, (LPCSTR)psz2, 0, 0) > 0);
                else
                    bEqual = (match_maskw(psz1, psz2, 0, 0) > 0);
            }
            else
            {
                if ( g_Plugin.bOldWindows )
                    bEqual = (lstrcmpA((LPCSTR)psz1, (LPCSTR)psz2) == 0);
                else
                    bEqual = (lstrcmpW(psz1, psz2) == 0);
            }
        }

        qs_bEditTextChanged = FALSE;

        if ( !bEqual )
        {
            DWORD dwStopEOF = g_Options.dwFlags[OPTF_SRCH_STOP_EOF];
            g_Options.dwFlags[OPTF_SRCH_STOP_EOF] = 0; // temporary disable
            qsearchDoSearchText( hEdit, QSEARCH_NEXT );
            g_Options.dwFlags[OPTF_SRCH_STOP_EOF] = dwStopEOF; // restore
        }

        qsearchDoTryHighlightAll(hDlg);
    }
    else
    {
        // no text selected - trying to find & highlight
        DWORD dwStopEOF = g_Options.dwFlags[OPTF_SRCH_STOP_EOF];
        g_Options.dwFlags[OPTF_SRCH_STOP_EOF] = 0; // temporary disable
        qsearchDoSearchText( hEdit, QSEARCH_NEXT );
        g_Options.dwFlags[OPTF_SRCH_STOP_EOF] = dwStopEOF; // restore

        qsearchDoTryHighlightAll(hDlg);
    }
}

INT_PTR CALLBACK qsearchDlgProc(HWND hDlg,
  UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND   hFindEdit = NULL;
    static HWND   hToolTip = NULL;
    static HMENU  hPopupMenuLoaded = NULL;
    static HMENU  hPopupMenu = NULL;
    static HBRUSH hTextNotFoundBrush = NULL;
    static HBRUSH hTextNotRegExpBrush = NULL;
    static HBRUSH hTextEOFBrush = NULL;
    //static BOOL   bHotKeyPressed = FALSE;

#ifdef _DEBUG
    Debug_Output("Dlg Msg 0x%X:  0x0%X  0x0%X\n", uMsg, wParam, lParam);
#endif

    switch ( uMsg )
    {
        case WM_COMMAND:
        {
            unsigned int id = LOWORD(wParam);
            if ( id == IDOK || id == IDOK_FINDPREV )
            {
                // Originally we get here when Enter is pressed. Why? Ask M$ ;)
                unsigned int uSearch;

                if ( id == IDOK )
                {
                    if ( GetKeyState(VK_QS_PICKUPTEXT) & 0x80 )
                    {
                        OnEditKeyDown_Enter_or_F3(hFindEdit, VK_RETURN);
                        return 1;
                    }
                }

                uSearch = QSEARCH_NEXT;
                if ( qs_bEditTextChanged && 
                     (qs_bForceFindFirst ||
                      g_Options.dwFlags[OPTF_SRCH_FROM_BEGINNING] ||
                      !g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE]) )
                {
                    uSearch = QSEARCH_FIRST;
                    qs_bForceFindFirst = FALSE;
                }
                if ( id == IDOK_FINDPREV )
                {
                    uSearch |= QSEARCH_FINDUP;
                }
                qs_bEditTextChanged = FALSE;
                /*if ( g_Options.dwFlags[OPTF_SRCH_PICKUP_SELECTION] & 0x01 )
                {
                    getEditFindText( hFindEdit, g_QSearchDlg.szFindTextW );
                    qsearchFindHistoryAdd( hFindEdit, g_QSearchDlg.szFindTextW, 0 );
                    qsPickUpSelection( hFindEdit );
                }*/
                getEditFindText( hFindEdit, g_QSearchDlg.szFindTextW );
                qsearchDoSearchText( hFindEdit, uSearch );
                if ( uSearch & QSEARCH_FIRST )
                {
                    qsearchDoTryHighlightAll( hDlg );
                }
                return 1;
            }
            else if ( id == IDCANCEL )
            {
                // We get here when Esc is pressed. Why? Ask M$ ;)
                if ( g_Options.dwFlags[OPTF_CATCH_MAIN_ESC] == ESC_THROW_TO_MAIN )
                {
                    PostMessage( g_Plugin.hMainWnd, WM_COMMAND, 0x10000 | IDM_FILE_EXIT, 0 );
                    return 0;
                }

                //qsearchDoQuit( hFindEdit, hToolTip, hPopupMenuLoaded, hTextNotFoundBrush, hTextNotRegExpBrush, hTextEOFBrush );
                qsearchDoShowHide(hDlg, FALSE, 0);
                return 1;
            }
            else if ( id == IDC_BT_CANCEL )
            {
                //qsearchDoQuit( hFindEdit, hToolTip, hPopupMenuLoaded, hTextNotFoundBrush, hTextNotRegExpBrush, hTextEOFBrush );
                qsearchDoShowHide(hDlg, FALSE, 0);
                return 1;
            }
            else if ( id == IDC_BT_FINDNEXT )
            {
                LPARAM uFindFlags = 0;
                if (g_Options.dwFlags[OPTF_SRCH_PICKUP_SELECTION] == PICKUP_SEL_IF_NOT_QSBTN)
                    uFindFlags |= QS_FF_NOPICKUPSEL;
                PostMessage( hDlg, QSM_FINDNEXT, FALSE, uFindFlags );
                //PostMessage( hDlg, WM_COMMAND, IDOK, 0 );
                return 1;
            }
            else if ( id == IDC_BT_FINDPREV )
            {
                LPARAM uFindFlags = 0;
                if (g_Options.dwFlags[OPTF_SRCH_PICKUP_SELECTION] == PICKUP_SEL_IF_NOT_QSBTN)
                    uFindFlags |= QS_FF_NOPICKUPSEL;
                PostMessage( hDlg, QSM_FINDNEXT, TRUE, uFindFlags );
                //PostMessage( hDlg, WM_COMMAND, IDOK_FINDPREV, 0 );
                return 1;
            }
            else if ( id == IDC_CH_MATCHCASE )
            {
                if ( HIWORD(wParam) == BN_CLICKED )
                {
                    HWND hChHighlightAll;

                    qs_bForceFindFirst = TRUE;
                    qs_bEditTextChanged = TRUE;
                    qsearchDoSetNotFound(hFindEdit, FALSE, FALSE, FALSE);

                    hChHighlightAll = GetDlgItem(hDlg, IDC_CH_HIGHLIGHTALL);
                    if ( SendMessage(hChHighlightAll, BM_GETCHECK, 0, 0) == BST_CHECKED )
                        qsUpdateHighlight(hDlg, hFindEdit);
                }
            }
            else if ( id == IDC_CH_WHOLEWORD )
            {
                if ( HIWORD(wParam) == BN_CLICKED )
                {
                    HWND hChHighlightAll;

                    qs_bForceFindFirst = TRUE;
                    qs_bEditTextChanged = TRUE;
                    qsearchDoSetNotFound(hFindEdit, FALSE, FALSE, FALSE);

                    hChHighlightAll = GetDlgItem(hDlg, IDC_CH_HIGHLIGHTALL);
                    if ( SendMessage(hChHighlightAll, BM_GETCHECK, 0, 0) == BST_CHECKED )
                    {
                        DWORD dwOnTheFlyMode;
                        DWORD dwStopEOF;

                        getEditFindText(hFindEdit, g_QSearchDlg.szFindTextW);

                        dwOnTheFlyMode = g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE];
                        g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE] = 1;
                        dwStopEOF = g_Options.dwFlags[OPTF_SRCH_STOP_EOF];
                        g_Options.dwFlags[OPTF_SRCH_STOP_EOF] = 0;
                        qsearchDoSearchText( hFindEdit, QSEARCH_FIRST );
                        g_Options.dwFlags[OPTF_SRCH_STOP_EOF] = dwStopEOF;
                        g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE] = dwOnTheFlyMode;

                        qsearchDoTryHighlightAll(hDlg);
                    }
                }
            }
            else if ( id == IDC_CH_HIGHLIGHTALL )
            {
                if ( HIWORD(wParam) == BN_CLICKED )
                {
                    HWND hChHighlightAll;

                    hChHighlightAll = GetDlgItem(hDlg, IDC_CH_HIGHLIGHTALL);
                    if ( SendMessage(hChHighlightAll, BM_GETCHECK, 0, 0) == BST_CHECKED )
                    {
                        g_Options.dwHighlightState |= HLS_IS_CHECKED;
                        qsUpdateHighlight(hDlg, hFindEdit);
                    }
                    else
                    {
                        if ( (g_Options.dwHighlightState & HLS_IS_CHECKED) == HLS_IS_CHECKED )
                            g_Options.dwHighlightState -= HLS_IS_CHECKED;
                        qsearchDoTryUnhighlightAll();
                    }
                }
            }
            else if ( id == IDC_CB_FINDTEXT )
            {
                switch ( HIWORD(wParam) )
                {
                    case CBN_SELENDOK:
                    case CBN_SELCHANGE:
                        qs_bEditTextChanged = TRUE;
                        qs_bForceFindFirst = TRUE;
                        g_QSearchDlg.uSearchOrigin = QS_SO_QSEARCH;
                        qsearchDoSetNotFound(hFindEdit, FALSE, FALSE, FALSE);
                        break;
                    case CBN_DROPDOWN:
                        getEditFindText(hFindEdit, g_QSearchDlg.szFindTextW);
                        qsearchFindHistoryAdd(hFindEdit, g_QSearchDlg.szFindTextW, 0);
                        /*{
                            HWND hCombo;
                            int iItem;

                            hCombo = GetDlgItem(g_QSearchDlg.hDlg, IDC_CB_FINDTEXT);
                            if ( g_Plugin.bOldWindows )
                                iItem = (int) SendMessageA(hCombo, CB_FINDSTRING, (WPARAM) (0), (LPARAM) g_QSearchDlg.szFindTextW);
                            else
                                iItem = (int) SendMessageW(hCombo, CB_FINDSTRING, (WPARAM) (0), (LPARAM) g_QSearchDlg.szFindTextW);

                            if ( iItem != CB_ERR )
                                SendMessage(hCombo, CB_SETCURSEL, iItem, 0);
                        }*/
                        break;
                }
            }
            else if ( (id >= IDM_START) && (id < IDM_START + OPTF_COUNT - 1) )
            {
                if ( hPopupMenu )
                {
                    unsigned int state = GetMenuState(hPopupMenu, id, MF_BYCOMMAND);
                    switch ( id - IDM_START )
                    {
                        case OPTF_SRCH_PICKUP_SELECTION:
                        case OPTF_SRCH_STOP_EOF:
                        case OPTF_CATCH_MAIN_ESC:
                        case OPTF_EDITOR_AUTOFOCUS:
                            if ( (state & MF_CHECKED) == MF_CHECKED )
                            {
                                if ( g_Options.dwFlags[id - IDM_START] & 0x01 )
                                {
                                    g_Options.dwFlags[id - IDM_START] -= 0x01;

                                    if ( id == IDM_START + OPTF_SRCH_STOP_EOF )
                                        qsearchDoSetNotFound(hFindEdit, FALSE, FALSE, FALSE);
                                }
                            }
                            else
                            {
                                g_Options.dwFlags[id - IDM_START] |= 0x01;
                            }
                            break;

                        default:
                            g_Options.dwFlags[id - IDM_START] = 
                                ( (state & MF_CHECKED) == MF_CHECKED ) ? 0 : 1;
                            break;
                    }
                    if ( id == IDM_SRCHONTHEFLYMODE )
                    {
                        /*if ( g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE] )
                        {
                            CheckMenuItem( hPopupMenu, IDM_SRCHUSESPECIALCHARS, 
                              MF_BYCOMMAND | MF_UNCHECKED );
                            g_Options.dwFlags[OPTF_SRCH_USE_SPECIALCHARS] = 0;
                        }*/
                        qsdlgShowHideWholeWordCheckBox(hDlg);
                        qs_nEditEOF = 0;
                        qsearchDoSetNotFound( hFindEdit, FALSE, FALSE, FALSE );
                    }
                    else if ( id == IDM_SRCHUSESPECIALCHARS )
                    {
                        if ( g_Options.dwFlags[OPTF_SRCH_USE_SPECIALCHARS] )
                        {
                            CheckMenuItem( hPopupMenu, IDM_SRCHUSEREGEXP, 
                              MF_BYCOMMAND | MF_UNCHECKED );
                            EnableMenuItem( hPopupMenu, IDM_SRCHREGEXPDOTNEWLINE,
                              MF_BYCOMMAND | MF_GRAYED );
                            g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] = 0;
                        }
                        qsdlgShowHideWholeWordCheckBox(hDlg);
                        qs_nEditEOF = 0;
                        qsearchDoSetNotFound( hFindEdit, FALSE, FALSE, FALSE );
                    }
                    else if ( id == IDM_SRCHUSEREGEXP )
                    {
                        if ( g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] )
                        {
                            CheckMenuItem( hPopupMenu, IDM_SRCHUSESPECIALCHARS, 
                              MF_BYCOMMAND | MF_UNCHECKED );
                            EnableMenuItem( hPopupMenu, IDM_SRCHREGEXPDOTNEWLINE,
                              MF_BYCOMMAND | MF_ENABLED );
                            g_Options.dwFlags[OPTF_SRCH_USE_SPECIALCHARS] = 0;
                        }
                        qsdlgShowHideWholeWordCheckBox(hDlg);
                        qs_nEditEOF = 0;
                        qsearchDoSetNotFound( hFindEdit, FALSE, FALSE, FALSE );
                    }
                    else if ( id == IDM_SRCHPICKUPSELECTION )
                    {
                        if ( !(g_Options.dwFlags[OPTF_SRCH_PICKUP_SELECTION] & 0x01) )
                        {
                            g_QSearchDlg.uSearchOrigin = QS_SO_UNKNOWN;
                        }
                    }
                    else if ( id == IDM_SRCHWNDDOCKEDTOP )
                    {
                        if ( g_QSearchDlg.pDockData )
                        {
                            state = DK_SETBOTTOM;
                            if ( g_Options.dwFlags[OPTF_SRCH_WND_DOCKEDTOP] )
                            {
                                state = DK_SETTOP;
                            }
                            SendMessage( g_Plugin.hMainWnd, AKD_DOCK, 
                              state, (LPARAM) g_QSearchDlg.pDockData );
                            PostMessage( g_Plugin.hMainWnd, AKD_RESIZE, 0, 0 );
                        }
                    }
                }
            }
            break;
        }
        //case WM_KEYDOWN:
        //{
        //    bHotKeyPressed = FALSE;
        //    if ( LOBYTE(wParam) == LOBYTE(qs_dwHotKey) )
        //    {
        //        bHotKeyPressed = isQSearchHotKeyPressed();
        //        return 0;
        //    }
        //    break;
        //}
        //case WM_KEYUP:
        //{
        //    if ( bHotKeyPressed )
        //    {
        //        bHotKeyPressed = FALSE;
        //        if ( !qs_bHotKeyPressedOnShow )
        //        {
        //            if ( hFindEdit )
        //            {
        //                SetFocus(hFindEdit);
        //            }
        //            return 0;
        //        }
        //    }
        //
        //    qs_bHotKeyPressedOnShow = FALSE;
        //    break;
        //}
        case WM_SYSKEYDOWN:
        {
            if ( g_Options.dwUseAltHotkeys )
            {
                if ( wParam == g_Options.dwAltMatchCase )
                {
                    qsChangeCkeckBoxState(IDC_CH_MATCHCASE);
                    return 1;
                }
                if ( wParam == g_Options.dwAltHighlightAll )
                {
                    qsChangeCkeckBoxState(IDC_CH_HIGHLIGHTALL);
                    return 1;
                }
                if ( wParam == g_Options.dwAltWholeWord )
                {
                    qsChangeCkeckBoxState(IDC_CH_WHOLEWORD);
                    return 1;
                }
            }
            //if ( LOBYTE(wParam) == LOBYTE(qs_dwHotKey) )
            //{
            //    bHotKeyPressed = isQSearchHotKeyPressed();
            //    return 0;
            //}
            break;
        }
        //case WM_SYSKEYUP:
        //{
        //    if ( bHotKeyPressed )
        //    {
        //        SendMessage(hDlg, WM_KEYUP, wParam, lParam);
        //        return 0;
        //    }
        //    break;
        //}
        case WM_SYSCOMMAND:
        {
            if ( wParam == SC_KEYMENU )
            {
                if ( lParam == 0x0D )
                {
                    // this is {Enter} in {Alt+Enter}
                    return 1; // disabling the annoying "ding" sound
                }
            }
            break;
        }
        case WM_SETFOCUS:
        {
            EnableWindow( 
              GetDlgItem(hDlg, IDC_CH_HIGHLIGHTALL), 
              g_bHighlightPlugin
            );

            if ( !hFindEdit )
            {
                hFindEdit = qsearchGetFindEdit(hDlg);
                g_QSearchDlg.hFindEdit = hFindEdit;
            }
            if ( g_Plugin.nDockedDlgIsWaitingForOnStart == 0 )
            {
                qs_bEditIsActive = TRUE;
                SetFocus(hFindEdit);
            }
            else
            {
                qs_bEditIsActive = FALSE;
                SetFocus(g_Plugin.hMainWnd);
            }
            break;
        }
        case WM_CTLCOLOREDIT:
        {
            if ( hFindEdit == (HWND) lParam )
            {
                int nBkColor = COLOR_BTNFACE; // normal inactive
                if ( qs_bEditIsActive || qs_bBtnFindIsFocused )
                {
                    if ( !qs_bEditNotFound )
                    {
                        if ( (!qs_bEditIsEOF) || (g_Options.colorEOF == RGB(0xFF,0xFF,0xFF)) )
                            nBkColor = COLOR_WINDOW; // normal active
                        else
                            nBkColor = 0x7FFE; // EOF active
                    }
                    else
                    {
                        if ( !qs_bEditNotRegExp )
                            nBkColor = 0x7FFF; // not found active
                        else
                            nBkColor = 0x7FFD; // not regexp
                    }
                }
                SetTextColor( (HDC) wParam, GetSysColor(COLOR_WINDOWTEXT) );
                SetBkMode( (HDC) wParam, TRANSPARENT );
                switch ( nBkColor )
                {
                    case 0x7FFF:
                        SetBkColor( (HDC) wParam, g_Options.colorNotFound );
                        return (LRESULT) hTextNotFoundBrush;

                    case 0x7FFE:
                        SetBkColor( (HDC) wParam, g_Options.colorEOF );
                        return (LRESULT) hTextEOFBrush;

                    case 0x7FFD:
                        SetBkColor( (HDC) wParam, g_Options.colorNotRegExp );
                        return (LRESULT) hTextNotRegExpBrush;

                    default:
                        SetBkColor( (HDC) wParam, GetSysColor(nBkColor) );
                        return (LRESULT) GetSysColorBrush(nBkColor);
                }
            }
            break;
        }
        case WM_SHOWWINDOW:
        {
            if ( wParam )
            {
                HWND hChHighlightAll;

                g_bHighlightPlugin = isHighlightMainActive();

                hChHighlightAll = GetDlgItem(hDlg, IDC_CH_HIGHLIGHTALL);
                EnableWindow( hChHighlightAll, g_bHighlightPlugin );
                SendMessage( hChHighlightAll, BM_SETCHECK, 
                  (((g_Options.dwHighlightState & HLS_IS_CHECKED) == HLS_IS_CHECKED) ? 
                       BST_CHECKED : BST_UNCHECKED), 0 );

                qs_dwHotKey = getQSearchHotKey();
                qs_bHotKeyPressedOnShow = isQSearchHotKeyPressed();

                if ( g_QSearchDlg.uWmShowFlags & QS_SF_CANPICKUPSELTEXT )
                {
                    if ( g_Options.dwFlags[OPTF_SRCH_PICKUP_SELECTION] & 0x01 )
                    {
                        if ( getAkelPadSelectedText(g_QSearchDlg.szFindTextW) )
                        {
                            if ( g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE] )
                            {
                                qs_bForceFindFirst = FALSE;
                                qsearchDoTryHighlightAll(hDlg);
                            }
                            else
                                qs_bForceFindFirst = TRUE;
                        }
                    }
                }

                if ( g_Plugin.nDockedDlgIsWaitingForOnStart == 0 )
                {
                    qs_bEditIsActive = TRUE;
                    SetFocus(hFindEdit);
                }
                else
                {
                    qs_bEditIsActive = FALSE;
                    SetFocus(g_Plugin.hMainWnd);
                }
            }
            break;
        }
        case WM_SIZE:
        {
            if ( wParam != SIZE_MINIMIZED )
            {
                if ( g_QSearchDlg.pDockData )
                {
                    g_QSearchDlg.pDockData->rcDragDrop.left = 0;
                    g_QSearchDlg.pDockData->rcDragDrop.top = 0;
                    g_QSearchDlg.pDockData->rcDragDrop.right = LOWORD(lParam);
                    g_QSearchDlg.pDockData->rcDragDrop.bottom = HIWORD(lParam);
                }
                return 1;
            }
            break;
        }
        case WM_CONTEXTMENU:
        {
            int   i;
            UINT  uCheck;
            POINT pt;

            // the cursor position can be negative (!) in case of 2nd monitor
            // pt.x = (int) (short) LOWORD(lParam);
            // pt.y = (int) (short) HIWORD(lParam);
            GetCursorPos(&pt);

            for ( i = 0; i < OPTF_COUNT - 1; i++ )
            {
                switch ( i )
                {
                    case OPTF_SRCH_PICKUP_SELECTION:
                    case OPTF_SRCH_STOP_EOF:
                    case OPTF_CATCH_MAIN_ESC:
                    case OPTF_EDITOR_AUTOFOCUS:
                        uCheck = ((g_Options.dwFlags[i] & 0x01) ? MF_CHECKED : MF_UNCHECKED);
                        break;

                    default:
                        uCheck = (g_Options.dwFlags[i] ? MF_CHECKED : MF_UNCHECKED);
                        break;
                }
                CheckMenuItem( hPopupMenu, IDM_START + i, MF_BYCOMMAND | uCheck );
            }

            if ( g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] )
                uCheck = MF_BYCOMMAND | MF_ENABLED;
            else
                uCheck = MF_BYCOMMAND | MF_GRAYED;
            EnableMenuItem( hPopupMenu, IDM_SRCHREGEXPDOTNEWLINE, uCheck );

            TrackPopupMenuEx(hPopupMenu, 0, pt.x, pt.y, hDlg, NULL);

            break;
        }
        case QSM_FINDNEXT:
        {
            DWORD dwSearch;

            if ( (g_Options.dwFlags[OPTF_SRCH_PICKUP_SELECTION] & 0x01) && 
                 (g_QSearchDlg.uSearchOrigin == QS_SO_EDITOR) && 
                 (!(lParam & QS_FF_NOPICKUPSEL)) )
            {
                getEditFindText( hFindEdit, g_QSearchDlg.szFindTextW );
                qsearchFindHistoryAdd( hFindEdit, g_QSearchDlg.szFindTextW, 0 );
                qsPickUpSelection( hFindEdit );
            }
            getEditFindText( hFindEdit, g_QSearchDlg.szFindTextW );
            if ( qs_bForceFindFirst )
            {
                qs_bForceFindFirst = FALSE;
                dwSearch = QSEARCH_FIRST;
                if ( wParam )
                    dwSearch |= QSEARCH_FINDUP;
                qsearchDoSearchText( hFindEdit, dwSearch );
                qsearchDoTryHighlightAll( hDlg );
            }
            else
            {
                dwSearch = QSEARCH_NEXT;
                if ( wParam )
                    dwSearch |= QSEARCH_FINDUP;
                qsearchDoSearchText( hFindEdit, dwSearch );
            }
            return 1;
        }
        case QSM_FINDFIRST:
        {
            DWORD dwSearch = QSEARCH_FIRST;

            if ( wParam & QSEARCH_NOFINDUP )
                dwSearch |= QSEARCH_NOFINDUP;

            if ( wParam & QSEARCH_NOFINDBEGIN )
                dwSearch |= QSEARCH_NOFINDBEGIN;

            getEditFindText( hFindEdit, g_QSearchDlg.szFindTextW );
            qsearchDoSearchText( hFindEdit, dwSearch );
            qsearchDoTryHighlightAll( hDlg );
            return 1;
        }
        case QSM_SELFIND:
        {
            qsearchDoSelFind( hFindEdit, (BOOL) wParam );
            return 1;
        }
        case QSM_PICKUPSELTEXT:
        {
            BOOL bPickedUp = qsPickUpSelection(hFindEdit);
            if ( lParam )
                *((BOOL *)lParam) = bPickedUp;
            return 1;
        }
        case QSM_GETHWNDEDIT:
        {
            if ( lParam )
                *((HWND *)lParam) = hFindEdit;
            return 1;
        }
        case QSM_GETHWNDCOMBO:
        {
            if ( lParam )
                *((HWND *)lParam) = GetDlgItem(hDlg, IDC_CB_FINDTEXT);
            return 1;
        }
        case QSM_SHOWHIDE:
        {
            qsearchDoShowHide( hDlg, (BOOL) wParam, (UINT) lParam );
            return 1;
        }
        case QSM_SETNOTFOUND:
        {
            qsearchDoSetNotFound( hFindEdit, (BOOL) wParam, FALSE, FALSE );
            return 1;
        }
        case QSM_CHECKHIGHLIGHT:
        {
            HWND hChHighlightAll;

            g_bHighlightPlugin = isHighlightMainActive();

            hChHighlightAll = GetDlgItem(hDlg, IDC_CH_HIGHLIGHTALL);
            EnableWindow( hChHighlightAll, g_bHighlightPlugin );
            SendMessage( hChHighlightAll, BM_SETCHECK, 
              (((g_Options.dwHighlightState & HLS_IS_CHECKED) == HLS_IS_CHECKED) ? 
                   BST_CHECKED : BST_UNCHECKED), 0 );

            return 1;
        }
        case QSM_UPDATEUI:
        {
            HWND hFocusedWnd = GetFocus();

            if ( wParam & QS_UU_WHOLEWORD )
            {
                // show/hide whole word check-box
                qsdlgShowHideWholeWordCheckBox(hDlg);
            }

            if ( wParam & QS_UU_FIND )
            {
                // set edit or combo-box find control
                BOOL isFindEditFocused = FALSE;

                if ( hFocusedWnd )
                {
                    if ( hFocusedWnd == hFindEdit )
                        isFindEditFocused = TRUE;
                }

                if ( qsearchIsFindHistoryEnabled() )
                {
                    // hide edit control
                    ShowWindow( GetDlgItem(hDlg, IDC_ED_FINDTEXT), SW_HIDE );
                    // show combo-box control
                    ShowWindow( GetDlgItem(hDlg, IDC_CB_FINDTEXT), SW_SHOWNORMAL );
                }
                else
                {
                    // hide combo-box control
                    ShowWindow( GetDlgItem(hDlg, IDC_CB_FINDTEXT), SW_HIDE );
                    // show edit control
                    ShowWindow( GetDlgItem(hDlg, IDC_ED_FINDTEXT), SW_SHOWNORMAL );
                }

                // update hFindEdit
                hFindEdit = qsearchGetFindEdit(hDlg);
                g_QSearchDlg.hFindEdit = hFindEdit;
                // set focus if needed
                if ( isFindEditFocused )
                    SetFocus(hFindEdit);
            }

            return 1;
        }
        case QSM_QUIT:
        {
            qsearchDoQuit( hFindEdit, hToolTip, hPopupMenuLoaded, hTextNotFoundBrush, hTextNotRegExpBrush, hTextEOFBrush );
            return 1;
        }
        case QSN_DLGSWITCH:
        {
            g_QSearchDlg.uSearchOrigin = QS_SO_QSEARCH;
            qs_bEditTextChanged = TRUE;
            qs_bForceFindFirst = TRUE;
            //getEditFindText( hFindEdit, g_QSearchDlg.szFindTextW );
            //if ( g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE] )
            //{
            //    qsearchDoTryHighlightAll( hDlg );
            //}
            return 1;
        }
        case WM_INITDIALOG:
        {
            HWND hDlgItm;

            g_QSearchDlg.bIsQSearchingRightNow = FALSE;
            g_QSearchDlg.bMouseJustLeavedFindEdit = FALSE;
            g_QSearchDlg.hDlg = hDlg;
            g_QSearchDlg.hFindEdit = qsearchGetFindEdit(hDlg);
            hFindEdit = g_QSearchDlg.hFindEdit;

            hDlgItm = GetDlgItem(hDlg, IDC_CH_MATCHCASE);
            if ( hDlgItm )
            {
                SendMessage( hDlgItm, BM_SETCHECK, 
                  g_QSearchDlg.bMatchCase ? BST_CHECKED : BST_UNCHECKED, 0 );
            }

            // restoring initial values of static vars
            qs_bEditIsActive = TRUE;
            qs_bEditCanBeNonActive = TRUE;
            qs_bEditSelJustChanged = FALSE;
            qs_bEditNotFound = FALSE;
            qs_bEditNotRegExp = FALSE;
            qs_bEditIsEOF = FALSE;
            qs_nEditEOF = 0;
            qs_bEditTextChanged = TRUE;
            qs_bHotKeyPressedOnShow = FALSE;
            qs_bBtnFindIsFocused = FALSE;
            qs_dwHotKey = 0;

            hTextNotFoundBrush = CreateSolidBrush(g_Options.colorNotFound);
            hTextNotRegExpBrush = CreateSolidBrush(g_Options.colorNotRegExp);
            if ( g_Options.colorEOF != RGB(0xFF,0xFF,0xFF) )
            {
                hTextEOFBrush = CreateSolidBrush(g_Options.colorEOF);
            }

            if ( qsearchIsFindHistoryEnabled() )
                hDlgItm = GetDlgItem(hDlg, IDC_CB_FINDTEXT);
            else
                hDlgItm = GetDlgItem(hDlg, IDC_ED_FINDTEXT);
            ShowWindow(hDlgItm, SW_SHOWNORMAL);

            SendMessage(hFindEdit, EM_SETLIMITTEXT, MAX_TEXT_SIZE - 1, 0);
            prev_editWndProc = setWndProc(hFindEdit, editWndProc);
            hDlgItm = GetDlgItem(hDlg, IDC_BT_CANCEL);
            if ( hDlgItm )
            {
                BUTTONDRAW bd;

                bd.dwFlags = BIF_CROSS;
                bd.hImage = NULL;
                bd.nImageWidth = 0;
                bd.nImageHeight = 0;
                SendMessage(g_Plugin.hMainWnd, AKD_SETBUTTONDRAW, (WPARAM)hDlgItm, (LPARAM)&bd);

                /* prev_btnWndProc = setWndProc(hDlgItm, btnWndProc); */
            }
            /*hDlgItm = GetDlgItem(hDlg, IDC_CH_MATCHCASE);
            if ( hDlgItm )
            {
                prev_chWndProc = setWndProc(hDlgItm, chWndProc);
            }*/

            g_QSearchDlg.hBtnFindNext = GetDlgItem(hDlg, IDC_BT_FINDNEXT);
            if ( g_QSearchDlg.hBtnFindNext )
            {
                prev_btnFindNextWndProc = setWndProc(g_QSearchDlg.hBtnFindNext, btnFindWndProc);
            }
            g_QSearchDlg.hBtnFindPrev = GetDlgItem(hDlg, IDC_BT_FINDPREV);
            if ( g_QSearchDlg.hBtnFindPrev )
            {
                prev_btnFindPrevWndProc = setWndProc(g_QSearchDlg.hBtnFindPrev, btnFindWndProc);
            }

            hToolTip = qsearchDoInitToolTip(hDlg, hFindEdit);
            if ( g_Plugin.bOldWindows )
            {
                hPopupMenuLoaded = LoadMenuA( g_Plugin.hInstanceDLL, 
                  MAKEINTRESOURCEA(IDR_MENU_OPTIONS) );
            }
            else
            {
                hPopupMenuLoaded = LoadMenuW( g_Plugin.hInstanceDLL, 
                  MAKEINTRESOURCEW(IDR_MENU_OPTIONS) );
            }
            hPopupMenu = GetSubMenu(hPopupMenuLoaded, 0);
            qsearchSetPopupMenuLang(hPopupMenu);
            qsearchSetDialogLang(hDlg);

            qsdlgShowHideWholeWordCheckBox(hDlg);

            if ( g_Plugin.bOldWindows )
            {
                ReadFindHistoryA();
            }
            else
            {
                ReadFindHistoryW();
            }

            if ( g_Options.dwFlags[OPTF_SRCH_PICKUP_SELECTION] & 0x01 )
            {
                getAkelPadSelectedText(g_QSearchDlg.szFindTextW);
            }
            setEditFindText(hFindEdit, g_QSearchDlg.szFindTextW);
            //SetFocus(hFindEdit);
            if ( !g_Options.dwFlags[OPTF_EDIT_FOCUS_SELECTALL] )
            {
                SendMessage(hFindEdit, EM_SETSEL, 0, -1);
#ifdef _DEBUG
                Debug_Output("qsearchDlgProc, WM_INITDIALOG, if (!SelectAll), SETSEL(0, -1)\n");
#endif
            }

            break;
        }
        default:
            break;
    }
    return 0;
}

static void fillToolInfoA(
  TOOLINFOA* lptiA,
  LPSTR      lpToolTipText,
  HWND       hWnd)
{
    static unsigned int uId = 0;
    RECT rect;

    GetClientRect(hWnd, &rect);

    lptiA->cbSize = sizeof(TOOLINFOA);
    lptiA->uFlags = TTF_SUBCLASS;
    lptiA->hwnd = hWnd;
    lptiA->hinst = g_Plugin.hInstanceDLL;
    lptiA->uId = uId;
    lptiA->lpszText = lpToolTipText;
    // ToolTip control will cover the whole window
    lptiA->rect.left = rect.left;
    lptiA->rect.top = rect.top;
    lptiA->rect.right = rect.right;
    lptiA->rect.bottom = rect.bottom;
    lptiA->lParam = 0;

    ++uId;
}

static void fillToolInfoW(
  TOOLINFOW* lptiW,
  LPWSTR     lpToolTipText,
  HWND       hWnd)
{
    static unsigned int uId = 0;
    RECT rect;

    GetClientRect(hWnd, &rect);

    lptiW->cbSize = sizeof(TOOLINFOW);
    lptiW->uFlags = TTF_SUBCLASS;
    lptiW->hwnd = hWnd;
    lptiW->hinst = g_Plugin.hInstanceDLL;
    lptiW->uId = uId;
    lptiW->lpszText = lpToolTipText;
    // ToolTip control will cover the whole window
    lptiW->rect.left = rect.left;
    lptiW->rect.top = rect.top;
    lptiW->rect.right = rect.right;
    lptiW->rect.bottom = rect.bottom;
    lptiW->lParam = 0;

    ++uId;
}

HWND qsearchDoInitToolTip(HWND hDlg, HWND hEdit)
{
    INITCOMMONCONTROLSEX iccex;
    HWND                 hToolTip = NULL;

    iccex.dwICC = ICC_WIN95_CLASSES;
    iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    InitCommonControlsEx( &iccex );

    if ( g_Plugin.bOldWindows )
    {
        hToolTip = CreateWindowExA( WS_EX_TOPMOST, TOOLTIPS_CLASSA, 
          0, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, 
          CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
          hDlg, 0, g_Plugin.hInstanceDLL, 0 );
    }
    else
    {
        hToolTip = CreateWindowExW( WS_EX_TOPMOST, TOOLTIPS_CLASSW, 
          0, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, 
          CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
          hDlg, 0, g_Plugin.hInstanceDLL, 0 );

    }

    if ( hToolTip )
    {
        SetWindowPos( hToolTip, HWND_TOPMOST,
          0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

        if ( g_Plugin.bOldWindows )
        {
            TOOLINFOA tiA;

            fillToolInfoA( &tiA, LPSTR_TEXTCALLBACKA, hEdit );
            // LPSTR_TEXTCALLBACKA means "send TTN_GETDISPINFOA to hEdit"
            SendMessage( hToolTip, TTM_ADDTOOLA, 0, (LPARAM) &tiA );

            fillToolInfoA( &tiA, (LPSTR) qsearchGetHintA(IDC_BT_CANCEL), 
              GetDlgItem(hDlg, IDC_BT_CANCEL) );
            SendMessage( hToolTip, TTM_ADDTOOLA, 0, (LPARAM) &tiA );

            fillToolInfoA( &tiA, (LPSTR) qsearchGetHintA(IDC_CH_MATCHCASE), 
              GetDlgItem(hDlg, IDC_CH_MATCHCASE) );
            SendMessage( hToolTip, TTM_ADDTOOLA, 0, (LPARAM) &tiA );

            fillToolInfoA( &tiA, (LPSTR) qsearchGetHintA(IDC_CH_WHOLEWORD), 
              GetDlgItem(hDlg, IDC_CH_WHOLEWORD) );
            SendMessage( hToolTip, TTM_ADDTOOLA, 0, (LPARAM) &tiA );

            fillToolInfoA( &tiA, (LPSTR) qsearchGetHintA(IDC_CH_HIGHLIGHTALL), 
              GetDlgItem(hDlg, IDC_CH_HIGHLIGHTALL) );
            SendMessage( hToolTip, TTM_ADDTOOLA, 0, (LPARAM) &tiA );

            if ( g_Options.dwNewUI == QS_UI_NEW_01 )
            {
                fillToolInfoA( &tiA, (LPSTR) qsearchGetHintA(IDC_BT_FINDNEXT), 
                  GetDlgItem(hDlg, IDC_BT_FINDNEXT) );
                SendMessage( hToolTip, TTM_ADDTOOLA, 0, (LPARAM) &tiA );

                fillToolInfoA( &tiA, (LPSTR) qsearchGetHintA(IDC_BT_FINDPREV), 
                  GetDlgItem(hDlg, IDC_BT_FINDPREV) );
                SendMessage( hToolTip, TTM_ADDTOOLA, 0, (LPARAM) &tiA );
            }
        }
        else
        {
            TOOLINFOW tiW;

            fillToolInfoW( &tiW, LPSTR_TEXTCALLBACKW, hEdit );
            // LPSTR_TEXTCALLBACKW means "send TTN_GETDISPINFOW to hEdit"
            SendMessageW( hToolTip, TTM_ADDTOOLW, 0, (LPARAM) &tiW );

            fillToolInfoW( &tiW, (LPWSTR) qsearchGetHintW(IDC_BT_CANCEL), 
              GetDlgItem(hDlg, IDC_BT_CANCEL) );
            SendMessageW( hToolTip, TTM_ADDTOOLW, 0, (LPARAM) &tiW );

            fillToolInfoW( &tiW, (LPWSTR) qsearchGetHintW(IDC_CH_MATCHCASE), 
              GetDlgItem(hDlg, IDC_CH_MATCHCASE) );
            SendMessageW( hToolTip, TTM_ADDTOOLW, 0, (LPARAM) &tiW );

            fillToolInfoW( &tiW, (LPWSTR) qsearchGetHintW(IDC_CH_WHOLEWORD), 
              GetDlgItem(hDlg, IDC_CH_WHOLEWORD) );
            SendMessageW( hToolTip, TTM_ADDTOOLW, 0, (LPARAM) &tiW );

            fillToolInfoW( &tiW, (LPWSTR) qsearchGetHintW(IDC_CH_HIGHLIGHTALL), 
              GetDlgItem(hDlg, IDC_CH_HIGHLIGHTALL) );
            SendMessageW( hToolTip, TTM_ADDTOOLW, 0, (LPARAM) &tiW );

            if ( g_Options.dwNewUI == QS_UI_NEW_01 )
            {
                fillToolInfoW( &tiW, (LPWSTR) qsearchGetHintW(IDC_BT_FINDNEXT), 
                  GetDlgItem(hDlg, IDC_BT_FINDNEXT) );
                SendMessageW( hToolTip, TTM_ADDTOOLW, 0, (LPARAM) &tiW );

                fillToolInfoW( &tiW, (LPWSTR) qsearchGetHintW(IDC_BT_FINDPREV), 
                  GetDlgItem(hDlg, IDC_BT_FINDPREV) );
                SendMessageW( hToolTip, TTM_ADDTOOLW, 0, (LPARAM) &tiW );
            }
        }
    }

    return hToolTip;
}

void qsearchDoQuit(HWND hEdit, HWND hToolTip, HMENU hPopupMenuLoaded, HBRUSH hBrush1, HBRUSH hBrush2, HBRUSH hBrush3)
{
    HWND hDlgItm;

    if ( g_Plugin.bOldWindows )
    {
        SaveFindHistoryA();
    }
    else
    {
        SaveFindHistoryW();
    }

    hDlgItm = GetDlgItem(g_QSearchDlg.hDlg, IDC_CH_MATCHCASE);
    if ( hDlgItm )
    {
        if ( SendMessage(hDlgItm, BM_GETCHECK, 0, 0) == BST_CHECKED )
            g_QSearchDlg.bMatchCase = TRUE;
        else
            g_QSearchDlg.bMatchCase = FALSE;
    }

    getEditFindText(hEdit, g_QSearchDlg.szFindTextW);

    if ( prev_editWndProc )
    {
        setWndProc(hEdit, prev_editWndProc);
        prev_editWndProc = NULL;
    }
    /* if ( prev_btnWndProc )
    {
        hDlgItm = GetDlgItem(g_QSearchDlg.hDlg, IDC_BT_CANCEL);
        setWndProc(hDlgItm, prev_btnWndProc);
        prev_btnWndProc = NULL;
    } */
    /*if ( prev_chWndProc )
    {
        hDlgItm = GetDlgItem(g_QSearchDlg.hDlg, IDC_CH_MATCHCASE);
        setWndProc(hDlgItm, prev_chWndProc);
        prev_chWndProc = NULL;
    }*/
    if ( prev_btnFindNextWndProc )
    {
        setWndProc(g_QSearchDlg.hBtnFindNext, prev_btnFindNextWndProc);
        prev_btnFindNextWndProc = NULL;
    }
    if ( prev_btnFindPrevWndProc )
    {
        setWndProc(g_QSearchDlg.hBtnFindPrev, prev_btnFindPrevWndProc);
        prev_btnFindPrevWndProc = NULL;
    }
    if ( hToolTip )
    {
        DestroyWindow( hToolTip );
    }
    if ( hBrush1 )
    {
        DeleteObject(hBrush1);
    }
    if ( hBrush2 )
    {
        DeleteObject(hBrush2);
    }
    if ( hBrush3 )
    {
        DeleteObject(hBrush3);
    }
    if ( hPopupMenuLoaded )
    {
        DestroyMenu(hPopupMenuLoaded);
    }
    DestroyWindow( g_QSearchDlg.hDlg );
    g_QSearchDlg.hDlg = NULL;

    if ( !g_Plugin.bAkelPadOnFinish )
    {
        Uninitialize();
        SendMessage(g_Plugin.hMainWnd, AKD_RESIZE, 0, 0);
        SetFocus(g_Plugin.hMainWnd);
    }
}

void qsearchDoSetNotFound(HWND hEdit, BOOL bNotFound, BOOL bNotRegExp, BOOL bEOF)
{
    if ( bNotFound )
        qsearchDoTryUnhighlightAll();

    qs_bEditNotFound = bNotFound;
    qs_bEditNotRegExp = bNotRegExp;
    qs_bEditIsEOF = bEOF;
    InvalidateRect(hEdit, NULL, TRUE);
    UpdateWindow(hEdit);
}

void qsearchDoShowHide(HWND hDlg, BOOL bShow, UINT uShowFlags)
{
    BOOL bChangeSelection = !IsWindowVisible(hDlg);

    qsearchDoSetNotFound( qsearchGetFindEdit(hDlg), FALSE, FALSE, FALSE );

    if ( bShow )
    {
        HWND hCloseBtn = GetDlgItem(hDlg, IDC_BT_CANCEL);

        SendMessage(hCloseBtn, BM_SETSTATE, FALSE, 0);
        SendMessage( 
            hCloseBtn, 
            BM_SETSTYLE, 
            WS_VISIBLE | WS_CHILD | BS_FLAT | BS_OWNERDRAW | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,
            TRUE );

        /*
        // The following line is commented because 
        // the hot key is re-read on WM_SHOWWINDOW.
        qs_dwHotKey = getQSearchHotKey();
        */
        qs_bHotKeyPressedOnShow = isQSearchHotKeyPressed();
    }
    else
    {
        qs_nEditEOF = 0;
    }

    g_QSearchDlg.uWmShowFlags = 0; // forbid to pick up selected text on WM_SHOWWINDOW
    SendMessage( g_Plugin.hMainWnd, AKD_DOCK,
      (bShow ? DK_SHOW : DK_HIDE), (LPARAM) g_QSearchDlg.pDockData );
    g_QSearchDlg.uWmShowFlags = 0; // just in case :)

    // Change AkelPad's plugin status (running/not running)
    if ( g_szFunctionQSearchW[0] )
    {
        if ( g_Plugin.bOldWindows )
        {
            PLUGINFUNCTION* pfA;

            pfA = (PLUGINFUNCTION *) SendMessage( g_Plugin.hMainWnd, 
              AKD_DLLFINDA, (WPARAM) g_szFunctionQSearchW, 0 );
            if ( pfA )
            {
                pfA->bRunning = bShow;
            }
        }
        else
        {
            PLUGINFUNCTION* pfW;

            pfW = (PLUGINFUNCTION *) SendMessage( g_Plugin.hMainWnd, 
              AKD_DLLFINDW, (WPARAM) g_szFunctionQSearchW, 0 );
            if ( pfW )
            {
                pfW->bRunning = bShow;
            }
        }

        // Send "empty" command to AkelPad to update the Toolbar plugin state
        SendMessage(g_Plugin.hMainWnd, WM_COMMAND, 0, 0);
    }

    if ( bShow )
    {
        BOOL bGotSelectedText = FALSE;
        HWND hEdit = qsearchGetFindEdit(hDlg);

        if ( uShowFlags & QS_SF_CANPICKUPSELTEXT )
        {
            if ( g_Options.dwFlags[OPTF_SRCH_PICKUP_SELECTION] & 0x01 )
            {
                bGotSelectedText = getAkelPadSelectedText(g_QSearchDlg.szFindTextW);
                if ( bGotSelectedText )
                {
                    bChangeSelection = TRUE;
                }
                else
                {
                    getEditFindText(hEdit, g_QSearchDlg.szFindTextW);
                }
                setEditFindText(hEdit, g_QSearchDlg.szFindTextW);
                if ( (!bChangeSelection) || g_Options.dwFlags[OPTF_EDIT_FOCUS_SELECTALL] )
                {
                    SendMessage(hEdit, EM_SETSEL, 0, -1);
    #ifdef _DEBUG
                    Debug_Output("qsearchDoShowHide, PickUpSel, SETSEL(0, -1)\n");
    #endif
                    qs_bEditSelJustChanged = TRUE;
                }
            }
        }
        if ( bChangeSelection && !g_Options.dwFlags[OPTF_EDIT_FOCUS_SELECTALL] )
        {
            SendMessage(hEdit, EM_SETSEL, 0, -1);
#ifdef _DEBUG
            Debug_Output("qsearchDoShowHide, ChangeSel, SETSEL(0, -1)\n");
#endif
            qs_bEditSelJustChanged = TRUE;
        }
        if ( bGotSelectedText )
        {
            if ( g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE] )
            {
                qs_bForceFindFirst = FALSE;
                qsearchDoTryHighlightAll(g_QSearchDlg.hDlg);
            }
            else
                qs_bForceFindFirst = TRUE;
        }
        SetFocus(hEdit);
    }
    else
    {
        //SendMessage(g_Plugin.hMainWnd, AKD_RESIZE, 0, 0);
        SetFocus(g_Plugin.hMainWnd);
    }
}

void qsearchDoSelFind(HWND hEdit, BOOL bFindPrev)
{
    wchar_t prevFindTextW[MAX_TEXT_SIZE];
    DWORD   prevSrchFromBeginning;
    DWORD   prevSrchUseSpecialChars;
    DWORD   prevSrchUseRegExp;

    if ( g_Plugin.bOldWindows )
    {
        lstrcpyA( (LPSTR) prevFindTextW, (LPCSTR) g_QSearchDlg.szFindTextW );
    }
    else
    {
        lstrcpyW( (LPWSTR) prevFindTextW, (LPCWSTR) g_QSearchDlg.szFindTextW );
    }

    if ( g_Options.dwFlags[OPTF_SRCH_SELFIND_PICKUP] )
    {
        // getting selected text with current search flags
        if ( getAkelPadSelectedText(g_QSearchDlg.szFindTextW) )
        {
            setEditFindText(hEdit, g_QSearchDlg.szFindTextW);
        }
    }

    // saving search flags
    prevSrchFromBeginning = g_Options.dwFlags[OPTF_SRCH_FROM_BEGINNING];
    prevSrchUseSpecialChars = g_Options.dwFlags[OPTF_SRCH_USE_SPECIALCHARS];
    prevSrchUseRegExp = g_Options.dwFlags[OPTF_SRCH_USE_REGEXP];

    // these search flags must be disabled here
    g_Options.dwFlags[OPTF_SRCH_FROM_BEGINNING] = 0;
    g_Options.dwFlags[OPTF_SRCH_USE_SPECIALCHARS] = 0;
    g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] = 0;

    // getting selected text with modified search flags
    if ( getAkelPadSelectedText(g_QSearchDlg.szFindTextW) )
    {
        DWORD   dwSearchParams;
        BOOL    bMatchCase;
        HWND    hDlgItm;

        bMatchCase = FALSE;
        if ( hDlgItm = GetDlgItem(g_QSearchDlg.hDlg, IDC_CH_MATCHCASE) )
        {
            if ( SendMessage(hDlgItm, BM_GETCHECK, 0, 0) == BST_CHECKED )
                bMatchCase = TRUE;
        }

        // clear the "not found" flag
        qs_bEditNotFound = FALSE;
        qs_bEditNotRegExp = FALSE;
        qs_bEditIsEOF = FALSE;

        if ( g_Plugin.bOldWindows )
        {
            int (WINAPI *cmpfuncA)(LPCSTR, LPCSTR) = bMatchCase ? lstrcmpA : lstrcmpiA;
            if ( cmpfuncA((LPCSTR) prevFindTextW, (LPCSTR) g_QSearchDlg.szFindTextW) != 0 )
            {
                qs_bEditTextChanged = TRUE;
                qs_nEditEOF = 0;
            }
        }
        else
        {
            int (WINAPI *cmpfuncW)(LPCWSTR, LPCWSTR) = bMatchCase ? lstrcmpW : lstrcmpiW;
            if ( cmpfuncW((LPCWSTR) prevFindTextW, (LPCWSTR) g_QSearchDlg.szFindTextW) != 0 )
            {
                qs_bEditTextChanged = TRUE;
                qs_nEditEOF = 0;
            }
        }

        dwSearchParams = QSEARCH_NEXT | QSEARCH_SEL;
        if ( bFindPrev )  dwSearchParams |= QSEARCH_SEL_FINDUP;
        qsearchDoSearchText(hEdit, dwSearchParams);
    }

    // restoring search flags
    g_Options.dwFlags[OPTF_SRCH_FROM_BEGINNING] = prevSrchFromBeginning;
    g_Options.dwFlags[OPTF_SRCH_USE_SPECIALCHARS] = prevSrchUseSpecialChars;
    g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] = prevSrchUseRegExp;
}

static void adjustIncompleteRegExA(char* szTextA)
{
    int n1, n2;

    if ( g_Options.dwAdjIncomplRegExp == 0 )
        return;

    if ( g_Options.dwAdjIncomplRegExp == 2 )
    {
        if ( g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE] == 0 )
            return;
    }

    if ( !szTextA )
        return; // NULL, nothing to do

    n1 = lstrlenA(szTextA);
    if ( n1 == 0 )
        return; // just "", nothing to do

    n2 = n1;
    // skipping trailing '?'
    while ( szTextA[n1 - 1] == '?' )
    {
        --n1;
        if ( n2 - n1 > 1 )
            return; // syntax error: "??"

        if ( n1 == 0 )
            return; // just "?", nothing to do
    }

    n2 = n1;
    // skipping trailing '+' or '*'
    while ( (szTextA[n1 - 1] == '+') || (szTextA[n1 - 1] == '*') )
    {
        --n1;
        if ( n2 - n1 > 1 )
            return; // syntax error: "++", "**", "+*" or "*+"

        if ( n1 == 0 )
            return; // just "+(?)" or "*(?)", nothing to do
    }

    if ( n1 == n2 )
        return; // no '+' or '*' found, nothing to do

    n2 = n1;
    // skipping '\' before the trailing '+' or '*'
    while ( (n2 > 0) && (szTextA[n2 - 1] == '\\') )
    {
        --n2;
    }

    if ( ((n1 - n2) % 2) != 0 )
        return; // '+' or '*' is escaped by '\', nothing to do

    // exclude trailing '+' or '*'
    szTextA[n1] = 0;
}

static void adjustIncompleteRegExW(wchar_t* szTextW)
{
    int n1, n2;

    if ( g_Options.dwAdjIncomplRegExp == 0 )
        return;

    if ( g_Options.dwAdjIncomplRegExp == 2 )
    {
        if ( g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE] == 0 )
            return;
    }

    if ( !szTextW )
        return; // NULL, nothing to do

    n1 = lstrlenW(szTextW);
    if ( n1 == 0 )
        return; // just "", nothing to do

    n2 = n1;
    // skipping trailing '?'
    while ( szTextW[n1 - 1] == L'?' )
    {
        --n1;
        if ( n2 - n1 > 1 )
            return; // syntax error: "??"

        if ( n1 == 0 )
            return; // just "?", nothing to do
    }

    n2 = n1;
    // skipping trailing '+' or '*'
    while ( (szTextW[n1 - 1] == L'+') || (szTextW[n1 - 1] == L'*') )
    {
        --n1;
        if ( n2 - n1 > 1 )
            return; // syntax error: "++", "**", "+*" or "*+"

        if ( n1 == 0 )
            return; // just "+(?)" or "*(?)", nothing to do
    }

    if ( n1 == n2 )
        return; // no '+' or '*' found, nothing to do

    n2 = n1;
    // skipping '\' before the trailing '+' or '*'
    while ( (n2 > 0) && (szTextW[n2 - 1] == L'\\') )
    {
        --n2;
    }

    if ( ((n1 - n2) % 2) != 0 )
        return; // '+' or '*' is escaped by '\', nothing to do

    // exclude trailing '+' or '*'
    szTextW[n1] = 0;
}

// searches for g_QSearchDlg.szFindTextW
void qsearchDoSearchText(HWND hEdit, DWORD dwParams)
{
    EDITINFO ei;
    HWND     hDlgItm;
    BOOL     bNotFound = FALSE;
    BOOL     bNotRegExp = FALSE;
    BOOL     bEOF = FALSE;
    DWORD    dwSearchFlags = FR_DOWN;

    if ( dwParams & QSEARCH_FIRST )
    {
        // searching for the first time, clear the EOF flag
        qs_nEditEOF = 0;
    }
    else
    {
        // searching for the same text again
        if ( qs_bEditNotFound )
            if ( g_Options.dwFlags[OPTF_SRCH_STOP_EOF] != STOP_EOF_WITHOUT_MSG )
                return;
    }

    if ( g_Plugin.bOldWindows )
    {
        if ( 0 == ((LPCSTR) g_QSearchDlg.szFindTextW)[0] )
        {
            qsearchDoSetNotFound(hEdit, FALSE, FALSE, FALSE);
            return;
        }
    }
    else
    {
        if ( 0 == ((LPCWSTR) g_QSearchDlg.szFindTextW)[0] )
        {
            qsearchDoSetNotFound(hEdit, FALSE, FALSE, FALSE);
            return;
        }
    }

    ei.hWndEdit = 0;
    SendMessage( g_Plugin.hMainWnd, AKD_GETEDITINFO, 0, (LPARAM) &ei );
    if ( !ei.hWndEdit )
    {
        return;
    }

    g_QSearchDlg.bIsQSearchingRightNow = TRUE;

    if ( ((dwParams & QSEARCH_SEL) != QSEARCH_SEL) ||
         g_Options.dwFlags[OPTF_SRCH_SELFIND_PICKUP] )
    {
        // not SelFindNext or SelFindPrev
        // or srch_selfind_pickup = 1
        g_QSearchDlg.bQSearching = TRUE;
    }

    if ( (dwParams & QSEARCH_SEL_FINDUP) ||
         (dwParams & QSEARCH_FINDUP) ||
         (((dwParams & QSEARCH_SEL) != QSEARCH_SEL) &&
          ((dwParams & QSEARCH_NOFINDUP) != QSEARCH_NOFINDUP) &&
          ((GetKeyState(VK_QS_FINDUP) & 0x80) == 0x80)) )
    {
        dwSearchFlags = FR_UP;
    }

    if ( ((dwParams & QSEARCH_SEL) != QSEARCH_SEL) &&
         ((dwParams & QSEARCH_NOFINDBEGIN) != QSEARCH_NOFINDBEGIN) &&
         ((dwParams & QSEARCH_FINDBEGIN) ||
          ((GetKeyState(VK_QS_FINDBEGIN) & 0x80) == 0x80)) )
    {
        dwSearchFlags |= FR_BEGINNING;
        qs_nEditEOF = 0;
    }

    if ( hDlgItm = GetDlgItem(g_QSearchDlg.hDlg, IDC_CH_MATCHCASE) )
    {
        if ( SendMessage(hDlgItm, BM_GETCHECK, 0, 0) == BST_CHECKED )
            dwSearchFlags |= FR_MATCHCASE;
    }

    //if ( !g_Options.dwFlags[OPTF_SRCH_USE_SPECIALCHARS] )
    {
        if ( hDlgItm = GetDlgItem(g_QSearchDlg.hDlg, IDC_CH_WHOLEWORD) )
        {
            if ( SendMessage(hDlgItm, BM_GETCHECK, 0, 0) == BST_CHECKED )
                dwSearchFlags |= FR_WHOLEWORD;
        }
    }

    // this forbids deactivating of the Edit control
    // when the text is typed and searched
    qs_bEditCanBeNonActive = FALSE;

    if ( dwParams & QSEARCH_FIRST )
    {
        if ( g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE] )
        {
            CHARRANGE_X cr = {0, 0};

            SendMessage( ei.hWndEdit, EM_EXGETSEL_X, 0, (LPARAM) &cr );
            cr.cpMax = cr.cpMin;
            SendMessage( ei.hWndEdit, EM_EXSETSEL_X, 0, (LPARAM) &cr );
        }
        if ( g_Options.dwFlags[OPTF_SRCH_FROM_BEGINNING] )
            dwSearchFlags |= FR_BEGINNING;
    }

    if ( g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] )
    {
        dwSearchFlags |= FRF_REGEXP;
        if ( g_Options.dwFlags[OPTF_SRCH_REGEXP_DOT_NEWLINE] == 0 )
        {
            dwSearchFlags |= FRF_REGEXPNONEWLINEDOT;
        }
    }

    if ( g_Plugin.bOldWindows )
    {
        TEXTFINDA tfA;
        int       srchEOF;
        BOOL      bSearchEx = FALSE;
        INT_X     iFindResult = -1;
        char      szFindTextA[MAX_TEXT_SIZE];

        //g_QSearchDlg.szFindTextW[0] = 0;
        //GetWindowTextA( hEdit, (LPSTR) g_QSearchDlg.szFindTextW, MAX_TEXT_SIZE - 1 );
        lstrcpyA( szFindTextA, (LPCSTR) getTextToSearch(g_QSearchDlg.szFindTextW, &bSearchEx) );
        if ( g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] )
        {
            adjustIncompleteRegExA(szFindTextA);
        }
        tfA.dwFlags = dwSearchFlags;
        tfA.pFindIt = szFindTextA;
        tfA.nFindItLen = -1;
        srchEOF = (dwSearchFlags & FR_UP) ? QSEARCH_EOF_UP : QSEARCH_EOF_DOWN;
        if ( ((qs_nEditEOF & srchEOF) == 0) || !IsWindowVisible(g_QSearchDlg.hDlg) )
        {
            if ( !bSearchEx )
            {
                iFindResult = (INT_X) SendMessage( g_Plugin.hMainWnd, 
                  AKD_TEXTFINDA, (WPARAM) ei.hWndEdit, (LPARAM) &tfA );
            }
            else
            {
                iFindResult = doFindTextExA( ei.hWndEdit, &tfA ) ;
            }
        }
        if ( iFindResult < 0 )
        {
            BOOL bContinueSearch = FALSE;

            if ( (dwSearchFlags & FRF_REGEXP) && (iFindResult <= -100) )
            {
                // RegExp syntax error
                bNotRegExp = TRUE;
                bNotFound = TRUE;
                bEOF = TRUE;
                qs_nEditEOF = 0;
            }
            else if ( g_Options.dwFlags[OPTF_SRCH_STOP_EOF] == STOP_EOF_WITHOUT_MSG )
            {
                bNotFound = TRUE;
                bEOF = TRUE;
                qs_nEditEOF = srchEOF;
            }
            else if ( (g_Options.dwFlags[OPTF_SRCH_STOP_EOF] & 0x01) && 
                      ((qs_nEditEOF & srchEOF) == 0) && 
                      (dwParams & QSEARCH_NEXT) )
            {
                HWND hWndFocused = GetFocus();

                qs_nEditEOF = srchEOF;
                if ( MessageBoxA( 
                       g_Plugin.hMainWnd, 
                       qsearchGetHintA(IDS_EOFREACHED),
                       "AkelPad (QSearch)",
                       MB_OKCANCEL | MB_ICONQUESTION | MB_DEFBUTTON1
                       ) == IDOK )
                {
                    bContinueSearch = TRUE;
                }
                else
                {
                    // When a Find Button loses focus, the value of qs_nEditEOF resets to 0.
                    // So we need to set the value of qs_nEditEOF here again.
                    qs_nEditEOF = srchEOF; 
                }

                if ( IsWindowVisible(g_QSearchDlg.hDlg) && (hWndFocused != ei.hWndEdit) )
                    SetFocus(hEdit);
                else
                    SetFocus(ei.hWndEdit);
            }
            else
            {
                bContinueSearch = TRUE;
            }

            if ( bContinueSearch )
            {
                bEOF = TRUE;

                if ( (dwSearchFlags & FR_UP) == FR_UP )
                {
                    INT_X       pos = 0;
                    CHARRANGE_X cr = {0, 0};

                    SendMessage( ei.hWndEdit, EM_EXGETSEL_X, 0, (LPARAM) &cr );
                    pos = cr.cpMin;
                    cr.cpMin = -1;
                    cr.cpMax = -1;
                    SendMessage( ei.hWndEdit, EM_EXSETSEL_X, 0, (LPARAM) &cr );
                    if ( tfA.dwFlags & FR_BEGINNING )
                    {
                        tfA.dwFlags -= FR_BEGINNING;
                    }
                    if ( !bSearchEx )
                    {
                        iFindResult = (INT_X) SendMessage( g_Plugin.hMainWnd, 
                          AKD_TEXTFINDA, (WPARAM) ei.hWndEdit, (LPARAM) &tfA );
                    }
                    else
                    {
                        iFindResult = doFindTextExA( ei.hWndEdit, &tfA ) ;
                    }
                    if ( iFindResult < 0 )
                    {
                        cr.cpMin = pos;
                        cr.cpMax = pos;
                        SendMessage( ei.hWndEdit, EM_EXSETSEL_X, 0, (LPARAM) &cr );
                        bNotFound = TRUE;
                    }
                }
                else
                {
                    if ( ((dwSearchFlags & FR_BEGINNING) != FR_BEGINNING) ||
                         (qs_nEditEOF & srchEOF) )
                    {
                        tfA.dwFlags = dwSearchFlags | FR_BEGINNING;
                        if ( !bSearchEx )
                        {
                            iFindResult = (INT_X) SendMessage( g_Plugin.hMainWnd, 
                              AKD_TEXTFINDA, (WPARAM) ei.hWndEdit, (LPARAM) &tfA );
                        }
                        else
                        {
                            iFindResult = doFindTextExA( ei.hWndEdit, &tfA ) ;
                        }
                        if ( iFindResult < 0 )
                        {
                            bNotFound = TRUE;
                        }
                    }
                    else
                    {
                        bNotFound = TRUE;
                    }
                }
                qs_nEditEOF = 0;
            }
        }
        else
        {
            qs_nEditEOF = 0;
        }
    }
    else
    {
        TEXTFINDW tfW;
        int       srchEOF;
        BOOL      bSearchEx = FALSE;
        INT_X     iFindResult = -1;
        wchar_t   szFindTextW[MAX_TEXT_SIZE];

        //g_QSearchDlg.szFindTextW[0] = 0;
        //GetWindowTextW( hEdit, (LPWSTR) g_QSearchDlg.szFindTextW, MAX_TEXT_SIZE - 1 );
        lstrcpyW( szFindTextW, (LPCWSTR) getTextToSearch(g_QSearchDlg.szFindTextW, &bSearchEx) );
        if ( g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] )
        {
            adjustIncompleteRegExW(szFindTextW);
        }
        tfW.dwFlags = dwSearchFlags;
        tfW.pFindIt = szFindTextW;
        tfW.nFindItLen = -1;
        srchEOF = (dwSearchFlags & FR_UP) ? QSEARCH_EOF_UP : QSEARCH_EOF_DOWN;
        if ( ((qs_nEditEOF & srchEOF) == 0) || !IsWindowVisible(g_QSearchDlg.hDlg) )
        {
            if ( !bSearchEx )
            {
                iFindResult = (INT_X) SendMessage( g_Plugin.hMainWnd, 
                  AKD_TEXTFINDW, (WPARAM) ei.hWndEdit, (LPARAM) &tfW );
            }
            else
            {
                iFindResult = doFindTextExW( ei.hWndEdit, &tfW ) ;
            }
        }
        if ( iFindResult < 0 )
        {
            BOOL bContinueSearch = FALSE;

            if ( (dwSearchFlags & FRF_REGEXP) && (iFindResult <= -100) )
            {
                // RegExp syntax error
                bNotRegExp = TRUE;
                bNotFound = TRUE;
                bEOF = TRUE;
                qs_nEditEOF = 0;
            }
            else if ( g_Options.dwFlags[OPTF_SRCH_STOP_EOF] == STOP_EOF_WITHOUT_MSG )
            {
                bNotFound = TRUE;
                bEOF = TRUE;
                qs_nEditEOF = srchEOF;
            }
            else if ( (g_Options.dwFlags[OPTF_SRCH_STOP_EOF] & 0x01) && 
                      ((qs_nEditEOF & srchEOF) == 0) &&
                      (dwParams & QSEARCH_NEXT) )
            {
                HWND hWndFocused = GetFocus();

                qs_nEditEOF = srchEOF;
                if ( MessageBoxW( 
                       g_Plugin.hMainWnd, 
                       qsearchGetHintW(IDS_EOFREACHED),
                       L"AkelPad (QSearch)",
                       MB_OKCANCEL | MB_ICONQUESTION | MB_DEFBUTTON1
                       ) == IDOK )
                {
                    bContinueSearch = TRUE;
                }
                else
                {
                    // When a Find Button loses focus, the value of qs_nEditEOF resets to 0.
                    // So we need to set the value of qs_nEditEOF here again.
                    qs_nEditEOF = srchEOF;
                }

                if ( IsWindowVisible(g_QSearchDlg.hDlg) && (hWndFocused != ei.hWndEdit) )
                    SetFocus(hEdit);
                else
                    SetFocus(ei.hWndEdit);
            }
            else
            {
                bContinueSearch = TRUE;
            }

            if ( bContinueSearch )
            {
                bEOF = TRUE;

                if ( (dwSearchFlags & FR_UP) == FR_UP )
                {
                    INT_X       pos = 0;
                    CHARRANGE_X cr = {0, 0};

                    SendMessage( ei.hWndEdit, EM_EXGETSEL_X, 0, (LPARAM) &cr );
                    pos = cr.cpMin;
                    cr.cpMin = -1;
                    cr.cpMax = -1;
                    SendMessage( ei.hWndEdit, EM_EXSETSEL_X, 0, (LPARAM) &cr );
                    if ( tfW.dwFlags & FR_BEGINNING )
                    {
                        tfW.dwFlags -= FR_BEGINNING;
                    }
                    if ( !bSearchEx )
                    {
                        iFindResult = (INT_X) SendMessage( g_Plugin.hMainWnd, 
                          AKD_TEXTFINDW, (WPARAM) ei.hWndEdit, (LPARAM) &tfW );
                    }
                    else
                    {
                        iFindResult = doFindTextExW( ei.hWndEdit, &tfW );
                    }
                    if ( iFindResult < 0 )
                    {
                        cr.cpMin = pos;
                        cr.cpMax = pos;
                        SendMessage( ei.hWndEdit, EM_EXSETSEL_X, 0, (LPARAM) &cr );
                        bNotFound = TRUE;
                    }
                }
                else
                {
                    if ( ((dwSearchFlags & FR_BEGINNING) != FR_BEGINNING) ||
                         (qs_nEditEOF & srchEOF) )
                    {
                        tfW.dwFlags = dwSearchFlags | FR_BEGINNING;
                        if ( !bSearchEx )
                        {
                            iFindResult = (INT_X) SendMessage( g_Plugin.hMainWnd, 
                              AKD_TEXTFINDW, (WPARAM) ei.hWndEdit, (LPARAM) &tfW );
                        }
                        else
                        {
                            iFindResult = doFindTextExW( ei.hWndEdit, &tfW );
                        }
                        if ( iFindResult < 0 )
                        {
                            bNotFound = TRUE;
                        }
                    }
                    else
                    {
                        bNotFound = TRUE;
                    }
                }
                qs_nEditEOF = 0;
            }
        }
        else
        {
            qs_nEditEOF = 0;
        }
    }

    //if ( bNotFound )
    //{
    //    if ( dwSearchFlags & FRF_REGEXP )
    //    {
    //        bNotRegExp = TRUE;
    //    }
    //}

    if ( (!bNotFound) && (!qs_nEditEOF) )
    {
        int         nLine;
        int         nLine1st;
        CHARRANGE_X cr = { 0, 0 };

        SendMessage( ei.hWndEdit, EM_EXGETSEL_X, 0, (LPARAM) &cr );
        nLine = (int) SendMessage(ei.hWndEdit, EM_EXLINEFROMCHAR, 0, cr.cpMin);
        nLine1st = (int) SendMessage(ei.hWndEdit, EM_GETFIRSTVISIBLELINE, 0, 0);
        if ( nLine < nLine1st )
        {
            SendMessage(ei.hWndEdit, EM_LINESCROLL, 0, nLine - nLine1st);
        }

        if ( (dwParams & QSEARCH_NEXT) ||
             ((dwParams & QSEARCH_FIRST) && !g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE]) )
        {
            UINT uUpdFlags = UFHF_MOVE_TO_TOP_IF_EXISTS;
            if ( (dwParams & QSEARCH_SEL) && !g_Options.dwFlags[OPTF_SRCH_SELFIND_PICKUP] )
                uUpdFlags |= UFHF_KEEP_EDIT_TEXT;
            qsearchFindHistoryAdd(hEdit, g_QSearchDlg.szFindTextW, uUpdFlags);
        }
    }

    // now the Edit control can be deactivated
    qs_bEditCanBeNonActive = TRUE;
    qsearchDoSetNotFound(hEdit, bNotFound, bNotRegExp, bEOF);

    g_QSearchDlg.bIsQSearchingRightNow = FALSE;
}

static void CallHighlightMain(void* phlParams)
{
    if ( g_Plugin.bOldWindows )
    {
        PLUGINCALLSENDA pcsA;

        pcsA.pFunction = (char *) cszHighlightMainA;
        //pcsA.bOnStart = FALSE;
        pcsA.lParam = (LPARAM) phlParams;
        pcsA.dwSupport = 0;

        SendMessageA( g_Plugin.hMainWnd, AKD_DLLCALLA, 0, (LPARAM) &pcsA );
    }
    else
    {
        PLUGINCALLSENDW pcsW;

        pcsW.pFunction = (wchar_t *) cszHighlightMainW;
        //pcsW.bOnStart = FALSE;
        pcsW.lParam = (LPARAM) phlParams;
        pcsW.dwSupport = 0;

        SendMessageW( g_Plugin.hMainWnd, AKD_DLLCALLW, 0, (LPARAM) &pcsW );
    }
}

void qsearchDoTryHighlightAll(HWND hDlg)
{
    if ( g_Plugin.bOldWindows )
    {
        if ( 0 == ((LPCSTR) g_QSearchDlg.szFindTextW)[0] )
        {
            // no text to search for: removing the highlight
            qsearchDoTryUnhighlightAll();
            return;
        }
    }
    else
    {
        if ( 0 == ((LPCWSTR) g_QSearchDlg.szFindTextW)[0] )
        {
            // no text to search for: removing the highlight
            qsearchDoTryUnhighlightAll();
            return;
        }
    }

    if ( g_bHighlightPlugin && !qs_bEditNotFound )
    {
        HWND hCh = GetDlgItem(hDlg, IDC_CH_HIGHLIGHTALL);

        if ( SendMessage(hCh, BM_GETCHECK, 0, 0) == BST_CHECKED )
        {
            EDITINFO  ei;

            ei.hWndEdit = NULL;
            SendMessage( g_Plugin.hMainWnd, 
            AKD_GETEDITINFO, (WPARAM) NULL, (LPARAM) &ei );

            if ( ei.hWndEdit )
            {
                CHARRANGE_X cr = { 0, 0 };

                SendMessage( ei.hWndEdit, EM_EXGETSEL_X, 0, (LPARAM) &cr );
                if ( cr.cpMin != cr.cpMax )
                {
                    DLLECHIGHLIGHT_MARK hlParams;
                    wchar_t szTextColor[16];
                    wchar_t szBkgndColor[16];

                    if ( g_Plugin.bOldWindows )
                    {
                        wsprintfA( (char *) szTextColor, "0" );
                        wsprintfA( (char *) szBkgndColor, "#%02X%02X%02X", 
                          GetRValue(g_Options.colorHighlight),
                          GetGValue(g_Options.colorHighlight),
                          GetBValue(g_Options.colorHighlight)
                        );
                    }
                    else
                    {
                        wsprintfW( szTextColor, L"0" );
                        wsprintfW( szBkgndColor, L"#%02X%02X%02X", 
                          GetRValue(g_Options.colorHighlight),
                          GetGValue(g_Options.colorHighlight),
                          GetBValue(g_Options.colorHighlight)
                        );
                    }

                    hlParams.dwStructSize = sizeof(DLLECHIGHLIGHT_MARK);
                    hlParams.nAction = DLLA_HIGHLIGHT_MARK;
                    hlParams.pColorText = (unsigned char *) szTextColor;
                    hlParams.pColorBk = (unsigned char *) szBkgndColor;
                    hlParams.dwMarkFlags = 0;
                    hlParams.dwFontStyle = 0;
                    hlParams.dwMarkID = g_Options.dwHighlightMarkID;
                    hlParams.wszMarkText = NULL;

                    hCh = GetDlgItem(hDlg, IDC_CH_MATCHCASE);
                    if ( SendMessage(hCh, BM_GETCHECK, 0, 0) == BST_CHECKED )
                        hlParams.dwMarkFlags |= MARKFLAG_MATCHCASE;

                    hCh = GetDlgItem(hDlg, IDC_CH_WHOLEWORD);
                    if ( SendMessage(hCh, BM_GETCHECK, 0, 0) == BST_CHECKED )
                        hlParams.dwMarkFlags |= MARKFLAG_WHOLEWORD;

#if AKELPAD_RUNTIME_VERSION_CHECK
                    if ( VersionCompare(getProgramVersion(&g_Plugin), MAKE_IDENTIFIER(4, 8, 8, 0)) >= 0 )
#endif
                    {
                        if ( g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] )
                        {
                            wchar_t szMarkText[MAX_TEXT_SIZE];

                            szMarkText[0] = 0;
                            if ( g_Plugin.bOldWindows )
                            {
                                MultiByteToWideChar( CP_ACP, 0, (LPCSTR) g_QSearchDlg.szFindTextW, -1, szMarkText, MAX_TEXT_SIZE - 1 );
                            }
                            else
                            {
                                lstrcpyW( szMarkText, g_QSearchDlg.szFindTextW );
                            }
                            adjustIncompleteRegExW( szMarkText );
                            if ( szMarkText[0] != 0 )
                            {
                                hlParams.dwMarkFlags |= MARKFLAG_REGEXP;
                                hlParams.wszMarkText = szMarkText;
                            }
                        }
                    }

                    CallHighlightMain( &hlParams );
                }
                else
                    qsearchDoTryUnhighlightAll(); // no selection
            }
        }
    }
    else
        qsearchDoTryUnhighlightAll(); // not found
}

void qsearchDoTryUnhighlightAll(void)
{
    if ( g_bHighlightPlugin )
    {
        DLLECHIGHLIGHT_UNMARK hlParams;

        hlParams.dwStructSize = sizeof(DLLECHIGHLIGHT_UNMARK);
        hlParams.nAction = DLLA_HIGHLIGHT_UNMARK;
        hlParams.dwMarkID = g_Options.dwHighlightMarkID;

        CallHighlightMain( &hlParams );
    }
}

HWND qsearchGetFindEdit(HWND hDlg)
{
    HWND hEdit;

    if ( qsearchIsFindHistoryEnabled() )
    {
        HWND  hCombo;
        POINT pt;

        hCombo = GetDlgItem(hDlg, IDC_CB_FINDTEXT);
        pt.x = 5;
        pt.y = 5;
        hEdit = ChildWindowFromPoint(hCombo, pt);
    }
    else
    {
        hEdit = GetDlgItem(hDlg, IDC_ED_FINDTEXT);
    }

    return hEdit;
}

BOOL qsearchFindHistoryAdd(HWND hEdit, const wchar_t* cszTextAW, UINT uUpdFlags)
{
    if ( qsearchIsFindHistoryEnabled() && cszTextAW && ((const char *)cszTextAW)[0] )
    {
        int   iItem;
        DWORD dwEditSel;
        HWND  hCombo;

        hCombo = GetDlgItem(g_QSearchDlg.hDlg, IDC_CB_FINDTEXT);
        if ( g_Plugin.bOldWindows )
        {
            dwEditSel = (DWORD) SendMessageA(hCombo, CB_GETEDITSEL, 0, 0);
            iItem = (int) SendMessageA(hCombo, CB_FINDSTRINGEXACT, (WPARAM) (-1), (LPARAM) cszTextAW);
            if ( iItem != 0 )
            {
                if ( (uUpdFlags & UFHF_MOVE_TO_TOP_IF_EXISTS) || (iItem == CB_ERR) )
                {
                    while ( iItem != CB_ERR )
                    {
                        SendMessageA(hCombo, CB_DELETESTRING, iItem, 0);
                        iItem = (int) SendMessageA(hCombo, CB_FINDSTRINGEXACT, (WPARAM) (-1), (LPARAM) cszTextAW);
                    }

                    iItem = (int) SendMessageA(hCombo, CB_GETCOUNT, 0, 0);
                    if ( iItem > (int) (g_Options.dwFindHistoryItems - 1) )
                    {
                        iItem = (int) (g_Options.dwFindHistoryItems - 1);
                        if ( iItem == (int) SendMessageW(hCombo, CB_GETCURSEL, 0, 0) )
                            --iItem; // do not remove the current item
                        if ( iItem >= 0 )
                            SendMessageA(hCombo, CB_DELETESTRING, g_Options.dwFindHistoryItems - 1, 0);
                    }

                    if ( SendMessageA(hCombo, CB_INSERTSTRING, 0, (LPARAM) cszTextAW) >= 0 )
                    {
                        if ( !(uUpdFlags & UFHF_KEEP_EDIT_TEXT) )
                        {
                            SendMessageA(hEdit, WM_SETREDRAW, FALSE, 0);
                            SendMessageA(hCombo, CB_SETCURSEL, 0, 0);
                            SendMessageA(hEdit, WM_SETREDRAW, TRUE, 0);
                            //editSetTrailSel(hEdit);
                            SendMessageA(hCombo, CB_SETEDITSEL, 0, dwEditSel);
                        }
                        return TRUE;
                    }
                }
                /*else
                {
                    SendMessageA(hEdit, WM_SETREDRAW, FALSE, 0);
                    SendMessageA(hCombo, CB_SETCURSEL, iItem, 0);
                    SendMessageA(hEdit, WM_SETREDRAW, TRUE, 0);
                    //editSetTrailSel(hEdit);
                    SendMessageA(hCombo, CB_SETEDITSEL, 0, dwEditSel);
                }*/
            }
        }
        else if ( cszTextAW[0] )
        {
            dwEditSel = (DWORD) SendMessageW(hCombo, CB_GETEDITSEL, 0, 0);
            iItem = (int) SendMessageW(hCombo, CB_FINDSTRINGEXACT, (WPARAM) (-1), (LPARAM) cszTextAW);
            if ( iItem != 0 )
            {
                if ( (uUpdFlags & UFHF_MOVE_TO_TOP_IF_EXISTS) || (iItem == CB_ERR) )
                {
                    while ( iItem != CB_ERR )
                    {
                        SendMessageW(hCombo, CB_DELETESTRING, iItem, 0);
                        iItem = (int) SendMessageW(hCombo, CB_FINDSTRINGEXACT, (WPARAM) (-1), (LPARAM) cszTextAW);
                    }

                    iItem = (int) SendMessageW(hCombo, CB_GETCOUNT, 0, 0);
                    if ( iItem > (int) (g_Options.dwFindHistoryItems - 1) )
                    {
                        iItem = (int) (g_Options.dwFindHistoryItems - 1);
                        if ( iItem == (int) SendMessageW(hCombo, CB_GETCURSEL, 0, 0) )
                            --iItem; // do not remove the current item
                        if ( iItem >= 0 )
                            SendMessageW(hCombo, CB_DELETESTRING, iItem, 0);
                    }

                    if ( SendMessageW(hCombo, CB_INSERTSTRING, 0, (LPARAM) cszTextAW) >= 0 )
                    {
                        if ( !(uUpdFlags & UFHF_KEEP_EDIT_TEXT) )
                        {
                            SendMessageW(hEdit, WM_SETREDRAW, FALSE, 0);
                            SendMessageW(hCombo, CB_SETCURSEL, 0, 0);
                            SendMessageW(hEdit, WM_SETREDRAW, TRUE, 0);
                            //editSetTrailSel(hEdit);
                            SendMessageW(hCombo, CB_SETEDITSEL, 0, dwEditSel);
                        }
                        return TRUE;
                    }
                }
                /*else
                {
                    SendMessageW(hEdit, WM_SETREDRAW, FALSE, 0);
                    SendMessageW(hCombo, CB_SETCURSEL, iItem, 0);
                    SendMessageW(hEdit, WM_SETREDRAW, TRUE, 0);
                    //editSetTrailSel(hEdit);
                    SendMessageW(hCombo, CB_SETEDITSEL, 0, dwEditSel);
                }*/
            }
        }
    }

    return FALSE;
}
