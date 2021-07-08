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
* @file mvHwsPortCtrlDbg.h
*
* @brief Port Control Port Debug
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsPortCtrlDbg_H
#define __mvHwsPortCtrlDbg_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal mvPortCtrlDebugParamSet function
* @endinternal
*
*/
void mvPortCtrlDebugParamSet(GT_U32 param);

#ifdef FREE_RTOS_HWS_ENHANCED_PRINT_MODE
/**
* @internal mvPortCtrlDebugRoutine function
* @endinternal
*
*/
void mvPortCtrlDebugRoutine(void* pvParameters);

/**
* @internal mvPortCtrlDbgLock function
* @endinternal
*
*/
GT_STATUS mvPortCtrlDbgLock(void);
GT_STATUS mvPortCtrlDbgUnlock(void);
#endif /* FREE_RTOS_HWS_ENHANCED_PRINT_MODE */

/**
* @internal mvPortCtrlDbgOnDemandRegsDump function
* @endinternal
*
*/
void mvPortCtrlDbgOnDemandRegsDump(GT_U8 portIndex, GT_U8 pcsNum, GT_U8 state);

/**
* @internal mvPortCtrlDbgO_CoreStatusDump function
* @endinternal
*
*/
void mvPortCtrlDbgO_CoreStatusDump(GT_U8 portIndex, GT_U8 state);

/**
* @internal mvPortCtrlDbgCfgRegsDump function
* @endinternal
*
*/
void mvPortCtrlDbgCfgRegsDump(GT_U32 active, GT_U32 mode);

/**
* @internal mvPortCtrlDbgIntRegsDump function
* @endinternal
*
*/
void mvPortCtrlDbgIntRegsDump(GT_U8 port, GT_U8 pcsNum, GT_U8 state);

/**
* @internal mvPortCtrlDbgAllIntRegsDump function
* @endinternal
*
*/
void mvPortCtrlDbgAllIntRegsDump(GT_U8 port, GT_U8 pcsNum, GT_U8 state);

/**
* @internal mvPortCtrlDbgAllRegsDump function
* @endinternal
*
* @brief   Print All AP internal registers
*/
void mvPortCtrlDbgAllRegsDump(GT_U8 port, GT_U8 pcsNum, GT_U8 state);

/**
* @internal mvPortCtrlDbgLinkRegsDump function
* @endinternal
*
*/
void mvPortCtrlDbgLinkRegsDump(GT_U8 port, GT_U8 laneNum, GT_U8 state);

/**
* @internal mvPortCtrlDbgCtrlRegsDump function
* @endinternal
*
*/
void mvPortCtrlDbgCtrlRegsDump(GT_U8 port, GT_U8 pcsNum, GT_U8 state);

#ifndef DISABLE_CLI
/**
* @internal mvPortCtrlDbgStatsDump function
* @endinternal
*
*/
void mvPortCtrlDbgStatsDump(GT_U8 port, GT_U8 peer);
#endif /* DISABLE_CLI */

/**
* @internal mvPortCtrlDbgReg function
* @endinternal
*
*/
void mvPortCtrlDbgReg(GT_U8 reg, GT_U8 value);

/**
* @internal mvPortCtrlDbgStatsReset function
* @endinternal
*
*/
void mvPortCtrlDbgStatsReset(GT_U8 port);

#ifndef DISABLE_CLI
/**
* @internal mvPortCtrlDbgIntropDump function
* @endinternal
*
*/
void mvPortCtrlDbgIntropDump(void);
#endif /* DISABLE_CLI */

/**
* @internal mvPortCtrlDbgIntropCfgParam function
* @endinternal
*
*/
void mvPortCtrlDbgIntropCfgParam(GT_U8 param, GT_U8 value);

/**
* @internal mvPortCtrlDbgIntropCfg function
* @endinternal
*
*/
void mvPortCtrlDbgIntropCfg(MV_HWS_IPC_PORT_AP_INTROP_STRUCT *apIntrop);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvHwsPortCtrlDbg_H */


