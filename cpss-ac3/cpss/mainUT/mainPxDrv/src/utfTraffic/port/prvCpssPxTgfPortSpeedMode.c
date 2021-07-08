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
* @file prvCpssPxTgfPortSpeedMode.c
*
* @brief Utility for invoking cpssPxPortModeSpeedSet API
*
* @version   1
********************************************************************************
*/


#include <utfTraffic/port/prvCpssPxTgfPortTxTailDropVariousPorts.h>
#include <utf/utfMain.h>
#include <extUtils/trafficEngine/tgfTrafficGenerator.h>
#include <utfTraffic/common/cpssPxTgfCommon.h>

#include <extUtils/trafficEngine/prvTgfLog.h>
#include <utf/private/prvUtfHelpers.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/px/port/cpssPxPortCtrl.h>

/**
* @internal prvTgfPxPortModeSpeedSet function
* @endinternal
*
* @brief   Configure Interface mode and speed on a specified port and execute
*          on port's serdeses power down and power up sequence.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfPxPortModeSpeedSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS rc;
    CPSS_PORTS_BMP_STC portsBmp;

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portNum);

    if ((ifMode >= CPSS_PORT_INTERFACE_MODE_NA_E)
        || (speed >= CPSS_PORT_SPEED_NA_E))
    {
        /* power off only */
        ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
        speed  = CPSS_PORT_SPEED_NA_E;
    }
    rc = cpssPxPortModeSpeedSet(
        devNum, &portsBmp, GT_FALSE/*powerUp*/, ifMode, speed);
    if (GT_OK != rc) return rc;

    if ((ifMode >= CPSS_PORT_INTERFACE_MODE_NA_E)
        || (speed >= CPSS_PORT_SPEED_NA_E))
    {
        return GT_OK; /* power off */
    }
    return cpssPxPortModeSpeedSet(
        devNum, &portsBmp, GT_TRUE/*powerUp*/, ifMode, speed);
}


