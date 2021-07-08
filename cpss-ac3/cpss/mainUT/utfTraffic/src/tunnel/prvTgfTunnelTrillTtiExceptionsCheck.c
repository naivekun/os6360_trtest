/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfTunnelTrillTtiKnownUcTransit.c
*
* DESCRIPTION:
*       Tunnel: TRILL Exceptions Check
*
* FILE REVISION NUMBER:
*       $Revision: 13 $
*
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfPortGen.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfTcamGen.h>
#include <tunnel/prvTgfTunnelTrillTtiExceptionsCheck.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* TRILL Hop Count (in bytes)*/
#define PRV_TGF_TRILL_HOP_COUNT_SIZE_CNS    2

/* TRILL VLAN Id */
#define PRV_TGF_TRILL_VLANID_CNS            5

/* Destination ePort */
#define PRV_TGF_DESTINATION_EPORT_CNS    1000

/* Source ePort */
#define PRV_TGF_SOURCE_EPORT_CNS           700

/* Tti entry index */
#define PRV_TGF_TTI_INDEX_CNS              80

/* Tunnel start entry index */
#define PRV_TGF_TUNNEL_START_INDEX_CNS      8

/* MAC2ME entry index */
#define PRV_TGF_MAC_TO_ME_INDEX_CNS         1

/* Adjacency entry index*/
#define PRV_TGF_ADJACENCY_INDEX_CNS         1

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS           0

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS        3

/* max number of VFD to use */
#define PRV_TGF_VFD_NUM_CNS                 10

/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 1;

/* value for cpu code base field */
#define PRV_TGF_CPU_CODE_BASE_CNS           200

/* max TRILL header version */
#define PRV_TGF_TRILL_MAX_VERSION           0

/* max number oof TTI exceptions*/
#define PRV_TGF_MAX_TTI_EXCEPTIONS          21

/* default number of TRILL options */
#define TGF_TRILL_OPTIONS_SIZE_CNS          4

/* 12 bits of ePort - TRILL unknown RBID */
#define PRV_TGF_TRILL_UNKNOWN_RBID_CNS      0xFFF

/* 12 bits of ePort - TRILL MY RBID */
#define PRV_TGF_TRILL_MY_RBID_CNS           0xFFE

/* TCAM floor used for TTI TRILL rules */
#define PRV_TGF_TTI_TCAM_FLOOR_CNS          0

/* TTI TCAM client group*/
#define PRV_TGF_TTI_TCAM_CLIENT_GROUP_CNS   0

/* TTI-0 TRILL rule index */
#define PRV_TGF_TTI_0_RULE_INDEX_CNS        0

/* TTI-1 TRILL rule index */
#define PRV_TGF_TTI_1_RULE_INDEX_CNS        6

/* offset of the passenger in the packet*/
#define PASSENGER_OFFSET_CNS    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + \
                                 TGF_ETHERTYPE_SIZE_CNS + TGF_TRILL_HEADER_SIZE_CNS)

#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

/******************************* Test regular packet (no options) **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x55, 0x44, 0x33, 0x22, 0x11},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x05}                 /* saMac */
};

/* mac DA part of packet restore */
static TGF_MAC_ADDR prvTgfPacketL2PartRestore = {
    0x00, 0x55, 0x44, 0x33, 0x22, 0x11
};


/* VLAN_TAG part of packet */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_TRILL_VLANID_CNS                      /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_22F3_TRILL_TAG_CNS};

/* packet's TRILL */
static TGF_PACKET_TRILL_STC prvTgfPacketTrillPart =
{
    0,                  /* version*/
    GT_FALSE,           /* mBit */
    0,                  /* opLength */
    0x20,               /* hopCount */
    0x6543,             /* eRbid */
    0x4321              /* iRbid*/
};

/***************** Incapsulation ethernet frame **********************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2PartInt = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                 /* saMac */
};

/* mac DA part of packet restore */
static TGF_MAC_ADDR prvTgfPacketL2PartIntRestore = {
    0x00, 0x00, 0x00, 0x00, 0x34, 0x02
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPartInt =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,        /* etherType */
    0, 0, PRV_TGF_TRILL_VLANID_CNS          /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33
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
    {TGF_PACKET_PART_L2_E,                  &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,            &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E,           &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_TRILL_E,               &prvTgfPacketTrillPart},
    /* incapsulation ethernet frame part */
    {TGF_PACKET_PART_L2_E,                  &prvTgfPacketL2PartInt},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,            &prvTgfPacketVlanTagPartInt},
    {TGF_PACKET_PART_PAYLOAD_E,             &prvTgfPacketPayloadPart}
};

/* Length of transmitted packet */
#define PRV_TGF_TRANSMITTED_PACKET_LEN_CNS    \
    TGF_L2_HEADER_SIZE_CNS * 2   + \
    TGF_VLAN_TAG_SIZE_CNS  * 2   + \
    TGF_ETHERTYPE_SIZE_CNS       + \
    TGF_TRILL_HEADER_SIZE_CNS    + \
    sizeof(prvTgfPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_TRANSMITTED_PACKET_LEN_CNS,      /* totalLen */
    (sizeof(prvTgfPacketPartArray)
        / sizeof(prvTgfPacketPartArray[0])), /* numOfParts */
    prvTgfPacketPartArray                    /* partsArray */
};

/* LENGTH of expected packet (original packet tunneled over TRILL packet) */
#define PRV_TGF_EXPECTED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS  + sizeof(prvTgfPayloadDataArr)


/* MAC for Tunnel Start Entry */
static TGF_MAC_ADDR prvTgfTunnelStartEntryMacDa = {
    0x66, 0x55, 0x44, 0x33, 0x22, 0x11
};

/* MAC DA - "All-RBridges MC address" */
static TGF_MAC_ADDR prvTgfPacketL2PartMacDa = {
    0x01, 0x80, 0xC2, 0x00, 0x00, 0x40
};

/* MAC DA - "All-RBridges Esadi address" */
static TGF_MAC_ADDR prvTgfPacketL2PartMacDaEsadi = {
    0x01, 0x80, 0xC2, 0x00, 0x00, 0x42
};

/******************************* Test enhanced packet (with options) **********************************/
/* packet's TRILL options */
static TGF_PACKET_TRILL_FIRST_OPTION_STC prvTgfPacketTrillOptionsPart =
{
    0,                  /* chbh */
    0,                  /* clte */
    0                   /* info */
};

/***************** Incapsulation ethernet frame **********************/
/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketOptionsPartArray[] =
{
    {TGF_PACKET_PART_L2_E,                  &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,            &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E,           &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_TRILL_E,               &prvTgfPacketTrillPart},
    {TGF_PACKET_PART_TRILL_FIRST_OPTION_E,  &prvTgfPacketTrillOptionsPart},
    /* incapsulation ethernet frame part */
    {TGF_PACKET_PART_L2_E,                  &prvTgfPacketL2PartInt},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,            &prvTgfPacketVlanTagPartInt},
    {TGF_PACKET_PART_PAYLOAD_E,             &prvTgfPacketPayloadPart}
};

/* Length of transmitted packet */
#define PRV_TGF_TRANSMITTED_OPTIONS_PACKET_LEN_CNS    \
    TGF_L2_HEADER_SIZE_CNS * 2   + \
    TGF_VLAN_TAG_SIZE_CNS  * 2   + \
    TGF_ETHERTYPE_SIZE_CNS       + \
    TGF_TRILL_HEADER_SIZE_CNS    + \
    TGF_TRILL_OPTIONS_SIZE_CNS   + \
    sizeof(prvTgfPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketOptionsInfo =
{
    PRV_TGF_TRANSMITTED_OPTIONS_PACKET_LEN_CNS, /* totalLen */
    (sizeof(prvTgfPacketOptionsPartArray)
    / sizeof(prvTgfPacketOptionsPartArray[0])), /* numOfParts */
    prvTgfPacketOptionsPartArray                /* partsArray */
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC            egressInfo;
    CPSS_INTERFACE_INFO_STC                     physicalInfo;
    GT_BOOL                                     trillEnableTransmitPort;
    GT_BOOL                                     trillEnableDestinationPort;
    GT_U16                                      trillOuterVid0;
    GT_U32                                      cpuCodeBase;
    GT_U32                                      dropCounter[PRV_TGF_MAX_TTI_EXCEPTIONS];
    CPSS_PACKET_CMD_ENT                         exceptionCommand[PRV_TGF_MAX_TTI_EXCEPTIONS];
    GT_U32                                      maxTrillVersion;
    PRV_TGF_TCAM_BLOCK_INFO_STC                 halfFloorInfoCfg[6];
    GT_U32                                      ttiTcamClientGroup;
    GT_BOOL                                     ttiTcamClientGroupEnable;
    PRV_TGF_TTI_KEY_SIZE_ENT                    keySize;
    GT_U32                                      etherType;
    GT_U8                                       udbOffset[1];
    PRV_TGF_TTI_OFFSET_TYPE_ENT                 udbOffsetType[1];
} prvTgfRestoreCfg;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfExceptionsInit function
* @endinternal
*
* @brief   Initialize drop counters and exceptions command default settings
*/
static GT_VOID prvTgfExceptionsInit
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_TTI_EXCEPTION_ENT           Index = PRV_TGF_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E;


    for (; Index <= PRV_TGF_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E; Index++)
    {
        /* Init all exceptions command */
        rc = prvTgfTtiExceptionCmdGet(prvTgfDevNum, Index, &(prvTgfRestoreCfg.exceptionCommand[Index]));
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCmdGet: %d", Index);

        rc = prvTgfTtiExceptionCmdSet(prvTgfDevNum, Index, CPSS_PACKET_CMD_DROP_HARD_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCmdSet: %d", Index);
    }
}

/**
* @internal prvTgfSendValidPacket function
* @endinternal
*
* @brief   send TRILL regular (no options) valid packet with no exceptions
*
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] transPacketLen           - transmitted packet length
*                                       None
*/
static GT_VOID prvTgfSendValidPacket
(
    IN TGF_PACKET_STC       *packetInfoPtr,
    IN GT_U32               transPacketLen
)
{
    GT_STATUS               rc = GT_OK;
    GT_U32                  portIter    = 0;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U8                   packetsCountRx;
    GT_U8                   packetsCountTx;
    GT_U32                  packetSize = 0;
    GT_U32                  packetIter  = 0;
    GT_U32                  triggerBitmap = 0;
    TGF_VFD_INFO_STC        vfdArray[PRV_TGF_VFD_NUM_CNS];
    GT_U32                  vfdIndex = 0;/* index in vfdArray */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: send Tunneled packet from port 0 with: */
    /* AUTODOC:   DA=00:55:44:33:22:11, SA=00:00:00:00:00:05, VID=5 */
    /* AUTODOC:   EtherType=0x22F3, hopCount=0x20, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=5 */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    packetInfoPtr, prvTgfBurstCount, 0, NULL,
                    prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify to get tunneled packet on port 3 with: */
    /* AUTODOC:   DA=66:55:44:33:22:11, SA=00:00:00:00:00:05, VID=5 */
    /* AUTODOC:   EtherType=0x22F3 hopCount=0x1F, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=5 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if (portIter == PRV_TGF_SEND_PORT_IDX_CNS)
        {
            packetsCountTx = (GT_U8)prvTgfBurstCount;
            packetsCountRx = (GT_U8)prvTgfBurstCount;
            packetSize     = transPacketLen;
        }
        else if (portIter == PRV_TGF_RECEIVE_PORT_IDX_CNS)
        {
            packetsCountTx = (GT_U8)prvTgfBurstCount;
            packetsCountRx = (GT_U8)prvTgfBurstCount;
            packetSize     = transPacketLen;
        }
        else
        {
            packetsCountRx = 0;
            packetsCountTx = 0;
            packetSize = 0;
        }

        /* check ETH counters */
        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                    prvTgfPortsArray[portIter],
                                    packetsCountRx,
                                    packetsCountTx,
                                    packetSize,
                                    prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
  }

 /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    /* setup receive portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* get trigger counters */

    /* check TRILL version */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;;
    vfdArray[vfdIndex].cycleCount = 1;
    vfdArray[vfdIndex].patternPtr[0] = (GT_U8)((prvTgfPacketTrillPart.version >> 6) & 0x3);
    vfdIndex++;

    /* check TRILL reserved  */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;
    vfdArray[vfdIndex].cycleCount = 1;
    vfdArray[vfdIndex].patternPtr[0] = 0;
    vfdIndex++;

    /* check TRILL M bit  */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;
    vfdArray[vfdIndex].cycleCount = 1;
    vfdArray[vfdIndex].patternPtr[0] = (GT_U8)((prvTgfPacketTrillPart.mBit >> 3) & 0x1);
    vfdIndex++;

    /* check TRILL options length + hop count */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;
    vfdArray[vfdIndex].cycleCount = 2;
    vfdArray[vfdIndex].patternPtr[0] = (GT_U8)(prvTgfPacketTrillPart.opLength & 0x7);
    vfdArray[vfdIndex].patternPtr[1] = (GT_U8)((prvTgfPacketTrillPart.opLength & 0x3) << 6) | (GT_U8)((prvTgfPacketTrillPart.hopCount - 1) & 0x3F);
    vfdIndex++;

    /* check TRILL eRbid */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + 2;
    vfdArray[vfdIndex].cycleCount = 2;
    vfdArray[vfdIndex].patternPtr[0] = (GT_U8)((prvTgfPacketTrillPart.eRbid >> 8) & 0xFF);
    vfdArray[vfdIndex].patternPtr[1] = (GT_U8)(prvTgfPacketTrillPart.eRbid & 0xFF);
    vfdIndex++;

     /* check TRILL iRbid */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + 4;
    vfdArray[vfdIndex].cycleCount = 2;
    vfdArray[vfdIndex].patternPtr[0] = (GT_U8)((prvTgfPacketTrillPart.iRbid >> 8) & 0xFF);
    vfdArray[vfdIndex].patternPtr[1] = (GT_U8)(prvTgfPacketTrillPart.iRbid & 0xFF);
    vfdIndex++;

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface,
                                                        vfdIndex, vfdArray, &triggerBitmap);
    PRV_UTF_LOG2_MAC("triggerBitmap = %d, rc = 0x%02X\n\n", triggerBitmap, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                    portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    for(packetIter = 0; packetIter < prvTgfBurstCount; packetIter++)
    {
        /* check if captured packet has all correct vfd's */
        UTF_VERIFY_EQUAL1_STRING_MAC((1 << vfdIndex) - 1,
                                     triggerBitmap & ((1 << vfdIndex) - 1),
                                     "   Errors while triggers summary result checking: packetIter =  %d\n", packetIter);

        /* verify the TRILL version */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, triggerBitmap & BIT_0,
                                     "\n   TRILL Version of captured packet must be: %02X",
                                     prvTgfPacketTrillPart.version);

        /* verify the TRILL reserved bits */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_1) >> 1,
                                     "\n   TRILL reserved bits of captured packet must be: 0");
        /* verify the TRILL M bit */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_2) >> 2,
                                     "\n   TRILL M bit must be: %02X",
                                     (prvTgfPacketTrillPart.mBit));

        /* verify the TRILL options length + TRILL header <Hop Count> is decremented */
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_3) >> 3,
                                     "\n   TRILL options length must be: %02X\n   TRILL Hop Count must be: %02X",
                                     prvTgfPacketTrillPart.opLength, (prvTgfPacketTrillPart.hopCount - 1));

        /* verify the TRILL eRbid */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_4) >> 4,
                                     "\n   TRILL eRbid must be: %02X",
                                     (prvTgfPacketTrillPart.eRbid));

        /* verify the TRILL iRbid */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_5) >> 5,
                                     "\n   TRILL iRbid must be: %02X",
                                     (prvTgfPacketTrillPart.iRbid));

        /* prepare next iteration */
        triggerBitmap = triggerBitmap >> vfdIndex;
    }
}

/**
* @internal prvTgfSendNotValidPacket function
* @endinternal
*
* @brief   send regular (no options) not valid packet with one bad exception
*
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] transPacketLen           - transmitted packet length
*                                       None
*/
static GT_VOID prvTgfSendNotValidPacket
(
    IN TGF_PACKET_STC               *packetInfoPtr,
    IN GT_U32                       transPacketLen
)
{
    GT_STATUS               rc = GT_OK;
    GT_U32                  portIter = 0;
    GT_U8                   packetsCountRx;
    GT_U8                   packetsCountTx;
    GT_U32                  packetSize = 0;
    /*GT_U32                  dropCounter;*/

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    packetInfoPtr, prvTgfBurstCount, 0, NULL,
                    prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if (portIter == PRV_TGF_SEND_PORT_IDX_CNS)
        {
            packetsCountTx = (GT_U8)prvTgfBurstCount;
            packetsCountRx = (GT_U8)prvTgfBurstCount;
            packetSize     = transPacketLen;
        }
        else
        {
            packetsCountRx = 0;
            packetsCountTx = 0;
            packetSize = 0;
        }

        /* check ETH counters */
        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                    prvTgfPortsArray[portIter],
                                    packetsCountRx,
                                    packetsCountTx,
                                    packetSize,
                                    prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
  }

    /* verify exception counter */
     /*rc = prvTgfTtiTrillDropCounterGet(prvTgfDevNum, exception, &dropCounter);
     UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfTtiTrillDropCounterGet: exception %d dropCounter %d\n",
                    exception,prvTgfBurstCount);*/
}

/******************************************************************************/

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelTrillTtiExceptionsCheckConfigSet function
* @endinternal
*
* @brief   Set configuration
*/
GT_VOID prvTgfTunnelTrillTtiExceptionsCheckConfigSet
(
    GT_VOID
)
{
    GT_STATUS                                   rc = GT_OK;
    PRV_TGF_TTI_RULE_UNT                        pattern;
    PRV_TGF_TTI_RULE_UNT                        mask;
    PRV_TGF_TTI_ACTION_2_STC                    ruleAction = {0};
    PRV_TGF_TTI_TRILL_ADJACENCY_STC             adjacencyPattern;
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMeMask;
    PRV_TGF_TUNNEL_START_ENTRY_UNT              tunnelEntry;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC            egressInfo;
    CPSS_INTERFACE_INFO_STC                     physicalInfo;
    PRV_TGF_TCAM_BLOCK_INFO_STC                 halfFloorInfoCfg[6] = {{PRV_TGF_TTI_TCAM_CLIENT_GROUP_CNS, 0},{PRV_TGF_TTI_TCAM_CLIENT_GROUP_CNS, 1}};
    PRV_TGF_TCAM_BLOCK_INFO_STC                 halfFloorInfoCfgAc3x[6] = {{PRV_TGF_TTI_TCAM_CLIENT_GROUP_CNS, 0},{PRV_TGF_TTI_TCAM_CLIENT_GROUP_CNS, 0},{PRV_TGF_TTI_TCAM_CLIENT_GROUP_CNS, 0},{PRV_TGF_TTI_TCAM_CLIENT_GROUP_CNS, 1},{PRV_TGF_TTI_TCAM_CLIENT_GROUP_CNS, 1},{PRV_TGF_TTI_TCAM_CLIENT_GROUP_CNS, 1}};

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &pattern, 0, sizeof(pattern));
    cpssOsMemSet((GT_VOID*) &mask,    0, sizeof(mask));
    cpssOsMemSet((GT_VOID*) &adjacencyPattern, 0, sizeof(adjacencyPattern));
    cpssOsMemSet((GT_VOID*) &macToMePattern,   0, sizeof(macToMePattern));
    cpssOsMemSet((GT_VOID*) &macToMeMask,      0, sizeof(macToMeMask));
    cpssOsMemSet((GT_VOID*) &tunnelEntry,      0, sizeof(tunnelEntry));
    cpssOsMemSet((GT_VOID*) &egressInfo,       0, sizeof(egressInfo));
    cpssOsMemSet((GT_VOID*) &physicalInfo,     0, sizeof(physicalInfo));

    /* AUTODOC: create TRILL VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_TRILL_VLANID_CNS, prvTgfPortsArray,
                                           NULL, NULL, 4);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefVlanEntryWithPortsSet);

    /* save TRILL engine enable configuration on transmit port */
    rc = prvTgfTtiPortTrillEnableGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &(prvTgfRestoreCfg.trillEnableTransmitPort));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillEnableGet);

    /* AUTODOC: enable TRILL engine on port 0 */
    rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillEnableSet);

    /* AUTODOC: save TRILL engine configuration on destination ePort */
    rc = prvTgfTtiPortTrillEnableGet(prvTgfDevNum, PRV_TGF_DESTINATION_EPORT_CNS, &(prvTgfRestoreCfg.trillEnableDestinationPort));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillEnableGet);

    /* AUTODOC: enable TRILL engine on destination ePort */
    rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum, PRV_TGF_DESTINATION_EPORT_CNS, GT_TRUE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillEnableSet);

    rc = prvTgfTtiEthernetTypeGet(prvTgfDevNum,
                                    PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED0_E, &(prvTgfRestoreCfg.etherType));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEthernetTypeGet: %d",
                                    prvTgfDevNum);

    /* AUTODOC: for PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED0_E set ethertype to 0x22F3 (TRILL) */
    rc = prvTgfTtiEthernetTypeSet(prvTgfDevNum,
                                    PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED0_E, TGF_ETHERTYPE_22F3_TRILL_TAG_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEthernetTypeSet: %d",
                                    prvTgfDevNum);

    /* AUTODOC: enable TTI lookup for port 0, flexible key TTI_KEY_UDB */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_UDB_UDE_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    rc = prvTgfTtiPacketTypeKeySizeGet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_UDE_E, &(prvTgfRestoreCfg.keySize));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeGet: %d", prvTgfDevNum);

    /* AUTODOC: set key size for PRV_TGF_TTI_KEY_UDB_UDE_E */
    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_UDE_E, PRV_TGF_TTI_KEY_SIZE_30_B_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /* configure TTI rule for TRILL */
    pattern.udbArray[0] = GT_FALSE;

    /* configure TTI rule mask for TRILL */
    mask.udbArray[0] = GT_TRUE;

    /* AUTODOC: save UBB 0 for PRV_TGF_TTI_KEY_UDB_UDE_E */
    rc = prvTgfTtiUserDefinedByteGet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_UDE_E, 0 , &prvTgfRestoreCfg.udbOffsetType[0], &prvTgfRestoreCfg.udbOffset[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteGet: %d", prvTgfDevNum);

    /* AUTODOC: set UBB 0 for PRV_TGF_TTI_KEY_UDB_UDE_E */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_UDE_E, 0, PRV_TGF_TTI_OFFSET_METADATA_E,20);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    /* configure TTI rule action */
    ruleAction.command                      = CPSS_PACKET_CMD_FORWARD_E;
    ruleAction.sourceEPortAssignmentEnable  = GT_TRUE;
    ruleAction.sourceEPort                  = PRV_TGF_SOURCE_EPORT_CNS;
    ruleAction.mplsCommand                  = CPSS_DXCH_TTI_MPLS_NOP_CMD_E;
    ruleAction.continueToNextTtiLookup      = GT_TRUE;

    if (!HARD_WIRE_TCAM_MAC(prvTgfDevNum))
    {
        /* AUTODOC: save TTI TCAM client group */
        rc = prvTgfTcamClientGroupGet(PRV_TGF_TCAM_TTI_E, &(prvTgfRestoreCfg.ttiTcamClientGroup), &(prvTgfRestoreCfg.ttiTcamClientGroupEnable));
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTcamClientGroupGet");

        /* AUTODOC: set TTI TCAM client group */
        rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_TTI_E, PRV_TGF_TTI_TCAM_CLIENT_GROUP_CNS, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTcamClientGroupSet");
    }

    /* AUTODOC: save PRV_TGF_TTI_TCAM_FLOOR_CNS configuration */
    rc = prvTgfTcamIndexRangeHitNumAndGroupGet(PRV_TGF_TTI_TCAM_FLOOR_CNS, &prvTgfRestoreCfg.halfFloorInfoCfg[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupGet");

    /* AUTODOC: set PRV_TGF_TTI_TCAM_FLOOR_CNS configuration */
    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.tcam.numBanksForHitNumGranularity == 6)
    {
        rc = prvTgfTcamIndexRangeHitNumAndGroupSet(PRV_TGF_TTI_TCAM_FLOOR_CNS, halfFloorInfoCfgAc3x);
    }
    else
    {
         rc = prvTgfTcamIndexRangeHitNumAndGroupSet(PRV_TGF_TTI_TCAM_FLOOR_CNS, halfFloorInfoCfg);
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupSet");

    /* AUTODOC:   add TTI-0 rule */
    rc = prvTgfTtiRule2Set(PRV_TGF_TTI_0_RULE_INDEX_CNS, PRV_TGF_TTI_RULE_UDB_30_E,
                          &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");

    cpssOsMemSet(&ruleAction, 0, sizeof(ruleAction));
    ruleAction.command                 = CPSS_PACKET_CMD_FORWARD_E;
    ruleAction.redirectCommand         = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ruleAction.egressInterface.devPort.portNum = PRV_TGF_DESTINATION_EPORT_CNS;
    ruleAction.ingressPipeBypass       = GT_TRUE;
    ruleAction.enableDecrementTtl      = GT_TRUE;
    ruleAction.mplsCommand             = CPSS_DXCH_TTI_MPLS_NOP_CMD_E;
    ruleAction.mplsTtl                 = 0;
    ruleAction.tunnelTerminate         = GT_FALSE;
    ruleAction.tunnelStart             = GT_FALSE;
    ruleAction.tunnelStartPtr          = PRV_TGF_TUNNEL_START_INDEX_CNS;
    ruleAction.ttPassengerPacketType   = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
    ruleAction.tag0VlanCmd             = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;

    /* AUTODOC:   add TTI-1 rule */
    rc = prvTgfTtiRule2Set(PRV_TGF_TTI_1_RULE_INDEX_CNS, PRV_TGF_TTI_RULE_UDB_30_E,
                          &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");

     /* save ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       PRV_TGF_DESTINATION_EPORT_CNS,
                                                       &(prvTgfRestoreCfg.egressInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoGet);

    /* set ePort attributes configuration */
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = PRV_TGF_TUNNEL_START_INDEX_CNS;
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    /* AUTODOC: config ePort 1000 attributes: */
    /* AUTODOC:   Tunnel Start, tunnelStartPtr=8 */
    /* AUTODOC:   tsPassenger=ETHERNET */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       PRV_TGF_DESTINATION_EPORT_CNS,
                                                       &egressInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* save destination ePort mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT_CNS,
                                                           &(prvTgfRestoreCfg.physicalInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableGet);

    /* set ePort mapping configuration */
    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    /* AUTODOC: map port 3 to ePort 1000 */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT_CNS,
                                                           &physicalInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);

    /* set TRILL tunnel start entry */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    tunnelEntry.trillCfg.vlanId                 = PRV_TGF_TRILL_VLANID_CNS;
    tunnelEntry.trillCfg.tagEnable              = GT_TRUE;
    tunnelEntry.trillCfg.trillHeader.version    = 0;
    tunnelEntry.trillCfg.trillHeader.mBit       = 0;
    tunnelEntry.trillCfg.trillHeader.opLength   = 0;
    cpssOsMemCpy(tunnelEntry.trillCfg.nextHopMacDa.arEther, prvTgfTunnelStartEntryMacDa, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: add Tunnel Start entry 8 with: */
    /* AUTODOC:   tunnelType=TRILL */
    /* AUTODOC:   vlanId=5, version=mBit=opLength=0 */
    /* AUTODOC:   DA=66:55:44:33:22:11 */
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS, CPSS_TUNNEL_TRILL_E, &tunnelEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTunnelStartEntrySet);

     /* set the Outer Tag0 VID that must be for all TRILL packets from the port
        The TRILL standard specifies that all TRILL packets on a link, if tagged, must use the VLAN-ID as specified by the designated RBridge */
    rc = prvTgfTtiPortTrillOuterVid0Get(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &(prvTgfRestoreCfg.trillOuterVid0));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillOuterVid0Get);

    /* AUTODOC: set Outer Tag0 VID to 5 for all TRILL packets from port 0 */
    rc = prvTgfTtiPortTrillOuterVid0Set(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TRILL_VLANID_CNS);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillOuterVid0Set);

    /* set MAC to ME table */
    cpssOsMemCpy((GT_VOID*)macToMePattern.mac.arEther,
                 (GT_VOID*)prvTgfPacketL2Part.daMac,
                 sizeof(macToMePattern.mac.arEther));
    macToMePattern.vlanId = PRV_TGF_TRILL_VLANID_CNS;

    cpssOsMemSet((GT_VOID*)macToMeMask.mac.arEther, 0xFF,
                 sizeof(macToMeMask.mac.arEther));
    macToMeMask.vlanId = 0xFFF;

    /* AUTODOC: config MAC to ME table 1: */
    /* AUTODOC:   DA=00:55:44:33:22:11, VID=5 */
    rc = prvTgfTtiMacToMeSet(PRV_TGF_MAC_TO_ME_INDEX_CNS, &macToMePattern, &macToMeMask);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiMacToMeSet);

    /* TRILL Adjacency Check Configuration */
    cpssOsMemCpy((GT_VOID*)adjacencyPattern.outerMacSa.arEther,
                 (GT_VOID*)prvTgfPacketL2Part.saMac,
                 sizeof(adjacencyPattern.outerMacSa.arEther));
    adjacencyPattern.trillMBit      = 0;
    adjacencyPattern.srcHwDevice      = prvTgfDevNum;
    adjacencyPattern.srcIsTrunk     = 0;
    adjacencyPattern.srcPortTrunk   = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    /* AUTODOC: config entry 1 in TRILL Adjacency Check dedicated TCAM: */
    /* AUTODOC:   SA=00:00:00:00:00:05, port=0 */
    rc = prvTgfTtiTrillAdjacencyCheckEntrySet(prvTgfDevNum, PRV_TGF_ADJACENCY_INDEX_CNS, &adjacencyPattern);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiTrillAdjacencyCheckEntrySet);

    /* save TRILL cpu code base */
    rc = prvTgfTtiTrillCpuCodeBaseGet(prvTgfDevNum,&(prvTgfRestoreCfg.cpuCodeBase));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiTrillCpuCodeBaseGet);

    /* AUTODOC: set TRILL cpu code base 200 */
    rc = prvTgfTtiTrillCpuCodeBaseSet(prvTgfDevNum,PRV_TGF_CPU_CODE_BASE_CNS);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiTrillCpuCodeBaseSet);

    /* save TRILL max version */
    rc = prvTgfTtiTrillMaxVersionGet(prvTgfDevNum,&(prvTgfRestoreCfg.maxTrillVersion));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiTrillMaxVersionGet);

    /* AUTODOC: set max TRILL version 0 */
    rc = prvTgfTtiTrillMaxVersionSet(prvTgfDevNum, PRV_TGF_TRILL_MAX_VERSION);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiTrillMaxVersionSet);
}

/**
* @internal prvTgfTunnelTrillTtiExceptionsCheckTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfTunnelTrillTtiExceptionsCheckTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    PRV_TGF_TTI_MAC_VLAN_STC    macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC    macToMeMask;
    /*PRV_TGF_TTI_TRILL_RBID_LTT_TABLE_ENTRY_STC  iRbidLttEntry;
    PRV_TGF_TTI_TRILL_RBID_LTT_TABLE_ENTRY_STC  eRbidLttEntry;*/
    PRV_TGF_TTI_TRILL_ADJACENCY_STC             adjacencyPattern;
    /* AUTODOC: GENERATE TRAFFIC: */
    prvTgfExceptionsInit();

    PRV_UTF_LOG0_MAC("\n======= TRILL Valid Packet =======\n");

    prvTgfSendValidPacket(&prvTgfPacketInfo,PRV_TGF_TRANSMITTED_PACKET_LEN_CNS);

    PRV_UTF_LOG0_MAC("\n======= TRILL Adjacency Exception Checking =======\n");

    PRV_UTF_LOG0_MAC("\n======= TRILL Verify IS-IS Adjacency =======\n");

    /* TRILL IS-IS Adjacency Check: Checks that the single-destination TRILL frame arrives from a {neighbor, port} for which an IS-IS adjacency exists.
       If not the global configurable <TRILL IS-IS Adjacency Cmd> is assigned to the packet, and its respective exception counter is incremented */

    /* TRILL Adjacency Check Configuration */
    cpssOsMemCpy((GT_VOID*)adjacencyPattern.outerMacSa.arEther,
                 (GT_VOID*)prvTgfPacketL2Part.daMac,
                 sizeof(adjacencyPattern.outerMacSa.arEther));
    adjacencyPattern.trillMBit      = 0;
    adjacencyPattern.srcHwDevice      = prvTgfDevNum;
    adjacencyPattern.srcIsTrunk     = 0;
    adjacencyPattern.srcPortTrunk   = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    adjacencyPattern.trillEgressRbid = 0;

    /* AUTODOC: config entry 1 in TRILL Adjacency Check dedicated TCAM: */
    /* AUTODOC:   SA=00:00:00:00:00:05, port=0 */
    rc = prvTgfTtiTrillAdjacencyCheckEntrySet(prvTgfDevNum, PRV_TGF_ADJACENCY_INDEX_CNS, &adjacencyPattern);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiTrillAdjacencyCheckEntrySet);

    /* AUTODOC: send Tunneled packet from port 0 with: */
    /* AUTODOC:   DA=00:55:44:33:22:11, SA=00:00:00:00:00:05, VID=5 */
    /* AUTODOC:   EtherType=0x22F3, hopCount=0x20, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=5 */
    prvTgfSendNotValidPacket(&prvTgfPacketInfo,PRV_TGF_TRANSMITTED_PACKET_LEN_CNS);
    /* AUTODOC: verify to get no traffic */

    /* restore TRILL Adjacency Check Configuration */
    cpssOsMemCpy((GT_VOID*)adjacencyPattern.outerMacSa.arEther,
                 (GT_VOID*)prvTgfPacketL2Part.saMac,
                 sizeof(adjacencyPattern.outerMacSa.arEther));
    adjacencyPattern.trillMBit      = 0;
    adjacencyPattern.srcHwDevice      = prvTgfDevNum;
    adjacencyPattern.srcIsTrunk     = 0;
    adjacencyPattern.srcPortTrunk   = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    adjacencyPattern.trillEgressRbid = 0;

    PRV_UTF_LOG0_MAC("\n======= TRILL Verify Tree Adjacency =======\n");

    /* TRILL Tree Adjacency Check: Checks that the multi-destination TRILL frame arrives from a {neighbor, port} that is a branch on the given TRILL distribution tree.
       If not the global configurable <TRILL Tree Adjacency Cmd> is assigned to the packet, and its respective exception counter is incremented */

    /* setup packet */
    prvTgfPacketTrillPart.mBit = GT_TRUE;
    cpssOsMemCpy((GT_VOID*)prvTgfPacketL2Part.daMac,
                 (GT_VOID*)prvTgfPacketL2PartMacDa,
                 sizeof(TGF_MAC_ADDR));

    /* TRILL Adjacency Check Configuration */
    cpssOsMemCpy((GT_VOID*)adjacencyPattern.outerMacSa.arEther,
                 (GT_VOID*)prvTgfPacketL2Part.daMac,
                 sizeof(adjacencyPattern.outerMacSa.arEther));

    adjacencyPattern.trillMBit      = 0;
    adjacencyPattern.srcHwDevice      = prvTgfDevNum;
    adjacencyPattern.srcIsTrunk     = 0;
    adjacencyPattern.srcPortTrunk   = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    adjacencyPattern.trillEgressRbid = 0;

    /* AUTODOC: config entry 1 in TRILL Adjacency Check dedicated TCAM: */
    /* AUTODOC:   SA=00:55:44:33:22:11, port=0 */
    rc = prvTgfTtiTrillAdjacencyCheckEntrySet(prvTgfDevNum, PRV_TGF_ADJACENCY_INDEX_CNS, &adjacencyPattern);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiTrillAdjacencyCheckEntrySet);

    /* AUTODOC: send Tunneled packet from port 0 with: */
    /* AUTODOC:   DA=01:80:C2:00:00:40, SA=00:00:00:00:00:05, VID=5 */
    /* AUTODOC:   EtherType=0x22F3, mBit=1, hopCount=0x20, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=5 */
    prvTgfSendNotValidPacket(&prvTgfPacketInfo,PRV_TGF_TRANSMITTED_PACKET_LEN_CNS);
    /* AUTODOC: verify to get no traffic */

    /* restore packet */
    prvTgfPacketTrillPart.mBit = GT_FALSE;
    cpssOsMemCpy((GT_VOID*)prvTgfPacketL2Part.daMac,
                 (GT_VOID*)prvTgfPacketL2PartRestore,
                 sizeof(TGF_MAC_ADDR));

    /* restore TRILL Adjacency Check Configuration */
    cpssOsMemCpy((GT_VOID*)adjacencyPattern.outerMacSa.arEther,
                 (GT_VOID*)prvTgfPacketL2Part.saMac,
                 sizeof(adjacencyPattern.outerMacSa.arEther));
    adjacencyPattern.trillMBit      = 0;
    adjacencyPattern.srcHwDevice      = prvTgfDevNum;
    adjacencyPattern.srcIsTrunk     = 0;
    adjacencyPattern.srcPortTrunk   = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    adjacencyPattern.trillEgressRbid = 0;

    PRV_UTF_LOG0_MAC("\n======= TRILL Packet General Exception Checking =======\n");

    PRV_UTF_LOG0_MAC("\n======= TRILL Verify Version =======\n");

    /* TRILL Version Check: If the packet TRILL header version is greater than the global configurable <Max TRILL version>,
       the global configurable <TRILL bad Version Cmd> is assigned to the packet, and its respective exception counter is incremented */

    /* setup packet */
    prvTgfPacketTrillPart.version = 0x1;

    /* AUTODOC: send Tunneled packet from port 0 with: */
    /* AUTODOC:   DA=00:55:44:33:22:11, SA=00:00:00:00:00:05, VID=5 */
    /* AUTODOC:   EtherType=0x22F3, version=1, hopCount=0x20, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=5 */
    prvTgfSendNotValidPacket(&prvTgfPacketInfo,PRV_TGF_TRANSMITTED_PACKET_LEN_CNS);
    /* AUTODOC: verify to get no traffic */

    /* restore packet */
    prvTgfPacketTrillPart.version = 0;

    PRV_UTF_LOG0_MAC("\n======= TRILL Verify Hop Count =======\n");

    /* TRILL HOP Count Check: If the packet TRILL header Hop Count is ZERO, the global configurable <TRILL Hop count is Zero exception Cmd>
       is assigned to the packet, and its respective exception counter is incremented */

    /* setup packet */
    prvTgfPacketTrillPart.hopCount = 0;

    /* AUTODOC: send Tunneled packet from port 0 with: */
    /* AUTODOC:   DA=00:55:44:33:22:11, SA=00:00:00:00:00:05, VID=5 */
    /* AUTODOC:   EtherType=0x22F3, hopCount=0x0, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=5 */
    prvTgfSendNotValidPacket(&prvTgfPacketInfo,PRV_TGF_TRANSMITTED_PACKET_LEN_CNS);
    /* AUTODOC: verify to get no traffic */

    /* restore packet */
    prvTgfPacketTrillPart.hopCount = 0x20;

    PRV_UTF_LOG0_MAC("\n======= TRILL Verify Options Check =======\n");

    /* TRILL Options Check: If the packet TRILL header includes options (.i.e., the TRILL header Op-length field is greater than zero),
       the global configurable <TRILL Options exception Cmd> is assigned to the packet, and its respective exception counter is incremented */

    /* setup packet */
    prvTgfPacketTrillPart.opLength = 0x1;

    /* AUTODOC: send Tunneled packet from port 0 with: */
    /* AUTODOC:   DA=00:55:44:33:22:11, SA=00:00:00:00:00:05, VID=5 */
    /* AUTODOC:   EtherType=0x22F3, opLength=1, hopCount=0x20, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=5 */
    prvTgfSendNotValidPacket(&prvTgfPacketOptionsInfo,PRV_TGF_TRANSMITTED_OPTIONS_PACKET_LEN_CNS);
    /* AUTODOC: verify to get no traffic */

    /* restore packet */
    prvTgfPacketTrillPart.opLength = 0;

    PRV_UTF_LOG0_MAC("\n======= TRILL Verify Critical Hop-By-Hop =======\n");

    /* TRILL Critical Hop-By-Hop Check: If the packet TRILL header includes options and the CHBH bit is set, the global configurable <TRILL CHBH exception Cmd>
       is assigned to the packet, and its respective exception counter is incremented */

    /* setup packet */
    prvTgfPacketTrillPart.opLength = 0x1;
    prvTgfPacketTrillOptionsPart.chbh = GT_TRUE;

    /* AUTODOC: send Tunneled packet from port 0 with: */
    /* AUTODOC:   DA=00:55:44:33:22:11, SA=00:00:00:00:00:05, VID=5 */
    /* AUTODOC:   EtherType=0x22F3, chbh=1,opLength=1,hopCount=0x20,eRbid=0x6543,iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=5 */
    prvTgfSendNotValidPacket(&prvTgfPacketOptionsInfo,PRV_TGF_TRANSMITTED_OPTIONS_PACKET_LEN_CNS);
    /* AUTODOC: verify to get no traffic */

    /* restore packet */
    prvTgfPacketTrillPart.opLength = 0;
    prvTgfPacketTrillOptionsPart.chbh = GT_FALSE;

    PRV_UTF_LOG0_MAC("\n======= TRILL Verify Bad Outer VLAN-ID =======\n");

    /* TRILL Bad Outer VLAN-ID Check: The TRILL standard specifies that all TRILL packets on a link, if tagged, must use the VLAN-ID as specified by the
       designated RBridge. If the packet TRILL outer tag VLAN-ID (Tag0 VLAN-ID) does not equal the global configurable <TRILL Outer Tag0 VID>, the global
       configurable <TRILL Bad Outer VID exception Cmd> is assigned to the packet, and its respective exception counter is incremented */

    /* setup packet */
    prvTgfPacketVlanTagPart.vid = 0x3;

    /* AUTODOC: send Tunneled packet from port 0 with: */
    /* AUTODOC:   DA=00:55:44:33:22:11, SA=00:00:00:00:00:05, VID=3 */
    /* AUTODOC:   EtherType=0x22F3, hopCount=0x20, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=5 */
    prvTgfSendNotValidPacket(&prvTgfPacketInfo,PRV_TGF_TRANSMITTED_PACKET_LEN_CNS);
    /* AUTODOC: verify to get no traffic */

    /* restore packet */
    prvTgfPacketVlanTagPart.vid = PRV_TGF_TRILL_VLANID_CNS;

    PRV_UTF_LOG0_MAC("\n======= TRILL Verify UC MAC2ME =======\n");

    /* TRILL UC MAC2ME Check: TRILL transit Unicast traffic, for which this RBridge is the nexthop, must have the MAC DA of this RBridge. Otherwise, the packet
       should be dropped by this RBridge. This is a normal occurrence if the link is attached to a bridged LAN and there are more than two peer RBridges on this link.
       It is required that the MAC DA's of this RBridge are configured in the MAC2ME table.
       If the packet is Unicast TRILL and its outer MAC DA does not match one of the of the MAC addresses configured in the MAC2ME table, the global configurable
       <TRILL UC Not MAC2ME exception Cmd> is assigned to the packet, and its respective exception counter is incremented */

    /* setup packet */
    cpssOsMemCpy((GT_VOID*)macToMePattern.mac.arEther,
                 (GT_VOID*)prvTgfPacketL2Part.daMac,
                 sizeof(macToMePattern.mac.arEther));
    macToMePattern.vlanId = 3;

    cpssOsMemSet((GT_VOID*)macToMeMask.mac.arEther, 0xFF,
                 sizeof(macToMeMask.mac.arEther));
    macToMeMask.vlanId = 0xFFF;

    /* AUTODOC: config MAC to ME table 1: */
    /* AUTODOC:   DA=00:55:44:33:22:11, VID=3 */
    rc = prvTgfTtiMacToMeSet(PRV_TGF_MAC_TO_ME_INDEX_CNS, &macToMePattern, &macToMeMask);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiMacToMeSet);

    /* AUTODOC: send Tunneled packet from port 0 with: */
    /* AUTODOC:   DA=00:55:44:33:22:11, SA=00:00:00:00:00:05, VID=5 */
    /* AUTODOC:   EtherType=0x22F3, hopCount=0x20, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=5 */
    prvTgfSendNotValidPacket(&prvTgfPacketInfo,PRV_TGF_TRANSMITTED_PACKET_LEN_CNS);
    /* AUTODOC: verify to get no traffic */

    /* restore packet */
    macToMePattern.vlanId = PRV_TGF_TRILL_VLANID_CNS;

    /* AUTODOC: config MAC to ME table 1: */
    /* AUTODOC:   DA=00:55:44:33:22:11, VID=5 */
    rc = prvTgfTtiMacToMeSet(PRV_TGF_MAC_TO_ME_INDEX_CNS, &macToMePattern, &macToMeMask);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiMacToMeSet);

    PRV_UTF_LOG0_MAC("\n======= TRILL Verify MC M-bit Consistency =======\n");

    /* TRILL MC M-bit Consistency Check: If the TRILL outer MAC DA is Multicast, and the TRILL header M-bit is clear, the global configurable
       <TRILL MC M-bit exception Cmd> is assigned to the packet, and its respective exception counter is incremented */

    /* setup packet */
    prvTgfPacketTrillPart.mBit = GT_FALSE;
    cpssOsMemCpy((GT_VOID*)prvTgfPacketL2Part.daMac,
                 (GT_VOID*)prvTgfPacketL2PartMacDa,
                 sizeof(TGF_MAC_ADDR));

    /* AUTODOC: send Tunneled packet from port 0 with: */
    /* AUTODOC:   DA=01:80:C2:00:00:40, SA=00:00:00:00:00:05, VID=5 */
    /* AUTODOC:   EtherType=0x22F3, hopCount=0x20, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=5 */
    prvTgfSendNotValidPacket(&prvTgfPacketInfo,PRV_TGF_TRANSMITTED_PACKET_LEN_CNS);
    /* AUTODOC: verify to get no traffic */

    /* restore packet */
    cpssOsMemCpy((GT_VOID*)prvTgfPacketL2Part.daMac,
                 (GT_VOID*)prvTgfPacketL2PartRestore,
                 sizeof(TGF_MAC_ADDR));
    prvTgfPacketTrillPart.mBit = GT_FALSE;

    PRV_UTF_LOG0_MAC("\n======= TRILL Verify UC M-bit Consistency =======\n");

    /* TRILL UC M-bit Consistency Check: If the TRILL outer MAC DA is UC, and the TRILL header M-bit is set, the global configurable
       <TRILL UC M-bit exception Cmd> is assigned to the packet, and its respective exception counter is incremented */

    /* setup packet */
    prvTgfPacketTrillPart.mBit = GT_TRUE;

    /* AUTODOC: send Tunneled packet from port 0 with: */
    /* AUTODOC:   DA=00:55:44:33:22:11, SA=00:00:00:00:00:05, VID=5 */
    /* AUTODOC:   EtherType=0x22F3, mBit=1,hopCount=0x20,eRbid=0x6543,iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=5 */
    prvTgfSendNotValidPacket(&prvTgfPacketInfo,PRV_TGF_TRANSMITTED_PACKET_LEN_CNS);
    /* AUTODOC: verify to get no traffic */

    /* restore packet */
    prvTgfPacketTrillPart.mBit = GT_FALSE;

    PRV_UTF_LOG0_MAC("\n======= TRILL Verify Outer UC Inner MC =======\n");

    /* TRILL Outer UC Inner MC Check: If the TRILL outer MAC DA is UC, and the inner DA is MC, the global configurable
       <TRILL outer UC inner MC exception Cmd> is assigned to the packet, and its respective exception counter is incremented */

    /* setup packet */
    cpssOsMemCpy((GT_VOID*)prvTgfPacketL2PartInt.daMac,
                 (GT_VOID*)prvTgfPacketL2PartMacDa,
                 sizeof(TGF_MAC_ADDR));

    /* AUTODOC: send Tunneled packet from port 0 with: */
    /* AUTODOC:   DA=00:55:44:33:22:11, SA=00:00:00:00:00:05, VID=5 */
    /* AUTODOC:   EtherType=0x22F3, hopCount=0x20,eRbid=0x6543,iRbid=0x4321 */
    /* AUTODOC:   passenger DA=01:80:C2:00:00:40, SA=00:00:00:00:00:22, VID=5 */
    prvTgfSendNotValidPacket(&prvTgfPacketInfo,PRV_TGF_TRANSMITTED_PACKET_LEN_CNS);
    /* AUTODOC: verify to get no traffic */

    /* restore packet */
    cpssOsMemCpy((GT_VOID*)prvTgfPacketL2PartInt.daMac,
                 (GT_VOID*)prvTgfPacketL2PartIntRestore,
                 sizeof(TGF_MAC_ADDR));

    PRV_UTF_LOG0_MAC("\n======= TRILL Verify MC DA =======\n");

    /* TRILL MC DA Check: If the TRILL header M-bit is set and the outer MAC DA is not the global configurable <All-RBridges Multicast address>,
       the global configurable <TRILL Bad MC DA exception Cmd> is assigned to the packet, and its respective exception counter is incremented */

    /* setup packet */
    prvTgfPacketTrillPart.mBit = GT_TRUE;
    cpssOsMemCpy((GT_VOID*)prvTgfPacketL2Part.daMac,
                 (GT_VOID*)prvTgfPacketL2PartMacDaEsadi,
                 sizeof(TGF_MAC_ADDR));

    /* AUTODOC: send Tunneled packet from port 0 with: */
    /* AUTODOC:   DA=01:80:C2:00:00:42, SA=00:00:00:00:00:05, VID=5 */
    /* AUTODOC:   EtherType=0x22F3, mBit=1,hopCount=0x20,eRbid=0x6543,iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=5 */
    prvTgfSendNotValidPacket(&prvTgfPacketInfo,PRV_TGF_TRANSMITTED_PACKET_LEN_CNS);
    /* AUTODOC: verify to get no traffic */

    /* restore packet */
    cpssOsMemCpy((GT_VOID*)prvTgfPacketL2Part.daMac,
                 (GT_VOID*)prvTgfPacketL2PartRestore,
                 sizeof(TGF_MAC_ADDR));
    prvTgfPacketTrillPart.mBit = GT_FALSE;
}

/**
* @internal prvTgfTunnelTrillTtiExceptionsCheckConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfTunnelTrillTtiExceptionsCheckConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMeMask;

    /* AUTODOC: RESTORE CONFIGURATION: */

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

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, tgfTrafficTableRxPcktTblClear);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_TRILL_VLANID_CNS);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefVlanEntryInvalidate);

    /* AUTODOC: restore TRILL ethertype 0x22F3 */
    rc = prvTgfTtiEthernetTypeSet(prvTgfDevNum, PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED0_E, prvTgfRestoreCfg.etherType);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiEthernetTypeSet);

    /* AUTODOC: restore TRILL engine state on port 0 */
    rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfRestoreCfg.trillEnableTransmitPort);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillEnableSet);

     /* restore trill engine enable on destination ePort */
    rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum, PRV_TGF_DESTINATION_EPORT_CNS, prvTgfRestoreCfg.trillEnableDestinationPort);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillEnableSet);

    /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_TRILL */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_UDB_UDE_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: restore key size for PRV_TGF_TTI_KEY_UDB_UDE_E */
    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_UDE_E, prvTgfRestoreCfg.keySize);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /* AUTODOC: restore UBB 0 for PRV_TGF_TTI_KEY_UDB_UDE_E */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_UDE_E, 0, prvTgfRestoreCfg.udbOffsetType[0], prvTgfRestoreCfg.udbOffset[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    if (!HARD_WIRE_TCAM_MAC(prvTgfDevNum))
    {
        /* AUTODOC: restore TTI TCAM client group */
        rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_TTI_E, prvTgfRestoreCfg.ttiTcamClientGroup, prvTgfRestoreCfg.ttiTcamClientGroupEnable);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTcamClientGroupSet");
    }

    /* AUTODOC: restore PRV_TGF_TTI_TCAM_FLOOR_CNS configuration */
    rc = prvTgfTcamIndexRangeHitNumAndGroupSet(PRV_TGF_TTI_TCAM_FLOOR_CNS, prvTgfRestoreCfg.halfFloorInfoCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupSet");

    /* AUTODOC: invalidate PRV_TGF_TTI_0_RULE_INDEX_CNS rule */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_0_RULE_INDEX_CNS, GT_FALSE);

    /* AUTODOC: invalidate PRV_TGF_TTI_1_RULE_INDEX_CNS rule */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_1_RULE_INDEX_CNS, GT_FALSE);

    /* AUTODOC: restore Outer Tag0 VID for all TRILL packets from port 0 */
    rc = prvTgfTtiPortTrillOuterVid0Set(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfRestoreCfg.trillOuterVid0);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillOuterVid0Set);

    /* AUTODOC: restore default of MAC to ME entry 1 */
    rc = prvTgfTtiMacToMeSet(PRV_TGF_MAC_TO_ME_INDEX_CNS, &macToMePattern, &macToMeMask);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiMacToMeSet);

    /* AUTODOC: restore ePort 1000 attributes */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       PRV_TGF_DESTINATION_EPORT_CNS,
                                                       &(prvTgfRestoreCfg.egressInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* AUTODOC: restore ePort PRV_TGF_DESTINATION_EPORT_CNS mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT_CNS,
                                                           &(prvTgfRestoreCfg.physicalInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);

    /* AUTODOC: restore TRILL CPU code base */
    rc = prvTgfTtiTrillCpuCodeBaseSet(prvTgfDevNum,prvTgfRestoreCfg.cpuCodeBase);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiTrillCpuCodeBaseSet);

    /* AUTODOC: restore TRILL max version */
    rc = prvTgfTtiTrillMaxVersionSet(prvTgfDevNum,prvTgfRestoreCfg.maxTrillVersion);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiTrillMaxVersionSet);

}

