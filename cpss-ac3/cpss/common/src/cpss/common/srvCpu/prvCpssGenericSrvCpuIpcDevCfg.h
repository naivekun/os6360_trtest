/*******************************************************************************
*              (c), Copyright 2015, Marvell International Ltd.                 *
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
* @file prvCpssGenericSrvCpuIpcDevCfg.h
*
* @brief Service CPU IPC per-device config
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssGenericSrvCpuIpcDevCfg_h__
#define __prvCpssGenericSrvCpuIpcDevCfg_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuIpc.h>
#include <cpss/generic/ipc/mvShmIpc.h>
#include "prvSrvCpuFirmware.h"


/********************************************************************/
/**** Indirect support                                           ****/
/********************************************************************/

typedef struct {
    GT_UINTPTR           base; /* == 0 for indirect */
    GT_UINTPTR           size;
    GT_U32               targetBase;
    GT_U32               shmAddr;
    GT_U32               shmSize;
    GT_UINTPTR           shmResvdRegs;
    CPSS_HW_DRIVER_STC  *drv;
    IPC_SHM_STC          shm;
    GT_BOOL              irqBound;
} PRV_SRVCPU_IPC_CHANNEL_STC;
GT_STATUS prvIpcAccessInit(PRV_SRVCPU_IPC_CHANNEL_STC *s, GT_BOOL init);
GT_STATUS prvIpcAccessDestroy(PRV_SRVCPU_IPC_CHANNEL_STC *s);
CPSS_HW_DRIVER_STC *prvSrvCpuRamDirectCreate(
    IN  GT_UINTPTR  virtStart,
    IN  GT_U32      targetStart,
    IN  GT_U32      targetSize
);
CPSS_HW_DRIVER_STC *prvSrvCpuRamIndirectCreate(
    IN  CPSS_HW_DRIVER_STC  *d,
    IN  GT_U32              addrSpace,
    IN  GT_U32              regStart,
    IN  GT_U32              size
);




/********************************************************************/
/**** Service CPU driver APIs                                    ****/
/********************************************************************/
typedef enum {
    PRV_SRVCPU_OPS_EXISTS_E,
    PRV_SRVCPU_OPS_IS_ON_E,
    PRV_SRVCPU_OPS_PRE_LOAD_E,
    PRV_SRVCPU_OPS_RESET_E,
    PRV_SRVCPU_OPS_UNRESET_E,
    PRV_SRVCPU_OPS_PRE_CHECK_IPC_E,
} PRV_SRVCPU_OPS_ENT;
typedef GT_STATUS (*PRV_SRVCPU_OPS_FUNC)(
    IN  CPSS_HW_INFO_STC    *hwInfo,
    IN  CPSS_HW_DRIVER_STC  *drv,
    IN  GT_U32              scpuId,
    IN  PRV_SRVCPU_OPS_ENT  op,
    OUT void                *ret
);
/* drivers */
GT_STATUS prvSrvCpuOpsMsys(
    IN  CPSS_HW_INFO_STC    *hwInfo,
    IN  CPSS_HW_DRIVER_STC  *drv,
    IN  GT_U32              scpuId,
    IN  PRV_SRVCPU_OPS_ENT  op,
    OUT void                *ret
);
GT_STATUS prvSrvCpuOpsBobK(
    IN  CPSS_HW_INFO_STC    *hwInfo,
    IN  CPSS_HW_DRIVER_STC  *drv,
    IN  GT_U32              scpuId,
    IN  PRV_SRVCPU_OPS_ENT  op,
    OUT void                *ret
);
GT_STATUS prvSrvCpuOpsCM3(
    IN  CPSS_HW_INFO_STC    *hwInfo,
    IN  CPSS_HW_DRIVER_STC  *drv,
    IN  GT_U32              scpuId,
    IN  PRV_SRVCPU_OPS_ENT  op,
    OUT void                *ret
);

extern PRV_SRVCPU_IPC_CHANNEL_STC* prvIpcDevCfg[PRV_CPSS_MAX_PP_DEVICES_CNS];

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssGenericSrvCpuIpcDevCfg_h__ */

