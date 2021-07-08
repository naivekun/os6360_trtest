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
* @file cpssDxChCatchUp.c
*
* @brief CPSS DxCh CatchUp functions.
*
* @version   1
********************************************************************************
*/



#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/config/private/prvCpssGenIntDefs.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>


/**
* @internal cpssDxChCatchUpPortModeDbSet function
* @endinternal
*
* @brief   This API fixes port interface mode value in software DB for specified
*         port during system recovery process
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
* @param[in] ifMode                   - Interface mode
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_STATE             - on not within system recovery process
*
* @note API should be called only within recovery process, otherwise
*       GT_BAD_STATE is returned
*       There are cases in which the Catchup can't decide explicity the Port
*       Interface Mode, so port interface mode is updated in the CPSS port
*       database with wrong value.
*       For example: KR and SR/LR. Catchup can not distinguish between the 2 modes,
*       since the parameters in HW may be the same for both Port Interface Modes.
*       In such cases application involvment is needed in order to update CPSS
*       Port database.
*       This API updates CPSS Port Datadase with port interface mode values.
*
*/
GT_STATUS cpssDxChCatchUpPortModeDbSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         port,
    IN CPSS_PORT_INTERFACE_MODE_ENT ifMode
)
{
    GT_STATUS              rc = GT_OK;
    GT_U32                 portMac;
    CPSS_PORT_SPEED_ENT    speed;

    /* Check recovery process*/
    if((systemRecoveryInfo.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
       (systemRecoveryInfo.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* check device*/
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    /* check port*/
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, port, portMac);

    speed = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,portMac);

    /* Currently the only interface needed to be fixed is SR/LR and KR*/
    if((ifMode != CPSS_PORT_INTERFACE_MODE_KR_E) && (ifMode != CPSS_PORT_INTERFACE_MODE_CR_E)&&
       (ifMode != CPSS_PORT_INTERFACE_MODE_CR_S_E) && (ifMode != CPSS_PORT_INTERFACE_MODE_CR_C_E)&&
       (ifMode != CPSS_PORT_INTERFACE_MODE_KR_S_E) && (ifMode != CPSS_PORT_INTERFACE_MODE_KR_C_E)&&
       (ifMode != CPSS_PORT_INTERFACE_MODE_SR_LR_E) &&
       !(ifMode == CPSS_PORT_INTERFACE_MODE_CR2_E && speed == CPSS_PORT_SPEED_40000_E) &&
       !(ifMode == CPSS_PORT_INTERFACE_MODE_CR2_E && speed == CPSS_PORT_SPEED_50000_E) &&
       !(ifMode == CPSS_PORT_INTERFACE_MODE_CR2_C_E && speed == CPSS_PORT_SPEED_40000_E) &&
       !(ifMode == CPSS_PORT_INTERFACE_MODE_CR2_C_E && speed == CPSS_PORT_SPEED_50000_E) &&
       !(ifMode == CPSS_PORT_INTERFACE_MODE_SR_LR2_E && speed == CPSS_PORT_SPEED_40000_E) &&
       !(ifMode == CPSS_PORT_INTERFACE_MODE_SR_LR2_E && speed == CPSS_PORT_SPEED_50000_E) &&
       !(ifMode == CPSS_PORT_INTERFACE_MODE_KR4_E && speed == CPSS_PORT_SPEED_40000_E) &&
       !(ifMode == CPSS_PORT_INTERFACE_MODE_KR4_E && speed == CPSS_PORT_SPEED_100G_E) &&
       !(ifMode == CPSS_PORT_INTERFACE_MODE_CR4_E && speed == CPSS_PORT_SPEED_40000_E)&&
       !(ifMode == CPSS_PORT_INTERFACE_MODE_CR4_E && speed == CPSS_PORT_SPEED_100G_E) &&
       !(ifMode == CPSS_PORT_INTERFACE_MODE_SR_LR4_E && speed == CPSS_PORT_SPEED_40000_E)&&
       !(ifMode == CPSS_PORT_INTERFACE_MODE_SR_LR4_E && speed == CPSS_PORT_SPEED_100G_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMac].portIfMode = ifMode;

    if (CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        if(speed != CPSS_PORT_SPEED_NA_E)
        {
            rc = prvCpssDxChLion2PortTypeSet(devNum, port, ifMode, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
    }
    else
    {
        prvCpssDxChPortTypeSet(devNum, port, ifMode, speed);
    }

    return GT_OK;
}

