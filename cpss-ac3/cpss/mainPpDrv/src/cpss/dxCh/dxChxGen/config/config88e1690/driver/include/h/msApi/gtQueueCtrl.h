#include <Copyright.h>
/**
********************************************************************************
* @file gtQueueCtrl.h
*
* @brief API/Structure definitions for Marvell Queue Control functionality.
*
* @version   /
********************************************************************************
*/
/*******************************************************************************
* gtQueueCtrl.h
*
* DESCRIPTION:
*       API/Structure definitions for Marvell Queue Control functionality.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*******************************************************************************/

#ifndef __prvCpssDrvGtQueueCtrl_h
#define __prvCpssDrvGtQueueCtrl_h

#include <msApiTypes.h>
#include <gtSysConfig.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/* Exported Port Control Types                                              */
/****************************************************************************/

/**
* @enum GT_CPSS_QUEUE_CFG
 *
 * @brief Enumeration for Queue configuration type
*/
typedef enum{

    /** 0x00: Port Schedule */
    GT_CPSS_QUEUE_CFG_PORT_SCHEDULE         = 0x00,

    /** 0x01: Filter QPri Enables */
    GT_CPSS_QUEUE_CFG_FILTER_Q_EN           = 0x01,

    /** 0x02 to 0x03: Per Port Scratch registers */
    GT_CPSS_QUEUE_CFG_PORT_SCRATCH_REG      = 0x02,

    /** 0x04: Dynamic Queue Pruning */
    GT_CPSS_QUEUE_CFG_DYNAMIC_Q_PRUNNING    = 0x04,

    /** 0x05: Dynamic Queue Status */
    GT_CPSS_QUEUE_CFG_DYNAMIC_Q_STATUS      = 0x05,

    /** 0x06: Dynamic Queue Yellow Control */
    GT_CPSS_QUEUE_CFG_DYNAMIC_Q_YELLOW_CTRL = 0x06,

    /** 0x10 to 0x17: Hard Queue Limit registers */
    GT_CPSS_QUEUE_CFG_H_Q_LIMIT_REG         = 0x10,

    /** 0x20 to 0x27: Yellow Queue Limit registers */
    GT_CPSS_QUEUE_CFG_Y_Q_LIMIT_REG         = 0x20,

    /** 0x30 to 0x3E: (Static) Begin PFC Thresholds */
    GT_CPSS_QUEUE_CFG_PFC_THRESHOLD_BEGIN   = 0x30,

    /** 0x31 to 0x3F: (Static) End PFC Thresholds */
    GT_CPSS_QUEUE_CFG_PFC_THRESHOLD_END     = 0x31,

    /** 0x40 to 0x47: (Virtual) Input Queue Counters */
    GT_CPSS_QUEUE_CFG_IN_Q_COUNT            = 0x40,

    /** 0x50: (Static) Begin FC Threshold */
    GT_CPSS_QUEUE_CFG_FC_THRESHOLD_BEGIN    = 0x50,

    /** 0x51: (Static) End FC Threshold */
    GT_CPSS_QUEUE_CFG_FC_THRESHOLD_END      = 0x51,

    /** 0x60 to 0x6F: Dynamic QoS Gain Settings (default = on) */
    GT_CPSS_QUEUE_CFG_QOS_GAIN_SET          = 0x60,

    /** 0x70 to 0x7F: Dyanmic PFC Gain Settings (default = off) */
    GT_CPSS_QUEUE_CFG_PFC_GAIN_SET          = 0x70

} GT_CPSS_QUEUE_CFG;


/**
* @enum GT_CPSS_PORT_SCHED_MODE
 *
 * @brief Enumeration of port scheduling mode
*/
typedef enum{

    /** @brief use 33,25,17,12,6,3,2,1 weighted fair
     *  scheduling
     */
    GT_CPSS_PORT_SCHED_WEIGHTED_RRB_1 = 0x0,

    /** @brief use a strict for priority 7 and weighted
     *  round robin for priorities 6,5,4,3,2,1 and 0
     */
    GT_CPSS_PORT_SCHED_STRICT_PRI7,

    /** @brief use a strict for priority 7,6 and weighted
     *  round robin for priorities 5,4,3,2,1 and 0
     */
    GT_CPSS_PORT_SCHED_STRICT_PRI7_6,

    /** @brief use a strict for priority 7,6,5 and weighted
     *  round robin for priorities 4,3,2,1 and 0
     */
    GT_CPSS_PORT_SCHED_STRICT_PRI7_6_5,

    /** @brief use a strict for priority 7,6,5,4 and
     *  weighted round robin for priorities 3,2,1 and 0
     */
    GT_CPSS_PORT_SCHED_STRICT_PRI7_6_5_4,

    /** @brief use a strict for priority 7,6,5,4,3 and
     *  weighted round robin for priorities 2,1 and 0
     */
    GT_CPSS_PORT_SCHED_STRICT_PRI7_6_5_4_3,

    /** @brief use a strict for priority 7,6,5,4,3,2
     *  and weighted round robin for priorities 1 and 0
     */
    GT_CPSS_PORT_SCHED_STRICT_PRI7_6_5_4_3_2,

    /** use a strict priority scheme for all priorities */
    GT_CPSS_PORT_SCHED_STRICT_PRI_1

} GT_CPSS_PORT_SCHED_MODE;

/****************************************************************************/
/* Exported Port Control Functions                                          */
/****************************************************************************/

/**
* @internal prvCpssDrvGprtSetQueueCtrl function
* @endinternal
*
* @brief   Set Queue control data to the Queue Control register.
*         The register(pointer) of Queue control are:
*         GT_CPSS_QUEUE_CFG_PORT_SCHEDULE
*         GT_CPSS_QUEUE_CFG_FILTER_Q_EN
*         GT_CPSS_QUEUE_CFG_PORT_SCRATCH_REG,   (acratch0 - 1 inc: 1)
*         GT_CPSS_QUEUE_CFG_H_Q_LIMIT_REG     (Q0 - Q7 inc: 1)
*         GT_CPSS_QUEUE_CFG_Y_Q_LIMIT_REG     (Q0 - Q7 inc: 1)
*         GT_CPSS_QUEUE_CFG_PFC_THRESHOLD_BEGIN  (Q0 - Q7 inc: 2)
*         GT_CPSS_QUEUE_CFG_PFC_THRESHOLD_END   (Q0 - Q7 inc: 2)
*         GT_CPSS_QUEUE_CFG_IN_Q_COUNT      (Q0 - Q7 inc: 1)
*         GT_CPSS_QUEUE_CFG_FC_THRESHOLD_BEGIN
*         GT_CPSS_QUEUE_CFG_FC_THRESHOLD_END
*         GT_CPSS_QUEUE_CFG_IN_PORT_COUNT
* @param[in] port                     - the logical  number
* @param[in] point                    - Pointer to the Queue control register.
* @param[in] data                     - Queue Control  written to the register
* @param[in] point                    to by the point above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtSetQueueCtrl
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_LPORT       port,
    IN  GT_U8               point,
    IN  GT_U8               data
);

/**
* @internal prvCpssDrvGprtGetQueueCtrl function
* @endinternal
*
* @brief   Get Queue control data from the Queue Control register.
*         The register(pointer) of Queue control are:
*         GT_CPSS_QUEUE_CFG_PORT_SCHEDULE
*         GT_CPSS_QUEUE_CFG_FILTER_Q_EN
*         GT_CPSS_QUEUE_CFG_PORT_SCRATCH_REG,   (acratch0 - 1 inc: 1)
*         GT_CPSS_QUEUE_CFG_H_Q_LIMIT_REG     (Q0 - Q7 inc: 1)
*         GT_CPSS_QUEUE_CFG_Y_Q_LIMIT_REG     (Q0 - Q7 inc: 1)
*         GT_CPSS_QUEUE_CFG_PFC_THRESHOLD_BEGIN  (Q0 - Q7 inc: 2)
*         GT_CPSS_QUEUE_CFG_PFC_THRESHOLD_END   (Q0 - Q7 inc: 2)
*         GT_CPSS_QUEUE_CFG_IN_Q_COUNT      (Q0 - Q7 inc: 1)
*         GT_CPSS_QUEUE_CFG_FC_THRESHOLD_BEGIN
*         GT_CPSS_QUEUE_CFG_FC_THRESHOLD_END
*         GT_CPSS_QUEUE_CFG_IN_PORT_COUNT
* @param[in] port                     - the logical  number
* @param[in] point                    - Pointer to the Queue control register.
*
* @param[out] data                     - Queue Control  written to the register
* @param[out] point                    to by the point above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetQueueCtrl
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_LPORT       port,
    IN  GT_U8               point,
    OUT GT_U8               *data
);

/**
* @internal prvCpssDrvGprtSetPortSched function
* @endinternal
*
* @brief   This routine sets Port Scheduling Mode.
*         When usePortSched is enablied, this mode is used to select the Queue
*         controller's scheduling on the port as follows:
*         GT_CPSS_PORT_SCHED_WEIGHTED_RRB_1 - use 33,25,17,12,6,3,2,1 weighted fair
*         scheduling
*         GT_CPSS_PORT_SCHED_STRICT_PRI_1 - use a strict priority scheme
* @param[in] port                     - the logical  number
* @param[in] mode                     - GT_CPSS_PORT_SCHED_MODE enum type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtSetPortSched
(
    IN  GT_CPSS_QD_DEV              *dev,
    IN  GT_CPSS_LPORT               port,
    IN  GT_CPSS_PORT_SCHED_MODE     mode
);

/**
* @internal prvCpssDrvGprtGetPortSched function
* @endinternal
*
* @brief   This routine gets Port Scheduling Mode.
*         When usePortSched is enablied, this mode is used to select the Queue
*         controller's scheduling on the port as follows:
*         GT_CPSS_PORT_SCHED_WEIGHTED_RRB_1 - use 33,25,17,12,6,3,2,1 weighted fair
*         scheduling
*         GT_CPSS_PORT_SCHED_STRICT_PRI_1 - use a strict priority scheme
* @param[in] port                     - the logical  number
*
* @param[out] mode                     - GT_CPSS_PORT_SCHED_MODE enum type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetPortSched
(
    IN  GT_CPSS_QD_DEV              *dev,
    IN  GT_CPSS_LPORT               port,
    OUT GT_CPSS_PORT_SCHED_MODE     *mode
);

#ifdef __cplusplus
}
#endif

#endif /* __prvCpssDrvGtQueueCtrl_h */

