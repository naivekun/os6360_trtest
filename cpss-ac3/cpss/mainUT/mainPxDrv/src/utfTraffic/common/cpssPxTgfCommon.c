/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file cpssPxTgfCommon.c
*
* @brief Common traffic helper API for enhanced UTs
*
* @version   1
********************************************************************************
*/

/*
    The define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files
    that already fixed the types of ports from GT_U8!

    NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <utfTraffic/common/cpssPxTgfCommon.h>
#include <extUtils/trafficEngine/prvTgfLog.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/config/private/prvCpssPxInfoEnhanced.h>

#include <cpss/px/port/cpssPxPortStat.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/ingress/cpssPxIngress.h>


/* macro to verify return code */
#define PRV_TGF_VERIFY_MAC(rc, msg)                                     \
    if (GT_OK != rc)                                                    \
    {                                                                   \
        PRV_UTF_LOG2_MAC("[TGF]: %s FAILED, rc = [%d]\n", msg, rc);     \
        return rc;                                                      \
    }


/* Enable/disable printing of transmitted frames info to log */
static GT_BOOL cpssPxTgfCommonFrameDumpEnable = GT_TRUE;

GT_PHYSICAL_PORT_NUM prvTgfPxPortsArray[PRV_CPSS_PX_PORTS_NUM_CNS] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
};


/**
* @internal prvCheckPortNum function
* @endinternal
*
* @brief   Check port number
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - the port number
*/
static GT_STATUS prvCheckPortNum
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_U32  portMacNum; /* port MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    return GT_OK;
}

/**
* @internal prvDumpFrame function
* @endinternal
*
* @brief   Print frame to log.
*
* @param[in] frame                    -  data
* @param[in] frameLength              - frame length
*/
static GT_VOID prvDumpFrame
(
    IN  GT_U8   *frame,
    IN  GT_U32  frameLength
)
{
    GT_U32  i;

    if (cpssPxTgfCommonFrameDumpEnable == GT_FALSE)
    {
        return;
    }

    PRV_TGF_LOG1_MAC("\n   Frame (%d bytes):", frameLength);

    if (frameLength > 256)
    {
        PRV_TGF_LOG1_MAC("\n   print only first 256 bytes (instead of [%d])", frameLength);
        frameLength = 256;
    }

    for (i = 0; i < frameLength; i++)
    {
        if ((i & 0x0F) == 0)
        {
            PRV_TGF_LOG1_MAC("\n   0x%04X : ", i);
        }

        PRV_TGF_LOG1_MAC(" %02X", frame[i]);
    }

    PRV_TGF_LOG0_MAC("\n\n");
}

/**
* @internal cpssPxTgfEthTransmitPacketPrintInfoEnableSet function
* @endinternal
*
* @brief   Enable/disable transmitted frames info printing to log.
*
* @param[in] enable                   - enable/disable frames info printing
*
* @retval GT_OK                    - on success.
*/
GT_STATUS cpssPxTgfEthTransmitPacketPrintInfoEnableSet
(
    IN  GT_BOOL     enable
)
{
    cpssPxTgfCommonFrameDumpEnable = enable;

    return GT_OK;
}

/**
* @internal cpssPxTgfEthSimpleFrameWithRandomPayloadCreate function
* @endinternal
*
* @brief   Incapsulate frame fields to array of bytes. Payload generated
*         automatically.
* @param[in] macDa                    - destination MAC address. If NULL - will set
*                                      FF:FF:FF:FF:FF:FF.
* @param[in] macSa                    - source MAC address. If NULL - will set
*                                      FF:FF:FF:FF:FF:FF.
* @param[in] isVlanNeeded             - is VLAN Tag needed:
*                                      GT_TRUE - VLAN tag will be incapsulated,
*                                      GT_FALSE - VALN tag will be skipped.
* @param[in] vlanId                   - VLAN ID (used when isVlanNeeded == GT_TRUE).
*                                      (APPLICABLE RANGES: 0..4095).
* @param[in] vlanCfi                  - VLAN CFI (used when isVlanNeeded == GT_TRUE).
*                                      (APPLICABLE RANGES: 0..1).
* @param[in] vlanTc                   - VLAN Traffic Class (used when isVlanNeeded == GT_TRUE).
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] etherType                - EtherType field.
*                                      (APPLICABLE RANGES: 0..0xFFFF).
* @param[in] payloadLength            - frame payload length.
*
* @param[out] framePtr                 - (pointer to) the frame data
* @param[out] frameLength              - frame length
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssPxTgfEthSimpleFrameWithRandomPayloadCreate
(
   IN  GT_U8            *macDa,
   IN  GT_U8            *macSa,
   IN  GT_BOOL          isVlanNeeded,
   IN  GT_U32           vlanId,
   IN  GT_U32           vlanCfi,
   IN  GT_U32           vlanTc,
   IN  GT_U32           etherType,
   IN  GT_U32           payloadLength,
   OUT GT_U8            *framePtr,
   OUT GT_U32           *frameLength
)
{
    GT_U32  i;
    GT_U32  j;

    CPSS_NULL_PTR_CHECK_MAC(framePtr);
    CPSS_NULL_PTR_CHECK_MAC(frameLength);

    if (((isVlanNeeded) && ((vlanId > 0x0FFF) || (vlanTc > 0x07) || (vlanCfi > 0x01))) ||
        (etherType > 0xFFFF))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    i = 0;

    /* MAC DA */
    for (j = 0; j < 6; j++)
    {
        framePtr[i++] = (macDa == NULL) ? 0xFF : macDa[j];
    }

    /* MAC SA */
    for (j = 0; j < 6; j++)
    {
        framePtr[i++] = (macSa == NULL) ? 0xFF : macSa[j];
    }

    /* VLAN */
    if (isVlanNeeded)
    {
        /* VLAN Tag protocol ID */
        framePtr[i++] = 0x81;
        framePtr[i++] = 0x00;

        /* VLAN Tag */
        framePtr[i++] = (vlanTc << 5) | (vlanCfi << 4) | (vlanId >> 8);
        framePtr[i++] = vlanId & 0xFF;
    }

    /* EtherType */
    framePtr[i++] = etherType >> 8;
    framePtr[i++] = etherType & 0xFF;

    /* payload */
    for (j = 0; j < payloadLength; j++)
    {
        framePtr[i++] = j & 0xFF;
    }

    *frameLength = i;

    return GT_OK;
}

/**
* @internal cpssPxTgfEthTransmitPackets function
* @endinternal
*
* @brief   Transmit packet to ingress port several (burstCount) times.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - the ingress port num
* @param[in] framePtr                 - (pointer to) the frame bytes
* @param[in] frameLength              - frame length
*                                      (APPLICABLE RANGES: 0x39..0x3FFC)
* @param[in] burstCount               - count of frame to send.
*                                      (APPLICABLE RANGES: 1..0xFFFF)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssPxTgfEthTransmitPackets
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   *framePtr,
    IN  GT_U32                  frameLength,
    IN  GT_U32                  burstCount
)
{
    GT_STATUS   rc;
    GT_U32      portMacNum;
    CPSS_INTERFACE_INFO_STC  ingressPortInterface;
    TGF_PACKET_STC           ingressPacketInfo;
    TGF_PACKET_PART_STC      ingressFullPart;
    TGF_PACKET_PAYLOAD_STC   ingressPayloadPart;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(framePtr);

    if ((burstCount < 0) || (burstCount > 0xFFFF) ||
        (frameLength < 0x39) || (frameLength > 0x3FFC))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_TGF_LOG2_MAC("== Try to transmit %d frames to port %d\n", burstCount, portNum);

    prvDumpFrame(framePtr, frameLength);

    ingressPortInterface.type = CPSS_INTERFACE_PORT_E;
    ingressPortInterface.devPort.portNum = portNum;
    ingressPortInterface.devPort.hwDevNum = devNum;

    ingressPacketInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    ingressPacketInfo.numOfParts = 1;
    ingressPacketInfo.partsArray = &ingressFullPart;

    ingressFullPart.type    = TGF_PACKET_PART_PAYLOAD_E;
    ingressFullPart.partPtr = &ingressPayloadPart;

    ingressPayloadPart.dataLength = frameLength;
    ingressPayloadPart.dataPtr    = framePtr;


    (GT_VOID) tgfTrafficTracePacketByteSet(GT_TRUE);

    rc = tgfTrafficGeneratorPortTxEthTransmit(&ingressPortInterface,
            &ingressPacketInfo, burstCount, 0, NULL, 0, 0, 0, GT_TRUE);
    PRV_TGF_VERIFY_MAC(rc, "tgfTrafficGeneratorPortTxEthTransmit");

    return GT_OK;
}

/**
* @internal cpssPxTgfEthCountersReset function
* @endinternal
*
* @brief   Reset the traffic generator counters on the port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssPxTgfEthCountersReset
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS   rc;
    GT_U32      portMacNum;
    CPSS_PX_PORT_MAC_COUNTERS_STC   dummyPxCounters;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    PRV_TGF_LOG1_MAC("== Reset MAC counters for %d port\n", portNum);

    rc = cpssPxPortMacCountersOnPortGet(devNum, portNum, &dummyPxCounters);
    PRV_TGF_VERIFY_MAC(rc, "cpssPxPortMacCountersOnPortGet");

    rc = prvTgfEgressPortForceLinkPassEnable(devNum, portNum, GT_TRUE, GT_TRUE);
    PRV_TGF_VERIFY_MAC(rc, "cpssPxPortForceLinkPassEnableSet");

    return GT_OK;
}

/**
* @internal cpssPxTgfEthCountersResetAll function
* @endinternal
*
* @brief   Reset the traffic generator counters for all ports
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssPxTgfEthCountersResetAll
(
    IN  GT_SW_DEV_NUM   devNum
)
{
    GT_STATUS   rc;
    GT_U32      i;
    CPSS_PX_PORT_MAC_COUNTERS_STC   dummyPxCounters;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    PRV_TGF_LOG0_MAC("== Reset MAC counters for ALL ports\n");

    /* go over all ports and read counters to reset them */
    for (i = 0; i < PRV_CPSS_PX_PORTS_NUM_CNS; i++)
    {
        if (prvCheckPortNum(devNum, i) != GT_OK)
        {
            continue;
        }

        rc = cpssPxPortMacCountersOnPortGet(devNum, i, &dummyPxCounters);
        if (rc != GT_OK)
        {
            PRV_TGF_LOG2_MAC("[TGF]: cpssPxPortMacCountersOnPortGet"
                             " FAILED for %d port, rc = [%d]\n", i, rc);
        }
    }

    /* force link UP of all test ports */
    for (i = 0; i < (sizeof(prvTgfPxPortsArray) / sizeof(prvTgfPxPortsArray[0])); i++)
    {
        rc = prvTgfEgressPortForceLinkPassEnable(devNum,
                prvTgfPxPortsArray[i], GT_TRUE, GT_TRUE);
        PRV_TGF_VERIFY_MAC(rc, "cpssPxPortForceLinkPassEnableSet");
    }

    return GT_OK;
}

/**
* @internal cpssPxTgfEthCountersRead function
* @endinternal
*
* @brief   Read the traffic generator counters on the port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] countersPtr              - (pointer to) the port counters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssPxTgfEthCountersRead
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT CPSS_PX_TGF_PORT_MAC_COUNTERS_STC   *countersPtr
)
{
    GT_STATUS   rc;
    GT_U32      portMacNum;
    CPSS_PX_PORT_MAC_COUNTERS_STC   cntrs;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(countersPtr);

    PRV_TGF_LOG1_MAC("== Read MAC counters for %d port\n", portNum);

    rc = cpssPxPortMacCountersOnPortGet(devNum, portNum, &cntrs);
    PRV_TGF_VERIFY_MAC(rc, "cpssPxPortMacCountersOnPortGet");

    cpssOsMemSet(countersPtr, 0, sizeof(CPSS_PX_TGF_PORT_MAC_COUNTERS_STC));

    countersPtr->goodPktsRcv =
            cntrs.mibCounter[CPSS_PX_PORT_MAC_COUNTER_GOOD_UNICAST_FRAMES_RECEIVED_E].l[0] +
            cntrs.mibCounter[CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_RECEIVED_E].l[0] +
            cntrs.mibCounter[CPSS_PX_PORT_MAC_COUNTER_BROADCAST_FRAMES_RECEIVED_E].l[0];

    countersPtr->goodPktsSent =
            cntrs.mibCounter[CPSS_PX_PORT_MAC_COUNTER_UNICAST_FRAME_SENT_E].l[0] +
            cntrs.mibCounter[CPSS_PX_PORT_MAC_COUNTER_BROADCAST_FRAMES_SENT_E].l[0] +
            cntrs.mibCounter[CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_SENT_E].l[0];

    return GT_OK;
}


/**
* @internal prvCpssPxTgfEthCountersReadAllAndDump function
* @endinternal
*
* @brief   Read and dump MAC counters for all ports.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
*
* @note Please use it just during development of enhanced test.
*
*/
GT_STATUS prvCpssPxTgfEthCountersReadAllAndDump
(
    IN  GT_SW_DEV_NUM   devNum
)
{
    GT_STATUS   rc;
    GT_U32      goodSent;
    GT_U32      goodRcv;
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_PX_PORT_MAC_COUNTERS_STC   cntrs;
    GT_BOOL     isPortUpped;


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    PRV_TGF_LOG0_MAC("== Dump of all MAC counters\n"
                     "+------+-----------+-----------+------+\n"
                     "| port | Tx frames | Rx frames | link |\n"
                     "+------+-----------+-----------+------+\n");

    /* go over all ports */
    for (portNum = 0; portNum < PRV_CPSS_PX_PORTS_NUM_CNS; portNum++)
    {
        rc = cpssPxPortLinkStatusGet(devNum, portNum, &isPortUpped);
        if (rc != GT_OK)
        {
            continue;
        }

        rc = cpssPxPortMacCountersOnPortGet(devNum, portNum, &cntrs);
        if (rc != GT_OK)
        {
            continue;
        }

        goodRcv =
                cntrs.mibCounter[CPSS_PX_PORT_MAC_COUNTER_GOOD_UNICAST_FRAMES_RECEIVED_E].l[0] +
                cntrs.mibCounter[CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_RECEIVED_E].l[0] +
                cntrs.mibCounter[CPSS_PX_PORT_MAC_COUNTER_BROADCAST_FRAMES_RECEIVED_E].l[0];

        goodSent =
                cntrs.mibCounter[CPSS_PX_PORT_MAC_COUNTER_UNICAST_FRAME_SENT_E].l[0] +
                cntrs.mibCounter[CPSS_PX_PORT_MAC_COUNTER_BROADCAST_FRAMES_SENT_E].l[0] +
                cntrs.mibCounter[CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_SENT_E].l[0];

        PRV_TGF_LOG4_MAC("|  %2d  | %9d | %9d | %s |\n",
                portNum, goodSent, goodRcv,
                (isPortUpped) ? " up " : "down");
    }

    PRV_TGF_LOG0_MAC("+------+-----------+-----------+------+\n");

    return GT_OK;
}

