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
* @file mvHwsD2dPcsIf.h
*
* @brief D2D PCS interface API
*
* @version   1
********************************************************************************
*/
#ifndef __mvHwsD2dPcsIf_H
#define __mvHwsD2dPcsIf_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesIf.h>

GT_STATUS mvHwsD2dPcsReset
(
    GT_U8   devNum,
    GT_U32  d2dNum,
    GT_BOOL enable
);

/**
* @internal mvHwsD2dPcsTestGen function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] portGroup                - D2D index
* @param[in] pcsNum                   - port number
*/
GT_STATUS mvHwsD2dPcsTestGen
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          pcsNum,
    MV_HWS_D2D_PRBS_CONFIG_STC      *configPtr
);

/**
* @internal mvHwsD2dPcsLoopbackSet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] portGroup                - portGroup
* @param[in] pcsNum                   - port number
* @param[in] lbType                   - loopback type
*/
GT_STATUS mvHwsD2dPcsLoopbackSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
);

/**
* @internal mvHwsD2dPcsLoopbackGet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] portGroup                - portGroup
* @param[in] pcsNum                   - port number
* @param[in] lbType                   - loopback type
*/
GT_STATUS mvHwsD2dPcsLoopbackGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
);

/**
* @internal mvHwsD2dPcsErrorInjectionSet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group
* @param[in] portNum                   - port number
* @param[in] laneIdx                   - lane 0..3
* @param[in] errorNum                 - number of errors to inject
*/
GT_STATUS mvHwsD2dPcsErrorInjectionSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portNum,  /* 1024.. */
    GT_U32                  laneIdx,  /* lane 0..3 */
    GT_U32                  errorNum    /* 1..0x7FF, 0x7FF enables continuously injection */
);

/**
* @internal mvHwsD2dPcsErrorCounterGet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group
* @param[in] port                     - port number 1024..
* @param[in] laneIdx                  - lane index
* @param[out] errorCntPtr             - (pointer to) number of errors counter
*/
GT_STATUS mvHwsD2dPcsErrorCounterGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  port,       /* port number */
    GT_U32                  laneIdx,  /* lane 0..3 */
    GT_U32                  *errorCntPtr
);

/**
* @internal mvHwsD2dPcsIfInit function
* @endinternal
 *
* @param[in] devNum                   - system device number
*/
GT_STATUS mvHwsD2dPcsIfInit(GT_U8 devNum, MV_HWS_PCS_FUNC_PTRS *funcPtrArray);


/**
* @internal hwsD2dPcsIfClose function
* @endinternal
 *
* @param[in] devNum                   - system device number
*/
void hwsD2dPcsIfClose
(
    GT_U8      devNum
);


/**
* @internal mvHwsD2dPcsInit function
* @endinternal
 *
* @param[in] driverPtr                - driver to write with smi
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
*/
GT_STATUS mvHwsD2dPcsInit
(
    CPSS_HW_DRIVER_STC  *driverPtr,
    GT_U8               devNum,
    GT_U32              d2dNum

);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvHwsD2dPcsIf_H */

