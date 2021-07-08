#include <Copyright.h>

/**
********************************************************************************
* @file gtSem.h
*
* @brief Operating System wrapper
*
* @version   /
********************************************************************************
*/
/********************************************************************************
* gtSem.h
*
* DESCRIPTION:
*       Operating System wrapper
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*******************************************************************************/

#ifndef __prvCpssDrvGtSem_h
#define __prvCpssDrvGtSem_h

#include <msApiTypes.h>
#include <gtSysConfig.h>

#ifdef __cplusplus
extern "C" {
#endif

/***** Defines  ********************************************************/

#define PRV_CPSS_OS_WAIT_FOREVER             0

#define PRV_CPSS_OS_MAX_TASKS                30
#define PRV_CPSS_OS_MAX_TASK_NAME_LENGTH     10

#define PRV_CPSS_OS_MAX_QUEUES               30
#define PRV_CPSS_OS_MAX_QUEUE_NAME_LENGTH    10

#define PRV_CPSS_OS_MAX_EVENTS               10

#define PRV_CPSS_OS_MAX_SEMAPHORES           50

#define PRV_CPSS_OS_EOF                      (-1)



/**
* @internal prvCpssDrvGtSemCreate function
* @endinternal
*
* @brief   Create semaphore.
*
* @param[in] state                    - beginning  of the semaphore, either SEM_EMPTY or SEM_FULL
*                                       GT_CPSS_SEM if success. Otherwise, NULL
*/
GT_CPSS_SEM prvCpssDrvGtSemCreate
(
    IN  GT_CPSS_QD_DEV          *dev,
    IN  GT_CPSS_SEM_BEGIN_STATE state
);

/**
* @internal prvCpssDrvGtSemDelete function
* @endinternal
*
* @brief   Delete semaphore.
*
* @param[in] smid                     - semaphore Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGtSemDelete
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_CPSS_SEM       smid
);

/**
* @internal prvCpssDrvGtSemTake function
* @endinternal
*
* @brief   Wait for semaphore.
*
* @param[in] smid                     - semaphore Id
* @param[in] timeOut                  - time out in miliseconds or 0 to wait forever
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval OS_TIMEOUT               - on time out
*/
GT_STATUS prvCpssDrvGtSemTake
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_SEM     smid,
    IN  GT_U32          timeOut
);

/**
* @internal prvCpssDrvGtSemGive function
* @endinternal
*
* @brief   release the semaphore which was taken previously.
*
* @param[in] smid                     - semaphore Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGtSemGive
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_SEM     smid
);

#ifdef __cplusplus
}
#endif

#endif  /* __prvCpssDrvGtSem_h */
/* Do Not Add Anything Below This Line */

