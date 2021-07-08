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
* @file prvTgfUnucUnregmcBcFiltering.c
*
* @brief L2 MLL Unknown Unicast, Unregistered Multicast and Broadcast filtering test
*
* @version   10
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfL2MllGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfNetIfGen.h>

#include <l2mll/prvTgfL2MllUtils.h>
#include <l2mll/prvTgfUnucUnregmcBcFiltering.h>

#include <gtUtil/gtBmPool.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_VLANID_CNS  2

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     3

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x99}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/* array of destination MACs for various traffic types */
static TGF_MAC_ADDR prvTgfMacTypesArr[] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
                                           {0x01, 0x00, 0x00, 0x00, 0x00, 0x01},
                                           {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                          };

/*****************************************************************************\
 *                           Static variables                                 *
 *****************************************************************************/
#define PRV_TGF_PACKET_LENGTH_CNS (64)
#define TX_QUEUE_0 (0)
#define SEND_VIDX (4096)

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal sdmaPacketParamsInit function
* @endinternal
*
* @brief   Init packet to send
*
* @param[out] pcktParamsPtr            - packet parameters
*                                       None
*
* @note Does not set parmeters connect with TxBufferQueueEvent
*
*/
static GT_VOID sdmaPacketParamsInit
(
    OUT PRV_TGF_NET_TX_PARAMS_STC *pcktParamsPtr
)
{
    cpssOsMemSet(pcktParamsPtr, 0, sizeof(PRV_TGF_NET_TX_PARAMS_STC));
    pcktParamsPtr->sdmaInfo.txQueue = TX_QUEUE_0;
    pcktParamsPtr->sdmaInfo.recalcCrc = 1;
    pcktParamsPtr->sdmaInfo.invokeTxBufferQueueEvent = GT_FALSE;
    pcktParamsPtr->packetIsTagged = GT_FALSE;
    pcktParamsPtr->dsaParam.dsaType = TGF_DSA_4_WORD_TYPE_E;
    pcktParamsPtr->dsaParam.dsaCommand = TGF_DSA_CMD_FROM_CPU_E;
    pcktParamsPtr->dsaParam.dsaInfo.fromCpu.srcHwDev = prvTgfDevNum;
    pcktParamsPtr->dsaParam.dsaInfo.fromCpu.cascadeControl = GT_FALSE;
    pcktParamsPtr->dsaParam.dsaInfo.fromCpu.tc = 0;
    pcktParamsPtr->dsaParam.dsaInfo.fromCpu.dp = 0;
    pcktParamsPtr->dsaParam.dsaInfo.fromCpu.egrFilterEn = GT_FALSE;
    pcktParamsPtr->dsaParam.dsaInfo.fromCpu.egrFilterRegistered = GT_FALSE;
    pcktParamsPtr->dsaParam.dsaInfo.fromCpu.srcId = 0;
    pcktParamsPtr->dsaParam.dsaInfo.fromCpu.dstInterface.type =
        CPSS_INTERFACE_VIDX_E;
    pcktParamsPtr->dsaParam.dsaInfo.fromCpu.dstInterface.vlanId = 1;
    pcktParamsPtr->dsaParam.dsaInfo.fromCpu.dstInterface.vidx = 4096;
    pcktParamsPtr->dsaParam.commonParams.cfiBit = 0;
    pcktParamsPtr->dsaParam.commonParams.vid = 1;
    pcktParamsPtr->dsaParam.commonParams.vpt = 0;
    pcktParamsPtr->dsaParam.commonParams.dropOnSource = GT_FALSE;
    pcktParamsPtr->dsaParam.commonParams.packetIsLooped = GT_FALSE;
}

/**
* @internal prvTgfL2MllVlanInit function
* @endinternal
*
* @brief   Initialize local vlan default settings
*
* @param[in] vlanId                   - vlan id.
*                                       None
*/
static GT_VOID prvTgfL2MllVlanInit
(
    IN GT_U16                                  vlanId
)
{
    GT_STATUS                   rc = GT_OK;
    CPSS_PORTS_BMP_STC          portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    CPSS_PORTS_BMP_STC          portsMembers;
    GT_U32                      portIter;

    /* clear entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* ports 0, 8, 18, 23 are VLAN Members */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,prvTgfPortsArray[portIter]);
    }

    /* set vlan entry */
    vlanInfo.unkSrcAddrSecBreach  = GT_FALSE;
    vlanInfo.unregNonIpMcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv6McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unkUcastCmd          = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4BcastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.ipv4IgmpToCpuEn      = GT_FALSE;
    vlanInfo.mirrToRxAnalyzerEn   = GT_FALSE;
    vlanInfo.ipv6IcmpToCpuEn      = GT_FALSE;
    vlanInfo.ipCtrlToCpuEn        = PRV_TGF_BRG_IP_CTRL_NONE_E;
    vlanInfo.ipv4IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv6IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv4IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6SiteIdMode       = CPSS_IP_SITE_ID_INTERNAL_E;
    vlanInfo.ipv4UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv4McastRouteEn     = GT_FALSE;
    vlanInfo.ipv6UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv6McastRouteEn     = GT_FALSE;
    vlanInfo.stgId                = 0;
    vlanInfo.autoLearnDisable     = GT_TRUE;/* working in controlled learning */
    vlanInfo.naMsgToCpuEn         = GT_TRUE;/* working in controlled learning */
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    vlanInfo.floodVidx            = 0x1000;
    vlanInfo.floodVidxMode        = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;

    /* AUTODOC: create VLAN 2 with untagged ports [0,1,2,3], floodVidx 0x1000 */
    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 vlanId,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d", vlanId);
}

/**
* @internal prvTgfUnucUnregmcBcFilteringConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfUnucUnregmcBcFilteringConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC   lttEntry;

    PRV_TGF_PAIR_READ_WRITE_FORM_ENT     mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC              mllPairEntry;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* Save L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationGetAndSave();

    /* AUTODOC: enable MLL lookup for all multi-target packets */
    rc = prvTgfL2MllLookupForAllEvidxEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupForAllEvidxEnableSet: enable %d",
                                 GT_TRUE);

    /* set VLAN entry */
    prvTgfL2MllVlanInit(PRV_TGF_VLANID_CNS);

    /* create L2 MLL LTT entry */
    cpssOsMemSet(&lttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

    lttEntry.mllPointer = 0;
    lttEntry.entrySelector = 0;

    /* AUTODOC: add L2 MLL LTT entry 4096 with: */
    /* AUTODOC:   mllPointer=0, mllMaskProfileEnable=False, mllMaskProfile=0 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 4096, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 4096");

    /* create L2 MLL entries */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(PRV_TGF_L2_MLL_PAIR_STC));

    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_WHOLE_E;
    mllPairEntry.nextPointer = 1;
    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    mllPairEntry.firstMllNode.last = GT_FALSE;
    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.firstMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[0];

    mllPairEntry.secondMllNode.last = GT_FALSE;
    mllPairEntry.secondMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.secondMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[1];

    /* AUTODOC: add L2 MLL entry 0: */
    /* AUTODOC: nextPointer=1, port=0,port=1 */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 0, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 0");


    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E;

    mllPairEntry.nextPointer = 0;
    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    mllPairEntry.firstMllNode.last = GT_TRUE;
    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.firstMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[2];

    /* AUTODOC: add L2 MLL entry 1: */
    /* AUTODOC:   nextPointer=0, port=2 */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 1, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 2");
}

/**
* @internal prvTgfL2MllFilteringSet function
* @endinternal
*
* @brief   Set L2 MLL entry Unknown Unicast, Unregistered Multicast and Broadcast
*         filtering configuration.
* @param[in] index                    - L2 MLL entry index.
* @param[in] unknowUc                 - enabling unknown UC filtering.
* @param[in] unregMc                  - enabling unregistered MC filtering.
* @param[in] bc                       - enabling BC filtering.
*                                       None
*/
GT_VOID prvTgfL2MllFilteringSet
(
    IN GT_U32 index,
    IN GT_BOOL unknowUc,
    IN GT_BOOL unregMc,
    IN GT_BOOL bc
)
{
    GT_STATUS rc;
    GT_U32                                      mllPairEntryIndex;
    PRV_TGF_PAIR_READ_WRITE_FORM_ENT            mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC                     mllPairEntry;


    if(index%2 == 0)
    {
        mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E;
        mllPairEntryIndex = index/2 ;
    }
    else
    {
        mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E;
        mllPairEntryIndex = index/2;
    }

    rc = prvTgfL2MllPairRead(prvTgfDevNum, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairRead: index %d", mllPairEntryIndex);

    if(index%2 == 0)
    {
        mllPairEntry.firstMllNode.unknownUcFilterEnable = unknowUc;
        mllPairEntry.firstMllNode.unregMcFilterEnable   = unregMc;
        mllPairEntry.firstMllNode.bcFilterEnable        = bc;
    }
    else
    {
        mllPairEntry.secondMllNode.unknownUcFilterEnable = unknowUc;
        mllPairEntry.secondMllNode.unregMcFilterEnable   = unregMc;
        mllPairEntry.secondMllNode.bcFilterEnable        = bc;
    }

    rc = prvTgfL2MllPairWrite(prvTgfDevNum, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index %d", mllPairEntryIndex);
}

/**
* @internal prvTgfTrafficTypeCreate function
* @endinternal
*
* @brief   Create and send various traffic types
*
* @param[in] type                     - 0: Unknown Unicast
*                                      1: Unregistered Multicast
*                                      2: Broadcast
*                                       None
*/
GT_VOID prvTgfTrafficTypeCreate
(
    IN GT_U32 type
)
{
    /* set destination MAC address in Packet */
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfMacTypesArr[type],
                 sizeof(prvTgfPacketL2Part.daMac));

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* send packet from port[PRV_TGF_SEND_PORT_IDX_CNS] */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);
}

/**
* @internal prvTgfCpuTrafficTypeCreate function
* @endinternal
*
* @brief   Create and send various traffic types
*
* @param[in] type                     - 0: Unknown Unicast
*                                      1: Unregistered Multicast
*                                      2: Broadcast
*                                       None
*/
GT_VOID prvTgfCpuTrafficTypeCreate
(
    IN GT_U32 type
)
{
    GT_U8                      *buffList[1];
    GT_U32                      buffLenList[1];
    GT_U32                      packetLen;
    GT_U8                      *bufferPtr;
    PRV_TGF_NET_TX_PARAMS_STC   pcktParams;
    GT_STATUS                   rc;

    /* AUTODOC: Prepare buffers*/
    bufferPtr = prvTgfTrafficGeneratorPoolGetBuffer();
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, (GT_UINTPTR)bufferPtr, "gtPoolGetBuf");
    buffList[0] = bufferPtr;
    packetLen = 0;

    /* AUTODOC: Select destination MAC */
    cpssOsMemCpy(bufferPtr, &prvTgfMacTypesArr[type],
        sizeof(prvTgfMacTypesArr[type]));
    packetLen += sizeof(prvTgfMacTypesArr[type]);

    /* AUTODOC: Write source MAC */
    cpssOsMemCpy(bufferPtr + packetLen, &prvTgfPacketL2Part.saMac,
        sizeof(prvTgfPacketL2Part.saMac));
    packetLen += sizeof(prvTgfPacketL2Part.saMac);

    /* AUTODOC: Set ethertype 0x33 0x33 */
    *(bufferPtr + packetLen + 0) = 0x33;
    *(bufferPtr + packetLen + 1) = 0x33;
    packetLen += 2;

    /* AUTODOC: Copy payload */
    cpssOsMemCpy(bufferPtr + packetLen, prvTgfPayloadDataArr,
        sizeof(prvTgfPayloadDataArr));
    packetLen += sizeof(prvTgfPayloadDataArr);

    buffLenList[0] = packetLen;

    /* AUTODOC: Init CPU packet */
    sdmaPacketParamsInit(&pcktParams);

    /* AUTODOC: Reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: Send CPU packet */
    rc = prvTgfNetIfSdmaTxPacketSend(prvTgfDevNum, &pcktParams, buffList,
        buffLenList, 1);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNetIfSdmaTxPacketSend: "
        "devNum=[%d], bufNum=[%d]", prvTgfDevNum, 1);
    cpssOsTimerWkAfter(50);

    /* AUTODOC: Free buffer */
    rc = prvTgfTrafficGeneratorPoolFreeBuffer(bufferPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "gtPoolFreeBuf");

}

/**
* @internal prvTgfTrafficCheck function
* @endinternal
*
* @brief   check expected traffic results
*
* @param[in] exResults                - array which contains the expected results for each port.
*
* @param[out] allEqual                 - (pointer to) GT_TRUE: all results as expected.
*                                      GT_FALSE: some results different then expected.
*                                       none.
*/
GT_VOID prvTgfTrafficCheck
(
    IN GT_U32 *exResults,
    OUT GT_BOOL *allEqual
)
{
    GT_STATUS rc;
    GT_U32    portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;

    *allEqual = GT_TRUE;

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        UTF_VERIFY_EQUAL2_STRING_MAC(exResults[portIter], portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);

        if( exResults[portIter] != portCntrs.goodPktsSent.l[0] )
            *allEqual = GT_FALSE;
    }
}

#define FILL_ARR_WITH_EX_RESULTS(_arr,r0,r1,r2,r3)  \
                                    _arr[0] = r0;   \
                                    _arr[1] = r1;   \
                                    _arr[2] = r2;   \
                                    _arr[3] = r3;

#define FILL_ARR_WITH_EX_3RESULTS(_arr,r0,r1,r2)    \
        FILL_ARR_WITH_EX_RESULTS(_arr,r0,r1,r2,1)

/**
* @internal prvTgfUnucUnregmcBcFilteringTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfUnucUnregmcBcFilteringTrafficGenerate
(
    GT_VOID
)
{
    GT_U32      exResults[4];
    GT_BOOL     allEqual;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: send Unknown UC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:00:01, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(0);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 1, 1, 1);

    /* AUTODOC: verify Ethernet packet on ports [0,1,2] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: send Unregister MC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(1);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 1, 1, 1);

    /* AUTODOC: verify Ethernet packet on ports [0,1,2] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: send BC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(2);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 1, 1, 1);

    /* AUTODOC: verify Ethernet packet on ports [0,1,2] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: modify L2 MLL entry 0: */
    /* AUTODOC:   enable unknown UC Filter */
    /* AUTODOC:   disable unreg MC Filter */
    /* AUTODOC:   disable BC Filter */
    prvTgfL2MllFilteringSet(0, GT_TRUE, GT_FALSE, GT_FALSE);

    /* AUTODOC: send Unknown UC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:00:01, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(0);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 0, 1, 1);

    /* AUTODOC: verify Ethernet packet on ports [1,2] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: send Unregister MC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(1);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 1, 1, 1);

    /* AUTODOC: verify Ethernet packet on ports [0,1,2] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: send BC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(2);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 1, 1, 1);

    /* AUTODOC: verify Ethernet packet on ports [0,1,2] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: modify L2 MLL entry 0: */
    /* AUTODOC:   disable unknown UC Filter */
    /* AUTODOC:   disable unreg MC Filter */
    /* AUTODOC:   disable BC Filter */
    prvTgfL2MllFilteringSet(0, GT_FALSE, GT_FALSE, GT_FALSE);

    /* AUTODOC: modify L2 MLL entry 1: */
    /* AUTODOC:   disable unknown UC Filter */
    /* AUTODOC:   enable unreg MC Filter */
    /* AUTODOC:   disable BC Filter */
    prvTgfL2MllFilteringSet(1, GT_FALSE, GT_TRUE, GT_FALSE);

    /* AUTODOC: send Unknown UC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:00:01, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(0);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 1, 1, 1);

    /* AUTODOC: verify Ethernet packet on ports [0,1,2] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: send Unregister MC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(1);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 1, 0, 1);

    /* AUTODOC: verify Ethernet packet on ports [0,2] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: send BC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(2);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 1, 1, 1);

    /* AUTODOC: verify Ethernet packet on ports [0,1,2] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: modify L2 MLL entry 1: */
    /* AUTODOC:   disable unknown UC Filter */
    /* AUTODOC:   disable unreg MC Filter */
    /* AUTODOC:   disable BC Filter */
    prvTgfL2MllFilteringSet(1, GT_FALSE, GT_FALSE, GT_FALSE);

    /* AUTODOC: modify L2 MLL entry 2: */
    /* AUTODOC:   disable unknown UC Filter */
    /* AUTODOC:   disable unreg MC Filter */
    /* AUTODOC:   enable BC Filter */
    prvTgfL2MllFilteringSet(2, GT_FALSE, GT_FALSE, GT_TRUE);

    /* AUTODOC: send Unknown UC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:00:01, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(0);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 1, 1, 1);

    /* AUTODOC: verify Ethernet packet on ports [0,1,2] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: send Unregister MC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(1);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 1, 1, 1);

    /* AUTODOC: verify Ethernet packet on ports [0,1,2] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: send BC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(2);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 1, 1, 0);

    /* AUTODOC: verify Ethernet packet on ports [0,1] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: modify L2 MLL entry 0: */
    /* AUTODOC:   enable unknown UC Filter */
    /* AUTODOC:   disable unreg MC Filter */
    /* AUTODOC:   disable BC Filter */
    prvTgfL2MllFilteringSet(0, GT_TRUE, GT_FALSE, GT_FALSE);

    /* AUTODOC: modify L2 MLL entry 1: */
    /* AUTODOC:   disable unknown UC Filter */
    /* AUTODOC:   enable unreg MC Filter */
    /* AUTODOC:   disable BC Filter */
    prvTgfL2MllFilteringSet(1, GT_FALSE, GT_TRUE, GT_FALSE);

    /* AUTODOC: send Unknown UC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:00:01, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(0);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 0, 1, 1);

    /* AUTODOC: verify Ethernet packet on ports [1,2] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: send Unregister MC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(1);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 1, 0, 1);

    /* AUTODOC: verify Ethernet packet on ports [0,2] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: send BC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(2);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 1, 1, 0);

    /* AUTODOC: verify Ethernet packet on ports [0,1] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: modify L2 MLL entry 1: */
    /* AUTODOC:   enable unknown UC Filter */
    /* AUTODOC:   enable unreg MC Filter */
    /* AUTODOC:   disable BC Filter */
    prvTgfL2MllFilteringSet(0, GT_TRUE, GT_TRUE, GT_FALSE);

    /* AUTODOC: modify L2 MLL entry 1: */
    /* AUTODOC:   disable unknown UC Filter */
    /* AUTODOC:   enable unreg MC Filter */
    /* AUTODOC:   enable BC Filter */
    prvTgfL2MllFilteringSet(1, GT_FALSE, GT_TRUE, GT_TRUE);

    /* AUTODOC: modify L2 MLL entry 2: */
    /* AUTODOC:   enable unknown UC Filter */
    /* AUTODOC:   disable unreg MC Filter */
    /* AUTODOC:   enable BC Filter */
    prvTgfL2MllFilteringSet(2, GT_TRUE, GT_FALSE, GT_TRUE);

    /* AUTODOC: send Unknown UC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:00:01, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(0);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 0, 1, 0);

    /* AUTODOC: verify Ethernet packet on ports [1] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: send Unregister MC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(1);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 0, 0, 1);

    /* AUTODOC: verify Ethernet packet on ports [2] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: send BC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(2);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 1, 0, 0);

    /* AUTODOC: verify Ethernet packet on ports [0] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

   /* AUTODOC: modify L2 MLL entry 0: */
    /* AUTODOC:   enable unknown UC Filter */
    /* AUTODOC:   enable unreg MC Filter */
    /* AUTODOC:   enable BC Filter */
    prvTgfL2MllFilteringSet(0, GT_TRUE, GT_TRUE, GT_TRUE);

    /* AUTODOC: modify L2 MLL entry 1: */
    /* AUTODOC:   disable unknown UC Filter */
    /* AUTODOC:   disable unreg MC Filter */
    /* AUTODOC:   disable BC Filter */
    prvTgfL2MllFilteringSet(1, GT_FALSE, GT_FALSE, GT_FALSE);

    /* AUTODOC: modify L2 MLL entry 2: */
    /* AUTODOC:   disable unknown UC Filter */
    /* AUTODOC:   disable unreg MC Filter */
    /* AUTODOC:   disable BC Filter */
    prvTgfL2MllFilteringSet(2, GT_FALSE, GT_FALSE, GT_FALSE);

    /* AUTODOC: send Unknown UC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:00:01, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(0);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 0, 1, 1);

    /* AUTODOC: verify Ethernet packet on ports [1,2] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: send Unregister MC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(1);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 0, 1, 1);

    /* AUTODOC: verify Ethernet packet on ports [1,2] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: send BC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(2);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 0, 1, 1);

    /* AUTODOC: verify Ethernet packet on ports [1,2] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: modify L2 MLL entry 1: */
    /* AUTODOC:   enable unknown UC Filter */
    /* AUTODOC:   enable unreg MC Filter */
    /* AUTODOC:   enable BC Filter */
    prvTgfL2MllFilteringSet(1, GT_TRUE, GT_TRUE, GT_TRUE);

    /* AUTODOC: send Unknown UC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:00:01, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(0);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 0, 0, 1);

    /* AUTODOC: verify Ethernet packet on ports [2] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: send Unregister MC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(1);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 0, 0, 1);

    /* AUTODOC: verify Ethernet packet on ports [2] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: send BC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(2);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 0, 0, 1);

    /* AUTODOC: verify Ethernet packet on ports [2] */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: modify L2 MLL entry 2: */
    /* AUTODOC:   enable unknown UC Filter */
    /* AUTODOC:   enable unreg MC Filter */
    /* AUTODOC:   enable BC Filter */
    prvTgfL2MllFilteringSet(2, GT_TRUE, GT_TRUE, GT_TRUE);

    /* AUTODOC: send Unknown UC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:00:01, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(0);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 0, 0, 0);

    /* AUTODOC: verify to get no traffic */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: send Unregister MC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(1);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 0, 0, 0);

    /* AUTODOC: verify to get no traffic */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: send BC Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:99, VID=2 */
    prvTgfTrafficTypeCreate(2);
    FILL_ARR_WITH_EX_3RESULTS(exResults, 0, 0, 0);

    /* AUTODOC: verify to get no traffic */
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: Check that  broadcast packet sent from CPU  does not passes */
    prvTgfL2MllFilteringSet(0, GT_FALSE, GT_FALSE, GT_FALSE);
    prvTgfL2MllFilteringSet(1, GT_FALSE, GT_FALSE, GT_FALSE);
    prvTgfL2MllFilteringSet(2, GT_FALSE, GT_FALSE, GT_FALSE);

    /* AUTODOC: Send Broadcast */
    prvTgfCpuTrafficTypeCreate(2);

    FILL_ARR_WITH_EX_RESULTS(exResults, 1, 1, 1, 0);
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

    /* AUTODOC: Check that  broadcast packet sent from CPU filtered */
    prvTgfL2MllFilteringSet(0, GT_FALSE, GT_FALSE, GT_TRUE);
    prvTgfL2MllFilteringSet(1, GT_FALSE, GT_FALSE, GT_TRUE);
    prvTgfL2MllFilteringSet(2, GT_FALSE, GT_FALSE, GT_TRUE);

    /* AUTODOC: Send Broadcast */
    prvTgfCpuTrafficTypeCreate(2);

    FILL_ARR_WITH_EX_RESULTS(exResults, 0, 0, 0, 0);
    prvTgfTrafficCheck(exResults, &allEqual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, allEqual, "prvTgfTrafficCheck");

}

/**
* @internal prvTgfUnucUnregmcBcFilteringConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfUnucUnregmcBcFilteringConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    lttIndexArray[1];
    GT_U32    mllIndexArray[3];

    /* AUTODOC: RESTORE CONFIGURATION: */
    /* AUTODOC: invalidate VLAN 2 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", PRV_TGF_VLANID_CNS);

    lttIndexArray[0] = 4096;
    mllIndexArray[0] = 0;
    mllIndexArray[1] = 1;
    mllIndexArray[2] = 2;

    /* AUTODOC: clear L2 MLL and LTT entries */
    prvTgfL2MllUtilsMllAndLttEntriesClear(lttIndexArray, 1, mllIndexArray, 3);

    /* AUTODOC: restore default L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationRestore();

    /* clear Rx CPU table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* stop Rx CPU capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);
}


