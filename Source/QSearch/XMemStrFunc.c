#include "XMemStrFunc.h"

// Special form of memcmp implementation to avoid
// compiler from replace this code with memcmp call.
int x_mem_cmp(const void *pSrc1, const void *pSrc2, UINT_PTR nBytes)
{
    if ( pSrc1 != pSrc2 )
    {
        const unsigned int* pSrcUint1 = (const unsigned int *) pSrc1;
        const unsigned int* pSrcUint2 = (const unsigned int *) pSrc2;

        if ( nBytes >= sizeof(unsigned int) )
        {
            for ( ; ; )
            {
                if ( *pSrcUint1 != *pSrcUint2 )
                    return ( *pSrcUint1 < *pSrcUint2 ? (-1) : 1 );

                nBytes -= sizeof(unsigned int);
                if ( nBytes < sizeof(unsigned int) )
                {
                    pSrc2 = NULL;
                    break;
                }
                ++pSrcUint1;
                ++pSrcUint2;
            }
        }

        if ( nBytes != 0 )
        {
            const unsigned char* pSrcByte1;
            const unsigned char* pSrcByte2;

            if ( pSrc2 == NULL )
            {
                ++pSrcUint1;
                ++pSrcUint2;
            }
            pSrcByte1 = (const unsigned char *) pSrcUint1;
            pSrcByte2 = (const unsigned char *) pSrcUint2;

            for ( ; ; )
            {
                if ( *pSrcByte1 != *pSrcByte2 )
                    return ( *pSrcByte1 < *pSrcByte2 ? (-1) : 1 );
                if ( --nBytes == 0 )
                    break;
                ++pSrcByte1;
                ++pSrcByte2;
            }
        }
    }

    return 0; // equal
}

// Special form of memcpy implementation to avoid
// compiler from replace this code with memcpy call.
void x_mem_cpy(void *pDest, const void *pSrc, UINT_PTR nBytes)
{
    if ( pDest != pSrc )
    {
        unsigned int* pDestUint = (unsigned int *) pDest;
        const unsigned int* pSrcUint = (const unsigned int *) pSrc;

        if ( nBytes >= sizeof(unsigned int) )
        {
            for ( ; ; )
            {
                *pDestUint = *pSrcUint;
                nBytes -= sizeof(unsigned int);
                if ( nBytes < sizeof(unsigned int) )
                {
                    pDest = NULL;
                    break;
                }
                ++pDestUint;
                ++pSrcUint;
            }
        }

        if ( nBytes != 0 )
        {
            unsigned char* pDestByte;
            const unsigned char* pSrcByte;

            if ( pDest == NULL )
            {
                ++pDestUint;
                ++pSrcUint;
            }
            pDestByte = (unsigned char *) pDestUint;
            pSrcByte = (const unsigned char *) pSrcUint;

            for ( ; ; )
            {
                *pDestByte = *pSrcByte;
                if ( --nBytes == 0 )
                    break;
                ++pDestByte;
                ++pSrcByte;
            }
        }
    }
}

// Special form of memset implementation to avoid
// compiler from replace this code with memset call.
void x_mem_set(void *pDest, unsigned int c, UINT_PTR nBytes)
{
    c &= 0xFF;

    if ( c != 0 )
    {
        unsigned int* pDestUint = (unsigned int *) pDest;

        if ( nBytes >= sizeof(unsigned int) )
        {
            unsigned int nValue = c + (c << 8) + (c << 16) + (c << 24);

            for ( ; ; )
            {
                *pDestUint = nValue;
                nBytes -= sizeof(unsigned int);
                if ( nBytes < sizeof(unsigned int) )
                {
                    pDest = NULL;
                    break;
                }
                ++pDestUint;
            }
        }

        if ( nBytes != 0 )
        {
            unsigned char *pDestByte;

            if ( pDest == NULL )
            {
                ++pDestUint;
            }
            pDestByte = (unsigned char *) pDestUint;

            for ( ; ; )
            {
                *pDestByte = (unsigned char) c;
                if ( --nBytes == 0 )
                    break;
                ++pDestByte;
            }
        }
    }
    else
    {
        x_zero_mem(pDest, nBytes);
    }
}

// Special form of ZeroMemory implementation to avoid
// compiler from replace this code with memset call
void x_zero_mem(void* pDest, UINT_PTR nBytes)
{
    unsigned int* pDestUint = (unsigned int *) pDest;

    if ( nBytes >= sizeof(unsigned int) )
    {
        for ( ; ; )
        {
            *pDestUint = 0;
            nBytes -= sizeof(unsigned int);
            if ( nBytes < sizeof(unsigned int) )
            {
                pDest = NULL;
                break;
            }
            ++pDestUint;
        }
    }

    if ( nBytes != 0 )
    {
        unsigned char *pDestByte;

        if ( pDest == NULL )
        {
            ++pDestUint;
        }
        pDestByte = (unsigned char *) pDestUint;

        for ( ; ; )
        {
            *pDestByte = 0;
            if ( --nBytes == 0 )
                break;
            ++pDestByte;
        }
    }
}

void* x_mem_alloc(unsigned int nSizeInBytes)
{
    return SysMemAlloc(nSizeInBytes);
}

void  x_mem_free(void* ptr)
{
    SysMemFree(ptr);
}

BOOL x_wstr_startswith(const WCHAR* str, const WCHAR* substr)
{
    for ( ; ; )
    {
        if ( *substr == 0 )
            break; // end of substr - OK

        if ( *substr != *str )
            return FALSE; // diff

        ++substr;
        ++str;
    }

    return TRUE;
}

BOOL x_wstr_endswith(const WCHAR* str, int nStrLen, const WCHAR* substr, int nSubstrLen)
{
    if ( nSubstrLen > nStrLen )
        return FALSE;
    
    str += (nStrLen - nSubstrLen);
    for ( ; ; )
    {
        if ( *str != *substr )
            return FALSE; // diff

        if ( *str == 0 )
            break; // end of both strings - OK

        ++str;
        ++substr;
    }

    return TRUE;
}

int x_wstr_findch(const WCHAR* str, WCHAR ch, int nOffset)
{
    const WCHAR* p;
    WCHAR cc;

    for ( p = str + nOffset; (cc = *p) != 0; ++p )
    {
        if ( cc == ch )
            return (int) (p - str);
    }

    return -1; // not found
}

int x_wstr_rfindch(const WCHAR* str, WCHAR ch, int nLen)
{
    if ( nLen > 0 )
    {
        const WCHAR* p;
        WCHAR cc;

        p = str + nLen;
        while ( p > str )
        {
            cc = *(--p);
            if ( cc == ch )
                return (int) (p - str);
        }
    }
    return -1; // not found
}

INT_PTR xatoiW(const wchar_t *wpStr, const wchar_t **wpNext)
{
    INT_PTR nNumber = 0;
    BOOL bMinus = FALSE;

    while (*wpStr == L' ' || *wpStr == L'\t')
        ++wpStr;
    if (*wpStr == L'+')
        ++wpStr;
    else if (*wpStr == L'-')
    {
        bMinus = TRUE;
        ++wpStr;
    }

    while (*wpStr >= L'0' && *wpStr <= L'9')
    {
        nNumber = (nNumber * 10) + (*wpStr - L'0');
        ++wpStr;
    }
    if (bMinus == TRUE) nNumber = 0 - nNumber;
    if (wpNext) *wpNext = wpStr;
    return nNumber;
}

int xitoaW(INT_PTR nNumber, wchar_t *wszStr)
{
    wchar_t wszReverse[128];
    int a;
    int b = 0;

    if (nNumber == 0)
    {
        if (wszStr) wszStr[b] = L'0';
        ++b;
    }
    else if (nNumber < 0)
    {
        if (wszStr) wszStr[b] = L'-';
        ++b;
        nNumber = 0 - nNumber;
    }
    for (a = 0; nNumber != 0; ++a)
    {
        wszReverse[a] = (wchar_t) (nNumber % 10) + L'0';
        nNumber = nNumber / 10;
    }
    if (!wszStr) return a + b + 1;

    while (--a >= 0) wszStr[b++] = wszReverse[a];
    wszStr[b] = L'\0';
    return b;
}

INT_PTR xhextoiW(const wchar_t *wpHexStr)
{
    INT_PTR nNumber = 0;
    INT_PTR nDigit;
    wchar_t ch;

    while (*wpHexStr == L' ' || *wpHexStr == L'\t')
        ++wpHexStr;
    if (*wpHexStr == L'0' && (*(wpHexStr+1) == L'x' || *(wpHexStr+1) == L'X'))
        wpHexStr += 2;

    for (;;)
    {
        ch = *wpHexStr;
        if (ch >= L'0' && ch <= L'9')
            nDigit = ch - L'0';
        else if (ch >= L'a' && ch <= L'f')
            nDigit = ch - L'a' + 10;
        else if (ch >= L'A' && ch <= L'F')
            nDigit = ch - L'A' + 10;
        else
            break;
        nNumber *= 16;
        nNumber += nDigit;
        ++wpHexStr;
    }

    return nNumber;
}

// tDynamicBuffer
void tDynamicBuffer_Init(tDynamicBuffer* pBuf)
{
    x_zero_mem( pBuf, sizeof(tDynamicBuffer) );
}

void tDynamicBuffer_Free(tDynamicBuffer* pBuf)
{
    if ( pBuf->ptr )
    {
        SysMemFree( pBuf->ptr );
        x_zero_mem( pBuf, sizeof(tDynamicBuffer) );
    }
}

BOOL tDynamicBuffer_Allocate(tDynamicBuffer* pBuf, UINT_PTR nBytesToAllocate)
{
    if ( pBuf->nBytesAllocated < nBytesToAllocate )
    {
        // allocate new memory
        tDynamicBuffer newBuf;

        tDynamicBuffer_Init(&newBuf);
        newBuf.ptr = SysMemAlloc( nBytesToAllocate );
        if ( !newBuf.ptr )
            return FALSE; // failed to allocate the memory

        newBuf.nBytesAllocated = nBytesToAllocate;
        
        // free the previously allocated memory
        if ( pBuf->ptr )
            SysMemFree( pBuf->ptr );

        // use the newly allocated memory
        x_mem_cpy( pBuf, &newBuf, sizeof(tDynamicBuffer) );
    }

    return TRUE; // OK
}

UINT_PTR tDynamicBuffer_Append(tDynamicBuffer* pBuf, const void* pData, UINT_PTR nBytes)
{
    UINT_PTR nBytesToStore;

    if ( nBytes == 0 )
        return pBuf->nBytesStored;

    nBytesToStore = pBuf->nBytesStored + nBytes;
    if ( nBytesToStore > pBuf->nBytesAllocated )
    {
        // allocate new memory
        tDynamicBuffer newBuf;
        UINT_PTR nBytesToAllocate;

        nBytesToAllocate = pBuf->nBytesAllocated;
        if ( nBytesToAllocate == 0 )
        {
            nBytesToAllocate = 64;
        }
        else if ( (nBytesToAllocate % 64) != 0 )
        {
            nBytesToAllocate = (1 + nBytesToAllocate/64)*64;
        }
        nBytesToAllocate = (1 + nBytesToStore/nBytesToAllocate)*nBytesToAllocate;
        tDynamicBuffer_Init(&newBuf);
        if ( tDynamicBuffer_Allocate(&newBuf, nBytesToAllocate) != 0 )
        {
            unsigned char* p = (unsigned char *) newBuf.ptr;
            // copy the previous data
            if ( pBuf->nBytesStored != 0 )
            {
                x_mem_cpy( p, pBuf->ptr, pBuf->nBytesStored );
                p += pBuf->nBytesStored;
            }
            // append the new data
            x_mem_cpy(p, pData, nBytes);

            // free the previously allocated memory
            if ( pBuf->ptr )
                SysMemFree( pBuf->ptr );

            // use the newly allocated memory
            x_mem_cpy( pBuf, &newBuf, sizeof(tDynamicBuffer) );
        }
        else
            return 0; // failed to allocate the memory
    }
    else
    {
        // use the existing memory
        unsigned char* p = (unsigned char *) pBuf->ptr;
        p += pBuf->nBytesStored;
        // append the new data
        x_mem_cpy(p, pData, nBytes);
    }

    pBuf->nBytesStored = nBytesToStore;
    return nBytesToStore; // OK
}

void tDynamicBuffer_Clear(tDynamicBuffer* pBuf)
{
    pBuf->nBytesStored = 0;
}
