#include "find_in_sorted_array.h"

// returns either a 0-based index or -1
int find_in_sorted_array(const INT_PTR* pArr, unsigned int nItems, INT_PTR val, BOOL* pbExactMatch)
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
