/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file appDemoExMxPmPortUtils.h
*
* @brief App demo ExMxPm port API headers.
*
* @version   2
********************************************************************************
*/

#ifndef __appDemoExMxPmPortUtilsh
#define __appDemoExMxPmPortUtilsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/port/cpssPortCtrl.h>

/**
* @internal gtAppDemoPuma3PortModeSpeedSet function
* @endinternal
*
* @brief   Example of configuration sequence of different ports interface modes
*         and speeds
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
* @param[in] ifMode                   - Interface mode.
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS gtAppDemoPuma3PortModeSpeedSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT   ifMode,
    IN  CPSS_PORT_SPEED_ENT  speed
);

/**
* @internal appDemoPuma3BoardTypeSet function
* @endinternal
*
* @brief   A pre-init API defining whether the board is a modular or a
*         standalone board.
* @param[in] devNum                   - device number
* @param[in] isModular                - GT_TRUE  - The board is a modular board.
*                                      - GT_FALSE - The board is a standalone board.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - otherwise.
*/
GT_STATUS appDemoPuma3BoardTypeSet
(
    GT_U8   devNum,
    GT_BOOL isModular
);

/**
* @internal appDemoPuma3LoopbackEnableSet function
* @endinternal
*
* @brief   A pre-init API defining whether the fabric ports are initialized with a
*         loopback or not.
* @param[in] devNum                   - device number
* @param[in] enableLoopBack           - GT_TRUE  - initialize fabric ports with loopback.
*                                      - GT_FALSE - initialize fabric ports without loopback.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - otherwise.
*/
GT_STATUS appDemoPuma3LoopbackEnableSet
(
    GT_U8   devNum,
    GT_BOOL enableLoopBack
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __appDemoExMxPmPortUtilsh */


