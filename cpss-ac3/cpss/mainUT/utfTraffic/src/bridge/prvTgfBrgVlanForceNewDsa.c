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
* @file prvTgfBrgVlanForceNewDsa.c
*
* @brief New DSA tag assignment
*
* @version   1
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <common/tgfCommon.h>
#include <common/tgfMirror.h>
#include <common/tgfBridgeGen.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <bridge/prvTgfBrgVlanForceNewDsa.h>

#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>

#define PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS 0
#define PRV_TGF_PORT_NUM prvTgfPortsArray[0]
#define NEW_VLAN_ID 4101
#define OLD_VLAN_ID 12

static GT_BOOL newDsaEnabled;
static GT_U16 forcedVlanId;
static GT_BOOL   prvTgfEnableMirror = GT_FALSE;
static GT_U32    prvTgfIndex        = 0;
static PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC originalAnalyzerInterface;

/* default number of packets to send */
static GT_U32       prvTgfBurstCount   = 1;


/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22},               /* srcMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0xFF}                /* dstMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, OLD_VLAN_ID                                   /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                        /* dataLength */
    prvTgfPayloadDataArr                                 /* dataPtr */
};

static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
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

/**
* @internal prvTgfBrgVlanForceNewDsaToCpuConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*/
GT_VOID prvTgfBrgVlanForceNewDsaToCpuConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC  interface;

    /* AUTODOC: Set VLAN for all of test ports */
    rc = prvTgfBrgDefVlanEntryWrite(NEW_VLAN_ID);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: Save and set enabled state of new DSA tag forcing */
    rc = prvTgfBrgVlanForceNewDsaToCpuEnableGet(prvTgfDevNum, &newDsaEnabled);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);
    rc = prvTgfBrgVlanForceNewDsaToCpuEnableSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: Save and set assigning new VID on rx port */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, PRV_TGF_PORT_NUM, &forcedVlanId);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, prvTgfDevNum, PRV_TGF_PORT_NUM);
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, PRV_TGF_PORT_NUM, NEW_VLAN_ID);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, rc, prvTgfDevNum, PRV_TGF_PORT_NUM,
        NEW_VLAN_ID);

    /* AUTODOC: Enable PVID forcing */
    rc = prvTgfBrgVlanPortForcePvidEnable(prvTgfDevNum, PRV_TGF_PORT_NUM,
        GT_TRUE);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, rc, prvTgfDevNum, PRV_TGF_PORT_NUM,
        GT_TRUE);

    /* AUTODOC: Set up mirroring to CPU in order to create TO_CPU packet */
    /* AUTODOC: Enable VLAN ingress mirroring */
    rc = prvTgfBrgVlanIngressMirrorEnable(prvTgfDevNum, NEW_VLAN_ID, GT_TRUE,
        PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS);
    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, rc, prvTgfDevNum, NEW_VLAN_ID, GT_TRUE,
        PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS);

    /* AUTODOC: Save default RX global analyzer interface */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet(prvTgfDevNum,
        &prvTgfEnableMirror, &prvTgfIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);

    /* AUTODOC: Enable global Rx mirroring */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(GT_TRUE,
        PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, GT_TRUE,
        PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS);

    /* AUTODOC: Save original analyzer interface settings */
    prvTgfMirrorAnalyzerInterfaceGet(PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS,
        prvTgfDevNum, &originalAnalyzerInterface);

    /* AUTODOC: Set analyzer port for analyzer */
    interface.interface.type = CPSS_INTERFACE_PORT_E;
    interface.interface.devPort.hwDevNum = prvTgfDevNum;
    interface.interface.devPort.portNum = CPSS_CPU_PORT_NUM_CNS;
    rc = prvTgfMirrorAnalyzerInterfaceSet(PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS,
        &interface);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS);

    /* AUTODOC: Set source-based forwarding mode */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(
        PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc,
        PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E);
}

/**
* @internal prvTgfBrgVlanForceNewDsaToCpuTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfBrgVlanForceNewDsaToCpuTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U8 packetBuf[256];
    GT_U32 packetBufLen = 256;
    GT_U32 packetLen;
    GT_U8  devNum;
    GT_U8  queue;
    CPSS_DXCH_NET_RX_PARAMS_STC dxChPcktParams;

    /* AUTODOC: Reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* AUTODOC: Setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo,
        prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d",
        prvTgfDevNum);

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* AUTODOC: Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, PRV_TGF_PORT_NUM);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
        "ERROR of StartTransmitting: %d, %d\n", prvTgfDevNum, PRV_TGF_PORT_NUM);

    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    tgfTrafficGeneratorRxInCpuNumWait(1 , 0 , NULL);

    rc = tgfTrafficTableRxPcktGet(0, packetBuf, &packetBufLen, &packetLen,
        &devNum, &queue, (GT_VOID *)&dxChPcktParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(NEW_VLAN_ID,
        dxChPcktParams.dsaParam.commonParams.vid,
        "Expected VLAN ID mismatch");

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");
}

/**
* @internal prvTgfBrgVlanForceNewDsaToCpuConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanForceNewDsaToCpuConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: Restore force new DSA enabled state */
    rc = prvTgfBrgVlanForceNewDsaToCpuEnableSet(newDsaEnabled);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, newDsaEnabled);

    /* AUTODOC: Restore PVID */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, PRV_TGF_PORT_NUM, forcedVlanId);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, rc, prvTgfDevNum, PRV_TGF_PORT_NUM,
        forcedVlanId);

    /* AUTODOC: Force PVID enabled state */
    rc = prvTgfBrgVlanPortForcePvidEnable(prvTgfDevNum, PRV_TGF_PORT_NUM,
        GT_FALSE);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, rc, prvTgfDevNum, PRV_TGF_PORT_NUM,
        GT_FALSE);

    /* AUTODOC: Restore original analyzer interface settings */
    rc = prvTgfMirrorAnalyzerInterfaceSet(PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS,
        &originalAnalyzerInterface);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS);

    /* AUTODOC: Restore global Rx mirroring configuration */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(prvTgfEnableMirror,
        prvTgfIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfEnableMirror);

    /* AUTODOC: Disable VLAN ingress mirroring. */
    rc = prvTgfBrgVlanIngressMirrorEnable(prvTgfDevNum, 0, GT_FALSE,
        PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS);
    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, rc, prvTgfDevNum, 0, GT_FALSE,
        PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS);

    /* AUTODOC: restore default forwarding mode for mirroring to analyzer */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(
        PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc,
        PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);

    /* AUTODOC: Invalidate VLAN */
    rc = prvTgfBrgDefVlanEntryInvalidate(NEW_VLAN_ID);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, NEW_VLAN_ID);

    /* AUTODOC: Flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, GT_TRUE);
}

