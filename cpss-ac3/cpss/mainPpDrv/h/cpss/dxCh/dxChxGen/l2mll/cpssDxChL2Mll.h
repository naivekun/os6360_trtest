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
* @file cpssDxChL2Mll.h
*
* @brief The CPSS DXCH L2 Mll definitions
*
* @version   21
********************************************************************************
*/
#ifndef __cpssDxChL2Mllh
#define __cpssDxChL2Mllh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>

/**
* @struct CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC
 *
 * @brief L2 MLL Exception Counters
*/
typedef struct{

    /** number of L2 MLL entries skipped. */
    GT_U32 skip;

    /** number of TTL exceptions in the L2 MLL. */
    GT_U32 ttl;

} CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC;

/**
* @enum CPSS_DXCH_L2MLL_PORT_TRUNK_CNT_MODE_ENT
 *
 * @brief the counter Set Port/Trunk mode
*/
typedef enum{

    /** @brief This counter-set counts all
     *  packets regardless of their
     *  In / Out port or Trunk.
     */
    CPSS_DXCH_L2MLL_DISREGARD_PORT_TRUNK_CNT_MODE_E = 0,

    /** @brief This counter-set counts packets Received /
     *  Transmitted via binded Port + Dev.
     */
    CPSS_DXCH_L2MLL_PORT_CNT_MODE_E                 = 1,

    /** @brief This counter-set counts packets Received /
     *  Transmitted via binded Trunk.
     */
    CPSS_DXCH_L2MLL_TRUNK_CNT_MODE_E                = 2

} CPSS_DXCH_L2MLL_PORT_TRUNK_CNT_MODE_ENT;

/**
* @enum CPSS_DXCH_L2MLL_VLAN_CNT_MODE_ENT
 *
 * @brief the counter Set vlan mode
*/
typedef enum{

    /** @brief This counter-set counts all packets
     *  regardless of their In / Out vlan.
     */
    CPSS_DXCH_L2MLL_DISREGARD_VLAN_CNT_MODE_E = 0,

    /** @brief This counter-set counts packets Received
     *  / Transmitted via binded vlan.
     */
    CPSS_DXCH_L2MLL_USE_VLAN_CNT_MODE_E       = 1

} CPSS_DXCH_L2MLL_VLAN_CNT_MODE_ENT;

/*
 * Typedef: struct CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC
 *
 * Description: L2 MLL  Counter Set interface mode configuration
 *
 * Fields:
 *      portTrunkCntMode - the counter Set Port/Trunk mode
 *      ipMode           - the cnt set IP interface mode , which protocol stack
 *                         ipv4 ,ipv6 or both (which is actully disregarding the
 *                         ip protocol)
 *      vlanMode         - the counter Set vlan mode.
 *      hwDevNum           - the binded HW devNum , relevant only if
 *                         portTrunkCntMode = CPSS_DXCH_IP_PORT_CNT_MODE_E
 *      port             - the binded por , relevant only if
 *                         portTrunkCntMode = CPSS_DXCH_IP_PORT_CNT_MODE_E
 *      trunk            - the binded trunk , relevant only if
 *                         portTrunkCntMode = CPSS_DXCH_IP_TRUNK_CNT_MODE_E
 *      portTrunk        - the above port/trunk
 *      vlanId           - the binded vlan , relevant only if
 *                         vlanMode = CPSS_DXCH_IP_USE_VLAN_CNT_MODE_E
 */
typedef struct CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STCT
{
    CPSS_DXCH_L2MLL_PORT_TRUNK_CNT_MODE_ENT portTrunkCntMode;
    CPSS_IP_PROTOCOL_STACK_ENT           ipMode;
    CPSS_DXCH_L2MLL_VLAN_CNT_MODE_ENT       vlanMode;
    GT_HW_DEV_NUM                        hwDevNum;
    union
    {
        GT_PORT_NUM                      port;
        GT_TRUNK_ID                      trunk;
    }portTrunk;
    GT_U16                               vlanId;
}CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC;

/**
* @enum CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_ENT
 *
 * @brief Selects the start entry of the MLL Table's next Link List element
*/
typedef enum{

    /** first mll is the next Linked List element */
    CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E   = 0,

    /** second mll is the next Linked List element */
    CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E

} CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_ENT;


/**
* @struct CPSS_DXCH_L2_MLL_LTT_ENTRY_STC
 *
 * @brief L2 MLL Lookup Translation Table (LTT) Entry
*/
typedef struct{

    /** @brief pointer to the MLL entry.
     *  valid range see in datasheet of specific device.
     */
    GT_U32 mllPointer;

    /** @brief Determines which of the two entries in the
     *  memory line pointed by mllPointer is the first
     *  linked-list member
     */
    CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_ENT entrySelector;

    /** @brief GT_TRUE: enable MLL entry according to match
     *  with its profile mask.
     *  GT_FALSE: MLL entry is used while ignoring
     *  the profile mask.
     */
    GT_BOOL mllMaskProfileEnable;

    /** @brief The mask profile. (APPLICABLE RANGES: 0..14)
     *  Relevant for mllMaskProfileEnable == GT_TRUE
     */
    GT_U32 mllMaskProfile;

} CPSS_DXCH_L2_MLL_LTT_ENTRY_STC;

/**
* @struct CPSS_DXCH_L2_MLL_ENTRY_STC
 *
 * @brief L2 MLL Entry
*/
typedef struct{

    /** @brief GT_TRUE: the last MLL entry in this list.
     *  GT_FALSE: there are more MLL entries in this
     *  list.
     */
    GT_BOOL last;

    /** @brief GT_TRUE: unknown UC traffic is not replicated
     *  by this MLL entry.
     *  GT_FALSE: unknown UC traffic is replicated by
     *  this MLL entry.
     */
    GT_BOOL unknownUcFilterEnable;

    /** @brief GT_TRUE: unregistered MC traffic is not
     *  replicated by this MLL entry.
     *  GT_FALSE: unregistered MC traffic is
     *  replicated by this MLL entry.
     */
    GT_BOOL unregMcFilterEnable;

    /** @brief GT_TRUE: Broadcast traffic is not replicated
     *  by this MLL entry.
     *  GT_FALSE: Broadcast traffic is replicated by
     *  this MLL entry.
     */
    GT_BOOL bcFilterEnable;

    /** @brief GT_TRUE: allow replication to the source
     *  ePort of multicast traffic.
     *  GT_FALSE: avoid replication to the source
     *  ePort of multicast traffic.
     *  (For xCat, Xcat3 ePort stands for logical port)
     */
    GT_BOOL mcLocalSwitchingEnable;

    /** @brief GT_TRUE: hop count of a replicated packet is
     *  no more than maxOutgoingHopCount.
     *  GT_FALSE: no hop count limit.
     *  (relevant for TRILL).
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL maxHopCountEnable;

    /** @brief The hop count of a replicated packet is no
     *  more than this value.
     *  Relevant only if maxHopCountEn set to GT_TRUE.
     *  (APPLICABLE RANGES: 0..63)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 maxOutgoingHopCount;

    /** @brief target Egress Interface. Valid types are:
     *  CPSS_INTERFACE_PORT_E
     *  CPSS_INTERFACE_TRUNK_E
     *  CPSS_INTERFACE_VIDX_E
     *  CPSS_INTERFACE_VID_E
     */
    CPSS_INTERFACE_INFO_STC egressInterface;

    /** @brief A per
     *  the relevant bit in this bitmap indicates
     *  whether the current MLL entry is masked or not.
     *  In each bit, a value of 0 indicates that the
     *  entry is skipped (masked), while a value of 1
     *  indicates that the current entry is active.
     *  This bitmap is indexed by the L2 MLL LTT entry
     *  field <mllMaskProfile>.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 maskBitmap;

    /** @brief if the packet's TTl is less than this field,
     *  the packet is not replicated. (APPLICABLE RANGES: 0..255)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 ttlThreshold;

    /** @brief GT_TRUE: bind entry to MLL counter indexed by
     *  <mllCounterIndex> field.
     *  GT_FALSE: entry is not binded to one of the
     *  MLL counters.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL bindToMllCounterEnable;

    /** @brief The MLL counter to bind the entry to.
     *  Relevant only if <bindToMllCounterEnable> == GT_TRUE.
     *  (APPLICABLE RANGES: 0..2)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 mllCounterIndex;

    /** @brief When enabled, and <MC Local Switching Enable>
     *  is disabled, source filtering ignores the least
     *  significant bit of the ePort.
     *  The assumption is that the two ePorts in the 1+1
     *  scheme are two contiguous ePort numbers, that
     *  differ only in the lsbit.
     *  (For xCat,Xcat3 ePort stands for logical port)
     */
    GT_BOOL onePlusOneFilteringEnable;

    /** @brief Mesh ID number
     *  Used for split horizon filtering (e.g. in VPLS,
     *  TRILL, MiM).
     *  Bobcat2; Caelum; Bobcat3; Aldrin; AC3X: (APPLICABLE RANGES: 0..255)
     *  xCat, xCat3: (APPLICABLE RANGES: 0..3)
     *  xCat device: applicable starting from revision C0
     */
    GT_U32 meshId;

    /** @brief whether this nexthop is tunnel start entry, in
     *  which case the outInteface & mac are irrlevant and
     *  the tunnel id is used.
     *  (APPLICABLE DEVICES: xCat, xCat3)
     *  xCat device: applicable starting from revision C0
     */
    GT_BOOL tunnelStartEnable;

    /** @brief the tunnel pointer in case this is a tunnel start
     *  <tunnelStartEnable> is GT_TRUE.
     *  (APPLICABLE DEVICES: xCat, xCat3)
     *  xCat device: applicable starting from revision C0
     */
    GT_U32 tunnelStartPointer;

    /** @brief the type of the tunneled, passenger packet.
     *  (APPLICABLE DEVICES: xCat, xCat3)
     *  xCat device: applicable starting from revision C0
     */
    CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT tunnelStartPassengerType;

} CPSS_DXCH_L2_MLL_ENTRY_STC;

/**
* @struct CPSS_DXCH_L2_MLL_PAIR_STC
 *
 * @brief Representation of the l2 muticast Link List pair in HW,
*/
typedef struct{

    /** the first Mll of the pair */
    CPSS_DXCH_L2_MLL_ENTRY_STC firstMllNode;

    /** the second Mll of the pair */
    CPSS_DXCH_L2_MLL_ENTRY_STC secondMllNode;

    /** pointer to the next MLL entry */
    GT_U32 nextPointer;

    /** @brief Selects the start entry of the MLL Table's next
     *  Linked List element
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_ENT entrySelector;

} CPSS_DXCH_L2_MLL_PAIR_STC;

/**
* @internal cpssDxChL2MllLookupForAllEvidxEnableSet function
* @endinternal
*
* @brief   Enable or disable MLL lookup for all multi-target packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE: MLL lookup is performed for all multi-target packets.
*                                      GT_FALSE: MLL lookup is performed only for multi-target
*                                      packets with eVIDX >= 4K. For packets with
*                                      eVidx < 4K L2 MLL is not accessed.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllLookupForAllEvidxEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
);

/**
* @internal cpssDxChL2MllLookupForAllEvidxEnableGet function
* @endinternal
*
* @brief   Get enabling status of MLL lookup for all multi-target packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: MLL lookup is performed for all multi-target
*                                      packets.
*                                      GT_FALSE: MLL lookup is performed only for multi-target
*                                      packets with eVIDX >= 4K. For packets with
*                                      eVidx < 4K L2 MLL is not accessed.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllLookupForAllEvidxEnableGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChL2MllLookupMaxVidxIndexSet function
* @endinternal
*
* @brief   Set the maximal VIDX value that refers to a port distribution list(VIDX).
*         By default the value is 4K-1
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] maxVidxIndex             - maximal VIDX value.
*                                      When cpssDxChL2MllLookupForAllEvidxEnableSet == FALSE
*                                      L2 replication is performed to all eVidx > maxVidxIndex
*                                      (APPLICABLE RANGES: 0..0xFFF)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant only when cpssDxChL2MllLookupForAllEvidxEnableSet == FALSE
*
*/
GT_STATUS cpssDxChL2MllLookupMaxVidxIndexSet
(
    IN GT_U8   devNum,
    IN GT_U32  maxVidxIndex
);

/**
* @internal cpssDxChL2MllLookupMaxVidxIndexGet function
* @endinternal
*
* @brief   Get the maximal VIDX value that refers to a port distribution list(VIDX).
*         By default the value is 4K-1
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] maxVidxIndexPtr          - (pointer to) maximal VIDX value.
*                                      When <Enable MLL Lookup for All eVidx> == FALSE
*                                      L2 replication is performed to all eVidx > <Max VIDX Index>
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant only when cpssDxChL2MllLookupForAllEvidxEnableGet== FALSE
*
*/
GT_STATUS cpssDxChL2MllLookupMaxVidxIndexGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *maxVidxIndexPtr
);

/**
* @internal cpssDxChL2MllTtlExceptionConfigurationSet function
* @endinternal
*
* @brief   Set configuration for L2 MLL TTL Exceptions.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trapEnable               - GT_TRUE: packet is trapped to the CPU with <cpuCode>
*                                      if packet's TTL is less than MLL entry field
*                                      <TTL Threshold>
*                                      GT_FALSE: no packet trap to CPU due to a TTL exception
*                                      in the MLL.
* @param[in] cpuCode                  - CPU code of packets that are trapped to CPU due to a
*                                      TTL exception in the MLL.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllTtlExceptionConfigurationSet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  trapEnable,
    IN CPSS_NET_RX_CPU_CODE_ENT cpuCode
);

/**
* @internal cpssDxChL2MllTtlExceptionConfigurationGet function
* @endinternal
*
* @brief   Get configuration of L2 MLL TTL Exceptions.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] trapEnablePtr            - (pointer to)
*                                      GT_TRUE: packet is trapped to the CPU with <cpuCode>
*                                      if packet's TTL is less than MLL entry field
*                                      <TTL Threshold>
*                                      GT_FALSE: no packet trap to CPU due to a TTL
*                                      exception in the MLL.
* @param[out] cpuCodePtr               - (pointer to) CPU code of packets that are trapped to
*                                      CPU due to a TTL exception in the MLL.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllTtlExceptionConfigurationGet
(
    IN  GT_U8                    devNum,
    OUT GT_BOOL                  *trapEnablePtr,
    OUT CPSS_NET_RX_CPU_CODE_ENT *cpuCodePtr
);

/**
* @internal cpssDxChL2MllExceptionCountersGet function
* @endinternal
*
* @brief   Get L2 MLL exception counters.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] countersPtr              - (pointer to) L2 MLL exception counters.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChL2MllExceptionCountersGet
(
    IN  GT_U8                                   devNum,
    OUT CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC  *countersPtr
);

/**
* @internal cpssDxChL2MllPortGroupExceptionCountersGet function
* @endinternal
*
* @brief   Get L2 MLL exception counters.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] countersPtr              - (pointer to) L2 MLL exception counters.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChL2MllPortGroupExceptionCountersGet
(
    IN  GT_U8                                   devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    OUT CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC  *countersPtr
);

/**
* @internal cpssDxChL2MllCounterGet function
* @endinternal
*
* @brief   Get L2 MLL counter.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - counter number. (APPLICABLE DEVICES Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon, Aldrin, AC3X)
*                                      (APPLICABLE RANGES: 0..2)
*
* @param[out] counterPtr               - (pointer to) L2 MLL counter value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChL2MllCounterGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  index,
    OUT GT_U32  *counterPtr
);

/**
* @internal cpssDxChL2MllPortGroupCounterGet function
* @endinternal
*
* @brief   Get L2 MLL counter.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] index                    - counter number. (APPLICABLE DEVICES Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X)(APPLICABLE RANGES: 0..2)
*
* @param[out] counterPtr               - (pointer to) L2 MLL counter value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChL2MllPortGroupCounterGet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U32              index,
    OUT GT_U32              *counterPtr
);


/**
* @internal cpssDxChL2MllSetCntInterfaceCfg function
* @endinternal
*
* @brief   Sets a L2 mll counter set's bounded inteface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] mllCntSet                - l2 mll counter set
*                                      (APPLICABLE RANGES: 0..1)
* @param[in] interfaceCfgPtr          - the L2 mll counter interface configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_OUT_OF_RANGE          - parameter is out of range
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllSetCntInterfaceCfg
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           mllCntSet,
    IN CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC    *interfaceCfgPtr
);

/**
* @internal cpssDxChL2MllGetCntInterfaceCfg function
* @endinternal
*
* @brief   Gets a L2 mll counter set's bounded inteface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] mllCntSet                - l2 mll counter set
*                                      (APPLICABLE RANGES: 0..1)
*
* @param[out] interfaceCfgPtr          - the L2 mll counter interface configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_OUT_OF_RANGE          - parameter is out of range
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllGetCntInterfaceCfg
(
    IN  GT_U8                                            devNum,
    IN  GT_U32                                           mllCntSet,
    OUT CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC    *interfaceCfgPtr
);

/**
* @internal cpssDxChL2MllPortGroupMcCntGet function
* @endinternal
*
* @brief   Get the L2MLL MC counter
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] mllCntSet                - l2 mll counter set
*                                      (APPLICABLE RANGES: 0..1)
*
* @param[out] mllOutMCPktsPtr          - According to the configuration of this cnt set, The
*                                      number of Multicast packets Duplicated by the
*                                      L2 MLL Engine
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllPortGroupMcCntGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  mllCntSet,
    OUT GT_U32                  *mllOutMCPktsPtr
);

/**
* @internal cpssDxChL2MllMcCntGet function
* @endinternal
*
* @brief   Get the L2MLL MC counter
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] mllCntSet                - l2 mll counter set
*                                      (APPLICABLE RANGES: 0..1)
*
* @param[out] mllOutMCPktsPtr          - According to the configuration of this cnt set, The
*                                      number of Multicast packets Duplicated by the
*                                      L2 MLL Engine
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllMcCntGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mllCntSet,
    OUT GT_U32  *mllOutMCPktsPtr
);


/**
* @internal cpssDxChL2MllSilentDropCntGet function
* @endinternal
*
* @brief   Get the silent drops in the L2 MLL replication block.
*         A drop occurs for a packet that was:
*         - Replicated in the TTI/IP MLL
*         AND
*         - All the elements of the linked list are filtered
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] silentDropPktsPtr        - (pointer to) the number of counted silent dropped packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllSilentDropCntGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *silentDropPktsPtr
);

/**
* @internal cpssDxChL2MllPortGroupSilentDropCntGet function
* @endinternal
*
* @brief   Get the silent drops in the L2 MLL replication block.
*         A drop occurs for a packet that was:
*         - Replicated in the TTI/IP MLL
*         AND
*         - All the elements of the linked list are filtered
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] silentDropPktsPtr        - (pointer to) the number of counted silent dropped packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllPortGroupSilentDropCntGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *silentDropPktsPtr
);

/**
* @internal cpssDxChL2MllLttEntrySet function
* @endinternal
*
* @brief   Set L2 MLL Lookup Translation Table (LTT) entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the LTT index. eVidx range.
* @param[in] lttEntryPtr              - (pointer to) L2 MLL LTT entry.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - LTT entry's parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllLttEntrySet
(
    IN GT_U8                            devNum,
    IN GT_U32                           index,
    IN CPSS_DXCH_L2_MLL_LTT_ENTRY_STC   *lttEntryPtr
);

/**
* @internal cpssDxChL2MllLttEntryGet function
* @endinternal
*
* @brief   Get L2 MLL Lookup Translation Table (LTT) entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the LTT index. eVidx range.
*
* @param[out] lttEntryPtr              - (pointer to) L2 MLL LTT entry.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllLttEntryGet
(
    IN  GT_U8                            devNum,
    IN  GT_U32                           index,
    OUT CPSS_DXCH_L2_MLL_LTT_ENTRY_STC   *lttEntryPtr
);

/**
* @internal cpssDxChL2MllPairWrite function
* @endinternal
*
* @brief   Write L2 MLL pair entry to L2 MLL Table.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] mllPairEntryIndex        - the mll Pair entry index
* @param[in] mllPairWriteForm         - the way to write the Mll pair, first part only/
*                                      second + next pointer only/ whole Mll pair
* @param[in] mllPairEntryPtr          - the L2 link list pair entry
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - MLL entry's parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChL2MllPairWrite
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       mllPairEntryIndex,
    IN CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT           mllPairWriteForm,
    IN CPSS_DXCH_L2_MLL_PAIR_STC                    *mllPairEntryPtr
);

/**
* @internal cpssDxChL2MllPairRead function
* @endinternal
*
* @brief   Read L2 MLL pair entry from L2 MLL Table.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] mllPairEntryIndex        - the mll Pair entry index
* @param[in] mllPairReadForm          - the way to read the Mll pair, first part only/
*                                      second + next pointer only/ whole Mll pair
*
* @param[out] mllPairEntryPtr          - the L2 link list pair entry
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChL2MllPairRead
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      mllPairEntryIndex,
    IN  CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT          mllPairReadForm,
    OUT CPSS_DXCH_L2_MLL_PAIR_STC                  *mllPairEntryPtr
);


/**
* @internal cpssDxChL2MllMultiTargetPortEnableSet function
* @endinternal
*
* @brief   Enable multi-target port mapping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - multi-target port mapping status:
*                                      GT_TRUE: enable multi-target port mapping
*                                      GT_FALSE: disable multi-target port mapping
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS cpssDxChL2MllMultiTargetPortEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
);

/**
* @internal cpssDxChL2MllMultiTargetPortEnableGet function
* @endinternal
*
* @brief   Get the multi-target port mapping enabling status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) multi-target port mapping status:
*                                      GT_TRUE: multi-target port mapping is enabled
*                                      GT_FALSE: multi-target port mapping is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS cpssDxChL2MllMultiTargetPortEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
);

/**
* @internal cpssDxChL2MllMultiTargetPortSet function
* @endinternal
*
* @brief   Set the multi-target port value and mask
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] value                    - multi-target port value
*                                      (APPLICABLE RANGES: 0..0x1ffff)
* @param[in] mask                     - multi-target port mask
*                                      (APPLICABLE RANGES: 0..0x1ffff)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range value or mask
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS cpssDxChL2MllMultiTargetPortSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      value,
    IN  GT_U32                      mask
);

/**
* @internal cpssDxChL2MllMultiTargetPortGet function
* @endinternal
*
* @brief   Get the multi-target port value and mask
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] valuePtr                 - (pointer to) multi-target port value
* @param[out] maskPtr                  - (pointer to) multi-target port mask
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS cpssDxChL2MllMultiTargetPortGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *valuePtr,
    OUT GT_U32                      *maskPtr
);

/**
* @internal cpssDxChL2MllMultiTargetPortBaseSet function
* @endinternal
*
* @brief   Set the multi-target port base
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portBase                 - multi-target port base
*                                      (APPLICABLE RANGES: 0..0x1ffff)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range port base
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS cpssDxChL2MllMultiTargetPortBaseSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portBase
);

/**
* @internal cpssDxChL2MllMultiTargetPortBaseGet function
* @endinternal
*
* @brief   Get the multi-target port base
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] portBasePtr              - (pointer to) multi-target port base
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS cpssDxChL2MllMultiTargetPortBaseGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *portBasePtr
);

/**
* @internal cpssDxChL2MllPortToVidxBaseSet function
* @endinternal
*
* @brief   Set the port to VIDX base
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vidxBase                 - port VIDX base
*                                      (APPLICABLE RANGES: 0..0xffff)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range vidx
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS cpssDxChL2MllPortToVidxBaseSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      vidxBase
);

/**
* @internal cpssDxChL2MllPortToVidxBaseGet function
* @endinternal
*
* @brief   Get the port to VIDX base
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] vidxBasePtr              - (pointer to) port VIDX base
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS cpssDxChL2MllPortToVidxBaseGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *vidxBasePtr
);

/**
* @internal cpssDxChL2MllSourceBasedFilteringConfigurationSet function
* @endinternal
*
* @brief   Set configuration for Source Based L2 MLL Filtering.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_FALSE: MESH ID filtering is not enabled
*                                      GT_TRUE:  MESH ID filtering is enabled
* @param[in] meshIdOffset             - The MESH ID assigned to a packet is conveyed in the SrcID
*                                      assigned to the packet.
*                                      This configuration specifies the location of the LSB of
*                                      the MESH ID in the SrcID field assigned to a packet
*                                      (APPLICABLE RANGES: 0..11)
* @param[in] meshIdSize               - Specifies the number of bits that are used for
*                                      the MESH ID in the SrcID field.
*                                      Together with <MESH ID offset>, the MESH ID assigned to
*                                      a packet can be extracted:
*                                      Packet's MESH ID = <SST ID> (<MESH ID size> - 1 +
*                                      <MESH ID offset> : <MESH ID offset>)
*                                      (APPLICABLE RANGES: 1..8)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_OUT_OF_RANGE          - on wrong meshIdOffset and meshIdSize
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllSourceBasedFilteringConfigurationSet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  enable,
    IN GT_U32                   meshIdOffset,
    IN GT_U32                   meshIdSize
);

/**
* @internal cpssDxChL2MllSourceBasedFilteringConfigurationGet function
* @endinternal
*
* @brief   Get configuration for Source Based L2 MLL Filtering.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: MESH ID filtering is not enabled
*                                      GT_TRUE:  MESH ID filtering is enabled
* @param[out] meshIdOffsetPtr          - (pointer to) The MESH ID assigned to a packet is conveyed
*                                      in the SrcID assigned to the packet.
*                                      This configuration specifies the location of the LSB of
*                                      the MESH ID in the SrcID field assigned to a packet
*                                      (APPLICABLE RANGES: 0..11)
* @param[out] meshIdSizePtr            - (pointer to) Specifies the number of bits that are used
*                                      for the MESH ID in the SrcID field.
*                                      Together with <MESH ID offset>, the MESH ID assigned to
*                                      a packet can be extracted:
*                                      Packet's MESH ID = <SST ID> (<MESH ID size> - 1 +
*                                      <MESH ID offset> : <MESH ID offset>)
*                                      (APPLICABLE RANGES: 1..8)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllSourceBasedFilteringConfigurationGet
(
    IN  GT_U8                    devNum,
    OUT GT_BOOL                  *enablePtr,
    OUT GT_U32                   *meshIdOffsetPtr,
    OUT GT_U32                   *meshIdSizePtr
);

/**
* @internal cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet function
* @endinternal
*
* @brief   Set virtual port mapping table entry. This entry maps
*         virtual target port to L2MLL entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] targetDevNum             - target device number. (APPLICABLE RANGES: 0..15)
* @param[in] targetPortNum            - target port number.   (APPLICABLE RANGES: 0..63)
* @param[in] mllPointer               - pointer to L2 MLL entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_HW_ERROR              - on Hardware error.
*
* @note If the target supports APS 1+1, there are 2 L2MLL entries in this list, one
*       with a target logical port for the working path and one with a target
*       logical port for the protection path. It is required that these 2 target
*       logical ports be consecutive and differ only in the least significant bit of
*       the port number.
*       xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet
(
    IN GT_U8                             devNum,
    IN GT_U8                             targetDevNum,
    IN GT_U8                             targetPortNum,
    IN GT_U32                            mllPointer
);

/**
* @internal cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet function
* @endinternal
*
* @brief   Get virtual port mapping table entry. This entry maps
*         virtual target port to L2MLL entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] targetDevNum             - target device number. (APPLICABLE RANGES: 0..15)
* @param[in] targetPortNum            - target port number.   (APPLICABLE RANGES: 0..63)
*
* @param[out] mllPointer               - (pointer to) L2 MLL entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_HW_ERROR              - on Hardware error.
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet
(
    IN  GT_U8                                devNum,
    IN  GT_U8                                targetDevNum,
    IN  GT_U8                                targetPortNum,
    OUT GT_U32                              *mllPointer
);

/**
* @internal cpssDxChL2MllVidxEnableSet function
* @endinternal
*
* @brief   Enable/disable MLL lookup for given vidx on the specified device.
*         When enabled the VIDX value is used as the L2MLL index.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] vidx                     - VIDX value. Valid range see datasheet for specific device.
* @param[in] enable                   - GT_TRUE: MLL lookup per VIDX is enabled.
*                                      GT_FALSE: MLL lookup per VIDX is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChL2MllVidxEnableSet
(
    IN GT_U8   devNum,
    IN GT_U16  vidx,
    IN GT_BOOL enable
);

/**
* @internal cpssDxChL2MllVidxEnableGet function
* @endinternal
*
* @brief   Get enabling status of MLL lookup for given vidx on the specified device.
*         When enabled the VIDX value is used as the L2MLL index.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] vidx                     - VIDX value. Valid range see datasheet for specific device.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: MLL lookup per VIDX is enabled.
*                                      GT_FALSE: MLL lookup per VIDX is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChL2MllVidxEnableGet
(
    IN GT_U8     devNum,
    IN GT_U16    vidx,
    OUT GT_BOOL *enablePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChL2Mllh */

