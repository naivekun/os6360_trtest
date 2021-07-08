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
*       FDB SA lookup controlling test in forward eDsa tag.
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
#define ING_NETWORK_PORT_IDX_CNS    0

/* target port */
#define EGR_CASCADE_PORT_IDX_CNS    2

/* final target device & port */
#define PRV_TGF_DSA_SRC_DEV_CNS     8
#define PRV_TGF_DSA_PORT_CNS        300

/******************************* Test packets *********************************/

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

/* Bypass PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
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

/******************************************************************************\
 *                   Private variables & definitions                          *
\******************************************************************************/

/* Parameters needed to be restored */
static GT_U32                            portsArrayForRestore[PRV_TGF_MAX_PORTS_NUM_CNS];

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfCscdDsaSkipFdbSaLookupTestSendAndCheck function
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
static GT_VOID prvTgfCscdDsaSkipFdbSaLookupTestSendAndCheck
(
    IN GT_U32                   ingressPortIndex,
    IN GT_U32                   egressPortIndex,
    IN TGF_PACKET_STC           *packetInfoPtr
)
{
    GT_U8                       packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32                      buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32                      packetActualLength = 0;
    GT_U8                       devNum;
    GT_U8                       queue;
    GT_U32                      portIter;
    GT_U32                      portsCount = prvTgfPortsNum;
    GT_STATUS                   rc = GT_OK;
    TGF_NET_DSA_STC             rxParam;
    TGF_PACKET_DSA_TAG_STC      dsaInfo;

    /* AUTODOC: reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: Set the global Auto learning Mode */
    rc = cpssDxChBrgFdbSaLookupSkipModeSet(prvTgfDevNum, CPSS_DXCH_FDB_SA_AUTO_LEARNING_MODE_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbSaLookupSkipModeSet failed!\n");

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

    /* AUTODOC: parse the DSA (it should be FORWARTD eDSA) so we can get the target ePort from it */
    rc = prvTgfTrafficGeneratorPacketDsaTagParse(
            &packetBuff[TGF_L2_HEADER_SIZE_CNS],/* start of DSA after the 12 bytes of the SA,DA mac addresses */
            &dsaInfo);

    UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfTrafficGeneratorPacketDsaTagParse");

    UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_TRUE, dsaInfo.dsaInfo.forward.skipFdbSaLookup,
            "FdbSaLookup Test: SA lookup TRUE not matched");

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", GT_TRUE);

    /* Set the global Skip SA lookup mode*/
    rc = cpssDxChBrgFdbSaLookupSkipModeSet(prvTgfDevNum, CPSS_DXCH_FDB_SA_MATCHING_MODE_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbSaLookupSkipModeSet failed!\n");

    /* AUTODOC: send packet. */
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

    /* get next entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E,
            GT_TRUE, GT_TRUE, packetBuff,
            &buffLen, &packetActualLength,
            &devNum, &queue, &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                             TGF_PACKET_TYPE_CAPTURE_E, GT_TRUE);

    /* AUTODOC: parse the DSA (it should be FORWARTD eDSA) so we can get the target ePort from it */
    rc = prvTgfTrafficGeneratorPacketDsaTagParse(
            &packetBuff[TGF_L2_HEADER_SIZE_CNS],/* start of DSA after the 12 bytes of the SA,DA mac addresses */
            &dsaInfo);

    UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfTrafficGeneratorPacketDsaTagParse");

    UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_FALSE, dsaInfo.dsaInfo.forward.skipFdbSaLookup,
            "FdbSaLookup Test: SA lookup False not matched");
}

/**
* @internal prvTgfCscdDsaSkipFdbSaLookupTestConfigurationSet function
* @endinternal
*
* @brief   Test initial configurations
*/
GT_VOID prvTgfCscdDsaSkipFdbSaLookupTestConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                        rc;
    CPSS_INTERFACE_INFO_STC          targetPortInfo;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* initialize randomizer */
    cpssOsSrand(prvUtfSeedFromStreamNameGet());

    /* target port detail to be mapped */
    cpssOsMemSet(&targetPortInfo, 0, sizeof(targetPortInfo));
    targetPortInfo.type = CPSS_INTERFACE_PORT_E;
    targetPortInfo.devPort.hwDevNum = prvTgfDevNum;
    targetPortInfo.devPort.portNum = prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS];

    /* AUTODOC: save prvTgfPortsArray */
    cpssOsMemCpy(portsArrayForRestore, prvTgfPortsArray,
                 sizeof(portsArrayForRestore));

    /* AUTODOC: set the egress port to be DSA tagged */
    rc = tgfTrafficGeneratorEgressCscdPortEnableSet(
            prvTgfDevNum,
            prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS],
            GT_TRUE,
            CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                         "Failed : tgfTrafficGeneratorEgressCscdPortEnableSet:[%d]", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:17, VLAN 1, ePort 0x300 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacketL2Part.daMac, PRV_TGF_VLANID_1_CNS,
            prvTgfDevNum,
            prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS],
            GT_TRUE);

    UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfCscdDsaSkipFdbSaLookupTestConfigurationRestore function
* @endinternal
*
* @brief   Test restore configurations
*/
GT_VOID prvTgfCscdDsaSkipFdbSaLookupTestConfigurationRestore
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

    /* AUTODOC: Set the global default learning Mode */
    rc = cpssDxChBrgFdbSaLookupSkipModeSet(prvTgfDevNum, CPSS_DXCH_FDB_SA_AUTO_LEARNING_MODE_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbSaLookupSkipModeSet failed!\n");

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "Failed : tgfTrafficGeneratorEgressCscdPortEnableSet:[%d]", prvTgfDevNum);

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
* @internal prvTgfCscdDsaSkipFdbSaLookupTest function
* @endinternal
*
* @brief   Test for SKIP FDB SA lookup in Forward eDsa tag
*          Send 2 pakcets before and after setting SA Skip bit
*          Verify the packets egressed from cascade port.
*          The packet before the configuration should have 0(SA skip bit not set)
*          The packet After the configuration should have 1(SA skip bit set)
*/
GT_VOID prvTgfCscdDsaSkipFdbSaLookupTest
(
    GT_VOID
)
{
    /* AUTODOC: Set common test configuration */
    prvTgfCscdDsaSkipFdbSaLookupTestConfigurationSet();

    /* AUTODOC: from Network port to Cascade port */
    PRV_UTF_LOG2_MAC(
            "\n*** from Network port %02d to Cascade port %02d ***\n",
            prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
            prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS]);

    /* AUTODOC: Send packet to cascade port*/
    prvTgfCscdDsaSkipFdbSaLookupTestSendAndCheck(ING_NETWORK_PORT_IDX_CNS,
                                                  EGR_CASCADE_PORT_IDX_CNS,
                                                  &prvTgfPacketInfo);

    /* AUTODOC: Restore common test configuration */
    prvTgfCscdDsaSkipFdbSaLookupTestConfigurationRestore();
}

