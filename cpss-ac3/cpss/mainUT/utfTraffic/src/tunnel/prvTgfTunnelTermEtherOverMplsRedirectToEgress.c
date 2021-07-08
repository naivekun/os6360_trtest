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
* @file prvTgfTunnelTermEtherOverMplsRedirectToEgress.c
*
* @brief Tunnel term Ethernet over Mpls redirect to egress
*
* @version   26
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
#include <common/tgfIpGen.h>
#include <common/tgfTunnelGen.h>
#include <tunnel/prvTgfTunnelTermEtherOverMplsRedirectToEgress.h>
#include <common/tgfPacketGen.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_VLANID_5_CNS            5

/* default VLAN Id */
#define PRV_TGF_VLANID_6_CNS            6

/* vlan 3000 to have only 'egress port' , and not anyware alse */
#define PRV_TGF_VLANID_3000_CNS         3000

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number to receive traffic from  */
#define PRV_TGF_EGR_PORT_IDX_CNS        1

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

/* MAC2ME entry index */
#define PRV_TGF_MAC_TO_ME_INDEX_CNS       1

/* Length of packet */
#define PRV_TGF_PACKET_LEN_CNS          96

/* Length of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

#define  BURST_COUNT_CNS    3

/* Parameters needed to be restored */
static GT_U32                            portsArrayForRestore[PRV_TGF_MAX_PORTS_NUM_CNS];

/* default number of packets to send */
static GT_U32 prvTgfBurstCount = BURST_COUNT_CNS;
/*do we test packets that ingress with DSA tag*/
static GT_BOOL tgfIngressIsCascadePort = GT_FALSE;
/* egress port vlan tag state */
static PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT egressPortVlanTagState  = PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E;

/* Whether expected packet is tagged */
static GT_BOOL passengerIsTagged = GT_TRUE;

/* Ethernet over Mpls */
TGF_PACKET_STC prvTgfEthernetOverMplsPacketInfo;

/* the TTI Rule index */
static GT_U32        prvTgfTtiRuleIndex        = 0;


/* types of protocols over MPLS (field in the tti key)*/
typedef enum{
    IPV4_OVER_MPLS_PROTOCOL_E,
    IPV6_OVER_MPLS_PROTOCOL_E,
    ETHERNET_OVER_MPLS_PROTOCOL_E
}PROTOCOL_OVER_MPLS_PROTOCOL_ENT;

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_TTI_MAC_MODE_ENT      macMode;
    GT_U8                         udbOfsetArray[PRV_TGF_TTI_MAX_UDB_CNS];
    PRV_TGF_TTI_OFFSET_TYPE_ENT   udbOffsetTypeArray[PRV_TGF_TTI_MAX_UDB_CNS];
    GT_U32                        pclIdOrig;
} prvTgfRestoreCfg;

static TGF_DSA_DSA_FORWARD_STC  prvTgfPacketDsaTagPart_forward = {
    PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E /*GT_TRUE*/ , /*srcIsTagged*/ /* set to GT_TRUE for the copy to CPU , to be with vlan tag 0 */
    8,/*srcHwDev*/
    GT_FALSE,/* srcIsTrunk */
    /*union*/ /* union fields are set in runtime due to 'Big endian' / 'little endian' issues */
    {
        /*trunkId*/
        5/*portNum*/
    },/*source;*/

    13,/*srcId*/

    GT_TRUE,/*egrFilterRegistered*/
    GT_FALSE,/*wasRouted*/
    51,/*qosProfileIndex*/

    /*CPSS_INTERFACE_INFO_STC         dstInterface*/
    {
        CPSS_INTERFACE_PORT_E,/*type*/

        /*struct*/{
            0,/*devNum*/
            15/*portNum*/
        },/*devPort*/

        0,/*trunkId*/
        0, /*vidx*/
        0,/*vlanId*/
        0,/*devNum*/
        0,/*fabricVidx*/
        0 /*index*/
    },/*dstInterface*/
    GT_FALSE,/*isTrgPhyPortValid*/
    0,/*dstEport --> filled in runtime */
    3,/*tag0TpidIndex*/
    GT_FALSE,/*origSrcPhyIsTrunk*/
    /* union */ /* union fields are set in runtime due to 'Big endian' / 'little endian' issues */
    {
        /*trunkId*/
        19/*portNum*/
    },/*origSrcPhy*/
    GT_FALSE,/*phySrcMcFilterEnable*/
    0, /* hash */
    GT_TRUE /*skipFdbSaLookup*/
};

static TGF_PACKET_DSA_TAG_STC  prvTgfPacketDsaTagPart = {
    TGF_DSA_CMD_FORWARD_E ,/*dsaCommand*/
    TGF_DSA_2_WORD_TYPE_E ,/*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,/*vpt*/
        0,/*cfiBit*/
        0,/*vid --> filled in runtime */
        GT_FALSE,/*dropOnSource*/
        GT_FALSE/*packetIsLooped*/
    }/*commonParams*/

/*    union{
        TGF_DSA_DSA_TO_CPU_STC         toCpu;
        TGF_DSA_DSA_FROM_CPU_STC       fromCpu;
        TGF_DSA_DSA_TO_ANALYZER_STC    toAnalyzer;
        TGF_DSA_DSA_FORWARD_STC        forward;
    }dsaInfo;*/
};

/*indicates if this test checks metadatafield numberOfMplsLabels
  valid only if this field is > 0*/
static GT_U32 tgfNumberOfLabels = 0;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelTermEtherOverMplsRedirectToEgressBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID prvTgfTunnelTermEtherOverMplsRedirectToEgressBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    TGF_PACKET_L2_STC *l2PartPtr;

    /* use L2 part */
    l2PartPtr = prvTgfEthernetOverMplsPacketInfo.partsArray[0].partPtr;


    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS, prvTgfPortsArray,
                                           NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_6_CNS, prvTgfPortsArray + 2,
                                           NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, port 1 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(l2PartPtr->daMac, PRV_TGF_VLANID_5_CNS,
                                         prvTgfDevNum, PRV_TGF_EGR_PORT_IDX_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

}


/**
* @internal prvTgfTunnelTermEtherOverMplsRedirectToEgressOldKeyTtiConfigSet function
* @endinternal
*
* @brief   Set TTI test settings:
*         - Enable port 0 for Eth lookup
*         - Set TTI rule action
*         - Set TTI rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID prvTgfTunnelTermEtherOverMplsRedirectToEgressOldKeyTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    PRV_TGF_TTI_RULE_UNT        pattern;
    PRV_TGF_TTI_RULE_UNT        mask;
    PRV_TGF_TTI_ACTION_STC      ruleAction = {0};
    GT_U8                       pclId = 0x1;

    TGF_PACKET_MPLS_STC      *mplsLabel1PartPtr;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /*use mpls label 1 part*/
    mplsLabel1PartPtr = prvTgfEthernetOverMplsPacketInfo.partsArray[6].partPtr;

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &pattern, 0, sizeof(pattern));
    cpssOsMemSet((GT_VOID*) &mask,    0, sizeof(mask));

    /* AUTODOC: enable the TTI lookup for PRV_TGF_TTI_KEY_MPLS_E at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_MPLS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* configure TTI rule action ,Pattern ,Mask */
    /* build legacy MPLS style rule */
    prvTgfTunnelTermEthernetOverMplsTtiBasicRule(pclId,prvTgfEthernetOverMplsPacketInfo.partsArray[0].partPtr,PRV_TGF_VLANID_5_CNS,mplsLabel1PartPtr,
        &ruleAction,&pattern,&mask);


    rc = prvTgfTtiRuleSet(prvTgfTtiRuleIndex, PRV_TGF_TTI_KEY_MPLS_E,
                          &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");

    /* AUTODOC: save pclId for config restore */
    rc = prvTgfTtiPclIdGet(prvTgfDevNum, PRV_TGF_TTI_KEY_MPLS_E, &prvTgfRestoreCfg.pclIdOrig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdGet: %d", prvTgfDevNum);

    /* AUTODOC: set pclId for mpls key */
    rc = prvTgfTtiPclIdSet(prvTgfDevNum, PRV_TGF_TTI_KEY_MPLS_E, pclId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet: %d", prvTgfDevNum);
}


/**
* @internal prvTgfTunnelTermEtherOverMplsRedirectToEgressCheckCaptureEgressTrafficOnPort function
* @endinternal
*
* @brief   check captured egress on specific port of the test.
*
* @param[in] passengerIsTagged        - whether the passenger expected packet is tagged
* @param[in] isTerminated             - whether the packet is terminated
*                                       None
*/
static void prvTgfTunnelTermEtherOverMplsRedirectToEgressCheckCaptureEgressTrafficOnPort
(
    GT_BOOL     passengerIsTagged,
    GT_BOOL     isTerminated
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_PACKET_STC expectedPacketInfo;
    GT_U32  actualCapturedNumOfPackets;/*actual number of packet captured from the interface*/
    GT_U32  ii;
    GT_VOID*    outerPassengerVlanPartPtr;
    GT_VOID*    innerPassengerVlanPartPtr;
    GT_BOOL forceNewDsaToCpu = GT_FALSE;/* when 'ForceNewDsaToCpu' the extra tag in addition to the DSA exists anyway ,
                   but the DSA tag info will state 'not tagged' so the CPSS will
                   remove the extra tag ! , and we will get the actual original packet
                   with out any additional tags
                   The flag is relevant for : xcat2,lion,2,3,bobcat2. */
    TGF_PACKET_VLAN_TAG_STC vlanTag_0x8100;

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    PRV_UTF_LOG1_MAC("check Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* copy the expected packet from the ingress packet */
    expectedPacketInfo.numOfParts = prvTgfEthernetOverMplsPacketInfo.numOfParts;
    expectedPacketInfo.totalLen = prvTgfEthernetOverMplsPacketInfo.totalLen;
    /* allocate proper memory for the 'expected' parts*/
    expectedPacketInfo.partsArray =
        cpssOsMalloc(expectedPacketInfo.numOfParts * sizeof(TGF_PACKET_PART_STC));
    if(expectedPacketInfo.partsArray == NULL)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_PTR,
                                     "cpssOsMalloc: failed");
        return;
    }

    /* copy expected parts from the original sent parts */
    for(ii = 0 ; ii < expectedPacketInfo.numOfParts ; ii++)
    {
        expectedPacketInfo.partsArray[ii] = prvTgfEthernetOverMplsPacketInfo.partsArray[ii];
    }

    if (isTerminated)
    {
         /* remove the tunnel part until we are in the passenger part */
        ii = 0; /* remove first L2 from tunnel */
        expectedPacketInfo.partsArray[ii].type = TGF_PACKET_PART_SKIP_E;
        expectedPacketInfo.partsArray[ii].partPtr = NULL;
        ii++;
        while (expectedPacketInfo.partsArray[ii].type != TGF_PACKET_PART_L2_E)
        {
            expectedPacketInfo.partsArray[ii].type = TGF_PACKET_PART_SKIP_E;
            expectedPacketInfo.partsArray[ii].partPtr = NULL;
            ii++;
        }

        if(tgfIngressIsCascadePort == GT_TRUE)
        {
#ifdef CHX_FAMILY
            rc = cpssDxChBrgVlanForceNewDsaToCpuEnableGet(prvTgfDevNum,&forceNewDsaToCpu);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChBrgVlanForceNewDsaToCpuEnableGet: failed \n");
#endif /*CHX_FAMILY*/

            /* the ingress passenger packet is with outer tag 0 inner tag 1 */
            /* the egress  packet is with outer tag 1 inner tag 0 */
            /* so need to 'swap' those 2 tags */

            ii = 0;
            while (ii < expectedPacketInfo.numOfParts)
            {
                if (expectedPacketInfo.partsArray[ii].type == TGF_PACKET_PART_VLAN_TAG_E)
                {
                    break;
                }
                ii++;
            }

            /*we got to first vlan tag swap with the next */
            outerPassengerVlanPartPtr = expectedPacketInfo.partsArray[ii].partPtr;
            innerPassengerVlanPartPtr = expectedPacketInfo.partsArray[ii+1].partPtr;
            switch(egressPortVlanTagState)
            {
                case PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E:
                    expectedPacketInfo.partsArray[ii].type = TGF_PACKET_PART_SKIP_E;
                    expectedPacketInfo.partsArray[ii+1].type = TGF_PACKET_PART_SKIP_E;
                    break;
                case PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E:
                    expectedPacketInfo.partsArray[ii+1].type = TGF_PACKET_PART_SKIP_E;
                    break;
                case PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E:
                    expectedPacketInfo.partsArray[ii].type = TGF_PACKET_PART_SKIP_E;
                    break;
                case PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E:
                    /* like ingress --> no modifications */
                    break;
                case PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E:
                    expectedPacketInfo.partsArray[ii].partPtr   = innerPassengerVlanPartPtr;
                    expectedPacketInfo.partsArray[ii+1].partPtr = outerPassengerVlanPartPtr;
                    break;
                case PRV_TGF_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E:
                    /* like ingress --> no modifications */
                    break;
                default:
                rc = GT_NOT_IMPLEMENTED;
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                             "prvTgfTunnelTermEtherOverMplsRedirectToEgressCheckCaptureEgressTrafficOnPort:"
                                             " egressPortVlanTagState = %d, rc = 0x%02X\n", egressPortVlanTagState,rc);
                    return;
            }


            if(GT_FALSE ==
               prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E))
            {
                /* can't set the device to use ethertypes from the ingress ... so can't keep original */
                ((TGF_PACKET_VLAN_TAG_STC*)expectedPacketInfo.partsArray[ii].partPtr)->etherType = TGF_ETHERTYPE_8100_VLAN_TAG_CNS;
                ((TGF_PACKET_VLAN_TAG_STC*)expectedPacketInfo.partsArray[ii+1].partPtr)->etherType = TGF_ETHERTYPE_8100_VLAN_TAG_CNS;
            }
            else
            if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
            {
                /* the CPU will get the vlan tags outside from the DSA */
                /* so the 'orig ethertype of outer vlan' is preserved. */

                /* !!! the capture is set to not recognize TPIDs on ingress !!!*/
            }
            else
            {
                /* the CPU will get the outer vlan tag info in the DSA */
                /* so the 'orig ethertype of outer vlan' is replaced by CPSS with '0x8100' */
                /* so need to expect 0x8100 and not 'orig' */
                vlanTag_0x8100 = *((TGF_PACKET_VLAN_TAG_STC*)expectedPacketInfo.partsArray[ii].partPtr);
                vlanTag_0x8100.etherType = TGF_ETHERTYPE_8100_VLAN_TAG_CNS;

                expectedPacketInfo.partsArray[ii].partPtr = &vlanTag_0x8100;
            }

        }
        else
        /* the passenger isn't tagged */
        if (passengerIsTagged == GT_FALSE)
        {
            ii = 0;
            while (ii < expectedPacketInfo.numOfParts)
            {
                if (expectedPacketInfo.partsArray[ii].type == TGF_PACKET_PART_VLAN_TAG_E)
                {
                    expectedPacketInfo.partsArray[ii].type = TGF_PACKET_PART_SKIP_E;
                    expectedPacketInfo.partsArray[ii].partPtr = NULL;
                }
                ii++;
            }
        }


    }

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
            &portInterface,
            &expectedPacketInfo,
            prvTgfBurstCount,/*numOfPackets*/
            0/*vfdNum*/,
            NULL /*vfdArray*/,
            NULL, /* bytesNum's skip list */
            0,    /* length of skip list */
            &actualCapturedNumOfPackets,
            NULL/*onFirstPacketNumTriggersBmpPtr*/);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                 " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);

    /* free the dynamic allocated memory */
    cpssOsFree(expectedPacketInfo.partsArray);
}

/**
* @internal prvTgfTunnelTermEtherOverMplsRedirectToEgressTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:01:02:03:34:02,
*         macSa = 00:04:05:06:07:01,
* @param[in] expectTraffic            - GT_TRUE: traffic should pass
*                                      GT_FALSE: traffic should be dropped
*                                       None
*/
GT_VOID prvTgfTunnelTermEtherOverMplsRedirectToEgressTrafficGenerate
(
    IN GT_BOOL expectTraffic
)
{
    GT_STATUS       rc = GT_OK;
    GT_U32          portsCount  = PRV_TGF_PORT_COUNT_CNS;
    GT_U32          portIter    = 0;
    GT_BOOL         isTerminated = GT_TRUE;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    GT_U32                          packetSize;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC:   send 3 Ethernet over Mpls tunneled packets from port 0 with: */
    /* AUTODOC:   DA=00:01:02:03:34:02, SA=00:04:05:06:07:01, VID=5 */


    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    &prvTgfEthernetOverMplsPacketInfo, prvTgfBurstCount, 0, NULL,
                    prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* check captured egress traffic on port */
    if (expectTraffic)
    {
        prvTgfTunnelTermEtherOverMplsRedirectToEgressCheckCaptureEgressTrafficOnPort(passengerIsTagged,isTerminated);
    }
    else
    {
        for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
        {
            GT_BOOL isOk;

            /* AUTODOC: read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            /* AUTODOC: check Tx and Rx counters */
            switch (portIter)
            {
                case PRV_TGF_SEND_PORT_IDX_CNS:
                    /* packetSize is not changed */
                    packetSize = PRV_TGF_PACKET_CRC_LEN_CNS + TGF_VLAN_TAG_SIZE_CNS;
                    expectedCntrs.goodOctetsSent.l[0] = packetSize * prvTgfBurstCount;
                    expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                    expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                    expectedCntrs.brdcPktsSent.l[0]   = 0;
                    expectedCntrs.mcPktsSent.l[0]     = 0;
                    expectedCntrs.goodOctetsRcv.l[0]  = packetSize * prvTgfBurstCount;
                    expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                    expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                    expectedCntrs.brdcPktsRcv.l[0]    = 0;
                    expectedCntrs.mcPktsRcv.l[0]      = 0;
                    break;

                default:
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
                    break;
            }

            PRV_TGF_VERIFY_COUNTERS_MAC(isOk, expectedCntrs, portCntrs);
            UTF_VERIFY_EQUAL0_STRING_MAC(isOk, GT_TRUE, "get another counters values.");

            /* print expected values if not equal */
            PRV_TGF_PRINT_DIFF_COUNTERS_MAC(isOk, expectedCntrs, portCntrs);
        }
    }

    return;
}

/**
* @internal prvTgfTunnelTermEtherOverMplsRedirectToEgressBuildPacketSet function
* @endinternal
*
* @brief   Build packet
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Packet description:
*       tunnel:
*       TGF_PACKET_PART_L2_E,
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_ETHERTYPE_E
*       TGF_PACKET_PART_MPLS_E
*       passenger:
*       TGF_PACKET_PART_L2_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_PAYLOAD_E
*
*/
GT_VOID prvTgfTunnelTermEtherOverMplsRedirectToEgressBuildPacketSet
(
    GT_VOID
)
{
    TGF_PACKET_STC          packetInfo;
    GT_STATUS               rc = GT_OK;
    GT_U32                  tpidIndex;
    GT_U32                  numfOfPassengerTags = 3;
    GT_U16                  etherType;
    GT_U16                  passnagerVlanId0;
    GT_U32  ii = 0;
    static TGF_PACKET_PART_STC partsArr[25];
    TGF_PACKET_MPLS_STC     *label1PartPtr;
    TGF_PACKET_MPLS_STC     *label2PartPtr;
    TGF_PACKET_MPLS_STC     *label3PartPtr;
    TGF_PACKET_MPLS_STC     *label4PartPtr;

    /* AUTODOC: save prvTgfPortsArray */
    cpssOsMemCpy(portsArrayForRestore, prvTgfPortsArray,
                 sizeof(portsArrayForRestore));

    if(tgfIngressIsCascadePort == GT_TRUE)
    {
        /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
         * configured cascade) are not remote ports */
        if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
        {
            rc = prvTgfDefPortsArrange(GT_FALSE,
                                       PRV_TGF_SEND_PORT_IDX_CNS,
                                       -1);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
        }
    }

    if(tgfIngressIsCascadePort == GT_TRUE)
    {
        prvTgfBurstCount = 1;
        prvTgfPacketNumberOfVlanTagsSet(GT_FALSE/*tunnel*/,1);
        prvTgfPacketNumberOfVlanTagsSet(GT_TRUE/*passenger*/,numfOfPassengerTags);
    }
    else
    {
        prvTgfPacketNumberOfVlanTagsSet(GT_TRUE/*passenger*/,0);
    }

    if(tgfNumberOfLabels > 0)
    {
        PRV_UTF_LOG1_MAC("======= Checking Ethernet over MPLS with [%d] labels =======\n",tgfNumberOfLabels);
        prvTgfPacketNumberOfMplsLabelsSet(tgfNumberOfLabels);
    }

    /* AUTODOC: get default Eth over Mpls packet */
    /*rc = prvTgfEthernetOverMplsPacketDefaultPacketGet(&packetInfo.numOfParts, &packetInfo.partsArray);*/
    rc = prvTgfPacketEthernetOverMplsPacketDefaultPacketGet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS,&packetInfo.numOfParts, &packetInfo.partsArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthernetOverMplsPacketDefaultPacketGet");

    prvTgfEthernetOverMplsPacketInfo.numOfParts = packetInfo.numOfParts;
    prvTgfEthernetOverMplsPacketInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    switch (tgfNumberOfLabels)
    {
    case 2:
        /* change label1 part */
        label1PartPtr = packetInfo.partsArray[6].partPtr;
        label1PartPtr->stack = 0;

        /* AUTODOC: change label1 of tunnel */
        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,0,label1PartPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

        /* change label2 part */
        label2PartPtr = packetInfo.partsArray[7].partPtr;
        label2PartPtr->stack = 1;

        /* AUTODOC: change label2 of tunnel */
        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,1,label2PartPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");
        break;

    case 3:
        /* change label1 part */
        label1PartPtr = packetInfo.partsArray[6].partPtr;
        label1PartPtr->stack = 0;

        /* AUTODOC: change label1 of tunnel */
        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,0,label1PartPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

        /* change label2 part */
        label2PartPtr = packetInfo.partsArray[7].partPtr;
        label2PartPtr->stack = 0;

        /* AUTODOC: change label2 of tunnel */
        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,1,label2PartPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

        /* change label3 part */
        label3PartPtr = packetInfo.partsArray[8].partPtr;
        label3PartPtr->stack = 1;

        /* AUTODOC: change label3 of tunnel */
        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,2,label3PartPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");
        break;

    case 4:
        /* change label1 part */
        label1PartPtr = packetInfo.partsArray[6].partPtr;
        label1PartPtr->stack = 0;

        /* AUTODOC: change label1 of tunnel */
        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,0,label1PartPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

        /* change label2 part */
        label2PartPtr = packetInfo.partsArray[7].partPtr;
        label2PartPtr->stack = 0;

        /* AUTODOC: change label2 of tunnel */
        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,1,label2PartPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

        /* change label3 part */
        label3PartPtr = packetInfo.partsArray[8].partPtr;
        label3PartPtr->stack = 0;

        /* AUTODOC: change label3 of tunnel */
        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,2,label3PartPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

        /* change label4 part */
        label4PartPtr = packetInfo.partsArray[9].partPtr;
        label4PartPtr->stack = 1;

        /* AUTODOC: change label4 of tunnel */
        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,3,label4PartPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");
        break;

    default:
        /* do nothing */
        break;
    }


    /* copy array to local array , to not modify the pointers inside the DB !!! */
    for(ii = 0 ; ii < prvTgfEthernetOverMplsPacketInfo.numOfParts ; ii++)
    {
        partsArr[ii] = packetInfo.partsArray[ii];
    }
    prvTgfEthernetOverMplsPacketInfo.partsArray =  partsArr;

    if(tgfIngressIsCascadePort == GT_TRUE)
    {
        PRV_UTF_LOG0_MAC("======= Start DSA test related Configuration =======\n");

        for(ii = 0 ; ii < prvTgfEthernetOverMplsPacketInfo.numOfParts ; ii++)
        {
            if(prvTgfEthernetOverMplsPacketInfo.partsArray[ii].type == TGF_PACKET_PART_VLAN_TAG_E)
            {
                break;
            }
        }

        if(ii == prvTgfEthernetOverMplsPacketInfo.numOfParts)
        {
            rc = GT_GET_ERROR;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTunnelTermEtherOverMplsRedirectToEgressBuildPacketSet");
            return;
        }

        /* AUTODOC: replace the first vlan tag (in the tunnel) with DSA tag with the same info */
        prvTgfPacketDsaTagPart.commonParams.vid =
            ((TGF_PACKET_VLAN_TAG_STC*)prvTgfEthernetOverMplsPacketInfo.partsArray[ii].partPtr)->vid;
        prvTgfPacketDsaTagPart.commonParams.vpt =
            ((TGF_PACKET_VLAN_TAG_STC*)prvTgfEthernetOverMplsPacketInfo.partsArray[ii].partPtr)->pri;
        prvTgfPacketDsaTagPart.commonParams.cfiBit =
            ((TGF_PACKET_VLAN_TAG_STC*)prvTgfEthernetOverMplsPacketInfo.partsArray[ii].partPtr)->cfi;

        prvTgfEthernetOverMplsPacketInfo.partsArray[ii].type = TGF_PACKET_PART_DSA_TAG_E;
        prvTgfEthernetOverMplsPacketInfo.partsArray[ii].partPtr = &prvTgfPacketDsaTagPart;

        /* NOTE: the 'Big Endian' is building the 'unions' in 'occurred' way ,
            so we must EXPLICITLY set 'union fields' to needed value */
        prvTgfPacketDsaTagPart_forward.source.portNum = 5;
        prvTgfPacketDsaTagPart_forward.origSrcPhy.portNum = 19;

        /* set dummy 'used vidx' because need to check the the TTI action will
           override it properly to redirect to egress port */
        prvTgfPacketDsaTagPart_forward.dstInterface.type = CPSS_INTERFACE_VIDX_E;
        prvTgfPacketDsaTagPart_forward.dstInterface.vidx = 0x543;

        /* do this right before the send of packet */
        /* bind the DSA tag FORWARD part (since union is used .. can't do it in compilation time) */
        prvTgfPacketDsaTagPart.dsaInfo.forward = prvTgfPacketDsaTagPart_forward;

        /* look for first MPLS (so will pass the vlan tags of the tunnel) */
        for(ii = 0 ; ii < prvTgfEthernetOverMplsPacketInfo.numOfParts ; ii++)
        {
            if(prvTgfEthernetOverMplsPacketInfo.partsArray[ii].type == TGF_PACKET_PART_MPLS_E)
            {
                break;
            }
        }

        if(ii == prvTgfEthernetOverMplsPacketInfo.numOfParts)
        {
            rc = GT_GET_ERROR;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTunnelTermEtherOverMplsRedirectToEgressBuildPacketSet");
            return;
        }

        /* look for first vlan tag in the passenger */
        for(/*continue*/ ; ii < prvTgfEthernetOverMplsPacketInfo.numOfParts ; ii++)
        {
            if(prvTgfEthernetOverMplsPacketInfo.partsArray[ii].type == TGF_PACKET_PART_VLAN_TAG_E)
            {
                break;
            }
        }

        if(ii == prvTgfEthernetOverMplsPacketInfo.numOfParts)
        {
            rc = GT_GET_ERROR;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTunnelTermEtherOverMplsRedirectToEgressBuildPacketSet");
            return;
        }

        tpidIndex = 1;/*start from index 1 , leave 0 alone */
        passnagerVlanId0 = PRV_TGF_VLANID_3000_CNS;
        /* set the most outer tag of the passenger with this VID */
        ((TGF_PACKET_VLAN_TAG_STC*)prvTgfEthernetOverMplsPacketInfo.partsArray[ii].partPtr)->vid = passnagerVlanId0;

        for(/*continue*/ ; ii < prvTgfEthernetOverMplsPacketInfo.numOfParts ; ii++)
        {
            if(prvTgfEthernetOverMplsPacketInfo.partsArray[ii].type == TGF_PACKET_PART_VLAN_TAG_E)
            {
                etherType = ((TGF_PACKET_VLAN_TAG_STC*)prvTgfEthernetOverMplsPacketInfo.partsArray[ii].partPtr)->etherType;

                if(etherType != TGF_ETHERTYPE_8100_VLAN_TAG_CNS)
                {
                    /*for ingress : set TPIDs for the tags of passenger */
                    rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,tpidIndex,etherType);
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

                    /*for egress : set TPIDs for the egress packet */
                    rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_EGRESS_E,tpidIndex,etherType);
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

                    tpidIndex++;
                }
            }
        }

        /* AUTODOC:set ingress port as cascade port */
        rc = tgfTrafficGeneratorIngressCscdPortEnableSet(
            prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            GT_TRUE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* AUTODOC: create VLAN 3000 with egress port index [1] */
        rc = prvTgfBrgDefVlanEntryWithPortsSet(passnagerVlanId0, &prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                               NULL, NULL, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

        /* AUTODOC: set egress port to be 'outer tag1 inner tag0' in the vlan that come from the Tag 0 of the passenger */
        /* AUTODOC: meaning that egress packet will have 'swapped' 2 outer tags */
        rc = prvTgfBrgVlanTagMemberAdd(passnagerVlanId0, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                               egressPortVlanTagState);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

        if(GT_FALSE ==
           prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E))
        {
            /* can't set the device to use ethertypes from the ingress ... so can keep original */
        }
        else
        {
            /* AUTODOC: for SIP5 set egress port to be with the same ethertypes as ingress recognized */
            rc = prvTgfBrgVlanEgressTagTpidSelectModeSet(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                CPSS_VLAN_ETHERTYPE0_E,
                PRV_TGF_BRG_VLAN_VID_TO_TPID_SELECT_TABLE_MODE_E);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfBrgVlanEgressTagTpidSelectModeSet: %d", prvTgfDevNum);

            rc = prvTgfBrgVlanEgressTagTpidSelectModeSet(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                CPSS_VLAN_ETHERTYPE1_E,
                PRV_TGF_BRG_VLAN_VID_TO_TPID_SELECT_TABLE_MODE_E);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfBrgVlanEgressTagTpidSelectModeSet: %d", prvTgfDevNum);
        }
    }


}

/**
* @internal prvTgfTunnelTermEtherOverMplsRedirectToEgressTtiConfigSet function
* @endinternal
*
* @brief   Set TTI test settings:
*         - Enable port 0 for Eth lookup
*         - Set Eth key lookup MAC mode to Mac DA
*         - Set TTI rule action
*         - Set TTI rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID prvTgfTunnelTermEtherOverMplsRedirectToEgressTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_STC   ruleAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;
    GT_U8                    pclId = 0x1;
    TGF_PACKET_MPLS_STC      *mplsLabel1PartPtr;
    PRV_TGF_TTI_RULE_UNT     ttiPattern_forUdbs;
    PRV_TGF_TTI_RULE_UNT     ttiMask_forUdbs;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /*use mpls label 1 part*/
    mplsLabel1PartPtr = prvTgfEthernetOverMplsPacketInfo.partsArray[6].partPtr;


     /* AUTODOC: enable the TTI lookup for PRV_TGF_TTI_KEY_UDB_MPLS_E at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_UDB_MPLS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");


    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for Mpls TCAM location */

    /* init TTI Pattern */
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    /* init TTI Mask */
    cpssOsMemSet(&ttiMask, 0xFF, sizeof(ttiMask));
    /* init Action */
    cpssOsMemSet(&ruleAction, 0, sizeof(ruleAction));

    /* configure TTI rule action ,Pattern ,Mask */
    /* build legacy MPLS style rule */
    prvTgfTunnelTermEthernetOverMplsTtiBasicRule(pclId,prvTgfEthernetOverMplsPacketInfo.partsArray[0].partPtr,PRV_TGF_VLANID_5_CNS,mplsLabel1PartPtr, /*&l2PartPtr->daMac*/
        &ruleAction,&ttiPattern,&ttiMask);

    /* convert the legacy mpls pattern/mask to UDBs style pattern/mask */
    rc = prvTgfTunnelTermEmulateLegacyMplsForUdbOnlyMplsPatternMaskBuild(
            &ttiPattern,
            &ttiMask ,
            &ttiPattern_forUdbs ,
            &ttiMask_forUdbs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelTermEmulateLegacyIpv4ForUdbOnlyIpv4PatternMaskBuild: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("======= Setting UDB Configuration =======\n");

    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E , PRV_TGF_TTI_KEY_SIZE_30_B_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    rc = prvTgfTtiRuleSet(prvTgfTtiRuleIndex,PRV_TGF_TTI_KEY_UDB_MPLS_E, &ttiPattern_forUdbs, &ttiMask_forUdbs, &ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");

    /* set the UDBs that needed for the 'traffic type' MPLS to get key like the legacy MPLS */
    prvTgfTunnelTermEmulateLegacyMplsForUdbOnlyMplsKeyNeededUdbsSet(PRV_TGF_TTI_KEY_UDB_MPLS_E);

    rc = prvTgfTtiPclIdSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_MPLS_E, pclId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet: %d", prvTgfDevNum);

}

/**
* @internal prvTgfTunnelTermEtherOverMplsRedirectToEgressConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] useUdbConf               - tti rule is udb
*                                       None
*
* @note 1. Restore TTI Configuration
*       2. Restore Base Configuration
*
*/
GT_VOID prvTgfTunnelTermEtherOverMplsRedirectToEgressConfigurationRestore
(
    GT_BOOL     useUdbConf
)
{

    GT_STATUS   rc        = GT_OK;
    GT_U32                  tpidIndex;
    GT_U16                  etherType;
    GT_U16                  passnagerVlanId0 = PRV_TGF_VLANID_3000_CNS;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 1. Restore TTI Configuration
     */

    /* AUTODOC: invalidate TTI rule 1 */
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);


    /* AUTODOC: restore the lookup Mac mode for IPv4 and DA */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_MPLS_E, prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);


    /* AUTODOC: disable the TTI lookup for IPv4 at port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_MPLS_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    if (useUdbConf == GT_TRUE)
    {
        /* AUTODOC: disable the TTI lookup for UDB IPv4_Other at port 0 */
        rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],PRV_TGF_TTI_KEY_UDB_MPLS_E,GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);
    }
    else
    {
        /* AUTODOC: restore pclId for mpls key */
        rc = prvTgfTtiPclIdSet(prvTgfDevNum, PRV_TGF_TTI_KEY_MPLS_E, prvTgfRestoreCfg.pclIdOrig);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet: %d", prvTgfDevNum);


        /* AUTODOC: disable the TTI lookup for UDB IPv4_Other at port 0 */
        rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_MPLS_E,GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);
    }



    /* -------------------------------------------------------------------------
     * 2. Restore Base Configuration
     */

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: restore default Precedence on port 0 */
    rc = prvTgfBrgVlanPortVidPrecedenceSet(prvTgfDevNum,
                                           prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                           CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidPrecedenceSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E);

    /* AUTODOC: invalidate VLAN entry 6 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_6_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /* AUTODOC: invalidate VLAN entry 5 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    if(tgfIngressIsCascadePort == GT_TRUE)
    {
        /*restore value*/
        prvTgfBurstCount = BURST_COUNT_CNS;

        /* AUTODOC: unset ingress port as cascade port */
        rc = tgfTrafficGeneratorIngressCscdPortEnableSet(
            prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            GT_FALSE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* AUTODOC: set 0x8100 to the 4 ingress egress TPID indexes 1..4 (inclusive)*/
        tpidIndex = 1;/*start from index 1 , leave 0 alone */
        for(tpidIndex = 1 ; tpidIndex < 5 ; tpidIndex++)
        {
            etherType = TGF_ETHERTYPE_8100_VLAN_TAG_CNS;
            rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,tpidIndex,etherType);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            /*for egress : set TPIDs for the egress packet */
            rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_EGRESS_E,tpidIndex,etherType);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }

        /* AUTODOC: invalidate VLAN entry 3000 */
        rc = prvTgfBrgVlanEntryInvalidate(passnagerVlanId0);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);


        if(GT_FALSE ==
           prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E))
        {
            /* can't set the device to use ethertypes from the ingress ... so nothing to restore */
        }
        else
        {
            /* AUTODOC: sip5 set egress port to be with own TPID not related to ingress recognition  */
            rc = prvTgfBrgVlanEgressTagTpidSelectModeSet(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                CPSS_VLAN_ETHERTYPE0_E,
                PRV_TGF_BRG_VLAN_EGRESS_TAG_TPID_SELECT_MODE_E);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfBrgVlanEgressTagTpidSelectModeSet: %d", prvTgfDevNum);

            rc = prvTgfBrgVlanEgressTagTpidSelectModeSet(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                CPSS_VLAN_ETHERTYPE1_E,
                PRV_TGF_BRG_VLAN_EGRESS_TAG_TPID_SELECT_MODE_E);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfBrgVlanEgressTagTpidSelectModeSet: %d", prvTgfDevNum);
        }
    }


    /* number of MPLS labels in the tunnel header is '0' */
    prvTgfTunnelTermEtherOverMplsRedirectToEgressNumOfMplsLabels(0);

    /* default setting: this is not DSA test */
    prvTgfTunnelTermEtherOverMplsRedirectToEgressDsaUse(GT_FALSE);

    /*Sets default values for global parametsers*/
    prvTgfPacketRestoreDefaultParameters();

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, portsArrayForRestore, sizeof(portsArrayForRestore));

    PRV_UTF_LOG0_MAC("======= End of Restoring Configuration =======\n");

}

#ifdef CHX_FAMILY
/* hook into CPSS internal logic */
extern GT_STATUS  sip5BuildUdbsNeededForLegacyKey_mpls
(
    IN GT_U8   devNum,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT keyType
);

extern GT_STATUS sip5TtiConfigSetLogic2HwUdbFormat
(
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT         ruleType,
    IN  GT_BOOL                             isPattern,
    IN  CPSS_DXCH_TTI_RULE_UNT             *logicFormatPtr,
    OUT GT_U32                             *hwFormatArray
);

extern GT_STATUS prvTgfTtiRuleConvertGenericToDx
(
    IN  PRV_TGF_TTI_RULE_TYPE_ENT      ruleType,
    IN  PRV_TGF_TTI_RULE_UNT         *patternPtr,
    IN  PRV_TGF_TTI_RULE_UNT         *maskPtr,
    OUT CPSS_DXCH_TTI_RULE_UNT       *dxChPatternPtr,
    OUT CPSS_DXCH_TTI_RULE_UNT       *dxChMaskPtr
);


#endif /*CHX_FAMILY*/


/**
* @internal prvTgfTunnelTermEmulateLegacyMplsForUdbOnlyMplsPatternMaskBuild function
* @endinternal
*
* @brief   build the pattern/mask UDBs that need to be emulated like legacy MPLS pattern/mask.
*
* @param[out] udbPatternArray          - UDBs style pattern
* @param[out] udbMaskArray             - UDBs style mask
*                                       None
*/
GT_STATUS prvTgfTunnelTermEmulateLegacyMplsForUdbOnlyMplsPatternMaskBuild
(
    IN   PRV_TGF_TTI_RULE_UNT   *legacyMplsPatternPtr,
    IN   PRV_TGF_TTI_RULE_UNT   *legacyMplsMaskPtr,
    OUT  PRV_TGF_TTI_RULE_UNT   *udbPatternArray,
    OUT  PRV_TGF_TTI_RULE_UNT   *udbMaskArray
)
{
#ifndef CHX_FAMILY
    return GT_OK;
#else  /*!CHX_FAMILY*/
    GT_STATUS   rc;
    GT_U32   hwFormatArray[32];
    GT_U32   ii;
    CPSS_DXCH_TTI_RULE_UNT dxchPattern;
    CPSS_DXCH_TTI_RULE_UNT dxchMask;
    GT_U32  tmpValue;

    /* clear the array */
    cpssOsMemSet(udbPatternArray, 0, sizeof(PRV_TGF_TTI_RULE_UNT));
    cpssOsMemSet(udbMaskArray, 0, sizeof(PRV_TGF_TTI_RULE_UNT));

    /* convert from the TGF format to the DXCH format */
    prvTgfTtiRuleConvertGenericToDx(PRV_TGF_TTI_RULE_MPLS_E,
            legacyMplsPatternPtr,legacyMplsMaskPtr,
            &dxchPattern,&dxchMask);

    /* for pattern - ask CPSS to build words array for 'legacy Mpls'  */
    rc = sip5TtiConfigSetLogic2HwUdbFormat(CPSS_DXCH_TTI_RULE_MPLS_E, GT_TRUE ,
        &dxchPattern,
        hwFormatArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    for(ii = 0 ; ii < CPSS_DXCH_TTI_MAX_UDB_CNS; ii++)
    {
        U32_GET_FIELD_IN_ENTRY_MAC(hwFormatArray,8*ii , 8,tmpValue);
        udbPatternArray->udbArray[ii] = (GT_U8)tmpValue;
    }

    /* for mask - ask CPSS to build words array for 'legacy MPLS'  */
    rc = sip5TtiConfigSetLogic2HwUdbFormat(CPSS_DXCH_TTI_RULE_MPLS_E, GT_TRUE ,
        &dxchMask,
        hwFormatArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    for(ii = 0 ; ii < CPSS_DXCH_TTI_MAX_UDB_CNS; ii++)
    {
        U32_GET_FIELD_IN_ENTRY_MAC(hwFormatArray,8*ii , 8,tmpValue);
        udbPatternArray->udbArray[ii] = (GT_U8)tmpValue;
    }

    return GT_OK;
#endif /*!CHX_FAMILY*/
}

/**
* @internal prvTgfTunnelTermEthernetOverMplsTtiBasicRule function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @param[in] pclId                    - pclId
* @param[in] macAddrPtr               - pointer to mac address
* @param[in] vid                      - vlan id
* @param[in] mplsLabel1PartPtr        - pointer to label 1
* @param[in,out] ttiActionPtr             - (pointer to) tti action
* @param[in,out] ttiPatternPtr            - (pointer to) tti pattern
* @param[in,out] ttiMaskPtr               - (pointer to) tti mask
* @param[in,out] ttiActionPtr             - (pointer to) tti action
* @param[in,out] ttiPatternPtr            - (pointer to) tti pattern
* @param[in,out] ttiMaskPtr               - (pointer to) tti mask
*                                       None
*/
GT_VOID prvTgfTunnelTermEthernetOverMplsTtiBasicRule
(
    IN GT_U8                       pclId,
    IN TGF_MAC_ADDR                *macAddrPtr,
    IN GT_U16                       vid,
    IN TGF_PACKET_MPLS_STC          *mplsLabel1PartPtr,
    INOUT PRV_TGF_TTI_ACTION_STC   *ttiActionPtr,
    INOUT PRV_TGF_TTI_RULE_UNT     *ttiPatternPtr,
    INOUT PRV_TGF_TTI_RULE_UNT     *ttiMaskPtr
)
{
    TGF_PARAM_NOT_USED(macAddrPtr);/*hide compiler's warning*/
     /* AUTODOC: set the TTI Rule Pattern, Mask and Action for Mpls TCAM location */

    /* configure TTI rule action */
    ttiActionPtr->tunnelTerminate       = GT_TRUE;
    ttiActionPtr->passengerPacketType   = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
    ttiActionPtr->tsPassengerPacketType = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    ttiActionPtr->copyTtlFromTunnelHeader = GT_FALSE;

    ttiActionPtr->command         = CPSS_PACKET_CMD_FORWARD_E;
    ttiActionPtr->redirectCommand = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiActionPtr->bridgeBypass    = GT_TRUE;

    ttiActionPtr->interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    ttiActionPtr->interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    ttiActionPtr->interfaceInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    ttiActionPtr->tunnelStart    = GT_FALSE;
    ttiActionPtr->tunnelStartPtr = 0;

    if(tgfIngressIsCascadePort == GT_TRUE)
    {
        /* indication to use the vlan tags of the passenger and not those of the tunnel */
        /* seems much better then 'PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E' */
        ttiActionPtr->tag0VlanCmd = PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E;
    }
    else
    {
        ttiActionPtr->tag0VlanCmd = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    }
    ttiActionPtr->tag0VlanId  = 0;
    ttiActionPtr->tag1UpCommand  = PRV_TGF_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E;

    ttiActionPtr->modifyUpEnable = PRV_TGF_TTI_MODIFY_UP_ENABLE_E;

    ttiActionPtr->userDefinedCpuCode = CPSS_NET_UDP_BC_MIRROR_TRAP3_E;

    /* AUTODOC: set TTI Pattern */
    ttiPatternPtr->mpls.common.pclId    = pclId;
    ttiPatternPtr->mpls.common.srcIsTrunk = GT_FALSE;
    ttiPatternPtr->mpls.common.srcPortTrunk = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    ttiPatternPtr->mpls.common.vid     = vid;

    ttiPatternPtr->mpls.exp0=mplsLabel1PartPtr->exp;
    ttiPatternPtr->mpls.label0= mplsLabel1PartPtr->label;
    ttiPatternPtr->mpls.numOfLabels = 1;
    ttiPatternPtr->mpls.protocolAboveMPLS= ETHERNET_OVER_MPLS_PROTOCOL_E;

    ttiPatternPtr->mpls.common.dsaSrcIsTrunk    = GT_FALSE;
    ttiPatternPtr->mpls.common.dsaSrcPortTrunk  = 0;
    ttiPatternPtr->mpls.common.dsaSrcDevice     = prvTgfDevNum;


    /* AUTODOC: set TTI Mask */

    cpssOsMemSet(ttiMaskPtr,0,sizeof(PRV_TGF_TTI_RULE_UNT));

    ttiMaskPtr->mpls.common.pclId            = BIT_10 - 1;/* 10 bits field */
    ttiMaskPtr->mpls.common.srcIsTrunk = 1;
    ttiMaskPtr->mpls.exp0              = BIT_3  - 1;/*3 bits field */
    ttiMaskPtr->mpls.label0            = BIT_20 - 1;/*20 bits field */
}

/**
* @internal prvTgfTunnelTermEmulateLegacyMplsForUdbOnlyMplsKeyNeededUdbsSet function
* @endinternal
*
* @brief   set the needed UDBs that need to be emulated like legacy MPLS key.
*
* @param[in] keyType                  - UDB only key type, that need to be emulated like legacy MPLS key.
*                                       None
*/
GT_VOID prvTgfTunnelTermEmulateLegacyMplsForUdbOnlyMplsKeyNeededUdbsSet
(
    PRV_TGF_TTI_KEY_TYPE_ENT     keyType
)
{
#ifndef CHX_FAMILY
    return ;
#else  /*!CHX_FAMILY*/
    GT_STATUS   rc;

    /* ask CPSS to build needed UDBs in same way as the legacy "PRV_TGF_TTI_KEY_MPLS_E" */
    rc = sip5BuildUdbsNeededForLegacyKey_mpls(prvTgfDevNum, (CPSS_DXCH_TTI_KEY_TYPE_ENT)keyType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "sip5BuildUdbsNeededForLegacyKey_mpls: %d", prvTgfDevNum);

    return;
#endif /*!CHX_FAMILY*/
}


/**
* @internal prvTgfTunnelTermEtherOverMplsRedirectToEgressConfiguredUdbConfigSet function
* @endinternal
*
* @brief   Set TTI test settings:
*         - Enable port 0 for Eth lookup
*         - Set TTI rule action
*         - Set TTI rule
*         - Set Udb
*/
GT_VOID prvTgfTunnelTermEtherOverMplsRedirectToEgressConfiguredUdbConfigSet
(
    GT_VOID
)
{

    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_STC   ruleAction = {0};
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;
    GT_U8                    pclId = 0x1;
    TGF_PACKET_L2_STC        *l2PartPtr;
    TGF_PACKET_MPLS_STC      *mplsLabel1PartPtr;
    GT_U32                   ii;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* use L2 part */
    l2PartPtr = prvTgfEthernetOverMplsPacketInfo.partsArray[0].partPtr;
    /*use mpls label 1 part*/
    mplsLabel1PartPtr = prvTgfEthernetOverMplsPacketInfo.partsArray[6].partPtr;


     /* AUTODOC: enable the TTI lookup for PRV_TGF_TTI_KEY_UDB_MPLS_E at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_UDB_MPLS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");


    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for Mpls TCAM location */

    /* init TTI Pattern */
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    /* init TTI Mask */
    cpssOsMemSet(&ttiMask, 0xFF, sizeof(ttiMask));

    /* configure TTI rule action */
    ruleAction.tunnelTerminate       = GT_TRUE;
    ruleAction.passengerPacketType   = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
    ruleAction.tsPassengerPacketType = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    ruleAction.copyTtlFromTunnelHeader = GT_FALSE;

    ruleAction.command         = CPSS_PACKET_CMD_FORWARD_E;
    ruleAction.redirectCommand = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ruleAction.bridgeBypass    = GT_TRUE;

    ruleAction.interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    ruleAction.interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    ruleAction.interfaceInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    ruleAction.tunnelStart    = GT_FALSE;
    ruleAction.tunnelStartPtr = 0;

    ruleAction.tag0VlanCmd = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ruleAction.tag0VlanId  = 0;
    ruleAction.tag1UpCommand  = PRV_TGF_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E;

    ruleAction.modifyUpEnable = PRV_TGF_TTI_MODIFY_UP_ENABLE_E;

    ruleAction.userDefinedCpuCode = CPSS_NET_UDP_BC_MIRROR_TRAP3_E;



    /* set TTI Pattern */
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));

     /* PCL ID */
    ttiPattern.udbArray[0] = pclId;
    /* MAC_DA */
    cpssOsMemCpy(&(ttiPattern.udbArray[1]), l2PartPtr->daMac, sizeof(TGF_MAC_ADDR));
    /* MAC_SA */
    cpssOsMemCpy(&(ttiPattern.udbArray[7]), l2PartPtr->saMac, sizeof(TGF_MAC_ADDR));
    /*  LABEL 0
        EXP0
        S0*/
    {
        GT_U32 mplsLabel = mplsLabel1PartPtr->stack << 8 | mplsLabel1PartPtr->exp << 9 | mplsLabel1PartPtr->label<<12;

        ttiPattern.udbArray[13] = (GT_U8)(mplsLabel >> 24);
        ttiPattern.udbArray[14] = (GT_U8)(mplsLabel >> 16);
        ttiPattern.udbArray[15] = (GT_U8)(mplsLabel >>  8);

    }
    /*Reserved*/
    ttiPattern.udbArray[16] = 0;
    ttiPattern.udbArray[17] = 0;
    ttiPattern.udbArray[18] = 0;
    ttiPattern.udbArray[19] = 0;


     /* set TTI Mask */
    cpssOsMemSet(&ttiMask, 0xFFFF, sizeof(ttiMask));/*0xFFFF*/
    ttiMask.udbArray[0] = 0;
    ttiMask.udbArray[16] = 0;
    ttiMask.udbArray[17] = 0;
    ttiMask.udbArray[18] = 0;
    ttiMask.udbArray[19] = 0;


    PRV_UTF_LOG0_MAC("======= Setting UDB Configuration =======\n");

    /* AUTODOC: add TTI rule 1 with UDB key on port 0 VLAN 5 with action: */
    /* AUTODOC:   cmd REDIRECT */
    /* AUTODOC:   UDB passenger packet type */
    /* AUTODOC:   DA=00:01:02:03:34:02 */


     rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E , PRV_TGF_TTI_KEY_SIZE_20_B_E);
     UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

     rc = prvTgfTtiRuleSet(prvTgfTtiRuleIndex,PRV_TGF_TTI_RULE_UDB_20_E, &ttiPattern, &ttiMask, &ruleAction);
     UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");

     /*save udb for restore*/
     for(ii=0; ii<20; ii++)
     {
         rc = prvTgfTtiUserDefinedByteGet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,ii,&prvTgfRestoreCfg.udbOffsetTypeArray[ii],&prvTgfRestoreCfg.udbOfsetArray[ii]);/*18*/
         UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteGet: %d", prvTgfDevNum);
     }


    /* MAC_DA
       SIP
       DIP
       PCL ID
       Local Source is Trunk
       Local Source ePort/TrunkID
       eVLAN
       VLAN Tag0 Exists
       DSA SrcIsTrunk
       DSA Tag Source Port/Trunk
       DSA Tag Source Device
       Tunneling Protocol
       IsARP
       */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,0,PRV_TGF_TTI_OFFSET_METADATA_E,22);/*18*/
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,1,PRV_TGF_TTI_OFFSET_L2_E,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,2,PRV_TGF_TTI_OFFSET_L2_E,1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,3,PRV_TGF_TTI_OFFSET_L2_E,2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,4,PRV_TGF_TTI_OFFSET_L2_E,3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,5,PRV_TGF_TTI_OFFSET_L2_E,4);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,6,PRV_TGF_TTI_OFFSET_L2_E,5);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,7,PRV_TGF_TTI_OFFSET_L2_E,6);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,8,PRV_TGF_TTI_OFFSET_L2_E,7);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,9,PRV_TGF_TTI_OFFSET_L2_E,8);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,10,PRV_TGF_TTI_OFFSET_L2_E,9);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,11,PRV_TGF_TTI_OFFSET_L2_E,10);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,12,PRV_TGF_TTI_OFFSET_L2_E,11);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,13,PRV_TGF_TTI_OFFSET_MPLS_MINUS_2_E,2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,14,PRV_TGF_TTI_OFFSET_MPLS_MINUS_2_E,3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,15,PRV_TGF_TTI_OFFSET_MPLS_MINUS_2_E,4);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,16,PRV_TGFH_TTI_OFFSET_INVALID_E,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,17,PRV_TGFH_TTI_OFFSET_INVALID_E,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,18,PRV_TGFH_TTI_OFFSET_INVALID_E,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,19,PRV_TGFH_TTI_OFFSET_INVALID_E,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);


    rc = prvTgfTtiPclIdSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_MPLS_E, pclId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet: %d", prvTgfDevNum);

}


/**
* @internal prvTgfTunnelTermEtherOverMplsRedirectToEgressConfiguredUdbConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 1. Restore TTI Configuration
*       2. Restore Base Configuration
*
*/
GT_VOID prvTgfTunnelTermEtherOverMplsRedirectToEgressConfiguredUdbConfigurationRestore
(
    GT_VOID
)
{

    GT_STATUS   rc        = GT_OK;
    GT_U32      ii;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 1. Restore TTI Configuration
     */

    /* AUTODOC: invalidate TTI rule 1 */
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);


    /* AUTODOC: restore the lookup Mac mode for IPv4 and DA */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_MPLS_E, prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);


    /* AUTODOC: disable the TTI lookup for IPv4 at port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_MPLS_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);


    /* AUTODOC: disable the TTI lookup for UDB IPv4_Other at port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],PRV_TGF_TTI_KEY_UDB_MPLS_E,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);




    /* -------------------------------------------------------------------------
     * 2. Restore Base Configuration
     */

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: restore default Precedence on port 0 */
    rc = prvTgfBrgVlanPortVidPrecedenceSet(prvTgfDevNum,
                                           prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                           CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidPrecedenceSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E);

    /* AUTODOC: invalidate VLAN entry 6 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_6_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /* AUTODOC: invalidate VLAN entry 5 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /*Sets default values for global parametsers*/
    prvTgfPacketRestoreDefaultParameters();

    /* -------------------------------------------------------------------------
     * 3. Restore UDB Configuration
     */
    for(ii=0; ii<20; ii++)
    {
          rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,ii,prvTgfRestoreCfg.udbOffsetTypeArray[ii],prvTgfRestoreCfg.udbOfsetArray[ii]);
          UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    }

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
}

/**
* @internal prvTgfTunnelTermEthernetOverMplsMacToMeEnableSet function
* @endinternal
*
* @brief   enable/disable MPLS TTI lookup only for mac to me packets
*
* @param[in] enable                   - GT_TRUE:  MPLS TTI lookup only for mac to me packets
*                                      GT_FALSE: disable MPLS TTI lookup only for mac to me packets
*                                       None
*/
GT_VOID prvTgfTunnelTermEthernetOverMplsMacToMeEnableSet
(
    IN GT_BOOL    enable
)
{
    GT_STATUS rc;

    rc = prvTgfTtiPortMplsOnlyMacToMeEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], enable);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfTtiPortMplsOnlyMacToMeEnableSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], enable);
}

/**
* @internal prvTgfTunnelTermEthernetOverMplsMacToMeSet function
* @endinternal
*
* @brief   Set a MAC 2 Me entry
*/
GT_VOID prvTgfTunnelTermEthernetOverMplsMacToMeSet
(
    GT_VOID
)
{
    GT_STATUS                                   rc;
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMeMask;
    TGF_PACKET_L2_STC *l2PartPtr;

    l2PartPtr = prvTgfEthernetOverMplsPacketInfo.partsArray[0].partPtr;

    /* set MAC to ME table */
    cpssOsMemCpy((GT_VOID*)macToMePattern.mac.arEther,
                 (GT_VOID*)&l2PartPtr->daMac,
                 sizeof(macToMePattern.mac.arEther));
    macToMePattern.vlanId = PRV_TGF_VLANID_5_CNS;

    cpssOsMemSet((GT_VOID*)macToMeMask.mac.arEther, 0xFF,
                 sizeof(macToMeMask.mac.arEther));
    macToMeMask.vlanId = 0xFFF;

    /* AUTODOC: config MAC to ME table entry index 1: */
    /* AUTODOC:   DA=00:00:00:00:34:02, VID=5 */
    rc = prvTgfTtiMacToMeSet(PRV_TGF_MAC_TO_ME_INDEX_CNS, &macToMePattern, &macToMeMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacToMeSet");
}

/**
* @internal prvTgfTunnelTermEthernetOverMplsMacToMeDel function
* @endinternal
*
* @brief   Delete a MAC 2 Me entry
*/
GT_VOID prvTgfTunnelTermEthernetOverMplsMacToMeDel
(
    GT_VOID
)
{
    GT_STATUS                 rc;
    PRV_TGF_TTI_MAC_VLAN_STC  macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC  macToMeMask;

    /* full mask for VLAN and MAC */
    macToMeMask.mac.arEther[0] =
    macToMeMask.mac.arEther[1] =
    macToMeMask.mac.arEther[2] =
    macToMeMask.mac.arEther[3] =
    macToMeMask.mac.arEther[4] =
    macToMeMask.mac.arEther[5] = 0xFF;
    macToMeMask.vlanId = 0xFFF;

    /* use 0 MAC DA and VID = 4095 */
    macToMePattern.mac.arEther[0] =
    macToMePattern.mac.arEther[1] =
    macToMePattern.mac.arEther[2] =
    macToMePattern.mac.arEther[3] =
    macToMePattern.mac.arEther[4] =
    macToMePattern.mac.arEther[5] = 0;
    macToMePattern.vlanId = 0xFFF;

    /* AUTODOC: Invalidate MAC to ME table entry index 1 */
    rc = prvTgfTtiMacToMeSet(PRV_TGF_MAC_TO_ME_INDEX_CNS, &macToMePattern, &macToMeMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacToMeSet");
}

/**
* @internal prvTgfTunnelTermEtherOverMplsRedirectToEgressDsaUse function
* @endinternal
*
* @brief   state that the test should use ingress DSA tag (ingress port is cascade port)
*
* @param[in] ingressIsCascadePort
*
* @note save info to be used for :
*       1. configure ingress port as cascade port
*       2. add DSA when send packet into ingress port
*       3. remove the 'cascade' from port when 'restore'
*
*/
GT_VOID prvTgfTunnelTermEtherOverMplsRedirectToEgressDsaUse
(
    IN GT_BOOL     ingressIsCascadePort
)
{
    tgfIngressIsCascadePort = ingressIsCascadePort;
}

/**
* @internal prvTgfTunnelTermEtherOverMplsRedirectToEgressNumOfMplsLabels function
* @endinternal
*
* @brief   state the number of MPLS labels in the tunnel header
*
* @param[in] numOfLabels
*/
GT_VOID prvTgfTunnelTermEtherOverMplsRedirectToEgressNumOfMplsLabels
(
    IN GT_U32     numOfLabels
)
{
    tgfNumberOfLabels = numOfLabels;
}

/**
* @internal prvTgfTunnelTermEtherOverMplsRedirectToEgressUdbKeyTtiConfigSet function
* @endinternal
*
* @brief   Set TTI test settings:
*         - Enable port 0 for Eth lookup
*         - Set TTI rule action
*         - Set TTI rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID prvTgfTunnelTermEtherOverMplsRedirectToEgressUdbKeyTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    PRV_TGF_TTI_ACTION_STC      ruleAction = {0};
    PRV_TGF_TTI_RULE_UNT        ttiPattern;
    PRV_TGF_TTI_RULE_UNT        ttiMask;
    GT_U8                       pclId = 0x1;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* AUTODOC: enable the TTI lookup for PRV_TGF_TTI_KEY_MPLS_E at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_UDB_MPLS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for Mpls TCAM location */

    /* init TTI Pattern */
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    /* init TTI Mask */
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));

    /* configure TTI rule action */
    ruleAction.tunnelTerminate       = GT_TRUE;
    ruleAction.passengerPacketType   = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;

    ruleAction.command         = CPSS_PACKET_CMD_FORWARD_E;
    ruleAction.redirectCommand = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;

    ruleAction.interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    ruleAction.interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    ruleAction.interfaceInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    ruleAction.bridgeBypass    = GT_TRUE;

    ruleAction.keepPreviousQoS = GT_TRUE;
    ruleAction.tunnelStart    = GT_FALSE;
    ruleAction.tunnelStartPtr = 0;

    ruleAction.tag0VlanCmd = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ruleAction.tag1UpCommand  = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;

    /* set TTI Mask & Pattern */
    /* PCL ID */
    ttiPattern.udbArray[0] = pclId;
    ttiMask.udbArray[0] = 0x1f;

    /* Local Device Source is Trunk */
    ttiPattern.udbArray[1] = 0;
    ttiMask.udbArray[1] = 0x20;

    /* Local Device Source ePort/TrunkID */
    ttiPattern.udbArray[2] = (GT_U8)prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    ttiMask.udbArray[2] = 0xff;

    /* eVlan */
    ttiPattern.udbArray[3] = PRV_TGF_VLANID_5_CNS;
    ttiMask.udbArray[3] = 0xff;

    /* Num of MPLS labels */
    ttiPattern.udbArray[4] = ((GT_U8)tgfNumberOfLabels << 4);
    ttiMask.udbArray[4] = 0x70;

    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E , PRV_TGF_TTI_KEY_SIZE_10_B_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    rc = prvTgfTtiRuleSet(prvTgfTtiRuleIndex,PRV_TGF_TTI_RULE_UDB_10_E, &ttiPattern, &ttiMask, &ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");

    rc = prvTgfTtiPclIdSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_MPLS_E, pclId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet: %d", prvTgfDevNum);

    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,0,PRV_TGF_TTI_OFFSET_METADATA_E,22);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,1,PRV_TGF_TTI_OFFSET_METADATA_E,22);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,2,PRV_TGF_TTI_OFFSET_METADATA_E,26);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,3,PRV_TGF_TTI_OFFSET_METADATA_E,24);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_MPLS_E,4,PRV_TGF_TTI_OFFSET_METADATA_E,19);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

}


