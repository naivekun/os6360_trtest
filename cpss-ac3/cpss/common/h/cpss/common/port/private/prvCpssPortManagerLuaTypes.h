/********************************************************************************
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
* @file prvCpssPortManagerLuaTypes.h.h
 *
 * @brief CPSS implementation for Port management LUA wrappers.
 *
* @version   1
********************************************************************************
*/
#include <cpss/common/port/cpssPortManager.h>

#ifndef __prvCpssPortManagerLuaTypes
#define __prvCpssPortManagerLuaTypes

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct{

    int dummy;
    /** Mandatory attributes */

    /** @brief Size of the attributes array
     *  modeParamsArr    - Array of attributes data to override the CPSS defaults with
     *  Comments:
     *  None.
     */

} PRV_CPSS_PORT_MANAGER_MODE_PARAMS_STC_L;

/**
* @struct CPSS_PORT_MANAGER_GENERIC_AP_PORT_PARAMS_STC
 *
 * @brief This structure contains all configurable attributes for an AP port.
*/
typedef struct{

    int dummy;


} PRV_CPSS_PORT_MANAGER_GENERIC_AP_PORT_PARAMS_STC_L;

/**
* @struct CPSS_PORT_MANAGER_GENERIC_PORT_PARAMS_STC
 *
 * @brief This structure contains all configurable attributes for a non-AP port.
*/
typedef struct{

    /** Mode parameters for the port */
    int dummy;
    /** @brief Size of the attributes array
     *  portAttributesArr   - Array of Non-AP-only attributes data to override the CPSS defaults with
     *  Comments:
     *  None.
     */

} PRV_CPSS_PORT_MANAGER_GENERIC_PORT_PARAMS_STC_L;

/**
* @struct CPSS_PORT_MANAGER_GENERIC_PARAMS_STC
 *
 * @brief This structure contains all configurable attributes for a given port.
*/
typedef struct{
    CPSS_PORT_MANAGER_PORT_TYPE_ENT                      portType;

    union {
        PRV_CPSS_PORT_MANAGER_GENERIC_PORT_PARAMS_STC_L       port;
        PRV_CPSS_PORT_MANAGER_GENERIC_AP_PORT_PARAMS_STC_L    apPort;
    } portParams;

} PRV_CPSS_PORT_MANAGER_GENERIC_PARAMS_STC_L;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPortManagerLuaTypes */
