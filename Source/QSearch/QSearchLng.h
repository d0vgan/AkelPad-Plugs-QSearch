#ifndef _qsearch_lng_h_
#define _qsearch_lng_h_
//---------------------------------------------------------------------------
#include <windows.h>


// lng funcs
void           qsearchSetDialogLang(HWND hDlg);
void           qsearchSetPopupMenuLang(HMENU hPopupMenu);
const char*    qsearchGetHintA(unsigned int uDlgItemID);
const wchar_t* qsearchGetHintW(unsigned int uDlgItemID);


//---------------------------------------------------------------------------
#endif
