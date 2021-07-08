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
* @file cpssDxChPortCn.c
*
* @brief CPSS implementation for Port Congestion Notification API.
*
* @version   50
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCn.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPort.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChMacCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChTables.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxDba.h>

/**
* @internal internal_cpssDxChPortCnModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable Congestion Notification (CN) mechanism.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - Enable/disable congestion notification mechanism.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Do not use the global enable/disable CCFC under traffic - instead use
*       cpssDxChPortCnQueueAwareEnableSet to enable/disable queue for BCN.
*
*/
static GT_STATUS internal_cpssDxChPortCnModeEnableSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_PORT_CN_MODE_ENT  enable
)
{
    GT_STATUS rc;               /* function return value */
    GT_U32    regAddr;          /* register address */
    GT_U32    regOffset;        /* register offset */
    GT_U32    controlRegVal;    /* BCN Control register value */
    GT_U32    enableBit;        /* boolean representation of the mode */
    GT_U32    value;            /* register field value */
    GT_U32    frequency;        /* clock frequency */
    GT_U32    sdqUnit;
    GT_U32    tileOffset;
    GT_U32    tileNum;
    GT_U32    numberOfTiles;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    switch(enable)
    {
        case CPSS_DXCH_PORT_CN_MODE_DISABLE_E:
            enableBit = 0;
            value     = 0;
        break;

        case CPSS_DXCH_PORT_CN_MODE_ENABLE_E:
            enableBit = 1;
            value     = 1;
        break;

        case CPSS_DXCH_PORT_CN_MODE_REACTION_POINT_E:
            if(! PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            enableBit = 1;
            value     = 2;
        break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((PRV_CPSS_SIP_6_CHECK_MAC(devNum)) && enableBit)
    {
        if(PRV_CPSS_PP_MAC(devNum)->coreClock >= 512)
        {
            frequency = 3;
        }
        else if (PRV_CPSS_PP_MAC(devNum)->coreClock >= 384)
        {
            frequency = 2;
        }
        else if (PRV_CPSS_PP_MAC(devNum)->coreClock >= 256)
        {
            frequency = 1;
        }
        else
        {
            frequency = 0;
        }

        switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
        {
            case 0:
            case 1:
                numberOfTiles = 1;
                break;
           case 2:
           case 4:
                numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
                break;

        }
        for(tileNum = 0; tileNum < numberOfTiles; tileNum++)
        {
            for(sdqUnit = 0; sdqUnit < MAX_DP_IN_TILE(devNum); sdqUnit++)
            {
                regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqUnit].QCN_Config;
                tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum/*tileId*/);
                regAddr += tileOffset;
                rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 2, frequency);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
            {
                regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.globalTailDropConfig;
                regOffset = (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) ? 10 : 9;
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    txqVer1.queue.congestNotification.cnGlobalConfigReg;
                regOffset = 0;
            }

            /* set CnEn */
            rc = prvCpssHwPpSetRegField(devNum, regAddr, regOffset, 1, enableBit);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfig;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.ttiUnitConfig;
        }

        /* set CNM Termination Mode in TTI */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 18, 2, value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        if (1 == enableBit)
        {
            controlRegVal = 0xD;
        }
        else
        {
            controlRegVal = 0x0;
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bcnRegs.bcnControlReg;

        /* set BCN Control register */
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 4, controlRegVal);
        if (rc != GT_OK)
        {
            return rc;
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.ttiUnitConfig;

        /* set Enable/Disable identification of CCFC frames in TTI */
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 19, 1, enableBit);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortCnModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable Congestion Notification (CN) mechanism.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - Enable/disable congestion notification mechanism.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Do not use the global enable/disable CCFC under traffic - instead use
*       cpssDxChPortCnQueueAwareEnableSet to enable/disable queue for BCN.
*
*/
GT_STATUS cpssDxChPortCnModeEnableSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_PORT_CN_MODE_ENT  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnModeEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChPortCnModeEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnModeEnableGet function
* @endinternal
*
* @brief   Gets the current status of Congestion Notification (CN) mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) status of Congestion Notification mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnModeEnableGet
(
    IN  GT_U8    devNum,
    OUT CPSS_DXCH_PORT_CN_MODE_ENT  *enablePtr
)
{
    GT_STATUS rc;        /* function return value */
    GT_U32    regAddr;   /* register address */
    GT_U32    value;     /* register field value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfig;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.ttiUnitConfig;
        }

        /* set CNM Termination Mode in TTI */
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 18, 2, &value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.ttiUnitConfig;

        /* Get Enable/Disable identification of CCFC frames in TTI */
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 19, 1, &value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    switch(value)
    {
        case 0:
            *enablePtr = CPSS_DXCH_PORT_CN_MODE_DISABLE_E;
        break;

        case 1:
            *enablePtr = CPSS_DXCH_PORT_CN_MODE_ENABLE_E;
        break;

        case 2:
            if(! PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                /* should not happen */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            *enablePtr = CPSS_DXCH_PORT_CN_MODE_REACTION_POINT_E;
        break;

        default:
            /* should not happen */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortCnModeEnableGet function
* @endinternal
*
* @brief   Gets the current status of Congestion Notification (CN) mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) status of Congestion Notification mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnModeEnableGet
(
    IN  GT_U8    devNum,
    OUT CPSS_DXCH_PORT_CN_MODE_ENT  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnModeEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChPortCnModeEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnProfileSet function
* @endinternal
*
* @brief   Binds a port to a Congestion Notification profile.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] profileIndex             - profile index (0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range profile index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. To disable port for triggering CN frames bind port to profile with
*       all thresholds set to maximum.
*       2. On Lion device CN Profile is part of the Tail-Drop Profile.
*       To bind port to Tail-Drop Profile use cpssDxChPortTxBindPortToDpSet.
*
*/
static GT_STATUS internal_cpssDxChPortCnProfileSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32   profileIndex
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 fieldOffset; /* the start bit number in the register */
    GT_U32 portGroupId; /* the port group Id - support multi-port-groups device */
    GT_U32  localPort;   /* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

    if (profileIndex > 7 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bcnRegs.
                portsBcnProfileReg0 + 4 * (localPort / 10);

    fieldOffset = (localPort % 10) * 3;

    return prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                     fieldOffset, 3, profileIndex);
}

/**
* @internal cpssDxChPortCnProfileSet function
* @endinternal
*
* @brief   Binds a port to a Congestion Notification profile.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] profileIndex             - profile index (0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range profile index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. To disable port for triggering CN frames bind port to profile with
*       all thresholds set to maximum.
*       2. On Lion device CN Profile is part of the Tail-Drop Profile.
*       To bind port to Tail-Drop Profile use cpssDxChPortTxBindPortToDpSet.
*
*/
GT_STATUS cpssDxChPortCnProfileSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32   profileIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnProfileSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, profileIndex));

    rc = internal_cpssDxChPortCnProfileSet(devNum, portNum, profileIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, profileIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnProfileGet function
* @endinternal
*
* @brief   Gets the port's Congestion Notification profile.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] profileIndexPtr          - (pointer to) profile index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnProfileGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32  *profileIndexPtr
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 fieldOffset; /* the start bit number in the register */
    GT_U32 portGroupId; /* the port group Id - support multi-port-groups device */
    GT_U32  localPort;   /* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(profileIndexPtr);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bcnRegs.
                portsBcnProfileReg0 + 4 * (localPort / 10);

    fieldOffset = (localPort % 10) * 3;

    return prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                     fieldOffset, 3, profileIndexPtr);
}

/**
* @internal cpssDxChPortCnProfileGet function
* @endinternal
*
* @brief   Gets the port's Congestion Notification profile.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] profileIndexPtr          - (pointer to) profile index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnProfileGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32  *profileIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnProfileGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, profileIndexPtr));

    rc = internal_cpssDxChPortCnProfileGet(devNum, portNum, profileIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, profileIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnProfileQueueThresholdSet function
* @endinternal
*
* @brief   Sets the buffer threshold for triggering CN frame for a given tc queue.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - profile index (0..7).
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] threshold                - buffer limit  for triggering CN frame on a given
*                                      tc queue (0..0x1FFF).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To bind port to CN profile in Lion use
*       cpssDxChPortCnProfileQueueConfigSet.
*
*/
static GT_STATUS internal_cpssDxChPortCnProfileQueueThresholdSet
(
    IN GT_U8    devNum,
    IN GT_U32   profileIndex,
    IN GT_U8    tcQueue,
    IN GT_U32   threshold
)
{
    GT_STATUS rc;    /* function return value */
    GT_U32 offset;   /* offset from the beginning of the entry */
    GT_U32 temp;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tcQueue);

    if (profileIndex > 7 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (threshold > 0x1FFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    offset = tcQueue * 13;

    temp = 32 - offset%32;

    if (temp >= 13)
    {
        return prvCpssDxChWriteTableEntryField(devNum,CPSS_DXCH_XCAT_TABLE_BCN_PROFILES_E,
                                               profileIndex,offset /32 ,offset % 32,13,threshold);
    }
    else
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,CPSS_DXCH_XCAT_TABLE_BCN_PROFILES_E,
                                               profileIndex,offset /32 ,offset % 32,temp,threshold);
        if (rc != GT_OK)
        {
            return rc;
        }

        return prvCpssDxChWriteTableEntryField(devNum,CPSS_DXCH_XCAT_TABLE_BCN_PROFILES_E,
                                               profileIndex,(offset /32)+1 ,0,13 - temp,threshold>>temp);

    }
}

/**
* @internal cpssDxChPortCnProfileQueueThresholdSet function
* @endinternal
*
* @brief   Sets the buffer threshold for triggering CN frame for a given tc queue.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - profile index (0..7).
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] threshold                - buffer limit  for triggering CN frame on a given
*                                      tc queue (0..0x1FFF).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To bind port to CN profile in Lion use
*       cpssDxChPortCnProfileQueueConfigSet.
*
*/
GT_STATUS cpssDxChPortCnProfileQueueThresholdSet
(
    IN GT_U8    devNum,
    IN GT_U32   profileIndex,
    IN GT_U8    tcQueue,
    IN GT_U32   threshold
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnProfileQueueThresholdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileIndex, tcQueue, threshold));

    rc = internal_cpssDxChPortCnProfileQueueThresholdSet(devNum, profileIndex, tcQueue, threshold);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileIndex, tcQueue, threshold));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnProfileQueueThresholdGet function
* @endinternal
*
* @brief   Gets the buffer threshold for triggering CN frame for a given tc queue.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - profile index (0..7).
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] thresholdPtr             - (pointer to) buffer limit threshold for
*                                      triggering CN frame on a given tc queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnProfileQueueThresholdGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   profileIndex,
    IN  GT_U8    tcQueue,
    OUT GT_U32   *thresholdPtr
)
{
    GT_STATUS rc;     /* function return value */
    GT_U32 value;     /* register value */
    GT_U32 offset;
    GT_U32 temp;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tcQueue);
    CPSS_NULL_PTR_CHECK_MAC(thresholdPtr);

    if (profileIndex > 7 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    offset = tcQueue * 13;

    temp = 32 - offset%32;

    if (temp >= 13)
    {
        return prvCpssDxChReadTableEntryField(devNum,CPSS_DXCH_XCAT_TABLE_BCN_PROFILES_E,
                                              profileIndex,offset/32,offset%32,13,thresholdPtr);
    }
    else
    {
        rc = prvCpssDxChReadTableEntryField(devNum,CPSS_DXCH_XCAT_TABLE_BCN_PROFILES_E,
                                            profileIndex,offset/32,offset%32,temp,&value);
        if (rc != GT_OK)
        {
            return rc;
        }
        *thresholdPtr = value;
        rc = prvCpssDxChReadTableEntryField(devNum,CPSS_DXCH_XCAT_TABLE_BCN_PROFILES_E,
                                            profileIndex,(offset/32)+1,0,13-temp,&value);
        if (rc != GT_OK)
        {
            return rc;
        }
        *thresholdPtr |= (value << temp);
    }
    return GT_OK;
}

/**
* @internal cpssDxChPortCnProfileQueueThresholdGet function
* @endinternal
*
* @brief   Gets the buffer threshold for triggering CN frame for a given tc queue.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - profile index (0..7).
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] thresholdPtr             - (pointer to) buffer limit threshold for
*                                      triggering CN frame on a given tc queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnProfileQueueThresholdGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   profileIndex,
    IN  GT_U8    tcQueue,
    OUT GT_U32   *thresholdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnProfileQueueThresholdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileIndex, tcQueue, thresholdPtr));

    rc = internal_cpssDxChPortCnProfileQueueThresholdGet(devNum, profileIndex, tcQueue, thresholdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileIndex, tcQueue, thresholdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnProfileThresholdSet function
* @endinternal
*
* @brief   Sets the buffer threshold for triggering CN frame.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - profile index (0..7).
* @param[in] threshold                - buffer limit threshold
*                                      for triggering CN frame (0..0x1FFF).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile index
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnProfileThresholdSet
(
    IN GT_U8    devNum,
    IN GT_U32   profileIndex,
    IN GT_U32   threshold
)
{

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    if (profileIndex > 7 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (threshold > 0x1FFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChWriteTableEntryField(devNum,CPSS_DXCH_XCAT_TABLE_BCN_PROFILES_E,
                                              profileIndex,3,8,13,threshold);
}

/**
* @internal cpssDxChPortCnProfileThresholdSet function
* @endinternal
*
* @brief   Sets the buffer threshold for triggering CN frame.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - profile index (0..7).
* @param[in] threshold                - buffer limit threshold
*                                      for triggering CN frame (0..0x1FFF).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile index
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnProfileThresholdSet
(
    IN GT_U8    devNum,
    IN GT_U32   profileIndex,
    IN GT_U32   threshold
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnProfileThresholdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileIndex, threshold));

    rc = internal_cpssDxChPortCnProfileThresholdSet(devNum, profileIndex, threshold);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileIndex, threshold));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnProfileThresholdGet function
* @endinternal
*
* @brief   Gets the buffer threshold for triggering CN frame.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - profile index (0..7).
*
* @param[out] thresholdPtr             - (pointer to) buffer limit threshold for
*                                      triggering CN frame.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number or profile index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnProfileThresholdGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  profileIndex,
    OUT GT_U32  *thresholdPtr
)
{

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(thresholdPtr);

    if (profileIndex > 7 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChReadTableEntryField(devNum,CPSS_DXCH_XCAT_TABLE_BCN_PROFILES_E,
                                              profileIndex,3,8,13,thresholdPtr);
}

/**
* @internal cpssDxChPortCnProfileThresholdGet function
* @endinternal
*
* @brief   Gets the buffer threshold for triggering CN frame.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - profile index (0..7).
*
* @param[out] thresholdPtr             - (pointer to) buffer limit threshold for
*                                      triggering CN frame.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number or profile index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnProfileThresholdGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  profileIndex,
    OUT GT_U32  *thresholdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnProfileThresholdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileIndex, thresholdPtr));

    rc = internal_cpssDxChPortCnProfileThresholdGet(devNum, profileIndex, thresholdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileIndex, thresholdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnQueueAwareEnableSet function
* @endinternal
*
* @brief   Enable/Disable Congestion Notification awareness on a given
*         tc queue.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] enable                   - GT_TRUE: CN aware. Packets enqueued to tcQueue are enabled to
*                                      trigger Congestion Notification frames.
*                                      GT_FALSE: CN blind. Packets enqueued to tcQueue never trigger
*                                      Congestion Notification frames.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnQueueAwareEnableSet
(
    IN GT_U8    devNum,
    IN GT_U8    tcQueue,
    IN GT_BOOL  enable
)
{
    GT_U32 regAddr;   /* register address */
    GT_U32 value;     /* register value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tcQueue);

    value = BOOL2BIT_MAC(enable);
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bcnRegs.bcnControlReg;

    return prvCpssDrvHwPpSetRegField(devNum, regAddr, 4 + tcQueue, 1, value);
}

/**
* @internal cpssDxChPortCnQueueAwareEnableSet function
* @endinternal
*
* @brief   Enable/Disable Congestion Notification awareness on a given
*         tc queue.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] enable                   - GT_TRUE: CN aware. Packets enqueued to tcQueue are enabled to
*                                      trigger Congestion Notification frames.
*                                      GT_FALSE: CN blind. Packets enqueued to tcQueue never trigger
*                                      Congestion Notification frames.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnQueueAwareEnableSet
(
    IN GT_U8    devNum,
    IN GT_U8    tcQueue,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnQueueAwareEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tcQueue, enable));

    rc = internal_cpssDxChPortCnQueueAwareEnableSet(devNum, tcQueue, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tcQueue, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnQueueAwareEnableGet function
* @endinternal
*
* @brief   Gets the status of Congestion Notification awareness on a given
*         tc queue.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] enablePtr                - (pointer to) status of Congestion Notification awareness
*                                      GT_TRUE: CN aware. Packets enqueued to tcQueue are enabled to
*                                      trigger Congestion Notification frames.
*                                      GT_FALSE: CN blind. Packets enqueued to tcQueue never trigger
*                                      Congestion Notification frames.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnQueueAwareEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    tcQueue,
    OUT GT_BOOL  *enablePtr
)
{
    GT_U32 regAddr;   /* register address */
    GT_U32 value;     /* register value */
    GT_STATUS rc;     /* function return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tcQueue);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bcnRegs.bcnControlReg;

    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 4 + tcQueue, 1, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChPortCnQueueAwareEnableGet function
* @endinternal
*
* @brief   Gets the status of Congestion Notification awareness on a given
*         tc queue.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] enablePtr                - (pointer to) status of Congestion Notification awareness
*                                      GT_TRUE: CN aware. Packets enqueued to tcQueue are enabled to
*                                      trigger Congestion Notification frames.
*                                      GT_FALSE: CN blind. Packets enqueued to tcQueue never trigger
*                                      Congestion Notification frames.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnQueueAwareEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    tcQueue,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnQueueAwareEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tcQueue, enablePtr));

    rc = internal_cpssDxChPortCnQueueAwareEnableGet(devNum, tcQueue, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tcQueue, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnFrameQueueSet function
* @endinternal
*
* @brief   Set traffic class queue associated with CN frames generated by device.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] tcQueue                  - traffic class queue associated with CN frames (0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CN frames must be enqueued to priority queues disabled to trigger CN.
*       Bobcat2; Caelum; Bobcat3, Aldrin, AC3X - The device use 'traffic class' from qosProfileId set by
*       cpssDxChPortCnMessageGenerationConfigSet()
*
*/
static GT_STATUS internal_cpssDxChPortCnFrameQueueSet
(
    IN GT_U8    devNum,
    IN GT_U8    tcQueue
)
{
    GT_U32 regAddr;   /* register address */
    GT_U32 fieldOffset;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tcQueue);

    if(PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.queue.congestNotification.cnGlobalConfigReg;
        fieldOffset = 1;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bcnRegs.bcnControlReg;
        fieldOffset = 12;
    }

    return prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 3, tcQueue);
}

/**
* @internal cpssDxChPortCnFrameQueueSet function
* @endinternal
*
* @brief   Set traffic class queue associated with CN frames generated by device.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] tcQueue                  - traffic class queue associated with CN frames (0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CN frames must be enqueued to priority queues disabled to trigger CN.
*       Bobcat2; Caelum; Bobcat3, Aldrin, AC3X - The device use 'traffic class' from qosProfileId set by
*       cpssDxChPortCnMessageGenerationConfigSet()
*
*/
GT_STATUS cpssDxChPortCnFrameQueueSet
(
    IN GT_U8    devNum,
    IN GT_U8    tcQueue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnFrameQueueSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tcQueue));

    rc = internal_cpssDxChPortCnFrameQueueSet(devNum, tcQueue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tcQueue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnFrameQueueGet function
* @endinternal
*
* @brief   Get traffic class queue associated with CN frames generated by device.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] tcQueuePtr               - (pointer to) traffic class queue associated with CN frames.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnFrameQueueGet
(
    IN  GT_U8    devNum,
    OUT GT_U8    *tcQueuePtr
)
{
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */
    GT_U32 value;     /* register value */
    GT_U32 fieldOffset;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(tcQueuePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.queue.congestNotification.cnGlobalConfigReg;
        fieldOffset = 1;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bcnRegs.bcnControlReg;
        fieldOffset = 12;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, 3, &value);
    if (rc != GT_OK)
    {
        return rc;
    }
    *tcQueuePtr = (GT_U8)value;
    return GT_OK;
}

/**
* @internal cpssDxChPortCnFrameQueueGet function
* @endinternal
*
* @brief   Get traffic class queue associated with CN frames generated by device.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] tcQueuePtr               - (pointer to) traffic class queue associated with CN frames.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnFrameQueueGet
(
    IN  GT_U8    devNum,
    OUT GT_U8    *tcQueuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnFrameQueueGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tcQueuePtr));

    rc = internal_cpssDxChPortCnFrameQueueGet(devNum, tcQueuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tcQueuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnEtherTypeSet function
* @endinternal
*
* @brief   Sets the EtherType to identify CN frames.
*         This EtherType also inserted in the CN header of generated CN frames.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] etherType                - EtherType for CN frames (0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range etherType values.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The CN EtherType must be unique in the system.
*
*/
static GT_STATUS internal_cpssDxChPortCnEtherTypeSet
(
    IN GT_U8    devNum,
    IN GT_U32   etherType
)
{
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    if(etherType > 0xFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).CNAndFCConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.ccfcEthertype;
    }
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 16, etherType);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).CNMHeaderConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltCnmHeaderConfig;
    }

    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 16, etherType);
}

/**
* @internal cpssDxChPortCnEtherTypeSet function
* @endinternal
*
* @brief   Sets the EtherType to identify CN frames.
*         This EtherType also inserted in the CN header of generated CN frames.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] etherType                - EtherType for CN frames (0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range etherType values.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The CN EtherType must be unique in the system.
*
*/
GT_STATUS cpssDxChPortCnEtherTypeSet
(
    IN GT_U8    devNum,
    IN GT_U32   etherType
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnEtherTypeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, etherType));

    rc = internal_cpssDxChPortCnEtherTypeSet(devNum, etherType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, etherType));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnEtherTypeGet function
* @endinternal
*
* @brief   Gets the EtherType for CN frames. The EtherType inserted in the CN
*         header.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
*
* @param[out] etherTypePtr             - (pointer to) EtherType for CN frames.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnEtherTypeGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *etherTypePtr
)
{
    GT_U32    regAddr;   /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    CPSS_NULL_PTR_CHECK_MAC(etherTypePtr);

    /* we are reading only ccfcEthertype register because it reflects the
       value in the hdrAltCnmHeaderConfig as well */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).CNAndFCConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.ccfcEthertype;
    }
    return prvCpssHwPpGetRegField(devNum, regAddr, 0, 16, etherTypePtr);
}

/**
* @internal cpssDxChPortCnEtherTypeGet function
* @endinternal
*
* @brief   Gets the EtherType for CN frames. The EtherType inserted in the CN
*         header.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
*
* @param[out] etherTypePtr             - (pointer to) EtherType for CN frames.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnEtherTypeGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *etherTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnEtherTypeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, etherTypePtr));

    rc = internal_cpssDxChPortCnEtherTypeGet(devNum, etherTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, etherTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnSpeedIndexSet function
* @endinternal
*
* @brief   Sets port speed index of the egress port. The device inserts this index
*         in the generated CCFC frame.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] portSpeedIndex           - port speed index (0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range port speed index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Panic Pause uses Port Speed Index 0 and 7 to indicate XON and XOFF.
*       Congested ports should use Port Speed Index 1 to 6 if
*       Panic Pause is used.
*
*/
static GT_STATUS internal_cpssDxChPortCnSpeedIndexSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32   portSpeedIndex
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 fieldOffset; /* the start bit number in the register */
    GT_U32 portGroupId; /* the port group Id - support multi-port-groups device */
    GT_U32  localPort;   /* local port - support multi-port-groups device */
    GT_U32 txqPortNum;  /* TXQ port number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_FALCON_E);

    if (portSpeedIndex > 7 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum, txqPortNum);

        regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).CCFCSpeedIndex[(txqPortNum >> 3)];
        fieldOffset = (txqPortNum % 8) * 3;

        return prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 3, portSpeedIndex);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bcnRegs.
                    portsSpeedIndexesReg0 + 4 * (localPort / 10);

        fieldOffset = (localPort % 10) * 3;

        return prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                         fieldOffset, 3, portSpeedIndex);
    }
}

/**
* @internal cpssDxChPortCnSpeedIndexSet function
* @endinternal
*
* @brief   Sets port speed index of the egress port. The device inserts this index
*         in the generated CCFC frame.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] portSpeedIndex           - port speed index (0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range port speed index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Panic Pause uses Port Speed Index 0 and 7 to indicate XON and XOFF.
*       Congested ports should use Port Speed Index 1 to 6 if
*       Panic Pause is used.
*
*/
GT_STATUS cpssDxChPortCnSpeedIndexSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32   portSpeedIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnSpeedIndexSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portSpeedIndex));

    rc = internal_cpssDxChPortCnSpeedIndexSet(devNum, portNum, portSpeedIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portSpeedIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnSpeedIndexGet function
* @endinternal
*
* @brief   Gets port speed index of the egress port. The device inserts this index
*         in the generated CCFC frame.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  -      - port number.
*
* @param[out] portSpeedIndexPtr        - (pointer to) port speed index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnSpeedIndexGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32   *portSpeedIndexPtr
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 fieldOffset; /* the start bit number in the register */
    GT_U32 portGroupId; /* the port group Id - support multi-port-groups device */
    GT_U32  localPort;   /* local port - support multi-port-groups device */
    GT_U32 txqPortNum;  /* TXQ port number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(portSpeedIndexPtr);

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum, txqPortNum);

        regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).CCFCSpeedIndex[(txqPortNum >> 3)];
        fieldOffset = (txqPortNum % 8) * 3;

        return prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, 3, portSpeedIndexPtr);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bcnRegs.
                    portsSpeedIndexesReg0 + 4 * (localPort / 10);

        fieldOffset = (localPort % 10) * 3;

        return prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                         fieldOffset, 3, portSpeedIndexPtr);
    }
}

/**
* @internal cpssDxChPortCnSpeedIndexGet function
* @endinternal
*
* @brief   Gets port speed index of the egress port. The device inserts this index
*         in the generated CCFC frame.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  -      - port number.
*
* @param[out] portSpeedIndexPtr        - (pointer to) port speed index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnSpeedIndexGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32   *portSpeedIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnSpeedIndexGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portSpeedIndexPtr));

    rc = internal_cpssDxChPortCnSpeedIndexGet(devNum, portNum, portSpeedIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portSpeedIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortCnFcEnableSet function
* @endinternal
*
* @brief   Enable/Disable generation of flow control packets as result of CN
*         frame termination or Panic Pause.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] enable                   - GT_TRUE:  enable. Flow control packets can be issued.
*                                      GT_FALSE: disable. Flow control packets can't be issued.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnFcEnableSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL  enable
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 fieldOffset; /* the start bit number in the register */
    GT_U32 value;       /* register value */
    GT_STATUS rc;       /* function return value */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* used MAC unit */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_FALCON_E);
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum, portNum);

    value = BOOL2BIT_MAC(enable);

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
       (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
       (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
    {
        for(portMacType = PRV_CPSS_PORT_GE_E; portMacType < PRV_CPSS_PORT_NOT_APPLICABLE_E; portMacType++)
        {
            if((portNum == CPSS_CPU_PORT_NUM_CNS) && (portMacType >= PRV_CPSS_PORT_XG_E))
                continue;

            if(portMacType >= PRV_CPSS_PORT_XG_E)
            {
                PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portNum,portMacType,&regAddr);
                fieldOffset = 15;
            }
            else
            {
                PRV_CPSS_DXCH_PORT_MAC_CTRL1_REG_MAC(devNum,portNum,portMacType,&regAddr);
                fieldOffset = 2;
            }

            if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                rc = prvCpssDrvHwPpSetRegField(devNum,regAddr,fieldOffset, 1, value);

                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    else /*lion*/
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).enFCTriggerByCNFrameOnPort[OFFSET_TO_WORD_MAC(portNum)];
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                txqVer1.queue.congestNotification.cnFcEn[OFFSET_TO_WORD_MAC(portNum)];
        }

        fieldOffset = OFFSET_TO_BIT_MAC(portNum);

        rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 1, value);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            rc = prvCpssDxChHwEgfEftFieldSet(devNum, portNum,
                PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_FC_TRIGGER_BY_CN_FRAME_ON_PORT_ENABLE_E,
                value);
            if (rc != GT_OK )
            {
                return rc;
            }
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                txqVer1.egr.filterConfig.egrCnFcEn[OFFSET_TO_WORD_MAC(portNum)];

            rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 1, value);
            if (rc != GT_OK )
            {
                return rc;
            }
        }
    }
    return GT_OK;

}

/**
* @internal cpssDxChPortCnFcEnableSet function
* @endinternal
*
* @brief   Enable/Disable generation of flow control packets as result of CN
*         frame termination or Panic Pause.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] enable                   - GT_TRUE:  enable. Flow control packets can be issued.
*                                      GT_FALSE: disable. Flow control packets can't be issued.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnFcEnableSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnFcEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortCnFcEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnFcEnableGet function
* @endinternal
*
* @brief   Gets the status of generation of flow control packets as result of CN
*         frame termination or Panic Pause on a given port.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE:  enable. Flow control packets is issued.
*                                      GT_FALSE: disable. Flow control packets is not issued.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnFcEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_U32 regAddr;    /* register address */
    GT_U32 fieldOffset;/* the start bit number in the register */
    GT_U32 value;      /* register value */
    GT_STATUS rc;      /* function return value */
    PRV_CPSS_PORT_TYPE_ENT portMacType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_FALCON_E);
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);


    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
       (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
       (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
    {
        portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum);

        if(portMacType >= PRV_CPSS_PORT_XG_E)
        {
            PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portNum,portMacType,&regAddr);
            fieldOffset = 15;
        }
        else
        {
            PRV_CPSS_DXCH_PORT_MAC_CTRL1_REG_MAC(devNum,portNum,portMacType,&regAddr);
            fieldOffset = 2;
        }

        rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, fieldOffset, 1, &value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else /*lion*/
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).enFCTriggerByCNFrameOnPort[OFFSET_TO_WORD_MAC(portNum)];
            fieldOffset = OFFSET_TO_BIT_MAC(portNum);

            rc = prvCpssHwPpGetRegField(devNum, regAddr,fieldOffset, 1, &value);
            if (rc != GT_OK)
            {
                return rc;
            }

        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
             txqVer1.egr.filterConfig.egrCnFcEn[OFFSET_TO_WORD_MAC(portNum)];
            fieldOffset = OFFSET_TO_BIT_MAC(portNum);

            rc = prvCpssHwPpGetRegField(devNum, regAddr,fieldOffset, 1, &value);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChPortCnFcEnableGet function
* @endinternal
*
* @brief   Gets the status of generation of flow control packets as result of CN
*         frame termination or Panic Pause on a given port.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE:  enable. Flow control packets is issued.
*                                      GT_FALSE: disable. Flow control packets is not issued.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnFcEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnFcEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortCnFcEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnPauseTriggerEnableSet function
* @endinternal
*
* @brief   Enable the CN triggering engine to trigger a pause frame.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] enable                   - GT_TRUE:  enable.
*                                      GT_FALSE: disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To enable proper operation of CCFC, this configuration must be
*       enabled for all network ports and disabled for all cascade ports.
*
*/
static GT_STATUS internal_cpssDxChPortCnPauseTriggerEnableSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL  enable
)
{
    GT_U32 regAddr;    /* register address */
    GT_U32 portGroupId;     /* the port group Id - support multi-port-groups device */
    GT_U32 value;      /* register value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

    value = BOOL2BIT_MAC(enable);
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bcnRegs.bcnPauseTriggerEnableReg;


    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    return prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, portNum, 1, value);
}

/**
* @internal cpssDxChPortCnPauseTriggerEnableSet function
* @endinternal
*
* @brief   Enable the CN triggering engine to trigger a pause frame.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] enable                   - GT_TRUE:  enable.
*                                      GT_FALSE: disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To enable proper operation of CCFC, this configuration must be
*       enabled for all network ports and disabled for all cascade ports.
*
*/
GT_STATUS cpssDxChPortCnPauseTriggerEnableSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnPauseTriggerEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortCnPauseTriggerEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnPauseTriggerEnableGet function
* @endinternal
*
* @brief   Gets the status triggering engine a pause frame on a given port.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE:  enable.
*                                      GT_FALSE: disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnPauseTriggerEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_U32 regAddr;    /* register address */
    GT_U32 portGroupId;     /* the port group Id - support multi-port-groups device */
    GT_U32 value;      /* register value */
    GT_STATUS rc;      /* function return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bcnRegs.bcnPauseTriggerEnableReg;

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,portNum, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }
    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChPortCnPauseTriggerEnableGet function
* @endinternal
*
* @brief   Gets the status triggering engine a pause frame on a given port.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE:  enable.
*                                      GT_FALSE: disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnPauseTriggerEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnPauseTriggerEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortCnPauseTriggerEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnFcTimerSet function
* @endinternal
*
* @brief   Sets timer value for the IEEE 802.3x/PFC frame issued as result
*         of CN frame termination.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] index                    -  (APPLICABLE RANGES: 0..7)
*                                      On AC3X remote ports: index (APPLICABLE RANGES: 0..63) (See Comment 4.)
* @param[in] timer                    - 802.3x/PFC pause time (APPLICABLE RANGES: 0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or speed index
* @retval GT_OUT_OF_RANGE          - on out of range pause time value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. xCat,xCat3: Panic Pause uses Port Speed Index 0 and 7 to indicate
*       XON and XOFF. Port timer configurations 0 and 7 must be configured
*       to 0x0 and 0xFFFF if Panic Pause is used.
*       2. xCat,xCat3: index is always congested port speed index (CCFC mode).
*       3. Lion, xCat2: index determined by
*       cpssDxChPortCnPrioritySpeedLocationSet.
*       4. AC3X: On remote ports, index is comprised of
*       (MSB) 2 bits of Speed code,
*       2 bits of qFB,
*       1 bit of qDelta sign
*       (LSB) 1 bit of qDelta absolute value > 255.
*
*/
static GT_STATUS internal_cpssDxChPortCnFcTimerSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32   index,
    IN GT_U32   timer
)
{
    GT_STATUS rc;
    GT_U32 regAddr;     /* register address */
    GT_U32 portMacNum;  /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if (timer > 0xFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macDrvMacCnFcTimerSetFunc)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macDrvMacCnFcTimerSetFunc(devNum,portNum,
                                  index,
                                  timer,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    if (index > 7 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE) ||
       (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
    {
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            regAddr = PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum).portSpeedTimer[index];
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).portSpeedTimer[index];
        }

        return prvCpssHwPpSetRegField(devNum, regAddr, 0, 16, timer);
    }
    else if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

        if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].
                            macRegsPerType[PRV_CPSS_PORT_GE_E].ccfcFcTimer + (index<<2);
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldData = timer;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 16;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 0;
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].
                            macRegsPerType[PRV_CPSS_PORT_XG_E].ccfcFcTimer + (index<<2);
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_XG_E].fieldData = timer;
            regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 16;
            regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = 0;
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].
                            macRegsPerType[PRV_CPSS_PORT_XLG_E].ccfcFcTimer + (index<<2);
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = timer;
            regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 16;
            regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 0;
        }

        return prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
    }
    else
    {
        GT_U32 portGroupId; /* the port group Id - support multi-port-groups device */

        if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum) == PRV_CPSS_PORT_XG_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                perPortRegs[portNum].ccfcFcTimerBaseXg;

            if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
            {
                timer = (timer >> 8) | ((timer & 0xFF) << 8);
            }
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                perPortRegs[portNum].ccfcFcTimerBaseGig;
        }

        regAddr += index*4;

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

        return prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,0, 16, timer);
    }
}

/**
* @internal cpssDxChPortCnFcTimerSet function
* @endinternal
*
* @brief   Sets timer value for the IEEE 802.3x/PFC frame issued as result
*         of CN frame termination.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] index                    -  (APPLICABLE RANGES: 0..7)
*                                      On AC3X remote ports: index (APPLICABLE RANGES: 0..63) (See Comment 4.)
* @param[in] timer                    - 802.3x/PFC pause time (APPLICABLE RANGES: 0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or speed index
* @retval GT_OUT_OF_RANGE          - on out of range pause time value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. xCat,xCat3: Panic Pause uses Port Speed Index 0 and 7 to indicate
*       XON and XOFF. Port timer configurations 0 and 7 must be configured
*       to 0x0 and 0xFFFF if Panic Pause is used.
*       2. xCat,xCat3: index is always congested port speed index (CCFC mode).
*       3. Lion, xCat2: index determined by
*       cpssDxChPortCnPrioritySpeedLocationSet.
*       4. AC3X: On remote ports, index is comprised of
*       (MSB) 2 bits of Speed code,
*       2 bits of qFB,
*       1 bit of qDelta sign
*       (LSB) 1 bit of qDelta absolute value > 255.
*
*/
GT_STATUS cpssDxChPortCnFcTimerSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32   index,
    IN GT_U32   timer
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnFcTimerSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, index, timer));

    rc = internal_cpssDxChPortCnFcTimerSet(devNum, portNum, index, timer);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, index, timer));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnFcTimerGet function
* @endinternal
*
* @brief   Gets timer value for the IEEE 802.3x/PFC frame issued as result
*         of CN frame termination.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] index                    -  (APPLICABLE RANGES: 0..7)
*                                      On AC3X remote ports: index (APPLICABLE RANGES: 0..63) (See Comment 3.)
*
* @param[out] timerPtr                 - (pointer to) 802.3x/PFC Pause time
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number, device or speed index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. xCat,xCat3: index is always congested port speed index (CCFC mode).
*       2. Lion, xCat2: index determined by
*       cpssDxChPortCnPrioritySpeedLocationSet.
*       3. AC3X: On remote ports, index is comprised of
*       (MSB) 2 bits of Speed code,
*       2 bits of qFB,
*       1 bit of qDelta sign
*       (LSB) 1 bit of qDelta absolute value > 255.
*
*/
static GT_STATUS internal_cpssDxChPortCnFcTimerGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U32   index,
    OUT GT_U32   *timerPtr
)
{
    GT_U32 regAddr;  /* register address */
    GT_U32 portGroupId;   /* the port group Id - support multi-port-groups device */
    GT_STATUS rc;    /* function return value */
    GT_U32 value;    /* register value */
    GT_U32 portMacNum;      /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(timerPtr);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macDrvMacCnFcTimerGetFunc)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macDrvMacCnFcTimerGetFunc(devNum,portNum,
                                  index,
                                  timerPtr,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    if (index > 7 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE) ||
       (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))

    {
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            regAddr = PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum).portSpeedTimer[index];
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).portSpeedTimer[index];
        }

        rc =  prvCpssHwPpGetRegField(devNum, regAddr, 0, 16, timerPtr);
        return rc;
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].
                macRegsPerType[PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum)].ccfcFcTimer;
    }
    else
    {
        if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum) >= PRV_CPSS_PORT_XG_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                perPortRegs[portNum].ccfcFcTimerBaseXg;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                perPortRegs[portNum].ccfcFcTimerBaseGig;
        }
    }
    regAddr += index*4;

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,0, 16, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_XG_E &&
      ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
       (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)))
    {
        *timerPtr = (value >> 8) | ((value & 0xFF) << 8);
    }
    else
    {
        *timerPtr = value;
    }
    return GT_OK;
}

/**
* @internal cpssDxChPortCnFcTimerGet function
* @endinternal
*
* @brief   Gets timer value for the IEEE 802.3x/PFC frame issued as result
*         of CN frame termination.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] index                    -  (APPLICABLE RANGES: 0..7)
*                                      On AC3X remote ports: index (APPLICABLE RANGES: 0..63) (See Comment 3.)
*
* @param[out] timerPtr                 - (pointer to) 802.3x/PFC Pause time
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number, device or speed index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. xCat,xCat3: index is always congested port speed index (CCFC mode).
*       2. Lion, xCat2: index determined by
*       cpssDxChPortCnPrioritySpeedLocationSet.
*       3. AC3X: On remote ports, index is comprised of
*       (MSB) 2 bits of Speed code,
*       2 bits of qFB,
*       1 bit of qDelta sign
*       (LSB) 1 bit of qDelta absolute value > 255.
*
*/
GT_STATUS cpssDxChPortCnFcTimerGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U32   index,
    OUT GT_U32   *timerPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnFcTimerGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, index, timerPtr));

    rc = internal_cpssDxChPortCnFcTimerGet(devNum, portNum, index, timerPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, index, timerPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortCnPanicPauseThresholdsSet function
* @endinternal
*
* @brief   Sets the thresholds for triggering a Panic Pause: Global XOFF/XON frame
*         over all ports enabled to send a Panic Pause.
*         A global XOFF frame is triggered if the number of buffers occupied
*         by the CN aware frames crosses up xoffLimit. A global XON frame is
*         triggered if the number of buffers occupied by the CN aware frames
*         crosses down xonLimit.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE: Enable Panic Pause.
*                                      GT_FALSE: Disable Panic Pause.
* @param[in] xoffLimit                - X-OFF limit (0..1FFF).Relevant only if enable==GT_TRUE.
* @param[in] xonLimit                 - X-ON limit (0..1FFF).Relevant only if enable==GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range xoffLimit or
*                                       xonLimit value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Panic Pause uses Port Speed Index 0 and 7 to indicate XON and XOFF.
*       Port timer configurations 0 and 7 must be configured
*       to 0x0 and 0xFFFF, see cpssDxChPortCnFcTimerSet.
*       Congested ports should use Port Speed Index 1 to 6,
*       see cpssDxChPortCnSpeedIndexSet.
*       2. Panic Pause cannot operate together with the IEEE 802.3x triggers.
*
*/
static GT_STATUS internal_cpssDxChPortCnPanicPauseThresholdsSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable,
    IN GT_U32   xoffLimit,
    IN GT_U32   xonLimit
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* for disable set to maximum value */
    if (enable == GT_FALSE)
    {
        xoffLimit = 0x1FFF;
        xonLimit = 0x1FFF;
    }
    if (xoffLimit > 0x1FFF || xonLimit > 0x1FFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bcnRegs.bcnGlobalPauseThresholdsReg;
    value = xoffLimit | xonLimit << 13;

    return prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 26, value);
}

/**
* @internal cpssDxChPortCnPanicPauseThresholdsSet function
* @endinternal
*
* @brief   Sets the thresholds for triggering a Panic Pause: Global XOFF/XON frame
*         over all ports enabled to send a Panic Pause.
*         A global XOFF frame is triggered if the number of buffers occupied
*         by the CN aware frames crosses up xoffLimit. A global XON frame is
*         triggered if the number of buffers occupied by the CN aware frames
*         crosses down xonLimit.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE: Enable Panic Pause.
*                                      GT_FALSE: Disable Panic Pause.
* @param[in] xoffLimit                - X-OFF limit (0..1FFF).Relevant only if enable==GT_TRUE.
* @param[in] xonLimit                 - X-ON limit (0..1FFF).Relevant only if enable==GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range xoffLimit or
*                                       xonLimit value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Panic Pause uses Port Speed Index 0 and 7 to indicate XON and XOFF.
*       Port timer configurations 0 and 7 must be configured
*       to 0x0 and 0xFFFF, see cpssDxChPortCnFcTimerSet.
*       Congested ports should use Port Speed Index 1 to 6,
*       see cpssDxChPortCnSpeedIndexSet.
*       2. Panic Pause cannot operate together with the IEEE 802.3x triggers.
*
*/
GT_STATUS cpssDxChPortCnPanicPauseThresholdsSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable,
    IN GT_U32   xoffLimit,
    IN GT_U32   xonLimit
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnPanicPauseThresholdsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable, xoffLimit, xonLimit));

    rc = internal_cpssDxChPortCnPanicPauseThresholdsSet(devNum, enable, xoffLimit, xonLimit);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable, xoffLimit, xonLimit));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnPanicPauseThresholdsGet function
* @endinternal
*
* @brief   Gets the thresholds for triggering a Panic Pause: Global XOFF/XON frame
*         over all ports enabled to send a Panic Pause.
*         A global XOFF frame is triggered if the number of buffers occupied
*         by the CN aware frames crosses up xoffLimit. A global XON frame is
*         triggered if the number of buffers occupied by the CN aware frames
*         crosses down xonLimit.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to):
*                                      GT_TRUE: Enable Panic Pause.
*                                      GT_FALSE: Disable Panic Pause.
* @param[out] xoffLimitPtr             - (pointer to) the X-OFF limit value.
*                                      Relevant only if enablePtr==GT_TRUE.
* @param[out] xonLimitPtr              - (pointer to) the X-ON limit value.
*                                      Relevant only if enablePtr==GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnPanicPauseThresholdsGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr,
    OUT GT_U32   *xoffLimitPtr,
    OUT GT_U32   *xonLimitPtr
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(xoffLimitPtr);
    CPSS_NULL_PTR_CHECK_MAC(xonLimitPtr);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bcnRegs.bcnGlobalPauseThresholdsReg;

    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 0, 26, &value);
    if (rc != GT_OK)
    {
        return rc;
    }
    *enablePtr = (value == 0x3FFFFFF)? GT_FALSE:GT_TRUE;
    *xoffLimitPtr = value & 0x1FFF;
    *xonLimitPtr = value >> 13;
    return GT_OK;
}

/**
* @internal cpssDxChPortCnPanicPauseThresholdsGet function
* @endinternal
*
* @brief   Gets the thresholds for triggering a Panic Pause: Global XOFF/XON frame
*         over all ports enabled to send a Panic Pause.
*         A global XOFF frame is triggered if the number of buffers occupied
*         by the CN aware frames crosses up xoffLimit. A global XON frame is
*         triggered if the number of buffers occupied by the CN aware frames
*         crosses down xonLimit.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to):
*                                      GT_TRUE: Enable Panic Pause.
*                                      GT_FALSE: Disable Panic Pause.
* @param[out] xoffLimitPtr             - (pointer to) the X-OFF limit value.
*                                      Relevant only if enablePtr==GT_TRUE.
* @param[out] xonLimitPtr              - (pointer to) the X-ON limit value.
*                                      Relevant only if enablePtr==GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnPanicPauseThresholdsGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr,
    OUT GT_U32   *xoffLimitPtr,
    OUT GT_U32   *xonLimitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnPanicPauseThresholdsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr, xoffLimitPtr, xonLimitPtr));

    rc = internal_cpssDxChPortCnPanicPauseThresholdsGet(devNum, enablePtr, xoffLimitPtr, xonLimitPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr, xoffLimitPtr, xonLimitPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnTerminationEnableSet function
* @endinternal
*
* @brief   Enable/Disable termination of CNM (Congestion Notification Message) on given port.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] enable                   - GT_TRUE: enable. CNM is terminated, i.e not transmited on this port.
*                                      GT_FALSE: disable. CNM isn't terminated, i.e transmited on this port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnTerminationEnableSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL  enable
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* register value */
    GT_STATUS rc;       /* function return value */
    GT_U32 fieldOffset;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum, portNum);

    value = (enable == GT_TRUE)? 0 : 1;

    fieldOffset = OFFSET_TO_BIT_MAC(portNum);

    if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).enCNFrameTxOnPort[OFFSET_TO_WORD_MAC(portNum)];
        }
        else
        {
            PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
              txqVer1.queue.congestNotification.cnEnCnFrameTx[OFFSET_TO_WORD_MAC(portNum)];
        }
        rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 1, value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        return prvCpssDxChHwEgfEftFieldSet(devNum,portNum,
            PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_CN_FRAME_TX_ON_PORT_ENABLE_E,
            value);
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                txqVer1.egr.filterConfig.egrCnEn[OFFSET_TO_WORD_MAC(portNum)];

        return prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 1, value);
    }
}

/**
* @internal cpssDxChPortCnTerminationEnableSet function
* @endinternal
*
* @brief   Enable/Disable termination of CNM (Congestion Notification Message) on given port.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] enable                   - GT_TRUE: enable. CNM is terminated, i.e not transmited on this port.
*                                      GT_FALSE: disable. CNM isn't terminated, i.e transmited on this port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnTerminationEnableSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnTerminationEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortCnTerminationEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnTerminationEnableGet function
* @endinternal
*
* @brief   Gets the status of CNM termination on given port.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: enable. CNM is terminated, i.e not transmited on this port.
*                                      GT_FALSE: disable. CNM isn't terminated, i.e transmited on this port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnTerminationEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* register value */
    GT_STATUS rc;       /* function return value */
    GT_U32 fieldOffset;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChHwEgfEftFieldGet(devNum,portNum,
            PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_CN_FRAME_TX_ON_PORT_ENABLE_E,
            &value);
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                txqVer1.egr.filterConfig.egrCnEn[OFFSET_TO_WORD_MAC(portNum)];
        fieldOffset = OFFSET_TO_BIT_MAC(portNum);

        rc = prvCpssHwPpGetRegField(devNum, regAddr,fieldOffset, 1, &value);
    }

    if (rc != GT_OK)
    {
        return rc;
    }
    *enablePtr = (value == 0) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssDxChPortCnTerminationEnableGet function
* @endinternal
*
* @brief   Gets the status of CNM termination on given port.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: enable. CNM is terminated, i.e not transmited on this port.
*                                      GT_FALSE: disable. CNM isn't terminated, i.e transmited on this port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnTerminationEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnTerminationEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortCnTerminationEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

static GT_STATUS cpssDxChPortCnSip6ProfileQueueConfigSet
(
    IN GT_U8                               devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN GT_U8                               tcQueue,
    IN CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
)
{
    GT_STATUS rc;     /* function return value */
    GT_U32 thresholdNumOfBits = 20; /* number of bits in threshold field*/
    GT_U32 alphaNumOfBits = 4; /* number of bits in alpha field */
    GT_U32 alphaHwValue = 0; /* dynamic factor alpha value */
    GT_U32 tableIndex; /* table index */

    if (cnProfileCfgPtr->threshold > BIT_MASK_MAC(thresholdNumOfBits))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    tableIndex = (profileSet << 4) + tcQueue;
    rc = prvCpssDxChWriteTableEntryField(devNum,
        CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_CONFIGURATIONS_E,
        tableIndex,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,44,1,
        BOOL2BIT_MAC(cnProfileCfgPtr->cnAware));

    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Updating TABLE_PREQ_QUEUE_CONFIGURATIONS for cnAware %d failed at index %d\n",
            BOOL2BIT_MAC(cnProfileCfgPtr->cnAware),16*(profileSet*16+tcQueue));
    }


    /* set threshold */
    rc = prvCpssDxChWriteTableEntryField(devNum,
        CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_CONFIGURATIONS_E,
        tableIndex,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,24,thresholdNumOfBits,
        cnProfileCfgPtr->threshold);

    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Updating TABLE_PREQ_QUEUE_CONFIGURATIONS for threshold %d failed at index %d\n",
            cnProfileCfgPtr->threshold,16*(profileSet*16+tcQueue));
    }

    /* Set alpha for QCN DBA for Falcon */
    PRV_CPSS_PORT_TX_TAIL_DROP_SIP6_ALPHA_TO_HW_CONVERT_MAC(cnProfileCfgPtr->alpha, alphaHwValue);

    rc = prvCpssDxChWriteTableEntryField(devNum,
        CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_CONFIGURATIONS_E,
        tableIndex,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,20,alphaNumOfBits,
        alphaHwValue);

    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Updating TABLE_PREQ_QUEUE_CONFIGURATIONS for alpha %d failed at index %d\n",
            alphaHwValue,16*(profileSet*16+tcQueue));
    }

    return rc;
}

static GT_STATUS cpssDxChPortCnSip6ProfileQueueConfigGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN  GT_U8                               tcQueue,
    OUT CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
)
{
    GT_STATUS rc;     /* function return value */
    GT_U32 fieldLenght = 20;  /* field length */
    GT_U32 alphaNumOfBits = 4; /* field length */
    GT_U32 value;
    GT_U32 alphaHwValue;
    GT_U32 tableIndex = (profileSet << 4) + tcQueue;
    GT_U32 hwArray[4];

    /* get CN awareness */
    rc = prvCpssDxChReadTableEntry(devNum, CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_CONFIGURATIONS_E, tableIndex, hwArray);

    U32_GET_FIELD_IN_ENTRY_MAC(hwArray, 44, 1, value);

    if (rc != GT_OK)
    {
        return rc;
    }
    cnProfileCfgPtr->cnAware = BIT2BOOL_MAC(value);

    /* get threshold */
    U32_GET_FIELD_IN_ENTRY_MAC(hwArray, 24, fieldLenght, cnProfileCfgPtr->threshold);

    U32_GET_FIELD_IN_ENTRY_MAC(hwArray, 20, alphaNumOfBits, alphaHwValue);

    /* Get alpha for QCN DBA for Falcon */
    PRV_CPSS_PORT_TX_TAIL_DROP_SIP6_HW_TO_ALPHA_CONVERT_MAC(cnProfileCfgPtr->alpha, alphaHwValue);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnProfileQueueConfigSet function
* @endinternal
*
* @brief   Sets CN profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - profile set.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] cnProfileCfgPtr          - pointer to CN Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile set
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CN Profile is part of the Tail-Drop Profile.
*       To bind port to Tail-Drop Profile use cpssDxChPortTxBindPortToDpSet.
*
*/
static GT_STATUS internal_cpssDxChPortCnProfileQueueConfigSet
(
    IN GT_U8                               devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN GT_U8                               tcQueue,
    IN CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
)
{
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */
    GT_U32 regOffset; /* register offset */
    GT_U32 fieldOffset; /* field offset */
    GT_U32 thresholdNumOfBits; /* number of bits in threshold field*/
    GT_U32 alphaNumOfBits = 3; /* number of bits in alpha field default to SIP5_25 */
    GT_U32 alphaHwValue = 0; /* dynamic factor alpha value */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tcQueue);
    CPSS_NULL_PTR_CHECK_MAC(cnProfileCfgPtr);
    PRV_CPSS_DXCH_DROP_PROFILE_CHECK_MAC(profileSet,devNum);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        rc = cpssDxChPortCnSip6ProfileQueueConfigSet(devNum, profileSet, tcQueue, cnProfileCfgPtr); /* new static function */
        return rc;
    }


    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) == GT_TRUE)
    {
        thresholdNumOfBits = 20;
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        thresholdNumOfBits = 16;
    }
    else
    {
        thresholdNumOfBits = 13;
    }

    if (cnProfileCfgPtr->threshold > BIT_MASK_MAC(thresholdNumOfBits))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }


    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* Bobcat2 A0 revision has erratum FE-535006
           - The number of CN Tail Drop profiles is limited to 8 when QCN is enabled */
        if((!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum)) && (profileSet > CPSS_PORT_TX_DROP_PROFILE_8_E))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.profileCNMTriggeringEnable[profileSet];
        fieldOffset = tcQueue;
    }
    else
    {
        /* set CN awareness */
        regOffset = (profileSet/4) * 0x4;
        fieldOffset = (profileSet%4)*8 + tcQueue;
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.queue.congestNotification.cnPrioQueueEnProfile + regOffset;
    }
    rc = prvCpssHwPpSetRegField(devNum, regAddr,fieldOffset, 1,
                                   BOOL2BIT_MAC(cnProfileCfgPtr->cnAware));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* set threshold */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).profilePriorityQueueCNThreshold[profileSet][tcQueue];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.queue.congestNotification.cnProfileThreshold[tcQueue][profileSet];
    }

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, thresholdNumOfBits, cnProfileCfgPtr->threshold);

    if (rc != GT_OK)
    {
        return rc;
    }

    /* Set alpha for QCN DBA for Aldrin 2 */
    if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_PORT_TX_TAIL_DROP_QCN_DBA_ALPHA_TO_HW_CONVERT_MAC(cnProfileCfgPtr->alpha, alphaHwValue);
        rc = prvCpssHwPpSetRegField(devNum, regAddr, thresholdNumOfBits, alphaNumOfBits, alphaHwValue);
    }

    return rc;
}

/**
* @internal cpssDxChPortCnProfileQueueConfigSet function
* @endinternal
*
* @brief   Sets CN profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - profile set.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] cnProfileCfgPtr          - pointer to CN Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile set
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CN Profile is part of the Tail-Drop Profile.
*       To bind port to Tail-Drop Profile use cpssDxChPortTxBindPortToDpSet.
*
*/
GT_STATUS cpssDxChPortCnProfileQueueConfigSet
(
    IN GT_U8                               devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN GT_U8                               tcQueue,
    IN CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnProfileQueueConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, tcQueue, cnProfileCfgPtr));

    rc = internal_cpssDxChPortCnProfileQueueConfigSet(devNum, profileSet, tcQueue, cnProfileCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, tcQueue, cnProfileCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnProfileQueueConfigGet function
* @endinternal
*
* @brief   Gets CN profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - profile set.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] cnProfileCfgPtr          - pointer to CN Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_STATE             - on wrong values of alpha
* @retval GT_BAD_PARAM             - on wrong device number, profile set
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnProfileQueueConfigGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN  GT_U8                               tcQueue,
    OUT CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
)
{
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */
    GT_U32 regOffset;    /* register offset */
    GT_U32 fieldOffset;  /* field offset */
    GT_U32 fieldLenght;  /* field length */
    GT_U32 alphaLenght = 3;  /* field length */
    GT_U32 value;
    GT_U32 alphaHwValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tcQueue);
    CPSS_NULL_PTR_CHECK_MAC(cnProfileCfgPtr);
    PRV_CPSS_DXCH_DROP_PROFILE_CHECK_MAC(profileSet,devNum);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        rc = cpssDxChPortCnSip6ProfileQueueConfigGet(devNum, profileSet, tcQueue, cnProfileCfgPtr); /* new static function */
        return rc;
    }

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    /* get CN awareness */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* Bobcat2 A0 revision has erratum FE-535006
           - The number of CN Tail Drop profiles is limited to 8 when QCN is enabled */
        if((!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum)) && (profileSet > CPSS_PORT_TX_DROP_PROFILE_8_E))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.profileCNMTriggeringEnable[profileSet];
        fieldOffset = tcQueue;
    }
    else
    {
        regOffset = (profileSet/4) * 0x4;
        fieldOffset = (profileSet%4)*8 + tcQueue;
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.queue.congestNotification.cnPrioQueueEnProfile + regOffset;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }
    cnProfileCfgPtr->cnAware = BIT2BOOL_MAC(value);

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) == GT_TRUE)
    {
        fieldLenght = 20;
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        fieldLenght = 16;
    }
    else
    {
        fieldLenght = 13;
    }

    /* get threshold */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).profilePriorityQueueCNThreshold[profileSet][tcQueue];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.queue.congestNotification.cnProfileThreshold[tcQueue][profileSet];
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, fieldLenght, &(cnProfileCfgPtr->threshold));
    if (rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldLenght, alphaLenght, &(alphaHwValue));
        /* Get alpha for QCN DBA for Aldrin 2 */
        PRV_CPSS_PORT_TX_TAIL_DROP_QCN_DBA_HW_TO_ALPHA_CONVERT_MAC(alphaHwValue, cnProfileCfgPtr->alpha);
    }

    return rc;
}

/**
* @internal cpssDxChPortCnProfileQueueConfigGet function
* @endinternal
*
* @brief   Gets CN profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - profile set.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] cnProfileCfgPtr          - pointer to CN Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_STATE             - on wrong values of alpha
* @retval GT_BAD_PARAM             - on wrong device number, profile set
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnProfileQueueConfigGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN  GT_U8                               tcQueue,
    OUT CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnProfileQueueConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, tcQueue, cnProfileCfgPtr));

    rc = internal_cpssDxChPortCnProfileQueueConfigGet(devNum, profileSet, tcQueue, cnProfileCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, tcQueue, cnProfileCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnQueueStatusModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable queue status mode. When enabled, CNM is sent to a
*         configurable target that examines the queue status.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE: CNM is sent to a targetDev/targetPort.
*                                      GT_FALSE: CNM is sent to a sampled packet's source.
* @param[in] targetHwDev              Target HW device number for the CNM.
*                                      Relevant only if enable==GT_TRUE.
* @param[in] targetPort               - Target port for the CNM.
*                                      Relevant only if enable==GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range targetDev or targetPort
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnQueueStatusModeEnableSet
(
    IN GT_U8            devNum,
    IN GT_BOOL          enable,
    IN GT_HW_DEV_NUM    targetHwDev,
    IN GT_PORT_NUM      targetPort
)
{
    GT_U32 value;       /* register value */
    GT_U32 regAddr;     /* register address */
    GT_U32 fieldSize;   /* register field size */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    if (enable == GT_TRUE)
    {
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            if ((targetHwDev > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(devNum)) ||
                 targetPort > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            value = (targetHwDev << 15) | (targetPort) | (1 << 25);
        }
        else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            if ((targetHwDev > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(devNum)) ||
                 targetPort > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            value = 1 | (targetHwDev << 1) | (targetPort << 11);
        }
        else
        {
            if (targetHwDev > 31 || targetPort > 63)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            value = 1 | (targetHwDev & 0x1F) << 1 | (targetPort & 0x3F) << 6;
        }
    }
    else
    {
        value = 0;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EREP.configurations.qcnConfigs1;
        fieldSize = 26;
        return prvCpssHwPpSetRegField(devNum, regAddr, 0, fieldSize, value);
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).CNTargetAssignment;
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            fieldSize = 26;
        }
        else
        {
            fieldSize = 24;
        }

        return prvCpssHwPpSetRegField(devNum, regAddr, 0, fieldSize, value);
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.congestNotification.cnGlobalConfigReg;

        return prvCpssHwPpSetRegField(devNum, regAddr, 8, 12, value);
    }
}

/**
* @internal cpssDxChPortCnQueueStatusModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable queue status mode. When enabled, CNM is sent to a
*         configurable target that examines the queue status.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE: CNM is sent to a targetDev/targetPort.
*                                      GT_FALSE: CNM is sent to a sampled packet's source.
* @param[in] targetHwDev              Target HW device number for the CNM.
*                                      Relevant only if enable==GT_TRUE.
* @param[in] targetPort               - Target port for the CNM.
*                                      Relevant only if enable==GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range targetDev or targetPort
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnQueueStatusModeEnableSet
(
    IN GT_U8            devNum,
    IN GT_BOOL          enable,
    IN GT_HW_DEV_NUM    targetHwDev,
    IN GT_PORT_NUM      targetPort
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnQueueStatusModeEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable, targetHwDev, targetPort));

    rc = internal_cpssDxChPortCnQueueStatusModeEnableSet(devNum, enable, targetHwDev, targetPort);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable, targetHwDev, targetPort));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnQueueStatusModeEnableGet function
* @endinternal
*
* @brief   Gets the current status of queue status mode.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) status of queue status mode.
*                                      GT_TRUE: CNM is sent to a targetDev/targetPort.
*                                      GT_FALSE: CNM is sent to a sampled packet's source.
* @param[out] targetHwDevPtr           (pointer to) Target HW device number for the CNM.
*                                      Relevant only if enable==GT_TRUE.
* @param[out] targetPortPtr            - (pointer to) Target port for the CNM.
*                                      Relevant only if enable==GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnQueueStatusModeEnableGet
(
    IN  GT_U8           devNum,
    OUT GT_BOOL         *enablePtr,
    OUT GT_HW_DEV_NUM   *targetHwDevPtr,
    OUT GT_PORT_NUM     *targetPortPtr
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */
    GT_U32 fieldSize; /* register field size */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(targetHwDevPtr);
    CPSS_NULL_PTR_CHECK_MAC(targetPortPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EREP.configurations.qcnConfigs1;
            fieldSize = 26;
            rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, fieldSize, &value);
        }
        else
        {
            fieldSize = 24;
            regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).CNTargetAssignment;
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
            {
                fieldSize = 26;
            }
            else
            {
                fieldSize = 24;
            }
            rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, fieldSize, &value);
        }
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.congestNotification.cnGlobalConfigReg;

        rc = prvCpssHwPpGetRegField(devNum, regAddr, 8, 12, &value);
    }
    if (rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        *enablePtr = BIT2BOOL_MAC((value >> 25) & 0x1);
    }
    else
    {
        *enablePtr = BIT2BOOL_MAC(value & 0x1);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        *targetHwDevPtr = ((value >> 1) & 0x3FF);
        *targetPortPtr = (value >> 11) & 0x3FFF;

        if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            *targetHwDevPtr = ((value >> 1) & 0x3FF);
            *targetPortPtr = (value >> 11) & 0x7FFF;
        }
        else
        {
            *targetHwDevPtr = ((value >> 15) & 0x3FF);
            *targetPortPtr = value & 0x7FFF;
        }
    }
    else
    {
        *targetHwDevPtr = ((value >> 1) & 0x1F);
        *targetPortPtr = ((value >> 6) & 0x3f);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortCnQueueStatusModeEnableGet function
* @endinternal
*
* @brief   Gets the current status of queue status mode.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) status of queue status mode.
*                                      GT_TRUE: CNM is sent to a targetDev/targetPort.
*                                      GT_FALSE: CNM is sent to a sampled packet's source.
* @param[out] targetHwDevPtr           (pointer to) Target HW device number for the CNM.
*                                      Relevant only if enable==GT_TRUE.
* @param[out] targetPortPtr            - (pointer to) Target port for the CNM.
*                                      Relevant only if enable==GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnQueueStatusModeEnableGet
(
    IN  GT_U8           devNum,
    OUT GT_BOOL         *enablePtr,
    OUT GT_HW_DEV_NUM   *targetHwDevPtr,
    OUT GT_PORT_NUM     *targetPortPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnQueueStatusModeEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr, targetHwDevPtr, targetPortPtr));

    rc = internal_cpssDxChPortCnQueueStatusModeEnableGet(devNum, enablePtr, targetHwDevPtr, targetPortPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr, targetHwDevPtr, targetPortPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnSampleEntrySet function
* @endinternal
*
* @brief   Sets CN sample interval entry.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - index: qFb 3 msb. (0..7)
* @param[in] entryPtr                 - (pointer to) entry settings.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, or entry index
* @retval GT_OUT_OF_RANGE          - on out of range interval
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.To implement CCFC functionality set entryPtr fields to:
*       interval = (MTU+256b)/16
*       randBitmap = 0
*       2.To achieve uniform distribution of random values, clear the interval
*       to be randomized to 0. For example, if interval = 0xF0 and
*       randBitmap = 0x0F then the sampling interval is a random value
*       in [0xF0,0xFF].
*
*/
static GT_STATUS internal_cpssDxChPortCnSampleEntrySet
(
    IN GT_U8    devNum,
    IN GT_U8    entryIndex,
    IN CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC *entryPtr
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(entryPtr);
    if (entryIndex > 7)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if (entryPtr->interval > 0xFFFF ||
        entryPtr->randBitmap > 0xFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    value = entryPtr->interval | entryPtr->randBitmap << 16;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).CNSampleTbl[entryIndex];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.queue.congestNotification.cnSampleTbl;
        regAddr += entryIndex*0x4;
    }

    return prvCpssHwPpWriteRegister(devNum, regAddr, value);
}

/**
* @internal cpssDxChPortCnSampleEntrySet function
* @endinternal
*
* @brief   Sets CN sample interval entry.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - index: qFb 3 msb. (0..7)
* @param[in] entryPtr                 - (pointer to) entry settings.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, or entry index
* @retval GT_OUT_OF_RANGE          - on out of range interval
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.To implement CCFC functionality set entryPtr fields to:
*       interval = (MTU+256b)/16
*       randBitmap = 0
*       2.To achieve uniform distribution of random values, clear the interval
*       to be randomized to 0. For example, if interval = 0xF0 and
*       randBitmap = 0x0F then the sampling interval is a random value
*       in [0xF0,0xFF].
*
*/
GT_STATUS cpssDxChPortCnSampleEntrySet
(
    IN GT_U8    devNum,
    IN GT_U8    entryIndex,
    IN CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnSampleEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, entryPtr));

    rc = internal_cpssDxChPortCnSampleEntrySet(devNum, entryIndex, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnSampleEntryGet function
* @endinternal
*
* @brief   Gets CN sample interval entry.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - index: qFb 3 msb. (0..7)
*
* @param[out] entryPtr                 - (pointer to) entry settings.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, or entry index
* @retval GT_OUT_OF_RANGE          - on out of range interval
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnSampleEntryGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    entryIndex,
    OUT CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC *entryPtr
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(entryPtr);
    if (entryIndex > 7)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).CNSampleTbl[entryIndex];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.queue.congestNotification.cnSampleTbl;
        regAddr += entryIndex*0x4;
    }
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &value);
    if (rc != GT_OK)
    {
        return rc;
    }
    entryPtr->interval = value & 0xFFFF;
    entryPtr->randBitmap = value >> 16;

    return GT_OK;
}

/**
* @internal cpssDxChPortCnSampleEntryGet function
* @endinternal
*
* @brief   Gets CN sample interval entry.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - index: qFb 3 msb. (0..7)
*
* @param[out] entryPtr                 - (pointer to) entry settings.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, or entry index
* @retval GT_OUT_OF_RANGE          - on out of range interval
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnSampleEntryGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    entryIndex,
    OUT CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnSampleEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, entryPtr));

    rc = internal_cpssDxChPortCnSampleEntryGet(devNum, entryIndex, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnFbCalcConfigSet function
* @endinternal
*
* @brief   Sets Fb (Feedback) calculation configuration.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] fbCalcCfgPtr             - (pointer to) feedback Calculation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To implement CCFC functionality set fbCalcCfgPtr fields to:
*       deltaEnable = 0
*
*/
static GT_STATUS internal_cpssDxChPortCnFbCalcConfigSet
(
    IN GT_U8    devNum,
    IN CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC *fbCalcCfgPtr
)
{
    GT_U32 value;     /* register value */
    GT_U32 wExpValue; /* wExp hw value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */
    GT_U32 fbMinMaxNumOfBits; /* number of bits in fbMin and fbMax fields */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(fbCalcCfgPtr);

    if(GT_TRUE == PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(devNum))
    {
        fbMinMaxNumOfBits = 24;
    }
    else
    {
        fbMinMaxNumOfBits = 20;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
    {
        if(fbCalcCfgPtr->fbMin > BIT_MASK_MAC(fbMinMaxNumOfBits) ||
          fbCalcCfgPtr->fbMax > BIT_MASK_MAC(fbMinMaxNumOfBits) ||
          fbCalcCfgPtr->fbLsb > 0x0E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        if (fbCalcCfgPtr->wExp > 7 || fbCalcCfgPtr->wExp < -8 ||
            fbCalcCfgPtr->fbMin > BIT_MASK_MAC(fbMinMaxNumOfBits) ||
            fbCalcCfgPtr->fbMax > BIT_MASK_MAC(fbMinMaxNumOfBits) ||
            fbCalcCfgPtr->fbLsb > 0x1F)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.globalQcnConfig;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 21, 4, fbCalcCfgPtr->fbLsb);
    }
    else
    {
        /* Wexp is given in 2s complement format */
        if (fbCalcCfgPtr->wExp >= 0)
        {
            wExpValue = (GT_U32)fbCalcCfgPtr->wExp;
        }
        else
        {
            wExpValue =(GT_U32)(16 + fbCalcCfgPtr->wExp);
        }

        /* set Feedback Calc Configurations register */
        value = BOOL2BIT_MAC(fbCalcCfgPtr->deltaEnable) |
          fbCalcCfgPtr->fbLsb << 1 |
          wExpValue << 6;

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).feedbackCalcConfigs;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
              txqVer1.queue.congestNotification.fbCalcConfigReg;
        }
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 10, value);
    }

    if (rc != GT_OK)
    {
        return rc;
    }

    /* set Feedback MIN register */
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.fbMin;
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).feedbackMIN;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.queue.congestNotification.fbMinReg;
    }
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, fbMinMaxNumOfBits, fbCalcCfgPtr->fbMin);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* set Feedback MAX register */
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.fbMax;
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).feedbackMAX;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.queue.congestNotification.fbMaxReg;
    }
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, fbMinMaxNumOfBits, fbCalcCfgPtr->fbMax);

}

/**
* @internal cpssDxChPortCnFbCalcConfigSet function
* @endinternal
*
* @brief   Sets Fb (Feedback) calculation configuration.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] fbCalcCfgPtr             - (pointer to) feedback Calculation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To implement CCFC functionality set fbCalcCfgPtr fields to:
*       deltaEnable = 0
*
*/
GT_STATUS cpssDxChPortCnFbCalcConfigSet
(
    IN GT_U8    devNum,
    IN CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC *fbCalcCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnFbCalcConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, fbCalcCfgPtr));

    rc = internal_cpssDxChPortCnFbCalcConfigSet(devNum, fbCalcCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, fbCalcCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnFbCalcConfigGet function
* @endinternal
*
* @brief   Gets Fb (Feedback) calculation configuration.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
*
* @param[out] fbCalcCfgPtr             - (pointer to) feedback Calculation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnFbCalcConfigGet
(
    IN  GT_U8    devNum,
    OUT CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC *fbCalcCfgPtr
)
{
    GT_U32 value;     /* register value */
    GT_U32 wExpValue; /* wExp hw value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */
    GT_U32 fbMinMaxNumOfBits; /* number of bits in fbMin and fbMax fields */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(fbCalcCfgPtr);

    if(GT_TRUE == PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(devNum))
    {
        fbMinMaxNumOfBits = 24;
    }
    else
    {
        fbMinMaxNumOfBits = 20;
    }

    /* get Feedback Calc Configurations register */
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.globalQcnConfig;
        rc = prvCpssHwPpReadRegister(devNum, regAddr, &value);
        fbCalcCfgPtr->fbLsb = U32_GET_FIELD_MAC(value, 21, 4);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).feedbackCalcConfigs;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
              txqVer1.queue.congestNotification.fbCalcConfigReg;
        }
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 10, &value);
        if (rc != GT_OK)
        {
            return rc;
        }
        fbCalcCfgPtr->deltaEnable = BIT2BOOL_MAC(value & 0x1);
        fbCalcCfgPtr->fbLsb = (value >> 1) & 0x1F;

        /* Wexp is given in 2s complement format */
        wExpValue = (value >> 6) & 0xF;
        if (wExpValue < 8)
        {
            fbCalcCfgPtr->wExp = wExpValue;
        }
        else
        {
            fbCalcCfgPtr->wExp = wExpValue - 16;
        }
    }

    /* get Feedback MIN register */
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.fbMin;
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).feedbackMIN;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.queue.congestNotification.fbMinReg;
    }
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, fbMinMaxNumOfBits, &(fbCalcCfgPtr->fbMin));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get Feedback MAX register */
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.fbMax;;
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).feedbackMAX;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.queue.congestNotification.fbMaxReg;
    }
    return prvCpssHwPpGetRegField(devNum, regAddr, 0, fbMinMaxNumOfBits, &(fbCalcCfgPtr->fbMax));
}

/**
* @internal cpssDxChPortCnFbCalcConfigGet function
* @endinternal
*
* @brief   Gets Fb (Feedback) calculation configuration.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
*
* @param[out] fbCalcCfgPtr             - (pointer to) feedback Calculation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnFbCalcConfigGet
(
    IN  GT_U8    devNum,
    OUT CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC *fbCalcCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnFbCalcConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, fbCalcCfgPtr));

    rc = internal_cpssDxChPortCnFbCalcConfigGet(devNum, fbCalcCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, fbCalcCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnPacketLengthSet function
* @endinternal
*
* @brief   Sets packet length used by the CN frames triggering logic.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] packetLength             - packet length.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or packet length.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Configuring a value other than CPSS_DXCH_PORT_CN_LENGTH_ORIG_PACKET_E
*       should be used when the packet length is unknown, e.g. Cut-through mode.
*
*/
static GT_STATUS internal_cpssDxChPortCnPacketLengthSet
(
    IN GT_U8                               devNum,
    IN CPSS_DXCH_PORT_CN_PACKET_LENGTH_ENT packetLength
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_FALCON_E);

    switch (packetLength)
    {
        case CPSS_DXCH_PORT_CN_LENGTH_ORIG_PACKET_E:
            value = 0;
            break;
        case CPSS_DXCH_PORT_CN_LENGTH_1_5_KB_E:
            value = 1;
            break;
        case CPSS_DXCH_PORT_CN_LENGTH_2_KB_E:
            value = 2;
            break;
        case CPSS_DXCH_PORT_CN_LENGTH_10_KB_E:
            value = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).CNGlobalConfig;
        return prvCpssHwPpSetRegField(devNum, regAddr, 2, 2, value);
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.congestNotification.cnGlobalConfigReg;
        return prvCpssHwPpSetRegField(devNum, regAddr, 6, 2, value);
    }
}

/**
* @internal cpssDxChPortCnPacketLengthSet function
* @endinternal
*
* @brief   Sets packet length used by the CN frames triggering logic.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] packetLength             - packet length.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or packet length.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Configuring a value other than CPSS_DXCH_PORT_CN_LENGTH_ORIG_PACKET_E
*       should be used when the packet length is unknown, e.g. Cut-through mode.
*
*/
GT_STATUS cpssDxChPortCnPacketLengthSet
(
    IN GT_U8                               devNum,
    IN CPSS_DXCH_PORT_CN_PACKET_LENGTH_ENT packetLength
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnPacketLengthSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, packetLength));

    rc = internal_cpssDxChPortCnPacketLengthSet(devNum, packetLength);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, packetLength));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnPacketLengthGet function
* @endinternal
*
* @brief   Gets packet length used by the CN frames triggering logic.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Falcon.
*
* @param[in] devNum                   - device number.
*
* @param[out] packetLengthPtr          - (pointer to) packet length.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnPacketLengthGet
(
    IN  GT_U8    devNum,
    OUT CPSS_DXCH_PORT_CN_PACKET_LENGTH_ENT  *packetLengthPtr
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(packetLengthPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).CNGlobalConfig;
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 2, 2, &value);
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.congestNotification.cnGlobalConfigReg;
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 6, 2, &value);
    }

    if (rc != GT_OK)
    {
        return rc;
    }
    *packetLengthPtr = (CPSS_DXCH_PORT_CN_PACKET_LENGTH_ENT)value;
    return GT_OK;
}

/**
* @internal cpssDxChPortCnPacketLengthGet function
* @endinternal
*
* @brief   Gets packet length used by the CN frames triggering logic.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Falcon.
*
* @param[in] devNum                   - device number.
*
* @param[out] packetLengthPtr          - (pointer to) packet length.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnPacketLengthGet
(
    IN  GT_U8    devNum,
    OUT CPSS_DXCH_PORT_CN_PACKET_LENGTH_ENT  *packetLengthPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnPacketLengthGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, packetLengthPtr));

    rc = internal_cpssDxChPortCnPacketLengthGet(devNum, packetLengthPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, packetLengthPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnMessageGenerationConfigSet function
* @endinternal
*
* @brief   Sets CNM generation configuration.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] cnmGenerationCfgPtr      - (pointer to) CNM generation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnMessageGenerationConfigSet
(
    IN GT_U8    devNum,
    IN CPSS_PORT_CNM_GENERATION_CONFIG_STC *cnmGenerationCfgPtr
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_U32 scaleFactorHwValue;
    GT_STATUS rc;     /* function return value */
    GT_U32 fieldLength;  /* field length */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E);

    CPSS_NULL_PTR_CHECK_MAC(cnmGenerationCfgPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if ((cnmGenerationCfgPtr->qosProfileId > 0x3FF) ||
            (cnmGenerationCfgPtr->sourceId >= BIT_12) ||
            (cnmGenerationCfgPtr->scaleFactor != 256 && cnmGenerationCfgPtr->scaleFactor != 512) ||
            (cnmGenerationCfgPtr->isRouted > 1) ||
            (cnmGenerationCfgPtr->version > 0xF))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        /* set CN Descriptor Attributes Register */
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            value = ((cnmGenerationCfgPtr->sourceId << 12) |
                    (BOOL2BIT_MAC(cnmGenerationCfgPtr->overrideUp)) |
                    (cnmGenerationCfgPtr->isRouted << 1) |
                    (cnmGenerationCfgPtr->qosProfileId << 2));
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EREP.configurations.qcnConfigs0;
        }
        else
        {
            value = (cnmGenerationCfgPtr->sourceId |
                    (BOOL2BIT_MAC(cnmGenerationCfgPtr->overrideUp) << 23) |
                    (cnmGenerationCfgPtr->isRouted << 22) |
                    (cnmGenerationCfgPtr->qosProfileId << 12));
            regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).CNDescAttributes;
        }
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 24, value);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Set HA CNM Header Configuration Register */
        scaleFactorHwValue = (cnmGenerationCfgPtr->scaleFactor == 256) ? 1 : 0;
        value = cnmGenerationCfgPtr->version << 16 |
                scaleFactorHwValue << 20;
        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).CNMHeaderConfig;
        rc = prvCpssHwPpWriteRegBitMask(devNum, regAddr, 0x1F0000, value);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* set HA CPID Register 0  and CPID Register 1*/
        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).CPIDReg0;

        /* Falcon has 18 bits for CPID Reg0 as 5 bits are used to denote one of
         * 32 queues
         */
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            value = ((cnmGenerationCfgPtr->cpidMsb[4] << 9) |
                     (cnmGenerationCfgPtr->cpidMsb[5] << 1) |
                     (cnmGenerationCfgPtr->cpidMsb[6] >> 7));

            fieldLength = 18;
        }
        else if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            value = ((cnmGenerationCfgPtr->cpidMsb[4] << 9) |
                     (cnmGenerationCfgPtr->cpidMsb[5] << 1) |
                     (cnmGenerationCfgPtr->cpidMsb[6] >> 7));

            fieldLength = 17;
        }
        else
        {
            value = ((cnmGenerationCfgPtr->cpidMsb[4] << 12) |
                     (cnmGenerationCfgPtr->cpidMsb[5] << 4) |
                     (cnmGenerationCfgPtr->cpidMsb[6] >> 4));

            fieldLength = 20;
        }

        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, fieldLength, value);
        if (rc != GT_OK)
        {
            return rc;
        }

        value = (cnmGenerationCfgPtr->cpidMsb[3] |
                     (cnmGenerationCfgPtr->cpidMsb[2] << 8) |
                     (cnmGenerationCfgPtr->cpidMsb[1] << 16) |
                     (cnmGenerationCfgPtr->cpidMsb[0] << 24));
        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).CPIDReg1;
        rc = prvCpssHwPpWriteRegister(devNum, regAddr, value);
        if (rc != GT_OK)
        {
            return rc;
        }

         /* set CN Global Configurations register */
        value = BOOL2BIT_MAC(cnmGenerationCfgPtr->cnUntaggedEnable);

        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* Yet unknown the relevant HW */
            rc = GT_OK;
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).CNGlobalConfig;

            if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
            {
                /* set QCN Src Info */
                value = (value << 6)| (BOOL2BIT_MAC(cnmGenerationCfgPtr->keepQcnSrcInfo) << 8);
                rc = prvCpssHwPpWriteRegBitMask(devNum, regAddr, 0x0140, value);
            }
            else
            {
                /* SIP5 */
                rc = prvCpssHwPpSetRegField(devNum, regAddr, 6, 1, value);
            }
        }
        if (rc != GT_OK)
        {
            return rc;
        }

        /* set HA Congestion Configuration Register */
        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).congestionNotificationConfig;

        /* append ingress packet header with QCN for Aldrin 2 and Falcon */
        if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum) || PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            value = (BOOL2BIT_MAC(cnmGenerationCfgPtr->forceCnTag)|
                    (BOOL2BIT_MAC(cnmGenerationCfgPtr->appendPacket) << 1));
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 2, value);
        }
        /* SIP5 */
        else
        {
            value = BOOL2BIT_MAC(cnmGenerationCfgPtr->forceCnTag);
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, value);
        }
        if (rc != GT_OK)
        {
            return rc;
        }

        /* set HA QCN Modifications Register */
        /* Globally configure QOffset and QDelta to use in QCN hdr */
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            value = cnmGenerationCfgPtr->qcnQOffset | cnmGenerationCfgPtr->qcnQDelta << 16;
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->HA.QCNModifications;
            rc = prvCpssHwPpWriteRegister(devNum, regAddr, value);
            if (rc != GT_OK)
            {
                return rc;
            }
        }


    }
    else if((CPSS_PP_FAMILY_DXCH_LION_E == PRV_CPSS_PP_MAC(devNum)->devFamily)||
            (CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        if (cnmGenerationCfgPtr->qosProfileId > 0x7F ||
            cnmGenerationCfgPtr->isRouted > 1 ||
            (cnmGenerationCfgPtr->overrideUp == GT_TRUE && cnmGenerationCfgPtr->cnmUp > 7) ||
            cnmGenerationCfgPtr->defaultVlanId > 0xFFF ||
            (cnmGenerationCfgPtr->scaleFactor != 256 &&
             cnmGenerationCfgPtr->scaleFactor != 512) ||
            cnmGenerationCfgPtr->version > 0xF ||
            cnmGenerationCfgPtr->flowIdTag > 0xFFFF)

        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        /* set Congestion Notification Configuration register */
        value = cnmGenerationCfgPtr->defaultVlanId |
                (BOOL2BIT_MAC(cnmGenerationCfgPtr->overrideUp) << 16) |
                ((cnmGenerationCfgPtr->cnmUp & 0x7) << 17) |
                (cnmGenerationCfgPtr->isRouted << 20) |
                (cnmGenerationCfgPtr->qosProfileId << 21) |
                (BOOL2BIT_MAC(cnmGenerationCfgPtr->forceCnTag) << 28);
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltCnConfig;
        rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, 0x1FFF0FFF, value);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* set CNM Header Configuration register*/
        scaleFactorHwValue = (cnmGenerationCfgPtr->scaleFactor == 256) ? 1 : 0;

        value = cnmGenerationCfgPtr->version << 16 |
                scaleFactorHwValue << 23 |
                ((!BOOL2BIT_MAC(cnmGenerationCfgPtr->appendPacket)) << 25);


        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltCnmHeaderConfig;
        rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, 0x28F0000, value);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* set CPID Register 0  and CPID Register 1*/
        value = ((cnmGenerationCfgPtr->cpidMsb[4] << 14) |
                     (cnmGenerationCfgPtr->cpidMsb[5] << 6) |
                     (cnmGenerationCfgPtr->cpidMsb[6] >> 2));


        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltCpidReg0;
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 22, value);
        if (rc != GT_OK)
        {
            return rc;
        }
        value = (cnmGenerationCfgPtr->cpidMsb[3] |
                     (cnmGenerationCfgPtr->cpidMsb[2] << 8) |
                     (cnmGenerationCfgPtr->cpidMsb[1] << 16) |
                     (cnmGenerationCfgPtr->cpidMsb[0] << 24));
        regAddr += 4;
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, value);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* set QCN CN-tag FlowID register*/
        value = cnmGenerationCfgPtr->flowIdTag;

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltCnTagFlowId;
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 16, value);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* set CN Global Configurations register */
        value = BOOL2BIT_MAC(cnmGenerationCfgPtr->cnUntaggedEnable);

        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).CNGlobalConfig;
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 6, 1, value);
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.congestNotification.cnGlobalConfigReg;
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 22, 1, value);
        }
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else /* XCAT2 */
    {
        if (cnmGenerationCfgPtr->qosProfileId > 0x7F ||
            cnmGenerationCfgPtr->defaultVlanId > 0xFFF)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        /* set Congestion Notification Configuration register */
        value = cnmGenerationCfgPtr->defaultVlanId |
                (cnmGenerationCfgPtr->qosProfileId << 21);
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltCnConfig;
        rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, 0xFE00FFF, value);
        if (rc != GT_OK)
        {
            return rc;
        }

        value = (!BOOL2BIT_MAC(cnmGenerationCfgPtr->appendPacket));
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltCnmHeaderConfig;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 25, 1, value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortCnMessageGenerationConfigSet function
* @endinternal
*
* @brief   Sets CNM generation configuration.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] cnmGenerationCfgPtr      - (pointer to) CNM generation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnMessageGenerationConfigSet
(
    IN GT_U8    devNum,
    IN CPSS_PORT_CNM_GENERATION_CONFIG_STC *cnmGenerationCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnMessageGenerationConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cnmGenerationCfgPtr));

    rc = internal_cpssDxChPortCnMessageGenerationConfigSet(devNum, cnmGenerationCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cnmGenerationCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnMessageGenerationConfigGet function
* @endinternal
*
* @brief   Gets CNM generation configuration.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number.
*
* @param[out] cnmGenerationCfgPtr      - (pointer to) CNM generation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnMessageGenerationConfigGet
(
    IN  GT_U8    devNum,
    OUT CPSS_PORT_CNM_GENERATION_CONFIG_STC *cnmGenerationCfgPtr
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */
    GT_U32 fieldLength;  /* field length */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E);
    CPSS_NULL_PTR_CHECK_MAC(cnmGenerationCfgPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        cpssOsMemSet(cnmGenerationCfgPtr, 0, sizeof(CPSS_PORT_CNM_GENERATION_CONFIG_STC));

        /* Get CN Descriptor Attributes Register */
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EREP.configurations.qcnConfigs0;
            rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 24, &value);
            if(GT_OK != rc)
            {
                return rc;
            }
            cnmGenerationCfgPtr->sourceId = U32_GET_FIELD_MAC(value, 12, 12);
            cnmGenerationCfgPtr->qosProfileId = U32_GET_FIELD_MAC(value, 2, 10);
            cnmGenerationCfgPtr->isRouted = U32_GET_FIELD_MAC(value, 1, 1);
            cnmGenerationCfgPtr->overrideUp = BIT2BOOL_MAC((U32_GET_FIELD_MAC(value, 0, 1)));
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).CNDescAttributes;
            rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 24, &value);
            if(GT_OK != rc)
            {
                return rc;
            }
            cnmGenerationCfgPtr->sourceId = U32_GET_FIELD_MAC(value, 0, 12);
            cnmGenerationCfgPtr->qosProfileId = U32_GET_FIELD_MAC(value, 12, 10);
            cnmGenerationCfgPtr->isRouted = U32_GET_FIELD_MAC(value, 22, 1);
            cnmGenerationCfgPtr->overrideUp = BIT2BOOL_MAC((U32_GET_FIELD_MAC(value, 23, 1)));
        }

        /* Get HA CNM Header Configuration Register */
        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).CNMHeaderConfig;
        rc = prvCpssHwPpReadRegister(devNum, regAddr, &value);
        if (rc != GT_OK)
        {
            return rc;
        }
        cnmGenerationCfgPtr->scaleFactor = (U32_GET_FIELD_MAC(value, 20, 1) == 1) ? 256 : 512;
        cnmGenerationCfgPtr->version = U32_GET_FIELD_MAC(value, 16, 4);

        /* set HA CPID Register 0  and CPID Register 1*/
        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).CPIDReg0;

        fieldLength = (PRV_CPSS_SIP_6_CHECK_MAC(devNum) ? 18 : (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ? 17 : 20));

        rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, fieldLength, &value);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            cnmGenerationCfgPtr->cpidMsb[4] = U32_GET_FIELD_MAC(value, 9, 8);
            cnmGenerationCfgPtr->cpidMsb[5] = U32_GET_FIELD_MAC(value, 1, 8);
            cnmGenerationCfgPtr->cpidMsb[6] = U32_GET_FIELD_MAC(value, 0, 1) << 7;
        }
        else if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            cnmGenerationCfgPtr->cpidMsb[4] = U32_GET_FIELD_MAC(value, 9, 8);
            cnmGenerationCfgPtr->cpidMsb[5] = U32_GET_FIELD_MAC(value, 1, 8);
            cnmGenerationCfgPtr->cpidMsb[6] = U32_GET_FIELD_MAC(value, 0, 1) << 7;
        }
        else
        {
            cnmGenerationCfgPtr->cpidMsb[4] = U32_GET_FIELD_MAC(value, 12, 8);
            cnmGenerationCfgPtr->cpidMsb[5] = U32_GET_FIELD_MAC(value, 4, 8);
            cnmGenerationCfgPtr->cpidMsb[6] = U32_GET_FIELD_MAC(value, 0, 4) << 4;
        }


        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).CPIDReg1;
        rc = prvCpssHwPpReadRegister(devNum, regAddr, &value);
        if (rc != GT_OK)
        {
            return rc;
        }
        cnmGenerationCfgPtr->cpidMsb[3] = U32_GET_FIELD_MAC(value, 0, 8);
        cnmGenerationCfgPtr->cpidMsb[2] = U32_GET_FIELD_MAC(value, 8, 8);
        cnmGenerationCfgPtr->cpidMsb[1] = U32_GET_FIELD_MAC(value, 16, 8);
        cnmGenerationCfgPtr->cpidMsb[0] = U32_GET_FIELD_MAC(value, 24, 8);

        /* Get CN Global Configurations register */
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* Yet unknown the relevant HW */
            cnmGenerationCfgPtr->cnUntaggedEnable = GT_FALSE;
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).CNGlobalConfig;
            rc = prvCpssHwPpGetRegField(devNum, regAddr, 6, 1, &value);

            if (rc != GT_OK)
            {
                return rc;
            }

            cnmGenerationCfgPtr->cnUntaggedEnable = BIT2BOOL_MAC(value);

            if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
            {
                /* Get Qcn Src Info */
                rc = prvCpssHwPpGetRegField(devNum, regAddr, 8, 1, &value);
                if (rc != GT_OK)
                {
                    return rc;
                }
                cnmGenerationCfgPtr->keepQcnSrcInfo = BIT2BOOL_MAC(value);
            }
        }

        /* Get HA Congestion Configuration Register */
        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).congestionNotificationConfig;
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, &value);
        if (rc != GT_OK)
        {
            return rc;
        }
        cnmGenerationCfgPtr->forceCnTag = BIT2BOOL_MAC(value);
        if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
        {
            /* Get Qcn Src Info */
            rc = prvCpssHwPpGetRegField(devNum, regAddr, 1, 1, &value);
            if (rc != GT_OK)
            {
                return rc;
            }
            cnmGenerationCfgPtr->appendPacket = BIT2BOOL_MAC(value);
        }
        /* Get HA QCN Modifications Register */
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->HA.QCNModifications;
            rc = prvCpssHwPpReadRegister(devNum, regAddr, &value);
            if (rc != GT_OK)
            {
                return rc;
            }
            cnmGenerationCfgPtr->qcnQOffset = value & 0xFFFF;
            cnmGenerationCfgPtr->qcnQDelta = value >> 16;
        }
    }
    else if((CPSS_PP_FAMILY_DXCH_LION_E == PRV_CPSS_PP_MAC(devNum)->devFamily)||
       (CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        /* get Congestion Notification Configuration register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltCnConfig;
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr,&value);
        if (rc != GT_OK)
        {
            return rc;
        }
        cnmGenerationCfgPtr->defaultVlanId = (GT_U16)(value & 0xFFF);
        cnmGenerationCfgPtr->overrideUp = BIT2BOOL_MAC((value>>16) & 0x1);
        cnmGenerationCfgPtr->cnmUp = (GT_U8)((value >> 17) & 0x7);
        cnmGenerationCfgPtr->isRouted = (GT_U8)((value >> 20) & 0x1);
        cnmGenerationCfgPtr->qosProfileId = (value >> 21) & 0x7F;
        cnmGenerationCfgPtr->forceCnTag = BIT2BOOL_MAC((value>>28) & 0x1);

        /* get CNM Header Configuration register*/
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltCnmHeaderConfig;
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &value);
        if (rc != GT_OK)
        {
            return rc;
        }

        cnmGenerationCfgPtr->version = value >> 16 & 0xf;
        cnmGenerationCfgPtr->scaleFactor = ((value >> 23) & 0x1) ? 256 : 512;
        cnmGenerationCfgPtr->appendPacket =  BIT2BOOL_MAC((!(value>>25) & 0x1));

        /* get CPID Register 0  and CPID Register 1*/
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltCpidReg0;
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &value);
        if (rc != GT_OK)
        {
            return rc;
        }

        cnmGenerationCfgPtr->cpidMsb[4] = (GT_U8)((value >> 14) & 0xFF);
        cnmGenerationCfgPtr->cpidMsb[5] = (GT_U8)((value >> 6) & 0xFF);
        cnmGenerationCfgPtr->cpidMsb[6] = (GT_U8)((value << 2) & 0xFF);

        regAddr += 4;
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr,&value);
        if (rc != GT_OK)
        {
            return rc;
        }
        cnmGenerationCfgPtr->cpidMsb[0] = (GT_U8)((value >> 24) & 0xFF);
        cnmGenerationCfgPtr->cpidMsb[1] = (GT_U8)((value >> 16) & 0xFF);
        cnmGenerationCfgPtr->cpidMsb[2] = (GT_U8)((value >> 8) & 0xFF);
        cnmGenerationCfgPtr->cpidMsb[3] = (GT_U8)(value & 0xFF);

        /* get QCN CN-tag FlowID register*/
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltCnTagFlowId;
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 0,16, &value);
        if (rc != GT_OK)
        {
            return rc;
        }
        cnmGenerationCfgPtr->flowIdTag = value;

        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).CNGlobalConfig;
            rc = prvCpssHwPpGetRegField(devNum, regAddr, 6, 1, &value);
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.congestNotification.cnGlobalConfigReg;
            rc = prvCpssHwPpGetRegField(devNum, regAddr, 22,1, &value);
        }
        if (rc != GT_OK)
        {
            return rc;
        }
        cnmGenerationCfgPtr->cnUntaggedEnable = BIT2BOOL_MAC(value);
    }
    else /* XCAT2 */
    {
        /* get Congestion Notification Configuration register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltCnConfig;
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr,&value);
        if (rc != GT_OK)
        {
            return GT_OK;
        }
        cnmGenerationCfgPtr->defaultVlanId = (GT_U16)(value & 0xFFF);
        cnmGenerationCfgPtr->qosProfileId = (value >> 21) & 0x7F;

        /* get CNM Header Configuration register*/
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltCnmHeaderConfig;
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &value);
        if (rc != GT_OK)
        {
            return rc;
        }

        cnmGenerationCfgPtr->appendPacket =  BIT2BOOL_MAC((!(value>>25) & 0x1));
    }
    return GT_OK;
}

/**
* @internal cpssDxChPortCnMessageGenerationConfigGet function
* @endinternal
*
* @brief   Gets CNM generation configuration.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number.
*
* @param[out] cnmGenerationCfgPtr      - (pointer to) CNM generation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnMessageGenerationConfigGet
(
    IN  GT_U8    devNum,
    OUT CPSS_PORT_CNM_GENERATION_CONFIG_STC *cnmGenerationCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnMessageGenerationConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cnmGenerationCfgPtr));

    rc = internal_cpssDxChPortCnMessageGenerationConfigGet(devNum, cnmGenerationCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cnmGenerationCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnPrioritySpeedLocationSet function
* @endinternal
*
* @brief   Sets location of the priority/speed bits in the CNM header for parsing
*         and generation of CNM.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] prioritySpeedLocation    - priority speed location.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnPrioritySpeedLocationSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_PORT_CN_PRIORITY_SPEED_LOCATION_ENT prioritySpeedLocation
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */
    GT_U32    startBit;          /* start bit of the field */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E);

    switch (prioritySpeedLocation)
    {
        case CPSS_DXCH_PORT_CN_PRIORITY_SPEED_CPID_E:
            value = 0;
            break;
        case CPSS_DXCH_PORT_CN_SDU_UP_E:
            value = 1;
            break;
        case CPSS_DXCH_PORT_CN_SPEED_E:
            value = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).haGlobalConfig;
            startBit = 16;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltGlobalConfig;
            startBit = 19;
        }

        rc =  prvCpssHwPpSetRegField(devNum, regAddr, startBit, 2, value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.globalUnitConfig;
    }
    return prvCpssHwPpSetRegField(devNum, regAddr, 8, 2, value);

}

/**
* @internal cpssDxChPortCnPrioritySpeedLocationSet function
* @endinternal
*
* @brief   Sets location of the priority/speed bits in the CNM header for parsing
*         and generation of CNM.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] prioritySpeedLocation    - priority speed location.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnPrioritySpeedLocationSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_PORT_CN_PRIORITY_SPEED_LOCATION_ENT prioritySpeedLocation
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnPrioritySpeedLocationSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, prioritySpeedLocation));

    rc = internal_cpssDxChPortCnPrioritySpeedLocationSet(devNum, prioritySpeedLocation);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, prioritySpeedLocation));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnPrioritySpeedLocationGet function
* @endinternal
*
* @brief   Gets location of the index (priority/speed) bits in the CNM header
*         for parsing and generation of CNM.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number.
*
* @param[out] prioritySpeedLocationPtr - (pointer to) priority speed location.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnPrioritySpeedLocationGet
(
    IN  GT_U8                                devNum,
    OUT CPSS_DXCH_PORT_CN_PRIORITY_SPEED_LOCATION_ENT *prioritySpeedLocationPtr
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */
    GT_U32    startBit;          /* start bit of the field */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E);

    CPSS_NULL_PTR_CHECK_MAC(prioritySpeedLocationPtr);

    if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).haGlobalConfig;
            startBit = 16;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltGlobalConfig;
            startBit = 19;
        }

        rc =  prvCpssHwPpGetRegField(devNum, regAddr, startBit, 2, &value);
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfig;
        rc =  prvCpssHwPpGetRegField(devNum, regAddr, 8, 2, &value);
    }

    if (rc != GT_OK)
    {
        return rc;
    }
    *prioritySpeedLocationPtr = (CPSS_DXCH_PORT_CN_PRIORITY_SPEED_LOCATION_ENT)value;
    return GT_OK;
}

/**
* @internal cpssDxChPortCnPrioritySpeedLocationGet function
* @endinternal
*
* @brief   Gets location of the index (priority/speed) bits in the CNM header
*         for parsing and generation of CNM.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number.
*
* @param[out] prioritySpeedLocationPtr - (pointer to) priority speed location.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnPrioritySpeedLocationGet
(
    IN  GT_U8                                devNum,
    OUT CPSS_DXCH_PORT_CN_PRIORITY_SPEED_LOCATION_ENT *prioritySpeedLocationPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnPrioritySpeedLocationGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, prioritySpeedLocationPtr));

    rc = internal_cpssDxChPortCnPrioritySpeedLocationGet(devNum, prioritySpeedLocationPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, prioritySpeedLocationPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @enum PRV_CPSS_PORT_CN_DB_4_QCN_MSG_ENT
 *
 * @brief Data base type for egf and qcn messages to pfc.
*/
typedef enum{

    /** CCFC data base type */
    PRV_CPSS_PORT_CN_DB_4_QCN_MSG_CCFC_E = 0,

    /** QCN data base type */
    PRV_CPSS_PORT_CN_DB_4_QCN_MSG_QCN_E

} PRV_CPSS_PORT_CN_DB_4_QCN_MSG_ENT;

/**
* @internal prvCpssDxChPortCnEgfQcnMsg2PfcTypeSet function
* @endinternal
*
* @brief   confgure Data base type for egf and qcn messages to pfc.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
*
* @param[in] devNum                   - device number.
* @param[in] type                     - data base type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number,portNum,tc
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortCnEgfQcnMsg2PfcTypeSet
(
    IN  GT_U8                         devNum,
    IN  PRV_CPSS_PORT_CN_DB_4_QCN_MSG_ENT type
)
{
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 regVal;
    GT_U32 bitIndex;  /* bit index */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    switch (type)
    {
        case PRV_CPSS_PORT_CN_DB_4_QCN_MSG_CCFC_E:
            regVal = 0;
        break;
        case PRV_CPSS_PORT_CN_DB_4_QCN_MSG_QCN_E:
            regVal = 1;
        break;
        default:
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        bitIndex = 13;
    }
    else
    {
        bitIndex = 10;
    }


    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TXQ.pfc.PFCTriggerGlobalConfig;
    rc = prvCpssHwPpPortGroupSetRegField(devNum, 0, regAddr,
                                            bitIndex,
                                            1,
                                            regVal);
    return rc;
}

/**
* @internal internal_cpssDxChPortCnMessageTypeSet function
* @endinternal
*
* @brief   Sets congestion notification message type - QCN or CCFC
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
*
* @param[in] devNum                   - device number.
* @param[in] mType                    - CN message type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnMessageTypeSet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PORT_CN_MESSAGE_TYPE_ENT   mType
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */
    PRV_CPSS_PORT_CN_DB_4_QCN_MSG_ENT type;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    switch (mType)
    {
        case CPSS_DXCH_PORT_CN_MESSAGE_TYPE_QCN_E:
            value = 0;
            type = PRV_CPSS_PORT_CN_DB_4_QCN_MSG_QCN_E;
            break;
        case CPSS_DXCH_PORT_CN_MESSAGE_TYPE_CCFC_E:
            value = 1;
            type = PRV_CPSS_PORT_CN_DB_4_QCN_MSG_CCFC_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).CNGlobalConfig;

    rc =  prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortCnEgfQcnMsg2PfcTypeSet(devNum,type);
    return rc;
}

/**
* @internal cpssDxChPortCnMessageTypeSet function
* @endinternal
*
* @brief   Sets congestion notification message type - QCN or CCFC
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
*
* @param[in] devNum                   - device number.
* @param[in] mType                    - CN message type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnMessageTypeSet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PORT_CN_MESSAGE_TYPE_ENT   mType
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnMessageTypeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mType));

    rc = internal_cpssDxChPortCnMessageTypeSet(devNum, mType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mType));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortCnMessageTypeGet function
* @endinternal
*
* @brief   Gets congestion notification message type - QCN or CCFC
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
*
* @param[in] devNum                   - device number.
*
* @param[out] mTypePtr                 - (pointer to) CN message type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnMessageTypeGet
(
    IN  GT_U8                                devNum,
    OUT CPSS_DXCH_PORT_CN_MESSAGE_TYPE_ENT   *mTypePtr
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(mTypePtr);

    regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).CNGlobalConfig;

    rc =  prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, &value);
    if(GT_OK != rc)
    {
        return rc;
    }

    *mTypePtr = (value == 0) ? CPSS_DXCH_PORT_CN_MESSAGE_TYPE_QCN_E : CPSS_DXCH_PORT_CN_MESSAGE_TYPE_CCFC_E;

    return GT_OK;
}

/**
* @internal cpssDxChPortCnMessageTypeGet function
* @endinternal
*
* @brief   Gets congestion notification message type - QCN or CCFC
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
*
* @param[in] devNum                   - device number.
*
* @param[out] mTypePtr                 - (pointer to) CN message type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnMessageTypeGet
(
    IN  GT_U8                                devNum,
    OUT CPSS_DXCH_PORT_CN_MESSAGE_TYPE_ENT   *mTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnMessageTypeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mTypePtr));

    rc = internal_cpssDxChPortCnMessageTypeGet(devNum, mTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortCnMessageTriggeringStateSet function
* @endinternal
*
* @brief   Sets CNM Triggering State Variables
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] qcnSampleInterval        -  parameter that defines the probability to trigger
*                                      the next CN frame based on the current value of the
*                                      calculated feedback (Fb), measured in Bytes.
*                                      (APPLICABLE RANGES: 0..0xFFFFF)
* @param[in] qlenOld                  - a snapshot of current length of the queue in buffers,
*                                      taken when the QcnSampleInterval expires.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*                                      Aldrin2: (APPLICABLE RANGES: 0..0xFFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number,portNum,tc
* @retval GT_OUT_OF_RANGE          - on wrong qcnSampleInterval, qlenOld
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note qlenOld reset value and QcnSampleInterval init value must be set by the
*       application prior to configuring the queue to be CN aware
*
*/
static GT_STATUS internal_cpssDxChPortCnMessageTriggeringStateSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   tcQueue,
    IN  GT_U32                  qcnSampleInterval,
    IN  GT_U32                  qlenOld
)
{
    GT_STATUS               rc;             /* return code */
    GT_U32                  txqPortNum;     /* TXQ port number */
    GT_U32                  hwArray[2];     /* table HW data */
    GT_U32                  tableIndex;     /* table index */
    CPSS_DXCH_TABLE_ENT tableType;      /* table type */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum, txqPortNum);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tcQueue);

    if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
    {
        if((qcnSampleInterval >= BIT_20) || (qlenOld >= BIT_20))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        if((qcnSampleInterval >= BIT_20) || (qlenOld >= BIT_16))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }

    U32_SET_FIELD_IN_ENTRY_MAC(hwArray, 0, 20, qcnSampleInterval);

    /* Aldrin-2 and above devices have QLenOld field of width 20 */
    if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
    {
        U32_SET_FIELD_IN_ENTRY_MAC(hwArray, 20, 20, qlenOld);
    }
    else
    {
        U32_SET_FIELD_IN_ENTRY_MAC(hwArray, 20, 16, qlenOld);
    }

    tableIndex = tcQueue | (txqPortNum << 3);
    tableType = CPSS_DXCH_SIP5_TABLE_CN_SAMPLE_INTERVALS_E;

    rc = prvCpssDxChWriteTableEntry(devNum, tableType, tableIndex, hwArray);

    return rc;
}

/**
* @internal cpssDxChPortCnMessageTriggeringStateSet function
* @endinternal
*
* @brief   Sets CNM Triggering State Variables
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] qcnSampleInterval        -  parameter that defines the probability to trigger
*                                      the next CN frame based on the current value of the
*                                      calculated feedback (Fb), measured in Bytes.
*                                      (APPLICABLE RANGES: 0..0xFFFFF)
* @param[in] qlenOld                  - a snapshot of current length of the queue in buffers,
*                                      taken when the QcnSampleInterval expires.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*                                      Aldrin2: (APPLICABLE RANGES: 0..0xFFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number,portNum,tc
* @retval GT_OUT_OF_RANGE          - on wrong qcnSampleInterval, qlenOld
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note qlenOld reset value and QcnSampleInterval init value must be set by the
*       application prior to configuring the queue to be CN aware
*
*/
GT_STATUS cpssDxChPortCnMessageTriggeringStateSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   tcQueue,
    IN  GT_U32                  qcnSampleInterval,
    IN  GT_U32                  qlenOld
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnMessageTriggeringStateSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tcQueue, qcnSampleInterval, qlenOld));

    rc = internal_cpssDxChPortCnMessageTriggeringStateSet(devNum, portNum, tcQueue, qcnSampleInterval, qlenOld);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tcQueue, qcnSampleInterval, qlenOld));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortCnMessageTriggeringStateGet function
* @endinternal
*
* @brief   Gets CNM Triggering State Variables
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] qcnSampleIntervalPtr     -  (pointer to) parameter that defines the probability to trigger
*                                      the next CN frame based on the current value of the
*                                      calculated feedback (Fb), measured in Bytes.
* @param[out] qlenOldPtr               - (pointer to) snapshot of current length of the queue in buffers,
*                                      taken when the QcnSampleInterval expires.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number,portNum,tc
* @retval GT_OUT_OF_RANGE          - on wrong qcnSampleInterval, qlenOld
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note qlenOld reset value and QcnSampleInterval init value must be set by the
*       application prior to configuring the queue to be CN aware
*
*/
static GT_STATUS internal_cpssDxChPortCnMessageTriggeringStateGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   tcQueue,
    OUT GT_U32                  *qcnSampleIntervalPtr,
    OUT GT_U32                  *qlenOldPtr
)
{
    GT_STATUS               rc;             /* return code */
    GT_U32                  txqPortNum;     /* TXQ port number */
    GT_U32                  hwArray[2];     /* table HW data */
    GT_U32                  tableIndex;     /* table index */
    CPSS_DXCH_TABLE_ENT tableType;      /* table type */
    GT_U32                  regValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum, txqPortNum);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tcQueue);
    CPSS_NULL_PTR_CHECK_MAC(qcnSampleIntervalPtr);
    CPSS_NULL_PTR_CHECK_MAC(qlenOldPtr);

    tableIndex = tcQueue | (txqPortNum << 3);
    tableType = CPSS_DXCH_SIP5_TABLE_CN_SAMPLE_INTERVALS_E;

    rc = prvCpssDxChReadTableEntry(devNum, tableType, tableIndex, hwArray);
    if(GT_OK != rc)
    {
        return rc;
    }

    U32_GET_FIELD_IN_ENTRY_MAC(hwArray, 0, 20, regValue);
    *qcnSampleIntervalPtr = regValue;

    /* Aldrin-2 and above devices have QLenOld field of width 20 */
    if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
    {
        U32_GET_FIELD_IN_ENTRY_MAC(hwArray, 20, 20, regValue);
    }
    else
    {
        U32_GET_FIELD_IN_ENTRY_MAC(hwArray, 20, 16, regValue);
    }
    *qlenOldPtr = regValue;

    return GT_OK;
}

/**
* @internal cpssDxChPortCnMessageTriggeringStateGet function
* @endinternal
*
* @brief   Gets CNM Triggering State Variables
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] qcnSampleIntervalPtr     -  (pointer to) parameter that defines the probability to trigger
*                                      the next CN frame based on the current value of the
*                                      calculated feedback (Fb), measured in Bytes.
* @param[out] qlenOldPtr               - (pointer to) snapshot of current length of the queue in buffers,
*                                      taken when the QcnSampleInterval expires.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number,portNum,tc
* @retval GT_OUT_OF_RANGE          - on wrong qcnSampleInterval, qlenOld
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note qlenOld reset value and QcnSampleInterval init value must be set by the
*       application prior to configuring the queue to be CN aware
*
*/
GT_STATUS cpssDxChPortCnMessageTriggeringStateGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   tcQueue,
    OUT GT_U32                  *qcnSampleIntervalPtr,
    OUT GT_U32                  *qlenOldPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnMessageTriggeringStateGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tcQueue, qcnSampleIntervalPtr, qlenOldPtr));

    rc = internal_cpssDxChPortCnMessageTriggeringStateGet(devNum, portNum, tcQueue, qcnSampleIntervalPtr, qlenOldPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tcQueue, qcnSampleIntervalPtr, qlenOldPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnPauseTimerMapTableEntrySet function
* @endinternal
*
* @brief   Sets Pause Timer Map Table Entry.
*         Table index is QCN-qFb or CCFC Speed bits.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - table entry index.
*                                      (APPLICABLE RANGES: 0..63)
* @param[in] pauseTimer               - pause timer.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, index
* @retval GT_OUT_OF_RANGE          - on wrong pauseTimer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnPauseTimerMapTableEntrySet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    IN  GT_U32                  pauseTimer
)
{
    GT_STATUS               rc;             /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_FALCON_E);
    if (index >= 64)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (pauseTimer >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChWriteTableEntry(
        devNum, CPSS_DXCH_SIP5_20_TABLE_TTI_QCN_TO_TIMER_PAUSE_MAP_E, index, &pauseTimer);

    return rc;
}

/**
* @internal cpssDxChPortCnPauseTimerMapTableEntrySet function
* @endinternal
*
* @brief   Sets Pause Timer Map Table Entry.
*         Table index is QCN-qFb or CCFC Speed bits.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - table entry index.
*                                      (APPLICABLE RANGES: 0..63)
* @param[in] pauseTimer               - pause timer.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, index
* @retval GT_OUT_OF_RANGE          - on wrong pauseTimer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnPauseTimerMapTableEntrySet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    IN  GT_U32                  pauseTimer
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnPauseTimerMapTableEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, pauseTimer));

    rc = internal_cpssDxChPortCnPauseTimerMapTableEntrySet(
        devNum, index, pauseTimer);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, pauseTimer));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnPauseTimerMapTableEntryGet function
* @endinternal
*
* @brief   Gets Pause Timer Map Table Entry.
*         Table index is QCN-qFb or CCFC Speed bits.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - table entry index.
*                                      (APPLICABLE RANGES: 0..63)
*
* @param[out] pauseTimerPtr            - (pointer to)pause timer.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, index
* @retval GT_BAD_PTR               - on NULL Pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnPauseTimerMapTableEntryGet
(
    IN   GT_U8                   devNum,
    IN   GT_U32                  index,
    OUT  GT_U32                  *pauseTimerPtr
)
{
    GT_STATUS               rc;             /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_FALCON_E);
    if (index >= 64)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    CPSS_NULL_PTR_CHECK_MAC(pauseTimerPtr);

    rc = prvCpssDxChReadTableEntry(
        devNum, CPSS_DXCH_SIP5_20_TABLE_TTI_QCN_TO_TIMER_PAUSE_MAP_E, index, pauseTimerPtr);

    return rc;
}

/**
* @internal cpssDxChPortCnPauseTimerMapTableEntryGet function
* @endinternal
*
* @brief   Gets Pause Timer Map Table Entry.
*         Table index is QCN-qFb or CCFC Speed bits.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - table entry index.
*                                      (APPLICABLE RANGES: 0..63)
*
* @param[out] pauseTimerPtr            - (pointer to)pause timer.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, index
* @retval GT_BAD_PTR               - on NULL Pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnPauseTimerMapTableEntryGet
(
    IN   GT_U8                   devNum,
    IN   GT_U32                  index,
    OUT  GT_U32                  *pauseTimerPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnPauseTimerMapTableEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, pauseTimerPtr));

    rc = internal_cpssDxChPortCnPauseTimerMapTableEntryGet(
        devNum, index, pauseTimerPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, pauseTimerPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnDbaAvailableBuffSet function
* @endinternal
*
* @brief   Set amount of buffers available for dynamic allocation for CN.
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] availableBuff            - amount of buffers available for dynamic allocation for CN.
*                                      (APPLICABLE RANGES: 0..0xFFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, index
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnDbaAvailableBuffSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          availableBuff
)
{
    GT_U32  regAddr;    /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
        CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_XCAT_E | CPSS_XCAT2_E | CPSS_XCAT3_E);

    /* Set Aldrin2 CN Available Buffers for QCN DBA. */
    if (availableBuff >= BIT_20)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* <CN Available Buffers> register */
    regAddr = PRV_CPSS_SIP_6_CHECK_MAC(devNum) ? PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.globalQcnConfig :
                                                 PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).cnAvailableBuffers;

    /* Set <cn_available_buffs> field of */
    /* <CN Available Buffers> register   */
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 20,
                                  availableBuff);
}

/**
* @internal cpssDxChPortCnDbaAvailableBuffSet function
* @endinternal
*
* @brief   Set amount of buffers available for dynamic allocation for CN.
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] availableBuff            - amount of buffers available for dynamic allocation for CN.
*                                      (APPLICABLE RANGES: 0..0xFFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, index
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnDbaAvailableBuffSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          availableBuff
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnDbaAvailableBuffSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, availableBuff));

    rc = internal_cpssDxChPortCnDbaAvailableBuffSet(devNum, availableBuff);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, availableBuff));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChPortCnDbaAvailableBuffGet function
 * @endinternal
 *
 * @brief   Get amount of buffers available for dynamic allocation for CN.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
 *
 * @param[in] devNum                   - device number
 *
 * @param[out] availableBuffPtr         - pointer to amount of buffers available for dynamic allocation for CN.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
static GT_STATUS internal_cpssDxChPortCnDbaAvailableBuffGet
(
 IN  GT_U8           devNum,
 OUT GT_U32          *availableBuffPtr
)
{
  GT_STATUS   rc;         /* return code */
  GT_U32      regAddr;    /* register address */
  GT_U32      regValue;   /* register value */

  PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
  PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
           CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_XCAT_E | CPSS_XCAT2_E | CPSS_XCAT3_E);
  CPSS_NULL_PTR_CHECK_MAC(availableBuffPtr);

  /* <CN Available Buffers> register */
  regAddr = PRV_CPSS_SIP_6_CHECK_MAC(devNum) ? PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.globalQcnConfig :
                                               PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).cnAvailableBuffers;

  /* Set <cn_available_buffs> field of */
  /* <CN Available Buffers> register   */
  rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 20, &regValue);
  if (rc != GT_OK)
  {
    return rc;
  }

  *availableBuffPtr = regValue;

  return GT_OK;
}

/**
 * @internal cpssDxChPortCnDbaAvailableBuffGet function
 * @endinternal
 *
 * @brief   Get amount of buffers available for dynamic allocation for CN.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
 *
 * @param[in] devNum                   - device number
 *
 * @param[out] availableBuffPtr         - pointer to amount of buffers available for dynamic allocation for CN.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssDxChPortCnDbaAvailableBuffGet
(
 IN  GT_U8           devNum,
 OUT GT_U32          *availableBuffPtr
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnDbaAvailableBuffGet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId, devNum, availableBuffPtr));

  rc = internal_cpssDxChPortCnDbaAvailableBuffGet(devNum, availableBuffPtr);

  CPSS_LOG_API_EXIT_MAC(funcId, rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, availableBuffPtr));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
 * @internal internal_cpssDxChPortCnDbaModeEnableSet function
 * @endinternal
 *
 * @brief   Enable/disable Dynamic Buffers Allocation (DBA) for QCN.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
 *
 * @param[in] devNum                   - device number
 * @param[in] enable                   - GT_TRUE -  DBA mode enabled
 *                                      GT_FALSE - DBA mode disabled.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
static GT_STATUS internal_cpssDxChPortCnDbaModeEnableSet
(
 IN  GT_U8           devNum,
 IN  GT_BOOL         enable
)
{
  GT_U32  regAddr;    /* register address */
  GT_U32  regValue;   /* register value */

  PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
  PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
           CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_XCAT_E | CPSS_XCAT2_E | CPSS_XCAT3_E | CPSS_FALCON_E);
  regValue = BOOL2BIT_MAC(enable);

  /* <CN Global Configuration Register> register */
  regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).CNGlobalConfig;

  /* Set <Enable dynamic buffer mode> field of  */
  /* <CN Global Cnfiguration Register> register  */
  return prvCpssHwPpSetRegField(devNum, regAddr, 7, 1, regValue);
}

/**
 * @internal cpssDxChPortCnDbaModeEnableSet function
 * @endinternal
 *
 * @brief   Enable/disable Dynamic Buffers Allocation (DBA) for QCN.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
 *
 * @param[in] devNum                   - device number
 * @param[in] enable                   - GT_TRUE -  DBA mode enabled
 *                                      GT_FALSE - DBA mode disabled.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssDxChPortCnDbaModeEnableSet
(
 IN  GT_U8           devNum,
 IN  GT_BOOL         enable
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnDbaModeEnableSet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

  rc = internal_cpssDxChPortCnDbaModeEnableSet(devNum, enable);

  CPSS_LOG_API_EXIT_MAC(funcId, rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
 * @internal internal_cpssDxChPortCnDbaModeEnableGet function
 * @endinternal
 *
 * @brief   Get state of Dynamic Buffers Allocation (DBA) for QCN.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
 *
 * @param[in] devNum                   - device number
 *
 * @param[out] enablePtr                - pointer to DBA mode state.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
static GT_STATUS internal_cpssDxChPortCnDbaModeEnableGet
(
 IN  GT_U8           devNum,
 OUT GT_BOOL         *enablePtr
)
{
  GT_STATUS   rc;         /* return code */
  GT_U32      regAddr;    /* register address */
  GT_U32      regValue;   /* register value */

  PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
  PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
           CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_XCAT_E | CPSS_XCAT2_E | CPSS_XCAT3_E | CPSS_FALCON_E);
  CPSS_NULL_PTR_CHECK_MAC(enablePtr);

  /* <CN Global Configuration Configuration> register */
  regAddr = PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(devNum).CNGlobalConfig;

  /* Get <Enable dynamic buffer mode> field of  */
  /* <CN Global ConfigurationConfiguration> register  */
  rc = prvCpssHwPpGetRegField(devNum, regAddr, 7, 1, &regValue);
  if (rc != GT_OK)
  {
    return rc;
  }

  *enablePtr = BIT2BOOL_MAC(regValue);

  return GT_OK;
}

/**
 * @internal cpssDxChPortCnDbaModeEnableGet function
 * @endinternal
 *
 * @brief   Get state of Dynamic Buffers Allocation (DBA) for QCN.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
 *
 * @param[in] devNum                   - device number
 *
 * @param[out] enablePtr               - pointer to DBA mode state.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssDxChPortCnDbaModeEnableGet
(
 IN  GT_U8           devNum,
 OUT GT_BOOL         *enablePtr
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnDbaModeEnableGet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

  rc = internal_cpssDxChPortCnDbaModeEnableGet(devNum, enablePtr);

  CPSS_LOG_API_EXIT_MAC(funcId, rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
* @internal internal_cpssDxChPortCnDbaPoolAvailableBuffSet function
* @endinternal
*
* @brief   Set amount of pool buffers available for dynamic allocation for CN.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] poolNum                  - pool number for QCN DBA buffers
*                                      (APPLICABLE RANGES: 0..1)
* @param[in] poolAvailableBuff        - amount of pool buffers available for dynamic allocation for CN.
*                                      (APPLICABLE RANGES: 0..0xFFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, index
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnDbaPoolAvailableBuffSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          poolNum,
    IN  GT_U32          poolAvailableBuff
)
{
    GT_U32  regAddr;    /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
        CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_XCAT_E | CPSS_XCAT2_E | CPSS_XCAT3_E | CPSS_ALDRIN2_E);

    /* Set Falcon CN Pool Available Buffers for QCN DBA. */
    if (poolAvailableBuff >= BIT_20)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    if (poolNum > 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* <CN Pool Available Buffers> register */
    if(poolNum)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.qcnPool1AvailableBuffers;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.qcnPool0AvailableBuffers;
    }

    /* Set <cn_pool_available_buffs> field of */
    /* <CN Pool Available Buffers> register   */
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 20,
                                  poolAvailableBuff);
}

/**
* @internal cpssDxChPortCnDbaPoolAvailableBuffSet function
* @endinternal
*
* @brief   Set amount of pool buffers available for dynamic allocation for CN.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] poolNum                  - pool number for QCN DBA buffers.
*                                      (APPLICABLE RANGES: 0..1)
* @param[in] poolAvailableBuff        - amount of pool buffers available for dynamic allocation for CN.
*                                      (APPLICABLE RANGES: 0..0xFFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, index
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnDbaPoolAvailableBuffSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          poolNum,
    IN  GT_U32          poolAvailableBuff
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnDbaPoolAvailableBuffSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, poolNum, poolAvailableBuff));

    rc = internal_cpssDxChPortCnDbaPoolAvailableBuffSet(devNum, poolNum, poolAvailableBuff);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, poolNum, poolAvailableBuff));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChPortCnDbaPoolAvailableBuffGet function
 * @endinternal
 *
 * @brief   Get amount of pool buffers available for dynamic allocation for CN.
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - device number
 * @param[in] poolNum                  - pool number for QCN DBA buffers
 *                                      (APPLICABLE RANGES: 0..1)
 * @param[out] poolAvailableBuffPtr    - pointer to amount of pool buffers available for dynamic allocation for CN.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
static GT_STATUS internal_cpssDxChPortCnDbaPoolAvailableBuffGet
(
 IN  GT_U8           devNum,
 IN  GT_BOOL         poolNum,
 OUT GT_U32          *poolAvailableBuffPtr
)
{
  GT_STATUS   rc;         /* return code */
  GT_U32      regAddr;    /* register address */
  GT_U32      regValue;   /* register value */

  PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
  PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
           CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_XCAT_E | CPSS_XCAT2_E | CPSS_XCAT3_E | CPSS_ALDRIN2_E);
  CPSS_NULL_PTR_CHECK_MAC(poolAvailableBuffPtr);

  /* Set Falcon CN Pool Available Buffers for QCN DBA. */
  if (poolNum > 1)
  {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
  }
  /* <CN Pool Available Buffers> register */
  if(poolNum)
  {
      regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.qcnPool1AvailableBuffers;
  }
  else
  {
      regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.qcnPool0AvailableBuffers;
  }

  /* Set <cn_pool_available_buffs> field of */
  /* <CN Pool Available Buffers> register   */
  rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 20, &regValue);
  if (rc != GT_OK)
  {
    return rc;
  }

  *poolAvailableBuffPtr = regValue;

  return GT_OK;
}

/**
 * @internal cpssDxChPortCnDbaPoolAvailableBuffGet function
 * @endinternal
 *
 * @brief   Get amount of pool buffers available for dynamic allocation for CN.
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - device number
 * @param[in] poolNum                  - pool number for QCN DBA buffers
 *                                      (APPLICABLE RANGES: 0..1)
 * @param[out] poolAvailableBuffPtr    - pointer to amount of pool buffers available for dynamic allocation for CN.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssDxChPortCnDbaPoolAvailableBuffGet
(
 IN  GT_U8           devNum,
 IN  GT_BOOL         poolNum,
 OUT GT_U32          *poolAvailableBuffPtr
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnDbaPoolAvailableBuffGet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId, devNum, poolNum, poolAvailableBuffPtr));

  rc = internal_cpssDxChPortCnDbaPoolAvailableBuffGet(devNum, poolNum, poolAvailableBuffPtr);

  CPSS_LOG_API_EXIT_MAC(funcId, rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, poolNum, poolAvailableBuffPtr));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
* @internal internal_cpssDxChPortCnQcnTriggerSet function
* @endinternal
*
* @brief   Set QCN trigger.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - index to the trigger threshold table
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] qcnTriggerPtr            - (pointer to) qcn trigger threshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, index
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnQcnTriggerSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    IN CPSS_DXCH_PORT_CN_TRIGGER_CONFIG_STC *qcnTriggerPtr
)
{
    GT_STATUS rc;
    GT_U32    regAddr;    /* register address */
    GT_U32    value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
        CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_XCAT_E | CPSS_XCAT2_E | CPSS_XCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(qcnTriggerPtr);

    /* Validate index to trigger threshold table */
    if (index >= 32)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* Validate Falcon CN Pool Available Buffers for QCN DBA. */
    if (qcnTriggerPtr->qcnTriggerTh0 >= BIT_14 ||
        qcnTriggerPtr->qcnTriggerTh1 >= BIT_14)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* <QCN Trigger> register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.qcnTrigger[index];

    value = (qcnTriggerPtr->qcnTriggerTh0) | (qcnTriggerPtr->qcnTriggerTh1 << 16);
    /* Set <qcn_trigger_th0/1> field of */
    /* <QCN Trigger> register   */
    rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, 0x3FFF3FFF, value);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnQcnTriggerSet function
* @endinternal
*
* @brief   Set threshold value for QCN trigger.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - index to trigger threshold table
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] qcnTriggerPtr            - (pointer to) qcn trigger threshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, index
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnQcnTriggerSet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               index,
    IN  CPSS_DXCH_PORT_CN_TRIGGER_CONFIG_STC *qcnTriggerPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnQcnTriggerSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, qcnTriggerPtr));

    rc = internal_cpssDxChPortCnQcnTriggerSet(devNum, index, qcnTriggerPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, qcnTriggerPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChPortCnQcnTriggerGet function
 * @endinternal
 *
 * @brief   Get threshold value for QCN trigger.
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - device number
 * @param[in] index                    - index to triger threshold table
 * @param[in] qcnTriggerPtr            - (pointer to) qcn trigger threshold
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
static GT_STATUS internal_cpssDxChPortCnQcnTriggerGet
(
 IN  GT_U8                                devNum,
 IN  GT_U32                               index,
 OUT CPSS_DXCH_PORT_CN_TRIGGER_CONFIG_STC *qcnTriggerPtr
)
{
  GT_STATUS   rc;         /* return code */
  GT_U32      regAddr;    /* register address */
  GT_U32      regValue;   /* register value */

  PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
  PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
           CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_XCAT_E | CPSS_XCAT2_E | CPSS_XCAT3_E | CPSS_ALDRIN2_E);
  CPSS_NULL_PTR_CHECK_MAC(qcnTriggerPtr);

  /* Validate index to trigger threshold table */
  if (index >= 32)
  {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
  }

  /* <CN Pool Available Buffers> register */
  regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.qcnTrigger[index];

  /* Set <cn_pool_available_buffs> field of */
  /* <CN Pool Available Buffers> register   */
  rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 32, &regValue);
  if (rc != GT_OK)
  {
    return rc;
  }

  qcnTriggerPtr->qcnTriggerTh1 = (regValue >> 16 ) & 0x3FFF;
  qcnTriggerPtr->qcnTriggerTh0 = regValue & 0x3FFF;

  return GT_OK;
}

/**
 * @internal cpssDxChPortCnQcnTriggerGet function
 * @endinternal
 *
 * @brief   Get configured threshold for QCN trigger.
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - device number
 * @param[in] index                    - index to trigger threshold table
 * @param[in] qcnTriggerPtr            - (pointer to) qcn trigger threshold
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssDxChPortCnQcnTriggerGet
(
 IN  GT_U8                                devNum,
 IN  GT_U32                               index,
 OUT CPSS_DXCH_PORT_CN_TRIGGER_CONFIG_STC *qcnTriggerPtr
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnQcnTriggerGet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, qcnTriggerPtr));

  rc = internal_cpssDxChPortCnQcnTriggerGet(devNum, index, qcnTriggerPtr);

  CPSS_LOG_API_EXIT_MAC(funcId, rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, qcnTriggerPtr));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
 * @internal internal_cpssDxChPortCnCNMEnableSet function
 * @endinternal
 *
 * @brief   Enable/disable CNM generation for CN Untagged Frames for QCN.
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2
 *
 * @param[in] devNum                   - device number
 * @param[in] enable                   - GT_TRUE -  CNM generation enabled.
 *                                      GT_FALSE - CNM generation disabled.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
static GT_STATUS internal_cpssDxChPortCnCNMEnableSet
(
 IN  GT_U8           devNum,
 IN  GT_BOOL         enable
)
{
  GT_U32  regAddr;    /* register address */
  GT_U32  regValue;   /* register value */

  PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
  PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
           CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_XCAT_E | CPSS_XCAT2_E | CPSS_XCAT3_E | CPSS_ALDRIN2_E);
  regValue = BOOL2BIT_MAC(enable);

  /* <Egress Filters Global Configuration> register */
  regAddr = PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).egrFilterConfigs.egrFiltersGlobal;

  /* Set <Enable CNM For CN Untagged Frames> field of  */
  /* <Egress Filters Global Configuration> register */
  return prvCpssHwPpSetRegField(devNum, regAddr, 15, 1, regValue);
}

/**
 * @internal cpssDxChPortCnCNMEnableSet function
 * @endinternal
 *
 * @brief   Enable/disable CNM generation for CN Untagged Frames for QCN.
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - device number
 * @param[in] enable                   - GT_TRUE -  CNM generation enabled.
 *                                      GT_FALSE - CNM generation disabled.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssDxChPortCnCNMEnableSet
(
 IN  GT_U8           devNum,
 IN  GT_BOOL         enable
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnCNMEnableSet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

  rc = internal_cpssDxChPortCnCNMEnableSet(devNum, enable);

  CPSS_LOG_API_EXIT_MAC(funcId, rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
 * @internal internal_cpssDxChPortCnCNMEnableGet function
 * @endinternal
 *
 * @brief   Get state of CNM enable for QCN.
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - device number
 *
 * @param[out] enablePtr               - pointer to CNM enable state.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
static GT_STATUS internal_cpssDxChPortCnCNMEnableGet
(
 IN  GT_U8           devNum,
 OUT GT_BOOL         *enablePtr
)
{
  GT_STATUS   rc;         /* return code */
  GT_U32      regAddr;    /* register address */
  GT_U32      regValue;   /* register value */

  PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
  PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
           CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_XCAT_E | CPSS_XCAT2_E | CPSS_XCAT3_E | CPSS_ALDRIN2_E);
  CPSS_NULL_PTR_CHECK_MAC(enablePtr);

  /* <Egress Filters Global Configuration> register */
  regAddr = PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).egrFilterConfigs.egrFiltersGlobal;

  /* Get <Enable CNM for CN Untagged Frames> field of  */
  /* <Egress Filters Global Configuration> register  */
  rc = prvCpssHwPpGetRegField(devNum, regAddr, 15, 1, &regValue);
  if (rc != GT_OK)
  {
    return rc;
  }

  *enablePtr = BIT2BOOL_MAC(regValue);

  return GT_OK;
}

/**
 * @internal cpssDxChPortCnCNMEnableGet function
 * @endinternal
 *
 * @brief   Get state of CNM enable for QCN.
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
 *
 * @param[in] devNum                   - device number
 * @param[out] enablePtr               - pointer to CNM enable state.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssDxChPortCnCNMEnableGet
(
 IN  GT_U8           devNum,
 OUT GT_BOOL         *enablePtr
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnCNMEnableGet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

  rc = internal_cpssDxChPortCnCNMEnableGet(devNum, enablePtr);

  CPSS_LOG_API_EXIT_MAC(funcId, rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}


/**
* @internal internal_cpssDxChPortCnProfilePortConfigSet function
* @endinternal
*
* @brief   Sets CN profile configurations for given port profile.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - profile set.
* @param[in] cnProfileCfgPtr          - pointer to CN Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile set or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CN Profile is part of the Tail-Drop Profile.
*       To bind port to Tail-Drop Profile use cpssDxChPortTxBindPortToDpSet.
*
*/
static GT_STATUS internal_cpssDxChPortCnProfilePortConfigSet
(
    IN GT_U8                               devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
)
{
    GT_STATUS rc;     /* function return value */
    GT_U32 thresholdNumOfBits = 20; /* number of bits in threshold field*/
    GT_U32 alphaNumOfBits = 4; /* number of bits in alpha field*/
    GT_U32 alphaHwValue = 0; /* dynamic factor alpha value */
    GT_U32 regData[4];
    GT_U32 value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E |
         CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(cnProfileCfgPtr);
    PRV_CPSS_DXCH_DROP_PROFILE_CHECK_MAC(profileSet,devNum);

    if (cnProfileCfgPtr->threshold > BIT_MASK_MAC(thresholdNumOfBits))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChReadTableEntry(devNum,
        CPSS_DXCH_SIP6_TABLE_PREQ_PORT_PROFILE_E,
        profileSet,
        regData);
    /* Set alpha for QCN DBA for Falcon */
    switch(cnProfileCfgPtr->alpha)
    {
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E:
            alphaHwValue = 0;
            break;

        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_03125_E:
            alphaHwValue = 1;
            break;

        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_0625_E:
            alphaHwValue = 2;
            break;

        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_125_E:
            alphaHwValue = 3;
            break;

        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E:
            alphaHwValue = 4;
            break;

        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E:
            alphaHwValue = 5;
            break;

        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E:
            alphaHwValue = 6;
            break;

        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E:
            alphaHwValue = 7;
            break;

        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E:
            alphaHwValue = 8;
            break;

        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E:
            alphaHwValue = 9;
            break;

        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_16_E:
            alphaHwValue = 0xa;
            break;

        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_32_E:
            alphaHwValue = 0xb;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    value = BOOL2BIT_MAC(cnProfileCfgPtr->cnAware);
    U32_SET_FIELD_IN_ENTRY_MAC(regData,71,1,value);
    U32_SET_FIELD_IN_ENTRY_MAC(regData,51,thresholdNumOfBits,cnProfileCfgPtr->threshold);
    U32_SET_FIELD_IN_ENTRY_MAC(regData,47,alphaNumOfBits,alphaHwValue);

    rc = prvCpssDxChWriteTableEntry(devNum,
        CPSS_DXCH_SIP6_TABLE_PREQ_PORT_PROFILE_E,
        profileSet,
        regData);
    if(rc !=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChWriteTableEntry failed\n");
    }

    return rc;
}

/**
* @internal cpssDxChPortCnProfilePortConfigSet function
* @endinternal
*
* @brief   Sets CN profile configurations for given port profile.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - profile set.
* @param[in] cnProfileCfgPtr          - pointer to CN Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile set or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CN Profile is part of the Tail-Drop Profile.
*       To bind port to Tail-Drop Profile use cpssDxChPortTxBindPortToDpSet.
*
*/
GT_STATUS cpssDxChPortCnProfilePortConfigSet
(
    IN GT_U8                               devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnProfilePortConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, cnProfileCfgPtr));

    rc = internal_cpssDxChPortCnProfilePortConfigSet(devNum, profileSet, cnProfileCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, cnProfileCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnProfilePortConfigGet function
* @endinternal
*
* @brief   Gets CN profile configurations for given port profile.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - profile set.
* @param[out] cnProfileCfgPtr         - pointer to CN Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_STATE             - on wrong values of alpha
* @retval GT_BAD_PARAM             - on wrong device number, profile set or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnProfilePortConfigGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    OUT CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
)
{
    GT_STATUS rc;     /* function return value */
    GT_U32 alphaNumOfBits = 4; /* field length */
    GT_U32 alphaHwValue;
    GT_U32 regData[4];
    GT_U32 value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E |
         CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(cnProfileCfgPtr);
    PRV_CPSS_DXCH_DROP_PROFILE_CHECK_MAC(profileSet,devNum);

    rc = prvCpssDxChReadTableEntry(devNum,
        CPSS_DXCH_SIP6_TABLE_PREQ_PORT_PROFILE_E,
        profileSet,regData);
    if(rc !=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChReadTableEntry failed");
    }

    U32_GET_FIELD_IN_ENTRY_MAC(regData,71,1,value);
    U32_GET_FIELD_IN_ENTRY_MAC(regData,51,20,cnProfileCfgPtr->threshold);
    U32_GET_FIELD_IN_ENTRY_MAC(regData,47,alphaNumOfBits,alphaHwValue);

    cnProfileCfgPtr->cnAware = BIT2BOOL_MAC(value);
    /* Get alpha for QCN DBA for Falcon */
    switch(alphaHwValue)
    {
        case 0:
          cnProfileCfgPtr->alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
          break;
        case 1:
          cnProfileCfgPtr->alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_03125_E;
          break;
        case 2:
          cnProfileCfgPtr->alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_0625_E;
          break;
        case 3:
          cnProfileCfgPtr->alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_125_E;
          break;
        case 4:
          cnProfileCfgPtr->alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E;
          break;
        case 5:
          cnProfileCfgPtr->alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E;
          break;
        case 6:
          cnProfileCfgPtr->alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E;
          break;
        case 7:
          cnProfileCfgPtr->alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E;
          break;
        case 8:
          cnProfileCfgPtr->alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E;
          break;
        case 9:
          cnProfileCfgPtr->alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E;
          break;
        case 0xa:
          cnProfileCfgPtr->alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_16_E;
          break;
        case 0xb:
          cnProfileCfgPtr->alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_32_E;
          break;
        default:
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssDxChPortCnProfilePortConfigGet function
* @endinternal
*
* @brief   Gets CN profile configurations for given port profile.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - profile set.
* @param[out] cnProfileCfgPtr         - pointer to CN Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_STATE             - on wrong values of alpha
* @retval GT_BAD_PARAM             - on wrong device number, profile set
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnProfilePortConfigGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    OUT CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnProfilePortConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, cnProfileCfgPtr));

    rc = internal_cpssDxChPortCnProfilePortConfigGet(devNum, profileSet, cnProfileCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, cnProfileCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortCnCongestedQPriorityLocationSet function
* @endinternal
*
* @brief   Sets location of the congested queue/priority bits in the CNM header for parsing
*         and generation of CNM.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                        - device number.
* @param[in] congestedQPriorityLocation    - congested q/ priority location.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnCongestedQPriorityLocationSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT congestedQPriorityLocation
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */
    GT_U32    startBit;          /* start bit of the field */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E |
         CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    switch (congestedQPriorityLocation)
    {
        case CPSS_DXCH_PORT_CN_CONG_Q_NUM_E:
            value = 0;
            break;
        case CPSS_DXCH_PORT_CN_QCN_ORIG_PRIO_E:
            value = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).haGlobalConfig;
    startBit = 16;

    rc =  prvCpssHwPpSetRegField(devNum, regAddr, startBit, 1, value);

    return rc;
}

/**
* @internal cpssDxChPortCnCongestedQPriorityLocationSet function
* @endinternal
*
* @brief   Sets location of the congested q/priority bits in the CNM header for parsing
*         and generation of CNM.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                        - device number.
* @param[in] congestedQPriorityLocation    - congested q/ priority location.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnCongestedQPriorityLocationSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT congestedQPriorityLocation
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnCongestedQPriorityLocationSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, congestedQPriorityLocation));

    rc = internal_cpssDxChPortCnCongestedQPriorityLocationSet(devNum, congestedQPriorityLocation);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, congestedQPriorityLocation));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCnCongestedQPriorityLocationGet function
* @endinternal
*
* @brief   Gets location of the index (congested q/priority) bits in the CNM header
*         for parsing and generation of CNM.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                         - device number.
* @param[out] congestedQPriorityLocationPtr - (pointer to) congested q priority location.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCnCongestedQPriorityLocationGet
(
    IN  GT_U8                                devNum,
    OUT CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT *congestedQPriorityLocationPtr
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */
    GT_U32    startBit;          /* start bit of the field */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E |
         CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(congestedQPriorityLocationPtr);

    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).haGlobalConfig;
    startBit = 16;

    rc =  prvCpssHwPpGetRegField(devNum, regAddr, startBit, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }
    *congestedQPriorityLocationPtr = (CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT)value;
    return GT_OK;
}

/**
* @internal cpssDxChPortCnCongestedQPriorityLocationGet function
* @endinternal
*
* @brief   Gets location of the index (congested q/priority) bits in the CNM header
*         for parsing and generation of CNM.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                         - device number.
* @param[out] congestedQPriorityLocationPtr - (pointer to) congested q priority location.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnCongestedQPriorityLocationGet
(
    IN  GT_U8                                devNum,
    OUT CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT *congestedQPriorityLocationPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCnCongestedQPriorityLocationGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, congestedQPriorityLocationPtr));

    rc = internal_cpssDxChPortCnCongestedQPriorityLocationGet(devNum, congestedQPriorityLocationPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, congestedQPriorityLocationPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


