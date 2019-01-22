#ifndef _qsearch_lng_h_
#define _qsearch_lng_h_
//---------------------------------------------------------------------------
#include <windows.h>


// lng funcs
void           qsearchSetDialogLang(HWND hDlg);
void           qsearchSetPopupMenuLang(HMENU hPopupMenu);
const char*    qsearchGetHintA(unsigned int uDlgItemID);
const wchar_t* qsearchGetHintW(unsigned int uDlgItemID);

#define QS_STRID_FINDALL_SEARCHINGFOR      101
#define QS_STRID_FINDALL_OCCURRENCESFOUND  102
const wchar_t* qsearchGetStringW(unsigned int uStringID);


//---------------------------------------------------------------------------
#endif
