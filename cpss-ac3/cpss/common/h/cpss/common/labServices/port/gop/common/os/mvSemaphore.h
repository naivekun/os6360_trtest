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
* mvCfgElementDb.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 12 $
*
*******************************************************************************/
#ifndef _mvSemaphore_h
#define _mvSemaphore_h

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

#define MV_SEMA_REG_BASE_CNM_PCI   ( 0x20500 )
#define MV_SEMA_REG_BASE_SWITCH_MG ( 0x800 )
#define MV_MAX_SEMA             128
#define MV_SEMA_AVAGO           0
#define MV_SEMA_CG              1


GT_BOOL mvSemaLock(GT_U8 devNum, GT_U32 num);
GT_BOOL mvSemaTryLock(GT_U8 devNum, GT_U32 num);
GT_BOOL mvSemaUnlock(GT_U8 devNum, GT_U32 num);
GT_STATUS mvSemaOwnerGet(GT_U8 devNum, GT_U32 *semaOwner);

#endif /* _mvSemaphore_h */
