#include "QSearchFindEx.h"
#include "QSearchDlg.h"
#include "QSearch.h"


#define  MAX_MASK_SIZE       MAX_TEXT_SIZE
#define  MAX_LINE_SIZE       0x10000
#define  MAX_LINES_TO_CHECK  16

// progress bar will not be 
// shown and updated before:
#define  LINES_TO_SKIP       50


// extern vars
extern PluginState     g_Plugin;
extern QSearchOpt      g_Options;
extern QSearchDlgState g_QSearchDlg;


// static (local) vars
static wchar_t szLineBufW[MAX_LINE_SIZE];

/*
  Original code of match_mask: 
    c_base/MatchMask.c v.1.1 (Jul 2007).
  Adapted for the QSearch plugin.

  // match_mask understands these wildcards:
  //              *  - any 0 or more characters (whole_word == 0)
  //              ** - any 0 or more non-space characters (whole_word != 0)
  //              ?  - any single character
  //              \* - '*' character (not a wildcard)
  //              \? - '?' character (not a wildcard)
  //              \\ - '\' character
  //
  // current implementation is case-sensitive
  // returns 1 if match, otherwise 0 (or -1 on word-break)
  //

*/

// QSearchFind_WholeWord:
#define QSF_WW_DELIM     1
#define QSF_WW_SPACE     2
#define QSF_WW_PRE_SPACE 3

int is_wordbreakw(int whole_word, const wchar_t wch)
{
    if ( whole_word == QSF_WW_DELIM )
    {
        static wchar_t szDelimitersW[128] = { 0 };
        static int     ok = 0;
        const wchar_t* p;

        if ( !ok )
        {
            // delimiters are not initialized yet
#if AKELPAD_RUNTIME_VERSION_CHECK
            if ( VersionCompare(getProgramVersion(&g_Plugin), MAKE_IDENTIFIER(4, 5, 1, 0)) >= 0 )
#endif
            {
                EDITINFO ei;

                ei.hWndEdit = NULL;
                SendMessage( g_Plugin.hMainWnd, AKD_GETEDITINFO, 0, (LPARAM) &ei );
                if ( ei.hWndEdit )
                {
                    szDelimitersW[0] = 0;
                    SendMessage( ei.hWndEdit, AEM_GETWORDDELIMITERS, 127, (LPARAM) szDelimitersW );
                    if ( szDelimitersW[0] != 0 )
                        ok = 1;
                }
            }
#if AKELPAD_RUNTIME_VERSION_CHECK
            else
                ok = 2;
#endif

            if ( ok != 1 )
                lstrcpyW(szDelimitersW, AES_WORDDELIMITERSW);
        }

        p = szDelimitersW;
        while ( *p )
        {
            if ( *(p++) == wch )
                return 1;
        }
    }
    else if ( whole_word == QSF_WW_SPACE )
    {
        switch ( wch )
        {
            case L' ':
            case L'\t':
            case L'\n':
            case L'\r':
            case 0:
                return 1;
        }
    }
    return 0;
}

int is_wordbreak(int whole_word, const char ch)
{
    if ( whole_word )
    {
        char    s[2];
        wchar_t ws[2];

        ws[0] = 0;
        s[0] = ch;
        s[1] = 0;
        MultiByteToWideChar(CP_ACP, 0, s, 1, ws, 2);
        return is_wordbreakw(whole_word, ws[0]);
    }
    return 0;
}

int match_mask(const char* mask, const char* str, char** last_pos, int whole_word)
{
    if ( last_pos )
        *last_pos = 0;

    if ( mask && str )
    {
        int matched = 0;
        int done = 0;

        while ( !done )
        {
            if ( *mask == '*' ) // 0 or more characters
            {
                ++mask;
                while ( *mask == '*' ) // "**", "***", ...
                {
                    if ( whole_word )
                        whole_word = QSF_WW_PRE_SPACE;
                    ++mask;
                }
                if ( *mask == 0 )
                {
                    matched = 1;
                }
                else
                {
                    if ( whole_word )
                    {
                        if ( whole_word == QSF_WW_PRE_SPACE )
                            whole_word = QSF_WW_SPACE;
                        else
                            whole_word = QSF_WW_DELIM;
                    }
                    matched = match_mask(mask, str, last_pos, whole_word);
                    while ( (matched == 0) && (*str != 0) )
                    {
                        ++str;
                        matched = match_mask(mask, str, last_pos, whole_word);
                    }
                }
                done = 1;
            }
            else if ( *mask == 0 ) // mask is over
            {
                matched = (*str == 0) ? 1 : 0;
                done = 1;
            }
            else if ( *mask == '\\' )
            {
                ++mask;
                if ( *mask == 0 )
                {
                    matched = (*str == 0) ? 1 : 0;
                    done = 1;
                }
                else
                {
                    if ( *mask == *str ) // exact match, case-sensitive
                    {
                        // word-break may be used explicitly (as part of the mask)
                        ++mask;
                        ++str;
                    }
                    else
                    {
                        matched = 0;
                        done = 1;
                    }
                }
            }
            else
            {
                if ( *mask == *str ) // exact match, case-sensitive
                {
                    // word-break may be used explicitly (as part of the mask)
                    ++mask;
                    ++str;
                }
                else if ( (*mask == '?') && (*str != 0) ) // any character
                {
                    if ( is_wordbreak(whole_word, *str) )
                    {
                        matched = -1;
                        done = 1;
                    }
                    else
                    {
                        ++mask;
                        ++str;
                    }
                }
                else
                {
                    if ( is_wordbreak(whole_word, *str) )
                        matched = -1;
                    else
                        matched = 0;
                    done = 1;
                }
            }
        }

        if ( last_pos && (str > *last_pos) )
            *last_pos = (char *) str;

        return matched;
    }
    return 0;
}

int match_maskw(const wchar_t* maskw, const wchar_t* strw, wchar_t** last_pos, int whole_word)
{
    if ( last_pos )
        *last_pos = 0;

    if ( maskw && strw )
    {
        int matched = 0;
        int done = 0;

        while ( !done )
        {
            if ( *maskw == L'*' ) // 0 or more characters
            {
                ++maskw;
                while ( *maskw == L'*' ) // "**", "***", ...
                {
                    if ( whole_word )
                        whole_word = QSF_WW_PRE_SPACE;
                    ++maskw;
                }
                if ( *maskw == 0 )
                {
                    matched = 1;
                }
                else
                {
                    if ( whole_word )
                    {
                        if ( whole_word == QSF_WW_PRE_SPACE )
                            whole_word = QSF_WW_SPACE;
                        else
                            whole_word = QSF_WW_DELIM;
                    }
                    matched = match_maskw(maskw, strw, last_pos, whole_word);
                    while ( (matched == 0) && (*strw != 0) )
                    {
                        ++strw;
                        matched = match_maskw(maskw, strw, last_pos, whole_word);
                    }
                }
                done = 1;
            }
            else if ( *maskw == 0 ) // mask is over
            {
                matched = (*strw == 0) ? 1 : 0;
                done = 1;
            }
            else if ( *maskw == L'\\' )
            {
                ++maskw;
                if ( *maskw == 0 )
                {
                    matched = (*strw == 0) ? 1 : 0;
                    done = 1;
                }
                else
                {
                    if ( *maskw == *strw ) // exact match, case-sensitive
                    {
                        // word-break may be used explicitly (as part of the mask)
                        ++maskw;
                        ++strw;
                    }
                    else
                    {
                        matched = 0;
                        done = 1;
                    }
                }
            }
            else
            {
                if ( *maskw == *strw ) // exact match, case-sensitive
                {
                    // word-break may be used explicitly (as part of the mask)
                    ++maskw;
                    ++strw;
                }
                else if ( (*maskw == L'?') && (*strw != 0) ) // any character
                {
                    if ( is_wordbreakw(whole_word, *strw) )
                    {
                        matched = -1;
                        done = 1;
                    }
                    else
                    {
                        ++maskw;
                        ++strw;
                    }
                }
                else
                {
                    if ( is_wordbreakw(whole_word, *strw) )
                        matched = -1;
                    else
                        matched = 0;
                    done = 1;
                }
            }
        }

        if ( last_pos && (strw > *last_pos) )
            *last_pos = (wchar_t *) strw;

        return matched;
    }
    return 0;
}

LPCWSTR getTextToSearch(LPCWSTR cszTextW, BOOL* pbSearchEx)
{
    *pbSearchEx = FALSE;

    if ( g_Options.dwFlags[OPTF_SRCH_USE_SPECIALCHARS] )
    {
        static wchar_t szSearchTextW[MAX_TEXT_SIZE];

        int  i, k;
        BOOL bEscapeChars, bMask;

        bEscapeChars = FALSE;
        bMask = FALSE;

        if ( g_Plugin.bOldWindows )
        {
            char ch;

            i = 0;
            while ( ((LPCSTR) cszTextW)[i] )
            {
                switch ( ((LPCSTR) cszTextW)[i] )
                {
                    case '*':
                    case '?':
                        bMask = TRUE;
                        break;
                    case '\\':
                        bEscapeChars = TRUE;
                        if ( ((LPCSTR) cszTextW)[i + 1] )
                            ++i;
                        break;
                    default:
                        break;
                }
                ++i;
            }

            if ( bMask )
            {
                *pbSearchEx = TRUE;
            }

            if ( bEscapeChars )
            {
                i = 0;
                k = 0;
                while ( 1 )
                {
                    ch = ((LPCSTR) cszTextW)[i++];
                    if ( ch == '\\' )
                    {
                        ch = ((LPCSTR) cszTextW)[i++];
                        switch ( ch )
                        {
                            case 't':
                                ((LPSTR) szSearchTextW)[k++] = '\t';
                                break;
                            case 'n':
                                ((LPSTR) szSearchTextW)[k++] = '\r';
                                break;
                            case '\\':
                            case '*':
                            case '?':
                                if ( bMask )
                                {
                                    ((LPSTR) szSearchTextW)[k++] = '\\';
                                }
                            default:
                                ((LPSTR) szSearchTextW)[k++] = ch;
                                break;
                        }
                    }
                    else
                    {
                        ((LPSTR) szSearchTextW)[k++] = ch;
                    }
                    if ( ch == 0 )
                    {
                        return szSearchTextW;
                    }
                }
            }
        }
        else
        {
            wchar_t wch;

            i = 0;
            while ( cszTextW[i] )
            {
                switch ( cszTextW[i] )
                {
                    case L'*':
                    case L'?':
                        bMask = TRUE;
                        break;
                    case L'\\':
                        bEscapeChars = TRUE;
                        if ( cszTextW[i + 1] )
                            ++i;
                        break;
                    default:
                        break;
                }
                ++i;
            }

            if ( bMask )
            {
                *pbSearchEx = TRUE;
            }

            if ( bEscapeChars )
            {
                i = 0;
                k = 0;
                while ( 1 )
                {
                    wch = cszTextW[i++];
                    if ( wch == L'\\' )
                    {
                        wch = cszTextW[i++];
                        switch ( wch )
                        {
                            case L't':
                                szSearchTextW[k++] = L'\t';
                                break;
                            case L'n':
                                szSearchTextW[k++] = L'\r';
                                break;
                            case L'\\':
                            case L'*':
                            case L'?':
                                if ( bMask )
                                {
                                    szSearchTextW[k++] = L'\\';
                                }
                            default:
                                szSearchTextW[k++] = wch;
                                break;
                        }
                    }
                    else
                    {
                        szSearchTextW[k++] = wch;
                    }
                    if ( wch == 0 )
                    {
                        return szSearchTextW;
                    }
                }
            }
        }
    }
    return cszTextW;
}

static void setSearchProgressBarState(BOOL bIsSearching, int nSearchRange)
{
    HWND hProgressBar = GetDlgItem(g_QSearchDlg.hDlg, IDC_PB_PROGRESS);
    if ( bIsSearching )
    {
        SendMessage( hProgressBar, PBM_SETRANGE32, 0, nSearchRange );
        SendMessage( hProgressBar, PBM_SETPOS, 0, 0 );
        SendMessage( hProgressBar, PBM_SETSTEP, 1, 0 );
        ShowWindow( GetDlgItem(g_QSearchDlg.hDlg, IDC_CH_MATCHCASE), SW_HIDE );
        ShowWindow( GetDlgItem(g_QSearchDlg.hDlg, IDC_CH_WHOLEWORD), SW_HIDE );
        ShowWindow( GetDlgItem(g_QSearchDlg.hDlg, IDC_CH_HIGHLIGHTALL), SW_HIDE );
        ShowWindow( hProgressBar, SW_SHOWNORMAL );
    }
    else
    {
        ShowWindow( hProgressBar, SW_HIDE );
        ShowWindow( GetDlgItem(g_QSearchDlg.hDlg, IDC_CH_MATCHCASE), SW_SHOWNORMAL );
        ShowWindow( GetDlgItem(g_QSearchDlg.hDlg, IDC_CH_WHOLEWORD), SW_SHOWNORMAL );
        ShowWindow( GetDlgItem(g_QSearchDlg.hDlg, IDC_CH_HIGHLIGHTALL), SW_SHOWNORMAL );
    }
}

static void updateSearchProgressBar(void)
{
    HWND hProgressBar = GetDlgItem(g_QSearchDlg.hDlg, IDC_PB_PROGRESS);
    SendMessage( hProgressBar, PBM_STEPIT, 0, 0 );
}

static BOOL hasLastAsteriskA(const char* maskA, int len)
{
    if ( maskA[--len] == '*' )
    {
        int k = 0;

        while ( (len > 0) && (maskA[--len] == '\\') )
        {
            ++k;
        }
        if ( (k % 2) == 0 )
            return TRUE;
    }
    return FALSE;
}

static int prepareFindMasksA(
  LPCSTR cszMaskEx, 
  char   outMasksA[MAX_LINES_TO_CHECK][MAX_MASK_SIZE], 
  /*int    outMasksLen[MAX_LINES_TO_CHECK],*/
  BOOL*  lpbWholeLastLine,
  BOOL*  lpbExactBeginning,
  BOOL*  lpbExactEnding)
{
    int nLines;
    int i;
    int k;
    int n;

    *lpbWholeLastLine = FALSE;
    *lpbExactBeginning = FALSE;
    *lpbExactEnding = FALSE;
    nLines = 1;
    k = 0;
    i = 0;
    while ( cszMaskEx[i] )
    {
        if ( cszMaskEx[i] == '\r' )
        {
            for ( n = 0; n < i - k; n++ )
            {
                outMasksA[nLines - 1][n] = cszMaskEx[k + n];
            }
            if ( (n > 0) || (nLines > 1) || (*lpbExactBeginning) )
            {
                if ( n > 0 )
                {
                    *lpbWholeLastLine = hasLastAsteriskA(outMasksA[nLines - 1], n);
                }
                outMasksA[nLines - 1][n] = 0;
                /*outMasksLen[nLines - 1] = n;*/
                ++nLines;
                if ( nLines > MAX_LINES_TO_CHECK )
                {
                    char szMessageA[100];

                    wsprintfA( 
                      szMessageA, 
                      "Can not search for more than %u text lines",
                      MAX_LINES_TO_CHECK
                    );
                    MessageBoxA(
                      g_Plugin.hMainWnd, 
                      szMessageA,
                      "QSearch Error",
                      MB_OK | MB_ICONERROR
                    );
                    return 0; // error
                }
            }
            else
            {
                *lpbExactBeginning = TRUE;
            }
            k = i + 1;
        }
        ++i;
    }
    for ( n = 0; n < i - k; n++ )
    {
        outMasksA[nLines - 1][n] = cszMaskEx[k + n];
    }
    if ( n > 0 )
    {
        if ( hasLastAsteriskA(outMasksA[nLines - 1], n) )
        {
            *lpbWholeLastLine = FALSE; //TRUE;
        }
        else
        {
            outMasksA[nLines - 1][n++] = '*';
            *lpbWholeLastLine = FALSE;
        }
        outMasksA[nLines - 1][n] = 0;
        /*outMasksLen[nLines - 1] = n;*/
    }
    else
    {
        --nLines;
        *lpbExactEnding = TRUE;
    }

    return nLines;
}

INT_X doFindTextExA(HWND hEd, TEXTFINDA* ptfA)
{
    /*int       pnMasksLen[MAX_LINES_TO_CHECK];*/
    char      szMasksA[MAX_LINES_TO_CHECK][MAX_MASK_SIZE];
    char*     pszLineA;
    int       nLinesToCheck;
    int       nLine;
    int       n;
    int       len;
    BOOL      bIsProgressBarVisible;
    BOOL      bSearchUp;
    BOOL      bWholeLastLine;
    BOOL      bExactBeginning;
    BOOL      bExactEnding;
    BOOL      bBack;
    int       nWholeWord;
    int       nEditMaxLine;
    int       nEditStartLine;
    INT_X       pos1;
    INT_X       pos2;
    CHARRANGE_X cr = { 0, 0 };

    pszLineA = (char *) szLineBufW;

    nLinesToCheck = prepareFindMasksA(ptfA->pFindIt, 
      szMasksA, /*pnMasksLen,*/ &bWholeLastLine, &bExactBeginning, &bExactEnding);
    if ( nLinesToCheck == 0 )
        return 0; // error

    nEditMaxLine = (int) SendMessage(hEd, EM_GETLINECOUNT, 0, 0);
    nEditMaxLine -= nLinesToCheck;
    if ( nEditMaxLine < 0 )
        return (-1); // not found

    if ( (ptfA->dwFlags & FR_MATCHCASE) != FR_MATCHCASE )
    {
        for ( n = 0; n < nLinesToCheck; n++ )
        {
            CharLowerA(szMasksA[n]);
        }
    }

    bSearchUp = ((ptfA->dwFlags & FR_UP) == FR_UP) ? TRUE : FALSE;
    nWholeWord = ((ptfA->dwFlags & FR_WHOLEWORD) == FR_WHOLEWORD) ? 1 : 0;
    pos1 = -1;
    nLine = bSearchUp ? (nLinesToCheck - 1) : 0;
    if ( (ptfA->dwFlags & FR_BEGINNING) != FR_BEGINNING )
    {
        SendMessage( hEd, EM_EXGETSEL_X, 0, (LPARAM) &cr );
        if ( cr.cpMin != cr.cpMax )
        {
            if ( bSearchUp )
                --cr.cpMax;
            else
                ++cr.cpMin;
        }
        nLine = (int) SendMessage(hEd, EM_EXLINEFROMCHAR, 
                        0, bSearchUp ? cr.cpMax : cr.cpMin);
    }
    nEditStartLine = nLine;

    bIsProgressBarVisible = FALSE;

    while ( bSearchUp ? (nLine >= nLinesToCheck - 1) : (nLine <= nEditMaxLine) )
    {
        pos1 = -1;

        for ( n = 0; n < nLinesToCheck; n++ )
        {
            *((WORD *) pszLineA) = MAX_LINE_SIZE - 1;
            len = (int) SendMessageA( 
              hEd, 
              EM_GETLINE, 
              bSearchUp ? (nLine - nLinesToCheck + 1 + n) : (nLine + n), 
              (LPARAM) pszLineA 
            );
            pszLineA[len] = 0;
            if ( (len - 1 >= 0) && (pszLineA[len - 1] == '\n') )
            {
                --len;
                pszLineA[len] = 0;
            }
            if ( (len - 1 >= 0) && (pszLineA[len - 1] == '\r') )
            {
                --len;
                pszLineA[len] = 0;
            }

            pos2 = 0;
            bBack = FALSE;
            if ( (!pszLineA[0]) && szMasksA[n][0] && 
                 ((nLinesToCheck == 1) || (szMasksA[n][0] != '*')) )
            {
                break;
            }
            else
            {
                char* ptr;
                char* pszMaskA;
                BOOL  bMatched = FALSE;

                pszMaskA = (char *) szMasksA[n];

                if ( nLine == nEditStartLine )
                {
                    if ( bSearchUp )
                    {
                        if ( n == nLinesToCheck - 1 )
                        {
                            if ( !bExactEnding )
                            {
                                pos2 = cr.cpMax;
                                pos2 -= (INT_X) SendMessage(hEd, EM_LINEINDEX, nLine, 0);
                                pszLineA[pos2] = 0;
                                --pos2;
                                if ( n > 0 )  pos2 = 0; // after '\n', exact beginning
                                bBack = TRUE;
                            }
                            else
                                break;
                        }
                    }
                    else
                    {
                        if ( n == 0 ) 
                        {
                            if ( !bExactBeginning )
                            {
                                pos2 = cr.cpMin;
                                pos2 -= (INT_X) SendMessage(hEd, EM_LINEINDEX, nLine, 0);
                                //++pos2;
                            }
                            else
                                break;
                        }
                    }
                }
                else
                {
                    if ( (/*bExactEnding ||*/ bSearchUp) && 
                         (n == nLinesToCheck - 1) )
                    {
                        if ( !bExactBeginning )
                        {
                            if ( n == 0 )
                            {
                                while ( *pszMaskA == '*' )
                                {
                                    ++pszMaskA;
                                    if ( *pszMaskA == 0 )
                                    {
                                        --pszMaskA;
                                        break;
                                    }
                                }
                                pos2 = (len > 0) ? (len - 1) : 0;
                            }
                            else
                            {
                                pos2 = 0; // after '\n', exact beginning
                            }
                            bBack = TRUE;
                        }
                    }
                }

                if ( (ptfA->dwFlags & FR_MATCHCASE) != FR_MATCHCASE )
                {
                    CharLowerA(pszLineA);
                }

                while ( (pos2 >= 0) && ((len > 0) ? (pos2 < len) : (pos2 <= len)) )
                {
                    if ( (match_mask(pszMaskA, pszLineA + pos2, &ptr, nWholeWord) > 0) && 
                         ((!bExactBeginning) || (pos2 == 0)) )
                    {
                        if ( pos1 < 0 )
                        {
                            pos1 = (INT_X) SendMessage(hEd, EM_LINEINDEX, 
                              bSearchUp ? (nLine - nLinesToCheck + 1) : nLine, 0);
                            pos1 += pos2;
                        }
                        bMatched = TRUE;
                        break;
                    }
                    else
                    {
                        if ( bBack )
                        {
                            --pos2;
                        }
                        else if ( (n == 0) && !bExactBeginning )
                        {
                            // When (n == 0), szMasksA[n][0] is the same as 
                            // szMasksA[0][0];
                            // if szMasksA[n] is started with "*" or "?*",
                            // it matches ANY beginning part of a string.
                            if ( nWholeWord || 
                                ((szMasksA[0][0] != '*') &&
                                 ((szMasksA[0][0] != '?') || (szMasksA[0][1] != '*'))) )
                                ++pos2;
                            else
                                break;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                if ( bMatched )
                    pos2 = (int) (ptr - pszLineA);
                else
                    break;
            }
        }

        if ( n == nLinesToCheck )
        {
            if ( bWholeLastLine )
            {
                pos2 = (INT_X) SendMessage(hEd, EM_LINEINDEX, 
                  bSearchUp ? nLine : (nLine + n - 1), 0);
                pos2 += (int) SendMessageA(hEd, EM_LINELENGTH, pos2, 0);
                if ( bSearchUp && (pos2 > cr.cpMax) )
                {
                    pos2 = cr.cpMax;
                }
            }
            else
            {
                pos2 += (INT_X) SendMessage(hEd, EM_LINEINDEX, 
                  bSearchUp ? nLine : (nLine + n - 1), 0);
            }
            cr.cpMin = pos1;
            cr.cpMax = pos2;
            SendMessage( hEd, EM_EXSETSEL_X, 0, (LPARAM) &cr );

            if ( bIsProgressBarVisible )
            {
                setSearchProgressBarState(FALSE, 0);
            }

            return 1; // found
        }

        if ( bIsProgressBarVisible )
        {
            if ( bSearchUp ? 
                   (((nEditStartLine - nLine) % LINES_TO_SKIP) == 0) :
                     (((nLine - nEditStartLine) % LINES_TO_SKIP) == 0) )
            {
                updateSearchProgressBar();
            }
        }
        else
        {
            if ( bSearchUp )
            {
                if ( (nEditStartLine - nLine == LINES_TO_SKIP) &&
                     (nLine > 2*LINES_TO_SKIP) )
                {
                    bIsProgressBarVisible = TRUE;
                    setSearchProgressBarState( 
                      TRUE, 
                      nEditStartLine/LINES_TO_SKIP 
                    );
                    updateSearchProgressBar();
                }
            }
            else
            {
                if ( (nLine - nEditStartLine == LINES_TO_SKIP) &&
                     (nEditMaxLine - nLine > 2*LINES_TO_SKIP) )
                {
                    bIsProgressBarVisible = TRUE;
                    setSearchProgressBarState( 
                      TRUE, 
                      (nEditMaxLine - nEditStartLine)/LINES_TO_SKIP 
                    );
                    updateSearchProgressBar();
                }
            }
        }

        if ( bSearchUp )
            --nLine;
        else
            ++nLine;
    }

    if ( bIsProgressBarVisible )
    {
        setSearchProgressBarState(FALSE, 0);
    }

    return (-1); // not found
}

static BOOL hasLastAsteriskW(const wchar_t* maskW, int len)
{
    if ( maskW[--len] == L'*' )
    {
        int k = 0;

        while ( (len > 0) && (maskW[--len] == L'\\') )
        {
            ++k;
        }
        if ( (k % 2) == 0 )
            return TRUE;
    }
    return FALSE;
}

static int prepareFindMasksW(
  LPCWSTR cszMaskEx, 
  wchar_t outMasksW[MAX_LINES_TO_CHECK][MAX_MASK_SIZE], 
  /*int     outMasksLen[MAX_LINES_TO_CHECK],*/
  BOOL*   lpbWholeLastLine,
  BOOL*   lpbExactBeginning,
  BOOL*   lpbExactEnding)
{
    int nLines;
    int i;
    int k;
    int n;

    *lpbWholeLastLine = FALSE;
    *lpbExactBeginning = FALSE;
    *lpbExactEnding = FALSE;
    nLines = 1;
    k = 0;
    i = 0;
    while ( cszMaskEx[i] )
    {
        if ( cszMaskEx[i] == L'\r' )
        {
            for ( n = 0; n < i - k; n++ )
            {
                outMasksW[nLines - 1][n] = cszMaskEx[k + n];
            }
            if ( (n > 0) || (nLines > 1) || (*lpbExactBeginning) )
            {
                if ( n > 0 )
                {
                    *lpbWholeLastLine = hasLastAsteriskW(outMasksW[nLines - 1], n);
                }
                outMasksW[nLines - 1][n] = 0;
                /*outMasksLen[nLines - 1] = n;*/
                ++nLines;
                if ( nLines > MAX_LINES_TO_CHECK )
                {
                    wchar_t szMessageW[100];

                    wsprintfW( 
                      szMessageW, 
                      L"Can not search for more than %u text lines",
                      MAX_LINES_TO_CHECK
                    );
                    MessageBoxW(
                      g_Plugin.hMainWnd, 
                      szMessageW,
                      L"QSearch Error",
                      MB_OK | MB_ICONERROR
                    );
                    return 0; // error
                }
            }
            else
            {
                *lpbExactBeginning = TRUE;
            }
            k = i + 1;
        }
        ++i;
    }
    for ( n = 0; n < i - k; n++ )
    {
        outMasksW[nLines - 1][n] = cszMaskEx[k + n];
    }
    if ( n > 0 )
    {
        if ( hasLastAsteriskW(outMasksW[nLines - 1], n) )
        {
            *lpbWholeLastLine = FALSE; //TRUE;
        }
        else
        {
            outMasksW[nLines - 1][n++] = L'*';
            *lpbWholeLastLine = FALSE;
        }
        outMasksW[nLines - 1][n] = 0;
        /*outMasksLen[nLines - 1] = n;*/
    }
    else
    {
        --nLines;
        *lpbExactEnding = TRUE;
    }

    return nLines;
}

INT_X doFindTextExW(HWND hEd, TEXTFINDW* ptfW)
{
    /*int       pnMasksLen[MAX_LINES_TO_CHECK];*/
    wchar_t   szMasksW[MAX_LINES_TO_CHECK][MAX_MASK_SIZE];
    wchar_t*  pszLineW;
    int       nLinesToCheck;
    int       nLine;
    int       n;
    int       len;
    BOOL      bIsProgressBarVisible;
    BOOL      bSearchUp;
    BOOL      bWholeLastLine;
    BOOL      bExactBeginning;
    BOOL      bExactEnding;
    BOOL      bBack;
    int       nWholeWord;
    int       nEditMaxLine;
    int       nEditStartLine;
    INT_X       pos1;
    INT_X       pos2;
    CHARRANGE_X cr = { 0, 0 };

    pszLineW = (wchar_t *) szLineBufW;

    nLinesToCheck = prepareFindMasksW(ptfW->pFindIt, 
      szMasksW, /*pnMasksLen,*/ &bWholeLastLine, &bExactBeginning, &bExactEnding);
    if ( nLinesToCheck == 0 )
        return 0; // error

    nEditMaxLine = (int) SendMessage(hEd, EM_GETLINECOUNT, 0, 0);
    nEditMaxLine -= nLinesToCheck;
    if ( nEditMaxLine < 0 )
        return (-1); // not found

    if ( (ptfW->dwFlags & FR_MATCHCASE) != FR_MATCHCASE )
    {
        for ( n = 0; n < nLinesToCheck; n++ )
        {
            CharLowerW(szMasksW[n]);
        }
    }

    bSearchUp = ((ptfW->dwFlags & FR_UP) == FR_UP) ? TRUE : FALSE;
    nWholeWord = ((ptfW->dwFlags & FR_WHOLEWORD) == FR_WHOLEWORD) ? 1 : 0;
    pos1 = -1;
    nLine = bSearchUp ? (nLinesToCheck - 1) : 0;
    if ( (ptfW->dwFlags & FR_BEGINNING) != FR_BEGINNING )
    {
        SendMessage( hEd, EM_EXGETSEL_X, 0, (LPARAM) &cr );
        if ( cr.cpMin != cr.cpMax )
        {
            if ( bSearchUp )
                --cr.cpMax;
            else
                ++cr.cpMin;
        }
        nLine = (int) SendMessage(hEd, EM_EXLINEFROMCHAR, 
                        0, bSearchUp ? cr.cpMax : cr.cpMin);
    }
    nEditStartLine = nLine;

    bIsProgressBarVisible = FALSE;

    while ( bSearchUp ? (nLine >= nLinesToCheck - 1) : (nLine <= nEditMaxLine) )
    {
        pos1 = -1;

        for ( n = 0; n < nLinesToCheck; n++ )
        {
            *((WORD *) pszLineW) = MAX_LINE_SIZE - 1;
            len = (int) SendMessageW( 
              hEd, 
              EM_GETLINE, 
              bSearchUp ? (nLine - nLinesToCheck + 1 + n) : (nLine + n), 
              (LPARAM) pszLineW 
            );
            pszLineW[len] = 0;
            if ( (len - 1 >= 0) && (pszLineW[len - 1] == L'\n') )
            {
                --len;
                pszLineW[len] = 0;
            }
            if ( (len - 1 >= 0) && (pszLineW[len - 1] == L'\r') )
            {
                --len;
                pszLineW[len] = 0;
            }

            pos2 = 0;
            bBack = FALSE;
            if ( (!pszLineW[0]) && szMasksW[n][0] && 
                 ((nLinesToCheck == 1) || (szMasksW[n][0] != L'*')) )
            {
                break;
            }
            else
            {
                wchar_t* ptr;
                wchar_t* pszMaskW;
                BOOL     bMatched = FALSE;

                pszMaskW = (wchar_t *) szMasksW[n];

                if ( nLine == nEditStartLine )
                {
                    if ( bSearchUp )
                    {
                        if ( n == nLinesToCheck - 1 )
                        {
                            if ( !bExactEnding )
                            {
                                pos2 = cr.cpMax;
                                pos2 -= (INT_X) SendMessage(hEd, EM_LINEINDEX, nLine, 0);
                                pszLineW[pos2] = 0;
                                --pos2;
                                if ( n > 0 )  pos2 = 0; // after '\n', exact beginning
                                bBack = TRUE;
                            }
                            else
                                break;
                        }
                    }
                    else
                    {
                        if ( n == 0 )
                        {
                            if ( !bExactBeginning )
                            {
                                pos2 = cr.cpMin;
                                pos2 -= (INT_X) SendMessage(hEd, EM_LINEINDEX, nLine, 0);
                                //++pos2;
                            }
                            else
                                break;
                        }
                    }
                }
                else
                {
                    if ( (/*bExactEnding ||*/ bSearchUp) && 
                         (n == nLinesToCheck - 1) )
                    {
                        if ( !bExactBeginning )
                        {
                            if ( n == 0 )
                            {
                                while ( *pszMaskW == L'*' )
                                {
                                    ++pszMaskW;
                                    if ( *pszMaskW == 0 )
                                    {
                                        --pszMaskW;
                                        break;
                                    }
                                }
                                pos2 = (len > 0) ? (len - 1) : 0;
                            }
                            else
                            {
                                pos2 = 0; // after '\n', exact beginning
                            }
                            bBack = TRUE;
                        }
                    }
                }

                if ( (ptfW->dwFlags & FR_MATCHCASE) != FR_MATCHCASE )
                {
                    CharLowerW(pszLineW);
                }

                while ( (pos2 >= 0) && ((len > 0) ? (pos2 < len) : (pos2 <= len)) )
                {
                    if ( (match_maskw(pszMaskW, pszLineW + pos2, &ptr, nWholeWord) > 0) &&
                         ((!bExactBeginning) || (pos2 == 0)) )
                    {
                        if ( pos1 < 0 )
                        {
                            pos1 = (INT_X) SendMessage(hEd, EM_LINEINDEX, 
                              bSearchUp ? (nLine - nLinesToCheck + 1) : nLine, 0);
                            pos1 += pos2;
                        }
                        bMatched = TRUE;
                        break;
                    }
                    else
                    {
                        if ( bBack )
                        {
                            --pos2;
                        }
                        else if ( (n == 0) && !bExactBeginning )
                        {
                            // When (n == 0), szMasksW[n][0] is the same as 
                            // szMasksW[0][0];
                            // if szMasksW[n] is started with L"*" or L"?*",
                            // it matches ANY beginning part of a string.
                            if ( nWholeWord || 
                                ((szMasksW[0][0] != L'*') &&
                                 ((szMasksW[0][0] != L'?') || (szMasksW[0][1] != L'*'))) )
                                ++pos2;
                            else
                                break;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                if ( bMatched )
                    pos2 = (int) (ptr - pszLineW);
                else
                    break;
            }
        }

        if ( n == nLinesToCheck )
        {
            if ( bWholeLastLine )
            {
                pos2 = (INT_X) SendMessage(hEd, EM_LINEINDEX, 
                  bSearchUp ? nLine : (nLine + n - 1), 0);
                pos2 += (int) SendMessageW(hEd, EM_LINELENGTH, pos2, 0);
                if ( bSearchUp && (pos2 > cr.cpMax) )
                {
                    pos2 = cr.cpMax;
                }
            }
            else
            {
                pos2 += (INT_X) SendMessage(hEd, EM_LINEINDEX, 
                  bSearchUp ? nLine : (nLine + n - 1), 0);
            }
            cr.cpMin = pos1;
            cr.cpMax = pos2;
            SendMessage( hEd, EM_EXSETSEL_X, 0, (LPARAM) &cr );

            if ( bIsProgressBarVisible )
            {
                setSearchProgressBarState(FALSE, 0);
            }

            return 1; // found
        }

        if ( bIsProgressBarVisible )
        {
            if ( bSearchUp ? 
                   (((nEditStartLine - nLine) % LINES_TO_SKIP) == 0) :
                     (((nLine - nEditStartLine) % LINES_TO_SKIP) == 0) )
            {
                updateSearchProgressBar();
            }
        }
        else
        {
            if ( bSearchUp )
            {
                if ( (nEditStartLine - nLine == LINES_TO_SKIP) &&
                     (nLine > 2*LINES_TO_SKIP) )
                {
                    bIsProgressBarVisible = TRUE;
                    setSearchProgressBarState( 
                      TRUE, 
                      nEditStartLine/LINES_TO_SKIP 
                    );
                    updateSearchProgressBar();
                }
            }
            else
            {
                if ( (nLine - nEditStartLine == LINES_TO_SKIP) &&
                     (nEditMaxLine - nLine > 2*LINES_TO_SKIP) )
                {
                    bIsProgressBarVisible = TRUE;
                    setSearchProgressBarState( 
                      TRUE, 
                      (nEditMaxLine - nEditStartLine)/LINES_TO_SKIP 
                    );
                    updateSearchProgressBar();
                }
            }
        }

        if ( bSearchUp )
            --nLine;
        else
            ++nLine;
    }

    if ( bIsProgressBarVisible )
    {
        setSearchProgressBarState(FALSE, 0);
    }

    return (-1); // not found
}
