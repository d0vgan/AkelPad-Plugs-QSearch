#define AEC_FUNCTIONS

#include "QSearch.h"
#include "QSearchDlg.h"
#include "QSearchSettDlg.h"
#include "QSearchLng.h"
#include "XMemStrFunc.h"


#define  QSEARCH_FIRST       0x000001
#define  QSEARCH_NEXT        0x000002
#define  QSEARCH_FINDALL     0x000010
#define  QSEARCH_SEL         0x000100
#define  QSEARCH_SEL_FINDUP  0x000200
#define  QSEARCH_NOFINDUP    0x001000
#define  QSEARCH_NOFINDBEGIN 0x002000
#define  QSEARCH_FINDUP      0x004000
#define  QSEARCH_FINDBEGIN   0x008000
#define  QSEARCH_USEDELAY    0x010000

#define  QSEARCH_EOF_DOWN    0x0001
#define  QSEARCH_EOF_UP      0x0002

#define  VK_QS_FINDBEGIN     VK_MENU     // Alt
#define  VK_QS_FINDUP        VK_SHIFT    // Shift
#define  VK_QS_PICKUPTEXT    VK_CONTROL  // Ctrl

#define  VK_QS_WW_SRCH_MODE  VK_CONTROL  // Ctrl


// extern vars
extern PluginState     g_Plugin;
extern QSearchDlgState g_QSearchDlg;
extern QSearchOpt      g_Options;
extern wchar_t         g_szFunctionQSearchW[128];
extern BOOL            g_bHighlightPlugin;


// helpers
static const wchar_t* getFileNameW(const wchar_t* cszFilePath)
{
    const wchar_t* p = cszFilePath;
    p += lstrlenW(cszFilePath);
    while ( p != cszFilePath )
    {
        --p;
        switch ( *p )
        {
            case L'\\':
            case L'/':
                return (p + 1);
        }
    }
    return cszFilePath;
}


// plugin call helpers
static void CallPluginFuncA(const char* cszFuncA, void* pParams)
{
    PLUGINCALLSENDA pcsA;

    pcsA.pFunction = cszFuncA;
    //pcsA.bOnStart = FALSE;
    pcsA.lParam = (LPARAM) pParams;
    pcsA.dwSupport = 0;

    SendMessageA( g_Plugin.hMainWnd, AKD_DLLCALLA, 0, (LPARAM) &pcsA );
}
static void CallPluginFuncW(const wchar_t* cszFuncW, void* pParams)
{
    PLUGINCALLSENDW pcsW;

    pcsW.pFunction = cszFuncW;
    //pcsW.bOnStart = FALSE;
    pcsW.lParam = (LPARAM) pParams;
    pcsW.dwSupport = 0;

    SendMessageW( g_Plugin.hMainWnd, AKD_DLLCALLW, 0, (LPARAM) &pcsW );
}


/* >>>>>>>>>>>>>>>>>>>>>>>> highlight plugin >>>>>>>>>>>>>>>>>>>>>>>> */
#define DLLA_HIGHLIGHT_MARK                2
#define DLLA_HIGHLIGHT_UNMARK              3
#define DLLA_HIGHLIGHT_FINDMARK            4

#define DLLA_CODER_SETALIAS         6
#define DLLA_CODER_GETALIAS         18
#define MAX_CODERALIAS              MAX_PATH

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

const char*    cszHighlightMainA = "Coder::HighLight";
const wchar_t* cszHighlightMainW = L"Coder::HighLight";

const char*    cszCoderSettingsA = "Coder::Settings";
const wchar_t* cszCoderSettingsW = L"Coder::Settings";

static void CallHighlightMain(void* phlParams)
{
    if ( g_Plugin.bOldWindows )
    {
        CallPluginFuncA(cszHighlightMainA, phlParams);
    }
    else
    {
        CallPluginFuncW(cszHighlightMainW, phlParams);
    }
}

static void CallCoderSettings(void* pstParams)
{
    if ( g_Plugin.bOldWindows )
    {
        CallPluginFuncA(cszCoderSettingsA, pstParams);
    }
    else
    {
        CallPluginFuncW(cszCoderSettingsW, pstParams);
    }
}

static void getCoderAliasW(wchar_t* pszAliasBufW)
{
    DLLECCODERSETTINGS_GETALIAS stParams;

    if ( pszAliasBufW )
        pszAliasBufW[0] = 0;

    stParams.dwStructSize = sizeof(DLLECCODERSETTINGS_GETALIAS);
    stParams.nAction = DLLA_CODER_GETALIAS;
    stParams.hWndEdit = NULL;
    stParams.hDoc = NULL;
    stParams.pszAlias = (unsigned char *) pszAliasBufW;

    CallCoderSettings( &stParams );
}

static void setCoderAliasW(const wchar_t* pszAliasBufW)
{
    DLLECCODERSETTINGS_SETALIAS stParams;

    stParams.dwStructSize = sizeof(DLLECCODERSETTINGS_SETALIAS);
    stParams.nAction = DLLA_CODER_SETALIAS;
    stParams.pszAlias = (const unsigned char *) pszAliasBufW;

    CallCoderSettings( &stParams );
}
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

typedef struct sDLLECLOG_OUTPUT_4 {
    UINT_PTR dwStructSize;
    INT_PTR nAction;
    LPCWSTR pszText;
    INT_PTR nTextLen;
    INT_PTR nAppend;
    INT_PTR nCodepage;
    LPCWSTR pszAlias;
} DLLECLOG_OUTPUT_4;

const char*    cszLogOutputA = "Log::Output";
const wchar_t* cszLogOutputW = L"Log::Output";

static void CallLogOutput(void* ploParams)
{
    if ( g_Plugin.bOldWindows )
    {
        CallPluginFuncA(cszLogOutputA, ploParams);
    }
    else
    {
        CallPluginFuncW(cszLogOutputW, ploParams);
    }
}
/* <<<<<<<<<<<<<<<<<<<<<<<< log plugin <<<<<<<<<<<<<<<<<<<<<<<< */


/* >>>>>>>>>>>>>>>>>>>>>>>> qsearchdlg state >>>>>>>>>>>>>>>>>>>>>>>> */
    void initializeQSearchDlgState(QSearchDlgState* pQSearchDlg)
    {
        pQSearchDlg->hDlg = NULL;
        pQSearchDlg->hFindEdit = NULL;
        pQSearchDlg->hBtnFindNext = NULL;
        pQSearchDlg->hBtnFindPrev = NULL;
        pQSearchDlg->hBtnFindAll = NULL;
        pQSearchDlg->hStInfo = NULL;
        pQSearchDlg->hPopupMenu = NULL;
        pQSearchDlg->hFindAllPopupMenu = NULL;
        pQSearchDlg->bMatchCase = FALSE;
        //pQSearchDlg->bOnDlgStart = FALSE;
        pQSearchDlg->bQSearching = FALSE;
        pQSearchDlg->bIsQSearchingRightNow = FALSE;
        pQSearchDlg->bMouseJustLeavedFindEdit = FALSE;
        pQSearchDlg->pDockData = NULL;
        pQSearchDlg->pSearchResultsFrame = NULL;
        pQSearchDlg->szFindTextW[0] = 0;
        pQSearchDlg->uSearchOrigin = QS_SO_UNKNOWN;
        pQSearchDlg->uWmShowFlags = 0;
    }
/* <<<<<<<<<<<<<<<<<<<<<<<< qsearchdlg state <<<<<<<<<<<<<<<<<<<<<<<< */


// static (local) vars
static WNDPROC prev_editWndProc = NULL;
static WNDPROC prev_btnFindNextWndProc = NULL;
static WNDPROC prev_btnFindPrevWndProc = NULL;
static WNDPROC prev_btnFindAllWndProc = NULL;
static WNDPROC prev_chWholeWordWndProc = NULL;
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

static UINT_PTR nFindAllTimerId = 0;
static CRITICAL_SECTION csFindAllTimerId;


// Helpers for find all...
// There is a single reason for so many helpers.
// The reason is: Flexibility.

// FindAllFlags
#define QS_FAF_SPECCHAR  0x0001
#define QS_FAF_REGEXP    0x0002
#define QS_FAF_MATCHCASE 0x0010
#define QS_FAF_WHOLEWORD 0x0020

typedef void (*tShowFindResults_Init)(const wchar_t* cszFindWhat, tDynamicBuffer* pBuf, tDynamicBuffer* pResultsBuf, DWORD dwFindAllFlags, const EDITINFO* pEditInfo);
typedef void (*tShowFindResults_AddOccurrence)(const tDynamicBuffer* pOccurrence, tDynamicBuffer* pResultsBuf);
typedef void (*tShowFindResults_Done)(unsigned int nOccurrences, tDynamicBuffer* pResultsBuf);

// CountOnly...
static void qsShowFindResults_CountOnly_Init(const wchar_t* cszFindWhat, tDynamicBuffer* pBuf, tDynamicBuffer* pResultsBuf, DWORD dwFindAllFlags, const EDITINFO* pEditInfo)
{
    // empty
}

static void qsShowFindResults_CountOnly_AddOccurrence(const tDynamicBuffer* pOccurrence, tDynamicBuffer* pResultsBuf)
{
    // empty
}

static void qsShowFindResults_CountOnly_Done(unsigned int nOccurrences, tDynamicBuffer* pResultsBuf)
{
    const wchar_t* cszTextFormat;
    int nLen;
    wchar_t szText[128];

    cszTextFormat = qsearchGetStringW(QS_STRID_FINDALL_OCCURRENCESFOUND);
    nLen = wsprintfW(szText, cszTextFormat, nOccurrences);
    if ( nLen > 0 )
    {
        --nLen;
        szText[nLen] = 0; // without the trailing '.'
    }
    SetWindowTextW(g_QSearchDlg.hStInfo, szText);
}

// LogOutput...
static void LogOutput_AddText(const wchar_t* cszText, UINT_PTR nLen)
{
    DLLECLOG_OUTPUT_4 loParams;

    loParams.dwStructSize = sizeof(DLLECLOG_OUTPUT_4);
    loParams.nAction = 4;
    loParams.pszText = cszText;
    loParams.nTextLen = nLen;
    loParams.nAppend = 2; // 2 = new line
    loParams.nCodepage = 0;
    loParams.pszAlias = NULL;
    CallLogOutput( &loParams );
}

static UINT_PTR formatSearchingForStringToBuf(tDynamicBuffer* pBuf, const wchar_t* cszFindWhat, DWORD dwFindAllFlags, const EDITINFO* pEditInfo)
{
    wchar_t* pszText;
    const wchar_t* cszTextFormat;
    const wchar_t* cszFileName;
    wchar_t chQuote;
    UINT_PTR nBytesToAllocate;
    UINT_PTR nLen;

    chQuote = L'\"';
    if ( dwFindAllFlags & QS_FAF_REGEXP )
        chQuote = L'/';

    //if ( dwFindAllFlags & QS_FAF_MATCHCASE )
    //    ...

    //if ( dwFindAllFlags & QS_FAF_WHOLEWORD )
    //    ...

    cszTextFormat = qsearchGetStringW(QS_STRID_FINDALL_SEARCHINGFOR);

    if ( pEditInfo->pFile )
        cszFileName = getFileNameW( (const wchar_t *) pEditInfo->pFile );
    else
        cszFileName = L"";

    nBytesToAllocate = lstrlenW(cszTextFormat);
    nBytesToAllocate += lstrlenW(cszFindWhat);
    nBytesToAllocate += lstrlenW(cszFileName);
    nBytesToAllocate += 1;
    nBytesToAllocate *= sizeof(wchar_t);

    if ( !tDynamicBuffer_Allocate(pBuf, nBytesToAllocate) )
        return 0; // failed to allocate the memory

    pszText = (wchar_t *) pBuf->ptr;
    nLen = (UINT_PTR) wsprintfW(pszText, cszTextFormat, chQuote, cszFindWhat, chQuote, cszFileName);
    pBuf->nBytesStored = nLen*sizeof(wchar_t);
    return nLen;
}

static void qsShowFindResults_LogOutput_Init(const wchar_t* cszFindWhat, tDynamicBuffer* pBuf, tDynamicBuffer* pResultsBuf, DWORD dwFindAllFlags, const EDITINFO* pEditInfo)
{
    DLLECLOG_OUTPUT_1 loParams;
    wchar_t szCoderAlias[MAX_CODERALIAS + 1];

    if ( g_Options.dwFindAllResult & QS_FINDALL_RSLT_CODERALIAS )
    {
        getCoderAliasW(szCoderAlias);
    }
    else
    {
        szCoderAlias[0] = 0;
    }

    loParams.dwStructSize = sizeof(DLLECLOG_OUTPUT_1);
    loParams.nAction = 1;
    loParams.pszProgram = NULL;
    loParams.pszWorkDir = NULL;
    loParams.pszRePattern = L"^\\((\\d+),(\\d+)\\)"; // corresponds to the output string format:
    loParams.pszReTags = L"/GOTOLINE=\\1:\\2";
    loParams.nInputCodepage = -2;
    loParams.nOutputCodepage = -2;
    loParams.nFlags = 2; // 2 = no input line
    loParams.pszAlias = szCoderAlias[0] ? szCoderAlias : NULL;

    CallLogOutput( &loParams );

    if ( (g_Options.dwFindAllResult & QS_FINDALL_RSLT_FILTERMODE) == 0 )
    {
        if ( g_Options.dwFindAllResult & QS_FINDALL_RSLT_SEARCHING )
        {
            UINT_PTR nLen = formatSearchingForStringToBuf(pBuf, cszFindWhat, dwFindAllFlags, pEditInfo);
            if ( nLen == 0 )
                return; // failed

            LogOutput_AddText( (const wchar_t *) pBuf->ptr, nLen );
        }
    }
}

static void qsShowFindResults_LogOutput_AddOccurrence(const tDynamicBuffer* pOccurrence, tDynamicBuffer* pResultsBuf)
{
    tDynamicBuffer_Append( pResultsBuf, pOccurrence->ptr, pOccurrence->nBytesStored );
    tDynamicBuffer_Append( pResultsBuf, L"\r", 1*sizeof(wchar_t) );
}

static void qsShowFindResults_LogOutput_Done(unsigned int nOccurrences, tDynamicBuffer* pResultsBuf)
{
    const wchar_t* cszTextFormat;
    UINT_PTR nLen;
    wchar_t szText[128];

    nLen = pResultsBuf->nBytesStored/sizeof(wchar_t); // without the trailing '\0'
    tDynamicBuffer_Append( pResultsBuf, L"\0", 1*sizeof(wchar_t) ); // the trailing '\0'
    LogOutput_AddText( (const wchar_t*) pResultsBuf->ptr, nLen );

    // the szText is also used in g_QSearchDlg.hStInfo
    cszTextFormat = qsearchGetStringW(QS_STRID_FINDALL_OCCURRENCESFOUND);
    nLen = (UINT_PTR) wsprintfW(szText, cszTextFormat, nOccurrences);

    if ( (g_Options.dwFindAllResult & QS_FINDALL_RSLT_FILTERMODE) == 0 )
    {
        if ( g_Options.dwFindAllResult & QS_FINDALL_RSLT_OCCFOUND )
        {
            LogOutput_AddText(szText, nLen);
        }
    }

    if ( nLen > 0 )
    {
        --nLen;
        szText[nLen] = 0; // without the trailing '.'
    }
    SetWindowTextW(g_QSearchDlg.hStInfo, szText);
}

// FileOutput...
static void qsShowFindResults_FileOutput_Init(const wchar_t* cszFindWhat, tDynamicBuffer* pBuf, tDynamicBuffer* pResultsBuf, DWORD dwFindAllFlags, const EDITINFO* pEditInfo)
{
    if ( (g_Options.dwFindAllResult & QS_FINDALL_RSLT_FILTERMODE) == 0 )
    {
        if ( g_Options.dwFindAllResult & QS_FINDALL_RSLT_SEARCHING )
        {
            if ( formatSearchingForStringToBuf(pBuf, cszFindWhat, dwFindAllFlags, pEditInfo) == 0 )
                return; // failed

            tDynamicBuffer_Append( pResultsBuf, pBuf->ptr, pBuf->nBytesStored );
            tDynamicBuffer_Append( pResultsBuf, L"\r", 1*sizeof(wchar_t) );
        }
    }
}

static void qsShowFindResults_FileOutput_AddOccurrence(const tDynamicBuffer* pOccurrence, tDynamicBuffer* pResultsBuf)
{
    tDynamicBuffer_Append( pResultsBuf, pOccurrence->ptr, pOccurrence->nBytesStored );
    tDynamicBuffer_Append( pResultsBuf, L"\r", 1*sizeof(wchar_t) );
}

static void qsShowFindResults_FileOutput_Done(unsigned int nOccurrences, tDynamicBuffer* pResultsBuf)
{
    const wchar_t* cszTextFormat;
    UINT_PTR nLen;
    BOOL bOutputResult;
    wchar_t  szText[128];
    wchar_t  szCoderAlias[MAX_CODERALIAS + 1];

    if ( g_Options.dwFindAllResult & QS_FINDALL_RSLT_CODERALIAS )
    {
        getCoderAliasW(szCoderAlias);
    }
    else
    {
        szCoderAlias[0] = 0;
    }

    // the szText is also used in g_QSearchDlg.hStInfo
    cszTextFormat = qsearchGetStringW(QS_STRID_FINDALL_OCCURRENCESFOUND);
    nLen = (UINT_PTR) wsprintfW(szText, cszTextFormat, nOccurrences);

    if ( ((g_Options.dwFindAllResult & QS_FINDALL_RSLT_FILTERMODE) == 0) &&
         (g_Options.dwFindAllResult & QS_FINDALL_RSLT_OCCFOUND) )
    {
        tDynamicBuffer_Append( pResultsBuf, szText, nLen*sizeof(wchar_t) );
    }
    else
    {
        if ( pResultsBuf->nBytesStored != 0 )
            pResultsBuf->nBytesStored -= sizeof(wchar_t); // exclude the trailing '\r'
    }

    tDynamicBuffer_Append( pResultsBuf, L"\0", 1*sizeof(wchar_t) ); // the trailing '\0'

    bOutputResult = FALSE;

    if ( ((g_Options.dwFindAllMode & QS_FINDALL_MASK) == QS_FINDALL_FILEOUTPUT_SNGL) &&
         (g_Plugin.nMDI == WMD_SDI) )
    {
        bOutputResult = TRUE;
    }
    else if ( ((g_Options.dwFindAllMode & QS_FINDALL_MASK) == QS_FINDALL_FILEOUTPUT_SNGL) &&
              (g_QSearchDlg.pSearchResultsFrame != NULL) && 
              SendMessageW(g_Plugin.hMainWnd, AKD_FRAMEISVALID, 0, (LPARAM) g_QSearchDlg.pSearchResultsFrame) )
    {
        SendMessageW(g_Plugin.hMainWnd, AKD_FRAMEACTIVATE, 0, (LPARAM) g_QSearchDlg.pSearchResultsFrame);
        bOutputResult = TRUE;
    }
    else if ( SendMessageW(g_Plugin.hMainWnd, WM_COMMAND, IDM_FILE_NEW, 0) == TRUE )
    {
        g_QSearchDlg.pSearchResultsFrame = NULL;
        bOutputResult = TRUE;
    }

    if ( bOutputResult )
    {
        EDITINFO  ei;

        if ( g_QSearchDlg.pSearchResultsFrame == NULL )
        {
            g_QSearchDlg.pSearchResultsFrame = (FRAMEDATA *) SendMessageW( g_Plugin.hMainWnd, AKD_FRAMEFIND, FWF_CURRENT, 0 );
        }

        ei.hWndEdit = NULL;
        SendMessageW( g_Plugin.hMainWnd, AKD_GETEDITINFO, (WPARAM) NULL, (LPARAM) &ei );
        if ( ei.hWndEdit )
        {
            AEAPPENDTEXTW aeatW;
            AECHARINDEX aeci;

            if ( szCoderAlias[0] )
            {
                setCoderAliasW(szCoderAlias);
            }

            if ( ((g_Options.dwFindAllMode & QS_FINDALL_MASK) == QS_FINDALL_FILEOUTPUT_SNGL) )
            {
                if ( SendMessageW(g_Plugin.hMainWnd, AKD_GETTEXTLENGTH, (WPARAM) ei.hWndEdit, 0) != 0 )
                {
                    aeatW.pText = L"\r\r";
                    aeatW.dwTextLen = (UINT_PTR) (-1);
                    aeatW.nNewLine = AELB_ASINPUT;
                    SendMessageW( ei.hWndEdit, AEM_APPENDTEXTW, 0, (LPARAM) &aeatW );
                }
            }

            /* SendMessageW( ei.hWndEdit, EM_REPLACESEL, FALSE, (LPARAM) pResultsBuf->ptr ); */
            aeatW.pText = (const wchar_t *) pResultsBuf->ptr;
            aeatW.dwTextLen = pResultsBuf->nBytesStored/sizeof(wchar_t) - 1; // excluding the trailing '\0'
            aeatW.nNewLine = AELB_ASINPUT;
            SendMessageW( ei.hWndEdit, AEM_APPENDTEXTW, 0, (LPARAM) &aeatW );

            SendMessageW( ei.hWndEdit, AEM_GETINDEX, AEGI_LASTCHAR, (LPARAM) &aeci );
            SendMessageW( ei.hWndEdit, AEM_EXSETSEL, (WPARAM) &aeci, (LPARAM) &aeci );
        }
    }

    if ( nLen > 0 )
    {
        --nLen;
        szText[nLen] = 0; // without the trailing '.'
    }
    SetWindowTextW(g_QSearchDlg.hStInfo, szText);
}

typedef struct sShowFindResults {
    tShowFindResults_Init pfnInit;                   // can't be NULL
    tShowFindResults_AddOccurrence pfnAddOccurrence; // can't be NULL
    tShowFindResults_Done pfnDone;                   // can't be NULL
} tShowFindResults;

typedef void (*tStoreResultCallback)(HWND hWndEdit, const AECHARRANGE* pcrFound, const tDynamicBuffer* pFindResult, tDynamicBuffer* pBuf, tDynamicBuffer* pResultsBuf, tShowFindResults_AddOccurrence pfnAddOccurrence);

static void qsStoreResultCallback(HWND hWndEdit, const AECHARRANGE* pcrFound, const tDynamicBuffer* pFindResult, tDynamicBuffer* pBuf, tDynamicBuffer* pResultsBuf, tShowFindResults_AddOccurrence pfnAddOccurrence)
{
    wchar_t* pStr;
    UINT_PTR nBytesToAllocate;

    nBytesToAllocate = pFindResult->nBytesStored;
    if ( (g_Options.dwFindAllResult & QS_FINDALL_RSLT_FILTERMODE) == 0 )
    {
        if ( g_Options.dwFindAllResult & QS_FINDALL_RSLT_POS )
        {
            nBytesToAllocate += 32*sizeof(wchar_t);
        }
        if ( g_Options.dwFindAllResult & QS_FINDALL_RSLT_LEN )
        {
            nBytesToAllocate += 16*sizeof(wchar_t);
        }
    }

    if ( !tDynamicBuffer_Allocate(pBuf, nBytesToAllocate) )
        return; // failed to allocate the memory

    // constructing the output string...
    pBuf->nBytesStored = 0;

    if ( (g_Options.dwFindAllResult & QS_FINDALL_RSLT_FILTERMODE) == 0 )
    {
        if ( g_Options.dwFindAllResult & QS_FINDALL_RSLT_POS )
        {
            INT_X nLinePos;
            int nUnwrappedLine;
            AECHARINDEX ci;

            if ( SendMessage(hWndEdit, AEM_GETWORDWRAP, 0, 0) != AEWW_NONE )
                nUnwrappedLine = (int) SendMessage(hWndEdit, AEM_GETUNWRAPLINE, pcrFound->ciMin.nLine, 0);
            else
                nUnwrappedLine = pcrFound->ciMin.nLine;

            x_mem_cpy(&ci, &pcrFound->ciMin, sizeof(AECHARINDEX));
            nLinePos = AEC_WrapLineBegin(&ci);

            pStr = (wchar_t *) pBuf->ptr;
            pBuf->nBytesStored = sizeof(wchar_t) * (UINT_PTR) wsprintfW( pStr, L"(%d,%d)\t", nUnwrappedLine + 1, (int) (nLinePos + 1) );
        }

        if ( g_Options.dwFindAllResult & QS_FINDALL_RSLT_LEN )
        {
            int nLen;
            AECHARINDEX ciBegin;
            AECHARINDEX ciEnd;
            AEINDEXSUBTRACT aeis;

            x_mem_cpy( &ciBegin, &pcrFound->ciMin, sizeof(AECHARINDEX) );
            x_mem_cpy( &ciEnd, &pcrFound->ciMax, sizeof(AECHARINDEX) );
            aeis.ciChar1 = &ciEnd;
            aeis.ciChar2 = &ciBegin;
            aeis.bColumnSel = FALSE;
            aeis.nNewLine = AELB_ASIS;
            nLen = (int) SendMessage( hWndEdit, AEM_INDEXSUBTRACT, 0, (LPARAM) &aeis );

            pStr = (wchar_t *) pBuf->ptr;
            if ( pBuf->nBytesStored != 0 )
            {
                pBuf->nBytesStored -= sizeof(wchar_t); // exclude the trailing L'\t'
                pStr += pBuf->nBytesStored/sizeof(wchar_t);
            }

            pBuf->nBytesStored += sizeof(wchar_t) * (UINT_PTR) wsprintfW( pStr, L"(%d)\t", nLen );
        }
    }

    tDynamicBuffer_Append(pBuf, pFindResult->ptr, pFindResult->nBytesStored);

    pfnAddOccurrence(pBuf, pResultsBuf);
}

#define QSFRM_LINE       1 // number of lines
#define QSFRM_CHAR       2 // number of chars
#define QSFRM_CHARINLINE 3 // number of chars within the current line

typedef struct sGetFindResultPolicy {
    short int nMode;       // one of QSFRM_*
    short int nBefore;     // number of lines or chars before the search result
    short int nAfter;      // number of lines or chars after the search result
    short int nMaxLineLen; // max line length to show (0 - no limit)
    tStoreResultCallback pfnStoreResultCallback;
} tGetFindResultPolicy;
// nBefore=0 and nAfter=0 with nMode=QSFRM_LINE means: whole line
// nBefore=0 and nAfter=0 with nMode=QSFRM_CHAR means: just the matching word

typedef void (*tFindResultCallback)(HWND hWndEdit, const AECHARRANGE* pcrFound, const tGetFindResultPolicy* pfrPolicy, tDynamicBuffer* pBuf, tDynamicBuffer* pBuf2, tDynamicBuffer* pResultsBuf, tShowFindResults_AddOccurrence pfnAddOccurrence);

static void qsFindResultCallback(HWND hWndEdit, const AECHARRANGE* pcrFound, const tGetFindResultPolicy* pfrPolicy, tDynamicBuffer* pBuf, tDynamicBuffer* pBuf2, tDynamicBuffer* pResultsBuf, tShowFindResults_AddOccurrence pfnAddOccurrence)
{
    AETEXTRANGEW tr;

    if ( !pfrPolicy->pfnStoreResultCallback )
        return; // no sense to retrieve the find result

    x_zero_mem( &tr, sizeof(AETEXTRANGEW) );
    x_mem_cpy( &tr.cr, pcrFound, sizeof(AECHARRANGE) );

    if ( (g_Options.dwFindAllResult & QS_FINDALL_RSLT_MATCHONLY) == 0 )
    {
        // TODO: check pfrPolicy->nMaxLineLen
        if ( pfrPolicy->nMode == QSFRM_CHAR )
        {
            AEINDEXOFFSET io;

            if ( pfrPolicy->nBefore > 0 )
            {
                io.ciCharIn = (AECHARINDEX *) &pcrFound->ciMin;
                io.ciCharOut = &tr.cr.ciMin;
                io.nOffset = -pfrPolicy->nBefore;
                io.nNewLine = AELB_R;
                SendMessage( hWndEdit, AEM_INDEXOFFSET, 0, (LPARAM) &io );
            }
            if ( pfrPolicy->nAfter > 0)
            {
                io.ciCharIn = (AECHARINDEX *) &pcrFound->ciMax;
                io.ciCharOut = &tr.cr.ciMax;
                io.nOffset = pfrPolicy->nAfter;
                io.nNewLine = AELB_R;
                SendMessage( hWndEdit, AEM_INDEXOFFSET, 0, (LPARAM) &io );
            }
        }
        else if ( pfrPolicy->nMode == QSFRM_CHARINLINE )
        {
            if ( tr.cr.ciMin.nCharInLine > pfrPolicy->nBefore )
                tr.cr.ciMin.nCharInLine -= pfrPolicy->nBefore;
            else
                tr.cr.ciMin.nCharInLine = 0;

            if ( tr.cr.ciMax.nCharInLine + pfrPolicy->nAfter < tr.cr.ciMax.lpLine->nLineLen )
                tr.cr.ciMax.nCharInLine += pfrPolicy->nAfter;
            else
                tr.cr.ciMax.nCharInLine = tr.cr.ciMax.lpLine->nLineLen;
        }
        else // QSFRM_LINE
        {
            if ( pfrPolicy->nBefore == 0 && pfrPolicy->nAfter == 0 )
            {
                tr.cr.ciMin.nCharInLine = 0;
                tr.cr.ciMax.nCharInLine = tr.cr.ciMax.lpLine->nLineLen;
            }
            else
            {
                INT_X  nLineStartPos;
                int    nUnwrappedLine;
                int    nLineCount;

                if ( pfrPolicy->nBefore > 0 )
                {
                    if ( SendMessage(hWndEdit, AEM_GETWORDWRAP, 0, 0) != AEWW_NONE )
                        nUnwrappedLine = (int) SendMessage( hWndEdit, AEM_GETUNWRAPLINE, (WPARAM) tr.cr.ciMin.nLine, 0 );
                    else
                        nUnwrappedLine = tr.cr.ciMin.nLine;

                    if ( nUnwrappedLine > pfrPolicy->nBefore )
                        nUnwrappedLine -= pfrPolicy->nBefore;
                    else
                        nUnwrappedLine = 0;

                    nLineStartPos = (INT_X) SendMessage( hWndEdit, EM_LINEINDEX, nUnwrappedLine, 0 );
                    SendMessage( hWndEdit, AEM_RICHOFFSETTOINDEX, (WPARAM) nLineStartPos, (LPARAM) &tr.cr.ciMin );
                }
                if ( pfrPolicy->nAfter > 0)
                {
                    if ( SendMessage(hWndEdit, AEM_GETWORDWRAP, 0, 0) != AEWW_NONE )
                        nUnwrappedLine = (int) SendMessage( hWndEdit, AEM_GETUNWRAPLINE, (WPARAM) tr.cr.ciMax.nLine, 0 );
                    else
                        nUnwrappedLine = tr.cr.ciMax.nLine;

                    nLineCount = (int) SendMessage( hWndEdit, EM_GETLINECOUNT, 0, 0 );
                    if ( nUnwrappedLine + pfrPolicy->nAfter < nLineCount )
                        nUnwrappedLine += pfrPolicy->nAfter;
                    else
                        nUnwrappedLine = nLineCount;

                    nLineStartPos = (INT_X) SendMessage( hWndEdit, EM_LINEINDEX, nUnwrappedLine, 0 );
                    SendMessage( hWndEdit, AEM_RICHOFFSETTOINDEX, (WPARAM) nLineStartPos, (LPARAM) &tr.cr.ciMax );
                }
            }
        }
    }

    tr.bColumnSel = FALSE;
    tr.pBuffer = NULL;
    tr.nNewLine = AELB_ASIS;
    tr.bFillSpaces = FALSE;

    tr.dwBufferMax = (UINT_PTR) SendMessage( hWndEdit, AEM_GETTEXTRANGEW, 0, (LPARAM) &tr );
    if ( tr.dwBufferMax == 0 )
        return; // no text to retrieve

    if ( !tDynamicBuffer_Allocate(pBuf, sizeof(wchar_t) * tr.dwBufferMax) )
            return; // failed to allocate the memory

    tr.pBuffer = (wchar_t *) pBuf->ptr;
    tr.pBuffer[0] = 0;
    pBuf->nBytesStored = sizeof(wchar_t) * (UINT_PTR) SendMessage( hWndEdit, AEM_GETTEXTRANGEW, 0, (LPARAM) &tr );
    pfrPolicy->pfnStoreResultCallback( hWndEdit, pcrFound, pBuf, pBuf2, pResultsBuf, pfnAddOccurrence );
}

typedef struct sQSFindAll {
    tFindResultCallback pfnFindResultCallback; // can be NULL
    tGetFindResultPolicy GetFindResultPolicy;
    tShowFindResults ShowFindResults;
    tDynamicBuffer buf;
    tDynamicBuffer buf2;
} tQSFindAll;


// funcs
HWND qsearchDoInitToolTip(HWND hDlg, HWND hEdit);
void qsearchDoQuit(HWND hEdit, HWND hToolTip, HMENU hPopupMenuLoaded, HBRUSH hBrush1, HBRUSH hBrush2, HBRUSH hBrush3);
void qsearchDoSearchText(HWND hEdit, DWORD dwParams, const DWORD dwOptFlags[], tQSFindAll* pFindAll /* = NULL */);
void qsearchDoSelFind(HWND hEdit, BOOL bFindPrev, const DWORD dwOptFlags[]);
void qsearchDoSetNotFound(HWND hEdit, BOOL bNotFound, BOOL bNotRegExp, BOOL bEOF);
void qsearchDoShowHide(HWND hDlg, BOOL bShow, UINT uShowFlags, const DWORD dwOptFlags[]);
void qsearchDoTryHighlightAll(HWND hDlg, const DWORD dwOptFlags[]);
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

static UINT qs_Get_SO_QSEARCH(const DWORD dwOptFlags[])
{
    UINT uSearchOrigin;
    if ( (dwOptFlags[OPTF_EDITOR_AUTOFOCUS_MOUSE] & 0x01) &&
         (dwOptFlags[OPTF_SRCH_PICKUP_SELECTION] & 0x01) )
        uSearchOrigin = QS_SO_QSEARCH_FORCED;
    else
        uSearchOrigin = QS_SO_QSEARCH;
    return uSearchOrigin;
}

static void qsdlgShowHideWholeWordCheckBox(HWND hDlg, const DWORD dwOptFlags[])
{
    HWND    hChWholeWord;
    wchar_t wszText[64];

    hChWholeWord = GetDlgItem(hDlg, IDC_CH_WHOLEWORD);

    if ( g_Plugin.bOldWindows )
    {
        lstrcpyA( (char *) wszText, qsearchGetTextA(IDC_CH_WHOLEWORD) );
        if ( dwOptFlags[OPTF_SRCH_USE_SPECIALCHARS] )
            lstrcatA( (char *) wszText, "*" );
        else if ( dwOptFlags[OPTF_SRCH_USE_REGEXP] )
            lstrcatA( (char *) wszText, "^" );
        SetWindowTextA(hChWholeWord, (const char *) wszText);
    }
    else
    {
        lstrcpyW( wszText, qsearchGetTextW(IDC_CH_WHOLEWORD) );
        if ( dwOptFlags[OPTF_SRCH_USE_SPECIALCHARS] )
            lstrcatW( wszText, L"*" );
        else if ( dwOptFlags[OPTF_SRCH_USE_REGEXP] )
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
        dwOptFlags[OPTF_SRCH_USE_SPECIALCHARS] ? SW_HIDE : SW_SHOWNORMAL
    );

    MoveWindow(
        hChHighlightAll,
        dwOptFlags[OPTF_SRCH_USE_SPECIALCHARS] ? 
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

static void setInfoEmpty()
{
    if ( g_QSearchDlg.hStInfo )
    {
        if ( g_Plugin.bOldWindows )
            SetWindowTextA(g_QSearchDlg.hStInfo, "");
        else
            SetWindowTextW(g_QSearchDlg.hStInfo, L"");
    }
}

static BOOL getAkelPadSelectedText(wchar_t szTextAW[MAX_TEXT_SIZE], const DWORD dwOptFlags[])
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
                if ( dwOptFlags[OPTF_SRCH_USE_SPECIALCHARS] ||
                     dwOptFlags[OPTF_SRCH_USE_REGEXP] )
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
                                if ( dwOptFlags[OPTF_SRCH_USE_REGEXP] )
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
                if ( dwOptFlags[OPTF_SRCH_USE_SPECIALCHARS] ||
                     dwOptFlags[OPTF_SRCH_USE_REGEXP] )
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
                                if ( dwOptFlags[OPTF_SRCH_USE_REGEXP] )
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
                 (hWndToFocus != g_QSearchDlg.hBtnFindPrev) &&
                 (hWndToFocus != g_QSearchDlg.hBtnFindAll) )
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
    else if ( hBtn == g_QSearchDlg.hBtnFindPrev )
        prevWndProc = prev_btnFindPrevWndProc;
    else
        prevWndProc = prev_btnFindAllWndProc;
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

static wchar_t virtKeyToCharW(DWORD dwKey)
{
    UINT uCh = MapVirtualKeyW(dwKey, 2 /*MAPVK_VK_TO_CHAR*/);
    wchar_t wch = (wchar_t) (uCh & 0xFFFF);
    return wch;
}

static char virtKeyToCharA(DWORD dwKey)
{
    UINT uCh = MapVirtualKeyA(dwKey, 2 /*MAPVK_VK_TO_CHAR*/);
    char ch = (char) (uCh & 0xFF);
    return ch;
}

#define AHKF_PARENTHESES 0x01
#define AHKF_INDENT      0x02
static void strFormatAltHotkeyW(wchar_t* szHotkeyW, DWORD dwAltHotkey, UINT uFlags)
{
    wchar_t* p;

    p = szHotkeyW;
    if ( uFlags & AHKF_INDENT )
        *(p++) = L' ';
    if ( uFlags & AHKF_PARENTHESES )
        *(p++) = L'(';
    *(p++) = L'A';
    *(p++) = L'l';
    *(p++) = L't';
    *(p++) = L'+';
    *(p++) = virtKeyToCharW(dwAltHotkey);
    if ( uFlags & AHKF_PARENTHESES )
        *(p++) = L')';
    *p = 0;
}

static void strFormatAltHotkeyA(char* szHotkeyA, DWORD dwAltHotkey, UINT uFlags)
{
    char* p;

    p = szHotkeyA;
    if ( uFlags & AHKF_INDENT )
        *(p++) = ' ';
    if ( uFlags & AHKF_PARENTHESES )
        *(p++) = '(';
    *(p++) = 'A';
    *(p++) = 'l';
    *(p++) = 't';
    *(p++) = '+';
    *(p++) = virtKeyToCharA(dwAltHotkey);
    if ( uFlags & AHKF_PARENTHESES )
        *(p++) = ')';
    *p = 0;
}

static void strAppendAltHotkeyW(wchar_t* strW, DWORD dwAltHotkey)
{
    wchar_t szHotKeyW[32];

    strFormatAltHotkeyW(szHotKeyW, dwAltHotkey, AHKF_INDENT | AHKF_PARENTHESES);
    lstrcatW(strW, szHotKeyW);
}

static void strAppendAltHotkeyA(char* strA, DWORD dwAltHotkey)
{
    char szHotKeyA[32];

    strFormatAltHotkeyA(szHotKeyA, dwAltHotkey, AHKF_INDENT | AHKF_PARENTHESES);
    lstrcatA(strA, szHotKeyA);
}

static void OnChWholeWordSrchMode()
{
    if ( g_Options.dwFlags[OPTF_SRCH_USE_SPECIALCHARS] )
    {
        g_Options.dwFlags[OPTF_SRCH_USE_SPECIALCHARS] = 0;
        g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] = 1;
        CheckMenuItem( g_QSearchDlg.hPopupMenu, IDM_SRCHUSESPECIALCHARS, MF_BYCOMMAND | MF_UNCHECKED );
        CheckMenuItem( g_QSearchDlg.hPopupMenu, IDM_SRCHREGEXPDOTNEWLINE, MF_BYCOMMAND | MF_CHECKED );
        EnableMenuItem( g_QSearchDlg.hPopupMenu, IDM_SRCHREGEXPDOTNEWLINE, MF_BYCOMMAND | MF_ENABLED );
    }
    else if ( g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] )
    {
        g_Options.dwFlags[OPTF_SRCH_USE_SPECIALCHARS] = 0;
        g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] = 0;
        CheckMenuItem( g_QSearchDlg.hPopupMenu, IDM_SRCHREGEXPDOTNEWLINE, MF_BYCOMMAND | MF_UNCHECKED );
        EnableMenuItem( g_QSearchDlg.hPopupMenu, IDM_SRCHREGEXPDOTNEWLINE, MF_BYCOMMAND | MF_GRAYED );
        CheckMenuItem( g_QSearchDlg.hPopupMenu, IDM_SRCHUSESPECIALCHARS, MF_BYCOMMAND | MF_UNCHECKED );
    }
    else
    {
        g_Options.dwFlags[OPTF_SRCH_USE_SPECIALCHARS] = 1;
        g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] = 0;
        CheckMenuItem( g_QSearchDlg.hPopupMenu, IDM_SRCHREGEXPDOTNEWLINE, MF_BYCOMMAND | MF_UNCHECKED );
        EnableMenuItem( g_QSearchDlg.hPopupMenu, IDM_SRCHREGEXPDOTNEWLINE, MF_BYCOMMAND | MF_GRAYED );
        CheckMenuItem( g_QSearchDlg.hPopupMenu, IDM_SRCHUSESPECIALCHARS, MF_BYCOMMAND | MF_CHECKED );
    }

    qsdlgShowHideWholeWordCheckBox(g_QSearchDlg.hDlg, g_Options.dwFlags);
    qs_nEditEOF = 0;
    qsearchDoSetNotFound( g_QSearchDlg.hFindEdit, FALSE, FALSE, FALSE );
}

static LRESULT CALLBACK chWholeWordWndProc(HWND hCh,
  UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch ( uMsg )
    {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
            if ( GetKeyState(VK_QS_WW_SRCH_MODE) & 0x80 )
            {
                OnChWholeWordSrchMode();
                return 0;
            }
            break;

        case WM_MBUTTONDOWN:
        case WM_MBUTTONDBLCLK:
            OnChWholeWordSrchMode();
            return 0;

        case WM_NOTIFY:
            if ( g_Plugin.bOldWindows )
            {
                if ( ((LPNMHDR) lParam)->code == TTN_GETDISPINFOA )
                {
                    static char szHintA[128];
                    char szHotkeyWordA[32];
                    char szHotkeyModeA[32];
                    LPNMTTDISPINFOA lpnmdiA;

                    szHotkeyWordA[0] = 0;
                    szHotkeyModeA[0] = 0;
                    if ( g_Options.dwUseAltHotkeys )
                    {
                        strFormatAltHotkeyA(szHotkeyWordA, g_Options.dwAltWholeWord, AHKF_INDENT | AHKF_PARENTHESES);
                        strFormatAltHotkeyA(szHotkeyModeA, g_Options.dwAltSearchMode, 0);
                        lstrcatA(szHotkeyModeA, " / ");
                    }
                    wsprintfA(szHintA, qsearchGetHintA(IDC_CH_WHOLEWORD), szHotkeyWordA, szHotkeyModeA);

                    lpnmdiA = (LPNMTTDISPINFOA) lParam;
                    SendMessageA(lpnmdiA->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);
                    lpnmdiA->lpszText = szHintA;
                    return 0;
                }
            }
            else
            {
                if ( ((LPNMHDR) lParam)->code == TTN_GETDISPINFOW )
                {
                    static wchar_t szHintW[128];
                    wchar_t szHotkeyWordW[32];
                    wchar_t szHotkeyModeW[32];
                    LPNMTTDISPINFOW lpnmdiW;

                    szHotkeyWordW[0] = 0;
                    szHotkeyModeW[0] = 0;
                    if ( g_Options.dwUseAltHotkeys )
                    {
                        strFormatAltHotkeyW(szHotkeyWordW, g_Options.dwAltWholeWord, AHKF_INDENT | AHKF_PARENTHESES);
                        strFormatAltHotkeyW(szHotkeyModeW, g_Options.dwAltSearchMode, 0);
                        lstrcatW(szHotkeyModeW, L" / ");
                    }
                    wsprintfW(szHintW, qsearchGetHintW(IDC_CH_WHOLEWORD), szHotkeyWordW, szHotkeyModeW);

                    lpnmdiW = (LPNMTTDISPINFOW) lParam;
                    SendMessageW(lpnmdiW->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);
                    lpnmdiW->lpszText = szHintW;
                    return 0;
                }
            }
            break;
    }

    return callWndProc(prev_chWholeWordWndProc, hCh, uMsg, wParam, lParam);
}

BOOL qsPickUpSelection(HWND hEdit, const DWORD dwOptFlags[], BOOL isHighlightAll)
{
    wchar_t prevFindTextW[MAX_TEXT_SIZE];

    if ( g_Plugin.bOldWindows )
    {
        lstrcpyA( (LPSTR) prevFindTextW, (LPCSTR) g_QSearchDlg.szFindTextW );
    }
    else
    {
        lstrcpyW( (LPWSTR) prevFindTextW, (LPCWSTR) g_QSearchDlg.szFindTextW );
    }

    if ( getAkelPadSelectedText(g_QSearchDlg.szFindTextW, dwOptFlags) )
    {
        HWND    hDlgItm;
        BOOL    bMatchCase;

        bMatchCase = FALSE;
        if ( hDlgItm = GetDlgItem(g_QSearchDlg.hDlg, IDC_CH_MATCHCASE) )
        {
            if ( SendMessage(hDlgItm, BM_GETCHECK, 0, 0) == BST_CHECKED )
                bMatchCase = TRUE;
        }

        if ( !isHighlightAll )
        {
            qs_bEditTextChanged = FALSE;

            if ( g_Plugin.bOldWindows )
            {
                int (WINAPI *cmpfuncA)(LPCSTR, LPCSTR) = bMatchCase ? lstrcmpA : lstrcmpiA;
                if ( cmpfuncA((LPCSTR) prevFindTextW, (LPCSTR) g_QSearchDlg.szFindTextW) != 0 )
                {
                    qs_bEditTextChanged = TRUE;
                }
            }
            else
            {
                int (WINAPI *cmpfuncW)(LPCWSTR, LPCWSTR) = bMatchCase ? lstrcmpW : lstrcmpiW;
                if ( cmpfuncW((LPCWSTR) prevFindTextW, (LPCWSTR) g_QSearchDlg.szFindTextW) != 0 )
                {
                    qs_bEditTextChanged = TRUE;
                }
            }
        }
        else
        {
            qs_bEditTextChanged = TRUE;
        }

        if ( qs_bEditTextChanged )
        {
            qsearchDoSetNotFound(hEdit, FALSE, FALSE, FALSE);
            setEditFindText(hEdit, g_QSearchDlg.szFindTextW);

            SendMessage(hEdit, EM_SETSEL, 0, -1);
            qs_bEditSelJustChanged = TRUE;

            if ( dwOptFlags[OPTF_SRCH_ONTHEFLY_MODE] )
            {
                qs_bForceFindFirst = FALSE;
                qsearchDoTryHighlightAll(g_QSearchDlg.hDlg, dwOptFlags);
            }
            else
                qs_bForceFindFirst = TRUE;
        }

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

static LRESULT OnEditKeyDown_Enter_or_F3(HWND hEdit, WPARAM wParam, const DWORD dwOptFlags[])
{
    if ( GetKeyState(VK_QS_PICKUPTEXT) & 0x80 )
    {
        if ( getAkelPadSelectedText(g_QSearchDlg.szFindTextW, dwOptFlags) )
        {
            setEditFindText(hEdit, g_QSearchDlg.szFindTextW);
            SendMessage( g_QSearchDlg.hDlg, QSM_SETNOTFOUND, FALSE, 0 );
            setInfoEmpty();
        }
        SendMessage(hEdit, EM_SETSEL, 0, -1);
#ifdef _DEBUG
        Debug_Output("editWndProc, WM_KEYDOWN, (RETURN||F3)&&PickUp, SETSEL(0, -1)\n");
#endif
        qs_bEditTextChanged = TRUE;
        if ( dwOptFlags[OPTF_SRCH_ONTHEFLY_MODE] )
        {
            qs_bForceFindFirst = FALSE;
            qsearchDoTryHighlightAll(g_QSearchDlg.hDlg, dwOptFlags);
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
                    g_QSearchDlg.uSearchOrigin = qs_Get_SO_QSEARCH(dwOptFlags);
                }
                else
                {
                    SendMessage(hCombo, CB_SHOWDROPDOWN, FALSE, 0);
                    SetFocus(hEdit);
                }
                return 0;
            }
        }

        if ( (GetKeyState(VK_QS_FINDBEGIN) & 0x80) && 
             (GetKeyState(VK_QS_FINDUP) & 0x80) )
        {
            SendMessage( g_QSearchDlg.hDlg, QSM_FINDALL, g_Options.dwFindAllMode & QS_FINDALL_MASK, 0 );
        }
        else
        {
            SendMessage( g_QSearchDlg.hDlg, WM_COMMAND, IDOK, 0 );
        }
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
                            g_QSearchDlg.uSearchOrigin = qs_Get_SO_QSEARCH(g_Options.dwFlags);
                        }
                    }
                }
            }

            if ( (wParam == VK_RETURN) || (wParam == VK_F3) )
            {
                return OnEditKeyDown_Enter_or_F3(hEdit, wParam, g_Options.dwFlags);
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
                        SendMessage( g_QSearchDlg.hDlg, QSM_FINDFIRST, QSEARCH_NOFINDUP | QSEARCH_NOFINDBEGIN | QSEARCH_USEDELAY, 0 );
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
                        SendMessage( g_QSearchDlg.hDlg, QSM_FINDFIRST, QSEARCH_NOFINDUP | QSEARCH_NOFINDBEGIN | QSEARCH_USEDELAY, 0 );
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
                        g_QSearchDlg.uSearchOrigin = qs_Get_SO_QSEARCH(g_Options.dwFlags);
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
                OnEditKeyDown_Enter_or_F3(hEdit, wParam, g_Options.dwFlags);
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
                        /*if ( (g_Options.dwFlags[OPTF_EDITOR_AUTOFOCUS_MOUSE] & AUTOFOCUS_EDITOR_ALWAYS) == AUTOFOCUS_EDITOR_ALWAYS )
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
                    SendMessage( g_QSearchDlg.hDlg, QSM_FINDFIRST, QSEARCH_NOFINDUP | QSEARCH_NOFINDBEGIN | QSEARCH_USEDELAY, 0 );
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
                     (hWndToFocus != g_QSearchDlg.hBtnFindPrev) &&
                     (hWndToFocus != g_QSearchDlg.hBtnFindAll) )
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
                if ( (g_Options.dwFlags[OPTF_QSEARCH_AUTOFOCUS_MOUSE] & 0x01) ||
                     (g_Options.dwFlags[OPTF_EDITOR_AUTOFOCUS_MOUSE] & 0x01) )
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
                    SendMessageA(lpnmdiA->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);
                    lpnmdiA->lpszText = (LPSTR) qsearchGetHintA(IDC_ED_FINDTEXT);
                    return 0;
                }
            }
            else
            {
                if ( ((LPNMHDR) lParam)->code == TTN_GETDISPINFOW )
                {
                    LPNMTTDISPINFOW lpnmdiW = (LPNMTTDISPINFOW) lParam;
                    SendMessageW(lpnmdiW->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);
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

static void qsUpdateHighlight(HWND hDlg, HWND hEdit, const DWORD dwOptFlags[])
{
    wchar_t szSelectedTextW[MAX_TEXT_SIZE];

    getEditFindText(hEdit, g_QSearchDlg.szFindTextW);

    if ( getAkelPadSelectedText(szSelectedTextW, dwOptFlags) )
    {
        BOOL bEqual = FALSE;

        if ( dwOptFlags[OPTF_SRCH_USE_REGEXP] )
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

            if ( dwOptFlags[OPTF_SRCH_USE_SPECIALCHARS] )
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
            DWORD dwOptFlagsTemp[OPTF_COUNT];
            copyOptionsFlags(dwOptFlagsTemp, dwOptFlags);
            dwOptFlagsTemp[OPTF_SRCH_STOP_EOF] = 0; // disabling here
            qsearchDoSearchText( hEdit, QSEARCH_NEXT, dwOptFlagsTemp, NULL );
        }

        qsearchDoTryHighlightAll(hDlg, dwOptFlags);
    }
    else
    {
        // no text selected - trying to find & highlight
        DWORD dwOptFlagsTemp[OPTF_COUNT];
        copyOptionsFlags(dwOptFlagsTemp, dwOptFlags);
        dwOptFlagsTemp[OPTF_SRCH_STOP_EOF] = 0; // disabling here
        qsearchDoSearchText( hEdit, QSEARCH_NEXT, dwOptFlagsTemp, NULL );

        qsearchDoTryHighlightAll(hDlg, dwOptFlags);
    }
}

INT_PTR qsearchDlgOnAltHotkey(HWND hDlg, WPARAM wParam)
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
        if ( wParam == g_Options.dwAltSearchMode )
        {
            OnChWholeWordSrchMode();
            return 1;
        }
    }
    return 0;
}

static void OnChMatchCaseOrWholeWordClicked(HWND hDlg)
{
    HWND hChHighlightAll;
    BOOL bHighlightAllChecked = FALSE;
    BOOL bProcessed = FALSE;

    hChHighlightAll = GetDlgItem(hDlg, IDC_CH_HIGHLIGHTALL);
    if ( SendMessage(hChHighlightAll, BM_GETCHECK, 0, 0) == BST_CHECKED )
        bHighlightAllChecked = TRUE;

    if ( bHighlightAllChecked &&
        g_QSearchDlg.uSearchOrigin == QS_SO_EDITOR &&
        g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE] &&
        g_Options.dwFlags[OPTF_SRCH_PICKUP_SELECTION] == PICKUP_SEL_ALWAYS )
    {
        qs_bForceFindFirst = TRUE;
        bProcessed = qsPickUpSelection(g_QSearchDlg.hFindEdit, g_Options.dwFlags, FALSE);
    }

    setInfoEmpty();

    if ( !bProcessed )
    {
        qs_bForceFindFirst = TRUE;
        qs_bEditTextChanged = TRUE;
        qsearchDoSetNotFound(g_QSearchDlg.hFindEdit, FALSE, FALSE, FALSE);

        if ( bHighlightAllChecked )
        {
            DWORD dwOptFlagsTemp[OPTF_COUNT];

            getEditFindText(g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextW);

            copyOptionsFlags(dwOptFlagsTemp, g_Options.dwFlags);
            dwOptFlagsTemp[OPTF_SRCH_ONTHEFLY_MODE] = 1;
            dwOptFlagsTemp[OPTF_SRCH_STOP_EOF] = 0;
            qsearchDoSearchText( g_QSearchDlg.hFindEdit, QSEARCH_FIRST, dwOptFlagsTemp, NULL );

            qsearchDoTryHighlightAll(hDlg, g_Options.dwFlags);
        }
    }
    else
    {
        qs_bEditTextChanged = TRUE;
        qsearchDoSetNotFound(g_QSearchDlg.hFindEdit, FALSE, FALSE, FALSE);
    }
}

INT_PTR CALLBACK qsearchDlgProc(HWND hDlg,
  UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND   hToolTip = NULL;
    static HMENU  hPopupMenuLoaded = NULL;
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
                        OnEditKeyDown_Enter_or_F3(g_QSearchDlg.hFindEdit, VK_RETURN, g_Options.dwFlags);
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
                    getEditFindText( g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextW );
                    qsearchFindHistoryAdd( g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextW, 0 );
                    qsPickUpSelection( g_QSearchDlg.hFindEdit, g_Options.dwFlags, FALSE );
                }*/
                getEditFindText( g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextW );
                qsearchDoSearchText( g_QSearchDlg.hFindEdit, uSearch, g_Options.dwFlags, NULL );
                if ( uSearch & QSEARCH_FIRST )
                {
                    qsearchDoTryHighlightAll( hDlg, g_Options.dwFlags );
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

                //qsearchDoQuit( g_QSearchDlg.hFindEdit, hToolTip, hPopupMenuLoaded, hTextNotFoundBrush, hTextNotRegExpBrush, hTextEOFBrush );
                qsearchDoShowHide(hDlg, FALSE, 0, g_Options.dwFlags);
                return 1;
            }
            else if ( id == IDC_BT_CANCEL )
            {
                //qsearchDoQuit( g_QSearchDlg.hFindEdit, hToolTip, hPopupMenuLoaded, hTextNotFoundBrush, hTextNotRegExpBrush, hTextEOFBrush );
                qsearchDoShowHide(hDlg, FALSE, 0, g_Options.dwFlags);
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
            else if ( id == IDC_BT_FINDALL )
            {
                PostMessage( hDlg, QSM_FINDALL, g_Options.dwFindAllMode & QS_FINDALL_MASK, 0 );
                return 1;
            }
            else if ( id == IDC_CH_MATCHCASE )
            {
                if ( HIWORD(wParam) == BN_CLICKED )
                {
                    OnChMatchCaseOrWholeWordClicked(hDlg);
                }
            }
            else if ( id == IDC_CH_WHOLEWORD )
            {
                if ( GetKeyState(VK_QS_WW_SRCH_MODE) & 0x80 )
                {
                }
                else if ( HIWORD(wParam) == BN_CLICKED )
                {
                    OnChMatchCaseOrWholeWordClicked(hDlg);
                }
            }
            else if ( id == IDC_CH_HIGHLIGHTALL )
            {
                if ( HIWORD(wParam) == BN_CLICKED )
                {
                    HWND hChHighlightAll;
                    BOOL bHighlightAllChecked = FALSE;
                    BOOL bProcessed = FALSE;

                    hChHighlightAll = GetDlgItem(hDlg, IDC_CH_HIGHLIGHTALL);
                    if ( SendMessage(hChHighlightAll, BM_GETCHECK, 0, 0) == BST_CHECKED )
                        bHighlightAllChecked = TRUE;

                    if ( bHighlightAllChecked &&
                         g_QSearchDlg.uSearchOrigin == QS_SO_EDITOR &&
                         g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE] &&
                         g_Options.dwFlags[OPTF_SRCH_PICKUP_SELECTION] == PICKUP_SEL_ALWAYS )
                    {
                        g_Options.dwHighlightState |= HLS_IS_CHECKED;
                        bProcessed = qsPickUpSelection(g_QSearchDlg.hFindEdit, g_Options.dwFlags, TRUE);
                    }

                    setInfoEmpty();

                    if ( !bProcessed )
                    {
                        if ( bHighlightAllChecked )
                        {
                            g_Options.dwHighlightState |= HLS_IS_CHECKED;
                            qsUpdateHighlight(hDlg, g_QSearchDlg.hFindEdit, g_Options.dwFlags);
                        }
                        else
                        {
                            if ( (g_Options.dwHighlightState & HLS_IS_CHECKED) == HLS_IS_CHECKED )
                                g_Options.dwHighlightState -= HLS_IS_CHECKED;
                            qsearchDoTryUnhighlightAll();
                        }
                    }

                    if ( bHighlightAllChecked &&
                         (g_Options.dwNewUI == QS_UI_NEW_02) &&
                         (g_Options.dwFindAllMode & QS_FINDALL_AUTO_COUNT_FLAG) )
                    {
                        PostMessage( g_QSearchDlg.hDlg, QSM_FINDALL, QS_FINDALL_COUNTONLY, 0 );
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
                        g_QSearchDlg.uSearchOrigin = qs_Get_SO_QSEARCH(g_Options.dwFlags);
                        qsearchDoSetNotFound(g_QSearchDlg.hFindEdit, FALSE, FALSE, FALSE);
                        setInfoEmpty();
                        break;
                    case CBN_DROPDOWN:
                        getEditFindText(g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextW);
                        qsearchFindHistoryAdd(g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextW, 0);
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
                if ( g_QSearchDlg.hPopupMenu )
                {
                    unsigned int state = GetMenuState(g_QSearchDlg.hPopupMenu, id, MF_BYCOMMAND);
                    switch ( id - IDM_START )
                    {
                        case OPTF_SRCH_PICKUP_SELECTION:
                        case OPTF_SRCH_STOP_EOF:
                        case OPTF_CATCH_MAIN_ESC:
                        case OPTF_EDITOR_AUTOFOCUS_MOUSE:
                            if ( (state & MF_CHECKED) == MF_CHECKED )
                            {
                                if ( g_Options.dwFlags[id - IDM_START] & 0x01 )
                                {
                                    g_Options.dwFlags[id - IDM_START] -= 0x01;

                                    if ( id == IDM_START + OPTF_SRCH_STOP_EOF )
                                        qsearchDoSetNotFound(g_QSearchDlg.hFindEdit, FALSE, FALSE, FALSE);
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
                        qsdlgShowHideWholeWordCheckBox(hDlg, g_Options.dwFlags);
                        qs_nEditEOF = 0;
                        qsearchDoSetNotFound( g_QSearchDlg.hFindEdit, FALSE, FALSE, FALSE );
                    }
                    else if ( id == IDM_SRCHUSESPECIALCHARS )
                    {
                        if ( g_Options.dwFlags[OPTF_SRCH_USE_SPECIALCHARS] )
                        {
                            CheckMenuItem( g_QSearchDlg.hPopupMenu, IDM_SRCHUSEREGEXP, 
                              MF_BYCOMMAND | MF_UNCHECKED );
                            EnableMenuItem( g_QSearchDlg.hPopupMenu, IDM_SRCHREGEXPDOTNEWLINE,
                              MF_BYCOMMAND | MF_GRAYED );
                            g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] = 0;
                        }
                        qsdlgShowHideWholeWordCheckBox(hDlg, g_Options.dwFlags);
                        qs_nEditEOF = 0;
                        qsearchDoSetNotFound( g_QSearchDlg.hFindEdit, FALSE, FALSE, FALSE );
                    }
                    else if ( id == IDM_SRCHUSEREGEXP )
                    {
                        if ( g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] )
                        {
                            CheckMenuItem( g_QSearchDlg.hPopupMenu, IDM_SRCHUSESPECIALCHARS, 
                              MF_BYCOMMAND | MF_UNCHECKED );
                            EnableMenuItem( g_QSearchDlg.hPopupMenu, IDM_SRCHREGEXPDOTNEWLINE,
                              MF_BYCOMMAND | MF_ENABLED );
                            g_Options.dwFlags[OPTF_SRCH_USE_SPECIALCHARS] = 0;
                        }
                        qsdlgShowHideWholeWordCheckBox(hDlg, g_Options.dwFlags);
                        qs_nEditEOF = 0;
                        qsearchDoSetNotFound( g_QSearchDlg.hFindEdit, FALSE, FALSE, FALSE );
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
            else if ( id == IDM_FINDALL_START )
            {
                if ( g_QSearchDlg.hFindAllPopupMenu )
                {
                    unsigned int state = GetMenuState(g_QSearchDlg.hFindAllPopupMenu, id, MF_BYCOMMAND);
                    if ( (state & MF_CHECKED) == MF_CHECKED )
                    {
                        if ( g_Options.dwFindAllMode & QS_FINDALL_AUTO_COUNT_FLAG )
                            g_Options.dwFindAllMode -= QS_FINDALL_AUTO_COUNT_FLAG;
                    }
                    else
                        g_Options.dwFindAllMode |= QS_FINDALL_AUTO_COUNT_FLAG;
                }
            }
            else if ( (id > IDM_FINDALL_START) && (id < IDM_FINDALL_START + QS_FINDALL_TOTAL_TYPES) )
            {
                DWORD dwFlags = 0;
                if ( g_Options.dwFindAllMode & QS_FINDALL_AUTO_COUNT_FLAG )
                    dwFlags |= QS_FINDALL_AUTO_COUNT_FLAG;
                g_Options.dwFindAllMode = id - IDM_FINDALL_START;
                g_Options.dwFindAllMode |= dwFlags;
            }
            else if ( id == IDM_FINDALL_SHOWLINE )
            {
                g_Options.dwFindAllResult |= QS_FINDALL_RSLT_WHOLELINE;
                if ( g_Options.dwFindAllResult & QS_FINDALL_RSLT_MATCHONLY )
                    g_Options.dwFindAllResult -= QS_FINDALL_RSLT_MATCHONLY;
            }
            else if ( id == IDM_FINDALL_SHOWMATCHONLY )
            {
                g_Options.dwFindAllResult |= QS_FINDALL_RSLT_MATCHONLY;
                if ( g_Options.dwFindAllResult & QS_FINDALL_RSLT_WHOLELINE )
                    g_Options.dwFindAllResult -= QS_FINDALL_RSLT_WHOLELINE;
            }
            else if ( id == IDM_FINDALL_FILTERMODE )
            {
                if ( g_Options.dwFindAllResult & QS_FINDALL_RSLT_FILTERMODE )
                    g_Options.dwFindAllResult -= QS_FINDALL_RSLT_FILTERMODE;
                else
                    g_Options.dwFindAllResult |= QS_FINDALL_RSLT_FILTERMODE;
            }
            else if ( id == IDM_FINDALL_SETTINGSDLG )
            {
                if ( g_Plugin.bOldWindows )
                {
                    /*
                    DialogBoxA(g_Plugin.hInstanceDLL,
                               MAKEINTRESOURCEA(IDD_FINDALL_SETTINGS),
                               g_Plugin.hMainWnd,
                               QSFndAllSettDlgProc);
                    */
                }
                else
                {
                    DialogBoxW(g_Plugin.hInstanceDLL,
                               MAKEINTRESOURCEW(IDD_FINDALL_SETTINGS),
                               g_Plugin.hMainWnd,
                               QSFndAllSettDlgProc);
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
        //            if ( g_QSearchDlg.hFindEdit )
        //            {
        //                SetFocus(g_QSearchDlg.hFindEdit);
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
            //qsearchDlgOnAltHotkey(hDlg, wParam);
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

            if ( !g_QSearchDlg.hFindEdit )
            {
                g_QSearchDlg.hFindEdit = qsearchGetFindEdit(hDlg);
            }
            if ( g_Plugin.nDockedDlgIsWaitingForOnStart == 0 )
            {
                qs_bEditIsActive = TRUE;
                SetFocus(g_QSearchDlg.hFindEdit);
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
            if ( g_QSearchDlg.hFindEdit == (HWND) lParam )
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
                        if ( getAkelPadSelectedText(g_QSearchDlg.szFindTextW, g_Options.dwFlags) )
                        {
                            if ( g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE] )
                            {
                                qs_bForceFindFirst = FALSE;
                                qsearchDoTryHighlightAll(hDlg, g_Options.dwFlags);
                            }
                            else
                                qs_bForceFindFirst = TRUE;
                        }
                    }
                }

                if ( g_Plugin.nDockedDlgIsWaitingForOnStart == 0 )
                {
                    qs_bEditIsActive = TRUE;
                    SetFocus(g_QSearchDlg.hFindEdit);
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
            HMENU hPopMnu;
            UINT  i;
            UINT  uCheck;
            POINT pt;

            // the cursor position can be negative (!) in case of 2nd monitor
            // pt.x = (int) (short) LOWORD(lParam);
            // pt.y = (int) (short) HIWORD(lParam);
            GetCursorPos(&pt);

            if ( (g_QSearchDlg.hBtnFindAll == (HWND) wParam) && !g_Plugin.bOldWindows )
            {
                uCheck = (g_Options.dwFindAllMode & QS_FINDALL_AUTO_COUNT_FLAG) ? MF_CHECKED : MF_UNCHECKED;
                CheckMenuItem( g_QSearchDlg.hFindAllPopupMenu, IDM_FINDALL_START, MF_BYCOMMAND | uCheck );

                for ( i = QS_FINDALL_COUNTONLY; i < QS_FINDALL_TOTAL_TYPES; i++ )
                {
                    uCheck = (i == (g_Options.dwFindAllMode & QS_FINDALL_MASK)) ? MF_CHECKED : MF_UNCHECKED;
                    CheckMenuItem( g_QSearchDlg.hFindAllPopupMenu, IDM_FINDALL_START + i, MF_BYCOMMAND | uCheck );
                }

                if ( g_Options.dwFindAllResult & QS_FINDALL_RSLT_MATCHONLY )
                {
                    CheckMenuItem( g_QSearchDlg.hFindAllPopupMenu, IDM_FINDALL_SHOWLINE, MF_BYCOMMAND | MF_UNCHECKED );
                    CheckMenuItem( g_QSearchDlg.hFindAllPopupMenu, IDM_FINDALL_SHOWMATCHONLY, MF_BYCOMMAND | MF_CHECKED );
                }
                else
                {
                    CheckMenuItem( g_QSearchDlg.hFindAllPopupMenu, IDM_FINDALL_SHOWMATCHONLY, MF_BYCOMMAND | MF_UNCHECKED );
                    CheckMenuItem( g_QSearchDlg.hFindAllPopupMenu, IDM_FINDALL_SHOWLINE, MF_BYCOMMAND | MF_CHECKED );
                }

                uCheck = (g_Options.dwFindAllResult & QS_FINDALL_RSLT_FILTERMODE) ? MF_CHECKED : MF_UNCHECKED;
                CheckMenuItem( g_QSearchDlg.hFindAllPopupMenu, IDM_FINDALL_FILTERMODE, MF_BYCOMMAND | uCheck );

                hPopMnu = g_QSearchDlg.hFindAllPopupMenu;
            }
            else
            {
                for ( i = 0; i < OPTF_COUNT - 1; i++ )
                {
                    switch ( i )
                    {
                        case OPTF_SRCH_PICKUP_SELECTION:
                        case OPTF_SRCH_STOP_EOF:
                        case OPTF_CATCH_MAIN_ESC:
                        case OPTF_EDITOR_AUTOFOCUS_MOUSE:
                            uCheck = ((g_Options.dwFlags[i] & 0x01) ? MF_CHECKED : MF_UNCHECKED);
                            break;

                        default:
                            uCheck = (g_Options.dwFlags[i] ? MF_CHECKED : MF_UNCHECKED);
                            break;
                    }
                    CheckMenuItem( g_QSearchDlg.hPopupMenu, IDM_START + i, MF_BYCOMMAND | uCheck );
                }

                if ( g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] )
                    uCheck = MF_BYCOMMAND | MF_ENABLED;
                else
                    uCheck = MF_BYCOMMAND | MF_GRAYED;
                EnableMenuItem( g_QSearchDlg.hPopupMenu, IDM_SRCHREGEXPDOTNEWLINE, uCheck );

                hPopMnu = g_QSearchDlg.hPopupMenu;
            }

            TrackPopupMenuEx(hPopMnu, 0, pt.x, pt.y, hDlg, NULL);

            break;
        }
        case QSM_FINDNEXT:
        case QSM_FINDALL:
        {
            DWORD dwSearch;
            DWORD dwFindAllMode;
            tQSFindAll qsfa;
            tQSFindAll* pqsfa;

            dwSearch = 0;

            if ( uMsg == QSM_FINDALL )
            {
                dwFindAllMode = (DWORD) wParam;

                x_zero_mem(&qsfa, sizeof(tQSFindAll));
                if ( (dwFindAllMode == QS_FINDALL_LOGOUTPUT) ||
                     (g_Options.dwNewUI != QS_UI_NEW_02) )
                {
                    // LogOutput
                    dwSearch |= QSEARCH_FINDALL;
                    qsfa.pfnFindResultCallback = qsFindResultCallback;
                    qsfa.GetFindResultPolicy.nMode = QSFRM_CHARINLINE;
                    qsfa.GetFindResultPolicy.nBefore = 80;
                    qsfa.GetFindResultPolicy.nAfter = 80;
                    qsfa.GetFindResultPolicy.nMaxLineLen = 0;
                    qsfa.GetFindResultPolicy.pfnStoreResultCallback = qsStoreResultCallback;
                    qsfa.ShowFindResults.pfnInit = qsShowFindResults_LogOutput_Init;
                    qsfa.ShowFindResults.pfnAddOccurrence = qsShowFindResults_LogOutput_AddOccurrence;
                    qsfa.ShowFindResults.pfnDone = qsShowFindResults_LogOutput_Done;
                }
                else if ( dwFindAllMode == QS_FINDALL_FILEOUTPUT_MULT ||
                          dwFindAllMode == QS_FINDALL_FILEOUTPUT_SNGL )
                {
                    // FileOutput
                    dwSearch |= QSEARCH_FINDALL;
                    qsfa.pfnFindResultCallback = qsFindResultCallback;
                    qsfa.GetFindResultPolicy.nMode = QSFRM_LINE;
                    qsfa.GetFindResultPolicy.nBefore = 0;
                    qsfa.GetFindResultPolicy.nAfter = 0;
                    qsfa.GetFindResultPolicy.nMaxLineLen = 0;
                    qsfa.GetFindResultPolicy.pfnStoreResultCallback = qsStoreResultCallback;
                    qsfa.ShowFindResults.pfnInit = qsShowFindResults_FileOutput_Init;
                    qsfa.ShowFindResults.pfnAddOccurrence = qsShowFindResults_FileOutput_AddOccurrence;
                    qsfa.ShowFindResults.pfnDone = qsShowFindResults_FileOutput_Done;
                }
                else
                {
                    // CountOnly
                    qsfa.pfnFindResultCallback = NULL;
                    qsfa.ShowFindResults.pfnInit = qsShowFindResults_CountOnly_Init;
                    qsfa.ShowFindResults.pfnAddOccurrence = qsShowFindResults_CountOnly_AddOccurrence;
                    qsfa.ShowFindResults.pfnDone = qsShowFindResults_CountOnly_Done;
                }
                pqsfa = &qsfa;
            }
            else
                pqsfa = NULL;

            if ( (g_Options.dwFlags[OPTF_SRCH_PICKUP_SELECTION] & 0x01) && 
                 (g_QSearchDlg.uSearchOrigin == QS_SO_EDITOR) && 
                 (!(lParam & QS_FF_NOPICKUPSEL)) )
            {
                getEditFindText( g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextW );
                qsearchFindHistoryAdd( g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextW, 0 );
                qsPickUpSelection( g_QSearchDlg.hFindEdit, g_Options.dwFlags, FALSE );
            }
            getEditFindText( g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextW );
            if ( qs_bForceFindFirst || qs_bEditTextChanged )
            {
                qs_bForceFindFirst = FALSE;
                qs_bEditTextChanged = FALSE;
                dwSearch |= (QSEARCH_FIRST | QSEARCH_NOFINDBEGIN);
                if ( wParam )
                    dwSearch |= QSEARCH_FINDUP;
                qsearchDoSearchText( g_QSearchDlg.hFindEdit, dwSearch, g_Options.dwFlags, pqsfa );
                qsearchDoTryHighlightAll( hDlg, g_Options.dwFlags );
            }
            else
            {
                dwSearch |= QSEARCH_NEXT;
                if ( wParam )
                    dwSearch |= QSEARCH_FINDUP;
                qsearchDoSearchText( g_QSearchDlg.hFindEdit, dwSearch, g_Options.dwFlags, pqsfa );
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

            if ( wParam & QSEARCH_USEDELAY )
                dwSearch |= QSEARCH_USEDELAY;

            getEditFindText( g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextW );
            qsearchDoSearchText( g_QSearchDlg.hFindEdit, dwSearch, g_Options.dwFlags, NULL );
            qsearchDoTryHighlightAll( hDlg, g_Options.dwFlags );
            return 1;
        }
        case QSM_SELFIND:
        {
            qsearchDoSelFind( g_QSearchDlg.hFindEdit, (BOOL) wParam, g_Options.dwFlags );
            return 1;
        }
        case QSM_PICKUPSELTEXT:
        {
            BOOL bPickedUp = qsPickUpSelection(g_QSearchDlg.hFindEdit, g_Options.dwFlags, FALSE);
            if ( lParam )
                *((BOOL *)lParam) = bPickedUp;
            return 1;
        }
        case QSM_GETHWNDEDIT:
        {
            if ( lParam )
                *((HWND *)lParam) = g_QSearchDlg.hFindEdit;
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
            qsearchDoShowHide( hDlg, (BOOL) wParam, (UINT) lParam, g_Options.dwFlags );
            return 1;
        }
        case QSM_SETNOTFOUND:
        {
            UINT uFlags;

            qsearchDoSetNotFound( g_QSearchDlg.hFindEdit, (BOOL) wParam, FALSE, FALSE );

            uFlags = (UINT) lParam;
            if ( uFlags & QS_SNF_SETINFOEMPTY )
            {
                setInfoEmpty();
            }
            if ( uFlags & QS_SNF_FORCEFINDFIRST )
            {
                qs_bForceFindFirst = TRUE;
                qs_bEditTextChanged = TRUE;
            }

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
                qsdlgShowHideWholeWordCheckBox(hDlg, g_Options.dwFlags);
            }

            if ( wParam & QS_UU_FIND )
            {
                // set edit or combo-box find control
                BOOL isFindEditFocused = FALSE;

                if ( hFocusedWnd )
                {
                    if ( hFocusedWnd == g_QSearchDlg.hFindEdit )
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
                g_QSearchDlg.hFindEdit = qsearchGetFindEdit(hDlg);
                // set focus if needed
                if ( isFindEditFocused )
                    SetFocus(g_QSearchDlg.hFindEdit);
            }

            return 1;
        }
        case QSM_QUIT:
        {
            qsearchDoQuit( g_QSearchDlg.hFindEdit, hToolTip, hPopupMenuLoaded, hTextNotFoundBrush, hTextNotRegExpBrush, hTextEOFBrush );
            return 1;
        }
        case QSN_DLGSWITCH:
        {
            g_QSearchDlg.uSearchOrigin = QS_SO_QSEARCH;
            qs_bEditTextChanged = TRUE;
            qs_bForceFindFirst = TRUE;
            //getEditFindText( g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextW );
            //if ( g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE] )
            //{
            //    qsearchDoTryHighlightAll( hDlg, g_Options.dwFlags );
            //}
            return 1;
        }
        case WM_INITDIALOG:
        {
            HWND hDlgItm;

            InitializeCriticalSection(&csFindAllTimerId);

            g_QSearchDlg.bIsQSearchingRightNow = FALSE;
            g_QSearchDlg.bMouseJustLeavedFindEdit = FALSE;
            g_QSearchDlg.hDlg = hDlg;
            g_QSearchDlg.hFindEdit = qsearchGetFindEdit(hDlg);

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

            SendMessage(g_QSearchDlg.hFindEdit, EM_SETLIMITTEXT, MAX_TEXT_SIZE - 1, 0);
            prev_editWndProc = setWndProc(g_QSearchDlg.hFindEdit, editWndProc);
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
            hDlgItm = GetDlgItem(hDlg, IDC_CH_WHOLEWORD);
            if ( hDlgItm )
            {
                prev_chWholeWordWndProc = setWndProc(hDlgItm, chWholeWordWndProc);
            }

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
            g_QSearchDlg.hBtnFindAll = GetDlgItem(hDlg, IDC_BT_FINDALL);
            if ( g_QSearchDlg.hBtnFindAll )
            {
                prev_btnFindAllWndProc = setWndProc(g_QSearchDlg.hBtnFindAll, btnFindWndProc);
            }
            g_QSearchDlg.hStInfo = GetDlgItem(hDlg, IDC_ST_INFO);

            hToolTip = qsearchDoInitToolTip(hDlg, g_QSearchDlg.hFindEdit);
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
            g_QSearchDlg.hPopupMenu = GetSubMenu(hPopupMenuLoaded, 0);
            g_QSearchDlg.hFindAllPopupMenu = GetSubMenu(hPopupMenuLoaded, 1);
            qsearchSetPopupMenuLang(g_QSearchDlg.hPopupMenu);
            qsearchSetFindAllPopupMenuLang(g_QSearchDlg.hFindAllPopupMenu);
            qsearchSetMainDlgLang(hDlg);

            qsdlgShowHideWholeWordCheckBox(hDlg, g_Options.dwFlags);

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
                getAkelPadSelectedText(g_QSearchDlg.szFindTextW, g_Options.dwFlags);
            }
            setEditFindText(g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextW);
            //SetFocus(g_QSearchDlg.hFindEdit);
            if ( !g_Options.dwFlags[OPTF_EDIT_FOCUS_SELECTALL] )
            {
                SendMessage(g_QSearchDlg.hFindEdit, EM_SETSEL, 0, -1);
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
            char szHintA[128];

            fillToolInfoA( &tiA, LPSTR_TEXTCALLBACKA, hEdit );
            // LPSTR_TEXTCALLBACKA means "send TTN_GETDISPINFOA to hEdit"
            SendMessage( hToolTip, TTM_ADDTOOLA, 0, (LPARAM) &tiA );

            lstrcpyA( szHintA, qsearchGetHintA(IDC_BT_CANCEL) );
            fillToolInfoA( &tiA, szHintA, GetDlgItem(hDlg, IDC_BT_CANCEL) );
            SendMessage( hToolTip, TTM_ADDTOOLA, 0, (LPARAM) &tiA );

            lstrcpyA( szHintA, qsearchGetHintA(IDC_CH_MATCHCASE) );
            if ( g_Options.dwUseAltHotkeys )
            {
                strAppendAltHotkeyA( szHintA, g_Options.dwAltMatchCase );
            }
            fillToolInfoA( &tiA, szHintA, GetDlgItem(hDlg, IDC_CH_MATCHCASE) );
            SendMessage( hToolTip, TTM_ADDTOOLA, 0, (LPARAM) &tiA );

            fillToolInfoA( &tiA, LPSTR_TEXTCALLBACKA, GetDlgItem(hDlg, IDC_CH_WHOLEWORD) );
            // LPSTR_TEXTCALLBACKA means "send TTN_GETDISPINFOA to hCheckbox"
            SendMessage( hToolTip, TTM_ADDTOOLA, 0, (LPARAM) &tiA );

            lstrcpyA( szHintA, qsearchGetHintA(IDC_CH_HIGHLIGHTALL) );
            if ( g_Options.dwUseAltHotkeys )
            {
                strAppendAltHotkeyA( szHintA, g_Options.dwAltHighlightAll );
            }
            fillToolInfoA( &tiA, szHintA, GetDlgItem(hDlg, IDC_CH_HIGHLIGHTALL) );
            SendMessage( hToolTip, TTM_ADDTOOLA, 0, (LPARAM) &tiA );

            if ( g_Options.dwNewUI == QS_UI_NEW_01 || 
                 g_Options.dwNewUI == QS_UI_NEW_02 )
            {
                fillToolInfoA( &tiA, (LPSTR) qsearchGetHintA(IDC_BT_FINDNEXT), 
                  GetDlgItem(hDlg, IDC_BT_FINDNEXT) );
                SendMessage( hToolTip, TTM_ADDTOOLA, 0, (LPARAM) &tiA );

                fillToolInfoA( &tiA, (LPSTR) qsearchGetHintA(IDC_BT_FINDPREV), 
                  GetDlgItem(hDlg, IDC_BT_FINDPREV) );
                SendMessage( hToolTip, TTM_ADDTOOLA, 0, (LPARAM) &tiA );

                if ( g_Options.dwNewUI == QS_UI_NEW_02 )
                {
                    fillToolInfoA( &tiA, (LPSTR) qsearchGetHintA(IDC_BT_FINDALL), 
                      GetDlgItem(hDlg, IDC_BT_FINDALL) );
                    SendMessage( hToolTip, TTM_ADDTOOLA, 0, (LPARAM) &tiA );
                }
            }
        }
        else
        {
            TOOLINFOW tiW;
            wchar_t szHintW[128];

            fillToolInfoW( &tiW, LPSTR_TEXTCALLBACKW, hEdit );
            // LPSTR_TEXTCALLBACKW means "send TTN_GETDISPINFOW to hEdit"
            SendMessageW( hToolTip, TTM_ADDTOOLW, 0, (LPARAM) &tiW );

            lstrcpyW( szHintW, qsearchGetHintW(IDC_BT_CANCEL) );
            fillToolInfoW( &tiW, szHintW, GetDlgItem(hDlg, IDC_BT_CANCEL) );
            SendMessageW( hToolTip, TTM_ADDTOOLW, 0, (LPARAM) &tiW );

            lstrcpyW( szHintW, qsearchGetHintW(IDC_CH_MATCHCASE) );
            if ( g_Options.dwUseAltHotkeys )
            {
                strAppendAltHotkeyW( szHintW, g_Options.dwAltMatchCase );
            }
            fillToolInfoW( &tiW, szHintW, GetDlgItem(hDlg, IDC_CH_MATCHCASE) );
            SendMessageW( hToolTip, TTM_ADDTOOLW, 0, (LPARAM) &tiW );

            fillToolInfoW( &tiW, LPSTR_TEXTCALLBACKW, GetDlgItem(hDlg, IDC_CH_WHOLEWORD) );
            // LPSTR_TEXTCALLBACKW means "send TTN_GETDISPINFOW to hCheckbox"
            SendMessageW( hToolTip, TTM_ADDTOOLW, 0, (LPARAM) &tiW );

            lstrcpyW( szHintW, qsearchGetHintW(IDC_CH_HIGHLIGHTALL) );
            if ( g_Options.dwUseAltHotkeys )
            {
                strAppendAltHotkeyW( szHintW, g_Options.dwAltHighlightAll );
            }
            fillToolInfoW( &tiW, szHintW, GetDlgItem(hDlg, IDC_CH_HIGHLIGHTALL) );
            SendMessageW( hToolTip, TTM_ADDTOOLW, 0, (LPARAM) &tiW );

            if ( g_Options.dwNewUI == QS_UI_NEW_01 ||
                 g_Options.dwNewUI == QS_UI_NEW_02 )
            {
                fillToolInfoW( &tiW, (LPWSTR) qsearchGetHintW(IDC_BT_FINDNEXT), 
                  GetDlgItem(hDlg, IDC_BT_FINDNEXT) );
                SendMessageW( hToolTip, TTM_ADDTOOLW, 0, (LPARAM) &tiW );

                fillToolInfoW( &tiW, (LPWSTR) qsearchGetHintW(IDC_BT_FINDPREV), 
                  GetDlgItem(hDlg, IDC_BT_FINDPREV) );
                SendMessageW( hToolTip, TTM_ADDTOOLW, 0, (LPARAM) &tiW );

                if ( g_Options.dwNewUI == QS_UI_NEW_02 )
                {
                    fillToolInfoW( &tiW, (LPWSTR) qsearchGetHintW(IDC_BT_FINDALL), 
                      GetDlgItem(hDlg, IDC_BT_FINDALL) );
                    SendMessageW( hToolTip, TTM_ADDTOOLW, 0, (LPARAM) &tiW );
                }
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
    if ( prev_chWholeWordWndProc )
    {
        hDlgItm = GetDlgItem(g_QSearchDlg.hDlg, IDC_CH_WHOLEWORD);
        setWndProc(hDlgItm, prev_chWholeWordWndProc);
        prev_chWholeWordWndProc = NULL;
    }
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
    if ( prev_btnFindAllWndProc )
    {
        setWndProc(g_QSearchDlg.hBtnFindAll, prev_btnFindAllWndProc);
        prev_btnFindAllWndProc = NULL;
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

    DeleteCriticalSection(&csFindAllTimerId);
}

void qsearchDoSetNotFound(HWND hEdit, BOOL bNotFound, BOOL bNotRegExp, BOOL bEOF)
{
    if ( bNotFound )
    {
        qsearchDoTryUnhighlightAll();
        setInfoEmpty();
    }

    qs_bEditNotFound = bNotFound;
    qs_bEditNotRegExp = bNotRegExp;
    qs_bEditIsEOF = bEOF;
    InvalidateRect(hEdit, NULL, TRUE);
    UpdateWindow(hEdit);
}

void qsearchDoShowHide(HWND hDlg, BOOL bShow, UINT uShowFlags, const DWORD dwOptFlags[])
{
    BOOL bChangeSelection = !IsWindowVisible(hDlg);

    qsearchDoSetNotFound( qsearchGetFindEdit(hDlg), FALSE, FALSE, FALSE );
    setInfoEmpty();

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
            if ( dwOptFlags[OPTF_SRCH_PICKUP_SELECTION] & 0x01 )
            {
                bGotSelectedText = getAkelPadSelectedText(g_QSearchDlg.szFindTextW, dwOptFlags);
                if ( bGotSelectedText )
                {
                    bChangeSelection = TRUE;
                }
                else
                {
                    getEditFindText(hEdit, g_QSearchDlg.szFindTextW);
                }
                setEditFindText(hEdit, g_QSearchDlg.szFindTextW);
                if ( (!bChangeSelection) || dwOptFlags[OPTF_EDIT_FOCUS_SELECTALL] )
                {
                    SendMessage(hEdit, EM_SETSEL, 0, -1);
    #ifdef _DEBUG
                    Debug_Output("qsearchDoShowHide, PickUpSel, SETSEL(0, -1)\n");
    #endif
                    qs_bEditSelJustChanged = TRUE;
                }
            }
        }
        if ( bChangeSelection && !dwOptFlags[OPTF_EDIT_FOCUS_SELECTALL] )
        {
            SendMessage(hEdit, EM_SETSEL, 0, -1);
#ifdef _DEBUG
            Debug_Output("qsearchDoShowHide, ChangeSel, SETSEL(0, -1)\n");
#endif
            qs_bEditSelJustChanged = TRUE;
        }
        if ( bGotSelectedText )
        {
            if ( dwOptFlags[OPTF_SRCH_ONTHEFLY_MODE] )
            {
                qs_bForceFindFirst = FALSE;
                qsearchDoTryHighlightAll(g_QSearchDlg.hDlg, dwOptFlags);
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

void qsearchDoSelFind(HWND hEdit, BOOL bFindPrev, const DWORD dwOptFlags[])
{
    wchar_t prevFindTextW[MAX_TEXT_SIZE];
    DWORD   dwOptFlagsTemp[OPTF_COUNT];

    if ( g_Plugin.bOldWindows )
    {
        lstrcpyA( (LPSTR) prevFindTextW, (LPCSTR) g_QSearchDlg.szFindTextW );
    }
    else
    {
        lstrcpyW( (LPWSTR) prevFindTextW, (LPCWSTR) g_QSearchDlg.szFindTextW );
    }

    if ( dwOptFlags[OPTF_SRCH_SELFIND_PICKUP] )
    {
        // getting selected text with current search flags
        if ( getAkelPadSelectedText(g_QSearchDlg.szFindTextW, dwOptFlags) )
        {
            setEditFindText(hEdit, g_QSearchDlg.szFindTextW);
        }
    }

    copyOptionsFlags(dwOptFlagsTemp, dwOptFlags);
    // these search flags must be disabled here
    dwOptFlagsTemp[OPTF_SRCH_FROM_BEGINNING] = 0;
    dwOptFlagsTemp[OPTF_SRCH_USE_SPECIALCHARS] = 0;
    dwOptFlagsTemp[OPTF_SRCH_USE_REGEXP] = 0;

    // getting selected text with modified search flags
    if ( getAkelPadSelectedText(g_QSearchDlg.szFindTextW, dwOptFlagsTemp) )
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
        qsearchDoSearchText(hEdit, dwSearchParams, dwOptFlagsTemp, NULL);
    }
}

static void adjustIncompleteRegExA(char* szTextA, const DWORD dwOptFlags[])
{
    int n1, n2;

    if ( g_Options.dwAdjIncomplRegExp == 0 )
        return;

    if ( g_Options.dwAdjIncomplRegExp == 2 )
    {
        if ( dwOptFlags[OPTF_SRCH_ONTHEFLY_MODE] == 0 )
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
        {
            // "++", "**", "+*" or "*+"
            // special case to preserve the first '+' or '*' from the two at the end
            szTextA[n1 + 1] = 0;
            return;
        }

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

static void adjustIncompleteRegExW(wchar_t* szTextW, const DWORD dwOptFlags[])
{
    int n1, n2;

    if ( g_Options.dwAdjIncomplRegExp == 0 )
        return;

    if ( g_Options.dwAdjIncomplRegExp == 2 )
    {
        if ( dwOptFlags[OPTF_SRCH_ONTHEFLY_MODE] == 0 )
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
        {
            // "++", "**", "+*" or "*+"
            // special case to preserve the first '+' or '*' from the two at the end
            szTextW[n1 + 1] = 0;
            return;
        }

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

static void convertFindExToRegExA(const char* cszFindExA, char* pszRegExA)
{
    char ch;
    char next_ch;

    while ( (ch = *cszFindExA) != 0 )
    {
        switch ( ch )
        {
            case '\\':
                *(pszRegExA++) = '\\';
                next_ch = *(cszFindExA + 1);
                switch ( next_ch )
                {
                    case '\\':  // "\\" -> "\\"
                    case 'n':   // "\n" -> "\n"
                    case 't':   // "\t" -> "\t"
                    case '*':   // "\*" -> "\*"
                    case '?':   // "\?" -> "\?"
                        *(pszRegExA++) = next_ch;
                        ++cszFindExA;
                        break;

                    default:     // "\" -> "\\"
                        *(pszRegExA++) = '\\';
                        break;
                }
                break;

            case '*':  // "*" -> ".*?" (any 0 or more chars, not greedy)
                *(pszRegExA++) = '.';
                *(pszRegExA++) = '*';
                *(pszRegExA++) = '?';
                break;

            case '?':  // "?" -> "." (any single char)
                *(pszRegExA++) = '.';
                break;

            case '.':
            case '(':
            case ')':
            case '[':
            case ']':
            case '{':
            case '}':
            case '^':
            case '$':
            case '+':
            case '|':
                *(pszRegExA++) = '\\';
                *(pszRegExA++) = ch;
                break;

            default:
                *(pszRegExA++) = ch;
                break;
        }

        ++cszFindExA;
    }

    *pszRegExA = 0;
}

static void convertFindExToRegExW(const wchar_t* cszFindExW, wchar_t* pszRegExW)
{
    wchar_t wch;
    wchar_t next_wch;

    while ( (wch = *cszFindExW) != 0 )
    {
        switch ( wch )
        {
            case L'\\':
                *(pszRegExW++) = L'\\';
                next_wch = *(cszFindExW + 1);
                switch ( next_wch )
                {
                    case L'\\':  // "\\" -> "\\"
                    case L'n':   // "\n" -> "\n"
                    case L't':   // "\t" -> "\t"
                    case L'*':   // "\*" -> "\*"
                    case L'?':   // "\?" -> "\?"
                        *(pszRegExW++) = next_wch;
                        ++cszFindExW;
                        break;

                    default:     // "\" -> "\\"
                        *(pszRegExW++) = L'\\';
                        break;
                }
                break;

            case L'*':  // "*" -> ".*?" (any 0 or more chars, not greedy)
                *(pszRegExW++) = L'.';
                *(pszRegExW++) = L'*';
                *(pszRegExW++) = L'?';
                break;

            case L'?':  // "?" -> "." (any single char)
                *(pszRegExW++) = L'.';
                break;

            case L'.':
            case L'(':
            case L')':
            case L'[':
            case L']':
            case L'{':
            case L'}':
            case L'^':
            case L'$':
            case L'+':
            case L'|':
                *(pszRegExW++) = L'\\';
                *(pszRegExW++) = wch;
                break;

            default:
                *(pszRegExW++) = wch;
                break;
        }

        ++cszFindExW;
    }

    *pszRegExW = 0;
}

static void CALLBACK CountAllTimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    EnterCriticalSection(&csFindAllTimerId);
    if ( idEvent == nFindAllTimerId )
    {
        KillTimer(NULL, idEvent);
        nFindAllTimerId = 0;
    }
    LeaveCriticalSection(&csFindAllTimerId);

    PostMessage( g_QSearchDlg.hDlg, QSM_FINDALL, QS_FINDALL_COUNTONLY, 0 );
}

// searches for g_QSearchDlg.szFindTextW
void qsearchDoSearchText(HWND hEdit, DWORD dwParams, const DWORD dwOptFlags[], tQSFindAll* pFindAll)
{
    EDITINFO ei;
    HWND     hDlgItm;
    BOOL     bNotFound = FALSE;
    BOOL     bNotRegExp = FALSE;
    BOOL     bEOF = FALSE;
    BOOL     bNeedsFindAllCountOnly = FALSE;
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
            if ( dwOptFlags[OPTF_SRCH_STOP_EOF] != STOP_EOF_WITHOUT_MSG )
                return;
    }

    if ( g_Plugin.bOldWindows )
    {
        if ( g_QSearchDlg.hStInfo )
        {
            if ( (g_Options.dwNewUI == QS_UI_NEW_02) &&
                 (g_Options.dwFindAllMode & QS_FINDALL_AUTO_COUNT_FLAG) &&
                 (0 != ((LPCSTR) g_QSearchDlg.szFindTextW)[0]) )
            {
                char szInfoTextA[128];
                int nLen;

                szInfoTextA[0] = 0;
                nLen = GetWindowTextA(g_QSearchDlg.hStInfo, szInfoTextA, 124);

                if ( szInfoTextA[0] == 0 )
                {
                    if ( (dwParams & QSEARCH_SEL) == 0 )
                        bNeedsFindAllCountOnly = TRUE;
                }
                else if ( dwParams & QSEARCH_FIRST )
                {
                    if ( (dwParams & QSEARCH_SEL) == 0 )
                        bNeedsFindAllCountOnly = TRUE;

                    if ( (nLen < 3) || (lstrcmpA(szInfoTextA + nLen - 3, "...") != 0) )
                    {
                        lstrcatA(szInfoTextA, "...");
                        SetWindowTextA(g_QSearchDlg.hStInfo, szInfoTextA);
                    }
                }
            }
            else
                SetWindowTextA(g_QSearchDlg.hStInfo, "");
        }

        if ( 0 == ((LPCSTR) g_QSearchDlg.szFindTextW)[0] )
        {
            qsearchDoSetNotFound(hEdit, FALSE, FALSE, FALSE);
            return;
        }
    }
    else
    {
        if ( g_QSearchDlg.hStInfo )
        {
            if ( (g_Options.dwNewUI == QS_UI_NEW_02) &&
                 (g_Options.dwFindAllMode & QS_FINDALL_AUTO_COUNT_FLAG) &&
                 (0 != ((LPCWSTR) g_QSearchDlg.szFindTextW)[0]) )
            {
                wchar_t szInfoTextW[128];
                int nLen;

                szInfoTextW[0] = 0;
                nLen = GetWindowTextW(g_QSearchDlg.hStInfo, szInfoTextW, 124);

                if ( szInfoTextW[0] == 0 )
                {
                    if ( (dwParams & QSEARCH_SEL) == 0 )
                        bNeedsFindAllCountOnly = TRUE;
                }
                else if ( dwParams & QSEARCH_FIRST )
                {
                    if ( (dwParams & QSEARCH_SEL) == 0 )
                        bNeedsFindAllCountOnly = TRUE;

                    if ( (nLen < 3) || (lstrcmpW(szInfoTextW + nLen - 3, L"...") != 0) )
                    {
                        lstrcatW(szInfoTextW, L"...");
                        SetWindowTextW(g_QSearchDlg.hStInfo, szInfoTextW);
                    }
                }
            }
            else
                SetWindowTextW(g_QSearchDlg.hStInfo, L"");
        }

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
         dwOptFlags[OPTF_SRCH_SELFIND_PICKUP] )
    {
        // not SelFindNext or SelFindPrev
        // or srch_selfind_pickup = 1
        g_QSearchDlg.bQSearching = TRUE;
    }

    if ( !pFindAll )
    {
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
    }
    else // pFindAll
    {
        qs_nEditEOF = 0;
    }

    if ( hDlgItm = GetDlgItem(g_QSearchDlg.hDlg, IDC_CH_MATCHCASE) )
    {
        if ( SendMessage(hDlgItm, BM_GETCHECK, 0, 0) == BST_CHECKED )
            dwSearchFlags |= FR_MATCHCASE;
    }

    //if ( !dwOptFlags[OPTF_SRCH_USE_SPECIALCHARS] )
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
        if ( dwOptFlags[OPTF_SRCH_ONTHEFLY_MODE] && !pFindAll )
        {
            CHARRANGE_X cr = {0, 0};

            SendMessage( ei.hWndEdit, EM_EXGETSEL_X, 0, (LPARAM) &cr );
            cr.cpMax = cr.cpMin;
            SendMessage( ei.hWndEdit, EM_EXSETSEL_X, 0, (LPARAM) &cr );
        }
        if ( dwOptFlags[OPTF_SRCH_FROM_BEGINNING] && ((dwParams & QSEARCH_NOFINDBEGIN) != QSEARCH_NOFINDBEGIN) )
            dwSearchFlags |= FR_BEGINNING;
    }

    if ( dwOptFlags[OPTF_SRCH_USE_REGEXP] )
    {
        dwSearchFlags |= FRF_REGEXP;
        if ( dwOptFlags[OPTF_SRCH_REGEXP_DOT_NEWLINE] == 0 )
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
        getTextToSearchA( (LPCSTR) g_QSearchDlg.szFindTextW, &bSearchEx, dwOptFlags, szFindTextA );
        if ( dwOptFlags[OPTF_SRCH_USE_REGEXP] )
        {
            adjustIncompleteRegExA(szFindTextA, dwOptFlags);
        }

        if ( !pFindAll )
        {
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
                else if ( dwOptFlags[OPTF_SRCH_STOP_EOF] == STOP_EOF_WITHOUT_MSG )
                {
                    bNotFound = TRUE;
                    bEOF = TRUE;
                    qs_nEditEOF = srchEOF;
                }
                else if ( (dwOptFlags[OPTF_SRCH_STOP_EOF] & 0x01) && 
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
        else // pFindAll
        {
            MessageBoxA( 
              g_Plugin.hMainWnd, 
              "Not supported in non-Unicode version",
              "AkelPad (QSearch)",
              MB_OK | MB_ICONEXCLAMATION
            );
        }
    }
    else // !g_Plugin.bOldWindows
    {
        TEXTFINDW tfW;
        int       srchEOF;
        BOOL      bSearchEx = FALSE;
        INT_X     iFindResult = -1;
        wchar_t   szFindTextW[MAX_TEXT_SIZE];

        //g_QSearchDlg.szFindTextW[0] = 0;
        //GetWindowTextW( hEdit, (LPWSTR) g_QSearchDlg.szFindTextW, MAX_TEXT_SIZE - 1 );
        getTextToSearchW( g_QSearchDlg.szFindTextW, &bSearchEx, dwOptFlags, szFindTextW );
        if ( dwOptFlags[OPTF_SRCH_USE_REGEXP] )
        {
            adjustIncompleteRegExW(szFindTextW, dwOptFlags);
        }

        if ( !pFindAll )
        {
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
                else if ( dwOptFlags[OPTF_SRCH_STOP_EOF] == STOP_EOF_WITHOUT_MSG )
                {
                    bNotFound = TRUE;
                    bEOF = TRUE;
                    qs_nEditEOF = srchEOF;
                }
                else if ( (dwOptFlags[OPTF_SRCH_STOP_EOF] & 0x01) && 
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
        else // pFindAll
        {
            DWORD                dwFindAllFlags;
            unsigned int         nMatches;
            unsigned int         nTotalMatches;  // all files
            FRAMEDATA*           pFrameInitial;  // all files
            FRAMEDATA*           pFrame;         // all files
            EDITINFO*            pEditInfo;
            tDynamicBuffer       resultsBuf;
            AEFINDTEXTW          aeftW;
            wchar_t              szFindAllW[2*MAX_TEXT_SIZE];

            tDynamicBuffer_Init( &resultsBuf );
            tDynamicBuffer_Allocate( &resultsBuf, 128*1024*sizeof(wchar_t) );

            aeftW.dwFlags = AEFR_DOWN;
            if ( dwSearchFlags & FR_MATCHCASE )
                aeftW.dwFlags |= AEFR_MATCHCASE;
            if ( dwSearchFlags & FR_WHOLEWORD )
                aeftW.dwFlags |= AEFR_WHOLEWORD;
            if ( bSearchEx )
            {
                aeftW.dwFlags |= (AEFR_REGEXP | AEFR_REGEXPNONEWLINEDOT);
            }
            else if ( dwSearchFlags & FRF_REGEXP )
            {
                aeftW.dwFlags |= AEFR_REGEXP;
                if ( dwSearchFlags & FRF_REGEXPNONEWLINEDOT )
                    aeftW.dwFlags |= AEFR_REGEXPNONEWLINEDOT;
            }

            if ( bSearchEx )
                convertFindExToRegExW(szFindTextW, szFindAllW);
            else
                lstrcpyW(szFindAllW, szFindTextW);
            aeftW.pText = szFindAllW;
            aeftW.dwTextLen = lstrlenW(szFindAllW);
            aeftW.nNewLine = AELB_ASIS;

            dwFindAllFlags = 0;
            if ( bSearchEx )
                dwFindAllFlags |= QS_FAF_SPECCHAR;
            else if ( dwSearchFlags & FRF_REGEXP )
                dwFindAllFlags |= QS_FAF_REGEXP;
            if ( dwSearchFlags & FR_MATCHCASE )
                dwFindAllFlags |= QS_FAF_MATCHCASE;
            if ( dwSearchFlags & FR_WHOLEWORD )
                dwFindAllFlags |= QS_FAF_WHOLEWORD;

            nTotalMatches = 0;

            if ( g_Options.dwFindAllResult & QS_FINDALL_RSLT_ALLFILES )
            {
                pFrame = (FRAMEDATA *) SendMessageW(g_Plugin.hMainWnd, AKD_FRAMEFIND, FWF_CURRENT, 0);
                pFrameInitial = pFrame;
            }
            else
            {
                pFrame = NULL;
                pFrameInitial = NULL;
            }

            for ( ; ; )
            {
                if ( g_Options.dwFindAllResult & QS_FINDALL_RSLT_ALLFILES )
                    pEditInfo = &pFrame->ei;
                else
                    pEditInfo = &ei;

                SendMessageW( pEditInfo->hWndEdit, AEM_GETINDEX, AEGI_FIRSTCHAR, (LPARAM) &aeftW.crSearch.ciMin );
                SendMessageW( pEditInfo->hWndEdit, AEM_GETINDEX, AEGI_LASTCHAR, (LPARAM) &aeftW.crSearch.ciMax);

                nMatches = 0;
                
                pFindAll->ShowFindResults.pfnInit(szFindTextW, &pFindAll->buf, &resultsBuf, dwFindAllFlags, pEditInfo);

                while ( SendMessageW(pEditInfo->hWndEdit, AEM_FINDTEXTW, 0, (LPARAM) &aeftW) )
                {
                    ++nMatches;
                    ++nTotalMatches;

                    if ( pFindAll->pfnFindResultCallback )
                        pFindAll->pfnFindResultCallback(pEditInfo->hWndEdit, &aeftW.crFound, &pFindAll->GetFindResultPolicy, &pFindAll->buf, &pFindAll->buf2, &resultsBuf, pFindAll->ShowFindResults.pfnAddOccurrence);

                    x_mem_cpy( &aeftW.crSearch.ciMin, &aeftW.crFound.ciMax, sizeof(AECHARINDEX) );
                }

                pFindAll->ShowFindResults.pfnDone(nMatches, &resultsBuf);

                if ( g_Options.dwFindAllResult & QS_FINDALL_RSLT_ALLFILES )
                {
                    pFrame = (FRAMEDATA *) SendMessageW(g_Plugin.hMainWnd, AKD_FRAMEFIND, FWF_NEXT, (LPARAM) pFrame);
                    if ( pFrame == pFrameInitial )
                        break;
                }
                else
                    break;
            }

            tDynamicBuffer_Free(&pFindAll->buf);
            tDynamicBuffer_Free(&pFindAll->buf2);

            tDynamicBuffer_Free(&resultsBuf);

            if ( nTotalMatches == 0 )
            {
                bNotFound = TRUE;
                if ( (dwSearchFlags & FRF_REGEXP) && (aeftW.nCompileErrorOffset > 0) )
                {
                    bNotRegExp = TRUE;
                    bEOF = TRUE;
                    qs_nEditEOF = 0;
                }
                else
                {
                    bEOF = TRUE;
                    qs_nEditEOF = QSEARCH_EOF_DOWN | QSEARCH_EOF_UP;
                }
            }
        }
    }

    if ( (!bNotFound) && (!qs_nEditEOF) )
    {
        if ( !pFindAll )
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
                 ((dwParams & QSEARCH_FIRST) && !dwOptFlags[OPTF_SRCH_ONTHEFLY_MODE]) )
            {
                UINT uUpdFlags = UFHF_MOVE_TO_TOP_IF_EXISTS;
                if ( (dwParams & QSEARCH_SEL) && !dwOptFlags[OPTF_SRCH_SELFIND_PICKUP] )
                    uUpdFlags |= UFHF_KEEP_EDIT_TEXT;
                qsearchFindHistoryAdd(hEdit, g_QSearchDlg.szFindTextW, uUpdFlags);
            }
        }
        else if ( dwParams & QSEARCH_FINDALL )
        {
            UINT uUpdFlags = UFHF_MOVE_TO_TOP_IF_EXISTS;
            if ( (dwParams & QSEARCH_SEL) && !dwOptFlags[OPTF_SRCH_SELFIND_PICKUP] )
                uUpdFlags |= UFHF_KEEP_EDIT_TEXT;
            qsearchFindHistoryAdd(hEdit, g_QSearchDlg.szFindTextW, uUpdFlags);
        }
    }

    // now the Edit control can be deactivated
    qs_bEditCanBeNonActive = TRUE;
    qsearchDoSetNotFound(hEdit, bNotFound, bNotRegExp, bEOF);

    g_QSearchDlg.bIsQSearchingRightNow = FALSE;

    if ( bNeedsFindAllCountOnly && !pFindAll )
    {
        UINT nDelayMs;
        UINT_PTR nTimerId;

        if ( (dwParams & (QSEARCH_FIRST | QSEARCH_USEDELAY)) == (QSEARCH_FIRST | QSEARCH_USEDELAY) )
            nDelayMs = g_Options.dwFindAllCountDelay;
        else
            nDelayMs = 0;

        if ( nDelayMs != 0 )
        {
            nTimerId = SetTimer(NULL, 0, nDelayMs, CountAllTimerProc);
        }
        else
        {
            nTimerId = 0;
        }

        EnterCriticalSection(&csFindAllTimerId);
        if ( nFindAllTimerId != 0 )
        {
            KillTimer(NULL, nFindAllTimerId);
        }
        nFindAllTimerId = nTimerId;
        LeaveCriticalSection(&csFindAllTimerId);

        if ( nDelayMs == 0 )
        {
            PostMessage( g_QSearchDlg.hDlg, QSM_FINDALL, QS_FINDALL_COUNTONLY, 0 );
        }
    }
}

void qsearchDoTryHighlightAll(HWND hDlg, const DWORD dwOptFlags[])
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
                    wchar_t szMarkTextBuf[MAX_TEXT_SIZE + 4]; // plus room for the leading & trailing "\\b"

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
                        if ( dwOptFlags[OPTF_SRCH_USE_REGEXP] )
                        {
                            wchar_t* pszMarkText;
                            int nLen;

                            pszMarkText = szMarkTextBuf + 2;
                            pszMarkText[0] = 0;
                            if ( g_Plugin.bOldWindows )
                            {
                                MultiByteToWideChar( CP_ACP, 0, (LPCSTR) g_QSearchDlg.szFindTextW, -1, pszMarkText, MAX_TEXT_SIZE - 1 );
                            }
                            else
                            {
                                lstrcpyW( pszMarkText, g_QSearchDlg.szFindTextW );
                            }
                            adjustIncompleteRegExW( pszMarkText, dwOptFlags );
                            if ( pszMarkText[0] != 0 )
                            {
                                hlParams.dwMarkFlags |= MARKFLAG_REGEXP;
                                if ( hlParams.dwMarkFlags & MARKFLAG_WHOLEWORD )
                                {
                                    // When MARKFLAG_REGEXP is set in dwMarkFlags,
                                    // MARKFLAG_WHOLEWORD seems to be ignored...
                                    hlParams.dwMarkFlags ^= MARKFLAG_WHOLEWORD;
                                    nLen = lstrlenW(pszMarkText);
                                    if ( !x_wstr_endswith(pszMarkText, nLen, L"\\b", 2) )
                                    {
                                        // the trailing "\\b"
                                        pszMarkText[nLen] = L'\\'; ++nLen;
                                        pszMarkText[nLen] = L'b'; ++nLen;
                                        pszMarkText[nLen] = 0;
                                    }
                                    if ( !x_wstr_startswith(pszMarkText, L"\\b") )
                                    {
                                        // the leading "\\b"
                                        pszMarkText = szMarkTextBuf;
                                        pszMarkText[0] = L'\\';
                                        pszMarkText[1] = L'b';
                                    }
                                }
                                hlParams.wszMarkText = pszMarkText;
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
