/*
 * (c), Copyright 2009-2013, Marvell International Ltd.  (Marvell)
 *
 * This code contains confidential information of Marvell.
 * No rights are granted herein under any patent, mask work right or copyright
 * of Marvell or any third party. Marvell reserves the right at its sole
 * discretion to request that this code be immediately returned to Marvell.
 * This code is provided "as is". Marvell makes no warranties, expressed,
 * implied or otherwise, regarding its accuracy, completeness or performance.
 */
/**
 * @brief Resource Manager UT functions implementation.
 *
* @file rm_ut.c
*
* $Revision: 1 $
 */

#include <cpss/generic/tm/tm_internal_ut_defs.h>


int tm_ut_verify_last_test_status;

int tm_ut_continue_on_fail_flag = 0;

int tm_ut_debug_flag = 0;

/**
* @internal tm_ut_verify_equal function
* @endinternal
*
* @brief   This routine is used for handling test failures (e != r)
*
* @param[in] test_name                -   test name.
* @param[in] expected                 -    value.
* @param[in] result                   -   received value.
*                                      lineNum     -   number of line in test file where test failure
*                                      has been found.
*                                      fileNamePtr -   (pointer to) name of test file where error has been
*                                      found.
*
* @retval !0                       -  Pass condition was correct. Test may continue its flow.
* @retval 0                        -  Pass condition is invalid. Test is broken.
*
* @note Test cases must not call this function directly.
*
*/
extern int tm_ut_verify_equal
(
    const char* test_name, 
    int         expected, 
    int         result, 
    char*       msg,
    int         line_num,
    const char* file_name
)
{
    if (expected != result) 
    {
        tm_printf("\t%s, line: %d %s: expected result: %d, result: %d, %s\n", 
                  file_name,
                  line_num,
                  test_name, 
                  expected, 
                  result, 
                  (msg != NULL) ? msg : "");

		return 0;
    }

    return 1;
}

/**
* @internal tm_ut_verify_not_equal function
* @endinternal
*
* @brief   This routine is used for handling test failures (e == r)
*
* @param[in] test_name                -   test name.
* @param[in] expected                 -    value.
* @param[in] result                   -   received value.
*                                      lineNum     -   number of line in test file where test failure
*                                      has been found.
*                                      fileNamePtr -   (pointer to) name of test file where error has been
*                                      found.
*
* @retval !0                       -  Pass condition was correct. Test may continue its flow.
* @retval 0                        -  Pass condition is invalid. Test is broken.
*
* @note Test cases must not call this function directly.
*
*/
extern int tm_ut_verify_not_equal
(
    const char* test_name, 
    int         expected, 
    int         result, 
    char*       msg,
    int         line_num,
    const char* file_name
)
{
    if (expected == result) 
    {
        tm_printf("\t%s, line: %d %s: not expected result: %d, result: %d, %s\n", 
                  file_name,
                  line_num,
                  test_name, 
                  expected, 
                  result, 
                  (msg != NULL) ? msg : "");

		return 0;
    }

    return 1;
}


void tm_ut_verify_test_start(const char *test_name)
{
	tm_ut_verify_last_test_status = 0;
    if (tm_ut_debug_flag)
        tm_printf("%s:\n", test_name);                          
}

/**
* @internal tm_ut_continue_on_fail function
* @endinternal
*
* @brief   This routine is used to continue on fail
*
* @param[in] Enable                   - 1 - continue on fail, Otherwise stop on fail
*
* @note Test cases must not call this function directly.
*
*/
extern void tm_ut_continue_on_fail
(
    int Enable
)
{
    if (Enable == 1)
    {
        tm_ut_continue_on_fail_flag = 1;
    }
    else
    {
        tm_ut_continue_on_fail_flag = 0;
    }
}

/**
* @internal tm_ut_set_debug_flag function
* @endinternal
*
* @brief   This routine is used to debug internal tm ut
*
* @param[in] Enable                   - 1 - debug prints on, Otherwise debug prints off
*
* @note Test cases must not call this function directly.
*
*/
int tm_ut_set_debug_flag
(
	int Enable
)
{
	if (Enable == 1)
	{
		tm_ut_debug_flag = 1;
	}
	else
	{
		tm_ut_debug_flag = 0;
	}

    return 0;
}








