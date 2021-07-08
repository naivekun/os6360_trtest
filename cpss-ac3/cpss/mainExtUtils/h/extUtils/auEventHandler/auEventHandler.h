/*******************************************************************************
*                Copyright 2015, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file auEventHandler.h
*
* @brief This module defines the API for CPSS FDB AU events
*
* @version   1
********************************************************************************
*/
#ifndef __auEventHandler_h__
#define __auEventHandler_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgFdb.h>

#define AU_EV_HANDLER_DEVNUM_ALL 0xff

/**
* @enum AU_EV_HANDLER_EV_TYPE_ENT
 *
 * @brief Event type
*/
typedef enum{

    /** CPSS_NA_E events */
    AU_EV_HANDLER_TYPE_NA_E = CPSS_NA_E,

    /** @brief CPSS_QA_E events
     *  ...
     */
    AU_EV_HANDLER_TYPE_QA_E = CPSS_QA_E,

    AU_EV_HANDLER_TYPE_QR_E = CPSS_QR_E,

    AU_EV_HANDLER_TYPE_AA_E = CPSS_AA_E,

    AU_EV_HANDLER_TYPE_TA_E = CPSS_TA_E,

    AU_EV_HANDLER_TYPE_SA_E = CPSS_SA_E,

    AU_EV_HANDLER_TYPE_QI_E = CPSS_QI_E,

    AU_EV_HANDLER_TYPE_FU_E = CPSS_FU_E,

    AU_EV_HANDLER_TYPE_HR_E = CPSS_HR_E,

    /** any type of events */
    AU_EV_HANDLER_TYPE_ANY_E

} AU_EV_HANDLER_EV_TYPE_ENT;

/*******************************************************************************
* AU_EV_CALLBACK_FUNC
*
* DESCRIPTION:
*       Function called to handle incoming Rx packet in the CPU
*
* APPLICABLE DEVICES: ALL Devices
*
* INPUTS:
*       cookie       - a cookie passed to auEventHandlerAddCallback()
*       evType       - event type
*       devNum       - Device number.
*       auMessagePtr - pointer to the AUQ message
*
* RETURNS:
*       GT_OK - no error
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*AU_EV_CALLBACK_FUNC)
(
  IN  GT_UINTPTR                   cookie,
  IN  AU_EV_HANDLER_EV_TYPE_ENT    evType,
  IN  GT_U8                        devNum,
  IN  CPSS_MAC_UPDATE_MSG_EXT_STC *auMessagePtr
);

/**
* @internal auEventHandlerLibInit function
* @endinternal
*
* @brief   Initialize library
*/
GT_STATUS auEventHandlerLibInit(GT_VOID);

/**
* @internal auEventHandlerInitHandler function
* @endinternal
*
* @brief   Initialize event handler for Rx/RxErr events
*         auEventHandlerDoCallbacks() must be called from the same process
*         (another thread of this process allowed)
* @param[in] devNum                   - the device number or AU_EV_HANDLER_DEVNUM_ALL
*                                      type    - event type
*
* @param[out] evHandlePtr              - handle
*                                       GT_OK on success
*                                       GT_ALREADY_EXIST success, already initialized
*                                       GT_BAD_PARAM
*                                       GT_FAIL
*/
GT_STATUS auEventHandlerInitHandler(
  IN  GT_U8                     devNum,
  IN  AU_EV_HANDLER_EV_TYPE_ENT evType,
  OUT GT_UINTPTR                *evHandlePtr
);

/**
* @internal auEventHandlerAddCallback function
* @endinternal
*
* @brief   Add callback function for Rx events
*
* @param[in] devNum                   - the device number or AU_EV_HANDLER_DEVNUM_ALL
*                                      type    - event type
* @param[in] cbFunc                   - callback function
* @param[in] cookie                   -  value to be passed to callback function
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS auEventHandlerAddCallback(
  IN  GT_U8                         devNum,
  IN  AU_EV_HANDLER_EV_TYPE_ENT     evType,
  IN  AU_EV_CALLBACK_FUNC           cbFunc,
  IN  GT_UINTPTR                    cookie
);

/**
* @internal auEventHandlerRemoveCallback function
* @endinternal
*
* @brief   Add callback function for Rx events
*
* @param[in] devNum                   - the device number or AU_EV_HANDLER_DEVNUM_ALL
* @param[in] evType                   - event type
* @param[in] cbFunc                   - callback function
* @param[in] cookie                   -  passed to auEventHandlerAddCallback()
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS auEventHandlerRemoveCallback(
  IN  GT_U8                         devNum,
  IN  AU_EV_HANDLER_EV_TYPE_ENT     evType,
  IN  AU_EV_CALLBACK_FUNC           cbFunc,
  IN  GT_UINTPTR                    cookie
);


/**
* @internal auEventHandlerDoCallbacks function
* @endinternal
*
* @brief   Execute all callback routines for event handler
*
* @param[in] handle                   - the of CPSS event handler
* @param[in] evType                   - event type
* @param[in] devNum                   - Device number.
* @param[in] auMessagePtr             - pointer to the AUQ message
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS auEventHandlerDoCallbacks
(
    IN GT_UINTPTR                   handle,
    IN AU_EV_HANDLER_EV_TYPE_ENT    evType,
    IN GT_U8                        devNum,
    IN CPSS_MAC_UPDATE_MSG_EXT_STC *auMessagePtr
);

#ifdef __cplusplus
}
#endif

#endif /* __auEventHandler_h__ */

