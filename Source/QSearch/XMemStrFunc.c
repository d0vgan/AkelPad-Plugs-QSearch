#include "XMemStrFunc.h"

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
    if ( nBytes != 0 )
    {
        unsigned char *pDestByte = (unsigned char *) pDest;

        for ( ; ; )
        {
            *pDestByte = (unsigned char) c;
            if ( --nBytes == 0 )
                break;
            ++pDestByte;
        }
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
