#include "tests_find_in_sorted_array.h"
#include "../code/find_in_sorted_array.h"

#include <assert.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void test_find_vals_in_sorted_array(const matchpos_t arr[], int arrSize, const matchpos_t val[], int valSize)
{
    int i;
    int result;
    BOOL bExactMatch;

    for (i = 0; i < valSize; ++i)
    {
        result = find_in_sorted_matchpos_array(arr, arrSize, val[i], &bExactMatch);
        if (i == 0)
        {
            assert(result == -1);
            assert(bExactMatch == FALSE);
        }
        else
        {
            assert(result == (i - 1) / 2);
            assert(bExactMatch == (i % 2) == 1 ? TRUE : FALSE);
        }
    }
}

static void test_find_in_sorted_array_0()
{
    const matchpos_t arr[] = { 10 };
    int arrSize = 0;
    int result;
    BOOL bExactMatch;

    result = find_in_sorted_matchpos_array(arr, arrSize, 5, &bExactMatch);
    assert(result == -1);
    assert(bExactMatch == FALSE);

    result = find_in_sorted_matchpos_array(arr, arrSize, 10, &bExactMatch);
    assert(result == -1);
    assert(bExactMatch == FALSE);

    result = find_in_sorted_matchpos_array(arr, arrSize, 15, &bExactMatch);
    assert(result == -1);
    assert(bExactMatch == FALSE);
}

static void test_find_in_sorted_array_1()
{
    const matchpos_t arr[] = { 10 };
    const matchpos_t val[] = { 5, 10, 15 };
    int arrSize = sizeof(arr) / sizeof(arr[0]);
    int valSize = sizeof(val) / sizeof(val[0]);

    test_find_vals_in_sorted_array(arr, arrSize, val, valSize);
}

static void test_find_in_sorted_array_2()
{
    const matchpos_t arr[] = { 10, 20 };
    const matchpos_t val[] = { 5, 10, 15, 20, 25 };
    int arrSize = sizeof(arr) / sizeof(arr[0]);
    int valSize = sizeof(val) / sizeof(val[0]);

    test_find_vals_in_sorted_array(arr, arrSize, val, valSize);
}

static void test_find_in_sorted_array_3()
{
    const matchpos_t arr[] = { 10, 20, 30 };
    const matchpos_t val[] = { 5, 10, 15, 20, 25, 30, 35 };
    int arrSize = sizeof(arr) / sizeof(arr[0]);
    int valSize = sizeof(val) / sizeof(val[0]);

    test_find_vals_in_sorted_array(arr, arrSize, val, valSize);
}

static void test_find_in_sorted_array_4()
{
    const matchpos_t arr[] = { 10, 20, 30, 40 };
    const matchpos_t val[] = { 5, 10, 15, 20, 25, 30, 35, 40, 45 };
    int arrSize = sizeof(arr) / sizeof(arr[0]);
    int valSize = sizeof(val) / sizeof(val[0]);

    test_find_vals_in_sorted_array(arr, arrSize, val, valSize);
}

static void test_find_in_sorted_array_5()
{
    const matchpos_t arr[] = { 10, 20, 30, 40, 50 };
    const matchpos_t val[] = { 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55 };
    int arrSize = sizeof(arr) / sizeof(arr[0]);
    int valSize = sizeof(val) / sizeof(val[0]);
    
    test_find_vals_in_sorted_array(arr, arrSize, val, valSize);
}

static void test_find_in_sorted_array_20()
{
    const matchpos_t arr[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200 };
    const matchpos_t val[] = { 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100, 
                          105, 110, 115, 120, 125, 130, 135, 140, 145, 150, 155, 160, 165, 170, 175, 180, 185, 190, 195, 200, 205 };
    int arrSize = sizeof(arr) / sizeof(arr[0]);
    int valSize = sizeof(val) / sizeof(val[0]);

    test_find_vals_in_sorted_array(arr, arrSize, val, valSize);
}

static void test_find_in_sorted_array_random()
{
    matchpos_t* val;
    matchpos_t* arr;
    int valSize;
    int arrSize;
    matchpos_t x;
    int i;

    arrSize = ((rand()*(unsigned int)time(NULL)) % 77777) + 1;
    arr = malloc(arrSize * sizeof(matchpos_t));
    assert(arr != NULL);

    valSize = 1 + 2*arrSize;
    val = malloc(valSize * sizeof(matchpos_t));
    assert(val != NULL);

    x = 0;
    for (i = 0; i < valSize; ++i)
    {
        val[i] = x;

        if ((i % 2) == 1)
        {
            arr[i / 2] = x;
        }

        x += (rand() % 13) + 1;
    }

    test_find_vals_in_sorted_array(arr, arrSize, val, valSize);

    free(val);
    free(arr);
}

static void test_matchpos_t()
{
    matchpos_t nMatchPos;

    nMatchPos = to_matchpos(0, 0);
    assert(nMatchPos == 0);
    assert(get_matchpos_line(nMatchPos) == 0);
    assert(get_matchpos_pos_in_line(nMatchPos) == 0);

    nMatchPos = to_matchpos(0, 12345);
    assert(nMatchPos == 12345);
    assert(get_matchpos_line(nMatchPos) == 0);
    assert(get_matchpos_pos_in_line(nMatchPos) == 12345);

    nMatchPos = to_matchpos(0, 0xFFFFFFFF);
    assert(nMatchPos == 0xFFFFFFFF);
    assert(get_matchpos_line(nMatchPos) == 0);
    assert(get_matchpos_pos_in_line(nMatchPos) == 0xFFFFFFFF);

    nMatchPos = to_matchpos(0x12345, 0);
    assert(nMatchPos == 0x1234500000000);
    assert(get_matchpos_line(nMatchPos) == 0x12345);
    assert(get_matchpos_pos_in_line(nMatchPos) == 0);

    nMatchPos = to_matchpos(0xFFFFFFFF, 0);
    assert(nMatchPos == 0xFFFFFFFF00000000);
    assert(get_matchpos_line(nMatchPos) == 0xFFFFFFFF);
    assert(get_matchpos_pos_in_line(nMatchPos) == 0);

    nMatchPos = to_matchpos(0x12345, 0x6789);
    assert(nMatchPos == 0x1234500006789);
    assert(get_matchpos_line(nMatchPos) == 0x12345);
    assert(get_matchpos_pos_in_line(nMatchPos) == 0x6789);

    nMatchPos = to_matchpos(0xFFFFFFFF, 0xFFFFFFFF);
    assert(nMatchPos == 0xFFFFFFFFFFFFFFFF);
    assert(get_matchpos_line(nMatchPos) == 0xFFFFFFFF);
    assert(get_matchpos_pos_in_line(nMatchPos) == 0xFFFFFFFF);
}

void tests_find_in_sorted_array()
{
    test_matchpos_t();
    test_find_in_sorted_array_0();
    test_find_in_sorted_array_1();
    test_find_in_sorted_array_2();
    test_find_in_sorted_array_3();
    test_find_in_sorted_array_4();
    test_find_in_sorted_array_5();
    test_find_in_sorted_array_20();
    test_find_in_sorted_array_random();

    printf("%s() passed!\n", __FUNCTION__);
}
