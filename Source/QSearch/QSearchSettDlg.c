#include "QSearchSettDlg.h"
#include "QSearch.h"
#include "QSearchLng.h"
#include "XMemStrFunc.h"


extern PluginState  g_Plugin;
extern QSearchOpt   g_Options;

// tGetFindResultPolicy.nMode
#define QSFRM_LINE        1 // number of lines
#define QSFRM_LINE_CR     2 // number of lines + trailing '\r'
#define QSFRM_CHARINLINE  3 // number of chars within the current line
#define QSFRM_CHAR        4 // number of chars


//////////////// Helpers... ////////////////

static BOOL AnyWindow_CenterWindow(HWND hWnd, HWND hParentWnd, BOOL bRepaint)
{
    RECT rectParent;
    RECT rect;
    INT  height, width;
    INT  x, y;

    GetWindowRect(hParentWnd, &rectParent);
    GetWindowRect(hWnd, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
    x = ((rectParent.right - rectParent.left) - width) / 2;
    x += rectParent.left;
    y = ((rectParent.bottom - rectParent.top) - height) / 2;
    y += rectParent.top;
    return MoveWindow(hWnd, x, y, width, height, bRepaint);
}

static BOOL CheckBox_IsChecked(HWND hDlg, UINT idCheckBox)
{
    HWND hCheckBox = GetDlgItem(hDlg, idCheckBox);
    if ( hCheckBox )
    {
        UINT uState = (UINT) SendMessageW(hCheckBox, BM_GETCHECK, 0, 0);
        if ( uState == BST_CHECKED || uState == BST_INDETERMINATE )
            return TRUE;
    }
    return FALSE;
}

static BOOL CheckBox_SetCheck(HWND hDlg, UINT idCheckBox, BOOL bChecked)
{
    HWND hCheckBox = GetDlgItem(hDlg, idCheckBox);
    if ( hCheckBox )
    {
        SendMessageW( hCheckBox, BM_SETCHECK, (bChecked ? BST_CHECKED : BST_UNCHECKED), 0 );
        return TRUE;
    }
    return FALSE;
}

//////////////// FndAllSettDlg... ////////////////

typedef struct sCheckBoxOptFlagItem {
    int id;
    unsigned int optf;
} tCheckBoxOptFlagItem;

static const tCheckBoxOptFlagItem arrCheckBoxOptions[] = {
    { IDC_CH_FA_HEADER,        QS_FINDALL_RSLT_SEARCHING     },
    { IDC_CH_FA_POSITION,      QS_FINDALL_RSLT_POS           },
    { IDC_CH_FA_LENGTH,        QS_FINDALL_RSLT_LEN           },
    { IDC_CH_FA_FOOTER,        QS_FINDALL_RSLT_OCCFOUND      },
    { IDC_CH_FA_FILTERMODE,    QS_FINDALL_RSLT_FILTERMODE    },
    { IDC_CH_FA_FILTERCONTEXT, QS_FINDALL_RSLT_FILTERCONTEXT },
    { IDC_CH_FA_SYNTAXTHEME,   QS_FINDALL_RSLT_CODERALIAS    },
    { 0,                       0 } // trailing "empty" item
};

static void FndAllSettDlg_OnCheckBoxClicked(HWND hDlg);
static BOOL FndAllSettDlg_OnOK(HWND hDlg);
static void FndAllSettDlg_OnCbOutputDestChanged(HWND hDlg);
static void FndAllSettDlg_OnCbModeChanged(HWND hDlg);
static void FndAllSettDlg_OnEdModeChanged(HWND hDlg);
static void FndAllSettDlg_OnInitDialog(HWND hDlg);
static void FndAllSettDlg_EndDialog(HWND hDlg, INT_PTR nResult);

INT_PTR CALLBACK QSFndAllSettDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if ( uMsg == WM_COMMAND )
    {
        switch ( LOWORD(wParam) )
        {
            case IDC_BT_OK:
            case IDOK:
            {
                if ( FndAllSettDlg_OnOK(hDlg) )
                {
                    FndAllSettDlg_EndDialog(hDlg, 1); // OK - returns 1
                }
                return 1;
            }

            case IDC_BT_CANCEL:
            case IDCANCEL:
            {
                FndAllSettDlg_EndDialog(hDlg, 0); // Cancel - returns 0
                return 1;
            }

            case IDC_CH_FA_HEADER:
            case IDC_CH_FA_POSITION:
            case IDC_CH_FA_LENGTH:
            case IDC_CH_FA_FOOTER:
            case IDC_RB_FA_WHOLELINE:
            case IDC_RB_FA_MATCHONLY:
            case IDC_CH_FA_FILTERMODE:
            case IDC_CH_FA_FILTERCONTEXT:
            {
                if ( HIWORD(wParam) == BN_CLICKED )
                {
                    FndAllSettDlg_OnCheckBoxClicked(hDlg);
                }
                break;
            }

            case IDC_CB_FA_OUTPUT_DEST:
            {
                if ( HIWORD(wParam) == CBN_SELCHANGE )
                {
                    FndAllSettDlg_OnCbOutputDestChanged(hDlg);
                }
                break;
            }

            case IDC_CB_FA_MODE:
            {
                if ( HIWORD(wParam) == CBN_SELCHANGE )
                {
                    FndAllSettDlg_OnCbModeChanged(hDlg);
                }
                break;
            }

            case IDC_ED_FA_BEFORE:
            case IDC_ED_FA_AFTER:
            {
                if ( HIWORD(wParam) == EN_CHANGE )
                {
                    FndAllSettDlg_OnEdModeChanged(hDlg);
                }
                break;
            }

            default:
                break;
        }
    }
    else if ( uMsg == WM_INITDIALOG )
    {
        FndAllSettDlg_OnInitDialog(hDlg);
    }

    return 0;
}

static DWORD getFindAllResultFlags(HWND hDlg)
{
    unsigned int i;
    DWORD dwFindAllResultFlags;

    dwFindAllResultFlags = 0;

    for ( i = 0; ; ++i )
    {
        if ( arrCheckBoxOptions[i].id == 0 && arrCheckBoxOptions[i].optf == 0 )
            break;

        if ( CheckBox_IsChecked(hDlg, arrCheckBoxOptions[i].id) )
            dwFindAllResultFlags |= arrCheckBoxOptions[i].optf;
    }

    if ( CheckBox_IsChecked(hDlg, IDC_RB_FA_MATCHONLY) )
        dwFindAllResultFlags |= QS_FINDALL_RSLT_MATCHONLY;
    else
        dwFindAllResultFlags |= QS_FINDALL_RSLT_WHOLELINE;

    return dwFindAllResultFlags;
}

static int getFindAllOutputMode(HWND hDlg, int* pnBefore, int* pnAfter)
{
    HWND hCbMode;
    HWND hEdBefore;
    HWND hEdAfter;
    int nMode;
    wchar_t szNum[8];

    hCbMode = GetDlgItem(hDlg, IDC_CB_FA_MODE);
    nMode = 1 + (int) SendMessageW(hCbMode, CB_GETCURSEL, 0, 0);

    szNum[0] = 0;
    hEdBefore = GetDlgItem(hDlg, IDC_ED_FA_BEFORE);
    GetWindowTextW(hEdBefore, szNum, 7);
    *pnBefore = (int) xatoiW(szNum, NULL);

    szNum[0] = 0;
    hEdAfter = GetDlgItem(hDlg, IDC_ED_FA_AFTER);
    GetWindowTextW(hEdAfter, szNum, 7);
    *pnAfter = (int) xatoiW(szNum, NULL);

    return nMode;
}

static void applyOutputOptions(HWND hDlg)
{
    HWND hCbOutputDest;
    int nOutputDest;

    hCbOutputDest = GetDlgItem(hDlg, IDC_CB_FA_OUTPUT_DEST);
    nOutputDest = 1 + (int) SendMessageW(hCbOutputDest, CB_GETCURSEL, 0, 0);

    if ( nOutputDest >= QS_FINDALL_COUNTONLY && nOutputDest <= QS_FINDALL_FILEOUTPUT_SNGL )
    {
        g_Options.dwFindAllMode = (g_Options.dwFindAllMode & 0xFF00) + nOutputDest;
    }

    if ( nOutputDest >= QS_FINDALL_LOGOUTPUT && nOutputDest <= QS_FINDALL_FILEOUTPUT_SNGL )
    {
        int nMode;
        int nBefore = 0;
        int nAfter = 0;

        nMode = getFindAllOutputMode(hDlg, &nBefore, &nAfter);

        if ( nOutputDest == QS_FINDALL_LOGOUTPUT )
        {
            g_Options.LogOutputFRP.nMode = nMode;
            g_Options.LogOutputFRP.nBefore = nBefore;
            g_Options.LogOutputFRP.nAfter = nAfter;
        }
        else
        {
            g_Options.FileOutputFRP.nMode = nMode;
            g_Options.FileOutputFRP.nBefore = nBefore;
            g_Options.FileOutputFRP.nAfter = nAfter;
        }
    }
}

static void updateExampleData(HWND hDlg)
{
    HWND hStExampleData;
    const wchar_t* cszTextFormat;
    const wchar_t* pszBegin;
    int nLen;
    DWORD dwFindAllResultFlags;
    int nMode;
    int nBefore = 0;
    int nAfter = 0;
    tDynamicBuffer infoBuf;
    wchar_t szTempText[128];
    wchar_t szMatch1[64];
    wchar_t szMatch2[64];

    tDynamicBuffer_Init(&infoBuf);
    tDynamicBuffer_Allocate(&infoBuf, 256*sizeof(wchar_t));

    szMatch1[0] = 0;
    szMatch2[0] = 0;

    nMode = getFindAllOutputMode(hDlg, &nBefore, &nAfter);
    dwFindAllResultFlags = getFindAllResultFlags(hDlg);

    if ( (dwFindAllResultFlags & QS_FINDALL_RSLT_FILTERMODE) == 0 )
    {
        if ( dwFindAllResultFlags & QS_FINDALL_RSLT_SEARCHING )
        {
            cszTextFormat = qsearchGetStringW(QS_STRID_FINDALL_SEARCHINGFOR);
            nLen = wsprintfW(szTempText, cszTextFormat, L'/', L"w[a-z]+d", L'/', L"Example.txt", 2);
            tDynamicBuffer_Append(&infoBuf, szTempText, nLen*sizeof(wchar_t));
            tDynamicBuffer_Append(&infoBuf, L"\n", 1*sizeof(wchar_t));
            lstrcatW(szMatch1, L"   ");
            lstrcatW(szMatch2, L"   ");
        }
        if ( dwFindAllResultFlags & QS_FINDALL_RSLT_POS )
        {
            lstrcatW(szMatch1, L"(1:3)");
            lstrcatW(szMatch2, L"(1:12)");
        }
        if ( dwFindAllResultFlags & QS_FINDALL_RSLT_LEN )
        {
            lstrcatW(szMatch1, L"(5)");
            lstrcatW(szMatch2, L"(4)");
        }
    }

    {
        const wchar_t* const szEntireLine = L"A world of words.\n";
        int nTempBefore;
        int nTempAfter;

        if ( szMatch1[0] != 0 )
            lstrcatW(szMatch1, L"\t");
        if ( dwFindAllResultFlags & QS_FINDALL_RSLT_MATCHONLY )
            lstrcatW(szMatch1, L"world\n");
        else
        {
            if ( nMode == QSFRM_CHAR || nMode == QSFRM_CHARINLINE )
            {
                pszBegin = szEntireLine + 2; // position before "world"
                nLen = 5; // length of "world"

                nTempBefore = nBefore;
                if ( nTempBefore > 2 ) // characters before "world"
                    nTempBefore = 2;
                if ( nTempBefore > 0 )
                {
                    pszBegin -= nTempBefore;
                    nLen += nTempBefore;
                }

                nTempAfter = nAfter;
                if ( nTempAfter > 10 ) // characters after "world"
                    nTempAfter = 10;
                if ( nTempAfter > 0 )
                    nLen += nTempAfter;

                lstrcpynW(szTempText, pszBegin, nLen + 1);
                szTempText[nLen++] = L'\n';
                szTempText[nLen] = 0;
                lstrcatW(szMatch1, szTempText);
            }
            else
                lstrcatW(szMatch1, szEntireLine);
        }

        if ( szMatch2[0] != 0 )
            lstrcatW(szMatch2, L"\t");
        if ( dwFindAllResultFlags & QS_FINDALL_RSLT_MATCHONLY )
            lstrcatW(szMatch2, L"word\n");
        else
        {
            if ( nMode == QSFRM_CHAR || nMode == QSFRM_CHARINLINE )
            {
                pszBegin = szEntireLine + 11; // position before "word"
                nLen = 4; // length of "word"

                nTempBefore = nBefore;
                if ( nTempBefore > 11 ) // characters before "word"
                    nTempBefore = 11;
                if ( nTempBefore > 0 )
                {
                    pszBegin -= nTempBefore;
                    nLen += nTempBefore;
                }

                nTempAfter = nAfter;
                if ( nTempAfter > 2 ) // characters after "word"
                    nTempAfter = 2;
                if ( nTempAfter > 0 )
                    nLen += nTempAfter;

                lstrcpynW(szTempText, pszBegin, nLen + 1);
                szTempText[nLen++] = L'\n';
                szTempText[nLen] = 0;
                lstrcatW(szMatch2, szTempText);
            }
            else
                lstrcatW(szMatch2, szEntireLine);
        }

        tDynamicBuffer_Append(&infoBuf, szMatch1, lstrlenW(szMatch1)*sizeof(wchar_t));
        if ( nMode == QSFRM_LINE_CR )
        {
            if ( !(dwFindAllResultFlags & QS_FINDALL_RSLT_FILTERMODE) || (dwFindAllResultFlags & QS_FINDALL_RSLT_FILTERCONTEXT) )
                tDynamicBuffer_Append(&infoBuf, L"\n", 1*sizeof(wchar_t));
        }
        tDynamicBuffer_Append(&infoBuf, szMatch2, lstrlenW(szMatch2)*sizeof(wchar_t));
    }

    if ( (dwFindAllResultFlags & QS_FINDALL_RSLT_FILTERMODE) == 0 )
    {
        if ( dwFindAllResultFlags & QS_FINDALL_RSLT_OCCFOUND )
        {
            cszTextFormat = qsearchGetStringW(QS_STRID_FINDALL_OCCURRENCESFOUND);
            nLen = wsprintfW(szTempText, cszTextFormat, 2); // "2 found."
            tDynamicBuffer_Append(&infoBuf, szTempText, nLen*sizeof(wchar_t));
        }
    }
    EnableWindow( GetDlgItem(hDlg, IDC_CH_FA_FILTERCONTEXT), 
      (dwFindAllResultFlags & QS_FINDALL_RSLT_FILTERMODE) != 0 ? TRUE : FALSE );

    tDynamicBuffer_Append(&infoBuf, L"\0", 1*sizeof(wchar_t));

    hStExampleData = GetDlgItem(hDlg, IDC_ST_FA_EXAMPLE_DATA);
    SetWindowTextW(hStExampleData, (const wchar_t *) infoBuf.ptr);
}

void FndAllSettDlg_OnCheckBoxClicked(HWND hDlg)
{
    updateExampleData(hDlg);
}

BOOL FndAllSettDlg_OnOK(HWND hDlg)
{
    g_Options.dwFindAllResult = getFindAllResultFlags(hDlg);
    applyOutputOptions(hDlg);

    return TRUE;
}

void FndAllSettDlg_OnCbOutputDestChanged(HWND hDlg)
{
    HWND hCbOutputDest;
    HWND hStMode;
    HWND hCbMode;
    HWND hStBefore;
    HWND hEdBefore;
    HWND hStAfter;
    HWND hEdAfter;
    int nOutputDest;
    int nMode;
    int nBefore;
    int nAfter;
    wchar_t szNum[8];

    hCbOutputDest = GetDlgItem(hDlg, IDC_CB_FA_OUTPUT_DEST);
    hStMode = GetDlgItem(hDlg, IDC_ST_FA_MODE);
    hCbMode = GetDlgItem(hDlg, IDC_CB_FA_MODE);
    hStBefore = GetDlgItem(hDlg, IDC_ST_FA_BEFORE);
    hEdBefore = GetDlgItem(hDlg, IDC_ED_FA_BEFORE);
    hStAfter = GetDlgItem(hDlg, IDC_ST_FA_AFTER);
    hEdAfter = GetDlgItem(hDlg, IDC_ED_FA_AFTER);

    nOutputDest = 1 + (int) SendMessageW(hCbOutputDest, CB_GETCURSEL, 0, 0);
    if ( nOutputDest == QS_FINDALL_LOGOUTPUT )
    {
        nMode = g_Options.LogOutputFRP.nMode;
        nBefore = g_Options.LogOutputFRP.nBefore;
        nAfter = g_Options.LogOutputFRP.nAfter;
    }
    else if ( nOutputDest == QS_FINDALL_FILEOUTPUT_MULT || nOutputDest == QS_FINDALL_FILEOUTPUT_SNGL )
    {
        nMode = g_Options.FileOutputFRP.nMode;
        nBefore = g_Options.FileOutputFRP.nBefore;
        nAfter = g_Options.FileOutputFRP.nAfter;
    }
    else // Count Only or unknown
    {
        nMode = -1;
        nBefore = -1;
        nAfter = -1;
    }

    if ( nMode >= QSFRM_LINE && nMode <= QSFRM_CHAR )
    {
        EnableWindow(hStMode, TRUE);
        EnableWindow(hCbMode, TRUE);
        EnableWindow(hStBefore, TRUE);
        EnableWindow(hStAfter, TRUE);
        EnableWindow(hEdBefore, TRUE);
        EnableWindow(hEdAfter, TRUE);

        SendMessageW( hCbMode, CB_RESETCONTENT, 0, 0 );
        SendMessageW( hCbMode, CB_ADDSTRING, 0, (LPARAM) qsearchGetStringW(QS_STRID_FINDALL_OUTPUT_CTX_LINES) );
        SendMessageW( hCbMode, CB_ADDSTRING, 0, (LPARAM) qsearchGetStringW(QS_STRID_FINDALL_OUTPUT_CTX_LINESCR) );
        SendMessageW( hCbMode, CB_ADDSTRING, 0, (LPARAM) qsearchGetStringW(QS_STRID_FINDALL_OUTPUT_CTX_CHARSINLINE) );
        SendMessageW( hCbMode, CB_ADDSTRING, 0, (LPARAM) qsearchGetStringW(QS_STRID_FINDALL_OUTPUT_CTX_CHARS) );

        SendMessageW(hCbMode, CB_SETCURSEL, nMode - 1, 0);

        if ( nBefore >= 0 && nBefore <= 999 )
        {
            wsprintfW(szNum, L"%d", nBefore);
            SetWindowTextW(hEdBefore, szNum);
        }
        else
        {
            SetWindowTextW(hEdBefore, L"0");
        }
        
        if ( nAfter >= 0 && nAfter <= 999 )
        {
            wsprintfW(szNum, L"%d", nAfter);
            SetWindowTextW(hEdAfter, szNum);
        }
        else
        {
            SetWindowTextW(hEdAfter, L"0");
        }

        FndAllSettDlg_OnCbModeChanged(hDlg);
    }
    else
    {
        EnableWindow(hStMode, FALSE);
        EnableWindow(hCbMode, FALSE);
        EnableWindow(hStBefore, FALSE);
        EnableWindow(hStAfter, FALSE);
        EnableWindow(hEdBefore, FALSE);
        EnableWindow(hEdAfter, FALSE);
        SendMessageW(hCbMode, CB_RESETCONTENT, 0, 0);
        SetWindowTextW(hEdBefore, L"");
        SetWindowTextW(hEdAfter, L"");
    }
}

void FndAllSettDlg_OnCbModeChanged(HWND hDlg)
{
    updateExampleData(hDlg);
}

void FndAllSettDlg_OnEdModeChanged(HWND hDlg)
{
    updateExampleData(hDlg);
}

void FndAllSettDlg_OnInitDialog(HWND hDlg)
{
    HWND hCbOutputDest;
    HWND hEdBefore;
    HWND hEdAfter;
    unsigned int i;
    BOOL bChecked;

    qsearchSetFindAllSettDlgLang(hDlg);

    AnyWindow_CenterWindow(hDlg, g_Plugin.hMainWnd, FALSE);

    for ( i = 0; ; ++i )
    {
        if ( arrCheckBoxOptions[i].id == 0 && arrCheckBoxOptions[i].optf == 0 )
            break;

        bChecked = (g_Options.dwFindAllResult & arrCheckBoxOptions[i].optf) ? TRUE : FALSE;
        CheckBox_SetCheck(hDlg, arrCheckBoxOptions[i].id, bChecked);
    }

    if ( g_Options.dwFindAllResult & QS_FINDALL_RSLT_MATCHONLY )
        CheckBox_SetCheck(hDlg, IDC_RB_FA_MATCHONLY, TRUE);
    else
        CheckBox_SetCheck(hDlg, IDC_RB_FA_WHOLELINE, TRUE);

    FndAllSettDlg_OnCheckBoxClicked(hDlg);

    hCbOutputDest = GetDlgItem(hDlg, IDC_CB_FA_OUTPUT_DEST);
    SendMessageW( hCbOutputDest, CB_ADDSTRING, 0, (LPARAM) qsearchGetStringW(QS_STRID_FINDALL_OUTPUT_DST_COUNTONLY) );
    SendMessageW( hCbOutputDest, CB_ADDSTRING, 0, (LPARAM) qsearchGetStringW(QS_STRID_FINDALL_OUTPUT_DST_LOG) );
    SendMessageW( hCbOutputDest, CB_ADDSTRING, 0, (LPARAM) qsearchGetStringW(QS_STRID_FINDALL_OUTPUT_DST_FILEN) );
    SendMessageW( hCbOutputDest, CB_ADDSTRING, 0, (LPARAM) qsearchGetStringW(QS_STRID_FINDALL_OUTPUT_DST_FILE1) );

    hEdBefore = GetDlgItem(hDlg, IDC_ED_FA_BEFORE);
    SendMessageW(hEdBefore, EM_SETLIMITTEXT, 3, 0);

    hEdAfter = GetDlgItem(hDlg, IDC_ED_FA_AFTER);
    SendMessageW(hEdAfter, EM_SETLIMITTEXT, 3, 0);

    i = g_Options.dwFindAllMode & 0x0F;
    if ( i >= QS_FINDALL_COUNTONLY && i <= QS_FINDALL_FILEOUTPUT_SNGL )
    {
        SendMessageW(hCbOutputDest, CB_SETCURSEL, i - 1, 0);
    }
    FndAllSettDlg_OnCbOutputDestChanged(hDlg);
}

void FndAllSettDlg_EndDialog(HWND hDlg, INT_PTR nResult)
{
    EndDialog(hDlg, nResult);
}
