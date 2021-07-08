/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* tgfNetifGen.h
*
* DESCRIPTION:
*       Header file for generic API of Network Interface.
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*
*******************************************************************************/
#ifndef __tgfNetIfGenh
#define __tgfNetIfGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfCommon.h>

/* macro to force using the eport instead of the physical port in the eDSA */
/* needed for prvUtfIsDoublePhysicalPortsModeUsed() */
#define PRV_TGF_NET_TX_GENERATOR_FORCE_USE_EPORT_MAC(packetParams) \
    {                                                                          \
        /* we can not use the 'physical port' info --> use the ePort */        \
        packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_FALSE;    \
        packetParams.dsaParam.dsaInfo.fromCpu.dstEport =                       \
            packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum;\
                                                                               \
        /* put valid value that the CPSS will not fail */                      \
        packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = 0x88;/* dont care*/ \
    }

/******************************************************************************\
 *                          Private type definitions                          *
\******************************************************************************/

/**
* @enum PRV_TGF_NET_TX_GENERATOR_RATE_MODE_ENT
 *
 * @brief Defines the rate mode for Tx SDMA working as packet Generator
*/
typedef enum{

    /** rate is measured as packets per second. */
    PRV_TGF_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E,

    /** interval between successive transmissions of the same packet. */
    PRV_TGF_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E,

    /** no interval between successive transmissions. */
    PRV_TGF_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E

} PRV_TGF_NET_TX_GENERATOR_RATE_MODE_ENT;

/**
* @enum PRV_TGF_NET_TX_GENERATOR_BURST_STATUS_ENT
 *
 * @brief Defines Tx SDMA Generator packet burst counter status.
*/
typedef enum{

    /** packet counter is idle. */
    PRV_TGF_NET_TX_GENERATOR_BURST_STATUS_IDLE_E,

    /** packet counter is running and has not yet reached its configurable limit. */
    PRV_TGF_NET_TX_GENERATOR_BURST_STATUS_RUN_E,

    /** packet counter has reached its configurable limit. */
    PRV_TGF_NET_TX_GENERATOR_BURST_STATUS_LIMIT_E,

    /** packet counter has been terminated by the CPU. */
    PRV_TGF_NET_TX_GENERATOR_BURST_STATUS_TERMINATE_E

} PRV_TGF_NET_TX_GENERATOR_BURST_STATUS_ENT;

/**
* @enum PRV_TGF_TX_SDMA_GENERATOR_MEM_CHECK_ENT
 *
 * @brief Define Tx SDMA Generator memory check result status.
*/
typedef enum{

    /** @brief memory check passed
     *  successfully.
     */
    PRV_TGF_TX_SDMA_GENERATOR_MEM_CHECK_PASS_E,

    /** @brief wrong value of buffer
     *  pointer in descriptor.
     */
    PRV_TGF_TX_SDMA_GENERATOR_MEM_CHECK_BAD_BUFF_PTR_E,

    /** @brief wrong value of next
     *  descriptor pointed.
     */
    PRV_TGF_TX_SDMA_GENERATOR_MEM_CHECK_BAD_NEXT_DESC_E,

    /** the queue memory is empty. */
    PRV_TGF_TX_SDMA_GENERATOR_MEM_CHECK_EMPTY_QUEUE_E,

    /** @brief memory scanning exceed
     *  the expected number of queue descriptors.
     */
    PRV_TGF_TX_SDMA_GENERATOR_MEM_CHECK_DESC_EXCEED_E

} PRV_TGF_TX_SDMA_GENERATOR_MEM_CHECK_ENT;

/**
* @struct PRV_TGF_NET_TX_PARAMS_STC
 *
 * @brief structure of Tx parameters , that contain full information on
 * how to send TX packet from CPU to PP.
*/
typedef struct{

    /** @brief the packet (buffers) contain the vlan TAG inside.
     *  GT_TRUE - the packet contain the vlan TAG inside.
     *  GT_FALSE - the packet NOT contain the vlan TAG inside.
     */
    GT_BOOL packetIsTagged;

    /** @brief The user's data to be returned to the Application when in
     *  the Application's buffers can be re-used .
     */
    GT_PTR cookie;

    /** the info needed when send packets using the SDMA. */
    TGF_NET_SDMA_TX_PARAMS_STC sdmaInfo;

    /** the DSA parameters */
    TGF_PACKET_DSA_TAG_STC dsaParam;

} PRV_TGF_NET_TX_PARAMS_STC;

/**
* @struct PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC
 *
 * @brief parameter of the "cpu code table" entry
*/
typedef struct{

    /** @brief The Traffic Class assigned to packets with this
     *  CPU code (APPLICABLE RANGES: 0..7).
     */
    GT_U8 tc;

    /** @brief The Drop Precedence assigned to packets with this
     *  CPU code.
     */
    CPSS_DP_LEVEL_ENT dp;

    /** @brief Enables/disable the truncation of packets assigned
     *  with this CPU code to up to 128 bytes.
     *  GT_FALSE - don't truncate packets assigned with
     *  this CPU code.
     *  GT_TRUE - truncate packets assigned with this
     *  CPU code to up to 128 bytes.
     */
    GT_BOOL truncate;

    /** @brief Rate Limiting mode
     *  CPSS_NET_CPU_CODE_RATE_LIMIT_LOCAL_E =
     *  Rate Limiting is done on packets forwarded to the
     *  CPU by the local device ONLY.
     *  CPSS_NET_CPU_CODE_RATE_LIMIT_AGGREGATE_E =
     *  Rate Limiting is done on all packets forwarded to
     *  the CPU.
     */
    CPSS_NET_CPU_CODE_RATE_LIMIT_MODE_ENT cpuRateLimitMode;

    /** @brief CPU Code Rate Limiter index
     *  The index to one of the 31 or 255 rate limiters
     *  0 = This CPU Code is not bound to any Rate
     *  Limiter, thus the rate of packets with this CPU
     *  code is not limited.
     *  1-31 or 1-255 = CPU Code is bound to a Rate limiter
     *  according to this index
     *  see API :
     *  cpssDxChNetIfCpuCodeRateLimiterTableSet(...)
     *  (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 1..31;
     *  Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 1..255)
     */
    GT_U32 cpuCodeRateLimiterIndex;

    /** @brief Index of one of the 32 or 256 statistical rate
     *  limiters for this CPU Code .
     *  0-31 or 0-255 = CPU Code is bound to a statistical Rate
     *  limiter according to this index
     *  see API :
     *  cpssDxChNetIfCpuCodeStatisticalRateLimitsTableSet(...)
     *  (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..31;
     *  Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 0..255)
     */
    GT_U32 cpuCodeStatRateLimitIndex;

    /** @brief The target device index for packets with this
     *  CPU code.
     *  0 = Packets with this CPU code are forwarded to
     *  the CPU attached to the local device.
     *  1-7 = Packets with this CPU code are forwarded
     *  to CPU attached to the device indexed by
     *  this field
     *  see API :
     *  cpssDxChNetIfNetDesignatedDeviceTableSet(...)
     */
    GT_U32 designatedDevNumIndex;

} PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC;

/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfNetIfSdmaTxGeneratorEnable function
* @endinternal
*
* @brief   This function enables selected Tx SDMA Generator.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - The Tx SDMA queue to enable.
* @param[in] burstEnable              - GT_TRUE for packets burst generation,
*                                      - GT_FALSE for continuous packets generation.
* @param[in] burstPacketsNumber       - Number of packets in burst.
*                                      Relevant only if burstEnable == GT_TRUE.
*                                      (APPLICABLE RANGES: 0..512M-1)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode or burst
*                                       size.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note If queue is empty, configuration is done beside the enabling which will
*       be implicitly performed after the first packet will be added to the
*       queue.
*       On each HW queue enabling the Tx SDMA Current Descriptor Pointer
*       register will set to point to the first descriptor in chain list.
*
*/
GT_STATUS prvTgfNetIfSdmaTxGeneratorEnable
(
    IN  GT_U8   devNum,
    IN  GT_U8   txQueue,
    IN  GT_BOOL burstEnable,
    IN  GT_U32  burstPacketsNumber
);

/**
* @internal prvTgfNetIfSdmaTxGeneratorDisable function
* @endinternal
*
* @brief   This function disables selected Tx SDMA Generator.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - The Tx SDMA queue to enable.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfNetIfSdmaTxGeneratorDisable
(
    IN  GT_U8   devNum,
    IN  GT_U8   txQueue
);

/**
* @internal prvTgfNetIfSdmaTxGeneratorRateSet function
* @endinternal
*
* @brief   This function sets Tx SDMA Generator transmission packet rate.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - The Tx SDMA queue.
* @param[in] rateMode                 - transmission rate mode.
* @param[in] rateValue                - packets per second for rateMode ==
*                                      PRV_TGF_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E,
*                                      nanoseconds for rateMode ==
*                                      PRV_TGF_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E.
*                                      Not relevant to PRV_TGF_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E.
*
* @param[out] actualRateValuePtr       - (pointer to) the actual configured rate value.
*                                      Same units as rateValue.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode, bad rate
*                                       mode or value.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_BAD_VALUE             - rate cannot be configured for Tx SDMA.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Rate can be set while Tx SDMA is enabled.
*
*/
GT_STATUS prvTgfNetIfSdmaTxGeneratorRateSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   txQueue,
    IN  PRV_TGF_NET_TX_GENERATOR_RATE_MODE_ENT  rateMode,
    IN  GT_U64                                  rateValue,
    OUT GT_U64                                  *actualRateValuePtr
);

/**
* @internal prvTgfNetIfSdmaTxGeneratorBurstStatusGet function
* @endinternal
*
* @brief   This function gets Tx SDMA Generator burst counter status.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - The Tx SDMA queue to enable.
*
* @param[out] burstStatusPtr           - (pointer to) Burst counter status.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfNetIfSdmaTxGeneratorBurstStatusGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U8                                       txQueue,
    OUT PRV_TGF_NET_TX_GENERATOR_BURST_STATUS_ENT   *burstStatusPtr
);

/**
* @internal prvTgfNetIfSdmaTxGeneratorPacketRemove function
* @endinternal
*
* @brief   This function removes packet from Tx SDMA working as a Packet Generator.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - The Tx SDMA queue to remove the packet from.
* @param[in] packetId                 - packet identification number assign during packet
*                                      addition to the Generator.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode or packet
*                                       is not transmitted on this Tx SDMA.
* @retval GT_BAD_VALUE             - removal of packet will violate the required
*                                       rate configuration.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Packet can be removed while Tx SDMA is enabled.
*
*/
GT_STATUS prvTgfNetIfSdmaTxGeneratorPacketRemove
(
    IN  GT_U8   devNum,
    IN  GT_U8   txQueue,
    IN  GT_U32  packetId
);

/**
* @internal prvTgfNetIfSdmaTxGeneratorPacketAdd function
* @endinternal
*
* @brief   This function adds a new packet to Tx SDMA working as Packet Generator.
*         This packet will be transmitted by the selected Tx SDMA with previous
*         packets already transmitted by this Packet generator.
* @param[in] devNum                   - Device number.
* @param[in] packetParamsPtr          - (pointer to) The internal packet params to be set
*                                      into the packet descriptors.
* @param[in] packetDataPtr            - (pointer to) packet data.
* @param[in] packetDataLength         - packet data length. Buffer size configured during
*                                      initialization phase must be sufficient for packet
*                                      length and the DSA tag that will be added to it.
*
* @param[out] packetIdPtr              - (pointer to) packet identification number, used by
*                                      other Packet Generator functions which require access
*                                      to this packet.
*
* @retval GT_OK                    - on success.
* @retval GT_NO_RESOURCE           - no available buffer or descriptor.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode, bad DSA
*                                       params or the data buffer is longer than
*                                       size configured.
* @retval GT_BAD_VALUE             - addition of packet will violate the required
*                                       rate configuration.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. Packet can be added while Tx SDMA is enabled.
*       2. The packet buffer supplied by the application is copied to internal
*       prealloocated generator buffer, therefore application can free or reuse
*       that buffer as soon as function returns.
*
*/
GT_STATUS prvTgfNetIfSdmaTxGeneratorPacketAdd
(
    IN  GT_U8                       devNum,
    IN  PRV_TGF_NET_TX_PARAMS_STC   *packetParamsPtr,
    IN  GT_U8                       *packetDataPtr,
    IN  GT_U32                      packetDataLength,
    OUT GT_U32                      *packetIdPtr
);

/**
* @internal prvTgfNetIfSdmaTxGeneratorPacketUpdate function
* @endinternal
*
* @brief   This function updates already transmitted packet content and parameters.
*
* @param[in] devNum                   - Device number.
* @param[in] packetId                 - packet identification number assign during packet
*                                      addition to the Generator.
* @param[in] packetParamsPtr          - (pointer to) The internal packet params to be set
*                                      into the packet descriptors.
* @param[in] packetDataPtr            - (pointer to) packet data.
* @param[in] packetDataLength         - packet data length. Buffer size configured during
*                                      initialization phase must be sufficient for packet
*                                      length and the DSA tag that will be added to it.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode, packet is
*                                       not transmitted on this Tx SDMA, bad DSA
*                                       params or the data buffer is longer than
*                                       size configured.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. Packet can be updated while Tx SDMA is enabled.
*       2. The packet buffer supplied by the application is copied to internal
*       prealloocated generator buffer, therefore application can free or reuse
*       that buffer as soon as function returns.
*
*/
GT_STATUS prvTgfNetIfSdmaTxGeneratorPacketUpdate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      packetId,
    IN  PRV_TGF_NET_TX_PARAMS_STC   *packetParamsPtr,
    IN  GT_U8                       *packetDataPtr,
    IN  GT_U32                      packetDataLength
);

/**
* @internal prvTgfNetIfSdmaTxQueueEnableGet function
* @endinternal
*
* @brief   Get status of the specified traffic class queue for TX
*         packets from CPU. -- SDMA relate
* @param[in] devNum                   - device number
*                                      queue     - traffic class queue
*
* @param[out] enablePtr                - GT_TRUE, enable queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on CPU port is not SDMA
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvTgfNetIfSdmaTxQueueEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   txQueue,
    OUT GT_BOOL *enablePtr
);

/**
* @internal prvTgfNetIfSdmaTxQueueEnable function
* @endinternal
*
* @brief   Set status of the specified traffic class queue for TX
*         packets from CPU. -- SDMA related
* @param[in] devNum                   - device number
*                                      queue     - traffic class queue
* @param[in] enable                   - GT_TRUE,  queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on CPU port is not SDMA
*/
GT_STATUS prvTgfNetIfSdmaTxQueueEnable
(
    IN  GT_U8   devNum,
    IN  GT_U8   txQueue,
    OUT GT_BOOL enable
);

/**
* @internal prvTgfNetIfSdmaTxGeneratorMemCheck function
* @endinternal
*
* @brief   This function verify that descriptors and buffers memory is arranged
*         to support optimal performance.
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - The Tx SDMA queue to enable.
*
* @param[out] checkResultPtr           - (pointer to) memory check result status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note memory check can be performed while queue is enabled.
*
*/
GT_STATUS prvTgfNetIfSdmaTxGeneratorMemCheck
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   txQueue,
    OUT PRV_TGF_TX_SDMA_GENERATOR_MEM_CHECK_ENT *checkResultPtr
);

/**
* @internal prvTgfNetIfCpuCodeTableSet function
* @endinternal
*
* @brief   Function to set the "CPU code table" entry, for a specific CPU code.
*
* @param[in] devNum                   - device number
* @param[in] cpuCode                  - the CPU code (key parameter to access the "CPU code table").
*                                      use CPSS_NET_ALL_CPU_OPCODES_E as a wildcard to be used when
*                                      need to set all the table entries with the same info.
* @param[in] entryInfoPtr             - (pointer to) The entry information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad cpuCode or bad one of
*                                       bad one entryInfoPtr parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfNetIfCpuCodeTableSet
(
    IN GT_U8                    devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT cpuCode,
    IN PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC *entryInfoPtr
);

/**
* @internal prvTgfNetIfCpuCodeTableGet function
* @endinternal
*
* @brief   Function to get the "CPU code table" entry, for a specific CPU code.
*
* @param[in] devNum                   - device number
* @param[in] cpuCode                  - the CPU code (key parameter to access the "CPU code table").
*                                      use CPSS_NET_ALL_CPU_OPCODES_E as a wildcard to be used when
*                                      need to set all the table entries with the same info.
*
* @param[out] entryInfoPtr             - (pointer to) The entry information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad cpuCode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfNetIfCpuCodeTableGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT cpuCode,
    OUT PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC *entryInfoPtr
);

/**
* @internal prvTgfNetIfCpuCodeDesignatedDeviceTableSet function
* @endinternal
*
* @brief   Function to set the "designated device Table" entry.
*         The cpssDxChNetIfCpuCodeTableSet(...) points to this table (for a
*         specific CPU Code)
* @param[in] devNum                   - device number
* @param[in] index                    -  into the designated device table (APPLICABLE RANGES: 1..7)
* @param[in] designatedHwDevNum       - The designated HW device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad index or bad
*                                       designatedHwDevNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfNetIfCpuCodeDesignatedDeviceTableSet
(
    IN GT_U8            devNum,
    IN GT_U32           index,
    IN GT_HW_DEV_NUM    designatedHwDevNum
);

/**
* @internal prvTgfNetIfCpuCodeDesignatedDeviceTableGet function
* @endinternal
*
* @brief   Function to get the "designated device Table" entry.
*         The cpssDxChNetIfCpuCodeTableSet(...) points to this table (for a
*         specific CPU Code)
* @param[in] devNum                   - device number
* @param[in] index                    -  into the designated device table (APPLICABLE RANGES: 1..7)
*
* @param[out] designatedHwDevNumPtr    - (pointer to)The designated HW device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfNetIfCpuCodeDesignatedDeviceTableGet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    OUT GT_HW_DEV_NUM   *designatedHwDevNumPtr
);

/**
* @internal prvTgfNetIfSdmaTxPacketSend function
* @endinternal
*
* @brief   This function verify that descriptors and buffers memory is arranged
*         to support optimal performance.
* @param[in] devNum                   - Device number
* @param[in] pcktParamsPtr            - The internal packet params to be set into packet
*                                      descriptors
* @param[in] buffList[]               - A packet data buffers list
* @param[in] buffLenList[]            - A list of buffers len in buffList
* @param[in] numOfBufs                - Length of bufflist
*
* @retval GT_OK                    - on success, or
* @retval GT_NO_RESOURCE           - if there is not enough free elements in the fifo
*                                       associated with the Event Request Handle.
* @retval GT_EMPTY                 - if there are not enough descriptors to do the sending.
* @retval GT_BAD_PARAM             - on bad DSA params or the data buffer is longer
*                                       than allowed. Buffer data can occupied up to the
*                                       maximum number of descriptors defined.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       GT_FAIL otherwise.
* @retval GT_BAD_STATE             - function is not implemented for selected family
*/
GT_STATUS prvTgfNetIfSdmaTxPacketSend
(
    IN GT_U8                        devNum,
    IN PRV_TGF_NET_TX_PARAMS_STC    *pcktParamsPtr,
    IN GT_U8                        *buffList[],
    IN GT_U32                       buffLenList[],
    IN GT_U32                       numOfBufs
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfNetIfGenh */


