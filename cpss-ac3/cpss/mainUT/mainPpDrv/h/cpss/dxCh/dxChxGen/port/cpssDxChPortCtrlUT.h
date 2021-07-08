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
* @file cpssDxChPortCtrlUT.h
*
* @brief Includes structures definition for the use in UT of Dx Cheetah Prestera SW
*
* @version   88
********************************************************************************
*/

#ifndef __cpssDxChPortCtrlUTh
#define __cpssDxChPortCtrlUTh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* get CPSS Common Driver facility types definitions */
#include <cpss/common/cpssTypes.h>

/* get CPSS definitions for port configurations */
#include <cpss/generic/port/cpssPortCtrl.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>


#define UT_MAX_PORTS                        512

typedef struct
{
    GT_PHYSICAL_PORT_NUM                    portNum;
    CPSS_DXCH_SHADOW_PORT_MAP_STC     portMap;
} PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC;

typedef struct
{
    GT_U32                                    Current;
    GT_U32                                    Len;
    GT_U32                                    Size;
    PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC    Data[UT_MAX_PORTS];
} HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC;

typedef struct
{
    GT_PHYSICAL_PORT_NUM                     portNum;
    CPSS_PORT_SPEED_ENT                      speed;
    OUT CPSS_PORT_INTERFACE_MODE_ENT         ifMode;
    GT_U32                                   macNum;
} PORT_NUM_SPEED_INTERFACE;

typedef struct
{
    GT_U32                      Current;
    GT_U32                      Len;
    GT_U32                      Size;
    PORT_NUM_SPEED_INTERFACE    Data[UT_MAX_PORTS];
} HDR_PORT_NUM_SPEED_INTERFACE_STC;

GT_STATUS prvPortShadowMapGet
(
    IN  GT_U8                                          devNum,
    OUT HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC    *portShadowPtr
);

GT_STATUS prvDefaultPortsMappingAndActivePortsParamSaving
(
    IN  GT_U8                                          devNum,
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC    *portShadowPtr,
    OUT HDR_PORT_NUM_SPEED_INTERFACE_STC              *portActivePtr
);

GT_STATUS prvPortActivateDeactivate
(
    IN GT_U8                         devNum, 
    IN GT_BOOL                       Mode, 
    IN GT_PHYSICAL_PORT_NUM          portNum, 
    IN CPSS_PORT_INTERFACE_MODE_ENT  ifMode, 
    IN CPSS_PORT_SPEED_ENT           speed
);

GT_STATUS prvPortsActivateDeactivate
(
    IN GT_U8                              devNum,
    IN GT_BOOL                            Mode,
    IN HDR_PORT_NUM_SPEED_INTERFACE_STC  *portActivePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortCtrlUTh */

