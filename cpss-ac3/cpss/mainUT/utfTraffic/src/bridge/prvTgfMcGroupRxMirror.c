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
* @file prvTgfMcGroupRxMirror.c
*
* @brief Create MC groups
* Define Mirror and Analyzer ports.
* Send Packets to MC Group.
* Check that Analyzer port receives the packets too.
*
* @version   5
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTrunkGen.h>

#include <common/tgfMirror.h>

#include <bridge/prvTgfMcGroupRxMirror.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* capture port index */
#define PRV_TGF_CPORT_IDX_CNS               2

/* default VLAN Id */
#define PRV_TGF_VLAN_ID_CNS                 2

/* default vidx */
#define PRV_TGF_VIDX_CNS                    1

/* default McGroup members array */
static GT_U16  prvTgfMcPortIdxArr[] =
{
    0, 2
};

/* analyzer port index */
#define PRV_TGF_ANALYZER_PORTIDX_CNS        1

/* analyzer interface index */
#define PRV_TGF_ANALYZER_INTERFACEIDX_CNS   0

/* mirror port index */
#define PRV_TGF_MIRROR_PORTIDX_CNS          3

/* port index to send traffic to */
#define PRV_TGF_TX_PORTIDX_CNS              3

/* default number of packets to send */
static GT_U32  prvTgfBurstCount             = 1;

/* expected captured packet number */
static GT_U32  prvTgfExpCaptureCount        = 3;

/* expected number of Rx and Tx packets on ports*/
static GT_U8 prvTgfPacketsCountRxTxArr[][PRV_TGF_PORTS_NUM_CNS] =
{
    {0, 0, 0, 1},  /* Rx count for generate trafic */
    {1, 1, 1, 1}   /* Tx count for generate trafic */
};

/* default macAddr for vidx FDB entry */
static TGF_MAC_ADDR prvTgfVidxMacAddr =
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06
};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x01, 0x02, 0x03, 0x04, 0x05, 0x06},               /* dstMac */
    {0x00, 0x00, 0x11, 0x11, 0x11, 0x11}                /* srcMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, 2                                             /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x08, 0x00, 0x45, 0x00, 0x00, 0x2A, 0x00, 0x00,
    0x00, 0x00, 0x40, 0x04, 0x7A, 0xD1, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                           /* dataLength */
    prvTgfPayloadDataArr                                    /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},      /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
   TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    CPSS_PORTS_BMP_STC  mcGroupPortMembers;
    GT_U32              analyzerInterfaceIdx;
    GT_BOOL             portRxMirrorModeEnabled;
    GT_U8               analyzerPort;
    GT_U8               analyzerDev;
} prvTgfRestoreCfg;

/******************************************************************************/


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfMcGroupRxMirrorIsMcMember function
* @endinternal
*
* @brief   Check PortIdx is a McGroup member
*
* @param[in] portIdx                  - port index number
*
* @retval GT_TRUE                  - on portIdx is a McGroup member
* @retval GT_FALSE                 - on portIdx is not a McGroup member
*/
static GT_BOOL prvTgfMcGroupRxMirrorIsMcMember
(
    IN GT_U32      portIdx
)
{
    GT_U32          mcMemb      = 0;
    GT_U32          membersNum  = 0;

    membersNum = sizeof(prvTgfMcPortIdxArr)/sizeof(prvTgfMcPortIdxArr[0]);

    /* iterate thru all trunk members*/
    for (mcMemb = 0; mcMemb < membersNum; mcMemb++ )
    {
        /* check port index is equal to McGroup member index */
        if (portIdx == prvTgfMcPortIdxArr[mcMemb])
        {
            return GT_TRUE;
        }
    }
    return GT_FALSE;
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/**
* @internal prvTgfMcGroupRxMirrorConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfMcGroupRxMirrorConfigSet
(
    GT_VOID
)
{
    GT_STATUS                   rc              = GT_OK;
    GT_U32                      portIter        = 0;
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry        = {{0}};

    /* AUTODOC: SETUP CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Setup config =====================\n\n");

    PRV_UTF_LOG0_MAC("\nSet Vlan configuration\n");

    /* AUTODOC: create VLAN 2 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLAN_ID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite(%d)",
                                 PRV_TGF_VLAN_ID_CNS);

    /* save default port members for MC group */
    rc = prvTgfBrgMcEntryRead(prvTgfDevNum, PRV_TGF_VIDX_CNS,
                              &prvTgfRestoreCfg.mcGroupPortMembers);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgMcEntryRead: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VIDX_CNS);

    PRV_UTF_LOG0_MAC("\nCreate MC Group\n");

    /* AUTODOC: create VIDX 1 with ports 0, 2 */
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, PRV_TGF_VIDX_CNS, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgMcEntryWrite");

    /* add MC group members */
    for (portIter = 0; portIter < sizeof(prvTgfMcPortIdxArr)/sizeof(prvTgfMcPortIdxArr[0]); portIter++)
    {
        rc = prvTgfBrgMcMemberAdd(prvTgfDevNum, PRV_TGF_VIDX_CNS,
                                  prvTgfPortsArray[prvTgfMcPortIdxArr[portIter]]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgMcMemberAdd,"\
                                     "  portNum = %d",
                                     prvTgfPortsArray[prvTgfMcPortIdxArr[portIter]]);
    }

    PRV_UTF_LOG0_MAC("\nAdd MAC entry\n");

    /* clear entry */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

    /* fill mac entry by defaults */
    macEntry.key.entryType               = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.dstInterface.type           = CPSS_INTERFACE_VIDX_E;
    macEntry.dstInterface.vidx           = PRV_TGF_VIDX_CNS;
    macEntry.key.key.macVlan.vlanId      = PRV_TGF_VLAN_ID_CNS;
    macEntry.daCommand                   = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.saCommand                   = PRV_TGF_PACKET_CMD_FORWARD_E;

    /* set default vidx FDB MAC addr */
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfVidxMacAddr, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: add FDB entry with MAC 01:02:03:04:05:06, VLAN 2, VIDX 1 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbMacEntrySet");

    PRV_UTF_LOG0_MAC("\nMirror\n");

    /* save mirroring status of port */
    rc = prvTgfMirrorRxPortGet(prvTgfPortsArray[PRV_TGF_MIRROR_PORTIDX_CNS],
                               &prvTgfRestoreCfg.portRxMirrorModeEnabled,
                               &prvTgfRestoreCfg.analyzerInterfaceIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfMirrorRxPortGet");

    /* AUTODOC: set port 3 to be Rx mirrored port */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_MIRROR_PORTIDX_CNS],
                          GT_TRUE,
                          PRV_TGF_ANALYZER_INTERFACEIDX_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfMirrorRxPortSet");

    /* save analyzer port */
    rc = prvTgfMirrorRxAnalyzerPortGet(prvTgfDevNum,
                                       &prvTgfRestoreCfg.analyzerPort,
                                       &prvTgfRestoreCfg.analyzerDev);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfMirrorRxAnalyzerPortGet");

    /* AUTODOC: sets port 1 as analyzer port of Rx mirrored ports */
    rc = prvTgfMirrorRxAnalyzerPortSet(prvTgfPortsArray[PRV_TGF_ANALYZER_PORTIDX_CNS],
                                       prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfMirrorRxAnalyzerPortSet");
}

/**
* @internal prvTgfMcGroupRxMirrorTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfMcGroupRxMirrorTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc             = GT_OK;
    GT_U32      portIter       = 0;
    GT_U32      expPacketLen   = 0;
    GT_U32      numTriggers    = 0;
    GT_U32      rxPacketsCount = 0;
    GT_U32      txPacketsCount = 0;

    CPSS_INTERFACE_INFO_STC     portInterface = {0};
    TGF_VFD_INFO_STC            vfdArray[2];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

    /* reset all counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersReset");

    /* enable capture */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CPORT_IDX_CNS];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_PCL_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet\n");

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetupEth: dev=%d, burst=%d",
                                 prvTgfDevNum, prvTgfBurstCount);

    /* AUTODOC: send packet on port 3 with: */
    /* AUTODOC:   DA=01:02:03:04:05:06, SA=00:00:11:11:11:11, VID=2 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORTIDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitingEth: dev=%d, port=%d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORTIDX_CNS]);

    /* disable capture */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_PCL_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, portInterface.devPort.portNum);

    /* AUTODOC: verify packet on all ports (incl. Analyzer port) */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* expected number of Tx packets */
        txPacketsCount = prvTgfPacketsCountRxTxArr[1][portIter];

        /* expected number of Rx packets */
        rxPacketsCount = prvTgfPacketsCountRxTxArr[0][portIter]+
                         ((PRV_TGF_CPORT_IDX_CNS == portIter) * txPacketsCount);

        expPacketLen =  prvTgfMcGroupRxMirrorIsMcMember(portIter) ?
                       PRV_TGF_PACKET_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS :
                       PRV_TGF_PACKET_LEN_CNS;

        /* check ETH counters */
        rc = prvTgfEthCountersCheck(prvTgfDevNum, prvTgfPortsArray[portIter],
                                    rxPacketsCount, txPacketsCount, expPacketLen,
                                    prvTgfBurstCount);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                     "ERROR of prvTgfEthCountersCheck"\
                                     "  Port=%02d  expectedRx=%02d"\
                                     "  expectedTx=%02d"\
                                     "  expectedLen=%02d\n",
                                     prvTgfPortsArray[portIter],
                                     rxPacketsCount, txPacketsCount,
                                     expPacketLen);
    }

    /* get trigger counters */
    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* get trigger counters where packet has MAC DA as macAddr */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    /* get trigger counters where packet has MAC SA as macAddr */
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset = sizeof(TGF_MAC_ADDR);
    vfdArray[1].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[1].patternPtr, prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface,
                                                        sizeof(vfdArray) / sizeof(vfdArray[0]),
                                                        vfdArray, &numTriggers);
    PRV_UTF_LOG1_MAC("numTriggers = %d\n", numTriggers);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    /* check if captured packet has the same MAC DA as prvTgfArpMac */
    PRV_UTF_LOG1_MAC("expTriggers = %d\n\n", prvTgfExpCaptureCount);

    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfExpCaptureCount, numTriggers,"\n ERROR: Got another MAC DA/SA\n");

}

/**
* @internal prvTgfMcGroupRxMirrorConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfMcGroupRxMirrorConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc          = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Restore config: ==================\n\n");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: restore state of Rx mirrored port */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_MIRROR_PORTIDX_CNS],
                               prvTgfRestoreCfg.portRxMirrorModeEnabled,
                               prvTgfRestoreCfg.analyzerInterfaceIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfMirrorRxPortSet");

    /* AUTODOC: restore analyzer port of Rx mirrored ports */
    rc = prvTgfMirrorRxAnalyzerPortSet(prvTgfRestoreCfg.analyzerPort,
                                       prvTgfRestoreCfg.analyzerDev);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfMirrorRxAnalyzerPortSet");

    /* AUTODOC: restore VIDX entry */
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, PRV_TGF_VIDX_CNS, &prvTgfRestoreCfg.mcGroupPortMembers);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgMcEntryWrite");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbFlush");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLAN_ID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgDefVlanEntryInvalidate: vlan %d",
                                 PRV_TGF_VLAN_ID_CNS);
}


