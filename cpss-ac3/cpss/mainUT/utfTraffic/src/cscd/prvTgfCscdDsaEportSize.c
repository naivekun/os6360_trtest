/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfBridgeDsaEportSize.c
*
* DESCRIPTION:
*       Eport size in Forward eDsa tag test advanced UT.
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
#include <cscd/prvTgfCscdDsaEportSizeCheck.h>

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

/********************** DSA tag  ***************************/
static TGF_DSA_DSA_FORWARD_STC  prvTgfPacketDsaTagPart_forward = {
    PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E , /*srcIsTagged*/
    16,/*srcHwDev*/
    GT_FALSE,/* srcIsTrunk */
    /*union*/
    {
        /*trunkId*/
        0/*portNum*/
    },/*source;*/

    0,/*srcId*/

    GT_FALSE,/*egrFilterRegistered*/
    GT_FALSE,/*wasRouted*/
    0,/*qosProfileIndex*/

    /*CPSS_INTERFACE_INFO_STC         dstInterface*/
    {
        CPSS_INTERFACE_VID_E,/*type*/

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
        0/*portNum*/
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
        0,/*vid*/
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
    {TGF_PACKET_PART_DSA_TAG_E,  &prvTgfPacketDsaTagPart},
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + 2 * TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/* LENGTH of packet */
#define PRV_TGF_CASCADE_PACKET_LEN_CNS \
 (TGF_L2_HEADER_SIZE_CNS + TGF_DSA_TAG_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS \
                                                + sizeof(prvTgfPayloadDataArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_CASCADE_PACKET_CRC_LEN_CNS  (PRV_TGF_CASCADE_PACKET_LEN_CNS \
                                                          + TGF_CRC_LEN_CNS)

/******************************************************************************\
 *                   Private variables & definitions                          *
\******************************************************************************/

/* Parameters needed to be restored */
static GT_U32                            portsArrayForRestore[PRV_TGF_MAX_PORTS_NUM_CNS];
static CPSS_INTERFACE_INFO_STC           savePhyInfo;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfEDsaTagRedirectMapEport2PhySet function
* @endinternal
*
* @brief   Maps ePort to physical interface
*
* @param[in] devNum                   - devNum
* @param[in] portNum                  -  ePort
* @param[in] phyInfoPtr               -  (pointer to) inteface to map
* @param[in] savePhyInfoPtr           -  (pointer to) save inteface
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfEDsaTagRedirectMapEport2PhySet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      portNum,
    IN  CPSS_INTERFACE_INFO_STC          *phyInfoPtr,
    OUT CPSS_INTERFACE_INFO_STC          *savePhyInfoPtr
)
{
    GT_STATUS rc;

    if (savePhyInfoPtr != NULL)
    {
        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(
            devNum, portNum, savePhyInfoPtr);
        PRV_UTF_VERIFY_GT_OK(
            rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableGet");
    }

    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(
        devNum, portNum, phyInfoPtr);
    PRV_UTF_VERIFY_GT_OK(
        rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet");

    return GT_OK;
}

/**
* @internal prvTgfCscdDsaEportSizeTestSendAndCheck function
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
static GT_VOID prvTgfCscdDsaEportSizeTestSendAndCheck
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
    TGF_DSA_DSA_FORWARD_STC    *forwardInfoPtr;

    /* AUTODOC: reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* bind the DSA tag FORWARD part */
    prvTgfPacketDsaTagPart.dsaInfo.forward = prvTgfPacketDsaTagPart_forward;

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

    /* get first entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E,
            GT_TRUE, GT_TRUE, packetBuff,
            &buffLen, &packetActualLength,
            &devNum, &queue, &rxParam);

    /* parse the DSA (it should be FORWARTD eDSA) so we can get the target ePort from it */
    rc = prvTgfTrafficGeneratorPacketDsaTagParse(
            &packetBuff[TGF_L2_HEADER_SIZE_CNS],/* start of DSA after the 12 bytes of the SA,DA mac addresses */
            &dsaInfo);

    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    forwardInfoPtr = &dsaInfo.dsaInfo.forward;

    UTF_VERIFY_EQUAL0_STRING_MAC(
            TGF_DSA_CMD_FORWARD_E, dsaInfo.dsaCommand,
            "EportSize Test: not FORWARD DSA");
    
    UTF_VERIFY_EQUAL0_STRING_MAC(
            TGF_DSA_4_WORD_TYPE_E, dsaInfo.dsaType,
            "EportSize Test: not 4 WORD DSA");
    
    UTF_VERIFY_EQUAL0_STRING_MAC(
            PRV_TGF_DSA_PORT_CNS, forwardInfoPtr->dstEport,
            "EportSize Test: Destination eport not matched");
}

/**
* @internal prvTgfCscdDsaEportSizeTestConfigurationSet function
* @endinternal
*
* @brief   Test initial configurations
*/
GT_VOID prvTgfCscdDsaEportSizeTestConfigurationSet
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

    /* AUTODOC: map ePort 300 to target physical port 2 */
    rc = prvTgfEDsaTagRedirectMapEport2PhySet(prvTgfDevNum, PRV_TGF_DSA_PORT_CNS,
            &targetPortInfo, &savePhyInfo);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                         "Failed : prvTgfEDsaTagRedirectMapEport2PhySet[%d]", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:17, VLAN 1, ePort 0x300 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacketL2Part.daMac, PRV_TGF_VLANID_1_CNS,
            prvTgfDevNum,
            PRV_TGF_DSA_PORT_CNS,
            GT_TRUE);

    UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfCscdDsaEportSizeTestConfigurationRestore function
* @endinternal
*
* @brief   Test restore configurations
*/
GT_VOID prvTgfCscdDsaEportSizeTestConfigurationRestore
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
* @internal prvTgfCscdDsaEportSizeTest function
* @endinternal
*
* @brief   Test for Egress Filter Registered field in Forward eDsa tag
*/
GT_VOID prvTgfCscdDsaEportSizeCheck
(
    GT_VOID
)
{
    /* AUTODOC: Set common test configuration */
    prvTgfCscdDsaEportSizeTestConfigurationSet();

    /* AUTODOC: from Network port to Cascade port */
        PRV_UTF_LOG2_MAC(
            "\n*** from Network port %02d to Cascade port %02d ***\n",
            prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
            prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS]);

    /* AUTODOC: Send DSA taged packet*/
    prvTgfCscdDsaEportSizeTestSendAndCheck(ING_NETWORK_PORT_IDX_CNS,
                                                  EGR_CASCADE_PORT_IDX_CNS,
                                                  &prvTgfPacketInfo);

    /* AUTODOC: restore map ePort to physical port */
    prvTgfEDsaTagRedirectMapEport2PhySet( prvTgfDevNum, PRV_TGF_DSA_PORT_CNS,
                                           &savePhyInfo, NULL);

    /* AUTODOC: Restore common test configuration */
    prvTgfCscdDsaEportSizeTestConfigurationRestore();
}
