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
* @file prvTgfTunnelMacInMacKey.c
*
* @brief Verify the functionality of MIM TTI key MAC DA or MAC SA
*
* @version   33
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfInlifGen.h>
#include <tunnel/prvTgfTunnelMacInMacKey.h>
#if (defined EXMXPM_FAMILY)
    #include <cpss/exMxPm/exMxPmGen/inlif/cpssExMxPmInlif.h>
    #include <cpss/exMxPm/exMxPmGen/config/private/prvCpssExMxPmInfo.h>
#endif


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* port to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number  */
#define PRV_TGF_EGR_PORT_IDX_CNS        3

/* egress port number for flooding in VLAN */
#define PRV_TGF_FLOOD_PORT_IDX_CNS        1

/* default VLAN Id 5 */
#define PRV_TGF_VLANID_5_CNS        5

/* default VLAN Id 6 */
#define PRV_TGF_VLANID_6_CNS        6

/* MAC2ME entry index */
#define PRV_TGF_MAC_TO_ME_INDEX_CNS     1

/* default number of packets to send */
static GT_U32   prvTgfBurstCount = 1;

/* additional L2 part */
static TGF_PACKET_L2_STC prvTgfPacketAdditionalL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                 /* saMac */
};

/*************************** Backbone packet's part ***************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketBackboneL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x05},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketBackboneVlanTagPart =
{
    TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS,                 /* etherType */
    0, 0, PRV_TGF_VLANID_5_CNS                          /* pri, cfi, VlanId */
};

/********************* Service incapsulation part *****************************/

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketServiceEtherTypePart = {TGF_ETHERTYPE_88E7_MIM_TAG_CNS};

/* Data of packet */
static GT_U8 prvTgfPacketServicePayloadDataArr[] =
{
    0xa0, 0x12, 0x34, 0x56
};

/* Service Payload part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketServicePayloadPart =
{
    sizeof(prvTgfPacketServicePayloadDataArr), /* dataLength */
    prvTgfPacketServicePayloadDataArr          /* dataPtr */
};

/***************************** Original part **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_5_CNS                          /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x32,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    {10, 10, 10, 10},   /* srcAddr */
    { 4,  4,  4,  4}    /* dstAddr */
};

/* Data of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d
};

/* Payload part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPacketPayloadDataArr), /* dataLength */
    prvTgfPacketPayloadDataArr          /* dataPtr */
};

/************************* Packet parts ***************************************/

/* PARTS of the packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketBackboneL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketBackboneVlanTagPart},

    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketServiceEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketServicePayloadPart},

    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of packet */
#define PRV_TGF_PACKET_LEN_CNS                                                 \
    2 * TGF_L2_HEADER_SIZE_CNS + 2 * TGF_VLAN_TAG_SIZE_CNS +                   \
    2 * TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS +                    \
    sizeof(prvTgfPacketPayloadDataArr) + sizeof(prvTgfPacketServicePayloadDataArr)

/* Length of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* Packet to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                           /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]), /* numOfParts */
    prvTgfPacketPartArray                                             /* partsArray */
};
/******************************************************************************/
#define FILLED_IN_RUNTIME   0
static  GT_U16  passengerVidArr[2] = {0x123,0x456};
#define TGF_ETHERTYPE_9999_VLAN_TAG_CNS     0x9999
#define TGF_ETHERTYPE_AAAA_VLAN_TAG_CNS     0xAAAA
#define TGF_ETHERTYPE_6666_VLAN_TAG_CNS     0x6666

/* tunnel term entry index */
#define PRV_TGF_MIM_TTI_INDEX_CNS           6
/* PCLID for the rule - for the ethernet packets */
#define PRV_TGF_TTI_MIM_PCL_ID_CNS           0x12C

/* passenger outer VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanOuterTagPart =
{
    TGF_ETHERTYPE_9999_VLAN_TAG_CNS,     /* etherType */
    0, 0, FILLED_IN_RUNTIME    /* pri, cfi, VlanId - filled in runtime */
};

/* passenger inner VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanInnerTagPart =
{
    TGF_ETHERTYPE_AAAA_VLAN_TAG_CNS,    /* etherType */
    0, 0, FILLED_IN_RUNTIME    /* pri, cfi, VlanId - filled in runtime */
};


/* passenger expected VLAN_TAG parts */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagExpectedPart[2];


/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketDummyEtherTypePart = {TGF_ETHERTYPE_6666_VLAN_TAG_CNS};

/* Data of packet (long version) */
static GT_U8 prvTgfPacketPayloadLongDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

/* Payload part (long version) */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketLongPayloadPart =
{
    sizeof(prvTgfPacketPayloadLongDataArr), /* dataLength */
    prvTgfPacketPayloadLongDataArr          /* dataPtr */
};


/* Mim - PARTS of the packet */
static TGF_PACKET_PART_STC prvTgfMimPacketPartArray[] =
{
    /*Backbone component*/
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketBackboneL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketBackboneVlanTagPart},

    /*Service encapsulation*/
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketServiceEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketServicePayloadPart},

        /* passenger --- Original customer frame*/
        {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
        {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanOuterTagPart},
        {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanInnerTagPart},
        {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketDummyEtherTypePart},
        {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* Packet to send */
static TGF_PACKET_STC prvTgfMimPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                                       /* totalLen */
    sizeof(prvTgfMimPacketPartArray) / sizeof(prvTgfMimPacketPartArray[0]), /* numOfParts */
    prvTgfMimPacketPartArray                                             /* partsArray */
};

/* Mim - PARTS of the packet */
static TGF_PACKET_PART_STC prvTgfEgressPassengerPacketPartArray[] =
{
    /*Backbone component*/
    {TGF_PACKET_PART_SKIP_E, NULL},
    {TGF_PACKET_PART_SKIP_E, NULL},

    /*Service encapsulation*/
    {TGF_PACKET_PART_SKIP_E, NULL},
    {TGF_PACKET_PART_SKIP_E, NULL},

        /* passenger --- Original customer frame*/
        {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
        {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanOuterTagPart},
        {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanInnerTagPart},
        {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketDummyEtherTypePart},
        {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* Packet to send */
static TGF_PACKET_STC prvTgfEgressPassengerPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                                       /* totalLen */
    sizeof(prvTgfEgressPassengerPacketPartArray) / sizeof(prvTgfEgressPassengerPacketPartArray[0]), /* numOfParts */
    prvTgfEgressPassengerPacketPartArray                                             /* partsArray */
};


/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* stored default VID */
static GT_U16   prvTgfDefVlanId = 0;

/* stored default MIM B-EtherType in TPID table */
static GT_U16   prvTgfDefIngressEtherType0 = 0;
static GT_U16   prvTgfDefIngressMaxVidRangePtr0 = 0;
static GT_U16   prvTgfDefEgressEtherType0 = 0;
static GT_U16   prvTgfDefEgressEtherType1 = 0;
static GT_U32   prvTgfDefiTagEthTypePtr = 0;
static GT_U32   profile0Get = 0;
static GT_U32   profile1Get = 0;
/* stored default inlifMode */
static PRV_TGF_INLIF_PORT_MODE_ENT     inlifMode;

/* expected number of Triggers on ports with index 0 */
static GT_U8 prvTgfPacketsCountTriggerArr[] =
{
    1, 1
};

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfTunnelTermTestInit function
* @endinternal
*
* @brief   Set initial test settings:
*         - Create VLAN 5 with ports 0, 8
*         - Create VLAN 6 with ports 18, 23
*         - Set VLAN 5 as port 0 default VLAN ID (PVID)
*         - Enable port 0 for MIM TTI lookup
*         - Set MIM TTI key lookup MAC mode to Mac DA
*         - Set TTI rule action
*         - Set TTI rule with un-masked parameters
* @param[in] ruleIndex                - index of the tunnel termination entry
* @param[in] macMode                  - MAC mode to use
* @param[in] isMacMasked              - enable\disable MAC mask
*                                       None
*/
static GT_VOID prvTgfTunnelTermTestInit
(
    IN GT_U32                         ruleIndex,
    IN PRV_TGF_TTI_MAC_MODE_ENT       macMode,
    IN GT_BOOL                        isMacMasked
)
{
    GT_STATUS                   rc       = GT_OK;
    PRV_TGF_TTI_RULE_UNT        pattern;
    PRV_TGF_TTI_RULE_UNT        mask;
    PRV_TGF_TTI_ACTION_STC      ruleAction = {0};
    GT_U8                       tagArray[] = {1, 1, 1, 1};
    GT_U16                      etherType;
    GT_U16                      etherType1 = TGF_ETHERTYPE_8100_VLAN_TAG_CNS;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* AUTODOC: create VLAN 5 with tagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS, prvTgfPortsArray,
                                           NULL, tagArray, prvTgfPortsNum / 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with tagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_6_CNS, prvTgfPortsArray + 2,
                                           NULL, tagArray + 2, prvTgfPortsNum / 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* get default port vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: set PVID 5 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* set Port VID Precedence */
    /* AUTODOC: set SOFT VID Precedence for port 0 */
    rc = prvTgfBrgVlanPortVidPrecedenceSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidPrecedenceSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E);


    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 6, port 3 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacketL2Part.daMac, PRV_TGF_VLANID_6_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* AUTODOC: set MIM EtherType to 0x88E7 */
    rc = prvTgfTtiMimEthTypeSet(TGF_ETHERTYPE_88E7_MIM_TAG_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMimEthTypeSet: %x", TGF_ETHERTYPE_88E7_MIM_TAG_CNS);

#ifdef CHX_FAMILY

    etherType = TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS;

#else /* EXMXPM_FAMILY */

    if(GT_TRUE == PRV_CPSS_EXMXPM_ERRATA_GET_MAC(prvTgfDevNum,
                                            PRV_CPSS_EXMXPM_FER_TT_PASSENGER_SOURCE_TAG_SETTING))
    {
        etherType = TGF_ETHERTYPE_8100_VLAN_TAG_CNS;
    }
    else
    {
        etherType = TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS;
    }
#endif

    /* get  Ingress VLAN Ether Type and Range */
    rc = prvTgfBrgVlanIngressEtherTypeGet(CPSS_VLAN_ETHERTYPE0_E,&prvTgfDefIngressEtherType0,&prvTgfDefIngressMaxVidRangePtr0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIngressEtherTypeGet: %d, %x",
                                 prvTgfDevNum,etherType);
    /* AUTODOC:  Sets Ingress VLAN Ether Type and Range : EtherType 0x88A8 / or 0x8100 */
    rc = prvTgfBrgVlanIngressEtherTypeSet(CPSS_VLAN_ETHERTYPE0_E,etherType,4095);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIngressEtherTypeSet: %d, %x",
                                 prvTgfDevNum,etherType);
    /* Get  Egress VLAN Ether Type 0 */
    rc = prvTgfBrgVlanEgressEtherTypeGet(prvTgfDevNum,CPSS_VLAN_ETHERTYPE0_E,&prvTgfDefEgressEtherType0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEgressEtherTypeGet: %d, %x",
                                 prvTgfDevNum,etherType);


    rc = prvTgfBrgVlanEgressEtherTypeSet(CPSS_VLAN_ETHERTYPE0_E,etherType1);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEgressEtherTypeSet: %d, %x",
                                 prvTgfDevNum,etherType1);


    /* Get  Egress VLAN Ether Type 1 */
    rc = prvTgfBrgVlanEgressEtherTypeGet(prvTgfDevNum,CPSS_VLAN_ETHERTYPE1_E,&prvTgfDefEgressEtherType1);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEgressEtherTypeGet: %d, %x",
                                 prvTgfDevNum,etherType1);

    rc = prvTgfBrgVlanEgressEtherTypeSet(CPSS_VLAN_ETHERTYPE1_E,etherType1);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEgressEtherTypeSet: %d, %x",
                                 prvTgfDevNum,etherType1);
    /* Get globally the MAC in MAC I-Tag Ethertype (802.1ah). */
    rc = prvTgfTunnelStartMimITagEtherTypeGet(prvTgfDevNum, &prvTgfDefiTagEthTypePtr);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartMimITagEtherTypeGet: %d, %x",
                                 prvTgfDevNum,TGF_ETHERTYPE_88E7_MIM_TAG_CNS);

    /* Sets globally the MAC in MAC I-Tag Ethertype (802.1ah). */
    rc = prvTgfTunnelStartMimITagEtherTypeSet(TGF_ETHERTYPE_88E7_MIM_TAG_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartMimITagEtherTypeSet: %d, %x",
                                 prvTgfDevNum,TGF_ETHERTYPE_88E7_MIM_TAG_CNS);

    /* AUTODOC: enable TTI lookup for port 0, key TTI_KEY_MIM */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_MIM_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* set MIM TTI key lookup MAC mode */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_MIM_E, macMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &ruleAction, 0, sizeof(ruleAction));

    /* configure TTI rule action */
    ruleAction.tunnelTerminate     = GT_TRUE;
    ruleAction.passengerPacketType = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
    ruleAction.command             = CPSS_PACKET_CMD_FORWARD_E;
    ruleAction.redirectCommand     = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ruleAction.tag0VlanCmd         = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ruleAction.tag0VlanId          = PRV_TGF_VLANID_6_CNS;
    ruleAction.tag1VlanCmd         = PRV_TGF_TTI_VLAN1_MODIFY_ALL_E;
    ruleAction.tunnelStart         = GT_FALSE;

    ruleAction.vlanCmd = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ruleAction.vlanId  = PRV_TGF_VLANID_6_CNS;
    ruleAction.vlanPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
    ruleAction.nestedVlanEnable = GT_FALSE;

    ruleAction.interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    ruleAction.interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    ruleAction.interfaceInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* clear entries */
    cpssOsMemSet((GT_VOID*) &pattern, 0, sizeof(pattern));
    cpssOsMemSet((GT_VOID*) &mask, 0, sizeof(mask));

    /* configure TTI rule */
    switch (macMode)
    {
        case PRV_TGF_TTI_MAC_MODE_DA_E:
            cpssOsMemCpy((GT_VOID*)pattern.mim.common.mac.arEther,
                         (GT_VOID*)prvTgfPacketBackboneL2Part.daMac,
                         sizeof(pattern.mim.common.mac.arEther));
            break;

        case PRV_TGF_TTI_MAC_MODE_SA_E:
            cpssOsMemCpy((GT_VOID*)pattern.mim.common.mac.arEther,
                         (GT_VOID*)prvTgfPacketBackboneL2Part.saMac,
                         sizeof(pattern.mim.common.mac.arEther));
            break;

        default:
            rc = GT_BAD_PARAM;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Get invalid macMode: %d", macMode);
    }

    pattern.mim.common.vid   = PRV_TGF_VLANID_5_CNS;
    pattern.mim.common.pclId = 4;
    pattern.mim.iSid         = 0x123456;
    pattern.mim.iUp          = 0x5;
    pattern.mim.iDp          = 0;

    /* configure TTI rule mask */
    mask.mim.common.pclId = 0x3FF;
    mask.mim.common.vid = 0xFFF;
    cpssOsMemSet((GT_VOID*) &(mask.mim.iSid), 0xFF,
                 sizeof(mask.mim.iSid));
    cpssOsMemSet((GT_VOID*) &(mask.mim.iUp), 0xFF,
                 sizeof(mask.mim.iUp));
    cpssOsMemSet((GT_VOID*) &(mask.mim.iDp), 0xFF,
                 sizeof(mask.mim.iDp));

    if (isMacMasked)
    {
        cpssOsMemSet((GT_VOID*) &(mask.mim.common.mac), 0xFF, sizeof(mask.mim.common.mac));
    }

    /* set TTI rule */
    rc = prvTgfTtiRuleSet(ruleIndex, PRV_TGF_TTI_KEY_MIM_E, &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelTermTestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] portNum                  - port number
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfTunnelTermTestPacketSend
(
    IN  GT_U32                         portNum,
    IN  TGF_PACKET_STC               *packetInfoPtr
)
{
    GT_STATUS               rc            = GT_OK;
    GT_U32                  portsCount    = prvTgfPortsNum;
    GT_U32                  portIter      = 0;
    CPSS_INTERFACE_INFO_STC portInterface = {0};

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* -------------------------------------------------------------------------
     * 1. Sending unknown unicast
     */
    PRV_UTF_LOG0_MAC("======= Sending unknown unicast =======\n");

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* configure portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* enable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d", prvTgfDevNum);

    /* send Packet from port 0 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d", prvTgfDevNum);

    /* disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d", prvTgfDevNum);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfTunnelTermTestReset function
* @endinternal
*
* @brief   Function clears FDB, VLAN tables and internal table of captured packets.
*/
static GT_VOID prvTgfTunnelTermTestReset
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

     /* -------------------------------------------------------------------------
    * 1. Restore Route Configuration
    */

    /* AUTODOC: restore default inlif port mode */
    rc = prvTgfInlifPortModeSet(PRV_TGF_SEND_PORT_IDX_CNS,inlifMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfInlifPortModeSet");


    PRV_UTF_LOG0_MAC("======= Restore Configuration =======\n");

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    if(GT_TRUE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E))
    {
        /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_MIM */
        rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_MIM_E, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);
    }

    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(0, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d", prvTgfDevNum, GT_FALSE);

   /* AUTODOC: restore Ingress VLAN Ether Type and Range  */
    rc = prvTgfBrgVlanIngressEtherTypeSet(CPSS_VLAN_ETHERTYPE0_E,prvTgfDefIngressEtherType0,prvTgfDefIngressMaxVidRangePtr0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIngressEtherTypeSet: %d, %x",
                                 prvTgfDevNum,prvTgfDefIngressEtherType0);

    /* AUTODOC: restore Egress  VLAN Ether Type0 and Range  */
    rc = prvTgfBrgVlanEgressEtherTypeSet(CPSS_VLAN_ETHERTYPE0_E,prvTgfDefEgressEtherType0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEgressEtherTypeSet: %d, %x",
                                 prvTgfDevNum,prvTgfDefEgressEtherType0);

    /* AUTODOC: restore Egress  VLAN Ether Type1 and Range  */
    rc = prvTgfBrgVlanEgressEtherTypeSet(CPSS_VLAN_ETHERTYPE1_E,prvTgfDefEgressEtherType1);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEgressEtherTypeSet: %d, %x",
                                 prvTgfDevNum,prvTgfDefEgressEtherType1);

    /* AUTODOC: restore globally MAC in MAC I-Tag Ethertype (802.1ah). */
    rc = prvTgfTunnelStartMimITagEtherTypeSet(prvTgfDefiTagEthTypePtr);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartMimITagEtherTypeSet: %d, %x",
                                 prvTgfDevNum,prvTgfDefiTagEthTypePtr);

   if(GT_TRUE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E))
    {
        /* AUTODOC: restore default MIM EtherType */
        /* set MIM EtherType */
        rc = prvTgfTtiMimEthTypeSet(TGF_ETHERTYPE_88E7_MIM_TAG_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMimEthTypeSet");
    }

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore default vlanId */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d", prvTgfDevNum, prvTgfDefVlanId);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_5_CNS);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_6_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_6_CNS);

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
}


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelTermUseMacSaInMimTtiLookupConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set index of the rule in the TCAM to 0
*         - set macMode PRV_TGF_TTI_MAC_MODE_SA_E
*         - set MAC mask as disable
*/
GT_VOID prvTgfTunnelTermUseMacSaInMimTtiLookupConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* init configuration */
    prvTgfTunnelTermTestInit(0, PRV_TGF_TTI_MAC_MODE_SA_E, GT_FALSE);
    /* AUTODOC: set MAC_MODE_SA for TTI_KEY_MIM */
    /* AUTODOC: add TTI rule 0 with: */
    /* AUTODOC:   key MIM, cmd FORWARD, redirectCmd REDIRECT_TO_EGRESS */
    /* AUTODOC:   pattern MIM pclId=4, vid=5, iSid=0x123456, iUp=0x5, iDp=0 */
    /* AUTODOC:   pattern MIM MAC=00:00:00:00:00:33 */
    /* AUTODOC:   action Tunnel Terminate, PASSENGER_ETHERNET_NO_CRC, port=3 */
}

/**
* @internal prvTgfTunnelTermUseMacSaInMimTtiLookupTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 MIM tunneled packet:
*         macDa = 00:00:00:00:00:22,
*         macSa = 00:00:00:00:34:02,
*         Tagged: true (with VLAN tag 5)
*         Ether Type: 0x8988 (MIM Ether type)
*         iSid: 0x123456
*         iUP: 0x5
*         iDP: 0
*         Success Criteria:
*         Packet striped and forwarded to port 23 VLAN 6
*/
GT_VOID prvTgfTunnelTermUseMacSaInMimTtiLookupTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc           = GT_OK;
    GT_U32                          portIter     = 0;
    GT_BOOL                         isEqualCntrs = GT_FALSE;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[2]={{0}};
    GT_U32                          tmpTrigger      = 0;
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    GT_U32                          packetSize;


    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

#ifdef EXMXPM_FAMILY
    if(GT_TRUE == PRV_CPSS_EXMXPM_ERRATA_GET_MAC(prvTgfDevNum,
                                            PRV_CPSS_EXMXPM_FER_TT_PASSENGER_SOURCE_TAG_SETTING))
    {
        /* override etherType in the prvTgfPacketBackboneVlanTagPart  */
        prvTgfPacketPartArray[1].partPtr = (GT_VOID*) &prvTgfPacketVlanTagPart;
    }
#endif

    /* send packet to be matched */
    /* AUTODOC: send MIM tunneled packet from port 0 with: */
    /* AUTODOC:   BB DA=00:00:00:00:34:05, SA=00:00:00:00:00:33 */
    /* AUTODOC:   BB EtherType=0x88A8, VID=5 */
    /* AUTODOC:   MIM EtherType=0x88E7 */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11, VID=5 */
    /* AUTODOC:   srcIP=10.10.10.10, dstIP=4.4.4.4 */
    prvTgfTunnelTermTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    PRV_UTF_LOG0_MAC("======= Checking Counters =======\n");

    /* AUTODOC: verify to get IPv4 packet on port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11 */
    /* AUTODOC:   srcIP=10.10.10.10, dstIP=4.4.4.4 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* clear expected counters */
        cpssOsMemSet(&expectedCntrs, 0, sizeof(expectedCntrs));

        /* calculate expected counters */
        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* original packet before tunnel terminate in VLAN 5 */
            packetSize = PRV_TGF_PACKET_CRC_LEN_CNS;

            expectedCntrs.goodOctetsSent.l[0] = packetSize * prvTgfBurstCount;
            expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
            expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
            expectedCntrs.goodOctetsRcv.l[0]  = packetSize * prvTgfBurstCount;
            expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
            expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
        }

        /* calculate expected counters */
        if (PRV_TGF_EGR_PORT_IDX_CNS == portIter)
        {
            /* packet after tunnel terminate in VLAN 6 */
            packetSize = PRV_TGF_PACKET_CRC_LEN_CNS
                - TGF_L2_HEADER_SIZE_CNS
                - TGF_VLAN_TAG_SIZE_CNS
                - TGF_ETHERTYPE_SIZE_CNS - sizeof(prvTgfPacketServicePayloadDataArr);

            expectedCntrs.goodOctetsSent.l[0] = packetSize * prvTgfBurstCount;
            expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
            expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
            expectedCntrs.goodOctetsRcv.l[0] = packetSize * prvTgfBurstCount;
            expectedCntrs.goodPktsRcv.l[0]   = prvTgfBurstCount;
            expectedCntrs.ucPktsRcv.l[0]     = prvTgfBurstCount;
        }

        /* check Rx\Tx counters */
        PRV_TGF_VERIFY_COUNTERS_MAC(isEqualCntrs, expectedCntrs, portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqualCntrs, GT_TRUE, "get another counters values.");

        /* print expected values if not equal */
        PRV_TGF_PRINT_DIFF_COUNTERS_MAC(isEqualCntrs, expectedCntrs, portCntrs);
    }

    /* set interface info */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* clear triggers */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

     /* get trigger counters where packet has MAC SA */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = sizeof(TGF_MAC_ADDR);
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));

    /* get trigger counters where packet has vlan tag 6 */
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset = 12;
    vfdArray[1].cycleCount = 4;
    vfdArray[1].patternPtr[0] = 0x81;
    vfdArray[1].patternPtr[1] = 0x00;
    vfdArray[1].patternPtr[2] = 0x00;
    vfdArray[1].patternPtr[3] = 0x06;

    /* get trigger counters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 2, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check first pattern of TriggerCounters */
    tmpTrigger = ((numTriggers & 1) != 0) ? 1 : 0;
    UTF_VERIFY_EQUAL6_STRING_MAC(prvTgfPacketsCountTriggerArr[0], tmpTrigger,
                         "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                                 prvTgfPacketL2Part.saMac[0], prvTgfPacketL2Part.saMac[1], prvTgfPacketL2Part.saMac[2],
                                 prvTgfPacketL2Part.saMac[3], prvTgfPacketL2Part.saMac[4], prvTgfPacketL2Part.saMac[5]);


    /* check second pattern of TriggerCounters */
    tmpTrigger = ((numTriggers & (1 << 1)) != 0) ? 1 : 0;
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketsCountTriggerArr[1], tmpTrigger,
                          "VLAN tag of captured packet must be: 81 00 00 06\n");


    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);
}

/**
* @internal prvTgfTunnelTermUseMacSaInMimTtiLookupConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelTermUseMacSaInMimTtiLookupConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfTunnelTermTestReset();
}

/**
* @internal prvTgfTunnelTermMaskMacInMimTtiLookupConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set index of the rule in the TCAM to 0
*         - set macMode PRV_TGF_TTI_MAC_MODE_DA_E
*         - set MAC mask as enable
*/
GT_VOID prvTgfTunnelTermMaskMacInMimTtiLookupConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* init configuration */
    prvTgfTunnelTermTestInit(0, PRV_TGF_TTI_MAC_MODE_DA_E, GT_TRUE);
    /* AUTODOC: set MAC_MODE_DA for TTI_KEY_MIM */
    /* AUTODOC: add TTI rule 0 with: */
    /* AUTODOC:   key MIM, cmd FORWARD, redirectCmd REDIRECT_TO_EGRESS */
    /* AUTODOC:   pattern MIM pclId=4, vid=5, iSid=0x123456, iUp=0x5, iDp=0 */
    /* AUTODOC:   pattern MIM MAC=00:00:00:00:34:05 */
    /* AUTODOC:   action Tunnel Terminate, PASSENGER_ETHERNET_NO_CRC, port=3 */
}

/**
* @internal prvTgfTunnelTermMaskMacInMimTtiLookupTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 MIM tunneled packet:
*         macDa = 00:00:00:00:00:22,
*         macSa = 00:00:00:00:34:05,
*         Tagged: true (with VLAN tag 5)
*         Ether Type: 0x8988 (MIM Ether type)
*         iSid: 0x123456
*         iUP: 0x5
*         iDP: 0
*         Success Criteria:
*         Packet striped and forwarded to port 23 VLAN 6
*/
GT_VOID prvTgfTunnelTermMaskMacInMimTtiLookupTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc           = GT_OK;
    GT_U32                          portIter     = 0;
    GT_BOOL                         isEqualCntrs = GT_FALSE;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[2]={{0}};
    GT_U32                          tmpTrigger      = 0;
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    GT_U32                          packetSize;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

#ifdef EXMXPM_FAMILY
    if(GT_TRUE == PRV_CPSS_EXMXPM_ERRATA_GET_MAC(prvTgfDevNum,
                                            PRV_CPSS_EXMXPM_FER_TT_PASSENGER_SOURCE_TAG_SETTING))
    {
        /* override etherType in the prvTgfPacketBackboneVlanTagPart  */
        prvTgfPacketPartArray[1].partPtr = (GT_VOID*) &prvTgfPacketVlanTagPart;
    }
#endif

    /* override default DA */
    prvTgfPacketPartArray[4].partPtr = (GT_VOID*) &prvTgfPacketAdditionalL2Part;

    /* send packet to be matched */
    /* AUTODOC: send MIM tunneled packet from port 0 with: */
    /* AUTODOC:   BB DA=00:00:00:00:34:05, SA=00:00:00:00:00:33 */
    /* AUTODOC:   BB EtherType=0x88A8, VID=5 */
    /* AUTODOC:   MIM EtherType=0x88E7 */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=5 */
    /* AUTODOC:   srcIP=10.10.10.10, dstIP=4.4.4.4 */
    prvTgfTunnelTermTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    PRV_UTF_LOG0_MAC("======= Checking Counters =======\n");

    /* AUTODOC: verify to get IPv4 packet on port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:22 */
    /* AUTODOC:   srcIP=10.10.10.10, dstIP=4.4.4.4 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* clear expected counters */
        cpssOsMemSet(&expectedCntrs, 0, sizeof(expectedCntrs));

        /* calculate expected counters */
        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* original packet before tunnel terminate in VLAN 5 */
            packetSize = PRV_TGF_PACKET_CRC_LEN_CNS;

            expectedCntrs.goodOctetsSent.l[0] = packetSize * prvTgfBurstCount;
            expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
            expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
            expectedCntrs.goodOctetsRcv.l[0]  = packetSize * prvTgfBurstCount;
            expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
            expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
        }

        /* calculate expected counters */
        if (PRV_TGF_EGR_PORT_IDX_CNS == portIter)
        {
            /* packet after tunnel terminate in VLAN 6 */
            packetSize = PRV_TGF_PACKET_CRC_LEN_CNS
                - TGF_L2_HEADER_SIZE_CNS
                - TGF_VLAN_TAG_SIZE_CNS
                - TGF_ETHERTYPE_SIZE_CNS - sizeof(prvTgfPacketServicePayloadDataArr);

            expectedCntrs.goodOctetsSent.l[0] = packetSize * prvTgfBurstCount;
            expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
            expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
            expectedCntrs.goodOctetsRcv.l[0] = packetSize * prvTgfBurstCount;
            expectedCntrs.goodPktsRcv.l[0]   = prvTgfBurstCount;
            expectedCntrs.ucPktsRcv.l[0]     = prvTgfBurstCount;
        }

        /* check Rx\Tx counters */
        PRV_TGF_VERIFY_COUNTERS_MAC(isEqualCntrs, expectedCntrs, portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqualCntrs, GT_TRUE, "get another counters values.");

        /* print expected values if not equal */
        PRV_TGF_PRINT_DIFF_COUNTERS_MAC(isEqualCntrs, expectedCntrs, portCntrs);
    }

    /* set interface info */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* clear triggers */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* get trigger counters where packet has MAC SA */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = sizeof(TGF_MAC_ADDR);
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketAdditionalL2Part.saMac, sizeof(TGF_MAC_ADDR));

    /* get trigger counters where packet has vlan tag 6 */
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset = 12;
    vfdArray[1].cycleCount = 4;
    vfdArray[1].patternPtr[0] = 0x81;
    vfdArray[1].patternPtr[1] = 0x00;
    vfdArray[1].patternPtr[2] = 0x00;
    vfdArray[1].patternPtr[3] = 0x06;

    /* get trigger counters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 2, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check first pattern of TriggerCounters */
    tmpTrigger = ((numTriggers & 1) != 0) ? 1 : 0;
    UTF_VERIFY_EQUAL6_STRING_MAC(prvTgfPacketsCountTriggerArr[0], tmpTrigger,
                         "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                                 prvTgfPacketL2Part.saMac[0], prvTgfPacketL2Part.saMac[1], prvTgfPacketL2Part.saMac[2],
                                 prvTgfPacketL2Part.saMac[3], prvTgfPacketL2Part.saMac[4], prvTgfPacketL2Part.saMac[5]);


    /* check second pattern of TriggerCounters */
    tmpTrigger = ((numTriggers & (1 << 1)) != 0) ? 1 : 0;
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketsCountTriggerArr[1], tmpTrigger,
                          "VLAN tag of captured packet must be: 81 00 00 06\n");


    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);
}

/**
* @internal prvTgfTunnelTermMaskMacInMimTtiLookupConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelTermMaskMacInMimTtiLookupConfigRestore
(
    GT_VOID
)
{
    /* restore default DA */
    prvTgfPacketPartArray[4].partPtr = (GT_VOID*) &prvTgfPacketL2Part;

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfTunnelTermTestReset();
}

/**
* @internal prvTgfTunnelTermMimBasicConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set index of the rule in the TCAM to 0
*         - set macMode PRV_TGF_TTI_MAC_MODE_DA_E
*         - set MAC mask as disable
*/
GT_VOID prvTgfTunnelTermMimBasicConfigSet
(
    GT_VOID
)
{
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: SETUP CONFIGURATION: */
    prvTgfTunnelTermTestInit(0, PRV_TGF_TTI_MAC_MODE_DA_E, GT_TRUE);
    /* AUTODOC: set MAC_MODE_DA for TTI_KEY_MIM */
    /* AUTODOC: add TTI rule 0 with: */
    /* AUTODOC:   key MIM, cmd FORWARD, redirectCmd REDIRECT_TO_EGRESS */
    /* AUTODOC:   pattern MIM pclId=4, vid=5, iSid=0x123456, iUp=0x5, iDp=0 */
    /* AUTODOC:   pattern MIM MAC=00:00:00:00:34:05 */
    /* AUTODOC:   action Tunnel Terminate, PASSENGER_ETHERNET_NO_CRC, port=3 */
}


/**
* @internal prvTgfTunnelTermMimInlifConfigurationSet function
* @endinternal
*
* @brief   Set Inlif Configuration
*/
GT_VOID prvTgfTunnelTermMimInlifConfigurationSet
(
    GT_VOID
)
{
#ifdef EXMXPM_FAMILY


    GT_STATUS                               rc = GT_OK;
    PRV_TGF_INLIF_ENTRY_STC                 inlifEntry;

    PRV_UTF_LOG0_MAC("======= Setting Inlif Route Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 1. Set Inlif Entry - that needs for Puma devices
     */

    /* Bridge Section */
    inlifEntry.bridgeEnable              = GT_TRUE;
    inlifEntry.autoLearnEnable           = GT_TRUE;
    inlifEntry.naMessageToCpuEnable      = GT_TRUE;
    inlifEntry.naStormPreventionEnable   = GT_FALSE;
    inlifEntry.unkSaUcCommand            = CPSS_PACKET_CMD_BRIDGE_E;
    inlifEntry.unkDaUcCommand            = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unkSaNotSecurBreachEnable = GT_TRUE;
    inlifEntry.untaggedMruIndex          = 0;
    inlifEntry.unregNonIpMcCommand       = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregIpMcCommand          = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregIpv4BcCommand        = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregNonIpv4BcCommand     = CPSS_PACKET_CMD_FORWARD_E;

    /* Router Section */
    inlifEntry.ipv4UcRouteEnable = GT_TRUE;
    inlifEntry.ipv4McRouteEnable = GT_FALSE;
    inlifEntry.ipv6UcRouteEnable = GT_FALSE;
    inlifEntry.ipv6McRouteEnable = GT_FALSE;
    inlifEntry.mplsRouteEnable   = GT_FALSE;
    inlifEntry.vrfId             = 0;

    /* SCT Section */
    inlifEntry.ipv4IcmpRedirectEnable         = GT_FALSE;
    inlifEntry.ipv6IcmpRedirectEnable         = GT_FALSE;
    inlifEntry.bridgeRouterInterfaceEnable    = GT_FALSE;
    inlifEntry.ipSecurityProfile              = 0;
    inlifEntry.ipv4IgmpToCpuEnable            = GT_FALSE;
    inlifEntry.ipv6IcmpToCpuEnable            = GT_FALSE;
    inlifEntry.udpBcRelayEnable               = GT_FALSE;
    inlifEntry.arpBcToCpuEnable               = GT_FALSE;
    inlifEntry.arpBcToMeEnable                = GT_FALSE;
    inlifEntry.ripv1MirrorEnable              = GT_FALSE;
    inlifEntry.ipv4LinkLocalMcCommandEnable   = GT_FALSE;
    inlifEntry.ipv6LinkLocalMcCommandEnable   = GT_FALSE;
    inlifEntry.ipv6NeighborSolicitationEnable = GT_FALSE;

    /* Misc Section */
    inlifEntry.mirrorToAnalyzerEnable = GT_FALSE;
    inlifEntry.mirrorToCpuEnable      = GT_FALSE;

     /* save inlif port mode */
    rc = prvTgfInlifPortModeGet(PRV_TGF_SEND_PORT_IDX_CNS, &inlifMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfInlifPortModeGet");

    /* set inlif mode to support single input logical interface for port 0  */
    rc = prvTgfInlifPortModeSet(PRV_TGF_SEND_PORT_IDX_CNS,
                                   PRV_TGF_INLIF_PORT_MODE_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfInlifPortModeSet");

    rc = prvTgfInlifEntrySet(PRV_TGF_INLIF_TYPE_PORT_E,
                             prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                             &inlifEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfInlifEntrySet: %d", prvTgfDevNum);


     /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* Force-Vlan-Mode-on-TT */
    rc = prvTgfInlifIntForceVlanModeOnTtSet(prvTgfDevNum,GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfInlifIntForceVlanModeOnTtSet");

    /* Disable VLAN Egress Filtering on specified device for Bridged Known Unicast packets*/
    rc = prvTgfBrgVlanEgressFilteringEnable(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfInlifIntForceVlanModeOnTtSet");

#endif /*EXMXPM_FAMILY*/
}

/**
* @internal prvTgfTunnelTermMimBasicTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 MIM tunneled packet:
*         macDa = 00:00:00:00:00:22,
*         macSa = 00:00:00:00:34:02,
*         Tagged: true (with VLAN tag 5)
*         Ether Type: 0x8988 (MIM Ether type)
*         iSid: 0x123456
*         iUP: 0x5
*         iDP: 0
*         Success Criteria:
*         Packet striped and forwarded to port 23 VLAN 6
* @param[in] expectTraffic            - whether to expect traffic on the egress port
*                                       None
*/
GT_VOID prvTgfTunnelTermMimBasicTrafficGenerate
(
    GT_BOOL   expectTraffic
)
{
    GT_STATUS                       rc           = GT_OK;
    GT_U32                          portIter     = 0;
    GT_BOOL                         isEqualCntrs = GT_FALSE;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[2]={{0}};
    GT_U32                          tmpTrigger      = 0;
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    GT_U32                          packetSize;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

#ifdef EXMXPM_FAMILY
    if(GT_TRUE == PRV_CPSS_EXMXPM_ERRATA_GET_MAC(prvTgfDevNum,
                                            PRV_CPSS_EXMXPM_FER_TT_PASSENGER_SOURCE_TAG_SETTING))
    {
        /* override etherType in the prvTgfPacketBackboneVlanTagPart  */
        prvTgfPacketPartArray[1].partPtr = (GT_VOID*) &prvTgfPacketVlanTagPart;
    }
#endif

    /* send packet to be matched */
    /* AUTODOC: send MIM tunneled packet from port 0 with: */
    /* AUTODOC:   BB DA=00:00:00:00:34:05, SA=00:00:00:00:00:33 */
    /* AUTODOC:   BB EtherType=0x88A8, VID=5 */
    /* AUTODOC:   MIM EtherType=0x88E7 */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11, VID=5 */
    /* AUTODOC:   srcIP=10.10.10.10, dstIP=4.4.4.4 */
    prvTgfTunnelTermTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    PRV_UTF_LOG0_MAC("======= Checking Counters =======\n");

    /* AUTODOC: verify to get IPv4 packet on port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11 */
    /* AUTODOC:   srcIP=10.10.10.10, dstIP=4.4.4.4 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* clear expected counters */
        cpssOsMemSet(&expectedCntrs, 0, sizeof(expectedCntrs));

        /* calculate expected counters */
        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* original packet before tunnel terminate in VLAN 5 */
            packetSize = PRV_TGF_PACKET_CRC_LEN_CNS;

            expectedCntrs.goodOctetsSent.l[0] = packetSize * prvTgfBurstCount;
            expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
            expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
            expectedCntrs.goodOctetsRcv.l[0]  = packetSize * prvTgfBurstCount;
            expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
            expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
        }
        else if (PRV_TGF_EGR_PORT_IDX_CNS == portIter)
        {
            if (expectTraffic == GT_TRUE)
            {
                /* packet after tunnel terminate in VLAN 6 */
                packetSize = PRV_TGF_PACKET_CRC_LEN_CNS
                    - TGF_L2_HEADER_SIZE_CNS
                    - TGF_VLAN_TAG_SIZE_CNS
                    - TGF_ETHERTYPE_SIZE_CNS - sizeof(prvTgfPacketServicePayloadDataArr);

                expectedCntrs.goodOctetsSent.l[0] = packetSize * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.goodOctetsRcv.l[0] = packetSize * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]     = prvTgfBurstCount;
            }
            else
            {
                expectedCntrs.goodOctetsSent.l[0] = 0;
                expectedCntrs.goodPktsSent.l[0]   = 0;
                expectedCntrs.ucPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = 0;
                expectedCntrs.goodPktsRcv.l[0]    = 0;
                expectedCntrs.ucPktsRcv.l[0]      = 0;
            }
        }
        else if (PRV_TGF_FLOOD_PORT_IDX_CNS == portIter)
        {
            if (expectTraffic == GT_TRUE)
            {
                expectedCntrs.goodOctetsSent.l[0] = 0;
                expectedCntrs.goodPktsSent.l[0]   = 0;
                expectedCntrs.ucPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = 0;
                expectedCntrs.goodPktsRcv.l[0]    = 0;
                expectedCntrs.ucPktsRcv.l[0]      = 0;
            }
            else
            {
                /* packet after flooding in VLAN 5 */
                packetSize = PRV_TGF_PACKET_CRC_LEN_CNS;

                expectedCntrs.goodOctetsSent.l[0] = packetSize * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.goodOctetsRcv.l[0] = 0;
                expectedCntrs.goodPktsRcv.l[0]   = 0;
                expectedCntrs.ucPktsRcv.l[0]     = 0;
            }
        }

        /* check Rx\Tx counters */
        PRV_TGF_VERIFY_COUNTERS_MAC(isEqualCntrs, expectedCntrs, portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqualCntrs, GT_TRUE, "get another counters values.");

        /* print expected values if not equal */
        PRV_TGF_PRINT_DIFF_COUNTERS_MAC(isEqualCntrs, expectedCntrs, portCntrs);
    }

    /* set interface info */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* clear triggers */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));


    if (PRV_TGF_EGR_PORT_IDX_CNS == portIter)
    {
        /* get trigger counters where packet has MAC SA */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = sizeof(TGF_MAC_ADDR);
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));

        /* get trigger counters where packet has vlan tag 6 */
        vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[1].modeExtraInfo = 0;
        vfdArray[1].offset = 12;
        vfdArray[1].cycleCount = 4;
        vfdArray[1].patternPtr[0] = 0x81;
        vfdArray[1].patternPtr[1] = 0x00;
        vfdArray[1].patternPtr[2] = 0x00;
        vfdArray[1].patternPtr[3] = 0x06;

        /* get trigger counters */
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 2, vfdArray, &numTriggers);
        PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

        PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

        /* check first pattern of TriggerCounters */
        tmpTrigger = ((numTriggers & 1) != 0) ? 1 : 0;
        UTF_VERIFY_EQUAL6_STRING_MAC(prvTgfPacketsCountTriggerArr[0], tmpTrigger,
                             "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                                     prvTgfPacketL2Part.saMac[0], prvTgfPacketL2Part.saMac[1], prvTgfPacketL2Part.saMac[2],
                                     prvTgfPacketL2Part.saMac[3], prvTgfPacketL2Part.saMac[4], prvTgfPacketL2Part.saMac[5]);


        /* check second pattern of TriggerCounters */
        tmpTrigger = ((numTriggers & (1 << 1)) != 0) ? 1 : 0;
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketsCountTriggerArr[1], tmpTrigger,
                              "VLAN tag of captured packet must be: 81 00 00 06\n");


        /* check TriggerCounters */
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                     portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    }
    else if (PRV_TGF_FLOOD_PORT_IDX_CNS == portIter)
    {
            /* calculate triggers */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = sizeof(TGF_MAC_ADDR);
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));

        /* get trigger counters */
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
        PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

        /* check TriggerCounters */
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                     portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    }
}

/**
* @internal prvTgfTunnelTermBasicMimConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelTermBasicMimConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfTunnelTermTestReset();
}


static void mimPassengerOuterTag0or1TestConfig
(
    IN GT_BOOL      start,
    IN GT_U32       senderPortIndex,
    IN GT_U32       targetPortIndex
)
{
    GT_STATUS                                   rc;
    static GT_U16                               ethertypeOrig[8];
    static GT_U16                               pvidOrig;
    GT_U32                                      tpidIndex = 1;
    static GT_U32                               mimPclId=0;
    static GT_U32                               origSenderTpidBmpArr[3];
    static GT_U32                               ethertype;
    static PRV_TGF_TTI_KEY_SIZE_ENT             keySize;
    PRV_TGF_TTI_RULE_UNT                        pattern;
    PRV_TGF_TTI_RULE_UNT                        mask;
    PRV_TGF_TTI_ACTION_2_STC                    ruleAction;
    PRV_TGF_TTI_RULE_UNT                        ttiPattern_forUdbs;
    PRV_TGF_TTI_RULE_UNT                        ttiMask_forUdbs;

    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&ruleAction, 0, sizeof(ruleAction));

    cpssOsMemSet(&ttiPattern_forUdbs, 0, sizeof(ttiPattern_forUdbs));
    cpssOsMemSet(&ttiMask_forUdbs, 0, sizeof(ttiMask_forUdbs));


    if(start == GT_TRUE)
    {
        /* Get PVID of sender port */
        rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,prvTgfPortsArray[senderPortIndex],&pvidOrig);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d",
                                     prvTgfPortsArray[senderPortIndex]);

        /* Set PVID of sender port to a Vland Id which is not associated with any port */
        rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,prvTgfPortsArray[senderPortIndex],3333);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %x",
                                     prvTgfPortsArray[senderPortIndex],3333);

        prvTgfPacketVlanOuterTagPart.vid = passengerVidArr[0];
        prvTgfPacketVlanInnerTagPart.vid = passengerVidArr[1];

        /* AUTODOC: add vlan 0x123 for the passenger tags , set egress port as 'outer tag0 inner tag1' */

        rc = prvTgfBrgDefVlanEntryWrite(passengerVidArr[0]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", passengerVidArr[0]);

        rc = prvTgfBrgVlanTagMemberAdd(passengerVidArr[0],prvTgfPortsArray[targetPortIndex],PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTagMemberAdd: %d,%d,%d",
                passengerVidArr[0],prvTgfPortsArray[targetPortIndex],PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);



        /* get EtherType in TPID table */
        rc = prvTgfBrgVlanTpidEntryGet(CPSS_DIRECTION_INGRESS_E,tpidIndex,&ethertypeOrig[tpidIndex]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntryGet: %d",
                                     prvTgfDevNum);

        /* AUTODOC: add ingress TPID entry 1 with MIM B-EtherType 0x88A8 */
        rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,tpidIndex,TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %x",
                                     prvTgfDevNum,TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS);
        tpidIndex++;

        /* get EtherType in TPID table */
        rc = prvTgfBrgVlanTpidEntryGet(CPSS_DIRECTION_INGRESS_E,tpidIndex,&ethertypeOrig[tpidIndex]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntryGet: %d",
                                     prvTgfDevNum);

        /* AUTODOC: add ingress TPID entry 2 with EtherType 0x9999 */
        rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,tpidIndex,TGF_ETHERTYPE_9999_VLAN_TAG_CNS);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %x",
                                     prvTgfDevNum,TGF_ETHERTYPE_9999_VLAN_TAG_CNS);
        tpidIndex++;


        /* get EtherType in TPID table */
        rc = prvTgfBrgVlanTpidEntryGet(CPSS_DIRECTION_INGRESS_E,tpidIndex,&ethertypeOrig[tpidIndex]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntryGet: %d",
                                     prvTgfDevNum);

        /* AUTODOC: add ingress TPID entry 3 with EtherType 0xAAAA */
        rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,tpidIndex,TGF_ETHERTYPE_AAAA_VLAN_TAG_CNS);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %x",
                                     prvTgfDevNum,TGF_ETHERTYPE_AAAA_VLAN_TAG_CNS);
        tpidIndex++;

       /* rc = prvTgfBrgVlanPortIngressTpidGet(prvTgfDevNum,prvTgfPortsArray[senderPortIndex],
                    CPSS_VLAN_ETHERTYPE0_E,
                    &origSenderTpidBmpArr[CPSS_VLAN_ETHERTYPE0_E]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidGet: %d", prvTgfDevNum);*/

        /* save tag0 tpid profile per ingress port profile */
        rc = prvTgfBrgVlanPortIngressTpidProfileGet(prvTgfDevNum,prvTgfPortsArray[senderPortIndex],CPSS_VLAN_ETHERTYPE0_E,GT_TRUE,&profile0Get);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidProfileGet: %d", profile0Get);

        /* save tag1 tpid profile per ingress port profile */
        rc = prvTgfBrgVlanPortIngressTpidProfileGet(prvTgfDevNum,prvTgfPortsArray[senderPortIndex],CPSS_VLAN_ETHERTYPE1_E,GT_TRUE,&profile1Get);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidProfileGet: %d", profile1Get);

        /* save  bitmap of TPID (Tag Protocol ID) per received profile for tag0 */
        rc = prvTgfBrgVlanIngressTpidProfileGet(prvTgfDevNum,profile0Get,CPSS_VLAN_ETHERTYPE0_E,&origSenderTpidBmpArr[CPSS_VLAN_ETHERTYPE0_E]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIngressTpidProfileGet: %d", origSenderTpidBmpArr[CPSS_VLAN_ETHERTYPE0_E]);
        /* save  bitmap of TPID (Tag Protocol ID) per received profile for tag1 */
        rc = prvTgfBrgVlanIngressTpidProfileGet(prvTgfDevNum,profile1Get,CPSS_VLAN_ETHERTYPE1_E,&origSenderTpidBmpArr[CPSS_VLAN_ETHERTYPE1_E]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIngressTpidProfileGet: %d", origSenderTpidBmpArr[CPSS_VLAN_ETHERTYPE1_E]);


        /* AUTODOC: set bits 1,2 to TPID0 bmp for ingress port */

        /* sets bitmap of TPID (Tag Protocol ID) for tag0 per profile */
        rc =  prvTgfBrgVlanIngressTpidProfileSet(prvTgfDevNum,profile0Get,CPSS_VLAN_ETHERTYPE0_E,BIT_1 | BIT_2);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIngressTpidProfileSet: %d", prvTgfDevNum);

        /* set TPID (Tag Protocol ID) for tag0 profile per ingress port */
        /* set pre-tti-lookup-ingress-eport table */
        rc =  prvTgfBrgVlanPortIngressTpidProfileSet(prvTgfDevNum,prvTgfPortsArray[senderPortIndex],CPSS_VLAN_ETHERTYPE0_E,GT_TRUE,profile0Get);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidProfileSet: %d", prvTgfDevNum);

        /* set TPID (Tag Protocol ID) for tag0 profile per ingress port */
        /* set pre-tti-lookup-ingress-eport table */
        rc =  prvTgfBrgVlanPortIngressTpidProfileSet(prvTgfDevNum,prvTgfPortsArray[senderPortIndex],CPSS_VLAN_ETHERTYPE0_E,GT_FALSE,profile0Get);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidProfileSet: %d", prvTgfDevNum);


        /* AUTODOC: set bit 3 to TPID1 bmp for ingress port */

        /* set bitmap of TPID (Tag Protocol ID) for tag1 per profile */
        rc =  prvTgfBrgVlanIngressTpidProfileSet(prvTgfDevNum,profile1Get,CPSS_VLAN_ETHERTYPE1_E,BIT_3);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIngressTpidProfileSet: %d", prvTgfDevNum);

        /* set TPID (Tag Protocol ID) for tag1 profile per ingress port */
        /* set pre-tti-lookup-ingress-eport table */
        rc =  prvTgfBrgVlanPortIngressTpidProfileSet(prvTgfDevNum,prvTgfPortsArray[senderPortIndex],CPSS_VLAN_ETHERTYPE1_E,GT_TRUE,profile1Get);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidProfileSet: %d", prvTgfDevNum);

        /* set TPID (Tag Protocol ID) for tag1 profile per ingress port */
        /* set pre-tti-lookup-ingress-eport table */
        rc =  prvTgfBrgVlanPortIngressTpidProfileSet(prvTgfDevNum,prvTgfPortsArray[senderPortIndex],CPSS_VLAN_ETHERTYPE1_E,GT_FALSE,profile1Get);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidProfileSet: %d", prvTgfDevNum);


        rc = prvTgfTtiEthernetTypeGet(prvTgfDevNum,PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED0_E, &ethertype);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidGet: %d", prvTgfDevNum);


        /* AUTODOC: for PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED0_E set ethertype to 0x88E78 (MIM) */
        rc = prvTgfTtiEthernetTypeSet(prvTgfDevNum,
                                      PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED0_E, TGF_ETHERTYPE_88E7_MIM_TAG_CNS);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEthernetTypeSet: %d",
                                     prvTgfDevNum);

        /* AUTODOC: enable TTI lookup for port 0, flexible key TTI_KEY_UDB */
        rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[senderPortIndex], PRV_TGF_TTI_KEY_UDB_UDE_E, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

        rc = prvTgfTtiPacketTypeKeySizeGet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_UDE_E,&keySize);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeGet: %d", prvTgfDevNum);
        /* AUTODOC: set key size for PRV_TGF_TTI_KEY_UDB_UDE_E */
        rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_UDE_E, PRV_TGF_TTI_KEY_SIZE_30_B_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);


        rc = prvTgfTtiPclIdGet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_UDE_E,&mimPclId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdGet");

        /* AUTODOC: set PCLID for MIM packets */
        rc = prvTgfTtiPclIdSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_UDE_E, PRV_TGF_TTI_MIM_PCL_ID_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet");

        /* configure TTI rule action */
        ruleAction.tunnelTerminate     = GT_TRUE;
        ruleAction.ttHeaderLength      = 4;

        ruleAction.ttPassengerPacketType = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
        ruleAction.command             = CPSS_PACKET_CMD_FORWARD_E;
        ruleAction.redirectCommand     = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
        ruleAction.tag0VlanCmd         = PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E;
        ruleAction.tag0VlanId          = 0xFFF;
        ruleAction.tag1VlanCmd         = PRV_TGF_TTI_VLAN1_MODIFY_UNTAGGED_E;
        ruleAction.tag1VlanId          = 0xEEE;
        ruleAction.bridgeBypass        = GT_TRUE;
        ruleAction.userDefinedCpuCode  = CPSS_NET_FIRST_USER_DEFINED_E;

        ruleAction.egressInterface.type            = CPSS_INTERFACE_PORT_E;
        ruleAction.egressInterface.devPort.hwDevNum  = prvTgfDevNum;
        ruleAction.egressInterface.devPort.portNum = prvTgfPortsArray[targetPortIndex];

        /* clear entries */
        cpssOsMemSet((GT_VOID*) &pattern, 0, sizeof(pattern));
        cpssOsMemSet((GT_VOID*) &mask, 0, sizeof(mask));

        /* configure TTI rule mask */
        mask.mim.common.pclId = 0x3FF;
        mask.mim.passengerPacketOuterTagExists=GT_TRUE;
        mask.mim.passengerPacketOuterTagVid = 0xFFF;

        /* configure TTI rule pattern */
        pattern.mim.common.pclId = PRV_TGF_TTI_MIM_PCL_ID_CNS;
        pattern.mim.passengerPacketOuterTagExists = GT_TRUE;
        pattern.mim.passengerPacketOuterTagVid = 0x123;

        /* convert the legacy mim pattern/mask to UDBs style pattern/mask */
        rc = prvTgfTunnelTermEmulateLegacyMimForUdbOnlyMimPatternMaskBuild(
                &pattern,
                &mask ,
                &ttiPattern_forUdbs ,
                &ttiMask_forUdbs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelTermEmulateLegacyMiMForUdbOnlyMiMPatternMaskBuild: %d", prvTgfDevNum);


        /* set TTI rule */
        rc = prvTgfTtiRule2Set(PRV_TGF_MIM_TTI_INDEX_CNS, PRV_TGF_TTI_KEY_UDB_UDE_E, &ttiPattern_forUdbs, &ttiMask_forUdbs, &ruleAction);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);
        prvTgfTunnelTermEmulateLegacyMimForUdbOnlyMimKeyNeededUdbsSet(PRV_TGF_TTI_KEY_UDB_UDE_E);
    }
    else
    {
        PRV_UTF_LOG0_MAC("======= Restore Configuration =======\n");

        PRV_UTF_LOG0_MAC("======= reset mac counters on all tested ports. =======\n");
        prvTgfEthCountersReset(prvTgfDevNum);

        /* AUTODOC: invalidate the vlan 0x123 for the passenger tags*/

        rc = prvTgfBrgVlanEntryInvalidate(passengerVidArr[0]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, passengerVidArr[0]);


        /* AUTODOC: restore ethertypes index 1..3 */
        rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,tpidIndex,ethertypeOrig[tpidIndex]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %x",
                                     prvTgfDevNum,ethertypeOrig[tpidIndex]);
        tpidIndex++;

        rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,tpidIndex,ethertypeOrig[tpidIndex]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %x",
                                     prvTgfDevNum,ethertypeOrig[tpidIndex]);
        tpidIndex++;

        rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,tpidIndex,ethertypeOrig[tpidIndex]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %x",
                                     prvTgfDevNum,ethertypeOrig[tpidIndex]);
        tpidIndex++;


        /* AUTODOC: restore TPID0 bmp for ingress port */

        /* sets bitmap of TPID (Tag Protocol ID) for tag0 per profile */
        rc =  prvTgfBrgVlanIngressTpidProfileSet(prvTgfDevNum,profile0Get,CPSS_VLAN_ETHERTYPE0_E,origSenderTpidBmpArr[CPSS_VLAN_ETHERTYPE0_E]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIngressTpidProfileSet: %d", prvTgfDevNum);

        /* set TPID (Tag Protocol ID) for tag0 profile per ingress port */
        /* set pre-tti-lookup-ingress-eport table */
        rc =  prvTgfBrgVlanPortIngressTpidProfileSet(prvTgfDevNum,prvTgfPortsArray[senderPortIndex],CPSS_VLAN_ETHERTYPE0_E,GT_TRUE,profile0Get);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidProfileSet: %d", prvTgfDevNum);

        /* set TPID (Tag Protocol ID) for tag0 profile per ingress port */
        /* set pre-tti-lookup-ingress-eport table */
        rc =  prvTgfBrgVlanPortIngressTpidProfileSet(prvTgfDevNum,prvTgfPortsArray[senderPortIndex],CPSS_VLAN_ETHERTYPE0_E,GT_FALSE,profile0Get);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidProfileSet: %d", prvTgfDevNum);


        /* AUTODOC: restore TPID1 bmp for ingress port */

        /* set bitmap of TPID (Tag Protocol ID) for tag1 per profile */
        rc =  prvTgfBrgVlanIngressTpidProfileSet(prvTgfDevNum,profile1Get,CPSS_VLAN_ETHERTYPE1_E,origSenderTpidBmpArr[CPSS_VLAN_ETHERTYPE1_E]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIngressTpidProfileSet: %d", prvTgfDevNum);

        /* set TPID (Tag Protocol ID) for tag1 profile per ingress port */
        /* set pre-tti-lookup-ingress-eport table */
        rc =  prvTgfBrgVlanPortIngressTpidProfileSet(prvTgfDevNum,prvTgfPortsArray[senderPortIndex],CPSS_VLAN_ETHERTYPE1_E,GT_TRUE,profile1Get);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidProfileSet: %d", prvTgfDevNum);

        /* set TPID (Tag Protocol ID) for tag1 profile per ingress port */
        /* set pre-tti-lookup-ingress-eport table */
        rc =  prvTgfBrgVlanPortIngressTpidProfileSet(prvTgfDevNum,prvTgfPortsArray[senderPortIndex],CPSS_VLAN_ETHERTYPE1_E,GT_FALSE,profile1Get);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidProfileSet: %d", prvTgfDevNum);

        /* AUTODOC: restore default ethernet type for PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED0_E */
        rc = prvTgfTtiEthernetTypeSet(prvTgfDevNum,
                                      PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED0_E, ethertype);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEthernetTypeSet: %d",
                                     prvTgfDevNum);

        /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_UDB */
        rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_UDB_UDE_E, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

        /* AUTODOC: restore  key size for PRV_TGF_TTI_KEY_UDB_UDE_E */
        rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_UDE_E, keySize);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

        /* AUTODOC: restore MIM PCLID */
        rc = prvTgfTtiPclIdSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_UDE_E,mimPclId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet");

        /* AUTODOC: invalidate TTI rules */
        rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_MIM_TTI_INDEX_CNS, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d", prvTgfDevNum, GT_FALSE);

        /* Restore PVID of sender port */
        rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,prvTgfPortsArray[senderPortIndex],pvidOrig);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %x",
                                     prvTgfPortsArray[senderPortIndex],pvidOrig);

        /* flush FDB include static entries */
        rc = prvTgfBrgFdbFlush(GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);
    }
}

static void mimPassengerOuterTag0or1TestConfigRule
(
    IN GT_U32       targetPortIndex
)
{
    GT_STATUS                                   rc;
    PRV_TGF_TTI_RULE_UNT                        pattern;
    PRV_TGF_TTI_RULE_UNT                        mask;
    PRV_TGF_TTI_ACTION_2_STC                    ruleAction;
    PRV_TGF_TTI_RULE_UNT                        ttiPattern_forUdbs;
    PRV_TGF_TTI_RULE_UNT                        ttiMask_forUdbs;

    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&ruleAction, 0, sizeof(ruleAction));

    cpssOsMemSet(&ttiPattern_forUdbs, 0, sizeof(ttiPattern_forUdbs));
    cpssOsMemSet(&ttiMask_forUdbs, 0, sizeof(ttiMask_forUdbs));


    /* configure TTI rule action */
    ruleAction.tunnelTerminate     = GT_TRUE;
    ruleAction.ttHeaderLength      = 4;

    ruleAction.ttPassengerPacketType = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
    ruleAction.command             = CPSS_PACKET_CMD_FORWARD_E;
    ruleAction.redirectCommand     = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ruleAction.tag0VlanCmd         = PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E;
    ruleAction.tag0VlanId          = 0xFFF;
    ruleAction.tag1VlanCmd         = PRV_TGF_TTI_VLAN1_MODIFY_UNTAGGED_E;
    ruleAction.tag1VlanId          = 0xEEE;
    ruleAction.bridgeBypass        = GT_TRUE;
    ruleAction.userDefinedCpuCode  = CPSS_NET_FIRST_USER_DEFINED_E;

    ruleAction.egressInterface.type            = CPSS_INTERFACE_PORT_E;
    ruleAction.egressInterface.devPort.hwDevNum  = prvTgfDevNum;
    ruleAction.egressInterface.devPort.portNum = prvTgfPortsArray[targetPortIndex];

    /* clear entries */
    cpssOsMemSet((GT_VOID*) &pattern, 0, sizeof(pattern));
    cpssOsMemSet((GT_VOID*) &mask, 0, sizeof(mask));

    /* configure TTI rule mask */
    mask.mim.common.pclId = 0x3FF;
    mask.mim.passengerPacketOuterTagExists=GT_TRUE;
    mask.mim.passengerPacketOuterTagVid = 0xFFF;

    /* configure TTI rule pattern */
    pattern.mim.common.pclId = PRV_TGF_TTI_MIM_PCL_ID_CNS;
    pattern.mim.passengerPacketOuterTagExists = GT_TRUE;
    pattern.mim.passengerPacketOuterTagVid = 0x456;

    /* convert the legacy mim pattern/mask to UDBs style pattern/mask */
    rc = prvTgfTunnelTermEmulateLegacyMimForUdbOnlyMimPatternMaskBuild(
            &pattern,
            &mask ,
            &ttiPattern_forUdbs ,
            &ttiMask_forUdbs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelTermEmulateLegacyMiMForUdbOnlyMiMPatternMaskBuild: %d", prvTgfDevNum);


    /* set TTI rule */
    rc = prvTgfTtiRule2Set(PRV_TGF_MIM_TTI_INDEX_CNS, PRV_TGF_TTI_KEY_UDB_UDE_E, &ttiPattern_forUdbs, &ttiMask_forUdbs, &ruleAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);
    prvTgfTunnelTermEmulateLegacyMimForUdbOnlyMimKeyNeededUdbsSet(PRV_TGF_TTI_KEY_UDB_UDE_E);

}

static void mimPassengerOuterTag0or1TestSwapPacketTagsInnerToOuter()
{
    prvTgfPacketVlanOuterTagPart.etherType = TGF_ETHERTYPE_AAAA_VLAN_TAG_CNS;
    prvTgfPacketVlanOuterTagPart.vid = passengerVidArr[1];
    prvTgfPacketVlanInnerTagPart.etherType = TGF_ETHERTYPE_9999_VLAN_TAG_CNS;
    prvTgfPacketVlanInnerTagPart.vid = passengerVidArr[0];

}

static void mimPassengerOuterTag0or1TestSwapPacketTagsOuterToInner()
{
    prvTgfPacketVlanOuterTagPart.etherType = TGF_ETHERTYPE_9999_VLAN_TAG_CNS;
    prvTgfPacketVlanOuterTagPart.vid = passengerVidArr[0];
    prvTgfPacketVlanInnerTagPart.etherType = TGF_ETHERTYPE_AAAA_VLAN_TAG_CNS;
    prvTgfPacketVlanInnerTagPart.vid = passengerVidArr[1];

}
/**
* @internal captureEgressTrafficOnPort function
* @endinternal
*
* @brief   start/stop capturing on specific port of the test .
*/
static void captureEgressTrafficOnPort
(
    IN GT_BOOL      start,
    IN GT_U32       capturedPortIndex
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC portInterface;

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevsArray[capturedPortIndex];
    portInterface.devPort.portNum = prvTgfPortsArray[capturedPortIndex];

    if(start ==GT_FALSE)
    {
        /* wait for packets come to CPU */
        (void) tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount , 500, NULL);
    }

    /* Start/Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, start);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum, start);
}

/**
* @internal checkCaptureEgressTrafficOnPort function
* @endinternal
*
* @brief   check captured egress on specific port of the test .
*
* @param[in] passengerTagMode         - passenger tag mode
* @param[in] capturedPortIndex        - the captured port index to check
* @param[in] sentPacketInfoPtr        - the packet that was sent (as reference to most parts of the expected packet)
* @param[in] expectPackets            - GT_TRUE: Expect packets on port
*                                      GT_FALSE: Do not expect any packets on port
*                                       None
*/
static void checkCaptureEgressTrafficOnPort
(
    IN CPSS_ETHER_MODE_ENT      passengerTagMode,
    IN TGF_PACKET_STC *sentPacketInfoPtr,
    IN GT_U32       capturedPortIndex,
    IN GT_BOOL      expectPackets
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_PACKET_STC *expectedPacketInfoPtr;
    GT_U32  actualCapturedNumOfPackets;/*actual number of packet captured from the interface*/
    GT_U32  ii;
    GT_U32 numberOfPackets; /* number of packets expected to be captured */

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[capturedPortIndex];

    PRV_UTF_LOG1_MAC("check Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* copy the expected packet from the ingress packet */
    cpssOsMemCpy(prvTgfEgressPassengerPacketPartArray,
        sentPacketInfoPtr->partsArray,
        sizeof(prvTgfEgressPassengerPacketPartArray));

    UTF_VERIFY_EQUAL2_STRING_MAC(sentPacketInfoPtr->numOfParts, prvTgfEgressPassengerPacketInfo.numOfParts,
                                 "checkCaptureEgressTrafficOnPort:prvTgfEgressPassengerPacketInfo.numOfParts[%d] != sentPacketInfoPtr->numOfParts[%d]"
                                 ,prvTgfEgressPassengerPacketInfo.numOfParts , sentPacketInfoPtr->numOfParts);

    /* remove the tunnel part until we are in the passenger part */
    for(ii = 1/*skip first L2*/ ; ii < prvTgfEgressPassengerPacketInfo.numOfParts; ii++)
    {
        prvTgfEgressPassengerPacketPartArray[ii-1].type = TGF_PACKET_PART_SKIP_E;
        prvTgfEgressPassengerPacketPartArray[ii-1].partPtr = NULL;

        if(prvTgfEgressPassengerPacketPartArray[ii].type == TGF_PACKET_PART_L2_E)
        {
            break;
        }
    }

    UTF_VERIFY_NOT_EQUAL1_STRING_MAC(ii, prvTgfEgressPassengerPacketInfo.numOfParts,
                                 "checkCaptureEgressTrafficOnPort: ii[%d] == prvTgfEgressPassengerPacketInfo.numOfParts"
                                 ,ii);

    expectedPacketInfoPtr = &prvTgfEgressPassengerPacketInfo;

    /* the egress not know to preserve the ingress ethertype , so expect 0x8100 */
    prvTgfPacketVlanTagExpectedPart[0] = prvTgfPacketVlanOuterTagPart;
    prvTgfPacketVlanTagExpectedPart[0].etherType = TGF_ETHERTYPE_8100_VLAN_TAG_CNS;
    prvTgfPacketVlanTagExpectedPart[1] = prvTgfPacketVlanInnerTagPart;
    prvTgfPacketVlanTagExpectedPart[1].etherType = TGF_ETHERTYPE_8100_VLAN_TAG_CNS;

    ii++;
    if(prvTgfEgressPassengerPacketInfo.partsArray[ii].type != TGF_PACKET_PART_VLAN_TAG_E)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfEgressPassengerPacketInfo.partsArray[ii].type , TGF_PACKET_PART_VLAN_TAG_E,
                                     "checkCaptureEgressTrafficOnPort: prvTgfEgressPassengerPacketInfo.partsArray[ii].type == [%d] not TGF_PACKET_PART_VLAN_TAG_E[%d]"
                                     ,prvTgfEgressPassengerPacketInfo.partsArray[ii].type , TGF_PACKET_PART_VLAN_TAG_E);
        return;
    }

    if(passengerTagMode == CPSS_VLAN_ETHERTYPE1_E)
    {
        /* swap the expected tag0 and tag1 in the egress packet */
        prvTgfEgressPassengerPacketInfo.partsArray[ii].partPtr = &prvTgfPacketVlanTagExpectedPart[1];
        ii++;
        prvTgfEgressPassengerPacketInfo.partsArray[ii].partPtr = &prvTgfPacketVlanTagExpectedPart[0];
    }
    else
    {
        prvTgfEgressPassengerPacketInfo.partsArray[ii].partPtr = &prvTgfPacketVlanTagExpectedPart[0];
        ii++;
        prvTgfEgressPassengerPacketInfo.partsArray[ii].partPtr = &prvTgfPacketVlanTagExpectedPart[1];
    }

    if (expectPackets == GT_TRUE)
    {
        numberOfPackets = prvTgfBurstCount;
    }
    else
    {
        numberOfPackets = 0;
    }
    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
            &portInterface,
            expectedPacketInfoPtr,
            numberOfPackets,/*numOfPackets*/
            0/*vfdNum*/,
            NULL /*vfdArray*/,
            NULL, /* bytesNum's skip list */
            0,    /* length of skip list */
            &actualCapturedNumOfPackets,
            NULL/*onFirstPacketNumTriggersBmpPtr*/);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                 " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);
}


/**
* @internal mimPassengerOuterTag0or1Run function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] passengerTagMode         - passenger tag mode
* @param[in] senderPortIndex          - index of the sending port
* @param[in] targetPortIndex          - index of the target port
* @param[in] expectPackets            - GT_TRUE: Expect packets on port
*                                      GT_FALSE: Do not expect any packets on port
*                                       None
*/
static GT_VOID mimPassengerOuterTag0or1Run
(
    IN CPSS_ETHER_MODE_ENT      passengerTagMode,
    IN GT_U32                   senderPortIndex,
    IN GT_U32                   targetPortIndex,
    IN GT_BOOL                  expectPackets
)
{
    GT_STATUS rc          ;
    TGF_PACKET_STC *packetInfoPtr = &prvTgfMimPacketInfo;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    PRV_UTF_LOG0_MAC("======= reset mac counters on all tested ports. =======\n");
    prvTgfEthCountersReset(prvTgfDevNum);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr , prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    /* start capturing on port */
    PRV_UTF_LOG1_MAC("======= start capturing on port [%d]of the test =======\n",prvTgfPortsArray[targetPortIndex]);
    captureEgressTrafficOnPort(GT_TRUE,targetPortIndex);

    /* -------------------------------------------------------------------------
     * 1. Sending MIM
     */
    PRV_UTF_LOG1_MAC("======= Sending MIM to ingress port [%d]=======\n",prvTgfPortsArray[senderPortIndex]);
    /*  send the packet to ingress port */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);

    PRV_UTF_LOG1_MAC("======= stop capturing on port [%d]of the test =======\n",prvTgfPortsArray[targetPortIndex]);
    /* stop capturing on port */
    captureEgressTrafficOnPort(GT_FALSE,targetPortIndex);

    PRV_UTF_LOG1_MAC("======= check captured egress traffic on port [%d]of the test =======\n",prvTgfPortsArray[targetPortIndex]);
    /*check captured egress traffic on all port of the test */
    checkCaptureEgressTrafficOnPort(passengerTagMode,packetInfoPtr,targetPortIndex, expectPackets);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}


/**
* @internal tgfTunnelTermMimPassengerOuterTag0or1Test function
* @endinternal
*
* @brief   Test MIM tunnel termination - passenger tag recognition - tag0 or tag1
*         according to test design
*         http://docil.marvell.com/webtop/drl/objectId/0900dd88801337dd
*         in Cabinets/SWITCHING/CPSS and PSS/R&D Internal/CPSS/DX/Test Design/Lion3
*/
GT_VOID tgfTunnelTermMimPassengerOuterTag0or1Test
(
    GT_VOID
)
{
    GT_U32       senderPortIndex = 3;
    GT_U32       targetPortIndex = 1;
    CPSS_ETHER_MODE_ENT      passengerTagMode;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);
    /* set configuration */
    PRV_UTF_LOG0_MAC("first packet send: passengerTagMode=CPSS_VLAN_ETHERTYPE0_E,ttiRule.passengerPacketOuterTagExists=0x123,packetPassenger=  99 99 01 23 aa aa 04 56\n"
                     "-->  outer tag is 0x123 ,tag0 is 0x123 so expect to have a match on the rule, and since port 18 is a member of vlan 0x123 recognized as eVlan (Tag0), packet will get to port 18.\n");
    passengerTagMode = CPSS_VLAN_ETHERTYPE0_E;
    prvTgfTtiPortPassengerOuterIsTag0Or1Set(prvTgfDevNum,prvTgfPortsArray[senderPortIndex],passengerTagMode);
    mimPassengerOuterTag0or1TestConfig(GT_TRUE/*start*/ ,senderPortIndex,targetPortIndex);
    /* send and check traffic */
    mimPassengerOuterTag0or1Run(passengerTagMode,senderPortIndex,targetPortIndex,GT_TRUE);

    PRV_UTF_LOG0_MAC("second packet send: passengerTagMode=CPSS_VLAN_ETHERTYPE0_E,ttiRule.passengerPacketOuterTagExists=0x123,packetPassenger= aa aa 04 56 99 99 01 23 \n"
                     "--> outer tag is 0x456 but tag0 is 0x123 so we will not have a match on the rule, packet will be dropped since no member in vlan 5 (original packet vlan).\n");

    mimPassengerOuterTag0or1TestSwapPacketTagsInnerToOuter();
    /* send and check no traffic */
    mimPassengerOuterTag0or1Run(passengerTagMode,senderPortIndex,targetPortIndex,GT_FALSE);

    PRV_UTF_LOG0_MAC("third packet send: passengerTagMode=CPSS_VLAN_ETHERTYPE1_E,ttiRule.passengerPacketOuterTagExists=0x123,packetPassenger= 99 99 01 23 aa aa 04 56 \n"
                     "--> outer tag is 0x123 but tag1 is 0x456 so we will not have a match on the rule, packet will be dropped since no member in vlan 5 (original packet vlan).\n");

    passengerTagMode = CPSS_VLAN_ETHERTYPE1_E;
    prvTgfTtiPortPassengerOuterIsTag0Or1Set(prvTgfDevNum,prvTgfPortsArray[senderPortIndex],passengerTagMode);
    mimPassengerOuterTag0or1TestSwapPacketTagsOuterToInner();
    /* send and check no traffic */
    mimPassengerOuterTag0or1Run(passengerTagMode,senderPortIndex,targetPortIndex,GT_FALSE);

    PRV_UTF_LOG0_MAC("forth packet send: passengerTagMode=CPSS_VLAN_ETHERTYPE1_E,ttiRule.passengerPacketOuterTagExists=0x123,packetPassenger= aa aa 04 56 99 99 01 23\n"
                     "--> outer tag is 0x456 and tag1 is 0x456 but the rule is 0x123 so we will not have a match on the rule, packet will be dropped since no member in vlan 5 (original packet vlan).\n");

    mimPassengerOuterTag0or1TestSwapPacketTagsInnerToOuter();

    /* send and check no traffic */
    mimPassengerOuterTag0or1Run(passengerTagMode,senderPortIndex,targetPortIndex,GT_FALSE);

    PRV_UTF_LOG0_MAC("fifth packet send: passengerTagMode=CPSS_VLAN_ETHERTYPE1_E,ttiRule.passengerPacketOuterTagExists=0x456,packetPassenger= aa aa 04 56 99 99 01 23\n"
                     "-->  outer tag is 0x456 ,tag1 is 0x456 so expect to have a match on the rule, and since port 18 is a member of vlan 0x123 recognized as eVlan (Tag0), packet will get to port 18.\n");
    /* configure new rule*/
    mimPassengerOuterTag0or1TestConfigRule(targetPortIndex);
    /* send and check traffic */
    mimPassengerOuterTag0or1Run(passengerTagMode,senderPortIndex,targetPortIndex,GT_TRUE);

    /* restore */
    mimPassengerOuterTag0or1TestSwapPacketTagsOuterToInner();
    mimPassengerOuterTag0or1TestConfig(GT_FALSE/*end*/ ,senderPortIndex,targetPortIndex);

   prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
}

/**
* @internal prvTgfTunnelTermMimMacToMeEnableSet function
* @endinternal
*
* @brief   enable/disable Mac in Mac TTI lookup only for mac to me packets
*
* @param[in] enable                   - GT_TRUE:  Mac in Mac TTI lookup only for mac to me packets
*                                      GT_FALSE: disable Mac in Mac TTI lookup only for mac to me packets
*                                       None
*/
GT_VOID prvTgfTunnelTermMimMacToMeEnableSet
(
    IN GT_BOOL    enable
)
{
    GT_STATUS rc;

    rc = prvTgfTtiPortMimOnlyMacToMeEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], enable);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfTtiPortMimOnlyMacToMeEnableSet: %d, %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_CNS, enable);
}

/**
* @internal prvTgfTunnelTermMimMacToMeSet function
* @endinternal
*
* @brief   Set a MAC 2 Me entry
*/
GT_VOID prvTgfTunnelTermMimMacToMeSet
(
    GT_VOID
)
{
    GT_STATUS                                   rc;
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMeMask;

    /* set MAC to ME table */
    cpssOsMemCpy((GT_VOID*)macToMePattern.mac.arEther,
                 (GT_VOID*)prvTgfPacketBackboneL2Part.daMac,
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
* @internal prvTgfTunnelTermMimMacToMeDel function
* @endinternal
*
* @brief   Delete a MAC 2 Me entry
*/
GT_VOID prvTgfTunnelTermMimMacToMeDel
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


#ifdef CHX_FAMILY
/* hook into CPSS internal logic */
extern GT_STATUS  sip5BuildUdbsNeededForLegacyKey_mim
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
* @internal prvTgfTunnelTermEmulateLegacyMimForUdbOnlyMimPatternMaskBuild function
* @endinternal
*
* @brief   build the pattern/mask UDBs that need to be emulated like legacy IPv4 pattern/mask.
*
* @param[out] udbPatternArray          - UDBs style pattern
* @param[out] udbMaskArray             - UDBs style mask
*                                       None
*/
GT_STATUS prvTgfTunnelTermEmulateLegacyMimForUdbOnlyMimPatternMaskBuild
(
    IN   PRV_TGF_TTI_RULE_UNT   *legacyMimPatternPtr,
    IN   PRV_TGF_TTI_RULE_UNT   *legacyMimMaskPtr,
    OUT  PRV_TGF_TTI_RULE_UNT   *udbPatternArray,
    OUT  PRV_TGF_TTI_RULE_UNT   *udbMaskArray
)
{
#ifndef CHX_FAMILY
    return GT_OK;
#else  /*!CHX_FAMILY*/
    GT_STATUS   rc;
    GT_U32   hwFormatArray[32]= {0};
    GT_U32   ii;
    CPSS_DXCH_TTI_RULE_UNT dxchPattern;
    CPSS_DXCH_TTI_RULE_UNT dxchMask;
    GT_U32  tmpValue;

    /* clear the array */
    cpssOsMemSet(udbPatternArray, 0, sizeof(PRV_TGF_TTI_RULE_UNT));
    cpssOsMemSet(udbMaskArray, 0, sizeof(PRV_TGF_TTI_RULE_UNT));

    /* convert from the TGF format to the DXCH format */
    prvTgfTtiRuleConvertGenericToDx(PRV_TGF_TTI_RULE_MIM_E,
            legacyMimPatternPtr,legacyMimMaskPtr,
            &dxchPattern,&dxchMask);

    /* for pattern - ask CPSS to build words array for 'legacy IPv4'  */
    rc = sip5TtiConfigSetLogic2HwUdbFormat(CPSS_DXCH_TTI_RULE_MIM_E, GT_TRUE ,
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
     cpssOsMemSet(hwFormatArray, 0, sizeof(hwFormatArray));
    /* for mask - ask CPSS to build words array for 'legacy IPv4'  */
    rc = sip5TtiConfigSetLogic2HwUdbFormat(CPSS_DXCH_TTI_RULE_MIM_E, GT_FALSE ,
        &dxchMask,
        hwFormatArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    for(ii = 0 ; ii < CPSS_DXCH_TTI_MAX_UDB_CNS; ii++)
    {
        U32_GET_FIELD_IN_ENTRY_MAC(hwFormatArray,8*ii , 8,tmpValue);
        udbMaskArray->udbArray[ii] = (GT_U8)tmpValue;
    }
    return GT_OK;
#endif /*!CHX_FAMILY*/
}


/**
* @internal prvTgfTunnelTermEmulateLegacyMimForUdbOnlyMimKeyNeededUdbsSet function
* @endinternal
*
* @brief   set the needed UDBs that need to be emulated like legacy MiM key.
*
* @param[in] keyType                  - UDB only key type, that need to be emulated like legacy MiM key.
*                                       None
*/
GT_VOID prvTgfTunnelTermEmulateLegacyMimForUdbOnlyMimKeyNeededUdbsSet
(
    PRV_TGF_TTI_KEY_TYPE_ENT     keyType
)
{
#ifndef CHX_FAMILY
    return ;
#else  /*!CHX_FAMILY*/
    GT_STATUS   rc;

    /* ask CPSS to build needed UDBs in same way as the legacy "PRV_TGF_TTI_KEY_IPV4_E" */
    rc = sip5BuildUdbsNeededForLegacyKey_mim(prvTgfDevNum, (CPSS_DXCH_TTI_KEY_TYPE_ENT)keyType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "sip5BuildUdbsNeededForLegacyKey_ipv4: %d", prvTgfDevNum);

    return;
#endif /*!CHX_FAMILY*/
}


