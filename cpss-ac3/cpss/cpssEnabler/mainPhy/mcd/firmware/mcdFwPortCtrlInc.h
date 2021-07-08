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
* mcdFwPortCtrlIncAp.h
*
* DESCRIPTION:
*       Port Control Include header files
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __mcdFwPortCtrlInc_H
#define __mcdFwPortCtrlInc_H

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
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <mcdInternalIpcDefs.h>
#include <mcdFwPortCtrlDefines.h>
#include <mcdFwPortCtrlApDefs.h>
#include <mcdFwPortCtrlApLogDefs.h>

/**/
#include <mcdFwPortCtrlIpc.h>
#include <mcdFwPortCtrlInit.h>
#include <mcdFwPortCtrlAp.h>
#include <mcdFwPortCtrlDb.h>
#include <mcdFwPortCtrlLog.h>
#include <mcdFwPortCtrlSupervisor.h>
#include <mcdFwPortCtrlPort.h>
#include <mcdFwPortCtrlGeneral.h>
#include <mcdFwPortCtrlDbg.h>

/*#define RMV_CONSORTIUM*/
#define AP_PRINT_MCD_MAC(x) /* \
    osPrintf("%d: ",mvPortCtrlCurrentTs());\
    osPrintf x*/

#define AP_PRINT_MCD2_MAC(x) /* \
    osPrintf("%d: ",mvPortCtrlCurrentTs());\
    osPrintf x*/

/* Cache invalidate. Used for validating memory values in
   arm7 as the memory is read directly by the Host CPU */
#if defined(INC_FREERTOS_H) && (defined(BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT))
    extern void armv7_dcache_wbinv_range( unsigned long base, unsigned long size);
#   define FW_CACHE_INVALIDATE(_ptr,_len) armv7_dcache_wbinv_range((unsigned long)(_ptr), (_len))
#else
#   define FW_CACHE_INVALIDATE(_ptr,_len)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mcdFwPortCtrlInc_H */

