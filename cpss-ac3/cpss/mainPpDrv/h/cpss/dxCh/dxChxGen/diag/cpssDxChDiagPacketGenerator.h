/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file cpssDxChDiagPacketGenerator.h
*
* @brief Diag Packet Generator APIs for CPSS DxCh.
*
* Sequence for generating packets on port:
* 1. Connect the port to packet generator and set configurations
* (cpssDxChDiagPacketGeneratorConnectSet)
* 2. Start transmission (cpssDxChDiagPacketGeneratorTransmitEnable)
* 3. If single burst transmit mode used:
* Check transmission done status (cpssDxChDiagPacketGeneratorBurstTransmitStatusGet)
* If continues transmit mode used:
* Stop transmission (cpssDxChDiagPacketGeneratorTransmitEnable)
* 4. Return the port to normal operation, disconnect the packet generator
* (cpssDxChDiagPacketGeneratorConnectSet)
*
* @version   7
********************************************************************************
*/
#ifndef __cpssDxChDiagPacketGeneratorh
#define __cpssDxChDiagPacketGeneratorh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/diag/cpssDiagPacketGenerator.h>

/**
* @struct CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC
*
* @brief Packet generator's configurations.
*/
typedef struct{

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** @brief Enables MAC DA incremental for every transmitted packet.
     *  GT_FLASE - MAC DA is constant for all packets
     *  GT_TRUE - MAC DA is incremented for every packet.
     */
    GT_BOOL macDaIncrementEnable;

    GT_U32 macDaIncrementLimit;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    /** @brief Enables insertion of vlan tag.
     *  GT_FLASE - vlan tag is not inserted.
     *  GT_TRUE - vlan tag is inserted after MAC SA.
     */
    GT_BOOL vlanTagEnable;

    /** @brief The VPT of the VLAN tag added to packet. (APPLICABLE RANGES: 0..7)
     *  (relevant only if vlanTagEnable == GT_TRUE)
     */
    GT_U8 vpt;

    /** @brief The CFI of the VLAN tag added to packet. (APPLICABLE RANGES: 0..1)
     *  (relevant only if vlanTagEnable == GT_TRUE)
     */
    GT_U8 cfi;

    /** @brief The VLAN ID of the VLAN tag added to packet. (APPLICABLE RANGES: 1..4095)
     *  (relevant only if vlanTagEnable == GT_TRUE)
     */
    GT_U16 vid;

    /** @brief The etherType to be sent in the packet after the VLAN tag(if any).
     *  (APPLICABLE RANGES: 0..0xFFFF)
     */
    GT_U16 etherType;

    /** The payload type to be placed after etherType. */
    CPSS_DIAG_PG_PACKET_PAYLOAD_TYPE_ENT payloadType;

    GT_U8 cyclicPatternArr[8];

    /** The packet length type. */
    CPSS_DIAG_PG_PACKET_LENGTH_TYPE_ENT packetLengthType;

    /** @brief The packet length without CRC. (APPLICABLE RANGES: 20..16383)
     *  (relevant only if packetLengthType == CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E)
     */
    GT_U32 packetLength;

    /** @brief Enables undersized packets transmission.
     *  (relevant only if packetLengthType == CPSS_DIAG_PG_PACKET_LENGTH_RANDOM_E)
     */
    GT_BOOL undersizeEnable;

    /** Packet generator transmit mode. */
    CPSS_DIAG_PG_TRANSMIT_MODE_ENT transmitMode;

    /** @brief Packet count to transmit in single burst. (APPLICABLE RANGES: 1..8191)
     *  The total packets number to transmit is:
     *  (packetCount) (packetCountMultiplier value).
     *  (relevant only if transmitMode == CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E)
     *  packetCountMultiplier- Packet count packet count multiplier.
     *  The total packets number to transmit is:
     *  (packetCount) (packetCountMultiplier value).
     *  (relevant only if transmitMode == CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E)
     */
    GT_U32 packetCount;

    CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_ENT packetCountMultiplier;

    /** Interpacket gap in bytes. (APPLICABLE RANGES: 0..0xFFFF) */
    GT_U32 ipg;

    /** Interface size (what is the word width to the port, instead of the TxDMA, BC2 only). */
    CPSS_DIAG_PG_IF_SIZE_ENT interfaceSize;

} CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC;


/**
* @internal cpssDxChDiagPacketGeneratorConnectSet function
* @endinternal
*
* @brief   Connect/Disconnect port to packet generator.
*         Set packet generator's configurations.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number to connect
*                                      its MAC to packet generator.
* @param[in] connect                  - GT_TRUE:  the given port's MAC to packet generator.
*                                      GT_FALSE: disconnect the given port's MAC from
*                                      packet generator; return the port's MAC
*                                      to normal egress pipe.
* @param[in] configPtr                - (pointer to) packet generator configurations.
*                                      Relevant only if enable==GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*
* @note 1. In case packet generator connected to other port the function perfroms
*       the following:
*       - stops traffic.
*       - connects packet generator to new port
*       - overrides packet generator's configurations
*       2. Before enabling the packet generator, port must be initialized,
*       i.e. port interface and speed must be set.
*
*/
GT_STATUS cpssDxChDiagPacketGeneratorConnectSet
(
    IN GT_U8                                 devNum,
    IN GT_PHYSICAL_PORT_NUM                  portNum,
    IN GT_BOOL                               connect,
    IN CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC *configPtr
);

/**
* @internal cpssDxChDiagPacketGeneratorConnectGet function
* @endinternal
*
* @brief   Get the connect status of specified port.
*         Get packet generator's configurations.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number to connect
*                                      its MAC to packet generator.
*
* @param[out] connectPtr               - (pointer to)
*                                      GT_TRUE: port connected to packet generator.
*                                      GT_FALSE: port not connected to packet generator.
* @param[out] configPtr                - (pointer to) packet generator configurations.
*                                      Relevant only if enable==GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChDiagPacketGeneratorConnectGet
(
    IN  GT_U8                                 devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    OUT GT_BOOL                              *connectPtr,
    OUT CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC *configPtr
);

/**
* @internal cpssDxChDiagPacketGeneratorTransmitEnable function
* @endinternal
*
* @brief   Enable/Disable(Start/Stop) transmission on specified port.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE: enable(start) transmission
*                                      GT_FALSE: disable(stop) transmission
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_STATE             - on port not connected to packet generator
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*
* @note 1. Before calling this function the port must be connected to packet
*       generator (cpssDxChDiagPacketGeneratorConnectSet),
*       otherwise GT_BAD_STATE is returned.
*
*/
GT_STATUS cpssDxChDiagPacketGeneratorTransmitEnable
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
);

/**
* @internal cpssDxChDiagPacketGeneratorBurstTransmitStatusGet function
* @endinternal
*
* @brief   Get burst transmission status on specified port.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] burstTransmitDonePtr     - (pointer to) burst transmit done status
*                                      GT_TRUE: burst transmission done
*                                      GT_FALSE: burst transmission not done
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on port not connected to packet generator
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Burst transmission status is clear on read.
*       2. Before calling this function the port must be connected to packet
*       generator (cpssDxChDiagPacketGeneratorConnectSet),
*       otherwise GT_BAD_STATE is returned.
*
*/
GT_STATUS cpssDxChDiagPacketGeneratorBurstTransmitStatusGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL              *burstTransmitDonePtr
);


/**
* @internal prvCpssDxChMacTGDrvInit function
* @endinternal
*
* @brief   Init Mac TG driver
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Falcon.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChMacTGDrvInit
(
    IN    GT_U8                   devNum
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChDiagPacketGeneratorh */
