#include <CUnit/Basic.h>
#include "test_lin_alg.h"
#include "test_basic_qp.h"

int main(){

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* linear algebra tests */
    CU_TestInfo suite_lin_alg[] = {
        { "test_vec_set_scalar", test_vec_set_scalar},
        { "test_vec_set_scalar_int", test_vec_set_scalar_int},
        { "test_vec_mult_scalar", test_vec_mult_scalar},
        { "test_vec_prod", test_vec_prod},
        { "test_vec_add_scaled", test_vec_add_scaled},
        { "test_vec_norm_inf", test_vec_norm_inf},
        { "test_vec_ew_recipr", test_vec_ew_recipr},
        { "test_vec_ew_max_vec", test_vec_ew_max_vec},
        { "test_vec_ew_min_vec", test_vec_ew_min_vec},
        { "test_vec_ew_mid_vec", test_vec_ew_mid_vec},
        { "test_vec_ew_prod", test_vec_ew_prod},
        { "test_vec_ew_div", test_vec_ew_div},
        { "test_vec_ew_sqrt", test_vec_ew_sqrt},
        CU_TEST_INFO_NULL,
    };

    /* basic qp */
    CU_TestInfo suite_basic_qp[] = {
        { "test_basic_qp", test_basic_qp},
        CU_TEST_INFO_NULL,
    };

    /* list of suites to be tested */
    CU_SuiteInfo suites[] = {
        { "lin_alg", NULL, NULL, reset_abc, NULL, suite_lin_alg},
        { "basic_qp", NULL, NULL, basic_qp_setup, basic_qp_teardown, suite_basic_qp},
        CU_SUITE_INFO_NULL,
    };

    if (CUE_SUCCESS != CU_register_suites(suites)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
        

}