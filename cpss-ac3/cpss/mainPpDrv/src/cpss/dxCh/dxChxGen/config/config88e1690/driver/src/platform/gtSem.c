#include <Copyright.h>
/**
********************************************************************************
* @file gtSem.c
*
* @brief Semaphore related routines
*
* @version   /
********************************************************************************
*/
/********************************************************************************
* gtSem.c
*
* DESCRIPTION:
*       Semaphore related routines
*
* DEPENDENCIES:
*       OS Dependent.
*
* FILE REVISION NUMBER:
*******************************************************************************/

#include <gtSem.h>


/**
* @internal prvCpssDrvGtSemCreate function
* @endinternal
*
* @brief   Create semaphore.
*
* @param[in] state                    - beginning  of the semaphore, either GT_CPSS_SEM_EMPTY or GT_CPSS_SEM_FULL
*                                       GT_CPSS_SEM if success. Otherwise, NULL
*/
GT_CPSS_SEM prvCpssDrvGtSemCreate
(
    IN GT_CPSS_QD_DEV           *dev,
    IN GT_CPSS_SEM_BEGIN_STATE  state
)
{
    if(dev->semCreate)
        return dev->semCreate(state);

    return 1; /* should return any value other than 0 to let it keep going */
}

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
)
{
    if((dev->semDelete) && (smid))
        return dev->semDelete(smid);

    return GT_OK;
}


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
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_CPSS_SEM       smid,
    IN  GT_U32            timeOut
)
{
    if(dev->semTake)
        return dev->semTake(smid, timeOut);

    return GT_OK;

}

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
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_CPSS_SEM       smid
)
{
    if(dev->semGive)
        return dev->semGive(smid);

    return GT_OK;
}


