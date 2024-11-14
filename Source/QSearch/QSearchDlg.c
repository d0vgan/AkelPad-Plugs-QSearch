#define AEC_FUNCTIONS

#include "QSearch.h"
#include "QSearchDlg.h"
#include "QSearchSettDlg.h"
#include "QSearchLng.h"
#include "XMemStrFunc.h"


#define  QSEARCH_FIRST          0x000001
#define  QSEARCH_NEXT           0x000002
#define  QSEARCH_FINDALL        0x000010
#define  QSEARCH_FINDALLFILES   0x000020
#define  QSEARCH_COUNTALL       0x000040
#define  QSEARCH_SEL            0x000100
#define  QSEARCH_SEL_FINDUP     0x000200
#define  QSEARCH_NOFINDUP_VK    0x001000 // affects VK_QS_FINDUP
#define  QSEARCH_NOFINDBEGIN_VK 0x002000 // affects VK_QS_FINDBEGIN, does _not_ affect the OPTF_SRCH_FROM_BEGINNING
#define  QSEARCH_FINDUP         0x004000
#define  QSEARCH_FINDBEGIN      0x008000
#define  QSEARCH_USEDELAY       0x010000
#define  QSEARCH_NOSETSEL_FIRST 0x100000
#define  QSEARCH_NOHISTORYUPD   0x200000

#define  QSEARCH_EOF_NONE    0
#define  QSEARCH_EOF_DOWN    0x0001
#define  QSEARCH_EOF_UP      0x0002
#define  QSEARCH_EOF_MASK    0x000F
#define  QSEARCH_EOF_IGNORE  0x0100

#define  VK_QS_FINDBEGIN     VK_MENU     // Alt
#define  VK_QS_FINDUP        VK_SHIFT    // Shift
#define  VK_QS_PICKUPTEXT    VK_CONTROL  // Ctrl

#define  VK_QS_WW_SRCH_MODE  VK_CONTROL  // Ctrl


// extern vars
extern PluginState     g_Plugin;
extern QSearchDlgState g_QSearchDlg;
extern QSearchOpt      g_Options;
extern wchar_t         g_szFunctionQSearchAW[128];
extern BOOL            g_bHighlightPlugin;
extern BOOL            g_bLogPlugin;
extern BOOL            g_bWordJustSelectedByFnd;
extern BOOL            g_bFrameActivated;


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

DWORD getFindAllFlags(const DWORD dwOptFlags[])
{
    DWORD dwFindAllFlags = 0;

    if ( dwOptFlags[OPTF_SRCH_USE_SPECIALCHARS] )
        dwFindAllFlags |= QS_FAF_SPECCHAR;
    else if ( dwOptFlags[OPTF_SRCH_USE_REGEXP] )
        dwFindAllFlags |= QS_FAF_REGEXP;
    if ( dwOptFlags[OPTF_SRCH_MATCHCASE] )
        dwFindAllFlags |= QS_FAF_MATCHCASE;
    if ( dwOptFlags[OPTF_SRCH_WHOLEWORD] )
        dwFindAllFlags |= QS_FAF_WHOLEWORD;

    return dwFindAllFlags;
}

// plugin call helpers
void CallPluginFuncA(const char* cszFuncA, void* pParams)
{
    PLUGINCALLSENDA pcsA;

    x_zero_mem(&pcsA, sizeof(PLUGINCALLSENDA));
    pcsA.pFunction = cszFuncA;
    pcsA.lParam = (LPARAM) pParams;

    SendMessageA( g_Plugin.hMainWnd, AKD_DLLCALLA, 0, (LPARAM) &pcsA );
}
void CallPluginFuncW(const wchar_t* cszFuncW, void* pParams)
{
    PLUGINCALLSENDW pcsW;

    x_zero_mem(&pcsW, sizeof(PLUGINCALLSENDW));
    pcsW.pFunction = cszFuncW;
    pcsW.lParam = (LPARAM) pParams;

    SendMessageW( g_Plugin.hMainWnd, AKD_DLLCALLW, 0, (LPARAM) &pcsW );
}

static DWORD getPluginHotKeyA(const char* cszPluginFuncA)
{
    PLUGINFUNCTION* pfA;

    pfA = (PLUGINFUNCTION *) SendMessage( g_Plugin.hMainWnd, AKD_DLLFINDA, (WPARAM) cszPluginFuncA, 0 );

    return pfA ? pfA->wHotkey : 0;
}

static DWORD getPluginHotKeyW(const wchar_t* cszPluginFuncW)
{
    PLUGINFUNCTION* pfW;

    pfW = (PLUGINFUNCTION *) SendMessageW( g_Plugin.hMainWnd, AKD_DLLFINDW, (WPARAM) cszPluginFuncW, 0 );

    return pfW ? pfW->wHotkey : 0;
}



/* >>>>>>>>>>>>>>>>>>>>>>>> qsearch plugin >>>>>>>>>>>>>>>>>>>>>>>> */
const char* cszQSearchFindAllA = "QSearch::FindAll";
const wchar_t* cszQSearchFindAllW = L"QSearch::FindAll";

const char* cszGoToNextFindAllMatchA = "QSearch::GoToNextFindAllMatch";
const wchar_t* cszGoToNextFindAllMatchW = L"QSearch::GoToNextFindAllMatch";

const char* cszGoToPrevFindAllMatchA = "QSearch::GoToPrevFindAllMatch";
const wchar_t* cszGoToPrevFindAllMatchW = L"QSearch::GoToPrevFindAllMatch";

static DWORD getQSearchHotKey(void)
{
    if ( g_szFunctionQSearchAW[0] )
    {
        if ( g_Plugin.bOldWindows )
        {
            return getPluginHotKeyA((const char *) g_szFunctionQSearchAW);
        }
        else
        {
            return getPluginHotKeyW(g_szFunctionQSearchAW);
        }
    }
    return 0;
}

static DWORD getFindAllHotKey(void)
{
    if ( g_Plugin.bOldWindows )
    {
        return getPluginHotKeyA(cszQSearchFindAllA);
    }

    return getPluginHotKeyW(cszQSearchFindAllW);
}

static DWORD getGoToNextFindAllMatchHotKey(void)
{
    if ( g_Plugin.bOldWindows )
    {
        return getPluginHotKeyA(cszGoToNextFindAllMatchA);
    }

    return getPluginHotKeyW(cszGoToNextFindAllMatchW);
}

static DWORD getGoToPrevFindAllMatchHotKey(void)
{
    if ( g_Plugin.bOldWindows )
    {
        return getPluginHotKeyA(cszGoToPrevFindAllMatchA);
    }

    return getPluginHotKeyW(cszGoToPrevFindAllMatchW);
}
/* <<<<<<<<<<<<<<<<<<<<<<<< qsearch plugin <<<<<<<<<<<<<<<<<<<<<<<< */


/* >>>>>>>>>>>>>>>>>>>>>>>> highlight plugin >>>>>>>>>>>>>>>>>>>>>>>> */
const char*    cszHighlightMainA = "Coder::HighLight";
const wchar_t* cszHighlightMainW = L"Coder::HighLight";

const char*    cszCoderSettingsA = "Coder::Settings";
const wchar_t* cszCoderSettingsW = L"Coder::Settings";

void CallHighlightMain(void* phlParams)
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

void CallCoderSettings(void* pstParams)
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

BOOL IsHighlightMainActive(void)
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

void GetCoderAliasW(wchar_t* pszAliasBufW)
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

void SetCoderAliasW(const wchar_t* cszAliasBufW)
{
    DLLECCODERSETTINGS_SETALIAS stParams;

    stParams.dwStructSize = sizeof(DLLECCODERSETTINGS_SETALIAS);
    stParams.nAction = DLLA_CODER_SETALIAS;
    stParams.pszAlias = (const unsigned char *) cszAliasBufW;

    CallCoderSettings( &stParams );
}

INT_PTR GetCoderVariableW(HWND hWndEdit, const wchar_t* cszVarName, wchar_t* pszVarValue)
{
    INT_PTR nValueLen;
    DLLCODERSETTINGS_GETVARIABLE stParams;

    nValueLen = 0;
    if ( pszVarValue )
    {
        pszVarValue[0] = 0;
    }

    stParams.dwStructSize = sizeof(DLLCODERSETTINGS_GETVARIABLE);
    stParams.nAction = DLLA_CODER_GETVARIABLE;
    stParams.hEditWnd = hWndEdit;
    stParams.hEditDoc = NULL;
    stParams.pszVarName = cszVarName;
    stParams.pszVarValue = pszVarValue;
    stParams.pnVarValueLen = &nValueLen;

    CallPluginFuncW(cszCoderSettingsW, &stParams);

    return nValueLen;
}
/* <<<<<<<<<<<<<<<<<<<<<<<< highlight plugin <<<<<<<<<<<<<<<<<<<<<<<< */


/* >>>>>>>>>>>>>>>>>>>>>>>> log plugin >>>>>>>>>>>>>>>>>>>>>>>> */
const char*    cszLogOutputA = "Log::Output";
const wchar_t* cszLogOutputW = L"Log::Output";

void CallLogOutput(void* ploParams)
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

BOOL IsLogOutputActive(void)
{
    if ( g_Plugin.bOldWindows )
    {
        PLUGINFUNCTION *pfA = (PLUGINFUNCTION *) SendMessageA( g_Plugin.hMainWnd,
            AKD_DLLFINDA, (WPARAM) cszLogOutputA, 0 );

        if ( pfA && pfA->bRunning )
        {
            return TRUE;
        }
    }
    else
    {
        PLUGINFUNCTION *pfW = (PLUGINFUNCTION *) SendMessageW( g_Plugin.hMainWnd,
            AKD_DLLFINDW, (WPARAM) cszLogOutputW, 0 );

        if ( pfW && pfW->bRunning )
        {
            return TRUE;
        }
    }

    return FALSE;
}
/* <<<<<<<<<<<<<<<<<<<<<<<< log plugin <<<<<<<<<<<<<<<<<<<<<<<< */


/* >>>>>>>>>>>>>>>>>>>>>>>> qsearchdlg state >>>>>>>>>>>>>>>>>>>>>>>> */
    matchpos_t to_matchpos_ae(const AECHARINDEX* ci, HWND hWndEdit)
    {
        BOOL bWordWrap = (SendMessageW(hWndEdit, AEM_GETWORDWRAP, 0, 0) != AEWW_NONE) ? TRUE : FALSE;
        return to_matchpos_ae_ex(ci, hWndEdit, bWordWrap);
    }

    matchpos_t to_matchpos_ae_ex(const AECHARINDEX* ci, HWND hWndEdit, BOOL bWordWrap)
    {
        int nLine;
        int nPosInLine;
        AECHARINDEX ciTemp;

        nLine = ci->nLine;
        if ( bWordWrap )
        {
            nLine = (int) SendMessage(hWndEdit, AEM_GETUNWRAPLINE, nLine, 0);
        }
        x_mem_cpy(&ciTemp, ci, sizeof(AECHARINDEX));
        nPosInLine = AEC_WrapLineBegin(&ciTemp);
        return to_matchpos(nLine, nPosInLine);
    }

    matchpos_t to_matchpos(unsigned int line, unsigned int pos_in_line)
    {
        matchpos_t pos = line;
    #ifdef _WIN64
        return ((pos << 32) | pos_in_line);
    #else
        pos = Int64ShllMod32(pos, 16); // ShiftCount is in the range of 0-31; we need 32
        return (Int64ShllMod32(pos, 16) | pos_in_line);
    #endif
    }

    unsigned int get_matchpos_line(matchpos_t pos)
    {
    #ifdef _WIN64
        return (unsigned int) (pos >> 32);
    #else
        pos = Int64ShrlMod32(pos, 16);
        return (unsigned int) Int64ShrlMod32(pos, 16);
    #endif
    }

    unsigned int get_matchpos_pos_in_line(matchpos_t pos)
    {
        return (unsigned int) (pos & 0xFFFFFFFF);
    }

    // returns either a 0-based index or -1
    int find_in_sorted_matchpos_array(const matchpos_t* pArr, unsigned int nItems, matchpos_t val, BOOL* pbExactMatch)
    {
        int nBegin;
        int nEnd;
        int nDiv;

        if ( nItems == 0 || val < pArr[0] )
        {
            *pbExactMatch = FALSE;
            return -1;
        }

        nBegin = 0;
        nEnd = nItems; // position after the last item

        for ( ; ; )
        {
            nDiv = (nEnd - nBegin)/2;
            if ( nDiv == 0 )
                break;

            nDiv += nBegin;
            if ( val < pArr[nDiv] )
                nEnd = nDiv;
            else
                nBegin = nDiv;
        }

        *pbExactMatch = (val == pArr[nBegin]) ? TRUE : FALSE;
        return nBegin;
    }

    void tQSSearchResultsItem_Init(tQSSearchResultsItem* pItem)
    {
        x_zero_mem(pItem, sizeof(tQSSearchResultsItem));
    }

    void tQSSearchResultsItem_Assign(tQSSearchResultsItem* pItem, const FRAMEDATA* pFrame, const wchar_t* cszFindWhat, DWORD dwFindAllFlags)
    {
        pItem->pFrame = pFrame;
        pItem->dwFindAllFlags = dwFindAllFlags;
        lstrcpyW(pItem->szFindTextW, cszFindWhat);
    }

    void tQSSearchResultsItem_Copy(tQSSearchResultsItem* pDstItem, const tQSSearchResultsItem* pSrcItem)
    {
        x_mem_cpy(pDstItem, pSrcItem, sizeof(tQSSearchResultsItem));
    }

    void initializeQSearchDlgState(QSearchDlgState* pQSearchDlg)
    {
        int i;

        pQSearchDlg->hDlg = NULL;
        pQSearchDlg->hFindEdit = NULL;
        pQSearchDlg->hFindListBox = NULL;
        pQSearchDlg->hBtnFindNext = NULL;
        pQSearchDlg->hBtnFindPrev = NULL;
        pQSearchDlg->hBtnFindAll = NULL;
        pQSearchDlg->hStInfo = NULL;
        pQSearchDlg->hPopupMenu = NULL;
        pQSearchDlg->hFindAllPopupMenu = NULL;
        pQSearchDlg->dwHotKeyQSearch = 0;
        pQSearchDlg->dwHotKeyFindAll = 0;
        pQSearchDlg->dwHotKeyGoToNextFindAllMatch = 0;
        pQSearchDlg->dwHotKeyGoToPrevFindAllMatch = 0;
        //pQSearchDlg->bOnDlgStart = FALSE;
        pQSearchDlg->bQSearching = FALSE;
        pQSearchDlg->bIsQSearchingRightNow = FALSE;
        pQSearchDlg->bMouseJustLeavedFindEdit = FALSE;
        pQSearchDlg->pDockData = NULL;
        pQSearchDlg->nResultsItemsCount = 0;
        for ( i = 0; i < MAX_RESULTS_FRAMES; i++ )
        {
            tQSSearchResultsItem_Init(&pQSearchDlg->SearchResultsItems[i]);
        }
        pQSearchDlg->szFindTextAW[0] = 0;
        pQSearchDlg->szFindAllFindTextW[0] = 0;
        pQSearchDlg->szLastHighlightTextW[0] = 0;
        pQSearchDlg->dwFindAllFlags = 0;
        pQSearchDlg->dwLastHighlightFlags = 0;
        pQSearchDlg->uSearchOrigin = QS_SO_UNKNOWN;
        pQSearchDlg->uWmShowFlags = 0;
        pQSearchDlg->crTextColor = GetSysColor(COLOR_WINDOWTEXT);
        pQSearchDlg->crBkgndColor = GetSysColor(COLOR_WINDOW);
        pQSearchDlg->hBkgndBrush = NULL;
        pQSearchDlg->hCurrentMatchSetInfoEditWnd = NULL;
        pQSearchDlg->nGoToNextFindAllPosToCompare = -1;
        pQSearchDlg->bFindAllWasUsingLogOutput = TRUE;
        tDynamicBuffer_Init(&pQSearchDlg->currentMatchesBuf);
        tDynamicBuffer_Init(&pQSearchDlg->findAllFramesBuf);
        tDynamicBuffer_Init(&pQSearchDlg->findAllMatchesBuf);
    }

    static void removeFrameFromResults(QSearchDlgState* pQSearchDlg, const FRAMEDATA* pFrame)
    {
        int i, j;

        i = 0;
        j = 0;
        while ( j < pQSearchDlg->nResultsItemsCount )
        {
            if ( pQSearchDlg->SearchResultsItems[j].pFrame != pFrame )
            {
                if ( i != j )
                {
                    tQSSearchResultsItem_Copy(&pQSearchDlg->SearchResultsItems[i], &pQSearchDlg->SearchResultsItems[j]);
                }
                ++i;
            }
            ++j;
        }

        if ( i != j ) // pFrame has been removed
        {
            pQSearchDlg->nResultsItemsCount = i;
            tQSSearchResultsItem_Init(&pQSearchDlg->SearchResultsItems[i]);
        }
    }

    void QSearchDlgState_AddResultsFrame(QSearchDlgState* pQSearchDlg, const FRAMEDATA* pFrame, const wchar_t* cszFindWhat, DWORD dwFindAllFlags)
    {
        if ( pQSearchDlg->nResultsItemsCount != 0 )
        {
            removeFrameFromResults(pQSearchDlg, pFrame); // to ensure that pFrame is unique
        }

        if ( pQSearchDlg->nResultsItemsCount == MAX_RESULTS_FRAMES ) // full
        {
            int i;

            for ( i = 0; i < MAX_RESULTS_FRAMES - 1; ++i )
            {
                tQSSearchResultsItem_Copy(&pQSearchDlg->SearchResultsItems[i], &pQSearchDlg->SearchResultsItems[i + 1]);
            }
            pQSearchDlg->nResultsItemsCount = MAX_RESULTS_FRAMES - 1;
        }

        tQSSearchResultsItem_Assign(&pQSearchDlg->SearchResultsItems[pQSearchDlg->nResultsItemsCount], pFrame, cszFindWhat, dwFindAllFlags);
        ++pQSearchDlg->nResultsItemsCount;
    }

    void QSearchDlgState_RemoveResultsFrame(QSearchDlgState* pQSearchDlg, const FRAMEDATA* pFrame)
    {
        if ( pQSearchDlg->nResultsItemsCount != 0 )
        {
            removeFrameFromResults(pQSearchDlg, pFrame);
        }
    }

    int QSearchDlgState_FindResultsFrame(const QSearchDlgState* pQSearchDlg, const FRAMEDATA* pFrame)
    {
        if ( pQSearchDlg->nResultsItemsCount != 0 )
        {
            int i;

            for ( i = 0; i < pQSearchDlg->nResultsItemsCount; ++i )
            {
                if ( pQSearchDlg->SearchResultsItems[i].pFrame == pFrame )
                    return i;
            }
        }

        return -1;
    }

    const FRAMEDATA* QSearchDlgState_GetSearchResultsFrame(const QSearchDlgState* pQSearchDlg)
    {
        const tQSSearchResultsItem* pItem = QSearchDlgState_GetSearchResultsItem((QSearchDlgState *) pQSearchDlg);
        return (pItem != NULL ? pItem->pFrame : NULL);
    }

    tQSSearchResultsItem* QSearchDlgState_GetSearchResultsItem(QSearchDlgState* pQSearchDlg)
    {
        int n = pQSearchDlg->nResultsItemsCount;
        return (n != 0 ? &pQSearchDlg->SearchResultsItems[n - 1] : NULL);
    }

    void QSearchDlgState_addCurrentMatch(QSearchDlgState* pQSearchDlg, matchpos_t nMatchPos)
    {
        tDynamicBuffer_Append(&pQSearchDlg->currentMatchesBuf, &nMatchPos, sizeof(matchpos_t));
    }

    void QSearchDlgState_clearCurrentMatches(QSearchDlgState* pQSearchDlg, BOOL bFreeMemory)
    {
        pQSearchDlg->hCurrentMatchSetInfoEditWnd = NULL;
        if ( bFreeMemory )
            tDynamicBuffer_Free(&pQSearchDlg->currentMatchesBuf);
        else
            tDynamicBuffer_Clear(&pQSearchDlg->currentMatchesBuf);
    }

    int QSearchDlgState_findInCurrentMatches(const QSearchDlgState* pQSearchDlg, matchpos_t nMatchPos, BOOL* pbExactMatch)
    {
        // items in the currentMatchesBuf are sorted
        return find_in_sorted_matchpos_array(
            (const matchpos_t *) pQSearchDlg->currentMatchesBuf.ptr,
            (unsigned int) (pQSearchDlg->currentMatchesBuf.nBytesStored/sizeof(matchpos_t)),
            nMatchPos,
            pbExactMatch
        );
    }

    void QSearchDlgState_addFindAllMatch(QSearchDlgState* pQSearchDlg, matchpos_t nMatchPos)
    {
        tDynamicBuffer_Append(&pQSearchDlg->findAllMatchesBuf, &nMatchPos, sizeof(matchpos_t));
    }

    void QSearchDlgState_addFindAllFrameItem(QSearchDlgState* pQSearchDlg, const tQSFindAllFrameItem* pItem)
    {
        tDynamicBuffer_Append(&pQSearchDlg->findAllFramesBuf, pItem, sizeof(tQSFindAllFrameItem));
    }

    void QSearchDlgState_clearFindAllMatchesAndFrames(QSearchDlgState* pQSearchDlg, BOOL bFreeMemory)
    {
        pQSearchDlg->szFindAllFindTextW[0] = 0;
        pQSearchDlg->dwFindAllFlags = 0;
        pQSearchDlg->nGoToNextFindAllPosToCompare = -1;
        pQSearchDlg->bFindAllWasUsingLogOutput = TRUE;
        if ( bFreeMemory )
        {
            tDynamicBuffer_Free(&pQSearchDlg->findAllFramesBuf);
            tDynamicBuffer_Free(&pQSearchDlg->findAllMatchesBuf);
        }
        else
        {
            tDynamicBuffer_Clear(&pQSearchDlg->findAllFramesBuf);
            tDynamicBuffer_Clear(&pQSearchDlg->findAllMatchesBuf);
        }
    }

    int QSearchDlgState_findInFindAllFrameItemMatches(const QSearchDlgState* pQSearchDlg, const tQSFindAllFrameItem* pItem, matchpos_t nMatchPos, BOOL* pbExactMatch)
    {
        // items in the findAllMatchesBuf are sorted within each pItem's range
        return find_in_sorted_matchpos_array(
            QSearchDlgState_getFindAllFrameItemMatches(pQSearchDlg, pItem),
            (unsigned int) pItem->nMatches,
            nMatchPos,
            pbExactMatch
        );
    }

    int QSearchDlgState_getFindAllFramesCount(const QSearchDlgState* pQSearchDlg)
    {
        return (pQSearchDlg->findAllFramesBuf.nBytesStored/sizeof(tQSFindAllFrameItem));
    }

    const tQSFindAllFrameItem* QSearchDlgState_getFindAllFrameItemByFrame(const QSearchDlgState* pQSearchDlg, const FRAMEDATA* pFrame)
    {
        // note: it may return an invalid item (QS_FIS_INVALID)!
        const tQSFindAllFrameItem* pItem;
        const tQSFindAllFrameItem* pEndItem;
        BOOL bFound;

        pItem = (const tQSFindAllFrameItem *) pQSearchDlg->findAllFramesBuf.ptr;
        pEndItem = pItem + pQSearchDlg->findAllFramesBuf.nBytesStored/sizeof(tQSFindAllFrameItem);
        bFound = FALSE;

        for ( ; pItem < pEndItem; ++pItem )
        {
            if ( pFrame == pItem->pFrame )
            {
                bFound = TRUE;
                break;
            }
        }

        return bFound ? pItem : NULL;
    }

    const matchpos_t* QSearchDlgState_getFindAllFrameItemMatches(const QSearchDlgState* pQSearchDlg, const tQSFindAllFrameItem* pItem)
    {
        return (const matchpos_t *) (((const BYTE *) pQSearchDlg->findAllMatchesBuf.ptr) + pItem->nBufBytesOffset);
    }

    matchpos_t QSearchDlgState_getFindAllFrameItemMatchAt(const QSearchDlgState* pQSearchDlg, const tQSFindAllFrameItem* pItem, int idx)
    {
        if ( idx >= 0 && idx < pItem->nMatches )
        {
            return QSearchDlgState_getFindAllFrameItemMatches(pQSearchDlg, pItem)[idx];
        }
        return MATCHPOS_INVALID;
    }

    const tQSFindAllFrameItem* QSearchDlgState_getFindAllValidFrameItemForward(QSearchDlgState* pQSearchDlg, const tQSFindAllFrameItem* pItem)
    {
        const tQSFindAllFrameItem* pEndItem;
        BOOL bFound;

        pEndItem = ((const tQSFindAllFrameItem *) pQSearchDlg->findAllFramesBuf.ptr) + pQSearchDlg->findAllFramesBuf.nBytesStored/sizeof(tQSFindAllFrameItem);
        bFound = FALSE;

        for ( ; pItem < pEndItem; ++pItem )
        {
            if ( !(pItem->nItemState & QS_FIS_INVALID) )
            {
                if ( SendMessageW(g_Plugin.hMainWnd, AKD_FRAMEISVALID, 0, (LPARAM) pItem->pFrame) )
                {
                    bFound = TRUE;
                    break;
                }

                ((tQSFindAllFrameItem *) pItem)->nItemState |= QS_FIS_INVALID;
            }
        }

        return bFound ? pItem : NULL;
    }

    const tQSFindAllFrameItem* QSearchDlgState_getFindAllValidFrameItemBackward(QSearchDlgState* pQSearchDlg, const tQSFindAllFrameItem* pItem)
    {
        const tQSFindAllFrameItem* pBeginItem;
        BOOL bFound;

        pBeginItem = (const tQSFindAllFrameItem *) pQSearchDlg->findAllFramesBuf.ptr;
        bFound = FALSE;

        for ( ; pItem >= pBeginItem; --pItem )
        {
            if ( !(pItem->nItemState & QS_FIS_INVALID) )
            {
                if ( SendMessageW(g_Plugin.hMainWnd, AKD_FRAMEISVALID, 0, (LPARAM) pItem->pFrame) )
                {
                    bFound = TRUE;
                    break;
                }

                ((tQSFindAllFrameItem *) pItem)->nItemState |= QS_FIS_INVALID;
            }
        }

        return bFound ? pItem : NULL;
    }

    BOOL QSearchDlgState_isFindAllFrameItemInternallyValid(const QSearchDlgState* pQSearchDlg, const tQSFindAllFrameItem* pItem)
    {
        return ( !(pItem->nItemState & QS_FIS_INVALID) && (pItem->nBufBytesOffset + pItem->nMatches*sizeof(matchpos_t) <= pQSearchDlg->findAllMatchesBuf.nBytesStored) );
    }

    BOOL QSearchDlgState_isFindAllMatchesEmpty(const QSearchDlgState* pQSearchDlg)
    {
        return ( pQSearchDlg->findAllMatchesBuf.nBytesStored == 0 || pQSearchDlg->findAllFramesBuf.nBytesStored == 0 );
    }

    BOOL QSearchDlgState_isFindAllSearchEqualToTheCurrentSearch(const QSearchDlgState* pQSearchDlg, const wchar_t* cszFindWhat, DWORD dwFindAllFlags)
    {
        // Note: Be sure to call this function only in Unicode Windows!
        return ( pQSearchDlg->dwFindAllFlags == dwFindAllFlags &&
                 ((pQSearchDlg->dwFindAllFlags & QS_FAF_MATCHCASE) ? lstrcmpW : lstrcmpiW)(cszFindWhat, pQSearchDlg->szFindAllFindTextW) == 0 );
    }

    BOOL QSearchDlgState_isLastHighlightedEqualToTheSearch(const QSearchDlgState* pQSearchDlg, const wchar_t* cszFindWhat, DWORD dwFindAllFlags)
    {
        wchar_t szFindWhatW[MAX_TEXT_SIZE];

        if ( g_Plugin.bOldWindows )
        {
            szFindWhatW[0] = 0;
            MultiByteToWideChar( CP_ACP, 0, (LPCSTR) cszFindWhat, -1, szFindWhatW, MAX_TEXT_SIZE - 1 );
            cszFindWhat = szFindWhatW;
        }

        return QSearchDlgState_isLastHighlightedEqualToTheSearchW(pQSearchDlg, cszFindWhat, dwFindAllFlags);
    }

    BOOL QSearchDlgState_isLastHighlightedEqualToTheSearchW(const QSearchDlgState* pQSearchDlg, const wchar_t* cszFindWhatW, DWORD dwFindAllFlags)
    {
        if ( dwFindAllFlags != g_QSearchDlg.dwLastHighlightFlags )
            return FALSE;

        if ( !g_Plugin.bOldWindows && !(dwFindAllFlags & QS_FAF_MATCHCASE) )
            return (lstrcmpiW(cszFindWhatW, g_QSearchDlg.szLastHighlightTextW) == 0);

        return (x_wstr_cmp(cszFindWhatW, g_QSearchDlg.szLastHighlightTextW) == 0);
    }

    void QSearchDlgState_clearLastHighlighted(QSearchDlgState* pQSearchDlg)
    {
        pQSearchDlg->szLastHighlightTextW[0] = 0;
        pQSearchDlg->dwLastHighlightFlags = 0;
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
static INT     qs_nEditIsEOF = 0;
static int     qs_nEditEOF = 0;
static BOOL    qs_bEditTextChanged = TRUE;
static BOOL    qs_bHotKeyQSearchPressedOnShow = FALSE;
static BOOL    qs_bForceFindFirst = FALSE;
static BOOL    qs_bBtnFindIsFocused = FALSE;

static UINT_PTR nFindAllTimerId = 0;
static CRITICAL_SECTION csFindAllTimerId;


static BOOL isCheckBoxChecked(HWND hDlg, int nItemId)
{
    BOOL bChecked = FALSE;
    HWND hCh = GetDlgItem(hDlg, nItemId);
    if ( hCh )
    {
        if ( SendMessage(hCh, BM_GETCHECK, 0, 0) == BST_CHECKED )
            bChecked = TRUE;
    }
    return bChecked;
}

void qsSetInfoEmpty(void)
{
    if ( g_QSearchDlg.hStInfo )
    {
        if ( g_Plugin.bOldWindows )
            SetWindowTextA(g_QSearchDlg.hStInfo, "");
        else
            SetWindowTextW(g_QSearchDlg.hStInfo, L"");

        #ifdef _DEBUG
          Debug_OutputW(L"%S -> InfoText = \"\"\n", __func__);
        #endif
    }

    QSearchDlgState_clearCurrentMatches(&g_QSearchDlg, FALSE);
}

static BOOL endsWithSubStrA(const char* szStrA, int nLen, const char* szSubA, int nSubLen)
{
    return ( nSubLen != 0 && nLen >= nSubLen && lstrcmpA(szStrA + nLen - nSubLen, szSubA) == 0 );
}

static BOOL endsWithSubStrW(const wchar_t* szStrW, int nLen, const wchar_t* szSubW, int nSubLen)
{
    return ( nSubLen != 0 && nLen >= nSubLen && lstrcmpW(szStrW + nLen - nSubLen, szSubW) == 0 );
}

static int appendToInfoTextA(char szInfoTextA[], int nInfoLen, const char* szTextAppendA, int nLenAppend)
{
    if ( nLenAppend != 0 )
    {
        if ( szInfoTextA[0] != 0 )
        {
            lstrcpyA(szInfoTextA + nInfoLen, " ");
            ++nInfoLen;
        }
        lstrcpyA(szInfoTextA + nInfoLen, szTextAppendA);
        nInfoLen += nLenAppend;
    }
    return nInfoLen;
}

static int appendToInfoTextW(wchar_t szInfoTextW[], int nInfoLen, const wchar_t* szTextAppendW, int nLenAppend)
{
    if ( nLenAppend != 0 )
    {
        if ( szInfoTextW[0] != 0 )
        {
            lstrcpyW(szInfoTextW + nInfoLen, L" ");
            ++nInfoLen;
        }
        lstrcpyW(szInfoTextW + nInfoLen, szTextAppendW);
        nInfoLen += nLenAppend;
    }
    return nInfoLen;
}

void qsSetInfoOccurrencesFound(unsigned int nOccurrences, unsigned int nFlags)
{
    #ifdef _DEBUG
      Debug_OutputA("%s: nOccurrences=%u, nFlags=%u\n", __func__, nOccurrences, nFlags);
    #endif

    if ( g_QSearchDlg.hStInfo )
    {
        int nLen;
        INT nIsEOF;
        wchar_t szInfoTextW[128];

        nIsEOF = 0;
        szInfoTextW[0] = 0;
        nLen = GetWindowTextW(g_QSearchDlg.hStInfo, szInfoTextW, 128 - 1);
        if ( szInfoTextW[0] != 0 )
        {
            if ( endsWithSubStrW(szInfoTextW, nLen, g_Options.szStatusEofCrossedDownAW, g_Options.nLenStatusEofCrossedDown) )
                nIsEOF = QSEARCH_EOF_DOWN;
            else if ( endsWithSubStrW(szInfoTextW, nLen, g_Options.szStatusEofCrossedUpAW, g_Options.nLenStatusEofCrossedUp) )
                nIsEOF = QSEARCH_EOF_UP;
        }

        nLen = 0;
        if ( ((nFlags & QS_SIOF_REMOVECURRENTMATCH) == 0) &&
             ((g_Options.dwFindAllMode & QS_FINDALL_AUTO_COUNT_FLAG) != 0) &&
             (nOccurrences != 0) &&
             (nOccurrences == g_QSearchDlg.currentMatchesBuf.nBytesStored/sizeof(matchpos_t)) )
        {
            matchpos_t nMatchPos;
            int nMatch;
            BOOL bExactMatch;
            AECHARINDEX aeCi;

            g_QSearchDlg.hCurrentMatchSetInfoEditWnd = GetWndEdit(g_Plugin.hMainWnd);
            SendMessage( g_QSearchDlg.hCurrentMatchSetInfoEditWnd, AEM_GETINDEX, AEGI_FIRSTSELCHAR, (LPARAM) &aeCi );
            nMatchPos = to_matchpos_ae(&aeCi, g_QSearchDlg.hCurrentMatchSetInfoEditWnd);
            nMatch = QSearchDlgState_findInCurrentMatches(&g_QSearchDlg, nMatchPos, &bExactMatch);
            if ( nMatch != -1 )
            {
                if ( bExactMatch )
                    nLen = wsprintfW(szInfoTextW, qsearchGetStringW(QS_STRID_FINDALL_OCCURRENCEOF), nMatch + 1);
                else
                    nLen = wsprintfW(szInfoTextW, qsearchGetStringW(QS_STRID_FINDALL_OCCURRENCEOF_NOTEXACT), nMatch + 1);
            }
            else
            {
                g_QSearchDlg.hCurrentMatchSetInfoEditWnd = NULL;
            }
        }
        nLen += wsprintfW(szInfoTextW + nLen, qsearchGetStringW(QS_STRID_FINDALL_OCCURRENCESFOUND), nOccurrences);
        if ( nLen > 0 )
        {
            --nLen;
            szInfoTextW[nLen] = 0; // without the trailing '.'
        }

        if ( nIsEOF != 0 && (nFlags & QS_SIOF_REMOVECURRENTMATCH) == 0 )
        {
            lstrcpyW(szInfoTextW + nLen, L" ");
            ++nLen;
            if ( nIsEOF == QSEARCH_EOF_DOWN )
            {
                lstrcpyW(szInfoTextW + nLen, g_Options.szStatusEofCrossedDownAW);
                nLen += g_Options.nLenStatusEofCrossedDown;
            }
            else
            {
                lstrcpyW(szInfoTextW + nLen, g_Options.szStatusEofCrossedUpAW);
                nLen += g_Options.nLenStatusEofCrossedUp;
            }
        }

        SetWindowTextW(g_QSearchDlg.hStInfo, szInfoTextW);

        #ifdef _DEBUG
          Debug_OutputW(L"%S -> InfoText = \"%s\"\n", __func__, szInfoTextW);
        #endif
    }
}

static int removeEofOrNotFoundFromInfoTextA(char szInfoTextA[], int nLen)
{
    int nEofLen = 0;

    if ( endsWithSubStrA(szInfoTextA, nLen, (const char *) g_Options.szStatusNotFoundAW, g_Options.nLenStatusNotFound) )
        nEofLen = g_Options.nLenStatusNotFound;
    else if ( endsWithSubStrA(szInfoTextA, nLen, (const char *) g_Options.szStatusNotRegExpAW, g_Options.nLenStatusNotRegExp) )
        nEofLen = g_Options.nLenStatusNotRegExp;
    else if ( endsWithSubStrA(szInfoTextA, nLen, (const char *) g_Options.szStatusEofCrossedDownAW, g_Options.nLenStatusEofCrossedDown) )
        nEofLen = g_Options.nLenStatusEofCrossedDown;
    else if ( endsWithSubStrA(szInfoTextA, nLen, (const char *) g_Options.szStatusEofCrossedUpAW, g_Options.nLenStatusEofCrossedUp) )
        nEofLen = g_Options.nLenStatusEofCrossedUp;

    if ( nEofLen != 0 )
    {
        nLen -= nEofLen;
        if ( nLen != 0 )
            --nLen;
        szInfoTextA[nLen] = 0;
    }

    return nLen;
}

static int removeEofOrNotFoundFromInfoTextW(wchar_t szInfoTextW[], int nLen)
{
    int nEofLen = 0;

    if ( endsWithSubStrW(szInfoTextW, nLen, g_Options.szStatusNotFoundAW, g_Options.nLenStatusNotFound) )
        nEofLen = g_Options.nLenStatusNotFound;
    else if ( endsWithSubStrW(szInfoTextW, nLen, g_Options.szStatusNotRegExpAW, g_Options.nLenStatusNotRegExp) )
        nEofLen = g_Options.nLenStatusNotRegExp;
    else if ( endsWithSubStrW(szInfoTextW, nLen, g_Options.szStatusEofCrossedDownAW, g_Options.nLenStatusEofCrossedDown) )
        nEofLen = g_Options.nLenStatusEofCrossedDown;
    else if ( endsWithSubStrW(szInfoTextW, nLen, g_Options.szStatusEofCrossedUpAW, g_Options.nLenStatusEofCrossedUp) )
        nEofLen = g_Options.nLenStatusEofCrossedUp;

    if ( nEofLen != 0 )
    {
        nLen -= nEofLen;
        if ( nLen != 0 )
            --nLen;
        szInfoTextW[nLen] = 0;
    }

    return nLen;
}

static void qsSetInfoEofOrNotFound(INT nIsEOF, BOOL bNotFound, BOOL bNotRegExp)
{
    if ( !g_QSearchDlg.hStInfo )
        return;

    if ( g_Plugin.bOldWindows )
    {
        char szInfoTextA[128];
        char szInfoTextA_0[128];
        int nLen;

        szInfoTextA[0] = 0;
        nLen = GetWindowTextA(g_QSearchDlg.hStInfo, szInfoTextA, 128 - 20);
        lstrcpyA(szInfoTextA_0, szInfoTextA);

        nLen = removeEofOrNotFoundFromInfoTextA(szInfoTextA, nLen);

        if ( bNotRegExp )
        {
            nLen = appendToInfoTextA(szInfoTextA, nLen, (const char *) g_Options.szStatusNotRegExpAW, g_Options.nLenStatusNotRegExp);
        }
        else if ( bNotFound )
        {
            nLen = appendToInfoTextA(szInfoTextA, nLen, (const char *) g_Options.szStatusNotFoundAW, g_Options.nLenStatusNotFound);
        }
        else if ( nIsEOF != 0 )
        {
            const char* cszStatusEofA;
            int nEofLen;

            if ( nIsEOF == QSEARCH_EOF_DOWN )
            {
                cszStatusEofA = (const char *) g_Options.szStatusEofCrossedDownAW;
                nEofLen = g_Options.nLenStatusEofCrossedDown;
            }
            else
            {
                cszStatusEofA = (const char *) g_Options.szStatusEofCrossedUpAW;
                nEofLen = g_Options.nLenStatusEofCrossedUp;
            }
            nLen = appendToInfoTextA(szInfoTextA, nLen, cszStatusEofA, nEofLen);
        }

        if ( lstrcmpA(szInfoTextA, szInfoTextA_0) != 0 )
        {
            SetWindowTextA(g_QSearchDlg.hStInfo, szInfoTextA);

            #ifdef _DEBUG
              Debug_OutputA("%s -> InfoText = \"%s\"\n", __func__, szInfoTextA);
            #endif
        }
    }
    else
    {
        wchar_t szInfoTextW[128];
        wchar_t szInfoTextW_0[128];
        int nLen;

        szInfoTextW[0] = 0;
        nLen = GetWindowTextW(g_QSearchDlg.hStInfo, szInfoTextW, 128 - 20);
        lstrcpyW(szInfoTextW_0, szInfoTextW);

        nLen = removeEofOrNotFoundFromInfoTextW(szInfoTextW, nLen);

        if ( bNotRegExp )
        {
            nLen = appendToInfoTextW(szInfoTextW, nLen, g_Options.szStatusNotRegExpAW, g_Options.nLenStatusNotRegExp);
        }
        else if ( bNotFound )
        {
            nLen = appendToInfoTextW(szInfoTextW, nLen, g_Options.szStatusNotFoundAW, g_Options.nLenStatusNotFound);
        }
        else if ( nIsEOF != 0 )
        {
            const wchar_t* cszStatusEofW;
            int nEofLen;

            if ( nIsEOF == QSEARCH_EOF_DOWN )
            {
                cszStatusEofW = g_Options.szStatusEofCrossedDownAW;
                nEofLen = g_Options.nLenStatusEofCrossedDown;
            }
            else
            {
                cszStatusEofW = g_Options.szStatusEofCrossedUpAW;
                nEofLen = g_Options.nLenStatusEofCrossedUp;
            }
            nLen = appendToInfoTextW(szInfoTextW, nLen, cszStatusEofW, nEofLen);
        }

        if ( lstrcmpW(szInfoTextW, szInfoTextW_0) != 0 )
        {
            SetWindowTextW(g_QSearchDlg.hStInfo, szInfoTextW);

            #ifdef _DEBUG
              Debug_OutputW(L"%S -> InfoText = \"%s\"\n", __func__, szInfoTextW);
            #endif
        }
    }
}

// Helpers for find all...
// There is a single reason for so many helpers.
// The reason is: Flexibility.

typedef struct sFindAllContext {
    // input
    const wchar_t* cszFindWhat;
    const AEFINDTEXTW* pFindTextW;
    const FRAMEDATA* pFrame;
    DWORD dwFindAllMode;
    DWORD dwFindAllFlags;
    DWORD dwFindAllResult;
    BOOL  bWordWrap; // in current file
    unsigned int nOccurrences; // in current file
    unsigned int nTotalOccurrences; // in all files
    unsigned int nTotalFiles;
    unsigned int nFilesWithOccurrences;
    // output
    int nLastLine; // last line in the results (in current file)
    int nLastOccurrenceLine; // last line with the occurrence (in current file)
    tDynamicBuffer ResultsBuf;
    tDynamicBuffer OccurrencesBuf;
} tFindAllContext;

typedef void (*tShowFindResults_Init)(tFindAllContext* pFindContext, tDynamicBuffer* pTempBuf);
typedef void (*tShowFindResults_AddOccurrence)(tFindAllContext* pFindContext, const tDynamicBuffer* pOccurrence);
typedef void (*tShowFindResults_Done)(tFindAllContext* pFindContext, tDynamicBuffer* pTempBuf);

typedef void (*tShowFindResults_AllFiles_Init)(tFindAllContext* pFindContext, tDynamicBuffer* pTempBuf);
typedef void (*tShowFindResults_AllFiles_Done)(tFindAllContext* pFindContext, tDynamicBuffer* pTempBuf);

// CountOnly...
static void qsShowFindResults_CountOnly_Init(tFindAllContext* pFindContext, tDynamicBuffer* pTempBuf)
{
    // empty
}

static void qsShowFindResults_CountOnly_AddOccurrence(tFindAllContext* pFindContext, const tDynamicBuffer* pOccurrence)
{
    // empty
}

static void qsShowFindResults_CountOnly_Done(tFindAllContext* pFindContext, tDynamicBuffer* pTempBuf)
{
    qsSetInfoOccurrencesFound_Tracking(pFindContext->nOccurrences, 0, "qsShowFindResults_CountOnly_Done");
}

static void qsShowFindResults_CountOnly_AllFiles_Init(tFindAllContext* pFindContext, tDynamicBuffer* pTempBuf)
{
    // empty
}

static void qsShowFindResults_CountOnly_AllFiles_Done(tFindAllContext* pFindContext, tDynamicBuffer* pTempBuf)
{
    // empty
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

HWND LogOutput_GetEditHwnd(void)
{
    HWND hEditWnd = NULL;
    DLLECLOG_OUTPUT_2 loParams;

    loParams.dwStructSize = sizeof(DLLECLOG_OUTPUT_2);
    loParams.nAction = 2;
    loParams.ptrToEditWnd = &hEditWnd;
    CallLogOutput( &loParams );

    return hEditWnd;
}

static wchar_t getQuoteChar(const tFindAllContext* pFindContext)
{
    wchar_t chQuote = L'\"';
    if ( pFindContext->dwFindAllFlags & QS_FAF_REGEXP )
        chQuote = L'/';

    //if ( pFindContext->dwFindAllFlags & QS_FAF_MATCHCASE )
    //    ...

    //if ( pFindContext->dwFindAllFlags & QS_FAF_WHOLEWORD )
    //    ...

    return chQuote;
}

static UINT_PTR formatSearchingForStringToBuf(tFindAllContext* pFindContext, tDynamicBuffer* pTempBuf)
{
    wchar_t* pszText;
    const wchar_t* cszTextFormat;
    const wchar_t* cszFileName;
    wchar_t chQuote;
    UINT_PTR nBytesToAllocate;
    UINT_PTR nLen;

    chQuote = getQuoteChar(pFindContext);

    cszTextFormat = qsearchGetStringW(QS_STRID_FINDALL_SEARCHINGFOR);

    cszFileName = (const wchar_t *) pFindContext->pFrame->ei.pFile;
    if ( cszFileName )
        cszFileName = getFileNameW(cszFileName);
    else
        cszFileName = L"";

    nBytesToAllocate = lstrlenW(cszTextFormat);
    nBytesToAllocate += lstrlenW(pFindContext->cszFindWhat);
    nBytesToAllocate += lstrlenW(cszFileName);
    nBytesToAllocate += 1; // 12;
    nBytesToAllocate *= sizeof(wchar_t);

    if ( !tDynamicBuffer_Allocate(pTempBuf, nBytesToAllocate) )
        return 0; // failed to allocate the memory

    pszText = (wchar_t *) pTempBuf->ptr;
    nLen = (UINT_PTR) wsprintfW(pszText, cszTextFormat, chQuote, pFindContext->cszFindWhat, chQuote, cszFileName, pFindContext->nOccurrences );
    pTempBuf->nBytesStored = nLen*sizeof(wchar_t);
    return nLen;
}

static UINT_PTR formatAllFilesSearchingForStringToBuf(tFindAllContext* pFindContext, tDynamicBuffer* pTempBuf)
{
    wchar_t* pszText;
    const wchar_t* cszTextFormat;
    wchar_t chQuote;
    UINT_PTR nBytesToAllocate;
    UINT_PTR nLen;

    chQuote = getQuoteChar(pFindContext);

    cszTextFormat = qsearchGetStringW(QS_STRID_FINDALL_SEARCHINGFORINFILES);

    nBytesToAllocate = lstrlenW(cszTextFormat);
    nBytesToAllocate += lstrlenW(pFindContext->cszFindWhat);
    nBytesToAllocate += 10;
    nBytesToAllocate *= sizeof(wchar_t);

    if ( !tDynamicBuffer_Allocate(pTempBuf, nBytesToAllocate) )
        return 0; // failed to allocate the memory

    pszText = (wchar_t *) pTempBuf->ptr;
    nLen = (UINT_PTR) wsprintfW(pszText, cszTextFormat, chQuote, pFindContext->cszFindWhat, chQuote, pFindContext->nTotalFiles);
    pTempBuf->nBytesStored = nLen*sizeof(wchar_t);
    return nLen;
}

static void initLogOutput(DWORD dwFindAllResult)
{
    DLLECLOG_OUTPUT_1 loParams;
    wchar_t szCoderAlias[MAX_CODERALIAS + 1];

    if ( g_bHighlightPlugin )
    {
        if ( dwFindAllResult & QS_FINDALL_RSLT_CODERALIAS )
            GetCoderAliasW(szCoderAlias);
        else
            lstrcpyW(szCoderAlias, L".qsfndall_tolog");
    }
    else
    {
        szCoderAlias[0] = 0;
    }

    loParams.dwStructSize = sizeof(DLLECLOG_OUTPUT_1);
    loParams.nAction = 1;
    loParams.pszProgram = NULL;
    loParams.pszWorkDir = NULL;
    if ( dwFindAllResult & QS_FINDALL_RSLT_ALLFILES )
    {
        loParams.pszRePattern = QS_FINDALL_REPATTERN_ALLFILES; // corresponds to the output string format
        loParams.pszReTags = QS_FINDALL_RETAGS_ALLFILES;
    }
    else
    {
        loParams.pszRePattern = QS_FINDALL_REPATTERN_SINGLEFILE; // corresponds to the output string format
        loParams.pszReTags = QS_FINDALL_RETAGS_SINGLEFILE;
    }
    loParams.nInputCodepage = -2;
    loParams.nOutputCodepage = -2;
    loParams.nFlags = 2; // 2 = no input line
    loParams.pszAlias = szCoderAlias[0] ? szCoderAlias : NULL;

    CallLogOutput( &loParams );
}

// funcs

enum eHighlightConditionFlags {
    QHC_CHECKBOX_CHECKED = 0x0000,
    QHC_IGNORE_CHECKBOX  = 0x0001,
    QHC_IGNORE_SELECTION = 0x0010,
    QHC_FORCE_HIGHLIGHT  = 0x0100,
    QHC_DONT_CUT_REGEXP  = 0x0200,
    QHC_FINDFIRST        = 0x1000
};
void qsearchDoTryHighlightAll(HWND hDlg, const wchar_t* cszFindWhatAW, const DWORD dwOptFlags[], DWORD dwHighlightConditionFlags);
void qsearchDoTryUnhighlightAll(void);

static void scrollEditToPositionAndHighlightTheMatches(HWND hWndEdit, INT_PTR nPos, DWORD dwFrpHighlight, const tFindAllContext* pFindContext)
{
    int nFirstVisibleLine;
    AECHARINDEX ci;

    SendMessageW( hWndEdit, AEM_RICHOFFSETTOINDEX, (WPARAM) nPos, (LPARAM) &ci );
    SendMessageW( hWndEdit, AEM_EXSETSEL, (WPARAM) &ci, (LPARAM) &ci );
    nFirstVisibleLine = (int) SendMessageW( hWndEdit, AEM_GETLINENUMBER, AEGL_FIRSTFULLVISIBLELINE, 0 );
    if ( ci.nLine > nFirstVisibleLine )
        SendMessageW( hWndEdit, AEM_LINESCROLL, AESB_VERT | AESB_ALIGNTOP, (LPARAM) (ci.nLine - nFirstVisibleLine) );

    if ( dwFrpHighlight != QSFRH_NONE )
    {
        BOOL bContinue = TRUE;
        AEFINDTEXTW aeftW;

        if ( ((pFindContext->dwFindAllResult & QS_FINDALL_RSLT_FILTERMODE) == 0) &&
             ((pFindContext->dwFindAllResult & QS_FINDALL_RSLT_SEARCHING) != 0) )
        {
            wchar_t szQuoteTextW[2];

            szQuoteTextW[0] = getQuoteChar(pFindContext);
            szQuoteTextW[1] = 0;

            // searching for a quote character, don't use AEFR_WHOLEWORD and AEFR_REGEXP!
            aeftW.dwFlags = AEFR_DOWN;
            aeftW.pText = szQuoteTextW;
            aeftW.dwTextLen = 1;
            aeftW.nNewLine = pFindContext->pFindTextW->nNewLine;
            x_mem_cpy( &aeftW.crSearch.ciMin, &ci, sizeof(AECHARINDEX) );
            SendMessageW( hWndEdit, AEM_GETINDEX, AEGI_LASTCHAR, (LPARAM) &aeftW.crSearch.ciMax);
            if ( SendMessageW(hWndEdit, AEM_FINDTEXTW, 0, (LPARAM) &aeftW) )
                x_mem_cpy( &aeftW.crSearch.ciMin, &aeftW.crFound.ciMax, sizeof(AECHARINDEX) );
            else
                bContinue = FALSE;
        }
        else
        {
            x_mem_cpy( &aeftW.crSearch.ciMin, &ci, sizeof(AECHARINDEX) );
            SendMessageW( hWndEdit, AEM_GETINDEX, AEGI_LASTCHAR, (LPARAM) &aeftW.crSearch.ciMax);
        }

        if ( bContinue )
        {
            // searching for a plain text or RegExp's text, don't use AEFR_WHOLEWORD and AEFR_REGEXP!
            aeftW.dwFlags = AEFR_DOWN;
            aeftW.pText = pFindContext->cszFindWhat;
            aeftW.dwTextLen = lstrlenW(pFindContext->cszFindWhat);
            aeftW.nNewLine = pFindContext->pFindTextW->nNewLine;
            SendMessageW( hWndEdit, AEM_GETINDEX, AEGI_LASTCHAR, (LPARAM) &aeftW.crSearch.ciMax);

            if ( SendMessageW(hWndEdit, AEM_FINDTEXTW, 0, (LPARAM) &aeftW) )
            {
                // Select the text
                SendMessageW( hWndEdit, AEM_EXSETSEL, (WPARAM) &aeftW.crFound.ciMin, (LPARAM) &aeftW.crFound.ciMax );

                if ( (dwFrpHighlight == QSFRH_IFCHECKED) ||
                     (dwFrpHighlight == QSFRH_ALWAYS) )
                {
                    // Highlight All
                    if ( (dwFrpHighlight == QSFRH_ALWAYS) ||
                         g_Options.dwFlags[OPTF_SRCH_HIGHLIGHTALL] )
                    {
                        // Highlighting doesn't work without this:
                        SendMessageW( hWndEdit, WM_PAINT, 0, 0 );

                        // Actual highlighting:
                        qsearchDoTryHighlightAll(g_QSearchDlg.hDlg, g_QSearchDlg.szFindTextAW, g_Options.dwFlags,
                            QHC_IGNORE_CHECKBOX | QHC_DONT_CUT_REGEXP | QHC_FORCE_HIGHLIGHT);
                    }
                }
            }
        }
    }
}

static void qsShowFindResults_LogOutput_Init(tFindAllContext* pFindContext, tDynamicBuffer* pTempBuf)
{
    if ( (pFindContext->dwFindAllResult & QS_FINDALL_RSLT_ALLFILES) == 0 )
    {
        HWND hWndEdit;
        INT_PTR nStartPos;

        initLogOutput(pFindContext->dwFindAllResult);

        hWndEdit = LogOutput_GetEditHwnd();
        nStartPos = SendMessageW(g_Plugin.hMainWnd, AKD_GETTEXTLENGTH, (WPARAM) hWndEdit, 0);

        if ( nStartPos != 0 )
            tDynamicBuffer_Append( &pFindContext->ResultsBuf, L"\r", 1*sizeof(wchar_t) ); // new line
    }
}

static void qsShowFindResults_LogOutput_AddOccurrence(tFindAllContext* pFindContext, const tDynamicBuffer* pOccurrence)
{
    tDynamicBuffer_Append( &pFindContext->OccurrencesBuf, pOccurrence->ptr, pOccurrence->nBytesStored );
    tDynamicBuffer_Append( &pFindContext->OccurrencesBuf, L"\r", 1*sizeof(wchar_t) ); // new line
}

static void qsShowFindResults_LogOutput_Done(tFindAllContext* pFindContext, tDynamicBuffer* pTempBuf)
{
    if ( (pFindContext->nOccurrences != 0) ||
         ((pFindContext->dwFindAllResult & QS_FINDALL_RSLT_ALLFILES) == 0) )
    {
        if ( (pFindContext->dwFindAllResult & QS_FINDALL_RSLT_FILTERMODE) == 0 )
        {
            if ( pFindContext->dwFindAllResult & QS_FINDALL_RSLT_SEARCHING )
            {
                if ( formatSearchingForStringToBuf(pFindContext, pTempBuf) )
                {
                    tDynamicBuffer_Append( &pFindContext->ResultsBuf, pTempBuf->ptr, pTempBuf->nBytesStored );
                    tDynamicBuffer_Append( &pFindContext->ResultsBuf, L"\r", 1*sizeof(wchar_t) ); // new line
                }
            }
        }

        tDynamicBuffer_Append( &pFindContext->ResultsBuf, pFindContext->OccurrencesBuf.ptr, pFindContext->OccurrencesBuf.nBytesStored );
    }

    tDynamicBuffer_Clear( &pFindContext->OccurrencesBuf );

    if ( (pFindContext->dwFindAllResult & QS_FINDALL_RSLT_ALLFILES) == 0 )
    {
        HWND hWndEdit;
        INT_PTR nStartPos;
        UINT_PTR nLen;

        hWndEdit = LogOutput_GetEditHwnd();
        nStartPos = SendMessageW(g_Plugin.hMainWnd, AKD_GETTEXTLENGTH, (WPARAM) hWndEdit, 0);
        if ( nStartPos != 0 )
            ++nStartPos; // new line was added in LogOutput_Init

        nLen = pFindContext->ResultsBuf.nBytesStored/sizeof(wchar_t); // without the trailing '\0'
        tDynamicBuffer_Append( &pFindContext->ResultsBuf, L"\0", 1*sizeof(wchar_t) ); // the trailing '\0'
        LogOutput_AddText( (const wchar_t*) pFindContext->ResultsBuf.ptr, nLen );

        scrollEditToPositionAndHighlightTheMatches(hWndEdit, nStartPos, g_Options.LogOutputFRP.nHighlight, pFindContext);
    }

    if ( (pFindContext->dwFindAllResult & QS_FINDALL_RSLT_ALLFILES) == 0 )
    {
        qsSetInfoOccurrencesFound_Tracking(pFindContext->nOccurrences, 0, "qsShowFindResults_LogOutput_Done");
    }
}

static void qsShowFindResults_LogOutput_AllFiles_Init(tFindAllContext* pFindContext, tDynamicBuffer* pTempBuf)
{
    HWND hWndEdit;
    INT_PTR nStartPos;

    initLogOutput(pFindContext->dwFindAllResult);

    hWndEdit = LogOutput_GetEditHwnd();
    nStartPos = SendMessageW(g_Plugin.hMainWnd, AKD_GETTEXTLENGTH, (WPARAM) hWndEdit, 0);

    if ( nStartPos != 0 )
        tDynamicBuffer_Append( &pFindContext->ResultsBuf, L"\r", 1*sizeof(wchar_t) ); // new line

    if ( (pFindContext->dwFindAllResult & QS_FINDALL_RSLT_FILTERMODE) == 0 )
    {
        if ( formatAllFilesSearchingForStringToBuf(pFindContext, pTempBuf) )
        {
            tDynamicBuffer_Append( &pFindContext->ResultsBuf, pTempBuf->ptr, pTempBuf->nBytesStored );
            tDynamicBuffer_Append( &pFindContext->ResultsBuf, L"\r", 1*sizeof(wchar_t) ); // new line
        }
    }
}

static void qsShowFindResults_LogOutput_AllFiles_Done(tFindAllContext* pFindContext, tDynamicBuffer* pTempBuf)
{
    HWND hWndEdit;
    INT_PTR nStartPos;
    const wchar_t* cszTextFormat;
    UINT_PTR nLen;
    wchar_t szText[128];

    hWndEdit = LogOutput_GetEditHwnd();
    nStartPos = SendMessageW(g_Plugin.hMainWnd, AKD_GETTEXTLENGTH, (WPARAM) hWndEdit, 0);
    if ( nStartPos != 0 )
        ++nStartPos; // new line was added in LogOutput_AllFiles_Init

    if ( (pFindContext->dwFindAllResult & QS_FINDALL_RSLT_FILTERMODE) == 0 )
    {
        cszTextFormat = qsearchGetStringW(QS_STRID_FINDALL_OCCURRENCESFOUNDINFILES);
        nLen = (UINT_PTR) wsprintfW(szText, cszTextFormat, pFindContext->nTotalOccurrences, pFindContext->nFilesWithOccurrences);
        tDynamicBuffer_Append( &pFindContext->ResultsBuf, szText, nLen*sizeof(wchar_t) );
        tDynamicBuffer_Append( &pFindContext->ResultsBuf, L"\r", 1*sizeof(wchar_t) ); // new line
    }

    nLen = pFindContext->ResultsBuf.nBytesStored/sizeof(wchar_t); // without the trailing '\0'
    tDynamicBuffer_Append( &pFindContext->ResultsBuf, L"\0", 1*sizeof(wchar_t) ); // the trailing '\0'
    LogOutput_AddText( (const wchar_t*) pFindContext->ResultsBuf.ptr, nLen );

    scrollEditToPositionAndHighlightTheMatches(hWndEdit, nStartPos, g_Options.LogOutputFRP.nHighlight, pFindContext);
}

// FileOutput...
static void qsShowFindResults_FileOutput_Init(tFindAllContext* pFindContext, tDynamicBuffer* pTempBuf)
{
}

static void qsShowFindResults_FileOutput_AddOccurrence(tFindAllContext* pFindContext, const tDynamicBuffer* pOccurrence)
{
    tDynamicBuffer_Append( &pFindContext->OccurrencesBuf, pOccurrence->ptr, pOccurrence->nBytesStored );
    tDynamicBuffer_Append( &pFindContext->OccurrencesBuf, L"\r", 1*sizeof(wchar_t) ); // new line
}

static void addResultsToFileOutput(tFindAllContext* pFindContext)
{
    const FRAMEDATA* pSearchResultsFrame;
    HWND hMainWnd;
    HWND hWndEdit;
    BOOL bSingleFileOutput;
    BOOL bOutputResult;
    BOOL bNewWindow;
    wchar_t szCoderAlias[MAX_CODERALIAS + 1];

    if ( g_bHighlightPlugin )
    {
        if ( pFindContext->dwFindAllResult & QS_FINDALL_RSLT_CODERALIAS )
            GetCoderAliasW(szCoderAlias);
        else
            lstrcpyW(szCoderAlias, L".qsfndall_tofile");
    }
    else
    {
        szCoderAlias[0] = 0;
    }

    pSearchResultsFrame = NULL;
    hMainWnd = g_Plugin.hMainWnd;
    hWndEdit = NULL;
    bSingleFileOutput = ((pFindContext->dwFindAllMode & QS_FINDALL_MASK) == QS_FINDALL_FILEOUTPUT_SNGL) ? TRUE : FALSE;
    bOutputResult = FALSE;
    bNewWindow = FALSE;

    if ( bSingleFileOutput && (g_Plugin.nMDI == WMD_SDI) )
    {
        bOutputResult = TRUE;
        // in this mode, we are appending the search results to the current document
        g_QSearchDlg.nGoToNextFindAllPosToCompare = (INT_PTR) SendMessageW(pFindContext->pFrame->ei.hWndEdit, AEM_GETRICHOFFSET, AEGI_LASTCHAR, 0);
    }
    else if ( bSingleFileOutput && (g_QSearchDlg.nResultsItemsCount != 0) &&
              SendMessageW(g_Plugin.hMainWnd, AKD_FRAMEISVALID, 0, (LPARAM) QSearchDlgState_GetSearchResultsFrame(&g_QSearchDlg)) )
    {
        tQSSearchResultsItem* pItem;

        pItem = QSearchDlgState_GetSearchResultsItem(&g_QSearchDlg);
        pSearchResultsFrame = pItem->pFrame;
        // updating the existing item:
        tQSSearchResultsItem_Assign(pItem, pSearchResultsFrame, pFindContext->cszFindWhat, pFindContext->dwFindAllFlags);

        if ( g_Plugin.nMDI != WMD_SDI )
            qs_bEditCanBeNonActive = TRUE;

        SendMessageW(g_Plugin.hMainWnd, AKD_FRAMEACTIVATE, 0, (LPARAM) pSearchResultsFrame);
        bOutputResult = TRUE;
    }
    else if ( ((pFindContext->dwFindAllMode & QS_FINDALL_MASK) == QS_FINDALL_FILEOUTPUT_MULT) &&
              (g_Plugin.nMDI == WMD_SDI) )
    {
        BOOL bOptionsWritten;
        tDynamicBuffer oldCmdLineEnd;
        tDynamicBuffer newCmdLineEnd;

        bOptionsWritten = FALSE;
        tDynamicBuffer_Init(&oldCmdLineEnd);
        tDynamicBuffer_Init(&newCmdLineEnd);

        if ( g_bHighlightPlugin )
        {
            UINT_PTR nOldLen;
            UINT_PTR nNewLen;

            nOldLen = (UINT_PTR) SendMessageW(g_Plugin.hMainWnd, AKD_GETMAININFO, MI_CMDLINEEND, 0);
            if ( tDynamicBuffer_Allocate(&oldCmdLineEnd, (nOldLen + 1)*sizeof(wchar_t)) )
            {
                const wchar_t* const cszCmdFmt = L"/Call(\"Coder::Settings\", 1, \"%s\") /Call(\"Coder::HighLight\", 2, \"0\", \"#%02X%02X%02X\", %u, 0, %u, %c%s%c, %d) ";

                nOldLen = (UINT_PTR) SendMessageW(g_Plugin.hMainWnd, AKD_GETMAININFO, MI_CMDLINEEND, (LPARAM) oldCmdLineEnd.ptr);
                oldCmdLineEnd.nBytesStored = nOldLen*sizeof(wchar_t);
                tDynamicBuffer_Append(&oldCmdLineEnd, L"\0", 1*sizeof(wchar_t)); // adding the trailing '\0'

                nNewLen = nOldLen + lstrlenW(cszCmdFmt) + lstrlenW(szCoderAlias) + pFindContext->pFindTextW->dwTextLen + 2;

                if ( tDynamicBuffer_Allocate(&newCmdLineEnd, nNewLen*sizeof(wchar_t)) )
                {
                    HANDLE hOptions;
                    unsigned int flags;
                    unsigned int id;
                    wchar_t q;
                    BYTE r, g, b;
                    PLUGINOPTIONW poW;

                    flags = 0;
                    if ( pFindContext->pFindTextW->dwFlags & AEFR_MATCHCASE )
                        flags |= 1;
                    if ( pFindContext->pFindTextW->dwFlags & AEFR_REGEXP )
                        flags |= 2;
                    if ( pFindContext->pFindTextW->dwFlags & AEFR_WHOLEWORD )
                        flags |= 4;

                    q = L'\"';
                    if ( x_wstr_findch(pFindContext->pFindTextW->pText, L'\"', 0) != -1 )
                    {
                        if ( x_wstr_findch(pFindContext->pFindTextW->pText, L'`', 0) == -1 )
                            q = L'`';
                        else if ( x_wstr_findch(pFindContext->pFindTextW->pText, L'\'', 0) == -1 )
                            q = L'\'';
                    }

                    id = g_Options.dwHighlightMarkID;
                    r = GetRValue(g_Options.colorHighlight);
                    g = GetGValue(g_Options.colorHighlight);
                    b = GetBValue(g_Options.colorHighlight);

                    nNewLen = wsprintfW( (LPWSTR) newCmdLineEnd.ptr, cszCmdFmt,
                        szCoderAlias, r, g, b, flags, id, q, pFindContext->pFindTextW->pText, q, pFindContext->pFindTextW->dwTextLen );
                    newCmdLineEnd.nBytesStored = nNewLen*sizeof(wchar_t);
                    tDynamicBuffer_Append(&newCmdLineEnd, oldCmdLineEnd.ptr, oldCmdLineEnd.nBytesStored);

                    if ( hOptions = (HANDLE) SendMessageW(g_Plugin.hMainWnd, AKD_BEGINOPTIONS, POB_SAVE, 0) )
                    {
                        poW.pOptionName = L"CmdLineEnd";
                        poW.dwType = PO_STRING;
                        poW.lpData = (BYTE *) newCmdLineEnd.ptr;
                        poW.dwData = (DWORD) newCmdLineEnd.nBytesStored;
                        SendMessageW( g_Plugin.hMainWnd, AKD_OPTION, (WPARAM) hOptions, (LPARAM) &poW );
                        SendMessageW( g_Plugin.hMainWnd, AKD_ENDOPTIONS, (WPARAM) hOptions, 0 );
                        bOptionsWritten = TRUE;
                    }
                }
            }
        }

        hMainWnd = (HWND) SendMessageW(g_Plugin.hMainWnd, WM_COMMAND, IDM_FILE_CREATENEW, 0); // creates a new SDI window
        hWndEdit = (HWND) SendMessageW(hMainWnd, AKD_GETFRAMEINFO, FI_WNDEDIT, 0);

        if ( g_bHighlightPlugin && bOptionsWritten )
        {
            HANDLE hOptions;
            PLUGINOPTIONW poW;

            if ( hOptions = (HANDLE) SendMessageW(g_Plugin.hMainWnd, AKD_BEGINOPTIONS, POB_SAVE, 0) )
            {
                poW.pOptionName = L"CmdLineEnd";
                poW.dwType = PO_STRING;
                poW.lpData = (BYTE *) oldCmdLineEnd.ptr;
                poW.dwData = (DWORD) oldCmdLineEnd.nBytesStored;
                SendMessageW( g_Plugin.hMainWnd, AKD_OPTION, (WPARAM) hOptions, (LPARAM) &poW );
                SendMessageW( g_Plugin.hMainWnd, AKD_ENDOPTIONS, (WPARAM) hOptions, 0 );
            }
        }

        tDynamicBuffer_Free(&oldCmdLineEnd);
        tDynamicBuffer_Free(&newCmdLineEnd);

        bOutputResult = TRUE;
        bNewWindow = TRUE;
    }
    else
    {
        if ( g_Plugin.nMDI != WMD_SDI )
            qs_bEditCanBeNonActive = TRUE;

        if ( SendMessageW(g_Plugin.hMainWnd, WM_COMMAND, IDM_FILE_NEW, 0) == TRUE )
        {
            bOutputResult = TRUE;
        }
    }

    if ( bOutputResult )
    {
        if ( pSearchResultsFrame == NULL && !bNewWindow )
        {
            pSearchResultsFrame = (FRAMEDATA *) SendMessageW( hMainWnd, AKD_FRAMEFIND, FWF_CURRENT, 0 );
            if ( g_Plugin.nMDI == WMD_SDI )
            {
                g_QSearchDlg.nResultsItemsCount = 0; // only one in SDI
            }
            QSearchDlgState_AddResultsFrame(&g_QSearchDlg, pSearchResultsFrame, pFindContext->cszFindWhat, pFindContext->dwFindAllFlags);
        }

        if ( bNewWindow && hWndEdit )
        {
            // This successfully works with AkelEdit control that belongs to another process
            SendMessageW(hWndEdit, WM_SETTEXT, 0, (LPARAM) pFindContext->ResultsBuf.ptr);
        }
        else if ( (hWndEdit = GetWndEdit(hMainWnd)) != NULL )
        {
            INT_PTR nStartPos;
            AEAPPENDTEXTW aeatW;

            if ( szCoderAlias[0] )
            {
                SetCoderAliasW(szCoderAlias);
            }

            nStartPos = SendMessageW(hMainWnd, AKD_GETTEXTLENGTH, (WPARAM) hWndEdit, 0);

            if ( bSingleFileOutput )
            {
                SendMessageW( hWndEdit, AEM_BEGINUNDOACTION, 0, 0 );

                if ( nStartPos != 0 )
                {
                    aeatW.pText = L"\r"; // new line
                    aeatW.dwTextLen = (UINT_PTR) (-1);
                    aeatW.nNewLine = AELB_ASINPUT;
                    SendMessageW( hWndEdit, AEM_APPENDTEXTW, 0, (LPARAM) &aeatW );
                    nStartPos += 1; // the new line above
                }
            }

            /* SendMessageW( ei.hWndEdit, EM_REPLACESEL, FALSE, (LPARAM) pFindContext->ResultsBuf.ptr ); */
            aeatW.pText = (const wchar_t *) pFindContext->ResultsBuf.ptr;
            aeatW.dwTextLen = pFindContext->ResultsBuf.nBytesStored/sizeof(wchar_t) - 1; // excluding the trailing '\0'
            aeatW.nNewLine = AELB_ASINPUT;
            SendMessageW( hWndEdit, AEM_APPENDTEXTW, 0, (LPARAM) &aeatW );

            if ( bSingleFileOutput )
            {
                SendMessageW( hWndEdit, AEM_ENDUNDOACTION, 0, 0 );
            }

            scrollEditToPositionAndHighlightTheMatches(hWndEdit, nStartPos, g_Options.FileOutputFRP.nHighlight, pFindContext);
        }
    }
}

static void qsShowFindResults_FileOutput_Done(tFindAllContext* pFindContext, tDynamicBuffer* pTempBuf)
{
    if ( (pFindContext->nOccurrences != 0) ||
         ((pFindContext->dwFindAllResult & QS_FINDALL_RSLT_ALLFILES) == 0) )
    {
        if ( (pFindContext->dwFindAllResult & QS_FINDALL_RSLT_FILTERMODE) == 0 )
        {
            if ( pFindContext->dwFindAllResult & QS_FINDALL_RSLT_SEARCHING )
            {
                if ( formatSearchingForStringToBuf(pFindContext, pTempBuf) )
                {
                    tDynamicBuffer_Append( &pFindContext->ResultsBuf, pTempBuf->ptr, pTempBuf->nBytesStored );
                    tDynamicBuffer_Append( &pFindContext->ResultsBuf, L"\r", 1*sizeof(wchar_t) ); // new line
                }
            }
        }

        tDynamicBuffer_Append( &pFindContext->ResultsBuf, pFindContext->OccurrencesBuf.ptr, pFindContext->OccurrencesBuf.nBytesStored );
    }

    tDynamicBuffer_Clear( &pFindContext->OccurrencesBuf );

    if ( (pFindContext->dwFindAllResult & QS_FINDALL_RSLT_ALLFILES) == 0 )
    {
        tDynamicBuffer_Append( &pFindContext->ResultsBuf, L"\0", 1*sizeof(wchar_t) ); // the trailing '\0'
        addResultsToFileOutput( pFindContext );
        qsSetInfoOccurrencesFound_Tracking(pFindContext->nOccurrences, 0, "qsShowFindResults_FileOutput_Done");
    }
}

static void qsShowFindResults_FileOutput_AllFiles_Init(tFindAllContext* pFindContext, tDynamicBuffer* pTempBuf)
{
    if ( (pFindContext->dwFindAllResult & QS_FINDALL_RSLT_FILTERMODE) == 0 )
    {
        if ( formatAllFilesSearchingForStringToBuf(pFindContext, pTempBuf) )
        {
            tDynamicBuffer_Append( &pFindContext->ResultsBuf, pTempBuf->ptr, pTempBuf->nBytesStored );
            tDynamicBuffer_Append( &pFindContext->ResultsBuf, L"\r", 1*sizeof(wchar_t) ); // new line
        }
    }
}

static void qsShowFindResults_FileOutput_AllFiles_Done(tFindAllContext* pFindContext, tDynamicBuffer* pTempBuf)
{
    const wchar_t* cszTextFormat;
    UINT_PTR nLen;
    wchar_t  szText[128];

    if ( (pFindContext->dwFindAllResult & QS_FINDALL_RSLT_FILTERMODE) == 0 )
    {
        cszTextFormat = qsearchGetStringW(QS_STRID_FINDALL_OCCURRENCESFOUNDINFILES);
        nLen = (UINT_PTR) wsprintfW(szText, cszTextFormat, pFindContext->nTotalOccurrences, pFindContext->nFilesWithOccurrences);
        tDynamicBuffer_Append( &pFindContext->ResultsBuf, szText, nLen*sizeof(wchar_t) );
        tDynamicBuffer_Append( &pFindContext->ResultsBuf, L"\r", 1*sizeof(wchar_t) ); // new line
    }

    tDynamicBuffer_Append( &pFindContext->ResultsBuf, L"\0", 1*sizeof(wchar_t) ); // the trailing '\0'
    addResultsToFileOutput( pFindContext );
}

typedef struct sShowFindResults {
    tShowFindResults_Init pfnInit;                   // can't be NULL
    tShowFindResults_AddOccurrence pfnAddOccurrence; // can't be NULL
    tShowFindResults_Done pfnDone;                   // can't be NULL
    tShowFindResults_AllFiles_Init pfnAllFilesInit;  // can't be NULL
    tShowFindResults_AllFiles_Done pfnAllFilesDone;  // can't be NULL
} tShowFindResults;

typedef void (*tStoreResultCallback)(tFindAllContext* pFindContext, const AECHARRANGE* pcrFound, const int nLinesBeforeAfter[2], const tDynamicBuffer* pFindResult, tDynamicBuffer* pTempBuf, tShowFindResults_AddOccurrence pfnAddOccurrence);

static inline int getUnwrappedLine(HWND hWndEdit, BOOL bWordWrap, int nLine)
{
    return bWordWrap ? (int) SendMessage(hWndEdit, AEM_GETUNWRAPLINE, nLine, 0) : nLine;
}

static void formatFindResult(const tFindAllContext* pFindContext, const AECHARRANGE* pcrFound, const int nLinesBeforeAfter[2], const tDynamicBuffer* pFindResult, tDynamicBuffer* pTempBuf)
{
    pTempBuf->nBytesStored = 0;

    if ( (pFindContext->dwFindAllResult & QS_FINDALL_RSLT_FILTERMODE) == 0 )
    {
        HWND hWndEdit;
        BOOL bAddFramePtr;
        int nUnwrappedLine;
        int nPrefixLen;
        wchar_t szPrefixBuf[128];

        hWndEdit = pFindContext->pFrame->ei.hWndEdit;
        bAddFramePtr = FALSE;
        nUnwrappedLine = 0;
        nPrefixLen = 0;
        szPrefixBuf[0] = 0;

        if ( pFindContext->dwFindAllResult & QS_FINDALL_RSLT_SEARCHING )
        {
            szPrefixBuf[nPrefixLen++] = L' ';
        }

        if ( pFindContext->dwFindAllResult & QS_FINDALL_RSLT_POS )
        {
            INT_X nLinePos;
            AECHARINDEX ci;

            szPrefixBuf[nPrefixLen++] = L'(';
            if ( pFindContext->dwFindAllResult & QS_FINDALL_RSLT_ALLFILES )
            {
                bAddFramePtr = TRUE;
            }
            else if ( g_Plugin.nMDI != WMD_SDI )
            {
                DWORD dwFindAllMode = (pFindContext->dwFindAllMode & QS_FINDALL_MASK);
                if ( (dwFindAllMode == QS_FINDALL_FILEOUTPUT_SNGL) ||
                     (dwFindAllMode == QS_FINDALL_FILEOUTPUT_MULT) )
                {
                    bAddFramePtr = TRUE;
                }
            }

            if ( bAddFramePtr )
            {
                nPrefixLen += xitoaW( (INT_PTR) pFindContext->pFrame, &szPrefixBuf[nPrefixLen] );
                szPrefixBuf[nPrefixLen++] = L' ';
            }

            nUnwrappedLine = getUnwrappedLine(hWndEdit, pFindContext->bWordWrap, pcrFound->ciMin.nLine);

            x_mem_cpy(&ci, &pcrFound->ciMin, sizeof(AECHARINDEX));
            nLinePos = AEC_WrapLineBegin(&ci);

            nPrefixLen += xitoaW( nUnwrappedLine + 1, &szPrefixBuf[nPrefixLen] );
            szPrefixBuf[nPrefixLen++] = L':';
            nPrefixLen += xitoaW( nLinePos + 1, &szPrefixBuf[nPrefixLen] );
            szPrefixBuf[nPrefixLen++] = L')';
            szPrefixBuf[nPrefixLen++] = L'\t';
        }

        if ( pFindContext->dwFindAllResult & QS_FINDALL_RSLT_LEN )
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

            if ( pFindContext->dwFindAllResult & QS_FINDALL_RSLT_POS )
            {
                nPrefixLen -= 1; // exclude the trailing '\t'
            }

            szPrefixBuf[nPrefixLen++] = L'(';
            nPrefixLen += xitoaW( nLen, &szPrefixBuf[nPrefixLen] );
            szPrefixBuf[nPrefixLen++] = L')';
            szPrefixBuf[nPrefixLen++] = L'\t';
        }

        szPrefixBuf[nPrefixLen] = 0; // the trailing '\0'

        if ( (nPrefixLen != 0) && (nLinesBeforeAfter[0] != 0 || nLinesBeforeAfter[1] != 0) )
        {
            int n1;
            int n2;
            int n;
            int i;
            wchar_t szPrefixBuf2[128];

            tDynamicBuffer_Allocate( pTempBuf, sizeof(wchar_t)*(nPrefixLen + 1)*(1 + nLinesBeforeAfter[0] + nLinesBeforeAfter[1]) + pFindResult->nBytesStored );

            n = -nLinesBeforeAfter[0];
            n1 = 0;
            n2 = x_wstr_findch( (const wchar_t *) pFindResult->ptr, L'\r', 0 );
            for ( ; ; )
            {
                if ( n == 0 )
                {
                    tDynamicBuffer_Append(pTempBuf, szPrefixBuf, sizeof(wchar_t)*nPrefixLen);
                }
                else
                {
                    i = -1;
                    lstrcpyW(szPrefixBuf2, szPrefixBuf);
                    if ( pFindContext->dwFindAllResult & QS_FINDALL_RSLT_POS )
                    {
                        i = x_wstr_findch(szPrefixBuf2, L'(', 0);
                        if ( i != -1 )
                        {
                            // szPrefixBuf2[i] = (n < 0) ? L'\x2193' : L'\x2191';
                            if ( bAddFramePtr )
                            {
                                // i = x_wstr_findch(szPrefixBuf2, L' ', i + 1);
                                ++i;
                                for ( ; szPrefixBuf2[i] != L' '; ++i )
                                {
                                    szPrefixBuf2[i] = L' ';
                                }
                            }
                        }
                        if ( i != -1 )
                        {
                            ++i;
                            i += xitoaW(nUnwrappedLine + n + 1, &szPrefixBuf2[i]);
                            for ( ; szPrefixBuf2[i] != L')'; ++i )
                            {
                                szPrefixBuf2[i] = L' ';
                            }
                            // szPrefixBuf2[i] = (n < 0) ? L'\x2193' : L'\x2191';
                        }
                    }
                    if ( pFindContext->dwFindAllResult & QS_FINDALL_RSLT_LEN )
                    {
                        i = x_wstr_findch(szPrefixBuf2, L'(', (i == -1) ? 0 : i);
                        if ( i != -1 )
                        {
                            for ( ; szPrefixBuf2[i] != L'\t'; ++i )
                            {
                                szPrefixBuf2[i] = L' ';
                            }
                        }
                    }
                    tDynamicBuffer_Append(pTempBuf, szPrefixBuf2, sizeof(wchar_t)*nPrefixLen);
                }
                if ( n2 != -1 )
                {
                    tDynamicBuffer_Append( pTempBuf, ((const wchar_t *) pFindResult->ptr) + n1, sizeof(wchar_t)*(n2 - n1) );
                    tDynamicBuffer_Append( pTempBuf, L"\r", sizeof(wchar_t) );
                    if ( ++n <= nLinesBeforeAfter[1] )
                    {
                        n1 = n2 + 1;
                        n2 = x_wstr_findch( (const wchar_t *) pFindResult->ptr, L'\r', n1 );
                    }
                    else
                        break;
                }
                else
                {
                    tDynamicBuffer_Append( pTempBuf, ((const wchar_t *) pFindResult->ptr) + n1, pFindResult->nBytesStored - sizeof(wchar_t)*n1 );
                    break;
                }
            }
        }
        else
        {
            tDynamicBuffer_Allocate(pTempBuf, sizeof(wchar_t)*nPrefixLen + pFindResult->nBytesStored);
            if ( nPrefixLen != 0 )
            {
                tDynamicBuffer_Append(pTempBuf, szPrefixBuf, sizeof(wchar_t)*nPrefixLen);
            }
            tDynamicBuffer_Append(pTempBuf, pFindResult->ptr, pFindResult->nBytesStored);
        }
    }
    else
    {
        tDynamicBuffer_Append(pTempBuf, pFindResult->ptr, pFindResult->nBytesStored);
    }
}

static void qsStoreResultCallback(tFindAllContext* pFindContext, const AECHARRANGE* pcrFound, const int nLinesBeforeAfter[2], const tDynamicBuffer* pFindResult, tDynamicBuffer* pTempBuf, tShowFindResults_AddOccurrence pfnAddOccurrence)
{
    formatFindResult(pFindContext, pcrFound, nLinesBeforeAfter, pFindResult, pTempBuf);
    pfnAddOccurrence(pFindContext, pTempBuf);
}


typedef struct sGetFindResultPolicy {
    int nMode;       // one of QSFRM_*
    int nBefore;     // number of lines or chars before the search result
    int nAfter;      // number of lines or chars after the search result
    int nMaxLineLen; // max line length to show (0 - no limit)
    tStoreResultCallback pfnStoreResultCallback;
} tGetFindResultPolicy;
// nBefore=0 and nAfter=0 with nMode=QSFRM_LINE means: whole line
// nBefore=0 and nAfter=0 with nMode=QSFRM_CHAR means: just the matching word

typedef void (*tFindResultCallback)(tFindAllContext* pFindContext, const AECHARRANGE* pcrFound, const tGetFindResultPolicy* pfrPolicy, tDynamicBuffer* pTempBuf, tDynamicBuffer* pTempBuf2, tShowFindResults_AddOccurrence pfnAddOccurrence);

static void qsFindResultCallback(tFindAllContext* pFindContext, const AECHARRANGE* pcrFound, const tGetFindResultPolicy* pfrPolicy, tDynamicBuffer* pTempBuf, tDynamicBuffer* pTempBuf2, tShowFindResults_AddOccurrence pfnAddOccurrence)
{
    AETEXTRANGEW tr;
    HWND         hWndEdit;
    UINT_PTR     nBytesToAllocate;
    BOOL         bAddLineCR;
    int          nLinesBeforeAfter[2];

    if ( !pfrPolicy->pfnStoreResultCallback )
        return; // no sense to retrieve the find result

    hWndEdit = pFindContext->pFrame->ei.hWndEdit;

    x_zero_mem( &tr, sizeof(AETEXTRANGEW) );
    x_mem_cpy( &tr.cr, pcrFound, sizeof(AECHARRANGE) );
    nLinesBeforeAfter[0] = 0;
    nLinesBeforeAfter[1] = 0;

    bAddLineCR = FALSE;
    if ( pfrPolicy->nMode == QSFRM_LINE_CR )
    {
        if ( !(pFindContext->dwFindAllResult & QS_FINDALL_RSLT_FILTERMODE) || (pFindContext->dwFindAllResult & QS_FINDALL_RSLT_FILTERCONTEXT) )
            bAddLineCR = TRUE;
    }

    if ( (pFindContext->dwFindAllResult & QS_FINDALL_RSLT_MATCHONLY) == 0 )
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
            if ( (pFindContext->dwFindAllResult & QS_FINDALL_RSLT_FILTERMODE) && !(pFindContext->dwFindAllResult & QS_FINDALL_RSLT_FILTERCONTEXT) )
            {
                if ( tr.cr.ciMin.nLine < pcrFound->ciMin.nLine )
                {
                    x_mem_cpy( &tr.cr.ciMin, &pcrFound->ciMin, sizeof(AECHARINDEX) );
                    AEC_WrapLineBegin(&tr.cr.ciMin);
                }
                if ( tr.cr.ciMax.nLine > pcrFound->ciMax.nLine )
                {
                    x_mem_cpy( &tr.cr.ciMax, &pcrFound->ciMax, sizeof(AECHARINDEX) );
                    AEC_WrapLineEnd(&tr.cr.ciMax);
                }
            }
        }
        else if ( pfrPolicy->nMode == QSFRM_CHARINLINE )
        {
            int nBefore;
            int nAfter;
            AECHARINDEX ci;

            nBefore = pfrPolicy->nBefore;
            x_mem_cpy(&ci, &tr.cr.ciMin, sizeof(AECHARINDEX));
            while ( nBefore != 0 && AEC_PrevCharInLine(&ci) != NULL )
            {
                x_mem_cpy(&tr.cr.ciMin, &ci, sizeof(AECHARINDEX));
                --nBefore;
            }

            nAfter = pfrPolicy->nAfter;
            x_mem_cpy(&ci, &tr.cr.ciMax, sizeof(AECHARINDEX));
            while ( nAfter != 0 && AEC_NextCharInLine(&ci) != NULL )
            {
                x_mem_cpy(&tr.cr.ciMax, &ci, sizeof(AECHARINDEX));
                --nAfter;
            }
            if ( nAfter != 0 )
            {
                tr.cr.ciMax.nCharInLine = tr.cr.ciMax.lpLine->nLineLen;
            }
        }
        else // QSFRM_LINE or QSFRM_LINE_CR
        {
            if ( (pfrPolicy->nBefore == 0 && pfrPolicy->nAfter == 0) ||
                 ((pFindContext->dwFindAllResult & QS_FINDALL_RSLT_FILTERMODE) && !(pFindContext->dwFindAllResult & QS_FINDALL_RSLT_FILTERCONTEXT)) )
            {
                AEC_WrapLineBegin(&tr.cr.ciMin);
                AEC_WrapLineEnd(&tr.cr.ciMax);
            }
            else
            {
                int nLines;

                if ( pfrPolicy->nBefore >= 0 )
                {
                    int nBefore, nMinLine;

                    nBefore = pfrPolicy->nBefore;
                    for ( ; ; )
                    {
                        AEC_WrapLineBegin(&tr.cr.ciMin);
                        nMinLine = getUnwrappedLine(hWndEdit, pFindContext->bWordWrap, tr.cr.ciMin.nLine);
                        if ( nBefore == 0 || nMinLine == 0 ||
                             (pFindContext->nLastLine != -1 && nMinLine <= pFindContext->nLastLine + 1) )
                        {
                            break;
                        }

                        AEC_PrevLine(&tr.cr.ciMin);
                        --nBefore;
                    }
                    nLinesBeforeAfter[0] = pfrPolicy->nBefore - nBefore;
                }

                nLines = pFindContext->nLastLine - getUnwrappedLine(hWndEdit, pFindContext->bWordWrap, tr.cr.ciMin.nLine);
                if ( nLines > 0 || ((nLines == 0 || nLines == -1) && pFindContext->nLastLine != -1) )
                {
                    if ( bAddLineCR )
                    {
                        if ( pFindContext->OccurrencesBuf.nBytesStored != 0 )
                            pFindContext->OccurrencesBuf.nBytesStored -= 1*sizeof(wchar_t); // exclude the trailing '\r'
                    }
                }
                if ( nLines >= 0 )
                {
                    int nPos;

                    if ( pfrPolicy->nAfter > 0 )
                    {
                        if ( pFindContext->nLastOccurrenceLine < getUnwrappedLine(hWndEdit, pFindContext->bWordWrap, pcrFound->ciMin.nLine) )
                            ++nLines;
                    }

                    nPos = (int) (pFindContext->OccurrencesBuf.nBytesStored/sizeof(wchar_t)) - 1; // position of the trailing '\r'
                    for ( ; nLines > 0 && nPos >= 0; --nLines )
                    {
                        nPos = x_wstr_rfindch( (const wchar_t *) pFindContext->OccurrencesBuf.ptr, L'\r', nPos ); // searches backwards starting from (nPos - 1)
                        if ( nPos != -1 ) // position of the trailing '\r'
                            pFindContext->OccurrencesBuf.nBytesStored = (nPos + 1)*sizeof(wchar_t); // preserving the trailing '\r'
                        else
                            pFindContext->OccurrencesBuf.nBytesStored = 0;
                    }
                }

                if ( pfrPolicy->nAfter >= 0)
                {
                    int nAfter = pfrPolicy->nAfter;
                    for ( ; ; )
                    {
                        AEC_WrapLineEnd(&tr.cr.ciMax);
                        if ( nAfter == 0 || AEC_IsLastCharInFile(&tr.cr.ciMax) )
                            break;

                        AEC_NextLine(&tr.cr.ciMax);
                        --nAfter;
                    }
                    nLinesBeforeAfter[1] = pfrPolicy->nAfter - nAfter;
                }

                pFindContext->nLastLine = getUnwrappedLine(hWndEdit, pFindContext->bWordWrap, tr.cr.ciMax.nLine);
            }
        }
    }

    pFindContext->nLastOccurrenceLine = getUnwrappedLine(hWndEdit, pFindContext->bWordWrap, pcrFound->ciMin.nLine);

    tr.bColumnSel = FALSE;
    tr.pBuffer = NULL;
    tr.nNewLine = AELB_R;
    tr.bFillSpaces = FALSE;

    tr.dwBufferMax = (UINT_PTR) SendMessage( hWndEdit, AEM_GETTEXTRANGEW, 0, (LPARAM) &tr );
    if ( tr.dwBufferMax == 0 )
        return; // no text to retrieve

    nBytesToAllocate = sizeof(wchar_t) * tr.dwBufferMax;
    if ( bAddLineCR )
        nBytesToAllocate += 1*sizeof(wchar_t); // for the trailing '\r'

    if ( !tDynamicBuffer_Allocate(pTempBuf, nBytesToAllocate) )
        return; // failed to allocate the memory

    pTempBuf->nBytesStored = 0;
    tr.pBuffer = (wchar_t *) pTempBuf->ptr;
    tr.pBuffer[0] = 0;
    pTempBuf->nBytesStored += sizeof(wchar_t) * (UINT_PTR) SendMessage( hWndEdit, AEM_GETTEXTRANGEW, 0, (LPARAM) &tr );
    if ( bAddLineCR )
    {
        tr.pBuffer += pTempBuf->nBytesStored / sizeof(wchar_t);
        *tr.pBuffer = L'\r'; // the trailing '\r'
        pTempBuf->nBytesStored += 1*sizeof(wchar_t); // includes the trailing '\r'
    }
    pfrPolicy->pfnStoreResultCallback( pFindContext, pcrFound, nLinesBeforeAfter, pTempBuf, pTempBuf2, pfnAddOccurrence );
}

typedef struct sQSFindAll {
    tFindResultCallback pfnFindResultCallback; // can be NULL
    tGetFindResultPolicy GetFindResultPolicy;
    tShowFindResults ShowFindResults;
    tDynamicBuffer tempBuf;
    tDynamicBuffer tempBuf2;
} tQSFindAll;


// funcs
HWND qsearchDoInitToolTip(HWND hDlg, HWND hEdit);
void qsearchDoQuit(HWND hEdit, HWND hToolTip, HMENU hPopupMenuLoaded, HBRUSH hBrush1, HBRUSH hBrush2, HBRUSH hBrush3);
void qsearchDoSearchText(HWND hEdit, const wchar_t* cszFindWhatAW, DWORD dwParams, const DWORD dwOptFlags[], tQSFindAll* pFindAll /* = NULL */);
void qsearchDoSelFind(HWND hEdit, BOOL bFindPrev, const DWORD dwOptFlags[]);
void qsearchDoSetNotFound(HWND hEdit, BOOL bNotFound, BOOL bNotRegExp, INT nIsEOF);
void qsearchDoShowHide(HWND hDlg, BOOL bShow, UINT uShowFlags, const DWORD dwOptFlags[]);
HWND qsearchGetFindEdit(HWND hDlg, HWND* phListBox);

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
    wchar_t wszTextAW[64];

    hChWholeWord = GetDlgItem(hDlg, IDC_CH_WHOLEWORD);

    if ( g_Plugin.bOldWindows )
    {
        lstrcpyA( (char *) wszTextAW, qsearchGetTextA(IDC_CH_WHOLEWORD) );
        if ( dwOptFlags[OPTF_SRCH_USE_SPECIALCHARS] )
            lstrcatA( (char *) wszTextAW, "*" );
        else if ( dwOptFlags[OPTF_SRCH_USE_REGEXP] )
            lstrcatA( (char *) wszTextAW, "^" );
        SetWindowTextA(hChWholeWord, (const char *) wszTextAW);
    }
    else
    {
        lstrcpyW( wszTextAW, qsearchGetTextW(IDC_CH_WHOLEWORD) );
        if ( dwOptFlags[OPTF_SRCH_USE_SPECIALCHARS] )
            lstrcatW( wszTextAW, L"*" );
        else if ( dwOptFlags[OPTF_SRCH_USE_REGEXP] )
            lstrcatW( wszTextAW, L"^" );
        SetWindowTextW(hChWholeWord, wszTextAW);
    }
}

BOOL qsIsHotKeyPressed(DWORD dwHotKey)
{
    if ( dwHotKey )
    {
        if ( GetKeyState(LOBYTE(dwHotKey)) & 0x80 )
        {
            if ( GetKeyState(VK_MENU) & 0x80 )
            {
                if ( (HIBYTE(dwHotKey) & HOTKEYF_ALT) != HOTKEYF_ALT )
                    return FALSE;
            }
            else
            {
                if ( HIBYTE(dwHotKey) & HOTKEYF_ALT )
                    return FALSE;
            }

            if ( GetKeyState(VK_CONTROL) & 0x80 )
            {
                if ( (HIBYTE(dwHotKey) & HOTKEYF_CONTROL) != HOTKEYF_CONTROL )
                    return FALSE;
            }
            else
            {
                if ( HIBYTE(dwHotKey) & HOTKEYF_CONTROL )
                    return FALSE;
            }

            if ( GetKeyState(VK_SHIFT) & 0x80 )
            {
                if ( (HIBYTE(dwHotKey) & HOTKEYF_SHIFT) != HOTKEYF_SHIFT )
                    return FALSE;
            }
            else
            {
                if ( HIBYTE(dwHotKey) & HOTKEYF_SHIFT )
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
        GetWindowTextW( hEdit, szTextAW, MAX_TEXT_SIZE - 1 );
}

static void setEditFindText(HWND hEdit, const wchar_t* pszTextAW)
{
    if ( g_Plugin.bOldWindows )
    {
        SetWindowTextA( hEdit, (LPCSTR) pszTextAW );
    }
    else
    {
        SetWindowTextW( hEdit, pszTextAW );
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

static BOOL getAkelPadSelectedText(wchar_t szTextAW[MAX_TEXT_SIZE], const DWORD dwOptFlags[])
{
    HWND hWndEdit;

    g_bWordJustSelectedByFnd = FALSE;

    hWndEdit = GetWndEdit(g_Plugin.hMainWnd);
    if ( hWndEdit )
    {
        CHARRANGE_X cr = { 0, 0 };

        SendMessage( hWndEdit, EM_EXGETSEL_X, 0, (LPARAM) &cr );
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
                SendMessageA( hWndEdit, EM_GETTEXTRANGE_X, 0, (LPARAM) &trA );
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
                trW.lpstrText = szTextAW;
                SendMessageW( hWndEdit, EM_GETTEXTRANGE_X, 0, (LPARAM) &trW );
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
        case WM_NOTIFY:
        {
            if ( hBtn == g_QSearchDlg.hBtnFindAll )
            {
                if ( g_Plugin.bOldWindows )
                {
                    if ( ((LPNMHDR) lParam)->code == TTN_GETDISPINFOA )
                    {
                        LPNMTTDISPINFOA lpnmdiA;

                        lpnmdiA = (LPNMTTDISPINFOA) lParam;
                        SendMessageA(lpnmdiA->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);
                        lpnmdiA->lpszText = (LPSTR) qsearchGetHintA(IDC_BT_FINDALL);
                        return 0;
                    }
                }
                else
                {
                    if ( ((LPNMHDR) lParam)->code == TTN_GETDISPINFOW )
                    {
                        LPNMTTDISPINFOW lpnmdiW;

                        lpnmdiW = (LPNMTTDISPINFOW) lParam;
                        SendMessageW(lpnmdiW->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);
                        lpnmdiW->lpszText = (LPWSTR) qsearchGetHintW(IDC_BT_FINDALL);
                        return 0;
                    }
                }
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

static HWND GetDlgItemAndRect(HWND hDlg, int nItemId, RECT* pRect)
{
    HWND hDlgItm;

    hDlgItm = GetDlgItem(hDlg, nItemId);
    if ( hDlgItm )
    {
        GetWindowRect( hDlgItm, pRect );
        ScreenToClient( hDlg, (POINT *) &pRect->left );
        ScreenToClient( hDlg, (POINT *) &pRect->right );
    }

    return hDlgItm;
}

static void MoveWindowByDx(HWND hWnd, const RECT* rc0, LONG dx)
{
    if ( hWnd )
    {
        MoveWindow(hWnd, rc0->left + dx, rc0->top, rc0->right - rc0->left, rc0->bottom - rc0->top, TRUE);
    }
}

static void ResizeWindowByDx(HWND hWnd, const RECT* rc0, LONG dx)
{
    if ( hWnd )
    {
        MoveWindow(hWnd, rc0->left, rc0->top, rc0->right - rc0->left + dx, rc0->bottom - rc0->top, TRUE);
    }
}

static void RedrawWindowByDx(HWND hWnd)
{
    if ( hWnd )
    {
        InvalidateRect(hWnd, NULL, TRUE);
        UpdateWindow(hWnd);
    }
}

static void RedrawWindowEntire(HWND hWnd)
{
    InvalidateRect(hWnd, NULL, TRUE);
    UpdateWindow(hWnd);
}

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

static void OnSrchModeChanged(void)
{
    qsdlgShowHideWholeWordCheckBox(g_QSearchDlg.hDlg, g_Options.dwFlags);
    qs_bForceFindFirst = TRUE;
    qs_nEditEOF = 0;
    qsearchDoSetNotFound( g_QSearchDlg.hFindEdit, FALSE, FALSE, 0 );

    if ( g_Options.dwFlags[OPTF_SRCH_HIGHLIGHTALL] )
    {
        qsearchDoTryUnhighlightAll();
    }

    qsSetInfoEmpty_Tracking("OnSrchModeChanged");
}

static void OnChWholeWordSrchMode(void)
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

    OnSrchModeChanged();
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

UINT qsPickUpSelection(HWND hEdit, const DWORD dwOptFlags[], BOOL isHighlightAll)
{
    UINT nResult = 0;
    wchar_t prevFindTextAW[MAX_TEXT_SIZE];

    strcpyAorW(prevFindTextAW, g_QSearchDlg.szFindTextAW);

    if ( getAkelPadSelectedText(g_QSearchDlg.szFindTextAW, dwOptFlags) )
    {
        if ( !isHighlightAll )
        {
            qs_bEditTextChanged = (strcmpAorW(prevFindTextAW, g_QSearchDlg.szFindTextAW, dwOptFlags[OPTF_SRCH_MATCHCASE]) != 0);
        }
        else
        {
            qs_bEditTextChanged = TRUE;
        }

        if ( qs_bEditTextChanged )
        {
            qsearchDoSetNotFound(hEdit, FALSE, FALSE, 0);
            setEditFindText(hEdit, g_QSearchDlg.szFindTextAW);

            SendMessage(hEdit, EM_SETSEL, 0, -1);
            qs_bEditSelJustChanged = TRUE;

            if ( dwOptFlags[OPTF_SRCH_ONTHEFLY_MODE] )
            {
                qs_bForceFindFirst = FALSE;
                qsearchDoTryHighlightAll(g_QSearchDlg.hDlg, g_QSearchDlg.szFindTextAW, dwOptFlags, QHC_CHECKBOX_CHECKED);
            }
            else
                qs_bForceFindFirst = TRUE;

            nResult |= QS_PSF_TEXTCHANGED;
        }

        nResult |= QS_PSF_PICKEDUP;
    }

    return nResult;
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
    BOOL isPickupText;
    BOOL isFindBegin;
    BOOL isFindUp;

    isPickupText = GetKeyState(VK_QS_PICKUPTEXT) & 0x80; // Ctrl
    isFindBegin = GetKeyState(VK_QS_FINDBEGIN) & 0x80; // Alt
    isFindUp = GetKeyState(VK_QS_FINDUP) & 0x80; // Shift

    if ( isPickupText && !isFindBegin && !isFindUp )
    {
        if ( getAkelPadSelectedText(g_QSearchDlg.szFindTextAW, dwOptFlags) )
        {
            setEditFindText(hEdit, g_QSearchDlg.szFindTextAW);
            SendMessage( g_QSearchDlg.hDlg, QSM_SETNOTFOUND, FALSE, 0 );
            qsSetInfoEmpty_Tracking("OnEditKeyDown_Enter_or_F3");
        }
        SendMessage(hEdit, EM_SETSEL, 0, -1);
        #ifdef _DEBUG
          Debug_OutputA("editWndProc, WM_KEYDOWN, (RETURN||F3)&&PickUp, SETSEL(0, -1)\n");
        #endif
        qs_bEditTextChanged = TRUE;
        if ( dwOptFlags[OPTF_SRCH_ONTHEFLY_MODE] )
        {
            qs_bForceFindFirst = FALSE;
            qsearchDoTryHighlightAll(g_QSearchDlg.hDlg, g_QSearchDlg.szFindTextAW, dwOptFlags, QHC_CHECKBOX_CHECKED);
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
                        //g_QSearchDlg.szFindTextAW[0] = 0;
                        //SendMessageA(hCombo, CB_GETLBTEXT, iItem, (LPARAM) g_QSearchDlg.szFindTextAW);
                        //SendMessageA(hCombo, CB_SETCURSEL, iItem, 0);
                    SendMessage(hCombo, CB_SHOWDROPDOWN, FALSE, 0);
                        //SetWindowTextA(hEdit, (LPCSTR) g_QSearchDlg.szFindTextAW);
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

        if ( isFindBegin && isFindUp && !isPickupText )
        {
            SendMessage( g_QSearchDlg.hDlg, QSM_FINDALL, g_Options.dwFindAllMode, 0 );
        }
        else if ( isPickupText && isFindUp )
        {
            SendMessage( g_QSearchDlg.hDlg, QSM_FINDALL, g_Options.dwFindAllMode | QS_FINDALL_RSLT_ALLFILES, 0 );
        }
        else if ( !isPickupText )
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
    static BOOL  bHotKeyQSearchPressed = FALSE;
    static BOOL  bHotKeyFindAllPressed = FALSE;
    static BOOL  bEditTrackingMouse = FALSE;

    #ifdef _DEBUG
      //Debug_OutputA("Edit Msg 0x%X:  0x0%X  0x0%X\n", uMsg, wParam, lParam);
    #endif

    switch ( uMsg )
    {
        case WM_KEYDOWN:
        {
            bHotKeyQSearchPressed = FALSE;
            bHotKeyFindAllPressed = FALSE;

            if ( (wParam == VK_DOWN) || (wParam == VK_UP) ||
                 (wParam == VK_PRIOR) || (wParam == VK_NEXT) )
            {
                if ( qsearchIsFindHistoryEnabled() )
                {
                    HWND hCombo = GetDlgItem(g_QSearchDlg.hDlg, IDC_CB_FINDTEXT);
                    if ( !SendMessage(hCombo, CB_GETDROPPEDSTATE, 0, 0) )
                    {
                        int iItem = (int) SendMessage(hCombo, CB_GETCURSEL, 0, 0);

                        getEditFindText(hEdit, g_QSearchDlg.szFindTextAW);
                        if ( qsearchFindHistoryAdd(hEdit, g_QSearchDlg.szFindTextAW, 0) )
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

            if ( wParam == VK_F3 )
            {
                // F3 is actually processed by qsearchDlgProc
                return 0;
            }
            else if ( wParam == VK_RETURN )
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
                        SendMessage( g_QSearchDlg.hDlg, QSM_FINDFIRST, QSEARCH_NOFINDUP_VK | QSEARCH_NOFINDBEGIN_VK | QSEARCH_USEDELAY, 0 );
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
                        SendMessage( g_QSearchDlg.hDlg, QSM_FINDFIRST, QSEARCH_NOFINDUP_VK | QSEARCH_NOFINDBEGIN_VK | QSEARCH_USEDELAY, 0 );
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
                // if ( GetKeyState(VK_CONTROL) & 0x80 ) // Ctrl+Tab, Ctrl+Shift+Tab, ...
                // {
                //
                // }
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
                          Debug_OutputA("editWndProc, WM_KEYDOWN, Ctrl+A, SETSEL(0, -1)\n");
                        #endif
                        return 0;
                    }
                }
            }

            if ( LOBYTE(wParam) == LOBYTE(g_QSearchDlg.dwHotKeyQSearch) )
            {
                bHotKeyQSearchPressed = qsIsHotKeyPressed(g_QSearchDlg.dwHotKeyQSearch);
                if ( bHotKeyQSearchPressed )
                {
                    return 0;
                }
            }
            if ( LOBYTE(wParam) == LOBYTE(g_QSearchDlg.dwHotKeyFindAll) )
            {
                bHotKeyFindAllPressed = qsIsHotKeyPressed(g_QSearchDlg.dwHotKeyFindAll);
                if ( bHotKeyFindAllPressed )
                {
                    return 0;
                }
            }
            if ( LOBYTE(wParam) == LOBYTE(g_QSearchDlg.dwHotKeyGoToNextFindAllMatch) )
            {
                bHotKeyFindAllPressed = qsIsHotKeyPressed(g_QSearchDlg.dwHotKeyGoToNextFindAllMatch);
                if ( bHotKeyFindAllPressed )
                {
                    return 0;
                }
            }
            if ( LOBYTE(wParam) == LOBYTE(g_QSearchDlg.dwHotKeyGoToPrevFindAllMatch) )
            {
                bHotKeyFindAllPressed = qsIsHotKeyPressed(g_QSearchDlg.dwHotKeyGoToPrevFindAllMatch);
                if ( bHotKeyFindAllPressed )
                {
                    return 0;
                }
            }
            break;
        }
        case WM_SYSKEYDOWN:
        {
            // it's required when g_QSearchDlg.dwHotKeyQSearch contains Alt
            // (Alt+key do not come as WM_KEYDOWN)
            if ( LOBYTE(wParam) == LOBYTE(g_QSearchDlg.dwHotKeyQSearch) )
            {
                bHotKeyQSearchPressed = qsIsHotKeyPressed(g_QSearchDlg.dwHotKeyQSearch);
                if ( bHotKeyQSearchPressed )
                {
                    return 0;
                }
            }
            if ( LOBYTE(wParam) == LOBYTE(g_QSearchDlg.dwHotKeyFindAll) )
            {
                bHotKeyFindAllPressed = qsIsHotKeyPressed(g_QSearchDlg.dwHotKeyFindAll);
                if ( bHotKeyFindAllPressed )
                {
                    return 0;
                }
            }
            if ( LOBYTE(wParam) == LOBYTE(g_QSearchDlg.dwHotKeyGoToNextFindAllMatch) )
            {
                bHotKeyFindAllPressed = qsIsHotKeyPressed(g_QSearchDlg.dwHotKeyGoToNextFindAllMatch);
                if ( bHotKeyFindAllPressed )
                {
                    return 0;
                }
            }
            if ( LOBYTE(wParam) == LOBYTE(g_QSearchDlg.dwHotKeyGoToPrevFindAllMatch) )
            {
                bHotKeyFindAllPressed = qsIsHotKeyPressed(g_QSearchDlg.dwHotKeyGoToPrevFindAllMatch);
                if ( bHotKeyFindAllPressed )
                {
                    return 0;
                }
            }
            if ( wParam == VK_F3 )
            {
                // Alt+F3 is actually processed by qsearchDlgProc
                return 0;
            }
            if ( wParam == VK_RETURN  )
            {
                // Alt+Enter pressed
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
            if ( bHotKeyFindAllPressed )
            {
                bHotKeyFindAllPressed = FALSE;
                return 0;
            }

            if ( bHotKeyQSearchPressed )
            {
                bHotKeyQSearchPressed = FALSE;
                if ( !qs_bHotKeyQSearchPressedOnShow )
                {
                    if ( g_Options.dwFlags[OPTF_HOTKEY_HIDES_PANEL] & 0x01 )
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

            qs_bHotKeyQSearchPressedOnShow = FALSE;

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
            if ( bHotKeyFindAllPressed )
            {
                bHotKeyFindAllPressed = FALSE;
                return 0;
            }

            if ( bHotKeyQSearchPressed )
            {
                SendMessage(hEdit, WM_KEYUP, wParam, lParam);
                return 0;
            }
            break;
        }
        case WM_CHAR:
        {
            if ( bHotKeyQSearchPressed ||
                 bHotKeyFindAllPressed ||
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
                    SendMessage( g_QSearchDlg.hDlg, QSM_FINDFIRST, QSEARCH_NOFINDUP_VK | QSEARCH_NOFINDBEGIN_VK | QSEARCH_USEDELAY, 0 );
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
            RedrawWindowEntire(hEdit);
            if ( qs_bEditCanBeNonActive )
            {
                if ( g_Options.dwFlags[OPTF_EDIT_FOCUS_SELECTALL] )
                {
                    SendMessage(hEdit, EM_SETSEL, 0, -1);
                    #ifdef _DEBUG
                      Debug_OutputA("editWndProc, WM_SETFOCUS, if (SelectAll), SETSEL(0, -1)\n");
                    #endif
                }
                else
                {
                    if ( !qs_bEditSelJustChanged )
                    {
                        SendMessage(hEdit, EM_SETSEL, dwSelPos1, dwSelPos2);
                        #ifdef _DEBUG
                          Debug_OutputA("editWndProc, WM_SETFOCUS, if (!SelJustChanged), SETSEL(%d, %d)\n", dwSelPos1, dwSelPos2);
                        #endif
                    }
                    else
                    {
                        qs_bEditSelJustChanged = FALSE;
                        SendMessage( hEdit, EM_GETSEL,
                          (WPARAM) &dwSelPos1, (LPARAM) &dwSelPos2 );
                        #ifdef _DEBUG
                          Debug_OutputA("editWndProc, WM_GETFOCUS, if (SelJustChanged), GETSEL(%d, %d)\n", dwSelPos1, dwSelPos2);
                        #endif
                    }
                }
            }
            g_QSearchDlg.bMouseJustLeavedFindEdit = FALSE;
            //#ifdef _DEBUG
            //  Debug_OutputA("QSearchDlg.c, editWndProc, WM_SETFOCUS, g_QSearchDlg.MouseJustLeavedFindEdit = FALSE;\n");
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
                      Debug_OutputA("editWndProc, WM_KILLFOCUS, GETSEL(%d, %d)\n", dwSelPos1, dwSelPos2);
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
                    //  Debug_OutputA("QSearchDlg.c, editWndProc, WM_KILLFOCUS, g_QSearchDlg.MouseJustLeavedFindEdit = FALSE;\n");
                    //#endif
                }
                /*
                RedrawWindowEntire(hEdit);
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
                //  Debug_OutputA("QSearchDlg.c, editWndProc, WM_MOUSEMOVE, g_QSearchDlg.MouseJustLeavedFindEdit = FALSE;\n");
                //#endif
            }
            break;
        }
        case WM_MOUSELEAVE:
        {
            bEditTrackingMouse = FALSE;
            g_QSearchDlg.bMouseJustLeavedFindEdit = TRUE;
            //#ifdef _DEBUG
            //  Debug_OutputA("QSearchDlg.c, editWndProc, WM_MOUSELEAVE, g_QSearchDlg.MouseJustLeavedFindEdit = TRUE;\n");
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
                        Debug_OutputA("editWndProc, WM_GETDLGCODE, GETSEL(%d, %d)\n", pos1, pos2);
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

static void qsFindAndHighlight(HWND hDlg, HWND hEdit, const wchar_t* cszFindWhatAW, DWORD dwParams, const DWORD dwOptFlags[])
{
    qsearchDoSearchText(hEdit, cszFindWhatAW, dwParams, dwOptFlags, NULL);
    qsearchDoTryHighlightAll(hDlg, cszFindWhatAW, dwOptFlags, QHC_CHECKBOX_CHECKED);
}

static void qsUpdateHighlight(HWND hDlg, HWND hEdit, const DWORD dwOptFlags[])
{
    wchar_t szSelectedTextAW[MAX_TEXT_SIZE];

    getEditFindText(hEdit, g_QSearchDlg.szFindTextAW);

    if ( getAkelPadSelectedText(szSelectedTextAW, dwOptFlags) )
    {
        BOOL bEqual = FALSE;

        if ( dwOptFlags[OPTF_SRCH_USE_REGEXP] )
        {
            bEqual = TRUE;
        }
        else
        {
            const wchar_t* pszFindTextAW;
            const wchar_t* pszSelTextAW;
            wchar_t szFindTextBufAW[MAX_TEXT_SIZE];
            wchar_t szSelTextBufAW[MAX_TEXT_SIZE];

            if ( dwOptFlags[OPTF_SRCH_MATCHCASE] )
            {
                // match case
                pszFindTextAW = g_QSearchDlg.szFindTextAW;
                pszSelTextAW = szSelectedTextAW;
            }
            else
            {
                // case-insensitive, needed for match_mask and match_maskw
                if ( g_Plugin.bOldWindows )
                {
                    lstrcpyA( (LPSTR) szFindTextBufAW, (LPCSTR) g_QSearchDlg.szFindTextAW );
                    CharUpperA( (LPSTR) szFindTextBufAW );
                    lstrcpyA( (LPSTR) szSelTextBufAW, (LPCSTR) szSelectedTextAW );
                    CharUpperA( (LPSTR) szSelTextBufAW );
                }
                else
                {
                    lstrcpyW( szFindTextBufAW, g_QSearchDlg.szFindTextAW );
                    CharUpperW( szFindTextBufAW );
                    lstrcpyW( szSelTextBufAW, szSelectedTextAW );
                    CharUpperW( szSelTextBufAW );
                }
                pszFindTextAW = szFindTextBufAW;
                pszSelTextAW = szSelTextBufAW;
            }

            if ( dwOptFlags[OPTF_SRCH_USE_SPECIALCHARS] )
            {
                if ( g_Plugin.bOldWindows )
                    bEqual = (match_mask((LPCSTR)pszFindTextAW, (LPCSTR)pszSelTextAW, 0, 0) > 0);
                else
                    bEqual = (match_maskw(pszFindTextAW, pszSelTextAW, 0, 0) > 0);
            }
            else
            {
                // in case of case-insensitive comparison, CharUpper have already been called,
                // so characters' case is the same (it's upper)
                bEqual = (strcmpAorW(pszFindTextAW, pszSelTextAW, TRUE) == 0);
            }
        }

        qs_bEditTextChanged = FALSE;

        if ( !bEqual )
        {
            DWORD dwOptFlagsTemp[OPTF_COUNT_TOTAL];
            copyOptionsFlags(dwOptFlagsTemp, dwOptFlags);
            dwOptFlagsTemp[OPTF_SRCH_STOP_EOF] = 0; // disabling here
            dwOptFlagsTemp[OPTF_SRCH_FROM_BEGINNING] = 0;
            qsearchDoSearchText( hEdit, g_QSearchDlg.szFindTextAW, QSEARCH_NEXT | QSEARCH_NOFINDBEGIN_VK | QSEARCH_NOFINDUP_VK, dwOptFlagsTemp, NULL );
        }

        qsearchDoTryHighlightAll(hDlg, g_QSearchDlg.szFindTextAW, dwOptFlags, QHC_CHECKBOX_CHECKED);
    }
    else
    {
        // no text selected - trying to find & highlight
        DWORD dwOptFlagsTemp[OPTF_COUNT_TOTAL];
        copyOptionsFlags(dwOptFlagsTemp, dwOptFlags);
        dwOptFlagsTemp[OPTF_SRCH_STOP_EOF] = 0; // disabling here
        dwOptFlagsTemp[OPTF_SRCH_FROM_BEGINNING] = 0;
        qsFindAndHighlight( hDlg, hEdit, g_QSearchDlg.szFindTextAW, QSEARCH_NEXT | QSEARCH_NOFINDBEGIN_VK | QSEARCH_NOFINDUP_VK, dwOptFlagsTemp );
    }
}

void qsUpdateHighlightForFindAll(const wchar_t* cszFindWhat, DWORD dwFindAllFlags, BOOL bForceHighlight)
{
    DWORD dwOptFlagsTemp[OPTF_COUNT_TOTAL];

    if ( !g_QSearchDlg.hDlg || !g_Options.dwFlags[OPTF_SRCH_HIGHLIGHTALL] || g_Plugin.bOldWindows )
        return;

    copyOptionsFlags(dwOptFlagsTemp, g_Options.dwFlags);
    dwOptFlagsTemp[OPTF_SRCH_FROM_BEGINNING] = 0;
    dwOptFlagsTemp[OPTF_SRCH_STOP_EOF] = 0;
    dwOptFlagsTemp[OPTF_SRCH_REGEXP_DOT_NEWLINE] = 0;
    dwOptFlagsTemp[OPTF_SRCH_USE_SPECIALCHARS] = (dwFindAllFlags & QS_FAF_SPECCHAR) ? 1 : 0;
    dwOptFlagsTemp[OPTF_SRCH_USE_REGEXP] = (dwFindAllFlags & QS_FAF_REGEXP) ? 1 : 0;
    dwOptFlagsTemp[OPTF_SRCH_MATCHCASE] = (dwFindAllFlags & QS_FAF_MATCHCASE) ? 1 : 0;
    dwOptFlagsTemp[OPTF_SRCH_WHOLEWORD] = (dwFindAllFlags & QS_FAF_WHOLEWORD) ? 1 : 0;

    qsearchDoTryHighlightAll(g_QSearchDlg.hDlg, cszFindWhat, dwOptFlagsTemp,
        QHC_CHECKBOX_CHECKED | QHC_IGNORE_SELECTION | QHC_DONT_CUT_REGEXP | (bForceHighlight ? QHC_FORCE_HIGHLIGHT : 0)
    );
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

static BOOL getEditorColors(COLORREF* pcrTextColor, COLORREF* pcrBkgndColor)
{
    PLUGINFUNCTION* pf;
    int nValuesRead;

    if ( g_Plugin.bOldWindows )
        return FALSE;

    nValuesRead = 0;
    pf = (PLUGINFUNCTION *) SendMessageW( g_Plugin.hMainWnd, AKD_DLLFINDW, (WPARAM) cszHighlightMainW, 0 );

    if ( pf && pf->bRunning )
    {
        HWND hWndEdit;
        COLORREF crColor;
        INT_PTR nValueLen;
        WCHAR szValue[64];

        hWndEdit = GetWndEdit(g_Plugin.hMainWnd);

        nValueLen = GetCoderVariableW(hWndEdit, L"HighLight_BasicTextColor", szValue);
        if ( nValueLen >= 7 && szValue[0] == _T('#') )
        {
            crColor = (COLORREF) xhextoiW(szValue + 1);
            *pcrTextColor = RGB( GetBValue(crColor), GetGValue(crColor), GetRValue(crColor) );
            ++nValuesRead;
        }

        nValueLen = GetCoderVariableW(hWndEdit, L"HighLight_BasicBkColor", szValue);
        if ( nValueLen >= 7 && szValue[0] == _T('#') )
        {
            crColor = (COLORREF) xhextoiW(szValue + 1);
            *pcrBkgndColor = RGB( GetBValue(crColor), GetGValue(crColor), GetRValue(crColor) );
            ++nValuesRead;
        }
    }

    return (nValuesRead == 2);
}

void qsearchDlgApplyEditorColors(void)
{
    COLORREF crTextColor;
    COLORREF crBkgndColor;
    BOOL bColorChanged;

    bColorChanged = FALSE;

    if ( !g_Options.dwUseEditorColors || !getEditorColors(&crTextColor, &crBkgndColor) )
    {
        crTextColor = GetSysColor(COLOR_WINDOWTEXT);
        crBkgndColor = GetSysColor(COLOR_WINDOW);
    }

    if ( g_QSearchDlg.crTextColor != crTextColor )
    {
        g_QSearchDlg.crTextColor = crTextColor;
        bColorChanged = TRUE;
    }

    if ( g_QSearchDlg.crBkgndColor != crBkgndColor )
    {
        g_QSearchDlg.crBkgndColor = crBkgndColor;
        if ( g_QSearchDlg.hBkgndBrush )
        {
            DeleteObject(g_QSearchDlg.hBkgndBrush);
        }
        g_QSearchDlg.hBkgndBrush = CreateSolidBrush(g_QSearchDlg.crBkgndColor);
        bColorChanged = TRUE;
    }

    if ( bColorChanged && g_QSearchDlg.hFindEdit && IsWindowVisible(g_QSearchDlg.hFindEdit) )
    {
        RedrawWindowEntire(g_QSearchDlg.hFindEdit);
    }
}

static void OnChMatchCaseOrWholeWordClicked(HWND hDlg)
{
    UINT nPickedUp = 0;
    BOOL bHighlightAllChecked = g_Options.dwFlags[OPTF_SRCH_HIGHLIGHTALL] ? TRUE : FALSE;

    qs_bEditNotFound = FALSE;

    if ( bHighlightAllChecked &&
        g_QSearchDlg.uSearchOrigin == QS_SO_EDITOR &&
        g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE] &&
        g_Options.dwFlags[OPTF_SRCH_PICKUP_SELECTION] == PICKUP_SEL_ALWAYS )
    {
        qs_bForceFindFirst = TRUE;
        nPickedUp = qsPickUpSelection(g_QSearchDlg.hFindEdit, g_Options.dwFlags, FALSE) & QS_PSF_PICKEDUP;
    }

    qsSetInfoEmpty_Tracking("OnChMatchCaseOrWholeWordClicked");

    if ( !nPickedUp )
    {
        qs_bForceFindFirst = TRUE;
        qs_bEditTextChanged = TRUE;
        qsearchDoSetNotFound(g_QSearchDlg.hFindEdit, FALSE, FALSE, 0);

        if ( bHighlightAllChecked )
        {
            DWORD dwOptFlagsTemp[OPTF_COUNT_TOTAL];

            getEditFindText(g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextAW);

            copyOptionsFlags(dwOptFlagsTemp, g_Options.dwFlags);
            dwOptFlagsTemp[OPTF_SRCH_ONTHEFLY_MODE] = 1;
            dwOptFlagsTemp[OPTF_SRCH_STOP_EOF] = 0;
            qsearchDoSearchText( g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextAW, QSEARCH_FIRST, dwOptFlagsTemp, NULL );

            qsearchDoTryHighlightAll(hDlg, g_QSearchDlg.szFindTextAW, g_Options.dwFlags, QHC_CHECKBOX_CHECKED | QHC_FINDFIRST);
        }
    }
    else
    {
        qs_bEditTextChanged = TRUE;
        qsearchDoSetNotFound(g_QSearchDlg.hFindEdit, FALSE, FALSE, 0);
    }
}

static void fillToolInfoA(
    TOOLINFOA* lptiA,
    LPSTR      lpToolTipText,
    HWND       hWnd,
    UINT       uId)
{
    RECT rect = { 0, 0, 0, 0 };

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
}

static void fillToolInfoW(
    TOOLINFOW* lptiW,
    LPWSTR     lpToolTipText,
    HWND       hWnd,
    UINT       uId)
{
    RECT rect = { 0, 0, 0, 0 };

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
}

INT_PTR CALLBACK qsearchDlgProc(HWND hDlg,
  UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND   hToolTip = NULL;
    static HMENU  hPopupMenuLoaded = NULL;
    static HBRUSH hTextNotFoundBrush = NULL;
    static HBRUSH hTextNotRegExpBrush = NULL;
    static HBRUSH hTextEOFBrush = NULL;
    //static BOOL   bHotKeyQSearchPressed = FALSE;
    static BOOL bHotKeyFindAllPressed = FALSE;

    static HWND hEdFindText = NULL;
    static HWND hCbFindText = NULL;
    static HWND hBtFindNext = NULL, hBtFindPrev = NULL, hBtFindAll = NULL;
    static HWND hBtCancel = NULL;
    static HWND hPbProgress = NULL;
    static HWND hChMatchCase = NULL, hChWholeWord = NULL, hChHighlightAll = NULL;
    static HWND hStInfo = NULL;

    static RECT rcDlg_0 = { 0, 0, 0, 0 };
    static RECT rcStInfo_0 = { 0, 0, 0, 0 };
    static RECT rcChHighlightAll_0 = { 0, 0, 0, 0 };
    static RECT rcChWholeWord_0 = { 0, 0, 0, 0 };
    static RECT rcChMatchCase_0 = { 0, 0, 0, 0 };
    static RECT rcPbProgress_0 = { 0, 0, 0, 0 };
    static RECT rcBtFindAll_0 = { 0, 0, 0, 0 };
    static RECT rcBtFindPrev_0 = { 0, 0, 0, 0 };
    static RECT rcBtFindNext_0 = { 0, 0, 0, 0 };
    static RECT rcCbFindText_0 = { 0, 0, 0, 0 };
    static RECT rcEdFindText_0 = { 0, 0, 0, 0 };

    #ifdef _DEBUG
      //Debug_OutputA("Dlg Msg 0x%X:  0x0%X  0x0%X\n", uMsg, wParam, lParam);
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
                BOOL bPrevNotFound;

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
                    getEditFindText( g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextAW );
                    qsearchFindHistoryAdd( g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextAW, 0 );
                    qsPickUpSelection( g_QSearchDlg.hFindEdit, g_Options.dwFlags, FALSE );
                }*/
                getEditFindText( g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextAW );
                bPrevNotFound = qs_bEditNotFound;
                qsearchDoSearchText( g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextAW, uSearch, g_Options.dwFlags, NULL );
                if ( g_bFrameActivated || (uSearch & QSEARCH_FIRST) || (bPrevNotFound && !qs_bEditNotFound) ||
                     g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] ||
                     !QSearchDlgState_isLastHighlightedEqualToTheSearch(&g_QSearchDlg, g_QSearchDlg.szFindTextAW, getFindAllFlags(g_Options.dwFlags)) )
                {
                    g_bFrameActivated = FALSE;
                    qsearchDoTryHighlightAll( hDlg, g_QSearchDlg.szFindTextAW, g_Options.dwFlags, QHC_CHECKBOX_CHECKED );
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
                DWORD dwAdditionalFlags = 0;
                if ( (GetKeyState(VK_MENU) & 0x80) ||
                     (GetKeyState(VK_CONTROL) & 0x80) ||
                     (GetKeyState(VK_SHIFT) & 0x80) )
                {
                    dwAdditionalFlags = QS_FINDALL_RSLT_ALLFILES;
                }
                PostMessage( hDlg, QSM_FINDALL, g_Options.dwFindAllMode | dwAdditionalFlags, 0 );
                return 1;
            }
            else if ( id == IDC_CH_MATCHCASE )
            {
                if ( HIWORD(wParam) == BN_CLICKED )
                {
                    g_Options.dwFlags[OPTF_SRCH_MATCHCASE] = isCheckBoxChecked(hDlg, IDC_CH_MATCHCASE);
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
                    g_Options.dwFlags[OPTF_SRCH_WHOLEWORD] = isCheckBoxChecked(hDlg, IDC_CH_WHOLEWORD);
                    OnChMatchCaseOrWholeWordClicked(hDlg);
                }
            }
            else if ( id == IDC_CH_HIGHLIGHTALL )
            {
                if ( HIWORD(wParam) == BN_CLICKED )
                {
                    UINT nPickedUp = 0;
                    BOOL bHighlightAllChecked = isCheckBoxChecked(hDlg, IDC_CH_HIGHLIGHTALL);

                    g_Options.dwFlags[OPTF_SRCH_HIGHLIGHTALL] = bHighlightAllChecked;

                    QSearchDlgState_clearLastHighlighted(&g_QSearchDlg);

                    if ( bHighlightAllChecked &&
                         g_QSearchDlg.uSearchOrigin == QS_SO_EDITOR &&
                         g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE] &&
                         g_Options.dwFlags[OPTF_SRCH_PICKUP_SELECTION] == PICKUP_SEL_ALWAYS )
                    {
                        g_Options.dwHighlightState |= HLS_IS_CHECKED;
                        nPickedUp = qsPickUpSelection(g_QSearchDlg.hFindEdit, g_Options.dwFlags, TRUE) & QS_PSF_PICKEDUP;
                    }

                    qsSetInfoEmpty_Tracking("qsearchDlgProc, IDC_CH_HIGHLIGHTALL");

                    if ( !nPickedUp )
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
                        qsearchDoSetNotFound(g_QSearchDlg.hFindEdit, FALSE, FALSE, 0);
                        qsSetInfoEmpty_Tracking("qsearchDlgProc, IDC_CB_FINDTEXT");
                        break;
                    case CBN_DROPDOWN:
                        getEditFindText(g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextAW);
                        qsearchFindHistoryAdd(g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextAW, 0);
                        /*{
                            HWND hCombo;
                            int iItem;

                            hCombo = GetDlgItem(g_QSearchDlg.hDlg, IDC_CB_FINDTEXT);
                            if ( g_Plugin.bOldWindows )
                                iItem = (int) SendMessageA(hCombo, CB_FINDSTRING, (WPARAM) (0), (LPARAM) g_QSearchDlg.szFindTextAW);
                            else
                                iItem = (int) SendMessageW(hCombo, CB_FINDSTRING, (WPARAM) (0), (LPARAM) g_QSearchDlg.szFindTextAW);

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
                        case OPTF_HOTKEY_HIDES_PANEL:
                        case OPTF_EDITOR_AUTOFOCUS_MOUSE:
                            if ( (state & MF_CHECKED) == MF_CHECKED )
                            {
                                if ( g_Options.dwFlags[id - IDM_START] & 0x01 )
                                {
                                    g_Options.dwFlags[id - IDM_START] -= 0x01;

                                    if ( id == IDM_START + OPTF_SRCH_STOP_EOF )
                                        qsearchDoSetNotFound(g_QSearchDlg.hFindEdit, FALSE, FALSE, 0);
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
                        qsearchDoSetNotFound( g_QSearchDlg.hFindEdit, FALSE, FALSE, 0 );
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
                        OnSrchModeChanged();
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
                        OnSrchModeChanged();
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
            else if ( id == IDM_FINDALL_AUTO_COUNT )
            {
                if ( g_QSearchDlg.hFindAllPopupMenu )
                {
                    unsigned int state = GetMenuState(g_QSearchDlg.hFindAllPopupMenu, id, MF_BYCOMMAND);
                    if ( (state & MF_CHECKED) == MF_CHECKED )
                    {
                        if ( g_Options.dwFindAllMode & QS_FINDALL_AUTO_COUNT_FLAG )
                            g_Options.dwFindAllMode -= QS_FINDALL_AUTO_COUNT_FLAG;

                        QSearchDlgState_clearCurrentMatches(&g_QSearchDlg, TRUE);
                        qsSetInfoEmpty_Tracking("qsearchDlgProc, IDM_FINDALL_AUTO_COUNT");
                    }
                    else
                        g_Options.dwFindAllMode |= QS_FINDALL_AUTO_COUNT_FLAG;
                }
            }
            else if ( (id >= IDM_FINDALL_START + QS_FINDALL_COUNTONLY) && (id < IDM_FINDALL_START + QS_FINDALL_TOTAL_TYPES) )
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
        case WM_KEYDOWN:
        {
            bHotKeyFindAllPressed = FALSE;

            if ( wParam == VK_F3 )
            {
                return OnEditKeyDown_Enter_or_F3(g_QSearchDlg.hFindEdit, wParam, g_Options.dwFlags);
            }
            if ( LOBYTE(wParam) == LOBYTE(g_QSearchDlg.dwHotKeyFindAll) )
            {
                bHotKeyFindAllPressed = qsIsHotKeyPressed(g_QSearchDlg.dwHotKeyFindAll);
                if ( bHotKeyFindAllPressed )
                {
                    if ( !g_Plugin.bOldWindows )
                    {
                        CallPluginFuncW(cszQSearchFindAllW, NULL);
                    }
                    return 0;
                }
            }
            if ( LOBYTE(wParam) == LOBYTE(g_QSearchDlg.dwHotKeyGoToNextFindAllMatch) )
            {
                bHotKeyFindAllPressed = qsIsHotKeyPressed(g_QSearchDlg.dwHotKeyGoToNextFindAllMatch);
                if ( bHotKeyFindAllPressed )
                {
                    if ( doGoToFindAllMatch(GTFAM_NEXT) )
                    {
                        SetFocus(GetWndEdit(g_Plugin.hMainWnd));
                    }
                    return 0;
                }
            }
            if ( LOBYTE(wParam) == LOBYTE(g_QSearchDlg.dwHotKeyGoToPrevFindAllMatch) )
            {
                bHotKeyFindAllPressed = qsIsHotKeyPressed(g_QSearchDlg.dwHotKeyGoToPrevFindAllMatch);
                if ( bHotKeyFindAllPressed )
                {
                    if ( doGoToFindAllMatch(GTFAM_PREV) )
                    {
                        SetFocus(GetWndEdit(g_Plugin.hMainWnd));
                    }
                    return 0;
                }
            }
            if ( wParam == VK_TAB )
            {
                if ( GetKeyState(VK_CONTROL) & 0x80 ) // Ctrl+Tab, Ctrl+Shift+Tab, ...
                {
                    HWND hWndEdit;

                    hWndEdit = GetWndEdit(g_Plugin.hMainWnd);
                    if ( hWndEdit )
                    {
                        SetFocus(hWndEdit);
                        if ( g_Plugin.bOldWindows )
                            PostMessageA(hWndEdit, WM_KEYDOWN, wParam, lParam);
                        else
                            PostMessageW(hWndEdit, WM_KEYDOWN, wParam, lParam);
                        return 1;
                    }
                }
            }
            break;
        }
        //case WM_KEYDOWN:
        //{
        //    bHotKeyQSearchPressed = FALSE;
        //    if ( LOBYTE(wParam) == LOBYTE(g_QSearchDlg.dwHotKeyQSearch) )
        //    {
        //        bHotKeyQSearchPressed = qsIsHotKeyPressed(g_QSearchDlg.dwHotKeyQSearch);
        //        return 0;
        //    }
        //    break;
        //}
        case WM_KEYUP:
        {
            if ( bHotKeyFindAllPressed )
            {
                bHotKeyFindAllPressed = FALSE;
                return 0;
            }

            if ( wParam == VK_TAB )
            {
                if ( GetKeyState(VK_CONTROL) & 0x80 ) // Ctrl+Tab, Ctrl+Shift+Tab, ...
                {
                    return 1;
                }
            }
            break;
        }
        //case WM_KEYUP:
        //{
        //    if ( bHotKeyQSearchPressed )
        //    {
        //        bHotKeyQSearchPressed = FALSE;
        //        if ( !qs_bHotKeyQSearchPressedOnShow )
        //        {
        //            if ( g_QSearchDlg.hFindEdit )
        //            {
        //                SetFocus(g_QSearchDlg.hFindEdit);
        //            }
        //            return 0;
        //        }
        //    }
        //
        //    qs_bHotKeyQSearchPressedOnShow = FALSE;
        //    break;
        //}
        case WM_SYSKEYDOWN:
        {
            if ( wParam == VK_F3 ) // Alt+F3
            {
                return OnEditKeyDown_Enter_or_F3(g_QSearchDlg.hFindEdit, wParam, g_Options.dwFlags);
            }
            if ( LOBYTE(wParam) == LOBYTE(g_QSearchDlg.dwHotKeyFindAll) )
            {
                bHotKeyFindAllPressed = qsIsHotKeyPressed(g_QSearchDlg.dwHotKeyFindAll);
                if ( bHotKeyFindAllPressed )
                {
                    if ( !g_Plugin.bOldWindows )
                    {
                        CallPluginFuncW(cszQSearchFindAllW, NULL);
                    }
                    return 0;
                }
            }
            if ( LOBYTE(wParam) == LOBYTE(g_QSearchDlg.dwHotKeyGoToNextFindAllMatch) )
            {
                bHotKeyFindAllPressed = qsIsHotKeyPressed(g_QSearchDlg.dwHotKeyGoToNextFindAllMatch);
                if ( bHotKeyFindAllPressed )
                {
                    if ( doGoToFindAllMatch(GTFAM_NEXT) )
                    {
                        SetFocus(GetWndEdit(g_Plugin.hMainWnd));
                    }
                    return 0;
                }
            }
            if ( LOBYTE(wParam) == LOBYTE(g_QSearchDlg.dwHotKeyGoToPrevFindAllMatch) )
            {
                bHotKeyFindAllPressed = qsIsHotKeyPressed(g_QSearchDlg.dwHotKeyGoToPrevFindAllMatch);
                if ( bHotKeyFindAllPressed )
                {
                    if ( doGoToFindAllMatch(GTFAM_PREV) )
                    {
                        SetFocus(GetWndEdit(g_Plugin.hMainWnd));
                    }
                    return 0;
                }
            }
            break;
        }
        case WM_SYSKEYUP:
        {
            if ( bHotKeyFindAllPressed )
            {
                bHotKeyFindAllPressed = FALSE;
                return 0;
            }

        //    if ( bHotKeyQSearchPressed )
        //    {
        //        SendMessage(hDlg, WM_KEYUP, wParam, lParam);
        //        return 0;
        //    }
            break;
        }
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
                g_QSearchDlg.hFindEdit = qsearchGetFindEdit(hDlg, &g_QSearchDlg.hFindListBox);
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
                HBRUSH   hBkBrush;
                COLORREF crBkColor;
                COLORREF crTextColor;

                if ( qs_bEditIsActive || qs_bBtnFindIsFocused )
                {
                    if ( !qs_bEditNotFound )
                    {
                        if ( (!qs_nEditIsEOF) || (g_Options.colorEOF == RGB(0xFF,0xFF,0xFF)) )
                        {
                            // normal active
                            if ( g_QSearchDlg.hBkgndBrush )
                            {
                                hBkBrush = g_QSearchDlg.hBkgndBrush;
                                crBkColor = g_QSearchDlg.crBkgndColor;
                                crTextColor = g_QSearchDlg.crTextColor;
                            }
                            else
                            {
                                hBkBrush = GetSysColorBrush(COLOR_WINDOW);
                                crBkColor = GetSysColor(COLOR_WINDOW);
                                crTextColor = GetSysColor(COLOR_WINDOWTEXT);
                            }
                        }
                        else
                        {
                            // EOF active
                            if ( hTextEOFBrush )
                                hBkBrush = hTextEOFBrush;
                            else if ( g_QSearchDlg.hBkgndBrush )
                                hBkBrush = g_QSearchDlg.hBkgndBrush;
                            else
                                hBkBrush = GetSysColorBrush(COLOR_WINDOW);
                            crBkColor = g_Options.colorEOF;
                            crTextColor = g_QSearchDlg.crTextColor;
                        }
                    }
                    else // not found
                    {
                        if ( !qs_bEditNotRegExp )
                        {
                            // not found active
                            hBkBrush = hTextNotFoundBrush;
                            crBkColor = g_Options.colorNotFound;
                            crTextColor = g_QSearchDlg.crTextColor;
                        }
                        else
                        {
                            // not regexp
                            hBkBrush = hTextNotRegExpBrush;
                            crBkColor = g_Options.colorNotRegExp;
                            crTextColor = g_QSearchDlg.crTextColor;
                        }
                    }
                }
                else // not active
                {
                    // normal inactive
                    hBkBrush = GetSysColorBrush(COLOR_BTNFACE);
                    crBkColor = GetSysColor(COLOR_BTNFACE);
                    crTextColor = GetSysColor(COLOR_WINDOWTEXT);
                }

                SetTextColor( (HDC) wParam, crTextColor );
                SetBkMode( (HDC) wParam, TRANSPARENT );
                SetBkColor( (HDC) wParam, crBkColor );
                return (LRESULT) hBkBrush;
            }
            break;
        }
        case WM_CTLCOLORLISTBOX:
        {
            if ( g_QSearchDlg.hFindListBox == (HWND) lParam )
            {
                SetTextColor( (HDC) wParam, g_QSearchDlg.hBkgndBrush ? g_QSearchDlg.crTextColor : GetSysColor(COLOR_WINDOWTEXT) );
                SetBkMode( (HDC) wParam, TRANSPARENT );
                SetBkColor( (HDC) wParam, g_QSearchDlg.hBkgndBrush ? g_QSearchDlg.crBkgndColor : GetSysColor(COLOR_WINDOW) );
                return (LRESULT) ( g_QSearchDlg.hBkgndBrush ? g_QSearchDlg.hBkgndBrush : GetSysColorBrush(COLOR_WINDOW) );
            }
            break;
        }
        case WM_SHOWWINDOW:
        {
            if ( wParam )
            {
                HWND hChHighlightAll;
                BOOL bHighlightAll;

                g_bHighlightPlugin = IsHighlightMainActive();

                hChHighlightAll = GetDlgItem(hDlg, IDC_CH_HIGHLIGHTALL);
                EnableWindow( hChHighlightAll, g_bHighlightPlugin );
                bHighlightAll = ((g_Options.dwHighlightState & HLS_IS_CHECKED) == HLS_IS_CHECKED) ? TRUE : FALSE;
                g_Options.dwFlags[OPTF_SRCH_HIGHLIGHTALL] = bHighlightAll;
                SendMessage( hChHighlightAll, BM_SETCHECK, (bHighlightAll ? BST_CHECKED : BST_UNCHECKED), 0 );

                g_QSearchDlg.dwHotKeyQSearch = getQSearchHotKey();
                g_QSearchDlg.dwHotKeyFindAll = getFindAllHotKey();
                g_QSearchDlg.dwHotKeyGoToNextFindAllMatch = getGoToNextFindAllMatchHotKey();
                g_QSearchDlg.dwHotKeyGoToPrevFindAllMatch = getGoToPrevFindAllMatchHotKey();
                qs_bHotKeyQSearchPressedOnShow = qsIsHotKeyPressed(g_QSearchDlg.dwHotKeyQSearch);

                if ( g_QSearchDlg.uWmShowFlags & QS_SF_CANPICKUPSELTEXT )
                {
                    if ( g_Options.dwFlags[OPTF_SRCH_PICKUP_SELECTION] & 0x01 )
                    {
                        if ( getAkelPadSelectedText(g_QSearchDlg.szFindTextAW, g_Options.dwFlags) )
                        {
                            if ( g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE] )
                            {
                                qs_bForceFindFirst = FALSE;
                                qsearchDoTryHighlightAll(hDlg, g_QSearchDlg.szFindTextAW, g_Options.dwFlags, QHC_CHECKBOX_CHECKED);
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

                if ( !g_bLogPlugin )
                    EnableMenuItem( g_QSearchDlg.hFindAllPopupMenu, IDM_FINDALL_LOGOUTPUT, MF_BYCOMMAND | MF_DISABLED );

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
                        case OPTF_HOTKEY_HIDES_PANEL:
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
            tQSFindAll* pqsfa;
            tQSFindAll qsfa;
            DWORD dwOptFlagsTemp[OPTF_COUNT_TOTAL];

            dwSearch = 0;
            if ( lParam & QS_FF_NOSETSELFIRST )
            {
                dwSearch |= QSEARCH_NOSETSEL_FIRST;
            }
            if ( lParam & QS_FF_NOFINDUP_VK )
            {
                dwSearch |= QSEARCH_NOFINDUP_VK;
            }
            if ( lParam & QS_FF_NOHISTORYUPD )
            {
                dwSearch |= QSEARCH_NOHISTORYUPD;
            }

            if ( uMsg == QSM_FINDALL )
            {
                dwFindAllMode = (DWORD) (wParam & QS_FINDALL_MASK);

                x_zero_mem(&qsfa, sizeof(tQSFindAll));
                tDynamicBuffer_Init(&qsfa.tempBuf);
                tDynamicBuffer_Init(&qsfa.tempBuf2);
                if ( (dwFindAllMode == QS_FINDALL_LOGOUTPUT) ||
                     (g_Options.dwNewUI != QS_UI_NEW_02) )
                {
                    // LogOutput
                    dwSearch |= QSEARCH_FINDALL;
                    if ( wParam & QS_FINDALL_RSLT_ALLFILES )
                        dwSearch |= QSEARCH_FINDALLFILES;
                    qsfa.pfnFindResultCallback = qsFindResultCallback;
                    qsfa.GetFindResultPolicy.nMode = g_Options.LogOutputFRP.nMode;
                    qsfa.GetFindResultPolicy.nBefore = g_Options.LogOutputFRP.nBefore;
                    qsfa.GetFindResultPolicy.nAfter = g_Options.LogOutputFRP.nAfter;
                    qsfa.GetFindResultPolicy.nMaxLineLen = 0;
                    qsfa.GetFindResultPolicy.pfnStoreResultCallback = qsStoreResultCallback;
                    qsfa.ShowFindResults.pfnInit = qsShowFindResults_LogOutput_Init;
                    qsfa.ShowFindResults.pfnAddOccurrence = qsShowFindResults_LogOutput_AddOccurrence;
                    qsfa.ShowFindResults.pfnDone = qsShowFindResults_LogOutput_Done;
                    qsfa.ShowFindResults.pfnAllFilesInit = qsShowFindResults_LogOutput_AllFiles_Init;
                    qsfa.ShowFindResults.pfnAllFilesDone = qsShowFindResults_LogOutput_AllFiles_Done;
                }
                else if ( dwFindAllMode == QS_FINDALL_FILEOUTPUT_MULT ||
                          dwFindAllMode == QS_FINDALL_FILEOUTPUT_SNGL )
                {
                    // FileOutput
                    dwSearch |= QSEARCH_FINDALL;
                    if ( wParam & QS_FINDALL_RSLT_ALLFILES )
                        dwSearch |= QSEARCH_FINDALLFILES;
                    qsfa.pfnFindResultCallback = qsFindResultCallback;
                    qsfa.GetFindResultPolicy.nMode = g_Options.FileOutputFRP.nMode;
                    qsfa.GetFindResultPolicy.nBefore = g_Options.FileOutputFRP.nBefore;
                    qsfa.GetFindResultPolicy.nAfter = g_Options.FileOutputFRP.nAfter;
                    qsfa.GetFindResultPolicy.nMaxLineLen = 0;
                    qsfa.GetFindResultPolicy.pfnStoreResultCallback = qsStoreResultCallback;
                    qsfa.ShowFindResults.pfnInit = qsShowFindResults_FileOutput_Init;
                    qsfa.ShowFindResults.pfnAddOccurrence = qsShowFindResults_FileOutput_AddOccurrence;
                    qsfa.ShowFindResults.pfnDone = qsShowFindResults_FileOutput_Done;
                    qsfa.ShowFindResults.pfnAllFilesInit = qsShowFindResults_FileOutput_AllFiles_Init;
                    qsfa.ShowFindResults.pfnAllFilesDone = qsShowFindResults_FileOutput_AllFiles_Done;
                }
                else
                {
                    // CountOnly
                    dwSearch |= QSEARCH_COUNTALL;
                    qsfa.pfnFindResultCallback = NULL;
                    qsfa.ShowFindResults.pfnInit = qsShowFindResults_CountOnly_Init;
                    qsfa.ShowFindResults.pfnAddOccurrence = qsShowFindResults_CountOnly_AddOccurrence;
                    qsfa.ShowFindResults.pfnDone = qsShowFindResults_CountOnly_Done;
                    qsfa.ShowFindResults.pfnAllFilesInit = qsShowFindResults_CountOnly_AllFiles_Init;
                    qsfa.ShowFindResults.pfnAllFilesDone = qsShowFindResults_CountOnly_AllFiles_Done;
                }
                pqsfa = &qsfa;
            }
            else
            {
                dwFindAllMode = 0;
                pqsfa = NULL;
            }

            copyOptionsFlags(dwOptFlagsTemp, g_Options.dwFlags);
            dwOptFlagsTemp[OPTF_SRCH_FROM_BEGINNING] = 0; // disabling here

            if ( (g_Options.dwFlags[OPTF_SRCH_PICKUP_SELECTION] & 0x01) &&
                 (g_QSearchDlg.uSearchOrigin == QS_SO_EDITOR) &&
                 (!(lParam & QS_FF_NOPICKUPSEL)) )
            {
                getEditFindText( g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextAW );
                qsearchFindHistoryAdd( g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextAW, 0 );
                qsPickUpSelection( g_QSearchDlg.hFindEdit, dwOptFlagsTemp, FALSE );
                dwSearch |= QSEARCH_NOSETSEL_FIRST;
            }
            getEditFindText( g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextAW );
            if ( qs_bForceFindFirst || qs_bEditTextChanged )
            {
                qs_bForceFindFirst = FALSE;
                qs_bEditTextChanged = FALSE;
                dwSearch |= (QSEARCH_FIRST | QSEARCH_NOFINDBEGIN_VK);
                if ( wParam )
                    dwSearch |= QSEARCH_FINDUP;
                qsearchDoSearchText( g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextAW, dwSearch, dwOptFlagsTemp, pqsfa );
                qsearchDoTryHighlightAll( hDlg, g_QSearchDlg.szFindTextAW, dwOptFlagsTemp,
                    QHC_CHECKBOX_CHECKED | QHC_FINDFIRST | QHC_IGNORE_SELECTION | (dwFindAllMode != QS_FINDALL_COUNTONLY ? QHC_DONT_CUT_REGEXP : 0) );
            }
            else
            {
                BOOL bPrevNotFound;

                bPrevNotFound = qs_bEditNotFound;
                dwSearch |= QSEARCH_NEXT;
                if ( wParam )
                    dwSearch |= QSEARCH_FINDUP;
                qsearchDoSearchText( g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextAW, dwSearch, dwOptFlagsTemp, pqsfa );
                if ( g_bFrameActivated || (bPrevNotFound && !qs_bEditNotFound) ||
                     (g_Options.dwFlags[OPTF_SRCH_USE_REGEXP] && dwFindAllMode != QS_FINDALL_COUNTONLY) ||
                     !QSearchDlgState_isLastHighlightedEqualToTheSearch(&g_QSearchDlg, g_QSearchDlg.szFindTextAW, getFindAllFlags(g_Options.dwFlags)) )
                {
                    g_bFrameActivated = FALSE;
                    qsearchDoTryHighlightAll( hDlg, g_QSearchDlg.szFindTextAW, dwOptFlagsTemp,
                        QHC_CHECKBOX_CHECKED | QHC_DONT_CUT_REGEXP | QHC_IGNORE_SELECTION );
                }
            }
            return 1;
        }
        case QSM_FINDFIRST:
        {
            DWORD dwSearch = QSEARCH_FIRST;

            if ( wParam & QSEARCH_NOFINDUP_VK )
                dwSearch |= QSEARCH_NOFINDUP_VK;

            if ( wParam & QSEARCH_NOFINDBEGIN_VK )
                dwSearch |= QSEARCH_NOFINDBEGIN_VK;

            if ( wParam & QSEARCH_USEDELAY )
                dwSearch |= QSEARCH_USEDELAY;

            if ( wParam & QSEARCH_NOHISTORYUPD )
                dwSearch |= QSEARCH_NOHISTORYUPD;

            getEditFindText( g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextAW );
            qsearchDoSearchText( g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextAW, dwSearch, g_Options.dwFlags, NULL );
            qsearchDoTryHighlightAll( hDlg, g_QSearchDlg.szFindTextAW, g_Options.dwFlags, QHC_CHECKBOX_CHECKED | QHC_FINDFIRST );
            return 1;
        }
        case QSM_SELFIND:
        {
            qsearchDoSelFind( g_QSearchDlg.hFindEdit, (BOOL) wParam, g_Options.dwFlags );
            return 1;
        }
        case QSM_PICKUPSELTEXT:
        {
            UINT nPickedUp;
            if ( wParam & QS_PS_UPDATEHISTORY )
            {
                getEditFindText( g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextAW );
                qsearchFindHistoryAdd( g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextAW, 0 );
            }
            nPickedUp = qsPickUpSelection(g_QSearchDlg.hFindEdit, g_Options.dwFlags, FALSE);
            if ( lParam )
                *((UINT *)lParam) = nPickedUp; // see QS_PSF_*
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

            uFlags = (UINT) lParam;
            qsearchDoSetNotFound( g_QSearchDlg.hFindEdit, (BOOL) wParam, FALSE, (uFlags & QS_SNF_IGNOREEOF) ? QSEARCH_EOF_IGNORE : 0 );

            if ( uFlags & QS_SNF_SETINFOEMPTY )
            {
                qsSetInfoEmpty_Tracking("qsearchDlgProc, QSM_SETNOTFOUND");
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
            BOOL bHighlightAll;

            g_bHighlightPlugin = IsHighlightMainActive();

            hChHighlightAll = GetDlgItem(hDlg, IDC_CH_HIGHLIGHTALL);
            EnableWindow( hChHighlightAll, g_bHighlightPlugin );
            bHighlightAll = ((g_Options.dwHighlightState & HLS_IS_CHECKED) == HLS_IS_CHECKED) ? TRUE : FALSE;
            g_Options.dwFlags[OPTF_SRCH_HIGHLIGHTALL] = bHighlightAll;
            SendMessage( hChHighlightAll, BM_SETCHECK, (bHighlightAll ? BST_CHECKED : BST_UNCHECKED), 0 );

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
                g_QSearchDlg.hFindEdit = qsearchGetFindEdit(hDlg, &g_QSearchDlg.hFindListBox);
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
            //getEditFindText( g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextAW );
            //if ( g_Options.dwFlags[OPTF_SRCH_ONTHEFLY_MODE] )
            //{
            //    qsearchDoTryHighlightAll( hDlg, g_Options.dwFlags, QHC_CHECKBOX_CHECKED );
            //}
            return 1;
        }
        case WM_INITDIALOG:
        {
            HWND hDlgItm;
            BOOL bHighlightAll;

            InitializeCriticalSection(&csFindAllTimerId);

            GetClientRect(hDlg, &rcDlg_0);
            hEdFindText = GetDlgItemAndRect(hDlg, IDC_ED_FINDTEXT, &rcEdFindText_0);
            hCbFindText = GetDlgItemAndRect(hDlg, IDC_CB_FINDTEXT, &rcCbFindText_0);
            hBtFindNext = GetDlgItemAndRect(hDlg, IDC_BT_FINDNEXT, &rcBtFindNext_0);
            hBtFindPrev = GetDlgItemAndRect(hDlg, IDC_BT_FINDPREV, &rcBtFindPrev_0);
            hBtFindAll = GetDlgItemAndRect(hDlg, IDC_BT_FINDALL, &rcBtFindAll_0);
            hBtCancel = GetDlgItem(hDlg, IDC_BT_CANCEL);
            hPbProgress = GetDlgItemAndRect(hDlg, IDC_PB_PROGRESS, &rcPbProgress_0);
            hChMatchCase = GetDlgItemAndRect(hDlg, IDC_CH_MATCHCASE, &rcChMatchCase_0);
            hChWholeWord = GetDlgItemAndRect(hDlg, IDC_CH_WHOLEWORD, &rcChWholeWord_0);
            hChHighlightAll = GetDlgItemAndRect(hDlg, IDC_CH_HIGHLIGHTALL, &rcChHighlightAll_0);
            hStInfo = GetDlgItemAndRect(hDlg, IDC_ST_INFO, &rcStInfo_0);

            g_QSearchDlg.bIsQSearchingRightNow = FALSE;
            g_QSearchDlg.bMouseJustLeavedFindEdit = FALSE;
            g_QSearchDlg.hDlg = hDlg;
            g_QSearchDlg.hFindEdit = qsearchGetFindEdit(hDlg, &g_QSearchDlg.hFindListBox);

            if ( hChMatchCase )
            {
                SendMessage( hChMatchCase, BM_SETCHECK,
                  g_Options.dwFlags[OPTF_SRCH_MATCHCASE] ? BST_CHECKED : BST_UNCHECKED, 0 );
            }

            // restoring initial values of static vars
            qs_bEditIsActive = TRUE;
            qs_bEditCanBeNonActive = TRUE;
            qs_bEditSelJustChanged = FALSE;
            qs_bEditNotFound = FALSE;
            qs_bEditNotRegExp = FALSE;
            qs_nEditIsEOF = 0;
            qs_nEditEOF = 0;
            qs_bEditTextChanged = TRUE;
            qs_bHotKeyQSearchPressedOnShow = FALSE;
            qs_bBtnFindIsFocused = FALSE;
            g_QSearchDlg.dwHotKeyQSearch = 0;
            g_QSearchDlg.dwHotKeyFindAll = getFindAllHotKey();
            g_QSearchDlg.dwHotKeyGoToNextFindAllMatch = getGoToNextFindAllMatchHotKey();
            g_QSearchDlg.dwHotKeyGoToPrevFindAllMatch = getGoToPrevFindAllMatchHotKey();

            qsearchDlgApplyEditorColors();

            hTextNotFoundBrush = CreateSolidBrush(g_Options.colorNotFound);
            hTextNotRegExpBrush = CreateSolidBrush(g_Options.colorNotRegExp);
            if ( g_Options.colorEOF != RGB(0xFF,0xFF,0xFF) )
            {
                hTextEOFBrush = CreateSolidBrush(g_Options.colorEOF);
            }

            if ( qsearchIsFindHistoryEnabled() )
                hDlgItm = hCbFindText;
            else
                hDlgItm = hEdFindText;
            ShowWindow(hDlgItm, SW_SHOWNORMAL);

            SendMessage(g_QSearchDlg.hFindEdit, EM_SETLIMITTEXT, MAX_TEXT_SIZE - 1, 0);
            prev_editWndProc = setWndProc(g_QSearchDlg.hFindEdit, editWndProc);
            if ( hBtCancel )
            {
                BUTTONDRAW bd;

                bd.dwFlags = BIF_CROSS;
                bd.hImage = NULL;
                bd.nImageWidth = 0;
                bd.nImageHeight = 0;
                SendMessage(g_Plugin.hMainWnd, AKD_SETBUTTONDRAW, (WPARAM)hBtCancel, (LPARAM)&bd);

                /* prev_btnWndProc = setWndProc(hBtCancel, btnWndProc); */
            }
            if ( hChWholeWord )
            {
                prev_chWholeWordWndProc = setWndProc(hChWholeWord, chWholeWordWndProc);
            }

            g_QSearchDlg.hBtnFindNext = hBtFindNext;
            if ( g_QSearchDlg.hBtnFindNext )
            {
                prev_btnFindNextWndProc = setWndProc(g_QSearchDlg.hBtnFindNext, btnFindWndProc);
            }
            g_QSearchDlg.hBtnFindPrev = hBtFindPrev;
            if ( g_QSearchDlg.hBtnFindPrev )
            {
                prev_btnFindPrevWndProc = setWndProc(g_QSearchDlg.hBtnFindPrev, btnFindWndProc);
            }
            g_QSearchDlg.hBtnFindAll = hBtFindAll;
            if ( g_QSearchDlg.hBtnFindAll )
            {
                prev_btnFindAllWndProc = setWndProc(g_QSearchDlg.hBtnFindAll, btnFindWndProc);
            }
            g_QSearchDlg.hStInfo = hStInfo;

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
                getAkelPadSelectedText(g_QSearchDlg.szFindTextAW, g_Options.dwFlags);
            }
            setEditFindText(g_QSearchDlg.hFindEdit, g_QSearchDlg.szFindTextAW);
            //SetFocus(g_QSearchDlg.hFindEdit);
            if ( !g_Options.dwFlags[OPTF_EDIT_FOCUS_SELECTALL] )
            {
                SendMessage(g_QSearchDlg.hFindEdit, EM_SETSEL, 0, -1);
                #ifdef _DEBUG
                  Debug_OutputA("qsearchDlgProc, WM_INITDIALOG, if (!SelectAll), SETSEL(0, -1)\n");
                #endif
            }

            g_bHighlightPlugin = IsHighlightMainActive();

            EnableWindow( hChHighlightAll, g_bHighlightPlugin );
            bHighlightAll = ((g_Options.dwHighlightState & HLS_IS_CHECKED) == HLS_IS_CHECKED) ? TRUE : FALSE;
            g_Options.dwFlags[OPTF_SRCH_HIGHLIGHTALL] = bHighlightAll;
            SendMessage( hChHighlightAll, BM_SETCHECK, (bHighlightAll ? BST_CHECKED : BST_UNCHECKED), 0 );

            break;
        }
        default:
            break;
    }

    // Dialog resize messages
    switch ( uMsg )
    {
        case WM_CREATE:
        case WM_INITDIALOG:
        case WM_GETMINMAXINFO:
        case WM_WINDOWPOSCHANGING:
        case WM_MOVE:
        case WM_SIZE:
        case WM_PAINT:
        {
            RECT rcDlgCurr;

            if ( GetClientRect(hDlg, &rcDlgCurr) )
            {
                LONG nWidthInit;
                LONG nWidthCurr;
                LONG dx;

                nWidthCurr = rcDlgCurr.right - rcDlgCurr.left;
                nWidthInit = rcDlg_0.right - rcDlg_0.left;
                dx = nWidthCurr - nWidthInit;

                if ( dx < 0 )
                {
                    if ( g_Options.dwEditMinWidth != 0 )
                    {
                        if ( rcEdFindText_0.right - rcEdFindText_0.left + dx < (LONG) g_Options.dwEditMinWidth )
                            dx = rcEdFindText_0.left + g_Options.dwEditMinWidth - rcEdFindText_0.right;
                    }
                    else
                        dx = 0;
                }
                else // dx >= 0
                {
                    if ( g_Options.dwEditMaxWidth != 0 )
                    {
                        if ( rcEdFindText_0.right - rcEdFindText_0.left + dx > (LONG) g_Options.dwEditMaxWidth )
                            dx = rcEdFindText_0.left + g_Options.dwEditMaxWidth - rcEdFindText_0.right;
                    }
                }

                MoveWindowByDx(hStInfo, &rcStInfo_0, dx);
                MoveWindowByDx(hChHighlightAll, &rcChHighlightAll_0, dx);
                MoveWindowByDx(hChWholeWord, &rcChWholeWord_0, dx);
                MoveWindowByDx(hChMatchCase, &rcChMatchCase_0, dx);
                MoveWindowByDx(hPbProgress, &rcPbProgress_0, dx);
                MoveWindowByDx(hBtFindAll, &rcBtFindAll_0, dx);
                MoveWindowByDx(hBtFindPrev, &rcBtFindPrev_0, dx);
                MoveWindowByDx(hBtFindNext, &rcBtFindNext_0, dx);
                ResizeWindowByDx(hCbFindText, &rcCbFindText_0, dx);
                ResizeWindowByDx(hEdFindText, &rcEdFindText_0, dx);

                RedrawWindowByDx(hEdFindText);
                RedrawWindowByDx(hCbFindText);
                RedrawWindowByDx(hBtFindNext);
                RedrawWindowByDx(hBtFindPrev);
                RedrawWindowByDx(hBtFindAll);
                RedrawWindowByDx(hPbProgress);
                RedrawWindowByDx(hChMatchCase);
                RedrawWindowByDx(hChWholeWord);
                RedrawWindowByDx(hChHighlightAll);
                RedrawWindowByDx(hStInfo);

                if ( g_QSearchDlg.pDockData )
                {
                    GetClientRect(hDlg, &g_QSearchDlg.pDockData->rcDragDrop);
                }

                if ( g_QSearchDlg.hFindEdit )
                {
                    if ( g_Plugin.bOldWindows )
                    {
                        TOOLINFOA tiA;

                        fillToolInfoA( &tiA, LPSTR_TEXTCALLBACKA, g_QSearchDlg.hFindEdit, IDC_ED_FINDTEXT );
                        // LPSTR_TEXTCALLBACKA means "send TTN_GETDISPINFOA to hEdit"
                        SendMessage( hToolTip, TTM_NEWTOOLRECTA, 0, (LPARAM) &tiA );
                    }
                    else
                    {
                        TOOLINFOW tiW;

                        fillToolInfoW( &tiW, LPSTR_TEXTCALLBACKW, g_QSearchDlg.hFindEdit, IDC_ED_FINDTEXT );
                        // LPSTR_TEXTCALLBACKW means "send TTN_GETDISPINFOW to hEdit"
                        SendMessageW( hToolTip, TTM_NEWTOOLRECTW, 0, (LPARAM) &tiW );
                    }
                }
            }
            break;
        }
    }

    return 0;
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

            fillToolInfoA( &tiA, LPSTR_TEXTCALLBACKA, hEdit, IDC_ED_FINDTEXT );
            // LPSTR_TEXTCALLBACKA means "send TTN_GETDISPINFOA to hEdit"
            SendMessage( hToolTip, TTM_ADDTOOLA, 0, (LPARAM) &tiA );

            lstrcpyA( szHintA, qsearchGetHintA(IDC_BT_CANCEL) );
            fillToolInfoA( &tiA, szHintA, GetDlgItem(hDlg, IDC_BT_CANCEL), IDC_BT_CANCEL );
            SendMessage( hToolTip, TTM_ADDTOOLA, 0, (LPARAM) &tiA );

            lstrcpyA( szHintA, qsearchGetHintA(IDC_CH_MATCHCASE) );
            if ( g_Options.dwUseAltHotkeys )
            {
                strAppendAltHotkeyA( szHintA, g_Options.dwAltMatchCase );
            }
            fillToolInfoA( &tiA, szHintA, GetDlgItem(hDlg, IDC_CH_MATCHCASE), IDC_CH_MATCHCASE );
            SendMessage( hToolTip, TTM_ADDTOOLA, 0, (LPARAM) &tiA );

            fillToolInfoA( &tiA, LPSTR_TEXTCALLBACKA, GetDlgItem(hDlg, IDC_CH_WHOLEWORD), IDC_CH_WHOLEWORD );
            // LPSTR_TEXTCALLBACKA means "send TTN_GETDISPINFOA to hCheckbox"
            SendMessage( hToolTip, TTM_ADDTOOLA, 0, (LPARAM) &tiA );

            lstrcpyA( szHintA, qsearchGetHintA(IDC_CH_HIGHLIGHTALL) );
            if ( g_Options.dwUseAltHotkeys )
            {
                strAppendAltHotkeyA( szHintA, g_Options.dwAltHighlightAll );
            }
            fillToolInfoA( &tiA, szHintA, GetDlgItem(hDlg, IDC_CH_HIGHLIGHTALL), IDC_CH_HIGHLIGHTALL );
            SendMessage( hToolTip, TTM_ADDTOOLA, 0, (LPARAM) &tiA );

            if ( g_Options.dwNewUI == QS_UI_NEW_01 ||
                 g_Options.dwNewUI == QS_UI_NEW_02 )
            {
                fillToolInfoA( &tiA, (LPSTR) qsearchGetHintA(IDC_BT_FINDNEXT),
                  GetDlgItem(hDlg, IDC_BT_FINDNEXT), IDC_BT_FINDNEXT );
                SendMessage( hToolTip, TTM_ADDTOOLA, 0, (LPARAM) &tiA );

                fillToolInfoA( &tiA, (LPSTR) qsearchGetHintA(IDC_BT_FINDPREV),
                  GetDlgItem(hDlg, IDC_BT_FINDPREV), IDC_BT_FINDPREV );
                SendMessage( hToolTip, TTM_ADDTOOLA, 0, (LPARAM) &tiA );

                if ( g_Options.dwNewUI == QS_UI_NEW_02 )
                {
                    fillToolInfoA( &tiA, LPSTR_TEXTCALLBACKA,
                      GetDlgItem(hDlg, IDC_BT_FINDALL), IDC_BT_FINDALL );
                    SendMessage( hToolTip, TTM_ADDTOOLA, 0, (LPARAM) &tiA );
                }
            }
        }
        else
        {
            TOOLINFOW tiW;
            wchar_t szHintW[128];

            fillToolInfoW( &tiW, LPSTR_TEXTCALLBACKW, hEdit, IDC_ED_FINDTEXT );
            // LPSTR_TEXTCALLBACKW means "send TTN_GETDISPINFOW to hEdit"
            SendMessageW( hToolTip, TTM_ADDTOOLW, 0, (LPARAM) &tiW );

            lstrcpyW( szHintW, qsearchGetHintW(IDC_BT_CANCEL) );
            fillToolInfoW( &tiW, szHintW, GetDlgItem(hDlg, IDC_BT_CANCEL), IDC_BT_CANCEL );
            SendMessageW( hToolTip, TTM_ADDTOOLW, 0, (LPARAM) &tiW );

            lstrcpyW( szHintW, qsearchGetHintW(IDC_CH_MATCHCASE) );
            if ( g_Options.dwUseAltHotkeys )
            {
                strAppendAltHotkeyW( szHintW, g_Options.dwAltMatchCase );
            }
            fillToolInfoW( &tiW, szHintW, GetDlgItem(hDlg, IDC_CH_MATCHCASE), IDC_CH_MATCHCASE );
            SendMessageW( hToolTip, TTM_ADDTOOLW, 0, (LPARAM) &tiW );

            fillToolInfoW( &tiW, LPSTR_TEXTCALLBACKW, GetDlgItem(hDlg, IDC_CH_WHOLEWORD), IDC_CH_WHOLEWORD );
            // LPSTR_TEXTCALLBACKW means "send TTN_GETDISPINFOW to hCheckbox"
            SendMessageW( hToolTip, TTM_ADDTOOLW, 0, (LPARAM) &tiW );

            lstrcpyW( szHintW, qsearchGetHintW(IDC_CH_HIGHLIGHTALL) );
            if ( g_Options.dwUseAltHotkeys )
            {
                strAppendAltHotkeyW( szHintW, g_Options.dwAltHighlightAll );
            }
            fillToolInfoW( &tiW, szHintW, GetDlgItem(hDlg, IDC_CH_HIGHLIGHTALL), IDC_CH_HIGHLIGHTALL );
            SendMessageW( hToolTip, TTM_ADDTOOLW, 0, (LPARAM) &tiW );

            if ( g_Options.dwNewUI == QS_UI_NEW_01 ||
                 g_Options.dwNewUI == QS_UI_NEW_02 )
            {
                fillToolInfoW( &tiW, (LPWSTR) qsearchGetHintW(IDC_BT_FINDNEXT),
                  GetDlgItem(hDlg, IDC_BT_FINDNEXT), IDC_BT_FINDNEXT );
                SendMessageW( hToolTip, TTM_ADDTOOLW, 0, (LPARAM) &tiW );

                fillToolInfoW( &tiW, (LPWSTR) qsearchGetHintW(IDC_BT_FINDPREV),
                  GetDlgItem(hDlg, IDC_BT_FINDPREV), IDC_BT_FINDPREV );
                SendMessageW( hToolTip, TTM_ADDTOOLW, 0, (LPARAM) &tiW );

                if ( g_Options.dwNewUI == QS_UI_NEW_02 )
                {
                    fillToolInfoW( &tiW, LPSTR_TEXTCALLBACKW,
                      GetDlgItem(hDlg, IDC_BT_FINDALL), IDC_BT_FINDALL );
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

    getEditFindText(hEdit, g_QSearchDlg.szFindTextAW);

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
    if ( g_QSearchDlg.hBkgndBrush )
    {
        DeleteObject(g_QSearchDlg.hBkgndBrush);
        g_QSearchDlg.hBkgndBrush = NULL;
    }
    if ( hPopupMenuLoaded )
    {
        DestroyMenu(hPopupMenuLoaded);
    }
    DestroyWindow( g_QSearchDlg.hDlg );
    g_QSearchDlg.hDlg = NULL;

    QSearchDlgState_clearCurrentMatches(&g_QSearchDlg, TRUE);
    QSearchDlgState_clearFindAllMatchesAndFrames(&g_QSearchDlg, TRUE);

    if ( !g_Plugin.bAkelPadOnFinish )
    {
        Uninitialize();
        SendMessage(g_Plugin.hMainWnd, AKD_RESIZE, 0, 0);
        SetFocus(g_Plugin.hMainWnd);
    }

    DeleteCriticalSection(&csFindAllTimerId);
}

void qsearchDoSetNotFound(HWND hEdit, BOOL bNotFound, BOOL bNotRegExp, INT nIsEOF)
{
    #ifdef _DEBUG
      Debug_OutputA("%s: bNotFound=%d, bNotRegExp=%d, nIsEOF=%d\n", __func__, bNotFound, bNotRegExp, nIsEOF);
    #endif

    if ( bNotFound )
    {
        qsearchDoTryUnhighlightAll();
        qsSetInfoEmpty_Tracking("qsearchDoSetNotFound, bNotFound");
    }

    qs_bEditNotFound = bNotFound;
    qs_bEditNotRegExp = bNotRegExp;
    if ( !(nIsEOF & QSEARCH_EOF_IGNORE) )
    {
        qs_nEditIsEOF = nIsEOF;
        qsSetInfoEofOrNotFound(nIsEOF, bNotFound, bNotRegExp);
    }
    else if ( bNotFound || bNotRegExp )
    {
        qsSetInfoEofOrNotFound(0, bNotFound, bNotRegExp);
    }
    RedrawWindowEntire(hEdit);
}

void qsearchDoShowHide(HWND hDlg, BOOL bShow, UINT uShowFlags, const DWORD dwOptFlags[])
{
    BOOL bChangeSelection = !IsWindowVisible(hDlg);

    qsearchDoSetNotFound( qsearchGetFindEdit(hDlg, NULL), FALSE, FALSE, 0 );

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
        g_QSearchDlg.dwHotKeyQSearch = getQSearchHotKey();
        */
        qs_bHotKeyQSearchPressedOnShow = qsIsHotKeyPressed(g_QSearchDlg.dwHotKeyQSearch);
    }
    else
    {
        qs_nEditEOF = 0;

        qsSetInfoEmpty_Tracking("qsearchDoShowHide, !bShow");
    }

    g_QSearchDlg.uWmShowFlags = 0; // forbid to pick up selected text on WM_SHOWWINDOW
    SendMessage( g_Plugin.hMainWnd, AKD_DOCK,
      (bShow ? DK_SHOW : DK_HIDE), (LPARAM) g_QSearchDlg.pDockData );
    g_QSearchDlg.uWmShowFlags = 0; // just in case :)

    // Change AkelPad's plugin status (running/not running)
    if ( g_szFunctionQSearchAW[0] )
    {
        if ( g_Plugin.bOldWindows )
        {
            PLUGINFUNCTION* pfA;

            pfA = (PLUGINFUNCTION *) SendMessage( g_Plugin.hMainWnd,
              AKD_DLLFINDA, (WPARAM) g_szFunctionQSearchAW, 0 );
            if ( pfA )
            {
                pfA->bRunning = bShow;
            }
        }
        else
        {
            PLUGINFUNCTION* pfW;

            pfW = (PLUGINFUNCTION *) SendMessage( g_Plugin.hMainWnd,
              AKD_DLLFINDW, (WPARAM) g_szFunctionQSearchAW, 0 );
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
        BOOL bFindTextChanged = FALSE;
        HWND hEdit = qsearchGetFindEdit(hDlg, NULL);

        if ( uShowFlags & QS_SF_CANPICKUPSELTEXT )
        {
            if ( dwOptFlags[OPTF_SRCH_PICKUP_SELECTION] & 0x01 )
            {
                wchar_t prevFindTextAW[MAX_TEXT_SIZE];

                strcpyAorW(prevFindTextAW, g_QSearchDlg.szFindTextAW);

                bGotSelectedText = getAkelPadSelectedText(g_QSearchDlg.szFindTextAW, dwOptFlags);
                if ( bGotSelectedText )
                {
                    bChangeSelection = TRUE;
                    bFindTextChanged = (strcmpAorW(prevFindTextAW, g_QSearchDlg.szFindTextAW, dwOptFlags[OPTF_SRCH_MATCHCASE]) != 0);

                    if ( bFindTextChanged )
                    {
                        qsSetInfoEmpty_Tracking("qsearchDoShowHide, bFindTextChanged");
                    }
                }
                else
                {
                    getEditFindText(hEdit, g_QSearchDlg.szFindTextAW);
                }
                setEditFindText(hEdit, g_QSearchDlg.szFindTextAW);
                if ( (!bChangeSelection) || dwOptFlags[OPTF_EDIT_FOCUS_SELECTALL] )
                {
                    SendMessage(hEdit, EM_SETSEL, 0, -1);
                    #ifdef _DEBUG
                      Debug_OutputA("qsearchDoShowHide, PickUpSel, SETSEL(0, -1)\n");
                    #endif
                    qs_bEditSelJustChanged = TRUE;
                }
            }
        }
        if ( bChangeSelection && !dwOptFlags[OPTF_EDIT_FOCUS_SELECTALL] )
        {
            SendMessage(hEdit, EM_SETSEL, 0, -1);
            #ifdef _DEBUG
              Debug_OutputA("qsearchDoShowHide, ChangeSel, SETSEL(0, -1)\n");
            #endif
            qs_bEditSelJustChanged = TRUE;
        }
        if ( bGotSelectedText )
        {
            if ( dwOptFlags[OPTF_SRCH_ONTHEFLY_MODE] )
            {
                qs_bForceFindFirst = FALSE;
                qsearchDoTryHighlightAll(g_QSearchDlg.hDlg, g_QSearchDlg.szFindTextAW, dwOptFlags, QHC_CHECKBOX_CHECKED);
            }
            else
                qs_bForceFindFirst = TRUE;
        }
        SetFocus(hEdit);
    }
    else
    {
        QSearchDlgState_clearCurrentMatches(&g_QSearchDlg, TRUE);

        //SendMessage(g_Plugin.hMainWnd, AKD_RESIZE, 0, 0);
        SetFocus(g_Plugin.hMainWnd);
    }
}

void qsearchDoSelFind(HWND hEdit, BOOL bFindPrev, const DWORD dwOptFlags[])
{
    DWORD   dwOptFlagsTemp[OPTF_COUNT_TOTAL];
    wchar_t prevFindTextAW[MAX_TEXT_SIZE];
    wchar_t selFindTextAW[MAX_TEXT_SIZE];

    strcpyAorW(prevFindTextAW, g_QSearchDlg.szFindTextAW);

    if ( dwOptFlags[OPTF_SRCH_SELFIND_PICKUP] )
    {
        // getting selected text with current search flags
        if ( getAkelPadSelectedText(g_QSearchDlg.szFindTextAW, dwOptFlags) )
        {
            setEditFindText(hEdit, g_QSearchDlg.szFindTextAW);
        }
    }

    copyOptionsFlags(dwOptFlagsTemp, dwOptFlags);
    // these search flags must be disabled here
    dwOptFlagsTemp[OPTF_SRCH_FROM_BEGINNING] = 0;
    dwOptFlagsTemp[OPTF_SRCH_USE_SPECIALCHARS] = 0;
    dwOptFlagsTemp[OPTF_SRCH_USE_REGEXP] = 0;

    // getting selected text with modified search flags
    if ( getAkelPadSelectedText(selFindTextAW, dwOptFlagsTemp) )
    {
        DWORD  dwSearchParams;

        // clear the "not found" flag
        qs_bEditNotFound = FALSE;
        qs_bEditNotRegExp = FALSE;
        qs_nEditIsEOF = 0;

        if ( strcmpAorW(prevFindTextAW, selFindTextAW, dwOptFlags[OPTF_SRCH_MATCHCASE]) != 0 )
        {
            qs_nEditEOF = 0;
        }

        dwSearchParams = QSEARCH_NEXT | QSEARCH_SEL;
        if ( bFindPrev )  dwSearchParams |= QSEARCH_SEL_FINDUP;
        qsearchDoSearchText(hEdit, selFindTextAW, dwSearchParams, dwOptFlagsTemp, NULL);
    }
}

static void adjustIncompleteRegExA(char* szTextA, const DWORD dwOptFlags[], BOOL bCutTrailingSequence)
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

    if ( bCutTrailingSequence )
    {
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
    }
    else
    {
        if ( szTextA[n1 - 1] == '?' )
            return; // trailing '?', nothing to do
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

    if ( bCutTrailingSequence )
    {
        // exclude trailing '+' or '*'
        szTextA[n1] = 0;
    }
    else
    {
        // adding trailing '?'
        szTextA[n1 + 1] = '?';
        szTextA[n1 + 2] = 0;
    }
}

static void adjustIncompleteRegExW(wchar_t* szTextW, const DWORD dwOptFlags[], BOOL bCutTrailingSequence)
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

    if ( bCutTrailingSequence )
    {
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
    }
    else
    {
        if ( szTextW[n1 - 1] == L'?' )
            return; // trailing '?', nothing to do
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

    if ( bCutTrailingSequence )
    {
        // exclude trailing '+' or '*'
        szTextW[n1] = 0;
    }
    else
    {
        // adding trailing '?'
        szTextW[n1 + 1] = L'?';
        szTextW[n1 + 2] = 0;
    }
}

// pszRegExA may require doubled length of cszFindExA
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

// pszRegExW may require doubled length of cszFindExW
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

// important: pszRegExpA must point to (szSomeTextBufA + 2)
static char* surroundRegExpWithWordBoundariesA(char* pszRegExpA)
{
    int nLen = lstrlenA(pszRegExpA);
    if ( nLen < 2 || pszRegExpA[nLen - 2] != '\\' || pszRegExpA[nLen - 1] != 'b' )
    {
        // the trailing "\\b"
        pszRegExpA[nLen++] = '\\';
        pszRegExpA[nLen++] = 'b';
        pszRegExpA[nLen] = 0;
    }
    if ( pszRegExpA[0] != '\\' || pszRegExpA[1] != 'b' )
    {
        // the leading "\\b"
        pszRegExpA -= 2;
        pszRegExpA[0] = '\\';
        pszRegExpA[1] = 'b';
    }
    return pszRegExpA;
}

// important: pszRegExpW must point to (szSomeTextBufW + 2)
static wchar_t* surroundRegExpWithWordBoundariesW(wchar_t* pszRegExpW)
{
    int nLen = lstrlenW(pszRegExpW);
    if ( nLen < 2 || pszRegExpW[nLen - 2] != L'\\' || pszRegExpW[nLen - 1] != L'b' )
    {
        // the trailing "\\b"
        pszRegExpW[nLen++] = L'\\';
        pszRegExpW[nLen++] = L'b';
        pszRegExpW[nLen] = 0;
    }
    if ( pszRegExpW[0] != L'\\' || pszRegExpW[1] != L'b' )
    {
        // the leading "\\b"
        pszRegExpW -= 2;
        pszRegExpW[0] = L'\\';
        pszRegExpW[1] = L'b';
    }
    return pszRegExpW;
}

void qsearchDoSearchText(HWND hEdit, const wchar_t* cszFindWhatAW, DWORD dwParams, const DWORD dwOptFlags[], tQSFindAll* pFindAll)
{
    HWND     hWndEdit;
    BOOL     bNotFound = FALSE;
    BOOL     bNotRegExp = FALSE;
    INT      nIsEOF = 0;
    int      srchEOF = 0;
    BOOL     bNeedsFindAllCountOnly = FALSE;
    DWORD    dwSearchFlags = FR_DOWN;

    qs_bEditTextChanged = FALSE;

    if ( dwParams & QSEARCH_FIRST )
    {
        // searching for the first time, clear the EOF flag
        qs_nEditEOF = 0;

        if ( !dwOptFlags[OPTF_SRCH_ONTHEFLY_MODE] )
        {
            if ( g_QSearchDlg.currentMatchesBuf.nBytesStored != 0 )
            {
                qsSetInfoEmpty_Tracking("qsearchDoSearchText, !OnTheFly");
            }
        }
    }
    else
    {
        // searching for the same text again
        if ( qs_bEditNotFound && ((dwParams & QSEARCH_FINDALLFILES) == 0) )
        {
            if ( dwOptFlags[OPTF_SRCH_STOP_EOF] != STOP_EOF_WITHOUT_MSG || qs_bEditNotFound != STOP_EOF_WITHOUT_MSG )
                return;

            if ( !pFindAll )
            {
                if ( (dwParams & QSEARCH_FINDUP) ||
                     (((dwParams & QSEARCH_NOFINDUP_VK) != QSEARCH_NOFINDUP_VK) && (GetKeyState(VK_QS_FINDUP) & 0x80)) )
                    srchEOF = QSEARCH_EOF_UP;
                else
                    srchEOF = QSEARCH_EOF_DOWN;

                if ( qs_nEditEOF == srchEOF )
                    return;
            }
        }
    }

    if ( g_Plugin.bOldWindows )
    {
        if ( g_QSearchDlg.hStInfo )
        {
            if ( (g_Options.dwNewUI == QS_UI_NEW_02) &&
                 (g_Options.dwFindAllMode & QS_FINDALL_AUTO_COUNT_FLAG) &&
                 (0 != ((LPCSTR) cszFindWhatAW)[0]) )
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
                        nLen = removeEofOrNotFoundFromInfoTextA(szInfoTextA, nLen);
                        lstrcatA(szInfoTextA, "...");
                        SetWindowTextA(g_QSearchDlg.hStInfo, szInfoTextA);

                        #ifdef _DEBUG
                          Debug_OutputA("%s -> InfoText = \"%s\"\n", __func__, szInfoTextA);
                        #endif
                    }
                }
            }
            else
            {
                SetWindowTextA(g_QSearchDlg.hStInfo, "");

                #ifdef _DEBUG
                  Debug_OutputA("%s -> InfoText = \"\"\n", __func__);
                #endif
            }
        }

        if ( 0 == ((LPCSTR) cszFindWhatAW)[0] )
        {
            qsearchDoSetNotFound(hEdit, FALSE, FALSE, 0);
            return;
        }
    }
    else
    {
        if ( g_QSearchDlg.hStInfo )
        {
            if ( (g_Options.dwNewUI == QS_UI_NEW_02) &&
                 (g_Options.dwFindAllMode & QS_FINDALL_AUTO_COUNT_FLAG) &&
                 (0 != cszFindWhatAW[0]) )
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
                        nLen = removeEofOrNotFoundFromInfoTextW(szInfoTextW, nLen);
                        lstrcatW(szInfoTextW, L"...");
                        SetWindowTextW(g_QSearchDlg.hStInfo, szInfoTextW);

                        #ifdef _DEBUG
                          Debug_OutputW(L"%S -> InfoText = \"%s\"\n", __func__, szInfoTextW);
                        #endif
                    }
                }
            }
            else
            {
                SetWindowTextW(g_QSearchDlg.hStInfo, L"");

                #ifdef _DEBUG
                  Debug_OutputW(L"%S -> InfoText = \"\"\n", __func__);
                #endif
            }
        }

        if ( 0 == cszFindWhatAW[0] )
        {
            qsearchDoSetNotFound(hEdit, FALSE, FALSE, 0);
            return;
        }
    }

    hWndEdit = GetWndEdit(g_Plugin.hMainWnd);
    if ( !hWndEdit )
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
              ((dwParams & QSEARCH_NOFINDUP_VK) != QSEARCH_NOFINDUP_VK) &&
              ((GetKeyState(VK_QS_FINDUP) & 0x80) == 0x80)) )
        {
            dwSearchFlags = FR_UP;
        }

        if ( ((dwParams & QSEARCH_SEL) != QSEARCH_SEL) &&
             ((dwParams & QSEARCH_NOFINDBEGIN_VK) != QSEARCH_NOFINDBEGIN_VK) &&
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

    if ( dwOptFlags[OPTF_SRCH_MATCHCASE] )
        dwSearchFlags |= FR_MATCHCASE;

    if ( dwOptFlags[OPTF_SRCH_WHOLEWORD] )
        dwSearchFlags |= FR_WHOLEWORD;

    // this forbids deactivating of the Edit control
    // when the text is typed and searched
    qs_bEditCanBeNonActive = FALSE;

    if ( dwParams & QSEARCH_FIRST )
    {
        if ( dwOptFlags[OPTF_SRCH_ONTHEFLY_MODE] && !pFindAll &&
             (dwParams & QSEARCH_NOSETSEL_FIRST) == 0 )
        {
            CHARRANGE_X cr = {0, 0};

            SendMessage( hWndEdit, EM_EXGETSEL_X, 0, (LPARAM) &cr );
            cr.cpMax = cr.cpMin;
            SendMessage( hWndEdit, EM_EXSETSEL_X, 0, (LPARAM) &cr );
        }
        if ( dwOptFlags[OPTF_SRCH_FROM_BEGINNING] )
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

    srchEOF = (dwSearchFlags & FR_UP) ? QSEARCH_EOF_UP : QSEARCH_EOF_DOWN;

    if ( g_Plugin.bOldWindows )
    {
        TEXTFINDA tfA;
        BOOL      bSearchEx = FALSE;
        INT_X     iFindResult = -1;
        char*     pszFindTextA;
        char      szFindTextBufA[MAX_TEXT_SIZE + 6];

        pszFindTextA = szFindTextBufA + 2; // preserving 2 chars for the leading "\b", if needed
        getTextToSearchA( (LPCSTR) cszFindWhatAW, &bSearchEx, dwOptFlags, pszFindTextA );

        if ( dwOptFlags[OPTF_SRCH_USE_REGEXP] )
        {
            if ( (dwParams & QSEARCH_FIRST) || !dwOptFlags[OPTF_SRCH_WHOLEWORD] )
            {
                BOOL bCutTrailingSequence = (dwParams & QSEARCH_FIRST) && dwOptFlags[OPTF_SRCH_ONTHEFLY_MODE] && !dwOptFlags[OPTF_SRCH_WHOLEWORD] && !pFindAll;
                adjustIncompleteRegExA(pszFindTextA, dwOptFlags, bCutTrailingSequence);
            }

            if ( dwOptFlags[OPTF_SRCH_WHOLEWORD] )
            {
                pszFindTextA = surroundRegExpWithWordBoundariesA(pszFindTextA);

                // this flag is not needed since "\b" was added above
                if ( dwSearchFlags & FR_WHOLEWORD )
                    dwSearchFlags ^= FR_WHOLEWORD;
            }
        }

        if ( !pFindAll )
        {
            tfA.dwFlags = dwSearchFlags;
            tfA.pFindIt = pszFindTextA;
            tfA.nFindItLen = -1;

            if ( ((qs_nEditEOF & srchEOF) == 0) || !IsWindowVisible(g_QSearchDlg.hDlg) )
            {
                if ( !bSearchEx )
                {
                    iFindResult = (INT_X) SendMessage( g_Plugin.hMainWnd,
                      AKD_TEXTFINDA, (WPARAM) hWndEdit, (LPARAM) &tfA );
                }
                else
                {
                    iFindResult = doFindTextExA( hWndEdit, &tfA ) ;
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
                    nIsEOF = srchEOF;
                    qs_nEditEOF = 0;
                }
                else if ( dwOptFlags[OPTF_SRCH_STOP_EOF] == STOP_EOF_WITHOUT_MSG )
                {
                    bNotFound = STOP_EOF_WITHOUT_MSG;
                    nIsEOF = srchEOF;
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

                    if ( IsWindowVisible(g_QSearchDlg.hDlg) && (hWndFocused != hWndEdit) )
                        SetFocus(hEdit);
                    else
                        SetFocus(hWndEdit);
                }
                else
                {
                    bContinueSearch = TRUE;
                }

                if ( bContinueSearch )
                {
                    nIsEOF = srchEOF;

                    if ( (dwSearchFlags & FR_UP) == FR_UP )
                    {
                        INT_X       pos = 0;
                        CHARRANGE_X cr = {0, 0};

                        SendMessage( hWndEdit, EM_EXGETSEL_X, 0, (LPARAM) &cr );
                        pos = cr.cpMin;

                        SendMessage( hWndEdit, WM_SETREDRAW, FALSE, 0 );
                        cr.cpMin = -1;
                        cr.cpMax = -1;
                        SendMessage( hWndEdit, EM_EXSETSEL_X, 0, (LPARAM) &cr );
                        if ( tfA.dwFlags & FR_BEGINNING )
                        {
                            tfA.dwFlags -= FR_BEGINNING;
                        }
                        if ( !bSearchEx )
                        {
                            iFindResult = (INT_X) SendMessage( g_Plugin.hMainWnd,
                              AKD_TEXTFINDA, (WPARAM) hWndEdit, (LPARAM) &tfA );
                        }
                        else
                        {
                            iFindResult = doFindTextExA( hWndEdit, &tfA ) ;
                        }
                        if ( iFindResult < 0 )
                        {
                            cr.cpMin = pos;
                            cr.cpMax = pos;
                            SendMessage( hWndEdit, EM_EXSETSEL_X, 0, (LPARAM) &cr );
                            bNotFound = TRUE;
                        }
                        SendMessage( hWndEdit, WM_SETREDRAW, TRUE, 0 );
                        RedrawWindowEntire( hWndEdit );
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
                                  AKD_TEXTFINDA, (WPARAM) hWndEdit, (LPARAM) &tfA );
                            }
                            else
                            {
                                iFindResult = doFindTextExA( hWndEdit, &tfA ) ;
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
        BOOL      bSearchEx = FALSE;
        INT_X     iFindResult = -1;
        wchar_t*  pszFindTextW;
        wchar_t   szFindTextBufW[MAX_TEXT_SIZE + 6];

        pszFindTextW = szFindTextBufW + 2; // preserving 2 chars for the leading "\b", if needed
        getTextToSearchW( cszFindWhatAW, &bSearchEx, dwOptFlags, pszFindTextW );

        if ( dwOptFlags[OPTF_SRCH_USE_REGEXP] )
        {
            if ( (dwParams & QSEARCH_FIRST) || !dwOptFlags[OPTF_SRCH_WHOLEWORD] )
            {
                BOOL bCutTrailingSequence = (dwParams & QSEARCH_FIRST) && dwOptFlags[OPTF_SRCH_ONTHEFLY_MODE] && !dwOptFlags[OPTF_SRCH_WHOLEWORD] && !pFindAll;
                adjustIncompleteRegExW(pszFindTextW, dwOptFlags, bCutTrailingSequence);
            }

            if ( dwOptFlags[OPTF_SRCH_WHOLEWORD] )
            {
                pszFindTextW = surroundRegExpWithWordBoundariesW(pszFindTextW);

                // this flag is not needed since "\b" was added above
                if ( dwSearchFlags & FR_WHOLEWORD )
                    dwSearchFlags ^= FR_WHOLEWORD;
            }
        }

        if ( !pFindAll )
        {
            tfW.dwFlags = dwSearchFlags;
            tfW.pFindIt = pszFindTextW;
            tfW.nFindItLen = -1;

            if ( ((qs_nEditEOF & srchEOF) == 0) || !IsWindowVisible(g_QSearchDlg.hDlg) )
            {
                if ( !bSearchEx )
                {
                    iFindResult = (INT_X) SendMessage( g_Plugin.hMainWnd,
                      AKD_TEXTFINDW, (WPARAM) hWndEdit, (LPARAM) &tfW );
                }
                else
                {
                    iFindResult = doFindTextExW( hWndEdit, &tfW ) ;
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
                    nIsEOF = srchEOF;
                    qs_nEditEOF = 0;
                }
                else if ( dwOptFlags[OPTF_SRCH_STOP_EOF] == STOP_EOF_WITHOUT_MSG )
                {
                    bNotFound = STOP_EOF_WITHOUT_MSG;
                    nIsEOF = srchEOF;
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

                    if ( IsWindowVisible(g_QSearchDlg.hDlg) && (hWndFocused != hWndEdit) )
                        SetFocus(hEdit);
                    else
                        SetFocus(hWndEdit);
                }
                else
                {
                    bContinueSearch = TRUE;
                }

                if ( bContinueSearch )
                {
                    nIsEOF = srchEOF;

                    if ( (dwSearchFlags & FR_UP) == FR_UP )
                    {
                        INT_X       pos = 0;
                        CHARRANGE_X cr = {0, 0};

                        SendMessage( hWndEdit, EM_EXGETSEL_X, 0, (LPARAM) &cr );
                        pos = cr.cpMin;

                        SendMessage( hWndEdit, WM_SETREDRAW, FALSE, 0 );
                        cr.cpMin = -1;
                        cr.cpMax = -1;
                        SendMessage( hWndEdit, EM_EXSETSEL_X, 0, (LPARAM) &cr );
                        if ( tfW.dwFlags & FR_BEGINNING )
                        {
                            tfW.dwFlags -= FR_BEGINNING;
                        }
                        if ( !bSearchEx )
                        {
                            iFindResult = (INT_X) SendMessage( g_Plugin.hMainWnd,
                              AKD_TEXTFINDW, (WPARAM) hWndEdit, (LPARAM) &tfW );
                        }
                        else
                        {
                            iFindResult = doFindTextExW( hWndEdit, &tfW );
                        }
                        if ( iFindResult < 0 )
                        {
                            cr.cpMin = pos;
                            cr.cpMax = pos;
                            SendMessage( hWndEdit, EM_EXSETSEL_X, 0, (LPARAM) &cr );
                            bNotFound = TRUE;
                        }
                        SendMessage( hWndEdit, WM_SETREDRAW, TRUE, 0 );
                        RedrawWindowEntire( hWndEdit );
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
                                  AKD_TEXTFINDW, (WPARAM) hWndEdit, (LPARAM) &tfW );
                            }
                            else
                            {
                                iFindResult = doFindTextExW( hWndEdit, &tfW );
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

            if ( (iFindResult >= 0) &&
                 ((g_Options.dwFindAllMode & QS_FINDALL_AUTO_COUNT_FLAG) != 0) &&
                 (g_QSearchDlg.currentMatchesBuf.nBytesStored != 0) )
            {
                qsSetInfoOccurrencesFound_Tracking( (unsigned int) (g_QSearchDlg.currentMatchesBuf.nBytesStored/sizeof(matchpos_t)), 0, "qsearchDoSearchText, iFindResult >= 0" );
            }
        }
        else // pFindAll
        {
            const FRAMEDATA*     pFrameInitial;   // all files
            HWND                 hFrameWndEdit;
            matchpos_t           nMatchPos;
            unsigned int         nCurrentMatches; // current (initial) file
            BOOL                 bAllFiles;
            tFindAllContext      FindContext;
            tDynamicBuffer       tempMatchesBuf;
            AEFINDTEXTW          aeftW;
            tQSFindAllFrameItem  fndAllFrameItem;
            wchar_t              szFindAllW[2*MAX_TEXT_SIZE];

            pFrameInitial = NULL;
            nCurrentMatches = 0;
            bAllFiles = FALSE;

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
                convertFindExToRegExW(pszFindTextW, szFindAllW);
            else
                lstrcpyW(szFindAllW, pszFindTextW);
            aeftW.pText = szFindAllW;
            aeftW.dwTextLen = lstrlenW(szFindAllW);
            aeftW.nNewLine = AELB_ASIS;

            FindContext.cszFindWhat = cszFindWhatAW;
            FindContext.pFindTextW = &aeftW;
            FindContext.pFrame = NULL;
            FindContext.dwFindAllMode = g_Options.dwFindAllMode;
            FindContext.dwFindAllFlags = getFindAllFlags(dwOptFlags);
            FindContext.dwFindAllResult = g_Options.dwFindAllResult;
            FindContext.bWordWrap = FALSE;
            FindContext.nOccurrences = 0;
            FindContext.nTotalOccurrences = 0;
            FindContext.nTotalFiles = 0;
            FindContext.nFilesWithOccurrences = 0;
            FindContext.nLastLine = -1;
            FindContext.nLastOccurrenceLine = -1;
            tDynamicBuffer_Init( &FindContext.ResultsBuf );
            tDynamicBuffer_Init( &FindContext.OccurrencesBuf );
            tDynamicBuffer_Allocate( &FindContext.ResultsBuf, 128*1024*sizeof(wchar_t) );
            tDynamicBuffer_Allocate( &FindContext.OccurrencesBuf, 128*1024*sizeof(wchar_t) );

            tDynamicBuffer_Init(&tempMatchesBuf);

            if ( g_Plugin.nMDI == WMD_SDI )
            {
                if ( FindContext.dwFindAllResult & QS_FINDALL_RSLT_ALLFILES )
                    FindContext.dwFindAllResult ^= QS_FINDALL_RSLT_ALLFILES;
            }
            else // MDI, PMDI
            {
                if ( dwParams & QSEARCH_FINDALLFILES )
                    FindContext.dwFindAllResult |= QS_FINDALL_RSLT_ALLFILES;
            }

            if ( (FindContext.dwFindAllResult & QS_FINDALL_RSLT_ALLFILES) &&
                 (pFindAll->pfnFindResultCallback != NULL) ) // not CountOnly
            {
                bAllFiles = TRUE;
            }

            x_zero_mem(&fndAllFrameItem, sizeof(tQSFindAllFrameItem));

            FindContext.pFrame = (FRAMEDATA *) SendMessageW(g_Plugin.hMainWnd, AKD_FRAMEFIND, FWF_CURRENT, 0);
            if ( FindContext.pFrame )
            {
                if ( bAllFiles )
                {
                    pFrameInitial = FindContext.pFrame;

                    FindContext.nTotalFiles = (unsigned int) SendMessage(g_Plugin.hMainWnd, AKD_FRAMESTATS, FWS_COUNTALL, 0);
                    if ( ((g_Options.dwFindAllMode & QS_FINDALL_MASK) == QS_FINDALL_FILEOUTPUT_SNGL) ||
                         ((g_Options.dwFindAllMode & QS_FINDALL_MASK) == QS_FINDALL_FILEOUTPUT_MULT) )
                    {
                        const FRAMEDATA* pFr;

                        pFr = pFrameInitial;
                        for ( ; ; )
                        {
                            if ( pFr == QSearchDlgState_GetSearchResultsFrame(&g_QSearchDlg) )
                            {
                                --FindContext.nTotalFiles;
                                break;
                            }

                            pFr = (FRAMEDATA *) SendMessageW(g_Plugin.hMainWnd, AKD_FRAMEFIND, FWF_NEXT, (LPARAM) pFr);
                            if ( pFr == pFrameInitial )
                                break;
                        }
                    }
                    pFindAll->ShowFindResults.pfnAllFilesInit(&FindContext, &pFindAll->tempBuf);
                }

                if ( (dwParams & QSEARCH_COUNTALL) == 0 )
                {
                    g_QSearchDlg.nGoToNextFindAllPosToCompare = -1;

                    if ( pFindAll->ShowFindResults.pfnInit == qsShowFindResults_LogOutput_Init )
                        g_QSearchDlg.bFindAllWasUsingLogOutput = TRUE;
                    else
                        g_QSearchDlg.bFindAllWasUsingLogOutput = FALSE;

                    // findAllFramesBuf
                    if ( g_QSearchDlg.findAllFramesBuf.nBytesAllocated > 128*sizeof(tQSFindAllFrameItem) )
                        tDynamicBuffer_Free(&g_QSearchDlg.findAllFramesBuf);

                    if ( g_QSearchDlg.findAllFramesBuf.nBytesAllocated == 0 )
                        tDynamicBuffer_Allocate(&g_QSearchDlg.findAllFramesBuf, FindContext.nTotalFiles*sizeof(tQSFindAllFrameItem));
                    else
                        tDynamicBuffer_Clear(&g_QSearchDlg.findAllFramesBuf);

                    // findAllMatchesBuf
                    if ( g_QSearchDlg.findAllMatchesBuf.nBytesAllocated > 16*1024*sizeof(matchpos_t) )
                        tDynamicBuffer_Free(&g_QSearchDlg.findAllMatchesBuf);

                    if ( g_QSearchDlg.findAllMatchesBuf.nBytesAllocated == 0 )
                        tDynamicBuffer_Allocate(&g_QSearchDlg.findAllMatchesBuf, 2*1024*sizeof(matchpos_t));
                    else
                        tDynamicBuffer_Clear(&g_QSearchDlg.findAllMatchesBuf);

                    lstrcpyW(g_QSearchDlg.szFindAllFindTextW, cszFindWhatAW);
                    g_QSearchDlg.dwFindAllFlags = FindContext.dwFindAllFlags;
                }

                for ( ; ; )
                {
                    if ( bAllFiles && FindContext.pFrame == QSearchDlgState_GetSearchResultsFrame(&g_QSearchDlg) )
                    {
                        // skip the SearchResultsFrame
                        FindContext.pFrame = (FRAMEDATA *) SendMessageW(g_Plugin.hMainWnd, AKD_FRAMEFIND, FWF_NEXT, (LPARAM) FindContext.pFrame);
                        if ( FindContext.pFrame != pFrameInitial )
                        {
                            if ( g_Plugin.nMDI == WMD_PMDI )
                            {
                                SendMessageW(g_Plugin.hMainWnd, AKD_FRAMEACTIVATE, 0, (LPARAM) FindContext.pFrame);
                            }
                        }
                        else
                            break;
                    }

                    hFrameWndEdit = FindContext.pFrame->ei.hWndEdit;
                    SendMessageW( hFrameWndEdit, AEM_GETINDEX, AEGI_FIRSTCHAR, (LPARAM) &aeftW.crSearch.ciMin );
                    SendMessageW( hFrameWndEdit, AEM_GETINDEX, AEGI_LASTCHAR, (LPARAM) &aeftW.crSearch.ciMax);

                    FindContext.bWordWrap = (SendMessageW(hFrameWndEdit, AEM_GETWORDWRAP, 0, 0) != AEWW_NONE) ? TRUE : FALSE;
                    FindContext.nOccurrences = 0;
                    FindContext.nLastLine = -1;
                    FindContext.nLastOccurrenceLine = -1;

                    pFindAll->ShowFindResults.pfnInit(&FindContext, &pFindAll->tempBuf);

                    if  ( !bAllFiles || pFrameInitial == FindContext.pFrame )
                    {
                        // currentMatchesBuf
                        if ( g_QSearchDlg.currentMatchesBuf.nBytesAllocated > 8*1024*sizeof(matchpos_t) )
                            tDynamicBuffer_Free(&g_QSearchDlg.currentMatchesBuf);

                        if ( g_QSearchDlg.currentMatchesBuf.nBytesAllocated == 0 )
                            tDynamicBuffer_Allocate(&g_QSearchDlg.currentMatchesBuf, 1024*sizeof(matchpos_t));
                        else
                            tDynamicBuffer_Clear(&g_QSearchDlg.currentMatchesBuf);

                        g_QSearchDlg.hCurrentMatchSetInfoEditWnd = NULL;
                    }

                    while ( SendMessageW(hFrameWndEdit, AEM_FINDTEXTW, 0, (LPARAM) &aeftW) )
                    {
                        ++FindContext.nOccurrences;
                        ++FindContext.nTotalOccurrences;

                        if ( pFindAll->pfnFindResultCallback )
                        {
                            pFindAll->pfnFindResultCallback(&FindContext, &aeftW.crFound, &pFindAll->GetFindResultPolicy, &pFindAll->tempBuf, &pFindAll->tempBuf2, pFindAll->ShowFindResults.pfnAddOccurrence);
                        }

                        nMatchPos = to_matchpos_ae_ex(&aeftW.crFound.ciMin, hFrameWndEdit, FindContext.bWordWrap);

                        if ( !bAllFiles || pFrameInitial == FindContext.pFrame )
                        {
                            QSearchDlgState_addCurrentMatch(&g_QSearchDlg, nMatchPos);
                        }

                        if ( (dwParams & QSEARCH_COUNTALL) == 0 )
                        {
                            QSearchDlgState_addFindAllMatch(&g_QSearchDlg, nMatchPos);
                        }

                        x_mem_cpy( &aeftW.crSearch.ciMin, &aeftW.crFound.ciMax, sizeof(AECHARINDEX) );
                    }

                    if ( FindContext.nOccurrences != 0 )
                    {
                        ++FindContext.nFilesWithOccurrences;

                        if ( (dwParams & QSEARCH_COUNTALL) == 0 )
                        {
                            // current fndAllFrame: nBufBytesOffset = _previous_ findAllMatchesBuf.nBytesStored
                            fndAllFrameItem.pFrame = FindContext.pFrame;
                            fndAllFrameItem.nMatches = FindContext.nOccurrences;

                            QSearchDlgState_addFindAllFrameItem(&g_QSearchDlg, &fndAllFrameItem);

                            // next fndAllFrame: nBufBytesOffset = _current_ findAllMatchesBuf.nBytesStored
                            fndAllFrameItem.nBufBytesOffset = g_QSearchDlg.findAllMatchesBuf.nBytesStored;
                        }
                    }

                    if ( pFrameInitial == FindContext.pFrame )
                    {
                        nCurrentMatches = FindContext.nOccurrences;

                        if ( bAllFiles )
                        {
                            tDynamicBuffer_Swap(&tempMatchesBuf, &g_QSearchDlg.currentMatchesBuf);
                        }
                    }

                    pFindAll->ShowFindResults.pfnDone(&FindContext, &pFindAll->tempBuf);

                    if ( bAllFiles )
                    {
                        FindContext.pFrame = (FRAMEDATA *) SendMessageW(g_Plugin.hMainWnd, AKD_FRAMEFIND, FWF_NEXT, (LPARAM) FindContext.pFrame);
                        if ( FindContext.pFrame != pFrameInitial )
                        {
                            if ( g_Plugin.nMDI == WMD_PMDI )
                            {
                                SendMessageW(g_Plugin.hMainWnd, AKD_FRAMEACTIVATE, 0, (LPARAM) FindContext.pFrame);
                            }
                        }
                        else
                            break;
                    }
                    else
                    {
                        tDynamicBuffer_Clear(&FindContext.ResultsBuf);
                        tDynamicBuffer_Clear(&FindContext.OccurrencesBuf);
                        break;
                    }
                }

                if ( bAllFiles )
                {
                    if ( g_Plugin.nMDI == WMD_PMDI )
                    {
                        SendMessageW(g_Plugin.hMainWnd, AKD_FRAMEACTIVATE, 0, (LPARAM) pFrameInitial);
                    }

                    bNeedsFindAllCountOnly = FALSE;

                    if ( pFrameInitial != QSearchDlgState_GetSearchResultsFrame(&g_QSearchDlg) )
                    {
                        tDynamicBuffer_Swap(&g_QSearchDlg.currentMatchesBuf, &tempMatchesBuf);
                        qsSetInfoOccurrencesFound_Tracking(nCurrentMatches, 0, "qsearchDoSearchText, bAllFiles");
                    }
                    else
                    {
                        qsSetInfoEmpty_Tracking("qsearchDoSearchText, bAllFiles");
                    }

                    pFindAll->ShowFindResults.pfnAllFilesDone(&FindContext, &pFindAll->tempBuf);
                    tDynamicBuffer_Clear(&FindContext.ResultsBuf);
                    tDynamicBuffer_Clear(&FindContext.OccurrencesBuf);
                }
            }

            tDynamicBuffer_Free(&pFindAll->tempBuf);
            tDynamicBuffer_Free(&pFindAll->tempBuf2);

            tDynamicBuffer_Free(&FindContext.ResultsBuf);
            tDynamicBuffer_Free(&FindContext.OccurrencesBuf);

            tDynamicBuffer_Free(&tempMatchesBuf);

            if ( FindContext.nTotalOccurrences == 0 )
            {
                bNotFound = TRUE;
                if ( (dwSearchFlags & FRF_REGEXP) && (aeftW.nCompileErrorOffset > 0) )
                {
                    bNotRegExp = TRUE;
                    nIsEOF = srchEOF;
                    qs_nEditEOF = 0;
                }
                else
                {
                    nIsEOF = srchEOF;
                    qs_nEditEOF = QSEARCH_EOF_DOWN | QSEARCH_EOF_UP;
                }
            }

            if ( (dwParams & QSEARCH_COUNTALL) == 0 )
            {
                if ( g_QSearchDlg.currentMatchesBuf.nBytesStored != 0 )
                {
                    // there are matches in the current file
                    qsUpdateHighlightForFindAll(g_QSearchDlg.szFindAllFindTextW, g_QSearchDlg.dwFindAllFlags, FALSE);
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

            SendMessage( hWndEdit, EM_EXGETSEL_X, 0, (LPARAM) &cr );
            nLine = (int) SendMessage(hWndEdit, EM_EXLINEFROMCHAR, 0, cr.cpMin);
            nLine1st = (int) SendMessage(hWndEdit, EM_GETFIRSTVISIBLELINE, 0, 0);
            if ( nLine < nLine1st )
            {
                SendMessage(hWndEdit, EM_LINESCROLL, 0, nLine - nLine1st);
            }

            if ( (dwParams & QSEARCH_NOHISTORYUPD) == 0 )
            {
                if ( (dwParams & QSEARCH_NEXT) ||
                     ((dwParams & QSEARCH_FIRST) && !dwOptFlags[OPTF_SRCH_ONTHEFLY_MODE]) )
                {
                    UINT uUpdFlags = UFHF_MOVE_TO_TOP_IF_EXISTS;
                    if ( (dwParams & QSEARCH_SEL) && !dwOptFlags[OPTF_SRCH_SELFIND_PICKUP] )
                        uUpdFlags |= UFHF_KEEP_EDIT_TEXT;
                    qsearchFindHistoryAdd(hEdit, cszFindWhatAW, uUpdFlags);
                }
            }
        }
        else if ( dwParams & QSEARCH_FINDALL )
        {
            if ( (dwParams & QSEARCH_NOHISTORYUPD) == 0 )
            {
                UINT uUpdFlags = UFHF_MOVE_TO_TOP_IF_EXISTS;
                if ( (dwParams & QSEARCH_SEL) && !dwOptFlags[OPTF_SRCH_SELFIND_PICKUP] )
                    uUpdFlags |= UFHF_KEEP_EDIT_TEXT;
                qsearchFindHistoryAdd(hEdit, cszFindWhatAW, uUpdFlags);
            }
        }
    }

    // now the Edit control can be deactivated
    qs_bEditCanBeNonActive = TRUE;
    qsearchDoSetNotFound(hEdit, bNotFound, bNotRegExp, (dwParams & QSEARCH_COUNTALL) ? (nIsEOF | QSEARCH_EOF_IGNORE) : nIsEOF);

    g_QSearchDlg.bIsQSearchingRightNow = FALSE;

    if ( bNeedsFindAllCountOnly && !pFindAll && IsWindowVisible(g_QSearchDlg.hDlg) )
    {
        UINT nDelayMs;
        UINT_PTR nTimerId;
        BOOL bGotCountAllResults = FALSE;

        if ( !QSearchDlgState_isFindAllMatchesEmpty(&g_QSearchDlg) &&
             !g_Plugin.bOldWindows &&
             QSearchDlgState_isFindAllSearchEqualToTheCurrentSearch(&g_QSearchDlg, cszFindWhatAW, getFindAllFlags(dwOptFlags)) )
        {
            const FRAMEDATA* pFrame;
            const tQSFindAllFrameItem* pItem;
            const matchpos_t* pItemMatches;

            pFrame = (FRAMEDATA *) SendMessageW(g_Plugin.hMainWnd, AKD_FRAMEFIND, FWF_CURRENT, 0);
            if ( pFrame && pFrame->ei.hWndEdit == hWndEdit )
            {
                pItem = QSearchDlgState_getFindAllFrameItemByFrame(&g_QSearchDlg, pFrame);
                if ( pItem && !(pItem->nItemState & (QS_FIS_INVALID | QS_FIS_TEXTCHANGED)) &&
                     QSearchDlgState_isFindAllFrameItemInternallyValid(&g_QSearchDlg, pItem) )
                {
                    #ifdef _DEBUG
                      Debug_OutputA("qsearchDoSearchText: copying the matches from findAllMatchesBuf to currentMatchesBuf\n");
                    #endif
                    pItemMatches = QSearchDlgState_getFindAllFrameItemMatches(&g_QSearchDlg, pItem);
                    tDynamicBuffer_Clear(&g_QSearchDlg.currentMatchesBuf);
                    tDynamicBuffer_Append(&g_QSearchDlg.currentMatchesBuf, pItemMatches, pItem->nMatches*sizeof(matchpos_t));
                    qsSetInfoOccurrencesFound_Tracking( (unsigned int) (g_QSearchDlg.currentMatchesBuf.nBytesStored/sizeof(matchpos_t)), 0, "qsearchDoSearchText, bNeedsFindAllCountOnly && !pFindAll" );
                    bGotCountAllResults = TRUE;
                }
            }
        }

        if ( !bGotCountAllResults )
        {
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
}

void qsearchDoTryHighlightAll(HWND hDlg, const wchar_t* cszFindWhatAW, const DWORD dwOptFlags[], DWORD dwHighlightConditionFlags)
{
    if ( g_Plugin.bOldWindows )
    {
        if ( 0 == ((LPCSTR) cszFindWhatAW)[0] )
        {
            // no text to search for: removing the highlight
            qsearchDoTryUnhighlightAll();
            return;
        }
    }
    else
    {
        if ( 0 == cszFindWhatAW[0] )
        {
            // no text to search for: removing the highlight
            qsearchDoTryUnhighlightAll();
            return;
        }
    }

    if ( g_bHighlightPlugin &&
         (!qs_bEditNotFound || (dwHighlightConditionFlags & QHC_FORCE_HIGHLIGHT)) )
    {
        if ( (dwHighlightConditionFlags & QHC_IGNORE_CHECKBOX) ||
             dwOptFlags[OPTF_SRCH_HIGHLIGHTALL] )
        {
            HWND hWndEdit;

            hWndEdit = GetWndEdit(g_Plugin.hMainWnd);
            if ( hWndEdit )
            {
                CHARRANGE_X cr = { 0, 0 };

                SendMessage( hWndEdit, EM_EXGETSEL_X, 0, (LPARAM) &cr );

                if ( (dwHighlightConditionFlags & QHC_IGNORE_SELECTION) != 0 || cr.cpMin != cr.cpMax )
                {
                    DWORD dwFindAllFlags;
                    BOOL bFindExAsRegExp;
                    DLLECHIGHLIGHT_MARK hlParams;
                    wchar_t szMarkTextBufW[2*MAX_TEXT_SIZE + 6]; // plus room for the leading & trailing "\\b"

                    x_zero_mem(&hlParams, sizeof(DLLECHIGHLIGHT_MARK));

                    if ( dwOptFlags[OPTF_SRCH_MATCHCASE] )
                        hlParams.dwMarkFlags |= MARKFLAG_MATCHCASE;
                    if ( dwOptFlags[OPTF_SRCH_WHOLEWORD] )
                        hlParams.dwMarkFlags |= MARKFLAG_WHOLEWORD;

                    bFindExAsRegExp = FALSE;

                    if ( dwOptFlags[OPTF_SRCH_USE_SPECIALCHARS] && !dwOptFlags[OPTF_SRCH_USE_REGEXP] )
                    {
                        if ( g_Plugin.bOldWindows )
                        {
                            if ( findSpecialCharA((LPCSTR) cszFindWhatAW) != -1 )
                                bFindExAsRegExp = TRUE;
                        }
                        else
                        {
                            if ( findSpecialCharW(cszFindWhatAW) != -1 )
                                bFindExAsRegExp = TRUE;
                        }
                    }

                    szMarkTextBufW[0] = 0;

                    if ( dwOptFlags[OPTF_SRCH_USE_REGEXP] || bFindExAsRegExp )
                    {
                        wchar_t* pszMarkTextW;

                        pszMarkTextW = szMarkTextBufW + 2; // preserving 2 chars for the leading "\b", if needed
                        pszMarkTextW[0] = 0;

                        if ( g_Plugin.bOldWindows )
                        {
                            const char* cszFindTextA;
                            char szRegExBufA[2*MAX_TEXT_SIZE];

                            if ( bFindExAsRegExp )
                            {
                                convertFindExToRegExA( (LPCSTR) cszFindWhatAW, szRegExBufA );
                                cszFindTextA = szRegExBufA;
                            }
                            else
                                cszFindTextA = (LPCSTR) cszFindWhatAW;

                            MultiByteToWideChar( CP_ACP, 0, cszFindTextA, -1, pszMarkTextW, 2*MAX_TEXT_SIZE - 1 );
                        }
                        else
                        {
                            if ( bFindExAsRegExp )
                                convertFindExToRegExW( cszFindWhatAW, pszMarkTextW );
                            else
                                lstrcpyW( pszMarkTextW, cszFindWhatAW );
                        }

                        if ( dwOptFlags[OPTF_SRCH_USE_REGEXP] && ((dwHighlightConditionFlags & QHC_FINDFIRST) || !dwOptFlags[OPTF_SRCH_WHOLEWORD]) )
                        {
                            BOOL bCutTrailingSequence = !(dwHighlightConditionFlags & QHC_DONT_CUT_REGEXP) && (dwHighlightConditionFlags & QHC_FINDFIRST) && dwOptFlags[OPTF_SRCH_ONTHEFLY_MODE] && !dwOptFlags[OPTF_SRCH_WHOLEWORD];
                            adjustIncompleteRegExW(pszMarkTextW, dwOptFlags, bCutTrailingSequence);
                        }

                        if ( pszMarkTextW[0] != 0 )
                        {
                            hlParams.dwMarkFlags |= MARKFLAG_REGEXP;
                            if ( hlParams.dwMarkFlags & MARKFLAG_WHOLEWORD )
                            {
                                // When MARKFLAG_REGEXP is set in dwMarkFlags,
                                // MARKFLAG_WHOLEWORD seems to be ignored...
                                hlParams.dwMarkFlags ^= MARKFLAG_WHOLEWORD;
                                pszMarkTextW = surroundRegExpWithWordBoundariesW(pszMarkTextW);
                            }
                            hlParams.wszMarkText = pszMarkTextW;
                        }
                    }
                    else if ( dwHighlightConditionFlags & QHC_IGNORE_SELECTION )
                    {
                        if ( g_Plugin.bOldWindows )
                            MultiByteToWideChar( CP_ACP, 0, (LPCSTR) cszFindWhatAW, -1, szMarkTextBufW, 2*MAX_TEXT_SIZE - 1 );
                        else
                            lstrcpyW(szMarkTextBufW, cszFindWhatAW);

                        hlParams.wszMarkText = szMarkTextBufW;
                    }

                    dwFindAllFlags = getFindAllFlags(dwOptFlags);

                    if ( (dwHighlightConditionFlags & QHC_FORCE_HIGHLIGHT) ||
                         !hlParams.wszMarkText ||
                         !QSearchDlgState_isLastHighlightedEqualToTheSearch(&g_QSearchDlg, cszFindWhatAW, dwFindAllFlags) )
                    {
                        wchar_t szTextColorAW[16];
                        wchar_t szBkgndColorAW[16];

                        if ( g_Plugin.bOldWindows )
                        {
                            wsprintfA( (char *) szTextColorAW, "0" );
                            wsprintfA( (char *) szBkgndColorAW, "#%02X%02X%02X",
                              GetRValue(g_Options.colorHighlight),
                              GetGValue(g_Options.colorHighlight),
                              GetBValue(g_Options.colorHighlight)
                            );
                        }
                        else
                        {
                            wsprintfW( szTextColorAW, L"0" );
                            wsprintfW( szBkgndColorAW, L"#%02X%02X%02X",
                              GetRValue(g_Options.colorHighlight),
                              GetGValue(g_Options.colorHighlight),
                              GetBValue(g_Options.colorHighlight)
                            );
                        }

                        hlParams.dwStructSize = sizeof(DLLECHIGHLIGHT_MARK);
                        hlParams.nAction = DLLA_HIGHLIGHT_MARK;
                        hlParams.pColorText = (unsigned char *) szTextColorAW;
                        hlParams.pColorBk = (unsigned char *) szBkgndColorAW;
                        hlParams.dwFontStyle = 0;
                        hlParams.dwMarkID = g_Options.dwHighlightMarkID;

                        CallHighlightMain( &hlParams );

                        g_QSearchDlg.dwLastHighlightFlags = dwFindAllFlags;
                        if ( g_Plugin.bOldWindows )
                            MultiByteToWideChar( CP_ACP, 0, (LPCSTR) cszFindWhatAW, -1, g_QSearchDlg.szLastHighlightTextW, MAX_TEXT_SIZE );
                        else
                            lstrcpyW(g_QSearchDlg.szLastHighlightTextW, cszFindWhatAW);
                    }
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

        QSearchDlgState_clearLastHighlighted(&g_QSearchDlg);
    }
}

HWND qsearchGetFindEdit(HWND hDlg, HWND* phListBox)
{
    HWND hEdit;

    if ( phListBox )
        *phListBox = NULL;

    if ( qsearchIsFindHistoryEnabled() )
    {
        HWND  hCombo;
        POINT pt;

        hCombo = GetDlgItem(hDlg, IDC_CB_FINDTEXT);
        pt.x = 5;
        pt.y = 5;
        hEdit = ChildWindowFromPoint(hCombo, pt);

        if ( phListBox && !g_Plugin.bOldWindows )
        {
#ifndef CB_GETCOMBOBOXINFO
    #define CB_GETCOMBOBOXINFO          0x0164
#endif
            COMBOBOXINFO info;

            x_zero_mem(&info, sizeof(info));
            info.cbSize = sizeof(info);
            if ( SendMessage(hCombo, CB_GETCOMBOBOXINFO, 0, (LPARAM) &info) )
                *phListBox = info.hwndList;
        }
    }
    else
    {
        hEdit = GetDlgItem(hDlg, IDC_ED_FINDTEXT);
    }

    return hEdit;
}

BOOL qsearchFindHistoryAdd(HWND hEdit, const wchar_t* cszTextAW, UINT uUpdFlags)
{
    if ( qsearchIsFindHistoryEnabled() && cszTextAW && 
         (g_Plugin.bOldWindows ? (((const char *)cszTextAW)[0] != 0) : (cszTextAW[0] != 0)) )
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

void strcpyAorW(LPWSTR lpDst, LPCWSTR lpSrc)
{
    if ( g_Plugin.bOldWindows )
        lstrcpyA( (LPSTR) lpDst, (LPCSTR) lpSrc );
    else
        lstrcpyW( lpDst, lpSrc );
}

int strcmpAorW(LPCWSTR lpStr1, LPCWSTR lpStr2, BOOL bMatchCase)
{
    return ( g_Plugin.bOldWindows ?
        (bMatchCase ? lstrcmpA : lstrcmpiA)((LPCSTR) lpStr1, (LPCSTR) lpStr2) :
            (bMatchCase ? lstrcmpW : lstrcmpiW)(lpStr1, lpStr2) );
}
