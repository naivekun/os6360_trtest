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
* @file cpssPxHalBpeEnhancedUT.c
*
* @brief Enhanced unit tests for cpssPxHalBpe that provides
* CPSS implementation for Pipe 802.1BR processing.
*
* @version   1
********************************************************************************
*/
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include "../../h/cpssPxHalBpe.h"
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/cpssPxPortStat.h>
#include <cpss/px/networkIf/cpssPxNetIf.h>

#define SLEEP_AFTER_PACKET_SEND     100
#define EXT_PORTS_IN_MC_GRP         4
#define BYTES_IN_E_TAG              8
#define BYTES_IN_C_TAG              4
#define BYTES_IN_CRC                4
#define BYTES_IN_MAC_ADDR           6
#define BYTES_IN_IPV6_HEADER        40

#define BUFF_LEN                                     20
#define PKT_BUFF_LEN                                 500
#define HAL_BPE_PKT_TYPE_CPU_TO_UPSTREAM_E           3
#define HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_UC_E 0
#define HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_MC_E 1
#define HAL_BPE_UPSTREAM_TRAFFIC_INDEX_CNS           0
#define HAL_BPE_NUM_RESERVED_ENTRIES_CNS             19

#ifdef  WIN32
#define __FUNCNAME__ __FUNCTION__
#else
#define __FUNCNAME__ __func__
#endif /* WIN32 */

extern GT_BOOL prvHalBpeDebugErrors;

/* macro to print error message */
#define BPE_LOG_ERROR_AND_RETURN_MAC(_rc, ...)                                                                                \
    do {                                                                                                                      \
        if (prvHalBpeDebugErrors == GT_TRUE) {                                                                                \
            cpssOsPrintSync("PxHalBpe enhancedUT ERROR func[%s] on line[%d] returned rc = %d: ", __FUNCNAME__, __LINE__, _rc);\
            cpssOsPrintSync(__VA_ARGS__);                                                                                     \
            cpssOsPrintSync("\n\n");                                                                                          \
            return _rc;                                                                                                       \
        }                                                                                                                     \
    } while (0)

#define BPE_LOG_ERROR_MAC(...)                                                                          \
    do {                                                                                                \
        if (prvHalBpeDebugErrors == GT_TRUE) {                                                          \
            cpssOsPrintSync("PxHalBpe enhancedUT ERROR func[%s] on line[%d]: ", __FUNCNAME__, __LINE__);\
            cpssOsPrintSync(__VA_ARGS__);                                                               \
            cpssOsPrintSync("\n\n");                                                                    \
        }                                                                                               \
    } while (0)

/* macro to log function information by formatted string */
#define BPE_LOG_INFORMATION_MAC(...)                                                                   \
    do {                                                                                               \
        if (prvHalBpeDebugErrors == GT_TRUE) {                                                         \
            cpssOsPrintSync("PxHalBpe enhancedUT INFO func[%s] on line[%d]: ", __FUNCNAME__, __LINE__);\
            cpssOsPrintSync(__VA_ARGS__);                                                              \
            cpssOsPrintSync("\n\n");                                                                   \
        }                                                                                              \
    } while (0)

/* macro to set a port in the bmp of ports */
#define PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmp,portNum)    \
    portsBmp |= 1 << ((portNum)& 0x1ffff)

/* macro to check if port is set in the bmp of ports (is corresponding bit is 1) */
#define PRV_CPSS_PX_HAL_PORTS_BMP_IS_PORT_SET_MAC(portsBmp,portNum)   \
    ((portsBmp & (1 << ((portNum)& 0x1ffff))) ? 1 : 0)

/* macro to clear a port in the bmp of ports */
#define PRV_CPSS_PX_HAL_PORTS_BMP_PORT_CLEAR_MAC(portsBmp,portNum)   \
    portsBmp &= ~(1 << ((portNum)& 0x1ffff))

static GT_U8 transmittedPkt[PKT_BUFF_LEN];
static GT_U8 expectedPkt[PKT_BUFF_LEN];

static GT_BOOL ignoreErrorOnPacketLength = GT_FALSE;

static GT_BOOL prvTgfPxHalBpeMaskRxEvents(GT_BOOL mask_or_unmask)
{
    GT_STATUS   rc;
    GT_U32      ii;

    /* Currently due to appDemo which handels the Rx events */
    CPSS_UNI_EV_CAUSE_ENT   evRxHndlrCauseArr[] =
    {
         CPSS_PP_RX_BUFFER_QUEUE0_E, \
         CPSS_PP_RX_BUFFER_QUEUE1_E, \
         CPSS_PP_RX_BUFFER_QUEUE2_E, \
         CPSS_PP_RX_BUFFER_QUEUE3_E, \
         CPSS_PP_RX_BUFFER_QUEUE4_E, \
         CPSS_PP_RX_BUFFER_QUEUE5_E, \
         CPSS_PP_RX_BUFFER_QUEUE6_E, \
         CPSS_PP_RX_BUFFER_QUEUE7_E
    };

    GT_U32 evRxHndlrCauseArrSize =
                        sizeof(evRxHndlrCauseArr)/sizeof(evRxHndlrCauseArr[0]);

    CPSS_EVENT_MASK_SET_ENT event_mask = (GT_TRUE == mask_or_unmask) ?
                                         CPSS_EVENT_MASK_E:CPSS_EVENT_UNMASK_E;

    for (ii=0;ii<evRxHndlrCauseArrSize;ii++) {
        rc = cpssEventDeviceMaskSet(0,
                                    evRxHndlrCauseArr[ii],
                                    event_mask);
        if(rc != GT_OK) {

            return (GT_FALSE);
        }
    }

    return (GT_TRUE);
}

/**
* @internal prvTgfPxHalBpeTracePacket function
* @endinternal
*
* @brief   Trace the packet bytes
*
* @param[in] bufferPtr                - (pointer to) the buffer that hold the packet bytes
* @param[in] length                   -  of packet
* @param[in] isCrcIncluded            - enable\disable printout CRC in log
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfPxHalBpeTracePacket
(
    IN GT_U8           *bufferPtr,
    IN GT_U32           length,
    IN GT_BOOL          isCrcIncluded
)
{
    GT_U32  iter = 0;

    CPSS_NULL_PTR_CHECK_MAC(bufferPtr);

    if (GT_TRUE == isCrcIncluded)
    {
        cpssOsPrintSync("\ntracePacket: packet trace, length [%d](include CRC)\n", length);
    }
    else
    {
        cpssOsPrintSync("\ntracePacket: packet trace, length [%d](CRC not included)\n", length);
    }

    for(iter = 0; iter < length; iter++)
    {
        if((iter & 0x0F) == 0)
        {
            cpssOsPrintSync("0x%4.4x :", iter);
        }

        cpssOsPrintSync(" %2.2x", bufferPtr[iter]);

        if((iter & 0x0F) == 0x0F)
        {
            cpssOsPrintSync("\n");
        }
    }

    cpssOsPrintSync("\n\n");

    return GT_OK;
}

/**
* @internal prvTgfPxHalBpeEnhancedUtExtended2UpstreamConfigSet function
* @endinternal
*
* @brief   This function configure extra configuration on the egress attributes of the ingress port and
*         add CPU port to the destination port bitmap.
*         DETAILED DESCRIPTION:
*         There is no HA thread to handle packets from CPU to extended/cascade ingress port.
*         Inorder to send packet from CPU to extended/cascade ingress port use the HA thread CPU2U.
*         HA thread CPU2U is activating U2C thread to remove the E-tag added by the CPU (For directing
*         the packet to the upstream port).
*         1. Need to define for ingress port and packetType = CPU_TO_UPSTREAM HA thread CPU2U.
*         2. Need to change egress target attributes of ingress port as this port is in loopback mode.
*         In order to remove the E-tag: The Target Port Table[43:32] - PCID should be set to the upstream port number.
*         3. Need to send the egress packet also to CPU port inoder to trace the received packet.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] ingressPort              - ingress port. must be extended/cascade
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Limitations: Can test only packets from ingress port extended/cascade to egress port upstream
*       as we change the egress attributes of the ingress port.
*
*/

static GT_STATUS prvTgfPxHalBpeEnhancedUtExtended2UpstreamConfigSet
(
    IN  GT_SW_DEV_NUM                      devNum,
    IN  GT_PORT_NUM                        ingressPort
)
{
    GT_STATUS                                   rc;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  operationInfo;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT        destPortInfo;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT   destInfoType;
    CPSS_PX_PORTS_BMP                           portsBmp;
    GT_BOOL                                     bypassLag;

    /* 1. Update Header Alteration Table that maps <Packet Type, Egress port> tuple to the relevant firmware thread */
    cpssOsMemSet(&operationInfo,0,sizeof(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT));
    rc = cpssPxEgressHeaderAlterationEntrySet(devNum,ingressPort,HAL_BPE_PKT_TYPE_CPU_TO_UPSTREAM_E,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E,&operationInfo);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressHeaderAlterationEntrySet failed");
    }

    /* 2. Set egress target attributes for ingress port */
    rc = cpssPxEgressTargetPortEntryGet(devNum, ingressPort, &destInfoType, &destPortInfo);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressTargetPortEntryGet failed");
    }

    destPortInfo.info_common.pcid = ingressPort;
    destPortInfo.info_common.tpid = 0x893F;

    rc = cpssPxEgressTargetPortEntrySet(devNum, ingressPort, destInfoType, &destPortInfo);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressTargetPortEntrySet failed");
    }

    /* 3. Add CPU port to the destination ports bitmap */
    rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,HAL_BPE_UPSTREAM_TRAFFIC_INDEX_CNS,&portsBmp,&bypassLag);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxIngressPortMapEntryGet failed");
    }

    /* Set appropriate bit for portNum */
    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmp,PRV_CPSS_PX_PORTS_NUM_CNS-1);

    /* Configure Dst port map table entry (UPSTREAM TRAFFIC) for the appropriate bit */
    rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,HAL_BPE_UPSTREAM_TRAFFIC_INDEX_CNS,portsBmp,bypassLag);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxIngressPortMapEntrySet failed");
    }

    return GT_OK;
}

/**
* @internal prvTgfPxHalBpeEnhancedUtExtended2UpstreamConfigRestore function
* @endinternal
*
* @brief   This function remove CPU port to the destination port bitmap.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

static GT_STATUS prvTgfPxHalBpeEnhancedUtExtended2UpstreamConfigRestore
(
    IN  GT_SW_DEV_NUM                      devNum
)
{
    GT_STATUS                                   rc;
    CPSS_PX_PORTS_BMP                           portsBmp;
    GT_BOOL                                     bypassLag;

    /* 1. remove CPU port from the destination ports bitmap */
    rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,HAL_BPE_UPSTREAM_TRAFFIC_INDEX_CNS,&portsBmp,&bypassLag);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxIngressPortMapEntryGet failed");
    }

    /* Unset appropriate bit for portNum */
    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_CLEAR_MAC(portsBmp,PRV_CPSS_PX_PORTS_NUM_CNS-1);

    /* Configure Dst port map table entry (UPSTREAM TRAFFIC) for the appropriate bit */
    rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,HAL_BPE_UPSTREAM_TRAFFIC_INDEX_CNS,portsBmp,bypassLag);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxIngressPortMapEntrySet failed");
    }

    return GT_OK;
}

/**
* @internal prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigSet function
* @endinternal
*
* @brief   This function configure extra configuration on the egress attributes of CPU port and
*         add CPU port to the destination port bitmap.
*         DETAILED DESCRIPTION:
*         Inorder to trap packet from upstream port to extended port, send packet also to CPU port.
*         Change CPU port behavior to behave the same as extended port.
*         1. Need to define for CPU port and packetType = UPSTREAM_TO_DOWNSTREAM_UC HA thread U2E.
*         2. Need to change egress target attributes of CPU port to be the same as extended port.
*         In order to remove the VLAN-Tag if the TPID after the E-Tag = Target Port Table[15:0] - TPID and
*         Shared Data Memory [65535:0] - Egress VLAN tag state [VLAN-ID16 + Target Port Table[19:16] - Target Port]=0.
*         3. Need to send the egress packet also to CPU port inoder to trace the received packet.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number
* @param[in] cid                      - E-channel Id
* @param[in] vid                      - vlan Id
* @param[in] isTagged                 - is vlan tagged
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Limitations: Can test only packets from ingress port upstream to egress port extended
*       as we change the egress attributes of the CPU port.
*
*/
static GT_STATUS prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigSet
(
    IN  GT_SW_DEV_NUM                      devNum,
    IN  GT_U32                             cid,
    IN  GT_U32                             vid,
    IN  GT_BOOL                            isTagged
)
{
    GT_STATUS                                   rc;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  operationInfo;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT        destPortInfo;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT   destInfoType;
    CPSS_PX_PORTS_BMP                           portsBmp;
    GT_BOOL                                     bypassLag;

    /* 1. Update Header Alteration Table that maps <Packet Type, Egress port> tuple to the relevant firmware thread */
    cpssOsMemSet(&operationInfo,0,sizeof(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT));
    rc = cpssPxEgressHeaderAlterationEntrySet(devNum,PRV_CPSS_PX_PORTS_NUM_CNS-1,HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_UC_E,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E,&operationInfo);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressHeaderAlterationEntrySet failed");
    }

    /* 2. Set egress target attributes for ingress port */
    rc = cpssPxEgressTargetPortEntryGet(devNum, PRV_CPSS_PX_PORTS_NUM_CNS-1, &destInfoType, &destPortInfo);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressTargetPortEntryGet failed");
    }

    destPortInfo.info_common.tpid = 0x8100; 
    destPortInfo.info_common.pcid = cid;

    rc = cpssPxEgressTargetPortEntrySet(devNum, PRV_CPSS_PX_PORTS_NUM_CNS-1,destInfoType, &destPortInfo);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressTargetPortEntrySet failed");
    }

    /* Get VLAN tag state entry for target ports */
    rc = cpssPxEgressVlanTagStateEntryGet(devNum,vid,&portsBmp);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressVlanTagStateEntryGet failed");
    }

    if(isTagged == GT_FALSE)
    {
        PRV_CPSS_PX_HAL_PORTS_BMP_PORT_CLEAR_MAC(portsBmp,PRV_CPSS_PX_PORTS_NUM_CNS-1);
    }
    else
    {
        PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmp,PRV_CPSS_PX_PORTS_NUM_CNS-1);
    }
     
    /* Set VLAN tag state entry for target ports */
    rc = cpssPxEgressVlanTagStateEntrySet(devNum,vid,portsBmp);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressVlanTagStateEntrySet failed");
    } 

    /* 3. Add CPU port to the destination ports bitmap */
    rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,cid+HAL_BPE_NUM_RESERVED_ENTRIES_CNS,&portsBmp,&bypassLag);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxIngressPortMapEntryGet failed");
    }

    /* Set appropriate bit for portNum */
    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmp,PRV_CPSS_PX_PORTS_NUM_CNS-1);

    /* Configure Dst port map table entry for the appropriate bit */
    rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,cid+HAL_BPE_NUM_RESERVED_ENTRIES_CNS,portsBmp,bypassLag);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxIngressPortMapEntrySet failed");
    }

    return GT_OK;
}

/**
* @internal prvTgfPxHalBpeEnhancedUtUpstream2CascadeUcConfigSet function
* @endinternal
*
* @brief   This function configure extra configuration on the egress attributes of CPU port and
*         add CPU port to the destination port bitmap.
*         DETAILED DESCRIPTION:
*         Inorder to trap packet from upstream port to cascade port, send packet also to CPU port.
*         Change CPU port behavior to behave the same as cascade port.
*         1. Need to define for CPU port and packetType = UPSTREAM_TO_DOWNSTREAM_UC HA thread U2C.
*         2. Need to change egress target attributes of CPU port to be the same as cascade port.
*         In order remove the E-Tag : Packet's E-Tag<E-CID> = Target Port Table[43:32] - PCID
*         3. Need to send the egress packet also to CPU port inoder to trace the received packet.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number
* @param[in] ecid                     - Transmitted Packet's E-Tag<E-CID>
* @param[in] pcid                     - port channel Id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Limitations: Can test only packets from ingress port upstream to egress port cascade
*       as we change the egress attributes of the CPU port.
*
*/
static GT_STATUS prvTgfPxHalBpeEnhancedUtUpstream2CascadeUcConfigSet
(
    IN  GT_SW_DEV_NUM                      devNum,
    IN  GT_U32                             ecid,
    IN  GT_U32                             pcid
)
{
    GT_STATUS                                   rc;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  operationInfo;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT        destPortInfo;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT   destInfoType;
    CPSS_PX_PORTS_BMP                           portsBmp;
    GT_BOOL                                     bypassLag;

    /* 1. Update Header Alteration Table that maps <Packet Type, Egress port> tuple to the relevant firmware thread */
    cpssOsMemSet(&operationInfo,0,sizeof(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT));
    rc = cpssPxEgressHeaderAlterationEntrySet(devNum,PRV_CPSS_PX_PORTS_NUM_CNS-1,HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_UC_E,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E,&operationInfo);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressHeaderAlterationEntrySet failed");
    }

    /* 2. Set egress target attributes for ingress port */
    rc = cpssPxEgressTargetPortEntryGet(devNum, PRV_CPSS_PX_PORTS_NUM_CNS-1, &destInfoType, &destPortInfo);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressTargetPortEntryGet failed");
    }

    destPortInfo.info_common.tpid = 0x8100;
    destPortInfo.info_common.pcid = pcid;

    rc = cpssPxEgressTargetPortEntrySet(devNum, PRV_CPSS_PX_PORTS_NUM_CNS-1,destInfoType, &destPortInfo);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressTargetPortEntrySet failed");
    }

    /* 3. Add CPU port to the destination ports bitmap */
    rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,ecid+HAL_BPE_NUM_RESERVED_ENTRIES_CNS,&portsBmp,&bypassLag);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxIngressPortMapEntryGet failed");
    }

    /* Set appropriate bit for portNum */
    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmp,PRV_CPSS_PX_PORTS_NUM_CNS-1);

    /* Configure Dst port map table entry for the appropriate bit */
    rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,ecid+HAL_BPE_NUM_RESERVED_ENTRIES_CNS,portsBmp,bypassLag);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxIngressPortMapEntrySet failed");
    }

    return GT_OK;
}

/**
* @internal prvTgfPxHalBpeEnhancedUtUpstream2ExtendedMcConfigSet function
* @endinternal
*
* @brief   This function configure extra configuration on the egress attributes of CPU port and
*         add CPU port to the destination port bitmap.
*         DETAILED DESCRIPTION:
*         Inorder to trap packet from upstream port to extended port, send packet also to CPU port.
*         Change CPU port behavior to behave the same as extended port.
*         1. Need to define for CPU port and packetType = UPSTREAM_TO_DOWNSTREAM_MC HA thread U2E.
*         2. Need to change egress target attributes of CPU port to be the same as extended port.
*         In order to remove the VLAN-Tag if the TPID after the E-Tag = Target Port Table[15:0] - TPID and
*         Shared Data Memory [65535:0] - Egress VLAN tag state [VLAN-ID16 + Target Port Table[19:16] - Target Port]=0.
*         3. Need to send the egress packet also to CPU port inoder to trace the received packet.
*         4. Need to set Src PortMap Table for CPU port to filter MC packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number
* @param[in] cid                      - E-channel Id
* @param[in] ingress_cid              - port channel-Id for MC SRC filtering
* @param[in] vid                      - vlan Id
* @param[in] isTagged                 - is vlan tagged
* @param[in] numOfUnicastChannels     - number of unicast E-Channels
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Limitations: Can test only packets from ingress port upstream to egress port extended
*       as we change the egress attributes of the CPU port.
*
*/
static GT_STATUS prvTgfPxHalBpeEnhancedUtUpstream2ExtendedMcConfigSet
(
    IN  GT_SW_DEV_NUM                      devNum,
    IN  GT_U32                             cid,
    IN  GT_U32                             ingress_cid,
    IN  GT_U32                             vid,
    IN  GT_BOOL                            isTagged,
    IN  GT_U32                             numOfUnicastChannels
)
{
    GT_STATUS                                   rc;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  operationInfo;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT        destPortInfo;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT   destInfoType;
    CPSS_PX_PORTS_BMP                           portsBmp;
    GT_U32                                      mcDstIndexCns;
    GT_BOOL                                     bypassLag;

    /* 1. Update Header Alteration Table that maps <Packet Type, Egress port> tuple to the relevant firmware thread */
    rc = cpssPxEgressHeaderAlterationEntrySet(devNum,PRV_CPSS_PX_PORTS_NUM_CNS-1,HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_MC_E,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E,&operationInfo);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressHeaderAlterationEntrySet failed");
    }

    /* 2. Set egress target attributes for ingress port */
    rc = cpssPxEgressTargetPortEntryGet(devNum, PRV_CPSS_PX_PORTS_NUM_CNS-1, &destInfoType, &destPortInfo);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressTargetPortEntryGet failed");
    }

    destPortInfo.info_common.tpid = 0x8100;
    destPortInfo.info_common.pcid = PRV_CPSS_PX_PORTS_NUM_CNS;

    rc = cpssPxEgressTargetPortEntrySet(devNum, PRV_CPSS_PX_PORTS_NUM_CNS-1,destInfoType, &destPortInfo);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressTargetPortEntrySet failed");
    }

    /* Get VLAN tag state entry for target ports */
    rc = cpssPxEgressVlanTagStateEntryGet(devNum,vid,&portsBmp);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressVlanTagStateEntryGet failed");
    }

    if(isTagged == GT_FALSE)
    {
        PRV_CPSS_PX_HAL_PORTS_BMP_PORT_CLEAR_MAC(portsBmp,PRV_CPSS_PX_PORTS_NUM_CNS-1);
    }
    else
    {
        PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmp,PRV_CPSS_PX_PORTS_NUM_CNS-1);
    }
     
    /* Set VLAN tag state entry for target ports */
    rc = cpssPxEgressVlanTagStateEntrySet(devNum,vid,portsBmp);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressVlanTagStateEntrySet failed");
    }     

    mcDstIndexCns = numOfUnicastChannels + HAL_BPE_NUM_RESERVED_ENTRIES_CNS -_4K;    

    /* 3. Add CPU port to the destination ports bitmap */
    rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,cid+mcDstIndexCns,&portsBmp,&bypassLag);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxIngressPortMapEntryGet failed");
    }

    /* Set appropriate bit for portNum */
    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmp,PRV_CPSS_PX_PORTS_NUM_CNS-1);

    /* Configure Dst port map table entry for the appropriate bit */
    rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,cid+mcDstIndexCns,portsBmp,bypassLag);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxIngressPortMapEntrySet failed");
    }

    if(ingress_cid != 0)
    {
        /* 4. Update Src PortMap Table for CPU port */
        rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,ingress_cid,&portsBmp,&bypassLag);
        if(GT_OK != rc)
        {
            return rc;
        }
        /* Set appropriate bit for portNum */
        PRV_CPSS_PX_HAL_PORTS_BMP_PORT_CLEAR_MAC(portsBmp,PRV_CPSS_PX_PORTS_NUM_CNS-1);

        /* Configure Src port map table entry pcid for the appropriate bit */
        rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,ingress_cid,portsBmp,bypassLag);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvTgfPxHalBpeEnhancedUtUpstream2CascadeMcConfigSet function
* @endinternal
*
* @brief   This function configure extra configuration on the egress attributes of CPU port and
*         add CPU port to the destination port bitmap.
*         DETAILED DESCRIPTION:
*         Inorder to trap packet from upstream port to cascade port, send packet also to CPU port.
*         Change CPU port behavior to behave the same as cascade port.
*         1. Need to define for CPU port and packetType = UPSTREAM_TO_DOWNSTREAM_MC HA thread U2E.
*         2. Need to change egress target attributes of CPU port to be the same as cascade port.
*         In order remove the E-Tag : Packet's E-Tag<E-CID> = Target Port Table[43:32] - PCID.
*         3. Need to send the egress packet also to CPU port inoder to trace the received packet.
*         4. Need to set Src PortMap Table for CPU port to filter MC packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number
* @param[in] cid                      - E-channel Id
* @param[in] ingress_cid              - port channel-Id for MC SRC filtering
* @param[in] numOfUnicastChannels     - number of unicast E-Channels
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Limitations: Can test only packets from ingress port upstream to egress port extended
*       as we change the egress attributes of the CPU port.
*
*/
static GT_STATUS prvTgfPxHalBpeEnhancedUtUpstream2CascadeMcConfigSet
(
    IN  GT_SW_DEV_NUM                      devNum,
    IN  GT_U32                             cid,
    IN  GT_U32                             ingress_cid,
    IN  GT_U32                             numOfUnicastChannels
)
{
    GT_STATUS                                   rc;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  operationInfo;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT        destPortInfo;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT   destInfoType;
    CPSS_PX_PORTS_BMP                           portsBmp;
    GT_U32                                      mcDstIndexCns;
    GT_BOOL                                     bypassLag;

    /* 1. Update Header Alteration Table that maps <Packet Type, Egress port> tuple to the relevant firmware thread */
    rc = cpssPxEgressHeaderAlterationEntrySet(devNum,PRV_CPSS_PX_PORTS_NUM_CNS-1,HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_MC_E,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E,&operationInfo);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressHeaderAlterationEntrySet failed");
    }

    /* 2. Set egress target attributes for ingress port */
    rc = cpssPxEgressTargetPortEntryGet(devNum, PRV_CPSS_PX_PORTS_NUM_CNS-1, &destInfoType, &destPortInfo);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressTargetPortEntryGet failed");
    }

    destPortInfo.info_common.tpid = 0x8100;

    rc = cpssPxEgressTargetPortEntrySet(devNum, PRV_CPSS_PX_PORTS_NUM_CNS-1,destInfoType, &destPortInfo);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressTargetPortEntrySet failed");
    }

    mcDstIndexCns = numOfUnicastChannels + HAL_BPE_NUM_RESERVED_ENTRIES_CNS -_4K;

    /* 3. Add CPU port to the destination ports bitmap */
    rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,cid+mcDstIndexCns,&portsBmp,&bypassLag);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxIngressPortMapEntryGet failed");
    }

    /* Set appropriate bit for portNum */
    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmp,PRV_CPSS_PX_PORTS_NUM_CNS-1);

    /* Configure Dst port map table entry for the appropriate bit */
    rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,cid+mcDstIndexCns,portsBmp,bypassLag);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxIngressPortMapEntrySet failed");
    }

    if(ingress_cid != 0)
    {
        /* 4. Update Src PortMap Table for CPU port */
        rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,ingress_cid,&portsBmp,&bypassLag);
        if(GT_OK != rc)
        {
            return rc;
        }
        /* Set appropriate bit for portNum */
        PRV_CPSS_PX_HAL_PORTS_BMP_PORT_CLEAR_MAC(portsBmp,PRV_CPSS_PX_PORTS_NUM_CNS-1);

        /* Configure Src port map table entry pcid for the appropriate bit */
        rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,ingress_cid,portsBmp,bypassLag);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigRestore function
* @endinternal
*
* @brief   This function remove extra configuration on the egress attributes of CPU port and
*         remove CPU port from the destination port bitmap.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] cid                      - E-channel Id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigRestore
(
    IN  GT_SW_DEV_NUM                      devNum,
    IN  GT_U32                             cid
)
{
    GT_STATUS                                   rc;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  operationInfo;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT        destPortInfo;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT   destInfoType;
    CPSS_PX_PORTS_BMP                           portsBmp;
    GT_BOOL                                     bypassLag;

    /* 1. Update Header Alteration Table that maps <Packet Type, Egress port> tuple to the relevant firmware thread */
    cpssOsMemSet(&operationInfo,0,sizeof(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT));
    rc = cpssPxEgressHeaderAlterationEntrySet(devNum,PRV_CPSS_PX_PORTS_NUM_CNS-1,HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_UC_E,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E,&operationInfo);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressHeaderAlterationEntrySet failed");
    }

    /* 2. Set egress target attributes for CPU port */
    rc = cpssPxEgressTargetPortEntryGet(devNum, PRV_CPSS_PX_PORTS_NUM_CNS-1, &destInfoType, &destPortInfo);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressTargetPortEntryGet failed");
    }

    destPortInfo.info_common.tpid = 0x893F;
/*    destPortInfo.info_common.vlanEgrTagState = GT_FALSE; */
    destPortInfo.info_common.pcid = 0;

    rc = cpssPxEgressTargetPortEntrySet(devNum, PRV_CPSS_PX_PORTS_NUM_CNS-1,destInfoType, &destPortInfo);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressTargetPortEntrySet failed");
    }

    /* 3. remove CPU port from the destination ports bitmap */
    rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,cid+HAL_BPE_NUM_RESERVED_ENTRIES_CNS,&portsBmp,&bypassLag);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxIngressPortMapEntryGet failed");
    }

    /* Unset appropriate bit for portNum */
    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_CLEAR_MAC(portsBmp,PRV_CPSS_PX_PORTS_NUM_CNS-1);

    /* Configure Dst port map table entry for the appropriate bit */
    rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,cid+HAL_BPE_NUM_RESERVED_ENTRIES_CNS,portsBmp,bypassLag);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxIngressPortMapEntrySet failed");
    }

    return GT_OK;
}

/**
* @internal prvTgfPxHalBpeEnhancedUtUpstream2CascadeUcConfigRestore function
* @endinternal
*
* @brief   This function remove extra configuration on the egress attributes of CPU port and
*         remove CPU port from the destination port bitmap.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] cid                      - E-channel Id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvTgfPxHalBpeEnhancedUtUpstream2CascadeUcConfigRestore
(
    IN  GT_SW_DEV_NUM                      devNum,
    IN  GT_U32                             cid
)
{
    return prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigRestore(devNum,cid);
}

/**
* @internal prvTgfPxHalBpeEnhancedUtUpstream2ExtendedMcConfigRestore function
* @endinternal
*
* @brief   This function remove extra configuration on the egress attributes of CPU port and
*         remove CPU port from the destination port bitmap.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] cid                      - E-channel Id
* @param[in] ingress_cid              - port channel-Id for MC SRC filtering
* @param[in] numOfUnicastChannels     - number of unicast E-Channels
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvTgfPxHalBpeEnhancedUtUpstream2ExtendedMcConfigRestore
(
    IN  GT_SW_DEV_NUM                      devNum,
    IN  GT_U32                             cid,
    IN  GT_U32                             ingress_cid,
    IN  GT_U32                             numOfUnicastChannels
)
{
    GT_STATUS                                   rc;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  operationInfo;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT        destPortInfo;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT   destInfoType;
    CPSS_PX_PORTS_BMP                           portsBmp;
    GT_U32                                      mcDstIndexCns;
    GT_BOOL                                     bypassLag;

    /* 1. Update Header Alteration Table that maps <Packet Type, Egress port> tuple to the relevant firmware thread */
    rc = cpssPxEgressHeaderAlterationEntrySet(devNum,PRV_CPSS_PX_PORTS_NUM_CNS-1,HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_MC_E,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E,&operationInfo);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressHeaderAlterationEntrySet failed");
    }

    /* 2. Set egress target attributes for CPU port */
    rc = cpssPxEgressTargetPortEntryGet(devNum, PRV_CPSS_PX_PORTS_NUM_CNS-1, &destInfoType, &destPortInfo);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressTargetPortEntryGet failed");
    }

    destPortInfo.info_common.tpid = 0x893F;
/*    destPortInfo.info_common.vlanEgrTagState = GT_FALSE; */

    rc = cpssPxEgressTargetPortEntrySet(devNum, PRV_CPSS_PX_PORTS_NUM_CNS-1,destInfoType, &destPortInfo);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxEgressTargetPortEntrySet failed");
    }

    mcDstIndexCns = numOfUnicastChannels + HAL_BPE_NUM_RESERVED_ENTRIES_CNS -_4K;

    /* 3. remove CPU port from the destination ports bitmap */
    rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,cid+mcDstIndexCns,&portsBmp,&bypassLag);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxIngressPortMapEntryGet failed");
    }

    /* Unset appropriate bit for portNum */
    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_CLEAR_MAC(portsBmp,PRV_CPSS_PX_PORTS_NUM_CNS-1);

    /* Configure Dst port map table entry for the appropriate bit */
    rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,cid+mcDstIndexCns,portsBmp,bypassLag);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxIngressPortMapEntrySet failed");
    }

    /* 4. Unset Src PortMap Table for CPU port */
    rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,ingress_cid,&portsBmp,&bypassLag);
    if(GT_OK != rc)
    {
        return rc;
    }
    /* Set appropriate bit for portNum */
    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmp,PRV_CPSS_PX_PORTS_NUM_CNS-1);

    /* Configure Src port map table entry pcid for the appropriate bit */
    rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,ingress_cid,portsBmp,bypassLag);
    if(GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvTgfPxHalBpeEnhancedUtUpstream2CascadeMcConfigRestore function
* @endinternal
*
* @brief   This function remove extra configuration on the egress attributes of CPU port and
*         remove CPU port from the destination port bitmap.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] cid                      - E-channel Id
* @param[in] ingress_cid              - port channel-Id for MC SRC filtering
* @param[in] numOfUnicastChannels     - number of unicast E-Channels
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvTgfPxHalBpeEnhancedUtUpstream2CascadeMcConfigRestore
(
    IN  GT_SW_DEV_NUM                      devNum,
    IN  GT_U32                             cid,
    IN  GT_U32                             ingress_cid,
    IN  GT_U32                             numOfUnicastChannels
)
{
    return prvTgfPxHalBpeEnhancedUtUpstream2ExtendedMcConfigRestore(devNum,cid,ingress_cid,numOfUnicastChannels);
}

/**
* @internal prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress function
* @endinternal
*
* @brief   This function transmits packet to specified 'ingressInterface'.
*         Counters are checked according to 'egressPortsBmp' and 'egressPacketLen'.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - The queue from which this packet is transmitted (APPLICABLE RANGES: 0..7).
* @param[in] ingressInterfacePtr      - ingress interface.
* @param[in] egressPortsBmp           - Bitmap where a port is set if it is expected to egress the packet. For trunks,
*                                      all members of trunk are set.
* @param[in] isEgressTrunk            - Specifies if egress interface is trunk.
* @param[in] packet                   - Buffer containing the  to be transmitted.
* @param[in] expectedPacket           - Buffer containing the expected packet to be received.
* @param[in] ingressPacketLen         - Length of packet buffer.
* @param[in] egressPacketLen[PRV_CPSS_PX_PORTS_NUM_CNS - 1] - Array of size [PRV_CPSS_PX_PORTS_NUM_CNS - 1], for every port set in 'egressPortsBmp'
*                                      specified the length of packet to be egressed from each port. If isEgressTrunk == TRUE,
* @param[in] egressPacketLen[PRV_CPSS_PX_PORTS_NUM_CNS - 1] is used to determine the length.
*
* @retval GT_OK                    - packet transmitted and egressed as expected
* @retval GT_FAIL                  - counters do not match the expected values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong devNum,portNum,rxQueue.
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress
(
    IN  GT_SW_DEV_NUM                      devNum,
    IN  GT_U32                             txQueue,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC *ingressInterfacePtr,
    IN  CPSS_PX_PORTS_BMP                  egressPortsBmp,
    IN  GT_BOOL                            isEgressTrunk,
    IN  GT_U8                              *packet,
    IN  GT_U8                              *expectedPacket,
    IN  GT_U32                             ingressPacketLen,
    IN  GT_U32                             egressPacketLen[PRV_CPSS_PX_PORTS_NUM_CNS - 1]
)
{
    GT_STATUS                           rc = GT_OK;
    GT_BOOL                             counterMismatch = GT_FALSE;
    GT_BOOL                             isPacketMc = GT_FALSE;
    GT_PORT_NUM                         portNum, ingressPort;
    CPSS_PX_PORT_MAC_COUNTERS_STC       portMacCounters[PRV_CPSS_PX_PORTS_NUM_CNS - 1];
    GT_U32                              actualValue, expectedValue;
    GT_U32                              bytesSent, bytesRecv, ucFramesSent, ucFramesRecv, mcFramesSent, mcFramesRecv;
    GT_U32                              totalBytesSent, totalBytesRecv, totalUcFramesSent, totalUcFramesRecv, totalMcFramesSent, totalMcFramesRecv;
    GT_U32                              i,rxQueue = 0; 
    GT_U32                              numOfBuff = BUFF_LEN;
    GT_U32                              expectedPacketLen;
    GT_U8                               packetBuffsArr[BUFF_LEN];
    GT_U32                              buffLenArr[BUFF_LEN];
    GT_U8*                              bufferPtr;
    GT_BOOL                             didError = GT_FALSE;
    GT_BOOL                             printError = GT_FALSE;
    GT_BOOL                             returnGtNoMore = GT_FALSE;

    /* Validate parameters */
    CPSS_NULL_PTR_CHECK_MAC(ingressInterfacePtr);
    CPSS_NULL_PTR_CHECK_MAC(packet);
    CPSS_NULL_PTR_CHECK_MAC(expectedPacket);

    ingressPort = ingressInterfacePtr->devPort.portNum;

    if((ingressInterfacePtr->type != CPSS_PX_HAL_BPE_INTERFACE_PORT_E) ||  (PRV_CPSS_PX_HAL_PORTS_BMP_IS_PORT_SET_MAC(egressPortsBmp, ingressPort) == GT_TRUE)) {
        return GT_BAD_PARAM;
    }

    if(packet[0] & 0x01) {
        isPacketMc = GT_TRUE;
    }

    /* Enable all ports */
    for(portNum = 0 ; portNum < PRV_CPSS_PX_PORTS_NUM_CNS - 1; portNum++)
    {
        rc = cpssPxIngressPortTargetEnableSet(devNum, portNum, GT_TRUE);
        if(rc != GT_OK) {
            BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxIngressPortTargetEnableSet failed on port %d", portNum);
        }

        rc = cpssPxPortForceLinkPassEnableSet(devNum, portNum, GT_TRUE);
        if(rc != GT_OK) {
            BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxPortForceLinkPassEnableSet failed on port %d", portNum);
        }
    }

    /* Mask Rx events */
    if(prvTgfPxHalBpeMaskRxEvents(GT_TRUE) == GT_FALSE) {
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "prvTgfPxHalBpeMaskRxEvents failed");
    }

    /* Set loopback on ingress port */
    rc = cpssPxPortInternalLoopbackEnableSet(devNum, ingressPort, GT_TRUE);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxPortInternalLoopbackEnableSet failed on port %d", ingressPort);
    }

    /* Reset port counters before packet send */
    for(portNum = 0; portNum < PRV_CPSS_PX_PORTS_NUM_CNS - 1; portNum++) {

        rc = cpssPxPortMacCountersOnPortGet(devNum, portNum, &portMacCounters[portNum]);
        if(rc != GT_OK) {
            BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxPortMacCountersOnPortGet failed on port %d", portNum);
        }
    }

    /* Wait: allow to print the transmitted packet */
    cpssOsTimerWkAfter(20);

    /* print the transmitted packet */
    BPE_LOG_INFORMATION_MAC("tracePacket: Transmitted packet:");
    prvTgfPxHalBpeTracePacket(packet, ingressPacketLen, GT_FALSE);

    /* Send packet to ingress port */
    rc = cpssPxHalBpeSendPacketToInterface(devNum, txQueue, ingressInterfacePtr, &packet, &ingressPacketLen, 1);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxHalBpeSendPacketToInterface failed");
    }

    /* Wait */
    cpssOsTimerWkAfter(SLEEP_AFTER_PACKET_SEND);

    /* get the packet from the device */
    bufferPtr = &packetBuffsArr[0];
    rc = cpssPxNetIfSdmaRxPacketGet(devNum, rxQueue,&numOfBuff,
                                        &bufferPtr,buffLenArr);

    if(rc == GT_NO_MORE) { /* GT_NO_MORE is good and not an error !!! */
        BPE_LOG_INFORMATION_MAC("cpssPxNetIfSdmaRxPacketGet returned GT_ON_MORE");
        returnGtNoMore = GT_TRUE;
        goto checkCounters;
    }
    else if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxNetIfSdmaRxPacketGet failed");
    }

    /* now you need to free the buffers */
    rc = cpssPxNetIfRxBufFree(devNum,rxQueue,&bufferPtr,numOfBuff);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxNetIfRxBufFree failed");
    }

    /* print the received packet */
    BPE_LOG_INFORMATION_MAC("tracePacket: Received packet:");
    prvTgfPxHalBpeTracePacket(bufferPtr,buffLenArr[0], GT_TRUE);

    /* compare the packets length */
    for(portNum = 0; portNum < PRV_CPSS_PX_PORTS_NUM_CNS - 1; portNum++) {
        if(PRV_CPSS_PX_HAL_PORTS_BMP_IS_PORT_SET_MAC(egressPortsBmp, portNum) == GT_TRUE)
        {
            expectedPacketLen = isEgressTrunk ? egressPacketLen[0] : egressPacketLen[portNum];
            if(buffLenArr[0] != expectedPacketLen && ignoreErrorOnPacketLength == GT_FALSE)
            {
                BPE_LOG_ERROR_MAC("tracePacket: ERROR - the expected packet length [%d] is different then the received packet length [%d]",egressPacketLen[portNum],buffLenArr[0]);
                didError = GT_TRUE;
            }
        }
    }

    /* compare the full packets except for CRC added by simulation. all the TO_CPU packets in SDMA mode come with 0x55555555 as CRC */
    for(portNum = 0; portNum < PRV_CPSS_PX_PORTS_NUM_CNS - 1; portNum++) {
        if(PRV_CPSS_PX_HAL_PORTS_BMP_IS_PORT_SET_MAC(egressPortsBmp, portNum) == GT_TRUE)
        {
            for(i = 0 ; (i < (buffLenArr[0] - BYTES_IN_CRC) && i < egressPacketLen[portNum])  ; i++)
            {
                if (bufferPtr[i] != expectedPacket[i]) {
                    BPE_LOG_ERROR_MAC("tracePacket: ERROR - packet byte[%d] mismatch: received = 0x%x, expected = 0x%x", i, bufferPtr[i], expectedPacket[i]);
                    didError = GT_TRUE;
                    printError = GT_TRUE;
                }
            }
        }
    }

    if(printError)
    {
        BPE_LOG_ERROR_MAC("tracePacket: ERROR - the expected packet is different then the received packet !");
    }
    else
    {
        BPE_LOG_INFORMATION_MAC("tracePacket: the expected packet is the same as received !");
    }
checkCounters:
    /* Get counters for ingerss port */
    rc = cpssPxPortMacCountersOnPortGet(devNum, ingressPort, &portMacCounters[ingressPort]);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxPortMacCountersOnPortGet failed on port %d", ingressPort);
    }
    /* Check number of unicast frames sent and received on ingerss port */
    actualValue = portMacCounters[ingressPort].mibCounter[CPSS_PX_PORT_MAC_COUNTER_UNICAST_FRAME_SENT_E].l[0];
    expectedValue = isPacketMc ? 0 : 1;
    if(actualValue != expectedValue) {
        BPE_LOG_ERROR_MAC("Port[%d] counter[UNICAST_FRAME_SENT]: expected = %d, actual = %d", ingressPort, expectedValue, actualValue);
        counterMismatch = GT_TRUE;
    }
    actualValue = portMacCounters[ingressPort].mibCounter[CPSS_PX_PORT_MAC_COUNTER_GOOD_UNICAST_FRAMES_RECEIVED_E].l[0];
    if(actualValue != expectedValue) {
        BPE_LOG_ERROR_MAC("Port[%d] counter[GOOD_UNICAST_FRAMES_RECEIVED]: expected = %d, actual = %d", ingressPort, expectedValue, actualValue);
        counterMismatch = GT_TRUE;
    }
    /* Check number of multicast frames sent and received on ingerss port */
    actualValue = portMacCounters[ingressPort].mibCounter[CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_SENT_E].l[0];
    expectedValue = isPacketMc ? 1 : 0;
    if(actualValue != expectedValue) {
        BPE_LOG_ERROR_MAC("Port[%d] counter[MULTICAST_FRAMES_SENT]: expected = %d, actual = %d", ingressPort, expectedValue, actualValue);
        counterMismatch = GT_TRUE;
    }
    actualValue = portMacCounters[ingressPort].mibCounter[CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_RECEIVED_E].l[0];
    if(actualValue != expectedValue) {
        BPE_LOG_ERROR_MAC("Port[%d] counter[MULTICAST_FRAMES_RECEIVED]: expected = %d, actual = %d", ingressPort, expectedValue, actualValue);
        counterMismatch = GT_TRUE;
    }
    /* Check number of bytes sent and received on ingress port */
    actualValue = portMacCounters[ingressPort].mibCounter[CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E].l[0];
    expectedValue = ingressPacketLen;
    if(actualValue != expectedValue) {
        BPE_LOG_ERROR_MAC("Port[%d] counter[GOOD_OCTETS_SENT]: expected = %d, actual = %d", ingressPort, expectedValue, actualValue);
        counterMismatch = GT_TRUE;
    }
    actualValue = portMacCounters[ingressPort].mibCounter[CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E].l[0];
    if(actualValue != expectedValue) {
        BPE_LOG_ERROR_MAC("Port[%d] counter[GOOD_OCTETS_RECEIVED]: expected = %d, actual = %d", ingressPort, expectedValue, actualValue);
        counterMismatch = GT_TRUE;
    }

    /* Get counters for the reset of the ports */
    totalBytesSent = 0;
    totalBytesRecv = 0;
    totalUcFramesSent = 0;
    totalUcFramesRecv = 0;
    totalMcFramesSent = 0;
    totalMcFramesRecv = 0;
    for(portNum = 0; portNum < PRV_CPSS_PX_PORTS_NUM_CNS - 1; portNum++) {
        if(portNum != ingressPort) {
            rc = cpssPxPortMacCountersOnPortGet(devNum, portNum, &portMacCounters[portNum]);
            if(rc != GT_OK) {
                BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxPortMacCountersOnPortGet failed on port %d", portNum);
            }
            bytesSent = portMacCounters[portNum].mibCounter[CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E].l[0];
            bytesRecv = portMacCounters[portNum].mibCounter[CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E].l[0];
            ucFramesSent = portMacCounters[portNum].mibCounter[CPSS_PX_PORT_MAC_COUNTER_UNICAST_FRAME_SENT_E].l[0];
            ucFramesRecv = portMacCounters[portNum].mibCounter[CPSS_PX_PORT_MAC_COUNTER_GOOD_UNICAST_FRAMES_RECEIVED_E].l[0];
            mcFramesSent = portMacCounters[portNum].mibCounter[CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_SENT_E].l[0];
            mcFramesRecv = portMacCounters[portNum].mibCounter[CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_RECEIVED_E].l[0];
            /* Check for traffic on un-expected egress ports */
            if(PRV_CPSS_PX_HAL_PORTS_BMP_IS_PORT_SET_MAC(egressPortsBmp, portNum) == GT_FALSE) {
                if(bytesSent > 0) {
                    BPE_LOG_ERROR_MAC("Port[%d] counter[GOOD_OCTETS_SENT]: expected = 0, actual = %d", portNum, bytesSent);
                    counterMismatch = GT_TRUE;
                }
                if(bytesRecv > 0) {
                    BPE_LOG_ERROR_MAC("Port[%d] counter[GOOD_OCTETS_RECEIVED]: expected = 0, actual = %d", portNum, bytesRecv);
                    counterMismatch = GT_TRUE;
                }
                if(ucFramesSent > 0) {
                    BPE_LOG_ERROR_MAC("Port[%d] counter[UNICAST_FRAME_SENT]: expected = 0, actual = %d", portNum, ucFramesSent);
                    counterMismatch = GT_TRUE;
                }
                if(ucFramesRecv > 0) {
                    BPE_LOG_ERROR_MAC("Port[%d] counter[GOOD_UNICAST_FRAMES_RECEIVED]: expected = 0, actual = %d", portNum, ucFramesRecv);
                    counterMismatch = GT_TRUE;
                }
                if(mcFramesSent > 0) {
                    BPE_LOG_ERROR_MAC("Port[%d] counter[MULTICAST_FRAMES_SENT]: expected = 0, actual = %d", portNum, mcFramesSent);
                    counterMismatch = GT_TRUE;
                }
                if(mcFramesRecv > 0) {
                    BPE_LOG_ERROR_MAC("Port[%d] counter[MULTICAST_FRAMES_RECEIVED]: expected = 0, actual = %d", portNum, mcFramesRecv);
                    counterMismatch = GT_TRUE;
                }
            }
            totalBytesSent    += bytesSent;
            totalBytesRecv    += bytesRecv;
            totalUcFramesSent += ucFramesSent;
            totalUcFramesRecv += ucFramesRecv;
            totalMcFramesSent += mcFramesSent;
            totalMcFramesRecv += mcFramesRecv;
        }
    }
    if(isEgressTrunk == GT_TRUE) {

        actualValue = totalUcFramesSent;
        expectedValue = (!isPacketMc && (egressPortsBmp != 0)) ? 1 : 0;
        if(actualValue != expectedValue) {
            BPE_LOG_ERROR_MAC("Trunk counter[UNICAST_FRAME_SENT]: expected = %d, actual = %d", expectedValue, actualValue);
            counterMismatch = GT_TRUE;
        }
        actualValue = totalUcFramesRecv;
        expectedValue = 0;
        if(actualValue != expectedValue) {
            BPE_LOG_ERROR_MAC("Trunk counter[GOOD_UNICAST_FRAMES_RECEIVED]: expected = %d, actual = %d", expectedValue, actualValue);
            counterMismatch = GT_TRUE;
        }
        actualValue = totalMcFramesSent;
        expectedValue = (isPacketMc && (egressPortsBmp != 0)) ? 1 : 0;
        if(actualValue != expectedValue) {
            BPE_LOG_ERROR_MAC("Trunk counter[MULTICAST_FRAMES_SENT]: expected = %d, actual = %d", expectedValue, actualValue);
            counterMismatch = GT_TRUE;
        }
        actualValue = totalMcFramesRecv;
        expectedValue = 0;
        if(actualValue != expectedValue) {
            BPE_LOG_ERROR_MAC("Trunk counter[MULTICAST_FRAMES_RECEIVED]: expected = %d, actual = %d", expectedValue, actualValue);
            counterMismatch = GT_TRUE;
        }
        actualValue = totalBytesSent;
        expectedValue = (egressPortsBmp != 0) ? egressPacketLen[0] : 0;
        if(actualValue != expectedValue) {
            BPE_LOG_ERROR_MAC("Trunk counter[GOOD_OCTETS_SENT]: expected = %d, actual = %d", expectedValue, actualValue);
            counterMismatch = GT_TRUE;
        }
        actualValue = totalBytesRecv;
        expectedValue = 0;
        if(actualValue != expectedValue) {
            BPE_LOG_ERROR_MAC("Trunk counter[GOOD_OCTETS_RECEIVED]: expected = %d, actual = %d", expectedValue, actualValue);
            counterMismatch = GT_TRUE;
        }
    }
    else {

        for(portNum = 0; portNum < PRV_CPSS_PX_PORTS_NUM_CNS - 1; portNum++) {
            if(PRV_CPSS_PX_HAL_PORTS_BMP_IS_PORT_SET_MAC(egressPortsBmp, portNum) == GT_TRUE) {

                bytesSent = portMacCounters[portNum].mibCounter[CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E].l[0];
                bytesRecv = portMacCounters[portNum].mibCounter[CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E].l[0];
                ucFramesSent = portMacCounters[portNum].mibCounter[CPSS_PX_PORT_MAC_COUNTER_UNICAST_FRAME_SENT_E].l[0];
                ucFramesRecv = portMacCounters[portNum].mibCounter[CPSS_PX_PORT_MAC_COUNTER_GOOD_UNICAST_FRAMES_RECEIVED_E].l[0];
                mcFramesSent = portMacCounters[portNum].mibCounter[CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_SENT_E].l[0];
                mcFramesRecv = portMacCounters[portNum].mibCounter[CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_RECEIVED_E].l[0];

                actualValue = ucFramesSent;
                expectedValue = isPacketMc ? 0 : 1;
                if(actualValue != expectedValue) {
                    BPE_LOG_ERROR_MAC("Port[%d] counter[UNICAST_FRAME_SENT]: expected = %d, actual = %d", portNum, expectedValue, actualValue);
                    counterMismatch = GT_TRUE;
                }
                actualValue = ucFramesRecv;
                expectedValue = 0;
                if(actualValue != expectedValue) {
                    BPE_LOG_ERROR_MAC("Port[%d] counter[GOOD_UNICAST_FRAMES_RECEIVED]: expected = %d, actual = %d", portNum, expectedValue, actualValue);
                    counterMismatch = GT_TRUE;
                }
                actualValue = mcFramesSent;
                expectedValue = isPacketMc ? 1 : 0;
                if(actualValue != expectedValue) {
                    BPE_LOG_ERROR_MAC("Port[%d] counter[MULTICAST_FRAMES_SENT]: expected = %d, actual = %d", portNum, expectedValue, actualValue);
                    counterMismatch = GT_TRUE;
                }
                actualValue = mcFramesRecv;
                expectedValue = 0;
                if(actualValue != expectedValue) {
                    BPE_LOG_ERROR_MAC("Port[%d] counter[MULTICAST_FRAMES_RECEIVED]: expected = %d, actual = %d", portNum, expectedValue, actualValue);
                    counterMismatch = GT_TRUE;
                }
                actualValue = bytesSent;
                expectedValue = egressPacketLen[portNum];
                if(actualValue != expectedValue) {
                    BPE_LOG_ERROR_MAC("Port[%d] counter[GOOD_OCTETS_SENT]: expected = %d, actual = %d", portNum, expectedValue, actualValue);
                    counterMismatch = GT_TRUE;
                }
                actualValue = bytesRecv;
                expectedValue = 0;
                if(actualValue != expectedValue) {
                    BPE_LOG_ERROR_MAC("Port[%d] counter[GOOD_OCTETS_RECEIVED]: expected = %d, actual = %d", portNum, expectedValue, actualValue);
                    counterMismatch = GT_TRUE;
                }
            }
        }
    }

    /* Unset loopback on ingress port */
    rc = cpssPxPortInternalLoopbackEnableSet(devNum, ingressPort, GT_FALSE);
    if(rc != GT_OK) {
        BPE_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxPortInternalLoopbackEnableSet failed on port %d", ingressPort);
    }

    /* Unmask Rx events */
    if(prvTgfPxHalBpeMaskRxEvents(GT_FALSE) == GT_FALSE) {
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "prvTgfPxHalBpeMaskRxEvents failed");
    }

    if(counterMismatch == GT_TRUE || didError == GT_TRUE) {
        return GT_FAIL;
    }
    if (returnGtNoMore == GT_TRUE) {
        return GT_NO_MORE;
    }
    return GT_OK;
}

UTF_TEST_CASE_MAC(prvTgfPxHalBpe_U2E_UC)
{
    GT_U8                              dev = 0;
    GT_STATUS                          st;
    GT_PORT_NUM                        extPort = 0;
    CPSS_PX_PORTS_BMP                  extPortBmp = 0;
    GT_PORT_NUM                        upPort = 12;
    GT_U32                             packetLen = 100;
    GT_U32                             egressPacketLen[PRV_CPSS_PX_PORTS_NUM_CNS - 1];
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC egressInterface, ingressInterface;
    union                              {
        GT_U16                         octetPair;
        GT_U8                          octet[2];
    }cid;

    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(extPortBmp, extPort);

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Perform common configuration */
        ingressInterface.type = egressInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        ingressInterface.devPort.devNum = egressInterface.devPort.devNum = dev;
        ingressInterface.devPort.portNum = upPort;
        egressInterface.devPort.portNum = extPort;

        st = cpssPxHalBpeInterfaceTypeSet(dev, &ingressInterface, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, ingressInterface.type, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);

        st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface.type, egressInterface.devPort.devNum, egressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);

        /* 1.1   Set numOfUnicastChannels == 4K - 20, numOfMulticastChannels == 4K,
           Configure unicast E-channel with cid == 1 and send packet */
        cid.octetPair = 1;

        st = cpssPxHalBpeNumOfChannelsSet(dev, _4K - 20, _4K);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, _4K - 20, _4K);

        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &egressInterface, cid.octetPair);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, egressInterface.devPort.devNum, egressInterface.devPort.portNum, cid.octetPair);

        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid.octetPair, &egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        st = cpssPxHalBpePortExtendedUpstreamSet(dev, extPort, &ingressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigSet(dev, cid.octetPair, 0xFFF, GT_FALSE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = 0x00;
        transmittedPkt[1] = expectedPkt[1] = 0x00;
        transmittedPkt[2] = expectedPkt[2] = 0x00;
        transmittedPkt[3] = expectedPkt[3] = 0x00;
        transmittedPkt[4] = expectedPkt[4] = dev + 1;
        transmittedPkt[5] = expectedPkt[5] = upPort + 1;

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* ingress E-Tag */
        transmittedPkt[12] = 0x89;
        transmittedPkt[13] = 0x3F;
        transmittedPkt[14] = 0x00;
        transmittedPkt[15] = 0x00;
        transmittedPkt[16] = cid.octet[1];
        transmittedPkt[17] = cid.octet[0];
        transmittedPkt[18] = 0x00;
        transmittedPkt[19] = 0x00;

        egressPacketLen[extPort] = packetLen - BYTES_IN_E_TAG;

        /* Send packet and check egress as expected */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  extPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssPxHalBpeUnicastEChannelDelete(dev, cid.octetPair);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* special enhanced UT restore configurations */
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigRestore(dev, cid.octetPair);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* 1.2   Set numOfUnicastChannels == 4K - 20, numOfMulticastChannels == 4K,
           Configure unicast E-channel with cid == 4K - 21 and send packet */
        cid.octetPair = _4K - 21;

        st = cpssPxHalBpeNumOfChannelsSet(dev, _4K - 20, _4K);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, _4K - 20, _4K);

        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &egressInterface, cid.octetPair);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, egressInterface.devPort.devNum, egressInterface.devPort.portNum, cid.octetPair);

        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid.octetPair, &egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        st = cpssPxHalBpePortExtendedUpstreamSet(dev, extPort, &ingressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigSet(dev, cid.octetPair, 0xFFF, GT_FALSE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        transmittedPkt[16] = cid.octet[1];
        transmittedPkt[17] = cid.octet[0];

        /* Send packet and check egress as expected */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  extPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssPxHalBpeUnicastEChannelDelete(dev, cid.octetPair);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* special enhanced UT restore configurations */
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigRestore(dev, cid.octetPair);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* 1.3   Set numOfUnicastChannels == 2, numOfMulticastChannels == 8K - 22,
           Configure unicast E-channel with cid == 1 and send packet */
        cid.octetPair = 1;

        st = cpssPxHalBpeNumOfChannelsSet(dev, 2, _8K - 22);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, 2, _8K - 22);

        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &egressInterface, cid.octetPair);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, egressInterface.devPort.devNum, egressInterface.devPort.portNum, cid.octetPair);

        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid.octetPair, &egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        st = cpssPxHalBpePortExtendedUpstreamSet(dev, extPort, &ingressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigSet(dev, cid.octetPair, 0xFFF, GT_FALSE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        transmittedPkt[16] = cid.octet[1];
        transmittedPkt[17] = cid.octet[0];

        /* Send packet and check egress as expected */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  extPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* special enhanced UT restore configurations */
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigRestore(dev, cid.octetPair);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);
    }
}

UTF_TEST_CASE_MAC(prvTgfPxHalBpe_U2E_UC_withTagMode)
{
    GT_U8                              dev = 0;
    GT_STATUS                          st;
    GT_PORT_NUM                        extPort = 0;
    CPSS_PX_PORTS_BMP                  extPortBmp = 0;
    GT_PORT_NUM                        upPort = 12;
    GT_U32                             packetLen = 100;
    GT_U32                             egressPacketLen[PRV_CPSS_PX_PORTS_NUM_CNS - 1];
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC egressInterface, ingressInterface;
    union                              {
        GT_U16                         octetPair;
        GT_U8                          octet[2];
    }cid;
    cid.octetPair = 0x22;

    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(extPortBmp, extPort);

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Configure device */
        ingressInterface.type = egressInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        ingressInterface.devPort.devNum = egressInterface.devPort.devNum = dev;
        ingressInterface.devPort.portNum = upPort;
        egressInterface.devPort.portNum = extPort;

        st = cpssPxHalBpeNumOfChannelsSet(dev, 100, 0);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, 100, 0);

        st = cpssPxHalBpeInterfaceTypeSet(dev, &ingressInterface, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, ingressInterface.type, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);

        st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface.type, egressInterface.devPort.devNum, egressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);

        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &egressInterface, cid.octetPair);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, egressInterface.devPort.devNum, egressInterface.devPort.portNum, cid.octetPair);

        st = cpssPxHalBpePortExtendedUpstreamSet(dev, extPort, &ingressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort);

        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid.octetPair, &egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* 1. Set egress tag state to untagged with vid = 0x66 */
        st = cpssPxHalBpePortExtendedUntaggedVlanAdd(dev, &egressInterface, 0x66);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface.type, egressInterface.devPort.devNum, egressInterface.devPort.portNum, 0x66);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigSet(dev, cid.octetPair, 0x66, GT_FALSE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* 1.1 Send tagged packet with vid == 0x66 to upstream port and expect it to egress untagged */

        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = 0x00;
        transmittedPkt[1] = expectedPkt[1] = 0x00;
        transmittedPkt[2] = expectedPkt[2] = 0x00;
        transmittedPkt[3] = expectedPkt[3] = 0x00;
        transmittedPkt[4] = expectedPkt[4] = dev + 1;
        transmittedPkt[5] = expectedPkt[5] = upPort + 1;

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* ingress E-Tag */
        transmittedPkt[12] = 0x89;
        transmittedPkt[13] = 0x3F;
        transmittedPkt[14] = 0x00;
        transmittedPkt[15] = 0x00;
        transmittedPkt[16] = cid.octet[1];
        transmittedPkt[17] = cid.octet[0];
        transmittedPkt[18] = 0x00;
        transmittedPkt[19] = 0x00;

        /* ingress C-Tag */
        transmittedPkt[20] = 0x81;
        transmittedPkt[21] = 0x00;
        transmittedPkt[22] = 0x00;
        transmittedPkt[23] = 0x66;

        egressPacketLen[extPort] = packetLen - BYTES_IN_E_TAG - BYTES_IN_C_TAG;

        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  extPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 Send E-tagged C-tagged packet with vid == 0x99 to upstream port and expect it to egress C-tagged */

        st = cpssPxHalBpePortExtendedUntaggedVlanDel(dev,&egressInterface,0x99);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigSet(dev, cid.octetPair, 0x99, GT_TRUE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* ingress + egress C-Tag */
        transmittedPkt[20] = expectedPkt[12] = 0x81;
        transmittedPkt[21] = expectedPkt[13] = 0x00;
        transmittedPkt[22] = expectedPkt[14] = 0x00;
        transmittedPkt[23] = expectedPkt[15] = 0x99;

        egressPacketLen[extPort] = packetLen - BYTES_IN_E_TAG;               

        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  extPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        
        /* 1.3 Send E-tagged packet to upstream port and expect it to egress untagged */

        st = cpssPxHalBpePortExtendedUntaggedVlanDel(dev,&egressInterface,0);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigSet(dev, cid.octetPair, 0, GT_FALSE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* ingress + egress C-Tag */
        transmittedPkt[20] = expectedPkt[12] = 0xFF;
        transmittedPkt[21] = expectedPkt[13] = 0xFF;
        transmittedPkt[22] = expectedPkt[14] = 0xFF;
        transmittedPkt[23] = expectedPkt[15] = 0xFF;

        egressPacketLen[extPort] = packetLen - BYTES_IN_E_TAG;

        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  extPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        
        /* 2. Set egress tag state to untagged 0x99 */

        st = cpssPxHalBpePortExtendedUntaggedVlanAdd(dev, &egressInterface, 0x99);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface.type, egressInterface.devPort.devNum, egressInterface.devPort.portNum, 0x99);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigSet(dev, cid.octetPair, 0x99, GT_FALSE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* 2.1 Send E-tagged C-tagged packet with vid == 0x99 to upstream port and expect it to egress untagged */

        /* ingress C-Tag */
        transmittedPkt[20] = 0x81;
        transmittedPkt[21] = 0x00;
        transmittedPkt[22] = 0x00;
        transmittedPkt[23] = 0x99;

        /* egress C-Tag */
        expectedPkt[12] = 0xFF;
        expectedPkt[13] = 0xFF;
        expectedPkt[14] = 0xFF;
        expectedPkt[15] = 0xFF;

        egressPacketLen[extPort] = packetLen - BYTES_IN_E_TAG - BYTES_IN_C_TAG;

        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  extPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        
        /* 2.2 Send E-tagged packet to upstream port and expect it to egress untagged */

        /* ingress + egress C-Tag */
        transmittedPkt[20] = expectedPkt[12] = 0xFF;
        transmittedPkt[21] = expectedPkt[13] = 0xFF;
        transmittedPkt[22] = expectedPkt[14] = 0xFF;
        transmittedPkt[23] = expectedPkt[15] = 0xFF;

        egressPacketLen[extPort] = packetLen - BYTES_IN_E_TAG;

        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  extPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        
        /* 3. Set egress tag state to tagged 0x66 */

        st = cpssPxHalBpePortExtendedUntaggedVlanDel(dev, &egressInterface, 0x66);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface.type, egressInterface.devPort.devNum, egressInterface.devPort.portNum, 0x66);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigSet(dev, cid.octetPair, 0x66, GT_TRUE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* 3.1 Send E-tagged C-tagged packet with vid == 0x66 to upstream port and expect it to egress C-tagged */

        /* ingress + egress C-Tag */
        transmittedPkt[20] = expectedPkt[12] = 0x81;
        transmittedPkt[21] = expectedPkt[13] = 0x00;
        transmittedPkt[22] = expectedPkt[14] = 0x00;
        transmittedPkt[23] = expectedPkt[15] = 0x66;

        egressPacketLen[extPort] = packetLen - BYTES_IN_E_TAG;

        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  extPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        
        /* 3.2 Send E-tagged C-tagged packet with vid == 0x99 to upstream port and expect it to egress C-tagged */

        st = cpssPxHalBpePortExtendedUntaggedVlanDel(dev, &egressInterface, 0x99);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface.type, egressInterface.devPort.devNum, egressInterface.devPort.portNum, 0x99);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigSet(dev, cid.octetPair, 0x99, GT_TRUE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* ingress + egress C-Tag */
        transmittedPkt[20] = expectedPkt[12] = 0x81;
        transmittedPkt[21] = expectedPkt[13] = 0x00;
        transmittedPkt[22] = expectedPkt[14] = 0x00;
        transmittedPkt[23] = expectedPkt[15] = 0x99;

        egressPacketLen[extPort] = packetLen - BYTES_IN_E_TAG;

        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  extPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 3.3 Send E-tagged packet to upstream port and expect it to egress untagged */

        /* ingress + egress C-Tag */
        transmittedPkt[20] = expectedPkt[12] = 0xFF;
        transmittedPkt[21] = expectedPkt[13] = 0xFF;
        transmittedPkt[22] = expectedPkt[14] = 0xFF;
        transmittedPkt[23] = expectedPkt[15] = 0xFF;

        egressPacketLen[extPort] = packetLen - BYTES_IN_E_TAG;

        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  extPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* special enhanced UT restore configurations */
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigRestore(dev, cid.octetPair);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);
    }
}

UTF_TEST_CASE_MAC(prvTgfPxHalBpe_U2E_MC)
{
    GT_U8                              dev = 0;
    GT_STATUS                          st;
    GT_PORT_NUM                        extPort[EXT_PORTS_IN_MC_GRP] = {0, 3, 15, 6};
    CPSS_PX_PORTS_BMP                  extPortsBmp = 0;
    GT_PORT_NUM                        upPort = 12;
    GT_PORT_NUM                        portIndex;
    GT_U32                             packetLen = 100;
    GT_U32                             egressPacketLen[PRV_CPSS_PX_PORTS_NUM_CNS - 1];
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC ingressInterface, egressInterface[EXT_PORTS_IN_MC_GRP + 1];
    union                              {
        GT_U16                         octetPair;
        GT_U8                          octet[2];
    }cid;    

    for(portIndex = 0; portIndex < EXT_PORTS_IN_MC_GRP; portIndex++) {
        PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(extPortsBmp, extPort[portIndex]);
        egressInterface[portIndex].type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        egressInterface[portIndex].devPort.portNum = extPort[portIndex];
        egressPacketLen[extPort[portIndex]] = packetLen - BYTES_IN_E_TAG;
    }
    egressInterface[portIndex].type = CPSS_PX_HAL_BPE_INTERFACE_NONE_E;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Configure device */
        ingressInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        ingressInterface.devPort.devNum = dev;
        ingressInterface.devPort.portNum = upPort;

        st = cpssPxHalBpeInterfaceTypeSet(dev, &ingressInterface, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, ingressInterface.type, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);

        for(portIndex = 0; portIndex < EXT_PORTS_IN_MC_GRP; portIndex++) {
            egressInterface[portIndex].devPort.devNum = dev;
            st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface[portIndex], CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface[portIndex].type, egressInterface[portIndex].devPort.devNum, egressInterface[portIndex].devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);
        }

        /* 1.1  Set numOfUnicastChannels == 20, numOfMulticastChannels == 8K - 19 - 20 */
        st = cpssPxHalBpeNumOfChannelsSet(dev, 20, _8K - 19 - 20);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, 0, _8K - 19 - 20);

        /* 1.1.1 Configure multicast E-channel with cid == 4K+21 and send packet */
        cid.octetPair = _4K+21;

        st = cpssPxHalBpeMulticastEChannelCreate(dev, cid.octetPair, egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        for(portIndex = 0; portIndex < EXT_PORTS_IN_MC_GRP; portIndex++) {
            st = cpssPxHalBpePortExtendedUpstreamSet(dev, extPort[portIndex], &ingressInterface);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort[portIndex]);

            st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &egressInterface[portIndex], portIndex + 1);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, egressInterface[portIndex].devPort.devNum, egressInterface[portIndex].devPort.portNum, portIndex + 1);
        }

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedMcConfigSet(dev, cid.octetPair, 0, 0, GT_FALSE, 20);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = 0x01;
        transmittedPkt[1] = expectedPkt[1] = 0x00;
        transmittedPkt[2] = expectedPkt[2] = 0x5E;
        transmittedPkt[3] = expectedPkt[3] = 0x00;
        transmittedPkt[4] = expectedPkt[4] = dev + 1;
        transmittedPkt[5] = expectedPkt[5] = upPort + 1;

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* ingress E-Tag */
        transmittedPkt[12] = 0x89;
        transmittedPkt[13] = 0x3F;
        transmittedPkt[14] = 0x00;
        transmittedPkt[15] = 0x00;
        transmittedPkt[16] = cid.octet[1];
        transmittedPkt[17] = cid.octet[0];
        transmittedPkt[18] = 0x00;
        transmittedPkt[19] = 0x00;

        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  extPortsBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
     
        st = cpssPxHalBpeMulticastEChannelDelete(dev, cid.octetPair);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* special enhanced UT restore configurations */
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedMcConfigRestore(dev, cid.octetPair, 0, 20);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* 1.1.2 Configure multicast E-channel with cid == 8K - 21 and send packet */
        cid.octetPair = _8K - 21;

        st = cpssPxHalBpeMulticastEChannelCreate(dev, cid.octetPair, egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        for(portIndex = 0; portIndex < EXT_PORTS_IN_MC_GRP; portIndex++) {
            st = cpssPxHalBpePortExtendedUpstreamSet(dev, extPort[portIndex], &ingressInterface);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort[portIndex]);
        }

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedMcConfigSet(dev, cid.octetPair, 0, 0, GT_FALSE, 20);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        transmittedPkt[16] = cid.octet[1];
        transmittedPkt[17] = cid.octet[0];

        /* Send packet and check egress as expected */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  extPortsBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        
        st = cpssPxHalBpeMulticastEChannelDelete(dev, cid.octetPair);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* special enhanced UT restore configurations */
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedMcConfigRestore(dev, cid.octetPair, 0, 20);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* 1.1.3 Configure multicast E-channel with cid == 8K - 4056 and send packet */
        cid.octetPair = _8K - 4056;

        st = cpssPxHalBpeMulticastEChannelCreate(dev, cid.octetPair, egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        for(portIndex = 0; portIndex < EXT_PORTS_IN_MC_GRP; portIndex++) {
            st = cpssPxHalBpePortExtendedUpstreamSet(dev, extPort[portIndex], &ingressInterface);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort[portIndex]);
        }

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedMcConfigSet(dev, cid.octetPair, 0, 0, GT_FALSE, 20);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        transmittedPkt[16] = cid.octet[1];
        transmittedPkt[17] = cid.octet[0];

        /* Send packet and check egress as expected */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  extPortsBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        
        st = cpssPxHalBpeMulticastEChannelDelete(dev, cid.octetPair);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* special enhanced UT restore configurations */
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedMcConfigRestore(dev, cid.octetPair, 0, 20);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* 1.2  Set numOfUnicastChannels == 4K - 19, numOfMulticastChannels == 4K */
        st = cpssPxHalBpeNumOfChannelsSet(dev, _4K - 19, _4K);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, _4K - 19, _4K);

        /* 1.2.1 Configure multicast E-channel with cid == 8K -20 and send packet */
        cid.octetPair = _8K - 20;

        st = cpssPxHalBpeMulticastEChannelCreate(dev, cid.octetPair, egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        for(portIndex = 0; portIndex < EXT_PORTS_IN_MC_GRP; portIndex++) {
            st = cpssPxHalBpePortExtendedUpstreamSet(dev, extPort[portIndex], &ingressInterface);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort[portIndex]);
        }

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedMcConfigSet(dev,cid.octetPair, 0, 0, GT_FALSE, _4K - 19);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        transmittedPkt[16] = cid.octet[1];
        transmittedPkt[17] = cid.octet[0];

        /* Send packet and check egress as expected */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  extPortsBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        
        st = cpssPxHalBpeMulticastEChannelDelete(dev, cid.octetPair);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* special enhanced UT restore configurations */
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedMcConfigRestore(dev, cid.octetPair, 0, _4K - 19);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* 1.2.2 Configure multicast E-channel with cid == 4075 + 4K and send packet */
        cid.octetPair = 4075 + _4K;

        st = cpssPxHalBpeMulticastEChannelCreate(dev, cid.octetPair, egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        for(portIndex = 0; portIndex < EXT_PORTS_IN_MC_GRP; portIndex++) {
            st = cpssPxHalBpePortExtendedUpstreamSet(dev, extPort[portIndex], &ingressInterface);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort[portIndex]);
        }

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedMcConfigSet(dev, cid.octetPair, 0, 0, GT_FALSE, _4K - 19);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        transmittedPkt[16] = cid.octet[1];
        transmittedPkt[17] = cid.octet[0];

        /* Send packet and check egress as expected */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  extPortsBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        
        st = cpssPxHalBpeMulticastEChannelDelete(dev, cid.octetPair);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* special enhanced UT restore configurations */
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedMcConfigRestore(dev, cid.octetPair, 0, _4K - 19);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* 1.2.3 Configure multicast E-channel with cid == 4K+4074 and send packet */
        cid.octetPair = _4K+4074;

        st = cpssPxHalBpeMulticastEChannelCreate(dev, cid.octetPair, egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        for(portIndex = 0; portIndex < EXT_PORTS_IN_MC_GRP; portIndex++) {
            st = cpssPxHalBpePortExtendedUpstreamSet(dev, extPort[portIndex], &ingressInterface);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort[portIndex]);
        }

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedMcConfigSet(dev,cid.octetPair,0,0,GT_FALSE,_4K - 19);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        transmittedPkt[16] = cid.octet[1];
        transmittedPkt[17] = cid.octet[0];

        /* Send packet and check egress as expected */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  extPortsBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* special enhanced UT restore configurations */
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedMcConfigRestore(dev, cid.octetPair, 0, _4K - 19);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);
    }
}

UTF_TEST_CASE_MAC(prvTgfPxHalBpe_U2E_MC_withSrcFiltering)
{
    GT_U8                              dev = 0;
    GT_STATUS                          st;
    GT_PORT_NUM                        extPort[EXT_PORTS_IN_MC_GRP] = {0, 3, 15, 6};
    GT_U32                             extPcid[EXT_PORTS_IN_MC_GRP] = {1, 2, 3, 4};
    CPSS_PX_PORTS_BMP                  extPortsBmp = 0;
    GT_PORT_NUM                        upPort = 12;
    GT_PORT_NUM                        portIndex;
    GT_U32                             packetLen = 100;
    GT_U32                             egressPacketLen[PRV_CPSS_PX_PORTS_NUM_CNS - 1];
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC ingressInterface, egressInterface[EXT_PORTS_IN_MC_GRP + 1];
    union                              {
        GT_U16                         octetPair;
        GT_U8                          octet[2];
    }cid;
    cid.octetPair = 99 + _4K;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(portIndex = 0; portIndex < EXT_PORTS_IN_MC_GRP; portIndex++) {
            PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(extPortsBmp, extPort[portIndex]);
            egressInterface[portIndex].type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
            egressInterface[portIndex].devPort.portNum = extPort[portIndex];
            egressPacketLen[extPort[portIndex]] = packetLen - BYTES_IN_E_TAG;
        }
        egressInterface[portIndex].type = CPSS_PX_HAL_BPE_INTERFACE_NONE_E;

        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = 0x01;
        transmittedPkt[1] = expectedPkt[1] = 0x00;
        transmittedPkt[2] = expectedPkt[2] = 0x5E;
        transmittedPkt[3] = expectedPkt[3] = 0x00;
        transmittedPkt[4] = expectedPkt[4] = dev + 1;
        transmittedPkt[5] = expectedPkt[5] = upPort + 1;

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* ingress E-Tag */
        transmittedPkt[12] = 0x89;
        transmittedPkt[13] = 0x3F;
        transmittedPkt[14] = 0x00;
        transmittedPkt[15] = 0x00;
        transmittedPkt[16] = cid.octet[1];
        transmittedPkt[17] = cid.octet[0];
        transmittedPkt[18] = 0x00;
        transmittedPkt[19] = 0x00;

        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Configure device */
        ingressInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        ingressInterface.devPort.devNum = dev;
        ingressInterface.devPort.portNum = upPort;

        st = cpssPxHalBpeNumOfChannelsSet(dev, 100, _4K);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, 100, _4K);

        st = cpssPxHalBpeInterfaceTypeSet(dev, &ingressInterface, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, ingressInterface.type, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);

        for(portIndex = 0; portIndex < EXT_PORTS_IN_MC_GRP; portIndex++) {
            egressInterface[portIndex].devPort.devNum = dev;

            st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface[portIndex], CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface[portIndex].type, egressInterface[portIndex].devPort.devNum, egressInterface[portIndex].devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);

            st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &egressInterface[portIndex], extPcid[portIndex]);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, egressInterface[portIndex].devPort.devNum, egressInterface[portIndex].devPort.portNum, extPcid[portIndex]);
        }

        st = cpssPxHalBpeMulticastEChannelCreate(dev, cid.octetPair, egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        for(portIndex = 0; portIndex < EXT_PORTS_IN_MC_GRP; portIndex++) {
            st = cpssPxHalBpePortExtendedUpstreamSet(dev, extPort[portIndex], &ingressInterface);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort[portIndex]);
        }

        /* For every extended port in the MC group, set its PCID as packet's ingress-ECID, so this port is filtered. */
        for(portIndex = 0; portIndex < EXT_PORTS_IN_MC_GRP; portIndex++) {

            /* special enhanced UT configurations*/
            st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedMcConfigSet(dev, cid.octetPair, extPcid[portIndex], 0, GT_FALSE, 100);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, cid.octetPair, extPcid[portIndex]);

            transmittedPkt[15] = extPcid[portIndex];

            PRV_CPSS_PX_HAL_PORTS_BMP_PORT_CLEAR_MAC(extPortsBmp, extPort[portIndex]);

            /* Send packet and check egress as expected */
            st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                      7,
                                                                      &ingressInterface,
                                                                      extPortsBmp,
                                                                      GT_FALSE,
                                                                      transmittedPkt,
                                                                      expectedPkt,
                                                                      packetLen,
                                                                      egressPacketLen);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NO_MORE, st);
            PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(extPortsBmp, extPort[portIndex]);

            /* special enhanced UT restore configurations */
            st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedMcConfigRestore(dev, cid.octetPair, extPcid[portIndex], 100);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPcid[portIndex]);
        }
    }
}

UTF_TEST_CASE_MAC(prvTgfPxHalBpe_U2C_UC)
{
    GT_U8                              dev = 0;
    GT_STATUS                          st;
    GT_PORT_NUM                        downPort = 0;
    CPSS_PX_PORTS_BMP                  downPortBmp = 0;
    GT_PORT_NUM                        upPort = 12;
    GT_U32                             packetLen = 100;
    GT_U32                             egressPacketLen[PRV_CPSS_PX_PORTS_NUM_CNS - 1];
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC egressInterface, ingressInterface;
    union                              {
        GT_U16                         octetPair;
        GT_U8                          octet[2];
    }ecid;
    union                              {
        GT_U16                         octetPair;
        GT_U8                          octet[2];
    }pcid;
    ecid.octetPair = 1;
    pcid.octetPair = 2;

    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(downPortBmp, downPort);

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Perform common configuration */
        ingressInterface.type = egressInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        ingressInterface.devPort.devNum = egressInterface.devPort.devNum = dev;
        ingressInterface.devPort.portNum = upPort;
        egressInterface.devPort.portNum = downPort;

        st = cpssPxHalBpeInterfaceTypeSet(dev, &ingressInterface, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, ingressInterface.type, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);

        st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface, CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface.type, egressInterface.devPort.devNum, egressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E);

        /* Set numOfUnicastChannels == 4K - 20, numOfMulticastChannels == 4K,
           Configure unicast E-channel with cid == 1 */

        st = cpssPxHalBpeNumOfChannelsSet(dev, _4K - 20, _4K);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, _4K - 20, _4K);

        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &egressInterface, pcid.octetPair);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, egressInterface.devPort.devNum, egressInterface.devPort.portNum, pcid.octetPair);

        st = cpssPxHalBpeUnicastEChannelCreate(dev, ecid.octetPair, &egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ecid.octetPair);

        st = cpssPxHalBpeUnicastEChannelCreate(dev, pcid.octetPair, &egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, pcid.octetPair);

        st = cpssPxHalBpePortExtendedUpstreamSet(dev, downPort, &ingressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, downPort);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2CascadeUcConfigSet(dev, pcid.octetPair, pcid.octetPair);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, pcid.octetPair, pcid.octetPair);

        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = 0x00;
        transmittedPkt[1] = expectedPkt[1] = 0x00;
        transmittedPkt[2] = expectedPkt[2] = 0x00;
        transmittedPkt[3] = expectedPkt[3] = 0x00;
        transmittedPkt[4] = expectedPkt[4] = dev + 1;
        transmittedPkt[5] = expectedPkt[5] = upPort + 1;

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* ingress E-Tag */
        transmittedPkt[12] = 0x89;
        transmittedPkt[13] = 0x3F;
        transmittedPkt[14] = 0x00;
        transmittedPkt[15] = 0x00;
        transmittedPkt[16] = pcid.octet[1];
        transmittedPkt[17] = pcid.octet[0];
        transmittedPkt[18] = 0x00;
        transmittedPkt[19] = 0x00;

        egressPacketLen[downPort] = packetLen - BYTES_IN_E_TAG;

        /* 1.1   Send packet with ECID == PCID of cascade port, we expect it to egress untagged */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  downPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* special enhanced UT restore configurations */
        st = prvTgfPxHalBpeEnhancedUtUpstream2CascadeUcConfigRestore(dev, pcid.octetPair);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, pcid.octetPair);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2CascadeUcConfigSet(dev, ecid.octetPair, pcid.octetPair);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, ecid.octetPair, pcid.octetPair);

        /* ingress + egress E-Tag */
        transmittedPkt[12] = expectedPkt[12] = 0x89;
        transmittedPkt[13] = expectedPkt[13] = 0x3F;
        transmittedPkt[14] = expectedPkt[14] = 0x00;
        transmittedPkt[15] = expectedPkt[15] = 0x00;
        transmittedPkt[16] = expectedPkt[16] = ecid.octet[1];
        transmittedPkt[17] = expectedPkt[17] = ecid.octet[0];
        transmittedPkt[18] = expectedPkt[18] = 0x00;
        transmittedPkt[19] = expectedPkt[19] = 0x00;

        egressPacketLen[downPort] = packetLen;

        /* 1.2   Send packet with ECID != PCID of cascade port, we expect it to egress tagged */

        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  downPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* special enhanced UT restore configurations */
        st = prvTgfPxHalBpeEnhancedUtUpstream2CascadeUcConfigRestore(dev, ecid.octetPair);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ecid.octetPair);
    }
}

UTF_TEST_CASE_MAC(prvTgfPxHalBpe_U2EC_UC_typeSwitch)
{
    GT_U8                              dev = 0;
    GT_STATUS                          st;
    GT_PORT_NUM                        downPort = 0;
    CPSS_PX_PORTS_BMP                  downPortBmp = 0;
    GT_PORT_NUM                        upPort = 12;
    GT_U32                             packetLen = 100;
    GT_U32                             egressPacketLen[PRV_CPSS_PX_PORTS_NUM_CNS - 1];
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC egressInterface, ingressInterface;
    union                              {
        GT_U16                         octetPair;
        GT_U8                          octet[2];
    }ecid;
    union                              {
        GT_U16                         octetPair;
        GT_U8                          octet[2];
    }pcid;
    ecid.octetPair = 1;
    pcid.octetPair = 2;

    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(downPortBmp, downPort);

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Perform common configuration */
        ingressInterface.type = egressInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        ingressInterface.devPort.devNum = egressInterface.devPort.devNum = dev;
        ingressInterface.devPort.portNum = upPort;
        egressInterface.devPort.portNum = downPort;

        /*  1.1  Set downstream port as extended */

        st = cpssPxHalBpeInterfaceTypeSet(dev, &ingressInterface, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, ingressInterface.type, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);

        st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface.type, egressInterface.devPort.devNum, egressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);

        st = cpssPxHalBpeNumOfChannelsSet(dev, _4K - 20, _4K);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, _4K - 20, _4K);

        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &egressInterface, pcid.octetPair);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, egressInterface.devPort.devNum, egressInterface.devPort.portNum, pcid.octetPair);

        st = cpssPxHalBpeUnicastEChannelCreate(dev, pcid.octetPair, &egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, pcid.octetPair);

        st = cpssPxHalBpePortExtendedUpstreamSet(dev, downPort, &ingressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, downPort);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigSet(dev, pcid.octetPair, 0, GT_FALSE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, pcid.octetPair);

        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = 0x00;
        transmittedPkt[1] = expectedPkt[1] = 0x00;
        transmittedPkt[2] = expectedPkt[2] = 0x00;
        transmittedPkt[3] = expectedPkt[3] = 0x00;
        transmittedPkt[4] = expectedPkt[4] = dev + 1;
        transmittedPkt[5] = expectedPkt[5] = upPort + 1;

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* ingress E-Tag */
        transmittedPkt[12] = 0x89;
        transmittedPkt[13] = 0x3F;
        transmittedPkt[14] = 0x00;
        transmittedPkt[15] = 0x00;
        transmittedPkt[16] = pcid.octet[1];
        transmittedPkt[17] = pcid.octet[0];
        transmittedPkt[18] = 0x00;
        transmittedPkt[19] = 0x00;

        egressPacketLen[downPort] = packetLen - BYTES_IN_E_TAG;

        /* 1.1.1  Send packet with ECID == PCID of extended port, we expect it to egress untagged */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  downPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        transmittedPkt[16] = ecid.octet[1];
        transmittedPkt[17] = ecid.octet[0];

        /* 1.1.2  Send packet with ECID != PCID of extended port, we expect it to be dropped */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  0,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NO_MORE, st);

        /* special enhanced UT restore configurations */
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigRestore(dev, ecid.octetPair);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ecid.octetPair);

        /*  1.2  Set downstream port as cascade */

        st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface, CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface.type, egressInterface.devPort.devNum, egressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E);

        st = cpssPxHalBpeUnicastEChannelCreate(dev, ecid.octetPair, &egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ecid.octetPair);

        st = cpssPxHalBpePortExtendedUpstreamSet(dev, downPort, &ingressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, downPort);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2CascadeUcConfigSet(dev,pcid.octetPair,pcid.octetPair);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, pcid.octetPair, pcid.octetPair);

        /* ingress E-Tag */
        transmittedPkt[12] = 0x89;
        transmittedPkt[13] = 0x3F;
        transmittedPkt[14] = 0x00;
        transmittedPkt[15] = 0x00;
        transmittedPkt[16] = pcid.octet[1];
        transmittedPkt[17] = pcid.octet[0];
        transmittedPkt[18] = 0x00;
        transmittedPkt[19] = 0x00;

        egressPacketLen[downPort] = packetLen - BYTES_IN_E_TAG;

        /* 1.2.1  Send packet with ECID == PCID of extended port, we expect it to egress untagged */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  downPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* special enhanced UT restore configurations */
        st = prvTgfPxHalBpeEnhancedUtUpstream2CascadeUcConfigRestore(dev,pcid.octetPair);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, pcid.octetPair);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2CascadeUcConfigSet(dev,ecid.octetPair,pcid.octetPair);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, ecid.octetPair, pcid.octetPair);

        /* ingress + egress E-Tag */
        transmittedPkt[12] = expectedPkt[12] = 0x89;
        transmittedPkt[13] = expectedPkt[13] = 0x3F;
        transmittedPkt[14] = expectedPkt[14] = 0xF0;
        transmittedPkt[15] = expectedPkt[15] = 0x00;
        transmittedPkt[16] = expectedPkt[16] = ecid.octet[1];
        transmittedPkt[17] = expectedPkt[17] = ecid.octet[0];
        transmittedPkt[18] = expectedPkt[18] = 0x00;
        transmittedPkt[19] = expectedPkt[19] = 0x00;

        egressPacketLen[downPort] = packetLen;

        /* 1.2.2  Send packet with ECID != PCID of cascade port, we expect it to egress tagged */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  downPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* special enhanced UT restore configurations */
        st = prvTgfPxHalBpeEnhancedUtUpstream2CascadeUcConfigRestore(dev, pcid.octetPair);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, pcid.octetPair);

        /*  1.3  Set downstream port as extended */

        st = cpssPxHalBpeUnicastEChannelDelete(dev, ecid.octetPair);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ecid.octetPair);

        st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface.type, egressInterface.devPort.devNum, egressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);

        st = cpssPxHalBpePortExtendedUpstreamSet(dev, downPort, &ingressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, downPort);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigSet(dev, pcid.octetPair, 0, GT_FALSE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, pcid.octetPair);

        /* ingress E-Tag */
        transmittedPkt[12] = 0x89;
        transmittedPkt[13] = 0x3F;
        transmittedPkt[14] = 0x00;
        transmittedPkt[15] = 0x00;
        transmittedPkt[16] = pcid.octet[1];
        transmittedPkt[17] = pcid.octet[0];
        transmittedPkt[18] = 0x00;
        transmittedPkt[19] = 0x00;

        /* egress E-Tag */
        expectedPkt[12] = 0xFF;
        expectedPkt[13] = 0xFF;
        expectedPkt[14] = 0xFF;
        expectedPkt[15] = 0xFF;
        expectedPkt[16] = 0xFF;
        expectedPkt[17] = 0xFF;
        expectedPkt[18] = 0xFF;
        expectedPkt[19] = 0xFF;

        egressPacketLen[downPort] = packetLen - BYTES_IN_E_TAG;

        /* 1.3.1  Send packet with ECID == PCID of extended port, we expect it to egress untagged */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  downPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        transmittedPkt[16] = ecid.octet[1];
        transmittedPkt[17] = ecid.octet[0];

        /* 1.3.2  Send packet with ECID != PCID of extended port, we expect it to be dropped */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  0,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NO_MORE, st);

        /* special enhanced UT restore configurations */
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigRestore(dev, pcid.octetPair);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, pcid.octetPair);
    }
}

UTF_TEST_CASE_MAC(prvTgfPxHalBpe_U2C_MC)
{
    GT_U8                              dev = 0;
    GT_STATUS                          st;
    GT_PORT_NUM                        extPort[EXT_PORTS_IN_MC_GRP] = {0, 3, 15, 6};
    CPSS_PX_PORTS_BMP                  extPortsBmp = 0;
    GT_PORT_NUM                        upPort = 12;
    GT_PORT_NUM                        portIndex;
    GT_U32                             packetLen = 100;
    GT_U32                             egressPacketLen[PRV_CPSS_PX_PORTS_NUM_CNS - 1];
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC ingressInterface, egressInterface[EXT_PORTS_IN_MC_GRP + 1];
    union                              {
        GT_U16                         octetPair;
        GT_U8                          octet[2];
    }cid;    

    for(portIndex = 0; portIndex < EXT_PORTS_IN_MC_GRP; portIndex++) {
        PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(extPortsBmp, extPort[portIndex]);
        egressInterface[portIndex].type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        egressInterface[portIndex].devPort.portNum = extPort[portIndex];
        egressPacketLen[extPort[portIndex]] = packetLen;
    }
    egressInterface[portIndex].type = CPSS_PX_HAL_BPE_INTERFACE_NONE_E;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Configure device */
        ingressInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        ingressInterface.devPort.devNum = dev;
        ingressInterface.devPort.portNum = upPort;

        st = cpssPxHalBpeInterfaceTypeSet(dev, &ingressInterface, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, ingressInterface.type, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);

        for(portIndex = 0; portIndex < EXT_PORTS_IN_MC_GRP; portIndex++) {
            egressInterface[portIndex].devPort.devNum = dev;
            st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface[portIndex], CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface[portIndex].type, egressInterface[portIndex].devPort.devNum, egressInterface[portIndex].devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E);
        }

        /* 1.1  Set numOfUnicastChannels == 0, numOfMulticastChannels == 8K - 19 */
        st = cpssPxHalBpeNumOfChannelsSet(dev, 0, _8K - 19);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, 0, _8K - 19);

        /* 1.1.1 Configure multicast E-channel with cid == 4K and send packet */
        cid.octetPair = _4K;

        st = cpssPxHalBpeMulticastEChannelCreate(dev, cid.octetPair, egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        for(portIndex = 0; portIndex < EXT_PORTS_IN_MC_GRP; portIndex++) {
            st = cpssPxHalBpePortExtendedUpstreamSet(dev, extPort[portIndex], &ingressInterface);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort[portIndex]);
        }

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2CascadeMcConfigSet(dev, cid.octetPair, 0, 0);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = 0x01;
        transmittedPkt[1] = expectedPkt[1] = 0x00;
        transmittedPkt[2] = expectedPkt[2] = 0x5E;
        transmittedPkt[3] = expectedPkt[3] = 0x00;
        transmittedPkt[4] = expectedPkt[4] = dev + 1;
        transmittedPkt[5] = expectedPkt[5] = upPort + 1;

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* ingress + egress E-Tag */
        transmittedPkt[12] = expectedPkt[12] = 0x89;
        transmittedPkt[13] = expectedPkt[13] = 0x3F;
        transmittedPkt[14] = expectedPkt[14] = 0x00;
        transmittedPkt[15] = expectedPkt[15] = 0x00;
        transmittedPkt[16] = expectedPkt[16] = cid.octet[1];
        transmittedPkt[17] = expectedPkt[17] = cid.octet[0];
        transmittedPkt[18] = expectedPkt[18] = 0x00;
        transmittedPkt[19] = expectedPkt[19] = 0x00;

        /* Send packet and check egress as expected */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  extPortsBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* special enhanced UT restore configurations */
        st = prvTgfPxHalBpeEnhancedUtUpstream2CascadeMcConfigRestore(dev, cid.octetPair, 0, 0);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);
    }
}

UTF_TEST_CASE_MAC(prvTgfPxHalBpe_U2C_MC_withSrcFiltering)
{
    GT_U8                              dev = 0;
    GT_STATUS                          st;
    GT_PORT_NUM                        extPort[EXT_PORTS_IN_MC_GRP] = {0, 3, 15, 6};
    GT_U32                             extPcid[EXT_PORTS_IN_MC_GRP] = {1, 2, 3, 4};
    CPSS_PX_PORTS_BMP                  extPortsBmp = 0;
    GT_PORT_NUM                        upPort = 12;
    GT_PORT_NUM                        portIndex;
    GT_U32                             packetLen = 100;
    GT_U32                             egressPacketLen[PRV_CPSS_PX_PORTS_NUM_CNS - 1];
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC ingressInterface, egressInterface[EXT_PORTS_IN_MC_GRP + 1];
    union                              {
        GT_U16                         octetPair;
        GT_U8                          octet[2];
    }cid;
    cid.octetPair = 99 + _4K;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(portIndex = 0; portIndex < EXT_PORTS_IN_MC_GRP; portIndex++) {
            PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(extPortsBmp, extPort[portIndex]);
            egressInterface[portIndex].type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
            egressInterface[portIndex].devPort.portNum = extPort[portIndex];
            egressPacketLen[extPort[portIndex]] = packetLen;
        }
        egressInterface[portIndex].type = CPSS_PX_HAL_BPE_INTERFACE_NONE_E;

        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = 0x01;
        transmittedPkt[1] = expectedPkt[1] = 0x00;
        transmittedPkt[2] = expectedPkt[2] = 0x5E;
        transmittedPkt[3] = expectedPkt[3] = 0x00;
        transmittedPkt[4] = expectedPkt[4] = dev + 1;
        transmittedPkt[5] = expectedPkt[5] = upPort + 1;

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* ingress + egress E-Tag */
        transmittedPkt[12] = expectedPkt[12] = 0x89;
        transmittedPkt[13] = expectedPkt[13] = 0x3F;
        transmittedPkt[14] = expectedPkt[14] = 0x00;
        transmittedPkt[15] = expectedPkt[15] = 0x00;
        transmittedPkt[16] = expectedPkt[16] = cid.octet[1];
        transmittedPkt[17] = expectedPkt[17] = cid.octet[0];
        transmittedPkt[18] = expectedPkt[18] = 0x00;
        transmittedPkt[19] = expectedPkt[19] = 0x00;

        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Configure device */
        ingressInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        ingressInterface.devPort.devNum = dev;
        ingressInterface.devPort.portNum = upPort;

        st = cpssPxHalBpeNumOfChannelsSet(dev, 100, _4K);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, 100, _4K);

        st = cpssPxHalBpeInterfaceTypeSet(dev, &ingressInterface, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, ingressInterface.type, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);

        for(portIndex = 0; portIndex < EXT_PORTS_IN_MC_GRP; portIndex++) {
            egressInterface[portIndex].devPort.devNum = dev;

            st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface[portIndex], CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface[portIndex].type, egressInterface[portIndex].devPort.devNum, egressInterface[portIndex].devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E);

            st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &egressInterface[portIndex], extPcid[portIndex]);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, egressInterface[portIndex].devPort.devNum, egressInterface[portIndex].devPort.portNum, extPcid[portIndex]);
        }

        st = cpssPxHalBpeMulticastEChannelCreate(dev, cid.octetPair, egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        for(portIndex = 0; portIndex < EXT_PORTS_IN_MC_GRP; portIndex++) {
            st = cpssPxHalBpePortExtendedUpstreamSet(dev, extPort[portIndex], &ingressInterface);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort[portIndex]);
        }

        /* For every cascade port in the MC group, set its PCID as packet's ingress-ECID, so this port is filtered. */
        for(portIndex = 0; portIndex < EXT_PORTS_IN_MC_GRP; portIndex++) {

            /* special enhanced UT configurations*/
            st = prvTgfPxHalBpeEnhancedUtUpstream2CascadeMcConfigSet(dev, cid.octetPair, extPcid[portIndex], 100);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, cid.octetPair, extPcid[portIndex]);

            transmittedPkt[15] = extPcid[portIndex];

            PRV_CPSS_PX_HAL_PORTS_BMP_PORT_CLEAR_MAC(extPortsBmp, extPort[portIndex]);

            /* Send packet and check egress as expected */
            st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                      7,
                                                                      &ingressInterface,
                                                                      extPortsBmp,
                                                                      GT_FALSE,
                                                                      transmittedPkt,
                                                                      expectedPkt,
                                                                      packetLen,
                                                                      egressPacketLen);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NO_MORE, st);

            PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(extPortsBmp, extPort[portIndex]);

            /* special enhanced UT restore configurations */
            st = prvTgfPxHalBpeEnhancedUtUpstream2CascadeMcConfigRestore(dev, cid.octetPair, extPcid[portIndex], 100);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, cid.octetPair, extPcid[portIndex]);
        }
    }
}

UTF_TEST_CASE_MAC(prvTgfPxHalBpe_U2EC_MC)
{
    GT_U8                              dev = 0;
    GT_STATUS                          st;
    GT_PORT_NUM                        extPort[EXT_PORTS_IN_MC_GRP] = {0, 3, 15, 6};
    CPSS_PX_PORTS_BMP                  extPortsBmp = 0;
    GT_PORT_NUM                        upPort = 12;
    GT_PORT_NUM                        portIndex;
    GT_U32                             packetLen = 100;
    GT_U32                             egressPacketLen[PRV_CPSS_PX_PORTS_NUM_CNS - 1];
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC ingressInterface, egressInterface[EXT_PORTS_IN_MC_GRP + 1];
    union                              {
        GT_U16                         octetPair;
        GT_U8                          octet[2];
    }cid;    

    for(portIndex = 0; portIndex < EXT_PORTS_IN_MC_GRP; portIndex++) {
        PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(extPortsBmp, extPort[portIndex]);
        egressInterface[portIndex].type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        egressInterface[portIndex].devPort.portNum = extPort[portIndex];
    }
    egressInterface[portIndex].type = CPSS_PX_HAL_BPE_INTERFACE_NONE_E;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(portIndex = 0; portIndex < EXT_PORTS_IN_MC_GRP; portIndex++) {
            egressInterface[portIndex].devPort.devNum = dev;
        }

        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Configure device */
        ingressInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        ingressInterface.devPort.devNum = dev;
        ingressInterface.devPort.portNum = upPort;

        st = cpssPxHalBpeInterfaceTypeSet(dev, &ingressInterface, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, ingressInterface.type, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);

        st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface[0], CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface[0].type, egressInterface[0].devPort.devNum, egressInterface[0].devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);
        
        st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface[1], CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface[1].type, egressInterface[1].devPort.devNum, egressInterface[1].devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);

        st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface[2], CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface[2].type, egressInterface[2].devPort.devNum, egressInterface[2].devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E);

        st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface[3], CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface[3].type, egressInterface[3].devPort.devNum, egressInterface[3].devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E);

        /* 1.1  Set numOfUnicastChannels == 20, numOfMulticastChannels == 8K - 19 - 20 */
        st = cpssPxHalBpeNumOfChannelsSet(dev, 20, _8K - 19 - 20);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, 20, _8K - 19 - 20);

        /* 1.1.1 Configure multicast E-channel with cid == 4K and send packet */
        cid.octetPair = _4K+21;

        st = cpssPxHalBpeMulticastEChannelCreate(dev, cid.octetPair, egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        for(portIndex = 0; portIndex < EXT_PORTS_IN_MC_GRP; portIndex++) {
            st = cpssPxHalBpePortExtendedUpstreamSet(dev, extPort[portIndex], &ingressInterface);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort[portIndex]);
        }

        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &egressInterface[0], extPort[0]+1);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, egressInterface[0].devPort.devNum, egressInterface[0].devPort.portNum, extPort[0]+1);

        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &egressInterface[1], extPort[1]+1);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, egressInterface[1].devPort.devNum, egressInterface[1].devPort.portNum, extPort[1]+1);

        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = 0x01;
        transmittedPkt[1] = expectedPkt[1] = 0x00;
        transmittedPkt[2] = expectedPkt[2] = 0x5E;
        transmittedPkt[3] = expectedPkt[3] = 0x00;
        transmittedPkt[4] = expectedPkt[4] = dev + 1;
        transmittedPkt[5] = expectedPkt[5] = upPort + 1;

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* ingress E-Tag */
        transmittedPkt[12] = 0x89;
        transmittedPkt[13] = 0x3F;
        transmittedPkt[14] = 0x00;
        transmittedPkt[15] = 0x00;
        transmittedPkt[16] = cid.octet[1];
        transmittedPkt[17] = cid.octet[0];
        transmittedPkt[18] = 0x00;
        transmittedPkt[19] = 0x00;

        egressPacketLen[extPort[0]] = packetLen - BYTES_IN_E_TAG;
        egressPacketLen[extPort[1]] = packetLen - BYTES_IN_E_TAG;
        egressPacketLen[extPort[2]] = packetLen;
        egressPacketLen[extPort[3]] = packetLen;

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedMcConfigSet(dev, cid.octetPair, 0, 0, GT_FALSE, 20);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* Send packet and check egress as expected */
        /* trap packet received from extended port ignore ERROR for packet length for cascade ports */
        ignoreErrorOnPacketLength = GT_TRUE;
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  extPortsBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* special enhanced UT restore configurations */
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedMcConfigRestore(dev, cid.octetPair, 0, 20);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2CascadeMcConfigSet(dev, cid.octetPair, 0, 20);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* egress E-Tag */
        expectedPkt[12] = 0x89;
        expectedPkt[13] = 0x3F;
        expectedPkt[14] = 0x00;
        expectedPkt[15] = 0x00;
        expectedPkt[16] = cid.octet[1];
        expectedPkt[17] = cid.octet[0];
        expectedPkt[18] = 0x00;
        expectedPkt[19] = 0x00;

        /* Send packet and check egress as cascade */
        /* trap packet received from cascade port ignore ERROR for packet length for extended ports */
        ignoreErrorOnPacketLength = GT_TRUE;
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  extPortsBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* special enhanced UT restore configurations */
        st = prvTgfPxHalBpeEnhancedUtUpstream2CascadeMcConfigRestore(dev, cid.octetPair, 0, 20);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        ignoreErrorOnPacketLength = GT_FALSE;
    }
}

UTF_TEST_CASE_MAC(prvTgfPxHalBpe_E2U_UC)
{
    GT_U8                              dev = 0;
    GT_STATUS                          st;
    GT_PORT_NUM                        extPort = 0;
    CPSS_PX_PORTS_BMP                  upPortBmp = 0;
    GT_PORT_NUM                        upPort = 12;
    GT_U32                             packetLen = 100;
    GT_U32                             egressPacketLen[PRV_CPSS_PX_PORTS_NUM_CNS - 1];
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC egressInterface, ingressInterface;
    union                              {
        GT_U16                         octetPair;
        GT_U8                          octet[2];
    }cid;

    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(upPortBmp, upPort);

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Perform common configuration */
        ingressInterface.type = egressInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        ingressInterface.devPort.devNum = egressInterface.devPort.devNum = dev;
        ingressInterface.devPort.portNum = extPort;
        egressInterface.devPort.portNum = upPort;

        st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface.type, egressInterface.devPort.devNum, egressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);

        st = cpssPxHalBpeInterfaceTypeSet(dev, &ingressInterface, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, ingressInterface.type, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);

        /* 1.1   Set numOfUnicastChannels == 4K - 20, numOfMulticastChannels == 4K,
           Configure unicast E-channel with cid == 1 and send packet */
        cid.octetPair = 1;

        st = cpssPxHalBpeNumOfChannelsSet(dev, _4K - 20, _4K);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, _4K - 20, _4K);

        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &ingressInterface, cid.octetPair);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, cid.octetPair);

        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid.octetPair, &ingressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        st = cpssPxHalBpePortExtendedUpstreamSet(dev, extPort, &egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtExtended2UpstreamConfigSet(dev, extPort);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort);

        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = 0x00;
        transmittedPkt[1] = expectedPkt[1] = 0x00;
        transmittedPkt[2] = expectedPkt[2] = 0x00;
        transmittedPkt[3] = expectedPkt[3] = 0x00;
        transmittedPkt[4] = expectedPkt[4] = dev + 1;
        transmittedPkt[5] = expectedPkt[5] = extPort + 1;

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* egress E-Tag */
        expectedPkt[12] = 0x89;
        expectedPkt[13] = 0x3F;
        expectedPkt[14] = 0x00;
        expectedPkt[15] = 0x00;
        expectedPkt[16] = cid.octet[1];
        expectedPkt[17] = cid.octet[0];
        expectedPkt[18] = 0x00;
        expectedPkt[19] = 0x00;

        egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

        /* Send packet and check egress as expected */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  upPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssPxHalBpeUnicastEChannelDelete(dev, cid.octetPair);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        /* special enhanced UT restore configurations*/
        st = prvTgfPxHalBpeEnhancedUtExtended2UpstreamConfigRestore(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2   Set numOfUnicastChannels == 4K - 20, numOfMulticastChannels == 4K,
           Configure unicast E-channel with cid == 4K - 21 and send packet */
        cid.octetPair = _4K - 21;

        st = cpssPxHalBpeNumOfChannelsSet(dev, _4K - 20, _4K);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, _4K - 20, _4K);

        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &ingressInterface, cid.octetPair);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, cid.octetPair);

        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid.octetPair, &ingressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        st = cpssPxHalBpePortExtendedUpstreamSet(dev, extPort, &egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtExtended2UpstreamConfigSet(dev, extPort);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort);

        /* Send packet and check egress as expected */
        expectedPkt[16] = cid.octet[1];
        expectedPkt[17] = cid.octet[0];
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  upPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* special enhanced UT restore configurations*/
        st = prvTgfPxHalBpeEnhancedUtExtended2UpstreamConfigRestore(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

UTF_TEST_CASE_MAC(prvTgfPxHalBpe_E2U_UC_upstreamTrunk)
{
    GT_U8                              dev = 0;
    GT_STATUS                          st;
    GT_PORT_NUM                        extPort = 0;
    CPSS_PX_PORTS_BMP                  upPortBmp = 0;
    GT_PORT_NUM                        upPort[3] = {12, 1, 5};
    GT_PORT_NUM                        portIndex;
    GT_U32                             packetLen = 100;
    GT_U32                             trunkId = 6;
    GT_U32                             egressPacketLen[PRV_CPSS_PX_PORTS_NUM_CNS - 1];
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC egressInterface[3], ingressInterface, trunkInterface;
    union                              {
        GT_U16                         octetPair;
        GT_U8                          octet[2];
    }cid;
    cid.octetPair = 1;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Perform common configuration */
        trunkInterface.type = CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E;
        trunkInterface.trunkId = trunkId;

        for (portIndex = 0; portIndex < 3; portIndex++) {
            PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(upPortBmp, upPort[portIndex]);
            egressInterface[portIndex].type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
            egressInterface[portIndex].devPort.devNum = dev;
            egressInterface[portIndex].devPort.portNum = upPort[portIndex];
            st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface[portIndex], CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface[portIndex].type, egressInterface[portIndex].devPort.devNum, egressInterface[portIndex].devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);
        }

        ingressInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        ingressInterface.devPort.devNum = dev;
        ingressInterface.devPort.portNum = extPort;
        st = cpssPxHalBpeInterfaceTypeSet(dev, &ingressInterface, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, ingressInterface.type, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);

        /* 1.1  Create upstream trunk with 3 ports, transmit untagged packet to extended port and
           expect the packet to egress tagged out of one of the trunk ports. */

        /* Create upstream trunk */
        st = cpssPxHalBpeTrunkCreate(dev, trunkId, CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, trunkId, CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E);

        /* Add 3 upstream ports to upstream trunk */
        st = cpssPxHalBpeTrunkPortsAdd(dev, trunkId, egressInterface, 3);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        st = cpssPxHalBpeNumOfChannelsSet(dev, _4K - 20, _4K);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, _4K - 20, _4K);

        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &ingressInterface, cid.octetPair);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, cid.octetPair);

        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid.octetPair, &ingressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        st = cpssPxHalBpePortExtendedUpstreamSet(dev, extPort, &trunkInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtExtended2UpstreamConfigSet(dev,extPort);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort);

        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = 0x00;
        transmittedPkt[1] = expectedPkt[1] = 0x00;
        transmittedPkt[2] = expectedPkt[2] = 0x00;
        transmittedPkt[3] = expectedPkt[3] = 0x00;
        transmittedPkt[4] = expectedPkt[4] = dev + 1;
        transmittedPkt[5] = expectedPkt[5] = extPort + 1;

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* egress E-Tag */
        expectedPkt[12] = 0x89;
        expectedPkt[13] = 0x3F;
        expectedPkt[14] = 0x00;
        expectedPkt[15] = 0x00;
        expectedPkt[16] = cid.octet[1];
        expectedPkt[17] = cid.octet[0];
        expectedPkt[18] = 0x00;
        expectedPkt[19] = 0x00;

        egressPacketLen[0] = packetLen + BYTES_IN_E_TAG;

        /* Send packet and check egress as expected */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  upPortBmp,
                                                                  GT_TRUE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2  Remove one of the ports from the trunk, transmit untagged packet to extended port and
           expect the packet to egress tagged out of one of the remaining trunk ports. */

        /* Remove first trunk port */
        st = cpssPxHalBpeTrunkPortsDelete(dev, trunkId, egressInterface, 1);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        PRV_CPSS_PX_HAL_PORTS_BMP_PORT_CLEAR_MAC(upPortBmp, upPort[0]);

        /* Send packet and check egress as expected */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  upPortBmp,
                                                                  GT_TRUE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.3  Remove another one of the ports from the trunk, transmit untagged packet to extended port and
           expect the packet to egress tagged out of the remaining trunk port. */

        /* Remove second trunk port */
        st = cpssPxHalBpeTrunkPortsDelete(dev, trunkId, egressInterface + 1, 1);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        PRV_CPSS_PX_HAL_PORTS_BMP_PORT_CLEAR_MAC(upPortBmp, upPort[1]);

        /* Send packet and check egress as expected */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  upPortBmp,
                                                                  GT_TRUE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.4  Remove the last port from the trunk, transmit untagged packet to extended port and
           expect the packet to be dropped. */

        /* Remove last trunk port */
        st = cpssPxHalBpeTrunkPortsDelete(dev, trunkId, egressInterface + 2, 1);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* Send packet and check egress as expected */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  0,
                                                                  GT_TRUE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* special enhanced UT restore configurations*/
        st = prvTgfPxHalBpeEnhancedUtExtended2UpstreamConfigRestore(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

UTF_TEST_CASE_MAC(prvTgfPxHalBpe_E2U_UC_upstreamTrunk_withLoadBalanceMode)
{
    GT_U8                              dev = 0;
    GT_STATUS                          st;
    CPSS_PX_PORTS_BMP                  upPortBmp;
    GT_PORT_NUM                        upPortOrderPort[4] = {4, 6, 7, 5};
    GT_PORT_NUM                        upPortOrderMac[4] = {7, 4, 5, 5};
    GT_PORT_NUM                        upPort[4] = {4, 5, 6, 7};
    GT_PORT_NUM                        extPort[4] = {0, 1, 2, 3};
    GT_U32                             extPcid[4] = {10, 11, 12, 13};
    GT_PORT_NUM                        portIndex, egressPort;
    GT_U32                             packetLen = 100;
    GT_U32                             trunkId = 6;
    GT_U32                             egressPacketLen[PRV_CPSS_PX_PORTS_NUM_CNS - 1];
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC egressInterface[4], ingressInterface[4], trunkInterface;
    union                              {
        GT_U16                         octetPair;
        GT_U8                          octet[2];
    }cid;
    cid.octetPair = 1;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Perform common configuration */
        trunkInterface.type = CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E;
        trunkInterface.trunkId = trunkId;

        st = cpssPxHalBpeNumOfChannelsSet(dev, _4K - 20, _4K);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, _4K - 20, _4K);

        for (portIndex = 0; portIndex < 4; portIndex++) {
            /* Configure upstream port */

            PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(upPortBmp, upPort[portIndex]);

            egressInterface[portIndex].type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
            egressInterface[portIndex].devPort.devNum = dev;
            egressInterface[portIndex].devPort.portNum = upPort[portIndex];
            st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface[portIndex], CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);

            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface[portIndex].type, egressInterface[portIndex].devPort.devNum, egressInterface[portIndex].devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);

            /* Configure extended port */

            ingressInterface[portIndex].type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
            ingressInterface[portIndex].devPort.devNum = dev;
            ingressInterface[portIndex].devPort.portNum = extPort[portIndex];

            st = cpssPxHalBpeInterfaceTypeSet(dev, &ingressInterface[portIndex], CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, ingressInterface[portIndex].type, ingressInterface[portIndex].devPort.devNum, ingressInterface[portIndex].devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);

            st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &ingressInterface[portIndex], extPcid[portIndex]);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ingressInterface[portIndex].devPort.devNum, ingressInterface[portIndex].devPort.portNum, extPcid[portIndex]);

            /* Configure unicast E-channel */
            st = cpssPxHalBpeUnicastEChannelCreate(dev, extPcid[portIndex], &ingressInterface[portIndex]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPcid[portIndex]);
        }

        /* Create upstream trunk */
        st = cpssPxHalBpeTrunkCreate(dev, trunkId, CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, trunkId, CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E);

        /* Add 4 upstream ports to upstream trunk */
        st = cpssPxHalBpeTrunkPortsAdd(dev, trunkId, egressInterface, 4);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        for(portIndex = 0; portIndex < 4; portIndex++) {
            /* Set trunk as upstream interface of extended ports */
            st = cpssPxHalBpePortExtendedUpstreamSet(dev, extPort[portIndex], &trunkInterface);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort[portIndex]);
        }

        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = 0x00;
        transmittedPkt[1] = expectedPkt[1] = 0x00;
        transmittedPkt[2] = expectedPkt[2] = 0x00;
        transmittedPkt[3] = expectedPkt[3] = 0x00;
        transmittedPkt[4] = expectedPkt[4] = dev + 1;
        transmittedPkt[5] = expectedPkt[5] = extPort[0] + 1;

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* egress E-Tag */
        expectedPkt[12] = 0x89;
        expectedPkt[13] = 0x3F;
        expectedPkt[14] = 0x00;
        expectedPkt[15] = 0x00;
        expectedPkt[16] = cid.octet[1];
        expectedPkt[17] = cid.octet[0];
        expectedPkt[18] = 0x00;
        expectedPkt[19] = 0x00;

        egressPacketLen[0] = packetLen + BYTES_IN_E_TAG;

        /* 1. Set trunk load balance mode to port based */
        st = cpssPxHalBpeTrunkLoadBalanceModeSet(dev, CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_PORT_BASE_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_PORT_BASE_E);

        for(portIndex = 0; portIndex < 4; portIndex++) {
            egressPort = upPortOrderPort[portIndex];
            /* special enhanced UT configurations*/
            st = prvTgfPxHalBpeEnhancedUtExtended2UpstreamConfigSet(dev, extPort[portIndex]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort[portIndex]);

            egressPacketLen[egressPort] = packetLen + BYTES_IN_E_TAG;
            upPortBmp = 0;
            PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(upPortBmp, egressPort);
            cid.octetPair = extPcid[portIndex];
            expectedPkt[16] = cid.octet[1];
            expectedPkt[17] = cid.octet[0];

            /* Send packet and check egress as expected */
            st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                      7,
                                                                      &ingressInterface[portIndex],
                                                                      upPortBmp,
                                                                      GT_FALSE,
                                                                      transmittedPkt,
                                                                      expectedPkt,
                                                                      packetLen,
                                                                      egressPacketLen);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        }

        /* 2. Set trunk load balance mode to MAC based */
        st = cpssPxHalBpeTrunkLoadBalanceModeSet(dev, CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_MAC_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_MAC_E);

        for(portIndex = 0; portIndex < 4; portIndex++) {
            egressPort = upPortOrderMac[portIndex];

            egressPacketLen[egressPort] = packetLen + BYTES_IN_E_TAG;
            upPortBmp = 0;
            PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(upPortBmp, egressPort);

            /* ingress + egress MAC DA */
            transmittedPkt[0] = expectedPkt[0]  = 0x00;
            transmittedPkt[1] = expectedPkt[1]  = upPort[portIndex] + 3;
            transmittedPkt[2] = expectedPkt[2]  = extPort[portIndex] + 3;
            transmittedPkt[3] = expectedPkt[3]  = upPortOrderPort[portIndex] + 3;
            transmittedPkt[4] = expectedPkt[4]  = dev + 1;
            transmittedPkt[5] = expectedPkt[5]  = extPcid[portIndex] + 3;

            /* ingress + egress MAC SA */
            transmittedPkt[6]  = expectedPkt[6]  = 0x00;
            transmittedPkt[7]  = expectedPkt[7]  = upPort[portIndex] + 1;
            transmittedPkt[8]  = expectedPkt[8]  = extPort[portIndex] + 1;
            transmittedPkt[9]  = expectedPkt[9]  = upPortOrderPort[portIndex] + 1;
            transmittedPkt[10] = expectedPkt[10] = extPcid[portIndex] + 1;
            transmittedPkt[11] = expectedPkt[11] = 0x99;

            cid.octetPair = extPcid[0];
            /* egress E-Tag */
            expectedPkt[12] = 0x89;
            expectedPkt[13] = 0x3F;
            expectedPkt[14] = 0x00;
            expectedPkt[15] = 0x00;
            expectedPkt[16] = cid.octet[1];
            expectedPkt[17] = cid.octet[0];
            expectedPkt[18] = 0x00;
            expectedPkt[19] = 0x00;

            /* Send packet and check egress as expected */
            st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                      7,
                                                                      &ingressInterface[0],
                                                                      upPortBmp,
                                                                      GT_FALSE,
                                                                      transmittedPkt,
                                                                      expectedPkt,
                                                                      packetLen,
                                                                      egressPacketLen);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        }

        /* special enhanced UT restore configurations*/
        st = prvTgfPxHalBpeEnhancedUtExtended2UpstreamConfigRestore(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

UTF_TEST_CASE_MAC(prvTgfPxHalBpe_C2U_UC)
{
    GT_U8                              dev = 0;
    GT_STATUS                          st;
    GT_PORT_NUM                        cscdPort = 0;
    CPSS_PX_PORTS_BMP                  upPortBmp = 0;
    GT_PORT_NUM                        upPort = 12;
    GT_U32                             packetLen = 100;
    GT_U32                             egressPacketLen[PRV_CPSS_PX_PORTS_NUM_CNS - 1];
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC egressInterface, ingressInterface;
    union                              {
        GT_U16                         octetPair;
        GT_U8                          octet[2];
    }cid;
    union                              {
        GT_U16                         octetPair;
        GT_U8                          octet[2];
    }pcid;
    cid.octetPair = 1;
    pcid.octetPair = 2;

    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(upPortBmp, upPort);

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Perform common configuration */
        ingressInterface.type = egressInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        ingressInterface.devPort.devNum = egressInterface.devPort.devNum = dev;
        ingressInterface.devPort.portNum = cscdPort;
        egressInterface.devPort.portNum = upPort;

        st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface.type, egressInterface.devPort.devNum, egressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);

        st = cpssPxHalBpeInterfaceTypeSet(dev, &ingressInterface, CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, ingressInterface.type, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E);

        st = cpssPxHalBpeNumOfChannelsSet(dev, _4K - 20, _4K);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, _4K - 20, _4K);

        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &ingressInterface, pcid.octetPair);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, pcid.octetPair);

        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid.octetPair, &ingressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        st = cpssPxHalBpePortExtendedUpstreamSet(dev, cscdPort, &egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cscdPort);

        /* special enhanced UT configurations */
        st = prvTgfPxHalBpeEnhancedUtExtended2UpstreamConfigSet(dev, cscdPort);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cscdPort);

        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = 0x00;
        transmittedPkt[1] = expectedPkt[1] = 0x00;
        transmittedPkt[2] = expectedPkt[2] = 0x00;
        transmittedPkt[3] = expectedPkt[3] = 0x00;
        transmittedPkt[4] = expectedPkt[4] = dev + 1;
        transmittedPkt[5] = expectedPkt[5] = cscdPort + 1;

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* 1.1 transmit un-E-tagged packet to cascade port, expect it to egress E-tagged out of upstream
           with  E-CID = PCID */

        /* egress E-Tag */
        expectedPkt[12] = 0x89;
        expectedPkt[13] = 0x3F;
        expectedPkt[14] = 0x00;
        expectedPkt[15] = 0x00;
        expectedPkt[16] = pcid.octet[1];
        expectedPkt[17] = pcid.octet[0];
        expectedPkt[18] = 0x00;
        expectedPkt[19] = 0x00;

        egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

        BPE_LOG_INFORMATION_MAC("Transmit untagged packet to cascade port %d.", cscdPort);
        /* Send packet and check egress as expected */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  upPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 transmit E-tagged packet to cascade port, expect it to egress E-tagged out of upstream
           with original E-CID */
        /* ingress + egress E-Tag */
        transmittedPkt[12] = expectedPkt[12] = 0x89;
        transmittedPkt[13] = expectedPkt[13] = 0x3F;
        transmittedPkt[14] = expectedPkt[14] = 0x00;
        transmittedPkt[15] = expectedPkt[15] = 0x00;
        transmittedPkt[16] = expectedPkt[16] = cid.octet[1];
        transmittedPkt[17] = expectedPkt[17] = cid.octet[0];
        transmittedPkt[18] = expectedPkt[18] = 0x00;
        transmittedPkt[19] = expectedPkt[19] = 0x00;

        egressPacketLen[upPort] = packetLen;

        /* Send packet and check egress as expected */
        BPE_LOG_INFORMATION_MAC("Transmit E-tagged packet to cascade port %d.", cscdPort);
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  upPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* special enhanced UT restore configurations*/
        st = prvTgfPxHalBpeEnhancedUtExtended2UpstreamConfigRestore(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

UTF_TEST_CASE_MAC(prvTgfPxHalBpe_U2E_UC_cncCounters)
{
    GT_U8                              dev = 0;
    GT_STATUS                          st;
    GT_PORT_NUM                        extPort = 0;
    CPSS_PX_PORTS_BMP                  extPortBmp = 0;
    GT_PORT_NUM                        upPort = 12;
    GT_U32                             egressPacketLen[PRV_CPSS_PX_PORTS_NUM_CNS - 1];
    GT_U32                             packetIndex, index, txqPort, tc, passDrop;
    GT_U32                             numOfSentPackets = 30, txQueue = 7, packetLen = 100;
    GT_U32                             ingressBlockNum = 1, egressBlockNum = 0;
    CPSS_PX_CNC_COUNTER_STC            cncEntry;
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC egressInterface, ingressInterface;
    GT_U32		                        pcp,queue;
    union                              {
        GT_U16                         octetPair;
        GT_U8                          octet[2];
    }cid;

    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(extPortBmp, extPort);

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Perform common configuration */
        ingressInterface.type = egressInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        ingressInterface.devPort.devNum = egressInterface.devPort.devNum = dev;
        ingressInterface.devPort.portNum = upPort;
        egressInterface.devPort.portNum = extPort;

        st = cpssPxHalBpeInterfaceTypeSet(dev, &ingressInterface, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, ingressInterface.type, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);

        st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface.type, egressInterface.devPort.devNum, egressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);

        /* 1.1   Set numOfUnicastChannels == 4K - 20, numOfMulticastChannels == 4K,
           Configure unicast E-channel with cid == 1 and send packet */
        cid.octetPair = 1;

        st = cpssPxHalBpeNumOfChannelsSet(dev, _4K - 20, _4K);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, _4K - 20, _4K);

        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &egressInterface, cid.octetPair);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, egressInterface.devPort.devNum, egressInterface.devPort.portNum, cid.octetPair);

        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid.octetPair, &egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        st = cpssPxHalBpePortExtendedUpstreamSet(dev, extPort, &ingressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort);          
        
        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigSet(dev, cid.octetPair, 0xFFF, GT_FALSE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);        

        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = 0x00;
        transmittedPkt[1] = expectedPkt[1] = 0x00;
        transmittedPkt[2] = expectedPkt[2] = 0x00;
        transmittedPkt[3] = expectedPkt[3] = 0x00;
        transmittedPkt[4] = expectedPkt[4] = dev + 1;
        transmittedPkt[5] = expectedPkt[5] = upPort + 1;

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* ingress E-Tag */
        transmittedPkt[12] = 0x89;
        transmittedPkt[13] = 0x3F;
        transmittedPkt[14] = 0x00; 
        transmittedPkt[15] = 0x00;
        transmittedPkt[16] = cid.octet[1];
        transmittedPkt[17] = cid.octet[0];
        transmittedPkt[18] = 0x00;
        transmittedPkt[19] = 0x00;

        egressPacketLen[extPort] = packetLen - BYTES_IN_E_TAG;

        /* Set both counter block 0 and counter block 1 to be used by HAL */
        st = cpssPxHalBpeDebugCncBlocksSet(dev, 3);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, 3);

        /* Bind ingress processing client to block 1 */
        st = cpssPxHalBpeCncConfigClient(dev, CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E, ingressBlockNum);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E, ingressBlockNum);

        /* Bind egress queue drop/pass client to block 0 */
        st = cpssPxHalBpeCncConfigClient(dev, CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E, egressBlockNum);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E, egressBlockNum);

        /* Reset CNC counters */
        for(index = 0; index < _1K; index++) {
            st = cpssPxCncCounterGet(dev, ingressBlockNum, index, CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, &cncEntry);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ingressBlockNum, index, CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E);

            st = cpssPxCncCounterGet(dev, egressBlockNum, index, CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, &cncEntry);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, egressBlockNum, index, CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E);
        }

        /* Send a large amount of packets */
        for(packetIndex = 0; packetIndex < numOfSentPackets; packetIndex++) {
            /* Send packet and check egress as expected */
            st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                      txQueue,
                                                                      &ingressInterface,
                                                                      extPortBmp,
                                                                      GT_FALSE,
                                                                      transmittedPkt,
                                                                      expectedPkt,
                                                                      packetLen,
                                                                      egressPacketLen);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        }

        /* Read CNC counters */
        /* Note that byte count values of CNC counters refer to the frame before header alteration takes place */
        for(index = 0; index < _1K; index++) {

            /* ingress processing client */
            st = cpssPxCncCounterGet(dev, ingressBlockNum, index, CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, &cncEntry);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ingressBlockNum, index, CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E);

            if (index == cid.octetPair + 19) { /* dst PortMap index of unicast E-channel */
                UTF_VERIFY_EQUAL1_STRING_MAC(numOfSentPackets, cncEntry.packetCount.l[0], "ingressCNC.packetCount mismatch on index %d", index);
                UTF_VERIFY_EQUAL1_STRING_MAC(0, cncEntry.packetCount.l[1], "ingressCNC.packetCount mismatch on index %d", index);
                UTF_VERIFY_EQUAL1_STRING_MAC(numOfSentPackets*packetLen, cncEntry.byteCount.l[0], "ingressCNC.byteCount mismatch on index %d", index);
                UTF_VERIFY_EQUAL1_STRING_MAC(0, cncEntry.byteCount.l[1], "ingressCNC.byteCount mismatch on index %d", index);
            }
            else if (index == 3 + upPort) { /* dst portMap index of traffic from CPU to upstream port */
                UTF_VERIFY_EQUAL1_STRING_MAC(numOfSentPackets, cncEntry.packetCount.l[0], "ingressCNC.packetCount mismatch on index %d", index);
                UTF_VERIFY_EQUAL1_STRING_MAC(0, cncEntry.packetCount.l[1], "ingressCNC.packetCount mismatch on index %d", index);
                UTF_VERIFY_EQUAL1_STRING_MAC(numOfSentPackets*(packetLen + BYTES_IN_E_TAG), cncEntry.byteCount.l[0], "ingressCNC.byteCount mismatch on index %d", index);
                UTF_VERIFY_EQUAL1_STRING_MAC(0, cncEntry.byteCount.l[1], "ingressCNC.byteCount mismatch on index %d", index);
            }
            else {
                UTF_VERIFY_EQUAL1_STRING_MAC(0, cncEntry.packetCount.l[0], "ingressCNC.packetCount mismatch on index %d", index);
                UTF_VERIFY_EQUAL1_STRING_MAC(0, cncEntry.packetCount.l[1], "ingressCNC.packetCount mismatch on index %d", index);
                UTF_VERIFY_EQUAL1_STRING_MAC(0, cncEntry.byteCount.l[0], "ingressCNC.byteCount mismatch on index %d", index);
                UTF_VERIFY_EQUAL1_STRING_MAC(0, cncEntry.byteCount.l[1], "ingressCNC.byteCount mismatch on index %d", index);
            }

            txqPort = index >> 4;
            tc = (index & 0xF) >> 1;
            passDrop = index & 0x1;

            /* egress queue pass/drop client */
            st = cpssPxCncCounterGet(dev, egressBlockNum, index, CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, &cncEntry);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, egressBlockNum, index, CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E);

            if(txqPort == extPort && tc == 0 && passDrop == 0) { /* extended port egress queue */
                UTF_VERIFY_EQUAL4_STRING_MAC(numOfSentPackets, cncEntry.packetCount.l[0], "egressCNC.packetCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(0, cncEntry.packetCount.l[1], "egressCNC.packetCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(numOfSentPackets*packetLen, cncEntry.byteCount.l[0], "egressCNC.byteCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(0, cncEntry.byteCount.l[1], "egressCNC.byteCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
            }
            else if (txqPort == upPort && tc == txQueue && passDrop == 0) { /* upstream port egress queue */
                UTF_VERIFY_EQUAL4_STRING_MAC(numOfSentPackets, cncEntry.packetCount.l[0], "egressCNC.packetCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(0, cncEntry.packetCount.l[1], "egressCNC.packetCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(numOfSentPackets*(packetLen + BYTES_IN_E_TAG), cncEntry.byteCount.l[0], "egressCNC.byteCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(0, cncEntry.byteCount.l[1], "egressCNC.byteCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
            }
            else if (txqPort == PRV_CPSS_PX_PORTS_NUM_CNS - 1 && tc == 0 && passDrop == 0) { /* CPU port egress queue */
                UTF_VERIFY_EQUAL4_STRING_MAC(numOfSentPackets, cncEntry.packetCount.l[0], "egressCNC.packetCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(0, cncEntry.packetCount.l[1], "egressCNC.packetCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(numOfSentPackets*packetLen, cncEntry.byteCount.l[0], "egressCNC.byteCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(0, cncEntry.byteCount.l[1], "egressCNC.byteCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
            }
            else {
                UTF_VERIFY_EQUAL4_STRING_MAC(0, cncEntry.packetCount.l[0], "egressCNC.packetCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(0, cncEntry.packetCount.l[1], "egressCNC.packetCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(0, cncEntry.byteCount.l[0], "egressCNC.byteCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(0, cncEntry.byteCount.l[1], "egressCNC.byteCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
            }
        }
        
        /* Mapping of E-Tag<PCP> 0..6 to egress queue 3 */
        queue = 3;
        for(pcp=0; pcp <=6; pcp++) {
            st = cpssPxHalBpeUpstreamQosMapSet(dev,pcp,queue);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, pcp, queue);
        }
        /* Mapping of E-Tag<PCP> 7 to egress queue 0 */
        pcp = 7;
        queue = 0;
        st = cpssPxHalBpeUpstreamQosMapSet(dev,pcp,queue);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, pcp, queue);

        
        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = 0x00;
        transmittedPkt[1] = expectedPkt[1] = 0x00;
        transmittedPkt[2] = expectedPkt[2] = 0x00;
        transmittedPkt[3] = expectedPkt[3] = 0x00;
        transmittedPkt[4] = expectedPkt[4] = dev + 1;
        transmittedPkt[5] = expectedPkt[5] = upPort + 1;

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* ingress E-Tag */
        transmittedPkt[12] = 0x89;
        transmittedPkt[13] = 0x3F;
        transmittedPkt[14] = 0xE5; /* pcp = 7 --> queue = 0 */
        transmittedPkt[15] = 0x55;
        transmittedPkt[16] = cid.octet[1];
        transmittedPkt[17] = cid.octet[0];
        transmittedPkt[18] = 0x00;
        transmittedPkt[19] = 0x00;

        egressPacketLen[extPort] = packetLen - BYTES_IN_E_TAG;       

        /* Reset CNC counters */
        for(index = 0; index < _1K; index++) {
            st = cpssPxCncCounterGet(dev, ingressBlockNum, index, CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, &cncEntry);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ingressBlockNum, index, CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E);

            st = cpssPxCncCounterGet(dev, egressBlockNum, index, CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, &cncEntry);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, egressBlockNum, index, CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E);
        }

        /* Send a large amount of packets */
        for(packetIndex = 0; packetIndex < numOfSentPackets; packetIndex++) {
            /* Send packet and check egress as expected */
            st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                      txQueue,
                                                                      &ingressInterface,
                                                                      extPortBmp,
                                                                      GT_FALSE,
                                                                      transmittedPkt,
                                                                      expectedPkt,
                                                                      packetLen,
                                                                      egressPacketLen);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        }

        /* Read CNC counters */
        /* Note that byte count values of CNC counters refer to the frame before header alteration takes place */
        for(index = 0; index < _1K; index++) {

            /* ingress processing client */
            st = cpssPxCncCounterGet(dev, ingressBlockNum, index, CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, &cncEntry);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ingressBlockNum, index, CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E);

            if (index == cid.octetPair + 19) { /* dst PortMap index of unicast E-channel */
                UTF_VERIFY_EQUAL1_STRING_MAC(numOfSentPackets, cncEntry.packetCount.l[0], "ingressCNC.packetCount mismatch on index %d", index);
                UTF_VERIFY_EQUAL1_STRING_MAC(0, cncEntry.packetCount.l[1], "ingressCNC.packetCount mismatch on index %d", index);
                UTF_VERIFY_EQUAL1_STRING_MAC(numOfSentPackets*packetLen, cncEntry.byteCount.l[0], "ingressCNC.byteCount mismatch on index %d", index);
                UTF_VERIFY_EQUAL1_STRING_MAC(0, cncEntry.byteCount.l[1], "ingressCNC.byteCount mismatch on index %d", index);
            }
            else if (index == 3 + upPort) { /* dst portMap index of traffic from CPU to upstream port */
                UTF_VERIFY_EQUAL1_STRING_MAC(numOfSentPackets, cncEntry.packetCount.l[0], "ingressCNC.packetCount mismatch on index %d", index);
                UTF_VERIFY_EQUAL1_STRING_MAC(0, cncEntry.packetCount.l[1], "ingressCNC.packetCount mismatch on index %d", index);
                UTF_VERIFY_EQUAL1_STRING_MAC(numOfSentPackets*(packetLen + BYTES_IN_E_TAG), cncEntry.byteCount.l[0], "ingressCNC.byteCount mismatch on index %d", index);
                UTF_VERIFY_EQUAL1_STRING_MAC(0, cncEntry.byteCount.l[1], "ingressCNC.byteCount mismatch on index %d", index);
            }
            else {
                UTF_VERIFY_EQUAL1_STRING_MAC(0, cncEntry.packetCount.l[0], "ingressCNC.packetCount mismatch on index %d", index);
                UTF_VERIFY_EQUAL1_STRING_MAC(0, cncEntry.packetCount.l[1], "ingressCNC.packetCount mismatch on index %d", index);
                UTF_VERIFY_EQUAL1_STRING_MAC(0, cncEntry.byteCount.l[0], "ingressCNC.byteCount mismatch on index %d", index);
                UTF_VERIFY_EQUAL1_STRING_MAC(0, cncEntry.byteCount.l[1], "ingressCNC.byteCount mismatch on index %d", index);
            }

            txqPort = index >> 4;
            tc = (index & 0xF) >> 1;
            passDrop = index & 0x1;

            /* egress queue pass/drop client */
            st = cpssPxCncCounterGet(dev, egressBlockNum, index, CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, &cncEntry);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, egressBlockNum, index, CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E);

            if(txqPort == extPort && tc == 0 && passDrop == 0) { /* extended port egress queue */
                UTF_VERIFY_EQUAL4_STRING_MAC(numOfSentPackets, cncEntry.packetCount.l[0], "egressCNC.packetCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(0, cncEntry.packetCount.l[1], "egressCNC.packetCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(numOfSentPackets*packetLen, cncEntry.byteCount.l[0], "egressCNC.byteCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(0, cncEntry.byteCount.l[1], "egressCNC.byteCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
            }
            else if (txqPort == upPort && tc == txQueue && passDrop == 0) { /* upstream port egress queue */
                UTF_VERIFY_EQUAL4_STRING_MAC(numOfSentPackets, cncEntry.packetCount.l[0], "egressCNC.packetCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(0, cncEntry.packetCount.l[1], "egressCNC.packetCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(numOfSentPackets*(packetLen + BYTES_IN_E_TAG), cncEntry.byteCount.l[0], "egressCNC.byteCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(0, cncEntry.byteCount.l[1], "egressCNC.byteCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
            }
            else if (txqPort == PRV_CPSS_PX_PORTS_NUM_CNS - 1 && tc == 0 && passDrop == 0) { /* CPU port egress queue */
                UTF_VERIFY_EQUAL4_STRING_MAC(numOfSentPackets, cncEntry.packetCount.l[0], "egressCNC.packetCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(0, cncEntry.packetCount.l[1], "egressCNC.packetCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(numOfSentPackets*packetLen, cncEntry.byteCount.l[0], "egressCNC.byteCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(0, cncEntry.byteCount.l[1], "egressCNC.byteCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
            }
            else {
                UTF_VERIFY_EQUAL4_STRING_MAC(0, cncEntry.packetCount.l[0], "egressCNC.packetCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(0, cncEntry.packetCount.l[1], "egressCNC.packetCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(0, cncEntry.byteCount.l[0], "egressCNC.byteCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
                UTF_VERIFY_EQUAL4_STRING_MAC(0, cncEntry.byteCount.l[1], "egressCNC.byteCount mismatch on index %d, port %d, tc %d pass/drop %d", index, txqPort, tc, passDrop);
            }
        }

        /* special enhanced UT restore configurations */
        st = prvTgfPxHalBpeEnhancedUtUpstream2ExtendedUcConfigRestore(dev, cid.octetPair);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);
    }
}

UTF_TEST_CASE_MAC(prvTgfPxHalBpe_E2U_UC_qosMapping)
{
    GT_U8                              dev = 0;
    GT_STATUS                          st;
    GT_PORT_NUM                        extPort = 0;
    CPSS_PX_PORTS_BMP                  upPortBmp = 0;
    GT_PORT_NUM                        upPort = 12;
    GT_U32                             packetLen = 100;
    GT_U32                             egressPacketLen[PRV_CPSS_PX_PORTS_NUM_CNS - 1];
    GT_U32                             pcp, dei;
    GT_U32                             defaultDei = 1;
    GT_U32                             defaultPcp = 4;
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC egressInterface, ingressInterface;
    CPSS_PX_HAL_BPE_QOSMAP_STC         qosMapArr[CPSS_PCP_RANGE_CNS][CPSS_DEI_RANGE_CNS];
    GT_U32                             vid = 0x44;
    union                              {
        GT_U16                         octetPair;
        GT_U8                          octet[2];
    }cid;
    cid.octetPair = 1;

    /* Create qos mapping */
    for(pcp = 0; pcp < CPSS_PCP_RANGE_CNS; pcp++)
    {
        for(dei = 0; dei < CPSS_DEI_RANGE_CNS; dei++)
        {
            qosMapArr[pcp][dei].newDei = CPSS_DEI_RANGE_CNS - dei - 1;
            qosMapArr[pcp][dei].newPcp = CPSS_PCP_RANGE_CNS - pcp - 1;
        }
    }

    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(upPortBmp, upPort);

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Perform common configuration */
        ingressInterface.type = egressInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        ingressInterface.devPort.devNum = egressInterface.devPort.devNum = dev;
        ingressInterface.devPort.portNum = extPort;
        egressInterface.devPort.portNum = upPort;

        st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface.type, egressInterface.devPort.devNum, egressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);

        st = cpssPxHalBpeInterfaceTypeSet(dev, &ingressInterface, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, ingressInterface.type, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);

        st = cpssPxHalBpeNumOfChannelsSet(dev, _4K - 20, _4K);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, _4K - 20, _4K);

        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &ingressInterface, cid.octetPair);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, cid.octetPair);

        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid.octetPair, &ingressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        st = cpssPxHalBpePortExtendedUpstreamSet(dev, extPort, &egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort);

        st = cpssPxHalBpePortExtendedQosDefaultPcpDeiSet(dev, extPort, defaultPcp, defaultDei);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, extPort, defaultPcp, defaultDei);

        st = cpssPxHalBpePortExtendedQosMapSet(dev, extPort, qosMapArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtExtended2UpstreamConfigSet(dev, extPort);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort);

        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = 0x00;
        transmittedPkt[1] = expectedPkt[1] = 0x00;
        transmittedPkt[2] = expectedPkt[2] = 0x00;
        transmittedPkt[3] = expectedPkt[3] = 0x00;
        transmittedPkt[4] = expectedPkt[4] = dev + 1;
        transmittedPkt[5] = expectedPkt[5] = extPort + 1;

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* 1.1 transmit untagged packet to extended port, expect it to egress E-tagged out of upstream
           with pcp, dei fields according to default values and according to qos map */

        /* egress E-Tag */
        expectedPkt[12] = 0x89;
        expectedPkt[13] = 0x3F;
        expectedPkt[14] = (qosMapArr[defaultPcp][defaultDei].newDei << 4) | (qosMapArr[defaultPcp][defaultDei].newPcp << 5);
        expectedPkt[15] = 0x00;
        expectedPkt[16] = cid.octet[1];
        expectedPkt[17] = cid.octet[0];
        expectedPkt[18] = 0x00;
        expectedPkt[19] = 0x00;

        egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

        BPE_LOG_INFORMATION_MAC("Transmit untagged packet to extended port %d.", extPort);
        /* Send packet and check egress as expected */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  upPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 transmit C-tagged packet to extended port, expect it to egress E-tagged+C-tagged out of upstream
           with pcp, dei fields according to default values and according to qos map in the E-tag and original pcp, dei
           values in the C-tag */
        for (pcp = 0; pcp < CPSS_PCP_RANGE_CNS; pcp++)
        {
            for (dei = 0; dei < CPSS_DEI_RANGE_CNS; dei++)
            {
                /* ingress C-Tag */
                transmittedPkt[12] = 0x81;
                transmittedPkt[13] = 0x00;
                transmittedPkt[14] = (dei << 4) | (pcp << 5);
                transmittedPkt[15] = vid;
                /* egress E-Tag */
                expectedPkt[12] = 0x89;
                expectedPkt[13] = 0x3F;
                expectedPkt[14] = (qosMapArr[pcp][dei].newDei << 4) | (qosMapArr[pcp][dei].newPcp << 5);
                expectedPkt[15] = 0x00;
                expectedPkt[16] = cid.octet[1];
                expectedPkt[17] = cid.octet[0];
                expectedPkt[18] = 0x00;
                expectedPkt[19] = 0x00;
                /* egress C-tag */
                expectedPkt[20] = 0x81;
                expectedPkt[21] = 0x00;
                expectedPkt[22] = (dei << 4) | (pcp << 5);
                expectedPkt[23] = vid;

                /* Send packet and check egress as expected */
                BPE_LOG_INFORMATION_MAC("Transmit C-tagged packet with PCP=%d DEI=%d to extended port %d.", pcp, dei, extPort);
                st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                          7,
                                                                          &ingressInterface,
                                                                          upPortBmp,
                                                                          GT_FALSE,
                                                                          transmittedPkt,
                                                                          expectedPkt,
                                                                          packetLen,
                                                                          egressPacketLen);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
        }

        /* special enhanced UT restore configurations*/
        st = prvTgfPxHalBpeEnhancedUtExtended2UpstreamConfigRestore(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

UTF_TEST_CASE_MAC(prvTgfPxHalBpe_E2U_UC_dataControlQosRule)
{
    GT_U8                                    dev = 0, ipProtocol = 0x11;
    GT_STATUS                                st;
    GT_PORT_NUM                              extPort = 0;
    CPSS_PX_PORTS_BMP                        upPortBmp = 0;
    GT_PORT_NUM                              upPort = 12;
    GT_U32                                   packetLen = 100;
    GT_U32                                   egressPacketLen[PRV_CPSS_PX_PORTS_NUM_CNS - 1];
    GT_U32                                   pcp, dei, macDaRuleIndex = 0, dscpIpv4RuleIndex = 1, dscpIpv6RuleIndex = 2, llcNonSnapRuleIndex = 3, llcSnapRuleIndex = 4;
    GT_U32                                   defaultDei = 1, defaultPcp = 4;
    GT_U32                                   macDaRulePcp, macDaRuleDei;
    GT_U32                                   dscpIpv4RulePcp, dscpIpv4RuleDei, dscpIpv6RulePcp, dscpIpv6RuleDei, llcNonSnapRulePcp, llcNonSnapRuleDei, llcSnapRulePcp, llcSnapRuleDei;
    GT_ETHERADDR                             portMacDa = {{0x11, 0x22, 0x33, 0x44, 0x55, 0x66}};
    GT_ETHERADDR                             ruleMacDa = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}};
    GT_IPADDR                                srcIpv4Addr = {0xAC1402FD};
    GT_IPADDR                                dstIpv4Addr = {0xAC140006};
    GT_IPV6ADDR                              srcIpv6Addr = {{0x20010DB8, 0x85A30000, 0x00008A2E, 0x03707334}};
    GT_IPV6ADDR                              dstIpv6Addr = {{0x20010000, 0x3238DFE1, 0x00630000, 0x0000FEFB}};
    GT_U16                                   ruleLlcDsapSsap = 0x4242, portLlcDsapSsap = 0xFEFE;
    GT_U16                                   ruleLlcSnapEtherType = 0x1234, portLlcSnapEtherType = 0xABCD;
    GT_U32                                   ruleDscpIpv4 = 0x2A, ruleDscpIpv6 = 0x15, portDscp = 0x3F;
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC       egressInterface, ingressInterface;
    CPSS_PX_HAL_BPE_QOSMAP_STC               portQosMap[CPSS_PCP_RANGE_CNS][CPSS_DEI_RANGE_CNS];
    CPSS_PX_HAL_BPE_QOSMAP_DATA_STC          ruleQosMap;
    CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC rulePattern, ruleMask;
    GT_U32                                   vid = 0x44;
    union                                {
        GT_U16                               octetPair;
        GT_U8                                octet[2];
    }cid;
    cid.octetPair = 1;

    /* Create qos mapping */
    for(pcp = 0; pcp < CPSS_PCP_RANGE_CNS; pcp++)
    {
        for(dei = 0; dei < CPSS_DEI_RANGE_CNS; dei++)
        {
            portQosMap[pcp][dei].newDei = CPSS_DEI_RANGE_CNS - dei - 1;
            portQosMap[pcp][dei].newPcp = CPSS_PCP_RANGE_CNS - pcp - 1;
        }
    }

    macDaRulePcp = (portQosMap[defaultPcp][defaultDei].newPcp + 1)%CPSS_PCP_RANGE_CNS;
    macDaRuleDei = (portQosMap[defaultPcp][defaultDei].newDei + 1)%CPSS_DEI_RANGE_CNS;

    dscpIpv4RulePcp = (macDaRulePcp + 1)%CPSS_PCP_RANGE_CNS;
    dscpIpv4RuleDei = (macDaRuleDei + 1)%CPSS_DEI_RANGE_CNS;

    dscpIpv6RulePcp = (dscpIpv4RulePcp + 1)%CPSS_PCP_RANGE_CNS;
    dscpIpv6RuleDei = (dscpIpv4RuleDei + 1)%CPSS_DEI_RANGE_CNS;

    llcNonSnapRulePcp = (dscpIpv6RulePcp + 1)%CPSS_PCP_RANGE_CNS;
    llcNonSnapRuleDei = (dscpIpv6RuleDei + 1)%CPSS_DEI_RANGE_CNS;

    llcSnapRulePcp = (llcNonSnapRulePcp + 1)%CPSS_PCP_RANGE_CNS;
    llcSnapRuleDei = (llcNonSnapRuleDei + 1)%CPSS_DEI_RANGE_CNS;

    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(upPortBmp, upPort);

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Perform common configuration */
        ingressInterface.type = egressInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        ingressInterface.devPort.devNum = egressInterface.devPort.devNum = dev;
        ingressInterface.devPort.portNum = extPort;
        egressInterface.devPort.portNum = upPort;

        st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface.type, egressInterface.devPort.devNum, egressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);

        st = cpssPxHalBpeInterfaceTypeSet(dev, &ingressInterface, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, ingressInterface.type, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);

        st = cpssPxHalBpeNumOfChannelsSet(dev, _4K - 20, _4K);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, _4K - 20, _4K);

        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &ingressInterface, cid.octetPair);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, cid.octetPair);

        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid.octetPair, &ingressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        st = cpssPxHalBpePortExtendedUpstreamSet(dev, extPort, &egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort);

        st = cpssPxHalBpePortExtendedQosDefaultPcpDeiSet(dev, extPort, defaultPcp, defaultDei);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, extPort, defaultPcp, defaultDei);

        st = cpssPxHalBpePortExtendedQosMapSet(dev, extPort, portQosMap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort);

        /* Configure qos rule to match MAC DA */
        rulePattern.macDa = ruleMacDa;

        cpssOsMemSet(&ruleMask, 0x00, sizeof(CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC));
        cpssOsMemSet(&(ruleMask.macDa), 0xFF, sizeof(GT_ETHERADDR));

        ruleQosMap.newDei = macDaRuleDei;
        ruleQosMap.newPcp = macDaRulePcp;
        ruleQosMap.queue = 0;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, macDaRuleIndex, &ruleQosMap, CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_ANY_E, &ruleMask, &rulePattern);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, macDaRuleIndex);

        /* Configure qos rule to match ipv4 DSCP and IP protocol */
        rulePattern.dscp = ruleDscpIpv4;

        cpssOsMemSet(&ruleMask, 0x00, sizeof(CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC));
        ruleMask.dscp = 0x3F;

        rulePattern.ipProtocol = ipProtocol;
        ruleMask.ipProtocol = 0xFF;

        ruleQosMap.newDei = dscpIpv4RuleDei;
        ruleQosMap.newPcp = dscpIpv4RulePcp;
        ruleQosMap.queue = 0;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, dscpIpv4RuleIndex, &ruleQosMap, CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV4_E, &ruleMask, &rulePattern);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dscpIpv4RuleIndex);

        /* Configure qos rule to match ipv6 DSCP */
        rulePattern.dscp = ruleDscpIpv6;

        cpssOsMemSet(&ruleMask, 0x00, sizeof(CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC));
        ruleMask.dscp = 0x3F;

        ruleQosMap.newDei = dscpIpv6RuleDei;
        ruleQosMap.newPcp = dscpIpv6RulePcp;
        ruleQosMap.queue = 0;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, dscpIpv6RuleIndex, &ruleQosMap, CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV6_E, &ruleMask, &rulePattern);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dscpIpv6RuleIndex);

        /* Configure qos rule to match LLC non SNAP packet */
        rulePattern.etherType = ruleLlcDsapSsap;

        cpssOsMemSet(&ruleMask, 0x00, sizeof(CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC));
        ruleMask.etherType = 0xFFFF;

        ruleQosMap.newDei = llcNonSnapRuleDei;
        ruleQosMap.newPcp = llcNonSnapRulePcp;
        ruleQosMap.queue = 0;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, llcNonSnapRuleIndex, &ruleQosMap, CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_LLC_NOT_SNAP_E, &ruleMask, &rulePattern);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, llcNonSnapRuleIndex);

        /* Configure qos rule to match LLC SNAP packet */
        rulePattern.etherType = ruleLlcSnapEtherType;

        cpssOsMemSet(&ruleMask, 0x00, sizeof(CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC));
        ruleMask.etherType = 0xFFFF;

        ruleQosMap.newDei = llcSnapRuleDei;
        ruleQosMap.newPcp = llcSnapRulePcp;
        ruleQosMap.queue = 0;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, llcSnapRuleIndex, &ruleQosMap, CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_ETHERNET_OR_LLC_SNAP_E, &ruleMask, &rulePattern);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, llcSnapRuleIndex);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtExtended2UpstreamConfigSet(dev, extPort);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort);

        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* 1.1 transmit untagged packet to extended port that does not match any rule pattern
           expect it to egress E-tagged out of upstream with pcp, dei fields according to default qos mapping */

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = portMacDa.arEther[0];
        transmittedPkt[1] = expectedPkt[1] = portMacDa.arEther[1];
        transmittedPkt[2] = expectedPkt[2] = portMacDa.arEther[2];
        transmittedPkt[3] = expectedPkt[3] = portMacDa.arEther[3];
        transmittedPkt[4] = expectedPkt[4] = portMacDa.arEther[4];
        transmittedPkt[5] = expectedPkt[5] = portMacDa.arEther[5];

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* egress E-Tag */
        expectedPkt[12] = 0x89;
        expectedPkt[13] = 0x3F;
        expectedPkt[14] = (portQosMap[defaultPcp][defaultDei].newDei << 4) | (portQosMap[defaultPcp][defaultDei].newPcp << 5);
        expectedPkt[15] = 0x00;
        expectedPkt[16] = cid.octet[1];
        expectedPkt[17] = cid.octet[0];
        expectedPkt[18] = 0x00;
        expectedPkt[19] = 0x00;

        egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

        /* Send packet and check egress as expected */
        BPE_LOG_INFORMATION_MAC("Transmit untagged packet not matching any rule to extended port %d.", extPort);
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  upPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 transmit untagged packet to extended port that matches MAC DA rule pattern
           expect it to egress E-tagged out of upstream with pcp, dei fields according to qos rule mapping */

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = ruleMacDa.arEther[0];
        transmittedPkt[1] = expectedPkt[1] = ruleMacDa.arEther[1];
        transmittedPkt[2] = expectedPkt[2] = ruleMacDa.arEther[2];
        transmittedPkt[3] = expectedPkt[3] = ruleMacDa.arEther[3];
        transmittedPkt[4] = expectedPkt[4] = ruleMacDa.arEther[4];
        transmittedPkt[5] = expectedPkt[5] = ruleMacDa.arEther[5];

        /* egress E-Tag */
        expectedPkt[12] = 0x89;
        expectedPkt[13] = 0x3F;
        expectedPkt[14] = (macDaRuleDei << 4) | (macDaRulePcp << 5);
        expectedPkt[15] = 0x00;
        expectedPkt[16] = cid.octet[1];
        expectedPkt[17] = cid.octet[0];
        expectedPkt[18] = 0x00;
        expectedPkt[19] = 0x00;

        egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

        /* Send packet and check egress as expected */
        BPE_LOG_INFORMATION_MAC("Transmit untagged packet matching MAC DA rule to extended port %d.", extPort);
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  upPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.3 transmit C-tagged packet to extended port that does not match any rule pattern, expect it to egress E-tagged + C-tagged out of upstream
           with pcp, dei fields according to default qos mapping in the E-tag and original pcp, dei values in the C-tag */

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = portMacDa.arEther[0];
        transmittedPkt[1] = expectedPkt[1] = portMacDa.arEther[1];
        transmittedPkt[2] = expectedPkt[2] = portMacDa.arEther[2];
        transmittedPkt[3] = expectedPkt[3] = portMacDa.arEther[3];
        transmittedPkt[4] = expectedPkt[4] = portMacDa.arEther[4];
        transmittedPkt[5] = expectedPkt[5] = portMacDa.arEther[5];

        for (pcp = 0; pcp < CPSS_PCP_RANGE_CNS; pcp++)
        {
            for (dei = 0; dei < CPSS_DEI_RANGE_CNS; dei++)
            {
                /* ingress + egress C-Tag */
                transmittedPkt[12] = expectedPkt[20] = 0x81;
                transmittedPkt[13] = expectedPkt[21] = 0x00;
                transmittedPkt[14] = expectedPkt[22] = (dei << 4) | (pcp << 5);
                transmittedPkt[15] = expectedPkt[23] = vid;
                /* egress E-Tag */
                expectedPkt[12] = 0x89;
                expectedPkt[13] = 0x3F;
                expectedPkt[14] = (portQosMap[pcp][dei].newDei << 4) | (portQosMap[pcp][dei].newPcp << 5);
                expectedPkt[15] = 0x00;
                expectedPkt[16] = cid.octet[1];
                expectedPkt[17] = cid.octet[0];
                expectedPkt[18] = 0x00;
                expectedPkt[19] = 0x00;

                egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

                /* Send packet and check egress as expected */
                BPE_LOG_INFORMATION_MAC("Transmit C-tagged packet with PCP=%d DEI=%d not matching any rule to extended port %d.", pcp, dei, extPort);
                st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                          7,
                                                                          &ingressInterface,
                                                                          upPortBmp,
                                                                          GT_FALSE,
                                                                          transmittedPkt,
                                                                          expectedPkt,
                                                                          packetLen,
                                                                          egressPacketLen);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
        }

        /* 1.4 transmit C-tagged packet to extended port that matches MAC DA rule pattern, expect it to egress E-tagged + C-tagged out of upstream
           with pcp, dei fields according to data control qos rule in the E-tag and original pcp, dei values in the C-tag */

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = ruleMacDa.arEther[0];
        transmittedPkt[1] = expectedPkt[1] = ruleMacDa.arEther[1];
        transmittedPkt[2] = expectedPkt[2] = ruleMacDa.arEther[2];
        transmittedPkt[3] = expectedPkt[3] = ruleMacDa.arEther[3];
        transmittedPkt[4] = expectedPkt[4] = ruleMacDa.arEther[4];
        transmittedPkt[5] = expectedPkt[5] = ruleMacDa.arEther[5];

        /* egress E-Tag */
        expectedPkt[12] = 0x89;
        expectedPkt[13] = 0x3F;
        expectedPkt[14] = (macDaRuleDei << 4) | (macDaRulePcp << 5);
        expectedPkt[15] = 0x00;
        expectedPkt[16] = cid.octet[1];
        expectedPkt[17] = cid.octet[0];
        expectedPkt[18] = 0x00;
        expectedPkt[19] = 0x00;

        for (pcp = 0; pcp < CPSS_PCP_RANGE_CNS; pcp++)
        {
            for (dei = 0; dei < CPSS_DEI_RANGE_CNS; dei++)
            {
                /* ingress + egress C-Tag */
                transmittedPkt[12] = expectedPkt[20] = 0x81;
                transmittedPkt[13] = expectedPkt[21] = 0x00;
                transmittedPkt[14] = expectedPkt[22] = (dei << 4) | (pcp << 5);
                transmittedPkt[15] = expectedPkt[23] = vid;

                egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

                /* Send packet and check egress as expected */
                BPE_LOG_INFORMATION_MAC("Transmit C-tagged packet with PCP=%d DEI=%d matching MAC DA rule to extended port %d.", pcp, dei, extPort);
                st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                          7,
                                                                          &ingressInterface,
                                                                          upPortBmp,
                                                                          GT_FALSE,
                                                                          transmittedPkt,
                                                                          expectedPkt,
                                                                          packetLen,
                                                                          egressPacketLen);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
        }        

        /* 1.5 transmit untagged ipv4 packet to extended port that matches DSCP rule pattern
           expect it to egress E-tagged out of upstream with pcp, dei fields according to data control qos rule */
        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = portMacDa.arEther[0];
        transmittedPkt[1] = expectedPkt[1] = portMacDa.arEther[1];
        transmittedPkt[2] = expectedPkt[2] = portMacDa.arEther[2];
        transmittedPkt[3] = expectedPkt[3] = portMacDa.arEther[3];
        transmittedPkt[4] = expectedPkt[4] = portMacDa.arEther[4];
        transmittedPkt[5] = expectedPkt[5] = portMacDa.arEther[5];

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* ingress + egress ipV4 header */
        transmittedPkt[12] = expectedPkt[20] = 0x08;
        transmittedPkt[13] = expectedPkt[21] = 0x00;
        transmittedPkt[14] = expectedPkt[22] = 0x45;
        transmittedPkt[15] = expectedPkt[23] = ruleDscpIpv4 << 2;
        transmittedPkt[16] = expectedPkt[24] = 0x00;
        transmittedPkt[17] = expectedPkt[25] = 0x44;
        transmittedPkt[18] = expectedPkt[26] = 0xAD;
        transmittedPkt[19] = expectedPkt[27] = 0x0B;
        transmittedPkt[20] = expectedPkt[28] = 0x00;
        transmittedPkt[21] = expectedPkt[29] = 0x00;
        transmittedPkt[22] = expectedPkt[30] = 0x40;
        transmittedPkt[23] = expectedPkt[31] = ipProtocol;
        transmittedPkt[24] = expectedPkt[32] = 0x72; /*checksum*/
        transmittedPkt[25] = expectedPkt[33] = 0x72; /*checksum*/
        transmittedPkt[26] = expectedPkt[34] = srcIpv4Addr.arIP[0];
        transmittedPkt[27] = expectedPkt[35] = srcIpv4Addr.arIP[1];
        transmittedPkt[28] = expectedPkt[36] = srcIpv4Addr.arIP[2];
        transmittedPkt[29] = expectedPkt[37] = srcIpv4Addr.arIP[3];
        transmittedPkt[30] = expectedPkt[38] = dstIpv4Addr.arIP[0];
        transmittedPkt[31] = expectedPkt[39] = dstIpv4Addr.arIP[1];
        transmittedPkt[32] = expectedPkt[40] = dstIpv4Addr.arIP[2];
        transmittedPkt[33] = expectedPkt[41] = dstIpv4Addr.arIP[3];

        /* egress E-Tag */
        expectedPkt[12] = 0x89;
        expectedPkt[13] = 0x3F;
        expectedPkt[14] = (dscpIpv4RuleDei << 4) | (dscpIpv4RulePcp << 5);
        expectedPkt[15] = 0x00;
        expectedPkt[16] = cid.octet[1];
        expectedPkt[17] = cid.octet[0];
        expectedPkt[18] = 0x00;
        expectedPkt[19] = 0x00;

        egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

        /* Send packet and check egress as expected */
        BPE_LOG_INFORMATION_MAC("Transmit untagged ipv4 packet matching DSCP rule to extended port %d.", extPort);
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  upPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        
        /* 1.6 transmit untagged ipv4 packet to extended port that does not match any rule pattern
           expect it to egress E-tagged out of upstream with pcp, dei fields according to extended port default */

        /* ingress + egress ipV4 header */
        transmittedPkt[15] = expectedPkt[23] = portDscp << 2;

        /* egress E-Tag */
        expectedPkt[14] = (portQosMap[defaultPcp][defaultDei].newDei << 4) | (portQosMap[defaultPcp][defaultDei].newPcp << 5);

        egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

        /* Send packet and check egress as expected */
        BPE_LOG_INFORMATION_MAC("Transmit untagged ipv4 packet not matching any rule to extended port %d.", extPort);
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  upPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.7 transmit C-tagged ipv4 packet to extended port that matches DSCP rule pattern
           expect it to egress E-tagged out of upstream with pcp, dei fields according to data control qos rule */

        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = portMacDa.arEther[0];
        transmittedPkt[1] = expectedPkt[1] = portMacDa.arEther[1];
        transmittedPkt[2] = expectedPkt[2] = portMacDa.arEther[2];
        transmittedPkt[3] = expectedPkt[3] = portMacDa.arEther[3];
        transmittedPkt[4] = expectedPkt[4] = portMacDa.arEther[4];
        transmittedPkt[5] = expectedPkt[5] = portMacDa.arEther[5];

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* ingress + egress ipV4 header */
        transmittedPkt[16] = expectedPkt[24] = 0x08;
        transmittedPkt[17] = expectedPkt[25] = 0x00;
        transmittedPkt[18] = expectedPkt[26] = 0x45;
        transmittedPkt[19] = expectedPkt[27] = ruleDscpIpv4 << 2;
        transmittedPkt[20] = expectedPkt[28] = 0x00;
        transmittedPkt[21] = expectedPkt[29] = 0x44;
        transmittedPkt[22] = expectedPkt[30] = 0xAD;
        transmittedPkt[23] = expectedPkt[31] = 0x0B;
        transmittedPkt[24] = expectedPkt[32] = 0x00;
        transmittedPkt[25] = expectedPkt[33] = 0x00;
        transmittedPkt[26] = expectedPkt[34] = 0x40;
        transmittedPkt[27] = expectedPkt[35] = ipProtocol;
        transmittedPkt[28] = expectedPkt[36] = 0x72; /*checksum*/
        transmittedPkt[29] = expectedPkt[37] = 0x72; /*checksum*/
        transmittedPkt[30] = expectedPkt[38] = srcIpv4Addr.arIP[0];
        transmittedPkt[31] = expectedPkt[39] = srcIpv4Addr.arIP[1];
        transmittedPkt[32] = expectedPkt[40] = srcIpv4Addr.arIP[2];
        transmittedPkt[33] = expectedPkt[41] = srcIpv4Addr.arIP[3];
        transmittedPkt[34] = expectedPkt[42] = dstIpv4Addr.arIP[0];
        transmittedPkt[35] = expectedPkt[43] = dstIpv4Addr.arIP[1];
        transmittedPkt[36] = expectedPkt[44] = dstIpv4Addr.arIP[2];
        transmittedPkt[37] = expectedPkt[45] = dstIpv4Addr.arIP[3];

        /* egress E-Tag */
        expectedPkt[12] = 0x89;
        expectedPkt[13] = 0x3F;
        expectedPkt[14] = (dscpIpv4RuleDei << 4) | (dscpIpv4RulePcp << 5);
        expectedPkt[15] = 0x00;
        expectedPkt[16] = cid.octet[1];
        expectedPkt[17] = cid.octet[0];
        expectedPkt[18] = 0x00;
        expectedPkt[19] = 0x00;

        for (pcp = 0; pcp < CPSS_PCP_RANGE_CNS; pcp++)
        {
            for (dei = 0; dei < CPSS_DEI_RANGE_CNS; dei++)
            {
                /* ingress + egress C-Tag */
                transmittedPkt[12] = expectedPkt[20] = 0x81;
                transmittedPkt[13] = expectedPkt[21] = 0x00;
                transmittedPkt[14] = expectedPkt[22] = (dei << 4) | (pcp << 5);
                transmittedPkt[15] = expectedPkt[23] = vid;

                egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

                /* Send packet and check egress as expected */
                BPE_LOG_INFORMATION_MAC("Transmit C-tagged ipv4 packet with PCP=%d DEI=%d matching DSCP rule to extended port %d.", pcp, dei, extPort);
                st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                          7,
                                                                          &ingressInterface,
                                                                          upPortBmp,
                                                                          GT_FALSE,
                                                                          transmittedPkt,
                                                                          expectedPkt,
                                                                          packetLen,
                                                                          egressPacketLen);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
        }

        /* 1.8 transmit C-tagged ipv4 packet to extended port that does not match any rule pattern
           expect it to egress E-tagged out of upstream with pcp, dei fields according to port qos mapping */

        /* ingress + egress ipV4 header */
        transmittedPkt[19] = expectedPkt[27] = portDscp << 2;

        for (pcp = 0; pcp < CPSS_PCP_RANGE_CNS; pcp++)
        {
            for (dei = 0; dei < CPSS_DEI_RANGE_CNS; dei++)
            {
                /* ingress + egress C-Tag */
                transmittedPkt[12] = expectedPkt[20] = 0x81;
                transmittedPkt[13] = expectedPkt[21] = 0x00;
                transmittedPkt[14] = expectedPkt[22] = (dei << 4) | (pcp << 5);
                transmittedPkt[15] = expectedPkt[23] = vid;

                /* egress E-Tag */
                expectedPkt[12] = 0x89;
                expectedPkt[13] = 0x3F;
                expectedPkt[14] = (portQosMap[pcp][dei].newDei << 4) | (portQosMap[pcp][dei].newPcp << 5);
                expectedPkt[15] = 0x00;
                expectedPkt[16] = cid.octet[1];
                expectedPkt[17] = cid.octet[0];
                expectedPkt[18] = 0x00;
                expectedPkt[19] = 0x00;

                egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

                /* Send packet and check egress as expected */
                BPE_LOG_INFORMATION_MAC("Transmit C-tagged ipv4 packet with PCP=%d DEI=%d not matching any rule to extended port %d.", pcp, dei, extPort);
                st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                          7,
                                                                          &ingressInterface,
                                                                          upPortBmp,
                                                                          GT_FALSE,
                                                                          transmittedPkt,
                                                                          expectedPkt,
                                                                          packetLen,
                                                                          egressPacketLen);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
        }

        /* 1.9 transmit untagged ipv6 packet to extended port that matches DSCP rule pattern
           expect it to egress E-tagged out of upstream with pcp, dei fields according to data control qos rule */
        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = portMacDa.arEther[0];
        transmittedPkt[1] = expectedPkt[1] = portMacDa.arEther[1];
        transmittedPkt[2] = expectedPkt[2] = portMacDa.arEther[2];
        transmittedPkt[3] = expectedPkt[3] = portMacDa.arEther[3];
        transmittedPkt[4] = expectedPkt[4] = portMacDa.arEther[4];
        transmittedPkt[5] = expectedPkt[5] = portMacDa.arEther[5];

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* ingress + egress ipV6 header */
        transmittedPkt[12] = expectedPkt[20] = 0x86;
        transmittedPkt[13] = expectedPkt[21] = 0xDD;
        transmittedPkt[14] = expectedPkt[22] = (0x06 << 4) | (((ruleDscpIpv6 << 2) & 0xF0) >> 4);
        transmittedPkt[15] = expectedPkt[23] = ((ruleDscpIpv6 << 2) & 0x0F) << 4;
        transmittedPkt[16] = expectedPkt[24] = 0x00;
        transmittedPkt[17] = expectedPkt[25] = 0x00;
        transmittedPkt[18] = expectedPkt[26] = 0x00;
        transmittedPkt[19] = expectedPkt[27] = packetLen - 2*BYTES_IN_MAC_ADDR - BYTES_IN_IPV6_HEADER;
        transmittedPkt[20] = expectedPkt[28] = ipProtocol + 1;
        transmittedPkt[21] = expectedPkt[29] = 0xFF;
        transmittedPkt[22] = expectedPkt[30] = srcIpv6Addr.arIP[0];
        transmittedPkt[23] = expectedPkt[31] = srcIpv6Addr.arIP[1];
        transmittedPkt[24] = expectedPkt[32] = srcIpv6Addr.arIP[2];
        transmittedPkt[25] = expectedPkt[33] = srcIpv6Addr.arIP[3];
        transmittedPkt[26] = expectedPkt[34] = srcIpv6Addr.arIP[4];
        transmittedPkt[27] = expectedPkt[35] = srcIpv6Addr.arIP[5];
        transmittedPkt[28] = expectedPkt[36] = srcIpv6Addr.arIP[6];
        transmittedPkt[29] = expectedPkt[37] = srcIpv6Addr.arIP[7];
        transmittedPkt[30] = expectedPkt[38] = srcIpv6Addr.arIP[8];
        transmittedPkt[31] = expectedPkt[39] = srcIpv6Addr.arIP[9];
        transmittedPkt[32] = expectedPkt[40] = srcIpv6Addr.arIP[10];
        transmittedPkt[33] = expectedPkt[41] = srcIpv6Addr.arIP[11];
        transmittedPkt[34] = expectedPkt[42] = srcIpv6Addr.arIP[12];
        transmittedPkt[35] = expectedPkt[43] = srcIpv6Addr.arIP[13];
        transmittedPkt[36] = expectedPkt[44] = srcIpv6Addr.arIP[14];
        transmittedPkt[37] = expectedPkt[45] = srcIpv6Addr.arIP[15];
        transmittedPkt[38] = expectedPkt[46] = dstIpv6Addr.arIP[0];
        transmittedPkt[39] = expectedPkt[47] = dstIpv6Addr.arIP[1];
        transmittedPkt[40] = expectedPkt[48] = dstIpv6Addr.arIP[2];
        transmittedPkt[41] = expectedPkt[49] = dstIpv6Addr.arIP[3];
        transmittedPkt[42] = expectedPkt[50] = dstIpv6Addr.arIP[4];
        transmittedPkt[43] = expectedPkt[51] = dstIpv6Addr.arIP[5];
        transmittedPkt[44] = expectedPkt[52] = dstIpv6Addr.arIP[6];
        transmittedPkt[45] = expectedPkt[53] = dstIpv6Addr.arIP[7];
        transmittedPkt[46] = expectedPkt[54] = dstIpv6Addr.arIP[8];
        transmittedPkt[47] = expectedPkt[55] = dstIpv6Addr.arIP[9];
        transmittedPkt[48] = expectedPkt[56] = dstIpv6Addr.arIP[10];
        transmittedPkt[49] = expectedPkt[57] = dstIpv6Addr.arIP[11];
        transmittedPkt[50] = expectedPkt[58] = dstIpv6Addr.arIP[12];
        transmittedPkt[51] = expectedPkt[59] = dstIpv6Addr.arIP[13];
        transmittedPkt[52] = expectedPkt[60] = dstIpv6Addr.arIP[14];
        transmittedPkt[53] = expectedPkt[61] = dstIpv6Addr.arIP[15];

        /* egress E-Tag */
        expectedPkt[12] = 0x89;
        expectedPkt[13] = 0x3F;
        expectedPkt[14] = (dscpIpv6RuleDei << 4) | (dscpIpv6RulePcp << 5);
        expectedPkt[15] = 0x00;
        expectedPkt[16] = cid.octet[1];
        expectedPkt[17] = cid.octet[0];
        expectedPkt[18] = 0x00;
        expectedPkt[19] = 0x00;

        egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

        /* Send packet and check egress as expected */
        BPE_LOG_INFORMATION_MAC("Transmit untagged ipv6 packet matching DSCP rule to extended port %d.", extPort);
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  upPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.10 transmit untagged ipv6 packet to extended port that does not match any rule pattern
           expect it to egress E-tagged out of upstream with pcp, dei fields according to extended port default */

        /* ingress + egress ipV6 header */
        transmittedPkt[14] = expectedPkt[22] = (0x06 << 4) | (((portDscp << 2) & 0xF0) >> 4);
        transmittedPkt[15] = expectedPkt[23] = ((portDscp << 2) & 0x0F) << 4;

        /* egress E-Tag */
        expectedPkt[14] = (portQosMap[defaultPcp][defaultDei].newDei << 4) | (portQosMap[defaultPcp][defaultDei].newPcp << 5);

        egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

        /* Send packet and check egress as expected */
        BPE_LOG_INFORMATION_MAC("Transmit untagged ipv6 packet not matching any rule to extended port %d.", extPort);
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  upPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.11 transmit C-tagged ipv6 packet to extended port that matches DSCP rule pattern
           expect it to egress E-tagged out of upstream with pcp, dei fields according to data control qos rule */

        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = portMacDa.arEther[0];
        transmittedPkt[1] = expectedPkt[1] = portMacDa.arEther[1];
        transmittedPkt[2] = expectedPkt[2] = portMacDa.arEther[2];
        transmittedPkt[3] = expectedPkt[3] = portMacDa.arEther[3];
        transmittedPkt[4] = expectedPkt[4] = portMacDa.arEther[4];
        transmittedPkt[5] = expectedPkt[5] = portMacDa.arEther[5];

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* ingress + egress ipV6 header */
        transmittedPkt[16] = expectedPkt[24] = 0x86;
        transmittedPkt[17] = expectedPkt[25] = 0xDD;
        transmittedPkt[18] = expectedPkt[26] = (0x06 << 4) | (((ruleDscpIpv6 << 2) & 0xF0) >> 4);
        transmittedPkt[19] = expectedPkt[27] = ((ruleDscpIpv6 << 2) & 0x0F) << 4;
        transmittedPkt[20] = expectedPkt[28] = 0x00;
        transmittedPkt[21] = expectedPkt[29] = 0x00;
        transmittedPkt[22] = expectedPkt[30] = 0x00;
        transmittedPkt[23] = expectedPkt[31] = packetLen - 2*BYTES_IN_MAC_ADDR - BYTES_IN_IPV6_HEADER;
        transmittedPkt[24] = expectedPkt[32] = ipProtocol + 1;
        transmittedPkt[25] = expectedPkt[33] = 0xFF;
        transmittedPkt[26] = expectedPkt[34] = srcIpv6Addr.arIP[0];
        transmittedPkt[27] = expectedPkt[35] = srcIpv6Addr.arIP[1];
        transmittedPkt[28] = expectedPkt[36] = srcIpv6Addr.arIP[2];
        transmittedPkt[29] = expectedPkt[37] = srcIpv6Addr.arIP[3];
        transmittedPkt[30] = expectedPkt[38] = srcIpv6Addr.arIP[4];
        transmittedPkt[31] = expectedPkt[39] = srcIpv6Addr.arIP[5];
        transmittedPkt[32] = expectedPkt[40] = srcIpv6Addr.arIP[6];
        transmittedPkt[33] = expectedPkt[41] = srcIpv6Addr.arIP[7];
        transmittedPkt[34] = expectedPkt[42] = srcIpv6Addr.arIP[8];
        transmittedPkt[35] = expectedPkt[43] = srcIpv6Addr.arIP[9];
        transmittedPkt[36] = expectedPkt[44] = srcIpv6Addr.arIP[10];
        transmittedPkt[37] = expectedPkt[45] = srcIpv6Addr.arIP[11];
        transmittedPkt[38] = expectedPkt[46] = srcIpv6Addr.arIP[12];
        transmittedPkt[39] = expectedPkt[47] = srcIpv6Addr.arIP[13];
        transmittedPkt[40] = expectedPkt[48] = srcIpv6Addr.arIP[14];
        transmittedPkt[41] = expectedPkt[49] = srcIpv6Addr.arIP[15];
        transmittedPkt[42] = expectedPkt[50] = dstIpv6Addr.arIP[0];
        transmittedPkt[43] = expectedPkt[51] = dstIpv6Addr.arIP[1];
        transmittedPkt[44] = expectedPkt[52] = dstIpv6Addr.arIP[2];
        transmittedPkt[45] = expectedPkt[53] = dstIpv6Addr.arIP[3];
        transmittedPkt[46] = expectedPkt[54] = dstIpv6Addr.arIP[4];
        transmittedPkt[47] = expectedPkt[55] = dstIpv6Addr.arIP[5];
        transmittedPkt[48] = expectedPkt[56] = dstIpv6Addr.arIP[6];
        transmittedPkt[49] = expectedPkt[57] = dstIpv6Addr.arIP[7];
        transmittedPkt[50] = expectedPkt[58] = dstIpv6Addr.arIP[8];
        transmittedPkt[51] = expectedPkt[59] = dstIpv6Addr.arIP[9];
        transmittedPkt[52] = expectedPkt[60] = dstIpv6Addr.arIP[10];
        transmittedPkt[53] = expectedPkt[61] = dstIpv6Addr.arIP[11];
        transmittedPkt[54] = expectedPkt[62] = dstIpv6Addr.arIP[12];
        transmittedPkt[55] = expectedPkt[63] = dstIpv6Addr.arIP[13];
        transmittedPkt[56] = expectedPkt[64] = dstIpv6Addr.arIP[14];
        transmittedPkt[57] = expectedPkt[65] = dstIpv6Addr.arIP[15];

        /* egress E-Tag */
        expectedPkt[12] = 0x89;
        expectedPkt[13] = 0x3F;
        expectedPkt[14] = (dscpIpv6RuleDei << 4) | (dscpIpv6RulePcp << 5);
        expectedPkt[15] = 0x00;
        expectedPkt[16] = cid.octet[1];
        expectedPkt[17] = cid.octet[0];
        expectedPkt[18] = 0x00;
        expectedPkt[19] = 0x00;

        for (pcp = 0; pcp < CPSS_PCP_RANGE_CNS; pcp++)
        {
            for (dei = 0; dei < CPSS_DEI_RANGE_CNS; dei++)
            {
                /* ingress + egress C-Tag */
                transmittedPkt[12] = expectedPkt[20] = 0x81;
                transmittedPkt[13] = expectedPkt[21] = 0x00;
                transmittedPkt[14] = expectedPkt[22] = (dei << 4) | (pcp << 5);
                transmittedPkt[15] = expectedPkt[23] = vid;

                egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

                /* Send packet and check egress as expected */
                BPE_LOG_INFORMATION_MAC("Transmit C-tagged ipv6 packet with PCP=%d DEI=%d matching DSCP rule to extended port %d.", pcp, dei, extPort);
                st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                          7,
                                                                          &ingressInterface,
                                                                          upPortBmp,
                                                                          GT_FALSE,
                                                                          transmittedPkt,
                                                                          expectedPkt,
                                                                          packetLen,
                                                                          egressPacketLen);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
        }

        /* 1.12 transmit C-tagged ipv6 packet to extended port that does not match any DSCP rule pattern
           expect it to egress E-tagged out of upstream with pcp, dei fields according to port qos mapping */

        /* ingress + egress ipV6 header */
        transmittedPkt[18] = expectedPkt[26] = (0x06 << 4) | (((portDscp << 2) & 0xF0) >> 4);
        transmittedPkt[19] = expectedPkt[27] = ((portDscp << 2) & 0x0F) << 4;

        for (pcp = 0; pcp < CPSS_PCP_RANGE_CNS; pcp++)
        {
            for (dei = 0; dei < CPSS_DEI_RANGE_CNS; dei++)
            {
                /* ingress + egress C-Tag */
                transmittedPkt[12] = expectedPkt[20] = 0x81;
                transmittedPkt[13] = expectedPkt[21] = 0x00;
                transmittedPkt[14] = expectedPkt[22] = (dei << 4) | (pcp << 5);
                transmittedPkt[15] = expectedPkt[23] = vid;

                /* egress E-Tag */
                expectedPkt[12] = 0x89;
                expectedPkt[13] = 0x3F;
                expectedPkt[14] = (portQosMap[pcp][dei].newDei << 4) | (portQosMap[pcp][dei].newPcp << 5);
                expectedPkt[15] = 0x00;
                expectedPkt[16] = cid.octet[1];
                expectedPkt[17] = cid.octet[0];
                expectedPkt[18] = 0x00;
                expectedPkt[19] = 0x00;

                egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

                /* Send packet and check egress as expected */
                BPE_LOG_INFORMATION_MAC("Transmit C-tagged ipv6 packet with PCP=%d DEI=%d not matching any rule to extended port %d.", pcp, dei, extPort);
                st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                          7,
                                                                          &ingressInterface,
                                                                          upPortBmp,
                                                                          GT_FALSE,
                                                                          transmittedPkt,
                                                                          expectedPkt,
                                                                          packetLen,
                                                                          egressPacketLen);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
        }

        /* 1.13 transmit untagged LLC non SNAP packet to extended port that matches LLC non SNAP rule pattern
           expect it to egress E-tagged out of upstream with pcp, dei fields according to data control qos rule */
        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = portMacDa.arEther[0];
        transmittedPkt[1] = expectedPkt[1] = portMacDa.arEther[1];
        transmittedPkt[2] = expectedPkt[2] = portMacDa.arEther[2];
        transmittedPkt[3] = expectedPkt[3] = portMacDa.arEther[3];
        transmittedPkt[4] = expectedPkt[4] = portMacDa.arEther[4];
        transmittedPkt[5] = expectedPkt[5] = portMacDa.arEther[5];

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* IEEE 802.3 length */
        transmittedPkt[12] = expectedPkt[20] = (packetLen & 0xFF00) >> 8;
        transmittedPkt[13] = expectedPkt[21] = packetLen & 0x00FF;

        /* ingress + egress LLC header */
        transmittedPkt[14] = expectedPkt[22] = (ruleLlcDsapSsap & 0xFF00) >> 8;
        transmittedPkt[15] = expectedPkt[23] = ruleLlcDsapSsap & 0x00FF;
        transmittedPkt[16] = expectedPkt[24] = 0x03;

        /* egress E-Tag */
        expectedPkt[12] = 0x89;
        expectedPkt[13] = 0x3F;
        expectedPkt[14] = (llcNonSnapRuleDei << 4) | (llcNonSnapRulePcp << 5);
        expectedPkt[15] = 0x00;
        expectedPkt[16] = cid.octet[1];
        expectedPkt[17] = cid.octet[0];
        expectedPkt[18] = 0x00;
        expectedPkt[19] = 0x00;

        egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

        /* Send packet and check egress as expected */
        BPE_LOG_INFORMATION_MAC("Transmit untagged LLC non SNAP packet matching LLC non SNAP rule to extended port %d.", extPort);
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  upPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.14 transmit untagged LLC non SNAP packet to extended port that does not match any rule pattern
           expect it to egress E-tagged out of upstream with pcp, dei fields according to extended port default */

        /* ingress + egress LLC header */
        transmittedPkt[14] = expectedPkt[22] = (portLlcDsapSsap & 0xFF00) >> 8;
        transmittedPkt[15] = expectedPkt[23] = portLlcDsapSsap & 0x00FF;
        transmittedPkt[16] = expectedPkt[24] = 0x03;

        /* egress E-Tag */
        expectedPkt[14] = (portQosMap[defaultPcp][defaultDei].newDei << 4) | (portQosMap[defaultPcp][defaultDei].newPcp << 5);

        egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

        /* Send packet and check egress as expected */
        BPE_LOG_INFORMATION_MAC("Transmit untagged LLC non SNAP packet not matching any rule to extended port %d.", extPort);
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  upPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.15 transmit C-tagged LLC non SNAP packet to extended port that matches LLC non SNAP rule pattern
           expect it to egress E-tagged out of upstream with pcp, dei fields according to data control qos rule */

        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = portMacDa.arEther[0];
        transmittedPkt[1] = expectedPkt[1] = portMacDa.arEther[1];
        transmittedPkt[2] = expectedPkt[2] = portMacDa.arEther[2];
        transmittedPkt[3] = expectedPkt[3] = portMacDa.arEther[3];
        transmittedPkt[4] = expectedPkt[4] = portMacDa.arEther[4];
        transmittedPkt[5] = expectedPkt[5] = portMacDa.arEther[5];

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* IEEE 802.3 length */
        transmittedPkt[16] = expectedPkt[24] = (packetLen & 0xFF00) >> 8;
        transmittedPkt[17] = expectedPkt[25] = packetLen & 0x00FF;

        /* ingress + egress LLC header */
        transmittedPkt[18] = expectedPkt[26] = (ruleLlcDsapSsap & 0xFF00) >> 8;
        transmittedPkt[19] = expectedPkt[27] = ruleLlcDsapSsap & 0x00FF;
        transmittedPkt[20] = expectedPkt[28] = 0x03;

        /* egress E-Tag */
        expectedPkt[12] = 0x89;
        expectedPkt[13] = 0x3F;
        expectedPkt[14] = (llcNonSnapRuleDei << 4) | (llcNonSnapRulePcp << 5);
        expectedPkt[15] = 0x00;
        expectedPkt[16] = cid.octet[1];
        expectedPkt[17] = cid.octet[0];
        expectedPkt[18] = 0x00;
        expectedPkt[19] = 0x00;

        for (pcp = 0; pcp < CPSS_PCP_RANGE_CNS; pcp++)
        {
            for (dei = 0; dei < CPSS_DEI_RANGE_CNS; dei++)
            {
                /* ingress + egress C-Tag */
                transmittedPkt[12] = expectedPkt[20] = 0x81;
                transmittedPkt[13] = expectedPkt[21] = 0x00;
                transmittedPkt[14] = expectedPkt[22] = (dei << 4) | (pcp << 5);
                transmittedPkt[15] = expectedPkt[23] = vid;

                egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

                /* Send packet and check egress as expected */
                BPE_LOG_INFORMATION_MAC("Transmit C-tagged LLC non SNAP packet with PCP=%d DEI=%d matching LLC non SNAP rule to extended port %d.", pcp, dei, extPort);
                st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                          7,
                                                                          &ingressInterface,
                                                                          upPortBmp,
                                                                          GT_FALSE,
                                                                          transmittedPkt,
                                                                          expectedPkt,
                                                                          packetLen,
                                                                          egressPacketLen);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
        }

        /* 1.16 transmit C-tagged LLC non SNAP packet to extended port that does not match any rule pattern
           expect it to egress E-tagged out of upstream with pcp, dei fields according to port qos mapping */

        /* ingress + egress LLC header */
        transmittedPkt[18] = expectedPkt[26] = (portLlcDsapSsap & 0xFF00) >> 8;
        transmittedPkt[19] = expectedPkt[27] = portLlcDsapSsap & 0x00FF;
        transmittedPkt[20] = expectedPkt[28] = 0x03;

        for (pcp = 0; pcp < CPSS_PCP_RANGE_CNS; pcp++)
        {
            for (dei = 0; dei < CPSS_DEI_RANGE_CNS; dei++)
            {
                /* ingress + egress C-Tag */
                transmittedPkt[12] = expectedPkt[20] = 0x81;
                transmittedPkt[13] = expectedPkt[21] = 0x00;
                transmittedPkt[14] = expectedPkt[22] = (dei << 4) | (pcp << 5);
                transmittedPkt[15] = expectedPkt[23] = vid;

                /* egress E-Tag */
                expectedPkt[12] = 0x89;
                expectedPkt[13] = 0x3F;
                expectedPkt[14] = (portQosMap[pcp][dei].newDei << 4) | (portQosMap[pcp][dei].newPcp << 5);
                expectedPkt[15] = 0x00;
                expectedPkt[16] = cid.octet[1];
                expectedPkt[17] = cid.octet[0];
                expectedPkt[18] = 0x00;
                expectedPkt[19] = 0x00;

                egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

                /* Send packet and check egress as expected */
                BPE_LOG_INFORMATION_MAC("Transmit C-tagged LLC non SNAP packet with PCP=%d DEI=%d not matching any rule to extended port %d.", pcp, dei, extPort);
                st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                          7,
                                                                          &ingressInterface,
                                                                          upPortBmp,
                                                                          GT_FALSE,
                                                                          transmittedPkt,
                                                                          expectedPkt,
                                                                          packetLen,
                                                                          egressPacketLen);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
        }

        /* 1.17 transmit untagged LLC SNAP packet to extended port that matches LLC SNAP rule pattern
           expect it to egress E-tagged out of upstream with pcp, dei fields according to data control qos rule */
        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = portMacDa.arEther[0];
        transmittedPkt[1] = expectedPkt[1] = portMacDa.arEther[1];
        transmittedPkt[2] = expectedPkt[2] = portMacDa.arEther[2];
        transmittedPkt[3] = expectedPkt[3] = portMacDa.arEther[3];
        transmittedPkt[4] = expectedPkt[4] = portMacDa.arEther[4];
        transmittedPkt[5] = expectedPkt[5] = portMacDa.arEther[5];

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* IEEE 802.3 length */
        transmittedPkt[12] = expectedPkt[20] = (packetLen & 0xFF00) >> 8;
        transmittedPkt[13] = expectedPkt[21] = packetLen & 0x00FF;

        /* ingress + egress LLC SNAP header */
        transmittedPkt[14] = expectedPkt[22] = 0xAA;
        transmittedPkt[15] = expectedPkt[23] = 0xAA;
        transmittedPkt[16] = expectedPkt[24] = 0x03;
        transmittedPkt[17] = expectedPkt[25] = 0x00;
        transmittedPkt[18] = expectedPkt[26] = 0x00;
        transmittedPkt[19] = expectedPkt[27] = 0x00;
        transmittedPkt[20] = expectedPkt[28] = (ruleLlcSnapEtherType & 0xFF00) >> 8;
        transmittedPkt[21] = expectedPkt[29] = ruleLlcSnapEtherType & 0x00FF;

        /* egress E-Tag */
        expectedPkt[12] = 0x89;
        expectedPkt[13] = 0x3F;
        expectedPkt[14] = (llcSnapRuleDei << 4) | (llcSnapRulePcp << 5);
        expectedPkt[15] = 0x00;
        expectedPkt[16] = cid.octet[1];
        expectedPkt[17] = cid.octet[0];
        expectedPkt[18] = 0x00;
        expectedPkt[19] = 0x00;

        egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

        /* Send packet and check egress as expected */
        BPE_LOG_INFORMATION_MAC("Transmit untagged LLC SNAP packet matching LLC SNAP rule to extended port %d.", extPort);
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  upPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.18 transmit untagged LLC SNAP packet to extended port that does not match any rule pattern
           expect it to egress E-tagged out of upstream with pcp, dei fields according to extended port default */

        /* ingress + egress LLC SNAP header */
        transmittedPkt[20] = expectedPkt[28] = (portLlcSnapEtherType & 0xFF00) >> 8;
        transmittedPkt[21] = expectedPkt[29] = portLlcSnapEtherType & 0x00FF;

        /* egress E-Tag */
        expectedPkt[14] = (portQosMap[defaultPcp][defaultDei].newDei << 4) | (portQosMap[defaultPcp][defaultDei].newPcp << 5);

        egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

        /* Send packet and check egress as expected */
        BPE_LOG_INFORMATION_MAC("Transmit untagged LLC SNAP packet not matching any rule to extended port %d.", extPort);
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  upPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.19 transmit C-tagged LLC SNAP packet to extended port that matches LLC SNAP rule pattern
           expect it to egress E-tagged out of upstream with pcp, dei fields according to data control qos rule */

        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = portMacDa.arEther[0];
        transmittedPkt[1] = expectedPkt[1] = portMacDa.arEther[1];
        transmittedPkt[2] = expectedPkt[2] = portMacDa.arEther[2];
        transmittedPkt[3] = expectedPkt[3] = portMacDa.arEther[3];
        transmittedPkt[4] = expectedPkt[4] = portMacDa.arEther[4];
        transmittedPkt[5] = expectedPkt[5] = portMacDa.arEther[5];

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* IEEE 802.3 length */
        transmittedPkt[16] = expectedPkt[24] = (packetLen & 0xFF00) >> 8;
        transmittedPkt[17] = expectedPkt[25] = packetLen & 0x00FF;

        /* ingress + egress LLC SNAP header */
        transmittedPkt[18] = expectedPkt[26] = 0xAA;
        transmittedPkt[19] = expectedPkt[27] = 0xAA;
        transmittedPkt[20] = expectedPkt[28] = 0x03;
        transmittedPkt[21] = expectedPkt[29] = 0x00;
        transmittedPkt[22] = expectedPkt[30] = 0x00;
        transmittedPkt[23] = expectedPkt[31] = 0x00;
        transmittedPkt[24] = expectedPkt[32] = (ruleLlcSnapEtherType & 0xFF00) >> 8;
        transmittedPkt[25] = expectedPkt[33] = ruleLlcSnapEtherType & 0x00FF;

        /* egress E-Tag */
        expectedPkt[12] = 0x89;
        expectedPkt[13] = 0x3F;
        expectedPkt[14] = (llcSnapRuleDei << 4) | (llcSnapRulePcp << 5);
        expectedPkt[15] = 0x00;
        expectedPkt[16] = cid.octet[1];
        expectedPkt[17] = cid.octet[0];
        expectedPkt[18] = 0x00;
        expectedPkt[19] = 0x00;

        for (pcp = 0; pcp < CPSS_PCP_RANGE_CNS; pcp++)
        {
            for (dei = 0; dei < CPSS_DEI_RANGE_CNS; dei++)
            {
                /* ingress + egress C-Tag */
                transmittedPkt[12] = expectedPkt[20] = 0x81;
                transmittedPkt[13] = expectedPkt[21] = 0x00;
                transmittedPkt[14] = expectedPkt[22] = (dei << 4) | (pcp << 5);
                transmittedPkt[15] = expectedPkt[23] = vid;

                egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

                /* Send packet and check egress as expected */
                BPE_LOG_INFORMATION_MAC("Transmit C-tagged LLC SNAP packet with PCP=%d DEI=%d matching LLC SNAP rule to extended port %d.", pcp, dei, extPort);
                st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                          7,
                                                                          &ingressInterface,
                                                                          upPortBmp,
                                                                          GT_FALSE,
                                                                          transmittedPkt,
                                                                          expectedPkt,
                                                                          packetLen,
                                                                          egressPacketLen);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
        }

        /* 1.20 transmit C-tagged LLC SNAP packet to extended port that does not match any rule pattern
           expect it to egress E-tagged out of upstream with pcp, dei fields according to port qos mapping */

        /* ingress + egress LLC SNAP header */
        transmittedPkt[24] = expectedPkt[32] = (portLlcSnapEtherType & 0xFF00) >> 8;
        transmittedPkt[25] = expectedPkt[33] = portLlcSnapEtherType & 0x00FF;

        for (pcp = 0; pcp < CPSS_PCP_RANGE_CNS; pcp++)
        {
            for (dei = 0; dei < CPSS_DEI_RANGE_CNS; dei++)
            {
                /* ingress + egress C-Tag */
                transmittedPkt[12] = expectedPkt[20] = 0x81;
                transmittedPkt[13] = expectedPkt[21] = 0x00;
                transmittedPkt[14] = expectedPkt[22] = (dei << 4) | (pcp << 5);
                transmittedPkt[15] = expectedPkt[23] = vid;

                /* egress E-Tag */
                expectedPkt[12] = 0x89;
                expectedPkt[13] = 0x3F;
                expectedPkt[14] = (portQosMap[pcp][dei].newDei << 4) | (portQosMap[pcp][dei].newPcp << 5);
                expectedPkt[15] = 0x00;
                expectedPkt[16] = cid.octet[1];
                expectedPkt[17] = cid.octet[0];
                expectedPkt[18] = 0x00;
                expectedPkt[19] = 0x00;

                egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

                /* Send packet and check egress as expected */
                BPE_LOG_INFORMATION_MAC("Transmit C-tagged LLC SNAP packet with PCP=%d DEI=%d not matching any rule to extended port %d.", pcp, dei, extPort);
                st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                          7,
                                                                          &ingressInterface,
                                                                          upPortBmp,
                                                                          GT_FALSE,
                                                                          transmittedPkt,
                                                                          expectedPkt,
                                                                          packetLen,
                                                                          egressPacketLen);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
        }

        /* special enhanced UT restore configurations*/
        st = prvTgfPxHalBpeEnhancedUtExtended2UpstreamConfigRestore(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

UTF_TEST_CASE_MAC(prvTgfPxHalBpe_E2U_UC_dataControlQosRule_IP2ME)
{
    GT_U8                                    dev = 0, ipProtocol = 0x11;
    GT_STATUS                                st;
    GT_PORT_NUM                              extPort = 0;
    CPSS_PX_PORTS_BMP                        upPortBmp = 0;
    GT_PORT_NUM                              upPort = 12;
    GT_U32                                   packetLen = 100;
    GT_U32                                   egressPacketLen[PRV_CPSS_PX_PORTS_NUM_CNS - 1];
    GT_U32                                   pcp, dei;
    GT_U32                                   defaultDei = 1, defaultPcp = 4;       
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC       egressInterface, ingressInterface;
    CPSS_PX_HAL_BPE_QOSMAP_STC               portQosMap[CPSS_PCP_RANGE_CNS][CPSS_DEI_RANGE_CNS];
    CPSS_PX_HAL_BPE_QOSMAP_DATA_STC          ruleQosMap;
    CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC rulePattern, ruleMask;    
    union                                {
        GT_U16                               octetPair;
        GT_U8                                octet[2];
    }cid;
    cid.octetPair = 1;

     /* Skip test for revision A0 */
    if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev) == 0)
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Create qos mapping */
    for(pcp = 0; pcp < CPSS_PCP_RANGE_CNS; pcp++)
    {
        for(dei = 0; dei < CPSS_DEI_RANGE_CNS; dei++)
        {
            portQosMap[pcp][dei].newDei = dei;
            portQosMap[pcp][dei].newPcp = pcp;
        }
    }   

    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(upPortBmp, upPort);

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Perform common configuration */
        ingressInterface.type = egressInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        ingressInterface.devPort.devNum = egressInterface.devPort.devNum = dev;
        ingressInterface.devPort.portNum = extPort;
        egressInterface.devPort.portNum = upPort;

        st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface.type, egressInterface.devPort.devNum, egressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);

        st = cpssPxHalBpeInterfaceTypeSet(dev, &ingressInterface, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, ingressInterface.type, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);

        st = cpssPxHalBpeNumOfChannelsSet(dev, _4K - 20, _4K);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, _4K - 20, _4K);

        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &ingressInterface, cid.octetPair);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, cid.octetPair);

        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid.octetPair, &ingressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        st = cpssPxHalBpePortExtendedUpstreamSet(dev, extPort, &egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort);

        st = cpssPxHalBpePortExtendedQosDefaultPcpDeiSet(dev, extPort, defaultPcp, defaultDei);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, extPort, defaultPcp, defaultDei);

        st = cpssPxHalBpePortExtendedQosMapSet(dev, extPort, portQosMap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort);

        /* Configure qos rule to match ipv4 macDA, DSCP, IP protocol, DIP and prefixLen */
        cpssOsMemSet(&rulePattern, 0x00, sizeof(CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC));
        cpssOsMemSet(&ruleMask, 0x00, sizeof(CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC));
        
        cpssOsMemSet(&(rulePattern.macDa), 0x0, sizeof(GT_ETHERADDR));
        cpssOsMemSet(&(ruleMask.macDa), 0xFF, sizeof(GT_ETHERADDR));

        rulePattern.dscp = 0x3F;        
        ruleMask.dscp = 0x3F;

        rulePattern.ipProtocol = ipProtocol;
        ruleMask.ipProtocol = 0xFF;

        rulePattern.dip.arIP[0] = 0x01;
        rulePattern.dip.arIP[1] = 0x02;
        rulePattern.dip.arIP[2] = 0x03;
        rulePattern.dip.arIP[3] = 0x04;
        cpssOsMemSet(&(ruleMask.dip) ,0xFF, sizeof(GT_IPADDR)); /* prefix len = 32 */

        rulePattern.etherType = 0xf800;
        ruleMask.etherType = 0xFFFF;

        ruleQosMap.newDei = defaultDei;
        ruleQosMap.newPcp = defaultPcp;
        ruleQosMap.queue = 0;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, 1, &ruleQosMap, CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV4_E, &ruleMask, &rulePattern);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, 1);

        /* Configure qos rule to match ipv4 macDA, DSCP, IP protocol, DIP and prefixLen */
        cpssOsMemSet(&rulePattern, 0x00, sizeof(CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC));
        cpssOsMemSet(&ruleMask, 0x00, sizeof(CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC));
        
        cpssOsMemSet(&(rulePattern.macDa), 0x1, sizeof(GT_ETHERADDR));
        cpssOsMemSet(&(ruleMask.macDa), 0xFF, sizeof(GT_ETHERADDR));

        rulePattern.dscp = 0x0;        
        ruleMask.dscp = 0x3F;

        rulePattern.ipProtocol = ipProtocol;
        ruleMask.ipProtocol = 0xFF;

        rulePattern.dip.arIP[0] = 0xE0;
        rulePattern.dip.arIP[1] = 0x00;
        rulePattern.dip.arIP[2] = 0x00;
        rulePattern.dip.arIP[3] = 0x00;
        cpssOsMemSet(&(ruleMask.dip) ,0xFF, sizeof(GT_IPADDR));
        ruleMask.dip.arIP[3] = 0x0; /* prefix len = 24 */        

        rulePattern.etherType = 0x0800;
        ruleMask.etherType = 0xFFFF;

        ruleQosMap.newDei = defaultDei;
        ruleQosMap.newPcp = defaultPcp;
        ruleQosMap.queue = 0;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, 2, &ruleQosMap, CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV4_E, &ruleMask, &rulePattern);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, 2);

        /* Configure qos rule to match ipv6 macDA, DSCP, IP protocol, DIP and prefixLen */
        cpssOsMemSet(&rulePattern, 0x00, sizeof(CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC));
        cpssOsMemSet(&ruleMask, 0x00, sizeof(CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC));
        
        cpssOsMemSet(&(rulePattern.macDa), 0x2, sizeof(GT_ETHERADDR));
        cpssOsMemSet(&(ruleMask.macDa), 0xFF, sizeof(GT_ETHERADDR));

        rulePattern.dscp = 0x15;        
        ruleMask.dscp = 0x3F;

        rulePattern.ipProtocol = ipProtocol+1;
        ruleMask.ipProtocol = 0xFF;

        rulePattern.dipv6.arIP[0] = 0x00;
        rulePattern.dipv6.arIP[1] = 0x01;
        rulePattern.dipv6.arIP[2] = 0x00;
        rulePattern.dipv6.arIP[3] = 0x02;  
        rulePattern.dipv6.arIP[4] = 0x00;
        rulePattern.dipv6.arIP[5] = 0x03;
        rulePattern.dipv6.arIP[6] = 0x00;
        rulePattern.dipv6.arIP[7] = 0x04;  
        rulePattern.dipv6.arIP[8] = 0x00;
        rulePattern.dipv6.arIP[9] = 0x05;
        rulePattern.dipv6.arIP[10] = 0x00;
        rulePattern.dipv6.arIP[11] = 0x06;  
        rulePattern.dipv6.arIP[12] = 0x00;
        rulePattern.dipv6.arIP[13] = 0x07;
        rulePattern.dipv6.arIP[14] = 0x00;
        rulePattern.dipv6.arIP[15] = 0x08;        
        cpssOsMemSet(&(ruleMask.dipv6) ,0xFF, sizeof(GT_IPV6ADDR)); /* prefix len = 128 */              

        rulePattern.etherType = 0x86DD;
        ruleMask.etherType = 0xFFFF;

        ruleQosMap.newDei = defaultDei;
        ruleQosMap.newPcp = defaultPcp;
        ruleQosMap.queue = 0;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, 3, &ruleQosMap, CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV6_E, &ruleMask, &rulePattern);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, 3);

        /* Configure qos rule to match ipv6 macDA, DSCP, IP protocol, DIP and prefixLen */
        cpssOsMemSet(&rulePattern, 0x00, sizeof(CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC));
        cpssOsMemSet(&ruleMask, 0x00, sizeof(CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC));
        
        cpssOsMemSet(&(rulePattern.macDa), 0x3, sizeof(GT_ETHERADDR));
        cpssOsMemSet(&(ruleMask.macDa), 0xFF, sizeof(GT_ETHERADDR));

        rulePattern.dscp = 0x2A;        
        ruleMask.dscp = 0x3F;

        rulePattern.ipProtocol = ipProtocol+1;
        ruleMask.ipProtocol = 0xFF;

        rulePattern.dipv6.arIP[0] = 0xFF;
        rulePattern.dipv6.arIP[1] = 0x01;
        rulePattern.dipv6.arIP[2] = 0x00;
        rulePattern.dipv6.arIP[3] = 0x00;  
        rulePattern.dipv6.arIP[4] = 0x00;
        rulePattern.dipv6.arIP[5] = 0x00;
        rulePattern.dipv6.arIP[6] = 0x00;
        rulePattern.dipv6.arIP[7] = 0x00;  
        rulePattern.dipv6.arIP[8] = 0x00;
        rulePattern.dipv6.arIP[9] = 0x00;
        rulePattern.dipv6.arIP[10] = 0x00;
        rulePattern.dipv6.arIP[11] = 0x00;  
        rulePattern.dipv6.arIP[12] = 0x00;
        rulePattern.dipv6.arIP[13] = 0x00;
        rulePattern.dipv6.arIP[14] = 0x00;
        rulePattern.dipv6.arIP[15] = 0x00;      
        
        cpssOsMemSet(&(ruleMask.dipv6) ,0x0, sizeof(GT_IPV6ADDR)); 
        ruleMask.dipv6.arIP[0] = 0xFF;
        ruleMask.dipv6.arIP[1] = 0xFF; /* prefix len = 16 */              
              
        rulePattern.etherType = 0x86DD;
        ruleMask.etherType = 0xFFFF;

        ruleQosMap.newDei = defaultDei;
        ruleQosMap.newPcp = defaultPcp;
        ruleQosMap.queue = 0;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, 4, &ruleQosMap, CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV6_E, &ruleMask, &rulePattern);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, 4);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtExtended2UpstreamConfigSet(dev, extPort);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, extPort);

        /* 1.1 transmit untagged ipv4 packet to extended port that matches DSCP rule pattern
           expect it to egress E-tagged out of upstream with pcp, dei fields according to data control qos rule */
        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = 0x00;
        transmittedPkt[1] = expectedPkt[1] = 0x00;
        transmittedPkt[2] = expectedPkt[2] = 0x00;
        transmittedPkt[3] = expectedPkt[3] = 0x00;
        transmittedPkt[4] = expectedPkt[4] = 0x00;
        transmittedPkt[5] = expectedPkt[5] = 0x00;

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* ingress + egress ipV4 header */
        transmittedPkt[12] = expectedPkt[20] = 0x08;
        transmittedPkt[13] = expectedPkt[21] = 0x00;
        transmittedPkt[14] = expectedPkt[22] = 0x45;
        transmittedPkt[15] = expectedPkt[23] = 0x3F << 2;
        transmittedPkt[16] = expectedPkt[24] = 0x00;
        transmittedPkt[17] = expectedPkt[25] = 0x2E;
        transmittedPkt[18] = expectedPkt[26] = 0x00;
        transmittedPkt[19] = expectedPkt[27] = 0x00;
        transmittedPkt[20] = expectedPkt[28] = 0x00;
        transmittedPkt[21] = expectedPkt[29] = 0x00;
        transmittedPkt[22] = expectedPkt[30] = 0x40;
        transmittedPkt[23] = expectedPkt[31] = ipProtocol;
        transmittedPkt[24] = expectedPkt[32] = 0x74; /*checksum*/
        transmittedPkt[25] = expectedPkt[33] = 0x79; /*checksum*/
        transmittedPkt[26] = expectedPkt[34] = 0x01;
        transmittedPkt[27] = expectedPkt[35] = 0x01;
        transmittedPkt[28] = expectedPkt[36] = 0x01;
        transmittedPkt[29] = expectedPkt[37] = 0x01;
        transmittedPkt[30] = expectedPkt[38] = 0x01;
        transmittedPkt[31] = expectedPkt[39] = 0x02;
        transmittedPkt[32] = expectedPkt[40] = 0x03;
        transmittedPkt[33] = expectedPkt[41] = 0x04;

        /* egress E-Tag */
        expectedPkt[12] = 0x89;
        expectedPkt[13] = 0x3F;
        expectedPkt[14] = (defaultDei << 4) | (defaultPcp << 5);
        expectedPkt[15] = 0x00;
        expectedPkt[16] = cid.octet[1];
        expectedPkt[17] = cid.octet[0];
        expectedPkt[18] = 0x00;
        expectedPkt[19] = 0x00;

        egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

        /* Send packet and check egress as expected */
        BPE_LOG_INFORMATION_MAC("Transmit untagged ipv4 packet matching macDA DSCP IPv4 address prefix len=32 rule to extended port %d.", extPort);
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  upPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

         /* 1.2 transmit untagged ipv4 packet to extended port that matches DSCP rule pattern
           expect it to egress E-tagged out of upstream with pcp, dei fields according to data control qos rule */
        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = 0x01;
        transmittedPkt[1] = expectedPkt[1] = 0x01;
        transmittedPkt[2] = expectedPkt[2] = 0x01;
        transmittedPkt[3] = expectedPkt[3] = 0x01;
        transmittedPkt[4] = expectedPkt[4] = 0x01;
        transmittedPkt[5] = expectedPkt[5] = 0x01;

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* ingress + egress ipV4 header */
        transmittedPkt[12] = expectedPkt[20] = 0x08;
        transmittedPkt[13] = expectedPkt[21] = 0x00;
        transmittedPkt[14] = expectedPkt[22] = 0x45;
        transmittedPkt[15] = expectedPkt[23] = 0x00 << 2;
        transmittedPkt[16] = expectedPkt[24] = 0x00;
        transmittedPkt[17] = expectedPkt[25] = 0x2E;
        transmittedPkt[18] = expectedPkt[26] = 0x00;
        transmittedPkt[19] = expectedPkt[27] = 0x00;
        transmittedPkt[20] = expectedPkt[28] = 0x00;
        transmittedPkt[21] = expectedPkt[29] = 0x00;
        transmittedPkt[22] = expectedPkt[30] = 0x40;
        transmittedPkt[23] = expectedPkt[31] = ipProtocol;
        transmittedPkt[24] = expectedPkt[32] = 0x98; /*checksum*/
        transmittedPkt[25] = expectedPkt[33] = 0xBD; /*checksum*/
        transmittedPkt[26] = expectedPkt[34] = 0x01;
        transmittedPkt[27] = expectedPkt[35] = 0x01;
        transmittedPkt[28] = expectedPkt[36] = 0x01;
        transmittedPkt[29] = expectedPkt[37] = 0x01;
        transmittedPkt[30] = expectedPkt[38] = 0xE0;
        transmittedPkt[31] = expectedPkt[39] = 0x00;
        transmittedPkt[32] = expectedPkt[40] = 0x00;
        transmittedPkt[33] = expectedPkt[41] = 0x00;

        /* egress E-Tag */
        expectedPkt[12] = 0x89;
        expectedPkt[13] = 0x3F;
        expectedPkt[14] = (defaultDei << 4) | (defaultPcp << 5);
        expectedPkt[15] = 0x00;
        expectedPkt[16] = cid.octet[1];
        expectedPkt[17] = cid.octet[0];
        expectedPkt[18] = 0x00;
        expectedPkt[19] = 0x00;

        egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

        /* Send packet and check egress as expected */
        BPE_LOG_INFORMATION_MAC("Transmit untagged ipv4 packet matching macDA DSCP IPv4 address prefix len=24 rule to extended port %d.", extPort);
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  upPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.3 transmit untagged ipv6 packet to extended port that matches DSCP rule pattern
           expect it to egress E-tagged out of upstream with pcp, dei fields according to data control qos rule */
        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = 0x02;
        transmittedPkt[1] = expectedPkt[1] = 0x02;
        transmittedPkt[2] = expectedPkt[2] = 0x02;
        transmittedPkt[3] = expectedPkt[3] = 0x02;
        transmittedPkt[4] = expectedPkt[4] = 0x02;
        transmittedPkt[5] = expectedPkt[5] = 0x02;

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* ingress + egress ipV6 header */
        transmittedPkt[12] = expectedPkt[20] = 0x86;
        transmittedPkt[13] = expectedPkt[21] = 0xDD;
        transmittedPkt[14] = expectedPkt[22] = (0x06 << 4) | (((0x15 << 2) & 0xF0) >> 4);
        transmittedPkt[15] = expectedPkt[23] = ((0x15 << 2) & 0x0F) << 4;
        transmittedPkt[16] = expectedPkt[24] = 0x00;
        transmittedPkt[17] = expectedPkt[25] = 0x00;
        transmittedPkt[18] = expectedPkt[26] = 0x00;
        transmittedPkt[19] = expectedPkt[27] = packetLen - 2*BYTES_IN_MAC_ADDR - BYTES_IN_IPV6_HEADER;
        transmittedPkt[20] = expectedPkt[28] = ipProtocol + 1;
        transmittedPkt[21] = expectedPkt[29] = 0xFF;
        transmittedPkt[22] = expectedPkt[30] = 0x01;
        transmittedPkt[23] = expectedPkt[31] = 0x01;
        transmittedPkt[24] = expectedPkt[32] = 0x01;
        transmittedPkt[25] = expectedPkt[33] = 0x01;
        transmittedPkt[26] = expectedPkt[34] = 0x01;
        transmittedPkt[27] = expectedPkt[35] = 0x01;
        transmittedPkt[28] = expectedPkt[36] = 0x01;
        transmittedPkt[29] = expectedPkt[37] = 0x01;
        transmittedPkt[30] = expectedPkt[38] = 0x01;
        transmittedPkt[31] = expectedPkt[39] = 0x01;
        transmittedPkt[32] = expectedPkt[40] = 0x01;
        transmittedPkt[33] = expectedPkt[41] = 0x01;
        transmittedPkt[34] = expectedPkt[42] = 0x01;
        transmittedPkt[35] = expectedPkt[43] = 0x01;
        transmittedPkt[36] = expectedPkt[44] = 0x01;
        transmittedPkt[37] = expectedPkt[45] = 0x01;
        transmittedPkt[38] = expectedPkt[46] = 0x00;
        transmittedPkt[39] = expectedPkt[47] = 0x01;
        transmittedPkt[40] = expectedPkt[48] = 0x00;
        transmittedPkt[41] = expectedPkt[49] = 0x02;
        transmittedPkt[42] = expectedPkt[50] = 0x00;
        transmittedPkt[43] = expectedPkt[51] = 0x03;
        transmittedPkt[44] = expectedPkt[52] = 0x00;
        transmittedPkt[45] = expectedPkt[53] = 0x04;
        transmittedPkt[46] = expectedPkt[54] = 0x00;
        transmittedPkt[47] = expectedPkt[55] = 0x05;
        transmittedPkt[48] = expectedPkt[56] = 0x00;
        transmittedPkt[49] = expectedPkt[57] = 0x06;
        transmittedPkt[50] = expectedPkt[58] = 0x00;
        transmittedPkt[51] = expectedPkt[59] = 0x07;
        transmittedPkt[52] = expectedPkt[60] = 0x00;
        transmittedPkt[53] = expectedPkt[61] = 0x08;

        /* egress E-Tag */
        expectedPkt[12] = 0x89;
        expectedPkt[13] = 0x3F;
        expectedPkt[14] = (defaultDei << 4) | (defaultPcp << 5);
        expectedPkt[15] = 0x00;
        expectedPkt[16] = cid.octet[1];
        expectedPkt[17] = cid.octet[0];
        expectedPkt[18] = 0x00;
        expectedPkt[19] = 0x00;

        egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

        /* Send packet and check egress as expected */
        BPE_LOG_INFORMATION_MAC("Transmit untagged ipv6 packet matching macDA DSCP IPv6 address prefix len=128 rule to extended port %d.", extPort);
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  upPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.4 transmit untagged ipv6 packet to extended port that matches DSCP rule pattern
           expect it to egress E-tagged out of upstream with pcp, dei fields according to data control qos rule */
        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = 0x03;
        transmittedPkt[1] = expectedPkt[1] = 0x03;
        transmittedPkt[2] = expectedPkt[2] = 0x03;
        transmittedPkt[3] = expectedPkt[3] = 0x03;
        transmittedPkt[4] = expectedPkt[4] = 0x03;
        transmittedPkt[5] = expectedPkt[5] = 0x03;

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* ingress + egress ipV6 header */
        transmittedPkt[12] = expectedPkt[20] = 0x86;
        transmittedPkt[13] = expectedPkt[21] = 0xDD;
        transmittedPkt[14] = expectedPkt[22] = (0x06 << 4) | (((0x2A << 2) & 0xF0) >> 4);
        transmittedPkt[15] = expectedPkt[23] = ((0x2A << 2) & 0x0F) << 4;
        transmittedPkt[16] = expectedPkt[24] = 0x00;
        transmittedPkt[17] = expectedPkt[25] = 0x00;
        transmittedPkt[18] = expectedPkt[26] = 0x00;
        transmittedPkt[19] = expectedPkt[27] = packetLen - 2*BYTES_IN_MAC_ADDR - BYTES_IN_IPV6_HEADER;
        transmittedPkt[20] = expectedPkt[28] = ipProtocol + 1;
        transmittedPkt[21] = expectedPkt[29] = 0xFF;
        transmittedPkt[22] = expectedPkt[30] = 0x01;
        transmittedPkt[23] = expectedPkt[31] = 0x01;
        transmittedPkt[24] = expectedPkt[32] = 0x01;
        transmittedPkt[25] = expectedPkt[33] = 0x01;
        transmittedPkt[26] = expectedPkt[34] = 0x01;
        transmittedPkt[27] = expectedPkt[35] = 0x01;
        transmittedPkt[28] = expectedPkt[36] = 0x01;
        transmittedPkt[29] = expectedPkt[37] = 0x01;
        transmittedPkt[30] = expectedPkt[38] = 0x01;
        transmittedPkt[31] = expectedPkt[39] = 0x01;
        transmittedPkt[32] = expectedPkt[40] = 0x01;
        transmittedPkt[33] = expectedPkt[41] = 0x01;
        transmittedPkt[34] = expectedPkt[42] = 0x01;
        transmittedPkt[35] = expectedPkt[43] = 0x01;
        transmittedPkt[36] = expectedPkt[44] = 0x01;
        transmittedPkt[37] = expectedPkt[45] = 0x01;
        transmittedPkt[38] = expectedPkt[46] = 0xFF;
        transmittedPkt[39] = expectedPkt[47] = 0x01;
        transmittedPkt[40] = expectedPkt[48] = 0x00;
        transmittedPkt[41] = expectedPkt[49] = 0x00;
        transmittedPkt[42] = expectedPkt[50] = 0x00;
        transmittedPkt[43] = expectedPkt[51] = 0x00;
        transmittedPkt[44] = expectedPkt[52] = 0x00;
        transmittedPkt[45] = expectedPkt[53] = 0x00;
        transmittedPkt[46] = expectedPkt[54] = 0x00;
        transmittedPkt[47] = expectedPkt[55] = 0x00;
        transmittedPkt[48] = expectedPkt[56] = 0x00;
        transmittedPkt[49] = expectedPkt[57] = 0x00;
        transmittedPkt[50] = expectedPkt[58] = 0x00;
        transmittedPkt[51] = expectedPkt[59] = 0x00;
        transmittedPkt[52] = expectedPkt[60] = 0x00;
        transmittedPkt[53] = expectedPkt[61] = 0x00;

        /* egress E-Tag */
        expectedPkt[12] = 0x89;
        expectedPkt[13] = 0x3F;
        expectedPkt[14] = (defaultDei << 4) | (defaultPcp << 5);
        expectedPkt[15] = 0x00;
        expectedPkt[16] = cid.octet[1];
        expectedPkt[17] = cid.octet[0];
        expectedPkt[18] = 0x00;
        expectedPkt[19] = 0x00;

        egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

        /* Send packet and check egress as expected */
        BPE_LOG_INFORMATION_MAC("Transmit untagged ipv6 packet matching macDA DSCP IPv6 address prefix len=16 rule to extended port %d.", extPort);
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  upPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* Configure qos rule to match ipv4 macDA, DSCP, IP protocol, DIP and prefixLen */
        cpssOsMemSet(&rulePattern, 0x00, sizeof(CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC));
        cpssOsMemSet(&ruleMask, 0x00, sizeof(CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC));
        
        cpssOsMemSet(&(rulePattern.macDa), 0x4, sizeof(GT_ETHERADDR));
        cpssOsMemSet(&(ruleMask.macDa), 0xFF, sizeof(GT_ETHERADDR));

        rulePattern.dscp = 0x3F;        
        ruleMask.dscp = 0x3F;

        rulePattern.ipProtocol = ipProtocol;
        ruleMask.ipProtocol = 0xFF;

        rulePattern.dip.arIP[0] = 0x04;
        rulePattern.dip.arIP[1] = 0x04;
        rulePattern.dip.arIP[2] = 0x04;
        rulePattern.dip.arIP[3] = 0x04;
        cpssOsMemSet(&(ruleMask.dip) ,0xFF, sizeof(GT_IPADDR)); /* prefix len = 32 */

        rulePattern.etherType = 0xf800;
        ruleMask.etherType = 0xFFFF;

        ruleQosMap.newDei = defaultDei;
        ruleQosMap.newPcp = defaultPcp;
        ruleQosMap.queue = 0;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, 5, &ruleQosMap, CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV4_E, &ruleMask, &rulePattern);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, 5);

        /* Configure qos rule to match ipv4 macDA, DSCP, IP protocol, DIP and prefixLen */
        cpssOsMemSet(&rulePattern, 0x00, sizeof(CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC));
        cpssOsMemSet(&ruleMask, 0x00, sizeof(CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC));
        
        cpssOsMemSet(&(rulePattern.macDa), 0x5, sizeof(GT_ETHERADDR));
        cpssOsMemSet(&(ruleMask.macDa), 0xFF, sizeof(GT_ETHERADDR));

        rulePattern.dscp = 0x3F;        
        ruleMask.dscp = 0x3F;

        rulePattern.ipProtocol = ipProtocol;
        ruleMask.ipProtocol = 0xFF;

        rulePattern.dip.arIP[0] = 0x05;
        rulePattern.dip.arIP[1] = 0x05;
        rulePattern.dip.arIP[2] = 0x05;
        rulePattern.dip.arIP[3] = 0x05;
        cpssOsMemSet(&(ruleMask.dip) ,0xFF, sizeof(GT_IPADDR)); /* prefix len = 32 */

        rulePattern.etherType = 0xf800;
        ruleMask.etherType = 0xFFFF;

        ruleQosMap.newDei = defaultDei;
        ruleQosMap.newPcp = defaultPcp;
        ruleQosMap.queue = 0;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, 6, &ruleQosMap, CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV4_E, &ruleMask, &rulePattern);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, 6);

        /* Configure qos rule to match ipv4 macDA, DSCP, IP protocol, DIP and prefixLen */
        cpssOsMemSet(&rulePattern, 0x00, sizeof(CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC));
        cpssOsMemSet(&ruleMask, 0x00, sizeof(CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC));
        
        cpssOsMemSet(&(rulePattern.macDa), 0x6, sizeof(GT_ETHERADDR));
        cpssOsMemSet(&(ruleMask.macDa), 0xFF, sizeof(GT_ETHERADDR));

        rulePattern.dscp = 0x3F;        
        ruleMask.dscp = 0x3F;

        rulePattern.ipProtocol = ipProtocol;
        ruleMask.ipProtocol = 0xFF;

        rulePattern.dip.arIP[0] = 0x06;
        rulePattern.dip.arIP[1] = 0x06;
        rulePattern.dip.arIP[2] = 0x06;
        rulePattern.dip.arIP[3] = 0x06;
        cpssOsMemSet(&(ruleMask.dip) ,0xFF, sizeof(GT_IPADDR)); /* prefix len = 32 */

        rulePattern.etherType = 0xf800;
        ruleMask.etherType = 0xFFFF;

        ruleQosMap.newDei = defaultDei;
        ruleQosMap.newPcp = defaultPcp;
        ruleQosMap.queue = 0;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, 7, &ruleQosMap, CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV4_E, &ruleMask, &rulePattern);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, 7);

        /* Configure qos rule to match ipv4 macDA, DSCP, IP protocol, DIP and prefixLen */
        cpssOsMemSet(&rulePattern, 0x00, sizeof(CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC));
        cpssOsMemSet(&ruleMask, 0x00, sizeof(CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC));
        
        cpssOsMemSet(&(rulePattern.macDa), 0x7, sizeof(GT_ETHERADDR));
        cpssOsMemSet(&(ruleMask.macDa), 0xFF, sizeof(GT_ETHERADDR));

        rulePattern.dscp = 0x3F;        
        ruleMask.dscp = 0x3F;

        rulePattern.ipProtocol = ipProtocol;
        ruleMask.ipProtocol = 0xFF;

        rulePattern.dip.arIP[0] = 0x07;
        rulePattern.dip.arIP[1] = 0x07;
        rulePattern.dip.arIP[2] = 0x07;
        rulePattern.dip.arIP[3] = 0x07;
        cpssOsMemSet(&(ruleMask.dip) ,0xFF, sizeof(GT_IPADDR)); /* prefix len = 32 */

        rulePattern.etherType = 0xf800;
        ruleMask.etherType = 0xFFFF;

        ruleQosMap.newDei = defaultDei;
        ruleQosMap.newPcp = defaultPcp;
        ruleQosMap.queue = 0;

        st = cpssPxHalBpeDataControlQosRuleAdd(dev, 8, &ruleQosMap, CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV4_E, &ruleMask, &rulePattern);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NO_RESOURCE, st, dev, 8);

        /* special enhanced UT restore configurations*/
        st = prvTgfPxHalBpeEnhancedUtExtended2UpstreamConfigRestore(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

UTF_TEST_CASE_MAC(prvTgfPxHalBpe_C2U_UC_qosMapping)
{
    GT_U8                              dev = 0;
    GT_STATUS                          st;
    GT_PORT_NUM                        cscdPort = 0;
    CPSS_PX_PORTS_BMP                  upPortBmp = 0;
    GT_PORT_NUM                        upPort = 12;
    GT_U32                             packetLen = 100;
    GT_U32                             egressPacketLen[PRV_CPSS_PX_PORTS_NUM_CNS - 1];
    GT_U32                             pcp, dei;
    GT_U32                             defaultDei = 1;
    GT_U32                             defaultPcp = 4;
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC egressInterface, ingressInterface;
    CPSS_PX_HAL_BPE_QOSMAP_STC         qosMapArr[CPSS_PCP_RANGE_CNS][CPSS_DEI_RANGE_CNS];
    GT_U32                             vid = 0x44;
    union                              {
        GT_U16                         octetPair;
        GT_U8                          octet[2];
    }cid;
    union                              {
        GT_U16                         octetPair;
        GT_U8                          octet[2];
    }pcid;
    cid.octetPair = 1;
    pcid.octetPair = 2;

    /* Create qos mapping */
    for(pcp = 0; pcp < CPSS_PCP_RANGE_CNS; pcp++)
    {
        for(dei = 0; dei < CPSS_DEI_RANGE_CNS; dei++)
        {
            qosMapArr[pcp][dei].newDei = dei;
            qosMapArr[pcp][dei].newPcp = pcp;
        }
    }

    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(upPortBmp, upPort);

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Perform common configuration */
        ingressInterface.type = egressInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        ingressInterface.devPort.devNum = egressInterface.devPort.devNum = dev;
        ingressInterface.devPort.portNum = cscdPort;
        egressInterface.devPort.portNum = upPort;

        st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface.type, egressInterface.devPort.devNum, egressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);

        st = cpssPxHalBpeInterfaceTypeSet(dev, &ingressInterface, CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, ingressInterface.type, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E);

        st = cpssPxHalBpeNumOfChannelsSet(dev, _4K - 20, _4K);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, _4K - 20, _4K);

        st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &ingressInterface, pcid.octetPair);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, pcid.octetPair);

        st = cpssPxHalBpeUnicastEChannelCreate(dev, cid.octetPair, &ingressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cid.octetPair);

        st = cpssPxHalBpePortExtendedUpstreamSet(dev, cscdPort, &egressInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cscdPort);

        st = cpssPxHalBpePortExtendedQosDefaultPcpDeiSet(dev, cscdPort, defaultPcp, defaultDei);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, cscdPort, defaultPcp, defaultDei);

        st = cpssPxHalBpePortExtendedQosMapSet(dev, cscdPort, qosMapArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cscdPort);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtExtended2UpstreamConfigSet(dev, cscdPort);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cscdPort);

        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = 0x00;
        transmittedPkt[1] = expectedPkt[1] = 0x00;
        transmittedPkt[2] = expectedPkt[2] = 0x00;
        transmittedPkt[3] = expectedPkt[3] = 0x00;
        transmittedPkt[4] = expectedPkt[4] = dev + 1;
        transmittedPkt[5] = expectedPkt[5] = cscdPort + 1;

        /* ingress + egress MAC SA */
        transmittedPkt[6]  = expectedPkt[6]  = 0x00;
        transmittedPkt[7]  = expectedPkt[7]  = 0x00;
        transmittedPkt[8]  = expectedPkt[8]  = 0x00;
        transmittedPkt[9]  = expectedPkt[9]  = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* 1.1 transmit un-E-tagged packet to cascade port, expect it to egress E-tagged out of upstream
           with pcp, dei fields according to default values and with E-CID = PCID
           send packet with C-tag with different pcp, dei to make sure it is not passed to the E-tag */

        /* ingress + egress C-Tag */
        transmittedPkt[12] = expectedPkt[20] = 0x81;
        transmittedPkt[13] = expectedPkt[21] = 0x00;
        transmittedPkt[14] = expectedPkt[22] = ((qosMapArr[defaultPcp][defaultDei].newDei + 1)%CPSS_DEI_RANGE_CNS << 4) | ((qosMapArr[defaultPcp][defaultDei].newPcp + 1)%CPSS_PCP_RANGE_CNS << 5);
        transmittedPkt[15] = expectedPkt[23] = vid;

        /* egress E-Tag */
        expectedPkt[12] = 0x89;
        expectedPkt[13] = 0x3F;
        expectedPkt[14] = (qosMapArr[defaultPcp][defaultDei].newDei << 4) | (qosMapArr[defaultPcp][defaultDei].newPcp << 5);
        expectedPkt[15] = 0x00;
        expectedPkt[16] = pcid.octet[1];
        expectedPkt[17] = pcid.octet[0];
        expectedPkt[18] = 0x00;
        expectedPkt[19] = 0x00;

        egressPacketLen[upPort] = packetLen + BYTES_IN_E_TAG;

        BPE_LOG_INFORMATION_MAC("Transmit untagged packet to cascade port %d.", cscdPort);
        /* Send packet and check egress as expected */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  upPortBmp,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 transmit E-tagged and C-tagged packet to cascade port, expect it to egress E-tagged and C-tagged out of upstream
           with pcp, dei fields according to default values and according to qos map in the E-tag and original pcp, dei
           values in the C-tag */
        for (pcp = 0; pcp < CPSS_PCP_RANGE_CNS; pcp++)
        {
            for (dei = 0; dei < CPSS_DEI_RANGE_CNS; dei++)
            {
                /* ingress + egress E-tag */
                transmittedPkt[12] = expectedPkt[12] = 0x89;
                transmittedPkt[13] = expectedPkt[13] = 0x3F;
                transmittedPkt[14] = expectedPkt[14] = (qosMapArr[pcp][dei].newDei << 4) | (qosMapArr[pcp][dei].newPcp << 5);
                transmittedPkt[15] = expectedPkt[15] = 0x00;
                transmittedPkt[16] = expectedPkt[16] = cid.octet[1];
                transmittedPkt[17] = expectedPkt[17] = cid.octet[0];
                transmittedPkt[18] = expectedPkt[18] = 0x00;
                transmittedPkt[19] = expectedPkt[19] = 0x00;

                /* egress + ingress C-tag */
                transmittedPkt[20] = expectedPkt[20] = 0x81;
                transmittedPkt[21] = expectedPkt[21] = 0x00;
                transmittedPkt[22] = expectedPkt[22] = ((qosMapArr[pcp][dei].newDei + 1)%CPSS_DEI_RANGE_CNS << 4) | ((qosMapArr[pcp][dei].newPcp + 1)%CPSS_PCP_RANGE_CNS << 5);
                transmittedPkt[23] = expectedPkt[23] = vid;

                egressPacketLen[upPort] = packetLen;

                /* Send packet and check egress as expected */
                BPE_LOG_INFORMATION_MAC("Transmit E-tagged packet with PCP=%d DEI=%d to cascade port %d.", pcp, dei, cscdPort);
                st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                          7,
                                                                          &ingressInterface,
                                                                          upPortBmp,
                                                                          GT_FALSE,
                                                                          transmittedPkt,
                                                                          expectedPkt,
                                                                          packetLen,
                                                                          egressPacketLen);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
        }

        /* special enhanced UT restore configurations*/
        st = prvTgfPxHalBpeEnhancedUtExtended2UpstreamConfigRestore(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

UTF_TEST_CASE_MAC(prvTgfPxHalBpe_U2C_UC_cascadeTrunk)
{
    GT_U8                              dev = 0;
    GT_STATUS                          st;
    GT_PORT_NUM                        cascadePort[3] = {12,1,5};
    CPSS_PX_PORTS_BMP                  cascadePortBmp = 0;
    GT_PORT_NUM                        upPort = 0;
    GT_PORT_NUM                        portIndex;
    GT_U32                             packetLen = 100;
    GT_U32                             trunkId = 6;
    GT_U32                             egressPacketLen[PRV_CPSS_PX_PORTS_NUM_CNS - 1];
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC egressInterface[3], ingressInterface, trunkInterface;
    union                              {
        GT_U16                         octetPair;
        GT_U8                          octet[2];
    }ecid;
    union                              {
        GT_U16                         octetPair;
        GT_U8                          octet[2];
    }pcid[3];
    ecid.octetPair = 6;    
    pcid[0].octetPair = 1;
    pcid[1].octetPair = 2;
    pcid[2].octetPair = 3;    

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Perform BPE init */
        st = cpssPxHalBpeInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Perform common configuration */
        trunkInterface.type = CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E;
        trunkInterface.trunkId = trunkId;

        ingressInterface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        ingressInterface.devPort.devNum = dev;
        ingressInterface.devPort.portNum = upPort;        

        /* Set numOfUnicastChannels == 4K - 20, numOfMulticastChannels == 4K,
           Configure unicast E-channel with cid == 1 */

        st = cpssPxHalBpeNumOfChannelsSet(dev, _4K - 20, _4K);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, _4K - 20, _4K);

        st = cpssPxHalBpeInterfaceTypeSet(dev, &ingressInterface, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, ingressInterface.type, ingressInterface.devPort.devNum, ingressInterface.devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);

        for (portIndex = 0; portIndex < 3; portIndex++) 
        {
            PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(cascadePortBmp, cascadePort[portIndex]);

            egressInterface[portIndex].type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
            egressInterface[portIndex].devPort.devNum = dev;
            egressInterface[portIndex].devPort.portNum = cascadePort[portIndex];                  

            st = cpssPxHalBpeInterfaceTypeSet(dev, &egressInterface[portIndex], CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, egressInterface[portIndex].type, egressInterface[portIndex].devPort.devNum, egressInterface[portIndex].devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E);

            st = cpssPxHalBpeInterfaceExtendedPcidSet(dev, &egressInterface[portIndex], pcid[portIndex].octetPair);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, egressInterface[portIndex].devPort.devNum, egressInterface[portIndex].devPort.portNum, pcid[portIndex].octetPair);

            st = cpssPxHalBpeUnicastEChannelCreate(dev, pcid[portIndex].octetPair, &egressInterface[portIndex]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, pcid[portIndex].octetPair);

            st = cpssPxHalBpePortExtendedUpstreamSet(dev, egressInterface[portIndex].devPort.portNum, &ingressInterface);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, egressInterface[portIndex].devPort.portNum);
        }                      

        /* Create cascade trunk */
        st = cpssPxHalBpeTrunkCreate(dev, trunkId, CPSS_PX_HAL_BPE_TRUNK_CASCADE_E);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, trunkId, CPSS_PX_HAL_BPE_TRUNK_CASCADE_E);

        st = cpssPxHalBpeUnicastEChannelCreate(dev, ecid.octetPair, &trunkInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ecid.octetPair);             

        /* Add 3 cascade ports to cascade trunk */
        st = cpssPxHalBpeTrunkPortsAdd(dev, trunkId, egressInterface, 3);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);                

        /* Set trunk load balance mode to port based */
        st = cpssPxHalBpeTrunkLoadBalanceModeSet(dev, CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_MAC_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_MAC_E);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2CascadeUcConfigSet(dev, pcid[0].octetPair, pcid[0].octetPair);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, pcid[0].octetPair, pcid[0].octetPair);

        cpssOsMemSet(transmittedPkt, 0xFF, PKT_BUFF_LEN);
        cpssOsMemSet(expectedPkt, 0xFF, PKT_BUFF_LEN);

        /* ingress + egress MAC DA */
        transmittedPkt[0] = expectedPkt[0] = 0x00;
        transmittedPkt[1] = expectedPkt[1] = 0x00;
        transmittedPkt[2] = expectedPkt[2] = 0x00;
        transmittedPkt[3] = expectedPkt[3] = 0x00;
        transmittedPkt[4] = expectedPkt[4] = dev + 1;
        transmittedPkt[5] = expectedPkt[5] = upPort + 1;

        /* ingress + egress MAC SA */
        transmittedPkt[6] = expectedPkt[6]   = 0x00;
        transmittedPkt[7] = expectedPkt[7]   = 0x00;
        transmittedPkt[8] = expectedPkt[8]   = 0x00;
        transmittedPkt[9] = expectedPkt[9]   = 0x00;
        transmittedPkt[10] = expectedPkt[10] = 0x00;
        transmittedPkt[11] = expectedPkt[11] = 0x99;

        /* ingress E-Tag */
        transmittedPkt[12] = 0x89;
        transmittedPkt[13] = 0x3F;
        transmittedPkt[14] = 0x00;
        transmittedPkt[15] = 0x00;
        transmittedPkt[16] = pcid[0].octet[1];
        transmittedPkt[17] = pcid[0].octet[0];
        transmittedPkt[18] = 0x00;
        transmittedPkt[19] = 0x00;                 

        egressPacketLen[cascadePort[0]] = packetLen - BYTES_IN_E_TAG;

        /* 1.1   Send packet with ECID == PCID of cascade port, we expect it to egress untagged */
        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  0x1000,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev))
        {            
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        }
        else
        {
            /* Bug in PIPE A0: if a port is a member in a LAG all the traffic to that port is going through LAG load balancing.
               This behavior doesn't allow to direct packets to specific port. Fixed in PIPE A1. */
            BPE_LOG_INFORMATION_MAC("BUG in PIPE A0: If a port is a member in a LAG all the traffic to that port is going through LAG load balancing");
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FAIL, st);
        }

        /* special enhanced UT restore configurations */
        st = prvTgfPxHalBpeEnhancedUtUpstream2CascadeUcConfigRestore(dev, pcid[0].octetPair);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, pcid[0].octetPair);

        /* special enhanced UT configurations*/
        st = prvTgfPxHalBpeEnhancedUtUpstream2CascadeUcConfigSet(dev, ecid.octetPair, pcid[0].octetPair);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, ecid.octetPair, pcid[0].octetPair);

        /* ingress + egress E-Tag */
        transmittedPkt[12] = expectedPkt[12] = 0x89;
        transmittedPkt[13] = expectedPkt[13] = 0x3F;
        transmittedPkt[14] = expectedPkt[14] = 0x00;
        transmittedPkt[15] = expectedPkt[15] = 0x00;
        transmittedPkt[16] = expectedPkt[16] = ecid.octet[1];
        transmittedPkt[17] = expectedPkt[17] = ecid.octet[0];
        transmittedPkt[18] = expectedPkt[18] = 0x00;
        transmittedPkt[19] = expectedPkt[19] = 0x00;

        egressPacketLen[cascadePort[1]] = packetLen;

        /* 1.2   Send packet with ECID == PCID of cascade trunk, we expect it to egress tagged */

        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  0x2,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        transmittedPkt[11] = expectedPkt[11] = 0x98;

        egressPacketLen[cascadePort[2]] = packetLen;

        /* 1.3   Send packet with ECID == PCID of cascade trunk, we expect it to egress tagged */

        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  0x20,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        transmittedPkt[11] = expectedPkt[11] = 0x94;

        egressPacketLen[cascadePort[0]] = packetLen;

        /* 1.4   Send packet with ECID == PCID of cascade trunk, we expect it to egress tagged */

        st = prvTgfPxHalBpeSendSinglePacketAndCheckExpectedEgress(dev,
                                                                  7,
                                                                  &ingressInterface,
                                                                  0x1000,
                                                                  GT_FALSE,
                                                                  transmittedPkt,
                                                                  expectedPkt,
                                                                  packetLen,
                                                                  egressPacketLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* special enhanced UT restore configurations */
        st = prvTgfPxHalBpeEnhancedUtUpstream2CascadeUcConfigRestore(dev, ecid.octetPair);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ecid.octetPair);
    }
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxTgfHalBpe suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxTgfHalBpe)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxHalBpe_U2E_UC)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxHalBpe_U2E_UC_withTagMode)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxHalBpe_U2E_MC)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxHalBpe_U2E_MC_withSrcFiltering)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxHalBpe_U2C_UC)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxHalBpe_U2EC_UC_typeSwitch)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxHalBpe_U2C_MC)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxHalBpe_U2C_MC_withSrcFiltering)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxHalBpe_U2EC_MC)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxHalBpe_E2U_UC)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxHalBpe_E2U_UC_upstreamTrunk)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxHalBpe_E2U_UC_upstreamTrunk_withLoadBalanceMode)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxHalBpe_C2U_UC)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxHalBpe_U2E_UC_cncCounters)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxHalBpe_E2U_UC_qosMapping)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxHalBpe_E2U_UC_dataControlQosRule)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxHalBpe_E2U_UC_dataControlQosRule_IP2ME)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxHalBpe_C2U_UC_qosMapping)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxHalBpe_U2C_UC_cascadeTrunk)

UTF_SUIT_END_TESTS_MAC(cpssPxTgfHalBpe)
