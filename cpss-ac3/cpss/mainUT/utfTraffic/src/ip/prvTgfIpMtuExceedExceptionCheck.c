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
* @file prvTgfIpMtuExceedExceptionCheck.c
*
* @brief verify that when mtu exceed an exception is throw and the correct comand is executed
*
*
* @version   29
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfStcGen.h>
#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTrunkGen.h>
#include <ip/prvTgfIpMtuExceedExceptionCheck.h>
#include <trunk/prvTgfTrunk.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* default number of packets to send */
#define PRV_TGF_BURST_COUNT_DEFAULT_CNS     1

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = PRV_TGF_BURST_COUNT_DEFAULT_CNS;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};

/*protocol stack*/
static CPSS_IP_PROTOCOL_STACK_ENT   protocolStack =CPSS_IP_PROTOCOL_IPV4_E;

/*packet type */
static CPSS_IP_UNICAST_MULTICAST_ENT    packetType = CPSS_IP_UNICAST_E;

/* destination ePort */
#define PRV_TGF_DESTINATION_EPORT_CNS   1000

#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
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

/*capture type*/
static TGF_CAPTURE_MODE_ENT captureType = TGF_CAPTURE_MODE_MIRRORING_E;

/* flag for the first call of saving default */
static GT_BOOL firstCallToSaveMtuDefaultValue=GT_TRUE;
static GT_BOOL firstCallToSaveExcDefaultCmd=GT_TRUE;

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC            egressInfo;
    CPSS_INTERFACE_INFO_STC                     physicalInfo;
    PRV_TGF_INLIF_PORT_MODE_ENT                 sendPortInlifMode;
    GT_U32                                      mtuDefaultValue;
    PRV_TGF_IP_EXC_CMD_ENT                      excDefaultCmd;
} prvTgfRestoreCfg;
/* parameters that is needed to be restored */


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/**
* @internal prvTgfIpv4UcCaptureToCpuConfigurationSet function
* @endinternal
*
* @brief   start capture to the CPU configuration set
*/
GT_VOID prvTgfIpv4UcCaptureToCpuConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

     /****************************/
    /* start capture to the CPU */
    /****************************/

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* set capturing mode */
    rc = tgfTrafficTableRxModeSet(TGF_RX_CYCLIC_E, 600);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxModeSet");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /****************************/
}
/**
* @internal prvTgfIpv4UcExceptionConfigurationSet function
* @endinternal
*
* @brief   save default values of exception type and exception command.
*         set new values: exception type= MTU_EXCEEDED_FOR_NON_DF_E
*         exception command= CMD_DROP_HARD
* @param[in] exceptionType            - exception type
* @param[in] exceptionCmd             - exception command
*                                       None
*/
GT_VOID prvTgfIpv4UcExceptionConfigurationSet
(
    IN PRV_TGF_IP_EXCEPTION_TYPE_ENT    exceptionType,
    IN PRV_TGF_IP_EXC_CMD_ENT           exceptionCmd
)
{
    GT_STATUS   rc = GT_OK;

    #ifdef CHX_FAMILY
        if( exceptionType==PRV_TGF_IP_EXCP_MTU_EXCEEDED_E)
            exceptionType=PRV_TGF_IP_EXCP_MTU_EXCEEDED_FOR_NON_DF_E;
    #endif

    if (firstCallToSaveExcDefaultCmd==GT_TRUE)
    {
        /* save default MTU Exception command */
         rc = prvTgfIpExceptionCommandGet(prvTgfDevNum,exceptionType,protocolStack,
                                        packetType,&(prvTgfRestoreCfg.excDefaultCmd));
         PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfIpExceptionCommandGet);
         firstCallToSaveExcDefaultCmd = GT_FALSE;
    }

     /* set MTU Exception cpommand*/
     prvTgfIpExceptionCommandSet(prvTgfDevNum,exceptionType,protocolStack,
                                    packetType,exceptionCmd);
}


/**
* @internal prvTgfIpv4UcMtuConfigurationSet function
* @endinternal
*
* @brief   save default mtu
*         & set a new mtu value
* @param[in] mtu                      - the maximum transmission unit
*                                       None
*/
GT_VOID prvTgfIpv4UcMtuConfigurationSet
(
    IN GT_U32                           mtu
)
{
    GT_STATUS   rc = GT_OK;


    if (firstCallToSaveMtuDefaultValue==GT_TRUE)
    {
        /* save default MTU value */
         rc = prvTgfIpMtuProfileGet(prvTgfDevNum,0,&(prvTgfRestoreCfg.mtuDefaultValue));
         PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfIpMtuProfileGet);
         firstCallToSaveMtuDefaultValue = GT_FALSE;
    }

     /* set MTU profile value */
     prvTgfIpMtuProfileSet(prvTgfDevNum,0,mtu);

}

/**
* @internal prvTgfIpv4UcMtuConfigurationRestore function
* @endinternal
*
* @brief   restore mtu,exception type and exception command values
*
* @param[in] exceptionType            - exception type
* @param[in] protocolStack            - protocol stack
* @param[in] packetType               - packet type
*                                       None
*/
GT_VOID prvTgfIpv4UcMtuConfigurationRestore
(
    IN PRV_TGF_IP_EXCEPTION_TYPE_ENT        exceptionType,
    IN CPSS_IP_PROTOCOL_STACK_ENT           protocolStack,
    IN CPSS_IP_UNICAST_MULTICAST_ENT        packetType
)
{
    GT_STATUS   rc = GT_OK;

    #ifdef CHX_FAMILY
        if( exceptionType==PRV_TGF_IP_EXCP_MTU_EXCEEDED_E)
            exceptionType=PRV_TGF_IP_EXCP_MTU_EXCEEDED_FOR_NON_DF_E;
    #endif

    /* restore mtu value configuration */
    rc = prvTgfIpMtuProfileSet(prvTgfDevNum,0,(prvTgfRestoreCfg.mtuDefaultValue));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfIpMtuProfileSet);

    /* restore default MTU Exception command */
    rc = prvTgfIpExceptionCommandSet(prvTgfDevNum,exceptionType,protocolStack,
                                    packetType,(prvTgfRestoreCfg.excDefaultCmd));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfIpExceptionCommandSet);

    firstCallToSaveMtuDefaultValue = GT_TRUE;
    firstCallToSaveExcDefaultCmd = GT_TRUE;
}

/**
* @internal prvTgfIpv4UcrxInCpuCheck function
* @endinternal
*
* @brief   Check that the CPU got the expected number of packets from the selected port.
*         NOTE: for egress mirror/STC this is the 'egress' port .
* @param[in] hwDevNum                 - HW device number that send the packets to the CPU.
* @param[in] portNum                  - port number that send the packets to the CPU.
* @param[in] numPackets               - the number of packets expected to be sent by this port to the CPU.
*                                       None
*/
static void   prvTgfIpv4UcrxInCpuCheck
(
    /*IN GT_U8               hwDevNum,
    IN GT_U8               portNum,
    IN GT_U32              numPackets*/

    IN GT_HW_DEV_NUM               hwDevNum,
    IN GT_PORT_NUM                 portNum,
    IN GT_U32                      numPackets
)
{
    GT_STATUS       rc            = GT_OK;
    GT_U32          origPacketLen = 0;
    GT_BOOL         getFirst      = GT_TRUE;
    GT_U8           currentDevNum = 0;
    GT_U8           queue         = 0;
    TGF_NET_DSA_STC rxParam       = {0};
    GT_U32          packetLen     = 0x600;
    static GT_U8    packetBuf[0x600];
    GT_U32          numPacketsGet = 0;

    /* Go through entries from rxNetworkIf table */
    while (GT_OK == rc)
    {
        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                           getFirst, GT_FALSE, packetBuf,
                                           &packetLen, &origPacketLen,
                                           &currentDevNum, &queue, &rxParam);
        getFirst = GT_FALSE; /* now we get the next */

        if (GT_OK != rc)
        {
            continue;
        }

        /* Skip all not expected packets */
        if ((rxParam.srcIsTrunk || !rxParam.dsaCmdIsToCpu) ||
            (rxParam.hwDevNum != hwDevNum) ||
            (rxParam.portNum != portNum))
        {
            continue;
        }

        numPacketsGet++;
    }

    /* Value GT_NO_MORE is ok in this case */
    rc = (GT_NO_MORE == rc ? GT_OK : rc);

    if(rc == GT_OK && (numPacketsGet != numPackets))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(numPackets, numPacketsGet, "prvTgfIpv4UcrxInCpuCheck");
    }

    return ;
}


/**
* @internal prvTgfIpv4UcMtuRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] exceptionCmd             - exception command
*                                       None
*/
GT_VOID prvTgfIpv4UcMtuRoutingTrafficGenerate
(
    IN GT_U32                            sendPortNum,
    IN GT_U32                            nextHopPortNum,
    IN PRV_TGF_IP_EXC_CMD_ENT           exceptionCmd
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;
    GT_U32               numVfd = 0;/* number of VFDs in vfdArray */

    GT_U32      numExpectedSampled=1;/*number of packets expected to be sampled to the CPU*/



    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     *
     */

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
        {
            /* reset IP couters and set ROUTE_ENTRY mode */
            rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }
    }

    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = nextHopPortNum;


    captureType = TGF_CAPTURE_MODE_MIRRORING_E;


    /* enable capture on next hop port/trunk */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                 prvTgfDevNum, nextHopPortNum);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketPartArray;

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, numVfd, &vfdArray[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* set transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 50);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* AUTODOC: send packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01 */
    /* AUTODOC:   VID=5, srcIP=1.1.1.1, dstIP=1.1.1.3 */

    /* send packet -- send from specific port -- even if member of a trunk */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPortNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, sendPortNum);
    cpssOsTimerWkAfter(1000);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);



        /* AUTODOC: verify routed packet on port 3 */
        if((prvTgfPortsArray[portIter]==sendPortNum) )
        {
            /* packetSize is not changed */
                expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
        }
        else
        {
            if ((exceptionCmd==PRV_TGF_IP_EXC_CMD_NONE_E)&& (prvTgfPortsArray[portIter]==nextHopPortNum))
                {
                    /* packetSize is not changed */
                    expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                    expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                    expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                    expectedCntrs.brdcPktsSent.l[0]   = 0;
                    expectedCntrs.mcPktsSent.l[0]     = 0;
                    expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                    expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                    expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                    expectedCntrs.brdcPktsRcv.l[0]    = 0;
                    expectedCntrs.mcPktsRcv.l[0]      = 0;
                }
            else
                {
                     /* for other ports */
                    expectedCntrs.goodOctetsSent.l[0] = 0;
                    expectedCntrs.goodPktsSent.l[0]   = 0;
                    expectedCntrs.ucPktsSent.l[0]     = 0;
                    expectedCntrs.brdcPktsSent.l[0]   = 0;
                    expectedCntrs.mcPktsSent.l[0]     = 0;
                    expectedCntrs.goodOctetsRcv.l[0]  = 0;
                    expectedCntrs.goodPktsRcv.l[0]    = 0;
                    expectedCntrs.ucPktsRcv.l[0]      = 0;
                    expectedCntrs.brdcPktsRcv.l[0]    = 0;
                    expectedCntrs.mcPktsRcv.l[0]      = 0;
                }
        }
        isOk =
            portCntrs.goodOctetsSent.l[0] == expectedCntrs.goodOctetsSent.l[0] &&
            portCntrs.goodPktsSent.l[0]   == expectedCntrs.goodPktsSent.l[0] &&
            portCntrs.ucPktsSent.l[0]     == expectedCntrs.ucPktsSent.l[0] &&
            portCntrs.brdcPktsSent.l[0]   == expectedCntrs.brdcPktsSent.l[0] &&
            portCntrs.mcPktsSent.l[0]     == expectedCntrs.mcPktsSent.l[0] &&
            portCntrs.goodOctetsRcv.l[0]  == expectedCntrs.goodOctetsRcv.l[0] &&
            portCntrs.goodPktsRcv.l[0]    == expectedCntrs.goodPktsRcv.l[0] &&
            portCntrs.ucPktsRcv.l[0]      == expectedCntrs.ucPktsRcv.l[0] &&
            portCntrs.brdcPktsRcv.l[0]    == expectedCntrs.brdcPktsRcv.l[0] &&
            portCntrs.mcPktsRcv.l[0]      == expectedCntrs.mcPktsRcv.l[0];

        UTF_VERIFY_EQUAL0_STRING_MAC(isOk, GT_TRUE, "get another counters values.");

        /* print expected values if bug */
        if (isOk != GT_TRUE) {
            PRV_UTF_LOG0_MAC("Expected values:\n");
            PRV_UTF_LOG1_MAC("  goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC("    goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("    brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("   goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }


    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", nextHopPortNum);

    /* get trigger counters where packet has MAC DA as prvTgfArpMac */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    /* disable capture on nexthope port , before check the packets to the CPU */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                 prvTgfDevNum, nextHopPortNum);

    /* check the packets to the CPU */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                     portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    if (exceptionCmd==PRV_TGF_IP_EXC_CMD_NONE_E) {
        /* AUTODOC: check if packet has the same MAC DA as ARP MAC */
        /* number of triggers should be according to number of transmit*/
        UTF_VERIFY_EQUAL6_STRING_MAC(((1<<prvTgfBurstCount)-1), numTriggers,
            "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
            prvTgfArpMac[0], prvTgfArpMac[1], prvTgfArpMac[2],
            prvTgfArpMac[3], prvTgfArpMac[4], prvTgfArpMac[5]);
    }
    else if(exceptionCmd==PRV_TGF_IP_EXC_CMD_TRAP_TO_CPU_E)
    {
             /* stop Rx capture */
            rc = tgfTrafficTableRxStartCapture(GT_FALSE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

             /* check the CPU that got the sampling from the ingress/egress port */
            prvTgfIpv4UcrxInCpuCheck(prvTgfDevsArray[0],
                              prvTgfPortsArray[0],
                              numExpectedSampled);

            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStcPortSampledPacketsCntrGet");
            /*UTF_VERIFY_EQUAL0_STRING_MAC(numExpectedSampled, numSampled, "numSampled");*/
    }

    /* -------------------------------------------------------------------------
     * 5. Get IP Counters
     */

    /* AUTODOC: get and print ip counters values */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");

    /* restore transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* disable the PCL exclude vid capturing */
    rc = tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet(GT_FALSE,0/*don't care*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet: %d, %d\n",
                                 prvTgfDevNum, nextHopPortNum);

    /* just for 'cleanup' */
    captureType = TGF_CAPTURE_MODE_MIRRORING_E;
}




