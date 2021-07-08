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
* @file gtAppInit.c
*
* @brief This file includes functions to be called on system initialization,
* for demo and special purposes.
*
* @version   47
********************************************************************************
*/



#include <gtOs/gtOsGen.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsIo.h>
#include <cmdShell/common/cmdCommon.h>
#include <cmdShell/shell/cmdMain.h>

/*support 'makefile' ability to define the size of the allocation */
#ifndef APP_DEMO_OSMEM_DEFAULT_MEM_INIT_SIZE
    /* Default memory size */
    #define APP_DEMO_OSMEM_DEFAULT_MEM_INIT_SIZE (2048*1024)
#endif /* ! APP_DEMO_OSMEM_DEFAULT_MEM_INIT_SIZE */


GT_STATUS appDemoCpssInit(GT_VOID);

/* sample code fo initializatuion befor the shell starts */
#ifdef INIT_BEFORE_SHELL

#include <gtOs/gtOsSem.h>
#include <gtOs/gtOsTask.h>


GT_SEM semCmdSysInit;

/**
* @internal cpssInitSystemCall function
* @endinternal
*
* @brief   This routine is the starting point of the Driver.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssInitSystemCall(GT_VOID)
{
    GT_STATUS rc;
    osPrintf("cpssInitSystem(28,1,0) will be called\n");
    rc = cpssInitSystem(28,1,0);
    osPrintf("cpssInitSystem(28,1,0) returned 0x%X\n", rc );
#if 0
    rc = osSemSignal(semCmdSysInit);
    osPrintf("Signal semaphore, rc = 0x%X\n", rc);
#endif
    return GT_OK;
}
/**
* @internal cpssInitSystemCallTask function
* @endinternal
*
* @brief   This routine is the starting point of the Driver.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssInitSystemCallTask(GT_VOID)
{
    GT_STATUS rc;
    GT_U32    taskSysInitCmd;

    rc = osWrapperOpen(NULL);
    osPrintf("osWrapperOpen called, rc = 0x%X\n", rc);
#if 0
    rc = osSemBinCreate("semCmdSysInit", OS_SEMB_EMPTY, &semCmdSysInit);
    osPrintf("Create semCmdSysInit semaphore, rc = 0x%X\n", rc);
#endif
    osTimerWkAfter(1000);
    rc = osTaskCreate (
        "CMD_InitSystem", 10 /* priority */,
        32768 /*stack size */,
        (unsigned (__TASKCONV *)(void*)) cpssInitSystemCall,
        NULL, &taskSysInitCmd);
    osPrintf("Create taskSysInitCmd task, rc = 0x%X, id = 0x%X \n",
           rc, taskSysInitCmd);
#if 0
    rc = osSemWait(semCmdSysInit, 600000); /* 10 minutes */
    osTimerWkAfter(1000);
    osPrintf("Wait for semaphore, rc = 0x%X\n", rc);
#endif
    return GT_OK;
}

#endif /*INIT_BEFORE_SHELL*/

/**
* @internal userAppInitialize function
* @endinternal
*
* @brief   This routine is the starting point of the Driver.
*         Called from userAppInit() or from win32:main()
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS userAppInitialize(
    IN GT_U32 devNum
)
{
    GT_STATUS rc;

    /* must be called before any Os function */
    rc = osWrapperOpen(NULL);
    if(rc != GT_OK)
    {
        osPrintf("osWrapper initialization failure!\n");
        return rc;
    }

    /* Initialize memory pool. It must be done before any memory allocations */
    /* must be before osWrapperOpen(...) that calls osStaticMalloc(...) */
    rc = osMemInit(APP_DEMO_OSMEM_DEFAULT_MEM_INIT_SIZE, GT_TRUE);
    if (rc != GT_OK)
    {
        osPrintf("osMemInit() failed, rc=%d\n", rc);
        return rc;
    }

    /* run appDemoCpssInit() directly, not from console => show board list => show sw version
     * this allow application to work without console task */
    rc = appDemoCpssInit();
    if (rc != GT_OK)
    {
        osPrintf("appDemoCpssInit() failed, rc=%d\n", rc);
    }

    /* Set gtInitSystem to be the init function */
    /*cpssInitSystemFuncPtr = (GT_INTFUNCPTR)cpssInitSystem;*/

#ifdef INIT_BEFORE_SHELL
    osPrintf("cpssInitSystemCallTask will be called\n");
    rc = cpssInitSystemCallTask();
    osPrintf("cpssInitSystemCallTask was called, rc=%d\n", rc);
#endif /*INIT_BEFORE_SHELL*/
    /* Start the command shell */



    return cmdInit(devNum);
} /* userAppInitialize */

/**
* @internal userAppInit function
* @endinternal
*
* @brief   This routine is the starting point of the Driver.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS userAppInit(GT_VOID)
{
    return userAppInitialize(0);
} /* userAppInit */





