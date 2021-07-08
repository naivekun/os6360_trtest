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
* @file prvTgfBrgGenPropL2ControlProtCmd.c
*
* @brief Cisco L2 protocol commands / Proprietary Layer 2 control MC
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>

#include <bridge/prvTgfBrgGenPropL2ControlProtCmd.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* VLAN Id */
#define PRV_TGF_VLANID_CNS  100

/* port to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* default number of packets to send */
static GT_U32        prvTgfBurstCount =   1;

                                      
/* array of Cisco MC reserved destination MAC addresses for the tests */
static TGF_MAC_ADDR prvTgfDaMacArr[] = { {0x01, 0x00, 0x0c, 0xcd, 0xcd, 0xce},    /*VLAN Bridge*/
                                         {0x01, 0x00, 0x0c, 0xcd, 0xcd, 0xcd},    /*STP Uplink Fast*/
                                         {0x01, 0x00, 0x0c, 0x00, 0x00, 0x00} };  /*Inter-switch Link*/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},                /* default BC Da Mac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                 /* sa Mac */
};
/* DATA of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] = {
    0xfa, 0xb5, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1f, 0x20,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPacketPayloadDataArr),                 /* dataLength */
    prvTgfPacketPayloadDataArr                          /* dataPtr */
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS +  sizeof(prvTgfPacketPayloadDataArr)

/* PARTS of packet */
static  TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part}, /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};
    /* PACKET to send */
static  TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                  /* totalLen */
    sizeof(prvTgfPacketPartArray)/sizeof(TGF_PACKET_PART_STC),/* numOfParts*/
    prvTgfPacketPartArray                                     /* partsArray*/
};


/* LENGTH of buffer*/
#define TGF_RX_BUFFER_SIZE_CNS   0x80


/******************************************************************************/

/* stored default PropL2Control command */
static CPSS_PACKET_CMD_ENT savePropL2ControlCmd;

/* stored default VID */
static GT_U16   prvTgfDefVlanId = 0;



/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/


/******************************* Inner functions ******************************/


/**
* @internal prvTgfPropL2ControlTrapCmdSet function
* @endinternal
*
* @brief   Set PropL2Control trapping command configuration
*
* @param[in] trapCmd                  - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*                                       None
*/
GT_VOID prvTgfPropL2ControlTrapCmdSet
(
    IN CPSS_PACKET_CMD_ENT  trapCmd
)
{
    GT_STATUS   rc;

    /* AUTODOC: set trapping command for Cisco-protocol MC packets */
    rc = prvTgfBrgGenCiscoL2ProtCmdSet(trapCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                                 "prvTgfBrgGenCiscoL2ProtCmdSet: %d");

}

/**
* @internal prvTgfBrgGenPropL2ControlTrafficGenerator function
* @endinternal
*
* @brief   Generate traffic and compare counters
*
* @param[in] packetToCpu              - send packet to cpu?
* @param[in] floodOrDrop              - send packet to ports?
* @param[in] pcktNum                  - packet index
*                                       None
*/
GT_VOID prvTgfBrgGenPropL2ControlTrafficGenerator
(
    IN GT_BOOL  packetToCpu,
    IN GT_BOOL  floodOrDrop,
    IN GT_U32   pcktNum
)
{
    GT_STATUS                       rc;
    GT_U32                          portIter = 0;
    
    GT_BOOL     getFirst = GT_TRUE;
    GT_U8       packetBuff[TGF_RX_BUFFER_SIZE_CNS] = {0};
    GT_U32      buffLen  = TGF_RX_BUFFER_SIZE_CNS;
    GT_U32      packetActualLength = 0;   
    GT_U8       devNum;
    GT_U8       queue;
    TGF_NET_DSA_STC rxParam;
    
    GT_STATUS   expectedRcRxToCpu = GT_OK;

    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfDaMacArr[pcktNum], sizeof(prvTgfPacketL2Part.daMac));

    

    /* AUTODOC: GENERATE TRAFFIC: */
    
    /* AUTODOC:   send MC packet with: */
    /* AUTODOC:   DA=01:00:0c:xx:xx:xx, SA=00:00:00:00:00:11 */
 

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, 
                             prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", 
                                 prvTgfDevNum);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    rc = prvTgfStartTransmitingEth(prvTgfDevNum, 
                                   prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                 "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, 
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
                                 
    /* read and check ethernet counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* AUTODOC: verify traffic is dropped */
        rc = prvTgfEthCountersCheck(
            prvTgfDevNum,
            prvTgfPortsArray[portIter],
            /*expected Rx*/((PRV_TGF_SEND_PORT_IDX_CNS  == portIter)  ? 1 : 0),
            /*expected Tx*/((PRV_TGF_SEND_PORT_IDX_CNS  == portIter)  ? 1 : ((floodOrDrop == GT_TRUE) ? 1 : 0)),
            prvTgfPacketInfo.totalLen,
            prvTgfBurstCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfEthCountersCheck: %d, %d\n",
            prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
    
    if ( GT_FALSE == packetToCpu )
    {
        expectedRcRxToCpu = GT_NO_MORE;
    }

    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* get first entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       getFirst, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(expectedRcRxToCpu, rc, 
                                 "tgfTrafficGeneratorRxInCpuGet %d");
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, 
                                            "tgfTrafficGeneratorRxInCpuGet %d");
}

/**
* @internal prvTgfBrgGenPropL2ControlSet function
* @endinternal
*
* @brief   Set test bridge configuration
*/
GT_VOID prvTgfBrgGenPropL2ControlSet
(
    IN GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, 
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], 
                                 &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
                                 prvTgfDevNum, 
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, 
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], 
                                 PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, 
                                 "ERROR of prvTgfBrgVlanPortVidSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: create VLAN 100 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS, prvTgfPortsArray,
                                           NULL, NULL, prvTgfPortsNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, 
                               "ERROR of prvTgfBrgDefVlanEntryWithPortsSet: %d",
                                 prvTgfDevNum);
    

    /* AUTODOC: Save Proprietary Layer 2 control protocol command */
    rc = prvTgfBrgGenCiscoL2ProtCmdGet(&savePropL2ControlCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                                 "prvTgfBrgGenCiscoL2ProtCmdGet: %d");
}

/**
* @internal prvTgfBrgGenPropL2ControlRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfBrgGenPropL2ControlRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: RESTORE CONFIGURATION: */


    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", 
                                 prvTgfDevNum, GT_TRUE);
    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfDefVlanId);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* AUTODOC: restore trapping command for Proprietary Layer 2 control MC packets */
    rc = prvTgfBrgGenCiscoL2ProtCmdSet(savePropL2ControlCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                                 "prvTgfBrgGenCiscoL2ProtCmdSet: %d");

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfBrgGenPropL2ControlTest function
* @endinternal
*
* @brief   PropL2Control configuration test
*/
GT_VOID prvTgfBrgGenPropL2ControlTest
(
    GT_VOID
)
{
    GT_U32 pcktNum=0;
    /* #1 forwarding packet*/
    /* Set configuration */
    prvTgfPropL2ControlTrapCmdSet(CPSS_PACKET_CMD_FORWARD_E);

    /* Generate traffic */
    prvTgfBrgGenPropL2ControlTrafficGenerator(GT_FALSE, GT_TRUE,pcktNum);
    pcktNum++;

    /* #2 mirrorring packet to CPU*/
    /* Set configuration */
    prvTgfPropL2ControlTrapCmdSet(CPSS_PACKET_CMD_MIRROR_TO_CPU_E);

    /* Generate traffic */
    prvTgfBrgGenPropL2ControlTrafficGenerator(GT_TRUE, GT_TRUE,pcktNum);
    pcktNum++;
    /* #3 trapping packet to CPU*/
    /* Set configuration */
    prvTgfPropL2ControlTrapCmdSet(CPSS_PACKET_CMD_TRAP_TO_CPU_E);

    /* Generate traffic */
    prvTgfBrgGenPropL2ControlTrafficGenerator(GT_TRUE, GT_FALSE,pcktNum);

}


