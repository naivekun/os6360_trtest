/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfFdbAging.c
*
* DESCRIPTION:
*       Bridge Generic APIs UT.
*
* FILE REVISION NUMBER:
*       $Revision: 20 $
*
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <bridge/prvTgfBrgGen.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS  2
/* the vlan ID that the from CPU uses ... WA to Erratum of device ... */
#define FROM_CPU_VLANID_CNS 4094
/* we have erratum : The ‘FROM_CPU’ packet is subject to ‘L2i filters’ (on my case ‘vlan not valid’) when “Bridge Bypass Mode" is in mode : "Bypass Forwarding Decision only"*/
static GT_BOOL isErratumForFromCpuForBridgeBypassForwardingDecisionOnlyMode = GT_TRUE;

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/******************************* Test packets **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x01, 0x80, 0xc2, 0x00, 0x00, 0x00},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                /* srcMac */
};

static TGF_PACKET_L2_STC prvTgfBypassPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0xBB},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0xAA}                 /* saMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    0x79D4,             /* csum */
    { 0,  0,  0,  0},   /* srcAddr */
    { 0,  0,  0,  2}    /* dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* DATA of bypass packet */
static GT_U8 prvTgfBypassPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

/* Bypass PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfBypassPacketPayloadPart = {
    sizeof(prvTgfBypassPayloadDataArr),                       /* dataLength */
    prvTgfBypassPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of Bypass packet */
static TGF_PACKET_PART_STC prvTgfBypassPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfBypassPacketL2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfBypassPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/* LENGTH of bypass packet */
#define PRV_TGF_BYPASS_PACKET_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfBypassPayloadDataArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_BYPASS_PACKET_CRC_LEN_CNS  (PRV_TGF_BYPASS_PACKET_LEN_CNS + TGF_CRC_LEN_CNS)

static TGF_PACKET_STC prvTgfBypassPacketInfo = {
    PRV_TGF_BYPASS_PACKET_CRC_LEN_CNS,                                   /* totalLen */
    sizeof(prvTgfBypassPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfBypassPacketPartArray                                        /* partsArray */
};


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfBrgGenIeeeReservedMcastProfilesTestInit function
* @endinternal
*
* @brief   Set VLAN entry.
*
* @param[in] vlanId                   -  to be configured
*                                       None
*/
static GT_VOID prvTgfBrgGenIeeeReservedMcastProfilesTestInit
(
    IN GT_U16           vlanId
)
{
    GT_U32                      portIter  = 0;
    GT_U32                      portCount = 0;
    GT_STATUS                   rc        = GT_OK;
    CPSS_PORTS_BMP_STC          portsMembers;
    CPSS_PORTS_BMP_STC          portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;

    /* clear entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));

    /* set vlan entry */
    vlanInfo.unkSrcAddrSecBreach  = GT_FALSE;
    vlanInfo.unregNonIpMcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv6McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unkUcastCmd          = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4BcastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.ipv4IgmpToCpuEn      = GT_FALSE;
    vlanInfo.mirrToRxAnalyzerEn   = GT_FALSE;
    vlanInfo.ipv6IcmpToCpuEn      = GT_FALSE;
    vlanInfo.ipCtrlToCpuEn        = PRV_TGF_BRG_IP_CTRL_NONE_E;
    vlanInfo.ipv4IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv6IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv4IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6SiteIdMode       = CPSS_IP_SITE_ID_INTERNAL_E;
    vlanInfo.ipv4UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv4McastRouteEn     = GT_FALSE;
    vlanInfo.ipv6UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv6McastRouteEn     = GT_FALSE;
    vlanInfo.stgId                = 0;
    vlanInfo.autoLearnDisable     = GT_TRUE;/* working in controlled learning */
    vlanInfo.naMsgToCpuEn         = GT_TRUE;/* working in controlled learning */
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    vlanInfo.floodVidx            = 0xFFF;
    vlanInfo.floodVidxMode        = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    vlanInfo.portIsolationMode    = PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;
    vlanInfo.fidValue              = vlanId;

    /* set vlan entry */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    portCount = prvTgfPortsNum;
    for (portIter = 0; portIter < portCount; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,prvTgfPortsArray[portIter]);

        if(vlanId == FROM_CPU_VLANID_CNS)
        {
            /* as this vlan is usually never valid , we unaware that if valid we
               need to allow the tag after the DSA to egress the device ...
               tagged/untagged packet */
            portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] = PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E;
        }
        else
        {
            portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] = PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;
        }
        /* reset counters and set force link up */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 vlanId,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d", vlanId);
}

/**
* @internal prvTgfBrgGenIeeeReservedMcastProfilesTestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] portNum                  - port number
* @param[in] notIngressPort           - VLAN member but not ingress port
*                                       None
*/
static GT_VOID prvTgfBrgGenIeeeReservedMcastProfilesTestPacketSend
(
    IN GT_U32 portNum,
    IN GT_U32 notIngressPort,
    IN GT_U32 expectPacketNum
)
{
    GT_STATUS       rc = GT_OK;
    GT_U8           devNum   = 0;
    GT_U8           queue    = 0;
    GT_BOOL         getFirst = GT_TRUE;
    GT_U32          buffLen  = 0x600;
    GT_U32          packetActualLength = 0;
    static  GT_U8   packetBuff[0x600] = {0};
    TGF_NET_DSA_STC rxParam;
    CPSS_INTERFACE_INFO_STC portInterface;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* setup portInterface for capturing.
      there is no packets should be captured on this port.
      because packet may be trapped or dropped on ingress one. */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = notIngressPort;

    /* enable capture on port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, portNum);

    /* send Packet from port 0 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, portNum);

    /* disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, portNum);

    /* Get entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       getFirst, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue,
                                       &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_PACKET_CRC_LEN_CNS * expectPacketNum, packetActualLength, "Number of expected packets is wrong\n");

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/**
* @internal prvTgfBrgGenIeeeReservedMcastProfilesConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Create VLAN 2 on all ports (0,8,18,23)
*         Set capture to CPU
*         BridgingModeSet = CPSS_BRG_MODE_802_1Q_E
*         GenIeeeReservedMcastTrapEnable = GT_TRUE
*         For all ports (0,8,18,23):
*         For all profileIndex (0...3):
*         IeeeReservedMcastProfileIndexSet (devNum = 0,
*         portNum,
*         profileIndex)
*         For all protocols (0...255)
*         IeeeReservedMcastProtCmdSet1 (dev = 0,
*         profileIndex,
*         protocol_iterator,
*         CPSS_PACKET_CMD_TRAP_TO_CPU_E)
*         Send Packet from to portNum.
*         IeeeReservedMcastProtCmdSet1 (dev = 0,
*         profileIndex,
*         protocol_iterator,
*         CPSS_PACKET_CMD_DROP_SOFT_E)
*         Success Criteria:
*         Fisrt packet is not captured on ports 0,8,18,23 but is captured in CPU.
*         Second packet is dropped.
*/
GT_VOID prvTgfBrgGenIeeeReservedMcastProfilesConfigurationSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: configure VLAN-aware mode */
    prvTgfBrgVlanBridgingModeSet(CPSS_BRG_MODE_802_1Q_E);

    /* AUTODOC: enable IEEE Reserved Mcast trap */
    prvTgfBrgGenIeeeReservedMcastTrapEnable(prvTgfDevNum, GT_TRUE);

    /* AUTODOC: create VLAN 2 with port 0,1,2,3 */
    prvTgfBrgGenIeeeReservedMcastProfilesTestInit(PRV_TGF_VLANID_CNS);
}

/**
* @internal prvTgfBrgGenIeeeReservedMcastProfilesTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to all ports the following IP4 packet:
*         000000   01 80 c2 00 00 03 00 00 00 00 00 11 81 00 00 02
*         000010   08 00 45 00 00 2a 00 00 00 00 40 ff 79 d4 00 00
*         000020   00 00 00 00 00 02 00 01 02 03 04 05 06 07 08 09
*         000030   0a 0b 0c 0d 0e 0f 10 11 12 13 14 15
*/
GT_VOID prvTgfBrgGenIeeeReservedMcastProfilesTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc;           /* returned status */
    GT_U8  portIter;        /* loop port iterator */
    GT_U32 protocolIter;    /* loop protocol iterator */
    GT_U32 profileIndex;    /* loop profile iterator */
    GT_U32  notIngressPort;  /* not ingress port */
    GT_U32 numOfProfiles;   /* number of profiles that the device supports : DX - 4 , Puma - 2 */
    GT_U32 numOfRanges;     /* number of ranges that the device supports :
                                DX - single range : 01-80-C2-00-00-00 - 01-80-C2-00-00-FF
                                Puma - 2 ranges :  01-80-C2-00-00-00 - 01-80-C2-00-00-0F
                                                   01-80-C2-00-00-20 - 01-80-C2-00-00-2F
                                */
    GT_U32 rangeIndex;     /* range index */
    GT_U32 protocolIterIncrement;/* increment value on each protocol iteration */
    GT_U32  startProtocol[2] = {0};/* first protocol number in each range */
    GT_U32  protocolsNum[2] = {0};/* number of protocols in each range */


    if(CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily))
    {
        numOfProfiles = 4;
        protocolIterIncrement = 0x20;
        numOfRanges  = 1;
        startProtocol[0] = 0;       /* 01-80-C2-00-00-00 - 01-80-C2-00-00-FF*/
        protocolsNum[0] = 0x100;
    }
    else/* Puma */
    {
        numOfProfiles = 2;
        protocolIterIncrement = 2;

        numOfRanges  = 2;
        startProtocol[0] = 0;       /* 01-80-C2-00-00-00 - 01-80-C2-00-00-0F*/
        protocolsNum[0] = 0x10;
        startProtocol[1] = 0x20;    /*01-80-C2-00-00-20 - 01-80-C2-00-00-2F*/
        protocolsNum[1] = 0x10;
    }

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: for all ports 0,1,2,3 do: */
#ifdef GM_USED
    portIter = 2;/* no loops on ports. --> reduce time */
    protocolIterIncrement *= 5;/* reduce time */
#else
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
#endif
    {
        /* use another port to check that packet was dropped (not forwarded ) */
        notIngressPort = prvTgfPortsArray[(portIter + 1)%prvTgfPortsNum];

        /* AUTODOC: for all profile Idx (0..3) do: */
#ifdef GM_USED
        profileIndex = numOfProfiles - 1;/* no loops on profiles. --> reduce time */
#else
        for(profileIndex = 0; profileIndex < numOfProfiles; profileIndex++)
#endif
        {
            /* AUTODOC: set profile indexes: */
            /* AUTODOC:   port 0 -> index (0,1,2,3) */
            /* AUTODOC:   port 1 -> index (0,1,2,3) */
            /* AUTODOC:   port 2 -> index (0,1,2,3) */
            /* AUTODOC:   port 3 -> index (0,1,2,3) */
            prvTgfBrgGenPortIeeeReservedMcastProfileIndexSet(prvTgfDevNum,
                                                             prvTgfPortsArray[portIter],
                                                             profileIndex);

            for(rangeIndex = 0; rangeIndex < numOfRanges; rangeIndex++)
            {
                for(protocolIter = startProtocol[rangeIndex];
                    protocolIter < (startProtocol[rangeIndex] + protocolsNum[rangeIndex]);
                    protocolIter += protocolIterIncrement)
                {
                    /* skip protocol 1 (PAUSE frame) */
                    if (protocolIter == 1)
                    {
                        continue;
                    }

                    /* AUTODOC: for each profile idx enable trap to the CPU IEEE reserved MC addresses in range 01:80:C2:00:00:[00..FF] */
                    prvTgfBrgGenIeeeReservedMcastProtCmdSet(prvTgfDevNum,
                                                            profileIndex,
                                                            (GT_U8)protocolIter,
                                                            CPSS_PACKET_CMD_TRAP_TO_CPU_E);

                    prvTgfPacketL2Part.daMac[5] = (GT_U8)protocolIter;
                    utfPrintKeepAlive();

                    /* AUTODOC: send IPv4 packets with DAs 01:80:C2:00:00:00..01:80:C2:00:00:FF from ports 0, 1, 2, 3 */
                    /* AUTODOC: verify packet is captured on CPU */
                    prvTgfBrgGenIeeeReservedMcastProfilesTestPacketSend(prvTgfPortsArray[portIter], notIngressPort, 1);


                    /* AUTODOC: set SOFT DROP command */
                    prvTgfBrgGenIeeeReservedMcastProtCmdSet(prvTgfDevNum,
                                                            profileIndex,
                                                            (GT_U8)protocolIter,
                                                            CPSS_PACKET_CMD_DROP_SOFT_E);

                    /* reset counters */
                    rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                                 prvTgfDevNum, prvTgfPortsArray[portIter]);

                    /* AUTODOC: send IPv4 packets with DAs 01:80:C2:00:00:00..01:80:C2:00:00:FF from port 0 */
                    /* AUTODOC: verify packet is not captured but dropped */
                    prvTgfBrgGenIeeeReservedMcastProfilesTestPacketSend(prvTgfPortsArray[portIter], notIngressPort, 0);

                    /* reset counters */
                    rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                                 prvTgfDevNum, prvTgfPortsArray[portIter]);
                }/*protocolIter*/
            }/*rangeIndex*/
        }/*profileIndex*/
    }/*portIter*/
}

/**
* @internal prvTgfBrgGenIeeeReservedMcastProfilesConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgGenIeeeReservedMcastProfilesConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /****************/
    /* Tables Reset */
    /****************/

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* AUTODOC: disable IEEE Reserved Mcast trap */
    prvTgfBrgGenIeeeReservedMcastTrapEnable(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenIeeeReservedMcastTrapEnable: %d",
                                 prvTgfDevNum);
}

/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* traffic generation sending port */
#define ING_PORT_IDX_CNS    1

/* PCL target port */
#define EGR_PCL_PORT_IDX_CNS    0

/* FDB target port */
#define EGR_FDB_PORT_IDX_CNS    2

/* PCL rule index */
static GT_U32   prvTgfPclRuleIndex = 1;

/* Save Bridge Bypass Mode for restore */
static PRV_TGF_BRG_GEN_BYPASS_MODE_ENT  prvTgfBridgeBypassMode;


/**
* @internal prvTgfTrafficPclRuleAndActionSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfTrafficPclRuleAndActionSet
(
    IN    GT_BOOL                       bypassBridgeEn
)
{
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
    PRV_TGF_PCL_ACTION_STC      action;

    /* mask for MAC address */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(mask.ruleStdNotIp.macDa.arEther, 0xFF,
                 sizeof(mask.ruleStdNotIp.macDa.arEther));

    /* pattern for MAC address */
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemCpy(pattern.ruleStdNotIp.macDa.arEther, prvTgfBypassPacketL2Part.daMac,
                 sizeof(prvTgfBypassPacketL2Part.daMac));

    /* action redirect */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd                                           = PRV_TGF_PACKET_CMD_FORWARD_E;
    action.mirror.cpuCode = 0xf8;
    action.redirect.redirectCmd                             = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    action.redirect.data.outIf.outInterface.type            = CPSS_INTERFACE_PORT_E;
    action.redirect.data.outIf.outInterface.devPort.hwDevNum  = prvTgfDevNum;
    action.redirect.data.outIf.outInterface.devPort.portNum =
        prvTgfPortsArray[EGR_PCL_PORT_IDX_CNS];

    if( GT_TRUE == bypassBridgeEn )
    {
        action.bypassBridge = GT_TRUE;
    }
    else
    {
        action.bypassBridge = GT_FALSE;
    }

    return prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
            prvTgfPclRuleIndex, &mask, &pattern, &action);
}

/**
* @internal prvTgfFdbMacLearningCheck function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfFdbMacLearningCheck
(
    IN TGF_MAC_ADDR macAddrArr
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_BRG_MAC_ENTRY_STC     macEntry;
    PRV_TGF_MAC_ENTRY_KEY_STC     macEntryKey;

    /* get FDB entry */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemSet(&macEntryKey, 0, sizeof(macEntryKey));
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;

    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, macAddrArr, sizeof(TGF_MAC_ADDR));

    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);

    /* Checking if there is MAC learning */
    if (rc != GT_OK)
    {
        PRV_UTF_LOG6_MAC("Device has NOT learned MAC SA %02X:%02X:%02X:%02X:%02X:%02X\n",
                macAddrArr[0],macAddrArr[1],macAddrArr[2],macAddrArr[3],macAddrArr[4],macAddrArr[5]);
        return GT_FAIL;
    }

    /* compare entryType */
    rc = (macEntry.key.entryType == PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E) ? GT_OK : GT_FAIL;
    PRV_UTF_VERIFY_GT_OK(rc, "(macEntry.key.entryType != MAC_ADDR_E");

    /* compare MAC address */
    rc = cpssOsMemCmp(macEntry.key.key.macVlan.macAddr.arEther,
                      macEntryKey.key.macVlan.macAddr.arEther,
                      sizeof(TGF_MAC_ADDR)) == 0 ? GT_OK : GT_FAIL;
    PRV_UTF_VERIFY_GT_OK(rc, "FDB has another MAC address");

    /* compare vlanId */
    rc = (macEntry.key.key.macVlan.vlanId == macEntryKey.key.macVlan.vlanId) ? GT_OK : GT_FAIL;
    PRV_UTF_VERIFY_GT_OK(rc, "FDB has another vlanid");

    /* compare dstInterface.type */
    rc = (macEntry.dstInterface.type == CPSS_INTERFACE_PORT_E) ? GT_OK : GT_FAIL;
    PRV_UTF_VERIFY_GT_OK(rc, "FDB has another dstInterface.type");

    /* compare dstInterface */
    if (macEntry.dstInterface.type == CPSS_INTERFACE_PORT_E) {
        rc = (macEntry.dstInterface.devPort.hwDevNum == prvTgfDevNum) ? GT_OK : GT_FAIL;
        PRV_UTF_VERIFY_GT_OK(rc, "FDB has another dstInterface.devPort.hwDevNum");
    }

    /* compare isStatic */
    rc = (macEntry.isStatic == GT_FALSE) ? GT_OK : GT_FAIL;
    if (rc == GT_OK)
        PRV_UTF_LOG6_MAC("Device has learned MAC SA %02X:%02X:%02X:%02X:%02X:%02X\n",
                macAddrArr[0],macAddrArr[1],macAddrArr[2],macAddrArr[3],macAddrArr[4],macAddrArr[5]);

    return rc;
};

/**
* @internal prvTgfBrgDefFdbMacEntryAndCommandOnPortSet function
* @endinternal
*
* @brief   Set FDB entry on port
*
* @param[in] macAddr                  - mac address
* @param[in] vlanId                   - vlan Id
* @param[in] dstDevNum                - destination device
* @param[in] dstPortNum               - destination port
* @param[in] daCommand                - destination command
* @param[in] isStatic                 - is the entry static/dynamic
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfBrgDefFdbMacEntryAndCommandOnPortSet
(
    IN TGF_MAC_ADDR                   macAddr,
    IN GT_U16                         vlanId,
    IN GT_U8                          dstDevNum,
    IN GT_PORT_NUM                    dstPortNum,
    IN PRV_TGF_PACKET_CMD_ENT         daCommand,
    IN GT_BOOL                        isStatic
)
{
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;

    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, macAddr, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId       = vlanId;
    macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = dstDevNum;
    macEntry.dstInterface.devPort.portNum = dstPortNum;
    macEntry.isStatic                     = isStatic;
    macEntry.daCommand                    = daCommand;

    return prvTgfBrgFdbMacEntrySet(&macEntry);
}

/**
* @internal prvTgfBrgGenBypassModeTestPacketSend function
* @endinternal
*
* @brief   Function sends packetand performs trace.
*
* @param[in] portInterfacePtr         - (pointer to) port interface (port only is legal param)
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfBrgGenBypassModeTestPacketSend
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN TGF_PACKET_STC           *packetInfoPtr
)
{
    GT_STATUS rc          = GT_OK;
    GT_U32    portsCount  = prvTgfPortsNum;
    GT_U32    portIter    = 0;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterfacePtr->devPort.hwDevNum, portInterfacePtr->devPort.portNum, GT_TRUE);

    /* send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS]);

    /* wait for packets come to CPU */
    (void) tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 500, NULL);

    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterfacePtr->devPort.hwDevNum, portInterfacePtr->devPort.portNum, GT_FALSE);

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterfacePtr->devPort.portNum);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfBrgGenBypassModeTestConfigurationSet function
* @endinternal
*
* @brief   Bridge Bypass mode initial configurations
*/
GT_VOID prvTgfBrgGenBypassModeTestConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 2 with port 0,1,2,3. */
    prvTgfBrgGenIeeeReservedMcastProfilesTestInit(PRV_TGF_VLANID_CNS);

    /* AUTODOC: flush FDB include static entries. */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_TRUE);

    /* AUTODOC: create a macEntry for bypass packet forwarding to port 2,
                packet command MIRROR. */
    rc = prvTgfBrgDefFdbMacEntryAndCommandOnPortSet(
            prvTgfBypassPacketL2Part.daMac, PRV_TGF_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[EGR_FDB_PORT_IDX_CNS],
            PRV_TGF_PACKET_CMD_MIRROR_TO_CPU_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefFdbMacEntryAndCommandOnPortSet: %d", prvTgfDevNum);

    /* init PCL Engine for send port */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[ING_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    /* AUTODOC: set PCL rule with bypass bridge DISABLED*/
    rc = prvTgfTrafficPclRuleAndActionSet(GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfTrafficPclRuleAndActionSet: %d, %d",
        prvTgfDevNum, GT_FALSE);

    /* Save Bridge Bypass Mode for restore */
    rc = prvTgfBrgGenBypassModeGet(prvTgfDevNum, &prvTgfBridgeBypassMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgGenBypassModeGet: %d, %d",
        prvTgfDevNum, prvTgfBridgeBypassMode);
}

/**
* @internal prvTgfBrgGenBypassModeTestConfigurationRestore function
* @endinternal
*
* @brief   Bridge Bypass mode restore configurations
*/
GT_VOID prvTgfBrgGenBypassModeTestConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: invalidate PCL rule */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     prvTgfPclRuleIndex,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 prvTgfPclRuleIndex,
                                 GT_FALSE);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_TRUE);

    /* AUTODOC: invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* restore Bridge Bypass Mode */
    rc = prvTgfBrgGenBypassModeSet(prvTgfBridgeBypassMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenBypassModeSet: %d",
                                 prvTgfBridgeBypassMode);

    if(isErratumForFromCpuForBridgeBypassForwardingDecisionOnlyMode == GT_TRUE)
    {
        /* invalidate vlan entry (and reset vlan entry) */
        rc = prvTgfBrgDefVlanEntryInvalidate(FROM_CPU_VLANID_CNS);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, FROM_CPU_VLANID_CNS);
    }
}

/**
* @internal prvTgfBrgGenBypassModeTest function
* @endinternal
*
* @brief   Bridge Bypass mode envelope
*/
GT_VOID prvTgfBrgGenBypassModeTest
(
    GT_VOID
)
{
    GT_STATUS                       rc;
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U32                          portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;

    GT_U8           packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32          buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32          packetActualLength = 0;
    GT_U8           devNum;
    GT_U8           queue;
    TGF_NET_DSA_STC rxParam;

    GT_STATUS   expectedRcRxToCpu = GT_OK;

    if(GT_TRUE == prvUtfIsGmCompilation())
    {
        /* the GM behave different then the HW !!! */
        /* the WM was fixed to be like HA */
        isErratumForFromCpuForBridgeBypassForwardingDecisionOnlyMode = GT_FALSE;
    }
    else
    {
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum))
        {
             /* In BC2 B0 errata has fixed */
             isErratumForFromCpuForBridgeBypassForwardingDecisionOnlyMode = GT_FALSE;
        }
    }

    prvTgfBrgGenBypassModeTestConfigurationSet();

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_FDB_PORT_IDX_CNS];

    /* packet for FDB forwarding - no bridge bypass, exit on port 2 (FDB), */
    /*                             mirroring expected.                     */

    /* AUTODOC: send packet. */
    prvTgfBrgGenBypassModeTestPacketSend(&portInterface, &prvTgfBypassPacketInfo);

    /* AUTODOC: packet forward to port 2 (FDB).  */
    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (ING_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
            continue;
        }

        /* check Tx counters */
        if (EGR_FDB_PORT_IDX_CNS == portIter)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
    }

    /* AUTODOC: packet MAC SA learning verified. */
    rc = prvTgfFdbMacLearningCheck(prvTgfBypassPacketL2Part.saMac);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFdbMacLearningCheck");

    /* AUTODOC: packet MIRROR to CPU (FDB).      */
    /* get 1 entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_TRUE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet");

    /* AUTODOC: flush FDB exclude static entries */
    rc = prvTgfBrgFdbFlush(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_FALSE);

    /* AUTODOC: set PCL rule with bypass bridge ENABLED*/
    rc = prvTgfTrafficPclRuleAndActionSet(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfTrafficPclRuleAndActionSet: %d, %d",
        prvTgfDevNum, GT_FALSE);

    /* AUTODOC: set bypass bridge mode to all except SA learning. */
    rc = prvTgfBrgGenBypassModeSet(PRV_TGF_BRG_GEN_BYPASS_MODE_ALL_EXCEPT_SA_LEARNING_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenBypassModeSet: %d",
                                 PRV_TGF_BRG_GEN_BYPASS_MODE_ALL_EXCEPT_SA_LEARNING_E);

    /* packet according to PCL action - bridge bypass, exit on port 0 (PCL), */
    /*                                  no mirroring expected.               */
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PCL_PORT_IDX_CNS];

    /* AUTODOC: send packet. */
    prvTgfBrgGenBypassModeTestPacketSend(&portInterface, &prvTgfBypassPacketInfo);

    /* AUTODOC: packet forward to port 0 (PCL).  */
    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (ING_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
            continue;
        }

        /* check Tx counters */
        if (EGR_PCL_PORT_IDX_CNS == portIter)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
    }

    /* AUTODOC: packet MAC SA learning verified. */
    rc = prvTgfFdbMacLearningCheck(prvTgfBypassPacketL2Part.saMac);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFdbMacLearningCheck");

    /* AUTODOC: packet NOT MIRROR to CPU, only FORWARD (PCL).      */
    /* get 1 entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_TRUE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet");

    /* AUTODOC: flush FDB exclude static entries */
    rc = prvTgfBrgFdbFlush(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_FALSE);

    /* AUTODOC: set bypass bridge mode to bypass only forwarding decision. */
    rc = prvTgfBrgGenBypassModeSet(PRV_TGF_BRG_GEN_BYPASS_MODE_ONLY_FORWARDING_DECISION_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenBypassModeSet: %d",
                                 PRV_TGF_BRG_GEN_BYPASS_MODE_ONLY_FORWARDING_DECISION_E);

    if(isErratumForFromCpuForBridgeBypassForwardingDecisionOnlyMode == GT_TRUE)
    {
        /* packet according to PCL action - bridge bypass, NO exit any port (the FROM_CPU filtered), */
        /*                                  NO mirroring expected                   */

        cpssOsPrintf("WARNING: Due to Erratum : The 'FROM_CPU' should be filtered !!! in 'Bridge Bypass Forwarding Decision Only Mode' \n");

        /* AUTODOC: send packet. */
        prvTgfBrgGenBypassModeTestPacketSend(&portInterface, &prvTgfBypassPacketInfo);

        /* it seems that the device filters the 'from cpu' packet when mode is
            'Bypass Forwarding Decision only' due to 'vlan not valid' */
        /* read counters */
        portIter = ING_PORT_IDX_CNS;
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL1_STRING_MAC(0/*filtered !!!*/, portCntrs.goodPktsRcv.l[0],
                                     "ERROR : The FROM_CPU should be filtered !!! (and not reach port %d)",
                                     prvTgfPortsArray[portIter]);
        /* clear all the counters */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            (void)prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        PRV_UTF_LOG0_MAC("WARNING: Due to Erratum : validate the VLAN that the 'FROM_CPU' uses and set the egress port member so the packet can egress in the next try !!! \n");
        /* validate the VLAN that the 'FROM_CPU' uses and set the egress port member .
           so the packet can egress in the next try !!! */
        prvTgfBrgGenIeeeReservedMcastProfilesTestInit(FROM_CPU_VLANID_CNS);

    }

    /* packet according to PCL action - bridge bypass, exit on port 0 (PCL), */
    /*                                  mirroring expected                   */

    /* AUTODOC: send packet. */
    prvTgfBrgGenBypassModeTestPacketSend(&portInterface, &prvTgfBypassPacketInfo);

    /* AUTODOC: packet forward to port 0 (PCL).  */
    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (ING_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
            continue;
        }

        /* check Tx counters */
        if (EGR_PCL_PORT_IDX_CNS == portIter)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
    }

    /* AUTODOC: packet MAC SA learning verified. */
    rc = prvTgfFdbMacLearningCheck(prvTgfBypassPacketL2Part.saMac);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFdbMacLearningCheck");

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_FALSE)
    {
        /* Due to Bobcat2 errata: No traffic to CPU in Bridge Bypass Forwarding Decision Only mode. */
        expectedRcRxToCpu = GT_NO_MORE;
    }

    /* AUTODOC: packet NOT MIRROR to CPU, only FORWARD (PCL).      */
    /* get 1 entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_TRUE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(expectedRcRxToCpu, rc, "tgfTrafficGeneratorRxInCpuGet");

    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet");

    prvTgfBrgGenBypassModeTestConfigurationRestore();
}

/**
* @internal prvTgfBrgPhysicalPortsSetUpTest function
* @endinternal
*
* @brief   Bridge physical port setup test
*/
GT_VOID prvTgfBrgPhysicalPortsSetUpTest
(
    GT_VOID
)
{
    GT_U32                  port;
    GT_U32                  numPorts;
    GT_BOOL                 status;
    CPSS_PORT_LOCK_CMD_ENT  cmd;
    GT_BOOL                 forwarddEn;
    GT_STATUS               rc;

    /* AUTODOC: check default values for cpssDxChBrgFdbNaToCpuPerPortSet and cpssDxChBrgFdbPortLearnStatusSet
       on all ports only for eArch devices */
    numPorts = 256;

    for (port = 0; port < numPorts; port++)
    {
        /* AUTODOC: get current value of forwarding of NA to CPU enable */
        rc = cpssDxChBrgFdbNaToCpuPerPortGet(prvTgfDevNum, port, &forwarddEn);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbNaToCpuPerPortGet");
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, forwarddEn, "cpssDxChBrgFdbNaToCpuPerPortGet: port[%d], forwardEn[%d]", port, forwarddEn);

        rc = cpssDxChBrgFdbPortLearnStatusGet(prvTgfDevNum, port, &status, &cmd);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbPortLearnStatusGet");
        if(appDemoSysConfig.forceAutoLearn == GT_TRUE)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, status, "cpssDxChBrgFdbPortLearnStatusGet:port[%d] learn status[%d]", port, status);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_FALSE, status, "cpssDxChBrgFdbPortLearnStatusGet:port[%d] learn status[%d]", port, status);
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(CPSS_LOCK_FRWRD_E, cmd, "cpssDxChBrgFdbPortLearnStatusGet:port[%d] learn command[%d]", port, cmd);
    }
}


