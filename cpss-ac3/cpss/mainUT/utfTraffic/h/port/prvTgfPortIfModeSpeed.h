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
* @file prvTgfPortIfModeSpeed.h
*
* @brief Port interface mode and speed configuration testing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPortIfModeSpeedh
#define __prvTgfPortIfModeSpeedh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfPortGen.h>

/**
* @internal prvTgfPortIfModeSpeed function
* @endinternal
*
* @brief   Run test over all ports and supported (interface;speed) pairs
*/
extern GT_VOID prvTgfPortIfModeSpeed
(
    GT_VOID
);

/**
* @internal prvTgfPortIfModeSpeedPortTest function
* @endinternal
*
* @brief   Run test per port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                       None
*/
extern GT_STATUS prvTgfPortIfModeSpeedPortTest
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum
);

/**
* @internal prvTgfPortIfModeSpeedPortModeTest function
* @endinternal
*
* @brief   Run test per port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*                                       None
*/
extern GT_STATUS prvTgfPortIfModeSpeedPortModeTest
(
    GT_U8                           devNum,
    GT_U8                           portNum,
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    CPSS_PORT_SPEED_ENT             speed
);

/**
* @internal prvTgfPortIfModeSpeedPortConfig function
* @endinternal
*
* @brief   Set port configuration speed and mode
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - number of port
* @param[in] ifMode                   - mode to set
* @param[in] speed                    -  to set
*                                       None
*/
GT_STATUS prvTgfPortIfModeSpeedPortConfig
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

/**
* @internal prvTgfPortExtendedModeTest function
* @endinternal
*
* @brief   Run test extended port
*/
GT_VOID prvTgfPortExtendedModeTest
(
    GT_PHYSICAL_PORT_NUM  port
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPortIfModeSpeedh */



