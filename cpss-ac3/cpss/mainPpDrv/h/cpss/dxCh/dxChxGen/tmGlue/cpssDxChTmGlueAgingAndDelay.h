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
* @file cpssDxChTmGlueAgingAndDelay.h
*
* @brief The CPSS DXCH Aging and Delay Measurements definitions
*
* @version   4
********************************************************************************
*/
#ifndef __cpssDxChTmGlueAgingAndDelayh
#define __cpssDxChTmGlueAgingAndDelayh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

/**
* @struct CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC
 *
 * @brief Aging Profile Thresholds
 * The time that the packet was stored in TM Queue is compared to
 * this thresholds. In case time is larger then threshold2 and
 * "Drop Aged Packet" setting is enabled --> the packet is dropped,
 * in all other cases the packet's Queue TM Drop Profile is changed
 * if a Threshold is crossed.
*/
typedef struct{

    /** first Threshold value (APPLICABLE RANGES: 0..0xFFFFF) */
    GT_U32 threshold0;

    /** second Threshold value (APPLICABLE RANGES: 0..0xFFFFF) */
    GT_U32 threshold1;

    /** third Threshold value (APPLICABLE RANGES: 0..0xFFFFF) */
    GT_U32 threshold2;

} CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC;

/**
* @struct CPSS_DXCH_TM_AGING_AND_DELAY_STATISTICS_STC
 *
 * @brief Read the counters representing packets that were queue to a
 * specific TM Queue.
 * depending on cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableSet
 * aged packets can be taken into account of the statistics
*/
typedef struct{

    /** @brief number of packets that passed through a TM Queue
     *  Counter is 24 bits
     */
    GT_U32 packetCounter;

    /** @brief number of non Aged octets that passed through a
     *  TM Queue
     *  Octet counter is 40 bits
     */
    GT_U64 octetCounter;

    /** @brief count the waiting time of all packets that passed
     *  through a TM Queue
     *  latency time counter is 44 bits
     */
    GT_U64 latencyTimeCounter;

    /** @brief number of aged packets that passed through a TM Queue
     *  Counter is 24 bits
     */
    GT_U32 agedPacketCounter;

    /** @brief maximal waiting time for packets that passed
     *  through a TM Queue
     *  Counter is 20 bits
     */
    GT_U32 maxLatencyTime;

    /** @brief minimal waiting time for packets that passed
     *  through a TM Queue
     *  Counter is 20 bits
     */
    GT_U32 minLatencyTime;

} CPSS_DXCH_TM_AGING_AND_DELAY_STATISTICS_STC;


/**
* @internal cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableSet function
* @endinternal
*
* @brief   Enable dropping of aged packets.
*         Sets if packets that are stored longer than highest aging threhold should
*         be dropped or queue Drop Profile shuld be changed to highest threshold DP.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  Drop
*                                      GT_FALSE: No Drop - change DP
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableSet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  enable
);

/**
* @internal cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableGet function
* @endinternal
*
* @brief   Get dropping aged packets status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE:  Drop
*                                      GT_FALSE: No Drop - change DP
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableGet
(
    IN  GT_U8                    devNum,
    OUT GT_BOOL                  *enablePtr
);

/**
* @internal cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableSet function
* @endinternal
*
* @brief   Enable statistics on packets dropped due to aging in
*         TM queue aging and delay counter set
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  Count
*                                      GT_FALSE: Do not count
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableSet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  enable
);

/**
* @internal cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableGet function
* @endinternal
*
* @brief   Get status of enable statistics on packets dropped due to aging in
*         TM queue aging and delay counter set
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE:  Count
*                                      GT_FALSE: Do not count
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableGet
(
    IN  GT_U8                    devNum,
    OUT GT_BOOL                  *enablePtr
);

/**
* @internal cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableSet function
* @endinternal
*
* @brief   Enable Drop of packets transmitted through TM-Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] tmPort                   - TM port
*                                      (APPLICABLE RANGES: 0..191)
* @param[in] enable                   - if enabled all packet transmitted through
*                                      TM-Port are dropped.
*                                      GT_TRUE  = Dropped Packets
*                                      GT_FALSE = Regular functionality
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_U32           tmPort,
    IN  GT_BOOL          enable
);

/**
* @internal cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet function
* @endinternal
*
* @brief   Return drop status of packets transmitted through TM-Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] tmPort                   - TM port
*                                      (APPLICABLE RANGES: 0..191)
*
* @param[out] enablePtr                - (pointer to) drop status of all packet
*                                      transmitted through TM-Port
*                                      GT_TRUE  = Dropped Packets
*                                      GT_FALSE = Regular functionality
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet
(
    IN  GT_U8            devNum,
    IN  GT_U32           tmPort,
    IN  GT_BOOL          *enablePtr
);

/**
* @internal cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet function
* @endinternal
*
* @brief   Set Configuration for Dropped Aged Packet Counter
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] agedPacketCouterQueue    - This is the mask on queue number for
*                                      triggering the aging drop counter
*                                      (APPLICABLE RANGES: 0..16383)
* @param[in] agedPacketCouterQueue    - This is the queue number for triggering
*                                      the aging drop counter
*                                      (APPLICABLE RANGES: 0..16383)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counter is increment if the following logic apply:
*       (agedPacketCouterQueueMask & PacketQueue) = agedPacketCouterQueue
*
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet
(
    IN  GT_U8            devNum,
    IN  GT_U32           agedPacketCouterQueueMask,
    IN  GT_U32           agedPacketCouterQueue
);

/**
* @internal cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet function
* @endinternal
*
* @brief   Get Configuration for Dropped Aged Packet Counter
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] agedPacketCouterQueueMaskPtr - (pointer to) the mask on queue number
*                                      for triggering the aging drop counter
* @param[out] agedPacketCouterQueuePtr - (pointer to) the queue number for
*                                      triggering the aging drop counter
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counter is increment if the following logic apply:
*       (agedPacketCouterQueueMask & PacketQueue) = agedPacketCouterQueue
*
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet
(
    IN  GT_U8            devNum,
    OUT GT_U32           *agedPacketCouterQueueMaskPtr,
    OUT GT_U32           *agedPacketCouterQueuePtr
);

/**
* @internal cpssDxChTmGlueAgingAndDelayAgedPacketCounterGet function
* @endinternal
*
* @brief   Read the counter that counts the number of aged packets that were dropped
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] agedPacketCounterPtr     - (pointer to) the counter value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Clear On Read Counter
*       The counter is increment if the following logic apply:
*       (agedPacketCouterQueueMask & PacketQueue) = agedPacketCouterQueue
*
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayAgedPacketCounterGet
(
    IN  GT_U8            devNum,
    OUT GT_U32           *agedPacketCounterPtr
);

/**
* @internal cpssDxChTmGlueAgingAndDelayTimerResolutionSet function
* @endinternal
*
* @brief   Set aging's timer units resolution.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*                                      timerResolution     - timer resolution in nanosec
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayTimerResolutionSet
(
    IN    GT_U8                    devNum,
    INOUT GT_U32                   *timerResolutionPtr
);

/**
* @internal cpssDxChTmGlueAgingAndDelayTimerResolutionGet function
* @endinternal
*
* @brief   Get aging's timer units resolution.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] timerResolutionPtr       - (pointer to) timer resolution in nanosec
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayTimerResolutionGet
(
    IN  GT_U8                    devNum,
    OUT GT_U32                   *timerResolutionPtr
);

/**
* @internal cpssDxChTmGlueAgingAndDelayTimerGet function
* @endinternal
*
* @brief   The aging and delay timer is the reference clock used for calculating
*         the time each packet was stored.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] timerPtr                 - (pointer to) the aging and delay timer
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The timer is incremented by 1 each time the free running counter
*       reach "0".
*       see cpssDxChTmGlueAgingAndDelayTimerResolutionSet
*
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayTimerGet
(
    IN  GT_U8                    devNum,
    OUT GT_U32                   *timerPtr
);

/**
* @internal cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet function
* @endinternal
*
* @brief   Configure the Aging Profile Thresholds Table
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] profileId                - profileId
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] thresholdsPtr            - (pointer to) thresholds values.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The thresholds units are set in cpssDxChTmGlueAgingAndDelayTimerResolutionSet
*
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet
(
    IN GT_U8                                                devNum,
    IN GT_U32                                               profileId,
    IN CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC  *thresholdsPtr
);

/**
* @internal cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet function
* @endinternal
*
* @brief   Get the Aging Profile Thresholds Table configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] profileId                - profileId
*                                      (APPLICABLE RANGES: 0..15)
*
* @param[out] thresholdsPtr            - (pointer to) thresholds values.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The thresholds units are set in cpssDxChTmGlueAgingAndDelayTimerResolutionSet
*
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet
(
    IN  GT_U8                                               devNum,
    IN  GT_U32                                              profileId,
    OUT CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC *thresholdsPtr
);


/**
* @internal cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet function
* @endinternal
*
* @brief   Configure Queue Aging Profile Table that Maps a Traffic Manager Queue ID
*         to an aging profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] queueId                  - Queue Id to configure
*                                      (APPLICABLE RANGES: 0..16383)
* @param[in] profileId                - Profile Id value to set for the given queueId
*                                      (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet
(
    IN GT_U8        devNum,
    IN GT_U32       queueId,
    IN GT_U32       profileId
);

/**
* @internal cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet function
* @endinternal
*
* @brief   Get Queue Aging Profile Table configuration that Maps a Traffic Manager
*         Queue ID to an aging profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] queueId                  - Queue Id to configure
*                                      (APPLICABLE RANGES: 0..16383)
*
* @param[out] profileIdPtr             - (pointer to)
*                                      Profile Id value to set for the given queueId
*                                      (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       queueId,
    OUT GT_U32       *profileIdPtr
);


/**
* @internal cpssDxChTmGlueAgingAndDelayCounterQueueIdSet function
* @endinternal
*
* @brief   Map one of aging and delay counter sets to a queuedId number.
*         The counters in the set perform delay and aging statistics on the packets
*         queued to the configured Queue-ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] counterSetIndex          - Aging and delay counter set index
*                                      (APPLICABLE RANGES: 0..99)
* @param[in] queueId                  - queue id
*                                      (APPLICABLE RANGES: 0..16383)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayCounterQueueIdSet
(
    IN GT_U8                    devNum,
    IN GT_U32                   counterSetIndex,
    IN GT_U32                   queueId
);

/**
* @internal cpssDxChTmGlueAgingAndDelayCounterQueueIdGet function
* @endinternal
*
* @brief   Get Mapping of aging and delay counter set to a queuedId number.
*         The counters in the set perform delay and aging statistics on the packets
*         queued to the configured Queue-ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] counterSetIndex          - Aging and delay counter set index
*                                      (APPLICABLE RANGES: 0..99)
*
* @param[out] queueIdPtr               - (pointer to) queue id
*                                      (APPLICABLE RANGES: 0..16383)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayCounterQueueIdGet
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   counterSetIndex,
    OUT GT_U32                   *queueIdPtr
);

/**
* @internal cpssDxChTmGlueAgingAndDelayCountersGet function
* @endinternal
*
* @brief   Read all the counters of aging and delay counter set
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] counterSetIndex          - aging and delay counter set
*                                      (APPLICABLE RANGES: 0..99)
*
* @param[out] agingCountersPtr         - (pointer to) aging and delay counters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Clear On Read Counter
*
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayCountersGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      counterSetIndex,
    OUT CPSS_DXCH_TM_AGING_AND_DELAY_STATISTICS_STC *agingCountersPtr
);

/**
* @internal cpssDxChTmGlueAgingAndDelayPerPortAgedPacketCounterGet function
* @endinternal
*
* @brief   Read the counter that counts the number of aged out packets
*         that should have been transmiited from tm-port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] tmPort                   - TM port
*                                      (APPLICABLE RANGES: 0..191)
*
* @param[out] agedOutPacketCounterPtr  - (pointer to) Aged Out Packet Counter
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Clear On Read Counter
*
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayPerPortAgedPacketCounterGet
(
    IN  GT_U8            devNum,
    IN  GT_U32           tmPort,
    OUT GT_U32           *agedOutPacketCounterPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChAgingAndDMh */


