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
#include "XMemStrFunc.h"


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

// Looks like "inline" C funcs are supported from VC 2015 (_MSC_VER = 1900)
#if _MSC_VER < 1900
#define inline __inline
#endif

#ifndef FR_WHOLEWORD
    #define FR_WHOLEWORD      0x00000002
#endif


#define  QS_UU_WHOLEWORD     0x0001 // show/hide whole word check-box
#define  QS_UU_FIND          0x0002 // set edit or combo-box find control
#define  QS_UU_ALL           0xFFFF // update all the controls

#define  QS_FF_NOPICKUPSEL   0x0001 // do not pick up selection
#define  QS_FF_NOSETSELFIRST 0x0002 // see QSEARCH_NOSETSEL_FIRST
#define  QS_FF_NOFINDUP      0x0004 // see QSEARCH_NOFINDUP
#define  QS_FF_NOHISTORYUPD  0x0008 // see QSEARCH_NOHISTORYUPD

#define  QS_PS_UPDATEHISTORY  0x01

#define  QS_SO_UNKNOWN        0
#define  QS_SO_QSEARCH        1
#define  QS_SO_EDITOR         2
#define  QS_SO_QSEARCH_FORCED 3

#define  QS_SF_CANPICKUPSELTEXT  0x0001
#define  QS_SF_DLGSWITCH         0x1000

#define  QS_SNF_SETINFOEMPTY     0x0001
#define  QS_SNF_FORCEFINDFIRST   0x0002
#define  QS_SNF_IGNOREEOF        0x0004

#define  QS_PSF_PICKEDUP     0x01
#define  QS_PSF_TEXTCHANGED  0x02


// messages...
#define  QSM_FINDFIRST      (WM_USER + 1001) /*  0, 0                        */
#define  QSM_FINDNEXT       (WM_USER + 1002) /*  (BOOL) bFindPrev, QS_FF_... */
#define  QSM_FINDALL        (WM_USER + 1003) /*  (DWORD) dwFindAllMode, 0    */
#define  QSM_SELFIND        (WM_USER + 1011) /*  (BOOL) bFindPrev, 0         */
#define  QSM_PICKUPSELTEXT  (WM_USER + 1015) /*  QS_PS_.., (UINT*)&nPickedUp */
#define  QSM_GETHWNDEDIT    (WM_USER + 1021) /*  0, (HWND *) &hWnd           */
#define  QSM_GETHWNDCOMBO   (WM_USER + 1022) /*  0, (HWND *) &hWnd           */
#define  QSM_SHOWHIDE       (WM_USER + 1052) /*  (BOOL) bShow, QS_SF_...     */
#define  QSM_SETNOTFOUND    (WM_USER + 1061) /*  (BOOL) bNotFound, QS_SNF_.. */
#define  QSM_CHECKHIGHLIGHT (WM_USER + 1071) /*  0, 0                        */
#define  QSM_UPDATEUI       (WM_USER + 1081) /*  QS_UU_... , 0               */
#define  QSM_QUIT           (WM_USER + 1101) /*  0, 0                        */
// notifications...
#define  QSN_DLGSWITCH      (WM_USER + 1501) /*  0, 0                        */


// tGetFindResultPolicy.nMode
#define QSFRM_LINE        1 // number of lines
#define QSFRM_LINE_CR     2 // number of lines + trailing '\r'
#define QSFRM_CHARINLINE  3 // number of chars within the current line
#define QSFRM_CHAR        4 // number of chars

// tGetFindResultPolicy.nHighlight
#define QSFRH_NONE        0 // don't highlight
#define QSFRH_SELECT      1 // select the "find what" text
#define QSFRH_IFCHECKED   2 // highlight if "Highlight All" is checked
#define QSFRH_ALWAYS      3 // highlight always

// Regular Expressions for Find All (All Files)
#define QS_FINDALL_REPATTERN_ALLFILES    L"^[ ]*\\((\\d+) (\\d+):(\\d+)\\)"
#define QS_FINDALL_RETAGS_ALLFILES       L"/FRAME=\\1 /GOTOLINE=\\2:\\3"
// Regular Expressions for Find All (Single File)
#define QS_FINDALL_REPATTERN_SINGLEFILE  L"^[ ]*\\((\\d+):(\\d+)\\)"
#define QS_FINDALL_RETAGS_SINGLEFILE     L"/GOTOLINE=\\1:\\2"

// FindAllFlags
#define QS_FAF_SPECCHAR  0x0001
#define QS_FAF_REGEXP    0x0002
#define QS_FAF_MATCHCASE 0x0010
#define QS_FAF_WHOLEWORD 0x0020

DWORD getFindAllFlags(const DWORD dwOptFlags[]);


/* >>>>>>>>>>>>>>>>>>>>>>>> qsearchdlg state >>>>>>>>>>>>>>>>>>>>>>>> */
    #define  MAX_TEXT_SIZE  250
    #define  MAX_RESULTS_FRAMES 16

    #define QS_FIS_INVALID     0x0001  // frame is invalid, e.g. it was closed
    #define QS_FIS_TEXTCHANGED 0x0002  // frame's text contains new modifications

    typedef unsigned __int64 matchpos_t; // represents a match position within a text

    #define MATCHPOS_INVALID ((matchpos_t)(-1))

    matchpos_t to_matchpos_ae(const AECHARINDEX* ci, HWND hWndEdit);
    matchpos_t to_matchpos_ae_ex(const AECHARINDEX* ci, HWND hWndEdit, BOOL bWordWrap);
    matchpos_t to_matchpos(unsigned int line, unsigned int pos_in_line);
    unsigned int get_matchpos_line(matchpos_t pos);
    unsigned int get_matchpos_pos_in_line(matchpos_t pos);

    // returns either a 0-based index or -1
    int find_in_sorted_matchpos_array(const matchpos_t* pArr, unsigned int nItems, matchpos_t val, BOOL* pbExactMatch);

    typedef struct sQSFindAllFrameItem {
        const FRAMEDATA* pFrame;
        UINT_PTR nItemState; // see QS_FIS_*
        INT_PTR nBufBytesOffset;
        INT_PTR nMatches;
    } tQSFindAllFrameItem;

    typedef struct sQSSearchResultsItem {
        const FRAMEDATA* pFrame;
        DWORD            dwFindAllFlags; // see QS_FAF_*
        wchar_t          szFindTextW[MAX_TEXT_SIZE];
    } tQSSearchResultsItem;

    void tQSSearchResultsItem_Init(tQSSearchResultsItem* pItem);
    void tQSSearchResultsItem_Assign(tQSSearchResultsItem* pItem, const FRAMEDATA* pFrame, const wchar_t* cszFindWhat, DWORD dwFindAllFlags);
    void tQSSearchResultsItem_Copy(tQSSearchResultsItem* pDstItem, const tQSSearchResultsItem* pSrcItem);

    typedef struct tQSearchDlgState {
        HWND             hDlg;
        HWND             hFindEdit;
        HWND             hFindListBox;
        HWND             hBtnFindNext;
        HWND             hBtnFindPrev;
        HWND             hBtnFindAll;
        HWND             hStInfo;
        HMENU            hPopupMenu;
        HMENU            hFindAllPopupMenu;
        DWORD            dwHotKeyQSearch;
        DWORD            dwHotKeyFindAll;
        DWORD            dwHotKeyGoToNextFindAllMatch;
        DWORD            dwHotKeyGoToPrevFindAllMatch;
        BOOL             bQSearching;
        BOOL             bIsQSearchingRightNow;
        BOOL             bMouseJustLeavedFindEdit;
        DOCK*            pDockData;
        int              nResultsItemsCount;
        tQSSearchResultsItem SearchResultsItems[MAX_RESULTS_FRAMES];
        wchar_t          szFindTextW[MAX_TEXT_SIZE];
        wchar_t          szFindAllFindTextW[MAX_TEXT_SIZE];
        wchar_t          szLastHighlightTextW[MAX_TEXT_SIZE];
        DWORD            dwFindAllFlags; // see QS_FAF_*
        DWORD            dwLastHighlightFlags; // see QS_FAF_*
        UINT             uSearchOrigin; // see QS_SO_*
        UINT             uWmShowFlags; // see QS_SF_*
        COLORREF         crTextColor;
        COLORREF         crBkgndColor;
        HBRUSH           hBkgndBrush;
        HWND             hCurrentMatchSetInfoEditWnd;
        BOOL             bFindAllWasUsingLogOutput;
        tDynamicBuffer   currentMatchesBuf; // match positions as matchpos_t
        tDynamicBuffer   findAllFramesBuf; // tQSFindAllFrameItem items
        tDynamicBuffer   findAllMatchesBuf; // match positions in all files as matchpos_t
    } QSearchDlgState;

    void initializeQSearchDlgState(QSearchDlgState* pQSearchDlg);
    void QSearchDlgState_AddResultsFrame(QSearchDlgState* pQSearchDlg, const FRAMEDATA* pFrame, const wchar_t* cszFindWhat, DWORD dwFindAllFlags);
    void QSearchDlgState_RemoveResultsFrame(QSearchDlgState* pQSearchDlg, const FRAMEDATA* pFrame);
    int  QSearchDlgState_FindResultsFrame(const QSearchDlgState* pQSearchDlg, const FRAMEDATA* pFrame);
    const FRAMEDATA* QSearchDlgState_GetSearchResultsFrame(const QSearchDlgState* pQSearchDlg);

    void QSearchDlgState_addCurrentMatch(QSearchDlgState* pQSearchDlg, matchpos_t nMatchPos);
    void QSearchDlgState_clearCurrentMatches(QSearchDlgState* pQSearchDlg, BOOL bFreeMemory);
    int  QSearchDlgState_findInCurrentMatches(const QSearchDlgState* pQSearchDlg, matchpos_t nMatchPos, BOOL* pbExactMatch);

    void QSearchDlgState_addFindAllMatch(QSearchDlgState* pQSearchDlg, matchpos_t nMatchPos);
    void QSearchDlgState_addFindAllFrameItem(QSearchDlgState* pQSearchDlg, const tQSFindAllFrameItem* pItem);
    void QSearchDlgState_clearFindAllMatchesAndFrames(QSearchDlgState* pQSearchDlg, BOOL bFreeMemory);
    int  QSearchDlgState_findInFindAllFrameItemMatches(const QSearchDlgState* pQSearchDlg, const tQSFindAllFrameItem* pItem, matchpos_t nMatchPos, BOOL* pbExactMatch);
    const tQSFindAllFrameItem* QSearchDlgState_getFindAllFrameItemByFrame(const QSearchDlgState* pQSearchDlg, const FRAMEDATA* pFrame);
    const matchpos_t* QSearchDlgState_getFindAllFrameItemMatches(const QSearchDlgState* pQSearchDlg, const tQSFindAllFrameItem* pItem);
    matchpos_t QSearchDlgState_getFindAllFrameItemMatchAt(const QSearchDlgState* pQSearchDlg, const tQSFindAllFrameItem* pItem, int idx);
    const tQSFindAllFrameItem* QSearchDlgState_getFindAllValidFrameItemForward(QSearchDlgState* pQSearchDlg, const tQSFindAllFrameItem* pItem);
    const tQSFindAllFrameItem* QSearchDlgState_getFindAllValidFrameItemBackward(QSearchDlgState* pQSearchDlg, const tQSFindAllFrameItem* pItem);
    BOOL QSearchDlgState_isFindAllFrameItemInternallyValid(const QSearchDlgState* pQSearchDlg, const tQSFindAllFrameItem* pItem);
    BOOL QSearchDlgState_isFindAllMatchesEmpty(const QSearchDlgState* pQSearchDlg);

    BOOL QSearchDlgState_isFindAllSearchEqualToTheCurrentSearch(const QSearchDlgState* pQSearchDlg, const wchar_t* cszFindWhat, DWORD dwFindAllFlags);

    BOOL QSearchDlgState_isLastHighlightedEqualToTheSearch(const QSearchDlgState* pQSearchDlg, const wchar_t* cszFindWhat, DWORD dwFindAllFlags);
    BOOL QSearchDlgState_isLastHighlightedEqualToTheSearchW(const QSearchDlgState* pQSearchDlg, const wchar_t* cszFindWhatW, DWORD dwFindAllFlags);
    void QSearchDlgState_clearLastHighlighted(QSearchDlgState* pQSearchDlg);

/* <<<<<<<<<<<<<<<<<<<<<<<< qsearchdlg state <<<<<<<<<<<<<<<<<<<<<<<< */


INT_PTR CALLBACK qsearchDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL qsearchIsFindHistoryEnabled(void);
BOOL qsearchIsFindHistoryBeingSaved(void);
BOOL qsearchIsSearchFlagsBeingSaved(void);
BOOL qsearchIsSavingHistoryToStdLocation(void);

INT_PTR qsearchDlgOnAltHotkey(HWND hDlg, WPARAM wParam);
void qsearchDlgApplyEditorColors(void);

void qsUpdateHighlightForFindAll(const wchar_t* cszFindWhat, DWORD dwFindAllFlags, BOOL bForceHighlight);

#ifdef _DEBUG
  #define qsSetInfoOccurrencesFound_Tracking(nOccurrences, nFlags, comment) { Debug_OutputA(comment ## " -> qsSetInfoOccurrencesFound\n"); qsSetInfoOccurrencesFound(nOccurrences, nFlags); }
  #define qsSetInfoEmpty_Tracking(comment)  { Debug_OutputA(comment ## " -> qsSetInfoEmpty\n"); qsSetInfoEmpty(); }
#else
  #define qsSetInfoOccurrencesFound_Tracking(nOccurrences, nFlags, comment) qsSetInfoOccurrencesFound(nOccurrences, nFlags)
  #define qsSetInfoEmpty_Tracking(comment)  qsSetInfoEmpty()
#endif

#define QS_SIOF_REMOVECURRENTMATCH 0x01
void qsSetInfoOccurrencesFound(unsigned int nOccurrences, unsigned int nFlags);

void qsSetInfoEmpty(void);

BOOL qsIsHotKeyPressed(DWORD dwHotKey);

// plugin call helpers
void CallPluginFuncA(const char* cszFuncA, void* pParams);
void CallPluginFuncW(const wchar_t* cszFuncW, void* pParams);

/* >>>>>>>>>>>>>>>>>>>>>>>> highlight plugin >>>>>>>>>>>>>>>>>>>>>>>> */
#define DLLA_HIGHLIGHT_MARK                2
#define DLLA_HIGHLIGHT_UNMARK              3
#define DLLA_HIGHLIGHT_FINDMARK            4

#define DLLA_CODER_SETALIAS         6
#define DLLA_CODER_GETALIAS         18
#define MAX_CODERALIAS              MAX_PATH

#define DLLA_CODER_GETVARIABLE      22

#define MARKFLAG_MATCHCASE 0x01
#define MARKFLAG_REGEXP    0x02
#define MARKFLAG_WHOLEWORD 0x04

// DLL External Call
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

typedef struct sDLLECCODERSETTINGS_GETALIAS {
    UINT_PTR dwStructSize;
    INT_PTR nAction;
    HWND hWndEdit;
    AEHDOC hDoc;
    unsigned char* pszAlias;
} DLLECCODERSETTINGS_GETALIAS;

typedef struct sDLLECCODERSETTINGS_SETALIAS {
    UINT_PTR dwStructSize;
    INT_PTR nAction;
    const unsigned char* pszAlias;
} DLLECCODERSETTINGS_SETALIAS;

typedef struct sDLLCODERSETTINGS_GETVARIABLE {
    UINT_PTR dwStructSize;
    INT_PTR nAction;
    HWND hEditWnd;
    LPVOID hEditDoc;
    LPCWSTR pszVarName;
    LPWSTR pszVarValue;
    INT_PTR* pnVarValueLen;
} DLLCODERSETTINGS_GETVARIABLE;

void CallHighlightMain(void* phlParams);
void CallCoderSettings(void* pstParams);
BOOL IsHighlightMainActive(void);
void GetCoderAliasW(wchar_t* pszAliasBufW);
void SetCoderAliasW(const wchar_t* cszAliasBufW);
INT_PTR GetCoderVariableW(HWND hWndEdit, const wchar_t* cszVarName, wchar_t* pszVarValue);
/* <<<<<<<<<<<<<<<<<<<<<<<< highlight plugin <<<<<<<<<<<<<<<<<<<<<<<< */

/* >>>>>>>>>>>>>>>>>>>>>>>> log plugin >>>>>>>>>>>>>>>>>>>>>>>> */
// DLL External Call
typedef struct sDLLECLOG_OUTPUT_1 {
    UINT_PTR dwStructSize;
    INT_PTR nAction;
    LPCWSTR pszProgram;
    LPCWSTR pszWorkDir;
    LPCWSTR pszRePattern;
    LPCWSTR pszReTags;
    INT_PTR nInputCodepage;
    INT_PTR nOutputCodepage;
    UINT_PTR nFlags;
    LPCWSTR pszAlias;
} DLLECLOG_OUTPUT_1;

typedef struct sDLLECLOG_OUTPUT_2 {
    UINT_PTR dwStructSize;
    INT_PTR nAction;
    void* ptrToEditWnd;
} DLLECLOG_OUTPUT_2;

typedef struct sDLLECLOG_OUTPUT_4 {
    UINT_PTR dwStructSize;
    INT_PTR nAction;
    LPCWSTR pszText;
    INT_PTR nTextLen;
    INT_PTR nAppend;
    INT_PTR nCodepage;
    LPCWSTR pszAlias;
} DLLECLOG_OUTPUT_4;

void CallLogOutput(void* ploParams);
BOOL IsLogOutputActive(void);
HWND LogOutput_GetEditHwnd(void);
/* <<<<<<<<<<<<<<<<<<<<<<<< log plugin <<<<<<<<<<<<<<<<<<<<<<<< */

//// Utils ////
void strcpyAorW(LPWSTR lpDst, LPCWSTR lpSrc);
int  strcmpAorW(LPCWSTR lpStr1, LPCWSTR lpStr2, BOOL bMatchCase);

//---------------------------------------------------------------------------
#endif
