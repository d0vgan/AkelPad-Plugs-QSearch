#include "find_in_sorted_array.h"

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
