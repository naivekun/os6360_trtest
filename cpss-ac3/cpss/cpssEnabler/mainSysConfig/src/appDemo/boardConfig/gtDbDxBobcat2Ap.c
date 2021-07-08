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
* @file gtDbDxBobcat2Ap.c
*
* @brief AP task for Bobcat2 board.
*
* @version   1
********************************************************************************
*/

#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/boardConfig/gtBoardsConfigFuncs.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>

#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInitLedCtrl.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpss/common/init/cpssInit.h>

/* structures definition for the use of Dx Cheetah Prestera core SW */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>

#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortAp.h>

/* include the external services */
#include <cmdShell/common/cmdExtServices.h>

/*******************************************************************************
 * External definitions
 ******************************************************************************/

/*******************************************************************************
 * Global variables
 ******************************************************************************/
#ifndef ASIC_SIMULATION_NO_HW

static GT_BOOL apLogInProgress = GT_FALSE;
static GT_BOOL apLogDisabled = GT_FALSE;

static unsigned __TASKCONV apLogTask
(
    GT_VOID * arg
)
{
    GT_STATUS               rc;
    GT_U32                  param;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;

    param = *(GT_U32*)arg;
    devNum = (GT_U8)(param & 0xFFFF);
    portNum = ((param >> 16) & 0xFFFF);

    while(1)
    {
        osTimerWkAfter(200);
        if(apLogDisabled)
        {
            apLogInProgress = GT_FALSE;
            apLogDisabled = GT_FALSE;
            break;
        }
        else
        {
            rc = cpssDxChPortApDebugInfoGet(devNum, portNum, GT_TRUE);
            if(rc != GT_OK)
            {
                cpssOsPrintf("\nAP LOG THREAD - CALL mvApPortCtrlDebugInfoShow FAIL");
            }

        }
    }
#ifdef __GNUC__
    /* to avoid warnings in gcc */
    return 0;
#endif
}

GT_STATUS appDemoApLog
(
    GT_BOOL enable,
    GT_U8   devNum,
    GT_PHYSICAL_PORT_NUM   portNum
)
{
    GT_TASK tid;
    GT_U32 param;

    param = devNum;
    param = param | ((portNum & 0xFFFF) << 16);


    if(enable)
    {
        if(apLogInProgress == GT_FALSE)
        {
            if(osTaskCreate("apLogging",
                255, 0x1000, apLogTask, &param, &tid) != GT_OK)
            {
                cpssOsPrintf("\nfailed to create thread for ap logging");
                apLogInProgress = GT_FALSE;
                return GT_FAIL;
            }
            else
            {
                apLogInProgress = GT_TRUE;
                cpssOsPrintf("\ncreate thread for ap logging");
            }
        }
        else
        {
            cpssOsPrintf("\nfailed to start ap logging - ap logging or stop logging is still in progress");
            return GT_FAIL;
        }
    }
    else
    {
        if(apLogDisabled)
        {
            cpssOsPrintf("\nfailed to stop ap logging - ap stop logging is already in progress");
            return GT_FAIL;
        }
        else
        {
            if(apLogInProgress)
            {
                cpssOsPrintf("\nstop ap logging");
                apLogDisabled = GT_TRUE;
            }
            else
            {
                cpssOsPrintf("\nfailed to stop ap logging - ap logging is not in progress");
                return GT_FAIL;
            }
        }
    }

    return GT_OK;
}

GT_STATUS appDemoDxChPortApLinkUpDebug
(
    GT_U8 devNum,
    GT_PHYSICAL_PORT_NUM portNum0,
    GT_PHYSICAL_PORT_NUM portNum1,
    CPSS_PORT_SPEED_ENT speed,
    CPSS_PORT_INTERFACE_MODE_ENT ifMode,
    GT_BOOL noneceDisableValue,
    GT_U32 timesLoop,
    GT_U32 linkCheckDelay,
    GT_U32 enDelay,
    GT_U32 disDelay
)
{
          GT_STATUS                           rc = GT_OK;
          CPSS_DXCH_PORT_AP_PARAMS_STC        apParams;
          GT_BOOL                             isLinkUp_0, isLinkUp_1;
          GT_U32                              times;

          /* Port AP params */
          cpssOsMemSet(&apParams, 0, sizeof(apParams));

          apParams.fcPause = GT_TRUE;
          apParams.fcAsmDir = 0;
          apParams.fecSupported = GT_TRUE;
          apParams.fecRequired =  GT_TRUE;
          apParams.noneceDisable = noneceDisableValue;
          apParams.laneNum = 0;
          apParams.modesAdvertiseArr[0].ifMode = ifMode;
          apParams.modesAdvertiseArr[0].speed = speed;

                   for (times = 0; times < timesLoop; times++) {
                   /* Enable AP */
                   rc = cpssDxChPortApPortConfigSet(devNum, portNum0, GT_TRUE, &apParams);
                   if (rc != GT_OK) {
                             cpssOsPrintf("cpssDxChPortApPortConfigSet 2 fail, return %d\n\r", rc);
                             return rc;
                   }

     cpssOsTimerWkAfter(1000*enDelay);

                   rc = cpssDxChPortApPortConfigSet(devNum, portNum1, GT_TRUE, &apParams);
                   if (rc != GT_OK) {
                             cpssOsPrintf("cpssDxChPortApPortConfigSet 2 fail, return %d\n\r", rc);
                             return rc;
                   }
                   cpssOsTimerWkAfter(1000*linkCheckDelay);

                   /* Check the status */
                   rc = cpssDxChPortLinkStatusGet(devNum, portNum0, &isLinkUp_0);
                   if (rc != GT_OK) {
                             cpssOsPrintf("cpssDxChPortLinkStatusGet 0 fail, return %d\n\r", rc);
                             return rc;
                   }
                   rc = cpssDxChPortLinkStatusGet(devNum, portNum1, &isLinkUp_1);
                   if (rc != GT_OK) {
                             cpssOsPrintf("cpssDxChPortLinkStatusGet 4 fail, return %d\n\r", rc);
                             return rc;
                   }
                   if ((GT_FALSE == isLinkUp_1) || (GT_FALSE == isLinkUp_0)) {
                             rc = cpssDxChPortApDebugInfoGet(devNum, 255, GT_FALSE);
                             if (rc != GT_OK) {
                                      cpssOsPrintf("cpssDxChPortApDebugInfoGet fail, return %d\n\r", rc);
                                      return rc;
                             }

                             cpssOsPrintf("Link status: Port0=%d Port4=%d\n\r", isLinkUp_0, isLinkUp_1);
                             return 0;
                   }

                   /* Disable AP */
                   rc = cpssDxChPortApPortConfigSet(devNum, portNum0, GT_FALSE, &apParams);
                   if (rc != GT_OK) {
                             cpssOsPrintf("cpssDxChPortApPortConfigSet 1 fail, return %d\n\r", rc);
                             return rc;
                   }

                   cpssOsTimerWkAfter(disDelay*1000);

                   rc = cpssDxChPortApPortConfigSet(devNum, portNum1, GT_FALSE, &apParams);
                   if (rc != GT_OK) {
                             cpssOsPrintf("cpssDxChPortApPortConfigSet 1 fail, return %d\n\r", rc);
                             return rc;
                   }

                   cpssOsPrintf("Test %d done\n", times);

                   cpssOsTimerWkAfter(disDelay*1000);
          }

          return rc;
}

#endif




