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
* @file genDrvDsrIntDrv.c
*
* @brief Includes interrupt managment routines.
*
* @version   1.1.2.1
********************************************************************************
*/

#include <gtExtDrv/drivers/gtIntDrv.h>
#include <gtExtDrv/drivers/pssBspApis.h>
#include <prestera/common/gtPresteraDefs.h>


typedef GT_VOID (*USER_DSR_PTR)(IN GT_U8 intVecMapIdx);

typedef struct
{
    GT_U32          intVecNum;
    GT_U8           intVecMapIdx;
    USER_DSR_PTR    routine;
} ISR_DSR_PARAMS; 

typedef struct intVec
{
    ISR_DSR_PARAMS  *isrDsrPrms;
    struct intVec   *nextPtr;
} INT_VEC;


static GT_TASK          dsrTid = 0;
static ISR_DSR_PARAMS   *dsrPrmsPtr[MAX_PP_DEVICES];
static GT_SEM           intLockSem = NULL;
static GT_SEM           dsrSem = NULL;

#define FIFO_SIZE       ((MAX_PP_DEVICES * 2) + 1)
static INT_VEC          intVecFifo[FIFO_SIZE];
static INT_VEC          *fifoHeadPtr = NULL;
static INT_VEC          *fifoTailPtr = NULL;


/**
* @internal extDrvIntIsr function
* @endinternal
*
* @brief   This is the main interrupt service routine bound to the interrupt vector.
*
* @param[in] parameter                - the ISR/DSR  structure pointer including the
*                                      interrupt vector number and the DSR signalling semaphore.
*                                       None.
*
* @note Invoked in ISR context !!
*
*/
static GT_VOID extDrvIntIsr
(
    IN  GT_U32      parameter
)
{
    ISR_DSR_PARAMS  *isrParamsPtr = (ISR_DSR_PARAMS*)parameter;
    
    /* disable the interrupt vector */
    osIntDisable(isrParamsPtr->intVecNum);
    
    fifoTailPtr->isrDsrPrms = isrParamsPtr;
    fifoTailPtr = fifoTailPtr->nextPtr;
    
    /* signal the DSR */
    osSemSignal(dsrSem);
}
    

/*******************************************************************************
* extDrvDsr
*
* DESCRIPTION:
*       This is the main Deferred Service Routine, from which the other
*       interrupt SRs are called according to the devices types.
*
* INPUTS:
*       param   - the ISR/DSR parameter structure pointer including the
*                 interrupt vector number and the DSR signalling semaphore.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
* COMMENTS:
*       Assumption: The bounded routine in the intConnect uses 
*                   osSetIntLockUnlock for preemption protection between DSR 
*                   and other tasks.
*
*******************************************************************************/
static unsigned __TASKCONV extDrvDsr
(
    IN GT_VOID * notUsed
)
{
    ISR_DSR_PARAMS  *dsrParamsPtr;
    
    while (1)
    {
        /* wait for the Isr signal */
        osSemWait(dsrSem, OS_WAIT_FOREVER);

        while (fifoHeadPtr != fifoTailPtr)
        {
            dsrParamsPtr = fifoHeadPtr->isrDsrPrms;
            fifoHeadPtr = fifoHeadPtr->nextPtr;

            /* invoke the bounded routine */
            (*(dsrParamsPtr->routine))(dsrParamsPtr->intVecMapIdx);
            
            /* enable the interrupt vector */
            osIntEnable(dsrParamsPtr->intVecNum);
        }
    }
}


/**
* @internal extDrvIntConnect function
* @endinternal
*
* @brief   Connect a specified C routine to a specified interrupt vector.
*
* @param[in] intVecNum                - interrupt vector number to attach to
* @param[in] routine                  -  to be called
* @param[in] parameter                -  to be passed to routine
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS extDrvIntConnect
(
    IN  GT_U32           intVecNum,
    IN  GT_VOIDFUNCPTR   routine,
    IN  GT_U32           parameter
)
{
    ISR_DSR_PARAMS  *isrDsrParamsPtr;
    GT_U32          i;
    static GT_BOOL  firstTime = GT_TRUE;
    
    if (GT_TRUE == firstTime)
    {
        /* create semaphore for interrupt lock unlock routines */
        if(osSemBinCreate("intLock", OS_SEMB_FULL, &intLockSem) != GT_OK)
        {
            return GT_FAIL;
        }
        
        /* clean the dsr task id table */
        osMemSet(dsrPrmsPtr, 0, sizeof(dsrPrmsPtr));
        
        /* create the interrupt vector FIFO linked chain */
        for (i = 0; i < (FIFO_SIZE-1); i++)
        {
            intVecFifo[i].isrDsrPrms  = NULL;
            intVecFifo[i].nextPtr     = &intVecFifo[i+1];
        }
        intVecFifo[i].nextPtr =  &intVecFifo[0];
        
        /* set the head and tail pointers of the FIFO */
        fifoHeadPtr = &intVecFifo[0];
        fifoTailPtr = &intVecFifo[0];
            
        /* create semaphore for signalling done from Isr to Dsr */
        if(osSemBinCreate("PssDsr", OS_SEMB_EMPTY, &dsrSem) != GT_OK)
        {
            return GT_FAIL;
        }
        
        /* create the Dsr task */
        if (osTaskCreate("PssDsr", DSR_PRIO_CNS, DSR_STACK_SIZE_CNS, extDrvDsr, 
                         (GT_VOID*)NULL, &dsrTid) != GT_OK)
        {
            return GT_FAIL;
        }
        firstTime = GT_FALSE;
    }
    
    if (dsrPrmsPtr[intVecNum] != NULL)
    {
        /* vector allready initialized */
        return GT_FAIL;
    }
    
    /* allocate memory for Isr and Dsr */
    isrDsrParamsPtr = osStaticMalloc(sizeof(ISR_DSR_PARAMS));
    if (NULL == isrDsrParamsPtr)
    {
        return GT_FAIL;
    }
    /* set all Isr/Dsr data parameters */
    isrDsrParamsPtr->intVecNum      = intVecNum;
    isrDsrParamsPtr->routine        = (USER_DSR_PTR)routine;
    isrDsrParamsPtr->intVecMapIdx   = (GT_U8)parameter;
    
    dsrPrmsPtr[intVecNum] = isrDsrParamsPtr;
    
    return osInterruptConnect(intVecNum, extDrvIntIsr, (GT_U32)isrDsrParamsPtr);
}


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
    if (osIntEnable(intVecNum) != GT_OK)
    {
        return GT_FAIL;
    }
    
    return GT_OK;
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
    if (osIntDisable(intVecNum) != GT_OK)
    {
        return GT_FAIL;
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
    if (NULL == key)
        return 1;

    switch (mode)
    {
        case INTR_MODE_UNLOCK:
            if (intLockSem)
            {
                osSemSignal(intLockSem);
            }
            break;

        case INTR_MODE_LOCK:
            if (intLockSem)
            {
                osSemWait(intLockSem, OS_WAIT_FOREVER);
            }
            break;

        default:
            return 1;
            break;
    }

    return 0;
}



