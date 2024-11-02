#ifndef _SOURCES_FIND_IN_SORTED_ARRAY_H_
#define _SOURCES_FIND_IN_SORTED_ARRAY_H_

#include <windows.h>

typedef unsigned __int64 matchpos_t;

matchpos_t to_matchpos(unsigned int line, unsigned int pos_in_line);
unsigned int get_matchpos_line(matchpos_t pos);
unsigned int get_matchpos_pos_in_line(matchpos_t pos);

int find_in_sorted_matchpos_array(const matchpos_t* pArr, unsigned int nItems, matchpos_t val, BOOL* pbExactMatch);

#endif
