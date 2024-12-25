#ifndef _qsearch_find_ex_h_
#define _qsearch_find_ex_h_
//---------------------------------------------------------------------------
#include <windows.h>
#include "AkelDllHeader.h"

INT_X   doFindTextExW(HWND hEd, TEXTFINDW* ptfW);
int     findSpecialCharW(LPCWSTR cszTextW);
void    getTextToSearchW(LPCWSTR cszTextW, BOOL* pbSearchEx, const DWORD dwOptFlags[], WCHAR out_pszSearchTextW[]);
int     match_maskw(const wchar_t* maskw, const wchar_t* strw, wchar_t** last_pos, int whole_word);
#ifdef _DEBUG
int     match_maskw0(const wchar_t* maskw, const wchar_t* strw, wchar_t** last_pos, int whole_word);
#endif

#ifdef QS_OLD_WINDOWS
INT_X   doFindTextExA(HWND hEd, TEXTFINDA* ptfA);
int     findSpecialCharA(LPCSTR cszTextA);
void    getTextToSearchA(LPCSTR cszTextA, BOOL* pbSearchEx, const DWORD dwOptFlags[], CHAR out_pszSearchTextA[]);
int     match_mask(const char* mask, const char* str, char** last_pos, int whole_word);
#endif

//---------------------------------------------------------------------------
#endif
