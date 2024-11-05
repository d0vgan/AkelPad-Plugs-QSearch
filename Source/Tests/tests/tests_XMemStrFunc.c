#include "tests_XMemStrFunc.h"
#include "../../QSearch/XMemStrFunc.h"

#include <assert.h>
#include <conio.h>
#include <stdio.h>

void test_x_mem_funcs()
{
    const unsigned char data1[9] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };
    const unsigned char data2[9] = { 0x01, 0x02, 0x03, 0x04, 0x00, 0x06, 0x07, 0x08, 0x09 };
    const unsigned char data3[9] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x00 };
    const unsigned char data4[9] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };
    const unsigned char data5[9] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    const unsigned char data6[9] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    unsigned char        data[9] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    assert(x_mem_cmp(data1, data2, 9) != 0);
    assert(x_mem_cmp(data1, data3, 9) != 0);
    assert(x_mem_cmp(data1, data4, 3) == 0);
    assert(x_mem_cmp(data1, data4, 5) == 0);
    assert(x_mem_cmp(data1, data4, 9) == 0);
    assert(x_mem_cmp(data1, data5, 9) != 0);

    x_zero_mem(data, 2);
    assert(x_mem_cmp(data, data5, 2) == 0);
    x_mem_cpy(data, data1, 2);
    assert(x_mem_cmp(data, data1, 2) == 0);

    x_zero_mem(data, 5);
    assert(x_mem_cmp(data, data5, 5) == 0);
    x_mem_cpy(data, data1, 5);
    assert(x_mem_cmp(data, data1, 5) == 0);

    x_zero_mem(data, 8);
    assert(x_mem_cmp(data, data5, 8) == 0);
    x_mem_cpy(data, data1, 8);
    assert(x_mem_cmp(data, data1, 8) == 0);

    x_zero_mem(data, 9);
    assert(x_mem_cmp(data, data5, 9) == 0);
    x_mem_cpy(data, data1, 9);
    assert(x_mem_cmp(data, data1, 9) == 0);

    x_mem_set(data, 0xFF, 1);
    assert(x_mem_cmp(data, data6, 1) == 0);

    x_mem_set(data, 0xFF, 4);
    assert(x_mem_cmp(data, data6, 4) == 0);

    x_mem_set(data, 0xFF, 9);
    assert(x_mem_cmp(data, data6, 9) == 0);
}

void test_x_wstr_startswith()
{
    const wchar_t* s = L"abc";

    assert(x_wstr_startswith(s, L"a") == TRUE);
    assert(x_wstr_startswith(s, L"ab") == TRUE);
    assert(x_wstr_startswith(s, L"abc") == TRUE);
    assert(x_wstr_startswith(s, L"") == TRUE);

    assert(x_wstr_startswith(s, L"abcd") == FALSE);
    assert(x_wstr_startswith(s, L"aa") == FALSE);
    assert(x_wstr_startswith(s, L"abb") == FALSE);
    assert(x_wstr_startswith(s, L"acc") == FALSE);
    assert(x_wstr_startswith(s, L"acb") == FALSE);
    assert(x_wstr_startswith(s, L"b") == FALSE);
    assert(x_wstr_startswith(s, L"c") == FALSE);
    assert(x_wstr_startswith(s, L"x") == FALSE);
    assert(x_wstr_startswith(s, L"xxx") == FALSE);
}

void test_x_wstr_endswith()
{
    const wchar_t* s = L"abc";
    int len = 3;

    assert(x_wstr_endswith(s, len, L"c", 1) == TRUE);
    assert(x_wstr_endswith(s, len, L"bc", 2) == TRUE);
    assert(x_wstr_endswith(s, len, L"abc", 3) == TRUE);
    assert(x_wstr_endswith(s, len, L"", 0) == TRUE);

    assert(x_wstr_endswith(s, len, L"abcd", 4) == FALSE);
    assert(x_wstr_endswith(s, len, L"cx", 2) == FALSE);
    assert(x_wstr_endswith(s, len, L"ab", 2) == FALSE);
    assert(x_wstr_endswith(s, len, L"aba", 3) == FALSE);
    assert(x_wstr_endswith(s, len, L"abd", 3) == FALSE);
    assert(x_wstr_endswith(s, len, L"xbc", 3) == FALSE);
    assert(x_wstr_endswith(s, len, L"bb", 2) == FALSE);
    assert(x_wstr_endswith(s, len, L"b", 1) == FALSE);
    assert(x_wstr_endswith(s, len, L"a", 1) == FALSE);
    assert(x_wstr_endswith(s, len, L"x", 1) == FALSE);
    assert(x_wstr_endswith(s, len, L"xxx", 3) == FALSE);
}

void test_x_wstr_findch()
{
    const wchar_t* s = L"abc";

    assert(x_wstr_findch(s, 'a', 0) == 0);
    assert(x_wstr_findch(s, 'b', 0) == 1);
    assert(x_wstr_findch(s, 'b', 1) == 1);
    assert(x_wstr_findch(s, 'c', 0) == 2);
    assert(x_wstr_findch(s, 'c', 1) == 2);
    assert(x_wstr_findch(s, 'c', 2) == 2);

    assert(x_wstr_findch(s, 'a', 1) == -1);
    assert(x_wstr_findch(s, 'b', 2) == -1);
    assert(x_wstr_findch(s, 'c', 3) == -1); // s is zero-terminated!

    assert(x_wstr_findch(s, 0, 0) == -1);
    assert(x_wstr_findch(s, 'x', 0) == -1);
}

void test_x_wstr_rfindch()
{
    const wchar_t* s = L"abc";
    int len = 3;

    assert(x_wstr_rfindch(s, 'a', len) == 0);
    assert(x_wstr_rfindch(s, 'a', len - 1) == 0);
    assert(x_wstr_rfindch(s, 'a', len - 2) == 0);
    assert(x_wstr_rfindch(s, 'b', len) == 1);
    assert(x_wstr_rfindch(s, 'b', len - 1) == 1);
    assert(x_wstr_rfindch(s, 'c', len) == 2);

    assert(x_wstr_rfindch(s, 'a', 0) == -1);
    assert(x_wstr_rfindch(s, 'b', 1) == -1);
    assert(x_wstr_rfindch(s, 'c', 2) == -1);

    assert(x_wstr_rfindch(s, 0, len) == -1);
    assert(x_wstr_rfindch(s, 'x', len) == -1);
}

void test_x_wstr_cmp()
{
    assert(x_wstr_cmp(L"", L"") == 0);
    assert(x_wstr_cmp(L"a", L"") == 1);
    assert(x_wstr_cmp(L"", L"a") == -1);
    assert(x_wstr_cmp(L"a", L"a") == 0);
    assert(x_wstr_cmp(L"abc", L"a") == 1);
    assert(x_wstr_cmp(L"a", L"abc") == -1);
    assert(x_wstr_cmp(L"abc", L"abc") == 0);
    assert(x_wstr_cmp(L"abc", L"Abc") == 1); // 'a' = 97, 'A' = 65, so 'a' > 'A'
    assert(x_wstr_cmp(L"abc", L"abC") == 1);
    assert(x_wstr_cmp(L"Abc", L"abc") == -1);
    assert(x_wstr_cmp(L"abC", L"abc") == -1);
}

void test_x_wstr_cpy()
{
    wchar_t dst[16] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    x_wstr_cpy(dst, L"");
    assert(dst[0] == 0);
    assert(dst[1] == 0xFF);

    x_wstr_cpy(dst, L"a");
    assert(dst[0] == L'a');
    assert(dst[1] == 0);
    assert(dst[2] == 0xFF);

    x_wstr_cpy(dst, L"ABC");
    assert(dst[0] == L'A');
    assert(dst[1] == L'B');
    assert(dst[2] == L'C');
    assert(dst[3] == 0);
    assert(dst[4] == 0xFF);

    x_wstr_cpy(dst, L"");
    assert(dst[0] == 0);
    assert(dst[1] == L'B');

    x_wstr_cpy(dst, L"xyz1234");
    assert(dst[0] == L'x');
    assert(dst[1] == L'y');
    assert(dst[2] == L'z');
    assert(dst[3] == L'1');
    assert(dst[4] == L'2');
    assert(dst[5] == L'3');
    assert(dst[6] == L'4');
    assert(dst[7] == 0);
    assert(dst[8] == 0xFF);

    x_wstr_cpy(dst, L"\0\0\0");
    assert(dst[0] == 0);
    assert(dst[1] == L'y');
    assert(dst[2] == L'z');
    assert(dst[3] == L'1');
}

void test_tDynamicBuffer_Allocate()
{
    tDynamicBuffer buf;
    const void* ptr;

    tDynamicBuffer_Init(&buf);
    assert(buf.ptr == NULL);
    assert(buf.nBytesAllocated == 0);
    assert(buf.nBytesStored == 0);

    tDynamicBuffer_Allocate(&buf, 0);
    assert(buf.ptr == NULL); // no new allocation
    assert(buf.nBytesAllocated == 0);
    assert(buf.nBytesStored == 0);

    tDynamicBuffer_Allocate(&buf, 16);
    assert(buf.ptr != NULL); // new allocation
    assert(buf.nBytesAllocated >= 16);
    assert(buf.nBytesStored == 0);

    ptr = buf.ptr;
    tDynamicBuffer_Allocate(&buf, 10);
    assert(buf.ptr == ptr); // no new allocation
    assert(buf.nBytesAllocated >= 16);
    assert(buf.nBytesStored == 0);

    ptr = buf.ptr;
    tDynamicBuffer_Allocate(&buf, 16);
    assert(buf.ptr == ptr); // no new allocation
    assert(buf.nBytesAllocated >= 16);
    assert(buf.nBytesStored == 0);

    ptr = buf.ptr;
    tDynamicBuffer_Allocate(&buf, 20);
    assert(buf.ptr != NULL);
    assert(buf.ptr != ptr); // new allocation
    assert(buf.nBytesAllocated >= 20);
    assert(buf.nBytesStored == 0);

    tDynamicBuffer_Free(&buf);
    assert(buf.ptr == NULL);
    assert(buf.nBytesAllocated == 0);
    assert(buf.nBytesStored == 0);
}

void test_tDynamicBuffer_Append_Clear()
{
    const char* str1 = "12345";
    const char* str2 = "67";
    const void* ptr;
    unsigned int strLen1 = 5;
    unsigned int strLen2 = 2;
    tDynamicBuffer buf;

    tDynamicBuffer_Init(&buf);
    assert(buf.ptr == NULL);
    assert(buf.nBytesAllocated == 0);
    assert(buf.nBytesStored == 0);

    tDynamicBuffer_Allocate(&buf, strLen1);
    assert(buf.ptr != NULL); // new allocation
    assert(buf.nBytesAllocated == strLen1);
    assert(buf.nBytesStored == 0);

    ptr = buf.ptr;
    tDynamicBuffer_Append(&buf, str1, strLen1);
    assert(buf.ptr == ptr); // no new allocation
    assert(buf.nBytesAllocated == strLen1);
    assert(buf.nBytesStored == strLen1);
    assert(memcmp(buf.ptr, str1, strLen1) == 0);

    ptr = buf.ptr;
    tDynamicBuffer_Append(&buf, str2, strLen2);
    assert(buf.ptr != NULL);
    assert(buf.ptr != ptr); // new allocation
    assert(buf.nBytesAllocated >= strLen1 + strLen2);
    assert(buf.nBytesStored == strLen1 + strLen2);
    assert(memcmp(buf.ptr, str1, strLen1) == 0);
    ptr = ((const char*)buf.ptr) + strLen1;
    assert(memcmp(ptr, str2, strLen2) == 0);

    tDynamicBuffer_Clear(&buf);
    assert(buf.ptr != NULL);
    assert(buf.nBytesAllocated >= strLen1 + strLen2);
    assert(buf.nBytesStored == 0);

    ptr = buf.ptr;
    tDynamicBuffer_Append(&buf, str2, strLen2);
    assert(buf.ptr == ptr); // no new allocation
    assert(buf.nBytesAllocated >= strLen1 + strLen2);
    assert(buf.nBytesStored == strLen2);
    assert(memcmp(buf.ptr, str2, strLen2) == 0);

    tDynamicBuffer_Free(&buf);
    assert(buf.ptr == NULL);
    assert(buf.nBytesAllocated == 0);
    assert(buf.nBytesStored == 0);

    tDynamicBuffer_Append(&buf, "", 0);
    assert(buf.ptr == NULL); // no new allocation
    assert(buf.nBytesAllocated == 0);
    assert(buf.nBytesStored == 0);

    tDynamicBuffer_Append(&buf, str1, strLen1);
    assert(buf.ptr != NULL); // new allocation
    assert(buf.nBytesAllocated >= strLen1);
    assert(buf.nBytesStored == strLen1);
    assert(memcmp(buf.ptr, str1, strLen1) == 0);

    tDynamicBuffer_Free(&buf);
    assert(buf.ptr == NULL);
    assert(buf.nBytesAllocated == 0);
    assert(buf.nBytesStored == 0);
}

void test_tDynamicBuffer_Swap()
{
    const char* str1 = "123";
    const char* str2 = "ABCDEF";
    int strLen1 = 3;
    int strLen2 = 6;
    tDynamicBuffer buf1;
    tDynamicBuffer buf2;
    void* ptr1;
    void* ptr2;
    UINT_PTR allocated1;
    UINT_PTR allocated2;
    UINT_PTR stored1;
    UINT_PTR stored2;

    tDynamicBuffer_Init(&buf1);
    tDynamicBuffer_Init(&buf2);

    tDynamicBuffer_Allocate(&buf1, 16);
    tDynamicBuffer_Allocate(&buf2, 48);

    tDynamicBuffer_Append(&buf1, str1, strLen1);
    tDynamicBuffer_Append(&buf2, str2, strLen2);

    assert(buf1.ptr != NULL);
    assert(buf2.ptr != NULL);
    assert(buf1.ptr != buf2.ptr);
    assert(buf1.nBytesStored == strLen1);
    assert(buf2.nBytesStored == strLen2);
    assert(buf1.nBytesAllocated != buf2.nBytesAllocated);

    ptr1 = buf1.ptr;
    allocated1 = buf1.nBytesAllocated;
    stored1 = buf1.nBytesStored;

    ptr2 = buf2.ptr;
    allocated2 = buf2.nBytesAllocated;
    stored2 = buf2.nBytesStored;

    tDynamicBuffer_Swap(&buf1, &buf2);

    assert(buf1.ptr == ptr2);
    assert(buf1.nBytesAllocated == allocated2);
    assert(buf1.nBytesStored == stored2);

    assert(buf2.ptr == ptr1);
    assert(buf2.nBytesAllocated == allocated1);
    assert(buf2.nBytesStored == stored1);

    tDynamicBuffer_Free(&buf1);
    tDynamicBuffer_Free(&buf2);
}

void tests_xmem_str_func()
{
    test_x_mem_funcs();
    test_x_wstr_startswith();
    test_x_wstr_endswith();
    test_x_wstr_findch();
    test_x_wstr_rfindch();
    test_x_wstr_cmp();
    test_x_wstr_cpy();
    test_tDynamicBuffer_Allocate();
    test_tDynamicBuffer_Append_Clear();
    test_tDynamicBuffer_Swap();

    printf("%s() passed!\n", __FUNCTION__);
}
