#ifndef _qsearch_dlg_h_
#define _qsearch_dlg_h_
//---------------------------------------------------------------------------

// may be needed for Dev-C++ (mingw32)
#ifndef _WIN32_IE
    #define _WIN32_IE 0x0400
#endif


#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "AkelDllHeader.h"
#include "QSearchFindEx.h"


// may be needed for VC++ 6.0
#ifndef GWLP_WNDPROC
    #define GWLP_WNDPROC     (-4)
#endif

// may be needed for VC++ 6.0
#if _MSC_VER <= 1200
    #ifndef SetWindowLongPtrA
        #define SetWindowLongPtrA SetWindowLongA
        #define GetWindowLongPtrA GetWindowLongA
    #endif
    #ifndef SetWindowLongPtrW
        #define SetWindowLongPtrW SetWindowLongW
        #define GetWindowLongPtrW GetWindowLongW
    #endif
    typedef LONG LONG_PTR;
#endif

#ifndef FR_WHOLEWORD
    #define FR_WHOLEWORD      0x00000002
#endif


#define  QS_UU_WHOLEWORD    0x0001 // show/hide whole word check-box
#define  QS_UU_FIND         0x0002 // set edit or combo-box find control
#define  QS_UU_ALL          0xFFFF // update all the controls

#define  QS_FF_NOPICKUPSEL  0x0001 // do not pick up selection

#define  QS_SO_UNKNOWN      0
#define  QS_SO_QSEARCH      1
#define  QS_SO_EDITOR       2

#define  QS_SF_CANPICKUPSELTEXT  0x0001
#define  QS_SF_DLGSWITCH         0x1000


// messages...
#define  QSM_FINDFIRST      (WM_USER + 1001) /*  0, 0                        */
#define  QSM_FINDNEXT       (WM_USER + 1002) /*  (BOOL) bFindPrev, QS_FF_... */
#define  QSM_SELFIND        (WM_USER + 1011) /*  (BOOL) bFindPrev, 0         */
#define  QSM_PICKUPSELTEXT  (WM_USER + 1015) /*  0, (BOOL *) &bPickedUp      */
#define  QSM_GETHWNDEDIT    (WM_USER + 1021) /*  0, (HWND *) &hWnd           */
#define  QSM_GETHWNDCOMBO   (WM_USER + 1022) /*  0, (HWND *) &hWnd           */
#define  QSM_SHOWHIDE       (WM_USER + 1052) /*  (BOOL) bShow, QS_SF_...     */
#define  QSM_SETNOTFOUND    (WM_USER + 1061) /*  (BOOL) bNotFound, 0         */
#define  QSM_CHECKHIGHLIGHT (WM_USER + 1071) /*  0, 0                        */
#define  QSM_UPDATEUI       (WM_USER + 1081) /*  QS_UU_... , 0               */
#define  QSM_QUIT           (WM_USER + 1101) /*  0, 0                        */
// notifications...
#define  QSN_DLGSWITCH      (WM_USER + 1501) /*  0, 0                        */


/* >>>>>>>>>>>>>>>>>>>>>>>> qsearchdlg state >>>>>>>>>>>>>>>>>>>>>>>> */
    #define  MAX_TEXT_SIZE  120

    typedef struct tQSearchDlgState {
        HWND    hDlg;
        HWND    hFindEdit;
        HWND    hBtnFindNext;
        HWND    hBtnFindPrev;
        BOOL    bMatchCase;
        //BOOL    bOnDlgStart;
        BOOL    bQSearching;
        BOOL    bIsQSearchingRightNow;
        BOOL    bMouseJustLeavedFindEdit;
        DOCK*   pDockData;
        wchar_t szFindTextW[MAX_TEXT_SIZE];
        UINT    uSearchOrigin; // see QS_SO_*
        UINT    uWmShowFlags; // see QS_SF_*
    } QSearchDlgState;

    void initializeQSearchDlgState(QSearchDlgState* pQSearchDlg);
/* <<<<<<<<<<<<<<<<<<<<<<<< qsearchdlg state <<<<<<<<<<<<<<<<<<<<<<<< */


INT_PTR CALLBACK qsearchDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL qsearchIsFindHistoryEnabled(void);
BOOL qsearchIsFindHistoryBeingSaved(void);
BOOL qsearchIsSearchFlagsBeingSaved(void);
BOOL qsearchIsSavingHistoryToStdLocation(void);

//---------------------------------------------------------------------------
#endif
