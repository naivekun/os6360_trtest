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
* @file prvTgfTransmitAllKindsOfTraffic.c
*
* @brief Transmit All kinds of traffic from all port
*
* @version   9
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <bridge/prvTgfTransmitAllKindsOfTraffic.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS               3

/* port index to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS        0

/* VDIX to send traffic to */
#define PRV_TGF_SEND_VDIX_CNS            1

/* default number of packets to send */
static GT_U32       prvTgfBurstCount   = 1;

/* number of members of MC Group */
static GT_U8        prvTgfMcMembersNum = 2;

/* array of source MACs for the tests */
static TGF_MAC_ADDR prvTgfSaMacArr[] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x88},
                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x88},
                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x88},
                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x88}
                                       };
/* array of destination MACs for the ports */
static TGF_MAC_ADDR prvTgfDaMacArr[] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x11},
                                        {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                        {0x01, 0x02, 0x03, 0x04, 0x05, 0x06}
                                       };
/* array of destination MACs for the FDB entries */
static TGF_MAC_ADDR prvTgfFdbMacArr[] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x11},
                                         {0x01, 0x02, 0x03, 0x04, 0x05, 0x06}
                                        };

/* array of expected pass of packet to the each port in each test */
static GT_BOOL prvTgfExpPacketPassArr[][PRV_TGF_PORTS_NUM_CNS] = {
    { GT_TRUE,  GT_TRUE,  GT_TRUE,  GT_TRUE},
    {GT_FALSE,  GT_TRUE,  GT_TRUE, GT_FALSE},
    { GT_TRUE,  GT_TRUE,  GT_TRUE,  GT_TRUE},
    { GT_TRUE,  GT_TRUE, GT_FALSE,  GT_TRUE}
};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                /* srcMac */
};
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};
/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x81, 0x00, 0x00, 0x02,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
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

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    GT_U16      vid;
} prvTgfRestoreCfg;

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBrgTransmitAllConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgTransmitAllConfigurationSet
(
    GT_VOID
)
{
    GT_U32                      fdbIter        = 0;
    GT_U32                      fdbCount       = sizeof(prvTgfFdbMacArr) / sizeof(prvTgfFdbMacArr[0]);
    GT_STATUS                   rc             = GT_OK;
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;
    PRV_TGF_MAC_ENTRY_KEY_STC   macEntryKey;


    /* AUTODOC: SETUP CONFIGURATION: */
    /* 1. Create vlan 3 on ports (0,0), (0,8), (0,18), (0,23)
     * 2. Test GT_ERROR for setting FDB MAC Table
     * 3. Create a macEntry with PORT interface
     * 4. Create MC Group and macEntry with VIDX interface
     * 5. Check FDB entries
     */

    /* AUTODOC: create VLAN 3 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* save default vlanId for restore */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfRestoreCfg.vid);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfRestoreCfg.vid);

    /* AUTODOC: set PVID to 3 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_VLANID_CNS);

    /* 2. Check GT_ERROR | GT_OUT_OF_RANGE for unknown VDIX 0 (4094 is last right) MC Group Table */
    rc = prvTgfBrgMcMemberAdd(prvTgfDevNum, 0, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgMcMemberAdd: %d, %d, %d",
                                 prvTgfDevNum, 0, prvTgfPortsArray[0]);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:11, VLAN 3, port 2 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfFdbMacArr[0], PRV_TGF_VLANID_CNS,
                                          prvTgfDevNum, prvTgfPortsArray[2], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);


    /* AUTODOC: create VIDX 1 with ports 0, 1 */
    rc = prvTgfBrgVidxEntrySet(PRV_TGF_SEND_VDIX_CNS, prvTgfPortsArray,
                               NULL, prvTgfMcMembersNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VDIX_CNS);

    /* AUTODOC: add FDB entry with MAC 01:02:03:04:05:06, VLAN 3, VIDX 1 */
    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfFdbMacArr[1], PRV_TGF_VLANID_CNS,
                                          PRV_TGF_SEND_VDIX_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidxSet: %d", prvTgfDevNum);

    /* AUTODOC: verify FDB table contains only added entries */
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;

    for (fdbIter = 0; fdbIter < fdbCount; fdbIter++)
    {
        cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, prvTgfFdbMacArr[fdbIter], sizeof(TGF_MAC_ADDR));
        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

        /* get FDB entry */
        rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);

        /* compare entryType */
        rc = (macEntry.key.entryType == PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E) ? GT_OK : GT_FAIL;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "vlanId: %d", macEntry.key.entryType);

        if (GT_OK == rc)
        {
            /* compare MAC address */
            rc = cpssOsMemCmp(macEntry.key.key.macVlan.macAddr.arEther,
                              macEntryKey.key.macVlan.macAddr.arEther,
                              sizeof(TGF_MAC_ADDR)) == 0 ? GT_OK : GT_FAIL;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "MAC address[0]: %d",
                                         macEntry.key.key.macVlan.macAddr.arEther[0]);

            /* compare vlanId */
            rc = (macEntry.key.key.macVlan.vlanId == macEntryKey.key.macVlan.vlanId) ? GT_OK : GT_FAIL;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "vlanId: %d", macEntryKey.key.macVlan.vlanId);
        }

        /* compare dstInterface.type */
        rc = ((macEntry.dstInterface.type == CPSS_INTERFACE_PORT_E) ||
             (macEntry.dstInterface.type == CPSS_INTERFACE_VIDX_E)) ? GT_OK : GT_FAIL;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "dstInterface.type: %d", macEntry.dstInterface.type);

        /* compare dstInterface */
        if (rc == GT_OK)
        {
            switch (macEntry.dstInterface.type)
            {
                case CPSS_INTERFACE_PORT_E:
                    rc = (macEntry.dstInterface.devPort.hwDevNum == prvTgfDevNum) ? GT_OK : GT_FAIL;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "dstInterface.devPort.hwDevNum: %d",
                                                 macEntry.dstInterface.devPort.hwDevNum);

                    rc = (macEntry.dstInterface.devPort.portNum == prvTgfPortsArray[2]) ? GT_OK : GT_FAIL;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "dstInterface.devPort.portNum: %d",
                                                 macEntry.dstInterface.devPort.portNum);
                    break;

                case CPSS_INTERFACE_VIDX_E:
                    rc = (macEntry.dstInterface.vidx == PRV_TGF_SEND_VDIX_CNS) ? GT_OK : GT_FAIL;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "dstInterface.vidx: %d",
                                                 macEntry.dstInterface.vidx);
                    break;

                default:
                    rc = GT_BAD_PARAM;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "not expected interface: %d",
                                                 macEntry.dstInterface.type);
            }
        }
    }
}

/**
* @internal prvTgfBrgTransmitAllTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgTransmitAllTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS       rc           = GT_OK;
    GT_U32          portsCount   = prvTgfPortsNum;
    GT_U32          portIter     = 0;
    GT_U32          ucPortIter   = 0;

    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[portIter]);

    /* -------------------------------------------------------------------------
     * send unicast packets from each port to all another ports
     */
    PRV_UTF_LOG0_MAC("======= Sending unicast packets to check FDB table =======\n");

    /* AUTODOC: send packet with mac DA 00:00:00:00:00:01 from port 0 (unknown unicast) */
    /* AUTODOC: verify packet arrives to ports 1,2,3 */
    /* AUTODOC: send packet with mac DA 00:00:00:00:00:11 from port 1 (known unicast to port) */
    /* AUTODOC: verify packet arrives to ports 2 */
    /* AUTODOC: send packet with mac DA FF:FF:FF:FF:FF:FF from port 2 (broadcast) */
    /* AUTODOC: verify packet arrives to ports 0,1,3 */
    /* AUTODOC: send packet with mac DA 01:02:03:04:05:06 from port 3 (known unicast to VIDX) */
    /* AUTODOC: verify packet arrives to ports 0,1 */
    for (ucPortIter = 0; ucPortIter < portsCount; ucPortIter++)
    {
        /* reset counters */
        for (portIter = 0; portIter < portsCount; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        PRV_UTF_LOG1_MAC("------- Sending packets from port %d -------\n", prvTgfPortsArray[ucPortIter]);
        /* set destination MAC address in Packet */
        cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfDaMacArr[ucPortIter],
                     sizeof(prvTgfPacketL2Part.daMac));

        /* set source MAC address in Packet */
        cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfSaMacArr[ucPortIter],
                     sizeof(prvTgfPacketL2Part.saMac));

        /* send Packet */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[ucPortIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[ucPortIter]);

        /* read and check counters */
        for (portIter = 0; portIter < portsCount; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            /* check Tx counters */
            if (prvTgfExpPacketPassArr[ucPortIter][portIter] == GT_TRUE)
            {
                if (ucPortIter == portIter)
                {
                    UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_PACKET_CRC_LEN_CNS) * prvTgfBurstCount,
                                                 portCntrs.goodOctetsSent.l[0],
                                                 "get another goodOctetsSent counter than expected");
                }
                else
                {
                    UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_PACKET_CRC_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS) * prvTgfBurstCount,
                                                 portCntrs.goodOctetsSent.l[0],
                                                 "get another goodOctetsSent counter than expected");
                }
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                             "get another goodPktsSent counter than expected");

                if (prvTgfDaMacArr[ucPortIter][0] == 0xFF)
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.brdcPktsSent.l[0],
                                                 "get another brdcPktsSent counter than expected");
                else if (prvTgfDaMacArr[ucPortIter][0] == 0x01)
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.mcPktsSent.l[0],
                                                 "get another mcPktsSent counter than expected");
                else
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.ucPktsSent.l[0],
                                                 "get another ucPktsSent counter than expected");
            }
            else
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodOctetsSent.l[0],
                                             "get another goodOctetsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                             "get another goodPktsSent counter than expected");

                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.brdcPktsSent.l[0],
                                             "get another brdcPktsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.mcPktsSent.l[0],
                                             "get another mcPktsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsSent.l[0],
                                             "get another ucPktsSent counter than expected");
            }

            if (ucPortIter == portIter)
            {
                /* check Rx counters for sending port */
                UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_PACKET_CRC_LEN_CNS) * prvTgfBurstCount,
                                             portCntrs.goodOctetsRcv.l[0],
                                             "get another goodOctetsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                             "get another goodPktsRcv counter than expected");

                if (prvTgfDaMacArr[ucPortIter][0] == 0xFF)
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.brdcPktsRcv.l[0],
                                                 "get another brdcPktsRcv counter than expected");
                else if (prvTgfDaMacArr[ucPortIter][0] == 0x01)
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.mcPktsRcv.l[0],
                                                 "get another mcPktsRcv counter than expected");
                else
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.ucPktsRcv.l[0],
                                                 "get another ucPktsRcv counter than expected");
            }
            else
            {
                /* check Rx counters for other ports */
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodOctetsRcv.l[0],
                                             "get another goodOctetsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsRcv.l[0],
                                             "get another goodPktsRcv counter than expected");

                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.brdcPktsRcv.l[0],
                                             "get another brdcPktsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.mcPktsRcv.l[0],
                                             "get another mcPktsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsRcv.l[0],
                                             "get another ucPktsRcv counter than expected");
            }
        }
    }
}

/**
* @internal prvTgfBrgTransmitAllConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgTransmitAllConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;


    if(prvTgfResetModeGet() == GT_TRUE)
    {
        /* skip CPSS/HW restoration specific for the test and run full system reset and initialization */
        prvTgfResetAndInitSystem();
    }
    else
    {
        /* AUTODOC: RESTORE CONFIGURATION: */

        /* flush FDB include static entries */
        rc = prvTgfBrgFdbFlush(GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

        /* AUTODOC: clear VIDX 1 */
        rc = prvTgfBrgVidxEntrySet(PRV_TGF_SEND_VDIX_CNS, NULL, NULL, 0);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d", prvTgfDevNum);

        /* AUTODOC: clear VIDX 0 */
        rc = prvTgfBrgVidxEntrySet(0, NULL, NULL, 0);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d", prvTgfDevNum);

        /* AUTODOC: restore default vlanId to all ports */
        rc = prvTgfBrgVlanPvidSet(prvTgfRestoreCfg.vid);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                     prvTgfDevNum, prvTgfRestoreCfg.vid);

        /* invalidate vlan entry (and reset vlan entry) */
        rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, PRV_TGF_VLANID_CNS);
    }
}

/**
* @internal prvTgfBrgCrcHashCalcModeTest function
* @endinternal
*
* @brief   Test CRC mode hash calculation
*/
GT_VOID prvTgfBrgCrcHashCalcModeTest
(
    GT_VOID
)
{
    GT_U32      portIter     = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    PRV_TGF_MAC_HASH_FUNC_MODE_ENT   prvTgfHashModeGet;
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: get current  HASH mode */
    rc =  prvTgfBrgFdbHashModeGet(prvTgfDevNum, &prvTgfHashModeGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbHashModeGet");

    /* AUTODOC: set CRC hash mode */
    rc =  prvTgfBrgFdbHashModeSet(prvTgfDevNum,
        PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) ?
        PRV_TGF_MAC_HASH_FUNC_CRC_MULTI_HASH_E : /* sip6 not supports 'CRC/XOR' . supports only 'multi-hash' */
        PRV_TGF_MAC_HASH_FUNC_CRC_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbHashModeSet");

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:11 VLAN 3, port 1 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfFdbMacArr[0], PRV_TGF_VLANID_CNS,
                                          prvTgfDevNum, prvTgfPortsArray[1], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);


    /* AUTODOC: GENERATE TRAFFIC: */

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo,  prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[portIter]);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }


    PRV_UTF_LOG1_MAC("------- Sending packets from port %d -------\n", prvTgfPortsArray[0]);
    /* set destination MAC address in Packet */
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfDaMacArr[1],
                 sizeof(prvTgfPacketL2Part.daMac));

    /* set source MAC address in Packet */
    cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfSaMacArr[0],
                 sizeof(prvTgfPacketL2Part.saMac));

    /* send Packet from port 0 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[0]);

    /* read and check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        /* check Tx counters */
        if (portIter == 0)
        {
            continue;
        }

        /* check counters */
        if (portIter == 1)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_PACKET_CRC_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS) * prvTgfBurstCount,
                                         portCntrs.goodOctetsSent.l[0],
                                         "get another goodOctetsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.ucPktsSent.l[0],
                                         "get another ucPktsSent counter than expected");

            /* check Rx counters for recieved port */
            UTF_VERIFY_EQUAL0_STRING_MAC(0,
                                         portCntrs.goodOctetsRcv.l[0],
                                         "get another goodOctetsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsRcv.l[0],
                                         "get another ucPktsRcv counter than expected");

        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodOctetsSent.l[0],
                                         "get another goodOctetsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsSent.l[0],
                                         "get another ucPktsSent counter than expected");

            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodOctetsRcv.l[0],
                                         "get another goodOctetsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsRcv.l[0],
                                         "get another ucPktsRcv counter than expected");
        }

    }

    /* AUTODOC: restore previous HASH mode */
    rc =  prvTgfBrgFdbHashModeSet(prvTgfDevNum,prvTgfHashModeGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbHashModeSet");
}

