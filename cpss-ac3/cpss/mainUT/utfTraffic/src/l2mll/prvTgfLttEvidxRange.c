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
* @file prvTgfLttEvidxRange.c
*
* @brief L2 MLL LTT eVIDX range test
*
* @version   5
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

#include <l2mll/prvTgfL2MllUtils.h>
#include <l2mll/prvTgfLttEvidxRange.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_VLANID_CNS  1

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     0

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

/* array of destination MACs for the FDB entries */
static TGF_MAC_ADDR prvTgfFdbMacArr[] = {{0x01, 0x00, 0x00, 0x00, 0x00, 0x01},
                                         {0x01, 0x00, 0x00, 0x00, 0x00, 0x02}
                                        };

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
static  L2_MLL_ENTRY_SHORT_INFO_STC     testMllDb[] =
{
    /* first list (2 in the same line)*/
    /* pointer from LTT index 0 (start mll 0) */
    {TEST_INDEX_BASE_MAC(0,0) ,      0,    GT_FALSE,   CPSS_INTERFACE_PORT_E,2},
    {TEST_INDEX_BASE_MAC(0,0),       0,    GT_TRUE,    CPSS_INTERFACE_PORT_E,2},

    /* second list (the second entry in 'odd' index) */
    /* pointer from LTT index 1 (start mll 111)*/
    {TEST_INDEX_BASE_MAC(1,111) , 0,    GT_FALSE,   CPSS_INTERFACE_PORT_E,2},
    {TEST_INDEX_BASE_MAC(1,111),  0,    GT_TRUE,    CPSS_INTERFACE_PORT_E,2},

    /* third list (the second entry in 'even' index) */
    /* pointer from LTT index 0x1000 (start mll 333)*/
    {TEST_INDEX_BASE_MAC(0x1000,333) , 0,    GT_FALSE,  CPSS_INTERFACE_PORT_E,3},
    {TEST_INDEX_BASE_MAC(0x1000,333),  0,    GT_TRUE,   CPSS_INTERFACE_PORT_E,3},

    {NOT_VALID_ENTRY_CNS,              0,          0,   0 , 0}/*must be last*/
};
#define  TEST_MLL_DB_NUM_ENTRIES_CNS    ((sizeof(testMllDb))/(sizeof(testMllDb[0])))

static  GT_U32     testMllDb_restoreInfo[TEST_MLL_DB_NUM_ENTRIES_CNS] =
{
    0
    /* filled in runtime according to actual MLL indexes used */
};

/**
* @internal prvTgfLttEvidxRangeConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfLttEvidxRangeConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32                          portsArray[1];

    /* AUTODOC: SETUP CONFIGURATION: */

    /* Save L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationGetAndSave();

    /* create eVidx 1 */
    portsArray[0] = prvTgfPortsArray[1];

    /* AUTODOC: create VIDX 1 with ports [1] */
    rc = prvTgfBrgVidxEntrySet(1, portsArray, NULL, 1);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d, %d",
                                 prvTgfDevNum, 1);

    /* AUTODOC: add FDB entry with MAC 01:00:00:00:00:01, VLAN 1, VIDX 1 */
    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfFdbMacArr[0], PRV_TGF_VLANID_CNS,
                                          1, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidxSet: eVidx 1");

    /* AUTODOC: add FDB entry with MAC 01:00:00:00:00:02, VLAN 1, VIDX 0x1000 */
    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfFdbMacArr[1], PRV_TGF_VLANID_CNS,
                                          4096, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidxSet: eVidx 4096");

    /* AUTODOC: set LTT and L2 MLL entries needed by the test */
    prvTgfL2MllLConfigSet(testMllDb,testMllDb_restoreInfo,
        MLL_INDEX_TEST_MODE_REFERENCE_FROM_END_OF_TALBE_E);

}

/**
* @internal prvTgfLttEvidxRangeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfLttEvidxRangeTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32    expectedTxPacket;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: disable MLL lookup for all multi-target packets */
    rc = prvTgfL2MllLookupForAllEvidxEnableSet(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupForAllEvidxEnableSet: enable %d",
                                 GT_FALSE);

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* set destination MAC address in Packet */
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfFdbMacArr[0], sizeof(prvTgfPacketL2Part.daMac));

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC: verify Ethernet packet on port 1 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Rx counters - Tx from port[1] (based on eVidx 1 port members) & from port[0] (the tester) */
        if (1 == portIter || PRV_TGF_SEND_PORT_IDX_CNS == portIter)
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

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* set destination MAC address in Packet */
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfFdbMacArr[1], sizeof(prvTgfPacketL2Part.daMac));

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:02, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC: verify Ethernet packet on port 2 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Rx counters - Tx from port[2] (based on eVidx 4096 MLL(2 length linked list), LTT at index 0) */
        /*                     & from port[0] (the tester) */
        switch(portIter)
        {
            case PRV_TGF_SEND_PORT_IDX_CNS:
                expectedTxPacket = 1;
                break;
            case 2:
                expectedTxPacket = 2;
                break;
            default:
                expectedTxPacket = 0;
                break;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);
    }

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: enable MLL lookup for all multi-target packets */
    rc = prvTgfL2MllLookupForAllEvidxEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupForAllEvidxEnableSet: enable %d",
                                 GT_TRUE);

    /* set destination MAC address in Packet */
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfFdbMacArr[0], sizeof(prvTgfPacketL2Part.daMac));

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC: verify Ethernet packet on port 2 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Rx counters - Tx from port[2] (based on eVidx 1 MLL(2 length linked list), LTT at index 1) */
        /*                     & from port[0] (the tester) */
        switch(portIter)
        {
            case PRV_TGF_SEND_PORT_IDX_CNS:
                expectedTxPacket = 1;
                break;
            case 2:
                expectedTxPacket = 2;
                break;
            default:
                expectedTxPacket = 0;
                break;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);
    }

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* set destination MAC address in Packet */
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfFdbMacArr[1], sizeof(prvTgfPacketL2Part.daMac));

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:02, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC: verify Ethernet packet on port 3 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Rx counters - Tx from port[3] (based on eVidx 4096 MLL(2 length linked list), LTT at index 4096) */
        /*                     & from port[0] (the tester) */
        switch(portIter)
        {
            case PRV_TGF_SEND_PORT_IDX_CNS: expectedTxPacket = 1;
                                            break;
            case 3: expectedTxPacket = 2;
                    break;
            default: expectedTxPacket = 0;
                     break;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: Set the maximal VIDX value that refers to a port distribution list(VIDX).
      to be 0 instead of the default that is 4095. */

    /* by setting maxVidxIndex=0 and prvTgfL2MllLookupForAllEvidxEnableSet=GT_FALSE MLL lookup is
       performed only for multi-target packets with eVIDX >= 0 and not as before eVIDX >= 4095 */

    rc = prvTgfL2MllLookupMaxVidxIndexSet(prvTgfDevNum, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupMaxVidxIndexSet: maxVidxIndex %d",
                                 0);

    /* AUTODOC: disable MLL lookup for all multi-target packets */
    rc = prvTgfL2MllLookupForAllEvidxEnableSet(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupForAllEvidxEnableSet: enable %d",
                                 GT_FALSE);

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* set destination MAC address in Packet */
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfFdbMacArr[0], sizeof(prvTgfPacketL2Part.daMac));

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC: verify Ethernet packet on port 2 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Rx counters - Tx from port[2] (based on eVidx 1 MLL(2 length linked list), LTT at index 1) */
        /*                     & from port[0] (the tester) */
        switch(portIter)
        {
            case PRV_TGF_SEND_PORT_IDX_CNS:
                expectedTxPacket = 1;
                break;
            case 2:
                expectedTxPacket = 2;
                break;
            default:
                expectedTxPacket = 0;
                break;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);
    }

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* set destination MAC address in Packet */
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfFdbMacArr[1], sizeof(prvTgfPacketL2Part.daMac));

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:02, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC: verify Ethernet packet on port 3 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Rx counters - Tx from port[2] (based on eVidx 4096 MLL(2 length linked list), LTT at index 0) */
        /*                     & from port[0] (the tester) */
        switch(portIter)
        {
            case PRV_TGF_SEND_PORT_IDX_CNS:
                expectedTxPacket = 1;
                break;
            case 2:
                expectedTxPacket = 2;
                break;
            default:
                expectedTxPacket = 0;
                break;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);
    }

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();
}

/**
* @internal prvTgfLttEvidxRangeConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfLttEvidxRangeConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: clear eVidx 1 */
    rc = prvTgfBrgVidxEntrySet(1, NULL, NULL, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d, %d",
                                 prvTgfDevNum, 1);

    /* AUTODOC: 'restore' : clear LTT and L2 MLL entries that were set by the test */
    prvTgfL2MllLConfigReset(testMllDb,testMllDb_restoreInfo);

    /* AUTODOC: restore default L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationRestore();

}


