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
* @file cpssDxChPortPfc.c
*
* @brief CPSS implementation for Priority Flow Control functionality.
*
* @version   41
********************************************************************************
*/

/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortPfc.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPort.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChMacCtrl.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqQfc.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqPfcc.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/prvCpssFalconTxqQfcRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqSdq.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqFcGopUtils.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqQfc.h>




/* PFC- minimal value for PB limit in sip 6 */
#define PFC_MIN_PB_TRESHOLD_VAL_CNS  3071

/* PFC/LLFC support 78 ports in sip 5.20 */
#define LLFC_NUM_PORTS_SIP_5_20_CNS  78

/* PFC/LLFC support 100 ports in sip 5.25 */
#define LLFC_NUM_PORTS_SIP_5_25_CNS  100

/* Convert CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT to HW value */
#define PRV_CPSS_PORT_PFC_DBA_ALPHA_TO_HW_CONVERT_MAC(alpha, hwValue) \
    switch (alpha)                                                            \
    {                                                                         \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E:                            \
            hwValue = 0;                                                      \
            break;                                                            \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E:                         \
            hwValue = 1;                                                      \
            break;                                                            \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E:                          \
            hwValue = 2;                                                      \
            break;                                                            \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E:                            \
            hwValue = 3;                                                      \
            break;                                                            \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E:                            \
            hwValue = 4;                                                      \
            break;                                                            \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E:                            \
            hwValue = 5;                                                      \
            break;                                                            \
        default:                                                              \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);    \
    }

/* Convert HW value to CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT */
#define PRV_CPSS_PORT_PFC_DBA_HW_TO_ALPHA_CONVERT_MAC(hwValue, alpha) \
    switch (hwValue)                                                          \
    {                                                                         \
        case 0:                                                               \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;                     \
            break;                                                            \
        case 1:                                                               \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E;                  \
            break;                                                            \
        case 2:                                                               \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E;                   \
            break;                                                            \
        case 3:                                                               \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E;                     \
            break;                                                            \
        case 4:                                                               \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E;                     \
            break;                                                            \
        case 5:                                                               \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E;                     \
            break;                                                            \
        default:                                                              \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);    \
    }

/**
* @internal internal_cpssDxChPortPfcEnableSet function
* @endinternal
*
* @brief   Enable/Disable PFC (Priority Flow Control) response functionality.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] pfcEnable                - PFC enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or PFC enable option
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If PFC response is enabled, the shaper’s baseline must be
*       at least 0x3FFFC0, see:
*       cpssDxChPortTxShaperBaselineSet.
*       Note: Triggering cannot be disabled by this API.
*
*/
static GT_STATUS internal_cpssDxChPortPfcEnableSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_PORT_PFC_ENABLE_ENT pfcEnable
)
{
    GT_U32 value;         /* register value */
    GT_U32 regAddr;       /* register address */
    GT_STATUS rc;         /* function return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    if (pfcEnable==CPSS_DXCH_PORT_PFC_ENABLE_TRIGGERING_ONLY_E)
    {
        value=0;
    }
    else if (pfcEnable==CPSS_DXCH_PORT_PFC_ENABLE_TRIGGERING_AND_RESPONSE_E)
    {
        value=1;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Set Enable Priority Based FC in TTI Unit Global Config reg. */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.globalUnitConfig;
    }
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 24, 1, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* no TXQ-DQ of SIP5 */
        return GT_OK;
    }

    /* Set value in Flow Control Response Config reg. */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).flowCtrlResponse.flowCtrlResponseConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.dq.flowControl.flowControlConfig;
    }

    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, value);
}

/**
* @internal cpssDxChPortPfcEnableSet function
* @endinternal
*
* @brief   Enable/Disable PFC (Priority Flow Control) response functionality.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] pfcEnable                - PFC enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or PFC enable option
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If PFC response is enabled, the shaper’s baseline must be
*       at least 0x3FFFC0, see:
*       cpssDxChPortTxShaperBaselineSet.
*       Note: Triggering cannot be disabled by this API.
*
*/
GT_STATUS cpssDxChPortPfcEnableSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_PORT_PFC_ENABLE_ENT pfcEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pfcEnable));

    rc = internal_cpssDxChPortPfcEnableSet(devNum, pfcEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pfcEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcEnableGet function
* @endinternal
*
* @brief   Get the status of PFC response functionality.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
*
* @param[out] pfcEnablePtr             - (pointer to) PFC enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Triggering is enabled by default.
*
*/
static GT_STATUS internal_cpssDxChPortPfcEnableGet
(
    IN  GT_U8    devNum,
    OUT CPSS_DXCH_PORT_PFC_ENABLE_ENT *pfcEnablePtr
)
{
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */
    GT_U32 responseValue; /* registers value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(pfcEnablePtr);

    /* Get <PfcEn> field in the PFC Trigger Global Config reg.
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.pfcRegs.pfcGlobalConfigReg;
    rc =  prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, &trigValue);
    if (rc != GT_OK)
    {
        return rc;
    }*/

    /* Get Enable Priority Based FC in TTI Unit Global Config reg. */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.globalUnitConfig;
    }
    rc =  prvCpssHwPpGetRegField(devNum, regAddr, 24, 1, &responseValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    *pfcEnablePtr = ((responseValue == 0) ?
            CPSS_DXCH_PORT_PFC_ENABLE_TRIGGERING_ONLY_E :
            CPSS_DXCH_PORT_PFC_ENABLE_TRIGGERING_AND_RESPONSE_E);

    return GT_OK;
}

/**
* @internal cpssDxChPortPfcEnableGet function
* @endinternal
*
* @brief   Get the status of PFC response functionality.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
*
* @param[out] pfcEnablePtr             - (pointer to) PFC enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Triggering is enabled by default.
*
*/
GT_STATUS cpssDxChPortPfcEnableGet
(
    IN  GT_U8    devNum,
    OUT CPSS_DXCH_PORT_PFC_ENABLE_ENT *pfcEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pfcEnablePtr));

    rc = internal_cpssDxChPortPfcEnableGet(devNum, pfcEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pfcEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcProfileIndexSet function
* @endinternal
*
* @brief   Binds a source port to a PFC profile.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range profile index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPfcProfileIndexSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32   profileIndex
)
{
    GT_U32 regAddr;    /* register address */
    GT_U32 fieldOffset;/* the start bit number in the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    if (profileIndex >= BIT_3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCPortProfile[(portNum >> 3)];
        fieldOffset = (portNum%8)*3;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.pfcRegs.pfcSourcePortProfile;
        regAddr += (portNum/8)*4;
        fieldOffset = (portNum%8)*3;
    }

    return prvCpssHwPpSetRegField(devNum, regAddr,fieldOffset, 3, profileIndex);
}

/**
* @internal cpssDxChPortPfcProfileIndexSet function
* @endinternal
*
* @brief   Binds a source port to a PFC profile.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range profile index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPfcProfileIndexSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32   profileIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcProfileIndexSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, profileIndex));

    rc = internal_cpssDxChPortPfcProfileIndexSet(devNum, portNum, profileIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, profileIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcProfileIndexGet function
* @endinternal
*
* @brief   Gets the port's PFC profile.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
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
static GT_STATUS internal_cpssDxChPortPfcProfileIndexGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32  *profileIndexPtr
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 fieldOffset; /* the start bit number in the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                    CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    CPSS_NULL_PTR_CHECK_MAC(profileIndexPtr);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCPortProfile[(portNum >> 3)];
        fieldOffset = (portNum%8)*3;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.pfcRegs.pfcSourcePortProfile;
        regAddr += (portNum/8)*4;
        fieldOffset = (portNum%8)*3;
    }

    return prvCpssHwPpGetRegField(devNum, regAddr,fieldOffset, 3, profileIndexPtr);
}

/**
* @internal cpssDxChPortPfcProfileIndexGet function
* @endinternal
*
* @brief   Gets the port's PFC profile.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
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
GT_STATUS cpssDxChPortPfcProfileIndexGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32  *profileIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcProfileIndexGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, profileIndexPtr));

    rc = internal_cpssDxChPortPfcProfileIndexGet(devNum, portNum, profileIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, profileIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcProfileQueueConfigSet function
* @endinternal
*
* @brief   Sets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..7)
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] pfcProfileCfgPtr         - pointer to PFC Profile configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All thresholds are set in buffers or packets.
*       See cpssDxChPortPfcCountingModeSet.
*
*/
static GT_STATUS internal_cpssDxChPortPfcProfileQueueConfigSet
(
    IN GT_U8    devNum,
    IN GT_U32   profileIndex,
    IN GT_U8    tcQueue,
    IN CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC     *pfcProfileCfgPtr
)
{
    GT_U32 regAddr;   /* register address */
    GT_U32 data;   /* register address */
    GT_STATUS rc;     /* function return value */
    CPSS_DXCH_TABLE_ENT tableType;
    GT_U32 xonAlphaHwValue = 0, xoffAlphaHwValue = 0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(pfcProfileCfgPtr);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tcQueue);

    if (profileIndex > 7 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if (pfcProfileCfgPtr->xoffThreshold > PRV_CPSS_SIP5_PFC_MAX_VALUE_MAC(devNum) ||
            pfcProfileCfgPtr->xonThreshold  > PRV_CPSS_SIP5_PFC_MAX_VALUE_MAC(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        if (pfcProfileCfgPtr->xoffThreshold >= BIT_11 ||
            pfcProfileCfgPtr->xonThreshold >= BIT_11)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        tableType = CPSS_DXCH_SIP5_TABLE_PFC_MODE_PROFILE_TC_XOFF_THRESHOLDS_E;

        rc = prvCpssDxChWriteTableEntry(devNum, tableType, ((profileIndex<<3) + tcQueue), &(pfcProfileCfgPtr->xoffThreshold));
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.pfcRegs.pfcProfileXoffThresholds;
        regAddr += profileIndex*0x20 + tcQueue*0x4;

        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 11, pfcProfileCfgPtr->xoffThreshold);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Set alpha for PFC DBA for Aldrin 2 */
    if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_PORT_PFC_DBA_ALPHA_TO_HW_CONVERT_MAC(pfcProfileCfgPtr->xonAlpha, xonAlphaHwValue);
        PRV_CPSS_PORT_PFC_DBA_ALPHA_TO_HW_CONVERT_MAC(pfcProfileCfgPtr->xoffAlpha, xoffAlphaHwValue);
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).dba.PFCProfileTCAlphaThresholds[profileIndex][tcQueue];
        data = xonAlphaHwValue | (xoffAlphaHwValue << 4);

        rc =  prvCpssHwPpSetRegField(devNum, regAddr, 0, 8, data);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        tableType = CPSS_DXCH_SIP5_TABLE_PFC_MODE_PROFILE_TC_XON_THRESHOLDS_E;

        return prvCpssDxChWriteTableEntry(devNum, tableType, ((profileIndex<<3) + tcQueue), &(pfcProfileCfgPtr->xonThreshold));
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.pfcRegs.pfcProfileXonThresholds;
        regAddr += profileIndex*0x20 + tcQueue*0x4;

        return prvCpssHwPpSetRegField(devNum, regAddr, 0, 11, pfcProfileCfgPtr->xonThreshold);
    }
}

/**
* @internal cpssDxChPortPfcProfileQueueConfigSet function
* @endinternal
*
* @brief   Sets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..7)
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] pfcProfileCfgPtr         - pointer to PFC Profile configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All thresholds are set in buffers or packets.
*       See cpssDxChPortPfcCountingModeSet.
*
*/
GT_STATUS cpssDxChPortPfcProfileQueueConfigSet
(
    IN GT_U8    devNum,
    IN GT_U32   profileIndex,
    IN GT_U8    tcQueue,
    IN CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC     *pfcProfileCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcProfileQueueConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileIndex, tcQueue, pfcProfileCfgPtr));

    rc = internal_cpssDxChPortPfcProfileQueueConfigSet(devNum, profileIndex, tcQueue, pfcProfileCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileIndex, tcQueue, pfcProfileCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcProfileQueueConfigGet function
* @endinternal
*
* @brief   Gets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..7)
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
*
* @param[out] pfcProfileCfgPtr         - pointer to PFC Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All thresholds are set in buffers or packets.
*       See cpssDxChPortPfcCountingModeSet.
*
*/
static GT_STATUS internal_cpssDxChPortPfcProfileQueueConfigGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   profileIndex,
    IN  GT_U8    tcQueue,
    OUT CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC     *pfcProfileCfgPtr
)
{
    GT_U32 regAddr;   /* register address */
    GT_U32 data;   /* register address */
    GT_STATUS rc;     /* function return value */
    CPSS_DXCH_TABLE_ENT tableType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(pfcProfileCfgPtr);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tcQueue);

    if (profileIndex > 7 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        tableType = CPSS_DXCH_SIP5_TABLE_PFC_MODE_PROFILE_TC_XOFF_THRESHOLDS_E;

        rc = prvCpssDxChReadTableEntry(devNum, tableType, ((profileIndex<<3) + tcQueue), &(pfcProfileCfgPtr->xoffThreshold));
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.pfcRegs.pfcProfileXoffThresholds;
        regAddr += profileIndex*0x20 + tcQueue*0x4;
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 11, &(pfcProfileCfgPtr->xoffThreshold));
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).dba.PFCProfileTCAlphaThresholds[profileIndex][tcQueue];
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 4, &(data));
        if (rc != GT_OK)
        {
            return rc;
        }

        PRV_CPSS_PORT_PFC_DBA_HW_TO_ALPHA_CONVERT_MAC(data, pfcProfileCfgPtr->xonAlpha);
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 4, 4, &(data));
        if (rc != GT_OK)
        {
            return rc;
        }

        PRV_CPSS_PORT_PFC_DBA_HW_TO_ALPHA_CONVERT_MAC(data, pfcProfileCfgPtr->xoffAlpha);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        tableType = CPSS_DXCH_SIP5_TABLE_PFC_MODE_PROFILE_TC_XON_THRESHOLDS_E;

        return prvCpssDxChReadTableEntry(devNum, tableType, ((profileIndex<<3) + tcQueue), &(pfcProfileCfgPtr->xonThreshold));
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.pfcRegs.pfcProfileXonThresholds;
        regAddr += profileIndex*0x20 + tcQueue*0x4;
        return prvCpssHwPpGetRegField(devNum, regAddr, 0, 11, &(pfcProfileCfgPtr->xonThreshold));
    }
}

/**
* @internal cpssDxChPortPfcProfileQueueConfigGet function
* @endinternal
*
* @brief   Gets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..7)
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
*
* @param[out] pfcProfileCfgPtr         - pointer to PFC Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All thresholds are set in buffers or packets.
*       See cpssDxChPortPfcCountingModeSet.
*
*/
GT_STATUS cpssDxChPortPfcProfileQueueConfigGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   profileIndex,
    IN  GT_U8    tcQueue,
    OUT CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC     *pfcProfileCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcProfileQueueConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileIndex, tcQueue, pfcProfileCfgPtr));

    rc = internal_cpssDxChPortPfcProfileQueueConfigGet(devNum, profileIndex, tcQueue, pfcProfileCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileIndex, tcQueue, pfcProfileCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcCountingModeSet function
* @endinternal
*
* @brief   Sets PFC counting mode.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] pfcCountMode             - PFC counting mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPfcCountingModeSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_PORT_PFC_COUNT_MODE_ENT  pfcCountMode
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    switch(pfcCountMode)
    {
        case CPSS_DXCH_PORT_PFC_COUNT_BUFFERS_MODE_E:
            value = 0;
            break;
        case CPSS_DXCH_PORT_PFC_COUNT_PACKETS_E:
            value = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* For lion2 and above configure TXQ unit as well */
    if (PRV_CPSS_PP_MAC(devNum)->appDevFamily != CPSS_LION_E)
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) != GT_TRUE)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.llfcRegs.pfcGlobalConfigReg;
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 1, 1, value);
            if (rc != GT_OK)
            {
                    return rc;
            }
        }
    }

    /* TXQ unit in Lion, CPFC unit in Lion2 and above*/
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCTriggerGlobalConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.pfcRegs.pfcGlobalConfigReg;
    }
    return prvCpssHwPpSetRegField(devNum, regAddr, 1, 1, value);

}

/**
* @internal cpssDxChPortPfcCountingModeSet function
* @endinternal
*
* @brief   Sets PFC counting mode.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] pfcCountMode             - PFC counting mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPfcCountingModeSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_PORT_PFC_COUNT_MODE_ENT  pfcCountMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcCountingModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pfcCountMode));

    rc = internal_cpssDxChPortPfcCountingModeSet(devNum, pfcCountMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pfcCountMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcCountingModeGet function
* @endinternal
*
* @brief   Gets PFC counting mode.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
*
* @param[out] pfcCountModePtr          - (pointer to) PFC counting mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPfcCountingModeGet
(
    IN  GT_U8    devNum,
    OUT CPSS_DXCH_PORT_PFC_COUNT_MODE_ENT  *pfcCountModePtr
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(pfcCountModePtr);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCTriggerGlobalConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.pfcRegs.pfcGlobalConfigReg;
    }
    rc =  prvCpssHwPpGetRegField(devNum, regAddr, 1, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }
    *pfcCountModePtr = (CPSS_DXCH_PORT_PFC_COUNT_MODE_ENT)value;

    return GT_OK;
}

/**
* @internal cpssDxChPortPfcCountingModeGet function
* @endinternal
*
* @brief   Gets PFC counting mode.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
*
* @param[out] pfcCountModePtr          - (pointer to) PFC counting mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPfcCountingModeGet
(
    IN  GT_U8    devNum,
    OUT CPSS_DXCH_PORT_PFC_COUNT_MODE_ENT  *pfcCountModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcCountingModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pfcCountModePtr));

    rc = internal_cpssDxChPortPfcCountingModeGet(devNum, pfcCountModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pfcCountModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcGlobalDropEnableSet function
* @endinternal
*
* @brief   Enable/Disable PFC global drop.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE: Enable PFC global drop.
*                                      GT_FALSE: Disable PFC global drop.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To configure drop threshold use cpssDxChPortPfcGlobalQueueConfigSet.
*
*/
static GT_STATUS internal_cpssDxChPortPfcGlobalDropEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_U32 bitIndex;  /* bit index */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    value = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        bitIndex = 10;
    }
    else
    {
        bitIndex = 9;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCTriggerGlobalConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.pfcRegs.pfcGlobalConfigReg;
    }
    return prvCpssHwPpSetRegField(devNum, regAddr, bitIndex, 1, value);
}

/**
* @internal cpssDxChPortPfcGlobalDropEnableSet function
* @endinternal
*
* @brief   Enable/Disable PFC global drop.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE: Enable PFC global drop.
*                                      GT_FALSE: Disable PFC global drop.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To configure drop threshold use cpssDxChPortPfcGlobalQueueConfigSet.
*
*/
GT_STATUS cpssDxChPortPfcGlobalDropEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcGlobalDropEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChPortPfcGlobalDropEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcGlobalDropEnableGet function
* @endinternal
*
* @brief   Gets the current status of PFC global drop.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) status of PFC functionality
*                                      GT_TRUE:  PFC global drop enabled.
*                                      GT_FALSE: PFC global drop disabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPfcGlobalDropEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */
    GT_U32 bitIndex;  /* bit index */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        bitIndex = 10;
    }
    else
    {
        bitIndex = 9;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCTriggerGlobalConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.pfcRegs.pfcGlobalConfigReg;
    }
    rc =  prvCpssHwPpGetRegField(devNum, regAddr, bitIndex, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }
    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChPortPfcGlobalDropEnableGet function
* @endinternal
*
* @brief   Gets the current status of PFC global drop.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) status of PFC functionality
*                                      GT_TRUE:  PFC global drop enabled.
*                                      GT_FALSE: PFC global drop disabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPfcGlobalDropEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcGlobalDropEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChPortPfcGlobalDropEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcGlobalQueueConfigSet function
* @endinternal
*
* @brief   Sets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] xoffThreshold            - XOFF threshold
*                                      (APPLICABLE RANGES: Lion, Lion2 0..0x7FF)
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..0x1FFFFFFF)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..0x1FFFF)
* @param[in] dropThreshold            - Drop threshold. When a global counter with given tcQueue
*                                      crosses up the dropThreshold the packets are dropped
*                                      (APPLICABLE RANGES: Lion, Lion2 0..0x7FF)
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..0x1FFFFFFF)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..0x1FFFF)
* @param[in] xonThreshold             - XON threshold
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3)
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..0x1FFFFFFF)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..0x1FFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. dropThreshold is used when PFC global drop is enabled.
*       See cpssDxChPortPfcGlobalDropEnableSet.
*       2. All thresholds are set in buffers or packets.
*       See cpssDxChPortPfcCountingModeSet.
*
*/
static GT_STATUS internal_cpssDxChPortPfcGlobalQueueConfigSet
(
    IN GT_U8    devNum,
    IN GT_U8    tcQueue,
    IN GT_U32   xoffThreshold,
    IN GT_U32   dropThreshold,
    IN GT_U32   xonThreshold
)
{
    GT_U32 regAddr;   /* register address */
    GT_U32 value;     /* register value */
    GT_STATUS rc;     /* function return value */
    GT_U32 portGroupsNum; /* number of port groups in device*/
    GT_U32 i;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tcQueue);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if (xoffThreshold > PRV_CPSS_SIP5_PFC_MAX_VALUE_MAC(devNum) ||
            dropThreshold > PRV_CPSS_SIP5_PFC_MAX_VALUE_MAC(devNum) ||
            xonThreshold  > PRV_CPSS_SIP5_PFC_MAX_VALUE_MAC(devNum) )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        if (xoffThreshold >= BIT_11 || dropThreshold >= BIT_11)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCGroupOfPortsTCXoffThresholds[tcQueue][0];
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0,
                    PRV_CPSS_SIP5_PFC_BITS_NUM_MAC(devNum), xoffThreshold);
        if (rc != GT_OK)
        {
            return rc;
        }

        regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCGroupOfPortsTCDropThresholds[tcQueue][0];
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0,
                    PRV_CPSS_SIP5_PFC_BITS_NUM_MAC(devNum), dropThreshold);
        if (rc != GT_OK)
        {
            return rc;
        }

        regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCGroupOfPortsTCXonThresholds[tcQueue][0];
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0,
                    PRV_CPSS_SIP5_PFC_BITS_NUM_MAC(devNum), xonThreshold);
        if (rc != GT_OK)
        {
            return rc;
        }

    }
    else
    {
        value = xoffThreshold | (dropThreshold << 11);

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.queue.pfcRegs.pfcGlobaGroupOfPortsThresholds;
        regAddr += tcQueue*0x4;

        portGroupsNum = (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E) ?
            PRV_CPSS_LION_PORT_GROUPS_NUM_CNS : PRV_CPSS_LION2_PORT_GROUPS_NUM_CNS;

        /* set the value to all port groups*/
        for (i=0; i < portGroupsNum; i++)
        {
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 22, value);
            if (rc != GT_OK)
            {
                return rc;
            }
            regAddr += 0x20;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortPfcGlobalQueueConfigSet function
* @endinternal
*
* @brief   Sets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] xoffThreshold            - XOFF threshold
*                                      (APPLICABLE RANGES: Lion, Lion2 0..0x7FF)
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..0x1FFFFFFF)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..0x1FFFF)
* @param[in] dropThreshold            - Drop threshold. When a global counter with given tcQueue
*                                      crosses up the dropThreshold the packets are dropped
*                                      (APPLICABLE RANGES: Lion, Lion2 0..0x7FF)
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..0x1FFFFFFF)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..0x1FFFF)
* @param[in] xonThreshold             - XON threshold
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3)
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..0x1FFFFFFF)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..0x1FFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. dropThreshold is used when PFC global drop is enabled.
*       See cpssDxChPortPfcGlobalDropEnableSet.
*       2. All thresholds are set in buffers or packets.
*       See cpssDxChPortPfcCountingModeSet.
*
*/
GT_STATUS cpssDxChPortPfcGlobalQueueConfigSet
(
    IN GT_U8    devNum,
    IN GT_U8    tcQueue,
    IN GT_U32   xoffThreshold,
    IN GT_U32   dropThreshold,
    IN GT_U32   xonThreshold
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcGlobalQueueConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tcQueue, xoffThreshold, dropThreshold, xonThreshold));

    rc = internal_cpssDxChPortPfcGlobalQueueConfigSet(devNum, tcQueue, xoffThreshold, dropThreshold, xonThreshold);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tcQueue, xoffThreshold, dropThreshold, xonThreshold));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcGlobalQueueConfigGet function
* @endinternal
*
* @brief   Gets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
*
* @param[out] xoffThresholdPtr         - (pointer to) Xoff threshold.
* @param[out] dropThresholdPtr         - (pointer to) Drop threshold. When a global counter with given tcQueue
*                                      crosses up the dropThreshold the packets are dropped.
* @param[out] xonThresholdPtr          - (pointer to) Xon threshold.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All thresholds are set in buffers or packets.
*       See cpssDxChPortPfcCountingModeSet.
*
*/
static GT_STATUS internal_cpssDxChPortPfcGlobalQueueConfigGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    tcQueue,
    OUT GT_U32   *xoffThresholdPtr,
    OUT GT_U32   *dropThresholdPtr,
    OUT GT_U32   *xonThresholdPtr
)
{
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */
    GT_U32 value;     /* register value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(xoffThresholdPtr);
    CPSS_NULL_PTR_CHECK_MAC(dropThresholdPtr);
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        CPSS_NULL_PTR_CHECK_MAC(xonThresholdPtr);
    }
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tcQueue);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCGroupOfPortsTCXoffThresholds[tcQueue][0];
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 0,
                    PRV_CPSS_SIP5_PFC_BITS_NUM_MAC(devNum), xoffThresholdPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCGroupOfPortsTCDropThresholds[tcQueue][0];
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 0,
                    PRV_CPSS_SIP5_PFC_BITS_NUM_MAC(devNum), dropThresholdPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCGroupOfPortsTCXonThresholds[tcQueue][0];
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 0,
                    PRV_CPSS_SIP5_PFC_BITS_NUM_MAC(devNum), xonThresholdPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.queue.pfcRegs.pfcGlobaGroupOfPortsThresholds;
        regAddr += tcQueue*0x4;
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 22, &value);
        if (rc != GT_OK)
        {
            return rc;
        }
        *xoffThresholdPtr = value & 0x7FF;
        *dropThresholdPtr = value >> 11;
    }
    return GT_OK;
}

/**
* @internal cpssDxChPortPfcGlobalQueueConfigGet function
* @endinternal
*
* @brief   Gets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
*
* @param[out] xoffThresholdPtr         - (pointer to) Xoff threshold.
* @param[out] dropThresholdPtr         - (pointer to) Drop threshold. When a global counter with given tcQueue
*                                      crosses up the dropThreshold the packets are dropped.
* @param[out] xonThresholdPtr          - (pointer to) Xon threshold.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All thresholds are set in buffers or packets.
*       See cpssDxChPortPfcCountingModeSet.
*
*/
GT_STATUS cpssDxChPortPfcGlobalQueueConfigGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    tcQueue,
    OUT GT_U32   *xoffThresholdPtr,
    OUT GT_U32   *dropThresholdPtr,
    OUT GT_U32   *xonThresholdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcGlobalQueueConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tcQueue, xoffThresholdPtr, dropThresholdPtr, xonThresholdPtr));

    rc = internal_cpssDxChPortPfcGlobalQueueConfigGet(devNum, tcQueue, xoffThresholdPtr, dropThresholdPtr, xonThresholdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tcQueue, xoffThresholdPtr, dropThresholdPtr, xonThresholdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcTimerMapEnableSet function
* @endinternal
*
* @brief   Enables mapping of PFC timer to priority queue for given scheduler profile.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile.
* @param[in] enable                   - Determines whether PFC timer to Priority Queue map
*                                      is used.
*                                      GT_TRUE: PFC timer to Priority Queue map used.
*                                      GT_FALSE: PFC timer to Priority Queue map bypassed.
*                                      1:1 mapping between a timer in PFC frame to an egress queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To bind port to scheduler profile use:
*       cpssDxChPortTxBindPortToSchedulerProfileSet.
*
*/
static GT_STATUS internal_cpssDxChPortPfcTimerMapEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    IN  GT_BOOL                                 enable

)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_SCHEDULER_PROFILE_CHECK_MAC(profileSet, devNum);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    value = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).flowCtrlResponse.schedulerProfilePFCTimerToPriorityQueueMapEnable[profileSet];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.dq.flowControl.schedulerProfileTcToPriorityMapEnable[profileSet];
    }
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, value);
}

/**
* @internal cpssDxChPortPfcTimerMapEnableSet function
* @endinternal
*
* @brief   Enables mapping of PFC timer to priority queue for given scheduler profile.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile.
* @param[in] enable                   - Determines whether PFC timer to Priority Queue map
*                                      is used.
*                                      GT_TRUE: PFC timer to Priority Queue map used.
*                                      GT_FALSE: PFC timer to Priority Queue map bypassed.
*                                      1:1 mapping between a timer in PFC frame to an egress queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To bind port to scheduler profile use:
*       cpssDxChPortTxBindPortToSchedulerProfileSet.
*
*/
GT_STATUS cpssDxChPortPfcTimerMapEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    IN  GT_BOOL                                 enable

)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcTimerMapEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, enable));

    rc = internal_cpssDxChPortPfcTimerMapEnableSet(devNum, profileSet, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcTimerMapEnableGet function
* @endinternal
*
* @brief   Get the status of PFS timer to priority queue mapping for given
*         scheduler profile.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile Set
*
* @param[out] enablePtr                - (pointer to) status of PFC timer to Priority Queue
*                                      mapping.
*                                      GT_TRUE: PFC timer to Priority Queue map used.
*                                      GT_FALSE: PFC timer to Priority Queue map bypassed.
*                                      1:1 mapping between a timer in PFC frame to an egress queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPfcTimerMapEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    OUT GT_BOOL                                *enablePtr

)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    PRV_CPSS_DXCH_SCHEDULER_PROFILE_CHECK_MAC(profileSet, devNum);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).flowCtrlResponse.schedulerProfilePFCTimerToPriorityQueueMapEnable[profileSet];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.dq.flowControl.schedulerProfileTcToPriorityMapEnable[profileSet];
    }
    rc =  prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }
    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChPortPfcTimerMapEnableGet function
* @endinternal
*
* @brief   Get the status of PFS timer to priority queue mapping for given
*         scheduler profile.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile Set
*
* @param[out] enablePtr                - (pointer to) status of PFC timer to Priority Queue
*                                      mapping.
*                                      GT_TRUE: PFC timer to Priority Queue map used.
*                                      GT_FALSE: PFC timer to Priority Queue map bypassed.
*                                      1:1 mapping between a timer in PFC frame to an egress queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPfcTimerMapEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    OUT GT_BOOL                                *enablePtr

)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcTimerMapEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, enablePtr));

    rc = internal_cpssDxChPortPfcTimerMapEnableGet(devNum, profileSet, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcTimerToQueueMapSet function
* @endinternal
*
* @brief   Sets PFC timer to priority queue map.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] pfcTimer                 - PFC timer (0..7)
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or PFC timer
* @retval GT_OUT_OF_RANGE          - on out of traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPfcTimerToQueueMapSet
(
    IN  GT_U8     devNum,
    IN  GT_U32    pfcTimer,
    IN  GT_U32    tcQueue

)
{
    GT_U32 regAddr;   /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tcQueue);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);


    if (pfcTimer >= 8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).flowCtrlResponse.PFCTimerToPriorityQueueMap[pfcTimer];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.dq.flowControl.pfcTimerToPriorityMapping[pfcTimer];
    }
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 3, tcQueue);
}

/**
* @internal cpssDxChPortPfcTimerToQueueMapSet function
* @endinternal
*
* @brief   Sets PFC timer to priority queue map.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] pfcTimer                 - PFC timer (0..7)
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or PFC timer
* @retval GT_OUT_OF_RANGE          - on out of traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPfcTimerToQueueMapSet
(
    IN  GT_U8     devNum,
    IN  GT_U32    pfcTimer,
    IN  GT_U32    tcQueue

)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcTimerToQueueMapSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pfcTimer, tcQueue));

    rc = internal_cpssDxChPortPfcTimerToQueueMapSet(devNum, pfcTimer, tcQueue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pfcTimer, tcQueue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcTimerToQueueMapGet function
* @endinternal
*
* @brief   Gets PFC timer to priority queue map.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] pfcTimer                 - PFC timer (0..7).
*
* @param[out] tcQueuePtr               - (pointer to) traffic class queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or PFC timer
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPfcTimerToQueueMapGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    pfcTimer,
    OUT GT_U32    *tcQueuePtr

)
{
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(tcQueuePtr);
    if (pfcTimer >= 8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);


    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).flowCtrlResponse.PFCTimerToPriorityQueueMap[pfcTimer];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.dq.flowControl.pfcTimerToPriorityMapping[pfcTimer];
    }

    rc =  prvCpssHwPpGetRegField(devNum, regAddr, 0, 3, tcQueuePtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortPfcTimerToQueueMapGet function
* @endinternal
*
* @brief   Gets PFC timer to priority queue map.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] pfcTimer                 - PFC timer (0..7).
*
* @param[out] tcQueuePtr               - (pointer to) traffic class queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or PFC timer
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPfcTimerToQueueMapGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    pfcTimer,
    OUT GT_U32    *tcQueuePtr

)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcTimerToQueueMapGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pfcTimer, tcQueuePtr));

    rc = internal_cpssDxChPortPfcTimerToQueueMapGet(devNum, pfcTimer, tcQueuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pfcTimer, tcQueuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcShaperToPortRateRatioSet function
* @endinternal
*
* @brief   Sets shaper rate to port speed ratio on given scheduler profile
*         and traffic class queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] shaperToPortRateRatio    - shaper rate to port speed ratio
*                                      in percentage (0..100).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number ,profile set
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range shaper rate to port speed ratio
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To bind port to scheduler profile use:
*       cpssDxChPortTxBindPortToSchedulerProfileSet.
*
*/
static GT_STATUS internal_cpssDxChPortPfcShaperToPortRateRatioSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    IN  GT_U8                                   tcQueue,
    IN  GT_U32                                  shaperToPortRateRatio

)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_SCHEDULER_PROFILE_CHECK_MAC(profileSet, devNum);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tcQueue);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum)) CPSS_BOOKMARK_TBD_FALCON_TXQ
    {
#if 0
    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);
#endif
        return GT_OK;
    }

    if (shaperToPortRateRatio > 100)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if (shaperToPortRateRatio == 0)
    {
        value = 0;
    }
    else
    {
        /* PFCFactor = ROUNDUP(shaperToPortRateRatio*128/100) - 1 */
        value = (shaperToPortRateRatio*128)/100 -
            (((shaperToPortRateRatio*128)%100 == 0) ? 1 :0);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).flowCtrlResponse.schedulerProfilePriorityQueuePFCFactor[profileSet][tcQueue];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.dq.flowControl.tcProfilePfcFactor[tcQueue][profileSet];
    }
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 7, value);
}

/**
* @internal cpssDxChPortPfcShaperToPortRateRatioSet function
* @endinternal
*
* @brief   Sets shaper rate to port speed ratio on given scheduler profile
*         and traffic class queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] shaperToPortRateRatio    - shaper rate to port speed ratio
*                                      in percentage (0..100).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number ,profile set
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range shaper rate to port speed ratio
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To bind port to scheduler profile use:
*       cpssDxChPortTxBindPortToSchedulerProfileSet.
*
*/
GT_STATUS cpssDxChPortPfcShaperToPortRateRatioSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    IN  GT_U8                                   tcQueue,
    IN  GT_U32                                  shaperToPortRateRatio

)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcShaperToPortRateRatioSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, tcQueue, shaperToPortRateRatio));

    rc = internal_cpssDxChPortPfcShaperToPortRateRatioSet(devNum, profileSet, tcQueue, shaperToPortRateRatio);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, tcQueue, shaperToPortRateRatio));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcShaperToPortRateRatioGet function
* @endinternal
*
* @brief   Gets shaper rate to port speed ratio on given scheduler profile
*         and traffic class queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] shaperToPortRateRatioPtr - (pointer to)shaper rate to port speed ratio
*                                      in percentage.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number ,profile set
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssDxChPortPfcShaperToPortRateRatioGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    IN  GT_U8                                   tcQueue,
    OUT GT_U32                                 *shaperToPortRateRatioPtr

)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(shaperToPortRateRatioPtr);
    PRV_CPSS_DXCH_SCHEDULER_PROFILE_CHECK_MAC(profileSet, devNum);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tcQueue);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);


    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).flowCtrlResponse.schedulerProfilePriorityQueuePFCFactor[profileSet][tcQueue];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.dq.flowControl.tcProfilePfcFactor[tcQueue][profileSet];
    }

    rc =  prvCpssHwPpGetRegField(devNum, regAddr, 0, 7, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (value == 0)
    {
        *shaperToPortRateRatioPtr = 0;
    }
    else
    {
        /* shaperToPortRateRatio = ROUNDDOWN(PFCFactor*100/128) + 1 */
        *shaperToPortRateRatioPtr = (value*100)/128 + 1;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortPfcShaperToPortRateRatioGet function
* @endinternal
*
* @brief   Gets shaper rate to port speed ratio on given scheduler profile
*         and traffic class queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] shaperToPortRateRatioPtr - (pointer to)shaper rate to port speed ratio
*                                      in percentage.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number ,profile set
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChPortPfcShaperToPortRateRatioGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    IN  GT_U8                                   tcQueue,
    OUT GT_U32                                 *shaperToPortRateRatioPtr

)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcShaperToPortRateRatioGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, tcQueue, shaperToPortRateRatioPtr));

    rc = internal_cpssDxChPortPfcShaperToPortRateRatioGet(devNum, profileSet, tcQueue, shaperToPortRateRatioPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, tcQueue, shaperToPortRateRatioPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortPfcCascadeEnableSet function
* @endinternal
*
* @brief   Enable/Disable insertion of DSA tag for PFC frames
*         transmitted on given port.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port number.
* @param[in] enable                   - GT_TRUE - PFC packets are transmitted with a DSA tag.
*                                      GT_FALSE - PFC packets are transmitted without DSA tag.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortPfcCascadeEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_U32 regAddr;    /* register address */
    GT_U32 value;      /* register value */
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum, portMacNum);

    value = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_TBD_BOOKMARK_FALCON /* definition for PFC configuration not ready yet */
        return GT_OK;
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;

        /* set PFC Cascade Port Enable */
        return prvCpssHwPpPortGroupSetRegField(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                regAddr, 9, 1, value);
    }
    else
    {
        if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

        PRV_CPSS_DXCH_PORT_MAC_CTRL1_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 3;
        }

        PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
            regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
            regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = 15;
        }

        PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
            regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
            regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 15;
        }

        return prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
    }
}

/**
* @internal internal_cpssDxChPortPfcForwardEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding of PFC frames to the ingress
*         pipeline of a specified port.
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] enable                   - GT_TRUE:  forward PFC frames to the ingress pipe,
*                                      GT_FALSE: do not forward PFC frames to the ingress pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note A packet is considered as a PFC frame if all of the following are true:
*       - Packet’s Length/EtherType field is 88-08
*       - Packet’s OpCode field is 01-01
*       - Packet’s MAC DA is 01-80-C2-00-00-01 or the port’s configured MAC Address
*
*/
static GT_STATUS internal_cpssDxChPortPfcForwardEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_BOOL enable
)
{
    GT_U32 regAddr;    /* register address */
    GT_U32 value;      /* register value */
    GT_U32 portMacNum;      /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    value = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        return prvCpssDxChPortSip6PfcPauseFwdEnableSet(devNum,portNum,enable);
    }

    if (CPSS_PP_FAMILY_DXCH_XCAT2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                                    perPortRegs[portMacNum].serialParameters1;
        return prvCpssDrvHwPpPortGroupSetRegField(devNum,
                                                CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                        regAddr, 1, 1, value);
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].serialParameters1;
    }
    else
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
    }

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 1;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = 6;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 6;
    }

    /* support for CG port */
    regAddr = PRV_DXCH_REG1_UNIT_CG_CONVERTERS_MAC(devNum,portMacNum).CGMAConvertersFcControl0;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_CG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = 17;/*<Forward PFC Enable>*/
    }

    return prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal cpssDxChPortPfcForwardEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding of PFC frames to the ingress
*         pipeline of a specified port.
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] enable                   - GT_TRUE:  forward PFC frames to the ingress pipe,
*                                      GT_FALSE: do not forward PFC frames to the ingress pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note A packet is considered as a PFC frame if all of the following are true:
*       - Packet’s Length/EtherType field is 88-08
*       - Packet’s OpCode field is 01-01
*       - Packet’s MAC DA is 01-80-C2-00-00-01 or the port’s configured MAC Address
*
*/
GT_STATUS cpssDxChPortPfcForwardEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcForwardEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortPfcForwardEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortPfcForwardEnableGet function
* @endinternal
*
* @brief   Get status of PFC frames forwarding
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (CPU port not supported)
*
* @param[out] enablePtr                - current forward PFC frames status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note A packet is considered as a PFC frame if all of the following are true:
*       - Packet’s Length/EtherType field is 88-08
*       - Packet’s OpCode field is 01-01
*       - Packet’s MAC DA is 01-80-C2-00-00-01 or the port’s configured MAC Address
*
*/
static GT_STATUS internal_cpssDxChPortPfcForwardEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_U32 regAddr;    /* register address */
    GT_U32 portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32 value;      /* register value */
    GT_STATUS rc;      /* return code */
    GT_U32 offset;     /* bit number inside register       */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* type of mac unit of port */
    GT_U32          portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        return prvCpssDxChPortSip6PfcPauseFwdEnableGet(devNum,portNum,enablePtr);
    }

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].serialParameters1;
        if((rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 1, 1, &value)) != GT_OK)
            return rc;
    }
    else
    {
        portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);
        if(portMacType == PRV_CPSS_PORT_CG_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_CG_CONVERTERS_MAC(devNum,portMacNum).CGMAConvertersFcControl0;
            offset  = 17;/*<Forward PFC Enable>*/
        }
        else
        {
            PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
            if(portMacType >= PRV_CPSS_PORT_XG_E)
            {
                offset = 6;
            }
            else
            {
                if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
                {
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].serialParameters1;
                }

                offset = 1;
            }
        }

        if((rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, offset, 1, &value)) != GT_OK)
            return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);
    return GT_OK;
}

/**
* @internal cpssDxChPortPfcForwardEnableGet function
* @endinternal
*
* @brief   Get status of PFC frames forwarding
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (CPU port not supported)
*
* @param[out] enablePtr                - current forward PFC frames status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note A packet is considered as a PFC frame if all of the following are true:
*       - Packet’s Length/EtherType field is 88-08
*       - Packet’s OpCode field is 01-01
*       - Packet’s MAC DA is 01-80-C2-00-00-01 or the port’s configured MAC Address
*
*/
GT_STATUS cpssDxChPortPfcForwardEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcForwardEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortPfcForwardEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcLossyDropQueueEnableSet function
* @endinternal
*
* @brief   Enable/Disable lossy drop for packets with given traffic class queue.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] enable                   - GT_TRUE: Lossy - when lossy drop threshold is
*                                      reached, packets assigned with given tcQueue are dropped.
*                                      - GT_FALSE: Lossless - when lossy drop threshold is
*                                      reached, packets assigned with given tcQueue are not dropped.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device provides a burst absortion system designed to sustain contain
*       the packets in case of an incast type traffic (many-to-one).
*       Two thresholds are used:
*       1. Lossy drop - packets packets with lossy TC are dropped in
*       congested port group.
*       2. PFC Xoff threshold - PFC is sent to all of the ports in the port group
*       with all timers set to 0xFFFF.
*
*/
static GT_STATUS internal_cpssDxChPortPfcLossyDropQueueEnableSet
(
    IN GT_U8     devNum,
    IN GT_U8     tcQueue,
    IN GT_BOOL   enable
)
{
    GT_U32 regAddr;   /* register address */
    GT_U32 value;     /* register value */
    GT_U32 bitIndex;  /* bit index of the field */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E );

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tcQueue);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            preEgrEngineGlobalConfig.lossyDropConfig;
        bitIndex = tcQueue;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->eq.globalConfig.dualDevIdAndLossyDropConfig;
        bitIndex = tcQueue + 1;
    }

    value = BOOL2BIT_MAC(enable);
    return prvCpssHwPpSetRegField(devNum, regAddr, bitIndex, 1, value);
}

/**
* @internal cpssDxChPortPfcLossyDropQueueEnableSet function
* @endinternal
*
* @brief   Enable/Disable lossy drop for packets with given traffic class queue.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] enable                   - GT_TRUE: Lossy - when lossy drop threshold is
*                                      reached, packets assigned with given tcQueue are dropped.
*                                      - GT_FALSE: Lossless - when lossy drop threshold is
*                                      reached, packets assigned with given tcQueue are not dropped.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device provides a burst absortion system designed to sustain contain
*       the packets in case of an incast type traffic (many-to-one).
*       Two thresholds are used:
*       1. Lossy drop - packets packets with lossy TC are dropped in
*       congested port group.
*       2. PFC Xoff threshold - PFC is sent to all of the ports in the port group
*       with all timers set to 0xFFFF.
*
*/
GT_STATUS cpssDxChPortPfcLossyDropQueueEnableSet
(
    IN GT_U8     devNum,
    IN GT_U8     tcQueue,
    IN GT_BOOL   enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcLossyDropQueueEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tcQueue, enable));

    rc = internal_cpssDxChPortPfcLossyDropQueueEnableSet(devNum, tcQueue, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tcQueue, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcLossyDropQueueEnableGet function
* @endinternal
*
* @brief   Get the status of lossy drop on given traffic class queue.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] enablePtr                (pointer to)
*                                      - GT_TRUE: Lossy - when lossy drop threshold is
*                                      reached, packets assigned with given tcQueue are dropped.
*                                      - GT_FALSE: Lossless - when lossy drop threshold is
*                                      reached, packets assigned with given tcQueue are not dropped.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
static GT_STATUS internal_cpssDxChPortPfcLossyDropQueueEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_U8     tcQueue,
    OUT GT_BOOL  *enablePtr
)
{
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */
    GT_U32 value;     /* register value */
    GT_U32 bitIndex;  /* bit index of the field */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E );

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tcQueue);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            preEgrEngineGlobalConfig.lossyDropConfig;
        bitIndex = tcQueue;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->eq.globalConfig.dualDevIdAndLossyDropConfig;
        bitIndex = tcQueue + 1;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, bitIndex, 1, &value);
    if (rc != GT_OK)
        return rc;
    *enablePtr = BIT2BOOL_MAC(value);
    return GT_OK;
}

/**
* @internal cpssDxChPortPfcLossyDropQueueEnableGet function
* @endinternal
*
* @brief   Get the status of lossy drop on given traffic class queue.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] enablePtr                (pointer to)
*                                      - GT_TRUE: Lossy - when lossy drop threshold is
*                                      reached, packets assigned with given tcQueue are dropped.
*                                      - GT_FALSE: Lossless - when lossy drop threshold is
*                                      reached, packets assigned with given tcQueue are not dropped.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS cpssDxChPortPfcLossyDropQueueEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_U8     tcQueue,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcLossyDropQueueEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tcQueue, enablePtr));

    rc = internal_cpssDxChPortPfcLossyDropQueueEnableGet(devNum, tcQueue, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tcQueue, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcLossyDropConfigSet function
* @endinternal
*
* @brief   Set lossy drop configurations.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] lossyDropConfigPtr       - (pointer to) lossy drop configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The device provides a burst absortion system designed to sustain contain
*       the packets in case of an incast type traffic (many-to-one).
*       Two thresholds are used:
*       1. Lossy drop - packets packets with lossy TC are dropped in
*       congested port group.
*       2. PFC Xoff threshold - PFC is sent to all of the ports in the port group
*       with all timers set to 0xFFFF.
*
*/
static GT_STATUS internal_cpssDxChPortPfcLossyDropConfigSet
(
    IN GT_U8     devNum,
    IN CPSS_DXCH_PORT_PFC_LOSSY_DROP_CONFIG_STC *lossyDropConfigPtr
)
{
    GT_U32 regAddr;   /* register address */
    GT_U32 value;     /* register value */
    GT_U32 bitIndex;  /* bit index of the field */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E );

    CPSS_NULL_PTR_CHECK_MAC(lossyDropConfigPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            preEgrEngineGlobalConfig.lossyDropConfig;
        bitIndex = 8;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->eq.globalConfig.dualDevIdAndLossyDropConfig;
        bitIndex = 9;
    }

    value = (BOOL2BIT_MAC(lossyDropConfigPtr->fromCpuLossyDropEnable) << 2) |
            (BOOL2BIT_MAC(lossyDropConfigPtr->toTargetSnifferLossyDropEnable) << 1) |
             BOOL2BIT_MAC(lossyDropConfigPtr->toCpuLossyDropEnable);
    return prvCpssHwPpSetRegField(devNum, regAddr, bitIndex, 3, value);
}

/**
* @internal cpssDxChPortPfcLossyDropConfigSet function
* @endinternal
*
* @brief   Set lossy drop configurations.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] lossyDropConfigPtr       - (pointer to) lossy drop configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The device provides a burst absortion system designed to sustain contain
*       the packets in case of an incast type traffic (many-to-one).
*       Two thresholds are used:
*       1. Lossy drop - packets packets with lossy TC are dropped in
*       congested port group.
*       2. PFC Xoff threshold - PFC is sent to all of the ports in the port group
*       with all timers set to 0xFFFF.
*
*/
GT_STATUS cpssDxChPortPfcLossyDropConfigSet
(
    IN GT_U8     devNum,
    IN CPSS_DXCH_PORT_PFC_LOSSY_DROP_CONFIG_STC *lossyDropConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcLossyDropConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, lossyDropConfigPtr));

    rc = internal_cpssDxChPortPfcLossyDropConfigSet(devNum, lossyDropConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, lossyDropConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcLossyDropConfigGet function
* @endinternal
*
* @brief   Get lossy drop configurations.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number.
*
* @param[out] lossyDropConfigPtr       - (pointer to) lossy drop configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The device provides a burst absortion system designed to sustain contain
*       the packets in case of an incast type traffic (many-to-one).
*       Two thresholds are used:
*       1. Lossy drop - packets packets with lossy TC are dropped in
*       congested port group.
*       2. PFC Xoff threshold - PFC is sent to all of the ports in the port group
*       with all timers set to 0xFFFF.
*
*/
static GT_STATUS internal_cpssDxChPortPfcLossyDropConfigGet
(
    IN  GT_U8     devNum,
    OUT CPSS_DXCH_PORT_PFC_LOSSY_DROP_CONFIG_STC *lossyDropConfigPtr
)
{
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */
    GT_U32 value;     /* register value */
    GT_U32 bitIndex;  /* bit index of the field */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E );

    CPSS_NULL_PTR_CHECK_MAC(lossyDropConfigPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            preEgrEngineGlobalConfig.lossyDropConfig;
        bitIndex = 8;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->eq.globalConfig.dualDevIdAndLossyDropConfig;
        bitIndex = 9;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, bitIndex, 3, &value);
    if (rc != GT_OK)
        return rc;

    lossyDropConfigPtr->fromCpuLossyDropEnable = BIT2BOOL_MAC(((value >> 2) & 1));
    lossyDropConfigPtr->toTargetSnifferLossyDropEnable = BIT2BOOL_MAC(((value >> 1) & 1));
    lossyDropConfigPtr->toCpuLossyDropEnable = BIT2BOOL_MAC((value & 1));

    return GT_OK;
}

/**
* @internal cpssDxChPortPfcLossyDropConfigGet function
* @endinternal
*
* @brief   Get lossy drop configurations.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number.
*
* @param[out] lossyDropConfigPtr       - (pointer to) lossy drop configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The device provides a burst absortion system designed to sustain contain
*       the packets in case of an incast type traffic (many-to-one).
*       Two thresholds are used:
*       1. Lossy drop - packets packets with lossy TC are dropped in
*       congested port group.
*       2. PFC Xoff threshold - PFC is sent to all of the ports in the port group
*       with all timers set to 0xFFFF.
*
*/
GT_STATUS cpssDxChPortPfcLossyDropConfigGet
(
    IN  GT_U8     devNum,
    OUT CPSS_DXCH_PORT_PFC_LOSSY_DROP_CONFIG_STC *lossyDropConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcLossyDropConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, lossyDropConfigPtr));

    rc = internal_cpssDxChPortPfcLossyDropConfigGet(devNum, lossyDropConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, lossyDropConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcTerminateFramesEnableSet function
* @endinternal
*
* @brief   Enable/Disable the ingress control pipe to parse and terminate received
*         flow control frames (PRC/802.3x)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   -  termination of flow control frames
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note If the configuration is disabled, flow control frames are processed by
*       the control pipe as regular data frames (that is they are subject to
*       learning, forwarding, filtering and mirroring) but they are not
*       terminated and the PFC timers are not extracted.
*
*/
static GT_STATUS internal_cpssDxChPortPfcTerminateFramesEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_U32      regAddr;
    GT_U32      hwData;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfig;

    hwData = BOOL2BIT_MAC(enable);
    return prvCpssDrvHwPpSetRegField(devNum, regAddr, 29, 1, hwData);
}

/**
* @internal cpssDxChPortPfcTerminateFramesEnableSet function
* @endinternal
*
* @brief   Enable/Disable the ingress control pipe to parse and terminate received
*         flow control frames (PRC/802.3x)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   -  termination of flow control frames
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note If the configuration is disabled, flow control frames are processed by
*       the control pipe as regular data frames (that is they are subject to
*       learning, forwarding, filtering and mirroring) but they are not
*       terminated and the PFC timers are not extracted.
*
*/
GT_STATUS cpssDxChPortPfcTerminateFramesEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcTerminateFramesEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChPortPfcTerminateFramesEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcTerminateFramesEnableGet function
* @endinternal
*
* @brief   Get whether the ingress control pipe to parse and terminate received
*         flow control frames (PRC/802.3x) is enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) termination of flow control frames enabling
*                                      status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note If the configuration is disabled, flow control frames are processed by
*       the control pipe as regular data frames (that is they are subject to
*       learning, forwarding, filtering and mirroring) but they are not
*       terminated and the PFC timers are not extracted.
*
*/
static GT_STATUS internal_cpssDxChPortPfcTerminateFramesEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_U32      regAddr;
    GT_U32      hwData;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfig;

    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 29, 1, &hwData);
    if (rc == GT_OK)
    {
        *enablePtr = BIT2BOOL_MAC(hwData);
    }
    return rc;
}

/**
* @internal cpssDxChPortPfcTerminateFramesEnableGet function
* @endinternal
*
* @brief   Get whether the ingress control pipe to parse and terminate received
*         flow control frames (PRC/802.3x) is enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) termination of flow control frames enabling
*                                      status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note If the configuration is disabled, flow control frames are processed by
*       the control pipe as regular data frames (that is they are subject to
*       learning, forwarding, filtering and mirroring) but they are not
*       terminated and the PFC timers are not extracted.
*
*/
GT_STATUS cpssDxChPortPfcTerminateFramesEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcTerminateFramesEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChPortPfcTerminateFramesEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortUnknownMacControlFramesCmdSet function
* @endinternal
*
* @brief   Set the unknown MAC control frames command
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] command                  - the command. valid values:
*                                      CPSS_PACKET_CMD_FORWARD_E
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*                                      CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or command
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortUnknownMacControlFramesCmdSet
(
    IN GT_U8                    devNum,
    IN CPSS_PACKET_CMD_ENT      command
)
{
    GT_U32      regAddr;
    GT_U32      value;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,command);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfigExt;
    return prvCpssDrvHwPpSetRegField(devNum, regAddr, 14, 3, value);
}

/**
* @internal cpssDxChPortUnknownMacControlFramesCmdSet function
* @endinternal
*
* @brief   Set the unknown MAC control frames command
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] command                  - the command. valid values:
*                                      CPSS_PACKET_CMD_FORWARD_E
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*                                      CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or command
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortUnknownMacControlFramesCmdSet
(
    IN GT_U8                    devNum,
    IN CPSS_PACKET_CMD_ENT      command
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortUnknownMacControlFramesCmdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, command));

    rc = internal_cpssDxChPortUnknownMacControlFramesCmdSet(devNum, command);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, command));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortUnknownMacControlFramesCmdGet function
* @endinternal
*
* @brief   Get the unknown MAC control frames command
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] commandPtr               - (pointer to) the command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortUnknownMacControlFramesCmdGet
(
    IN  GT_U8                   devNum,
    OUT CPSS_PACKET_CMD_ENT     *commandPtr
)
{
    GT_U32      regAddr;
    GT_U32      value;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(commandPtr);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfigExt;

    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 14, 3, &value);
    if (rc == GT_OK)
    {
        PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(*commandPtr,value);
    }
    return rc;
}

/**
* @internal cpssDxChPortUnknownMacControlFramesCmdGet function
* @endinternal
*
* @brief   Get the unknown MAC control frames command
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] commandPtr               - (pointer to) the command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortUnknownMacControlFramesCmdGet
(
    IN  GT_U8                   devNum,
    OUT CPSS_PACKET_CMD_ENT     *commandPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortUnknownMacControlFramesCmdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, commandPtr));

    rc = internal_cpssDxChPortUnknownMacControlFramesCmdGet(devNum, commandPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, commandPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortFlowControlPacketsCntGet function
* @endinternal
*
* @brief   Get the number of received and dropped Flow Control packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] receivedCntPtr           - the number of received packets
* @param[out] droppedCntPtr            - the number of dropped packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counters are cleared on read.
*
*/
static GT_STATUS internal_cpssDxChPortFlowControlPacketsCntGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *receivedCntPtr,
    OUT GT_U32      *droppedCntPtr
)
{
    GT_U32      regAddr;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(receivedCntPtr);
    CPSS_NULL_PTR_CHECK_MAC(droppedCntPtr);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).receivedFlowCtrlPktsCntr;
    rc = prvCpssPortGroupsCounterSummary(devNum, regAddr, 0, 16, receivedCntPtr,NULL);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).droppedFlowCtrlPktsCntr;
    return prvCpssPortGroupsCounterSummary(devNum, regAddr, 0, 16, droppedCntPtr,NULL);
}

/**
* @internal cpssDxChPortFlowControlPacketsCntGet function
* @endinternal
*
* @brief   Get the number of received and dropped Flow Control packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] receivedCntPtr           - the number of received packets
* @param[out] droppedCntPtr            - the number of dropped packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counters are cleared on read.
*
*/
GT_STATUS cpssDxChPortFlowControlPacketsCntGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *receivedCntPtr,
    OUT GT_U32      *droppedCntPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortFlowControlPacketsCntGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, receivedCntPtr, droppedCntPtr));

    rc = internal_cpssDxChPortFlowControlPacketsCntGet(devNum, receivedCntPtr, droppedCntPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, receivedCntPtr, droppedCntPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcQueueCounterGet function
* @endinternal
*
* @brief   Get the number buffers/packets per proirity queue.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue on the port
*
* @param[out] cntPtr                   - (pointer to) number buffers/packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, tcQueue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Buffer or packet counting mode is set by cpssDxChPortPfcCountingModeSet.
*
*/
static GT_STATUS internal_cpssDxChPortPfcQueueCounterGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   tcQueue,
    OUT GT_U32  *cntPtr
)
{
    GT_U32      regAddr;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(cntPtr);

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tcQueue);

    if(GT_TRUE == PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        rc = prvCpssFalconTxqPfccGlobalTcCounterGet(devNum,
            PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC,tcQueue,cntPtr);
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCGroupOfPortsTCCntr[0][tcQueue];
        rc = prvCpssPortGroupsCounterSummary(devNum, regAddr, 0, 29, cntPtr, NULL);
     }

    return rc;
}

/**
* @internal cpssDxChPortPfcQueueCounterGet function
* @endinternal
*
* @brief   Get the number buffers/packets per proirity queue.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue on the port
*
* @param[out] cntPtr                   - (pointer to) number buffers/packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, tcQueue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Buffer or packet counting mode is set by cpssDxChPortPfcCountingModeSet.
*
*/
GT_STATUS cpssDxChPortPfcQueueCounterGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   tcQueue,
    OUT GT_U32  *cntPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcQueueCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tcQueue, cntPtr));

    rc = internal_cpssDxChPortPfcQueueCounterGet(devNum, tcQueue, cntPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tcQueue, cntPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcSourcePortToPfcCounterSet function
* @endinternal
*
* @brief   Set PFC mapping of a physical port to a PFC counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] pfcCounterNum            - PFC counter (APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - on wrong pfcCounterNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPfcSourcePortToPfcCounterSet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U32  pfcCounterNum
)
{
    GT_U32  regAddr;    /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    if (pfcCounterNum >= BIT_7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCSourcePortToPFCIndexMap[portNum];

    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 7, pfcCounterNum);
}

/**
* @internal cpssDxChPortPfcSourcePortToPfcCounterSet function
* @endinternal
*
* @brief   Set PFC mapping of a physical port to a PFC counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] pfcCounterNum            - PFC counter (APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - on wrong pfcCounterNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPfcSourcePortToPfcCounterSet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U32  pfcCounterNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcSourcePortToPfcCounterSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, pfcCounterNum));

    rc = internal_cpssDxChPortPfcSourcePortToPfcCounterSet(devNum, portNum, pfcCounterNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, pfcCounterNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcSourcePortToPfcCounterGet function
* @endinternal
*
* @brief   Get PFC mapping of a physical port to a PFC counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] pfcCounterNumPtr         (pointer to) PFC counter
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssDxChPortPfcSourcePortToPfcCounterGet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32  *pfcCounterNumPtr
)
{
    GT_U32  regAddr;    /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(pfcCounterNumPtr);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCSourcePortToPFCIndexMap[portNum];

    return prvCpssHwPpGetRegField(devNum, regAddr, 0, 7, pfcCounterNumPtr);
}

/**
* @internal cpssDxChPortPfcSourcePortToPfcCounterGet function
* @endinternal
*
* @brief   Get PFC mapping of a physical port to a PFC counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] pfcCounterNumPtr         (pointer to) PFC counter
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChPortPfcSourcePortToPfcCounterGet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32  *pfcCounterNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcSourcePortToPfcCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, pfcCounterNumPtr));

    rc = internal_cpssDxChPortPfcSourcePortToPfcCounterGet(devNum, portNum, pfcCounterNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, pfcCounterNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortPfcCounterGet function
* @endinternal
*
* @brief   Get PFC counter value per PFC counter and traffic class.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] pfcCounterNum            - PFC counter (APPLICABLE RANGES: 0..127)
*
* @param[out] pfcCounterValuePtr       - (pointer to) PFC counter value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - on wrong pfcCounterNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
*@note    In SIP6 pfcCounterNum is used as physical port number.
*@note    In case tcQueue equal 0xFF the counter value refer to all TC's consumed by port.
*/
static GT_STATUS internal_cpssDxChPortPfcCounterGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   tcQueue,
    IN  GT_U32  pfcCounterNum,
    OUT GT_U32  *pfcCounterValuePtr
)
{
    GT_STATUS rc;  /* return code */
    CPSS_DXCH_TABLE_ENT tableType;
    GT_U32  maxNum; /* maximal counter number */
    GT_U32  numOfBits; /* number of bits in the counter */
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;/*port mapping shadow*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(pfcCounterValuePtr);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE)
    {
        PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tcQueue);
    }
    else
    {   /*For SIP6  PRV_CPSS_DXCH_SIP6_TXQ_ALL_Q_MAC is valid value,it mean
                        we want to get counter for port not specific tc*/
        if(tcQueue!= PRV_CPSS_DXCH_SIP6_TXQ_ALL_Q_MAC)
        {
            PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tcQueue);
        }
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, pfcCounterNum/*physicalPortNumber*/, /*OUT*/&portMapShadowPtr);
        if (rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortPhysicalPortMapShadowDBGet failed for port %d",
                pfcCounterNum );
        }
        if(portMapShadowPtr->valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "portMapShadowPtr->valid == GT_FALSE for port %d",
                pfcCounterNum );
        }

        rc = prvCpssFalconTxqPfccPortTcCounterGet(devNum,PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC,tcQueue,
            portMapShadowPtr->portMap.rxDmaNum,pfcCounterValuePtr);

        if (rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqPfccCounterGet failed for port %d",
                pfcCounterNum );
        }

        return rc;
    }
    else if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
    {
        maxNum  = LLFC_NUM_PORTS_SIP_5_25_CNS;
        numOfBits = 21;
    }
    else if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        maxNum  = LLFC_NUM_PORTS_SIP_5_20_CNS;
        numOfBits = 21;
    }
    else
    {
        maxNum = 128;
        numOfBits = 29;
    }

    if (pfcCounterNum >= maxNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "pfcCounterNum[%d] is out of range [0..%d]",
            pfcCounterNum , maxNum-1);
    }

    tableType = CPSS_DXCH_SIP5_TABLE_PFC_LLFC_COUNTERS_E;

    rc = prvCpssDxChReadTableEntryField(devNum, tableType, ((pfcCounterNum<<3) + tcQueue), 0, 0, numOfBits, pfcCounterValuePtr);

    return rc;
}

/**
* @internal cpssDxChPortPfcCounterGet function
* @endinternal
*
* @brief   Get PFC counter value per PFC counter and traffic class.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] pfcCounterNum            - PFC counter (APPLICABLE RANGES: 0..127)
*
* @param[out] pfcCounterValuePtr       - (pointer to) PFC counter value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - on wrong pfcCounterNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
*@note    In SIP6 pfcCounterNum is used as physical port number.
*@note    In case tcQueue equal 0xFF the counter value refer to all TC's consumed by port.
*/
GT_STATUS cpssDxChPortPfcCounterGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   tcQueue,
    IN  GT_U32  pfcCounterNum,
    OUT GT_U32  *pfcCounterValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tcQueue, pfcCounterNum, pfcCounterValuePtr));

    rc = internal_cpssDxChPortPfcCounterGet(devNum, tcQueue, pfcCounterNum, pfcCounterValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tcQueue, pfcCounterNum, pfcCounterValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal internal_cpssDxChPortPfcXonMessageFilterEnableSet function
* @endinternal
*
* @brief   Enable/Disable PFC (Priority Flow Control) filtering of
*         continuous flow control XON messages to priority shaper.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE:  Enable  PFC XON Message Filter.
*                                      GT_FALSE: Disable PFC XON Message Filter.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or enable option
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Enable this feature in a system with periodic flow control XON messages.
*
*/
GT_STATUS internal_cpssDxChPortPfcXonMessageFilterEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_U32 regAddr;   /* register address */
    GT_U32 value;     /* register value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);


    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    value = BOOL2BIT_MAC(enable);
    regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).flowCtrlResponse.flowCtrlResponseConfig;

    return prvCpssHwPpSetRegField(devNum, regAddr, 2, 1, value);
}

/**
* @internal cpssDxChPortPfcXonMessageFilterEnableSet function
* @endinternal
*
* @brief   Enable/Disable PFC (Priority Flow Control) filtering of
*         continuous flow control XON messages to priority shaper.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE:  Enable  PFC XON Message Filter.
*                                      GT_FALSE: Disable PFC XON Message Filter.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or enable option
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Enable this feature in a system with periodic flow control XON messages.
*
*/
GT_STATUS cpssDxChPortPfcXonMessageFilterEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcXonMessageFilterEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChPortPfcXonMessageFilterEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcXonMessageFilterEnableGet function
* @endinternal
*
* @brief   Gets current status of the filtering of
*         continuous flow control XON messages to priority shaper.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) Enable  PFC XON Message Filter.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChPortPfcXonMessageFilterEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_U32 regAddr;   /* register address */
    GT_U32 value;     /* register value */
    GT_STATUS rc;     /* function return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);


    regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).flowCtrlResponse.flowCtrlResponseConfig;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 2, 1, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChPortPfcXonMessageFilterEnableGet function
* @endinternal
*
* @brief   Gets current status of the filtering of
*         continuous flow control XON messages to priority shaper.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) Enable  PFC XON Message Filter.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPfcXonMessageFilterEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcXonMessageFilterEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChPortPfcXonMessageFilterEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcDbaAvailableBuffSet function
* @endinternal
*
* @brief   Set amount of buffers available for dynamic allocation for PFC.
*
* @note   APPLICABLE DEVICES:      Aldrin2,Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      (APPLICABLE DEVICES Falcon)
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
* @param[in] availableBuff            - amount of buffers available for dynamic allocation for PFC.
*                                      (APPLICABLE RANGES: 0..0xFFFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, index
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPfcDbaAvailableBuffSet
(
    IN  GT_U8            devNum,
    IN  GT_DATA_PATH_BMP dataPathBmp,
    IN  GT_U32          availableBuff
)
{
    GT_U32                                        regAddr;    /* register address */
    GT_U32                                        ii,lDpSize;
    PRV_CPSS_DXCH_SIP6_TXQ_QFC_GLOBAL_PFC_CFG_STC cfg;
    GT_STATUS                                     rc;
    GT_U32                                        localDpId,tileId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
    CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
        CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_XCAT3_E);

     if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
     {
        lDpSize = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp;

        /* Shifting a number of bits greater than or
                   equal to the width of the promoted left operand is undefined behavior in C*/
        if(lDpSize<32)
        {
            if(dataPathBmp == CPSS_DATA_PATH_UNAWARE_MODE_CNS)
            {
               dataPathBmp = (1<<lDpSize)-1;
            }

            if(dataPathBmp>>lDpSize)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            if(dataPathBmp == CPSS_DATA_PATH_UNAWARE_MODE_CNS)
            {
                dataPathBmp=(GT_U32)-1;
            }
        }

        if(dataPathBmp == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }


        if (availableBuff >= 1<<TXQ_QFC_GLOBAL_PFC_CONF_PB_AVAILABLE_BUFFERS_FIELD_SIZE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }


        for(ii = 0;dataPathBmp&& (ii < lDpSize); ++ii,dataPathBmp>>=1)
        {
            if(dataPathBmp&1)
            {

              rc = prvCpssFalconGlobalDpToTileAndLocalDp(devNum,ii,&localDpId,&tileId);
              if (rc != GT_OK)
              {
                   return rc;
              }
              rc = prvCpssFalconTxqQfcGlobalPfcCfgGet(devNum,tileId,localDpId,&cfg);
              if(rc!=GT_OK)
              {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcGlobalPfcCfgGet failed for dp %d",localDpId);
              }

              if(availableBuff!=cfg.pbAvailableBuffers)
              {
                cfg.pbAvailableBuffers = availableBuff;
                rc = prvCpssFalconTxqQfcGlobalPfcCfgSet(devNum,tileId,localDpId,&cfg);

                if(rc!=GT_OK)
                {
                   CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcGlobalPfcCfgSet failed for dp %d",localDpId);
                }
              }
            }
        }
     }
     else
     {
        /* Set Aldrin2 CN Available Buffers for QCN DBA. */
        if (availableBuff >= BIT_24)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        /* <PFC Available Buffers> register */
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).dba.PFCAvailableBuffers;

        /* Set <pfc_available_buffers> field of */
        /* <PFC Available Buffers> register   */
        return prvCpssHwPpSetRegField(devNum, regAddr, 0, 24,
                                      availableBuff);
     }

     return GT_OK;
}

/**
* @internal cpssDxChPortPfcDbaAvailableBuffSet function
* @endinternal
*
* @brief   Set amount of buffers available for dynamic allocation for PFC.
*
* @note   APPLICABLE DEVICES:      Aldrin2,Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      (APPLICABLE DEVICES Falcon)
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
* @param[in] availableBuff            - amount of buffers available for dynamic allocation for PFC.
*                                      (APPLICABLE RANGES: 0..0xFFFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, index
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPfcDbaAvailableBuffSet
(
    IN  GT_U8            devNum,
    IN  GT_DATA_PATH_BMP dataPathBmp,
    IN  GT_U32           availableBuff
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcDbaAvailableBuffSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum,dataPathBmp, availableBuff));

    rc = internal_cpssDxChPortPfcDbaAvailableBuffSet(devNum,dataPathBmp,availableBuff);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, availableBuff));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChPortPfcDbaAvailableBuffGet function
 * @endinternal
 *
 * @brief   Get amount of buffers available for dynamic allocation for PFC.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2,Falcon.
 * @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
 *

 * @param[in] devNum                   - device number
 * @param[in] dataPathBmp              - bitmap of Data Paths
 *                                      (APPLICABLE DEVICES Falcon)
 *                                      NOTEs:
 *                                      1. for non multi data paths device this parameter is
 *                                      IGNORED.
 *                                      2. for multi data paths device:
 *                                      bitmap must be set with at least one bit representing
 *                                      valid data path(s). If a bit of non valid data path
 *                                      is set then function returns GT_BAD_PARAM.
 *                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
 *                                       3. read only from first data path of the bitmap.
 * @param[out] availableBuffPtr            - (pointer to)amount of buffers available for dynamic allocation for PFC.
 *                                      (APPLICABLE RANGES: 0..0xFFFFFF)

 * @retval GT_OK                    - on success
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
static GT_STATUS internal_cpssDxChPortPfcDbaAvailableBuffGet
(
 IN  GT_U8           devNum,
 IN  GT_DATA_PATH_BMP dataPathBmp,
 OUT GT_U32          *availableBuffPtr
)
{
  GT_STATUS   rc;         /* return code */
  GT_U32      regAddr;    /* register address */
  GT_U32      regValue;   /* register value */
  GT_U32                                        ii,lDpSize;
  PRV_CPSS_DXCH_SIP6_TXQ_QFC_GLOBAL_PFC_CFG_STC cfg;
  GT_U32                                        localDpId,tileId;


  PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
  PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
       CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
      CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_XCAT3_E);

  CPSS_NULL_PTR_CHECK_MAC(availableBuffPtr);

  if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
  {
     lDpSize = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp;

     /* Shifting a number of bits greater than or
                   equal to the width of the promoted left operand is undefined behavior in C*/
     if(lDpSize<32)
     {
         if(dataPathBmp == CPSS_DATA_PATH_UNAWARE_MODE_CNS)
         {
            dataPathBmp = (1<<lDpSize)-1;
         }

         if(dataPathBmp>>lDpSize)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
         }
     }
     else
     {
         if(dataPathBmp == CPSS_DATA_PATH_UNAWARE_MODE_CNS)
         {
             dataPathBmp=(GT_U32)-1;
         }
     }

     if(dataPathBmp == 0)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
     }

     for(ii = 0;dataPathBmp&& (ii < lDpSize); ++ii,dataPathBmp>>=1)
     {
         if(dataPathBmp&1)
         {
           rc = prvCpssFalconGlobalDpToTileAndLocalDp(devNum,ii,&localDpId,&tileId);
           if (rc != GT_OK)
           {
                return rc;
           }
           rc = prvCpssFalconTxqQfcGlobalPfcCfgGet(devNum,tileId,localDpId,&cfg);
           if(rc!=GT_OK)
           {
              CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcGlobalPfcCfgGet failed for dp %d",localDpId);
           }
           *availableBuffPtr = cfg.pbAvailableBuffers;
           break;
         }
     }
  }
  else
  {

      /* <PFC Available Buffers> register */
      regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).dba.PFCAvailableBuffers;

      /* Set <pfc_available_buffers> field of */
      /* <PFC Available Buffers> register   */
      rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 24, &regValue);
      if (rc != GT_OK)
      {
        return rc;
      }

      *availableBuffPtr = regValue;
   }

  return GT_OK;
}

/**
 * @internal cpssDxChPortPfcDbaAvailableBuffGet function
 * @endinternal
 *
 * @brief   Get amount of buffers available for dynamic allocation for PFC.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2,Falcon.
 * @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
 *

 * @param[in] devNum                   - device number
 * @param[in] dataPathBmp              - bitmap of Data Paths
 *                                      (APPLICABLE DEVICES Falcon)
 *                                      NOTEs:
 *                                      1. for non multi data paths device this parameter is
 *                                      IGNORED.
 *                                      2. for multi data paths device:
 *                                      bitmap must be set with at least one bit representing
 *                                      valid data path(s). If a bit of non valid data path
 *                                      is set then function returns GT_BAD_PARAM.
 *                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
                                         3. read only from first data path of the bitmap.
 * @param[out] availableBuffPtr            - (pointer to)amount of buffers available for dynamic allocation for PFC.
 *                                      (APPLICABLE RANGES: 0..0xFFFFFF)

 * @retval GT_OK                    - on success
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssDxChPortPfcDbaAvailableBuffGet
(
 IN  GT_U8           devNum,
 IN  GT_DATA_PATH_BMP dataPathBmp,
 OUT GT_U32          *availableBuffPtr
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcDbaAvailableBuffGet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId, devNum, availableBuffPtr));

  rc = internal_cpssDxChPortPfcDbaAvailableBuffGet(devNum,dataPathBmp, availableBuffPtr);

  CPSS_LOG_API_EXIT_MAC(funcId, rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, availableBuffPtr));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
 * @internal internal_cpssDxChPortPfcDbaModeEnableSet function
 * @endinternal
 *
 * @brief   Enable/disable XON/XOFF Thresholds Dynamic Buffers Allocation (DBA) for PFC.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon
 *
 * @param[in] devNum                   - device number
 * @param[in] enable                   - GT_TRUE -  DBA mode disabled
 *                                      GT_FALSE - DBA mode enabled.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
static GT_STATUS internal_cpssDxChPortPfcDbaModeEnableSet
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
  regValue = (enable == GT_TRUE) ? 0 : 1;

  /* <DBA Disable Register> register */
  regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).dba.PFCDynamicBufferAllocationDisable;

  /* Unset <pfc_dyn_buf_alloc_dis> field of  */
  /* <DBA Disable Register> register  */
  return prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, regValue);
}

/**
 * @internal cpssDxChPortPfcDbaModeEnableSet function
 * @endinternal
 *
 * @brief   Enable/disable XON/XOFF Thresholds Dynamic Buffers Allocation (DBA) for QCN.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
 *
 * @param[in] devNum                   - device number
 * @param[in] enable                   - GT_TRUE -  DBA mode disabled
 *                                      GT_FALSE - DBA mode enabled.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssDxChPortPfcDbaModeEnableSet
(
 IN  GT_U8           devNum,
 IN  GT_BOOL         enable
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcDbaModeEnableSet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

  rc = internal_cpssDxChPortPfcDbaModeEnableSet(devNum, enable);

  CPSS_LOG_API_EXIT_MAC(funcId, rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
 * @internal internal_cpssDxChPortPfcDbaModeEnableGet function
 * @endinternal
 *
 * @brief   Get state of Dynamic Buffers Allocation (DBA) for PFC.
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
static GT_STATUS internal_cpssDxChPortPfcDbaModeEnableGet
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

  /* <DBA Disable Register> register */
  regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).dba.PFCDynamicBufferAllocationDisable;

  /* Get <pfc_dyn_buf_alloc_dis> field of  */
  /* <DBA Disable> register  */
  rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, &regValue);
  if (rc != GT_OK)
  {
    return rc;
  }

  *enablePtr = regValue ? GT_FALSE : GT_TRUE;

  return GT_OK;
}

/**
 * @internal cpssDxChPortPfcDbaModeEnableGet function
 * @endinternal
 *
 * @brief   Get state of Dynamic Buffers Allocation (DBA) for PFC.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2.
 * @note   NOT APPLICABLE DEVICES:   xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
GT_STATUS cpssDxChPortPfcDbaModeEnableGet
(
 IN  GT_U8           devNum,
 OUT GT_BOOL         *enablePtr
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcDbaModeEnableGet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

  rc = internal_cpssDxChPortPfcDbaModeEnableGet(devNum, enablePtr);

  CPSS_LOG_API_EXIT_MAC(funcId, rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
* @internal  internal_cpssDxChPortPfcGlobalPbLimitSet  function
* @endinternal
*
* @brief   Global limit on the PB occupied buffers, once this threshold is crossed, PFC OFF will be sent to all port.tc.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:   xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable sending XOFF once PB limit is crossed
* @param[in] globalPbLimit         -   packet buffer limit(Valid if enable equal GT_TRUE,must be greater then 3071*number of tiles)
* @retval GT_OK                              - on success.
* @retval GT_BAD_PARAM             - wrong input parameter
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPortPfcGlobalPbLimitSet
(
     IN  GT_U8          devNum,
     IN  GT_BOOL        enable,
     IN  GT_U32         globalPbLimit
)
{

 GT_STATUS rc;
 GT_U32 numberOfTiles,i,j;

  PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
  PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
      CPSS_XCAT3_E | CPSS_LION2_E |
      CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_BOBCAT3_E |
      CPSS_AC3X_E|CPSS_ALDRIN2_E);

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

  if(globalPbLimit<=numberOfTiles*PFC_MIN_PB_TRESHOLD_VAL_CNS)
  {
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "globalPbLimit is too small %d",globalPbLimit);
  }

  for(i=0;i<numberOfTiles;i++)
  {
        for(j=0;j<MAX_DP_IN_TILE(devNum);j++)
        {

            rc = prvCpssFalconTxqQfcGlobalPfcCfgPbCongestionSet(devNum,i,j,enable);

            if(rc!=GT_OK)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcGlobalPbLimitSet fail");
            }

            if(enable == GT_TRUE)
            {
                rc = prvCpssFalconTxqQfcGlobalPbLimitSet(devNum,i,j,globalPbLimit);

                if(rc!=GT_OK)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcGlobalPbLimitSet fail");
                }
            }
        }
  }


  return GT_OK;
}

/**
* @internal  cpssDxChPortPfcGlobalPbLimitSet  function
* @endinternal
*
* @brief   Global limit on the PB occupied buffers, once this threshold is crossed, PFC OFF will be sent to all port.tc.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:   xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable sending XOFF once PB limit is crossed
* @param[in] globalPbLimit         -   packet buffer limit (Valid if enable equal GT_TRUE,must be greater then 512)
* @retval GT_OK                              - on success.
* @retval GT_BAD_PARAM             - wrong input parameter
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/

GT_STATUS cpssDxChPortPfcGlobalPbLimitSet
(
     IN  GT_U8     devNum,
     IN GT_BOOL    enable,
     IN  GT_U32    globalPbLimit
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcGlobalPbLimitSet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable,globalPbLimit));

  rc = internal_cpssDxChPortPfcGlobalPbLimitSet(devNum, enable,globalPbLimit);

  CPSS_LOG_API_EXIT_MAC(funcId, rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable,globalPbLimit));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}


/**
* @internal  internal_cpssDxChPortPfcGlobalPbLimitGet  function
* @endinternal
*
* @brief   Global limit on the PB occupied buffers, once this threshold is crossed, PFC OFF will be sent to all port.tc.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] enablePtr                   - (pointer to)enable sending XOFF once PB limit is crossed
* @param[in] globalPbLimitPtr         -   (pointer to)packet buffer limit
* @retval GT_OK                              - on success.
* @retval GT_BAD_PARAM             - wrong input parameter
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPortPfcGlobalPbLimitGet
(
     IN    GT_U8      devNum,
     OUT   GT_BOOL    *enablePtr,
     OUT   GT_U32     *globalPbLimitPtr
)
{

 GT_STATUS rc;
 PRV_CPSS_DXCH_SIP6_TXQ_QFC_GLOBAL_PFC_CFG_STC  qfcCfg;


  PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
  PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
      CPSS_XCAT3_E | CPSS_LION2_E |
      CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_BOBCAT3_E |
      CPSS_AC3X_E|CPSS_ALDRIN2_E);

   /*Since all QFCs are synced read only from TILE 0 QFC 0 */

    rc = prvCpssFalconTxqQfcGlobalPfcCfgGet(devNum,0,0,&qfcCfg);

    if(rc!=GT_OK)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcGlobalPfcCfgGet fail");
    }

     *enablePtr = qfcCfg.pbCongestionPfcEnable;


      rc = prvCpssFalconTxqQfcGlobalPbLimitGet(devNum,0,0,globalPbLimitPtr);

       if(rc!=GT_OK)
       {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcGlobalPbLimitSet fail");
       }

  return GT_OK;
}


/**
* @internal  cpssDxChPortPfcGlobalPbLimitSet  function
* @endinternal
*
* @brief   Global limit on the PB occupied buffers, once this threshold is crossed, PFC OFF will be sent to all port.tc.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] enablePtr                   - (pointer to)enable sending XOFF once PB limit is crossed
* @param[in] globalPbLimitPtr         -   (pointer to)packet buffer limit
* @retval GT_OK                              - on success.
* @retval GT_BAD_PARAM             - wrong input parameter
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPortPfcGlobalPbLimitGet
(
     IN    GT_U8      devNum,
     OUT   GT_BOOL    *enablePtr,
     OUT   GT_U32     *globalPbLimitPtr
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcGlobalPbLimitGet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr,globalPbLimitPtr));

  rc = internal_cpssDxChPortPfcGlobalPbLimitGet(devNum, enablePtr,globalPbLimitPtr);

  CPSS_LOG_API_EXIT_MAC(funcId, rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr,globalPbLimitPtr));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
* @internal cpssDxChPortPfcGenerationEnableSet function
* @endinternal
*
* @brief   Global Enable/Disable PFC (Priority Flow Control) Generation  functionality.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] enable                - PFC enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             -wrong input parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPortPfcGenerationEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
        GT_STATUS rc ;
        PRV_CPSS_PFCC_TILE_INIT_STC  pfccDb;

       PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
       PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
       CPSS_XCAT3_E | CPSS_LION_E |  CPSS_LION2_E |
       CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_BOBCAT3_E |
       CPSS_AC3X_E|CPSS_ALDRIN2_E);

        rc = prvCpssFalconTxqPfccUnitCfgGet(devNum,PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC,&pfccDb);

        if(rc!=GT_OK)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcGlobalPfcCfgGet fail");
        }

        /*Note that at QFC level PFC generation is always enabled ,control PFC generation only at PFCC*/

        if(pfccDb.pfccEnable != enable)
        {
                pfccDb.pfccEnable = enable;

                 rc = prvCpssFalconTxqPfccUnitCfgSet(devNum,PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC,&pfccDb);

                if(rc !=GT_OK)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcGlobalPfcCfgSet fail");
                }
        }

        return GT_OK;
}

/**
* @internal cpssDxChPortPfcGenerationEnableSet function
* @endinternal
*
* @brief   Global Enable/Disable PFC (Priority Flow Control) Generation  functionality.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] enable                - PFC enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             -wrong input parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPortPfcGenerationEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcGenerationEnableSet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

  rc = internal_cpssDxChPortPfcGenerationEnableSet(devNum, enable);

  CPSS_LOG_API_EXIT_MAC(funcId, rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
* @internal cpssDxChPortPfcGenerationEnableGet function
* @endinternal
*
* @brief    Enable/Disable PFC (Priority Flow Control) Generation  functionality.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] enablePtr                -(pointer to) PFC enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             -wrong input parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPortPfcGenerationEnableGet
(
    IN GT_U8     devNum,
    OUT GT_BOOL  *enablePtr
)
{
        GT_STATUS rc ;
        PRV_CPSS_PFCC_TILE_INIT_STC  pfccDb;

       PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
       PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
       CPSS_XCAT3_E |  CPSS_LION2_E |
       CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_BOBCAT3_E |
       CPSS_AC3X_E|CPSS_ALDRIN2_E);

        rc = prvCpssFalconTxqPfccUnitCfgGet(devNum,PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC,&pfccDb);

        if(rc!=GT_OK)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcGlobalPfcCfgGet fail");
        }

        *enablePtr = pfccDb.pfccEnable;

        return GT_OK;
}



/**
* @internal cpssDxChPortPfcGenerationEnableGet function
* @endinternal
*
* @brief   Get enable/disable PFC (Priority Flow Control) Generation  functionality.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] enablePtr                -(pointer to) PFC enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             -wrong input parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPortPfcGenerationEnableGet
(
    IN GT_U8     devNum,
    OUT GT_BOOL  *enablePtr
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcGenerationEnableGet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

  rc = internal_cpssDxChPortPfcGenerationEnableGet(devNum, enablePtr);

  CPSS_LOG_API_EXIT_MAC(funcId, rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
* @internal cpssDxChPortPfcGlobalTcThresholdSet function
* @endinternal
*
* @brief   Set global TC enable and threshold
*  When enable, once Global TC counter is above the calculated threshold,
*  PFC OFF message is sent to all ports, for the specific TC.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             - device number.
* @param[in] tc                        - Traffic class[0..7]
* @param[in] enable               - Global TC PFC enable option.
* @param[in] thresholdCfgPtr - (pointer to) threshold struct
* @param[in] hysteresisCfgPtr - (pointer to) hysteresis struct
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             -wrong input parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPortPfcGlobalTcThresholdSet
(
     IN  GT_U8                        devNum,
     IN  GT_U32                       tc,
     IN  GT_BOOL                       enable,
     IN  CPSS_DXCH_PFC_THRESHOLD_STC   *thresholdCfgPtr,
     IN CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
)
{   GT_STATUS                       rc;
    GT_U32                          globalTcIndex;/*start index of global tc in PFCC table*/
    PRV_CPSS_PFCC_CFG_ENTRY_STC     entry;

     PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
     PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
       CPSS_XCAT3_E |  CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
       CPSS_ALDRIN_E | CPSS_BOBCAT3_E |CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);
    CPSS_NULL_PTR_CHECK_MAC(thresholdCfgPtr);
    CPSS_NULL_PTR_CHECK_MAC(hysteresisCfgPtr);

    /*configure QFC*/
    rc = prvCpssFalconTxqQfcGlobalTcPfcThresholdSet(devNum,tc,enable,thresholdCfgPtr,hysteresisCfgPtr);

    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcGlobalTcPfcThresholdSet failed for tc %d",tc);
    }

    /*configure PFFC*/
    globalTcIndex = (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp)*CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC;

    if(enable == GT_TRUE)
    {
      entry.entryType = PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC;
      entry.globalTc = tc;
    }
    else /*delete entry by setting to bubble of size 1*/
    {
      entry.entryType = PRV_CPSS_PFCC_CFG_ENTRY_TYPE_BUBBLE;
      entry.numberOfBubbles = 1;
    }

    rc = prvCpssFalconTxqPfccCfgTableEntrySet(devNum,PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC,
            globalTcIndex+tc,&entry);

    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqPfccCfgTableEntrySet failed for tc %d",tc);
    }

    return rc;
}

/**
* @internal internal_cpssDxChPortPfcGlobalTcThresholdSet function
* @endinternal
*
* @brief   Set global TC enable and threshold
*  When enable, once Global TC counter is above the calculated threshold,
*  PFC OFF message is sent to all ports, for the specific TC.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             - device number.
* @param[in] tc                        - Traffic class[0..7]
* @param[in] enable               - Global TC PFC enable option.
* @param[in] thresholdCfgPtr - (pointer to) threshold struct
* @param[in] hysteresisCfgPtr - (pointer to) hysteresis struct
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             -wrong input parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPortPfcGlobalTcThresholdSet
(
     IN  GT_U8                              devNum,
     IN  GT_U32                             tc,
     IN GT_BOOL                             enable,
     IN CPSS_DXCH_PFC_THRESHOLD_STC         *thresholdCfgPtr,
     IN CPSS_DXCH_PFC_HYSTERESIS_CONF_STC   *hysteresisCfgPtr
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcGlobalTcThresholdSet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId,devNum,tc,enable,thresholdCfgPtr,hysteresisCfgPtr));

  rc = internal_cpssDxChPortPfcGlobalTcThresholdSet(devNum,tc,enable,thresholdCfgPtr,hysteresisCfgPtr);

  CPSS_LOG_API_EXIT_MAC(funcId,rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId,rc,devNum,tc,enable,thresholdCfgPtr));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
* @internal cpssDxChPortPfcGlobalTcThresholdGet function
* @endinternal
*
* @brief   Get global TC enable and threshold .
*  When enable, once Global TC counter is above the calculated threshold,
*  PFC OFF message is sent to all ports, for the specific TC.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             - device number.
* @param[in] tc                        - Traffic class[0..7]
* @param[out] enable                - (pointer to) Global TC PFC enable option.
* @param[out] thresholdCfgPtr - (pointer to) threshold struct
* @param[out] hysteresisCfgPtr - (pointer to) hysteresis struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             -wrong input parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPortPfcGlobalTcThresholdGet
(
     IN  GT_U8                         devNum,
     IN  GT_U32                        tc,
     OUT GT_BOOL                       *enablePtr,
     OUT CPSS_DXCH_PFC_THRESHOLD_STC   *thresholdCfgPtr,
     OUT CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
)
{
    GT_STATUS                       rc;

     PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
     PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
       CPSS_XCAT3_E |  CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
       CPSS_ALDRIN_E | CPSS_BOBCAT3_E |CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);
    CPSS_NULL_PTR_CHECK_MAC(thresholdCfgPtr);
    CPSS_NULL_PTR_CHECK_MAC(hysteresisCfgPtr);

    /*read from QFC 0  only*/
    rc = prvCpssFalconTxqQfcGlobalTcPfcThresholdGet(devNum,tc,enablePtr,thresholdCfgPtr,hysteresisCfgPtr);

    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcGlobalTcPfcThresholdGet failed for tc %d",tc);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortPfcGlobalTcThresholdGet function
* @endinternal
*
* @brief   Get global TC enable and threshold .
*  When enable, once Global TC counter is above the calculated threshold,
*  PFC OFF message is sent to all ports, for the specific TC.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             -    device number.
* @param[in] tc                        -    Traffic class[0..7]
* @param[out] enable                - (pointer to) Global TC PFC enable option.
* @param[out] thresholdCfgPtr - (pointer to) threshold struct
* @param[in] hysteresisCfgPtr - (pointer to) hysteresis struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             -wrong input parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPortPfcGlobalTcThresholdGet
(
     IN  GT_U8                         devNum,
     IN  GT_U32                        tc,
     OUT GT_BOOL                       *enablePtr,
     OUT CPSS_DXCH_PFC_THRESHOLD_STC   *thresholdCfgPtr,
     OUT CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcGlobalTcThresholdGet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId,devNum,tc,enablePtr,thresholdCfgPtr,hysteresisCfgPtr));

  rc = internal_cpssDxChPortPfcGlobalTcThresholdGet(devNum,tc,enablePtr,thresholdCfgPtr,hysteresisCfgPtr);

  CPSS_LOG_API_EXIT_MAC(funcId,rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId,rc,devNum,tc,enablePtr,thresholdCfgPtr,hysteresisCfgPtr));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
* @internal internal_cpssDxChPortPfcPortThresholdSet function
* @endinternal
*
* @brief   Sets  PFC OFF port limit. When Flow Control is enabled, PFC-OFF packet sent
*         when the number of buffers for this port is more than thresholdCfg limit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[out] enable                -  (pointer to) Global port PFC enable option.
* @param[out] thresholdCfgPtr - (pointer to) threshold struct
* @param[in] hysteresisCfgPtr - (pointer to) hysteresis struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
static GT_STATUS internal_cpssDxChPortPfcPortThresholdSet
(
     IN GT_U8                              devNum,
     IN GT_PHYSICAL_PORT_NUM               portNum,
     IN GT_BOOL                            enable,
     IN CPSS_DXCH_PFC_THRESHOLD_STC       *thresholdCfgPtr,
     IN CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
)
{
    GT_STATUS                       rc;
    GT_U32                          tileNum,dpNum,localdpPortNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
       CPSS_XCAT3_E |  CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
       CPSS_ALDRIN_E | CPSS_BOBCAT3_E |CPSS_AC3X_E|CPSS_ALDRIN2_E);

    /*Set PFCC*/

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localdpPortNum,NULL);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }

    rc = prvCpssFalconTxqPffcTableSyncSet(devNum,PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC,dpNum,localdpPortNum,0xFF,enable);
    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqPffcTableSyncSet failed for portNum %d",portNum);
    }
    /*Set QFC*/

    rc = prvCpssFalconTxqQfcPortPfcThresholdSet(devNum,tileNum,dpNum,localdpPortNum,enable,thresholdCfgPtr,hysteresisCfgPtr);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqQfcPortPfcThresholdSet  failed for portNum  %d  ",portNum);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortPfcPortThresholdSet function
* @endinternal
*
* @brief   Sets  PFC OFF port limit. When Flow Control is enabled, PFC-OFF packet sent
*         when the number of buffers for this port is more than thresholdCfg limit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -       physical device number
* @param[in] portNum                        - physical port number
* @param[in] enable                -          Global port PFC enable option.
* @param[in] thresholdCfgPtr -        (pointer to) threshold struct
* @param[in] hysteresisCfgPtr - (pointer to) hysteresis struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcPortThresholdSet
(
     IN GT_U8                              devNum,
     IN GT_PHYSICAL_PORT_NUM               portNum,
     IN GT_BOOL                            enable,
     IN CPSS_DXCH_PFC_THRESHOLD_STC       *thresholdCfgPtr,
     IN CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcPortThresholdSet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId,devNum,portNum,enable,thresholdCfgPtr,hysteresisCfgPtr));

  rc = internal_cpssDxChPortPfcPortThresholdSet(devNum,portNum,enable,thresholdCfgPtr,hysteresisCfgPtr);

  CPSS_LOG_API_EXIT_MAC(funcId,rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId,rc,devNum,portNum,enable,thresholdCfgPtr,hysteresisCfgPtr));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}


/**
* @internal internal_cpssDxChPortPfcPortThresholdGet function
* @endinternal
*
* @brief   Gets  PFC OFF port limit. When Flow Control is enabled, PFC-OFF packet sent
*         when the number of buffers for this port is more than thresholdCfg limit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -       physical device number
* @param[in] portNum                        - physical port number
* @param[out] enablePtr                -    (pointer to) Global port PFC enable option.
* @param[out] thresholdCfgPtr -        (pointer to) threshold struct
* @param[out] hysteresisCfgPtr - (pointer to) hysteresis struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
static GT_STATUS internal_cpssDxChPortPfcPortThresholdGet
(
     IN GT_U8                              devNum,
     IN GT_PHYSICAL_PORT_NUM               portNum,
     IN GT_BOOL                            *enablePtr,
     OUT CPSS_DXCH_PFC_THRESHOLD_STC       *thresholdCfgPtr,
     OUT CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
)
{
    GT_STATUS                       rc;
    GT_U32                          tileNum,dpNum,localdpPortNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
       CPSS_XCAT3_E |  CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
       CPSS_ALDRIN_E | CPSS_BOBCAT3_E |CPSS_AC3X_E|CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(thresholdCfgPtr);
    CPSS_NULL_PTR_CHECK_MAC(hysteresisCfgPtr);

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localdpPortNum,NULL);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }

    /*Get from QFC*/

    rc = prvCpssFalconTxqQfcPortPfcThresholdGet(devNum,tileNum,dpNum,localdpPortNum,enablePtr,thresholdCfgPtr,hysteresisCfgPtr);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqQfcPortPfcThresholdSet  failed for portNum  %d  ",portNum);
    }

    return GT_OK;
}


/**
* @internal internal_cpssDxChPortPfcPortThresholdGet function
* @endinternal
*
* @brief   Gets  PFC OFF port limit. When Flow Control is enabled, PFC-OFF packet sent
*         when the number of buffers for this port is more than thresholdCfg limit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -       physical device number
* @param[in] portNum                        - physical port number
* @param[out] enablePtr                -    (pointer to) Global port PFC enable option.
* @param[out] thresholdCfgPtr -        (pointer to) threshold struct
* @param[out] hysteresisCfgPtr - (pointer to) hysteresis struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcPortThresholdGet
(
     IN GT_U8                              devNum,
     IN GT_PHYSICAL_PORT_NUM               portNum,
     IN GT_BOOL                            *enablePtr,
     OUT CPSS_DXCH_PFC_THRESHOLD_STC       *thresholdCfgPtr,
     OUT CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcPortThresholdGet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId,devNum,portNum,enablePtr,thresholdCfgPtr,hysteresisCfgPtr));

  rc = internal_cpssDxChPortPfcPortThresholdGet(devNum,portNum,enablePtr,thresholdCfgPtr,hysteresisCfgPtr);

  CPSS_LOG_API_EXIT_MAC(funcId,rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId,rc,devNum,portNum,enablePtr,thresholdCfgPtr,hysteresisCfgPtr));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
* @internal internal_cpssDxChPortPfcPortTcThresholdSet function
* @endinternal
*
* @brief   Sets  PFC OFF port/tc  limit. When Flow Control is enabled, PFC-OFF packet sent
*         when the number of buffers for this port/TC  is more than thresholdCfg limit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] tc                        -      Traffic class[0..7]
* @param[in] enable                       port/tc PFC enable option.
* @param[in] thresholdCfgPtr - (pointer to) threshold struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
static GT_STATUS internal_cpssDxChPortPfcPortTcThresholdSet
(
     IN  GT_U8                              devNum,
     IN  GT_PHYSICAL_PORT_NUM               portNum,
     IN  GT_U8                              tc,
     IN  GT_BOOL                            enable,
     IN  CPSS_DXCH_PFC_THRESHOLD_STC        *thresholdCfgPtr
)
{
    GT_STATUS                       rc;
    GT_U32                          tileNum,dpNum,localdpPortNum,index;
    PRV_CPSS_PFCC_CFG_ENTRY_STC     entry;
    GT_BOOL                         pfccSetRequired;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
       CPSS_XCAT3_E |  CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
       CPSS_ALDRIN_E | CPSS_BOBCAT3_E |CPSS_AC3X_E|CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(thresholdCfgPtr);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);

    pfccSetRequired=GT_FALSE;

    /*Handle  PFCC*/

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localdpPortNum,NULL);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }

    index= (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp)*localdpPortNum+dpNum+tileNum*(MAX_DP_IN_TILE(devNum));

   /*Check if the PFCC entry exist already*/
    rc = prvCpssFalconTxqPfccCfgTableEntryGet(devNum,PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC,index,&entry);
    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqPfccCfgTableEntryGet failed for index %d",index);
    }

    if(entry.entryType == PRV_CPSS_PFCC_CFG_ENTRY_TYPE_PORT)
    {
        /*Entry exist .Set PFCC only if bit is not set*/
        if(((entry.tcBitVecEn)&(1<<tc))== 0)
        {
            pfccSetRequired = GT_TRUE;
            entry.tcBitVecEn|=(1<<tc);
        }
    }
    else
    {
        pfccSetRequired = GT_TRUE;
        entry.tcBitVecEn=(1<<tc);
    }

    if(pfccSetRequired == GT_TRUE)
    {
        rc = prvCpssFalconTxqPffcTableSyncSet(devNum,PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC,dpNum,localdpPortNum,entry.tcBitVecEn,enable);
        if(rc!=GT_OK)
        {
          CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqPffcTableSyncSet failed for portNum %d",portNum);
        }
    }
    /*Set QFC*/
    rc = prvCpssFalconTxqQfcPortTcPfcThresholdSet(devNum,tileNum,dpNum,localdpPortNum,tc,enable,thresholdCfgPtr);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqQfcPortTcPfcThresholdSet  failed for portNum  %d  ",portNum);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortPfcPortTcThresholdSet function
* @endinternal
*
* @brief   Sets  PFC OFF port/tc limit. When Flow Control is enabled, PFC-OFF packet sent
*         when the number of buffers for this port/TC  is more than thresholdCfg limit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] tc                        -      Traffic class[0..7]
* @param[in] enable                       port/tc PFC enable option.
* @param[in] thresholdCfgPtr - (pointer to) threshold struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcPortTcThresholdSet
(
     IN  GT_U8                              devNum,
     IN  GT_PHYSICAL_PORT_NUM               portNum,
     IN  GT_U8                              tc,
     IN  GT_BOOL                            enable,
     IN  CPSS_DXCH_PFC_THRESHOLD_STC        *thresholdCfgPtr
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcPortTcThresholdSet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId,devNum,portNum,enable,tc,thresholdCfgPtr));

  rc = internal_cpssDxChPortPfcPortTcThresholdSet(devNum,portNum,tc,enable,thresholdCfgPtr);

  CPSS_LOG_API_EXIT_MAC(funcId,rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId,rc,devNum,portNum,tc,enable,thresholdCfgPtr));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
* @internal internal_cpssDxChPortPfcPortTcThresholdGet function
* @endinternal
*
* @brief   Gets  PFC OFF port/tc limit. When Flow Control is enabled, PFC-OFF packet sent
*         when the number of buffers for this port/TC  is more than thresholdCfg limit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] tc                        -      Traffic class[0..7]
* @param[in] enablePtr                       (pointer to) port/tc PFC enable option.
* @param[in] thresholdCfgPtr - (pointer to) threshold struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
static GT_STATUS internal_cpssDxChPortPfcPortTcThresholdGet
(
     IN GT_U8                          devNum,
     IN GT_PHYSICAL_PORT_NUM           portNum,
     IN GT_U8                          tc,
     IN GT_BOOL                        *enablePtr,
     IN CPSS_DXCH_PFC_THRESHOLD_STC    *thresholdCfgPtr
)
{
    GT_STATUS                       rc;
    GT_U32                          tileNum,dpNum,localdpPortNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
       CPSS_XCAT3_E |  CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
       CPSS_ALDRIN_E | CPSS_BOBCAT3_E |CPSS_AC3X_E|CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(thresholdCfgPtr);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localdpPortNum,NULL);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }

    /*Get from QFC*/

    rc = prvCpssFalconTxqQfcPortTcPfcThresholdGet(devNum,tileNum,dpNum,localdpPortNum,tc,enablePtr,thresholdCfgPtr);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqQfcPortTcPfcThresholdGet  failed for portNum  %d  ",portNum);
    }

    return GT_OK;
}


/**
* @internal cpssDxChPortPfcPortTcThresholdGet function
* @endinternal
*
* @brief   Gets  PFC OFF port/tc limit. When Flow Control is enabled, PFC-OFF packet sent
*         when the number of buffers for this port/TC  is more than thresholdCfg limit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] tc                        -      Traffic class[0..7]
* @param[in] enablePtr                       (pointer to) port/tc PFC enable option.
* @param[in] thresholdCfgPtr - (pointer to) threshold struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcPortTcThresholdGet
(
     IN GT_U8                          devNum,
     IN GT_PHYSICAL_PORT_NUM           portNum,
     IN  GT_U8                         tc,
     IN GT_BOOL                        *enablePtr,
     IN CPSS_DXCH_PFC_THRESHOLD_STC    *thresholdCfgPtr
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcPortTcThresholdGet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId,devNum,portNum,enablePtr,thresholdCfgPtr));

  rc = internal_cpssDxChPortPfcPortTcThresholdGet(devNum,portNum,tc,enablePtr,thresholdCfgPtr);

  CPSS_LOG_API_EXIT_MAC(funcId,rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId,rc,devNum,portNum,enablePtr,tc,thresholdCfgPtr));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
* @internal internal_cpssDxChPortPfcResponceEnableSet function
* @endinternal
*
* @brief   Enable /Disable  PFC response per port/TC.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] tc                        -      Traffic class[0..7]
* @param[in] enable                        port/tc PFC responce enable option.
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
static GT_STATUS internal_cpssDxChPortPfcResponceEnableSet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_U32                  tc,
    IN GT_BOOL                 enable
)
{

    GT_STATUS                       rc;
    GT_U32                          tileNum,dpNum,localdpPortNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E |  CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
        CPSS_ALDRIN_E | CPSS_BOBCAT3_E |CPSS_AC3X_E|CPSS_ALDRIN2_E);

   /*TC validity is checked inside prvCpssFalconTxqSdqPfcControlEnableSet*/

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localdpPortNum,&mappingType);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }

    if(mappingType!=CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "PFC responce configured not on local port");
    }

    rc = prvCpssFalconTxqSdqPfcControlEnableSet(devNum,tileNum,dpNum,localdpPortNum,tc,enable);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqPfcControlEnableSet  failed for portNum  %d  ",portNum);
    }

    return rc;

}

/**
* @internal cpssDxChPortPfcResponceEnableSet function
* @endinternal
*
* @brief   Enable /Disable  PFC response per port/TC.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] tc                        -      Traffic class[0..7]
* @param[in] enable                        port/tc PFC responce enable option.
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcResponceEnableSet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_U32                  tc,
    IN GT_BOOL                 enable
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcResponceEnableSet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId,devNum,portNum,tc,enable));

  rc = internal_cpssDxChPortPfcResponceEnableSet(devNum,portNum,tc,enable);

  CPSS_LOG_API_EXIT_MAC(funcId,rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId,devNum,portNum,tc,enable));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
* @internal internal_cpssDxChPortPfcResponceEnableGet function
* @endinternal
*
* @brief   Get enable /Disable  PFC response status per port/TC.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] tc                        -      Traffic class[0..7]
* @param[in] enablePtr                   (pointer to)port/tc PFC responce enable option.
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
static GT_STATUS internal_cpssDxChPortPfcResponceEnableGet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_U32                  tc,
    IN GT_BOOL                 *enablePtr
)
{

    GT_STATUS                       rc;
    GT_U32                          tileNum,dpNum,localdpPortNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E |  CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
        CPSS_ALDRIN_E | CPSS_BOBCAT3_E |CPSS_AC3X_E|CPSS_ALDRIN2_E);

   /*TC validity is checked inside prvCpssFalconTxqSdqPfcControlEnableSet*/

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localdpPortNum,&mappingType);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }

    if(mappingType!=CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "PFC responce configured not on local port");
    }

    rc = prvCpssFalconTxqSdqPfcControlEnableGet(devNum,tileNum,dpNum,localdpPortNum,tc,enablePtr);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqPfcControlEnableGet  failed for portNum  %d  ",portNum);
    }

    return rc;

}

/**
* @internal internal_cpssDxChPortPfcResponceEnableGet function
* @endinternal
*
* @brief   Get enable /Disable  PFC response status per port/TC.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] tc                        -      Traffic class[0..7]
* @param[in] enablePtr                   (pointer to)port/tc PFC responce enable option.
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcResponceEnableGet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_U32                  tc,
    IN GT_BOOL                 *enablePtr
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcResponceEnableGet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId,devNum,portNum,tc,enablePtr));

  rc = internal_cpssDxChPortPfcResponceEnableGet(devNum,portNum,tc,enablePtr);

  CPSS_LOG_API_EXIT_MAC(funcId,rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId,devNum,portNum,tc,enablePtr));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
* @internal internal_cpssDxChPortPfcTcToQueueMapSet function
* @endinternal
*
* @brief   Sets PFC TC to  queue map.Meaning this table define which Q should be paused on reception of
*               perticular TC
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] tcForPfcResponce                       -      Traffic class[0..15]
* @param[in] queue                                              queue number [0..15]
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
static GT_STATUS internal_cpssDxChPortPfcTcToQueueMapSet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_U32                  tcForPfcResponce,
    IN GT_U32                  queue
)
{

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E |  CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
        CPSS_ALDRIN_E | CPSS_BOBCAT3_E |CPSS_AC3X_E|CPSS_ALDRIN2_E);

    return prvCpssFalconTxqUtilsQueueTcSet(devNum,portNum,queue,tcForPfcResponce);
}

/**
* @internal  cpssDxChPortPfcTcToQueueMapSet function
* @endinternal
*
* @brief   Sets PFC TC to  queue map.Meaning this table define which Q should be paused on reception of
*               perticular TC
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] tcForPfcResponce                       -      Traffic class[0..15]
* @param[in] queue                                              queue number [0..15]
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcTcToQueueMapSet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_U32                  tcForPfcResponce,
    IN GT_U32                  queue
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcTcToQueueMapSet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId,devNum,portNum,tcForPfcResponce,queue));

  rc = internal_cpssDxChPortPfcTcToQueueMapSet(devNum,portNum,tcForPfcResponce,queue);

  CPSS_LOG_API_EXIT_MAC(funcId,rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId,devNum,portNum,tcForPfcResponce,queue));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
* @internal internal_cpssDxChPortPfcTcToQueueMapGet function
* @endinternal
*
* @brief   Gets PFC TC to  queue map.Meaning this table define which Q should be paused on reception of
*               perticular TC
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] portNum                                         physical port number
* @param[in] queue                                              queue number [0..15]
* @param[out] tcForPfcResponce                      (pointer to)Traffic class[0..15]
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
static GT_STATUS internal_cpssDxChPortPfcTcToQueueMapGet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_U32                  queue,
    IN GT_U32                  *tcForPfcResponcePtr
)
{

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E |  CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
        CPSS_ALDRIN_E | CPSS_BOBCAT3_E |CPSS_AC3X_E|CPSS_ALDRIN2_E);

    return prvCpssFalconTxqUtilsQueueTcGet(devNum,portNum,queue,tcForPfcResponcePtr);
}

/**
* @internal  cpssDxChPortPfcTcToQueueMapGet function
* @endinternal
*
* @brief   Gets PFC TC to  queue map.Meaning this table define which Q should be paused on reception of
*               perticular TC
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] portNum                                         physical port number
* @param[in] queue                                              queue number [0..15]
* @param[out] tcForPfcResponce                      (pointer to)Traffic class[0..15]
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcTcToQueueMapGet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_U32                  queue,
    IN GT_U32                  *tcForPfcResponcePtr
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcTcToQueueMapGet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId,devNum,portNum,queue,tcForPfcResponcePtr));

  rc = internal_cpssDxChPortPfcTcToQueueMapGet(devNum,portNum,queue,tcForPfcResponcePtr);

  CPSS_LOG_API_EXIT_MAC(funcId,rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId,devNum,portNum,queue,tcForPfcResponcePtr));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}


static GT_STATUS internal_cpssDxChPortPfcReponceModeSet
(
    IN  GT_U8                                devNum,
    IN  GT_DATA_PATH_BMP                     dataPathBmp,
    IN  CPSS_DXCH_PORT_PFC_RESPONCE_MODE_ENT mode
)
{
    GT_STATUS rc;
    GT_U32    lDpSize,ii;
    GT_U32    localDpId,tileId;
    GT_U32    macNum;

    lDpSize = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp;

    /* Shifting a number of bits greater than or
                   equal to the width of the promoted left operand is undefined behavior in C*/
    if(lDpSize<32)
    {
        if(dataPathBmp == CPSS_DATA_PATH_UNAWARE_MODE_CNS)
        {
           dataPathBmp = (1<<lDpSize)-1;
        }

        if(dataPathBmp>>lDpSize)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        if(dataPathBmp == CPSS_DATA_PATH_UNAWARE_MODE_CNS)
        {
            dataPathBmp=(GT_U32)-1;
        }
    }

    if(dataPathBmp == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for(ii = 0;dataPathBmp&& (ii < lDpSize); ++ii,dataPathBmp>>=1)
    {
        if(dataPathBmp&1)
        {

          rc = prvCpssFalconGlobalDpToTileAndLocalDp(devNum,ii,&localDpId,&tileId);
          if (rc != GT_OK)
          {
               return rc;
          }
          /*Assumption :
                        1. Port 0 is mapped
                        2.For port that has MAC ,MAC equal DMA
                      */
          rc = prvCpssFalconTxqUtilsFalconTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvert(devNum,tileId,localDpId,0,&macNum);
          if (rc != GT_OK)
          {
               return rc;
          }

         rc = prvCpssDxChPortSip6PfcConfigFcuControlRxTypeSet(devNum,macNum,mode==CPSS_DXCH_PORT_PFC_RESPONCE_MODE_16_TC_E);

         if(rc!=GT_OK)
         {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortSip6PfcConfigFcuControlRxTypeSet failed for dp %d",localDpId);
         }

        }

   }

    return GT_OK;
 }

/**
* @internal  cpssDxChPortPfcReponceModeSet function
* @endinternal
*
* @brief   Set PFC responce mode (8TC per port or 16 TC per port)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      (APPLICABLE DEVICES Falcon)
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.

* @param[in] mode                                               responce mode
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcReponceModeSet
(
    IN  GT_U8                                devNum,
    IN  GT_DATA_PATH_BMP                     dataPathBmp,
    IN  CPSS_DXCH_PORT_PFC_RESPONCE_MODE_ENT mode
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcReponceModeSet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId,devNum,dataPathBmp,mode));

  rc = internal_cpssDxChPortPfcReponceModeSet(devNum,dataPathBmp,mode);

  CPSS_LOG_API_EXIT_MAC(funcId,rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId,devNum,dataPathBmp,mode));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}


static GT_STATUS internal_cpssDxChPortPfcReponceModeGet
(
    IN  GT_U8                                devNum,
    IN  GT_DATA_PATH_BMP                     dataPathBmp,
    IN  CPSS_DXCH_PORT_PFC_RESPONCE_MODE_ENT *modePtr
)
{
    GT_STATUS rc;
    GT_U32    lDpSize,ii;
    GT_U32    localDpId,tileId;
    GT_U32    macNum;
    GT_BOOL   responceMode16Tc = GT_FALSE;

    *modePtr = CPSS_DXCH_PORT_PFC_RESPONCE_MODE_8_TC_E;

    lDpSize = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp;
    /* Shifting a number of bits greater than or
                       equal to the width of the promoted left operand is undefined behavior in C*/
    if(lDpSize<32)
    {
        if(dataPathBmp == CPSS_DATA_PATH_UNAWARE_MODE_CNS)
        {
           dataPathBmp = (1<<lDpSize)-1;
        }

        if(dataPathBmp>>lDpSize)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        if(dataPathBmp == CPSS_DATA_PATH_UNAWARE_MODE_CNS)
        {
            dataPathBmp=(GT_U32)-1;
        }
    }

    if(dataPathBmp == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }




    for(ii = 0;dataPathBmp&& (ii < lDpSize); ++ii,dataPathBmp>>=1)
    {
        if(dataPathBmp&1)
        {
          rc = prvCpssFalconGlobalDpToTileAndLocalDp(devNum,ii,&localDpId,&tileId);
          if (rc != GT_OK)
          {
               return rc;
          }
          /*Assumption :
                        1. Port 0 is mapped
                        2.For port that has MAC ,MAC equal DMA
                      */
          rc = prvCpssFalconTxqUtilsFalconTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvert(devNum,tileId,localDpId,0,&macNum);
          if (rc != GT_OK)
          {
               return rc;
          }

         rc = prvCpssDxChPortSip6PfcConfigFcuControlRxTypeGet(devNum,macNum,&responceMode16Tc);

         if(rc!=GT_OK)
         {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortSip6PfcConfigFcuControlRxTypeSet failed for dp %d",localDpId);
         }
         break;
        }

   }

   if(responceMode16Tc == GT_TRUE)
   {
     *modePtr = CPSS_DXCH_PORT_PFC_RESPONCE_MODE_16_TC_E;
   }

    return GT_OK;
 }
/**
* @internal  cpssDxChPortPfcReponceModeGet function
* @endinternal
*
* @brief   Get PFC responce mode (8TC per port or 16 TC per port)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      (APPLICABLE DEVICES Falcon)
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.

* @param[in] modePtr                                              (pointer to ) responce mode
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcReponceModeGet
(
    IN  GT_U8                                devNum,
    IN  GT_DATA_PATH_BMP                     dataPathBmp,
    IN  CPSS_DXCH_PORT_PFC_RESPONCE_MODE_ENT *modePtr
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcReponceModeGet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId,devNum,dataPathBmp,modePtr));

  rc = internal_cpssDxChPortPfcReponceModeGet(devNum,dataPathBmp,modePtr);

  CPSS_LOG_API_EXIT_MAC(funcId,rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId,devNum,dataPathBmp,modePtr));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
* @internal  internal_cpssDxChPortPfcLinkPauseCompatibilityModeEnableSet
* @endinternal
*
* @brief   If enabled, an IEEE 802.3x PAUSE frame is treated as PFC with all eight timers valid
*              and the timer value equal to the Timer value in the 802.3x Pause frame.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] portNum                                         physical port number
* @param[in] enable                                              If equal GT_TRUE ,treat IEEE 802.3x PAUSE frame  as PFC
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
static GT_STATUS internal_cpssDxChPortPfcLinkPauseCompatibilityModeEnableSet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_BOOL                  enable
)
{

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E |  CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
        CPSS_ALDRIN_E | CPSS_BOBCAT3_E |CPSS_AC3X_E|CPSS_ALDRIN2_E);

    return prvCpssDxChPortSip6PfcLinkPauseCompatibilityEnableSet(devNum,portNum,enable);
}

/**
* @internal  cpssDxChPortPfcLinkPauseCompatibilityModeEnableSet
* @endinternal
*
* @brief   If enabled, an IEEE 802.3x PAUSE frame is treated as PFC with all eight timers valid
*              and the timer value equal to the Timer value in the 802.3x Pause frame.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] portNum                                         physical port number
* @param[in] enable                                              If equal GT_TRUE ,treat IEEE 802.3x PAUSE frame  as PFC
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcLinkPauseCompatibilityModeEnableSet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_BOOL                  enable
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcLinkPauseCompatibilityModeEnableSet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId,devNum,portNum,enable));

  rc = internal_cpssDxChPortPfcLinkPauseCompatibilityModeEnableSet(devNum,portNum,enable);

  CPSS_LOG_API_EXIT_MAC(funcId,rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId,devNum,portNum,enable));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}



/**
* @internal  internal_cpssDxChPortPfcLinkPauseCompatibilityModeEnableGet
* @endinternal
*
* @brief   Get Link Pause Compatibility Mode .If enabled, an IEEE 802.3x PAUSE frame is treated as PFC with all eight timers valid
*              and the timer value equal to the Timer value in the 802.3x Pause frame.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] portNum                                         physical port number
* @param[in] enable                                              If equal GT_TRUE ,treat IEEE 802.3x PAUSE frame  as PFC
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
static GT_STATUS internal_cpssDxChPortPfcLinkPauseCompatibilityModeEnableGet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_BOOL                 *enablePtr
)
{

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E |  CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
        CPSS_ALDRIN_E | CPSS_BOBCAT3_E |CPSS_AC3X_E|CPSS_ALDRIN2_E);

    return prvCpssDxChPortSip6PfcLinkPauseCompatibilityEnableGet(devNum,portNum,enablePtr);
}

/**
* @internal cpssDxChPortPfcLinkPauseCompatibilityModeEnableGet
* @endinternal
*
* @brief   Get Link Pause Compatibility Mode .If enabled, an IEEE 802.3x PAUSE frame is treated as PFC with all eight timers valid
*              and the timer value equal to the Timer value in the 802.3x Pause frame.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] portNum                                         physical port number
* @param[in] enable                                              If equal GT_TRUE ,treat IEEE 802.3x PAUSE frame  as PFC
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcLinkPauseCompatibilityModeEnableGet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_BOOL                 *enablePtr
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcLinkPauseCompatibilityModeEnableGet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId,devNum,portNum,enablePtr));

  rc = internal_cpssDxChPortPfcLinkPauseCompatibilityModeEnableGet(devNum,portNum,enablePtr);

  CPSS_LOG_API_EXIT_MAC(funcId,rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId,devNum,portNum,enablePtr));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
* @internal internal_cpssDxChPortPfcDbaTcAvailebleBuffersSet
* @endinternal
*
* @brief   Set amount of   buffers available for dynamic allocation for PFC for specific TC.
*  This API can also aupdate value for Port/TC availeble buffers.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.

* @param[in] confMode       Define what availeble buffer value should be updated (Global TC/Port-TC/Both)
* @param[in] tc       Traffic class [0..7]
* @param[in] availableBuffers - amount of available buffers
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
static GT_STATUS  internal_cpssDxChPortPfcDbaTcAvailableBuffersSet
(
   IN  GT_U8   devNum,
   IN  GT_DATA_PATH_BMP dataPathBmp,
   IN  CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_ENT  confMode,
   IN  GT_U8   tc,
   IN  GT_U32  availableBuffers
)
{
    GT_U32 lDpSize,ii,localDpId,tileId;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
    CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
        CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_XCAT3_E);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);

    if (prvCpssDxchFalconCiderVersionGet()<=PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_06_06_E)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "This API si not supported on this Cider version");
    }

    lDpSize = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp;

    /* Shifting a number of bits greater than or
            equal to the width of the promoted left operand is undefined behavior in C*/
    if(lDpSize<32)
    {

        if(dataPathBmp == CPSS_DATA_PATH_UNAWARE_MODE_CNS)
        {
           dataPathBmp = (1<<lDpSize)-1;
        }

        if(dataPathBmp>>lDpSize)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        if(dataPathBmp == CPSS_DATA_PATH_UNAWARE_MODE_CNS)
        {
            dataPathBmp=(GT_U32)-1;
        }
    }

    if(dataPathBmp == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for(ii = 0;dataPathBmp&& (ii < lDpSize); ++ii,dataPathBmp>>=1)
    {
        if(dataPathBmp&1)
        {

          rc = prvCpssFalconGlobalDpToTileAndLocalDp(devNum,ii,&localDpId,&tileId);
          if (rc != GT_OK)
          {
               return rc;
          }

          if(confMode!=CPSS_PORT_TX_PFC_AVAILEBLE_BUFF_CONF_MODE_GLOBAL_AND_PORT_TC_E)
          {
              rc = prvCpssFalconTxqQfcTcAvaileblePoolBuffersSet(devNum,tileId,localDpId,confMode,tc,availableBuffers);
              if(rc!=GT_OK)
              {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcTcAvaileblePoolBuffersSet failed for dp %d",localDpId);
              }
          }
          else
          {
            rc = prvCpssFalconTxqQfcTcAvaileblePoolBuffersSet(devNum,tileId,localDpId,CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_GLOBAL_TC_E,tc,availableBuffers);
            if(rc!=GT_OK)
            {
               CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcTcAvaileblePoolBuffersSet failed for dp %d",localDpId);
            }

            rc = prvCpssFalconTxqQfcTcAvaileblePoolBuffersSet(devNum,tileId,localDpId,CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_PORT_TC_E,tc,availableBuffers);
            if(rc!=GT_OK)
            {
               CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcTcAvaileblePoolBuffersSet failed for dp %d",localDpId);
            }
          }

        }
   }

   return GT_OK;
}

/**
* @internal cpssDxChPortPfcDbaTcAvailableBuffersSet
* @endinternal
*
* @brief   Set amount of   buffers available for dynamic allocation for PFC for specific TC.
*  This API can also aupdate value for Port/TC availeble buffers.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.

* @param[in] confMode       Define what availeble buffer value should be updated (Global TC/Port-TC/Both)
* @param[in] tc       Traffic class [0..7]
* @param[in] availableBuffers - amount of available buffers
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS  cpssDxChPortPfcDbaTcAvailableBuffersSet
(
   IN  GT_U8   devNum,
   IN  GT_DATA_PATH_BMP dataPathBmp,
   IN  CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_ENT  confMode,
   IN  GT_U8   tc,
   IN  GT_U32  availableBuffers
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcDbaTcAvailableBuffersSet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId,devNum,dataPathBmp,confMode,tc,availableBuffers));

  rc = internal_cpssDxChPortPfcDbaTcAvailableBuffersSet(devNum,dataPathBmp,confMode,tc,availableBuffers);

  CPSS_LOG_API_EXIT_MAC(funcId,rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId,devNum,dataPathBmp,confMode,tc,availableBuffers));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
* @internal internal_cpssDxChPortPfcDbaTcAvailableBuffersGet
* @endinternal
*
* @brief   Get amount of   buffers available for dynamic allocation for PFC for specific TC.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.

* @param[in] confMode       Define what availeble buffer value should be updated (Global TC/Port-TC/Both)
* @param[in] tc       Traffic class [0..7]
* @param[out] availableBuffersPtr - (pointer to)amount of available buffers
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
static GT_STATUS  internal_cpssDxChPortPfcDbaTcAvailableBuffersGet
(
   IN  GT_U8   devNum,
   IN  GT_DATA_PATH_BMP dataPathBmp,
   IN  CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_ENT  confMode,
   IN  GT_U8   tc,
   OUT  GT_U32  *availableBuffersPtr
)
{
    GT_U32 lDpSize,ii,localDpId,tileId;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
    CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
        CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_XCAT3_E);

    CPSS_NULL_PTR_CHECK_MAC(availableBuffersPtr);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);

    if (prvCpssDxchFalconCiderVersionGet()<=PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_06_06_E)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "This API si not supported on this Cider version");
    }

    if(confMode == CPSS_PORT_TX_PFC_AVAILEBLE_BUFF_CONF_MODE_GLOBAL_AND_PORT_TC_E)
    {
        /*Can't get both*/
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "CPSS_PORT_TX_PFC_AVAILEBLE_BUFF_CONF_MODE_GLOBAL_AND_PORT_TC_E ca not be used as argument");
    }

    lDpSize = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp;

   /* Shifting a number of bits greater than or
            equal to the width of the promoted left operand is undefined behavior in C*/
    if(lDpSize<32)
    {

        if(dataPathBmp == CPSS_DATA_PATH_UNAWARE_MODE_CNS)
        {
           dataPathBmp = (1<<lDpSize)-1;
        }

        if(dataPathBmp>>lDpSize)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        if(dataPathBmp == CPSS_DATA_PATH_UNAWARE_MODE_CNS)
        {
            dataPathBmp=(GT_U32)-1;
        }
    }

    if(dataPathBmp == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for(ii = 0;dataPathBmp&& (ii < lDpSize); ++ii,dataPathBmp>>=1)
    {
        if(dataPathBmp&1)
        {

          rc = prvCpssFalconGlobalDpToTileAndLocalDp(devNum,ii,&localDpId,&tileId);
          if (rc != GT_OK)
          {
               return rc;
          }

          rc = prvCpssFalconTxqQfcTcAvaileblePoolBuffersGet(devNum,tileId,localDpId,confMode,tc,availableBuffersPtr);
          if(rc!=GT_OK)
          {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcTcAvaileblePoolBuffersGet failed for dp %d",localDpId);
          }
          break;
        }
   }

   return GT_OK;
}

/**
* @internal internal_cpssDxChPortPfcDbaTcAvailableBuffersGet
* @endinternal
*
* @brief   Get amount of   buffers available for dynamic allocation for PFC for specific TC.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.

* @param[in] confMode       Define what availeble buffer value should be updated (Global TC/Port-TC/Both)
* @param[in] tc       Traffic class [0..7]
* @param[out] availableBuffersPtr - (pointer to)amount of available buffers
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS  cpssDxChPortPfcDbaTcAvailableBuffersGet
(
   IN  GT_U8   devNum,
   IN  GT_DATA_PATH_BMP dataPathBmp,
   IN  CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_ENT  confMode,
   IN  GT_U8   tc,
   OUT  GT_U32  *availableBuffersPtr
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcDbaTcAvailableBuffersGet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId,devNum,dataPathBmp,confMode,tc,availableBuffersPtr));

  rc = internal_cpssDxChPortPfcDbaTcAvailableBuffersGet(devNum,dataPathBmp,confMode,tc,availableBuffersPtr);

  CPSS_LOG_API_EXIT_MAC(funcId,rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId,devNum,dataPathBmp,confMode,tc,availableBuffersPtr));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}

/**
* @internal internal_cpssDxChPortPfcTcResourceModeSet
* @endinternal
*
* @brief  Configure mode that count occupied buffers (Packet buffer occupied or Pool occupied).
*              In case Pool mode is selected ,headroom subtraction  can be enabled by settin mode to
*              CPSS_PORT_TX_PFC_RESOURCE_MODE_POOL_WITH_HEADROOM_SUBTRACTION_E
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] tc       Traffic class [0..7]
* @param[in] mode - resource mode ( PB/Pool/Pool with headroom subtraction)
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
static GT_STATUS internal_cpssDxChPortPfcTcResourceModeSet
(
    IN GT_U8                                 devNum,
    IN GT_U8                                 tc,
    IN CPSS_PORT_TX_PFC_RESOURCE_MODE_ENT    mode
)
{
    GT_STATUS rc;
    GT_U32 poolId;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
    CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
        CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_XCAT3_E);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);

    if (prvCpssDxchFalconCiderVersionGet()<=PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_06_06_E)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "This API si not supported on this Cider version");
    }

    if(mode!=CPSS_PORT_TX_PFC_RESOURCE_MODE_GLOBAL_E)
    {
        /*Get pool id*/
        rc = prvCpssFalconTxqPfccMapTcToPoolGet(devNum,PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC,tc,&poolId);
        if(rc!=GT_OK)
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqPfccMapTcToPoolGet failed for tc %d",tc);
        }
    }
    else
    {
        poolId = PRV_TC_MAP_PB_MAC;
    }

    rc = prvCpssFalconTxqPfccTcResourceModeSet(devNum,PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC,tc,GT_TRUE,poolId,
        (mode==CPSS_PORT_TX_PFC_RESOURCE_MODE_GLOBAL_E)?GT_FALSE:GT_TRUE,
        (mode==CPSS_PORT_TX_PFC_RESOURCE_MODE_POOL_WITH_HEADROOM_SUBTRACTION_E)?GT_TRUE:GT_FALSE);

    if(rc!=GT_OK)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqPfccMapTcToPoolGet failed for tc %d",tc);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortPfcTcResourceModeSet
* @endinternal
*
* @brief  Configure mode that count occupied buffers (Packet buffer occupied or Pool occupied).
*              In case Pool mode is selected ,headroom subtraction  can be enabled by settin mode to
*              CPSS_PORT_TX_PFC_RESOURCE_MODE_POOL_WITH_HEADROOM_SUBTRACTION_E
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] tc       Traffic class [0..7]
* @param[in] mode - resource mode ( PB/Pool/Pool with headroom subtraction)
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcTcResourceModeSet
(
    IN GT_U8                                 devNum,
    IN GT_U8                                 tc,
    IN CPSS_PORT_TX_PFC_RESOURCE_MODE_ENT    mode
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcTcResourceModeSet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId,devNum,tc,mode));

  rc = internal_cpssDxChPortPfcTcResourceModeSet(devNum,tc,mode);

  CPSS_LOG_API_EXIT_MAC(funcId,rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId,devNum,tc,mode));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}


/**
* @internal internal_cpssDxChPortPfcTcResourceModeGet
* @endinternal
*
* @brief  Get configured mode that count occupied buffers (Packet buffer occupied or Pool occupied).
*              In case Pool mode is selected ,headroom subtraction  can be enabled by settin mode to
*              CPSS_PORT_TX_PFC_RESOURCE_MODE_POOL_WITH_HEADROOM_SUBTRACTION_E
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] tc       Traffic class [0..7]
* @param[in] modePtr - (pointer to)resource mode ( PB/Pool/Pool with headroom subtraction)
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
static GT_STATUS internal_cpssDxChPortPfcTcResourceModeGet
(
    IN GT_U8                                    devNum,
    IN GT_U8                                    tc,
    IN CPSS_PORT_TX_PFC_RESOURCE_MODE_ENT       *modePtr
)
{
    GT_STATUS   rc;
    GT_U32      poolId;
    GT_BOOL     headRoom;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
    CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
        CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_XCAT3_E);

    CPSS_NULL_PTR_CHECK_MAC(modePtr);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);

    if (prvCpssDxchFalconCiderVersionGet()<=PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_06_06_E)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "This API si not supported on this Cider version");
    }

    rc = prvCpssFalconTxqPfccTcResourceModeGet(devNum,PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC,tc,&poolId,&headRoom);

    if(rc!=GT_OK)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqPfccMapTcToPoolGet failed for tc %d",tc);
    }

    if(poolId == PRV_TC_MAP_PB_MAC)
    {
        *modePtr = CPSS_PORT_TX_PFC_RESOURCE_MODE_GLOBAL_E;
    }
    else
    {
        if(GT_FALSE==headRoom)
        {
            *modePtr = CPSS_PORT_TX_PFC_RESOURCE_MODE_POOL_E;
        }
        else
        {
            *modePtr = CPSS_PORT_TX_PFC_RESOURCE_MODE_POOL_WITH_HEADROOM_SUBTRACTION_E;
        }
    }

   return GT_OK;
}


/**
* @internal cpssDxChPortPfcTcResourceModeGet
* @endinternal
*
* @brief  Get configured mode that count occupied buffers (Packet buffer occupied or Pool occupied).
*              In case Pool mode is selected ,headroom subtraction  can be enabled by settin mode to
*              CPSS_PORT_TX_PFC_RESOURCE_MODE_POOL_WITH_HEADROOM_SUBTRACTION_E
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] tc       Traffic class [0..7]
* @param[in] modePtr - (pointer to)resource mode ( PB/Pool/Pool with headroom subtraction)
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcTcResourceModeGet
(
    IN GT_U8                                    devNum,
    IN GT_U8                                    tc,
    IN CPSS_PORT_TX_PFC_RESOURCE_MODE_ENT       *modePtr
)
{
  GT_STATUS rc;

  CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPfcTcResourceModeGet);
  CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
  CPSS_LOG_API_ENTER_MAC((funcId,devNum,tc,modePtr));

  rc = internal_cpssDxChPortPfcTcResourceModeGet(devNum,tc,modePtr);

  CPSS_LOG_API_EXIT_MAC(funcId,rc);
  CPSS_APP_SPECIFIC_CB_MAC((funcId,devNum,tc,modePtr));
  CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

  return rc;
}



