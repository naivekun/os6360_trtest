/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfPclPortListCross.c
*
* DESCRIPTION:
*       Specific PCL features testing
*
* FILE REVISION NUMBER:
*       $Revision: 6 $
*
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCosGen.h>
#include <pcl/prvTgfPclMiscellanous.h>

/* There are two tests: Ingress PCL test and Egress PCL test              */

/* The Ingress PCL test:                                                  */
/* All prvTgfPortsNum ports add to VLANS 10,11,12,13 as taggged           */
/* prvTgfPortsNum rules matching source port using port list field        */
/* Each rule updates the VID                                              */
/* Each of prvTgfPortsNum ports configured for Ingress PCL                */
/* The test sends packet to each of 4 ports                               */
/* The packet modified VID hecked using egress port capture               */

/* The Egress PCL test:                                                   */
/* All prvTgfPortsNum ports add to VLANS 10,11,12,13 as taggged           */
/* prvTgfPortsNum rules matching target port using port list field        */
/* Each rule updates the VID                                              */
/* Each of prvTgfPortsNum ports configured for Egress PCL                 */
/* The test sends packet to exit from each of 4 ports                     */
/* The packet modified VID hecked using egress port capture               */

/* source VLAN Id */
#define PRV_TGF_SRC_VLANID_CNS             2

/* base of VLAN Ids */
#define PRV_TGF_VLANID_BASE_CNS            10
/* base of rule indexes */
#define PRV_TGF_RULE_INDEX_BASE_CNS         3

/* use extended or standard rule for testing */
#define PRV_TGF_RULE_EXT_USED_CNS 1

#if (PRV_TGF_RULE_EXT_USED_CNS)

#define PRV_TGF_RULE_SIZE_CNS CPSS_PCL_RULE_SIZE_EXT_E

#define PRV_TGF_RULE_IPCL_FORMAT_NOT_IP_CNS \
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E

#define PRV_TGF_RULE_IPCL_FORMAT_IPV4_CNS \
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E

#define PRV_TGF_RULE_IPCL_FORMAT_IPV6_CNS \
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E

#define PRV_TGF_RULE_IPCL_UNION_MEMBER_CNS ruleExtNotIpv6

#define PRV_TGF_RULE_EPCL_FORMAT_NOT_IP_CNS \
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E

#define PRV_TGF_RULE_EPCL_FORMAT_IPV4_CNS \
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E

#define PRV_TGF_RULE_EPCL_FORMAT_IPV6_CNS \
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E

#define PRV_TGF_RULE_EPCL_UNION_MEMBER_CNS ruleEgrExtNotIpv6

#else /*(PRV_TGF_RULE_EXT_USED_CNS)*/

#define PRV_TGF_RULE_SIZE_CNS CPSS_PCL_RULE_SIZE_STD_E

#define PRV_TGF_RULE_IPCL_FORMAT_NOT_IP_CNS \
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E

#define PRV_TGF_RULE_IPCL_FORMAT_IPV4_CNS \
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E

#define PRV_TGF_RULE_IPCL_FORMAT_IPV6_CNS \
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E

#define PRV_TGF_RULE_IPCL_UNION_MEMBER_CNS ruleStdNotIp

#define PRV_TGF_RULE_EPCL_FORMAT_NOT_IP_CNS \
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E

#define PRV_TGF_RULE_EPCL_FORMAT_IPV4_CNS \
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E

#define PRV_TGF_RULE_EPCL_FORMAT_IPV6_CNS \
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E

#define PRV_TGF_RULE_EPCL_UNION_MEMBER_CNS ruleEgrStdNotIp


#endif /*PRV_TGF_RULE_EXT_USED_CNS*/


/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* stored default VID */
static GT_U16   prvTgfDefVlanId = 0;

/* store port mapping */
static GT_BOOL portListPortMappingEnable = GT_TRUE;
static GT_U32 portListPortMappingGroup   = 0;
static GT_U32 portListPortMappingOffset  = 0x1F;


/*********************** packet (Eth Other packet) ************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC packet1TgfL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x55}                 /* saMac */
};

static TGF_PACKET_VLAN_TAG_STC packet1VlanTag =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
    0, /* tag UP */
    0, /* tag CFI*/
    PRV_TGF_SRC_VLANID_CNS /* VID */
};

/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket1EtherTypePart =
    {0x3333};

/* DATA of packet */
static GT_U8 packet1TgfPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC packet1TgfPayloadPart = {
    sizeof(packet1TgfPayloadDataArr),                 /* dataLength */
    packet1TgfPayloadDataArr                          /* dataPtr */
};

/* PARTS of packet1 */
static TGF_PACKET_PART_STC packet1TgfPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &packet1TgfL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E , &packet1VlanTag},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &packet1TgfPayloadPart}
};

/* PACKET1 to send */
static TGF_PACKET_STC packet1TgfInfo = {
    (TGF_L2_HEADER_SIZE_CNS
     + TGF_VLAN_TAG_SIZE_CNS
     + TGF_ETHERTYPE_SIZE_CNS
     + sizeof(packet1TgfPayloadDataArr)),                        /* totalLen */
    (sizeof(packet1TgfPartArray) / sizeof(TGF_PACKET_PART_STC)), /* numOfParts */
    packet1TgfPartArray                                          /* partsArray */
};

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfPclPortListCrossCfgVlanSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclPortListCrossCfgVlanSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    i;

    static GT_U8     tagArray[256] = {0};
    if (tagArray[0] == 0)
    {
        cpssOsMemSet(tagArray, 0xFF, sizeof(tagArray));
    }

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(
        prvTgfDevNum, prvTgfPortsArray[0], &prvTgfDefVlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* AUTODOC: create VLAN 2 with all tagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(
        PRV_TGF_SRC_VLANID_CNS /*VID*/,
        prvTgfPortsArray, NULL, tagArray, prvTgfPortsNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWithPortsSet");

    /* range of VIDs assigned by PCL */
    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        /* AUTODOC: create VLANs [10..13] with all tagged ports */
        rc = prvTgfBrgDefVlanEntryWithPortsSet(
            (GT_U16)(i + PRV_TGF_VLANID_BASE_CNS),
            prvTgfPortsArray, NULL, tagArray, prvTgfPortsNum);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWithPortsSet");
    }

    /* AUTODOC: set PVID 2 for all ports */
    rc = prvTgfBrgVlanPvidSet(PRV_TGF_SRC_VLANID_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPvidSet");

    return GT_OK;
};

/**
* @internal prvTgfPclPortListCrossCfgVlanRestore function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclPortListCrossCfgVlanRestore
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;
    GT_U32    i;

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgVlanPvidSet");

    /* invalidate vlan entry (and reset vlan entry) */
    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        rc = prvTgfBrgDefVlanEntryInvalidate(
            (GT_U16)(i + PRV_TGF_VLANID_BASE_CNS));
        PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");
    }

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SRC_VLANID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");


    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */

    return GT_OK;
}

/**
* @internal prvTgfPclPortListCrossPortMappingSaveForRestore function
* @endinternal
*
* @brief   Save Port mapping for Port List mode for restore
*
* @param[in] direction                - PCL dircetion: IPCL or EPCL
*                                       None
*/
static GT_STATUS prvTgfPclPortListCrossPortMappingSaveForRestore
(
    CPSS_PCL_DIRECTION_ENT direction
)
{
    GT_STATUS rc = GT_OK;

    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Save Port Mapping data for restore */
        rc = prvTgfPclPortListPortMappingGet(prvTgfDevNum,
                                             direction,
                                             prvTgfPortsArray[0],
                                             &portListPortMappingEnable,
                                             &portListPortMappingGroup,
                                             &portListPortMappingOffset);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG6_MAC("\n[TGF]: prvTgfPclPortListPortMappingGet FAILED,"
                         " rc = [%d], %d, %d, %d, %d, %d\n", rc,
                         direction,
                         prvTgfPortsArray[0],
                         portListPortMappingEnable,
                         portListPortMappingGroup,
                         portListPortMappingOffset);;
    }

    return rc;
}

/**
* @internal prvTgfPclPortListCrossPortMappingSet function
* @endinternal
*
* @brief   Set Port mapping for Port List mode
*
* @param[in] direction                - PCL dircetion: IPCL or EPCL
* @param[in] enable                   - port enabling for Port List
* @param[in] group                    - port  in Port List
* @param[in] offset                   - port  in Port List
*                                       None
*/
static GT_STATUS prvTgfPclPortListCrossPortMappingSet
(
    CPSS_PCL_DIRECTION_ENT direction,
    GT_BOOL                enable,
    GT_U32                 group,
    GT_U32                 offset
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    ii;
    GT_U32    bitOffset;

    for (ii = 0; (ii < prvTgfPortsNum); ii++)
    {
        if( 0x1F < offset)
        {
            bitOffset = ii;
        }
        else
        {
            bitOffset = offset;
        }

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        {
            rc = prvTgfPclPortListPortMappingSet(direction,
                                                 prvTgfPortsArray[ii],
                                                 enable,
                                                 group,
                                                 bitOffset);
        }

        if (GT_OK != rc)
        {
            PRV_UTF_LOG6_MAC("\n[TGF]: prvTgfPclPortListPortMappingSet FAILED,"
                             " rc = [%d], %d, %d, %d, %d, %d\n", rc,
                             direction,
                             prvTgfPortsArray[ii],
                             enable,
                             group,
                             bitOffset);
            return rc;
        }
    }

    return rc;
}

/**
* @internal prvTgfPclPortListCrossCfgIpclRestore function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclPortListCrossCfgIpclRestore
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;
    GT_U8     i;

    /* AUTODOC: disable ingress policy per device */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclIngressPolicyEnable");

    /* AUTODOC: disable using port-list in search keys for lookup0 */
    rc = prvTgfPclLookupCfgPortListEnableSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        0 /*subLookupNum*/,
        GT_FALSE /*enable*/);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclLookupCfgPortListEnableSet");

    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        /* AUTODOC: disable ingress policy on all ports */
        rc = prvTgfPclPortIngressPolicyEnable(
            prvTgfPortsArray[i], GT_FALSE);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortIngressPolicyEnable");
    }

    /* AUTODOC: invalidate PCL rules */
    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        rc = prvTgfPclRuleValidStatusSet(
            PRV_TGF_RULE_SIZE_CNS,
            (i + PRV_TGF_RULE_INDEX_BASE_CNS), GT_FALSE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG2_MAC(
                "\n[TGF]: prvTgfPclRuleValidStatusSet FAILED,"
                " rc = [%d], pclId = %d\n", rc, i);
            rc1 = rc;
        }
    }

    /* AUTODOC: Restore Port List Port Mapping */
    rc1 = prvTgfPclPortListCrossPortMappingSet(CPSS_PCL_DIRECTION_INGRESS_E,
                                               portListPortMappingEnable,
                                               portListPortMappingGroup,
                                               portListPortMappingOffset);

    return rc1;
}

/**
* @internal prvTgfPclPortListCrossCfgEpclRestore function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclPortListCrossCfgEpclRestore
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;
    GT_U8     i;

    rc = prvTgfPclLookupCfgPortListEnableSet(
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        0 /*subLookupNum*/,
        GT_FALSE /*enable*/);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclLookupCfgPortListEnableSet");

    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        /* disable egress policy per devices */
        rc = prvTgfPclEgressPclPacketTypesSet(
            prvTgfDevNum, prvTgfPortsArray[i],
            PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_FALSE);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressPclPacketTypesSet");
    }

    /* AUTODOC: disables egress policy per devices */
    rc = prvTgfPclEgressPolicyEnable(GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressPolicyEnable");

    /* invalidate PCL rules */
    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        rc = prvTgfPclRuleValidStatusSet(
            PRV_TGF_RULE_SIZE_CNS,
            prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(i + PRV_TGF_RULE_INDEX_BASE_CNS), GT_FALSE);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");
    }

    /* AUTODOC: Restore Port List Port Mapping */
    rc1 = prvTgfPclPortListCrossPortMappingSet(CPSS_PCL_DIRECTION_EGRESS_E,
                                               portListPortMappingEnable,
                                               portListPortMappingGroup,
                                               portListPortMappingOffset);

    return rc1;
}

/**
* @internal prvTgfPclPortListCrossTrafficGenerate function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclPortListCrossTrafficGenerate
(
    IN  TGF_PACKET_STC *packetInfoPtr,
    IN  GT_U32           portNumSend,
    IN  GT_U32           portNumReceive,
    IN  GT_U32          expectedVid,
    OUT GT_U32         *numTriggersPtr
)
{
    GT_STATUS               rc, rc1 = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[1];

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    PRV_UTF_LOG2_MAC(
        "sending packet: sendPort [%d] receivePort [%d] \n",
        portNumSend, portNumReceive);

    *numTriggersPtr = 0;

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfEthCountersReset");

    /* setup receive portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = portNumReceive;

    /* enable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "tgftrafficgeneratorporttxethcaptureset");

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevNum, packetInfoPtr,
        1 /*burstCount*/, 0 /*numVfd*/, NULL);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNumSend);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    cpssOsTimerWkAfter(200);

    /* disable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficGeneratorPortTxEthCaptureSet");

    /* check if there is captured packet with specified UP and DSCP fields */
    vfdArray[0].mode          = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset        = TGF_L2_HEADER_SIZE_CNS
                              + TGF_ETHERTYPE_SIZE_CNS; /*UP,CFI,VID*/
    vfdArray[0].cycleCount    = 2;
    vfdArray[0].patternPtr[0] = (GT_U8)((expectedVid >> 8) & 0x0F);
    vfdArray[0].patternPtr[1] = (GT_U8)(expectedVid & 0xFF);


    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portNumReceive);

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(
            &portInterface, 1, vfdArray, numTriggersPtr);
    if (GT_NO_MORE != rc && GT_OK != rc)
        rc1 = rc;

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfEthCountersReset");

    return rc1;
}

/**
* @internal prvTgfPclPortListCrossCfgIpclSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclPortListCrossCfgIpclSet
(
    IN GT_VOID
)
{
    GT_STATUS                         rc = GT_OK;
    static GT_BOOL                    isDeviceInited = GT_FALSE;
    CPSS_INTERFACE_INFO_STC           interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC        lookupCfg;
    CPSS_PCL_DIRECTION_ENT            direction;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT accessMode;
    PRV_TGF_PCL_RULE_FORMAT_UNT       mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT       patt;
    PRV_TGF_PCL_ACTION_STC            action;
    GT_U32                            i;

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

    /* AUTODOC: enables ingress policy for all ports */
    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        rc = prvTgfPclPortIngressPolicyEnable(
            prvTgfPortsArray[i], GT_TRUE);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortIngressPolicyEnable");
    }


    /* configure accessModes for lookups 0_0 */
    direction  = CPSS_PCL_DIRECTION_INGRESS_E;
    accessMode = PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E;

    /* set PCL configuration table for ports */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.pclId                  = 0;
    lookupCfg.dualLookup             = GT_FALSE;
    lookupCfg.pclIdL01               = 0;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_RULE_IPCL_FORMAT_NOT_IP_CNS;
    lookupCfg.groupKeyTypes.ipv4Key  =
        PRV_TGF_RULE_IPCL_FORMAT_IPV4_CNS;
    lookupCfg.groupKeyTypes.ipv6Key  =
        PRV_TGF_RULE_IPCL_FORMAT_IPV6_CNS;

    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        /* AUTODOC: configure lookup0 ingress entry by Port area 0,1,2,3 */
        rc = prvTgfPclPortLookupCfgTabAccessModeSet(
                prvTgfPortsArray[i], direction,
                CPSS_PCL_LOOKUP_0_E, 0 /*sublookup*/, accessMode);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

        interfaceInfo.devPort.portNum = prvTgfPortsArray[i];

        /* AUTODOC: set PCL config table for ports 0,1,2,3 lookup0 with: */
        /* AUTODOC:   nonIpKey=INGRESS_EXT_NOT_IPV6 */
        /* AUTODOC:   ipv4Key=INGRESS_EXT_NOT_IPV6 */
        /* AUTODOC:   ipv6Key=INGRESS_EXT_IPV6_L2 */
        rc = prvTgfPclCfgTblSet(
            &interfaceInfo, direction, CPSS_PCL_LOOKUP_0_E, &lookupCfg);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclCfgTblSet 0");
    }

    /* AUTODOC: save Port List Port Mapping for restore */
    rc = prvTgfPclPortListCrossPortMappingSaveForRestore(
                                                CPSS_PCL_DIRECTION_INGRESS_E);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* AUTODOC: set Port List Port Mapping */
    rc = prvTgfPclPortListCrossPortMappingSet(CPSS_PCL_DIRECTION_INGRESS_E,
                                              GT_TRUE, 0, 0x20);
    if (GT_OK != rc)
    {
        return rc;
    }

    cpssOsMemSet(&mask,   0, sizeof(mask));
    cpssOsMemSet(&patt,   0, sizeof(patt));
    cpssOsMemSet(&action, 0, sizeof(action));

    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.vlan.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;

    /* mirror-capturing                           */
    /* reflects packet using loopback             */
    /* replicates packet using ingress mirroring  */
    /* sets packet's VID==0 using PVID to drop it */
    /* PCL rules modifyind VID must check the     */
    /* previous packet VID to left it dropped     */
    mask.PRV_TGF_RULE_IPCL_UNION_MEMBER_CNS.common.vid = 0xFFF;
    patt.PRV_TGF_RULE_IPCL_UNION_MEMBER_CNS.common.vid =
        PRV_TGF_SRC_VLANID_CNS;

    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        /* for each rule it's own VID */
        action.vlan.vlanId = (GT_U16)(PRV_TGF_VLANID_BASE_CNS + i);

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        {
            mask.PRV_TGF_RULE_IPCL_UNION_MEMBER_CNS.common.portListBmp.ports[0] =
                                                                        0xFFFFFFF;
            mask.PRV_TGF_RULE_IPCL_UNION_MEMBER_CNS.common.portListBmp.ports[0] &=
                                                                        (~ (1 << i));
        }
        else
        {
            /* port list bitmap pattern - all bits zeros                     */
            /* port list bitmap mask - all bits ones beside the matched port */
            cpssOsMemSet(
                &(mask.PRV_TGF_RULE_IPCL_UNION_MEMBER_CNS.common.portListBmp),
                0xFF, sizeof(CPSS_PORTS_BMP_STC));
            /* each rule matches it's own port */
            mask.PRV_TGF_RULE_IPCL_UNION_MEMBER_CNS.common.portListBmp
                .ports[prvTgfPortsArray[i] / 32] &=
                (~ (1 << (prvTgfPortsArray[i] % 32)));
        }

        /* AUTODOC: set PCL rules 3,4,5,6 with: */
        /* AUTODOC:   format INGRESS_EXT_NOT_IPV6, cmd=FORWARD */
        /* AUTODOC:   pattern VID=2 */
        /* AUTODOC:   enable modify VLAN, action VLANs=[10..13] */
        rc = prvTgfPclRuleWithOptionsSet(
            PRV_TGF_RULE_IPCL_FORMAT_NOT_IP_CNS,
            (PRV_TGF_RULE_INDEX_BASE_CNS + i),
            0 /*ruleOptionsBmp*/,
            &mask, &patt, &action);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclRuleSet");
    }

    /* AUTODOC: enable ingress using port-list in search keys for lookup0 */
    rc = prvTgfPclLookupCfgPortListEnableSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        0 /*subLookupNum*/,
        GT_TRUE /*enable*/);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclLookupCfgPortListEnableSet");

    return GT_OK;
}

/**
* @internal prvTgfPclPortListCrossCfgEpclSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclPortListCrossCfgEpclSet
(
    IN GT_VOID
)
{
    GT_STATUS                         rc = GT_OK;
    static GT_BOOL                    isDeviceInited = GT_FALSE;
    CPSS_INTERFACE_INFO_STC           interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC        lookupCfg;
    CPSS_PCL_DIRECTION_ENT            direction;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT accessMode;
    PRV_TGF_PCL_RULE_FORMAT_UNT       mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT       patt;
    PRV_TGF_PCL_ACTION_STC            action;
    GT_U32                            i;

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

    /* AUTODOC: enables egress policy per devices */
    rc = prvTgfPclEgressPolicyEnable(GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclEgressPolicyEnable");

    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        /* AUTODOC: enables egress policy for all ports and non-tunneled packets */
        rc = prvTgfPclEgressPclPacketTypesSet(
            prvTgfDevNum, prvTgfPortsArray[i],
            PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_TRUE);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclEgressPclPacketTypesSet");
    }

    /* configure accessModes for lookups 0_0 */
    direction  = CPSS_PCL_DIRECTION_EGRESS_E;
    accessMode = PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E;

    /* set PCL configuration table for ports */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.pclId                  = 0;
    lookupCfg.dualLookup             = GT_FALSE;
    lookupCfg.pclIdL01               = 0;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_RULE_EPCL_FORMAT_NOT_IP_CNS;
    lookupCfg.groupKeyTypes.ipv4Key  =
        PRV_TGF_RULE_EPCL_FORMAT_IPV4_CNS;
    lookupCfg.groupKeyTypes.ipv6Key  =
        PRV_TGF_RULE_EPCL_FORMAT_IPV6_CNS;

    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        /* AUTODOC: configure lookup0 egress entry by Port area 0,1,2,3 */
        rc = prvTgfPclPortLookupCfgTabAccessModeSet(
                prvTgfPortsArray[i], direction,
                CPSS_PCL_LOOKUP_0_E, 0 /*sublookup*/, accessMode);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

        interfaceInfo.devPort.portNum = prvTgfPortsArray[i];

        /* AUTODOC: set PCL config table for ports 0,1,2,3 lookup0 with: */
        /* AUTODOC:   nonIpKey=EGRESS_EXT_NOT_IPV6 */
        /* AUTODOC:   ipv4Key=EGRESS_EXT_NOT_IPV6 */
        /* AUTODOC:   ipv6Key=EGRESS_EXT_IPV6_L2 */
        rc = prvTgfPclCfgTblSet(
            &interfaceInfo, direction, CPSS_PCL_LOOKUP_0_E, &lookupCfg);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclCfgTblSet 0");
    }

    /* AUTODOC: save Port List Port Mapping for restore */
    rc = prvTgfPclPortListCrossPortMappingSaveForRestore(
                                                CPSS_PCL_DIRECTION_EGRESS_E);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* AUTODOC: set Port List Port Mapping */
    rc = prvTgfPclPortListCrossPortMappingSet(CPSS_PCL_DIRECTION_EGRESS_E,
                                              GT_TRUE, 0, 0x20);
    if (GT_OK != rc)
    {
        return rc;
    }

    cpssOsMemSet(&mask,   0, sizeof(mask));
    cpssOsMemSet(&patt,   0, sizeof(patt));
    cpssOsMemSet(&action, 0, sizeof(action));

    action.egressPolicy = GT_TRUE;
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.vlan.egressVlanId0Cmd =
        PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;

    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        /* for each rule it's own VID */
        action.vlan.vlanId = (GT_U16)(PRV_TGF_VLANID_BASE_CNS + i);

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        {
            mask.PRV_TGF_RULE_EPCL_UNION_MEMBER_CNS.common.portListBmp.ports[0] =
                                                                        0xFFFFFFF;
            mask.PRV_TGF_RULE_EPCL_UNION_MEMBER_CNS.common.portListBmp.ports[0] &=
                                                                        (~ (1 << i));
        }
        else
        {
            /* port list bitmap pattern - all bits zeros                     */
            /* port list bitmap mask - all bits ones beside the matched port */
            cpssOsMemSet(
                &(mask.PRV_TGF_RULE_EPCL_UNION_MEMBER_CNS.common.portListBmp),
                0xFF, sizeof(CPSS_PORTS_BMP_STC));
            /* each rule matches it's own port */
            mask.PRV_TGF_RULE_EPCL_UNION_MEMBER_CNS.common.portListBmp
                .ports[prvTgfPortsArray[i] / 32] &=
                (~ (1 << (prvTgfPortsArray[i] % 32)));
        }

        /* AUTODOC: set PCL rules 3,4,5,6 with: */
        /* AUTODOC:   format EGRESS_EXT_NOT_IPV6, cmd=FORWARD */
        /* AUTODOC:   pattern VID=2 */
        /* AUTODOC:   enable modify VLAN, action VLANs=[10..13] */
        rc = prvTgfPclRuleWithOptionsSet(
            PRV_TGF_RULE_EPCL_FORMAT_NOT_IP_CNS,
            prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(PRV_TGF_RULE_INDEX_BASE_CNS + i),
            0 /*ruleOptionsBmp*/,
            &mask, &patt, &action);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclRuleSet");
    }

    /* AUTODOC: enable egress using port-list in search keys for lookup0 */
    rc = prvTgfPclLookupCfgPortListEnableSet(
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        0 /*subLookupNum*/,
        GT_TRUE /*enable*/);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclLookupCfgPortListEnableSet");

    return GT_OK;
}

/**
* @internal prvTgfPclPortListCrossIpclCheck function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfPclPortListCrossIpclCheck
(
    IN  GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    numTriggers;
    GT_U32    expectedVid;
    GT_U32     sendPort;
    GT_U32     receivePort;
    GT_U32    i;

    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        sendPort = prvTgfPortsArray[i];

        /* any port other from sendPort */
        receivePort = prvTgfPortsArray[(i + 1) % prvTgfPortsNum];

        expectedVid = (PRV_TGF_VLANID_BASE_CNS + i);

        /* AUTODOC: send 4 Ethernet packet on ports 0,1,2,3 with: */
        /* AUTODOC:   DA=00:00:00:00:34:03, SA=00:00:00:00:00:55 */
        /* AUTODOC:   VID=2, EtherType=0x3333 */
        rc = prvTgfPclPortListCrossTrafficGenerate(
            &packet1TgfInfo, sendPort, receivePort,
            expectedVid, &numTriggers);
        UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc,
            "prvTgfPclPortListModeTrafficGenerate: dev=%d ingr=%d, egr=%d",
            prvTgfDevNum, sendPort, receivePort);
        /* AUTODOC: verify traffic: */
        /* AUTODOC:   tagged 4 packets VID=[10..13] received on ports 1,2,3,0 */

        /* check triggers */
        UTF_VERIFY_EQUAL2_STRING_MAC(1, numTriggers,
            "tgfTrafficGeneratorPortTxEthTriggerCountersGet:\n"
            "   !!! Captured packet has wrong VID fields !!!\n"
            "   numTriggers = %d, rc = 0x%02X", numTriggers, rc);
    }
}

/**
* @internal prvTgfPclPortListCrossEpclCheck function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfPclPortListCrossEpclCheck
(
    IN  GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    numTriggers;
    GT_U32    expectedVid;
    GT_U32     sendPort;
    GT_U32     receivePort;
    GT_U32    i;

    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        receivePort = prvTgfPortsArray[i];

        /* any port other from receivePort */
        sendPort = prvTgfPortsArray[(i + 1) % prvTgfPortsNum];

        expectedVid = (PRV_TGF_VLANID_BASE_CNS + i);

        /* AUTODOC: send 4 Ethernet packet on ports 1,2,3,0 with: */
        /* AUTODOC:   DA=00:00:00:00:34:03, SA=00:00:00:00:00:55 */
        /* AUTODOC:   VID=2, EtherType=0x3333 */
        rc = prvTgfPclPortListCrossTrafficGenerate(
            &packet1TgfInfo, sendPort, receivePort,
            expectedVid, &numTriggers);
        UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc,
            "prvTgfPclPortListModeTrafficGenerate: dev=%d ingr=%d, egr=%d",
            prvTgfDevNum, sendPort, receivePort);
        /* AUTODOC: verify traffic: */
        /* AUTODOC:   tagged 4 packets VID=[10..13] received on ports 0,1,2,3 */

        /* check triggers */
        UTF_VERIFY_EQUAL2_STRING_MAC(1, numTriggers,
            "tgfTrafficGeneratorPortTxEthTriggerCountersGet:\n"
            "   !!! Captured packet has wrong VID fields !!!\n"
            "   numTriggers = %d, rc = 0x%02X", numTriggers, rc);
    }
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/*******************************************************************************
* prvTgfPclPortListCrossIpclTest
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID prvTgfPclPortListCrossIpclTest
(
    GT_VOID
)
{
    GT_STATUS                            rc, rc1 = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* set common configuration */
    rc = prvTgfPclPortListCrossCfgVlanSet();
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortListCrossCfgVlanSet");

    /* set IPCL configuration */
    rc = prvTgfPclPortListCrossCfgIpclSet();
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortListCrossCfgIpclSet");

    /* -------------------------------------------------------------------------
     * 2. Generate traffic
     */

    /* AUTODOC: GENERATE TRAFFIC: */
    prvTgfPclPortListCrossIpclCheck();

    /* -------------------------------------------------------------------------
     * 3. Restore configuration
     */

    /* AUTODOC: RESTORE CONFIGURATION: */
    rc = prvTgfPclPortListCrossCfgIpclRestore();
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclPortListCrossCfgIpclRestore");

    rc = prvTgfPclPortListCrossCfgVlanRestore();
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclPortListCrossCfgVlanRestore");

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
}

/*******************************************************************************
* prvTgfPclPortListCrossEpclTest
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID prvTgfPclPortListCrossEpclTest
(
    GT_VOID
)
{
    GT_STATUS                            rc, rc1 = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* set common configuration */
    rc = prvTgfPclPortListCrossCfgVlanSet();
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortListCrossCfgVlanSet");

    /* set IPCL configuration */
    rc = prvTgfPclPortListCrossCfgEpclSet();
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortListCrossCfgEpclSet");

    /* -------------------------------------------------------------------------
     * 2. Generate traffic
     */

    /* AUTODOC: GENERATE TRAFFIC: */
    prvTgfPclPortListCrossEpclCheck();

    /* -------------------------------------------------------------------------
     * 3. Restore configuration
     */

    /* AUTODOC: RESTORE CONFIGURATION: */
    rc = prvTgfPclPortListCrossCfgEpclRestore();
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclPortListCrossCfgEpclRestore");

    rc = prvTgfPclPortListCrossCfgVlanRestore();
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclPortListCrossCfgVlanRestore");

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
}




