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
* @file gtOsTask.h
*
* @brief Operating System wrapper. Task facility.
*
* @version   13
********************************************************************************
*/

#ifndef __gtOsTaskh
#define __gtOsTaskh

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>

/************ Defines  ********************************************************/
#ifdef WIN32
#define __TASKCONV
#else
#define __TASKCONV
#endif

/************* Typedefs *******************************************************/
#if (!defined __cmdExtServices_h_) || (defined PSS_PRODUCT)
typedef GT_U32  GT_TASK;
#endif 
/************* Functions ******************************************************/

/**
* @internal osTaskCreate function
* @endinternal
*
* @brief   Create OS Task/Thread and start it.
*
* @param[in] name                     - task name, string no longer then OS_MAX_TASK_NAME_LENGTH
* @param[in] prio                     - task priority 255 - 0 => HIGH
* @param[in] stack                    - task Stack Size
*                                      start_addr - task Function to execute
*                                      arglist    - pointer to list of parameters for task function
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osTaskCreate
(
    IN  const GT_CHAR *name,
    IN  GT_U32      prio,
    IN  GT_U32      stack,
    IN  unsigned    (__TASKCONV *start_addr)(GT_VOID*),
    IN  GT_VOID     *arglist,
    OUT GT_TASK     *tid
);

/**
* @internal osTaskDelete function
* @endinternal
*
* @brief   Deletes existing task/thread.
*
* @param[in] tid                      - Task ID
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note If tid = 0, delete calling task (itself)
*
*/
GT_STATUS osTaskDelete
(
    IN GT_TASK tid
);

/**
* @internal osTaskSuspend function
* @endinternal
*
* @brief   Suspends existing task/thread.
*
* @param[in] tid                      - Task ID
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note If tid = 0, suspend calling task (itself)
*
*/
GT_STATUS osTaskSuspend
(
    IN GT_TASK tid
);

/**
* @internal osTaskResume function
* @endinternal
*
* @brief   Resumes existing task/thread.
*
* @param[in] tid                      - Task ID
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osTaskResume
(
    IN GT_TASK tid
);

/**
* @internal osTaskGetSelf function
* @endinternal
*
* @brief   returns the current task (thread) id
*
* @param[out] tid                      -  the current task (thread) id
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - if parameter is invalid
*/
GT_STATUS osTaskGetSelf
(
    OUT GT_U32 *tid
);

/**
* @internal osSetTaskPrior function
* @endinternal
*
* @brief   Changes priority of task/thread.
*
* @param[in] tid                      - Task ID
* @param[in] newprio                  - new priority of task
*
* @param[out] oldprio                  - old priority of task
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note If tid = 0, change priotity of calling task (itself)
*
*/
GT_STATUS osSetTaskPrior
(
    IN  GT_TASK tid,
    IN  GT_U32  newprio,
    OUT GT_U32  *oldprio
);

/**
* @internal osGetTaskPrior function
* @endinternal
*
* @brief   Gets priority of task/thread.
*
* @param[in] tid                      - Task ID
*
* @param[out] prio                     - priority of task
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note If tid = 0, gets priotity of calling task (itself)
*
*/
GT_STATUS osGetTaskPrior
(
    IN  GT_TASK tid,
    OUT GT_U32  *prio
);

/**
* @internal CPSS_osTaskLock function
* @endinternal
*
* @brief   Disable task rescheduling of current task.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
#define osTaskLock CPSS_osTaskLock
GT_STATUS CPSS_osTaskLock (GT_VOID);

/**
* @internal CPSS_osTaskUnLock function
* @endinternal
*
* @brief   Enable task rescheduling.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/

#define osTaskUnLock CPSS_osTaskUnLock
GT_STATUS CPSS_osTaskUnLock (GT_VOID);

#ifdef __cplusplus
}
#endif

#endif  /* __gtOsTaskh */
/* Do Not Add Anything Below This Line */



