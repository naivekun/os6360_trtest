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
* @file cpssDxChIpfix.c
*
* @brief CPSS APIs for IP Flow Information Export (IPFIX).
*
* @version   21
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/ipfix/private/prvCpssDxChIpfixLog.h>

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/ipfix/cpssDxChIpfix.h>
#include <cpss/dxCh/dxCh3/policer/private/prvCpssDxCh3Policer.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiag.h>


/* Policer timer 6 is the IPfix nano timer. */
#define PRV_CPSS_DXCH_IPFIX_NANO_TIMER_CNS          6

/* Policer timer 7 is the second timer [31:0] */
#define PRV_CPSS_DXCH_IPFIX_SECOND_LSB_TIMER_CNS    7

/* Policer timer 8 is the second timer [63:32] */
#define PRV_CPSS_DXCH_IPFIX_SECOND_MSB_TIMER_CNS    8

/* 10^9 nano seconds in second.
NOTE: the device hole 30 bits that allow value of: 1,073,741,824
    that is 7.3% more then 1,000,000,000
*/
#define NANO_SECOND_MAX_CNS 1000000000

/**
* @internal internal_cpssDxChIpfixEntrySet function
* @endinternal
*
* @brief   Sets IPFIX entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - index of IPFIX Entry.
*                                      Range: see datasheet for specific device.
* @param[in] ipfixEntryPtr            - pointer to the IPFIX Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_OUT_OF_RANGE          - on value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In order to set IPFIX entry under traffic perform the following
*       algorithm(none-zero setting of IPFIX Counters while enabled\still
*       under traffic may cause data coherency problems):
*       - disconnect entry from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entry back to traffic
*       This API address the same memory area (counters place) as can be set
*       by "cpssDxCh3PolicerBillingEntrySet".
*
*/
static GT_STATUS internal_cpssDxChIpfixEntrySet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                              entryIndex,
    IN  CPSS_DXCH_IPFIX_ENTRY_STC           *ipfixEntryPtr
)
{
    return cpssDxChIpfixPortGroupEntrySet(devNum,
                                          CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                          stage,
                                          entryIndex,
                                          ipfixEntryPtr);
}

/**
* @internal cpssDxChIpfixEntrySet function
* @endinternal
*
* @brief   Sets IPFIX entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - index of IPFIX Entry.
*                                      Range: see datasheet for specific device.
* @param[in] ipfixEntryPtr            - pointer to the IPFIX Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_OUT_OF_RANGE          - on value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In order to set IPFIX entry under traffic perform the following
*       algorithm(none-zero setting of IPFIX Counters while enabled\still
*       under traffic may cause data coherency problems):
*       - disconnect entry from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entry back to traffic
*       This API address the same memory area (counters place) as can be set
*       by "cpssDxCh3PolicerBillingEntrySet".
*
*/
GT_STATUS cpssDxChIpfixEntrySet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                              entryIndex,
    IN  CPSS_DXCH_IPFIX_ENTRY_STC           *ipfixEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, entryIndex, ipfixEntryPtr));

    rc = internal_cpssDxChIpfixEntrySet(devNum, stage, entryIndex, ipfixEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, entryIndex, ipfixEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpfixEntryGet function
* @endinternal
*
* @brief   Gets IPFIX entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - index of IPFIX Entry.
*                                      Range: see datasheet for specific device.
* @param[in] reset                    -  flag (please refer to COMMENTS section for further
*                                      explanations):
*                                      GT_TRUE  - performing read and reset atomic operation.
*                                      GT_FALSE - performing read entry operation only.
*
* @param[out] ipfixEntryPtr            - pointer to the IPFIX Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_BAD_STATE             - on counter entry type mismatch.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note When counters are read in clear on read mode (reset == GT_TRUE), The
*       following actions are done on the different IPFIX entry fields:
*       - The <Packet Counter>, <Byte Counter>, and <Dropped Packet Counter>
*       fields are cleared.
*       - The <Timestamp> field is updated to the current time.
*       - The <Last Sampled Value> is cleared.
*
*/
static GT_STATUS internal_cpssDxChIpfixEntryGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                              entryIndex,
    IN  GT_BOOL                             reset,
    OUT CPSS_DXCH_IPFIX_ENTRY_STC           *ipfixEntryPtr
)
{
    return cpssDxChIpfixPortGroupEntryGet(devNum,
                                          CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                          stage,
                                          entryIndex,
                                          reset,
                                          ipfixEntryPtr);
}

/**
* @internal cpssDxChIpfixEntryGet function
* @endinternal
*
* @brief   Gets IPFIX entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - index of IPFIX Entry.
*                                      Range: see datasheet for specific device.
* @param[in] reset                    -  flag (please refer to COMMENTS section for further
*                                      explanations):
*                                      GT_TRUE  - performing read and reset atomic operation.
*                                      GT_FALSE - performing read entry operation only.
*
* @param[out] ipfixEntryPtr            - pointer to the IPFIX Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_BAD_STATE             - on counter entry type mismatch.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note When counters are read in clear on read mode (reset == GT_TRUE), The
*       following actions are done on the different IPFIX entry fields:
*       - The <Packet Counter>, <Byte Counter>, and <Dropped Packet Counter>
*       fields are cleared.
*       - The <Timestamp> field is updated to the current time.
*       - The <Last Sampled Value> is cleared.
*
*/
GT_STATUS cpssDxChIpfixEntryGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                              entryIndex,
    IN  GT_BOOL                             reset,
    OUT CPSS_DXCH_IPFIX_ENTRY_STC           *ipfixEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, entryIndex, reset, ipfixEntryPtr));

    rc = internal_cpssDxChIpfixEntryGet(devNum, stage, entryIndex, reset, ipfixEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, entryIndex, reset, ipfixEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpfixDropCountModeSet function
* @endinternal
*
* @brief   Configures IPFIX drop counting mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] mode                     - drop counting mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChIpfixDropCountModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT      mode
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      fieldValue;     /* field value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    switch (mode)
    {
        case CPSS_DXCH_IPFIX_DROP_COUNT_MODE_METER_ONLY_E:
            fieldValue = 0;
            break;
        case CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ALL_E:
            fieldValue = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).IPFIXCtrl;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].ipfixControl;
    }

    /* Set IPFIX Control register, <Enable dropped packets counting> field */
    return prvCpssHwPpSetRegField(devNum, regAddr, 8, 1, fieldValue);
}

/**
* @internal cpssDxChIpfixDropCountModeSet function
* @endinternal
*
* @brief   Configures IPFIX drop counting mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] mode                     - drop counting mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixDropCountModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT      mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixDropCountModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, mode));

    rc = internal_cpssDxChIpfixDropCountModeSet(devNum, stage, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpfixDropCountModeGet function
* @endinternal
*
* @brief   Gets IPFIX drop counting mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] modePtr                  - pointer to drop counting mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChIpfixDropCountModeGet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT     *modePtr
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      fieldValue;     /* field value  */
    GT_STATUS   rc;             /* return code  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(modePtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).IPFIXCtrl;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].ipfixControl;
    }

    /* Get IPFIX Control register, <Enable dropped packets counting> field */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 8, 1, &fieldValue);
    if (GT_OK != rc)
    {
        return rc;
    }

    *modePtr = ( fieldValue == 0 ) ? CPSS_DXCH_IPFIX_DROP_COUNT_MODE_METER_ONLY_E :
                                     CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ALL_E ;

    return GT_OK;
}

/**
* @internal cpssDxChIpfixDropCountModeGet function
* @endinternal
*
* @brief   Gets IPFIX drop counting mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] modePtr                  - pointer to drop counting mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixDropCountModeGet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT     *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixDropCountModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, modePtr));

    rc = internal_cpssDxChIpfixDropCountModeGet(devNum, stage, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpfixCpuCodeSet function
* @endinternal
*
* @brief   Sets the 6 most significant bits of the CPU Code in case of mirroring.
*         The two least significant bits are taken from the IPfix entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] cpuCode                  - A cpu code to set the 6 most significant bits.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or cpu code.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChIpfixCpuCodeSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN CPSS_NET_RX_CPU_CODE_ENT         cpuCode
)
{
    GT_U32  regAddr;                                    /* register address */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;  /* DSA code */
    GT_STATUS   rc;                                     /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    rc = prvCpssDxChNetIfCpuToDsaCode(cpuCode,&dsaCpuCode);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* Since this CPU code is expected to set only the 6 MSb, while the 2 LSb  */
    /* are to be set by the IPFIX entry <CPU sub code> field, the 2 LSb should */
    /* be unset, thus enabling a range for 4 consecutive CPU codes.            */
    if( (dsaCpuCode & 0x3) != 0x0 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).IPFIXCtrl;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].ipfixControl;
    }

    /* Set IPFIX Control register, <ipfix cpu code > field */
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 6, (((GT_U32)dsaCpuCode) >> 2));
}

/**
* @internal cpssDxChIpfixCpuCodeSet function
* @endinternal
*
* @brief   Sets the 6 most significant bits of the CPU Code in case of mirroring.
*         The two least significant bits are taken from the IPfix entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] cpuCode                  - A cpu code to set the 6 most significant bits.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or cpu code.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixCpuCodeSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN CPSS_NET_RX_CPU_CODE_ENT         cpuCode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixCpuCodeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, cpuCode));

    rc = internal_cpssDxChIpfixCpuCodeSet(devNum, stage, cpuCode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, cpuCode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpfixCpuCodeGet function
* @endinternal
*
* @brief   Gets the 6 most significant bits of the CPU Code in case of mirroring.
*         The two least significant bits are taken from the IPfix entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] cpuCodePtr               - pointer to a cpu code to set the 6 most significant bits.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChIpfixCpuCodeGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT CPSS_NET_RX_CPU_CODE_ENT        *cpuCodePtr
)
{
    GT_U32  regAddr;                                    /* register address */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;  /* DSA code */
    GT_U32  tempCode;                                   /* dsa 6 LSb    */
    GT_STATUS   rc;                                     /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(cpuCodePtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).IPFIXCtrl;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].ipfixControl;
    }

    /* Get IPFIX Control register, <ipfix cpu code> field */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 6, &tempCode);
    if( rc != GT_OK )
    {
        return rc;
    }

    dsaCpuCode = (PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT)(tempCode << 2);

    return prvCpssDxChNetIfDsaToCpuCode(dsaCpuCode,cpuCodePtr);
}

/**
* @internal cpssDxChIpfixCpuCodeGet function
* @endinternal
*
* @brief   Gets the 6 most significant bits of the CPU Code in case of mirroring.
*         The two least significant bits are taken from the IPfix entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] cpuCodePtr               - pointer to a cpu code to set the 6 most significant bits.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixCpuCodeGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT CPSS_NET_RX_CPU_CODE_ENT        *cpuCodePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixCpuCodeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, cpuCodePtr));

    rc = internal_cpssDxChIpfixCpuCodeGet(devNum, stage, cpuCodePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, cpuCodePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpfixAgingEnableSet function
* @endinternal
*
* @brief   Enables or disabled the activation of aging for IPfix.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] enable                   - GT_TRUE for enable, GT_FALSE for disable.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChIpfixAgingEnableSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN GT_BOOL                          enable
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      fieldValue;     /* field value  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControlReg;
    }

    fieldValue = ( enable == GT_FALSE ) ? 0 : 1;

    /* Set Policer Control register, <Enable IPfix aging> field */
    return prvCpssHwPpSetRegField(devNum, regAddr, 22, 1, fieldValue);
}

/**
* @internal cpssDxChIpfixAgingEnableSet function
* @endinternal
*
* @brief   Enables or disabled the activation of aging for IPfix.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] enable                   - GT_TRUE for enable, GT_FALSE for disable.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixAgingEnableSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN GT_BOOL                          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixAgingEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, enable));

    rc = internal_cpssDxChIpfixAgingEnableSet(devNum, stage, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpfixAgingEnableGet function
* @endinternal
*
* @brief   Gets enabling status of aging for IPfix.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] enablePtr                - pointer to: GT_TRUE for enable, GT_FALSE for disable.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChIpfixAgingEnableGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT GT_BOOL                         *enablePtr
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      fieldValue;     /* field value  */
    GT_STATUS   rc;             /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControlReg;
    }

    /* Get Policer Control register, <Enable IPfix aging> field */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 22, 1, &fieldValue);
    if (GT_OK != rc)
    {
        return rc;
    }

    *enablePtr = ( fieldValue == 0 ) ? GT_FALSE : GT_TRUE ;

    return GT_OK;
}

/**
* @internal cpssDxChIpfixAgingEnableGet function
* @endinternal
*
* @brief   Gets enabling status of aging for IPfix.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] enablePtr                - pointer to: GT_TRUE for enable, GT_FALSE for disable.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixAgingEnableGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT GT_BOOL                         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixAgingEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, enablePtr));

    rc = internal_cpssDxChIpfixAgingEnableGet(devNum, stage, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpfixWraparoundConfSet function
* @endinternal
*
* @brief   Configures IPFIX wraparound parameters.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] confPtr                  - pointer to wraparound configuration structure.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, action or threshold.
* @retval GT_OUT_OF_RANGE          - on value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Since there is a HW limitation that wraparound byte threshold 18 LSBs
*       must be set to 0, the function ignores the 18 LSBs input value and set
*       them to 0.
*
*/
static GT_STATUS internal_cpssDxChIpfixWraparoundConfSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC       *confPtr
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      fieldValue;     /* field value  */
    GT_STATUS   rc;             /* return code  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(confPtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    switch (confPtr->action)
    {
        case CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_NONE_E:
            fieldValue = 0;
            break;
        case CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_CLEAR_E:
            fieldValue = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* check counters thresholds validity */
    if( confPtr->dropThreshold >= BIT_30 ||
        confPtr->packetThreshold >= BIT_30 ||
        confPtr->byteThreshold.l[1] >= BIT_4 ||
        ((confPtr->byteThreshold.l[1] == 0) && (confPtr->byteThreshold.l[0] < BIT_18)) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* Byte threshold 18lsb must be 0. */
    confPtr->byteThreshold.l[0] &= 0xFFFC0000 ;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).IPFIXCtrl;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].ipfixControl;
    }

    /* Set IPFIX Control register, <IPfix Wrap Around Action> field */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 9, 1, fieldValue);
    if (GT_OK != rc)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerIPFIXDroppedPktCountWAThreshold;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        PLR[stage].ipfixDroppedWaThreshold;
    }

    /* Set IPFIX dropped packet count WA threshold register */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 30, confPtr->dropThreshold);
    if (GT_OK != rc)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerIPFIXPktCountWAThreshold;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        PLR[stage].ipfixPacketWaThreshold;
    }

    /* Set IPFIX packet count WA threshold register */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 30, confPtr->packetThreshold);
    if (GT_OK != rc)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerIPFIXByteCountWAThresholdLSB;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        PLR[stage].ipfixByteWaThresholdLsb;
    }

    /* Set PLR IPFIX byte count WA threshold LSB register */
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, confPtr->byteThreshold.l[0]);
    if (GT_OK != rc)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerIPFIXByteCountWAThresholdMSB;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        PLR[stage].ipfixByteWaThresholdMsb;
    }

    /* Set PLR IPFIX byte count WA threshold MSB register */
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 4, confPtr->byteThreshold.l[1]);
}

/**
* @internal cpssDxChIpfixWraparoundConfSet function
* @endinternal
*
* @brief   Configures IPFIX wraparound parameters.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] confPtr                  - pointer to wraparound configuration structure.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, action or threshold.
* @retval GT_OUT_OF_RANGE          - on value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Since there is a HW limitation that wraparound byte threshold 18 LSBs
*       must be set to 0, the function ignores the 18 LSBs input value and set
*       them to 0.
*
*/
GT_STATUS cpssDxChIpfixWraparoundConfSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC       *confPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixWraparoundConfSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, confPtr));

    rc = internal_cpssDxChIpfixWraparoundConfSet(devNum, stage, confPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, confPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpfixWraparoundConfGet function
* @endinternal
*
* @brief   Get IPFIX wraparound configuration.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] confPtr                  - pointer to wraparound configuration structure.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChIpfixWraparoundConfGet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC      *confPtr
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      fieldValue;     /* field value  */
    GT_STATUS   rc;             /* return code  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(confPtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).IPFIXCtrl;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].ipfixControl;
    }

    /* Get IPFIX Control register, <IPfix Wrap Around Action> field */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 9, 1, &fieldValue);
    if (GT_OK != rc)
    {
        return rc;
    }

    confPtr->action = (fieldValue == 0) ? CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_NONE_E :
                                          CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_CLEAR_E;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerIPFIXDroppedPktCountWAThreshold;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        PLR[stage].ipfixDroppedWaThreshold;
    }

    /* Get IPFIX dropped packet count WA threshold register */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 30, &(confPtr->dropThreshold));
    if (GT_OK != rc)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerIPFIXPktCountWAThreshold;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        PLR[stage].ipfixPacketWaThreshold;
    }

    /* Get IPFIX packet count WA threshold register */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 30, &(confPtr->packetThreshold));
    if (GT_OK != rc)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerIPFIXByteCountWAThresholdLSB;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        PLR[stage].ipfixByteWaThresholdLsb;
    }

    /* Get PLR IPFIX byte count WA threshold LSB register */
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &(confPtr->byteThreshold.l[0]));
    if (GT_OK != rc)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerIPFIXByteCountWAThresholdMSB;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        PLR[stage].ipfixByteWaThresholdMsb;
    }

    /* Get PLR IPFIX byte count WA threshold MSB register */
    return prvCpssHwPpGetRegField(devNum, regAddr, 0, 4, &(confPtr->byteThreshold.l[1]));
}

/**
* @internal cpssDxChIpfixWraparoundConfGet function
* @endinternal
*
* @brief   Get IPFIX wraparound configuration.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] confPtr                  - pointer to wraparound configuration structure.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixWraparoundConfGet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC      *confPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixWraparoundConfGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, confPtr));

    rc = internal_cpssDxChIpfixWraparoundConfGet(devNum, stage, confPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, confPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChIpfixGetBitmap function
* @endinternal
*
* @brief   Utility function to get bitmap or part of it.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] startIndex               - the first bits mapbit.
* @param[in] endIndex                 - the last bits mapbit.
* @param[in] memStart                 - address of the bitmap start.
* @param[in] reset                    - clear the bits after read:
*                                      GT_TRUE - clear,
*                                      GT_FALSE - don't clear.
*
* @param[out] outPtr                   - pointer to a buffer containig the bitmap data.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
*/
static GT_STATUS prvCpssDxChIpfixGetBitmap
(
    IN GT_U8                devNum,
    IN GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN GT_U32               startIndex,
    IN GT_U32               endIndex,
    IN GT_U32               memStart,
    IN GT_BOOL              reset,
    OUT GT_U32              *outPtr
)
{
    GT_U32  i,j;
    GT_U32  memData;        /* address of the complete HW bitmap */
    GT_U32  startWord;      /* the word in the HW bitmap with contains the "startIndex" bit */
    GT_U32  endWord;        /* the word in the HW bitmap with contains the "endIndex" bit */
    GT_U32  startBitInWord; /* "startIndex" bit place in "startWord" */
    GT_U32  endBitInWord;   /* "endIndex" bit place in "endWord" */
    GT_STATUS rc;           /* return code */
    GT_U32  portGroupId;    /* the port group Id - support multi-port-groups device */

    CPSS_NULL_PTR_CHECK_MAC(outPtr);

    startWord = startIndex / 32;
    endWord   = endIndex / 32;
    startBitInWord = startIndex % 32;
    endBitInWord   = endIndex % 32;

    for( j = 0 ; j <= (endWord - startWord) ; j++ )
    {
        outPtr[j] = 0;
    }

    /* loop on all active port groups in the bmp */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        j = 0;

        /* Only if startWord is different from endWord the following for loop is done. */
        /* During each iteration consecutive 32 bits are read but only relevant ones   */
        /* are set to 0 in case reset flag is enabled.                                 */
        for( i = startWord ; i < endWord ; i++ )
        {
            /* Get bits from the first word starting at startBitInWord place. */
            rc = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId,
                                                     (memStart + i*4), &memData);
            if (rc != GT_OK)
            {
                return rc;
            }
            outPtr[j] |= (memData >> startBitInWord);

            if( reset == GT_TRUE )
            {
                rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,
                                                        (memStart + i*4),
                                                        startBitInWord, 32 - startBitInWord, 0);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }

            /* In case startBitInWord is not 0 bits should be read from the adjacent word. */
            if( startBitInWord != 0 )
            {
                rc = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId,
                                                         (memStart + i*4 + 4), &memData);
                if (rc != GT_OK)
                {
                    return rc;
                }
                outPtr[j] |= (memData << (32 - startBitInWord));

                if( reset == GT_TRUE )
                {
                    if( i == (endWord-1))
                    {
                        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,
                                                                (memStart + i*4 + 4),
                                                                0, endBitInWord + 1, 0);
                    }
                    else
                    {
                        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,
                                                                (memStart + i*4 + 4),
                                                                0, startBitInWord, 0);
                    }

                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
            j++;
        }

        /* Reading bits from the endWord (which is the only word in case  startIndex and */
        /* endIndex are in the same HW word) in case endBitInWord >= startBitInWord,     */
        /* otherwise the bits from endWord were alreay read in the for loop.             */
        if( endBitInWord >= startBitInWord )
        {
            rc = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId,
                                                     (memStart + i*4), &memData);
            if (rc != GT_OK)
            {
                return rc;
            }

            outPtr[j] |= (memData >> startBitInWord);

            if( reset == GT_TRUE )
            {
                rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,
                                                        (memStart + i*4),
                                                        startBitInWord,
                                                        endBitInWord - startBitInWord + 1,
                                                        0);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    return GT_OK;
}

/**
* @internal internal_cpssDxChIpfixWraparoundStatusGet function
* @endinternal
*
* @brief   Retrieves a bitmap of wraparound entries indexes.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] startIndex               - the first entry index to get wraparound indication on.
* @param[in] endIndex                 - the last entry index to get wraparound indication on.
* @param[in] reset                    - clear the bits after read:
*                                      GT_TRUE - clear,
*                                      GT_FALSE - don't clear.
*
* @param[out] bmpPtr                   - pointer to a bitmap indication wraparound.
*                                      each bit indicate: 0 - no WA occured, 1 - WA occured.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note It is application responsibility to allocate the buffer size for the
*       bitmap(bmpPtr). Since the buffer is made from GT_U32 elements, each
*       element contains 32 indications.
*
*/
static GT_STATUS internal_cpssDxChIpfixWraparoundStatusGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32                           startIndex,
    IN GT_U32                           endIndex,
    IN GT_BOOL                          reset,
    OUT GT_U32                          *bmpPtr
)
{
    return cpssDxChIpfixPortGroupWraparoundStatusGet(devNum,
                                                     CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                     stage,
                                                     startIndex,
                                                     endIndex,
                                                     reset,
                                                     bmpPtr);
}

/**
* @internal cpssDxChIpfixWraparoundStatusGet function
* @endinternal
*
* @brief   Retrieves a bitmap of wraparound entries indexes.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] startIndex               - the first entry index to get wraparound indication on.
* @param[in] endIndex                 - the last entry index to get wraparound indication on.
* @param[in] reset                    - clear the bits after read:
*                                      GT_TRUE - clear,
*                                      GT_FALSE - don't clear.
*
* @param[out] bmpPtr                   - pointer to a bitmap indication wraparound.
*                                      each bit indicate: 0 - no WA occured, 1 - WA occured.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note It is application responsibility to allocate the buffer size for the
*       bitmap(bmpPtr). Since the buffer is made from GT_U32 elements, each
*       element contains 32 indications.
*
*/
GT_STATUS cpssDxChIpfixWraparoundStatusGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32                           startIndex,
    IN GT_U32                           endIndex,
    IN GT_BOOL                          reset,
    OUT GT_U32                          *bmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixWraparoundStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, startIndex, endIndex, reset, bmpPtr));

    rc = internal_cpssDxChIpfixWraparoundStatusGet(devNum, stage, startIndex, endIndex, reset, bmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, startIndex, endIndex, reset, bmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpfixAgingStatusGet function
* @endinternal
*
* @brief   Retrieves a bitmap of aged entries indexes.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] startIndex               - the first entry index to get aging indication on.
* @param[in] endIndex                 - the last entry index to get aging indication on.
* @param[in] reset                    - clear the bits after read:
*                                      GT_TRUE - clear,
*                                      GT_FALSE - don't clear.
*
* @param[out] bmpPtr                   - pointer to a bitmap indication aging.
*                                      each bit indicate: 0 - no packet recieved on flow (aged).
*                                      1 - packet recieved on flow.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note It is application responsibility to allocate the buffer size for the
*       bitmap(bmpPtr). Since the buffer is made from GT_U32 elements, each
*       element contains 32 indications.
*
*/
static GT_STATUS internal_cpssDxChIpfixAgingStatusGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32                           startIndex,
    IN GT_U32                           endIndex,
    IN GT_BOOL                          reset,
    OUT GT_U32                          *bmpPtr
)
{
    return cpssDxChIpfixPortGroupAgingStatusGet(devNum,
                                                CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                stage,
                                                startIndex,
                                                endIndex,
                                                reset,
                                                bmpPtr);
}

/**
* @internal cpssDxChIpfixAgingStatusGet function
* @endinternal
*
* @brief   Retrieves a bitmap of aged entries indexes.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] startIndex               - the first entry index to get aging indication on.
* @param[in] endIndex                 - the last entry index to get aging indication on.
* @param[in] reset                    - clear the bits after read:
*                                      GT_TRUE - clear,
*                                      GT_FALSE - don't clear.
*
* @param[out] bmpPtr                   - pointer to a bitmap indication aging.
*                                      each bit indicate: 0 - no packet recieved on flow (aged).
*                                      1 - packet recieved on flow.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note It is application responsibility to allocate the buffer size for the
*       bitmap(bmpPtr). Since the buffer is made from GT_U32 elements, each
*       element contains 32 indications.
*
*/
GT_STATUS cpssDxChIpfixAgingStatusGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32                           startIndex,
    IN GT_U32                           endIndex,
    IN GT_BOOL                          reset,
    OUT GT_U32                          *bmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixAgingStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, startIndex, endIndex, reset, bmpPtr));

    rc = internal_cpssDxChIpfixAgingStatusGet(devNum, stage, startIndex, endIndex, reset, bmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, startIndex, endIndex, reset, bmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpfixTimestampUploadSet function
* @endinternal
*
* @brief   Configures IPFIX timestamp upload per stages.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] uploadPtr                - pointer to timestamp upload configuration of a stage.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or upload mode.
* @retval GT_OUT_OF_RANGE          - on value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Actual upload operation is triggered by
*       "cpssDxChIpfixTimestampUploadTrigger".
*
*/
static GT_STATUS internal_cpssDxChIpfixTimestampUploadSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    IN CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC    *uploadPtr
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      fieldValue;     /* field value  */
    GT_U32      nanoSecHwVal;   /* hw value of nanoseconds */
    GT_STATUS   rc;             /* return code  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    CPSS_NULL_PTR_CHECK_MAC(uploadPtr);

    if( uploadPtr->timer.nanoSecondTimer >= NANO_SECOND_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    switch(uploadPtr->uploadMode)
    {
        case CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_ABSOLUTE_E:
            fieldValue = 0;
            break;
        case CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_INCREMENTAL_E:
            fieldValue = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).IPFIXCtrl;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].ipfixControl;
    }

    /* Set IPFIX Control register, <time stamp upload mode> field */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 7, 1, fieldValue);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* Check if TOD nanoseconds errata is enabled */
    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                                               PRV_CPSS_DXCH_XCAT_TOD_NANO_SEC_SET_WA_E))
    {
        /* nanoseconds value should be multiplies of 20ns */
        nanoSecHwVal = (uploadPtr->timer.nanoSecondTimer / 20) * 20;
    }
    else
    {
        nanoSecHwVal = uploadPtr->timer.nanoSecondTimer;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).IPFIXNanoTimerStampUpload;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        PLR[stage].ipfixNanoTimerStampUpload;
    }

    /* Set PLR IPFIX nano timer stamp upload */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 30, nanoSecHwVal);
    if (GT_OK != rc)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).IPFIXSecondsLSbTimerStampUpload;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        PLR[stage].ipfixSecLsbTimerStampUpload;
    }

    /* Set PLR IPFIX Seconds LSb timer stamp upload */
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, uploadPtr->timer.secondTimer.l[0]);
    if (GT_OK != rc)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).IPFIXSecondsMSbTimerStampUpload;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        PLR[stage].ipfixSecMsbTimerStampUpload;
    }

    /* Set PLR IPFIX Seconds MSb timer stamp upload */
    return prvCpssHwPpWriteRegister(devNum, regAddr, uploadPtr->timer.secondTimer.l[1]);
}

/**
* @internal cpssDxChIpfixTimestampUploadSet function
* @endinternal
*
* @brief   Configures IPFIX timestamp upload per stages.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] uploadPtr                - pointer to timestamp upload configuration of a stage.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or upload mode.
* @retval GT_OUT_OF_RANGE          - on value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Actual upload operation is triggered by
*       "cpssDxChIpfixTimestampUploadTrigger".
*
*/
GT_STATUS cpssDxChIpfixTimestampUploadSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    IN CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC    *uploadPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixTimestampUploadSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, uploadPtr));

    rc = internal_cpssDxChIpfixTimestampUploadSet(devNum, stage, uploadPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, uploadPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpfixTimestampUploadGet function
* @endinternal
*
* @brief   Gets IPFIX timestamp upload configuration for a stage.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] uploadPtr                - pointer to timestamp upload configuration of a stage.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChIpfixTimestampUploadGet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    OUT CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC   *uploadPtr
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      fieldValue;     /* field value  */
    GT_STATUS   rc;             /* return code  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    CPSS_NULL_PTR_CHECK_MAC(uploadPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).IPFIXCtrl;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].ipfixControl;
    }

    /* Get IPFIX Control register, <time stamp upload mode> field */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 7, 1, &fieldValue);
    if (GT_OK != rc)
    {
        return rc;
    }

    uploadPtr->uploadMode = (fieldValue == 0) ?
                            CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_ABSOLUTE_E:
                            CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_INCREMENTAL_E;


    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).IPFIXNanoTimerStampUpload;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        PLR[stage].ipfixNanoTimerStampUpload;
    }

    /* Get PLR IPFIX nano timer stamp upload */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 30, &(uploadPtr->timer.nanoSecondTimer));
    if (GT_OK != rc)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).IPFIXSecondsLSbTimerStampUpload;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        PLR[stage].ipfixSecLsbTimerStampUpload;
    }

    /* Get PLR IPFIX Seconds LSb timer stamp upload */
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &(uploadPtr->timer.secondTimer.l[0]));
    if (GT_OK != rc)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).IPFIXSecondsMSbTimerStampUpload;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        PLR[stage].ipfixSecMsbTimerStampUpload;
    }

    /* Get PLR IPFIX Seconds MSb timer stamp upload */
    return prvCpssHwPpReadRegister(devNum, regAddr, &(uploadPtr->timer.secondTimer.l[1]));
}

/**
* @internal cpssDxChIpfixTimestampUploadGet function
* @endinternal
*
* @brief   Gets IPFIX timestamp upload configuration for a stage.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] uploadPtr                - pointer to timestamp upload configuration of a stage.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixTimestampUploadGet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    OUT CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC   *uploadPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixTimestampUploadGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, uploadPtr));

    rc = internal_cpssDxChIpfixTimestampUploadGet(devNum, stage, uploadPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, uploadPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpfixTimestampUploadTrigger function
* @endinternal
*
* @brief   Triggers the IPFIX timestamp upload operation.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_BAD_STATE             - if previous upload operation is not
*                                       finished yet.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note CPSS_DXCH_POLICER_STAGE_INGRESS_0_E triggering controls
*       CPSS_DXCH_POLICER_STAGE_INGRESS_1 & CPSS_DXCH_POLICER_STAGE_EGRESS_E
*       as well, all uploads are done simultenously.
*
*/
static GT_STATUS internal_cpssDxChIpfixTimestampUploadTrigger
(
    IN GT_U8    devNum
)
{
    GT_U32      regAddr;            /* register address */
    GT_BOOL     prevUploadStatus;   /* previous upload state */
    GT_STATUS   rc;                 /* return code  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);

    /* Read triggering field to check if previous operation already done.    */
    rc = cpssDxChIpfixTimestampUploadStatusGet(devNum, &prevUploadStatus);
    if (GT_OK != rc)
    {
        return rc;
    }

    if ( GT_FALSE == prevUploadStatus)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* trigger the upload */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,CPSS_DXCH_POLICER_STAGE_INGRESS_0_E).IPFIXCtrl;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        PLR[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E].ipfixControl;
    }

    /* Set IPFIX Control register, <time stamp trig> field */
    return prvCpssHwPpSetRegField(devNum, regAddr, 6, 1, 1);
}

/**
* @internal cpssDxChIpfixTimestampUploadTrigger function
* @endinternal
*
* @brief   Triggers the IPFIX timestamp upload operation.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_BAD_STATE             - if previous upload operation is not
*                                       finished yet.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note CPSS_DXCH_POLICER_STAGE_INGRESS_0_E triggering controls
*       CPSS_DXCH_POLICER_STAGE_INGRESS_1 & CPSS_DXCH_POLICER_STAGE_EGRESS_E
*       as well, all uploads are done simultenously.
*
*/
GT_STATUS cpssDxChIpfixTimestampUploadTrigger
(
    IN GT_U8    devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixTimestampUploadTrigger);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChIpfixTimestampUploadTrigger(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpfixTimestampUploadStatusGet function
* @endinternal
*
* @brief   Gets IPFIX timestamp upload operation status.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] uploadStatusPtr          - pointer to indication whether upload was done.
*                                      GT_TRUE: upload is finished.
*                                      GT_FALSE: upload still in progess.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChIpfixTimestampUploadStatusGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL *uploadStatusPtr
)
{
    GT_U32      regAddr;        /* register address */
    GT_STATUS   rc;             /* return code  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(uploadStatusPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,CPSS_DXCH_POLICER_STAGE_INGRESS_0_E).IPFIXCtrl;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        PLR[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E].ipfixControl;
    }

    /* check that all port groups are ready */
    rc = prvCpssPortGroupBusyWait(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                regAddr,6,
                GT_TRUE);/* only to check the bit --> no 'busy wait' */
    if(rc == GT_BAD_STATE)
    {
        rc = GT_OK;
        /* not all port groups has the bit with value 0 */
        *uploadStatusPtr = GT_FALSE;
    }
    else
    {
        *uploadStatusPtr = GT_TRUE;
    }

    return rc;
}

/**
* @internal cpssDxChIpfixTimestampUploadStatusGet function
* @endinternal
*
* @brief   Gets IPFIX timestamp upload operation status.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] uploadStatusPtr          - pointer to indication whether upload was done.
*                                      GT_TRUE: upload is finished.
*                                      GT_FALSE: upload still in progess.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixTimestampUploadStatusGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL *uploadStatusPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixTimestampUploadStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, uploadStatusPtr));

    rc = internal_cpssDxChIpfixTimestampUploadStatusGet(devNum, uploadStatusPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, uploadStatusPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpfixTimerGet function
* @endinternal
*
* @brief   Gets IPFIX timer
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] timerPtr                 - pointer to IPFIX timer.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChIpfixTimerGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT CPSS_DXCH_IPFIX_TIMER_STC       *timerPtr
)
{
    GT_U32      regAddr;        /* register address */
    GT_STATUS   rc;             /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(timerPtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerTimerTbl;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                            PLR[stage].policerTimerTbl;
    }

    /* Get IPFIX nano timer */
    rc = prvCpssHwPpGetRegField(devNum,
                                   regAddr + (PRV_CPSS_DXCH_IPFIX_NANO_TIMER_CNS * 4),
                                   0, 30, &(timerPtr->nanoSecondTimer));
    if (GT_OK != rc)
    {
        return rc;
    }

    /* Get IPFIX second LSb timer */
    rc = prvCpssHwPpReadRegister(devNum,
                                   regAddr + (PRV_CPSS_DXCH_IPFIX_SECOND_LSB_TIMER_CNS * 4),
                                   &(timerPtr->secondTimer.l[0]));
    if (GT_OK != rc)
    {
        return rc;
    }

    /* Get IPFIX second MSb timer */
    rc = prvCpssHwPpReadRegister(devNum,
                                   regAddr + (PRV_CPSS_DXCH_IPFIX_SECOND_MSB_TIMER_CNS * 4),
                                   &(timerPtr->secondTimer.l[1]));
    return GT_OK;
}

/**
* @internal cpssDxChIpfixTimerGet function
* @endinternal
*
* @brief   Gets IPFIX timer
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] timerPtr                 - pointer to IPFIX timer.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixTimerGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT CPSS_DXCH_IPFIX_TIMER_STC       *timerPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixTimerGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, timerPtr));

    rc = internal_cpssDxChIpfixTimerGet(devNum, stage, timerPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, timerPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpfixAlarmEventsGet function
* @endinternal
*
* @brief   Retrieves up to 16 IPFIX entries indexes where alarm events (due to
*         sampling) occured.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] eventsArr[CPSS_DXCH_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS] - array of alarm events, each valid element contains
*                                      the index of IPFIX entry which caused the alarm.
* @param[out] eventsNumPtr             - The number of valid entries in eventsPtr list.
*                                      (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChIpfixAlarmEventsGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT GT_U32                          eventsArr[CPSS_DXCH_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS],
    OUT GT_U32                          *eventsNumPtr
)
{
    return cpssDxChIpfixPortGroupAlarmEventsGet(devNum,
                                                CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                stage,
                                                eventsArr,
                                                eventsNumPtr);
}

/**
* @internal cpssDxChIpfixAlarmEventsGet function
* @endinternal
*
* @brief   Retrieves up to 16 IPFIX entries indexes where alarm events (due to
*         sampling) occured.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] eventsArr[CPSS_DXCH_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS] - array of alarm events, each valid element contains
*                                      the index of IPFIX entry which caused the alarm.
* @param[out] eventsNumPtr             - The number of valid entries in eventsPtr list.
*                                      (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixAlarmEventsGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT GT_U32                          eventsArr[CPSS_DXCH_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS],
    OUT GT_U32                          *eventsNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixAlarmEventsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, eventsArr, eventsNumPtr));

    rc = internal_cpssDxChIpfixAlarmEventsGet(devNum, stage, eventsArr, eventsNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, eventsArr, eventsNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpfixPortGroupEntrySet function
* @endinternal
*
* @brief   Sets IPFIX entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - index of IPFIX Entry.
*                                      Range: see datasheet for specific device.
* @param[in] ipfixEntryPtr            - pointer to the IPFIX Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_OUT_OF_RANGE          - on value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In order to set IPFIX entry under traffic perform the following
*       algorithm(none-zero setting of IPFIX Counters while enabled\still
*       under traffic may cause data coherency problems):
*       - disconnect entry from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entry back to traffic
*       This API address the same memory area (counters place) as can be set
*       by "cpssDxCh3PolicerBillingEntrySet".
*
*/
static GT_STATUS internal_cpssDxChIpfixPortGroupEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                              entryIndex,
    IN  CPSS_DXCH_IPFIX_ENTRY_STC           *ipfixEntryPtr
)
{
    GT_STATUS rc;                   /* return code */
    GT_U32    regAddr;                /* register address */
    GT_U32    hwData[8];              /* HW data */
    GT_U32    samplingActionHwVal;    /* HW value of samplingAction */
    GT_U32    randomFlagHwVal;        /* HW value of randomFlag */
    GT_U32    samplingModeHwVal;      /* HW value of samplingMode */
    GT_U32    portGroupId;            /* port group id */
    GT_U32    wordAddr;               /* entry word's address */
    GT_U16    portGroupNum;           /* number of port groups      */
    GT_U16    portGroupIndex;         /* index of port group in BMP */
    GT_U64    averageByteCounter;     /* byte counter value to write into HW */
    GT_U64    packetCounter;          /* packet counter value received into API */
    GT_U64    averagePacketCounter;   /* packet counter value to write into HW */
    GT_U64    dropCounter;            /* drop counter value received into API */
    GT_U64    averageDropCounter;     /* drop counter value to write into HW */
    CPSS_DXCH_TABLE_ENT tableType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(ipfixEntryPtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    /* Check index of Policer Metering Entry */
    PRV_CPSS_DXCH_POLICERS_NUM_CHECK_MAC(devNum,stage,entryIndex);

    /* check params validity */

    if( ipfixEntryPtr->dropCounter >= BIT_30 ||
        ipfixEntryPtr->packetCount >= BIT_30 ||
        ipfixEntryPtr->byteCount.l[1] >= BIT_4 ||
        ipfixEntryPtr->samplingWindow.l[1] >= BIT_4 ||
        ipfixEntryPtr->cpuSubCode >= BIT_2 ||
        ipfixEntryPtr->timeStamp >= BIT_16 ||
        ipfixEntryPtr->lastSampledValue.l[1] >= BIT_4)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }


    switch (ipfixEntryPtr->samplingAction)
    {
        case CPSS_DXCH_IPFIX_SAMPLING_ACTION_ALARM_E:
            samplingActionHwVal = 0;
            break;
        case CPSS_DXCH_IPFIX_SAMPLING_ACTION_MIRROR_E:
            /* On the egress, sampling can only trigger Alarm events for devices before Falcon */
            /* since mirroring to the CPU is supported from the egress pipe for Falcon and new devices. */
            if( !(PRV_CPSS_SIP_6_CHECK_MAC(devNum)) && (stage == CPSS_DXCH_POLICER_STAGE_EGRESS_E))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            samplingActionHwVal = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (ipfixEntryPtr->randomFlag)
    {
        case CPSS_DXCH_IPFIX_SAMPLING_DIST_DETERMINISTIC_E:
            randomFlagHwVal = 0;
            break;
        case CPSS_DXCH_IPFIX_SAMPLING_DIST_RANDOM_E:
            randomFlagHwVal = 1;
            if( ipfixEntryPtr->logSamplingRange >= BIT_6 )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (ipfixEntryPtr->samplingMode)
    {
        case CPSS_DXCH_IPFIX_SAMPLING_MODE_DISABLE_E:
            samplingModeHwVal = 0;
            break;
        case CPSS_DXCH_IPFIX_SAMPLING_MODE_PACKET_E:
            samplingModeHwVal = 1;
            break;
        case CPSS_DXCH_IPFIX_SAMPLING_MODE_BYTE_E:
            samplingModeHwVal = 2;
            break;
        case CPSS_DXCH_IPFIX_SAMPLING_MODE_TIME_E:
            samplingModeHwVal = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* calculate number of port groups */
    rc = prvCpssPortGroupsNumActivePortGroupsInBmpGet(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_IPLR_E,&portGroupNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    packetCounter.l[0] = ipfixEntryPtr->packetCount;
    packetCounter.l[1] = 0;
    dropCounter.l[0] = ipfixEntryPtr->dropCounter;
    dropCounter.l[1] = 0;


    /* Check if Policer Counting memory errata is enabled */
    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                                               PRV_CPSS_DXCH_LION_POLICER_COUNTING_MEM_ACCESS_WA_E))
    {
        /* check that the previous action is finished in each portGroup, do busy wait */
        rc = prvCpssDxCh3PolicerAccessControlBusyWait(devNum, portGroupsBmp, stage);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).
                                                        policerTableAccessData[0];
        }
        else
        {
            /* Get Ingress Policer Table Access Data 0 Register Address */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                            PLR[stage].policerTblAccessDataReg;
        }
    }
    else
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCountingTbl;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                            PLR[stage].policerCountingTbl;
        }

        regAddr += (entryIndex * 0x20);
    }

    if (CPSS_DXCH_POLICER_STAGE_INGRESS_0_E == stage )
    {
        tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E;
    }
    else if (CPSS_DXCH_POLICER_STAGE_INGRESS_1_E == stage )
    {
        tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E;
    }
    else /* CPSS_DXCH_POLICER_STAGE_EGRESS_E */
    {
        tableType = CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_COUNTING_E;
    }

    /* loop on all port groups to set the counter value */
    portGroupIndex = 0;
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        /* Nullify the hwData */
        cpssOsMemSet(hwData, 0, sizeof(hwData));

        /* calculate average value for byte counter */
        prvCpssPortGroupCounterAverageValueGet(portGroupNum, portGroupIndex,
                                                   ipfixEntryPtr->byteCount,
                                                   &averageByteCounter);

        /* calculate average value for packet counter */
        prvCpssPortGroupCounterAverageValueGet(portGroupNum, portGroupIndex,
                                                   packetCounter,
                                                   &averagePacketCounter);

        /* calculate average value for drop counter */
        prvCpssPortGroupCounterAverageValueGet(portGroupNum, portGroupIndex,
                                                   dropCounter,
                                                   &averageDropCounter);

        /************************/
        /* Create HW entry data */
        /************************/
        hwData[0] = averageByteCounter.l[0];
        hwData[1] = averageByteCounter.l[1] |
                    ((averagePacketCounter.l[0] & 0xFFFFFFF) << 4);
        hwData[2] = (averagePacketCounter.l[0] >> 28) |
                    (ipfixEntryPtr->timeStamp << 2) |
                    (samplingModeHwVal << 18) |
                    (randomFlagHwVal << 20) |
                    ((ipfixEntryPtr->logSamplingRange & 0x3F) << 21) |
                    (samplingActionHwVal << 27) |
                    (ipfixEntryPtr->cpuSubCode << 28) |
                    ((ipfixEntryPtr->samplingWindow.l[0] & 0x3) << 30);
        hwData[3] = (ipfixEntryPtr->samplingWindow.l[0]  >> 2) |
                    ((ipfixEntryPtr->samplingWindow.l[1] & 0x3) << 30);
        hwData[4] = (ipfixEntryPtr->samplingWindow.l[1]  >> 2) |
                    ((ipfixEntryPtr->lastSampledValue.l[0] & 0x3FFFFFFF) << 2);
        hwData[5] = (ipfixEntryPtr->lastSampledValue.l[0] >> 30) |
                    (ipfixEntryPtr->lastSampledValue.l[1] << 2) |
                    ((ipfixEntryPtr->randomOffset & 0x3FFFFFF) << 6);
        hwData[6] = (ipfixEntryPtr->randomOffset >> 26) |
                    ((averageDropCounter.l[0] & 0x3FFFFFF) << 6);
        hwData[7] = (averageDropCounter.l[0] >> 26);

        /* In the last word (8), 5th bit shouild be set as Counter mode is IPFIX */
        hwData[7] |= ( 1 << 4);


        portGroupIndex++;

        /* update HW table shadow (if supported and requested) */
        rc = prvCpssDxChPortGroupShadowEntryWrite(devNum, portGroupId,
                                                  tableType,
                                                  entryIndex, hwData);
        if (rc != GT_OK)
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "update hw table shadow failed");
        }

        /* Check if Policer Counting memory errata is enabled */
         if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                  PRV_CPSS_DXCH_LION_POLICER_COUNTING_MEM_ACCESS_WA_E))
         {
             /* Indirect access to counting memory should be used */
             rc = prvCpssHwPpPortGroupWriteRam(
                 devNum, portGroupId, regAddr, 8/* words*/, hwData);
             if (rc != GT_OK)
             {
                 return rc;
             }

             rc = prvCpssDxCh3PolicerInternalTableAccess(devNum,
                                 portGroupId,
                                 stage,
                                 entryIndex,
                                 PRV_CPSS_DXCH3_POLICER_CNTR_READ_AND_WRITE_E);
             if (rc != GT_OK)
             {
                 return rc;
             }

         }
         else
         {
             /* set first 7 words of IPFIX entry */
             rc = prvCpssHwPpPortGroupWriteRam(
                 devNum, portGroupId, regAddr, 7/* words*/, hwData);
             if (rc != GT_OK)
             {
                 return rc;
             }

             /* verify that all words were written before write last one.
                because last one triggers write of whole entry */
             GT_SYNC;

             /* set last word */
             wordAddr = regAddr + 0x1C;
             rc = prvCpssHwPpPortGroupWriteRegister(devNum, portGroupId, wordAddr, hwData[7]);
             if (rc != GT_OK)
             {
                 return rc;
             }
         }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChIpfixPortGroupEntrySet function
* @endinternal
*
* @brief   Sets IPFIX entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - index of IPFIX Entry.
*                                      Range: see datasheet for specific device.
* @param[in] ipfixEntryPtr            - pointer to the IPFIX Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_OUT_OF_RANGE          - on value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In order to set IPFIX entry under traffic perform the following
*       algorithm(none-zero setting of IPFIX Counters while enabled\still
*       under traffic may cause data coherency problems):
*       - disconnect entry from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entry back to traffic
*       This API address the same memory area (counters place) as can be set
*       by "cpssDxCh3PolicerBillingEntrySet".
*
*/
GT_STATUS cpssDxChIpfixPortGroupEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                              entryIndex,
    IN  CPSS_DXCH_IPFIX_ENTRY_STC           *ipfixEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixPortGroupEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, stage, entryIndex, ipfixEntryPtr));

    rc = internal_cpssDxChIpfixPortGroupEntrySet(devNum, portGroupsBmp, stage, entryIndex, ipfixEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, stage, entryIndex, ipfixEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpfixPortGroupEntryGet function
* @endinternal
*
* @brief   Gets IPFIX entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - index of IPFIX Entry.
*                                      Range: see datasheet for specific device.
* @param[in] reset                    -  flag (please refer to COMMENTS section for further
*                                      explanations):
*                                      GT_TRUE  - performing read and reset atomic operation.
*                                      GT_FALSE - performing read entry operation only.
*
* @param[out] ipfixEntryPtr            - pointer to the IPFIX Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_BAD_STATE             - on counter entry type mismatch.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note When counters are read in clear on read mode (reset == GT_TRUE), The
*       following actions are done on the different IPFIX entry fields:
*       - The <Packet Counter>, <Byte Counter>, and <Dropped Packet Counter>
*       fields are cleared.
*       - The <Timestamp> field is updated to the current time.
*       - The <Last Sampled Value> is cleared.
*
*/
static GT_STATUS internal_cpssDxChIpfixPortGroupEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                              entryIndex,
    IN  GT_BOOL                             reset,
    OUT CPSS_DXCH_IPFIX_ENTRY_STC           *ipfixEntryPtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value   */
    GT_U32      samplingActionHwVal;    /* HW value of samplingAction */
    GT_U32      randomFlagHwVal;        /* HW value of randomFlag */
    GT_U32      samplingModeHwVal;      /* HW value of samplingMode */
    GT_U32      portGroupId;            /* port group id */
    GT_U32      firstActivePortGroupId; /* first active port group id */
    GT_STATUS   rc;                     /* return code */

    PRV_CPSS_DXCH3_POLICER_ACCESS_CTRL_CMD_ENT  accessAction; /* access action */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(ipfixEntryPtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    /* Check index of Policer Metering Entry */
    PRV_CPSS_DXCH_POLICERS_NUM_CHECK_MAC(devNum,stage,entryIndex);

    /* Define Policer Table Indirect Access triggered action */
    accessAction = (reset == GT_TRUE) ?
                    PRV_CPSS_DXCH3_POLICER_CNTR_READ_AND_RESET_E :
                    PRV_CPSS_DXCH3_POLICER_CNTR_READ_ONLY_E;

    /* loop on all active port groups in the bmp */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        /* Perform indirect access to the Policer Table */
        rc = prvCpssDxCh3PolicerInternalTableAccess(devNum,
                                                    portGroupId,
                                                    stage,
                                                    entryIndex,
                                                    accessAction);
        if (GT_OK != rc)
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    /* Get the first active port group */
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(devNum, portGroupsBmp,
                                                        firstActivePortGroupId);

    /* Get Ingress Policer Table Access Data 0 Register Address */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).
                                                    policerTableAccessData[0];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                            PLR[stage].policerTblAccessDataReg;
    }

    /* Get Ingress Policer Table Access Data 7 Register */
    regAddr += 28;

    rc = prvCpssHwPpPortGroupReadRegister(devNum, firstActivePortGroupId,
                                             regAddr, &regValue);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* Check that counter mode is IPFIX and not Billing */
    if (((regValue >> 4) & 0x1) == 0x0 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPortGroupsBmpCounterSummary(devNum, portGroupsBmp,
                                                regAddr,0,4,
                                                &ipfixEntryPtr->dropCounter,
                                                NULL);
    if (GT_OK != rc)
    {
        return rc;
    }

    ipfixEntryPtr->dropCounter = (ipfixEntryPtr->dropCounter << 26);

    /* Set the register address back to the register 0 */
    regAddr -= 28;

    rc = prvCpssPortGroupsBmpCounterSummary(devNum, portGroupsBmp,
                                                regAddr,0,32,
                                                NULL,
                                                &ipfixEntryPtr->byteCount);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* Get Ingress Policer Table Access Data 1 Register */
    regAddr += 4;

    rc = prvCpssPortGroupsBmpMultiCounterSummary(devNum,
                                                     portGroupsBmp, regAddr,
                                                     0,4,
                                                     &regValue,/*byteCount:4MSbit*/
                                                     4,28,
                                                     &ipfixEntryPtr->packetCount,/*packetCount:28LSbit*/
                                                     0,0,NULL);
    if (GT_OK != rc)
    {
        return rc;
    }

    ipfixEntryPtr->byteCount.l[1] += regValue;

    /* Get Ingress Policer Table Access Data 2 Register */
    regAddr += 4;

    rc = prvCpssPortGroupsBmpCounterSummary(devNum, portGroupsBmp,
                                                regAddr,0,2,
                                                &regValue,
                                                NULL);
    if (GT_OK != rc)
    {
        return rc;
    }

    ipfixEntryPtr->packetCount += (regValue << 28);


    rc = prvCpssHwPpPortGroupReadRegister(devNum, firstActivePortGroupId,
                                             regAddr, &regValue);
    if (GT_OK != rc)
    {
        return rc;
    }

    ipfixEntryPtr->timeStamp = U32_GET_FIELD_MAC(regValue,2,16);

    samplingModeHwVal = U32_GET_FIELD_MAC(regValue,18,2);
    switch(samplingModeHwVal)
    {
        case 0: ipfixEntryPtr->samplingMode = CPSS_DXCH_IPFIX_SAMPLING_MODE_DISABLE_E;
                break;
        case 1: ipfixEntryPtr->samplingMode = CPSS_DXCH_IPFIX_SAMPLING_MODE_PACKET_E;
                break;
        case 2: ipfixEntryPtr->samplingMode = CPSS_DXCH_IPFIX_SAMPLING_MODE_BYTE_E;
                break;
        case 3: ipfixEntryPtr->samplingMode = CPSS_DXCH_IPFIX_SAMPLING_MODE_TIME_E;
                break;
        default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    randomFlagHwVal = U32_GET_FIELD_MAC(regValue,20,1);
    ipfixEntryPtr->randomFlag = (randomFlagHwVal == 0) ?
                                 CPSS_DXCH_IPFIX_SAMPLING_DIST_DETERMINISTIC_E:
                                 CPSS_DXCH_IPFIX_SAMPLING_DIST_RANDOM_E;

    ipfixEntryPtr->logSamplingRange = U32_GET_FIELD_MAC(regValue,21,6);

    samplingActionHwVal = U32_GET_FIELD_MAC(regValue,27,1);
    ipfixEntryPtr->samplingAction = (samplingActionHwVal == 0) ?
                                     CPSS_DXCH_IPFIX_SAMPLING_ACTION_ALARM_E:
                                     CPSS_DXCH_IPFIX_SAMPLING_ACTION_MIRROR_E;

    ipfixEntryPtr->cpuSubCode = U32_GET_FIELD_MAC(regValue,28,2);
    ipfixEntryPtr->samplingWindow.l[0] = U32_GET_FIELD_MAC(regValue,30,2);

    /* loop on all active port groups in the bmp */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        rc = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId,
                                             regAddr, &regValue);
        if (GT_OK != rc)
        {
            return rc;
        }

        /* for timeStamp use the highest value from all port groups */
        if( ipfixEntryPtr->timeStamp < U32_GET_FIELD_MAC(regValue,2,16) )
        {
            ipfixEntryPtr->timeStamp = U32_GET_FIELD_MAC(regValue,2,16);
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    /* Get Ingress Policer Table Access Data 3 Register */
    regAddr += 4;

    rc = prvCpssHwPpPortGroupReadRegister(devNum, firstActivePortGroupId,
                                             regAddr, &regValue);
    if (GT_OK != rc)
    {
        return rc;
    }

    ipfixEntryPtr->samplingWindow.l[0] |= ((regValue & 0x3FFFFFFF) << 2);
    ipfixEntryPtr->samplingWindow.l[1] = U32_GET_FIELD_MAC(regValue,30,2);

    /* Get Ingress Policer Table Access Data 4 Register */
    regAddr += 4;

    rc = prvCpssHwPpPortGroupReadRegister(devNum, firstActivePortGroupId,
                                             regAddr, &regValue);
    if (GT_OK != rc)
    {
        return rc;
    }

    ipfixEntryPtr->samplingWindow.l[1] |= ((regValue & 0x3) << 2);
    ipfixEntryPtr->lastSampledValue.l[0] = U32_GET_FIELD_MAC(regValue,2,30);

    /* Get Ingress Policer Table Access Data 5 Register */
    regAddr += 4;

    rc = prvCpssHwPpPortGroupReadRegister(devNum, firstActivePortGroupId,
                                             regAddr, &regValue);
    if (GT_OK != rc)
    {
        return rc;
    }

    ipfixEntryPtr->lastSampledValue.l[0] |= ((regValue & 0x3) << 30);
    ipfixEntryPtr->lastSampledValue.l[1] = U32_GET_FIELD_MAC(regValue,2,4);
    ipfixEntryPtr->randomOffset = U32_GET_FIELD_MAC(regValue,6,26);

    /* Get Ingress Policer Table Access Data 6 Register */
    regAddr += 4;

    rc = prvCpssHwPpPortGroupReadRegister(devNum, firstActivePortGroupId,
                                             regAddr, &regValue);
    if (GT_OK != rc)
    {
        return rc;
    }

    ipfixEntryPtr->randomOffset |= ((regValue & 0x3F) << 26);

    rc = prvCpssPortGroupsBmpCounterSummary(devNum, portGroupsBmp,
                                                regAddr,6,26,
                                                &regValue,
                                                NULL);
    if (GT_OK != rc)
    {
        return rc;
    }

    ipfixEntryPtr->dropCounter += regValue;

    return GT_OK;
}

/**
* @internal cpssDxChIpfixPortGroupEntryGet function
* @endinternal
*
* @brief   Gets IPFIX entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - index of IPFIX Entry.
*                                      Range: see datasheet for specific device.
* @param[in] reset                    -  flag (please refer to COMMENTS section for further
*                                      explanations):
*                                      GT_TRUE  - performing read and reset atomic operation.
*                                      GT_FALSE - performing read entry operation only.
*
* @param[out] ipfixEntryPtr            - pointer to the IPFIX Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_BAD_STATE             - on counter entry type mismatch.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note When counters are read in clear on read mode (reset == GT_TRUE), The
*       following actions are done on the different IPFIX entry fields:
*       - The <Packet Counter>, <Byte Counter>, and <Dropped Packet Counter>
*       fields are cleared.
*       - The <Timestamp> field is updated to the current time.
*       - The <Last Sampled Value> is cleared.
*
*/
GT_STATUS cpssDxChIpfixPortGroupEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                              entryIndex,
    IN  GT_BOOL                             reset,
    OUT CPSS_DXCH_IPFIX_ENTRY_STC           *ipfixEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixPortGroupEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, stage, entryIndex, reset, ipfixEntryPtr));

    rc = internal_cpssDxChIpfixPortGroupEntryGet(devNum, portGroupsBmp, stage, entryIndex, reset, ipfixEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, stage, entryIndex, reset, ipfixEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpfixPortGroupWraparoundStatusGet function
* @endinternal
*
* @brief   Retrieves a bitmap of wraparound entries indexes.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] startIndex               - the first entry index to get wraparound indication on.
* @param[in] endIndex                 - the last entry index to get wraparound indication on.
* @param[in] reset                    - clear the bits after read:
*                                      GT_TRUE - clear,
*                                      GT_FALSE - don't clear.
*
* @param[out] bmpPtr                   - pointer to a bitmap indication wraparound.
*                                      each bit indicate: 0 - no WA occured, 1 - WA occured.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note It is application responsibility to allocate the buffer size for the
*       bitmap(bmpPtr). Since the buffer is made from GT_U32 elements, each
*       element contains 32 indications.
*
*/
static GT_STATUS internal_cpssDxChIpfixPortGroupWraparoundStatusGet
(
    IN GT_U8                            devNum,
    IN GT_PORT_GROUPS_BMP               portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32                           startIndex,
    IN GT_U32                           endIndex,
    IN GT_BOOL                          reset,
    OUT GT_U32                          *bmpPtr
)
{
    GT_U32 regAddr; /* ipfix wraparound alert table base address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(bmpPtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    /* Check index of Policer Metering Entries */
    PRV_CPSS_DXCH_POLICERS_NUM_CHECK_MAC(devNum,stage,startIndex);
    PRV_CPSS_DXCH_POLICERS_NUM_CHECK_MAC(devNum,stage,endIndex);

    if( startIndex > endIndex )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).ipfixWrapAroundAlertTbl;

        /* Bobcat2 A0 supports only 2K indications */
        if ((endIndex >= _2K) && (!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                            PLR[stage].ipfixWrapAroundAlertTbl;
    }

    return prvCpssDxChIpfixGetBitmap(devNum, portGroupsBmp, startIndex, endIndex,
                                     regAddr, reset, bmpPtr);
}

/**
* @internal cpssDxChIpfixPortGroupWraparoundStatusGet function
* @endinternal
*
* @brief   Retrieves a bitmap of wraparound entries indexes.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] startIndex               - the first entry index to get wraparound indication on.
* @param[in] endIndex                 - the last entry index to get wraparound indication on.
* @param[in] reset                    - clear the bits after read:
*                                      GT_TRUE - clear,
*                                      GT_FALSE - don't clear.
*
* @param[out] bmpPtr                   - pointer to a bitmap indication wraparound.
*                                      each bit indicate: 0 - no WA occured, 1 - WA occured.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note It is application responsibility to allocate the buffer size for the
*       bitmap(bmpPtr). Since the buffer is made from GT_U32 elements, each
*       element contains 32 indications.
*
*/
GT_STATUS cpssDxChIpfixPortGroupWraparoundStatusGet
(
    IN GT_U8                            devNum,
    IN GT_PORT_GROUPS_BMP               portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32                           startIndex,
    IN GT_U32                           endIndex,
    IN GT_BOOL                          reset,
    OUT GT_U32                          *bmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixPortGroupWraparoundStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, stage, startIndex, endIndex, reset, bmpPtr));

    rc = internal_cpssDxChIpfixPortGroupWraparoundStatusGet(devNum, portGroupsBmp, stage, startIndex, endIndex, reset, bmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, stage, startIndex, endIndex, reset, bmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpfixPortGroupAgingStatusGet function
* @endinternal
*
* @brief   Retrieves a bitmap of aged entries indexes.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] startIndex               - the first entry index to get aging indication on.
* @param[in] endIndex                 - the last entry index to get aging indication on.
* @param[in] reset                    - clear the bits after read:
*                                      GT_TRUE - clear,
*                                      GT_FALSE - don't clear.
*
* @param[out] bmpPtr                   - pointer to a bitmap indication aging.
*                                      each bit indicate: 0 - no packet recieved on flow (aged).
*                                      1 - packet recieved on flow.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note It is application responsibility to allocate the buffer size for the
*       bitmap(bmpPtr). Since the buffer is made from GT_U32 elements, each
*       element contains 32 indications.
*
*/
static GT_STATUS internal_cpssDxChIpfixPortGroupAgingStatusGet
(
    IN GT_U8                            devNum,
    IN GT_PORT_GROUPS_BMP               portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32                           startIndex,
    IN GT_U32                           endIndex,
    IN GT_BOOL                          reset,
    OUT GT_U32                          *bmpPtr
)
{
    GT_U32 regAddr; /* ipfix aging alert table base address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(bmpPtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    /* Check index of Policer Metering Entries */
    PRV_CPSS_DXCH_POLICERS_NUM_CHECK_MAC(devNum,stage,startIndex);
    PRV_CPSS_DXCH_POLICERS_NUM_CHECK_MAC(devNum,stage,endIndex);

    if( startIndex > endIndex )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).ipfixAgingAlertTbl;

        /* Bobcat2 A0 supports only 2K indications */
        if ((endIndex >= _2K) && (!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                            PLR[stage].ipfixAgingAlertTbl;
    }

    return prvCpssDxChIpfixGetBitmap(devNum, portGroupsBmp, startIndex, endIndex,
                                     regAddr, reset, bmpPtr);
}

/**
* @internal cpssDxChIpfixPortGroupAgingStatusGet function
* @endinternal
*
* @brief   Retrieves a bitmap of aged entries indexes.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] startIndex               - the first entry index to get aging indication on.
* @param[in] endIndex                 - the last entry index to get aging indication on.
* @param[in] reset                    - clear the bits after read:
*                                      GT_TRUE - clear,
*                                      GT_FALSE - don't clear.
*
* @param[out] bmpPtr                   - pointer to a bitmap indication aging.
*                                      each bit indicate: 0 - no packet recieved on flow (aged).
*                                      1 - packet recieved on flow.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note It is application responsibility to allocate the buffer size for the
*       bitmap(bmpPtr). Since the buffer is made from GT_U32 elements, each
*       element contains 32 indications.
*
*/
GT_STATUS cpssDxChIpfixPortGroupAgingStatusGet
(
    IN GT_U8                            devNum,
    IN GT_PORT_GROUPS_BMP               portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32                           startIndex,
    IN GT_U32                           endIndex,
    IN GT_BOOL                          reset,
    OUT GT_U32                          *bmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixPortGroupAgingStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, stage, startIndex, endIndex, reset, bmpPtr));

    rc = internal_cpssDxChIpfixPortGroupAgingStatusGet(devNum, portGroupsBmp, stage, startIndex, endIndex, reset, bmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, stage, startIndex, endIndex, reset, bmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpfixPortGroupAlarmEventsGet function
* @endinternal
*
* @brief   Retrieves up to 16 IPFIX entries indexes where alarm events (due to
*         sampling) occured.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] eventsArr[CPSS_DXCH_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS] - array of alarm events, each valid element contains
*                                      the index of IPFIX entry which caused the alarm.
* @param[out] eventsNumPtr             - The number of valid entries in eventsPtr list.
*                                      (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChIpfixPortGroupAlarmEventsGet
(
    IN GT_U8                            devNum,
    IN GT_PORT_GROUPS_BMP               portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT GT_U32                          eventsArr[CPSS_DXCH_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS],
    OUT GT_U32                          *eventsNumPtr
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      regData;        /* register address */
    GT_STATUS   rc;             /* return code */
    GT_U32      portGroupId;    /* the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(eventsArr);
    CPSS_NULL_PTR_CHECK_MAC(eventsNumPtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    *eventsNumPtr = 0;

    /* loop on all active port groups in the bmp */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).IPFIXSampleEntriesLog1;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                            PLR[stage].ipfixSampleEntriesLog1;
        }

        rc = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId, regAddr, &regData);
        if (GT_OK != rc)
        {
            return rc;
        }

        if( U32_GET_FIELD_MAC(regData,15,1) == 0x1 )
        {
            eventsArr[*eventsNumPtr] = U32_GET_FIELD_MAC(regData,0,15);
            (*eventsNumPtr)++;
        }

        if( U32_GET_FIELD_MAC(regData,31,1) == 0x1 )
        {
            eventsArr[*eventsNumPtr] = U32_GET_FIELD_MAC(regData,16,15);
            (*eventsNumPtr)++;
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).IPFIXSampleEntriesLog0;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                            PLR[stage].ipfixSampleEntriesLog0;
        }

        rc = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId, regAddr, &regData);
        if (GT_OK != rc)
        {
            return rc;
        }

        if( U32_GET_FIELD_MAC(regData,15,1) == 0x1 )
        {
            eventsArr[*eventsNumPtr] = U32_GET_FIELD_MAC(regData,0,15);
            (*eventsNumPtr)++;
        }

        if( U32_GET_FIELD_MAC(regData,31,1) == 0x1 )
        {
            eventsArr[*eventsNumPtr] = U32_GET_FIELD_MAC(regData,16,15);
            (*eventsNumPtr)++;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    return GT_OK;
}

/**
* @internal cpssDxChIpfixPortGroupAlarmEventsGet function
* @endinternal
*
* @brief   Retrieves up to 16 IPFIX entries indexes where alarm events (due to
*         sampling) occured.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] eventsArr[CPSS_DXCH_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS] - array of alarm events, each valid element contains
*                                      the index of IPFIX entry which caused the alarm.
* @param[out] eventsNumPtr             - The number of valid entries in eventsPtr list.
*                                      (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixPortGroupAlarmEventsGet
(
    IN GT_U8                            devNum,
    IN GT_PORT_GROUPS_BMP               portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT GT_U32                          eventsArr[CPSS_DXCH_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS],
    OUT GT_U32                          *eventsNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixPortGroupAlarmEventsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, stage, eventsArr, eventsNumPtr));

    rc = internal_cpssDxChIpfixPortGroupAlarmEventsGet(devNum, portGroupsBmp, stage, eventsArr, eventsNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, stage, eventsArr, eventsNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpfixTimestampToCpuEnableSet function
* @endinternal
*
* @brief   Configures all TO_CPU DSA tags to include a Timestamp.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] enable                   - GT_TRUE: TO_CPU DSA tag contains a Timestamp.
*                                      GT_FALSE: TO_CPU DSA tag does not contain a Timestamp.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChIpfixTimestampToCpuEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      regData;    /* register data */
    GT_U32    startBit;          /* start bit of the field */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);



    regData = ( enable == GT_FALSE ) ? 0 : 1;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).haGlobalConfig;
        startBit = 13;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltGlobalConfig;
        startBit = 12;
    }

    /* Set Header Alteration Global Configuration Register, */
    /* <Enable IPFIX Timestamping To CPU> field.            */
    return prvCpssHwPpSetRegField(devNum, regAddr, startBit, 1, regData);
}

/**
* @internal cpssDxChIpfixTimestampToCpuEnableSet function
* @endinternal
*
* @brief   Configures all TO_CPU DSA tags to include a Timestamp.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] enable                   - GT_TRUE: TO_CPU DSA tag contains a Timestamp.
*                                      GT_FALSE: TO_CPU DSA tag does not contain a Timestamp.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixTimestampToCpuEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixTimestampToCpuEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChIpfixTimestampToCpuEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpfixTimestampToCpuEnableGet function
* @endinternal
*
* @brief   Gets enabling status of TO_CPU DSA tags including a Timestamp.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: TO_CPU DSA tag contains a Timestamp.
*                                      GT_FALSE: TO_CPU DSA tag does not contain a Timestamp.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChIpfixTimestampToCpuEnableGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      regData;    /* register data */
    GT_U32    startBit;          /* start bit of the field */
    GT_STATUS   rc;             /* return code  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).haGlobalConfig;
        startBit = 13;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltGlobalConfig;
        startBit = 12;
    }

    /* Get Header Alteration Global Configuration Register, */
    /* <Enable IPFIX Timestamping To CPU> field.            */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, startBit, 1, &regData);
    if (GT_OK != rc)
    {
        return rc;
    }

    *enablePtr = ( regData == 0 ) ? GT_FALSE : GT_TRUE ;

    return GT_OK;
}

/**
* @internal cpssDxChIpfixTimestampToCpuEnableGet function
* @endinternal
*
* @brief   Gets enabling status of TO_CPU DSA tags including a Timestamp.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: TO_CPU DSA tag contains a Timestamp.
*                                      GT_FALSE: TO_CPU DSA tag does not contain a Timestamp.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixTimestampToCpuEnableGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixTimestampToCpuEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChIpfixTimestampToCpuEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


