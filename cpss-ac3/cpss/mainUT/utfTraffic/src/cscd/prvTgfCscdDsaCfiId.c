/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfCscdCfiId.c
*
* DESCRIPTION:
*       CFI Id test in Dsa tag.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
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
#include <extUtils/trafficEngine/private/prvTgfTrafficParser.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCscdGen.h>
#include <common/tgfCosGen.h>
#include <cscd/prvTgfCscdDsaCfiId.h>

#ifdef CHX_FAMILY
#include <cpss/generic/cos/cpssCosTypes.h>
#include <cpss/dxCh/dxChxGen/cos/cpssDxChCos.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#endif /*CHX_FAMILY*/

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_SEND_VLANID_CNS        0x2

/* traffic generation sending port */
#define ING_NETWORK_PORT_IDX_CNS    0

/* target port */
#define EGR_CASCADE_PORT_IDX_CNS    1

#define PRV_TGF_VLAN_ETHERTYPE_MODE     CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E
/******************************* Test packets *********************************/
static CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT    prvTgfRestoreLookupMode;
static CPSS_CSCD_LINK_TYPE_STC              origCascadeLink;
static CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT   origSrcPortTrunkHashEn;
static CPSS_CSCD_PORT_TYPE_ENT              prvTgfRestorePortType;
static GT_BOOL                              expCfiId;
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x26, 0xfb, 0x03, 0x67, 0x62, 0x1e},                /* daMac */
    {0xdc, 0x34, 0x60, 0x36, 0xd3, 0xe8}                 /* saMac */
};

/* DATA of bypass packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

/* First VLAN_TAG part */
TGF_PACKET_VLAN_TAG_STC prvTgfVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_SEND_VLANID_CNS                           /* pri, cfi, VlanId */
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet */
#define PRV_TGF_TAG_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* Bypass PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfTaggedPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* PACKET to send */
static TGF_PACKET_STC prvTgfTaggedPacketInfo = {
    PRV_TGF_TAG_PACKET_LEN_CNS,                                            /* totalLen */
    sizeof(prvTgfTaggedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfTaggedPacketPartArray                                        /* partsArray */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/******************************************************************************\
 *                   Private variables & definitions                          *
\******************************************************************************/

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfCscdDsaCfiIdTestSendAndCheck function
* @endinternal
*
* @brief   Function sends packet and check results
*
* @param[in] ingressPortIndex         - ingress port index
* @param[in] egressPortIndex          - egress port index
* @param[in] packetInfoPtr            - (pointer to) send packet
*                                      vfdArray          - VFD array with expected results
*                                       None
*/
static GT_VOID prvTgfCscdDsaCfiIdTestSendAndCheck
(
    IN GT_U32                   ingressPortIndex,
    IN GT_U32                   egressPortIndex,
    IN TGF_PACKET_STC           *packetInfoPtr
)
{
    GT_U32                      portIter;
    GT_STATUS                   rc;
    TGF_NET_DSA_STC             rxParam;
    GT_U8                       packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32                      buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U8                       devNum;
    GT_U8                       queue;
    GT_U32                      packetActualLength = 0;
    TGF_PACKET_DSA_TAG_STC      dsaInfo;

    /* AUTODOC: reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: send packet*/
    rc = prvTgfTransmitPacketsWithCapture(
            prvTgfDevNum,
            prvTgfPortsArray[ingressPortIndex],
            packetInfoPtr,
            1      /*burstCount*/,
            0      /*numVfd*/,
            NULL   /*vfdArray*/,
            prvTgfDevNum,
            prvTgfPortsArray[egressPortIndex],
            TGF_CAPTURE_MODE_MIRRORING_E,
            200                                /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfTransmitPacketsWithCapture");


    /* get first entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E,
            GT_TRUE, GT_TRUE, packetBuff,
            &buffLen, &packetActualLength,
            &devNum, &queue, &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
            TGF_PACKET_TYPE_CAPTURE_E, GT_TRUE);

    /* AUTODOC: parse the DSA (it should be FORWARTD eDSA) so we can get the CfiID*/
    rc = prvTgfTrafficGeneratorPacketDsaTagParse(
            &packetBuff[TGF_L2_HEADER_SIZE_CNS],/* start of DSA after the 12 bytes of the SA,DA mac addresses */
            &dsaInfo);

    UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfTrafficGeneratorPacketDsaTagParse Failed");
    UTF_VERIFY_EQUAL0_STRING_MAC(dsaInfo.dsaCommand,
                TGF_DSA_CMD_FORWARD_E, "prvTgfTrafficGeneratorPacketDsaTagParse Failed: Not forward DSA");

    /* AUTODOC: parse the DSA for CFI bit, word0[16]*/
    if ((GT_U8)(expCfiId) != dsaInfo.commonParams.cfiBit)
    {
        rc = GT_FAIL;
        UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCscdDsaCfiId Failed: CFI ID did not match");
    }
}

/**
* @internal prvTgfCscdDsaCfiIdTestConfigurationSet function
* @endinternal
*
* @brief   Test initial configurations
*/
GT_VOID prvTgfCscdDsaCfiIdTestConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc;
    GT_U32                                  portIter  = 0;
    CPSS_PORTS_BMP_STC                      portsMembers;
    CPSS_PORTS_BMP_STC                      portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC             vlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC    portsTaggingCmd;
    CPSS_MAC_ENTRY_EXT_STC                  macEntry;
    CPSS_CSCD_LINK_TYPE_STC                 cascadeLink;
    GT_BOOL                                 egressAttributesLocallyEn=GT_FALSE;

    /* start with no ports */
    /* all ports are 'untagged' */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* reset vlan entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));

    /* reset portsTaggingCmd */
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

    /* AUTODOC: SETUP CONFIGURATION: */
    /* initialize randomizer */
    cpssOsSrand(prvUtfSeedFromStreamNameGet());

    /* Get the original lookup mode */
    rc = cpssDxChCscdDevMapLookupModeGet(prvTgfDevNum,
            &prvTgfRestoreLookupMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCscdDevMapLookupModeGet");

    rc = cpssDxChCscdDevMapLookupModeSet(prvTgfDevNum,
            CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E /* 0 */);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCscdDevMapLookupModeSet");

    /* Get the original caccade mapping */
    rc = cpssDxChCscdDevMapTableGet(prvTgfDevNum,
            prvTgfDevNum,
            0,
            prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS],
            &origCascadeLink,
            &origSrcPortTrunkHashEn,
            &egressAttributesLocallyEn);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCscdDevMapTableGet");

    cpssOsMemSet(&cascadeLink, 0, sizeof(cascadeLink));
    cascadeLink.linkNum = prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS];
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    rc = cpssDxChCscdDevMapTableSet(prvTgfDevNum,
            prvTgfDevNum /* Target hw device number */,
            prvTgfDevNum, /* source hw devive number */
            prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS],
            &cascadeLink,
            CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E,
            GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCscdDevMapTableSet");

    /* Get the original port type */
    rc = cpssDxChCscdPortTypeGet(prvTgfDevNum,
            prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS],
            CPSS_PORT_DIRECTION_TX_E,
            &prvTgfRestorePortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCscdPortTypeGet");

    rc = cpssDxChCscdPortTypeSet(prvTgfDevNum,
            prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS],
            CPSS_PORT_DIRECTION_TX_E,
            CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCscdPortTypeSet");

    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers, prvTgfPortsArray[portIter]);
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsTagging, prvTgfPortsArray[portIter]);
        portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] = PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E;

        /* reset counters and set force link up */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* write Vlan entry */
    vlanInfo.autoLearnDisable     = GT_TRUE;
    vlanInfo.naMsgToCpuEn         = GT_TRUE;
    vlanInfo.floodVidx            = 0XFFF;
    vlanInfo.fidValue             = PRV_SEND_VLANID_CNS;
    rc = cpssDxChBrgVlanEntryWrite(prvTgfDevNum, PRV_SEND_VLANID_CNS,
            &portsMembers, &portsTagging,
            &vlanInfo, &portsTaggingCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanEntryWrite");

    /* Add Port members */
    for(portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, PRV_SEND_VLANID_CNS,
                prvTgfPortsArray[portIter], GT_TRUE /*isTagged*/ );
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d, %d",
                prvTgfDevNum, PRV_SEND_VLANID_CNS,
                prvTgfPortsArray[portIter], GT_TRUE);

    }

    /* Set the default VID for ingress port */
    rc = cpssDxChBrgVlanPortVidSet(prvTgfDevNum,
            prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS], CPSS_DIRECTION_INGRESS_E,
            PRV_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortVidSet");

    rc = cpssDxChCosPortDpToCfiDeiMapEnableSet(prvTgfDevNum,
            prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS],
            GT_TRUE /*DP-to-CFI/DEI mapping */);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortDpToCfiDeiMapEnableSet");

    /* AUTODOC: Create a macEntry with */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_SEND_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS];
    macEntry.isStatic                       = GT_TRUE;
    macEntry.daCommand                      = CPSS_MAC_TABLE_FRWRD_E;
    macEntry.saCommand                      = CPSS_MAC_TABLE_FRWRD_E;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 0X2, port 1 */
    rc = cpssDxChBrgFdbMacEntrySet(prvTgfDevNum, &macEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbMacEntrySet");
}

/**
* @internal prvTgfCscdDsaSkipCfiIdTestConfigurationRestore function
* @endinternal
*
* @brief   Test restore configurations
*/
GT_VOID prvTgfCscdDsaCfiIdTestConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                   rc;

    /* AUTODOC: RESTORE CONFIGURATION: */
    /* AUTODOC: Restore the egress port */
    rc = tgfTrafficGeneratorEgressCscdPortEnableSet(
            prvTgfDevNum,
            prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS],
            GT_FALSE,
            CPSS_CSCD_PORT_NETWORK_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorEgressCscdPortEnableSet");

    /* Restore Lookup Mode */
    rc = cpssDxChCscdDevMapLookupModeSet(prvTgfDevNum, prvTgfRestoreLookupMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCscdDevMapLookupModeSet");

    /* Restore the Dev mapping */
    rc = cpssDxChCscdDevMapTableSet(prvTgfDevNum,
            prvTgfDevNum,
            0,
            prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS],
            &origCascadeLink,
            origSrcPortTrunkHashEn,
            GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCscdDevMapTableSet");

    /* Restore the port type */
    rc = cpssDxChCscdPortTypeSet(prvTgfDevNum,
            prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS],
            CPSS_PORT_DIRECTION_BOTH_E,
            prvTgfRestorePortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCscdPortTypeSet");

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_SEND_VLANID_CNS);

    /* Restore the default VID for ingress port */
    rc = cpssDxChBrgVlanPortVidSet(prvTgfDevNum,
            prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS], CPSS_DIRECTION_INGRESS_E,
            1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortVidSet");

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", GT_TRUE);

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfCscdDsaCfiIdTest function
* @endinternal
*
* @brief   Test for CFI bit in DSA tag
*/
GT_VOID prvTgfCscdDsaCfiIdTest
(
    GT_VOID
)
{
    GT_STATUS                   rc;

    /* AUTODOC: Set common test configuration */
    prvTgfCscdDsaCfiIdTestConfigurationSet();

    /* AUTODOC: from Network port to Cascade port */
    PRV_UTF_LOG2_MAC(
            "\n*** from Network port %02d to Cascade port %02d ***\n",
            prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
            prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS]);

    /* Case 1:  Set DpToCfi = 1
     *          Packet CFI bit = 0(Incase of tagged)
     *          Send Tagged/Untagged Packet
     *          Verify both the case CFI bit should be set in forward DSA.(expCfiId == 1)
     * Case 2:  Set DpToCfi = 0
     *          Packet CFI bit = 0(Incase of tagged)
     *          Send Tagged/Untagged Packet
     *          Verify both the case CFI bit should be set in forward DSA.(expCfiId == 0)
     * Case 3:  Set DpToCfi = 1
     *          Packet CFI bit = 1
     *          Send Tagged Packet
     *          Verify the CFI bit should be set in forward DSA.(expCfiId == 1)
     * Case 3:  Set DpToCfi = 0
     *          Packet CFI bit = 1
     *          Send Tagged Packet
     *          Verify the CFI bit should be set in forward DSA.(expCfiId == 0)
     */
    /* Case 1 */
    expCfiId = 1;
    rc = cpssDxChCosDpToCfiDeiMapSet( prvTgfDevNum, 0/* CPSS_DP_GREEN_E */, expCfiId/* cfiDeiBit */);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosDpToCfiDeiMapSet");

    /* AUTODOC: Send a tagged packet(cfiId = 0) from network port to cascade port and verify DSA tag */
    prvTgfCscdDsaCfiIdTestSendAndCheck(ING_NETWORK_PORT_IDX_CNS,
                                                  EGR_CASCADE_PORT_IDX_CNS,
                                                  &prvTgfTaggedPacketInfo);

    /* AUTODOC: Send a untagged packet from network port to cascade port and verify DSA tag */
    prvTgfCscdDsaCfiIdTestSendAndCheck(ING_NETWORK_PORT_IDX_CNS,
                                                  EGR_CASCADE_PORT_IDX_CNS,
                                                  &prvTgfTaggedPacketInfo);
    /* Case 2 */
    expCfiId = 0;
    rc = cpssDxChCosDpToCfiDeiMapSet( prvTgfDevNum, 0/* CPSS_DP_GREEN_E */, expCfiId/* cfiDeiBit */);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosDpToCfiDeiMapSet");

    /* AUTODOC: Send a tagged packet(cfiId = 0) from network port to cascade port and verify DSA tag */
    prvTgfCscdDsaCfiIdTestSendAndCheck(ING_NETWORK_PORT_IDX_CNS,
                                                  EGR_CASCADE_PORT_IDX_CNS,
                                                  &prvTgfPacketInfo);

    /* AUTODOC: Send a untagged packet from network port to cascade port and verify DSA tag */
    prvTgfCscdDsaCfiIdTestSendAndCheck(ING_NETWORK_PORT_IDX_CNS,
                                                  EGR_CASCADE_PORT_IDX_CNS,
                                                  &prvTgfTaggedPacketInfo);

    /* Case 3 */
    /* AUTODOC: Set the ingress packet CFI bit to 1 */
    prvTgfVlanTag0Part.cfi = 1;
    expCfiId = 0;
    rc = cpssDxChCosDpToCfiDeiMapSet( prvTgfDevNum, 0/* CPSS_DP_GREEN_E */, expCfiId/* cfiDeiBit */);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosDpToCfiDeiMapSet");

    /* AUTODOC: Send a tagged packet(cfiId = 0) from network port to cascade port and verify DSA tag */
    prvTgfCscdDsaCfiIdTestSendAndCheck(ING_NETWORK_PORT_IDX_CNS,
                                                  EGR_CASCADE_PORT_IDX_CNS,
                                                  &prvTgfPacketInfo);

    /* Case 4 */
    expCfiId = 1;
    rc = cpssDxChCosDpToCfiDeiMapSet( prvTgfDevNum, 0/* CPSS_DP_GREEN_E */, expCfiId/* cfiDeiBit */);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosDpToCfiDeiMapSet");

    /* AUTODOC: Send a tagged packet(cfiId = 0) from network port to cascade port and verify DSA tag */
    prvTgfCscdDsaCfiIdTestSendAndCheck(ING_NETWORK_PORT_IDX_CNS,
                                                  EGR_CASCADE_PORT_IDX_CNS,
                                                  &prvTgfPacketInfo);

    /* Restore to default */
    expCfiId = 0;
    rc = cpssDxChCosDpToCfiDeiMapSet( prvTgfDevNum, 0/* CPSS_DP_GREEN_E */, expCfiId/* cfiDeiBit */);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosDpToCfiDeiMapSet");

    /* AUTODOC: Restore common test configuration */
    prvTgfCscdDsaCfiIdTestConfigurationRestore();
}

