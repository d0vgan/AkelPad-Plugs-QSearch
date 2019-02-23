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

int tDynamicBuffer_Allocate(tDynamicBuffer* pBuf, UINT_PTR nBytesToAllocate)
{
    if ( pBuf->nBytesAllocated < nBytesToAllocate )
    {
        if ( pBuf->ptr )
            SysMemFree( pBuf->ptr );

        x_zero_mem( pBuf, sizeof(tDynamicBuffer) );
        pBuf->ptr = SysMemAlloc( nBytesToAllocate );
        if ( pBuf->ptr )
            pBuf->nBytesAllocated = nBytesToAllocate;
        else
            return 0; // failed to allocate the memory
    }

    return 1; // OK
}

// tDynamicBufferEx
void tDynamicBufferEx_Init(tDynamicBufferEx* pBuf)
{
    x_zero_mem( pBuf, sizeof(tDynamicBufferEx) );
}

void tDynamicBufferEx_Free(tDynamicBufferEx* pBuf)
{
    tDynamicBuffer_Free( &pBuf->buf );
    pBuf->nBytesStored = 0;
}

int tDynamicBufferEx_Allocate(tDynamicBufferEx* pBuf, UINT_PTR nBytesToAllocate)
{
    pBuf->nBytesStored = 0;
    return tDynamicBuffer_Allocate( &pBuf->buf, nBytesToAllocate );
}

int tDynamicBufferEx_Append(tDynamicBufferEx* pBuf, const void* pData, UINT_PTR nBytes)
{
    unsigned char* p;
    UINT_PTR nBytesAllocated;
    UINT_PTR nBytesToStore;
    tDynamicBuffer newBuf;

    nBytesAllocated = pBuf->buf.nBytesAllocated;
    nBytesToStore = pBuf->nBytesStored + nBytes;
    if ( nBytesToStore > nBytesAllocated )
    {
        // allocate new memory
        UINT_PTR nBytesToAllocate;

        if ( nBytesAllocated == 0 )
        {
            nBytesAllocated = 64;
        }
        else if ( (nBytesAllocated % 64) != 0 )
        {
            nBytesAllocated = (1 + nBytesAllocated/64)*64;
        }
        nBytesToAllocate = (1 + nBytesToStore/nBytesAllocated)*nBytesAllocated;
        tDynamicBuffer_Init(&newBuf);
        if ( tDynamicBuffer_Allocate(&newBuf, nBytesToAllocate) == 0 )
            return 0; // failed to allocate the memory

        p = (unsigned char *) newBuf.ptr;
        x_mem_cpy( p, pBuf->buf.ptr, pBuf->nBytesStored ); // copy the previous data
    }
    else
    {
        // use the existing memory
        p = (unsigned char *) pBuf->buf.ptr;
    }

    p += pBuf->nBytesStored;
    x_mem_cpy(p, pData, nBytes); // append the new data
    if ( nBytesToStore > nBytesAllocated )
    {
        tDynamicBuffer_Free( &pBuf->buf ); // free the previously allocated memory
        x_mem_cpy( &pBuf->buf, &newBuf, sizeof(tDynamicBuffer) ); // use the newly allocated memory
    }

    pBuf->nBytesStored = nBytesToStore;
    return nBytesToStore; // OK
}
