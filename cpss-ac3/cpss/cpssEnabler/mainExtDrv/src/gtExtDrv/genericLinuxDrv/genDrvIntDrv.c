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
* @file genDrvIntDrv.c
*
* @brief Interrupt driver for linux uClinux based boards.
*
* @version   5
********************************************************************************
*/
#define _GNU_SOURCE

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
#ifdef  PRESTERA_SYSCALLS
#   include <linux/unistd.h>
#endif
#include <errno.h>
#include <string.h>

#include "kerneldrv/include/presteraGlob.h"

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
    mv_kmod_uintptr_t cookie;
    GT_U32 intNum;
    GT_VOIDINTFUNCPTR _userRoutine;
    GT_TASK             tid;
};

#define MAX_INTERRUPT 8
static struct intTaskParams_STC intArr[MAX_INTERRUPT];
static GT_U32 intArrSize = 0;

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
    /*
    GT_U32 i;
    for(i=0; i < intArrSize; i++)
    { 
        if(intArr[i].intNum == intVecNum)
        {
            if (0 == intArr[i].tid)
            {
                fprintf(stderr, "intEnable - tid(0): %s\n", strerror(errno));
                return GT_FAIL;
            }

            if (osTaskResume(intArr[i].tid))
            {
                fprintf(stderr, "osTaskResume: %s\n", strerror(errno));
                return GT_FAIL;
            }  
        }
    }*/ 

    /* Enable the IRQ  */
    /*if (ioctl (gtPpFd, PRESTERA_IOC_INTENABLE, (mv_kmod_uintptr_t)intVecNum))
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
    /*if (ioctl (gtPpFd, PRESTERA_IOC_INTDISABLE, (mv_kmod_uintptr_t)intVecNum))
    {
        fprintf(stderr, "Interrupt disabled failed: errno(%s)\n",
                        strerror(errno));
        return GT_FAIL;
    }*/
  /*  GT_U32 i;
    for(i=0; i < intArrSize; i++)
    { 
        if(intArr[i].intNum == intVecNum)
        {

            if (0 == intArr[i].tid)
            {
                fprintf(stderr, "intDisable - tid(0): %s\n", strerror(errno));
                return GT_FAIL;
            }

            if (osTaskSuspend(intArr[i].tid))
            {
                fprintf(stderr, "osTaskSuspend: %s\n", strerror(errno));
                return GT_FAIL;
            }
        }
    }
*/
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
    GT_U32 i = (GT_U32)((GT_UINTPTR)paramsPtr);
    GT_U32 devNum;
    mv_kmod_uintptr_t cookie;
    GT_VOIDINTFUNCPTR   routine;

    cookie =  intArr[i].cookie;
    devNum =  intArr[i].devNum;


    /* No need to wait because not interrupts in the driver until intEnable */
    while (1)
    {
        /* Wait for interrupt */
        if (prestera_ctl (PRESTERA_IOC_WAIT, cookie))
        {
            fprintf(stderr, "Interrupt wait failed: errno(%s)\n",
                    strerror(errno));
            return (int)GT_FAIL;
        }
        routine = intArr[i]._userRoutine;
        routine(devNum);
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
GT_STATUS extDrvIntConnect
(
    IN  GT_U32           intVec,
    IN  GT_VOIDFUNCPTR   routine,
    IN  GT_U32           parameter
)
{
    GT_CHAR taskName[32];
    GT_U32  i;
    GT_U32 intArrIndex = intArrSize;
    struct GT_VectorCookie_STC vector_cookie;

    if (intVec == 0)
    {
        fprintf(stderr, "extDrvIntConnect(): intVec==0 => BAD_PARAM\n");
        return GT_BAD_PARAM;
    }
    if (intVec > 127)
    {
        fprintf(stderr, "unknown interrupt vector: %ul\n", (int)intVec);
        return GT_FAIL;
    }
    /* search if already bound */
    for (i = 0; i < intArrSize; i++)
    {
        if (intArr[i].intNum == intVec)
        {
            intArr[i].devNum = parameter;
            return GT_OK;
           } else if(intArr[i].intNum == 0) {
            intArrIndex = i;
            break;  
        }
    }

    if (i == MAX_INTERRUPT)
    {
        fprintf(stderr, "extDrvIntConnect: too many interrupts already connected (%d)\n", intArrSize);
        return GT_FAIL;
    }
    intArr[intArrIndex].intNum = intVec;


    vector_cookie.vector = intVec;

    if (ioctl (gtPpFd, PRESTERA_IOC_INTCONNECT, &vector_cookie))
    {
        fprintf(stderr, "Interrupt connect failed: errno(%s)\n",
                        strerror(errno));
        return GT_FAIL;
    }
    
    if(intArrIndex == intArrSize)
            intArrSize++;
    intArr[intArrIndex]. _userRoutine = (GT_VOIDINTFUNCPTR)routine;
    sprintf(taskName, "intTask%d", intVec);

    intArr[intArrIndex].cookie = vector_cookie.cookie;
    intArr[intArrIndex].devNum = parameter;
    
    osTaskCreate(taskName, 0/*prio*/, 0x2000/*stk size*/,
                  intTask, (GT_VOID_PTR)((GT_UINTPTR)(intArrIndex)),
                   &intArr[intArrIndex].tid);

    if (0 == intArr[intArrIndex].tid)
    {
        fprintf(stderr, "extDrvIntConnect: osTaskCreate(%s): %s\n", taskName, strerror(errno));
        return GT_FAIL;
    }

    return  GT_OK;
}


/**
* @internal extDrvIntDisconnect function
* @endinternal
*
* @brief   Disconnect specified interrupt vector.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS extDrvIntDisconnect
(
    IN  GT_U32           intVec
)
{
    GT_CHAR taskName[32];
    GT_STATUS rc;
    GT_U32  i;
    struct GT_VectorCookie_STC vector_cookie;

    if (intVec == 0)
    {
        fprintf(stderr,"extDrvIntDisconnect(): intVec==0 => BAD_PARAM\n");
        return GT_BAD_PARAM;
    }
    /* search if already bound */
    for (i = 0; i < intArrSize; i++)
    {
        if (intArr[i].intNum == intVec)
        {
            sprintf(taskName,"intTask.%d", intVec);
            rc = osTaskDelete(intArr[i].tid);
            if (rc != GT_OK)
            {
                fprintf(stderr, "extDrvIntDisconnect: osTaskDelete(%s): %s\n", taskName, strerror(errno));
                return GT_FAIL;
            }

            vector_cookie.vector = intVec;

            if (ioctl (gtPpFd, PRESTERA_IOC_INTDISCONNECT, &vector_cookie))
            {
                fprintf(stderr, "Interrupt Disconnect failed: errno(%s)\n",
                        strerror(errno));
                return GT_FAIL;
            }

            intArr[i].intNum = 0;
            intArrSize--;
            return GT_OK;

        }
    }
    return GT_OK;
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




