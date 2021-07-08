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
* @file prvTgfStc.c
*
* @brief CPSS STC (Sampling to CPU)
*
* @version   14
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfBridgeGen.h>
#include <common/tgfCommon.h>
#include <common/tgfStcGen.h>
#include <mirror/prvTgfStc.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS  5

/* STC limit */
#define PRV_TGF_INGRESS_STC_LIMIT_CNS   3
/* egress STC limit */
#define PRV_TGF_EGRESS_STC_LIMIT_CNS   5

/* ether type used for "vlan tag" */
#define TGF_ETHERTYPE_8100_VLAN_TAG_CNS 0x8100

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         1

/* egress port number for egress STC
   Use port 3 for TM systems to use port not connected to TM and use separate TxQ */
#define PRV_TGF_EGRESS_PORT_IDX_MAC() (prvUtfIsTrafficManagerUsed()) ? 3 : 2

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 10;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}                /* srcMac */
};


/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};


/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x02, 0x81, 0x00, 0x05, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
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
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/******************************************************************************\
 *                           Private declarations                             *
\******************************************************************************/


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal rxInCpuCheck function
* @endinternal
*
* @brief   Check that the CPU got the expected number of packets from the selected port.
*         NOTE: for egress mirror/STC this is the 'egress' port .
* @param[in] hwDevNum                 - HW device number that send the packets to the CPU.
* @param[in] portNum                  - port number that send the packets to the CPU.
* @param[in] numPackets               - the number of packets expected to be sent by this port to the CPU.
*                                       None
*/
static void   rxInCpuCheck
(
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
    CPSS_INTERFACE_INFO_STC portInterface;

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = hwDevNum;
    portInterface.devPort.portNum = portNum;

    /* Go through entries from rxNetworkIf table */
    while (GT_OK == rc)
    {
        packetLen     = 0x600;
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&portInterface,
                                            TGF_PACKET_TYPE_REGULAR_E,
                                           getFirst, GT_FALSE, packetBuf,
                                           &packetLen, &origPacketLen,
                                           &currentDevNum, &queue, &rxParam);
        getFirst = GT_FALSE; /* now we get the next */

        if (GT_OK != rc)
        {
            continue;
        }

        numPacketsGet++;
    }

    /* Value GT_NO_MORE is ok in this case */
    rc = (GT_NO_MORE == rc ? GT_OK : rc);

    if(rc == GT_OK && (numPacketsGet != numPackets))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(numPackets, numPacketsGet, "rxInCpuCheck");
    }

    return ;
}

/**
* @internal prvTgfStcBasicConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         1. create VLAN with it's members
*         2. enable ingress/egress STC
*         3. set reload mode
*         4. set STC limit on ingress/egress port
*         5. reset sampled counter
*         6. CPU RX table:
*         clear table , set capturing mode , start capture.
* @param[in] stcType                  - STC source type : ingress/egress
* @param[in] reloadMode               - reload mode : continuous / triggered
*                                       None
*/
GT_VOID prvTgfStcBasicConfigurationSet
(
    IN PRV_TGF_STC_TYPE_ENT                 stcType,
    IN PRV_TGF_STC_COUNT_RELOAD_MODE_ENT    reloadMode
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32  getCountDown,getCountDown1;/*get countdown counter (num packets till next STC)*/
    GT_U32  stcPortIndex;/* port index of the ingress/egress port */
    GT_U32  stcLimit;/* stc limit */
    GT_BOOL waEnabled = GT_FALSE;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

    if(stcType == PRV_TGF_STC_INGRESS_E)
    {
        prvTgfBurstCount   = 10;
        stcPortIndex = PRV_TGF_SEND_PORT_IDX_CNS;
        stcLimit = PRV_TGF_INGRESS_STC_LIMIT_CNS;
    }
    else
    {
        prvTgfBurstCount   = 17;
        stcPortIndex = PRV_TGF_EGRESS_PORT_IDX_MAC();
        stcLimit = PRV_TGF_EGRESS_STC_LIMIT_CNS;
    }


    /* Eggress STC Table errata */
    rc = prvUtfDeviceFamilyGet(prvTgfDevsArray[stcPortIndex], &devFamily);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvUtfDeviceFamilyGet: %d", prvTgfDevsArray[stcPortIndex]);

    if((CPSS_PP_FAMILY_DXCH_LION_E == devFamily) && (stcType == PRV_TGF_STC_EGRESS_E))
    {
        waEnabled = GT_TRUE;
    }


    /* set VLAN entry */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_VLANID_CNS);

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /****************************/
    /* set generic ingress STC  */
    /****************************/

    /* enable ingress/egress STC in the device */
    rc = prvTgfStcEnableSet(stcType,GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStcEnableSet");

    rc = prvTgfStcReloadModeSet(stcType,reloadMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStcReloadModeSet");

    if(stcType == PRV_TGF_STC_INGRESS_E)
    {
        rc = prvTgfStcIngressCountModeSet(PRV_TGF_STC_COUNT_ALL_PACKETS_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStcIngressCountModeSet");
    }

    /*get countdown counter (num packets till next STC)*/
    rc = prvTgfStcPortCntrGet(prvTgfDevsArray[stcPortIndex],
                               prvTgfPortsArray[stcPortIndex],
                               stcType,
                               &getCountDown);
    if (!waEnabled)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStcPortCntrGet");
    }
    else
    {
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_SUPPORTED, rc, prvTgfDevsArray[stcPortIndex]);
    }



    /*set limit on the ingress/egress port */
    rc = prvTgfStcPortLimitSet(prvTgfDevsArray[stcPortIndex],
                               prvTgfPortsArray[stcPortIndex],
                               stcType,
                               stcLimit);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStcPortLimitSet");

    /*get countdown counter (num packets till next STC)*/
    rc = prvTgfStcPortCntrGet(prvTgfDevsArray[stcPortIndex],
                               prvTgfPortsArray[stcPortIndex],
                               stcType,
                               &getCountDown1);
    if (!waEnabled)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStcPortCntrGet");
        /* the count down is not changed by the 'write' operation ,
           but only by the first packet !!! */
        UTF_VERIFY_EQUAL0_STRING_MAC(getCountDown,getCountDown1,"mismatch Limit");
    }
    else
    {
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_SUPPORTED, rc, prvTgfDevsArray[stcPortIndex]);
    }

    /* reset sampled counter */
    rc = prvTgfStcPortSampledPacketsCntrSet(prvTgfDevsArray[stcPortIndex],
                               prvTgfPortsArray[stcPortIndex],
                               stcType,
                               0);
    if (!waEnabled)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStcPortSampledPacketsCntrSet");
    }
    else
    {
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_SUPPORTED, rc, prvTgfDevsArray[stcPortIndex]);
    }


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

}

/**
* @internal prvTgfStcBasicTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic , with next steps:
*         1. clear counters
*         2. build packet (TX setup)
*         3. send packets from 'Sender port' (regardless to ingress/egress STC)
*         4. check port counters (traffic do flooding)
*         for trigger mode --> change 7 times the limit and resent packets.
*         5. check CPU got the 'expected' num of STC packets from the ingress/egress port.
*         6. check that sampled counter in the 'per port' STC entry match the 'expected'
* @param[in] stcType                  - STC source type : ingress/egress
* @param[in] reloadMode               - reload mode : continuous / triggered
*                                       None
*/
GT_VOID prvTgfStcBasicTrafficGenerate
(
    IN PRV_TGF_STC_TYPE_ENT                 stcType,
    IN PRV_TGF_STC_COUNT_RELOAD_MODE_ENT    reloadMode
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       ii;
    GT_U32       portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32      numSampled;/*number of packets that sampled to the CPU (according to STC entry in the device)*/
    GT_U32      numExpectedSampled;/*number of packets expected to be sampled to the CPU*/
    GT_U32  stcPortIndex;/* port index of the ingress/egress port */
    GT_U32  stcLimit;/* stc limit */
    GT_U32  reloadId;/*reload iterator*/
    GT_U32  maxReloadNum;/*max reload Number*/
    GT_U32  getCountDown,getCountDown1;/*get countdown counter (num packets till next STC)*/
    GT_U32  numOfEvents;/*number of events expected (interrupts) */
    CPSS_UNI_EV_CAUSE_ENT uniEvent;/* unified event */
    GT_BOOL waEnabled = GT_FALSE;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

    if(stcType == PRV_TGF_STC_INGRESS_E)
    {
        prvTgfBurstCount   = 10;
        stcPortIndex = PRV_TGF_SEND_PORT_IDX_CNS;
        stcLimit = PRV_TGF_INGRESS_STC_LIMIT_CNS;
        uniEvent = CPSS_PP_MAC_SFLOW_E;
    }
    else
    {
        prvTgfBurstCount   = 17;
        stcPortIndex = PRV_TGF_EGRESS_PORT_IDX_MAC();
        stcLimit = PRV_TGF_EGRESS_STC_LIMIT_CNS;
        uniEvent = CPSS_PP_EGRESS_SFLOW_E;
    }
    /* Eggress STC Table errata */
    rc = prvUtfDeviceFamilyGet(prvTgfDevsArray[stcPortIndex], &devFamily);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvUtfDeviceFamilyGet: %d", prvTgfDevsArray[stcPortIndex]);

    if((CPSS_PP_FAMILY_DXCH_LION_E == devFamily) && (stcType == PRV_TGF_STC_EGRESS_E))
    {
        waEnabled = GT_TRUE;
    }

    /* clear interrupts (from previous time) */
    rc = utfGenEventCounterGet(prvTgfDevsArray[stcPortIndex],
        uniEvent,GT_TRUE,&numOfEvents);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoGenEventCounterGet");

    if(reloadMode == PRV_TGF_STC_COUNT_RELOAD_TRIGGERED_E)
    {
        /* on every trigger generate new limit */
        maxReloadNum = 7;
    }
    else
    {
        maxReloadNum = 1;
    }

    for(reloadId = 0 ; reloadId < maxReloadNum ; reloadId++)
    {
        /* reset counters */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevsArray[portIter], prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevsArray[portIter], prvTgfPortsArray[portIter]);
        }

        /* setup Packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, 1, 0, NULL);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                     prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfBurstCount, 0, NULL);

        /* send in loop of 1 to avoid lost of events in BM */
        for(ii = 0 ; ii < prvTgfBurstCount ; ii++)
        {
            /* send Packets */
            rc = prvTgfStartTransmitingEth(prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                    prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);


            /* the simulation needs more sleep between packets to avoid missed events */
            cpssOsTimerWkAfter(200);
        }

        /* check counters */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevsArray[portIter], prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevsArray[portIter], prvTgfPortsArray[portIter]);

            if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
            {
                /* check Rx counters */
                UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                             "get another goodPktsRcv counter than expected -> port [%d]",prvTgfPortsArray[portIter]);
                /* check Tx counters */
                UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                             "get another goodPktsSent counter than expected -> port [%d]",prvTgfPortsArray[portIter]);
                continue;
            }

            /* check Tx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected -> port [%d]",prvTgfPortsArray[portIter]);
            /* check Rx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected -> port [%d]",prvTgfPortsArray[portIter]);
        }

        /* reset counters */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevsArray[portIter], prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevsArray[portIter], prvTgfPortsArray[portIter]);
        }

        if(reloadMode == PRV_TGF_STC_COUNT_RELOAD_TRIGGERED_E)
        {
            /* on every trigger generate new limit */
            stcLimit ++;

            /*get countdown counter (num packets till next STC)*/
            rc = prvTgfStcPortCntrGet(prvTgfDevsArray[stcPortIndex],
                                       prvTgfPortsArray[stcPortIndex],
                                       stcType,
                                       &getCountDown);
            if (!waEnabled)
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStcPortCntrGet");
            }
            else
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_SUPPORTED, rc, prvTgfDevsArray[stcPortIndex]);
            }

            /*set limit on the ingress/egress port */
            rc = prvTgfStcPortLimitSet(prvTgfDevsArray[stcPortIndex],
                                       prvTgfPortsArray[stcPortIndex],
                                       stcType,
                                       stcLimit);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStcPortLimitSet");

            /*get countdown counter (num packets till next STC)*/
            rc = prvTgfStcPortCntrGet(prvTgfDevsArray[stcPortIndex],
                                       prvTgfPortsArray[stcPortIndex],
                                       stcType,
                                       &getCountDown1);
            if (!waEnabled)
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStcPortCntrGet");
                /* the count down is not changed by the 'write' operation ,
                   but only by the first packet !!! */
                UTF_VERIFY_EQUAL0_STRING_MAC(getCountDown,getCountDown1,"mismatch Limit");
            }
            else
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_SUPPORTED, rc, prvTgfDevsArray[stcPortIndex]);
            }

        }

    }/*reloadId*/

    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    if(reloadMode == PRV_TGF_STC_COUNT_RELOAD_CONTINUOUS_E)
    {
        numExpectedSampled = prvTgfBurstCount / stcLimit;
    }
    else /* triggered */
    {
        numExpectedSampled = maxReloadNum;
    }

    /* check the CPU that got the sampling from the ingress/egress port */
    rxInCpuCheck(prvTgfDevsArray[stcPortIndex],
                      prvTgfPortsArray[stcPortIndex],
                      numExpectedSampled);


    /* get sampled counter */
    rc = prvTgfStcPortSampledPacketsCntrGet(prvTgfDevsArray[stcPortIndex],
                               prvTgfPortsArray[stcPortIndex],
                               stcType,
                               &numSampled);
    if (!waEnabled)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStcPortSampledPacketsCntrGet");
        UTF_VERIFY_EQUAL0_STRING_MAC(numExpectedSampled, numSampled, "numSampled");
    }
    else
    {
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_SUPPORTED, rc, prvTgfDevsArray[stcPortIndex]);
    }


    /* get interrupts */
    rc = utfGenEventCounterGet(prvTgfDevsArray[stcPortIndex],
        uniEvent,GT_TRUE,&numOfEvents);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoGenEventCounterGet");

    if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevsArray[stcPortIndex]) && prvUtfIsGmCompilation())
    {
        /* interrupts not bound/implemented */
    }
    else
    if(stcType == PRV_TGF_STC_EGRESS_E && devFamily == CPSS_PP_FAMILY_PUMA_E)
    {
        /* FEr in Puma2 --> the egress STC hold no valid interrupt ! */

        /* Ref#: FE-xxx-10010110
           TXQ STC Interrupt is not valid (PRV_CPSS_EXMXPM_FER_TXQ_STC_INTR_NOT_VALID_E).*/
        UTF_VERIFY_EQUAL0_STRING_MAC(0, numOfEvents, "numEvents");
    }
    else if((reloadMode == PRV_TGF_STC_COUNT_RELOAD_CONTINUOUS_E) && (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevsArray[stcPortIndex])))
    {
        if ((stcType == PRV_TGF_STC_EGRESS_E) && UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(prvTgfDevsArray[stcPortIndex]))
        {
            /* BC2 A0 Egress STC has bug and generates interrupts */
            UTF_VERIFY_EQUAL0_STRING_MAC(numExpectedSampled + 1, numOfEvents, "numEvents");
        }
        else
        {
            /* Bobcat2 and above do not generate interrupts for STC in continuous mode */
            UTF_VERIFY_EQUAL0_STRING_MAC(0, numOfEvents, "numEvents");
        }
    }
    else if(reloadMode == PRV_TGF_STC_COUNT_RELOAD_CONTINUOUS_E)
    {
        /* NOTE: the event received also when continuous mode */
        UTF_VERIFY_EQUAL0_STRING_MAC((numExpectedSampled + 1), numOfEvents, "numEvents");
    }
    else
    {
        GT_BOOL err568346 = ( PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevsArray[stcPortIndex]) &&
                             !PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevsArray[stcPortIndex])) ? GT_TRUE : GT_FALSE;

        if(CPSS_PP_FAMILY_DXCH_ALDRIN2_E == devFamily)
        {
            /* Aldrin2 have similar bug to ERR-568346 - see Jira TXQ-2047 */
            err568346 = GT_TRUE;
        }

        if (err568346 == GT_TRUE && (stcType == PRV_TGF_STC_EGRESS_E))
        {
            /* Bobcat2 B0 has erratum ERR-568346 for Egress STC.
               It generates number of interrupts more then expected. */
            if (numOfEvents < numExpectedSampled)
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(numExpectedSampled, numOfEvents, "numEvents");
            }
        }
        else
        {
            GT_U32             portGroupId;
            GT_U32             numOfPortGroups = 0;

            if ((stcType == PRV_TGF_STC_EGRESS_E) && (PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevsArray[stcPortIndex])))
            {
                /* the reload is done in all port groups in egress STC */
                PRV_TGF_PP_START_LOOP_PORT_GROUPS_MAC(prvTgfDevsArray[stcPortIndex], portGroupId)
                {
                    numOfPortGroups++;
                }
                PRV_TGF_PP_END_LOOP_PORT_GROUPS_MAC(prvTgfDevsArray[stcPortIndex],portGroupId);
                numExpectedSampled *= numOfPortGroups;
            }

            if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevsArray[stcPortIndex]))
            {
                /* interrupts not bound/implemented */
                numExpectedSampled = 0;
            }

            UTF_VERIFY_EQUAL0_STRING_MAC(numExpectedSampled, numOfEvents, "numEvents");
        }
    }
}

/**
* @internal prvTgfStcBasicConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*
* @param[in] stcType                  - STC source type : ingress/egress
* @param[in] reloadMode               - reload mode : continuous / triggered
*                                       None
*/
GT_VOID prvTgfStcBasicConfigurationRestore
(
    IN PRV_TGF_STC_TYPE_ENT                 stcType,
    IN PRV_TGF_STC_COUNT_RELOAD_MODE_ENT    reloadMode
)
{
    GT_STATUS rc;
    GT_U32  getCountDown;/*get countdown counter (num packets till next STC)*/
    GT_U32  stcPortIndex;/* port index of the ingress/egress port */
    GT_U32  stcLimit;/* stc limit */
    GT_U32       ii;
    GT_BOOL waEnabled = GT_FALSE;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

    if(stcType == PRV_TGF_STC_INGRESS_E)
    {
        stcPortIndex = PRV_TGF_SEND_PORT_IDX_CNS;
        stcLimit = PRV_TGF_INGRESS_STC_LIMIT_CNS;
    }
    else
    {
        stcPortIndex = PRV_TGF_EGRESS_PORT_IDX_MAC();
        stcLimit = PRV_TGF_EGRESS_STC_LIMIT_CNS;
    }

    /* Eggress STC Table errata */
    rc = prvUtfDeviceFamilyGet(prvTgfDevsArray[stcPortIndex], &devFamily);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvUtfDeviceFamilyGet: %d", prvTgfDevsArray[stcPortIndex]);

    if((CPSS_PP_FAMILY_DXCH_LION_E == devFamily) && (stcType == PRV_TGF_STC_EGRESS_E))
    {
        waEnabled = GT_TRUE;
    }


    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);

    if(reloadMode == PRV_TGF_STC_COUNT_RELOAD_CONTINUOUS_E)
    {
        /*need to do cleanup to allow this test or any other STC test on this
            ingress/egress port :

            1. set trigger mode with limit 0
            2. set stcLimit packets.
                this will assure that sampling should go to CPU and not more
                'Count down' will be on this port.
            3. send stcLimit packets to force another sample the CPU.
        */
        rc = prvTgfStcReloadModeSet(stcType,PRV_TGF_STC_COUNT_RELOAD_TRIGGERED_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStcReloadModeSet");

        /*set limit on the ingress/egress port */
        rc = prvTgfStcPortLimitSet(prvTgfDevsArray[stcPortIndex],
                                   prvTgfPortsArray[stcPortIndex],
                                   stcType,
                                   0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStcPortLimitSet");

        /* send in loop of 1 to avoid lost of events in BM */
        for(ii = 0 ; ii < prvTgfBurstCount ; ii++)
        {
            /* send Packets */
            rc = prvTgfStartTransmitingEth(prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                    prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        }

        /*get countdown counter (num packets till next STC)*/
        rc = prvTgfStcPortCntrGet(prvTgfDevsArray[stcPortIndex],
                                   prvTgfPortsArray[stcPortIndex],
                                   stcType,
                                   &getCountDown);
        if (!waEnabled)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStcPortCntrGet");
            UTF_VERIFY_EQUAL0_STRING_MAC(0,getCountDown,"mismatch Limit");
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_SUPPORTED, rc, prvTgfDevsArray[stcPortIndex]);
        }
    }

    /* disable ingress/egress STC in the device */
    rc = prvTgfStcEnableSet(stcType,GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStcEnableSet");

    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);


    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);


    TGF_PARAM_NOT_USED(stcLimit); /* prevent warning: not used variable */
}



