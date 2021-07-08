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
* @file prvTgfMaskProfileSkipAndCounters.c
*
* @brief L2 MLL Mask Profile, Skip counter & MLL counters test
*
* @version   7
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfL2MllGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>

#include <l2mll/prvTgfL2MllUtils.h>
#include <l2mll/prvTgfMaskProfileSkipAndCounters.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_VLANID_CNS  1

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     0

/* number of MLL counters */
#define PRV_TGF_MLL_COUNTERS_NUM_CNS    3

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x01, 0x00, 0x00, 0x00, 0x00, 0x01},               /* dstMac */
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

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/


/**
* @internal prvTgfMaskProfileSkipAndCountersConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfMaskProfileSkipAndCountersConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc;

    PRV_TGF_PAIR_READ_WRITE_FORM_ENT     mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC              mllPairEntry;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* Save L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationGetAndSave();

    /* AUTODOC: enable MLL lookup for all multi-target packets */
    rc = prvTgfL2MllLookupForAllEvidxEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupForAllEvidxEnableSet: enable %d",
                                 GT_TRUE);

    /* AUTODOC: add FDB entry with MAC 01:00:00:00:00:01, VLAN 1, eVIDX 1 */
    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfPacketL2Part.daMac, PRV_TGF_VLANID_CNS,
                                          1, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidxSet: eVidx 1");

    /* create L2 MLL entries */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(PRV_TGF_L2_MLL_PAIR_STC));

    mllPairWriteForm=PRV_TGF_PAIR_READ_WRITE_WHOLE_E;
    mllPairEntry.nextPointer = 1;
    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    /* AUTODOC:   port=1 , maskBitmap=0x1, mllCntrIdx=0 */
    mllPairEntry.firstMllNode.last = GT_FALSE;
    mllPairEntry.firstMllNode.maskBitmap = 0x1;
    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.firstMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[1];
    mllPairEntry.firstMllNode.bindToMllCounterEnable = GT_TRUE;
    mllPairEntry.firstMllNode.mllCounterIndex = 0;


    /* AUTODOC:   port=2, maskBitmap=0x2, mllCntrIdx=1 */
    mllPairEntry.secondMllNode.last = GT_FALSE;
    mllPairEntry.secondMllNode.maskBitmap = 0x2;
    mllPairEntry.secondMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.secondMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[2];
    mllPairEntry.secondMllNode.bindToMllCounterEnable = GT_TRUE;
    mllPairEntry.secondMllNode.mllCounterIndex = 1;

    /* AUTODOC: add L2 MLL entry 0: */
    /* AUTODOC:   nextPointer=1 */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 0, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 0");

    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E;
    mllPairEntry.nextPointer=0;
    mllPairEntry.entrySelector=PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    mllPairEntry.firstMllNode.last = GT_TRUE;
    mllPairEntry.firstMllNode.maskBitmap = 0x4000;
    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.firstMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[3];
    mllPairEntry.firstMllNode.bindToMllCounterEnable = GT_TRUE;
    mllPairEntry.firstMllNode.mllCounterIndex = 2;

    /* AUTODOC: add L2 MLL entry 1: */
    /* AUTODOC:   nextPointer=0, port=3 */
    /* AUTODOC:   maskBitmap=0x4000, mllCntrIdx=2 */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 1, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 1");

    rc = prvTgfL2MllPairRead(prvTgfDevNum, 1, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairRead: index 1");

    mllPairEntry.firstMllNode.ttlThreshold = 0x0;

    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 1, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 1");
}

/**
* @internal prvTgfReadMllCountersAndVerifyClear function
* @endinternal
*
* @brief   Get MLL counters values and verified they are cleared after read.
*
* @param[out] countersArr              - (pointer to) array which contains the MLL counters values.
*                                       None
*/
static GT_VOID prvTgfReadMllCountersAndVerifyClear
(
    OUT GT_U32 *countersArr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32  i;
    GT_U32  tempCounter;

    for( i = 0; i < PRV_TGF_MLL_COUNTERS_NUM_CNS ; i++ )
    {
        rc = prvTgfL2MllCounterGet(prvTgfDevNum, i, countersArr + i);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllCounterGet: index %d", i);

        rc = prvTgfL2MllCounterGet(prvTgfDevNum, i, &tempCounter);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllCounterGet: index %d", i);

        /* check counter is cleared */
        UTF_VERIFY_EQUAL1_STRING_MAC(0, tempCounter, "L2 MLL counter %d is not clear "
                                     "as expected", i);
    }
}

/**
* @internal prvTgfMaskProfileSkipAndCountersTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfMaskProfileSkipAndCountersTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32    expectedTxPacket;
    PRV_TGF_L2_MLL_EXCEPTION_COUNTERS_STC  counters;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC   lttEntry;

    PRV_TGF_PAIR_READ_WRITE_FORM_ENT     mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC              mllPairEntry;

    GT_U32  mllCounters[PRV_TGF_MLL_COUNTERS_NUM_CNS];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* Profile mask enable, bit 0 */

    /* create L2 MLL LTT entry */
    cpssOsMemSet(&lttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

    lttEntry.mllPointer = 0;
    lttEntry.entrySelector = 0;
    lttEntry.mllMaskProfileEnable = GT_TRUE;
    lttEntry.mllMaskProfile = 0;

    /* AUTODOC: add L2 MLL LTT entry 1 with: */
    /* AUTODOC:   mllPointer=0, mllMaskProfileEnable=True, mllMaskProfile=0 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 1, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 1");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* clear Skip exception counter */
    rc = prvTgfL2MllExceptionCountersGet(prvTgfDevNum, &counters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllExceptionCountersGet");

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    prvTgfReadMllCountersAndVerifyClear(mllCounters);

    /* AUTODOC: verify Ethernet packet on ports 0,1 */
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
            case 1: expectedTxPacket = 1;
                    break;
            default: expectedTxPacket = 0;
                     break;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);

        if( portIter != PRV_TGF_SEND_PORT_IDX_CNS )
        {
                /* AUTODOC: verify MLL counters=1 on port 0 */
            UTF_VERIFY_EQUAL1_STRING_MAC(expectedTxPacket, mllCounters[portIter-1],
                          "MLL counter different then expected for counter %d", portIter-1);
        }
    }

    /* read (& clear) Skip exception counter */
    rc = prvTgfL2MllExceptionCountersGet(prvTgfDevNum, &counters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllExceptionCountersGet");

    /* AUTODOC: verify L2 MLL exception Skip counter=2 */
    UTF_VERIFY_EQUAL0_STRING_MAC(2, counters.skip,
                                 "Skip counter different then expected");

    /* Profile mask enable, bit 1 */

    /* create L2 MLL LTT entry */
    cpssOsMemSet(&lttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

    lttEntry.mllPointer = 0;
    lttEntry.entrySelector = 0;
    lttEntry.mllMaskProfileEnable = GT_TRUE;
    lttEntry.mllMaskProfile = 1;

    /* AUTODOC: add L2 MLL LTT entry 1 with: */
    /* AUTODOC:   mllPointer=0, mllMaskProfileEnable=True, mllMaskProfile=1 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 1, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 1");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    prvTgfReadMllCountersAndVerifyClear(mllCounters);

    /* AUTODOC: verify Ethernet packet on ports 0,2 */
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
            case 2: expectedTxPacket = 1;
                    break;
            default: expectedTxPacket = 0;
                     break;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);

        if( portIter != PRV_TGF_SEND_PORT_IDX_CNS )
        {
                /* AUTODOC: verify MLL counters=1 on port 1 */
            UTF_VERIFY_EQUAL1_STRING_MAC(expectedTxPacket, mllCounters[portIter-1],
                          "MLL counter different then expected for counter %d", portIter-1);
        }
    }

    /* read (& clear) Skip exception counter */
    rc = prvTgfL2MllExceptionCountersGet(prvTgfDevNum, &counters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllExceptionCountersGet");

    /* AUTODOC: verify L2 MLL exception Skip counter=2 */
    UTF_VERIFY_EQUAL0_STRING_MAC(2, counters.skip,
                                 "Skip counter different then expected");

    /* Profile mask enable, bit 14 */

    /* create L2 MLL LTT entry */
    cpssOsMemSet(&lttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

    lttEntry.mllPointer = 0;
    lttEntry.entrySelector = 0;
    lttEntry.mllMaskProfileEnable = GT_TRUE;
    lttEntry.mllMaskProfile = 14;

    /* AUTODOC: add L2 MLL LTT entry 1 with: */
    /* AUTODOC:   mllPointer=0, mllMaskProfileEnable=True, mllMaskProfile=14 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 1, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 1");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    prvTgfReadMllCountersAndVerifyClear(mllCounters);

    /* AUTODOC: verify Ethernet packet on ports 0,3 */
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
            case 3: expectedTxPacket = 1;
                    break;
            default: expectedTxPacket = 0;
                     break;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);

        if( portIter != PRV_TGF_SEND_PORT_IDX_CNS )
        {
                /* AUTODOC: verify MLL counters=1 on port 2 */
            UTF_VERIFY_EQUAL1_STRING_MAC(expectedTxPacket, mllCounters[portIter-1],
                          "MLL counter different then expected for counter %d", portIter-1);
        }
    }

    /* read (& clear) Skip exception counter */
    rc = prvTgfL2MllExceptionCountersGet(prvTgfDevNum, &counters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllExceptionCountersGet");

    /* AUTODOC: verify L2 MLL exception Skip counter=2 */
    UTF_VERIFY_EQUAL0_STRING_MAC(2, counters.skip,
                                 "Skip counter different then expected");


    /* Profile mask disable */

    /* create L2 MLL LTT entry */
    cpssOsMemSet(&lttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

    lttEntry.mllPointer = 0;
    lttEntry.entrySelector = 0;
    lttEntry.mllMaskProfileEnable = GT_FALSE;
    lttEntry.mllMaskProfile = 0;

    /* AUTODOC: add L2 MLL LTT entry 1 with: */
    /* AUTODOC:   mllPointer=0, mllMaskProfileEnable=False, mllMaskProfile=0 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 1, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 1");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    prvTgfReadMllCountersAndVerifyClear(mllCounters);

    /* AUTODOC: verify Ethernet packet on all ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        UTF_VERIFY_EQUAL2_STRING_MAC(1, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);

        if( portIter != PRV_TGF_SEND_PORT_IDX_CNS )
        {
                /* AUTODOC: verify MLL counters=1 on ports [0,1,2] */
            UTF_VERIFY_EQUAL1_STRING_MAC(1, mllCounters[portIter-1],
                          "MLL counter different then expected for counter %d", portIter-1);
        }
    }

    /* read (& clear) Skip exception counter */
    rc = prvTgfL2MllExceptionCountersGet(prvTgfDevNum, &counters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllExceptionCountersGet");

    /* AUTODOC: verify L2 MLL exception Skip counter=0 */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, counters.skip,
                                 "Skip counter different then expected");

    /* unbind MLL entry 2 from MLL counter 2 */

    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E;

    rc = prvTgfL2MllPairRead(prvTgfDevNum, 1, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairRead: index 1");

    mllPairEntry.firstMllNode.bindToMllCounterEnable = GT_FALSE;

    /* AUTODOC: update L2 MLL entry 1: */
    /* AUTODOC:   bindToMllCounterEnable=False */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 1, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 0");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    prvTgfReadMllCountersAndVerifyClear(mllCounters);

    /* AUTODOC: verify Ethernet packet on all ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        UTF_VERIFY_EQUAL2_STRING_MAC(1, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);

        if( portIter != PRV_TGF_SEND_PORT_IDX_CNS )
        {
                /* AUTODOC: verify MLL counters=1 on ports [0,1] */
            UTF_VERIFY_EQUAL1_STRING_MAC(((portIter < 3)?1:0), mllCounters[portIter-1],
                          "MLL counter different then expected for counter %d", portIter-1);
        }
    }

    /* read (& clear) Skip exception counter */
    rc = prvTgfL2MllExceptionCountersGet(prvTgfDevNum, &counters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllExceptionCountersGet");

    /* AUTODOC: verify L2 MLL exception Skip counter=0 */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, counters.skip,
                                 "Skip counter different then expected");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();
}

/**
* @internal prvTgfMaskProfileSkipAndCountersConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfMaskProfileSkipAndCountersConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    lttIndexArray[1];
    GT_U32    mllIndexArray[3];

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    lttIndexArray[0] = 0;
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
}


