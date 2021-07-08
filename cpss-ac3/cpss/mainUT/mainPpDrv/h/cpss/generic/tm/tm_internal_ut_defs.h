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
 * @brief tm internal UT interface.
 *
* @file tm_internal_ut_defs.h
*
* $Revision: 1 $
 */
 
#ifndef TM_UT_DEFS_H
#define TM_UT_DEFS_H

#include <stdlib.h>
#include "core/tm_os_interface.h"

/*	Not all compilers support  __FUNCTION__ and no generic way to recognize this 
	so currently we will not use this feature
*/
#define __func__ "******"


extern int tm_ut_verify_last_test_status;
extern int tm_ut_continue_on_fail_flag;
extern int tm_ut_debug_flag;

/**
* @internal tm_ut_verify_equal function
* @endinternal
*
* @brief   This routine is used for handling test failures (e != r)
*
* @param[in] test_name                -   test name.
* @param[in] expected                 -    value.
* @param[in] result                   -   received value.
* @param[in] lineNum                  -   number of line in test file where test failure
*                                      has been found.
* @param[in] fileNamePtr              -   (pointer to) name of test file where error has been
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
    int         lineNum,
    const char  *fileNamePtr
);

/**
* @internal tm_ut_verify_not_equal function
* @endinternal
*
* @brief   This routine is used for handling test failures (e == r)
*
* @param[in] test_name                -   test name.
* @param[in] expected                 -    value.
* @param[in] result                   -   received value.
* @param[in] lineNum                  -   number of line in test file where test failure
*                                      has been found.
* @param[in] fileNamePtr              -   (pointer to) name of test file where error has been
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
    int         lineNum,
    const char  *fileNamePtr
);

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
);

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
extern int tm_ut_set_debug_flag
(
	int Enable
);



extern void tm_ut_verify_test_start(const char *test_name);

#define tm_ut_verify_equal_mac(expected, result, msg)               \
    if (tm_ut_verify_equal(__func__, expected, result, msg, __LINE__, __FILE__) == 0)   \
	{																\
		tm_ut_verify_last_test_status = -1;							\
        if (!tm_ut_continue_on_fail_flag)                           \
            return -1;												\
	}

#define tm_ut_verify_not_equal_mac(expected, result, msg)           \
    if (tm_ut_verify_not_equal(__func__, expected, result, msg, __LINE__, __FILE__) == 0)   \
	{																\
		tm_ut_verify_last_test_status = -1;							\
        if (!tm_ut_continue_on_fail_flag)                           \
            return -1;												\
	}


#define tm_ut_verify_test_mac(expected, result, test_name)          \
    if (expected != result)                                         \
    {                                                               \
        tm_printf("%s: fail\n", test_name);							\
        if (!tm_ut_continue_on_fail_flag)                           \
            return -1;												\
    }                                                               \
    else                                                            \
    {                                                               \
        if (tm_ut_debug_flag)                                       \
            tm_printf("%s pass\n", test_name);	    				\
    }

#define tm_ut_verify_tm_lib_hndl_mac(tm_ctl_PTR)							        \
	if (tm_ctl_PTR == NULL)													        \
	{																				\
		tm_printf("\t%s, line: %d %s\n", "tm lib bad handle", __LINE__, __FILE__);	\
		return -1;																	\
	}	

   
#endif   /* TM_UT_DEFS_H */

