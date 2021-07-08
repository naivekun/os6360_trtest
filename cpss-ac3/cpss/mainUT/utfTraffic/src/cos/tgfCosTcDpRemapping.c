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
* @file tgfCosTcDpRemapping.c
*
* @brief Tc and Dp remapping on stack ports.
*
* @version   5
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <utf/private/prvUtfExtras.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfPacketGen.h>
#include <common/tgfCosGen.h>
#include <common/tgfCscdGen.h>
#include <common/tgfPortGen.h>
#include <cos/tgfCosTcDpRemapping.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_VLANID_5_CNS            5
#define PRV_TGF_VLANID_6_CNS            6

/* Traffic Classes used in test */
#define PRV_TGF_COS_TEST_TC0 3

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number to receive traffic from */
#define PRV_TGF_EGR_PORT_IDX_CNS        3

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4
/* the DSA 'src devNum' */
#define SRC_DSA_DEV_NUM(devNum)     (GT_HW_DEV_NUM)((devNum) + 4)

#define DSA_INFO_INDEX  2 /* index in prvTgfPacketPartArray[]*/


static CPSS_QOS_ENTRY_STC  portQoSAttrCfgGet;
/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 1;
static TGF_PACKET_STC prvTgfEtherPacketInfo;
static GT_U32 qosProfileIndex = 7;
static PRV_TGF_COS_PROFILE_STC     profileEntryGet;
static CPSS_CSCD_PORT_TYPE_ENT     portTypeGet;
static CPSS_CSCD_PORT_TYPE_ENT     portTypeGet0;
static GT_BOOL                     tcRemapEnableGet;
static GT_BOOL                     tcRemapEnableGet0;
static GT_U32                      remappedTcGet;
static CPSS_DP_LEVEL_ENT           remappedDpGet;
static GT_U32                      remappedPfcTcGet;
static GT_BOOL                     trustDsaTagModeGet;
static PRV_TGF_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT  portQosDsaTrustModeGet;
static GT_BOOL                     txQueueingTxEnableGet;
static GT_U8                       txQueueRemaped = 6;
static CPSS_PORT_EGRESS_CNT_MODE_ENT   defSetModeBmp;
static GT_U8                           defPortNum;
static GT_U16                          defVlanId;
static GT_U8                           defTc;
static CPSS_DP_LEVEL_ENT               defDpLevel;
static PRV_TGF_PORT_PROFILE_ENT     rxCscdPortProfile;
/* Egress Counters Set number */
static GT_U8                       prvTgfEgressCntrSetNum = 0;
static CPSS_QOS_PORT_TRUST_MODE_ENT trustModeGet;
static PRV_TGF_CSCD_QOS_TC_DP_REMAP_INDEX_STC tcDpRemapping;
static GT_U32   origDevTableBmp[PRV_TGF_FDB_DEV_TABLE_SIZE_CNS] = {0};
static PRV_TGF_DEV_MAP_LOOKUP_MODE_ENT   lookupModeGet;
static CPSS_CSCD_LINK_TYPE_STC      cascadeLinkGet;
static PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT srcPortTrunkHashEnGet;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x92},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                /* srcMac */
};
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_5_CNS                          /* pri, cfi, VlanId */
};
/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x04, 0x07, 0x0a, 0x0d, 0x01, 0x04, 0x07,
    0x02, 0x05, 0x08, 0x0b, 0x0e, 0x02, 0x05, 0x08,
    0x03, 0x06, 0x09, 0x0c, 0x0f, 0x03, 0x06, 0x09,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x43
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};
#define VLAN_TAG_INFO_INDEX  1 /* index in prvTgfPacketPartArray[]*/
#define DSA_INFO_INDEX  2 /* index in prvTgfPacketPartArray[]*/


static TGF_DSA_DSA_FORWARD_STC  prvTgfPacketDsaTagPart_forward = {
    PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E /*GT_TRUE*/ , /*srcIsTagged*/ /* set to GT_TRUE for the copy to CPU , to be with vlan tag 0 */
    8,/*srcHwDev*/
    GT_FALSE,/* srcIsTrunk */
    /*union*/
    {
        /*trunkId*/
        5/*portNum*/
    },/*source;*/

    13,/*srcId*/

    GT_FALSE,/*egrFilterRegistered*/
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
    /* union */
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
    TGF_DSA_4_WORD_TYPE_E ,/*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,/*vpt*/
        0,/*cfiBit*/
        PRV_TGF_VLANID_5_CNS,/*vid*/
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


/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_SKIP_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_DSA_TAG_E    , &prvTgfPacketDsaTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                             /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfCosTcDpRemappingOnStackPortBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID prvTgfCosTcDpRemappingOnStackPortBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       tagArray[] = {0, 0, 0, 0};

    /* AUTODOC: SETUP CONFIGURATION: */

    /* add tags */
    tagArray[0] = (GT_U8)GT_TRUE;
    tagArray[1] = (GT_U8)GT_TRUE;
    tagArray[2] = (GT_U8)GT_TRUE;
    tagArray[3] = (GT_U8)GT_TRUE;

    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with untagged\tagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS, prvTgfPortsArray,
                                           NULL, tagArray, 4);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
}



/**
* @internal prvTgfTcDpRemappingOnStackPortBuildPacket function
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
*       TGF_PACKET_PART_L2_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_IPV4_E
*       TGF_PACKET_PART_PAYLOAD_E
*
*/
GT_VOID prvTgfTcDpRemappingOnStackPortBuildPacket
(
   GT_VOID
)
{
    TGF_PACKET_STC          packetInfo;
    GT_STATUS               rc = GT_OK;

    /* AUTODOC: get default Etherneet packet */
    rc = prvTgfPacketEthernetPacketDefaultPacketGet(&packetInfo.numOfParts, &packetInfo.partsArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv4PacketDefaultPacketGet");
    prvTgfEtherPacketInfo.numOfParts = packetInfo.numOfParts;
    prvTgfEtherPacketInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    prvTgfEtherPacketInfo.partsArray =  packetInfo.partsArray;
}


/**
* @internal prvTgfCosTcDpRemappingOnStackPortQoSConfigSet function
* @endinternal
*
* @brief   Set QoS Configuration
*
* @param[in] testNumber               - number of test
*                                       None
*/
GT_VOID prvTgfCosTcDpRemappingOnStackPortQoSConfigSet
(
    IN GT_U32 testNumber
)
{
    GT_STATUS rc = GT_OK;
    CPSS_QOS_ENTRY_STC      portQoSAttrCfg;
    PRV_TGF_COS_PROFILE_STC profileEntry = {0,0,0,0,0};

    /* save current trust mode */
    rc = prvTgfCosPortQosTrustModeGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &trustModeGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
               "prvTgfCosPortQosTrustModeGet: %d, %d", prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC:  Configure Port to No_Trust */
    rc = prvTgfCosPortQosTrustModeSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      CPSS_QOS_PORT_NO_TRUST_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
              "prvTgfCosPortQosTrustModeSet: %d, %d", prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    if (testNumber == 1)
    {
        /* get QoS port attributes */
        rc = prvTgfCosPortQosConfigGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &portQoSAttrCfgGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                   "prvTgfCosPortQosConfigGet: %d, %d", prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        /* AUTODOC: set QoS port attributes */
        portQoSAttrCfg.qosProfileId     = qosProfileIndex;
        portQoSAttrCfg.assignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
        portQoSAttrCfg.enableModifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        portQoSAttrCfg.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        rc = prvTgfCosPortQosConfigSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &portQoSAttrCfg);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                  "prvTgfCosPortQosConfigSet: %d, %d", prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* get QoS profile 1 entry */
        rc = prvTgfCosProfileEntryGet(prvTgfDevNum,qosProfileIndex,&profileEntryGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntryGet");

        /* AUTODOC: set QoS profile 1 with TC3 */
        profileEntry.dropPrecedence = CPSS_DP_GREEN_E;
        profileEntry.dscp = 0;
        profileEntry.exp = 0;
        profileEntry.trafficClass = 3;
        profileEntry.userPriority = 0;

       rc = prvTgfCosProfileEntrySet(qosProfileIndex, &profileEntry);
       UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet");
    }
    if (testNumber == 2)
    {
        /* get dsa qos trust mode status*/
        rc = prvTgfCosTrustDsaTagQosModeGet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],&trustDsaTagModeGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosTrustDsaTagQosModeGet");
       /* set trust DSA tag QoS mode*/
        rc = prvTgfCosTrustDsaTagQosModeSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosTrustDsaTagQosModeSet");
    }
}



/**
* @internal prvTgfCosTcDpRemappingOnStackPortCscdConfigSet function
* @endinternal
*
* @brief   Set Cascade Configuration
*
* @param[in] testNumber               - test number
*                                       None
*/
GT_VOID prvTgfCosTcDpRemappingOnStackPortCscdConfigSet
(
    IN GT_U32 testNumber
)
{
    GT_STATUS rc;
    rc = prvTgfCscdPortTypeGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS], CPSS_PORT_DIRECTION_TX_E,
                                                        &portTypeGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet");
    /* AUTODOC: set port 54 as cascade one for egress direction */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                                        CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

    if (testNumber == 2)
    {
        rc = prvTgfCscdPortTypeGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], CPSS_PORT_DIRECTION_RX_E,&portTypeGet0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet");
        /* AUTODOC: set port 0 as cascade one for ingress direction */
        rc =  prvTgfCscdPortTypeSet(prvTgfDevNum,CPSS_PORT_DIRECTION_RX_E,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        rc =  prvTgfCscdPortQosDsaModeGet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],&portQosDsaTrustModeGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortQosDsaModeGet");
        /* AUTODOC: set extended DSA qos mode for cascade port*/
        rc =  prvTgfCscdPortQosDsaModeSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],PRV_TGF_CSCD_PORT_QOS_DSA_TRUST_MODE_EXTENDED_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortQosDsaModeSet");

    }
}


/**
* @internal prvTgfCosTcDpRemappingOnStackPortTcDpRemappingConfigSet function
* @endinternal
*
* @brief   Set Tc and DP Remapping Configuration
*
* @param[in] testNumber               - test number
*                                       None
*/
GT_VOID prvTgfCosTcDpRemappingOnStackPortTcDpRemappingConfigSet
(
    IN GT_U32 testNumber
)
{
    GT_STATUS rc = GT_OK;
    tcDpRemapping.dp = CPSS_DP_GREEN_E;
    tcDpRemapping.dsaTagCmd = PRV_TGF_NET_DSA_CMD_FORWARD_E;
    if (testNumber == 1)
    {
        tcDpRemapping.isStack = GT_FALSE;
    }
    if (testNumber == 2)
    {
        tcDpRemapping.isStack = GT_TRUE;
    }
    tcDpRemapping.tc      = 3;

    /* SIP 5_20 fields - use default Target Port profile and MC or UC packets */
    tcDpRemapping.targetPortTcProfile = 0;
    tcDpRemapping.packetIsMultiDestination = (testNumber == 1) ? GT_TRUE : GT_FALSE;

    if(!PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum))
    {
        rc =  prvTgfCscdQosPortTcRemapEnableGet(prvTgfDevNum,
                                                prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                                &tcRemapEnableGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdQosPortTcRemapEnableGet");

        /* AUTODOC: set port 54 as stack port: egress stack port */
        rc =  prvTgfCscdQosPortTcRemapEnableSet(prvTgfDevNum,
                                                prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                                GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdQosPortTcRemapEnableSet");
        if (testNumber == 2)
        {
            rc =  prvTgfCscdQosPortTcRemapEnableGet(prvTgfDevNum,
                                                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                    &tcRemapEnableGet);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdQosPortTcRemapEnableGet");

            /* AUTODOC: set port 0 as stack port: ingress stack port whose trafic is going to be remaped */
            /* on egress stack port 54                                                                   */
            rc =  prvTgfCscdQosPortTcRemapEnableSet(prvTgfDevNum,
                                                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                    GT_TRUE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdQosPortTcRemapEnableSet");

        }
    }
    else
    {
        if (testNumber == 2)
        {
            rc = prvTgfCscdPortTcProfiletGet(prvTgfDevNum,
                                             prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                             CPSS_PORT_DIRECTION_RX_E,&rxCscdPortProfile);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTcProfiletGet");

            /* AUTODOC: set port 0 as stack port: ingress stack port whose trafic is going to be remaped */
            /* on egress stack port 54                                                                   */
            rc = prvTgfCscdPortTcProfiletSet(prvTgfDevNum,
                                             prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                             CPSS_PORT_DIRECTION_RX_E,
                                             PRV_TGF_PORT_PROFILE_CSCD_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTcProfiletSet");

        }
    }

    rc =  prvTgfCscdQosTcDpRemapTableGet(prvTgfDevNum,&tcDpRemapping,&remappedTcGet,&remappedDpGet, &remappedPfcTcGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdQosTcDpRemapTableGet");
    /* AUTODOC: perform mapping of {tc = 3, dp = 0}-> {tc = 6, dp = 1}*/
    rc =  prvTgfCscdQosTcDpRemapTableSet(prvTgfDevNum,&tcDpRemapping,txQueueRemaped, CPSS_DP_YELLOW_E, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdQosTcDpRemapTableGet");
}



/**
* @internal prvTgfCosTcDpRemappingOnStackPortTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:01,
* @param[in] testNumber               - test number
*                                       None
*/
GT_VOID prvTgfCosTcDpRemappingOnStackPortTrafficGenerate
(
    IN GT_U32 testNumber
)
{
    GT_STATUS       rc = GT_OK;
    GT_U32          portsCount  = PRV_TGF_PORT_COUNT_CNS;
    GT_U32          portIter    = 0;
    TGF_PACKET_STC          *packetInfoPtr;
    CPSS_PORT_EGRESS_CNTR_STC       egressCntrs;
    GT_U8          tcForCount;

    CPSS_INTERFACE_INFO_STC portInterface;

    /* Save default Egress Counters set0: Port, TC and DP */
    rc = prvTgfPortEgressCntrModeGet(prvTgfDevNum,
                                     prvTgfEgressCntrSetNum,
                                     &defSetModeBmp,
                                     &defPortNum,
                                     &defVlanId,
                                     &defTc,
                                     &defDpLevel);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrModeGet: %d", prvTgfDevNum);

    /* Before BC3: MIB counts according to incoming <TC> and not according to incoming <queue_priority>.
       The feature was fixed in BC3 and queue number is used for counting. */
    tcForCount = PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum) ? txQueueRemaped : 3;

    /* AUTODOC: GENERATE TRAFFIC: */
    if (testNumber == 1)
    {
        packetInfoPtr = &prvTgfEtherPacketInfo;

        /* Configure Egress Counters set0: Port, TC and DP */
        rc = prvTgfPortEgressCntrModeSet(prvTgfEgressCntrSetNum,
                                         CPSS_EGRESS_CNT_PORT_E | CPSS_EGRESS_CNT_TC_E | CPSS_EGRESS_CNT_DP_E,
                                         prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                         0,
                                         tcForCount,
                                         CPSS_DP_YELLOW_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrModeSet: %d", prvTgfDevNum);

    }
    else
    {
        packetInfoPtr = &prvTgfPacketInfo;

        /* Configure Egress Counters set0: Port, TC and DP */
        rc = prvTgfPortEgressCntrModeSet(prvTgfEgressCntrSetNum,
                                         CPSS_EGRESS_CNT_PORT_E | CPSS_EGRESS_CNT_TC_E | CPSS_EGRESS_CNT_DP_E,
                                         prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS]+1,
                                         0,
                                         tcForCount,
                                         CPSS_DP_YELLOW_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrModeSet: %d", prvTgfDevNum);

    }

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

   /* reset counters */
    rc = prvTgfPortEgressCntrsGet(
        prvTgfEgressCntrSetNum, &egressCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPortEgressCntrsGet\n");

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: send 1  Ethernet  packets from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11, VID=5 */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    packetInfoPtr, prvTgfBurstCount, 0, NULL,
                    prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    /* read counters */
    rc = prvTgfPortEgressCntrsGet(
        prvTgfEgressCntrSetNum, &egressCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPortEgressCntrsGet\n");
    /* AUTODOC:   check egress counters - outUcFrames */
    UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, egressCntrs.outUcFrames, "The number of UC packets is not as expected %d", prvTgfBurstCount);

     rc = prvTgfEthCountersCheck(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                1, 1, 0, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");


    /* disable queue txQueueRemaped - expect no traffic */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* API to disable queueing is not applicable to Falcon */
        CPSS_TBD_BOOKMARK_FALCON
        return;
    }

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    rc =  prvTgfPortTxQueueTxEnableGet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        txQueueRemaped, &txQueueingTxEnableGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxQueueTxEnableGet");

    PRV_UTF_LOG0_MAC("======= disable queue 6 - expect no traffic =======\n");
    if(prvUtfIsGmCompilation())
    {
        rc =  prvTgfPortTxQueueingEnableSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                            txQueueRemaped, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxQueueTxEnableSet");
    }
    else
    {
        rc =  prvTgfPortTxQueueTxEnableSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                            txQueueRemaped, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxQueueTxEnableSet");
    }

    /* check that the egress counter was reset on previous read */
    rc = prvTgfPortEgressCntrsGet(
        prvTgfEgressCntrSetNum, &egressCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPortEgressCntrsGet\n");
    /* AUTODOC:   check egress counters - outUcFrames */
    UTF_VERIFY_EQUAL1_STRING_MAC(0, egressCntrs.outUcFrames, "The number of UC packets is not as expected %d", 0);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "tgfTrafficTableRxPcktTblClear");

    /* setup receive portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* enable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "tgftrafficgeneratorporttxethcaptureset");


    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevNum, packetInfoPtr,
        prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfSetTxSetupEth");

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfStartTransmitingEth");

    cpssOsTimerWkAfter(10);

    if(prvUtfIsGmCompilation())
    {
        /* the packet is blocked and will not be send to queue */
        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                    0, 0, 0, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

        /* enable the queue */
        rc =  prvTgfPortTxQueueingEnableSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                            txQueueRemaped, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxQueueingEnableSet");

        /* for GM need to send packet again */

        /* start transmitting */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfStartTransmitingEth");

        /* read counters */
        rc = prvTgfPortEgressCntrsGet(
            prvTgfEgressCntrSetNum, &egressCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfPortEgressCntrsGet\n");
        /* AUTODOC:   check egress counters - outUcFrames */
        UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, egressCntrs.outUcFrames, "The number of UC packets is not as expected %d", prvTgfBurstCount);

         rc = prvTgfEthCountersCheck(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                1, 1, 0, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

        rc =  prvTgfPortTxQueueingEnableSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                            txQueueRemaped, txQueueingTxEnableGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxQueueTxEnableSet");
    }
    else
    {
        /* the packet is blocked and will not be send to queue */
        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                    0, 0, 0, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

        /* read counters=1, since the counter is counting on queue 3 before the blocking of queue 6 is discovered */
        rc = prvTgfPortEgressCntrsGet(
            prvTgfEgressCntrSetNum, &egressCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfPortEgressCntrsGet\n");
        /* AUTODOC:   check egress counters - outUcFrames */
        UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, egressCntrs.outUcFrames, "The number of UC packets is not as expected %d", prvTgfBurstCount);

        PRV_UTF_LOG0_MAC("======= enable queue 6 - expect traffic =======\n");
        /* enable the queue */
        rc =  prvTgfPortTxQueueTxEnableSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                            txQueueRemaped, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxQueueingEnableSet");

        cpssOsTimerWkAfter(10);

        /* the packet is released and send to queue */

        /* This for simulation purposes : the queue was released .
          wait for processing of all packets after queue was released. */
        (void)tgfTrafficGeneratorCheckProcessignDone(GT_FALSE, 0);

        /* read counters = 0, since the counting was done before the blocking of the queue 6,
           and once we read it in previous call it is reset */
        rc = prvTgfPortEgressCntrsGet(
            prvTgfEgressCntrSetNum, &egressCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfPortEgressCntrsGet\n");
        /* AUTODOC:   check egress counters - outUcFrames */
        UTF_VERIFY_EQUAL1_STRING_MAC(0, egressCntrs.outUcFrames, "The number of UC packets is not as expected %d", 0);

        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                    1, 1, 0, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

        rc =  prvTgfPortTxQueueTxEnableSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                            txQueueRemaped, txQueueingTxEnableGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxQueueTxEnableSet");
    }

    /* disable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgftrafficgeneratorporttxethcaptureset");

    return;
}


/**
* @internal prvTgfCosTcDpRemappingOnStackPortConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*
* @param[in] testNumber               - test number
*                                       None
*/
GT_VOID prvTgfCosTcDpRemappingOnStackPortConfigurationRestore
(
    IN GT_U32 testNumber
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* restore previous state of port 54 */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],portTypeGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");
    if(testNumber == 2)
    {
        /* restore previous state of port 0 */
        rc =  prvTgfCscdPortTypeSet(prvTgfDevNum,CPSS_PORT_DIRECTION_RX_E,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],portTypeGet0);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* restore DSA qos mode for cascade port 0*/
        rc =  prvTgfCscdPortQosDsaModeSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],portQosDsaTrustModeGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortQosDsaModeSet");

       /* restore trust DSA tag QoS mode for port 0*/
        rc = prvTgfCosTrustDsaTagQosModeSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],trustDsaTagModeGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosTrustDsaTagQosModeSet");
    }

    /* restore QoS trust mode */
    rc = prvTgfCosPortQosTrustModeSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],trustModeGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
              "prvTgfCosPortQosTrustModeSet: %d, %d", prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    if(testNumber == 1)
    {
        /* restore QoS port attributes */
        rc = prvTgfCosPortQosConfigSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &portQoSAttrCfgGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                  "prvTgfCosPortQosConfigSet: %d, %d", prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* restore QoS profile  entry */
        rc = prvTgfCosProfileEntrySet(qosProfileIndex, &profileEntryGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet");
    }

    if(!PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum))
    {
        /* restore remap enable status for egress port */
        rc =  prvTgfCscdQosPortTcRemapEnableSet(prvTgfDevNum,
                                                prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                                tcRemapEnableGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdQosPortTcRemapEnableSet");
        if(testNumber == 2)
        {
            rc =  prvTgfCscdQosPortTcRemapEnableSet(prvTgfDevNum,
                                                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                    tcRemapEnableGet0);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdQosPortTcRemapEnableSet");
        }
    }
    else
    {
        if (testNumber == 2)
        {
            /* AUTODOC: Restore port profile of ingress port 0 */
            rc = prvTgfCscdPortTcProfiletSet(prvTgfDevNum,
                                             prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                             CPSS_PORT_DIRECTION_RX_E,
                                             rxCscdPortProfile);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTcProfiletSet");
        }
    }

    /* restore tc and dp */
    rc =  prvTgfCscdQosTcDpRemapTableSet(prvTgfDevNum,&tcDpRemapping,remappedTcGet, remappedDpGet, remappedPfcTcGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdQosTcDpRemapTableGet");
    if(testNumber == 2)
    {
        rc = prvTgfBrgFdbDeviceTableSet(&origDevTableBmp[0]);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        /* restore cascade device map lookup mode */
        rc = prvTgfCscdDevMapLookupModeSet(lookupModeGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* restore the device map table  */
        rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,
                                      SRC_DSA_DEV_NUM(prvTgfDevNum),
                                      prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS] + 1,
                                      &cascadeLinkGet,
                                      srcPortTrunkHashEnGet,
                                      GT_FALSE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    }

    /* restore default Egress Counters set0: Port, TC and DP */
    rc = prvTgfPortEgressCntrModeSet(prvTgfEgressCntrSetNum,
                                     defSetModeBmp,
                                     defPortNum,
                                     defVlanId,
                                     defTc,
                                     defDpLevel);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrModeSet: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     *   Restore Base Configuration
     */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: invalidate VLAN entry 5 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);
}

/**
* @internal prvTgfCosTcDpRemappingOnStackPortDsaTagSet function
* @endinternal
*
* @brief   1. enable running the test in DSA tag mode.
*         3. set DSA tag type
*/
GT_VOID prvTgfCosTcDpRemappingOnStackPortDsaTagSet
(
    GT_VOID
)
{
    GT_HW_DEV_NUM  srcDsaHwDevNum     = SRC_DSA_DEV_NUM(prvTgfDevNum);
    GT_U32  devTableBmp[PRV_TGF_FDB_DEV_TABLE_SIZE_CNS];
    CPSS_CSCD_LINK_TYPE_STC      cascadeLink;
    GT_STATUS   rc;
    prvTgfPacketDsaTagPart.dsaType = TGF_DSA_4_WORD_TYPE_E;
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;

    /* set DSA tag info related values */

    /* bind the DSA tag info */
    prvTgfPacketPartArray[DSA_INFO_INDEX].type    = TGF_PACKET_PART_DSA_TAG_E;
    prvTgfPacketPartArray[DSA_INFO_INDEX].partPtr = &prvTgfPacketDsaTagPart;
    prvTgfPacketPartArray[VLAN_TAG_INFO_INDEX].type    = TGF_PACKET_PART_SKIP_E;

    prvTgfPacketDsaTagPart_forward.srcHwDev = srcDsaHwDevNum;
    /* set the target port of the packet in the DSA tag info */
    prvTgfPacketDsaTagPart_forward.dstInterface.type = CPSS_INTERFACE_PORT_E;
    prvTgfPacketDsaTagPart_forward.dstInterface.devPort.hwDevNum = srcDsaHwDevNum;
    prvTgfPacketDsaTagPart_forward.dstInterface.devPort.portNum = prvTgfPortsArray[3]+1;

    /* indication that the source device found the FDB entry in it's FDB */
    prvTgfPacketDsaTagPart_forward.egrFilterRegistered = GT_TRUE;
    prvTgfPacketDsaTagPart_forward.isTrgPhyPortValid = GT_FALSE;
    prvTgfPacketDsaTagPart_forward.dstEport = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS]+1;
    prvTgfPacketDsaTagPart_forward.origSrcPhyIsTrunk = GT_FALSE;
    prvTgfPacketDsaTagPart_forward.origSrcPhy.portNum = CPSS_NULL_PORT_NUM_CNS;/*dummy port*/
    prvTgfPacketDsaTagPart_forward.source.portNum = 1;
    prvTgfPacketDsaTagPart_forward.qosProfileIndex = 0x0c;

    /* bind the DSA tag FORWARD part (since union is used .. can't do it in compilation time) */
    prvTgfPacketDsaTagPart.dsaInfo.forward = prvTgfPacketDsaTagPart_forward;

    /* allow traffic to be in the FDB that is associated with the 'remote device' */
    rc = prvTgfBrgFdbDeviceTableGet(&origDevTableBmp[0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    devTableBmp[0] = origDevTableBmp[0] | (1<<srcDsaHwDevNum);
    rc = prvTgfBrgFdbDeviceTableSet(&devTableBmp[0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    rc =  prvTgfCscdDevMapLookupModeGet(prvTgfDevNum,&lookupModeGet);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    /*Set lookup mode for accessing the Device Map table.*/
    rc = prvTgfCscdDevMapLookupModeSet(PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* set the device map table */
    rc =  prvTgfCscdDevMapTableGet( prvTgfDevNum,
                                    srcDsaHwDevNum,
                                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS] + 1,
                                    &cascadeLinkGet,
                                    &srcPortTrunkHashEnGet);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* set the device map table to know where to send traffic destined to SRC_DSA_DEV_NUM(prvTgfDevNum) */
    cascadeLink.linkNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,
                                  srcDsaHwDevNum,
                                  prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS] + 1,
                                  &cascadeLink,
                                  PRV_TGF_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E,
                                  GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

}


