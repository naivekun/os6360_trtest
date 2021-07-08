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
* @file prvTgfBrgGenPropL2ControlProtCmdAf.c
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
#include <common/tgfAutoFlow.h>

#include <bridge/prvTgfBrgGenPropL2ControlProtCmdAf.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
                                      
/* array of Cisco MC reserved destination MAC addresses for the tests */
static TGF_MAC_ADDR prvTgfDaMacArr[] = { {0x01, 0x00, 0x0c, 0xcd, 0xcd, 0xce},    /*VLAN Bridge*/
                                         {0x01, 0x00, 0x0c, 0xcd, 0xcd, 0xcd},    /*STP Uplink Fast*/
                                         {0x01, 0x00, 0x0c, 0x00, 0x00, 0x00}     /*Inter-switch Link*/
};

static GT_U8 prvTgfPayloadDataArr[] = {
    0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
    0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
    0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
    0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
    0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
    0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
    0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00
    };

static PRV_TGF_AF_DATA_STC dataStc ={NULL,0};


/* LENGTH of buffer*/
#define TGF_RX_BUFFER_SIZE_CNS   0x600


/******************************************************************************/

/* stored default PropL2Control command */
static CPSS_PACKET_CMD_ENT savePropL2ControlCmd;

static PRV_TGF_AF_TRAFFIC_UNT             trafficUn;
static PRV_TGF_AF_BRIDGE_UNT              bridgeUn;

/* default VLAN Id */
static GT_U16 prvTgfVlanId = 100;

/* default number of packets to send */
static GT_U32        prvTgfBurstCount = 1;

/*default ingress port index*/
static GT_U32 prvTgfIngPortIdx=0;

/*default packet size*/
static GT_U32        prvTgfPacketSize = 64;
/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/


/******************************* Inner functions ******************************/


/**
* @internal prvTgfPropL2ControlTrapCmdSetAf function
* @endinternal
*
* @brief   Set PropL2Control trapping command configuration
*
* @param[in] trapCmd                  - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*                                       None
*/
GT_VOID prvTgfPropL2ControlTrapCmdSetAf
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
* @internal prvTgfBrgGenPropL2ControlTrafficGeneratorAf function
* @endinternal
*
* @brief   Generate traffic and compare counters
*
* @param[in] packetToCpu              - send packet to cpu?
* @param[in] floodOrDrop              - send packet to ports?
* @param[in] pcktNum                  - packet index
*                                       None
*/
GT_VOID prvTgfBrgGenPropL2ControlTrafficGeneratorAf
(
    IN GT_BOOL  packetToCpu,
    IN GT_BOOL  floodOrDrop,
    IN GT_U32   pcktNum
)
{
    GT_STATUS                       rc;
    GT_U32                          portIter = 0;
    GT_U32                          idx;
    GT_U32                          packetSize;
    GT_BOOL     getFirst = GT_TRUE;
    GT_U8       packetBuff[TGF_RX_BUFFER_SIZE_CNS] = {0};
    GT_U32      buffLen  = TGF_RX_BUFFER_SIZE_CNS;
    GT_U32      packetActualLength = 0;  
    GT_U8       devNum;
    GT_U8       queue;
    TGF_NET_DSA_STC rxParam;
    
    
    GT_STATUS   expectedRcRxToCpu = GT_OK;
    /* setup packet */
   for(idx=0;idx<PRV_TGF_AF_MAC_ADDR_LEN_CNS;idx++)
   {
      trafficUn.daMac[idx]=prvTgfDaMacArr[pcktNum][idx];
   }
   tgfAutoFlowTrafficPreset(PRV_TGF_AF_TRAFFIC_PARAM_MAC_DA_E,&trafficUn);
   tgfAutoFlowTxSetup();
  
   tgfAutoFlowBridgeGet(PRV_TGF_AF_BRIDGE_PARAM_INGRESS_PORT_IDX_E,&bridgeUn);
   prvTgfIngPortIdx=bridgeUn.ingressPortIdx;
   tgfAutoFlowTrafficGet(PRV_TGF_AF_TRAFFIC_PARAM_BURST_COUNT_E,&trafficUn);
   prvTgfBurstCount=trafficUn.burstCount;
   tgfAutoFlowTrafficGet(PRV_TGF_AF_TRAFFIC_PARAM_PACKET_SIZE_E,&trafficUn); 
   prvTgfPacketSize=trafficUn.packetSize;

   /*checks that the retrieved value from the AF library is the relevant value*/
   bridgeUn.ingTag1Pid=0xcccc;
   tgfAutoFlowBridgePreset(PRV_TGF_AF_BRIDGE_PARAM_INGRESS_TAG_1_PID_E,bridgeUn);
   tgfAutoFlowBridgeGet(PRV_TGF_AF_BRIDGE_PARAM_INGRESS_TAG_1_PID_E,&bridgeUn);

   /* AUTODOC: GENERATE TRAFFIC: */
   
   tgfAutoFlowTrafficGet(PRV_TGF_AF_TRAFFIC_PARAM_DATA_STC_E,&trafficUn);
   dataStc=trafficUn.dataStc;
    
    /* AUTODOC:   send MC packet with: */
    /* AUTODOC:   DA=01:00:0c:xx:xx:xx, SA=00:00:00:00:00:11 */
 

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");
    
   
    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    tgfAutoFlowTxStart();
                                 
    /* read and check ethernet counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        packetSize=prvTgfPacketSize;
        /* AUTODOC: verify traffic is dropped */
        rc = prvTgfEthCountersCheck(
            prvTgfDevNum,
            prvTgfPortsArray[portIter],
            /*expected Rx*/((prvTgfIngPortIdx  == portIter)  ? 1 : 0),
            /*expected Tx*/((prvTgfIngPortIdx  == portIter)  ? 1 : ((floodOrDrop == GT_TRUE) ? 1 : 0)),
            packetSize,
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

    /* get first entries from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                   getFirst, GT_TRUE, packetBuff,
                                   &buffLen, &packetActualLength,
                                   &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(expectedRcRxToCpu, rc, 
                             "tgfTrafficGeneratorRxInCpuGet %d");

    for(idx=1;idx<prvTgfBurstCount;idx++)
    {
        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(expectedRcRxToCpu, rc, 
                                 "tgfTrafficGeneratorRxInCpuGet %d");
    }
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, 
                                            "tgfTrafficGeneratorRxInCpuGet %d");
}

/**
* @internal prvTgfBrgGenPropL2ControlSetAf function
* @endinternal
*
* @brief   Set test bridge configuration
*/
GT_VOID prvTgfBrgGenPropL2ControlSetAf
(
    IN GT_VOID
)
{
    GT_STATUS   rc;
   
    /* AUTODOC: SETUP CONFIGURATION: */
    
    /* AUTODOC: Save Proprietary Layer 2 control protocol command */
    rc = prvTgfBrgGenCiscoL2ProtCmdGet(&savePropL2ControlCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                                 "prvTgfBrgGenCiscoL2ProtCmdGet: %d");
    bridgeUn.ingTag1Pid=0x88a8;
    tgfAutoFlowBridgePreset(PRV_TGF_AF_BRIDGE_PARAM_INGRESS_TAG_1_PID_E,bridgeUn);
    bridgeUn.egTag1Pid=0x88a8;
    tgfAutoFlowBridgePreset(PRV_TGF_AF_BRIDGE_PARAM_EGRESS_TAG_1_PID_E,bridgeUn);
    bridgeUn.egTagCmd=PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E;
    tgfAutoFlowBridgePreset(PRV_TGF_AF_BRIDGE_PARAM_EGRESS_TAGGING_CMD_E,bridgeUn);

    trafficUn.vlanTagType=PRV_TGF_AF_VLAN_TAG_TYPE_DOUBLE_TAGGED_E;
    tgfAutoFlowTrafficPreset(PRV_TGF_AF_TRAFFIC_PARAM_VLAN_TAG_TYPE_E,&trafficUn);
    trafficUn.dataStc.dataPtr=prvTgfPayloadDataArr;
    trafficUn.dataStc.dataLen=sizeof(prvTgfPayloadDataArr);
    tgfAutoFlowTrafficPreset(PRV_TGF_AF_TRAFFIC_PARAM_DATA_STC_E,&trafficUn);
    tgfAutoFlowCreate();

    tgfAutoFlowBridgeGet(PRV_TGF_AF_BRIDGE_PARAM_INGRESS_VLAN_E,&bridgeUn);
    prvTgfVlanId=bridgeUn.ingressVlan;
    
}

/**
* @internal prvTgfBrgGenPropL2ControlRestoreAf function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfBrgGenPropL2ControlRestoreAf
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: restore trapping command for Proprietary Layer 2 control MC packets */
    rc = prvTgfBrgGenCiscoL2ProtCmdSet(savePropL2ControlCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                                 "prvTgfBrgGenCiscoL2ProtCmdSet: %d");
    tgfAutoFlowDel();

}

/**
* @internal prvTgfBrgGenPropL2ControlTestAf function
* @endinternal
*
* @brief   PropL2Control configuration test
*/
GT_VOID prvTgfBrgGenPropL2ControlTestAf
(
    GT_VOID
)
{
    GT_U32 pcktNum=0;
    /* #1 forwarding packet*/
    /* Set configuration */
    prvTgfPropL2ControlTrapCmdSetAf(CPSS_PACKET_CMD_FORWARD_E);

    /* Generate traffic */
    prvTgfBrgGenPropL2ControlTrafficGeneratorAf(GT_FALSE, GT_TRUE,pcktNum);
    pcktNum++;

    /* #2 mirrorring packet to CPU*/
    /* Set configuration */
    prvTgfPropL2ControlTrapCmdSetAf(CPSS_PACKET_CMD_MIRROR_TO_CPU_E);

    /* Generate traffic */
    prvTgfBrgGenPropL2ControlTrafficGeneratorAf(GT_TRUE, GT_TRUE,pcktNum);
    pcktNum++;
    /* #3 trapping packet to CPU*/
    /* Set configuration */
    prvTgfPropL2ControlTrapCmdSetAf(CPSS_PACKET_CMD_TRAP_TO_CPU_E);

    /* Generate traffic */
    prvTgfBrgGenPropL2ControlTrafficGeneratorAf(GT_TRUE, GT_FALSE,pcktNum);
}


