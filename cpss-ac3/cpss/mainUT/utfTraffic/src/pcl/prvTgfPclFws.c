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
* @file prvTgfPclFws.c
*
* @brief PCL Full Wire Speed tests
*
* @version   20
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfPortGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCosGen.h>
#include <common/tgfVntGen.h>
#include <port/prvTgfPortFWS.h>
#include <common/tgfCncGen.h>
#include <cnc/prvTgfCncGen.h>
#include <common/tgfTcamGen.h>
#include <cnc/prvTgfCncVlanL2L3.h>
#include <cpssCommon/private/prvCpssMath.h>

GT_U32 prvTgfPclFwsAddSize = 0;
GT_U32 prvTgfFWSPclCfgUnderTraffic = 1;
GT_U32 prvTgfPclCheckMatch = 0;

#define V_TCAM_1 0

/* default vTCAM manager */
#define V_TCAM_MANAGER 1
#define GUARANTEED_NUM_OF_RULES_PER_V_TCAM 1024

/* L2 part of packet FWS */
static TGF_PACKET_L2_STC prvTgfPacketFWSL2Part = {
    {0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0x11},               /* dstMac */
    {0x00, 0x88, 0x99, 0x33, 0x44, 0x11}                /* srcMac */
};


/* DATA of packet */
static GT_U8 prvTgfPayloadDataFWSArr[] = {
    0x12, 0x34, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab,
    0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc,
    0xde, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadFWSPart = {
    sizeof(prvTgfPayloadDataFWSArr),                       /* dataLength */
    prvTgfPayloadDataFWSArr                                /* dataPtr */
};

/* PARTS of packet FWS */
static TGF_PACKET_PART_STC prvTgfPacketFWSPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketFWSL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadFWSPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_FWS_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataFWSArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_FWS_CRC_LEN_CNS  PRV_TGF_PACKET_FWS_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET FWS to send */
static TGF_PACKET_STC prvTgfPacketFWSInfo = {
    PRV_TGF_PACKET_FWS_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketFWSPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketFWSPartArray                                        /* partsArray */
};

/* PCL TCAM group binding */
#define PCL_TCAM_GROUP_CNS  (HARD_WIRE_TCAM_MAC(prvTgfDevNum) ? 1U : 0U)

/***************************    packet for IPv6    *******************/

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {
    TGF_ETHERTYPE_86DD_IPV6_TAG_CNS
};

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketFWSIpv6Part = {
    6,                  /* version */
    0,                  /* trafficClass */
    0,                  /* flowLabel */
    0x02,               /* payloadLen */
    0x3b,               /* nextHeader */
    0x40,               /* hopLimit */
    {0x2222, 0, 0, 0, 0, 0, 0x0000, 0x2222}, /* TGF_IPV6_ADDR srcAddr */
    {0xff00, 0, 0, 0, 0, 0, 0x0000, 0x2222}  /* TGF_IPV6_ADDR dstAddr */
};

/* PARTS of IPv6 packet */
static TGF_PACKET_PART_STC prvTgfIpv6PacketFWSPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketFWSL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketFWSIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadFWSPart}
};

/* LENGTH of IPv6 packet */
#define PRV_TGF_IPV6_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV6_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* IPv6 PACKET1 to send */
static TGF_PACKET_STC prvTgfIpv6PacketFWSInfo = {
    PRV_TGF_PACKET_FWS_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfIpv6PacketFWSPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfIpv6PacketFWSPartArray                                        /* partsArray */
};

/* CNC counters format - Only packets are used in this test */
PRV_TGF_CNC_COUNTER_FORMAT_ENT  prvTgfPclFwsCncFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_2_E;

/**
* @internal prvTgfFWSTest function
* @endinternal
*
* @brief   Configure full wirespeed loop and generate traffic
*
* @param[in] configEnable             - GT_TRUE configure all needed setting and send traffic
*                                      - GT_FALSE do not change configuation but send traffic
*                                       None
*/
GT_VOID prvTgfFWSTest
(
    IN GT_BOOL              configEnable
)
{
    GT_STATUS rc;
    GT_U16    firstVid = 5;
    rc = prvTgfFWSLoopConfigTrafficGenerate(firstVid, &prvTgfPacketFWSInfo, configEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfFWSLoopConfigTrafficGenerate: vid %d", firstVid);
}

/* PCL IDs - use different MSBs for port list testing */
#define PRV_TGF_IPCL_PCL_ID_0_0_CNS  0x000
#define PRV_TGF_IPCL_PCL_ID_0_1_CNS  0x102
#define PRV_TGF_IPCL_PCL_ID_1_CNS    0x204
#define PRV_TGF_EPCL_PCL_ID_CNS      0x310

/* PCL Rule first index */
#define PRV_TGF_START_RULE_CNS   10

/* QoS Profile per ingress ports */
#define PRV_FWS_PORT_QOS_PROFILE_MAC(_portNum) (127 - _portNum)

/* number of PCL Lookups in the test */
GT_U32 prvTgfPclFWSLookups = 2;

/* difference in packets per second between CNC counters rate and port Rx rate*/
GT_U32 prvTgfPortFwsCncRateMaxDiff = 5;

/* enable debug printing to log */
GT_U32 prvTgfPclFwsDebug = 0;

GT_U32 prvTgfPclFwsDebugSet(GT_U32 debug)
{
    GT_U32 oldDebug = prvTgfPclFwsDebug;
    prvTgfPclFwsDebug = debug;
    return oldDebug;
}

GT_U32 prvTgfPclFwsLookupDebug = 0;
GT_U32 prvTgfPclFwsLookupDebugSet(GT_U32 debug)
{
    GT_U32 oldDebug = prvTgfPclFwsLookupDebug;
    prvTgfPclFwsLookupDebug = debug;
    return oldDebug;
}

GT_U32 prvTgfPclFwsCncBlockBmpDebug = 0xFFFFFFFF;

GT_U32 prvTgfPclFwsCncBlockBmpDebugSet(GT_U32 debug)
{
    GT_U32 oldDebug = prvTgfPclFwsCncBlockBmpDebug;
    prvTgfPclFwsCncBlockBmpDebug = debug;
    return oldDebug;
}


/**
* @internal prvTgfPclFwsUseSeparateTcamBlockPerLookupGet function
* @endinternal
*
* @brief   Checks if need use separate TCAM block per PCL lookup.
*          TCAM BW for Aldrin2 and BC3 supports only one lookup per block in FWS.
*          Separate blocks must be used for such devices.
*
* @retval GT_FALSE                - same TCAM block may be used
* @retval GT_TRUE                 - separate TCAM blocks must be used
*
*/
GT_BOOL prvTgfPclFwsUseSeparateTcamBlockPerLookupGet(void)
{
    return (HARD_WIRE_TCAM_MAC(prvTgfDevNum) || PRV_CPSS_SIP_5_25_CHECK_MAC(prvTgfDevNum)) ? GT_TRUE : GT_FALSE;
}

/**
* @internal prvTgfPclFwsUtPclLibRuleIndexGet function
* @endinternal
*
* @brief   Calculate PCL lib rule index for specific lookup and relative index.
*
* @param[in] devNum                   -  device number
* @param[in] testRuleIdx              -  relative rule index
* @param[in] lookupId                 -  lookup number
*
* @retval PCL lib rule index
*
*/
GT_U32 prvTgfPclFwsUtPclLibRuleIndexGet(GT_U8 devNum, GT_U32 testRuleIdx, GT_U32 lookup)
{
    if (HARD_WIRE_TCAM_MAC(devNum))
    {
        return testRuleIdx + appDemoDxChTcamIpclBaseIndexGet(devNum, lookup);
    }
    else if (prvTgfPclFwsUseSeparateTcamBlockPerLookupGet())
    {
        /* use TTI_0 TCAM blocks. Test use 30 Byte rule - delete base to 3 to get UT working */
        return testRuleIdx + (appDemoDxChTcamTtiBaseIndexGet(devNum, 0) / 3);
    }

    return testRuleIdx;
}


/**
* @internal prvTgfPclFWSRuleConfig function
* @endinternal
*
* @brief   Configure PCL rule
*
* @param[in] lookupId                 -  lookup number
* @param[in] pclId                    -  PCL ID
* @param[in] portNum                  -  port number
* @param[in] ruleIndex                -  rule index
* @param[in] tcamType                 -  TCAM type
* @param[in] usePortList              - use port list instead of PCL id and source port
* @param[in] useVirtTcam              - GT_TRUE enables using vTCAMs insted of port configuration
* @param[in] prioBasedVTcam           - GT_TRUE enables creating priority based vTCAMs
* @param[in] ruleSize                 - vTCAM Rule size to be tested (currently only 30 and 80 bytes supported)
*
* @param[out] maskPtr                  - pointer to mask buffer
* @param[out] patternPtr               - pointer to pattern buffer
* @param[out] actionPtr                - pointer to action buffer
*                                       None
*/
static GT_STATUS prvTgfPclFWSRuleConfig
(
    IN GT_U32 lookupId,
    IN GT_U32 pclId,
    IN GT_U32 portNum,
    IN GT_U32 ruleIndex,
    OUT PRV_TGF_PCL_RULE_FORMAT_UNT *maskPtr,
    OUT PRV_TGF_PCL_RULE_FORMAT_UNT *patternPtr,
    OUT PRV_TGF_PCL_ACTION_STC      *actionPtr,
    IN GT_U32                        tcamType,
    IN GT_BOOL                       usePortList,
    IN GT_BOOL                       useVirtTcam,
    IN GT_BOOL                       prioBasedVTcam,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT            ruleSize
)
{
    GT_STATUS                          rc;         /* return code */
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat; /* rule format */
    GT_BOOL                            isPuma = GT_FALSE; /* is Puma configuration */

    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes; /*for vTCAM with priority*/
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       tcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_PCL_ACTION_STC                   dxChAction;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              dxChPattern;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              dxChMask;
    GT_U32                                     vtcamId = V_TCAM_1;

    cpssOsMemSet(maskPtr, 0, sizeof(PRV_TGF_PCL_RULE_FORMAT_UNT));
    cpssOsMemSet(patternPtr, 0, sizeof(PRV_TGF_PCL_RULE_FORMAT_UNT));
    cpssOsMemSet(actionPtr, 0, sizeof(PRV_TGF_PCL_ACTION_STC));

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA3_E))
    {
        isPuma = GT_TRUE;
    }

    if(ruleSize == CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E)
    {
        ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E;
        /* configure classification by PCL ID, source port */

        /* there is no MAC TO ME configuration */
        maskPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.macToMe = 0x1;
        patternPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.macToMe = 0;

        /* the packet is IP */
        maskPtr->ruleUltraIpv6PortVlanQos.isIp = 0x1;
        patternPtr->ruleUltraIpv6PortVlanQos.isIp = 1;

        /* L2 valid , there is NO TT in the test */
        maskPtr->ruleUltraIpv6PortVlanQos.isL2Valid = 0x1;
        patternPtr->ruleUltraIpv6PortVlanQos.isL2Valid = 0x1;

        /* packet is untagged */
        maskPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.isTagged = 0x1;
        patternPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.isTagged = 0;

        /* VID is default of port */
        maskPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.vid = 0xFFF;
        rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
                                     portNum,
                                     &patternPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.vid);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("prvTgfBrgVlanPortVidGet: rc %d, port %d\n", rc, portNum);
            return rc;
        }

        /* ether type of the packet - IPv6 */
        maskPtr->ruleUltraIpv6PortVlanQos.etherType = 0xFFFF;
        patternPtr->ruleUltraIpv6PortVlanQos.etherType = TGF_ETHERTYPE_86DD_IPV6_TAG_CNS;

        if (tcamType == 0)
        {
            /* 0 = The L2 Encapsulation is LLC NON-SNAP.
               1 = The L2 Encapsulation is Ethernet V2 or LLC with SNAP.*/
            maskPtr->ruleUltraIpv6PortVlanQos.l2Encap = 0x1;
            patternPtr->ruleUltraIpv6PortVlanQos.l2Encap = 1;
        }
        else
        {
            /*  0 = The L2 Encapsulation is LLC NON-SNAP.
                1 = The L2 Encapsulation is Ethernet V2.
                2 = reserved.
                3 = The L2 Encapsulation is LLC with SNAP.*/
            maskPtr->ruleUltraIpv6PortVlanQos.l2Encap = 0x3;
            patternPtr->ruleUltraIpv6PortVlanQos.l2Encap = 1;
        }

        /* MAC SA */
        cpssOsMemSet(&(maskPtr->ruleUltraIpv6PortVlanQos.macSa), 0xFF, sizeof(GT_ETHERADDR));
        cpssOsMemCpy(&(patternPtr->ruleUltraIpv6PortVlanQos.macSa), prvTgfPacketFWSL2Part.saMac,sizeof(GT_ETHERADDR));

        /* MAC DA without LSB, LSB is changing in the test */
        cpssOsMemSet(&(maskPtr->ruleUltraIpv6PortVlanQos.macDa), 0xFF, sizeof(GT_ETHERADDR));
        cpssOsMemCpy(&(patternPtr->ruleUltraIpv6PortVlanQos.macDa), prvTgfPacketFWSL2Part.daMac,sizeof(GT_ETHERADDR));
        maskPtr->ruleUltraIpv6PortVlanQos.macDa.arEther[5] = 0;
        patternPtr->ruleUltraIpv6PortVlanQos.macDa.arEther[5] = 0;

        if ((usePortList == GT_TRUE) && (lookupId == 1))
        {
            /* PCL ID - 10 bits */
            maskPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.pclId = 0x300;
            patternPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.pclId = (GT_U16)(pclId & 0x300);

            /* use port list bitmap - all existing bits */
            CPSS_PORTS_BMP_PORT_SET_ALL_MAC(&(maskPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.portListBmp));
            UTF_MASK_PORTS_BMP_WITH_EXISTING_MAC(prvTgfDevNum,maskPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.portListBmp);

            CPSS_PORTS_BMP_PORT_SET_MAC(&(patternPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.portListBmp),portNum);
        }
        else
        {
            /* PCL ID - 10 bits */
            maskPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.pclId = 0x3FF;
            patternPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.pclId = (GT_U16)pclId;

            if (UTF_GET_MAX_PHY_PORTS_NUM_MAC(prvTgfDevNum) > 64)
            {
                /* source port - 7 bits */
                maskPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.sourcePort = 0x7F;
            }
            else
            {
                /* source port - 6 bits */
                maskPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.sourcePort = 0x3F;
            }

            patternPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.sourcePort = portNum &
               maskPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.sourcePort;
        }
    }
    else
    {
        ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
        /* configure classification by PCL ID, source port */

        /* there is no MAC TO ME configuration */
        maskPtr->ruleStdNotIp.common.macToMe = 0x1;
        patternPtr->ruleStdNotIp.common.macToMe = 0;

        /* the packet is NOT IP */
        maskPtr->ruleStdNotIp.common.isIp = 0x1;
        patternPtr->ruleStdNotIp.common.isIp = 0;

        /* packet is NOT ARP */
        maskPtr->ruleStdNotIp.common.isArp = 0x1;
        patternPtr->ruleStdNotIp.common.isArp = 0;

        /* L2 valid , there is NO TT in the test */
        maskPtr->ruleStdNotIp.common.isL2Valid = 0x1;
        patternPtr->ruleStdNotIp.common.isL2Valid = 0x1;

        /* packet is untagged */
        maskPtr->ruleStdNotIp.common.isTagged = 0x1;
        patternPtr->ruleStdNotIp.common.isTagged = 0;

        /* VID is default of port */
        maskPtr->ruleStdNotIp.common.vid = 0xFFF;
        rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
                                     portNum,
                                     &patternPtr->ruleStdNotIp.common.vid);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("prvTgfBrgVlanPortVidGet: rc %d, port %d\n", rc, portNum);
            return rc;
        }

        /* ether type of the packet */
        maskPtr->ruleStdNotIp.etherType = 0xFFFF;
        patternPtr->ruleStdNotIp.etherType = prvTgfPayloadDataFWSArr[1] |
            (prvTgfPayloadDataFWSArr[0] << 8);

        if (tcamType == 0)
        {
            /* 0 = The L2 Encapsulation is LLC NON-SNAP.
               1 = The L2 Encapsulation is Ethernet V2 or LLC with SNAP.*/
            maskPtr->ruleStdNotIp.l2Encap = 0x1;
            patternPtr->ruleStdNotIp.l2Encap = 1;
        }
        else
        {
            /*  0 = The L2 Encapsulation is LLC NON-SNAP.
                1 = The L2 Encapsulation is Ethernet V2.
                2 = reserved.
                3 = The L2 Encapsulation is LLC with SNAP.*/
            maskPtr->ruleStdNotIp.l2Encap = 0x3;
            patternPtr->ruleStdNotIp.l2Encap = 1;
        }

        /* MAC SA */
        cpssOsMemSet(&(maskPtr->ruleStdNotIp.macSa), 0xFF, sizeof(GT_ETHERADDR));
        cpssOsMemCpy(&(patternPtr->ruleStdNotIp.macSa), prvTgfPacketFWSL2Part.saMac,sizeof(GT_ETHERADDR));

        /* MAC DA without LSB, LSB is changing in the test */
        cpssOsMemSet(&(maskPtr->ruleStdNotIp.macDa), 0xFF, sizeof(GT_ETHERADDR));
        cpssOsMemCpy(&(patternPtr->ruleStdNotIp.macDa), prvTgfPacketFWSL2Part.daMac,sizeof(GT_ETHERADDR));
        maskPtr->ruleStdNotIp.macDa.arEther[5] = 0;
        patternPtr->ruleStdNotIp.macDa.arEther[5] = 0;

        if (isPuma == GT_FALSE)
        {
            /* use default QoS profile */
            maskPtr->ruleStdNotIp.common.qosProfile = 0x7f;
            if (lookupId == 0)
            {
                patternPtr->ruleStdNotIp.common.qosProfile = (GT_U8)PRV_FWS_PORT_QOS_PROFILE_MAC(portNum);
            }
            else
                patternPtr->ruleStdNotIp.common.qosProfile = (GT_U8)portNum;

            if (prvTgfPclFwsLookupDebug == 1)
            {
                maskPtr->ruleStdNotIp.common.qosProfile =
                patternPtr->ruleStdNotIp.common.qosProfile = 0;
            }
        }

        if ((usePortList == GT_TRUE) && (lookupId == 1))
        {
            /* PCL ID - 10 bits */
            maskPtr->ruleStdNotIp.common.pclId = 0x300;
            patternPtr->ruleStdNotIp.common.pclId = (GT_U16)(pclId & 0x300);

            /* use port list bitmap - all existing bits */
            CPSS_PORTS_BMP_PORT_SET_ALL_MAC(&(maskPtr->ruleStdNotIp.common.portListBmp));
            UTF_MASK_PORTS_BMP_WITH_EXISTING_MAC(prvTgfDevNum,maskPtr->ruleStdNotIp.common.portListBmp);

            CPSS_PORTS_BMP_PORT_SET_MAC(&(patternPtr->ruleStdNotIp.common.portListBmp),portNum);
        }
        else
        {
            /* PCL ID - 10 bits */
            maskPtr->ruleStdNotIp.common.pclId = 0x3FF;
            patternPtr->ruleStdNotIp.common.pclId = (GT_U16)pclId;

            if (UTF_GET_MAX_PHY_PORTS_NUM_MAC(prvTgfDevNum) > 64)
            {
                /* source port - 7 bits */
                maskPtr->ruleStdNotIp.common.sourcePort = 0x7F;
            }
            else
            {
                /* source port - 6 bits */
                maskPtr->ruleStdNotIp.common.sourcePort = 0x3F;
            }

            patternPtr->ruleStdNotIp.common.sourcePort = portNum &
               maskPtr->ruleStdNotIp.common.sourcePort;
        }
    }


    if (isPuma == GT_FALSE)
    {
        /* action - CNC counter */
        actionPtr->matchCounter.enableMatchCount = GT_TRUE;
        actionPtr->matchCounter.matchCounterIndex = portNum + (100 * lookupId);
    }
    else
    {
       /* Puma version */
        actionPtr->policer.policerEnable =
            PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
        actionPtr->policer.policerId = portNum + (100 * lookupId);
    }

    actionPtr->egressPolicy = GT_FALSE;
    actionPtr->pktCmd       = CPSS_PACKET_CMD_FORWARD_E;

    if (prvTgfPclCheckMatch && (lookupId == 1))
    {
        actionPtr->pktCmd       = CPSS_PACKET_CMD_DROP_HARD_E;
    }

    if ((lookupId == 0) && (isPuma == GT_FALSE))
    {
        /* change QoS profile to be port number */
        actionPtr->qos.profileAssignIndex = GT_TRUE;
        actionPtr->qos.profileIndex = portNum;
    }

    /* set rule */
    if(useVirtTcam == GT_TRUE)
    {
        ruleAttributes.priority = (prioBasedVTcam == GT_TRUE) ? 1000 : 0 ;

        tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
        if(ruleSize == CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E)
        {
            tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E;
        }
        else
        {
            tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E; /*PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;*/
        }
        ruleData.valid                   = GT_TRUE;

        cpssOsMemSet(&dxChAction, 0, sizeof(dxChAction));
        cpssOsMemSet(&dxChPattern, 0, sizeof(dxChPattern));
        cpssOsMemSet(&dxChMask, 0, sizeof(dxChMask));

        rc = prvTgfConvertGenericToDxChRuleAction(actionPtr, &dxChAction);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRuleAction FAILED, rc = [%d]", rc);

            return rc;
        }

        if(ruleSize == CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E)
        {
            rc = prvTgfConvertGenericToDxChIngRuleUltraIpv6PortVlanQos(&(patternPtr->ruleUltraIpv6PortVlanQos),
                                                                  &(dxChPattern.ruleUltraIpv6PortVlanQos));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngRuleUltraIpv6PortVlanQos for patternPtr FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert into device specific Mask */
            rc = prvTgfConvertGenericToDxChIngRuleUltraIpv6PortVlanQos(&(maskPtr->ruleUltraIpv6PortVlanQos),
                                                          &(dxChMask.ruleUltraIpv6PortVlanQos));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngRuleUltraIpv6PortVlanQos for maskPtr FAILED, rc = [%d]", rc);

                return rc;
            }
        }
        else
        {
            rc = prvTgfConvertGenericToDxChIngStdNotIp(&(patternPtr->ruleStdNotIp),
                                                                  &(dxChPattern.ruleStdNotIp));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngStdNotIp for patternPtr FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert into device specific Mask */
            rc = prvTgfConvertGenericToDxChIngStdNotIp(&(maskPtr->ruleStdNotIp),
                                                          &(dxChMask.ruleStdNotIp));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIngStdNotIp for maskPtr FAILED, rc = [%d]", rc);

                return rc;
            }
        }

        ruleData.rule.pcl.actionPtr     = &dxChAction;
        ruleData.rule.pcl.maskPtr       = &dxChMask;
        ruleData.rule.pcl.patternPtr    = &dxChPattern;


        if (prvTgfPclFwsUseSeparateTcamBlockPerLookupGet() && (pclId == PRV_TGF_IPCL_PCL_ID_0_1_CNS))
        {
            /*for bobcat3 need to build separate vtcam for each client*/
            vtcamId = V_TCAM_1 + 1;

        }
        rc = cpssDxChVirtualTcamRuleWrite(V_TCAM_MANAGER, vtcamId, ruleIndex,
                                          &ruleAttributes,
                                          &tcamRuleType, &ruleData);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG3_MAC("cpssDxChVirtualTcamRuleWrite failure \n rc[%d],\nruleIndex = %d, vtcam id: %d\n",
                             rc, ruleIndex ,vtcamId);
            return rc;
        }
    }
    else
    {
        rc = prvTgfPclRuleWithOptionsSet(
            ruleFormat, ruleIndex,
            0,
            maskPtr, patternPtr, actionPtr);
    }

    return rc;
}

/**
* @internal prvTgfPclCncSizeGet function
* @endinternal
*
* @brief   Get CNC counters number info
*
* @param[out] cncBlockCntrsPtr         - pointer to number of CNC counters in a block
* @param[out] cncBlocksPtr             - pointer to number of CNC blocks
*                                       None
*/
GT_VOID prvTgfPclCncSizeGet
(
    OUT GT_U32          *cncBlockCntrsPtr,
    OUT GT_U32          *cncBlocksPtr
)
{
    GT_U32            cncCntrs; /* total number of CNC counters */
    GT_STATUS         rc;       /* return status */

    *cncBlocksPtr = 0;
    *cncBlockCntrsPtr = 0;

    rc = prvUtfCfgTableNumEntriesGet(prvTgfDevNum, PRV_TGF_CFG_TABLE_CNC_BLOCK_E, cncBlockCntrsPtr);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvUtfCfgTableNumEntriesGet: CNC_BLOCK dev %d",
            prvTgfDevNum);

    rc = prvUtfCfgTableNumEntriesGet(prvTgfDevNum, PRV_TGF_CFG_TABLE_CNC_E, &cncCntrs);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvUtfCfgTableNumEntriesGet: CNC dev %d",
            prvTgfDevNum);

    if ((cncCntrs > 0) && ((*cncBlockCntrsPtr) > 0))
    {
        *cncBlocksPtr = cncCntrs / (*cncBlockCntrsPtr);
    }
}

/**
* @internal prvTgfPclCncClientGet function
* @endinternal
*
* @brief   Get CNC client for a block
*
* @param[in] cncBlocks                - number of CNC blocks
* @param[in] blockIdx                 - CNC block's index
*                                       None
*/
PRV_TGF_CNC_CLIENT_ENT prvTgfPclCncClientGet
(
    IN GT_U32          cncBlocks,
    IN GT_U32          blockIdx
)
{
    PRV_TGF_CNC_CLIENT_ENT client; /* CNC client ID */
    GT_BOOL                isPuma = GT_FALSE; /* is Puma device */

    if (cncBlocks > 8)
    {
        if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA3_E))
        {
            isPuma = GT_TRUE;
        }

        client = (PRV_TGF_CNC_CLIENT_ENT)blockIdx;

        if (isPuma == GT_TRUE)
        {
            /* Puma3 device */
            switch (blockIdx)
            {
                case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E:
                case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E:
                case PRV_TGF_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E:
                case PRV_TGF_CNC_CLIENT_EGRESS_PCL_E:
                case PRV_TGF_CNC_CLIENT_ARP_TABLE_ACCESS_E:
                case PRV_TGF_CNC_CLIENT_TUNNEL_START_E:
                case PRV_TGF_CNC_CLIENT_TTI_E:
                    break;
                default: /* bind other blocks with TS */
                    client = PRV_TGF_CNC_CLIENT_TUNNEL_START_E;
                    break;
            }
        }
        else
        {
            switch (blockIdx)
            {
                case PRV_TGF_CNC_CLIENT_L2L3_INGRESS_VLAN_E:
                case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E:
                case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E:
                case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E:
                case PRV_TGF_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E:
                case PRV_TGF_CNC_CLIENT_EGRESS_PCL_E:
                case PRV_TGF_CNC_CLIENT_ARP_TABLE_ACCESS_E:
                case PRV_TGF_CNC_CLIENT_TUNNEL_START_E:
                case PRV_TGF_CNC_CLIENT_TTI_E:
                    break;
                default: /* bind other blocks with TS */
                    client = PRV_TGF_CNC_CLIENT_TUNNEL_START_E;
                    break;
            }
        }
    }
    else if (cncBlocks > 4)
    {
        /* use first clients */
        client = (PRV_TGF_CNC_CLIENT_ENT)blockIdx;
    }
    else
    {
        /* use by PCL only */
        switch (blockIdx)
        {
            case 0: client = PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E; break;
            case 1: client = PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E; break;
            case 2: client = PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E; break;
            default: client = PRV_TGF_CNC_CLIENT_EGRESS_PCL_E; break;
        }
    }

    return client;
}

static GT_U32   pcl1_tcamGroup_save;
static GT_BOOL  pcl1_tcamGroupEnable_save;

/* TCAM floor info saved for restore */
static PRV_TGF_TCAM_BLOCK_INFO_STC saveFloorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_FLOORS_CNS][PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS];

/**
* @internal prvTgfPclFWSConfig function
* @endinternal
*
* @brief   Configure PCL and CNC.
*
* @param[in] useVirtTcam              - GT_TRUE enables using vTCAMs insted of port configuration
* @param[in] prioBasedVTcam           - GT_TRUE enables creating priority based vTCAMs
* @param[in] ruleSize                 - vTCAM Rule size to be tested (currently only 30 and 80 bytes supported)
*
* @param[out] firstStdRulePtr          - pointer to first standard rule index
* @param[out] lastStdRulePtr           - pointer to last standard rule index
*                                       None
*/
GT_VOID prvTgfPclFWSConfig
(
    OUT GT_U32    *firstStdRulePtr,
    OUT GT_U32    *lastStdRulePtr,
    IN  GT_BOOL    useVirtTcam,
    IN  GT_BOOL    prioBasedVTcam,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT            ruleSize
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;
    GT_U8                            devNum = prvTgfDevNum;
    GT_PHYSICAL_PORT_NUM             portNum;              /* port number */
    GT_U32                           ruleIndex = PRV_TGF_START_RULE_CNS;
    GT_U32                           utPclLibRuleIndex = PRV_TGF_START_RULE_CNS; /* UT rule index for PCL lib */
    GT_U32                           vtcam_1_RuleIndex = PRV_TGF_START_RULE_CNS; /* index for the second vtcam client */
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    GT_U32                           indexRangesBmp[4];
    GT_U32                           cncBlockCntrs;
    GT_U32                           cncBlocks;
    GT_U32                           ii;
    PRV_TGF_CNC_CLIENT_ENT           client;
    CPSS_QOS_ENTRY_STC               portQosCfg;
    GT_U32                           tcamType = 0; /* 0 - xcat, lion
                                                      1 - xcat2, lion2, SIP5 */
    GT_BOOL                          usePortList = GT_FALSE;
    GT_U32                           vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC  vTcamInfo;
    GT_U32                           vTcamId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC  vTcamInfo_1; /*bobcat3 needs separate vtcam for each client*/
    GT_U32                           vTcamId_1;
    GT_U32                           floorIndex;
    /* in useVirtTcam case*/
    CPSS_DXCH_PCL_ACTION_STC                   dxChAction;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       tcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              dxMask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              dxPattern;

    if(prvTgfPclFwsUseSeparateTcamBlockPerLookupGet() && (!HARD_WIRE_TCAM_MAC(prvTgfDevNum)))
    {
        /* AUTODOC: Save TCAM floor info for restore */
        for( floorIndex = 0 ; floorIndex < prvTgfTcamFloorsNumGet() ; floorIndex++ )
        {
            rc = prvTgfTcamIndexRangeHitNumAndGroupGet(floorIndex,
                                                  &saveFloorInfoArr[floorIndex][0]);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupGet: %d",
                floorIndex);
        }

        /* AUTODOC: Save PCL TCAM group binding for restore */
        rc = prvTgfTcamClientGroupGet(PRV_TGF_TCAM_IPCL_1_E, &pcl1_tcamGroup_save, &pcl1_tcamGroupEnable_save);
        UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_OK, rc, "prvTgfTcamClientGroupGet: %d, %d, %d",
                PRV_TGF_TCAM_IPCL_1_E, pcl1_tcamGroup_save, pcl1_tcamGroupEnable_save);

        /* AUTODOC: PCL TCAM group binding */
        rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_IPCL_1_E, (PCL_TCAM_GROUP_CNS + 1), GT_TRUE);
        UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
                PRV_TGF_TCAM_IPCL_1_E, (PCL_TCAM_GROUP_CNS + 1), GT_TRUE);
    }

    if (useVirtTcam == GT_TRUE)
    {
        vTcamMngId                     = V_TCAM_MANAGER;
        vTcamId                        = V_TCAM_1;
        vTcamInfo.clientGroup          = PCL_TCAM_GROUP_CNS; /* default for IPCL and EPCL*/
        vTcamInfo.hitNumber            = 0;
        vTcamInfo.ruleSize             = ruleSize;
        vTcamInfo.autoResize           = GT_FALSE;

        if (ruleSize == CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E)
        {
            vTcamInfo.guaranteedNumOfRules = GUARANTEED_NUM_OF_RULES_PER_V_TCAM / 4;
        }
        else
        {
            vTcamInfo.guaranteedNumOfRules = GUARANTEED_NUM_OF_RULES_PER_V_TCAM;
        }

        if( prioBasedVTcam == GT_TRUE )
        {
            vTcamInfo.ruleAdditionMethod =
                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E;
        }
        else
        {
            vTcamInfo.ruleAdditionMethod =
                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;
        }

        /* Create vTCAM manager */
        rc = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
        if (rc == GT_ALREADY_EXIST)
            rc = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerCreate failed\n");


        rc = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,&prvTgfDevNum,1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        /* 1. Create vTCAM(1) */
        rc = cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamCreate 0 failed for vTcamMngId = %d, st = %d\n", vTcamMngId, rc);

        /* 2. Create vTCAM(2) for Bobcat3*/
        if (prvTgfPclFwsUseSeparateTcamBlockPerLookupGet())
        {
            vTcamId_1                        = V_TCAM_1 + 1;
            /* copy data from first vTcam */
            vTcamInfo_1                      = vTcamInfo;
            vTcamInfo_1.clientGroup          = PCL_TCAM_GROUP_CNS + 1 ; /* default for IPCL_1*/

            rc = cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId_1, &vTcamInfo_1);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamCreate 1 failed for vTcamMngId = %d, st = %d\n", vTcamMngId, rc);
        }
    }

    /* get CNC size info */
    prvTgfPclCncSizeGet(&cncBlockCntrs,&cncBlocks);

    /* use first range */
    indexRangesBmp[0] = 0x1;
    indexRangesBmp[1] = 0x0;
    indexRangesBmp[2] = 0x0;
    indexRangesBmp[3] = 0x0;

    *firstStdRulePtr = ruleIndex;

    for (ii = 0; ii < cncBlocks; ii++)
    {
        if (prvTgfPclFwsCncBlockBmpDebug != 0xFFFFFFFF)
        {
            if (!(prvTgfPclFwsCncBlockBmpDebug & (1 << ii)))
            {
                continue;
            }
        }

        /* Get CNC client for a block */
        client = prvTgfPclCncClientGet(cncBlocks, ii);

        /* configure CNC */
        rc = prvTgfCncTestCncBlockConfigure(
            ii,
            client,
            GT_TRUE /*enable*/,
            indexRangesBmp,
            prvTgfPclFwsCncFormat);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfCncTestCncBlockConfigure: block %d, client %d",
                ii,
                client);

    }

    /* enable PCL clients for CNC */
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA3_E | UTF_CPSS_PP_E_ARCH_CNS ))
    {
        rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E,GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet");
    }

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_XCAT2_E | UTF_LION2_E | UTF_CPSS_PP_E_ARCH_CNS))
    {
        tcamType = 1;
        usePortList = GT_TRUE;
        if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum,  UTF_CPSS_PP_E_ARCH_CNS))
        {
            /* SIP5 devices cannot use port list in the test because of
               big number of ports */
            usePortList = GT_FALSE;
        }
    }

    /* PCL Global configurations */
    if (prvTgfPclFWSLookups > 1)
    {
        if (usePortList == GT_TRUE)
        {
            rc = prvTgfPclLookupCfgPortListEnableSet(
                CPSS_PCL_DIRECTION_INGRESS_E,
                CPSS_PCL_LOOKUP_0_E,
                1 /*subLookupNum*/,
                GT_TRUE /*enable*/);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfPclLookupCfgPortListEnableSet");
        }
    }

    rc = prvUtfNextMacPortReset(&portNum, devNum);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG0_MAC("prvUtfNextMacPortReset failure 1 \n");
        return;
    }

    /* For device go over all available MAC physical ports. */
    while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
    {
        if (portNum >= (CPSS_MAX_PORTS_NUM_CNS-1))
        {
            PRV_UTF_LOG1_MAC("port too big  %d\n", portNum);
            return;
        }
        prvTgfPortsArray[0] = portNum;

        if(GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA3_E))
        {
            if (prvUtfIsTrafficManagerUsed() && (portNum >= 128))
            {
                /* skip interlacken channels */
                continue;
            }

            if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum) && (portNum >= 60))
            {
                /* skip cascade and CCFC ports */
                continue;
            }

            /* set QoS Trust for remote ports because they are DSA tagged and treated as VLAN tagged - trust L2 */
            if (prvCpssDxChPortRemotePortCheck(prvTgfDevNum, portNum))
            {
                rc = prvTgfCosPortQosTrustModeSet(portNum, CPSS_QOS_PORT_NO_TRUST_E);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosTrustModeSet: %d, %d", prvTgfDevNum, portNum);
            }

            /* configure QoS Profile */
            portQosCfg.qosProfileId     = PRV_FWS_PORT_QOS_PROFILE_MAC(portNum);
            portQosCfg.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
            portQosCfg.enableModifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
            portQosCfg.assignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
            rc = prvTgfCosPortQosConfigSet(portNum, &portQosCfg);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigSet: %d", portNum);
        }
        else
        {
            /* skip Fabric port configuration */
            if (portNum >= 32)
            {
                continue;
            }
        }


        /* configure rule */
        rc = prvTgfPclFWSRuleConfig(0, PRV_TGF_IPCL_PCL_ID_0_0_CNS, portNum,
                                        ruleIndex, &mask, &pattern, &action, tcamType, usePortList,
                                        useVirtTcam, prioBasedVTcam, ruleSize);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfPclFWSRuleConfig: lookup#0 %d, %d", prvTgfDevNum,
            portNum);
        ruleIndex++;

        cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
        lookupCfg.enableLookup           = GT_TRUE;
        if (prvTgfPclFWSLookups > 1)
        {
            /* configure rule for PLC_1 lookup */
            if ((useVirtTcam == GT_TRUE) && prvTgfPclFwsUseSeparateTcamBlockPerLookupGet())
            {
                /* virtual tcam case, pcl1 need seperate vtcam and seperate index */
                rc = prvTgfPclFWSRuleConfig(1, PRV_TGF_IPCL_PCL_ID_0_1_CNS, portNum,
                                            vtcam_1_RuleIndex, &mask, &pattern, &action, tcamType, usePortList,
                                            useVirtTcam, prioBasedVTcam, ruleSize);
                UTF_VERIFY_EQUAL2_STRING_MAC(
                    GT_OK, rc, "prvTgfPclFWSRuleConfig: lookup#1 %d, %d", portNum, vtcam_1_RuleIndex);
                vtcam_1_RuleIndex++;
            }
            else
            {
                /* without vTcam or same vTcam is used for both lookups  */
                utPclLibRuleIndex = prvTgfPclFwsUtPclLibRuleIndexGet(prvTgfDevNum, ruleIndex, 1);

                rc = prvTgfPclFWSRuleConfig(1, PRV_TGF_IPCL_PCL_ID_0_1_CNS, portNum,
                                            utPclLibRuleIndex, &mask, &pattern, &action, tcamType, usePortList,
                                            useVirtTcam, prioBasedVTcam, ruleSize);
                UTF_VERIFY_EQUAL2_STRING_MAC(
                    GT_OK, rc, "prvTgfPclFWSRuleConfig: lookup#1 %d, %d", portNum, utPclLibRuleIndex);

                ruleIndex++;
            }

            lookupCfg.dualLookup             = GT_TRUE;
        }

        lookupCfg.pclId                  = PRV_TGF_IPCL_PCL_ID_0_0_CNS;
        lookupCfg.pclIdL01               = PRV_TGF_IPCL_PCL_ID_0_1_CNS;
        lookupCfg.groupKeyTypes.nonIpKey =
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
        lookupCfg.groupKeyTypes.ipv4Key  =
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
        if(ruleSize == CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E)
        {
            lookupCfg.groupKeyTypes.ipv6Key  =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E;
        }
        else
        {
            lookupCfg.groupKeyTypes.ipv6Key  =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
        }

        /* enable IPCL 0_0 and IPCL 0_1 lookups */
        rc = prvTgfPclDefPortInitExt2(
            portNum,
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            &lookupCfg);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInitExt2: %d, %d", prvTgfDevNum,
            portNum);

        /* enable Lookup 1 for Puma3 device */
        if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA3_E))
        {
            lookupCfg.pclId = PRV_TGF_IPCL_PCL_ID_0_1_CNS;
            rc = prvTgfPclDefPortInitExt2(
                portNum,
                CPSS_PCL_DIRECTION_INGRESS_E,
                CPSS_PCL_LOOKUP_1_E,
                &lookupCfg);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, rc, "prvTgfPclDefPortInitExt2: %d, %d", prvTgfDevNum,
                portNum);
        }

        if (prvTgfPclFWSLookups > 2)
        {
            lookupCfg.dualLookup             = GT_FALSE;
            lookupCfg.pclId                  = PRV_TGF_IPCL_PCL_ID_1_CNS;

            /* enable IPCL 1 lookups */
            rc = prvTgfPclDefPortInitExt2(
                portNum,
                CPSS_PCL_DIRECTION_INGRESS_E,
                CPSS_PCL_LOOKUP_1_E,
                &lookupCfg);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, rc, "prvTgfPclDefPortInitExt2: %d, %d", prvTgfDevNum,
                portNum);

            if (prvTgfPclFWSLookups > 3)
            {
                cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
                lookupCfg.enableLookup           = GT_TRUE;
                lookupCfg.pclId                  = PRV_TGF_EPCL_PCL_ID_CNS;
                lookupCfg.groupKeyTypes.nonIpKey =
                    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
                lookupCfg.groupKeyTypes.ipv4Key  =
                    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
                lookupCfg.groupKeyTypes.ipv6Key  =
                    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;

                /* enable EPCL lookup */
                rc = prvTgfPclDefPortInitExt2(
                    portNum,
                    CPSS_PCL_DIRECTION_EGRESS_E,
                    CPSS_PCL_LOOKUP_0_E,
                    &lookupCfg);
                UTF_VERIFY_EQUAL2_STRING_MAC(
                    GT_OK, rc, "prvTgfPclDefPortInitExt2: %d, %d", prvTgfDevNum,
                    portNum);
            }
        }
    }

    *lastStdRulePtr = ruleIndex - 1;

    if(useVirtTcam == GT_TRUE)
    {

        ruleData.valid                   = GT_TRUE;
        ruleData.rule.pcl.actionPtr      = &dxChAction;
        ruleData.rule.pcl.maskPtr        = &dxMask;
        ruleData.rule.pcl.patternPtr     = &dxPattern;
        tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
        if (ruleSize == CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E)
        {
            tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E;
        }
        else
        {
            tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
        }


        /* AUTODOC: add last rule with PRV_TGF_PACKET_CMD_DROP_HARD_E action for checking ordering in vTCAM*/
        cpssOsMemSet(&dxMask, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&dxPattern, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&action, 0, sizeof(action));
        /* AUTODOC: action - hard drop */
        action.pktCmd         = PRV_TGF_PACKET_CMD_DROP_HARD_E;
        action.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E;

        prvTgfConvertGenericToDxChRuleAction(&action, &dxChAction);

        /* ruleIndex must be more then lastStdRulePtr */
        ruleIndex                = vTcamInfo.guaranteedNumOfRules - 1;

        if (prioBasedVTcam)
        {
            ruleAttributes.priority  = 1500;
        }

        rc = cpssDxChVirtualTcamRuleWrite(V_TCAM_MANAGER , V_TCAM_1, ruleIndex,
                                          &ruleAttributes,
                                          &tcamRuleType, &ruleData);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "cpssDxChVirtualTcamRuleWrite failure \n rc[%d],\nruleIndex = %d",
                    rc, ruleIndex );
        if (prvTgfPclFwsUseSeparateTcamBlockPerLookupGet() && (prvTgfPclFWSLookups > 1))
        {
            rc = cpssDxChVirtualTcamRuleWrite(V_TCAM_MANAGER , V_TCAM_1 + 1 /*vtcam id*/, ruleIndex,
                                              &ruleAttributes,
                                              &tcamRuleType, &ruleData);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, rc, "cpssDxChVirtualTcamRuleWrite failure \n rc[%d],\nruleIndex = %d",
                        rc, ruleIndex );
        }
    }

    /* restore port configuration according to device family */
    prvTgfPortsArrayByDevTypeSet();

}

/**
* @internal prvTgfPclFWSRestore function
* @endinternal
*
* @brief   Restore PCL and CNC configuration
*
* @param[in] useVirtTcam              - GT_TRUE enables using vTCAMs insted of port configuration
*                                       None
*/
GT_VOID prvTgfPclFWSRestore
(
    IN GT_BOOL useVirtTcam
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;
    GT_U8                            devNum = prvTgfDevNum;
    GT_PHYSICAL_PORT_NUM             portNum;              /* port number */
    GT_U32                           ruleIndex = PRV_TGF_START_RULE_CNS;
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    GT_U32                           indexRangesBmp[4];
    GT_U32                           cncBlockCntrs;
    GT_U32                           cncBlocks;
    GT_U32                           ii;
    GT_U32                           floorIndex;
    PRV_TGF_CNC_CLIENT_ENT           client;
    CPSS_PCL_RULE_SIZE_ENT           ruleSize;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC  vTcamInfo;
    GT_U32                           counterIdx;
    PRV_TGF_CNC_COUNTER_STC          counter;
    CPSS_QOS_ENTRY_STC               portQosCfg;
    GT_U32                           utPclLibRuleIndex = PRV_TGF_START_RULE_CNS; /* UT rule index for PCL lib */
    GT_U32                           vtcam_1_RuleIndex = PRV_TGF_START_RULE_CNS; /* index for the second vtcam client */
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;
    GT_U32                           tcamType = 0; /* 0 - xcat, lion
                                                      1 - xcat2, lion2 */

    ruleSize = CPSS_PCL_RULE_SIZE_STD_E;
    cpssOsMemSet(&counter, 0, sizeof(counter));

    /* get CNC size info */
    prvTgfPclCncSizeGet(&cncBlockCntrs,&cncBlocks);

    /* use first range */
    indexRangesBmp[0] = 0x0;
    indexRangesBmp[1] = 0x0;
    indexRangesBmp[2] = 0x0;
    indexRangesBmp[3] = 0x0;

    for (ii = 0; ii < cncBlocks; ii++)
    {
        /* Get CNC client for a block */
        client = prvTgfPclCncClientGet(cncBlocks, ii);

        /* disable client */
        rc = prvTgfCncBlockClientEnableSet(
            ii, client, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientEnableSet: block %d client %d", ii, client);

        /* reset ranges */
        rc = prvTgfCncBlockClientRangesSet(
            ii, client, indexRangesBmp);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientRangesSet: block %d client %d", ii, client);

        /* clean counters */
        for (counterIdx = 0; counterIdx < cncBlockCntrs; counterIdx++)
        {
            rc = prvTgfCncCounterSet(
                ii, counterIdx,
                prvTgfPclFwsCncFormat, &counter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncCounterSet: block %d idx %d", ii, counterIdx);
        }
    }

    /* disable PCL clients for CNC */
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA3_E | UTF_CPSS_PP_E_ARCH_CNS ))
    {
        rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E,GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet");
    }

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_XCAT2_E | UTF_LION2_E | UTF_CPSS_PP_E_ARCH_CNS))
    {
        tcamType = 1;
    }

    /* PCL Global configurations */
    if (prvTgfPclFWSLookups > 1)
    {
        if (tcamType >= 1)
        {
            rc = prvTgfPclLookupCfgPortListEnableSet(
                CPSS_PCL_DIRECTION_INGRESS_E,
                CPSS_PCL_LOOKUP_0_E,
                1 /*subLookupNum*/,
                GT_FALSE /*enable*/);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfPclLookupCfgPortListEnableSet");
        }
    }


    rc = prvUtfNextMacPortReset(&portNum, devNum);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG0_MAC("prvUtfNextMacPortReset failure 1 \n");
        return;
    }

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup = GT_FALSE;
    lookupCfg.dualLookup = GT_FALSE;

    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    interfaceInfo.type = CPSS_INTERFACE_PORT_E;

    /* For device go over all available MAC physical ports. */
    while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
    {
        if (portNum >= (CPSS_MAX_PORTS_NUM_CNS-1))
        {
            PRV_UTF_LOG1_MAC("port too big  %d\n", portNum);
            return;
        }

        prvTgfPortsArray[0] = portNum;
        if(GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA3_E))
        {
            if (prvUtfIsTrafficManagerUsed() && (portNum >= 128))
            {
                /* skip interlacken channels */
                continue;
            }

            if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum) && (portNum >= 60))
            {
                /* skip cascade and CCFC ports */
                continue;
            }

            /* restore QoS Trust */
            if (prvCpssDxChPortRemotePortCheck(prvTgfDevNum, portNum))
            {
                rc = prvTgfCosPortQosTrustModeSet(portNum, CPSS_QOS_PORT_TRUST_L2_L3_E);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosTrustModeSet: %d, %d", prvTgfDevNum, portNum);
            }

            /* configure QoS Profile */
            portQosCfg.qosProfileId     = 0;
            portQosCfg.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
            portQosCfg.enableModifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
            portQosCfg.assignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
            rc = prvTgfCosPortQosConfigSet(portNum, &portQosCfg);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigSet: %d", portNum);
        }
        else
        {
            /* skip Fabric port configuration */
            if (portNum >= 32)
            {
                continue;
            }
        }

        if(useVirtTcam == GT_TRUE)
        {
            /* clean rule */
            rc = cpssDxChVirtualTcamRuleDelete(V_TCAM_MANAGER, V_TCAM_1, ruleIndex);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete(vTCAM:%d) for ruleId[%d] failed\n",
                                             V_TCAM_1, ruleIndex);
            ruleIndex++;

            if (prvTgfPclFWSLookups > 1)
            {
                if (prvTgfPclFwsUseSeparateTcamBlockPerLookupGet())
                {
                    rc = cpssDxChVirtualTcamRuleDelete(V_TCAM_MANAGER, V_TCAM_1 +1, vtcam_1_RuleIndex);

                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete(vTCAM:%d) for ruleId[%d] failed\n",
                                                 V_TCAM_1 + 1, vtcam_1_RuleIndex);
                    vtcam_1_RuleIndex++;
                }
                else
                {
                    rc = cpssDxChVirtualTcamRuleDelete(V_TCAM_MANAGER, V_TCAM_1, ruleIndex);

                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete(vTCAM:%d) for ruleId[%d] failed\n",
                                                     V_TCAM_1, ruleIndex);
                    ruleIndex++;
                }

            }
        }
        else
        {
            /* clean rule */
            rc = prvTgfPclRuleValidStatusSet(ruleSize, ruleIndex, GT_FALSE);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                         ruleSize, ruleIndex, GT_FALSE);
            ruleIndex++;

            if (prvTgfPclFWSLookups > 1)
            {
                /* clean rule */
                 utPclLibRuleIndex = prvTgfPclFwsUtPclLibRuleIndexGet(prvTgfDevNum, ruleIndex, 1);
                 rc = prvTgfPclRuleValidStatusSet(ruleSize, utPclLibRuleIndex, GT_FALSE);
                 UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                         ruleSize, utPclLibRuleIndex, GT_FALSE);

                 ruleIndex++;
            }
        }

        interfaceInfo.devPort.hwDevNum = prvTgfDevNum;
        interfaceInfo.devPort.portNum = portNum;

        lookupCfg.groupKeyTypes.nonIpKey =
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
        lookupCfg.groupKeyTypes.ipv4Key =
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
        lookupCfg.groupKeyTypes.ipv6Key =
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

        /* disable IPCL 0_0 and IPCL 0_1 lookups */
        rc = prvTgfPclCfgTblSet(
            &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_E, &lookupCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

        rc = prvTgfPclPortIngressPolicyEnable(portNum, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
            prvTgfDevNum, portNum);

        if (prvTgfPclFWSLookups > 2)
        {
            /* disable IPCL 1 lookup */
            rc = prvTgfPclCfgTblSet(
                &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
                CPSS_PCL_LOOKUP_1_E, &lookupCfg);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

            if (prvTgfPclFWSLookups > 3)
            {
                rc = prvTgfPclEgressPclPacketTypesSet(
                    prvTgfDevNum, portNum,
                    PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_FALSE);
                UTF_VERIFY_EQUAL2_STRING_MAC(
                    GT_OK, rc, "prvTgfPclEgressPclPacketTypesSet: %d, %d", prvTgfDevNum,
                    PRV_TGF_PCL_EGRESS_PKT_NON_TS_E);

                lookupCfg.groupKeyTypes.nonIpKey =
                    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
                lookupCfg.groupKeyTypes.ipv4Key  =
                    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
                lookupCfg.groupKeyTypes.ipv6Key  =
                    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;

                /* disable EPCL lookup */
                rc = prvTgfPclCfgTblSet(
                    &interfaceInfo, CPSS_PCL_DIRECTION_EGRESS_E,
                    CPSS_PCL_LOOKUP_0_E, &lookupCfg);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");
            }
        }
    }

    if(useVirtTcam == GT_TRUE)
    {
        /* clean last hard drop rule */
        cpssOsMemSet (&vTcamInfo, 0, sizeof(vTcamInfo));
        rc = cpssDxChVirtualTcamInfoGet(V_TCAM_MANAGER, V_TCAM_1, &vTcamInfo);

        rc = cpssDxChVirtualTcamRuleDelete(V_TCAM_MANAGER, V_TCAM_1, vTcamInfo.guaranteedNumOfRules - 1);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete(vTCAM:%d) for ruleId[%d] failed\n",
                                         V_TCAM_1, ruleIndex);

        rc = cpssDxChVirtualTcamRemove(V_TCAM_MANAGER, V_TCAM_1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRemove failed\n");

        if (prvTgfPclFwsUseSeparateTcamBlockPerLookupGet())
        {
            rc = cpssDxChVirtualTcamInfoGet(V_TCAM_MANAGER, V_TCAM_1 + 1, &vTcamInfo);

            rc = cpssDxChVirtualTcamRuleDelete(V_TCAM_MANAGER, V_TCAM_1 + 1, vTcamInfo.guaranteedNumOfRules - 1);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete(vTCAM:%d) for ruleId[%d] failed\n",
                                             V_TCAM_1 + 1, ruleIndex);

            rc = cpssDxChVirtualTcamRemove(V_TCAM_MANAGER, V_TCAM_1+ 1);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRemove failed\n");

        }

        rc = cpssDxChVirtualTcamManagerDevListRemove(V_TCAM_MANAGER, &prvTgfDevNum, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
                                     V_TCAM_MANAGER);

        rc = cpssDxChVirtualTcamManagerDelete(V_TCAM_MANAGER);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     V_TCAM_MANAGER);
        if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
        {
            /* Restore TCAM Active number of floors */
            rc = cpssDxChTcamActiveFloorsSet(prvTgfDevNum,
                    fineTuningPtr->tableSize.policyTcamRaws/CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS);

            UTF_VERIFY_EQUAL1_STRING_MAC(
                    GT_OK, rc, "cpssDxChTcamActiveFloorsSet failed for device: %d", prvTgfDevNum);
        }
    }

    if(prvTgfPclFwsUseSeparateTcamBlockPerLookupGet() && (!HARD_WIRE_TCAM_MAC(prvTgfDevNum)))
    {
        /* AUTODOC: Restore PCL TCAM group binding */
        rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_IPCL_1_E, pcl1_tcamGroup_save, pcl1_tcamGroupEnable_save);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
                PRV_TGF_TCAM_IPCL_1_E, pcl1_tcamGroup_save, pcl1_tcamGroupEnable_save);

        /* AUTODOC: Restore TCAM floor info */
        for( floorIndex = 0 ; floorIndex < prvTgfTcamFloorsNumGet() ; floorIndex++ )
        {
            rc = prvTgfTcamIndexRangeHitNumAndGroupSet(floorIndex,
                                                  &saveFloorInfoArr[floorIndex][0]);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupSet: %d",
                floorIndex);
        }
    }

    /* restore port configuration according to device family */
    prvTgfPortsArrayByDevTypeSet();

}

/**
* @internal prvTgfCncDump function
* @endinternal
*
* @brief   Dump all non zero CNC counters
*
* @param[in] blockNum                 - CNC block number
*                                       None
*/
GT_VOID prvTgfCncDump
(
    GT_U32 blockNum
)
{
    GT_STATUS                   rc;
    GT_U32                      counterIdx, countNum = 0;
    PRV_TGF_CNC_COUNTER_STC     counter;
    GT_U32                      maxCounters;
    double                      bytes;

    rc = prvUtfCfgTableNumEntriesGet(prvTgfDevNum, PRV_TGF_CFG_TABLE_CNC_BLOCK_E, &maxCounters);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("prvUtfCfgTableNumEntriesGet: error %d\n", rc);
        return;
    }

    cpssOsMemSet(&counter, 0, sizeof(counter));

    PRV_UTF_LOG1_MAC("CNC Block: %d\n", blockNum);
    PRV_UTF_LOG0_MAC("Index | Packet Count | Byte Count\n");

    for (counterIdx = 0; counterIdx < maxCounters; counterIdx++)
    {
        rc = prvTgfCncCounterGet(
            blockNum, counterIdx,
            prvTgfPclFwsCncFormat, &counter);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("prvTgfCncCounterGet: error %d counter %d\n", rc, counterIdx);
            break;
        }

        /* skip zero counter */
        if (counter.byteCount.l[0] == 0 &&
            counter.byteCount.l[1] == 0 &&
            counter.packetCount.l[0] == 0 &&
            counter.packetCount.l[1] == 0)
        {
            continue;
        }
        countNum++;
        bytes = ((double)counter.byteCount.l[1]) * 4294967296.0;
        bytes += counter.byteCount.l[0];
        PRV_UTF_LOG3_MAC("  %4d | %9d     | %f \n", counterIdx, counter.packetCount.l[0], bytes);
    }

    PRV_UTF_LOG1_MAC("\nCounters active: %d\n", countNum);
}


/**
* @internal prvTgfPclFWSCncCheck function
* @endinternal
*
* @brief   Check that CNC is like port's Rx rate.
*
* @param[in] portRxRateArr[CPSS_MAX_PORTS_NUM_CNS] - Rx rate of port in packets per second, may be NULL
*
* @retval GT_OK                    - on OK
* @retval GT_FAIL                  - on failure
*/
static GT_STATUS  prvTgfPclFWSCncCheck
(
  IN   GT_U32  portRxRateArr[CPSS_MAX_PORTS_NUM_CNS]
)
{
    GT_PHYSICAL_PORT_NUM    portNum;   /* ingress port number */
    GT_STATUS               rc = GT_OK; /* return code */
    GT_STATUS               rc1 = GT_OK; /* return code */
    GT_U8                   devNum = prvTgfDevNum; /* device number */
    GT_U32                  diff; /* difference between two values */
    GT_U32                  startTimeStampRxArr[CPSS_MAX_PORTS_NUM_CNS]; /* VNT time stamp start  */
    GT_U32                  endTimeStamp;   /* VNT time stamp end */
    GT_U32                  cncRate;        /* VNT time stamp based CNC packets rate */
    GT_U32                  cncBlockCntrs;   /* number of CNC counters per block */
    GT_U32                  cncBlocks;       /* CNC blocks number */
    GT_U32                  ii;              /* iterator */
    PRV_TGF_CNC_CLIENT_ENT  client;          /* CNC client */
    GT_U32                  counterIdx;      /* counter index */
    PRV_TGF_CNC_COUNTER_STC counter;         /* counter */
    GT_PORT_GROUPS_BMP      portGroupBmp;    /* port group bitmap */
    CPSS_PORT_SPEED_ENT     speed; /*port's speed*/
    GT_U32                  cncRateMaxDiff; /*cnc rate max diff */

    /* get CNC size info */
    prvTgfPclCncSizeGet(&cncBlockCntrs,&cncBlocks);

    if(GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_LION2_E | UTF_CPSS_PP_E_ARCH_CNS | UTF_XCAT3_E))
    {
        /* time stamp measurement is accurate one. */
        prvTgfPortFwsCncRateMaxDiff = 10;
    }
    else
    {
        /* Lion2,Puma3, Xcat3, Bobcat2 and above have emulation of single counter read.
           The CPSS reads all counters from HW for single counter get.
           Time stamp stores time of last read counter from all port counters
           but not required one.
           Therefore the time stamp is not accurate. */

        if(GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, (UTF_BOBCAT3_E | UTF_ALDRIN2_E)))
        {
            prvTgfPortFwsCncRateMaxDiff = (prvTgfDevicePortMode == PRV_TGF_DEVICE_PORT_MODE_XLG_E) ?
                                        450: 350;
        }
        else
        {
            /* BC3 FWS tests are not balanced well and one 10G port may has bigger diff between CNC and MAC counters rate. */
            prvTgfPortFwsCncRateMaxDiff = 3000;
        }
    }

    /* provide time to finish printouts and interrupt handlers
       before start rate measurement */
    cpssOsTimerWkAfter(500);


    for (ii = 0; ii < cncBlocks; ii++)
    {
        /* Get CNC client for a block */
        client = prvTgfPclCncClientGet(cncBlocks, ii);

        if (client != PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E &&
            client != PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E)
        {
            continue;
        }

        for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
        {
            if (portRxRateArr[portNum] == 0)
            {
                /* skip not used port */
                continue;
            }

            /* use PortGroupsBmp API in the test */
            /* set <currPortGroupsBmp> */
            TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(portNum);
            portGroupBmp = currPortGroupsBmp;

            /* restore values after using currPortGroupsBmp*/
            usePortGroupsBmp = GT_FALSE;
            currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;


            counterIdx = portNum;
            if ((client == PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E) ||
                (client == PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E))
            {
                counterIdx += 100;
            }

            rc = prvTgfCncPortGroupCounterGet(portGroupBmp,
                ii, counterIdx,
                prvTgfPclFwsCncFormat, &counter);
            if (rc != GT_OK)
            {
                PRV_UTF_LOG2_MAC("prvTgfCncCounterGet: block %d idx %d\n", ii, counterIdx);
                return rc;
            }

            rc = prvTgfVntPortLastReadTimeStampGet(devNum, portNum, &startTimeStampRxArr[portNum]);
            if (rc != GT_OK)
            {
                PRV_UTF_LOG2_MAC("prvTgfVntPortLastReadTimeStampGet failure RX: dev %d port %d\n", devNum, portNum);
                return rc;
            }
        }

        /* sleep */
        cpssOsTimerWkAfter(1000);


        for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
        {
            if (portRxRateArr[portNum] == 0)
            {
                /* skip not used port */
                continue;
            }

            cncRateMaxDiff = prvTgfPortFwsCncRateMaxDiff;

            /* use PortGroupsBmp API in the test */
            /* set <currPortGroupsBmp> */
            TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(portNum);
            portGroupBmp = currPortGroupsBmp;

            /* restore values after using currPortGroupsBmp*/
            usePortGroupsBmp = GT_FALSE;
            currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

            counterIdx = portNum;
            if ((client == PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E) ||
                (client == PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E))
            {
                counterIdx += 100;
            }

            rc = prvTgfCncPortGroupCounterGet(portGroupBmp,
                ii, counterIdx,
                prvTgfPclFwsCncFormat, &counter);
            if (rc != GT_OK)
            {
                PRV_UTF_LOG2_MAC("prvTgfCncCounterGet: block %d idx %d\n", ii, counterIdx);
                return rc;
            }

            rc = prvTgfVntPortLastReadTimeStampGet(devNum, portNum, &endTimeStamp);
            if (rc != GT_OK)
            {
                PRV_UTF_LOG2_MAC("prvTgfVntPortLastReadTimeStampGet failure RX: dev %d port %d\n", devNum, portNum);
                return rc;
            }

            /* calculate actual rate */
            cncRate = prvTgfCommonVntTimeStampsRateGet(devNum, startTimeStampRxArr[portNum], endTimeStamp, counter.packetCount.l[0]);
             /* for 100G ports, the difference can be bigger */
            rc = cpssDxChPortSpeedGet(devNum,portNum,&speed);
            if (rc != GT_OK)
            {
                PRV_UTF_LOG2_MAC(" cpssDxChPortSpeedGet failure RX: dev %d port %d\n", devNum, portNum);
                return rc;
            }

            if (CPSS_PORT_SPEED_100G_E == speed)
            {
                /* 1 % is good enough for rate comparison */
                cncRateMaxDiff = (GT_U32)(cncRate*0.01);
            }

            if (cncRate > portRxRateArr[portNum])
            {
                diff = cncRate - portRxRateArr[portNum];
            }
            else
            {
                diff = portRxRateArr[portNum] - cncRate;
            }

            if (diff > cncRateMaxDiff)
            {
                PRV_UTF_LOG4_MAC("prvTgfFWSPclCncCheck failure block %d port %d cnc rate %d port rate %d\n", ii, portNum, cncRate, portRxRateArr[portNum]);
                rc1 = GT_FAIL;
            }

            if (prvTgfPclFwsDebug)
            {
                PRV_UTF_LOG4_MAC("prvTgfFWSPclCncCheck: block %d port %d cnc rate %d port rate %d\n", ii, portNum, cncRate, portRxRateArr[portNum]);
            }
        }
    }

    return rc1;
}

/**
* @internal prvTgfPclFWSPortAndCncCntrsCheckOrReset function
* @endinternal
*
* @brief   Check that CNC counter is same as Rx one.
*         Or reset both CNC and Rx counters
* @param[in] portRxRateArr[CPSS_MAX_PORTS_NUM_CNS] - Rx rate of port in packets per second, may be NULL
* @param[in] check                    - if to check/reset Rx counters
*
* @retval GT_OK                    - on OK
* @retval GT_FAIL                  - on failure
*/
static GT_STATUS  prvTgfPclFWSPortAndCncCntrsCheckOrReset
(
  IN   GT_U32   portRxRateArr[CPSS_MAX_PORTS_NUM_CNS],
  IN   GT_BOOL  check
)
{
    GT_PHYSICAL_PORT_NUM    portNum;   /* ingress port number */
    GT_STATUS               rc = GT_OK; /* return code */
    GT_STATUS               rc1 = GT_OK; /* return code */
    GT_U8                   devNum = prvTgfDevNum; /* device number */
    GT_U32                  cncBlockCntrs;   /* number of CNC counters per block */
    GT_U32                  cncBlocks;       /* CNC blocks number */
    GT_U32                  ii;              /* iterator */
    PRV_TGF_CNC_CLIENT_ENT  client;          /* CNC client */
    GT_U32                  counterIdx;      /* counter index */
    PRV_TGF_CNC_COUNTER_STC cncCounter;         /* counter */
    GT_PORT_GROUPS_BMP      portGroupBmp;    /* port group bitmap */
    GT_U64                  portCntrValue;   /* value of port counters */
    GT_U64                  portDropCntrValue;   /* value of port drop event counters */
    GT_U64                 *portCntrArr;        /* value of counter per port */
    GT_U32                  isPuma = GT_FALSE; /* is Puma system */

    portCntrArr = cpssOsMalloc(CPSS_MAX_PORTS_NUM_CNS * sizeof(portCntrArr[0]));
    if (portCntrArr == NULL)
    {
        PRV_UTF_LOG2_MAC("portCntrArr: Not enough memory %d * %d\n", CPSS_MAX_PORTS_NUM_CNS, sizeof(portCntrArr[0]));
        return GT_OUT_OF_CPU_MEM;
    }
    cpssOsMemSet(portCntrArr, 0xFE, CPSS_MAX_PORTS_NUM_CNS * sizeof(portCntrArr[0]));

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA3_E))
    {
        isPuma = GT_TRUE;
    }

    /* get CNC size info */
    prvTgfPclCncSizeGet(&cncBlockCntrs,&cncBlocks);

    for (ii = 0; ii < cncBlocks; ii++)
    {
        /* Get CNC client for a block */
        client = prvTgfPclCncClientGet(cncBlocks, ii);

        if (isPuma == GT_TRUE)
        {
            if (client != PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E &&
                client != PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E)
            {
                continue;
            }
        }
        else
        {
            if (client != PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E &&
                client != PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E)
            {
                continue;
            }
        }

        for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
        {
            if (portRxRateArr[portNum] == 0)
            {
                /* skip not used port */
                continue;
            }

            /* use PortGroupsBmp API in the test */
            /* set <currPortGroupsBmp> */
            TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(portNum);
            portGroupBmp = currPortGroupsBmp;

            /* restore values after using currPortGroupsBmp*/
            usePortGroupsBmp = GT_FALSE;
            currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

            counterIdx = portNum;
            if (client == PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E ||
                client == PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E)
            {
                counterIdx += 100;
            }

            rc = prvTgfCncPortGroupCounterGet(portGroupBmp,
                ii, counterIdx,
                prvTgfPclFwsCncFormat, &cncCounter);
            if (rc != GT_OK)
            {
                PRV_UTF_LOG2_MAC("prvTgfCncCounterGet: block %d idx %d\n", ii, counterIdx);
                cpssOsFree(portCntrArr);
                return rc;
            }

            rc = prvTgfPortMacCounterGet(devNum, portNum,
                                          CPSS_GOOD_UC_PKTS_RCV_E,
                                          &portCntrValue);
            if (rc != GT_OK)
            {
                PRV_UTF_LOG2_MAC("prvTgfPortMacCounterGet: dev %d port %d\n", devNum, portNum);
                cpssOsFree(portCntrArr);
                return rc;
            }

            rc = prvTgfPortMacCounterGet(devNum, portNum,
                                          CPSS_DROP_EVENTS_E,
                                          &portDropCntrValue);
            if (rc != GT_OK)
            {
                PRV_UTF_LOG2_MAC("prvTgfPortMacCounterGet: dev %d port %d\n", devNum, portNum);
                cpssOsFree(portCntrArr);
                return rc;
            }

            if (check)
            {
                if (client == PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E)
                {
                    /* port counter value is valid only for first read
                       because it get reset on read.
                       store value of port counter after first read. */
                    portCntrArr[portNum] = prvCpssMathSub64(portCntrValue, portDropCntrValue);
                }

                if (prvTgfPclFwsDebug)
                {
                    PRV_UTF_LOG6_MAC("DBG block %d port %d cnc %d:%d port %d:%d\n", ii, portNum,
                                        cncCounter.packetCount.l[1],
                                        cncCounter.packetCount.l[0],
                                        portCntrArr[portNum].l[1],
                                        portCntrArr[portNum].l[0]);
                }

                if (portCntrArr[portNum].l[0] != cncCounter.packetCount.l[0])
                {
                    PRV_UTF_LOG6_MAC("prvTgfFWSPclCncCheck failure block %d port %d cnc %d:%d port %d:%d\n", ii, portNum,
                                        cncCounter.packetCount.l[1],
                                        cncCounter.packetCount.l[0],
                                        portCntrArr[portNum].l[1],
                                        portCntrArr[portNum].l[0]);
                    rc1 = GT_FAIL;
                }
            }
        }
    }

    cpssOsFree(portCntrArr);
    return rc1;
}

/* rule move step in rules */
GT_U32 prvTgfPclMoveStep = 1;

/* number of move iterations */
GT_U32 prvTgfPclMoveIterNum = 100;

/**
* @internal prvTgfPclFWSMoveRules function
* @endinternal
*
* @brief   Move PCL rules.
*
* @param[in] firstStdRule             - first standard rule index
* @param[in] lastStdRule              - last standard rule index
* @param[in] useVirtTcam              - GT_TRUE enables using vTCAMs insted of port configuration
* @param[in] prioBasedVTcam           - GT_TRUE enables creating priority based vTCAMs
*                                       None
*/
static GT_STATUS prvTgfPclFWSMoveRules
(
    IN GT_U32    firstStdRule,
    IN GT_U32    lastStdRule,
    IN GT_BOOL   useVirtTcam,
    IN GT_BOOL   prioBasedVTcam
)
{
    GT_STATUS        rc = GT_OK; /* return status*/
    GT_U32           ii;         /* rules iterator */
    GT_U32           ruleCount;  /* number of rules to move */
    GT_U32           srcRuleIdx; /* source rule index */
    GT_U32           dstRuleIdx; /* destination rule index */
    GT_U32           kk;         /* moves iterator */
    GT_U32           moveStep;   /* rule index step to move rules */
    GT_U32           vtcamId_1 = V_TCAM_1 + 1;         /* vtcam id for bobcat3, client pcl1*/
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;  /* rule attributes for prioBasedVTcam case */
    GT_U32           prioRuleIndex; /* rule index for priority changes */
    CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT equalRulePosition;


    ruleCount = lastStdRule - firstStdRule + 1;
    moveStep  = prvTgfPclMoveStep;
    if (prvTgfPclFwsUseSeparateTcamBlockPerLookupGet() && (useVirtTcam == GT_FALSE))
    {
        /* BC3 use even rules for lookup#0 and odd for lookup#1.
           Use step to be even for such devices. */
        /* when using vtcam on bobcat3, there are 2 rule indexes, first for lookup#0, and second for lookup#1*/
        moveStep  = 2;
    }

    for (kk = 0; kk < prvTgfPclMoveIterNum; kk++)
    {
        /* change equalRulePosition each two iterations */
        if ((kk & 2) == 0)
        {
            /* POSITION_FIRST iterations  */
            equalRulePosition = CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E;
        }
        else
        {
            /* POSITION_LAST iterations */
            equalRulePosition = CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E;
        }
        for (ii = 0; ii < ruleCount; ii++)
        {
            if ((kk & 1) == 0)
            {
                /* even iteration - move rules UP */
                srcRuleIdx = lastStdRule - ii;

                /* each odd rule is for lookup#1. Align index for BC3 devices. */
                if (prvTgfPclFwsUseSeparateTcamBlockPerLookupGet() && (srcRuleIdx & 1) && (useVirtTcam == GT_FALSE))
                {
                    srcRuleIdx = prvTgfPclFwsUtPclLibRuleIndexGet(prvTgfDevNum, srcRuleIdx, 1);
                }

                dstRuleIdx = srcRuleIdx + moveStep;
                ruleAttributes.priority = 1100;
                prioRuleIndex = srcRuleIdx;
            }
            else
            {
                /* odd iteration - move rules down */
                srcRuleIdx = firstStdRule + moveStep + ii;

                /* each odd rule is for lookup#1. Align index for BC3 devices. */
                if (prvTgfPclFwsUseSeparateTcamBlockPerLookupGet() && (srcRuleIdx & 1) && (useVirtTcam == GT_FALSE))
                {
                    srcRuleIdx = prvTgfPclFwsUtPclLibRuleIndexGet(prvTgfDevNum, srcRuleIdx, 1);
                }

                dstRuleIdx = srcRuleIdx - moveStep;
                ruleAttributes.priority = 900;
                prioRuleIndex = firstStdRule + ii;
            }

            if(useVirtTcam == GT_TRUE)
            {
                if(prioBasedVTcam == GT_TRUE)
                {
                    rc = cpssDxChVirtualTcamRulePriorityUpdate(
                                V_TCAM_MANAGER, V_TCAM_1,
                                prioRuleIndex, ruleAttributes.priority,
                                equalRulePosition);
                    if (rc != GT_OK)
                    {
                        PRV_UTF_LOG3_MAC("cpssDxChVirtualTcamRulePriorityUpdate failed: rc %d ruleId %d prio %d\n",
                                         rc, prioRuleIndex, ruleAttributes.priority);
                        return rc;
                    }

                    if (prvTgfPclFwsUseSeparateTcamBlockPerLookupGet() && (prvTgfPclFWSLookups > 1))
                    {
                        /*bobcat3 needs separate vtcam for each client, so move the vtcam for the anoter client*/
                         rc = cpssDxChVirtualTcamRulePriorityUpdate(
                                V_TCAM_MANAGER, vtcamId_1,
                                prioRuleIndex, ruleAttributes.priority,
                                equalRulePosition);
                         if (rc != GT_OK)
                         {
                             PRV_UTF_LOG3_MAC("cpssDxChVirtualTcamRulePriorityUpdate failed: rc %d ruleId %d prio %d\n",
                                              rc, prioRuleIndex, ruleAttributes.priority);
                             return rc;
                         }

                    }
                }
                else
                {
                    rc = cpssDxChVirtualTcamRuleMove(V_TCAM_MANAGER, V_TCAM_1,
                                                     srcRuleIdx, dstRuleIdx);
                    if (rc != GT_OK)
                    {
                        PRV_UTF_LOG4_MAC("cpssDxChVirtualTcamRuleMove failed: rc %d src %d dst %d, vtcam id %d\n",
                                         rc, srcRuleIdx, dstRuleIdx,V_TCAM_1);
                        return rc;
                    }
                     if (prvTgfPclFwsUseSeparateTcamBlockPerLookupGet() && (prvTgfPclFWSLookups > 1))
                    {
                         /*bobcat3 needs separate vtcam for each client, so move the vtcam for the anoter client*/
                         rc = cpssDxChVirtualTcamRuleMove(V_TCAM_MANAGER, vtcamId_1,
                                                     srcRuleIdx, dstRuleIdx);
                         if (rc != GT_OK)
                         {
                             PRV_UTF_LOG4_MAC("cpssDxChVirtualTcamRuleMove failed: rc %d src %d dst %d, vtcam id %d\n",
                                         rc, srcRuleIdx, dstRuleIdx, vtcamId_1);
                             return rc;
                         }

                    }
                }
            }
            else
            {
                rc = prvTgfPclRuleCopy(CPSS_PCL_RULE_SIZE_STD_E,srcRuleIdx,dstRuleIdx);
                if (rc != GT_OK)
                {
                    PRV_UTF_LOG3_MAC("prvTgfPclRuleCopy: rc %d src %d dst %d", rc, srcRuleIdx, dstRuleIdx);
                    return rc;
                }

                rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,srcRuleIdx,GT_FALSE);
                if (rc != GT_OK)
                {
                    PRV_UTF_LOG2_MAC("prvTgfPclRuleValidStatusSet: rc %d src %d", rc, srcRuleIdx);
                    return rc;
                }
            }
        }

        /* change move step after odd iteration end */
        if ((kk & 1) == 1)
        {
            if (prvTgfPclFwsUseSeparateTcamBlockPerLookupGet() && (useVirtTcam == GT_FALSE))
            {
                /* BC3 use even rules for lookup#0 and odd for lookup#1.
                   Use step to be even for such devices. */
                moveStep  += 2;
            }
            else
            {
                moveStep++;
            }

            if ((kk & 7) == 7)
            {
                utfPrintKeepAlive();
            }
        }
    }
    return GT_OK;
}
/**
* @internal prvTgfPclFWSCncTest function
* @endinternal
*
* @brief   Full wire speed PCL and CNC test:
*         1. Generate FWS traffic
*         2. Check FWS traffic
*         3. Configure PCL and CNC
*         4. Check CNC counters rate to be like Rx Port rate
*/
GT_VOID prvTgfPclFWSCncTest
(
    GT_VOID
)
{
    GT_STATUS rc;               /* return code */
    GT_U16    firstVid = 5;     /* first VLAN id for the test */
    GT_U32    firstStdRule;     /* first rule index */
    GT_U32    lastStdRule;      /* last rule index */
    GT_U32    packetSize = PRV_TGF_PACKET_FWS_CRC_LEN_CNS; /* packet size */
    GT_U32    portRxRateArr[CPSS_MAX_PORTS_NUM_CNS];       /* rate of packets per port */

    cpssOsMemSet(portRxRateArr, 0, sizeof(portRxRateArr));

    /* configure and generate FWS */
    rc = prvTgfFWSLoopConfigTrafficGenerate(firstVid, &prvTgfPacketFWSInfo, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfFWSLoopConfigTrafficGenerate: vid %d", firstVid);

    /* check FWS traffic */
    rc = prvTgfFWSTrafficCheck(packetSize, GT_TRUE, portRxRateArr);
    prvTgfFWSTrafficRatePrint(packetSize,portRxRateArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficCheck: size %d", packetSize);

    utfPrintKeepAlive();
    /* configure PCL and CNC */
    prvTgfPclFWSConfig(&firstStdRule, &lastStdRule, GT_FALSE, GT_FALSE, CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E);

    /* the function above (prvTgfPclFWSConfig) load the TCAM and decreases 100G ports rate in device like Bobcat3 and above.
       use generate FWS again to return the expected rate*/
    if (prvTgfPclFwsUseSeparateTcamBlockPerLookupGet())
    {
        /* and generate FWS */
        rc = prvTgfFWSLoopConfigTrafficGenerate(firstVid, &prvTgfPacketFWSInfo, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfFWSLoopConfigTrafficGenerate: vid %d", firstVid);
    }

    /* check FWS traffic */
    rc = prvTgfFWSTrafficCheck(packetSize, GT_TRUE, portRxRateArr);
    prvTgfFWSTrafficRatePrint(packetSize,portRxRateArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficCheck after PCL: size %d", packetSize);

    utfPrintKeepAlive();

    /* check CNC rate */
    rc = prvTgfPclFWSCncCheck(portRxRateArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfFWSPclCncCheck: size %d", packetSize);

}

/**
* @internal prvTgfPclFWSCncRestore function
* @endinternal
*
* @brief   Restore configuration after Full wire speed PCL and CNC test
*
* @param[in] useVirtTcam              - GT_TRUE enables using vTCAMs insted of port configuration
*                                       None
*/
GT_VOID prvTgfPclFWSCncRestore
(
     IN GT_BOOL useVirtTcam
)
{
    /* stop traffic and restore port/vlan config */
    prvTgfFWSRestore();

    /* restore PCL and CNC config */
    prvTgfPclFWSRestore(useVirtTcam);
}

/**
* @internal prvTgfPclFWSMoveTest function
* @endinternal
*
* @brief   Full wire speed PCL Move test:
*         1. Generate FWS traffic
*         2. Check FWS traffic
*         3. Configure PCL and CNC
*         4. Check CNC counters rate to be like Rx Port rate
*         5. Stop traffic, reset MAC and CNC counters
*         6. Start traffic
*         7. Move PCEs up and down several times
*         8. Stop traffic and compare CNC and MAC Rx counters.
* @param[in] useVirtTcam              - GT_TRUE enables using vTCAMs insted of port configuration
* @param[in] prioBasedVTcam           - GT_TRUE enables creating priority based vTCAMs
* @param[in] ruleSize                 - Rule size to be tested (currently only 30 and 80 bytes supported)
*                                       None
*/
GT_VOID prvTgfPclFWSMoveTest
(
    IN  GT_BOOL    useVirtTcam,
    IN  GT_BOOL    prioBasedVTcam,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT            ruleSize
)
{
    GT_STATUS rc;               /* return code */
    GT_U16    firstVid = 5;     /* first VLAN id for the test */
    GT_U32    firstStdRule;     /* first rule index */
    GT_U32    lastStdRule;      /* last rule index */
    GT_U32    packetSize = PRV_TGF_PACKET_FWS_CRC_LEN_CNS  + prvTgfPclFwsAddSize; /* packet size */
    GT_U32    portRxRateArr[CPSS_MAX_PORTS_NUM_CNS];       /* rate of packets per port */
    TGF_PACKET_STC                 *packetInfoPtr;

    cpssOsMemSet(portRxRateArr, 0, sizeof(portRxRateArr));

    prvTgfPacketFWSInfo.totalLen += prvTgfPclFwsAddSize;

    /* configure and generate FWS */
    if (ruleSize == CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E)
    {
        packetInfoPtr = &prvTgfIpv6PacketFWSInfo;
    }
    else
    {
        packetInfoPtr = &prvTgfPacketFWSInfo;
    }
    rc = prvTgfFWSLoopConfigTrafficGenerate(firstVid, packetInfoPtr, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfFWSLoopConfigTrafficGenerate: vid %d", firstVid);
    if (!prvTgfFWSPclCfgUnderTraffic)
    {
        /* stop traffic */
        rc = prvTgfFWSTrafficStop();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficStop");
    }
    /* configure PCL and CNC */
    prvTgfPclFWSConfig(&firstStdRule, &lastStdRule, useVirtTcam,
                       prioBasedVTcam, ruleSize);

    if (!prvTgfFWSPclCfgUnderTraffic || prvTgfPclFwsUseSeparateTcamBlockPerLookupGet())
    {
        /* generate FWS */
        rc = prvTgfFWSLoopConfigTrafficGenerate(firstVid, packetInfoPtr, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfFWSLoopConfigTrafficGenerate: vid %d", firstVid);
    }

    utfPrintKeepAlive();

    /* check FWS traffic */
    rc = prvTgfFWSTrafficCheck(packetSize, GT_TRUE, portRxRateArr);
    prvTgfFWSTrafficRatePrint(packetSize,portRxRateArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficCheck after PCL: size %d", packetSize);

    utfPrintKeepAlive();

    /* check CNC rate */
    rc = prvTgfPclFWSCncCheck(portRxRateArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfFWSPclCncCheck: size %d", packetSize);

    utfPrintKeepAlive();

    /* stop traffic */
    rc = prvTgfFWSTrafficStop();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficStop");

    if (prvTgfFWSPclCfgUnderTraffic)
    {
        /* reset Port and CNC counters */
        rc = prvTgfPclFWSPortAndCncCntrsCheckOrReset(portRxRateArr, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclFWSPortAndCncCntrsReset");

        /* generate FWS */
        rc = prvTgfFWSLoopConfigTrafficGenerate(firstVid, packetInfoPtr, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfFWSLoopConfigTrafficGenerate: vid %d", firstVid);
    }

    /* move rules under traffic */
    if (prvTgfPclFwsDebug)
    {
        PRV_UTF_LOG2_MAC("prvTgfPclFWSMoveTest: move rules first %d last %d\n", firstStdRule, lastStdRule);
    }
    rc = prvTgfPclFWSMoveRules(firstStdRule, lastStdRule, useVirtTcam, prioBasedVTcam);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclFWSMoveRules: first %d last %d", firstStdRule, lastStdRule);

    if (!prvTgfFWSPclCfgUnderTraffic)
    {
        /* reset Port and CNC counters */
        rc = prvTgfPclFWSPortAndCncCntrsCheckOrReset(portRxRateArr, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclFWSPortAndCncCntrsReset");

        /* generate FWS */
        rc = prvTgfFWSLoopConfigTrafficGenerate(firstVid, packetInfoPtr, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfFWSLoopConfigTrafficGenerate: vid %d", firstVid);
    }

    /* stop traffic */
    rc = prvTgfFWSTrafficStop();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficStop");

    /* Compare Port and CNC counters */
    rc = prvTgfPclFWSPortAndCncCntrsCheckOrReset(portRxRateArr, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclFWSPortAndCncCntrsReset");
}


