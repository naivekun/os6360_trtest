/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsPortCtrlInit.c
*
* @brief Port Control Initialization
*
* @version   1
********************************************************************************
*/
#include <mvHwsPortCtrlInc.h>
#include <mvHwsServiceCpuInt.h>

/**
* @internal mvPortCtrlHwInit function
* @endinternal
*
* @brief   Initialize HW related tasks
*         - IPC infrastructure
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlHwInit(void)
{
    /* Initialize Interrupts */
    mvHwsServCpuIntrInit(0/* To Add Shared memory section for interrupt map*/);

    /* Initialize IPC */
    CHECK_STATUS(mvPortCtrlIpcInit());

    return GT_OK;
}

/**
* @internal mvPortCtrlSwInit function
* @endinternal
*
* @brief   Initialize Application related tasks
*         - Initialize Firmware - AAPL structures
*         - Initialize Database
*         - Initialize Real-time Log
*         - Initialize OS Resources
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlSwInit(void)
{
    /* Initialize Device */
    CHECK_STATUS(mvPortCtrlDevInit());
    /* Initialize Database */
    CHECK_STATUS(mvPortCtrlDbInit());
    /* Initialize Real-time Log */
    CHECK_STATUS(mvPortCtrlLogInit());
    /* Initialize OS Resources */
    CHECK_STATUS(mvPortCtrlOsResourcesInit());

    return GT_OK;
}



