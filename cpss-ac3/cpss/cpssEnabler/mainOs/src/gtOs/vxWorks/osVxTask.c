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
* @file osVxTask.c
*
* @brief VxWorks Operating System wrapper. Task facility implementation.
*
* @version   6
********************************************************************************
*/

#include <vxWorks.h>
#include <taskLib.h>
#include <string.h>

#include <gtOs/gtOsTask.h>
#include <gtOs/gtOsGen.h>


/************ Defines  ********************************************************/

#define VXW_DEF_STACK_SIZE  0x2000

#ifdef RTOS_ON_SIM

#include <asicSimulation/SCIB/scib.h>

#define PROTECT_TASK_DOING_READ_WRITE_START_MAC         SCIB_SEM_TAKE
#define PROTECT_TASK_DOING_READ_WRITE_END_MAC           SCIB_SEM_SIGNAL

GT_STATUS osTaskManagerRegister(
    IN GT_TASK      tid,
    IN const GT_CHAR  *namePtr
);
GT_STATUS osTaskManagerUnRegister(
    IN GT_TASK      tid
);
GT_STATUS osTaskManagerSuspendAllExcludeVip(
    GT_VOID
);
GT_STATUS osTaskManagerResumeAllExcludeVip(
    GT_VOID
);
GT_STATUS osTaskManagerSuspend(
    IN GT_TASK      tid
);
GT_STATUS osTaskManagerResume(
    IN GT_TASK      tid
);
#endif /*RTOS_ON_SIM*/


/************ Public Functions ************************************************/

/**
* @internal osTaskCreate function
* @endinternal
*
* @brief   Create OS Task and start it.
*
* @param[in] name                     - task name, string no longer then OS_MAX_TASK_NAME_LENGTH
* @param[in] prio                     - task priority 1 - 64 => HIGH
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
    IN  GT_U32  prio,
    IN  GT_U32  stack,
    IN  unsigned (__TASKCONV *start_addr)(GT_VOID*),
    IN  GT_VOID  *arglist,
    OUT GT_TASK  *tid
)
{
    GT_32 new_size;
    GT_CHAR name_copy[65];

    IS_WRAPPER_OPEN_STATUS;
    if (stack == 0)
      stack = VXW_DEF_STACK_SIZE;

    new_size = stack & ~0x01UL;

    strncpy(name_copy, name, 64);
    name_copy[64] = 0;

    *tid = taskSpawn (name_copy,
                      prio, 0,
                      new_size,
                      (FUNCPTR) start_addr,
                      (int) arglist, 0, 0, 0, 0, 0, 0, 0, 0, 0);

#ifdef RTOS_ON_SIM
    if ( (osTaskManagerRegister(*tid,name)) != GT_OK )
       return GT_FAIL;
#endif /*RTOS_ON_SIM*/

    return GT_OK;
}


/**
* @internal osTaskDelete function
* @endinternal
*
* @brief   Deletes existing task.
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
)
{
    STATUS rc;

    IS_WRAPPER_OPEN_STATUS;
    if (tid == 0)
        tid = taskIdSelf();

    rc = taskDelete(tid);

    if (rc != OK)
        return GT_FAIL;

#ifdef RTOS_ON_SIM
    if ( (osTaskManagerUnRegister(tid)) != GT_OK )
       return GT_FAIL;
#endif /*RTOS_ON_SIM*/

    return GT_OK;
}

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
)
{
    STATUS rc;

    IS_WRAPPER_OPEN_STATUS;
    if (tid == 0)
      tid = taskIdSelf();

    rc = taskSuspend(tid);

    if (rc != OK)
      return GT_FAIL;

#ifdef RTOS_ON_SIM
    if ((osTaskManagerSuspend(tid)) != GT_OK)
      return GT_FAIL;
#endif /*RTOS_ON_SIM*/

    return GT_OK;
}


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
)
{
    STATUS rc;

    IS_WRAPPER_OPEN_STATUS;
    rc = taskResume(tid);
    if (rc != OK)
      return GT_FAIL;

#ifdef RTOS_ON_SIM
    if ((osTaskManagerResume(tid)) != GT_OK)
      return GT_FAIL;
#endif /*RTOS_ON_SIM*/

    return GT_OK;
}


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
)
{
    IS_WRAPPER_OPEN_STATUS;
    /* check validity of function arguments */
    if (tid == NULL)
        return GT_FAIL;
    *tid = taskIdSelf();
    return GT_OK;
}


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
)
{
    STATUS status = OK;

    IS_WRAPPER_OPEN_STATUS;
    if (oldprio != NULL)
    {
        status = taskPriorityGet((int)tid,(int*)oldprio);
    }

    status = taskPrioritySet((int)tid, (int)newprio);

    if (status != OK)
        return GT_FAIL;

    return GT_OK;
}

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
)
{
    STATUS status;

    IS_WRAPPER_OPEN_STATUS;
    if (prio == NULL)
        return GT_FAIL;

    status = taskPriorityGet((int)tid,(int*)prio);

    if (status != OK)
        return GT_FAIL;

    return GT_OK;
}

/**
* @internal osTaskLock function
* @endinternal
*
* @brief   Disable task rescheduling of current task.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osTaskLock(GT_VOID)
{
    STATUS rc;

    IS_WRAPPER_OPEN_STATUS;

#ifndef RTOS_ON_SIM
    rc = taskLock();
    if ( rc != OK )
       return GT_FAIL;
#else /*RTOS_ON_SIM*/
    PROTECT_TASK_DOING_READ_WRITE_START_MAC;
    rc = osTaskManagerSuspendAllExcludeVip();
    /* the unlock of the 'protection' can be done only during 'tasks unlock'
      see osTaskUnLock */
    /*PROTECT_TASK_DOING_READ_WRITE_END_MAC;*/
    if ( rc != GT_OK )
       return GT_FAIL;
#endif /*RTOS_ON_SIM*/

    return GT_OK;
}


/**
* @internal osTaskUnLock function
* @endinternal
*
* @brief   Enable task rescheduling.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osTaskUnLock (GT_VOID)
{
    STATUS rc;

    IS_WRAPPER_OPEN_STATUS;
#ifndef RTOS_ON_SIM
    rc = taskUnlock();
    if ( rc != OK )
       return GT_FAIL;
#else /*RTOS_ON_SIM*/
    rc = osTaskManagerResumeAllExcludeVip();
    PROTECT_TASK_DOING_READ_WRITE_END_MAC;
    if ( rc != GT_OK )
       return GT_FAIL;
#endif /*RTOS_ON_SIM*/

    return GT_OK;
}




