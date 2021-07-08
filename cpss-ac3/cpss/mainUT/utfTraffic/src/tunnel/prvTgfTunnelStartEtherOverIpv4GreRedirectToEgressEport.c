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
* @file prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEport.c
*
* @brief Tunnel start Ethernet over Ipv4 GRE redirect to egress ePort
*
* @version   10
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
#include <common/tgfTunnelGen.h>
#include <tunnel/prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEport.h>
#include <common/tgfCosGen.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_VLANID_5_CNS            5

/* default VLAN Id */
#define PRV_TGF_VLANID_6_CNS            6

/* destination ePort */
#define PRV_TGF_DESTINATION_EPORT_CNS   (UTF_CPSS_PP_MAX_PORT_NUM_CNS(prvTgfDevNum)  - 5)

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number to receive traffic from  */
#define PRV_TGF_EGR_PORT_IDX_CNS        2

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

/* number of vfd's */
#define PRV_TGF_VFD_NUM_CNS             6

/* pcl rule index */
#define PRV_TGF_PCL_RULE_IDX_CNS        1

/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 3;

/* expected number of packets on ports */
static GT_U8 prvTgfPacketsCountRxTxArr[PRV_TGF_PORTS_NUM_CNS] = { 1, 0, 1, 0 };

#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }
/***************** Tunnel config params  *******************************/

/* line index for the tunnel start entry in the router ARP / tunnel start table */
static GT_U32 prvTgfRouterArpTunnelStartLineIndex = 8;

/* Tunnel next hop MAC DA */
static TGF_MAC_ADDR  prvTgfTunnelMacDa = {0x88, 0x99, 0x77, 0x66, 0x55, 0x88};


/***************** Incapsulation ethernet frame **********************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},    /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x25}     /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,        /* etherType */
    0, 0, PRV_TGF_VLANID_5_CNS              /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {0x0030};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),  /* dataLength */
    prvTgfPayloadDataArr           /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of packet */
#define PRV_TGF_PACKET_LEN_CNS    \
       TGF_L2_HEADER_SIZE_CNS   + \
       TGF_VLAN_TAG_SIZE_CNS    + \
       TGF_ETHERTYPE_SIZE_CNS   + \
       sizeof(prvTgfPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                  /* totalLen */
    (sizeof(prvTgfPacketPartArray)
        / sizeof(prvTgfPacketPartArray[0])), /* numOfParts */
    prvTgfPacketPartArray                    /* partsArray */
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    CPSS_INTERFACE_INFO_STC       physicalInfo;
} prvTgfRestoreCfg;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
static GT_BOOL useDscpRemapOnTs = GT_FALSE;
static GT_BOOL egressQosDscpEnableGet = GT_FALSE;
static GT_U32  dscp2dscpGet = GT_FALSE;
static GT_U32  egressMappingTableIndexGet = 0;
#define ORIG_OUTER_IP_DSCP_CNS               0x22
#define OUTER_IP_DSCP_AFTER_DSCP_REMAP_CNS   0x33
/* due to the errata that actual DSCP-TO-DSCP is from 0-To-new */
/* so although the TS DSCP is 0x22 and remap to 0x33
    the behavior will be  remap of ??? to 0x11
*/
static GT_U32  errata_dscp2dscpGet = GT_FALSE;
#define ERRATA__OUTER_IP_DSCP_AFTER_DSCP_REMAP_CNS   0x11
#define ERRATA__IN_IP_DSCP_CNS                       0x12
static GT_U32 orig_prvTgfBurstCount = 3;
static GT_U32 errata_testNum = 0;
static TGF_ETHER_TYPE origEtherType = 0x0030;
/**
* @internal prvTgfTunnelDscpRemarkSet function
* @endinternal
*
* @brief   Egress QoS mapping set for DSCP-to-DSCP (by egress port)
*
* @param[in] egressPort               - the port to set configuration on.
* @param[in] doRemap                  - do we set/restore the DSCP-to-DSCP remap
*                                       None
*/
static GT_VOID prvTgfTunnelDscpRemarkSet
(
    IN GT_U32  egressPort,
    IN GT_BOOL doRemap
)
{
    GT_STATUS rc;
    GT_U32  mappingTableIndex = 5;
    GT_U32  inDscp = ORIG_OUTER_IP_DSCP_CNS , outDscp = OUTER_IP_DSCP_AFTER_DSCP_REMAP_CNS;
    GT_U32  errata_inDscp = ERRATA__IN_IP_DSCP_CNS , errata_outDscp = ERRATA__OUTER_IP_DSCP_AFTER_DSCP_REMAP_CNS;
    GT_U16  errata_byteIndexOfIpHeader = 0;

    if(doRemap == GT_TRUE)
    {
        if(errata_testNum > 3)
        {
            rc = GT_FAIL;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "testNum > 3 \n");
            return;
        }

        PRV_UTF_LOG1_MAC("start Errata test #%d",errata_testNum);

        if(errata_testNum == 0)
        {
            errata_byteIndexOfIpHeader = 0;
            origEtherType = prvTgfPacketEtherTypePart.etherType;
            prvTgfPacketEtherTypePart.etherType = 0xABAB;/* > 0x0600 --> Eth2*/

            PRV_UTF_LOG0_MAC("======= errata : ETH2 =======\n");

            ipv4Case_lbl:
            PRV_UTF_LOG0_MAC("======= errata : assume ipv4 =======\n");
            /* for the errata behavior */
            if (GT_TRUE == prvUtfIsGmCompilation() &&
               PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_FALSE)
            {
                prvTgfPayloadDataArr[errata_byteIndexOfIpHeader] = 0x45;/*emulate ipv4*/
                prvTgfPayloadDataArr[errata_byteIndexOfIpHeader+1] = (GT_U8)(errata_inDscp << 2);
            }
            else
            {
                /* due to errata the board consider the packet as ipv6 instead of ipv4,
                   so the dscp bits are taken according to ipv6 packet */
                prvTgfPayloadDataArr[errata_byteIndexOfIpHeader] =(GT_U8)( 0x40/*try to emulate ipv4*/ |
                                                                  (errata_inDscp >> 2));/*even tried to emulate ipv4 -->
                                                                        still considered as ipv6 therefore field is like in IPv6 header --> fill DSCP 4 MSB*/
                prvTgfPayloadDataArr[errata_byteIndexOfIpHeader+1] = (GT_U8)((errata_inDscp << 6) |
                                                                 0x5)/*dummy last nibble*/;/*fill DSCP 2 LSB*/
            }
        }
        else
        if(errata_testNum == 1)
        {
            errata_byteIndexOfIpHeader = 0;

            PRV_UTF_LOG0_MAC("======= errata : ETH2 checks =======\n");

            ipv6Case_lbl:
            PRV_UTF_LOG0_MAC("======= errata : assume ipv6 =======\n");
            /* for the errata behavior */
            prvTgfPayloadDataArr[errata_byteIndexOfIpHeader] = (GT_U8)(errata_inDscp >> 2);/*emulate none ipv4 --> ipv6 *//*DSCP 4 MSB*/
            prvTgfPayloadDataArr[errata_byteIndexOfIpHeader+1] = (GT_U8)(errata_inDscp << 6);/*DSCP 2 LSB*/
        }
        else /* 2,3 */
        {
            prvTgfPacketEtherTypePart.etherType = origEtherType;/* 0x0030 --> LLC(non-SNAP) --> length of the packet  */

            PRV_UTF_LOG0_MAC("======= errata : LLC(non-SNAP) checks =======\n");
            /* restore values changed by previous tests for the errata behavior */
            prvTgfPayloadDataArr[0] = 0x00;
            prvTgfPayloadDataArr[1] = 0x01;
            if(errata_testNum == 2)
            {
                errata_byteIndexOfIpHeader = 3;/* the LLC header is 2 bytes (the 'length') (when none-SNAP)*/
                /* do ipv4 emulation */
                goto ipv4Case_lbl;
            }
            else /*errata_testNum == 3*/
            {
                errata_byteIndexOfIpHeader = 3;/* the LLC header is 2 bytes (the 'length') (when none-SNAP)*/
                /* do ipv6 emulation */
                goto ipv6Case_lbl;
            }
        }

        if(errata_testNum == 0)
        {
            orig_prvTgfBurstCount = prvTgfBurstCount;
            /* AUTODOC: get egress QoS Dscp2Dscp mapping enable state */
            rc = prvTgfCosPortEgressQoSDscpMappingEnableGet(prvTgfDevNum, egressPort, &egressQosDscpEnableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                       "prvTgfCosPortEgressQoSDscpMappingEnableGet: %d, %d", prvTgfDevNum, egressPort);
            /* AUTODOC: get egress eport  mapping table select */
            rc = prvTgfCosPortEgressQoSMappingTableIndexGet(prvTgfDevNum, egressPort,&egressMappingTableIndexGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                         "prvTgfCosPortEgressQoSMappingTableIndexGet: %d, %d", prvTgfDevNum, egressPort);
            /* AUTODOC: get egress eport Dscp2Dscp mapping */
             rc = prvTgfCosEgressDscp2DscpMappingEntryGet(prvTgfDevNum, mappingTableIndex, inDscp, &dscp2dscpGet);
             UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfCosEgressExp2ExpMappingEntryGet : %d",prvTgfDevNum);

            /* AUTODOC: (errata) get egress eport Dscp2Dscp mapping */
             rc = prvTgfCosEgressDscp2DscpMappingEntryGet(prvTgfDevNum, mappingTableIndex, errata_inDscp, &errata_dscp2dscpGet);
             UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfCosEgressExp2ExpMappingEntryGet : %d",prvTgfDevNum);
        }

        prvTgfBurstCount = 1;
        /* AUTODOC: set egress QoS Dscp2Dscp mapping enable state */
        rc = prvTgfCosPortEgressQoSDscpMappingEnableSet(prvTgfDevNum, egressPort, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfCosPortEgressQoSDscpMappingEnableSet: %d, %d", prvTgfDevNum, egressPort);
        /* AUTODOC: set egress eport  mapping table select */
        rc = prvTgfCosPortEgressQoSMappingTableIndexSet(prvTgfDevNum, egressPort,mappingTableIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfCosPortEgressQoSMappingTableIndexSet: %d, %d", prvTgfDevNum, egressPort);

         /* AUTODOC: set egress eport Dscp2Dscp mapping */
         rc = prvTgfCosEgressDscp2DscpMappingEntrySet(prvTgfDevNum, mappingTableIndex, inDscp, outDscp);
         UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfCosEgressExp2ExpMappingEntrySet : %d",prvTgfDevNum);

         /* AUTODOC: (errata) set egress eport Dscp2Dscp mapping */
         rc = prvTgfCosEgressDscp2DscpMappingEntrySet(prvTgfDevNum, mappingTableIndex, errata_inDscp, errata_outDscp);
         UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfCosEgressExp2ExpMappingEntrySet : %d",prvTgfDevNum);

        errata_testNum ++;
    }
    else
    {
        prvTgfPayloadDataArr[0] = 0x00;
        prvTgfPayloadDataArr[1] = 0x01;
        prvTgfPayloadDataArr[3] = 0x03;
        prvTgfPayloadDataArr[4] = 0x04;
        prvTgfPacketEtherTypePart.etherType = origEtherType;

        prvTgfBurstCount = orig_prvTgfBurstCount;
        /* AUTODOC: restore egress QoS Dscp2Dscp mapping enable state */
        rc = prvTgfCosPortEgressQoSDscpMappingEnableSet(prvTgfDevNum, egressPort, egressQosDscpEnableGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfCosPortEgressQoSDscpMappingEnableSet: %d, %d", prvTgfDevNum, egressPort);
        /* AUTODOC: restore egress eport  mapping table select */
        rc = prvTgfCosPortEgressQoSMappingTableIndexSet(prvTgfDevNum, egressPort,egressMappingTableIndexGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfCosPortEgressQoSMappingTableIndexSet: %d, %d", prvTgfDevNum, egressPort);
        /* AUTODOC: restore egress eport Dscp2Dscp mapping */
        rc = prvTgfCosEgressDscp2DscpMappingEntrySet(prvTgfDevNum, mappingTableIndex, inDscp, dscp2dscpGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfCosEgressDscp2DscpMappingEntrySet : %d",prvTgfDevNum);

        /* AUTODOC: (errata)restore egress eport Dscp2Dscp mapping */
        rc = prvTgfCosEgressDscp2DscpMappingEntrySet(prvTgfDevNum, mappingTableIndex, inDscp, errata_dscp2dscpGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfCosEgressDscp2DscpMappingEntrySet : %d",prvTgfDevNum);
    }
}


/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportBridgeConfigSet
(
    IN GT_BOOL __useDscpRemapOnTs
)
{
    GT_STATUS rc = GT_OK;

    /* save the value for later use */
    useDscpRemapOnTs = __useDscpRemapOnTs;

    if(useDscpRemapOnTs == GT_TRUE)
    {
        PRV_UTF_LOG0_MAC("======= Run the test with 'Dscp Remap On Ts' =======\n");
    }

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS, prvTgfPortsArray,
                                           NULL, NULL, 4);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* set default vlan entry for vlan ID 6 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_6_CNS, prvTgfPortsArray + 2,
                                           NULL, NULL, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, ePort 1000 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfPacketL2Part.daMac,
                                          PRV_TGF_VLANID_5_CNS,
                                          prvTgfDevNum,
                                          PRV_TGF_DESTINATION_EPORT_CNS,
                                          GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                        "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    if(useDscpRemapOnTs == GT_TRUE)
    {
        prvTgfTunnelDscpRemarkSet(PRV_TGF_DESTINATION_EPORT_CNS , GT_TRUE);
    }
}

/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportTunnelConfigSet function
* @endinternal
*
* @brief   Set Tunnel Configuration
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportTunnelConfigSet
(
    GT_VOID
)
{
    GT_STATUS                      rc = GT_OK;
    CPSS_TUNNEL_TYPE_ENT           tunnelType;
    PRV_TGF_TUNNEL_START_ENTRY_UNT tunnelEntry;

    PRV_UTF_LOG0_MAC("======= Setting Tunnel Configuration =======\n");

    /* clear tunnelEntry */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;
    }
    else
    {
        tunnelType = CPSS_TUNNEL_X_OVER_GRE_IPV4_E;
    }

    /*** Set a tunnel start entry ***/
    tunnelEntry.ipv4Cfg.tagEnable    = GT_TRUE;
    tunnelEntry.ipv4Cfg.vlanId       = PRV_TGF_VLANID_5_CNS;
    tunnelEntry.ipv4Cfg.upMarkMode   = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.ipv4Cfg.dscpMarkMode = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.ipv4Cfg.dscp         = ORIG_OUTER_IP_DSCP_CNS;
    tunnelEntry.ipv4Cfg.ttl          = 40;
    tunnelEntry.ipv4Cfg.ethType      = CPSS_TUNNEL_GRE0_ETHER_TYPE_E;
    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        tunnelEntry.ipv4Cfg.ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
    }

    cpssOsMemSet((GT_VOID*) &tunnelEntry.ipv4Cfg.srcIp.arIP, 0xA,
                 sizeof(tunnelEntry.ipv4Cfg.srcIp.arIP));
    cpssOsMemSet((GT_VOID*) &tunnelEntry.ipv4Cfg.destIp.arIP, 4,
                 sizeof(tunnelEntry.ipv4Cfg.destIp.arIP));

    /* tunnel next hop MAC DA */
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.macDa.arEther,
                 prvTgfTunnelMacDa, sizeof(prvTgfTunnelMacDa));

    /* AUTODOC: add Tunnel Start entry 8 with: */
        /* AUTODOC:   tunnelType=X_OVER_GRE_IPV4 */
        /* AUTODOC:   vlanId=5, ttl=40, ethType=TUNNEL_GRE0_ETHER_TYPE */
        /* AUTODOC:   upMarkMode=dscpMarkMode=MARK_FROM_ENTRY */
        /* AUTODOC:   DA=88:99:77:66:55:88, srcIp=10.10.10.10, dstIp=4.4.4.4 */
    rc = prvTgfTunnelStartEntrySet(prvTgfRouterArpTunnelStartLineIndex,
                                   tunnelType, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfTunnelStartEntrySet: %d", prvTgfDevNum);

    /* clear tunnelEntry */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    /* AUTODOC: get and check added TS Entry */
    rc = prvTgfTunnelStartEntryGet(prvTgfDevNum, prvTgfRouterArpTunnelStartLineIndex,
                                   &tunnelType, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntryGet: %d", prvTgfDevNum);

    /* Check vlanId field */
    UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_VLANID_5_CNS, tunnelEntry.ipv4Cfg.vlanId,
                     "tunnelEntry.ipv4Cfg.vlanId: %d", tunnelEntry.ipv4Cfg.vlanId);

    /* Check macDa field */
    rc = cpssOsMemCmp(tunnelEntry.ipv4Cfg.macDa.arEther, prvTgfTunnelMacDa,
                      sizeof(prvTgfTunnelMacDa)) == 0 ?  GT_OK : GT_FALSE;
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "tunnelEntry.ipv4Cfg.macDa.arEther[5]: 0x%2X",
                            tunnelEntry.ipv4Cfg.macDa.arEther[5]);
}

/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportPclConfigSet function
* @endinternal
*
* @brief   Set Pcl configuration
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportPclConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    CPSS_INTERFACE_INFO_STC          physicalInfo;

    /* AUTODOC: init PCL Engine for send port 0: */
        /* AUTODOC:   ingress direction, LOOKUP_0 */
        /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
        /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
        /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInit(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                              CPSS_PCL_DIRECTION_INGRESS_E,
                              CPSS_PCL_LOOKUP_0_E,
                              PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                              PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                              PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    /******* set PCL rule - FORWARD packet with MAC DA = ... 34 02 *******/
    /* clear mask */
    cpssOsMemSet(&mask, 0, sizeof(mask));

    /* clear pattern */
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* clear action */
    cpssOsMemSet(&action, 0, sizeof(action));

    /* setup action */
    action.pktCmd               = CPSS_PACKET_CMD_FORWARD_E;
    action.redirect.redirectCmd = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;

    action.redirect.data.outIf.outInterface.type = CPSS_INTERFACE_PORT_E;
    action.redirect.data.outIf.outInterface.devPort.hwDevNum  = prvTgfDevNum;
    action.redirect.data.outIf.outInterface.devPort.portNum = PRV_TGF_DESTINATION_EPORT_CNS;

    action.redirect.data.outIf.outlifType = PRV_TGF_OUTLIF_TYPE_TUNNEL_E;

    action.redirect.data.outIf.outlifPointer.tunnelStartPtr.tunnelType =
                            PRV_TGF_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ETHERNET_E;
    action.redirect.data.outIf.outlifPointer.tunnelStartPtr.ptr =
                                            prvTgfRouterArpTunnelStartLineIndex;

    /* setup pattern */
    cpssOsMemCpy(pattern.ruleStdNotIp.macDa.arEther,
                 prvTgfPacketL2Part.daMac,
                 sizeof(pattern.ruleStdNotIp.macDa.arEther));

    /* setup mask */
    cpssOsMemSet(mask.ruleStdNotIp.macDa.arEther, 0xFF,
                 sizeof(mask.ruleStdNotIp.macDa.arEther));

    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    /* AUTODOC: set PCL rule 1 with: */
        /* AUTODOC:   format INGRESS_STD_NOT_IP, cmd=FORWARD */
        /* AUTODOC:   redirectCmd=REDIRECT_CMD_OUT_IF, ePort=1000  */
        /* AUTODOC:   outlifType=TUNNEL, tunnelType=ETHERNET, tsPtr=8 */
        /* AUTODOC:   pattern DA=00:00:00:00:34:02 */
    rc = prvTgfPclRuleSet(ruleFormat, PRV_TGF_PCL_RULE_IDX_CNS,
                          &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                 prvTgfDevNum, ruleFormat);

     /* save ePort mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT_CNS,
                                                           &(prvTgfRestoreCfg.physicalInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableGet);

    /* set ePort mapping configuration */
    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: map port 2 to ePort 1000 */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT_CNS,
                                                           &physicalInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);
}

/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:01,
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    portIter    = 0;
    GT_U32    packetSize  = 0;
    GT_U32    packetIter  = 0;
    GT_U32    numTriggers = 0;

    TGF_VFD_INFO_STC         vfdArray[PRV_TGF_VFD_NUM_CNS] = {{0}};
    CPSS_INTERFACE_INFO_STC  portInterface;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: send 3 Ethernet packets from port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:25, VID=5 */
        /* AUTODOC:   EtherType=0x0030 */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL,
                    prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                         prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify to get 3 tunneled packets on port 2 with: */
        /* AUTODOC:   DA=88:99:77:66:55:88, SA=00:00:00:00:00:05, VID=5 */
        /* AUTODOC:   EtherType=0x6558 */
        /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:25 */
        /* AUTODOC:   passenger EtherType=0x0030 */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* calculate packet length */
        packetSize = prvTgfPacketInfo.totalLen +
                    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS +
                     TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS)  * (portIter == PRV_TGF_EGR_PORT_IDX_CNS);

        /* check ETH counters */
        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                    prvTgfPortsArray[portIter],
                                    prvTgfPacketsCountRxTxArr[portIter],
                                    prvTgfPacketsCountRxTxArr[portIter],
                                    packetSize, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
    }

    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* get trigger counters */

    /* check da mac */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset     = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfTunnelMacDa, sizeof(TGF_MAC_ADDR));

    /* check vlan ethertype */
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].offset     = sizeof(prvTgfTunnelMacDa) * 2;
    vfdArray[1].cycleCount = TGF_ETHERTYPE_SIZE_CNS;

    vfdArray[1].patternPtr[0] = TGF_ETHERTYPE_8100_VLAN_TAG_CNS >> 8;
    vfdArray[1].patternPtr[1] = TGF_ETHERTYPE_8100_VLAN_TAG_CNS & 0xFF;

    /* check vlan id */
    vfdArray[2].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[2].offset     = vfdArray[1].offset + vfdArray[1].cycleCount;
    vfdArray[2].cycleCount = 2;
    vfdArray[2].patternPtr[0] = 0;
    vfdArray[2].patternPtr[1] = PRV_TGF_VLANID_5_CNS;

    /* check ethertype */
    vfdArray[3].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[3].offset     = vfdArray[2].offset + vfdArray[2].cycleCount;
    vfdArray[3].cycleCount = 2;
    vfdArray[3].patternPtr[0] = TGF_ETHERTYPE_0800_IPV4_TAG_CNS >> 8;
    vfdArray[3].patternPtr[1] = TGF_ETHERTYPE_0800_IPV4_TAG_CNS & 0xFF;

    /* check ipv4 header (version + TOS) */
    vfdArray[4].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[4].offset     = vfdArray[3].offset + vfdArray[3].cycleCount;
    vfdArray[4].cycleCount = 2;
    vfdArray[4].patternPtr[0] = 0x45;
    if(useDscpRemapOnTs == GT_FALSE)
    {
        vfdArray[4].patternPtr[1] = ORIG_OUTER_IP_DSCP_CNS << 2;/*TOS (DSCP shifted by 2 bits)*/
    }
    else
    {
        if (PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum))
        {
            vfdArray[4].patternPtr[1] = OUTER_IP_DSCP_AFTER_DSCP_REMAP_CNS << 2;/*TOS (DSCP shifted by 2 bits)*/
        }
        else
        {
            /* use ERRATA__OUTER_IP_DSCP_AFTER_DSCP_REMAP_CNS and not OUTER_IP_DSCP_AFTER_DSCP_REMAP_CNS
               due to the errata */
            vfdArray[4].patternPtr[1] = ERRATA__OUTER_IP_DSCP_AFTER_DSCP_REMAP_CNS << 2;/*TOS (DSCP shifted by 2 bits)*/
        }
    }

    /* check ipv4 header (totalLen) */
    vfdArray[5].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[5].offset     = vfdArray[4].offset + vfdArray[4].cycleCount;
    vfdArray[5].cycleCount = 2;
    vfdArray[5].patternPtr[0] = 0;
    vfdArray[5].patternPtr[1] = 0x56;

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface,
                           PRV_TGF_VFD_NUM_CNS, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                    portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL1_STRING_MAC((1 << (PRV_TGF_VFD_NUM_CNS * prvTgfBurstCount)) - 1, numTriggers,
                                 "wrong numTriggers =  %d\n", numTriggers);

    for(packetIter = 0; packetIter < prvTgfBurstCount; packetIter++)
    {
        /* check if captured packet has all correct vfd's */
        UTF_VERIFY_EQUAL1_STRING_MAC((1 << PRV_TGF_VFD_NUM_CNS) - 1,
                      numTriggers & ((1 << PRV_TGF_VFD_NUM_CNS) - 1),
            "   Errors while triggers summary result checking: packetIter = %d\n", packetIter);

        /* check if captured packet has the same MAC DA as prvTgfArpMac */
        UTF_VERIFY_EQUAL6_STRING_MAC(GT_TRUE, numTriggers & BIT_0,
                "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                                        prvTgfPacketL2Part.daMac[0],
                                        prvTgfPacketL2Part.daMac[1],
                                        prvTgfPacketL2Part.daMac[2],
                                        prvTgfPacketL2Part.daMac[3],
                                        prvTgfPacketL2Part.daMac[4],
                                        prvTgfPacketL2Part.daMac[5]);

        /* check if captured packet has the same vlan Ethertype */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, (numTriggers & BIT_1) >> 1,
                "\n   Ethertype of captured packet must be: %04X",
                                       (vfdArray[1].patternPtr[0] << 8) | vfdArray[1].patternPtr[1]);

        /* check if captured packet has the same vlan id */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, (numTriggers & BIT_2) >> 2,
                "\n   vlan id of captured packet must be: %X",
                                       (vfdArray[2].patternPtr[0] << 8) | vfdArray[2].patternPtr[1]);

        /* check if captured packet has the same ethertype */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, (numTriggers & BIT_3) >> 3,
                "\n   ethertype of captured packet must be: %02X",
                                       (vfdArray[3].patternPtr[0] << 8) | vfdArray[3].patternPtr[1]);

        /* check if captured packet has the same ipv4 version */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, (numTriggers & BIT_4) >> 4,
                "\n   2 bytes of ipv4 version and TOS of captured packet must be: %X",
                                       (vfdArray[4].patternPtr[0] << 8) | vfdArray[4].patternPtr[1]);

        /* check if captured packet has the same ipv4 total length */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, (numTriggers & BIT_5) >> 5,
                "\n   ipv4 total length of captured packet must be: %X",
                                       (vfdArray[5].patternPtr[0] << 8) | vfdArray[5].patternPtr[1]);

        /* prepare next iteration */
        numTriggers = numTriggers >> PRV_TGF_VFD_NUM_CNS;

    }

    if(useDscpRemapOnTs == GT_TRUE &&
       (errata_testNum <= 3))/* this recursive will do errata_testNum 1..3 */
    {
        /*this function increment errata_testNum */
        prvTgfTunnelDscpRemarkSet(PRV_TGF_DESTINATION_EPORT_CNS , GT_TRUE);

        /* run this function again (recursive) */
        prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportTrafficGenerate();
    }

}

/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: disable ingress policy on port 0 */
    rc = prvTgfPclPortIngressPolicyEnable(
                prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfPclPortIngressPolicyEnable: %d", prvTgfDevNum);

    /* AUTODOC: invalidate PCL rule 1 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PRV_TGF_PCL_RULE_IDX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                "prvTgfPclRuleValidStatusSet: %d, %d", prvTgfDevNum, GT_TRUE);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* Invalidate VLAN entry 6 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_6_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /* AUTODOC: invalidate VLAN entry 5 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /* AUTODOC: restore ePort 1000 mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT_CNS,
                                                           &(prvTgfRestoreCfg.physicalInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);


    if(useDscpRemapOnTs == GT_TRUE)
    {
        /* AUTODOC: restore ePort 1000 DSCP remark settings */
        prvTgfTunnelDscpRemarkSet(PRV_TGF_DESTINATION_EPORT_CNS , GT_FALSE);
    }

    errata_testNum = 0;
}


