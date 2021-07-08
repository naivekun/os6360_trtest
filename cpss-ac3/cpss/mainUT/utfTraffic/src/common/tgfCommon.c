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
* @file tgfCommon.c
*
* @brief Common helper API for enhanced UTs
*
* @version   97
********************************************************************************
*/

/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <utf/utfMain.h>
#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <common/tgfMirror.h>
#include <common/tgfBridgeGen.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <common/tgfCommon.h>
#include <common/tgfCscdGen.h>
#include <common/tgfCncGen.h>
#include <port/prvTgfPortIfModeSpeed.h>

/* include the AppDemo to get info about the infrastructure */
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPortGen.h>
#include <common/tgfVntGen.h>
#include <port/prvTgfPortFWS.h>
#include <cpss/common/init/cpssInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>

/*Auto Flow library*/
#include <common/tgfAutoFlow.h>
#ifdef EXMXPM_FAMILY
#include <../../cpssEnabler/mainCmd/h/cmdShell/common/cmdExtServices.h>
#include <cpss/exMxPm/exMxPmGen/config/private/prvCpssExMxPmInfo.h>
#include <cpss/exMxPm/exMxPmGen/bridge/cpssExMxPmBrgVlan.h>
#endif


/******************************************************************************\
 *              Default device\port numbers initialization                    *
\******************************************************************************/
/* devices array */
GT_U8   prvTgfDevsArray[PRV_TGF_MAX_PORTS_NUM_CNS]  =
    {PRV_TGF_DEV_0_CNS, PRV_TGF_DEV_0_CNS,  PRV_TGF_DEV_0_CNS,  PRV_TGF_DEV_0_CNS,
     PRV_TGF_DEV_0_CNS, PRV_TGF_DEV_0_CNS,  PRV_TGF_DEV_0_CNS,  PRV_TGF_DEV_0_CNS,
     PRV_TGF_DEV_0_CNS, PRV_TGF_DEV_0_CNS,  PRV_TGF_DEV_0_CNS,  PRV_TGF_DEV_0_CNS,
     PRV_TGF_DEV_0_CNS, PRV_TGF_DEV_0_CNS,  PRV_TGF_DEV_0_CNS,  PRV_TGF_DEV_0_CNS,
     PRV_TGF_DEV_0_CNS, PRV_TGF_DEV_0_CNS,  PRV_TGF_DEV_0_CNS,  PRV_TGF_DEV_0_CNS
     };

/* number of ports/devices in arrays of :prvTgfPortsArray,prvTgfDevsArray */
GT_U8   prvTgfPortsNum = PRV_TGF_DEFAULT_NUM_PORTS_IN_TEST_CNS;

/* Flag indication presence of AC3X (Aldrin + 88E1690) in the system */
static GT_BOOL prvTgfIsXcat3x = GT_FALSE;


/* specific port mode for tests */
PRV_TGF_DEVICE_PORT_MODE_ENT prvTgfDevicePortMode = PRV_TGF_DEVICE_PORT_MODE_DEFAULT_E;

/* port array size per device type */
#define PRV_TGF_DEV_TYPE_COUNT_CNS PRV_TGF_DEVICE_TYPE_COUNT_E

/* illegal value for 'physical' port numbers in port's array */
#define PRV_TGF_INVALID_PORT_NUM_CNS    UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(prvTgfDevNum)

/* set not used ports with illegal values */
static void setInvalidPortsInArr(
    IN GT_U32  portsArr[] ,
    IN GT_U32  startIndex ,
    IN GT_U32  numIndexes)
{
    GT_U32  ii;

    for(ii = startIndex ; ii < (startIndex+numIndexes) ; ii++)
    {
        portsArr[ii] = PRV_TGF_INVALID_PORT_NUM_CNS;
    }
}

/* flag to indicate if the system in PreInit stage */
static GT_BOOL  prvTgfCommonPreInitStage = GT_TRUE;

/* default ports array per device type */
static GT_U32   prvTgfDefPortsArray[PRV_TGF_DEV_TYPE_COUNT_CNS + 1][PRV_TGF_MAX_PORTS_NUM_CNS] =
        {
            {0,  8, 18, 23, 0, 0, 0, 0, 0, 0,
             0,  0,  0,  0, 0, 0, 0, 0, 0, 0},  /* 28 port devices */
#ifdef EXMXPM_FAMILY
            /* the Puma3 device hold '64' physical ports but the network ports
               that are interesting for us, should be like in Puma2 ('28' port device)*/
            {0,  8, 18, 23, 0, 0, 0, 0, 0, 0,
             0,  0,  0,  0, 0, 0, 0, 0, 0, 0},  /* 64 port devices */
#else /* ! EXMXPM_FAMILY */
            {0, 18, 36, 58, 0, 0, 0, 0, 0, 0,
             0,  0,  0,  0, 0, 0, 0, 0, 0, 0},  /* 64 port devices */
#endif /* ! EXMXPM_FAMILY */
            {120, 56, 86, 102, 0, 0, 0, 0, 0, 0,
               0,  0,  0,   0, 0, 0, 0, 0, 0, 0},  /* 128 port devices */
            /* mast be last -- used only for ERROR */
            {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}
        };

/* default port array for Lion2/3 40G configuration for 64 ports mode
   the tests should use local ports 0, 4, 8, 9 */
static GT_U32   prvTgfDef40GPortsArray_64[PRV_TGF_MAX_PORTS_NUM_CNS] =
            {0,  20, 40, 57, 0, 0, 0, 0, 0, 0,
             0,  0,  0,  0, 0, 0, 0, 0, 0, 0};

/* default port array for Lion2/3 40G configuration for 64 ports mode
             the tests should use local ports 0, 4, 8, 9 */
static GT_U32   prvTgfDef40GPortsArray_128[PRV_TGF_MAX_PORTS_NUM_CNS] =
            {120,  56, 84, 104, 0, 0, 0, 0, 0, 0,
             0,  0,  0,  0, 0, 0, 0, 0, 0, 0};

/* number of used ports in default ports array per device type */
static GT_U8   prvTgfDefPortsArraySize[PRV_TGF_DEV_TYPE_COUNT_CNS + 1] = {4, 4 , 4 , 4};

/* the selected run type - pre init (allowed before cpssInitSystem) parameter to force the array of ports used in the tests*/
static PRV_TGF_DEVICE_TYPE_ENT tgfDeviceTypeRunModePreInit = PRV_TGF_DEVICE_TYPE_COUNT_E;

/* the max number of ports that the device type supports */
static GT_U32  tgfDeviceTypeNumOfPorts[PRV_TGF_DEV_TYPE_COUNT_CNS + 1] = {28 , 64 , 128 , 0xFFFFFFFF};

/* seed value for random engine */
static GT_U32   prvTgfRandomSeedNum = 0;

/* flag to indecate if portsArray filled by valid random values */
static GT_BOOL  prvTgfIsRandomPortsSet = GT_FALSE;

/* flag to indecate if portsArray filled inPreInit stage */
static GT_BOOL  prvTgfPortsArrayPreInitSet = GT_FALSE;

/* the name of UTF device type  */
static GT_CHAR* tgfDeviceTypeNameArr[PRV_TGF_DEV_TYPE_COUNT_CNS + 1] = {
     "PRV_TGF_28_PORT_DEVICE_TYPE_E"
    ,"PRV_TGF_64_PORT_DEVICE_TYPE_E"
    ,"PRV_TGF_128_PORT_DEVICE_TYPE_E"
    ," ERROR "
    };


static GT_BOOL  useLastTpid = GT_FALSE;

static GT_U32   debugSleepTime = 0;


/* flag to use VNT time stamps for rate calculation */
extern GT_U32 prvTgfCommonVntTimeStampsInUse;

/* debug flag to check rate by VNT Time Stamp */
extern GT_U32 prvTgfCommonVntTimeStampsDebug;

extern GT_U32  prvTgfGetRate(IN GT_U32 countEnd, IN GT_U32 countStart, IN GT_U32 timeOutMilliSec);

GT_VOID prvTgfCommonPortsCntrRateGet
(
    IN  GT_U32 portIdxArr[],
    IN  GT_U32 numOfPorts,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    IN  GT_U32 timeOut,
    OUT GT_U32 outRateArr[]
);

/******************************************************************************\
 *                             Define section                                 *
\******************************************************************************/

/* total number of counters */
#define PRV_TGF_MAX_COUNTER_NUM_CNS    35

/* packet for transmitting */
static TGF_PACKET_STC   *prvTgfPacketInfoPtr = NULL;

/* array of VFDs */
static TGF_VFD_INFO_STC *prvTgfVfdArray      = NULL;

/* number of frames */
static GT_U32            prvTgfBurstCount    = 0;

/* number of VFDs */
static GT_U32            prvTgfNumVfd        = 0;

/* sleep time in milliseconds  */
#define hugeBurstSleepTime  50

#define RESET_VALUE_CNS 0

/* default packets count for huge burst */
#define PRV_TGF_DEF_BURST_TRACE_CNS 16

/* allow to do 'sleep' after X packets sent.
when = 0 , meaning NO SLEEP needed during the burst of 'burstCount'*/
static GT_U32            prvTgfBurstSleepAfterXCount = RESET_VALUE_CNS;
/* the sleep time (in milliseconds) after X packets sent , see parameter
prvTgfBurstSleepAfterXCount */
static GT_U32            prvTgfBurstSleepTime = RESET_VALUE_CNS;

/* packets count for huge burst */
static GT_U32            prvTgfBurstTraceCount = PRV_TGF_DEF_BURST_TRACE_CNS;


/* when SA macs burst is large we need to allow the appDemo to learn the SA
   (due to controlled learning) , so we set flag to state the
   'Need sleep after X count'

   flag is 'Auto cleared'

   SET by function prvTgfCommonIncrementalSaMacSend(...) for SA learning.
   checked by sendMultiDestinationTraffic(...)
*/
static GT_BOOL prvTgfUseSleepAfterXCount = GT_FALSE;

/* Loopback configuration status */
GT_BOOL tgfLoopBackCt10And40Enable = GT_FALSE;
/******************************* common payload *******************************/

/* Data of packet (long version) */
static GT_U8 prvTgfPacketPayloadLongDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

/* Payload part (long version) */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketLongPayloadPart =
{
    sizeof(prvTgfPacketPayloadLongDataArr), /* dataLength */
    prvTgfPacketPayloadLongDataArr          /* dataPtr */
};

/******************************* multi destination packet **********************************/

/*a.Send BC (FF:FF:FF:FF:FF:FF) with 100 incremental SA.*/
static TGF_PACKET_L2_STC multiDestination_prvTgfPacketL2Part_BC =
{
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},                /* daMac */
    {0x00, 0x01, 0x11, 0x11, 0x11, 0x11}                 /* saMac */
};

/*b.Send unregistered MC (01:02:03:04:05:06) with 100 incremental SA.*/
static TGF_PACKET_L2_STC multiDestination_prvTgfPacketL2Part_MC =
{
    {0x01, 0x02, 0x03, 0x04, 0x05, 0x06},                /* daMac */
    {0x00, 0x01, 0x11, 0x11, 0x11, 0x11}                 /* saMac */
};

/*c.Send unknown unicast (00:09:99:99:99:99) with 100 incremental DA and static SA */
/*d.Send unknown unicast (00:09:99:99:99:99) with 100 static DA and incremental SA*/
static TGF_PACKET_L2_STC multiDestination_prvTgfPacketL2Part_unk_UC =
{
    {0x00, 0x09, 0x99, 0x99, 0x99, 0x99},                /* daMac */
    {0x00, 0x01, 0x11, 0x11, 0x11, 0x11}                 /* saMac */
};

static  TGF_PACKET_L2_STC*  multiDestination_prvTgfPacketL2Array[] =
{
    &multiDestination_prvTgfPacketL2Part_BC,
    &multiDestination_prvTgfPacketL2Part_MC,
    &multiDestination_prvTgfPacketL2Part_unk_UC,
    &multiDestination_prvTgfPacketL2Part_unk_UC
};

/* PARTS of packet */
static TGF_PACKET_PART_STC multiDestination_prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &multiDestination_prvTgfPacketL2Part_BC},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* Length of packet */
#define MULTI_DESTINATION_PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadLongDataArr)

/* packet to send */
static TGF_PACKET_STC multiDestination_prvTgfPacketInfo =
{
    MULTI_DESTINATION_PRV_TGF_PACKET_LEN_CNS,                 /* totalLen */
    (sizeof(multiDestination_prvTgfPacketPartArray)
        / sizeof(multiDestination_prvTgfPacketPartArray[0])), /* numOfParts */
    multiDestination_prvTgfPacketPartArray                    /* partsArray */
};

static TGF_VFD_INFO_STC  multiDestination_prvTgfPacketL2Part_SA_incemental_vfdInfo=
{
    TGF_VFD_MODE_INCREMENT_E,
    1,
    0,
    6,  /* offset in the packet to override */
    {0},/* saMac */
    {0},/* mask */
    6,
    NULL,
    0,
    0
};

static TGF_VFD_INFO_STC  multiDestination_prvTgfPacketL2Part_DA_incemental_vfdInfo=
{
    TGF_VFD_MODE_INCREMENT_E,
    1,
    0,
    0,  /* offset in the packet to override */
    {0},/* daMac */
    {0},/* mask */
    6,
    NULL,
    0,
    0
};

static TGF_VFD_INFO_STC  multiDestination_prvTgfPacketL2Part_SA_static_vfdInfo=
{
    TGF_VFD_MODE_STATIC_E,
    1,
    0,
    6,  /* offset in the packet to override */
    {0},/* daMac */
    {0},/* mask */
    6,
    NULL,
    0,
    0,
};

/******************************************************************************\
 *                     Private function's implementation                      *
\******************************************************************************/

/**
* @internal prvTgfCommonFixHwDevId function
* @endinternal
*
* @brief   Function fixes using SW dev Id instead of HW dev Id
*
* @param[in] swDevId                  - value that should be hwDevId, but probably used swDevId
* @param[in] toKeepNull               - relevant only when swDevId == 0
*                                      GT_TRUE - to return 0, GT_FALSE - convert to HW Id
*                                       HW dev Id
*/
GT_U32 prvTgfCommonFixHwDevId
(
    IN  GT_U32  swDevId,
    IN  GT_BOOL toKeepNull
)
{
    GT_U8                      dev;
    PRV_CPSS_GEN_PP_CONFIG_STC *cfgPtr;

    if ((toKeepNull != GT_FALSE) && (swDevId == 0))
    {
        return 0;
    }

    if (swDevId >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        /* used value is not SW device id */
        return swDevId;
    }

    if (PRV_CPSS_PP_MAC(swDevId) == NULL)
    {
        /* used value is not SW device id */
        return swDevId;
    }

    /* check that used value is not known HW device Id */
    for (dev = 0; (dev < PRV_CPSS_MAX_PP_DEVICES_CNS); dev++)
    {
        cfgPtr = PRV_CPSS_PP_MAC(dev);
        if (cfgPtr == NULL)
        {
            continue;
        }
        if (cfgPtr->hwDevNum == swDevId)
        {
            /* used value known HW device id */
            return swDevId;
        }
    }

    /* conversion SW device Id to HW device Id*/
    return PRV_CPSS_PP_MAC(swDevId)->hwDevNum;
}

/**
* @internal prvTgfCommonVntTimeStampsDebugSet function
* @endinternal
*
* @brief   Private function to set debug flag for time stamp debug
*/
GT_U32 prvTgfCommonVntTimeStampsDebugSet(GT_U32 value)
{
    GT_U32 oldValue = prvTgfCommonVntTimeStampsDebug;
    prvTgfCommonVntTimeStampsDebug = value;
    return oldValue;
}

/**
* @internal prvTgfCountersPrint function
* @endinternal
*
* @brief   Private function to print port's counters
*
* @param[in] portCountersPtr          - (pointer to) port's counters to print
* @param[in] portNum                  - port number
*                                       None
*/
static GT_VOID prvTgfCountersPrint
(
    IN const CPSS_PORT_MAC_COUNTER_SET_STC   *portCountersPtr,
    IN  GT_PHYSICAL_PORT_NUM           portNum
)
{
    static const GT_CHAR *cntFieldNames[PRV_TGF_MAX_COUNTER_NUM_CNS] =
                            {"goodOctetsRcv", "badOctetsRcv", "macTransmitErr",
                             "goodPktsRcv", "badPktsRcv", "brdcPktsRcv", "mcPktsRcv",
                             "pkts64Octets", "pkts65to127Octets", "pkts128to255Octets",
                             "pkts256to511Octets", "pkts512to1023Octets", "pkts1024tomaxOoctets",
                             "goodOctetsSent", "goodPktsSent", "excessiveCollisions",
                             "mcPktsSent", "brdcPktsSent", "unrecogMacCntrRcv",
                             "fcSent", "goodFcRcv", "dropEvents", "undersizePkts",
                             "fragmentsPkts", "oversizePkts", "jabberPkts", "macRcvError",
                             "badCrc", "collisions", "lateCollisions", "badFcRcv", "ucPktsRcv",
                             "ucPktsSent", "multiplePktsSent", "deferredPktsSent"};
    GT_U32   cntHi32Bits  = 0;
    GT_U32   cntLow32Bits = 0;
    GT_U32   iter         = 0;
    GT_U32  *portCntPtr   = NULL;
    GT_BOOL  isNonZeroCnt = GT_FALSE;


    portCntPtr = (GT_U32*) portCountersPtr;

    /* print counters */
    PRV_UTF_LOG1_MAC("Port [%d] counters:\n", portNum);

    for (iter = 0; iter < PRV_TGF_MAX_COUNTER_NUM_CNS; iter++)
    {
        /* get Hi and Low 32 bits of the port counter */
        cntHi32Bits  = *(portCntPtr + 1);
        cntLow32Bits = *(portCntPtr++);

        /* print only non zero counters */
        if ((cntHi32Bits != 0) || (cntLow32Bits != 0))
        {
            /* find non zero counter */
            isNonZeroCnt = GT_TRUE;

            PRV_UTF_LOG1_MAC(" %s = ", cntFieldNames[iter]);
            if (cntHi32Bits != 0)
            {
                PRV_UTF_LOG1_MAC("%d", cntHi32Bits);
            }
            PRV_UTF_LOG1_MAC("%d\n", cntLow32Bits);
        }

        portCntPtr++;
    }

    /* check if all counters are zero */
    if (GT_FALSE == isNonZeroCnt)
    {
        PRV_UTF_LOG0_MAC(" All counters are 0\n");
    }
    PRV_UTF_LOG0_MAC("\n");
}


/******************************************************************************\
 *                              API implementation                            *
\******************************************************************************/

/**
* @internal tgfSimSleep function
* @endinternal
*
* @brief   function to sleep given time (simulation only)
*
* @param[in] sleepTime                - sleep time (milliseconds)
*                                       None
*/
GT_VOID tgfSimSleep
(
    IN GT_U32  sleepTime
)
{
#ifdef ASIC_SIMULATION
    cpssOsTimerWkAfter(sleepTime);
#endif /* ASIC_SIMULATION */
}


/**
* @internal prvTgfCommonIsExternalTcamUseForced function
* @endinternal
*
* @brief   Checks the preconfigured request to use External TCAM instead of Internal TCAM in tests.
*         For such preconfiguration set forceExtTcamUseInTests=1.
*
* @retval IsExternalTcamUseForced  - GT_TRUE use External TCAM, GT_FALSE - use internal TCAM
*/
GT_BOOL prvTgfCommonIsExternalTcamUseForced
(
    GT_VOID
)
{
#ifdef EXMXPM_FAMILY
    static GT_BOOL firstCall = GT_FALSE;
    static GT_BOOL result    = GT_FALSE;
    GT_U32 value;

    if (PRV_CPSS_EXMXPM_PP_MAC(prvTgfDevNum)->
        moduleCfg.externalTcamCfg.externalTcamUsed == GT_FALSE)
    {
        /* external TCAM not used */
        return GT_FALSE;
    }

    if (firstCall != GT_FALSE)
    {
        return result;
    }

    firstCall = GT_TRUE;

    if (cmdAppDbEntryGet("forceExtTcamUseInTests", &value) != GT_OK)
    {
        result = GT_FALSE;
        return result;
    }

    result = (value == 0) ? GT_FALSE : GT_TRUE;
    return result;
#else /*EXMXPM_FAMILY*/
    return GT_FALSE;
#endif
}

/**
* @internal prvTgfResetCountersEth function
* @endinternal
*
* @brief   Reset port counters and set this port in linkup mode
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
*/
GT_STATUS prvTgfResetCountersEth
(
    IN GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_STATUS rc = GT_OK;

    /* map input params */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = devNum;
    portInterface.devPort.portNum = portNum;

    /* reset counters */
    rc = tgfTrafficGeneratorPortCountersEthReset(&portInterface);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgfTrafficGeneratorPortCountersEthReset FAILED, rc = [%d]", rc);

        return rc;
    }

    /* set port into linkup mode */
    rc = tgfTrafficGeneratorPortForceLinkUpEnableSet(&portInterface, GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgfTrafficGeneratorPortForceLinkUpEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
}

/**
* @internal prvTgfSetTxSetupEth function
* @endinternal
*
* @brief   Setting transmit parameters
*
* @param[in] devNum                   - the CPU device number
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               - number of frames
* @param[in] numVfd                   - number of VFDs
* @param[in] vfdArray[]               - (pointer to) array of VFDs
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfSetTxSetupEth
(
    IN GT_U8                devNum,
    IN TGF_PACKET_STC      *packetInfoPtr,
    IN GT_U32               burstCount,
    IN GT_U32               numVfd,
    IN TGF_VFD_INFO_STC     vfdArray[]
)
{
    GT_STATUS rc = GT_OK;

    /* enable tracing */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgfTrafficTracePacketByteSet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* set the CPU device to send traffic */
    rc = tgfTrafficGeneratorCpuDeviceSet(devNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgfTrafficGeneratorCpuDeviceSet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* bind parameters into local variables */
    prvTgfPacketInfoPtr = packetInfoPtr;
    prvTgfBurstCount    = burstCount;
    prvTgfNumVfd        = numVfd;
    prvTgfVfdArray      = vfdArray;

    /* default for option that can be set by function prvTgfSetTxSetup2Eth(...) */
    prvTgfBurstSleepAfterXCount = RESET_VALUE_CNS;
    prvTgfBurstSleepTime        = RESET_VALUE_CNS;


    return (GT_ALREADY_EXIST == rc) ? GT_OK : rc;
}

/**
* @internal prvTgfSetTxSetup2Eth function
* @endinternal
*
* @brief   Setting transmit 2 parameters .
*         NOTE:
*         This function should be called after prvTgfSetTxSetupEth(..) ,
*         when need to set the parameters of this function.
* @param[in] sleepAfterXCount         - do 'sleep' after X packets sent
*                                      when = 0 , meaning NO SLEEP needed during the burst
*                                      of 'burstCount'
* @param[in] sleepTime                - sleep time (in milliseconds) after X packets sent , see
*                                      parameter sleepAfterXCount
*
* @retval GT_OK                    - on success.
*/
GT_STATUS prvTgfSetTxSetup2Eth
(
    IN GT_U32               sleepAfterXCount,
    IN GT_U32               sleepTime
)
{

    prvTgfBurstSleepAfterXCount = sleepAfterXCount;
    prvTgfBurstSleepTime        = sleepTime;

    return GT_OK;
}

/**
* @internal prvTgfBurstTraceSet function
* @endinternal
*
* @brief   Set max number of packets in burst count that will be printed
*
* @param[in] packetCount              - disable logging when burst is lager then packetCount
*
* @retval GT_OK                    - on success.
*
* @note To restore default setting call function with packetCount == 0
*
*/
GT_STATUS prvTgfBurstTraceSet
(
    IN GT_U32       packetCount
)
{
    prvTgfBurstTraceCount = (0 == packetCount) ? PRV_TGF_DEF_BURST_TRACE_CNS : packetCount;

    return GT_OK;
}

/**
* @internal prvTgfStartTransmitingEth function
* @endinternal
*
* @brief   Transmit packet
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
*/
GT_STATUS prvTgfStartTransmitingEth
(
    IN GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum
)
{
    CPSS_INTERFACE_INFO_STC  portInterface;
    GT_STATUS                rc         = GT_OK;
    GT_U32                      freeIndex;
    GT_U32                      index;
    PRV_TGF_TX_DEV_PORT_STC     *modePtr;
    GT_STATUS                   modeRc;
    GT_U32                      burstCount = prvTgfBurstCount;

    /* get DB entry  */
    modeRc = prvTgfTxContModeEntryGet(devNum, portNum, &modePtr, &index, &freeIndex);

    /* clear portInterface */
    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));

    /* fill portInterface struct */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = devNum;
    portInterface.devPort.portNum = portNum;

    /* set port in loopback mode */
    if (prvTgfFwsUse != 1)
    {
        rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_TRUE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG2_MAC("[TGF]: tgfTrafficGeneratorPortLoopbackModeEnableSet FAILED, rc = [%d] , port[%d]", rc , portNum);

            return rc;
        }
    }

    if (modeRc == GT_OK)
    {
        /* continuous mode is chosen */
        rc = prvTgfMirrorRxPortSet(portNum, GT_TRUE, index);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfMirrorRxPortSet FAILED, rc = [%d]", rc);
            return rc;
        }

        /* set burst count to provide WS traffic */
        burstCount = prvTgfWsBurst;

        /* previous logic result in link down/up and link change interrupts.
           Link change event handler manages egress link filtering for SIP_5_20 and above.
           Need to provide time for event handler before start send packets. */
        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            cpssOsTimerWkAfter(5);
        }
    }

    /* transmit packet */
    rc = tgfTrafficGeneratorPortTxEthTransmit(&portInterface,
                                              prvTgfPacketInfoPtr,
                                              burstCount,
                                              prvTgfNumVfd,
                                              prvTgfVfdArray,
                                              prvTgfBurstSleepAfterXCount,
                                              prvTgfBurstSleepTime,
                                              prvTgfBurstTraceCount,
                                              GT_TRUE);


    /* do not disable loopback for continuous */
    if ((modeRc != GT_OK) && (prvTgfFwsUse != 1))
    {
        /* disable loopback mode on port */
        (void) tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_FALSE);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgfTrafficGeneratorPortTxEthTransmit FAILED, rc = [%d]", rc);
    }

    return rc;
}
/**
* @internal prvTgfStopTransmitingEth function
* @endinternal
*
* @brief   Stop transmit packets
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
*/
GT_STATUS prvTgfStopTransmitingEth
(
    IN GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum
)
{
    CPSS_INTERFACE_INFO_STC  portInterface;
    GT_STATUS                rc         = GT_OK;
    GT_U32                      freeIndex;
    GT_U32                      index;
    PRV_TGF_TX_DEV_PORT_STC     *modePtr;
    GT_STATUS                   modeRc;

    /* get DB entry  */
    modeRc = prvTgfTxContModeEntryGet(devNum, portNum, &modePtr, &index, &freeIndex);

    if (modeRc != GT_OK)
    {
        /* port in burst mode. Nothing to do. */
        return GT_OK;
    }


    /* clear portInterface */
    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));

    /* fill portInterface struct */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = devNum;
    portInterface.devPort.portNum = portNum;

    /* the Port Loopback disable under traffic may result in problems on port.
      Disable port MAC before disable loopback to minimize problems */
    rc = prvTgfPortEnableSet(devNum, portNum, GT_FALSE);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* disable loopback mode on port */
    (void) tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_FALSE);

    /* enable port back */
    rc = prvTgfPortEnableSet(devNum, portNum, GT_TRUE);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* disable mirroring */
    rc = prvTgfMirrorRxPortSet(portNum, GT_FALSE, index);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfMirrorRxPortSet FAILED, rc = [%d]", rc);
        return rc;
    }

    return GT_OK;
}
/**
* @internal prvTgfReadPortCountersEth function
* @endinternal
*
* @brief   Read port's counters
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enablePrint              - enable\disable counters print
*
* @param[out] portCountersPtr          - (pointer to) received counters values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_FAIL                  - if test doesn't pass comparing.
*/
GT_STATUS prvTgfReadPortCountersEth
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  GT_BOOL                        enablePrint,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC *portCountersPtr
)
{
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_STATUS                       rc = GT_OK;


    /* clear portInterface */
    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));

    /* fill portInterface struct */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = devNum;
    portInterface.devPort.portNum = portNum;

    /* read counters */
    rc = tgfTrafficGeneratorPortCountersEthRead(&portInterface, portCountersPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgfTrafficGeneratorPortCountersEthRead FAILED, rc = [%d]", rc);

        return rc;
    }

    /* print counters */
    if (GT_TRUE == enablePrint)
    {
        prvTgfCountersPrint(portCountersPtr, portNum);
    }

    return rc;
}

/**
* @internal prvTgfPrintPortCountersEth function
* @endinternal
*
* @brief   Print counters of all TGF ports
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_FAIL                  - if test doesn't pass comparing.
*/
GT_STATUS prvTgfPrintPortCountersEth
(
    GT_VOID
)
{
    OUT CPSS_PORT_MAC_COUNTER_SET_STC  portCounters;
    GT_U32  portIndex;
    GT_STATUS rc;

    PRV_UTF_LOG1_MAC(
        "Reading counters of %d ports:\n",
        prvTgfPortsNum);

    for (portIndex = 0; (portIndex < prvTgfPortsNum); portIndex++)
    {
        rc = prvTgfReadPortCountersEth(
            prvTgfDevsArray[portIndex],
            prvTgfPortsArray[portIndex],
            GT_FALSE /*enablePrint*/,
            &portCounters);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC(
                "prvTgfReadPortCountersEth failed, dev: %d, port: %d\n",
                prvTgfDevsArray[portIndex],
                prvTgfPortsArray[portIndex]);
            return rc;
        }
        PRV_UTF_LOG6_MAC(
            "dev %d port %d RX packets %d, bytes %d, TX packets %d, bytes %d\n",
            prvTgfDevsArray[portIndex], prvTgfPortsArray[portIndex],
            portCounters.goodPktsRcv.l[0], portCounters.goodOctetsRcv.l[0],
            portCounters.goodPktsSent.l[0], portCounters.goodOctetsSent.l[0]);
    }

    return GT_OK;
}

/**
* @internal prvTgfPacketSizeGet function
* @endinternal
*
* @brief   Calculate packet size
*
* @param[in] partsArray[]             - packet part's array
* @param[in] partsCount               - number of parts
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_FAIL                  - if test doesn't pass comparing.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfPacketSizeGet
(
    IN  TGF_PACKET_PART_STC  partsArray[],
    IN  GT_U32               partsCount,
    OUT GT_U32              *packetSizePtr
)
{
    GT_U32      packetIter = 0;
    GT_U32      packetSize = 0;


    CPSS_NULL_PTR_CHECK_MAC(partsArray);
    CPSS_NULL_PTR_CHECK_MAC(packetSizePtr);

    /* calc packet size */
    packetSize = 0;
    for (packetIter = 0; packetIter < partsCount; packetIter++)
    {
        switch (partsArray[packetIter].type)
        {
            case TGF_PACKET_PART_L2_E:
                packetSize += TGF_L2_HEADER_SIZE_CNS;
                break;

            case TGF_PACKET_PART_VLAN_TAG_E:
                packetSize += TGF_VLAN_TAG_SIZE_CNS;
                break;

            case TGF_PACKET_PART_DSA_TAG_E:
                /* take the length of the DSA tag from the type (1 or 2 or 3 or 4 words) */
                packetSize += TGF_DSA_NUM_BYTES_MAC(partsArray[packetIter].partPtr);
                break;

            case TGF_PACKET_PART_ETHERTYPE_E:
                packetSize += TGF_ETHERTYPE_SIZE_CNS;
                break;

            case TGF_PACKET_PART_MPLS_E:
                packetSize += TGF_MPLS_HEADER_SIZE_CNS;
                break;

            case TGF_PACKET_PART_IPV4_E:
                packetSize += TGF_IPV4_HEADER_SIZE_CNS;
                break;

            case TGF_PACKET_PART_IPV6_E:
                packetSize += TGF_IPV6_HEADER_SIZE_CNS;
                break;

            case TGF_PACKET_PART_TCP_E:
                packetSize += TGF_TCP_HEADER_SIZE_CNS;
                break;

            case TGF_PACKET_PART_UDP_E:
                packetSize += TGF_UDP_HEADER_SIZE_CNS;
                break;

            case TGF_PACKET_PART_ARP_E:
                packetSize += TGF_ARP_HEADER_SIZE_CNS;
                break;

            case TGF_PACKET_PART_ICMP_E:
                packetSize += TGF_ICMP_HEADER_SIZE_CNS;
                break;

            case TGF_PACKET_PART_WILDCARD_E:
                packetSize += ((TGF_PACKET_PAYLOAD_STC*) partsArray[packetIter].partPtr)->dataLength;
                break;

            case TGF_PACKET_PART_PAYLOAD_E:
                packetSize += ((TGF_PACKET_WILDCARD_STC*) partsArray[packetIter].partPtr)->numOfBytes;
                break;

            case TGF_PACKET_PART_GRE_E:
                packetSize += 4;
                if (((TGF_PACKET_GRE_STC*) partsArray[packetIter].partPtr)->checkSumPresent)
                {
                    packetSize += 4;
                }
                break;

            case TGF_PACKET_PART_SKIP_E:
                break;/* skip this part */

            default:
                return GT_BAD_PARAM;
        }
    }

    *packetSizePtr = packetSize;

    return GT_OK;
}

/**
* @internal prvTgfPortCapturedPacketPrint function
* @endinternal
*
* @brief   Print captured packet on specific port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
*/
GT_STATUS prvTgfPortCapturedPacketPrint
(
    IN GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum
)
{
    GT_STATUS       rc, rc1  = GT_OK;
    GT_U8           dev      = 0;
    GT_U8           queue    = 0;
    GT_BOOL         getFirst = GT_TRUE;
    GT_U32          buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32          packetActualLength = 0;
    TGF_NET_DSA_STC rxParam = {0};
    GT_U8           packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
#ifdef GM_USED
    GT_U32          totalDelay = 1000;
    GT_U32          delay      = 100;
#endif /*GM_USED*/
    CPSS_INTERFACE_INFO_STC  portInterface;

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = devNum;
    portInterface.devPort.portNum = portNum;

    /* disable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_FALSE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgfTrafficTracePacketByteSet FAILED, rc = [%d]", rc);
    }

    while(1)
    {
        buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;

        /* get entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&portInterface,
                                           TGF_PACKET_TYPE_CAPTURE_E,
                                           getFirst, GT_TRUE, packetBuff,
                                           &buffLen, &packetActualLength,
                                           &dev, &queue, &rxParam);
        if (GT_OK != rc)
        {
#ifdef GM_USED
            if ((getFirst == GT_TRUE) && (totalDelay >= delay))
            {
                /* wait for yet nor reached packets */
                cpssOsTimerWkAfter(delay);
                totalDelay -= delay;
                continue;
            }
#endif /*GM_USED*/
            break;
        }

        /* get the next entry */
        getFirst = GT_FALSE;

        /* enable packet trace */
        rc = tgfTrafficTracePacketByteSet(GT_TRUE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: tgfTrafficTracePacketByteSet FAILED, rc = [%d]", rc);
        }

        /* trace packet */
        PRV_UTF_LOG2_MAC(
            "[TGF]: Captured packet dev [%d] port [%d]\n",
            devNum, portNum);
        rc = tgfTrafficTracePacket(packetBuff, packetActualLength, GT_TRUE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: tgfTrafficTracePacket FAILED, rc = [%d]", rc);

            break;
        }

        /* disable packet trace */
        rc = tgfTrafficTracePacketByteSet(GT_FALSE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: tgfTrafficTracePacketByteSet FAILED, rc = [%d]", rc);
        }
    }

    /* enable packet trace */
    rc1 = tgfTrafficTracePacketByteSet(GT_TRUE);
    if (GT_OK != rc1)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgfTrafficTracePacketByteSet FAILED, rc = [%d]", rc1);
    }

    return ((GT_NO_MORE == rc || GT_NOT_FOUND == rc) && (GT_OK == rc1)) ? GT_OK : rc;
}

/**
* @internal prvTgfEthCountersCheck function
* @endinternal
*
* @brief   Read and check port's counters
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] packetsCountRx           - Expected number of Rx packets
* @param[in] packetsCountTx           - Expected number of Tx packets
* @param[in] packetSize               - Size of packets, 0 means don't check
* @param[in] burstCount               - Number of packets with the same size
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
*/
GT_STATUS prvTgfEthCountersCheck
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  GT_U32                         packetsCountRx,
    IN  GT_U32                         packetsCountTx,
    IN  GT_U32                         packetSize,
    IN  GT_U32                         burstCount
)
{
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32                        isCountersOk;
    GT_STATUS                     rc;
    GT_U32                        i;
    GT_U32                        expected_goodPktsSent;
    GT_U32                        expected_goodPktsRcv;
    GT_U32                        expected_goodOctetsSent;
    GT_U32                        expected_goodOctetsRcv;
    GT_U32                        result_goodPktsSent;
    GT_U32                        result_goodPktsRcv;
    GT_U32                        result_goodOctetsSent;
    GT_U32                        result_goodOctetsRcv;

    /* initial values*/
    isCountersOk = 0;

    /* clear result accomulators */
    result_goodPktsSent    = 0;
    result_goodPktsRcv     = 0;
    result_goodOctetsSent  = 0;
    result_goodOctetsRcv   = 0;

    /* calculate expected numder of pakets */
    expected_goodPktsSent   = packetsCountTx * burstCount;
    expected_goodPktsRcv    = packetsCountRx * burstCount;
    expected_goodOctetsSent = (packetSize + TGF_CRC_LEN_CNS) * packetsCountTx * burstCount;
    expected_goodOctetsRcv  = (packetSize + TGF_CRC_LEN_CNS) * packetsCountRx * burstCount;

    /* this loop provides the additional attempts of counter reading  */
    /* in the case when counters less then expected                   */
    /* it can fix the problems occuring due to slow simulation        */
    for (i = 0; (i < 10); i++)
    {
        /* read counters - clear on read registers */
        rc = prvTgfReadPortCountersEth(
            devNum, portNum, GT_FALSE, &portCntrs);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfReadPortCountersEth");

        /* accomulate the read result */
        result_goodPktsSent    += portCntrs.goodPktsSent.l[0];
        result_goodPktsRcv     += portCntrs.goodPktsRcv.l[0];
        result_goodOctetsSent  += portCntrs.goodOctetsSent.l[0];
        result_goodOctetsRcv   += portCntrs.goodOctetsRcv.l[0];


        /* check Tx and Rx counters */
        isCountersOk =
            result_goodPktsSent   == expected_goodPktsSent &&
            result_goodPktsRcv    == expected_goodPktsRcv;
        if (packetSize != 0)
        {
            isCountersOk = isCountersOk &&
                result_goodOctetsSent == expected_goodOctetsSent &&
                result_goodOctetsRcv  == expected_goodOctetsRcv;
        }

        if (isCountersOk)
        {
            break;
        }

#ifdef GM_USED
        /* result already more then expected */
        if (result_goodPktsSent > expected_goodPktsSent)
        {
            break;
        }
        if (result_goodPktsRcv > expected_goodPktsRcv)
        {
            break;
        }
        if (packetSize != 0)
        {
            if (result_goodOctetsSent > expected_goodOctetsSent)
            {
                break;
            }
            if (result_goodOctetsRcv > expected_goodOctetsRcv)
            {
                break;
            }
        }

        /* wait for yet nor reached packets */
        cpssOsTimerWkAfter(100);

#else /*GM_USED*/
        i = 0x0FFFFFF0; /*instead of "break" to avoid coverity warning */
#endif /*GM_USED*/
    }

    /* print retrieved values if bug */
    PRV_UTF_LOG1_MAC("Port %d - Retrieved Counter values:\n", portNum);
    if (result_goodPktsSent)
        PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", result_goodPktsSent);
    if (result_goodPktsRcv)
        PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", result_goodPktsRcv);
    if (result_goodOctetsSent)
        PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", result_goodOctetsSent);
    if (result_goodOctetsRcv)
        PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", result_goodOctetsRcv);
    if ((result_goodPktsSent == 0) &&
        (result_goodPktsRcv == 0) &&
        (result_goodOctetsSent == 0) &&
        (result_goodOctetsRcv == 0))
    {
        PRV_UTF_LOG0_MAC(" zero \n" );
    }
    PRV_UTF_LOG0_MAC(" \n");

    if (isCountersOk == 0)
    {
        /* print expected values if bug */
        PRV_UTF_LOG0_MAC("Expected Counter values:\n");
        if (expected_goodPktsSent)
            PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", expected_goodPktsSent);
        if (expected_goodPktsRcv)
            PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", expected_goodPktsRcv);
        if (expected_goodOctetsSent)
            PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", expected_goodOctetsSent);
        if (expected_goodOctetsRcv)
            PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", expected_goodOctetsRcv);
        if ((expected_goodPktsSent == 0) &&
            (expected_goodPktsRcv == 0) &&
            (expected_goodOctetsSent == 0) &&
            (expected_goodOctetsRcv == 0))
        {
            PRV_UTF_LOG0_MAC(" zero \n" );
        }
        PRV_UTF_LOG0_MAC(" \n");
    }

    return (isCountersOk == 0) ? GT_FAIL : GT_OK;
}

/**
* @internal prvTgfEthPortSetCountersCheck function
* @endinternal
*
* @brief   Read and check counters of set of ports
*
* @param[in] devNum                   - device number
* @param[in] allPortsBmpPtr           - (pointer to)bitmap of all checked ports
* @param[in] rxPortsBmpPtr            - (pointer to)bitmap of ports expected to receive packets
*                                      must be subset of allPortsBmpPtr
* @param[in] txPortsBmpPtr            - (pointer to)bitmap of ports expected to send packets
*                                      must be subset of allPortsBmpPtr
* @param[in] packetSize               - Size of packets, 0 means don't check
* @param[in] burstCount               - Number of packets with the same size
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
*/
GT_STATUS prvTgfEthPortSetCountersCheck
(
    IN  GT_U8                          devNum,
    IN  CPSS_PORTS_BMP_STC             *allPortsBmpPtr,
    IN  CPSS_PORTS_BMP_STC             *rxPortsBmpPtr,
    IN  CPSS_PORTS_BMP_STC             *txPortsBmpPtr,
    IN  GT_U32                         packetSize,
    IN  GT_U32                         burstCount
)
{
    GT_STATUS                      rc, rc1;
    GT_PHYSICAL_PORT_NUM           portNum;
    GT_U32                         packetsCountRx;
    GT_U32                         packetsCountTx;

    rc1 = GT_OK;

    for (portNum = 0; (portNum < CPSS_MAX_PORTS_NUM_CNS); portNum++)
    {
        if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(allPortsBmpPtr, portNum) == 0)
        {
            continue;
        }
        packetsCountRx = CPSS_PORTS_BMP_IS_PORT_SET_MAC(rxPortsBmpPtr, portNum);
        packetsCountTx = CPSS_PORTS_BMP_IS_PORT_SET_MAC(txPortsBmpPtr, portNum);
        rc = prvTgfEthCountersCheck(
            devNum, portNum, packetsCountRx, packetsCountTx,
            packetSize, burstCount);
        if ((rc != GT_OK) && (rc1 == GT_OK))
        {
            rc1 = rc;
        }
    }

    return rc1;
}

/**
* @internal prvTgfEthCountersReset function
* @endinternal
*
* @brief   Reset all Ethernet port's counters
*         also do tgfTrafficTableRxPcktTblClear()
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong interface
*/
GT_STATUS prvTgfEthCountersReset
(
    IN GT_U8 devNum
)
{
    GT_U32    portIter;
    GT_STATUS rc, rc1 = GT_OK;

    PRV_UTF_LOG1_MAC("======= reset mac counters of first[%d] tested ports. =======\n",prvTgfPortsNum);
    for (portIter = 0; portIter < prvTgfPortsNum ; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(devNum, prvTgfPortsArray[portIter]);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfResetCountersEth");
    }

    PRV_UTF_LOG0_MAC("======= Clear capturing RxPcktTable. =======\n");
    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    return rc1;
}

/**
* @internal prvTgfEthIngressPacketsCounterCheck function
* @endinternal
*
* @brief   Read and check port's ingress packets counter
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] packetsCountRx           - Expected number of Rx packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
*/
GT_STATUS prvTgfEthIngressPacketsCounterCheck
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  GT_U32                         packetsCountRx
)
{
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32                        isCountersOk;
    GT_STATUS                     rc;
    GT_U32                        i;
    GT_U32                        expected_goodPktsRcv;
    GT_U32                        result_goodPktsSent;
    GT_U32                        result_goodPktsRcv;
    GT_U32                        result_goodOctetsSent;
    GT_U32                        result_goodOctetsRcv;

    /* initial values*/
    isCountersOk = 0;

    /* clear result accomulators */
    result_goodPktsSent    = 0;
    result_goodPktsRcv     = 0;
    result_goodOctetsSent  = 0;
    result_goodOctetsRcv   = 0;

    expected_goodPktsRcv   = packetsCountRx;

    /* this loop provides the additional attempts of counter reading  */
    /* in the case when counters less then expected                   */
    /* it can fix the problems occuring due to slow simulation        */
    for (i = 0; (i < 10); i++)
    {
        /* read counters - clear on read registers */
        rc = prvTgfReadPortCountersEth(
            devNum, portNum, GT_FALSE, &portCntrs);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfReadPortCountersEth");

        /* accomulate the read result */
        result_goodPktsSent    += portCntrs.goodPktsSent.l[0];
        result_goodPktsRcv     += portCntrs.goodPktsRcv.l[0];
        result_goodOctetsSent  += portCntrs.goodOctetsSent.l[0];
        result_goodOctetsRcv   += portCntrs.goodOctetsRcv.l[0];

        /* check Tx and Rx counters */
        isCountersOk = result_goodPktsRcv == expected_goodPktsRcv;

        if (isCountersOk)
        {
            break;
        }

#ifdef GM_USED
        if (result_goodPktsRcv > expected_goodPktsRcv)
        {
            break;
        }

        /* wait for yet nor reached packets */
        cpssOsTimerWkAfter(100);

#else /*GM_USED*/
        i = 0x0FFFFFF0; /*instead of "break" to avoid coverity warning */
#endif /*GM_USED*/
    }

    /* print retrieved values if bug */
    PRV_UTF_LOG1_MAC("Port %d - Retrieved Counter values:\n", portNum);
    if (result_goodPktsSent)
        PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", result_goodPktsSent);
    if (result_goodPktsRcv)
        PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", result_goodPktsRcv);
    if (result_goodOctetsSent)
        PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", result_goodOctetsSent);
    if (result_goodOctetsRcv)
        PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", result_goodOctetsRcv);
    PRV_UTF_LOG0_MAC(" \n");

    if (isCountersOk == 0)
    {
        /* print expected values if bug */
        PRV_UTF_LOG0_MAC("Expected Counter values:\n");
            PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", expected_goodPktsRcv);
        PRV_UTF_LOG0_MAC(" \n");
    }

    return (isCountersOk == 0) ? GT_FAIL : GT_OK;
}

/**
* @internal prvTgfEthCounterLoopbackEgrPort function
* @endinternal
*
* @brief   Enable/disable loopback on port to count ingress packets
*         instead of egress counters.
*         Workaround on BC2 GM that counts egress packets dropped by EPCL.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] loopback                 - GT_TRUE - enable loopback.
*                                      pVid and pVidForce retrieved, saved,
*                                      and set pVid=0, pVidForce=GT_TRUE
*                                      GT_FALSE - disable loopback.
*                                      pVid and pVidForce restored.
* @param[in,out] pVidPtr                  - (pointer to) Port VID.
* @param[in,out] pVidForcePtr             - (pointer to) Port VID Force.
* @param[in,out] pVidPtr                  - (pointer to) Port VID.
* @param[in,out] pVidForcePtr             - (pointer to) Port VID Force.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
*/
GT_STATUS prvTgfEthCounterLoopbackEgrPort
(
    IN    GT_U8                          devNum,
    IN    GT_PHYSICAL_PORT_NUM           portNum,
    IN    GT_BOOL                        loopback,
    INOUT GT_U16                         *pVidPtr,
    INOUT GT_BOOL                        *pVidForcePtr
)
{
    GT_STATUS rc;
    CPSS_INTERFACE_INFO_STC  portInterface;

    CPSS_NULL_PTR_CHECK_MAC(pVidPtr);
    CPSS_NULL_PTR_CHECK_MAC(pVidForcePtr);

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = devNum;
    portInterface.devPort.portNum = portNum;

    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(
        &portInterface, loopback);
    if (GT_OK != rc)
    {
        return rc;
    }

#ifdef CHX_FAMILY

    if (loopback != GT_FALSE)
    {
        /* Enable Loopback */

        /* get current PVID */
        rc = cpssDxChBrgVlanPortVidGet(
            devNum, portNum, CPSS_DIRECTION_INGRESS_E, pVidPtr);
        if (GT_OK != rc)
        {
            return rc;
        }

        /* get PVID Force Enable for the port */
        rc = cpssDxChBrgVlanForcePvidEnableGet(
            devNum, portNum, pVidForcePtr);
        if (GT_OK != rc)
        {
            return rc;
        }

        /* set PVID = 0 for the port */
        rc = cpssDxChBrgVlanPortVidSet(
            devNum, portNum, CPSS_DIRECTION_INGRESS_E, 0);
        if (GT_OK != rc)
        {
            return rc;
        }

        /* set force PVID for all packets in the port */
        rc = cpssDxChBrgVlanForcePvidEnable(
            devNum, portNum, GT_TRUE);
        if (GT_OK != rc)
        {
            return rc;
        }
    }
    else
    {
        /* Disable Loopback */

        /* restore current PVID */
        rc = cpssDxChBrgVlanPortVidSet(
            devNum, portNum, CPSS_DIRECTION_INGRESS_E, *pVidPtr);
        if (GT_OK != rc)
        {
            return rc;
        }

        /* restore PVID Force Enable for the port */
        rc = cpssDxChBrgVlanForcePvidEnable(
            devNum, portNum, *pVidForcePtr);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    if (loopback != GT_FALSE)
    {
        /* Enable Loopback */

        /* get current PVID */
        rc = cpssExMxPmBrgVlanPortPvidGet(
            devNum, (GT_U8)portNum, pVidPtr);
        if (GT_OK != rc)
        {
            return rc;
        }

        /* get PVID Force Enable for the port */
        rc = cpssExMxPmBrgVlanPortForcePvidEnableGet(
            devNum, (GT_U8)portNum, pVidForcePtr);
        if (GT_OK != rc)
        {
            return rc;
        }

        /* set PVID = 0 for the port */
        rc = cpssExMxPmBrgVlanPortPvidSet(
            devNum, (GT_U8)portNum, 0);
        if (GT_OK != rc)
        {
            return rc;
        }

        /* set force PVID for all packets in the port */
        rc = cpssExMxPmBrgVlanPortForcePvidEnableSet(
            devNum, (GT_U8)portNum, GT_TRUE);
        if (GT_OK != rc)
        {
            return rc;
        }
    }
    else
    {
        /* Disable Loopback */

        /* restore current PVID */
        rc = cpssExMxPmBrgVlanPortPvidSet(
            devNum, (GT_U8)portNum, *pVidPtr);
        if (GT_OK != rc)
        {
            return rc;
        }

        /* restore PVID Force Enable for the port */
        rc = cpssExMxPmBrgVlanPortForcePvidEnableSet(
            devNum, (GT_U8)portNum, *pVidForcePtr);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(loopback);
    TGF_PARAM_NOT_USED(rc);
    TGF_PARAM_NOT_USED(portInterface);
    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)) */
}

/**
* @internal sendMultiDestinationTraffic function
* @endinternal
*
* @brief   Function sends packets.
*
* @param[in] devNum                   - the device to send traffic from.
* @param[in] portNum                  - the port to send traffic from.
* @param[in] burstNum                 - number of packets to send in the burst
* @param[in] trace                    - enable\disable packet tracing
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] vfdNum                   - VFD number
* @param[in] vfdArray[]               - VFD array to override packet info
*                                       None
*/
static GT_VOID sendMultiDestinationTraffic
(
    IN  GT_U8               devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  GT_U32              burstNum,
    IN  GT_BOOL             trace,
    IN TGF_PACKET_STC       *packetInfoPtr,
    IN GT_U8                vfdNum,
    IN TGF_VFD_INFO_STC     vfdArray[]
)
{
    GT_STATUS   rc;

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(devNum, packetInfoPtr, burstNum, vfdNum, vfdArray);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: devNum[%d],portNum[%d],vfdNum[%d]",
                                 devNum, burstNum, vfdNum, 0);


    if(prvTgfUseSleepAfterXCount == GT_TRUE)
    {
        /* we do 'Controlled learning' , so we need to allow AppDemo to process
           the AU messages , so do 'sleep' after x count of frames that so SA
           learning */

        if(debugSleepTime)
        {
            /* so every 1K/4 of packets sent , we do sleep */
            prvTgfSetTxSetup2Eth(_1K/4 , debugSleepTime);
        }
        else
        {
            /* so every 1K/4 of packets sent , we do sleep of 50 milliseconds */
            prvTgfSetTxSetup2Eth(_1K/4 , hugeBurstSleepTime);
        }

        /* 'Auto clear' the flag */
        prvTgfUseSleepAfterXCount = GT_FALSE;
    }

    /* set tracing */
    rc = tgfTrafficTracePacketByteSet(trace);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTracePacketByteSet: devNum[%d]", devNum);

    /* send Packet from requested port  */
    rc = prvTgfStartTransmitingEth(devNum, portNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: devNum[%d],portNum[%d]",
                                 devNum, portNum);

    /* enable tracing */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTracePacketByteSet: devNum[%d]", devNum);
}

/**
* @internal prvTgfCommonMultiDestinationTrafficTypeSend function
* @endinternal
*
* @brief   like prvTgfCommonMultiDestinationTrafficSend it :
*         Send multi-destination traffic , meaning from requested port:
*         Send BC (FF:FF:FF:FF:FF:FF) with burstNum incremental SA.
*         Send unregistered MC (01:02:03:04:05:06) with burstNum incremental SA.
*         Send unknown unicast (00:09:99:99:99:99) with burstNum incremental DA and static SA
*         Send unknown unicast (00:09:99:99:99:99) with burstNum static DA and incremental SA
*         BUT the packet itself (beside the SA,DA) is given by the caller.
* @param[in] devNum                   - the device to send traffic from.
* @param[in] portNum                  - the port to send traffic from.
* @param[in] burstNum                 - number of packets to send in the burst
* @param[in] trace                    - enable\disable packet tracing
* @param[in] packetPtr                - pointer to the packet
* @param[in] type                     - multi-destination  of sending
*                                       None
*/
void prvTgfCommonMultiDestinationTrafficTypeSend
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  GT_U32  burstNum,
    IN  GT_BOOL trace,
    IN  TGF_PACKET_STC *packetPtr,
    IN  PRV_TGF_MULTI_DESTINATION_TYPE_ENT  type
)
{
    TGF_VFD_INFO_STC        vfdInfo;
    TGF_PACKET_PART_STC     *l2PartPtr;
    TGF_PACKET_L2_STC       *origL2PartPtr;/* pointer to original L2 part */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) &vfdInfo, 0, sizeof(vfdInfo));

    if(packetPtr == NULL)
    {
        packetPtr = &multiDestination_prvTgfPacketInfo;
    }

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    utfPrintKeepAlive();

    l2PartPtr = &packetPtr->partsArray[0];
    /* save pointer to original L2 part */
    origL2PartPtr = l2PartPtr->partPtr;

    if(type == PRV_TGF_MULTI_DESTINATION_TYPE_ALL_E ||
       type == PRV_TGF_MULTI_DESTINATION_TYPE_BC_E )
    {
        /*a.        Send BC (FF:FF:FF:FF:FF:FF) with burstNum incremental SA.*/
        l2PartPtr->partPtr = multiDestination_prvTgfPacketL2Array[0];
        vfdInfo = multiDestination_prvTgfPacketL2Part_SA_incemental_vfdInfo;
        cpssOsMemCpy(vfdInfo.patternPtr, multiDestination_prvTgfPacketL2Part_BC.saMac, vfdInfo.cycleCount);

        /* send packets */
        sendMultiDestinationTraffic(devNum , portNum , burstNum , trace, packetPtr, 1, &vfdInfo);
    }


    if(type == PRV_TGF_MULTI_DESTINATION_TYPE_ALL_E ||
       type == PRV_TGF_MULTI_DESTINATION_TYPE_MC_E )
    {
        /*b.        Send unregistered MC (01:02:03:04:05:06) with burstNum incremental SA. */
        l2PartPtr->partPtr = multiDestination_prvTgfPacketL2Array[1];
        vfdInfo = multiDestination_prvTgfPacketL2Part_SA_incemental_vfdInfo;
        cpssOsMemCpy(vfdInfo.patternPtr, multiDestination_prvTgfPacketL2Part_MC.saMac, vfdInfo.cycleCount);

        /* send packets */
        sendMultiDestinationTraffic(devNum , portNum , burstNum , trace, packetPtr, 1, &vfdInfo);
    }

    if(type == PRV_TGF_MULTI_DESTINATION_TYPE_ALL_E ||
       type == PRV_TGF_MULTI_DESTINATION_TYPE_UNKNOWN_UC_DA_INCREMENT_E )
    {
        /*c.        Send unknown unicast (00:09:99:99:99:99) with burstNum incremental DA and static SA */
        l2PartPtr->partPtr = multiDestination_prvTgfPacketL2Array[2];
        vfdInfo = multiDestination_prvTgfPacketL2Part_DA_incemental_vfdInfo;
        cpssOsMemCpy(vfdInfo.patternPtr, multiDestination_prvTgfPacketL2Part_unk_UC.daMac, vfdInfo.cycleCount);

        /* send packets */
        sendMultiDestinationTraffic(devNum , portNum , burstNum , trace, packetPtr, 1, &vfdInfo);
    }

    if(type == PRV_TGF_MULTI_DESTINATION_TYPE_ALL_E ||
       type == PRV_TGF_MULTI_DESTINATION_TYPE_UNKNOWN_UC_SA_INCREMENT_E )
    {
        /*d.        Send unknown unicast (00:09:99:99:99:99) with burstNum static DA and incremental SA  */
        l2PartPtr->partPtr = multiDestination_prvTgfPacketL2Array[3];
        vfdInfo = multiDestination_prvTgfPacketL2Part_SA_incemental_vfdInfo;
        cpssOsMemCpy(vfdInfo.patternPtr, multiDestination_prvTgfPacketL2Part_unk_UC.saMac, vfdInfo.cycleCount);

        /* send packets */
        sendMultiDestinationTraffic(devNum , portNum , burstNum , trace, packetPtr, 1, &vfdInfo);
    }

    /* restore pointer to original L2 part */
    l2PartPtr->partPtr = origL2PartPtr;
    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfCommonMultiDestinationTrafficCounterCheck function
* @endinternal
*
* @brief   check that traffic counter match the needed value
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] expectedCounterValue     - expected counter value (counter of goodPktsSent.l[0])
*                                       None
*/
void prvTgfCommonMultiDestinationTrafficCounterCheck
(
    IN GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN GT_U32   expectedCounterValue
)
{
    GT_STATUS rc;
    CPSS_PORT_MAC_COUNTER_SET_STC  counters;

    rc = prvTgfReadPortCountersEth(devNum, portNum, GT_TRUE, &counters);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: devNum[%d],portNum[%d]",
                                 devNum, portNum);

    UTF_VERIFY_EQUAL2_STRING_MAC(expectedCounterValue,
                                 counters.goodPktsSent.l[0],
                                 "expected counter [%d] but got [%d]",
                                 expectedCounterValue,counters.goodPktsSent.l[0]);
    return ;
}

/**
* @internal prvTgfCommonIncrementalSaMacSend function
* @endinternal
*
* @brief   Send packets with incremental SA mac address (that start from specified mac
*         address)
*         -- this used for 'mac incremental learning'
* @param[in] devNum                   - the device to send traffic from.
* @param[in] portNum                  - the port to send traffic from.
* @param[in] saMacPtr                 - (pointer to) the base mac address (of source mac)
* @param[in] burstNum                 - number of packets to send in the burst
*                                       None
*/
void prvTgfCommonIncrementalSaMacSend
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  TGF_MAC_ADDR    *saMacPtr,
    IN  GT_U32  burstNum
)
{
    TGF_VFD_INFO_STC        vfdInfo;

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) &vfdInfo, 0, sizeof(vfdInfo));

    utfPrintKeepAlive();

    /* Send unknown unicast (00:09:99:99:99:99) with burstNum static DA and incremental SA  */
    multiDestination_prvTgfPacketPartArray[0].partPtr = multiDestination_prvTgfPacketL2Array[3];
    vfdInfo = multiDestination_prvTgfPacketL2Part_SA_incemental_vfdInfo;
    cpssOsMemCpy(vfdInfo.patternPtr, *saMacPtr, vfdInfo.cycleCount);

    /* when burst is large we need to allow the appDemo to learn the SA
       (due to controlled learning) , so we set flag to state the
       'Need sleep after X count'

       flag is 'Auto cleared'
    */
    prvTgfUseSleepAfterXCount = GT_TRUE;
    /* send packets */
    sendMultiDestinationTraffic(devNum , portNum , burstNum ,GT_TRUE, &multiDestination_prvTgfPacketInfo, 1, &vfdInfo);
}

/**
* @internal prvTgfCommonIncrementalSaMacByPortNumSend function
* @endinternal
*
* @brief   Send packets with incremental SA mac address .
*         this start from mac address of:  [00:00:port:port:port:port]
*         For example :
*         for port 0x22   --> [00:00:22:22:22:22]
*         and for port 0x6 --> [00:00:06:06:06:06]
*         -- this used for 'mac incremental learning'
* @param[in] devNum                   - the device to send traffic from.
* @param[in] portNum                  - the port to send traffic from.
* @param[in] burstNum                 - number of packets to send in the burst
*                                       None
*/
void prvTgfCommonIncrementalSaMacByPortNumSend
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  GT_U32  burstNum
)
{
    TGF_MAC_ADDR    saMac;

    /* build the source mac address */
    saMac[0] = 0;
    saMac[1] = 0;
    saMac[2] = (GT_U8)portNum;
    saMac[3] = (GT_U8)portNum;
    saMac[4] = (GT_U8)portNum;
    saMac[5] = (GT_U8)portNum;

    /* call to send by staring from specific mac address */
    prvTgfCommonIncrementalSaMacSend(devNum,portNum,&saMac,burstNum);
}

/**
* @internal prvTgfCommonIncrementalDaMacSend function
* @endinternal
*
* @brief   Send packets with incremental DA mac address (that start from specified mac
*         address) and static SA mac address
*         -- this used for checking the already learned mac addresses
* @param[in] devNum                   - the device to send traffic from.
* @param[in] portNum                  - the port to send traffic from.
* @param[in] saMacPtr                 - (pointer to) the SA mac address
* @param[in] daMacPtr                 - (pointer to) the DA base mac address
* @param[in] burstNum                 - number of packets to send in the burst
*                                       None
*/
void prvTgfCommonIncrementalDaMacSend
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  TGF_MAC_ADDR    *saMacPtr,
    IN  TGF_MAC_ADDR    *daMacPtr,
    IN  GT_U32  burstNum
)
{
    TGF_VFD_INFO_STC        vfdInfo[2];

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdInfo, 0, sizeof(vfdInfo));

    utfPrintKeepAlive();

    /*  Send unicast with incremental DA and static SA */
    multiDestination_prvTgfPacketPartArray[0].partPtr = multiDestination_prvTgfPacketL2Array[2];
    vfdInfo[0] = multiDestination_prvTgfPacketL2Part_DA_incemental_vfdInfo;
    cpssOsMemCpy(vfdInfo[0].patternPtr, *daMacPtr, vfdInfo[0].cycleCount);
    vfdInfo[1] = multiDestination_prvTgfPacketL2Part_SA_static_vfdInfo;
    cpssOsMemCpy(vfdInfo[1].patternPtr, *saMacPtr, vfdInfo[1].cycleCount);

    /* when burst is large we need to allow the simulation to process Tx frames */
    prvTgfUseSleepAfterXCount = GT_TRUE;

    /* send packets */
    sendMultiDestinationTraffic(devNum , portNum , burstNum , GT_TRUE, &multiDestination_prvTgfPacketInfo, 2, vfdInfo);
}

/**
* @internal prvTgfCommonIncrementalDaMacByPortNumSend function
* @endinternal
*
* @brief   Send packets with incremental DA mac address (according to dstPortNum)
*         and static SA mac address (according to portNum)
*         DA start from mac address of:  [00:00:dstPortNum:dstPortNum:dstPortNum:dstPortNum]
*         For example :
*         for port 0x22   --> [00:00:22:22:22:22]
*         and for port 0x6 --> [00:00:06:06:06:06]
*         SA start from mac address of:  [00:00:portNum:portNum:portNum:portNum]
*         For example :
*         for port 0x33   --> [00:00:33:33:33:33]
*         and for port 0x8 --> [00:00:08:08:08:08]
*         -- this used for checking the already learned mac addresses
* @param[in] devNum                   - the device to send traffic from.
* @param[in] portNum                  - the port to send traffic from.
*                                      this port number also used for SA mac address
* @param[in] dstPortNum               - the alleged port number to receive the traffic
*                                      this port number used for base DA mac address
*                                      daMacPtr     - (pointer to) the DA base mac address
* @param[in] burstNum                 - number of packets to send in the burst
*                                       None
*/
void prvTgfCommonIncrementalDaMacByPortNumSend
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  GT_U32   dstPortNum,
    IN  GT_U32  burstNum
)
{
    TGF_MAC_ADDR    saMac;
    TGF_MAC_ADDR    daMac;

    /* build the source mac address */
    saMac[0] = 0;
    saMac[1] = 0;
    saMac[2] = (GT_U8)portNum;
    saMac[3] = (GT_U8)portNum;
    saMac[4] = (GT_U8)portNum;
    saMac[5] = (GT_U8)portNum;

    /* build the source mac address */
    daMac[0] = 0;
    daMac[1] = 0;
    daMac[2] = (GT_U8)dstPortNum;
    daMac[3] = (GT_U8)dstPortNum;
    daMac[4] = (GT_U8)dstPortNum;
    daMac[5] = (GT_U8)dstPortNum;

    /* call to send by staring from specific mac addresses */
    prvTgfCommonIncrementalDaMacSend(devNum,portNum,&saMac,&daMac,burstNum);
}

/**
* @internal prvTgfCommonDebugSleepTime function
* @endinternal
*
* @brief   set debug time (milliseconds) for sleep after every burst of 512 packets
*
* @param[in] time                     -  in milliseconds
*                                       None
*/
GT_STATUS prvTgfCommonDebugSleepTime
(
    IN  GT_U32   time
)
{
    debugSleepTime = time;
    return GT_OK;
}


/**
* @internal prvTgfCommonMultiDestinationTrafficSend function
* @endinternal
*
* @brief   Send multi-destination traffic , meaning from requested port:
*         Send BC (FF:FF:FF:FF:FF:FF) with burstNum incremental SA.
*         Send unregistered MC (01:02:03:04:05:06) with burstNum incremental SA.
*         Send unknown unicast (00:09:99:99:99:99) with burstNum incremental DA and static SA
*         Send unknown unicast (00:09:99:99:99:99) with burstNum static DA and incremental SA
* @param[in] devNum                   - the device to send traffic from.
* @param[in] portNum                  - the port to send traffic from.
* @param[in] burstNum                 - number of packets to send in the burst
* @param[in] trace                    - enable\disable packet tracing
*                                       None
*/
void prvTgfCommonMultiDestinationTrafficSend
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  GT_U32  burstNum,
    IN  GT_BOOL trace
)
{
    prvTgfCommonMultiDestinationTrafficTypeSend(
            devNum,portNum,burstNum,trace,&multiDestination_prvTgfPacketInfo,
            PRV_TGF_MULTI_DESTINATION_TYPE_ALL_E);
}

/**
* @internal prvTgfTransmitPacketsWithCapture function
* @endinternal
*
* @brief   None
*
* @param[in] inDevNum                 - ingress device number
* @param[in] inPortNum                - ingress port number
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               - number of frames
* @param[in] numVfd                   - number of VFDs
* @param[in] vfdArray[]               - (pointer to) array of VFDs
* @param[in] outDevNum                - egress device number
* @param[in] outPortNum               - egress port number
* @param[in] captureMode              - packet capture mode
* @param[in] captureOnTime            - time the capture will be enabe (in milliseconds)
*                                       None
*/
GT_STATUS prvTgfTransmitPacketsWithCapture
(
    IN GT_U8                inDevNum,
    IN GT_U32                inPortNum,
    IN TGF_PACKET_STC       *packetInfoPtr,
    IN GT_U32               burstCount,
    IN GT_U32               numVfd,
    IN TGF_VFD_INFO_STC     vfdArray[],
    IN GT_U8                outDevNum,
    IN GT_U32                outPortNum,
    IN TGF_CAPTURE_MODE_ENT captureMode,
    IN GT_U32               captureOnTime
)
{
    GT_STATUS               rc = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    /* setup receive portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = outDevNum;
    portInterface.devPort.portNum = outPortNum;

    /* enable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, captureMode, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "tgftrafficgeneratorporttxethcaptureset");


    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(
        inDevNum, packetInfoPtr,
        burstCount, numVfd, vfdArray);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(inDevNum, inPortNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    cpssOsTimerWkAfter(captureOnTime);

    /* enable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, captureMode, GT_FALSE);
    PRV_UTF_VERIFY_GT_OK(rc, "tgftrafficgeneratorporttxethcaptureset");

    return rc;
}

/**
* @internal prvTgfTransmitPackets function
* @endinternal
*
* @brief   None
*
* @param[in] inDevNum                 - ingress device number
* @param[in] inPortNum                - ingress port number
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               - number of frames
* @param[in] numVfd                   - number of VFDs
* @param[in] vfdArray[]               - (pointer to) array of VFDs
*                                       None
*/
GT_STATUS prvTgfTransmitPackets
(
    IN GT_U8                inDevNum,
    IN GT_U32                inPortNum,
    IN TGF_PACKET_STC       *packetInfoPtr,
    IN GT_U32               burstCount,
    IN GT_U32               numVfd,
    IN TGF_VFD_INFO_STC     vfdArray[]
)
{
    GT_STATUS               rc = GT_OK;

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(
        inDevNum, packetInfoPtr,
        burstCount, numVfd, vfdArray);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(inDevNum, inPortNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    return rc;
}

/**
* @internal prvTgfTransmitPacketsWithDelay function
* @endinternal
*
* @brief   None
*
* @param[in] inDevNum                 - ingress device number
* @param[in] inPortNum                - ingress port number
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               - number of frames
* @param[in] numVfd                   - number of VFDs
* @param[in] vfdArray[]               - (pointer to) array of VFDs
*                                       None
* @param[in] sleepTime                - Delay between each packet
*/
GT_STATUS prvTgfTransmitPacketsWithDelay
(
    IN GT_U8                inDevNum,
    IN GT_U32                inPortNum,
    IN TGF_PACKET_STC       *packetInfoPtr,
    IN GT_U32               burstCount,
    IN GT_U32               numVfd,
    IN TGF_VFD_INFO_STC     vfdArray[],
    IN GT_U32               sleepTime
)
{
    GT_STATUS               rc = GT_OK;

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(
        inDevNum, packetInfoPtr,
        burstCount, numVfd, vfdArray);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    rc = prvTgfSetTxSetup2Eth(1, sleepTime);

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(inDevNum, inPortNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    /* remove Tx delay */
    rc = prvTgfSetTxSetup2Eth(RESET_VALUE_CNS, RESET_VALUE_CNS);

    return rc;
}

/**
* @internal prvTgfDefPortsArrayFromListSet function
* @endinternal
*
* @brief   Set predefined number of ports and port's array in PreInit or PostInit
*         phase
* @param[in] numOfPorts               - number of ports
* @param[in] portNum                  - list of ports
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_SIZE              - wrong array size
* @retval GT_BAD_PTR               - wrong pointer
*/
GT_STATUS prvTgfDefPortsArrayFromListSet
(
    IN GT_U8      numOfPorts,
    IN GT_U32     portNum,
    IN ...
)
{
    CPSS_PORTS_BMP_STC  portBmp    = {{0}};
    GT_U32               tmpPortNum = 0;
    GT_U8               portIter   = 0;
    va_list             argParam;


    /* init variable argument list */
    va_start(argParam, portNum);

    /* check number of ports to add */
    if (numOfPorts > PRV_TGF_MAX_PORTS_NUM_CNS)
    {
        /* free VA list pointer */
        va_end(argParam);
        return GT_BAD_SIZE;
    }

    /* set number of ports and first port in list */
    prvTgfPortsNum = numOfPorts;
    tmpPortNum     = portNum;

    /* clear port bitmap */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBmp);

    /* check ports in array */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* in PreInit stage cannot check dev\ports so wait till initialization */
        if (GT_FALSE == prvTgfCommonPreInitStage)
        {
            /* check that ports is valid */
            if(prvCpssDxChPortRemotePortCheck(prvTgfDevNum,tmpPortNum))
            {
                /* allow remote ports (AC3X)*/
            }
            else
            if ((tmpPortNum) != CPSS_CPU_PORT_NUM_CNS)
            {
                if((tmpPortNum >= PRV_CPSS_PP_MAC(prvTgfDevNum)->numOfPorts)
                   || !PRV_CPSS_PHY_PORT_IS_EXIST_MAC(prvTgfDevNum, tmpPortNum))
                {
                    /* free VA list pointer */
                    va_end(argParam);
                    return GT_BAD_PARAM;
                }
            }
        }

        /* check that ports is unique */
        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portBmp, tmpPortNum))
        {
            /* free VA list pointer */
            va_end(argParam);
            return GT_BAD_PARAM;
        }
        else
        {
            CPSS_PORTS_BMP_PORT_SET_MAC(&portBmp, tmpPortNum);
        }

        /* add new port number */
        prvTgfPortsArray[portIter] = tmpPortNum;

        /* get next port number from argument list */
        tmpPortNum = va_arg(argParam, GT_U32);
    }

    /* free VA list pointer */
    va_end(argParam);

    /* in PreInit stage cannot check dev\ports so wait till initialization */
    if (GT_FALSE == prvTgfCommonPreInitStage)
    {
        /* set not used ports with illegal values */
        setInvalidPortsInArr(prvTgfPortsArray,prvTgfPortsNum,(PRV_TGF_MAX_PORTS_NUM_CNS - prvTgfPortsNum));
    }

    /* set flag to check ports in PostInit stage */
    prvTgfPortsArrayPreInitSet = prvTgfCommonPreInitStage;

    return GT_OK;
}

/**
* @internal prvTgfDefPortsArrayModeSet function
* @endinternal
*
* @brief   Set default arrays of test ports according to specific mode.
*
* @param[in] portMode                 - port mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong input parameter
*/
GT_STATUS prvTgfDefPortsArrayModeSet
(
    IN PRV_TGF_DEVICE_PORT_MODE_ENT     portMode
)
{
    GT_U32   *db64Ptr;  /* pointer to data for default update of 64 ports mode */
    GT_U32   *db128Ptr; /* pointer to data for default update of 128 ports mode */
    GT_U32   portNum, localPort;

    switch (portMode)
    {
        case PRV_TGF_DEVICE_PORT_MODE_XLG_E:
            db64Ptr = prvTgfDef40GPortsArray_64;
            db128Ptr = prvTgfDef40GPortsArray_128;
            for (portNum = 0; portNum < 128; portNum++)
            {
                localPort = portNum & 0xF;
                if ((localPort == 0) || (localPort == 4) || (localPort == 8) || (localPort == 9))
                {
                    /* only local ports 0,4,8,9 are in use */
                    continue;
                }

                /* remove port from FWS tests */
                (void)prvTgfPortFWSSkipPort(portNum, 1);
            }

            /* need to use more packets for FWS test in XLG mode */
            (void)prvTgfFWSTraffCount(9,8);

            break;
        /* nothing to update */
        default: return GT_OK;
    }

    /* 64 port mode */
    cpssOsMemCpy(&(prvTgfDefPortsArray[PRV_TGF_64_PORT_DEVICE_TYPE_E][0]), db64Ptr, sizeof(prvTgfDefPortsArray[0]));

    /* 128 port mode */
    cpssOsMemCpy(&(prvTgfDefPortsArray[PRV_TGF_128_PORT_DEVICE_TYPE_E][0]), db128Ptr, sizeof(prvTgfDefPortsArray[0]));

    prvTgfDevicePortMode = portMode;
    return GT_OK;
}

/**
* @internal prvTgfDefPortsArraySet function
* @endinternal
*
* @brief   Set predefined number of ports and port's array
*
* @param[in] portsArray[]             - array of ports
* @param[in] arraySize                - number of ports
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_SIZE              - wrong array size
* @retval GT_BAD_PTR               - wrong pointer
*
* @note Call this function each time when needs to override default ports array
*       by specific values
*
*/
GT_STATUS prvTgfDefPortsArraySet
(
    IN GT_U32    portsArray[],
    IN GT_U8    arraySize
)
{
    GT_U32              portIter = 0;
    CPSS_PORTS_BMP_STC  portBmp;


    /* check array */
    CPSS_NULL_PTR_CHECK_MAC(portsArray);

    /* check array size */
    if (arraySize > PRV_TGF_MAX_PORTS_NUM_CNS)
    {
        return GT_BAD_SIZE;
    }

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBmp);

    /* check ports in array */
    for (portIter = 0; portIter < arraySize; portIter++)
    {
        /* check that ports is valid */
        PRV_CPSS_DXCH_ENHANCED_PHY_PORT_OR_CPU_PORT_CHECK_MAC(prvTgfDevNum, portsArray[portIter]);

        /* check that ports is unique */
        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portBmp,portsArray[portIter]))
        {
            return GT_BAD_PARAM;
        }
        else
        {
            CPSS_PORTS_BMP_PORT_SET_MAC(&portBmp,portsArray[portIter]);
        }
    }

    /* set number of ports */
    prvTgfPortsNum = arraySize;

    /* set ports array */
    cpssOsMemCpy(prvTgfPortsArray, portsArray, sizeof(portsArray[0]) * arraySize);

    /* set not used ports with illegal values */
    setInvalidPortsInArr(prvTgfPortsArray,prvTgfPortsNum,(PRV_TGF_MAX_PORTS_NUM_CNS - prvTgfPortsNum));

    return GT_OK;
}

/**
* @internal prvTgfDefPortsArrayPortToPortIndex function
* @endinternal
*
* @brief   Search for port in ports array and return the index of this port in the
*         array.
* @param[in] port                     -  to search
*
* @param[out] portIndexPtr             - points to index of the port in the array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - wrong pointer
* @retval GT_NOT_FOUND             - port is not part of ports array
*/
GT_STATUS prvTgfDefPortsArrayPortToPortIndex
(
    IN  GT_U32   port,
    OUT GT_U8   *portIndexPtr
)
{
    GT_U8       i;

    CPSS_NULL_PTR_CHECK_MAC(portIndexPtr);

    for (i = 0 ; i < prvTgfPortsNum ; i++)
    {
        if (prvTgfPortsArray[i] == port)
        {
            *portIndexPtr = i;
            return GT_OK;
        }
    }

    /* port wasn't foudn in the array */
    return GT_NOT_FOUND;
}

/**
* @internal prvTgfTestsPortRangeSelectPreInit function
* @endinternal
*
* @brief   force pre init the device type for ports array to be used in the UTF.
*         allow to set tests with ports : 0..59 or 0..123 for a device with 124 ports.
*         for example : by default AslanB will test ports 0..59 BUT by call from
*         the terminal we can make it to use ports 0..123
*
* @retval GT_OK                    - in case of success
*/
GT_STATUS prvTgfTestsPortRangeSelectPreInit
(
    IN GT_U32   numOfPorts
)
{
    GT_U32      ii;

    for(ii = 0 ; ii < PRV_TGF_DEVICE_TYPE_COUNT_E; ii++)
    {
        if(numOfPorts <= tgfDeviceTypeNumOfPorts[ii])
        {
            /* the device is in the needed category */
            break;
        }
    }
    tgfDeviceTypeRunModePreInit = ii;

    return GT_OK;
}

/**
* @internal prvTgfTestsPortRangeSelect function
* @endinternal
*
* @brief   force device type for ports array to be used in the UTF.
*         allow to set tests with ports : 0..59 or 0..123 for a device with 124 ports.
*         for example : by default AslanB will test ports 0..59 BUT by call from
*         the terminal we can make it to use ports 0..123
*/
GT_VOID prvTgfTestsPortRangeSelect
(
    IN GT_U32   numOfPorts
)
{
    GT_U32                ii;
    GT_U32                jj = 2;
    GT_PHYSICAL_PORT_NUM  portNum;

    /* check if not using pre-init setting , then look for the proper set of ports */
    if (tgfDeviceTypeRunModePreInit == PRV_TGF_DEVICE_TYPE_COUNT_E )
    {
        for(ii = 0 ; ii < PRV_TGF_DEVICE_TYPE_COUNT_E; ii++)
        {
            if(tgfLoopBackCt10And40Enable == GT_TRUE)
            {
                /* Clear uplink port database. */
                CPSS_PORTS_BMP_PORT_CLEAR_MAC(&appDemoPpConfigList[ii].uplinkPortGroupPortsBmp, 73);
                for(portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
                {
                    if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&appDemoPpConfigList[ii].uplinkPortGroupPortsBmp, portNum))
                    {
                        prvTgfDefPortsArray[ii + 1][jj] = portNum;
                        jj++;
                    }
                    if(jj > 3)
                    {
                        break;
                    }

                }

            }

            if(numOfPorts <= tgfDeviceTypeNumOfPorts[ii])
            {
                /* the device is in the needed category */
                break;
            }
        }
    }
    else
    {
        /* use the pre init value */
        ii = tgfDeviceTypeRunModePreInit;
        /* allow this function to be called again (from terminal) and to set specific settings */
        tgfDeviceTypeRunModePreInit = PRV_TGF_DEVICE_TYPE_COUNT_E;
        if(tgfLoopBackCt10And40Enable == GT_TRUE)
        {
            CPSS_PORTS_BMP_PORT_CLEAR_MAC(&appDemoPpConfigList[ii].uplinkPortGroupPortsBmp, 73);
            for(portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
            {
                if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&appDemoPpConfigList[ii].uplinkPortGroupPortsBmp, portNum))
                {
                    prvTgfDefPortsArray[ii][jj] = portNum;
                    jj++;
                }
                if(jj > 3)
                {
                    break;
                }

            }
        }
    }

    /* update default number of ports */
    prvTgfPortsNum = prvTgfDefPortsArraySize[ii];

    /* update default port array */
    cpssOsMemCpy(prvTgfPortsArray, prvTgfDefPortsArray[ii], sizeof(prvTgfPortsArray));


    /* first attempt of BC2 ports fixing */
    for (jj = 0; jj < prvTgfPortsNum; jj++)
    {
        while(prvTgfPortsArray[jj] >= PRV_CPSS_PP_MAC(prvTgfDevNum)->numOfPorts)
        {
            /* support for bobcat2 to fit port 58 to be 54 (with in 0..55) */
            prvTgfPortsArray[jj] -= 2;
        }
    }

    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E ||
       PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E ||
       PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {/* see also function utfIsMustUseOrigPorts(IN GT_U8  devNum)
        use the 12 ports that the AppDemo initialize */
        /* the bobcat3 was initialized in AppDemo with only 12 ports ,
           because the GM supports only single data path (12 ports)
           so we are not able to choose any port we want */
#ifdef GM_USED
        prvTgfPortsArray[0] =  0;
        prvTgfPortsArray[1] = 18;
        prvTgfPortsArray[2] = 36;
        prvTgfPortsArray[3] = 58;
        prvTgfPortsArray[4] = 54;
        prvTgfPortsArray[5] =  1;
        prvTgfPortsArray[6] =  2;
        prvTgfPortsArray[7] =  3;
        prvTgfPortsArray[8] = 64;
        prvTgfPortsArray[9] = 65;
        prvTgfPortsArray[10]= 80;
        prvTgfPortsArray[11]= 79;

        if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            /* swap index 0,3 (to be like in WM and BM - for Falcon) */
            prvTgfPortsArray[0] = 58;
            prvTgfPortsArray[3] =  0;
        }

#else /* not GM */
        prvTgfPortsArray[0] =  0;
        prvTgfPortsArray[1] = 12;
        prvTgfPortsArray[2] = 36;
        prvTgfPortsArray[3] = 58;
        prvTgfPortsArray[4] = 54;
        prvTgfPortsArray[5] = 16;
        prvTgfPortsArray[6] = 20;
        prvTgfPortsArray[7] = 30;
        if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            /* swap index 0,3 */
            prvTgfPortsArray[0] = 58;
            prvTgfPortsArray[3] =  0;

            /* Port numbers higher than 69 are not in the default port map */
            prvTgfPortsArray[8] = 67;
            prvTgfPortsArray[9] = 68;
        }
        else
        {
            prvTgfPortsArray[8] = 69;
            prvTgfPortsArray[9] = 70;
        }
        prvTgfPortsArray[10]= 65;
        prvTgfPortsArray[11]= 64;
#endif

        if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E &&
           PRV_CPSS_PP_MAC(prvTgfDevNum)->multiPipe.numOfTiles >= 2)
        {
            /* available ports are : 0..119 (121..127 for CPU SDMAs) */
            static GT_U32   falconMultiTilePorts[12] = {
                119, 18,111, 65,
                 64, 71, 92, 58,
                118,109, 97, 83
            };

            for(jj = 0 ; jj < 12 ; jj++)
            {
                prvTgfPortsArray[jj] = falconMultiTilePorts[jj];
            }

        }

        prvTgfPortsNum = 12; /* for the setInvalidPortsInArr to come after the 12 ports */

#ifndef ASIC_SIMULATION
        /* increase WS burst to support 100G ports */
        prvTgfWsBurstSet(250);
#endif

        if(prvUtfOffsetFrom0ForUsedPortsGet())
        {
            GT_U32  offset = prvUtfOffsetFrom0ForUsedPortsGet();
            for(jj = 0 ; jj < prvTgfPortsNum ; jj++)
            {
                prvTgfPortsArray[jj] += offset;/* match the values that the cpssInitSystem set */
            }

        }
        else
        if(GT_TRUE == prvUtfIsDoublePhysicalPortsModeUsed())
        {
            for(jj = 0 ; jj < prvTgfPortsNum ; jj++)
            {
                prvTgfPortsArray[jj] += 256;/* match the values that the cpssInitSystem set */
            }
        }

        if (prvUtfIsGmCompilation())
        {
            GT_U8   devNum = prvTgfDevNum;
            GT_STATUS   rc;
            /* troubles with interrupts with GM */
            for(jj = 0 ; jj < prvTgfPortsNum ; jj++)
            {
                rc = cpssDxChBrgEgrFltPortLinkEnableSet(devNum,
                     prvTgfPortsArray[jj],
                     CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E  /*don't filter*/);
                if(rc != GT_OK)
                {
                    PRV_UTF_LOG1_MAC("UTF: error : cpssDxChBrgEgrFltPortLinkEnableSet:  port [%d] \n" ,
                                     prvTgfPortsArray[jj]);

                    return ;
                }
            }
        }

        if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
        {
             /* exclued 100G ports 0, 8, 16 and other ports those fails in 100G lanes */
             prvTgfPortFWSSkipPort(0,GT_TRUE);
             prvTgfPortFWSSkipPort(1,GT_TRUE);
             prvTgfPortFWSSkipPort(2,GT_TRUE);
             prvTgfPortFWSSkipPort(3,GT_TRUE);

             /* allocated by 100G port 4 */
             prvTgfPortFWSSkipPort(5,GT_TRUE);
             prvTgfPortFWSSkipPort(6,GT_TRUE);
             prvTgfPortFWSSkipPort(7,GT_TRUE);

             /* allocated by 100G port 8 */
             prvTgfPortFWSSkipPort(8,GT_TRUE);
             prvTgfPortFWSSkipPort(9,GT_TRUE);
             prvTgfPortFWSSkipPort(10,GT_TRUE);
             prvTgfPortFWSSkipPort(11,GT_TRUE);

             /* allocated by 100G port 12 */
             prvTgfPortFWSSkipPort(13,GT_TRUE);
             prvTgfPortFWSSkipPort(14,GT_TRUE);
             prvTgfPortFWSSkipPort(15,GT_TRUE);

             /* allocated by 100G port 16 */
             prvTgfPortFWSSkipPort(16,GT_TRUE);
             prvTgfPortFWSSkipPort(17,GT_TRUE);
             prvTgfPortFWSSkipPort(18,GT_TRUE);
             prvTgfPortFWSSkipPort(19,GT_TRUE);

             prvTgfPortFWSSkipPort(34,GT_TRUE);

             prvTgfPortFWSSkipPort(76,GT_TRUE);
             prvTgfPortFWSSkipPort(77,GT_TRUE);
             prvTgfPortFWSSkipPort(78,GT_TRUE);
        }
        else if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
        {
            /* Aldrin2's BW is 403.2 Gpbs for 600 Mhz Core Clock,
               use 100G ports 4, 12 and 40 ports of 10G - 24-33, 48-56, CPU port.
               Aldrin2 has ~ 30% speedup for 10G ports in MAC loopback.
               Remove 3 ports from 40 to avoid big oversubscription. */

             /* exclude 100G ports 0, 8, 16 and other ports those fails in 100G lanes */
             prvTgfPortFWSSkipPort(0,GT_TRUE);
             prvTgfPortFWSSkipPort(1,GT_TRUE);
             prvTgfPortFWSSkipPort(2,GT_TRUE);
             prvTgfPortFWSSkipPort(3,GT_TRUE);

             /* allocated by 100G port 4 */
             prvTgfPortFWSSkipPort(5,GT_TRUE);
             prvTgfPortFWSSkipPort(6,GT_TRUE);
             prvTgfPortFWSSkipPort(7,GT_TRUE);

             /* allocated by 100G port 8 */
             prvTgfPortFWSSkipPort(8,GT_TRUE);
             prvTgfPortFWSSkipPort(9,GT_TRUE);
             prvTgfPortFWSSkipPort(10,GT_TRUE);
             prvTgfPortFWSSkipPort(11,GT_TRUE);

             /* allocated by 100G port 12 */
             prvTgfPortFWSSkipPort(13,GT_TRUE);
             prvTgfPortFWSSkipPort(14,GT_TRUE);
             prvTgfPortFWSSkipPort(15,GT_TRUE);

             /* allocated by 100G port 16 */
             prvTgfPortFWSSkipPort(16,GT_TRUE);
             prvTgfPortFWSSkipPort(17,GT_TRUE);
             prvTgfPortFWSSkipPort(18,GT_TRUE);
             prvTgfPortFWSSkipPort(19,GT_TRUE);

             /* allocated by 100G port 20 */
             prvTgfPortFWSSkipPort(20,GT_TRUE);
             prvTgfPortFWSSkipPort(21,GT_TRUE);
             prvTgfPortFWSSkipPort(22,GT_TRUE);
             prvTgfPortFWSSkipPort(23,GT_TRUE);

             /* skip ports 34-47*/
             prvTgfPortFWSSkipPort(34,GT_TRUE);
             prvTgfPortFWSSkipPort(35,GT_TRUE);
             prvTgfPortFWSSkipPort(36,GT_TRUE);
             prvTgfPortFWSSkipPort(37,GT_TRUE);
             prvTgfPortFWSSkipPort(38,GT_TRUE);
             prvTgfPortFWSSkipPort(39,GT_TRUE);
             prvTgfPortFWSSkipPort(40,GT_TRUE);
             prvTgfPortFWSSkipPort(41,GT_TRUE);
             prvTgfPortFWSSkipPort(42,GT_TRUE);
             prvTgfPortFWSSkipPort(43,GT_TRUE);
             prvTgfPortFWSSkipPort(44,GT_TRUE);
             prvTgfPortFWSSkipPort(45,GT_TRUE);
             prvTgfPortFWSSkipPort(46,GT_TRUE);
             prvTgfPortFWSSkipPort(47,GT_TRUE);

             /* skip ports 57-77*/
             prvTgfPortFWSSkipPort(57,GT_TRUE);
             prvTgfPortFWSSkipPort(58,GT_TRUE);
             prvTgfPortFWSSkipPort(64,GT_TRUE);
             prvTgfPortFWSSkipPort(65,GT_TRUE);
             prvTgfPortFWSSkipPort(66,GT_TRUE);
             prvTgfPortFWSSkipPort(67,GT_TRUE);
             prvTgfPortFWSSkipPort(68,GT_TRUE);
             prvTgfPortFWSSkipPort(69,GT_TRUE);
             prvTgfPortFWSSkipPort(70,GT_TRUE);
             prvTgfPortFWSSkipPort(71,GT_TRUE);
             prvTgfPortFWSSkipPort(72,GT_TRUE);
             prvTgfPortFWSSkipPort(73,GT_TRUE);
             prvTgfPortFWSSkipPort(74,GT_TRUE);
             prvTgfPortFWSSkipPort(75,GT_TRUE);
             prvTgfPortFWSSkipPort(76,GT_TRUE);
             prvTgfPortFWSSkipPort(77,GT_TRUE);

             /* skip 3 ports more */
             prvTgfPortFWSSkipPort(26,GT_TRUE);
             prvTgfPortFWSSkipPort(50,GT_TRUE);
             prvTgfPortFWSSkipPort(55,GT_TRUE);

             /* increase number of packets to generate stable FWS traffic on 100G ports */
#ifndef ASIC_SIMULATION
             prvTgfFWSTraffCount(40,8);
#endif
        }
    }
    else if((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
            (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
    {
        if (GT_TRUE == prvTgfXcat3xExists())
        {
            if((PRV_CPSS_PP_MAC(1) != NULL) &&
               (PRV_CPSS_PP_MAC(1)->devFamily) == CPSS_PX_FAMILY_PIPE_E)
            {
                prvTgfPortsArray[0]  = 4; /* remote port 1G on 88e1690 #3 local port 8 */
                prvTgfPortsArray[1]  = 5;  /* local 40G                                 */
                prvTgfPortsArray[2]  = 6; /* remote port 1G on 88e1690 #1 local port 6 */
                prvTgfPortsArray[3]  = 7; /* remote port 1G on 88e1690 #2 local port 6 */
                prvTgfPortsArray[4]  = 8; /* remote port 1G on 88e1690 #1 local port 1 */
                prvTgfPortsArray[5]  = 9; /* local 10G                                 */
                prvTgfPortsArray[6]  = 10; /* remote port 1G on 88e1690 #3 local port 7 */
                prvTgfPortsArray[7]  = 11;  /* local 40G                                 */
                prvTgfPortsArray[8]  = 12; /* remote port 1G on 88e1690 #1 local port 5 */
                prvTgfPortsArray[9]  = 13; /* remote port 1G on 88e1690 #2 local port 7 */
                prvTgfPortsArray[10] = 14; /* remote port 1G on 88e1690 #1 local port 4 */
                prvTgfPortsArray[11] = 15; /* local 10G                                 */
            }
            else
            {
                prvTgfPortsArray[0]  = 35; /* remote port 1G on 88e1690 #3 local port 8 */
                prvTgfPortsArray[1]  = 4;  /* local 40G                                 */
                prvTgfPortsArray[2]  = 17; /* remote port 1G on 88e1690 #1 local port 6 */
                prvTgfPortsArray[3]  = 25; /* remote port 1G on 88e1690 #2 local port 6 */
                prvTgfPortsArray[4]  = 12; /* remote port 1G on 88e1690 #1 local port 1 */
                prvTgfPortsArray[5]  = 11; /* local 10G                                 */
                prvTgfPortsArray[6]  = 34; /* remote port 1G on 88e1690 #3 local port 7 */
                prvTgfPortsArray[7]  = 0;  /* local 40G                                 */
                prvTgfPortsArray[8]  = 16; /* remote port 1G on 88e1690 #1 local port 5 */
                prvTgfPortsArray[9]  = 26; /* remote port 1G on 88e1690 #2 local port 7 */
                prvTgfPortsArray[10] = 15; /* remote port 1G on 88e1690 #1 local port 4 */
                prvTgfPortsArray[11] = 10; /* local 10G                                 */
            }


            prvTgfPortsNum = 12;

            utfSetMustUseOrigPorts(GT_TRUE);

            /* exclude cascade ports from FWS tests */
            prvTgfPortFWSSkipPort(72,GT_TRUE);
            prvTgfPortFWSSkipPort(73,GT_TRUE);
            prvTgfPortFWSSkipPort(74,GT_TRUE);
            prvTgfPortFWSSkipPort(75,GT_TRUE);
            prvTgfPortFWSSkipPort(76,GT_TRUE);
            prvTgfPortFWSSkipPort(77,GT_TRUE);

            /* exclude two 10G ports to avoid oversubscription over 161G */
            prvTgfPortFWSSkipPort(10,GT_TRUE);
            prvTgfPortFWSSkipPort(11,GT_TRUE);

        }
        else
        {
            prvTgfPortsArray[0] =  0;
            prvTgfPortsArray[1] =  8;
            prvTgfPortsArray[2] = 12;
            prvTgfPortsArray[3] = 30;

            /* Aldrin have 2:1 oversubscription.
               Disable half of ports from FWS tests. */
            prvTgfPortFWSSkipPort(1,GT_TRUE);
            prvTgfPortFWSSkipPort(2,GT_TRUE);
            prvTgfPortFWSSkipPort(5,GT_TRUE);
            prvTgfPortFWSSkipPort(8,GT_TRUE);
            prvTgfPortFWSSkipPort(9,GT_TRUE);
            prvTgfPortFWSSkipPort(11,GT_TRUE);
            prvTgfPortFWSSkipPort(14,GT_TRUE);
            prvTgfPortFWSSkipPort(15,GT_TRUE);

            prvTgfPortFWSSkipPort(16,GT_TRUE);
            prvTgfPortFWSSkipPort(19,GT_TRUE);
            prvTgfPortFWSSkipPort(22,GT_TRUE);
            prvTgfPortFWSSkipPort(23,GT_TRUE);
            prvTgfPortFWSSkipPort(24,GT_TRUE);
            prvTgfPortFWSSkipPort(27,GT_TRUE);
            prvTgfPortFWSSkipPort(28,GT_TRUE);
            prvTgfPortFWSSkipPort(31,GT_TRUE);

            /* the only 1 G port cannot be used in FWS tests */
            prvTgfPortFWSSkipPort(32,GT_TRUE);
        }
    }
    else if (PRV_CPSS_DXCH_CAELUM_CHECK_MAC(prvTgfDevNum))
    {
        /* Caelum has oversubscription by default configuration.
           Ports BW is 48+120 G but processing pipe supports only 120 G.
           Skip some ports from FWS tests to avoid massive oversubscription.
           10G ports generates ~ 30% more then 10G in loopack mode. */

        /* skip Giga ports */
        prvTgfPortFWSSkipPort(8,GT_TRUE);
        prvTgfPortFWSSkipPort(19,GT_TRUE);
        prvTgfPortFWSSkipPort(21,GT_TRUE);
        prvTgfPortFWSSkipPort(34,GT_TRUE);
        prvTgfPortFWSSkipPort(45,GT_TRUE);

        /* skip 10G ports */
        prvTgfPortFWSSkipPort(56,GT_TRUE);
        prvTgfPortFWSSkipPort(57,GT_TRUE);
        prvTgfPortFWSSkipPort(59,GT_TRUE);
        prvTgfPortFWSSkipPort(64,GT_TRUE);
        prvTgfPortFWSSkipPort(67,GT_TRUE);
        prvTgfPortFWSSkipPort(70,GT_TRUE);

        /* skip OOB CPU port */
        prvTgfPortFWSSkipPort(90,GT_TRUE);
    }
    else if (PRV_CPSS_DXCH_CETUS_CHECK_MAC(prvTgfDevNum) && (GT_TRUE == prvUtfIsTrafficManagerUsed()))
    {
        /* Cetus with TM has 25 Gbps bandwidth only.
           Leave only two 10G ports in test and skip other ports. */
        prvTgfPortFWSSkipPort(57,GT_TRUE);
        prvTgfPortFWSSkipPort(58,GT_TRUE);
        prvTgfPortFWSSkipPort(59,GT_TRUE);
        prvTgfPortFWSSkipPort(64,GT_TRUE);
        prvTgfPortFWSSkipPort(65,GT_TRUE);
        prvTgfPortFWSSkipPort(66,GT_TRUE);
        prvTgfPortFWSSkipPort(67,GT_TRUE);
        prvTgfPortFWSSkipPort(68,GT_TRUE);
        prvTgfPortFWSSkipPort(69,GT_TRUE);
        prvTgfPortFWSSkipPort(70,GT_TRUE);

#ifndef ASIC_SIMULATION
        /* increase WS burst to support TM ports */
        prvTgfWsBurstSet(70);
#endif

    }

    switch(PRV_CPSS_PP_MAC(prvTgfDevNum)->devType)
    {
        case CPSS_98DX8332_Z0_CNS:
            prvTgfPortsArray[0] =  0;
            prvTgfPortsArray[1] = 18;
            prvTgfPortsArray[2] = 33;
            prvTgfPortsArray[3] = 68;
            break;
        case CPSS_BOBK_CETUS_DEVICES_CASES_MAC: /*Cetus*/
            /* see also function utfIsMustUseOrigPorts(IN GT_U8  devNum) */
            prvTgfPortsArray[0] = 71;
            prvTgfPortsArray[1] = 64;
            prvTgfPortsArray[2] = 68;
            prvTgfPortsArray[3] = 56;
            prvTgfPortsArray[4] = 57;
            prvTgfPortsArray[5] = 58;
            prvTgfPortsArray[6] = 59;
            prvTgfPortsArray[7] = 65;
            prvTgfPortsArray[8] = 66;
            prvTgfPortsArray[9] = 67;
            prvTgfPortsArray[10]= 69;
            prvTgfPortsArray[11]= 70;

            prvTgfPortsNum = 12; /* for the setInvalidPortsInArr to come after the 12 ports */
            break;
        case CPSS_BOBCAT2_ALL_DEVICES_CASES_MAC: /* BC2 */
            if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devType == CPSS_98DX4253_CNS)
            {
                prvTgfPortsArray[0] = 0;
                prvTgfPortsArray[1] = 8;
                prvTgfPortsArray[2] = 14;
                prvTgfPortsArray[3] = 24;
            }
            else
            {
                /* the GM of BC2,Bobk access the MIB MAC Counters of the port according
                   to physical port and not according to 'MAC port'.
                   Therefore need to use 1:1 mapped ports for GM but not map below.  */
                if (GT_FALSE == prvUtfIsGmCompilation())
                {
                    prvTgfPortsArray[0] =  0;
                    prvTgfPortsArray[1] = 18;
                    prvTgfPortsArray[2] = 58;
                    prvTgfPortsArray[3] = 82;
                }
            }
            break;
        default:
            break;
    }

    /* set not used ports with illegal values */
    setInvalidPortsInArr(prvTgfPortsArray,prvTgfPortsNum,(PRV_TGF_MAX_PORTS_NUM_CNS - prvTgfPortsNum));

    PRV_UTF_LOG2_MAC("UTF: used port type [%s] , with ports up to[%d]\n" ,
                     tgfDeviceTypeNameArr[ii],
                     tgfDeviceTypeNumOfPorts[ii] );


    /* update default number of ports */
    prvTgfPortsNum = prvTgfDefPortsArraySize[ii];
}

/* debug variable and function to set it                     */
/* prevents recalculation of port list done by several tests */
static GT_BOOL prvTgfPortsArrayRecalculaionDisable = GT_FALSE;
GT_STATUS prvTgfPortsArrayRecalculaionDisableSet(GT_BOOL disable)
{
    prvTgfPortsArrayRecalculaionDisable = disable;
    return GT_OK;
}

/**
* @internal prvTgfPortsArrayByDevTypeSet function
* @endinternal
*
* @brief   Override default ports from prvTgfPortsArray by new ports from
*         prvTgfDefPortsArray depending on device type
*/
GT_VOID prvTgfPortsArrayByDevTypeSet
(
    GT_VOID
)
{
    GT_U8       devNum = prvTgfDevNum;

    if (prvTgfPortsArrayRecalculaionDisable != GT_FALSE)
    {
        return;
    }

    /* device type index in port array */
    if(PRV_CPSS_PP_MAC(devNum)->numOfPorts > 60 ||
       PRV_CPSS_SIP_5_CHECK_MAC(devNum)/* supports 128/256/512 physical ports regardless to num MAC ports */)
    {
        /* by default we run the Lion2 with ports only from hemisphere 0
           see function prvTgfPortsDevTypeForceSet
        */
        prvTgfTestsPortRangeSelect(60);
    }
    else
    {
        prvTgfTestsPortRangeSelect(PRV_CPSS_PP_MAC(devNum)->numOfPorts);
    }
    return;
}

/**
* @internal prvTgfPortsArrayByDevTypeRandomSet function
* @endinternal
*
* @brief   Fill ports array with valid random port numbers
*
* @param[in] seed                     -  value for random engine
*                                       None
*/
GT_VOID prvTgfPortsArrayByDevTypeRandomSet
(
    IN  GT_U32      seed
)
{
    CPSS_PORTS_BMP_STC  portBmp    = {{0}};
    GT_U32               tmpPortNum = 0;
    GT_U32              portIter   = 0;


    /* clear port bitmap */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBmp);

    /* set static params for random engine */
    prvTgfIsRandomPortsSet = GT_TRUE;
    prvTgfRandomSeedNum    = seed;

    /* in PreInit stage cannot check dev\ports so wait till initialization */
    if (GT_FALSE == prvTgfCommonPreInitStage)
    {
        /* set specific seed for random generator */
        cpssOsSrand(seed);

        portIter = 0;

        /* fill port array with random generated port numbers */
        while (portIter < prvTgfPortsNum)
        {
            /* generate random port number */
            tmpPortNum = (cpssOsRand() % UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(prvTgfDevNum));

            /* check that ports is valid and unique */
            if ((GT_FALSE == PRV_CPSS_PHY_PORT_IS_EXIST_MAC(prvTgfDevNum, tmpPortNum)) ||
                (GT_TRUE == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portBmp, tmpPortNum)))
            {
                /* try to generate another port number */
                continue;
            }
            else
            {
                /* set appropriate bit for unique port in bitmap */
                CPSS_PORTS_BMP_PORT_SET_MAC(&portBmp, tmpPortNum);
            }

            /* set valid unique port number into port array */
            prvTgfPortsArray[portIter++] = tmpPortNum;
        }
    }

    return;
}

/**
* @internal prvTgfPortsArrayPrint function
* @endinternal
*
* @brief   Print number of used ports and port's array prvTgfPortsArray
*/
GT_VOID prvTgfPortsArrayPrint
(
    GT_VOID
)
{
    GT_U32  portIter = 0;


    /* print out number of ports */
    PRV_UTF_LOG1_MAC("Number of ports = %d\n", prvTgfPortsNum);

    /* print out port's array */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        PRV_UTF_LOG2_MAC("  prvTgfPortsArray[%d] = %d\n",
                         portIter, prvTgfPortsArray[portIter]);
    }

    return;
}

/**
* @internal prvTgfCommonMemberForceInit function
* @endinternal
*
* @brief   init the need to force members into vlan/mc groups.
*         to support cascade ports and ring ports.
*
* @note called for debug from terminal , so function is not static
*
*/
GT_STATUS prvTgfCommonMemberForceInit(
    IN PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT tagCmd
)
{
    GT_U32  ii;
    PRV_TGF_MEMBER_FORCE_INFO_STC   member;
    GT_U8   devNum;
    GT_STATUS   rc;
    GT_U32  devIdx;

    devNum = prvTgfDevNum;

    member.forceToVlan = GT_TRUE;
    member.forceToVidx = GT_TRUE;
    member.vlanInfo.tagged = GT_TRUE;

    if(PRV_CPSS_IS_DEV_EXISTS_MAC(devNum) == 0)
    {
        return GT_NOT_INITIALIZED;
    }

    if (GT_OK != appDemoDevIdxGet(devNum, &devIdx))
    {
        return GT_FAIL;
    }

    if(0 == CPSS_PORTS_BMP_IS_ZERO_MAC(&appDemoPpConfigList[devIdx].multiPortGroupRingPortsBmp))
    {
        /* state that the tests may look in 4 port groups for FDB entry */
        prvTgfNumOfPortGroups = PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.numOfPortGroups;

        /* such system NOT work with storm prevention ! */
        prvTgfBrgFdbForceIgnoreNaStormPrevention(GT_TRUE);/* FORCE to ignore the storm prevention */
    }

    for(ii = 0 ; ii < CPSS_MAX_PORTS_NUM_CNS; ii++)
    {
        if((0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&appDemoPpConfigList[devIdx].multiPortGroupRingPortsBmp,ii)) &&
           (0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&appDemoPpConfigList[devIdx].relayPortGroupPortsBmp,ii)))
        {
            continue;
        }

        member.member.devNum = devNum;
        member.member.portNum = ii;

        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&appDemoPpConfigList[devIdx].relayPortGroupPortsBmp,ii))
        {
            member.forceToVidx = GT_TRUE;
            member.vlanInfo.tagCmd = PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E;
        }
        else
        {
            member.vlanInfo.tagCmd = tagCmd;/* settings for 'cascade ports' (and ring ports) */
            if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
               (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)||
               (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                /* lionB , Lion2; Bobcat2; Caelum; Bobcat3 */
                member.forceToVidx = GT_FALSE;/* must not add to vidx */
            }


        }
        rc = prvTgfCommonMemberForceInfoSet(&member);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvTgfCommonInit function
* @endinternal
*
* @brief   Initialize TFG configuration
*
* @param[in] firstDevNum              - device number of first device.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note Put all necessary initializations into this function
*
*/
GT_STATUS prvTgfCommonInit
(
    GT_U8 firstDevNum
)
{
    GT_U32  portIter = 0;
    GT_U32   i = 0;
    GT_U32  errors = 0;

    /* PreInit stage is finished */
    prvTgfCommonPreInitStage = GT_FALSE;

    prvTgfDevNum = firstDevNum;

    /* set not used ports with illegal values */
    setInvalidPortsInArr(prvTgfPortsArray,prvTgfPortsNum,(PRV_TGF_MAX_PORTS_NUM_CNS - prvTgfPortsNum));

    for( i = 0 ; i < PRV_TGF_MAX_PORTS_NUM_CNS ; i++ )
    {
        prvTgfDevsArray[i] = firstDevNum;

    }

    for( i = 0 ; i < CPSS_MAX_PORTS_NUM_CNS ; i++ )
    {
        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&appDemoPpConfigList[firstDevNum].relayPortGroupPortsBmp,i))
        {
            tgfLoopBackCt10And40Enable = GT_TRUE;
            break;
        }
    }


    /* if used PreInit ports array set so need to check if ports is valid */
    if ((GT_TRUE == prvTgfPortsArrayPreInitSet) && (tgfLoopBackCt10And40Enable == GT_FALSE))
    {
        GT_U32 macNum;
        /* check ports in default port array */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* check that ports is valid */
            PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(
                prvTgfDevNum, prvTgfPortsArray[portIter], macNum);
        }
    }
    else /* no PreInit --> set default ports array */
    {
        /* set port configuration according to device family */
        prvTgfPortsArrayByDevTypeSet();
    }

    /* generate random ports array if needed */
    if (GT_TRUE == prvTgfIsRandomPortsSet)
    {
        prvTgfPortsArrayByDevTypeRandomSet(prvTgfRandomSeedNum);
    }

    if(PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(prvTgfDevNum))
    {
        GT_U32 portGroupId = 0 ;
        GT_U32 iterationsNum = 0;

        /* make sure that ports of non exist core will not appear in the test*/
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* check that ports is valid */
            if(0 == (PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.activePortGroupsBmp &
               (1 << PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(prvTgfDevNum,prvTgfPortsArray[portIter]))))
            {
                prvTgfPortsArray[portIter] = PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(prvTgfDevNum, portGroupId , ((2 * portIter)%12));
                portGroupId++;
                portGroupId %= PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups;
                portIter--;/* retry the new suggested port */
                iterationsNum++;

                if(iterationsNum > (2 * PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups))
                {
                    /* not found proper odd/even port , so exit */
                    errors ++;
                    break;
                }

                if(iterationsNum > PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups)
                {
                    /* not found proper even port , so try odd one */
                    prvTgfPortsArray[portIter] = PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(prvTgfDevNum, portGroupId , ((2 * portIter)%12));
                    prvTgfPortsArray[portIter]++;
                }

                continue;
            }

            iterationsNum = 0;
        }

        /* find duplications */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            for(i = (portIter + 1); i < prvTgfPortsNum; i++)
            {
                if(prvTgfPortsArray[i] == prvTgfPortsArray[portIter])
                {
                    errors ++;
                    break;
                }
            }
        }
    }

    if(errors)
    {
        PRV_UTF_LOG1_MAC("prvTgfCommonInit: number of errors = %d \n" , errors);
    }
    /*initializing Auto Flow library*/
    tgfAutoFlowLibInit();

    return prvTgfCommonMemberForceInit(
        PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
}

/**
* @internal prvTgfCommonUseLastTpid function
* @endinternal
*
* @brief   state that test use last TPIDs for ingress or not
*
* @param[in] enable
*                                      GT_TRUE  - the device  use last TPIDs for ingress
*                                      GT_FALSE - the device  NOT use last TPIDs for ingress
*/
void prvTgfCommonUseLastTpid(
    IN GT_BOOL  enable
)
{
    useLastTpid = enable;
}

/**
* @internal prvTgfCommonIsUseLastTpid function
* @endinternal
*
* @brief   check if the test use last TPIDs for ingress
*
* @retval GT_TRUE                  - the device use last TPIDs for ingress
* @retval GT_FALSE                 - the device NOT use last TPIDs for ingress
*/
GT_BOOL prvTgfCommonIsUseLastTpid(
    void
)
{
    return useLastTpid;
}

/**
* @internal prvTgfCommonPortTxTwoUcTcRateByCncGet function
* @endinternal
*
* @brief   Get rate of two unicast flows by CNC counters.
*
* @param[in] portIdx                  - index of egress port
* @param[in] tc1                      - traffic class of first unicast flow
* @param[in] tc2                      - traffic class of second unicast flow
* @param[in] timeOut                  - timeout in milli seconds to calculate rate
*
* @param[out] outRate1Ptr              - (pointer to) Tx rate of first flow
* @param[out] outRate1Ptr              - (pointer to) Tx rate of second flow
* @param[out] dropRate1Ptr             - (pointer to) drop rate of first flow
* @param[out] dropRate1Ptr             - (pointer to) drop rate of second flow
*                                       none
*/
GT_VOID prvTgfCommonPortTxTwoUcTcRateByCncGet
(
    IN  GT_U32 portIdx,
    IN  GT_U32 tc1,
    IN  GT_U32 tc2,
    IN  GT_U32 timeOut,
    OUT GT_U32 *outRate1Ptr,
    OUT GT_U32 *outRate2Ptr,
    OUT GT_U32 *dropRate1Ptr,
    OUT GT_U32 *dropRate2Ptr
)
{
    PRV_TGF_CNC_CLIENT_ENT      client = PRV_TGF_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E;
    PRV_TGF_CNC_COUNTER_FORMAT_ENT    format = PRV_TGF_CNC_COUNTER_FORMAT_MODE_2_E;
    PRV_TGF_CNC_COUNTER_FORMAT_ENT    saveFormat[2];
    GT_U32                      indexRangeBitMap[4];
    GT_U32                      cntrIdxArr[4];
    GT_U32                      startTimeStampArr[4];
    GT_U32                      endTimeStampArr[4];
    GT_U32                      ii;
    GT_U32                      cntrBlockIndex;
    GT_U32                      blockNum;
    PRV_TGF_CNC_COUNTER_STC     counterArr[4];
    GT_STATUS                   rc;                 /* return codes */
    GT_PHYSICAL_PORT_NUM        portNum;            /* port number */

    portNum = prvTgfPortsArray[portIdx];

    if (prvTgfCommonVntTimeStampsDebug)
        PRV_UTF_LOG3_MAC("\nCNC rate Port %d TC1 %d TC2 %d\n", portNum, tc1, tc2);

    /* Pass counters */
    cntrIdxArr[0] = prvTgfCncTxqClientIndexGet(prvTgfDevNum, GT_FALSE /*cnMode*/, 0 /*droppedPacket*/, portNum, tc1, 0);
    cntrIdxArr[1] = prvTgfCncTxqClientIndexGet(prvTgfDevNum, GT_FALSE /*cnMode*/, 0 /*droppedPacket*/, portNum, tc2, 0);

    /* Drop counters */
    cntrIdxArr[2] = prvTgfCncTxqClientIndexGet(prvTgfDevNum, GT_FALSE /*cnMode*/, 1 /*droppedPacket*/, portNum, tc1, 0);
    cntrIdxArr[3] = prvTgfCncTxqClientIndexGet(prvTgfDevNum, GT_FALSE /*cnMode*/, 1 /*droppedPacket*/, portNum, tc2, 0);

    /* use block 0 for Pass counters and block 1 for Drop counters */
    for (blockNum = 0; blockNum < 2; blockNum++)
    {
        rc = prvTgfCncBlockClientEnableSet(blockNum, client, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientEnableSet: %d", blockNum);

        cpssOsBzero((GT_CHAR *)indexRangeBitMap, sizeof(indexRangeBitMap));
        indexRangeBitMap[0] = 1 << (cntrIdxArr[blockNum * 2] / _1K);
        rc = prvTgfCncBlockClientRangesSet(blockNum,client,indexRangeBitMap);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientRangesSet: %d", blockNum);

        rc = prvTgfCncCounterFormatGet(blockNum, &saveFormat[blockNum]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterFormatSet");

        rc = prvTgfCncCounterFormatSet(blockNum, format);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterFormatSet");
    }

    /* reset counters */
    for (ii = 0; ii < 4; ii++)
    {
        cntrBlockIndex = cntrIdxArr[ii] % _1K;
        blockNum = ii / 2;
        rc = prvTgfCncCounterGet(blockNum, cntrBlockIndex,format, &counterArr[ii]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncCounterGet: %d %d",blockNum, cntrBlockIndex);

        rc = prvTgfVntPortLastReadTimeStampGet(prvTgfDevNum, portNum, &startTimeStampArr[ii]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfVntPortLastReadTimeStampGet: dev %d port %d", prvTgfDevNum,portNum);
    }

    /* sleep */
    cpssOsTimerWkAfter(timeOut);

    /* get counters */
    for (ii = 0; ii < 4; ii++)
    {
        cntrBlockIndex = cntrIdxArr[ii] % _1K;
        blockNum = ii / 2;
        rc = prvTgfCncCounterGet(blockNum, cntrBlockIndex,format, &counterArr[ii]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncCounterGet: %d %d",blockNum, cntrBlockIndex);

        rc = prvTgfVntPortLastReadTimeStampGet(prvTgfDevNum, portNum, &endTimeStampArr[ii]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfVntPortLastReadTimeStampGet: dev %d port %d", prvTgfDevNum,portNum);
    }

    /* get rates by VNT Time Stamps */
    *outRate1Ptr = prvTgfCommonVntTimeStampsRateGet(prvTgfDevNum, startTimeStampArr[0], endTimeStampArr[0],counterArr[0].packetCount.l[0]);
    *outRate2Ptr = prvTgfCommonVntTimeStampsRateGet(prvTgfDevNum, startTimeStampArr[1], endTimeStampArr[1],counterArr[1].packetCount.l[0]);
    *dropRate1Ptr = prvTgfCommonVntTimeStampsRateGet(prvTgfDevNum, startTimeStampArr[2], endTimeStampArr[2],counterArr[2].packetCount.l[0]);
    *dropRate2Ptr = prvTgfCommonVntTimeStampsRateGet(prvTgfDevNum, startTimeStampArr[3], endTimeStampArr[3],counterArr[3].packetCount.l[0]);

    if (prvTgfCommonVntTimeStampsDebug)
    {
        PRV_UTF_LOG1_MAC("CNC---rate egressCntrsArr[0].outUcFrames %d\n", *outRate1Ptr);
        PRV_UTF_LOG1_MAC("CNC---rate egressCntrsArr[1].outUcFrames %d\n", *outRate2Ptr);
        PRV_UTF_LOG1_MAC("CNC---rate egressCntrsArr[0].txqFilterDisc %d\n", *dropRate1Ptr);
        PRV_UTF_LOG1_MAC("CNC---rate egressCntrsArr[1].txqFilterDisc %d\n", *dropRate2Ptr);
    }

    /* restore default of CNC Counters */
    for (blockNum = 0; blockNum < 2; blockNum++)
    {
        rc = prvTgfCncBlockClientEnableSet(blockNum, client, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientEnableSet: %d", blockNum);

        cpssOsBzero((GT_CHAR *)indexRangeBitMap, sizeof(indexRangeBitMap));
        rc = prvTgfCncBlockClientRangesSet(blockNum,client,indexRangeBitMap);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientRangesSet: %d", blockNum);

        rc = prvTgfCncCounterFormatSet(blockNum, saveFormat[blockNum]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterFormatSet");
    }

    /* reset counters */
    for (ii = 0; ii < 4; ii++)
    {
        cntrBlockIndex = cntrIdxArr[ii] % _1K;
        blockNum = ii / 2;
        rc = prvTgfCncCounterGet(blockNum, cntrBlockIndex,format, &counterArr[ii]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncCounterGet: %d %d",blockNum, cntrBlockIndex);
    }
}

/**
* @internal prvTgfCommonPortTxTwoUcTcRateGet function
* @endinternal
*
* @brief   Get rate of two unicast flows by egress counters.
*
* @param[in] portIdx                  - index of egress port
* @param[in] tc1                      - traffic class of first unicast flow
* @param[in] tc2                      - traffic class of second unicast flow
* @param[in] timeOut                  - timeout in milli seconds to calculate rate
*
* @param[out] outRate1Ptr              - (pointer to) Tx rate of first flow
* @param[out] outRate1Ptr              - (pointer to) Tx rate of second flow
* @param[out] dropRate1Ptr             - (pointer to) drop rate of first flow
* @param[out] dropRate1Ptr             - (pointer to) drop rate of second flow
*                                       none
*/
GT_VOID prvTgfCommonPortTxTwoUcTcRateGet
(
    IN  GT_U32 portIdx,
    IN  GT_U32 tc1,
    IN  GT_U32 tc2,
    IN  GT_U32 timeOut,
    OUT GT_U32 *outRate1Ptr,
    OUT GT_U32 *outRate2Ptr,
    OUT GT_U32 *dropRate1Ptr,
    OUT GT_U32 *dropRate2Ptr
)
{
    CPSS_PORT_EGRESS_CNTR_STC   egressCntrsArr[2]; /* egress counters */
    GT_U8                       cntrSetId;         /* iterator */
    GT_U8                       tc;                /* traffic class */
    GT_STATUS                   rc, rc1;           /* return codes */
    GT_U32                       portNum;           /* port number */
    GT_U32      startNano, endNano;   /* start and end time seconds part */
    GT_U32      startSec, endSec;     /* start and end time nanoseconds part */
    GT_U32      startMilli, endMilli; /* start and end time in milliseconds */
    GT_U32      startTimeStamp;       /* debug - VNT time stamp start */
    GT_U32      endTimeStamp;         /* debug - VNT time stamp start */
    GT_U32      rate;                 /* debug - VNT time stamp based rate */

    if (prvTgfPortEgressCntIsCaelumErratumExist(prvTgfDevNum))
    {
        /* use CNC for rate calculations */
        prvTgfCommonPortTxTwoUcTcRateByCncGet(portIdx,tc1,tc2,timeOut,outRate1Ptr,outRate2Ptr,dropRate1Ptr,dropRate2Ptr);
        return;
    }

    portNum = prvTgfPortsArray[portIdx];
    /* set mode of Egress Port Counters.
       enable filter by TC and Port. */
    for (cntrSetId = 0; (cntrSetId < 2); cntrSetId++)
    {
        tc = (GT_U8)((cntrSetId == 0) ? tc1 : tc2);
        rc = prvTgfPortEgressCntrModeSet(cntrSetId,
                                         CPSS_EGRESS_CNT_PORT_E | CPSS_EGRESS_CNT_TC_E,
                                         portNum,
                                         0, tc, 0);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrModeSet: %d", tc);
    }

    /* reset counters */
    rc  = prvTgfPortEgressCntrsGet(0, &egressCntrsArr[0]);
    rc1 = prvTgfPortEgressCntrsGet(1, &egressCntrsArr[0]);

    /* save start time */
    cpssOsTimeRT(&startSec,&startNano);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrsGet\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc1, "prvTgfPortEgressCntrsGet\n");

    rc = prvTgfVntPortLastReadTimeStampGet(prvTgfDevNum, portNum, &startTimeStamp);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfVntPortLastReadTimeStampGet: dev %d port %d", prvTgfPortsArray[portIdx],portNum);

    /* sleep */
    cpssOsTimerWkAfter(timeOut);

    /* get counters */
    rc = prvTgfPortEgressCntrsGet(0, &egressCntrsArr[0]);
    rc1 = prvTgfPortEgressCntrsGet(1, &egressCntrsArr[1]);

    /* get end time */
    cpssOsTimeRT(&endSec,&endNano);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrsGet\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc1, "prvTgfPortEgressCntrsGet\n");

    rc = prvTgfVntPortLastReadTimeStampGet(prvTgfDevNum, portNum, &endTimeStamp);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfVntPortLastReadTimeStampGet: dev %d port %d", prvTgfPortsArray[portIdx], portNum);

    startMilli = startSec * 1000 + startNano / 1000000;
    endMilli = endSec * 1000 + endNano / 1000000;
    /* calculate exact timeout */
    timeOut = endMilli - startMilli;

    *outRate1Ptr = prvTgfGetRate(egressCntrsArr[0].outUcFrames, 0, timeOut);
    *outRate2Ptr = prvTgfGetRate(egressCntrsArr[1].outUcFrames, 0, timeOut);
    *dropRate1Ptr = prvTgfGetRate(egressCntrsArr[0].txqFilterDisc, 0, timeOut);
    *dropRate2Ptr = prvTgfGetRate(egressCntrsArr[1].txqFilterDisc, 0, timeOut);

    if (prvTgfCommonVntTimeStampsInUse)
    {
        /* get rates by VNT Time Stamps */
        *outRate1Ptr = prvTgfCommonVntTimeStampsRateGet(prvTgfDevNum, startTimeStamp, endTimeStamp,egressCntrsArr[0].outUcFrames);
        *outRate2Ptr = prvTgfCommonVntTimeStampsRateGet(prvTgfDevNum, startTimeStamp, endTimeStamp,egressCntrsArr[1].outUcFrames);
        *dropRate1Ptr = prvTgfCommonVntTimeStampsRateGet(prvTgfDevNum, startTimeStamp, endTimeStamp,egressCntrsArr[0].txqFilterDisc);
        *dropRate2Ptr = prvTgfCommonVntTimeStampsRateGet(prvTgfDevNum, startTimeStamp, endTimeStamp,egressCntrsArr[1].txqFilterDisc);
    }

    if (prvTgfCommonVntTimeStampsDebug)
    {
        rate = prvTgfCommonVntTimeStampsRateGet(prvTgfDevNum, startTimeStamp, endTimeStamp,egressCntrsArr[0].outUcFrames);
        PRV_UTF_LOG1_MAC("---rate egressCntrsArr[0].outUcFrames %d\n", rate);
        rate = prvTgfCommonVntTimeStampsRateGet(prvTgfDevNum, startTimeStamp, endTimeStamp,egressCntrsArr[1].outUcFrames);
        PRV_UTF_LOG1_MAC("---rate egressCntrsArr[1].outUcFrames %d\n", rate);
        rate = prvTgfCommonVntTimeStampsRateGet(prvTgfDevNum, startTimeStamp, endTimeStamp,egressCntrsArr[0].txqFilterDisc);
        PRV_UTF_LOG1_MAC("---rate egressCntrsArr[0].txqFilterDisc %d\n", rate);
        rate = prvTgfCommonVntTimeStampsRateGet(prvTgfDevNum, startTimeStamp, endTimeStamp,egressCntrsArr[1].txqFilterDisc);
        PRV_UTF_LOG1_MAC("---rate egressCntrsArr[1].txqFilterDisc %d\n", rate);
    }

    /* check other counters. they must be 0 */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, egressCntrsArr[0].brgEgrFilterDisc, "TC1 brgEgrFilterDisc\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, egressCntrsArr[0].egrFrwDropFrames, "TC1 egrFrwDropFrames\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, egressCntrsArr[0].outBcFrames, "TC1 outBcFrames\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, egressCntrsArr[0].outCtrlFrames, "TC1 outCtrlFrames\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, egressCntrsArr[0].outMcFrames, "TC1 outMcFrames\n");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, egressCntrsArr[1].brgEgrFilterDisc, "TC2 brgEgrFilterDisc\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, egressCntrsArr[1].egrFrwDropFrames, "TC2 egrFrwDropFrames\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, egressCntrsArr[1].outBcFrames, "TC2 outBcFrames\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, egressCntrsArr[1].outCtrlFrames, "TC2 outCtrlFrames\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, egressCntrsArr[1].outMcFrames, "TC2 outMcFrames\n");

    /* restore default of Egress Port Counters */
    rc = prvTgfPortEgressCntrModeSet(0, 0, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrModeSet: %d", 0);

    rc = prvTgfPortEgressCntrModeSet(1, 0, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrModeSet: %d", 0);

}

/**
* @internal prvTgfCommonPortCntrRateGet function
* @endinternal
*
* @brief   Get rate of specific port MAC MIB counter.
*
* @param[in] portIdx                  - index of egress port
* @param[in] cntrName                 - name of counter
* @param[in] timeOut                  - timeout in milli seconds to calculate rate
*
* @param[out] outRatePtr               - (pointer to) rate of port's counter
*                                       none
*/
GT_VOID prvTgfCommonPortCntrRateGet
(
    IN  GT_U32                      portIdx,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    IN  GT_U32                      timeOut,
    OUT GT_U32                      *outRatePtr
)
{
    GT_U32 portIdxArr[1];

    portIdxArr[0] = portIdx;
    prvTgfCommonPortsCntrRateGet(portIdxArr,1,cntrName,timeOut,outRatePtr);
}

/**
* @internal prvTgfCommonPortsCntrRateGet function
* @endinternal
*
* @brief   Get rate of specific ports MAC MIB counter.
*
* @param[in] portIdxArr[]             - array of indexes of ports
* @param[in] numOfPorts               - number of ports
* @param[in] cntrName                 - name of counter
* @param[in] timeOut                  - timeout in milli seconds to calculate rate
*
* @param[out] outRateArr[]             - (pointer to) array of rate of ports
*                                       none
*/
GT_VOID prvTgfCommonPortsCntrRateGet
(
    IN  GT_U32 portIdxArr[],
    IN  GT_U32 numOfPorts,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    IN  GT_U32 timeOut,
    OUT GT_U32 outRateArr[]
)
{
    GT_STATUS rc;
    GT_U32  ii;
    GT_U8    devNumArr[PRV_TGF_TX_PORTS_MAX_NUM_CNS];
    GT_U32   portNumArr[PRV_TGF_TX_PORTS_MAX_NUM_CNS];

    if (numOfPorts > PRV_TGF_TX_PORTS_MAX_NUM_CNS)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_TX_PORTS_MAX_NUM_CNS, numOfPorts, "prvTgfCommonPortsCntrRateGet: too much ports %d", numOfPorts);
        return;
    }

    for(ii = 0 ; ii < numOfPorts; ii++)
    {
        devNumArr[ii] = prvTgfDevNum;
        portNumArr[ii] = prvTgfPortsArray[portIdxArr[ii]];
    }

    rc = prvTgfCommonPortsCntrRateGet_byDevPort(devNumArr,portNumArr,numOfPorts,timeOut,cntrName,outRateArr,0,NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCommonPortsCntrRateGet_byDevPort: failed");
}


/**
* @internal prvTgfCommonPortTxRateGet function
* @endinternal
*
* @brief   Get Tx rate of port by MAC MIB counters.
*
* @param[in] portIdx                  - index of egress port
* @param[in] timeOut                  - timeout in milli seconds to calculate rate
*
* @param[out] outRatePtr               - (pointer to) Tx rate of port
*                                       none
*/
GT_VOID prvTgfCommonPortTxRateGet
(
    IN  GT_U32 portIdx,
    IN  GT_U32 timeOut,
    OUT GT_U32 *outRatePtr
)
{
    prvTgfCommonPortCntrRateGet(portIdx, CPSS_GOOD_UC_PKTS_SENT_E,
                                timeOut,outRatePtr);
}

/**
* @internal prvTgfCommonPortRxRateGet function
* @endinternal
*
* @brief   Get Rx rate of port by MAC MIB counters.
*
* @param[in] portIdx                  - index of ingress port
* @param[in] timeOut                  - timeout in milli seconds to calculate rate
*
* @param[out] outRatePtr               - (pointer to) Rx rate of port
*                                       none
*/
GT_VOID prvTgfCommonPortRxRateGet
(
    IN  GT_U32 portIdx,
    IN  GT_U32 timeOut,
    OUT GT_U32 *outRatePtr
)
{
    prvTgfCommonPortCntrRateGet(portIdx, CPSS_GOOD_UC_PKTS_RCV_E,
                                timeOut,outRatePtr);
}

/**
* @internal prvTgfCommonPortsRxRateGet function
* @endinternal
*
* @brief   Get Rx rate of ports by MAC MIB counters.
*
* @param[in] portIdxArr[]             - array of indexes of ingress ports
* @param[in] numOfPorts               - number of ports
* @param[in] timeOut                  - timeout in milli seconds to calculate rate
*
* @param[out] outRateArr[]             - (pointer to) array of Rx rate of ports
*                                       none
*/
GT_VOID prvTgfCommonPortsRxRateGet
(
    IN  GT_U32 portIdxArr[],
    IN  GT_U32 numOfPorts,
    IN  GT_U32 timeOut,
    OUT GT_U32 outRateArr[]
)
{
    prvTgfCommonPortsCntrRateGet(portIdxArr, numOfPorts, CPSS_GOOD_PKTS_RCV_E,
                                timeOut,outRateArr);
}

/**
* @internal prvTgfCommonPortsTxRateGet function
* @endinternal
*
* @brief   Get Tx rate of ports by MAC MIB counters.
*
* @param[in] portIdxArr[]             - array of indexes of egress ports
* @param[in] numOfPorts               - number of ports
* @param[in] timeOut                  - timeout in milli seconds to calculate rate
*
* @param[out] outRateArr[]             - (pointer to) array of Tx rate of ports
*                                       none
*/
GT_VOID prvTgfCommonPortsTxRateGet
(
    IN  GT_U32 portIdxArr[],
    IN  GT_U32 numOfPorts,
    IN  GT_U32 timeOut,
    OUT GT_U32 outRateArr[]
)
{
    prvTgfCommonPortsCntrRateGet(portIdxArr, numOfPorts, CPSS_GOOD_PKTS_SENT_E,
                                timeOut,outRateArr);
}

/**
* @internal prvTgfCommonPortWsRateGet function
* @endinternal
*
* @brief   Get wire speed rate of port in packets per second
*
* @param[in] portIdx                  - index of egress port
* @param[in] packetSize               - packet size in bytes including CRC
*                                       wire speed rate of port in packets per second.
*
* @retval 0xFFFFFFFF               - on error
*/
GT_U32 prvTgfCommonPortWsRateGet
(
    IN  GT_U32 portIdx,
    IN  GT_U32 packetSize
)
{
    return prvTgfCommonDevicePortWsRateGet(prvTgfDevNum, prvTgfPortsArray[portIdx],packetSize);
 }

/*******************************************************************************
* prvTgfPrefixPrint
*
* DESCRIPTION:
*       Print IP address, mask and prefix
*
* INPUTS:
*       protocol  - protocol type
*       ipAddrPtr - (pointer to) IP address
*       ipMaskPtr - (pointer to) IP mask
*       prefixLen - prefix length
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK        - on success
*       GT_BAD_PARAM - on wrong parameters
*       GT_BAD_PTR   - on null pointer
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvTgfPrefixPrint
(
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN GT_U8                      *ipAddrPtr,
    IN GT_U8                      *ipMaskPtr,
    IN GT_U8                      prefixLen
)
{
    GT_U8   byteIter     = 0;
    GT_U8   maxPrefixLen = 0;
    GT_U8   separator    = 0;
    GT_CHAR *grpFormatPtr;
    GT_CHAR *grpSeparatorPtr;


    /* check parameters */
    CPSS_NULL_PTR_CHECK_MAC(ipAddrPtr);
    CPSS_NULL_PTR_CHECK_MAC(ipMaskPtr);

    /* check protocol type and set max prefix length */
    switch (protocol)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            /* set max prefix length and byte separaot step */
            maxPrefixLen = sizeof(TGF_IPV4_ADDR);
            separator    = 1;

            /* set prefix print format */
            grpFormatPtr    = "%03d";
            grpSeparatorPtr = ".";

            break;

        case CPSS_IP_PROTOCOL_IPV6_E:
            /* set max prefix length and byte separaot step */
            maxPrefixLen = sizeof(TGF_IPV6_ADDR);
            separator    = 2;

            /* set prefix print format */
            grpFormatPtr    = "%02X";
            grpSeparatorPtr = ":";

            break;

        default:
            return GT_BAD_PARAM;
    }

    /* print IP address */
    PRV_UTF_LOG0_MAC("ip=[");
    for (byteIter = 0; byteIter < maxPrefixLen - 1; byteIter++)
    {
        PRV_UTF_LOG1_MAC(grpFormatPtr, ipAddrPtr[byteIter]);
        if ((byteIter + 1) % separator == 0)
        {
            PRV_UTF_LOG0_MAC(grpSeparatorPtr);
        }
    }
    PRV_UTF_LOG1_MAC(grpFormatPtr, ipAddrPtr[maxPrefixLen - 1]);

    /* print mask */
    PRV_UTF_LOG0_MAC("], mask=[");
    for (byteIter = 0; byteIter < maxPrefixLen - 1; byteIter++)
    {
        PRV_UTF_LOG1_MAC(grpFormatPtr, ipMaskPtr[byteIter]);
        if ((byteIter + 1) % separator == 0)
        {
            PRV_UTF_LOG0_MAC(grpSeparatorPtr);
        }
    }
    PRV_UTF_LOG1_MAC(grpFormatPtr, ipMaskPtr[maxPrefixLen - 1]);

    /* print prefix */
    PRV_UTF_LOG0_MAC("], prefix=[");
    for (byteIter = 0; byteIter < maxPrefixLen - 1; byteIter++)
    {
        PRV_UTF_LOG1_MAC(grpFormatPtr, ipAddrPtr[byteIter] & ipMaskPtr[byteIter]);
        if ((byteIter + 1) % separator == 0)
        {
            PRV_UTF_LOG0_MAC(grpSeparatorPtr);
        }
    }
    PRV_UTF_LOG1_MAC(grpFormatPtr, ipAddrPtr[maxPrefixLen - 1] & ipMaskPtr[maxPrefixLen - 1]);

    /* print prefix length */
    PRV_UTF_LOG1_MAC("]/%d\n", prefixLen);

    return GT_OK;
 }

/*******************************************************************************
* prvTgfCommonAllBridgeCntrsReset
*
* DESCRIPTION:
*       Reset hw counters - egress counters, bridge host group counters,
*       bridge ingress counters, bridge host group counters.
*
* INPUTS:
*                none
*
* OUTPUTS:
*                none
*
* RETURNS:
*       none
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvTgfCommonAllBridgeCntrsReset
(
    GT_VOID
)
{
    GT_STATUS                     rc, rc1 = GT_OK;
    GT_U8                         devNum  = 0;
    CPSS_PORT_EGRESS_CNTR_STC     egrCntr = {0};
    PRV_TGF_BRG_HOST_CNTR_STC     hostGroupCntr = {0};
    CPSS_BRIDGE_INGRESS_CNTR_STC  ingressCntr = {0};
    GT_U32                        matrixCntSaDaPkts = {0};


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* Gets a egress counters */
        rc = prvTgfPortEgressCntrsGet(0, &egrCntr);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortEgressCntrsGet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }

        /* Gets a egress counters */
        rc = prvTgfPortEgressCntrsGet(1, &egrCntr);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortEgressCntrsGet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }

        /* Gets Bridge Host group counters */
        rc = prvTgfBrgCntHostGroupCntrsGet(devNum, &hostGroupCntr);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgCntHostGroupCntrsGet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }

        /* Gets Bridge Host group counters */
        rc = prvTgfBrgCntBridgeIngressCntrsGet(devNum, PRV_TGF_BRG_CNTR_SET_ID_0_E, &ingressCntr);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgCntBridgeIngressCntrsGet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }

        /* Gets a Bridge ingress counters */
        rc = prvTgfBrgCntBridgeIngressCntrsGet(devNum, PRV_TGF_BRG_CNTR_SET_ID_1_E, &ingressCntr);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgCntBridgeIngressCntrsGet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }

        /* Gets a Bridge ingress counters */
        rc = prvTgfBrgCntMatrixGroupCntrsGet(devNum, &matrixCntSaDaPkts);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgCntMatrixGroupCntrsGet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }

    return rc1;
}

/**
* @internal prvTgfCommonForceLinkUpOnAllTestedPorts function
* @endinternal
*
* @brief   force link up on all tested ports.
*/
void prvTgfCommonForceLinkUpOnAllTestedPorts
(
    void
)
{
    GT_U32  portIter;/* port iterator */
    CPSS_INTERFACE_INFO_STC portInterface;

    portInterface.type            = CPSS_INTERFACE_PORT_E;

    /* force linkup on all ports involved */
    for(portIter = 0; portIter < prvTgfPortsNum ; portIter++)
    {
        portInterface.devPort.hwDevNum  = prvTgfDevsArray[portIter];
        portInterface.devPort.portNum = prvTgfPortsArray[portIter];

        tgfTrafficGeneratorPortForceLinkUpEnableSet(&portInterface,GT_TRUE);
    }
}

/**
* @internal prvTgfPortsArrayPrintAll function
* @endinternal
*
* @brief   Print number of used ports and port's array prvTgfPortsArray
*/
void prvTgfPortsArrayPrintAll(void)
{
    GT_U32  portIter = 0;

    /* print out number of ports */
    PRV_UTF_LOG1_MAC("Number of ports = %d\n", prvTgfPortsNum);

    /* print out port's array */
    for (portIter = 0; portIter < PRV_TGF_MAX_PORTS_NUM_CNS; portIter++)
    {
        PRV_UTF_LOG2_MAC("  prvTgfPortsArray[%d] = %d\n",
                         portIter, prvTgfPortsArray[portIter]);
    }

    return;
}

/**
* @internal prvTgfDefPortsArrange function
* @endinternal
*
* @brief   Reorder content of prvTgfPortsArray to ensure that items with specified
*         indices are ports satisfying the condition.
* @param[in] isRemote                 - the condition.
*                                      GT_TRUE  - ports should be remote ports
*                                      GT_FALSE - ports must not be remote ports.
* @param[in] index                    - first  that is need to be checked.
*                                      ...        - rest of the indices. The negative values mark the end of
*                                      the list.
*
* @retval GT_BAD_PARAM             - wrong index. Index exceeds either size of prvTgfPortsArray
*                                       or the number of actual (not PRV_TGF_INVALID_PORT_NUM_CNS)
*                                       ports in prvTgfPortsArray.
* @retval GT_FAIL                  - prvTgfPortsArrays items can't be organized to satisfy
*                                       requirements.
*/
GT_STATUS prvTgfDefPortsArrange
(
    IN GT_BOOL isRemote,
    IN GT_32   index,
    IN ...
)
{
    va_list   args;
    GT_U32    port;
    GT_BOOL   found;
    GT_U32    ixGood = 0;   /* index of prvTgfPortsArray satisfying condition */
    GT_U32    bmp[(PRV_TGF_MAX_PORTS_NUM_CNS + 31) / 32] = {0};
    GT_STATUS rc = GT_OK;

    va_start(args, index);
    while (index >= 0)
    {
        if (index >= PRV_TGF_MAX_PORTS_NUM_CNS ||
            prvTgfPortsArray[index] == PRV_TGF_INVALID_PORT_NUM_CNS)
        {
            rc = GT_BAD_PARAM;
            break;
        }

        found = GT_FALSE;
        while (ixGood < PRV_TGF_MAX_PORTS_NUM_CNS &&
               prvTgfPortsArray[ixGood] != PRV_TGF_INVALID_PORT_NUM_CNS)
        {
            /* check if we handled this item before */
            if (! (bmp[ixGood>>5] & (1 << ixGood & 0x1f)))
            {
                /* does this item satisfy condition? */
                if (isRemote == prvCpssDxChPortRemotePortCheck(prvTgfDevNum,
                                                               prvTgfPortsArray[ixGood]))
                {
                    found = GT_TRUE;
                    break;
                }
            }
            ixGood++;
        }

        if (GT_FALSE == found)
        {
            return GT_FAIL;
        }

        /* swap items */
        port = prvTgfPortsArray[index];
        prvTgfPortsArray[index] = prvTgfPortsArray[ixGood];
        prvTgfPortsArray[ixGood] = port;
        /* mark the index as handled */
        bmp[index>>5] |= (1 << index & 0x1f);

        /* get next index */
        index = va_arg(args, GT_32);
    }
    va_end(args);

    return rc;
}


/**
* @internal prvTgfFilterDefPortsArray function
* @endinternal
*
* @brief   Fill destination array with only ports from prvTgfPortsArray which
*         appropriate to specified filter
* @param[in] getRemote                - a filter.
*                                      GT_TRUE  - get only remote ports.
*                                      GT_FALSE - get only local ports.
* @param[in] destArrLen               - maximum number of items in the destination array
*
* @param[out] destArr                  - destination array with gathered ports
*                                       actual number of items in destArr. Can't exceed destArrLen
*/
GT_U32 prvTgfFilterDefPortsArray
(
    IN  GT_BOOL                  getRemote,
    OUT GT_U32                   *destArr,
    IN  GT_U32                   destArrLen
)
{
    GT_U32 cnt = 0;
    GT_U32 i;

    for (i=0; (i < PRV_TGF_MAX_PORTS_NUM_CNS
               && prvTgfPortsArray[i] != PRV_TGF_INVALID_PORT_NUM_CNS
               && cnt < destArrLen); i++)
    {
        if(getRemote == prvCpssDxChPortRemotePortCheck(prvTgfDevNum,prvTgfPortsArray[i]))
        {
            destArr[cnt++] = prvTgfPortsArray[i];
        }
    }
    /* setInvalidPortsInArr(destArr, cnt,(PRV_TGF_MAX_PORTS_NUM_CNS - cnt)); */

    return cnt;
}


/**
* @internal prvTgfXcat3xExistsSet function
* @endinternal
*
* @brief   Raise flag indicating existance of AC3X (Aldrin + 88E1690)
*         in the system
* @param[in] isExists                 - GT_TRUE  - exists
*                                      GT_FALSE - not exists
*                                       None.
*/
GT_VOID prvTgfXcat3xExistsSet
(
    GT_BOOL isExists
)
{
    prvTgfIsXcat3x = isExists;
}

/**
* @internal prvTgfXcat3xExists function
* @endinternal
*
* @brief   Get flag indicating existance of AC3X (Aldrin + 88E1690) in the system
*
* @retval GT_TRUE                  - exists
* @retval GT_FALSE                 - not exists
*/
GT_BOOL prvTgfXcat3xExists
(
    GT_VOID
)
{
    return prvTgfIsXcat3x;
}

/**
* @internal prvTgfFilterDsaSpecialPortsArray function
* @endinternal
*
* @brief   Fill destination array with only ports from prvTgfPortsArray which
*         appropriate to specified filter
* @param[in] dsaType                 - a filter according to the DSA type.
*                       check if the port number is special (59-63) when need to
*                       be in the DSA tag as the DSA hold limited number of bits
*                       for the portNum.
*
* @param[in] destArrLen     - maximum number of items in the destination array
*
* @param[out] destArr        - destination array with gathered ports
*                             actual number of items in destArr. Can't exceed destArrLen
*/
GT_U32 prvTgfFilterDsaSpecialPortsArray
(
    IN  TGF_DSA_TYPE_ENT         dsaType,
    OUT GT_U32                   *destArr,
    IN  GT_U32                   destArrLen
)
{
    GT_U32 cnt = 0;
    GT_U32 ii;
    GT_U32  filteredPorts[PRV_TGF_MAX_PORTS_NUM_CNS];
    GT_U32  filteredPortsNum = 0;

    for (ii=0; (ii < PRV_TGF_MAX_PORTS_NUM_CNS
       && prvTgfPortsArray[ii] != PRV_TGF_INVALID_PORT_NUM_CNS
       && cnt < destArrLen); ii++)
    {
        if(GT_FALSE == tgfTrafficGeneratorIsSpecialPortNumAfterReductionInDsa(
            prvTgfDevNum,
            prvTgfPortsArray[ii],
            dsaType))
        {
            /* the port is allowed to be in the DSA test , it's bits are not
               reduced to special ports */
            destArr[cnt++] = prvTgfPortsArray[ii];
        }
        else
        {
            filteredPorts[filteredPortsNum++] = prvTgfPortsArray[ii];
        }
    }

    if(prvTgfPortsArray == destArr)/* The same pointer ! */
    {
        /* add the filtered ports back to prvTgfPortsArray[] */
        /* maybe the test will need it for other purpose     */
        for(ii = cnt; ii < (cnt + filteredPortsNum); ii++)
        {
            prvTgfPortsArray[ii] = filteredPorts[ii - cnt];
        }
    }

    return cnt;
}

/**
* @internal prvTgfFilterLimitedBitsSpecialPortsArray function
* @endinternal
*
* @brief   Fill destination array with only ports from prvTgfPortsArray which
*         appropriate to specified filter
* @param[in] numOfBits - a filter according to the number of bits.
*                       check if the port number is special (59-63) when need to
*                       be in limited to <numOfBits> .
* @param[in] offset    - offset to added to ports in the array , only to check
*                       that value not in the filtered range .
*
* @param[in] destArrLen     - maximum number of items in the destination array
*
* @param[out] destArr        - destination array with gathered ports
*                             actual number of items in destArr. Can't exceed destArrLen
*/
GT_U32 prvTgfFilterLimitedBitsSpecialPortsArray
(
    IN  GT_U32                   numOfBits,
    IN  GT_32                    offset,
    OUT GT_U32                   *destArr,
    IN  GT_U32                   destArrLen
)
{
    GT_U32 cnt = 0;
    GT_U32 ii;
    GT_U32  filteredPorts[PRV_TGF_MAX_PORTS_NUM_CNS];
    GT_U32  filteredPortsNum = 0;

    for (ii=0; (ii < PRV_TGF_MAX_PORTS_NUM_CNS
       && prvTgfPortsArray[ii] != PRV_TGF_INVALID_PORT_NUM_CNS
       && cnt < destArrLen); ii++)
    {
        if(GT_FALSE == tgfTrafficGeneratorIsSpecialPortNumAfterReductionInBits(
            prvTgfDevNum,
            prvTgfPortsArray[ii] + offset,
            numOfBits))
        {
            /* the port is allowed to be in the <numOfBits> test , it's bits are not
               reduced to special ports */
            destArr[cnt++] = prvTgfPortsArray[ii];
        }
        else
        {
            filteredPorts[filteredPortsNum++] = prvTgfPortsArray[ii];
        }
    }

    if(prvTgfPortsArray == destArr)/* The same pointer ! */
    {
        /* add the filtered ports back to prvTgfPortsArray[] */
        /* maybe the test will need it for other purpose     */
        for(ii = cnt; ii < (cnt + filteredPortsNum); ii++)
        {
            prvTgfPortsArray[ii] = filteredPorts[ii - cnt];
        }
    }

    return cnt;
}



/**
* @internal prvTgfPortModeGet function
* @endinternal
*
* @brief   return the number of ports use by the Device for 
*          falcon . 64,128,256,512 and 1024
*  
*/
GT_U32 prvTgfPortModeGet
(
  GT_VOID
)
{
  
    return PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(prvTgfDevNum).phyPort;

}