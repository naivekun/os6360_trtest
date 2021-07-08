/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file utfMain.h
*
* @brief Public header which must be included to all test suits .c files
* Contains declaration of all UTF functionality, like listing, running,
* and displaying statistics.
* Defines macros to declare, start and finalize suit and test case.
* Defines macros to check test failures.
*
* @version   50
********************************************************************************
*/
#ifndef __utfMainh
#define __utfMainh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef _MSC_VER
    /* Disable warning messages for VC W4 warnings level */
    #pragma warning(disable: 4152)
#endif /* _MSC_VER */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/events/cpssGenEventUnifyTypes.h>

/* defines */

/* prototype for callback function that need to be called on error .
   this to allow a test to 'dump' into LOG/terminal important info that may
   explain why the test failed. -- this is advanced debug tool

    the 'registration' of this call back is done via function utfCallBackFunctionOnErrorSave
*/
typedef void (*UTF_CALL_BACK_FUNCTION_ON_ERROR_TYPE) (void);


/* Max Utf parameter name lenght */
#define UTF_MAX_PARAM_NAME_LEN_CNS  50

/* Max Utf value lenght (in Ansi format) */
#define UTF_MAX_VALUE_LEN_CNS       15

/* Max UTF result array size (maximum number of parameters passed to file)*/
#define UTF_RESULT_ARRAY_MAX_CNS    15

/* Max UTF result file size*/
#define UTF_MAX_LOG_SIZE_CNS      8196

/* Unit test fail  This error is returned by UTF to indicate that some test fails.*/
#define GT_UTF_TEST_FAILED          (CPSS_PRESTERA_ERROR_BASE_CNS + (0x10000))

#define UTF_ARGVP_ARR_1_MAC(argvp,arg1)   \
            GT_UINTPTR argvp[1];                                                \
            argvp[0] = (GT_UINTPTR)(arg1);

#define UTF_ARGVP_ARR_2_MAC(argvp,arg1,arg2)   \
            GT_UINTPTR argvp[2];                                                \
            argvp[0] = (GT_UINTPTR)(arg1);                                      \
            argvp[1] = (GT_UINTPTR)(arg2);

#define UTF_ARGVP_ARR_3_MAC(argvp,arg1,arg2,arg3)   \
            GT_UINTPTR argvp[3];                                                \
            argvp[0] = (GT_UINTPTR)(arg1);                                      \
            argvp[1] = (GT_UINTPTR)(arg2);                                      \
            argvp[2] = (GT_UINTPTR)(arg3);

#define UTF_ARGVP_ARR_4_MAC(argvp,arg1,arg2,arg3,arg4)   \
            GT_UINTPTR argvp[4];                                                \
            argvp[0] = (GT_UINTPTR)(arg1);                                      \
            argvp[1] = (GT_UINTPTR)(arg2);                                      \
            argvp[2] = (GT_UINTPTR)(arg3);                                      \
            argvp[3] = (GT_UINTPTR)(arg4);

#define UTF_ARGVP_ARR_5_MAC(argvp,arg1,arg2,arg3,arg4,arg5)   \
            GT_UINTPTR argvp[5];                                                \
            argvp[0] = (GT_UINTPTR)(arg1);                                      \
            argvp[1] = (GT_UINTPTR)(arg2);                                      \
            argvp[2] = (GT_UINTPTR)(arg3);                                      \
            argvp[3] = (GT_UINTPTR)(arg4);                                      \
            argvp[4] = (GT_UINTPTR)(arg5);

#define UTF_ARGVP_ARR_6_MAC(argvp,arg1,arg2,arg3,arg4,arg5,arg6)   \
            GT_UINTPTR argvp[6];                                                \
            argvp[0] = (GT_UINTPTR)(arg1);                                      \
            argvp[1] = (GT_UINTPTR)(arg2);                                      \
            argvp[2] = (GT_UINTPTR)(arg3);                                      \
            argvp[3] = (GT_UINTPTR)(arg4);                                      \
            argvp[4] = (GT_UINTPTR)(arg5);                                      \
            argvp[5] = (GT_UINTPTR)(arg6);

#define UTF_ARGVP_ARR_7_MAC(argvp,arg1,arg2,arg3,arg4,arg5,arg6,arg7)   \
            GT_UINTPTR argvp[7];                                                \
            argvp[0] = (GT_UINTPTR)(arg1);                                      \
            argvp[1] = (GT_UINTPTR)(arg2);                                      \
            argvp[2] = (GT_UINTPTR)(arg3);                                      \
            argvp[3] = (GT_UINTPTR)(arg4);                                      \
            argvp[4] = (GT_UINTPTR)(arg5);                                      \
            argvp[5] = (GT_UINTPTR)(arg6);                                      \
            argvp[6] = (GT_UINTPTR)(arg7);

#define UTF_ARGVP_ARR_8_MAC(argvp,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8)   \
            GT_UINTPTR argvp[8];                                                \
            argvp[0] = (GT_UINTPTR)(arg1);                                      \
            argvp[1] = (GT_UINTPTR)(arg2);                                      \
            argvp[2] = (GT_UINTPTR)(arg3);                                      \
            argvp[3] = (GT_UINTPTR)(arg4);                                      \
            argvp[4] = (GT_UINTPTR)(arg5);                                      \
            argvp[5] = (GT_UINTPTR)(arg6);                                      \
            argvp[6] = (GT_UINTPTR)(arg7);                                      \
            argvp[7] = (GT_UINTPTR)(arg8);

#define UTF_ARGVP_ARR_9_MAC(argvp,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9)\
            GT_UINTPTR argvp[9];                                                \
            argvp[0] = (GT_UINTPTR)(arg1);                                      \
            argvp[1] = (GT_UINTPTR)(arg2);                                      \
            argvp[2] = (GT_UINTPTR)(arg3);                                      \
            argvp[3] = (GT_UINTPTR)(arg4);                                      \
            argvp[4] = (GT_UINTPTR)(arg5);                                      \
            argvp[5] = (GT_UINTPTR)(arg6);                                      \
            argvp[6] = (GT_UINTPTR)(arg7);                                      \
            argvp[7] = (GT_UINTPTR)(arg8);                                      \
            argvp[8] = (GT_UINTPTR)(arg9);

#define UTF_ARGVP_ARR_10_MAC(argvp,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10)\
            GT_UINTPTR argvp[10];                                                \
            argvp[0] = (GT_UINTPTR)(arg1);                                      \
            argvp[1] = (GT_UINTPTR)(arg2);                                      \
            argvp[2] = (GT_UINTPTR)(arg3);                                      \
            argvp[3] = (GT_UINTPTR)(arg4);                                      \
            argvp[4] = (GT_UINTPTR)(arg5);                                      \
            argvp[5] = (GT_UINTPTR)(arg6);                                      \
            argvp[6] = (GT_UINTPTR)(arg7);                                      \
            argvp[7] = (GT_UINTPTR)(arg8);                                      \
            argvp[8] = (GT_UINTPTR)(arg9);                                      \
            argvp[9] = (GT_UINTPTR)(arg10);

#define UTF_ARGVP_ARR_11_MAC(argvp,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,arg11)\
            GT_UINTPTR argvp[11];                                               \
            argvp[0] = (GT_UINTPTR)(arg1);                                      \
            argvp[1] = (GT_UINTPTR)(arg2);                                      \
            argvp[2] = (GT_UINTPTR)(arg3);                                      \
            argvp[3] = (GT_UINTPTR)(arg4);                                      \
            argvp[4] = (GT_UINTPTR)(arg5);                                      \
            argvp[5] = (GT_UINTPTR)(arg6);                                      \
            argvp[6] = (GT_UINTPTR)(arg7);                                      \
            argvp[7] = (GT_UINTPTR)(arg8);                                      \
            argvp[8] = (GT_UINTPTR)(arg9);                                      \
            argvp[9] = (GT_UINTPTR)(arg10);                                     \
            argvp[10] = (GT_UINTPTR)(arg11);

#define UTF_ARGVP_ARR_12_MAC(argvp,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,arg11,arg12)\
            GT_UINTPTR argvp[12];                                               \
            argvp[0] = (GT_UINTPTR)(arg1);                                      \
            argvp[1] = (GT_UINTPTR)(arg2);                                      \
            argvp[2] = (GT_UINTPTR)(arg3);                                      \
            argvp[3] = (GT_UINTPTR)(arg4);                                      \
            argvp[4] = (GT_UINTPTR)(arg5);                                      \
            argvp[5] = (GT_UINTPTR)(arg6);                                      \
            argvp[6] = (GT_UINTPTR)(arg7);                                      \
            argvp[7] = (GT_UINTPTR)(arg8);                                      \
            argvp[8] = (GT_UINTPTR)(arg9);                                      \
            argvp[9] = (GT_UINTPTR)(arg10);                                     \
            argvp[10] = (GT_UINTPTR)(arg11);                                     \
            argvp[11] = (GT_UINTPTR)(arg12);


/* Check given function for wrong enum values (from array) */
extern GT_U32  enumsIndex;
extern GT_U32 tempParamValue;

#define UTF_ENUMS_CHECK_WITH_RET_CODE_MAC(func, param, rc)                     \
    {                                                                          \
        enumsIndex = 0;                                                        \
        tempParamValue = param;                                                \
        for(enumsIndex = 0; enumsIndex < utfInvalidEnumArrSize; enumsIndex++)  \
        {                                                                      \
              param = utfInvalidEnumArr[enumsIndex];                           \
              st = func;                                                       \
              if(st != GT_NOT_APPLICABLE_DEVICE)                               \
              { /*for some devices the family supported but not all revisions */\
                /*so we expect GT_NOT_APPLICABLE_DEVICE and not GT_BAD_PARAM*/ \
                  UTF_VERIFY_EQUAL1_STRING_MAC(rc, st,                         \
                                "Fail on wrong enum value: %d", param);        \
              }                                                                \
        }                                                                      \
        param = tempParamValue;                                                \
    }

#define UTF_ENUMS_CHECK_MAC(func, param)                                       \
    UTF_ENUMS_CHECK_WITH_RET_CODE_MAC(func, param, GT_BAD_PARAM)


/* defines the number of messages that can be given as 'General info' see function
   utfGeneralStateMessageSave(...) */
#define PRV_UTF_GENERAL_STATE_INFO_NUM_CNS          20

/* typedefs */
/**
* @enum UTF_LOG_OUTPUT_ENT
 *
 * @brief presents values of available kinds of log output
*/
typedef enum{

    UTF_LOG_OUTPUT_SERIAL_ALL_E = 0,

    /** @brief output to serial interface only
     *  summary report
     */
    UTF_LOG_OUTPUT_SERIAL_FINAL_E = 1,

    /** output to file */
    UTF_LOG_OUTPUT_FILE_E = 2,

    /** @brief same the UTF_LOG_OUTPUT_SERIAL_FINAL_E
     *  with debug output support
     */
    UTF_LOG_OUTPUT_SERIAL_FINAL_WITH_DEBUG_E = 3

} UTF_LOG_OUTPUT_ENT;

/**
* @enum UTF_TEST_TYPE_ENT
 *
 * @brief presents values of available types of tests
*/
typedef enum{

    /** no test type */
    UTF_TEST_TYPE_NONE_E = 0,

    /** general test type */
    UTF_TEST_TYPE_GEN_E,

    /** CH test type */
    UTF_TEST_TYPE_CHX_E,

    /** Salsa test type */
    UTF_TEST_TYPE_SAL_E,

    /** EX test type */
    UTF_TEST_TYPE_EX_E,

    /** PM test type */
    UTF_TEST_TYPE_PM_E,

    /** traffic test type */
    UTF_TEST_TYPE_TRAFFIC_E,

    /** traffic test type when FDB is not unified */
    UTF_TEST_TYPE_TRAFFIC_FDB_NON_UNIFIED_E,

    /** PX test type (mainUT --> not UTF) */
    UTF_TEST_TYPE_PX_E,

    /** PX test type + traffic. (UTF) */
    UTF_TEST_TYPE_PX_TRAFFIC_E,

    UTF_TEST_TYPE_LAST_E

} UTF_TEST_TYPE_ENT;

/*
 * Typedef: UTF_TEST_CASE_FUNC_PTR
 *
 * Description: pointer to test case funcion
 *
 * Comments: test case function should receive nothing (void) and shouldn't
 *      return anything (void)
 */
typedef GT_VOID (*UTF_TEST_CASE_FUNC_PTR)(GT_VOID);

/**
* @struct UTF_RESULT_PARAM_STC
 *
 * @brief contain UTF result parameter, that passed from tests to result file
*/
typedef struct{

    GT_CHAR paramName[UTF_MAX_PARAM_NAME_LEN_CNS];

    /** parameter value */
    GT_U32 paramValue;

    /** parameter Delta (in persents) */
    GT_U8 paramDelta;

} UTF_RESULT_PARAM_STC;

/* defines */
/*
 * Description: Defines begin of a singe suit.
 *
 * Parameters:
 *      x - suit name
 *
 * Comments: Name of suit must be unique between all the suits.
 */
#define UTF_SUIT_BEGIN_TESTS_MAC(x)         \
    GT_STATUS utfSuit_##x(GT_VOID) {        \
        GT_STATUS st = GT_OK;               \
        static const GT_VOID *s_##x[] = {   \
            #x,                             \

#define UTF_SUIT_PBR_BEGIN_TESTS_MAC(x)     \
           UTF_SUIT_BEGIN_TESTS_MAC(x)      \

/*
 * Description: Defines begin of a singe suit with preinit condition.
 *

 * Parameters:
 *      x - suit name
 *
 * Comments: Name of suit must be unique between all the suits.
 */
#define UTF_SUIT_WITH_PREINIT_CONDITION_BEGIN_TESTS_MAC(x)          \
    GT_STATUS utfSuit_##x(GT_VOID) {                                \
        GT_STATUS st = GT_OK;                                       \
        GT_U32 value = 0;                                           \
        static const GT_VOID *s_##x[] = {                           \
            #x,                                                     \

/*
 * Description: Finalize suit declaration.
 *
 * Parameters:
 *      x - suit name
 *
 * Comments: This macro must be placed after all declared test cases.
 */
#define UTF_SUIT_END_TESTS_MAC(x)           \
        0};                                 \
        st = utfSuitAdd(s_##x);             \
        return st;                          \
    }                                       \

/*
 * Description: Finalize suit declaration with preinit condition.
 *
 * Parameters:
 *      x - suit name
 *      y - preinit conditional string. The configuration value is retieved
 *          by this string, compared with expected result z and decision of
 *          including test into test suit or not is done
 *      z - expected value
 * Comments: This macro must be placed after all declared test cases.
 */
#define UTF_SUIT_WITH_PREINIT_CONDITION_END_TESTS_MAC(x,y,z)         \
        0};                                                          \
        if ((appDemoDbEntryGet(y, &value) == GT_OK)&&(value == z))   \
            st = utfSuitAdd(s_##x);                                  \
        return st;                                                   \
    }                                                                \

/*
 * Description: Finalize suit declaration for PBR mode.
 *
 * Parameters:
 *      x - suit name
 *      n - number of tests to including into the test suit in PBR mode.
 *          In other modes, all tests will be included.
 * Comments: This macro must be placed after all declared test cases.
 */
#define UTF_SUIT_PBR_END_TESTS_MAC(x,n)                              \
        0};                                                          \
        if (s_##x != NULL)                                           \
        {                                                            \
            PRV_TGF_IP_ROUTING_MODE_ENT routingMode;                 \
            prvTgfIpRoutingModeGet(&routingMode);                    \
            if (routingMode == PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E) \
                s_##x[1 + 2 * n] = NULL;     /* keep n tests */      \
            st = utfSuitAdd(s_##x);                                  \
        }                                                            \
        return st;                                                   \
    }                                                                \


/*
 * Description: Includes test case into test suit.
 *
 * Parameters:
 *      x - tested function name
 *
 * Comments: Name of test case must be unique inside one suit and
 *      can be not unique in different suits.
 */
#define UTF_SUIT_DECLARE_TEST_MAC(x) #x, x##UT,

/*
 * Description: Start test case implementation with this macro.
 *
 * Parameters:
 *      x - tested function name
 *
 * Comments: use this name later in test suit declaration.
 */
#define UTF_TEST_CASE_MAC(x) static GT_VOID x##UT(GT_VOID)

extern void utfExtraTestEnded(void);
/*
 * Description: macro to call UT test for given test name
 *
 * Parameters:
 *      x - test name
 *
 * Comments: use this to call from one test to other test
 */
#define UTF_TEST_CALL_MAC(x)    \
    x##UT();                    \
    utfExtraTestEnded()/* indicate that 'sub' test ended */

/* UT_TEST_FUNC_TYPE - test function type */
typedef GT_VOID (*UT_TEST_FUNC_TYPE)(GT_VOID);
/* test function name */
#define UTF_TEST_NAME_MAC(x)    x##UT

/*
 * Description: This macro checks if received value is equal to expected value.
 *      It is used when there is no any argument in output string.
 *
 * Parameters:
 *      e - expected value
 *      r - received value
 *
 * Comments: Test cases must use this macro for test flow validation.
 *      Output string has format accordingly to "CPSS Unit Test SDD":[CPSS_UT_0130]
 */
#define UTF_VERIFY_EQUAL0_PARAM_MAC(e, r)                                               \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            err = utfFailureMsgLog((const GT_CHAR *)NULL, (const GT_UINTPTR *)NULL, 0);                     \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is equal to expected value.
 *      It is used when there is only one argument in output string.
 *
 * Parameters:
 *      e -       expected value
 *      r -       received value
 *      arg1 -    argument for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 *      Output string has format accordingly to "CPSS Unit Test SDD":[CPSS_UT_0130]
 */
#define UTF_VERIFY_EQUAL1_PARAM_MAC(e, r, arg1)                                         \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_1_MAC(argvp,arg1);                                            \
            err = utfFailureMsgLog((const GT_CHAR *)NULL, argvp, 1);                 \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is equal to expected value.
 *      It is used when there are two arguments in output string.
 *
 * Parameters:
 *      e -           expected value
 *      r -           received value
 *      arg1, arg2 -  arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 *      Output string has format accordingly to "CPSS Unit Test SDD":[CPSS_UT_0130]
 */
#define UTF_VERIFY_EQUAL2_PARAM_MAC(e, r, arg1, arg2)                                   \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_2_MAC(argvp,arg1,arg2);                                       \
            err = utfFailureMsgLog((const GT_CHAR *)NULL, argvp, 2);                    \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is equal to expected value.
 *      It is used when there are three arguments in output string.
 *
 * Parameters:
 *      e -                   expected value
 *      r -                   received value
 *      arg1, arg2, arg3 -    arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 *      Output string has format accordingly to "CPSS Unit Test SDD":[CPSS_UT_0130]
 */
#define UTF_VERIFY_EQUAL3_PARAM_MAC(e, r, arg1, arg2, arg3)                             \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_3_MAC(argvp,arg1,arg2,arg3);                                  \
            err = utfFailureMsgLog((const GT_CHAR *)NULL, argvp, 3);                    \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is equal to expected value.
 *      It is used when there are four arguments in output string.
 *
 * Parameters:
 *      e                       -    expected value
 *      r                       -    received value
 *      arg1, arg2, arg3, arg4  -    arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 *      Output string has format accordingly to "CPSS Unit Test SDD":[CPSS_UT_0130]
 */
#define UTF_VERIFY_EQUAL4_PARAM_MAC(e, r, arg1, arg2, arg3, arg4)                       \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_4_MAC(argvp,arg1,arg2,arg3,arg4);                             \
            err = utfFailureMsgLog((const GT_CHAR *)NULL, argvp, 4);                    \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is equal to expected value.
 *      It is used when there are five arguments in output string.
 *
 * Parameters:
 *      e                               -    expected value
 *      r                               -    received value
 *      arg1, arg2, arg3, arg4, arg5    -    arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 *      Output string has format accordingly to "CPSS Unit Test SDD":[CPSS_UT_0130]
 */
#define UTF_VERIFY_EQUAL5_PARAM_MAC(e, r, arg1, arg2, arg3, arg4, arg5)                 \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_5_MAC(argvp,arg1,arg2,arg3,arg4,arg5);                        \
            err = utfFailureMsgLog(NULL, argvp, 5);                                     \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is equal to expected value.
 *      It is used when there are six arguments in output string.
 *
 * Parameters:
 *      e                                   -    expected value
 *      r                                   -    received value
 *      arg1, arg2, arg3, arg4, arg5, arg6  -    arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 *      Output string has format accordingly to "CPSS Unit Test SDD":[CPSS_UT_0130]
 */
#define UTF_VERIFY_EQUAL6_PARAM_MAC(e, r, arg1, arg2, arg3, arg4, arg5, arg6)           \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_6_MAC(argvp,arg1,arg2,arg3,arg4,arg5,arg6);                   \
            err = utfFailureMsgLog(NULL, argvp, 6);                                     \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)
/*
 * Description: This macro checks if received value is equal to expected value.
 *      It is used when there are seven arguments in output string.
 *
 * Parameters:
 *      e                                   -    expected value
 *      r                                   -    received value
 *      arg1, arg2, arg3, arg4, arg5, arg6, arg7  -    arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 *      Output string has format accordingly to "CPSS Unit Test SDD":[CPSS_UT_0130]
 */
#define UTF_VERIFY_EQUAL7_PARAM_MAC(e, r, arg1, arg2, arg3, arg4, arg5, arg6, arg7)     \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_7_MAC(argvp,arg1,arg2,arg3,arg4,arg5,arg6,arg7);              \
            err = utfFailureMsgLog(NULL, argvp, 7);                                     \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is NOT equal to expected value.
 *      It is used when there is no any argument in output string.
 *
 * Parameters:
 *      e -  expected value
 *      r -  received value
 *
 * Comments: Test cases must use this macro for test flow validation.
 *      Output string has format accordingly to "CPSS Unit Test SDD":[CPSS_UT_0130]
 */
#define UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(e, r)                                           \
    do {                                                                                \
        GT_BOOL err = utfNotEqualVerify(e, r, __LINE__, __FILE__);                      \
        if (GT_FALSE == err) {                                                          \
            err = utfFailureMsgLog((const GT_CHAR *)NULL, (GT_UINTPTR *)NULL, 0);       \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is NOT equal to expected value.
 *      It is used when there is only one argument in output string.
 *
 * Parameters:
 *      e -       expected value
 *      r -       received value
 *      arg1 -    argument for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 *      Output string has format accordingly to "CPSS Unit Test SDD":[CPSS_UT_0130]
 */
#define UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(e, r, arg1)                                     \
    do {                                                                                \
        GT_BOOL err = utfNotEqualVerify(e, r, __LINE__, __FILE__);                      \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_1_MAC(argvp,arg1);                                            \
            err = utfFailureMsgLog((const GT_CHAR *)NULL, argvp, 1);                    \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is NOT equal to expected value.
 *      It is used when there are two arguments in output string.
 *
 * Parameters:
 *      e -           expected value
 *      r -           received value
 *      arg1, arg2 -  arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 *      Output string has format accordingly to "CPSS Unit Test SDD":[CPSS_UT_0130]
 */
#define UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(e, r, arg1, arg2)                               \
    do {                                                                                \
        GT_BOOL err = utfNotEqualVerify(e, r, __LINE__, __FILE__);                      \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_2_MAC(argvp,arg1,arg2);                        \
            err = utfFailureMsgLog((GT_CHAR*)NULL, argvp, 2);                                     \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is NOT equal to expected value.
 *      It is used when there are three arguments in output string.
 *
 * Parameters:
 *      e -                   expected value
 *      r -                   received value
 *      arg1, arg2, arg3 -    arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 *      Output string has format accordingly to "CPSS Unit Test SDD":[CPSS_UT_0130]
 */
#define UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(e, r, arg1, arg2, arg3)                         \
    do {                                                                                \
        GT_BOOL err = utfNotEqualVerify(e, r, __LINE__, __FILE__);                      \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_3_MAC(argvp,arg1,arg2,arg3);                                  \
            err = utfFailureMsgLog((GT_CHAR*)NULL, argvp, 3);                           \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is NOT equal to expected value.
 *      It is used when there are four arguments in output string.
 *
 * Parameters:
 *      e                       -    expected value
 *      r                       -    received value
 *      arg1, arg2, arg3, arg4  -    arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 *      Output string has format accordingly to "CPSS Unit Test SDD":[CPSS_UT_0130]
 */
#define UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(e, r, arg1, arg2, arg3, arg4)                   \
    do {                                                                                \
        GT_BOOL err = utfNotEqualVerify(e, r, __LINE__, __FILE__);                      \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_4_MAC(argvp,arg1,arg2,arg3,arg4);                        \
            err = utfFailureMsgLog(NULL, argvp, 4);                                     \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is NOT equal to expected value.
 *      It is used when there are five arguments in output string.
 *
 * Parameters:
 *      e                               -    expected value
 *      r                               -    received value
 *      arg1, arg2, arg3, arg4, arg5    -    arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 *      Output string has format accordingly to "CPSS Unit Test SDD":[CPSS_UT_0130]
 */
#define UTF_VERIFY_NOT_EQUAL5_PARAM_MAC(e, r, arg1, arg2, arg3, arg4, arg5)             \
    do {                                                                                \
        GT_BOOL err = utfNotEqualVerify(e, r, __LINE__, __FILE__);                      \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_5_MAC(argvp,arg1,arg2,arg3,arg4,arg5);                        \
            err = utfFailureMsgLog(NULL, argvp, 5);                                     \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is NOT equal to expected value.
 *      It is used when there are six arguments in output string.
 *
 * Parameters:
 *      e                                   -    expected value
 *      r                                   -    received value
 *      arg1, arg2, arg3, arg4, arg5, arg6  -    arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 *      Output string has format accordingly to "CPSS Unit Test SDD":[CPSS_UT_0130]
 */
#define UTF_VERIFY_NOT_EQUAL6_PARAM_MAC(e, r, arg1, arg2, arg3, arg4, arg5, arg6)       \
    do {                                                                                \
        GT_BOOL err = utfNotEqualVerify(e, r, __LINE__, __FILE__);                      \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_6_MAC(argvp,arg1,arg2,arg3,arg4,arg5,arg6);                   \
            err = utfFailureMsgLog(NULL, argvp, 6);                                     \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is NOT equal to expected value.
 *      It is used when there are seven arguments in output string.
 *
 * Parameters:
 *      e                                   -    expected value
 *      r                                   -    received value
 *      arg1, arg2, arg3, arg4, arg5, arg6, arg7  -    arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 *      Output string has format accordingly to "CPSS Unit Test SDD":[CPSS_UT_0130]
 */
#define UTF_VERIFY_NOT_EQUAL7_PARAM_MAC(e, r, arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
    do {                                                                                \
        GT_BOOL err = utfNotEqualVerify(e, r, __LINE__, __FILE__);                      \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_7_MAC(argvp,arg1,arg2,arg3,arg4,arg5,arg6,arg7);              \
            err = utfFailureMsgLog(NULL, argvp, 7);                                     \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is equal to expected value.
 *      It is used when there is no any argument in output string.
 *
 * Parameters:
 *      e -    expected value
 *      r -    received value
 *      s -    format string for output
 *
 * Comments: Test cases must use this macro for test flow validation.
 */
#define UTF_VERIFY_EQUAL0_STRING_MAC(e, r, s)                                           \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            err = utfFailureMsgLog(s, (GT_UINTPTR *)NULL, 0);                           \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is equal to expected value.
 *      It is used when there is only one argument in output string.
 *
 * Parameters:
 *      e -    expected value
 *      r -    received value
 *      s -    format string for output
 *      arg1 - argument for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 */
#define UTF_VERIFY_EQUAL1_STRING_MAC(e, r, s, arg1)                                     \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_1_MAC(argvp,arg1);                        \
            err = utfFailureMsgLog(s, argvp, 1);                                        \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is equal to expected value.
 *      It is used when there are two arguments in output string.
 *
 * Parameters:
 *      e -           expected value
 *      r -           received value
 *      s -           format string for output
 *      arg1, arg2 -  arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 */
#define UTF_VERIFY_EQUAL2_STRING_MAC(e, r, s, arg1, arg2)                               \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_2_MAC(argvp,arg1,arg2);                        \
            err = utfFailureMsgLog(s, argvp, 2);                                        \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is equal to expected value.
 *      It is used when there are three arguments in output string.
 *
 * Parameters:
 *      e -                 expected value
 *      r -                 received value
 *      s -                 format string for output
 *      arg1, arg2, arg3 -  arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 */
#define UTF_VERIFY_EQUAL3_STRING_MAC(e, r, s, arg1, arg2, arg3)                         \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_3_MAC(argvp,arg1,arg2,arg3);                        \
            err = utfFailureMsgLog(s, argvp, 3);                                        \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is equal to expected value.
 *      It is used when there are four arguments in output string.
 *
 * Parameters:
 *      e                       -  expected value
 *      r                       -  received value
 *      s                       -  format string for output
 *      arg1, arg2, arg3, arg4  -  arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 */
#define UTF_VERIFY_EQUAL4_STRING_MAC(e, r, s, arg1, arg2, arg3, arg4)                   \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_4_MAC(argvp,arg1,arg2,arg3,arg4);                        \
            err = utfFailureMsgLog(s, argvp, 4);                                        \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is equal to expected value.
 *      It is used when there are five arguments in output string.
 *
 * Parameters:
 *      e                               -  expected value
 *      r                               -  received value
 *      s                               -  format string for output
 *      arg1, arg2, arg3, arg4, arg5    -  arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 */
#define UTF_VERIFY_EQUAL5_STRING_MAC(e, r, s, arg1, arg2, arg3, arg4, arg5)             \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_5_MAC(argvp,arg1,arg2,arg3,arg4,arg5);                        \
            err = utfFailureMsgLog(s, argvp, 5);                                        \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is equal to expected value.
 *      It is used when there are six arguments in output string.
 *
 * Parameters:
 *      e                                   -  expected value
 *      r                                   -  received value
 *      s                                   -  format string for output
 *      arg1, arg2, arg3, arg4, arg5, arg6  -  arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 */
#define UTF_VERIFY_EQUAL6_STRING_MAC(e, r, s, arg1, arg2, arg3, arg4, arg5,arg6)        \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_6_MAC(argvp,arg1,arg2,arg3,arg4,arg5,arg6);                   \
            err = utfFailureMsgLog(s, argvp, 6);                                        \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is equal to expected value.
 *      It is used when there are seven arguments in output string.
 *
 * Parameters:
 *      e                                   -  expected value
 *      r                                   -  received value
 *      s                                   -  format string for output
 *      arg1, arg2, arg3, arg4, arg5, arg6, arg7  -  arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 */
#define UTF_VERIFY_EQUAL7_STRING_MAC(e, r, s, arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_7_MAC(argvp,arg1,arg2,arg3,arg4,arg5,arg6,arg7);              \
            err = utfFailureMsgLog(s, argvp, 7);                                        \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is NOT equal to expected value.
 *      It is used when there is no any argument in output string.
 *
 * Parameters:
 *      e -    expected value
 *      r -    received value
 *      s -    format string for output
 *
 * Comments: Test cases must use this macro for test flow validation.
 */
#define UTF_VERIFY_NOT_EQUAL0_STRING_MAC(e, r, s)                                       \
    do {                                                                                \
        GT_BOOL err = utfNotEqualVerify(e, r, __LINE__, __FILE__);                      \
        if (GT_FALSE == err) {                                                          \
            err = utfFailureMsgLog(s, NULL, 0);                                         \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is NOT equal to expected value.
 *      It is used when there is only one argument in output string.
 *
 * Parameters:
 *      e -    expected value
 *      r -    received value
 *      s -    format string for output
 *      arg1 - argument for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 */
#define UTF_VERIFY_NOT_EQUAL1_STRING_MAC(e, r, s, arg1)                                 \
    do {                                                                                \
        GT_BOOL err = utfNotEqualVerify(e, r, __LINE__, __FILE__);                      \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_1_MAC(argvp,arg1);                        \
            err = utfFailureMsgLog(s, argvp, 1);                                        \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is NOT equal to expected value.
 *      It is used when there are two arguments in output string.
 *
 * Parameters:
 *      e -           expected value
 *      r -           received value
 *      s -           format string for output
 *      arg1, arg2 -  arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 */
#define UTF_VERIFY_NOT_EQUAL2_STRING_MAC(e, r, s, arg1, arg2)                           \
    do {                                                                                \
        GT_BOOL err = utfNotEqualVerify(e, r, __LINE__, __FILE__);                      \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_2_MAC(argvp,arg1,arg2);                        \
            err = utfFailureMsgLog(s, argvp, 2);                                        \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is NOT equal to expected value.
 *      It is used when there are three arguments in output string.
 *
 * Parameters:
 *      e -                 expected value
 *      r -                 received value
 *      s -                 format string for output
 *      arg1, arg2, arg3 -  arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 */
#define UTF_VERIFY_NOT_EQUAL3_STRING_MAC(e, r, s, arg1, arg2, arg3)                     \
    do {                                                                                \
        GT_BOOL err = utfNotEqualVerify(e, r, __LINE__, __FILE__);                      \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_3_MAC(argvp,arg1,arg2,arg3);                        \
            err = utfFailureMsgLog(s, argvp, 3);                                        \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is NOT equal to expected value.
 *      It is used when there are four arguments in output string.
 *
 * Parameters:
 *      e                       -  expected value
 *      r                       -  received value
 *      s                       -  format string for output
 *      arg1, arg2, arg3, arg4  -  arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 */
#define UTF_VERIFY_NOT_EQUAL4_STRING_MAC(e, r, s, arg1, arg2, arg3, arg4)               \
    do {                                                                                \
        GT_BOOL err = utfNotEqualVerify(e, r, __LINE__, __FILE__);                      \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_4_MAC(argvp,arg1,arg2,arg3,arg4);                        \
            err = utfFailureMsgLog(s, argvp, 4);                                        \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is NOT equal to expected value.
 *      It is used when there are five arguments in output string.
 *
 * Parameters:
 *      e                               -  expected value
 *      r                               -  received value
 *      s                               -  format string for output
 *      arg1, arg2, arg3, arg4, arg5    -  arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 */
#define UTF_VERIFY_NOT_EQUAL5_STRING_MAC(e, r, s, arg1, arg2, arg3, arg4, arg5)         \
    do {                                                                                \
        GT_BOOL err = utfNotEqualVerify(e, r, __LINE__, __FILE__);                      \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_5_MAC(argvp,arg1,arg2,arg3,arg4,arg5);                        \
            err = utfFailureMsgLog(s, argvp, 5);                                        \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is NOT equal to expected value.
 *      It is used when there are six arguments in output string.
 *
 * Parameters:
 *      e                                   -  expected value
 *      r                                   -  received value
 *      s                                   -  format string for output
 *      arg1, arg2, arg3, arg4, arg5, arg6  -  arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 */
#define UTF_VERIFY_NOT_EQUAL6_STRING_MAC(e, r, s, arg1, arg2, arg3, arg4, arg5, arg6)   \
    do {                                                                                \
        GT_BOOL err = utfNotEqualVerify(e, r, __LINE__, __FILE__);                      \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_6_MAC(argvp,arg1,arg2,arg3,arg4,arg5,arg6);                   \
            err = utfFailureMsgLog(s, argvp, 6);                                        \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is NOT equal to expected value.
 *      It is used when there are seven arguments in output string.
 *
 * Parameters:
 *      e                                   -  expected value
 *      r                                   -  received value
 *      s                                   -  format string for output
 *      arg1, arg2, arg3, arg4, arg5, arg6, arg7  -  arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 */
#define UTF_VERIFY_NOT_EQUAL7_STRING_MAC(e, r, s, arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
    do {                                                                                \
        GT_BOOL err = utfNotEqualVerify(e, r, __LINE__, __FILE__);                      \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_7_MAC(argvp,arg1,arg2,arg3,arg4,arg5,arg6,arg7);              \
            err = utfFailureMsgLog(s, argvp, 7);                                        \
            if (GT_FALSE == err) return;                                                \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is equal to expected value.
 *      It is used when there are two arguments in output string.
 *
 * Parameters:
 *      e -           expected value
 *      r -           received value
 * Comments: Test cases must use this macro for test flow validation.
 */
#define UTF_VERIFY_EQUAL0_NO_RETURN_MAC(e, r)                                           \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            err = utfFailureMsgLog((const GT_CHAR *)NULL, (const GT_UINTPTR *)NULL, 0); \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is equal to expected value.
 *      It is used when there are two arguments in output string.
 *
 * Parameters:
 *      e -           expected value
 *      r -           received value
 *      arg1, arg2 -  arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 *      Output string has format accordingly to "CPSS Unit Test SDD":[CPSS_UT_0130]
 */
#define UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(e, r, arg1, arg2)                                   \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_2_MAC(argvp,arg1,arg2);                                       \
            err = utfFailureMsgLog((const GT_CHAR *)NULL, argvp, 2);                    \
        }                                                                               \
    } while(0)


 /* Description: This macro checks if received value is equal to expected value.
 *      It is used when there are three arguments in output string.
 *
 * Parameters:
 *      e -                   expected value
 *      r -                   received value
 *      arg1, arg2, arg3 -    arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 *      Output string has format accordingly to "CPSS Unit Test SDD":[CPSS_UT_0130]
 */

#define UTF_VERIFY_EQUAL3_NO_RETURN_MAC(e, r, arg1, arg2, arg3)                         \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_3_MAC(argvp,arg1,arg2,arg3);                                  \
            err = utfFailureMsgLog((const GT_CHAR *)NULL, argvp, 3);                    \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is equal to expected value.
 *      It is used when there is only one argument in output string.
 *
 * Parameters:
 *      e -    expected value
 *      r -    received value
 *      s -    format string for output
 *      arg1 - argument for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 */
#define UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(e, r, s, arg1)                           \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_1_MAC(argvp,arg1);                                            \
            err = utfFailureMsgLog(s, argvp, 1);                                        \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is equal to expected value.
 *      It is used when there are two arguments in output string.
 *
 * Parameters:
 *      e -           expected value
 *      r -           received value
 *      s -           format string for output
 *      arg1, arg2 -  arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 */
#define UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(e, r, s, arg1, arg2)                               \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_2_MAC(argvp,arg1,arg2);                        \
            err = utfFailureMsgLog(s, argvp, 2);                                        \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is equal to expected value.
 *      It is used when there are three arguments in output string.
 *
 * Parameters:
 *      e -                 expected value
 *      r -                 received value
 *      s -                 format string for output
 *      arg1, arg2, arg3 -  arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 */
#define UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(e, r, s, arg1, arg2, arg3)                         \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_3_MAC(argvp,arg1,arg2,arg3);                        \
            err = utfFailureMsgLog(s, argvp, 3);                                        \
        }                                                                               \
    } while(0)

/*
 * Description: This macro checks if received value is NOT equal to expected value.
 *      It is used when there are two arguments in output string.
 *
 * Parameters:
 *      e -           expected value
 *      r -           received value
 *      arg1, arg2 -  arguments for output string
 *
 * Comments: Test cases must use this macro for test flow validation.
 *      Output string has format accordingly to "CPSS Unit Test SDD":[CPSS_UT_0130]
 */
#define UTF_VERIFY_NOT_EQUAL2_PARAM_NO_RETURN_MAC(e, r, arg1, arg2)                               \
    do {                                                                                \
        GT_BOOL err = utfNotEqualVerify(e, r, __LINE__, __FILE__);                      \
        if (GT_FALSE == err) {                                                          \
            UTF_ARGVP_ARR_2_MAC(argvp,arg1,arg2);                        \
            err = utfFailureMsgLog((GT_CHAR*)NULL, argvp, 2);                                     \
        }                                                                               \
    } while(0)

/* default device number */
extern __THREAD_LOCAL GT_U8    prvTgfDevNum; /* = 0; */
/* flag to state the test uses port groups bmp */
extern GT_BOOL              usePortGroupsBmp; /* default is  GT_FALSE*/;
/* bmp of port groups to use in test */
extern GT_PORT_GROUPS_BMP   currPortGroupsBmp /* default is CPSS_PORT_GROUP_UNAWARE_MODE_CNS*/;
/* number of port groups that we look for FDB */
extern GT_U32   prvTgfNumOfPortGroups /* = 0 */;


/**
* @internal utfPreInitPhase function
* @endinternal
*
* @brief   This routine start initializing UTF core. It must be called first.
*
* @retval GT_OK                    -  UTF initialization was successful.
* @retval GT_CREATE_ERROR          -  UTF has no more free entries to register test cases.
* @retval GT_FAIL                  -  General failure error. Should never happen.
*
* @note It's good hint to insert this function inside some global configuration
*       function, as part of initialization phase.
*
*/
GT_STATUS utfPreInitPhase
(
    GT_VOID
);

/**
* @internal utfPostInitPhase function
* @endinternal
*
* @brief   This routine finish initializing UTF core. It must be called last.
*
* @param[in] st                       - return code from UTF initialize phase.
*                                       None.
*
* @note It's good hint to insert this function inside some global configuration
*       function, as part of initialization phase.
*
*/
GT_VOID utfPostInitPhase
(
    GT_STATUS st
);

/**
* @internal utfInit function
* @endinternal
*
* @brief   This routine initializes UTF core.
*         It must be called after utfPreInitPhase.
* @param[in] firstDevNum              - device number of first device.
*
* @retval GT_OK                    -  UTF initialization was successful.
* @retval GT_CREATE_ERROR          -  UTF has no more free entries to register test cases.
* @retval GT_FAIL                  -  General failure error. Should never happen.
*
* @note It's good hint to insert this function inside some global configuration
*       function, as part of initialization phase.
*
*/
GT_STATUS utfInit
(
    GT_U8 firstDevNum
);

/**
* @internal utfHelp function
* @endinternal
*
* @brief   This routine log a help about the function, if function name is specified
*         as input argument. In case of NULL input argument this function displays
*         all available UTF functions.
* @param[in] utfFuncNamePtr           - (pointer to) UTF-function name.
*
* @retval GT_OK                    -  Help logs information successfully.
* @retval GT_BAD_PARAM             -  Required function doesn't exist.
* @retval GT_FAIL                  -  General failure error. Should never happen.
*/
GT_STATUS utfHelp
(
    IN const GT_CHAR *utfFuncNamePtr
);

/**
* @internal utfTestsList function
* @endinternal
*
* @brief   This routine prints list of configured suits in case of NULL input argument
*         or print list of test cases inside given suit.
* @param[in] suitNamePtr              -   (pointer to) suit name.
*
* @retval GT_OK                    -  Printing of suits was successful.
* @retval GT_NOT_FOUND             -  Given suit wasn't found in the UTF suit list.
* @retval GT_FAIL                  -  General failure error. Should never happen.
*/
GT_STATUS utfTestsList
(
    IN const GT_CHAR *suitNamePtr
);

/**
* @internal utfTestsTypeList function
* @endinternal
*
* @brief   This routine prints all tests list of specific test type.
*
* @param[in] testType                 - (list of) type of tests, use 0 to set the end of list
*
* @retval GT_OK                    -  Printing of suits was successful.
* @retval GT_NOT_FOUND             -  Given suit wasn't found in the UTF suit list.
* @retval GT_FAIL                  -  General failure error. Should never happen.
*/
GT_STATUS utfTestsTypeList
(
    IN UTF_TEST_TYPE_ENT    testType,
    IN ...
);

/**
* @internal utfSkipListAdd function
* @endinternal
*
* @brief   Add a test to skip list
*
* @param[in] str                      - test name: "suitName.testName"
* @param[in] reason                   - skip  (message)
*                                       GT_STATUS
*/
GT_STATUS utfSkipListAdd(
  IN  const GT_CHAR *str,
  IN  const GT_CHAR *reason
);

/**
* @internal utfTestsRun function
* @endinternal
*
* @brief   Runs all suits in case of NULL input path (testPathPtr),
*         or suit if suite name is specified as testPathPtr,
*         or specific test in the suit.
* @param[in] testPathPtr              -   (pointer to) name of suite or test case to be run.
*                                      In case of test case has format <suit.testcase>.
* @param[in] numOfRuns                -   defines how many times tests will be executed in loop.
* @param[in] fContinue                -   defines will test be interrupted after first failure condition.
*
* @retval GT_OK                    -  Running of tests was successful.
* @retval GT_NOT_FOUND             -  Test path wasn't found.
* @retval GT_BAD_PARAM             -  Bad number of cycles was passed.
* @retval GT_UTF_TEST_FAILED       -  Some test fails.
* @retval GT_FAIL                  -  General failure error. Should never happen.
*/
GT_STATUS utfTestsRun
(
    IN const GT_CHAR *testPathPtr,
    IN GT_U32        numOfRuns,
    IN GT_BOOL       fContinue
);

/**
* @internal utfTestsTypeRun function
* @endinternal
*
* @brief   Runs all suits in case of NULL input path (testPathPtr),
*         or suit if suite name is specified as testPathPtr,
*         or specific test in the suit.
* @param[in] numOfRuns                - defines how many times tests will be executed in loop
* @param[in] fContinue                - defines will test be interrupted after first failure condition
* @param[in] testType                 - (list of) type of tests, use 0 to set the end of list
*
* @retval GT_OK                    -  Running of tests was successful.
* @retval GT_NOT_FOUND             -  Test path wasn't found.
* @retval GT_BAD_PARAM             -  Bad number of cycles was passed.
* @retval GT_UTF_TEST_FAILED       -  Some test fails.
* @retval GT_FAIL                  -  General failure error. Should never happen.
*/
GT_STATUS utfTestsTypeRun
(
    IN GT_U32               numOfRuns,
    IN GT_BOOL              fContinue,
    IN UTF_TEST_TYPE_ENT    testType,
    IN ...
);

/**
* @internal utfTestsStartRun function
* @endinternal
*
* @brief   Runs suits or specific tests from the start point.
*
* @param[in] testPathPtr              -   (pointer to) name of start suite or test case to be run.
*                                      In case of test case has format <suit.testcase>.
* @param[in] numOfRuns                -   defines how many times tests will be executed in loop.
* @param[in] fContinue                -   defines will test be interrupted after first failure condition.
*
* @retval GT_OK                    -  Running of tests was successful.
* @retval GT_NOT_FOUND             -  Test path wasn't found.
* @retval GT_BAD_PARAM             -  Bad number of cycles was passed.
* @retval GT_UTF_TEST_FAILED       -  Some test fails.
* @retval GT_FAIL                  -  General failure error. Should never happen.
*/
GT_STATUS utfTestsStartRun
(
    IN const GT_CHAR *testPathPtr,
    IN GT_U32        numOfRuns,
    IN GT_BOOL       fContinue
);

/**
* @internal utfLogOutputSelect function
* @endinternal
*
* @brief   Switches logger output between serial interface and file.
*
* @param[in] eLogOutput               -   kind of log output
*
* @retval GT_OK                    -  Output log interface was selected successfully.
* @retval GT_BAD_PARAM             -  Invalid output interface id was passed.
* @retval GT_FAIL                  -  General failure error. Should never happen.
*
* @note By default output is done into serial interface.
*       Use this function if you want to have a log file.
*
*/
GT_STATUS utfLogOutputSelect
(
    IN UTF_LOG_OUTPUT_ENT eLogOutput
);

/**
* @internal utfLogOpen function
* @endinternal
*
* @brief   Opens logger with defined file name.
*
* @param[in] fileName                 -   log output file name
*
* @retval GT_OK                    -  logger has been successfully opened.
* @retval GT_BAD_PARAM             -  Invalid output interface id was passed.
* @retval GT_FAIL                  -  General failure error. Should never happen.
*
* @note Available kinds of log output are serial interface and file.
*
*/
GT_STATUS utfLogOpen
(
    IN const GT_CHAR* fileName
);

/**
* @internal utfLogClose function
* @endinternal
*
* @brief   This routine closes logger.
*/
GT_STATUS utfLogClose
(
    GT_VOID
);

/**
* @internal prvUtfLogReadParam function
* @endinternal
*
* @brief   Read parameters from file to array.
*
* @param[in] fileName                 - log output file name to check
* @param[in] numberParams             - number of parameters to read
*
* @param[out] arrayOfParamPtr          - array of params from result file
*
* @retval GT_OK                    - logger has been successfully opened.
* @retval GT_BAD_PARAM             - Invalid output interface id was passed.
* @retval GT_BAD_SIZE              - Invalid input file size.
* @retval GT_NO_RESOURCE           - Memory error.
* @retval GT_FAIL                  - General failure error. Should never happen.
*
* @note this functions used by utfLogResultRun
*
*/
GT_STATUS prvUtfLogReadParam
(
    IN  const GT_CHAR         *fileName,
    IN  GT_8                  numberParams,
    OUT UTF_RESULT_PARAM_STC *arrayOfParamPtr
);

/**
* @internal utfTestTimeOutSet function
* @endinternal
*
* @brief   This routine set timeout for single test.
*
* @param[in] testTimeOut              -   test timeout in milliseconds (0 to wait forever).
*
* @retval GT_OK                    - on success
*/
GT_STATUS utfTestTimeOutSet
(
    IN GT_U32   testTimeOut
);

/**
* @internal utfEqualVerify function
* @endinternal
*
* @brief   This routine is used for handling test failures (e != r) ,
*         error calculation.
* @param[in] e                        -   expected value.
* @param[in] r                        -   received value.
* @param[in] lineNum                  -   number of line in test file where test failure
*                                      has been found.
* @param[in] fileNamePtr              -   (pointer to) name of test file where error has been
*                                      found.
*
* @param[out] e
*
* @retval GT_TRUE                  -  Pass condition was correct. Test may continue its flow.
* @retval GT_FALSE                 -  Pass condition is invalid. Test is broken.
*
* @note Test cases must not call this function directly.
*       It is responsibility of UTF macro to continue test flow after failure
*       or not - is specified as an argument in utfRunTests routine.
*
*/
GT_BOOL utfEqualVerify
(
    IN GT_UINTPTR   e,
    IN GT_UINTPTR   r,
    IN GT_32        lineNum,
    IN GT_CHAR      *fileNamePtr
);

/**
* @internal utfNotEqualVerify function
* @endinternal
*
* @brief   This routine is used for handling test failures (e == r), error
*         calculation.
* @param[in] e                        -   expected value.
* @param[in] r                        -   received value.
* @param[in] lineNum                  -   number of line in test file where test failure
*                                      has been found.
* @param[in] fileNamePtr              -   (pointer to) name of test file where test failure
*                                      has been found.
*
* @param[out] e
*
* @retval GT_TRUE                  -  Pass condition was correct. Test may continue its flow.
* @retval GT_FALSE                 -  Pass condition is invalid. Test is broken.
*
* @note Test cases must not call this function directly.
*       It is responsibility of UTF macro to continue test flow after failure
*       or not - is specified as an argument in utfRunTests routine.
*
*/
GT_BOOL utfNotEqualVerify
(
    IN GT_UINTPTR   e,
    IN GT_UINTPTR   r,
    IN GT_32        lineNum,
    IN GT_CHAR      *fileNamePtr
);

/**
* @internal utfFailureMsgLog function
* @endinternal
*
* @brief   This function is used for displaying failure information.
*
* @param[in] failureMsgPtr            -   (pointer to) failure message (format string).
* @param[in] argvPtr[]                -   (pointer to) vector of failure message arguments.
* @param[in] argc                     -   number of failure message arguments.
*
* @retval GT_TRUE                  -  Test may continue its flow.
* @retval GT_FALSE                 -  Test must be interrupted.
*
* @note Test cases must not call this function directly.
*
*/
GT_BOOL utfFailureMsgLog
(
    IN const GT_CHAR    *failureMsgPtr,
    IN const GT_UINTPTR argvPtr[],
    IN GT_U32           argc
);

/**
* @internal utfSuitAdd function
* @endinternal
*
* @brief   This routine adds Suit.
*
* @param[in] suitPtr[]                -   (pointer to) a structure, where
*                                      first 2 bytes define a pointer to a Suit name
*                                      next blocks of 4 bytes define tests of this Suit, where
*                                      first 2 bytes define Test name,
*                                      last 2 bytes define pointer to test case function.
*
* @retval GT_OK                    -  Suit has been successully added.
* @retval GT_CREATE_ERROR          -  it's impossible to add Suit because maximum count of
*                                       tests has been already gained.
*/
GT_STATUS utfSuitAdd
(
    IN const GT_VOID *suitPtr[]
);

/**
* @internal prvUtfLogPrintEnable function
* @endinternal
*
* @brief   This routine Enables\Disables printing.
*/
UTF_LOG_OUTPUT_ENT prvUtfLogPrintEnable
(
    UTF_LOG_OUTPUT_ENT utfLogOutputSelectMode
);

/**
* @internal prvUtfLogPrintModeGet function
* @endinternal
*
* @brief   This routine gets printing mode.
*/
UTF_LOG_OUTPUT_ENT prvUtfLogPrintModeGet
(
    GT_VOID
);

/**
* @internal prvUtfLogPrintEnableGet function
* @endinternal
*
* @brief   This routine gets printing mode.
*/
GT_BOOL prvUtfLogPrintEnableGet
(
    GT_VOID
);

/**
* @internal utfDeclareTestType function
* @endinternal
*
* @brief   Declare global test type
*
* @param[in] testType                 -   test type
*                                       none
*/
GT_VOID utfDeclareTestType
(
    IN UTF_TEST_TYPE_ENT testType
);

/**
* @internal utfSkipTests function
* @endinternal
*
* @brief   Declare test type to skip
*
* @param[in] testType                 -   test type
*
* @retval GT_OK                    - on success
*/
GT_STATUS utfSkipTests
(
    IN UTF_TEST_TYPE_ENT testType
);

/**
* @internal utfDefineTests function
* @endinternal
*
* @brief   Declare test type to run.
*         every call to this API override the previous call.
* @param[in] testType                 -   test type
*                                      NOTE: value UTF_TEST_TYPE_NONE_E --> used here as 'ALL' (and not NONE)
*
* @retval GT_OK                    - on success
*/
GT_STATUS utfDefineTests
(
    IN UTF_TEST_TYPE_ENT testType
);


/**
* @internal utfLogResultRun function
* @endinternal
*
* @brief   This function executes result file logging.
*
* @param[in] paramArrayPtr            - array of parameters, that passed to file
* @param[in] numberParam              - number of parameters to read
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note none
*
*/
GT_STATUS utfLogResultRun
(
    IN  UTF_RESULT_PARAM_STC         *paramArrayPtr,
    IN  GT_U8                         numberParam,
    OUT GT_BOOL                      *paramDeltaCompare
);

/**
* @internal utfStartTimer function
* @endinternal
*
* @brief   This function starts UTF timer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note none
*
*/
GT_STATUS utfStartTimer
(
    GT_VOID
);

/**
* @internal utfStopTimer function
* @endinternal
*
* @brief   This function stop UTF timer and return time measure in miliseconds.
*
* @param[out] timeElapsedPtr           - pointer to time elapsed value
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note none
*
*/
GT_STATUS utfStopTimer
(
    OUT GT_U32   *timeElapsedPtr
);

/**
* @internal utfMemoryHeapCounterInit function
* @endinternal
*
* @brief   This function initialize memory heap counter.
*
* @note Should be called before utfMemoryHeapLeakageStatusGet()
*
*/
GT_VOID utfMemoryHeapCounterInit
(
    GT_VOID
);

/**
* @internal utfMemoryHeapLeakageStatusGet function
* @endinternal
*
* @brief   This function get memory heap leakage status.
*
* @param[out] memPtr                   - (pointer to) memory leak size value
*
* @retval GT_OK                    - on success (no memory leak).
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_FAIL                  - on failure (memory leak detected).
*
* @note It is delta of current allocated bytes number and the value of allocation
*       counter set by previous utfMemoryHeapStatusInit() function
*
*/
GT_STATUS utfMemoryHeapLeakageStatusGet
(
    OUT GT_U32 *memPtr
);
/**
* @internal utfPrintKeepAlive function
* @endinternal
*
* @brief   This function print "." so user can see that test / test still running.
*         keep alive indication , that test is doing long processing , and between tests.
*
* @note none
*
*/
GT_VOID utfPrintKeepAlive
(
    GT_VOID
);

/**
* @internal utfGeneralStateMessageSave function
* @endinternal
*
* @brief   This function saves message about the general state.
*         Defines string with parameters to add to logger when test fail .
*         this string is about 'general state' of the test , like global parameters.
*         the caller can put into DB up to PRV_UTF_GENERAL_STATE_INFO_NUM_CNS such strings.
*         the DB is cleared at start of each test.
*         the DB is printed when test fail
* @param[in] index                    -  in the DB
* @param[in] formatStringPtr          -   (pointer to) format string.
*                                      when NULL - the index in DB is cleared.
*                                      ...             -    string arguments.
*
* @retval GT_OK                    -  the info saved to DB
* @retval GT_BAD_PARAM             -  the index >= PRV_UTF_GENERAL_STATE_INFO_NUM_CNS
*/
GT_STATUS utfGeneralStateMessageSave
(
    IN GT_U32           index,
    IN const GT_CHAR    *formatStringPtr,
    IN                  ...
);

/**
* @internal utfCallBackFunctionOnErrorSave function
* @endinternal
*
* @brief   This function saves a callback function.
*         callback function that need to be called on error .
*         this to allow a test to 'dump' into LOG/terminal important info that may
*         explain why the test failed. -- this is advanced debug tool
*         the DB is cleared at start of each test.
*         the DB is called when test fail
* @param[in] index                    -  in the DB
* @param[in] callBackFunc             - the call back function to save (can be NULL)
*
* @retval GT_OK                    -  the info saved to DB
* @retval GT_BAD_PARAM             -  the index >= PRV_UTF_GENERAL_STATE_INFO_NUM_CNS
*/
GT_STATUS utfCallBackFunctionOnErrorSave
(
    IN GT_U32           index,
    IN UTF_CALL_BACK_FUNCTION_ON_ERROR_TYPE    callBackFunc
);

/**
* @internal utfRandomRunModeSet function
* @endinternal
*
* @brief   This function enables test's random run mode
*
* @param[in] enable                   - enable\disable random run mode
* @param[in] seed                     - random number  (relevant only for random run mode)
*
* @retval GT_OK                    - on success
*/
GT_STATUS utfRandomRunModeSet
(
    IN GT_BOOL  enable,
    IN GT_U32   seed
);

/**
* @internal utfRandomRunModeGet function
* @endinternal
*
* @brief   return test's random run mode
*
* @param[out] enablePtr                - (pointer to) enable\disable random run mode
* @param[out] seedPtr                  - (pointer to) random number seed (relevant only for random run mode)
*
* @retval GT_OK                    - on success
*/
GT_STATUS utfRandomRunModeGet
(
    OUT GT_BOOL  *enablePtr,
    OUT GT_U32   *seedPtr
);

/**
* @internal utfTestRunFirstSet function
* @endinternal
*
* @brief   This function put specific test to be run first
*
* @param[in] testNamePtr              - test name to make it run first
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - on test not found
*/
GT_STATUS utfTestRunFirstSet
(
    IN const GT_CHAR   *testNamePtr
);

/*******************************************************************************
* utfTestNameGet
*
* DESCRIPTION:
*     return the name (string) of the current running test.
*
* INPUTS:
*     None
*
* OUTPUTS:
*     None
*
* RETURNS:
*     string - the name of the TEST
*
* COMMENTS:
*       assumption : called only for context of running test
*
*******************************************************************************/
const GT_CHAR* utfTestNameGet(GT_VOID);

/**
* @internal utfTestPrintPassOkSummary function
* @endinternal
*
* @brief   set indication if to print the passing tests that currently run
*
* @param[in] enable                   - GT_TRUE  -  the print of all passed tests
*                                      GT_FALSE - disable the print of all passed tests
*                                       GT_OK
*/
GT_STATUS utfTestPrintPassOkSummary(
    IN GT_BOOL  enable
);

/**
* @internal utfMemoryLeakPauseSet function
* @endinternal
*
* @brief   This function changed memory leak test control state.
*
* @param[in] onPause                  - GT_TRUE temporary disabled memLeak tests,
*                                      GT_FALSE to continue.
*
* @retval GT_OK                    - on success (no memory leak).
* @retval GT_BAD_VALUE             - on illegal input parameters value
* @retval GT_FAIL                  - on failure (memory leak detected).
*
* @note none
*
*/
GT_STATUS utfMemoryLeakPauseSet
(
    GT_BOOL onPause
);

/**
* @internal utfMemoryLeakResetCounter function
* @endinternal
*
* @brief   This function reset memory leak counter.
*
* @retval GT_OK                    - on success.
*
* @note none
*
*/
GT_STATUS utfMemoryLeakResetCounter
(
    GT_VOID
);

/* typedef for Debug Post Test Exit Finction  */
/* used for looking for the test finihed with */
/* wrong PP configuration                     */
typedef GT_VOID (*PRV_UTF_DEBUG_POST_TEST_EXIT_FUNC_PTR)
(
    IN const GT_CHAR*          suitNamePtr,
    IN const GT_CHAR*          testNamePtr
);

/**
* @internal utfDebugPostTestExitBind function
* @endinternal
*
* @brief   This function binds Debug Post Test Exit Finction.
*
* @param[in] exitFuncPtr              - (pointer to) Debug Post Test Exit Finction.
*                                      NULL value means unbind.
*                                       none.
*/
GT_VOID utfDebugPostTestExitBind
(
    IN PRV_UTF_DEBUG_POST_TEST_EXIT_FUNC_PTR exitFuncPtr
);

/**
* @internal utfErrorCountGet function
* @endinternal
*
* @brief   This function return the number of errors from last time called.
*/
GT_U32 utfErrorCountGet(void);

/**
* @internal utfLogStatusGet function
* @endinternal
*
* @brief   This function checks log status readiness before new API call.
*
* @retval GT_OK                    - log is ready to run
* @retval GT_BAD_STATE             - log is state machine in bad state to run log
*/
GT_STATUS utfLogStatusGet
(
    GT_VOID
);

/*************************************************************************************
* utfTestTaskIdAddrGet
*
* DESCRIPTION:
*     Return the address of a variable storing a current test's task (thread) id.
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS :
*       the address of a task (thread) id of a  current test.
*
* COMMENTS:
*       None.
*
************************************************************************************/
GT_U32* utfTestTaskIdAddrGet
(
    GT_VOID
);

/**
* @internal prvUtfRestoreOrigTestedPorts function
* @endinternal
*
* @brief   restore the ports that set by the 'engine' as preparation to the test
*           (without the test changes)
*
*/
void prvUtfRestoreOrigTestedPorts(
    void
);

/**
* @internal utfSleepAfterTestSet function
* @endinternal
*
* @brief   This function sets/gets delay in msec after each test.
*         0 means no delay
*/
GT_VOID utfSleepAfterTestSet(GT_U32 delayMsec);
GT_U32  utfSleepAfterTestGet(GT_VOID);

/**
* @internal utfIntprTableCheckThreshSet function
* @endinternal
*
* @brief   This function sets/gets threshhold for event table check.
*         0 means no check is performed
*         After each test event counters are tested.
*         if there is at least one event greater of equal to thresold,
*         the table is printed .
*/
GT_VOID utfIntprTableCheckThreshSet(GT_U32 THRESHOLD);
GT_U32  utfIntprTableCheckThreshGet(GT_VOID);

/**
* @internal utfContinueFlagGet function
* @endinternal
*
* @brief   This function returns continue flag.
*
* @retval GT_TRUE                  -  Test may continue its flow.
* @retval GT_FALSE                 -  Test must be interrupted.
*/
GT_BOOL utfContinueFlagGet
(
);

/**
* @internal utfGenEventCounterGet function
* @endinternal
*
* @brief   Wrapper for appDemoGenEventCounterGet function
*
* @param[in] devNum                   - device number
* @param[in] uniEvent                 - unified event
* @param[in] clearOnRead              - do we 'clear' the counter after 'read' it
*                                      GT_TRUE - set counter to 0 after get it's value
*                                      GT_FALSE - don't update the counter (only read it)
*
* @param[out] counterPtr               - (pointer to)the counter (the number of times that specific event happened)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum or uniEvent.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - the counters DB not initialized for the device.
*
* @note none
*
*/
GT_STATUS utfGenEventCounterGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN  GT_BOOL                  clearOnRead,
    OUT GT_U32                  *counterPtr
);

/**
* @internal prvUtfCtxAlloc function
* @endinternal
*
* @brief   Allocates and fill with zeroes memory for utfCtxPtr pointer.
*
* @retval GT_OK                    -  Running of tests was successful.
* @retval GT_OUT_OF_CPU_MEM        -  Memory allocation problems.
*/
GT_STATUS prvUtfCtxAlloc
(
    GT_VOID
);

/**
* @internal prvUtfCtxDealloc function
* @endinternal
*
* @brief   Deallocates memory for utfCtxPtr pointer.
*/
GT_VOID prvUtfCtxDealloc
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __utfMainh */

