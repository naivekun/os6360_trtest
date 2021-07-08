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
* @file cpssDxChTam.h
*
* @brief CPSS definitions for configuring, gathering info and statistics
*        for the TAM (Telemetry Analytics and Monitoring) feature
*
* @version   1
********************************************************************************
*/

#ifndef __cpssDxChTamh
#define __cpssDxChTamh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/extServices/os/gtOs/gtGenTypes.h>

#define CPSS_DXCH_TAM_HISTOGRAM_BIN_VALUES      4 /*PRV_CPSS_DXCH_TAM_FW_BINS*/
#define CPSS_DXCH_TAM_HISTOGRAM_BIN_LEVELS      CPSS_DXCH_TAM_HISTOGRAM_BIN_VALUES - 1 /*PRV_CPSS_DXCH_TAM_FW_BIN_LEVELS*/


/**
* @enum CPSS_DXCH_TAM_COUNTER_TYPE_ENT
*
* @brief Define type for telemetry measured resources - the counters on buffers.
*        Identifies a specific counter within the HW hierarchy.
*
*/
typedef enum
{
    /** BM Global Buffer Counter */
    CPSS_DXCH_TAM_INGRESS_GLOBAL_BUFFER_COUNTER_E = 0x1,

    /** Ingress Sniffer Buffers Counter - IngressMirroredBuffsCnt */
    CPSS_DXCH_TAM_INGRESS_GLOBAL_MIRROR_BUFFER_COUNTER_E,

    /** TBD */
    CPSS_DXCH_TAM_INGRESS_PORT_BUFFER_COUNTER_E,

    /** PFC Group of Ports TC Counter. per port+queue. */
    CPSS_DXCH_TAM_INGRESS_PG_TC_BUFFER_COUNTER_E,

    /** Total Buffers Counter */
    CPSS_DXCH_TAM_EGRESS_GLOBAL_BUFFER_COUNTER_E,

    /** Multicast Buffers Counter */
    CPSS_DXCH_TAM_EGRESS_GLOBAL_MULTI_BUFFER_COUNTER_E,

    /** Egress Sniffer Buffers Counter - EgressMirrorBuffsCnt */
    CPSS_DXCH_TAM_EGRESS_GLOBAL_MIRROR_BUFFER_COUNTER_E,

    /** Port Buffers Counter. per port. */
    CPSS_DXCH_TAM_EGRESS_PORT_BUFFER_COUNTER_E,

    /** Queue Maintenance Buffers Tail Drop Queue Buffer Counters. per port+queue. */
    CPSS_DXCH_TAM_EGRESS_QUEUE_BUFFER_COUNTER_E,

    /** Priority Shared Buffers Counter. per TBD.  */
    CPSS_DXCH_TAM_EGRESS_SHARED_POOL_BUFFER_COUNTER_E,

    /** Microburst duration counter */
    CPSS_DXCH_TAM_MICROBURST_DURATION_COUNTER_E,

}CPSS_DXCH_TAM_COUNTER_TYPE_ENT;

/**
* @enum CPSS_DXCH_TAM_TRACKING_MODE_ENT
*
* @brief Define tracking modes for telemetry measured resources.
*
*/
typedef enum
{
    /** Current value tracking mode */
    CPSS_DXCH_TAM_TRACKING_MODE_CURRENT_E,

    /** Peak value tracking mode */
    CPSS_DXCH_TAM_TRACKING_MODE_PEAK_E,

    /** Minimum value tracking mode */
    CPSS_DXCH_TAM_TRACKING_MODE_MINIMUM_E,

    /** Average value tracking mode */
    CPSS_DXCH_TAM_TRACKING_MODE_AVERAGE_E,

    /** Histogram tracking mode */
    CPSS_DXCH_TAM_TRACKING_MODE_HISTOGRAM_E

}CPSS_DXCH_TAM_TRACKING_MODE_ENT;

/**
* @struct CPSS_DXCH_TAM_COUNTER_ID_STC
*
* @brief This structure defines the unique counter ID.
*/
typedef struct
{
    /** the counter type. next fields are according to counter type */
    CPSS_DXCH_TAM_COUNTER_TYPE_ENT      type;
    GT_U32                              id;
    GT_U32                              port;
    GT_U32                              queue;

} CPSS_DXCH_TAM_COUNTER_ID_STC;

/**
* @struct CPSS_DXCH_TAM_STATISTIC_STC
*
* @brief This structure defines the unique counter statistic.
*/
typedef struct
{
    CPSS_DXCH_TAM_TRACKING_MODE_ENT   mode;     /** tracking mode */
    CPSS_DXCH_TAM_COUNTER_ID_STC      counter;  /** the base counter for the statistic */

} CPSS_DXCH_TAM_STATISTIC_STC;

/**
* @struct CPSS_DXCH_TAM_COUNTER_STATISTICS_STC
*
* @brief Counter Statistics data structure.
*/
typedef struct
{
    GT_U64  current;    /** current value */
    GT_U64  minimum;    /** minimum value */
    GT_U64  peak;       /** peak/maximum value */
    GT_U64  average;    /** average value */

} CPSS_DXCH_TAM_COUNTER_STATISTICS_STC;

/**
* @struct CPSS_DXCH_TAM_MICROBURST_STATISTICS_STC
*
* @brief Microburst Statistics data structure.
*/
typedef struct
{
    GT_U32  lastDuration;           /** last uBurst duration in us */
    GT_U32  shortestDuration;       /** shortest uBurst duration in us */
    GT_U32  longestDuration;        /** longest uBurst duration in us */
    GT_U32  averageDuration;        /** average uBurst duration in us */
    GT_U32  numMicrobursts;         /** number of uBursts */

} CPSS_DXCH_TAM_MICROBURST_STATISTICS_STC;

/**
* @struct CPSS_DXCH_TAM_HISTOGRAM_BIN_STC
*
* @brief Histogram bins configuration structure.
*/
typedef struct
{
    GT_U64  bin[CPSS_DXCH_TAM_HISTOGRAM_BIN_LEVELS];

} CPSS_DXCH_TAM_HISTOGRAM_BIN_STC;

/**
* @struct CPSS_DXCH_TAM_HISTOGRAM_BIN_TEMP_STC
*
* @brief Histogram bins configuration structure.
*/
typedef struct
{
    GT_U32  bin0;
    GT_U32  bin1;
    GT_U32  bin2;

} CPSS_DXCH_TAM_HISTOGRAM_BIN_TEMP_STC;

/**
* @struct CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC
*
* @brief Histogram statistics data structure.
*/
typedef struct
{
    GT_U32  histogramId;
    GT_U64  value[CPSS_DXCH_TAM_HISTOGRAM_BIN_VALUES];

} CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC;

/**
* @struct CPSS_DXCH_TAM_THRESHOLD_BREACH_EVENT_STC
*
* @brief Threshold Breach Event data.
*/
typedef struct
{
    GT_U32      thresholdId;    /** Threshold for the breach event */
    GT_U64      value;          /** Statistic value for the breach event */

} CPSS_DXCH_TAM_THRESHOLD_BREACH_EVENT_STC;

/**
* @internal cpssDxChTamInit function
* @endinternal
*
* @brief   init process for TAM feature.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                   - physical device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChTamInit
(
    IN   GT_U8                          devNum
);

/**
* @internal cpssDxChTamDebugControl function
* @endinternal
*
* @brief  enable/disable TAM statistics measurements by FW.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                   - physical device number.
* @param[in] startStopStatistics      - GT_TRUE  - enable statistics measurements
*                                       GT_FALSE - disable statistics measurements
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChTamDebugControl
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  startStopStatistics
);

/**
* @internal cpssDxChTamFwDebugCountersGet function
* @endinternal
*
* @brief  can be used to check if fw running ok.
*         if the counters value is incremented between calls it means the fw enabled
*         and statistics calculated.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[out] fwCounter            - this counter incremented if the tam task started.
* @param[out] fwStatCounter        - this counter incremented if tam statistics enabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTamFwDebugCountersGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *fwCounter,
    OUT GT_U32   *fwStatCounter
);

/**
* @internal cpssDxChTamSnapshotSet function
* @endinternal
*
* @brief  Create snaphot of measured resources for atomic operation.
*         Snapshot #0 has high priority.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] snapshotId            - snapshot id.
* @param[in] listSize              - number of counters in countersArray
*                                    0 - delete snapshot
* @param[in] countersArray         - array of counters.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_VALUE             - on invalid snapshotId
* @retval GT_NO_RESOURCE           - when listSize > freeCounters
* @retval GT_ALREADY_EXIST         - when snapshotId already exists
*                                    or When try to delete snapshot with existing histogram or microburst
*
* @note   Release restriction: CPSS_DXCH_TAM_MICROBURST_DURATION_COUNTER_E cannot be a snapshot counter.
*
*/
GT_STATUS cpssDxChTamSnapshotSet
(
    IN   GT_U8                          devNum,
    IN   GT_U32                         snapshotId,
    IN   GT_U32                         listSize,
    IN   CPSS_DXCH_TAM_COUNTER_ID_STC   countersArray[]
);

/**
* @internal cpssDxChTamSnapshotGet function
* @endinternal
*
* @brief  Get snapshot configuration.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] snapshotId            - snapshot id.
* @param[in,out] listSize          - in - countersPtr array size
*                                    out - number of counters for this snapshot
* @param[out] countersArray        - array of counters. Pointer to preallocated list (by the caller).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - if listSize==0
* @retval GT_BAD_VALUE             - on not valid snapshotId
* @retval GT_NO_SUCH               - if snapshot free
* @retval GT_BAD_SIZE              - if listSize < number of counters in this snapshot
*
* @note   Release restriction: CPSS_DXCH_TAM_MICROBURST_DURATION_COUNTER_E cannot be a snapshot counter.
*
*/
GT_STATUS cpssDxChTamSnapshotGet
(
    IN      GT_U8                          devNum,
    IN      GT_U32                         snapshotId,
    INOUT   GT_U32                         *listSize,
    OUT     CPSS_DXCH_TAM_COUNTER_ID_STC   countersArray[]
);

/**
* @internal cpssDxChTamSnapshotStatGet function
* @endinternal
*
* @brief  Get snaphot data of measured resources for atomic operation.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] snapshotId            - snapshot id.
* @param[in,out] numOfStats        - in - statsDataPtr array size
*                                    out - number of counters for this snapshot.
* @param[out] statsDataArray       - array of counters data. Pointer to preallocated list (by the caller).
* @param[in,out] numOfHistograms   - in - histValuePtr array size
*                                    out - number of histograms for this snapshot.
* @param[out] histValueArray       - array of histogram data. Pointer to preallocated list (by the caller).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - if numOfStats==0
* @retval GT_BAD_VALUE             - on wrong snapshotId
* @retval GT_NO_SUCH               - if snapshot free
* @retval GT_BAD_SIZE              - if numOfStats < number of counters in this snapshot
*
*/
GT_STATUS cpssDxChTamSnapshotStatGet
(
    IN     GT_U8                                devNum,
    IN     GT_U32                               snapshotId,
    INOUT  GT_U32                               *numOfStats,
    OUT    CPSS_DXCH_TAM_COUNTER_STATISTICS_STC statsDataArray[],
    INOUT  GT_U32                               *numOfHistograms,
    OUT    CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC    histValueArray[]
);

/**
* @internal cpssDxChTamMicroburstSet function
* @endinternal
*
* @brief  Create microburst. The microburst measurements will start after creation.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] microburstId          - microburst id.
* @param[in] tamCounterPtr         - Pointer to base counter for the microburst.
*                                    if null - delete the microburst
* @param[in] highWatermarkLevel    - Breach high watermark level for this counter.
* @param[in] lowWatermarkLevel     - Breach low watermark level for this counter.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note   Release restriction: we cannot create microburst on CPSS_DXCH_TAM_MICROBURST_DURATION_COUNTER_E.
*
*/
GT_STATUS cpssDxChTamMicroburstSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       microburstId,
    IN CPSS_DXCH_TAM_COUNTER_ID_STC *tamCounterPtr,
    IN GT_U32                       highWatermarkLevel,
    IN GT_U32                       lowWatermarkLevel
);

/**
* @internal cpssDxChTamMicroburstGet function
* @endinternal
*
* @brief  Get microburst configuration.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] microburstId          - microburst id.
* @param[out] tamCounterPtr        - Pointer to base counter for the microburst.
* @param[out] highWatermarkLevel   - Breach high watermark level for this counter.
* @param[out] lowWatermarkLevel    - Breach low watermark level for this counter.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChTamMicroburstGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          microburstId,
    OUT CPSS_DXCH_TAM_COUNTER_ID_STC    *tamCounterPtr,
    OUT GT_U32                          *highWatermarkLevel,
    OUT GT_U32                          *lowWatermarkLevel
);

/**
* @internal cpssDxChTamMicroburstStatGet function
* @endinternal
*
* @brief  Get microburst data of measured resources.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] microburstId          - snapshot id.
* @param[out] statsDataPtr         - microburst statistics data.
* @param[out] histValuePtr         - histogram statistics data.
*                                    NULL - if no histogram for this microburst.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChTamMicroburstStatGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  microburstId,
    OUT CPSS_DXCH_TAM_MICROBURST_STATISTICS_STC *statsDataPtr,
    OUT CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC       *histValuePtr
);

/**
* @internal cpssDxChTamHistogramSet function
* @endinternal
*
* @brief  Configure a histogram bins and clear values.  The histogram measurements will start after creation.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] histogramId           - histogram id.
* @param[in] counterPtr            - counter to calculate the histogram.
*                                    if null - delete the histogram
* @param[in] binsPtr               - histogram configuration structure. bin3 > bin2 > bin1 > 0.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note   If the counter type is: CPSS_DXCH_TAM_MICROBURST_DURATION_COUNTER_E -
*         The histogram counts microburst time values. (Microburst defined by cpssDxChTamMicroburstSePI).
*         In this case the histogram statistics need to be get by cpssDxChTamMicroburstStatGet API.
* @note   For other counter types - The histogram counts counter values.
*         (Counters that were defined in cpssDxChTamSnapshotSet API).
*         In this case the histogram statistics need to be get by cpssDxChTamSnapshotStatGet API.
*
*/
GT_STATUS cpssDxChTamHistogramSet
(
    IN GT_U8                            devNum,
    IN GT_U32                           histogramId,
    IN CPSS_DXCH_TAM_COUNTER_ID_STC     *counterPtr,
    IN CPSS_DXCH_TAM_HISTOGRAM_BIN_STC  *binsPtr
);


/**
* @internal cpssDxChTamHistogramSetTemp function
* @endinternal
*
* @brief  Temporary API to use in LUA instead of cpssDxChTamHistogramSet untill the problem with U64 will be resolved.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] histogramId           - histogram id.
* @param[in] counterPtr            - counter to calculate the histogram. NULL to disable.
* @param[in] binsPtr               - histogram configuration structure. bin[2] > bin[1] > bin[0] > 0.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChTamHistogramSetTemp
(
    IN GT_U8                            devNum,
    IN GT_U32                           histogramId,
    IN CPSS_DXCH_TAM_COUNTER_ID_STC     *counterPtr,
    IN CPSS_DXCH_TAM_HISTOGRAM_BIN_TEMP_STC  *binsPtr
);

/**
* @internal cpssDxChTamHistogramGet function
* @endinternal
*
* @brief  Get configuration of the histogram.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] histogramId           - histogram id.
* @param[out] counterPtr           - counter to calculate the histogram.
* @param[out] binsPtr              - histogram configuration structure.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChTamHistogramGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          histogramId,
    OUT CPSS_DXCH_TAM_COUNTER_ID_STC    *counterPtr,
    OUT CPSS_DXCH_TAM_HISTOGRAM_BIN_STC *binsPtr
);

/**
* @internal cpssDxChTamThresholdSet function
* @endinternal
*
* @brief  Create and configure a statistics treshold. Set Breach level for this
*         statistics. The statistic should be active. If specified, a threshold
*         breach event will be recorded when the buffer usage goes beyond the level.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] thresholdId           - threshold id.
* @param[in] statisticPtr          - the statistic to monitor.
*                                    if null - delete the histogram
* @param[in] thresholdLevel        - breach watermark level for this statistic
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChTamThresholdSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       thresholdId,
    IN CPSS_DXCH_TAM_STATISTIC_STC  *statisticPtr,
    IN GT_U64                       thresholdLevel
);

/**
* @internal cpssDxChTamThresholdGet function
* @endinternal
*
* @brief  Get threshold configuration.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] thresholdId           - threshold id.
* @param[out] statisticPtr         - the statistic to monitor that was defined at set API.
* @param[out] thresholdLevel       - breach watermark level for this statistic
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChTamThresholdGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      thresholdId,
    OUT CPSS_DXCH_TAM_STATISTIC_STC *statisticPtr,
    OUT GT_U64                      *thresholdLevel
);

/**
* @internal cpssDxChTamThresholdNotificationsGet function
* @endinternal
*
* @brief  Get list of thresholds that exedded its thresholdLevel.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in,out] numOfThresholds   - in - thresholdsArray size
*                                    out - number of thresholds.
* @param[out] thresholdsArray      - array of thresholds that were crossed its thresholdLevel. Pointer to preallocated list (by the caller).
*  
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChTamThresholdNotificationsGet
(
    IN    GT_U8                                    devNum,
    INOUT GT_U32                                   *numOfThresholds,
    OUT   CPSS_DXCH_TAM_THRESHOLD_BREACH_EVENT_STC thresholdsArray[]
);

/**
* @internal CPSS_DXCH_TAM_TRESHOLD_BREACH_EVENT_FUNC function
* @endinternal
*
* @brief  TAM event notification function.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] listSize              - Number of events.
* @param[in] data                  - Pointer to TAM events data array.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
typedef void (*CPSS_DXCH_TAM_TRESHOLD_BREACH_EVENT_FUNC) (
    IN GT_U8                                    devNum,
    IN GT_U32                                   listSize,
    IN CPSS_DXCH_TAM_THRESHOLD_BREACH_EVENT_STC thresholdArray[]
);

/**
* @internal cpssDxChTamEventNotifyBind function
* @endinternal
*
* @brief  Register a notify callback function.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] notifyFunc            - function pointer.

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChTamEventNotifyBind
(
    IN   GT_U8                                      devNum,
    IN   CPSS_DXCH_TAM_TRESHOLD_BREACH_EVENT_FUNC   *notifyFunc

);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTamh */
