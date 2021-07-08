/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtOsDebug.h
*
* @brief Operating System wrapper. Debug facility.
*
* @version   2
********************************************************************************
*/

#ifndef __gtOsDebugh
#define __gtOsDebugh

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>
#include <gtOs/gtOsTask.h>

#define OS_DEBUG_UTILIZATION_NOT_AVAILABLE_CNS  101

/**
* @struct OS_DEBUG_UTIL_MEAS_STC
 *
 * @brief Utilization measurement.
*/
typedef struct{

    /** utilization for last 5 seconds in percents. */
    GT_U32 utilize_5_sec;

    /** utilization for last minute in percents. */
    GT_U32 utilize_min;

    /** @brief utilization for last 5 minutes in percents.
     *  Comments:
     *  OS_DEBUG_UTILIZATION_NOT_AVAILABLE_CNS is used when utilization
     *  measurement is not available.
     */
    GT_U32 utilize_5_min;

} OS_DEBUG_UTIL_MEAS_STC;

/**
* @internal osDebugServicesInit function
* @endinternal
*
* @brief   Initializes the Debug Services.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osDebugServicesInit(GT_VOID);

/**
* @internal osDebugTaskUtilStart function
* @endinternal
*
* @brief   Starts task utilization measurement.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osDebugTaskUtilStart(GT_VOID);

/**
* @internal osDebugTaskUtilStop function
* @endinternal
*
* @brief   Stops task utilization measurement.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osDebugTaskUtilStop(GT_VOID);

/**
* @internal osDebugTaskUtilGet function
* @endinternal
*
* @brief   Gets task utilization measurements for given task.
*
* @param[in] taskId                   - ID of task whose measurement is to be given.
*
* @param[out] utilizationPtr           - (pointer to) utilization measurements.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osDebugTaskUtilGet
(
    IN GT_TASK  taskId,
    OUT OS_DEBUG_UTIL_MEAS_STC *utilizationPtr
);

/**
* @internal osDebugCpuUtilGet function
* @endinternal
*
* @brief   Gets CPU utilization measurement.
*
* @param[out] utilizationPtr           - (pointer to) utilization measurements.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osDebugCpuUtilGet
(
    OUT OS_DEBUG_UTIL_MEAS_STC *utilizationPtr
);

/**
* @internal osDebugFreeMemGet function
* @endinternal
*
* @brief   Gets the free memory in the system.
*
* @param[out] freeMemPtr               - (pointer to) free memory in Kbytes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osDebugFreeMemGet
(
    OUT GT_U32 *freeMemPtr
);

#ifdef __cplusplus
}
#endif

#endif  /* __gtOsDebugh */


