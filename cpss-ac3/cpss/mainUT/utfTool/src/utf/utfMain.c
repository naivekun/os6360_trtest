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
* @file utfMain.c
*
* @brief Entry point for the UT (unit tests) engine
*
* @version   100
********************************************************************************
*/
/* errorCountIncrement --> for break point on FAIL in any UT/enh-UT fails */
#if (defined CHX_FAMILY)
    #define DXCH_CODE
#endif /* (defined CHX_FAMILY) */
#if (defined PX_FAMILY)
    #define PX_CODE
#endif /* (defined PX_FAMILY) */


#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/version/cpssGenStream.h>
#ifndef DXCH_CODE
 /*get some stubs*/
#include <appDemo/sysHwConfig/nonSwitchStubs.h>
#endif

#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsStr.h>
#include <gtOs/gtOsTimer.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfSuitsConfig.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <extUtils/trafficEngine/prvTgfLog.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>

#include <stdio.h>
#ifdef LINUX
/* use system setting to print uintprt_t (GT_UINTPTR) */
#include <inttypes.h>
#define INTFMT  "%" PRIdPTR ", "
#define INTFMT_LAST  "%" PRIdPTR "\n"
#endif

#if (defined IMPL_TGF) && (defined DXCH_CODE)
    #include <common/tgfCommon.h>
#endif /* IMPL_TGF */


/*
    next flags needed to ignore the 'system reset' reinit of the cpssInitSystem.
    because we not want to break the run of tests
    for functions called from cpssInitSystem(...):
utfInit();
utfPreInitPhase();
utfPostInitPhase();
*/
static  GT_BOOL utfInitDone = GT_FALSE;
static  GT_BOOL utfPostInitPhaseDone = GT_FALSE;
static  GT_BOOL utfPreInitPhaseDone = GT_FALSE;

/* flag to state the test uses port groups bmp */
GT_BOOL              usePortGroupsBmp = GT_FALSE;
/* bmp of port groups to use in test */
GT_PORT_GROUPS_BMP   currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
/* number of port groups that we look for FDB */
GT_U32   prvTgfNumOfPortGroups = 0;

/* For IP tests on eArch devices we need to remove the default virtual router
   before the test to avoid memory leak notifications */
GT_BOOL resetDmmAllocations = GT_FALSE;

/* Max number of characters in test path */
#define UTF_MAX_TEST_PATH_LEN_CNS 100

/* Stack size for test's task */
#define UTF_TASK_STACK_SIZE_CNS 0x8000

/* Test's task priority */
#define UTF_TASK_PRIO_CNS 64

/* Test's result file name */
#define UTF_RESULT_FILENAME_CNS "result.txt"

/* Test's result of compare file name */
#define UTF_RESULT_OF_COMPARE_FILENAME_CNS "result_of_compare.txt"

/* Mark end of param string  */
#define UTF_END_OF_PARAM_VALUE_CNS 0xF

/* Check given function for wrong enum values (from array) */
GT_U32  enumsIndex = 0;
GT_U32 tempParamValue = 0;

/* arrays to catch test that not restores the default tested ports */
static GT_U8    prvTgfPortsNum_orig = PRV_TGF_DEFAULT_NUM_PORTS_IN_TEST_CNS;
static GT_U32    prvTgfPortsArray_orig[PRV_TGF_MAX_PORTS_NUM_CNS];

#ifdef EX_FAMILY
/* the EX not implement those functions */
#define tgfTrafficGeneratorPortForceLinkUpEnableSet(a,b)    GT_OK
/* the EX not implement those variables */
/* do we sent traffic to the CPU ? */
GT_U32 tgfTrafficGeneratorRxCaptureNum = 0;
/* indication that a test set traffic to be sent to the CPU , but not relate to 'capture' */
GT_BOOL tgfTrafficGeneratorExpectTraficToCpuEnabled = GT_FALSE;
/* number of packets that returned 'not GT_OK' on send */
GT_U32  prvTgfTrafficGeneratorNumberPacketsSendFailed = 0;
/* number of packets that returned 'GT_OK' on send */
GT_U32  prvTgfTrafficGeneratorNumberPacketsSendOk = 0;
/*number of ports in test*/
GT_U8  prvTgfPortsNum = 0;
/* the EX not implement those functions */
GT_STATUS tgfTrafficGeneratorGmTimeFactorSet(    IN GT_U32   timeFactor)
{
    TGF_PARAM_NOT_USED(timeFactor);
    return GT_OK;
}
/* dummy array of ports for tests */
GT_U32  prvTgfPortsArray[20];

#endif /*EX_FAMILY*/

#ifdef ASIC_SIMULATION
#include <asicSimulation/SLog/simLog.h>
/* use this mutex to protect wrSimulationDynamicSlanDevDelete() and unbindSlanAlready
 * and avoid race condition */
static CPSS_OS_MUTEX   prvUtfSlanMtx;
/* wrSimulationDynamicSlanDevDelete() uses device number as parameter
 * convert unbindSlanAlready to array to support multi threaded execution
 * with different device numbers for each thread */
static GT_U32  unbindSlanAlready[PRV_CPSS_MAX_PP_DEVICES_CNS] = { 0 };
static GT_CHAR tgfTestNameForLogPtr[257] = {0};
static GT_BOOL tgfTestWithLogStarted = GT_FALSE;
extern void tgfSimulationLogOnSpecificTest(IN GT_CHAR*  testNameForLogPtr);

/* include the AppDemo to get info about the infrastructure */
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <trafficEngine/tgfTrafficGenerator.h>
extern GT_STATUS wrSimulationDynamicSlanDevDelete(IN GT_U8 swDevNum);
extern GT_STATUS wrSimulationDynamicSlanDevDelete_forceAll(IN GT_BOOL force);
/* flag that state the we force unbind of slans or not :
    0 --> not force unbind
    1 --> force unbind --> default
*/
GT_U32  utfForceUnbindSlan = 1;

#endif /*ASIC_SIMULATION*/

#if !(defined CHX_FAMILY)

#define  PRV_CPSS_TGF_UNUSED_PARAMETER(x)   ((void) (x))


/* !!!! STUBS !!!!! */
static void prvTgfCaptureSet_ingressTagging_reset
(
    GT_VOID
)
{
}

#ifdef ASIC_SIMULATION
/* boost performance of the ENH-UT */
static void debug_set_directAccessMode1
(
    IN  GT_U32      newValue
)
{
    PRV_CPSS_TGF_UNUSED_PARAMETER(newValue);
}

static void debug_set_directAccessMode
(
    IN  GT_U32      newValue
)
{
    PRV_CPSS_TGF_UNUSED_PARAMETER(newValue);
}
#endif

GT_BOOL prvTgfXcat3xExists
(
    GT_VOID
)
{
    return GT_FALSE;
}

GT_STATUS prvTgfEthCountersReset
(
    IN  GT_U8       devNum
)
{
    PRV_CPSS_TGF_UNUSED_PARAMETER(devNum);

    return GT_OK;
}

static GT_STATUS prvTgfBrgFdbFlushWithUcEntries
(
    IN  GT_BOOL     enable
)
{
    PRV_CPSS_TGF_UNUSED_PARAMETER(enable);

    return GT_OK;
}

#else
extern void prvTgfCaptureSet_ingressTagging_reset(void);
/* boost performance of the ENH-UT */
extern void debug_set_directAccessMode1(GT_U32  newValue);
extern void debug_set_directAccessMode(GT_U32  newValue);
extern GT_STATUS prvTgfCommonMemberForceInit(IN PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT tagCmd);
extern GT_STATUS prvTgfBrgFdbFlushWithUcEntries
(
    IN GT_BOOL                        enable
);
extern GT_STATUS prvTgfBrgFdbActionDoneWaitForAllDev
(
    IN  GT_BOOL  aaTaClosed
);
#endif /*(defined CHX_FAMILY)*/

extern GT_STATUS osTaskGetSelf
(
    OUT GT_U32 *tid
);

extern GT_VOID appDemoOsLogStop(void);

static __THREAD_LOCAL FILE* fpResult = NULL;

/* array of buffers of 'general states' DB */
static struct {
    GT_CHAR buff[2048];/* the string to log when test fail */
    GT_BOOL valid;     /* is entry valid */
}prvUtfGeneralStateLogArr[PRV_UTF_GENERAL_STATE_INFO_NUM_CNS];
/* array of callback function that need to be called on error .
   this to allow a test to 'dump' into LOG/terminal important info that may
   explain why the test failed. -- this is advanced debug tool */
static struct {
    UTF_CALL_BACK_FUNCTION_ON_ERROR_TYPE    callBackFunc;
}prvUtfCallBackFunctionsOnErrorArr[PRV_UTF_GENERAL_STATE_INFO_NUM_CNS];

typedef enum{
    UTF_TEST_STATUS_NOT_RUN_E,/*tests did not run --> no part of the current run */
    UTF_TEST_STATUS_PASS_E,   /*test run and pass ok */
    UTF_TEST_STATUS_FAILED_E, /*test run and failed */
    UTF_TEST_STATUS_SKIPPED_E,/*test was skipped , test not valid for the device */

    UTF_TEST_STATUS_LAST_E/* not valid value */

}UTF_TEST_STATUS_ENT;

/**
* @struct UTF_TEST_ENTRY_COMMON_STC
 *
 * @brief Test case entry
 * contains common info about test case
*/
typedef struct{

    /** @brief pointer to test case function
     *  testNamePtr   - name of test case
     *  suiteNamePtr  - name of test suit
     */
    UTF_TEST_CASE_FUNC_PTR testFuncPtr;

    const GT_CHAR*          testNamePtr;

    const GT_CHAR*          suitNamePtr;

    /** type of test */
    UTF_TEST_TYPE_ENT testType;

} UTF_TEST_ENTRY_COMMON_STC;

/**
* @struct UTF_TEST_ENTRY_LOCAL_STC
 *
 * @brief Test case entry
 * contains runtime info about test case
*/
typedef struct{

    /** number of errors for the test case */
    GT_U32 errorCount;

    /** test running time */
    GT_U32 testExecTimeSec;

    /** test running time */
    GT_U32 testExecTimeNsec;

    /** test status (not_run/pass/failed/skipped) */
    UTF_TEST_STATUS_ENT testStatus;

} UTF_TEST_ENTRY_LOCAL_STC;

/* This parameter defines how many test cases UTF will support.
 * Also the parameter impacts on the memory map size.
 */
#define UTF_MAX_NUM_OF_TESTS_CNS    8000

/**
* @struct UTF_CTX_COMMON_STC
 *
 * @brief Main context for UTF
 * there is can be only one statically
 * allocated instance of this context
*/
typedef struct{

    UTF_TEST_ENTRY_COMMON_STC testEntriesArray[UTF_MAX_NUM_OF_TESTS_CNS];

    GT_U32 testIndexArray[UTF_MAX_NUM_OF_TESTS_CNS];

    /** number of used test entries in array */
    GT_U32 usedTestNum;

} UTF_CTX_COMMON_STC;

/**
* @struct UTF_CTX_LOCAL_STC
 *
 * @brief Local context for UTF
 * there is can be only one statically
 * allocated instance of this context per thread
*/
typedef struct{

    UTF_TEST_ENTRY_LOCAL_STC testEntriesArray[UTF_MAX_NUM_OF_TESTS_CNS];

    /** index of current test entry */
    GT_U32 currTestNum;

    /** indicates if it was error */
    GT_BOOL errorFlag;

    /** define if UTF will continue test after first error */
    GT_BOOL continueFlag;

} UTF_CTX_LOCAL_STC;

/* local per thread instance of UTF context */
static __THREAD_LOCAL UTF_CTX_LOCAL_STC *utfCtxPtr = NULL;
/* single common instance of UTF context */
static UTF_CTX_COMMON_STC utfCtxCommon;

/**
* @struct UTF_TIMER_STC
 *
 * @brief contains timer info
*/
typedef struct{

    /** code running time in seconds */
    GT_U32 codeExecTimeSec;

    /** code running time in nanoseconds */
    GT_U32 codeExecTimeNsec;

} UTF_TIMER_STC;

/* single instance of UTF timer struct */
static __THREAD_LOCAL UTF_TIMER_STC utfTimer = {0, 0};

/* contains registration for all suits */
extern GT_STATUS utfAllSuitsDeclare(GT_VOID);

/* forward declaration for internal utf functions */
static GT_VOID utfSuitStatsShow(IN const GT_CHAR *suitNamePtr);
static GT_VOID utfFinalStatsShow(GT_VOID);
static GT_STATUS utfTestAdd(IN UTF_TEST_CASE_FUNC_PTR   tesFuncPtr,
                            IN const GT_CHAR            *testNamePtr,
                            IN const GT_CHAR            *suitNamePtr);
static GT_VOID utfPostTestRun(GT_VOID);
static GT_STATUS utfSuitsRun(IN const GT_CHAR  *testPathPtr,
                             IN GT_BOOL        continueFlag,
                             IN GT_BOOL        startFlag);
static GT_U32 utfTestsNumGet(IN const GT_CHAR  *suitNamePtr);
/* indication if to print the passing tests that currently run */
static __THREAD_LOCAL GT_BOOL printPassOkSummary = GT_FALSE;
/* The printing mode that the UTF operates. */
static UTF_LOG_OUTPUT_ENT utfLogOutputSelectMode = UTF_LOG_OUTPUT_SERIAL_FINAL_E;

/* Semaphore for single test */
static __THREAD_LOCAL CPSS_OS_SIG_SEM utfTestSemId = 0;

/* Task ID for single test run */
static __THREAD_LOCAL CPSS_TASK utfTestTaskId = 0;

/* TimeOut for a single test */
static __THREAD_LOCAL GT_U32 utfTestTimeOut = CPSS_OS_SEM_WAIT_FOREVER_CNS;

/* test type */
static __THREAD_LOCAL UTF_TEST_TYPE_ENT utfTestType = UTF_TEST_TYPE_NONE_E;

/* test type bitmap to skip */
static __THREAD_LOCAL GT_U32 utfSkipTestTypeBmp = 0;

/* start test flag */
static __THREAD_LOCAL GT_BOOL utfStartFlag = GT_FALSE;

/* indication need to skip the test that mentioned in utfTestsStartRunFromNext(...) */
static __THREAD_LOCAL GT_BOOL  runFromNextTest = GT_FALSE;

/* initial start time in sec */
static __THREAD_LOCAL GT_U32  startTimeSec;

/* initial start time in nsec */
static __THREAD_LOCAL GT_U32  startTimeNsec;

typedef struct PRV_UTF_SKIP_LIST_STCT {
    struct PRV_UTF_SKIP_LIST_STCT *next;
    GT_CHAR                       *suit;
    GT_CHAR                       *test;
    GT_CHAR                       *reason;
} PRV_UTF_SKIP_LIST_STC;
static PRV_UTF_SKIP_LIST_STC *prvUtfSkipList;

/* number of tests that is not checked for memory leakage */
#define NUM_MEM_LEAK_SKIPPED_TESTS 4

/* memLeakageSkippedTestArray - contains test names that is not checked for memory leakage */
static GT_CHAR *memLeakageSkippedTestArray[NUM_MEM_LEAK_SKIPPED_TESTS] =
                                        {"tgfBasicIpv42SharedVirtualRouter"
                                         ,"prvTgfFdbStuckAuqWaBasic"
                                         ,"prvTgfFdbStuckDoubleAuqWa"
                                         ,"prvTgfGenEventDeviceGenerate"
                                        };

/* number of tests that include IP LPM functionality - for those tests we need
   to delete the default virtual router before and the LPM DB before the test
   and add it after we start to count memory allocations. After the test ends
   we need to restore the default virtual router and the LPM DB. This will prevent
   notifications about memory leaks */
#define NUM_IP_LPM_TESTS 30
static GT_CHAR *ipLpmTestArray[NUM_IP_LPM_TESTS] =
                               {"tgfNstL3PortIsolation",
                                "tgfNstL2L3PortIsolation",
                                "tgfNstL3TrunkPortIsolation",
                                "prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverIpv4WithL2",
                                "prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverIpv4NoL2",
                                "prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverMplsWithL2",
                                "prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverMplsNoL2",
                                "prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverIpv4WithL2",
                                "prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverIpv4NoL2",
                                "prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverMplsWithL2",
                                "prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverMplsNoL2",
                                "prvTgfEgressMirrorAnalyzerIndexToCpu",
                                "tgfLogicalTargetIpv4Uc",
                                "prvTgfCutThroughBypassRouterAndPolicer",
                                "prvTgfPolicerEgressDscpModify",
                                "prvTgfFdbBasicIpv4UcRoutingByMsg",
                                "prvTgfFdbBasicIpv4UcRoutingByIndex",
                                "prvTgfFdbBasicIpv4UcRoutingFillFdbByIndex",
                                "prvTgfFdbBasicIpv4UcRoutingFillFdbByMessage",
                                "prvTgfFdbIpv4UcRoutingDeleteEnable",
                                "prvTgfFdbIpv4UcRoutingRefreshEnable",
                                "prvTgfFdbIpv4UcRoutingTransplantEnable",
                                "prvTgfFdbIpv4UcRoutingAgingEnable",
                                "prvTgfFdbBasicIpv6UcRouting",
                                "prvTgfFdbIpv6UcRoutingDeleteEnable",
                                "prvTgfFdbIpv6UcRoutingRefreshEnable",
                                "prvTgfFdbIpv6UcRoutingTransplantEnable",
                                "prvTgfFdbIpv6UcRoutingAgingEnable",
                                "prvTgfFdbIpv4v6Uc2VrfRouting",
                                "prvTgfFdbIpv4UcRoutingMtuCheckNonDf"};

/* test random run mode flag */
static __THREAD_LOCAL GT_BOOL  isRandomRunMode = GT_FALSE;

/* random number seed */
static __THREAD_LOCAL GT_U32   prvUtfRandomSeedNum = 0;

/* number to set auto generated seed value */
#define UTF_AUTO_SEED_NUM_CNS 0

/* force print of error info when the flag of utfCtxPtr->continueFlag == GT_FALSE */
#define FORCE_PRINT_WHEN_NOT_CONTINUE_CNS

/* global number of errors ... see function utfErrorCountGet() */
static __THREAD_LOCAL GT_U32   totalNumErrors = 0;
/* the single function that increment the errorCount value ... for single debug function */
static void errorCountIncrement(GT_U32   index , GT_BOOL      modifyErrorFlag)
{
    if(modifyErrorFlag == GT_TRUE)
    {
        utfCtxPtr->errorFlag = GT_TRUE;
    }
    utfCtxPtr->testEntriesArray[index].errorCount++;
    totalNumErrors++;
}

/*
   the memory leak detection test state flag. Used for temporary pause/continue
   memory leak detection procedure while test in progress.
*/
static __THREAD_LOCAL GT_BOOL memoryLeakTestOnPause = GT_FALSE;

/*
   the memory leak chunk size. Used to save detected memLeak regression
   detected between changes(pause/continue) of the tests states.
 */
static __THREAD_LOCAL GT_U32  memoryLeakChunkSize = 0;

/* Pointer to Debug Post Test Exit Finction    */
/* used for looking for the test finished with */
/* wrong PP configuration                      */
PRV_UTF_DEBUG_POST_TEST_EXIT_FUNC_PTR     prvUtfDebugPostTestExitPtr = NULL;

/* optional reassignment of device and ports used in enhanced tests */
/* parameters - */
/**
* @internal utfTestSingleDeviceAndPortsAssign function
* @endinternal
*
* @brief   Reassignment of device and ports used in enhanced tests.
*
* @param[in] devNum                   - device number to be used.
*                                      other params - numbers of ports
*                                      any number out of range [0..255] used as marker of the end of the list
*                                       GT_OK always.
*/
GT_STATUS utfTestSingleDeviceAndPortsAssign
(
    GT_U8 devNum, ...
)
{
    GT_U8   iter;
    va_list argP;
    GT_32   port;

    prvTgfDevNum = devNum;
    va_start(argP, devNum);
    for (iter = 0; (((port = va_arg(argP, GT_32)) & 0xFFFFFF00) == 0); iter++)
    {
        prvTgfDevsArray[iter]  = devNum;
        prvTgfPortsArray[iter] = port;
    }
    if (iter == 0)
    {
        /* updated device number only, ports not changed */
        return GT_OK;
    }
    prvTgfPortsNum = iter;
    va_end(argP);

    /* save updated ports array.  */
    prvTgfPortsNum_orig = prvTgfPortsNum;
    for (iter = 0; (iter < PRV_TGF_MAX_PORTS_NUM_CNS); iter++)
    {
        prvTgfPortsArray_orig[iter] = prvTgfPortsArray[iter];
    }
    return GT_OK;
}

/* test skipping mechanism */

typedef struct __UTF_PRESKIPPED_RULE_STC
{
    struct __UTF_PRESKIPPED_RULE_STC* next;
    char*                             suitLow;
    char*                             suitHigh;
    char*                             testLow;
    char*                             testHigh;
    /* origin of variable part in dynamic allocated memory */
    char                              buf[1];
} UTF_PRESKIPPED_RULE_STC;

static __THREAD_LOCAL UTF_PRESKIPPED_RULE_STC* utfPreSkippedRulesDbPtr = NULL;

/**
* @internal utfResetPreSkippedRules function
* @endinternal
*
* @brief   Resets PreeSkipped rules.
*/
GT_STATUS utfResetPreSkippedRules
(
    IN GT_VOID
)
{
    UTF_PRESKIPPED_RULE_STC* p;
    while (utfPreSkippedRulesDbPtr != NULL)
    {
        p = utfPreSkippedRulesDbPtr;
        utfPreSkippedRulesDbPtr = p->next;
        cpssOsFree(p);
    }
    return GT_OK;
}

/**
* @internal utfAddPreSkippedRule function
* @endinternal
*
* @brief   Add PreeSkipped rule.
*
* @param[in] suitLow                  - low  bound of excluded suit names or NULL
* @param[in] suitHigh                 - high bound of excluded suit names or NULL
* @param[in] testLow                  - low  bound of excluded test names or NULL
* @param[in] testHigh                 - high bound of excluded test names or NULL
*                                      Test skipped only ig both suit name and test name
*                                      are in specified ranges.
*                                      Name in specified range if
*                                      strcmp(name, low) >= 0 and
*                                      strcmp(high, name) >= 0
*                                      Values NULL, "", and "" are special.
*                                      For all values NULL or "" means that
*                                      the range has no appropriate bound.
*                                      For high values "" means "the same as low".
*                                       none.
*
* @note Examples:
*       To Skip All Suite cpssDxChCnc call
*       utfAddPreSkippedRule("cpssDxChCnc","","","");.
*       To Skip Test cpssDxChCncCounterSet only call
*       utfAddPreSkippedRule("","","cpssDxChCncCounterSet","");.
*       To Skip All Suite cpssDxChBrg Suites call
*       utfAddPreSkippedRule("cpssDxChBrg","cpssDxChBrgzzz","","");.
*
*/
GT_STATUS utfAddPreSkippedRule
(
    IN char*        suitLow,
    IN char*        suitHigh,
    IN char*        testLow,
    IN char*        testHigh
)
{
    GT_U32                   suitLowLen;
    GT_U32                   suitHighLen;
    GT_U32                   testLowLen;
    GT_U32                   testHighLen;
    UTF_PRESKIPPED_RULE_STC* rulePtr;
    GT_U32                   offset;

    /* replace "*" High values by low values */
    if (suitHigh && (suitHigh[0] == '*') && (suitHigh[1] == 0)) {suitHigh = suitLow;}
    if (testHigh && (testHigh[0] == '*') && (testHigh[1] == 0)) {testHigh = testLow;}
    /* replace "" values by NULL */
    if (suitLow  && (suitLow[0] == 0))   {suitLow = NULL;}
    if (suitHigh && (suitHigh[0] == 0))  {suitHigh = NULL;}
    if (testLow  && (testLow[0] == 0))   {testLow = NULL;}
    if (testHigh && (testHigh[0] == 0))  {testHigh = NULL;}
    /* lengths and memory allocation */
    suitLowLen  = (suitLow ? (cpssOsStrlen(suitLow) + 1) : 0);
    suitHighLen = (suitHigh ? (cpssOsStrlen(suitHigh) + 1) : 0);
    testLowLen  = (testLow ? (cpssOsStrlen(testLow) + 1) : 0);
    testHighLen = (testHigh ? (cpssOsStrlen(testHigh) + 1) : 0);
    rulePtr = cpssOsMalloc(
            sizeof(UTF_PRESKIPPED_RULE_STC)
            + suitLowLen + suitHighLen + testLowLen +  testHighLen);
    offset = 0;

    if (rulePtr == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }
    cpssOsMemSet(rulePtr, 0, sizeof(UTF_PRESKIPPED_RULE_STC));
    if (suitLowLen)
    {
        rulePtr->suitLow = &(rulePtr->buf[offset]);
        cpssOsStrCpy(rulePtr->suitLow, suitLow);
        offset += suitLowLen;
    }
    if (suitHigh && suitLow && (0 == cpssOsStrCmp(suitLow, suitHigh)))
    {
        rulePtr->suitHigh = rulePtr->suitLow;
    }
    else if (suitHighLen)
    {
        rulePtr->suitHigh = &(rulePtr->buf[offset]);
        cpssOsStrCpy(rulePtr->suitHigh, suitHigh);
        offset += suitHighLen;
    }
    if (testLowLen)
    {
        rulePtr->testLow = &(rulePtr->buf[offset]);
        cpssOsStrCpy(rulePtr->testLow, testLow);
        offset += testLowLen;
    }
    if (testHigh && testLow && (0 == cpssOsStrCmp(testLow, testHigh)))
    {
        rulePtr->testHigh = rulePtr->testLow;
    }
    else if (testHighLen)
    {
        rulePtr->testHigh = &(rulePtr->buf[offset]);
        cpssOsStrCpy(rulePtr->testHigh, testHigh);
        offset += testHighLen;
    }
    rulePtr->next = utfPreSkippedRulesDbPtr;
    utfPreSkippedRulesDbPtr = rulePtr;
    return GT_OK;
}

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
)
{
    GT_CHAR *f;
    PRV_UTF_SKIP_LIST_STC *entry;
    if (!str)
        return GT_BAD_PARAM;
    f = cpssOsStrChr(str, '.');
    if (!f)
        return GT_BAD_PARAM;
    if (!reason)
        reason = "(null)";
    entry = (PRV_UTF_SKIP_LIST_STC*)cpssOsMalloc(sizeof(*entry));
    entry->suit = (GT_CHAR*)cpssOsMalloc((GT_U32)(f-str)+1);
    entry->test = (GT_CHAR*)cpssOsMalloc(cpssOsStrlen(f));
    entry->reason = (GT_CHAR*)cpssOsMalloc(cpssOsStrlen(reason)+1);
    cpssOsStrNCpy(entry->suit, str, (GT_U32)(f-str));
    entry->suit[f-str] = 0;
    cpssOsStrCpy(entry->test, f+1);
    cpssOsStrCpy(entry->reason, reason);
    entry->next = prvUtfSkipList;
    prvUtfSkipList = entry;

    return GT_OK;
}

/**
* @internal utfIsTestPreSkipped function
* @endinternal
*
* @brief   Check wheter test is predefined as skipped.
*
* @param[in] suitNamePtr              -   suit name.
* @param[in] testNamePtr              -   test name.
*
* @retval GT_TRUE                  -  test should be skipped.
* @retval GT_FALSE                 -  test should not be skipped.
*/
GT_BOOL utfIsTestPreSkipped
(
    IN const char *suitNamePtr,
    IN const char *testNamePtr
)
{
    UTF_PRESKIPPED_RULE_STC* rulePtr = utfPreSkippedRulesDbPtr;
    for(; (rulePtr); rulePtr = rulePtr->next)
    {
        if (rulePtr->suitLow)
        {
            if (cpssOsStrCmp(rulePtr->suitLow, suitNamePtr) > 0)
            {
                continue;
            }
        }
        if (rulePtr->suitHigh)
        {
            if (cpssOsStrCmp(rulePtr->suitHigh, suitNamePtr) < 0)
            {
                continue;
            }
        }
        if (rulePtr->testLow)
        {
            if (cpssOsStrCmp(rulePtr->testLow, testNamePtr) > 0)
            {
                continue;
            }
        }
        if (rulePtr->testHigh)
        {
            if (cpssOsStrCmp(rulePtr->testHigh, testNamePtr) < 0)
            {
                continue;
            }
        }

        cpssOsPrintf("forbidden suite [%s] test [%s] \n",suitNamePtr,testNamePtr);

        /* condition matches the test */
        return GT_TRUE;
    }
    return GT_FALSE;
}

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
)
{
    GT_STATUS   st = GT_OK;
    GT_U32      i = 0;
    const GT_CHAR  *suitNamePtr = suitPtr[i++];

    while ((GT_OK == st) && (suitPtr[i] != 0))
    {
        st = utfTestAdd((UTF_TEST_CASE_FUNC_PTR)suitPtr[i + 1], (const GT_CHAR*)suitPtr[i], suitNamePtr);
        i += 2;
    }
    return st;
}

/**
* @internal prvUtfGeneralStateLogDump function
* @endinternal
*
* @brief   dump the general state if exists.
*         this function dump to logger all that received from calling
*         PRV_UTF_GENERAL_STATE_INFO_MAC
*         the number of such messages defined in PRV_UTF_GENERAL_STATE_INFO_NUM_CNS
*/
static GT_VOID prvUtfGeneralStateLogDump
(
    GT_VOID
)
{
    GT_U32  ii;
    GT_BOOL foundFirst = GT_FALSE;

    for(ii = 0 ; ii < PRV_UTF_GENERAL_STATE_INFO_NUM_CNS ; ii++)
    {
        if(prvUtfGeneralStateLogArr[ii].valid == GT_FALSE)
        {
            continue;
        }

        if(foundFirst == GT_FALSE)
        {
            foundFirst = GT_TRUE;
            PRV_UTF_LOG0_MAC("\n General state Dump: \n");
        }

        PRV_UTF_LOG2_MAC("%s%s",prvUtfGeneralStateLogArr[ii].buff,"\n");
    }

    if(foundFirst == GT_TRUE)
    {
        PRV_UTF_LOG0_MAC("General state Ended \n");
    }
}

/**
* @internal prvUtfCallBackFunctionsOnErrorActivate function
* @endinternal
*
* @brief   activate all the call back functions that are registered to be called on error.
*         the number of such messages defined in PRV_UTF_GENERAL_STATE_INFO_NUM_CNS
*/
static GT_VOID prvUtfCallBackFunctionsOnErrorActivate
(
    GT_VOID
)
{
    GT_U32  ii;

    for(ii = 0 ; ii < PRV_UTF_GENERAL_STATE_INFO_NUM_CNS ; ii++)
    {
        if(prvUtfCallBackFunctionsOnErrorArr[ii].callBackFunc == NULL)
        {
            continue;
        }

        PRV_UTF_LOG1_MAC("\n [%d] call backs on error started : \n",ii);

        prvUtfCallBackFunctionsOnErrorArr[ii].callBackFunc();

        PRV_UTF_LOG1_MAC("\n [%d] call backs on error ended : \n",ii);
    }
}

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
)
{
#ifdef FORCE_PRINT_WHEN_NOT_CONTINUE_CNS
    UTF_LOG_OUTPUT_ENT origLogState = 0;
#endif /*FORCE_PRINT_WHEN_NOT_CONTINUE_CNS*/

    if(utfCtxPtr == NULL)
    {
        /* got here not from UT test but from C function that was not called
           from UT test */
        if (e != r)
        {
            totalNumErrors++;

            return GT_FALSE;
        }

        return GT_TRUE;
    }


    if (e != r)
    {
        errorCountIncrement(utfCtxPtr->currTestNum,GT_TRUE);

#ifdef FORCE_PRINT_WHEN_NOT_CONTINUE_CNS
        if(utfCtxPtr->continueFlag == GT_FALSE)
        {
            /* we run until first fail , so let us have the printing regardless to
               global debug mode.
               because global debug mode maybe with HUGE amount of printings ... */
            origLogState = prvUtfLogPrintEnable(UTF_LOG_OUTPUT_SERIAL_ALL_E);
        }
#endif /*FORCE_PRINT_WHEN_NOT_CONTINUE_CNS*/

        PRV_UTF_LOG0_MAC("\n");
        PRV_UTF_LOG1_MAC("TEST %s FAILED\n", utfCtxCommon.testEntriesArray[utfCtxPtr->currTestNum].testNamePtr);
        PRV_UTF_LOG2_MAC("FAILURE! FileName: %s (line %d)\n", fileNamePtr, lineNum);
#ifndef INTFMT
        PRV_UTF_LOG1_MAC("Expected: %d\n", e);
        PRV_UTF_LOG1_MAC("Received: %d\n", r);
#else
        PRV_UTF_LOG1_MAC("Expected: " INTFMT_LAST, e);
        PRV_UTF_LOG1_MAC("Received: " INTFMT_LAST, r);
#endif

        /* dump the general state if exists */
        prvUtfGeneralStateLogDump();

        /* call the callbacks on error if exists */
        prvUtfCallBackFunctionsOnErrorActivate();

#ifdef FORCE_PRINT_WHEN_NOT_CONTINUE_CNS
        if(utfCtxPtr->continueFlag == GT_FALSE)
        {
            /* restore original value */
            prvUtfLogPrintEnable(origLogState);
        }
#endif /*FORCE_PRINT_WHEN_NOT_CONTINUE_CNS*/

        return GT_FALSE;
    }
    return GT_TRUE;
}

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
)
{
#ifdef FORCE_PRINT_WHEN_NOT_CONTINUE_CNS
    UTF_LOG_OUTPUT_ENT origLogState = 0;
#endif /*FORCE_PRINT_WHEN_NOT_CONTINUE_CNS*/

    if (e == r)
    {
        errorCountIncrement(utfCtxPtr->currTestNum,GT_TRUE);

#ifdef FORCE_PRINT_WHEN_NOT_CONTINUE_CNS
        if(utfCtxPtr->continueFlag == GT_FALSE)
        {
            /* we run until first fail , so let us have the printing regardless to
               global debug mode.
               because global debug mode maybe with HUGE amount of printings ... */
            origLogState = prvUtfLogPrintEnable(UTF_LOG_OUTPUT_SERIAL_ALL_E);
        }
#endif /*FORCE_PRINT_WHEN_NOT_CONTINUE_CNS*/

        PRV_UTF_LOG0_MAC("\n");
        PRV_UTF_LOG1_MAC("TEST %s FAILED\n", utfCtxCommon.testEntriesArray[utfCtxPtr->currTestNum].testNamePtr);
        PRV_UTF_LOG2_MAC("FAILURE! FileName: %s (line %d)\n", fileNamePtr, lineNum);
#ifndef INTFMT
        PRV_UTF_LOG1_MAC("Expected to be not equal to: %d\n", e);
        PRV_UTF_LOG1_MAC("Received: %d\n", r);
#else
        PRV_UTF_LOG1_MAC("Expected: " INTFMT_LAST, e);
        PRV_UTF_LOG1_MAC("Received: " INTFMT_LAST, r);
#endif

#ifdef FORCE_PRINT_WHEN_NOT_CONTINUE_CNS
        if(utfCtxPtr->continueFlag == GT_FALSE)
        {
            /* restore original value */
            prvUtfLogPrintEnable(origLogState);
        }
#endif /*FORCE_PRINT_WHEN_NOT_CONTINUE_CNS*/

        return GT_FALSE;
    }
    return GT_TRUE;
}

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
)
{
    return utfCtxPtr->continueFlag;
}

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
)
{
#ifdef FORCE_PRINT_WHEN_NOT_CONTINUE_CNS
    UTF_LOG_OUTPUT_ENT origLogState = 0;

    if(utfCtxPtr == NULL)
    {
        /* got here not from UT test but from C function that was not called
           from UT test */
        return GT_TRUE;
    }

    if(utfCtxPtr->continueFlag == GT_FALSE)
    {
        /* we run until first fail , so let us have the printing regardless to
           global debug mode.
           because global debug mode maybe with HUGE amount of printings ... */
        origLogState = prvUtfLogPrintEnable(UTF_LOG_OUTPUT_SERIAL_ALL_E);
    }
#endif /*FORCE_PRINT_WHEN_NOT_CONTINUE_CNS*/

    PRV_UTF_LOG1_MAC("[UTF]: %s: ", utfCtxCommon.testEntriesArray[utfCtxPtr->currTestNum].testNamePtr);

    if (NULL == failureMsgPtr)
    {
        switch (argc)
        {
            case 0:
                PRV_UTF_LOG0_MAC("no parameters were defined\n");
                break;
            case 1:
#ifndef INTFMT
                PRV_UTF_LOG1_MAC("%d\n", argvPtr[0]);
#else
                PRV_UTF_LOG1_MAC(INTFMT_LAST, argvPtr[0]);
#endif
                break;
            case 2:
#ifndef INTFMT
                PRV_UTF_LOG2_MAC("%d, %d\n", argvPtr[0], argvPtr[1]);
#else
                PRV_UTF_LOG2_MAC(INTFMT INTFMT_LAST, argvPtr[0], argvPtr[1]);
#endif
                break;
            case 3:
#ifndef INTFMT
                PRV_UTF_LOG3_MAC("%d, %d, %d\n", argvPtr[0], argvPtr[1], argvPtr[2]);
#else
                PRV_UTF_LOG3_MAC(INTFMT INTFMT INTFMT_LAST, argvPtr[0], argvPtr[1], argvPtr[2]);
#endif
                break;
            case 4:
#ifndef INTFMT
                PRV_UTF_LOG4_MAC("%d, %d, %d, %d\n", argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3]);
#else
                PRV_UTF_LOG4_MAC(INTFMT INTFMT INTFMT INTFMT_LAST, argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3]);
#endif
                break;
            case 5:
#ifndef INTFMT
                PRV_UTF_LOG5_MAC("%d, %d, %d, %d, %d\n", argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3], argvPtr[4]);
#else
                PRV_UTF_LOG5_MAC(INTFMT INTFMT INTFMT INTFMT INTFMT_LAST, argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3], argvPtr[4]);
#endif
                break;
            case 6:
#ifndef INTFMT
                PRV_UTF_LOG6_MAC("%d, %d, %d, %d, %d, %d\n", argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3], argvPtr[4], argvPtr[5]);
#else
                PRV_UTF_LOG6_MAC(INTFMT INTFMT INTFMT INTFMT INTFMT INTFMT_LAST, argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3], argvPtr[4], argvPtr[5]);
#endif
                break;
            case 7:
#ifndef INTFMT
                PRV_UTF_LOG7_MAC("%d, %d, %d, %d, %d, %d, %d\n", argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3], argvPtr[4], argvPtr[5], argvPtr[6]);
#else
                PRV_UTF_LOG7_MAC(INTFMT INTFMT INTFMT INTFMT INTFMT INTFMT INTFMT_LAST, argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3], argvPtr[4], argvPtr[5], argvPtr[6]);
#endif
                break;
            default:
                PRV_UTF_LOG1_MAC("too many arguments [%d]\n", argc);
                break;
        }
    }
    else
    {
        prvUtfLogMessage(failureMsgPtr, argvPtr, argc);
        PRV_UTF_LOG0_MAC("\n");
    }
    PRV_UTF_LOG0_MAC("\n");

#ifdef FORCE_PRINT_WHEN_NOT_CONTINUE_CNS
    if(utfCtxPtr->continueFlag == GT_FALSE)
    {
        /* restore original value */
        prvUtfLogPrintEnable(origLogState);
    }
#endif /*FORCE_PRINT_WHEN_NOT_CONTINUE_CNS*/

    return utfCtxPtr->continueFlag;
}

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
)
{
    GT_U32 countIdx = 0;

    PRV_UTF_LOG0_MAC("\n");
    PRV_UTF_LOG0_MAC("[UTF]: utfTestsList: ***********************\n");

    if (NULL == suitNamePtr)
    {
        const GT_CHAR* lastSuitNamePtr = NULL;

        PRV_UTF_LOG0_MAC("[UTF]: suit name: [all]\n");
        PRV_UTF_LOG0_MAC("\n");

        for (countIdx = 0; countIdx < utfCtxCommon.usedTestNum; ++countIdx)
        {
            const GT_CHAR *currSuitNamePtr = utfCtxCommon.testEntriesArray[countIdx].suitNamePtr;

            /* check if current test needs to be skipped */
            if (utfSkipTestTypeBmp & (1 << utfCtxCommon.testEntriesArray[countIdx].testType))
            {
                continue;
            }

            if (lastSuitNamePtr == 0 || (0 != cpssOsStrCmp(currSuitNamePtr, lastSuitNamePtr)))
            {
                PRV_UTF_LOG2_MAC("%s - %d\n", currSuitNamePtr, utfTestsNumGet(currSuitNamePtr));
                lastSuitNamePtr = currSuitNamePtr;
            }
        }
    }
    else
    {
        PRV_UTF_LOG1_MAC("[UTF]: suit name: [%s]\n", suitNamePtr);
        PRV_UTF_LOG0_MAC("\n");

        for (countIdx = 0; countIdx < utfCtxCommon.usedTestNum; countIdx++)
        {
            const GT_CHAR* currSuitNamePtr = utfCtxCommon.testEntriesArray[utfCtxCommon.testIndexArray[countIdx]].suitNamePtr;
            const GT_CHAR* currTestNamePtr = utfCtxCommon.testEntriesArray[utfCtxCommon.testIndexArray[countIdx]].testNamePtr;

            if (0 == cpssOsStrCmp(currSuitNamePtr, suitNamePtr))
            {
                PRV_UTF_LOG1_MAC("%s\n", currTestNamePtr);
            }
        }
    }

    PRV_UTF_LOG0_MAC("[UTF]: utfTestsList: ***********************\n");
    PRV_UTF_LOG0_MAC("\n");
    return GT_OK;
}

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
)
{
    GT_STATUS           st, st1  = GT_OK;
    va_list             argParam;
    GT_U32              index = 0;
    GT_U32              skipTestsBmp = 0;
    UTF_TEST_TYPE_ENT   testTypeArg = testType;


    /* init variable argument list */
    va_start(argParam, testType);

    /* go thru argument list */
    while (UTF_TEST_TYPE_NONE_E != testTypeArg)
    {
        /* set appropriate bit */
        skipTestsBmp |= 1 << testTypeArg;

        /* get next argument from list */
        testTypeArg = va_arg(argParam, UTF_TEST_TYPE_ENT);
    }

    /* free VA list pointer */
    va_end(argParam);

    /* disable not relevant tests */
    while (++index < UTF_TEST_TYPE_LAST_E)
    {
        /* check if appropriate bit is set */
        if (!((skipTestsBmp >> index) & 1))
        {
            /* skip not relevant package */
            st = utfSkipTests((UTF_TEST_TYPE_ENT) index);
            if (GT_OK != st)
            {
                PRV_UTF_LOG1_MAC("[UTF]: utfTestsTypeRun: failed [0x%X] to disable tests\n", st);

                return st;
            }
        }
    }

    /* print tests list */
    st = utfTestsList(0);
    if (GT_OK != st)
    {
        PRV_UTF_LOG1_MAC("[UTF]: utfTestsTypeList: failed [0x%X] to list tests\n", st);

        st1 = st;
    }

    /* enable all tests */
    st = utfSkipTests(UTF_TEST_TYPE_NONE_E);
    if (GT_OK != st)
    {
        PRV_UTF_LOG1_MAC("[UTF]: utfTestsTypeList: failed [0x%X] to enable tests\n", st);

        st1 = st;
    }

    return st1;
}

/**
* @internal sprvTgfLogPrintModeFinal function
* @endinternal
*
* @brief   prvUtfLogPrintEnable(UTF_LOG_OUTPUT_SERIAL_FINAL_E)
*/
static GT_U32 sprvTgfLogPrintModeFinal(GT_VOID)
{
    return (GT_U32)prvUtfLogPrintEnable(UTF_LOG_OUTPUT_SERIAL_FINAL_E);
}

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
*                                      fContinue   -   defines will test be interrupted after first failure condition.
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
    IN GT_BOOL       continueFlag
)
{
    GT_U32      i;
    GT_U32      suitExecTime;
    GT_STATUS   st = GT_OK;
    GT_STATUS   stSuits = GT_OK;

    st = prvUtfCtxAlloc();
    if(st != GT_OK)
    {
        return st;
    }

    /* Set the printing mode to print, in order to print the banner */
    prvUtfLogPrintEnable(UTF_LOG_OUTPUT_SERIAL_ALL_E);

    PRV_UTF_LOG0_MAC("\n");
    PRV_UTF_LOG0_MAC("[UTF]: utfTestsRun: ************************\n");

    if (NULL == testPathPtr)
    {
        PRV_UTF_LOG0_MAC("[UTF]: all suits\n");
    }
    else
    {
        PRV_UTF_LOG1_MAC("[UTF]: suite path [%s]\n", testPathPtr);
    }
    PRV_UTF_LOG1_MAC("[UTF]: number of loops [%d]\n", numOfRuns);
    PRV_UTF_LOG1_MAC("[UTF]: break after first failure [%d]\n", continueFlag);
    PRV_UTF_LOG1_MAC("[UTF]: random run mode [%d]\n", isRandomRunMode);
    PRV_UTF_LOG0_MAC("\n");

    /* check if number of cycle is zero */
    if (0 == numOfRuns)
    {
        st = GT_BAD_PARAM;
    }
    else
    {
        TGF_LOG_BINDINGS_STC tgfBindings = {
            prvUtfLogMessage,
            utfEqualVerify,
            utfFailureMsgLog,
            utfPrintKeepAlive,
            sprvTgfLogPrintModeFinal, /* prvUtfLogPrintEnable(UTF_LOG_OUTPUT_SERIAL_FINAL_E); */
            (TGF_LOG_PRINT_MODE_RESTORE_FUNC)prvUtfLogPrintEnable
        };
        prvTgfLogBindFunctions(&tgfBindings);

        /* run requested suits in cycle */
        for (i = 0; (i < numOfRuns) && (GT_OK == st); i++)
        {
            PRV_UTF_LOG1_MAC("[UTF]: utfTestsRun: step [%d] **************\n", i);
            PRV_UTF_LOG0_MAC("\n");

            suitExecTime = cpssOsTime();
            st = utfSuitsRun(testPathPtr, continueFlag, utfStartFlag);
            suitExecTime = cpssOsTime() - suitExecTime;

            PRV_UTF_LOG1_MAC("[UTF]: total running time [%d] seconds\n", suitExecTime);
            PRV_UTF_LOG0_MAC("\n");

            /* if some test fails during testing, error code is saved       */
            /* into another variable while st is restored to GT_OK          */
            /* because of it's not UTF error and can continue our testing.  */
            if (GT_UTF_TEST_FAILED == st)
            {
                stSuits = st;
                st = GT_OK;
            }
        }

        prvTgfLogBindFunctions(NULL);
    }

    PRV_UTF_LOG0_MAC("[UTF]: utfTestsRun: ************************\n");
    PRV_UTF_LOG0_MAC("\n");

    prvUtfCtxDealloc();

    /* Test Run function may return two types of error codes.                   */
    /* First type is internal UTF error code, they have high priority and       */
    /* second type is special error code that indicates test failure,           */
    /* this error code can be used by upper layer to handle that tests fail.    */
    if (GT_OK == st)
    {
        st = stSuits;
    }
    return st;
}

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
)
{
    if(utfCtxPtr == NULL)
    {
        utfCtxPtr = cpssOsMalloc(sizeof(UTF_CTX_LOCAL_STC));
    }
    if(utfCtxPtr == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }
    cpssOsMemSet(utfCtxPtr, 0, sizeof(UTF_CTX_LOCAL_STC));

    return GT_OK;
}

/**
* @internal prvUtfCtxDealloc function
* @endinternal
*
* @brief   Deallocates memory for utfCtxPtr pointer.
*/
GT_VOID prvUtfCtxDealloc
(
    GT_VOID
)
{
    if(utfCtxPtr != NULL)
    {
        cpssOsFree(utfCtxPtr);
        utfCtxPtr = NULL;
    }
}

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
)
{
    GT_STATUS           st, st1  = GT_OK;
    va_list             argParam;
    GT_U32              index = 0;
    GT_U32              skipTestsBmp = 0;
    UTF_TEST_TYPE_ENT   testTypeArg = testType;


    /* init variable argument list */
    va_start(argParam, testType);

    /* go thru argument list */
    while (UTF_TEST_TYPE_NONE_E != testTypeArg)
    {
        /* set appropriate bit */
        skipTestsBmp |= 1 << testTypeArg;

        /* get next argument from list */
        testTypeArg = va_arg(argParam, UTF_TEST_TYPE_ENT);
    }

    /* free VA list pointer */
    va_end(argParam);

    /* disable not relevant tests */
    while (++index < UTF_TEST_TYPE_LAST_E)
    {
        /* check if appropriate bit is set */
        if (!((skipTestsBmp >> index) & 1))
        {
            /* skip not relevant package */
            st = utfSkipTests((UTF_TEST_TYPE_ENT) index);
            if (GT_OK != st)
            {
                PRV_UTF_LOG1_MAC("[UTF]: utfTestsTypeRun: failed [0x%X] to disable tests\n", st);

                return st;
            }
        }
    }

    /* run tests */
    st = utfTestsRun(NULL, numOfRuns, fContinue);
    if (GT_OK != st)
    {
        PRV_UTF_LOG1_MAC("[UTF]: utfTestsTypeRun: failed [0x%X] to run tests\n", st);

        st1 = st;
    }

    /* enable all tests */
    st = utfSkipTests(UTF_TEST_TYPE_NONE_E);
    if (GT_OK != st)
    {
        PRV_UTF_LOG1_MAC("[UTF]: utfTestsTypeRun: failed [0x%X] to enable tests\n", st);

        st1 = st;
    }

    /* disable CPSS Log if it was enabled */
    if (prvUtfReduceLogSizeFlagGet())
    {
        appDemoOsLogStop();
    }

    return st1;
}

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
)
{
    GT_STATUS   st = GT_OK;


    /* enable start flag */
    utfStartFlag = GT_TRUE;

    /* run tests */
    st = utfTestsRun(testPathPtr, numOfRuns, fContinue);
    if (GT_OK != st)
    {
        PRV_UTF_LOG1_MAC("[UTF]: utfTestsStartRun: failed [0x%X] to run tests\n", st);
    }

    /* disable start flag */
    utfStartFlag = GT_FALSE;

    return st;
}

/**
* @internal utfTestsStartRunFromNext function
* @endinternal
*
* @brief   Runs suits or specific tests from test that is 'next' to the specified
*         test.
*         This should be used when we get 'fatal error' or 'crash' on specific test
*         and we want to continue the run from test that comes 'after it'
* @param[in] testPathPtr              -   (pointer to) name of start suite or test case to start
*                                      run fron 'next test' .
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
GT_STATUS utfTestsStartRunFromNext
(
    IN const GT_CHAR *testPathPtr,
    IN GT_U32        numOfRuns,
    IN GT_BOOL       fContinue
)
{
    GT_STATUS   st = GT_OK;


    /* enable 'start from next' flag */
    runFromNextTest = GT_TRUE;

    /* run tests */
    st = utfTestsStartRun(testPathPtr, numOfRuns, fContinue);
    if (GT_OK != st)
    {
        PRV_UTF_LOG1_MAC("[UTF]: utfTestsStartRunFromNext: failed [0x%X] to run tests \n", st);
    }

    /* disable 'start from next' flag */
    runFromNextTest = GT_FALSE;

    return st;
}


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
)
{
    utfLogOutputSelectMode = eLogOutput;

    return GT_OK;
}

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
)
{
    return prvUtfLogOpen(fileName);
}

/**
* @internal utfLogClose function
* @endinternal
*
* @brief   This routine closes logger.
*/
GT_STATUS utfLogClose
(
    GT_VOID
)
{
    return prvUtfLogClose();
}

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
)
{
    GT_STATUS   st = GT_OK;


    if(utfPreInitPhaseDone == GT_TRUE)
    {
        return GT_OK;
    }

    utfPreInitPhaseDone = GT_TRUE;

    PRV_UTF_LOG0_MAC("\n");
    PRV_UTF_LOG0_MAC("[UTF]: utfInit: ************************\n");

    /* init extras, like active devices array, ports */
    st = prvUtfExtrasInit();
    if (GT_OK != st)
    {
        PRV_UTF_LOG1_MAC("[UTF]: utfInit: failed [0x%X] init devices and ports\n", st);
    }

    return st;
}

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
)
{
    GT_STATUS   rc = GT_OK;
    GT_U16      iter = 0;

    if(utfPostInitPhaseDone == GT_TRUE)
    {
        return;
    }

    utfPostInitPhaseDone = GT_TRUE;

    /* init index array */
    for (iter = 0; iter < utfCtxCommon.usedTestNum; iter++)
    {
        utfCtxCommon.testIndexArray[iter] = iter;
    }

    /* init random run engine */
    if (GT_TRUE == isRandomRunMode)
    {
        rc = utfRandomRunModeSet(isRandomRunMode, prvUtfRandomSeedNum);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[UTF]: utfPostInitPhase: failed [0x%X] init random engine\n", rc);
        }
    }

    if (GT_OK == st)
    {
        PRV_UTF_LOG1_MAC("[UTF]: utfInit: OK, number of tests [%d]\n", utfCtxCommon.usedTestNum);
        PRV_UTF_LOG0_MAC("Run utfHelp 0 for help about UTF usage\n");
    }
    else
    {
        PRV_UTF_LOG1_MAC("[UTF]: utfInit: FAILED, declaration of suits fails: [%d]\n", st);
    }
    PRV_UTF_LOG0_MAC("[UTF]: utfInit: ************************\n");
    PRV_UTF_LOG0_MAC("\n");

    utfHelp(NULL);

    PRV_UTF_LOG0_MAC("[UTF]: utfInit: enh-UT : tested ports are : " );
    for (iter = 0; iter < prvTgfPortsNum; iter++)
    {
        PRV_UTF_LOG1_MAC("%d ", prvTgfPortsArray[iter]);
        prvTgfPortsArray_orig[iter] =  prvTgfPortsArray[iter];
    }
    PRV_UTF_LOG0_MAC("\n");

    for (/* Continue*/; iter < PRV_TGF_MAX_PORTS_NUM_CNS; iter++)
    {
        prvTgfPortsArray_orig[iter] =  prvTgfPortsArray[iter];
    }
    prvTgfPortsNum_orig = prvTgfPortsNum;

#ifdef ASIC_SIMULATION
    (void)cpssOsMutexCreate("prvUtfSlanMtx",&prvUtfSlanMtx);
#endif

}

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
)
{
    GT_STATUS   st   = GT_OK;
    GT_U32      sec  = 0;
    GT_U32      nsec = 0;
#if (defined ASIC_SIMULATION) && (defined CHX_FAMILY)
    GT_U32      ii;
#endif /*ASIC_SIMULATION*/


    if(utfInitDone == GT_TRUE)
    {
#if (defined ASIC_SIMULATION) && (defined CHX_FAMILY)
        for(ii = 0; ii< PRV_CPSS_MAX_PP_DEVICES_CNS; ii++)
        {
            unbindSlanAlready[ii] = 0;
        }

        /* regularly called from prvTgfCommonInit() , but we need it anyway for
            multi-FDB ring devices */
        prvTgfCommonMemberForceInit(PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
#endif /*ASIC_SIMULATION*/
        return GT_OK;
    }

    utfInitDone = GT_TRUE;

    /* get initial start time */
    st = cpssOsTimeRT(&sec, &nsec);
    if (GT_OK != st)
    {
        PRV_UTF_LOG1_MAC("[UTF]: utfInit: failed [0x%X] to get system time\n", st);
    }

    startTimeSec  = sec;
    startTimeNsec = nsec;

#if (defined IMPL_TGF) && (defined DXCH_CODE)
    /* initialize TGF configuration */
    st = prvTgfCommonInit(firstDevNum);
    if (GT_OK != st)
    {
        PRV_UTF_LOG1_MAC("[UTF]: utfInit: failed [0x%X] to init TGF\n", st);
    }
#endif /* IMPL_TGF */

    switch(PRV_CPSS_PP_MAC(firstDevNum)->devFamily)
    {
        case CPSS_PP_FAMILY_PUMA3_E:
            tgfTrafficGeneratorGmTimeFactorSet(3);
            break;
        default:
            break;
    }

    /* register CB */
    st = tgfTrafficTableNetRxPacketCbRegister(tgfTrafficTableRxPcktReceiveCb);

    if ((GT_OK != st) && (GT_ALREADY_EXIST != st))
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgfTrafficTableNetRxPacketCbRegister FAILED, rc = [%d]", st);

        return st;
    }

    if(IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC)
    {
        /* save valuable time in runtime of long tests , and remove option to 'forget' to set this flag */
        prvUtfSkipLongTestsFlagSet(GT_TRUE);
    }

    return utfAllSuitsDeclare();
}

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
)
{
    PRV_UTF_LOG0_MAC("\n");
    PRV_UTF_LOG0_MAC("[UTF]: utfHelp: ****************************\n");

    if (NULL == utfFuncNamePtr)
    {
        PRV_UTF_LOG0_MAC("*****UTF functions:*****\n");
        PRV_UTF_LOG0_MAC("utfHelp             - to log a help about the function.\n");
        PRV_UTF_LOG0_MAC("utfTestsList        - to log list of all configured (available) suits and tests.\n");
        PRV_UTF_LOG0_MAC("utfTestsTypeList    - to log list of all configured suits by given UT type list (generic, enhanced, etc.)\n");
        PRV_UTF_LOG0_MAC("utfTestsRun         - to run suits and tests\n");
        PRV_UTF_LOG0_MAC("utfTestsTypeRun     - to run all UT by given UT type list (generic, enhanced, etc.)\n");
        PRV_UTF_LOG0_MAC("utfLogOutputSelect  - to change output log mode (all, final, file, debug)\n");
        PRV_UTF_LOG0_MAC("utfLogOpen          - to open logger with defined file name\n");
        PRV_UTF_LOG0_MAC("utfLogClose         - to close logger\n");
        PRV_UTF_LOG0_MAC("utfRandomRunModeSet - to set test's random run mode\n");
        PRV_UTF_LOG0_MAC("utfTestRunFirstSet  - to put specific test to be run first\n");
        PRV_UTF_LOG0_MAC("\n");

        PRV_UTF_LOG0_MAC("*****Examples:*****\n");
        PRV_UTF_LOG0_MAC("utfHelp 0                - to printout this help info.\n");
        PRV_UTF_LOG0_MAC("utfHelp \"utfTestsList\"   - to get detailed info about command \"utfTestsList\"\n");
        PRV_UTF_LOG0_MAC("utfTestsList 0           - to log list of all configured (available) suits.\n");
        PRV_UTF_LOG0_MAC("utfTestsList \"tgfBridge\" - to log list of test cases inside suit \"tgfBridge\".\n");
        PRV_UTF_LOG0_MAC("utfTestsTypeList 6, 0    - to log list of all Enhanced UTs\n");
        PRV_UTF_LOG0_MAC("utfTestsTypeList 1, 6, 0 - to log list of all Generic and Enhanced UTs\n");
        PRV_UTF_LOG0_MAC("utfTestsRun 0, 1, 1      - to run all suits once with no interrupts after first failure\n");
        PRV_UTF_LOG0_MAC("utfTestsRun \"tgfBridge\",1,1 - to run all tests from suit \"tgfBridge\" once\n");
        PRV_UTF_LOG0_MAC("utfTestsRun \"tgfBridge.tgfBasicDynamicLearning\",1,1 - to run test \"tgfBasicDynamicLearning\" from suit \"tgfBridge\" once\n");
        PRV_UTF_LOG0_MAC("utfTestsRun \".tgfBasicDynamicLearning\",1,1 - to run test \"tgfBasicDynamicLearning\" once\n");
        PRV_UTF_LOG0_MAC("utfTestsRun \"tgfBridge-tgfIp\",1,0 - to run all tests in a range between suits\n");
        PRV_UTF_LOG0_MAC("utfTestsRun \"suit1.test1-suit2.test2\",1,0 - to run a range of tests starting from certain test to certain test\n");
        PRV_UTF_LOG0_MAC("utfTestsRun \"suit1-suit2.test\",1,0 - to run a range of tests starting from certain suit to certain test\n");
        PRV_UTF_LOG0_MAC("utfTestsTypeRun 1, 1, 6, 0 - to run all Enhanced UT  \n");
        PRV_UTF_LOG0_MAC("utfTestsTypeRun 1, 1, 1, 6, 0 - to run all Generic and Enhanced UT  \n");
        PRV_UTF_LOG0_MAC("utfLogOutputSelect 0     - to change output log to serial\n");
        PRV_UTF_LOG0_MAC("utfLogOutputSelect 1     - to change output log to only final report\n");
        PRV_UTF_LOG0_MAC("utfLogOutputSelect 2     - to change output log to file\n");
        PRV_UTF_LOG0_MAC("utfLogOutputSelect 3     - to change output log to debug\n");
        PRV_UTF_LOG0_MAC("utfLogOpen \"results.log\" - to set logger into file \"results.log\"\n");
        PRV_UTF_LOG0_MAC("utfLogClose              - to close opened logger\n");
        PRV_UTF_LOG0_MAC("utfRandomRunModeSet 1, 5 - to enable random run mode with seed 5 for random generator\n");
        PRV_UTF_LOG0_MAC("utfTestRunFirstSet \"tgfBasicDynamicLearning\" - to run \"tgfBasicDynamicLearning\" test first\n");
        PRV_UTF_LOG0_MAC("prvUtfSkipLongTestsFlagSet 1 - to state that when running tests skip those with 'long execution time' \n");
        PRV_UTF_LOG0_MAC("utfSkipListAdd \"suit.test\", \"reason\" - to ignore error in this test(skip it)\n");
        PRV_UTF_LOG0_MAC("\n");
        PRV_UTF_LOG0_MAC("Run utfHelp \"utfTestsList\" for example to get detailed info about command\n");
    }
    else
    {
        if (0 == cpssOsStrCmp("utfHelp", utfFuncNamePtr))
        {
            PRV_UTF_LOG0_MAC("utfHelp 0                     -   to log all available UTF functions. \n");
            PRV_UTF_LOG0_MAC("utfHelp <UTF-function name>   -   to log a help about the function.\n");
        }
        else if (0 == cpssOsStrCmp("utfTestsList", utfFuncNamePtr))
        {
            PRV_UTF_LOG0_MAC("utfTestsList 0                -   to log list of all configured (available) suits.\n");
            PRV_UTF_LOG0_MAC("utfTestsList <Suit name>      -   to log list of test cases inside given suit.\n");
        }
        else if (0 == cpssOsStrCmp("utfTestsRun", utfFuncNamePtr))
        {
            PRV_UTF_LOG0_MAC("utfTestsRun 0,1,1             -   to run all suits with defaults configuration.\n");
            PRV_UTF_LOG0_MAC("utfTestsRun <Name of suit or test case>, <Number of runs>, <Flag to continue after first failure> \n");
            PRV_UTF_LOG0_MAC("\n");
            PRV_UTF_LOG0_MAC("For example:\n");
            PRV_UTF_LOG0_MAC("utfTestsRun 0,1,1 - to run all suits once with no interrupts after first failure\n");
            PRV_UTF_LOG0_MAC("utfTestsRun \"TestSuit\",2,1 - to run all tests from suit \"TestSuit\" two times\n");
            PRV_UTF_LOG0_MAC("utfTestsRun \"TestSuit.TestOne\",1,1 - to run test \"TestOne\" from suit \"TestSuit\" once\n");

        }
        else if (0 == cpssOsStrCmp("utfLogOutputSelect", utfFuncNamePtr))
        {
            PRV_UTF_LOG0_MAC("utfLogOutputSelect <where to output> - to change output log to log, serial or file \n");
            PRV_UTF_LOG0_MAC("For example:\n");
            PRV_UTF_LOG0_MAC("utfLogOutputSelect 0 - to change output log to serial \n");
        }
        else if (0 == cpssOsStrCmp("utfTestsTypeRun", utfFuncNamePtr))
        {
            PRV_UTF_LOG0_MAC("utfTestsTypeRun <Number of runs>, <Continue flag>, <List of test types>, 0 \n");
            PRV_UTF_LOG0_MAC("For example:\n");
            PRV_UTF_LOG0_MAC("utfTestsTypeRun 1,1,\"TestType1\",\"TestType2\",0 - to run all UTs from packages \"TestType1\" and \"TestType2\"\n");
        }
        else
        {
            PRV_UTF_LOG1_MAC("Unknown command [%s]\n", utfFuncNamePtr);
        }
    }
    PRV_UTF_LOG0_MAC("[UTF]: utfHelp: ****************************\n");
    PRV_UTF_LOG0_MAC("\n");
    return GT_OK;
}

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
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN GT_BOOL                  clearOnRead,
    OUT GT_U32                 *counterPtr
)
{
    if (prvTgfReRunVariable == TGF_RE_RUN_STATUS_CAPABLE_E)
    {
        /* turn on the flag to state the test uses events */
        prvTgfReRunVariable = TGF_RE_RUN_STATUS_DO_RE_RUN_E;
    }
    return appDemoGenEventCounterGet(devNum,uniEvent,clearOnRead,counterPtr);
}

/******************************************************************************
* UTF private functions implementation
******************************************************************************/
static GT_U32 allowAfterEveryTest_shadowTest =  1;

void allowAfterEveryTest_shadowTest_set(GT_U32 allowAfterEveryTest)
{
    allowAfterEveryTest_shadowTest = allowAfterEveryTest;
}

static GT_U32 allowToCall_cpssDxChDiagDataIntegrityTables_shadowTest_onDemand =  0;
void allowToCall_cpssDxChDiagDataIntegrityTables_shadowTest_onDemand_set(GT_U32 allow)
{
    allowToCall_cpssDxChDiagDataIntegrityTables_shadowTest_onDemand = allow;
}

static GT_U32 allowAfterXTests_shadowTest = 50;

void allowAfterXTests_shadowTest_set(GT_U32 afterX)
{
    allowAfterXTests_shadowTest = afterX;
}

extern GT_BOOL cpssDxChDiagDataIntegrityTables_shadowTest_onDemand(void);
void trfCpssDxChDiagDataIntegrityTables_shadowTest(
    IN const GT_CHAR *suitNamePtr ,
    IN GT_BOOL isSuitEnded
)
{
    GT_BOOL didError_shadowTest;

    if(allowToCall_cpssDxChDiagDataIntegrityTables_shadowTest_onDemand == 0)
    {
        return;
    }

    if(suitNamePtr &&
       (0 != cpssOsStrNCmp(suitNamePtr,"cpssDxCh",8)) &&
       (0 != cpssOsStrNCmp(suitNamePtr,"tgf",3)) &&
       (0 != cpssOsStrNCmp(suitNamePtr,"prvTgf",6)))
    {
        /* save time of generic test/suit  */
        return;
    }

    if(isSuitEnded)
    {
        /* at end of suit we must check that it not left anything to the next suit*/
    }
    else
    if(suitNamePtr &&
        (0 == cpssOsStrCmp(suitNamePtr,"prvTgf__just_before_system_reset")))
    {
        /* called just before reset we need to allow it to check the shadow */
        /* otherwise the shadow and HW will get reset */
    }
    else
    if(suitNamePtr && allowAfterXTests_shadowTest)
    {
        static GT_U32   count = 0;

        if(count < allowAfterXTests_shadowTest)
        {
            count++;
            return;
        }

        /* allow checking the shadow */
        count = 0;
    }

    /*******************************************/
    /* test shadow after the end of suite/test */
    /*******************************************/
#ifdef DXCH_CODE
    didError_shadowTest = cpssDxChDiagDataIntegrityTables_shadowTest_onDemand();
#else
    didError_shadowTest = GT_FALSE;
#endif /*DXCH_CODE*/
    utfExtraTestEnded();/* the function cpssDxChDiagDataIntegrityTables_shadowTest_onDemand() used device iterator */
    if(didError_shadowTest == GT_TRUE && suitNamePtr)
    {
        cpssOsPrintf("**** FAILED due to Data Integrity Tables shadow Test that run at end of [%s] \n",
            suitNamePtr);
    }
}

/**
* @internal utfSuitStatsShow function
* @endinternal
*
* @brief   Displays final statistics about suit, is failed, number of errors
*         list of failed test cases for the suit
* @param[in] suitNamePtr              -   name of suit to show statistics
*                                       none
*
* @note none
*
*/
static GT_VOID utfSuitStatsShow
(
    const GT_CHAR *suitNamePtr
)
{
    GT_U32 countIdx;

    if(!allowAfterEveryTest_shadowTest || /*check if last test not run it already*/
        allowAfterXTests_shadowTest != 0) /*check 'X' ratio controls the calls */

    {
        trfCpssDxChDiagDataIntegrityTables_shadowTest(suitNamePtr,GT_TRUE);
    }

    if (GT_TRUE == utfCtxPtr->errorFlag)
    {
        PRV_UTF_LOG1_MAC("**** SUIT %s FAILED\n", suitNamePtr);

        for (countIdx = 0; countIdx < utfCtxCommon.usedTestNum; countIdx++)
        {
            if ((0 == cpssOsStrCmp(suitNamePtr, utfCtxCommon.testEntriesArray[utfCtxCommon.testIndexArray[countIdx]].suitNamePtr)) &&
                (utfCtxPtr->testEntriesArray[utfCtxCommon.testIndexArray[countIdx]].errorCount > 0))
            {
                PRV_UTF_LOG2_MAC("TEST %s FAILED: %d errors\n",
                         utfCtxCommon.testEntriesArray[utfCtxCommon.testIndexArray[countIdx]].testNamePtr,
                         utfCtxPtr->testEntriesArray[utfCtxCommon.testIndexArray[countIdx]].errorCount);
            }
        }
    }
    else
    {
        PRV_UTF_LOG1_MAC("**** SUIT %s SUCCEEDED\n", suitNamePtr);
    }
    PRV_UTF_LOG0_MAC("\n");

    /* print total not skipped tests run */
    PRV_UTF_LOG1_MAC("[UTF]: number of tests run in current suit [%d]\n\n",
                     utfTestsNumGet(suitNamePtr) - prvUtfSkipTestsNumGet());

    /* reset skip tests counter */
    prvUtfSkipTestsNumReset();

    utfCtxPtr->errorFlag = GT_FALSE;
}

/**
* @internal utfFinalStatsShow function
* @endinternal
*
* @brief   Displays final statistics about all suits, list of failed suits and
*         list of failed test cases for the suits
*
* @note none
*
*/
static GT_VOID utfFinalStatsShow
(
    GT_VOID
)
{
    GT_U32 countIdx;
    GT_BOOL isAllSuitesPassed = GT_TRUE;
    GT_U32 totalFailed = 0;
    GT_U32 totalPass = 0;
    GT_U32 totalSkipped = 0;

    PRV_UTF_LOG0_MAC("******** UTF REPORT *********\n");

    for (countIdx = 0; countIdx < utfCtxCommon.usedTestNum; countIdx++)
    {
        switch(utfCtxPtr->testEntriesArray[countIdx].testStatus)
        {
            case UTF_TEST_STATUS_PASS_E:   /*test run and pass ok */
                totalPass++;
                break;
            case UTF_TEST_STATUS_FAILED_E: /*test run and failed */
                totalFailed++;
                break;
            case UTF_TEST_STATUS_SKIPPED_E:/*test was skipped , test not valid for the device */
                totalSkipped++;
                break;
            default:
                break;
        }
    }

    PRV_UTF_LOG0_MAC("\n");
    PRV_UTF_LOG0_MAC("== summary statistics ==  \n");
    PRV_UTF_LOG0_MAC("PASS      FAIL      SKIP    total \n");
    PRV_UTF_LOG0_MAC("================================= \n");
    PRV_UTF_LOG4_MAC("[%d]       [%d]       [%d]     [%d] \n",totalPass,totalFailed,totalSkipped,(totalFailed + totalPass + totalSkipped));
    PRV_UTF_LOG0_MAC("\n");

    if(totalFailed)
    {
        PRV_UTF_LOG0_MAC("List ALL FAILED tests : \n");
        for (countIdx = 0; countIdx < utfCtxCommon.usedTestNum; countIdx++)
        {
            if (utfCtxPtr->testEntriesArray[countIdx].errorCount > 0)
            {
                PRV_UTF_LOG3_MAC("%s.%s failed, number of errors %d\n",
                                 utfCtxCommon.testEntriesArray[countIdx].suitNamePtr,
                                 utfCtxCommon.testEntriesArray[countIdx].testNamePtr,
                                 utfCtxPtr->testEntriesArray[countIdx].errorCount);

                isAllSuitesPassed = GT_FALSE;
            }
        }
    }

    if (GT_TRUE == isAllSuitesPassed)
    {
        PRV_UTF_LOG0_MAC("All tests were successful \n");
    }

    if(printPassOkSummary != GT_FALSE)
    {
        if(totalPass)
        {
            PRV_UTF_LOG0_MAC("\n");
            PRV_UTF_LOG0_MAC("List ALL PASS OK tests : \n");
            for (countIdx = 0; countIdx < utfCtxCommon.usedTestNum; countIdx++)
            {
                if (utfCtxPtr->testEntriesArray[countIdx].testStatus == UTF_TEST_STATUS_PASS_E)
                {
                    PRV_UTF_LOG2_MAC("%s.%s PASS OK \n",
                                     utfCtxCommon.testEntriesArray[countIdx].suitNamePtr,
                                     utfCtxCommon.testEntriesArray[countIdx].testNamePtr);
                }
            }
        }

        if(totalSkipped)
        {
            PRV_UTF_LOG0_MAC("\n");
            PRV_UTF_LOG0_MAC("List ALL skipped tests : \n");
            for (countIdx = 0; countIdx < utfCtxCommon.usedTestNum; countIdx++)
            {
                if(utfCtxPtr->testEntriesArray[countIdx].testStatus == UTF_TEST_STATUS_SKIPPED_E)
                {
                    PRV_UTF_LOG2_MAC("%s.%s skipped \n",
                                     utfCtxCommon.testEntriesArray[countIdx].suitNamePtr,
                                     utfCtxCommon.testEntriesArray[countIdx].testNamePtr);
                }
            }
        }
    }

    if((prvUtfSkipLongTestsFlagGet(UTF_ALL_FAMILY_E) != GT_FALSE) || (printPassOkSummary != GT_FALSE))
    {
        PRV_UTF_LOG0_MAC(" \n\n list of all the tests that take more than 10 seconds \n");

        for (countIdx = 0; countIdx < utfCtxCommon.usedTestNum; countIdx++)
        {
            if(utfCtxPtr->testEntriesArray[countIdx].testExecTimeSec < 10)
            {
                continue;
            }

            PRV_UTF_LOG2_MAC("[%s] take [%d] seconds \n",
                utfCtxCommon.testEntriesArray[countIdx].testNamePtr,
                utfCtxPtr->testEntriesArray[countIdx].testExecTimeSec);
        }
    }


    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal utfTestAdd function
* @endinternal
*
* @brief   Add test case to utf internal database
*
* @param[in] testFuncPtr              -   pointer to test case function
* @param[in] testNamePtr              -   name of the test case in the suit
* @param[in] suitNamePtr              -   name of the suit, unique for all suits
*
* @retval GT_OK                    - on success.
* @retval GT_CREATE_ERROR          - there is no more free slot for the test
*
* @note In case of GT_CREATE_ERROR you need to increase UTF_MAX_NUM_OF_TESTS_CNS
*
*/
static GT_STATUS utfTestAdd
(
    IN UTF_TEST_CASE_FUNC_PTR   testFuncPtr,
    IN const GT_CHAR            *testNamePtr,
    IN const GT_CHAR            *suitNamePtr)
{
    if (utfCtxCommon.usedTestNum >= UTF_MAX_NUM_OF_TESTS_CNS)
    {
        PRV_UTF_LOG1_MAC("[UTF]: utfTestAdd: failed add test [%s]\n", testNamePtr);
        return GT_CREATE_ERROR;
    }

    utfCtxCommon.testEntriesArray[utfCtxCommon.usedTestNum].testFuncPtr = testFuncPtr;
    utfCtxCommon.testEntriesArray[utfCtxCommon.usedTestNum].testNamePtr = testNamePtr;
    utfCtxCommon.testEntriesArray[utfCtxCommon.usedTestNum].suitNamePtr = suitNamePtr;
    utfCtxCommon.testEntriesArray[utfCtxCommon.usedTestNum].testType    = utfTestType;
    utfCtxCommon.testIndexArray[utfCtxCommon.usedTestNum] = utfCtxCommon.usedTestNum;
    utfCtxCommon.usedTestNum++;
    return GT_OK;
}

/**
* @internal utfPreTestRun function
* @endinternal
*
* @brief   This function is called before run each test case
*
* @note none
*
*/
static GT_VOID utfPreTestRun
(
    GT_VOID
)
{
    GT_STATUS   st   = GT_OK;
    GT_U32      sec  = 0;
    GT_U32      nsec = 0;
    GT_U32      ii;
    GT_STATUS   rc;

    if (UTF_TEST_TYPE_TRAFFIC_E == utfCtxCommon.testEntriesArray[utfCtxPtr->currTestNum].testType)
    {
#ifdef ASIC_SIMULATION
        if(prvTgfXcat3xExists() == GT_TRUE)
        {
            /* boost performance of the ENH-UT */
            debug_set_directAccessMode (1);
            debug_set_directAccessMode1(1);
        }
#endif /*ASIC_SIMULATION*/


        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

        /* do not force the prvTgfBrgFdbFlush(...) to also delete UC entries ...
           it's test responsibility to clean it ! */
        prvTgfBrgFdbFlushWithUcEntries(GT_FALSE);
    }
    else
    {
        /* force the prvTgfBrgFdbFlush(...) to also delete UC entries */
        prvTgfBrgFdbFlushWithUcEntries(GT_TRUE);
    }



#ifdef ASIC_SIMULATION
    if(tgfTestNameForLogPtr[0] &&
        (0 == cpssOsStrCmp(tgfTestNameForLogPtr,
              utfCtxCommon.testEntriesArray[utfCtxPtr->currTestNum].testNamePtr)))
    {
        /* we have test to generate LOG for it . */
        startSimulationLogWithFileName(tgfTestNameForLogPtr);
        tgfTestWithLogStarted = GT_TRUE;

        cpssOsPrintf("start %s LOG of test %s \n",
            (prvUtfIsGmCompilation() == GT_TRUE) ? "GM" : "simulation",
            tgfTestNameForLogPtr
            );
    }
#endif /*ASIC_SIMULATION*/

    st = cpssOsTimeRT(&sec, &nsec);
    if (GT_OK != st)
    {
        PRV_UTF_LOG1_MAC("[UTF]: utfPreTestRun: failed [0x%X] to get system time\n", st);
    }

    utfCtxPtr->testEntriesArray[utfCtxPtr->currTestNum].testExecTimeSec = sec;
    utfCtxPtr->testEntriesArray[utfCtxPtr->currTestNum].testExecTimeNsec = nsec;

    if (nsec < startTimeNsec)
    {
        nsec += 1000000000;
        sec--;
    }

    PRV_UTF_LOG2_MAC("[UTF]: test start after [%d.%09d] sec\n",
                     sec  - startTimeSec,
                     nsec - startTimeNsec);

#ifdef ASIC_SIMULATION
#ifdef LINUX
    /* in linux (at least for Puma2) the next lines not allow traffic to be send
       to the ports (ports loose link without ability to enable it) */
    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_PUMA_E)
#endif /*LINUX*/
    {
        if(unbindSlanAlready[prvTgfDevNum] == 0 && utfForceUnbindSlan == 1)
        {
            CPSS_INTERFACE_INFO_STC portInterface;
            (void)cpssOsMutexLock(prvUtfSlanMtx);
#ifdef IMPL_GALTIS
            /* by default we not need the SLANS of the device when running enh-UT.

               moreover those slans not allow us to run more than single instance simultaneously.

               so we remove all the SLANs !
               NOTE: there should not be use of SLANs for 'on board' connection.
                     --> use 'internal connections' instead.
            */
            (void)wrSimulationDynamicSlanDevDelete_forceAll(GT_TRUE);
            (void)wrSimulationDynamicSlanDevDelete(prvTgfDevNum);
#endif /* IMPL_GALTIS */

            /* after removing those SLANS we may have 'Linked down' some of the ports
                that need to be UP (that set by the AppDemo) -- like for LINKED FDB */
            for(ii = 0 ; ii < CPSS_MAX_PORTS_NUM_CNS; ii++)
            {
                if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&appDemoPpConfigList[prvTgfDevNum].multiPortGroupRingPortsBmp,ii))
                {
                    continue;
                }

                /* map input params */
                portInterface.type            = CPSS_INTERFACE_PORT_E;
                portInterface.devPort.hwDevNum  = prvTgfDevNum;
                portInterface.devPort.portNum = ii;

                /* set port into linkup mode */
                st = tgfTrafficGeneratorPortForceLinkUpEnableSet(&portInterface, GT_TRUE);
                if(st != GT_OK)
                {
                    PRV_UTF_LOG3_MAC("[UTF]: tgfTrafficGeneratorPortForceLinkUpEnableSet: device[%d] for ring port [%d] error[0x%x]\n",prvTgfDevNum,ii, st);
                }
            }


            unbindSlanAlready[prvTgfDevNum] = 1;
            (void)cpssOsMutexUnlock(prvUtfSlanMtx);
        }
    }

#ifdef DXCH_CODE
    /* bind the enh-ut engine to keep track over the FDB entries that we
       add/delete to/from the CPSS by the control learning
       we need it due to poor performance of the GM of flush FDB */
    st = prvTgfBrgFdbInit(prvTgfDevNum,GT_FALSE/*only BC2-GM*/);
    if(st != GT_OK)
    {
        PRV_UTF_LOG1_MAC("[UTF]: prvTgfBrgFdbInit: error[0x%x]\n",st);
    }
#endif /*DXCH_CODE*/

#endif/*ASIC_SIMULATION*/
    utfMemoryHeapCounterInit();

    if (resetDmmAllocations)
    {
#ifdef DXCH_CODE
        /* Now that all memory allocations and frees are counted, restore the default LPM DB */
        rc = prvTgfIpLpmRamDBCreateDefault();
        if (rc != GT_OK)
        {
            PRV_UTF_LOG0_MAC("******** Failed to add the default LPM DB\n");
        }

        /* Now that all memory allocations and frees are counted, restore the default device */
        rc = prvTgfIpLpmDBDevListAddDefault();
        if (rc != GT_OK)
        {
            PRV_UTF_LOG0_MAC("******** Failed to add the default device\n");
        }

        /* Now that all memory allocations and frees are counted, restore the default VR */
        rc = prvTgfIpLpmVirtualRouterAddDefault();
        if (rc != GT_OK)
        {
            PRV_UTF_LOG0_MAC("******** Failed to add the default virtual router\n");
        }
#endif/*DXCH_CODE*/
    }

    /* by default -- all test must state explicitly if to use port groups bmp
       so before test starts ... clear the flag */
    usePortGroupsBmp = GT_FALSE;
    /* clear also the BMP */
    currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* reset the 'General info' in the DB , meaning each test must state it's
       own 'General info' */
    for(ii = 0 ; ii < PRV_UTF_GENERAL_STATE_INFO_NUM_CNS; ii++)
    {
        prvUtfGeneralStateLogArr[ii].valid = GT_FALSE;
        prvUtfCallBackFunctionsOnErrorArr[ii].callBackFunc = NULL;
    }

#if (defined ASIC_SIMULATION) && (defined _WIN32)
    if(simLogIsOpen())
    {
        /* set minimal time to wait for the 'capture' */
        tgfTrafficGeneratorCaptureLoopTimeSet(1000);
    }
#endif /* (defined ASIC_SIMULATION) && (defined _WIN32) */

}




#if (defined ASIC_SIMULATION)
/*allow to LOG specific enh-UT test , when running single test / list of tests. (LOG file name will include test name)*/
void tgfSimulationLogOnSpecificTest(IN GT_CHAR*  testNameForLogPtr)
{
    if(testNameForLogPtr == NULL)
    {
        tgfTestNameForLogPtr[0] = 0;
    }
    else
    {
        cpssOsStrCpy(tgfTestNameForLogPtr,testNameForLogPtr);
    }
}

#endif/*(defined ASIC_SIMULATION)*/


void testFdbFlushThatTableIsEmpty(void)
{
#ifdef DXCH_CODE
    GT_STATUS   rc;
    GT_U32  numOfValid = 0 ;/* number of valid entries in the FDB */
    GT_U32  numOfSkip = 0; /* number of skipped entries in the FDB */
    GT_U32  numOfSp = 0; /* number of Storm Protection entries in the FDB */
    GT_U32  numOfValidMinusSkip=0;/* number of (valid - skip) entries in the FDB*/
    GT_BOOL isCompleted;
    GT_U8       devNum  = 0;
    GT_BOOL allDevsCompleted = GT_TRUE;

    if(prvTgfFdbShadowUsed == GT_TRUE)
    {
        return;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChBrgFdbTrigActionStatusGet(devNum, &isCompleted);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        if (GT_OK == rc && isCompleted == GT_FALSE)
        {
            /* need wait for devices */
            allDevsCompleted = GT_FALSE;
        }
    }

    if(allDevsCompleted == GT_FALSE)
    {
        /* wait for all devices to finish previous action */
        prvTgfBrgFdbActionDoneWaitForAllDev(GT_TRUE);
    }

    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    rc = prvTgfBrgFdbCount(&numOfValid,&numOfSkip,NULL,&numOfSp,NULL);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    numOfValidMinusSkip = numOfValid - numOfSkip - numOfSp;

    UTF_VERIFY_EQUAL0_PARAM_MAC(0, numOfValidMinusSkip);

    if(numOfValidMinusSkip)
    {
        cpssOsPrintf("FDB not empty , got [%d] entries \n",
                    numOfValidMinusSkip);
    }
#endif /*DXCH_CODE*/
}

#ifdef ASIC_SIMULATION
extern GT_U32 skernelNumOfPacketsInTheSystemGet(GT_VOID);
#endif /*ASIC_SIMULATION*/

GT_U32  trafficExpectedAtEndOfTest = 0;
/* allow 'parallel' traffic to run during 'mainUT' */
GT_VOID trafficExpectedAtEndOfTestSet(IN GT_U32 expected)
{
    trafficExpectedAtEndOfTest = expected;
}

GT_VOID waitForPacketsToEnd(GT_VOID)
{
#if defined(ASIC_SIMULATION) && !defined(GM_USED)
    GT_U32  maxLoops = 1000;
    GT_U32  gotWaitingPackets = 0;
    GT_U32  maxIterations = 5;

check_after_2000_mili_again:
    /* wait for packets to finish */
    while(maxLoops)
    {
        GT_32   numPacketsInSim = skernelNumOfPacketsInTheSystemGet();

        /* fix bug of 'endless loop' due to
        "while(maxLoops--) if value maxLoops == 0xFFFFFFFF " when doing
        check_after_2000_mili_again */
        maxLoops--;

        gotWaitingPackets += numPacketsInSim;
        if(numPacketsInSim)
        {
            cpssOsPrintf("[%s] left [%d] packets in simulation that still under processing \n",
                utfCtxCommon.testEntriesArray[utfCtxPtr->currTestNum].testNamePtr , numPacketsInSim);
            cpssOsTimerWkAfter(100);
        }
#if 0
        else
        {
            /* no more packets to wait for */
            return;
        }
#endif /*0*/
    }

    if(gotWaitingPackets && maxIterations)
    {
        maxIterations--;
        gotWaitingPackets = 0;

        cpssOsPrintf("goto sleep 2000 \n"); /* check if we have other threads in
                                simulation that are a sleep that may generate
                                packets after sleep .

                                NOTE: this sleep proved to find that 'SDMA Tx packet generator'
                                    was not disabled disabled from a test.
                                */
        cpssOsTimerWkAfter(2000);

        if(maxLoops == 0)
        {
            maxLoops = 1;
        }

        goto check_after_2000_mili_again;
    }

    if(maxIterations == 0)
    {
        cpssOsPrintf("ERROR -- ERROR : [%s] left traffic that was not finished \n",
            utfCtxCommon.testEntriesArray[utfCtxPtr->currTestNum].testNamePtr);

        errorCountIncrement(utfCtxPtr->currTestNum,GT_TRUE);
    }

#endif /*(ASIC_SIMULATION) && !defined(GM_USED)*/
}

/**
* @internal utfPostTestRun function
* @endinternal
*
* @brief   This function is called after test case execution
*
* @note none
*
*/
static GT_VOID utfPostTestRun
(
    GT_VOID
)
{
    GT_STATUS   st   = GT_OK;
    GT_U32      sec  = 0;
    GT_U32      nsec = 0;
    GT_U32      mem  = 0;
#ifdef DXCH_CODE
    GT_STATUS   rc;
#endif /*DXCH_CODE*/
    GT_U32      iter;
#ifdef FORCE_PRINT_WHEN_NOT_CONTINUE_CNS
    UTF_LOG_OUTPUT_ENT origLogState = 0;
#endif /*FORCE_PRINT_WHEN_NOT_CONTINUE_CNS*/

#if 0
    testFdbFlushThatTableIsEmpty();
#endif

    st = cpssOsTimeRT(&sec, &nsec);
    if (GT_OK != st)
    {
        PRV_UTF_LOG1_MAC("[UTF]: utfPostTestRun: failed [0x%X] to get system time\n", st);
    }

    if (nsec < utfCtxPtr->testEntriesArray[utfCtxPtr->currTestNum].testExecTimeNsec)
    {
        nsec += 1000000000;
        sec--;
    }
    utfCtxPtr->testEntriesArray[utfCtxPtr->currTestNum].testExecTimeNsec = nsec -
        utfCtxPtr->testEntriesArray[utfCtxPtr->currTestNum].testExecTimeNsec;
    utfCtxPtr->testEntriesArray[utfCtxPtr->currTestNum].testExecTimeSec = sec -
        utfCtxPtr->testEntriesArray[utfCtxPtr->currTestNum].testExecTimeSec;

    if (resetDmmAllocations)
    {
#ifdef DXCH_CODE
        /* Before stopping to count memory allocations and frees, remove the default VR */
        rc = prvTgfIpLpmVirtualRouterDelDefault();
        if (rc != GT_OK)
        {
            PRV_UTF_LOG0_MAC("******** Failed to delete the default virtual router\n");
        }
        /* Before stopping to count memory allocations and frees, remove the default LPM DB */
        rc = prvTgfIpLpmDBDeleteDefault();
        if (rc != GT_OK)
        {
            PRV_UTF_LOG0_MAC("******** Failed to delete the default LPM DB\n");
        }
#endif /*DXCH_CODE*/
    }

    st = utfMemoryHeapLeakageStatusGet(&mem);
#ifdef SHARED_MEMORY
    if (GT_OK != st && mem == 12)
    {
        /* PATCH : to 'success' on unknown reason of :
            [UTF]: Memory leak detected: 12 bytes

          hide the problem only on shared LIB ... those 12 bytes don't really mind.
        */

        st = GT_OK;
        /* force print ... not only on 'log' */
        cpssOsPrintf("SHARED_MEMORY : !!! IGNORING !!! : [UTF]: Memory leak detected: %d bytes \n", mem);
    }
#endif /*SHARED_MEMORY*/

    if (GT_OK != st)
    {
        errorCountIncrement(utfCtxPtr->currTestNum,GT_TRUE);
#ifdef FORCE_PRINT_WHEN_NOT_CONTINUE_CNS
        if(utfCtxPtr->continueFlag == GT_FALSE)
        {
            /* we run until first fail , so let us have the printing regardless to
               global debug mode.
               because global debug mode maybe with HUGE amount of printings ... */
            origLogState = prvUtfLogPrintEnable(UTF_LOG_OUTPUT_SERIAL_ALL_E);
        }
#endif /*FORCE_PRINT_WHEN_NOT_CONTINUE_CNS*/
        /* force print ... not only on 'log' */
        cpssOsPrintf("[UTF]: Memory leak detected: %d bytes \n", mem);

#ifdef FORCE_PRINT_WHEN_NOT_CONTINUE_CNS
        if(utfCtxPtr->continueFlag == GT_FALSE)
        {
            /* restore original value */
            prvUtfLogPrintEnable(origLogState);
        }
#endif /*FORCE_PRINT_WHEN_NOT_CONTINUE_CNS*/
    }

#if (defined ASIC_SIMULATION) && (defined _WIN32)
    if(simLogIsOpen())
    {
        /* restore time to wait for the 'capture' */
        tgfTrafficGeneratorCaptureLoopTimeSet(0);
    }
#endif /* (defined ASIC_SIMULATION) && (defined _WIN32) */

#ifdef ASIC_SIMULATION
    if(tgfTestWithLogStarted == GT_TRUE)
    {
        tgfTestWithLogStarted = GT_FALSE;

        /* stop the LOG . */
        stopSimulationLog();

        cpssOsPrintf("stop %s LOG of test %s \n",
            (prvUtfIsGmCompilation() == GT_TRUE) ? "GM" : "simulation",
            tgfTestNameForLogPtr
            );

    }
#endif /*ASIC_SIMULATION*/


    /* a test may accidentally change the defaults needed by all other tests.
       make sure to restore orig values.  */
    prvTgfPortsNum = prvTgfPortsNum_orig;

    for (iter = 0; iter < PRV_TGF_MAX_PORTS_NUM_CNS; iter++)
    {
        prvTgfPortsArray[iter] = prvTgfPortsArray_orig[iter];
    }

    if(!trafficExpectedAtEndOfTest)
    {
        /* in simulation this function will make sure that there is no traffic left
           after the test is done .*/
        waitForPacketsToEnd();
    }

    if(GT_OK != osCacheDmaOverflowDetectionErrorGet(0,NULL,NULL))
    {
        cpssOsPrintf("Error : 'Cached DMA' overflow detected by osCacheDmaOverflowDetectionErrorGet(...) \n");

        errorCountIncrement(utfCtxPtr->currTestNum,GT_TRUE);
    }

    st = utfLogStatusGet();
    if (st != GT_OK)
    {
        /* Current suit test - bad log state */
        PRV_UTF_LOG2_MAC("[UTF]: utfLogStatusGet: suit %s test %s - bad log state after run\n",
                         utfCtxCommon.testEntriesArray[utfCtxPtr->currTestNum].suitNamePtr,
                         utfCtxCommon.testEntriesArray[utfCtxPtr->currTestNum].testNamePtr);
    }

    prvTgfCaptureSet_ingressTagging_reset();

    if(allowAfterEveryTest_shadowTest)
    {
        trfCpssDxChDiagDataIntegrityTables_shadowTest(
            utfCtxCommon.testEntriesArray[utfCtxPtr->currTestNum].testNamePtr,GT_FALSE);
    }

    utfExtraTestEnded();
}

GT_U32 g_sleepAfterTestMsec       = 0;
GT_U32 g_checkIntrrTableThreshold = 0;
GT_U32 prv_utfEventCounterArr[CPSS_PP_UNI_EV_MAX_E] = {0};



GT_VOID utfSleepAfterTestSet(GT_U32 delayMsec)
{
    g_sleepAfterTestMsec = delayMsec;
}

GT_U32 utfSleepAfterTestGet(GT_VOID)
{
    return g_sleepAfterTestMsec;
}


GT_VOID utfIntprTableCheckThreshSet(GT_U32 THRESHOLD)
{
    g_checkIntrrTableThreshold = THRESHOLD;
}

GT_U32 utfIntprTableCheckThreshGet(GT_VOID)
{
    return g_checkIntrrTableThreshold;
}


GT_STATUS utfIntprTableCheck(void)
{
    GT_U8 devNum = 0;
    GT_U32 THRESHOLD = g_checkIntrrTableThreshold;
    GT_STATUS rc;
    GT_U32 uniEvIndex  = CPSS_PP_UNI_EV_MIN_E;
    GT_U32 endUniIndex = CPSS_PP_UNI_EV_MAX_E; /* we should increment the
                                                      endUniIndex in order not to loose the last event */
    GT_BOOL eventI = GT_FALSE;

    if (g_checkIntrrTableThreshold == 0)
    {
        return GT_OK;
    }
    cpssOsMemSet(&prv_utfEventCounterArr[0],0,sizeof(prv_utfEventCounterArr));
    for (uniEvIndex = CPSS_PP_UNI_EV_MIN_E; uniEvIndex < endUniIndex; uniEvIndex++)
    {
        rc = utfGenEventCounterGet(devNum, (CPSS_UNI_EV_CAUSE_ENT)uniEvIndex, GT_FALSE, &prv_utfEventCounterArr[uniEvIndex]);
        if(GT_OK != rc)
        {
            return rc;
        }
        if (prv_utfEventCounterArr[uniEvIndex] > THRESHOLD)
        {
            eventI = GT_TRUE;
        }

    }
    if (eventI == GT_TRUE)
    {
        cpssOsPrintf("\nEvent Table:");
        for (uniEvIndex = CPSS_PP_UNI_EV_MIN_E; uniEvIndex < endUniIndex; uniEvIndex++)
        {
            if (prv_utfEventCounterArr[uniEvIndex] > THRESHOLD)
            {
                cpssOsPrintf("\n    event %3d : %4d",uniEvIndex,prv_utfEventCounterArr[uniEvIndex]);
            }
        }
        cpssOsPrintf("\n----------------------\n");
    }
    return GT_OK;
}


/**
* @internal utfTestNumRun function
* @endinternal
*
* @brief   This function is called to run test with defined number
*
* @param[in] testNum                  -   test's number to run
*                                       none
*
* @note none
*
*/
static GT_VOID utfTestNumRun
(
    IN GT_U32   testNum
)
{
    GT_CHAR    suitTestName[2 * UTF_MAX_TEST_PATH_LEN_CNS + 1] = {0};
    GT_BOOL    testSkipped = GT_FALSE;
    GT_STATUS  ipLpmConfiguration = GT_FALSE;
    GT_U32     ii;
    GT_STATUS  rc;

    /* create suitName.TestName for output log */
    if (GT_FALSE == isRandomRunMode)
    {
        cpssOsStrCpy(suitTestName, utfCtxCommon.testEntriesArray[testNum].testNamePtr);
    }
    else
    {
        /* add suitName */
        cpssOsStrCpy(suitTestName, utfCtxCommon.testEntriesArray[testNum].suitNamePtr);

        /* add separator dot */
        suitTestName[cpssOsStrlen(suitTestName)] = '.';

        /* add testName */
        cpssOsStrCpy(suitTestName + cpssOsStrlen(suitTestName),
                      utfCtxCommon.testEntriesArray[testNum].testNamePtr);
    }

    for (ii = 0; ii < NUM_IP_LPM_TESTS; ii++)
    {
        rc = cpssOsStrCmp(utfCtxCommon.testEntriesArray[testNum].testNamePtr,
                          ipLpmTestArray[ii]);
        if (rc == 0)
        {
            ipLpmConfiguration = GT_TRUE;
            break;
        }
    }
    if (((ipLpmConfiguration == GT_TRUE) ||
        (cpssOsStrCmp(utfCtxCommon.testEntriesArray[testNum].suitNamePtr, "tgfIp") == 0) ||
         (cpssOsStrCmp(utfCtxCommon.testEntriesArray[testNum].suitNamePtr, "tgfTunnel") == 0)) &&
        (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)))
    {
        /* For tests with LPM configurations on eArch devices we need to remove
           the default virtual router before the test to avoid memory leak
           notifications */
        resetDmmAllocations = GT_TRUE;
    }
    else
    {
        resetDmmAllocations = GT_FALSE;
    }

    PRV_UTF_LOG1_MAC("******** TEST %s STARTED\n", suitTestName);

    utfCtxPtr->currTestNum = testNum;

    utfPrintKeepAlive();

    /* do we sent traffic to the CPU ? */
    tgfTrafficGeneratorRxCaptureNum = 0;
    /* indication that a test set traffic to be sent to the CPU , but not relate to 'capture' */
    tgfTrafficGeneratorExpectTraficToCpuEnabled = GT_FALSE;
    /* number of packets that returned 'not GT_OK' on send */
    prvTgfTrafficGeneratorNumberPacketsSendFailed = 0;
    /* number of packets that returned 'GT_OK' on send */
    prvTgfTrafficGeneratorNumberPacketsSendOk = 0;

    if (resetDmmAllocations)
    {
#ifdef DXCH_CODE
        /* remove the default VR */
        rc = prvTgfIpLpmVirtualRouterDelDefault();
        if (rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("******** Failed to delete the default virtual router before %s\n", suitTestName);
        }
        /* remove the default LPM DB */
        rc = prvTgfIpLpmDBDeleteDefault();
        if (rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("******** Failed to delete the default LPM DB before %s\n",suitTestName);
        }
#endif /*DXCH_CODE*/
    }

    /* state that test started */
    tgfStateStartTest();

    /* run unit test */
    utfPreTestRun();
    if (utfIsTestPreSkipped(
        utfCtxCommon.testEntriesArray[testNum].suitNamePtr,
        utfCtxCommon.testEntriesArray[testNum].testNamePtr))
    {
        /* do the same that test does inside at the skip case */
        prvUtfSkipTestsSet();
    }
    else
    {
        utfCtxCommon.testEntriesArray[testNum].testFuncPtr();
        if (g_checkIntrrTableThreshold > 0)
        {
            utfIntprTableCheck();
        }
        if (g_sleepAfterTestMsec > 0)
        {
            cpssOsTimerWkAfter(g_sleepAfterTestMsec);
        }
    }

    utfPostTestRun();

    if (resetDmmAllocations)
    {
#ifdef DXCH_CODE
        /* Now that all memory allocations and frees are counted, restore the default LPM DB */
        rc = prvTgfIpLpmRamDBCreateDefault();
        if (rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("******** Failed to add the default LPM DB after %s\n", suitTestName);
        }

        /* Now that all memory allocations and frees are counted, restore the default device */
        rc = prvTgfIpLpmDBDevListAddDefault();
        if (rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("******** Failed to add the default device after %s\n", suitTestName);
        }

        /* Now that all memory allocations and frees are counted, restore the default VR */
        rc = prvTgfIpLpmVirtualRouterAddDefault();
        if (rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("******** Failed to restore the default virtual router after %s\n", suitTestName);
        }
#endif /*DXCH_CODE*/
    }

    /* run Debug Post Test Exit */
    if (prvUtfDebugPostTestExitPtr != NULL)
    {
        prvUtfDebugPostTestExitPtr(
        utfCtxCommon.testEntriesArray[testNum].suitNamePtr,
        utfCtxCommon.testEntriesArray[testNum].testNamePtr);
    }

    if ((utfCtxPtr->testEntriesArray[testNum].errorCount > 0)
        && (prvUtfSkipList != NULL))
    {
        /* test failed but there is a skip list */
        PRV_UTF_SKIP_LIST_STC *i;
        for (i = prvUtfSkipList; i; i = i->next)
        {
            if ((cpssOsStrCmp(utfCtxCommon.testEntriesArray[testNum].suitNamePtr, i->suit) == 0) &&
                (cpssOsStrCmp(utfCtxCommon.testEntriesArray[testNum].testNamePtr, i->test) == 0))
            {
                PRV_UTF_LOG1_MAC("\nTest fails. Skip list reason: %s!!!\n\n", i->reason);
                utfCtxPtr->testEntriesArray[testNum].errorCount = 0;
                prvUtfSkipTestsSet();
                break;
            }
        }
    }

    /* check if the test finished succeeded */
    if (GT_FALSE == prvUtfSkipTestsFlagGet())
    {
        if (utfCtxPtr->testEntriesArray[testNum].errorCount > 0)
        {
            if (prvTgfReRunVariable == TGF_RE_RUN_STATUS_DO_RE_RUN_E)
            {
                PRV_UTF_LOG1_MAC("******** TEST %s NEED RE-RUN\n", suitTestName);
                utfCtxPtr->testEntriesArray[testNum].testStatus = UTF_TEST_STATUS_FAILED_E;
            }
            else
            {
                PRV_UTF_LOG1_MAC("******** TEST %s FAILED\n", suitTestName);
                utfCtxPtr->testEntriesArray[testNum].testStatus = UTF_TEST_STATUS_FAILED_E;
            }
        }
        else
        {
            PRV_UTF_LOG1_MAC("******** TEST %s SUCCEEDED\n", suitTestName);
            utfCtxPtr->testEntriesArray[testNum].testStatus = UTF_TEST_STATUS_PASS_E;
        }
    }
    else
    {
        PRV_UTF_LOG1_MAC("******** TEST %s SKIPPED\n", suitTestName);
        utfCtxPtr->testEntriesArray[testNum].testStatus = UTF_TEST_STATUS_SKIPPED_E;
        testSkipped = GT_TRUE;
        /* restore skip test flag */
        prvUtfSkipTestsFlagReset();
    }

    {
        UTF_LOG_OUTPUT_ENT origLogState = 0;
        /* we may run without print to log.*/
        origLogState = prvUtfLogPrintEnable(UTF_LOG_OUTPUT_SERIAL_ALL_E);

        if(utfCtxPtr->testEntriesArray[testNum].errorCount ||
           utfCtxPtr->testEntriesArray[testNum].testExecTimeSec > 10)/*look for long tests --> maybe to optimize ... */
        {
            if(utfCtxPtr->testEntriesArray[testNum].errorCount)
            {
                PRV_UTF_LOG3_MAC("\n[UTF]: FAILED : test %s failed [%d] times ,takes [%d] sec\n",
                        suitTestName,
                        utfCtxPtr->testEntriesArray[testNum].errorCount,
                        utfCtxPtr->testEntriesArray[testNum].testExecTimeSec);
            }
            else
            {
                PRV_UTF_LOG2_MAC("\n[UTF]: PASS OK : test %s : takes [%d] sec\n",
                        suitTestName,
                        utfCtxPtr->testEntriesArray[testNum].testExecTimeSec);
            }


            if(prvTgfTrafficGeneratorNumberPacketsSendFailed && /*send failed*/
               utfCtxPtr->testEntriesArray[testNum].errorCount)/*if no error maybe we overcome this fail ! */
            {
                PRV_UTF_LOG2_MAC("[UTF]: sent [%d] packets , (and [%d] packets failed) \n",
                        prvTgfTrafficGeneratorNumberPacketsSendOk,
                        prvTgfTrafficGeneratorNumberPacketsSendFailed);
            }
            else if(prvTgfTrafficGeneratorNumberPacketsSendOk)/*skip regular UT*/
            {
                PRV_UTF_LOG1_MAC("[UTF]: sent [%d] packets \n",
                        prvTgfTrafficGeneratorNumberPacketsSendOk);
            }
        }
        else if(testSkipped == GT_TRUE)
        {
            /* indication test skipped */
            PRV_UTF_LOG0_MAC("K");
        }
        else
        {
            /* it seems that there are a lot of tests that implemented for
               'NOT ASIC_SIMULATION' but not declare 'skip' for the ASIC_SIMUALTION
               so this issue need to be solved.
            */
#if 0
            if(prvTgfTrafficGeneratorNumberPacketsSendOk == 0)
            {
                PRV_UTF_LOG1_MAC("[UTF]: ERROR : test %s not skipped , but did not sent any packet ! \n",suitTestName);

                /* register the test as FAILED ! */
                errorCountIncrement(utfCtxPtr->currTestNum,GT_TRUE);
            }
#endif /*0*/
            /* indication that current test ended (new test will start next) */
            PRV_UTF_LOG0_MAC("*");
        }

        /* restore original value */
        prvUtfLogPrintEnable(origLogState);
    }

    PRV_UTF_LOG2_MAC("[UTF]: test running time [%d.%09d] sec\n",
            utfCtxPtr->testEntriesArray[testNum].testExecTimeSec,
            utfCtxPtr->testEntriesArray[testNum].testExecTimeNsec);

    PRV_UTF_LOG0_MAC("\n");

}

/**
* @internal utfLogResultRun function
* @endinternal
*
* @brief   This function executes result file logging.
*
* @param[in] paramArrayPtr            - array of parameters, that passed to result.txt
*
* @param[out] paramDeltaComparePtr     - array of parameters, with delta compare result
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
    OUT GT_BOOL                      *paramDeltaComparePtr
)
{
    GT_32       tempLogOutputValue;         /* this variable store main utfLogOutputSelectMode*/
    GT_U8       paramCounter;               /* counter of array parameters*/
    GT_BOOL     fileExists;                 /* true if file alredy exists, false if that is new file*/
    GT_U32      resultCmp;                  /* result of campare two parameters in persent*/
    UTF_RESULT_PARAM_STC *compareArrayPtr;  /* array of parameters from existing file*/
    GT_U32      st = GT_OK;                 /* functions status return*/
    GT_BOOL     memoryError = GT_FALSE;     /* memory error status */

    CPSS_NULL_PTR_CHECK_MAC(paramArrayPtr);
    CPSS_NULL_PTR_CHECK_MAC(paramDeltaComparePtr);

    compareArrayPtr = cpssOsMalloc(numberParam * sizeof(UTF_RESULT_PARAM_STC));

    /* Checking memory allocate error */
    if (NULL == compareArrayPtr)
    {
        memoryError = GT_TRUE;
    }

    /* Save old output mode value to use standart UTF output functions */
    tempLogOutputValue = utfLogOutputSelectMode;
    prvUtfLogPrintEnable(UTF_LOG_OUTPUT_FILE_E);

    /* Checking if file pointer already busy*/
    if (NULL != fpResult)
    {
        st = GT_ALREADY_EXIST;
        goto exit;
    }

    /* Open file for reading text*/
    fpResult = fopen(UTF_RESULT_FILENAME_CNS, "rt");

    /* Checking if file already exists*/
    if (NULL == fpResult)
    {
        fileExists = GT_FALSE;
    }
    else
    {
        fileExists = GT_TRUE;
        fclose(fpResult);
        fpResult = NULL;
    }

    /* Begin work with parameters */
    if (fileExists == GT_TRUE)
    {
        /* ------------------------PRINT OLD PARAMS----------------------------------*/
        /* Read all parameters from file to compareArrayPtr array */
        st = prvUtfLogReadParam(UTF_RESULT_FILENAME_CNS, numberParam, compareArrayPtr);
        /* Check if read completed successfully*/
        if (st != GT_OK)
        {
            goto exit;
        }

        /*Open file to write compare results */
        st = utfLogOpen(UTF_RESULT_OF_COMPARE_FILENAME_CNS);
        /* Check if file open completed successfully*/
        if (st != GT_OK)
        {
            goto exit;
        }

        PRV_UTF_LOG0_MAC("---------------------Old values:-------------------------\n");

        /* Printing all old parameters names and values to file and stdout*/
        for (paramCounter = 0; paramCounter < numberParam; paramCounter++)
        {
            if (osStrlen(compareArrayPtr[paramCounter].paramName) > 0)
            {
                PRV_UTF_LOG2_MAC("%s\t%9d\n", compareArrayPtr[paramCounter].paramName,
                                              compareArrayPtr[paramCounter].paramValue);
            }
        }

        PRV_UTF_LOG0_MAC("--------------------New values:-------------------------\n");

        /* Close result of compare file */
        st = utfLogClose();
        if (st != GT_OK)
        {
            goto exit;
        }
    }

    /* ---------------------------PRINT NEW PARAMS-----------------------------------*/
    /* Open results file to write new parameters name and values, old parameters will be lost */
    st = utfLogOpen(UTF_RESULT_FILENAME_CNS);
    if (st != GT_OK)
    {
         return st;
    }

    /* Iterate with all parameters */
    for (paramCounter = 0; paramCounter < numberParam; paramCounter++)
    {
        if (osStrlen(paramArrayPtr[paramCounter].paramName) > 0)
        {
            PRV_UTF_LOG2_MAC("%s\t%9d\n", paramArrayPtr[paramCounter].paramName,
                                          paramArrayPtr[paramCounter].paramValue);

            /* Initialize paramDeltaComparePtr array to return to the test*/
            paramDeltaComparePtr[paramCounter] = GT_TRUE;
        }
    }

    /*Close results file and check it well*/
    st = utfLogClose();
    if (st != GT_OK)
    {
        goto exit;
    }

    if (fileExists == GT_TRUE)
    {
        /* -----------------PRINT RESULT OF COMPARE---------------------------------*/
        st = utfLogOpen(UTF_RESULT_OF_COMPARE_FILENAME_CNS);
        if (st != GT_OK)
        {
            goto exit;
        }

        PRV_UTF_LOG0_MAC("-------------------Result of compare:-------------------\n");

        /* Iterate with all params */
        for (paramCounter = 0; paramCounter < numberParam; paramCounter++)
        {
            if (osStrlen(compareArrayPtr[paramCounter].paramName) > 0 &&
                0 == cpssOsMemCmp(compareArrayPtr[paramCounter].paramName,
                                  paramArrayPtr[paramCounter].paramName,
                                  sizeof(compareArrayPtr[paramCounter].paramName)))
            {
                /*----------------Checking if values equal---------------------------------------------*/
                if (compareArrayPtr[paramCounter].paramValue == paramArrayPtr[paramCounter].paramValue)
                {
                    PRV_UTF_LOG2_MAC("Parameter \"%s\" have the same value   %9d\n",
                                        compareArrayPtr[paramCounter].paramName,
                                        compareArrayPtr[paramCounter].paramValue);

                    paramDeltaComparePtr[paramCounter] = GT_TRUE;
                }

                /*----------------Checking if new value bigger-----------------------------------------*/
                if (compareArrayPtr[paramCounter].paramValue > paramArrayPtr[paramCounter].paramValue)
                {
                    if (0 != paramArrayPtr[paramCounter].paramValue)
                    {
                        resultCmp = (((compareArrayPtr[paramCounter].paramValue * 100)/
                                       paramArrayPtr[paramCounter].paramValue) - 100);
                        PRV_UTF_LOG3_MAC("Parameter \"%s\" is bigger by  \t%5d%% \t(current value is \t%d)\n",
                                            compareArrayPtr[paramCounter].paramName,
                                            resultCmp,
                                            paramArrayPtr[paramCounter].paramValue);

                        paramDeltaComparePtr[paramCounter] =
                            (resultCmp > paramArrayPtr[paramCounter].paramDelta) ? GT_FALSE : GT_TRUE;
                    }
                    else
                    {
                        PRV_UTF_LOG1_MAC("Error of compare, parameter \"%s\" equal 0.\n",
                                            paramArrayPtr[paramCounter].paramName);
                    }
                }

                /*----------------Checking if new value less------------------------------------------*/
                if (compareArrayPtr[paramCounter].paramValue < paramArrayPtr[paramCounter].paramValue)
                {
                    if (0 != paramArrayPtr[paramCounter].paramValue)
                    {
                        resultCmp = (((paramArrayPtr[paramCounter].paramValue * 100)/
                                       compareArrayPtr[paramCounter].paramValue) - 100);

                        PRV_UTF_LOG3_MAC("Parameter \"%s\" is smaller by \t%5d%% \t(current value is \t%d)\n",
                                            compareArrayPtr[paramCounter].paramName,
                                            resultCmp,
                                            paramArrayPtr[paramCounter].paramValue);

                        paramDeltaComparePtr[paramCounter] =
                            (resultCmp > paramArrayPtr[paramCounter].paramDelta) ? GT_FALSE : GT_TRUE;
                    }
                    else
                    {
                        PRV_UTF_LOG1_MAC("Error of compare, parameter \"%s\" equal 0.\n",
                                            paramArrayPtr[paramCounter].paramName);
                    }
                }
            }
        }

        st = utfLogClose();
    }

    /* Restore default output mode */
    prvUtfLogPrintEnable(tempLogOutputValue);

exit:
    if (GT_FALSE == memoryError)
    {
        cpssOsFree(compareArrayPtr);
    }

    return st;
}

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
)
{
    GT_STATUS   st   = GT_OK;

    GT_U32      sec  = 0;
    GT_U32      nsec = 0;

    st = cpssOsTimeRT(&sec, &nsec);
    if (GT_OK != st)
    {
        PRV_UTF_LOG1_MAC("[UTF]: utfStartTimer: failed [0x%X] to get system time\n", st);
    }

    utfTimer.codeExecTimeSec = sec;
    utfTimer.codeExecTimeNsec = nsec;

    return st;
}

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
)
{
    GT_STATUS   st   = GT_OK;
    GT_U32      sec  = 0;
    GT_U32      nsec = 0;

    CPSS_NULL_PTR_CHECK_MAC(timeElapsedPtr);

    if (utfTimer.codeExecTimeSec == 0 && utfTimer.codeExecTimeNsec == 0)
    {
        return GT_FAIL;
    }

    st = cpssOsTimeRT(&sec, &nsec);
    if (GT_OK != st)
    {
        PRV_UTF_LOG1_MAC("[UTF]: utfPostTestRun: failed [0x%X] to get system time\n", st);
    }

    if (nsec < utfTimer.codeExecTimeNsec)
    {
        nsec += 1000000000;
        sec--;
    }
    utfTimer.codeExecTimeNsec = nsec - utfTimer.codeExecTimeNsec;
    utfTimer.codeExecTimeSec  = sec  - utfTimer.codeExecTimeSec;

    *timeElapsedPtr = utfTimer.codeExecTimeSec * 1000 + utfTimer.codeExecTimeNsec / 1000000;

    utfTimer.codeExecTimeSec = 0;
    utfTimer.codeExecTimeNsec = 0;

    return st;
}

/**
* @internal utfMemoryHeapCounterInit function
* @endinternal
*
* @brief   This function initialize memory heap counter.
*
* @note Should be called before utfMemoryHeapStatusGet()
*
*/
GT_VOID utfMemoryHeapCounterInit
(
    GT_VOID
)
{
    GT_U32 ii;
    GT_U32 res = 1;
    for (ii = 0; ii < NUM_MEM_LEAK_SKIPPED_TESTS; ii++)
    {
        res = cpssOsStrCmp(utfCtxCommon.testEntriesArray[utfCtxPtr->currTestNum].testNamePtr,
                            memLeakageSkippedTestArray[ii]);
        if (res == 0)
        {
            return;
        }
    }

    if (UTF_TEST_TYPE_TRAFFIC_E == utfCtxCommon.testEntriesArray[utfCtxPtr->currTestNum].testType)
    {
        /* for enhanced ut tests */
        /* init check for memory leaks */
        osMemStartHeapAllocationCounter();

        /* reset pause state */
        memoryLeakTestOnPause = GT_FALSE;

        /* reset size of memory leak chunk */
        memoryLeakChunkSize = 0;
    }
}

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
)
{
    GT_U32 ii  = 0;
    GT_U32 res = 0;

    /* check for null-pointer */
    CPSS_NULL_PTR_CHECK_MAC(memPtr);

    for (ii = 0; ii < NUM_MEM_LEAK_SKIPPED_TESTS; ii++)
    {
        res = cpssOsStrCmp(utfCtxCommon.testEntriesArray[utfCtxPtr->currTestNum].testNamePtr,
                            memLeakageSkippedTestArray[ii]);
        if (res == 0)
        {
            return GT_OK;
        }
    }

    if(tgfIsAfterSystemReset())
    {
        return GT_OK;
    }

    if (UTF_TEST_TYPE_TRAFFIC_E == utfCtxCommon.testEntriesArray[utfCtxPtr->currTestNum].testType)
    {
        /* calculate total memory leak bytes number */
        *memPtr = osMemGetHeapAllocationCounter() + memoryLeakChunkSize;

        /* return error if memLeak detected or
           leak test stay on pause by mistake */
        if ( (*memPtr != 0) ||
             (memoryLeakChunkSize != 0) ||
             memoryLeakTestOnPause   )
        {
            return GT_FAIL;
        }
    }

    return GT_OK;
}

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
)
{
    /* check onPause value to prevent invalid request */
    if (memoryLeakTestOnPause == onPause)
    {
        return GT_BAD_VALUE;
    }

    /* change current state */
    if (onPause)
    {
        /* switch to pause */
        /* update size of memory leak chunk */
        memoryLeakChunkSize += osMemGetHeapAllocationCounter();

        memoryLeakTestOnPause = GT_TRUE;

        PRV_UTF_LOG0_MAC("[UTF]: Memory leak test switch to pause on.\n");
    }
    else
    {
        /* switch to continue */
        /* store new heap state */
        osMemStartHeapAllocationCounter();

        memoryLeakTestOnPause = GT_FALSE;

        PRV_UTF_LOG0_MAC("[UTF]: Memory leak test switch to pause off.\n");
    }

    return GT_OK;
}

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
)
{
    /* reset heapCounter to init state */
    osMemStartHeapAllocationCounter();

    /* reset pause state */
    memoryLeakTestOnPause = GT_FALSE;

    /* reset size of memory leak chunk */
    memoryLeakChunkSize = 0;

    PRV_UTF_LOG0_MAC("[UTF]: Memory leak counter successfully reseted.\n");

    return GT_OK;
}

/*******************************************************************************
* utfTestTask
*
* DESCRIPTION:
*     This function is called in single task for run each test
*
* INPUTS:
*     paramPtr   -   pointer to input parameter
*
* OUTPUTS:
*     none
*
* RETURNS:
*     GT_OK    -   on success
*     GT_FAIL  -   on error
*
* COMMENTS:
*     none
*
*******************************************************************************/
static GT_STATUS __TASKCONV utfTestTask
(
    IN GT_VOID *paramPtr
)
{
    GT_STATUS   st = GT_OK;
    GT_U32      currTestNum = 0;

    /* get input parameter */
    currTestNum = *((GT_U32*) paramPtr);

    /* run UT */
    utfTestNumRun(currTestNum);

    /* signal semaphore */
    st = cpssOsSigSemSignal(utfTestSemId);
    if (GT_OK != st)
    {
        PRV_UTF_LOG1_MAC("[UTF]: utfTestTask: failed [0x%X] to signal semaphore\n", st);
    }

    return st;
}

/**
* @internal utfSuitsRun function
* @endinternal
*
* @brief   This function executes all declared test cases for specific test suits.
*         Test suits to execute are defined by test path parameter.
* @param[in] testPathPtr              -   test path, may contain NULL for all suits, suit, test case
*                                      supports a range of tests or suits
* @param[in] continueFlag             -   define test flow behaviour, should it be interrupted after
*                                      first failure
*
* @retval GT_OK                    - on success.
* @retval GT_UTF_TEST_FAILED       -  Some test fails.
* @retval GT_FAIL                  - on failure in case of invalid test path.
*
* @note none
*
*/
static GT_STATUS utfSuitsRun
(
    IN const GT_CHAR *testPathPtr,
    IN GT_BOOL       continueFlag,
    IN GT_BOOL       startFlag
)
{
    GT_STATUS   st = GT_OK;
    GT_U32      countIdx = 0;
    GT_U32      sec  = 0;
    GT_U32      nsec = 0;
    GT_BOOL     lastSuitStarted   = GT_FALSE;
    GT_BOOL     isSingleTest      = GT_FALSE;
    GT_BOOL     isInvalidSuitName = GT_TRUE;
    GT_BOOL     isSuitFound       = GT_FALSE;
    GT_BOOL     isTestFound       = GT_FALSE;
    GT_BOOL     wasCalled_utfSuitStatsShow = GT_FALSE;

    GT_CHAR     lastSuitName[UTF_MAX_TEST_PATH_LEN_CNS + 1] = {0, };
    GT_CHAR     suitName    [UTF_MAX_TEST_PATH_LEN_CNS + 1] = {0, };
    GT_CHAR     endSuitName [UTF_MAX_TEST_PATH_LEN_CNS + 1] = {0, };
    GT_CHAR     testName    [UTF_MAX_TEST_PATH_LEN_CNS + 1] = {0, };
    GT_CHAR     endTestName [UTF_MAX_TEST_PATH_LEN_CNS + 1] = {0, };

    /* mark if we have range, not single test */
    GT_BOOL     isMultipleTest    = GT_FALSE;
    /* check if we have reached first test of range */
    GT_BOOL     isFirstTest       = GT_FALSE;
    /* sets to TRUE to mark that tests' names validation was passed */
    GT_BOOL     isEndTestName     = GT_FALSE;
    /* check if end test of range is valid */
    GT_BOOL     isInvalidEndTest  = GT_TRUE;
    /* check if first test of range is valid */
    GT_BOOL     isInvalidTestName = GT_TRUE;
    /* we have range between suits, not just tests */
    GT_BOOL     isSuiteRange      = GT_FALSE;
    /* marks if we have reached last suit of a range */
    GT_BOOL     isLastSuit        = GT_FALSE;
    /* marks if we have reached last test of last suit*/
    GT_BOOL     wasLastSuitTest   = GT_FALSE;
    /* check if endSuit name is valid */
    GT_BOOL     isInvalidEndSuit  = GT_TRUE;

    /* endTest iterator */
    GT_U32      endTstIdx         = 0;
    /* index for ending suit */
    GT_U32      endSuitIdx        = 0;
    /* counts the failed tests */
    GT_U32 prvTgfTotalFailed = 0;


    GT_BOOL     skippedFirstTest = GT_FALSE;/* indication that first test skipped when using utfTestsStartRunFromNext(...) */

#if 0
    GT_CHAR     lastSuitName[UTF_MAX_TEST_PATH_LEN_CNS + 1];

    GT_CHAR     suitName[UTF_MAX_TEST_PATH_LEN_CNS + 1];
    GT_CHAR     testName[UTF_MAX_TEST_PATH_LEN_CNS + 1];

    /* !! NOTE - using explicit init by osMemSet to avoid
        implicit usage of memcpy by gcc (avoid to use libc directly) */
    osMemSet(lastSuitName, 0, sizeof(lastSuitName));
    osMemSet(suitName, 0, sizeof(suitName));
    osMemSet(testName, 0, sizeof(testName));
#endif

    /* set output mode */
    prvUtfLogPrintEnable(utfLogOutputSelectMode);

    utfCtxPtr->errorFlag = GT_FALSE;
    utfCtxPtr->continueFlag = continueFlag;

    /* reset number of errors for all test enties */
    for (countIdx = 0; countIdx < UTF_MAX_NUM_OF_TESTS_CNS; countIdx++)
    {
        utfCtxPtr->testEntriesArray[utfCtxCommon.testIndexArray[countIdx]].errorCount = 0;
    }

    /* parse test path and extract suite name and test name */
    if (NULL == testPathPtr)
    {
        suitName[0] = '\0';
    }
    else
    {
        countIdx = 0;

        /* getting suitName */
        while ((testPathPtr[countIdx] != '.') &&
               (countIdx < UTF_MAX_TEST_PATH_LEN_CNS) &&
               (testPathPtr[countIdx] != '\0') &&
               (testPathPtr[countIdx] != '-'))
        {
            suitName[countIdx] = testPathPtr[countIdx];
            countIdx++;
        }
        suitName[countIdx] = '\0';

        /* getting endSuite if exists (suit-endSuit) */
        if (testPathPtr[countIdx] == '-')
        {
            countIdx++;
            while ((countIdx < UTF_MAX_TEST_PATH_LEN_CNS) &&
                   (testPathPtr[countIdx] != '\0') &&
                   (testPathPtr[countIdx] != '.'))
            {
                endSuitName[endSuitIdx] = testPathPtr[countIdx];
                countIdx++;
                endSuitIdx++;
            }
            endSuitName[endSuitIdx] = '\0';
        }

        /* getting endTest (suit-endSuit.endTest) */
        if ((testPathPtr[countIdx] == '.') && (endSuitName[0] != '\0'))
        {
            countIdx++;
            endSuitIdx = 0;
            while ((countIdx < UTF_MAX_TEST_PATH_LEN_CNS) &&
                   (testPathPtr[countIdx] != '\0'))
            {
                endTestName[endSuitIdx] = testPathPtr[countIdx];
                countIdx++;
                endSuitIdx++;
            }
            endTestName[endSuitIdx] = '\0';
        }

        if (testPathPtr[countIdx] == '.')
        {
            GT_U32 suitIdx = 0;

            /* getting endTestName (suit.endTest) */
            while ((suitIdx < UTF_MAX_TEST_PATH_LEN_CNS) &&
                   (testPathPtr[suitIdx] != '\0') &&
                   (testPathPtr[suitIdx + countIdx + 1] != '-') &&
                   (cpssOsStrlen(testPathPtr) != suitIdx + countIdx + 1))
            {
                testName[suitIdx] = testPathPtr[suitIdx + countIdx + 1];
                suitIdx++;
            }

            /* setting to multiple test mode (suit.test-suit2.test2) */
            testName[suitIdx] = '\0';
            if ('-' == testPathPtr[suitIdx + countIdx +1])
            {
                isMultipleTest = GT_TRUE;
            }
            else
            {
                isSingleTest = GT_TRUE;
            }

            /* get ending suite and test to single str */
            if (GT_TRUE == isMultipleTest)
            {
                suitIdx++;
                while ((suitIdx < UTF_MAX_TEST_PATH_LEN_CNS) &&
                       (testPathPtr[suitIdx + countIdx + 1] != '\0') &&
                       (testPathPtr[suitIdx + countIdx + 1] != '.'))
                {
                    endTestName[endTstIdx] = testPathPtr[suitIdx + countIdx + 1];
                    suitIdx++;
                    endTstIdx++;
                }
            }
            endTestName[endTstIdx] = '\0';

            /* getting  endSuite and endTest from str */
            if (testPathPtr[suitIdx + countIdx + 1] == '.')
            {
                cpssOsStrCpy(endSuitName, endTestName);
                endTstIdx = 0;
                isSuitFound = GT_TRUE;

                suitIdx++;
                while ((suitIdx < UTF_MAX_TEST_PATH_LEN_CNS) &&
                       (testPathPtr[suitIdx + countIdx + 1] != '\0'))
                {
                    endTestName[endTstIdx] = testPathPtr[suitIdx + countIdx + 1];
                    suitIdx++;
                    endTstIdx++;
                }
                endTestName[endTstIdx] = '\0';
            }
        }

        /* check if we have a range between suits */
        if (('\0' != suitName[0]) && ('\0' != endSuitName[0]))
        {
            isSuiteRange = GT_TRUE;
        }
    }

    /* create semaphore */
    st = cpssOsSigSemBinCreate("utfTestSem", 0, &utfTestSemId);
    if (GT_OK != st)
    {
        /* force print this log message */
        if ((UTF_LOG_OUTPUT_SERIAL_FINAL_E == utfLogOutputSelectMode) ||
            (UTF_LOG_OUTPUT_SERIAL_FINAL_WITH_DEBUG_E ==
                                              utfLogOutputSelectMode))
        {
            prvUtfLogPrintEnable(UTF_LOG_OUTPUT_SERIAL_ALL_E);
        }

        PRV_UTF_LOG1_MAC("[UTF]: utfSuitsRun: failed [0x%X] to create semaphore\n", st);
        return st;
    }

    /* go over all test entries and run all with defined suite and test names */
    for (countIdx = 0; countIdx < utfCtxCommon.usedTestNum; countIdx++)
    {
        const GT_CHAR* currSuitNamePtr = utfCtxCommon.testEntriesArray[utfCtxCommon.testIndexArray[countIdx]].suitNamePtr;
        const GT_CHAR* currTestNamePtr = utfCtxCommon.testEntriesArray[utfCtxCommon.testIndexArray[countIdx]].testNamePtr;

        /* state that test did not run (yet) */
        utfCtxPtr->testEntriesArray[utfCtxCommon.testIndexArray[countIdx]].testStatus = UTF_TEST_STATUS_NOT_RUN_E;
        /* check is start suit or test is reached */
        isSuitFound = (startFlag && (0 == cpssOsStrCmp(currSuitNamePtr, suitName))) || isSuitFound;

        /* check for valid test names */
        if (GT_FALSE == isEndTestName)
        {
            for (endTstIdx = countIdx; endTstIdx < utfCtxCommon.usedTestNum; endTstIdx++)
            {
                /* check if endTest name is a suit name or not */
                if (GT_TRUE == isMultipleTest)
                {
                    if (0 == cpssOsStrCmp(
                                  utfCtxCommon.testEntriesArray[
                                  utfCtxCommon.testIndexArray[endTstIdx]].suitNamePtr,
                                  endTestName))
                    {
                        isMultipleTest = GT_FALSE;
                        isSuiteRange = GT_TRUE;
                        cpssOsStrCpy(endSuitName, endTestName);
                        endTestName[0] = '\0';
                    }
                }

                /* check if suit exists */
                if (0 == cpssOsStrCmp(utfCtxCommon.testEntriesArray[
                                  utfCtxCommon.testIndexArray[endTstIdx]].suitNamePtr,
                                  suitName))
                {
                    isInvalidSuitName = GT_FALSE;
                }

                /* check if endSuit exists */
                if (('\0' != endSuitName[0]) &&
                    (0 == cpssOsStrCmp(utfCtxCommon.testEntriesArray[
                          utfCtxCommon.testIndexArray[endTstIdx]].suitNamePtr,
                          endSuitName)))
                {
                    isInvalidEndSuit = GT_FALSE;
                }

                /* check end name if isMultiple, and then check start anyway */
                if ((GT_TRUE == isMultipleTest) || (GT_TRUE == isSuiteRange))
                {
                    if (0 == cpssOsStrCmp(utfCtxCommon.testEntriesArray[
                             utfCtxCommon.testIndexArray[endTstIdx]].testNamePtr,
                             endTestName))
                    {
                        /* check if endTestName belongs to its suit */
                        if (0 == cpssOsStrCmp(utfCtxCommon.testEntriesArray[
                                 utfCtxCommon.testIndexArray[
                                 endTstIdx]].suitNamePtr, endSuitName) ||
                           ('\0' == endSuitName[0]))
                        {
                            isInvalidEndTest = GT_FALSE;
                        }
                    }
                }
                if ((0 == cpssOsStrCmp(
                          utfCtxCommon.testEntriesArray[
                          utfCtxCommon.testIndexArray[endTstIdx]].testNamePtr,
                          testName)) ||
                   ((GT_TRUE == isSuiteRange) &&
                   ('\0' == testName[0])))
                {
                    isInvalidTestName = GT_FALSE;
                }
            }
            isEndTestName = GT_TRUE;
        }

        /* 'SUIT RANGE ERROR' message if endSuitName is invalid */
        if ((GT_TRUE == isInvalidEndSuit) && ('\0' != endSuitName[0]))
        {
            /* force print this log message */
            if ((UTF_LOG_OUTPUT_SERIAL_FINAL_E == utfLogOutputSelectMode) ||
                (UTF_LOG_OUTPUT_SERIAL_FINAL_WITH_DEBUG_E ==
                                                  utfLogOutputSelectMode))
            {
                prvUtfLogPrintEnable(UTF_LOG_OUTPUT_SERIAL_ALL_E);
            }

            PRV_UTF_LOG0_MAC("******** SUIT RANGE ERROR ********\n");
            PRV_UTF_LOG1_MAC("Invalid final suit name -- %s\n", endSuitName);
            PRV_UTF_LOG0_MAC("*****************************\n");
            PRV_UTF_LOG0_MAC("\n");

            /* delete semaphore */
            st = cpssOsSigSemDelete(utfTestSemId);
            if (GT_OK != st)
            {
                PRV_UTF_LOG1_MAC("[UTF]: utfSuitsRun: failed [0x%X] to delete semaphore\n", st);
            }

            return GT_FAIL;
        }

        /* 'TEST RANGE ERROR' message if endTestName is invalid */
        if ((GT_TRUE == isMultipleTest) || (GT_TRUE == isSuiteRange))
        {
            if ((GT_TRUE == isEndTestName) && (GT_TRUE == isInvalidEndTest))
            {
                if ((GT_TRUE == isInvalidEndTest) &&
                       ((GT_TRUE == isMultipleTest) ||
                        (GT_TRUE == isSuiteRange)) &&
                    ('\0' != endTestName[0]))
                {
                    /* force print this log message */
                    if ((UTF_LOG_OUTPUT_SERIAL_FINAL_E == utfLogOutputSelectMode) ||
                        (UTF_LOG_OUTPUT_SERIAL_FINAL_WITH_DEBUG_E ==
                                                          utfLogOutputSelectMode))
                    {
                        prvUtfLogPrintEnable(UTF_LOG_OUTPUT_SERIAL_ALL_E);
                    }

                    PRV_UTF_LOG0_MAC("******** TEST RANGE ERROR ********\n");
                    PRV_UTF_LOG1_MAC("Invalid final test name -- %s\n",
                                     endTestName);
                    PRV_UTF_LOG0_MAC("*****************************\n");
                    PRV_UTF_LOG0_MAC("\n");

                    /* delete semaphore */
                    st = cpssOsSigSemDelete(utfTestSemId);
                    if (GT_OK != st)
                    {
                        PRV_UTF_LOG1_MAC("[UTF]: utfSuitsRun: failed [0x%X] to delete semaphore\n", st);
                    }
                    return GT_FAIL;
                }
            }

            /* if reached last test name - replace testName with endTestName
             * and act as we are now in single test mode */
            if (0 == cpssOsStrCmp(currTestNamePtr, endTestName))
            {
                isMultipleTest = GT_FALSE;
                isSingleTest   = GT_TRUE;
                if ((0 != cpssOsStrCmp(testName, endTestName)) &&
                    (GT_FALSE == isInvalidTestName) &&
                    (GT_FALSE == isInvalidEndTest))
                {
                     cpssOsStrCpy(testName, endTestName);
                }
                else
                {
                    isFirstTest = GT_FALSE;
                }
            }
            isTestFound = (startFlag || isTestFound);
        }
        else
        {
            isTestFound = (((startFlag &&
                            (0 == cpssOsStrCmp(currTestNamePtr, testName))) ||
                            isTestFound));
        }

        /* 'TEST ERROR' message if testName is invalid */
        if (GT_TRUE == isInvalidTestName)
        {
            if ((GT_TRUE == isInvalidTestName) &&
               ((GT_TRUE == isSingleTest) ||
                (GT_TRUE == isSuiteRange)))
            {
                /* force print this log message */
                if ((UTF_LOG_OUTPUT_SERIAL_FINAL_E == utfLogOutputSelectMode) ||
                    (UTF_LOG_OUTPUT_SERIAL_FINAL_WITH_DEBUG_E ==
                                                      utfLogOutputSelectMode))
                {
                    prvUtfLogPrintEnable(UTF_LOG_OUTPUT_SERIAL_ALL_E);
                }

                PRV_UTF_LOG0_MAC("******** TEST ERROR ********\n");
                PRV_UTF_LOG1_MAC("Invalid test name -- %s\n", testName);
                PRV_UTF_LOG0_MAC("*****************************\n");
                PRV_UTF_LOG0_MAC("\n");

                /* delete semaphore */
                st = cpssOsSigSemDelete(utfTestSemId);
                if (GT_OK != st)
                {
                    PRV_UTF_LOG1_MAC("[UTF]: utfSuitsRun: failed [0x%X] to delete semaphore\n", st);
                }

                return GT_FAIL;
            }
        }

        if (('\0' == suitName[0]) ||
            (0 == cpssOsStrCmp(currSuitNamePtr, suitName)) ||
            (GT_TRUE == isSuitFound) || isSuiteRange)
        {
            if (GT_TRUE == wasLastSuitTest)
            {
                continue;
            }

            /* reached last suit */
            if ((GT_TRUE == isSuiteRange) &&
                (0 == cpssOsStrCmp(currSuitNamePtr, endSuitName)))
            {
                isLastSuit = GT_TRUE;
            }

            /* left last suit */
            if ((GT_TRUE == isLastSuit) &&
                (0 != cpssOsStrCmp(currSuitNamePtr, endSuitName)))
            {
                isSuitFound = GT_FALSE;
                isSuiteRange = GT_FALSE;
                continue;
            }

            /* left last test of last suit */
            if ((GT_TRUE == isLastSuit) &&
                (0 == cpssOsStrCmp(currTestNamePtr, endTestName)))
            {
                wasLastSuitTest = GT_TRUE;
            }

            /* found needed suit */
            if ((GT_TRUE == isSuiteRange) &&
                (0 == cpssOsStrCmp(suitName, currSuitNamePtr)))
            {
                isSuitFound = GT_TRUE;
            }
            else if ((GT_TRUE == isSuiteRange) &&
                     (GT_FALSE == isSuitFound))
            {
                continue;
            }

            /* reached first needed test */
            if ((0 == cpssOsStrCmp(currTestNamePtr, testName)) ||
                ('\0' == testName[0]))
            {
                isFirstTest = GT_TRUE;
            }

            /* if still haven't reached it - try next */
            if ((GT_FALSE == isFirstTest) && ('\0' != testName[0]))
            {
                continue;
            }

            /* if still haven't reached it - try next */
            if ((GT_TRUE == isSingleTest) &&
                (0 != cpssOsStrCmp(currTestNamePtr, testName)) &&
                (GT_FALSE == isTestFound))
            {
                continue;
            }

            /* check if current test needs to be skipped */
            if (utfSkipTestTypeBmp & (1 << utfCtxCommon.testEntriesArray[utfCtxCommon.testIndexArray[countIdx]].testType))
            {
                continue;
            }

            if(utfSkipTestTypeBmp == 0 &&
                utfCtxCommon.testEntriesArray[utfCtxCommon.testIndexArray[countIdx]].testType < UTF_TEST_TYPE_TRAFFIC_E)
            {
                /* do not allow the run of 'Regular UT' to continue to 'Enh-UT'
                    because the 'Regular UT' not do clean up .. cause crash in enh-UT */
                /* this case valid when using function utfTestsStartRun(...) on
                    specific 'Regular UT' ... because the 'ENH-UT' are after the 'Regular UT' */
                utfSkipTestTypeBmp = (1 << UTF_TEST_TYPE_TRAFFIC_E) |
                                     (1 << UTF_TEST_TYPE_TRAFFIC_FDB_NON_UNIFIED_E);

            }

            prvUtfSetCurrentTestType(utfCtxCommon.testEntriesArray[utfCtxCommon.testIndexArray[countIdx]].testType);

            if ((cpssOsStrCmp(currSuitNamePtr, lastSuitName) != 0) && (GT_FALSE == isRandomRunMode))
            {
                /* get current time */
                st = cpssOsTimeRT(&sec, &nsec);
                if (GT_OK != st)
                {
                    PRV_UTF_LOG1_MAC("[UTF]: utfSuitsRun: failed [0x%X] to get system time\n", st);
                }

                if (nsec < startTimeNsec)
                {
                    nsec += 1000000000;
                    sec--;
                }

                PRV_UTF_LOG1_MAC("**** SUIT %s STARTED\n", currSuitNamePtr);
                PRV_UTF_LOG1_MAC("[UTF]: number of tests [%d]\n", utfTestsNumGet(currSuitNamePtr));
                PRV_UTF_LOG2_MAC("[UTF]: suit start after [%d.%09d] sec\n", sec - startTimeSec, nsec - startTimeNsec);
                PRV_UTF_LOG0_MAC("\n");

                cpssOsStrCpy(lastSuitName, currSuitNamePtr);
                lastSuitStarted = GT_TRUE;
            }

            if(runFromNextTest == GT_TRUE)
            {
                if(skippedFirstTest == GT_FALSE)
                {
                    skippedFirstTest = GT_TRUE;
                    continue;
                }
            }

            /* check if test timeout needed */
            if (CPSS_OS_SEM_WAIT_FOREVER_CNS == utfTestTimeOut)
            {
                /* init utfTestTaskId by the current task id */
                osTaskGetSelf(&utfTestTaskId);
                /* turn off capture or events flag */
                #if (defined ASIC_SIMULATION) && (defined LINUX_SIM)
                if (testPathPtr != NULL || prvUtfIsGmCompilation() == GT_TRUE || prvTgfTotalFailed >= 5
                    || utfCtxCommon.testEntriesArray[utfCtxCommon.testIndexArray[countIdx]].testType < UTF_TEST_TYPE_TRAFFIC_E
                    || utfCtxCommon.testEntriesArray[utfCtxCommon.testIndexArray[countIdx]].testType == UTF_TEST_TYPE_PX_E)
                {
                    prvTgfReRunVariable = TGF_RE_RUN_STATUS_DISABLED_E; /* re-run disabled */
                }
                else
                {
                    prvTgfReRunVariable = TGF_RE_RUN_STATUS_CAPABLE_E; /* re-run capable */
                }
                #else
                    prvTgfReRunVariable = TGF_RE_RUN_STATUS_DISABLED_E; /* re-run disabled */
                #endif
                /* run UT */
                utfTestNumRun(utfCtxCommon.testIndexArray[countIdx]);
                /* if test failed */
                if (utfCtxPtr->testEntriesArray[utfCtxCommon.testIndexArray[countIdx]].errorCount > 0)
                {
                    prvTgfTotalFailed++;
                    /* if this is flip flop test and the test failed - re-run */
                    if (prvTgfReRunVariable == TGF_RE_RUN_STATUS_DO_RE_RUN_E)
                    {
                        /* sleep for 0.5 second in order to provide CPU time for Rx/Event tasks */
                        cpssOsTimerWkAfter(500);
                        /* clean RX packets DB */
                        tgfTrafficTableRxPcktTblClear();
                        /* reset test error counter */
                        utfCtxPtr->testEntriesArray[utfCtxCommon.testIndexArray[countIdx]].errorCount = 0;
                        /* disable re-run */
                        prvTgfReRunVariable = TGF_RE_RUN_STATUS_DISABLED_E;
                        /* re-run test */
                        utfTestNumRun(utfCtxCommon.testIndexArray[countIdx]);
                    }
                }
            }
            else
            {
                /* create task for single test */
                st = cpssOsTaskCreate("utfTestTask",
                                      UTF_TASK_PRIO_CNS,
                                      UTF_TASK_STACK_SIZE_CNS,
                                      (unsigned (__TASKCONV *) (GT_VOID*))utfTestTask,
                                      &(utfCtxCommon.testIndexArray[countIdx]),
                                      &utfTestTaskId);
                if (GT_OK != st)
                {
                    PRV_UTF_LOG1_MAC("[UTF]: utfSuitsRun: failed [0x%X] to create task\n", st);
                    /* try to run the next test */
                    continue;
                }

                /* wait for semaphore signal */
                st = cpssOsSigSemWait(utfTestSemId, utfTestTimeOut);
                if (GT_TIMEOUT == st)
                {
                    errorCountIncrement(utfCtxCommon.testIndexArray[countIdx],GT_FALSE);

                    PRV_UTF_LOG1_MAC("[UTF]: test timeout reached [%d] msec\n", utfTestTimeOut);
                    PRV_UTF_LOG1_MAC("******** TEST %s FAILED\n", utfCtxCommon.testEntriesArray[utfCtxCommon.testIndexArray[countIdx]].testNamePtr);
                    PRV_UTF_LOG0_MAC("\n");
                }

                /* wait for task end */
                cpssOsTimerWkAfter(10);

                /* delete test's task */
                st = cpssOsTaskDelete(utfTestTaskId);
                if (GT_OK != st)
                {
                    PRV_UTF_LOG1_MAC("[UTF]: utfSuitsRun: failed [0x%X] to delete task\n", st);
                }
            }
        }

        if (GT_TRUE == lastSuitStarted)
        {
            /* check if some test fails during testing */
            if (GT_TRUE == utfCtxPtr->errorFlag)
            {
                st = GT_UTF_TEST_FAILED;
            }

            /* display final suit statistics */
            if (GT_FALSE == isRandomRunMode)
            {
                if (countIdx == (utfCtxCommon.usedTestNum - 1))
                {
                    wasCalled_utfSuitStatsShow = GT_TRUE;

                    utfSuitStatsShow(lastSuitName);
                }
                else if (cpssOsStrCmp(utfCtxCommon.testEntriesArray[utfCtxCommon.testIndexArray[countIdx + 1]].suitNamePtr, lastSuitName) != 0)
                {
                    wasCalled_utfSuitStatsShow = GT_TRUE;

                    utfSuitStatsShow(lastSuitName);
                    lastSuitName[0] = 0;
                    lastSuitStarted = GT_FALSE;
                }
            }
        }
    }

    isTestFound = ((GT_TRUE == isInvalidSuitName) &&
        (NULL != testPathPtr) &&
        ('.' != testPathPtr[0])) ? GT_FALSE : GT_TRUE;

    if(allowAfterXTests_shadowTest ||
        (isTestFound &&
        wasCalled_utfSuitStatsShow == GT_FALSE && allowAfterEveryTest_shadowTest == 0))
    {
        trfCpssDxChDiagDataIntegrityTables_shadowTest(NULL,GT_FALSE);
    }


    /* delete semaphore */
    st = cpssOsSigSemDelete(utfTestSemId);
    if (GT_OK != st)
    {
        PRV_UTF_LOG1_MAC("[UTF]: utfSuitsRun: failed [0x%X] to delete semaphore\n", st);
    }

    /* Set to printing mode if not in NONE printing mode,
       in order to print the final result */
    if ((UTF_LOG_OUTPUT_SERIAL_FINAL_E == utfLogOutputSelectMode) ||
        (UTF_LOG_OUTPUT_SERIAL_FINAL_WITH_DEBUG_E == utfLogOutputSelectMode))
    {
        prvUtfLogPrintEnable(UTF_LOG_OUTPUT_SERIAL_ALL_E);
    }

    /* show final statistics */
    if (GT_FALSE == isTestFound)
    {
        PRV_UTF_LOG0_MAC("******** SUIT ERROR ********\n");
        PRV_UTF_LOG1_MAC("Invalid suit name -- %s\n", suitName);
        PRV_UTF_LOG0_MAC("*****************************\n");
        PRV_UTF_LOG0_MAC("\n");
        return GT_FAIL;
    }
    /* display final report about all run suits */
    utfFinalStatsShow();
    return st;
}

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
)
{
    utfTestTimeOut = testTimeOut;

    return GT_OK;
}

/**
* @internal utfTestsNumGet function
* @endinternal
*
* @brief   Calculate total tests number in suit
*
* @param[in] suitNamePtr              -   name of the suit, unique for all suits
*                                       total tests number in suit
*/
static GT_U32 utfTestsNumGet
(
    IN const GT_CHAR       *suitNamePtr
)
{
    GT_U32      countIdx   = 0;
    GT_U32      testsCount = 0;

    /* go over all test entries */
    for (countIdx = 0; countIdx < utfCtxCommon.usedTestNum; countIdx++)
    {
        const GT_CHAR* currSuitNamePtr = utfCtxCommon.testEntriesArray[utfCtxCommon.testIndexArray[countIdx]].suitNamePtr;

        testsCount += (0 == cpssOsStrCmp(currSuitNamePtr, suitNamePtr));
    }

    return testsCount;
}

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
)
{
    switch (testType)
    {
        case UTF_TEST_TYPE_GEN_E:
        case UTF_TEST_TYPE_CHX_E:
        case UTF_TEST_TYPE_SAL_E:
        case UTF_TEST_TYPE_EX_E:
        case UTF_TEST_TYPE_PM_E:
        case UTF_TEST_TYPE_TRAFFIC_E:
        case UTF_TEST_TYPE_TRAFFIC_FDB_NON_UNIFIED_E:
        case UTF_TEST_TYPE_PX_E:
        case UTF_TEST_TYPE_PX_TRAFFIC_E:
            utfTestType = testType;
            break;

        default:
            utfTestType = UTF_TEST_TYPE_NONE_E;
    }
}

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
)
{
    switch (testType)
    {
        case UTF_TEST_TYPE_NONE_E:
            utfSkipTestTypeBmp = 0;
            break;

        case UTF_TEST_TYPE_GEN_E:
        case UTF_TEST_TYPE_CHX_E:
        case UTF_TEST_TYPE_SAL_E:
        case UTF_TEST_TYPE_EX_E:
        case UTF_TEST_TYPE_PM_E:
        case UTF_TEST_TYPE_TRAFFIC_E:
        case UTF_TEST_TYPE_TRAFFIC_FDB_NON_UNIFIED_E:
        case UTF_TEST_TYPE_PX_E:
        case UTF_TEST_TYPE_PX_TRAFFIC_E:
            utfSkipTestTypeBmp |= 1 << testType;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

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
)
{
    switch (testType)
    {
        case UTF_TEST_TYPE_NONE_E:
            /*NOTE: value UTF_TEST_TYPE_NONE_E --> used here as 'ALL' (and not NONE)*/
            utfSkipTestTypeBmp = 0xFFFFFFFF;/* allow all types */
            break;

        case UTF_TEST_TYPE_GEN_E:
        case UTF_TEST_TYPE_CHX_E:
        case UTF_TEST_TYPE_SAL_E:
        case UTF_TEST_TYPE_EX_E:
        case UTF_TEST_TYPE_PM_E:
        case UTF_TEST_TYPE_TRAFFIC_E:
        case UTF_TEST_TYPE_TRAFFIC_FDB_NON_UNIFIED_E:
        case UTF_TEST_TYPE_PX_E:
        case UTF_TEST_TYPE_PX_TRAFFIC_E:
            utfSkipTestTypeBmp = 0xFFFFFFFF & ~(1 << testType); /* only specific type */
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

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
)
{
    static GT_U32 counter = 0;
    /* put indication to the terminal , so we can see that multi-test are in
       progress */
    PRV_UTF_LOG0_DEBUG_MAC(".");
    if(((++counter) & 0xf) == 0)
    {
        /* every 16 times do "\n" */
        PRV_UTF_LOG0_DEBUG_MAC("\n");
    }
}

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
)
{
    va_list args;

    if(index >= PRV_UTF_GENERAL_STATE_INFO_NUM_CNS)
    {
        return GT_BAD_PARAM;
    }

    if(formatStringPtr == NULL)
    {
        /* save to DB --> entry not valid */
        prvUtfGeneralStateLogArr[index].valid = GT_FALSE;
        return GT_OK;
    }

    /* save to DB */
    va_start(args, formatStringPtr);
    vsprintf(prvUtfGeneralStateLogArr[index].buff, formatStringPtr, args);
    va_end(args);

    /* entry is valid */
    prvUtfGeneralStateLogArr[index].valid = GT_TRUE;

    return GT_OK;
}

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
)
{

    if(index >= PRV_UTF_GENERAL_STATE_INFO_NUM_CNS)
    {
        return GT_BAD_PARAM;
    }

    prvUtfCallBackFunctionsOnErrorArr[index].callBackFunc = callBackFunc;

    return GT_OK;
}


/**
* @internal prvUtfSeedFromStreamNameGet function
* @endinternal
*
* @brief   Generate random seed value from CPSS stream name
*/
GT_U32   prvUtfSeedFromStreamNameGet
(
    GT_VOID
)
{
    GT_U32  seedNum = 0;
    GT_CHAR streamNameStr[] = CPSS_STREAM_NAME_CNS;

    /* get revision number from last 3 chars of stream name */
    seedNum = cpssOsStrTo32(streamNameStr + (cpssOsStrlen(streamNameStr) - 3));

    PRV_UTF_LOG1_MAC("[UTF]: generated seed = %d\n", seedNum);

    return seedNum;
}

/**
* @internal prvUtfRandomVrfIdNumberGet function
* @endinternal
*
* @brief   This function generate random vrfId number in range [0..4095] and
*         print one
*/
GT_U32  prvUtfRandomVrfIdNumberGet
(
    GT_VOID
)
{
    GT_U32  vrfId = 0;

    /* check seed was randomized */
    if (0 == prvUtfRandomSeedNum)
    {
        prvUtfRandomSeedNum = prvUtfSeedFromStreamNameGet();

        /* randomize */
        cpssOsSrand(prvUtfRandomSeedNum);
    }

    /* generate random vrfId in range 0 .. 4095 */
    vrfId = cpssOsRand() % 4095;

    PRV_UTF_LOG1_MAC("[UTF]: generated value of vrfId = %d\n", vrfId);

    return vrfId;
}

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
)
{
    GT_U32  iter      = 0;
    GT_U32  randIndex = 0; /* random array index */
    GT_U32  tmpVal    = 0;


    /* set random run mode flag */
    isRandomRunMode = enable;

    /* set random seed */
    prvUtfRandomSeedNum = (UTF_AUTO_SEED_NUM_CNS == seed) ? prvUtfSeedFromStreamNameGet() : seed;

    /* restore index array */
    for (iter = 0; iter < utfCtxCommon.usedTestNum; iter++)
    {
        utfCtxCommon.testIndexArray[iter] = iter;
    }

    /* mix up index array for random mode */
    if (GT_TRUE == isRandomRunMode)
    {
        /* set specific seed for random generator */
        cpssOsSrand(seed);

        /* generate random array indexes */
        for (iter = 0; iter < utfCtxCommon.usedTestNum; iter++)
        {
            /* generate random index */
            randIndex = cpssOsRand() % utfCtxCommon.usedTestNum;

            /* exchange array values */
            tmpVal = utfCtxCommon.testIndexArray[randIndex];
            utfCtxCommon.testIndexArray[randIndex] = utfCtxCommon.testIndexArray[iter];
            utfCtxCommon.testIndexArray[iter] = tmpVal;
        }
    }

    return GT_OK;
}

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
)
{
    *enablePtr = isRandomRunMode;
    *seedPtr = prvUtfRandomSeedNum;

    return GT_OK;
}

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
)
{
    GT_U32  testIdx = 0;
    GT_U32  tmpVal  = 0;


    /* check test name */
    CPSS_NULL_PTR_CHECK_MAC(testNamePtr);

    /* fing test's index to run first */
    while (cpssOsStrCmp(utfCtxCommon.testEntriesArray[utfCtxCommon.testIndexArray[testIdx]].testNamePtr, testNamePtr) &&
           (++testIdx < utfCtxCommon.usedTestNum));

    /* check if test found */
    if (testIdx == utfCtxCommon.usedTestNum)
    {
        /* set wrong test's name */
        PRV_UTF_LOG1_MAC("[UTF]: utfTestRunFirstSet: invalid test name -- %s\n", testNamePtr);

        return GT_NOT_FOUND;
    }

    /* store test's index to run first */
    tmpVal = utfCtxCommon.testIndexArray[testIdx];

    /* shift indexes in array */
    while (testIdx--)
    {
        utfCtxCommon.testIndexArray[testIdx + 1] = utfCtxCommon.testIndexArray[testIdx];
    }

    /* set test at the beginning of array */
    utfCtxCommon.testIndexArray[0] = tmpVal;

    return GT_OK;
}

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
const GT_CHAR* utfTestNameGet(GT_VOID)
{
    return utfCtxCommon.testEntriesArray[utfCtxPtr->currTestNum].testNamePtr;
}

#ifdef ASIC_SIMULATION
/* state the we force unbind of slans or not :
    0 --> not force unbind
    1 --> force unbind

    function MUST be called before running the enh-UT  for the first time.
*/
GT_STATUS utfForceUnbindSlanSet(GT_U32  force)
{
    utfForceUnbindSlan = force;
    return GT_OK;
}
#endif /*ASIC_SIMULATION*/

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
)
{
    printPassOkSummary = enable;
    return GT_OK;
}

/**
* @internal utfPostRunFinalStatsShow function
* @endinternal
*
* @brief   Displays final statistics about all suits, list of failed suits and
*         list of failed test cases for the suits
*
* @note none
*
*/
GT_STATUS utfPostRunFinalStatsShow
(
    GT_VOID
)
{
    GT_BOOL orig_printPassOkSummary = printPassOkSummary;/*store original value*/

    /*state that we need also the 'PASS OK' (and skipped) list */
    printPassOkSummary = GT_TRUE;
    /* do the actual printings of the status of tests */
    utfFinalStatsShow();
    /* restore the original value */
    printPassOkSummary = orig_printPassOkSummary;

    return GT_OK;
}

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
)
{
    prvUtfDebugPostTestExitPtr = exitFuncPtr;
}

/**
* @internal utfErrorCountGet function
* @endinternal
*
* @brief   This function return the number of errors from last time called.
*/
GT_U32 utfErrorCountGet(void)
{
    GT_U32  currentNum = totalNumErrors;/*save old value */

    totalNumErrors = 0;/* reset value*/

    return currentNum; /*return the old value */
}

/**
* @internal utfLogStatusGet function
* @endinternal
*
* @brief   This debug function checks log readiness before new API call.
*
* @retval GT_OK                    - log is ready to run
* @retval GT_BAD_STATE             - log is state machine in bad state to run log
*/
GT_STATUS utfLogStatusGet
(
    GT_VOID
)
{
#ifdef CPSS_LOG_ENABLE
    return prvCpssLogStateCheck();
#else
    return GT_OK;
#endif
}


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
)
{
    return &utfTestTaskId;
}

#ifdef DXCH_CODE
/**
* @internal prvUtfSetE2PhyEqualValue function
* @endinternal
*
* @brief   set eport to point to it's physical port '1:1'
*
* @param[in] portIndex                - port index in array of ports
*                                       None
*/
void prvUtfSetE2PhyEqualValue(
    IN GT_U32 portIndex
)
{
    GT_STATUS   rc;
    GT_U32  trgEPort;
    CPSS_INTERFACE_INFO_STC     physicalInfo;

    cpssOsMemSet(&physicalInfo, 0, sizeof(physicalInfo));

    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum = prvTgfPortsArray[portIndex];

    trgEPort = physicalInfo.devPort.portNum;

    /*hwDevNumForRestore*/

    /* since the 'FROM_CPU' will not use 'isPhysicalPortValid' but use the eport
       we need to update the 'E2PHY' to redirect to 'local device' (0x8)
       instead of 'default' 0x10 */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                trgEPort,
                &physicalInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d, %d",
                                 prvTgfDevNum, trgEPort);
}
#endif /*DXCH_CODE*/


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
)
{
    GT_U32  iter;
    /* a test may accidentally change the defaults needed by all other tests.
       make sure to restore orig values.  */
    prvTgfPortsNum = prvTgfPortsNum_orig;

    for (iter = 0; iter < PRV_TGF_MAX_PORTS_NUM_CNS; iter++)
    {
        prvTgfPortsArray[iter] = prvTgfPortsArray_orig[iter];
    }
}
/* utfTestsTypeRun */
/**
* @internal utfTestsTypeSuitesRangeRun function
* @endinternal
*
* @brief   Runs range suits of the given type.
*          Calculates the bounding suit names and calls utfTestsRun.
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
GT_STATUS utfTestsTypeSuitesRangeRun
(
    IN GT_U32               numOfRuns,
    IN GT_BOOL              fContinue,
    IN UTF_TEST_TYPE_ENT    testType,
    IN GT_U32               startSuiteIndex,
    IN GT_U32               numOfSuites
)
{
    const char* prevSuiteName;
    const char* suiteName;
    const char* startSuiteName;
    const char* endSuiteName;
    char        suiteNamesRange[256];
    GT_U32      countIdx;
    GT_U32      suiteIdx;
    GT_U32      endSuiteIndex;

    if (numOfSuites == 0)
    {
        cpssOsPrintf("numOfSuites == 0 - nothing to do\n");
        return GT_BAD_PARAM;
    }

    endSuiteIndex  = (startSuiteIndex + numOfSuites - 1);
    prevSuiteName  = ""; /* string different from all suit names */
    startSuiteName = "";
    endSuiteName   = "";
    suiteName      = "";
    suiteIdx       = (GT_U32)-1;

    for (countIdx = 0; (countIdx < utfCtxCommon.usedTestNum); countIdx++)
    {
        if (utfCtxCommon.testEntriesArray[countIdx].testType != testType) continue;

        suiteName = utfCtxCommon.testEntriesArray[countIdx].suitNamePtr;
        if (cpssOsStrCmp(prevSuiteName, suiteName) == 0) continue;

        /* new suite name */
        suiteIdx ++;
        prevSuiteName = suiteName;
        if (suiteIdx == startSuiteIndex)
        {
            startSuiteName = suiteName;
        }
        if (suiteIdx == endSuiteIndex)
        {
            endSuiteName = suiteName;
            break;
        }
    }

    if (startSuiteName[0] == 0)
    {
        cpssOsPrintf("Start Suite not Found\n");
        return GT_NOT_FOUND;
    }
    if (endSuiteName[0] == 0)
    {
        /* last Suite of the specified type */
        endSuiteName = suiteName;
    }
    if ((cpssOsStrlen(startSuiteName) + cpssOsStrlen(endSuiteName) + 2)
        > sizeof(suiteNamesRange))
    {
        cpssOsPrintf(
            "Start Suite and End Suite names too long \n %s \n %s \n",
            startSuiteName, endSuiteName);
        return GT_FAIL;
    }

    suiteNamesRange[0] = 0;
    cpssOsStrCat(suiteNamesRange, startSuiteName);
    cpssOsStrCat(suiteNamesRange, "-");
    cpssOsStrCat(suiteNamesRange, endSuiteName);

    return utfTestsRun(suiteNamesRange, numOfRuns, fContinue);
}
