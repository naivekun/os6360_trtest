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
* @file gt85xxIntDrv.c
*
* @brief Interrupt driver
*
* @version   2
********************************************************************************
*/
#include <gtOs/gtOsGen.h>
#include <gtOs/gtOsIntr.h>
#include <gtOs/gtOsTask.h>
#include <gtOs/gtOsMem.h>



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#include "kerneldrv/include/prestera_glob.h"

#define IN
#define OUT

/******************************************************************************
* struct intTaskParams_STC
* 
* DESCRIPTION:
*       This structure is used to pass parameters to intTask()
*******************************************************************************/
struct intTaskParams_STC {
    GT_U32 devNum;
    GT_U32 cookie;
};

static GT_U32 tid;
static GT_VOIDINTFUNCPTR _userRoutine;

/*************** Globals ******************************************************/

/* file descriptor returnd by openning the PP *nix device driver */
extern GT_32 gtPpFd;


/**
* @internal extDrvIntEnable function
* @endinternal
*
* @brief   Enable corresponding interrupt bits
*
* @param[in] intVecNum                - new interrupt bits
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS extDrvIntEnable
(
    IN GT_U32   intVecNum
)
{
    if (0 == tid)
    {
        fprintf(stderr, "intEnable - tid(0): %s\n", strerror(errno));
        return GT_FAIL;
    }

    if (osTaskResume(tid))
    {
        fprintf(stderr, "osTaskResume: %s\n", strerror(errno));
        return GT_FAIL;
    }

    /* Enable the IRQ  */
    /*if (ioctl (gtPpFd, PRESTERA_IOC_INTENABLE, intVecNum))
    {
        fprintf(stderr, "Interrupt enabled failed: errno(%s)\n",
                        strerror(errno));
        return GT_FAIL;
    }*/

    return GT_OK ;
}


/**
* @internal extDrvIntDisable function
* @endinternal
*
* @brief   Disable corresponding interrupt bits.
*
* @param[in] intVecNum                - new interrupt bits
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS extDrvIntDisable
(
    IN GT_U32   intVecNum
)
{
    /* Disable the irq */
    /*if (ioctl (gtPpFd, PRESTERA_IOC_INTDISABLE, intVecNum))
    {
        fprintf(stderr, "Interrupt disabled failed: errno(%s)\n",
                        strerror(errno));
        return GT_FAIL;
    }*/

    if (0 == tid)
    {
        fprintf(stderr, "intDisable - tid(0): %s\n", strerror(errno));
        return GT_FAIL;
    }

    if (osTaskSuspend(tid))
    {
        fprintf(stderr, "osTaskSuspend: %s\n", strerror(errno));
        return GT_FAIL;
    }

    return GT_OK ;
}

/*******************************************************************************
* intTask
*
* DESCRIPTION:
*       Interrupt handler task.
*
* INPUTS:
*       param1  - device number
*       param2  - ISR cookie
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
unsigned __TASKCONV intTask(GT_VOID *paramsPtr)
{
    GT_U32 	devNum;
    GT_U32	cookie;	

    cookie = ((struct intTaskParams_STC*)paramsPtr)->cookie;
     
    devNum = ((struct intTaskParams_STC*)paramsPtr)->devNum;

    osFree(paramsPtr);	

    /* No need to wait because not interrupts in the driver until intEnable */
    while (1)
    {
        /* Wait for interrupt */
        if (ioctl (gtPpFd, PRESTERA_IOC_WAIT, &cookie))
        {
            fprintf(stderr, "Interrupt wait failed: errno(%s)\n",
                            strerror(errno));
            return (int)GT_FAIL;
        }

        osTaskLock();

        _userRoutine(devNum);

        osTaskUnLock();
    }

    return 0;
}

/**
* @internal extDrvIntConnect function
* @endinternal
*
* @brief   Connect a specified C routine to a specified interrupt vector.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_U32 extDrvIntConnect
(
    IN  GT_U32           intVec,
    IN  GT_VOIDFUNCPTR   routine,
    IN  GT_U32           parameter
)
{
    static GT_U32 intialized[2] = { 0, 0 };
    GT_U8 taskName[32];
    GT_VecotrCookie_STC vector_cookie;
    struct intTaskParams_STC* paramsPtr;    

    if (intVec > 63)
    {
        fprintf(stderr, "unknown interrupt vector: %ul\n", (int)intVec);
        return GT_FAIL;
    }
    if (intialized[intVec >> 5] & (1 << (intVec & 0x1F)))
    {
        fprintf(stderr, "intialized: %s\n", strerror(errno));
        return GT_FAIL;
    }
    intialized[intVec >> 5] |= (1 << (intVec & 0x1F));

    vector_cookie.vector = intVec;

    if (ioctl (gtPpFd, PRESTERA_IOC_INTCONNECT, &vector_cookie))
    {
        fprintf(stderr, "Interrupt connect failed: errno(%s)\n",
                        strerror(errno));
        return GT_FAIL;
    }

    _userRoutine = (GT_VOIDINTFUNCPTR)routine;

    sprintf((char*)taskName, "intTask%d", intVec);

    paramsPtr = (struct intTaskParams_STC*)osMalloc(sizeof(*paramsPtr));
    paramsPtr->cookie = vector_cookie.cookie;
    paramsPtr->devNum = parameter;

    osTaskCreate((char*)taskName, 0/*prio*/, 0x2000/*stk size*/,
				 intTask, paramsPtr, &tid);

    if (0 == tid)
    {
        osFree(paramsPtr);
        fprintf(stderr, "extDrvIntConnect: osTaskCreate(%s): %s\n", taskName, strerror(errno));
        return GT_FAIL;
    }

    return  GT_OK;
}


/**
* @internal extDrvSetIntLockUnlock function
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
*/
GT_32 extDrvSetIntLockUnlock
(
    IN      INTERRUPT_MODE mode,
    INOUT   GT_32          *key
)
{
    return GT_OK;
}




