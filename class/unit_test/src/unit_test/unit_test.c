#include <unit_test.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <utils/utils.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>
#include <CUnit/Basic.h>
#include <CUnit/Console.h>
#include <assert.h>

#define MAX_TEST_CASE_SIZE 11
typedef struct private_unit_test_t private_unit_test_t;
struct private_unit_test_t {
    /**
     * @brief public interface
     */
    unit_test_t public;

    /**
     * @brief number of test case
     */
    int case_num;
};

static int suite_success_init() 
{
    return 0;
}

static int suite_success_clean() 
{
    return 0;
}

/**
 * @brief test case
 */
static CU_TestInfo testcase[MAX_TEST_CASE_SIZE] ;
static CU_SuiteInfo suites[] = {
    {"testSuite", suite_success_init, suite_success_clean, testcase},
    CU_SUITE_INFO_NULL,
};

static int init_test(private_unit_test_t *this)
{
    if (CU_initialize_registry()) {
        printf("Initialization of Test Registry failed. \n");
        return -1;
    }

    if (CU_get_registry() == NULL) return -1;
    if (CU_is_test_running()) return 0;

    return 0;
}

METHOD(unit_test_t, destroy_, void, private_unit_test_t *this)
{
    free(this);
    CU_cleanup_registry();
} 

METHOD(unit_test_t, run_test_, void, private_unit_test_t *this)
{
    testcase[this->case_num].pName     = NULL;
    testcase[this->case_num].pTestFunc = NULL;
    if (CU_register_suites(suites) != CUE_SUCCESS) {
        printf("Registry suites failed. \n");
        return;
    }
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_get_error();
}

METHOD(unit_test_t, add_test_, void, private_unit_test_t *this, char *unit_test_name, void (*test_fun) (void))
{
    if (this->case_num + 1 >= MAX_TEST_CASE_SIZE) return;
    testcase[this->case_num].pName = unit_test_name;
    testcase[this->case_num].pTestFunc = test_fun;
    this->case_num++;
}

unit_test_t *unit_test_create(const char *unit_test_name, void (*test_fun) (void))
{
    private_unit_test_t *this;

    INIT(this, 
        .public = {
            .add_test = _add_test_,
            .run_test = _run_test_,
            .destroy  = _destroy_,
        },
        .case_num  = 0,
    );

    if (init_test(this) < 0) {
        _destroy_(this);
        return NULL;
    }

    return &this->public;
}
