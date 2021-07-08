/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfCscdFdbSaLookup.c
*
* DESCRIPTION:
*       FDB SA learning controlling test in forward eDsa tag.
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
#include <cscd/prvTgfCscdFdbSaLookup.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_VLANID_1_CNS        1

/* traffic generation sending port */
#define ING_CASCADE_PORT_IDX_CNS    1

/* target port */
#define EGR_NETWORK_PORT_IDX_CNS    2

/* final target device & port */
#define PRV_TGF_DSA_SRC_DEV_CNS     8
#define PRV_TGF_DSA_PORT_CNS        58

/******************************* Test packets *********************************/

static TGF_MAC_ADDR prvTgfSaMacArr[] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x07},
                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x08}};

static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x17},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x08}                 /* saMac */
};

/* VLAN_TAG0 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_1_CNS                          /* pri, cfi, VlanId */
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

/********************** DSA tag  ***************************/
static TGF_DSA_DSA_FORWARD_STC  prvTgfPacketDsaTagPart_forward = {
    PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E , /*srcIsTagged*/
    16,/*srcHwDev*/
    GT_FALSE,/* srcIsTrunk */
    /*union*/
    {
        /*trunkId*/
        1/*portNum*/
    },/*source;*/

    0,/*srcId*/

    GT_FALSE,/*egrFilterRegistered*/
    GT_FALSE,/*wasRouted*/
    0,/*qosProfileIndex*/

    /*CPSS_INTERFACE_INFO_STC         dstInterface*/
    {
        CPSS_INTERFACE_PORT_E,/*type*/

        /*struct*/{
            0,/*devNum*/
            2/*portNum*/
        },/*devPort*/

        0,/*trunkId*/
        0, /*vidx*/
        1,/*vlanId*/
        0,/*devNum*/
        0,/*fabricVidx*/
        0 /*index*/
    },/*dstInterface*/
    GT_FALSE,/*isTrgPortValid*/
    0,/*dstEport*/
    0,/*TPID Index*/
    GT_FALSE,/*srcIsPe*/
    /*union*/
    {
        /*trunkId*/
        1/*portNum*/
    },/*origSrcPhy;*/
    GT_TRUE,/*phySrcMcFilterEnable*/
    0, /* hash */
    GT_TRUE /*skipFdbSaLookup*/
};

static TGF_PACKET_DSA_TAG_STC  prvTgfPacketDsaTagPart = {
    TGF_DSA_CMD_FORWARD_E ,/*dsaCommand*/
    TGF_DSA_4_WORD_TYPE_E ,/*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,/*vpt*/
        0,/*cfiBit*/
        PRV_TGF_VLANID_1_CNS,/*vid*/
        GT_FALSE,/*dropOnSource*/
        GT_FALSE/*packetIsLooped*/
    }/*commonParams*/

/*    union{
        TGF_DSA_DSA_TO_CPU_STC         toCpu;
        TGF_DSA_DSA_FROM_CPU_STC       fromCpu;
        TGF_DSA_DSA_TO_ANALYZER_STC    toAnalyzer;
        TGF_DSA_DSA_FORWARD_STC        forward;
    }dsaInfo;*/
};

/* Bypass PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* size of packet that came from cascade port */
#define PRV_TGF_CASCADE_PACKET_SIZE_CNS (  TGF_L2_HEADER_SIZE_CNS        \
                                           + TGF_eDSA_TAG_SIZE_CNS       \
                                           + TGF_VLAN_TAG_SIZE_CNS       \
                                           + sizeof(prvTgfPayloadDataArr))

/* parts of packet that came from cascade port */
static TGF_PACKET_PART_STC prvTgfCascadePacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfPacketDsaTagPart},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* a cascade packet info */
static TGF_PACKET_STC prvTgfCascadePacketInfo = {
    PRV_TGF_CASCADE_PACKET_SIZE_CNS,                                    /* totalLen */
    sizeof(prvTgfCascadePacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfCascadePacketPartsArray                                       /* partsArray */
};

/******************************************************************************\
 *                   Private variables & definitions                          *
\******************************************************************************/

/* Parameters needed to be restored */
static GT_U32                            portsArrayForRestore[PRV_TGF_MAX_PORTS_NUM_CNS];

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfCscdDsaSkipFdbSaLearningTestSendAndCheck function
* @endinternal
*
* @brief   Function sends packet and check results
*
* @param[in] ingressPortIndex         - ingress port index
* @param[in] packetInfoPtr            - (pointer to) send packet
*                                      vfdArray          - VFD array with expected results
*                                       None
*/
static GT_VOID prvTgfCscdDsaSkipFdbSaLearningTestSendAndCheck
(
    IN GT_U32                   ingressPortIndex,
    IN TGF_PACKET_STC           *packetInfoPtr
)
{
    GT_U32                          portIter = 0;
    GT_STATUS                       rc = GT_OK;
    PRV_TGF_MAC_ENTRY_KEY_STC       macEntryKey;
    PRV_TGF_BRG_MAC_ENTRY_STC       macEntry;

    /* bind the DSA tag FORWARD part */
    prvTgfPacketDsaTagPart.dsaInfo.forward = prvTgfPacketDsaTagPart_forward;

    /* AUTODOC: reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                "prvTgfResetCountersEth: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: Configure Port mode not to Skip the SA learnng*/
    rc = cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet(prvTgfDevNum,
                                    prvTgfPortsArray[ingressPortIndex],
                                    GT_FALSE /*acceptSkipSaLookup*/ );
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet failed!\n");

    /* AUTODOC: Send packet with MAC - 00:00:00:00:00:07 */
    cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfSaMacArr[0],
                                 sizeof(prvTgfPacketL2Part.saMac));
    rc = prvTgfTransmitPackets(
            prvTgfDevNum,
            prvTgfPortsArray[ingressPortIndex],
            packetInfoPtr,
            1      /*burstCount*/,
            0      /*numVfd*/,
            NULL   /*vfdArray*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPackets");

    /* AUTODOC: Configure Port mode to Skip the SA learnng*/
    rc = cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet(prvTgfDevNum,
                                    prvTgfPortsArray[ingressPortIndex],
                                    GT_TRUE /*acceptSkipSaLookup*/ );
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet failed!\n");

    /* AUTODOC: Send packet with MAC - 00:00:00:00:00:07*/
    cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfSaMacArr[1],
                                 sizeof(prvTgfPacketL2Part.saMac));
    rc = prvTgfTransmitPackets(
            prvTgfDevNum,
            prvTgfPortsArray[ingressPortIndex],
            packetInfoPtr,
            1      /*burstCount*/,
            0      /*numVfd*/,
            NULL   /*vfdArray*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPackets");

    /* AUTODOC: Check FDB entries phase - read FDB entries and verify MACs*/
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = PRV_TGF_VLANID_1_CNS;
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

    /* AUTODOC: Verify 00:00:00:00:00:07 should be present in FDB table */
    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, prvTgfSaMacArr[0], sizeof(TGF_MAC_ADDR));
    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);

    /* AUTODOC: Verify 00:00:00:00:00:07 should not be present in FDB table */
    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, prvTgfSaMacArr[1], sizeof(TGF_MAC_ADDR));
    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfCscdDsaSkipFdbSaLearningTestConfigurationSet function
* @endinternal
*
* @brief   Test initial configurations
*/
GT_VOID prvTgfCscdDsaSkipFdbSaLearningTestConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                        rc;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* initialize randomizer */
    cpssOsSrand(prvUtfSeedFromStreamNameGet());

    /* AUTODOC: save prvTgfPortsArray */
    cpssOsMemCpy(portsArrayForRestore, prvTgfPortsArray,
                 sizeof(portsArrayForRestore));

    /* AUTODOC: set the Ingress port to be DSA tagged */
    rc = tgfTrafficGeneratorIngressCscdPortEnableSet(
            prvTgfDevNum,
            prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS],
            GT_TRUE);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "Failed : tgfTrafficGeneratorIngressCscdPortEnableSet:[%d]", prvTgfDevNum);

    /* AUTODOC: Set the global Auto learning Mode */
    rc = cpssDxChBrgFdbSaLookupSkipModeSet(prvTgfDevNum, CPSS_DXCH_FDB_SA_AUTO_LEARNING_MODE_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbSaLookupSkipModeSet failed!\n");

}

/**
* @internal prvTgfCscdDsaSkipFdbSaLearningTestConfigurationRestore function
* @endinternal
*
* @brief   Test restore configurations
*/
GT_VOID prvTgfCscdDsaSkipFdbSaLearningTestConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                   rc;

    /* AUTODOC: RESTORE CONFIGURATION: */
    /* AUTODOC: Restore ingress port */
    rc = tgfTrafficGeneratorIngressCscdPortEnableSet(
            prvTgfDevNum,
            prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS],
            GT_FALSE);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "Failed : tgfTrafficGeneratorIngressCscdPortEnableSet:[%d]", prvTgfDevNum);

    /* AUTODOC: Configure default Port mode not to Skip the SA learnng*/
    rc = cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet(prvTgfDevNum,
                                    prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS],
                                    GT_FALSE /*acceptSkipSaLookup*/ );
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet failed!\n");

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", GT_TRUE);

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, portsArrayForRestore, sizeof(portsArrayForRestore));
}

/**
* @internal prvTgfCscdDsaSkipFdbSaLearningTest function
* @endinternal
*
* @brief   Test for SKIP FDB SA lookup in Forward eDsa tag
*          Send 2 pakcets with different SA MAC before and after setting "AcceptFdbSaLookupSkipEnable" bit
*          Verify the FDB table MAC belongs to the packet sent before setting "AcceptFdbSaLookupSkipEnable" should be learnt,
*          MAC belongs to packet sent after setting "AcceptFdbSaLookupSkipEnable" should not be present in FDB table.
*/
GT_VOID prvTgfCscdDsaSkipFdbSaLearningTest
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC     physicalInfo;

    cpssOsMemSet(&physicalInfo, 0, sizeof(physicalInfo));

    /* AUTODOC: Set common test configuration */
    prvTgfCscdDsaSkipFdbSaLearningTestConfigurationSet();

    /* AUTODOC: from Network port to Cascade port */
    PRV_UTF_LOG2_MAC(
            "\n*** from Cascade port %02d to Network port %02d ***\n",
            prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS],
            prvTgfPortsArray[EGR_NETWORK_PORT_IDX_CNS]);

    /* AUTODOC: Send packet to cascade port*/
    prvTgfCscdDsaSkipFdbSaLearningTestSendAndCheck(ING_CASCADE_PORT_IDX_CNS,
                                                  &prvTgfCascadePacketInfo);

    /* AUTODOC: Restore common test configuration */ 
    prvTgfCscdDsaSkipFdbSaLearningTestConfigurationRestore();
}
