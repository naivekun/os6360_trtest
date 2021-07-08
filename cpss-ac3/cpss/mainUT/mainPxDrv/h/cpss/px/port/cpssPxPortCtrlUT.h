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
* @file cpssPxPortCtrlUT.h
*
* @brief Includes structures definition for the use in UT of px
*
* @version   1
********************************************************************************
*/

#ifndef __cpssPxPortCtrlUTh
#define __cpssPxPortCtrlUTh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* get CPSS Common Driver facility types definitions */
#include <cpss/common/cpssTypes.h>

/* get CPSS definitions for port configurations */
#include <cpss/px/port/cpssPxPortCtrl.h>

#include <cpss/px/config/private/prvCpssPxInfo.h>

#define UT_MAX_PORTS                        16
#define IS_XAUI_SUPPORTED_ON_PORT_MAC(port)    \
        (port < 12)


typedef struct
{
    GT_PHYSICAL_PORT_NUM                    portNum;
    CPSS_PX_SHADOW_PORT_MAP_STC     portMap;
} PORT_NUM_CPSS_PX_PORT_MAP_SHADOW_STC;

typedef struct
{
    GT_U32                                    Current;
    GT_U32                                    Len;
    GT_U32                                    Size;
    PORT_NUM_CPSS_PX_PORT_MAP_SHADOW_STC    Data[UT_MAX_PORTS];
} HDR_PORT_NUM_CPSS_PX_PORT_MAP_SHADOW_STC;

typedef struct
{
    GT_PHYSICAL_PORT_NUM                     portNum;
    CPSS_PORT_SPEED_ENT                      speed;
    CPSS_PORT_INTERFACE_MODE_ENT             ifMode;
    GT_U32                                   macNum;
} PORT_NUM_SPEED_INTERFACE;

typedef struct
{
    GT_U32                      Current;
    GT_U32                      Len;
    GT_U32                      Size;
    PORT_NUM_SPEED_INTERFACE    Data[UT_MAX_PORTS];
} HDR_PORT_NUM_SPEED_INTERFACE_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxPortCtrlUTh */

