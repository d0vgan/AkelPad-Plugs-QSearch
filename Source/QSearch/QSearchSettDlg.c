#include "QSearchSettDlg.h"

static BOOL FndAllSettDlg_OnOK(HWND hDlg);
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

	return 0;
}

BOOL FndAllSettDlg_OnOK(HWND hDlg)
{
    // saving the settings...
    return TRUE;
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
