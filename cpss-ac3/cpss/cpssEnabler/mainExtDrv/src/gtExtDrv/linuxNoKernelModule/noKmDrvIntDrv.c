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
* @file noKmDrvIntDrv.c
*
* @brief Interrupt driver for linux based boards.
*
* @version   1
********************************************************************************
*/
#define _BSD_SOURCE
#define _DEFAULT_SOURCE

#include <gtExtDrv/drivers/gtPciDrv.h>
#include <gtOs/gtOsTask.h>
#include <gtOs/gtOsMem.h>
#include "prvNoKmDrv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

unsigned long long extDrvInterruptCount = 0;

#ifdef NOKM_DRV_EMULATE_INTERRUPTS
CPSS_HW_INFO_STC* prvExtDrvNokmGetMappingByIntNum(GT_U32 intNum);
#endif
struct intTaskParams_STC {
    GT_U32              intNum;
    GT_BOOL             enabled;
    GT_VOIDINTFUNCPTR   routine;
    GT_U32              param;
    GT_UINTPTR          regsBase;
    GT_TASK             tid;
};
#define MAX_INTERRUPT 32
static struct intTaskParams_STC intArr[MAX_INTERRUPT];
static GT_U32 intArrSize = 0;
static int mvIntDrvFd = -1;


/*******************************************************************************
* intTaskDrv
*
* DESCRIPTION:
*       Interrupt handler task (mvIntDrv used).
*
* INPUTS:
*       paramsPtr - index in intArr[]
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
static unsigned __TASKCONV intTaskDrv(GT_VOID *paramsPtr)
{
    GT_U32 i = (GT_U32)((GT_UINTPTR)paramsPtr);
    char  slot;

    slot = (char)(intArr[i].regsBase);

    while (1)
    {
        int ret;
        GT_VOIDINTFUNCPTR   routine;
        GT_U32              param;

        ret = read(mvIntDrvFd, NULL, slot);
        if (ret < 0)
        {
            if (errno == EINTR)
                continue;
            fprintf(stderr, "Interrupt wait failed: errno(%s)\n",
                            strerror(errno));
            return (int)GT_FAIL;
        }
        routine = intArr[i].routine;
        param = intArr[i].param;
        if (intArr[i].enabled == GT_TRUE)
        {
            routine(param);
        } else {
            /* intArr[i].enabled != GT_TRUE */
            usleep(50000); /* 500ms */
        }
    }
    return 0;
}
#ifdef NOKM_DRV_EMULATE_INTERRUPTS
/* the sleep time for polling mode - value in 'microseconds' */
static GT_U32 pollingSleepTime = 20000;/* 20ms */
void NOKM_DRV_EMULATE_INTERRUPTS_pollingSleepTime_Set(GT_U32 new_milisec_pollingSleepTime)
{
    if(new_milisec_pollingSleepTime == 0)
    {
        /* restore 'default' */
        pollingSleepTime = 20000;/* 20ms */
    }
    else
    {
        pollingSleepTime = 1000 * new_milisec_pollingSleepTime;
    }

    return;
}
/*******************************************************************************
* intTaskPool
*
* DESCRIPTION:
*       Interrupt handler task (polling mode).
*
* INPUTS:
*       paramsPtr - index in intArr[]
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
static unsigned __TASKCONV intTaskPoll(GT_VOID *paramsPtr)
{
    GT_U32 i = (GT_U32)((GT_UINTPTR)paramsPtr);

    while (1)
    {
        GT_VOIDINTFUNCPTR   routine;
        GT_U32              param;

        routine = intArr[i].routine;
        param = intArr[i].param;
        if (intArr[i].enabled == GT_TRUE)
        {
            /* the Interrupt Summary register is cheched within the "routine" */
            /* therfore no need to check its content in advanced.             */
            routine(param);
            usleep(pollingSleepTime); /* value in 'microseconds' */
        } else {
            /* intArr[i].enabled != GT_TRUE */
            usleep(50000); /* 50ms */
        }
    }
    return 0;
}
#endif /* NOKM_DRV_EMULATE_INTERRUPTS */

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
#ifdef MTS_BUILD    /* MTS patch */
	return GT_OK; /* ? osIntEnable(intVecNum); */
#else
    GT_U32 i;
    for (i = 0; i < intArrSize; i++)
    {
        if (intArr[i].intNum == intVecNum)
        {
            intArr[i].enabled = GT_TRUE;
            return GT_OK;
        }
    }
    return GT_FAIL;
#endif
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
#ifdef MTS_BUILD /* MTS patch */
	return GT_OK; /* ? osIntDisable(intVecNum); */
#else
    GT_U32 i;
    for (i = 0; i < intArrSize; i++)
    {
        if (intArr[i].intNum == intVecNum)
        {
            intArr[i].enabled = GT_FALSE;
            return GT_OK;
        }
    }
    return GT_FAIL;
#endif
}

/**
* @internal prvExtDrvMvIntDrvConnected function
* @endinternal
*
* @brief   Use mvIntDrv kernel module
*
* @retval GT_TRUE                  - if yes
* @retval GT_FALSE                 - if error
*/
GT_BOOL prvExtDrvMvIntDrvConnected(void)
{
    if (mvIntDrvFd == -1)
    {
        mvIntDrvFd = open("/dev/mvIntDrv", O_RDWR);
        if (mvIntDrvFd < 0)
        {
            mvIntDrvFd = -2; /* failed to open, no more try */
        }
        else
        {
            NOKMDRV_IPRINTF(("/dev/mvIntDrv opened, use interrupts from HW\n"));
        }
    }
    return (mvIntDrvFd < 0) ? GT_FALSE : GT_TRUE;
}

/**
* @internal prvExtDrvMvIntDrvEnableMsi function
* @endinternal
*
* @brief   Enable MSI interrupts for PCI device
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS prvExtDrvMvIntDrvEnableMsi(
    DBDF_DECL
)
{
    char cmd[6] = { 'm', (char)pciBus, (char)pciDev, (char)pciFunc, 0, 0 };

    if (mvIntDrvFd < 0)
        return GT_FAIL;

    if (pciDomain  == 0)
    {
        /* PCI domain = 0x0000 */
        return (write(mvIntDrvFd, cmd, 4) < 0) ? GT_FAIL : GT_OK;
    }
    cmd[0] = 'M';
    cmd[1] = pciDomain & 0xff;  /* domain LSB */
    cmd[2] = (pciDomain >> 8) & 0xff; /* domain MSB */
    cmd[3] = (char)pciBus;
    cmd[4] = (char)pciDev;
    cmd[5] = (char)pciFunc;
    return (write(mvIntDrvFd, cmd, 6) < 0) ? GT_FAIL : GT_OK;
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
    GT_CHAR taskName[40];
    GT_STATUS rc;
    GT_U32  i;

    if (intVec == 0)
    {
        NOKMDRV_IPRINTF(("extDrvIntDisconnect(): intVec==0 => BAD_PARAM\n"));
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

            if (prvExtDrvMvIntDrvConnected() == GT_TRUE && (intVec & 0xffff0000) != 0x11ab0000)
            {
                int slot;
                char buf[5];
                buf[0] = 'R';
                memcpy(buf+1, &intVec, 4);
                slot = (int)write(mvIntDrvFd, buf, 5);
                if (slot < 0)
                {
                    fprintf(stderr, "mvIntDrv:write returned %d\n", slot);
                    return GT_FAIL;
                }

                if(intArr[i].regsBase == slot+1) 
                { 
                    intArr[i].intNum = 0;
                    intArr[i].enabled = GT_FALSE;
                    intArr[i].routine = 0;
                    intArr[i].param = 0;
                }
            }

            return GT_OK;
        }
    }
    return GT_OK;
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
#ifdef MTS_BUILD    /* MTS patch */
    return osInterruptConnect(intVec, routine, parameter);
#else
    GT_CHAR taskName[40];
#ifdef NOKM_DRV_EMULATE_INTERRUPTS
    CPSS_HW_INFO_STC *hwInfoPtr;
#endif
    GT_STATUS rc;
    GT_U32  i;
    GT_U32 intArrIndex = intArrSize;

    if (intVec == 0)
    {
        NOKMDRV_IPRINTF(("extDrvIntConnect(): intVec==0 => BAD_PARAM\n"));
        return GT_BAD_PARAM;
    }
    /* search if already bound */
    for (i = 0; i < intArrSize; i++)
    {
        if (intArr[i].intNum == intVec)
        {
            GT_BOOL eSaved = intArr[i].enabled;
            intArr[i].enabled = GT_FALSE;
            intArr[i].param = parameter;
            intArr[i].routine = (GT_VOIDINTFUNCPTR)routine;
            intArr[i].enabled = eSaved;
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
    intArr[intArrIndex].enabled = GT_TRUE;
    intArr[intArrIndex].routine = (GT_VOIDINTFUNCPTR)routine;
    intArr[intArrIndex].param = parameter;

    if (prvExtDrvMvIntDrvConnected() == GT_TRUE && (intVec & 0xffff0000) != 0x11ab0000)
    {
        int slot;
        char buf[5];
        buf[0] = 'C';
        memcpy(buf+1, &intVec, 4);
        slot = (int)write(mvIntDrvFd, buf, 5);
        if (slot < 0)
        {
            fprintf(stderr, "mvIntDrv:write returned %d\n", slot);
            return GT_FAIL;
        }
        intArr[intArrIndex].regsBase = slot;
        if(intArrIndex == intArrSize)
            intArrSize++;

        sprintf(taskName,"intTask.%d", intVec);

        rc = osTaskCreate(taskName, 0/*prio*/, 0x2000/*stk size*/,
                intTaskDrv, (GT_VOID_PTR)((GT_UINTPTR)(intArrIndex)),
                &intArr[intArrIndex].tid);
        if (rc != GT_OK || 0 == intArr[intArrIndex].tid)
        {
            fprintf(stderr, "extDrvIntConnect: osTaskCreate(%s): %s\n", taskName, strerror(errno));
            return GT_FAIL;
        }
    }

#ifdef NOKM_DRV_EMULATE_INTERRUPTS
    else
    {
        hwInfoPtr = prvExtDrvNokmGetMappingByIntNum(intVec);
        if (hwInfoPtr == NULL)
            return GT_FAIL;

        intArr[intArrIndex].regsBase = hwInfoPtr->resource.switching.start;
        if(intArrIndex == intArrSize)
            intArrSize++;

        if (hwInfoPtr->busType != CPSS_HW_INFO_BUS_TYPE_MBUS_E)
        {
            sprintf(taskName, "intTask%02x:%02x:%d",
                    (intVec>>8) & 0xff, (intVec>>3) & 0x1f, intVec & 0x7);
            NOKMDRV_IPRINTF(("start irq polling task for PEX %02x:%02x.%d\n",
                    (intVec>>8) & 0xff, (intVec>>3) & 0x1f, intVec & 0x7));
        }
        else
        {
            strcpy(taskName, "intTaskff:ff:ff"); /* MSYS */
            NOKMDRV_IPRINTF(("start irq polling task for MBUS connected PP\n"));
        }

        rc = osTaskCreate(taskName, 0/*prio*/, 0x2000/*stk size*/,
                intTaskPoll, (GT_VOID_PTR)((GT_UINTPTR)(intArrIndex)),
                &intArr[intArrIndex].tid);
        if (rc != GT_OK || 0 == intArr[intArrIndex].tid)
        {
            fprintf(stderr, "extDrvIntConnect: osTaskCreate(%s): %s\n", taskName, strerror(errno));
            return GT_FAIL;
        }
    }
#endif  /* NOKM_DRV_EMULATE_INTERRUPTS */
#endif  /* LINUX - MTS patch*/
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




