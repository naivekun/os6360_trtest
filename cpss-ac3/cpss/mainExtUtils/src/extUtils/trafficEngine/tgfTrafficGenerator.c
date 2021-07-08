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
* @file tgfTrafficGenerator.c
*
* @brief emulate traffic generator capabilities
* reset counters
* read counters
* send traffic
* capture received traffic
* set trigger on received traffic
*
* @version   143
********************************************************************************
*/

#ifdef COW_INTERNAL_COMPILE
    /* those 3 are defined , so remove them */
    #undef EXMXPM_FAMILY
    #undef EX_FAMILY
    #undef CHX_FAMILY

    /* define only the tested one */
    #define CHX_FAMILY

    #undef IMPL_GALTIS
    #define IMPL_GALTIS
#endif /*COW_INTERNAL_COMPILE*/

/* for VC as usually used compiler - check mismatch of sizeof().  */
#ifdef _VISUALC
    #define CHECK_SIZE_MISMATCH
#endif

#if (defined CHX_FAMILY)
    #define DXCH_CODE
#endif /* (defined CHX_FAMILY) */
#if (defined PX_FAMILY)
    #define PX_CODE
#endif /* (defined PX_FAMILY) */

/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/common/cpssTypes.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <extUtils/trafficEngine/tgfTrafficGenerator.h>
#include <extUtils/trafficEngine/tgfTrafficTable.h>
#include <extUtils/iterators/extUtilDevPort.h>
#include <gtOs/gtOsTimer.h>
#include <gtUtil/gtBmPool.h>
#include <extUtils/trafficEngine/prvTgfLog.h>
#include <extUtils/trafficEngine/private/prvTgfInternalDefs.h>

#ifdef PX_CODE
    #include <cpss/px/networkIf/cpssPxNetIf.h>
    #include <cpss/px/port/cpssPxPortStat.h>
    #include <cpss/px/port/cpssPxPortCtrl.h>
    #include <cpss/px/config/private/prvCpssPxInfo.h>
    #include <cpss/px/port/private/prvCpssPxPort.h>
#endif /*PX_CODE*/

#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/port/cpssDxChPortStat.h>
    #include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
    #include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
    #include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
    #include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
    #include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
    #include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
    #include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
    #include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChCfg.h>
    #include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
    #include <cpss/dxCh/dxChxGen/pcl/private/prvCpssDxChPcl.h>
    #include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
    #include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
    #include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>
    #include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
    #include <cpss/dxCh/dxChxGen/vnt/cpssDxChVnt.h>

typedef GT_U32 (*APP_DEMO_DXCH_TCAM_PCL_CONVERTED_INDEX_GET_FUN)
(
    IN     GT_U8                                devNum,
    IN     GT_U32                               index,
    IN     CPSS_DXCH_TCAM_RULE_SIZE_ENT         ruleSize
);
APP_DEMO_DXCH_TCAM_PCL_CONVERTED_INDEX_GET_FUN appDemoDxChTcamPclConvertedIndexGetPtr = NULL;
#define TGF_DXCH_TCAM_PCL_CONVRT_IDX_GET_MAC(devNum,index,ruleSize) \
    ((appDemoDxChTcamPclConvertedIndexGetPtr == NULL) \
        ? 0 \
        : appDemoDxChTcamPclConvertedIndexGetPtr(devNum,index,ruleSize))

#if 0
extern GT_STATUS appDemoDxChNewPclTcamSupportSet
(
    IN  GT_BOOL             enable
);
#endif
#endif /* CHX_FAMILY */


#ifdef ASIC_SIMULATION
#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SLog/simLog.h>
#endif /*ASIC_SIMULATION*/

static GT_U8  txQueueNum = 7;

GT_BOOL tgfPrvIntendedError = GT_FALSE;

typedef GT_STATUS (*appDemoDbEntryGet_TYPE)
(
    IN  GT_CHAR *namePtr,
    OUT GT_U32  *valuePtr
);

appDemoDbEntryGet_TYPE appDemoDbEntryGet_func = NULL;

GT_U32 *tgfCmdCpssNetEnhUtUseCaptureToCpuPtr = NULL;

/* the GM not supports mapping of 'Target physical port' to 'MAC number'
   therefore any 'Physical port' with 'MAC number' > 71 is declined , so GM will
   not crash on MIB counters of such port
*/
#define BC2_GM_NOT_SUPPORT_TXQ_PORT_MAPPING_CNS     1

#define PRV_TGF_STC_S2D_FIELD_COPY_MAC(dstStcPtr, srcStcPtr, field)            \
        (dstStcPtr)->field = (srcStcPtr)->field
#define PRV_TGF_STC_S2D_UNION_FIELD_COPY_MAC(dstStcPtr, srcStcPtr, field)            \
        cpssOsMemCpy((&(dstStcPtr)->field),&((srcStcPtr)->field),sizeof((dstStcPtr)->field))
#define PRV_TGF_STC_D2S_FIELD_COPY_MAC(dstStcPtr, srcStcPtr, field)            \
        (srcStcPtr)->field = (dstStcPtr)->field
#define PRV_TGF_STC_D2S_UNION_FIELD_COPY_MAC(dstStcPtr, srcStcPtr, field)            \
        PRV_TGF_STC_S2D_UNION_FIELD_COPY_MAC(srcStcPtr,dstStcPtr,field)


/* flag to use contineouse FWS */
GT_U32 prvTgfFwsUse = 0;

/* indication for function tgfTrafficGeneratorPortTxEthCaptureSet(...)
       to allow loopback on the 'captured port' or not. */
static GT_BOOL txEthCaptureDoLoopback = GT_TRUE;

/* indication to wait some time after disable loopback. This allows RX task to handle
   all packets before RX capture will be disabled in RX task */
static GT_BOOL txEthCaptureWaitAfterDisableLoopback = GT_FALSE;

/* mark DB entry as free */
#define  PRV_TGF_TX_MODE_FREE_DB_ENTRY(_index) U32_SET_FIELD_MAC(txDevPortContUsedBmp,(_index), 1, 0)

/* mark DB entry as allocated */
#define  PRV_TGF_TX_MODE_ALLOC_DB_ENTRY(_index) U32_SET_FIELD_MAC(txDevPortContUsedBmp,(_index), 1, 1)

/* is DB not empty */
#define PRV_TRG_TX_MODE_DB_NOT_EMPTY (txDevPortContUsedBmp)

/* the DB */
static PRV_TGF_TX_DEV_PORT_STC txDevPortArr[PRV_TGF_TX_PORTS_MAX_NUM_CNS];
static GT_U32 txDevPortContUsedBmp = 0;

#ifdef DXCH_CODE
/* store mirroring mode and descriptors limits */
static CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT mirrModeStore;
static CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT      mirrPriorityMode;

static GT_U32 mirrRxDescStore;
static GT_U32 mirrTxDescStore;
#endif /*DXCH_CODE*/

#ifdef PX_CODE
static GT_U32  prvTgfRestoreCpuPortBmp[CPSS_MAX_PORTS_NUM_CNS] = {0};
#endif /* PX_CODE */

#ifdef PX_CODE
static CPSS_PX_PORTS_BMP prvTgfSrcTableEntryRestore;
#endif /* PX_CODE */

#ifndef ASIC_SIMULATION
/* number of packets to get WS traffic. */
GT_U32 prvTgfWsBurst = 35;
/* flag to use VNT time stamps for rate calculation */
GT_U32 prvTgfCommonVntTimeStampsInUse = 1;
#else /*ASIC_SIMULATION*/
/* number of packets to get 'WS' traffic. */
GT_U32 prvTgfWsBurst = 1;
/*simulation not supports the feature*/
GT_U32 prvTgfCommonVntTimeStampsInUse = 0;
#endif/*ASIC_SIMULATION*/

/* debug flag to check rate by VNT Time Stamp */
GT_U32 prvTgfCommonVntTimeStampsDebug = 0;
/* check if the device supports prvTgfCommonVntTimeStampsInUse */
/* CH+ device not support it */
#define DEV_SUPPORT_VNT_TIME_STAMPS_MAC(devNum) \
    ((PRV_CPSS_PP_MAC(devNum)->appDevFamily & (CPSS_CH1_E | CPSS_CH1_DIAMOND_E)) ? 0:1)
/******************************************************************************\
 *                              Define section                                *
\******************************************************************************/
/* number of packets thae we set as max 'burst' when working with SDMA in 'abort' mode */
/* set this value , so traffic that go to the CPU will not be dropped do to lack
   of 'RX to CPU' resources

   the problem occurs to me only in linux simulation (and not in win32)


*/
#if (defined _WIN32)
    /* win 32 sim */
    #define SDMA_ABORT_MAX_CNS  32
#else
        #if (defined LINUX_SIM)
            /* linux sim */
            #define SDMA_ABORT_MAX_CNS  4 /* 16 was not got enough */
        #else
            /* vxWorks HW (not checked on linux HW) */
            #define SDMA_ABORT_MAX_CNS  16 /*was set to 16 because on Lion BOARD value 32 wan not good enough */
        #endif
#endif

#define MAX_RETRY_CNS   5

#if 0
/* the GM of Puma3 supports send of the 'from cpu' and the loopback
   traffic in the context of the sending task (this current task)
   so AFTER the cpss called for sending the packet , the GM already finished
   processing all the traffic.
   */
    #ifdef GM_USED
        #define SUPPORT_ALL_IN_SINGLE_CONTEXT_MAC   \
            ((PRV_CPSS_PP_MAC(prvTgfCpuDevNum)->devFamily == CPSS_PP_FAMILY_PUMA3_E) ? 1 : 0)
    #else
        #define SUPPORT_ALL_IN_SINGLE_CONTEXT_MAC   0
    #endif
#else /*1*/
    /* although the puma3 GM supports mac loopback on the ports.
       there is problem on the fabric ports that set as 'mac internal loopback' .
       so the appdemo not set those ports with internal mac loopback but as 'external mac loopbac'
       (with the help of the simulation that covers the GM) */
    #define SUPPORT_ALL_IN_SINGLE_CONTEXT_MAC   0
#endif/*1*/

#define DEV_0_CNS    0
/* default device number */
__THREAD_LOCAL GT_U8   prvTgfDevNum       = DEV_0_CNS;

/* flag where to configure loopback on port */
PRV_TGF_LOOPBACK_UNIT_ENT prvTgfLoopbackUnit = PRV_TGF_LOOPBACK_UNIT_MAC_E;


/* list of ports in devices that must be added to all vlans and MC (vidx) groups */
PRV_TGF_MEMBER_FORCE_INFO_STC  prvTgfDevPortForceArray[PRV_TGF_FORCE_MEMBERS_COUNT_CNS];
/* number of ports/devices in arrays of :prvTgfDevPortForceArray[] */
/* by default there are no members that need to forced to all vlans */
GT_U32    prvTgfDevPortForceNum = 0;


/* do we sent traffic to the CPU ? */
GT_U32 tgfTrafficGeneratorRxCaptureNum = 0;
/* indication that a test set traffic to be sent to the CPU , but not relate to 'capture' */
GT_BOOL tgfTrafficGeneratorExpectTraficToCpuEnabled = GT_FALSE;
/* number of packets that returned 'not GT_OK' on send */
GT_U32  prvTgfTrafficGeneratorNumberPacketsSendFailed = 0;
/* number of packets that returned 'GT_OK' on send */
GT_U32  prvTgfTrafficGeneratorNumberPacketsSendOk = 0;

/* enable print of TX Packet events in log file */
GT_BOOL prvTgfTrafficPrintPacketTxEnable = GT_TRUE;

/* the number of 'HW errors' that we got from trying to send packets.

    once we get those error in consecutive , there will be no recovery !

    we need to end the running ... to stop wasting time .
*/
static GT_U32   consecutiveHwErrors = 0;

#ifdef DXCH_CODE
/* default PCL Ingress Policy state */
static GT_BOOL  prvTgfDefIngressPolicyState = GT_FALSE;
#endif

static GT_U32   miiCounter = 10;
/* the device number that is connected to the CPU via it we transmit traffic
   to the 'loopback' port */
GT_U8   prvTgfCpuDevNum = 0;

/* default number of time loops to send packet */
#define TGF_DEFAULT_TIME_LOOP_NUM_CNS   5

/* number of time loops to send packet */
static GT_U32   prvTgfTimeLoop = TGF_DEFAULT_TIME_LOOP_NUM_CNS;

/* maximal time in 10 milliseconds for burst Tx.
   default is 300 milliseconds

    changed to 300 because : Lion2 in WIN32 simulation with ports 0..127 needed more then 100 milliseconds
*/
#ifdef  EMBEDDED
    /* run on hardware - two tick sleep is enough */
#define COUNTERS_POOLING_MAX_COUNT_CNS 2
#else
    /* run on simulation */
#define COUNTERS_POOLING_MAX_COUNT_CNS 300
#endif

GT_U32 prvTgfTrafficRxCheckMaxCount = COUNTERS_POOLING_MAX_COUNT_CNS;

/* maximal number of simulation idle state pooling in milliseconds */
#ifdef LINUX_SIM
/* minimal sleep time */
#define MINIMAL_SLEEP_CNS 40
/* mandatory sleep time */
#define MANDATORY_SLEEP_CNS 10
#else
/* minimal sleep time */
#define MINIMAL_SLEEP_CNS 10
/* mandatory sleep time */
#define MANDATORY_SLEEP_CNS 0
#endif

/* the Linux and Windows parallel UT run required minimum 5000 milliseconds */
#define IDLE_POOLING_MAX_COUNT_CNS 5000
/*
state that the test doing storming on the ingress port (ingress traffic
will be send back to ingress port that is in 'loopback mode').
this indication needed for WM to know to break the loop regardless to
indications from skernelStatusGet()
*/
static GT_BOOL  tgfStormingExpected = GT_FALSE;

/* default number for time factor for GM devices */
#define TGF_DEFAULT_GM_TIME_FACTOR_CNS   3
/* time factor for GM devices , due to very slow processing of packets (2 per second) */
/* for 'fine tuning' / debug use function tgfTrafficGeneratorGmTimeFactorSet(...) */
static GT_U32   prvTgfGmTimeFactor = TGF_DEFAULT_GM_TIME_FACTOR_CNS;/* used when GM_USED */

/* default number of loops to get captured packet */
#define TGF_DEFAULT_CAPTURE_LOOP_NUM_CNS 7

/* number of time loops to get captured packet */
GT_U32   prvTgfCaptureTimeLoop = TGF_DEFAULT_CAPTURE_LOOP_NUM_CNS;

/*flag to force link on huge sending*/
static GT_U32   debug_forcePrint = 0;

#ifdef GM_USED
/* do we wait for traffic that need to trap to cpu */
static GT_U32   retryRxToCpuAndWait = 1;
#else
/* do we wait for traffic that need to trap to cpu */
static GT_U32   retryRxToCpuAndWait = 0;
#endif

/* use this devNum for 'from cpu' packets for the 'src Dev' */
#define SIP5_SRC_DEV_FROM_CPU    0x3f0


/* ports array */
GT_U32   prvTgfPortsArray[PRV_TGF_MAX_PORTS_NUM_CNS] =
{
    0, 8, 18, 23, /* 4 ports of 'default' tests */
    0, 0,  0,  0,
    0, 0,  0,  0,
    0, 0,  0,  0,
    0, 0,  0,  0
};


#define  DEV_CHECK_MAC(dev) if(!PRV_CPSS_IS_DEV_EXISTS_MAC(dev)) return GT_BAD_PARAM
#ifdef CHX_FAMILY
    #ifndef TGF_SPECIFIC_DEVICE_FORMAT_SIZE_CNS
        #define TGF_SPECIFIC_DEVICE_FORMAT_SIZE_CNS  sizeof(CPSS_DXCH_NET_RX_PARAMS_STC)
    #endif /* TGF_SPECIFIC_DEVICE_FORMAT_SIZE_CNS */
    /* E ARCH supported by the device */
    #define TGF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(_devNum)            \
        (((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) ?                     \
           (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(_devNum)) :    \
           GT_FALSE)
    #define TGF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(_devNum)              \
        (((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) ?                     \
           (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(_devNum)) :      \
           GT_FALSE)
#else
    #define TGF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(_devNum) \
        /* dummy check to allow 'macro' to optionally return 'GT_TRUE' so code that */ \
        /* uses it will never state 'statement not reached' */      \
        (((_devNum) == 255 ) ? GT_TRUE : GT_FALSE)
#endif /* CHX_FAMILY */

#ifndef TGF_SPECIFIC_DEVICE_FORMAT_SIZE_CNS
    #define TGF_SPECIFIC_DEVICE_FORMAT_SIZE_CNS  1
#endif /* TGF_SPECIFIC_DEVICE_FORMAT_SIZE_CNS */

/*number of buffer at the pool*/
#define   TGF_MAX_NUM_OF_BUFFERS_AT_POOL    20

static GT_POOL_ID prvTgfTxBuffersPoolId = 0;

/* set PCL id for the capture */
static GT_U16   prvTgfTrafficCapturePclId = (BIT_7 - 1);/* use 7 bits to support ExMx */

/* PCL id that will represent the  capture */
#define   TGF_CAPTURE_PCL_ID_CNS (prvTgfTrafficCapturePclId)

/* set index for the PCL to use for capture*/
static GT_U32   prvTgfTrafficCapturePclRuleIndex = 255;

/* rule index that will represent the capture */
#define TGF_PCL_RULE_INDEX_CNS (prvTgfTrafficCapturePclRuleIndex)

/* use this etherType to recognize 'Tagged packes' that send
  from the traffic generator to the PP */
static GT_U16 etherTypeForVlanTag = TGF_ETHERTYPE_8100_VLAN_TAG_CNS;

static GT_BOOL  ignore_etherTypeForVlanTag = GT_FALSE;


    #define MAX_MIRROR_PORTS_CNS    32

    /* PVID per port struct typedef */
    typedef struct
    {
        GT_BOOL         isValid;
        GT_U8           devNum;
        GT_PORT_NUM     portNum;
        GT_U16          pvid;
} PRV_TGF_PVID_PER_PORT_STC;

#ifdef DXCH_CODE
    /* global PVID DB per device, per port */
static PRV_TGF_PVID_PER_PORT_STC prvTgfGlobalPvidDatabaseArr[
    MAX_MIRROR_PORTS_CNS] = { {0} };
#endif

/* do we need to capture by PCL all packets , exclude those of specific vlan ? */
static GT_BOOL  pclCaptureExcludeVidEnabled = GT_FALSE;
/* the specific vlan to exclude from PCL capture .
  relevant only when  pclCaptureExcludeVidEnabled == GT_TRUE */
static GT_BOOL  pclCaptureExcludeVid = 0;

#ifdef DXCH_CODE
/* pointer to cpssDxChNetIfSdmaSyncTxPacketSend function */
GT_STATUS (*cpssDxChNetIfSdmaSyncTxPacketSendPtr)
(
     IN GT_U8                                    devNum,
     IN CPSS_DXCH_NET_TX_PARAMS_STC              *pcktParamsPtr,
     IN GT_U8                                    *buffList[],
     IN GT_U32                                   buffLenList[],
     IN GT_U32                                   numOfBufs
) = cpssDxChNetIfSdmaSyncTxPacketSend;

/* pointer to cpssDxChNetIfMiiSyncTxPacketSend function */
GT_STATUS (*cpssDxChNetIfMiiSyncTxPacketSendPtr)
(
     IN GT_U8                                    devNum,
     IN CPSS_DXCH_NET_TX_PARAMS_STC              *pcktParamsPtr,
     IN GT_U8                                    *buffList[],
     IN GT_U32                                   buffLenList[],
     IN GT_U32                                   numOfBufs
) = cpssDxChNetIfMiiSyncTxPacketSend;
#endif/*DXCH_CODE*/


/******************************************************************************\
 *                              API implementation                            *
\******************************************************************************/
static GT_STATUS autoChecksumField
(
    IN TGF_AUTO_CHECKSUM_FULL_INFO_STC  *checksumInfoPtr,
    IN TGF_AUTO_CHECKSUM_EXTRA_INFO_STC *checksumExtraInfoPtr
);

#ifdef GM_USED
    /* convert CPSS devNum, coreId to simulation deviceId */
    extern GT_STATUS prvCpssDrvDevNumToSimDevIdConvert
    (
        IN  GT_U8       devNum,
        IN  GT_U32      portGroupId,
        OUT GT_U32      *simDeviceIdPtr
    );

    /* set the 'loopback force mode' on a port of device */
    extern GT_STATUS scibPortLoopbackForceModeSet
    (
        IN  GT_U32      deviceId,
        IN  GT_U32      portNum,
        IN  GT_U32      mode
    );

    /* set the 'link state' on a port of device */
    extern GT_STATUS scibPortLinkStateSet
    (
        IN  GT_U32      deviceId,
        IN  GT_U32      portNum,
        IN GT_BOOL      linkState
    );
#endif /* GM_USED */

GT_U32 prvTgfWsBurstSet(GT_U32 newVal)
{
    GT_U32 oldVal =  prvTgfWsBurst;
    prvTgfWsBurst = newVal;
    return oldVal;
}

/**
* @internal internalIsNumInList function
* @endinternal
*
* @brief   Check is number in list
*
* @param[in] checkNum                 - number for check
* @param[in] numListArr[]             - array of numbers (can be NULL)
* @param[in] numListLen               - members number in numListArr[] (can be 0)
*
* @retval GT_TRUE                  - num was found in numListArr.
* @retval GT_FALSE                 - in otherwise.
*/
static GT_BOOL internalIsNumInList
(
    IN GT_U32   checkNum,
    IN GT_U32   numListArr[],
    IN GT_U32   numListLen
)
{
    GT_U32      i = 0;
    GT_U32      high, low;

    if ((numListLen == 0) || (NULL == numListArr))
    {
        return GT_FALSE;
    }

    for (i=0; i<numListLen; i++)
    {
        high = ((numListArr[i] >> 16) & 0xFFFF);
        low  = (numListArr[i] & 0xFFFF);
        if ((checkNum >= low) && (checkNum <= high))
        {
            return GT_TRUE;
        }

        if (numListArr[i] == checkNum)
        {
            return GT_TRUE;
        }
    }
    return GT_FALSE;
}

/**
* @internal tgfTrafficGeneratorPacketChecksumUpdate function
* @endinternal
*
* @brief   Update checksums in packet built by tgfTrafficEnginePacketBuild
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - parser caused bad state that should not happen
*
* @note Input parameters are arrays returened by tgfTrafficEnginePacketBuild.
*
*/
GT_STATUS tgfTrafficGeneratorPacketChecksumUpdate
(
    OUT TGF_AUTO_CHECKSUM_FULL_INFO_STC *checksumInfoPtr,
    OUT TGF_AUTO_CHECKSUM_EXTRA_INFO_STC *checksumExtraInfoPtr
)
{
    return autoChecksumField(checksumInfoPtr, checksumExtraInfoPtr);
}

/**
* @internal tgfTrafficGeneratorCpuDeviceSet function
* @endinternal
*
* @brief   Set the CPU device though it we send traffic to the 'Loop back port'
*
* @param[in] cpuDevice                - the CPU device number
*
* @retval GT_OK                    - on success
*/
GT_STATUS tgfTrafficGeneratorCpuDeviceSet
(
    IN GT_U8    cpuDevice
)
{
    if (!PRV_CPSS_IS_DEV_EXISTS_MAC(cpuDevice))
    {
        return GT_BAD_PARAM;
    }

    prvTgfCpuDevNum = cpuDevice;

    /* avoid compiler warnings */
    TGF_PARAM_NOT_USED(prvTgfTrafficCapturePclId);
    TGF_PARAM_NOT_USED(prvTgfTrafficCapturePclRuleIndex);

    return GT_OK;
}

#ifdef GM_USED
/**
* @internal simulateExternalLoopbackPortSet function
* @endinternal
*
* @brief   WA for GM devices that do not know to 'loopback'
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS simulateExternalLoopbackPortSet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL                   enable
)
{
    /* WA for GM devices that do not know to 'loopback' */
    GT_STATUS   rc;
    GT_U32  portGroupId;
    GT_U32  localPortNum;
    GT_U32  simDeviceId;
    GT_U32 portMacNum;      /* MAC number */

#ifdef CHX_FAMILY
    if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PX_FAMILY_PIPE_E)
    {
        PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
        if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
        {
            #if BC2_GM_NOT_SUPPORT_TXQ_PORT_MAPPING_CNS == 1
            if(!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                if(portNum >= PRV_CPSS_PP_MAC(devNum)->numOfPorts)
                {
                    /* the GM of BC2,Bobk not access the 'MIB' of the port according
                       to phy port and not according to 'MAC port' , so allowing such
                       high port into the configuration CRASH the GM ! */
                    return GT_FAIL;
                }
            }
            #endif
        }
    }
#else
    portMacNum = portNum;
#endif
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    localPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum);
    if((portGroupId == 2 || portGroupId == 3) &&
       PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_PUMA3_E)
    {
        /* the Puma3 device has 4 pipes that handle packets but there
           are only 2 MG units . so we have only devObjPtr->portGroupId = 0 and 1 */
        /* but still we need to get from the GM packet need to be sent to
            pipe 0 or pipe 1 or pipe 2 or pipe 3 with local port 0..11 */

        portGroupId  -=  2;
        localPortNum += 32;
    }

    rc = prvCpssDrvDevNumToSimDevIdConvert(devNum,portGroupId,&simDeviceId);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = scibPortLoopbackForceModeSet(simDeviceId,localPortNum,enable);
    return rc;
}
/**
* @internal simulatePortForceLinkUpSet function
* @endinternal
*
* @brief   WA for GM devices that not force link up according to 'mac control'
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS simulatePortForceLinkUpSet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL                   enable
)
{
    /* WA for GM devices that do not know to 'force link UP' */
    GT_STATUS   rc;
    GT_U32  portGroupId;
    GT_U32  localPortNum;
    GT_U32  simDeviceId;
    GT_U32 portMacNum;      /* MAC number */
    GT_BOOL linkUp;

#ifdef CHX_FAMILY
    if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PX_FAMILY_PIPE_E)
    {
        PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
        if (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
        {
#if BC2_GM_NOT_SUPPORT_TXQ_PORT_MAPPING_CNS == 1
            if (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                if (portNum >= PRV_CPSS_PP_MAC(devNum)->numOfPorts)
                {
                    /* the GM of BC2,Bobk not access the 'MIB' of the port according
                    to phy port and not according to 'MAC port' , so allowing such
                    high port into the configuration CRASH the GM ! */
                    return GT_FAIL;
                }
            }
#endif
        }
    }
#else
    portMacNum = portNum;
#endif
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    localPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum);
    if((portGroupId == 2 || portGroupId == 3) &&
       PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_PUMA3_E)
    {
        /* the Puma3 device has 4 pipes that handle packets but there
           are only 2 MG units . so we have only devObjPtr->portGroupId = 0 and 1 */
        /* but still we need to get from the GM packet need to be sent to
            pipe 0 or pipe 1 or pipe 2 or pipe 3 with local port 0..11 */

        portGroupId  -=  2;
        localPortNum += 32;
    }


    rc = prvCpssDrvDevNumToSimDevIdConvert(devNum,portGroupId,&simDeviceId);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* get link status */
    rc = cpssDxChPortLinkStatusGet(devNum, portNum, &linkUp);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = scibPortLinkStateSet(simDeviceId,localPortNum,enable);

    if(linkUp != enable)
    {
        cpssOsTimerWkAfter(300);/* allow the port fully 'get UP' before we send traffic to it */
    }

    return rc;
}

#endif /*GM_USED*/

#ifdef CHX_FAMILY

/*                                                "
NOTE: the emulation of DB and functions that start with prefix : sip5Emulate_
are COPIED TO : <galtisAgent\wrapCpss\dxCh\bridge\wrapCpssDxChBridgeVlan.c> --> 'Galtis wrapper' --> named wrSip5Emulate_
to remove the dependencies between the 2 libraries.
*/

static GT_U32 sip5Emulate_tpidBmpTable[2][8] = {{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}};
static GT_U32 sip5Emulate_tpidBmpTable_used[2][8] = {{1,0,0,0,0,0,0,0},{1,0,0,0,0,0,0,0}};


GT_STATUS sip5Emulate_IngressTpidProfileClear
(
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN  GT_U32               profileIndex
)
{
    GT_U32  tag = (ethMode ==  CPSS_VLAN_ETHERTYPE0_E) ? 0 : 1;

    if(profileIndex >= 8)
    {
        return GT_BAD_PARAM;
    }

    sip5Emulate_tpidBmpTable_used[tag][profileIndex] = 0;
    sip5Emulate_tpidBmpTable[tag][profileIndex] = 0xFFFFFFFF;

    return GT_OK;
}

GT_STATUS sip5Emulate_IngressTpidProfileGet
(
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN  GT_U32               tpidBmp,
    OUT GT_U32               *profilePtr
)
{
    GT_U32  ii;
    GT_U32  iiFirstFree = 0xFF;
    GT_U32  tag = (ethMode ==  CPSS_VLAN_ETHERTYPE0_E) ? 0 : 1;

    /* look for the tpidBmp in the table */
    for(ii = 0 ; ii < 8; ii++)
    {
        if(sip5Emulate_tpidBmpTable_used[tag][ii] == 0)
        {
            if(iiFirstFree == 0xFF)
            {
                iiFirstFree = ii;
            }

            continue;
        }

        if(sip5Emulate_tpidBmpTable[tag][ii] == tpidBmp)
        {
            break;
        }
    }

    if(ii == 8) /* not found in the table */
    {
        if(iiFirstFree == 0xFF)/*not found empty entry*/
        {
            return GT_FULL;
        }

        ii = iiFirstFree;
    }

    if(ii >= 8)
    {
        return GT_BAD_STATE;
    }

    *profilePtr = ii;

    sip5Emulate_tpidBmpTable_used[tag][ii] = 1;
    sip5Emulate_tpidBmpTable[tag][ii] = tpidBmp;

    return GT_OK;
}

static GT_STATUS sip5Emulate_cpssDxChBrgVlanPortIngressTpidSet
(
    IN  GT_U8                devNum,
    IN  GT_U32                portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN  GT_U32               tpidBmp
)
{
    GT_STATUS   rc;
    GT_U32  profile;

    if(TGF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_FALSE)
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }

    /* look for the tpidBmp in the table */
    rc = sip5Emulate_IngressTpidProfileGet(ethMode,tpidBmp,&profile);
    if(rc != GT_OK)
    {
       return rc;
    }

    rc = cpssDxChBrgVlanIngressTpidProfileSet(devNum,profile,ethMode,tpidBmp);
    if(rc != GT_OK)
    {
       return rc;
    }

    /* write to pre-tti-lookup-ingress-eport table */
    rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum,
                portNum,
                ethMode,
                GT_TRUE,
                profile);
    if(rc != GT_OK)
    {
       return rc;
    }

   /* write to post-tti-lookup-ingress-eport table */
    rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum,
                portNum,
                ethMode,
                GT_FALSE,
                profile);

    return rc;
}

static GT_STATUS sip5Emulate_cpssDxChBrgVlanPortIngressTpidGet
(
    IN  GT_U8                devNum,
    IN  GT_U32                portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    OUT GT_U32               *tpidBmpPtr
)
{
    GT_STATUS   rc;
    GT_U32  profile;

    if(TGF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_FALSE)
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }

    /* read from pre-tti-lookup-ingress-eport table */
    rc = cpssDxChBrgVlanPortIngressTpidProfileGet(devNum,
                portNum,
                ethMode,
                GT_TRUE,/* default value */
                &profile);
    if(rc != GT_OK)
    {
       return rc;
    }


    rc = cpssDxChBrgVlanIngressTpidProfileGet(devNum,profile,ethMode,tpidBmpPtr);

    return rc;
}
#endif /*CHX_FAMILY*/


/**
* @internal prvTgfBrgVlanPortIngressTpidSet function
* @endinternal
*
* @brief   Function sets bitmap of TPID (Tag Protocol ID) table indexes per
*         ingress port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ethMode                  - TAG0/TAG1 selector
* @param[in] tpidBmp                  - bitmap represent entries in the TPID Table
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - tpidBmp initialized with more then 8 bits
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortIngressTpidSet
(
    IN  GT_U8                devNum,
    IN  GT_U32                portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN  GT_U32               tpidBmp
)
{
#ifdef CHX_FAMILY
    if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PX_FAMILY_PIPE_E)
    {
        GT_STATUS   rc;
        GT_U32  ii;
        PRV_TGF_MEMBER_FORCE_INFO_STC *currMemPtr;

       /* PRV_TGF_LOG4_MAC("prvTgfBrgVlanPortIngressTpidSet(%d,%d,%d,%d) \n",
                         devNum, portNum, ethMode, tpidBmp); */


        /* add the classification for all cascade/ring ports , with full bmp of ethertype 0,1 */

        /* look for this member in the DB */
        currMemPtr = &prvTgfDevPortForceArray[0];
        for (ii = 0; ii < prvTgfDevPortForceNum; ii++, currMemPtr++)
        {
            if (devNum != currMemPtr->member.devNum)
            {
                continue;
            }

            if (currMemPtr->forceToVlan == GT_FALSE)
            {
                /* member not need to be forced to any vlan */
                continue;
            }

            if (currMemPtr->member.portNum >= CPSS_MAX_PORTS_NUM_CNS)
            {
                return GT_BAD_PARAM;
            }

            /* set the member with full bmp for the ethertype */
            rc = cpssDxChBrgVlanPortIngressTpidSet(currMemPtr->member.devNum,
                                                   currMemPtr->member.portNum,
                                                   ethMode, 0x000000FF);
            if(rc != GT_OK)
            {
                PRV_TGF_LOG1_MAC("[TGF]: cpssDxChBrgVlanPortIngressTpidSet FAILED, rc = [%d]", rc);
                return rc;
            }
        }

        if (TGF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            /* emulate for SIP5*/
            rc = sip5Emulate_cpssDxChBrgVlanPortIngressTpidSet(devNum, portNum, ethMode, tpidBmp);
            if(rc != GT_OK)
            {
                PRV_TGF_LOG1_MAC("[TGF]: sip5Emulate_cpssDxChBrgVlanPortIngressTpidSet FAILED, rc = [%d]", rc);
            }
            return rc;
        }


        /* call device specific API */
        return cpssDxChBrgVlanPortIngressTpidSet(devNum, portNum, ethMode, tpidBmp);
    }
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    devNum  = devNum ;
    portNum = portNum;
    ethMode = ethMode;
    tpidBmp = tpidBmp;
    return GT_BAD_STATE;
#endif
    return GT_OK;
}

/**
* @internal prvTgfBrgVlanPortIngressTpidGet function
* @endinternal
*
* @brief   Function gets bitmap of TPID (Tag Protocol ID) table indexes per
*         ingress port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ethMode                  - TAG0/TAG1 selector
*
* @param[out] tpidBmpPtr               - (pointer to) bitmap represent 8 entries at the TPID Table
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - tpidBmp initialized with more then 8 bits
* @retval GT_BAD_PTR               - tpidBmpPtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortIngressTpidGet
(
    IN  GT_U8                devNum,
    IN  GT_U32                portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    OUT GT_U32              *tpidBmpPtr
)
{
#ifdef CHX_FAMILY
    if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PX_FAMILY_PIPE_E)
    {
        GT_STATUS rc;
        if (TGF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            /* emulate for SIP5*/
            rc = sip5Emulate_cpssDxChBrgVlanPortIngressTpidGet(devNum, portNum, ethMode, tpidBmpPtr);
            if(rc != GT_OK)
            {
                PRV_TGF_LOG1_MAC("[TGF]: sip5Emulate_cpssDxChBrgVlanPortIngressTpidGet FAILED, rc = [%d]", rc);
            }
            return rc;
        }
        /* call device specific API */
        rc = cpssDxChBrgVlanPortIngressTpidGet(devNum, portNum, ethMode, tpidBmpPtr);
        if(rc != GT_OK)
        {
            PRV_TGF_LOG1_MAC("[TGF]: cpssDxChBrgVlanPortIngressTpidGet FAILED, rc = [%d]", rc);
        }
        return rc;
    }
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    devNum  = devNum ;
    portNum = portNum;
    ethMode = ethMode;
    tpidBmpPtr = tpidBmpPtr;
    return GT_BAD_STATE;
#endif
    return GT_OK;
}
#ifdef CHX_FAMILY
GT_STATUS prvCpssDxChPortCgMacRxEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state
);

GT_STATUS prvCpssDxChPortMtiMacRxEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state
);

GT_STATUS prvCpssDxChPortCgMacRxEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL   *statePtr
);

GT_STATUS prvCpssDxChPortMtiMacRxEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL   *statePtr
);

/**
* @internal tgfTrafficGeneratorIsCgPortForceLinkUpWaReq function
* @endinternal
*
* @brief   Part of WA for force link UP in CG ports.
*         Function checks if WA needed for port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @retval GT_TRUE                  - WA is needed
* @retval GT_FALSE                 - WA is not needed
*/
GT_BOOL tgfTrafficGeneratorIsCgPortForceLinkUpWaReq
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum
)
{
#ifdef ASIC_SIMULATION
    /* simulation does not support and does not need the WA*/
    return GT_FALSE;
#else

    GT_STATUS rc;
    CPSS_PORT_MAC_TYPE_ENT   macType;
    rc = cpssDxChPortMacTypeGet(devNum, portNum, &macType);
    if(rc != GT_OK)
    {
        return GT_FALSE;
    }
    /* the WA is needed for port in CG mode */
    return (macType == CPSS_PORT_MAC_TYPE_CG_E) ? GT_TRUE : GT_FALSE;
#endif
}

/**
* @internal tgfTrafficGeneratorIsMtiPortForceLinkUpWaReq function
* @endinternal
*
* @brief   Part of WA for force link UP in MTI ports.
*         Function checks if WA needed for port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @retval GT_TRUE                  - WA is needed
* @retval GT_FALSE                 - WA is not needed
*/
GT_BOOL tgfTrafficGeneratorIsMtiPortForceLinkUpWaReq
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum
)
{
    GT_STATUS rc;
    CPSS_PORT_MAC_TYPE_ENT   macType;
    rc = cpssDxChPortMacTypeGet(devNum, portNum, &macType);
    if(rc != GT_OK)
    {
        return GT_FALSE;
    }
    /* the WA is needed for port in MTI mode */
    return ((macType == CPSS_PORT_MAC_TYPE_MTI_64_E) || (macType == CPSS_PORT_MAC_TYPE_MTI_400_E)) ? GT_TRUE : GT_FALSE;
}


/* DB for CG Force Link WA.
   This is bitmap of port each bit means loopback status of a port:
   1 - test set port in MAC/PCS loopback
   0 - test do not set port in MAC/PCS loopback */
static CPSS_PORTS_BMP_STC  portIsInLoopback;

/**
* @internal tgfTrafficGeneratorPortForceLinkCheckWa function
* @endinternal
*
* @brief   WA to check force link UP state for CG/MTI ports.
*         Function checks Loopback and RX and TX enable/disable to get CG/MTI MAC force link status.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[out] enablePtr               -  (pointer to) port force link state
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameteres
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS tgfTrafficGeneratorPortForceLinkCheckWa
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                *enablePtr
)
{
    GT_STATUS   rc;
    GT_BOOL     loopbackEnable;
    GT_BOOL     rxEnable;
    GT_BOOL     portEnable;

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(tgfTrafficGeneratorIsCgPortForceLinkUpWaReq(devNum, portNum))
    {
        rc = cpssDxChPortInternalLoopbackEnableGet(devNum, portNum, &loopbackEnable);
        if(rc != GT_OK)
        {
            PRV_TGF_LOG1_MAC("FAIL : cpssDxChPortInternalLoopbackEnableGet - port[%d]\n",
                             portNum);
            return rc;
        }

        rc = prvCpssDxChPortCgMacRxEnableGet(devNum, portNum, &rxEnable);
        if(rc != GT_OK)
        {
            PRV_TGF_LOG1_MAC("FAIL : prvCpssDxChPortCgMacRxEnableGet - port[%d]\n",
                             portNum);
            return rc;
        }
    }
    else
    if(tgfTrafficGeneratorIsMtiPortForceLinkUpWaReq(devNum, portNum))
    {
        rc = cpssDxChPortInternalLoopbackEnableGet(devNum, portNum, &loopbackEnable);
        if(rc != GT_OK)
        {
            PRV_TGF_LOG1_MAC("FAIL : cpssDxChPortInternalLoopbackEnableGet - port[%d]\n",
                             portNum);
            return rc;
        }

        rc = prvCpssDxChPortMtiMacRxEnableGet(devNum, portNum, &rxEnable);
        if(rc != GT_OK)
        {
            PRV_TGF_LOG1_MAC("FAIL : prvCpssDxChPortMtiMacRxEnableGet - port[%d]\n",
                             portNum);
            return rc;
        }
    }
    else
    {
        /* No WA needed */
        *enablePtr = GT_FALSE;
        return GT_OK;
    }

    rc = prvCpssDxChPortEnableGet(devNum, portNum, &portEnable);
    if(rc != GT_OK)
    {
        PRV_TGF_LOG1_MAC("FAIL : prvCpssDxChPortEnableGet - port[%d]\n",
                         portNum);
        return rc;
    }

    *enablePtr = (loopbackEnable == GT_TRUE && rxEnable == GT_FALSE && portEnable == GT_TRUE);

    return GT_OK;
}

/**
* @internal tgfTrafficGeneratorPortForceLinkWa function
* @endinternal
*
* @brief   WA for force link UP in CG ports.
*         Function manages PCS Loopback and RX enable/disable to get CG MAC working.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   -  / disable force link UP
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
*/
GT_STATUS tgfTrafficGeneratorPortForceLinkWa
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    if(tgfTrafficGeneratorIsCgPortForceLinkUpWaReq(devNum, portNum))
    {
        GT_STATUS   rc;
        GT_BOOL     rxEnable;

        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portIsInLoopback, portNum) == 0)
        {
            rc = cpssDxChPortInternalLoopbackEnableSet(devNum, portNum, enable);
            if(rc != GT_OK)
            {
                PRV_TGF_LOG1_MAC("FAIL : cpssDxChPortInternalLoopbackEnableSet - port[%d]\n",
                                 portNum);
                return rc;
            }

            rxEnable = (enable) ? GT_FALSE : GT_TRUE;
            rc = prvCpssDxChPortCgMacRxEnableSet(devNum, portNum, rxEnable);
            if(rc != GT_OK)
            {
                PRV_TGF_LOG1_MAC("FAIL : prvCpssDxChPortCgMacRxEnableSet - port[%d]\n",
                                 portNum);
                return rc;
            }
        }
    }

    if(tgfTrafficGeneratorIsMtiPortForceLinkUpWaReq(devNum, portNum))
    {
        GT_STATUS   rc;
        GT_BOOL     rxEnable;

        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portIsInLoopback, portNum) == 0)
        {
            rc = cpssDxChPortInternalLoopbackEnableSet(devNum, portNum, enable);
            if(rc != GT_OK)
            {
                PRV_TGF_LOG1_MAC("FAIL : cpssDxChPortInternalLoopbackEnableSet - port[%d]\n",
                                 portNum);
                return rc;
            }

            rxEnable = (enable) ? GT_FALSE : GT_TRUE;
            rc = prvCpssDxChPortMtiMacRxEnableSet(devNum, portNum, rxEnable);
            if(rc != GT_OK)
            {
                PRV_TGF_LOG1_MAC("FAIL : prvCpssDxChPortMtiMacRxEnableSet - port[%d]\n",
                                 portNum);
                return rc;
            }
        }
    }

    return GT_OK;
}

#endif
/**
* @internal tgfTrafficGeneratorPortLoopbackModeEnableSet function
* @endinternal
*
* @brief   Set port in 'loopback' mode
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] enable                   -  / disable (loopback/no loopback)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS tgfTrafficGeneratorPortLoopbackModeEnableSet
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN GT_BOOL                   enable
)
{
    CPSS_NULL_PTR_CHECK_MAC(portInterfacePtr);
    TGF_PARAM_NOT_USED(enable);

    if(portInterfacePtr->type != CPSS_INTERFACE_PORT_E)
    {
        return GT_BAD_PARAM;
    }

#ifdef DXCH_CODE
    if(PRV_CPSS_PP_MAC(portInterfacePtr->devPort.hwDevNum)->devFamily !=
       CPSS_PX_FAMILY_PIPE_E)
    {
        GT_STATUS   rc;
        GT_U32      tpidBmp = (enable == GT_TRUE) ? 0x3f : 0xff;
        GT_U8       devNum;

        devNum = (GT_U8)portInterfacePtr->devPort.hwDevNum;

#ifdef GM_USED
        /* for GM - BOBCAT2 - external loopback is supported by simulation help
           since there is no GOP unit in GM */
        if (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
        {
            /* WA for the GM devices that not support internal mac loopback port.     */
            simulateExternalLoopbackPortSet(devNum,
                                        portInterfacePtr->devPort.portNum,
                                        enable);
            return GT_OK;
        }
#endif
        /* Set ingress TPID select for TAG0 --> set to use only 6 TPID,
           to not recognize TPIDs that used only for egress between cascade/ring ports.
           this setting needed when :
           1. we capture traffic to the CPU.
           2. the port send traffic 'in loopback' and at the ingress it should
                not be recognize ethertypes that relate to 'egress'
        */

         /* need to fix test to fit to portNum of type GT_PORT_NUM and devNum
               of type GT_HW_DEV_NUM
               Need to fix call to prvTgfCommonIsDeviceForce that gets GT_U8 for devNum
               instead of GT_HW_DEV_NUM
               Need to remove all casting when accomodating test to lion2*/
            CPSS_TBD_BOOKMARK_EARCH

        if(GT_TRUE == prvTgfCommonIsDeviceForce(devNum))
        {
            rc = prvTgfBrgVlanPortIngressTpidSet(devNum,
                                                 portInterfacePtr->devPort.portNum,
                                                 CPSS_VLAN_ETHERTYPE0_E,
                                                 tpidBmp);
            if(rc != GT_OK)
            {
                return rc;
            }

            rc = prvTgfBrgVlanPortIngressTpidSet(devNum,
                                                 portInterfacePtr->devPort.portNum,
                                                 CPSS_VLAN_ETHERTYPE1_E,
                                                 tpidBmp);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        switch(prvTgfLoopbackUnit)
        {
            case PRV_TGF_LOOPBACK_UNIT_MAC_E:
                /* Lion2 and Hooper has erratum FE-2102944 - The unwanted packet
                   is received and transmitted by the device after MAC reset.
                   cpssDxChPortInternalLoopbackEnableSet use MAC reset. So tests
                   should use cpssDxChPortPcsLoopbackModeSet to avoid redundand packets. */
                if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
                {
                    return cpssDxChPortPcsLoopbackModeSet(devNum,
                                                           portInterfacePtr->devPort.portNum,
                                                           enable ? CPSS_DXCH_PORT_PCS_LOOPBACK_TX2RX_E :
                                                                    CPSS_DXCH_PORT_PCS_LOOPBACK_DISABLE_E);
                }
                else
                {
                    if(tgfTrafficGeneratorIsCgPortForceLinkUpWaReq(devNum, portInterfacePtr->devPort.portNum))
                    {
                        if (enable)
                        {
                            CPSS_PORTS_BMP_PORT_SET_MAC(&portIsInLoopback, portInterfacePtr->devPort.portNum);
                        }
                        else
                        {
                            CPSS_PORTS_BMP_PORT_CLEAR_MAC(&portIsInLoopback, portInterfacePtr->devPort.portNum);
                        }

                        return prvCpssDxChPortCgMacRxEnableSet(devNum,portInterfacePtr->devPort.portNum,enable);
                    }
                    else if(tgfTrafficGeneratorIsMtiPortForceLinkUpWaReq(devNum, portInterfacePtr->devPort.portNum))
                    {
                        if (enable)
                        {
                            CPSS_PORTS_BMP_PORT_SET_MAC(&portIsInLoopback, portInterfacePtr->devPort.portNum);
                        }
                        else
                        {
                            CPSS_PORTS_BMP_PORT_CLEAR_MAC(&portIsInLoopback, portInterfacePtr->devPort.portNum);
                        }

                        return prvCpssDxChPortMtiMacRxEnableSet(devNum,portInterfacePtr->devPort.portNum,enable);
                    }
                    else
                    {
                        return cpssDxChPortInternalLoopbackEnableSet(devNum,
                                                                     portInterfacePtr->devPort.portNum,
                                                                     enable);
                    }
                }

            case PRV_TGF_LOOPBACK_UNIT_SERDES_E:
                return cpssDxChPortSerdesLoopbackModeSet(devNum,
                                                         portInterfacePtr->devPort.portNum,
                                                         0xFF, /* exact bitmap not important here */
                                                         CPSS_DXCH_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E);
            default:
                return GT_NOT_SUPPORTED;
        }
    }
#endif /*DXCH_CODE*/
#ifdef PX_CODE
        if(PRV_CPSS_PP_MAC(portInterfacePtr->devPort.hwDevNum)->devFamily ==
           CPSS_PX_FAMILY_PIPE_E)
        {
        GT_SW_DEV_NUM devNum;

        devNum = (GT_SW_DEV_NUM)portInterfacePtr->devPort.hwDevNum;
        switch(prvTgfLoopbackUnit)
        {
            case PRV_TGF_LOOPBACK_UNIT_MAC_E:

                return cpssPxPortInternalLoopbackEnableSet(devNum,
                    portInterfacePtr->devPort.portNum, enable);

            case PRV_TGF_LOOPBACK_UNIT_SERDES_E:
                return cpssPxPortSerdesLoopbackModeSet(devNum,
                    portInterfacePtr->devPort.portNum,
                    CPSS_PX_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E);
            default:
                return GT_NOT_SUPPORTED;
        }
    }
#endif /*PX_CODE*/

#if !(defined CHX_FAMILY) && !(defined PX_FAMILY)
    /* for now other families support just loopback in MAC */
    CPSS_COVERITY_NON_ISSUE_BOOKMARK
    /* coverity[unreachable] */
    if(prvTgfLoopbackUnit != PRV_TGF_LOOPBACK_UNIT_MAC_E)
        return GT_NOT_SUPPORTED;
    return GT_BAD_STATE;
#endif /*!(defined CHX_FAMILY) && !(defined PX_FAMILY)*/
    return GT_OK;
}


/**
* @internal prvTgfCommonMemberForceInfoSet function
* @endinternal
*
* @brief   add member info to DB of 'members to force to vlan/mc groups'
*         in case that the member already exists (dev,port) the other parameters
*         are 'updated' according to last setting.
* @param[in] memberInfoPtr            - (pointer to) member info
*
* @retval GT_OK                    - on success
* @retval GT_FULL                  - the DB is full and no more members can be added -->
*                                       need to make DB larger.
*/
GT_STATUS prvTgfCommonMemberForceInfoSet(
    IN PRV_TGF_MEMBER_FORCE_INFO_STC    *memberInfoPtr
)
{
    GT_U32  ii;
    PRV_TGF_MEMBER_FORCE_INFO_STC *currMemPtr;

    /* look for this member in the DB */
    currMemPtr = &prvTgfDevPortForceArray[0];
    for(ii = 0 ; ii < prvTgfDevPortForceNum; ii++,currMemPtr++)
    {
        if(memberInfoPtr->member.devNum == currMemPtr->member.devNum &&
           memberInfoPtr->member.portNum == currMemPtr->member.portNum )
        {
            break;
        }
    }

    if(ii == prvTgfDevPortForceNum)
    {
        /* new member */

        if(prvTgfDevPortForceNum >= PRV_TGF_FORCE_MEMBERS_COUNT_CNS)
        {
            /* the table already full , can't add new member */
            return GT_FULL;
        }

        prvTgfDevPortForceNum++;
    }

    /* copy the member into the DB */
    *currMemPtr = *memberInfoPtr;


    return GT_OK;
}

/**
* @internal prvTgfCommonIsDeviceForce function
* @endinternal
*
* @brief   check if the device has ports that used with force configuration
*
* @retval GT_TRUE                  - the device force configuration
* @retval GT_FALSE                 - the device NOT force configuration
*/
GT_BOOL prvTgfCommonIsDeviceForce(
    IN GT_U8    devNum
)
{
    GT_U32  ii;
    PRV_TGF_MEMBER_FORCE_INFO_STC *currMemPtr;

    /* look for this member in the DB */
    currMemPtr = &prvTgfDevPortForceArray[0];
    for(ii = 0 ; ii < prvTgfDevPortForceNum; ii++,currMemPtr++)
    {
        if(devNum == currMemPtr->member.devNum)
        {
            return GT_TRUE;
        }
    }

    return GT_FALSE;
}

static GT_U32 onEmulator_sleepTime = 0;
GT_STATUS tgf_cpssDeviceRunCheck_onEmulator_sleepTime_Set(GT_U32    sleepTime)
{
    onEmulator_sleepTime = sleepTime;
    return GT_OK;
}
GT_U32 tgf_cpssDeviceRunCheck_onEmulator_sleepTime_Get(void)
{
    return onEmulator_sleepTime;
}

/**
* @internal tgfTrafficGeneratorPortForceLinkUpEnableSet function
* @endinternal
*
* @brief   Set port in 'force linkup' mode
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] enable                   -  / disable (force/not force)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS tgfTrafficGeneratorPortForceLinkUpEnableSet
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN GT_BOOL                   enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc;
    GT_BOOL linkUp = GT_FALSE;
    CPSS_PORT_SPEED_ENT  forceSpeed,newSpeed;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode,newIfMode;
    GT_U32  sleepTime;
    GT_BOOL forceMtiWaNeeded;
#endif

    CPSS_NULL_PTR_CHECK_MAC(portInterfacePtr);
    TGF_PARAM_NOT_USED(enable);

    if(portInterfacePtr->type != CPSS_INTERFACE_PORT_E)
    {
        return GT_BAD_PARAM;
    }

#ifdef CHX_FAMILY
    if (PRV_CPSS_PP_MAC((GT_U8)portInterfacePtr->devPort.hwDevNum)->devFamily != CPSS_PX_FAMILY_PIPE_E)
    {
        devNum = (GT_U8)portInterfacePtr->devPort.hwDevNum;
        portNum = (GT_PHYSICAL_PORT_NUM)portInterfacePtr->devPort.portNum;
        /* Indicate MTI WA is needed */
        forceMtiWaNeeded = tgfTrafficGeneratorIsMtiPortForceLinkUpWaReq(devNum, portNum);
        if (enable == GT_TRUE)
        {
            if (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
            {
                /* currently it seems that the BC2 not need any extra config beside the 'force link up' */
#ifdef GM_USED
                /* the GM not hold logic to set linkup from the 'MAC register' so we explicitly advice it */
                simulatePortForceLinkUpSet((GT_U8)portInterfacePtr->devPort.hwDevNum,
                                           portInterfacePtr->devPort.portNum,
                                           enable);
#endif
            }

            /* get link status */
            rc = cpssDxChPortLinkStatusGet(devNum, portNum, &linkUp);
            if (rc != GT_OK)
            {
                return rc;
            }


            if (linkUp == GT_FALSE)
            {
                /* link is down. Set port speed to be maximal
                according to interface mode.
                there is no 'other side' that the port can be 'auto negotiation'
                with. The speed and duplex should be forced too. */
                if (enable == GT_TRUE)
                {
                    /* get interface mode */
                    rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }

                    /* do not change speed for other then in following switch case
                    interfaces.  port is already configured for proper speed. */
                    forceSpeed = CPSS_PORT_SPEED_NA_E;

                    switch (ifMode)
                    {
                    case CPSS_PORT_INTERFACE_MODE_REDUCED_GMII_E:
                    case CPSS_PORT_INTERFACE_MODE_MII_E:
                    case CPSS_PORT_INTERFACE_MODE_SGMII_E:
                    case CPSS_PORT_INTERFACE_MODE_GMII_E:
                    case CPSS_PORT_INTERFACE_MODE_MII_PHY_E:
                    case CPSS_PORT_INTERFACE_MODE_100BASE_FX_E:
                    case CPSS_PORT_INTERFACE_MODE_QSGMII_E:
                        /* force speed to be common acceptable 100M */
                        forceSpeed = CPSS_PORT_SPEED_100_E;
                        if (prvCpssDxChPortRemotePortCheck(devNum, portNum))
                        {
                            /* 88E1690 ports work only in 1G mode in loopback */
                            forceSpeed = CPSS_PORT_SPEED_1000_E;
                        }

                        break;
                    case CPSS_PORT_INTERFACE_MODE_XGMII_E:
                        /* NOTE: this section was added to support board of xcat-FE , with 4 XG ports.
                            the trunk tests uses port 27 , and without next configuration the
                            BM (buffer management) in the device was stuck */
                        if (0 == PRV_CPSS_SIP_5_CHECK_MAC(devNum))
                        {
                            rc = cpssDxChPortInterfaceModeSet(devNum, portNum, ifMode);
                            if (rc != GT_OK)
                            {
                                PRV_TGF_LOG2_MAC("tgfTrafficGeneratorPortForceLinkUpEnableSet: "
                                                 "FAIL : cpssDxChPortInterfaceModeSet port[%d],ifMode[%d]\n",
                                                 portNum, ifMode);
                                return rc;
                            }
                            /* set speed to be 10G */
                            rc = cpssDxChPortSpeedSet(devNum, portNum, CPSS_PORT_SPEED_10000_E);
                            if (rc != GT_OK)
                            {
                                PRV_TGF_LOG1_MAC("FAIL : cpssDxChPortSpeedSet port[%d]\n",
                                                 portNum);
                                return rc;
                            }
                            /* SERDES, XPCS, MAC configuration */
                            rc = cpssDxChPortSerdesPowerStatusSet(devNum, portNum,
                                                                  CPSS_PORT_DIRECTION_BOTH_E,
                                                                  0xF, GT_TRUE);
                            if (rc != GT_OK)
                            {
                                PRV_TGF_LOG1_MAC("FAIL : cpssDxChPortSerdesPowerStatusSet port[%d]\n",
                                                 portNum);
                                return rc;
                            }
                        }
                        else
                        {
                            /* the device with such interface mode can keep it with out change */
                        }

                        break;

                    case CPSS_PORT_INTERFACE_MODE_NA_E:

                        newIfMode = CPSS_PORT_INTERFACE_MODE_NA_E; /* 'non init' value */
                        newSpeed  = CPSS_PORT_SPEED_10000_E;

                        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E ||
                            PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
                        {
                            if (portNum & 1)
                            {
                                /* on Lion/Lion2 only odd ports supposed to be used as loopback */
                                newIfMode = CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E;
                                newSpeed  = CPSS_PORT_SPEED_10000_E;
                            }
                        }
                        else
                        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E ||
                                   PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E ||
                                   PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E ||
                                   PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E)
                        {
                            /* all ports should support 10G */
                            newIfMode = CPSS_PORT_INTERFACE_MODE_KR_E;
                            newSpeed  = CPSS_PORT_SPEED_10000_E;
                        }
                        else
                        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
                        {
                            newIfMode = CPSS_PORT_INTERFACE_MODE_KR_E;
                            newSpeed  = CPSS_PORT_SPEED_50000_E;
                        }
                        else
                        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
                        {
#ifndef ASIC_SIMULATION
                            /* ensure at least two ports in 10G mode.
                            Ports prvTgfPortsArray[3] and 50 are used in NetIf WS test */
                            if ((portNum == prvTgfPortsArray[3]) || (portNum == 80) || (portNum == 50) || (portNum == 58))
                            {
                                /* port #3 should be at least 10G */
                                newIfMode = CPSS_PORT_INTERFACE_MODE_KR_E;
                                newSpeed  = CPSS_PORT_SPEED_10000_E;
                            }
                            else
#endif
                            {
                                /* all ports should support SGMII Giga mode .. so try to set it */
                                newIfMode = CPSS_PORT_INTERFACE_MODE_SGMII_E;
                                newSpeed  = CPSS_PORT_SPEED_1000_E;

                                switch (PRV_CPSS_PP_MAC(devNum)->devType)
                                {
                                case CPSS_BOBK_CETUS_DEVICES_CASES_MAC:
                                    /* Cetus use 1G ports. */
                                    forceSpeed = CPSS_PORT_SPEED_1000_E; break;
                                default:
                                    break;
                                }
                            }
                        }

                        if (newIfMode != CPSS_PORT_INTERFACE_MODE_NA_E)
                        {
                            CPSS_PORTS_BMP_STC portsBmp;

                            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
                            CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portNum);
                            rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE,
                                                          newIfMode,
                                                          newSpeed);
                            if (rc != GT_OK)
                            {
                                PRV_TGF_LOG3_MAC("tgfTrafficGeneratorPortForceLinkUpEnableSet FAIL : cpssDxChPortModeSpeedSet port[%d],newIfMode[%d],newSpeed[%d]\n",
                                                 portNum, newIfMode, newSpeed);
                                return rc;
                            }
                        }
                        break;
                    default:
                        break;
                    }

                    if (forceSpeed != CPSS_PORT_SPEED_NA_E)
                    {
                        /* set port speed */
                        rc = cpssDxChPortSpeedSet(devNum, portNum, forceSpeed);
                        if (rc != GT_OK)
                        {
                            PRV_TGF_LOG1_MAC("FAIL : cpssDxChPortSpeedSet - port[%d]\n",
                                             portNum);
                            return rc;
                        }

                        /* set the port to not be 'auto negotiation' */
                        rc = cpssDxChPortSpeedAutoNegEnableSet(devNum, portNum, GT_FALSE);
                        if (rc != GT_OK)
                        {
                            PRV_TGF_LOG1_MAC("FAIL : cpssDxChPortSpeedAutoNegEnableSet - port[%d]\n",
                                             portNum);
                            return rc;
                        }

                        /* set the port to not be 'auto negotiation' */
                        rc = cpssDxChPortDuplexAutoNegEnableSet(devNum, portNum, GT_FALSE);
                        if (rc != GT_OK)
                        {
                            PRV_TGF_LOG1_MAC("FAIL : cpssDxChPortDuplexAutoNegEnableSet - port[%d]\n",
                                             portNum);
                            return rc;
                        }

                        /* set port to be full duplex */
                        rc = cpssDxChPortDuplexModeSet(devNum, portNum, CPSS_PORT_FULL_DUPLEX_E);
                        if (rc != GT_OK)
                        {
                            PRV_TGF_LOG1_MAC("FAIL : cpssDxChPortDuplexModeSet - port[%d]\n",
                                             portNum);
                            return rc;
                        }
                    }
                }
            }
        } /*if force link up enabled */

        if (enable == GT_TRUE && linkUp == GT_FALSE && forceMtiWaNeeded == GT_FALSE)
        {
            /* disable 'force link Down' (if exists) */
            rc = cpssDxChPortForceLinkDownEnableSet(devNum, portNum, GT_FALSE);
            if (rc != GT_OK)
            {
                PRV_TGF_LOG1_MAC("FAIL : cpssDxChPortForceLinkDownEnableSet (disable) - port[%d]\n",
                                 portNum);
                return rc;
            }
        }

        if (forceMtiWaNeeded == GT_FALSE)
        {
            rc = cpssDxChPortForceLinkPassEnableSet(devNum, portNum, enable);
            if (rc != GT_OK)
            {
                PRV_TGF_LOG1_MAC("FAIL : cpssDxChPortForceLinkPassEnableSet - port[%d]\n",
                                 portNum);
                return rc;
            }
        }

        rc = tgfTrafficGeneratorPortForceLinkWa(devNum, portNum, enable);
        if (rc != GT_OK)
        {
            PRV_TGF_LOG1_MAC("FAIL : tgfTrafficGeneratorPortForceLinkWa - port[%d]\n",
                             portNum);
            return rc;
        }

        if (prvCpssDxChPortRemotePortCheck(devNum, portNum) ||
            PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portLinkStatusState;

            portLinkStatusState =  enable ?
                CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E :
                CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;

            if (enable == GT_TRUE)
            {
                GT_BOOL     portEnabled;

                rc = cpssDxChPortEnableGet(devNum, portNum, &portEnabled);
                /* if the caller set the MAC to be disabled ...
                we need to assume that the EGF filter should treat as 'link down' */
                if (rc == GT_OK && portEnabled == GT_FALSE)
                {
                    portLinkStatusState = CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;
                    PRV_TGF_LOG1_MAC("port[%d] : The port's MAC is disabled : setting EGF filter to be 'link down' \n",
                                     portNum);
                }
            }

            /* need to manage EGF filter for remote ports.
            SIP_5_20 devices use Link Change event for EGF management.
            Sometimes event task get CPU time too late and test start
            send packet before it. Need to manage EGF here too.  */
            rc = cpssDxChBrgEgrFltPortLinkEnableSet(devNum, portNum, portLinkStatusState);
            if (rc != GT_OK)
            {
                PRV_TGF_LOG1_MAC("FAIL : cpssDxChBrgEgrFltPortLinkEnableSet - port[%d]\n",
                                 portNum);
                return rc;
            }
        }

        if (cpssDeviceRunCheck_onEmulator())
        {
            sleepTime = tgf_cpssDeviceRunCheck_onEmulator_sleepTime_Get();
             }
            else
        {
            sleepTime = 0;
        }

#ifdef  ASIC_SIMULATION
        /*on WM we know that we need the 30 mills -- regardless to 'emulation mode'*/
        sleepTime = 30;
#endif /*ASIC_SIMULATION*/

        if (enable == GT_TRUE && linkUp == GT_FALSE && sleepTime)
        {
            cpssOsTimerWkAfter(sleepTime); /* allow the port fully 'get UP' before we send traffic to it */
        }
        return GT_OK;
    }

#endif /* CHX_FAMILY */


#ifdef PX_FAMILY
    return cpssPxPortForceLinkPassEnableSet((GT_SW_DEV_NUM)portInterfacePtr->devPort.hwDevNum,
                                           (GT_U8)portInterfacePtr->devPort.portNum,
                                           enable);
#endif /* PX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) && !(defined EX_FAMILY) && !(defined PX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) && !(defined EX_FAMILY) */

    return GT_OK;
}

/**
* @internal tgfTrafficGeneratorPortCountersEthReset function
* @endinternal
*
* @brief   Reset the traffic generator counters on the port
*
* @param[in] portInterfacePtr         - (pointer to) port interface
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS tgfTrafficGeneratorPortCountersEthReset
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr
)
{
    GT_STATUS   rc;

    CPSS_NULL_PTR_CHECK_MAC(portInterfacePtr);

    if(portInterfacePtr->type != CPSS_INTERFACE_PORT_E)
    {
        return GT_BAD_PARAM;
    }

#ifdef CHX_FAMILY
{
    GT_U8       devNum;
    devNum = (GT_U8)portInterfacePtr->devPort.hwDevNum;

    if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PX_FAMILY_PIPE_E)
    {
        CPSS_PORT_MAC_COUNTER_SET_STC   dummyCounters;

        /* avoid compiler warning */
        dummyCounters.goodOctetsRcv.l[0] = 0;

        rc = cpssDxChPortMacCountersOnPortGet((GT_U8)portInterfacePtr->devPort.hwDevNum,
                                                portInterfacePtr->devPort.portNum,
                                                &dummyCounters);
        if(rc != GT_OK)
        {
            PRV_TGF_LOG1_MAC("FAIL : cpssDxChPortMacCountersOnPortGet - port[%d]\n",
                             portInterfacePtr->devPort.portNum);
        }
        return rc;
    }
}
#endif /* CHX_FAMILY */

#ifdef PX_FAMILY
{
    CPSS_PX_PORT_MAC_COUNTERS_STC   dummyPxCounters;
    /*cpssExMxPmPortCapturedMacCountersGet*/
    rc = cpssPxPortMacCountersOnPortGet(portInterfacePtr->devPort.hwDevNum,
                                           portInterfacePtr->devPort.portNum,
                                           &dummyPxCounters);
    if(rc != GT_OK)
    {
        PRV_TGF_LOG1_MAC("FAIL : cpssPxPortMacCountersOnPortGet - port[%d]\n",
                         portInterfacePtr->devPort.portNum);
    }
    return rc;
}
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined PX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) && !(defined EX_FAMILY) */
    return GT_OK;
}

/**
* @internal tgfTrafficGeneratorPortCountersEthRead function
* @endinternal
*
* @brief   Read the traffic generator counters on the port
*
* @param[in] portInterfacePtr         - (pointer to) port interface
*
* @param[out] countersPtr              - (pointer to) the counters of port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS tgfTrafficGeneratorPortCountersEthRead
(
    IN  CPSS_INTERFACE_INFO_STC        *portInterfacePtr,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC  *countersPtr
)
{
    GT_STATUS   rc = GT_OK;

    CPSS_NULL_PTR_CHECK_MAC(portInterfacePtr);
    CPSS_NULL_PTR_CHECK_MAC(countersPtr);

    if(portInterfacePtr->type != CPSS_INTERFACE_PORT_E)
    {
        return GT_BAD_PARAM;
    }

#ifdef CHX_FAMILY
{
    GT_U8       devNum;
    devNum = (GT_U8)portInterfacePtr->devPort.hwDevNum;

    if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PX_FAMILY_PIPE_E)
    {
        rc = cpssDxChPortMacCountersOnPortGet((GT_U8)portInterfacePtr->devPort.hwDevNum,
                                              portInterfacePtr->devPort.portNum,
                                              countersPtr);
        if(rc != GT_OK)
        {
            PRV_TGF_LOG1_MAC("FAIL : cpssDxChPortMacCountersOnPortGet - port[%d]\n",
                             portInterfacePtr->devPort.portNum);
        }
    }
}
#endif /* CHX_FAMILY */


    if(countersPtr->goodPktsRcv.l[0] == 0)
    {
        countersPtr->goodPktsRcv.l[0] = countersPtr->ucPktsRcv.l[0] + countersPtr->mcPktsRcv.l[0] + countersPtr->brdcPktsRcv.l[0];
        countersPtr->goodPktsRcv.l[1] = countersPtr->ucPktsRcv.l[1] + countersPtr->mcPktsRcv.l[1] + countersPtr->brdcPktsRcv.l[1];
    }

    if(countersPtr->goodPktsSent.l[0] == 0)
    {
        countersPtr->goodPktsSent.l[0] = countersPtr->ucPktsSent.l[0] + countersPtr->mcPktsSent.l[0] + countersPtr->brdcPktsSent.l[0];
        countersPtr->goodPktsSent.l[1] = countersPtr->ucPktsSent.l[1] + countersPtr->mcPktsSent.l[1] + countersPtr->brdcPktsSent.l[1];
    }

    return rc;
}

GT_U32      trgEPortInsteadPhyPort = 0;
GT_BOOL     use_trgEPortInsteadPhyPort = GT_FALSE;

/**
* @internal tgfStateTrgEPortInsteadPhyPort function
* @endinternal
*
* @brief   Option to state that the cpu will send packets to trgEport that is not
*         the 'physical port' (portNum) that used by prvLuaTgfTransmitPackets(..portNum..)
*         prvLuaTgfTransmitPacketsWithCapture(..inPortNum..)
*         use enable = GT_TRUE to state to start using trgEPort
*         use enable = GT_FALSE to state to start NOT using
*         trgEPort (trgEPort is ignored). use the phyPort
* @param[in] trgEPort                 - egress device number
* @param[in] enable                   - (GT_BOOL) do we use
*                                       eport or the phy port
*                                       return_code
*/
GT_STATUS tgfStateTrgEPortInsteadPhyPort(
    IN GT_U32                trgEPort,
    IN GT_BOOL               enable
)
{
    use_trgEPortInsteadPhyPort  =  enable;
    trgEPortInsteadPhyPort      =  trgEPort;

    return GT_OK;
}


#ifdef CHX_FAMILY

/**
* @internal prvTgfDxChNetIfParamSet function
* @endinternal
*
* @brief   Fill in DxCh structure for send packet info
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] vpt                      - VPT of the packet
* @param[in] cfiBit                   - CFI bit
* @param[in] vid                      - VID of the packet
* @param[in] packetIsTagged           - packet is tagged flag
* @param[in] doForceDsa               - GT_FALSE - choose DSA tag automatically
*                                      GT_TRUE - use DSA tag defined by forcedDsa
* @param[in] forcedDsa                - DSA tag type, used when doForceDsa == GT_TRUE
*
* @param[out] dxChPcktParamsPtr        - pointer to DxCh structure for send packet info.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
*/
static GT_STATUS prvTgfDxChNetIfParamSet
(
    IN CPSS_INTERFACE_INFO_STC            *portInterfacePtr,
    IN GT_U32                              vpt,
    IN GT_U32                              cfiBit,
    IN GT_U32                              vid,
    IN GT_BOOL                             packetIsTagged,
    IN GT_BOOL                             doForceDsa,
    IN TGF_DSA_TYPE_ENT                    forcedDsa,
    OUT CPSS_DXCH_NET_TX_PARAMS_STC       *dxChPcktParamsPtr
)
{
    GT_U8                               devNum;
    GT_STATUS                           rc;
    CPSS_DXCH_NET_DSA_FROM_CPU_STC    *dxChfromCpuPtr    = NULL;
    CPSS_DXCH_NET_DSA_TYPE_ENT         dxChDsaTagType;

    devNum = (GT_U8)portInterfacePtr->devPort.hwDevNum;

    cpssOsMemSet(dxChPcktParamsPtr, 0, sizeof(*dxChPcktParamsPtr));

    switch (forcedDsa)
    {
        case TGF_DSA_1_WORD_TYPE_E: dxChDsaTagType = CPSS_DXCH_NET_DSA_1_WORD_TYPE_ENT; break;
        case TGF_DSA_2_WORD_TYPE_E: dxChDsaTagType = CPSS_DXCH_NET_DSA_2_WORD_TYPE_ENT; break;
        case TGF_DSA_3_WORD_TYPE_E: dxChDsaTagType = CPSS_DXCH_NET_DSA_3_WORD_TYPE_ENT; break;
        case TGF_DSA_4_WORD_TYPE_E: dxChDsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT; break;
        default: dxChDsaTagType = forcedDsa; break;
    }

    dxChPcktParamsPtr->cookie = NULL;
    dxChPcktParamsPtr->packetIsTagged = packetIsTagged;

    dxChPcktParamsPtr->sdmaInfo.recalcCrc = GT_TRUE;
    dxChPcktParamsPtr->sdmaInfo.txQueue   = txQueueNum;
    dxChPcktParamsPtr->sdmaInfo.evReqHndl = 0;
    dxChPcktParamsPtr->sdmaInfo.invokeTxBufferQueueEvent = GT_TRUE;

    if (doForceDsa == GT_FALSE)
    {
        dxChPcktParamsPtr->dsaParam.commonParams.dsaTagType     = CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E;
    }
    else
    {
        dxChPcktParamsPtr->dsaParam.commonParams.dsaTagType     = dxChDsaTagType;
    }

    dxChPcktParamsPtr->dsaParam.commonParams.vpt            = (GT_U8) vpt;
    dxChPcktParamsPtr->dsaParam.commonParams.cfiBit         = (GT_U8) cfiBit;
    dxChPcktParamsPtr->dsaParam.commonParams.vid            = (GT_U16) vid;
    dxChPcktParamsPtr->dsaParam.commonParams.dropOnSource   = GT_FALSE;
    dxChPcktParamsPtr->dsaParam.commonParams.packetIsLooped = GT_FALSE;

    dxChPcktParamsPtr->dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;

    dxChfromCpuPtr = &dxChPcktParamsPtr->dsaParam.dsaInfo.fromCpu;

    if(TGF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* support ability to send to phy port > 63 */
        if (doForceDsa == GT_FALSE)
        {
            dxChPcktParamsPtr->dsaParam.commonParams.dsaTagType     = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
        }

        dxChfromCpuPtr->isTrgPhyPortValid = GT_TRUE;
        if( CPSS_INTERFACE_PORT_E == portInterfacePtr->type)
        {
            dxChfromCpuPtr->dstEport = portInterfacePtr->devPort.portNum;
            if(use_trgEPortInsteadPhyPort == GT_TRUE)
            {
                dxChfromCpuPtr->dstEport = trgEPortInsteadPhyPort;
            }
#if 0
            if((use_trgEPortInsteadPhyPort == GT_TRUE) && (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfCpuDevNum)))
            {
                dxChfromCpuPtr->isTrgPhyPortValid = GT_FALSE;
            }
#endif
        }
    }

    dxChfromCpuPtr->dstInterface        = *portInterfacePtr;
    if( CPSS_INTERFACE_PORT_E == portInterfacePtr->type )
    {
        rc = extUtilHwDeviceNumberGet((portInterfacePtr->devPort.hwDevNum),
                                     &dxChfromCpuPtr->dstInterface.devPort.hwDevNum);
        if(GT_OK != rc)
            return rc;

        if(dxChfromCpuPtr->dstInterface.devPort.portNum >= 256)
        {
            GT_U32  value;
            if(PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfCpuDevNum) && appDemoDbEntryGet_func &&
                (appDemoDbEntryGet_func("doublePhysicalPorts", &value) == GT_OK) &&
                (value != 0))
            {
                dxChfromCpuPtr->isTrgPhyPortValid = GT_FALSE;
            }

            dxChfromCpuPtr->dstInterface.devPort.portNum &= 0xFF;
        }
    }
    dxChfromCpuPtr->tc                  = 7;
    dxChfromCpuPtr->dp                  = CPSS_DP_GREEN_E;
    dxChfromCpuPtr->egrFilterEn         = GT_FALSE;
    dxChfromCpuPtr->cascadeControl      = GT_TRUE;
    dxChfromCpuPtr->egrFilterRegistered = GT_FALSE;
    dxChfromCpuPtr->srcId               = 0;

    if(TGF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* value that in bypass bridge mode <Bypass Forwarding Decision only> */
        dxChfromCpuPtr->srcHwDev = SIP5_SRC_DEV_FROM_CPU;
    }
    else
    {
        rc = extUtilHwDeviceNumberGet(devNum, &dxChfromCpuPtr->srcHwDev);
        if(GT_OK != rc)
            return rc;
    }
    dxChfromCpuPtr->extDestInfo.devPort.dstIsTagged = dxChPcktParamsPtr->packetIsTagged;
    dxChfromCpuPtr->extDestInfo.devPort.mailBoxToNeighborCPU = GT_FALSE;

    return GT_OK;
}
#endif
/**
* @internal prvTgfTrafficGeneratorNetIfSyncTxPacketSend function
* @endinternal
*
* @brief   Transmit the traffic to the port
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] bufferPtr                - (pointer to) the buffer to send
* @param[in] bufferLength             - buffer length (include CRC bytes)
* @param[in] useMii                   - Use MII flag
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The packet will ingress to the PP after 'loopback' and will act as
*       it was received from a traffic generator connected to the port
*
*/
static GT_STATUS prvTgfTrafficGeneratorNetIfSyncTxPacketSend
(
    IN CPSS_INTERFACE_INFO_STC          *portInterfacePtr,
    IN GT_U8                            *bufferPtr,
    IN GT_U32                            bufferLength,
    IN GT_BOOL                           useMii
)
{
#ifdef CHX_FAMILY
        GT_U8   devNum    = 0;
        GT_U32  numOfBufs = 0;
        GT_U32  vpt       = 0;
        GT_U32  cfiBit    = 0;
        GT_U32  vid       = 0;
        GT_BOOL packetIsTagged = GT_FALSE;

        GT_STATUS   rc;
        CPSS_DXCH_NET_TX_PARAMS_STC       dxChPcktParams;

        GT_U8   *buffList[1];
        GT_U32  buffLenList[1];

        CPSS_NULL_PTR_CHECK_MAC(portInterfacePtr);
        CPSS_NULL_PTR_CHECK_MAC(bufferPtr);

        buffList[0] = bufferPtr;
        buffLenList[0] = bufferLength;

        numOfBufs = 1;
        devNum    = prvTgfCpuDevNum;
        if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PX_FAMILY_PIPE_E)
        {
            if ((ignore_etherTypeForVlanTag == GT_FALSE) &&
                (bufferPtr[12] == (GT_U8)(etherTypeForVlanTag >> 8)) &&
                (bufferPtr[13] == (GT_U8)(etherTypeForVlanTag & 0xFF)))
            {
                packetIsTagged = GT_TRUE;
                vpt    = (GT_U8)(bufferPtr[14] >> 5);
                cfiBit = (GT_U8)((bufferPtr[14] >> 4) & 1);
                vid    = ((bufferPtr[14] & 0xF) << 8) | bufferPtr[15];
            }
            else
            {
                packetIsTagged = GT_FALSE;
                vpt    = 0;
                cfiBit = 0;
                vid    = 4094;
            }

            /* In sip 6 diffrents port mode have diffrent number of vlan entries*/
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                GT_U32      numVlanEntries = 0;

                rc = cpssDxChCfgTableNumEntriesGet(devNum,CPSS_DXCH_CFG_TABLE_VLAN_E,&numVlanEntries);
                if (rc != GT_OK)
                {
                    return rc;
                }
                vid = (vid > numVlanEntries ? (numVlanEntries - 1) : vid);
            }

            /* fill in DXCH info */
            rc = prvTgfDxChNetIfParamSet(portInterfacePtr, vpt, cfiBit, vid,
                                         packetIsTagged, GT_FALSE, 0, &dxChPcktParams);
            if (rc != GT_OK)
            {
                return rc;
            }

            if ((useMii == GT_FALSE) || PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(devNum))
            {
                rc = cpssDxChNetIfSdmaSyncTxPacketSendPtr(devNum, &dxChPcktParams, buffList, buffLenList, numOfBufs);
            }
            else
            {
                rc = cpssDxChNetIfMiiSyncTxPacketSendPtr(devNum, &dxChPcktParams, buffList, buffLenList, numOfBufs);
            }
            return rc;
        }
#endif /* CHX_FAMILY */

#ifdef PX_CODE
{
    GT_STATUS   rc;
    GT_SW_DEV_NUM devNum = portInterfacePtr->devPort.hwDevNum;
    CPSS_PP_FAMILY_TYPE_ENT devFamily = PRV_CPSS_PP_MAC(devNum) ? PRV_CPSS_PP_MAC(devNum)->devFamily : 0;
    GT_U8   *buffList[1];
    GT_U32  buffLenList[1];
    GT_U32  numOfBufs = 1;

    CPSS_NULL_PTR_CHECK_MAC(portInterfacePtr);
    CPSS_NULL_PTR_CHECK_MAC(bufferPtr);

    buffList[0]    = bufferPtr;
    buffLenList[0] = bufferLength;

    if(CPSS_IS_PX_FAMILY_MAC(devFamily))
    {
        CPSS_PX_NET_TX_PARAMS_STC  pcktParams;

        pcktParams.txQueue = 7;
        pcktParams.recalcCrc = GT_TRUE;

        rc = cpssPxNetIfSdmaSyncTxPacketSend(devNum,&pcktParams,buffList,buffLenList,numOfBufs);
        if(rc != GT_OK)
        {
            cpssOsPrintf("[TGF]: cpssPxNetIfSdmaSyncTxPacketSend FAILED, rc = [%d]", rc);
            return rc;
        }

        return GT_OK;
    }
}
#endif /*PX_CODE*/

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) && !(defined EX_FAMILY)
    useMii = useMii;
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) && !(defined EX_FAMILY) */
    return GT_OK;
}

static GT_BOOL  generatorPoolInit = GT_FALSE;
/**
* @internal prvTgfTrafficGeneratorNetIfSyncTxBurstSend function
* @endinternal
*
* @brief   Transmit the traffic to the port
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] bufferPtr                - (pointer to) the buffer to send
* @param[in] bufferLength             - buffer length (include CRC bytes)
* @param[in] packetsNumber            - number of packets to send
* @param[in] doForceDsa               - GT_FALSE - choose DSA tag automatically
*                                      GT_TRUE - use DSA tag defined by forcedDsa
* @param[in] forcedDsa                - DSA tag type, used when doForceDsa == GT_TRUE
* @param[in] useMii                   - use MII
*
* @param[out] timeMilliPtr             - pointer to duration of TX operation in milliseconds.
* @param[out] sendFailPtr              - number of failed TX operations
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfTrafficGeneratorNetIfSyncTxBurstSend
(
    IN CPSS_INTERFACE_INFO_STC          *portInterfacePtr,
    IN GT_U8                            *bufferPtr,
    IN GT_U32                            bufferLength,
    IN GT_U32                            packetsNumber,
    IN GT_BOOL                           doForceDsa,
    IN TGF_DSA_TYPE_ENT                  forcedDsa,
    IN GT_BOOL                           useMii,
    OUT double                          *timeMilliPtr,
    OUT GT_U32                          *sendFailPtr
)
{
#ifdef CHX_FAMILY
        GT_U8   devNum    = 0;
        GT_U32  numOfBufs = 0;
        GT_U32  vpt       = 0;
        GT_U32  cfiBit    = 0;
        GT_U32  vid       = 0;
        GT_BOOL packetIsTagged = GT_FALSE;
        GT_STATUS   rc = GT_OK;
        CPSS_DXCH_NET_TX_PARAMS_STC       dxChPcktParams;
        GT_U32                             ii;
        GT_U32                             sendFail = 0;
        GT_U32                             sendOk = 0;
        GT_STATUS                          sendFailRC;
        GT_U32 startSec, endSec;     /* start and end time seconds part */
        GT_U32 startNano, endNano;   /* start and end time nanoseconds part */
        double startMilli, endMilli; /* start and end time in milliseconds */

        GT_U8   *buffList[1];
        GT_U32  buffLenList[1];

        CPSS_NULL_PTR_CHECK_MAC(portInterfacePtr);
        CPSS_NULL_PTR_CHECK_MAC(bufferPtr);

        buffList[0] = bufferPtr;
        buffLenList[0] = bufferLength;

        numOfBufs = 1;
        devNum    = prvTgfCpuDevNum;

        if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PX_FAMILY_PIPE_E)
        {
            if ((ignore_etherTypeForVlanTag == GT_FALSE) &&
                (bufferPtr[12] == (GT_U8)(etherTypeForVlanTag >> 8)) &&
                (bufferPtr[13] == (GT_U8)(etherTypeForVlanTag & 0xFF)))
            {
                packetIsTagged = GT_TRUE;
                vpt    = (GT_U8)(bufferPtr[14] >> 5);
                cfiBit = (GT_U8)((bufferPtr[14] >> 4) & 1);
                vid    = ((bufferPtr[14] & 0xF) << 8) | bufferPtr[15];
            }
            else
            {
                packetIsTagged = GT_FALSE;
                vpt    = 0;
                cfiBit = 0;
                vid    = 4094;
            }

            /* In sip 6 diffrents port mode have diffrent number of vlan entries*/
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                GT_U32      numVlanEntries = 0;

                rc = cpssDxChCfgTableNumEntriesGet(devNum,CPSS_DXCH_CFG_TABLE_VLAN_E,&numVlanEntries);
                if (rc != GT_OK)
                {
                    return rc;
                }
                vid = (vid > numVlanEntries ? (numVlanEntries - 1) : vid);
            }

            /* fill in DXCH info */
            rc = prvTgfDxChNetIfParamSet(portInterfacePtr, vpt, cfiBit, vid,
                                         packetIsTagged, doForceDsa, forcedDsa,
                                         &dxChPcktParams);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* store start time */
            cpssOsTimeRT(&startSec, &startNano);

            sendFailRC = GT_OK;
            for (ii = 0; ii < packetsNumber; ii++)
            {
                rc = cpssDxChNetIfSdmaSyncTxPacketSendPtr(devNum, &dxChPcktParams, buffList, buffLenList, numOfBufs);
                if (GT_OK != rc)
                {
                    sendFail++;
                    sendFailRC = rc;
                }
                else
                {
                    sendOk++;
                }
            }

            /* get end time */
            cpssOsTimeRT(&endSec, &endNano);

            startMilli = startSec * 1000 + startNano / 1000000;
            endMilli = endSec * 1000 + endNano / 1000000;

            *timeMilliPtr = (endMilli - startMilli);
            *sendFailPtr  = sendFail;

            return  sendFailRC;
        }
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) && !(defined EX_FAMILY)
    portInterfacePtr = portInterfacePtr ;
    bufferPtr        = bufferPtr        ;
    bufferLength     = bufferLength     ;
    packetsNumber    = packetsNumber    ;
    doForceDsa       = doForceDsa       ;
    forcedDsa        = forcedDsa        ;
    useMii           = useMii           ;
    timeMilliPtr     = timeMilliPtr     ;
    sendFailPtr      = sendFailPtr      ;

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) && !(defined EX_FAMILY) */
    return GT_OK;
}

#ifdef CHX_FAMILY
    CPSS_DXCH_NET_TX_PARAMS_STC       prvTgfRxToTxdxChPcktParams;
#endif /* CHX_FAMILY */

/**
* @internal prvTgfTrafficGeneratorRxToTxInfoSet function
* @endinternal
*
* @brief   Fill in structure for Rx To Tx send packet info
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] vpt                      - VPT of the packet
* @param[in] cfiBit                   - CFI bit
* @param[in] vid                      - VID of the packet
* @param[in] packetIsTagged           - packet is tagged flag
* @param[in] doForceDsa               - GT_FALSE - choose DSA tag automatically
*                                      GT_TRUE - use DSA tag defined by forcedDsa
* @param[in] forcedDsa                - DSA tag type, used when doForceDsa == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
*/
GT_STATUS prvTgfTrafficGeneratorRxToTxInfoSet
(
    IN CPSS_INTERFACE_INFO_STC            *portInterfacePtr,
    IN GT_U32                              vpt,
    IN GT_U32                              cfiBit,
    IN GT_U32                              vid,
    IN GT_BOOL                             packetIsTagged,
    IN GT_BOOL                             doForceDsa,
    IN TGF_DSA_TYPE_ENT                    forcedDsa
)
{
#ifdef CHX_FAMILY
{
    GT_U8       devNum;
    GT_STATUS   rc;
    devNum = (GT_U8)portInterfacePtr->devPort.hwDevNum;

    if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PX_FAMILY_PIPE_E)
    {
        rc = prvTgfDxChNetIfParamSet(portInterfacePtr, vpt, cfiBit, vid,
                                 packetIsTagged, doForceDsa, forcedDsa,
                                 &prvTgfRxToTxdxChPcktParams);
        if(rc != GT_OK)
        {
            PRV_TGF_LOG1_MAC("[TGF]: prvTgfDxChNetIfParamSet FAILED, rc = [%d]", rc);
        }
        return rc;
    }
}
#endif

    portInterfacePtr = portInterfacePtr ;
    vpt              = vpt              ;
    cfiBit           = cfiBit           ;
    vid              = vid              ;
    packetIsTagged   = packetIsTagged   ;
    doForceDsa       = doForceDsa       ;
    forcedDsa        = forcedDsa        ;

    return GT_NOT_IMPLEMENTED;
}

/**
* @internal prvTgfNetIfRxToTxInfoSet function
* @endinternal
*
* @brief   The function prepares TGF insrastructure for automatic Tx packets from CPU
*         for any Rx packet.
*         All packets came to CPU will be TXed to port defined by input parameter.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] packetIsTagged           - packet is tagged flag
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
*/
GT_STATUS prvTgfNetIfRxToTxInfoSet
(
    IN GT_U8                               devNum,
    IN GT_U32                              portNum,
    IN GT_BOOL                             packetIsTagged
)
{
    GT_STATUS                   rc;
    CPSS_INTERFACE_INFO_STC     portInterface;
    GT_U32  vid       = 4094;

    /* assign target port */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = devNum;
    portInterface.devPort.portNum  = portNum;
#ifdef DXCH_CODE
    /* In sip 6 diffrents port mode have diffrent number of vlan entries*/
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        GT_U32      numVlanEntries = 0;

        rc = cpssDxChCfgTableNumEntriesGet(devNum,CPSS_DXCH_CFG_TABLE_VLAN_E,&numVlanEntries);
        if (rc != GT_OK)
        {
            return rc;
        }
        vid = (vid > numVlanEntries ? (numVlanEntries - 1) : vid);
    }
#endif
    /* store Rx to Tx info */
    rc = prvTgfTrafficGeneratorRxToTxInfoSet(&portInterface, 0, 0, vid, packetIsTagged,
                                             GT_FALSE, /* ignored*/ TGF_DSA_1_WORD_TYPE_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* enable Rx to Tx */
    tgfTrafficTableRxToTxSet(GT_TRUE);

    /* disable Do Counting only mode to get Rx to Tx processing */
    tgfTrafficTableRxDoCountOnlySet(GT_FALSE);

    /* Disable TX DMA and other interrups to avoid interrupts influence on test results.
       These interrupts are not used for TX from CPU in synch mode. */
    cpssEventDeviceMaskSet(devNum, CPSS_PP_TX_END_E, CPSS_EVENT_MASK_E);
    cpssEventDeviceMaskSet(devNum, CPSS_PP_TX_ERR_QUEUE_E, CPSS_EVENT_MASK_E);
    cpssEventDeviceMaskSet(devNum, CPSS_PP_RX_CNTR_OVERFLOW_E, CPSS_EVENT_MASK_E);

    return GT_OK;
}

/**
* @internal prvTgfTrafficGeneratorRxToTxSend function
* @endinternal
*
* @brief   Transmit the traffic to predefined port by prvTgfTrafficGeneratorRxToTxInfoSet
*
* @param[in] devNum                   - device number
* @param[in] numOfBufs                - number of buffers for packet
* @param[in] buffList[]               - list of buffers
* @param[in] buffLenList[]            - buffer length list (include CRC bytes)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfTrafficGeneratorRxToTxSend
(
    IN GT_U8            devNum,
    IN GT_U32           numOfBufs,
    IN GT_U8            *buffList[],
    IN GT_U32           buffLenList[]
)
{
#ifdef CHX_FAMILY
    if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PX_FAMILY_PIPE_E)
    {
        GT_BOOL useMii;
        useMii =(PRV_CPSS_PP_MAC(prvTgfCpuDevNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E) ? GT_TRUE : GT_FALSE;
        if (useMii == GT_TRUE)
        {
            return cpssDxChNetIfMiiSyncTxPacketSendPtr(devNum, &prvTgfRxToTxdxChPcktParams, buffList, buffLenList, numOfBufs);
        }
        return cpssDxChNetIfSdmaSyncTxPacketSendPtr(devNum, &prvTgfRxToTxdxChPcktParams, buffList, buffLenList, numOfBufs);
    }
#endif /* CHX_FAMILY */

    devNum      = devNum      ;
    numOfBufs   = numOfBufs   ;
    buffList    = buffList    ;
    buffLenList = buffLenList ;

    return GT_NOT_IMPLEMENTED;
}

/*******************************************************************************
* prvTgfTrafficGeneratorPoolGetBuffer
*
* DESCRIPTION:
*       Allocates buffer from pool for sending packets
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_VOID* - Pointer to the new allocated buffer. NULL is returned in case
*               no buffer is not available.
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID* prvTgfTrafficGeneratorPoolGetBuffer
(
    GT_VOID
)
{
    return gtPoolGetBuf(prvTgfTxBuffersPoolId);
}

/**
* @internal prvTgfTrafficGeneratorPoolFreeBuffer function
* @endinternal
*
* @brief   Frees buffer from pool for sending packets
*
* @param[in] bufPtr                   - Pointer to buffer to be freed
*
* @retval GT_OK                    - For successful operation.
* @retval GT_BAD_PTR               - The returned buffer is not belongs to that pool
*/
GT_STATUS prvTgfTrafficGeneratorPoolFreeBuffer
(
    IN GT_VOID *bufPtr
)
{
    return gtPoolFreeBuf(prvTgfTxBuffersPoolId, bufPtr);
}

/**
* @internal prvTgfTrafficGeneratorPoolInit function
* @endinternal
*
* @brief   Init the pool for sending packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfTrafficGeneratorPoolInit
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    if(GT_FALSE == generatorPoolInit)
    {
        GT_BOOL useDMA = GT_TRUE;
#if defined CHX_FAMILY
        if (PRV_CPSS_PP_MAC(prvTgfCpuDevNum)->cpuPortMode != CPSS_NET_CPU_PORT_MODE_SDMA_E)
            useDMA = GT_FALSE;
#endif
        /* create pool of buffers from cache */
        if (useDMA == GT_TRUE)
        {
            rc = gtPoolCreateDmaPool(TGF_TX_BUFFER_MAX_SIZE_CNS,
                                     GT_4_BYTE_ALIGNMENT,
                                     TGF_MAX_NUM_OF_BUFFERS_AT_POOL,
                                     GT_TRUE,
                                     &prvTgfTxBuffersPoolId);
        }
        else
        {
            rc = gtPoolCreatePool(TGF_TX_BUFFER_MAX_SIZE_CNS,
                                     GT_4_BYTE_ALIGNMENT,
                                     TGF_MAX_NUM_OF_BUFFERS_AT_POOL,
                                     GT_TRUE,
                                     &prvTgfTxBuffersPoolId);
        }
        if (GT_OK != rc)
        {
            return rc;
        }

        generatorPoolInit = GT_TRUE;
    }



    return rc;
}

/* statistic DB */
#define TGF_STAT_LOOPS_CNS     (IDLE_POOLING_MAX_COUNT_CNS + 1)

/* number of Simulation Idle state pooling loops */
GT_U32 tgfStatLoops[TGF_STAT_LOOPS_CNS];
/* number of Rx MAC Counters pooling loops */
GT_U32 tgfStatCntrLoops[TGF_STAT_LOOPS_CNS];

/**
* @internal tgfStatLoopsDump function
* @endinternal
*
* @brief   Dump statistic DB.
*
* @retval GT_OK                    - on success
*/
int tgfStatLoopsDump(void)
{
    int ii;

    PRV_TGF_LOG0_MAC("\n Idle state Loops statistic\n");
    for (ii = 0; ii < TGF_STAT_LOOPS_CNS; ii++)
    {
        if (tgfStatLoops[ii])
        {
            PRV_TGF_LOG2_MAC("loop %2d count %d\n", ii, tgfStatLoops[ii]);
        }
    }

    PRV_TGF_LOG0_MAC("\n Counters Loops statistic\n");
    for (ii = 0; ii < TGF_STAT_LOOPS_CNS; ii++)
    {
        if (tgfStatCntrLoops[ii])
        {
            PRV_TGF_LOG2_MAC("loop %2d count %d\n", ii, tgfStatCntrLoops[ii]);
        }
    }

    return GT_OK;
}

/**
* @internal tgfTrafficGeneratorCheckProcessignDone function
* @endinternal
*
* @brief   Check that all packets were processed by device.
*         The function checks Idle state of device - works on simulation only.
* @param[in] sleepAfterCheck          - GT_TRUE - sleep some time after Idle check
*                                      GT_FALSE - do not sleep after Idle check
* @param[in] loopCount                - number of 1 milisecond sleeps done before call of
*                                      the function. Used only if sleepAfterCheck == GT_TRUE
*
* @retval 0                        - pooling detected that system is still busy
* @retval other value              - number of pooling loops till detection of busy.
*/
GT_U32 tgfTrafficGeneratorCheckProcessignDone
(
    IN GT_BOOL  sleepAfterCheck,
    IN GT_U32   loopCount
)
{
#if defined(ASIC_SIMULATION) && !defined(GM_USED)

    GT_U32 status; /* simulation Idle or Busy status */
    GT_U32 overallSleep; /* overall sleep time */
    GT_U32 loops; /* loops counter */

startLoops_lbl:
    if(tgfStormingExpected == GT_FALSE)
    {
        loops = IDLE_POOLING_MAX_COUNT_CNS;
    }
    else
    {
        loops = 100;
    }
    overallSleep = loopCount;

    while(loops)
    {
        status = skernelStatusGet();
        if (status == 0)
        {
            /* the simulation is Idle. Simulation finished processing
               of packets. */
            tgfStatLoops[IDLE_POOLING_MAX_COUNT_CNS - loops]++;
            break;
        }

        /* simulation is Busy. Check it again after sleep. */
        cpssOsTimerWkAfter(1);
        overallSleep++;
        loops--;
    }

    if (loops == 0)
    {
        if(simLogIsOpen())
        {
            /* we need to wait more for the end of processing */
            goto startLoops_lbl;
        }

        tgfStatLoops[IDLE_POOLING_MAX_COUNT_CNS]++;
    }

    if (sleepAfterCheck == GT_TRUE)
    {
        if (overallSleep < MINIMAL_SLEEP_CNS)
        {
            /* the test need some minimal time for CPSS Event handler tasks */
            cpssOsTimerWkAfter(MINIMAL_SLEEP_CNS);
        }
        else
        {
            cpssOsTimerWkAfter(MANDATORY_SLEEP_CNS);
        }
    }

    return loops;
#else
    return IDLE_POOLING_MAX_COUNT_CNS;
#endif
}

/**
* @internal tgfTrafficGeneratorStormingExpected function
* @endinternal
*
* @brief   state that the test doing storming on the ingress port (ingress traffic
*         will be send back to ingress port that is in 'loopback mode').
*         this indication needed for WM to know to break the loop regardless to
*         indications from skernelStatusGet()
* @param[in] stormingExpected         - GT_TRUE - storming is expected
*                                      GT_FALSE - storming is NOT expected
*                                       nothing
*/
GT_VOID tgfTrafficGeneratorStormingExpected(IN GT_BOOL stormingExpected)
{
    tgfStormingExpected = stormingExpected;
}

/**
* @internal prvTgfDevPortMacCountersClearOnReadSet function
* @endinternal
*
* @brief   Enable or disable MAC Counters Clear on read per group of ports
*         for Tri-Speed Ports or per port for HyperG.Stack ports.
*         For Tri-Speed Ports groups are:
*         - Ports 0 through 5
*         - Ports 6 through 11
*         - Ports 12 through 17
*         - Ports 18 through 23
* @param[in] devNum
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   -  clear on read for MAC counters
*                                      GT_FALSE - Counters are not cleared.
*                                      GT_TRUE - Counters are cleared.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDevPortMacCountersClearOnReadSet
(
    IN GT_U8    devNum,
    IN GT_U32   portNum,
    IN GT_BOOL  enable
)
{
#ifdef PX_CODE
    if(PRV_CPSS_PP_MAC(devNum) && CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        /* call device specific API */
        return cpssPxPortMacCountersClearOnReadSet(devNum, portNum, enable);
    }
#endif /*PX_CODE*/

#if (defined CHX_FAMILY)
    /* call device specific API */
    return cpssDxChPortMacCountersClearOnReadSet(devNum, portNum, enable);

#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);


    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfDevPortMacCountersClearOnReadGet function
* @endinternal
*
* @brief   Get status (Enable or disable) of MAC Counters Clear on read per group of ports
*         for Tri-Speed Ports or per port for HyperG.Stack ports.
*         For Tri-Speed Ports groups are:
*         - Ports 0 through 5
*         - Ports 6 through 11
*         - Ports 12 through 17
*         - Ports 18 through 23
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - pointer to enable clear on read for MAC counters
*                                      GT_FALSE - Counters are not cleared.
*                                      GT_TRUE - Counters are cleared.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDevPortMacCountersClearOnReadGet
(
    IN GT_U8    devNum,
    IN GT_U32    portNum,
    OUT GT_BOOL  *enablePtr
)
{
#ifdef PX_CODE
    if(PRV_CPSS_PP_MAC(devNum) && CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        /* call device specific API */
        return cpssPxPortMacCountersClearOnReadGet(devNum, portNum, enablePtr);
    }
#endif /*PX_CODE*/


#if (defined CHX_FAMILY)
    /* call device specific API */
    return cpssDxChPortMacCountersClearOnReadGet(devNum, portNum, enablePtr);

#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enablePtr);


    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortMacCounterGet function
* @endinternal
*
* @brief   Gets Ethernet MAC counter for a specified port on specified device.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*                                      CPU port counters are valid only when using "Ethernet CPU
*                                      port" (not using the SDMA interface).
*                                      when using the SDMA interface refer to the API
*                                      cpssDxChNetIfSdmaRxCountersGet(...)
* @param[in] cntrName                 - specific counter name
*
* @param[out] cntrValuePtr             - (pointer to) current counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The 10G MAC MIB counters are 64-bit wide.
*       Not supported counters: CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E,
*       CPSS_BadFC_RCV_E, CPSS_GOOD_PKTS_RCV_E, CPSS_GOOD_PKTS_SENT_E.
*
*/
GT_STATUS prvTgfPortMacCounterGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
)
{
#ifdef CHX_FAMILY
{
    if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PX_FAMILY_PIPE_E)
    {
       GT_STATUS  rc;
       /* Get Port MAC MIB counters */
       rc = cpssDxChMacCounterGet(devNum, portNum, cntrName, cntrValuePtr);
       if (GT_OK != rc)
       {
           PRV_TGF_LOG1_MAC(
               "[TGF]: cpssDxChMacCounterGet FAILED, rc = [%d]", rc);
       }
       return rc;
    }
}
#endif /* CHX_FAMILY */

#ifdef PX_CODE
    if(PRV_CPSS_PP_MAC(devNum) && CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        CPSS_PX_PORT_MAC_COUNTER_ENT px_cntrName;
        if(cntrName == CPSS_GOOD_OCTETS_RCV_E)
        {
            px_cntrName = CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E;
        }
        else
        if(cntrName == CPSS_GOOD_OCTETS_SENT_E)
        {
            px_cntrName = CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E;
        }
        else
        {
            return GT_NOT_IMPLEMENTED;
        }

        /* call device specific API */
        return cpssPxPortMacCounterGet(devNum, portNum, px_cntrName , cntrValuePtr);
    }
#endif /*PX_CODE*/


#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(cntrName);
    TGF_PARAM_NOT_USED(cntrValuePtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
    return GT_OK;
}


/**
* @internal tgfTrafficGeneratorCheckTxDone function
* @endinternal
*
* @brief   Check that all packets were entered to port.
*         The function checks Rx MAC MIB counter to check that all packets
*         entered to ingress test port.
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] burstCount               - number of frames (non-zero value)
* @param[in] packetSize               - size of single packet
* @param[in] cntrName                 - specific counter name
* @param[in] counterPreTxPtr          - pointer to counter value before Tx
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS tgfTrafficGeneratorCheckTxDone
(
    IN CPSS_INTERFACE_INFO_STC         *portInterfacePtr,
    IN GT_U32                           burstCount,
    IN GT_U32                           packetSize,
    IN CPSS_PORT_MAC_COUNTERS_ENT       cntrName,
    IN GT_U64                           *counterPreTxPtr
)
{
    GT_STATUS   rc;            /* return status */
    GT_U32      portNum = 0;   /* port number */
    GT_BOOL     clearOnRead;   /* clear on read status */
    GT_U64      portRxCntr;    /* port RX MAC MIB counter value */
    GT_U32      burstSize = burstCount * packetSize; /* size of burst in bytes */
    GT_U32      loopCount, loops; /* loop counters */
    GT_U8       devNum = 0;    /* device number */
    CPSS_PP_FAMILY_TYPE_ENT  devFamily; /* device family */
    GT_U32      prevCount;/* previous packets count*/
    GT_U32      countFail; /* MAC counters pooling failure indication:
                             0 - no failure, other - failure and last loop number */

    /* reset MAC counters read loop */
    loopCount = 0;
    countFail = 0;

    /* skip counters check for FWS traffic */
    if (prvTgfFwsUse == 0)
    {
        portNum = portInterfacePtr->devPort.portNum;

/* TODO */

        /* get device number */
#if 0
        if (GT_OK != prvTgfPortDeviceNumByPortGet(portNum, &devNum))
        {
            return GT_BAD_PARAM;
        }
#endif
        devNum = (GT_U8)(portInterfacePtr->devPort.hwDevNum);

        /* get device family */
        DEV_CHECK_MAC(devNum);
        devFamily = PRV_CPSS_PP_MAC(devNum)->devFamily;

#ifdef CHX_FAMILY
        if(CPSS_IS_DXCH_FAMILY_MAC(devFamily))
        {
            GT_BOOL isCpuPort;

            rc = cpssDxChPortPhysicalPortMapIsCpuGet(devNum,portNum,&isCpuPort);
            if (rc != GT_OK)
            {
                return rc;
            }

            if(isCpuPort)
            {
                /* the counters value is not relevant as for sending from the CPU hold no counters */
                goto afterCountersCheck_lbl;
            }
        }
#endif /*CHX_FAMILY*/

        /* check counters for DX device only.
          Puma2 GM has Clear-On-Read not working for ports 18, 23 */
        if (CPSS_IS_DXCH_FAMILY_MAC(devFamily) ||
            CPSS_IS_PX_FAMILY_MAC(devFamily))
        {
            /* get status of clear on read */
            rc = prvTgfDevPortMacCountersClearOnReadGet(devNum, portNum, &clearOnRead);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* disable clear counters on read */
            if (clearOnRead != GT_FALSE)
            {
                rc = prvTgfDevPortMacCountersClearOnReadSet(devNum, portNum, GT_FALSE);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }

            portRxCntr.l[0] = 0;
            portRxCntr.l[1] = 0;
            prevCount = 0;
            do
            {
                /* NOTE :
                   use counter of bytes and not packets because for packets we will need to accumulate 3 counters of : uc,mc,bc
                   so working with bytes saves total runtime
                */

                /* get Rx counter value */
                rc = prvTgfPortMacCounterGet(devNum, portNum, cntrName, &portRxCntr);
                if (rc != GT_OK)
                {
                    return rc;
                }

                if ((portRxCntr.l[0] - counterPreTxPtr->l[0]) >= burstSize)
                {
                    /* all packets were come to port's Rx MAC */
                    break;
                }

                loopCount++;
                cpssOsTimerWkAfter(1);

                if((burstSize > 1) && (loopCount > 200))
                {
                    /* after minimum of 200 iterations (200 milliseconds)
                        lets examine that the device still process packets.
                        do that by checking if the number of receives packets increase.
                    */

                    if(prevCount == portRxCntr.l[0])
                    {
                        /* no processing done in the last period .. no need to wait till end of iterations */
                        countFail = loopCount;
                        break;
                    }

                    if(prevCount > portRxCntr.l[0])
                    {
                        /* should not happen -- ERROR */
                        return (GT_STATUS)GT_ERROR; /*casting needed for DIAB compiler*/
                    }
                }

                prevCount = portRxCntr.l[0];

            }while(loopCount < prvTgfTrafficRxCheckMaxCount);

            tgfStatCntrLoops[loopCount]++;

            if (loopCount >= prvTgfTrafficRxCheckMaxCount)
            {
                /* probably packets were lost */
                countFail = loopCount;
            }

            if (clearOnRead != GT_FALSE)
            {
                /* restore enable clear counters on read */
                rc = prvTgfDevPortMacCountersClearOnReadSet(devNum, portNum, GT_TRUE);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        } /* if(CPSS_IS_DXCH_FAMILY_MAC(devFamily)) */
    }

#ifdef CHX_FAMILY
afterCountersCheck_lbl:
#endif /*CHX_FAMILY*/

    /* let the packets be sent */
#ifdef  EMBEDDED
    /* run on hardware - one 10 millisecond sleep is enough */
    loops = 1;
#else
    /* run on simulation */
    loops = prvTgfTimeLoop;
#endif

    if(cpssDeviceRunCheck_onEmulator())
    {
        /* run on Emulator , the device is processing the packet(s) slowly ! */
        loops = prvTgfTimeLoop;
    }

#if defined(ASIC_SIMULATION) && !defined(GM_USED)
    if (prvTgfFwsUse == 0 &&
        tgfTrafficGeneratorCheckProcessignDone(GT_TRUE, loopCount) == 0)
    {
        /* probably packets were lost */
        PRV_TGF_LOG2_MAC("\nWv: Counters count[%d], port [%d]\n", loopCount, portInterfacePtr->devPort.portNum);
    }
#else
#ifdef GM_USED
    if(SUPPORT_ALL_IN_SINGLE_CONTEXT_MAC)
    {
        loops = 0;
    }
#endif /*GM_USED*/

    /* adapt MAC counters loopCount 1 millisecond sleep for units of
       10 millisecond sleep loops */
    loopCount = loopCount / 10;

    if (loopCount < loops)
    {
        loops -= loopCount;
        while(loops--)/* allow debugging by set this value = 1000 */
        {
            cpssOsTimerWkAfter(10);
        }
    }
#endif
    if(countFail)
    {
        PRV_TGF_LOG2_MAC("\nWw: Counters count[%d] Idle count[%d]\n", countFail, loops);
#ifdef  EMBEDDED

        /* ON HW this is real indication that packet got lost .... maybe this is intended
          (in this case prvTgfIntendedError should be set to GT_TRUE)
        but on most cases this is indication of unexpected filter by the device !!!
        make it  cpssOsPrintf and not PRV_TGF_LOG0_MAC because we need those
        errors regardless to LOG print mode !!!
        */
        if (!tgfPrvIntendedError)
        {
            cpssOsPrintf("ERROR : The 'FROM_CPU' packet of the TGF engine did not reach the target port %d/%d ('src port' of the test !!!) \n", devNum, portNum);
            cpssOsPrintf("ERROR : Check what caused the Device to filter the packet \n");
        }
 #endif

    }

    return GT_OK;
}

/**
* @internal tgfTrafficGeneratorPreTxCountGet function
* @endinternal
*
* @brief   Get counter value for Tx Done check
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] cntrName                 - specific counter name
*
* @param[out] counterPtr               - pointer to counter.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS tgfTrafficGeneratorPreTxCountGet
(
    IN  CPSS_INTERFACE_INFO_STC         *portInterfacePtr,
    IN  CPSS_PORT_MAC_COUNTERS_ENT      cntrName,
    OUT GT_U64                           *counterPtr
)
{
    GT_STATUS   rc;            /* return status */
    GT_U32      portNum;       /* port number */
    GT_BOOL     clearOnRead;   /* clear on read status */
    GT_U8       devNum;        /* device number */
    CPSS_PP_FAMILY_TYPE_ENT  devFamily; /* device family */

    counterPtr->l[0] = 0;
    counterPtr->l[1] = 0;

    /* skip counters check for FWS traffic */
    if (prvTgfFwsUse == 0)
    {
        portNum = portInterfacePtr->devPort.portNum;

/*TODO*/
        /* get device number */
        devNum = (GT_U8)portInterfacePtr->devPort.hwDevNum;
#if 0
        if (GT_OK != prvTgfPortDeviceNumByPortGet(portNum, &devNum))
        {
            return GT_BAD_PARAM;
        }
#endif

        /* get device family */
        DEV_CHECK_MAC(devNum);
        devFamily = PRV_CPSS_PP_MAC(devNum)->devFamily;

#ifdef CHX_FAMILY
        if(CPSS_IS_DXCH_FAMILY_MAC(devFamily))
        {
            GT_BOOL isCpuPort;

            rc = cpssDxChPortPhysicalPortMapIsCpuGet(devNum,portNum,&isCpuPort);
            if (rc != GT_OK)
            {
                return rc;
            }

            if(isCpuPort)
            {
                /* the counters value is not relevant as for sending from the CPU hold no counters */
                return GT_OK;
            }
        }
#endif /*CHX_FAMILY*/

        /* check counters for DX device only.
          Puma2 GM has Clear-On-Read not working for ports 18, 23 */
        if (CPSS_IS_DXCH_FAMILY_MAC(devFamily) ||
            CPSS_IS_PX_FAMILY_MAC(devFamily))
        {
            /* get status of clear on read */
            rc = prvTgfDevPortMacCountersClearOnReadGet(devNum, portNum, &clearOnRead);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* disable clear counters on read */
            if (clearOnRead != GT_FALSE)
            {
                rc = prvTgfDevPortMacCountersClearOnReadSet(devNum, portNum, GT_FALSE);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }

            /* get Rx counter value */
            rc = prvTgfPortMacCounterGet(devNum, portNum, cntrName, counterPtr);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (clearOnRead != GT_FALSE)
            {
                /* restore enable clear counters on read */
                rc = prvTgfDevPortMacCountersClearOnReadSet(devNum, portNum, GT_TRUE);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }

        } /* if (CPSS_IS_DXCH_FAMILY_MAC(devFamily)) */
    }

    return GT_OK;
}

/**
* @internal tgfTrafficGeneratorPortTxEthTransmit function
* @endinternal
*
* @brief   Transmit the traffic to the port
*         NOTE:
*         1. allow to transmit with chunks of burst:
*         after x sent frames --> do sleep of y millisecond
*         see parameters
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               - number of frames (non-zero value)
* @param[in] numVfd                   - number of VFDs
* @param[in] vfdArray[]               - pointer to array of VFDs (may be NULL if numVfd = 0)
* @param[in] sleepAfterXCount         - do 'sleep' after X packets sent
*                                      when = 0 , meaning NO SLEEP needed during the burst
*                                      of 'burstCount'
* @param[in] sleepTime                - sleep time (in milliseconds) after X packets sent , see
*                                      parameter sleepAfterXCount
* @param[in] traceBurstCount          - number of packets in burst count that will be printed
* @param[in] loopbackEnabled          - Check RCV counters when TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The packet will ingress to the PP after 'loopback' and will act as
*       it was received from a traffic generator connected to the port
*
*/
GT_STATUS tgfTrafficGeneratorPortTxEthTransmit
(
    IN CPSS_INTERFACE_INFO_STC         *portInterfacePtr,
    IN TGF_PACKET_STC                  *packetInfoPtr,
    IN GT_U32                           burstCount,
    IN GT_U32                           numVfd,
    IN TGF_VFD_INFO_STC                 vfdArray[],
    IN GT_U32                           sleepAfterXCount,
    IN GT_U32                           sleepTime,
    IN GT_U32                           traceBurstCount,
    IN GT_BOOL                          loopbackEnabled
)
{
    GT_STATUS   rc, rc1    = GT_OK;
    GT_U32      i, j;
    GT_U8       *bufferPtr = NULL;
    GT_U32      xCounter   ;/* counter of already sent packets ,
                            for sleep when xCounter = 0 ,
                            relevant only when sleepAfterXCount != 0*/
    GT_BOOL     hugeSend = GT_FALSE;/* is the burst 'huge'*/
    GT_U32      maxRetry = MAX_RETRY_CNS;  /* max retry number on error */
    GT_BOOL     doProgressIndication = GT_FALSE;
    GT_BOOL     avoidKeepAlive = GT_FALSE;/* avoid keep alive print */
    TGF_AUTO_CHECKSUM_FULL_INFO_STC checksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* auto checksum info */
    TGF_AUTO_CHECKSUM_EXTRA_INFO_STC extraChecksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* extra checksum info */
    GT_BOOL     useMii;/* do we use MII or SDMA */
    GT_U32      packetTotalLen = 0;/* packet total length (not include CRC) */
    GT_U32      numOfBytesInPacketBuff;/* number of bytes in packetBuff*/ /* save space of CRC*/
    GT_BOOL     keepAliveDone = GT_FALSE;
    GT_U64      counter;
    CPSS_PORT_MAC_COUNTERS_ENT cntrName; /* counter name */
#ifdef GM_USED
    GT_U32      loops; /* iterator */
#endif
    CPSS_NULL_PTR_CHECK_MAC(portInterfacePtr);
    CPSS_NULL_PTR_CHECK_MAC(packetInfoPtr);

    cntrName = (loopbackEnabled == GT_TRUE)
            ? CPSS_GOOD_OCTETS_RCV_E
            : CPSS_GOOD_OCTETS_SENT_E;

    if(numVfd)
    {
        CPSS_NULL_PTR_CHECK_MAC(vfdArray);
    }

    if(burstCount == 0)
    {
        return GT_BAD_PARAM;
    }

#ifdef PX_FAMILY
    TGF_PARAM_NOT_USED(keepAliveDone); /* prevent warning: not used variable */

    useMii = GT_FALSE;
#else
        #if defined CHX_FAMILY
            useMii =((PRV_CPSS_PP_MAC(prvTgfCpuDevNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E) ? GT_TRUE:
                     (PRV_CPSS_PP_MAC(prvTgfCpuDevNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_NONE_E) ? GT_FALSE:
                     PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(prvTgfCpuDevNum) ? GT_TRUE : GT_FALSE);
        #else
            useMii = GT_FALSE;
        #endif
#endif

    if(useMii == GT_TRUE)
    {
        if(burstCount > 128 && sleepAfterXCount == 0)
        {
            keepAliveDone = GT_TRUE;
            tgfPrintKeepAlive();
        }

#ifdef _WIN32
        if(burstCount >= 16 && sleepAfterXCount == 0)
        {
            sleepAfterXCount = miiCounter;
            sleepTime = 10;
            avoidKeepAlive = GT_TRUE;
        }
#endif /*_WIN32*/

    }
    else  /* SDMA */
    {
        /* the simulation in win32 need to process the 'loopback packets'.
           otherwise all the simulation buffers may be over because the SDMA
           processing is sending to the loopbacked port frames in new buffers but
           the smain task is still busy with new SDAM frame , so no one process
           the looped frames , so no one free those buffers ... no more buffers ...
           tests fail */

        /* NOTE: this code was also needed in XCAT board --> see test tgfTablesFillingLttCheckIpv4 */
        if(burstCount > 128 && sleepAfterXCount == 0)
        {
            keepAliveDone = GT_TRUE;
            tgfPrintKeepAlive();
            sleepAfterXCount = 64;
            sleepTime = 10;
            avoidKeepAlive = GT_TRUE;
        }

        if(burstCount > SDMA_ABORT_MAX_CNS &&
            (tgfTrafficGeneratorRxCaptureNum ||
            tgfTrafficGeneratorExpectTraficToCpuEnabled))
        {
#ifdef CHX_FAMILY
            if (PRV_CPSS_PP_MAC(prvTgfCpuDevNum)->devFamily != CPSS_PX_FAMILY_PIPE_E)
            {
                if (PRV_CPSS_DXCH_ERRATA_GET_MAC(prvTgfCpuDevNum, PRV_CPSS_DXCH3_SDMA_WA_E) == GT_TRUE)
                {
                    if (keepAliveDone == GT_FALSE)
                    {
                        keepAliveDone = GT_TRUE;
                        tgfPrintKeepAlive();
                    }
                    /* the device not support 'SDMA retry' for traffic trap to the CPU */
                    /* and we don't want to loose traffic trapped to the CPU */
                    sleepAfterXCount = SDMA_ABORT_MAX_CNS;
                    sleepTime = 10;
                    avoidKeepAlive = GT_TRUE;
                }
            }
#endif /*CHX_FAMILY*/
        }
    }

#ifdef GM_USED
    if(SUPPORT_ALL_IN_SINGLE_CONTEXT_MAC)
    {
        sleepTime = 0;
    }

#endif /*GM_USED*/


    xCounter = sleepAfterXCount;

    cpssOsMemSet(checksumInfoArr,0,sizeof(checksumInfoArr));

    /* init the pool */
    rc = prvTgfTrafficGeneratorPoolInit();
    if(rc != GT_OK)
    {
        return rc;
    }

    if(portInterfacePtr->type != CPSS_INTERFACE_PORT_E)
    {
        return GT_BAD_PARAM;
    }

    /* get buffer from the pool */
    bufferPtr = gtPoolGetBuf(prvTgfTxBuffersPoolId);
    if(NULL == bufferPtr)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    numOfBytesInPacketBuff = TGF_RX_BUFFER_MAX_SIZE_CNS - 4;/* save space of CRC*/
    /* build the basic frame info */                            /* save 4 bytes space for CRC */
    rc = tgfTrafficEnginePacketBuild(packetInfoPtr, bufferPtr ,&numOfBytesInPacketBuff, checksumInfoArr,extraChecksumInfoArr);
    if(rc != GT_OK)
    {
        PRV_TGF_LOG1_MAC("[TGF]: tgfTrafficEnginePacketBuild FAILED, rc = [%d]", rc);
        goto exit_cleanly_lbl;
    }

    packetTotalLen = numOfBytesInPacketBuff;

    if((packetTotalLen + TGF_CRC_LEN_CNS) > TGF_TX_BUFFER_MAX_SIZE_CNS)
    {
        return GT_BAD_PARAM;
    }

    if((packetTotalLen + TGF_CRC_LEN_CNS) < TGF_TX_BUFFER_MIN_SIZE_CNS)
    {
        PRV_TGF_LOG1_MAC("utf Transmit: WARNING !!! packet size is too small [%d] \n",
                         packetTotalLen + TGF_CRC_LEN_CNS);
    }

    hugeSend = (burstCount > traceBurstCount) ? GT_TRUE : GT_FALSE;

    if((hugeSend == GT_TRUE) && (debug_forcePrint == 0))
    {
        /* we can't allow printings for huge number of frames */
        rc = tgfTrafficTracePacketByteSet(GT_FALSE);
        if(rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }
    }

    if (prvTgfTrafficPrintPacketTxEnable)
    {
        PRV_TGF_LOG3_MAC("utf : transmit [%d] packets into dev [%d] port[%d] \n",
            burstCount,
            portInterfacePtr->devPort.hwDevNum,
            portInterfacePtr->devPort.portNum);
    }

    if(hugeSend == GT_TRUE)
    {
#ifdef ASIC_SIMULATION
        if(burstCount > 1000)
        {
            PRV_TGF_LOG3_MAC("utf Transmit: Huge burst[%d] to dev [%d] port[%d] \n",
                             burstCount,
                             portInterfacePtr->devPort.hwDevNum,
                             portInterfacePtr->devPort.portNum);
        }
#endif /*ASIC_SIMULATION*/
        doProgressIndication = GT_TRUE;
    }

    /* get mac counter before Tx */
    rc = tgfTrafficGeneratorPreTxCountGet(portInterfacePtr, cntrName, &counter);
    if(rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    for (i = 0; i < burstCount; i++)
    {
        for(j = 0; j < numVfd; j++)
        {
            vfdArray[j].modeExtraInfo = i;

            /* over ride the buffer with VFD info */
            rc = tgfTrafficEnginePacketVfdApply(&vfdArray[j], bufferPtr, packetTotalLen);
            if(rc != GT_OK)
            {
                goto exit_cleanly_lbl;
            }
        }

        /* apply auto checksum fields after applying the VFD*/
        rc = autoChecksumField(checksumInfoArr,extraChecksumInfoArr);
        if(rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }

        /* add the ability to trace the packet content */
        rc = tgfTrafficTracePacket(bufferPtr,
                                   packetTotalLen,
                                   GT_FALSE);
        if(rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }

        if(sleepAfterXCount && ((--xCounter) == 0))
        {
            /* allow the 'sleep' after X packets sent , to let AppDemo task to
               process AUQ */
            if(avoidKeepAlive == GT_FALSE)
            {
                tgfPrintKeepAlive();
            }

            cpssOsTimerWkAfter(sleepTime);
            xCounter   = sleepAfterXCount;
        }

        if(doProgressIndication == GT_TRUE)
        {
            if(4999 == (i % 5000))
            {
#ifdef ASIC_SIMULATION
                PRV_TGF_LOG1_MAC("sent [%d]",i+1);
#endif /*ASIC_SIMULATION*/
            }
            else if(1999 == (i % 2000))
            {
                tgfPrintKeepAlive();
            }
        }
retry_send_lbl:
        /* send the buffer to the device,port */
        rc = prvTgfTrafficGeneratorNetIfSyncTxPacketSend(portInterfacePtr,
                                                             bufferPtr,
                                                             packetTotalLen + TGF_CRC_LEN_CNS,
                                                             useMii);
        if(rc != GT_OK)
        {
            prvTgfTrafficGeneratorNumberPacketsSendFailed++;

            if(sleepTime)
            {
                cpssOsTimerWkAfter(sleepTime);
            }
            else
            {
                cpssOsTimerWkAfter(10);
            }

            /* retry to send again ! */
            if(--maxRetry)
            {
                PRV_TGF_LOG0_MAC("^");
                goto retry_send_lbl;
            }


            if(rc == GT_HW_ERROR)
            {
                consecutiveHwErrors++;

#ifdef ASIC_SIMULATION
                if(consecutiveHwErrors == 5)
                {
                    skernelFatalError("prvTgfTrafficGeneratorNetIfSyncTxPacketSend : unable to send packets , too many errors ! \n");
                }
#endif/*ASIC_SIMULATION*/
            }

            goto exit_cleanly_lbl;
        }
        else
        {
            prvTgfTrafficGeneratorNumberPacketsSendOk++;
        }
#ifdef GM_USED
        /* time factor for GM devices , due to very slow processing of packets (2 per second) */
        /* for 'fine tuning' / debug use function tgfTrafficGeneratorGmTimeFactorSet(...) */

        loops = prvTgfTimeLoop * prvTgfGmTimeFactor;

        if(SUPPORT_ALL_IN_SINGLE_CONTEXT_MAC)
        {
            loops = 0;
        }

        while(loops--)/* allow debugging by set this value = 1000 */
        {
            cpssOsTimerWkAfter(10);
        }
#endif /*GM_USED*/
        maxRetry = MAX_RETRY_CNS;/* restore the 5 retries per frame */
    }

    /* we managed to send the packets */
    consecutiveHwErrors = 0;


    if(sleepAfterXCount)
    {
        /* allow appDemo process the last section of the burst */
        cpssOsTimerWkAfter(sleepTime);
    }


#ifdef ASIC_SIMULATION
    if(hugeSend == GT_TRUE && burstCount > 1000)
    {
        PRV_TGF_LOG0_MAC("utf Transmit: Huge burst Ended \n");
    }
#endif /*ASIC_SIMULATION*/

    rc1 = tgfTrafficGeneratorCheckTxDone(portInterfacePtr,
            burstCount, packetTotalLen  + TGF_CRC_LEN_CNS,
            cntrName, &counter);

exit_cleanly_lbl:
    if(hugeSend == GT_TRUE)
    {
        /* restore value */
        rc1 = tgfTrafficTracePacketByteSet(GT_TRUE);
    }

    if(bufferPtr)
    {
        /* release the buffer back to the pool */
        rc1 = gtPoolFreeBuf(prvTgfTxBuffersPoolId, bufferPtr);
    }

    rc = rc1 != GT_OK ? rc1 : rc;

    return rc;
}

/**
* @internal tgfTrafficGeneratorPortTxEthBurstTransmit function
* @endinternal
*
* @brief   Transmit the traffic burst to the port.
*         Used for TX SDMA performance measurement.
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               - number of frames (non-zero value)
* @param[in] doForceDsa               - GT_FALSE - choose DSA tag automatically
*                                      GT_TRUE - use DSA tag defined by forcedDsa
* @param[in] forcedDsa                - DSA tag type, used when doForceDsa == GT_TRUE
*
* @param[out] timeMilliPtr             - pointer to duration of TX operation in milliseconds.
* @param[out] sendFailPtr              - number of failed TX operations
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS tgfTrafficGeneratorPortTxEthBurstTransmit
(
    IN CPSS_INTERFACE_INFO_STC         *portInterfacePtr,
    IN TGF_PACKET_STC                  *packetInfoPtr,
    IN GT_U32                           burstCount,
    IN GT_BOOL                          doForceDsa,
    IN TGF_DSA_TYPE_ENT                 forcedDsa,
    OUT double                         *timeMilliPtr,
    OUT GT_U32                         *sendFailPtr
)
{
    GT_STATUS   rc, rc1    = GT_OK;
    GT_U8       *bufferPtr = NULL;
    TGF_AUTO_CHECKSUM_FULL_INFO_STC checksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* auto checksum info */
    TGF_AUTO_CHECKSUM_EXTRA_INFO_STC extraChecksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* extra checksum info */
    GT_U32      packetTotalLen = 0;/* packet total length (not include CRC) */
    GT_U32      numOfBytesInPacketBuff;/* number of bytes in packetBuff*/ /* save space of CRC*/
    GT_BOOL     useMii = GT_FALSE;

    CPSS_NULL_PTR_CHECK_MAC(portInterfacePtr);
    CPSS_NULL_PTR_CHECK_MAC(packetInfoPtr);

    if(burstCount == 0)
    {
        return GT_BAD_PARAM;
    }

    cpssOsMemSet(checksumInfoArr,0,sizeof(checksumInfoArr));

    /* init the pool */
    rc = prvTgfTrafficGeneratorPoolInit();
    if(rc != GT_OK)
    {
        return rc;
    }

    if(portInterfacePtr->type != CPSS_INTERFACE_PORT_E)
    {
        return GT_BAD_PARAM;
    }

    /* get buffer from the pool */
    bufferPtr = gtPoolGetBuf(prvTgfTxBuffersPoolId);
    if(NULL == bufferPtr)
    {
        return GT_OUT_OF_CPU_MEM;
    }

#if defined CHX_FAMILY
    useMii =((PRV_CPSS_PP_MAC(prvTgfCpuDevNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E) ? GT_TRUE:
             (PRV_CPSS_PP_MAC(prvTgfCpuDevNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_NONE_E) ? GT_FALSE:
             PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(prvTgfCpuDevNum) ? GT_TRUE : GT_FALSE);
#endif
    numOfBytesInPacketBuff = TGF_RX_BUFFER_MAX_SIZE_CNS - 4;/* save space of CRC*/
    /* build the basic frame info */                            /* save 4 bytes space for CRC */
    rc = tgfTrafficEnginePacketBuild(packetInfoPtr, bufferPtr ,&numOfBytesInPacketBuff, checksumInfoArr,extraChecksumInfoArr);
    if(rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    packetTotalLen = numOfBytesInPacketBuff;

    if((packetTotalLen + TGF_CRC_LEN_CNS) > TGF_TX_BUFFER_MAX_SIZE_CNS)
    {
        return GT_BAD_PARAM;
    }

    if((packetTotalLen + TGF_CRC_LEN_CNS) < TGF_TX_BUFFER_MIN_SIZE_CNS)
    {
        PRV_TGF_LOG1_MAC("utf Transmit: WARNING !!! packet size is too small [%d] \n",
                         packetTotalLen + TGF_CRC_LEN_CNS);
    }

    if (prvTgfTrafficPrintPacketTxEnable)
    {
        PRV_TGF_LOG2_MAC("utf : transmit [%d] packets into port[%d] \n",
            burstCount,
            portInterfacePtr->devPort.portNum);
    }

    /* apply auto checksum fields after applying the VFD*/
    rc = autoChecksumField(checksumInfoArr,extraChecksumInfoArr);
    if(rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* add the ability to trace the packet content */
    rc = tgfTrafficTracePacket(bufferPtr,
                               packetTotalLen,
                               GT_FALSE);
    if(rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }


    /* send the buffer to the device,port */
    rc = prvTgfTrafficGeneratorNetIfSyncTxBurstSend(portInterfacePtr,
                                                         bufferPtr,
                                                         packetTotalLen + TGF_CRC_LEN_CNS,
                                                         burstCount, doForceDsa, forcedDsa,
                                                         useMii,
                                                         timeMilliPtr, sendFailPtr);
    if(rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

exit_cleanly_lbl:
    if(bufferPtr)
    {
        /* release the buffer back to the pool */
        rc1 = gtPoolFreeBuf(prvTgfTxBuffersPoolId, bufferPtr);
    }

    rc = rc1 != GT_OK ? rc1 : rc;

    return rc;
}

/**
* @internal tgfTrafficGeneratorRxInCpuGet function
* @endinternal
*
* @brief   Get entry from rxNetworkIf table
*
* @param[in] packetType               - the packet type to get
* @param[in] getFirst                 - get first/next entry
* @param[in] trace                    - enable\disable packet tracing
* @param[in,out] packetBufLenPtr          - the length of the user space for the packet
*
* @param[out] packetBufPtr             - packet's buffer (pre allocated by the user)
* @param[in,out] packetBufLenPtr          - length of the copied packet to gtBuf
* @param[out] packetLenPtr             - Rx packet original length
* @param[out] devNumPtr                - packet's device number
* @param[out] queuePtr                 - Rx queue in which the packet was received.
* @param[out] rxParamsPtr              - specific device Rx info format.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - on more entries
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note For 'captured' see API tgfTrafficGeneratorPortTxEthCaptureSet
*
*/
GT_STATUS tgfTrafficGeneratorRxInCpuGet
(
    IN    TGF_PACKET_TYPE_ENT   packetType,
    IN    GT_BOOL               getFirst,
    IN    GT_BOOL               trace,
    OUT   GT_U8                *packetBufPtr,
    INOUT GT_U32               *packetBufLenPtr,
    OUT   GT_U32               *packetLenPtr,
    OUT   GT_U8                *devNumPtr,
    OUT   GT_U8                *queuePtr,
    OUT   TGF_NET_DSA_STC      *rxParamsPtr
)
{
    TGF_PACKET_TYPE_ENT currPacketType = TGF_PACKET_TYPE_REGULAR_E;
    static GT_U32       currentIndex   = 0;
    GT_STATUS           rc             = GT_OK;
    GT_BOOL             newGetFirst    = getFirst;
    GT_8                specificDeviceFormat[TGF_SPECIFIC_DEVICE_FORMAT_SIZE_CNS] = {0};
#ifdef CHX_FAMILY
    CPSS_DXCH_NET_RX_PARAMS_STC   *dxChPcktParamsPtr   = NULL;
#endif

    CPSS_NULL_PTR_CHECK_MAC(packetBufPtr);
    CPSS_NULL_PTR_CHECK_MAC(packetBufLenPtr);
    CPSS_NULL_PTR_CHECK_MAC(packetLenPtr);
    CPSS_NULL_PTR_CHECK_MAC(devNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(queuePtr);
    CPSS_NULL_PTR_CHECK_MAC(rxParamsPtr);

start_lbl:
    currPacketType = TGF_PACKET_TYPE_REGULAR_E;

    if(newGetFirst == GT_TRUE)
    {
        currentIndex = 0;
        if(retryRxToCpuAndWait)
        {
            /* wait only once , to allow context switching for all the needed packets */
            tgfTrafficGeneratorRxInCpuNumWait(currentIndex+1 , 0 , NULL);
        }
    }
    else
    {
        currentIndex++;
    }

    newGetFirst = GT_FALSE;

    rc = tgfTrafficTableRxPcktGet(currentIndex, packetBufPtr, packetBufLenPtr, packetLenPtr,
                                  devNumPtr, queuePtr, specificDeviceFormat);

    if(GT_OUT_OF_RANGE == rc)
    {
        /* table is done */
        return GT_NO_MORE;
    }
    else if(rc == GT_NOT_FOUND)
    {
        /* current index has not valid entry, but table is not done yet */
        goto start_lbl;
    }
    else if(rc != GT_OK)
    {
        return rc;
    }

#ifdef PX_CODE
    rxParamsPtr->dsaCmdIsToCpu = GT_TRUE;
#endif /*PX_CODE*/

#ifdef CHX_FAMILY
    if(PRV_CPSS_PP_MAC(*devNumPtr)->devFamily != CPSS_PX_FAMILY_PIPE_E)
    {
        dxChPcktParamsPtr = (CPSS_DXCH_NET_RX_PARAMS_STC*) specificDeviceFormat;

        if(CPSS_DXCH_NET_DSA_CMD_TO_CPU_E == dxChPcktParamsPtr->dsaParam.dsaType)
        {
            rxParamsPtr->dsaCmdIsToCpu = GT_TRUE;
            rxParamsPtr->cpuCode       = dxChPcktParamsPtr->dsaParam.dsaInfo.toCpu.cpuCode;
            rxParamsPtr->srcIsTrunk    = dxChPcktParamsPtr->dsaParam.dsaInfo.toCpu.srcIsTrunk;

            rc = extUtilSwDeviceNumberGet(dxChPcktParamsPtr->dsaParam.dsaInfo.toCpu.hwDevNum,
                                               &rxParamsPtr->hwDevNum);
            if(GT_OK != rc)
                return rc;
            rxParamsPtr->portNum       = dxChPcktParamsPtr->dsaParam.dsaInfo.toCpu.interface.portNum;
            rxParamsPtr->originByteCount = dxChPcktParamsPtr->dsaParam.dsaInfo.toCpu.originByteCount;

            if((dxChPcktParamsPtr->dsaParam.dsaInfo.toCpu.cpuCode == CPSS_NET_LAST_USER_DEFINED_E)||
               (dxChPcktParamsPtr->dsaParam.dsaInfo.toCpu.cpuCode == CPSS_NET_INGRESS_MIRRORED_TO_ANLYZER_E))
            {
                /* 'Captured' packet */
                currPacketType = TGF_PACKET_TYPE_CAPTURE_E;
            }
            /* those 2 parameters actually relevant to eDsa*/
            rxParamsPtr->trunkId       = dxChPcktParamsPtr->dsaParam.dsaInfo.toCpu.interface.srcTrunkId;
            rxParamsPtr->ePortNum      = dxChPcktParamsPtr->dsaParam.dsaInfo.toCpu.interface.ePort;

        }
        else
        {
            cpssOsPrintf("ERROR : tgfTrafficGeneratorRxInCpuGet : Got unexpected non 'TO_CPU' DSA in the CPU , got Type[%s] \n",
                (CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E    == dxChPcktParamsPtr->dsaParam.dsaType) ? "FROM_CPU"    :
                (CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E == dxChPcktParamsPtr->dsaParam.dsaType) ? "TO_ANALYZER" :
                "FORWARD");
            /* in case of unexpected value , set explicit values and not 'Randon callstack' values */
            cpssOsMemSet(dxChPcktParamsPtr,0xFF,sizeof(CPSS_DXCH_NET_RX_PARAMS_STC));
        }
    }
#endif /* CHX_FAMILY */

    if((currPacketType != packetType) && (packetType != TGF_PACKET_TYPE_ANY_E))
    {
        /* not matching packet type, so try to get next packet */
        goto start_lbl;
    }

    /* add the ability to trace the packet content */
    if (GT_TRUE == trace)
    {
        PRV_TGF_LOG2_MAC(
            "[TGF]: Packet received by CPU from dev [%d] port[%d]:\n",
            rxParamsPtr->hwDevNum, rxParamsPtr->portNum);
        rc = tgfTrafficTracePacket(packetBufPtr,
                                   MIN(*packetLenPtr, *packetBufLenPtr),
                                   GT_TRUE);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    /* add ability to trace the packet parts fields */
    CPSS_TBD_BOOKMARK

    /* add ability to trace the Rx specific device parameters */
    CPSS_TBD_BOOKMARK

    return rc;
}
/* indication to use rxParamPtr->ePortNum or rxParamPtr->portNum :
    if(comparePortAsEPort)
    {
        portNum = rxParamPtr->ePortNum;
    }
    else
    {
        portNum = rxParamPtr->portNum;
    }
*/
static GT_U32   comparePortAsEPort = 0;
static GT_U32   forceCompareAsPhysicalPort = 0;


/**
* @internal tgfTrafficGeneratorExpectedInterfaceEport function
* @endinternal
*
* @brief   set the expected mode of 'port' to be : ePort / phyPort
*         because the CPSS_INTERFACE_INFO_STC not hold difference between eport and
*         physical port , this setting will give the functions of 'expected' packets
*         to know if expected from the eport or from the physical port.
*         NOTEs:
*         1. by defaults : physical port
*         2. after compares done , the default value restored automatically by the engine
*
* @retval GT_TRUE                  - packet came from the expected interface
* @retval GT_FALSE                 - packet came from unexpected interface
*/
GT_STATUS tgfTrafficGeneratorExpectedInterfaceEport
(
    IN PRV_TGF_EXPECTED_INTERFACE_TYPE_ENT portInterfaceType
)
{
    if(TGF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfCpuDevNum) == GT_TRUE)
    {
        comparePortAsEPort = (portInterfaceType == PRV_TGF_EXPECTED_INTERFACE_TYPE_EPORT_NUM_E) ? 1 : 0;
        forceCompareAsPhysicalPort = (portInterfaceType == PRV_TGF_EXPECTED_INTERFACE_TYPE_FORCE_PHYSICAL_PORT_NUM_E) ? 1 : 0;
    }
    else
    {
        /* do not modify value for devices that not support eport */
    }

    return GT_OK;
}



/**
* @internal sprvTgfTrunkDbIsMemberOfTrunk function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         Checks if a member (device,port) is a trunk member.
*         if it is trunk member the function retrieve the trunkId of the trunk.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] memberPtr                - (pointer to) the member to check if is trunk member
*
* @param[out] trunkIdPtr               - (pointer to) trunk id of the port .
*                                      this pointer allocated by the caller.
*                                      this can be NULL pointer if the caller not require the
*                                      trunkId(only wanted to know that the member belongs to a
*                                      trunk)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_FOUND             - the pair (devNum,port) not a trunk member
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS sprvTgfTrunkDbIsMemberOfTrunk
(
    IN  GT_U8                   devNum,
    IN  CPSS_TRUNK_MEMBER_STC   *memberPtr,
    OUT GT_TRUNK_ID             *trunkIdPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
   GT_STATUS   rc;
   CPSS_TRUNK_MEMBER_STC localMember, *localMemberPtr;

   if( NULL != memberPtr)
   {
       localMember = *memberPtr;
       rc = extUtilHwDeviceNumberGet((GT_U8)memberPtr->hwDevice, &(localMember.hwDevice));
       if (GT_OK != rc)
       {
           PRV_TGF_LOG1_MAC(
               "[TGF]: prvUtfHwDeviceNumberGet FAILED, rc = [%d]", rc);

           return rc;
       }
       localMemberPtr = &localMember;
   }
   else
       localMemberPtr = NULL;

   /* call device specific API */
#ifdef EXMXPM_FAMILY
    rc = cpssExMxPmTrunkDbIsMemberOfTrunk(devNum, localMemberPtr , trunkIdPtr);
    #else
    rc = cpssDxChTrunkDbIsMemberOfTrunk(devNum, localMemberPtr , trunkIdPtr);
#endif
    if (GT_OK != rc && rc != GT_NOT_FOUND)
    {
#ifdef EXMXPM_FAMILY
       PRV_TGF_LOG1_MAC(
           "[TGF]: cpssExMxPmTrunkDbIsMemberOfTrunk FAILED, rc = [%d]", rc);
#else
       PRV_TGF_LOG1_MAC(
           "[TGF]: cpssDxChTrunkDbIsMemberOfTrunk FAILED, rc = [%d]", rc);
#endif
    }

    return rc;

#else
    devNum = devNum;
    memberPtr = memberPtr;
    trunkIdPtr = trunkIdPtr;

    return GT_BAD_STATE;
#endif
}

static GT_U32  allowPrint_not_expected_interface = 0;
GT_STATUS  allowPrint_not_expected_interface_checkPacketFromExpectedInterface(
    IN GT_U32   allowPrint
)
{
    allowPrint_not_expected_interface = allowPrint;
    return GT_OK;
}


/**
* @internal checkPacketFromExpectedInterface function
* @endinternal
*
* @brief   check if packet came from the expected interface (port/eport/trunk)
*
* @param[in] portInterfacePtr         - the expected interface info
* @param[in] rxParamPtr               - the info about origin of the packet
*
* @retval GT_TRUE                  - packet came from the expected interface
* @retval GT_FALSE                 - packet came from unexpected interface
*/
static GT_BOOL checkPacketFromExpectedInterface
(
    IN  CPSS_INTERFACE_INFO_STC         *portInterfacePtr,
    IN  TGF_NET_DSA_STC                 *rxParamPtr
)
{
    GT_STATUS   rc;
    GT_TRUNK_ID     trunkId;/* trunk Id for the packet came from port that is member of */
    CPSS_TRUNK_MEMBER_STC   trunkMember;/* temporary trunk member */
    GT_U32  portNum;

    /* check the DSA tag */
    if (GT_FALSE == rxParamPtr->dsaCmdIsToCpu)
    {
        if(allowPrint_not_expected_interface)
        {
            cpssOsPrintf("packet came without 'TO_CPU' dsa format \n");
        }
        /* the packet NOT came from where I expected it to be */
        return GT_FALSE;
    }

    if(GT_TRUE == rxParamPtr->srcIsTrunk)
    {
        if(TGF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfCpuDevNum) == GT_TRUE)
        {
            /* the eDSA tag supports the 'Physical port' regardless to the trunk indication */
            /* so continue ... */
        }
        else
        {
            if(allowPrint_not_expected_interface)
            {
                cpssOsPrintf("packet came from trunk but expected port \n");
            }
            /* the packet NOT came from where I expected it to be */
            return GT_FALSE;
        }
    }


    if(portInterfacePtr == NULL)
    {
        /* interface is not a filter issue */
    }
    else
    if(portInterfacePtr->type == CPSS_INTERFACE_PORT_E)
    {
        GT_BOOL usePhyPort;
        GT_U32 sip5_numBitsPhysicalPortInToCpuDsa_mask = 0xFF;/* 8 bits */

#ifdef CHX_FAMILY
        if(rxParamPtr->cpuCode == CPSS_NET_CPU_TO_CPU_E)
        {
            /* the packet came from other CPU ... the caller is not expected to know the 'srcDev' */
            if(GT_FALSE == PRV_CPSS_SIP_5_CHECK_MAC(prvTgfCpuDevNum))
            {
                /* sip 4 : the port num on the DSA is coming '0' ?! ...
                   in sip5 it comes 63 ! */
                if(rxParamPtr->portNum == 0)
                {
                    /* the packet came from where I 'expected' it to be */
                    return GT_TRUE;
                }
            }
            else
            {
                /* sip5  : the port num will be checked ... (should be '0x3f' ... the CPU port)       */
            }
        }
        else
#endif /*CHX_FAMILY*/
        if (rxParamPtr->hwDevNum != portInterfacePtr->devPort.hwDevNum)
        {
            if(allowPrint_not_expected_interface)
            {
                cpssOsPrintf("packet came with hwDevNum[%d] but expected [%d] \n",
                    rxParamPtr->hwDevNum ,
                    portInterfacePtr->devPort.hwDevNum);
            }
            return GT_FALSE;
        }
        if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfCpuDevNum))
        {
            sip5_numBitsPhysicalPortInToCpuDsa_mask = 0x3FF;/* 10 bits */
        }
        else if(PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfCpuDevNum))
        {
            sip5_numBitsPhysicalPortInToCpuDsa_mask = 0x1FF;/* 9 bits */
        }

        if(forceCompareAsPhysicalPort)
        {
            /* force compare as physical port */
            usePhyPort = GT_TRUE;
        }
        else
        if(comparePortAsEPort)
        {
            usePhyPort = GT_FALSE;
        }
        else
        {
            usePhyPort = GT_TRUE;
        }

        if(usePhyPort == GT_TRUE)
        {
            GT_PHYSICAL_PORT_NUM  expectedPortNum = portInterfacePtr->devPort.portNum;

            portNum = rxParamPtr->portNum;

#ifdef CHX_FAMILY
            if(expectedPortNum == CPSS_CPU_PORT_NUM_CNS && /* support 'wild card' CPU port number */
               txQueueNum >= 8)                            /* even for queues 8..max */
            {
                /* the test may send packet from the 'CPU' but not from queue 0..7
                   that relate to 'port 63' ..
                   so expected port may differ from 63  */
                rc = cpssDxChNetIfSdmaQueueToPhysicalPortGet(prvTgfCpuDevNum,
                    txQueueNum,
                    &expectedPortNum);
                if(rc != GT_OK)
                {
                    if(allowPrint_not_expected_interface)
                    {
                        cpssOsPrintf("error getting mapping txQueueNum [%d] to 'cpu port' number \n",
                            txQueueNum);
                    }
                    return GT_FALSE;
                }
            }
#endif

            if(portNum != (expectedPortNum/*phyPort*/ & sip5_numBitsPhysicalPortInToCpuDsa_mask))
            {
                if(allowPrint_not_expected_interface)
                {
                    cpssOsPrintf("packet came with portNum[%d] but expected [%d] (mask of [0x%x] on expected)\n",
                        portNum ,
                        expectedPortNum,
                        sip5_numBitsPhysicalPortInToCpuDsa_mask);
                }

                /* the packet NOT came from where I expected it to be */
                return GT_FALSE;
            }

        }
        else
        {
            portNum = rxParamPtr->ePortNum;

            if(portNum != (portInterfacePtr->devPort.portNum/*eport*/))
            {
                /* the packet NOT came from where I expected it to be */
                if(allowPrint_not_expected_interface)
                {
                    cpssOsPrintf("packet came with portNum[%d] but expected [%d] \n",
                        portNum ,
                        portInterfacePtr->devPort.portNum);
                }

                return GT_FALSE;
            }
        }

    }
    else /* interface is trunk */
    {
        if(GT_TRUE == rxParamPtr->srcIsTrunk)/* this can be only in eDSA tag */
        {
            /* the eDSA tag supports the 'Physical port' regardless to the trunk indication */
            if (portInterfacePtr->trunkId != rxParamPtr->trunkId)
            {
                if(allowPrint_not_expected_interface)
                {
                    cpssOsPrintf("packet came with trunkId[%d] but expected [%d] \n",
                        rxParamPtr->trunkId ,
                        portInterfacePtr->trunkId);
                }
                /* packet not from the needed trunk */
                /* the packet NOT came from where I expected it to be */
                return GT_FALSE;
            }
        }
        else
        {
            /* the trunk members still send traffic to CPU with the portNum ! */
            trunkMember.port = rxParamPtr->portNum;
            trunkMember.hwDevice = rxParamPtr->hwDevNum;

            rc = sprvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&trunkId);
            if(rc == GT_OK)
            {
                /* the packet came from port that is member of trunk */

                if(trunkId != portInterfacePtr->trunkId)
                {
                    if(allowPrint_not_expected_interface)
                    {
                        cpssOsPrintf("packet came with {dev[%d],port[%d]} in trunkId[%d] but expected trunkId[%d] \n",
                            trunkMember.port,
                            trunkMember.hwDevice,
                            trunkId ,
                            portInterfacePtr->trunkId);
                    }
                    /* packet not from the needed trunk */
                    /* the packet NOT came from where I expected it to be */
                    return GT_FALSE;
                }
            }
            else
            {
                if(allowPrint_not_expected_interface)
                {
                    cpssOsPrintf("packet not from a trunk (expected trunkId[%d])\n",
                        portInterfacePtr->trunkId);
                }
                /* packet not from a trunk */
                /* the packet NOT came from where I expected it to be */
                return GT_FALSE;
            }
        }
    }

    return GT_TRUE;
}

/**
* @internal tgfTrafficGeneratorPortTxEthTriggerCountersGet function
* @endinternal
*
* @brief   Get number of triggers on port/trunk
*         when 'trunk' implementation actually loops on all enabled members.
* @param[in] portInterfacePtr         - (pointer to) port/trunk interface
* @param[in] numVfd                   - number of VFDs
* @param[in] vfdArray[]               - pointer to array of VFDs
*
* @param[out] numTriggersBmpPtr        - (pointer to) bitmap of triggers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The 'captured' (emulation of capture) must be set on this port/trunk
*       see tgfTrafficGeneratorPortTxEthCaptureSet
*       Triggers bitmap has the following structure:
*       N - number of captured packets on this port
*       K - number of VFDs (equal to numVfd)
*       |   Packet0   |   Packet1   |   |    PacketN  |
*       +----+----+---+----+----+----+---+----+ ... +----+----+---+----+
*       |Bit0|Bit1|...|BitK|Bit0|Bit1|...|BitK|   |Bit0|Bit1|...|BitK|
*       +----+----+---+----+----+----+---+----+   +----+----+---+----+
*       Bit[i] for Packet[j] set to 1 means that VFD[i] is matched for captured
*       packet number j.
*
*/
GT_STATUS tgfTrafficGeneratorPortTxEthTriggerCountersGet
(
    IN  CPSS_INTERFACE_INFO_STC         *portInterfacePtr,
    IN  GT_U32                           numVfd,
    IN  TGF_VFD_INFO_STC                 vfdArray[],
    OUT GT_U32                          *numTriggersBmpPtr
)
{
    GT_U8           devNum   = 0;
    GT_U8           queue    = 0;
    GT_BOOL         getFirst = GT_TRUE;
    GT_U32          i        = 0;
    GT_BOOL         match    = GT_FALSE;
    GT_STATUS       rc       = GT_OK;
    GT_STATUS       rc1      = GT_OK;
    GT_U32          buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32          packetActualLength = 0;
    GT_U32          packetCount = 0;
    GT_U8           packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    TGF_NET_DSA_STC rxParam = {0};
#ifdef GM_USED
    GT_U32          maxWaitTime     = 1200; /* first packet wait time */
    GT_U32          nextWaitTime    = 200;
    GT_U32          delayTime       = 100;
#endif /*GM_USED*/

#ifdef GM_USED
    if(PRV_CPSS_PP_MAC(prvTgfCpuDevNum)->devFamily == CPSS_PP_FAMILY_PUMA3_E)
    {
        maxWaitTime     = prvTgfGmTimeFactor*100; /* first packet wait time */
        nextWaitTime    = prvTgfGmTimeFactor*50;
        delayTime       = prvTgfGmTimeFactor*20;
    }
#endif /*GM_USED*/


    CPSS_NULL_PTR_CHECK_MAC(portInterfacePtr);
    CPSS_NULL_PTR_CHECK_MAC(vfdArray);
    CPSS_NULL_PTR_CHECK_MAC(numTriggersBmpPtr);

    if (numVfd == 0)
    {
        return GT_BAD_PARAM;
    }

    if (portInterfacePtr->type != CPSS_INTERFACE_PORT_E &&
        portInterfacePtr->type != CPSS_INTERFACE_TRUNK_E)

    {
        return GT_BAD_PARAM;
    }

    (*numTriggersBmpPtr) = 0;

    while(1)
    {
        buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
        /* get entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E,
                                           getFirst, GT_TRUE, packetBuff,
                                           &buffLen, &packetActualLength,
                                           &devNum, &queue, &rxParam);
#ifdef GM_USED
        /* wait for yet not reached packets               */
        if ((rc == GT_NO_MORE) && (maxWaitTime >= delayTime))
        {
            maxWaitTime -= delayTime;
            cpssOsTimerWkAfter(delayTime);
            continue;
        }
        /* for the fist packet we are ready to wait       */
        /* sevelal delays, for the nexts no more then one */
        maxWaitTime = nextWaitTime;
#endif /*GM_USED*/

        if (rc != GT_OK)
        {
            break;
        }

        getFirst = GT_FALSE; /* now we get the next */

        if(GT_FALSE ==
            checkPacketFromExpectedInterface(portInterfacePtr,&rxParam))
        {
            /* packet not came from expected interface */
            continue;
        }

        /* calculate number of captured packets */
        packetCount++;

        /* check the VFD parameters */
        for (i = 0; i < numVfd ;i++)
        {
            rc = tgfTrafficEnginePacketVfdCheck(
                &vfdArray[i], packetBuff, buffLen, &match);

            /* set appropriate bit to 1 if VFD is matched */
            *numTriggersBmpPtr |= ((GT_U32) match) << (i + (packetCount - 1) * numVfd);
        }
    }

    /* restore default */
    tgfTrafficGeneratorExpectedInterfaceEport(PRV_TGF_EXPECTED_INTERFACE_TYPE_PHYSICAL_PORT_NUM_E);

    return (GT_OK == rc1) ? rc : rc1;
}

/**
* @internal tgfTrafficGeneratorTxEthTriggerCheck function
* @endinternal
*
* @brief   Check if the buffer triggered by the trigger parameters
*
* @param[in] bufferPtr                - (pointer to) the buffer
* @param[in] bufferLength             - length of the buffer
* @param[in] numVfd                   - number of VFDs
* @param[in] vfdArray[]               - array of VFDs
*
* @param[out] triggeredPtr             - (pointer to) triggered / not triggered
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The 'captured' (emulation of capture) must be set on this port see
*       tgfTrafficGeneratorPortTxEthCaptureSet, tgfTrafficGeneratorRxInCpuGet
*
*/
GT_STATUS tgfTrafficGeneratorTxEthTriggerCheck
(
    IN  GT_U8                           *bufferPtr,
    IN  GT_U32                           bufferLength,
    IN  GT_U32                           numVfd,
    IN  TGF_VFD_INFO_STC                 vfdArray[],
    OUT GT_BOOL                         *triggeredPtr
)
{
    GT_BOOL     match = GT_FALSE;
    GT_U32      i     = 0;
    GT_STATUS   rc    = GT_OK;

    CPSS_NULL_PTR_CHECK_MAC(vfdArray);
    CPSS_NULL_PTR_CHECK_MAC(triggeredPtr);

    if(numVfd == 0)
    {
        return GT_BAD_PARAM;
    }

    *triggeredPtr = GT_FALSE;

    /* check the VFD parameters */
    for(i = 0; i < numVfd ;i++)
    {
        rc = tgfTrafficEnginePacketVfdCheck(&vfdArray[i], bufferPtr, bufferLength, &match);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(match != GT_TRUE)
        {
            break;
        }
    }

    if(i == numVfd)
    {
        *triggeredPtr = GT_TRUE;
    }

    return rc;
}

#define NOT_VALID_CNS   0xFFFFFFFF

#ifdef CHX_FAMILY
typedef struct{
    GT_U8           devNum;
    GT_U32          portNum;
    GT_U32          bmpLookupEnabled;
}MY_CAPTURE_TTI_PORT_INFO_STC;
#define MAX_CAPTURE_PORTS_CNS   8
static MY_CAPTURE_TTI_PORT_INFO_STC captureTtiPortInfoArr[MAX_CAPTURE_PORTS_CNS];
static GT_BOOL initDone_captureDevPortArr = GT_FALSE;

static GT_U32   ttiLookupsArr_xcat[] = {
     CPSS_DXCH_TTI_KEY_IPV4_E
    ,CPSS_DXCH_TTI_KEY_MPLS_E
    ,CPSS_DXCH_TTI_KEY_ETH_E
    ,CPSS_DXCH_TTI_KEY_MIM_E
    ,NOT_VALID_CNS
};
static GT_U32   ttiLookupsArr_eArch[] = {
     CPSS_DXCH_TTI_KEY_IPV4_E
    ,CPSS_DXCH_TTI_KEY_MPLS_E
    ,CPSS_DXCH_TTI_KEY_ETH_E
    ,CPSS_DXCH_TTI_KEY_MIM_E
    ,CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E
    ,CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E
    ,CPSS_DXCH_TTI_KEY_UDB_MPLS_E
    ,CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E
    ,CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E
    ,CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E
    ,CPSS_DXCH_TTI_KEY_UDB_IPV6_E
    ,CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E
    ,CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E
    ,CPSS_DXCH_TTI_KEY_UDB_UDE_E
    ,CPSS_DXCH_TTI_KEY_UDB_UDE1_E
    ,CPSS_DXCH_TTI_KEY_UDB_UDE2_E
    ,CPSS_DXCH_TTI_KEY_UDB_UDE3_E
    ,CPSS_DXCH_TTI_KEY_UDB_UDE4_E
    ,CPSS_DXCH_TTI_KEY_UDB_UDE5_E
    ,CPSS_DXCH_TTI_KEY_UDB_UDE6_E
    ,NOT_VALID_CNS
};

/**
* @internal prvTgfCaptureSet_ttiLookupInfo function
* @endinternal
*
* @brief   disable any TTI lookups on this physical port ... to remove option that
*         it will ignore the 'mirror'/'ipcl' configurations
*         or
*         restore the TTI lookups on this physical port
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] start                    - start/stop capture
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfCaptureSet_ttiLookupInfo
(
    IN GT_U8    devNum,
    IN GT_PORT_NUM   portNum,
    IN GT_BOOL  start
)
{
    GT_STATUS rc;
    GT_U32  ii,jj;
    GT_U32  *arrayPtr = NULL;
    GT_U32  bmpLookupEnabled = 0;
    GT_BOOL enabled;
    GT_U32  firstFreeIndex = NOT_VALID_CNS;

    if(initDone_captureDevPortArr == GT_FALSE)
    {
        for(ii = 0 ; ii < MAX_CAPTURE_PORTS_CNS ; ii++)
        {
            captureTtiPortInfoArr[ii].devNum = 0;
            captureTtiPortInfoArr[ii].portNum  = NOT_VALID_CNS;
            captureTtiPortInfoArr[ii].bmpLookupEnabled   = 0;
        }
        initDone_captureDevPortArr = GT_TRUE;
    }

    for(ii = 0 ; ii < MAX_CAPTURE_PORTS_CNS ; ii++)
    {
        if (start == GT_TRUE)
        {
            if(captureTtiPortInfoArr[ii].portNum == NOT_VALID_CNS && firstFreeIndex == NOT_VALID_CNS)
            {
                firstFreeIndex = ii;
            }

            if(captureTtiPortInfoArr[ii].portNum == portNum &&
               captureTtiPortInfoArr[ii].devNum == devNum)
            {
                /* the port already exists ?! --> ERROR in the test !!!
                   ... but it seems that there are several of those tests already !!! */
                PRV_TGF_LOG1_MAC("TGF: WARNING (potential Error) : The 'capture' on port [%d] is 'start' again without 'stop' after 'start' of previous time \n",
                    portNum);

                /* assume that we already disabled the tti lookups ... and we
                   will 'restore' it to proper values at the 'stop' */
                return GT_FAIL;
            }
        }
        else
        if (start == GT_FALSE &&
            captureTtiPortInfoArr[ii].portNum == portNum &&
            captureTtiPortInfoArr[ii].devNum == devNum)
        {
            break;
        }
    }

    if (ii == MAX_CAPTURE_PORTS_CNS)
    {
        if(start == GT_TRUE)
        {
            if (firstFreeIndex != NOT_VALID_CNS)
            {
                ii = firstFreeIndex;
            }
            else
            {
                cpssOsPrintf("TGF: The capture tti support table is full \n");
                return GT_FULL;
            }
        }
        else
        {
            cpssOsPrintf("TGF: WARNING (potential Error) : The capture tti support table not found port [%d] \n",
                portNum);

            /* assume that someone try to disabled port that already disabled*/
            return GT_FAIL;
        }
    }

    if(TGF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_FALSE)
    {
        arrayPtr = ttiLookupsArr_xcat;
    }
    else
    {
        arrayPtr = ttiLookupsArr_eArch;
    }

    if(start == GT_TRUE)
    {
        for( jj = 0 ; arrayPtr[jj] != NOT_VALID_CNS ; jj++)
        {
            /* save the lookup value */
            rc = cpssDxChTtiPortLookupEnableGet(devNum,portNum,
                (CPSS_DXCH_TTI_KEY_TYPE_ENT)arrayPtr[jj],
                &enabled);
            if (rc == GT_NOT_APPLICABLE_DEVICE)
            {
                /* for devices that does not support TTI lookup */
                enabled = GT_FALSE;
            }
            else if (rc != GT_OK)
            {
                return rc;
            }

            if(enabled == GT_TRUE)
            {
                bmpLookupEnabled |= 1 << arrayPtr[jj];

                /* disable the lookup value */
                rc = cpssDxChTtiPortLookupEnableSet(devNum,portNum,
                    (CPSS_DXCH_TTI_KEY_TYPE_ENT)arrayPtr[jj],
                    GT_FALSE);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }

        /* save to DB */
        captureTtiPortInfoArr[ii].devNum = devNum;
        captureTtiPortInfoArr[ii].portNum  = portNum;
        captureTtiPortInfoArr[ii].bmpLookupEnabled   = bmpLookupEnabled;
    }
    else
    {
        bmpLookupEnabled = captureTtiPortInfoArr[ii].bmpLookupEnabled;

        /* restore values */
        for( jj = 0 ; arrayPtr[jj] != NOT_VALID_CNS ; jj++)
        {
            if(bmpLookupEnabled & (1 << arrayPtr[jj]))
            {
                /* re-enable the lookup value */
                rc = cpssDxChTtiPortLookupEnableSet(devNum,portNum,
                    (CPSS_DXCH_TTI_KEY_TYPE_ENT)arrayPtr[jj],
                    GT_TRUE);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }

        captureTtiPortInfoArr[ii].devNum = 0;
        captureTtiPortInfoArr[ii].portNum  = NOT_VALID_CNS;
        captureTtiPortInfoArr[ii].bmpLookupEnabled   = 0;
    }


    return GT_OK;
}


typedef struct{
    GT_U8           devNum;
    GT_U32          portNum;
    /* tagging info */
    GT_U32          orig_tpid_profile[2];/* original - port's profile of tag0,tag1 */

}MY_CAPTURE_TPID_PORT_INFO_STC;
static MY_CAPTURE_TPID_PORT_INFO_STC captureTpidPortInfoArr[MAX_CAPTURE_PORTS_CNS];
static GT_BOOL initDone_captureTpidPortInfoArr = GT_FALSE;

static GT_U32 captureTpid_orig_profile_bmp[2];/* original - bmp of the profiles of tag0,tag1 */
static GT_U32 captureTpid_unusedTpidProfile[2];/* original - the profiles of tag0,tag1 that point to bmp of 'ZEROes'*/

/* The 'capture' need to disable the TPID recognition.
   by default we use profile 6 to hold bmp of 0 (both for tag0 and for tag 1)
   using profile 6 like LUA tests uses
*/
#define DEFAULT_TPID_WITH_ZERO_BMP_CNS      6
/* by default we enable the disabling of TPIDs for captured packet ...
   so we will get in the CPU the original EtherType_0 */
static GT_U32   prvTgfCaptureTpidWithZeroBmp = DEFAULT_TPID_WITH_ZERO_BMP_CNS;

void prvTgfCaptureTpidWithZeroBmpDisable(void)
{
    prvTgfCaptureTpidWithZeroBmp = 0xFFFFFFFF;
}
void prvTgfCaptureTpidWithZeroBmpRestore(void)
{
    prvTgfCaptureTpidWithZeroBmp = DEFAULT_TPID_WITH_ZERO_BMP_CNS;
}

/* make sure that one test not impact other test */
void prvTgfCaptureSet_ingressTagging_reset(void)
{
    initDone_captureTpidPortInfoArr = GT_FALSE;
    prvTgfCaptureTpidWithZeroBmpRestore();
}

static GT_STATUS prvTgfCaptureSet_ingressTagging_init(void)
{
    GT_U32 dbIndex;
    if(initDone_captureTpidPortInfoArr == GT_FALSE)
    {
        for(dbIndex = 0 ; dbIndex < MAX_CAPTURE_PORTS_CNS ; dbIndex++)
        {
            captureTpidPortInfoArr[dbIndex].portNum = NOT_VALID_CNS;
            captureTpidPortInfoArr[dbIndex].devNum  = 0;
        }

        captureTpid_orig_profile_bmp[0] = NOT_VALID_CNS;
        captureTpid_orig_profile_bmp[1] = NOT_VALID_CNS;
        captureTpid_unusedTpidProfile[0] = NOT_VALID_CNS;
        captureTpid_unusedTpidProfile[1] = NOT_VALID_CNS;

        initDone_captureTpidPortInfoArr = GT_TRUE;
    }
    return GT_OK;
}

/**
* @internal prvTgfCaptureSet_ingressTagging_values function
* @endinternal
*
* @brief   disable/restore vlan tag (TPID) recognition on the packet so tagged packet will
*         be trapped to CPU without loosing the original etherType
* @param[in] devNum                   - device number
* @param[in] start                    - start/stop capture
* @param[in] ethMode                  - etherTypeMode (tag 0 / tag 1)
* @param[in] unusedTpidProfile        - the TPID profile index to use
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfCaptureSet_ingressTagging_values
(
    IN GT_U8    devNum,
    IN GT_BOOL  start,
    IN CPSS_ETHER_MODE_ENT ethMode,
    IN GT_U32   unusedTpidProfile
)
{
    GT_STATUS   rc;
    GT_U32  tpidBmp = 0;
    static  GT_U32  myCounter = 0;

    if(!PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        /* not implemented */
        return GT_OK;
    }

    prvTgfCaptureSet_ingressTagging_init();

    if(start == GT_TRUE)
    {
        if((ethMode-CPSS_VLAN_ETHERTYPE0_E) >= 2)
        {
            return GT_BAD_PARAM;
        }

        rc = cpssDxChBrgVlanIngressTpidProfileGet(devNum,
            unusedTpidProfile,ethMode,&tpidBmp);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* save to the DB */
        captureTpid_orig_profile_bmp[ethMode-CPSS_VLAN_ETHERTYPE0_E] = tpidBmp;
        captureTpid_unusedTpidProfile[ethMode-CPSS_VLAN_ETHERTYPE0_E] = unusedTpidProfile;

        /* set bmp to not recognize any TPID */
        tpidBmp = 0;
        rc = cpssDxChBrgVlanIngressTpidProfileSet(devNum,
            unusedTpidProfile,ethMode,tpidBmp);
        if(rc != GT_OK)
        {
            return rc;
        }

        myCounter++;
    }
    else
    {
        if(myCounter)
        {
            myCounter--;
        }

        if(myCounter != 0)
        {
            /* we can not restore yet the values */
            return GT_OK;
        }

        /* restore the bmp */
        tpidBmp = captureTpid_orig_profile_bmp[ethMode-CPSS_VLAN_ETHERTYPE0_E];

        rc = cpssDxChBrgVlanIngressTpidProfileSet(devNum,
            unusedTpidProfile,ethMode,tpidBmp);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* clear the DB */
        captureTpid_orig_profile_bmp[ethMode-CPSS_VLAN_ETHERTYPE0_E] = NOT_VALID_CNS;
        captureTpid_unusedTpidProfile[ethMode-CPSS_VLAN_ETHERTYPE0_E] = NOT_VALID_CNS;
    }

    return GT_OK;

}

/**
* @internal prvTgfCaptureSet_ingressTagging function
* @endinternal
*
* @brief   disable/restore vlan tag (TPID) recognition on the packet so tagged packet will
*         be trapped to CPU without loosing the original etherType
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] start                    - start/stop capture
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfCaptureSet_ingressTagging
(
    IN GT_U8    devNum,
    IN GT_PORT_NUM   portNum,
    IN GT_BOOL  start
)
{
    GT_STATUS rc;
    CPSS_ETHER_MODE_ENT ethMode;
    GT_U32  profile,oldProfile;
    GT_U32  dbIndex = 0;
    GT_U32  firstFreeIndex = NOT_VALID_CNS;
    GT_U32  ii;

    if(!PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        /* not implemented */
        return GT_OK;
    }

    /* make sure init was done */
    prvTgfCaptureSet_ingressTagging_init();

    if(captureTpid_orig_profile_bmp[0] == NOT_VALID_CNS &&
       captureTpid_orig_profile_bmp[1] == NOT_VALID_CNS)
    {
        return GT_OK;
    }

    for(ii = 0 ; ii < MAX_CAPTURE_PORTS_CNS ; ii++)
    {
        if (start == GT_TRUE)
        {
            if(captureTpidPortInfoArr[ii].portNum == NOT_VALID_CNS && firstFreeIndex == NOT_VALID_CNS)
            {
                firstFreeIndex = ii;
            }

            if(captureTpidPortInfoArr[ii].portNum == portNum &&
               captureTpidPortInfoArr[ii].devNum == devNum)
            {
                /* the port already exists ?! --> ERROR in the test !!!
                   ... but it seems that there are several of those tests already !!! */
                PRV_TGF_LOG1_MAC("TGF: WARNING (potential Error) : The 'capture' on port [%d] is 'start' again without 'stop' after 'start' of previous time \n",
                    portNum);

                /* assume that we already disabled the 'TPID recognition' ... and we
                   will 'restore' it to proper values at the 'stop' */
                return GT_FAIL;
            }
        }
        else
        if (start == GT_FALSE &&
            captureTpidPortInfoArr[ii].portNum == portNum &&
            captureTpidPortInfoArr[ii].devNum == devNum)
        {
            break;
        }
    }

    if (ii == MAX_CAPTURE_PORTS_CNS)
    {
        if(start == GT_TRUE)
        {
            if (firstFreeIndex != NOT_VALID_CNS)
            {
                dbIndex = firstFreeIndex;
            }
            else
            {
                cpssOsPrintf("TGF: The capture 'TPID recognition' support table is full \n");
                return GT_FULL;
            }
        }
        else
        {
            cpssOsPrintf("TGF: WARNING (potential Error) : The capture 'TPID recognition' support table not found port [%d] \n",
                portNum);

            /* assume that someone try to disabled port that already disabled*/
            return GT_FAIL;
        }
    }
    else
    {
        dbIndex = ii;
    }


    for(ethMode  = CPSS_VLAN_ETHERTYPE0_E ;
        ethMode <= CPSS_VLAN_ETHERTYPE1_E ;
        ethMode++)
    {
        if(captureTpid_orig_profile_bmp[ethMode-CPSS_VLAN_ETHERTYPE0_E] == NOT_VALID_CNS)
        {
            /* no manipulation needed */
            continue;
        }

        if(start == GT_TRUE)
        {
            rc = cpssDxChBrgVlanPortIngressTpidProfileGet(devNum,portNum,
                ethMode,GT_TRUE,/*isDefaultProfile*/
                &oldProfile);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* save to the DB the TPID profile to restore */
            captureTpidPortInfoArr[dbIndex].orig_tpid_profile[ethMode-CPSS_VLAN_ETHERTYPE0_E] = oldProfile;

            /* new profile */
            profile = captureTpid_unusedTpidProfile[ethMode-CPSS_VLAN_ETHERTYPE0_E];
        }
        else
        {
            /* restore */
            profile = captureTpidPortInfoArr[dbIndex].orig_tpid_profile[ethMode-CPSS_VLAN_ETHERTYPE0_E];
        }

        rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum,portNum,
            ethMode,GT_TRUE,/*isDefaultProfile*/
            profile);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(start == GT_TRUE)
    {
        captureTpidPortInfoArr[dbIndex].portNum = portNum ;
        captureTpidPortInfoArr[dbIndex].devNum  = devNum;
    }
    else
    {
        captureTpidPortInfoArr[dbIndex].portNum = NOT_VALID_CNS ;
        captureTpidPortInfoArr[dbIndex].devNum  = 0;
    }


    return GT_OK;
}

/**
* @internal prvTgfDxChPclCaptureSet function
* @endinternal
*
* @brief   Start/Stop capture the traffic that egress the port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] start                    - start/stop capture
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The Start 'Capture' is emulated by setting next for the port:
*       1. ingress PCL rule for ALL traffic from this port to TRAP to the CPU
*       we will assign it with 'Application specific cpu code' to
*       distinguish from other traffic go to the CPU
*       The Stop 'Capture' is emulated by setting next for the port:
*       1. remove the ingress PCL rule that function
*       tgfTrafficGeneratorPortTxEthCaptureStart added
*
*/
GT_STATUS prvTgfDxChPclCaptureSet
(
    IN GT_U8    devNum,
    IN GT_PORT_NUM   portNum,
    IN GT_BOOL  start
)
{
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    GT_U32                             ruleIndex  = 0;
    GT_STATUS                          rc         = GT_OK;
    static GT_U32                      first      = 1;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT      mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT      pattern;
    CPSS_DXCH_PCL_ACTION_STC           action;
    CPSS_INTERFACE_INFO_STC            portInterface;
    CPSS_DXCH_PCL_LOOKUP_CFG_STC       lookupCfg;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT       tcamRuleSize;

    PRV_CPSS_DXCH_PCL_CONVERT_RULE_FORMAT_TO_TCAM_RULE_SIZE_VAL_MAC(tcamRuleSize,ruleFormat);

    if(GT_TRUE == start)
    {
        if(first)
        {
            /* get default Ingress Policy state */
            rc = cpssDxChPclIngressPolicyEnableGet(devNum, &prvTgfDefIngressPolicyState);
            if (GT_OK != rc)
            {
                return rc;
            }

            /* enable Ingress Policy */
            rc = cpssDxChPclIngressPolicyEnable(devNum, GT_TRUE);
            if (GT_OK != rc)
            {
                return rc;
            }

            TGF_RESET_PARAM_MAC(mask);
            TGF_RESET_PARAM_MAC(pattern);
            TGF_RESET_PARAM_MAC(action);

            /* prepare rule index */

            ruleIndex = TGF_DXCH_TCAM_PCL_CONVRT_IDX_GET_MAC(devNum,
                                                                   TGF_PCL_RULE_INDEX_CNS,
                                                                   tcamRuleSize);

            /* need to match Only the PCL id */
            mask.ruleStdNotIp.common.pclId    = 0x3FF;/*10 bits exact match on this field */
            pattern.ruleStdNotIp.common.pclId = TGF_CAPTURE_PCL_ID_CNS;

            /* trap to CPU */
            action.pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

            /* set specific CPU code for those 'captured' packets */
            action.mirror.cpuCode = CPSS_NET_LAST_USER_DEFINED_E;

            /* add the rule to the PCL */
            rc = cpssDxChPclRuleSet(
                devNum, ruleFormat, ruleIndex, 0 /*ruleOptionsBmp*/,
                &mask, &pattern, &action);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(pclCaptureExcludeVidEnabled == GT_TRUE)
            {
                /* we have single VLAN that we not want capture to the CPU*/
                mask.ruleStdNotIp.common.vid    = 0xFFF;/*exact match on this field */
                pattern.ruleStdNotIp.common.vid = pclCaptureExcludeVid;

                cpssOsMemSet(&action,0,sizeof(action));
                action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
                action.redirect.redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E;

                /* prepare rule index - 1*/
                ruleIndex = TGF_DXCH_TCAM_PCL_CONVRT_IDX_GET_MAC(devNum,
                                                                       TGF_PCL_RULE_INDEX_CNS - 1,
                                                                       tcamRuleSize);

                /* add the rule to the PCL -- in order prior to  the 'trap' rule ! */
                rc = cpssDxChPclRuleSet(
                    devNum, ruleFormat, ruleIndex, 0 /*ruleOptionsBmp*/,
                    &mask, &pattern, &action);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }

            if(portNum < PRV_CPSS_DXCH_MAX_DEFAULT_EPORT_NUMBER_MAC(devNum))
            {
                rc = cpssDxChPclPortLookupCfgTabAccessModeSet(
                        devNum, portNum,
                        CPSS_PCL_DIRECTION_INGRESS_E,
                        CPSS_PCL_LOOKUP_0_E,
                        0,
                        CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }

            first = 0;
        }
    }
    else
    {
        /* restore default Ingress Policy state */
        rc = cpssDxChPclIngressPolicyEnable(devNum, prvTgfDefIngressPolicyState);
        if (GT_OK != rc)
        {
            return rc;
        }

        ruleIndex = TGF_DXCH_TCAM_PCL_CONVRT_IDX_GET_MAC(devNum,
                                                               TGF_PCL_RULE_INDEX_CNS,
                                                               tcamRuleSize);

        /* invalidate PCL rule */
        rc = cpssDxChPclRuleValidStatusSet(devNum, CPSS_PCL_RULE_SIZE_STD_E,
                                           ruleIndex, GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(pclCaptureExcludeVidEnabled == GT_TRUE)
        {
            /* prepare rule index - 1*/
            ruleIndex = TGF_DXCH_TCAM_PCL_CONVRT_IDX_GET_MAC(devNum,
                                                                   TGF_PCL_RULE_INDEX_CNS - 1,
                                                                   tcamRuleSize);

            /* invalidate PCL rule */
            rc = cpssDxChPclRuleValidStatusSet(devNum, CPSS_PCL_RULE_SIZE_STD_E,
                                               ruleIndex, GT_FALSE);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        first = 1;
    }

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.portNum = portNum;
    portInterface.devPort.hwDevNum  = devNum;

    TGF_RESET_PARAM_MAC(lookupCfg);

    lookupCfg.pclId        = TGF_CAPTURE_PCL_ID_CNS;
    lookupCfg.groupKeyTypes.nonIpKey = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    /* start - enable lookup, stop - disable lookup */
    lookupCfg.enableLookup = start;

    rc = extUtilHwDeviceNumberGet(portInterface.devPort.hwDevNum,
                                 &portInterface.devPort.hwDevNum);
    if (GT_OK != rc)
    {
        return rc;
    }

    if(portNum < PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum))
    {
        rc = cpssDxChPclCfgTblSet(devNum, &portInterface,
                                  CPSS_PCL_DIRECTION_INGRESS_E,
                                  CPSS_PCL_LOOKUP_0_E,
                                  &lookupCfg);
        if (GT_OK != rc)
        {
            return rc;
        }
    }


    rc = cpssDxChPclPortIngressPolicyEnable(devNum, portNum, start);

    return rc;
}

/* flag allow to put conditional break point in cpssDxChBrgVlanPortVidSet to
   skip cases that called with pvid == 0*/
/* !! extern !!*/ GT_U32 setPvidFrom_prvTgfDxChMirroringCaptureSet = 0;

/**
* @internal prvTgfDxChMirroringCaptureSet function
* @endinternal
*
* @brief   Start/Stop capture the traffic that egress the port.
*         Temporary function for DxChXcat A1 that uses Mirroring for packet
*         capture, while PCL development in progress.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] start                    - start/stop capture
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The Start 'Capture' is emulated by setting next for the port:
*       1. Enable mirroring for this port.
*       2. Drop all packets by forcing PVID = 0.
*       3. Set CPU port as analyzer port.
*       The Stop 'Capture' is emulated by setting next for the port:
*       1. Disable mirroring for this port.
*       2. Disable Force PVID.
*
*/
static GT_STATUS prvTgfDxChMirroringCaptureSet
(
    IN GT_U8    devNum,
    IN GT_PORT_NUM   portNum,
    IN GT_BOOL  start
)
{
    GT_STATUS       rc = GT_OK, rc1;
    GT_U16          hwValue;
    CPSS_PP_FAMILY_TYPE_ENT  devFamily;
    CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC analyzerInf;
    GT_HW_DEV_NUM   hwDevNum;
    GT_U32  ii;

    /*******************************/
    /* Configure Mirroring to CPU  */
    /*******************************/

    if(GT_TRUE == start)
    {
        for(ii = 0 ; ii < MAX_MIRROR_PORTS_CNS; ii ++)
        {
            if(prvTgfGlobalPvidDatabaseArr[ii].isValid == GT_FALSE)
            {
                /* a port must be removed before added again */
                /* so we not look for it as 'Already exists' */

                /* give the port free index */
                break;
            }
        }

        if(ii == MAX_MIRROR_PORTS_CNS)
        {
            /* no free ports in DB */
            return GT_FULL;
        }

        /* get current PVID */
        rc = cpssDxChBrgVlanPortVidGet(devNum,
                                       portNum,
                                       CPSS_DIRECTION_INGRESS_E,
                                       &hwValue);
        if (GT_OK != rc)
        {
            return rc;
        }

        /* store current PVID value */
        prvTgfGlobalPvidDatabaseArr[ii].pvid = hwValue;
        prvTgfGlobalPvidDatabaseArr[ii].devNum = devNum;
        prvTgfGlobalPvidDatabaseArr[ii].portNum = portNum;
        prvTgfGlobalPvidDatabaseArr[ii].isValid  = GT_TRUE;

        setPvidFrom_prvTgfDxChMirroringCaptureSet = 1;

        /* set PVID = 0 for the port */
        rc = cpssDxChBrgVlanPortVidSet(devNum,
                                       portNum,
                                       CPSS_DIRECTION_INGRESS_E,
                                       0);

        setPvidFrom_prvTgfDxChMirroringCaptureSet = 0;
        if (GT_OK != rc)
        {
            return rc;
        }

        /* set force PVID for all packets in the port */
        rc = cpssDxChBrgVlanForcePvidEnable(devNum,
                                            portNum,
                                            GT_TRUE);
        if (GT_OK != rc)
        {
            return rc;
        }

        DEV_CHECK_MAC(devNum);
        devFamily = PRV_CPSS_PP_MAC(devNum)->devFamily;

        /* call device specific API */
        if (devFamily < CPSS_PP_FAMILY_DXCH_XCAT_E)
        {
            rc = extUtilHwDeviceNumberGet(devNum, &hwDevNum);
            if(GT_OK != rc)
                return rc;
            rc = cpssDxChMirrorRxAnalyzerPortSet(devNum,
                                                 CPSS_CPU_PORT_NUM_CNS,
                                                 hwDevNum);
        }
        else
        {
            analyzerInf.interface.type = CPSS_INTERFACE_PORT_E;

            rc = extUtilHwDeviceNumberGet(devNum,
                                         &analyzerInf.interface.devPort.hwDevNum);
            if(GT_OK != rc)
                return rc;
            analyzerInf.interface.devPort.portNum = CPSS_CPU_PORT_NUM_CNS;

            /* use interface 0 for Rx analyzer by default */
            rc = cpssDxChMirrorAnalyzerInterfaceSet(devNum, 0, &analyzerInf);
        }

        /* enable mirroring */
        rc = cpssDxChMirrorRxPortSet(devNum,
                                     portNum,
                                     GT_FALSE, /* use ePort for Lion2 */
                                     GT_TRUE,
                                     0);
        if (GT_OK != rc)
        {
            return rc;
        }
    }
    else
    {
        /* disable mirroring */
        rc1 = cpssDxChMirrorRxPortSet(devNum,
                                     portNum,
                                     GT_FALSE, /* use ePort for Lion2 */
                                     GT_FALSE,
                                     0);
        if (GT_OK != rc1)
        {
            rc = rc1;
        }

        /* disable force PVID */
        rc1 = cpssDxChBrgVlanForcePvidEnable(devNum,
                                            portNum,
                                            GT_FALSE);
        if (GT_OK != rc1)
        {
            rc = rc1;
        }

        for(ii = 0 ; ii < MAX_MIRROR_PORTS_CNS; ii ++)
        {
            if(prvTgfGlobalPvidDatabaseArr[ii].isValid == GT_FALSE)
            {
                /* look for the port in the DB */
                continue;
            }

            if(prvTgfGlobalPvidDatabaseArr[ii].portNum == portNum &&
               prvTgfGlobalPvidDatabaseArr[ii].devNum == devNum )
            {
                /* port was found */
                break;
            }
        }

        if(ii == MAX_MIRROR_PORTS_CNS)
        {
            /* not found ?! */
            return GT_NOT_FOUND;
        }

        hwValue = prvTgfGlobalPvidDatabaseArr[ii].pvid;

        prvTgfGlobalPvidDatabaseArr[ii].isValid = GT_FALSE;

        /* restore PVID */
        rc = cpssDxChBrgVlanPortVidSet(devNum,
                                       portNum,
                                       CPSS_DIRECTION_INGRESS_E,
                                       hwValue);
        if (GT_OK != rc1)
        {
            rc = rc1;
        }

    }

    return rc;
}
#else /*CHX_FAMILY*/
GT_STATUS tgfTrafficGeneratorPortTxEthCaptureSet
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN TGF_CAPTURE_MODE_ENT      mode,
    IN GT_BOOL                   start
)
{
    return GT_NOT_IMPLEMENTED;
}
GT_STATUS prvTgfCaptureSet_ingressTagging_values
(
    IN GT_U8    devNum,
    IN GT_BOOL  start,
    IN CPSS_ETHER_MODE_ENT ethMode,
    IN GT_U32   unusedTpidProfile
)
{
    return GT_NOT_IMPLEMENTED;
}
#endif /* CHX_FAMILY */

/* for debug force specific capture mode , with out changing the test */
static GT_BOOL              forceCaptureModeEnabled = GT_FALSE;
static TGF_CAPTURE_MODE_ENT forceCaptureMode = TGF_CAPTURE_MODE_PCL_E;
/* debug function to force capture mode */
extern GT_STATUS prvTgfCaptureForceModeEnableSet(
    IN GT_BOOL                  enable ,
    IN TGF_CAPTURE_MODE_ENT     forcedMode
)
{
    if(enable != GT_FALSE &&
        forcedMode != TGF_CAPTURE_MODE_MIRRORING_E &&
        forcedMode != TGF_CAPTURE_MODE_PCL_E)
    {
        return GT_BAD_PARAM;
    }

    cpssOsPrintf("The capture force mode is : [%s] (was [%s]) \n" ,
        enable                  == GT_FALSE ? "disabled" : "enabled",
        forceCaptureModeEnabled == GT_FALSE ? "disabled" : "enabled");

    if(enable != GT_FALSE)
    {
        cpssOsPrintf("New mode is : [%s] (was [%s]) \n" ,
            forcedMode       == TGF_CAPTURE_MODE_PCL_E ? "PCL" : "Rx Mirroring",
            forceCaptureMode == TGF_CAPTURE_MODE_PCL_E ? "PCL" : "Rx Mirroring");
    }

    forceCaptureModeEnabled = enable;
    forceCaptureMode        = forcedMode;

    return GT_OK;
}

#ifdef DXCH_CODE
static GT_BOOL prvTgfCaptureForceTtiLookupUnchanged = GT_FALSE;
void prvTgfCaptureForceTtiDisableModeSet(
    IN GT_BOOL                  forceTtiLookupUnchanged
)
{
    prvTgfCaptureForceTtiLookupUnchanged = forceTtiLookupUnchanged;
}

/**
* @internal prvTgfCaptureSet function
* @endinternal
*
* @brief   Start/Stop capture the traffic that egress the port
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] mode                     - packet capture mode
* @param[in] start                    - start/stop capture on this port.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface, mode
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The Start 'Capture' is emulated by setting next for the port:
*       1. ingress PCL rule for ALL traffic from this port to TRAP to the CPU
*       we will assign it with 'Application specific cpu code' to
*       distinguish from other traffic go to the CPU
*       The Stop 'Capture' is emulated by setting next for the port:
*       2. remove the ingress PCL rule that function
*       tgfTrafficGeneratorPortTxEthCaptureStart added
*
*/
static GT_STATUS prvTgfCaptureSet
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN TGF_CAPTURE_MODE_ENT     mode,
    IN GT_BOOL                  start
)
{

    if(forceCaptureModeEnabled == GT_TRUE)
    {
        /* for debug force specific capture mode , with out changing the test */
        mode = forceCaptureMode;
    }


#ifdef CHX_FAMILY
    if (PRV_CPSS_PP_MAC(portInterfacePtr->devPort.hwDevNum)->devFamily != CPSS_PX_FAMILY_PIPE_E)
    {
        GT_STATUS rc;
        GT_U8    devNum = (GT_U8)portInterfacePtr->devPort.hwDevNum;

        if (prvTgfCaptureForceTtiLookupUnchanged == GT_FALSE)
        {
            /* disable any TTI lookups on this physical port ... to remove option that
            it will ignore the 'mirror'/'ipcl' configurations
            or
            restore the TTI lookups on this physical port */
            rc = prvTgfCaptureSet_ttiLookupInfo(devNum,
                portInterfacePtr->devPort.portNum,
                start);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if (start == GT_TRUE && prvTgfCaptureTpidWithZeroBmp == DEFAULT_TPID_WITH_ZERO_BMP_CNS)
        {
            prvTgfCaptureEnableUnmatchedTpidProfile(devNum, start, DEFAULT_TPID_WITH_ZERO_BMP_CNS);
        }
        /* disable/restore vlan tag (TPID) recognition on the packet so tagged packet will
        be trapped to CPU without loosing the original etherType */
        rc = prvTgfCaptureSet_ingressTagging(devNum,
            portInterfacePtr->devPort.portNum,
            start);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (start == GT_FALSE && prvTgfCaptureTpidWithZeroBmp == DEFAULT_TPID_WITH_ZERO_BMP_CNS)
        {
            prvTgfCaptureEnableUnmatchedTpidProfile(devNum, start, DEFAULT_TPID_WITH_ZERO_BMP_CNS);
        }

        switch (mode)
        {
            case TGF_CAPTURE_MODE_PCL_E:
                return  prvTgfDxChPclCaptureSet(devNum,
                                                portInterfacePtr->devPort.portNum,
                                                start);
            case TGF_CAPTURE_MODE_MIRRORING_E:
                return  prvTgfDxChMirroringCaptureSet(devNum,
                                                      portInterfacePtr->devPort.portNum,
                                                      start);
            default:
                return GT_BAD_PARAM;
        }
    }



#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) && !(defined EX_FAMILY)
    TGF_PARAM_NOT_USED(portInterfacePtr);
    TGF_PARAM_NOT_USED(start);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) && !(defined EX_FAMILY) */
    return GT_OK;
}

/**
* @internal portTxEthCaptureSet function
* @endinternal
*
* @brief   Start/Stop capture the traffic that egress the port
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] mode                     - packet capture mode
* @param[in] start                    - start/stop capture on this port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The Start capture is emulated by setting next for the port:
*       1. loopback on the port (so all traffic that egress the port will
*       ingress the same way)
*       2. ingress PCL rule for ALL traffic from this port to TRAP to the CPU
*       we will assign it with 'Application specific cpu code' to
*       distinguish from other traffic go to the CPU
*       The Stop capture is emulated by setting next for the port:
*       1. disable loopback on the port
*       2. remove the ingress PCL rule that function
*       tgfTrafficGeneratorPortTxEthCaptureStart added
*
*/
static GT_STATUS portTxEthCaptureSet
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN TGF_CAPTURE_MODE_ENT      mode,
    IN GT_BOOL                   start
)
{
    GT_STATUS rc = GT_OK;
    CPSS_INTERFACE_INFO_STC  localPortInterface;

    CPSS_NULL_PTR_CHECK_MAC(portInterfacePtr);

    if(portInterfacePtr->type != CPSS_INTERFACE_PORT_E)
    {
        return GT_BAD_PARAM;
    }

    localPortInterface = *portInterfacePtr;

    portInterfacePtr = NULL;/* make sure not to use it any more ... only localPortInterface */

    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(localPortInterface.devPort.hwDevNum))
    {
        /* was not initialized properly ...
           WA: to not modify a lot of tests */
        localPortInterface.devPort.hwDevNum = prvTgfDevNum;
    }

    if (GT_FALSE == start)
    {
        GT_U32    loops;
#ifndef  EMBEDDED
        /* let the packets be captured */
        loops = prvTgfCaptureTimeLoop;
#else
        /* one sleep is enough for HW */
        loops = 1;
#endif
        while(loops--)/* allow debugging by set this value = 1000 */
        {
            cpssOsTimerWkAfter(TGF_DEFAULT_CAPTURE_SLEEP_TIME_CNS);
        }

#if defined(ASIC_SIMULATION) && !defined(GM_USED)
        {
            if(skernelNumOfPacketsInTheSystemGet())
            {
                loops = 1000;/* up to 10 sec */
                while(loops--)
                {
                    cpssOsTimerWkAfter(10);
                    if(skernelNumOfPacketsInTheSystemGet() == 0)
                    {
                        break;
                    }
                }
            }
        }
#endif /*ASIC_SIMULATION*/

        if(tgfTrafficGeneratorRxCaptureNum)
        {
            tgfTrafficGeneratorRxCaptureNum--;
        }
        else
        {
            /* error */
        }

        if(txEthCaptureDoLoopback == GT_TRUE)
        {
            /* Disable Port Loopback in the begging of configuration for DISABLE case.
               This is proper position for Capture Enable/Disable under traffic. */
            rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&localPortInterface, start);
            if (GT_OK != rc)
            {
                return rc;
            }

            if(txEthCaptureWaitAfterDisableLoopback)
            {
                /* disable capture under traffic requres some time to handle
                   already captured packets after loopback disabled */
                cpssOsTimerWkAfter(5);
            }
        }
    }
    else
    {
        tgfTrafficGeneratorRxCaptureNum++;
    }

    rc = prvTgfCaptureSet(&localPortInterface, mode, start);
    if (GT_OK != rc)
    {
        return rc;
    }

#ifdef IMPL_GALTIS
    /* notify the 'galtis tables' of 'rx packet to CPU' that the enhUT uses
       'capture' with specific CPU codes */
    if (tgfCmdCpssNetEnhUtUseCaptureToCpuPtr != NULL)
    {
        *tgfCmdCpssNetEnhUtUseCaptureToCpuPtr = start;
        rc = GT_OK;
    }
    else
    {
        rc = GT_NOT_IMPLEMENTED;
    }
    if (GT_OK != rc)
    {
        return rc;
    }
#endif /*IMPL_GALTIS*/

    rc = prvTgfTrafficTableRxStartCapture(start);
    if (GT_OK != rc)
    {
        return rc;
    }

    if(txEthCaptureDoLoopback == GT_TRUE)
    {
        /* Enable Port Loopback only in the end of configuration for ENABLE case.
           This is proper position for Capture Enable/Disable under traffic. */
        if (GT_FALSE != start)
        {
            rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&localPortInterface, start);
            if (GT_OK != rc)
            {
                return rc;
            }
        }
    }


    return rc;
}

/**
* @internal tgfTrafficGeneratorPortTxEthCaptureSet function
* @endinternal
*
* @brief   Start/Stop capture the traffic that egress the port/trunk
*         when 'trunk' implementation actually loops on all enabled members.
* @param[in] portInterfacePtr         - (pointer to) port/trunk interface
* @param[in] mode                     - packet capture mode
* @param[in] start                    - start/stop capture on this port/trunk
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The Start capture is emulated by setting next for the port:
*       1. loopback on the port (so all traffic that egress the port will
*       ingress the same way)
*       2. ingress PCL rule for ALL traffic from this port to TRAP to the CPU
*       we will assign it with 'Application specific cpu code' to
*       distinguish from other traffic go to the CPU
*       The Stop capture is emulated by setting next for the port:
*       1. disable loopback on the port
*       2. remove the ingress PCL rule that function
*       tgfTrafficGeneratorPortTxEthCaptureStart added
*
*/
GT_STATUS tgfTrafficGeneratorPortTxEthCaptureSet
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN TGF_CAPTURE_MODE_ENT      mode,
    IN GT_BOOL                   start
)
{
    GT_U32                  numOfEnabledMembers;
    CPSS_TRUNK_MEMBER_STC   enabledMembersArray[PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
    GT_U32                  ii;
    CPSS_INTERFACE_INFO_STC tmpInterface;
    GT_STATUS rc,rc1 = GT_OK;

    if (portInterfacePtr->type == CPSS_INTERFACE_TRUNK_E)
    {
        /* get all trunk members */
        numOfEnabledMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
        rc = prvCpssGenericTrunkDbEnabledMembersGet(prvTgfDevNum,portInterfacePtr->trunkId,&numOfEnabledMembers,enabledMembersArray);
        if(rc != GT_OK)
        {
            PRV_TGF_LOG2_MAC("prvCpssGenericTrunkDbEnabledMembersGet: %d %d",
                              prvTgfDevNum, portInterfacePtr->trunkId);
            return rc;
        }

        if(numOfEnabledMembers == 0)
        {
            /* empty trunk ??? */
            PRV_TGF_LOG0_MAC("[TGF]: tgfTrafficGeneratorPortTxEthCaptureSet no trunk members");
            return GT_EMPTY;
        }

        /*set all trunk members as 'captured' start/stop */
        tmpInterface.type = CPSS_INTERFACE_PORT_E;
        for(ii = 0 ; ii < numOfEnabledMembers; ii++)
        {
            tmpInterface.devPort.portNum = enabledMembersArray[ii].port;
            tmpInterface.devPort.hwDevNum = enabledMembersArray[ii].hwDevice;

            rc = extUtilSwDeviceNumberGet(tmpInterface.devPort.hwDevNum,
                                          &tmpInterface.devPort.hwDevNum);
            if(rc != GT_OK)
            {
                rc1 = rc;
            }

            rc = portTxEthCaptureSet(&tmpInterface,mode,start);
            if(rc != GT_OK)
            {
                rc1 = rc;
            }
        }

        return rc1;
    }
    else
    if(portInterfacePtr->type == CPSS_INTERFACE_PORT_E)
    {
        if (portInterfacePtr->devPort.portNum == NOT_VALID_CNS)
        {
            /* do capture of traffic trapped/mirrored to CPU due to test configuration without interventton of traffic generator */
            prvTgfTrafficTableRxStartCapture(start);
            return GT_OK;
        }
    }

    return portTxEthCaptureSet(portInterfacePtr,mode,start);
}
#endif /*DXCH_CODE*/

/**
* @internal tgfTrafficGeneratorLoopTimeSet function
* @endinternal
*
* @brief   Set sleeping time after sending burst of packets from the CPU
*
* @param[in] timeInSec                - time in seconds
*
* @retval GT_OK                    - on success
*
* @note Needed for debugging purposes
*
*/
GT_STATUS tgfTrafficGeneratorLoopTimeSet
(
    IN GT_U32   timeInSec
)
{
    if(timeInSec)
    {
        prvTgfTimeLoop = timeInSec * 2;
    }
    else
    {
#ifdef  EMBEDDED
        /* run on hardware */
        prvTgfTimeLoop = 1;
#else
        /* run on simulation */
        prvTgfTimeLoop = TGF_DEFAULT_TIME_LOOP_NUM_CNS;
#endif
    }

    return GT_OK;
}

/**
* @internal tgfTrafficGeneratorGmTimeFactorSet function
* @endinternal
*
* @brief   Set time factor for GM devices , due to very slow processing of
*         packets (2 per second)
* @param[in] timeFactor               - factor for the time
*
* @retval GT_OK                    - on success
*
* @note Needed for debugging purposes
*
*/
GT_STATUS tgfTrafficGeneratorGmTimeFactorSet
(
    IN GT_U32   timeFactor
)
{
    if(timeFactor == 0)
    {
        timeFactor = 1;
    }

    prvTgfGmTimeFactor = timeFactor;

    return GT_OK;
}


/**
* @internal tgfTrafficGeneratorCaptureLoopTimeSet function
* @endinternal
*
* @brief   Set sleeping time before disabling packet's capturing
*
* @param[in] timeInMSec               - time in miliseconds (0 for restore defaults)
*
* @retval GT_OK                    - on success
*
* @note Needed for debugging purposes
*
*/
GT_STATUS tgfTrafficGeneratorCaptureLoopTimeSet
(
    IN GT_U32   timeInMSec
)
{
    prvTgfCaptureTimeLoop = (timeInMSec) ? (timeInMSec / TGF_DEFAULT_CAPTURE_SLEEP_TIME_CNS) : TGF_DEFAULT_CAPTURE_LOOP_NUM_CNS;

    return GT_OK;
}


/**
* @internal tgfTrafficGeneratorEtherTypeIgnoreSet function
* @endinternal
*
* @brief   indication to IGNORE setting of 'etherTypeForVlanTag' that is set by
*         tgfTrafficGeneratorEtherTypeForVlanTagSet(...) and default is '0x8100'
* @param[in] ignore                   - GT_TRUE  - to      etherTypeForVlanTag.
*                                      GT_FALSE - to NOT ignore etherTypeForVlanTag .
*                                       None
*/
extern GT_VOID tgfTrafficGeneratorEtherTypeIgnoreSet
(
    IN GT_BOOL ignore
)
{
    ignore_etherTypeForVlanTag = ignore;
}

/**
* @internal tgfTrafficGeneratorEtherTypeIgnoreGet function
* @endinternal
*
* @brief   get state of flag to IGNORE setting of 'etherTypeForVlanTag' that is set
*         by tgfTrafficGeneratorEtherTypeForVlanTagSet(...) and default is '0x8100'
*
* @retval GT_TRUE                  - ignore etherTypeForVlanTag.
* @retval GT_FALSE                 - not ignore etherTypeForVlanTag .
*/
extern GT_BOOL tgfTrafficGeneratorEtherTypeIgnoreGet
(
    GT_VOID
)
{
    return ignore_etherTypeForVlanTag;
}

/**
* @internal tgfTrafficGeneratorEtherTypeForVlanTagSet function
* @endinternal
*
* @brief   set etherType to recognize 'Tagged packes' that send from the traffic
*         generator to the PP
* @param[in] etherType                - new etherType
*                                      (was initialized as 0x8100)
*                                       None
*/
GT_VOID tgfTrafficGeneratorEtherTypeForVlanTagSet
(
    IN GT_U16   etherType
)
{
    etherTypeForVlanTag = etherType;
}

/**
* @internal internalAutoChecksumField function
* @endinternal
*
* @brief   apply auto checksum fields after applying the VFD
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - parser caused bad state that should not happen
*
* @note Buffer must be allocated for CRC (4 bytes)
*
*/
static GT_STATUS internalAutoChecksumField
(
    IN TGF_AUTO_CHECKSUM_INFO_STC *checksumInfoPtr,
    IN TGF_AUTO_CHECKSUM_EXTRA_INFO_STC *checksumExtraInfoPtr
)
{
    GT_STATUS rc;
    GT_U32  ii,kk;
    TGF_AUTO_CHECKSUM_INFO_STC *currChecksumInfoPtr;/* current checksum info pointer */
    GT_U8   *pseudoPtr;/* pointer to pseudo header buffer */
    GT_U32  pseudoNumBytes;/* pseudo header number of bytes */
    GT_U32  headerLen,totalLen,l4Length;

    currChecksumInfoPtr = checksumInfoPtr;
    for(ii = 0 ; ii < TGF_AUTO_CHECKSUM_FIELD_LAST_E ; ii++,currChecksumInfoPtr++)
    {
        if(currChecksumInfoPtr->enable != GT_TRUE ||
           currChecksumInfoPtr->checkSumPtr == NULL)
        {
            continue;
        }

        switch(ii)
        {
            case TGF_AUTO_CHECKSUM_FIELD_TCP_E:
            case TGF_AUTO_CHECKSUM_FIELD_UDP_E:
                if(checksumExtraInfoPtr->isIpv4 == GT_FALSE)
                {   /* ipv6 */
                    /* Sip ,Dip */
                    for(kk = 0 ; kk < 32 ; kk++)
                    {
                        checksumExtraInfoPtr->pseudo.buffer[kk]   = checksumExtraInfoPtr->startL3HeadrPtr[8+kk];
                    }

                    /* zero */
                    checksumExtraInfoPtr->pseudo.buffer[kk++] = 0;
                    checksumExtraInfoPtr->pseudo.buffer[kk++] = 0;
                    /* l4 length*/
                    l4Length =  (checksumExtraInfoPtr->startL3HeadrPtr[4] << 8)  |
                                 checksumExtraInfoPtr->startL3HeadrPtr[5];
                    checksumExtraInfoPtr->pseudo.buffer[kk++] = checksumExtraInfoPtr->startL3HeadrPtr[4];
                    checksumExtraInfoPtr->pseudo.buffer[kk++] = checksumExtraInfoPtr->startL3HeadrPtr[5];

                    checksumExtraInfoPtr->pseudo.buffer[kk++] = 0;
                    checksumExtraInfoPtr->pseudo.buffer[kk++] = 0;
                    checksumExtraInfoPtr->pseudo.buffer[kk++] = 0;
                    checksumExtraInfoPtr->pseudo.buffer[kk++] = checksumExtraInfoPtr->startL3HeadrPtr[6];

                    checksumExtraInfoPtr->pseudo.numBytes = kk;
                }
                else
                {
                    headerLen = 4 * (checksumExtraInfoPtr->startL3HeadrPtr[0] & 0xF);
                    totalLen =  (checksumExtraInfoPtr->startL3HeadrPtr[2] << 8) |
                                 checksumExtraInfoPtr->startL3HeadrPtr[3];
                    l4Length = totalLen - headerLen;

                    /* Sip ,Dip */
                    for(kk = 0 ; kk < 8 ; kk++)
                    {
                        checksumExtraInfoPtr->pseudo.buffer[kk] = checksumExtraInfoPtr->startL3HeadrPtr[12+kk];
                    }

                    /* zero */
                    checksumExtraInfoPtr->pseudo.buffer[kk++] = 0;
                    /* protocol */
                    checksumExtraInfoPtr->pseudo.buffer[kk++] = checksumExtraInfoPtr->startL3HeadrPtr[9];
                    /* l4 length*/
                    checksumExtraInfoPtr->pseudo.buffer[kk++] = (GT_U8)(l4Length >> 8);
                    checksumExtraInfoPtr->pseudo.buffer[kk++] = (GT_U8)(l4Length);

                    checksumExtraInfoPtr->pseudo.numBytes = kk;

                }

                if(ii == TGF_AUTO_CHECKSUM_FIELD_TCP_E)
                {
                    /* TCP not hold length , so take it from ipv4/6 info */
                    currChecksumInfoPtr->numBytes = l4Length;
                }

                pseudoPtr = checksumExtraInfoPtr->pseudo.buffer;
                pseudoNumBytes = checksumExtraInfoPtr->pseudo.numBytes;

                if(checksumExtraInfoPtr->udpLengthFieldPtr)
                {
                    /* UDP field of length to be same as the calculated length */
                    checksumExtraInfoPtr->udpLengthFieldPtr[0] = (GT_U8)(l4Length >> 8);
                    checksumExtraInfoPtr->udpLengthFieldPtr[1] = (GT_U8)(l4Length);

                    currChecksumInfoPtr->numBytes = l4Length;
                }

                break;
            default:
                pseudoPtr = NULL;
                pseudoNumBytes = 0;
                break;
        }

        /* apply checksum auto-calc on needed section of the packet */
        currChecksumInfoPtr->checkSumPtr[0] = 0;
        currChecksumInfoPtr->checkSumPtr[1] = 0;
        rc = tgfTrafficEnginePacketCheckSum16BitsCalc(
                currChecksumInfoPtr->startPtr,
                currChecksumInfoPtr->numBytes,
                pseudoPtr,
                pseudoNumBytes,
                currChecksumInfoPtr->checkSumPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }


    return GT_OK;
}

/**
* @internal autoChecksumField function
* @endinternal
*
* @brief   apply auto checksum fields after applying the VFD
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - parser caused bad state that should not happen
*
* @note Buffer must be allocated for CRC (4 bytes)
*
*/
static GT_STATUS autoChecksumField
(
    IN TGF_AUTO_CHECKSUM_FULL_INFO_STC  *checksumInfoPtr,
    IN TGF_AUTO_CHECKSUM_EXTRA_INFO_STC *checksumExtraInfoPtr
)
{
    GT_STATUS rc;
    GT_U32  ii;

    /* do reverse loop because UDP/TCP checksum need to be calced only after whole it's payload already known */
    for(ii = TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS - 1;
         /*no end condition*/;
         ii--)
    {
        rc = internalAutoChecksumField(checksumInfoPtr[ii].singleInfoArr,&checksumExtraInfoPtr[ii]);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(ii == 0)
        {
            break;
        }
    }

    return GT_OK;
}

void setMiiCounter(IN GT_U32 newMiiCounter)
{
    miiCounter = newMiiCounter;
}


void debugForcePrint(IN GT_U32 forcePrint)
{
    debug_forcePrint = forcePrint;
}

/**
* @internal tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet function
* @endinternal
*
* @brief   do we need to capture by PCL all packets , exclude those of specific vlan ?
*         this is relevant only when 'PCL capture' used (TGF_CAPTURE_MODE_PCL_E)
* @param[in] excludeEnable            - GT_TRUE - exclude
*                                      GT_FALSE - no excluding.
* @param[in] excludedVid              - the vid to exclude .
*                                      relevant only when  excludeEnable = GT_TRUE
*
* @retval GT_OK                    - on success
*/
GT_STATUS tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet
(
    IN GT_BOOL  excludeEnable,
    IN GT_U16   excludedVid
)
{

    pclCaptureExcludeVidEnabled = excludeEnable;
    pclCaptureExcludeVid = excludedVid;

    return GT_OK;
}

/* allow to set : do we wait for traffic that need to trap to cpu */
GT_STATUS tgfTrafficGeneratorRetryRxToCpuAndWaitSet(IN GT_U32 allow)
{
    retryRxToCpuAndWait = allow;
    return GT_OK;
}

/**
* @internal tgfTrafficGeneratorRxInCpuNumWait function
* @endinternal
*
* @brief   wait for ALL packets that expected in the CPU, to received.
*
* @param[in] numOfExpectedPackets     - number of packets
* @param[in] maxTimeToWait            - MAX time (in milliseconds) to wait for the packets.
*                                      The functions will return as soon as all packets
*                                      received (but not more then this max time)
*
* @param[out] timeActuallyWaitPtr      - time in milliseconds actually waited for the packets.
*                                      NOTE: this parameter can be NULL.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not get all packets on time
* @retval GT_BAD_PARAM             - wrong numOfExpectedPackets
*/
GT_STATUS tgfTrafficGeneratorRxInCpuNumWait
(
    IN GT_U32   numOfExpectedPackets,
    IN GT_U32   maxTimeToWait,
    OUT GT_U32  *timeActuallyWaitPtr
)
{
    GT_STATUS           rc             ;
    static GT_8         specificDeviceFormat[TGF_SPECIFIC_DEVICE_FORMAT_SIZE_CNS] = {0};
    static GT_U8        packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32              packetBufLen;
    GT_U32              origPacketLen = 0;
    GT_U8               devNum;
    GT_U8               queue;

    if(numOfExpectedPackets == 0)
    {
        return GT_BAD_PARAM;
    }


    if(maxTimeToWait == 0)
    {
        maxTimeToWait = prvTgfCaptureTimeLoop * TGF_DEFAULT_CAPTURE_SLEEP_TIME_CNS;
    }

    if(timeActuallyWaitPtr)
    {
        *timeActuallyWaitPtr = 0;
    }

    do{
        packetBufLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
        rc = tgfTrafficTableRxPcktGet((numOfExpectedPackets - 1), packetBuff ,&packetBufLen, &origPacketLen,
                                      &devNum, &queue, specificDeviceFormat);

        if(rc == GT_NOT_FOUND)
        {
            if(maxTimeToWait--)
            {
                cpssOsTimerWkAfter(1);
                if(timeActuallyWaitPtr)
                {
                    (*timeActuallyWaitPtr) ++;
                }
            }
            else
            {
                /* the max time to wait is over ...*/
                break;
            }
        }
        else if(rc == GT_OK)
        {
            /* index of last packet is not empty */
            break;
        }
        else
        {
            /* some kind of error ? */
            break;
        }
    }while(1);

    return rc;
}


/**
* @internal tgfTrafficGeneratorExpectTraficToCpu function
* @endinternal
*
* @brief   state that current test expect traffic to the CPU
*         NOTE: this function should not be called for cases that test uses
*         'capture' to the CPU.
* @param[in] enable                   - GT_TRUE    - test expect traffic to the CPU.
*                                      GT_FALSE   - test NOT expect traffic to the CPU.
*
* @retval GT_OK                    - on success
*/
GT_STATUS tgfTrafficGeneratorExpectTraficToCpu
(
    IN GT_BOOL  enable
)
{
    tgfTrafficGeneratorExpectTraficToCpuEnabled = enable;

    return GT_OK;
}

#define SPECIAL_PORT_NUM_MIN 59
#define SPECIAL_PORT_NUM_MAX 63
#define CHECK_SPECIAL_PORT_NUM_AFTER_6_BITS_REDUCE(portNum)   \
    {                                                         \
        GT_U32  value6bits = portNum & 0x3f;                  \
        GT_U32  origValue  = portNum;                         \
        if(value6bits != origValue)                           \
        {                                                     \
            /* we reduced bits ... check that the reduced value is not 'special' port value */ \
            if(value6bits >= SPECIAL_PORT_NUM_MIN &&          \
               value6bits <= SPECIAL_PORT_NUM_MAX)            \
            {                                                 \
                /* generate warning to the test that the actual target port is 'special'*/ \
                cpssOsPrintf("WARNING the DSA hold 6 bits of trgPort[%d] that is 'special value' (reduced from [%d]) \n", \
                    value6bits,origValue);                    \
            }                                                 \
        }                                                     \
    }

/**
* @internal tgfTrafficGeneratorIsSpecialPortNumAfterReductionInDsa function
* @endinternal
*
* @brief   check if the port number is special (59-63) when need to be in the DSA tag
*           as the DSA hold limited number of bits for the portNum.
*
* @param[in] devNum  - the device number
* @param[in] portNum - the port number
* @param[in] dsaType - the DSA type (1/2/4 words)
*
* @retval GT_TRUE  - the port is     special only because the DSA hold less bits
*                    than actually set in the portNum
*         GT_FALSE - the port is not special or not reduced in the DSA
*/
GT_BOOL tgfTrafficGeneratorIsSpecialPortNumAfterReductionInDsa(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN TGF_DSA_TYPE_ENT     dsaType
)
{
    GT_STATUS   rc;
    GT_HW_DEV_NUM hwDevNum;
    GT_U32  numBits;

    rc = extUtilHwDeviceNumberGet(devNum, &hwDevNum);
    if(GT_OK != rc)
        return GT_FALSE;

    switch(dsaType)
    {
        case TGF_DSA_1_WORD_TYPE_E:
            numBits = 5;
            break;
        case TGF_DSA_2_WORD_TYPE_E:
            if(PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(hwDevNum))
            {
                numBits = 7;
            }
            else
            {
                numBits = 6;
            }
            break;
        default: /* eDsa - 4 words */
            numBits = 8;
            break;
    }

    return tgfTrafficGeneratorIsSpecialPortNumAfterReductionInBits(devNum,portNum,numBits);
}

/**
* @internal tgfTrafficGeneratorIsSpecialPortNumAfterReductionInBits function
* @endinternal
*
* @brief   check if the port number is special (59-63) when need to be with limited
*           number of bits for the portNum.
*
* @param[in] devNum  - the device number
* @param[in] portNum - the port number
* @param[in] numBits - the number of bits
*
* @retval GT_TRUE  - the port is     special only because the 'numBits' less bits
*                    than actually set in the portNum
*         GT_FALSE - the port is not special or not reduced in the 'numBits'
*/
GT_BOOL tgfTrafficGeneratorIsSpecialPortNumAfterReductionInBits(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               numBits
)
{
    GT_U32  mask = (1 << numBits) - 1;
    GT_U32  finalPortNum;

    if(portNum <= mask)
    {
        /* the caller will not loose bits , meaning that he wanted to sent the
           packet with this info */
        return GT_FALSE;
    }
    finalPortNum = portNum & mask;

    if(finalPortNum  >= SPECIAL_PORT_NUM_MIN &&
       finalPortNum  <= SPECIAL_PORT_NUM_MAX)
    {
        /* the caller will loose bits and get into 'special range' */
        return GT_TRUE;
    }

    return GT_FALSE;
}

#ifdef CHX_FAMILY


/**
* @internal dxchDsaTagBuild function
* @endinternal
*
* @brief   Build DSA tag bytes on the packet from the DSA tag parameters --> dxch devices
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dsaInfoPtr               - (pointer to) the DSA parameters that need to be built into
*                                      the packet
*
* @param[out] dsaBytesPtr              - pointer to the start of DSA tag in the packet
*                                      This pointer in the packet should hold space for :
*                                      4 bytes when using regular DSA tag.
*                                      8 bytes when using extended DSA tag.
*                                      This function only set values into the 4 or 8 bytes
*                                      according to the parameters in parameter dsaInfoPtr.
*                                      NOTE : this memory is the caller's responsibility (to
*                                      allocate / free)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS dxchDsaTagBuild
(
    IN  TGF_PACKET_DSA_TAG_STC         *dsaInfoPtr,
    OUT GT_U8                          *dsaBytesPtr
)
{
    GT_U8                          devNum = prvTgfDevNum;
    CPSS_DXCH_NET_DSA_PARAMS_STC   dsaInfo;
    GT_STATUS                      rc;

    cpssOsMemSet(&dsaInfo,0,sizeof(dsaInfo));

    /* type of DSA : 1/2/3/4 words */
    dsaInfo.commonParams.dsaTagType = dsaInfoPtr->dsaType;
    /* command of DSA : to_cpu/from_cpu/to_analyzer/forward */
    dsaInfo.dsaType                 = dsaInfoPtr->dsaCommand;

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.commonParams,  &dsaInfoPtr->commonParams,  vpt             );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.commonParams,  &dsaInfoPtr->commonParams,  cfiBit          );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.commonParams,  &dsaInfoPtr->commonParams,  vid             );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.commonParams,  &dsaInfoPtr->commonParams,  dropOnSource    );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.commonParams,  &dsaInfoPtr->commonParams,  packetIsLooped  );

    switch(dsaInfoPtr->dsaCommand)
    {
        case  TGF_DSA_CMD_TO_CPU_E:
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toCpu,  &dsaInfoPtr->dsaInfo.toCpu,  isEgressPipe  );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toCpu,  &dsaInfoPtr->dsaInfo.toCpu,  isTagged      );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toCpu,  &dsaInfoPtr->dsaInfo.toCpu,  hwDevNum      );
            rc = extUtilHwDeviceNumberGet(dsaInfo.dsaInfo.toCpu.hwDevNum,
                                         &dsaInfo.dsaInfo.toCpu.hwDevNum);
            if(GT_OK != rc)
                return rc;
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toCpu,  &dsaInfoPtr->dsaInfo.toCpu,  srcIsTrunk    );
#ifdef CHECK_SIZE_MISMATCH
            if (sizeof(dsaInfo.dsaInfo.toCpu.interface) != sizeof(dsaInfoPtr->dsaInfo.toCpu.interface))
            {
                return GT_BAD_STATE;
            }
#endif
            PRV_TGF_STC_S2D_UNION_FIELD_COPY_MAC(&dsaInfo.dsaInfo.toCpu,&dsaInfoPtr->dsaInfo.toCpu,  interface   );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toCpu,  &dsaInfoPtr->dsaInfo.toCpu,  cpuCode       );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toCpu,  &dsaInfoPtr->dsaInfo.toCpu,  wasTruncated  );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toCpu,  &dsaInfoPtr->dsaInfo.toCpu,  originByteCount);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toCpu,  &dsaInfoPtr->dsaInfo.toCpu,  timestamp     );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toCpu,  &dsaInfoPtr->dsaInfo.toCpu,  packetIsTT    );
#ifdef CHECK_SIZE_MISMATCH
            if (sizeof(dsaInfo.dsaInfo.toCpu.flowIdTtOffset) != sizeof(dsaInfoPtr->dsaInfo.toCpu.flowIdTtOffset))
            {
                return GT_BAD_STATE;
            }
#endif
            PRV_TGF_STC_S2D_UNION_FIELD_COPY_MAC(&dsaInfo.dsaInfo.toCpu,&dsaInfoPtr->dsaInfo.toCpu,flowIdTtOffset);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toCpu,  &dsaInfoPtr->dsaInfo.toCpu,  tag0TpidIndex );


            switch(dsaInfoPtr->dsaType)
            {
                case TGF_DSA_2_WORD_TYPE_E:
                    if(PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(dsaInfo.dsaInfo.toCpu.hwDevNum))
                    {
                        dsaInfo.dsaInfo.toCpu.interface.portNum &= 0x7f;
                    }
                    else
                    {
                        dsaInfo.dsaInfo.toCpu.interface.portNum &= 0x3f;
                    }
                    break;
            case TGF_DSA_4_WORD_TYPE_E:
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        /* the DSA tag limited to 10 bits */
                        dsaInfo.dsaInfo.toCpu.interface.portNum &= 0x3ff;

                    }
                    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        /* the DSA tag limited to 9 bits */
                        dsaInfo.dsaInfo.toCpu.interface.portNum &= 0x1ff;
                    }
                    else
                    {
                        /* the DSA tag limited to 8 bits */
                        dsaInfo.dsaInfo.toCpu.interface.portNum &= 0xff;
                    }
                    break;
                default:
                    break;
            }

            break;
        case TGF_DSA_CMD_FROM_CPU_E:
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.fromCpu,  &dsaInfoPtr->dsaInfo.fromCpu,  dstInterface  );
            if(dsaInfo.dsaInfo.fromCpu.dstInterface.type == CPSS_INTERFACE_PORT_E)
            {
                rc = extUtilHwDeviceNumberGet(dsaInfo.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum,
                                             &dsaInfo.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum);
                if(GT_OK != rc)
                    return rc;
            }
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.fromCpu,  &dsaInfoPtr->dsaInfo.fromCpu,  tc            );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.fromCpu,  &dsaInfoPtr->dsaInfo.fromCpu,  dp            );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.fromCpu,  &dsaInfoPtr->dsaInfo.fromCpu,  egrFilterEn   );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.fromCpu,  &dsaInfoPtr->dsaInfo.fromCpu,  cascadeControl);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.fromCpu,  &dsaInfoPtr->dsaInfo.fromCpu,  egrFilterRegistered);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.fromCpu,  &dsaInfoPtr->dsaInfo.fromCpu,  srcId         );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.fromCpu,  &dsaInfoPtr->dsaInfo.fromCpu,  srcHwDev      );
            rc = extUtilHwDeviceNumberGet(dsaInfo.dsaInfo.fromCpu.srcHwDev,
                                         &dsaInfo.dsaInfo.fromCpu.srcHwDev);
            if(GT_OK != rc)
                return rc;
#ifdef CHECK_SIZE_MISMATCH
            if (sizeof(dsaInfo.dsaInfo.fromCpu.extDestInfo) != sizeof(dsaInfoPtr->dsaInfo.fromCpu.extDestInfo))
            {
                return GT_BAD_STATE;
            }
#endif
            PRV_TGF_STC_S2D_UNION_FIELD_COPY_MAC(&dsaInfo.dsaInfo.fromCpu,&dsaInfoPtr->dsaInfo.fromCpu,  extDestInfo   );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.fromCpu,  &dsaInfoPtr->dsaInfo.fromCpu,  isTrgPhyPortValid);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.fromCpu,  &dsaInfoPtr->dsaInfo.fromCpu,  dstEport      );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.fromCpu,  &dsaInfoPtr->dsaInfo.fromCpu,  tag0TpidIndex );

            if(dsaInfo.dsaInfo.fromCpu.dstInterface.type == CPSS_INTERFACE_PORT_E)
            {
                if(dsaInfo.commonParams.dsaTagType == CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT)
                {
                    if(dsaInfo.dsaInfo.fromCpu.dstInterface.devPort.portNum >= BIT_8)
                    {
                        if(dsaInfo.dsaInfo.fromCpu.isTrgPhyPortValid == GT_TRUE)
                        {
                            /* we must use the 'E2PHY' to convert eport to physical port */
                            dsaInfo.dsaInfo.fromCpu.isTrgPhyPortValid = GT_FALSE;
                            dsaInfo.dsaInfo.fromCpu.dstEport = dsaInfo.dsaInfo.fromCpu.dstInterface.devPort.portNum;
                        }
                        else
                        {
                            /* the dsaInfo.dsaInfo.fromCpu.dstInterface.devPort.portNum
                               is already 'dont care' */
                        }

                        /* do not let the CPSS to fail the operation */
                        dsaInfo.dsaInfo.fromCpu.dstInterface.devPort.portNum &= 0xFF;
                    }
                }
            }

            break;
        case TGF_DSA_CMD_TO_ANALYZER_E:
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toAnalyzer,  &dsaInfoPtr->dsaInfo.toAnalyzer,  rxSniffer    );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toAnalyzer,  &dsaInfoPtr->dsaInfo.toAnalyzer,  isTagged     );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toAnalyzer,  &dsaInfoPtr->dsaInfo.toAnalyzer,  analyzerTrgType);
#ifdef CHECK_SIZE_MISMATCH
            if (sizeof(dsaInfo.dsaInfo.toAnalyzer.devPort) != sizeof(dsaInfoPtr->dsaInfo.toAnalyzer.devPort))
            {
                return GT_BAD_STATE;
            }
#endif
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toAnalyzer.devPort,  &dsaInfoPtr->dsaInfo.toAnalyzer.devPort,  hwDevNum);
            rc = extUtilHwDeviceNumberGet(dsaInfo.dsaInfo.toAnalyzer.devPort.hwDevNum,
                                         &dsaInfo.dsaInfo.toAnalyzer.devPort.hwDevNum);
            if(GT_OK != rc)
                return rc;
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toAnalyzer.devPort,  &dsaInfoPtr->dsaInfo.toAnalyzer.devPort,  portNum    );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toAnalyzer.devPort,  &dsaInfoPtr->dsaInfo.toAnalyzer.devPort,  ePort      );

#ifdef CHECK_SIZE_MISMATCH
            if (sizeof(dsaInfo.dsaInfo.toAnalyzer.extDestInfo) != sizeof(dsaInfoPtr->dsaInfo.toAnalyzer.extDestInfo))
            {
                return GT_BAD_STATE;
            }
#endif
            PRV_TGF_STC_S2D_UNION_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toAnalyzer,  &dsaInfoPtr->dsaInfo.toAnalyzer,  extDestInfo                );

            if(dsaInfoPtr->dsaInfo.toAnalyzer.analyzerTrgType == CPSS_INTERFACE_PORT_E)
            {
                rc = extUtilHwDeviceNumberGet(dsaInfo.dsaInfo.toAnalyzer.extDestInfo.devPort.analyzerHwTrgDev,
                                             &dsaInfo.dsaInfo.toAnalyzer.extDestInfo.devPort.analyzerHwTrgDev);
                if(GT_OK != rc)
                    return rc;
            }

            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toAnalyzer,  &dsaInfoPtr->dsaInfo.toAnalyzer,  tag0TpidIndex   );

            switch(dsaInfoPtr->dsaType)
            {
                case TGF_DSA_2_WORD_TYPE_E:
                    if(PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(dsaInfo.dsaInfo.toAnalyzer.devPort.hwDevNum))
                    {
                        dsaInfo.dsaInfo.toAnalyzer.devPort.portNum &= 0x7f;
                    }
                    else
                    {
                        dsaInfo.dsaInfo.toAnalyzer.devPort.portNum &= 0x3f;
                    }
                    break;
                case TGF_DSA_4_WORD_TYPE_E:
                    /* the DSA tag limited to 8 bits */
                    dsaInfo.dsaInfo.toAnalyzer.devPort.portNum &= 0xff;

                    if(dsaInfoPtr->dsaInfo.toAnalyzer.analyzerTrgType == CPSS_INTERFACE_PORT_E)
                    {
                        dsaInfo.dsaInfo.toAnalyzer.extDestInfo.devPort.analyzerTrgPort &= 0xff;
                    }
                    break;
                default:
                    break;
            }

            break;
        case TGF_DSA_CMD_FORWARD_E:
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  srcIsTagged           );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  srcHwDev                );
            rc = extUtilHwDeviceNumberGet(dsaInfo.dsaInfo.forward.srcHwDev,
                                         &dsaInfo.dsaInfo.forward.srcHwDev);
            if(GT_OK != rc)
                return rc;
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  srcIsTrunk            );
#ifdef CHECK_SIZE_MISMATCH
            if (sizeof(dsaInfo.dsaInfo.forward.source) != sizeof(dsaInfoPtr->dsaInfo.forward.source))
            {
                return GT_BAD_STATE;
            }
#endif
            PRV_TGF_STC_S2D_UNION_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  source                );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  srcId                 );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  egrFilterRegistered   );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  wasRouted             );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  qosProfileIndex       );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  dstInterface          );
            if(dsaInfo.dsaInfo.forward.dstInterface.type == CPSS_INTERFACE_PORT_E)
            {
                rc = extUtilHwDeviceNumberGet(dsaInfo.dsaInfo.forward.dstInterface.devPort.hwDevNum,
                                             &dsaInfo.dsaInfo.forward.dstInterface.devPort.hwDevNum);
                if(GT_OK != rc)
                    return rc;

                switch(dsaInfoPtr->dsaType)
                {
                    case TGF_DSA_2_WORD_TYPE_E:
                        if(PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(dsaInfo.dsaInfo.forward.dstInterface.devPort.hwDevNum))
                        {
                            dsaInfo.dsaInfo.forward.dstInterface.devPort.portNum &= 0x7f;
                        }
                        else
                        {
                            CHECK_SPECIAL_PORT_NUM_AFTER_6_BITS_REDUCE(dsaInfo.dsaInfo.forward.dstInterface.devPort.portNum);

                            dsaInfo.dsaInfo.forward.dstInterface.devPort.portNum &= 0x3f;
                        }
                        break;
                    case TGF_DSA_4_WORD_TYPE_E:
                        /* the DSA tag limited to 8 bits */
                        dsaInfo.dsaInfo.forward.dstInterface.devPort.portNum &= 0xff;
                        break;
                    default:
                        break;
                }
            }

            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  isTrgPhyPortValid     );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  dstEport              );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  tag0TpidIndex         );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  origSrcPhyIsTrunk     );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  skipFdbSaLookup       );
#ifdef CHECK_SIZE_MISMATCH
            if (sizeof(dsaInfo.dsaInfo.forward.origSrcPhy) != sizeof(dsaInfoPtr->dsaInfo.forward.origSrcPhy))
            {
                return GT_BAD_STATE;
            }
#endif
            PRV_TGF_STC_S2D_UNION_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  origSrcPhy      );
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  phySrcMcFilterEnable  );

            switch(dsaInfoPtr->dsaType)
            {
                case TGF_DSA_1_WORD_TYPE_E:
                    if(dsaInfo.dsaInfo.forward.srcIsTrunk == GT_FALSE)
                    {
                        dsaInfo.dsaInfo.forward.source.portNum &= 0x1f;
                    }
                    dsaInfo.dsaInfo.forward.srcHwDev &= 0x1f;
                    break;
                case TGF_DSA_2_WORD_TYPE_E:
                    if(dsaInfo.dsaInfo.forward.srcIsTrunk == GT_FALSE)
                    {
                        if((PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(dsaInfo.dsaInfo.forward.srcHwDev)) ||
                           (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum)))
                        {
                            dsaInfo.dsaInfo.forward.source.portNum &= 0x7f;
                        }
                        else
                        {
                            dsaInfo.dsaInfo.forward.source.portNum &= 0x3f;
                        }
                    }
                    dsaInfo.dsaInfo.forward.srcHwDev &= 0x1f;
                    break;
                case TGF_DSA_4_WORD_TYPE_E:
                    /* no limit on :
                       dsaInfo.dsaInfo.forward.source.portNum
                       as this is srcEport
                    */
                    /* the DSA tag IS NOT limited to 8 bits ,
                       but to 12 bits due to 'MUX' with trunkId !!!*/
                    dsaInfo.dsaInfo.forward.origSrcPhy.portNum &= 0xfff;
                    break;
                default:
                    break;
            }

            break;
        default:
            return GT_NOT_IMPLEMENTED;

    }

    /* call the CPSS to build the DSA tag according to the given info */
    rc = cpssDxChNetIfDsaTagBuild (devNum,&dsaInfo,dsaBytesPtr);


    return rc;
}
/**
* @internal dxchDsaTagParse function
* @endinternal
*
* @brief   parse the DSA tag bytes from the packet into the DSA tag parameters .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dsaBytesPtr              - pointer to the start of DSA tag in the packet.
*
* @param[out] dsaInfoPtr               - (pointer to) the DSA parameters that parsed from the packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS dxchDsaTagParse
(
    IN  GT_U8                          *dsaBytesPtr,
    OUT TGF_PACKET_DSA_TAG_STC         *dsaInfoPtr
)
{
    GT_U8                          devNum = prvTgfDevNum;
    CPSS_DXCH_NET_DSA_PARAMS_STC   dsaInfo;
    GT_STATUS                      rc;

    cpssOsMemSet(&dsaInfo,0,sizeof(dsaInfo));

    /* call the CPSS to parse the DSA tag parameters from the DSA tag on the packet */
    rc = cpssDxChNetIfDsaTagParse (devNum,dsaBytesPtr,&dsaInfo);
    if(rc != GT_OK)
    {
        return rc;
    }


    /* type of DSA : 1/2/3/4 words */
    dsaInfoPtr->dsaType = dsaInfo.commonParams.dsaTagType;
    /* command of DSA : to_cpu/from_cpu/to_analyzer/forward */
    dsaInfoPtr->dsaCommand = dsaInfo.dsaType;

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.commonParams,  &dsaInfoPtr->commonParams,  vpt             );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.commonParams,  &dsaInfoPtr->commonParams,  cfiBit          );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.commonParams,  &dsaInfoPtr->commonParams,  vid             );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.commonParams,  &dsaInfoPtr->commonParams,  dropOnSource    );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.commonParams,  &dsaInfoPtr->commonParams,  packetIsLooped  );

    switch(dsaInfoPtr->dsaCommand)
    {
        case  TGF_DSA_CMD_TO_CPU_E:
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toCpu,  &dsaInfoPtr->dsaInfo.toCpu,  isEgressPipe  );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toCpu,  &dsaInfoPtr->dsaInfo.toCpu,  isTagged      );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toCpu,  &dsaInfoPtr->dsaInfo.toCpu,  hwDevNum      );
            rc = extUtilSwDeviceNumberGet(dsaInfoPtr->dsaInfo.toCpu.hwDevNum,
                                               &dsaInfoPtr->dsaInfo.toCpu.hwDevNum);
            if(GT_OK != rc)
                return rc;
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toCpu,  &dsaInfoPtr->dsaInfo.toCpu,  srcIsTrunk    );
#ifdef CHECK_SIZE_MISMATCH
            if (sizeof(dsaInfo.dsaInfo.toCpu.interface) != sizeof(dsaInfoPtr->dsaInfo.toCpu.interface))
            {
                return GT_BAD_STATE;
            }
#endif
            PRV_TGF_STC_D2S_UNION_FIELD_COPY_MAC(&dsaInfo.dsaInfo.toCpu,&dsaInfoPtr->dsaInfo.toCpu,  interface   );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toCpu,  &dsaInfoPtr->dsaInfo.toCpu,  cpuCode       );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toCpu,  &dsaInfoPtr->dsaInfo.toCpu,  wasTruncated  );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toCpu,  &dsaInfoPtr->dsaInfo.toCpu,  originByteCount);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toCpu,  &dsaInfoPtr->dsaInfo.toCpu,  timestamp     );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toCpu,  &dsaInfoPtr->dsaInfo.toCpu,  packetIsTT    );
#ifdef CHECK_SIZE_MISMATCH
            if (sizeof(dsaInfo.dsaInfo.toCpu.flowIdTtOffset) != sizeof(dsaInfoPtr->dsaInfo.toCpu.flowIdTtOffset))
            {
                return GT_BAD_STATE;
            }
#endif
            PRV_TGF_STC_D2S_UNION_FIELD_COPY_MAC(&dsaInfo.dsaInfo.toCpu,&dsaInfoPtr->dsaInfo.toCpu,flowIdTtOffset);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toCpu,  &dsaInfoPtr->dsaInfo.toCpu,  tag0TpidIndex );
            break;
        case TGF_DSA_CMD_FROM_CPU_E:
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.fromCpu,  &dsaInfoPtr->dsaInfo.fromCpu,  dstInterface  );
            if(dsaInfo.dsaInfo.fromCpu.dstInterface.type == CPSS_INTERFACE_PORT_E)
            {
                rc = extUtilSwDeviceNumberGet(dsaInfoPtr->dsaInfo.fromCpu.dstInterface.devPort.hwDevNum,
                                                   &dsaInfoPtr->dsaInfo.fromCpu.dstInterface.devPort.hwDevNum);
                if(GT_OK != rc)
                    return rc;
            }
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.fromCpu,  &dsaInfoPtr->dsaInfo.fromCpu,  tc            );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.fromCpu,  &dsaInfoPtr->dsaInfo.fromCpu,  dp            );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.fromCpu,  &dsaInfoPtr->dsaInfo.fromCpu,  egrFilterEn   );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.fromCpu,  &dsaInfoPtr->dsaInfo.fromCpu,  cascadeControl);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.fromCpu,  &dsaInfoPtr->dsaInfo.fromCpu,  egrFilterRegistered);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.fromCpu,  &dsaInfoPtr->dsaInfo.fromCpu,  srcId         );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.fromCpu,  &dsaInfoPtr->dsaInfo.fromCpu,  srcHwDev        );
            rc = extUtilSwDeviceNumberGet(dsaInfoPtr->dsaInfo.fromCpu.srcHwDev,
                                               &dsaInfoPtr->dsaInfo.fromCpu.srcHwDev);
            if(GT_OK != rc)
                return rc;
#ifdef CHECK_SIZE_MISMATCH
            if (sizeof(dsaInfo.dsaInfo.fromCpu.extDestInfo) != sizeof(dsaInfoPtr->dsaInfo.fromCpu.extDestInfo))
            {
                return GT_BAD_STATE;
            }
#endif
            PRV_TGF_STC_D2S_UNION_FIELD_COPY_MAC(&dsaInfo.dsaInfo.fromCpu,&dsaInfoPtr->dsaInfo.fromCpu,  extDestInfo   );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.fromCpu,  &dsaInfoPtr->dsaInfo.fromCpu,  isTrgPhyPortValid);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.fromCpu,  &dsaInfoPtr->dsaInfo.fromCpu,  dstEport      );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.fromCpu,  &dsaInfoPtr->dsaInfo.fromCpu,  tag0TpidIndex );
            break;
        case TGF_DSA_CMD_TO_ANALYZER_E:
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toAnalyzer,  &dsaInfoPtr->dsaInfo.toAnalyzer,  rxSniffer    );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toAnalyzer,  &dsaInfoPtr->dsaInfo.toAnalyzer,  isTagged     );
#ifdef CHECK_SIZE_MISMATCH
            if (sizeof(dsaInfo.dsaInfo.toAnalyzer.devPort) != sizeof(dsaInfoPtr->dsaInfo.toAnalyzer.devPort))
            {
                return GT_BAD_STATE;
            }
#endif
            rc = extUtilSwDeviceNumberGet(dsaInfoPtr->dsaInfo.toAnalyzer.devPort.hwDevNum,
                                               &dsaInfoPtr->dsaInfo.toAnalyzer.devPort.hwDevNum);
            if(GT_OK != rc)
                return rc;
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toAnalyzer.devPort,  &dsaInfoPtr->dsaInfo.toAnalyzer.devPort,  portNum    );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toAnalyzer.devPort,  &dsaInfoPtr->dsaInfo.toAnalyzer.devPort,  ePort      );

#ifdef CHECK_SIZE_MISMATCH
            if (sizeof(dsaInfo.dsaInfo.toAnalyzer.extDestInfo) != sizeof(dsaInfoPtr->dsaInfo.toAnalyzer.extDestInfo))
            {
                return GT_BAD_STATE;
            }
#endif
            PRV_TGF_STC_D2S_UNION_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toAnalyzer,  &dsaInfoPtr->dsaInfo.toAnalyzer,  extDestInfo                );

            if(dsaInfoPtr->dsaInfo.toAnalyzer.analyzerTrgType == CPSS_INTERFACE_PORT_E)
            {
                rc = extUtilSwDeviceNumberGet(dsaInfoPtr->dsaInfo.toAnalyzer.extDestInfo.devPort.analyzerHwTrgDev,
                                                   &dsaInfoPtr->dsaInfo.toAnalyzer.extDestInfo.devPort.analyzerHwTrgDev);
                if(GT_OK != rc)
                    return rc;
            }

            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.toAnalyzer,  &dsaInfoPtr->dsaInfo.toAnalyzer,  tag0TpidIndex   );
            break;
        case TGF_DSA_CMD_FORWARD_E:
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  srcIsTagged           );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  srcHwDev                );
            rc = extUtilSwDeviceNumberGet(dsaInfoPtr->dsaInfo.forward.srcHwDev,
                                               &dsaInfoPtr->dsaInfo.forward.srcHwDev);
            if(GT_OK != rc)
                return rc;
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  srcIsTrunk            );
#ifdef CHECK_SIZE_MISMATCH
            if (sizeof(dsaInfo.dsaInfo.forward.source) != sizeof(dsaInfoPtr->dsaInfo.forward.source))
            {
                return GT_BAD_STATE;
            }
#endif
            PRV_TGF_STC_D2S_UNION_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  source                );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  srcId                 );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  egrFilterRegistered   );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  wasRouted             );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  qosProfileIndex       );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  dstInterface          );
            if(dsaInfo.dsaInfo.forward.dstInterface.type == CPSS_INTERFACE_PORT_E)
            {
                rc = extUtilSwDeviceNumberGet(dsaInfoPtr->dsaInfo.forward.dstInterface.devPort.hwDevNum,
                                                   &dsaInfoPtr->dsaInfo.forward.dstInterface.devPort.hwDevNum);
                if(GT_OK != rc)
                    return rc;
            }
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  isTrgPhyPortValid     );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  dstEport              );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  tag0TpidIndex         );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  origSrcPhyIsTrunk     );
#ifdef CHECK_SIZE_MISMATCH
            if (sizeof(dsaInfo.dsaInfo.forward.origSrcPhy) != sizeof(dsaInfoPtr->dsaInfo.forward.origSrcPhy))
            {
                return GT_BAD_STATE;
            }
#endif
            PRV_TGF_STC_D2S_UNION_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  origSrcPhy      );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  phySrcMcFilterEnable  );
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(  &dsaInfo.dsaInfo.forward,  &dsaInfoPtr->dsaInfo.forward,  skipFdbSaLookup       );

            break;
        default:
            return GT_NOT_IMPLEMENTED;

    }

    return GT_OK;
}

#endif /*CHX_FAMILY*/

/**
* @internal prvTgfTrafficGeneratorPacketDsaTagBuild function
* @endinternal
*
* @brief   Build DSA tag bytes on the packet from the DSA tag parameters .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dsaInfoPtr               - (pointer to) the DSA parameters that need to be built into
*                                      the packet
*
* @param[out] dsaBytesPtr              - pointer to the start of DSA tag in the packet
*                                      This pointer in the packet should hold space for :
*                                      4 bytes when using regular DSA tag.
*                                      8 bytes when using extended DSA tag.
*                                      This function only set values into the 4 or 8 bytes
*                                      according to the parameters in parameter dsaInfoPtr.
*                                      NOTE : this memory is the caller's responsibility (to
*                                      allocate / free)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrafficGeneratorPacketDsaTagBuild
(
    IN  TGF_PACKET_DSA_TAG_STC         *dsaInfoPtr,
    OUT GT_U8                          *dsaBytesPtr
)
{
    GT_STATUS   rc = GT_NOT_IMPLEMENTED;
#ifdef CHX_FAMILY
    rc =  dxchDsaTagBuild(dsaInfoPtr,dsaBytesPtr);
#else
    dsaInfoPtr = dsaInfoPtr;
    dsaBytesPtr = dsaBytesPtr;
#endif /*CHX_FAMILY*/

    return rc;
}

/**
* @internal prvTgfTrafficGeneratorPacketDsaTagParse function
* @endinternal
*
* @brief   parse the DSA tag bytes from the packet into the DSA tag parameters .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dsaBytesPtr              - pointer to the start of DSA tag in the packet.
*
* @param[out] dsaInfoPtr               - (pointer to) the DSA parameters that parsed from the packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrafficGeneratorPacketDsaTagParse
(
    IN  GT_U8                          *dsaBytesPtr,
    OUT TGF_PACKET_DSA_TAG_STC         *dsaInfoPtr
)
{
    GT_STATUS   rc = GT_NOT_IMPLEMENTED;
#ifdef CHX_FAMILY
    rc =  dxchDsaTagParse(dsaBytesPtr,dsaInfoPtr);
#else
    dsaInfoPtr = dsaInfoPtr;
    dsaBytesPtr = dsaBytesPtr;
#endif /*CHX_FAMILY*/

    return rc;
}

#ifdef ASIC_SIMULATION
extern void simulationOfFutureFix_TTI_remotePhysicalPort_origSrcPhyIsTrunk_set(GT_U32   newValue);
extern void simulationOfFutureFix_HA_origIsTrunk_reset_set(GT_U32   newValue);
extern void simulationOfFutureFix_HA_SrcIdIgnored_set(GT_U32   newValue);
#endif /*ASIC_SIMULATION*/

#ifdef DXCH_CODE

typedef enum{
    REMOTE_PORTS_EDSA_ERRATUM_ORIG_PHY_PORT_E,
    REMOTE_PORTS_EDSA_ERRATUM_ORIG_IS_TRUNK_E,
    REMOTE_PORTS_EDSA_ERRATUM_ORIG_TRUNK_ID_E,
    REMOTE_PORTS_EDSA_ERRATUM_SRC_ID_E,
}REMOTE_PORTS_EDSA_ERRATUM_ENT;

static GT_U32  testEDsaRemotePortsFixed = 0;
void prvTgfForceEDsaRemotePortsFixedSet(IN GT_U32 eDSAFixed)
{
    testEDsaRemotePortsFixed = eDSAFixed;
#ifdef ASIC_SIMULATION
    {
        GT_U32  holdErratum = testEDsaRemotePortsFixed ? 0 : 1;

        /*checking that fixes for the 2 Errata are good.*/
        simulationOfFutureFix_TTI_remotePhysicalPort_origSrcPhyIsTrunk_set(holdErratum);
        simulationOfFutureFix_HA_origIsTrunk_reset_set(holdErratum);
        simulationOfFutureFix_HA_SrcIdIgnored_set(holdErratum);
    }
#endif /*ASIC_SIMULATION*/
}

static GT_BOOL eDsaFromRemoteErratumGet(
    IN GT_U8  devNum ,
    IN REMOTE_PORTS_EDSA_ERRATUM_ENT type)
{
    switch(type)
    {
        case REMOTE_PORTS_EDSA_ERRATUM_ORIG_PHY_PORT_E:
        case REMOTE_PORTS_EDSA_ERRATUM_ORIG_IS_TRUNK_E:
        case REMOTE_PORTS_EDSA_ERRATUM_ORIG_TRUNK_ID_E:
        case REMOTE_PORTS_EDSA_ERRATUM_SRC_ID_E:
            break;
        default:
            return GT_FALSE;
    }

    if(!PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        /* remote ports are not relevant to those devices */
        return GT_FALSE;
    }

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E) &&
       (PRV_CPSS_PP_MAC(devNum)->revision > 0))
    {
        /* the Erratum was fixed */
        return GT_FALSE;
    }

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) &&
       (PRV_CPSS_PP_MAC(devNum)->revision > 0))
    {
        /* the Erratum was fixed */
        return GT_FALSE;
    }

    if(testEDsaRemotePortsFixed)
    {
        /* allow to test a'fixed' behavior on simulation of the device */
        return GT_FALSE;
    }

    /* the device not handle properly remote physical ports */
    return GT_TRUE;
}


#endif /*CHX_FAMILY*/

/**
* @internal prvTgfTrafficGeneratorPacketDsaTagGetOrigSrcPhy function
* @endinternal
*
* @brief   Get original src port (it's different than src port on remote ports).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number (either local or remote)
*                                       Original physical port number to be used at DSA tag.
*/
GT_PHYSICAL_PORT_NUM prvTgfTrafficGeneratorPacketDsaTagGetOrigSrcPhy
(
    IN  GT_U8                          devNum,
    IN  GT_PORT_NUM                    portNum
)
{
    GT_PHYSICAL_PORT_NUM           origSrcPortNum;

    origSrcPortNum = portNum;
#ifdef DXCH_CODE
    /* check if the device hold Erratum on the EDsa that hold info for packet
       ingress from 'remote physical port' */
    if(GT_FALSE == eDsaFromRemoteErratumGet(devNum,
        REMOTE_PORTS_EDSA_ERRATUM_ORIG_PHY_PORT_E))
    {
        /* the device not hold the erratum */
        return origSrcPortNum;
    }

    if (prvCpssDxChPortRemotePortCheck(devNum, portNum))
    {
        /* Original Src. Phy is at the remote port */
        GT_U32  portIndex;
        PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr;
        remotePhyMacInfoPtr = prvCpssDxChCfgPort88e1690RemotePhysicalPortInfoGet(
                                devNum,
                                portNum,
                                &portIndex);

        if(remotePhyMacInfoPtr)
        {
            origSrcPortNum =
                remotePhyMacInfoPtr->connectedPhyMacInfo.remotePortInfoArr[portIndex].remoteMacPortNum;

            PRV_TGF_LOG2_MAC("\n NOTE: (due to Erratum)expect eDSA <origPhysicalPort/trunkId> From a (remote physical port) is local port of phy1690[%d] instead of 'remote port'[%d]\n",
                origSrcPortNum,portNum);
        }
    }
#else
    devNum = devNum;
#endif /*DXCH_CODE*/

    return origSrcPortNum;
}
/**
* @internal prvTgfTrafficGeneratorPacketDsaTagGetSrcId function
* @endinternal
*
* @brief   Get SRC-ID in eDSA (it's different from expected on remote ports with erratum).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] ingressPortNum           - ingress port number (either local or remote)
* @param[in] cascadePortNum           - egress-cascade port number (local port NEVER remote)
* @param[in] srcId                    - expected SRC-ID in eDSA
*                                       actual srcId that the device will have in eDSA
*/
GT_PHYSICAL_PORT_NUM prvTgfTrafficGeneratorPacketDsaTagGetSrcId
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           ingressPortNum,
    IN  GT_PHYSICAL_PORT_NUM           cascadePortNum,
    IN  GT_U32                         srcId
)
{
#ifdef DXCH_CODE
    GT_STATUS   rc;
    GT_U32      portSrcId;
    /* check if the device hold Erratum on the EDsa that hold info for packet
       ingress from 'remote physical port' */
    if(GT_FALSE == eDsaFromRemoteErratumGet(devNum,
        REMOTE_PORTS_EDSA_ERRATUM_SRC_ID_E))
    {
        /* the device not hold the erratum */
        return srcId;
    }

    if (GT_FALSE == prvCpssDxChPortRemotePortCheck(devNum, ingressPortNum))
    {
        /* the erratum not relevant to non-remote ports */
        return srcId;
    }

    /* the src-id in the eDSA/DSA is the srcID from the egress port (the cascade port) */
    rc = cpssDxChCscdCentralizedChassisSrcIdGet(devNum,cascadePortNum,&portSrcId);
    if(rc == GT_OK)
    {
        PRV_TGF_LOG4_MAC("\n NOTE: (due to Erratum)expect DSA/eDSA <SRC-ID> From a (remote physical port[%d]) to use egress port[%d] <Centralized Chassis Src-Id> [%d] instead of [%d]\n",
            ingressPortNum,cascadePortNum,portSrcId,srcId);

        return portSrcId;
    }

#else
    devNum = devNum;
    ingressPortNum = ingressPortNum;
    cascadePortNum = cascadePortNum;
#endif /*DXCH_CODE*/

    return srcId;
}
/**
* @internal prvTgfTrafficGeneratorPacketDsaTagGetOrigIsTrunk function
* @endinternal
*
* @brief   Get origIsTrunk in eDSA (it's different from expected on remote ports with erratum).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] ingressPortNum           - ingress port number (either local or remote)
* @param[in] origIsTrunk              - expected  in eDSA
*                                       origIsTrunk to be used at DSA tag.
*/
GT_BOOL prvTgfTrafficGeneratorPacketDsaTagGetOrigIsTrunk
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           ingressPortNum,
    IN  GT_BOOL                        origIsTrunk
)
{
#ifdef DXCH_CODE
    /* check if the device hold Erratum on the EDsa that hold info for packet
       ingress from 'remote physical port' */
    if(GT_FALSE == eDsaFromRemoteErratumGet(devNum,
        REMOTE_PORTS_EDSA_ERRATUM_ORIG_IS_TRUNK_E))
    {
        /* the device not hold the erratum */
        return origIsTrunk;
    }

    if (GT_FALSE == prvCpssDxChPortRemotePortCheck(devNum, ingressPortNum))
    {
        /* the erratum not relevant to non-remote ports */
        return origIsTrunk;
    }

    if(origIsTrunk == GT_FALSE)
    {
        /* avoid printing warning */
        return origIsTrunk;
    }

    PRV_TGF_LOG1_MAC("\n NOTE: (due to Erratum)expect DSA/eDSA <origIsTrunk> From a (remote physical port[%d]) to use GT_FALSE instead of GT_TRUE\n",
        ingressPortNum);

    /* the erratum will set it as GT_FALSE */
    return GT_FALSE;

#else
    devNum = devNum;
    ingressPortNum = ingressPortNum;
    return origIsTrunk;
#endif /*DXCH_CODE*/
}


/**
* @internal prvTgfTrafficGeneratorPacketDsaTagGetOrigPhyIsTrunk function
* @endinternal
*
* @brief   Get origPhysicalIsTrunk in eDSA (it's different from expected on remote ports with erratum).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] ingressPortNum           - ingress port number (either local or remote)
* @param[in] origPhysicalIsTrunk      - expected origIsTrunk in eDSA
*                                       origPhysicalIsTrunk to be used at DSA tag.
*/
GT_BOOL prvTgfTrafficGeneratorPacketDsaTagGetOrigPhyIsTrunk
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           ingressPortNum,
    IN  GT_BOOL                        origPhysicalIsTrunk
)
{
#ifdef DXCH_CODE
    /* check if the device hold Erratum on the EDsa that hold info for packet
       ingress from 'remote physical port' */
    if(GT_FALSE == eDsaFromRemoteErratumGet(devNum,
        REMOTE_PORTS_EDSA_ERRATUM_ORIG_IS_TRUNK_E))
    {
        /* the device not hold the erratum */
        return origPhysicalIsTrunk;
    }

    if (GT_FALSE == prvCpssDxChPortRemotePortCheck(devNum, ingressPortNum))
    {
        /* the erratum not relevant to non-remote ports */
        return origPhysicalIsTrunk;
    }

    if(origPhysicalIsTrunk == GT_FALSE)
    {
        /* avoid printing warning */
        return origPhysicalIsTrunk;
    }

    PRV_TGF_LOG1_MAC("\n NOTE: (due to Erratum)expect eDSA <origPhysicalIsTrunk> From a (remote physical port[%d]) to use GT_FALSE instead of GT_TRUE\n",
        ingressPortNum);

    /* the erratum will set it as GT_FALSE */
    return GT_FALSE;

#else
    devNum = devNum;
    ingressPortNum = ingressPortNum;
    return origPhysicalIsTrunk;
#endif /*DXCH_CODE*/
}


#ifdef DXCH_CODE
/**
* @internal sprvTgfCscdPortTypeSet function
* @endinternal
*
* @brief   Configure a PP port to be a cascade port. Application is responsible
*         for setting the default values of the port.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - physical device number
*         portDirection  - port's direction:
*         CPSS_DIRECTION_INGRESS_E - source port
*         CPSS_DIRECTION_EGRESS_E - target port
*         CPSS_DIRECTION_BOTH_E  - both source and target ports
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portDirection            - port's direction:
*                                      CPSS_DIRECTION_INGRESS_E - source port
*                                      CPSS_DIRECTION_EGRESS_E  - target port
*                                      CPSS_DIRECTION_BOTH_E    - both source and target ports
*                                      APPLICABLE DEVICES: Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
* @param[in] portNum                  - The port to be configured as cascade
* @param[in] portType                 - cascade  type regular/extended DSA tag port or network port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS sprvTgfCscdPortTypeSet
(
    IN GT_U8                        devNum,
    IN CPSS_PORT_DIRECTION_ENT      portDirection,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN CPSS_CSCD_PORT_TYPE_ENT      portType
)
{

    if(portDirection != CPSS_PORT_DIRECTION_BOTH_E)
    {
        if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
        {
            return GT_NOT_SUPPORTED;
        }

        if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
        {
            /*supported*/
        }
        else
        {
            switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
            {
                case CPSS_PP_FAMILY_DXCH_LION_E   :
                case CPSS_PP_FAMILY_DXCH_LION2_E  :
                    break;
                default:
                    /* the device not supports different configuration for the ingress from egress */
                    return GT_NOT_SUPPORTED;
            }
        }
    }

#ifdef CHX_FAMILY
    if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PX_FAMILY_PIPE_E)
    {
        GT_STATUS rc;
        rc = cpssDxChCscdPortTypeSet(devNum, portNum, portDirection, portType);
        if(rc != GT_OK)
        {
            PRV_TGF_LOG1_MAC("[TGF]: cpssDxChCscdPortTypeSet FAILED, rc = [%d]", rc);
        }
        return rc;
    }
#endif /* CHX_FAMILY */

    portNum = portNum;
    portType = portType;

    return GT_NOT_IMPLEMENTED;
}

/**
* @internal tgfTrafficGeneratorIngressCscdPortEnableSet function
* @endinternal
*
* @brief   enable/disable the port to assume that ingress traffic hold DSA tag
*         info.
*         NOTE: relevant only to devices that support detach of ingress and
*         egress cascade configurations.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - The port to be configured as cascade enable/disabled
* @param[in] enable                   - enable/disable the ingress DSA tag processing
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note this API needed to support testing of DSA tag processing by the device
*       (DSA that not processed on the ingress of the CPU_PORT)
*
*/
GT_STATUS tgfTrafficGeneratorIngressCscdPortEnableSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN GT_BOOL                      enable
)
{
    CPSS_CSCD_PORT_TYPE_ENT portType =  (enable == GT_TRUE) ?
            CPSS_CSCD_PORT_DSA_MODE_1_WORD_E : /*dummy value --> should be different then CPSS_CSCD_PORT_NETWORK_E */
            CPSS_CSCD_PORT_NETWORK_E;

    return sprvTgfCscdPortTypeSet(devNum,CPSS_PORT_DIRECTION_RX_E,portNum,portType);
}

/**
* @internal tgfTrafficGeneratorEgressCscdPortEnableSet function
* @endinternal
*
* @brief   enable/disable the port to assume that egress traffic hold DSA tag
*         info.
*         NOTE: relevant only to devices that support detach of ingress and
*         egress cascade configurations.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - The port to be configured as cascade enable/disabled
* @param[in] enable                   - enable/disable the ingress DSA tag processing
* @param[in] cscdPortType             - type of cascade port (APPLICABLE VALUES:
*                                      CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E,
*                                      CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E,
*                                      CPSS_CSCD_PORT_NETWORK_E).
*                                      Relevant only when enable is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note this API needed to support testing of DSA tag processing by the device
*       (DSA that not processed on the ingress of the CPU_PORT)
*
*/
GT_STATUS tgfTrafficGeneratorEgressCscdPortEnableSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN GT_BOOL                      enable,
    IN CPSS_CSCD_PORT_TYPE_ENT      cscdPortType
)
{
    if ((cscdPortType != CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E) &&
        (cscdPortType != CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E) &&
        (cscdPortType != CPSS_CSCD_PORT_NETWORK_E))
    {
        return GT_BAD_PARAM;
    }

    if (enable == GT_FALSE)
    {
        cscdPortType = CPSS_CSCD_PORT_NETWORK_E;
    }

    return sprvTgfCscdPortTypeSet(devNum, CPSS_PORT_DIRECTION_TX_E, portNum, cscdPortType);
}
#endif /*DXCH_CODE*/

/**
* @internal tgfTrafficGeneratorPortTxEthCaptureCompare function
* @endinternal
*
* @brief   compare captured packets with the input packet format (+VFD array)
*         NOTE: the function not care about reason why packet reached the CPU ,
*         is it due to 'capture' or other reason.
* @param[in] portInterfacePtr         - (pointer to) port/trunk interface
*                                      NOTE: can be NULL --> meaning 'any interface'
* @param[in] packetInfoPtr            - the packet format to compare the captured packet with.
* @param[in] numOfPackets             - number of packets expected to be captured on the interface (can be 0)
* @param[in] numVfd                   - number of VFDs (can be 0)
* @param[in] vfdArray[]               - pointer to array of VFDs (can be NULL when numVfd == 0)
* @param[in] byteNumMaskList[]        - pointer to array of bytes for which the comparison
*                                      is prohibited (can be NULL when byteNumMaskListLen == 0)
*                                      supports both single number elements and
*                                      range elements in format (low | (high << 16))
* @param[in] byteNumMaskListLen       - number of members in byteNumMaskList(can be 0)
*
* @param[out] actualCapturedNumOfPacketsPtr - (pointer to) actual number of packet captured from
*                                      the interface
* @param[out] onFirstPacketNumTriggersBmpPtr - (pointer to) bitmap of triggers - that
*                                      relate only to first packet  (can be NULL when numVfd == 0)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Triggers bitmap has the following structure:
*       K - number of VFDs (equal to numVfd)
*       |   Packet0   |
*       +----+----+---+----+
*       |Bit0|Bit1|...|BitK|
*       +----+----+---+----+
*       Bit[i] to 1 means that VFD[i] is matched for first captured packet .
*
*/
GT_STATUS tgfTrafficGeneratorPortTxEthCaptureCompare
(
    IN  CPSS_INTERFACE_INFO_STC     *portInterfacePtr,
    IN  TGF_PACKET_STC      *packetInfoPtr,
    IN  GT_U32              numOfPackets,
    IN  GT_U32              numVfd,
    IN  TGF_VFD_INFO_STC    vfdArray[],
    IN  GT_U32              byteNumMaskList[],
    IN  GT_U32              byteNumMaskListLen,
    OUT GT_U32              *actualCapturedNumOfPacketsPtr,
    OUT GT_U32              *onFirstPacketNumTriggersBmpPtr
)
{
    GT_U8       *expectedPacketBufferPtr = NULL;
    TGF_AUTO_CHECKSUM_FULL_INFO_STC checksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* auto checksum info */
    TGF_AUTO_CHECKSUM_EXTRA_INFO_STC extraChecksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* extra checksum info */
    GT_U32      expectedPacketNumOfBytesInPacketBuff;/* number of bytes in packetBuff*/ /* save space of CRC*/
    GT_U32      numOfMismatch;/* number of bytes mismatched */
    GT_U32      lastMismatchByteIndex;/* byte index of last mismatch ... in order to manage sequences of mismatch */
    GT_U32      lastMismatchStartByteIndex;/* byte index of start of last mismatch ... in order to manage sequences of mismatch */
    GT_U32      lastMismatchRecorded;/* last mismatch that we recorded */
    GT_STATUS       rc1      = GT_OK;
    GT_BOOL     useMii;/* do we use MII or SDMA */
    GT_BOOL     useEmunatedSdma = GT_FALSE;/* do we use MII but with emulated SDMA */
    GT_U8           devNum   = 0;
    GT_U8           queue    = 0;
    GT_BOOL         getFirst = GT_TRUE;
    GT_U32          i        = 0;
    GT_BOOL         match    = GT_FALSE;
    GT_STATUS       rc       = GT_OK;
    GT_U32          buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32          packetActualLength = 0;
    GT_U32          lengthTolerance = 0;
    GT_U32          packetCount = 0;
    GT_U8           packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    TGF_NET_DSA_STC rxParam = {0};


#ifdef GM_USED
    GT_U32          maxWaitTime     = 1200; /* first packet wait time */
    GT_U32          nextWaitTime    = 200;
    GT_U32          delayTime       = 100;
#endif /*GM_USED*/
    GT_U32          origLogPrintMode;
    GT_U32          didError;


#ifdef GM_USED
    if(PRV_CPSS_PP_MAC(prvTgfCpuDevNum)->devFamily == CPSS_PP_FAMILY_PUMA3_E)
    {
        maxWaitTime     = prvTgfGmTimeFactor*100; /* first packet wait time */
        nextWaitTime    = prvTgfGmTimeFactor*50;
        delayTime       = prvTgfGmTimeFactor*20;
    }
#endif /*GM_USED*/

#ifdef EXMXPM_FAMILY
    useMii = GT_FALSE;
#else
        #if defined CHX_FAMILY
            useMii =((PRV_CPSS_PP_MAC(prvTgfCpuDevNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E) ? GT_TRUE:
                     (PRV_CPSS_PP_MAC(prvTgfCpuDevNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_NONE_E) ? GT_FALSE:
                     PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(prvTgfCpuDevNum) ? GT_TRUE : GT_FALSE);

            useEmunatedSdma =((PRV_CPSS_PP_MAC(prvTgfCpuDevNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E) ? GT_FALSE:
                              useMii);
        #else
            useMii = GT_FALSE;
        #endif
#endif

    CPSS_NULL_PTR_CHECK_MAC(actualCapturedNumOfPacketsPtr);

    if (numVfd != 0)
    {
        CPSS_NULL_PTR_CHECK_MAC(vfdArray);
        CPSS_NULL_PTR_CHECK_MAC(onFirstPacketNumTriggersBmpPtr);

        (*onFirstPacketNumTriggersBmpPtr) = 0;
    }

    if (portInterfacePtr != NULL &&
        portInterfacePtr->type != CPSS_INTERFACE_PORT_E &&
        portInterfacePtr->type != CPSS_INTERFACE_TRUNK_E)

    {
        return GT_BAD_PARAM;
    }

    cpssOsMemSet(checksumInfoArr,0,sizeof(checksumInfoArr));

    /* get buffer from the pool */
    expectedPacketBufferPtr = gtPoolGetBuf(prvTgfTxBuffersPoolId);
    if(NULL == expectedPacketBufferPtr)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    expectedPacketNumOfBytesInPacketBuff = TGF_RX_BUFFER_MAX_SIZE_CNS - 4;/* save space of CRC*/
    /* build the basic frame info */                            /* save 4 bytes space for CRC */
    rc = tgfTrafficEnginePacketBuild(packetInfoPtr, expectedPacketBufferPtr ,&expectedPacketNumOfBytesInPacketBuff, checksumInfoArr,extraChecksumInfoArr);
    if(rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    if((expectedPacketNumOfBytesInPacketBuff + TGF_CRC_LEN_CNS) > TGF_TX_BUFFER_MAX_SIZE_CNS)
    {
        rc =  GT_BAD_PARAM;
        goto exit_cleanly_lbl;
    }

    while(1)
    {
        didError = GT_FALSE;

        /* disable printings , so only the relevant packets will be printed */
        origLogPrintMode = tgfLogPrintModeFinal();

        buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
        /* get entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_ANY_E,
                                           getFirst, GT_TRUE, packetBuff,
                                           &buffLen, &packetActualLength,
                                           &devNum, &queue, &rxParam);
        if(rc != GT_OK)
        {
            PRV_TGF_LOG1_MAC("FAIL: tgfTrafficGeneratorRxInCpuGet - rc[%d]\n", rc);
        }

        /* restore the print */
        tgfLogPrintModeRestore(origLogPrintMode);

#ifdef GM_USED
        /* wait for yet not reached packets               */
        if ((rc == GT_NO_MORE) && (maxWaitTime >= delayTime))
        {
            maxWaitTime -= delayTime;
            cpssOsTimerWkAfter(delayTime);
            continue;
        }
        /* for the fist packet we are ready to wait       */
        /* sevelal delays, for the nexts no more then one */
        maxWaitTime = nextWaitTime;
#endif /*GM_USED*/

        if (rc != GT_OK)
        {
            break;
        }

        getFirst = GT_FALSE; /* now we get the next */

        if(GT_FALSE ==
            checkPacketFromExpectedInterface(portInterfacePtr,&rxParam))
        {
            /* packet not came from expected interface */
            continue;
        }

        /* calculate number of captured packets */
        packetCount++;

        {
            for (i = 0; i < numVfd ;i++)
            {
                vfdArray[i].modeExtraInfo = (packetCount - 1);

                /* over ride the buffer with VFD info */
                rc = tgfTrafficEnginePacketVfdApply(&vfdArray[i], expectedPacketBufferPtr, expectedPacketNumOfBytesInPacketBuff);
                if(rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }
            }

            /* apply auto checksum fields after applying the VFD*/
            rc = autoChecksumField(checksumInfoArr,extraChecksumInfoArr);
            if(rc != GT_OK)
            {
                goto exit_cleanly_lbl;
            }

            /* check the VFD parameters */
            for (i = 0; i < numVfd ;i++)
            {
                rc = tgfTrafficEnginePacketVfdCheck(
                    &vfdArray[i], packetBuff, packetActualLength, &match);

                if(onFirstPacketNumTriggersBmpPtr)
                {
                    /* set appropriate bit to 1 if VFD is matched */
                    (*onFirstPacketNumTriggersBmpPtr) |= ((GT_U32) match) << (i + (packetCount - 1) * numVfd);
                }
            }
        }

        if(packetActualLength == (expectedPacketNumOfBytesInPacketBuff + 4))
        {
            /* assume that the expected packet did not added CRC to calculations */
            expectedPacketNumOfBytesInPacketBuff += 4;
        }


        /* compare the packets length */
        if(packetActualLength != expectedPacketNumOfBytesInPacketBuff)
        {
            rc1 = GT_BAD_SIZE;
            PRV_TGF_LOG4_MAC("tgfTrafficGeneratorPortTxEthCaptureCompare: on [%s][%d] expected packet length [%d] , got packet length [%d] \n",
                              portInterfacePtr == NULL ? "any" : portInterfacePtr->type == CPSS_INTERFACE_PORT_E ? "port" : "trunk",
                              portInterfacePtr == NULL ? -1 : portInterfacePtr->type == CPSS_INTERFACE_PORT_E ? portInterfacePtr->devPort.portNum : portInterfacePtr->trunkId,
                              expectedPacketNumOfBytesInPacketBuff , packetActualLength);
            didError = GT_TRUE;
        }

        numOfMismatch = 0;
        lastMismatchByteIndex = 0;
        lastMismatchStartByteIndex = 0;
        lastMismatchRecorded = 0;

        if(useMii == GT_TRUE && useEmunatedSdma == GT_TRUE)
        {
            /* all the TO_CPU (in SDMA) come with 0x55555555 as CRC , so we not check it */
            lengthTolerance = 4;
        }
        else if(useMii == GT_FALSE && packetActualLength >= 4)
        {
            /* all the TO_CPU (in SDMA) come with 0x55555555 as CRC , so we not check it */
            lengthTolerance = 4;
        }

        /* compare the full packet */
        for(i = 0 ; (i < (packetActualLength - lengthTolerance)) && (i < expectedPacketNumOfBytesInPacketBuff)  ; i++)
        {
            if ((!internalIsNumInList(i, byteNumMaskList, byteNumMaskListLen)) &&
                (packetBuff[i] != expectedPacketBufferPtr[i]))
            {
                didError = GT_TRUE;

                if( numOfMismatch == 0)/* first mismatch */
                {
                    /* new mismatch sequence */
                    numOfMismatch++;
                    lastMismatchStartByteIndex = i;
                }
                else if((lastMismatchByteIndex + 1) == i)
                {
                    /* we are in the same mismatch sequence of the last one */
                    /* so update the last byte of this sequence */
                }
                else
                {
                    /* new mismatch sequence */
                    numOfMismatch++;
                    lastMismatchStartByteIndex = i;
                }

                lastMismatchByteIndex = i;
            }
            else /* current byte match between the packets */
            {
                /* when recognize end of mismatch sequence , print it */
                if(numOfMismatch < 10 &&
                   (lastMismatchRecorded != numOfMismatch) )
                {
                    GT_U32  iter;
                    GT_U8*  bufferPtr;
                    GT_U32  length;
                    /* record the last label */
recordLastMismatch_lbl:
                    lastMismatchRecorded = numOfMismatch;

                    bufferPtr = &packetBuff[lastMismatchStartByteIndex];
                    length = (lastMismatchByteIndex - lastMismatchStartByteIndex + 1);

                    rc1 = GT_BAD_VALUE;

                    PRV_TGF_LOG3_MAC("[TGF]: tgfTrafficGeneratorPortTxEthCaptureCompare: numOfMismatch[%d] :"
                        " bytes from captured packet from byte[%d] to byte [%d]\n",
                        numOfMismatch,
                        lastMismatchStartByteIndex,
                        lastMismatchByteIndex);

                    for(iter = 0; iter < length ; iter++)
                    {
                        if((iter & 0x0F) == 0)
                        {
                            PRV_TGF_LOG1_MAC("0x%4.4x :", iter);
                        }

                        PRV_TGF_LOG1_MAC(" %2.2x", bufferPtr[iter]);

                        if((iter & 0x0F) == 0x0F)
                        {
                            PRV_TGF_LOG0_MAC("\n");
                        }
                    }/*iter*/
                    PRV_TGF_LOG0_MAC("\n");

                    bufferPtr = &expectedPacketBufferPtr[lastMismatchStartByteIndex];

                    PRV_TGF_LOG0_MAC("[TGF]: bytes from expected packet \n");

                    for(iter = 0; iter < length ; iter++)
                    {
                        if((iter & 0x0F) == 0)
                        {
                            PRV_TGF_LOG1_MAC("0x%4.4x :", iter);
                        }

                        PRV_TGF_LOG1_MAC(" %2.2x", bufferPtr[iter]);

                        if((iter & 0x0F) == 0x0F)
                        {
                            PRV_TGF_LOG0_MAC("\n");
                        }
                    }/*iter*/
                    PRV_TGF_LOG0_MAC("\n");

                }/*numOfMismatch < 10*/
            }/*else --> current byte match between the packets */
        }/*for(i...)*/

        /* when recognize end of mismatch sequence , print it */
        if(numOfMismatch < 10 &&
           (lastMismatchRecorded != numOfMismatch) )
        {
            goto recordLastMismatch_lbl;
        }


        if(didError || packetCount == 1)
        {
            PRV_TGF_LOG2_MAC("tgfTrafficGeneratorPortTxEthCaptureCompare: the received packet on interface[%s][%d]: \n",
                portInterfacePtr == NULL ? "any" :portInterfacePtr->type == CPSS_INTERFACE_PORT_E ? "port" : "trunk" ,
                portInterfacePtr == NULL ? -1    :portInterfacePtr->type == CPSS_INTERFACE_PORT_E ? portInterfacePtr->devPort.portNum : portInterfacePtr->trunkId);
            tgfTrafficTracePacket(packetBuff, packetActualLength, GT_TRUE);

            if(didError)
            {
                PRV_TGF_LOG0_MAC("tgfTrafficGeneratorPortTxEthCaptureCompare: the expected packet : \n");
                tgfTrafficTracePacket(expectedPacketBufferPtr, expectedPacketNumOfBytesInPacketBuff, GT_TRUE);
            }
            else
            {
                PRV_TGF_LOG0_MAC("tgfTrafficGeneratorPortTxEthCaptureCompare: the expected packet -- the same as received ! \n");
            }
        }

    }/*while(1)*/


    if( numOfPackets != packetCount )
    {
        PRV_TGF_LOG4_MAC("tgfTrafficGeneratorPortTxEthCaptureCompare: on [%s][%d] expected [%d] packets , got [%d] packets",
                      portInterfacePtr == NULL ? "any" : portInterfacePtr->type == CPSS_INTERFACE_PORT_E ? "port" : "trunk",
                      portInterfacePtr == NULL ? -1    : portInterfacePtr->type == CPSS_INTERFACE_PORT_E ? portInterfacePtr->devPort.portNum : portInterfacePtr->trunkId,
                      numOfPackets, packetCount);

        /* the caller UT must fail if numOfPackets != packetCount */
        rc = GT_FAIL;
    }

exit_cleanly_lbl:

    /* restore default */
    tgfTrafficGeneratorExpectedInterfaceEport(PRV_TGF_EXPECTED_INTERFACE_TYPE_PHYSICAL_PORT_NUM_E);

    if(expectedPacketBufferPtr)
    {
        /* release the buffer back to the pool */
        gtPoolFreeBuf(prvTgfTxBuffersPoolId, expectedPacketBufferPtr);
    }

    PRV_TGF_LOG0_MAC("\n\n");
    *actualCapturedNumOfPacketsPtr = packetCount;

    if(rc == GT_NO_MORE)
    {
        rc = GT_OK;
    }

    return (rc1 != GT_OK) ? rc1 : rc;
}

/**
* @internal tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet function
* @endinternal
*
* @brief   Get entry from rxNetworkIf table , for expected interface only.
*         similar to tgfTrafficGeneratorRxInCpuGet , but filtering the packets
*         from non relevant interfaces.
* @param[in] portInterfacePtr         - (pointer to) port/trunk interface
*                                      NOTE: can be NULL --> meaning 'any interface'
* @param[in] packetType               - the packet type to get
* @param[in] getFirst                 - get first/next entry
* @param[in] trace                    - enable\disable packet tracing
* @param[in,out] packetBufLenPtr          - the length of the user space for the packet
*
* @param[out] packetBufPtr             - packet's buffer (pre allocated by the user)
* @param[in,out] packetBufLenPtr          - length of the copied packet to gtBuf
* @param[out] packetLenPtr             - Rx packet original length
* @param[out] devNumPtr                - packet's device number
* @param[out] queuePtr                 - Rx queue in which the packet was received.
* @param[out] rxParamsPtr              - specific device Rx info format.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - on more entries
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note For 'captured' see API tgfTrafficGeneratorPortTxEthCaptureSet
*
*/
GT_STATUS tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN    TGF_PACKET_TYPE_ENT   packetType,
    IN    GT_BOOL               getFirst,
    IN    GT_BOOL               trace,
    OUT   GT_U8                *packetBufPtr,
    INOUT GT_U32               *packetBufLenPtr,
    OUT   GT_U32               *packetLenPtr,
    OUT   GT_U8                *devNumPtr,
    OUT   GT_U8                *queuePtr,
    OUT   TGF_NET_DSA_STC      *rxParamsPtr
)
{
    GT_STATUS   rc;
    GT_BOOL     fromExpectedInterface;

    do{
        rc = tgfTrafficGeneratorRxInCpuGet(packetType,getFirst,trace,
                        packetBufPtr,packetBufLenPtr,
                        packetLenPtr,devNumPtr,queuePtr,rxParamsPtr);
        if(rc != GT_OK)
        {
            /* error / no more packets */
            return rc;
        }

        getFirst = GT_FALSE; /* now we get the next */

        fromExpectedInterface =
            checkPacketFromExpectedInterface(portInterfacePtr,rxParamsPtr);
    }/* skip packets from non wanted interfaces */
    while(fromExpectedInterface == GT_FALSE);


    /* restore default */
    tgfTrafficGeneratorExpectedInterfaceEport(PRV_TGF_EXPECTED_INTERFACE_TYPE_PHYSICAL_PORT_NUM_E);

    return GT_OK;
}

/**
* @internal tgfTrafficGeneratorStatDump function
* @endinternal
*
* @brief   Dump statistic of traffic generator
*/
GT_VOID tgfTrafficGeneratorStatDump
(
    GT_VOID
)
{
    cpssOsPrintf(" Traffic Generator: Packet Sent OK %d\n", prvTgfTrafficGeneratorNumberPacketsSendOk);
    cpssOsPrintf(" Traffic Generator: Packet Sent Failed %d\n", prvTgfTrafficGeneratorNumberPacketsSendFailed);
}

/**
* @internal prvTgfTrafficPrintPacketTxEnableSet function
* @endinternal
*
* @brief   Enable or Disable printing of TX Packet event
*
* @param[in] printEnable              - GT_FALSE - disable
*                                      GT_TRUE  - enable
*                                       previous state
*/
GT_BOOL prvTgfTrafficPrintPacketTxEnableSet
(
    GT_BOOL printEnable
)
{
    GT_BOOL prevState = prvTgfTrafficPrintPacketTxEnable;
    prvTgfTrafficPrintPacketTxEnable = printEnable;
    return prevState;
}

/**
* @internal prvTgfTrafficPrintPacketTxEnableGet function
* @endinternal
*
* @brief   Returns Enable or Disable printing of TX Packet event
*
* @retval GT_FALSE                 - disable
* @retval GT_TRUE                  - enable
*/
GT_BOOL prvTgfTrafficPrintPacketTxEnableGet(GT_VOID)
{
    return prvTgfTrafficPrintPacketTxEnable;
}


extern GT_VOID prvTgfTrafficTableRxPacketSystemReset(GT_VOID);
/**
* @internal tgfTrafficGeneratorSystemReset function
* @endinternal
*
* @brief   Preparation for system reset
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID tgfTrafficGeneratorSystemReset
(
    GT_VOID
)
{
    prvTgfTrafficTableRxPacketSystemReset();
    if(prvTgfTxBuffersPoolId)
    {
        gtPoolDeletePool(prvTgfTxBuffersPoolId);
        prvTgfTxBuffersPoolId = 0;
    }

    /* we will need new DMA allocations after system reset */
    generatorPoolInit = GT_FALSE;
}

/**
* @internal tgfTrafficGeneratorPortTxEthCaptureDoLoopbackSet function
* @endinternal
*
* @brief   indication for function tgfTrafficGeneratorPortTxEthCaptureSet(...)
*         to allow loopback on the 'captured port' or not.
*/
GT_VOID tgfTrafficGeneratorPortTxEthCaptureDoLoopbackSet
(
    IN GT_BOOL      doLoopback
)
{
    txEthCaptureDoLoopback =   doLoopback;
}

/**
* @internal tgfTrafficGeneratorPortTxEthCaptureWaitAfterDisableLoopbackSet function
* @endinternal
*
* @brief   indication for function tgfTrafficGeneratorPortTxEthCaptureSet(...)
*         to wait some time after loopback disable. This provides time to handle
*         all packets those already captured by device.
* @param[in] waitAfterDisableLoopback - enable/disable wait
*                                       None.
*/
GT_VOID tgfTrafficGeneratorPortTxEthCaptureWaitAfterDisableLoopbackSet
(
    IN GT_BOOL      waitAfterDisableLoopback
)
{
    txEthCaptureWaitAfterDisableLoopback =   waitAfterDisableLoopback;
}

/**
* @internal prvTgfTxContModeEntryGet function
* @endinternal
*
* @brief   Get entry from transmit continuous mode DB
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] modePtr                  - (pointer to) mode entry
* @param[out] idxPtr                   - (pointer to) entry index
* @param[out] freeIdxPtr               - (pointer to) free entry index
*
* @retval GT_OK                    - on success.
* @retval GT_NO_SUCH               - on entry not exist.
*/
GT_STATUS prvTgfTxContModeEntryGet
(
    IN GT_U8                            devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT PRV_TGF_TX_DEV_PORT_STC         **modePtr,
    OUT GT_U32                          *idxPtr,
    OUT GT_U32                          *freeIdxPtr
)
{
    GT_U32 freeIndex = PRV_TGF_TX_PORTS_MAX_NUM_CNS; /* index of first free entry */
    GT_U32 ii; /* iterator */
    GT_BOOL found = GT_FALSE; /* is entry found */
    GT_U32 foundIdx = 0; /* index of found entry */
    static PRV_TGF_TX_DEV_PORT_STC dummyEntry = {0};

    *idxPtr = 0;
    if(modePtr)
    {
        *modePtr = &dummyEntry;/* fix Klockwork issue 5287 */
    }

    /* check bitmap */
    if (txDevPortContUsedBmp == 0)
    {
        if(freeIdxPtr)
        {
            *freeIdxPtr = 0;
        }
        if(modePtr)
        {
            *modePtr = &(txDevPortArr[0]);
        }
        return GT_NO_SUCH;
    }

    /* find index in the DB */
    for (ii = 0; ii < PRV_TGF_TX_PORTS_MAX_NUM_CNS; ii++)
    {
        if (txDevPortContUsedBmp & (1 << ii))
        {
            /* check devNum and portNum */
            if ((txDevPortArr[ii].devNum == devNum) &&
                (txDevPortArr[ii].portNum == portNum))
            {
                /* entry found */
                found = GT_TRUE;
                foundIdx = ii;
            }
        }
        else
        {
            freeIndex = ii;
        }
    }

    if(freeIdxPtr)
    {
       *freeIdxPtr = freeIndex;
    }

   if (freeIndex != PRV_TGF_TX_PORTS_MAX_NUM_CNS)
   {
        if(modePtr)
        {
           *modePtr = &(txDevPortArr[freeIndex]);
        }
   }

   if (found == GT_FALSE)
       return GT_NO_SUCH;

    if(modePtr)
    {
       *modePtr = &(txDevPortArr[foundIdx]);
    }
   *idxPtr = foundIdx;

   return GT_OK;
}

/**
* @internal prvTgfTxModeSetupEth function
* @endinternal
*
* @brief   Transmit mode configuration
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
*/
GT_STATUS prvTgfTxModeSetupEth
(
    IN GT_U8                            devNum,
    IN GT_PHYSICAL_PORT_NUM             portNum,
    IN PRV_TGF_TX_MODE_ENT              mode,
    IN PRV_TGF_TX_CONT_MODE_UNIT_ENT    unitsType,
    IN GT_U32                           units
)
{
#ifdef DXCH_CODE
    if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PX_FAMILY_PIPE_E)
    {
        GT_U32                      freeIndex;
        GT_U32                      index;
        PRV_TGF_TX_DEV_PORT_STC     *modePtr;
        CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC   analyzerInf;
        GT_STATUS                   rc;

        /* get DB entry  */
        rc = prvTgfTxContModeEntryGet(devNum, portNum, &modePtr, &index, &freeIndex);
        switch (mode)
        {
            case PRV_TGF_TX_MODE_SINGLE_BURST_E:
                prvTgfFwsUse = 0;
                if (rc != GT_OK)
                {
                    /* port already in burst mode or FWS mode */
                    return GT_OK;
                }

                /* mark entry as free*/
                PRV_TGF_TX_MODE_FREE_DB_ENTRY(index);

                if (!PRV_TRG_TX_MODE_DB_NOT_EMPTY)
                {
                    if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        /* restore mirroring mode */
                        rc = cpssDxChMirrorToAnalyzerForwardingModeSet(devNum,mirrModeStore);
                        if (rc != GT_OK)
                        {
                            PRV_TGF_LOG1_MAC("[TGF]: cpssDxChMirrorToAnalyzerForwardingModeSet FAILED, rc = [%d]", rc);
                            return rc;
                        }

                        /* restore Rx descriptors limits */
                        rc = cpssDxChPortTxSniffedPcktDescrLimitSet(devNum,
                                                                mirrRxDescStore,
                                                                mirrTxDescStore);
                        if (rc != GT_OK)
                        {
                            PRV_TGF_LOG1_MAC("[TGF]: cpssDxChPortTxSniffedPcktDescrLimitSet FAILED, rc = [%d]", rc);
                            return rc;
                        }

                        /* restore Rx buffers limits */
                        rc = cpssDxChPortTxSniffedBuffersLimitSet(devNum,
                                                                mirrRxDescStore,
                                                                mirrTxDescStore);
                        if ((rc != GT_OK) && (rc != GT_NOT_APPLICABLE_DEVICE))
                        {
                            PRV_TGF_LOG1_MAC("[TGF]: cpssDxChPortTxSniffedBuffersLimitSet FAILED, rc = [%d]", rc);
                            return rc;
                        }
                    }

                    /* restore mirroring enhanced priority mode */
                    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E ||
                    PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
                    {
                        rc = cpssDxChMirrorEnhancedMirroringPriorityModeSet(devNum, mirrPriorityMode);
                        if ((rc != GT_OK) && (rc != GT_NOT_APPLICABLE_DEVICE))
                        {
                            PRV_TGF_LOG1_MAC("[TGF]: cpssDxChMirrorEnhancedMirroringPriorityModeSet FAILED, rc = [%d]", rc);
                            return rc;
                        }
                    }
                }

                {
                    /* restore analyzer */
                    CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC cpss_analyzerInf;
                    cpss_analyzerInf.interface = modePtr->analyzerInf.interface;
                    rc = cpssDxChMirrorAnalyzerInterfaceSet(devNum,index,&cpss_analyzerInf);
                    if (rc != GT_OK)
                    {
                        PRV_TGF_LOG1_MAC("[TGF]: cpssDxChMirrorAnalyzerInterfaceSet FAILED, rc = [%d]", rc);
                        return rc;
                    }
                }

                break;
            case PRV_TGF_TX_MODE_CONTINUOUS_E:
                prvTgfFwsUse = 0;
                if (rc == GT_OK)
                {
                    /* port is already in the mode.
                    update DB entry. */
                    modePtr->units = units;
                    modePtr->unitsType = unitsType;
                    return GT_OK;
                }

                if (freeIndex == PRV_TGF_TX_PORTS_MAX_NUM_CNS)
                {
                    /* there is no place for new port */
                    return GT_FULL;
                }

                if (!PRV_TRG_TX_MODE_DB_NOT_EMPTY)
                {
                    if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        /* this is first port in continuous mode.
                        configure mirroring.                    */
                        /* store mode */
                        rc = cpssDxChMirrorToAnalyzerForwardingModeGet(devNum, &mirrModeStore);
                        if (rc != GT_OK)
                        {
                            PRV_TGF_LOG1_MAC("[TGF]: cpssDxChMirrorToAnalyzerForwardingModeGet FAILED, rc = [%d]", rc);
                            return rc;
                        }

                        /* set source based mode */
                        rc = cpssDxChMirrorToAnalyzerForwardingModeSet(devNum,CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E);
                        if (rc != GT_OK)
                        {
                            PRV_TGF_LOG1_MAC("[TGF]: cpssDxChMirrorToAnalyzerForwardingModeSet FAILED, rc = [%d]", rc);
                            return rc;
                        }

                        /* store Tx and Rx descriptors limits */
                        rc = cpssDxChPortTxSniffedPcktDescrLimitGet(devNum, &mirrRxDescStore,&mirrTxDescStore);
                        if (rc != GT_OK)
                        {
                            PRV_TGF_LOG1_MAC("[TGF]: cpssDxChPortTxSniffedPcktDescrLimitGet FAILED, rc = [%d]", rc);
                            return rc;
                        }

                        /* set Rx descriptors limits to be WS burst * number of mirrored ports */
                        rc = cpssDxChPortTxSniffedPcktDescrLimitSet(devNum,
                                                                prvTgfWsBurst * PRV_TGF_TX_PORTS_MAX_NUM_CNS,
                                                                mirrTxDescStore);
                        if (rc != GT_OK)
                        {
                            PRV_TGF_LOG1_MAC("[TGF]: cpssDxChPortTxSniffedPcktDescrLimitSet FAILED, rc = [%d]", rc);
                            return rc;
                        }

                        /* set Rx buffers limits to be WS burst * number of mirrored ports */
                        rc = cpssDxChPortTxSniffedBuffersLimitSet(devNum,
                                                                prvTgfWsBurst * PRV_TGF_TX_PORTS_MAX_NUM_CNS,
                                                                mirrTxDescStore);
                        if ((rc != GT_OK) && (rc != GT_NOT_APPLICABLE_DEVICE))
                        {
                            PRV_TGF_LOG1_MAC("[TGF]: cpssDxChPortTxSniffedBuffersLimitSet FAILED, rc = [%d]", rc);
                            return rc;
                        }
                    }

                    /* xCat and xCat3 device has weak mirroring duplication arbitrer with default settings.
                    Need to provide priority to duplicated traffic for stable WS generation. */
                    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E ||
                    PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
                    {
                        /* store mirroring priority mode */
                        rc = cpssDxChMirrorEnhancedMirroringPriorityModeGet(devNum,&mirrPriorityMode);
                        /* first Xcat revisions does not support the feature - ignore error */
                        if ((rc != GT_OK) && (rc != GT_NOT_APPLICABLE_DEVICE))
                        {
                            PRV_TGF_LOG1_MAC("[TGF]: cpssDxChMirrorEnhancedMirroringPriorityModeGet FAILED, rc = [%d]", rc);
                            return rc;
                        }

                        if(rc == GT_OK)
                        {
                            rc = cpssDxChMirrorEnhancedMirroringPriorityModeSet(devNum,CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_SP_INGRESS_EGRESS_MIRROR_E);
                            if (rc != GT_OK)
                            {
                                PRV_TGF_LOG1_MAC("[TGF]: cpssDxChMirrorEnhancedMirroringPriorityModeSet FAILED, rc = [%d]", rc);
                                return rc;
                            }
                        }
                    }
                }

                /* mark entry as allocated */
                PRV_TGF_TX_MODE_ALLOC_DB_ENTRY(freeIndex);
                modePtr->devNum = devNum;
                modePtr->portNum = portNum;
                modePtr->units = units;
                modePtr->unitsType = unitsType;


                {
                    CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC cpss_analyzerInf;
                    /* store analyzer interface */
                    rc = cpssDxChMirrorAnalyzerInterfaceGet(devNum, freeIndex, &cpss_analyzerInf);
                    modePtr->analyzerInf.interface = cpss_analyzerInf.interface;
                    if (rc != GT_OK)
                    {
                        PRV_TGF_LOG1_MAC("[TGF]: cpssDxChMirrorAnalyzerInterfaceGet FAILED, rc = [%d]", rc);
                        return rc;
                    }
                }

                analyzerInf.interface.type = CPSS_INTERFACE_PORT_E;
                analyzerInf.interface.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);
                analyzerInf.interface.devPort.portNum = portNum;

                /* set analyzer to be Tx port */
                rc = cpssDxChMirrorAnalyzerInterfaceSet(devNum,freeIndex,&analyzerInf);
                if (rc != GT_OK)
                {
                    PRV_TGF_LOG1_MAC("[TGF]: cpssDxChMirrorAnalyzerInterfaceSet FAILED, rc = [%d]", rc);
                    return rc;
                }
                break;
            case PRV_TGF_TX_MODE_CONTINUOUS_FWS_E:
                prvTgfFwsUse = 1;
                break;
            default: return GT_BAD_PARAM;
        }
    }
    return GT_OK;
#endif /* DXCH_CODE */
#ifdef PX_CODE
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PX_FAMILY_PIPE_E)
    {
        GT_U32                      freeIndex;
        GT_U32                      index;
        PRV_TGF_TX_DEV_PORT_STC     *modePtr;

        /* get DB entry  */
        prvTgfTxContModeEntryGet(devNum, portNum, &modePtr, &index, &freeIndex);
        switch (mode)
        {
            case PRV_TGF_TX_MODE_SINGLE_BURST_E:
                break;
            case PRV_TGF_TX_MODE_CONTINUOUS_E:
            case PRV_TGF_TX_MODE_CONTINUOUS_FWS_E:
                prvTgfFwsUse = 1;

                /* mark entry as allocated */
                PRV_TGF_TX_MODE_ALLOC_DB_ENTRY(freeIndex);
                modePtr->devNum = devNum;
                modePtr->portNum = portNum;
                modePtr->unitsType = unitsType;
                break;
            default: return GT_BAD_PARAM;
        }
    }
    return GT_OK;
#endif /*PX_CODE*/
#if !defined(PX_CODE) && !defined(DXCH_CODE)
    devNum    = devNum     ;
    portNum   = portNum    ;
    mode      = mode       ;
    unitsType = unitsType  ;
    units     = units      ;

    return GT_NOT_IMPLEMENTED;
#endif /*DXCH_CODE*/
}

/**
* @internal tgfTrafficGeneratorWsModeOnPortSet function
* @endinternal
*
* @brief   set WS mode on port , by setting rxMirror and setting proper tx mode
*
* @param[in] devNum                   - device number
*                                      portNum - port number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
*/
GT_STATUS  tgfTrafficGeneratorWsModeOnPortSet(IN GT_U8    devNum, IN GT_U32  portNum)
{
    GT_U32                      mirrIndex;
    GT_STATUS rc,modeRc;
#if defined(PX_CODE)
    CPSS_INTERFACE_INFO_STC portInterface;
#endif

    /* configure WS mode */
    rc = prvTgfTxModeSetupEth(devNum, portNum,
                            PRV_TGF_TX_MODE_CONTINUOUS_E,
                            PRV_TGF_TX_CONT_MODE_UNIT_WS_E,
                            0);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get DB entry  */
    modeRc = prvTgfTxContModeEntryGet(devNum, portNum,NULL, &mirrIndex, NULL);
    if (modeRc == GT_OK)
    {
#ifdef DXCH_CODE
        if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PX_FAMILY_PIPE_E)
        {
            /* continuous mode is chosen */
            rc = cpssDxChMirrorRxPortSet(devNum, portNum,
                                GT_FALSE /* eport not physical port */ ,
                                GT_TRUE,/* enable */
                                mirrIndex);
            return rc;
        }
#endif
#ifdef PX_CODE
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PX_FAMILY_PIPE_E)
        {
            GT_BOOL   bypassLagDesignatedBitmap;
            rc = cpssPxIngressPortMapEntryGet(devNum,
                CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 16 * portNum,
                &prvTgfSrcTableEntryRestore, &bypassLagDesignatedBitmap);
            if(GT_OK != rc)
            {
                return rc;
            }

            rc = cpssPxIngressPortMapEntrySet(devNum,
                CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 16 * portNum, 0x0FFFF, GT_FALSE);
            if(GT_OK != rc)
            {
                return rc;
            }

            cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
            portInterface.type             = CPSS_INTERFACE_PORT_E;
            portInterface.devPort.hwDevNum = devNum;
            portInterface.devPort.portNum  = portNum;
            rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_TRUE);
            return GT_OK;
        }
#endif
        return GT_NOT_IMPLEMENTED;
    }

    return GT_OK;
}
/**
* @internal tgfTrafficGeneratorStopWsOnPort function
* @endinternal
*
* @brief   stop WS on port , by disable rxMirror and setting proper tx mode
*
* @param[in] devNum                   - device number
*                                      portNum - port number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
*/
GT_STATUS  tgfTrafficGeneratorStopWsOnPort(IN GT_U8    devNum, IN GT_U32  portNum)
{
    GT_STATUS                   modeRc;
    GT_STATUS                   rc = GT_OK;
    GT_U32 mirrIndex;
#if defined(DXCH_CODE) || defined(PX_CODE)
    CPSS_INTERFACE_INFO_STC portInterface;
#endif
    /* get DB entry  */
    modeRc = prvTgfTxContModeEntryGet(devNum, portNum,NULL, &mirrIndex, NULL);
    if (modeRc != GT_OK)
    {
        /* port in burst mode. */
        return GT_NOT_FOUND;
    }

    /* do not return on error from here , because need to restore the mode */

    /*****************************/
    /** disable the mac loopback */
    /*****************************/

#ifdef DXCH_CODE
    if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PX_FAMILY_PIPE_E)
    {
        cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
        /* setup receive outPortInterface for capturing */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = devNum;/*doing convert to PRV_CPSS_HW_DEV_NUM_MAC(devNum); inside tgfTrafficGeneratorPortLoopbackModeEnableSet */
        portInterface.devPort.portNum = portNum;

        /* the Port Loopback disable under traffic may result in problems on port.
        Disable port MAC before disable loopback to minimize problems */
        (void)cpssDxChPortEnableSet(devNum, portNum, GT_FALSE);

        /* disable loopback mode on port */
        (void) tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_FALSE);

        /* restore the port */
        (void)cpssDxChPortEnableSet(devNum, portNum, GT_TRUE);

        /*****************************/
        /** disable the rx mirroring */
        /*****************************/
        (void)cpssDxChMirrorRxPortSet(devNum, portNum,
                            GT_FALSE /* eport not physical port */ ,
                            GT_FALSE,/* disable not enable */
                                mirrIndex);

        /* configure default mode */
        rc = prvTgfTxModeSetupEth(devNum, portNum,
                                PRV_TGF_TX_MODE_SINGLE_BURST_E,
                                PRV_TGF_TX_CONT_MODE_UNIT_WS_E,
                            0);
    }
#endif /*DXCH_CODE*/
#ifdef PX_CODE
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PX_FAMILY_PIPE_E)
    {
        /***********************************/
        /* clear the forced configurations */
        /***********************************/

        /************************************/
        /* remove : link UP on ingress port */
        /************************************/
        rc = cpssPxPortForceLinkPassEnableSet(devNum,portNum,GT_FALSE);
        if(rc != GT_OK)
        {
            cpssOsPrintf("failed : cpssPxPortForceLinkPassEnableSet , on port[%d] \n" ,
                portNum);
            return rc;
        }

        rc = cpssPxIngressPortMapEntrySet(devNum,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, 16 * portNum,
            prvTgfSrcTableEntryRestore, GT_FALSE);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* restore traffic ingress from the CPU port */
        rc = cpssPxIngressPortMapEntrySet(devNum,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PRV_CPSS_PX_CPU_DMA_NUM_CNS,
            prvTgfRestoreCpuPortBmp[portNum], GT_FALSE);
        if(rc != GT_OK)
        {
            cpssOsPrintf("[TGF]: px_PCP_DST_PORT_MAP_TABLE_set FAILED, rc = [%d]", rc);
            return rc;
        }

        cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
        /* setup receive outPortInterface for capturing */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = devNum;/*doing convert to PRV_CPSS_HW_DEV_NUM_MAC(devNum); inside tgfTrafficGeneratorPortLoopbackModeEnableSet */
        portInterface.devPort.portNum = portNum;

        /* the Port Loopback disable under traffic may result in problems on port.
        Disable port MAC before disable loopback to minimize problems */
        (void)cpssPxPortEnableSet(devNum, portNum, GT_FALSE);

        /* disable loopback mode on port */
        (void) tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_FALSE);

        /* restore the port */
        (void)cpssPxPortEnableSet(devNum, portNum, GT_TRUE);

        /* configure default mode */
        rc = prvTgfTxModeSetupEth(devNum, portNum,
                                PRV_TGF_TX_MODE_SINGLE_BURST_E,
                                PRV_TGF_TX_CONT_MODE_UNIT_WS_E,
                                0);
    }
#endif
#if !defined(PX_CODE) && !defined(DXCH_CODE)
        rc = GT_NOT_IMPLEMENTED;
#endif
    return rc;
}


/**
* @internal prvTgfCommonDevicePortWsRateGet function
* @endinternal
*
* @brief   Get wire speed rate of port in packets per second
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] packetSize               - packet size in bytes including CRC
*                                       wire speed rate of port in packets per second.
*
* @retval 0xFFFFFFFF               - on error
*/
GT_U32 prvTgfCommonDevicePortWsRateGet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32 packetSize
)
{
    GT_STATUS   rc;
    CPSS_PORT_SPEED_ENT portSpeed = CPSS_PORT_SPEED_NA_E; /* port speed in enum values */
    GT_U32      speed;        /* speed in kilobits per second */
    double      packetRate;   /* packet rate in packets per second */
    GT_U32      l1PacketSize; /* packet size in media in bits */
    GT_U32      rateOut;      /* integer value of rate */

    /* get start value - reset counter */
#ifdef DXCH_CODE
    if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PX_FAMILY_PIPE_E)
    {
        rc =  cpssDxChPortSpeedGet(devNum, portNum,&portSpeed);
        if (rc != GT_OK)
        {
            PRV_TGF_LOG1_MAC("cpssDxChPortSpeedGet: %d", portNum);
            return 0xFFFFFFFF;
        }
    }
#endif
#ifdef PX_CODE
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PX_FAMILY_PIPE_E)
    {
        rc =  cpssPxPortSpeedGet(devNum, portNum, &portSpeed);
        if (rc != GT_OK)
        {
            PRV_TGF_LOG1_MAC("cpssPxPortSpeedGet: %d", portNum);
            return 0xFFFFFFFF;
        }
    }
#endif
#if !(defined DXCH_CODE) && !(defined PX_CODE)
    devNum = devNum;
    portNum = portNum;
    portSpeed = 0;
    rc =  GT_NOT_IMPLEMENTED;
#endif /*!defined(DXCH_CODE) && !defined(PX_CODE)*/

    switch(portSpeed)
    {
        case CPSS_PORT_SPEED_10_E:    speed =     10000; break;
        case CPSS_PORT_SPEED_100_E:   speed =    100000; break;
        case CPSS_PORT_SPEED_1000_E:  speed =   1000000; break;
        case CPSS_PORT_SPEED_10000_E: speed =  10000000; break;
        case CPSS_PORT_SPEED_12000_E: speed =  12000000; break;
        case CPSS_PORT_SPEED_2500_E:  speed =   2500000; break;
        case CPSS_PORT_SPEED_5000_E:  speed =   5000000; break;
        case CPSS_PORT_SPEED_13600_E: speed =  13600000; break;
        case CPSS_PORT_SPEED_20000_E: speed =  20000000; break;
        case CPSS_PORT_SPEED_40000_E: speed =  40000000; break;
        case CPSS_PORT_SPEED_16000_E: speed =  16000000; break;
        case CPSS_PORT_SPEED_15000_E: speed =  15000000; break;
        case CPSS_PORT_SPEED_100G_E:  speed = 100000000; break;
        case CPSS_PORT_SPEED_107G_E:  speed = 107000000; break;
        case CPSS_PORT_SPEED_102G_E:  speed = 102000000; break;
        case CPSS_PORT_SPEED_75000_E: speed =  75000000; break;
        case CPSS_PORT_SPEED_12500_E: speed =  12500000; break;
        case CPSS_PORT_SPEED_26700_E: speed =  26700000; break;
        default:
            PRV_TGF_LOG1_MAC("wrong speed: %d", portSpeed);return 0xFFFFFFFF;
    }

    /* add 20 bytes IPG and preamble to get L1 byte size */
    l1PacketSize = (packetSize + 20) * 8;

    packetRate = speed;
    packetRate = (packetRate * 1000) / l1PacketSize;

    /* convert floating point to integer */
    rateOut = (GT_U32) packetRate;

    return rateOut;
}

/**
* @internal prvTgfVntPortLastReadTimeStampGet function
* @endinternal
*
* @brief   Gets the last-read-time-stamp counter value that represent the exact
*         timestamp of the last read operation to the PP registers
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] timeStampValuePtr        - (pointer to) timestamp of the last read operation
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
*/
GT_STATUS prvTgfVntPortLastReadTimeStampGet
(
    IN  GT_U8                         devNum,
    IN  GT_PHYSICAL_PORT_NUM          portNum,
    OUT GT_U32                       *timeStampValuePtr
)
{
#ifdef DXCH_CODE
    if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PX_FAMILY_PIPE_E)
    {
        GT_STATUS rc;
        GT_PORT_GROUPS_BMP  portGroupBmp;

        /* port group are in bits 4, 5 of port number */
        portGroupBmp = 1 << (portNum >> 4);

        /* call device specific API */
        rc = cpssDxChVntPortGroupLastReadTimeStampGet(devNum, portGroupBmp, timeStampValuePtr);
        if(rc != GT_OK)
        {
            PRV_TGF_LOG1_MAC("[TGF]: cpssDxChVntPortGroupLastReadTimeStampGet FAILED, rc = [%d]", rc);
        }
        return rc;
    }
#endif /* CHX_CODE */

#ifdef PX_FAMILY
    if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PX_FAMILY_PIPE_E)
    {
        GT_STATUS rc;

        /* call device specific API */
        rc = cpssPxCfgLastRegAccessTimeStampGet(devNum, timeStampValuePtr);
        if(rc != GT_OK)
        {
            PRV_TGF_LOG1_MAC("[TGF]: cpssPxCfgLastRegAccessTimeStampGet FAILED, rc = [%d]", rc);
        }
        return rc;
    }
#endif /* PX_FAMILY */

#if !(defined DXCH_CODE) && !(defined PX_FAMILY)
    devNum = devNum;
    portNum = portNum;
    timeStampValuePtr = timeStampValuePtr;
    return GT_BAD_STATE;
#endif /* !(defined CHX_CODE) && !(defined EXMXPM_FAMILY) */
    return GT_OK;
}


/**
* @internal prvUtfDeviceCoreClockGet function
* @endinternal
*
* @brief   Get core clock in Herz of device
*
* @param[in] dev                      -   device id of ports
*
* @param[out] coreClockPtr             - (pointer to)core clock in Herz
*
* @retval GT_OK                    -  Get revision of device OK
* @retval GT_BAD_PARAM             -  Invalid device id
* @retval GT_BAD_PTR               -  Null pointer
*/
GT_STATUS prvUtfDeviceCoreClockGet
(
    IN GT_U8          dev,
    OUT GT_U32       *coreClockPtr
)
{
    /* device must be active */
    DEV_CHECK_MAC(dev);

    /* check for null pointer */
    CPSS_NULL_PTR_CHECK_MAC(coreClockPtr);

    switch (PRV_CPSS_PP_MAC(dev)->coreClock)
    {
        case 222: *coreClockPtr = 222222222; break;
        case 167: *coreClockPtr = 166666667; break;
        case 362: *coreClockPtr = 362500000; break;
        default: *coreClockPtr = PRV_CPSS_PP_MAC(dev)->coreClock * 1000000;
            break;
    }
    return GT_OK;
}

/**
* @internal prvTgfCommonVntTimeStampsRateGet function
* @endinternal
*
* @brief   Get rate of events by VNT Time Stamps.
*
* @param[in] dev                      - device number
* @param[in] startTimeStamp           - start counting time stamp
* @param[in] endTimeStamp             - end counting time stamp
* @param[in] eventCount               - events count between end and start time stamps
*                                       events rate in events per second
*/
GT_U32 prvTgfCommonVntTimeStampsRateGet
(
    IN  GT_U8          dev,
    IN  GT_U32         startTimeStamp,
    IN  GT_U32         endTimeStamp,
    IN  GT_U32         eventCount
)
{
    GT_STATUS rc;           /* return code*/
    GT_U32    coreClock;    /* device core clock in HZ */
    double    diff;         /* difference between end and start time stamps */
    double    rate;         /* rate of events */
    GT_U32    rateOut;      /* integer value of rate */

    /* get core closk in HZ */
    rc = prvUtfDeviceCoreClockGet(dev,&coreClock);
    if (rc != GT_OK)
    {
        PRV_TGF_LOG1_MAC("prvUtfDeviceCoreClockGet: %d", dev);
        return 0xFFFFFFFF;
    }

    /* calculate difference between end and start */
    if (startTimeStamp > endTimeStamp)
    {
       /* take care of wraparound of end time */
       diff = 0xFFFFFFFFU;
       diff = diff + 1 + endTimeStamp - startTimeStamp;
    }
    else
       diff = endTimeStamp - startTimeStamp;

   /* time stamp is in device core clocks.
     event rate is number of events divided by time between them.
     the time is core clock period multiplied on number of core clocks.
     rate = eventCount / time, where time = diff * (1/coreClock) */
   rate = eventCount;
   rate = (rate * coreClock) / diff;

   /* convert float point value to unsigned integer */
   rateOut = (GT_U32)rate;

   return rateOut;
}

/**
* @internal prvTgfGetRate function
* @endinternal
*
* @brief   Get rate in event per second.
*
* @param[in] countEnd                 - events count in the end of period
*                                      countStart - events count in the start of period
*                                      timeOutMilliSec - timeout in milli seconds
*                                       rate in events per seconds
*/
GT_U32  prvTgfGetRate(IN GT_U32 countEnd, IN GT_U32 countStart, IN GT_U32 timeOutMilliSec)
{
    GT_U32 delta; /* difference between end and start */
    GT_U32 rate;  /* result */

    delta = countEnd - countStart;
    if(delta < (0xFFFFFFFF / 1000))
    {
        rate = (1000 * delta) / timeOutMilliSec;
    }
    else if (delta < (0xFFFFFFFF / 100))
    {
        rate = ((100 * delta) / timeOutMilliSec) * 10;
    }
    else if (delta < (0xFFFFFFFF / 10))
    {
        rate = ((10 * delta) / timeOutMilliSec) * 100;
    }
    else
    {
        rate = 1000 * (delta / timeOutMilliSec);
    }
    return rate;
}

/**
* @internal prvTgfCommonPortsCntrRateGet_byDevPort function
* @endinternal
*
* @brief   Get rate of specific ports MAC MIB for up to 2 counters.
*
* @param[in] devNumArr[]              - array of devNum
* @param[in] portNumArr[]             - array of portNum
* @param[in] numOfPorts               - number of ports
* @param[in] cntrName1                - name of first counter
* @param[in] timeOut                  - timeout in milli seconds to calculate rate
* @param[in] cntrName2                - name of second counter
*
* @param[out] outRateArr1[]            - (pointer to) array of rate of ports for cntrName1.
* @param[out] outRateArr2[]            - (pointer to) array of rate of ports for cntrName2.
*                                      if NULL then this rate ignored.
*                                       none
*/
GT_STATUS prvTgfCommonPortsCntrRateGet_byDevPort
(
    IN  GT_U8  devNumArr[],
    IN  GT_U32 portNumArr[],
    IN  GT_U32 numOfPorts,
    IN  GT_U32 timeOut,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName1,
    OUT GT_U32 outRateArr1[],
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName2,
    OUT GT_U32 outRateArr2[]
)
{
    GT_U64      cntrValueArr1[GET_RATE_MAX_PORTS_CNS];  /* value of counters */
    GT_U64      cntrValueArr2[GET_RATE_MAX_PORTS_CNS];  /* value of counters */
    GT_U32      startSec, endSec;     /* start and end time seconds part */
    GT_U32      startNano, endNano;   /* start and end time nanoseconds part */
    GT_U32      startMilli, endMilli; /* start and end time in milliseconds */
    GT_U32      portNum;              /* port number */
    GT_U8       devNum;
    GT_STATUS   rc;                   /* return code */
    GT_U32      startTimeStampArr[GET_RATE_MAX_PORTS_CNS];       /* VNT time stamp start */
    GT_U32      endTimeStampArr[GET_RATE_MAX_PORTS_CNS];         /* VNT time stamp start */
    GT_U32      rate;                 /* VNT time stamp based rate */
    GT_U32      ii;                   /* iterator */
    CPSS_PORT_MAC_COUNTERS_ENT        cntrNameArr1[3]; /* names of counters */
    CPSS_PORT_MAC_COUNTERS_ENT        cntrNameArr2[3]; /* names of counters */
    GT_U32      numOfCntrs1,numOfCntrs2;           /* number of counters */
    GT_U32      cntrNameIdx;          /* iterator */
    GT_U64      tmp;                  /* temp variable */

    for (ii = 0; ii < numOfPorts; ii++)
    {
        outRateArr1[ii] = 0;
        if(outRateArr2)
        {
            outRateArr2[ii] = 0;
        }
    }

    /* check number of ports */
    if (numOfPorts > GET_RATE_MAX_PORTS_CNS || numOfPorts == 0)
    {
        /*error*/
        return GT_BAD_SIZE;
    }

    if(PRV_CPSS_PP_MAC(devNumArr[0])->devFamily != CPSS_PX_FAMILY_PIPE_E) /* Update to support heterogeneous devices*/
    {
        if (cntrName1 == CPSS_GOOD_PKTS_RCV_E)
        {
            cntrNameArr1[0] = CPSS_BRDC_PKTS_RCV_E;
            cntrNameArr1[1] = CPSS_MC_PKTS_RCV_E;
            cntrNameArr1[2] = CPSS_GOOD_UC_PKTS_RCV_E;
            numOfCntrs1     = 3;
        }
        else if (cntrName1 == CPSS_GOOD_PKTS_SENT_E)
        {
            cntrNameArr1[0] = CPSS_BRDC_PKTS_SENT_E;
            cntrNameArr1[1] = CPSS_MC_PKTS_SENT_E;
            cntrNameArr1[2] = CPSS_GOOD_UC_PKTS_SENT_E;
            numOfCntrs1     = 3;
        }
        else
        {
            cntrNameArr1[0] = cntrName1;
            numOfCntrs1     = 1;
        }

        if(outRateArr2 == NULL)
        {
            numOfCntrs2 = 0;
        }
        else
        if (cntrName2 == CPSS_GOOD_PKTS_RCV_E)
        {
            cntrNameArr2[0] = CPSS_BRDC_PKTS_RCV_E;
            cntrNameArr2[1] = CPSS_MC_PKTS_RCV_E;
            cntrNameArr2[2] = CPSS_GOOD_UC_PKTS_RCV_E;
            numOfCntrs2     = 3;
        }
        else if (cntrName2 == CPSS_GOOD_PKTS_SENT_E)
        {
            cntrNameArr2[0] = CPSS_BRDC_PKTS_SENT_E;
            cntrNameArr2[1] = CPSS_MC_PKTS_SENT_E;
            cntrNameArr2[2] = CPSS_GOOD_UC_PKTS_SENT_E;
            numOfCntrs2     = 3;
        }
        else
        {
            cntrNameArr2[0] = cntrName2;
            numOfCntrs2     = 1;
        }
    }
    else
    {
        cntrNameArr1[0] = cntrName1;
        numOfCntrs1     = 1;
        cntrNameArr2[0] = cntrName2;
        numOfCntrs2     = 1;
    }

    /* reset counters by read them */
    for (ii = 0; ii < numOfPorts; ii++)
    {
        devNum =  devNumArr[ii];
        portNum = portNumArr[ii];

        if(0 == DEV_SUPPORT_VNT_TIME_STAMPS_MAC(devNum))
        {
            prvTgfCommonVntTimeStampsInUse = 0;
        }

        for (cntrNameIdx = 0; cntrNameIdx < numOfCntrs1; cntrNameIdx++)
        {
            rc = prvTgfPortMacCounterGet(devNum, portNum,
                                         cntrNameArr1[cntrNameIdx],
                                         &tmp);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        for (cntrNameIdx = 0; cntrNameIdx < numOfCntrs2; cntrNameIdx++)
        {
            rc = prvTgfPortMacCounterGet(devNum, portNum,
                                         cntrNameArr2[cntrNameIdx],
                                         &tmp);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        rc = prvTgfVntPortLastReadTimeStampGet(devNum, portNum, &startTimeStampArr[ii]);
        if(rc != GT_OK && prvTgfCommonVntTimeStampsInUse)
        {
            return rc;
        }
    }

    /* store start time */
    cpssOsTimeRT(&startSec,&startNano);

    /* sleep */
    cpssOsTimerWkAfter(timeOut);

    /* get counter */
    for (ii = 0; ii < numOfPorts; ii++)
    {
        devNum =  devNumArr[ii];
        portNum = portNumArr[ii];

        cntrValueArr1[ii].l[0] = 0;
        for (cntrNameIdx = 0; cntrNameIdx < numOfCntrs1; cntrNameIdx++)
        {
            rc =  prvTgfPortMacCounterGet(devNum, portNum,
                                          cntrNameArr1[cntrNameIdx],
                                          &tmp);
            if(rc != GT_OK)
            {
                return rc;
            }
            cntrValueArr1[ii].l[0] += tmp.l[0];
        }

        cntrValueArr2[ii].l[0] = 0;
        for (cntrNameIdx = 0; cntrNameIdx < numOfCntrs2; cntrNameIdx++)
        {
            rc =  prvTgfPortMacCounterGet(devNum, portNum,
                                          cntrNameArr2[cntrNameIdx],
                                          &tmp);
            if(rc != GT_OK)
            {
                return rc;
            }
            cntrValueArr2[ii].l[0] += tmp.l[0];
        }

        rc = prvTgfVntPortLastReadTimeStampGet(devNum, portNum, &endTimeStampArr[ii]);
        if(rc != GT_OK && prvTgfCommonVntTimeStampsInUse)
        {
            return rc;
        }
    }

    /* get end time */
    cpssOsTimeRT(&endSec,&endNano);

    startMilli = startSec * 1000 + startNano / 1000000;
    endMilli = endSec * 1000 + endNano / 1000000;

    timeOut = (endMilli - startMilli);

    for (ii = 0; ii < numOfPorts; ii++)
    {
        portNum = portNumArr[ii];
        outRateArr1[ii] = prvTgfGetRate(cntrValueArr1[ii].l[0], 0, timeOut);
        if (prvTgfCommonVntTimeStampsDebug)
        {
            PRV_TGF_LOG2_MAC("---cpssOsTimeRT port %d rate %d\n", portNum, outRateArr1[ii]);
        }

        if(outRateArr2)
        {
            outRateArr2[ii] = prvTgfGetRate(cntrValueArr2[ii].l[0], 0, timeOut);
            if (prvTgfCommonVntTimeStampsDebug)
            {
                PRV_TGF_LOG2_MAC("---cpssOsTimeRT port %d rate %d\n", portNum, outRateArr2[ii]);
            }
        }
    }

    if (prvTgfCommonVntTimeStampsInUse)
    {
        for (ii = 0; ii < numOfPorts; ii++)
        {
            devNum =  devNumArr[ii];

            outRateArr1[ii] = prvTgfCommonVntTimeStampsRateGet(devNum, startTimeStampArr[ii], endTimeStampArr[ii], cntrValueArr1[ii].l[0]);

            if(outRateArr2)
            {
                outRateArr2[ii] = prvTgfCommonVntTimeStampsRateGet(devNum, startTimeStampArr[ii], endTimeStampArr[ii], cntrValueArr2[ii].l[0]);
            }
        }
    }

    if (prvTgfCommonVntTimeStampsDebug)
    {
        for (ii = 0; ii < numOfPorts; ii++)
        {
            devNum =  devNumArr[ii];
            portNum = portNumArr[ii];

            rate = prvTgfCommonVntTimeStampsRateGet(devNum, startTimeStampArr[ii], endTimeStampArr[ii], cntrValueArr1[ii].l[0]);
            PRV_TGF_LOG5_MAC("---port %d rate1 %d, start %d end %d count %d \n", portNum, rate, startTimeStampArr[ii], endTimeStampArr[ii], cntrValueArr1[ii].l[0]);

            if(outRateArr2)
            {
                rate = prvTgfCommonVntTimeStampsRateGet(devNum, startTimeStampArr[ii], endTimeStampArr[ii], cntrValueArr2[ii].l[0]);
                PRV_TGF_LOG5_MAC("---port %d rate2 %d, start %d end %d count %d \n", portNum, rate, startTimeStampArr[ii], endTimeStampArr[ii], cntrValueArr2[ii].l[0]);
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvTgfCommonDiffInPercentCalc function
* @endinternal
*
* @brief   Calculate difference between input values in percent
*
* @param[in] value1                   -  to compare
* @param[in] value2                   -  to compare
*                                       difference between input values in percent
*/
GT_U32 prvTgfCommonDiffInPercentCalc
(
    IN  GT_U32 value1,
    IN  GT_U32 value2
)
{
    GT_UL64      delta ;      /* difference between ports WS rate and input one*/
    GT_U32      percent;     /* difference between ports WS rate*/
    GT_UL64     value1_64 = (GT_UL64)value1;
    GT_UL64     value2_64 = (GT_UL64)value2;

    GT_UL64      deltamult200;

    if (value1 > value2)
    {
        delta = value1_64 - value2_64;
    }
    else
    {
        delta = value2_64 - value1_64;
    }

    /* calculate difference in percent of port's rate */
    /* By multiplying by two, adding one and dividing by two, you are
        effectively adding a half. This makes the integer division do a
        rounding instead of truncating.*/
    deltamult200 = delta * 200;

    percent = (GT_U32)((deltamult200 + 1) / (value1_64 * 2));

    return percent;
}


/**
* @internal prvTgfCaptureEnableUnmatchedTpidProfile function
* @endinternal
*
* @brief   Configure (or restore) specified TPID profile to not match with
*         any TPID in packets. This profile will be used by TGF to treat incoming
*         packets on captured port as untagged. This saves us from moving of
*         vlan tag infromation into TO_CPU DSA tag so a packet's original
*         etherType will not be lost.
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - configure TPID profile tpidIndex to not match
*                                      with any TPID.
*                                      GT_FALSE - restore state of TPID profile as it was before
*                                      configuring.
* @param[in] tpidIndex                - the TPID profile index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong profile
*/
GT_STATUS prvTgfCaptureEnableUnmatchedTpidProfile
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable,
    IN GT_U32   tpidIndex
)
{
#ifdef DXCH_CODE
    GT_STATUS rc;
    rc = prvTgfCaptureSet_ingressTagging_values(devNum, enable,
                                                CPSS_VLAN_ETHERTYPE0_E,
                                                tpidIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvTgfCaptureSet_ingressTagging_values(devNum, enable,
                                                CPSS_VLAN_ETHERTYPE1_E,
                                                tpidIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(enable == GT_TRUE)
    {
        prvTgfCaptureTpidWithZeroBmp = tpidIndex;
    }
    else
    {
        /* restore to defaults */
        prvTgfCaptureTpidWithZeroBmp = DEFAULT_TPID_WITH_ZERO_BMP_CNS;
    }

    return GT_OK;
#else
    devNum     = devNum    ;
    enable     = enable    ;
    tpidIndex  = tpidIndex ;


    return GT_NOT_IMPLEMENTED;
#endif
}
/**
* @internal tgfDefaultTxSdmaQueueSet function
* @endinternal
*
* @brief   Debug function to set the tx SDMA queue used in tgf when sending a packet
*
* @param[in] txQNum                   - the queue number (0-31)
*
* @retval GT_OK                    - Queue was set as requested
*/
GT_STATUS tgfDefaultTxSdmaQueueSet
(
    IN  GT_U8 txQNum
)
{

    txQueueNum = txQNum;

    return GT_OK;
}

#ifdef PX_FAMILY
/**
* @internal prvTgfPxTgfIngressCaptureEnable function
* @endinternal
*
* @brief   Enable/disable capture on destination port
*
* @param[in] devNum                   - the device number
* @param[in] enableCapture            - enable/disable capture
* @param[in] packetType               - packet type (restricted to the port number dedicated to capture)
* @param[in] tableType                - source/destination table type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
static GT_STATUS prvTgfPxTgfIngressCaptureEnable
(
    IN GT_SW_DEV_NUM     devNum,
    IN GT_BOOL enableCapture,
    IN CPSS_PX_PACKET_TYPE  packetType,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT  tableType
)
{
    GT_STATUS rc;
    GT_U32 tableIdx;

    /* Original data */
    static CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyDataOrig;
    static CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMaskOrig;
    static CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC  packetTypeFormatOrig[2];
    static CPSS_PX_PORTS_BMP                           portsBmpOrig[2];
    static CPSS_PX_PACKET_TYPE                         packetTypeOrig[2];
    static GT_BOOL                                     packetTypeKeyEntryEnableOrig = GT_FALSE;

    /* Data to be set to enable capture */
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC  packetTypeFormat;
    CPSS_PX_PORTS_BMP                           portsBmp;
    GT_BOOL                                     bypassLagDesignatedBitmap;

    tableIdx = (tableType == CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E) ? 0 : 1;
    if (enableCapture == GT_TRUE)
    {
        /* Reset data */
        cpssOsMemSet(&keyData, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
        cpssOsMemSet(&keyMask, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
        cpssOsMemSet(&packetTypeFormat, 0, sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));
        portsBmp = 0;

        packetTypeOrig[tableIdx] = packetType;

        /* Save original port map packet type format */
        rc = cpssPxIngressPortMapPacketTypeFormatEntryGet(devNum,
                                        tableType,
                                        packetTypeOrig[0],
                                        &packetTypeFormatOrig[tableIdx]);
        if(rc != GT_OK)
        {
            cpssOsPrintf("[TGF]: cpssPxIngressPortMapPacketTypeFormatEntryGet FAILED, rc = [%d]", rc);
            return rc;
        }

        if (tableType == CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E)
        {
            /* Save packet entry enable status */
            rc = cpssPxIngressPacketTypeKeyEntryEnableGet(devNum,
                                            packetType,
                                            &packetTypeKeyEntryEnableOrig);
            if (rc != GT_OK)
            {
                cpssOsPrintf("[TGF]: cpssPxIngressPacketTypeKeyEntryEnableGet FAILED, rc = [%d]", rc);
                return rc;
            }

            /* Enable packet entry */
            rc = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum,
                                            packetType,
                                            GT_TRUE);
            if (rc != GT_OK)
            {
                cpssOsPrintf("[TGF]: cpssPxIngressPacketTypeKeyEntryEnableSet FAILED, rc = [%d]", rc);
                return rc;
            }

            /* Save original packet type key */
            rc = cpssPxIngressPacketTypeKeyEntryGet(devNum,
                                            packetType,
                                            &keyDataOrig, &keyMaskOrig);
            if (rc != GT_OK)
            {
                cpssOsPrintf("[TGF]: cpssPxIngressPacketTypeKeyEntryGet FAILED, rc = [%d]", rc);
                return rc;
            }

            /* Override packet type relevant data by predefined values to enable capture */
            keyData.profileIndex = packetType;
            keyMask.profileIndex = 0x3f;


            rc = cpssPxIngressPacketTypeKeyEntrySet(devNum,
                                            packetType,
                                            &keyData, &keyMask);
            if (rc != GT_OK)
            {
                cpssOsPrintf("[TGF]: cpssPxIngressPacketTypeKeyEntrySet FAILED, rc = [%d]", rc);
                return rc;
            }

            packetTypeFormat.indexConst = packetType;
            packetTypeFormat.indexMax = 4095;

            rc = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            packetType,
                                            &packetTypeFormat);
            if (rc != GT_OK)
            {
                cpssOsPrintf("[TGF]: cpssPxIngressPortMapPacketTypeFormatEntrySet FAILED, rc = [%d]", rc);
                return rc;
            }

        }
        else
        {
            /* Keep values unchanged */
            packetTypeFormat.indexConst = packetTypeFormatOrig[tableIdx].indexConst;
            packetTypeFormat.indexMax = packetTypeFormatOrig[tableIdx].indexMax;
        }

        /* Save original port map map entry */
        rc = cpssPxIngressPortMapEntryGet(devNum,
                                            tableType,
                                            packetTypeFormat.indexConst,
                                            &portsBmpOrig[tableIdx],
                                            &bypassLagDesignatedBitmap);
        if (rc != GT_OK)
        {
            cpssOsPrintf("[TGF]: cpssPxIngressPortMapEntryGet FAILED, rc = [%d]", rc);
            return rc;
        }

        rc = cpssPxIngressPortMapEntrySet(devNum,
                                            tableType,
                                            packetTypeFormat.indexConst,
                                            portsBmp, GT_FALSE);
        if (rc != GT_OK)
        {
            cpssOsPrintf("[TGF]: cpssPxIngressPortMapEntrySet FAILED, rc = [%d]", rc);
            return rc;
        }
    }
    else
    {

        /* Restore original data */
        rc = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum,
                                            packetTypeOrig[0],
                                            packetTypeKeyEntryEnableOrig);
        if (rc != GT_OK)
        {
            cpssOsPrintf("[TGF]: cpssPxIngressPacketTypeKeyEntryEnableSet FAILED, rc = [%d]", rc);
            return rc;
        }

        rc = cpssPxIngressPacketTypeKeyEntrySet(devNum,
                                            packetTypeOrig[0],
                                            &keyDataOrig, &keyMaskOrig);
        if (rc != GT_OK)
        {
            cpssOsPrintf("[TGF]: cpssPxIngressPacketTypeKeyEntrySet FAILED, rc = [%d]", rc);
            return rc;
        }


        for (tableIdx = 0; tableIdx < 2; tableIdx++)
        {
            rc = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
                                               (tableIdx == 0) ? CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E :
                                                                 CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
                                                packetTypeOrig[0],
                                                &packetTypeFormatOrig[tableIdx]);
            if (rc != GT_OK)
            {
                cpssOsPrintf("[TGF]: cpssPxIngressPortMapPacketTypeFormatEntrySet FAILED, rc = [%d]", rc);
                return rc;
            }

            rc = cpssPxIngressPortMapEntrySet(devNum,
                                            (tableIdx == 0) ? CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E :
                                                              CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
                                            packetTypeFormatOrig[tableIdx].indexConst,
                                            portsBmpOrig[tableIdx], GT_FALSE);
            if (rc != GT_OK)
            {
                cpssOsPrintf("[TGF]: cpssPxIngressPortMapEntrySet FAILED, rc = [%d]", rc);
                return rc;
            }
        }

    }

    return GT_OK;
}

/**
* @internal prvTgfPxSkipPortWithNoMac function
* @endinternal
*
* @brief   check if port hold MAC -- if not --> need to be skipped.
*
* @param[in] devNum                   - the device number
*                                      portNum       - the ingress port num
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
*/
GT_STATUS prvTgfPxSkipPortWithNoMac(IN GT_SW_DEV_NUM devNum , IN GT_PHYSICAL_PORT_NUM portNum)
{
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

#if 0 /* the MACRO that cant use because generate ERROR LOG messages */
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
#endif
    if(portNum >= PRV_CPSS_PX_PORTS_NUM_CNS)
    {
        return GT_BAD_PARAM;
    }

    if(PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].valid != GT_TRUE)
    {
        return GT_BAD_PARAM;
    }

    /* check CPSS DB to avoid ERROR LOG messages on not valid ports */
    if(PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].portMap.mappingType !=
       CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
        return GT_BAD_PARAM;
    }


    return GT_OK;
}

static GT_BOOL ignoreCounterCheck = GT_FALSE;

/**
* @internal tgfTrafficGeneratorPxIgnoreCounterCheckSet function
* @endinternal
*
* @brief   Ignore counter check in tgfTrafficGeneratorPxTransmitPacketAndCheckResult
*
* @param[in] ignore                   - GT_TRUE - ignore
*                                      GT_FALSE - not ignore
*                                       None.
*/
GT_VOID tgfTrafficGeneratorPxIgnoreCounterCheckSet
(
    GT_BOOL ignore
)
{
    ignoreCounterCheck = ignore;
}

/**
* @internal clearAllMacCounters function
* @endinternal
*
* @brief   clear mac counters in the device.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - ingress port number
* @param[in] numOfEgressPortsInfo     - number of egress ports
*                                       none.
*/
static void clearAllMacCounters(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               numOfEgressPortsInfo
)
{
    GT_U32  ii;
    CPSS_PX_PORT_MAC_COUNTERS_STC counters;
    for(ii = 0 ; ii < PRV_CPSS_PX_PORTS_NUM_CNS; ii++)
    {
        if(GT_OK != prvTgfPxSkipPortWithNoMac(devNum,ii))
        {
            continue;
        }

        /* reset only portNum if no egress ports defined */
        if ((numOfEgressPortsInfo == 0) && (portNum != ii))
        {
            continue;
        }

        (void)cpssPxPortMacCountersOnPortGet(devNum,ii,&counters);
    }
}

/*======================================================================================*/
/* issue of MAC MIB counters was found on EMULATOR of PIPE                              */
/* even though port was 'force link up' the port did not count traffic in the MIB       */
/* (port link status showed 'up' and also got 'link status changed' and status 'up')    */
/* SO the WA is to set port in mac loopback (+PCS) that do MIB modifications OK         */
/* and to set MRU =0 on such port ... so unwanted packet will not 'bridge' the device   */
/*======================================================================================*/
/* WA for ports that force link up but not count on egress                              */
static GT_U32   egress_port_MIB_WA = 1;
/* function to allow to check if issue solved on EMULATOR !!! */
/* NOTE: on simulation it works OK                            */
GT_STATUS pipe_egress_port_MIB_WA_set(GT_U32    doWA)
{
    egress_port_MIB_WA = doWA;
    return GT_OK;
}


/**
* @internal pxInjectToIngressPortAndCheckEgress function
* @endinternal
*
* @brief   send packet to ingress a port and check expected egress ports.
*         function will set :
*         1. link UP
*         2. MAC LOOPBACK
*         3. set 'forwarding' tables to direct the packet from CPU to the port.
*         3. send from cpu packet to egress the port and LOOPBACK will make it
*         ingress the port.
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the ingress port num
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               - number of frames
* @param[in] numOfEgressPortsInfo     - number of egress ports info in array of egressPortsArr[]
*                                      if 0 --> not checking egress ports
* @param[in] egressPortsArr[]         - array of egress ports to check for counters and packets
* @param[in] isOrigSender             - indication that caller is 'original' or 'recursive'
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_GET_ERROR             - on general fail (not expected egress info )
*/
static GT_STATUS pxInjectToIngressPortAndCheckEgress
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN TGF_PACKET_STC      *packetInfoPtr,
    IN GT_U32               burstCount,
    IN GT_U32               numOfEgressPortsInfo,
    IN TGF_EXPECTED_EGRESS_INFO_STC     egressPortsArr[],
    IN GT_BOOL              isOrigSender
)
{
    GT_STATUS rc;
    TGF_EXPECTED_EGRESS_INFO_STC    *currEgressPtr;
    GT_PHYSICAL_PORT_NUM    egress_portNum;/* the egress portNum */
    CPSS_INTERFACE_INFO_STC     ingressPortInterface;
    GT_U32  ii;
    GT_U32  firstPortToCapture = 0xffffffff;
    GT_U32  firstIndexToCapture = 0xffffffff;
    GT_U32  actualCapturedNumOfPackets;
    CPSS_PX_PORT_MAC_COUNTER_ENT cntrName;
    CPSS_PX_PORT_MAC_COUNTERS_STC   portCounters;
    GT_U32                      cntr32Value;
    GT_BOOL                     didError = GT_FALSE;
    GT_U32                      restoreCapturedPortBmp = 0;
    GT_U32                      restoreCpuPortBmp = 0;
    GT_U32                      PTS_Src_Idx_Constant = 0;
    GT_U32                      restoreCapturedSrcPortsBmp = 0;
    GT_BOOL                     bypassLagDesignatedBitmap;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     packetTypeFormat;
    GT_U32  do_egress_port_MIB_WA   = cpssDeviceRunCheck_onEmulator() &&
                                      egress_port_MIB_WA
#ifdef ASIC_SIMULATION
                                        &&
                                      (!ignoreCounterCheck)
#endif
                                      ;/* do problems on WM in : do shell-execute  utfTestsTypeRun 1,0,9,0 */
    if(numOfEgressPortsInfo && (burstCount != 1))
    {
        cpssOsPrintf("pxInjectToIngressPortAndCheckEgress : not supports 'send' of more than single packet \n");
        return GT_BAD_PARAM;
    }

    if(isOrigSender == GT_TRUE)
    {
        /************************************/
        /* clear mac counters in the device */
        /************************************/
        clearAllMacCounters(devNum, portNum, numOfEgressPortsInfo);
        /***********************************************************/
        /* clear the 'captured' table before we start capturing    */
        /* so the captured port will not see previous packets      */
        /***********************************************************/
        (void)tgfTrafficTableRxPcktTblClear();

        /* state to engine that we start capture */
        rc = prvTgfTrafficTableRxStartCapture(GT_TRUE);
        if (GT_OK != rc)
        {
            cpssOsPrintf("failed : prvTgfTrafficTableRxStartCapture start \n");
            return rc;
        }

        /*********************************/
        /* force link UP on ingress port */
        /*********************************/
        rc = cpssPxPortForceLinkPassEnableSet(devNum,portNum,GT_TRUE);
        if(rc != GT_OK)
        {
            cpssOsPrintf("failed : cpssPxPortForceLinkPassEnableSet , on port[%d] \n" ,
                portNum);
            return rc;
        }


        /************************************************/
        /* add the port to enabled state                */
        /* no need to restore (done by link down event) */
        /************************************************/
        rc = cpssPxIngressPortTargetEnableSet(devNum,portNum,GT_TRUE);
        if(rc != GT_OK)
        {
            cpssOsPrintf("failed : cpssPxIngressPortTargetEnableSet , on port[%d] \n" ,
                portNum);
            return rc;
        }

        /************************************/
        /* set mac loopback on ingress port */
        /************************************/
        rc = cpssPxPortInternalLoopbackEnableSet(devNum,portNum,GT_TRUE);
        if(rc != GT_OK)
        {
            cpssOsPrintf("failed : cpssPxPortInternalLoopbackEnableSet , on port[%d] \n" ,
                portNum);
            return rc;
        }

        /* set that traffic from the CPU is sent to the specific port */
        {

            /* appDemo set profile per src port to direct traffic to needed port */
            rc = cpssPxIngressPortMapEntryGet(devNum,
                                              CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                              PRV_CPSS_PX_CPU_DMA_NUM_CNS,
                                              &restoreCpuPortBmp,
                                              &bypassLagDesignatedBitmap);
            if(rc != GT_OK)
            {
                cpssOsPrintf("[TGF]: cpssPxIngressPortMapEntryGet FAILED, rc = [%d]", rc);
                return rc;
            }


            /* appDemo set profile per src port to direct traffic to needed port */
            rc = cpssPxIngressPortMapEntrySet(devNum,
                                              CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                              PRV_CPSS_PX_CPU_DMA_NUM_CNS,
                                              1 << portNum, GT_FALSE);
            if(rc != GT_OK)
            {
                cpssOsPrintf("[TGF]: cpssPxIngressPortMapEntrySet FAILED, rc = [%d]", rc);
                return rc;
            }
        }
    }

    if(numOfEgressPortsInfo && egressPortsArr)
    {
        for(ii = 0 ; ii < numOfEgressPortsInfo ; ii++)
        {
            currEgressPtr = &egressPortsArr[ii];

            egress_portNum = currEgressPtr->portNum;

            if(isOrigSender == GT_TRUE)
            {
                /********************************/
                /* force link UP on egress port */
                /********************************/
                rc = cpssPxPortForceLinkPassEnableSet(devNum,egress_portNum,GT_TRUE);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("failed : cpssPxPortForceLinkPassEnableSet , on port[%d] \n" ,
                        egress_portNum);
                    return rc;
                }

                /************************************************/
                /* add the port to enabled state                */
                /* no need to restore (done by link down event) */
                /************************************************/
                rc = cpssPxIngressPortTargetEnableSet(devNum,egress_portNum,GT_TRUE);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("failed : cpssPxIngressPortTargetEnableSet , on port[%d] \n" ,
                        portNum);
                    return rc;
                }
            }

            if(currEgressPtr->pktInfo)
            {
                if(firstIndexToCapture == 0xffffffff)
                {
                    firstIndexToCapture = ii;
                    firstPortToCapture = egress_portNum;
                }/*firstIndexToCapture == 0xffffffff*/
            }/*currEgressPtr->pktInfo*/
        }
    }

    if(firstPortToCapture != 0xffffffff)
    {
        /************************************/
        /* set mac loopback on egress port */
        /************************************/
        rc = cpssPxPortInternalLoopbackEnableSet(devNum,firstPortToCapture,GT_TRUE);
        if(rc != GT_OK)
        {
            cpssOsPrintf("failed : cpssPxPortInternalLoopbackEnableSet , on port[%d] \n" ,
                firstPortToCapture);
            return rc;
        }

        if(do_egress_port_MIB_WA)
        {
            /* restore MRU on the port */
            rc = cpssPxPortMruSet(devNum, firstPortToCapture,
                                              1522);
            if(rc != GT_OK)
            {
                cpssOsPrintf("[TGF]: cpssPxPortMruSet FAILED, rc = [%d]", rc);
                return rc;
            }
        }

        /* set that traffic ingress from the egress port is sent to the CPU */
        {
            rc = prvTgfPxTgfIngressCaptureEnable(devNum, GT_TRUE, firstPortToCapture, CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E);
            if(rc != GT_OK)
            {
                cpssOsPrintf("[TGF]: prvTgfPxTgfIngressCaptureEnable FAILED, rc = [%d]", rc);
                return rc;
            }

            /* appDemo set profile per src port to direct traffic to needed port */
            rc = cpssPxIngressPortMapEntryGet(devNum,
                                              CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                              firstPortToCapture,&restoreCapturedPortBmp,
                                              &bypassLagDesignatedBitmap);
            if(rc != GT_OK)
            {
                cpssOsPrintf("[TGF]: cpssPxIngressPortMapEntryGet FAILED, rc = [%d]", rc);
                return rc;
            }

            /* appDemo set profile per src port to direct traffic to needed port */
            rc = cpssPxIngressPortMapEntrySet(devNum,
                                              CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                              firstPortToCapture,
                                              1 << PRV_CPSS_PX_CPU_DMA_NUM_CNS, GT_FALSE);
            if(rc != GT_OK)
            {
                cpssOsPrintf("[TGF]: cpssPxIngressPortMapEntrySet FAILED, rc = [%d]", rc);
                return rc;
            }

            rc = cpssPxIngressPortMapPacketTypeFormatEntryGet(devNum, CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
                                                              firstPortToCapture, &packetTypeFormat);
            if(rc != GT_OK)
            {
                cpssOsPrintf("[TGF]: cpssPxIngressPortMapPacketTypeFormatEntryGet FAILED, rc = [%d]", rc);
                return rc;
            }

            PTS_Src_Idx_Constant = packetTypeFormat.indexConst;

            rc = prvTgfPxTgfIngressCaptureEnable(devNum, GT_TRUE, PTS_Src_Idx_Constant, CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E);
            if(rc != GT_OK)
            {
                cpssOsPrintf("[TGF]: prvTgfPxTgfIngressCaptureEnable FAILED, rc = [%d]", rc);
                return rc;
            }

            rc = cpssPxIngressPortMapEntryGet(devNum,
                                              CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
                                              PTS_Src_Idx_Constant,
                                              &restoreCapturedSrcPortsBmp,
                                              &bypassLagDesignatedBitmap);
            if(rc != GT_OK)
            {
                cpssOsPrintf("[TGF]: cpssPxIngressPortMapEntryGet FAILED, rc = [%d]", rc);
                return rc;
            }

            /* add the CPU to allow trap to CPU */
            rc = cpssPxIngressPortMapEntrySet(devNum,
                                              CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
                                              PTS_Src_Idx_Constant,
                                              restoreCapturedSrcPortsBmp | 1 << PRV_CPSS_PX_CPU_DMA_NUM_CNS, GT_FALSE);
            if(rc != GT_OK)
            {
                cpssOsPrintf("[TGF]: cpssPxIngressPortMapEntrySet FAILED, rc = [%d]", rc);
                return rc;
            }
        }

    }

    if(isOrigSender == GT_TRUE && do_egress_port_MIB_WA)
    {
        /* WA for ports that force link up but not count on egress */
        for(ii = 0 ; ii < numOfEgressPortsInfo ; ii++)
        {
            if(egressPortsArr[ii].portNum == firstPortToCapture)
            {
                /* do not kill MRU on 'real' captured port */
                continue;
            }

            if(egressPortsArr[ii].portNum == portNum)
            {
                /* do not kill MRU on 'real' ingress port */
                continue;
            }

            rc = cpssPxPortInternalLoopbackEnableSet(devNum,egressPortsArr[ii].portNum,GT_TRUE);
            if(rc != GT_OK)
            {
                cpssOsPrintf("failed : cpssPxPortInternalLoopbackEnableSet , on port[%d] \n" ,
                    egressPortsArr[ii].portNum);
                return rc;
            }

            /* set MRU to kill traffic from the port */
            rc = cpssPxPortMruSet(devNum, egressPortsArr[ii].portNum,
                                              0);/*DISCARD the traffic*/
            if(rc != GT_OK)
            {
                cpssOsPrintf("[TGF]: cpssPxPortMruSet FAILED, rc = [%d]", rc);
                return rc;
            }
        }
    }



    ingressPortInterface.type = CPSS_INTERFACE_PORT_E;
    ingressPortInterface.devPort.portNum = portNum;
    ingressPortInterface.devPort.hwDevNum = devNum;


    tgfTrafficTracePacketByteSet(GT_TRUE);

    rc = tgfTrafficGeneratorPortTxEthTransmit(&ingressPortInterface,packetInfoPtr,
        burstCount,
        0,/*numVfd*/
        NULL,/*vfdArray[]*/
        0,/*sleepAfterXCount*/
        0,/*sleepTime*/
        1,/*traceBurstCount -- allow the first packet to be traced */
        GT_TRUE /*loopbackEnabled*/);


    if(rc != GT_OK)
    {
        cpssOsPrintf("failed : tgfTrafficGeneratorPortTxEthTransmit , on port[%d] \n" ,
            portNum);
        return rc;
    }

    if(firstPortToCapture != 0xffffffff)
    {
        /**************************************/
        /* remove mac loopback on egress port */
        /**************************************/
        rc = cpssPxPortInternalLoopbackEnableSet(devNum,firstPortToCapture,GT_FALSE);
        if(rc != GT_OK)
        {
            cpssOsPrintf("failed : cpssPxPortInternalLoopbackEnableSet , on port[%d] \n" ,
                firstPortToCapture);
            return rc;
        }
        /* restore traffic ingress from the egress port */
        prvTgfPxTgfIngressCaptureEnable(devNum, GT_FALSE, 0, CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_LAST);
    }

    /*************************/
    /* check egress counters */
    /*************************/
    if(isOrigSender == GT_TRUE && numOfEgressPortsInfo && egressPortsArr && !ignoreCounterCheck)
    {
        for(ii = 0 ; ii < numOfEgressPortsInfo ; ii++)
        {
            currEgressPtr = &egressPortsArr[ii];

            egress_portNum = currEgressPtr->portNum;
            if(currEgressPtr->pktInfo)
            {
                /* the counters of captured port are checked by expected packet length */
                continue;
            }

            rc = cpssPxPortMacCountersOnPortGet(devNum,egress_portNum,&portCounters);
            if(rc != GT_OK)
            {
                cpssOsPrintf("failed : cpssPxPortMacCountersOnPortGet , on port[%d] \n" ,
                    egress_portNum);
                return rc;
            }

            cntrName = CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E;
            cntr32Value = portCounters.mibCounter[cntrName].l[0];
            if(cntr32Value != currEgressPtr->numBytes)
            {
                PRV_TGF_LOG3_MAC("ERROR : egress port[%d] : sent octets counter : got[%d] expected[%d] \n",
                    egress_portNum,
                    cntr32Value,
                    currEgressPtr->numBytes);
                didError = GT_TRUE;
            }

            cntr32Value = 0;
            cntrName = CPSS_PX_PORT_MAC_COUNTER_UNICAST_FRAME_SENT_E;
            cntr32Value += portCounters.mibCounter[cntrName].l[0];
            cntrName = CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_SENT_E;
            cntr32Value += portCounters.mibCounter[cntrName].l[0];
            cntrName = CPSS_PX_PORT_MAC_COUNTER_BROADCAST_FRAMES_SENT_E;
            cntr32Value += portCounters.mibCounter[cntrName].l[0];

            if(cntr32Value != currEgressPtr->burstCount)
            {
                PRV_TGF_LOG3_MAC("ERROR : egress port[%d] : sent frames counter : got[%d] expected[%d] \n",
                    egress_portNum,
                    cntr32Value,
                    currEgressPtr->burstCount);
                didError = GT_TRUE;
            }
        }
    }

    if(firstPortToCapture != 0xffffffff)
    {
        currEgressPtr = &egressPortsArr[firstIndexToCapture];

        if(currEgressPtr->pktInfo && currEgressPtr->pktInfo->numOfParts)
        {
            PRV_TGF_LOG1_MAC("UTF: check what captured on egress port[%d] \n",firstPortToCapture);
            if (cpssDeviceRunCheck_onEmulator())
            {
                cpssOsTimerWkAfter(500);
            }
            rc = tgfTrafficGeneratorPortTxEthCaptureCompare(NULL,/*portInterfacePtr -- we cant specify the egress port */
                currEgressPtr->pktInfo,
                1,/*numOfPackets*/
                0,/*numVfd*/
                NULL,/*vfdArray[]*/
                NULL,/*byteNumMaskList[]*/
                0,/*byteNumMaskListLen*/
                &actualCapturedNumOfPackets,
                NULL/*onFirstPacketNumTriggersBmpPtr*/
                );

            /***********************************************************/
            /* clear the 'captured' table after we got packets from it */
            /* so the next captured port will not see current packet   */
            /***********************************************************/
            (void)tgfTrafficTableRxPcktTblClear();

            if(rc != GT_OK)
            {
                cpssOsPrintf("failed : tgfTrafficGeneratorPortTxEthCaptureCompare , on port[%d] \n" ,
                    firstPortToCapture);

                didError = GT_TRUE;
                /* continue the processing , because we need to restore configurations.
                   and we need to get information about errors in other ports (if any) */
            }
        }
    }

    /*********************************************************/
    /* treat other ports that need to be captured to the CPU */
    /*********************************************************/
    if(isOrigSender == GT_TRUE && numOfEgressPortsInfo && egressPortsArr)
    {
        for(ii = 0 ; ii < numOfEgressPortsInfo ; ii++)
        {
            currEgressPtr = &egressPortsArr[ii];

            egress_portNum = currEgressPtr->portNum;

            if(firstIndexToCapture == ii)
            {
                /* we already tested this port */
            }
            else
            if(currEgressPtr->pktInfo && currEgressPtr->pktInfo->numOfParts)
            {
                /* ask to send again to check only 'egress port' */
                rc = pxInjectToIngressPortAndCheckEgress(devNum,portNum,
                        packetInfoPtr,burstCount,
                        1,currEgressPtr,
                        GT_FALSE/*isOrigSender*/);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("failed : pxInjectToIngressPortAndCheckEgress , for EGRESS port[%d] \n" ,
                        egress_portNum);

                    didError = GT_TRUE;
                    /* continue the processing , because we need to restore configurations.
                       and we need to get information about errors in other ports (if any) */
                }
            }
        }
    }

    /***********************************/
    /* clear the forced configurations */
    /***********************************/
    if(isOrigSender == GT_TRUE)
    {
        /************************************/
        /* remove : link UP on ingress port */
        /************************************/
        rc = cpssPxPortForceLinkPassEnableSet(devNum,portNum,GT_FALSE);
        if(rc != GT_OK)
        {
            cpssOsPrintf("failed : cpssPxPortForceLinkPassEnableSet , on port[%d] \n" ,
                portNum);
            return rc;
        }

        /*****************************************/
        /* remove : mac loopback on ingress port */
        /*****************************************/
        rc = cpssPxPortInternalLoopbackEnableSet(devNum,portNum,GT_FALSE);
        if(rc != GT_OK)
        {
            cpssOsPrintf("failed : cpssPxPortInternalLoopbackEnableSet , on port[%d] \n" ,
                portNum);
            return rc;
        }

        if (egressPortsArr)
        {
            for(ii = 0 ; ii < numOfEgressPortsInfo ; ii++)
            {
                currEgressPtr = &egressPortsArr[ii];

                egress_portNum = currEgressPtr->portNum;
                /*****************************************/
                /* remove : force link UP on egress port */
                /*****************************************/
                rc = cpssPxPortForceLinkPassEnableSet(devNum,egress_portNum,GT_FALSE);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("failed : cpssPxPortForceLinkPassEnableSet , on port[%d] \n" ,
                        egress_portNum);
                    return rc;
                }
            }
        }

        /* restore traffic ingress from the CPU port */
        rc = cpssPxIngressPortMapEntrySet(devNum,
                                          CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                          PRV_CPSS_PX_CPU_DMA_NUM_CNS,
                                          restoreCpuPortBmp, GT_FALSE);
        if(rc != GT_OK)
        {
            cpssOsPrintf("[TGF]: px_PCP_DST_PORT_MAP_TABLE_set FAILED, rc = [%d]", rc);
            return rc;
        }

        /* state to engine that we stop capture */
        rc = prvTgfTrafficTableRxStartCapture(GT_TRUE);
        if (GT_OK != rc)
        {
            cpssOsPrintf("failed : prvTgfTrafficTableRxStartCapture stop \n");
            return rc;
        }

        /* let the appdemo time to process link down events , to the next
           force link up + cpssPxIngressPortTargetEnableSet(dev,port,'UP')*/
        cpssOsTimerWkAfter(50);
    }

    if(isOrigSender == GT_TRUE && do_egress_port_MIB_WA)
    {
        /* WA for ports that force link up but not count on egress */
        for(ii = 0 ; ii < numOfEgressPortsInfo ; ii++)
        {
            if(egressPortsArr[ii].portNum == firstPortToCapture)
            {
                /* already */
                continue;
            }

            /* remove the loopback */
            rc = cpssPxPortInternalLoopbackEnableSet(devNum,egressPortsArr[ii].portNum,GT_FALSE);
            if(rc != GT_OK)
            {
                cpssOsPrintf("failed : cpssPxPortInternalLoopbackEnableSet , on port[%d] \n" ,
                    egressPortsArr[ii].portNum);
                return rc;
            }

            /* restore profile per src port */
            /* restore MRU on the port */
            rc = cpssPxPortMruSet(devNum, egressPortsArr[ii].portNum,
                                              1522);
            if(rc != GT_OK)
            {
                cpssOsPrintf("[TGF]: cpssPxPortMruSet FAILED, rc = [%d]", rc);
                return rc;
            }
        }
    }

    return didError ? GT_GET_ERROR : GT_OK;
}

/**
* @internal pxInjectToIngressPort function
* @endinternal
*
* @brief   send packet to ingress port
*         function will set :
*         1. link UP
*         2. MAC LOOPBACK
*         3. set 'forwarding' tables to direct the packet from CPU to the port.
*         3. send from cpu packet to egress the port and LOOPBACK will make it
*         ingress the port.
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the ingress port num
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               - number of frames
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_GET_ERROR             - on general fail (not expected egress info )
*/
static GT_STATUS pxInjectToIngressPort
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN TGF_PACKET_STC      *packetInfoPtr,
    IN GT_U32               burstCount
)
{
    GT_STATUS                   rc;
    CPSS_INTERFACE_INFO_STC     ingressPortInterface;
    GT_BOOL                     bypassLagDesignatedBitmap;

    /*********************************/
    /* force link UP on ingress port */
    /*********************************/
    rc = cpssPxPortForceLinkPassEnableSet(devNum,portNum,GT_TRUE);
    if(rc != GT_OK)
    {
        cpssOsPrintf("failed : cpssPxPortForceLinkPassEnableSet , on port[%d] \n" ,
            portNum);
        return rc;
    }

    /************************************************/
    /* add the port to enabled state                */
    /* no need to restore (done by link down event) */
    /************************************************/
    rc = cpssPxIngressPortTargetEnableSet(devNum,portNum,GT_TRUE);
    if(rc != GT_OK)
    {
        cpssOsPrintf("failed : cpssPxIngressPortTargetEnableSet , on port[%d] \n" ,
            portNum);
        return rc;
    }

    /************************************/
    /* set mac loopback on ingress port */
    /************************************/
    rc = cpssPxPortInternalLoopbackEnableSet(devNum,portNum,GT_TRUE);
    if(rc != GT_OK)
    {
        cpssOsPrintf("failed : cpssPxPortInternalLoopbackEnableSet , on port[%d] \n" ,
            portNum);
        return rc;
    }

    /* appDemo set profile per src port to direct traffic to needed port */
    rc = cpssPxIngressPortMapEntryGet(devNum,
                                        CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                        PRV_CPSS_PX_CPU_DMA_NUM_CNS,
                                        &prvTgfRestoreCpuPortBmp[portNum],
                                        &bypassLagDesignatedBitmap);
    if(rc != GT_OK)
    {
        cpssOsPrintf("[TGF]: cpssPxIngressPortMapEntryGet FAILED, rc = [%d]", rc);
        return rc;
    }


    /* appDemo set profile per src port to direct traffic to needed port */
    rc = cpssPxIngressPortMapEntrySet(devNum,
                                        CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                        PRV_CPSS_PX_CPU_DMA_NUM_CNS,
                                        1 << portNum, GT_FALSE);
    if(rc != GT_OK)
    {
        cpssOsPrintf("[TGF]: cpssPxIngressPortMapEntrySet FAILED, rc = [%d]", rc);
        return rc;
    }

    ingressPortInterface.type = CPSS_INTERFACE_PORT_E;
    ingressPortInterface.devPort.portNum = portNum;
    ingressPortInterface.devPort.hwDevNum = devNum;


    tgfTrafficTracePacketByteSet(GT_TRUE);

    rc = tgfTrafficGeneratorPortTxEthTransmit(&ingressPortInterface,packetInfoPtr,
        burstCount,
        0,/*numVfd*/
        NULL,/*vfdArray[]*/
        0,/*sleepAfterXCount*/
        0,/*sleepTime*/
        1,/*traceBurstCount -- allow the first packet to be traced */
        GT_TRUE /*loopbackEnabled*/);

    if(rc != GT_OK)
    {
        cpssOsPrintf("failed : tgfTrafficGeneratorPortTxEthTransmit , on port[%d] \n" ,
            portNum);
        return rc;
    }

    /* Deconfigure later on demand*/
    if(prvTgfWsBurst == 1)
    {
        return rc;
    }

    /***********************************/
    /* clear the forced configurations */
    /***********************************/

    /************************************/
    /* remove : link UP on ingress port */
    /************************************/
    rc = cpssPxPortForceLinkPassEnableSet(devNum,portNum,GT_FALSE);
    if(rc != GT_OK)
    {
        cpssOsPrintf("failed : cpssPxPortForceLinkPassEnableSet , on port[%d] \n" ,
            portNum);
        return rc;
    }

    /*****************************************/
    /* remove : mac loopback on ingress port */
    /*****************************************/
    rc = cpssPxPortInternalLoopbackEnableSet(devNum,portNum,GT_FALSE);
    if(rc != GT_OK)
    {
        cpssOsPrintf("failed : cpssPxPortInternalLoopbackEnableSet , on port[%d] \n" ,
            portNum);
        return rc;
    }

    /* restore traffic ingress from the CPU port */
    rc = cpssPxIngressPortMapEntrySet(devNum,
                                      CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                      PRV_CPSS_PX_CPU_DMA_NUM_CNS,
                                      prvTgfRestoreCpuPortBmp[portNum], GT_FALSE);
    if(rc != GT_OK)
    {
        cpssOsPrintf("[TGF]: px_PCP_DST_PORT_MAP_TABLE_set FAILED, rc = [%d]", rc);
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvTgfPxInjectToIngressPortAndCheckEgress function
* @endinternal
*
* @brief   Send packet to ingress port and check expected egress ports.
*         function will set :
*         1. link UP
*         2. MAC LOOPBACK
*         3. set 'forwarding' tables to direct the packet from CPU to the port.
*         3. send from cpu packet to egress the port and LOOPBACK will make it
*         ingress the port.
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the ingress port num
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               - number of frames
* @param[in] numOfEgressPortsInfo     - number of egress ports info in array of egressPortsArr[]
*                                      if 0 --> not checking egress ports
* @param[in] egressPortsArr[]         - array of egress ports to check for counters and packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfPxInjectToIngressPortAndCheckEgress
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN TGF_PACKET_STC      *packetInfoPtr,
    IN GT_U32               burstCount,
    IN GT_U32               numOfEgressPortsInfo,
    IN TGF_EXPECTED_EGRESS_INFO_STC     egressPortsArr[]
)
{
    return pxInjectToIngressPortAndCheckEgress(devNum,portNum,
            packetInfoPtr,burstCount,
            numOfEgressPortsInfo,egressPortsArr,
            GT_TRUE/*isOrigSender*/);
}

/**
* @internal tgfTrafficGeneratorPxTransmitPacketAndCheckResult function
* @endinternal
*
* @brief   Send packet to ingress port and check expected egress ports.
*         function will set :
*         1. link UP
*         2. MAC LOOPBACK
*         3. set 'forwarding' tables to direct the packet from CPU to the port.
*         3. send from cpu packet to egress the port and LOOPBACK will make it
*         ingress the port.
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the ingress port num
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               - number of frames
* @param[in] numOfEgressPortsInfo     - number of egress ports info in array of egressPortsArr[]
*                                      if 0 --> not checking egress ports
* @param[in] egressPortsArr[]         - array of egress ports to check for counters and packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS tgfTrafficGeneratorPxTransmitPacketAndCheckResult
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN TGF_PACKET_STC      *packetInfoPtr,
    IN GT_U32               burstCount,
    IN GT_U32               numOfEgressPortsInfo,
    IN TGF_EXPECTED_EGRESS_INFO_STC     egressPortsArr[]
)
{
    return prvTgfPxInjectToIngressPortAndCheckEgress(devNum, portNum,
                                                     packetInfoPtr, burstCount,
                                                     numOfEgressPortsInfo, egressPortsArr);
}

/**
* @internal tgfTrafficGeneratorPxTransmitPacket function
* @endinternal
*
* @brief   Send packet to ingress port.
*         function will set :
*         1. link UP
*         2. MAC LOOPBACK
*         3. set 'forwarding' tables to direct the packet from CPU to the port.
*         3. send from cpu packet to egress the port and LOOPBACK will make it
*         ingress the port.
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the ingress port num
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               - number of frames
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS tgfTrafficGeneratorPxTransmitPacket
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN TGF_PACKET_STC      *packetInfoPtr,
    IN GT_U32               burstCount
)
{
    return pxInjectToIngressPort(devNum, portNum, packetInfoPtr, burstCount);
}

/**
* @internal prvTgfEgressPortForceLinkPassEnable function
* @endinternal
*
* @brief  Manage force link pass on egress port.
*         In case of running on emulator (MAC MIB counter issue),
*         loopback state and MRU should be changed depending on
*         force link pass and loopback state on egress port.
*
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the egress port num
* @param[in] enableForceLinkUp        - enable/disable force link pass on egress port
* @param[in] doWa                     - enable/disable MAC MIB counters WA on egress port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
*/
GT_STATUS prvTgfEgressPortForceLinkPassEnable
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL              enableForceLinkUp,
    IN GT_BOOL              doWa

)
{
    GT_STATUS   rc;
    GT_BOOL     enableLoopBack;
    GT_U32      do_egress_port_MIB_WA =
        cpssDeviceRunCheck_onEmulator() && egress_port_MIB_WA && doWa;

    rc = cpssPxPortForceLinkPassEnableSet(devNum, portNum, enableForceLinkUp);
    if(rc != GT_OK)
    {
        cpssOsPrintf("failed : cpssPxPortForceLinkPassEnableSet , on port[%d] \n" ,
            portNum);
        return rc;
    }

    if (do_egress_port_MIB_WA)
    {
        enableLoopBack = enableForceLinkUp;
        rc = cpssPxPortInternalLoopbackEnableSet(devNum, portNum, enableLoopBack);
        if(rc != GT_OK)
        {
            cpssOsPrintf("failed : "
                         "cpssPxPortInternalLoopbackEnableSet , on port[%d] \n" , portNum);
            return rc;
        }

        if (enableLoopBack == GT_FALSE)
        {
            rc = cpssPxPortMruSet(devNum, portNum, 1522);
            if(rc != GT_OK)
            {
                cpssOsPrintf("[TGF]: cpssPxPortMruSet [1522] FAILED, rc = [%d]", rc);
                return rc;
            }
        }
        else
        {
            rc = cpssPxPortMruSet(devNum, portNum, 0);
            if(rc != GT_OK)
            {
                cpssOsPrintf("[TGF]: cpssPxPortMruSet [0] FAILED, rc = [%d]", rc);
                return rc;
            }
        }
    }

    return GT_OK;
}

#endif

