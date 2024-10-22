#include "tests/tests_find_in_sorted_array.h"
#include "tests/tests_XMemStrFunc.h"

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static int run_tests()
{
    srand((unsigned int) time(NULL));

    tests_xmem_str_func();
    tests_find_in_sorted_array();

    printf("Everything is OK!!!\n");
    return 0;
}

int main()
{
    return run_tests();
}
