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
* @file cpssPxTgfCommon.h
*
* @brief Common traffic helper API for enhanced UTs
*
* @version   1
********************************************************************************
*/
#ifndef __cpssPxTgfCommon_h__
#define __cpssPxTgfCommon_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/common/cpssTypes.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>


extern GT_PHYSICAL_PORT_NUM prvTgfPxPortsArray[PRV_CPSS_PX_PORTS_NUM_CNS];

/**
* @struct CPSS_PX_TGF_PORT_MAC_COUNTERS_STC
 *
 * @brief Structure of TxQ drop counters.
*/
typedef struct{

    /** number of good received Ethernet frames. */
    GT_U32 goodPktsRcv;

    /** number of good sent Ethernet frames. */
    GT_U32 goodPktsSent;

} CPSS_PX_TGF_PORT_MAC_COUNTERS_STC;


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
);

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
* @param[in] payloadLength            - frame payload length
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
);

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
);

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
);

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
);

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
);

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
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxTgfCommon_h__ */

