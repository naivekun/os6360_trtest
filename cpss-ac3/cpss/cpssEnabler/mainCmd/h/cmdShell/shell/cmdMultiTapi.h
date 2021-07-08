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
* @file cmdMultiTapi.h
*
* @brief MultiTapi definitions
*
*
* @version   3
********************************************************************************
*/
#ifndef __cmdMultiTapih
#define __cmdMultiTapih

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***** Include files ***************************************************/

#include <cmdShell/common/cmdCommon.h>
#if (!defined  CPSS_PRODUCT) && (!defined PSS_PRODUCT)
#  include <cpss/common/cpssTypes.h>/* this include for cpss only */
#else
#  include <prestera/common/gtLinkedList.h>
#  include <prestera/os/gtTypes.h>
#endif

/* default unit for Galtis commands execution */
#define DEFAULT_UNIT 0

/**
* @struct GT_UNIT_IP_INFO_STC
 *
 * @brief Stores IP address of a unit.
*/
typedef struct{

    /** unit number */
    GT_U8 unitNum;

    /** unit IP address */
    GT_IPADDR ipAddr;

} GT_UNIT_IP_INFO_STC;

/**
* @internal cmdGetIpForUnit function
* @endinternal
*
* @brief   Get the IP address of a specified unit.
*
* @param[in] unitNum                  - Number of unit.
*
* @param[out] ipAddr                   - The IP address of the unit.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_FOUND             - IP address not found for the unit.
*/
GT_STATUS cmdGetIpForUnit
(
    IN  GT_U8    unitNum,
    OUT GT_IPADDR *ipAddr
);

/**
* @internal cmdSetIpForUnit function
* @endinternal
*
* @brief   Set the IP address of a specified unit.
*
* @param[in] unitNum                  - Number of unit.
* @param[in] ipAddr                   - The IP address of the unit.
*
* @retval GT_OK                    - on success.
* @retval GT_ALREADY_EXIST         - unit or IP address already exist.
* @retval GT_BAD_PARAM             - illegal unit number.
* @retval GT_SET_ERROR             - unable to set IP address because system has
*                                       been already initialized.
* @retval GT_BAD_STATE             - command is not executed on valid unit.
* @retval GT_ABORTED               - operation aborted due to failure in connecting to
*                                       the new unit.
* @retval GT_FAIL                  - otherwise.
*
* @note It's recommended to use this API only before all the system units are
*       initialized.
*
*/
GT_STATUS cmdSetIpForUnit
(
    IN  GT_U8     unitNum,
    IN  GT_IPADDR ipAddr
);

/**
* @internal cmdIsMultiTapiSystem function
* @endinternal
*
* @brief   Get information on whether the system has more than one TAPI.
*
* @retval GT_TRUE                  - System is multi-TAPI.
* @retval GT_FALSE                 - System is single-TAPI.
*/
GT_BOOL cmdIsMultiTapiSystem
(
    GT_VOID
);

/**
* @internal cmdGetUnitsNumber function
* @endinternal
*
* @brief   Get the number of units in the system.
*/
GT_U8 cmdGetUnitsNumber
(
    GT_VOID
);

/**
* @internal cmdIsUnitExist function
* @endinternal
*
* @brief   Check if a specific unit exists in the database.
*
* @param[in] unit                     - Unit number.
*
* @retval GT_TRUE                  - The unit exists.
* @retval GT_FALSE                 - The unit doesn't exist.
*/
GT_BOOL cmdIsUnitExist
(
    IN GT_U8 unit
);

/**
* @internal cmdSwitchUnit function
* @endinternal
*
* @brief   Implementation of the switching parser - switch the active unit.
*
* @param[in] selectedUnit             - selected unit to work with.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_FOUND             - the unit doesn't exist.
*/
GT_STATUS cmdSwitchUnit
(
    IN GT_U8 selectedUnit
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cmdMultiTapih */


