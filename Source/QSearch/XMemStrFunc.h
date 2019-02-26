#ifndef _mem_str_func_h_
#define _mem_str_func_h_
//---------------------------------------------------------------------------
#include <windows.h>

#define SysMemAlloc(sizeInBytes) (void *) GlobalAlloc( GMEM_FIXED, (sizeInBytes) )
#define SysMemFree(ptr)          GlobalFree( (HGLOBAL) (ptr) )

// memcpy
void x_mem_cpy(void *pDest, const void *pSrc, UINT_PTR nBytes);

// memset
void x_mem_set(void *pDest, unsigned int c, UINT_PTR nBytes);

// ZeroMemory
void x_zero_mem(void* pDest, UINT_PTR nBytes);

// allocates a memory block; to be freed with x_mem_free
void* x_mem_alloc(unsigned int nSizeInBytes);

// deallocates a memory block allocated with x_mem_alloc
void  x_mem_free(void* ptr);

// Note: str and substr must not be NULL!
BOOL x_wstr_startswith(const WCHAR* str, const WCHAR* substr);
BOOL x_wstr_endswith(const WCHAR* str, int nStrLen, const WCHAR* substr, int nSubstrLen);

// tDynamicBuffer
typedef struct sDynamicBuffer {
    void* ptr; // pointer to the allocated memory
    UINT_PTR nBytesAllocated; // number of bytes allocated
    UINT_PTR nBytesStored; // number of bytes currently stored in ptr
} tDynamicBuffer;

void tDynamicBuffer_Init(tDynamicBuffer* pBuf);
void tDynamicBuffer_Free(tDynamicBuffer* pBuf);
BOOL tDynamicBuffer_Allocate(tDynamicBuffer* pBuf, UINT_PTR nBytesToAllocate);
UINT_PTR tDynamicBuffer_Append(tDynamicBuffer* pBuf, const void* pData, UINT_PTR nBytes);

//---------------------------------------------------------------------------
#endif
