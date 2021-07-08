/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* mvHwsD2dIf.h
*
* DESCRIPTION:
*        D2d interface header file
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
******************************************************************************/

#ifndef __mvHwsD2dIf_H
#define __mvHwsD2dIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>

#define HWS_D2D_STEP_PER_RAVEN_CNS   4
#define HWS_D2D_NUMBER_PER_RAVEN_CNS 2
#define HWS_D2D_ALL                  0xFFFFFFFF
#define HWS_D2D_PORTS_NUM_CNS        8 /* number of ports in D2D*/

#define PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum) (d2dNum%2==0)

#define PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum) (d2dNum>>1)
#define PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, isRaven) ((d2dIdx << 1) + isRaven)

/* 4 d2d in each  raven (2 raven+2 eagle) */
#define PRV_HWS_D2D_GET_RAVEN_IDX_MAC(d2dNum) (d2dNum>>2)

/* convert Global port MAC number to global D2D index
  271 is total network ports in 12.8 falcon
  1024 is the first D2D port
  */
#define PRV_HWS_D2D_CONVERT_PORT_TO_D2D_IDX_MAC(port) \
        ((port <= 271)? (port/HWS_D2D_PORTS_NUM_CNS): (port >= 1024)? ((port-1024)>>1):port)
#define PRV_HWS_D2D_IS_REDUCE_PORT_MAC(port) (GT_FALSE) /*TODO add REDUCE port*/

/* convert global port number to local D2D port number*/
#define PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(port) \
        ((port <= 271)? (port%HWS_D2D_PORTS_NUM_CNS): (port >= 1024)? (((port-1024)>>1)%HWS_D2D_PORTS_NUM_CNS): PRV_HWS_D2D_IS_REDUCE_PORT_MAC(port)? HWS_D2D_PORTS_NUM_CNS: 0)

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsD2dIf_H */


