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
* @file prvTgfLion2Fabric.c
*
* @brief App Demo Lion2Fabric testing implementation.
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <common/tgfLion2FabricGen.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <trafficEngine/private/prvTgfTrafficParser.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPortGen.h>
#include <common/tgfCosGen.h>
#include <common/tgfLion2FabricGen.h>
#include <lion2Fabric/prvTgfLion2Fabric.h>

typedef struct
{
    GT_U32  portIndex; /* index in prvTgfDevsArray/prvTgfPortsArray */
    GT_BOOL rxPort;    /* should receive packets                    */
    GT_BOOL txPort;    /* should send packets                       */
} PRV_TGF_LION2_FABRIC_PORT_USE_STC;

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x12, 0x34, 0x56, 0x78, 0x9A},                /* daMac */
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55}                 /* saMac */
};

/* TGF_PACKET_DSA_TAG_STC will be calculated dynamy */

/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart =
{
    0x3333
};

/* Payload of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPacketPayloadDataArr),            /* dataLength */
    prvTgfPacketPayloadDataArr                     /* dataPtr */
};

/* Packet Parts Array Patter*/
static TGF_PACKET_PART_STC prvTgfPacketPartsArrayPattern[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_DSA_TAG_E,   NULL}, /* will be overriden */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* number of packet parts for eDSA tagged packets in the tests */
#define PRV_TGF_LION2_FABRIC_PKT_PARTS_NUM_CNS \
    (sizeof(prvTgfPacketPartsArrayPattern) / sizeof(TGF_PACKET_PART_STC))

/* total packet size */
#define PRV_TGF_LION2_FABRIC_PKT_SIZE_CNS                                    \
    (TGF_L2_HEADER_SIZE_CNS + TGF_eDSA_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + sizeof(prvTgfPacketPayloadDataArr))

/* Tagged IPV4 Other Passenger Packet Info */
static TGF_PACKET_STC prvTgfPacketInfoPattern = {
    PRV_TGF_LION2_FABRIC_PKT_SIZE_CNS,      /* totalLen */
    PRV_TGF_LION2_FABRIC_PKT_PARTS_NUM_CNS, /* numOfParts */
    (TGF_PACKET_PART_STC*)NULL              /* partsArray */
};

/* VIDX for VLAN flooding */
#define PRV_TGF_LION2_FABRIC_VIDX_FOR_VLAN_ID_CNS 0xFFF

/* used bridge counter set */
#define PRV_TGF_LION2_FABRIC_BRG_CNT_SET_CNS 1

/**
* @internal prvTgfLion2FabricBuildPacket function
* @endinternal
*
* @brief   Builds eDSA tagged packet :
*
* @param[in] dsaCommand               - dsa Command
* @param[in] vlanId                   - vlanId
* @param[in] targetIsDevice           - GT_TRUE - Device, GT_FALSE - VIDX (or VLAN)
* @param[in] devHwIdOrVidx            - device HW Id Or Vidx (according to targetIsDevice)
*                                      for VLAN use PRV_TGF_LION2_FABRIC_VIDX_FOR_VLAN_ID_CNS
* @param[in] tc                       - TC
* @param[in] dp                       - DP
*
* @param[out] prvTgfPacketDsaTagPtr    - (pointer to)memory for eDSA tag descriptor
* @param[out] prvTgfPacketPartsArray[] - (pointer to)memory for packet parts array
*                                      sized PRV_TGF_LION2_FABRIC_PKT_PARTS_NUM_CNS
* @param[out] prvTgfPacketInfoPtr      - (pointer to)memory for packet info
*                                       None
*/
static GT_VOID prvTgfLion2FabricBuildPacket
(
    IN TGF_DSA_CMD_ENT                       dsaCommand,
    IN  GT_U16                               vlanId,
    IN  GT_BOOL                              targetIsDevice,
    IN  GT_U32                               devHwIdOrVidx,
    IN  GT_U8                                tc,
    IN  CPSS_DP_LEVEL_ENT                    dp,
    OUT TGF_PACKET_DSA_TAG_STC               *prvTgfPacketDsaTagPtr,
    OUT TGF_PACKET_PART_STC                  prvTgfPacketPartsArray[],
    OUT TGF_PACKET_STC                       *prvTgfPacketInfoPtr
)
{
    GT_U32 i;

    cpssOsMemCpy(
        prvTgfPacketPartsArray,
        prvTgfPacketPartsArrayPattern,
        sizeof(prvTgfPacketPartsArrayPattern));

    for (i = 0; (i < PRV_TGF_LION2_FABRIC_PKT_PARTS_NUM_CNS); i++)
    {
        if ((prvTgfPacketPartsArray[i].type == TGF_PACKET_PART_DSA_TAG_E)
            && (prvTgfPacketPartsArray[i].partPtr == NULL))
        {
            prvTgfPacketPartsArray[i].partPtr = prvTgfPacketDsaTagPtr;
        }
    }

    cpssOsMemCpy(
        prvTgfPacketInfoPtr,
        &prvTgfPacketInfoPattern,
        sizeof(prvTgfPacketInfoPattern));
    prvTgfPacketInfoPtr->partsArray = prvTgfPacketPartsArray;

    cpssOsMemSet(prvTgfPacketDsaTagPtr, 0, sizeof(TGF_PACKET_DSA_TAG_STC));
    prvTgfPacketDsaTagPtr->dsaType          = TGF_DSA_4_WORD_TYPE_E;
    prvTgfPacketDsaTagPtr->commonParams.vid = vlanId;
    prvTgfPacketDsaTagPtr->dsaCommand       = dsaCommand;
    switch (dsaCommand)
    {
        default:
        case TGF_DSA_CMD_TO_CPU_E:
            /* set HW CPU code to zero */
            prvTgfPacketDsaTagPtr->dsaInfo.toCpu.cpuCode =
                CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;
            /* no parameters needed */
            break;
        case TGF_DSA_CMD_FROM_CPU_E:
            if (targetIsDevice != GT_FALSE)
            {
                prvTgfPacketDsaTagPtr->dsaInfo.fromCpu.dstInterface.type =
                    CPSS_INTERFACE_PORT_E;
                prvTgfPacketDsaTagPtr->dsaInfo.fromCpu.dstInterface.devPort.hwDevNum =
                    (GT_HW_DEV_NUM)devHwIdOrVidx;
                prvTgfPacketDsaTagPtr->dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;
            }
            else
            {
                if (devHwIdOrVidx != PRV_TGF_LION2_FABRIC_VIDX_FOR_VLAN_ID_CNS)
                {
                    prvTgfPacketDsaTagPtr->dsaInfo.fromCpu.dstInterface.type =
                        CPSS_INTERFACE_VIDX_E;
                    prvTgfPacketDsaTagPtr->dsaInfo.fromCpu.dstInterface.vidx =
                        (GT_U16)devHwIdOrVidx;
                }
                else
                {
                    prvTgfPacketDsaTagPtr->dsaInfo.fromCpu.dstInterface.type =
                        CPSS_INTERFACE_VID_E;
                    prvTgfPacketDsaTagPtr->dsaInfo.fromCpu.dstInterface.vlanId =
                        (GT_U16)devHwIdOrVidx;
                }
            }
            prvTgfPacketDsaTagPtr->dsaInfo.fromCpu.tc = tc;
            /* DP not relevant                                 */
            /* prvTgfPacketDsaTagPtr->dsaInfo.fromCpu.dp = dp; */
            break;
        case TGF_DSA_CMD_TO_ANALYZER_E:
            /* TC and DP not relevant                          */
            if (targetIsDevice != GT_FALSE)
            {
                prvTgfPacketDsaTagPtr->dsaInfo.toAnalyzer.analyzerTrgType =
                    CPSS_INTERFACE_PORT_E;
                prvTgfPacketDsaTagPtr->dsaInfo.toAnalyzer.extDestInfo.devPort.analyzerHwTrgDev =
                    (GT_HW_DEV_NUM)devHwIdOrVidx;
                prvTgfPacketDsaTagPtr->dsaInfo.toAnalyzer.extDestInfo.devPort.analyzerIsTrgPortValid =
                    GT_TRUE;
            }
            else
            {
                prvTgfPacketDsaTagPtr->dsaInfo.toAnalyzer.analyzerTrgType =
                    CPSS_INTERFACE_VIDX_E;
                prvTgfPacketDsaTagPtr->dsaInfo.toAnalyzer.extDestInfo.multiDest.analyzerEvidx =
                    (GT_U16)devHwIdOrVidx;
            }
            break;
        case TGF_DSA_CMD_FORWARD_E:
            if (targetIsDevice != GT_FALSE)
            {
                prvTgfPacketDsaTagPtr->dsaInfo.forward.dstInterface.type =
                    CPSS_INTERFACE_PORT_E;
                prvTgfPacketDsaTagPtr->dsaInfo.forward.dstInterface.devPort.hwDevNum =
                    (GT_HW_DEV_NUM)devHwIdOrVidx;
                prvTgfPacketDsaTagPtr->dsaInfo.forward.isTrgPhyPortValid = GT_TRUE;
            }
            else
            {
                if (devHwIdOrVidx != PRV_TGF_LION2_FABRIC_VIDX_FOR_VLAN_ID_CNS)
                {
                    prvTgfPacketDsaTagPtr->dsaInfo.forward.dstInterface.type =
                        CPSS_INTERFACE_VIDX_E;
                    prvTgfPacketDsaTagPtr->dsaInfo.forward.dstInterface.vidx =
                        (GT_U16)devHwIdOrVidx;
                }
                else
                {
                    prvTgfPacketDsaTagPtr->dsaInfo.forward.dstInterface.type =
                        CPSS_INTERFACE_VID_E;
                    prvTgfPacketDsaTagPtr->dsaInfo.forward.dstInterface.vlanId =
                        (GT_U16)devHwIdOrVidx;
                }
            }
            prvTgfPacketDsaTagPtr->dsaInfo.forward.qosProfileIndex = ((tc << 2) | dp);
            break;
    }
}

/**
* @internal prvTgfLion2FabricSendPacket function
* @endinternal
*
* @brief   Send eDSA tagged packet :
*
* @param[in] devNum                   - ingress device number
* @param[in] portNum                  - ingress port number
* @param[in] burstCount               - number of frames
* @param[in] dsaCommand               - dsa Command
* @param[in] vlanId                   - vlanId
* @param[in] targetIsDevice           - GT_TRUE - Device, GT_FALSE - VIDX (or VLAN)
* @param[in] devHwIdOrVidx            - device HW Id Or Vidx (according to targetIsDevice)
*                                      for VLAN use PRV_TGF_LION2_FABRIC_VIDX_FOR_VLAN_ID_CNS
* @param[in] tc                       - TC
* @param[in] dp                       - DP
*                                       None
*/
static GT_STATUS prvTgfLion2FabricSendPacket
(
    IN  GT_U8                                devNum,
    IN  GT_U32                                portNum,
    IN  GT_U32                               burstCount,
    IN  TGF_DSA_CMD_ENT                      dsaCommand,
    IN  GT_U16                               vlanId,
    IN  GT_BOOL                              targetIsDevice,
    IN  GT_U32                               devHwIdOrVidx,
    IN  GT_U8                                tc,
    IN  CPSS_DP_LEVEL_ENT                    dp
)
{
    GT_STATUS                       rc = GT_OK;
    OUT TGF_PACKET_DSA_TAG_STC      prvTgfPacketDsaTag;
    OUT TGF_PACKET_PART_STC         prvTgfPacketPartsArray[PRV_TGF_LION2_FABRIC_VIDX_FOR_VLAN_ID_CNS];
    OUT TGF_PACKET_STC              prvTgfPacketInfo;

    prvTgfLion2FabricBuildPacket(
        dsaCommand, vlanId, targetIsDevice, devHwIdOrVidx, tc, dp,
        &prvTgfPacketDsaTag, prvTgfPacketPartsArray, &prvTgfPacketInfo);

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(
        devNum, &prvTgfPacketInfo,
        burstCount, 0 /*numVfd*/, NULL /*vfdArray*/);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(devNum, portNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    return GT_OK;
}

/**
* @internal prvTgfLion2FabricSendAndCheckPacket function
* @endinternal
*
* @brief   Send eDSA tagged packet and check:
*
* @param[in] portsNum                 - number of checked ports
* @param[in] portsArr[]               - array of checked ports
* @param[in] sendPortIndex            - send Port Index
* @param[in] burstCount               - number of frames
* @param[in] dsaCommand               - dsa Command
* @param[in] vlanId                   - vlanId
* @param[in] targetIsDevice           - GT_TRUE - Device, GT_FALSE - VIDX (or VLAN)
* @param[in] devHwIdOrVidx            - device HW Id Or Vidx (according to targetIsDevice)
*                                      for VLAN use PRV_TGF_LION2_FABRIC_VIDX_FOR_VLAN_ID_CNS
* @param[in] tc                       - TC
* @param[in] dp                       - DP
* @param[in] sourcePktQosIsSame       - source Packet Qos Is Same
*                                       None
*/
static GT_VOID prvTgfLion2FabricSendAndCheckPacket
(
    IN  GT_U32                               portsNum,
    IN PRV_TGF_LION2_FABRIC_PORT_USE_STC     portsArr[],
    IN  GT_U32                               sendPortIndex,
    IN  GT_U32                               burstCount,
    IN TGF_DSA_CMD_ENT                       dsaCommand,
    IN  GT_U16                               vlanId,
    IN  GT_BOOL                              targetIsDevice,
    IN  GT_U32                               devHwIdOrVidx,
    IN  GT_U8                                tc,
    IN  CPSS_DP_LEVEL_ENT                    dp,
    IN  GT_BOOL                              sourcePktQosIsSame
)
{
    GT_STATUS                   rc;
    CPSS_PORTS_BMP_STC          allPortsBmp;
    CPSS_PORTS_BMP_STC          rxPortsBmp;
    CPSS_PORTS_BMP_STC          txPortsBmp;
    GT_U32                      i;
    GT_PHYSICAL_PORT_NUM        portNum;
    GT_U32                      expectedEgressPorts;
    GT_U32                      outFrames;
    CPSS_PORT_EGRESS_CNTR_STC   egrCntr;
    CPSS_QOS_ENTRY_STC          portQosCfg;

    /* Force Soft Qos Profile Assignment Precedence */
    /* It should be default                         */
    /* Needed to be overriden by PCL.              */
    cpssOsMemSet(&portQosCfg, 0, sizeof(portQosCfg));
    portQosCfg.assignPrecedence =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    portQosCfg.enableModifyDscp =
        CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    portQosCfg.enableModifyUp =
        CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    rc = prvTgfCosPortQosConfigSet(
        prvTgfPortsArray[sendPortIndex],
        &portQosCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCosPortQosConfigSet");

    prvTgfEthCountersReset(prvTgfDevsArray[sendPortIndex]);

    /* portNum used only to determinate devNum */
    rc = prvTgfPortEgressCntrModeSet(
        PRV_TGF_LION2_FABRIC_BRG_CNT_SET_CNS,
        (CPSS_EGRESS_CNT_TC_E | CPSS_EGRESS_CNT_DP_E),
        prvTgfPortsArray[sendPortIndex] /*portNum*/, 0 /*vlanId*/,
        tc, dp);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPortEgressCntrModeSet");


    rc = prvTgfLion2FabricSendPacket(
        prvTgfDevsArray[sendPortIndex], prvTgfPortsArray[sendPortIndex],
        burstCount, dsaCommand, vlanId, targetIsDevice, devHwIdOrVidx, tc, dp);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfLion2FabricSendPacket");

    expectedEgressPorts = 0;
    cpssOsMemSet(&allPortsBmp, 0, sizeof(allPortsBmp));
    cpssOsMemSet(&rxPortsBmp, 0, sizeof(rxPortsBmp));
    cpssOsMemSet(&txPortsBmp, 0, sizeof(txPortsBmp));
    for (i = 0; (i < portsNum); i++)
    {
        portNum = prvTgfPortsArray[portsArr[i].portIndex];
        CPSS_PORTS_BMP_PORT_SET_MAC((&allPortsBmp), portNum);
        if (portsArr[i].rxPort != GT_FALSE)
        {
            CPSS_PORTS_BMP_PORT_SET_MAC((&rxPortsBmp), portNum);
        }
        if (portsArr[i].txPort != GT_FALSE)
        {
            CPSS_PORTS_BMP_PORT_SET_MAC((&txPortsBmp), portNum);
            if ((portsArr[i].portIndex != sendPortIndex)
                || (sourcePktQosIsSame != GT_FALSE))
            {
                expectedEgressPorts ++;
            }
        }
    }
    if (sourcePktQosIsSame != GT_FALSE)
    {
        if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(
            (&allPortsBmp), prvTgfPortsArray[sendPortIndex]) == 0)
        {
            /* if ingress looped back port out of tested list */
            expectedEgressPorts ++;
        }
    }

    rc = prvTgfEthPortSetCountersCheck(
        prvTgfDevsArray[sendPortIndex],
        &allPortsBmp, &rxPortsBmp, &txPortsBmp,
        PRV_TGF_LION2_FABRIC_PKT_SIZE_CNS /*packetSize*/, burstCount);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfEthPortSetCountersCheck");

    rc = prvTgfPortEgressCntrsGet(
        PRV_TGF_LION2_FABRIC_BRG_CNT_SET_CNS, &egrCntr);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPortEgressCntrsGet");

    PRV_UTF_LOG3_MAC(
        "egress counters: outUcFrames %d, outMcFrames %d, outBcFrames %d\n",
        egrCntr.outUcFrames, egrCntr.outMcFrames, egrCntr.outBcFrames);

    outFrames =  egrCntr.outUcFrames + egrCntr.outMcFrames + egrCntr.outBcFrames;
    UTF_VERIFY_EQUAL0_STRING_MAC(
        (expectedEgressPorts * burstCount), outFrames, "Egress Countners Summary");
}

/**
* @internal prvTgfLion2FabricToDev function
* @endinternal
*
* @brief   Set test configuration:
*/
GT_VOID prvTgfLion2FabricToDev
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32                                sendPortIndex = 1;
    GT_U32                                cpuPortIndex = 3;
    GT_U32                                egressDevPortIndex = 2;
    PRV_TGF_LION2_FABRIC_PORT_USE_STC     portsArr[] =
    {
        {0, GT_FALSE, GT_FALSE},
        {1, GT_TRUE, GT_TRUE},
        {2, GT_FALSE, GT_TRUE},
        {3, GT_FALSE, GT_FALSE}
    };
    GT_U32                                portsNum =
        (sizeof(portsArr) / sizeof(portsArr[0]));
    GT_U32                                egressDevHwId = 0x33;
    GT_U32                                burstCount = 1;
    CPSS_PORTS_BMP_STC                    targetPortsBmp;
    GT_U8                                 packetTc = 5;
    CPSS_DP_LEVEL_ENT                     packetDp = CPSS_DP_YELLOW_E;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= prvTgfLion2FabricToDev =======\n");

    /* AUTODOC: init Lion2 Fabric */
    rc = prvTgfLion2FabricInit(
        prvTgfDevNum, prvTgfPortsArray[cpuPortIndex]);
    if (rc != GT_OK)
    {
        prvTgfLion2FabricCleanUp(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfLion2FabricInit");
        return;
    }

    /* AUTODOC: Lion2 Fabric: VIDX with ingress port */
    cpssOsMemSet(&targetPortsBmp, 0, sizeof(targetPortsBmp));
    CPSS_PORTS_BMP_PORT_SET_MAC((&targetPortsBmp), prvTgfPortsArray[sendPortIndex]);

    rc = prvTgfLion2FabricForwardAddBc2Vidx(
        prvTgfDevNum, 123 /*bc2Vidx*/, &targetPortsBmp);
    if (rc != GT_OK)
    {
        prvTgfLion2FabricCleanUp(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfLion2FabricForwardAddBc2Vidx");
        return;
    }

    /* AUTODOC: Lion2 Fabric Map egress device to port */
    rc = prvTgfLion2FabricForwardAddBc2Device(
        prvTgfDevNum, egressDevHwId, prvTgfPortsArray[egressDevPortIndex]);
    if (rc != GT_OK)
    {
        prvTgfLion2FabricCleanUp(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfLion2FabricForwardAddBc2Device");
        return;
    }

    /* AUTODOC: Lion2 Fabric Qos configuration */
    rc = prvTgfLion2FabricConfigureQos(
        prvTgfDevNum,
        packetTc/*toCpuTc*/,
        packetDp/*toCpuDp*/,
        packetTc/*toAnalyzerTc*/,
        packetDp/*toAnalyzerDp*/,
        packetDp/*fromCpuDp*/);
    if (rc != GT_OK)
    {
        prvTgfLion2FabricCleanUp(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfLion2FabricConfigureQos");
        return;
    }

    /* AUTODOC: Send forwarded packet and Check counters */
    PRV_UTF_LOG0_MAC("Send Forwarded packet\n");
    prvTgfLion2FabricSendAndCheckPacket(
        portsNum, portsArr, sendPortIndex, burstCount,
        TGF_DSA_CMD_FORWARD_E,
        1 /*vlanId*/,
        GT_TRUE /*targetIsDevice*/,
        egressDevHwId /*devHwIdOrVidx*/,
        packetTc /*tc*/,
        packetDp /*dp*/,
        GT_FALSE /*sourcePktQosIsSame*/);

    /* AUTODOC: Send From Cpu packet and Check counters */
    PRV_UTF_LOG0_MAC("Send From CPU packet\n");
    prvTgfLion2FabricSendAndCheckPacket(
        portsNum, portsArr, sendPortIndex, burstCount,
        TGF_DSA_CMD_FROM_CPU_E,
        1 /*vlanId*/,
        GT_TRUE /*targetIsDevice*/,
        egressDevHwId /*devHwIdOrVidx*/,
        packetTc /*tc*/,
        packetDp /*dp*/,
        GT_FALSE /*sourcePktQosIsSame*/);

    /* AUTODOC: Send To Analyser packet and Check counters */
    PRV_UTF_LOG0_MAC("Send To Analyser packet\n");
    prvTgfLion2FabricSendAndCheckPacket(
        portsNum, portsArr, sendPortIndex, burstCount,
        TGF_DSA_CMD_TO_ANALYZER_E,
        1 /*vlanId*/,
        GT_TRUE /*targetIsDevice*/,
        egressDevHwId /*devHwIdOrVidx*/,
        packetTc /*tc*/,
        packetDp /*dp*/,
        GT_FALSE /*sourcePktQosIsSame*/);

    /* AUTODOC: Clean Up Lion2 Fabric */
    rc = prvTgfLion2FabricCleanUp(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfLion2FabricCleanUp");
}

/**
* @internal tgfLion2FabricToVidx function
* @endinternal
*
* @brief   Set test configuration:
*/
GT_VOID tgfLion2FabricToVidx
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32                                sendPortIndex = 1;
    GT_U32                                cpuPortIndex = 3;
    GT_U32                                egressDevPortIndex0 = 0;
    GT_U32                                egressDevPortIndex1 = 2;
    PRV_TGF_LION2_FABRIC_PORT_USE_STC     portsArr[] =
    {
        {0, GT_FALSE, GT_TRUE},
        {1, GT_TRUE, GT_TRUE},
        {2, GT_FALSE, GT_TRUE},
        {3, GT_FALSE, GT_FALSE}
    };
    GT_U32                                portsNum =
        (sizeof(portsArr) / sizeof(portsArr[0]));
    GT_U32                                egressVidx = 0x123;
    GT_U32                                burstCount = 1;
    CPSS_PORTS_BMP_STC                    targetPortsBmp;
    GT_U8                                 packetTc = 6;
    CPSS_DP_LEVEL_ENT                     packetDp = CPSS_DP_RED_E;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= prvTgfLion2FabricToVidx =======\n");

    /* AUTODOC: init Lion2 Fabric */
    rc = prvTgfLion2FabricInit(
        prvTgfDevNum, prvTgfPortsArray[cpuPortIndex]);
    if (rc != GT_OK)
    {
        prvTgfLion2FabricCleanUp(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfLion2FabricInit");
        return;
    }

    /* AUTODOC: Lion2 Fabric Map dummy device to ingress port */
    rc = prvTgfLion2FabricForwardAddBc2Device(
        prvTgfDevNum, 0xFF /*hwDevNum*/, prvTgfPortsArray[sendPortIndex]);
    if (rc != GT_OK)
    {
        prvTgfLion2FabricCleanUp(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfLion2FabricForwardAddBc2Device");
        return;
    }

    /* AUTODOC: Lion2 Fabric: VIDX  */
    cpssOsMemSet(&targetPortsBmp, 0, sizeof(targetPortsBmp));
    CPSS_PORTS_BMP_PORT_SET_MAC((&targetPortsBmp), prvTgfPortsArray[egressDevPortIndex0]);
    CPSS_PORTS_BMP_PORT_SET_MAC((&targetPortsBmp), prvTgfPortsArray[egressDevPortIndex1]);

    rc = prvTgfLion2FabricForwardAddBc2Vidx(
        prvTgfDevNum, egressVidx /*bc2Vidx*/, &targetPortsBmp);
    if (rc != GT_OK)
    {
        prvTgfLion2FabricCleanUp(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfLion2FabricForwardAddBc2Vidx");
        return;
    }

    /* AUTODOC: Lion2 Fabric Qos configuration */
    rc = prvTgfLion2FabricConfigureQos(
        prvTgfDevNum,
        packetTc/*toCpuTc*/,
        packetDp/*toCpuDp*/,
        packetTc/*toAnalyzerTc*/,
        packetDp/*toAnalyzerDp*/,
        packetDp/*fromCpuDp*/);
    if (rc != GT_OK)
    {
        prvTgfLion2FabricCleanUp(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfLion2FabricConfigureQos");
        return;
    }

    /* AUTODOC: Send forwarded packet and Check counters */
    PRV_UTF_LOG0_MAC("Send Forwarded packet\n");
    prvTgfLion2FabricSendAndCheckPacket(
        portsNum, portsArr, sendPortIndex, burstCount,
        TGF_DSA_CMD_FORWARD_E,
        1 /*vlanId*/,
        GT_FALSE /*targetIsDevice*/,
        egressVidx /*devHwIdOrVidx*/,
        packetTc /*tc*/,
        packetDp /*dp*/,
        GT_FALSE /*sourcePktQosIsSame*/);

    /* AUTODOC: Send From Cpu packet and Check counters */
    PRV_UTF_LOG0_MAC("Send From CPU packet\n");
    prvTgfLion2FabricSendAndCheckPacket(
        portsNum, portsArr, sendPortIndex, burstCount,
        TGF_DSA_CMD_FROM_CPU_E,
        1 /*vlanId*/,
        GT_FALSE /*targetIsDevice*/,
        egressVidx /*devHwIdOrVidx*/,
        packetTc /*tc*/,
        packetDp /*dp*/,
        GT_FALSE /*sourcePktQosIsSame*/);

    /* AUTODOC: Send To Analyser packet and Check counters */
    PRV_UTF_LOG0_MAC("Send To Analyser packet\n");
    prvTgfLion2FabricSendAndCheckPacket(
        portsNum, portsArr, sendPortIndex, burstCount,
        TGF_DSA_CMD_TO_ANALYZER_E,
        1 /*vlanId*/,
        GT_FALSE /*targetIsDevice*/,
        egressVidx /*devHwIdOrVidx*/,
        packetTc /*tc*/,
        packetDp /*dp*/,
        GT_FALSE /*sourcePktQosIsSame*/);

    /* AUTODOC: Clean Up Lion2 Fabric */
    rc = prvTgfLion2FabricCleanUp(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfLion2FabricCleanUp");
}

/**
* @internal tgfLion2FabricToVlan function
* @endinternal
*
* @brief   Set test configuration:
*/
GT_VOID tgfLion2FabricToVlan
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U16                                packetVlanId = 0x345;
    GT_U32                                sendPortIndex = 1;
    GT_U32                                cpuPortIndex = 3;
    GT_U32                                egressDevPortIndex0 = 0;
    GT_U32                                egressDevPortIndex1 = 2;
    PRV_TGF_LION2_FABRIC_PORT_USE_STC     portsArr[] =
    {
        {0, GT_FALSE, GT_TRUE},
        {1, GT_TRUE, GT_TRUE},
        {2, GT_FALSE, GT_TRUE},
        {3, GT_FALSE, GT_FALSE}
    };
    GT_U32                                portsNum =
        (sizeof(portsArr) / sizeof(portsArr[0]));
    GT_U32                                burstCount = 1;
    CPSS_PORTS_BMP_STC                    targetPortsBmp;
    GT_U8                                 packetTc = 4;
    CPSS_DP_LEVEL_ENT                     packetDp = CPSS_DP_GREEN_E;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= prvTgfLion2FabricToVlan =======\n");

    /* AUTODOC: init Lion2 Fabric */
    rc = prvTgfLion2FabricInit(
        prvTgfDevNum, prvTgfPortsArray[cpuPortIndex]);
    if (rc != GT_OK)
    {
        prvTgfLion2FabricCleanUp(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfLion2FabricInit");
        return;
    }

    /* AUTODOC: Lion2 Fabric Map dummy device to ingress port */
    rc = prvTgfLion2FabricForwardAddBc2Device(
        prvTgfDevNum, 0xFF /*hwDevNum*/, prvTgfPortsArray[sendPortIndex]);
    if (rc != GT_OK)
    {
        prvTgfLion2FabricCleanUp(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfLion2FabricForwardAddBc2Device");
        return;
    }

    /* AUTODOC: Lion2 Fabric: VLAN  */
    cpssOsMemSet(&targetPortsBmp, 0, sizeof(targetPortsBmp));
    CPSS_PORTS_BMP_PORT_SET_MAC((&targetPortsBmp), prvTgfPortsArray[egressDevPortIndex0]);
    CPSS_PORTS_BMP_PORT_SET_MAC((&targetPortsBmp), prvTgfPortsArray[egressDevPortIndex1]);

    rc = prvTgfLion2FabricForwardAddBc2Vlan(
        prvTgfDevNum, packetVlanId /*bc2VlanId*/, &targetPortsBmp);
    if (rc != GT_OK)
    {
        prvTgfLion2FabricCleanUp(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfLion2FabricForwardAddBc2Vidx");
        return;
    }

    /* AUTODOC: Lion2 Fabric Qos configuration */
    rc = prvTgfLion2FabricConfigureQos(
        prvTgfDevNum,
        packetTc/*toCpuTc*/,
        packetDp/*toCpuDp*/,
        packetTc/*toAnalyzerTc*/,
        packetDp/*toAnalyzerDp*/,
        packetDp/*fromCpuDp*/);
    if (rc != GT_OK)
    {
        prvTgfLion2FabricCleanUp(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfLion2FabricConfigureQos");
        return;
    }

    /* AUTODOC: Send forwarded packet and Check counters */
    PRV_UTF_LOG0_MAC("Send Forwarded packet\n");
    prvTgfLion2FabricSendAndCheckPacket(
        portsNum, portsArr, sendPortIndex, burstCount,
        TGF_DSA_CMD_FORWARD_E,
        packetVlanId /*vlanId*/,
        GT_FALSE /*targetIsDevice*/,
        PRV_TGF_LION2_FABRIC_VIDX_FOR_VLAN_ID_CNS /*devHwIdOrVidx*/,
        packetTc /*tc*/,
        packetDp /*dp*/,
        GT_FALSE /*sourcePktQosIsSame*/);

    /* AUTODOC: Send From Cpu packet and Check counters */
    PRV_UTF_LOG0_MAC("Send From CPU packet\n");
    prvTgfLion2FabricSendAndCheckPacket(
        portsNum, portsArr, sendPortIndex, burstCount,
        TGF_DSA_CMD_FROM_CPU_E,
        packetVlanId /*vlanId*/,
        GT_FALSE /*targetIsDevice*/,
        PRV_TGF_LION2_FABRIC_VIDX_FOR_VLAN_ID_CNS /*devHwIdOrVidx*/,
        packetTc /*tc*/,
        packetDp /*dp*/,
        GT_FALSE /*sourcePktQosIsSame*/);

    /* AUTODOC: Clean Up Lion2 Fabric */
    rc = prvTgfLion2FabricCleanUp(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfLion2FabricCleanUp");
}

/**
* @internal tgfLion2FabricToCpu function
* @endinternal
*
* @brief   Set test configuration:
*/
GT_VOID tgfLion2FabricToCpu
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32                                sendPortIndex = 1;
    GT_U32                                cpuPortIndex = 3;
    PRV_TGF_LION2_FABRIC_PORT_USE_STC     portsArr[] =
    {
        {0, GT_FALSE, GT_FALSE},
        {1, GT_TRUE, GT_TRUE},
        {2, GT_FALSE, GT_FALSE},
        {3, GT_FALSE, GT_TRUE}
    };
    GT_U32                                portsNum =
        (sizeof(portsArr) / sizeof(portsArr[0]));
    GT_U32                                burstCount = 1;
    CPSS_PORTS_BMP_STC                    targetPortsBmp;
    GT_U8                                 packetTc = 3;
    CPSS_DP_LEVEL_ENT                     packetDp = CPSS_DP_YELLOW_E;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= prvTgfLion2FabricToDev =======\n");

    /* AUTODOC: init Lion2 Fabric */
    rc = prvTgfLion2FabricInit(
        prvTgfDevNum, prvTgfPortsArray[cpuPortIndex]);
    if (rc != GT_OK)
    {
        prvTgfLion2FabricCleanUp(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfLion2FabricInit");
        return;
    }

    /* AUTODOC: Lion2 Fabric: VIDX with ingress port */
    cpssOsMemSet(&targetPortsBmp, 0, sizeof(targetPortsBmp));
    CPSS_PORTS_BMP_PORT_SET_MAC((&targetPortsBmp), prvTgfPortsArray[sendPortIndex]);

    rc = prvTgfLion2FabricForwardAddBc2Vidx(
        prvTgfDevNum, 123 /*bc2Vidx*/, &targetPortsBmp);
    if (rc != GT_OK)
    {
        prvTgfLion2FabricCleanUp(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfLion2FabricForwardAddBc2Vidx");
        return;
    }

    /* AUTODOC: Lion2 Fabric Qos configuration */
    rc = prvTgfLion2FabricConfigureQos(
        prvTgfDevNum,
        packetTc/*toCpuTc*/,
        packetDp/*toCpuDp*/,
        packetTc/*toAnalyzerTc*/,
        packetDp/*toAnalyzerDp*/,
        packetDp/*fromCpuDp*/);
    if (rc != GT_OK)
    {
        prvTgfLion2FabricCleanUp(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfLion2FabricConfigureQos");
        return;
    }

    /* AUTODOC: Send To Cpu packet and Check counters */
    PRV_UTF_LOG0_MAC("Send To Cpu packet\n");
    prvTgfLion2FabricSendAndCheckPacket(
        portsNum, portsArr, sendPortIndex, burstCount,
        TGF_DSA_CMD_TO_CPU_E,
        1 /*vlanId*/,
        GT_TRUE /*targetIsDevice*/,
        0 /*devHwIdOrVidx*/,
        packetTc /*tc*/,
        packetDp /*dp*/,
        GT_FALSE /*sourcePktQosIsSame*/);

    /* AUTODOC: Clean Up Lion2 Fabric */
    rc = prvTgfLion2FabricCleanUp(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfLion2FabricCleanUp");
}




