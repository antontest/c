/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unit_test.h>
#include <CUnit/CUnit.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
int max(int a, int b)
{
    return a > b ? a : b;
}

void maxi(void)
{
    CU_ASSERT(max(1, 2) == 2);
    CU_ASSERT(max(1, 1) == 1);
    CU_ASSERT(max(4, 2) == 4);
    CU_ASSERT_EQUAL(max(-1, -2), -1);
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    unit_test_t *test = unit_test_create();
    test->add_test(test, "test_for_maxi", maxi);
    test->run_test(test);
    test->destroy(test);

    return rt;
}
