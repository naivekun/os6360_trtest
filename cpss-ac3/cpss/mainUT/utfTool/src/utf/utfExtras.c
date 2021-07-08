/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvUtfExtras.c
*
* DESCRIPTION:
*       defines API for helpers functions
*       which are specific for cpss unit testing.
*
* FILE REVISION NUMBER:
*       $Revision: 75
*
*******************************************************************************/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC


#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <extUtils/trafficEngine/tgfTrafficEngine.h>
#include <common/tgfCommon.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <extUtils/trafficEngine/prvTgfLog.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>

#if (defined EX_FAMILY) || (defined MX_FAMILY)
#define CALL_EX_MX_CODE_MAC
#include <cpss/exMx/exMxGen/config/private/prvCpssExMxInfo.h>
#endif /* (defined EX_FAMILY) || (defined MX_FAMILY) */

#ifdef CHX_FAMILY
    #ifndef CALL_EX_MX_CODE_MAC
        /*the include to H file of exmx clash with the h file of the dxch
            (about definition of _txPortRegs)*/
        #include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
        #define CALL_DXCH_CODE_MAC
    #endif /*CALL_EX_MX_CODE_MAC*/
    #include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
    #include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
    #include <common/tgfIpGen.h>
    #include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>

/*  HIR used flag */
extern GT_BOOL isHirApp;
#endif /*CHX_FAMILY*/

#ifdef EXMXPM_FAMILY
#include <cpss/exMxPm/exMxPmGen/cpssHwInit/cpssExMxPmHwInit.h>
#include <cpss/exMxPm/exMxPmGen/config/cpssExMxPmCfg.h>
#include <cpss/exMxPm/exMxPmGen/config/private/prvCpssExMxPmInfo.h>
#endif /*EXMXPM_FAMILY*/

#ifdef PX_FAMILY
#include <cpss/px/config/private/prvCpssPxInfo.h>
#endif /*PX_FAMILY*/

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <appDemo/sysHwConfig/appDemoDb.h>
/* include the AppDemo to get info about the infrastructure */
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>

#include <trafficEngine/tgfTrafficTable.h>

#ifndef GT_NA
#define GT_NA ((GT_U32)~0)
#endif

/******************************************************************************\
 *                        Packet Processor Definitions                       *
\******************************************************************************/

/* macro to check that device exists */
#define  DEV_CHECK_MAC(dev) if(!PRV_CPSS_IS_DEV_EXISTS_MAC(dev)) return GT_BAD_PARAM


#if (defined CHX_FAMILY)
#define  PORT_TO_MAC_IN_PHY_CHECK_MAC(dev,port,macPortNum) \
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(dev, port, macPortNum)
#else
#define  PORT_TO_MAC_IN_PHY_CHECK_MAC           PORT_TO_MAC_CHECK_MAC
#endif


#if (defined CHX_FAMILY)
#define  PORT_TO_MAC_CHECK_MAC(dev,port,macPortNum) \
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(dev, port, macPortNum)
#elif (defined PX_FAMILY)
#define  PORT_TO_MAC_CHECK_MAC(dev,port,macPortNum) \
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(dev, port, macPortNum)
#else
#define  PORT_TO_MAC_CHECK_MAC(dev,port,macPortNum) \
    PRV_CPSS_PHY_PORT_CHECK_MAC(dev, port); \
    macPortNum = port
#endif


#if (defined CHX_FAMILY)
#define  PORT_TO_MAC_IN_PHY_OR_CPU_PORT_CHECK_MAC(dev,port,macPortNum) \
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(dev, port, macPortNum);
#endif

#if (defined PX_FAMILY)
#define  PX_PORT_TO_MAC_IN_PHY_OR_CPU_PORT_CHECK_MAC(dev, port, macPortNum) PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(dev, port, macPortNum);
#endif

/* max number of recursive iterations */
#define ITERATION_MAX_DEPTH_CNS     5
/* current depth of the iteration */
static __THREAD_LOCAL GT_U32   utIteratorDepth = 0;
/* indication that the all iterations are done */
static __THREAD_LOCAL GT_BOOL  uftIsLastIterationDone = GT_FALSE;

/**
* @struct EXTRAS_CTX_STC
 *
 * @brief Extras context storage.
 * There is can be only one statically allocated
 * instance of this storage. Used for saving
 * devices, ports, and iterators run-time info.
*/
typedef struct{

    GT_U32 devArray[PRV_CPSS_MAX_PP_DEVICES_CNS];

    GT_U32 phyNumPortsArray[PRV_CPSS_MAX_PP_DEVICES_CNS];

    GT_U32 virtNumPortsArray[PRV_CPSS_MAX_PP_DEVICES_CNS];

    /** current active device id for iteration thru phy ports */
    GT_U8 devForPhyPorts;

    /** @brief current active device id for iteration thru virt ports
     *  currentApplicableFamiliesHandle[] - the applicable families for the device iteration (per 'depth')
     */
    GT_U8 devForVirtPorts;

    GT_U32 currentApplicableFamiliesHandle[ITERATION_MAX_DEPTH_CNS];

} EXTRAS_CTX_STC;

/* single instance of extras context, used for iteration */
static EXTRAS_CTX_STC extCtx;

/* Extras internal functions forward declaration */
static GT_STATUS devCheck(IN GT_U8 dev);
static GT_STATUS phyPortCheck(IN GT_U8 dev, IN GT_PHYSICAL_PORT_NUM port);
static GT_STATUS phyPortOrCpuCheck(IN GT_U8 dev, IN GT_PHYSICAL_PORT_NUM port);
static GT_STATUS virtPortCheck(IN GT_U8 dev, IN GT_PORT_NUM port);

static GT_STATUS macPortCheck(IN GT_U8 dev, IN GT_PORT_NUM port);
static GT_STATUS txqPortCheck(IN GT_U8 dev, IN GT_PORT_NUM port);
static GT_STATUS rxDmaPortCheck(IN GT_U8 dev, IN GT_PORT_NUM port);
static GT_STATUS txDmaPortCheck(IN GT_U8 dev, IN GT_PORT_NUM port);

static GT_VOID firstTimeTrunkConvertInit(GT_VOID);

/* flag to state that the DXCH (xcat) devices need the WA of trunk-Id conversions */
GT_BOOL prvUtfDxChTrunkWaNeeded = GT_FALSE;

/* indication the trunk convert done for first time */
static __THREAD_LOCAL GT_BOOL firstTimeTrunkConvert = GT_TRUE;

/* number of skipped UT in suit */
static __THREAD_LOCAL GT_U32 prvUtfSkipTestNum = 0;

/* flag to indicate is current test is skipped */
static __THREAD_LOCAL GT_BOOL prvUtfIsTestSkip = GT_FALSE;

/* flag to indicate if long running tests should be skipped */
static __THREAD_LOCAL GT_BOOL prvUtfIsLongTestsSkip = GT_FALSE;

/* flag to indicate that tests execution is baseline one */
static __THREAD_LOCAL GT_BOOL prvUtfIsBaselineTestsExecution = GT_FALSE;

/* flag of limitation of CPSS API logging */
static __THREAD_LOCAL GT_BOOL prvUtfIsReduceLogSize = GT_FALSE;

/* use 56 as the first trunk-ID */
#define WA_FIRST_TRUNK_CNS  56
#define WA_LAST_TRUNK_CNS  (124+1)

/* Array with invalid enum values to check.
   Using GT_U32 type instead of GT_32 can be helpful to find some bugs */
GT_U32 utfInvalidEnumArr[] = {0x5AAAAAA5, 0xFFFFFFFF};

/* Invalid enum array size */
GT_U32 utfInvalidEnumArrSize = sizeof(utfInvalidEnumArr) / sizeof(utfInvalidEnumArr[0]);

/* the generic iterator type */
static __THREAD_LOCAL UTF_GENERIC_PORT_ITERATOR_TYPE_E currentGenericPortIteratorType = UTF_GENERIC_PORT_ITERATOR_TYPE_PHYSICAL_E;
#define ITERATOR_INIT_VALUE_CNS     0xFFFFFFFF
/*'is After System Reset' ?*/
static GT_BOOL isAfterSystemReset = GT_FALSE;

static GT_U32   totalDeviceNum = 0;/* total number of devices */
/******************************************************************************\
 *                          Fabric Adapter Definitions                        *
\******************************************************************************/

#ifdef IMPL_FA

/* External usage environment parameters */
extern CORE_FA_CONFIG  *coreFaDevs[PRV_CPSS_MAX_PP_DEVICES_CNS];

/**
* @struct EXTRAS_FA_CTX_STC
 *
 * @brief Extras context storage.
 * There is can be only one statically allocated
 * instance of this storage. Used for saving
 * devices fabric adapret, ports, and iterators run-time info.
*/
typedef struct{

    GT_U8 devArray[PRV_CPSS_MAX_PP_DEVICES_CNS];

    GT_U8 faNumPortsArray[PRV_CPSS_MAX_PP_DEVICES_CNS];

    /** contains number of Fabric ports for each device */
    GT_U8 devForFaPorts;

    GT_U32 currentApplicableTypesHandle;

} EXTRAS_FA_CTX_STC;

/* single instance of extras context, used for iteration */
static EXTRAS_FA_CTX_STC extFaCtx;

/* Extras internal functions forward declaration */
static GT_STATUS devFaCheck(IN GT_U8 dev);

/* Returns GT_OK if fabric port is active */
static GT_STATUS faPortCheck(IN GT_U8 dev, IN GT_U8 port);

/* macro to check that FA device exists */
#define  FA_CHECK_MAC(dev) if (GT_OK != devFaCheck(dev)) return GT_BAD_PARAM

#endif /* IMPL_FA */


/******************************************************************************\
 *                                 Xbar Definitions                           *
\******************************************************************************/

#ifdef IMPL_XBAR

/* External usage environment parameters */
extern CORE_XBAR_CONFIG  coreXbarDevs[PRV_CPSS_MAX_PP_DEVICES_CNS];

/**
* @struct EXTRAS_XBAR_CTX_STC
 *
 * @brief Extras context storage.
 * There is can be only one statically allocated
 * instance of this storage. Used for saving
 * devices xbar adapret, ports, and iterators run-time info.
*/
typedef struct{

    GT_U8 devArray[PRV_CPSS_MAX_PP_DEVICES_CNS];

    GT_U8 faNumPortsArray[PRV_CPSS_MAX_PP_DEVICES_CNS];

    /** contains number of Fabric ports for each device */
    GT_U8 devForFaPorts;

    GT_U32 currentApplicableTypesHandle;

} EXTRAS_XBAR_CTX_STC;

/* single instance of extras context, used for iteration */
static __THREAD_LOCAL EXTRAS_XBAR_CTX_STC extXbarCtx;

/* Extras internal functions forward declaration */
static GT_STATUS devXbarCheck(IN GT_U8 dev);

/* Returns GT_OK if fabric port is active */
static GT_STATUS xbarFaPortCheck(IN GT_U8 dev, IN GT_U8 port);

/* macro to check that Xbar device exists */
#define  XBAR_CHECK_MAC(dev) if (GT_OK != devXbarCheck(dev)) return GT_BAD_PARAM

#endif /* IMPL_XBAR */

#define MAX_MUTEX_NUM    1024
/**
* @struct PRV_CPSS_DXCH_TEST_MUTEX_INFO_STC
 *
 * @brief A structure to hold Id of threads that locked or waiting for other mutexes
*/
typedef struct{

    /** mutex locked by thread with Id stored in this field. */
    GT_U32 lock;

    /** thread with Id stored in this field is waiting for this mutex */
    GT_U32 wait;

} PRV_CPSS_DXCH_TEST_MUTEX_INFO_STC;

/* Store mutex lock/unlock function */
static CPSS_OS_MUTEX_LOCK_FUNC lockSavedFunc;
static CPSS_OS_MUTEX_UNLOCK_FUNC unlockSavedFunc;

/* Structure hold mutexes info: which thread lock or wait for mutex */
static PRV_CPSS_DXCH_TEST_MUTEX_INFO_STC mutexState[MAX_MUTEX_NUM];

/* mutex to protect Safe Lock function functionality */
static CPSS_OS_MUTEX   prvUtfSafeLockProtection;

/* Flag to indicate that we have deadlock */
static GT_BOOL isDeadlockDetected = GT_FALSE;

static UTF_TEST_TYPE_ENT prvTgfCurrentTestType;

extern GT_STATUS cpssInitSystem
(
    IN  GT_U32  boardIdx,
    IN  GT_U32  boardRevId,
    IN  GT_U32  reloadEeprom
);

extern void cpssInitSystemGet
(
    OUT GT_U32  *boardIdxPtr,
    OUT GT_U32  *boardRevIdPtr,
    OUT GT_U32  *reloadEepromPtr
);

extern GT_STATUS cpssResetSystem
(
    IN GT_BOOL doHwReset
);

/**
* @internal prvUtfSetCurrentTestType function
* @endinternal
*
* @brief   Sets current test type for proper device selection with
*         prvUtfNextDeviceGet.
* @param[in] testType                 - test type.
*                                       None.
*/
GT_VOID prvUtfSetCurrentTestType
(
    IN UTF_TEST_TYPE_ENT testType
)
{
    prvTgfCurrentTestType = testType;
}

/**
* @internal prvUtfIsCpuPort function
* @endinternal
*
* @brief   Clarifies if given port is CPU port.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
*
* @retval GT_TRUE                  - CPU Port, GT_FALSE - other.
*/
GT_BOOL prvUtfIsCpuPort
(
    GT_U8                 devNum,
    GT_PHYSICAL_PORT_NUM  portNum
)
{
    GT_BOOL isCpuPort = ((portNum == CPSS_CPU_PORT_NUM_CNS) ? GT_TRUE : GT_FALSE);
#ifdef CHX_FAMILY
    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* this function returns GT_BAD_PARAM, GT_TRUE and GT_FALSE */
        GT_STATUS rc;

        if(portNum >= PRV_CPSS_MAX_PP_PORTS_NUM_CNS)
        {
            /* avoid CPSS ERROR LOG from reporting error about the port number */
            return GT_FALSE;
        }

        rc = prvCpssDxChPortPhysicalPortIsCpu(
            devNum, portNum, &isCpuPort);
        return ((rc != GT_OK) ? GT_FALSE : isCpuPort);
    }
    else
    {
        return isCpuPort;
    }
#else /* CHX_FAMILY */
    devNum = devNum;    /* prevent warnings about unused parameter */

    return isCpuPort;
#endif /* CHX_FAMILY */
}

/**
* @internal prvUtfExtrasInit function
* @endinternal
*
* @brief   Initialize extras for cpss unit testing.
*         This function inits available devices and ports per device.
*/
GT_STATUS prvUtfExtrasInit
(
    GT_VOID
)
{
    GT_STATUS st = GT_OK;
    GT_U8 dev;
    GT_BOOL devWasFound     = GT_FALSE;
    GT_BOOL devFaWasFound   = GT_FALSE;
    GT_BOOL devXbarWasFound = GT_FALSE;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32  ii;

    cpssOsBzero((GT_CHAR*)&extCtx, sizeof(extCtx));

#ifdef IMPL_FA
    cpssOsBzero((GT_CHAR*)&extFaCtx, sizeof(extFaCtx));
#endif /* IMPL_FA */

#ifdef IMPL_XBAR
    cpssOsBzero((GT_CHAR*)&extXbarCtx, sizeof(extXbarCtx));
#endif /* IMPL_XBAR */

    for (dev = 0; dev < PRV_CPSS_MAX_PP_DEVICES_CNS; dev++)
    {
        if (GT_OK == devCheck(dev))
        {
            devFamily = PRV_CPSS_PP_MAC(dev)->devFamily;

            switch (devFamily)
            {
                case CPSS_PP_FAMILY_TWISTC_E:
                    extCtx.devArray[dev] = UTF_TWISTC_E;
                    break;
                case CPSS_PP_FAMILY_TWISTD_E:
                    extCtx.devArray[dev] = UTF_TWISTD_E;
                    break;
                case CPSS_PP_FAMILY_SAMBA_E:
                    extCtx.devArray[dev] = UTF_SAMBA_E;
                    break;
                case CPSS_PP_FAMILY_TIGER_E:
                    extCtx.devArray[dev] = UTF_TIGER_E;
                    break;
                case CPSS_PP_FAMILY_SALSA_E:
                    extCtx.devArray[dev] = UTF_SALSA_E;
                    break;
                case CPSS_PP_FAMILY_PUMA_E:
                    extCtx.devArray[dev] = UTF_PUMA2_E;
                    break;
                case CPSS_PP_FAMILY_PUMA3_E:
                    extCtx.devArray[dev] = UTF_PUMA3_E;
                    break;
                case CPSS_PP_FAMILY_CHEETAH_E:
                    extCtx.devArray[dev] = UTF_CH1_E;
                    break;
                case CPSS_PP_FAMILY_CHEETAH2_E:
                    extCtx.devArray[dev] = UTF_CH2_E;
                    break;
                case CPSS_PP_FAMILY_CHEETAH3_E:
                    extCtx.devArray[dev] = UTF_CH3_E;
                    break;
                case CPSS_PP_FAMILY_DXCH_XCAT_E:
                    extCtx.devArray[dev] = (0 == PRV_CPSS_PP_MAC(dev)->revision) ? UTF_CH3_E : UTF_XCAT_E;
                    break;
                case CPSS_PP_FAMILY_DXCH_LION_E:
                    extCtx.devArray[dev] = UTF_LION_E;
                    break;
                case CPSS_PP_FAMILY_DXCH_LION2_E:
                    extCtx.devArray[dev] = UTF_LION2_E;
                    break;
                case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
                    if(PRV_CPSS_PP_MAC(dev)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
                    {
                        extCtx.devArray[dev] = UTF_CAELUM_E;
                    }
                    else
                    {
                        extCtx.devArray[dev] = UTF_BOBCAT2_E;
                    }
                    break;
                case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
                    extCtx.devArray[dev] = UTF_BOBCAT3_E;
                    break;
                case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
                    extCtx.devArray[dev] = UTF_ALDRIN2_E;
                    break;
                case CPSS_PP_FAMILY_DXCH_FALCON_E:
                    extCtx.devArray[dev] = UTF_FALCON_E;
                    break;
                case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
                    extCtx.devArray[dev] = UTF_ALDRIN_E;
                    break;
                case CPSS_PP_FAMILY_DXCH_AC3X_E:
                    extCtx.devArray[dev] = UTF_AC3X_E;
                    break;
                case CPSS_PP_FAMILY_DXCH_XCAT2_E:
                    extCtx.devArray[dev] = UTF_XCAT2_E;
                    break;
                case CPSS_PP_FAMILY_DXCH_XCAT3_E:
                    extCtx.devArray[dev] = UTF_XCAT3_E;
                    break;
                case CPSS_PX_FAMILY_PIPE_E:
                    extCtx.devArray[dev] = UTF_PIPE_E;
                    break;

                default:
                    extCtx.devArray[dev] = 0;
                    break;
            }

            totalDeviceNum++;

#ifdef CHX_FAMILY
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
            {
                extCtx.phyNumPortsArray[dev] = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(dev);
            }
            else
#endif /* CHX_FAMILY */
            {
                extCtx.phyNumPortsArray[dev] = PRV_CPSS_PP_MAC(dev)->numOfPorts;
            }

#ifdef PX_FAMILY
            if(PRV_CPSS_PP_MAC(dev)->functionsSupportedBmp & PRV_CPSS_PX_FUNCTIONS_SUPPORT_CNS)
            {
                extCtx.phyNumPortsArray[dev] = PRV_CPSS_PX_PORTS_NUM_CNS;
            }
#endif /*PX_FAMILY*/


            extCtx.virtNumPortsArray[dev] =
#ifdef CHX_FAMILY
                PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(dev);
#else /*! CHX_FAMILY*/
                PRV_CPSS_PP_MAC(dev)->numOfVirtPorts;
#endif /* CHX_FAMILY */
            for(ii = 0 ; ii < ITERATION_MAX_DEPTH_CNS; ii++)
            {
                extCtx.currentApplicableFamiliesHandle[ii] = UTF_ALL_FAMILY_E;
            }
            devWasFound = GT_TRUE;

            PRV_UTF_LOG3_MAC("[UTF]: prvUtfExtrasInit: PP dev [%d], phy ports [%d], virt ports [%d] ",/* no \n */
                             dev,
                             extCtx.phyNumPortsArray[dev],
                             extCtx.virtNumPortsArray[dev]);

            if(PRV_CPSS_PP_MAC(dev)->numOfVirtPorts > PRV_CPSS_PP_MAC(dev)->numOfPorts)
            {
                /* For the devices that supports 'virtual ports' ,
                   in order to find places where we iterate on physical ports
                   instead of on virtual ports , add 1 to the number of
                   'physical ports' --> the iterator of prvUtfNextPhyPortGet(...)
                   will return the 'port = PRV_CPSS_PP_MAC(dev)->numOfPorts' for
                   'non physical' port , but if the API of CPSS supports 'virtula port'
                   it will return GT_OK (where test expect GT_BAD_PARAM) ... so
                   we will know that : TEST is wrong to use 'physical port' iteration
                   and it should use 'virtual port' iteration */

                extCtx.phyNumPortsArray[dev] ++;
            }

            cpssOsPrintf("MAC ports:");
            printBitmapArr("---",
                PRV_CPSS_PP_MAC(dev)->existingPorts.ports,
                PRV_CPSS_PP_MAC(dev)->numOfPorts,
                0xFFFFFFFF/* ignore the value */);
        }
        else
        {
            extCtx.devArray[dev] = 0;
            extCtx.phyNumPortsArray[dev] = 0;
            extCtx.virtNumPortsArray[dev] = 0;
        }

#ifdef IMPL_FA

        if (GT_OK == devFaCheck(dev))
        {
            switch (coreFaDevs[dev]->devType)
            {
                case GT_FA_98FX900A:
                    extFaCtx.devArray[dev] = UTF_CPSS_FA_TYPE_98FX900A_CNS;
                break;
                case GT_FA_98FX902A:
                    extFaCtx.devArray[dev] = UTF_CPSS_FA_TYPE_98FX902A_CNS;
                break;
                case GT_FA_98FX910A:
                    extFaCtx.devArray[dev] = UTF_CPSS_FA_TYPE_98FX910A_CNS;
                break;
                case GT_FA_98FX915A:
                    extFaCtx.devArray[dev] = UTF_CPSS_FA_TYPE_98FX915A_CNS;
                break;
                default:
                    extFaCtx.devArray[dev] = 0;
            }

            extFaCtx.faNumPortsArray[dev] = coreFaDevs[dev]->numOfFports;
            devFaWasFound = GT_TRUE;

            PRV_UTF_LOG2_MAC("[UTF]: prvUtfExtrasInit: FA dev [%d], fa ports [%d]\n",
                             dev,
                             extFaCtx.faNumPortsArray[dev]);
        }
        else
        {
            extFaCtx.devArray[dev] = 0;
        }
#endif /* IMPL_FA */

#ifdef IMPL_XBAR

        if (GT_OK == devXbarCheck(dev))
        {
            switch (coreXbarDevs[dev].devType)
            {
                case GT_XBAR_98FX900A:
                    extXbarCtx.devArray[dev] = UTF_CPSS_XBAR_TYPE_98FX900A_CNS;
                break;
                case GT_XBAR_98FX902A:
                    extXbarCtx.devArray[dev] = UTF_CPSS_XBAR_TYPE_98FX902A_CNS;
                break;
                case GT_XBAR_98FX910A:
                    extXbarCtx.devArray[dev] = UTF_CPSS_XBAR_TYPE_98FX910A_CNS;
                break;
                case GT_XBAR_98FX915A:
                    extXbarCtx.devArray[dev] = UTF_CPSS_XBAR_TYPE_98FX915A_CNS;
                break;
                case GT_XBAR_98FX9010A:
                    extXbarCtx.devArray[dev] = UTF_CPSS_XBAR_TYPE_98FX9010A_CNS;
                break;
                case GT_XBAR_98FX9110:
                    extXbarCtx.devArray[dev] = UTF_CPSS_XBAR_TYPE_98FX9110A_CNS;
                break;
                case GT_XBAR_98FX9210:
                    extXbarCtx.devArray[dev] = UTF_CPSS_XBAR_TYPE_98FX9210A_CNS;
                break;
                default:
                    extXbarCtx.devArray[dev] = 0;
            }

            extXbarCtx.faNumPortsArray[dev] = coreXbarDevs[dev].numOfFports;
            devXbarWasFound = GT_TRUE;

            PRV_UTF_LOG2_MAC("[UTF]: prvUtfExtrasInit: Xbar dev [%d], fa ports [%d]\n",
                             dev,
                             extXbarCtx.faNumPortsArray[dev]);
        }
        else
        {
            extXbarCtx.devArray[dev] = 0;
        }
#endif /* IMPL_XBAR */
    }

    if ((GT_FALSE == devWasFound) && (GT_FALSE == devFaWasFound) && (GT_FALSE == devXbarWasFound))
    {
        PRV_UTF_LOG0_MAC("[UTF]: prvUtfExtrasInit: no active device was found\n");
        st = GT_FAIL;
    }

    return st;
}


/******************************************************************************\
 *                        Packet Processor Implementation                     *
\******************************************************************************/

static __THREAD_LOCAL GT_U32   utDevMaxIterations = 0;
static __THREAD_LOCAL GT_BOOL  utfIsSingleIterationPerFamily[ITERATION_MAX_DEPTH_CNS] = {GT_FALSE};
static __THREAD_LOCAL GT_BOOL  internal_utfIsSingleIterationPerFamily = GT_FALSE;
static __THREAD_LOCAL GT_U32   utfTestedFamiliesArr[ITERATION_MAX_DEPTH_CNS][CPSS_PP_FAMILY_LAST_E] = {{0}};/* reduce run of logic per single device of the family */
static __THREAD_LOCAL GT_U32   utPerFamilyMaxIterations = 1;/*relevant only when utfIsSingleIterationPerFamily == GT_TRUE */

static __THREAD_LOCAL GT_BOOL  doReverseIterations[ITERATION_MAX_DEPTH_CNS] = {GT_FALSE};
static __THREAD_LOCAL GT_U32   internal_utDevMaxIterations[ITERATION_MAX_DEPTH_CNS] = {0};
static __THREAD_LOCAL GT_U32   internal_utDevActualIterations[ITERATION_MAX_DEPTH_CNS] = {0};
static __THREAD_LOCAL GT_BOOL  utCurrentTestEnded = GT_TRUE;
static __THREAD_LOCAL GT_U32   internal_skippedDevicesBmp[PRV_CPSS_MAX_PP_DEVICES_CNS / 32] = {0};

/**
* @internal prvUtfResetSkippedDevices function
* @endinternal
*
* @brief   Reset Skipped devices bitmap
*/
GT_STATUS prvUtfResetSkippedDevices
(
    IN GT_VOID
)
{
    cpssOsMemSet(internal_skippedDevicesBmp, 0, sizeof(internal_skippedDevicesBmp));
    return GT_OK;
}

/**
* @internal prvUtfSkipAllDevices function
* @endinternal
*
* @brief   Reset all devices in bitmap
*
* @note We use it in multi thread runs to disable all devices.
*
*/
GT_STATUS prvUtfSkipAllDevices
(
    IN GT_VOID
)
{
    cpssOsMemSet(internal_skippedDevicesBmp, 0xff, sizeof(internal_skippedDevicesBmp));
    return GT_OK;
}
/**
* @internal prvUtfSetSkippedDevice function
* @endinternal
*
* @brief   Set akipped device
*
* @param[in] devNum                   - device number
*                                       GT_OK always.
*/
GT_STATUS prvUtfSetSkippedDevice
(
    IN GT_U8 devNum
)
{
    if (devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        return GT_OK;
    }
    internal_skippedDevicesBmp[devNum / 32] |= (1 << (devNum & 0x1F));
    return GT_OK;
}

/**
* @internal prvUtfSetActiveDevice function
* @endinternal
*
* @brief   Set active device
*
* @param[in] devNum                   - device number
*                                       GT_OK always.
*
* @note We should use it to init prvTgfDevNum in multi thread run or after
*       prvUtfSkipAllDevices().
*
*/
GT_STATUS prvUtfSetActiveDevice
(
    IN GT_U8 devNum
)
{
    if (devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        return GT_OK;
    }
    internal_skippedDevicesBmp[devNum / 32] &= ~(1 << (devNum & 0x1F));
    prvTgfDevNum = devNum;
    return GT_OK;
}

/**
* @internal prvUtfIsSkippedDevice function
* @endinternal
*
* @brief   Check is device skipped
*
* @param[in] devNum                   - device number
*
* @retval GT_TRUE                  - skipped, GT_FALSE - otherwise
*/
GT_BOOL prvUtfIsSkippedDevice
(
    IN GT_U8 devNum
)
{
    if (devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        return GT_FALSE;
    }
    return ((internal_skippedDevicesBmp[devNum / 32] & (1 << (devNum & 0x1F))) != 0);
}

/**
* @internal prvUtfSetMaxIterationsOnDevice function
* @endinternal
*
* @brief   state the max number of different device that will be returned from
*         prvUtfNextDeviceGet(...)
*         NOTE:
*         1. max iterations supported = 2
*         2. the configuration is reset AFTER every call to prvUtfNextNotApplicableDeviceReset(...)
*         3. default is loop on 'all devices'
*
* @retval GT_OK                    -  All OK
* @retval GT_BAD_PARAM             - maxIterations is bad
*/
GT_STATUS prvUtfSetMaxIterationsOnDevice
(
    IN GT_U32   maxIterations
)
{
    if(maxIterations > 2)
    {
        return GT_BAD_PARAM;
    }

    utDevMaxIterations = maxIterations;

    return GT_OK;
}

/**
* @internal prvUtfSetSingleIterationPerFamily function
* @endinternal
*
* @brief   state that only single device per family can be returned from
*         prvUtfNextDeviceGet(...)
*         NOTE:
*         1. the configuration is reset AFTER every call to prvUtfNextNotApplicableDeviceReset(...)
*         2. default is 'no limit'
*
* @retval GT_OK                    -  All OK
*/
GT_STATUS prvUtfSetSingleIterationPerFamily(void)
{
    internal_utfIsSingleIterationPerFamily = GT_TRUE;

    return GT_OK;
}

/**
* @internal prvUtfNextNotApplicableDeviceReset function
* @endinternal
*
* @brief   Reset device iterator must be called before go over all devices
*
* @param[in,out] devPtr                   - iterator thru devices
* @param[in] notApplicableFamiliesHandle - handle of non-applicable devices
*
* @retval GT_OK                    -  iterator initialization OK
* @retval GT_FAIL                  -  general failure error
*/
GT_STATUS prvUtfNextNotApplicableDeviceReset
(
    INOUT   GT_U8   *devPtr,
    IN      GT_U32  notApplicableFamiliesHandle
)
{
    if (NULL == devPtr)
    {
        return GT_FAIL;
    }

    /* Make PIPE not applicable if case we are not running PIPE tests */
    if((prvTgfCurrentTestType != UTF_TEST_TYPE_PX_E) &&
       (prvTgfCurrentTestType != UTF_TEST_TYPE_PX_TRAFFIC_E))
    {
        notApplicableFamiliesHandle |= UTF_PIPE_E;
    }

    /* if applicable devices are not specified - error */
    if (UTF_ALL_FAMILY_E == notApplicableFamiliesHandle)
    {
        return GT_FAIL;
    }

    *devPtr = 0xFF;

    if(utCurrentTestEnded == GT_TRUE)
    {
        utCurrentTestEnded = GT_FALSE;
        utIteratorDepth = 0;
        uftIsLastIterationDone = GT_FALSE;
    }
    else
    {
        if((utIteratorDepth+1) == ITERATION_MAX_DEPTH_CNS)
        {
            cpssOsPrintf("prvUtfNextNotApplicableDeviceReset : Error , reached ITERATION_MAX_DEPTH_CNS ... please check it \n");
            return GT_FAIL;
        }

        utIteratorDepth ++;
    }

    extCtx.currentApplicableFamiliesHandle[utIteratorDepth] = UTF_ALL_FAMILY_E & ~(notApplicableFamiliesHandle);

    {/* init all needed iteration info */
        cpssOsMemSet(utfTestedFamiliesArr[utIteratorDepth],0,sizeof(utfTestedFamiliesArr[utIteratorDepth]));
        utfIsSingleIterationPerFamily[utIteratorDepth] = GT_FALSE;
        doReverseIterations[utIteratorDepth] = GT_FALSE;

        if(internal_utfIsSingleIterationPerFamily == GT_TRUE)
        {
            utfIsSingleIterationPerFamily[utIteratorDepth] = GT_TRUE;
            doReverseIterations[utIteratorDepth] = GT_TRUE;
        }

        if(utDevMaxIterations)
        {
            doReverseIterations[utIteratorDepth] = GT_TRUE;
            internal_utDevMaxIterations[utIteratorDepth] = utDevMaxIterations;
        }
        else
        {
            internal_utDevMaxIterations[utIteratorDepth] = 0;
        }


        internal_utDevActualIterations[utIteratorDepth] = 0;
        utDevMaxIterations = 0;/* reset the value ... as test must state explicitly
                                before EVERY call to prvUtfNextNotApplicableDeviceReset */

        if(doReverseIterations[utIteratorDepth] == GT_TRUE)
        {
            (*devPtr) = PRV_CPSS_MAX_PP_DEVICES_CNS;
        }

        internal_utfIsSingleIterationPerFamily = GT_FALSE;/* reset the value ... as test must state explicitly
                                before EVERY call to prvUtfNextNotApplicableDeviceReset */
    }


    return GT_OK;
}

/**
* @internal prvUtfNextDeviceReset function
* @endinternal
*
* @brief   Reset device iterator must be called before go over all devices
*
* @param[in,out] devPtr                   -   iterator thru devices
* @param[in] applicableFamiliesHandle - handle of applicable devices
*
* @retval GT_OK                    -  iterator initialization OK
* @retval GT_FAIL                  -  general failure error
*/
GT_STATUS prvUtfNextDeviceReset
(
    INOUT   GT_U8   *devPtr,
    IN      GT_U32  applicableFamiliesHandle
)
{
    GT_U32  notAppFamilyBmp = UTF_ALL_FAMILY_E;

    CPSS_TBD_BOOKMARK   /* All the calls to prvUtfNextDeviceReset must be removed !!!
                and replaced with calls to PRV_TGF_NOT_APPLIC_DEV_RESET_MAC (like in DXCH code) */

    if(UTF_CPSS_PP_FAMILY_PUMA_CNS & applicableFamiliesHandle)
    {
        /* WA : support Puma devices

           the Puma code (legacy code) calls :
           st = prvUtfNextDeviceReset(&dev, UTF_EXMXPM_FAMILY_SET_CNS);

           so we convert it to : iterate on all devices ! (assuming only Puma exists)
        */

        notAppFamilyBmp = UTF_NONE_FAMILY_E;
    }

    /* prepare device iterator */
    return prvUtfNextNotApplicableDeviceReset(devPtr, notAppFamilyBmp);
}

/**
* @internal prvUtfNextDeviceGet function
* @endinternal
*
* @brief   This routine returns next active/non active device id.
*         Call the function until non GT_OK error. Note that
*         iterator must be initialized before.
* @param[in,out] devPtr                   -   device id
* @param[in] activeIs                 -   set to GT_TRUE for getting active devices
*/
GT_STATUS prvUtfNextDeviceGet
(
    INOUT GT_U8     *devPtr,
    IN GT_BOOL      activeIs
)
{
    GT_STATUS st = GT_OK;

    if (NULL == devPtr)
    {
        return GT_FAIL;
    }

    firstTimeTrunkConvertInit();

retry_next_device_lbl:

    st = GT_FAIL;
    if(doReverseIterations[utIteratorDepth] == GT_FALSE)
    {
        (*devPtr)++;
    }
    else
    {
        if((*devPtr) == 0)
        {
            /* we just finished with last device */
            goto done_device_iterations_lbl;
        }

        (*devPtr)--;
    }

    while (*devPtr < PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        if ((extCtx.devArray[*devPtr] != 0) &&
            ((((extCtx.devArray[*devPtr] & extCtx.currentApplicableFamiliesHandle[utIteratorDepth]) == 0)
              ? GT_FALSE : GT_TRUE) == activeIs))
        {
            switch(prvTgfCurrentTestType)
            {
                case UTF_TEST_TYPE_PX_E:
                case UTF_TEST_TYPE_PX_TRAFFIC_E:
                    if(UTF_PIPE_E != extCtx.devArray[*devPtr])
                    {
                        goto retry_next_device_lbl;
                    }
                    break;
                default:
                    if(UTF_PIPE_E == extCtx.devArray[*devPtr])
                    {
                        goto retry_next_device_lbl;
                    }
            }

            if (prvUtfIsSkippedDevice(*devPtr))
            {
                /* try to find next device - this should be skipped */
                cpssOsPrintf("prvUtfNextDeviceGet : Device %d skipped\n", *devPtr);
                goto retry_next_device_lbl;
            }
            st = GT_OK;
            break;
        }

        if(doReverseIterations[utIteratorDepth] == GT_FALSE)
        {
            (*devPtr)++;
        }
        else
        {
            if((*devPtr) == 0)
            {
                /* we just finished with last device */
                break;
            }
            (*devPtr)--;
        }
    }

    /* check to see if need to reduce devices iterations that come from the same family */
    if(utfIsSingleIterationPerFamily[utIteratorDepth] == GT_TRUE &&
        st == GT_OK && activeIs == GT_TRUE)
    {
        GT_U8   dev = (*devPtr);

        if(PRV_CPSS_IS_DEV_EXISTS_MAC(dev))
        {
            CPSS_PP_FAMILY_TYPE_ENT devFamily = PRV_CPSS_PP_MAC(dev)->devFamily;
            if(devFamily < CPSS_PP_FAMILY_LAST_E)
            {
                utfTestedFamiliesArr[utIteratorDepth][devFamily] ++;

                if(utfTestedFamiliesArr[utIteratorDepth][devFamily] > utPerFamilyMaxIterations)
                {
                    /* try to find next device */
                    goto retry_next_device_lbl;
                }
            }
            if (prvUtfIsSkippedDevice(dev))
            {
                /* try to find next device */
                cpssOsPrintf("prvUtfNextDeviceGet : Device %d skipped\n", dev);
                goto retry_next_device_lbl;
            }
        }
    }

done_device_iterations_lbl:

    /* check if the amount of the iterated devices is correct */
    if(internal_utDevMaxIterations[utIteratorDepth] && st == GT_OK)
    {
        internal_utDevActualIterations[utIteratorDepth] ++;
        if(internal_utDevMaxIterations[utIteratorDepth] >= internal_utDevActualIterations[utIteratorDepth])
        {
            /* do not allow more iterations */
            st = GT_FAIL;
        }
    }

    if(st == GT_FAIL)
    {
        if(utIteratorDepth)
        {
            utIteratorDepth--;
        }
        else /* utIteratorDepth == 0 */
        {
            if(uftIsLastIterationDone == GT_TRUE)
            {
                /* Error */
                cpssOsPrintf("prvUtfNextDeviceGet : Error last iteration already already done ?! \n");
            }

            uftIsLastIterationDone = GT_TRUE;
        }
    }

    return st;
}


/**
* @internal nextOutOfRangePortGet function
* @endinternal
*
* @brief   This routine returns next NON active "out of range" port.
*         Call the function until non GT_OK error. the function will return up to 100 non valid ports
* @param[in,out] portPtr                  -   port id
* @param[in] minPort                  -   the minimal port number to generate
*/
static GT_STATUS nextOutOfRangePortGet
(
    INOUT GT_PORT_NUM       *portPtr,
    IN      GT_U32          minPort
)
{
    static GT_BOOL  isFirstOutOfRange = GT_TRUE;
    GT_U32  nonActivePortStep, numOfNonActivePorts;

#ifndef DXCH_FAMILY
    /* the portNum in exmxpm is GT_U8 */
    nonActivePortStep = 1;
#else
    /* the portNum in DXCH is GT_U32 */
    nonActivePortStep = UTF_E_ARCH_NEXT_PORT_STEP_CNS;
#endif
    if(isFirstOutOfRange == GT_TRUE)
    {
        isFirstOutOfRange = GT_FALSE;
        /* get several virtual ports that are out of range ! */
        /* first value to check */
        (*portPtr) = minPort;
    }
    else
    {
        numOfNonActivePorts = (GT_TRUE == prvUtfReduceLogSizeFlagGet()) ? 2 : 100;
#ifdef PX_FAMILY
        nonActivePortStep = UTF_E_ARCH_NEXT_PORT_STEP_CNS;
        /* iteration of 5 ports beyond port 16 */
        if(numOfNonActivePorts > 5)
        {
            numOfNonActivePorts = 5;
        }
#endif
        /* next value to check */
        (*portPtr) += (nonActivePortStep - 1);/* the caller already incremented by 1 the value */

        if( (((*portPtr) - minPort) / nonActivePortStep) > numOfNonActivePorts)
        {
            isFirstOutOfRange = GT_TRUE;
            /* enough iterations done */
            return GT_FAIL;
        }
    }

    return GT_OK;
}


/**
* @internal prvUtfNextPhyPortReset function
* @endinternal
*
* @brief   Reset phy ports iterator must be called before go over all ports
*
* @param[in,out] portPtr                  -   iterator thru ports
* @param[in] dev                      -   device id of ports
*
* @retval GT_OK                    -  iterator initialization OK
* @retval GT_FAIL                  -  general failure error
*/
GT_STATUS prvUtfNextPhyPortReset
(
    INOUT GT_PHYSICAL_PORT_NUM     *portPtr,
    IN GT_U8        dev
)
{
    GT_STATUS st = GT_OK;

    if ((NULL == portPtr) || (dev >= PRV_CPSS_MAX_PP_DEVICES_CNS))
    {
        return GT_FAIL;
    }

    /* init values for iteration thru phy ports */
    extCtx.devForPhyPorts = dev;
    *portPtr = ITERATOR_INIT_VALUE_CNS;

    /* device must be active */
    if (0 == extCtx.devArray[dev])
    {
        st = GT_FAIL;
    }
    return st;
}

/**
* @internal prvUtfNextPhyPortGet function
* @endinternal
*
* @brief   This routine returns next active/non active physical port id.
*         Call the function until non GT_OK error. Note that
*         iterator must be initialized before.
* @param[in,out] portPtr                  -   port id
* @param[in] activeIs                 -   set to GT_TRUE for getting active ports
*/
GT_STATUS prvUtfNextPhyPortGet
(
    INOUT GT_PHYSICAL_PORT_NUM     *portPtr,
    IN GT_BOOL      activeIs
)
{
    GT_STATUS st = GT_OK;
    GT_BOOL portValidIs;

    if (NULL == portPtr)
    {
        return GT_FAIL;
    }

    st = GT_FAIL;

    if((*portPtr) == ITERATOR_INIT_VALUE_CNS)
    {
        (*portPtr) = 0;
    }
    else
    {
        (*portPtr) += (GT_TRUE == prvUtfReduceLogSizeFlagGet()) ? 64 : 1;
    }

    while (*portPtr < extCtx.phyNumPortsArray[extCtx.devForPhyPorts])
    {
        if (activeIs == GT_TRUE)
        {
            portValidIs = (GT_BOOL)(GT_OK == phyPortCheck(extCtx.devForPhyPorts, *portPtr));
        }
        else
        {
            /* Do not return CPU port if activeIs == GT_FALSE */
            portValidIs = (GT_BOOL)(GT_OK == phyPortOrCpuCheck(extCtx.devForPhyPorts, *portPtr));
        }

        if (activeIs == portValidIs)
        {
            st = GT_OK;
            break;
        }
        (*portPtr)++;
    }

    if((activeIs == GT_FALSE) && (st == GT_FAIL))/* no 'non active' port found within the virtual ports range */
    {
        if (GT_TRUE == prvUtfIsCpuPort(extCtx.devForPhyPorts, (*portPtr)))
        {
            /* Do not return CPU port if activeIs == GT_FALSE */
            (*portPtr)++;/* the caller test must check 'physical' CPU port in sperate means */
        }

        return nextOutOfRangePortGet(portPtr,extCtx.phyNumPortsArray[extCtx.devForPhyPorts]);
    }

    return st;
}

/**
* @internal prvUtfNextVirtPortReset function
* @endinternal
*
* @brief   Reset virtual ports iterator must be called before go over all ports
*
* @param[in,out] portPtr                  -   iterator thru ports
* @param[in] hwDev                    -   HW device id of ports
*
* @retval GT_OK                    -  iterator initialization OK
* @retval GT_FAIL                  -  general failure error
*/
GT_STATUS prvUtfNextVirtPortReset
(
    INOUT GT_PORT_NUM     *portPtr,
    IN    GT_HW_DEV_NUM    hwDev
)
{
    GT_STATUS st = GT_OK;

    /* need to fix code to support lion2 - change PRV_CPSS_MAX_PP_DEVICES_CNS,
       change extCtx.devForVirtPorts to be HW_DEV_NUM */
    CPSS_TBD_BOOKMARK_EARCH

    if ((NULL == portPtr) || (hwDev >= PRV_CPSS_MAX_PP_DEVICES_CNS))
    {
        return GT_FAIL;
    }

    /* init values for iteration thru phy ports */
    extCtx.devForVirtPorts = (GT_U8)hwDev;
    *portPtr = ITERATOR_INIT_VALUE_CNS;

    /* device must be active */
    if (0 == extCtx.devArray[hwDev])
    {
        st = GT_FAIL;
    }
    return st;
}

/**
* @internal prvUtfNextVirtPortGet function
* @endinternal
*
* @brief   This routine returns next active/non active virtual port id.
*         Call the function until non GT_OK error. Note that
*         iterator must be initialized before.
* @param[in,out] portPtr                  -   port id
* @param[in] activeIs                 -   set to GT_TRUE for getting active ports
*/
GT_STATUS prvUtfNextVirtPortGet
(
    INOUT GT_PORT_NUM       *portPtr,
    IN GT_BOOL              activeIs
)
{
    GT_STATUS st = GT_FAIL;

#ifdef CHX_FAMILY
    GT_U8     dev = extCtx.devForVirtPorts;
#endif

    if (NULL == portPtr)
    {
        return GT_FAIL;
    }

#ifdef CHX_FAMILY
    /* for port < 128 step is 1, for port => 128 step is 101 */
    if((*portPtr) == ITERATOR_INIT_VALUE_CNS ||
        (0 == UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)) ||
        ((*portPtr) < PRV_CPSS_PP_MAC(dev)->numOfPorts))
#endif
    {
        if((*portPtr) == ITERATOR_INIT_VALUE_CNS)
        {
            (*portPtr) = 0;
        }
        else if (GT_TRUE == prvUtfReduceLogSizeFlagGet())
        {
            (*portPtr) += UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev) / 3;
        }
        else
        {
            (*portPtr)++;
        }
    }
#ifdef CHX_FAMILY
    else /* eArch and port >= 'max physical ports' */
    {
        if (GT_TRUE == prvUtfReduceLogSizeFlagGet())
        {
            (*portPtr) += (_8K - PRV_CPSS_PP_MAC(dev)->numOfPorts) / 3;
        }
        else
        {
            /* do larger steps due to large range of ePorts ... to keep tests runtime to reasonable length */
            (*portPtr) += UTF_E_ARCH_NEXT_PORT_STEP_CNS;
#ifdef GM_USED
            /* due to performance issues , reduce the iterations even more ... */
            (*portPtr) += 7*UTF_E_ARCH_NEXT_PORT_STEP_CNS;
#endif /*GM_USED*/
        }
    }
#endif /*CHX_FAMILY*/

    while (*portPtr < extCtx.virtNumPortsArray[extCtx.devForVirtPorts])
    {
        /* skip CPU port from both active and not active ports */
        if (GT_FALSE == prvUtfIsCpuPort(extCtx.devForVirtPorts, (*portPtr)))
        {
            GT_BOOL portValidIs = (GT_BOOL)(GT_OK == virtPortCheck(extCtx.devForVirtPorts, *portPtr));
            if (activeIs == portValidIs)
            {
                st = GT_OK;
                break;
            }
        }
        (*portPtr)++;
    }

    if((activeIs == GT_FALSE) && (st == GT_FAIL))/* no 'non active' port found within the virtual ports range */
    {
#ifdef CHX_FAMILY
        return nextOutOfRangePortGet(portPtr,extCtx.virtNumPortsArray[extCtx.devForVirtPorts]);
#else
        /* the puma supports 54 virtual ports + cpu port ,
        but caller of this function sometime use the port just as '6 bits value' */
        if((*portPtr) < 64)
        {
            (*portPtr) = 64;
        }

        return nextOutOfRangePortGet(portPtr,64);
#endif /*EXMXPM_FAMILY*/
    }

    return st;

}

/**
* @internal prvUtfNextMacPortReset function
* @endinternal
*
* @brief   Reset MAC ports iterator must be called before go over all ports
*
* @param[in,out] portPtr                  -   iterator thru ports
* @param[in] dev                      -   device id of ports
*
* @retval GT_OK                    -  iterator initialization OK
* @retval GT_FAIL                  -  general failure error
*/
GT_STATUS prvUtfNextMacPortReset
(
    INOUT GT_PHYSICAL_PORT_NUM  *portPtr,
    IN    GT_U8                 dev
)
{
    GT_STATUS st = GT_OK;

    /* need to fix code to support lion2 - change PRV_CPSS_MAX_PP_DEVICES_CNS,
       change extCtx.devForVirtPorts to be HW_DEV_NUM */
    CPSS_TBD_BOOKMARK_EARCH

    if ((NULL == portPtr) || (dev >= PRV_CPSS_MAX_PP_DEVICES_CNS))
    {
        return GT_FAIL;
    }

    /* init values for iteration thru phy ports */
    extCtx.devForPhyPorts = (GT_U8)dev;
    *portPtr = ITERATOR_INIT_VALUE_CNS;

    /* device must be active */
    if (0 == extCtx.devArray[dev])
    {
        st = GT_FAIL;
    }
    return st;
}


/**
* @internal prvUtfNextMacPortGet function
* @endinternal
*
* @brief   This routine returns next active/non active MAC port id.
*         Call the function until non GT_OK error. Note that
*         iterator must be initialized before.
* @param[in,out] portPtr                  -   port id
* @param[in] activeIs                 -   set to GT_TRUE for getting active ports
*/
GT_STATUS prvUtfNextMacPortGet
(
    INOUT GT_PHYSICAL_PORT_NUM  *portPtr,
    IN GT_BOOL                  activeIs
)
{
    GT_STATUS st = GT_OK;
    GT_U8     dev = extCtx.devForPhyPorts;

    if (NULL == portPtr)
    {
        return GT_FAIL;
    }

    st = GT_FAIL;

    if(!PRV_CPSS_SIP_5_CHECK_MAC(dev))
    {
        return prvUtfNextPhyPortGet(portPtr, activeIs);
    }

    if((*portPtr) == ITERATOR_INIT_VALUE_CNS)
    {
        (*portPtr) = 0;
    }
    else
    {
        (*portPtr) += (GT_TRUE == prvUtfReduceLogSizeFlagGet()) ? 18 : 1;
    }

    while (*portPtr < extCtx.phyNumPortsArray[extCtx.devForPhyPorts])
    {
        /* skip CPU port from both active and not active ports */
        if (GT_FALSE != prvUtfIsCpuPort(extCtx.devForPhyPorts, (*portPtr)))
        {
            (*portPtr)++;
            continue;
        }
        if (activeIs != GT_FALSE)
        {
            /* active - all ports with valid mac */
            if (GT_OK == macPortCheck(extCtx.devForPhyPorts, *portPtr))
            {
                st = GT_OK;
                break;
            }
        }
        else
        {
            /* not active  - invalid physical ports in range */
            if (GT_OK != phyPortOrCpuCheck(extCtx.devForPhyPorts, *portPtr))
            {
                st = GT_OK;
                break;
            }
        }
        (*portPtr)++;
    }

    if((activeIs == GT_FALSE) && (st == GT_FAIL))/* no 'non active' port found within the virtual ports range */
    {
        return nextOutOfRangePortGet(portPtr,extCtx.phyNumPortsArray[extCtx.devForPhyPorts]);
    }

    return st;
}

/**
* @internal prvUtfNextTxqPortReset function
* @endinternal
*
* @brief   Reset TXQ ports iterator must be called before go over all ports
*
* @param[in,out] portPtr                  -   iterator thru ports
* @param[in] dev                      -   device id of ports
*
* @retval GT_OK                    -  iterator initialization OK
* @retval GT_FAIL                  -  general failure error
*/
GT_STATUS prvUtfNextTxqPortReset
(
    INOUT GT_PHYSICAL_PORT_NUM  *portPtr,
    IN    GT_U8                 dev
)
{
    return prvUtfNextMacPortReset(portPtr,dev);
}

/**
* @internal prvUtfNextTxqPortGet function
* @endinternal
*
* @brief   This routine returns next active/non active TXQ port id.
*         Call the function until non GT_OK error. Note that
*         iterator must be initialized before.
* @param[in,out] portPtr                  -   port id
* @param[in] activeIs                 -   set to GT_TRUE for getting active ports
*/
GT_STATUS prvUtfNextTxqPortGet
(
    INOUT GT_PHYSICAL_PORT_NUM  *portPtr,
    IN GT_BOOL                  activeIs
)
{
    GT_STATUS st = GT_FAIL;
    GT_U8     dev = extCtx.devForPhyPorts;

    if (NULL == portPtr)
    {
        return GT_FAIL;
    }

    if(!PRV_CPSS_SIP_5_CHECK_MAC(dev))
    {
        return prvUtfNextPhyPortGet(portPtr, activeIs);
    }

    if((*portPtr) == ITERATOR_INIT_VALUE_CNS)
    {
        (*portPtr) = 0;
    }
    else
    {
        (*portPtr) += (GT_TRUE == prvUtfReduceLogSizeFlagGet()) ? 18 : 1;
    }

    while (*portPtr < extCtx.phyNumPortsArray[extCtx.devForPhyPorts])
    {
        /* skip CPU port from both active and not active ports */
        if (GT_FALSE == prvUtfIsCpuPort(extCtx.devForPhyPorts, (*portPtr)))
        {
            GT_BOOL portValidIs = (GT_BOOL)(GT_OK == txqPortCheck(extCtx.devForPhyPorts, *portPtr));
            if (activeIs == portValidIs)
            {
                st = GT_OK;
                break;
            }
        }
        (*portPtr)++;
    }

    if((activeIs == GT_FALSE) && (st == GT_FAIL))/* no 'non active' port found within the virtual ports range */
    {
        return nextOutOfRangePortGet(portPtr,extCtx.phyNumPortsArray[extCtx.devForPhyPorts]);
    }

    return st;
}

/**
* @internal prvUtfNextRxDmaPortReset function
* @endinternal
*
* @brief   Reset RxDma ports iterator. Must be called before go over all ports.
*
* @param[in,out] portPtr                  -   iterator thru ports
* @param[in] dev                      -   device id of ports
*
* @retval GT_OK                    -  iterator initialization OK
* @retval GT_FAIL                  -  general failure error
*/
GT_STATUS prvUtfNextRxDmaPortReset
(
    INOUT   GT_PHYSICAL_PORT_NUM    *portPtr,
    IN      GT_U8                   dev
)
{
    GT_STATUS st = GT_OK;

    /* need to fix code to support lion2 - change PRV_CPSS_MAX_PP_DEVICES_CNS,
       change extCtx.devForVirtPorts to be HW_DEV_NUM */
    CPSS_TBD_BOOKMARK_EARCH

    if ((NULL == portPtr) || (dev >= PRV_CPSS_MAX_PP_DEVICES_CNS))
    {
        return GT_FAIL;
    }

    /* init values for iteration thru phy ports */
    extCtx.devForPhyPorts = (GT_U8)dev;
    *portPtr = ITERATOR_INIT_VALUE_CNS;

    /* device must be active */
    if (0 == extCtx.devArray[dev])
    {
        st = GT_FAIL;
    }
    return st;
}

/**
* @internal prvUtfNextRxDmaPortGet function
* @endinternal
*
* @brief   This routine returns next active/non active RxDma port id.
*         Call the function until non GT_OK error.
*         Note that iterator must be initialized before.
* @param[in,out] portPtr                  -   port id
* @param[in] activeIs                 -   set to GT_TRUE for getting active ports
*/
GT_STATUS prvUtfNextRxDmaPortGet
(
    INOUT   GT_PHYSICAL_PORT_NUM    *portPtr,
    IN      GT_BOOL                 activeIs
)
{
    GT_STATUS st = GT_FAIL;
    GT_U8     dev = extCtx.devForPhyPorts;

    if (NULL == portPtr)
    {
        return GT_FAIL;
    }

    if(!PRV_CPSS_SIP_5_CHECK_MAC(dev))
    {
        return prvUtfNextPhyPortGet(portPtr, activeIs);
    }

    if((*portPtr) == ITERATOR_INIT_VALUE_CNS)
    {
        (*portPtr) = 0;
    }
    else
    {
        (*portPtr) += (GT_TRUE == prvUtfReduceLogSizeFlagGet()) ? 18 : 1;
    }

    while (*portPtr < extCtx.phyNumPortsArray[extCtx.devForPhyPorts])
    {
        /* skip CPU port from both active and not active ports */
        if (GT_FALSE == prvUtfIsCpuPort(extCtx.devForPhyPorts, (*portPtr)))
        {
            GT_BOOL portValidIs = (GT_BOOL)(GT_OK == rxDmaPortCheck(extCtx.devForPhyPorts, *portPtr));
            if (activeIs == portValidIs)
            {
                st = GT_OK;
                break;
            }
        }
        (*portPtr)++;
    }

    if((activeIs == GT_FALSE) && (st == GT_FAIL))/* no 'non active' port found within the virtual ports range */
    {
        return nextOutOfRangePortGet(portPtr,extCtx.phyNumPortsArray[extCtx.devForPhyPorts]);
    }

    return st;
}

/**
* @internal prvUtfNextTxDmaPortReset function
* @endinternal
*
* @brief   Reset TxDma ports iterator. Must be called before go over all ports.
*
* @param[in,out] portPtr                  -   iterator thru ports
* @param[in] dev                      -   device id of ports
*
* @retval GT_OK                    -  iterator initialization OK
* @retval GT_FAIL                  -  general failure error
*/
GT_STATUS prvUtfNextTxDmaPortReset
(
    INOUT   GT_PHYSICAL_PORT_NUM    *portPtr,
    IN      GT_U8                   dev
)
{
    GT_STATUS st = GT_OK;

    /* need to fix code to support lion2 - change PRV_CPSS_MAX_PP_DEVICES_CNS,
       change extCtx.devForVirtPorts to be HW_DEV_NUM */
    CPSS_TBD_BOOKMARK_EARCH

    if ((NULL == portPtr) || (dev >= PRV_CPSS_MAX_PP_DEVICES_CNS))
    {
        return GT_FAIL;
    }

    /* init values for iteration thru phy ports */
    extCtx.devForPhyPorts = (GT_U8)dev;
    *portPtr = ITERATOR_INIT_VALUE_CNS;

    /* device must be active */
    if (0 == extCtx.devArray[dev])
    {
        st = GT_FAIL;
    }
    return st;
}

/**
* @internal prvUtfNextTxDmaPortGet function
* @endinternal
*
* @brief   This routine returns next active/non active TxDma port id.
*         Call the function until non GT_OK error.
*         Note that iterator must be initialized before.
* @param[in,out] portPtr                  -   port id
* @param[in] activeIs                 -   set to GT_TRUE for getting active ports
*/
GT_STATUS prvUtfNextTxDmaPortGet
(
    INOUT   GT_PHYSICAL_PORT_NUM    *portPtr,
    IN      GT_BOOL                 activeIs
)
{

    GT_STATUS st = GT_FAIL;
    GT_U8     dev = extCtx.devForPhyPorts;

    if (NULL == portPtr)
    {
        return GT_FAIL;
    }

    if(!PRV_CPSS_SIP_5_CHECK_MAC(dev))
    {
        return prvUtfNextPhyPortGet(portPtr, activeIs);
    }

    if((*portPtr) == ITERATOR_INIT_VALUE_CNS)
    {
        (*portPtr) = 0;
    }
    else
    {
        (*portPtr) += (GT_TRUE == prvUtfReduceLogSizeFlagGet()) ? 18 : 1;
    }

    while (*portPtr < extCtx.phyNumPortsArray[extCtx.devForPhyPorts])
    {
        /* skip CPU port from both active and not active ports */
        if (GT_FALSE == prvUtfIsCpuPort(extCtx.devForPhyPorts, (*portPtr)))
        {
            GT_BOOL portValidIs = (GT_BOOL)(GT_OK == txDmaPortCheck(extCtx.devForPhyPorts, *portPtr));
            if (activeIs == portValidIs)
            {
                st = GT_OK;
                break;
            }
        }
        (*portPtr)++;
    }

    if((activeIs == GT_FALSE) && (st == GT_FAIL))/* no 'non active' port found within the virtual ports range */
    {
        return nextOutOfRangePortGet(portPtr,extCtx.phyNumPortsArray[extCtx.devForPhyPorts]);
    }

    return st;
}

/**
* @internal prvUtfNextDefaultEPortReset function
* @endinternal
*
* @brief   Reset 'Default ePort' iterator must be called before go over all default ePorts.
*         for device that not support default ePorts this iteration is just like
*         prvUtfNextPhyPortReset().
* @param[in] hwDev                    -   HW device id of ports
*
* @param[out] portPtr                  -   (pointer to) first iteration
*
* @retval GT_OK                    -  iterator initialization OK
* @retval GT_FAIL                  -  general failure error
*/
GT_STATUS prvUtfNextDefaultEPortReset
(
    OUT   GT_PORT_NUM     *portPtr,
    IN    GT_HW_DEV_NUM    hwDev
)
{
#ifndef CHX_FAMILY
    return prvUtfNextVirtPortReset(portPtr,hwDev);
#endif
#ifdef CHX_FAMILY
    GT_STATUS st = GT_OK;

    /* need to fix code to support lion2 - change PRV_CPSS_MAX_PP_DEVICES_CNS,
       change extCtx.devForVirtPorts to be HW_DEV_NUM */
    CPSS_TBD_BOOKMARK_EARCH

    if ((NULL == portPtr) || (hwDev >= PRV_CPSS_MAX_PP_DEVICES_CNS))
    {
        return GT_FAIL;
    }

    /* init values for iteration thru phy ports */
    extCtx.devForVirtPorts = (GT_U8)hwDev;

    if(0 == UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(hwDev))
    {
        return prvUtfNextPhyPortReset(portPtr,(GT_U8)hwDev);
    }

    *portPtr = ITERATOR_INIT_VALUE_CNS;

    /* device must be active */
    if (0 == extCtx.devArray[hwDev])
    {
        st = GT_FAIL;
    }
    return st;
#endif /* CHX_FAMILY */
}

#ifdef CHX_FAMILY
/* Returns GT_OK if default ePort is active */
static GT_STATUS defaultEPortCheck(IN GT_U8 dev, IN GT_PORT_NUM port)
{
    PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(dev, port)

    return GT_OK;
}
#endif /* CHX_FAMILY */

/**
* @internal prvUtfNextDefaultEPortGet function
* @endinternal
*
* @brief   This routine returns next active/non active 'default ePort' id.
*         for device that not support default ePorts this iteration is just like
*         prvUtfNextPhyPortGet().
*         Call the function until non GT_OK error. Note that
*         iterator must be initialized before.
* @param[in,out] portPtr                  -   port id
* @param[in] activeIs                 -   set to GT_TRUE for getting active ports
*/
GT_STATUS prvUtfNextDefaultEPortGet
(
    INOUT GT_PORT_NUM       *portPtr,
    IN GT_BOOL              activeIs
)
{
#ifndef CHX_FAMILY
    return prvUtfNextVirtPortGet(portPtr,activeIs);
#endif

#ifdef CHX_FAMILY
    if(0 == UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(extCtx.devForVirtPorts))
    {
        return prvUtfNextPhyPortGet(portPtr,activeIs);
    }

    if (NULL == portPtr)
    {
        return GT_FAIL;
    }

    if((*portPtr) == ITERATOR_INIT_VALUE_CNS)
    {
        (*portPtr) = 0;
    }
    else
    {
        (*portPtr) += (GT_TRUE == prvUtfReduceLogSizeFlagGet()) ? 128 : 1;
    }

    if(activeIs == GT_FALSE)/* no 'non active' port found within the virtual ports range */
    {
        return nextOutOfRangePortGet(portPtr,PRV_CPSS_DXCH_MAX_DEFAULT_EPORT_NUMBER_MAC(extCtx.devForVirtPorts));
    }


    if(GT_OK != defaultEPortCheck(extCtx.devForVirtPorts, *portPtr))
    {
        return GT_FAIL;
    }

    return GT_OK;
#endif /* CHX_FAMILY */
}

/**
* @internal prvUtfMaxDefaultEPortNumberGet function
* @endinternal
*
* @brief   Get the number of default ePorts.
*
* @param[out] defaultEPortsNum         - number of defauls ePorts.
*                                      GT_OK           -   Get next port was OK
*                                      GT_FAIL         -   There is no active ports
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS prvUtfMaxDefaultEPortNumberGet
(
    OUT GT_PORT_NUM       *defaultEPortsNum
)
{
#ifdef CHX_FAMILY
    if(1 == UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(extCtx.devForVirtPorts))
    {
        *defaultEPortsNum = PRV_CPSS_DXCH_MAX_DEFAULT_EPORT_NUMBER_MAC(extCtx.devForVirtPorts);
    }
    else
#endif /* CHX_FAMILY */
    {
        *defaultEPortsNum = 1;
    }

    return GT_OK;
}

/**
* @internal prvUtfPortTypeGet function
* @endinternal
*
* @brief   This routine returns port type.
*
* @param[in] dev                      -  device id
* @param[in] port                     -  physical port
*
* @param[out] portTypePtr              -   type of port
*                                      GT_OK           -   Get port type was OK
*                                      GT_BAD_PARAM    -   Invalid port or device id
*                                      GT_BAD_PTR      -   Null pointer
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS prvUtfPortTypeGet
(
    IN GT_U8                    dev,
    IN GT_PHYSICAL_PORT_NUM     port,
    OUT PRV_CPSS_PORT_TYPE_ENT  *portTypePtr
)
{
    GT_STATUS st = GT_OK;
    GT_U32    macPortNum = port;
#if (defined CHX_FAMILY)
    CPSS_PORT_MAC_TYPE_ENT  portMacType;    /* Intermediate result */
#endif

    PRV_CPSS_GEN_PP_CONFIG_STC  *devPtr;

    DEV_CHECK_MAC(dev);

    /* check for null pointer */
    CPSS_NULL_PTR_CHECK_MAC(portTypePtr);

#if (defined CHX_FAMILY)
    if(prvCpssDxChPortRemotePortCheck(dev,port))
    {

        st = cpssDxChPortMacTypeGet (dev, port, &portMacType);
        switch(portMacType)
        {
            case CPSS_PORT_MAC_TYPE_NOT_EXISTS_E:
                *portTypePtr = PRV_CPSS_PORT_NOT_EXISTS_E;
                break;

            case CPSS_PORT_MAC_TYPE_FE_E:
                *portTypePtr = PRV_CPSS_PORT_FE_E;
                break;

            case CPSS_PORT_MAC_TYPE_GE_E:
                *portTypePtr = PRV_CPSS_PORT_GE_E;
                break;

            case CPSS_PORT_MAC_TYPE_XG_E:
                *portTypePtr = PRV_CPSS_PORT_XG_E;
                break;

            case CPSS_PORT_MAC_TYPE_XLG_E:
                *portTypePtr = PRV_CPSS_PORT_XLG_E;
                break;

            case CPSS_PORT_MAC_TYPE_HGL_E:
                *portTypePtr = PRV_CPSS_PORT_HGL_E;
                break;

            case CPSS_PORT_MAC_TYPE_CG_E:
                *portTypePtr = PRV_CPSS_PORT_CG_E;
                break;

            case CPSS_PORT_MAC_TYPE_ILKN_E:
                *portTypePtr = PRV_CPSS_PORT_ILKN_E;
                break;

            case CPSS_PORT_MAC_TYPE_NOT_APPLICABLE_E:
                *portTypePtr = PRV_CPSS_PORT_NOT_APPLICABLE_E;
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

    }
    else
#endif /*CHX_FAMILY*/
    {
        PORT_TO_MAC_CHECK_MAC(dev, port, macPortNum);

        devPtr = PRV_CPSS_PP_MAC(dev);

        /* Get port type */
        *portTypePtr = devPtr->phyPortInfoArray[macPortNum].portType;
    }
    return st;
}

/**
* @internal prvUtfPortMacNumberGet function
* @endinternal
*
* @brief   This routine returns MAC number of a port.
*
* @param[in] dev                      -  device id
* @param[in] port                     -  physical port
*
* @param[out] macNumPtr                -   MAC number of port
*                                      GT_OK           -   Get port type was OK
*                                      GT_BAD_PARAM    -   Invalid port or device id
*                                      GT_BAD_PTR      -   Null pointer
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS prvUtfPortMacNumberGet
(
    IN GT_U8                    dev,
    IN GT_PHYSICAL_PORT_NUM     port,
    OUT GT_U32                 *macNumPtr
)
{
    GT_STATUS st = GT_OK;
    GT_U32    macPortNum = port;

    DEV_CHECK_MAC(dev);

    PORT_TO_MAC_CHECK_MAC(dev, port, macPortNum);

    /* Get port type */
    *macNumPtr = macPortNum;
    return st;
}

/**
* @internal prvUtfPortMacModuloCalc function
* @endinternal
*
* @brief   This routine returns modulo of MAC number of a port.
*
* @param[in] dev                      -  device id
* @param[in] port                     -  physical port
* @param[in] modulo                   -  modulo
*
* @param[out] modulo                   of MAC number of port
*                                      COMMENTS:
*                                      None.
*/
GT_U32 prvUtfPortMacModuloCalc
(
    IN GT_U8                    dev,
    IN GT_PHYSICAL_PORT_NUM     port,
    IN GT_U32                   modulo
)
{
    GT_STATUS st;
    GT_U32    macPortNum = port;

    st = prvUtfPortMacNumberGet(dev, port, &macPortNum);
    if (st != GT_OK)
    {
        return 0;
    }

    return (macPortNum % modulo);
}

/**
* @internal prvUtfPortInterfaceModeGet function
* @endinternal
*
* @brief   This routine returns port interface mode.
*
* @param[in] dev                      -  device id
* @param[in] port                     -  physical port
*
* @param[out] portIfModePtr            -   (pointer to) port interface mode
*                                      GT_OK           -   Get port type was OK
*                                      GT_BAD_PARAM    -   Invalid port or device id
*                                      GT_BAD_PTR      -   Null pointer
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS prvUtfPortInterfaceModeGet
(
    IN GT_U8                            dev,
    IN GT_PHYSICAL_PORT_NUM             port,
    OUT CPSS_PORT_INTERFACE_MODE_ENT    *portIfModePtr
)
{
    GT_STATUS st = GT_OK;
    GT_U32    macPortNum = port;

    PRV_CPSS_GEN_PP_CONFIG_STC  *devPtr;

    DEV_CHECK_MAC(dev);

    devPtr = PRV_CPSS_PP_MAC(dev);

    if(devPtr->devFamily == CPSS_PX_FAMILY_PIPE_E)
    {
#ifdef PX_FAMILY
        PX_PORT_TO_MAC_IN_PHY_OR_CPU_PORT_CHECK_MAC(dev, port, macPortNum);
#endif
    }
    else
    {
#ifdef CHX_FAMILY
        PORT_TO_MAC_IN_PHY_OR_CPU_PORT_CHECK_MAC(dev, port, macPortNum);
#endif
    }

    /* check for null pointer */
    CPSS_NULL_PTR_CHECK_MAC(portIfModePtr);

    /* Get port type */
    *portIfModePtr = devPtr->phyPortInfoArray[macPortNum].portIfMode;
    return st;
}


/**
* @internal prvUtfPortSpeedGet function
* @endinternal
*
* @brief   This routine returns port speed.
*
* @param[in] dev                      -  device id
* @param[in] port                     -  physical port
*
* @param[out] portSpeedPtr             -   (pointer to) port speed
*                                      GT_OK           -   Get port type was OK
*                                      GT_BAD_PARAM    -   Invalid port or device id
*                                      GT_BAD_PTR      -   Null pointer
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS prvUtfPortSpeedGet
(
    IN GT_U8                    dev,
    IN GT_PHYSICAL_PORT_NUM     port,
    OUT CPSS_PORT_SPEED_ENT     *portSpeedPtr
)
{
    GT_STATUS st = GT_OK;
    GT_U32    macPortNum = port;

    PRV_CPSS_GEN_PP_CONFIG_STC  *devPtr;

    DEV_CHECK_MAC(dev);

    PORT_TO_MAC_CHECK_MAC(dev, port, macPortNum);

    /* check for null pointer */
    CPSS_NULL_PTR_CHECK_MAC(portSpeedPtr);

    devPtr = PRV_CPSS_PP_MAC(dev);

    /* Get port type */
    *portSpeedPtr = devPtr->phyPortInfoArray[macPortNum].portSpeed;
    return st;
}


/**
* @internal prvUtfPortTypeOptionsGet function
* @endinternal
*
* @brief   This routine returns port type options.
*
* @param[in] dev                      -  device id
* @param[in] port                     -  physical port
*
* @param[out] portTypeOptionsPtr       -   type of port
*                                      GT_OK           -   Get port type was OK
*                                      GT_BAD_PARAM    -   Invalid port or device id
*                                      GT_BAD_PTR      -   Null pointer
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS prvUtfPortTypeOptionsGet
(
    IN GT_U8                            dev,
    IN GT_PHYSICAL_PORT_NUM             port,
    OUT PRV_CPSS_PORT_TYPE_OPTIONS_ENT  *portTypeOptionsPtr
)
{
    GT_STATUS st = GT_OK;
    GT_U32    macPortNum = port;

    PRV_CPSS_GEN_PP_CONFIG_STC  *devPtr;

    DEV_CHECK_MAC(dev);

    PORT_TO_MAC_IN_PHY_CHECK_MAC(dev, port, macPortNum);

#if (defined CHX_FAMILY)
    if (GT_TRUE == prvCpssDxChPortRemotePortCheck(dev,port))
    {
        *portTypeOptionsPtr = PRV_CPSS_GE_PORT_GE_ONLY_E;
    }
    else
#endif
    {
        /* check for null pointer */
        CPSS_NULL_PTR_CHECK_MAC(portTypeOptionsPtr);

        devPtr = PRV_CPSS_PP_MAC(dev);

        /* Get port type */
        *portTypeOptionsPtr = devPtr->phyPortInfoArray[macPortNum].portTypeOptions;
    }
    return st;
}


/**
* @internal prvUtfPortIsFlexLinkGet function
* @endinternal
*
* @brief   This routine returns port is flex link flag.
*
* @param[in] dev                      -  device id
* @param[in] port                     -  physical port
*
* @param[out] isFlexLinkPtr            -   is flex link
*                                      GT_OK           -   Get port type was OK
*                                      GT_BAD_PARAM    -   Invalid port or device id
*                                      GT_BAD_PTR      -   Null pointer
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS prvUtfPortIsFlexLinkGet
(
    IN GT_U8                    dev,
    IN GT_PHYSICAL_PORT_NUM     port,
    OUT GT_BOOL                 *isFlexLinkPtr
)
{
    GT_STATUS st = GT_OK;
    GT_U32    macPortNum = port;

    PRV_CPSS_GEN_PP_CONFIG_STC  *devPtr;

    DEV_CHECK_MAC(dev);

    PORT_TO_MAC_IN_PHY_CHECK_MAC(dev, port, macPortNum);

#if (defined CHX_FAMILY)
    if (GT_TRUE == prvCpssDxChPortRemotePortCheck(dev,port))
    {
        *isFlexLinkPtr = GT_FALSE;
    }
    else
#endif
    {
        /* check for null pointer */
        CPSS_NULL_PTR_CHECK_MAC(isFlexLinkPtr);

        devPtr = PRV_CPSS_PP_MAC(dev);

        /* Get port type */
        *isFlexLinkPtr = devPtr->phyPortInfoArray[macPortNum].isFlexLink;
    }
    return st;
}


/**
* @internal prvUtfDeviceTypeGet function
* @endinternal
*
* @brief   This routine returns device type. Some tests require device type
*         to make proper function behaviour validation.
* @param[in] dev                      -  device id
*
* @param[out] devTypePtr               -   type of device
*                                      GT_OK           -   Get device type was OK
*                                      GT_BAD_PARAM    -   Invalid device id
*                                      GT_BAD_PTR      -   Null pointer
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS prvUtfDeviceTypeGet
(
    IN GT_U8                    dev,
    OUT CPSS_PP_DEVICE_TYPE     *devTypePtr
)
{
    GT_STATUS st = GT_OK;

    PRV_CPSS_GEN_PP_CONFIG_STC  *devPtr;

    DEV_CHECK_MAC(dev);

    /* check for null pointer */
    CPSS_NULL_PTR_CHECK_MAC(devTypePtr);

    devPtr = PRV_CPSS_PP_MAC(dev);

    /* Get device type */
    *devTypePtr = devPtr->devType;
    return st;
}

/**
* @internal prvUtfDeviceFamilyGet function
* @endinternal
*
* @brief   This routine returns device family. Some tests require device family
*         to make proper function behaviour validation.
* @param[in] dev                      -  device id
*
* @param[out] devFamilyPtr             -   family of device
*                                      GT_OK           -   Get device family was OK
*                                      GT_BAD_PARAM    -   Invalid device id
*                                      GT_BAD_PTR      -   Null pointer
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS prvUtfDeviceFamilyGet
(
    IN GT_U8                        dev,
    OUT CPSS_PP_FAMILY_TYPE_ENT     *devFamilyPtr
)
{
    DEV_CHECK_MAC(dev);

    /* check for null pointer */
    CPSS_NULL_PTR_CHECK_MAC(devFamilyPtr);

    /* Get device family */
    *devFamilyPtr = PRV_CPSS_PP_MAC(dev)->devFamily;

    return GT_OK;
}

/**
* @internal prvUtfMaxMcGroupsGet function
* @endinternal
*
* @brief   This routine returns maximum value for MC groups per device.
*
* @param[in] dev                      -  device id
*
* @param[out] maxMcGroupsPtr           -   maximum value for multicast groups
*                                      GT_OK           -   Get max MC groups was OK
*                                      GT_BAD_PARAM    -   Invalid device id
*                                      GT_BAD_PTR      -   Null pointer
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS prvUtfMaxMcGroupsGet
(
    IN GT_U8    dev,
    OUT GT_U16  *maxMcGroupsPtr
)
{
    DEV_CHECK_MAC(dev);

    /* check for null pointer */
    CPSS_NULL_PTR_CHECK_MAC(maxMcGroupsPtr);

#ifdef CALL_EX_MX_CODE_MAC
    if(PRV_CPSS_EXMX_FAMILY_CHECK_MAC(dev))
    {
        /* ExMx devices */
        *maxMcGroupsPtr = PRV_CPSS_EXMX_DEV_MODULE_CFG_MAC(dev)->bridgeCfg.maxMcGroups;
    }
    else
#endif /* CALL_EX_MX_CODE_MAC */
    {
        *maxMcGroupsPtr = 0;
    }

    return GT_OK;
}

/**
* @internal prvUtfNumPortsGet function
* @endinternal
*
* @brief   get the number of physical ports in the device
*
* @param[in] dev                      -   device id of ports
*
* @param[out] numOfPortsPtr            - (pointer to)number of physical ports
*
* @retval GT_OK                    -  get the number of physical ports in the device OK
* @retval GT_BAD_PARAM             -  Invalid device id
* @retval GT_BAD_PTR               -  Null pointer
*/
GT_STATUS prvUtfNumPortsGet
(
    IN GT_U8        dev,
    OUT GT_U32       *numOfPortsPtr
)
{
    /* device must be active */
    if (dev >= PRV_CPSS_MAX_PP_DEVICES_CNS ||  0 == extCtx.devArray[dev])
    {
        return GT_BAD_PARAM;
    }

    /* check for null pointer */
    CPSS_NULL_PTR_CHECK_MAC(numOfPortsPtr);

    *numOfPortsPtr = extCtx.phyNumPortsArray[dev];

    return GT_OK;
}

/**
* @internal prvUtfNumVirtualPortsGet function
* @endinternal
*
* @brief   get the number of Virtual ports in the device
*
* @param[in] dev                      -   device id of ports
*
* @retval GT_OK                    -  get the number of virtual ports in the device OK
* @retval GT_BAD_PARAM             -  Invalid device id
* @retval GT_BAD_PTR               -  Null pointer
*/
GT_STATUS prvUtfNumVirtualPortsGet
(
    IN GT_U8        dev,
    OUT GT_U32      *numOfVirtualPortsPtr
)
{
    /* device must be active */
    if (dev >= PRV_CPSS_MAX_PP_DEVICES_CNS || 0 == extCtx.devArray[dev])
    {
        return GT_BAD_PARAM;
    }

    /* check for null pointer */
    CPSS_NULL_PTR_CHECK_MAC(numOfVirtualPortsPtr);

    *numOfVirtualPortsPtr = extCtx.virtNumPortsArray[dev];

    return GT_OK;
}

/**
* @internal prvUtfDeviceRevisionGet function
* @endinternal
*
* @brief   Get revision number of device
*
* @param[in] dev                      -   device id of ports
*
* @param[out] revisionPtr              - (pointer to)revision number
*
* @retval GT_OK                    -  Get revision of device OK
* @retval GT_BAD_PARAM             -  Invalid device id
* @retval GT_BAD_PTR               -  Null pointer
*/
GT_STATUS prvUtfDeviceRevisionGet
(
    IN GT_U8        dev,
    OUT GT_U8       *revisionPtr
)
{
    /* device must be active */
    DEV_CHECK_MAC(dev);

    /* check for null pointer */
    CPSS_NULL_PTR_CHECK_MAC(revisionPtr);

    *revisionPtr = PRV_CPSS_PP_MAC(dev)->revision;

    return GT_OK;
}

/**
* @internal prvUtfDeviceTestSupport function
* @endinternal
*
* @brief   check if the device supported for the test type
*
* @param[in] devNum                   -   device id
* @param[in] utTestSupport            - bmp of test types support , see :
*                                      UTF_CPSS_PP_FAMILY_TWISTC_CNS
*                                      UTF_CPSS_PP_FAMILY_TWISTD_CNS
*                                      UTF_CPSS_PP_FAMILY_SAMBA_CNS
*                                      UTF_CPSS_PP_FAMILY_SALSA_CNS
*                                      UTF_CPSS_PP_FAMILY_SOHO_CNS
*                                      UTF_CPSS_PP_FAMILY_CHEETAH_CNS
*                                      UTF_CPSS_PP_FAMILY_CHEETAH2_CNS
*                                      UTF_CPSS_PP_FAMILY_CHEETAH3_CNS
*                                      UTF_CPSS_PP_FAMILY_TIGER_CNS
*                                      UTF_CPSS_PP_FAMILY_PUMA_CNS
*                                      UTF_CPSS_PP_FAMILY_XCAT_CNS
*                                      UTF_CPSS_PP_FAMILY_LION_A_CNS
*                                      UTF_CPSS_PP_FAMILY_LION_CNS
*
* @retval GT_TRUE                  - the device support the test type
* @retval GT_FALSE                 - the device not support the test type
*/
GT_BOOL prvUtfDeviceTestSupport
(
    IN GT_U8        devNum,
    IN GT_U32       utTestSupport
)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, utTestSupport);
    if(st != GT_OK)
    {
        return GT_FALSE;
    }

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(dev == devNum)
        {
            return GT_TRUE;
        }
    }

    return GT_FALSE;
}

/**
* @internal prvUtfDeviceTestNotSupport function
* @endinternal
*
* @brief   Check if the device is not supported for the test type
*
* @param[in] devNum                   - device id
* @param[in] notApplicableDeviceBmp   - bmp of not applicable device types
*
* @retval GT_TRUE                  - the device support the test type
* @retval GT_FALSE                 - the device not support the test type
*/
GT_BOOL prvUtfDeviceTestNotSupport
(
    IN GT_U8        devNum,
    IN GT_U32       notApplicableDeviceBmp
)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    /* prepare device iterator */
    st = prvUtfNextNotApplicableDeviceReset(&dev, notApplicableDeviceBmp);
    if(st != GT_OK)
    {
        return GT_FALSE;
    }

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(dev == devNum)
        {
            /* indicate that we are done with this iterator */
            if(utIteratorDepth)
            {
                utIteratorDepth--;
            }

            return GT_TRUE;
        }
    }

    return GT_FALSE;
}

/**
* @internal prvUtfSkipTestsNumReset function
* @endinternal
*
* @brief   Reset skip test counter
*/
GT_VOID prvUtfSkipTestsNumReset
(
    GT_VOID
)
{
    prvUtfSkipTestNum = 0;
}

/**
* @internal prvUtfSkipTestsNumGet function
* @endinternal
*
* @brief   Get total number of skipped tests
*/
GT_U32 prvUtfSkipTestsNumGet
(
    GT_VOID
)
{
    return prvUtfSkipTestNum;
}

/**
* @internal prvUtfSkipTestsFlagReset function
* @endinternal
*
* @brief   Reset skip test flag
*/
GT_VOID prvUtfSkipTestsFlagReset
(
    GT_VOID
)
{
    prvUtfIsTestSkip = GT_FALSE;
}

/**
* @internal prvUtfSkipTestsFlagGet function
* @endinternal
*
* @brief   Get current skip test flag
*/
GT_BOOL prvUtfSkipTestsFlagGet
(
    GT_VOID
)
{
    return prvUtfIsTestSkip;
}

/**
* @internal prvUtfSkipTestsSet function
* @endinternal
*
* @brief   Set skip flag and update number of skipped tests
*/
GT_VOID prvUtfSkipTestsSet
(
    GT_VOID
)
{
    /* mark current test as skipped */
    prvUtfIsTestSkip = GT_TRUE;

    /* increase number of skipped tests */
    prvUtfSkipTestNum++;
}

/**
* @internal prvUtfSkipLongTestsFlagSet function
* @endinternal
*
* @brief   Set skip long test flag
*
* @param[in] enable                   - enable\disable skip long tests
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvUtfSkipLongTestsFlagSet
(
    IN GT_BOOL enable
)
{
    prvUtfIsLongTestsSkip = enable;

    return GT_OK;
}

/**
* @internal prvUtfSkipLongTestsFlagGet function
* @endinternal
*
* @brief   Get skip long test flag
*
* @param[in] skipFamilyBmp            - bitmap of families for which skip flag applyed
*                                       skip long tests flag
*/
GT_BOOL prvUtfSkipLongTestsFlagGet
(
    IN GT_U32 skipFamilyBmp
)
{
    /* check if current device included in skip bmp */
    if (extCtx.devArray[prvTgfDevNum] & skipFamilyBmp)
    {
        return prvUtfIsLongTestsSkip;
    }
    else
    {
        /* current device is out of skip bmp -> do not need to skip */
        return GT_FALSE;
    }
}

/**
* @internal prvUtfCheckFamilyBmp function
* @endinternal
*
* @brief   Check that tested device is part of familyBmp
*
* @param[in] familyBmp                - bitmap of families
*
* @retval GT_TRUE                  - tested device is part of familyBmp
* @retval GT_FALSE                 - tested device is not part of familyBmp
*/
GT_BOOL prvUtfCheckFamilyBmp
(
    IN GT_U32 familyBmp
)
{
    /* check if current device included in skip bmp */
    return (extCtx.devArray[prvTgfDevNum] & familyBmp) ? GT_TRUE : GT_FALSE;
}

/**
* @internal prvUtfSkipNonBaselineTestsFlagGet function
* @endinternal
*
* @brief   Get skip test flag for non baseline tests execution
*/
GT_BOOL prvUtfSkipNonBaselineTestsFlagGet
(
    IN GT_VOID
)
{
    return (prvUtfIsBaselineTestsExecution == GT_FALSE)? GT_TRUE : GT_FALSE;
}

/**
* @internal prvUtfBaselineTestsExecutionFlagGet function
* @endinternal
*
* @brief   Get baseline tests execution flag
*/
GT_BOOL prvUtfBaselineTestsExecutionFlagGet
(
    IN GT_VOID
)
{
    return prvUtfIsBaselineTestsExecution;
}

/**
* @internal prvUtfBaselineTestsExecutionFlagSet function
* @endinternal
*
* @brief   Set baseline tests execution flag
*
* @param[in] enable                   - baseline tests execution flag
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvUtfBaselineTestsExecutionFlagSet
(
    IN GT_BOOL enable
)
{
    prvUtfIsBaselineTestsExecution = enable;

    return GT_OK;
}

/**
* @internal prvUtfReduceLogSizeFlagGet function
* @endinternal
*
* @brief   Get a flag of limitation of CPSS API logging
*/
GT_BOOL prvUtfReduceLogSizeFlagGet
(
    IN GT_VOID
)
{
    return prvUtfIsReduceLogSize;
}

/**
* @internal prvUtfReduceLogSizeFlagSet function
* @endinternal
*
* @brief   Set a flag of limitation of CPSS API logging
*
* @param[in] enable                   - set (GT_TRUE) or reset(GT_FALSE) the flag
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvUtfReduceLogSizeFlagSet
(
    IN GT_BOOL enable
)
{
    prvUtfIsReduceLogSize = enable;

    return GT_OK;
}

/**
* @internal firstTimeTrunkConvertInit function
* @endinternal
*
* @brief   initialize the DB for the trunk Id conversion
*/
static GT_VOID firstTimeTrunkConvertInit(GT_VOID)
{
    GT_U32  ii;

    if(firstTimeTrunkConvert == GT_FALSE)
    {
        /* initialization already done */
        return;
    }

    firstTimeTrunkConvert = GT_FALSE;

    /* Add element for each device in system    */
    for(ii = 0; ii < PRV_CPSS_MAX_PP_DEVICES_CNS; ii++)
    {
        if(PRV_CPSS_PP_MAC(ii) == NULL)
        {
            continue;
        }

        if(0 == CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(ii)->devFamily))
        {
            continue;
        }

#ifdef CALL_DXCH_CODE_MAC
        /* use the internal DxCh DB */
        if(PRV_CPSS_DXCH_PP_MAC(ii)->errata.
            info_PRV_CPSS_DXCH_XCAT_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_E.
                enabled == GT_TRUE)
        {
            prvUtfDxChTrunkWaNeeded = GT_TRUE;
            break;
        }
#else /*CALL_DXCH_CODE_MAC*/
        break;
#endif /*CALL_DXCH_CODE_MAC*/
    }
}

/**
* @internal prvUtfTrunkConvertFromTestToCpss function
* @endinternal
*
* @brief   convert trunkId from test value
*         to value that the CPSS expect
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the test
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the CPSS
*                                       none
*/
GT_VOID prvUtfTrunkConvertFromTestToCpss
(
   INOUT GT_TRUNK_ID *trunkIdPtr
)
{
    GT_TRUNK_ID newTrunkId;/* new trunkId s*/

    firstTimeTrunkConvertInit();

    if(prvUtfDxChTrunkWaNeeded == GT_FALSE)
    {
        /* this must be after calling firstTimeTrunkConvertInit(...) */

        /* WA not needed */
        return;
    }

    /* WA for trunk Erratum : "wrong trunk id source port
       information of packet to CPU" */

    /* need to do the WA */
    /* convert trunk-Id from test (1..30) to trunkId that fit the WA  (66..124, even) */

    if((*trunkIdPtr) > WA_LAST_TRUNK_CNS)  /* bigger then last (not first) */
    {
        /*no change*/
        return;
    }
    else if((*trunkIdPtr) == 0)
    {
        /*no change*/
        return;
    }

    newTrunkId = (GT_TRUNK_ID)(WA_FIRST_TRUNK_CNS + ((*trunkIdPtr) * 2));
    if(newTrunkId >= 128)
    {
        /* can't convert */
        return;
    }

    (*trunkIdPtr) = newTrunkId;

    return;
}

/**
* @internal prvUtfTrunkConvertFromCpssToTest function
* @endinternal
*
* @brief   convert trunkId from CPSS value to value that the TEST expect (get)
*
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the CPSS
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the test
*                                       none
*/
GT_VOID prvUtfTrunkConvertFromCpssToTest
(
   INOUT GT_TRUNK_ID *trunkIdPtr
)
{
    firstTimeTrunkConvertInit();

    if(prvUtfDxChTrunkWaNeeded == GT_FALSE)
    {
        /* this must be after calling firstTimeTrunkConvertInit(...) */

        /* WA not needed */
        return;
    }

    if((*trunkIdPtr) > WA_LAST_TRUNK_CNS)
    {
        /*no change*/
        return;
    }
    else if((*trunkIdPtr) < WA_FIRST_TRUNK_CNS)
    {
        /* can't convert */
        return;
    }
    else if((*trunkIdPtr) == 0)
    {
        /*no change*/
        return;
    }

    (*trunkIdPtr) = (GT_TRUNK_ID)(((*trunkIdPtr) - WA_FIRST_TRUNK_CNS) / 2);

    return;
}

/******************************************************************************/
/* extras internal functions implementation                                   */
/******************************************************************************/

/* Returns GT_OK if device is active */
static GT_STATUS devCheck(IN GT_U8 dev)
{
    return PRV_CPSS_IS_DEV_EXISTS_MAC(dev) ? GT_OK : GT_BAD_PARAM;
}

/* Returns GT_OK if physical port is active */
static GT_STATUS phyPortCheck(IN GT_U8 dev, IN GT_PHYSICAL_PORT_NUM port)
{
    if(PRV_CPSS_PP_MAC(dev)->functionsSupportedBmp &
           PRV_CPSS_PX_FUNCTIONS_SUPPORT_CNS)
    {
#ifdef PX_FAMILY
            PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(dev, port);
#endif
    }
    else
    if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
    {
#ifdef CHX_FAMILY
        if (prvCpssDxChPortReservedPortCheck(dev, port) == GT_TRUE)
        {
            /* Reserved physical ports can't be used */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        /* NOTE: this is iterator on 'physical' port (and not MAC/DMA port !!!)
           so in eArch the 'physical' is detached from the 'MAC/DMA' */
        if(PRV_CPSS_PP_MAC(dev)->functionsSupportedBmp &
           PRV_CPSS_DXCH_FUNCTIONS_SUPPORT_CNS)
        {
            PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(dev, port);
        }
#endif
    }
    else
    {
        PRV_CPSS_PHY_PORT_CHECK_MAC(dev, port);
    }


    return GT_OK;
}

static GT_STATUS phyPortOrCpuCheck(IN GT_U8 dev, IN GT_PHYSICAL_PORT_NUM port)
{
    if(PRV_CPSS_PP_MAC(dev)->functionsSupportedBmp &
       PRV_CPSS_PX_FUNCTIONS_SUPPORT_CNS)
    {
        return phyPortCheck(dev,port);
    }
    else
#ifdef CHX_FAMILY
    if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
    {
        if (prvCpssDxChPortReservedPortCheck(dev, port) == GT_TRUE)
        {
            /* Reserved physical port - not active port */
            return GT_OK;
        }
        /* NOTE: this is iterator on 'physical' port (and not MAC/DMA port !!!)
           so in eArch the 'physical' is detached from the 'MAC/DMA' */
        PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(dev, port);
    }
    else
    {
        PRV_CPSS_PHY_PORT_OR_CPU_PORT_CHECK_MAC(dev, port);
    }
#endif
    return GT_OK;
}


/* Returns GT_OK if virtual port is active */
static GT_STATUS virtPortCheck(IN GT_U8 dev, IN GT_PORT_NUM port)
{
    if(PRV_CPSS_PP_MAC(dev)->functionsSupportedBmp &
       PRV_CPSS_PX_FUNCTIONS_SUPPORT_CNS)
    {
        return phyPortOrCpuCheck(dev,port);
    }
    else
    if(PRV_CPSS_PP_MAC(dev)->functionsSupportedBmp &
       PRV_CPSS_DXCH_FUNCTIONS_SUPPORT_CNS)
    {
        if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
#ifdef CHX_FAMILY
            PRV_CPSS_DXCH_PORT_CHECK_MAC(dev, port)
#endif
        }
        else
        {
            /* DX device ,no virtual ports , only physical ports */
            PRV_CPSS_PHY_PORT_CHECK_MAC(dev, port);
        }
    }
    else
    {
        PRV_CPSS_VIRTUAL_PORT_CHECK_MAC(dev, port);
    }

    return GT_OK;
}

static GT_STATUS macPortCheck(IN GT_U8 dev, IN GT_PHYSICAL_PORT_NUM port)
{
    if(PRV_CPSS_PP_MAC(dev)->functionsSupportedBmp & PRV_CPSS_PX_FUNCTIONS_SUPPORT_CNS)
    {
#ifdef PX_FAMILY
        GT_U32 macPortNum;
        PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(dev, port, macPortNum);
#endif /*PX_FAMILY*/
    }
    else
    if(PRV_CPSS_PP_MAC(dev)->functionsSupportedBmp & PRV_CPSS_DXCH_FUNCTIONS_SUPPORT_CNS)
    {
        if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
#ifdef CHX_FAMILY
            GT_U32 macPortNum;
            PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(dev, port, macPortNum);
#endif /*CHX_FAMILY*/
        }
        else
        {
            /* DX device ,no virtual ports , only physical ports */
            PRV_CPSS_PHY_PORT_CHECK_MAC(dev, port);
        }
    }
    else
    {
        PRV_CPSS_PHY_PORT_CHECK_MAC(dev, port);
    }

    return GT_OK;
}

/* Returns GT_OK if TXQ port is mapped */
static GT_STATUS txqPortCheck(IN GT_U8 dev, IN GT_PHYSICAL_PORT_NUM port)
{
    if(PRV_CPSS_PP_MAC(dev)->functionsSupportedBmp &
       PRV_CPSS_PX_FUNCTIONS_SUPPORT_CNS)
    {
#ifdef PX_FAMILY
        GT_U32 txqPortNum;
        PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(dev, port, txqPortNum);
#endif
    }
    else
    if(PRV_CPSS_PP_MAC(dev)->functionsSupportedBmp &
       PRV_CPSS_DXCH_FUNCTIONS_SUPPORT_CNS)
    {
        if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
#ifdef CHX_FAMILY
            GT_U32 txqPortNum;
            PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(dev, port, txqPortNum);
#endif
        }
        else
        {
            /* DX device ,no virtual ports , only physical ports */
            PRV_CPSS_PHY_PORT_CHECK_MAC(dev, port);
        }
    }
    else
    {
        PRV_CPSS_PHY_PORT_CHECK_MAC(dev, port);
    }

    return GT_OK;
}

/* Returns GT_OK if rxDma port is mapped */
static GT_STATUS rxDmaPortCheck(IN GT_U8 dev, IN GT_PHYSICAL_PORT_NUM port)
{
    if(PRV_CPSS_PP_MAC(dev)->functionsSupportedBmp &
       PRV_CPSS_PX_FUNCTIONS_SUPPORT_CNS)
    {
#ifdef PX_FAMILY
        GT_U32 rxDmaPortNum;
        PRV_CPSS_PX_PORT_NUM_CHECK_AND_RXDMA_NUM_GET_MAC(dev, port, rxDmaPortNum);
#endif
    }
    else
    if(PRV_CPSS_PP_MAC(dev)->functionsSupportedBmp &
       PRV_CPSS_DXCH_FUNCTIONS_SUPPORT_CNS)
    {
        if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
#ifdef CHX_FAMILY
            GT_U32 rxDmaPortNum;
            PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_RXDMA_NUM_GET_MAC(dev, port, rxDmaPortNum);
#endif
        }
        else
        {
            /* DX device ,no virtual ports , only physical ports */
            PRV_CPSS_PHY_PORT_CHECK_MAC(dev, port);
        }
    }
    else
    {
        PRV_CPSS_PHY_PORT_CHECK_MAC(dev, port);
    }

    return GT_OK;
}

/* Returns GT_OK if txDma port is mapped */
static GT_STATUS txDmaPortCheck(IN GT_U8 dev, IN GT_PHYSICAL_PORT_NUM port)
{
    if(PRV_CPSS_PP_MAC(dev)->functionsSupportedBmp &
       PRV_CPSS_PX_FUNCTIONS_SUPPORT_CNS)
    {
#ifdef PX_FAMILY
        GT_U32 txDmaPortNum;
        PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXDMA_NUM_GET_MAC(dev, port, txDmaPortNum);
#endif
    }
    else
    if(PRV_CPSS_PP_MAC(dev)->functionsSupportedBmp &
       PRV_CPSS_DXCH_FUNCTIONS_SUPPORT_CNS)
    {
        if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
#ifdef CHX_FAMILY
            GT_U32 txDmaPortNum;
            PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_TXDMA_NUM_GET_MAC(dev, port, txDmaPortNum);
#endif
        }
        else
        {
            /* DX device ,no virtual ports , only physical ports */
            PRV_CPSS_PHY_PORT_CHECK_MAC(dev, port);
        }
    }
    else
    {
        PRV_CPSS_PHY_PORT_CHECK_MAC(dev, port);
    }

    return GT_OK;
}

/*******************************************************************************
* prvUtfCfgTableNumEntriesGet
*
*       the function return the number of entries of each individual table in
*       the HW.
*
* INPUTS:
*       dev     - physical device number
*       table   - type of the specific table
*
* OUTPUTS:
*       numEntriesPtr - (pointer to) number of entries
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on wrong device number or table type
*       GT_BAD_PTR      - on NULL pointer
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS prvUtfCfgTableNumEntriesGet
(
    IN GT_U8                 dev,
    IN PRV_TGF_CFG_TABLE_ENT table,
    OUT GT_U32               *numEntriesPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_STATUS rc;   /* return code */
#if (defined CHX_FAMILY)
    CPSS_DXCH_CFG_TABLES_ENT dxChTable;
#else
    CPSS_EXMXPM_CFG_TABLES_ENT exMxPmTable;
#endif

    /* device must be active */
    DEV_CHECK_MAC(dev);

    /* check for null pointer */
    CPSS_NULL_PTR_CHECK_MAC(numEntriesPtr);

#if (defined CHX_FAMILY)
    switch(table)
    {
        case PRV_TGF_CFG_TABLE_VLAN_E: dxChTable = CPSS_DXCH_CFG_TABLE_VLAN_E;
            break;
        case PRV_TGF_CFG_TABLE_VIDX_E: dxChTable = CPSS_DXCH_CFG_TABLE_VIDX_E;
            break;
        case PRV_TGF_CFG_TABLE_FDB_E: dxChTable = CPSS_DXCH_CFG_TABLE_FDB_E;
            break;
        case PRV_TGF_CFG_TABLE_ROUTER_NEXT_HOP_E: dxChTable = CPSS_DXCH_CFG_TABLE_ROUTER_NEXT_HOP_E;
            break;
        case PRV_TGF_CFG_TABLE_MAC_TO_ME_E: dxChTable = CPSS_DXCH_CFG_TABLE_MAC_TO_ME_E;
            break;
        case PRV_TGF_CFG_TABLE_ARP_E: dxChTable = CPSS_DXCH_CFG_TABLE_ARP_E;
            break;
        case PRV_TGF_CFG_TABLE_TUNNEL_START_E: dxChTable = CPSS_DXCH_CFG_TABLE_TUNNEL_START_E;
            break;
        case PRV_TGF_CFG_TABLE_STG_E: dxChTable = CPSS_DXCH_CFG_TABLE_STG_E;
            break;
        case PRV_TGF_CFG_TABLE_ACTION_E: dxChTable = CPSS_DXCH_CFG_TABLE_PCL_ACTION_E;
            break;
        case PRV_TGF_CFG_TABLE_TCAM_E: dxChTable = CPSS_DXCH_CFG_TABLE_PCL_TCAM_E;
            break;
        case PRV_TGF_CFG_TABLE_POLICER_METERS_E: dxChTable = CPSS_DXCH_CFG_TABLE_POLICER_METERS_E;
            break;
        case PRV_TGF_CFG_TABLE_POLICER_COUNTERS_E: dxChTable = CPSS_DXCH_CFG_TABLE_POLICER_BILLING_COUNTERS_E;

            if(PRV_CPSS_IS_DEV_EXISTS_MAC(prvTgfDevNum) &&
               PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum))
            {
                *numEntriesPtr =
                    PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->policer.memSize[0]+
                    PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->policer.memSize[1];
                return GT_OK;
            }

            break;
        case PRV_TGF_CFG_TABLE_MLL_DIT_E: dxChTable = CPSS_DXCH_CFG_TABLE_MLL_PAIR_E;
            break;
        case PRV_TGF_CFG_TABLE_ROUTER_LTT_E: dxChTable = CPSS_DXCH_CFG_TABLE_ROUTER_LTT_E;
            break;
        case PRV_TGF_CFG_TABLE_ROUTER_TCAM_E: dxChTable = CPSS_DXCH_CFG_TABLE_ROUTER_TCAM_E;
            break;
        case PRV_TGF_CFG_TABLE_TTI_TCAM_E: dxChTable = CPSS_DXCH_CFG_TABLE_TTI_TCAM_E;
            break;
        case PRV_TGF_CFG_TABLE_QOS_PROFILE_E: dxChTable = CPSS_DXCH_CFG_TABLE_QOS_PROFILE_E;
            break;
        case PRV_TGF_CFG_TABLE_CNC_E: dxChTable = CPSS_DXCH_CFG_TABLE_CNC_E;
            break;
        case PRV_TGF_CFG_TABLE_CNC_BLOCK_E: dxChTable = CPSS_DXCH_CFG_TABLE_CNC_BLOCK_E;
            break;
        case PRV_TGF_CFG_TABLE_TRUNK_E: dxChTable = CPSS_DXCH_CFG_TABLE_TRUNK_E;
            break;
        default: return GT_BAD_PARAM;
    }
    rc = cpssDxChCfgTableNumEntriesGet(dev, dxChTable, numEntriesPtr);
#else
    switch(table)
    {
        case PRV_TGF_CFG_TABLE_VLAN_E: exMxPmTable = CPSS_EXMXPM_CFG_TABLE_VLAN_E;
            break;
        case PRV_TGF_CFG_TABLE_VIDX_E: exMxPmTable = CPSS_EXMXPM_CFG_TABLE_VIDX_E;
            break;
        case PRV_TGF_CFG_TABLE_FDB_E: exMxPmTable = CPSS_EXMXPM_CFG_TABLE_FDB_E;
            break;
        case PRV_TGF_CFG_TABLE_ROUTER_NEXT_HOP_E: exMxPmTable = CPSS_EXMXPM_CFG_TABLE_ROUTER_NEXT_HOP_E;
            break;
        case PRV_TGF_CFG_TABLE_MAC_TO_ME_E: exMxPmTable = CPSS_EXMXPM_CFG_TABLE_MAC_TO_ME_E;
            break;
        case PRV_TGF_CFG_TABLE_ARP_E: exMxPmTable = CPSS_EXMXPM_CFG_TABLE_ARP_TUNNEL_START_E;
            break;
        case PRV_TGF_CFG_TABLE_TUNNEL_START_E: exMxPmTable = CPSS_EXMXPM_CFG_TABLE_ARP_TUNNEL_START_E;
            break;
        case PRV_TGF_CFG_TABLE_STG_E: exMxPmTable = CPSS_EXMXPM_CFG_TABLE_STG_E;
            break;
        case PRV_TGF_CFG_TABLE_ACTION_E: exMxPmTable = CPSS_EXMXPM_CFG_TABLE_INTERNAL_ACTION_E;
            break;
        case PRV_TGF_CFG_TABLE_TCAM_E: exMxPmTable = CPSS_EXMXPM_CFG_TABLE_INTERNAL_TCAM_E;
            break;
        case PRV_TGF_CFG_TABLE_POLICER_METERS_E: exMxPmTable = CPSS_EXMXPM_CFG_TABLE_POLICER_METERS_COUNTERS_E;
            break;
        case PRV_TGF_CFG_TABLE_POLICER_COUNTERS_E: exMxPmTable = CPSS_EXMXPM_CFG_TABLE_POLICER_METERS_COUNTERS_E;
            break;
        case PRV_TGF_CFG_TABLE_MLL_DIT_E: exMxPmTable = CPSS_EXMXPM_CFG_TABLE_DIT_E;
            break;
        case PRV_TGF_CFG_TABLE_FDB_LUT_E: exMxPmTable = CPSS_EXMXPM_CFG_TABLE_FDB_LUT_E;
            break;
        case PRV_TGF_CFG_TABLE_FDB_TCAM_E: exMxPmTable = CPSS_EXMXPM_CFG_TABLE_FDB_TCAM_E;
            break;
        case PRV_TGF_CFG_TABLE_MPLS_E: exMxPmTable = CPSS_EXMXPM_CFG_TABLE_MPLS_E;
            break;
        case PRV_TGF_CFG_TABLE_CNC_E: exMxPmTable = CPSS_EXMXPM_CFG_TABLE_CNC_E;
            break;
        case PRV_TGF_CFG_TABLE_CNC_BLOCK_E: exMxPmTable = CPSS_EXMXPM_CFG_TABLE_CNC_BLOCK_E;
            break;
        default: return GT_BAD_PARAM;
    }
    rc = cpssExMxPmCfgTableNumEntriesGet(dev, exMxPmTable, numEntriesPtr);
#endif

    return rc;

#else
    dev = dev;
    table = table;
    numEntriesPtr = numEntriesPtr;

    return GT_BAD_STATE;
#endif
}


/**
* @internal prvUtfCounterRegisterSet function
* @endinternal
*
* @brief   Write counter value to given register
*         in the specific port group in the device
* @param[in] devNum                   - The PP to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - The register's address to write to.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
*                                      data        - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note
*       Galtis:
*       None.
*
*/
static GT_STATUS prvUtfCounterRegisterSet
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    IN GT_U32   value
)
{
    if(fieldLength == 32)
    {
        return prvCpssHwPpPortGroupWriteRegister(devNum, portGroupId,
                                                    regAddr, value);
    }
    else
    {
        return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                               fieldOffset, fieldLength, value);
    }
}

/**
* @internal prvTgfConvertRegAddr function
* @endinternal
*
* @brief   This Function converts register address of test to proper one
*         in specific device.
* @param[in] devNum                   - device number
* @param[in,out] regAddrPtr               - (pointer to) input register address
* @param[in,out] regAddrPtr               - (pointer to) output register address
*                                      GT_OK    - Get next port was OK
*                                      GT_FAIL  - There is no active ports
*                                      COMMENTS:
*                                      None.
*/
static GT_STATUS prvTgfConvertRegAddr
(
    IN    GT_U8                   devNum,
    INOUT GT_U32                 *regAddrPtr
)
{
    devNum = devNum;
    regAddrPtr = regAddrPtr;
#ifdef CALL_DXCH_CODE_MAC
        {
            PRV_CPSS_DXCH_UNIT_ENT  dxchUnitId;

            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
            {
                /* currently all places that calls this function uses 'Lion2' based DB */
                dxchUnitId = prvCpssDxChHwRegAddrToUnitIdConvert(LION2_DEVICE_INDICATION_CNS,*regAddrPtr);
                switch(dxchUnitId)
                {
                    case PRV_CPSS_DXCH_UNIT_L2I_E:
                    case PRV_CPSS_DXCH_UNIT_FDB_E:
                    case PRV_CPSS_DXCH_UNIT_EQ_E:
                    case PRV_CPSS_DXCH_UNIT_MLL_E:
                        /* the counter register addresses changed , so caller need
                           use proper regDb */
                        CPSS_TBD_BOOKMARK
                        return GT_FAIL;
                    default:
                        /* so the register address need to be converted to the offset in the 'moved' base address */
                        convertRegAddrToNewUnitsBaseAddr(devNum, regAddrPtr);
                        break;
                }
            }
        }
#endif /*CALL_DXCH_CODE_MAC*/
        return GT_OK;
}

/**
* @internal prvUtfPrivatePerPortGroupPerRegisterCounterGet function
* @endinternal
*
* @brief   This Function checks counters in the simulation.
*
* @param[in] inRegAddr                - The register's address to write to.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] getCounter               - Pointer to get counter function.
* @param[in] setCounter               - Pointer to set counter function.
* @param[in] getPerGroupCounter       - Pointer to get per port group counter function.
* @param[in] setPerGroupCounter       - Pointer to set per port group counter function.
* @param[in] getCounterPerReg         - Pointer to get counter function.
* @param[in] setCounterPerReg         - Pointer to set counter function.
* @param[in] getPerGroupCounterPerReg - Pointer to get per port group counter function.
* @param[in] setPerGroupCounterPerReg - Pointer to set per port group counter function.
*/
static GT_VOID prvUtfPrivatePerPortGroupPerRegisterCounterGet
(
    IN GT_U32                                           inRegAddr,
    IN GT_U32                                           fieldOffset,
    IN GT_U32                                           fieldLength,
    IN PRV_UTF_CPSS_GET_COUNTER_FUN                     getCounter,
    IN PRV_UTF_CPSS_SET_COUNTER_FUN                     setCounter,
    IN PRV_UTF_CPSS_GET_PER_GROUP_COUNTER_FUN           getPerGroupCounter,
    IN PRV_UTF_CPSS_SET_PER_GROUP_COUNTER_FUN           setPerGroupCounter,
    IN PRV_UTF_CPSS_GET_COUNTER_PER_REG_FUN             getCounterPerReg,
    IN PRV_UTF_CPSS_SET_COUNTER_PER_REG_FUN             setCounterPerReg,
    IN PRV_UTF_CPSS_GET_PER_GROUP_COUNTER_PER_REG_FUN   getPerGroupCounterPerReg,
    IN PRV_UTF_CPSS_SET_PER_GROUP_COUNTER_PER_REG_FUN   setPerGroupCounterPerReg
)
{
    GT_STATUS               st = GT_OK;         /* return status */
    GT_U8                   devNum;             /* device number */
    GT_U32                  writeValueArr[3];   /* write counter value */
    GT_U32                  counterValue = 0;   /* read counter value */
    GT_U32                  i;                  /* iterator */
    GT_PORT_GROUPS_BMP      portGroupsBmp = 0;  /* port group bitmap */
    GT_U32                  portGroupId;        /* port group id */
    GT_U32                  countMaxValue;      /* counter maximal value */
    GT_U32                  regAddr = inRegAddr; /* register address */

    if(fieldLength == 32)
    {
        countMaxValue = 0xFFFFFFFF;
    }
    else
    {
        countMaxValue = (1 << fieldLength) - 1;
    }

    writeValueArr[0] = countMaxValue;
    writeValueArr[1] = countMaxValue / 2;
    writeValueArr[2] = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    /*   writeValue = countMaxValue , countMaxValue/2, 0
         Write countMaxValue to counter by prvCpssHwPpPortGroupSetRegField /
         prvCpssHwPpWriteRegister.
         Read counter by old (not per port group API) and
         compare value with countMaxValue */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
#if (defined CHX_FAMILY)
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            /* test not suitable for eArch devices , as offsets in the units not match */
            continue;
        }
#endif
        regAddr = inRegAddr;
        if(GT_OK != prvTgfConvertRegAddr(devNum,&regAddr))
        {
            return;
        }

        for(i = 0; i < 3; i++)
        {
            if((setCounter == NULL) && (setCounterPerReg == NULL))
            {
                st = prvUtfCounterRegisterSet(devNum, 0, regAddr, fieldOffset,
                                              fieldLength, writeValueArr[i]);
            }
            else
            {
                if(setCounter != NULL)
                {
                    st = setCounter(devNum, writeValueArr[i]);
                }
                else
                {
                    st = setCounterPerReg(devNum, regAddr, writeValueArr[i]);
                }
            }
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            if(getCounter != NULL)
            {
                st = getCounter(devNum, &counterValue);
            }
            else if(getCounterPerReg != NULL)
            {
                st = getCounterPerReg(devNum, regAddr, &counterValue);
            }
            else
            {
                st = GT_FAIL;
            }
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(writeValueArr[i], counterValue,
               "get another enable than was set: %d", devNum);
        }

    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    /* port_group = first_port_group... last_port_group
       writeValue = countMaxValue , countMaxValue/2, 0
       Write writeValue to counter by prvCpssHwPpPortGroupSetRegField for port_group.
       Read counter by new per port group API for port_group and
       compare value with writeValue.
       Read counter by new per port group API for all other existing port groups
       and see that them was not changed.
       Read counter by new per port group API
       for CPSS_PORT_GROUP_UNAWARE_MODE_CNS port groups and
       see that it sum of all port groups. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
#if (defined CHX_FAMILY)
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            /* test not suitable for eArch devices , as offsets in the units not match */
            continue;
        }
#endif
        regAddr = inRegAddr;
        if(GT_OK != prvTgfConvertRegAddr(devNum,&regAddr))
        {
            return;
        }

        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)
        {
            portGroupsBmp = 1 << portGroupId;

            for(i = 0; i < 3; i++)
            {
                if((setPerGroupCounter == NULL) &&
                   (setPerGroupCounterPerReg == NULL))
                {
                    st = prvUtfCounterRegisterSet(devNum, portGroupId, regAddr,
                                                 fieldOffset,
                                                 fieldLength, writeValueArr[i]);
                }
                else
                {
                    if(setPerGroupCounter != NULL)
                    {
                        st = setPerGroupCounter(devNum, portGroupsBmp,
                                                writeValueArr[i]);
                    }
                    else
                    {
                        st = setPerGroupCounterPerReg(devNum, portGroupsBmp,
                                                     regAddr,
                                                     writeValueArr[i]);
                    }
                }
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp);

                if(getPerGroupCounter != NULL)
                {
                    st = getPerGroupCounter(devNum, portGroupsBmp,
                                                   &counterValue);
                }
                else if(getPerGroupCounterPerReg != NULL)
                {
                    st = getPerGroupCounterPerReg(devNum, portGroupsBmp,
                                                  regAddr, &counterValue);
                }
                else
                {
                    st = GT_FAIL;
                }

                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp);
                UTF_VERIFY_EQUAL2_STRING_MAC(writeValueArr[i], counterValue,
                "get another enable than was set: %d%d", devNum, portGroupsBmp);

                /* Read counter by new per port group API for all
                  other existing port groups and see that them was not changed. */
                #ifdef CHX_FAMILY
                portGroupsBmp = (~portGroupsBmp) & (PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp);
                #else
                portGroupsBmp = (~portGroupsBmp) & 0xF;
                #endif
                if(getPerGroupCounter != NULL)
                {
                    st = getPerGroupCounter(devNum, portGroupsBmp, &counterValue);
                }
                else if(getPerGroupCounterPerReg != NULL)
                {
                    st = getPerGroupCounterPerReg(devNum, portGroupsBmp,
                                                   regAddr, &counterValue);
                }
                else
                {
                    st = GT_FAIL;
                }

                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp);
                UTF_VERIFY_EQUAL2_STRING_MAC(0, counterValue,
                "get another enable than was set: %d%d", devNum, portGroupsBmp);
                #ifdef CHX_FAMILY
                portGroupsBmp = (~portGroupsBmp) & (PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp);
                #else
                portGroupsBmp = (~portGroupsBmp) & 0xF;
                #endif
                /* Read counter by new per port group API
                  for CPSS_PORT_GROUP_UNAWARE_MODE_CNS port groups and
                  see that it sum of all port groups. */
                if((setPerGroupCounter == NULL) &&
                   (setPerGroupCounterPerReg == NULL))
                {
                    st = prvUtfCounterRegisterSet(devNum, portGroupId, regAddr,
                                                 fieldOffset,
                                                 fieldLength, writeValueArr[i]);
                }
                else
                {
                    if(setPerGroupCounter != NULL)
                    {
                        st = setPerGroupCounter(devNum, portGroupsBmp,
                                                writeValueArr[i]);
                    }
                    else
                    {
                        st = setPerGroupCounterPerReg(devNum, portGroupsBmp,
                                                     regAddr,
                                                     writeValueArr[i]);
                    }
                }
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp);

                if(getPerGroupCounter != NULL)
                {
                    st = getPerGroupCounter(devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            &counterValue);
                }
                else if(getPerGroupCounterPerReg != NULL)
                {
                    st = getPerGroupCounterPerReg(devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            regAddr, &counterValue);
                }
                else
                {
                    st = GT_FAIL;
                }

                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
                UTF_VERIFY_EQUAL1_STRING_MAC(writeValueArr[i], counterValue,
                "get another enable than was set: %d%d", devNum);

            }
        }PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    /*   writeValue = countMaxValue , countMaxValue/2, 0
         Write countMaxValue to counter by prvCpssHwPpPortGroupSetRegField /
         prvCpssHwPpWriteRegister.
         Read counter by old (not per port group API) and
         compare value with countMaxValue */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
#if (defined CHX_FAMILY)
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            /* test not suitable for eArch devices , as offsets in the units not match */
            continue;
        }
#endif
        regAddr = inRegAddr;
        if(GT_OK != prvTgfConvertRegAddr(devNum,&regAddr))
        {
            return;
        }

        for(i = 0; i < 3; i++)
        {
            st = prvUtfCounterRegisterSet(devNum, 0, regAddr, fieldOffset,
                                              fieldLength, writeValueArr[i]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            if(getCounter != NULL)
            {
                st = getCounter(devNum, &counterValue);
            }
            else if(getCounterPerReg != NULL)
            {
                st = getCounterPerReg(devNum, regAddr, &counterValue);
            }
            else
            {
                st = GT_FAIL;
            }
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(writeValueArr[i], counterValue,
               "get another enable than was set: %d", devNum);
        }

    }
}



/**
* @internal prvUtfPerPortGroupCounterGet function
* @endinternal
*
* @brief   This Function checks counters in the simulation.
*/
GT_VOID prvUtfPerPortGroupCounterGet
(
    IN GT_U32                                   regAddr,
    IN GT_U32                                   fieldOffset,
    IN GT_U32                                   fieldLength,
    IN PRV_UTF_CPSS_GET_COUNTER_FUN             getCounter,
    IN PRV_UTF_CPSS_SET_COUNTER_FUN             setCounter,
    IN PRV_UTF_CPSS_GET_PER_GROUP_COUNTER_FUN   getPerGroupCounter,
    IN PRV_UTF_CPSS_SET_PER_GROUP_COUNTER_FUN   setPerGroupCounter
)
{
    prvUtfPrivatePerPortGroupPerRegisterCounterGet(regAddr, fieldOffset,
                                                 fieldLength, getCounter,
                                                 setCounter,getPerGroupCounter,
                                                 setPerGroupCounter,
                                                 NULL, NULL, NULL, NULL);
}

/**
* @internal prvUtfPerPortGroupCounterPerRegGet function
* @endinternal
*
* @brief   This Function checks counters in the simulation for
*         specific register adddress.
* @param[in] regAddr                  - The register's address to write to.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] getCounterPerReg         - Pointer to get counter function.
* @param[in] setCounterPerReg         - Pointer to set counter function.
* @param[in] getPerGroupCounterPerReg - Pointer to get per port group counter function.
* @param[in] setPerGroupCounterPerReg - Pointer to set per port group counter function.
*/
GT_VOID prvUtfPerPortGroupCounterPerRegGet
(
    IN GT_U32                                           regAddr,
    IN GT_U32                                           fieldOffset,
    IN GT_U32                                           fieldLength,
    IN PRV_UTF_CPSS_GET_COUNTER_PER_REG_FUN             getCounterPerReg,
    IN PRV_UTF_CPSS_SET_COUNTER_PER_REG_FUN             setCounterPerReg,
    IN PRV_UTF_CPSS_GET_PER_GROUP_COUNTER_PER_REG_FUN   getPerGroupCounterPerReg,
    IN PRV_UTF_CPSS_SET_PER_GROUP_COUNTER_PER_REG_FUN   setPerGroupCounterPerReg
)
{
    prvUtfPrivatePerPortGroupPerRegisterCounterGet(regAddr, fieldOffset,
                                                   fieldLength,
                                                   NULL, NULL, NULL, NULL,
                                                   getCounterPerReg,
                                                   setCounterPerReg,
                                                   getPerGroupCounterPerReg,
                                                   setPerGroupCounterPerReg);
}

/**
* @internal prvUtfIsPbrModeUsed function
* @endinternal
*
* @brief   This routine returns GT_TRUE if PBR mode used. GT_FALSE otherwise.
*/
GT_BOOL prvUtfIsPbrModeUsed()
{
    GT_U32 value;

    return ((appDemoDbEntryGet("usePolicyBasedRouting", &value) == GT_OK)
            && (value == GT_TRUE)) ?  GT_TRUE : GT_FALSE;
}

/**
* @internal prvUtfIsDoublePhysicalPortsModeUsed function
* @endinternal
*
* @brief   This routine returns GT_TRUE if 'doublePhysicalPorts' mode used. GT_FALSE otherwise.
*/
GT_BOOL prvUtfIsDoublePhysicalPortsModeUsed(void)
{
    GT_U32  value;
    if(PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum) &&
        (appDemoDbEntryGet("doublePhysicalPorts", &value) == GT_OK) &&
        (value != 0))
    {
        return GT_TRUE;
    }

    return GT_FALSE;
}

/**
* @internal prvUtfIsNumberPhysicalPortsModeUsed function
* @endinternal
*
* @brief   This routine Return value of physicalPortsNumber  if 
*          'numberPhysicalPorts' mode used. GT_FALSE otherwise.
*/
GT_U32 prvUtfIsNumberPhysicalPortsModeUsed(void)
{
    GT_U32  value;
    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) &&
        (appDemoDbEntryGet("numberPhysicalPorts", &value) == GT_OK) &&
        (value != 0))
    {
        return value;
    }

    return 0;
}

/**
* @internal prvUtfIsTrafficManagerUsed function
* @endinternal
*
* @brief   This routine returns GT_TRUE if Traffic Manager (TM) used. GT_FALSE otherwise.
*/
GT_BOOL prvUtfIsTrafficManagerUsed
(
    GT_VOID
)
{
    GT_U32  boardIdx;
    GT_U32  boardRevId;
    GT_U32  reloadEeprom;
    GT_BOOL tmUsed = GT_FALSE;

    /* get init parameters */
    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* TM is used only for BC2 boards - board index 29*/
    if (boardIdx == 29)
    {
        if ((boardRevId == 2) || (boardRevId == 4) || (boardRevId == 12))
        {
            tmUsed = GT_TRUE;
        }
    }

    return tmUsed;
}

/* functions to support port as GT_U8 .
   NOTE: code must not call them directly !
         code must call ONLY the functions WITHOUT the '_old' in the name !!!
*/
GT_STATUS prvUtfNextPhyPortReset_old
(
    INOUT GT_U8     *portPtr,
    IN GT_U8        dev
)
{
    GT_STATUS   rc;
    GT_PHYSICAL_PORT_NUM    phyPortNum;

    rc = prvUtfNextPhyPortReset(&phyPortNum,dev);

    (*portPtr) = (GT_U8)(phyPortNum & 0xFF);

    return rc;
}

/* functions to support port as GT_U8 .
   NOTE: code must not call them directly !
         code must call ONLY the functions WITHOUT the '_old' in the name !!!
*/
GT_STATUS prvUtfNextPhyPortGet_old
(
    INOUT GT_U8     *portPtr,
    IN GT_BOOL      activeIs
)
{
    GT_STATUS   rc;
    GT_PHYSICAL_PORT_NUM    phyPortNum;

    if(portPtr == NULL)
    {
        return GT_FAIL;
    }

    if((*portPtr) == 0xFF)
    {
        phyPortNum = ITERATOR_INIT_VALUE_CNS;
    }
    else
    {
       phyPortNum = *portPtr;
    }

    rc = prvUtfNextPhyPortGet(&phyPortNum,activeIs);

    (*portPtr) = (GT_U8)(phyPortNum & 0xFF);

    return rc;
}

/* functions to support port as GT_U8 .
   NOTE: code must not call them directly !
         code must call ONLY the functions WITHOUT the '_old' in the name !!!
*/
GT_STATUS prvUtfPortTypeGet_old
(
    IN GT_U8                    dev,
    IN GT_U8                    port,
    OUT PRV_CPSS_PORT_TYPE_ENT  *portTypePtr
)
{
    return prvUtfPortTypeGet(dev,port,portTypePtr);
}


/* functions to support port as GT_U8 .
   NOTE: code must not call them directly !
         code must call ONLY the functions WITHOUT the '_old' in the name !!!
*/
GT_STATUS prvUtfPortInterfaceModeGet_old
(
    IN GT_U8                    dev,
    IN GT_U8                    port,
    OUT CPSS_PORT_INTERFACE_MODE_ENT    *portIfModePtr
)
{
    return prvUtfPortInterfaceModeGet(dev,port,portIfModePtr);
}

/* functions to support port as GT_U8 .
   NOTE: code must not call them directly !
         code must call ONLY the functions WITHOUT the '_old' in the name !!!
*/
GT_STATUS prvUtfPortSpeedGet_old
(
    IN GT_U8                    dev,
    IN GT_U8                    port,
    OUT CPSS_PORT_SPEED_ENT     *portSpeedPtr
)
{
    return prvUtfPortSpeedGet(dev,port,portSpeedPtr);
}

/* functions to support port as GT_U8 .
   NOTE: code must not call them directly !
         code must call ONLY the functions WITHOUT the '_old' in the name !!!
*/
GT_STATUS prvUtfPortTypeOptionsGet_old
(
    IN GT_U8                            dev,
    IN GT_U8                            port,
    OUT PRV_CPSS_PORT_TYPE_OPTIONS_ENT  *portTypeOptionsPtr
)
{
    return prvUtfPortTypeOptionsGet(dev,port,portTypeOptionsPtr);
}

/* functions to support port as GT_U8 .
   NOTE: code must not call them directly !
         code must call ONLY the functions WITHOUT the '_old' in the name !!!
*/
GT_STATUS prvUtfPortIsFlexLinkGet_old
(
    IN GT_U8                            dev,
    IN GT_U8                            port,
    OUT GT_BOOL                         *isFlexLinkPtr
)
{
    return prvUtfPortIsFlexLinkGet(dev,port,isFlexLinkPtr);
}


/* functions to support port as GT_U8 .
   NOTE: code must not call them directly !
         code must call ONLY the functions WITHOUT the '_old' in the name !!!
*/
GT_STATUS prvUtfNextVirtPortReset_old
(
    INOUT GT_U8             *portPtr,
    IN    GT_HW_DEV_NUM    hwDev
)
{
    GT_STATUS   rc;
    GT_PORT_NUM    virtPortNum;

    rc = prvUtfNextVirtPortReset(&virtPortNum,hwDev);

    (*portPtr) = (GT_U8)(virtPortNum & 0xFF);

    return rc;
}
/* functions to support port as GT_U8 .
   NOTE: code must not call them directly !
         code must call ONLY the functions WITHOUT the '_old' in the name !!!
*/
GT_STATUS prvUtfNextVirtPortGet_old
(
    INOUT GT_U8             *portPtr,
    IN GT_BOOL              activeIs
)
{
    GT_STATUS   rc;
    GT_PORT_NUM    virtPortNum;

    if(portPtr == NULL)
    {
        return GT_FAIL;
    }

    if((*portPtr) == 0xFF)
    {
        virtPortNum = ITERATOR_INIT_VALUE_CNS;
    }
    else
    {
        virtPortNum = *portPtr;
    }

    rc = prvUtfNextVirtPortGet(&virtPortNum,activeIs);

    (*portPtr) = (GT_U8)(virtPortNum & 0xFF);

    return rc;
}


/* functions to support port as GT_U8 .
   NOTE: code must not call them directly !
         code must call ONLY the functions WITHOUT the '_old' in the name !!!
*/
GT_STATUS prvUtfNextMacPortReset_old
(
    INOUT GT_U8             *portPtr,
    IN GT_U8                dev
)
{
    GT_STATUS   rc;
    GT_U32      macPortNum;

    rc = prvUtfNextMacPortReset(&macPortNum,dev);

    (*portPtr) = (GT_U8)(macPortNum & 0xFF);

    return rc;
}
/* functions to support port as GT_U8 .
   NOTE: code must not call them directly !
         code must call ONLY the functions WITHOUT the '_old' in the name !!!
*/
GT_STATUS prvUtfNextMacPortGet_old
(
    INOUT GT_U8             *portPtr,
    IN GT_BOOL              activeIs
)
{
    GT_STATUS   rc;
    GT_PORT_NUM    macPortNum;

    if(portPtr == NULL)
    {
        return GT_FAIL;
    }

    if((*portPtr) == 0xFF)
    {
        macPortNum = ITERATOR_INIT_VALUE_CNS;
    }
    else
    {
        macPortNum = *portPtr;
    }

    rc = prvUtfNextVirtPortGet(&macPortNum,activeIs);

    (*portPtr) = (GT_U8)(macPortNum & 0xFF);

    return rc;
}

/* functions to support port as GT_U8 .
   NOTE: code must not call them directly !
         code must call ONLY the functions WITHOUT the '_old' in the name !!!
*/
GT_STATUS prvUtfNextTxqPortReset_old
(
    INOUT GT_U8             *portPtr,
    IN    GT_U8             dev
)
{
    GT_STATUS   rc;
    GT_U32      txqPortNum;

    rc = prvUtfNextMacPortReset(&txqPortNum,dev);

    (*portPtr) = (GT_U8)(txqPortNum & 0xFF);

    return rc;
}
/* functions to support port as GT_U8 .
   NOTE: code must not call them directly !
         code must call ONLY the functions WITHOUT the '_old' in the name !!!
*/
GT_STATUS prvUtfNextTxqPortGet_old
(
    INOUT GT_U8             *portPtr,
    IN GT_BOOL              activeIs
)
{
    GT_STATUS   rc;
    GT_PORT_NUM    txqPortNum;

    if(portPtr == NULL)
    {
        return GT_FAIL;
    }

    if((*portPtr) == 0xFF)
    {
        txqPortNum = ITERATOR_INIT_VALUE_CNS;
    }
    else
    {
        txqPortNum = *portPtr;
    }

    rc = prvUtfNextVirtPortGet(&txqPortNum,activeIs);

    (*portPtr) = (GT_U8)(txqPortNum & 0xFF);

    return rc;
}

/* functions to support port as GT_U8 .
   NOTE: code must not call them directly !
         code must call ONLY the functions WITHOUT the '_old' in the name !!!
*/
GT_STATUS prvUtfNextRxDmaPortReset_old
(
    INOUT GT_U8             *portPtr,
    IN GT_U8                dev
)
{
    GT_STATUS       rc;
    GT_U32          rxDmaPortNum;

    rc = prvUtfNextRxDmaPortReset(&rxDmaPortNum, dev);

    (*portPtr) = (GT_U8)(rxDmaPortNum & 0xFF);

    return rc;
}

/* functions to support port as GT_U8 .
   NOTE: code must not call them directly !
         code must call ONLY the functions WITHOUT the '_old' in the name !!!
*/
GT_STATUS prvUtfNextRxDmaPortGet_old
(
    INOUT GT_U8             *portPtr,
    IN  GT_BOOL              activeIs
)
{
    GT_STATUS       rc;
    GT_PORT_NUM     rxDmaPortNum;

    if(portPtr == NULL)
    {
        return GT_FAIL;
    }

    if((*portPtr) == 0xFF)
    {
        rxDmaPortNum = ITERATOR_INIT_VALUE_CNS;
    }
    else
    {
        rxDmaPortNum = *portPtr;
    }

    rc = prvUtfNextVirtPortGet(&rxDmaPortNum, activeIs);

    (*portPtr) = (GT_U8)(rxDmaPortNum & 0xFF);

    return rc;
}

/* functions to support port as GT_U8 .
   NOTE: code must not call them directly !
         code must call ONLY the functions WITHOUT the '_old' in the name !!!
*/
GT_STATUS prvUtfNextTxDmaPortReset_old
(
    INOUT GT_U8             *portPtr,
    IN GT_U8                dev
)
{
    GT_STATUS       rc;
    GT_U32          txDmaPortNum;

    rc = prvUtfNextTxDmaPortReset(&txDmaPortNum, dev);

    (*portPtr) = (GT_U8)(txDmaPortNum & 0xFF);

    return rc;
}

/* functions to support port as GT_U8 .
   NOTE: code must not call them directly !
         code must call ONLY the functions WITHOUT the '_old' in the name !!!
*/
GT_STATUS prvUtfNextTxDmaPortGet_old
(
    INOUT GT_U8             *portPtr,
    IN  GT_BOOL              activeIs
)
{
    GT_STATUS       rc;
    GT_PORT_NUM     txDmaPortNum;

    if(portPtr == NULL)
    {
        return GT_FAIL;
    }

    if((*portPtr) == 0xFF)
    {
        txDmaPortNum = ITERATOR_INIT_VALUE_CNS;
    }
    else
    {
        txDmaPortNum = *portPtr;
    }

    rc = prvUtfNextVirtPortGet(&txDmaPortNum, activeIs);

    (*portPtr) = (GT_U8)(txDmaPortNum & 0xFF);

    return rc;
}

/**
* @internal prvUtfExternalTcamIsUsedGet function
* @endinternal
*
* @brief   Get external TCAM usage status
*
* @param[in] dev                      - SW device number
*
* @param[out] isUsed                   - (pointer to) external TCAM is used
*                                      GT_TRUE - external TCAM
*                                      GT_FALSE - internal TCAM
*
* @retval GT_OK                    -  Get revision of device OK
* @retval GT_BAD_PARAM             -  Invalid device id
* @retval GT_BAD_PTR               -  Null pointer
*/
GT_STATUS prvUtfExternalTcamIsUsedGet
(
    IN  GT_U8       dev,
    OUT GT_BOOL     *isUsed
)
{

#if (defined EXMXPM_FAMILY)

    DEV_CHECK_MAC(dev);

    /* check for null pointer */
    CPSS_NULL_PTR_CHECK_MAC(isUsed);

    /* Get device family */
    *isUsed = PRV_CPSS_EXMXPM_PP_MAC(dev)->moduleCfg.externalTcamCfg.externalTcamUsed;

    return GT_OK;

#else
    TGF_PARAM_NOT_USED(dev);

    *isUsed = GT_FALSE;
    return GT_OK;
#endif

}


/**
* @internal prvUtfNextGenericPortItaratorTypeSet function
* @endinternal
*
* @brief   set the generic port iterator type
*
* @param[in,out] type                     -   iterator
*
* @retval GT_OK                    -  iterator type OK
* @retval GT_FAIL                  -  iterator type NOT OK
*/
GT_STATUS prvUtfNextGenericPortItaratorTypeSet
(
    INOUT UTF_GENERIC_PORT_ITERATOR_TYPE_E     type
)
{
    switch(type)
    {
        case UTF_GENERIC_PORT_ITERATOR_TYPE_PHYSICAL_E:
        case UTF_GENERIC_PORT_ITERATOR_TYPE_VIRTUAL_E:
        case UTF_GENERIC_PORT_ITERATOR_TYPE_DEFAULT_EPORT_E:
        case UTF_GENERIC_PORT_ITERATOR_TYPE_MAC_E:
        case UTF_GENERIC_PORT_ITERATOR_TYPE_TXQ_E:
            break;
        default:
            break;
    }

    currentGenericPortIteratorType = type;

    return GT_OK;

}


/**
* @internal prvUtfNextGenericPortReset function
* @endinternal
*
* @brief   Reset generic ports iterator must be called before go over all ports
*         NOTE: caller should set the 'iterator type' before calling this see prvUtfNextGenericPortItaratorTypeSet
* @param[in,out] portPtr                  -   iterator thru ports
* @param[in] dev                      -   device id of ports
*
* @retval GT_OK                    -  iterator initialization OK
* @retval GT_FAIL                  -  general failure error
*/
GT_STATUS prvUtfNextGenericPortReset
(
    INOUT UTF_PORT_NUM     *portPtr,
    IN GT_U8        dev
)
{
    switch(currentGenericPortIteratorType)
    {
        case UTF_GENERIC_PORT_ITERATOR_TYPE_PHYSICAL_E:
            return prvUtfNextPhyPortReset(portPtr,dev);
        case UTF_GENERIC_PORT_ITERATOR_TYPE_VIRTUAL_E:
            return prvUtfNextVirtPortReset(portPtr,dev);
        case UTF_GENERIC_PORT_ITERATOR_TYPE_DEFAULT_EPORT_E:
            return prvUtfNextDefaultEPortReset(portPtr,dev);

        case UTF_GENERIC_PORT_ITERATOR_TYPE_MAC_E:
            return prvUtfNextMacPortReset(portPtr,dev);
        case UTF_GENERIC_PORT_ITERATOR_TYPE_TXQ_E:
            return prvUtfNextTxqPortReset(portPtr,dev);
        case UTF_GENERIC_PORT_ITERATOR_TYPE_RXDMA_E:
            return prvUtfNextRxDmaPortReset(portPtr,dev);
        case UTF_GENERIC_PORT_ITERATOR_TYPE_TXDMA_E:
            return prvUtfNextTxDmaPortReset(portPtr,dev);

        default:
            return GT_FAIL;
    }
}

/**
* @internal prvUtfNextGenericPortGet function
* @endinternal
*
* @brief   This routine returns next active/non active generic port id.
*         Call the function until non GT_OK error. Note that
*         iterator must be initialized before.
*         NOTE: caller should set the 'iterator type' before calling this see prvUtfNextGenericPortItaratorTypeSet
* @param[in,out] portPtr                  -   port id
* @param[in] activeIs                 -   set to GT_TRUE for getting active ports
*/
GT_STATUS prvUtfNextGenericPortGet
(
    INOUT UTF_PORT_NUM       *portPtr,
    IN GT_BOOL              activeIs
)
{
    switch(currentGenericPortIteratorType)
    {
        case UTF_GENERIC_PORT_ITERATOR_TYPE_PHYSICAL_E:
            return prvUtfNextPhyPortGet(portPtr,activeIs);
        case UTF_GENERIC_PORT_ITERATOR_TYPE_VIRTUAL_E:
            return prvUtfNextVirtPortGet(portPtr,activeIs);
        case UTF_GENERIC_PORT_ITERATOR_TYPE_DEFAULT_EPORT_E:
            return prvUtfNextDefaultEPortGet(portPtr,activeIs);
        case UTF_GENERIC_PORT_ITERATOR_TYPE_MAC_E:
            return prvUtfNextMacPortGet(portPtr,activeIs);
        case UTF_GENERIC_PORT_ITERATOR_TYPE_TXQ_E:
            return prvUtfNextTxqPortGet(portPtr,activeIs);
        case UTF_GENERIC_PORT_ITERATOR_TYPE_RXDMA_E:
            return prvUtfNextRxDmaPortGet(portPtr,activeIs);
        case UTF_GENERIC_PORT_ITERATOR_TYPE_TXDMA_E:
            return prvUtfNextTxDmaPortGet(portPtr,activeIs);
        default:
            return GT_FAIL;
    }
}

/**
* @internal prvUtfIterationReduce function
* @endinternal
*
* @brief   As the GM hold low performance , we need to reduce large iterations on tables.
*         this function allow to limit the number of iterations on a table by setting
*         larger step to iterate the entries in the table.
* @param[in] dev                      - SW device number
* @param[in] tableSize                - number of entries in the table
* @param[in] maxIterations            - maximum iterations required
* @param[in] origStep                 - the step that would be used when special reduce is not needed.
*                                       the actual step to use (after reduce).
*/
GT_U32 prvUtfIterationReduce
(
    IN  GT_U8       dev,
    IN  GT_U32      tableSize,
    IN  GT_U32      maxIterations,
    IN  GT_U32      origStep
)
{
    GT_U32  step = (tableSize / maxIterations)*origStep;/* total of x entries to check regardless to table size */
    /* for multi port groups device - extra reducing */
    if (PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(dev))
    {
        /* because x port groups , do calibration to maintain same time for the test */
        step *= PRV_CPSS_PP_MAC(dev)->portGroupsInfo.numOfPortGroups;
    }
    step++;

    return step;
}

static GT_BOOL useForcedResetMode = GT_FALSE;
static GT_BOOL forcedResetMode = GT_FALSE;
/**
* @internal prvTgfResetModeGet function
* @endinternal
*
* @brief   Get reset mode for enhanced UT's.
*/
GT_BOOL prvTgfResetModeGet
(
    GT_VOID
)
{
    GT_U32 devIdx;
    GT_U32 maxDevIdx = (sizeof(appDemoPpConfigList) / sizeof(appDemoPpConfigList[0]));

    if(prvTgfDevNum < utfFirstDevNumGet())
    {
        return GT_FALSE;
    }

    if(useForcedResetMode == GT_TRUE)
    {
        return forcedResetMode;
    }

    for (devIdx = 0; (devIdx < maxDevIdx); devIdx++)
    {
        /* those devices supported by ASIC_SIMULATION */
        if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E ||
            PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
        {
            /*allow to use <devSupportSystemReset_HwSoftReset>*/
        }
        else
        /* for LION, LION2, XCAT, XCAT2 and XCAT3 we don't allow to do 'soft reset' only remove device and do board specific type reset */
        if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E) && (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily <= CPSS_PP_FAMILY_DXCH_LION2_E))
        {
            return GT_FALSE;
        }
        if (appDemoPpConfigList[devIdx].valid == GT_FALSE)
        {
            continue;
        }
        if (appDemoPpConfigList[devIdx].devNum != prvTgfDevNum)
        {
            continue;
        }
        return appDemoPpConfigList[devIdx].devSupportSystemReset_HwSoftReset;
    }

    return GT_FALSE;
}
/**
* @internal prvTgfResetModeSet function
* @endinternal
*
* @brief   Set reset mode for enhanced UT's.
*         allow to temporary treat device as 'non-supporting'
* @param[in] mode                     - GT_TRUE  - state that system supports Reset.
*                                      GT_FALSE - state that system not supports Reset.
*                                       reset mode
*/
GT_VOID prvTgfResetModeSet
(
    GT_BOOL mode
)
{
    if(mode == GT_FALSE)
    {
        useForcedResetMode = GT_TRUE;
    }
    else
    {
        useForcedResetMode = GT_FALSE;
    }

    forcedResetMode = mode;

}

/**
* @internal prvTgfResetModeGet_gtShutdownAndCoreRestart function
* @endinternal
*
* @brief   Get reset mode for enhanced UT's, for gtShutdownAndCoreRestart()
*/
GT_BOOL prvTgfResetModeGet_gtShutdownAndCoreRestart
(
    GT_VOID
)
{
    GT_U32 devIdx;
    GT_U32 maxDevIdx = (sizeof(appDemoPpConfigList) / sizeof(appDemoPpConfigList[0]));
    GT_U32  devNum;

    if (isHirApp == GT_TRUE)
    {
        /* 
CPSS_SYSTEM_RESET_TYPE_RESET_SW_IN_gtShutdownAndCoreRestart_E: HIR not supports
         */
         return GT_FALSE;
    }
    
    for (devIdx = 0; (devIdx < maxDevIdx); devIdx++)
    {
        if (appDemoPpConfigList[devIdx].valid == GT_FALSE)
        {
            continue;
        }
        devNum = appDemoPpConfigList[devIdx].devNum;
        if(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmDevIsInitilized == GT_TRUE)
        {
            /* the TM was initialized : the 're-init' fail on HW : */
            /*
                max poll IF #0

                RL: DDR3 poll failed(2) for IF 0 CS 0 bus 0max poll IF #0

                RL: DDR3 poll failed(2) for IF 0 CS 0 bus 1max poll IF #0

                RL: DDR3 poll failed(2) for IF 0 CS 0 bus 2max poll IF #0

                RL: DDR3 poll failed(2) for IF 0 CS 0 bus 3ddr3TipDynamicReadLeveling failure
                ********   DRAM initialization Failed (res 0x1)   ********
                cpssDxChTmGlueDramInit returned: 0x1 at file ./gtDbDxBobcat2.c, line = 6320
                appDemoBc2DramOrTmInit returned: 0x1 at file ./gtDbDxBobcat2.c, line = 6382
            */
            cpssOsPrintf("Device [%d] not supports gtShutdownAndCoreRestart , because TM initialized (traffic manager) \n");
            return GT_FALSE;
        }

#ifndef ASIC_SIMULATION
        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) &&
           (prvCpssDrvPpConfig[devNum]->hwInfo[0].busType == CPSS_HW_INFO_BUS_TYPE_MBUS_E))
        {
            /* reconfiguration of CPSS result in OOB port link down/up.
               NFS stuck Linux in case it access FS during link down.*/
            cpssOsPrintf("Device [%d] not supports gtShutdownAndCoreRestart , because OOB initialized and link down and NFS may stuck\n");
            return GT_FALSE;

        }
#endif
    }

    return GT_TRUE;
}

/*
indication for enhanced UT that do : reset, includes cpssInitSystem.
to preserve the 'pre initSystem' parameters of the previous one.
NOTE: default is 'preserve'
*/
static GT_BOOL  tgfSupportSystemResetPreserveModes = GT_TRUE;
/**
* @internal prvTgfSupportSystemResetPreserveModesSet function
* @endinternal
*
* @brief   Function for enhanced UT that do : reset, includes cpssInitSystem.
*         to preserve the 'pre initSystem' parameters of the previous one.
*         NOTE: default is 'preserve'
*         NOTE: function for debug purposes.
* @param[in] systemResetPreserveModes - GT_TRUE  - preserve the 'pre initSystem' parameters of the previous one.
*                                      - GT_FALSE - NOT preserve the 'pre initSystem' parameters of the previous one.
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvTgfSupportSystemResetPreserveModesSet
(
    IN GT_BOOL systemResetPreserveModes
)
{
    tgfSupportSystemResetPreserveModes = systemResetPreserveModes;

    return GT_OK;
}

extern void trfCpssDxChDiagDataIntegrityTables_shadowTest(
    IN const GT_CHAR *suitNamePtr ,
    IN GT_BOOL isSuitEnded
);

#ifndef __tgfIpGenh
typedef enum
{
    PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E  = 0,
    PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E     = 1,
    PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_LAST_E
} PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT;
#endif

static GT_BOOL doHwReset = GT_TRUE;
/**
  @internal prvTgfRemoveAndInsertSystem(void)
 @endinternal

 @brief function for UT remove and insert all PacketProcessor

 @retval GT_OK               - on success
 @retval GT_NOT_SUPPORTED    - on system that not support it
  **/

GT_VOID prvTgfCpssPpRemoveAndInsert(void)
{

    GT_U8 i,k=0;
    GT_32 rc;
    GT_32 devCount=appDemoPpConfigDevAmount;
    typedef struct{
        GT_PCI_INFO pciInfo;
        GT_U32 devIdx;
    } PCI_DEV_INFO; 
    PCI_DEV_INFO pciDevInfo[2];;
     
/*  pciDevInfo = (PCI_DEV_INFO*)cpssOsMalloc(sizeof(PCI_DEV_INFO)*devCount);*/
    for(i = 0; i < PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
    {
        if (appDemoPpConfigList[i].valid == GT_TRUE)
        {
            pciDevInfo[k].devIdx = i;
            pciDevInfo[k].pciInfo.pciIdSel= appDemoPpConfigList[i].pciInfo.pciIdSel;
            pciDevInfo[k].pciInfo.pciBusNum= appDemoPpConfigList[i].pciInfo.pciBusNum;
            pciDevInfo[k].pciInfo.funcNo = appDemoPpConfigList[i].pciInfo.funcNo;
            if(devCount > k+1)
                k++;
        }
    }

    for(i=0 ; i< devCount; i++)
    {
        rc = cpssPpRemove(pciDevInfo[i].devIdx,GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"Removed PP success for dev\n");

        rc = cpssPpInsert(pciDevInfo[i].pciInfo.pciBusNum,
                     pciDevInfo[i].pciInfo.pciIdSel,
                     pciDevInfo[i].devIdx);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Inserted PP success for dev\n");
    }

    isAfterSystemReset = GT_TRUE;
   /*  cpssOsFree(pciDevInfo);*/
}
/**
* @internal prvTgfResetAndInitSystem function
* @endinternal
*
* @brief   Function for enhanced UT reset, includes cpssInitSystem.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - on system that not support it
*/
GT_STATUS prvTgfResetAndInitSystem(void)
{
    GT_STATUS rc;
    GT_U32  boardIdx;
    GT_U32  boardRevId;
    GT_U32  reloadEeprom;
    GT_U32  firstDevNum = 0xFF;
    GT_BOOL pbrModeUsed = GT_FALSE;
    GT_U32  maxNumOfPbrEntries = 0;
    GT_BOOL maxNumOfPbrEntrieWasSet = GT_FALSE;
    PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT  blocksAllocationMethod = PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E;
    GT_BOOL blocksAllocationMethodWasSet = GT_FALSE;
    GT_U32  doublePhysicalPorts = 0;
    GT_U32 value=0;
    static GT_CHAR  devForceModeSystemReset_HwSoftReset_str[100];
    GT_U32 devIdx;
    GT_U32 maxDevIdx = (sizeof(appDemoPpConfigList) / sizeof(appDemoPpConfigList[0]));
    GT_U32  devNum;
    GT_U32 set_lion2PortsConfigType = 0;
    GT_U32 sharedTableMode = GT_NA;
#if (defined CHX_FAMILY)
    GT_BOOL forceLinkUpWa;
    CPSS_PORTS_BMP_STC  portIsInLoopback;
#endif
     if (isHirApp == GT_TRUE)
     {
         prvTgfCpssPpRemoveAndInsert();
         return GT_OK;
     }
   
    if(prvTgfDevNum < utfFirstDevNumGet())
    {
        return GT_NOT_SUPPORTED;
    }

    if(GT_FALSE == prvTgfResetModeGet() && doHwReset == GT_TRUE)
    {
        PRV_UTF_LOG0_MAC("prvTgfResetAndInitSystem: ******** main device not declared as supporting the feature *** \n");
        /* main device not declared as supporting the feature */
        return GT_NOT_SUPPORTED;
    }

    cpssOsBzero(devForceModeSystemReset_HwSoftReset_str, sizeof(devForceModeSystemReset_HwSoftReset_str));

    cpssOsPrintf("prvTgfResetAndInitSystem :started . doHwReset[%d] \n",doHwReset);

    if(tgfSupportSystemResetPreserveModes == GT_TRUE)
    {
        /* check if the system was initialized with "firstDevNum"*/
        appDemoDbEntryGet("firstDevNum",&firstDevNum);
        /* check if the system was 'PBR' */
        pbrModeUsed = prvUtfIsPbrModeUsed();
        /* check dynamic memory allocation mode */
        if(appDemoDbEntryGet("lpmRamMemoryBlocksCfg.lpmRamBlocksAllocationMethod", &value) == GT_OK)
        {
            blocksAllocationMethod = (PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT)value;
            blocksAllocationMethodWasSet = GT_TRUE;
        }
        /* check max PBR entries configuration  */
        if(appDemoDbEntryGet("maxNumOfPbrEntries", &value) == GT_OK)
        {
            maxNumOfPbrEntries = value;
            maxNumOfPbrEntrieWasSet=GT_TRUE;
        }

        /* check doublePhysicalPorts configuration  */
        if(appDemoDbEntryGet("doublePhysicalPorts", &value) == GT_OK)
        {
            doublePhysicalPorts = value;
        }

        if(appDemoDbEntryGet("sharedTableMode", &value) == GT_OK)
        {
            sharedTableMode = value;
        }
    }

#if (defined CHX_FAMILY)
    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PX_FAMILY_PIPE_E)
    {
        GT_U32 port;
        for(port = 0; port < CPSS_MAX_PORTS_NUM_CNS /*support 256 ports in bc2*/; port++)
        {
            rc = tgfTrafficGeneratorPortForceLinkCheckWa(prvTgfDevNum, port, &forceLinkUpWa);
            if(GT_OK != rc && GT_NOT_INITIALIZED != rc)
            {
                PRV_TGF_LOG1_MAC("FAIL : tgfTrafficGeneratorPortForceLinkCheckWa - port[%d]\n",
                                 port);
                return rc;
            }

            /* Indicate MTI WA is needed */
            if (forceLinkUpWa)
            {
                CPSS_PORTS_BMP_PORT_SET_MAC(&portIsInLoopback, port);
            }
            else
            {
                CPSS_PORTS_BMP_PORT_CLEAR_MAC(&portIsInLoopback, port);
            }
        }
    }

    if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) &&
        (doHwReset == GT_FALSE))
    {
        CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
        /* Lion2 have erratum FE-2102944. Port reconfiguration from Gig MAC to 10/20/40 G MAC
           result in first packet corruption. And this fails tests.
           Need to avoid port's reconfiguration by cpssDxChPortModeSpeedSet.
           Use "lion2PortsConfigType", 0 (SGMII) for the purpose. */
        rc = cpssDxChPortInterfaceModeGet(prvTgfDevNum, 0, &ifMode);
        if (rc == GT_OK)
        {
            if ((ifMode == CPSS_PORT_INTERFACE_MODE_SGMII_E) ||
                (ifMode == CPSS_PORT_INTERFACE_MODE_1000BASE_X_E))
            {
                set_lion2PortsConfigType = 1;
            }
        }
    }
#endif

    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);
                                                /* the prefix "prvTgf" needed */
    trfCpssDxChDiagDataIntegrityTables_shadowTest("prvTgf__just_before_system_reset",GT_FALSE);

    rc = cpssResetSystem(doHwReset);
    if(GT_OK != rc)
    {
        /* function will return GT_NOT_SUPPORTED for devices that not support it */
        PRV_UTF_LOG0_MAC("prvTgfResetAndInitSystem: ******** cpssResetSystem failed *** \n");
        return rc;
    }

    if(firstDevNum != 0 && firstDevNum != 0xFF)
    {
        appDemoDbEntryAdd("firstDevNum",firstDevNum);
    }
    if(pbrModeUsed == GT_TRUE)
    {
        appDemoDbEntryAdd("usePolicyBasedRouting",GT_TRUE);
    }
    if (blocksAllocationMethodWasSet==GT_TRUE)
    {
        switch(blocksAllocationMethod)
        {
            case PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E:
                value = 0;
                break;
            case PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E:
                value = 1;
                break;
            default:
                return GT_BAD_PARAM;
        }
        appDemoDbEntryAdd("lpmRamMemoryBlocksCfg.lpmRamBlocksAllocationMethod", value);
    }
    if (maxNumOfPbrEntrieWasSet==GT_TRUE)
    {
        appDemoDbEntryAdd("maxNumOfPbrEntries", maxNumOfPbrEntries);
    }

    if(doublePhysicalPorts)
    {
        appDemoDbEntryAdd("doublePhysicalPorts", doublePhysicalPorts);
    }

    if(sharedTableMode != GT_NA)
    {
        appDemoDbEntryAdd("sharedTableMode", sharedTableMode);
    }

    if (set_lion2PortsConfigType)
    {
        /* configure port mode to be 1G - SGMII */
        appDemoDbEntryAdd("lion2PortsConfigType", 0);
    }

    rc = cpssInitSystem(boardIdx, boardRevId, reloadEeprom);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG0_MAC("prvTgfResetAndInitSystem: ******** cpssInitSystem failed *** \n");
        return rc;
    }

    for (devIdx = 0; (devIdx < maxDevIdx); devIdx++)
    {
        if (appDemoPpConfigList[devIdx].valid == GT_FALSE)
        {
            continue;
        }

        devNum = appDemoPpConfigList[devIdx].devNum;

        cpssOsSprintf(devForceModeSystemReset_HwSoftReset_str,"devForceModeSystemReset_HwSoftReset_str_%d",devNum);
        if(appDemoStaticDbEntryGet(devForceModeSystemReset_HwSoftReset_str,&value)== GT_OK)
        {
            if(value)
            {
                cpssOsPrintf("prvTgfResetAndInitSystem: restore devSupportSystemReset_HwSoftReset TRUE\n");
                appDemoPpConfigList[devIdx].devSupportSystemReset_HwSoftReset = GT_TRUE;
            }
            else
            {
                cpssOsPrintf("prvTgfResetAndInitSystem: restore devSupportSystemReset_HwSoftReset FALSE\n");
                appDemoPpConfigList[devIdx].devSupportSystemReset_HwSoftReset = GT_FALSE;
            }
        }

#if (defined CHX_FAMILY)
        if(appDemoPpConfigList[devIdx].devFamily != CPSS_PX_FAMILY_PIPE_E)
        {
            if(doHwReset == GT_FALSE)
            {
                GT_U32 port;
                GT_BOOL isForced;
                CPSS_INTERFACE_INFO_STC portInterface;

                for(port = 0 ; port < CPSS_MAX_PORTS_NUM_CNS /*support 256 ports in bc2*/; port++)
                {
                    forceLinkUpWa = CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portIsInLoopback, port);
                    if (forceLinkUpWa)
                    {
                        rc = GT_OK;
                        isForced = GT_TRUE;
                    }
                    else
                    {
                        rc = cpssDxChPortForceLinkPassEnableGet((GT_U8)devNum, port, &isForced);
                    }
                    /* the Bobcat2 HW on low ports (QSGMII) did not liked the 'FORCE link UP'
                    and caused 100M ports to become 10M without ability to send/receive packets */
                    if(rc == GT_OK && isForced == GT_TRUE)
                    {
                        cpssOsPrintf("'refresh' force link up on ports [%d] \n",port);
                        (void)cpssDxChPortForceLinkPassEnableSet((GT_U8)devNum, port, GT_FALSE);
                        osTimerWkAfter(10);

                        portInterface.type            = CPSS_INTERFACE_PORT_E;
                        portInterface.devPort.hwDevNum  = devNum;
                        portInterface.devPort.portNum = port;

                        /* use TGF function to configure proper port settings for traffic tests */
                        rc = tgfTrafficGeneratorPortForceLinkUpEnableSet(&portInterface, GT_TRUE);
                        if (GT_OK != rc)
                        {
                            cpssOsPrintf("prvTgfResetAndInitSystem: tgfTrafficGeneratorPortForceLinkUpEnableSet failure rc = [%d]\n", rc);
                        }
                    }
                }
                osTimerWkAfter(10);
            }
        }
#endif
    }

    isAfterSystemReset = GT_TRUE;

    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E ||
       PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E ||
       PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        CPSS_TBD_BOOKMARK_BOBCAT3
        /* PATCH : until the simulation and the CPSS will send link up/down indication
            about all 74 MAC ports

            needed because the call to cpssDxChBrgEgrFltPortLinkEnableSet(...)
        */
#ifdef CHX_FAMILY
        prvTgfTestsPortRangeSelect(60);
#endif /*CHX_FAMILY*/
    }

    cpssOsPrintf("prvTgfResetAndInitSystem :finished . doHwReset[%d] \n",doHwReset);

    return GT_OK;

}

/**
* @internal prvTgfResetAndInitSystem_gtShutdownAndCoreRestart function
* @endinternal
*
* @brief   Function for enhanced UT reset, includes cpssInitSystem.
*         the function emulates the PSS function of gtShutdownAndCoreRestart()
*         with one 'little' restriction , this function NOT clean any HW config !
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - on system that not support it
*/
GT_STATUS prvTgfResetAndInitSystem_gtShutdownAndCoreRestart(void)
{
    GT_STATUS rc;

    if(GT_FALSE == prvTgfResetModeGet_gtShutdownAndCoreRestart())
    {
        PRV_UTF_LOG0_MAC("prvTgfResetAndInitSystem_gtShutdownAndCoreRestart: ******** main device not declared as supporting the feature *** \n");
        /* main device not declared as supporting the feature */
        return GT_NOT_SUPPORTED;
    }

    doHwReset = GT_FALSE;
    rc = prvTgfResetAndInitSystem();
    doHwReset = GT_TRUE;

    return rc ;
}

/**
* @internal tgfStateStartTest function
* @endinternal
*
* @brief   state that test started
*
* @note needed for system reset feature
*
*/
GT_VOID tgfStateStartTest(GT_VOID)
{
    isAfterSystemReset = GT_FALSE;
}
/**
* @internal tgfIsAfterSystemReset function
* @endinternal
*
* @brief   get indication that we are After System Reset.
*
* @note needed for system reset feature
*
*/
GT_BOOL tgfIsAfterSystemReset(GT_VOID)
{
    return isAfterSystemReset;
}

/**
* @internal prvUtfIsCpuPortMacSupported function
* @endinternal
*
* @brief   Function checks support of MAC layer existence for CPU Port.
*
* @param[in] dev                      -  device id
*
* @retval GT_TRUE                  - CPU Port has MAC and MAC related APIs may be called
* @retval GT_FALSE                 - CPU Port does not have MAC and MAC related APIs returns error code
*/
GT_BOOL prvUtfIsCpuPortMacSupported
(
    IN GT_U8                    dev
)
{
#if (defined CHX_FAMILY)
    GT_BOOL isCpuMacExist;

    isCpuMacExist = (((dev) < PRV_CPSS_MAX_PP_DEVICES_CNS) ?
           (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE) :
           GT_TRUE);
    return isCpuMacExist;
#elif (defined PX_FAMILY)
    dev = dev;
    return GT_FALSE;
#else
    dev = dev;
    return GT_TRUE;
#endif
}

/**
* @internal prvUtfIsVplsModeUsed function
* @endinternal
*
* @brief   This routine returns GT_TRUE if VPLS mode used. GT_FALSE otherwise.
*
* @param[in] devNum                   - device number
*/
GT_BOOL prvUtfIsVplsModeUsed
(
    IN GT_U8 devNum
)
{
#if (defined CALL_EX_MX_CODE_MAC)
    return GT_FALSE;
#elif (defined CHX_FAMILY)
    if (0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        return GT_FALSE;
    }
    if (0 == PRV_CPSS_DXCH_FAMILY_CHECK_MAC(devNum))
    {
        return GT_FALSE;
    }
    return PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_ENABLED_MAC(devNum);
#else /* (defined CHX_FAMILY) */
    devNum = devNum;
    return GT_FALSE;
#endif
}

/**
* @internal utfFirstDevNumGet function
* @endinternal
*
* @brief   This routine returns the first devNum applicable.
*/
GT_U8   utfFirstDevNumGet(void)
{
    GT_U8 dev;

    prvUtfNextNotApplicableDeviceReset(&dev,UTF_NONE_FAMILY_E);
    prvUtfNextDeviceGet(&dev,GT_TRUE);

    /* indicate that we are done with this iterator */
    if(utIteratorDepth)
    {
        utIteratorDepth--;
    }

    return dev;
}

/**
* @internal utfExtraTestEnded function
* @endinternal
*
* @brief   Function to state that the current test ended.
*/
void utfExtraTestEnded(void)
{
    utCurrentTestEnded = GT_TRUE;
}

/**
* @internal prvUtfValidPortsGet function
* @endinternal
*
* @brief   get valid ports (according to portType) starting from 'start number'
*
* @param[in] dev                      - the device.
* @param[in] startPortNum             - the minimal port number (inclusive)
* @param[in] numOfPortsToGet          - number of ports to get (according to portType)
*
* @param[out] validPortsArr[]          - array of valid port.(according to portType)
*                                      in the case that there are not enough valid MAC ports.
*                                      the array filled with PRV_TGF_INVALID_VALUE_CNS
*                                      Returns :
*                                      None.
*                                      COMMENTS:
*                                      None.
*/
void prvUtfValidPortsGet(
    IN  GT_U8       dev ,
    IN UTF_GENERIC_PORT_ITERATOR_TYPE_E     portType,
    IN  GT_U32      startPortNum,
    OUT GT_U32      validPortsArr[],
    IN  GT_U32      numOfPortsToGet
)
{
    GT_STATUS st;
    UTF_PHYSICAL_PORT_NUM     port;
    GT_U32  ii = 0;

    prvUtfNextGenericPortItaratorTypeSet(portType);

    st = prvUtfNextGenericPortReset(&port, dev);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    if(startPortNum > 0)
    {
        port = startPortNum - 1;
    }

    /* 1.1. For all active devices go over all available physical ports. */
    while(GT_OK == prvUtfNextGenericPortGet(&port, GT_TRUE))
    {
        if((ii+1) > numOfPortsToGet)
        {
            break;
        }

        validPortsArr[ii] = port;
        ii++;
    }

    /* in the case that there are not enough valid ports. */
    for(/*ii continu*/; ii < numOfPortsToGet; ii++)
    {
        validPortsArr[ii] = PRV_TGF_INVALID_VALUE_CNS;
    }

    return;
}

static GT_BOOL  mustUseOrigPorts = GT_FALSE;
void utfSetMustUseOrigPorts(IN GT_BOOL  useOrigPorts)
{
    mustUseOrigPorts = useOrigPorts;
}
/**
* @internal utfIsMustUseOrigPorts function
* @endinternal
*
* @brief   check if must use only ports from prvTgfPortsArray[]
*         check if there is problem with using 'any port num' , and must work with
*         the 12 ports that the DB hold from the initialization.
*/
GT_U32  utfIsMustUseOrigPorts(IN GT_U8  devNum)
{
    if(mustUseOrigPorts == GT_TRUE)
    {
        return GT_TRUE;
    }

#ifdef CHX_FAMILY
    if(IS_BOBK_DEV_CETUS_MAC(devNum))
    {
        /* the CETUS hold only 12 ports , so we are not able to choose any port we want */
        return 1;
    }
#endif

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        /* the bobcat3 was initialized in AppDemo with only 12 ports ,
           because the GM supports only single data path (12 ports)
           so we are not able to choose any port we want */
        return 1;
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        /* let appDemo select explicit 12 ports */
        return 1;
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        /* let appDemo select explicit 12 ports */
        return 1;
    }

    return 0;
}

/* Lock mutex with possibility to detect deadlock */
static GT_STATUS prvUtfSafeLock(IN GT_MUTEX mtx)
{
    GT_STATUS    rc;
    GT_U32       index;
    GT_U32       selfId, ownerId;

    /* get current threadId*/
    if (osTaskGetSelf(&selfId) != GT_OK)
        return GT_FAIL;

    /* lock safe lock protection */
    if (lockSavedFunc(prvUtfSafeLockProtection) != GT_OK)
    {
        return GT_FAIL;
    }

    /* get thread that owns current mutex (if exist) */
    ownerId = mutexState[mtx].lock;

    /* check for deadlocks */
    if (ownerId != 0)
    {
        for (index = 0;index < MAX_MUTEX_NUM; index++)
        {
            if ((mutexState[index].wait == ownerId) && (mutexState[index].lock == selfId))
            {
                /* deadlock!!*/
                cpssOsPrintf("Deadlock!! Mutexes: mtx = [%d], blocker = [%d], selfId = [%d], ownerId = [%d] \n", mtx, index, selfId, ownerId);
                isDeadlockDetected = GT_TRUE;

                /* unlock safe lock protection */
                if (unlockSavedFunc(prvUtfSafeLockProtection) != GT_OK)
                {
                    return GT_FAIL;
                }

                return GT_FAIL;
            }
        }
    }

    /* set status thread xxx trying to lock mtx */
    mutexState[mtx].wait = selfId;
    /* unlock safe lock protection */
    if (unlockSavedFunc(prvUtfSafeLockProtection) != GT_OK)
    {
        return GT_FAIL;
    }

    rc = lockSavedFunc(mtx);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* set status mutex is owned by thread xxx */
    if (mutexState[mtx].wait == selfId)
    {
        mutexState[mtx].wait = 0;
    }
    mutexState[mtx].lock = selfId;

    return GT_OK;
}

/* Unlock mutex with deadlock detection */
static GT_STATUS prvUtfSafeUnlock(GT_MUTEX mtx)
{
    GT_STATUS   rc;

    mutexState[mtx].lock = 0;
    rc = unlockSavedFunc(mtx);
    return rc;
}

/**
* @internal prvUtfSafeDeadLockCheck function
* @endinternal
*
* @brief   Return value of flag, that indicates if we have deadlock
*/
GT_BOOL prvUtfSafeDeadLockCheck()
{
    return isDeadlockDetected;
}

/**
* @internal prvUtfSafeDeadLockStatusClear function
* @endinternal
*
* @brief   Clean value of flag, that indicates if we have deadlock
*/
GT_VOID prvUtfSafeDeadLockStatusClear()
{
    isDeadlockDetected = GT_FALSE;
    return;
}

/**
* @internal utfUnsafeLock function
* @endinternal
*
* @brief   Lock mutex
*
* @param[in] mtx                      - mutex Id.
*/
GT_STATUS utfUnsafeLock(IN GT_MUTEX mtx)
{
   return cpssOsMutexLock(mtx);
}

/**
* @internal utfUnsafeUnlock function
* @endinternal
*
* @brief   Unlock mutex
*
* @param[in] mtx                      - mutex Id.
*/
GT_STATUS utfUnsafeUnlock(IN GT_MUTEX mtx)
{
   return cpssOsMutexUnlock(mtx);
}

/**
* @internal utfStartUsingSafeLocking function
* @endinternal
*
* @brief   Redefine mutex lock/unlock functions to catch deadlock
*/
GT_STATUS utfStartUsingSafeLocking(GT_VOID)
{
    lockSavedFunc = cpssOsMutexLock;
    unlockSavedFunc = cpssOsMutexUnlock;

    cpssOsMutexLock = prvUtfSafeLock;
    cpssOsMutexUnlock = prvUtfSafeUnlock;

    /* initialize Safe Lock Protection mutex */
    return cpssOsMutexCreate("prvUtfSafeLockProtection",&prvUtfSafeLockProtection);
}

/**
* @internal utfStopUsingSafeLocking function
* @endinternal
*
* @brief   Restore default mutex functions
*/
GT_STATUS utfStopUsingSafeLocking(GT_VOID)
{
    cpssOsMutexLock = lockSavedFunc;
    cpssOsMutexUnlock = unlockSavedFunc;

    /* delete Safe Lock Protection mutex */
    return cpssOsMutexDelete(prvUtfSafeLockProtection);
}

/**
* @internal utfTcamPortGroupsBmpForCurrentPortGroupId function
* @endinternal
*
* @brief   Creates from port group id tcam port group bmp
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - portgroup ID
*/
GT_PORT_GROUPS_BMP utfTcamPortGroupsBmpForCurrentPortGroupId
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId
)
{
    GT_PORT_GROUPS_BMP portGroupsBmp;
    GT_PORT_GROUPS_BMP tcam_portGroupsBmp;

    portGroupsBmp = 1 << portGroupId;
    if(!PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        /* no more to do */
    }
    else
    if(PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        /* every 4 port groups share the tcam */
        portGroupsBmp = 0xF << (portGroupId/4);
    }
    else
    {
        tcam_portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
#ifdef CHX_FAMILY
        /* sip 5 multi-port group device */
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,tcam_portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
#endif /*CHX_FAMILY*/

        if(0 == (tcam_portGroupsBmp & portGroupsBmp))
        {
            /* port group should be skipped */
            return 0;
        }
    }

    return portGroupsBmp;
}

GT_U32 utfFamilyTypeGet(IN GT_U8 devNum)
{
    if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        return UTF_NONE_FAMILY_E;
    }

    return extCtx.devArray[devNum];
}

/**
* @internal prvUtfIsAc3xWithPipeSystem function
* @endinternal
*
* @brief   Is current system AC3X + PIPE
*
* @retval GT_TRUE                  - exists
* @retval GT_FALSE                 - not exists
*/
GT_BOOL prvUtfIsAc3xWithPipeSystem
(
    GT_VOID
)
{
    GT_U32  boardIdx;
    GT_U32  boardRevId;
    GT_U32  reloadEeprom;

    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);
    if((32 == boardIdx) && (5 == boardRevId))
    {
        return GT_TRUE;
    }
    else
    {
        return GT_FALSE;
    }
}

static GT_U32   prvUtfOffsetFrom0ForUsedPorts = 0;
/**
* @internal prvUtfOffsetFrom0ForUsedPortsGet function
* @endinternal
*
* @brief   the function return the 'offset from 0' for the used ports in
*         prvTgfPortsArray[].
*/
GT_U32 prvUtfOffsetFrom0ForUsedPortsGet(void)
{
    return  prvUtfOffsetFrom0ForUsedPorts;
}
/**
* @internal prvUtfOffsetFrom0ForUsedPortsSet function
* @endinternal
*
* @brief   Set the 'offset from 0' for the used ports in prvTgfPortsArray[].
*
* @param[in] offset                   - the offset
*                                       None.
*/
GT_VOID prvUtfOffsetFrom0ForUsedPortsSet(IN GT_U32   offset)
{
    prvUtfOffsetFrom0ForUsedPorts = offset;
}


