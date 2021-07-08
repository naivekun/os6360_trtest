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
* @file prvTgfPclFullLookupControl.c
*
* @brief Specific PCL features testing
*
* @version   19
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCosGen.h>
#include <pcl/prvTgfPclFullLookupControl.h>

/* max iteration's name length */
#define PRV_TGF_MAX_ITERATION_NAME_LEN_CNS 100

/* number of send traffic iteration */
#define PRV_TGF_MAX_ITER_CNS          1

/* packet's User Priority */
#define PRV_TGF_UP_PACKET_CNS         0

/* port's default User Priority */
#define PRV_TGF_UP_DEFAULT_CNS        7

/* map's new User Priority */
#define PRV_TGF_UP_NEW_CNS            6

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS            5

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     0

/* port number to forward traffic to */
#define PRV_TGF_FDB_PORT_IDX_CNS      1

/* conversion CFG_TBL_INDEX to PCL_ID (for this test only)*/
/* 9 LSBs and set bit9 to 1 - never be 0                  */
#define PRV_CFG_IDX_TO_PCL_ID_MAC(_cgfId) (((_cgfId) & 0x1FF) | 0x200)

/*********************** packet 1 (IPv4 TCP packet) ************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket1L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                 /* saMac */
};
/* VLAN_TAG part of packet1 */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacket1VlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                     /* etherType */
    PRV_TGF_UP_PACKET_CNS, 0, PRV_TGF_VLANID_CNS         /* pri, cfi, VlanId */
};
/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket1EtherTypePart =
    {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacket1Ipv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    6,                  /* protocol */
    0x4C9B,             /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};
/* packet's TCP part */
static TGF_PACKET_TCP_STC prvTgfPacket1TcpPart =
{
    1,                  /* src port */
    2,                  /* dst port */
    1,                  /* sequence number */
    2,                  /* acknowledgment number */
    0,                  /* data offset */
    0,                  /* reserved */
    0x00,               /* flags */
    4096,               /* window */
    0xC1A9,             /* csum */
    0                   /* urgent pointer */
};
/* DATA of packet */
static GT_U8 prvTgfPacket1PayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket1PayloadPart = {
    sizeof(prvTgfPacket1PayloadDataArr),                 /* dataLength */
    prvTgfPacket1PayloadDataArr                          /* dataPtr */
};

/* PARTS of packet IPv4 */
static TGF_PACKET_PART_STC prvTgfPacket0PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1Ipv4Part},
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacket1TcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacket0Info = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + TGF_TCP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacket1PayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacket0PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket0PartArray                                        /* partsArray */
};

/* PARTS of packet IPv4 */
static TGF_PACKET_PART_STC prvTgfPacket1PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacket1VlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1Ipv4Part},
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacket1TcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacket1Info = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + TGF_TCP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacket1PayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacket1PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket1PartArray                                        /* partsArray */
};

/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* stored default VID */
static GT_U16   prvTgfDefVlanId = 0;

/* expected number of sent packets on PRV_TGF_FDB_PORT_NUM_CNS port */
static GT_U8 prvTgfPacketsCountRxFdb = 1;

/* expected number of transmitted packets on PRV_TGF_FDB_PORT_NUM_CNS port */
static GT_U8 prvTgfPacketsCountTxFdb = 1;

/* expected number of triggers */
static GT_U8 prvTgfNumTriggers       = 3;

/* indexes of QoS profiles for using in PCL rules and UP,DSCP patterns */
static GT_U16 prvTgfQosProfileId[8] = {0, 1, 2, 3, 4, 5, 6, 7};
static GT_U16 prvTgfUp1;
static GT_U16 prvTgfUp2;
static GT_U16 prvTgfUp3;

/* indexes of PCL Rules - 255, 511, 383(1023 - fail in PBR), 10, 11, 12 */
static GT_U16 relative_prvTgfPclIndex[6] = {0xFF, 0x1FF, 0x17F, 10, 11, 12};
static GT_U16 prvTgfPclIndex[6];
static GT_U16 prvTgfPclId0;
static GT_U16 prvTgfPclId1;
static GT_U16 prvTgfPclId2;

/* indexes of entries in pclCfgTable
 * 4096 + PortNum = 4096 + prvTgfPortsArray[PRV_TGF_SEND_PORT_NUM_CNS] */
static GT_U16 prvTgfIdxPort          = 4096;  /* 0x1000 */
static GT_U16 prvTgfIdx1             = 4190;  /* 0x105E */
static GT_U16 prvTgfIdx2             = 4191;  /* 0x105F */


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfPclFullLookupControlQosUpSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclFullLookupControlQosUpSet
(
    IN GT_U32 portNum,
    IN GT_U8 qosProfileIdxPort,
    IN GT_U8 qosProfileIdxMap,
    IN GT_U8 upPattern,
    IN GT_BOOL restore
)
{
    GT_STATUS               rc = GT_OK;
    GT_U8                   qosProfileId;
    GT_U32                  qosProfileCount;
    PRV_TGF_COS_PROFILE_STC cosProfile;
    CPSS_QOS_ENTRY_STC      portQosCfg;

    qosProfileCount = sizeof(prvTgfQosProfileId)
                    / sizeof(prvTgfQosProfileId[0]);

    /* set CoS profile entries with different UP and DSCP */
    for (qosProfileId = 0; qosProfileId < qosProfileCount; qosProfileId++)
    {
        /* reset to default */
        cpssOsMemSet(&cosProfile, 0, sizeof(cosProfile));

        if (restore == GT_FALSE)
        {   /* define QoS Profile */
            cosProfile.dropPrecedence = CPSS_DP_GREEN_E;
            cosProfile.userPriority   = /* 1<<1 = 0x20 */
            cosProfile.trafficClass   =
            cosProfile.dscp           = /* 1<<2 = 0x04 */
            cosProfile.exp            = prvTgfQosProfileId[qosProfileId];
        }

        rc = prvTgfCosProfileEntrySet(qosProfileId, &cosProfile);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCosProfileEntrySet 1");
    }

    /* set port's default QoS Profile for not tagged packets or NO_TRUST mode */
    portQosCfg.assignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    if (restore == GT_FALSE)
    {
        portQosCfg.qosProfileId     = qosProfileIdxPort;
        portQosCfg.enableModifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
        portQosCfg.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
    }
    else
    {
        portQosCfg.qosProfileId     = 0;
        portQosCfg.enableModifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        portQosCfg.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    }

    rc = prvTgfCosPortQosConfigSet(portNum, &portQosCfg);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCosPortQosConfigSet");

    if (restore == GT_FALSE)
    {
        /* set port's Qos Trust Mode as TRUST_L2 */
        rc = prvTgfCosPortQosTrustModeSet(portNum, CPSS_QOS_PORT_TRUST_L2_E);
    }
    else
    {
        /* reset port's Qos Trust Mode as TRUST_L2_L3 */
        rc = prvTgfCosPortQosTrustModeSet(portNum, CPSS_QOS_PORT_TRUST_L2_L3_E);
    }
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCosPortQosTrustModeSet");

    if (restore == GT_FALSE)
    {
        /* set QoS Map for tagged packets with specified User Priority field */
        rc = prvTgfCosUpCfiDeiToProfileMapSet(0/*mappingTableIndex*/,PRV_TGF_COS_UP_PROFILE_INDEX_DEFAULT_CNS,
                                              upPattern,
                                              0 /*cfiDeiBit*/,
                                              qosProfileIdxMap);
    }
    else
    {
        /* reset QoS Map for tagged packets with specified User Priority field */
        rc = prvTgfCosUpCfiDeiToProfileMapSet(0/*mappingTableIndex*/,PRV_TGF_COS_UP_PROFILE_INDEX_DEFAULT_CNS,
                                              upPattern,
                                              0 /*cfiDeiBit*/,
                                              0);
    }

    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCosUpCfiDeiToProfileMapSet");

    if (prvCpssDxChPortRemotePortCheck(prvTgfDevNum, portNum))
    {
        /* set QoS Map for tagged packets with default User Priority field as well */
        rc = prvTgfCosPortDefaultUpGet(prvTgfDevNum, portNum, &upPattern);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (restore == GT_FALSE)
        {
            /* set QoS Map for tagged packets with specified User Priority field */
            rc = prvTgfCosUpCfiDeiToProfileMapSet(0/*mappingTableIndex*/,PRV_TGF_COS_UP_PROFILE_INDEX_DEFAULT_CNS,
                                                  upPattern,
                                                  0 /*cfiDeiBit*/,
                                                  qosProfileIdxPort);
        }
        else
        {
            /* reset QoS Map for tagged packets with specified User Priority field */
            rc = prvTgfCosUpCfiDeiToProfileMapSet(0/*mappingTableIndex*/,PRV_TGF_COS_UP_PROFILE_INDEX_DEFAULT_CNS,
                                                  upPattern,
                                                  0 /*cfiDeiBit*/,
                                                  0);
        }

        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCosUpCfiDeiToProfileMapSet");
    }
 
    return rc;
}

/**
* @internal prvTgfDefConfigurationSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfDefConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U8     tagArray[] = {0, 1, 0, 0};

    /* AUTODOC: create VLAN 5 with all tagged port 1, untagged ports 0,2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS,
            prvTgfPortsArray, NULL, tagArray, prvTgfPortsNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWithPortsSet");

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 &prvTgfDefVlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* AUTODOC: set PVID 5 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    return rc;
};

/**
* @internal prvTgfConfigurationRestore function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;
    GT_U8     pclId;
    GT_U32    pclCount;
    CPSS_PCL_DIRECTION_ENT           direction;
    CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT accessMode;
    GT_U32       portNum;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: disable ingress policy per device */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclIngressPolicyEnable");

    /* AUTODOC: disable ingress policy on port 0 */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortIngressPolicyEnable");

    pclCount = sizeof(prvTgfPclIndex) / sizeof(prvTgfPclIndex[0]);

    /* AUTODOC: invalidate PCL rules 0xFF, 0x1FF, 0x17F, 10, 11, 12 */
    for (pclId = 0; pclId < pclCount; pclId++)
    {
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                         prvTgfPclIndex[pclId], GT_FALSE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG2_MAC("\n[TGF]: prvTgfPclRuleValidStatusSet FAILED,"
                " rc = [%d], pclId = %d\n", rc, prvTgfPclIndex[pclId]);
            rc1 = rc;
        }
    }

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgVlanPvidSet");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* AUTODOC: reset QoS configuration */
    rc = prvTgfPclFullLookupControlQosUpSet(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            PRV_TGF_UP_DEFAULT_CNS, /*qosProfileIdxPort*/
            PRV_TGF_UP_NEW_CNS,     /*qosProfileIdxMap*/
            PRV_TGF_UP_PACKET_CNS,   /*upPattern*/
            GT_TRUE);               /* reset */
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclFullLookupControlQosUpSet");

    /* restore accessModes for lookups 0_0 , 0_1 , 1 */
    direction  = CPSS_PCL_DIRECTION_INGRESS_E;
    lookupNum  = CPSS_PCL_LOOKUP_0_E;
    accessMode = PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E;

    portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    /* AUTODOC: restore accessMode for Lookup 0_0 */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
            portNum, direction, lookupNum, 0 /*sublookup*/, accessMode);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortLookupCfgTabAccessModeSet 0_0");

    /* AUTODOC: restore accessMode for Lookup 0_1 */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
            portNum, direction, lookupNum, 1 /*sublookup*/, accessMode);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortLookupCfgTabAccessModeSet 0_1");

    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        lookupNum  = CPSS_PCL_LOOKUP_1_E;

        /* AUTODOC: restore accessMode for Lookup 1 */
        rc = prvTgfPclPortLookupCfgTabAccessModeSet(
                portNum, direction, lookupNum, 0 /*sublookup*/, accessMode);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortLookupCfgTabAccessModeSet 1");
    }


    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    return rc1;
}

/**
* @internal prvTgfPclFullLookupControlTrafficGenerate function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclFullLookupControlTrafficGenerate
(
    IN  TGF_PACKET_STC *packetInfoPtr,
    IN  GT_U32           portNumSend,
    IN  GT_U32           portNumReceive,
    IN  GT_U8           packetsCountRx,
    IN  GT_U8           packetsCountTx,
    IN  GT_U32          packetSize,
    IN  GT_U32          expectedUp,
    IN  GT_U32          expectedDscp,
    OUT GT_U32         *numTriggersPtr
)
{
    GT_STATUS               rc, rc1 = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[3];

    *numTriggersPtr = 0;

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfEthCountersReset");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    /* setup receive portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = portNumReceive;

    /* enable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "tgftrafficgeneratorporttxethcaptureset");

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr,
                             1 /*burstCount*/, 0 /*numVfd*/, NULL);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* AUTODOC: send IPv4 TCP packet port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    /* AUTODOC:   srcPort=1, dstPort=2 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNumSend);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    /* disable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficGeneratorPortTxEthCaptureSet");

    /* check ETH counter of FDB port */
    rc = prvTgfEthCountersCheck(prvTgfDevNum, portNumReceive,
        packetsCountRx, packetsCountTx, packetSize, 1);
    PRV_UTF_VERIFY_RC1(rc, "get another counters values.");

    /* check if there is captured packet with specified UP and DSCP fields */
    vfdArray[0].mode          = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset        = TGF_L2_HEADER_SIZE_CNS
                              + TGF_ETHERTYPE_SIZE_CNS; /*UP*/
    vfdArray[0].cycleCount    = 1;
    vfdArray[0].patternPtr[0] = (GT_U8) (expectedUp << 5) /*0xC0*/;

    vfdArray[1].mode          = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset        = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
                              + TGF_ETHERTYPE_SIZE_CNS + 1; /*DSCP*/
    vfdArray[1].cycleCount    = 1;
    vfdArray[1].patternPtr[0] = (GT_U8) (expectedDscp << 2) /*0x18*/;

    vfdArray[2].mode          = TGF_VFD_MODE_STATIC_E;
    vfdArray[2].modeExtraInfo = 0;
    vfdArray[2].offset        = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
                              + TGF_ETHERTYPE_SIZE_CNS + 0x0A; /*CheckSum*/
    vfdArray[2].cycleCount    = 2;

    /* IPv4 header's checksum MUST be recalculated if any of fields is changed.
       DSCP changing is expected. */
    vfdArray[2].patternPtr[0] = (GT_U8) (prvTgfPacket1Ipv4Part.csum >> 8)    /*0x4C*/;
    vfdArray[2].patternPtr[1] = (GT_U8) ((prvTgfPacket1Ipv4Part.csum & 0xFF) /*0x9B*/
                                         - vfdArray[1].patternPtr[0]);

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portNumReceive);

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(
            &portInterface, 3, vfdArray, numTriggersPtr);
    if (GT_NO_MORE != rc && GT_OK != rc)
        rc1 = rc;

    /* print captured packets from receive port */
    if(prvTgfNumTriggers != 0 && (*numTriggersPtr & 0x04) == 0)
    {
        PRV_UTF_LOG2_MAC("Got other chksum in IPv4 header. Expected checksum = 0x%02x%02x\n",
                vfdArray[2].patternPtr[0], vfdArray[2].patternPtr[1]);
        rc1 = GT_FAIL;
    }

    *numTriggersPtr &= 0x03;

    return rc1;
};

/**
* @internal prvTgfPclFullLookupControlPclCfgTblSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclFullLookupControlPclCfgTblSet
(
    IN GT_U32                         portNum,
    IN GT_U16                        idx1,
    IN GT_U16                        idx2,
    IN GT_U16                        idxPort
)
{
    GT_STATUS                        rc = GT_OK;
    static GT_BOOL                   isDeviceInited = GT_FALSE;
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;
    CPSS_PCL_DIRECTION_ENT           direction;
    CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT accessMode;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT nonIpKey;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ipv4Key;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ipv6Key;

    /* set default values */
    nonIpKey  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    ipv4Key   = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    ipv6Key   = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    /* -------------------------------------------------------------------------
     * 1. Enable PCL
     */

    if (GT_FALSE == isDeviceInited)
    {
        /* init PCL */
        rc = prvTgfPclInit();
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclInit");

        /* do not init next time */
        isDeviceInited = GT_TRUE;
    }

    /* AUTODOC: enables ingress policy per devices */
    rc = prvTgfPclIngressPolicyEnable(GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclIngressPolicyEnable");

    /* AUTODOC: enables ingress policy for port 0 */
    rc = prvTgfPclPortIngressPolicyEnable(portNum, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortIngressPolicyEnable");

    /* -------------------------------------------------------------------------
     * 2. Configure lookup0_0 ingress entry in VLAN area (idx1 = 4190)
     * Configure lookup0_1 ingress entry in VLAN area (idx2 = 4191)
     */

    /* configure accessModes for lookups 0_0 and 0_1 */
    direction  = CPSS_PCL_DIRECTION_INGRESS_E;
    lookupNum  = CPSS_PCL_LOOKUP_0_E;
    accessMode = PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_VLAN_E;

    /* AUTODOC: configure lookup0_0 ingress entry by VLAN area (idx1 = 4190) */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
            portNum, direction, lookupNum, 0 /*sublookup*/, accessMode);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortLookupCfgTabAccessModeSet 0_0");

    /* AUTODOC: configure lookup0_1 ingress entry by VLAN area (idx2 = 4191) */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
            portNum, direction, lookupNum, 1 /*sublookup*/, accessMode);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortLookupCfgTabAccessModeSet 0_1");

    /* set PCL configuration table for VLAN */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    interfaceInfo.type   = CPSS_INTERFACE_VID_E;
    interfaceInfo.vlanId = PRV_TGF_VLANID_CNS;

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.pclId                  = PRV_CFG_IDX_TO_PCL_ID_MAC(idx1);
    lookupCfg.dualLookup             = GT_TRUE;
    lookupCfg.pclIdL01               = PRV_CFG_IDX_TO_PCL_ID_MAC(idx2);
    lookupCfg.groupKeyTypes.nonIpKey = nonIpKey;
    lookupCfg.groupKeyTypes.ipv4Key  = ipv4Key;
    lookupCfg.groupKeyTypes.ipv6Key  = ipv6Key;

    /* AUTODOC: set PCL dual lookup config table for VLAN with: */
    /* AUTODOC:   nonIpKey=INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key=INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key=INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclCfgTblSet(&interfaceInfo, direction, lookupNum, &lookupCfg);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclCfgTblSet VLAN");

    /* -------------------------------------------------------------------------
     * 3. Configure lookup1 ingress entry for port (idxPort = 4096 + port)
     */

    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        /* configure accessMode for lookup 1 */
        direction  = CPSS_PCL_DIRECTION_INGRESS_E;
        lookupNum  = CPSS_PCL_LOOKUP_1_E;
        accessMode = PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E;

        /* AUTODOC: configure lookup0_1 ingress entry by PORT area 0 */
        rc = prvTgfPclPortLookupCfgTabAccessModeSet(
                portNum, direction, lookupNum, 0 /*sublookup*/, accessMode);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortLookupCfgTabAccessModeSet 1");

        /* set PCL configuration table for ingress port */
        cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
        interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
        interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
        interfaceInfo.devPort.portNum = portNum;

        cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
        lookupCfg.enableLookup           = GT_TRUE;
        lookupCfg.pclId                  = PRV_CFG_IDX_TO_PCL_ID_MAC(idxPort);
        lookupCfg.dualLookup             = GT_FALSE;
        lookupCfg.pclIdL01               = 0;
        lookupCfg.groupKeyTypes.nonIpKey = nonIpKey;
        lookupCfg.groupKeyTypes.ipv4Key  = ipv4Key;
        lookupCfg.groupKeyTypes.ipv6Key  = ipv6Key;

        /* AUTODOC: set PCL config table for VLAN with: */
        /* AUTODOC:   nonIpKey=INGRESS_STD_NOT_IP */
        /* AUTODOC:   ipv4Key=INGRESS_STD_IP_L2_QOS */
        /* AUTODOC:   ipv6Key=INGRESS_STD_IP_L2_QOS */
        rc = prvTgfPclCfgTblSet(&interfaceInfo, direction, lookupNum, &lookupCfg);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclCfgTblSet Port");
    }

    return rc;
}

/**
* @internal prvTgfPclFullLookupControlPclRuleSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclFullLookupControlPclRuleSet
(
    IN GT_U32                                 pclRuleIndex,
    IN PRV_TGF_PCL_ACTION_LOOKUP_CONFIG_STC  *lookupCfgPtr,
    IN PRV_TGF_PCL_ACTION_QOS_STC            *qosPtr,
    IN PRV_TGF_PCL_ACTION_VLAN_STC           *vlanPtr
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      patt;
    PRV_TGF_PCL_ACTION_STC           action;

    cpssOsMemSet(&mask,   0, sizeof(mask));
    cpssOsMemSet(&patt,   0, sizeof(patt));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* set mask, pattern and action - only pclId */
    mask.ruleStdIpL2Qos.common.pclId = 0x3FF;
    patt.ruleStdIpL2Qos.common.pclId = (GT_U16)
        PRV_CFG_IDX_TO_PCL_ID_MAC(lookupCfgPtr->ipclConfigIndex);
    action.pktCmd                    = CPSS_PACKET_CMD_FORWARD_E;

    cpssOsMemCpy(&action.lookupConfig, lookupCfgPtr, sizeof(*lookupCfgPtr));
    cpssOsMemCpy(&action.qos,          qosPtr,       sizeof(*qosPtr));
    cpssOsMemCpy(&action.vlan,         vlanPtr,      sizeof(*vlanPtr));

    rc = prvTgfPclRuleSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                          pclRuleIndex, &mask, &patt, &action);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclRuleSet");

    return rc;
}

/**
* @internal prvTgfPclFullLookupControlBaseQosCheck function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfPclFullLookupControlBaseQosCheck
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    numTriggers;

    /* send packet without VLAN Tag and check ETH counters */
    PRV_UTF_LOG0_MAC("=== 1.1 check port default QoS functionality ===\n");

    /* AUTODOC: generate non-tagged traffic to check default Qos */
    rc = prvTgfPclFullLookupControlTrafficGenerate(&prvTgfPacket0Info,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS],
            prvTgfPacketsCountRxFdb, prvTgfPacketsCountTxFdb,
            prvTgfPacket0Info.totalLen + TGF_VLAN_TAG_SIZE_CNS,
            PRV_TGF_UP_DEFAULT_CNS, PRV_TGF_UP_DEFAULT_CNS, &numTriggers);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclFullLookupControlTrafficGenerate: %d", prvTgfDevNum);
    /* AUTODOC: verify traffic: */
    /* AUTODOC:   tagged packet (UP 7, DSCP 7) received on FDB port 1 */

    /* check triggers */
    UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfNumTriggers, numTriggers,
        "tgfTrafficGeneratorPortTxEthTriggerCountersGet:\n"
        "   !!! Captured packet has wrong UP and DSCP fields !!!\n"
        "   !!! Default QoS functionality FAILED             !!!\n"
        "   numTriggers = %d, rc = 0x%02X", numTriggers, rc);

    /* send packet with VLAN Tag and check ETH counters */
    PRV_UTF_LOG0_MAC("=== 1.2 check port map QoS functionality ===\n");

    /* AUTODOC: generate tagged traffic to check port map QoS */
    rc = prvTgfPclFullLookupControlTrafficGenerate(&prvTgfPacket1Info,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS],
            prvTgfPacketsCountRxFdb, prvTgfPacketsCountTxFdb,
            prvTgfPacket1Info.totalLen,
            PRV_TGF_UP_NEW_CNS, PRV_TGF_UP_NEW_CNS, &numTriggers);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclFullLookupControlTrafficGenerate: %d", prvTgfDevNum);
    /* AUTODOC: verify traffic: */
    /* AUTODOC:   tagged packet (UP 6, DSCP 6) received on FDB port 1 */

    /* check triggers */
    UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfNumTriggers, numTriggers,
        "tgfTrafficGeneratorPortTxEthTriggerCountersGet:\n"
        "   !!! Captured packet has wrong UP and DSCP fields !!!\n"
        "   !!! Map QoS functionality FAILED                 !!!\n"
        "   numTriggers = %d, rc = 0x%02X", numTriggers, rc);
}

/**
* @internal prvTgfPclFullLookupControlBaseLookupCheck function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfPclFullLookupControlBaseLookupCheck
(
    GT_VOID
)
{
    GT_STATUS                            rc = GT_OK;
    GT_U32                               numTriggers;
    PRV_TGF_PCL_ACTION_LOOKUP_CONFIG_STC lCfg;
    PRV_TGF_PCL_ACTION_QOS_STC           qos;
    PRV_TGF_PCL_ACTION_VLAN_STC          vlan;
    GT_U32                              expectedUp;
    GT_U32                              expectedDscp;

    cpssOsMemSet(&lCfg, 0, sizeof(lCfg));
    cpssOsMemSet(&qos,  0, sizeof(qos));
    cpssOsMemSet(&vlan, 0, sizeof(vlan));

    /* overwrite next lookupConfig 0_1 by lookupConfig 0_0.
      So lookup 0_1 will take PCL configuration from index prvTgfIdx1.
      But the test does not sets PCL configuration for index prvTgfIdx1.
      And lookup 0_1 will NOT be executed. So result action will be set by
      Lookup 1 - Up/DSCP 3 */
    qos.profileAssignIndex         = GT_TRUE;
    qos.modifyUp                   = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
    qos.modifyDscp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
    qos.profileIndex               = prvTgfUp1;
    lCfg.ipclConfigIndex           = prvTgfIdx1;
    lCfg.pcl0_1OverrideConfigIndex = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
    lCfg.pcl1OverrideConfigIndex   = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;

    /* AUTODOC: overwrite next lookupConfig 0_1 (UP, DSCP) by lookupConfig 0_0 */
    rc = prvTgfPclFullLookupControlPclRuleSet(prvTgfPclId0, &lCfg, &qos, &vlan);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclFullLookupControlPclRuleSet: %d", prvTgfDevNum);

    /* overwrite next lookupConfig 1 by lookupConfig 0_1 */
    qos.profileIndex               = prvTgfUp2;
    lCfg.ipclConfigIndex           = prvTgfIdx2;
    lCfg.pcl0_1OverrideConfigIndex = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        lCfg.pcl1OverrideConfigIndex   = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
    }
    else
    {
        /* Can't override bypassed stage */
        lCfg.pcl1OverrideConfigIndex   = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
    }

    /* AUTODOC: overwrite next lookupConfig 1 (UP, DSCP) by lookupConfig 0_1 */
    rc = prvTgfPclFullLookupControlPclRuleSet(prvTgfPclId1, &lCfg, &qos, &vlan);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclFullLookupControlPclRuleSet: %d", prvTgfDevNum);

    /* it's no next lookupConfig - overwrite nothing */
    qos.profileIndex               = prvTgfUp3;
    lCfg.ipclConfigIndex           = prvTgfIdxPort;
    lCfg.pcl0_1OverrideConfigIndex = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
    lCfg.pcl1OverrideConfigIndex   = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;

    rc = prvTgfPclFullLookupControlPclRuleSet(prvTgfPclId2, &lCfg, &qos, &vlan);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclFullLookupControlPclRuleSet: %d", prvTgfDevNum);

    /* generate traffic and check ETH counters.
       result action will be set by Lookup 1 - Up/DSCP 3  */
    /* AUTODOC: generate tagged traffic to check Lookup 1 */
    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        expectedUp   = prvTgfUp3;
        expectedDscp = prvTgfUp3;
    }
    else
    {
        /* result action will be set by Lookup 0 - Up/DSCP 1  */
        expectedUp   = prvTgfUp1;
        expectedDscp = prvTgfUp1;
    }
    rc = prvTgfPclFullLookupControlTrafficGenerate(&prvTgfPacket1Info,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS],
            prvTgfPacketsCountRxFdb, prvTgfPacketsCountTxFdb,
            prvTgfPacket1Info.totalLen,
            expectedUp, expectedDscp, &numTriggers);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclFullLookupControlTrafficGenerate: %d", prvTgfDevNum);
    /* AUTODOC: verify traffic: */
    /* AUTODOC:   tagged packet (UP 3, DSCP 3) received on FDB port 1 */

    /* check triggers */
    UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfNumTriggers, numTriggers,
        "tgfTrafficGeneratorPortTxEthTriggerCountersGet:\n"
        "   !!! Captured packet has wrong UP and DSCP fields !!!\n"
        "   !!! Lookup_0_0 overwriting FAILED                !!!\n"
        "   numTriggers = %d, rc = 0x%02X", numTriggers, rc);
}

/**
* @internal prvTgfPclFullLookupControlPclRuleUpdate function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfPclFullLookupControlPclRuleUpdate
(
    IN GT_U32                                pclRuleIndex,
    IN GT_U32                                expectedUp,
    IN GT_U32                                expectedDscp,
    IN PRV_TGF_PCL_ACTION_LOOKUP_CONFIG_STC *lCfgPtr,
    IN PRV_TGF_PCL_ACTION_QOS_STC           *qosPtr,
    IN PRV_TGF_PCL_ACTION_VLAN_STC          *vlanPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    numTriggers;

    /* update PCL Rule */
    rc = prvTgfPclFullLookupControlPclRuleSet(pclRuleIndex,
                                              lCfgPtr, qosPtr, vlanPtr);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclFullLookupControlPclRuleSet: %d", prvTgfDevNum);

    /* generate traffic and check ETH counters */
    rc = prvTgfPclFullLookupControlTrafficGenerate(&prvTgfPacket1Info,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS],
            prvTgfPacketsCountRxFdb, prvTgfPacketsCountTxFdb,
            prvTgfPacket1Info.totalLen,
            expectedUp, expectedDscp, &numTriggers);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclFullLookupControlTrafficGenerate: %d", prvTgfDevNum);

    /* check triggers */
    UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfNumTriggers, numTriggers,
        "tgfTrafficGeneratorPortTxEthTriggerCountersGet:\n"
        "   !!! Captured packet has wrong UP and DSCP fields !!!\n"
        "   numTriggers = %d, rc = 0x%02X", numTriggers, rc);
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfPclFullLookupControl function
* @endinternal
*
* @brief   Set test configuration
*         create VLAN
*         create a macEntry for packet 1
*         set QoS configuration
*         Generate traffic
*         0. send packet, check base QoS functionality
*         init PCL
*         set 3 PCL Lookups 0_0, 0_1, 1
*         do (
*         set 3 PCL rules for 3 PCL Lookup - overwrite 0_1 by 0_0
*         1. send traffic
*         update - not overwrite next lookupConfig 0_1 by lookupConfig 0_0
*         2. send traffic
*         update - not overwrite next lookupConfig 1 by lookupConfig 0_1
*         3. send traffic
*         update - change VLAN ID
*         4. send traffic
*         update PCL IDs to repeate the test with new PCL Rules
*         ) while (2 time)
*         Restore configuration
*/
GT_VOID prvTgfPclFullLookupControl
(
    GT_VOID
)
{
    GT_STATUS                            rc = GT_OK;
    GT_U32                               stageIter;
    PRV_TGF_PCL_ACTION_LOOKUP_CONFIG_STC lCfg;
    PRV_TGF_PCL_ACTION_QOS_STC           qos;
    PRV_TGF_PCL_ACTION_VLAN_STC          vlan;
    GT_U32          ii;

    for(ii = 0 ; ii< 6 ; ii++)
    {
        prvTgfPclIndex[ii] = (GT_U16)prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet((ii%3) , relative_prvTgfPclIndex[ii]);
    }

    /* -------------------------------------------------------------------------
     * 1. Set Base configuration
     */

    /* AUTODOC: SETUP CONFIGURATION: */

    /* set default values */
    prvTgfUp1 = prvTgfQosProfileId[1];  prvTgfPclId0 = prvTgfPclIndex[0];
    prvTgfUp2 = prvTgfQosProfileId[2];  prvTgfPclId1 = prvTgfPclIndex[1];
    prvTgfUp3 = prvTgfQosProfileId[3];  prvTgfPclId2 = prvTgfPclIndex[2];

    /* set common configuration */
    rc = prvTgfDefConfigurationSet();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfDefConfigurationSet");

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacket1L2Part.daMac, PRV_TGF_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: set 8 CoS profile entries with: */
    /* AUTODOC:   qosProfileId [0..7] */
    /* AUTODOC:   DP=GREEN, UP=TC=DSCP=EXP=[0..7] */
    /* AUTODOC: enable modify UP\DSCP for port 0 */
    /* AUTODOC: set Qos Trust Mode as TRUST_L2 for port 0 */
    /* AUTODOC: set QoS Map for tagged packets with specified User Priority field */
    /* AUTODOC: map UP Profile 0, UP 0 and CFI/DEI bit 0 to QoS Profile 6 */
    rc = prvTgfPclFullLookupControlQosUpSet(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            PRV_TGF_UP_DEFAULT_CNS, /*qosProfileIdxPort*/
            PRV_TGF_UP_NEW_CNS,     /*qosProfileIdxMap*/
            PRV_TGF_UP_PACKET_CNS,   /*upPattern*/
            GT_FALSE);               /* set */
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclFullLookupControlQosUpSet");

    /* AUTODOC: GENERATE TRAFFIC: */
    /* -------------------------------------------------------------------------
     * 2. Generate traffic
     */

    PRV_UTF_LOG0_MAC("=== 0. check base QoS functionality ===\n");
    prvTgfPclFullLookupControlBaseQosCheck();
    PRV_UTF_LOG0_MAC("=== END OF STAGE 0 ===\n");

    /* init PCL and set 3 PCL Lookups 0_0, 0_1, 1 */
    rc = prvTgfPclFullLookupControlPclCfgTblSet(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            prvTgfIdx1, prvTgfIdx2, prvTgfIdxPort);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclFullLookupControlPclCfgTblSet");

    for(stageIter = 1; stageIter <= 2; stageIter++)
    {
        /* AUTODOC: set 3 PCL rules for 3 PCL Lookup - overwrite 0_1 by 0_0 */
        PRV_UTF_LOG0_MAC("=== 1. check lookup0_0 overwriting ===\n");

        prvTgfPclFullLookupControlBaseLookupCheck();

        /* AUTODOC: update - not overwrite next lookupConfig 0_1 by lookupConfig 0_0 */
        cpssOsMemSet(&lCfg, 0, sizeof(lCfg));
        cpssOsMemSet(&qos,  0, sizeof(qos));
        cpssOsMemSet(&vlan, 0, sizeof(vlan));
        PRV_UTF_LOG0_MAC("=== 2. check lookup0_1 overwriting ===\n");
        qos.profileAssignIndex         = GT_TRUE;
        qos.modifyUp                   = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
        qos.modifyDscp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
        qos.profileIndex               = prvTgfUp1;
        lCfg.ipclConfigIndex           = prvTgfIdx1;
        lCfg.pcl0_1OverrideConfigIndex = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
        lCfg.pcl1OverrideConfigIndex   = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;

        prvTgfPclFullLookupControlPclRuleUpdate(prvTgfPclId0, prvTgfUp2, prvTgfUp2,
                                                &lCfg, &qos, &vlan);
        /* AUTODOC: verify traffic: */
        /* AUTODOC:   tagged packet (UP 5, DSCP 5) received on FDB port 1 */

        if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
        {
            /* AUTODOC: update - not overwrite next lookupConfig 1 by lookupConfig 0_1 */
            PRV_UTF_LOG0_MAC("=== 3. check lookup1 priority ===\n");
            qos.profileIndex               = prvTgfUp2;
            lCfg.ipclConfigIndex           = prvTgfIdx2;

            prvTgfPclFullLookupControlPclRuleUpdate(prvTgfPclId1, prvTgfUp3, prvTgfUp3,
                                                    &lCfg, &qos, &vlan);
            /* AUTODOC: verify traffic: */
            /* AUTODOC:   tagged packet (UP 6, DSCP6 ) received on FDB port 1 */

            /* AUTODOC: update - change VLAN ID */
            PRV_UTF_LOG0_MAC("=== 4. check VLAN changing ===\n");
            cpssOsMemSet(&qos,  0, sizeof(qos));
            cpssOsMemSet(&lCfg, 0, sizeof(lCfg));
            vlan.modifyVlan                = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
            vlan.vlanId                    = 7;
            prvTgfPacketsCountRxFdb        = /* packet disappear from VLAN */
            prvTgfPacketsCountTxFdb        = /* if vlanId is changed */
            prvTgfNumTriggers              = 0;

            prvTgfPclFullLookupControlPclRuleUpdate(prvTgfPclId2, prvTgfUp3, prvTgfUp3,
                                                    &lCfg, &qos, &vlan);
            /* AUTODOC: verify traffic: */
            /* AUTODOC:   tagged packet (UP 6, DSCP 6) received on FDB port 1 */
        }

        /* update PCL IDs to repeat the test with new PCL Rules */
        PRV_UTF_LOG1_MAC("=== END OF STAGE %d ===\n", stageIter);
        prvTgfUp1 = prvTgfQosProfileId[4];  prvTgfPclId0 = prvTgfPclIndex[3];
        prvTgfUp2 = prvTgfQosProfileId[5];  prvTgfPclId1 = prvTgfPclIndex[4];
        prvTgfUp3 = prvTgfQosProfileId[6];  prvTgfPclId2 = prvTgfPclIndex[5];
        prvTgfPacketsCountRxFdb = prvTgfPacketsCountTxFdb = 1;
        prvTgfNumTriggers = 3;
    }

    /* -------------------------------------------------------------------------
     * 3. Restore configuration
     */

    rc = prvTgfConfigurationRestore();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfConfigurationRestore");
}


