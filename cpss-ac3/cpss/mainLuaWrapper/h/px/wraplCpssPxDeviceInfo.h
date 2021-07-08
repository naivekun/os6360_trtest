/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file wraplCpssPxDeviceInfo.h
*
* @brief PX device info wrapper
*
* @version   1
********************************************************************************
*/
#ifndef __wraplCpssPxDeviceInfo_h__
#define __wraplCpssPxDeviceInfo_h__


#include <cpssCommon/wraplCpssExtras.h>
#include <cpssCommon/wrapCpssDebugInfo.h>


/**
* @internal wrlMvPxVersionGet function
* @endinternal
*
* @brief   Getting of cpss px version name.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] L                        - lua state
*                                       Count of put lua_State parameters
*/
int wrlMvPxVersionGet
(
    IN lua_State    *L
);


#endif /* __wraplCpssPxDeviceInfo_h__ */

