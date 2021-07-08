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
* @file prvTgfPclPumaPortGroup.c
*
* @brief Puma Port Group specific PCL features testing
*
* @version   7
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <pcl/prvTgfPclMiscellanous.h>

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT0_IDX_CNS     0
#define PRV_TGF_SEND_PORT1_IDX_CNS     3

/* dummy port index */
#define PRV_TGF_DUMMY_PORT_IDX_CNS   0xFFFF

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS  2

/* port number to receive traffic from */
#define PRV_TGF_PCL_RULE_IDX_CNS  5

/* configuration 0 new DSCP */
#define PRV_TGF_PCL_NEW_DSCP0_CNS  0x2B

/* configuration 1 new DSCP */
#define PRV_TGF_PCL_NEW_DSCP1_CNS  0x1A

/* configuration 0 TCP/UDB port comparator index */
#define PRV_TGF_PCL_COMPARATOR_INDEX0_CNS  3

/* configuration 1 TCP/UDB port comparator index */
#define PRV_TGF_PCL_COMPARATOR_INDEX1_CNS  5

/* UDB offest in MAC_DA */
#define PRV_TGF_PCL_UDB_OFFSET_IN_MAC_DA_CNS  5

/* Data of packet */
static GT_U8 prvTgfPacket1PayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};

/* Payload part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket1PayloadPart =
{
    sizeof(prvTgfPacket1PayloadDataArr), /* dataLength */
    prvTgfPacket1PayloadDataArr          /* dataPtr */
};

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket1_0_L2Part = {
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x44},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x55, 0x66}                 /* saMac */
};

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket1_1_L2Part = {
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x88},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x55, 0x66}                 /* saMac */
};

/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket1Ipv4EtherTypePart =
    {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* packet's IPv4 part */
static TGF_PACKET_IPV4_STC prvTgfPacket1_0_UdpIpPart =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    (TGF_IPV4_HEADER_SIZE_CNS
     + TGF_UDP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacket1PayloadDataArr)), /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    17,                 /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    {2, 2, 2, 2},       /* srcAddr */
    {1, 1, 1, 1}        /* dstAddr */
};

/* packet's IPv4 part */
static TGF_PACKET_IPV4_STC prvTgfPacket1_1_UdpIpPart =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    (TGF_IPV4_HEADER_SIZE_CNS
     + TGF_UDP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacket1PayloadDataArr)), /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    17,                 /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    {3, 3, 3, 3},       /* srcAddr */
    {1, 1, 1, 1}        /* dstAddr */
};

/* packet's UDP part */
static TGF_PACKET_UDP_STC prvTgfPacket1_0_UdpPart =
{
    0x31,                  /* src port */
    7,                     /* dst port */
    (TGF_UDP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacket1PayloadDataArr)), /* length */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS /* csum */
};

/* packet's UDP part */
static TGF_PACKET_UDP_STC prvTgfPacket1_1_UdpPart =
{
    0x32,                  /* src port */
    8,                     /* dst port */
    (TGF_UDP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacket1PayloadDataArr)), /* length */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS /* csum */
};

/* PARTS of packet UDP */
static TGF_PACKET_PART_STC prvTgfUdpPacket1_0_PartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1_0_L2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1Ipv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1_0_UdpIpPart},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacket1_0_UdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* PARTS of packet UDP */
static TGF_PACKET_PART_STC prvTgfUdpPacket1_1_PartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1_1_L2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1Ipv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1_1_UdpIpPart},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacket1_1_UdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* Length of UDP packet */
#define PRV_TGF_UDP_PACKET_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
    TGF_IPV4_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS + \
    sizeof(prvTgfPacket1PayloadDataArr))

/* Length of UDP packet with CRC */
#define PRV_TGF_UDP_PACKET_CRC_LEN_CNS  \
    (PRV_TGF_UDP_PACKET_LEN_CNS + TGF_CRC_LEN_CNS)

/* UDP packet to send */
static TGF_PACKET_STC prvTgfUdpPacket1_0_Info =
{
    PRV_TGF_UDP_PACKET_LEN_CNS,                                             /* totalLen */
    (sizeof(prvTgfUdpPacket1_0_PartArray)
        / sizeof(prvTgfUdpPacket1_0_PartArray[0])), /* numOfParts */
    prvTgfUdpPacket1_0_PartArray                                                /* partsArray */
};

/* UDP packet to send */
static TGF_PACKET_STC prvTgfUdpPacket1_1_Info =
{
    PRV_TGF_UDP_PACKET_LEN_CNS,                                             /* totalLen */
    (sizeof(prvTgfUdpPacket1_1_PartArray)
        / sizeof(prvTgfUdpPacket1_1_PartArray[0])), /* numOfParts */
    prvTgfUdpPacket1_1_PartArray                                                /* partsArray */
};

/**
* @internal prvTgfPclPumaPortGroupPclCfgSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
static GT_VOID prvTgfPclPumaPortGroupPclCfgSet
(
    IN GT_U32              cfgSetNum,
    IN GT_U32              port0Index,
    IN GT_U32              port1Index
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;

    /* bypass PRV_TGF_DUMMY_PORT_IDX_CNS */
    if (port0Index != PRV_TGF_DUMMY_PORT_IDX_CNS)
    {
        /* Init and configure all needed per port */
        rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[port0Index],
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E    /*nonIpKey*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E   /*ipv4Key*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d, %d",
            prvTgfDevNum, prvTgfPortsArray[port0Index]);
    }

    /* bypass PRV_TGF_DUMMY_PORT_IDX_CNS */
    if (port1Index != PRV_TGF_DUMMY_PORT_IDX_CNS)
    {
        /* Init and configure all needed per port */
        rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[port1Index],
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E    /*nonIpKey*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E   /*ipv4Key*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d, %d",
            prvTgfDevNum, prvTgfPortsArray[port1Index]);
    }

    /* copmarator configuration */
    switch (cfgSetNum)
    {
        default:
        case 0:
            rc = prvTgfPclTcpUdpPortComparatorSet(
                CPSS_PCL_DIRECTION_INGRESS_E,
                CPSS_L4_PROTOCOL_UDP_E,
                PRV_TGF_PCL_COMPARATOR_INDEX0_CNS /*entryIndex*/,
                CPSS_L4_PROTOCOL_PORT_SRC_E,
                CPSS_COMPARE_OPERATOR_GTE,
                (GT_U16)(prvTgfPacket1_0_UdpPart.srcPort - 1) /*value*/);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfPclTcpUdpPortComparatorSet");
            break;
        case 1:
            rc = prvTgfPclTcpUdpPortComparatorSet(
                CPSS_PCL_DIRECTION_INGRESS_E,
                CPSS_L4_PROTOCOL_UDP_E,
                PRV_TGF_PCL_COMPARATOR_INDEX1_CNS /*entryIndex*/,
                CPSS_L4_PROTOCOL_PORT_DST_E,
                CPSS_COMPARE_OPERATOR_LTE,
                (GT_U16)(prvTgfPacket1_1_UdpPart.dstPort + 1) /*value*/);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfPclTcpUdpPortComparatorSet");
            break;
    }

    /* UDB configuration */
    switch (cfgSetNum)
    {
        default:
        case 0:
            rc = prvTgfPclUserDefinedByteSet(
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
                PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E,
                CPSS_PCL_DIRECTION_INGRESS_E,
                20 /*udbIndex*/,
                PRV_TGF_PCL_OFFSET_TCP_UDP_COMPARATOR_E,
                0 /*offset*/);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfPclUserDefinedByteSet");
            rc = prvTgfPclUserDefinedByteSet(
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
                PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E,
                CPSS_PCL_DIRECTION_INGRESS_E,
                21 /*udbIndex*/,
                PRV_TGF_PCL_OFFSET_L2_E,
                PRV_TGF_PCL_UDB_OFFSET_IN_MAC_DA_CNS /*offset*/);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfPclUserDefinedByteSet");
            break;
        case 1:
            rc = prvTgfPclUserDefinedByteSet(
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
                PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E,
                CPSS_PCL_DIRECTION_INGRESS_E,
                20 /*udbIndex*/,
                PRV_TGF_PCL_OFFSET_TCP_UDP_COMPARATOR_E,
                0 /*offset*/);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfPclUserDefinedByteSet");
            rc = prvTgfPclUserDefinedByteSet(
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
                PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E,
                CPSS_PCL_DIRECTION_INGRESS_E,
                21 /*udbIndex*/,
                PRV_TGF_PCL_OFFSET_L2_E,
                PRV_TGF_PCL_UDB_OFFSET_IN_MAC_DA_CNS /*offset*/);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfPclUserDefinedByteSet");
            break;
    }

    /* rule and action */
    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.qos.modifyDscp =
        CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;

    switch (cfgSetNum)
    {
        default:
        case 0:
            /* UDB 20-21 */
            action.qos.dscp = PRV_TGF_PCL_NEW_DSCP0_CNS;
            mask.ruleStdIpv4L4.udb[20]    = 0xFF;
            pattern.ruleStdIpv4L4.udb[20] =
                (1 << PRV_TGF_PCL_COMPARATOR_INDEX0_CNS);
            mask.ruleStdIpv4L4.udb[21]    = 0xFF;
            pattern.ruleStdIpv4L4.udb[21] =
                prvTgfPacket1_0_L2Part.daMac[
                    PRV_TGF_PCL_UDB_OFFSET_IN_MAC_DA_CNS];
            break;
        case 1:
            /* UDB 20-21 */
            action.qos.dscp = PRV_TGF_PCL_NEW_DSCP1_CNS;
            mask.ruleStdIpv4L4.udb[20]    = 0xFF;
            pattern.ruleStdIpv4L4.udb[20] =
                (1 << PRV_TGF_PCL_COMPARATOR_INDEX1_CNS);
            mask.ruleStdIpv4L4.udb[21]    = 0xFF;
            pattern.ruleStdIpv4L4.udb[21] =
                prvTgfPacket1_1_L2Part.daMac[
                    PRV_TGF_PCL_UDB_OFFSET_IN_MAC_DA_CNS];
            break;
    }
    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        PRV_TGF_PCL_RULE_IDX_CNS, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet");
}

/**
* @internal prvTgfPclPumaPortGroupPclCfg function
* @endinternal
*
* @brief   Set test PCL configuration
*/
static GT_VOID prvTgfPclPumaPortGroupPclCfg
(
    IN GT_BOOL             portGroupsBmpEnable,
    IN GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN GT_U32              cfgSetNum,
    IN GT_U32              port0Index,
    IN GT_U32              port1Index
)
{
    /* Clear capturing RxPcktTable */
    tgfTrafficTableRxPcktTblClear();

    usePortGroupsBmp = portGroupsBmpEnable;
    currPortGroupsBmp = portGroupsBmp;

    prvTgfPclPumaPortGroupPclCfgSet(
    cfgSetNum, port0Index, port1Index);

    usePortGroupsBmp = GT_FALSE;
    currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
}

/**
* @internal prvTgfPclPumaPortGroupPclCfgReset function
* @endinternal
*
* @brief   Reset test PCL configuration
*/
static GT_VOID prvTgfPclPumaPortGroupPclCfgReset
(
    IN GT_U32              port0Index,
    IN GT_U32              port1Index
)
{
    GT_STATUS                        rc, rc1 = GT_OK;
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;
    GT_U32                           i;

    usePortGroupsBmp = GT_FALSE;
    currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* AUTODOC: restore default PCL configuration */
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup = GT_FALSE;
    lookupCfg.dualLookup   = GT_FALSE;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    /* bypass PRV_TGF_DUMMY_PORT_IDX_CNS */
    if (port0Index != PRV_TGF_DUMMY_PORT_IDX_CNS)
    {
        /* Init and configure all needed per port */
        rc = prvTgfPclDefPortInitExt2(
            prvTgfPortsArray[port0Index],
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            &lookupCfg);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclDefPortInit");
    }

    /* bypass PRV_TGF_DUMMY_PORT_IDX_CNS */
    if (port1Index != PRV_TGF_DUMMY_PORT_IDX_CNS)
    {
        /* Init and configure all needed per port */
        rc = prvTgfPclDefPortInitExt2(
            prvTgfPortsArray[port1Index],
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            &lookupCfg);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclDefPortInit");
    }

    /* AUTODOC: invalidate UDP comparator entry 3 */
    rc = prvTgfPclTcpUdpPortComparatorSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_L4_PROTOCOL_UDP_E,
        PRV_TGF_PCL_COMPARATOR_INDEX0_CNS /*entryIndex*/,
        CPSS_L4_PROTOCOL_PORT_SRC_E,
        CPSS_COMPARE_OPERATOR_INVALID_E,
        0 /*value*/);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclTcpUdpPortComparatorSet");

    /* AUTODOC: invalidate UDP comparator entry 5 */
    rc = prvTgfPclTcpUdpPortComparatorSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_L4_PROTOCOL_UDP_E,
        PRV_TGF_PCL_COMPARATOR_INDEX1_CNS /*entryIndex*/,
        CPSS_L4_PROTOCOL_PORT_DST_E,
        CPSS_COMPARE_OPERATOR_INVALID_E,
        0 /*value*/);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclTcpUdpPortComparatorSet");

    /* AUTODOC: invalidate User Defined Bytes (UDB) idx [20..22] */
    for (i = 20; (i < 23); i++)
    {
        rc = prvTgfPclUserDefinedByteSet(
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
            PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E,
            CPSS_PCL_DIRECTION_INGRESS_E,
            i /*udbIndex*/,
            PRV_TGF_PCL_OFFSET_INVALID_E,
            0 /*offset*/);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclUserDefinedByteSet");
    }

    /* AUTODOC: invalidate PCL rule and action */
    rc = prvTgfPclRuleValidStatusSet(
        CPSS_PCL_RULE_SIZE_STD_E,
        PRV_TGF_PCL_RULE_IDX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleValidStatusSet");

    /* Clear capturing RxPcktTable */
    tgfTrafficTableRxPcktTblClear();

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
}

/**
* @internal prvTgfPclPumaPortGroupTrafficCheck function
* @endinternal
*
* @brief   Send traffic and check results
*/
static GT_VOID prvTgfPclPumaPortGroupTrafficCheck
(
    IN TGF_PACKET_STC *packetInfoPtr,
    IN GT_U32         sendPortNum,
    IN GT_U32         receivedPortNum,
    IN GT_U32         receivedDscp
)
{
    GT_STATUS                       rc = GT_OK;
    CPSS_INTERFACE_INFO_STC         portInterface;
    TGF_VFD_INFO_STC                vfdArray[1];
    GT_U32                          numTriggersBmp;

    numTriggersBmp = 0;

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = receivedPortNum;

    /* DSCP checking */
    cpssOsMemSet(vfdArray, 0, sizeof(vfdArray));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset = 15; /* 6 + 6 + 2 + 1*/
    vfdArray[0].cycleCount = 1;
    vfdArray[0].patternPtr[0] = (GT_U8)(receivedDscp << 2);

    PRV_UTF_LOG1_MAC("Send Packet to Port [%d]", sendPortNum);
    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevNum, sendPortNum,
        packetInfoPtr,
        1 /*burstCount*/,
        0 /*numVfd*/,
        NULL /*vfdArray*/,
        prvTgfDevNum, receivedPortNum,
        TGF_CAPTURE_MODE_MIRRORING_E,
        1000 /*captureOnTime*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPacketsWithCapture: %d, %d",
        prvTgfDevNum, sendPortNum);

    PRV_UTF_LOG1_MAC(
        "Capture Packet from Port [%d]",
        portInterface.devPort.portNum);
    rc =  tgfTrafficGeneratorPortTxEthTriggerCountersGet(
            &portInterface,
            1 /*numVfd*/,
            vfdArray,
            &numTriggersBmp);
    if (rc != GT_NO_MORE)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d %d",
            prvTgfDevNum, receivedPortNum);
    }
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
        0, numTriggersBmp, "found patterns bitmap");
}

/**
* @internal prvTgfPclPumaPortGroupTestUnaware function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclPumaPortGroupTestUnaware
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */

    /* link up all tested ports */
    prvTgfCommonForceLinkUpOnAllTestedPorts();

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, lookup0 */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IPV4_L4 */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */

    /* AUTODOC: configure UDP comparator entry with: */
    /* AUTODOC:   index 3, ingress direction */
    /* AUTODOC:   source port type, GTE compare operator */
    /* AUTODOC:   value=0x30 */

    /* AUTODOC: configure User Defined Byte (UDB) idx 20: */
    /* AUTODOC:   format INGRESS_STD_IPV4_L4 */
    /* AUTODOC:   packet type IPV4_UDP */
    /* AUTODOC:   offsetType OFFSET_TCP_UDP_COMPARATOR, offset 0 */

    /* AUTODOC: configure User Defined Byte (UDB) idx 21: */
    /* AUTODOC:   format INGRESS_STD_IPV4_L4 */
    /* AUTODOC:   packet type IPV4_UDP */
    /* AUTODOC:   offsetType OFFSET_L2, offset 5 */

    /* AUTODOC: set PCL rule 5 with: */
    /* AUTODOC:   format INGRESS_STD_IPV4_L4, cmd=FORWARD */
    /* AUTODOC:   pattern UDB=[0x20, 0x44] */
    /* AUTODOC:   enable modify DSCP, DSCP=0x2B */
    prvTgfPclPumaPortGroupPclCfg(
        GT_FALSE  /*portGroupsBmpEnable*/,
        CPSS_PORT_GROUP_UNAWARE_MODE_CNS  /*portGroupsBmp*/,
        0 /*cfgSetNum*/,
        PRV_TGF_SEND_PORT0_IDX_CNS,
        PRV_TGF_DUMMY_PORT_IDX_CNS);

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: send UDP packet on port 0 with: */
    /* AUTODOC:   DA=00:01:02:03:04:44, SA=00:00:00:00:55:66 */
    /* AUTODOC:   srcIP=2.2.2.2, dstIP=1.1.1.1 */
    /* AUTODOC:   srcPort=0x31, dstPort=7 */
    prvTgfPclPumaPortGroupTrafficCheck(
        &prvTgfUdpPacket1_0_Info,
        prvTgfPortsArray[PRV_TGF_SEND_PORT0_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        PRV_TGF_PCL_NEW_DSCP0_CNS /*receivedDscp*/);
    /* AUTODOC: verify traffic on port 2: */
    /* AUTODOC:   UDP packet has updated DSCP=0x2B */

    /* AUTODOC: send UDP packet on port 0 with: */
    /* AUTODOC:   DA=00:01:02:03:04:88, SA=00:00:00:00:55:66 */
    /* AUTODOC:   srcIP=3.3.3.3, dstIP=1.1.1.1 */
    /* AUTODOC:   srcPort=0x32, dstPort=8 */
    prvTgfPclPumaPortGroupTrafficCheck(
        &prvTgfUdpPacket1_1_Info,
        prvTgfPortsArray[PRV_TGF_SEND_PORT0_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        ((prvTgfPacket1_1_UdpIpPart.typeOfService >> 2) & 0x3F) /*receivedDscp*/);
    /* AUTODOC: verify traffic on port 2: */
    /* AUTODOC:   UDP packet has the same DSCP */

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclPumaPortGroupPclCfgReset(
        PRV_TGF_SEND_PORT0_IDX_CNS,
        PRV_TGF_SEND_PORT1_IDX_CNS);

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: init PCL Engine for send port 3: */
    /* AUTODOC:   ingress direction, lookup0 */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IPV4_L4 */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */

    /* AUTODOC: configure UDP comparator entry with: */
    /* AUTODOC:   index 5, ingress direction */
    /* AUTODOC:   dst port type, LTE compare operator */
    /* AUTODOC:   value=9 */

    /* AUTODOC: configure User Defined Byte (UDB) idx 20: */
    /* AUTODOC:   format INGRESS_STD_IPV4_L4 */
    /* AUTODOC:   packet type IPV4_UDP */
    /* AUTODOC:   offsetType OFFSET_TCP_UDP_COMPARATOR, offset 0 */

    /* AUTODOC: configure User Defined Byte (UDB) idx 21: */
    /* AUTODOC:   format INGRESS_STD_IPV4_L4 */
    /* AUTODOC:   packet type IPV4_UDP */
    /* AUTODOC:   offsetType OFFSET_L2, offset 5 */

    /* AUTODOC: set PCL rule 5 with: */
    /* AUTODOC:   format INGRESS_STD_IPV4_L4, cmd=FORWARD */
    /* AUTODOC:   pattern UDB=[0x20, 0x88] */
    /* AUTODOC:   enable modify DSCP, DSCP=0x1A */
    prvTgfPclPumaPortGroupPclCfg(
        GT_FALSE  /*portGroupsBmpEnable*/,
        CPSS_PORT_GROUP_UNAWARE_MODE_CNS  /*portGroupsBmp*/,
        1 /*cfgSetNum*/,
        PRV_TGF_SEND_PORT1_IDX_CNS,
        PRV_TGF_DUMMY_PORT_IDX_CNS);

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: send UDP packet on port 3 with: */
    /* AUTODOC:   DA=00:01:02:03:04:88, SA=00:00:00:00:55:66 */
    /* AUTODOC:   srcIP=3.3.3.3, dstIP=1.1.1.1 */
    /* AUTODOC:   srcPort=0x32, dstPort=8 */
    prvTgfPclPumaPortGroupTrafficCheck(
        &prvTgfUdpPacket1_1_Info,
        prvTgfPortsArray[PRV_TGF_SEND_PORT1_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        PRV_TGF_PCL_NEW_DSCP1_CNS /*receivedDscp*/);
    /* AUTODOC: verify traffic on port 2: */
    /* AUTODOC:   UDP packet has updated DSCP=0x1A */

    /* AUTODOC: send UDP packet on port 3 with: */
    /* AUTODOC:   DA=00:01:02:03:04:44, SA=00:00:00:00:55:66 */
    /* AUTODOC:   srcIP=2.2.2.2, dstIP=1.1.1.1 */
    /* AUTODOC:   srcPort=0x31, dstPort=7 */
    prvTgfPclPumaPortGroupTrafficCheck(
        &prvTgfUdpPacket1_0_Info,
        prvTgfPortsArray[PRV_TGF_SEND_PORT1_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        ((prvTgfPacket1_0_UdpIpPart.typeOfService >> 2) & 0x3F) /*receivedDscp*/);
    /* AUTODOC: verify traffic on port 2: */
    /* AUTODOC:   UDP packet has the same DSCP */

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclPumaPortGroupPclCfgReset(
        PRV_TGF_SEND_PORT0_IDX_CNS,
        PRV_TGF_SEND_PORT1_IDX_CNS);
}

/**
* @internal prvTgfPclPumaPortGroupTestAware function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclPumaPortGroupTestAware
(
    GT_VOID
)
{
    GT_U32                          port0GroupId;
    GT_U32                          port1GroupId;

    /* link up all tested ports */
    prvTgfCommonForceLinkUpOnAllTestedPorts();

    port0GroupId =
        PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT0_IDX_CNS]);
    port1GroupId =
        PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT1_IDX_CNS]);
    if (port0GroupId == port1GroupId)
    {
        /*test not applicable */
        return;
    }

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, lookup0 */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IPV4_L4 */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */

    /* AUTODOC: configure UDP comparator entry with: */
    /* AUTODOC:   index 3, ingress direction */
    /* AUTODOC:   source port type, GTE compare operator */
    /* AUTODOC:   value=0x30 */

    /* AUTODOC: configure User Defined Byte (UDB) idx 20: */
    /* AUTODOC:   format INGRESS_STD_IPV4_L4 */
    /* AUTODOC:   packet type IPV4_UDP */
    /* AUTODOC:   offsetType OFFSET_TCP_UDP_COMPARATOR, offset 0 */

    /* AUTODOC: configure User Defined Byte (UDB) idx 21: */
    /* AUTODOC:   format INGRESS_STD_IPV4_L4 */
    /* AUTODOC:   packet type IPV4_UDP */
    /* AUTODOC:   offsetType OFFSET_L2, offset 5 */

    /* AUTODOC: set PCL rule 5 with: */
    /* AUTODOC:   format INGRESS_STD_IPV4_L4, cmd=FORWARD */
    /* AUTODOC:   pattern UDB=[0x20, 0x44] */
    /* AUTODOC:   enable modify DSCP, DSCP=0x2B */
    prvTgfPclPumaPortGroupPclCfg(
        GT_TRUE  /*portGroupsBmpEnable*/,
        (1 << port0GroupId)  /*portGroupsBmp*/,
        0 /*cfgSetNum*/,
        PRV_TGF_SEND_PORT0_IDX_CNS,
        PRV_TGF_DUMMY_PORT_IDX_CNS);

    /* AUTODOC: init PCL Engine for send port 3: */
    /* AUTODOC:   ingress direction, lookup0 */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IPV4_L4 */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */

    /* AUTODOC: configure UDP comparator entry with: */
    /* AUTODOC:   index 5, ingress direction */
    /* AUTODOC:   dst port type, LTE compare operator */
    /* AUTODOC:   value=9 */

    /* AUTODOC: configure User Defined Byte (UDB) idx 20: */
    /* AUTODOC:   format INGRESS_STD_IPV4_L4 */
    /* AUTODOC:   packet type IPV4_UDP */
    /* AUTODOC:   offsetType OFFSET_TCP_UDP_COMPARATOR, offset 0 */

    /* AUTODOC: configure User Defined Byte (UDB) idx 21: */
    /* AUTODOC:   format INGRESS_STD_IPV4_L4 */
    /* AUTODOC:   packet type IPV4_UDP */
    /* AUTODOC:   offsetType OFFSET_L2, offset 5 */

    /* AUTODOC: set PCL rule 5 with: */
    /* AUTODOC:   format INGRESS_STD_IPV4_L4, cmd=FORWARD */
    /* AUTODOC:   pattern UDB=[0x20, 0x88] */
    /* AUTODOC:   enable modify DSCP, DSCP=0x1A */
    prvTgfPclPumaPortGroupPclCfg(
        GT_TRUE  /*portGroupsBmpEnable*/,
        (1 << port1GroupId)  /*portGroupsBmp*/,
        1 /*cfgSetNum*/,
        PRV_TGF_SEND_PORT1_IDX_CNS,
        PRV_TGF_DUMMY_PORT_IDX_CNS);

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: send UDP packet on port 0 with: */
    /* AUTODOC:   DA=00:01:02:03:04:44, SA=00:00:00:00:55:66 */
    /* AUTODOC:   srcIP=2.2.2.2, dstIP=1.1.1.1 */
    /* AUTODOC:   srcPort=0x31, dstPort=7 */
    prvTgfPclPumaPortGroupTrafficCheck(
        &prvTgfUdpPacket1_0_Info,
        prvTgfPortsArray[PRV_TGF_SEND_PORT0_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        PRV_TGF_PCL_NEW_DSCP0_CNS /*receivedDscp*/);
    /* AUTODOC: verify traffic on port 2: */
    /* AUTODOC:   UDP packet has updated DSCP=0x2B */

    /* AUTODOC: send UDP packet on port 0 with: */
    /* AUTODOC:   DA=00:01:02:03:04:88, SA=00:00:00:00:55:66 */
    /* AUTODOC:   srcIP=3.3.3.3, dstIP=1.1.1.1 */
    /* AUTODOC:   srcPort=0x32, dstPort=8 */
    prvTgfPclPumaPortGroupTrafficCheck(
        &prvTgfUdpPacket1_1_Info,
        prvTgfPortsArray[PRV_TGF_SEND_PORT0_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        ((prvTgfPacket1_1_UdpIpPart.typeOfService >> 2) & 0x3F) /*receivedDscp*/);
    /* AUTODOC: verify traffic on port 2: */
    /* AUTODOC:   UDP packet has the same DSCP */

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: send UDP packet on port 3 with: */
    /* AUTODOC:   DA=00:01:02:03:04:88, SA=00:00:00:00:55:66 */
    /* AUTODOC:   srcIP=3.3.3.3, dstIP=1.1.1.1 */
    /* AUTODOC:   srcPort=0x32, dstPort=8 */
    prvTgfPclPumaPortGroupTrafficCheck(
        &prvTgfUdpPacket1_1_Info,
        prvTgfPortsArray[PRV_TGF_SEND_PORT1_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        PRV_TGF_PCL_NEW_DSCP1_CNS /*receivedDscp*/);
    /* AUTODOC: verify traffic on port 2: */
    /* AUTODOC:   UDP packet has updated DSCP=0x1A */

    /* AUTODOC: send UDP packet on port 3 with: */
    /* AUTODOC:   DA=00:01:02:03:04:44, SA=00:00:00:00:55:66 */
    /* AUTODOC:   srcIP=2.2.2.2, dstIP=1.1.1.1 */
    /* AUTODOC:   srcPort=0x31, dstPort=7 */
    prvTgfPclPumaPortGroupTrafficCheck(
        &prvTgfUdpPacket1_0_Info,
        prvTgfPortsArray[PRV_TGF_SEND_PORT1_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        ((prvTgfPacket1_0_UdpIpPart.typeOfService >> 2) & 0x3F) /*receivedDscp*/);
    /* AUTODOC: verify traffic on port 2: */
    /* AUTODOC:   UDP packet has the same DSCP */

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclPumaPortGroupPclCfgReset(
        PRV_TGF_SEND_PORT0_IDX_CNS,
        PRV_TGF_SEND_PORT1_IDX_CNS);

}




