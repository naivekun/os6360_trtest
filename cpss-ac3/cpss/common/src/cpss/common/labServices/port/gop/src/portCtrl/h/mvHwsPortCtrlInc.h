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
* @file mvHwsPortCtrlInc.h
*
* @brief Port Control Include header files
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsPortCtrlInc_H
#define __mvHwsPortCtrlInc_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include "printf.h"
#include <gtOs/gtOsMem.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcDefs.h>
#include <mvHwsPortCtrlDefines.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApDefs.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApLogDefs.h>
#include <mvHwsPortCtrlIpc.h>
#include <mvHwsPortCtrlInit.h>
#include <mvHwsPortCtrlAp.h>
#include <mvHwsPortCtrlDb.h>
#include <mvHwsPortCtrlLog.h>
#include <mvHwsPortCtrlSupervisor.h>
#include <mvHwsPortCtrlPort.h>
#ifdef AP_GENERAL_TASK
#include <mvHwsPortCtrlGeneral.h>
#endif
#include <mvHwsPortCtrlDbg.h>

/* Cache invalidate. Used for validating memory values in
   arm7 as the memory is read directly by the Host CPU */
#if defined(INC_FREERTOS_H) && !defined(CM3)
    extern void armv7_dcache_wbinv_range( unsigned long base, unsigned long size);
#   define FW_CACHE_INVALIDATE(_ptr,_len) armv7_dcache_wbinv_range((unsigned long)(_ptr), (_len))
#else
#   define FW_CACHE_INVALIDATE(_ptr,_len)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvHwsPortCtrlInc_H */

