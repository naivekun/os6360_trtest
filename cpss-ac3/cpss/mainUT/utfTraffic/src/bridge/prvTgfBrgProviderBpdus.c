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
* @file prvTgfBrgProviderBpdus.c
*
* @brief Provider BPDUs - Change Command
*
* @version   3
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
#include <bridge/prvTgfBrgProviderBpdus.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/*  default VLAN Id */
#define PRV_TGF_DEF_VLANID_CNS                  1

/*  VLAN Id 3 */
#define PRV_TGF_VLANID_2_CNS                    2

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS               0

/* capturing port number  */
#define PRV_TGF_CAPTURE_PORT_IDX_CNS            1

/* namber of packets to send */
#define PRV_TGF_SEND_PACKETS_NUM_CNS            4

/* profile index */
#define PRV_TGF_PROFILE_INDEX_CNS               0

/* protocol value */
#define PRV_TGF_PROTOCOL_NUM_CNS                2

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* expected number of Rx packets on ports*/
static GT_U8 prvTgfPacketsCountRxArr[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {1, 1, 0, 0},
    {1, 1, 0, 0},
    {1, 0, 0, 0},
    {1, 0, 0, 0}
};

/* expected number of Tx packets on ports*/
static GT_U8 prvTgfPacketsCountTxArr[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {1, 1, 1, 1},
    {1, 1, 1, 1},
    {1, 0, 0, 0},
    {1, 0, 0, 0}
};

/* config array of the packet command */
static CPSS_PACKET_CMD_ENT prvTgfPacketCmdArr[PRV_TGF_SEND_PACKETS_NUM_CNS] = 
{
    CPSS_PACKET_CMD_FORWARD_E,
    CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
    CPSS_PACKET_CMD_TRAP_TO_CPU_E,
    CPSS_PACKET_CMD_DROP_SOFT_E
};

/* expected number of capturing packets */
static GT_U8 numTriggersArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    3, 3, 0, 0
};

/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x01, 0x80, 0xc2, 0x00, 0x00, 0x22},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_2_CNS                          /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of tagged packet */
static TGF_PACKET_PART_STC prvTgfTaggedPacketPartArray[] = 
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of TAGGED packet */
#define PRV_TGF_TAGGED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* TAGGED PACKET to send */
static TGF_PACKET_STC prvTgfTaggedPacketInfo = 
{
    PRV_TGF_TAGGED_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfTaggedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfTaggedPacketPartArray                                        /* partsArray */
};

/******************************************************************************/
/*************************** Restore config ***********************************/

/* Parameters needed to be restored */

/* parametrs for restore default Vlan entry */
static struct
{
    CPSS_PORTS_BMP_STC                  portsMembers;
    CPSS_PORTS_BMP_STC                  portsTagging;
    GT_BOOL                             isValid;
    PRV_TGF_BRG_VLAN_INFO_STC           vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  taggingCmd;
} prvTgfRestoreCfg;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBrgProviderBpdusConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgProviderBpdusConfigSet
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U8                       tagArray[]  = {1, 1, 1, 1};


    /* AUTODOC: SETUP CONFIGURATION: */

    /* save default Vlan entry parametrs */
    rc = prvTgfBrgVlanEntryRead(prvTgfDevNum, PRV_TGF_DEF_VLANID_CNS,
                                &prvTgfRestoreCfg.portsMembers,
                                &prvTgfRestoreCfg.portsTagging,
                                &prvTgfRestoreCfg.isValid,
                                &prvTgfRestoreCfg.vlanInfo,
                                &prvTgfRestoreCfg.taggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryRead %d", prvTgfDevNum);

    /* invalidate VLAN 1 entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_DEF_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate 0");

    /* AUTODOC: create VLAN 2 with all tagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_2_CNS,
                                           prvTgfPortsArray,
                                           NULL, tagArray, 4);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: enable mirroring to CPU Multicast packets */
    rc = prvTgfBrgGenIeeeReservedMcastTrapEnable(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenIeeeReservedMcastTrapEnable: %d", prvTgfDevNum);

    /* AUTODOC: select IEEE Reserved MC profile 0 associated with port 0 */
    rc = prvTgfBrgGenPortIeeeReservedMcastProfileIndexSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenPortIeeeReservedMcastProfileIndexSet: %d", prvTgfDevNum);

    /* set the CPU code for specific IEEE reserved multicast */
    /* AUTODOC: set RSRVD_MC_ADDR CPU code for profile 0 and GARP protocol 2 */
    rc = prvTgfBrgSctIeeeReservedMcProtocolCpuCodeSet(PRV_TGF_PROFILE_INDEX_CNS, 
                                                      PRV_TGF_SCT_IEEE_RSV_MC_PROTOCOL_GARP_E,
                                                      PRV_TGF_PROTOCOL_NUM_CNS, 
                                                      CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgSctIeeeReservedMcProtocolCpuCodeSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfBrgProviderBpdusTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgProviderBpdusTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U8                       portIter = 0;
    GT_U8                       sendIter = 0;
    GT_U32                      numTriggers    = 0;
    TGF_VFD_INFO_STC            vfdArray[2];
    CPSS_INTERFACE_INFO_STC     portInterface;


    /* AUTODOC: GENERATE TRAFFIC: */

    for (sendIter = 0; sendIter < PRV_TGF_SEND_PACKETS_NUM_CNS; sendIter++)
    {
        /* set the command for specific IEEE reserved multicast */
        /* AUTODOC: set following cmds for profile 0 and standard protocol 2: */
        /* AUTODOC:   FORWARD */
        /* AUTODOC:   MIRROR_TO_CPU */
        /* AUTODOC:   TRAP_TO_CPU */
        /* AUTODOC:   DROP_SOFT */
        rc = prvTgfBrgSctIeeeReservedMcProtocolCmdSet(PRV_TGF_PROFILE_INDEX_CNS, 
                                                      PRV_TGF_SCT_IEEE_RSV_MC_PROTOCOL_GARP_E, 
                                                      PRV_TGF_PROTOCOL_NUM_CNS, 
                                                      prvTgfPacketCmdArr[sendIter]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgSctIeeeReservedMcProtocolCmdSet: %d", prvTgfDevNum);

        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* reset counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

        /* enable capture */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_PCL_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfTaggedPacketInfo,
                                 prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

        /* AUTODOC: for each cmd send packet on port 0 with: */
        /* AUTODOC:   DA=01:80:C2:00:00:22, SA=00:00:00:00:00:02, VID=2 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* disable capture */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_PCL_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, portInterface.devPort.portNum);

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   1 packet - received on ports 1,2,3 */
        /* AUTODOC:   2 packet - received on ports 1,2,3 */
        /* AUTODOC:   3 packet - trapped to CPU */
        /* AUTODOC:   4 packet - dropped */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* check ETH counters */
            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                        prvTgfPortsArray[portIter],
                                        prvTgfPacketsCountRxArr[sendIter][portIter],
                                        prvTgfPacketsCountTxArr[sendIter][portIter],
                                        PRV_TGF_TAGGED_PACKET_LEN_CNS, prvTgfBurstCount);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
        }

        /* get Trigger Counters */
        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

        /* clear VFD array */
        cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

        /* set vfd for destination MAC */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 0;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

        vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[1].modeExtraInfo = 0;
        vfdArray[1].offset = TGF_L2_HEADER_SIZE_CNS;
        vfdArray[1].cycleCount = 4;
        vfdArray[1].patternPtr[0] = TGF_ETHERTYPE_8100_VLAN_TAG_CNS >> 8;
        vfdArray[1].patternPtr[1] = TGF_ETHERTYPE_8100_VLAN_TAG_CNS & 0xFF;
        vfdArray[1].patternPtr[3] = PRV_TGF_VLANID_2_CNS;

        /* get trigger counters */
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 2, vfdArray, &numTriggers);
        PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

        UTF_VERIFY_EQUAL2_STRING_MAC(numTriggersArr[sendIter], numTriggers,
                         "get another trigger that expected: expected - %d, recieved - %d\n",
                         numTriggersArr[sendIter], numTriggers);
    }
}

/**
* @internal prvTgfBrgProviderBpdusConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgProviderBpdusConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;


    /* AUTODOC: RESTORE CONFIGURATION: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore the command for specific IEEE reserved multicast */
    rc = prvTgfBrgSctIeeeReservedMcProtocolCmdSet(PRV_TGF_PROFILE_INDEX_CNS, 
                                                  PRV_TGF_SCT_IEEE_RSV_MC_PROTOCOL_STD_E, 
                                                  PRV_TGF_PROTOCOL_NUM_CNS,
                                                  CPSS_PACKET_CMD_FORWARD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgSctIeeeReservedMcProtocolCmdSet: %d", prvTgfDevNum);

    /* AUTODOC: restore CPU code for specific IEEE reserved multicast */
    rc = prvTgfBrgSctIeeeReservedMcProtocolCpuCodeSet(PRV_TGF_PROFILE_INDEX_CNS, 
                                                      PRV_TGF_SCT_IEEE_RSV_MC_PROTOCOL_STD_E,
                                                      PRV_TGF_PROTOCOL_NUM_CNS,
                                                      CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgSctIeeeReservedMcProtocolCpuCodeSet: %d", prvTgfDevNum);

    /* AUTODOC: reset mirroring to CPU Multicast packets */
    rc = prvTgfBrgGenIeeeReservedMcastTrapEnable(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenIeeeReservedMcastTrapEnable: %d", prvTgfDevNum);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_2_CNS);

    /* AUTODOC: restore default VLAN 1 on all ports */
    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum, PRV_TGF_DEF_VLANID_CNS,
                                &prvTgfRestoreCfg.portsMembers,
                                &prvTgfRestoreCfg.portsTagging,
                                &prvTgfRestoreCfg.vlanInfo,
                                &prvTgfRestoreCfg.taggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite %d", prvTgfDevNum);
}


