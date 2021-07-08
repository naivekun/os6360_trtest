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
* @file prvTgfFdbPerEgressPortUnregIpv4McFiltering.c
*
* @brief Verify per-Egress port filtering for unregistered IPv4 MC packets.
*
* @version   5
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTrunkGen.h>

#include <bridge/prvTgfFdbPerEgressPortUnregIpv4McFiltering.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* capture port index */
#define PRV_TGF_CPORT_IDX_CNS               1

/* default vlanId */
#define PRV_TGF_VLAN_ID_CNS                 2

/* default FDB portIdx */
#define PRV_TGF_FDB_PORT_IDX_CNS            1

/* portIdx to send traffic to */
#define PRV_TGF_TX_PORT_IDX_CNS             0

/* default number of packets to send */
static GT_U32  prvTgfBurstCount           = 10;

/* default traffic Tx delay in mS */
#define PRV_TGF_TX_DELAY_CNS                100

/* total tests stages */
#define PRV_TGF_STAGE_NUM_CNS               4

/* expected number of Rx and Tx packets on ports*/
static GT_U8 prvTgfPacketsCountRxTxArr[][PRV_TGF_PORTS_NUM_CNS] =
{
    {1, 0, 0, 0},  /* Rx count for unfiltered unregistered MC packets       */
    {1, 1, 1, 1},  /* Tx count for unfiltered unregistered MC packets       */
    {1, 0, 0, 0},  /* Rx count for filtered unregistered MC packets         */
    {1, 0, 0, 0},  /* Tx count for filtered unregistered MC packets         */
    {1, 0, 0, 0},  /* Rx count for filtered known MC packets                */
    {1, 1, 1, 1},  /* Tx count for filtered known MC packets                */
    {1, 0, 0, 0},  /* Rx count for unfiltered unregistered MC packets       */
    {1, 1, 1, 1},  /* Tx count for unfiltered unregistered MC packets       */
};

/* default stage number (from 0) to set egress port MC packet filtering */
#define PRV_TGF_SET_FILTER_STAGE_NUM_CNS    1

/* default stage number (from 0) to reset egress port MC packet filtering */
#define PRV_TGF_RESET_FILTER_STAGE_NUM_CNS  3

/* default stages number (from 0) to send known MC packet */
#define PRV_TGF_KNOWN_PACKET_STAGE_NUM_CNS  2

/* expected capture triggers */
static GT_U32  prvTgfExpTriggersArr[] =
{
    0x0FFFFF,   /* vfd[0]=1, vfd[1]=1 for unfiltered unregistered MC packets */
    0x00,       /* vfd[0]=0, vfd[1]=0 for filtered unregistered MC packets   */
    0x0FFFFF,   /* vfd[0]=1, vfd[1]=1 for filtered known MC packets          */
    0x0FFFFF    /* vfd[0]=0, vfd[1]=0 for unfiltered unregistered MC packets */
};

/* default FDB entry macAddr */
static TGF_MAC_ADDR prvTgfFdbEntryMacAddr = {0x01, 0x00, 0x5E, 0x00, 0x00, 0x02};

/* default unknown macAddr */
static TGF_MAC_ADDR prvTgfUnknownMacAddr = {0x01, 0x00, 0x5E, 0x00, 0x00, 0x01};
/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x01, 0x00, 0x5E, 0x00, 0x00, 0x01},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x88}                /* srcMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLAN_ID_CNS                           /* pri, cfi, VlanId */
};

/* default packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart =
{
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part =
{
    4,                                  /* version                */
    5,                                  /* headerLen              */
    0,                                  /* typeOfService          */
    0x2E,                               /* totalLen = 46 (20+26)  */
    0,                                  /* id                     */
    0,                                  /* flags                  */
    0,                                  /* offset                 */
    0x40,                               /* timeToLive = 64        */
    0xFF,                               /* protocol - reserved    */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,  /* chksum                 */
    {192,   1,   1,   1},               /* srcAddr = ipSrc        */
    {224,   0,   0,   1}                /* dstAddr = ipGrp        */
};

/* data of packet, len = 26 (0x1a) */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};

/* payload part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),       /* dataLength */
    prvTgfPayloadDataArr                /* dataPtr    */
};

/* parts of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},       /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},  /* VLAN_TAG part */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},/* ethertype     */
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},     /* packet's IPv4 */
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}   /* PAYLOAD part  */
};

/* length of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
    TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* length of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* packet to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen   */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    GT_BOOL  egressPortMcFilteringMode[PRV_TGF_MAX_PORTS_NUM_CNS];
} prvTgfRestoreCfg;

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/**
* @internal prvTgfFdbPerEgressPortUnregIpv4McFilteringConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfFdbPerEgressPortUnregIpv4McFilteringConfigSet
(
    GT_VOID
)
{
    GT_U32      portIter    = 0;
    GT_STATUS   rc          = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Setup config =====================\n\n");

    /* AUTODOC: create VLAN 2 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLAN_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgDefVlanEntryWrite");

    /* AUTODOC: set PVID 2 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, 
                                 prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                 PRV_TGF_VLAN_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgVlanPortVidSet");

    /* AUTODOC: add FDB entry with MAC 01:00:5e:00:00:02, VLAN 2, VID 2 */
    rc = prvTgfBrgDefFdbMacEntryOnVidSet(prvTgfFdbEntryMacAddr,
                                          PRV_TGF_VLAN_ID_CNS,
                                          PRV_TGF_VLAN_ID_CNS,
                                          GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                                 "ERROR of prvTgfBrgDefFdbMacEntryOnVidSet");

    for(portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* save egress port filtering mode */
        rc = prvTgfBrgEgressPortUnregMcFilteringGet(prvTgfDevNum, 
                                                    prvTgfPortsArray[portIter],
                                                    &prvTgfRestoreCfg.egressPortMcFilteringMode[portIter]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, 
                                     "ERROR of prvTgfBrgEgressPortUnregMcFilteringGet, port #%d",
                                     prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfFdbPerEgressPortUnregIpv4McFilteringTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbPerEgressPortUnregIpv4McFilteringTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc              = GT_OK;
    GT_U32      portIter        = 0;
    GT_U32      expPacketLen    = 0;
    GT_U32      numTriggers     = 0;
    GT_U32      rxPacketsCount  = 0;
    GT_U32      txPacketsCount  = 0;
    GT_U32      stageNum        = 0;

    CPSS_INTERFACE_INFO_STC     portInterface   = {0};
    TGF_VFD_INFO_STC            vfdArray[2]     = {{0}};

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: Iterate thru 4 stages: */
    for (stageNum = 0; stageNum < PRV_TGF_STAGE_NUM_CNS; stageNum++ )
    {
        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                                     "ERROR of tgfTrafficTableRxPcktTblClear");

        /* reset all counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                                     "ERROR of prvTgfEthCountersReset");

        /* print log message */
        switch (stageNum)
        {
            case PRV_TGF_SET_FILTER_STAGE_NUM_CNS:
                PRV_UTF_LOG0_MAC("\nEnable egress port unregistered IPv4 MC packet filtering\n");

                /* AUTODOC: for Stage#2 enable unreg MC filtering for all ports except Tx */
                for(portIter = 0; portIter < prvTgfPortsNum; portIter++)
                {
                    if ( PRV_TGF_TX_PORT_IDX_CNS == portIter)
                    {
                        continue;
                    }

                    /* enable filtering mode */
                    rc = prvTgfBrgEgressPortUnregMcFilteringSet( prvTgfPortsArray[portIter], GT_TRUE);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, 
                                                 "ERROR of prvTgfBrgEgressPortUnregMcFilteringSet: port=%d",
                                                 prvTgfPortsArray[portIter]);
                }
                break;

            case PRV_TGF_RESET_FILTER_STAGE_NUM_CNS:
                PRV_UTF_LOG0_MAC("\nDisable egress port unregistered IPv4 MC packet filtering\n");

                /* AUTODOC: for Stage#4 disable unreg MC filtering for all ports except Tx */
                for(portIter = 0; portIter < prvTgfPortsNum; portIter++)
                {
                    if ( PRV_TGF_TX_PORT_IDX_CNS == portIter)
                    {
                        continue;
                    }

                    /* disable filtering mode */
                    rc = prvTgfBrgEgressPortUnregMcFilteringSet( prvTgfPortsArray[portIter], GT_FALSE);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, 
                                                 "ERROR of prvTgfBrgEgressPortUnregMcFilteringSet: port=%d",
                                                 prvTgfPortsArray[portIter]);
                }
                break;

            default:
                break;
        }

        /* set Mac addr */
        if (PRV_TGF_KNOWN_PACKET_STAGE_NUM_CNS == stageNum)
        {
            PRV_UTF_LOG0_MAC("Send known MC packet\n");
            cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfFdbEntryMacAddr,
                         sizeof(prvTgfPacketL2Part.daMac));
        }
        else
        {
            PRV_UTF_LOG0_MAC("Send unregistered MC packet\n");
            cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfUnknownMacAddr,
                         sizeof(prvTgfPacketL2Part.daMac));
        }

        /* enable capture */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CPORT_IDX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, 
                                                    TGF_CAPTURE_MODE_PCL_E, 
                                                    GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                                     "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet\n");

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, 
                                 prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                     "ERROR of SetTxSetupEth: dev=%d, burst=%d",
                                     prvTgfDevNum, prvTgfBurstCount);

        /* set Tx delay */
        rc = prvTgfSetTxSetup2Eth(1, PRV_TGF_TX_DELAY_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth");

        /* AUTODOC: for stages 1,2,4 send 10 unreg IPv4 MC packets on port 0 with: */
        /* AUTODOC:   DA=01:00:5e:00:00:01, SA=00:00:00:00:00:88, VID=2 */
        /* AUTODOC:   sIP=192.1.1.1, dIP=224.0.0.1 */
        
        /* AUTODOC: for stage 3 send 10 known IPv4 MC packets on port 0 with: */
        /* AUTODOC:   DA=01:00:5e:00:00:02, SA=00:00:00:00:00:88, VID=2 */
        /* AUTODOC:   sIP=192.1.1.1, dIP=224.0.0.1 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, 
                                       prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]); 
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                     "ERROR of StartTransmitingEth: dev=%d, port=%d",
                                     prvTgfDevNum, 
                                     prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);

        /* remove Tx delay */
        rc = prvTgfSetTxSetup2Eth(0, 0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth");

        /* disable capture */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, 
                                                    TGF_CAPTURE_MODE_PCL_E, 
                                                    GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                     "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, portInterface.devPort.portNum);

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   1 stage - 10 packets received on ports 1,2,3 */
        /* AUTODOC:   2 stage - traffic filtered */
        /* AUTODOC:   3 stage - 10 packets received on ports 1,2,3 */
        /* AUTODOC:   4 stage - 10 packets received on ports 1,2,3 */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {           
            /* expected number of Tx packets */
            txPacketsCount = prvTgfPacketsCountRxTxArr[1 + (stageNum*2)][portIter];

            /* expected number of Rx packets */
            rxPacketsCount = prvTgfPacketsCountRxTxArr[0 + (stageNum*2)][portIter]+
                             ((PRV_TGF_CPORT_IDX_CNS == portIter) * txPacketsCount);

            /* expected packet size, packet exit without VLAN tag */
            if (portIter == PRV_TGF_TX_PORT_IDX_CNS)
            {
                expPacketLen =  PRV_TGF_PACKET_LEN_CNS;
            }
            else
            {
                expPacketLen =  PRV_TGF_PACKET_LEN_CNS - 4;
            }
            

            /* check ETH counters */
            rc = prvTgfEthCountersCheck(prvTgfDevNum, prvTgfPortsArray[portIter], 
                                        rxPacketsCount, txPacketsCount, expPacketLen,
                                        prvTgfBurstCount);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, 
                                         "ERROR of prvTgfEthCountersCheck"\
                                         "  Port=%02d  expectedRx=%02d"\
                                         "  expectedTx=%02d"\
                                         "  expectedLen=%02d\n",
                                         prvTgfPortsArray[portIter],
                                         rxPacketsCount, txPacketsCount, 
                                         expPacketLen);
        }

        /* get trigger counters */
        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

        /* get trigger counters where packet has MAC DA as macAddr */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 0;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[0].patternPtr, 
                     prvTgfPacketL2Part.daMac, sizeof(prvTgfPacketL2Part.daMac));

        /* get trigger counters where packet has MAC SA as macAddr */
        vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[1].modeExtraInfo = 0;
        vfdArray[1].offset = sizeof(TGF_MAC_ADDR);
        vfdArray[1].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[1].patternPtr, 
                     prvTgfPacketL2Part.saMac, sizeof(prvTgfPacketL2Part.saMac));

        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface,
                                                            sizeof(vfdArray) / sizeof(vfdArray[0]),
                                                            vfdArray, &numTriggers);
        PRV_UTF_LOG1_MAC("numTriggers = %d\n", numTriggers);

        /* check TriggerCounters */
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                     "ERROR of tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                     portInterface.devPort.hwDevNum, 
                                     portInterface.devPort.portNum);

        /* check if captured packet has the same MAC DA as prvTgfArpMac */
        PRV_UTF_LOG1_MAC("expTriggers = %d\n\n", prvTgfExpTriggersArr[stageNum]);

        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfExpTriggersArr[stageNum], 
                                     numTriggers,
                                     "\n ERROR: Got another MAC DA/SA\n");
    }
}

/**
* @internal prvTgfFdbPerEgressPortUnregIpv4McFilteringConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbPerEgressPortUnregIpv4McFilteringConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter    = 0;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Restore config: ==================\n\n");

    /* AUTODOC: restore egress port mode for all ports */
    for(portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfBrgEgressPortUnregMcFilteringSet(prvTgfPortsArray[portIter],
                                                  prvTgfRestoreCfg.egressPortMcFilteringMode[portIter]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, 
                                     "ERROR of prvTgfBrgEgressPortUnregMcFilteringSet, port #%d",
                                     prvTgfPortsArray[portIter]);
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                                 "ERROR of tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbFlush");

    /* invalidate and reset vlan entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLAN_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                                 "ERROR of prvTgfBrgDefVlanEntryInvalidate");

    /* AUTODOC: restore default vlan Id for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, 
                                 prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgVlanPortVidSet");
}


