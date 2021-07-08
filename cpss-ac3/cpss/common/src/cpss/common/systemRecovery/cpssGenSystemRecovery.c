/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file cpssGenSystemRecovery.c
*
* @brief Generic System Recovery ApIs
*
* @version   9
********************************************************************************
*/
#ifdef CHX_FAMILY
#define CPSS_LOG_IN_MODULE_ENABLE
#endif /*CHX_FAMILY*/

#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/config/private/prvCpssGenIntDefs.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecoveryLog.h>


/* define the HSU  global DB */
CPSS_SYSTEM_RECOVERY_INFO_STC   systemRecoveryInfo;
PRV_CPSS_SYSTEM_RECOVERY_MSG_DISABLE_MODE_FUN     prvCpssSystemRecoveryMsgDisableModeHandleFuncPtr = NULL;
PRV_CPSS_SYSTEM_RECOVERY_CATCH_UP_FUN             prvCpssSystemRecoveryCatchUpHandleFuncPtr = NULL;

/**
* @internal internal_cpssSystemRecoveryStateSet function
* @endinternal
*
* @brief   This function inform cpss about system recovery state
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] systemRecoveryInfoPtr    - pointer to system recovery information
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on bad systemRecoveryState
*/
static GT_STATUS internal_cpssSystemRecoveryStateSet
(
    IN CPSS_SYSTEM_RECOVERY_INFO_STC   *systemRecoveryInfoPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_NULL_PTR_CHECK_MAC(systemRecoveryInfoPtr);
    switch(systemRecoveryInfoPtr->systemRecoveryState)
    {
    case CPSS_SYSTEM_RECOVERY_PREPARATION_STATE_E:
    case CPSS_SYSTEM_RECOVERY_INIT_STATE_E:
    case CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E:
    case CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E:
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    systemRecoveryInfo.systemRecoveryState = systemRecoveryInfoPtr->systemRecoveryState;
    systemRecoveryInfo.systemRecoveryMode  = systemRecoveryInfoPtr->systemRecoveryMode;
    systemRecoveryInfo.systemRecoveryProcess = systemRecoveryInfoPtr->systemRecoveryProcess;

    switch(systemRecoveryInfoPtr->systemRecoveryProcess)
    {
    case CPSS_SYSTEM_RECOVERY_PROCESS_HSU_E:
        if (systemRecoveryInfoPtr->systemRecoveryMode.haCpuMemoryAccessBlocked == GT_TRUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if ( ((systemRecoveryInfoPtr->systemRecoveryMode.continuousRx == GT_FALSE)          ||
              (systemRecoveryInfoPtr->systemRecoveryMode.continuousTx == GT_FALSE)          ||
              (systemRecoveryInfoPtr->systemRecoveryMode.continuousAuMessages == GT_FALSE)  ||
              (systemRecoveryInfoPtr->systemRecoveryMode.continuousFuMessages == GT_FALSE) ) &&
              (systemRecoveryInfo.systemRecoveryState == CPSS_SYSTEM_RECOVERY_PREPARATION_STATE_E) )
        {

               /* make specific to this mode arrangements for RX,TX,AU,FU queues*/
            CPSS_FUNCPTR_CALL_NULL_COMPARED_MAC(prvCpssSystemRecoveryMsgDisableModeHandleFuncPtr,rc);

        }
        break;
    case CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E:
        if ( (systemRecoveryInfoPtr->systemRecoveryMode.haCpuMemoryAccessBlocked == GT_TRUE) ||
             (systemRecoveryInfoPtr->systemRecoveryMode.continuousRx == GT_TRUE)             ||
             (systemRecoveryInfoPtr->systemRecoveryMode.continuousTx == GT_TRUE)             ||
             (systemRecoveryInfoPtr->systemRecoveryMode.continuousAuMessages == GT_TRUE)     ||
             (systemRecoveryInfoPtr->systemRecoveryMode.continuousFuMessages == GT_TRUE))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if (systemRecoveryInfo.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E)
        {

              CPSS_FUNCPTR_CALL_NULL_COMPARED_MAC(prvCpssSystemRecoveryCatchUpHandleFuncPtr,rc);
        }
        break;

    case CPSS_SYSTEM_RECOVERY_PROCESS_HA_E:
        if ( (systemRecoveryInfoPtr->systemRecoveryMode.haCpuMemoryAccessBlocked == GT_TRUE) &&
             ((systemRecoveryInfoPtr->systemRecoveryMode.continuousRx == GT_TRUE)         ||
              (systemRecoveryInfoPtr->systemRecoveryMode.continuousTx == GT_TRUE)         ||
              (systemRecoveryInfoPtr->systemRecoveryMode.continuousAuMessages == GT_TRUE) ||
              (systemRecoveryInfoPtr->systemRecoveryMode.continuousFuMessages == GT_TRUE)) )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
        if (systemRecoveryInfo.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E)
        {
             CPSS_FUNCPTR_CALL_NULL_COMPARED_MAC(prvCpssSystemRecoveryCatchUpHandleFuncPtr,rc);
        }
        break;
    case CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E:
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (systemRecoveryInfo.systemRecoveryState == CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E)
    {
        /* it means that system recovery process is finished  */
        systemRecoveryInfo.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E;
        systemRecoveryInfo.systemRecoveryMode.continuousAuMessages = GT_FALSE;
        systemRecoveryInfo.systemRecoveryMode.continuousFuMessages = GT_FALSE;
        systemRecoveryInfo.systemRecoveryMode.continuousRx = GT_FALSE;
        systemRecoveryInfo.systemRecoveryMode.continuousTx = GT_FALSE;
        systemRecoveryInfo.systemRecoveryMode.haCpuMemoryAccessBlocked = GT_FALSE;
    }

    return rc;
}

/**
* @internal cpssSystemRecoveryStateSet function
* @endinternal
*
* @brief   This function inform cpss about system recovery state
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] systemRecoveryInfoPtr    - pointer to system recovery information
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on bad systemRecoveryState
*/
GT_STATUS cpssSystemRecoveryStateSet
(
    IN CPSS_SYSTEM_RECOVERY_INFO_STC   *systemRecoveryInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssSystemRecoveryStateSet);

    CPSS_ZERO_LEVEL_API_LOCK_MAC;
    CPSS_LOG_API_ENTER_MAC((funcId, systemRecoveryInfoPtr));

    rc = internal_cpssSystemRecoveryStateSet(systemRecoveryInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, systemRecoveryInfoPtr));
    CPSS_ZERO_LEVEL_API_UNLOCK_MAC;

    return rc;
}

/**
* @internal internal_cpssSystemRecoveryStateGet function
* @endinternal
*
* @brief   This function retrieve system recovery information
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @param[out] systemRecoveryInfoPtr    - pointer to system recovery
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on bad systemRecoveryState.
*/
static GT_STATUS internal_cpssSystemRecoveryStateGet
(
    OUT CPSS_SYSTEM_RECOVERY_INFO_STC   *systemRecoveryInfoPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_NULL_PTR_CHECK_MAC(systemRecoveryInfoPtr);

    *systemRecoveryInfoPtr = systemRecoveryInfo;

    return rc;
}

/**
* @internal cpssSystemRecoveryStateGet function
* @endinternal
*
* @brief   This function retrieve system recovery information
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @param[out] systemRecoveryInfoPtr    - pointer to system recovery
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on bad systemRecoveryState.
*/
GT_STATUS cpssSystemRecoveryStateGet
(
    OUT CPSS_SYSTEM_RECOVERY_INFO_STC   *systemRecoveryInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssSystemRecoveryStateGet);

    CPSS_ZERO_LEVEL_API_LOCK_MAC;
    CPSS_LOG_API_ENTER_MAC((funcId, systemRecoveryInfoPtr));

    rc = internal_cpssSystemRecoveryStateGet(systemRecoveryInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, systemRecoveryInfoPtr));
    CPSS_ZERO_LEVEL_API_UNLOCK_MAC;

    return rc;
}

