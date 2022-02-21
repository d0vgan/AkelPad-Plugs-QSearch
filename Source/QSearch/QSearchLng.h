#ifndef _qsearch_lng_h_
#define _qsearch_lng_h_
//---------------------------------------------------------------------------
#include <windows.h>


// lng funcs
void           qsearchSetMainDlgLang(HWND hDlg);
void           qsearchSetFindAllSettDlgLang(HWND hDlg);
void           qsearchSetPopupMenuLang(HMENU hPopupMenu);
void           qsearchSetFindAllPopupMenuLang(HMENU hFindAllPopupMenu);
const char*    qsearchGetHintA(unsigned int uDlgItemID);
const wchar_t* qsearchGetHintW(unsigned int uDlgItemID);
const char*    qsearchGetTextA(unsigned int uDlgItemID);
const wchar_t* qsearchGetTextW(unsigned int uDlgItemID);

#define QS_STRID_FINDALL_SEARCHINGFOR            101
#define QS_STRID_FINDALL_OCCURRENCESFOUND        102
#define QS_STRID_FINDALL_SEARCHINGFORINFILES     103
#define QS_STRID_FINDALL_OCCURRENCESFOUNDINFILES 104
#define QS_STRID_FINDALL_OUTPUT_CTX_LINES        121
#define QS_STRID_FINDALL_OUTPUT_CTX_LINESCR      122
#define QS_STRID_FINDALL_OUTPUT_CTX_CHARSINLINE  123
#define QS_STRID_FINDALL_OUTPUT_CTX_CHARS        124
#define QS_STRID_FINDALL_OUTPUT_DST_COUNTONLY    141
#define QS_STRID_FINDALL_OUTPUT_DST_LOG          142
#define QS_STRID_FINDALL_OUTPUT_DST_FILEN        143
#define QS_STRID_FINDALL_OUTPUT_DST_FILE1        144
const wchar_t* qsearchGetStringW(unsigned int uStringID);


//---------------------------------------------------------------------------
#endif
