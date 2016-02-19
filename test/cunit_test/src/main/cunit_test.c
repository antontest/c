/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <CUnit/Console.h>
#include <CUnit/TestDB.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
int max(int a, int b)
{
    return a > b ? a : b;
}

void test_maxi(void)
{
    CU_ASSERT(max(1, 2) == 2);
    CU_ASSERT(max(1, -2) == 1);
    CU_ASSERT(max(-2, -2) == -2);
}

CU_TestInfo testcase[] = {
    {"test_for_maxi", test_maxi},
    CU_TEST_INFO_NULL
};

int suite_success_init()
{
    return 0;
}

int suite_success_clean()
{
    return 0;
}

CU_SuiteInfo suites[] = {
    {"testSuite1", suite_success_init, suite_success_clean, testcase},
    CU_SUITE_INFO_NULL,
};

void AddTests() {
    assert(NULL != CU_get_registry());
    assert(!CU_is_test_running());
    if (CUE_SUCCESS != CU_register_suites(suites)) {
        exit(1);
    }
}

int RunTest() {
    if (CU_initialize_registry()) {
        printf(" Initialization of Test Registry failed. \n");
        exit(1);
    } else {
        AddTests();
        CU_basic_set_mode(CU_BRM_VERBOSE);
        CU_basic_run_tests();
        CU_cleanup_registry();
        return CU_get_error();
    }
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    printf("hello\n");

    RunTest();
    return rt;
}
