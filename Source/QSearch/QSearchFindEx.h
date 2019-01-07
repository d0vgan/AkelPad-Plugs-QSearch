#ifndef _qsearch_find_ex_h_
#define _qsearch_find_ex_h_
//---------------------------------------------------------------------------
#include <windows.h>
#include "AkelDllHeader.h"

INT_X   doFindTextExA(HWND hEd, TEXTFINDA* ptfA);
INT_X   doFindTextExW(HWND hEd, TEXTFINDW* ptfW);
LPCWSTR getTextToSearch(LPCWSTR cszTextW, BOOL* pbSearchEx);

int     match_mask(const char* mask, const char* str, char** last_pos, int whole_word);
int     match_maskw(const wchar_t* maskw, const wchar_t* strw, wchar_t** last_pos, int whole_word);

//---------------------------------------------------------------------------
#endif
