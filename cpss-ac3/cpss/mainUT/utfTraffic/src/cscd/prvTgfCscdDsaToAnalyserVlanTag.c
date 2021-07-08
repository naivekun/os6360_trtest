/*******************************************************************************
*              (C), Copyright 2001, Marvell International Ltd.                 *
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
* @file prvTgfCscdDsaToAnalyserVlanTag.c
*
* @brief Test a eVLAN CFI, UP, TPID fields in the eDsa tag (To Analyser command, Rx mirroring).
*
* Case0: The packet is sent from a network port to a cascade port
* Ingress port mirrored and not cascaded.
* MAC_DA learned in FDB to egress port different from Rx Analyzer.
* Rx Analyzer port is cascaded and captured
* Ckeck eDSA tag of packet egressed from Rx Analyzer.
*
* Case1: The packet is sent from a cascade port to a cascade port
* No FDB enries and no mirrored ports.
* Rx Analyzer configured on remote device.
* Remote device mapped to cascaded egress port.
* Egress port is checked: the packet must be egressed unchanged.
*
* Case2: The packet is sent from a cascade port to a network port
* No FDB enries and no mirrored ports.
* Egress port configured as Rx Analyzer.
* Egress port is checked: the packet must be egressed wthout eDSA tag.
*
* @version   1
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfCscdGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfMirror.h>
#include <cscd/prvTgfCscdDsaToAnalyserVlanTag.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* ports */
#define PRV_TGF_RX_PORT_IDX_CNS       1
#define PRV_TGF_TX_PORT_IDX_CNS       2
#define PRV_TGF_FDB_PORT_IDX_CNS      3

static GT_U32 prvTgfCscdDsaToAnalyzerVlanTagFieldLevelCheck = 0;

/* for debug purposes - enabling field level checking */
void prvTgfCscdDsaToAnalyzerVlanTagFieldLevelCheckSet(GT_U32 value)
{
    prvTgfCscdDsaToAnalyzerVlanTagFieldLevelCheck = value;
}

/* Tested parameteres to be randomized assigned */
static  GT_U16 prvTgfRandomAssignedVlanId;
static  GT_U8  prvTgfRandomAssignedUp;
static  GT_U8  prvTgfRandomAssignedCfi;
static  GT_U16 prvTgfRandomAssignedVlanTpId;
static  GT_U8  prvTgfRandomAssignedVlanTpIndex;

/* saved PVID */
static TGF_VLAN_ID prvTgfDefVlanId = 1;

/******************************* Test packets *********************************/
/* L2 part of packet  */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                /* saMac */
};

/* VLAN_TAG0 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    0, /* etherType */ /* ranomized assigned in runtime */
    0, 0, 0    /* pri, cfi, vid */ /* ranomized assigned in runtime */
};

static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEthertypePart =
{
    0x3456
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9, 0xF8,
    0xF7, 0xF6, 0xF5, 0xF4, 0xF3, 0xF2, 0xF1, 0xF0,
    0xEF, 0xEE, 0xED, 0xEC, 0xEB, 0xEA, 0xE9, 0xE8,
    0xE7, 0xE6, 0xE5, 0xE4, 0xE3, 0xE2, 0xE1, 0xE0,
    0xDF, 0xDE, 0xDD, 0xDC, 0xDB, 0xDA, 0xD9, 0xD8,
    0xD7, 0xD6, 0xD5, 0xD4, 0xD3, 0xD2, 0xD1, 0xD0,
    0xCF, 0xCE, 0xCD, 0xCC, 0xCB, 0xCA, 0xC9, 0xC8,
    0xC7, 0xC6, 0xC5, 0xC4, 0xC3, 0xC2, 0xC1, 0xC0
};


/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* size of packet that came from network port */
#define PRV_TGF_NETWORK_PACKET_SIZE_CNS  \
    (TGF_L2_HEADER_SIZE_CNS              \
    + TGF_VLAN_TAG_SIZE_CNS              \
    + TGF_ETHERTYPE_SIZE_CNS             \
    + sizeof(prvTgfPayloadDataArr))

/* size of packet that came from cascade port */
#define PRV_TGF_CASCADE_PACKET_SIZE_CNS  \
    (TGF_L2_HEADER_SIZE_CNS              \
    + TGF_eDSA_TAG_SIZE_CNS              \
    + TGF_ETHERTYPE_SIZE_CNS             \
    + sizeof(prvTgfPayloadDataArr))

/* DSA tag without command-specific part(dsaInfo)              */
/* contains unions inside - can be initialized only at runtime */
static TGF_PACKET_DSA_TAG_STC prvTgfPacketDsaTagPart;
static TGF_PACKET_DSA_TAG_STC prvTgfPacketLocalSrcDsaTagPart;

/* parts of packet that came from network port */
static TGF_PACKET_PART_STC prvTgfNetworkPacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,         &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,   &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E,  &prvTgfPacketEthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,    &prvTgfPacketPayloadPart}
};

/* a network packet info */
static TGF_PACKET_STC prvTgfNetworkPacketInfo = {
    PRV_TGF_NETWORK_PACKET_SIZE_CNS,                                    /* totalLen */
    sizeof(prvTgfNetworkPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfNetworkPacketPartsArray                                       /* partsArray */
};

/* parts of packet that came from cascade port */
static TGF_PACKET_PART_STC prvTgfCascadePacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfPacketDsaTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E,  &prvTgfPacketEthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* a cascade packet info */
static TGF_PACKET_STC prvTgfCascadePacketInfo = {
    PRV_TGF_CASCADE_PACKET_SIZE_CNS,                                    /* totalLen */
    sizeof(prvTgfCascadePacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfCascadePacketPartsArray                                       /* partsArray */
};

/* parts of packet that came from cascade port */
static TGF_PACKET_PART_STC prvTgfCascadePacketLocalSrcPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfPacketLocalSrcDsaTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E,  &prvTgfPacketEthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* a cascade packet info */
static TGF_PACKET_STC prvTgfCascadePacketLocalSrcInfo = {
    PRV_TGF_CASCADE_PACKET_SIZE_CNS,                                    /* totalLen */
    sizeof(prvTgfCascadePacketLocalSrcPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfCascadePacketLocalSrcPartsArray                                       /* partsArray */
};

/* some original values saved to be restored at the end of the test. */
static CPSS_CSCD_PORT_TYPE_ENT           prvTgfSavedRxCscdPortType;
static CPSS_CSCD_PORT_TYPE_ENT           prvTgfSavedTxCscdPortType;
static PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   prvTgfSavedToAnalyzerForwardingMode;
static GT_U16 prvTgfSavedIngressVlanTpId;
static GT_U16 prvTgfSavedEgressVlanTpId;
static GT_U32 prvTgfSavedPortsArray[PRV_TGF_MAX_PORTS_NUM_CNS];


/******************************************************************************\
 *                            TEST IMPLEMENTATION                             *
\******************************************************************************/

/************************** PRIVATE DECLARATIONS ****************************/


/**
* @internal prvTgfCscdDsaToAnalyzerVlanTagInitRandomAssignedValues function
* @endinternal
*
* @brief   Common init values used in test
*/
static GT_VOID prvTgfCscdDsaToAnalyzerVlanTagInitRandomAssignedValues
(
    GT_VOID
)
{
    /* initialize randomizer */
    cpssOsSrand(prvUtfSeedFromStreamNameGet());

    /* VID in range 2..0xFFF */
    prvTgfRandomAssignedVlanId = (cpssOsRand() % (0x1000 - 2)) + 2;
    /* UP in range 0..7 */
    prvTgfRandomAssignedUp   = (cpssOsRand() % 8);
    /* CFI in range 0..1 */
    prvTgfRandomAssignedCfi   = (cpssOsRand() % 1);

    /* Vlan Tp Id in range 0x8000..0xFFFF */
    prvTgfRandomAssignedVlanTpId =
        (cpssOsRand() % (0x10000 - 0x8000)) + 0x8000;

    /* Vlan Tp Index in range 0..7 */
    prvTgfRandomAssignedVlanTpIndex = (cpssOsRand() % 8);

    PRV_UTF_LOG0_MAC("RandomAssignedValues used in test:\n");
    PRV_UTF_LOG1_MAC("prvTgfRandomAssignedVlanId: 0x%X\n", prvTgfRandomAssignedVlanId);
    PRV_UTF_LOG1_MAC("prvTgfRandomAssignedUp: %d\n", prvTgfRandomAssignedUp);
    PRV_UTF_LOG1_MAC("prvTgfRandomAssignedCfi: %d\n", prvTgfRandomAssignedCfi);
    PRV_UTF_LOG1_MAC("prvTgfRandomAssignedVlanTpId: 0x%X\n", prvTgfRandomAssignedVlanTpId);
    PRV_UTF_LOG1_MAC("prvTgfRandomAssignedVlanTpIndex: %d\n", prvTgfRandomAssignedVlanTpIndex);
}

/**
* @internal prvTgfCscdDsaToAnalyzerVlanTagInitAndSave function
* @endinternal
*
* @brief   Common init for tests and saving all needed for restore
*
* @param[in] rxCascaded               - rx port cascaded
* @param[in] txCascaded               - tx port cascaded
*                                       None
*/
static GT_VOID prvTgfCscdDsaToAnalyzerVlanTagInitAndSave
(
    IN GT_BOOL rxCascaded,
    IN GT_BOOL txCascaded
)
{
    GT_STATUS      rc;
    GT_HW_DEV_NUM  prvTgfSavedHwDevNum;

    prvTgfCscdDsaToAnalyzerVlanTagInitRandomAssignedValues();

    rc = prvUtfHwDeviceNumberGet(
        prvTgfDevsArray[PRV_TGF_RX_PORT_IDX_CNS], &prvTgfSavedHwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet");

    /* force link up for all ports */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* AUTODOC: create VLAN as TAG iwith TAG0 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        prvTgfRandomAssignedVlanId, PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* init packet generator DSA structure */
    cpssOsMemSet(&prvTgfPacketDsaTagPart, 0, sizeof(prvTgfPacketDsaTagPart));

    prvTgfPacketDsaTagPart.dsaCommand          = TGF_DSA_CMD_TO_ANALYZER_E;
    prvTgfPacketDsaTagPart.dsaType             = TGF_DSA_4_WORD_TYPE_E;
    prvTgfPacketDsaTagPart.commonParams.vid    = prvTgfRandomAssignedVlanId;
    prvTgfPacketDsaTagPart.commonParams.vpt    = prvTgfRandomAssignedUp;
    prvTgfPacketDsaTagPart.commonParams.cfiBit = prvTgfRandomAssignedCfi;
    prvTgfPacketDsaTagPart.dsaInfo.toAnalyzer.tag0TpidIndex =
        prvTgfRandomAssignedVlanTpIndex;
    prvTgfPacketDsaTagPart.dsaInfo.toAnalyzer.isTagged        = GT_TRUE;
    prvTgfPacketDsaTagPart.dsaInfo.toAnalyzer.rxSniffer       = GT_TRUE;
    prvTgfPacketDsaTagPart.dsaInfo.toAnalyzer.analyzerTrgType = CPSS_INTERFACE_PORT_E;
    prvTgfPacketDsaTagPart.dsaInfo.toAnalyzer.extDestInfo.devPort.analyzerHwTrgDev
        = prvTgfDevsArray[PRV_TGF_TX_PORT_IDX_CNS];
    prvTgfPacketDsaTagPart.dsaInfo.toAnalyzer.extDestInfo.devPort.analyzerTrgPort
        = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];
    prvTgfPacketDsaTagPart.dsaInfo.toAnalyzer.extDestInfo.devPort.analyzerIsTrgPortValid
        = GT_TRUE;
    prvTgfPacketDsaTagPart.dsaInfo.toAnalyzer.extDestInfo.devPort.analyzerTrgEport
        = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];

    /* packet source device should be different from Rx device */
    prvTgfPacketDsaTagPart.dsaInfo.toAnalyzer.devPort.hwDevNum =
        prvTgfSavedHwDevNum + 1;

    cpssOsMemCpy(
        &prvTgfPacketLocalSrcDsaTagPart, &prvTgfPacketDsaTagPart,
        sizeof(prvTgfPacketDsaTagPart));

    prvTgfPacketLocalSrcDsaTagPart.dsaInfo.toAnalyzer.devPort.hwDevNum =
        prvTgfDevsArray[PRV_TGF_RX_PORT_IDX_CNS];
    prvTgfPacketLocalSrcDsaTagPart.dsaInfo.toAnalyzer.devPort.portNum =
        prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
    prvTgfPacketLocalSrcDsaTagPart.dsaInfo.toAnalyzer.devPort.ePort =
        prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];

    /* VLAN Tag */
    prvTgfPacketVlanTagPart.etherType = prvTgfRandomAssignedVlanTpId;
    prvTgfPacketVlanTagPart.vid       = prvTgfRandomAssignedVlanId;
    prvTgfPacketVlanTagPart.pri       = prvTgfRandomAssignedUp;
    prvTgfPacketVlanTagPart.cfi       = prvTgfRandomAssignedCfi;

        /* AUTODOC: save a cascade configuration of the rx-port */
    rc = prvTgfCscdPortTypeGet(
        prvTgfDevsArray[PRV_TGF_RX_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
        CPSS_PORT_DIRECTION_RX_E,
        &prvTgfSavedRxCscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet");

    /* AUTODOC: save a cascade configuration of the tx-port */
    rc = prvTgfCscdPortTypeGet(
        prvTgfDevsArray[PRV_TGF_TX_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
        CPSS_PORT_DIRECTION_TX_E,
        &prvTgfSavedTxCscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet");

    rc = prvTgfMirrorToAnalyzerForwardingModeGet(
        prvTgfDevsArray[PRV_TGF_RX_PORT_IDX_CNS],
        &prvTgfSavedToAnalyzerForwardingMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeGet");

    rc = prvTgfBrgVlanTpidEntryGet(
        CPSS_DIRECTION_INGRESS_E,
        prvTgfRandomAssignedVlanTpIndex,
        &prvTgfSavedIngressVlanTpId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntryGet");

    rc = prvTgfBrgVlanTpidEntryGet(
        CPSS_DIRECTION_EGRESS_E,
        prvTgfRandomAssignedVlanTpIndex,
        &prvTgfSavedEgressVlanTpId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntryGet");

    if (rxCascaded != GT_FALSE)
    {
        /* AUTODOC: set a cascade configuration of the rx-port */
        rc = prvTgfCscdPortTypeSet(
            prvTgfDevsArray[PRV_TGF_RX_PORT_IDX_CNS],
            CPSS_PORT_DIRECTION_RX_E,
            prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
            CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");
    }

    if (txCascaded != GT_FALSE)
    {
        /* AUTODOC: set a cascade configuration of the tx-port */
        rc = prvTgfCscdPortTypeSet(
            prvTgfDevsArray[PRV_TGF_TX_PORT_IDX_CNS],
            CPSS_PORT_DIRECTION_TX_E,
            prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
            CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");
    }

    rc = prvTgfMirrorToAnalyzerForwardingModeSet(
        PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeSet");

    rc = prvTgfBrgVlanTpidEntrySet(
        CPSS_DIRECTION_INGRESS_E,
        prvTgfRandomAssignedVlanTpIndex,
        prvTgfRandomAssignedVlanTpId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet");

    rc = prvTgfBrgVlanTpidEntrySet(
        CPSS_DIRECTION_EGRESS_E,
        prvTgfRandomAssignedVlanTpIndex,
        prvTgfRandomAssignedVlanTpId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet");

    rc = prvTgfBrgVlanEgressTagTpidSelectModeSet(
        prvTgfDevsArray[PRV_TGF_TX_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
        CPSS_VLAN_ETHERTYPE0_E,
        PRV_TGF_BRG_VLAN_VID_TO_TPID_SELECT_TABLE_MODE_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEgressTagTpidSelectModeSet");

}

/**
* @internal prvTgfCscdDsaToAnalyzerVlanTagCfgReset function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_STATUS prvTgfCscdDsaToAnalyzerVlanTagCfgReset
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC analyzerIf;

    /* AUTODOC: reset counters, force Link UP */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfEthCountersReset");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: invalidate VLAN Table Entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(prvTgfRandomAssignedVlanId);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* AUTODOC: restore default vlanId to ingress port */
    prvTgfBrgVlanPortVidSet(
        prvTgfDevsArray[PRV_TGF_RX_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS], prvTgfDefVlanId);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgVlanPortVidSet");

    /* AUTODOC: restore a cascade configuration of the rx-port */
    rc = prvTgfCscdPortTypeSet(
        prvTgfDevsArray[PRV_TGF_RX_PORT_IDX_CNS],
        CPSS_PORT_DIRECTION_RX_E,
        prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
        prvTgfSavedRxCscdPortType);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfCscdPortTypeSet");

    /* AUTODOC: restore a cascade configuration of the tx-port */
    rc = prvTgfCscdPortTypeSet(
        prvTgfDevsArray[PRV_TGF_TX_PORT_IDX_CNS],
        CPSS_PORT_DIRECTION_TX_E,
        prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
        prvTgfSavedTxCscdPortType);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfCscdPortTypeSet");

    /* AUTODOC: restore a analyzer-0 configuration */
    cpssOsMemSet(&analyzerIf, 0, sizeof(analyzerIf));
    analyzerIf.interface.type = CPSS_INTERFACE_PORT_E;
    rc = prvTgfMirrorAnalyzerInterfaceSet(
        0/*index*/, &analyzerIf);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfMirrorAnalyzerInterfaceSet");

    /* AUTODOC: restore Rx port mirror configuration */
    rc = prvTgfMirrorRxPortSet(
        prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
        GT_FALSE/*enable*/,
        0/*index*/);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfMirrorRxPortSet");

    rc = prvTgfMirrorToAnalyzerForwardingModeSet(
        prvTgfSavedToAnalyzerForwardingMode);
    PRV_UTF_VERIFY_RC1(
        rc, "prvTgfMirrorToAnalyzerForwardingModeSet");

    rc = prvTgfBrgVlanTpidEntrySet(
        CPSS_DIRECTION_INGRESS_E,
        prvTgfRandomAssignedVlanTpIndex,
        prvTgfSavedIngressVlanTpId);
    PRV_UTF_VERIFY_RC1(
        rc, "prvTgfBrgVlanTpidEntrySet");

    rc = prvTgfBrgVlanTpidEntrySet(
        CPSS_DIRECTION_EGRESS_E,
        prvTgfRandomAssignedVlanTpIndex,
        prvTgfSavedEgressVlanTpId);
    PRV_UTF_VERIFY_RC1(
        rc, "prvTgfBrgVlanTpidEntrySet");

    rc = prvTgfBrgVlanEgressTagTpidSelectModeSet(
        prvTgfDevsArray[PRV_TGF_TX_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
        CPSS_VLAN_ETHERTYPE0_E,
        PRV_TGF_BRG_VLAN_EGRESS_TAG_TPID_SELECT_MODE_E);
    PRV_UTF_VERIFY_RC1(
        rc, "prvTgfBrgVlanEgressTagTpidSelectModeSet");

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, prvTgfSavedPortsArray, sizeof(prvTgfSavedPortsArray));

    /* AUTODOC: restore content of TPID profile 7 */
    rc = prvTgfCaptureEnableUnmatchedTpidProfile(prvTgfDevNum, GT_FALSE, 7);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfCaptureEnableUnmatchedTpidProfile")

    return rc1;
}

/**
* @internal prvTgfCscdDsaToAnalyzerVlanTagFdbSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfCscdDsaToAnalyzerVlanTagFdbSet
(
    IN GT_U8   *macDaPtr,
    IN GT_U8   devNum,
    IN GT_U32   port,
    IN GT_U16  vlan
)
{
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;
    GT_STATUS                   rc;

    /* AUTODOC: add FDB entry with MAC  */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, macDaPtr, 6);

    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = vlan;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum    = devNum;
    macEntry.dstInterface.devPort.portNum   = port;
    macEntry.isStatic                       = GT_TRUE;
    macEntry.daCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.saCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;

    /* AUTODOC: add FDB entry with MAC */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite");
}

/**
* @internal prvTgfCscdDsaToAnalyzerVlanTagCfg function
* @endinternal
*
* @brief   Save configuration to be restored at the end of test
*
* @param[in] rxCascaded               - rx port cascaded
* @param[in] txCascaded               - tx port cascaded
* @param[in] toSetFdb                 - make FDB entry for packets MAC_DA
* @param[in] toSetAnalyzer            - set TX port to be Rx Analyzer
*                                       None
*/
static GT_VOID prvTgfCscdDsaToAnalyzerVlanTagCfg
(
    IN GT_BOOL rxCascaded,
    IN GT_BOOL txCascaded,
    IN GT_BOOL toSetFdb,
    IN GT_BOOL toSetAnalyzer
)
{
    GT_STATUS rc;
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC analyzerIf;

    /* AUTODOC: save prvTgfPortsArray */
    cpssOsMemCpy(prvTgfSavedPortsArray, prvTgfPortsArray,
                 sizeof(prvTgfSavedPortsArray));

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   PRV_TGF_RX_PORT_IDX_CNS,
                                   PRV_TGF_TX_PORT_IDX_CNS,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }

    /* AUTODOC: configure TPID profile 7 to not match any etherType and use it for capturing */
    rc = prvTgfCaptureEnableUnmatchedTpidProfile(prvTgfDevNum, GT_TRUE, 7);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCaptureEnableUnmatchedTpidProfile");

    prvTgfCscdDsaToAnalyzerVlanTagInitAndSave(
       rxCascaded, txCascaded);

    if (toSetFdb != GT_FALSE)
    {
         prvTgfCscdDsaToAnalyzerVlanTagFdbSet(
            prvTgfPacketL2Part.daMac,
            prvTgfDevsArray[PRV_TGF_FDB_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS],
            prvTgfRandomAssignedVlanId);
    }

    if (toSetAnalyzer != GT_FALSE)
    {
         /* AUTODOC: set a analyzer-0 configuration */
         cpssOsMemSet(&analyzerIf, 0, sizeof(analyzerIf));
         analyzerIf.interface.type = CPSS_INTERFACE_PORT_E;
         analyzerIf.interface.devPort.hwDevNum =
             prvTgfDevsArray[PRV_TGF_TX_PORT_IDX_CNS];
         analyzerIf.interface.devPort.portNum =
             prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];
         rc = prvTgfMirrorAnalyzerInterfaceSet(
             0/*index*/, &analyzerIf);
         UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet");
    }


    if (toSetAnalyzer != GT_FALSE)
    {
        /* AUTODOC: set Rx port mirror configuration */
        rc = prvTgfMirrorRxPortSet(
            prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
            GT_TRUE/*enable*/,
            0/*index*/);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet");
    }
}

/**
* @internal prvTgfCscdDsaToAnalyzerVlanTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] prvTgfPacketInfoPtr      - (pointer to) packet info
*                                       None
*/
static GT_VOID prvTgfCscdDsaToAnalyzerVlanTagTrafficGenerate
(
    IN TGF_PACKET_STC *prvTgfPacketInfoPtr
)
{
    GT_STATUS            rc         = GT_OK;

    /* AUTODOC: Transmit Packets With Capture*/

    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevsArray[PRV_TGF_RX_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
        prvTgfPacketInfoPtr,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevsArray[PRV_TGF_TX_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
        TGF_CAPTURE_MODE_PCL_E,
        200                                /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPacketsWithCapture");
}

/**
* @internal prvTgfCscdDsaToAnalyzerVlanTagTrafficDsaCheck function
* @endinternal
*
* @brief   Checks egress traffic eDSA Tag - VLAN Tag related fields
*
* @param[in] tpIdIndex                - tp Id Index
* @param[in] up                       - up
* @param[in] cfi                      - cfi
* @param[in] vid                      - vid
*                                       None
*/
static GT_VOID prvTgfCscdDsaToAnalyzerVlanTagTrafficDsaCheck
(
    IN GT_U32  tpIdIndex,
    IN GT_U32  up,
    IN GT_U32  cfi,
    IN GT_U32  vid
)
{
    GT_STATUS                       rc         = GT_OK;
    CPSS_INTERFACE_INFO_STC         portInterface;
    TGF_VFD_INFO_STC                vfdArray[6];
    GT_U32                          numTriggersBmp;

    /* AUTODOC: Check VID from Tag of captured packet */

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    cpssOsMemSet(vfdArray, 0, sizeof(vfdArray));

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevsArray[PRV_TGF_TX_PORT_IDX_CNS];
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];

    /* STATIC_NO_MATCH_IGNORED mode dedicated to use OR-MASK */

    /* Packet command - eDSA word0[31:30] */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_NO_MATCH_IGNORED_E;
    vfdArray[0].offset = 6 + 6;
    vfdArray[0].cycleCount = 1;
    vfdArray[0].patternPtr[0] = (GT_U8)(2 << 6); /* 2 == TO_ANALYZER */
    vfdArray[0].bitMaskPtr[0] = (GT_U8)(0xFF & (~ 0xC0));

    /* tpIdIndex - eDSA word2[2:0] */
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_NO_MATCH_IGNORED_E;
    vfdArray[1].offset = 6 + 6 + (4 * 2) + 3;
    vfdArray[1].cycleCount = 1;
    vfdArray[1].patternPtr[0] = (GT_U8)(tpIdIndex & 0x07);
    vfdArray[1].bitMaskPtr[0] = (GT_U8)(0xFF & (~ 0x07));

    /*up - eDSA word0[15:13] */
    vfdArray[2].mode = TGF_VFD_MODE_STATIC_NO_MATCH_IGNORED_E;
    vfdArray[2].offset = 6 + 6 + 2;
    vfdArray[2].cycleCount = 1;
    vfdArray[2].patternPtr[0] = (GT_U8)((up << 5) & 0xE0);
    vfdArray[2].bitMaskPtr[0] = (GT_U8)(0xFF & (~ 0xE0));

    /*cfi - eDSA word0[16] */
    vfdArray[3].mode = TGF_VFD_MODE_STATIC_NO_MATCH_IGNORED_E;
    vfdArray[3].offset = 6 + 6 + 1;
    vfdArray[3].cycleCount = 1;
    vfdArray[3].patternPtr[0] = (GT_U8)(cfi & 0x01);
    vfdArray[3].bitMaskPtr[0] = (GT_U8)(0xFF & (~ 0x01));

    /*vid[11:0] - eDSA word0[11:0] */
    vfdArray[4].mode = TGF_VFD_MODE_STATIC_NO_MATCH_IGNORED_E;
    vfdArray[4].offset = 6 + 6 + 2;
    vfdArray[4].cycleCount = 2;
    vfdArray[4].patternPtr[0] = (GT_U8)((vid >> 8) & 0x0F);
    vfdArray[4].bitMaskPtr[0] = (GT_U8)(0xFF & (~ 0x0F));
    vfdArray[4].patternPtr[1] = (GT_U8)(vid & 0xFF);
    vfdArray[4].bitMaskPtr[1] = 0;

    /*vid[15:12] - eDSA word3[30:27] */
    vfdArray[5].mode = TGF_VFD_MODE_STATIC_NO_MATCH_IGNORED_E;
    vfdArray[5].offset = 6 + 6 + (4 * 3);
    vfdArray[5].cycleCount = 1;
    vfdArray[5].patternPtr[0] = (GT_U8)((vid >> 9) & 0x78);
    vfdArray[5].bitMaskPtr[0] = (GT_U8)(0xFF & (~ 0x78));

    rc =  tgfTrafficGeneratorPortTxEthTriggerCountersGet(
            &portInterface,
            6 /*numVfd*/,
            vfdArray,
            &numTriggersBmp);
    if (rc != GT_NO_MORE)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d",
            prvTgfDevNum);
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(
        0x3F, numTriggersBmp, "received patterns bitmap");
}

/**
* @internal prvTgfCscdDsaToAnalyzerVlanTagTrafficVTagCheck function
* @endinternal
*
* @brief   Checks egress VLAN Tag fields
*
* @param[in] tpId                     - tp Id
* @param[in] up                       - up
* @param[in] cfi                      - cfi
* @param[in] vid                      - vid
*                                       None
*/
static GT_VOID prvTgfCscdDsaToAnalyzerVlanTagTrafficVTagCheck
(
    IN GT_U32  tpId,
    IN GT_U32  up,
    IN GT_U32  cfi,
    IN GT_U32  vid
)
{
    GT_STATUS                       rc         = GT_OK;
    CPSS_INTERFACE_INFO_STC         portInterface;
    TGF_VFD_INFO_STC                vfdArray[4];
    GT_U32                          numTriggersBmp;

    /* AUTODOC: Check VID from Tag of captured packet */

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    cpssOsMemSet(vfdArray, 0, sizeof(vfdArray));

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevsArray[PRV_TGF_TX_PORT_IDX_CNS];
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];

    /* STATIC_NO_MATCH_IGNORED mode dedicated to use OR-MASK */

    /* tpId */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_NO_MATCH_IGNORED_E;
    vfdArray[0].offset = 6 + 6;
    vfdArray[0].cycleCount = 2;
    vfdArray[0].patternPtr[0] = (GT_U8)((tpId >> 8) & 0xFF);
    vfdArray[0].bitMaskPtr[0] = 0;
    vfdArray[0].patternPtr[1] = (GT_U8)(tpId & 0xFF);
    vfdArray[0].bitMaskPtr[1] = 0;

    /*up*/
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_NO_MATCH_IGNORED_E;
    vfdArray[1].offset = 6 + 6 + 2;
    vfdArray[1].cycleCount = 1;
    vfdArray[1].patternPtr[0] = (GT_U8)((up << 5) & 0xE0);
    vfdArray[1].bitMaskPtr[0] = (GT_U8)(0xFF & (~ 0xE0));

    /*cfi*/
    vfdArray[2].mode = TGF_VFD_MODE_STATIC_NO_MATCH_IGNORED_E;
    vfdArray[2].offset = 6 + 6 + 2;
    vfdArray[2].cycleCount = 1;
    vfdArray[2].patternPtr[0] = (GT_U8)((cfi << 4) & 0x10);
    vfdArray[2].bitMaskPtr[0] = (GT_U8)(0xFF & (~ 0x10));

    /*vid*/
    vfdArray[3].mode = TGF_VFD_MODE_STATIC_NO_MATCH_IGNORED_E;
    vfdArray[3].offset = 6 + 6 + 2;
    vfdArray[3].cycleCount = 2;
    vfdArray[3].patternPtr[0] = (GT_U8)((vid >> 8) & 0x0F);
    vfdArray[3].bitMaskPtr[0] = (GT_U8)(0xFF & (~ 0x0F));
    vfdArray[3].patternPtr[1] = (GT_U8)(vid & 0xFF);
    vfdArray[3].bitMaskPtr[1] = 0;

    rc =  tgfTrafficGeneratorPortTxEthTriggerCountersGet(
            &portInterface,
            4 /*numVfd*/,
            vfdArray,
            &numTriggersBmp);
    if (rc != GT_NO_MORE)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d",
            prvTgfDevNum);
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(
        0x0F, numTriggersBmp, "received patterns bitmap");
}

/**
* @internal prvTgfCscdDsaToAnalyzerVlanTagTrafficPacketLevelCheck function
* @endinternal
*
* @brief   Checks egress VLAN Tag fields
*
* @param[in] packetInfoPtr            - the packet format to compare the captured packet with.
*                                       None
*/
static GT_VOID prvTgfCscdDsaToAnalyzerVlanTagTrafficPacketLevelCheck
(
    IN  TGF_PACKET_STC      *packetInfoPtr
)
{
    CPSS_INTERFACE_INFO_STC     portInterface;
    GT_STATUS                   rc;
    GT_U32                      actualCapturedNumOfPackets;
    GT_U32                      onFirstPacketNumTriggersBmp;

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevsArray[PRV_TGF_TX_PORT_IDX_CNS];
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];

    /* if capture packet different from pattern GT_BAD_VALUE returned */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
        &portInterface,
        packetInfoPtr,
        1 /*numOfPackets*/,
        0 /*numVfd*/,
        NULL /*vfdArray*/,
        NULL /*byteNumMaskList*/,
        0 /*byteNumMaskListLen*/,
        &actualCapturedNumOfPackets,
        &onFirstPacketNumTriggersBmp);
    if (rc != GT_NO_MORE)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureCompare: %d");
    }
}

/**
* @internal prvTgfCscdDsaToAnalyzerVlanTagNetworkToCascaded function
* @endinternal
*
* @brief   Test sending packet to network port and checking the Cascaded RX Analyser
*         output eDSA tag.
*/
GT_VOID prvTgfCscdDsaToAnalyzerVlanTagNetworkToCascaded
(
    GT_VOID
)
{
    prvTgfCscdDsaToAnalyzerVlanTagCfg(
        GT_FALSE/*rxCascaded*/,
        GT_TRUE/*txCascaded*/,
        GT_TRUE/*toSetFdb*/,
        GT_TRUE/*toSetAnalyzer*/);

    prvTgfCscdDsaToAnalyzerVlanTagTrafficGenerate(
        &prvTgfNetworkPacketInfo);

    prvTgfPrintPortCountersEth();

    if (prvTgfCscdDsaToAnalyzerVlanTagFieldLevelCheck)
    {
        prvTgfCscdDsaToAnalyzerVlanTagTrafficDsaCheck(
            prvTgfRandomAssignedVlanTpIndex,
            prvTgfRandomAssignedUp,
            prvTgfRandomAssignedCfi,
            prvTgfRandomAssignedVlanId);
    }
    else
    {
        prvTgfCscdDsaToAnalyzerVlanTagTrafficPacketLevelCheck(
            &prvTgfCascadePacketLocalSrcInfo);
    }

    prvTgfCscdDsaToAnalyzerVlanTagCfgReset();
}

/**
* @internal prvTgfCscdDsaToAnalyzerVlanTagCascadedToNetwork function
* @endinternal
*
* @brief   Test sending packet to Cascaded port and checking the network RX Analyser
*         output VLAN tag.
*/
GT_VOID prvTgfCscdDsaToAnalyzerVlanTagCascadedToNetwork
(
    GT_VOID
)
{
    prvTgfCscdDsaToAnalyzerVlanTagCfg(
        GT_TRUE/*rxCascaded*/,
        GT_FALSE/*txCascaded*/,
        GT_FALSE/*toSetFdb*/,
        GT_TRUE/*toSetAnalyzer*/);

    prvTgfCscdDsaToAnalyzerVlanTagTrafficGenerate(
        &prvTgfCascadePacketInfo);

    prvTgfPrintPortCountersEth();

    if (prvTgfCscdDsaToAnalyzerVlanTagFieldLevelCheck)
    {
        prvTgfCscdDsaToAnalyzerVlanTagTrafficVTagCheck(
            prvTgfRandomAssignedVlanTpId,
            prvTgfRandomAssignedUp,
            prvTgfRandomAssignedCfi,
            prvTgfRandomAssignedVlanId);
    }
    else
    {
        prvTgfCscdDsaToAnalyzerVlanTagTrafficPacketLevelCheck(
            &prvTgfNetworkPacketInfo);
    }

    prvTgfCscdDsaToAnalyzerVlanTagCfgReset();
}

/**
* @internal prvTgfCscdDsaToAnalyzerVlanTagCascadedToCascaded function
* @endinternal
*
* @brief   Test sending packet to Cascaded port and checking the Cascaded RX Analyser
*         output eDSA tag.
*/
GT_VOID prvTgfCscdDsaToAnalyzerVlanTagCascadedToCascaded
(
    GT_VOID
)
{
    prvTgfCscdDsaToAnalyzerVlanTagCfg(
        GT_TRUE/*rxCascaded*/,
        GT_TRUE/*txCascaded*/,
        GT_FALSE/*toSetFdb*/,
        GT_TRUE/*toSetAnalyzer*/);

    prvTgfCscdDsaToAnalyzerVlanTagTrafficGenerate(
        &prvTgfCascadePacketInfo);

    prvTgfPrintPortCountersEth();

    if (prvTgfCscdDsaToAnalyzerVlanTagFieldLevelCheck)
    {
        prvTgfCscdDsaToAnalyzerVlanTagTrafficDsaCheck(
            prvTgfRandomAssignedVlanTpIndex,
            prvTgfRandomAssignedUp,
            prvTgfRandomAssignedCfi,
            prvTgfRandomAssignedVlanId);
    }
    else
    {
        prvTgfCscdDsaToAnalyzerVlanTagTrafficPacketLevelCheck(
            &prvTgfCascadePacketInfo);
    }

    prvTgfCscdDsaToAnalyzerVlanTagCfgReset();
}

