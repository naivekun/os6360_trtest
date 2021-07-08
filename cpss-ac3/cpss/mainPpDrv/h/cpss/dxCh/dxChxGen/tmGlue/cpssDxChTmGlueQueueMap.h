/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file cpssDxChTmGlueQueueMap.h
*
* @brief Traffic Manager Glue - Packet Queue Id mapping.
*
* @version   6
********************************************************************************
*/

#ifndef __cpssDxChTmGlueQueueMaph
#define __cpssDxChTmGlueQueueMaph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>

/* amount of bits in TM Queue Id */
#define CPSS_DXCH_TM_GLUE_QUEUE_ID_BIT_WIDTH_CNS 14

/**
* @enum CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_ENT
 *
 * @brief Traffic Manager Queue Id Bit Select type.
*/
typedef enum{

    /** set bit to zero anycase. */
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_ZERO_E,

    /** take bit from Egress PCL Acion Field "Queue Id". */
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_EPCL_QUEUE_ID_E,

    /** take bit from packet flow Id (set by TTI, IPCL, EPCL). */
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_FLOW_ID_E,

    /** take bit from Local Device Target Pysical Port Number. */
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_LOCAL_TARGET_PHY_PORT_E,

    /** take bit from Packet TM (Mapped) Traffic Class. */
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TM_TC_E,

    /** take bit from Packet Hash. */
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_PACKET_HASH_E,

    /** take bit from Target Device Number. */
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TARGET_DEV_E,

    /** take bit from Target Device Target Pysical Port Number. */
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TARGET_PHY_PORT_E,

    /** take bit from Target ePort Number. */
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TARGET_EPORT_E,

    /** set bit to one anycase. */
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_ONE_E

} CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_ENT;

/**
* @struct CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_STC
 *
 * @brief Traffic Manager Queue Id Bit Select data.
*/
typedef struct{

    /** type of value to select bit from it. */
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_ENT selectType;

    /** @brief index of bit to select from value.
     *  (APPLICABLE RANGES: 0..15).
     *  Comment:
     */
    GT_U32 bitSelector;

} CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_STC;


/**
* @struct CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC
 *
 * @brief Traffic Manager Queue Id Bit Select Table entry.
*/
typedef struct{

    /** base value to be added to queue Id combined from bits. */
    GT_U32 queueIdBase;

    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_STC bitSelectArr[CPSS_DXCH_TM_GLUE_QUEUE_ID_BIT_WIDTH_CNS];

} CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC;

/**
* @enum CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT
 *
 * @brief TO_CPU packet mapping to queue mode.
*/
typedef enum{

    /** Use regular index for accessing queue id bits selector index. */
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_REGULAR_E,

    /** Use TO_CPU Queue Bit Selector configuration. */
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_USE_TO_CPU_CONFIG_E

} CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT;

/**
* @enum CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT
 *
 * @brief Multicast packet mapping to queue mode.
*/
typedef enum{

    /** Use regular index for accessing the queue selector index. */
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_REGULAR_E,

    /** Force EPCL QueueID. */
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_FORCE_EPCL_CONFIG_E

} CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT;

/**
* @enum CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT
 *
 * @brief Accessing to Queue Bit Selector mode.
*/
typedef enum{

    /** Use the Target Local Port[7:0] as Queue Bit Select Table index. */
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_LOCAL_TARGET_PORT_E,

    /** Use the Target Device map table output as Queue Bit Select Table index. */
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_TARGET_DEV_MAP_E

} CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT;

/**
* @enum CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_CMD_ENT
 *
 * @brief Egress L1 Packet Length Offset Command.
 * The updated length used by TM for Scaduling and Shating.
*/
typedef enum{

    /** Add specified value to L1 Packet Length. */
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_CMD_ADD_E,

    /** Subtract specified value from L1 Packet Length. */
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_CMD_SUBTRACT_E

} CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_CMD_ENT;

/**
* @struct CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_STC
 *
 * @brief Traffic Manager Egress L1 Packet Length Offset Configuration.
*/
typedef struct{

    /** L1 Packet Length Offset Command. */
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_CMD_ENT offsetCmd;

    /** @brief L1 Packet Length Bytes Offset value.
     *  (APPLICABLE RANGES: 0..0x7F).
     *  Comment:
     */
    GT_U32 offsetValue;

} CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_STC;

/**
* @internal cpssDxChTmGlueQueueMapBypassEnableSet function
* @endinternal
*
* @brief   The function sets enable/disable state of Bypass TM Queue Mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] bypass                   - GT_TRUE - bypass, GT_FALSE - mapping.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Bypassing TM Queue Mapping should not be activated on TM enabled systems.
*
*/
GT_STATUS cpssDxChTmGlueQueueMapBypassEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   bypass
);

/**
* @internal cpssDxChTmGlueQueueMapBypassEnableGet function
* @endinternal
*
* @brief   The function gets enable/disable state of Bypass TM Queue Mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] bypassPtr                - (pointer to)GT_TRUE - bypass, GT_FALSE - mapping.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueQueueMapBypassEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *bypassPtr
);

/**
* @internal cpssDxChTmGlueQueueMapBitSelectTableEntrySet function
* @endinternal
*
* @brief   The function sets the Queue Id Map Bit Select Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - entry index
*                                      (APPLICABLE RANGES: 0..255).
* @param[in] entryPtr                 - (pointer to) entry contents
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on out-of-range parameters
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueQueueMapBitSelectTableEntrySet
(
    IN  GT_U8                                              devNum,
    IN  GT_U32                                             entryIndex,
    IN  CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC   *entryPtr
);

/**
* @internal cpssDxChTmGlueQueueMapBitSelectTableEntryGet function
* @endinternal
*
* @brief   The function gets the Queue Id Map Bit Select Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - entry index
*                                      (APPLICABLE RANGES: 0..255).
*
* @param[out] entryPtr                 - (pointer to) entry contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on unexpected HW value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueQueueMapBitSelectTableEntryGet
(
    IN  GT_U8                                              devNum,
    IN  GT_U32                                             entryIndex,
    OUT CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC   *entryPtr
);

/**
* @internal cpssDxChTmGlueQueueMapTargetDeviceToIndexMapSet function
* @endinternal
*
* @brief   The function sets the Queue Id Map Bit Select Table Entry index
*         mapped to the given target device.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] targetHwDevId            - target HW device Id
*                                      (APPLICABLE RANGES: 0..1023).
* @param[in] entryIndex               - index of Queue Id Map Bit Select Table Entry
*                                      being mapped to the given target device.
*                                      (APPLICABLE RANGES: 0..255).
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on out-of-range parameters
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueQueueMapTargetDeviceToIndexMapSet
(
    IN  GT_U8           devNum,
    IN  GT_HW_DEV_NUM   targetHwDevId,
    IN  GT_U32          entryIndex
);

/**
* @internal cpssDxChTmGlueQueueMapTargetDeviceToIndexMapGet function
* @endinternal
*
* @brief   The function gets the Queue Id Map Bit Select Table Entry index
*         mapped to the given target device.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] targetHwDevId            - target HW device Id
*                                      (APPLICABLE RANGES: 0..1023).
*
* @param[out] entryIndexPtr            - (pointer to)index of Queue Id Map Bit Select Table Entry
*                                      being mapped to the given target device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueQueueMapTargetDeviceToIndexMapGet
(
    IN  GT_U8           devNum,
    IN  GT_HW_DEV_NUM   targetHwDevId,
    OUT GT_U32          *entryIndexPtr
);

/**
* @internal cpssDxChTmGlueQueueMapCpuCodeToTcMapSet function
* @endinternal
*
* @brief   The function sets the mapped TM Traffic Class for CPU Code.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] cpuCode                  - CPU code
*                                      CPSS_NET_ALL_CPU_OPCODES_E - to configure all table entries.
* @param[in] tmTc                     - mapped TM Traffic Class.
*                                      (APPLICABLE RANGES: 0..15).
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on out-of-range parameters
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueQueueMapCpuCodeToTcMapSet
(
    IN  GT_U8                      devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT   cpuCode,
    IN  GT_U32                     tmTc
);

/**
* @internal cpssDxChTmGlueQueueMapCpuCodeToTcMapGet function
* @endinternal
*
* @brief   The function gets the mapped TM Traffic Class for CPU Code.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] cpuCode                  - CPU code
*
* @param[out] tmTcPtr                  - (pointer to) mapped TM Traffic Class.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueQueueMapCpuCodeToTcMapGet
(
    IN  GT_U8                      devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT   cpuCode,
    OUT GT_U32                     *tmTcPtr
);

/**
* @internal cpssDxChTmGlueQueueMapTcToTcMapSet function
* @endinternal
*
* @brief   The function sets mapping of Traffic Class To TM Traffic Class.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] packetDsaCommand         - packet Outgoing DSA Tag command.
*                                      Forward, From CPU and Mirror only valid
* @param[in] isUnicast                - GT_TRUE - unicast, GT_FALSE - multicast
* @param[in] tc                       - Traffic Class
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] tmTc                     - Traffic Manager Traffic Class
*                                      (APPLICABLE RANGES: 0..15).
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on out-of-range parameters
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueQueueMapTcToTcMapSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_DXCH_NET_DSA_CMD_ENT   packetDsaCommand,
    IN  GT_BOOL                     isUnicast,
    IN  GT_U32                      tc,
    IN  GT_U32                      tmTc
);

/**
* @internal cpssDxChTmGlueQueueMapTcToTcMapGet function
* @endinternal
*
* @brief   The function gets mapping of Traffic Class To TM Traffic Class.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] packetDsaCommand         - packet Outgoing DSA Tag command.
*                                      Forward, From CPU and Mirror only valid
* @param[in] isUnicast                - GT_TRUE - unicast, GT_FALSE - multicast
* @param[in] tc                       - Traffic Class
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] tmTcPtr                  - (pointer to)Traffic Manager Traffic Class
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueQueueMapTcToTcMapGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_DXCH_NET_DSA_CMD_ENT   packetDsaCommand,
    IN  GT_BOOL                     isUnicast,
    IN  GT_U32                      tc,
    OUT GT_U32                      *tmTcPtr
);

/**
* @internal cpssDxChTmGlueQueueMapEgressPolicerForceSet function
* @endinternal
*
* @brief   The function sets force state for TM queue Id based policing.
*         If force the previous policer settings of the packet overridden.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] queueId                  - Traffic Manager Queue Id
*                                      (APPLICABLE RANGES: 0..0x3FFF).
* @param[in] forceMeter               - force meter,
*                                      GT_TRUE - force, GT_FALSE - don't force.
* @param[in] forceCounting            - force counting
*                                      GT_TRUE - force, GT_FALSE - don't force.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueQueueMapEgressPolicerForceSet
(
    IN  GT_U8    devNum,
    IN  GT_U32   queueId,
    IN  GT_BOOL  forceMeter,
    IN  GT_BOOL  forceCounting
);

/**
* @internal cpssDxChTmGlueQueueMapEgressPolicerForceGet function
* @endinternal
*
* @brief   The function gets force state for TM queue Id based policing.
*         If force the previous policer settings of the packet overridden.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] queueId                  - Traffic Manager Queue Id
*                                      (APPLICABLE RANGES: 0..0x3FFF).
*
* @param[out] forceMeterPtr            - (pointer to)force meter
*                                      GT_TRUE - force, GT_FALSE - don't force.
* @param[out] forceCountingPtr         - (pointer to)force counting
*                                      GT_TRUE - force, GT_FALSE - don't force.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueQueueMapEgressPolicerForceGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   queueId,
    OUT GT_BOOL  *forceMeterPtr,
    OUT GT_BOOL  *forceCountingPtr
);

/**
* @internal cpssDxChTmGlueQueueMapToCpuModeSet function
* @endinternal
*
* @brief   The function sets mode of index’s calculation in Queue Id Bit Select Table
*         for TO CPU packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] toCpuMode                - mode of index’s calculation in
*                                      Queue Id Bit Select Table for TO CPU packets.
* @param[in] toCpuSelectorIndex       - index in Queue Id Bit Select Table
*                                      for TO CPU packets.
*                                      Always stored to HW, but used only when
* @param[in] toCpuMode                is "Use TO_CPU Queue Bit Selector configuration".
*                                      (APPLICABLE RANGES: 0..0xFF).
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on out-of-range parameters
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueQueueMapToCpuModeSet
(
    IN  GT_U8                                         devNum,
    IN  CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT   toCpuMode,
    IN  GT_U32                                        toCpuSelectorIndex
);

/**
* @internal cpssDxChTmGlueQueueMapToCpuModeGet function
* @endinternal
*
* @brief   The function gets mode of index’s calculation in Queue Id Bit Select Table
*         for TO CPU packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] toCpuModePtr             - (pointer to)mode of index’s calculation in
*                                      Queue Id Bit Select Table for TO CPU packets.
* @param[out] toCpuSelectorIndexPtr    - (pointer to)index in Queue Id Bit Select Table
*                                      for TO CPU packets.
*                                      Always retrieved from HW, but used only when
*                                      toCpuMode is "Use TO_CPU Queue Bit Selector configuration".
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueQueueMapToCpuModeGet
(
    IN  GT_U8                                         devNum,
    OUT CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT   *toCpuModePtr,
    OUT GT_U32                                        *toCpuSelectorIndexPtr
);

/**
* @internal cpssDxChTmGlueQueueMapMcModeSet function
* @endinternal
*
* @brief   The function sets mode of index’s calculation in Queue Id Bit Select Table
*         for Multicast packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] mcMode                   - mode of index’s calculation in
*                                      Queue Id Bit Select Table for Multicast packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueQueueMapMcModeSet
(
    IN  GT_U8                                     devNum,
    IN  CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT   mcMode
);

/**
* @internal cpssDxChTmGlueQueueMapMcModeGet function
* @endinternal
*
* @brief   The function gets mode of index’s calculation in Queue Id Bit Select Table
*         for Multicast packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] mcModePtr                - (pointer to)mode of index’s calculation in
*                                      Queue Id Bit Select Table for Multicast packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueQueueMapMcModeGet
(
    IN  GT_U8                                     devNum,
    OUT CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT   *mcModePtr
);

/**
* @internal cpssDxChTmGlueQueueMapSelectorTableAccessModeSet function
* @endinternal
*
* @brief   The function sets Bit Selector Table Access Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - Bit Selector Table Access Mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueQueueMapSelectorTableAccessModeSet
(
    IN  GT_U8                                                 devNum,
    IN  CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT  mode
);

/**
* @internal cpssDxChTmGlueQueueMapSelectorTableAccessModeGet function
* @endinternal
*
* @brief   The function gets Bit Selector Table Access Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to)Bit Selector Table Access Mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueQueueMapSelectorTableAccessModeGet
(
    IN  GT_U8                                                 devNum,
    OUT CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT  *modePtr
);

/**
* @internal cpssDxChTmGlueQueueMapL1PacketLengthOffsetSet function
* @endinternal
*
* @brief   The function sets Egress L1 Packet Length Offset Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] physicalPort             - physical port number.
* @param[in] lengthOffsetPtr          - (pointer to)Egress L1 Packet Length Offset Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on out-of-range parameters
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueQueueMapL1PacketLengthOffsetSet
(
    IN  GT_U8                                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                                     physicalPort,
    IN  CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_STC  *lengthOffsetPtr
);

/**
* @internal cpssDxChTmGlueQueueMapL1PacketLengthOffsetGet function
* @endinternal
*
* @brief   The function gets Egress L1 Packet Length Offset Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] physicalPort             - physical port number.
*
* @param[out] lengthOffsetPtr          - (pointer to)Egress L1 Packet Length Offset Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueQueueMapL1PacketLengthOffsetGet
(
    IN  GT_U8                                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                                     physicalPort,
    OUT CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_STC  *lengthOffsetPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenCscdh */



