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

#define QS_STRID_FINDALL_SEARCHINGFOR      101
#define QS_STRID_FINDALL_OCCURRENCESFOUND  102
const wchar_t* qsearchGetStringW(unsigned int uStringID);


//---------------------------------------------------------------------------
#endif
