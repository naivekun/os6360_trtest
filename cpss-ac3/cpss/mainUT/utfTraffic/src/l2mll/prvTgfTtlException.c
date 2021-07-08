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
* @file prvTgfTtlException.c
*
* @brief L2 MLL TTL exception test
*
* @version   7
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

#include <l2mll/prvTgfL2MllUtils.h>
#include <l2mll/prvTgfLttEvidxRange.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_VLANID_CNS  1

/* VLAN Id  */
#define PRV_TGF_VLANID_3_CNS  3
#define PRV_TGF_VLANID_4_CNS  4

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     0

/* default values for restore */
static PRV_TGF_BRG_VLAN_TRANSLATION_ENT vlanPortTranslationEnableDef;
static GT_U16  vlanTranslationEntryDef;

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x01, 0x00, 0x00, 0x00, 0x00, 0x01},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x99}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    0x76CB,             /* csum */
    { 1,  1,  1,  1},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfTtlExceptionConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfTtlExceptionConfigurationSet
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

    /* AUTODOC: add FDB entry with MAC 01:00:00:00:00:01, VLAN 1, VIDX 1 */
    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfPacketL2Part.daMac, PRV_TGF_VLANID_CNS,
                                          1, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidxSet: eVidx 1");

    /* create L2 MLL LTT entry */
    cpssOsMemSet(&lttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

    lttEntry.mllPointer = 0;
    lttEntry.entrySelector = 0;

    /* AUTODOC: add L2 MLL LTT entry 1 with mllPointer=0 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 1, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 0");

    /* create L2 MLL entry */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(PRV_TGF_L2_MLL_PAIR_STC));

    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_WHOLE_E;

    mllPairEntry.nextPointer = 0;
    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.firstMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[2];
    mllPairEntry.firstMllNode.ttlThreshold = 0x0;
    mllPairEntry.firstMllNode.last = GT_FALSE;

    /* AUTODOC: the counter of <TTL Exception Counter> incremented only on the last replication . */
    /* AUTODOC: TTL Exception affects only that last copy that is not skipped */
    /* AUTODOC: so point to next MLL with dummy port */

    mllPairEntry.secondMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.secondMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[2] - 1;/* port not covered by the tests */
    mllPairEntry.secondMllNode.ttlThreshold = 0x0;
    mllPairEntry.secondMllNode.last = GT_TRUE;

    /* AUTODOC: add L2 MLL entry 0: */
    /* AUTODOC:  port=2 port='dummy Port'*/
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 0, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 0");

    /* clear Rx CPU table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: start Rx CPU capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");
}

/**
* @internal prvTgfTtlExceptionTtlVariations function
* @endinternal
*
* @brief   Check correct L2 MLL TTL exception behavior for different packet TTL &
*         L2 MLL TTL treshold vales.
* @param[in] packetTtl                - packet TTL
* @param[in] mllTtl                   - mll treshold TTL
* @param[in] exTxPacket               - expected number of transmit packets
* @param[in] exCounter                - expected value of TTL exception counter
*                                       None
*/
GT_VOID prvTgfTtlExceptionTtlVariations
(
    IN GT_U8  packetTtl,
    IN GT_U8  mllTtl,
    IN GT_U32 exTxPacket,
    IN GT_U32 exCounter
)
{
    GT_STATUS rc;
    GT_U32    portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32    expectedTxPacket;
    PRV_TGF_L2_MLL_EXCEPTION_COUNTERS_STC  counters;

    PRV_TGF_PAIR_READ_WRITE_FORM_ENT     mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC              mllPairEntry;

    /* packet TTL = 0x40, L2 MLL treshold = 0x7F */
    prvTgfPacketIpv4Part.timeToLive = packetTtl;

    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E;
    rc = prvTgfL2MllPairRead(prvTgfDevNum, 0, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairRead: index 0");

    mllPairEntry.firstMllNode.ttlThreshold = mllTtl;

    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 0, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 0");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* clear TTL exception counter */
    rc = prvTgfL2MllExceptionCountersGet(prvTgfDevNum, &counters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllExceptionCountersGet");

    /* AUTODOC: send IPv4 packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=1 */
    /* AUTODOC:   srcIP=1.1.1.1, dstIP=1.1.1.3 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Rx counters */
        switch(portIter)
        {
            case PRV_TGF_SEND_PORT_IDX_CNS: expectedTxPacket = 1;
                                            break;
            case 2: expectedTxPacket = exTxPacket;
                    break;
            default: expectedTxPacket = 0;
                     break;
        }

        UTF_VERIFY_EQUAL4_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d, "
                                     "packet TTL = %d, L2 MLL TTL = %d",
                                     portIter, prvTgfPortsArray[portIter], packetTtl, mllTtl);
    }

    /* read (& clear) TTL exception counter */
    rc = prvTgfL2MllExceptionCountersGet(prvTgfDevNum, &counters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllExceptionCountersGet");

    /* check TTL exception counter */
    UTF_VERIFY_EQUAL2_STRING_MAC(exCounter, counters.ttl,
                                 "TTL exception counter[%d] different then expected[%d], "
                                 "packet TTL = %d, L2 MLL TTL = %d",
                                 counters.ttl, exCounter);
}

/**
* @internal prvTgfTtlExceptionTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfTtlExceptionTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U8                trigPacketBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32               buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32               packetActualLength = 0;
    GT_U8                devNum;
    GT_U8                queue;
    TGF_NET_DSA_STC      rxParam;

    /* AUTODOC: GENERATE TRAFFIC: */

     /* set destination MAC address in Packet */
    prvTgfPacketVlanTagPart.vid = PRV_TGF_VLANID_CNS;

    /* AUTODOC: enable TTL exception trap to CPU with MLL_RPF_TRAP code */
    rc = prvTgfL2MllTtlExceptionConfigurationSet(prvTgfDevNum, GT_TRUE, CPSS_NET_MLL_RPF_TRAP_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllTtlExceptionConfigurationSet: "
                                            "enable , CPSS_NET_MLL_RPF_TRAP_E");

    /* AUTODOC: update L2 MLL treshold=0x7F for packet TTL=0x40 */
    prvTgfTtlExceptionTtlVariations(0x40, 0x7F, 0, 1);
    /* AUTODOC: verify counters: */
    /* AUTODOC:   TX counters = 0 */
    /* AUTODOC:   TTL exception counter = 1 */

    /* AUTODOC: get first entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                  GT_TRUE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &devNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    /* AUTODOC: verify CPU code = MLL_RPF_TRAP */
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_MLL_RPF_TRAP_E, rxParam.cpuCode,
                                 "sampled packet cpu code different then expected");

    /* Get next entry from captured packet's table - no entry expected*/
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, trigPacketBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue,
                                       &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    /* AUTODOC: disable TTL exception trap to CPU with MLL_RPF_TRAP code */
    rc = prvTgfL2MllTtlExceptionConfigurationSet(prvTgfDevNum, GT_FALSE, CPSS_NET_MLL_RPF_TRAP_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllTtlExceptionConfigurationSet: "
                                            "disable , CPSS_NET_MLL_RPF_TRAP_E");

    /* AUTODOC: update L2 MLL treshold=0x7F for packet TTL=0x40 */
    prvTgfTtlExceptionTtlVariations(0x40, 0x7F, 0, 1);
    /* AUTODOC: verify counters: */
    /* AUTODOC:   TX counters = 0 */
    /* AUTODOC:   TTL exception counter = 1 */

    /* Get entry from captured packet's table - no entry expected*/
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, trigPacketBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue,
                                       &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    /* AUTODOC: update L2 MLL treshold=0x7F for packet TTL=0x7F */
    prvTgfTtlExceptionTtlVariations(0x7F, 0x7F, 1, 0);
    /* AUTODOC: verify counters: */
    /* AUTODOC:   TX counters = 1 */
    /* AUTODOC:   TTL exception counter = 0 */

    /* AUTODOC: update L2 MLL treshold=0x7F for packet TTL=0x80 */
    prvTgfTtlExceptionTtlVariations(0x80, 0x7F, 1, 0);
    /* AUTODOC: verify counters: */
    /* AUTODOC:   TX counters = 1 */
    /* AUTODOC:   TTL exception counter = 0 */

    /* AUTODOC: update L2 MLL treshold=0xF for packet TTL=0x0 */
    prvTgfTtlExceptionTtlVariations(0x0, 0x0, 1, 0);
    /* AUTODOC: verify counters: */
    /* AUTODOC:   TX counters = 1 */
    /* AUTODOC:   TTL exception counter = 0 */

    /* AUTODOC: update L2 MLL treshold=0x7F for packet TTL=0x1 */
    prvTgfTtlExceptionTtlVariations(0x1, 0x0, 1, 0);
    /* AUTODOC: verify counters: */
    /* AUTODOC:   TX counters = 1 */
    /* AUTODOC:   TTL exception counter = 0 */

    /* AUTODOC: update L2 MLL treshold=0xFF for packet TTL=0x7F */
    prvTgfTtlExceptionTtlVariations(0x7F, 0xFF, 0, 1);
    /* AUTODOC: verify counters: */
    /* AUTODOC:   TX counters = 0 */
    /* AUTODOC:   TTL exception counter = 1 */

    /* AUTODOC: update L2 MLL treshold=0xFF for packet TTL=0xFF */
    prvTgfTtlExceptionTtlVariations(0xFF, 0xFF, 1, 0);
    /* AUTODOC: verify counters: */
    /* AUTODOC:   TX counters = 1 */
    /* AUTODOC:   TTL exception counter = 0 */

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();
}

/**
* @internal prvTgfTtlExceptionConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfTtlExceptionConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    lttIndexArray[1];
    GT_U32    mllIndexArray[1];

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    lttIndexArray[0] = 0;
    mllIndexArray[0] = 0;

    /* AUTODOC: clear L2 MLL and LTT entries */
    prvTgfL2MllUtilsMllAndLttEntriesClear(lttIndexArray, 1, mllIndexArray, 1);

    /* AUTODOC: restore default L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationRestore();

    /* AUTODOC: clear Rx CPU table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: stop Rx CPU capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");
}

/**
* @internal prvTgfTtlExceptionVlanPortTranslationConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_STATUS prvTgfTtlExceptionVlanPortTranslationConfigurationSet
(
    GT_U32 ttlThreshold
)
{
    GT_STATUS rc;
    PRV_TGF_BRG_VLAN_INFO_STC       cpssVlanInfo;   /* cpss vlan info format    */
    CPSS_PORTS_BMP_STC              portsMembers;
    CPSS_PORTS_BMP_STC              portsTagging;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC    lttEntry;
    PRV_TGF_L2_MLL_ENTRY_STC        mllEntry;
    PRV_TGF_L2_MLL_PAIR_STC         mllPairEntry;
    CPSS_INTERFACE_INFO_STC         eToPhy;

     /* Save L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationGetAndSave();

    /* AUTODOC: create VLAN, VLAN 3 serves both as VLAN and as eVLAN */
    cpssOsMemSet(&cpssVlanInfo, 0, sizeof(cpssVlanInfo));
    cpssOsMemSet(&portsMembers, 0, sizeof(CPSS_PORTS_BMP_STC));
    cpssOsMemSet(&portsTagging, 0, sizeof(CPSS_PORTS_BMP_STC));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));

    cpssVlanInfo.unknownMacSaCmd = CPSS_PACKET_CMD_FORWARD_E;
    cpssVlanInfo.floodVidx      = 5000;
    cpssVlanInfo.floodVidxMode  = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    cpssVlanInfo.naMsgToCpuEn   = GT_TRUE;

    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum, PRV_TGF_VLANID_3_CNS, &portsMembers, &portsTagging, &cpssVlanInfo, &portsTaggingCmd);
    if( GT_OK != rc)
      return rc;

    rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, PRV_TGF_VLANID_3_CNS, 0, GT_TRUE);
    if( GT_OK != rc)
      return rc;
    rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, PRV_TGF_VLANID_3_CNS, 18, GT_TRUE);
    if( GT_OK != rc)
      return rc;

    /* AUTODOC: create VLAN 4  */
    cpssOsMemSet(&cpssVlanInfo, 0, sizeof(cpssVlanInfo));

    cpssVlanInfo.floodVidx      = 5000; /* not relevant ?*/
    cpssVlanInfo.floodVidxMode  = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    cpssVlanInfo.naMsgToCpuEn   = GT_TRUE;

    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum, PRV_TGF_VLANID_4_CNS, &portsMembers, &portsTagging, &cpssVlanInfo, &portsTaggingCmd);
    if( GT_OK != rc)
      return rc;

    rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, PRV_TGF_VLANID_4_CNS, 18, GT_TRUE);
    if( GT_OK != rc)
      return rc;
    rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, PRV_TGF_VLANID_4_CNS, 0, GT_TRUE);
    if( GT_OK != rc)
      return rc;

    /* AUTODOC: save default value for restore */
    rc = prvTgfBrgVlanPortTranslationEnableGet(prvTgfDevNum, 101, CPSS_DIRECTION_EGRESS_E, &vlanPortTranslationEnableDef);
    if( GT_OK != rc)
      return rc;

    /* AUTODOC: enable egress VLAN translation and configure translation
    from VLAN 3 to VLAN 4 on port 101 */
    rc = prvTgfBrgVlanPortTranslationEnableSet(prvTgfDevNum, 101, CPSS_DIRECTION_EGRESS_E, GT_TRUE);
    if( GT_OK != rc)
      return rc;

    /* AUTODOC: save default value for restore */
    rc = prvTgfBrgVlanTranslationEntryRead(prvTgfDevNum, PRV_TGF_VLANID_3_CNS, CPSS_DIRECTION_EGRESS_E, &vlanTranslationEntryDef);
    if( GT_OK != rc)
      return rc;

    rc = prvTgfBrgVlanTranslationEntryWrite(prvTgfDevNum, PRV_TGF_VLANID_3_CNS, CPSS_DIRECTION_EGRESS_E, PRV_TGF_VLANID_4_CNS);
    if( GT_OK != rc)
      return rc;

    /* AUTODOC: create L2MLL groups*/
    cpssOsMemSet(&lttEntry, 0, sizeof(lttEntry));
    lttEntry.mllPointer = 7;
    lttEntry.mllMaskProfileEnable = GT_FALSE;
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 5000 - 4096, &lttEntry);
    if( GT_OK != rc)
      return rc;

    cpssOsMemSet(&mllEntry, 0, sizeof(mllEntry));
    cpssOsMemSet(&mllPairEntry, 0, sizeof(mllPairEntry));
    mllPairEntry.nextPointer = 0;
    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;


    mllEntry.unknownUcFilterEnable = GT_FALSE;
    mllEntry.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllEntry.egressInterface.devPort.hwDevNum = prvTgfDevNum; /* the eport is on local device*/;
    mllEntry.egressInterface.devPort.portNum = 100;
    mllEntry.ttlThreshold = ttlThreshold;
    mllEntry.bindToMllCounterEnable = GT_TRUE;
    mllEntry.mllCounterIndex = 0;
    mllEntry.last = GT_FALSE;

    mllPairEntry.firstMllNode = mllEntry;

    if( GT_OK != rc)
      return rc;

    mllEntry.ttlThreshold = ttlThreshold;
    mllEntry.egressInterface.devPort.portNum = 101;
    mllEntry.mllCounterIndex = 1;
    mllEntry.last = GT_TRUE;
    mllPairEntry.secondMllNode = mllEntry;

    rc = prvTgfL2MllPairWrite(prvTgfDevNum,7,PRV_TGF_PAIR_READ_WRITE_WHOLE_E,&mllPairEntry);
    if( GT_OK != rc)
      return rc;

    cpssOsMemSet(&eToPhy,0,sizeof(CPSS_INTERFACE_INFO_STC));
    eToPhy.type = CPSS_INTERFACE_PORT_E;
    eToPhy.devPort.hwDevNum = prvTgfDevNum;
    eToPhy.devPort.portNum = 0;
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum, 100, &eToPhy);
    if( GT_OK != rc)
      return rc;
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum, 101, &eToPhy);
    if( GT_OK != rc)
      return rc;

    return GT_OK;
}

/**
* @internal prvTgfTtlExceptionVlanPortTranslationTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfTtlExceptionVlanPortTranslationTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32    expectedTxPacket;

    GT_U32 acount, i;

    PRV_TGF_L2_MLL_EXCEPTION_COUNTERS_STC expcount;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: clear TTL exception counter and skip counter (clear on read) */
    rc = prvTgfL2MllExceptionCountersGet(prvTgfDevNum, &expcount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllExceptionCountersGet");


    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 1 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=1 */

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

     /* set destination MAC address in Packet */
    prvTgfPacketVlanTagPart.vid = PRV_TGF_VLANID_3_CNS;

    /* packet TTL = 0x40 */
    prvTgfPacketIpv4Part.timeToLive = 0X40;

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* send Packet from port 18 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[1]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[1]);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);

    /* AUTODOC: verify Ethernet packet on port 1 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Rx counters - Tx from & from port[1] (the tester) */
        if ( 1 == portIter)
        {
            expectedTxPacket = 1;
        }
        else
        {
            expectedTxPacket = 0;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);
    }


    /* AUTODOC: read (& clear) TTL exception counter */
    rc = prvTgfL2MllExceptionCountersGet(prvTgfDevNum, &expcount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllExceptionCountersGet");

    /* AUTODOC: check TTL exception counter */
    UTF_VERIFY_EQUAL2_STRING_MAC(1, expcount.ttl,
                                 "TTL exception counter[%d] different then expected[%d]\n",
                                 expcount.ttl, 1);

    /* AUTODOC: check TTL exception counter */
    UTF_VERIFY_EQUAL2_STRING_MAC(2, expcount.skip,
                                 "SKIP exception counter[%d] different then expected[%d]\n",
                                 expcount.skip, 2);
    for(i=0; i<2; i++)
    {
        rc = prvTgfL2MllCounterGet(0,i,&acount);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "Counter %d: %d\n", i, acount);
    }

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

}

/**
* @internal prvTgfTtlExceptionVlanPortTranslationConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfTtlExceptionVlanPortTranslationConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    lttIndexArray[1];
    GT_U32    mllIndexArray[1];

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    lttIndexArray[0] = 4;
    mllIndexArray[0] = 7;

    /* AUTODOC: clear L2 MLL and LTT entries */
    prvTgfL2MllUtilsMllAndLttEntriesClear(lttIndexArray, 1, mllIndexArray, 1);

    /* AUTODOC: restore default L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationRestore();

    /* invalidate VLAN 2 entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_3_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate 2");

     /* invalidate VLAN 3 entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_4_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate 3");


   /* AUTODOC: restore default egress VLAN translation and configuration translation
    for VLAN 1 on port 101 */
    rc = prvTgfBrgVlanPortTranslationEnableSet(prvTgfDevNum, 101, CPSS_DIRECTION_EGRESS_E, vlanPortTranslationEnableDef);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortTranslationEnableSet");

    rc = prvTgfBrgVlanTranslationEntryWrite(prvTgfDevNum, PRV_TGF_VLANID_3_CNS, CPSS_DIRECTION_EGRESS_E, vlanTranslationEntryDef);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTranslationEntryWrite");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: clear Rx CPU table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: stop Rx CPU capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");
}


