#include "QSearchSettDlg.h"
#include "QSearch.h"


extern PluginState  g_Plugin;
extern QSearchOpt   g_Options;


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
        UINT uState = (UINT) SendMessage(hCheckBox, BM_GETCHECK, 0, 0);
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
        SendMessage( hCheckBox, BM_SETCHECK, (bChecked ? BST_CHECKED : BST_UNCHECKED), 0 );
        return TRUE;
    }
    return FALSE;
}

//////////////// FndAllSettDlg... ////////////////

static BOOL FndAllSettDlg_OnOK(HWND hDlg);
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

BOOL FndAllSettDlg_OnOK(HWND hDlg)
{
    DWORD dwFindAllResult = 0;

    if ( CheckBox_IsChecked(hDlg, IDC_CH_FA_HEADER) )
        dwFindAllResult |= QS_FINDALL_RSLT_SEARCHING;

    if ( CheckBox_IsChecked(hDlg, IDC_CH_FA_POSITION) )
        dwFindAllResult |= QS_FINDALL_RSLT_POS;

    if ( CheckBox_IsChecked(hDlg, IDC_CH_FA_LENGTH) )
        dwFindAllResult |= QS_FINDALL_RSLT_LEN;

    if ( CheckBox_IsChecked(hDlg, IDC_CH_FA_FOOTER) )
        dwFindAllResult |= QS_FINDALL_RSLT_OCCFOUND;

    if ( CheckBox_IsChecked(hDlg, IDC_CH_FA_COLORTHEME) )
        dwFindAllResult |= QS_FINDALL_RSLT_CODERALIAS;
    
    g_Options.dwFindAllResult = dwFindAllResult;

    return TRUE;
}

static void FndAllSettDlg_OnInitDialog(HWND hDlg)
{
    BOOL bChecked;

    AnyWindow_CenterWindow(hDlg, g_Plugin.hMainWnd, FALSE);

    bChecked = (g_Options.dwFindAllResult & QS_FINDALL_RSLT_SEARCHING) ? TRUE : FALSE;
    CheckBox_SetCheck(hDlg, IDC_CH_FA_HEADER, bChecked);

    bChecked = (g_Options.dwFindAllResult & QS_FINDALL_RSLT_POS) ? TRUE : FALSE;
    CheckBox_SetCheck(hDlg, IDC_CH_FA_POSITION, bChecked);

    bChecked = (g_Options.dwFindAllResult & QS_FINDALL_RSLT_LEN) ? TRUE : FALSE;
    CheckBox_SetCheck(hDlg, IDC_CH_FA_LENGTH, bChecked);

    // bChecked = (g_Options.dwFindAllResult & QS_FINDALL_RSLT_LINE) ? TRUE : FALSE;

    // bChecked = (g_Options.dwFindAllResult & QS_FINDALL_RSLT_MATCH) ? TRUE : FALSE;

    bChecked = (g_Options.dwFindAllResult & QS_FINDALL_RSLT_OCCFOUND) ? TRUE : FALSE;
    CheckBox_SetCheck(hDlg, IDC_CH_FA_FOOTER, bChecked);

    bChecked = (g_Options.dwFindAllResult & QS_FINDALL_RSLT_CODERALIAS) ? TRUE : FALSE;
    CheckBox_SetCheck(hDlg, IDC_CH_FA_COLORTHEME, bChecked);
}

void FndAllSettDlg_EndDialog(HWND hDlg, INT_PTR nResult)
{
//    if ( sd_hToolTip )
//    {
//        DestroyWindow(sd_hToolTip);
//        sd_hToolTip = NULL;
//    }
    EndDialog(hDlg, nResult);
}
