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
* @file prvTgfTunnelTermEtherOverIpv4GreRedirectToEgress.c
*
* @brief Tunnel term Ethernet over Ipv4 GRE redirect to egress
*
* @version   23
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <utf/private/prvUtfExtras.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfDitGen.h>
#include <common/tgfPacketGen.h>
#include <tunnel/prvTgfTunnelTermEtherOverIpv4GreRedirectToEgress.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_VLANID_5_CNS            5

/* default VLAN Id */
#define PRV_TGF_VLANID_6_CNS            6

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number to receive traffic from */
#define PRV_TGF_EGR_PORT_IDX_CNS        1

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

/* default tunnel term entry index */
#define PRV_TGF_TTI_INDEX_CNS           2

/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 3;

/* Ethernet over IPv4 */
TGF_PACKET_STC prvTgfEthernetOverIpv4PacketInfo;

/* Global configuration IPv4 or IPv6 Total Length Deduction Value */
#define PRV_TGF_MACSEC_DEDUCTION_VALUE_CNS  24

/* default value for ipv4 total length field */
#define PRV_TGF_IPV4_TOTAL_LENGTH_VALUE_CNS 0x58

/* Wether packet is expected to be trapped in CPU */
GT_BOOL isPacketTrappedToCPU = GT_FALSE;

/* Wether expected packet is tagged */
GT_BOOL passengerIsTagged = GT_FALSE;

/* Wether this is macSec test */
GT_BOOL isMacSecTest = GT_FALSE;

/*************************** Restore config ***********************************/
/* flags to specify first call of saving defaults */
static GT_BOOL firstCallToSaveMacSecValues = GT_TRUE;
static GT_BOOL firstCallToSaveEthTypeValues = GT_TRUE;
static GT_BOOL firstCallToSaveMacModeValue = GT_TRUE;

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_TTI_MAC_MODE_ENT                    macMode;
    GT_U32                                      ethType1;
    GT_U32                                      ethType2;
    PRV_TGF_INLIF_PORT_MODE_ENT                 inlifMode;
    PRV_TGF_DIT_VPWS_TARGET_TAGGED_MODE_ENT     vpwsTargetTaggedMode;
    GT_BOOL                       macSecEnable;
    GT_U32                        macSecLengthDeductionValue;
} prvTgfRestoreCfg;

/* parts of packet TGF_PACKET_WILDCARD_STC */
static GT_U8 zeroArr1[] = { 0x00, 0x00, 0x00, 0x00 };
static GT_U8 zeroArr2[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*
* @param[in] isTaggedVlansUsed        - to mark vlans tagged or not
*                                       None
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressBridgeConfigSet
(
    IN GT_BOOL isTaggedVlansUsed
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       tagArray[] = {0, 0};
    TGF_PACKET_L2_STC *l2PartPtr;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* add tags */
    tagArray[0] = (GT_U8)isTaggedVlansUsed;
    tagArray[1] = (GT_U8)isTaggedVlansUsed;

    passengerIsTagged = isTaggedVlansUsed;

    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with untagged\tagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS, prvTgfPortsArray,
                                           NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with untagged\tagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_6_CNS, prvTgfPortsArray + 2,
                                           NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* use L2 part */
    l2PartPtr = prvTgfEthernetOverIpv4PacketInfo.partsArray[0].partPtr;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, vid 5 */
    rc = prvTgfBrgDefFdbMacEntryOnVidSet(l2PartPtr->daMac,
                                         PRV_TGF_VLANID_5_CNS, PRV_TGF_VLANID_5_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);
    
}

/**
* @internal prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressInilifConfigurationSet function
* @endinternal
*
* @brief   Set inlif configuration
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressInilifConfigurationSet
(
    GT_VOID
)
{
    /* AUTODOC: Set inlif configuration on port 0 */
    prvTgfTunnelTermInilifConfigurationSet(PRV_TGF_SEND_PORT_IDX_CNS,&prvTgfRestoreCfg.inlifMode);
}

/**
* @internal prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressTtiConfigSet function
* @endinternal
*
* @brief   Set TTI test settings:
*         - Enable port 0 for Eth lookup
*         - Set Eth key lookup MAC mode to Mac DA
*         - Set TTI rule action
*         - Set TTI rule
* @param[in] ttHeaderLength           - Tunnel header length in units of Bytes.
*                                      Qranularity is in 2 Bytes.
* @param[in] passengerParsingOfTransitNonMplsTransitTunnelEnable -
*                                      Whether to parse the passenger of transit tunnel
*                                      packets other than MPLS.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressTtiConfigSet
(
    GT_U32      ttHeaderLength,
    GT_BOOL     passengerParsingOfTransitNonMplsTransitTunnelEnable
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_MAC_MODE_ENT macMode = PRV_TGF_TTI_MAC_MODE_DA_E;
    PRV_TGF_TTI_RULE_UNT     pattern;
    PRV_TGF_TTI_RULE_UNT     mask;
    PRV_TGF_TTI_ACTION_2_STC ruleAction;
    PRV_TGF_TTI_IPV4_RULE_DEFAULT_INFO_STC defaultInfo;
    GT_U32  ethType1 = 0x6558;
    GT_U32  ethType2 = 0x6677;
    GT_U32  ethTypeGet1;
    GT_U32  ethTypeGet2;
    TGF_PACKET_L2_STC   *l2PartPtr;
    TGF_PACKET_IPV4_STC *ipv4PartPrt;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* clear pattern and mask */
    cpssOsMemSet((GT_VOID*) &pattern, 0, sizeof(pattern));
    cpssOsMemSet((GT_VOID*) &mask,    0, sizeof(mask));
    cpssOsMemSet((GT_VOID*) &ruleAction, 0, sizeof(ruleAction));

    /* AUTODOC: enable the TTI lookup for TTI_KEY_IPV4 at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
    {
        /* if this is Puma device */

        if (firstCallToSaveEthTypeValues == GT_TRUE)
        {        
            /* save the current gre ethernet type 1 */
            rc = prvTgfTtiIpv4GreEthTypeGet(prvTgfDevNum, PRV_TGF_TUNNEL_GRE0_ETHER_TYPE_E,
                                          &prvTgfRestoreCfg.ethType1);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiIpv4GreEthTypeGet: %d", prvTgfDevNum);

             /* save the current gre ethernet type 2 */
            rc = prvTgfTtiIpv4GreEthTypeGet(prvTgfDevNum, PRV_TGF_TUNNEL_GRE1_ETHER_TYPE_E,
                                          &prvTgfRestoreCfg.ethType2);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiIpv4GreEthTypeGet: %d", prvTgfDevNum);
   
            firstCallToSaveEthTypeValues = GT_FALSE;
        }
        /* AUTODOC: for PRV_TGF_TUNNEL_GRE0_ETHER_TYPE_E set ethertype to 0x6558 */
        rc = prvTgfTtiIpv4GreEthTypeSet(prvTgfDevNum,
                                      PRV_TGF_TUNNEL_GRE0_ETHER_TYPE_E, ethType1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiIpv4GreEthTypeSet: %d",
                                     prvTgfDevNum);

        /* get gre ethernet type 1 */
        rc = prvTgfTtiIpv4GreEthTypeGet(prvTgfDevNum,
                                      PRV_TGF_TUNNEL_GRE0_ETHER_TYPE_E, &ethTypeGet1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiIpv4GreEthTypeGet: %d",
                                     prvTgfDevNum);

        /* AUTODOC: read and verify ethType 1 */
        UTF_VERIFY_EQUAL0_STRING_MAC(ethType1, ethTypeGet1, "wrong ethType1 get");

        /* AUTODOC: for PRV_TGF_TUNNEL_GRE1_ETHER_TYPE_E set ethertype to 0x6677 */
        rc = prvTgfTtiIpv4GreEthTypeSet(prvTgfDevNum,
                                      PRV_TGF_TUNNEL_GRE1_ETHER_TYPE_E, ethType2);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiIpv4GreEthTypeSet: %d",
                                     prvTgfDevNum);

        /* get gre ethernet type 2 */
        rc = prvTgfTtiIpv4GreEthTypeGet(prvTgfDevNum,
                                      PRV_TGF_TUNNEL_GRE1_ETHER_TYPE_E, &ethTypeGet2);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiIpv4GreEthTypeGet: %d",
                                     prvTgfDevNum);

       /* AUTODOC: read and verify ethType 2 */
        UTF_VERIFY_EQUAL0_STRING_MAC(ethType2, ethTypeGet2, "wrong ethType2 get");
    }
    else
    {
        if (firstCallToSaveEthTypeValues == GT_TRUE)
        {    
            /* save the current gre ethernet type 1 */
            rc = prvTgfTtiEthernetTypeGet(prvTgfDevNum, PRV_TGF_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E,
                                          &prvTgfRestoreCfg.ethType1);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEthernetTypeGet: %d", prvTgfDevNum);

            /* save the current gre ethernet type 2 */
            rc = prvTgfTtiEthernetTypeGet(prvTgfDevNum, PRV_TGF_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE1_E,
                                          &prvTgfRestoreCfg.ethType2);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEthernetTypeGet: %d", prvTgfDevNum);

            firstCallToSaveEthTypeValues = GT_FALSE;
        }
        /* AUTODOC: for TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0 set ethertype to 0x6558 */
        rc = prvTgfTtiEthernetTypeSet(prvTgfDevNum,
                                      PRV_TGF_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E, ethType1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEthernetTypeSet: %d",
                                     prvTgfDevNum);

        /* get gre ethernet type 1 */
        rc = prvTgfTtiEthernetTypeGet(prvTgfDevNum,
                                      PRV_TGF_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E, &ethTypeGet1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEthernetTypeGet: %d",
                                     prvTgfDevNum);

        /* AUTODOC: read and verify ethType 1 */
        UTF_VERIFY_EQUAL0_STRING_MAC(ethType1, ethTypeGet1, "wrong ethType1 get");

        /* AUTODOC: for TUNNEL_ETHERTYPE_TYPE_IPV4_GRE1 set ethertype to 0x6677 */
         rc = prvTgfTtiEthernetTypeSet(prvTgfDevNum,
                                      PRV_TGF_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE1_E, ethType2);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEthernetTypeSet: %d",
                                     prvTgfDevNum);

        /* get gre ethernet type 2 */
        rc = prvTgfTtiEthernetTypeGet(prvTgfDevNum,
                                      PRV_TGF_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE1_E, &ethTypeGet2);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEthernetTypeGet: %d",
                                     prvTgfDevNum);

        /* AUTODOC: read and verify ethType 2 */
        UTF_VERIFY_EQUAL0_STRING_MAC(ethType2, ethTypeGet2, "wrong ethType2 get");
    }


    if(firstCallToSaveMacModeValue == GT_TRUE)
    {
        /* save the current lookup Mac mode for IPv4 and DA */
        rc = prvTgfTtiMacModeGet(prvTgfDevNum, PRV_TGF_TTI_KEY_IPV4_E, &prvTgfRestoreCfg.macMode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeGet: %d", prvTgfDevNum);

        firstCallToSaveMacModeValue = GT_FALSE;
    }

    /* AUTODOC: set MAC_MODE_DA for TTI_KEY_IPV4 */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_IPV4_E, macMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* AUTODOC: configure default TTI action values */
    rc = prvTgfTtiDefaultAction2Get(&ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiDefaultAction2Get");

    /* AUTODOC: configure specific TTI action values */
    ruleAction.ttPassengerPacketType   = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
    ruleAction.redirectCommand = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ruleAction.egressInterface.type            = CPSS_INTERFACE_PORT_E;
    ruleAction.egressInterface.devPort.hwDevNum  = prvTgfDevNum;
    ruleAction.egressInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    if (ttHeaderLength)
    {
        ruleAction.ttHeaderLength = ttHeaderLength;
    }
    if (passengerParsingOfTransitNonMplsTransitTunnelEnable)
    {
        ruleAction.tunnelTerminate       = GT_FALSE;
        ruleAction. passengerParsingOfTransitNonMplsTransitTunnelEnable = passengerParsingOfTransitNonMplsTransitTunnelEnable;
        ruleAction.ttPassengerPacketType = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
    }

    /* use L2 part */
    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_L2_E, &prvTgfEthernetOverIpv4PacketInfo, (GT_VOID *)&l2PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    /* use ipv4 part */
    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_IPV4_E, &prvTgfEthernetOverIpv4PacketInfo, (GT_VOID *)&ipv4PartPrt);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    cpssOsMemCpy((GT_VOID*)defaultInfo.common.mac.arEther,
                 (GT_VOID*)l2PartPtr->daMac,
                 sizeof(GT_ETHERADDR));
    defaultInfo.common.srcPort = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    defaultInfo.common.vid = PRV_TGF_VLANID_5_CNS;
    cpssOsMemCpy(defaultInfo.destIp.arIP, ipv4PartPrt->dstAddr,
                 sizeof(GT_IPADDR));
    cpssOsMemCpy(defaultInfo.srcIp.arIP, ipv4PartPrt->srcAddr,
                 sizeof(GT_IPADDR));

    /* AUTODOC: configure default IPv4 pattern and mask values */
    rc = prvTgfTtiDefaultIpv4PatternMaskGet(defaultInfo, &pattern.ipv4, &mask.ipv4);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiDefaultIpv4PatternMaskGet");

    /* AUTODOC: configure spesific IPv4 pattern and mask values */
    pattern.ipv4.tunneltype = 4;

    /* AUTODOC: add TTI rule 2 with: */
    /* AUTODOC:   key IPv4, cmd FORWARD, redirectCmd REDIRECT_TO_EGRESS */
    /* AUTODOC:   pattern pclId=1, srcPort=0, vid=5, tunneltype=4 */
    /* AUTODOC:   pattern MAC=00:00:00:00:34:02 */
    /* AUTODOC:   pattern srcIp=10.10.10.10, dstIp=4.4.4.4 */
    /* AUTODOC:   action Tunnel Terminate, PASSENGER_ETHERNET_NO_CRC, port=1 */
    /* AUTODOC:   tag0VlanCmd=DO_NOT_MODIFY, vid=0, tag1VlanCmd=MODIFY_ALL */
    rc = prvTgfTtiRule2Set(PRV_TGF_TTI_INDEX_CNS, PRV_TGF_TTI_KEY_IPV4_E,
                          &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");

    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {         
        if (firstCallToSaveMacSecValues == GT_TRUE)
        {  
            /* AUTODOC: save MACSec Total Length Deduction Enable State */
            rc = prvTgfTtiPortIpTotalLengthDeductionEnableGet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfRestoreCfg.macSecEnable);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortIpTotalLengthDeductionEnableGet");

            /* AUTODOC: save MACSec Total Length Deduction Value */
            rc = prvTgfTtiIpTotalLengthDeductionValueGet(prvTgfDevNum,CPSS_IP_PROTOCOL_IPV4_E,&prvTgfRestoreCfg.macSecLengthDeductionValue);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiIpTotalLengthDeductionValueGet");

            firstCallToSaveMacSecValues=GT_FALSE;
        }
    }


    /* set 'trap' of exception */
    rc = prvTgfTtiExceptionCmdSet(prvTgfDevNum, PRV_TGF_TTI_EXCEPTION_IPV4_HEADER_ERROR_E, CPSS_PACKET_CMD_TRAP_TO_CPU_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCmdSet");

}

/**
* @internal prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressBuildPacketSet function
* @endinternal
*
* @brief   Build packet
*
* @param[in] testNum                  - special parameters for the test
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
*       TGF_PACKET_PART_IPV4_E
*       TGF_PACKET_PART_PAYLOAD_E (GRE)
*       passenger:
*       TGF_PACKET_PART_L2_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_PAYLOAD_E
*
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressBuildPacketSet
(
    GT_U32  testNum
)
{
    TGF_PACKET_STC          packetInfo;
    GT_STATUS               rc = GT_OK;
    TGF_PACKET_IPV4_STC     *ipv4PartPtr;
    TGF_PACKET_WILDCARD_STC *headerLengthPartPtr;
    if (testNum == 1)
    {
        prvTgfPacketTunnelHeaderForceTillPassengersSet(4);
    }
    else if (testNum == 2 || testNum == 3)
    {
        prvTgfPacketTunnelHeaderForceTillPassengersSet(6);
    }
    /* AUTODOC: get default Eth over IPv4 packet */
    rc = prvTgfPacketEthOverIpv4PacketDefaultPacketGet(&packetInfo.numOfParts, &packetInfo.partsArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthOverIpv4PacketDefaultPacketGet");

    /* change ipv4 part */
    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_IPV4_E, &packetInfo, (GT_VOID *)&ipv4PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    ipv4PartPtr->totalLen = PRV_TGF_IPV4_TOTAL_LENGTH_VALUE_CNS;
    ipv4PartPtr->protocol = TGF_IPV4_GRE_PROTOCOL_CNS;

    if (isMacSecTest)
    {
        ipv4PartPtr->totalLen += PRV_TGF_MACSEC_DEDUCTION_VALUE_CNS;
    }

    /* AUTODOC: changed Ipv4 of tunnel */
    rc = prvTgfPacketEthOverIpv4PacketHeaderDataSet(TGF_PACKET_PART_IPV4_E,GT_FALSE,0,ipv4PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthOverIpv4PacketHeaderDataSet");
    if (testNum == 1)
    {
        /* AUTODOC: change changed number of 'zero' in header length */
        rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_TUNNEL_HEADER_FORCE_TILL_PASSENGER_E, &packetInfo, (GT_VOID *)&headerLengthPartPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

        headerLengthPartPtr->bytesPtr = zeroArr1;
        headerLengthPartPtr->numOfBytes = sizeof(zeroArr1);

        rc = prvTgfPacketEthOverIpv4PacketHeaderDataSet(TGF_PACKET_PART_TUNNEL_HEADER_FORCE_TILL_PASSENGER_E,GT_FALSE,0,headerLengthPartPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthOverIpv4PacketHeaderDataSet");
    }
    if (testNum == 2)
    {
        /* AUTODOC: change changed number of 'zero' in header length */
        rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_TUNNEL_HEADER_FORCE_TILL_PASSENGER_E, &packetInfo, (GT_VOID *)&headerLengthPartPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

        headerLengthPartPtr->bytesPtr = zeroArr2;
        headerLengthPartPtr->numOfBytes = sizeof(zeroArr2);

        rc = prvTgfPacketEthOverIpv4PacketHeaderDataSet(TGF_PACKET_PART_TUNNEL_HEADER_FORCE_TILL_PASSENGER_E,GT_FALSE,0,headerLengthPartPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthOverIpv4PacketHeaderDataSet");
    }

    prvTgfEthernetOverIpv4PacketInfo.numOfParts = packetInfo.numOfParts;
    prvTgfEthernetOverIpv4PacketInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    prvTgfEthernetOverIpv4PacketInfo.partsArray =  packetInfo.partsArray;
}

/**
* @internal prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressCheckCaptureEgressTrafficOnPort function
* @endinternal
*
* @brief   check captured egress on specific port of the test.
*
* @param[in] passengerIsTagged        - wether the passenger expected packet is tagged
* @param[in] isTerminated             - wether the packet is terminated
*                                       None
*/
static void prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressCheckCaptureEgressTrafficOnPort
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

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    PRV_UTF_LOG1_MAC("check Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* copy the expected packet from the ingress packet */
    expectedPacketInfo.numOfParts = prvTgfEthernetOverIpv4PacketInfo.numOfParts;
    expectedPacketInfo.totalLen = prvTgfEthernetOverIpv4PacketInfo.totalLen;
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
        expectedPacketInfo.partsArray[ii] = prvTgfEthernetOverIpv4PacketInfo.partsArray[ii];
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
* @internal prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:01,
* @param[in] testNum                  - test number
*                                       None
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressTrafficGenerate
(
    GT_U32 testNum
)
{
    GT_STATUS       rc = GT_OK;
    GT_U32          portsCount  = PRV_TGF_PORT_COUNT_CNS;
    GT_U32          portIter    = 0;
    GT_U8           devNum   = 0;
    GT_U8           queue    = 0;
    GT_BOOL         getFirst = GT_TRUE;
    GT_U32          buffLen  = 0x600;
    GT_U32          packetActualLength = 0;
    GT_U8           packetBuff[0x600] = {0};
    TGF_NET_DSA_STC rxParam;
    GT_U32          length = 0;
    GT_BOOL         isTerminated = GT_TRUE;

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

    /* AUTODOC: send 3 Ethernet over IPv4 tunneled packets from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11, VID=5 */
    /* AUTODOC:   srcIP=10.10.10.10, dstIP=4.4.4.4 */
    /* AUTODOC:   GRE=00:00:65:58 */
    /* AUTODOC:   passenger DA=00:00:00:00:00:22, SA=00:00:00:00:00:25, VID=5 */

    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    &prvTgfEthernetOverIpv4PacketInfo, prvTgfBurstCount, 0, NULL,
                    prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    if (testNum == 0)
    {
        if (isPacketTrappedToCPU)
        {
            /* Get entry from captured packet's table */
            rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                               getFirst, GT_TRUE, packetBuff,
                                               &buffLen, &packetActualLength,
                                               &devNum, &queue,
                                               &rxParam);

            rc = tgfTrafficEnginePacketLengthCalc(&prvTgfEthernetOverIpv4PacketInfo, &length);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficEnginePacketLengthCalc");

            UTF_VERIFY_EQUAL0_STRING_MAC(length, packetActualLength, "Number of expected packets is wrong - \n");

            return;
        }
    }
    if (testNum == 3)
    {
        isTerminated = GT_FALSE;
    }

    /* check captured egress traffic on port */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressCheckCaptureEgressTrafficOnPort(passengerIsTagged,isTerminated);

    return;
}

/**
* @internal prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;


    /* AUTODOC: RESTORE CONFIGURATION: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: invalidate TTI rule 2 */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: invalidate VLAN entry 6 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_6_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /* AUTODOC: invalidate VLAN entry 5 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /* AUTODOC: restore default inlif mode */
    rc = prvTgfInlifPortModeSet(PRV_TGF_SEND_PORT_IDX_CNS, prvTgfRestoreCfg.inlifMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfInlifPortModeSet: %d", prvTgfDevNum);

    /* AUTODOC: restore MAC_MODE_DA for TTI_KEY_IPV4 */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_IPV4_E, prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    firstCallToSaveMacModeValue = GT_TRUE;

    /* AUTODOC: disable TTI lookup for TTI_KEY_IPV4 at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
    {
        /* if this is Puma device */

        /* AUTODOC: restore GRE ethernet type 1 */
        rc = prvTgfTtiIpv4GreEthTypeSet(prvTgfDevNum,
                                      PRV_TGF_TUNNEL_GRE0_ETHER_TYPE_E,  prvTgfRestoreCfg.ethType1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiIpv4GreEthTypeSet: %d",
                                     prvTgfDevNum);

        /* AUTODOC: restore GRE ethernet type 2 */
         rc = prvTgfTtiIpv4GreEthTypeSet(prvTgfDevNum,
                                      PRV_TGF_TUNNEL_GRE1_ETHER_TYPE_E, prvTgfRestoreCfg.ethType2);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiIpv4GreEthTypeSet: %d",
                                     prvTgfDevNum);

        firstCallToSaveEthTypeValues = GT_TRUE;
    }
    else
    {
        /* AUTODOC: restore GRE ethernet type 1 */
        rc = prvTgfTtiEthernetTypeSet(prvTgfDevNum,
                                      PRV_TGF_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E, prvTgfRestoreCfg.ethType1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEthernetTypeSet: %d",
                                     prvTgfDevNum);

        /* AUTODOC: restore GRE ethernet type 2 */
        rc = prvTgfTtiEthernetTypeSet(prvTgfDevNum,
                                      PRV_TGF_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE1_E, prvTgfRestoreCfg.ethType2);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEthernetTypeSet: %d",
                                     prvTgfDevNum);

        firstCallToSaveEthTypeValues = GT_TRUE;
    }

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
    {
        /* if this is Puma device */
        /* AUTODOC: default target tagged mode for VPWS packets */
        rc = prvTgfDitVpwsTargetTaggedModeSet(prvTgfDevNum,prvTgfRestoreCfg.vpwsTargetTaggedMode);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfDitVpwsTargetTaggedModeSet");
    }

    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        /* AUTODOC: restore MACSec Total Length Deduction Enable State */
        rc = prvTgfTtiPortIpTotalLengthDeductionEnableSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfRestoreCfg.macSecEnable);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortIpTotalLengthDeductionEnableSet");

        /* AUTODOC: restore MACSec Total Length Deduction Value */
        rc = prvTgfTtiIpTotalLengthDeductionValueSet(prvTgfDevNum,CPSS_IP_PROTOCOL_IPV4_E,prvTgfRestoreCfg.macSecLengthDeductionValue);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiIpTotalLengthDeductionValueGet");

        firstCallToSaveMacSecValues=GT_TRUE;
    }

    /* restore 'hard drop' */
    rc = prvTgfTtiExceptionCmdSet(prvTgfDevNum, PRV_TGF_TTI_EXCEPTION_IPV4_HEADER_ERROR_E, CPSS_PACKET_CMD_DROP_HARD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCmdSet");

    isPacketTrappedToCPU = GT_FALSE;
    isMacSecTest = GT_FALSE;

    prvTgfPacketRestoreDefaultParameters();

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
}

/**
* @internal prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressMacSecConfigSet function
* @endinternal
*
* @brief   Set TTI test settings:
*         - Enable port 0 for Eth lookup
*         - Set Eth key lookup MAC mode to Mac DA
*         - Set TTI rule action
*         - Set TTI rule
* @param[in] includeMacSecConfig      - whether to include MACSec configuration
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressMacSecConfigSet
(
    GT_BOOL includeMacSecConfig
)
{
    GT_STATUS                rc = GT_OK;

    /* AUTODOC: set MACSec Total Length Deduction Enable State = GT_FALSE */
    rc = prvTgfTtiPortIpTotalLengthDeductionEnableSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortIpTotalLengthDeductionEnableSet");

    /* AUTODOC: set MACSec Total Length Deduction Value = 0*/
    rc = prvTgfTtiIpTotalLengthDeductionValueSet(prvTgfDevNum,CPSS_IP_PROTOCOL_IPV4_E,0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiIpTotalLengthDeductionValueSet");

    isPacketTrappedToCPU = GT_TRUE;
    isMacSecTest = GT_TRUE;

    if (includeMacSecConfig)
    {
        /* AUTODOC: set MACSec Total Length Deduction Enable State = GT_TRUE */
        rc = prvTgfTtiPortIpTotalLengthDeductionEnableSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortIpTotalLengthDeductionEnableSet");

        /* AUTODOC: set MACSec Total Length Deduction Value */
        rc = prvTgfTtiIpTotalLengthDeductionValueSet(prvTgfDevNum,CPSS_IP_PROTOCOL_IPV4_E,PRV_TGF_MACSEC_DEDUCTION_VALUE_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiIpTotalLengthDeductionValueSet");

        isPacketTrappedToCPU = GT_FALSE;
    }
}


