#ifndef _resource_h_
#define _resource_h_
//---------------------------------------------------------------------------
#include "QSearch_defs.h"

#ifndef IDC_STATIC
#define IDC_STATIC (-1)
#endif

#define  IDI_QSEARCH                100

#define  IDD_QSEARCH                1000
#define  IDC_ED_FINDTEXT            1001
#define  IDC_CB_FINDTEXT            1002
#define  IDC_CH_MATCHCASE           1003
#define  IDC_CH_WHOLEWORD           1004
#define  IDC_PB_PROGRESS            1005
#define  IDC_CH_HIGHLIGHTALL        1006
#define  IDC_BT_CANCEL              1010
#define  IDC_BT_OK                  1011
#define  IDC_ST_INFO                1015
#define  IDR_MENU_OPTIONS           1020
#define  IDS_EOFREACHED             1040

#define  IDD_QSEARCH_NEW_01         2000
#define  IDD_QSEARCH_NEW_02         2100
#define  IDC_BT_FINDNEXT            2001
#define  IDC_BT_FINDPREV            2002
#define  IDOK_FINDPREV              2003
#define  IDC_BT_FINDALL             2004

#define  IDD_FINDALL_SETTINGS       3000
#define  IDC_ST_EXAMPLE             3001

#define  IDM_START                  10001
#define  IDM_SRCHONTHEFLYMODE       (IDM_START + OPTF_SRCH_ONTHEFLY_MODE)
#define  IDM_SRCHFROMBEGINNING      (IDM_START + OPTF_SRCH_FROM_BEGINNING)
#define  IDM_SRCHUSESPECIALCHARS    (IDM_START + OPTF_SRCH_USE_SPECIALCHARS)
#define  IDM_SRCHUSEREGEXP          (IDM_START + OPTF_SRCH_USE_REGEXP)
#define  IDM_SRCHREGEXPDOTNEWLINE   (IDM_START + OPTF_SRCH_REGEXP_DOT_NEWLINE)
#define  IDM_SRCHPICKUPSELECTION    (IDM_START + OPTF_SRCH_PICKUP_SELECTION)
#define  IDM_SRCHSELFINDPICKUP      (IDM_START + OPTF_SRCH_SELFIND_PICKUP)
#define  IDM_SRCHSTOPEOF            (IDM_START + OPTF_SRCH_STOP_EOF)
#define  IDM_SRCHWNDDOCKEDTOP       (IDM_START + OPTF_SRCH_WND_DOCKEDTOP)
#define  IDM_EDITFOCUSSELECTALL     (IDM_START + OPTF_EDIT_FOCUS_SELECTALL)
#define  IDM_CATCHMAINF3            (IDM_START + OPTF_CATCH_MAIN_F3)
#define  IDM_CATCHMAINESC           (IDM_START + OPTF_CATCH_MAIN_ESC)
#define  IDM_HOTKEYHIDESPANEL       (IDM_START + OPTF_HOTKEY_HIDES_PANEL)
#define  IDM_EDITORAUTOFOCUSMOUSE   (IDM_START + OPTF_EDITOR_AUTOFOCUS_MOUSE)
#define  IDM_QSEARCHAUTOFOCUSMOUSE  (IDM_START + OPTF_QSEARCH_AUTOFOCUS_MOUSE)
#define  IDM_QSEARCHAUTOFOCUSFILE   (IDM_START + OPTF_QSEARCH_AUTOFOCUS_FILE)

#define  IDM_FINDALL_START          10101
#define  IDM_FINDALL_AUTO_COUNT     (IDM_FINDALL_START + QS_FINDALL_AUTO_COUNT)
#define  IDM_FINDALL_COUNTONLY      (IDM_FINDALL_START + QS_FINDALL_COUNTONLY)
#define  IDM_FINDALL_LOGOUTPUT      (IDM_FINDALL_START + QS_FINDALL_LOGOUTPUT)
#define  IDM_FINDALL_FILEOUTPUT     (IDM_FINDALL_START + QS_FINDALL_FILEOUTPUT)
#define  IDM_FINDALL_SETTINGSDLG    10151

//---------------------------------------------------------------------------
#endif
