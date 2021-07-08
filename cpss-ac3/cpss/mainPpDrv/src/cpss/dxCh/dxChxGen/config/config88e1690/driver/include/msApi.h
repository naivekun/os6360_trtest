#include <Copyright.h>

/**
********************************************************************************
* @file msApi.h
*
* @brief API definitions for 88E1690
*
*/
/********************************************************************************
* msApi.h
*
* DESCRIPTION:
*       API definitions for 88E1690
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#ifndef __prvCpssDrvMsApi_h
#define __prvCpssDrvMsApi_h
#ifdef __cplusplus
extern "C" {
#endif

/* minimum need header files*/
#include <msApiTypes.h>
#include <h/driver/gtHwAccess.h>
#include <h/platform/gtSem.h>
#include <h/msApi/gtSysConfig.h>
#include <h/msApi/gtUtils.h>

/* add modules whatever you needed */
#include <h/msApi/gtTCAM.h>
#include <h/msApi/gtVct.h>
#include <h/msApi/gtBrgFdb.h>
#include <h/msApi/gtBrgStu.h>
#include <h/msApi/gtBrgVtu.h>
#include <h/msApi/gtPIRL.h>
#include <h/msApi/gtPortRmon.h>
#include <h/msApi/gtPortCtrl.h>
#include <h/msApi/gtPortPrioMap.h>
#include <h/msApi/gtQueueCtrl.h>
#include <h/msApi/gtSysCtrl.h>
#include <h/msApi/gtPhyCtrl.h>
#include <h/msApi/gtIMP.h>
#ifdef __cplusplus
}
#endif

#endif /* __prvCpssDrvMsApi_h */

