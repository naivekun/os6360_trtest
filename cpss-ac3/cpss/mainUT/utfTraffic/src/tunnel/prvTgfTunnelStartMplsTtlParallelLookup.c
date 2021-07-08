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
* @file prvTgfTunnelStartMplsTtlParallelLookup.c
*
* @brief Tunnel Start: Mpls TTL functionality for POP operation in Parallel Lookup
*
* @version   2
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTunnelGen.h>
#include <tunnel/prvTgfTunnelStartMpls.h>
#include <tunnel/prvTgfTunnelStartMplsTtlParallelLookup.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* max iteration's name length */
#define PRV_TGF_MAX_ITERATION_NAME_LEN_CNS 100

/* number of send traffic iteration */
#define PRV_TGF_MAX_ITER_CNS              1

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* port number to send traffic to in default VLAN */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* port number of unused port in default VLAN */
#define PRV_TGF_FDB_PORT_IDX_CNS          1

/* port number to send traffic to */
#define PRV_TGF_FREE_PORT_IDX_CNS         2

/* port number to receive traffic from in nextHop VLAN */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS      3

#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

/* this is the first call to the function - need to do global configuration */
static GT_BOOL firstCall = GT_TRUE;

static struct{

    GT_U32 isValid;
    GT_U32 ttiRuleIndex;
    GT_U32 copyTtlExpFromTunnelHeader;      /* copyTtlExpFromTunnelHeader value for lookup  */
    GT_U32 enableDecrementTtl;              /* enableDecrementTtl value for lookup  */
}lookupInfoStc[PRV_TGF_MAX_LOOKUPS_NUM_CNS];


/* original tcam segment mode value (for configuration restore) */
static PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_ENT tcamSegmentModeOrig;


/************************ common parts of packets **************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket1L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                 /* saMac */
};
/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket1EtherTypePart = {
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacket1Ipv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x32,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfPacket1PayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket1PayloadPart = {
    sizeof(prvTgfPacket1PayloadDataArr),                 /* dataLength */
    prvTgfPacket1PayloadDataArr                          /* dataPtr */
};

/************************* packet 1 (IPv4 packet) **************************/

/* PARTS of packet 1 */
static TGF_PACKET_PART_STC prvTgfPacket1PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1Ipv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};
/* PACKET 1 to send */
static TGF_PACKET_STC prvTgfPacket1Info = {
    ( TGF_L2_HEADER_SIZE_CNS
    + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS
    + sizeof(prvTgfPacket1PayloadDataArr)),                       /* totalLen */
    sizeof(prvTgfPacket1PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket1PartArray                                        /* partsArray */
};

/**************************** packet MPLS **********************************/
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketMplsVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};
/* MPLS ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketMplsMplsEtherTypePart = {
    TGF_ETHERTYPE_8847_MPLS_TAG_CNS
};

/********************* packet MPLS with 3 labels ***************************/
/* MPLS 4 part */
static TGF_PACKET_MPLS_STC prvTgfPacketMplsMpls4Part = {
    0x44,               /* label */
    8,                  /* exp */
    0,                  /* stack */
    0xcc                /* timeToLive */
};

/* MPLS 3 part */
static TGF_PACKET_MPLS_STC prvTgfPacketMplsMpls3Part = {
    0x33,               /* label */
    7,                  /* exp */
    0,                  /* stack */
    0xdd                /* timeToLive */
};
/* MPLS 2 part */
static TGF_PACKET_MPLS_STC prvTgfPacketMplsMpls2Part = {
    0x22,               /* label */
    6,                  /* exp */
    0,                  /* stack */
    0xee                /* timeToLive */
};
/* MPLS 1 part */
static TGF_PACKET_MPLS_STC prvTgfPacketMplsMpls1Part = {
    0x11,               /* label */
    5,                  /* exp */
    1,                  /* stack */
    0xFF                /* timeToLive */
};

/* PARTS of packet MPLS 4 */
static TGF_PACKET_PART_STC prvTgfPacketMpls4PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketMplsVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsMpls4Part},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsMpls3Part},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsMpls2Part},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsMpls1Part},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1Ipv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};
/* PACKET MPLS 4 to send */
static TGF_PACKET_STC prvTgfPacketMpls4Info = {
    ( TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
    + TGF_ETHERTYPE_SIZE_CNS + TGF_MPLS_HEADER_SIZE_CNS
    + TGF_MPLS_HEADER_SIZE_CNS + TGF_MPLS_HEADER_SIZE_CNS
    + TGF_MPLS_HEADER_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS
    + sizeof(prvTgfPacket1PayloadDataArr)),                           /* totalLen */
    sizeof(prvTgfPacketMpls4PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketMpls4PartArray                                        /* partsArray */
};

/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* MAC for Tunnel Start Entry */
static TGF_MAC_ADDR prvTgfStartEntryMac = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x11
};

static const GT_CHAR prvTgfIterationNameArray[][PRV_TGF_MAX_ITERATION_NAME_LEN_CNS] = {
    "sending packet to port [%d] ...\n"
};
static TGF_PACKET_STC *prvTgfPacketInfoArray[] = {
    &prvTgfPacket1Info
};

/* stored default Vlan ID */
static GT_U16   prvTgfDefVlanId = 0;

/* the Arp Address index of the Router ARP Table
 * to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32   prvTgfRouterArpIndex = 1;

/* define expected number of received packets (Sent packets) */
static GT_U8 prvTgfPacketsCountRxArray[][PRV_TGF_PORTS_NUM_CNS] = {
/* ports:  SEND VLN1 VLN2 RECEIVE */
             {1,  0,  0,  1}
};

/* define expected number of transmitted packets */
static GT_U8 prvTgfPacketsCountTxArray[][PRV_TGF_PORTS_NUM_CNS] = {
/* ports:  SEND VLN1 VLN2 RECEIVE */
             {1,  0,  0,  1}
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_TTI_MAC_MODE_ENT        macMode;
} prvTgfRestoreCfg;

/******************************************************************************/


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfPortCaptureEnable function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfPortCaptureEnable
(
    IN GT_U32   portNum,
    IN GT_BOOL start
)
{
    GT_STATUS               rc = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = portNum;

    /* enable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, start);
    PRV_UTF_VERIFY_GT_OK(rc, "tgfTrafficGeneratorPortTxEthCaptureSet");

    return rc;
};

/**
* @internal prvTgfPortCapturePacketPrint function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfPortCapturePacketPrint
(
    IN GT_U32        portNum,
    IN TGF_MAC_ADDR macPattern,
    IN GT_BOOL      lookup3Test,
    OUT GT_U32     *numTriggersPtr
)
{
    GT_STATUS               rc = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[2];

    CPSS_NULL_PTR_CHECK_MAC(numTriggersPtr);
    lookup3Test = lookup3Test;
    *numTriggersPtr = 0;

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portNum);

    /* set nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = portNum;

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set pattern as MAC to select captured packets */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, macPattern, sizeof(TGF_MAC_ADDR));

    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset = 15;
    vfdArray[1].cycleCount = 3;

    if (PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS==PRV_TGF_MAX_LOOKUPS_NUM_CNS)
    {
        /*  00 00 00 00 00 11 00 00 00 00 00 06 88 47 00 01
            1b cb 45 00 00 32 00 00 00 00 40 ff 4b 9e 16 16
            16 16 01 01 01 03 00 01 02 03 04 05 06 07 08 09
            0a 0b 0c 0d 0e 0f 10 11 12 13 14 15 16 17 18 19
            1a 1b 1c 1d 55 55 55 55 */

        vfdArray[1].patternPtr[0] = 0x01;
        vfdArray[1].patternPtr[1] = 0x1b;
        /* the ttl of the outerLabel (of the send packet) decrement by 1 */
        vfdArray[1].patternPtr[2] = 0xcb;
    }
    else/*PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS == 2*/
    {
        /*  00 00 00 00 00 11 00 00 00 00 00 06 88 47 00 02
            2c cb 00 01 1b ff 45 00 00 32 00 00 00 00 40 ff
            4b 9e 16 16 16 16 01 01 01 03 00 01 02 03 04 05
            06 07 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 14 15
            16 17 18 19 1a 1b 1c 1d 55 55 55 55 */
        vfdArray[1].patternPtr[0] = 0x02;
        vfdArray[1].patternPtr[1] = 0x2c;
        /*the ttl of the outerLabel (of the send packet) decrement by 1 */
        vfdArray[1].patternPtr[2] = 0xcb;
    }
    /* check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 2, vfdArray, numTriggersPtr);
    rc = rc == GT_NO_MORE ? GT_OK : rc;

    return rc;
};

/**
* @internal prvTgfVlanSet function
* @endinternal
*
* @brief   Set VLAN entry
*
* @param[in] vlanId                   -  to be configured
* @param[in] portsMembers             - VLAN's ports Members
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfVlanSet
(
    IN GT_U16          vlanId,
    IN CPSS_PORTS_BMP_STC portsMembers
)
{
    GT_STATUS                          rc = GT_OK;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    CPSS_PORTS_BMP_STC                 portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC          vlanInfo;

    /* clear ports tagging */
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* set vlan entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    vlanInfo.autoLearnDisable = GT_TRUE; /* working in controlled learning */
    vlanInfo.naMsgToCpuEn     = GT_TRUE; /* working in controlled learning */

    /* set vlan entry */
    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 vlanId,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanEntryWrite");

    return rc;
};

/**
* @internal prvTgfDefBridgeConfSet function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfDefBridgeConfSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PORTS_BMP_STC              vlanMembers = {{0, 0}};

    /* clear entry */
    cpssOsMemSet(&vlanMembers, 0, sizeof(vlanMembers));

    /* ports 0, 8 are VLAN_1 Members */
    CPSS_PORTS_BMP_PORT_SET_MAC(&vlanMembers,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&vlanMembers,prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS]);

    prvTgfVlanSet(PRV_TGF_SEND_VLANID_CNS, vlanMembers);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfVlanSet");

    /* clear entry */
    cpssOsMemSet(&vlanMembers, 0, sizeof(vlanMembers));

    /* ports 18, 23 are VLAN_2 Members */
    CPSS_PORTS_BMP_PORT_SET_MAC(&vlanMembers,prvTgfPortsArray[PRV_TGF_FREE_PORT_IDX_CNS]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&vlanMembers,prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

    prvTgfVlanSet(PRV_TGF_NEXTHOPE_VLANID_CNS, vlanMembers);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfVlanSet");

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidGet");

    /* set default vlanId */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgFdbFlush");

    /* create a macEntry for original packet 1 in VLAN 5 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacket1L2Part.daMac, PRV_TGF_SEND_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS], GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* create a macEntry for tunneled packet 1 in VLAN 6 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacket1L2Part.daMac, PRV_TGF_NEXTHOPE_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    return rc;
};

/**
* @internal prvTgfConfigurationRestore function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U8       lookupNum;

    /* invalidate TTI rules */
    for(lookupNum = 0; lookupNum < PRV_TGF_MAX_LOOKUPS_NUM_CNS; lookupNum++)
    {
        if(0 == lookupInfoStc[lookupNum].isValid)
        {
            break;
        }

        /* AUTODOC: invalidate TTI rules */
        rc = prvTgfTtiRuleValidStatusSet(lookupInfoStc[lookupNum].ttiRuleIndex, GT_FALSE);
        PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfTtiRuleValidStatusSet");
    }

     /* AUTODOC: restore TCAM segment mode for Mpls key */
    rc = prvTgfTtiTcamSegmentModeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_MPLS_E, tcamSegmentModeOrig);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfTtiTcamSegmentModeSet");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfBrgFdbFlush");

    /* restore default vlanId */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfBrgVlanPvidSet");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryInvalidate");

    return rc;
};

/**
* @internal prvTgfTrafficGenerate function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    sendIter;

    /* sending packets */
    for (sendIter = 0; sendIter < PRV_TGF_MAX_ITER_CNS; sendIter++)
    {
        PRV_UTF_LOG1_MAC(
            prvTgfIterationNameArray[sendIter],
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* setup transmit params */
        rc = prvTgfSetTxSetupEth(
            prvTgfDevNum, prvTgfPacketInfoArray[sendIter],
            1 /*burstCount*/, 0 /*numVfd*/, NULL);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

        /* start transmitting */
        rc = prvTgfStartTransmitingEth(
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");
    }

    return rc;
};

/**
* @internal prvTgfTtiEnable function
* @endinternal
*
* @brief   This function enables the TTI lookup
*         and sets the lookup Mac mode
*         for the specified key type
* @param[in] portNum                  - port number
* @param[in] ttiKeyType               - TTI key type
* @param[in] macMode                  - MAC mode to use
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfTtiEnable
(
    IN  GT_U32                    portNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT ttiKeyType,
    IN  PRV_TGF_TTI_MAC_MODE_ENT macMode,
    IN  GT_BOOL                  isTtiEnable
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    ethType;
    PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_ENT   tcamSegmentMode;

    if (GT_TRUE == isTtiEnable)
    {
        /* set MIM EtherType */
        ethType = TGF_ETHERTYPE_88E7_MIM_TAG_CNS;

        /* save current Eth TTI key lookup MAC mode */
        rc = prvTgfTtiMacModeGet(prvTgfDevNum, ttiKeyType, &prvTgfRestoreCfg.macMode);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiMacModeGet");
    }
    else
    {
        /* restore default MIM EtherType */
        ethType = TGF_ETHERTYPE_88E7_MIM_TAG_CNS;

        /* restore Eth TTI key lookup MAC mode */
        macMode = prvTgfRestoreCfg.macMode;
    }

    /* set MIM EtherType */
    rc = prvTgfTtiMimEthTypeSet(ethType);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiMimEthTypeSet");

    /* enable/disable port for ttiKeyType TTI lookup */
    rc = prvTgfTtiPortLookupEnableSet(portNum, ttiKeyType, isTtiEnable);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiPortLookupEnableSet");

    /* set TTI lookup MAC mode */
    rc = prvTgfTtiMacModeSet(ttiKeyType, macMode);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiMacModeSet");

    /* AUTODOC: save TCAM segment mode for Mpls key */
    rc = prvTgfTtiTcamSegmentModeGet(prvTgfDevNum, PRV_TGF_TTI_KEY_MPLS_E, &tcamSegmentModeOrig);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfTtiTcamSegmentModeGet");

    tcamSegmentMode =  PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_TRUE ?
                                                PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_FOUR_TCAM_E :
                                                PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_E ;

    /* AUTODOC: set TCAM segment mode for Mpls key */
    rc = prvTgfTtiTcamSegmentModeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_MPLS_E, tcamSegmentMode);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfTtiTcamSegmentModeSet");

    return rc;
};

/**
* @internal prvTgfDefTtiMplsPopRuleSetPerLookupSet function
* @endinternal
*
* @brief   None
*
* @param[in] lookupNum                - the lookupNumber rule to configure
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfDefTtiMplsPopRuleSetPerLookupSet
(
    IN GT_U8                                lookupNum
)
{
    GT_STATUS                   rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC    ttiAction;
    PRV_TGF_TTI_RULE_UNT        ttiPattern;
    PRV_TGF_TTI_RULE_UNT        ttiMask;

    /* cpssDxChTtiRuleSet(CPSS_DXCH_TTI_MPLS_RULE_STC, CPSS_DXCH_TTI_ACTION_2_STC) */

    /* clear action */
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));

    /* set action MPLS command */
    if (lookupNum!=(PRV_TGF_MAX_LOOKUPS_NUM_CNS-1))
    {
        ttiAction.mplsCommand               = PRV_TGF_TTI_MPLS_POP1_LABEL_COMMAND_E;
    }
    else
    {
        ttiAction.mplsCommand                   = PRV_TGF_TTI_MPLS_NOP_COMMAND_E;
    }

    /* set action - redirect to port 23 */
    ttiAction.tunnelTerminate               = GT_FALSE;
    ttiAction.ttPassengerPacketType         = PRV_TGF_TTI_PASSENGER_MPLS_E;
    ttiAction.tsPassengerPacketType         = PRV_TGF_TUNNEL_PASSENGER_OTHER_E;
    ttiAction.command                       = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.tag0VlanCmd                   = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ttiAction.tag0VlanId                    = 6;
    ttiAction.tag1VlanCmd                   = PRV_TGF_TTI_VLAN1_MODIFY_UNTAGGED_E;
    ttiAction.tag1VlanId                    = 0;
    ttiAction.keepPreviousQoS               = GT_TRUE;
    ttiAction.tunnelStart                   = GT_FALSE;
    ttiAction.tunnelStartPtr                = 0;
    ttiAction.arpPtr                        = prvTgfRouterArpIndex;

    ttiAction.copyTtlExpFromTunnelHeader    = lookupInfoStc[lookupNum].copyTtlExpFromTunnelHeader;
    ttiAction.enableDecrementTtl            = lookupInfoStc[lookupNum].enableDecrementTtl;

    ttiAction.redirectCommand                   = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction.egressInterface.type              = CPSS_INTERFACE_PORT_E;
    ttiAction.egressInterface.devPort.hwDevNum  = prvTgfDevNum;

    switch (lookupNum)
    {
        case 0:
            ttiAction.egressInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_FREE_PORT_IDX_CNS];
            break;
        case 1:
            ttiAction.egressInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
            break;
        case 2:
            ttiAction.egressInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_FREE_PORT_IDX_CNS];
            break;
        case 3:
            ttiAction.egressInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
            break;
        default:
            return GT_BAD_PARAM;
    }

    if(lookupNum != (PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS-1) )
    {
        ttiAction.continueToNextTtiLookup = GT_TRUE;
    }

    /* mask and pattern - mpls labels */
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(ttiMask.mpls.common.mac.arEther, 0xFF, sizeof(TGF_MAC_ADDR));
    ttiMask.mpls.common.vid         = 0x0F;
    ttiMask.mpls.common.pclId       = 0x3FF;

    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemCpy(ttiPattern.mpls.common.mac.arEther, prvTgfPacket1L2Part.daMac, sizeof(TGF_MAC_ADDR));
    ttiPattern.mpls.common.vid      = PRV_TGF_SEND_VLANID_CNS;
    ttiPattern.mpls.common.pclId    = 2;

    /* set TTI Rule */
    rc = prvTgfTtiRule2Set(lookupInfoStc[lookupNum].ttiRuleIndex,
                            PRV_TGF_TTI_KEY_MPLS_E, &ttiPattern, &ttiMask, &ttiAction);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiRuleSet");

    return rc;
};

/**
* @internal prvTgfDefArpEntrySet function
* @endinternal
*
* @brief   Set a default ARP entry
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfDefArpEntrySet
(
    GT_VOID
)
{
    GT_STATUS                      rc = GT_OK;
    GT_ETHERADDR                   arpMacAddr;

    /* convert MAC address to other format */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfStartEntryMac, sizeof(TGF_MAC_ADDR));

    /* write a ARP MAC address to the Router ARP Table */
    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpRouterArpAddrWrite");

    return rc;
};

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelMplsPopTtlParallelLookup function
* @endinternal
*
* @brief   MPLS LSR POP Functionality and TTL check for parallel Lookup
*
* @param[in] copyTtlExpFromTunnelHeaderArr[] - array of copyTtlExpFromTunnelHeader for lookups
* @param[in] enableDecrementTtlArr[]  - array of copyTtlExpFromTunnelHeader for lookups
* @param[in] lookup3Test              - testing forth lookup configuration
* @param[in] restoreConfig            - GT_TRUE:restore
*                                      GT_FALSE: do not restore
*                                       None
*/
GT_VOID prvTgfTunnelMplsPopTtlParallelLookup
(
    GT_BOOL copyTtlExpFromTunnelHeaderArr[],
    GT_BOOL enableDecrementTtlArr[],
    GT_BOOL lookup3Test,
    GT_BOOL restoreConfig
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    portIter;
    GT_U32    packetSize;
    GT_U32    numTriggers = 0;
    GT_U8       lookupNum;


    PRV_UTF_LOG1_MAC("\nVLAN %d CONFIGURATION:\n", PRV_TGF_SEND_VLANID_CNS);
    PRV_UTF_LOG2_MAC("  Port members: [%d], [%d]\n",prvTgfPortsArray[0], prvTgfPortsArray[1]);

    PRV_UTF_LOG1_MAC("VLAN %d CONFIGURATION:\n", PRV_TGF_NEXTHOPE_VLANID_CNS);
    PRV_UTF_LOG2_MAC("  Port members: [%d], [%d]\n",prvTgfPortsArray[2], prvTgfPortsArray[3]);

    PRV_UTF_LOG0_MAC("FDB CONFIGURATION:\n");
    PRV_UTF_LOG1_MAC("  MAC DA ...:34:02 to port [%d]\n", prvTgfPortsArray[1]);
    PRV_UTF_LOG1_MAC("  MAC DA ...:00:11 to port [%d]\n", prvTgfPortsArray[3]);

    PRV_UTF_LOG0_MAC("PCL CONFIGURATION:\n");
    PRV_UTF_LOG0_MAC("  CLEAR\n");

    PRV_UTF_LOG0_MAC("TTI CONFIGURATION:\n");
    PRV_UTF_LOG2_MAC("  FORWARD packet with MAC DA ..:34:02 to port [%d]\n"
            "  in VLAN %d with MIM tunnel (MAC DA must be ...:00:11)\n",
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], PRV_TGF_NEXTHOPE_VLANID_CNS);

    PRV_UTF_LOG0_MAC("TRAFFIC GENERATION:\n");
    PRV_UTF_LOG0_MAC("  Sending MPLS packet with 4 labels:\n"
                     "   label_0=0x44(outer label)\n   label_1=0x33\n label_1=0x22\n   label_2=0x11\n");
    PRV_UTF_LOG1_MAC("   and MAC DA=...:34:02\n  to port [%d]\n",prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    PRV_UTF_LOG0_MAC("EXPECTED RESULTS:\n");
    PRV_UTF_LOG1_MAC("  packet is not forwarded in VLAN %d\n", PRV_TGF_SEND_VLANID_CNS);
    PRV_UTF_LOG2_MAC("  packet is forwarded to port [%d] VLAN %d with ",
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], PRV_TGF_NEXTHOPE_VLANID_CNS);

    PRV_UTF_LOG0_MAC("   and MAC DA=...:00:11\n\n");

    /* AUTODOC: set TTI lookups configuration */
    for(lookupNum = 0; lookupNum < PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS ; lookupNum++)
    {
        PRV_UTF_LOG1_MAC("\n============================ Process lookup %d =======================\n", lookupNum);

        if((0 == lookupNum)&&(firstCall==GT_TRUE))
        {
            /* -------------------------------------------------------------------------
             * 1. Set configuration

             */

            /* set VLANs */
            rc = prvTgfDefBridgeConfSet();
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefBridgeConfSet: %d", prvTgfDevNum);

            /* enable TTI lookup for MPLS key and MAC DA mode */
            if (GT_OK == rc) {
                rc = prvTgfTtiEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                     PRV_TGF_TTI_KEY_MPLS_E,
                                     PRV_TGF_TTI_MAC_MODE_DA_E,
                                     GT_TRUE);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEnable: %d", prvTgfDevNum);
            }

            /* set ARP Entry (Write ARP address 00:00:00:00:00:11 at index 1) */
            if (GT_OK == rc) {
                rc = prvTgfDefArpEntrySet();
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefArpEntrySet: %d", prvTgfDevNum);
            }
            firstCall=GT_FALSE;
        }

        /* AUTODOC: init lookup data */
        lookupInfoStc[lookupNum].isValid = 1;
        lookupInfoStc[lookupNum].ttiRuleIndex = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(lookupNum , 3);
        lookupInfoStc[lookupNum].copyTtlExpFromTunnelHeader = copyTtlExpFromTunnelHeaderArr[lookupNum];
        lookupInfoStc[lookupNum].enableDecrementTtl = enableDecrementTtlArr[lookupNum];

        /* set TTI rules for MPLS Pop key */
        if (GT_OK == rc) {
            rc = prvTgfDefTtiMplsPopRuleSetPerLookupSet(lookupNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefTtiMplsPopRuleSetPerLookupSet: %d", prvTgfDevNum);
        }
    }

    /* -------------------------------------------------------------------------
     * 2. Generate traffic and Check counters
     */

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* select MPLS packet to send */
    prvTgfPacketInfoArray[0] = &prvTgfPacketMpls4Info;

    /* enable capturing of receive port */
    rc = prvTgfPortCaptureEnable(
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPortCaptureEnable: %d", prvTgfDevNum);

    /* generate traffic */
    rc = prvTgfTrafficGenerate();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrafficGenerate: %d", prvTgfDevNum);

    /* disable capturing of receive port */
    rc = prvTgfPortCaptureEnable(
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPortCaptureEnable: %d", prvTgfDevNum);

    /* check ETH counters */
    for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
    {
        if (PRV_TGF_RECEIVE_PORT_IDX_CNS == portIter)
        {
            /* packet after tunnel start in VLAN 6 */
            packetSize = prvTgfPacketInfoArray[0]->totalLen;

            if (PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS==2)
            {
                packetSize -= 2 * TGF_MPLS_HEADER_SIZE_CNS;
            }
            else/*PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS == 4*/
            {
                packetSize -= 3 * TGF_MPLS_HEADER_SIZE_CNS;
            }
        }
        else
        {
            /* original packet before tunneling in VLAN 5 */
            packetSize = prvTgfPacketInfoArray[0]->totalLen;
        }

        if (PRV_TGF_SEND_PORT_IDX_CNS != portIter)
        {
            packetSize -= TGF_VLAN_TAG_SIZE_CNS;
        }

        /* check ETH counters */
        rc = prvTgfEthCountersCheck(
            prvTgfDevNum,
            prvTgfPortsArray[portIter],
            prvTgfPacketsCountRxArray[0][portIter],
            prvTgfPacketsCountTxArray[0][portIter],
            packetSize, 1 /*burstCount*/);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
    }

    /* print captured packets from receive port with prvTgfStartEntryMac */
    rc = prvTgfPortCapturePacketPrint(
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            prvTgfStartEntryMac, lookup3Test, &numTriggers);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfPortCapturePacketPrint:"
                                 " port = %d, rc = 0x%02X\n",
                                 prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], rc);

    /* check if captured packet has the same MAC DA as prvTgfArpMac */
    UTF_VERIFY_EQUAL6_STRING_MAC(3, numTriggers,
            "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X\n",
            prvTgfStartEntryMac[0], prvTgfStartEntryMac[1], prvTgfStartEntryMac[2],
            prvTgfStartEntryMac[3], prvTgfStartEntryMac[4], prvTgfStartEntryMac[5]);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfPortCapturePacketPrint: %d", prvTgfDevNum);


    if (restoreConfig==GT_TRUE)
    {
        /* -------------------------------------------------------------------------
         * 3. Restore configuration
         */

        /* restore TTI lookup and MAC DA mode for Eth key */
        rc = prvTgfTtiEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                             PRV_TGF_TTI_KEY_MPLS_E,
                             PRV_TGF_TTI_MAC_MODE_DA_E,
                             GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEnable: %d", prvTgfDevNum);

        /* restore common configuration */
        rc = prvTgfConfigurationRestore();
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfConfigurationRestore: %d", prvTgfDevNum);

        prvTgfPclRestore();

        firstCall=GT_TRUE;
    }
}


