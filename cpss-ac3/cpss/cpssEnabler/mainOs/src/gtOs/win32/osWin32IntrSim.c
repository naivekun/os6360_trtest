/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/


#include <gtOs/gtOsIntr.h>
#include <gtOs/gtOsTask.h>
#include <gtOs/gtOsSem.h>
#include <gtOs/gtOsIo.h>

/* ROS definitions and SHOST API */
#include <os/simTypes.h>
#include <common/SHOST/HOST_D/EXP/HOST_D.H>

#include <asicSimulation/SInit/sinit.h>
#include <asicSimulation/SDistributed/sdistributed.h>
#include <cpssCommon/cpssPresteraDefs.h>


/* define macro that WA the numbering issue in  the SHOST */
#define WA_SHOST_NUMBERING(_vector) ((_vector) + 1)

#include <asicSimulation/SCIB/scib.h>

/* in VXWORKS the ISR is locking all the regular tasks */
#define WIN32_ISR_IN_VXWORKS_STYLE  1
/* in LINUX the DSR is regular task and not locking other tasks */
#define WIN32_ISR_IN_LINUX_STYLE    2

/* since our systems are 'linux' ... work in linux style .
    NOTE: the AC3X must have it ... otherwise dead locks.
*/
#define WIN32_STYLE     WIN32_ISR_IN_LINUX_STYLE


/*#define PROTECT_TASK_DOING_READ_WRITE_START_MAC         SCIB_SEM_TAKE done in the SHOST */

/*#define PROTECT_TASK_DOING_READ_WRITE_END_MAC           SCIB_SEM_SIGNAL  done in the SHOST*/

/****** Types ************************/
typedef struct {
    GT_VOIDFUNCPTR isrCb;
    GT_U32         param;
    GT_U32         isConnected;/* indication that is connected */
}OS_ISR_INFO_STC;

/******Local Variables ************************/
static GT_U32 osInterGo = 0;

#if WIN32_STYLE   ==  WIN32_ISR_IN_LINUX_STYLE
static GT_MUTEX IntLockmutex;
/* function that suspend and resume 'application tasks' (and SLANs) in the SHOST */
extern void t_preemmption(GT_U32 type,GT_U32 *old_ptr);
enum{
    T_PREEMMPTION_LOCK_E = 0,
    T_PREEMMPTION_UNLOCK_E = 1
};


/* The interrupt locking is needed for mutual exclusion prevention between   */
/* ISR and user task on the event queues.                                   */
#define INT_LOCK(key)                                                   \
    key = 0;                                                            \
    PRV_CPSS_LOCK_BY_MUTEX;                                             \
    osSetIntLockUnlock(INTR_MODE_LOCK, &(key))

#define INT_UNLOCK(key)                                                 \
    osSetIntLockUnlock(INTR_MODE_UNLOCK, &(key));                   \
    PRV_CPSS_UNLOCK_BY_MUTEX


#endif

static OS_ISR_INFO_STC    isrInfo[MAX_UINT_8];

/***** Public Functions ************************************************/
/**
* @internal osIntEnable function
* @endinternal
*
* @brief   Enable corresponding interrupt bits
*
* @param[in] intVecNum                - new interrupt bits
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osIntEnable
(
    IN GT_U32   intVecNum
)
{
    SHOSTG_interrupt_enable_one(WA_SHOST_NUMBERING(intVecNum));

    return GT_OK;
}

/**
* @internal osIntDisable function
* @endinternal
*
* @brief   Disable corresponding interrupt bits.
*
* @param[in] intVecNum                - new interrupt bits
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osIntDisable
(
    IN GT_U32   intVecNum
)
{
    SHOSTG_interrupt_disable_one(WA_SHOST_NUMBERING(intVecNum));

    return GT_OK;
}

#if WIN32_STYLE   ==  WIN32_ISR_IN_VXWORKS_STYLE
/*******************************************************************************
* osSetIntLockUnlock
*
* DESCRIPTION:
*       Lock/unlock interrupts
*
* INPUTS:
*       mode   - interrupt state lock/unlock
*       key    - if mode is INTR_MODE_UNLOCK, lock key returned by
*                preceding interrupt disable call
*
* OUTPUTS:
*       key    - if mode is INTR_MODE_LOCK lock key for the interrupt
*                level
*
* RETURNS:
*       Lock key for the interrupt level
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_32 osSetIntLockUnlock
(
    IN      INTERRUPT_MODE mode,
    INOUT   GT_32          *key
)
{
    static INTERRUPT_MODE currMode = INTR_MODE_UNLOCK;
    INTERRUPT_MODE temp;

    temp = currMode;
    currMode = mode;

    if (mode == INTR_MODE_UNLOCK)
    {
        if (osInterGo == 0)
        {
            SHOSTG_go_intr_go ();
            osInterGo = 1;
        }

        SHOSTG_interrupt_enable(*key);
    }
    else
    {
/*        PROTECT_TASK_DOING_READ_WRITE_START_MAC; done in the SHOST */

        temp = SHOSTG_interrupt_disable();

/*        PROTECT_TASK_DOING_READ_WRITE_END_MAC; done in the SHOST */

        *key = temp;
    }

    return temp;
}
#elif WIN32_STYLE   ==  WIN32_ISR_IN_LINUX_STYLE
/**
* @internal osSetIntLockUnlock function
* @endinternal
*
* @brief   Lock/unlock interrupts
*
* @param[in] mode                     - interrupt state lock/unlock
* @param[in,out] key                      - if mode is INTR_MODE_UNLOCK, lock  returned by
*                                      preceding interrupt disable call
* @param[in,out] key                      - if mode is INTR_MODE_LOCK lock  for the interrupt
*                                      level
*                                       Lock key for the interrupt level
*
* @note implementation copied from same function for 'LINUX_SIM' from file:
*       cpssEnabler\mainOs\src\gtOs\linux\osLinuxIntr.c
*
*/
GT_32 osSetIntLockUnlock
(
    IN      INTERRUPT_MODE mode,
    INOUT   GT_32          *key
)
{
    GT_32 retValue = 0;
    static GT_U32 numOfNotInitLockings = 0;
    (void)key;

    if (mode == INTR_MODE_LOCK)
    {
        if(IntLockmutex)
        {
            osMutexLock(IntLockmutex);
        }
        else
        {
            numOfNotInitLockings++;
        }
    }
    else
    {
        if(IntLockmutex)
        {
            osMutexUnlock(IntLockmutex);
        }
        else
        {
            numOfNotInitLockings--;
        }
    }

    return retValue;
}
#endif

/**
* @internal osIsr function
* @endinternal
*
* @brief   lock all other tasks when ISR is invoked
*/
static GT_VOID osIsr(GT_U32  vector)
{
#if WIN32_STYLE   ==  WIN32_ISR_IN_LINUX_STYLE
    GT_32     intKey;          /* The interrupt lock key       */
#endif
   /* osTaskLock(); Done in SHOST */

    if(sasicgSimulationRole == SASICG_SIMULATION_ROLE_DISTRIBUTED_APPLICATION_SIDE_VIA_BROKER_E)
    {
        if(sdistAppViaBrokerInterruptMaskMode ==
           INTERRUPT_MODE_BROKER_AUTOMATICALLY_MASK_INTERRUPT_LINE)
        {
            /* the broker put itself into 'masked' mode --> so we need to release it */
            simDistributedInterruptUnmask(vector);/* note no need to use macro WA_SHOST_NUMBERING on this vector ! */

            /* we took the 'Easy way' , and set 'unmask' before reading the PP
               cause register .

               because if we do it after calling the isrCb , we will need much
               more complex mechanism , to avoid lose of interrupts !
            */
        }
    }

    #if WIN32_STYLE   ==  WIN32_ISR_IN_LINUX_STYLE
        /* unlock the tasks that done by the caller() */
        t_preemmption(T_PREEMMPTION_UNLOCK_E,NULL);

        /* do same sequence as regular task that need to be protected from ISR and other tasks */
        INT_LOCK(intKey);
    #endif

    if(isrInfo[vector].isConnected)
    {
        ((GT_VOIDINTFUNCPTR)(isrInfo[vector].isrCb))(isrInfo[vector].param);
    }

    #if WIN32_STYLE   ==  WIN32_ISR_IN_LINUX_STYLE
        INT_UNLOCK(intKey);

        /* lock the tasks so the caller can free it */
        t_preemmption(T_PREEMMPTION_LOCK_E,NULL);
    #endif

  /*  osTaskUnLock(); Done in SHOST */
}

/**
* @internal osInterruptConnect function
* @endinternal
*
* @brief   Connect a specified C routine to a specified interrupt vector.
*
* @param[in] vector                   - interrupt  number to attach to
* @param[in] routine                  -  to be called
* @param[in] parameter                -  to be passed to routine
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osInterruptConnect
(
    IN  GT_U32           vector,
    IN  GT_VOIDFUNCPTR   routine,
    IN  GT_U32           parameter
)
{
    if (vector >= MAX_UINT_8)
    {
        return GT_FAIL;
    }

#if WIN32_STYLE   ==  WIN32_ISR_IN_LINUX_STYLE
    {
        GT_STATUS   rc;
        char   semName[50];
        if(IntLockmutex == 0)
        {
            /* create semaphores */
            osSprintf(semName, "IntLockmutex");

            /* create 'reentrant' mutex ! */
            rc = osMutexCreate(semName, &IntLockmutex);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }
#endif /*WIN32_STYLE*/

    if(isrInfo[vector].isConnected)
    {
        /* already bound ... the SHOST not support 'unbind' */
        return GT_OK;
    }
    /* update DB */
    isrInfo[vector].isrCb = routine ;
    isrInfo[vector].param = parameter;
    isrInfo[vector].isConnected = 1;

    /* call SHOST to bind ISR with simulated interrupts engine */
    SHOSTG_bind_interrupt_with_param ( 15, WA_SHOST_NUMBERING(vector),
                                       osIsr, NULL, vector);
    return GT_OK;
}

/**
* @internal osInterruptDisconnect function
* @endinternal
*
* @brief   DisConnect a specified interrupt vector.
*
* @param[in] vector                - interrupt number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osInterruptDisconnect
(
    IN  GT_U32      vector
)
{
    if (vector >= MAX_UINT_8)
    {
        return GT_FAIL;
    }

    /* the SHOST not support 'unbind' (there is no implementation of SHOSTG_unbind_interrupt_with_param)*/
    /*isrInfo[vector].isConnected = 0;*/

    return GT_OK;
}
