#ifndef __UNIT_TEST_H__
#define __UNIT_TEST_H__

typedef struct unit_test_t unit_test_t;
struct unit_test_t {
    /**
     * @brief add test case 
     */
    void (*add_test) (unit_test_t *this, char *unit_test_name, void (*test_fun) (void));

    /**
     * @brief run unit_test test
     */
    void (*run_test) (unit_test_t *this);

    /**
     * @brief destroy instance 
     */
    void (*destroy) (unit_test_t *this);
};

/**
 * @brief create unit_test instance 
 *
 * @param unit_test_name
 * @param test_fun
 */
unit_test_t *unit_test_create();

#endif /* __unit_test_H__ */
