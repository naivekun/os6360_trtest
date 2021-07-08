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
* @file cpssDxChCos.c
*
* @brief CPSS DXCh Cos facility implementation.
*
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/generic/cos/cpssCosTypes.h>
#include <cpss/dxCh/dxChxGen/cos/cpssDxChCos.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCosLog.h>

/*******************************************************************************
* Local Macros
*******************************************************************************/

/* check the MAC QoS Table index range */
#define PRV_CPSS_MAX_MAC_QOS_ENTRIES_CHECK_MAC(_index) \
        if (((_index) > 7) || ((_index) == 0)) CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

/* convert modify attribute from SW enum to HW value */
#define PRV_CPSS_ATTR_MODIFY_TO_HW_CNV_MAC(_hwValue, _swValue)    \
    switch(_swValue)                                                           \
    {                                                                          \
        case CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E: _hwValue = 0; break;\
        case CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:        _hwValue = 1; break;\
        case CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:       _hwValue = 2; break;\
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                          \
    }

/**
* @internal internal_cpssDxChCosProfileEntrySet function
* @endinternal
*
* @brief   Configures the Profile Table Entry and
*         Initial Drop Precedence (DP) for Policer.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - index of a profile in the profile table
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..127).
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..1023).
* @param[in] cosPtr                   - Pointer to new CoS values for packet
*                                      (dp = CPSS_DP_YELLOW_E is not applicable).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or cos.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DxCh3 devices Initial DP supports 3 levels (Yellow, Red, Green),
*       but QoS profile entry supports only 2 levels by the following way:
*       - The function set value 0 for Green DP
*       - The function set value 1 for both Yellow and Red DPs.
*
*/
static GT_STATUS internal_cpssDxChCosProfileEntrySet
(
     IN  GT_U8                      devNum,
     IN  GT_U32                     profileIndex,
     IN  CPSS_DXCH_COS_PROFILE_STC  *cosPtr
)
{
    GT_U32      tc;         /* Traffic Class of the Profile */
    GT_U32      up;         /* User Priority of the Profile */
    GT_U32      dp;         /* Drop Precedence of the Profile */
    GT_U32      dscp;       /* dscp of the profile */
    GT_U32      regData;    /* the data to be written/read to/from register */
    GT_U32      regAddr;    /* register address */
    GT_U32      fieldOffset;/* offset of the field in the register */
    GT_U32      regOffset;  /* the offset of the register */
    GT_STATUS   rc;         /* return code */
    CPSS_DXCH_TABLE_ENT   tableType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_QOS_PROFILE_ID_CHECK_MAC(devNum, profileIndex);
    CPSS_NULL_PTR_CHECK_MAC(cosPtr);
    PRV_CPSS_DXCH_COS_CHECK_TX_QUEUES_NUM_MAC(devNum, cosPtr->trafficClass);
    PRV_CPSS_DXCH_COS_CHECK_UP_MAC(cosPtr->userPriority);
    PRV_CPSS_DXCH_COS_CHECK_DSCP_MAC(cosPtr->dscp);

    tc = cosPtr->trafficClass;
    up = cosPtr->userPriority;
    dscp = cosPtr->dscp;

    if (0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        if (0 == PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum))
        {
            PRV_CPSS_DXCH_COS_CHECK_DP_MAC(cosPtr->dropPrecedence);
        }
        else
        {
            /* DxCh3 */
            PRV_CPSS_DXCH3_COS_CHECK_DP_MAC(cosPtr->dropPrecedence);
        }
        dp = (cosPtr->dropPrecedence == CPSS_DP_GREEN_E) ? 0 : 1;
    }
    else
    {
        /* LION */
        PRV_CPSS_DXCH3_COS_DP_CONVERT_MAC(cosPtr->dropPrecedence, dp);
    }

    regData = (tc << 11) | (dp << 9) | (up << 6) | dscp;

    /* writing to the entry to QoS Profile Register */
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                         CPSS_DXCH_TABLE_QOS_PROFILE_E,
                                         profileIndex,
                                         0, /*word num */
                                         0, /* start bit */
                                         14, /* length */
                                         regData);
    if (rc != GT_OK)
    {
        return rc;
    }


    /* write the exp to the QoSProfile to EXP table - DxCh2 and above */
    if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_CHEETAH_E)
    {
        PRV_CPSS_DXCH_COS_CHECK_EXP_MAC(cosPtr->exp);

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            regData = cosPtr->exp;
            rc = prvCpssDxChWriteTableEntry(devNum,
                                             CPSS_DXCH_SIP5_TABLE_HA_QOS_PROFILE_TO_EXP_E,
                                             profileIndex,
                                             &regData);
        }
        else
        {
            /* get register address */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.qosProfile2ExpTable;

            /* adding the offset of the register                    */
            /* Offset Formula: 0x07800300 + Entry<n>*0x4 (0<=n<16)  */
            /* each register contains 8 entries of 3 bits each      */
            regAddr +=  (profileIndex / 8) * 4;

            fieldOffset = (profileIndex % 8) * 3;

            rc = prvCpssHwPpSetRegField(devNum,regAddr,fieldOffset,3,cosPtr->exp);
        }
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* updating <Dp> in Initial DP table */
        PRV_CPSS_DXCH3_COS_DP_CONVERT_MAC(cosPtr->dropPrecedence, dp);
        for(tableType = CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_0_QOS_ATTRIBUTE_TABLE_E;
            tableType <= CPSS_DXCH_SIP6_TABLE_EGRESS_POLICER_QOS_ATTRIBUTE_TABLE_E;
            tableType ++)
        {
            /* updating <InitialDp> field */
            rc = prvCpssDxChWriteTableEntryField(devNum,
                tableType,
                profileIndex,
                PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                3,2,dp);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else
    if (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum))
    {
        /* updating <Dp> in Initial DP table, for Ingress stage #0 */
        PRV_CPSS_DXCH3_COS_DP_CONVERT_MAC(cosPtr->dropPrecedence, dp);
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,0).policerInitialDP[0];
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[0].policerInitialDPReg;
        }
        regOffset = (profileIndex / 16) * 4;
        regAddr += regOffset;
        fieldOffset = (profileIndex % 16) * 2;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 2, dp);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* check if Ingress stage 1, exists */
        if (PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.iplrSecondStageSupported)
        {
            /* updating <Dp> in Initial DP table, for Ingress stage #1 */
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,1).policerInitialDP[0];
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[1].policerInitialDPReg;
            }
            regAddr += regOffset;
            rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 2, dp);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            /* Egress policer initial Drop priority */
                regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(
                    devNum,CPSS_DXCH_POLICER_STAGE_EGRESS_E).policerInitialDP[0];
            regOffset = (profileIndex / 16) * 4;
            regAddr += regOffset;
            fieldOffset = (profileIndex % 16) * 2;
            rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 2, dp);
        }
        else
        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.qosProfile2DpTable;

            /* adding the offset of the register                    */
            /* 0x0F000340 + 4*n: where 0=<n<=7 */
            /* each register contains 16 entries of 2 bits each      */
            regAddr += regOffset;

            rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 2, dp);
        }
    }
    else
    {
        /* updating <Dp> in RemarkTable */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_REMARKING_E,
                                             profileIndex,
                                             0,
                                             7,
                                             1,
                                             dp);
    }

    return rc;
}

/**
* @internal cpssDxChCosProfileEntrySet function
* @endinternal
*
* @brief   Configures the Profile Table Entry and
*         Initial Drop Precedence (DP) for Policer.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - index of a profile in the profile table
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..127).
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..1023).
* @param[in] cosPtr                   - Pointer to new CoS values for packet
*                                      (dp = CPSS_DP_YELLOW_E is not applicable).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or cos.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DxCh3 devices Initial DP supports 3 levels (Yellow, Red, Green),
*       but QoS profile entry supports only 2 levels by the following way:
*       - The function set value 0 for Green DP
*       - The function set value 1 for both Yellow and Red DPs.
*
*/
GT_STATUS cpssDxChCosProfileEntrySet
(
     IN  GT_U8                      devNum,
     IN  GT_U32                     profileIndex,
     IN  CPSS_DXCH_COS_PROFILE_STC  *cosPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosProfileEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileIndex, cosPtr));

    rc = internal_cpssDxChCosProfileEntrySet(devNum, profileIndex, cosPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileIndex, cosPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosProfileEntryGet function
* @endinternal
*
* @brief   Get Profile Table Entry configuration and
*         initial Drop Precedence (DP) for Policer.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - index of a profile in the profile table
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..127).
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..1023).
*
* @param[out] cosPtr                   - Pointer to new CoS values for packet.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or cos.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DxCh3 devices Initial DP supports 3 levels (Yellow, Red, Green),
*       but QoS profile entry supports only 2 levels.
*       Therefore DP value is taken from the Initial DP table.
*
*/
static GT_STATUS internal_cpssDxChCosProfileEntryGet
(
     IN  GT_U8                      devNum,
     IN  GT_U32                     profileIndex,
     OUT CPSS_DXCH_COS_PROFILE_STC  *cosPtr
)
{
    GT_U32      dp;         /* Drop Precedence of the Profile */
    GT_U32      regData;    /* the data to be written/read to/from register */
    GT_U32      regOffset;  /* offset in the register to read data from */
    GT_U32      regAddr;    /* register address */
    GT_STATUS   rc;         /* return code */
    CPSS_DXCH_TABLE_ENT   tableType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_QOS_PROFILE_ID_CHECK_MAC(devNum, profileIndex);
    CPSS_NULL_PTR_CHECK_MAC(cosPtr);

    /* reading the entry to QoS Profile Register */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_TABLE_QOS_PROFILE_E,
                                        profileIndex,
                                        0, /*word num */
                                        0, /* start bit */
                                        14, /* length */
                                        &regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    cosPtr->trafficClass = (regData >> 11) & 0x7; /* 3 bits */

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* NOTE: all the tables with the same value ... so read from IPLR 0*/
        tableType = CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_0_QOS_ATTRIBUTE_TABLE_E;
        /* reading <InitialDp> field */
        rc = prvCpssDxChReadTableEntryField(devNum,
            tableType,
            profileIndex,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            3,2,&dp);
        if (rc != GT_OK)
        {
            return rc;
        }

        PRV_CPSS_DXCH3_COS_DP_TO_SW_CONVERT_MAC(dp, cosPtr->dropPrecedence);
    }
    else
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily >=
        CPSS_PP_FAMILY_CHEETAH3_E)
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,0).policerInitialDP[0];
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[0].policerInitialDPReg;
        }
        regAddr +=  (profileIndex / 16) * 4;
        regOffset = (profileIndex % 16) * 2;
        rc = prvCpssHwPpGetRegField(devNum, regAddr, regOffset, 2, &dp);
        if (rc != GT_OK)
        {
            return rc;
        }

        PRV_CPSS_DXCH3_COS_DP_TO_SW_CONVERT_MAC(dp, cosPtr->dropPrecedence);
    }
    else
    {
        dp = (regData >> 9) & 0x1;                    /* 1 bits */
        cosPtr->dropPrecedence = (dp==0) ? CPSS_DP_GREEN_E : CPSS_DP_RED_E;
    }

    cosPtr->userPriority = (regData >> 6) & 0x7;  /* 3 bits */
    cosPtr->dscp = regData & 0x3F;                /* 6 bits */

    /* read the exp from the QoSProfile to EXP table  - DxCh2 and above */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChReadTableEntry(devNum,
                                         CPSS_DXCH_SIP5_TABLE_HA_QOS_PROFILE_TO_EXP_E,
                                         profileIndex,
                                         &regData);

        cosPtr->exp = regData;
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_CHEETAH_E)
    {
        /* get register address */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.qosProfile2ExpTable;

        /* adding the offset of the register                    */
        /* Offset Formula: 0x07800300 + Entry<n>*0x4 (0<=n<16)  */
        /* each register contains 8 entries of 3 bits each      */
        regAddr +=  (profileIndex / 8) * 4;

        regOffset = (profileIndex % 8) * 3;

        rc = prvCpssHwPpGetRegField(devNum,regAddr,regOffset,3,&cosPtr->exp);
    }
    else
    {
        cosPtr->exp = 0;
    }

    return rc;
}

/**
* @internal cpssDxChCosProfileEntryGet function
* @endinternal
*
* @brief   Get Profile Table Entry configuration and
*         initial Drop Precedence (DP) for Policer.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - index of a profile in the profile table
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..127).
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..1023).
*
* @param[out] cosPtr                   - Pointer to new CoS values for packet.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or cos.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DxCh3 devices Initial DP supports 3 levels (Yellow, Red, Green),
*       but QoS profile entry supports only 2 levels.
*       Therefore DP value is taken from the Initial DP table.
*
*/
GT_STATUS cpssDxChCosProfileEntryGet
(
     IN  GT_U8                      devNum,
     IN  GT_U32                     profileIndex,
     OUT CPSS_DXCH_COS_PROFILE_STC  *cosPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosProfileEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileIndex, cosPtr));

    rc = internal_cpssDxChCosProfileEntryGet(devNum, profileIndex, cosPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileIndex, cosPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosDscpToProfileMapSet function
* @endinternal
*
* @brief   Maps the packet DSCP (or remapped DSCP) to a QoS Profile.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum    - device number.
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
* @param[in] dscp                     - DSCP of a IP packet (APPLICABLE RANGES: 0..63).
* @param[in] profileIndex             - profile index, which is assigned to a IP packet with
*                                      the DSCP on ports with enabled trust L3 or trust L2-L3.
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..127)
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..1023).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, dscp or profileIndex
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosDscpToProfileMapSet
(
    IN GT_U8    devNum,
    IN GT_U32   mappingTableIndex,
    IN GT_U8    dscp,
    IN GT_U32   profileIndex
)
{

    GT_U32      bitOffset;          /* offset in the register */
    GT_U32      regAddr;            /* register address */
    GT_STATUS   rc;                 /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_QOS_PROFILE_ID_CHECK_MAC(devNum, profileIndex);
    PRV_CPSS_DXCH_COS_CHECK_DSCP_MAC(dscp);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_DSCP_TO_QOS_PROFILE_E,
                                            mappingTableIndex,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            10 * dscp,/* start bit */
                                            10, /* num of bits */
                                            profileIndex);
        return rc;
    }

    /* getting the base address */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.qosProfileRegs.dscp2QosProfileMapTable;
    /* adding the offset of the register
       DSCP<4n...4n+3>2QoSProfile Map Table (0<=n<16),
       (16 registers in steps of 0x4) */

    regAddr +=  (dscp / 4) * 4;

    bitOffset = (dscp % 4) * 8;

    rc = prvCpssHwPpSetRegField(devNum, regAddr, bitOffset, 7, profileIndex);

    return rc;
}

/**
* @internal cpssDxChCosDscpToProfileMapSet function
* @endinternal
*
* @brief   Maps the packet DSCP (or remapped DSCP) to a QoS Profile.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum    - device number.
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
* @param[in] dscp                     - DSCP of a IP packet (APPLICABLE RANGES: 0..63).
* @param[in] profileIndex             - profile index, which is assigned to a IP packet with
*                                      the DSCP on ports with enabled trust L3 or trust L2-L3.
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..127)
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..1023).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, dscp or profileIndex
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosDscpToProfileMapSet
(
    IN GT_U8    devNum,
    IN GT_U32   mappingTableIndex,
    IN GT_U8    dscp,
    IN GT_U32   profileIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosDscpToProfileMapSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mappingTableIndex, dscp, profileIndex));

    rc = internal_cpssDxChCosDscpToProfileMapSet(devNum, mappingTableIndex, dscp, profileIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mappingTableIndex, dscp, profileIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosDscpToProfileMapGet function
* @endinternal
*
* @brief   Get the Mapping: packet DSCP (or remapped DSCP) to a QoS Profile.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum    - device number.
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin; AC3X.
* @param[in] dscp                     - DSCP of a IP packet (APPLICABLE RANGES: 0..63)
*
* @param[out] profileIndexPtr          - (pointer to) profile index, which is assigned to
*                                      an IP packet with the DSCP on ports with enabled
*                                      trust L3 or trust L2-L3.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, dscp .
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosDscpToProfileMapGet
(
    IN  GT_U8    devNum,
    IN GT_U32   mappingTableIndex,
    IN  GT_U8    dscp,
    OUT GT_U32  *profileIndexPtr
)
{

    GT_U32      bitOffset;          /* offset in the register */
    GT_U32      hwValue;            /* HW value of field in the register*/
    GT_U32      regAddr;            /* register address */
    GT_STATUS   rc;                 /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_COS_CHECK_DSCP_MAC(dscp);
    CPSS_NULL_PTR_CHECK_MAC(profileIndexPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_DSCP_TO_QOS_PROFILE_E,
                                            mappingTableIndex,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            10 * dscp,/* start bit */
                                            10, /* num of bits */
                                            profileIndexPtr);
        return rc;
    }

    /* getting the base address */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.qosProfileRegs.dscp2QosProfileMapTable;
    /* adding the offset of the register
       DSCP<4n...4n+3>2QoSProfile Map Table (0<=n<16),
       (16 registers in steps of 0x4) */

    regAddr +=  (dscp / 4) * 4;

    bitOffset = (dscp % 4) * 8;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, bitOffset, 7, &hwValue);

    *profileIndexPtr = hwValue;

    return rc;
}

/**
* @internal cpssDxChCosDscpToProfileMapGet function
* @endinternal
*
* @brief   Get the Mapping: packet DSCP (or remapped DSCP) to a QoS Profile.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum    - device number.
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
* @param[in] dscp                     - DSCP of a IP packet (APPLICABLE RANGES: 0..63)
*
* @param[out] profileIndexPtr          - (pointer to) profile index, which is assigned to
*                                      an IP packet with the DSCP on ports with enabled
*                                      trust L3 or trust L2-L3.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, dscp .
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosDscpToProfileMapGet
(
    IN  GT_U8    devNum,
    IN GT_U32   mappingTableIndex,
    IN  GT_U8    dscp,
    OUT GT_U32  *profileIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosDscpToProfileMapGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mappingTableIndex, dscp, profileIndexPtr));

    rc = internal_cpssDxChCosDscpToProfileMapGet(devNum, mappingTableIndex, dscp, profileIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mappingTableIndex, dscp, profileIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosDscpMutationEntrySet function
* @endinternal
*
* @brief   Maps the packet DSCP to a new, mutated DSCP.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - device number.
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin; AC3X.
* @param[in] dscp                     - DSCP of a IP packet (APPLICABLE RANGES: 0..63).
* @param[in] newDscp                  - new DSCP, which is assigned to a IP packet with the DSCP on
*                                      ports with enabled trust L3 or trust L2-L3 and enabled DSCP mutation.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, dscp or newDscp
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosDscpMutationEntrySet
(
    IN GT_U8   devNum,
    IN GT_U32   mappingTableIndex,
    IN GT_U8   dscp,
    IN GT_U8   newDscp
)
{
    GT_U32      bitOffset;      /* offset in the register */
    GT_U32      regAddr;        /* register address */
    GT_STATUS   rc;             /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_COS_CHECK_DSCP_MAC(dscp);
    PRV_CPSS_DXCH_COS_CHECK_DSCP_MAC(newDscp);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_DSCP_TO_DSCP_E,
                                            mappingTableIndex,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            6 * dscp ,/* start bit */
                                            6, /* num of bits */
                                            newDscp);
        return rc;
    }

    /* getting the base address */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.qosProfileRegs.dscp2DscpMapTable;
    /* adding the offset of the register
       DSCP<4n...4n+3>2QoSProfile Map Table (0<=n<16),
       (16 registers in steps of 0x4) */

    regAddr +=  (dscp / 4) * 4;

    bitOffset = (dscp % 4) * 8;

    rc = prvCpssHwPpSetRegField(devNum, regAddr, bitOffset, 6, newDscp);

    return rc;

}

/**
* @internal cpssDxChCosDscpMutationEntrySet function
* @endinternal
*
* @brief   Maps the packet DSCP to a new, mutated DSCP.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - device number.
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin; AC3X.
* @param[in] dscp                     - DSCP of a IP packet (APPLICABLE RANGES: 0..63).
* @param[in] newDscp                  - new DSCP, which is assigned to a IP packet with the DSCP on
*                                      ports with enabled trust L3 or trust L2-L3 and enabled DSCP mutation.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, dscp or newDscp
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosDscpMutationEntrySet
(
    IN GT_U8   devNum,
    IN GT_U32   mappingTableIndex,
    IN GT_U8   dscp,
    IN GT_U8   newDscp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosDscpMutationEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mappingTableIndex, dscp, newDscp));

    rc = internal_cpssDxChCosDscpMutationEntrySet(devNum, mappingTableIndex, dscp, newDscp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mappingTableIndex, dscp, newDscp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosDscpMutationEntryGet function
* @endinternal
*
* @brief   Get the Mapping: packet DSCP to a new, mutated DSCP.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - device number.
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin; AC3X.
* @param[in] dscp                     - DSCP of a IP packet (APPLICABLE RANGES: 0..63)
*
* @param[out] newDscpPtr               - new DSCP, which is assigned to a IP packet with the DSCP on
*                                      ports with enabled trust L3 or trust L2-L3 and enabled DSCP mutation.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, dscp
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosDscpMutationEntryGet
(
    IN  GT_U8   devNum,
    IN GT_U32   mappingTableIndex,
    IN  GT_U8   dscp,
    OUT GT_U8  *newDscpPtr
)
{
    GT_U32      bitOffset;      /* offset in the register */
    GT_U32      regAddr;        /* register address */
    GT_U32      hwValue;        /* HW value of field in the register*/
    GT_STATUS   rc;             /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_COS_CHECK_DSCP_MAC(dscp);
    CPSS_NULL_PTR_CHECK_MAC(newDscpPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_DSCP_TO_DSCP_E,
                                            mappingTableIndex,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            6 * dscp ,/* start bit */
                                            6, /* num of bits */
                                            &hwValue);

        *newDscpPtr = (GT_U8)hwValue;

        return rc;
    }

    /* getting the base address */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.qosProfileRegs.dscp2DscpMapTable;
    /* adding the offset of the register
       DSCP<4n...4n+3>2QoSProfile Map Table (0<=n<16),
       (16 registers in steps of 0x4) */

    regAddr +=  (dscp / 4) * 4;

    bitOffset = (dscp % 4) * 8;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, bitOffset, 6, &hwValue);

    *newDscpPtr = (GT_U8)hwValue;

    return rc;

}

/**
* @internal cpssDxChCosDscpMutationEntryGet function
* @endinternal
*
* @brief   Get the Mapping: packet DSCP to a new, mutated DSCP.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - device number.
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin; AC3X.
* @param[in] dscp                     - DSCP of a IP packet (APPLICABLE RANGES: 0..63)
*
* @param[out] newDscpPtr               - new DSCP, which is assigned to a IP packet with the DSCP on
*                                      ports with enabled trust L3 or trust L2-L3 and enabled DSCP mutation.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, dscp
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosDscpMutationEntryGet
(
    IN  GT_U8   devNum,
    IN GT_U32   mappingTableIndex,
    IN  GT_U8   dscp,
    OUT GT_U8  *newDscpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosDscpMutationEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mappingTableIndex, dscp, newDscpPtr));

    rc = internal_cpssDxChCosDscpMutationEntryGet(devNum, mappingTableIndex, dscp, newDscpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mappingTableIndex, dscp, newDscpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosUpCfiDeiToProfileMapSet function
* @endinternal
*
* @brief   Maps the UP Profile Index and packet's User Priority and CFI/DEI bit (Canonical Format
*         Indicator/Drop Eligibility Indicator) to QoS Profile.
*         The mapping relevant for L2 and L2-L3 QoS Trust modes for 802.1Q
*         or DSA tagged packets.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum     - device number.
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
* @param[in] upProfileIndex           - the UP profile index (table selector)
*                                      (APPLICABLE DEVICES Lion; xCat2; Lion2).
*                                      see also API : cpssDxChCosPortUpProfileIndexSet
* @param[in] up                       - User Priority of a VLAN or DSA tagged packet (APPLICABLE RANGES: 0..7).
* @param[in] cfiDeiBit                - value of CFI/DEI bit (APPLICABLE RANGES: 0..1).
*                                      (APPLICABLE DEVICES DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3).
* @param[in] profileIndex             - QoS Profile index, which is assigned to a VLAN or DSA
*                                      tagged packet with the UP on ports with enabled trust
*                                      L2 or trust L2-L3.
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..127)
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..1023).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, up or cfiDeiBit.
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.)
* @retval GT_OUT_OF_RANGE          - on profileIndex out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosUpCfiDeiToProfileMapSet
(
    IN GT_U8    devNum,
    IN GT_U32   mappingTableIndex,
    IN GT_U32   upProfileIndex,
    IN GT_U8    up,
    IN GT_U8    cfiDeiBit,
    IN GT_U32   profileIndex
)
{
    GT_STATUS   rc;
    GT_U32      bitOffset;          /* offset in the register */
    GT_U32      regAddr;            /* register address */
    GT_U32      cfiStep;        /* cfi parameter step in address */
    GT_U32      upProfileStep;  /* upProfileIndex parameter step in address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_COS_CHECK_UP_MAC(up);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        upProfileIndex = 0;/* not used parameter as the profile is common to 4 QOS tables for 12 profiles */
    }
    else
    /* check profileIndex bit */
    if (PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
    {
        PRV_CPSS_DXCH_CHECK_KEY_UP_PROFILE_INDEX_MAC(upProfileIndex);
    }
    else
    {
        upProfileIndex = 0;/* set parameter so next code will be generic */
    }

    /* Check the QoS profile Index range */
    if(profileIndex >= (GT_U32)PRV_CPSS_DXCH_QOS_PROFILE_MAX_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* check CFI bit */
    if(PRV_CPSS_PP_MAC(devNum)->devFamily >= CPSS_PP_FAMILY_CHEETAH3_E)
    {
        PRV_CPSS_DXCH3_CHECK_CFI_DEI_BIT_MAC(cfiDeiBit);
    }
    else
    {
        cfiDeiBit = 0;/* set parameter so next code will be generic */
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_UP_CFI_TO_QOS_PROFILE_E,
                                            mappingTableIndex,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            10 * ((cfiDeiBit * 8) + up) ,/* start bit */
                                            10, /* num of bits */
                                            profileIndex);
        return rc;
    }

    /* Get the base address of UP2QosProfileMapTable register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        bridgeRegs.qosProfileRegs.up2QosProfileMapTable;

    /* Calculate the actual register address according to the CFI bit value */
    if (PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
    {
        cfiStep = 0x8;
        upProfileStep = 0x10;
    }
    else
    {
        cfiStep = 0x10;
        upProfileStep = 0; /* set parameter so next code will be generic */
    }

    /* consider the cfi parameter */
    regAddr += (cfiDeiBit * cfiStep);
    /* consider the up profile index parameter */
    regAddr += (upProfileIndex * upProfileStep);

    /* Add the UP<4n...4n+3>2QoSProfile Map Table(0<=n<2) register offset,
       2 registers in steps of 0x4 per CFI/DEI bit value
    */
    regAddr +=  (up / 4) * 4;
    bitOffset = (up % 4) * 8;

    /* Set the QoS Profile index */
    return prvCpssHwPpSetRegField(devNum, regAddr, bitOffset, 7, profileIndex);
}

/**
* @internal cpssDxChCosUpCfiDeiToProfileMapSet function
* @endinternal
*
* @brief   Maps the UP Profile Index and packet's User Priority and CFI/DEI bit (Canonical Format
*         Indicator/Drop Eligibility Indicator) to QoS Profile.
*         The mapping relevant for L2 and L2-L3 QoS Trust modes for 802.1Q
*         or DSA tagged packets.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum     - device number.
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
* @param[in] upProfileIndex           - the UP profile index (table selector)
*                                      (APPLICABLE DEVICES Lion; xCat2; Lion2).
*                                      see also API : cpssDxChCosPortUpProfileIndexSet
* @param[in] up                       - User Priority of a VLAN or DSA tagged packet (APPLICABLE RANGES: 0..7).
* @param[in] cfiDeiBit                - value of CFI/DEI bit (APPLICABLE RANGES: 0..1).
*                                      (APPLICABLE DEVICES DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3).
* @param[in] profileIndex             - QoS Profile index, which is assigned to a VLAN or DSA
*                                      tagged packet with the UP on ports with enabled trust
*                                      L2 or trust L2-L3.
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..127)
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..1023).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, up or cfiDeiBit.
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.)
* @retval GT_OUT_OF_RANGE          - on profileIndex out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosUpCfiDeiToProfileMapSet
(
    IN GT_U8    devNum,
    IN GT_U32   mappingTableIndex,
    IN GT_U32   upProfileIndex,
    IN GT_U8    up,
    IN GT_U8    cfiDeiBit,
    IN GT_U32   profileIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosUpCfiDeiToProfileMapSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mappingTableIndex, upProfileIndex, up, cfiDeiBit, profileIndex));

    rc = internal_cpssDxChCosUpCfiDeiToProfileMapSet(devNum, mappingTableIndex, upProfileIndex, up, cfiDeiBit, profileIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mappingTableIndex, upProfileIndex, up, cfiDeiBit, profileIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosUpCfiDeiToProfileMapGet function
* @endinternal
*
* @brief   Get the Map from UP Profile Index and packet's User Priority and CFI/DEI bit (Canonical Format
*         Indicator/Drop Eligibility Indicator) to QoS Profile.
*         The mapping relevant for L2 and L2-L3 QoS Trust modes for 802.1Q
*         or DSA tagged packets.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum     - device number.
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
* @param[in] upProfileIndex           - the UP profile index (table selector)
*                                      (APPLICABLE DEVICES Lion; xCat2; Lion2).
*                                      see also API : cpssDxChCosPortUpProfileIndexSet
* @param[in] up                       - User Priority of a VLAN or DSA tagged packet (APPLICABLE RANGES: 0..7).
* @param[in] cfiDeiBit                - value of CFI/DEI bit (APPLICABLE RANGES: 0..1).
*                                      (APPLICABLE DEVICES DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3).
*
* @param[out] profileIndexPtr          - (pointer to)QoS Profile index, which is assigned to a
*                                      VLAN or DSA tagged packet with the UP on ports with
*                                      enabled trust L2 or trust L2-L3.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, up or cfiDeiBit.
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.)
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosUpCfiDeiToProfileMapGet
(
    IN  GT_U8   devNum,
    IN GT_U32   mappingTableIndex,
    IN GT_U32   upProfileIndex,
    IN  GT_U8   up,
    IN  GT_U8   cfiDeiBit,
    OUT GT_U32 *profileIndexPtr
)
{
    GT_STATUS   rc ;            /* return value */
    GT_U32      bitOffset;      /* offset in the register */
    GT_U32      regAddr;        /* register address */
    GT_U32      cfiStep;        /* cfi parameter step in address */
    GT_U32      upProfileStep;  /* upProfileIndex parameter step in address */
    GT_U32      hwValue;        /* hw value read from register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_COS_CHECK_UP_MAC(up);
    CPSS_NULL_PTR_CHECK_MAC(profileIndexPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        upProfileIndex = 0;/* not used parameter as the profile is common to 4 QOS tables for 12 profiles */
    }
    else
    /* check profileIndex bit */
    if (PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
    {
        PRV_CPSS_DXCH_CHECK_KEY_UP_PROFILE_INDEX_MAC(upProfileIndex);
    }
    else
    {
        upProfileIndex = 0;/* set parameter so next code will be generic */
    }

    /* check CFI bit */
    if(PRV_CPSS_PP_MAC(devNum)->devFamily >= CPSS_PP_FAMILY_CHEETAH3_E)
    {
        PRV_CPSS_DXCH3_CHECK_CFI_DEI_BIT_MAC(cfiDeiBit);
    }
    else
    {
        cfiDeiBit = 0;/* set parameter so next code will be generic */
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_UP_CFI_TO_QOS_PROFILE_E,
                                            mappingTableIndex,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            10 * ((cfiDeiBit * 8) + up) ,/* start bit */
                                            10, /* num of bits */
                                            profileIndexPtr);
        return rc;
    }

    /* Get the base address of UP2QosProfileMapTable register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        bridgeRegs.qosProfileRegs.up2QosProfileMapTable;

    /* Calculate the actual register address according to the CFI bit value */
    if (PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
    {
        cfiStep = 0x8;
        upProfileStep = 0x10;
    }
    else
    {
        cfiStep = 0x10;
        upProfileStep = 0; /* set parameter so next code will be generic */
    }

    /* consider the cfi parameter */
    regAddr += (cfiDeiBit * cfiStep);
    /* consider the up profile index parameter */
    regAddr += (upProfileIndex * upProfileStep);


    /* Add the UP<4n...4n+3>2QoSProfile Map Table(0<=n<2) register offset,
       2 registers in steps of 0x4 per CFI/DEI bit value
    */
    regAddr +=  (up / 4) * 4;
    bitOffset = (up % 4) * 8;

    /* Get the QoS Profile index */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, bitOffset, 7, &hwValue);
    *profileIndexPtr = hwValue;

    return rc;
}

/**
* @internal cpssDxChCosUpCfiDeiToProfileMapGet function
* @endinternal
*
* @brief   Get the Map from UP Profile Index and packet's User Priority and CFI/DEI bit (Canonical Format
*         Indicator/Drop Eligibility Indicator) to QoS Profile.
*         The mapping relevant for L2 and L2-L3 QoS Trust modes for 802.1Q
*         or DSA tagged packets.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum     - device number.
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
* @param[in] upProfileIndex           - the UP profile index (table selector)
*                                      (APPLICABLE DEVICES Lion; xCat2; Lion2).
*                                      see also API : cpssDxChCosPortUpProfileIndexSet
* @param[in] up                       - User Priority of a VLAN or DSA tagged packet (APPLICABLE RANGES: 0..7).
* @param[in] cfiDeiBit                - value of CFI/DEI bit (APPLICABLE RANGES: 0..1).
*                                      (APPLICABLE DEVICES DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3).
*
* @param[out] profileIndexPtr          - (pointer to)QoS Profile index, which is assigned to a
*                                      VLAN or DSA tagged packet with the UP on ports with
*                                      enabled trust L2 or trust L2-L3.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, up or cfiDeiBit.
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.)
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosUpCfiDeiToProfileMapGet
(
    IN  GT_U8   devNum,
    IN GT_U32   mappingTableIndex,
    IN GT_U32   upProfileIndex,
    IN  GT_U8   up,
    IN  GT_U8   cfiDeiBit,
    OUT GT_U32 *profileIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosUpCfiDeiToProfileMapGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mappingTableIndex, upProfileIndex, up, cfiDeiBit, profileIndexPtr));

    rc = internal_cpssDxChCosUpCfiDeiToProfileMapGet(devNum, mappingTableIndex, upProfileIndex, up, cfiDeiBit, profileIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mappingTableIndex, upProfileIndex, up, cfiDeiBit, profileIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosPortDpToCfiDeiMapEnableSet function
* @endinternal
*
* @brief   Enables or disables mapping of Drop Precedence to Drop Eligibility
*         Indicator bit.
*         When enabled on egress port the DEI(CFI) bit at the outgoing VLAN
*         tag is set according to the packet DP(after mapping).
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number including CPU port.
* @param[in] enable                   - enable/disable DP-to-CFI/DEI mapping on the-to-CFI/DEI mapping on the
*                                      egress port:
*                                      GT_TRUE  - DP-to-CFI/DEI mapping enable on port.
*                                      GT_FALSE - DP-to-CFI/DEI mapping disable on port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortDpToCfiDeiMapEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_U32      regAddr;  /* the register address */
    GT_U32      regData;  /* data to be written to the register */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;/* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    regData = (enable == GT_TRUE) ? 1 : 0;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        return prvCpssDxChWriteTableEntryField(devNum,
                                               CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                               portNum,
                                               PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                               SIP5_HA_EPORT_TABLE_1_FIELDS_DP_TO_CFI_MAP_ENABLE_E, /* field name */
                                               PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                               regData);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
        {
            /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
            portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
            localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

            /* Get the address of DP2CFI Enable Per Port register */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                egrTxQConf.txQueueDpToCfiPerPortEnReg;

            /* Remap CPU Port Number to the offset in the register */
            if(CPSS_CPU_PORT_NUM_CNS == localPort)
            {
                localPort = 31;
            }

            /* Enable/Disables DP mapping to CFI/DEI value for current port */
            return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, localPort, 1, regData);
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.
                    distributor.dpToCfiRemapEn[OFFSET_TO_WORD_MAC(portNum)];

            return prvCpssHwPpSetRegField(devNum, regAddr,
                    OFFSET_TO_BIT_MAC(portNum) , 1, regData);
        }
    }
}

/**
* @internal cpssDxChCosPortDpToCfiDeiMapEnableSet function
* @endinternal
*
* @brief   Enables or disables mapping of Drop Precedence to Drop Eligibility
*         Indicator bit.
*         When enabled on egress port the DEI(CFI) bit at the outgoing VLAN
*         tag is set according to the packet DP(after mapping).
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number including CPU port.
* @param[in] enable                   - enable/disable DP-to-CFI/DEI mapping on the-to-CFI/DEI mapping on the
*                                      egress port:
*                                      GT_TRUE  - DP-to-CFI/DEI mapping enable on port.
*                                      GT_FALSE - DP-to-CFI/DEI mapping disable on port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortDpToCfiDeiMapEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortDpToCfiDeiMapEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChCosPortDpToCfiDeiMapEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosPortDpToCfiDeiMapEnableGet function
* @endinternal
*
* @brief   Gets status (Enabled/Disabled) of Drop Precedence mapping to
*         Drop Eligibility Indicator bit.
*         When enabled on egress port the DEI(CFI) bit at the outgoing VLAN
*         tag is set according to the packet DP(after mapping).
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number including CPU port.
*
* @param[out] enablePtr                - (pointer to) value of DP-to-CFI/DEI mapping on the
*                                      egress port (enable/disable):
*                                      GT_TRUE  - DP-to-CFI/DEI mapping enable on port.
*                                      GT_FALSE - DP-to-CFI/DEI mapping disable on port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortDpToCfiDeiMapEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32      regAddr;  /* the register address */
    GT_U32      hwValue;  /* HW value of field in the register*/
    GT_STATUS   rc;       /* return code */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;/* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_HA_EPORT_TABLE_1_FIELDS_DP_TO_CFI_MAP_ENABLE_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            &hwValue);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
        {
            /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
            portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
            localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

            /* Get the address of DP2CFI Enable Per Port register */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                egrTxQConf.txQueueDpToCfiPerPortEnReg;

            /* Remap CPU Port Number to the offset in the register */
            if(CPSS_CPU_PORT_NUM_CNS == localPort)
            {
                localPort = 31;
            }

            /* Get the DP-to-CFI/DEI mapping value for current port */
            rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, localPort, 1, &hwValue);
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.
                    distributor.dpToCfiRemapEn[OFFSET_TO_WORD_MAC(portNum)];

            rc = prvCpssHwPpGetRegField(devNum, regAddr,
                    OFFSET_TO_BIT_MAC(portNum) , 1, &hwValue);
        }
    }

    if(rc != GT_OK)
        return rc;

    *enablePtr = (hwValue == 1) ? GT_TRUE : GT_FALSE;

    return rc;
}

/**
* @internal cpssDxChCosPortDpToCfiDeiMapEnableGet function
* @endinternal
*
* @brief   Gets status (Enabled/Disabled) of Drop Precedence mapping to
*         Drop Eligibility Indicator bit.
*         When enabled on egress port the DEI(CFI) bit at the outgoing VLAN
*         tag is set according to the packet DP(after mapping).
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number including CPU port.
*
* @param[out] enablePtr                - (pointer to) value of DP-to-CFI/DEI mapping on the
*                                      egress port (enable/disable):
*                                      GT_TRUE  - DP-to-CFI/DEI mapping enable on port.
*                                      GT_FALSE - DP-to-CFI/DEI mapping disable on port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortDpToCfiDeiMapEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortDpToCfiDeiMapEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChCosPortDpToCfiDeiMapEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosPortDpToCfiDei1MapEnableSet function
* @endinternal
*
* @brief   Enables or disables mapping of Drop Precedence to Drop Eligibility
*         Indicator 1 bit.
*         When enabled on egress port the DEI(CFI) bit at the outgoing Tag1 VLAN
*         tag is set according to the packet DP(after mapping).
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number including CPU port.
* @param[in] enable                   - enable/disable DP-to-CFI1/DEI1 mapping on the-to-CFI1/DEI1 mapping on the
*                                      egress port:
*                                      GT_TRUE  - DP-to-CFI1/DEI1 mapping enable on port.
*                                      GT_FALSE - DP-to-CFI1/DEI1 mapping disable on port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortDpToCfiDei1MapEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_U32  regData;    /* data to be written to the register */
    GT_U32  regOffset;  /* data offset in the register*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |CPSS_XCAT_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

    regData = (enable == GT_TRUE) ? 1 : 0;

    if(CPSS_CPU_PORT_NUM_CNS == portNum)
    {
        regOffset = 31;
    }
    else
    {
        regOffset = portNum;
    }

    return prvCpssHwPpSetRegField(devNum,
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                       egrTxQConf.br802_1ConfigRegs.txQueueDpToCfi1PerPortEnReg,
                regOffset, 1, regData);
}

/**
* @internal cpssDxChCosPortDpToCfiDei1MapEnableSet function
* @endinternal
*
* @brief   Enables or disables mapping of Drop Precedence to Drop Eligibility
*         Indicator 1 bit.
*         When enabled on egress port the DEI(CFI) bit at the outgoing Tag1 VLAN
*         tag is set according to the packet DP(after mapping).
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number including CPU port.
* @param[in] enable                   - enable/disable DP-to-CFI1/DEI1 mapping on the-to-CFI1/DEI1 mapping on the
*                                      egress port:
*                                      GT_TRUE  - DP-to-CFI1/DEI1 mapping enable on port.
*                                      GT_FALSE - DP-to-CFI1/DEI1 mapping disable on port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortDpToCfiDei1MapEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortDpToCfiDei1MapEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChCosPortDpToCfiDei1MapEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosPortDpToCfiDei1MapEnableGet function
* @endinternal
*
* @brief   Gets status (Enabled/Disabled) of Drop Precedence mapping to
*         Drop Eligibility Indicator 1 bit.
*         When enabled on egress port the DEI(CFI) bit at the outgoing Tag1 VLAN
*         tag is set according to the packet DP(after mapping).
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number including CPU port.
*
* @param[out] enablePtr                - (pointer to) value of DP-to-CFI1/DEI1 mapping on the
*                                      egress port (enable/disable):
*                                      GT_TRUE  - DP-to-CFI1/DEI1 mapping enable on port.
*                                      GT_FALSE - DP-to-CFI1/DEI1 mapping disable on port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortDpToCfiDei1MapEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32      hwValue;  /* HW value of field in the register*/
    GT_U32      hwOffset; /* HW value offset in the register*/
    GT_STATUS   rc;       /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |CPSS_XCAT_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(CPSS_CPU_PORT_NUM_CNS == portNum)
    {
        hwOffset = 31;
    }
    else
    {
        hwOffset = portNum;
    }

    rc = prvCpssHwPpGetRegField(devNum,
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                       egrTxQConf.br802_1ConfigRegs.txQueueDpToCfi1PerPortEnReg,
                hwOffset, 1, &hwValue);

    if(GT_OK != rc)
    {
        return rc;
    }

    *enablePtr = (hwValue == 1) ? GT_TRUE : GT_FALSE;

    return rc;
}

/**
* @internal cpssDxChCosPortDpToCfiDei1MapEnableGet function
* @endinternal
*
* @brief   Gets status (Enabled/Disabled) of Drop Precedence mapping to
*         Drop Eligibility Indicator 1 bit.
*         When enabled on egress port the DEI(CFI) bit at the outgoing Tag1 VLAN
*         tag is set according to the packet DP(after mapping).
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number including CPU port.
*
* @param[out] enablePtr                - (pointer to) value of DP-to-CFI1/DEI1 mapping on the
*                                      egress port (enable/disable):
*                                      GT_TRUE  - DP-to-CFI1/DEI1 mapping enable on port.
*                                      GT_FALSE - DP-to-CFI1/DEI1 mapping disable on port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortDpToCfiDei1MapEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortDpToCfiDei1MapEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChCosPortDpToCfiDei1MapEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosDpToCfiDeiMapSet function
* @endinternal
*
* @brief   Maps Drop Precedence to Drop Eligibility Indicator bit.
*         Ports that support S-Tags can be enabled to map the packet's DP
*         (derived from QoS Profile assignment) to the DEI bit(appeared
*         in the outgoing S-Tag) by cpssDxChCosPortDpToCfiDeiMapEnableSet().
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] dp                       - Drop Precedence [Green, Red].
* @param[in] cfiDeiBit                - value of CFI/DEI bit (APPLICABLE RANGES: 0..1).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or dp.
* @retval GT_OUT_OF_RANGE          - on cfiDeiBit out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Three DP levels are mapped to only two values of CFI/DEI bit.
*
*/
static GT_STATUS internal_cpssDxChCosDpToCfiDeiMapSet
(
    IN GT_U8                devNum,
    IN CPSS_DP_LEVEL_ENT    dp,
    IN GT_U8                cfiDeiBit
)
{
    GT_U32      bitOffset;  /* offset in the register */
    GT_U32      regAddr;    /* register address       */
    GT_U32      hwValue;    /* HW value               */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    PRV_CPSS_DXCH_COS_DP_TO_HW_CHECK_AND_CONVERT_MAC(
        devNum, dp, hwValue);

    if(cfiDeiBit > BIT_0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr= PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->ERMRK.ERMRKDP2CFITable;
        return prvCpssHwPpSetRegField(devNum, regAddr, hwValue, 1, (GT_U32)cfiDeiBit);
    }

    if (0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        bitOffset = hwValue;

        /* Get the address of DP2CFI register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            egrTxQConf.txQueueDpToCfiReg;
    }
    else
    {
        bitOffset = (7 + hwValue);

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.queue.distributor.generalConfig;
    }

    /* Set the CFI/DEI bit */
    return prvCpssHwPpSetRegField(
        devNum, regAddr, bitOffset, 1, (GT_U32)cfiDeiBit);
}

/**
* @internal cpssDxChCosDpToCfiDeiMapSet function
* @endinternal
*
* @brief   Maps Drop Precedence to Drop Eligibility Indicator bit.
*         Ports that support S-Tags can be enabled to map the packet's DP
*         (derived from QoS Profile assignment) to the DEI bit(appeared
*         in the outgoing S-Tag) by cpssDxChCosPortDpToCfiDeiMapEnableSet().
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] dp                       - Drop Precedence [Green, Red].
* @param[in] cfiDeiBit                - value of CFI/DEI bit (APPLICABLE RANGES: 0..1).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or dp.
* @retval GT_OUT_OF_RANGE          - on cfiDeiBit out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Three DP levels are mapped to only two values of CFI/DEI bit.
*
*/
GT_STATUS cpssDxChCosDpToCfiDeiMapSet
(
    IN GT_U8                devNum,
    IN CPSS_DP_LEVEL_ENT    dp,
    IN GT_U8                cfiDeiBit
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosDpToCfiDeiMapSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, dp, cfiDeiBit));

    rc = internal_cpssDxChCosDpToCfiDeiMapSet(devNum, dp, cfiDeiBit);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, dp, cfiDeiBit));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosDpToCfiDeiMapGet function
* @endinternal
*
* @brief   Gets Drop Precedence mapping to Drop Eligibility Indicator bit.
*         Ports that support S-Tags can be enabled to map the packet's DP
*         (derived from QoS Profile assignment) to the DEI bit(appeared
*         in the outgoing S-Tag) by cpssDxChCosPortDpToCfiDeiMapEnableSet().
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] dp                       - Drop Precedence [Green, Red].
*
* @param[out] cfiDeiBitPtr             - (pointer to) value of CFI/DEI bit.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or dp.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Three DP levels are mapped to only two values of CFI/DEI bit.
*
*/
static GT_STATUS internal_cpssDxChCosDpToCfiDeiMapGet
(
    IN  GT_U8               devNum,
    IN  CPSS_DP_LEVEL_ENT   dp,
    OUT GT_U8               *cfiDeiBitPtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      hwValue;    /* HW value of field in the register*/
    GT_U32      bitOffset;  /* offset in the register */
    GT_STATUS   rc;         /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    PRV_CPSS_DXCH_COS_DP_TO_HW_CHECK_AND_CONVERT_MAC(
        devNum, dp, hwValue);
    CPSS_NULL_PTR_CHECK_MAC(cfiDeiBitPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr= PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->ERMRK.ERMRKDP2CFITable;
        bitOffset = hwValue;
        rc = prvCpssHwPpGetRegField(devNum, regAddr, bitOffset, 1, &hwValue);
        *cfiDeiBitPtr = (GT_U8)hwValue;
        return rc;
    }

    if (0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        bitOffset = hwValue;

        /* Get the address of DP2CFI register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            egrTxQConf.txQueueDpToCfiReg;
    }
    else
    {
        bitOffset = (7 + hwValue);

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.queue.distributor.generalConfig;
    }

    /* Get the HW value of CFI/DEI bit */
    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, bitOffset, 1, &hwValue);
    *cfiDeiBitPtr = (GT_U8)hwValue;
    return rc;
}

/**
* @internal cpssDxChCosDpToCfiDeiMapGet function
* @endinternal
*
* @brief   Gets Drop Precedence mapping to Drop Eligibility Indicator bit.
*         Ports that support S-Tags can be enabled to map the packet's DP
*         (derived from QoS Profile assignment) to the DEI bit(appeared
*         in the outgoing S-Tag) by cpssDxChCosPortDpToCfiDeiMapEnableSet().
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] dp                       - Drop Precedence [Green, Red].
*
* @param[out] cfiDeiBitPtr             - (pointer to) value of CFI/DEI bit.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or dp.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Three DP levels are mapped to only two values of CFI/DEI bit.
*
*/
GT_STATUS cpssDxChCosDpToCfiDeiMapGet
(
    IN  GT_U8               devNum,
    IN  CPSS_DP_LEVEL_ENT   dp,
    OUT GT_U8               *cfiDeiBitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosDpToCfiDeiMapGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, dp, cfiDeiBitPtr));

    rc = internal_cpssDxChCosDpToCfiDeiMapGet(devNum, dp, cfiDeiBitPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, dp, cfiDeiBitPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosExpToProfileMapSet function
* @endinternal
*
* @brief   Maps the packet MPLS exp to a QoS Profile.
*         APPLICABLE DEVICES:
*         DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond.
*         INPUTS:
*         devNum     - device number.
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
* @param[in] exp                      - mpls  (APPLICABLE RANGES: 0..7).
* @param[in] profileIndex             - profile index, which is assigned to a VLAN or
*                                      Marvell tagged packet with the up on ports with
*                                      enabled trust L2 or trust L2-L3.
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..127).
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..1023).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, exp or profileIndex
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosExpToProfileMapSet
(
    IN  GT_U8   devNum,
    IN GT_U32   mappingTableIndex,
    IN  GT_U32  exp,
    IN  GT_U32  profileIndex
)
{
    GT_U32      bitOffset;          /* offset in the register */
    GT_U32      regAddr;            /* register address */
    GT_STATUS   rc;                 /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E);

    PRV_CPSS_DXCH_COS_CHECK_EXP_MAC(exp);
    PRV_CPSS_DXCH_QOS_PROFILE_ID_CHECK_MAC(devNum,profileIndex);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_EXP_TO_QOS_PROFILE_E,
                                            mappingTableIndex,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            10 * exp ,/* start bit */
                                            10, /* num of bits */
                                            profileIndex);
        return rc;
    }

    /* getting the base address */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.qosProfileRegs.exp2QosProfileMapTable;

    /* adding the offset of the register                */
    /* EXP<4n...4n+3>2QoSProfileEntry*0x4 (0<=n<1)      */
    /* each register contains 4 entries of 8 bits each  */
    regAddr +=  (exp / 4) * 4;

    bitOffset = (exp % 4) * 8;

    rc = prvCpssHwPpSetRegField(devNum,regAddr,bitOffset,7,profileIndex);
    return rc;
}

/**
* @internal cpssDxChCosExpToProfileMapSet function
* @endinternal
*
* @brief   Maps the packet MPLS exp to a QoS Profile.
*         APPLICABLE DEVICES:
*         DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond.
*         INPUTS:
*         devNum     - device number.
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
* @param[in] exp                      - mpls  (APPLICABLE RANGES: 0..7).
* @param[in] profileIndex             - profile index, which is assigned to a VLAN or
*                                      Marvell tagged packet with the up on ports with
*                                      enabled trust L2 or trust L2-L3.
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..127).
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..1023).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, exp or profileIndex
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Starting with BobCat2, there is no direct MPLS EXP to QoS profile mapping, rather this function sets
*       MPLS EXP to QoS Profile mappings entry in the Trust QoS Mapping table. Mapping table per ePort is
*       set by cpssDxChCosPortTrustQosMappingTableIndexSet
*
*/
GT_STATUS cpssDxChCosExpToProfileMapSet
(
    IN  GT_U8   devNum,
    IN GT_U32   mappingTableIndex,
    IN  GT_U32  exp,
    IN  GT_U32  profileIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosExpToProfileMapSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mappingTableIndex, exp, profileIndex));

    rc = internal_cpssDxChCosExpToProfileMapSet(devNum, mappingTableIndex, exp, profileIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mappingTableIndex, exp, profileIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosExpToProfileMapGet function
* @endinternal
*
* @brief   Returns the mapping of packet MPLS exp to a QoS Profile.
*         APPLICABLE DEVICES:
*         DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond.
*         INPUTS:
*         devNum     - device number.
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin; AC3X.
* @param[in] exp                      - mpls exp.
*
* @param[out] profileIndexPtr          - (pointer to)profile index, which is assigned to
*                                      a VLAN or Marvell tagged packet with the up on
*                                      ports with enabled trust L2 or trust L2-L3.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, exp
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosExpToProfileMapGet
(
    IN  GT_U8   devNum,
    IN GT_U32   mappingTableIndex,
    IN  GT_U32  exp,
    OUT GT_U32  *profileIndexPtr
)
{
    GT_U32      bitOffset;          /* offset in the register */
    GT_U32      regAddr;            /* register address */
    GT_STATUS   rc;                 /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E);

    PRV_CPSS_DXCH_COS_CHECK_EXP_MAC(exp);
    CPSS_NULL_PTR_CHECK_MAC(profileIndexPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_EXP_TO_QOS_PROFILE_E,
                                            mappingTableIndex,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            10 * exp ,/* start bit */
                                            10, /* num of bits */
                                            profileIndexPtr);
        return rc;
    }

    /* getting the base address */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.qosProfileRegs.exp2QosProfileMapTable;

    /* adding the offset of the register                */
    /* EXP<4n...4n+3>2QoSProfileEntry*0x4 (0<=n<1)      */
    /* each register contains 4 entries of 8 bits each  */
    regAddr +=  (exp / 4) * 4;

    bitOffset = (exp % 4) * 8;

    rc = prvCpssHwPpGetRegField(devNum,regAddr,bitOffset,7,profileIndexPtr);
    return rc;
}

/**
* @internal cpssDxChCosExpToProfileMapGet function
* @endinternal
*
* @brief   Returns the mapping of packet MPLS exp to a QoS Profile.
*         APPLICABLE DEVICES:
*         DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond.
*         INPUTS:
*         devNum     - device number.
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin; AC3X.
* @param[in] exp                      - mpls exp.
*
* @param[out] profileIndexPtr          - (pointer to)profile index, which is assigned to
*                                      a VLAN or Marvell tagged packet with the up on
*                                      ports with enabled trust L2 or trust L2-L3.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, exp
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosExpToProfileMapGet
(
    IN  GT_U8   devNum,
    IN GT_U32   mappingTableIndex,
    IN  GT_U32  exp,
    OUT GT_U32  *profileIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosExpToProfileMapGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mappingTableIndex, exp, profileIndexPtr));

    rc = internal_cpssDxChCosExpToProfileMapGet(devNum, mappingTableIndex, exp, profileIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mappingTableIndex, exp, profileIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosPortQosConfigSet function
* @endinternal
*
* @brief   Configures the port's QoS attributes.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] portQosCfgPtr            - Pointer to QoS related configuration of a port.
* @param[in] portQosCfgPtr            -
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..127);
* @param[in] portQosCfgPtr            and
* @param[in] portQosCfgPtr
*                                      only [CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
*                                      CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E]
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port or portQosCfgPtr
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortQosConfigSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN CPSS_QOS_ENTRY_STC  *portQosCfgPtr
)
{
    GT_U8         modifyDscp;   /* 1 bit that specifies if dscp modification
                                is enabled or disabled */
    GT_U8         modifyUp;     /* 1 bit that specifies if up modification
                                is enabled or disabled */
    GT_U8         precedence;   /* port precedence */
    GT_U32        qosProfileId; /* Port QoS Profile index */
    GT_U32        qosWord;      /* entry word which contains the QoS params */
    GT_U32        modifyDscpOffset; /* offset of modifyDscp in qosWord */
    GT_U32        modifyUpOffset;   /* offset of modifyUp in qosWord */
    GT_U32        precedenceOffset; /* offset of port precedence in qosWord */
    GT_U32        profileIdOffset; /* offset of qosProfileId in qosWord */
    GT_U32        regData;          /* data to be written or read to/from register*/
    GT_U32        regMask;          /* Qos data fields mask */
    GT_STATUS     rc;               /* return code            */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(portQosCfgPtr);
    PRV_CPSS_DXCH_QOS_PROFILE_ID_CHECK_MAC(devNum, portQosCfgPtr->qosProfileId);

    /* getting QoS Configuration fields */
    qosProfileId = portQosCfgPtr->qosProfileId;
    PRV_CPSS_CONVERT_PRECEDENCE_TO_HW_VAL_MAC(precedence, portQosCfgPtr->assignPrecedence)

    switch(portQosCfgPtr->enableModifyDscp)
    {
        case CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:
            modifyDscp = 0;
            break;
        case CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
            modifyDscp = 1;
            break;
        default:
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(portQosCfgPtr->enableModifyUp)
    {
        case CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:
            modifyUp = 0;
            break;
        case CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
            modifyUp = 1;
            break;
        default:
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* bit 0 */
        regData = modifyDscp;       /*bit 67*/
        /* bit 1 */
        regData |= modifyUp << 1;   /*bit 68*/
        /* bit 2 */
        regData |= precedence << 2; /*bit 69*/

        /* write entry to pre-tti-lookup-ingress-eport Table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                             portNum,
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                             SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PORT_MODIFY_DSCP_E, /* field name */
                                             3, /* know length of 3 fields !!! */
                                             regData);
        if(rc != GT_OK)
        {
            return rc;
        }

        regData = qosProfileId ;

        /* write entry to pre-tti-lookup-ingress-eport Table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                             portNum,
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                             SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PORT_QOS_PROFILE_E, /* field name */
                                             PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                             regData);

    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            /* xCat, xCat3, lion and xCat2 */
            qosWord          = 1;
            modifyDscpOffset = 5;
            modifyUpOffset   = 4;
            precedenceOffset = 3;
            profileIdOffset  = 12;
            regMask = 0xFFF80FC7; /* mask entry, zero bits: 3,4,5 and 12-18 */
        }
        else
        {
            qosWord          = 0;
            modifyDscpOffset = 8;
            modifyUpOffset   = 7;
            precedenceOffset = 13;
            profileIdOffset  = 25;
            regMask = 0x01FFDE7F; /* mask entry, zero bits: 7,8,13 and 25-31 */
        }

        /* read entry from Ports VLAN and QoS Configuration Table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                            portNum,
                                            qosWord,
                                            0,
                                            32,
                                            &regData);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* update entry with QoS parameters*/
        regData &= regMask;

        regData |= (modifyUp << modifyUpOffset);

        regData |= (modifyDscp << modifyDscpOffset);

        regData |= (precedence << precedenceOffset);

        regData |= (qosProfileId << profileIdOffset);

        /* write entry to Ports VLAN and QoS Configuration Table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                             portNum,
                                             qosWord,
                                             0,
                                             32,
                                             regData);

    }
    return rc;
}

/**
* @internal cpssDxChCosPortQosConfigSet function
* @endinternal
*
* @brief   Configures the port's QoS attributes.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] portQosCfgPtr            - Pointer to QoS related configuration of a port.
* @param[in] portQosCfgPtr            -
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..127);
* @param[in] portQosCfgPtr            and
* @param[in] portQosCfgPtr
*                                      only [CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
*                                      CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E]
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port or portQosCfgPtr
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortQosConfigSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN CPSS_QOS_ENTRY_STC  *portQosCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortQosConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portQosCfgPtr));

    rc = internal_cpssDxChCosPortQosConfigSet(devNum, portNum, portQosCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portQosCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosPortQosConfigGet function
* @endinternal
*
* @brief   Get the port's QoS attributes configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @param[out] portQosCfgPtr            - Pointer to QoS related configuration of a port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port or portQosCfgPtr
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortQosConfigGet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    OUT CPSS_QOS_ENTRY_STC  *portQosCfgPtr

)
{
    GT_U32        modifyDscp;   /* 1 bit that specifies if dscp modification
                                is enabled or disabled */
    GT_U32        modifyUp;     /* 1 bit that specifies if up modification
                                is enabled or disabled */
    GT_U32        precedence;   /* port precedence */
    GT_U32        qosWord;      /* entry word which contains the QoS params */
    GT_U32        modifyDscpOffset; /* offset of modifyDscp in qosWord */
    GT_U32        modifyUpOffset;   /* offset of modifyUp in qosWord */
    GT_U32        precedenceOffset; /* offset of port precedence in qosWord */
    GT_U32        profileIdOffset;  /* offset of qos Profile Id in qosWord */
    GT_U32        regData;          /* data to be written or read to/from register*/
    GT_U32        regData1;         /* data to be read from register1*/
    GT_STATUS     rc;               /* return code            */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(portQosCfgPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        modifyDscpOffset = 0; /*bit 67*/
        modifyUpOffset   = 1; /*bit 68*/
        precedenceOffset = 2; /*bit 69*/
        profileIdOffset  = 3; /*bit 51*/

        /* read entry from pre-tti-lookup-ingress-eport Table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                             CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                             portNum,
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                             SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PORT_MODIFY_DSCP_E, /* field name */
                                             3, /* know length of 3 fields !!! */
                                             &regData);

        if(rc != GT_OK)
        {
            return rc;
        }

        /* read entry from pre-tti-lookup-ingress-eport Table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                             CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                             portNum,
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                             SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PORT_QOS_PROFILE_E, /* field name */
                                             PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                             &regData1);

        regData |= regData1 << profileIdOffset;
        portQosCfgPtr->qosProfileId = (regData >> profileIdOffset) & 0x3FF;
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
             /* xCat, xCat3, lion and xCat2 */
            qosWord          = 1;
            modifyDscpOffset = 5;
            modifyUpOffset   = 4;
            precedenceOffset = 3;
            profileIdOffset  = 12;
        }
        else
        {
            qosWord          = 0;
            modifyDscpOffset = 8;
            modifyUpOffset   = 7;
            precedenceOffset = 13;
            profileIdOffset  = 25;
        }

        /* read entry from Ports VLAN and QoS Configuration Table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                            portNum,
                                            qosWord,
                                            0,
                                            32,
                                            &regData);

        portQosCfgPtr->qosProfileId = (regData >> profileIdOffset) & 0x7F;
    }

    /*  entry with QoS parameters */
    modifyUp     = (regData >> modifyUpOffset) & 0x1;
    modifyDscp   = (regData >> modifyDscpOffset) & 0x1;
    precedence   = (regData >> precedenceOffset) & 0x1;

    /* Getting QoS Configuration fields */
    PRV_CPSS_CONVERT_HW_VAL_TO_PRECEDENCE_MAC(portQosCfgPtr->assignPrecedence, precedence);

    portQosCfgPtr->enableModifyDscp = (modifyDscp == 1) ?
        CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E : CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;

    portQosCfgPtr->enableModifyUp = (modifyUp == 1) ?
        CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E : CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;

    return rc;
}

/**
* @internal cpssDxChCosPortQosConfigGet function
* @endinternal
*
* @brief   Get the port's QoS attributes configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @param[out] portQosCfgPtr            - Pointer to QoS related configuration of a port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port or portQosCfgPtr
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortQosConfigGet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    OUT CPSS_QOS_ENTRY_STC  *portQosCfgPtr

)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortQosConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portQosCfgPtr));

    rc = internal_cpssDxChCosPortQosConfigGet(devNum, portNum, portQosCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portQosCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosMacQosEntrySet function
* @endinternal
*
* @brief   Set the QoS Attribute of the MAC QoS Table Entr
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIdx                 - Index of a MAC QoS table's entry (APPLICABLE RANGES: 1..7).
* @param[in] macQosCfgPtr             - Pointer to QoS related configuration of a MAC QoS table's entry.
* @param[in] macQosCfgPtr             -
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..127).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, entryIndex or macQosCfgPtr
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosMacQosEntrySet
(
    IN GT_U8               devNum,
    IN GT_U32              entryIdx,
    IN CPSS_QOS_ENTRY_STC *macQosCfgPtr
)
{
    GT_U32      regData;            /* data to be written to register */
    GT_U32      hwValue;            /* HW value of field */
    GT_U32      regAddr;            /* register address */
    GT_STATUS   rc;                 /* return code */
    GT_U32      offset;             /* bits offsets */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_MAX_MAC_QOS_ENTRIES_CHECK_MAC(entryIdx);
    CPSS_NULL_PTR_CHECK_MAC(macQosCfgPtr);

    PRV_CPSS_DXCH_QOS_PROFILE_ID_CHECK_MAC(devNum, macQosCfgPtr->qosProfileId);

    regData = macQosCfgPtr->qosProfileId;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        offset = 3;

        /* index is '1' based , but access the regDb with '0' based index */
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    MACBasedQoSTable.MACQoSTableEntry[entryIdx - 1];
    }
    else
    {
        offset = 0;
        /* getting the base address */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.qosProfileRegs.macQosTable;

        regAddr += (entryIdx - 1) * 4;
    }

    /* convert Enable Modify UP to HW value */
    PRV_CPSS_ATTR_MODIFY_TO_HW_CNV_MAC(hwValue, macQosCfgPtr->enableModifyUp);
    regData |= (hwValue << (7 + offset));

    /* convert Enable Modify DSCP to HW value */
    PRV_CPSS_ATTR_MODIFY_TO_HW_CNV_MAC(hwValue, macQosCfgPtr->enableModifyDscp);
    regData |= (hwValue << (9 + offset));

    /* convert MAC QoS Precedence to HW value */
    PRV_CPSS_CONVERT_PRECEDENCE_TO_HW_VAL_MAC(hwValue, macQosCfgPtr->assignPrecedence);
    regData |= (hwValue << (11 + offset));

    /* write to MAC QoS Register */
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, regData);

    return rc;
}

/**
* @internal cpssDxChCosMacQosEntrySet function
* @endinternal
*
* @brief   Set the QoS Attribute of the MAC QoS Table Entr
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIdx                 - Index of a MAC QoS table's entry (APPLICABLE RANGES: 1..7).
* @param[in] macQosCfgPtr             - Pointer to QoS related configuration of a MAC QoS table's entry.
* @param[in] macQosCfgPtr             -
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..127).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, entryIndex or macQosCfgPtr
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosMacQosEntrySet
(
    IN GT_U8               devNum,
    IN GT_U32              entryIdx,
    IN CPSS_QOS_ENTRY_STC *macQosCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosMacQosEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIdx, macQosCfgPtr));

    rc = internal_cpssDxChCosMacQosEntrySet(devNum, entryIdx, macQosCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIdx, macQosCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosMacQosEntryGet function
* @endinternal
*
* @brief   Returns the QoS Attribute of the MAC QoS Table Entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIdx                 - Index of a MAC QoS table's entry (APPLICABLE RANGES: 1..7).
*
* @param[out] macQosCfgPtr             - Pointer to QoS related configuration of a MAC QoS
*                                      table's entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, entryIndex or macQosCfgPtr.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosMacQosEntryGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              entryIdx,
    OUT CPSS_QOS_ENTRY_STC *macQosCfgPtr
)
{
    GT_U32      regData;            /* data to be written to register */
    GT_U32      regAddr;            /* register address */
    GT_STATUS   rc;                 /* return code */
    GT_U32      qosProfileId_numBits;/*number of bits in qosProfileId*/
    GT_U32      offset;             /* bits offsets */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(macQosCfgPtr);
    PRV_CPSS_MAX_MAC_QOS_ENTRIES_CHECK_MAC(entryIdx);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        qosProfileId_numBits = 10;
        offset = 3;

        /* index is '1' based , but access the regDb with '0' based index */
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    MACBasedQoSTable.MACQoSTableEntry[entryIdx - 1];
    }
    else
    {
        qosProfileId_numBits = 7;
        offset = 0;
        /* getting the base address */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.qosProfileRegs.macQosTable;

        regAddr += (entryIdx - 1) * 4;
    }

    /* Read the MAC QoS Register */
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &regData);

    macQosCfgPtr->qosProfileId = regData & (BIT_MASK_MAC(qosProfileId_numBits)); /* 7 or 10 bits */

    macQosCfgPtr->assignPrecedence =
        (((regData >> (11 + offset)) & 0x1) == 0) ? CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E :
                                         CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;

    switch((regData >>  (9 + offset)) & 0x3) /* 2 bits */
    {
        case 0:
            macQosCfgPtr->enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
            break;
        case 1:
            macQosCfgPtr->enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
            break;
        case 2:
            macQosCfgPtr->enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
            break;
    }

    switch((regData >>  (7 + offset)) & 0x3) /* 2 bits */
    {
        case 0:
            macQosCfgPtr->enableModifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
            break;
        case 1:
            macQosCfgPtr->enableModifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
            break;
        case 2:
            macQosCfgPtr->enableModifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
            break;
    }

    return rc;
}

/**
* @internal cpssDxChCosMacQosEntryGet function
* @endinternal
*
* @brief   Returns the QoS Attribute of the MAC QoS Table Entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIdx                 - Index of a MAC QoS table's entry (APPLICABLE RANGES: 1..7).
*
* @param[out] macQosCfgPtr             - Pointer to QoS related configuration of a MAC QoS
*                                      table's entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, entryIndex or macQosCfgPtr.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosMacQosEntryGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              entryIdx,
    OUT CPSS_QOS_ENTRY_STC *macQosCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosMacQosEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIdx, macQosCfgPtr));

    rc = internal_cpssDxChCosMacQosEntryGet(devNum, entryIdx, macQosCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIdx, macQosCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosMacQosConflictResolutionSet function
* @endinternal
*
* @brief   Configure QoS Marking Conflict Mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] macQosResolvType         - Enum holding two attributes for selecting
*                                      the SA command or the DA command
*                                      (0 - DA command, 1 - SA caommand).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or macQosResolvType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosMacQosConflictResolutionSet
(
    IN GT_U8                        devNum,
    IN CPSS_MAC_QOS_RESOLVE_ENT     macQosResolvType
)
{
    GT_U32      regData;    /* data to be written to register */
    GT_U32      regAddr;    /* register address */
    GT_STATUS   rc;         /* return code            */
    GT_U32      fieldOffset;/* The start bit number in the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    switch (macQosResolvType)
    {
        case CPSS_MAC_QOS_GET_FROM_DA_E:
            regData = 1;
            break;
        case CPSS_MAC_QOS_GET_FROM_SA_E:
            regData = 0;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeGlobalConfig0;
        fieldOffset = 11;
    }
    else
    {
        /* write field <MACQosConflictMode> in Bridge Global Register0 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.
                    bridgeGlobalConfigRegArray[0];
        fieldOffset = 22;
    }

    rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 1, regData);
    return rc;

}

/**
* @internal cpssDxChCosMacQosConflictResolutionSet function
* @endinternal
*
* @brief   Configure QoS Marking Conflict Mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] macQosResolvType         - Enum holding two attributes for selecting
*                                      the SA command or the DA command
*                                      (0 - DA command, 1 - SA caommand).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or macQosResolvType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosMacQosConflictResolutionSet
(
    IN GT_U8                        devNum,
    IN CPSS_MAC_QOS_RESOLVE_ENT     macQosResolvType
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosMacQosConflictResolutionSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, macQosResolvType));

    rc = internal_cpssDxChCosMacQosConflictResolutionSet(devNum, macQosResolvType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, macQosResolvType));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosMacQosConflictResolutionGet function
* @endinternal
*
* @brief   Get Configured QoS Marking Conflict Mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] macQosResolvTypePtr      - Pointer Enum holding two optional attributes for
*                                      selecting the SA command or the DA
*                                      command (0 - DA command, 1 - SA caommand).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or macQosResolvType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosMacQosConflictResolutionGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_MAC_QOS_RESOLVE_ENT    *macQosResolvTypePtr
)
{
    GT_U32      regData;    /* data to be written to register */
    GT_U32      regAddr;    /* register address */
    GT_STATUS   rc;         /* return code            */
    GT_U32      fieldOffset;/* The start bit number in the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(macQosResolvTypePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeGlobalConfig0;
        fieldOffset = 11;
    }
    else
    {
        /* write field <MACQosConflictMode> in Bridge Global Register0 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.
                    bridgeGlobalConfigRegArray[0];
        fieldOffset = 22;
    }

    /* Read field <MACQosConflictMode> in Bridge Global Register0 */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, 1, &regData);

    *macQosResolvTypePtr =
        (regData == 1) ? CPSS_MAC_QOS_GET_FROM_DA_E : CPSS_MAC_QOS_GET_FROM_SA_E;


    return rc;

}

/**
* @internal cpssDxChCosMacQosConflictResolutionGet function
* @endinternal
*
* @brief   Get Configured QoS Marking Conflict Mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] macQosResolvTypePtr      - Pointer Enum holding two optional attributes for
*                                      selecting the SA command or the DA
*                                      command (0 - DA command, 1 - SA caommand).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or macQosResolvType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosMacQosConflictResolutionGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_MAC_QOS_RESOLVE_ENT    *macQosResolvTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosMacQosConflictResolutionGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, macQosResolvTypePtr));

    rc = internal_cpssDxChCosMacQosConflictResolutionGet(devNum, macQosResolvTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, macQosResolvTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosRemarkTblEntrySet function
* @endinternal
*
* @brief   Sets new profile index for out profile packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - profile index in the QoS Profile Table
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..127).
* @param[in] newProfileIndex          - new profile index, which is assigned for out-profile
*                                      packets according to Traffic Conditioner command.
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..127).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, profileIndex or newProfileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosRemarkTblEntrySet
(
    IN GT_U8    devNum,
    IN GT_U32   profileIndex,
    IN GT_U32   newProfileIndex
)
{
    GT_STATUS   rc;                 /* return code            */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_QOS_PROFILE_ID_CHECK_MAC(devNum, profileIndex);
    PRV_CPSS_DXCH_QOS_PROFILE_ID_CHECK_MAC(devNum, newProfileIndex);


    /* updating data with new profile index */
    /* writing the entry to QoS Remark Register */
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                         CPSS_DXCH_TABLE_REMARKING_E,
                                         profileIndex,
                                         0,
                                         0,
                                         7,
                                         newProfileIndex);
    return rc;

}

/**
* @internal cpssDxChCosRemarkTblEntrySet function
* @endinternal
*
* @brief   Sets new profile index for out profile packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - profile index in the QoS Profile Table
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..127).
* @param[in] newProfileIndex          - new profile index, which is assigned for out-profile
*                                      packets according to Traffic Conditioner command.
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..127).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, profileIndex or newProfileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosRemarkTblEntrySet
(
    IN GT_U8    devNum,
    IN GT_U32   profileIndex,
    IN GT_U32   newProfileIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosRemarkTblEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileIndex, newProfileIndex));

    rc = internal_cpssDxChCosRemarkTblEntrySet(devNum, profileIndex, newProfileIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileIndex, newProfileIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosRemarkTblEntryGet function
* @endinternal
*
* @brief   Get new profile index setting for out profile packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - profile index in the QoS Profile Table
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..127).
*
* @param[out] newProfileIndexPtr       - (pointer to) new profile index, which will be
*                                      assigned for out-profile packets according to
*                                      Traffic Conditioner command.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, profileIndex
* @retval GT_BAD_PTR               - NULL == newProfileIndexPtr
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosRemarkTblEntryGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   profileIndex,
    OUT GT_U32  *newProfileIndexPtr
)
{
    GT_STATUS   rc;                 /* return code            */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_QOS_PROFILE_ID_CHECK_MAC(devNum, profileIndex);
    CPSS_NULL_PTR_CHECK_MAC(newProfileIndexPtr);


    /* Read the entry to QoS Remark Register */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                       CPSS_DXCH_TABLE_REMARKING_E,
                                       profileIndex,
                                       0,
                                       0,
                                       7,
                                       newProfileIndexPtr);
    return rc;

}

/**
* @internal cpssDxChCosRemarkTblEntryGet function
* @endinternal
*
* @brief   Get new profile index setting for out profile packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - profile index in the QoS Profile Table
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..127).
*
* @param[out] newProfileIndexPtr       - (pointer to) new profile index, which will be
*                                      assigned for out-profile packets according to
*                                      Traffic Conditioner command.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, profileIndex
* @retval GT_BAD_PTR               - NULL == newProfileIndexPtr
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosRemarkTblEntryGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   profileIndex,
    OUT GT_U32  *newProfileIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosRemarkTblEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileIndex, newProfileIndexPtr));

    rc = internal_cpssDxChCosRemarkTblEntryGet(devNum, profileIndex, newProfileIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileIndex, newProfileIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortDefaultUPSet function
* @endinternal
*
* @brief   Set default user priority (VPT) for untagged packet to a given port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - packet processor device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] defaultUserPrio          - default user priority (VPT) (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortDefaultUPSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_U8        defaultUserPrio
)
{
    GT_U32      pup;        /* User Priority for port */
    GT_U32      startBit;   /* the word's bit at which the field starts */
    GT_STATUS   rc;         /* function call return value */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;
    GT_U32                  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_COS_CHECK_UP_MAC(defaultUserPrio);

    /* update PUP for port */
    pup = defaultUserPrio;

    /* support remote port that need remote PVID */
    if(prvCpssDxChPortRemotePortCheck(devNum,portNum))
    {
        PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

        /* Get PHY MAC object ptr */
        portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

        /* check if MACPHY callback should run */
        if (portMacObjPtr != NULL && portMacObjPtr->macDrvMacPortDefaultUPSetFunc)
        {
            GT_BOOL doPpMacConfig = GT_TRUE;

            rc = portMacObjPtr->macDrvMacPortDefaultUPSetFunc(devNum,portNum,
                                      defaultUserPrio,
                                      CPSS_MACDRV_STAGE_PRE_E,
                                      &doPpMacConfig);
            if(rc!=GT_OK)
            {
                return rc;
            }

            if(doPpMacConfig == GT_FALSE)
            {
                return GT_OK;
            }
        }
    }


    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* write to pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PORT_UP0_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        pup);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
             /* xCat, xCat3, lion and xCat2 */
            startBit  = 0;
        }
        else
        {
            startBit  = 12;
        }

        /* write data to Ports VLAN and QoS Configuration Table, word1 */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                             portNum,
                                             1,
                                             startBit,
                                             3,
                                             pup);
    }

    return rc;
}

/**
* @internal cpssDxChPortDefaultUPSet function
* @endinternal
*
* @brief   Set default user priority (VPT) for untagged packet to a given port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - packet processor device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] defaultUserPrio          - default user priority (VPT) (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortDefaultUPSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_U8        defaultUserPrio
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortDefaultUPSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, defaultUserPrio));

    rc = internal_cpssDxChPortDefaultUPSet(devNum, portNum, defaultUserPrio);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, defaultUserPrio));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortDefaultUPGet function
* @endinternal
*
* @brief   Get default user priority (VPT) for untagged packet to a given port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - packet processor device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @param[out] defaultUserPrioPtr       - (pointer to) default user priority (VPT).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortDefaultUPGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U8           *defaultUserPrioPtr
)
{
    GT_U32      pup;        /* User Priority for port */
    GT_U32      startBit;   /* the word's bit at which the field starts */
    GT_STATUS   rc;         /* function call return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(defaultUserPrioPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* read the value from pre-tti-lookup-ingress-eport table  */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PORT_UP0_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &pup);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
             /* xCat, xCat3, lion and xCat2 */
            startBit  = 0;
        }
        else
        {
            startBit  = 12;
        }

        /* read data from Ports VLAN and QoS Configuration Table, word1 */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                             portNum,
                                             1,
                                             startBit,
                                             3,
                                             &pup);
    }

    /* update PUP for port */
    *defaultUserPrioPtr = (GT_U8)pup;

    return rc;
}

/**
* @internal cpssDxChPortDefaultUPGet function
* @endinternal
*
* @brief   Get default user priority (VPT) for untagged packet to a given port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - packet processor device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @param[out] defaultUserPrioPtr       - (pointer to) default user priority (VPT).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortDefaultUPGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U8           *defaultUserPrioPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortDefaultUPGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, defaultUserPrioPtr));

    rc = internal_cpssDxChPortDefaultUPGet(devNum, portNum, defaultUserPrioPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, defaultUserPrioPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortModifyUPSet function
* @endinternal
*
* @brief   Enable/Disable overriding a tagged packet's User Priority by
*         other assignment mechanisms.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - packet processor device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] upOverrideEnable         - GT_TRUE, enable overriding a tagged packet's
*                                      User Priority by other assignment
*                                      mechanisms.
*                                      GT_FALSE for disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortModifyUPSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      upOverrideEnable
)
{
    GT_U32      regData;
    GT_U32      startWord; /* the table word at which the field starts */
    GT_U32      startBit;  /* the word's bit at which the field starts */
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    regData = (upOverrideEnable == GT_TRUE) ? 1 : 0;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* write to pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PORT_MODIFY_UP_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        regData);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
             /* xCat, xCat3, lion and xCat2 */
            startWord = 1;
            startBit  = 4;
        }
        else
        {
            startWord = 0;
            startBit  = 7;
        }

        /* write entry to Ports VLAN and QoS Configuration Table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                             portNum,
                                             startWord,
                                             startBit,
                                             1,
                                             regData);
    }

    return rc;
}

/**
* @internal cpssDxChPortModifyUPSet function
* @endinternal
*
* @brief   Enable/Disable overriding a tagged packet's User Priority by
*         other assignment mechanisms.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - packet processor device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] upOverrideEnable         - GT_TRUE, enable overriding a tagged packet's
*                                      User Priority by other assignment
*                                      mechanisms.
*                                      GT_FALSE for disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortModifyUPSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      upOverrideEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortModifyUPSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, upOverrideEnable));

    rc = internal_cpssDxChPortModifyUPSet(devNum, portNum, upOverrideEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, upOverrideEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortModifyUPGet function
* @endinternal
*
* @brief   Get Enable/Disable overriding a tagged packet's User Priority by
*         other assignment mechanisms setting.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - packet processor device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @param[out] upOverrideEnablePtr      - GT_TRUE, enable overriding a tagged packet's
*                                      User Priority by other assignment
*                                      mechanisms.
*                                      GT_FALSE for disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortModifyUPGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *upOverrideEnablePtr
)
{
    GT_U32      regData;
    GT_U32      startWord; /* the table word at which the field starts */
    GT_U32      startBit;  /* the word's bit at which the field starts */
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(upOverrideEnablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* read the value from pre-tti-lookup-ingress-eport table  */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PORT_MODIFY_UP_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &regData);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
             /* xCat, xCat3, lion and xCat2 */
            startWord = 1;
            startBit  = 4;
        }
        else
        {
            startWord = 0;
            startBit  = 7;
        }

        /* write entry to Ports VLAN and QoS Configuration Table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                             portNum,
                                             startWord,
                                             startBit,
                                             1,
                                             &regData);
    }

    *upOverrideEnablePtr = (regData == 1) ? GT_TRUE : GT_FALSE;

    return rc;
}

/**
* @internal cpssDxChPortModifyUPGet function
* @endinternal
*
* @brief   Get Enable/Disable overriding a tagged packet's User Priority by
*         other assignment mechanisms setting.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - packet processor device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @param[out] upOverrideEnablePtr      - GT_TRUE, enable overriding a tagged packet's
*                                      User Priority by other assignment
*                                      mechanisms.
*                                      GT_FALSE for disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortModifyUPGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *upOverrideEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortModifyUPGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, upOverrideEnablePtr));

    rc = internal_cpssDxChPortModifyUPGet(devNum, portNum, upOverrideEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, upOverrideEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChCosPortQosTrustModeSet function
* @endinternal
*
* @brief   Configures port's Trust Mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] portQosTrustMode         - QoS trust mode of a port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port or trustMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortQosTrustModeSet
(
    IN GT_U8                         devNum,
    IN GT_PORT_NUM                   portNum,
    IN CPSS_QOS_PORT_TRUST_MODE_ENT  portQosTrustMode
)
{
    GT_U32      regData;   /* the data to be written to rgister */
    GT_U32      startWord; /* the table word at which the field starts */
    GT_U32      startBit;  /* the word's bit at which the field starts */
    GT_STATUS   rc;        /* return code */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

     /* 2 bits */
    switch(portQosTrustMode)
    {
       case CPSS_QOS_PORT_NO_TRUST_E:
           regData = 0;
           break;
        case CPSS_QOS_PORT_TRUST_L2_E:
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
                regData = 2;
            else
                regData = 1;
           break;
        case CPSS_QOS_PORT_TRUST_L3_E:
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
                regData = 1;
            else
                regData = 2;
           break;
       case CPSS_QOS_PORT_TRUST_L2_L3_E:
           regData = 3;
           break;
       default:
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* write to pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRUST_DSCP_E, /* field name */
                                        2, /* value for 2 fields !!! */
                                        regData);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
             /* xCat, xCat3, lion and xCat2 */
            startWord = 1;
            startBit  = 7;
        }
        else
        {
            startWord = 0;
            startBit  = 10;
        }

        /* write data to Ports VLAN and QoS Configuration Table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                            portNum,
                                            startWord,
                                            startBit,
                                            2,
                                            regData);
    }
    return rc;
}

/**
* @internal cpssDxChCosPortQosTrustModeSet function
* @endinternal
*
* @brief   Configures port's Trust Mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] portQosTrustMode         - QoS trust mode of a port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port or trustMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortQosTrustModeSet
(
    IN GT_U8                         devNum,
    IN GT_PORT_NUM                   portNum,
    IN CPSS_QOS_PORT_TRUST_MODE_ENT  portQosTrustMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortQosTrustModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portQosTrustMode));

    rc = internal_cpssDxChCosPortQosTrustModeSet(devNum, portNum, portQosTrustMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portQosTrustMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosPortQosTrustModeGet function
* @endinternal
*
* @brief   Get Configured port's Trust Mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @param[out] portQosTrustModePtr      - QoS trust mode of a port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port or trustMode.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortQosTrustModeGet
(
    IN  GT_U8                         devNum,
    IN  GT_PORT_NUM                   portNum,
    OUT CPSS_QOS_PORT_TRUST_MODE_ENT  *portQosTrustModePtr
)
{
    GT_U32      regData;   /* the data to be written to rgister */
    GT_U32      startWord; /* the table word at which the field starts */
    GT_U32      startBit;  /* the word's bit at which the field starts */
    GT_STATUS   rc;        /* return code */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(portQosTrustModePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* read the value from pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRUST_DSCP_E, /* field name */
                                        2, /* value for 2 fields !!! */
                                        &regData);
        switch(regData)
        {
           case 0:
               *portQosTrustModePtr = CPSS_QOS_PORT_NO_TRUST_E;
               break;
           case 1:
               *portQosTrustModePtr = CPSS_QOS_PORT_TRUST_L3_E;
               break;
           case 2:
               *portQosTrustModePtr = CPSS_QOS_PORT_TRUST_L2_E;
               break;
           case 3:
               *portQosTrustModePtr = CPSS_QOS_PORT_TRUST_L2_L3_E;
               break;
        }
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            /* xCat, xCat3, lion and xCat2 */
            startWord = 1;
            startBit  = 7;
        }
        else
        {
            startWord = 0;
            startBit  = 10;
        }

        /* write data to Ports VLAN and QoS Configuration Table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                            portNum,
                                            startWord,
                                            startBit,
                                            2,
                                            &regData);

        switch(regData)
        {
           case 0:
               *portQosTrustModePtr = CPSS_QOS_PORT_NO_TRUST_E;
               break;
           case 1:
               *portQosTrustModePtr = CPSS_QOS_PORT_TRUST_L2_E;
               break;
           case 2:
               *portQosTrustModePtr = CPSS_QOS_PORT_TRUST_L3_E;
               break;
           case 3:
               *portQosTrustModePtr = CPSS_QOS_PORT_TRUST_L2_L3_E;
               break;
        }
    }
    return rc;
}

/**
* @internal cpssDxChCosPortQosTrustModeGet function
* @endinternal
*
* @brief   Get Configured port's Trust Mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @param[out] portQosTrustModePtr      - QoS trust mode of a port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port or trustMode.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortQosTrustModeGet
(
    IN  GT_U8                         devNum,
    IN  GT_PORT_NUM                   portNum,
    OUT CPSS_QOS_PORT_TRUST_MODE_ENT  *portQosTrustModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortQosTrustModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portQosTrustModePtr));

    rc = internal_cpssDxChCosPortQosTrustModeGet(devNum, portNum, portQosTrustModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portQosTrustModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosPortReMapDSCPSet function
* @endinternal
*
* @brief   Enable/Disable DSCP-to-DSCP re-mapping.
*         Relevant for L2-L3 or L3 trust modes.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] enableDscpMutation       - GT_TRUE: Packet's DSCP is remapped.
*                                      GT_FALSE: Packet's DSCP is not remapped.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortReMapDSCPSet
(
    IN GT_U8                        devNum,
    IN GT_PORT_NUM                  portNum,
    IN GT_BOOL                      enableDscpMutation
)
{
    GT_U32      regData;    /* the data to be written to rgister */
    GT_U32      startWord;  /* the table word at which the field starts */
    GT_U32      startBit;   /* the word's bit at which the field starts */
    GT_STATUS   rc;         /* return code */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    regData = (enableDscpMutation == GT_TRUE) ? 1 : 0;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* write to pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_MAP_DSCP_TO_DSCP_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        regData);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
             /* xCat, xCat3, lion and xCat2 */
            startWord = 1;
            startBit  = 6;
        }
        else
        {
            startWord = 0;
            startBit  = 9;
        }

        /* write data to Ports VLAN and QoS Configuration Table <ReMapDSCP> field */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                            portNum,
                                            startWord,
                                            startBit,
                                            1,
                                            regData);
    }
    return rc;
}

/**
* @internal cpssDxChCosPortReMapDSCPSet function
* @endinternal
*
* @brief   Enable/Disable DSCP-to-DSCP re-mapping.
*         Relevant for L2-L3 or L3 trust modes.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] enableDscpMutation       - GT_TRUE: Packet's DSCP is remapped.
*                                      GT_FALSE: Packet's DSCP is not remapped.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortReMapDSCPSet
(
    IN GT_U8                        devNum,
    IN GT_PORT_NUM                  portNum,
    IN GT_BOOL                      enableDscpMutation
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortReMapDSCPSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enableDscpMutation));

    rc = internal_cpssDxChCosPortReMapDSCPSet(devNum, portNum, enableDscpMutation);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enableDscpMutation));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosPortReMapDSCPGet function
* @endinternal
*
* @brief   Get status (Enabled/Disabled) for DSCP-to-DSCP re-mapping.
*         Relevant for L2-L3 or L3 trust modes.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enableDscpMutationPtr    - GT_TRUE: Packet's DSCP is remapped.
*                                      GT_FALSE: Packet's DSCP is not remapped.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortReMapDSCPGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enableDscpMutationPtr
)
{
    GT_U32      regData;    /* the data to be written to rgister */
    GT_U32      startWord;  /* the table word at which the field starts */
    GT_U32      startBit;   /* the word's bit at which the field starts */
    GT_STATUS   rc;         /* return code */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enableDscpMutationPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* read the value from pre-tti-lookup-ingress-eport table  */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_MAP_DSCP_TO_DSCP_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &regData);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
             /* xCat, xCat3, lion and xCat2 */
            startWord = 1;
            startBit  = 6;
        }
        else
        {
            startWord = 0;
            startBit  = 9;
        }

        /* write data to Ports VLAN and QoS Configuration Table <ReMapDSCP> field */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                            portNum,
                                            startWord,
                                            startBit,
                                            1,
                                            &regData);
    }

     *enableDscpMutationPtr = (regData == 1) ? GT_TRUE : GT_FALSE;

    return rc;
}

/**
* @internal cpssDxChCosPortReMapDSCPGet function
* @endinternal
*
* @brief   Get status (Enabled/Disabled) for DSCP-to-DSCP re-mapping.
*         Relevant for L2-L3 or L3 trust modes.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enableDscpMutationPtr    - GT_TRUE: Packet's DSCP is remapped.
*                                      GT_FALSE: Packet's DSCP is not remapped.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortReMapDSCPGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enableDscpMutationPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortReMapDSCPGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enableDscpMutationPtr));

    rc = internal_cpssDxChCosPortReMapDSCPGet(devNum, portNum, enableDscpMutationPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enableDscpMutationPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosPortVlanQoSCfgEntryGet function
* @endinternal
*
* @brief   Read an entry from HW from Ports VLAN and QoS Configuration Table.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] port                     - logical number, CPU port
*
* @param[out] entryPtr                 - pointer to memory where will be placed next entry.
*                                      size 32 Bytes.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortVlanQoSCfgEntryGet
(
    IN    GT_U8         devNum,
    IN    GT_PORT_NUM   port,
    OUT   GT_U32        *entryPtr
)
{
    GT_STATUS       rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);


    CPSS_NULL_PTR_CHECK_MAC(entryPtr);
    PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, port);


    /* Read entry from Ports VLAN and QoS Configuration Table */
    rc = prvCpssDxChReadTableEntry(devNum,
                                   CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                   port,
                                   entryPtr);
    return rc;
}

/**
* @internal cpssDxChCosPortVlanQoSCfgEntryGet function
* @endinternal
*
* @brief   Read an entry from HW from Ports VLAN and QoS Configuration Table.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] port                     - logical number, CPU port
*
* @param[out] entryPtr                 - pointer to memory where will be placed next entry.
*                                      size 32 Bytes.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortVlanQoSCfgEntryGet
(
    IN    GT_U8         devNum,
    IN    GT_PORT_NUM   port,
    OUT   GT_U32        *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortVlanQoSCfgEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, port, entryPtr));

    rc = internal_cpssDxChCosPortVlanQoSCfgEntryGet(devNum, port, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, port, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosRemarkEntryGet function
* @endinternal
*
* @brief   Read an entry from HW Policers QoS Remarking and Initial DP Table.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] index                    -  of entry
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..127).
*
* @param[out] entryPtr                 - pointer to memory where will be placed next entry. size 4 Bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or entry index.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosRemarkEntryGet
(
    IN    GT_U8     devNum,
    IN    GT_U32    index,
    OUT   GT_U32   *entryPtr
)
{
    GT_STATUS       rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(entryPtr);
    PRV_CPSS_DXCH_QOS_PROFILE_ID_CHECK_MAC(devNum, index);


    /* Policers QoS Remarking and Initial DP Table */
    rc = prvCpssDxChReadTableEntry(devNum,
                                   CPSS_DXCH_TABLE_REMARKING_E,
                                   index,
                                   entryPtr);
    return rc;
}

/**
* @internal cpssDxChCosRemarkEntryGet function
* @endinternal
*
* @brief   Read an entry from HW Policers QoS Remarking and Initial DP Table.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] index                    -  of entry
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..127).
*
* @param[out] entryPtr                 - pointer to memory where will be placed next entry. size 4 Bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or entry index.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosRemarkEntryGet
(
    IN    GT_U8     devNum,
    IN    GT_U32    index,
    OUT   GT_U32   *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosRemarkEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, entryPtr));

    rc = internal_cpssDxChCosRemarkEntryGet(devNum, index, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosQoSProfileEntryGet function
* @endinternal
*
* @brief   Read an entry from HW QoS Profile Table on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] index                    -  of entry
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..127).
*
* @param[out] entryPtr                 - pointer to memory where will be placed next entry. size 4 Bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or entry index.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosQoSProfileEntryGet
(
    IN    GT_U8     devNum,
    IN    GT_U32    index,
    OUT   GT_U32   *entryPtr
)
{
    GT_STATUS       rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);
    PRV_CPSS_DXCH_QOS_PROFILE_ID_CHECK_MAC(devNum, index);

    /* Read an entry from QoS Profile Table */
    rc = prvCpssDxChReadTableEntry(devNum,
                                   CPSS_DXCH_TABLE_QOS_PROFILE_E,
                                   index,
                                   entryPtr);
    return rc;
}

/**
* @internal cpssDxChCosQoSProfileEntryGet function
* @endinternal
*
* @brief   Read an entry from HW QoS Profile Table on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] index                    -  of entry
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..127).
*
* @param[out] entryPtr                 - pointer to memory where will be placed next entry. size 4 Bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or entry index.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosQoSProfileEntryGet
(
    IN    GT_U8     devNum,
    IN    GT_U32    index,
    OUT   GT_U32   *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosQoSProfileEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, entryPtr));

    rc = internal_cpssDxChCosQoSProfileEntryGet(devNum, index, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosTrustDsaTagQosModeSet function
* @endinternal
*
* @brief   Set trust DSA tag QoS mode. To enable end to end QoS in cascade system,
*         the DSA carries QoS profile assigned to the packet in previous device.
*         When the mode is set to GT_TRUE and the packet's DSA tag contains QoS
*         profile, the QoS profile assigned to the packet is assigned from the
*         DSA tag.
*         Relevant for cascading port only.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - trust DSA tag QoS mode.
*                                      GT_FALSE - The QoS profile isn't assigned from DSA tag.
*                                      GT_TRUE  - If the packets DSA tag contains a QoS profile,
*                                      it is assigned to the packet.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosTrustDsaTagQosModeSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      regData;    /* register data */
    GT_U32      startWord;  /* the table word at which the field starts */
    GT_U32      startBit;   /* the word's bit at which the field starts */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    regData = (enable == GT_TRUE) ? 1 : 0;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* write to pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRUST_DSA_TAG_QOS_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        regData);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
             /* xCat, xCat3, lion and xCat2 */
            startWord = 1;
            startBit  = 10;
        }
        else
        {
            startWord = 0;
            startBit  = 12;
        }

        /* write data to Ports VLAN and QoS Configuration Table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                            portNum,
                                            startWord,
                                            startBit,
                                            1,
                                            regData);
    }

    return rc;
}

/**
* @internal cpssDxChCosTrustDsaTagQosModeSet function
* @endinternal
*
* @brief   Set trust DSA tag QoS mode. To enable end to end QoS in cascade system,
*         the DSA carries QoS profile assigned to the packet in previous device.
*         When the mode is set to GT_TRUE and the packet's DSA tag contains QoS
*         profile, the QoS profile assigned to the packet is assigned from the
*         DSA tag.
*         Relevant for cascading port only.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - trust DSA tag QoS mode.
*                                      GT_FALSE - The QoS profile isn't assigned from DSA tag.
*                                      GT_TRUE  - If the packets DSA tag contains a QoS profile,
*                                      it is assigned to the packet.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosTrustDsaTagQosModeSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosTrustDsaTagQosModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChCosTrustDsaTagQosModeSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosTrustDsaTagQosModeGet function
* @endinternal
*
* @brief   Get trust DSA tag QoS mode. To enable end to end QoS in cascade system,
*         the DSA carries QoS profile assigned to the packet in previous device.
*         When the mode is GT_TRUE and the packet's DSA tag contains QoS
*         profile, the QoS profile assigned to the packet is assigned from the
*         DSA tag.
*         Relevant for cascading port only.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number, CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - trust DSA tag QoS mode.
*                                      GT_FALSE - The QoS profile isn't assigned from DSA tag.
*                                      GT_TRUE  - If the packets DSA tag contains a QoS profile,
*                                      it is assigned to the packet.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosTrustDsaTagQosModeGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32 regData;     /* register data */
    GT_U32 startWord;   /* the table word at which the field starts */
    GT_U32 startBit;    /* the word's bit at which the field starts */
    GT_STATUS rc;       /* return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* read the value from pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRUST_DSA_TAG_QOS_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &regData);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            /* xCat, xCat3, lion and xCat2 */
            startWord = 1;
            startBit  = 10;
        }
        else
        {
            startWord = 0;
            startBit  = 12;
        }

        /* read data to Ports VLAN and QoS Configuration Table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                            portNum,
                                            startWord,
                                            startBit,
                                            1,
                                            &regData);
    }
    if(rc != GT_OK)
        return rc;

    *enablePtr = (regData == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssDxChCosTrustDsaTagQosModeGet function
* @endinternal
*
* @brief   Get trust DSA tag QoS mode. To enable end to end QoS in cascade system,
*         the DSA carries QoS profile assigned to the packet in previous device.
*         When the mode is GT_TRUE and the packet's DSA tag contains QoS
*         profile, the QoS profile assigned to the packet is assigned from the
*         DSA tag.
*         Relevant for cascading port only.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number, CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - trust DSA tag QoS mode.
*                                      GT_FALSE - The QoS profile isn't assigned from DSA tag.
*                                      GT_TRUE  - If the packets DSA tag contains a QoS profile,
*                                      it is assigned to the packet.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosTrustDsaTagQosModeGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosTrustDsaTagQosModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChCosTrustDsaTagQosModeGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosTrustExpModeSet function
* @endinternal
*
* @brief   Enable/Disable trust EXP mode for MPLS packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - trust the MPLS header's EXP field.
*                                      GT_FALSE - QoS profile from Exp To Qos Profile table
*                                      not assigned to the MPLS packet.
*                                      GT_TRUE  - QoS profile from Exp To Qos Profile table
*                                      assigned to the MPLS packet.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosTrustExpModeSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      regData;    /* register data */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    regData = (enable == GT_FALSE) ? 0 : 1;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* write to pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRUST_EXP_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        regData);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        /* write data to Ports VLAN and QoS Configuration Table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                             portNum,
                                             1 /*startWord*/,
                                             9 /*startBit*/,
                                             1,
                                             regData);
    }

    return rc;
}

/**
* @internal cpssDxChCosTrustExpModeSet function
* @endinternal
*
* @brief   Enable/Disable trust EXP mode for MPLS packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - trust the MPLS header's EXP field.
*                                      GT_FALSE - QoS profile from Exp To Qos Profile table
*                                      not assigned to the MPLS packet.
*                                      GT_TRUE  - QoS profile from Exp To Qos Profile table
*                                      assigned to the MPLS packet.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosTrustExpModeSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosTrustExpModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChCosTrustExpModeSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosTrustExpModeGet function
* @endinternal
*
* @brief   Get Enable/Disable state of trust EXP mode for MPLS packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - (pointer to) trust the MPLS header's EXP field.
*                                      GT_FALSE - QoS profile from Exp To Qos Profile table
*                                      not assigned to the MPLS packet.
*                                      GT_TRUE  - QoS profile from Exp To Qos Profile table
*                                      assigned to the MPLS packet.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosTrustExpModeGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32      regData;     /* register data */
    GT_STATUS   rc;          /* return value */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

   if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
   {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* read the value from pre-tti-lookup-ingress-eport table  */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRUST_EXP_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &regData);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        /* read data to Ports VLAN and QoS Configuration Table */
        rc = prvCpssDxChReadTableEntryField(
            devNum,
            CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
            portNum,
            1 /*startWord*/,
            9 /*startBit*/,
            1,
            &regData);
    }
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (regData == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssDxChCosTrustExpModeGet function
* @endinternal
*
* @brief   Get Enable/Disable state of trust EXP mode for MPLS packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - (pointer to) trust the MPLS header's EXP field.
*                                      GT_FALSE - QoS profile from Exp To Qos Profile table
*                                      not assigned to the MPLS packet.
*                                      GT_TRUE  - QoS profile from Exp To Qos Profile table
*                                      assigned to the MPLS packet.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosTrustExpModeGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosTrustExpModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChCosTrustExpModeGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosL2TrustModeVlanTagSelectSet function
* @endinternal
*
* @brief   Select type of VLAN tag (either Tag0 or Tag1) for Trust L2 Qos mode.
*         APPLICABLE DEVICES:
*         xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*         INPUTS:
*         devNum     - device number.
*         portNum     - port number, CPU port.
*         In eArch devices portNum is default ePort for default VLAN Tag type.
*         isDefaultVlanTagType - indication that the vlanTagType is the default vlanTagType or
*         used for re-parse after TTI lookup:
*         1. Parse inner Layer2 after TT
*         2. Re-parse Layer2 after popping EVB/BPE tag
*         3. Parsing of passenger packet of non-TT packets (TRILL, IP-GRE, MPLS)
*         GT_TRUE - used as default vlanTagType
*         GT_FALSE - used for re-parse after TTI lookup.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort for default VLAN Tag type.
* @param[in] isDefaultVlanTagType     - indication that the vlanTagType is the default vlanTagType or
*                                      used for re-parse after TTI lookup:
*                                      1. Parse inner Layer2 after TT
*                                      2. Re-parse Layer2 after popping EVB/BPE tag
*                                      3. Parsing of passenger packet of non-TT packets (TRILL, IP-GRE, MPLS)
*                                      GT_TRUE  - used as default vlanTagType
*                                      GT_FALSE - used for re-parse after TTI lookup.
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
* @param[in] vlanTagType              - Vlan Tag Type (tag0 or tag1)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port or vlanTagType.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosL2TrustModeVlanTagSelectSet
(
    IN GT_U8                     devNum,
    IN GT_PORT_NUM               portNum,
    IN GT_BOOL                   isDefaultVlanTagType,
    IN CPSS_VLAN_TAG_TYPE_ENT    vlanTagType
)
{
    GT_U32 rc = GT_OK;
    GT_U32 regData;    /* register data */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    switch (vlanTagType)
    {
        case CPSS_VLAN_TAG0_E:
            regData = 0;
            break;
        case CPSS_VLAN_TAG1_E:
            regData = 1;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(isDefaultVlanTagType == GT_TRUE)
        {
            PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

            /* write to pre-tti-lookup-ingress-eport table */
            rc = prvCpssDxChWriteTableEntryField(devNum,
                                                CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                                portNum,
                                                PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                                SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRUST_L2_QOS_TAG0_OR_TAG1_E, /* field name */
                                                PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                                regData);
        }
        else
        {
            PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
            /* write to post-tti-lookup-ingress-eport table */
            rc = prvCpssDxChWriteTableEntryField(devNum,
                                                CPSS_DXCH_SIP5_TABLE_POST_TTI_LOOKUP_INGRESS_EPORT_E,
                                                portNum,
                                                PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                                SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_TRUST_L2_QOS_TAG0_OR_TAG1_E, /* field name */
                                                PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                                regData);
        }
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        /* write data to Ports VLAN and QoS Configuration Table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                            portNum,
                                            2 /*startWord*/,
                                            1 /*startBit*/,
                                            1,
                                            regData);
    }

    return rc;
}

/**
* @internal cpssDxChCosL2TrustModeVlanTagSelectSet function
* @endinternal
*
* @brief   Select type of VLAN tag (either Tag0 or Tag1) for Trust L2 Qos mode.
*         APPLICABLE DEVICES:
*         xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*         INPUTS:
*         devNum     - device number.
*         portNum     - port number, CPU port.
*         In eArch devices portNum is default ePort for default VLAN Tag type.
*         isDefaultVlanTagType - indication that the vlanTagType is the default vlanTagType or
*         used for re-parse after TTI lookup:
*         1. Parse inner Layer2 after TT
*         2. Re-parse Layer2 after popping EVB/BPE tag
*         3. Parsing of passenger packet of non-TT packets (TRILL, IP-GRE, MPLS)
*         GT_TRUE - used as default vlanTagType
*         GT_FALSE - used for re-parse after TTI lookup.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort for default VLAN Tag type.
* @param[in] isDefaultVlanTagType     - indication that the vlanTagType is the default vlanTagType or
*                                      used for re-parse after TTI lookup:
*                                      1. Parse inner Layer2 after TT
*                                      2. Re-parse Layer2 after popping EVB/BPE tag
*                                      3. Parsing of passenger packet of non-TT packets (TRILL, IP-GRE, MPLS)
*                                      GT_TRUE  - used as default vlanTagType
*                                      GT_FALSE - used for re-parse after TTI lookup.
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
* @param[in] vlanTagType              - Vlan Tag Type (tag0 or tag1)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port or vlanTagType.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosL2TrustModeVlanTagSelectSet
(
    IN GT_U8                     devNum,
    IN GT_PORT_NUM               portNum,
    IN GT_BOOL                   isDefaultVlanTagType,
    IN CPSS_VLAN_TAG_TYPE_ENT    vlanTagType
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosL2TrustModeVlanTagSelectSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, isDefaultVlanTagType, vlanTagType));

    rc = internal_cpssDxChCosL2TrustModeVlanTagSelectSet(devNum, portNum, isDefaultVlanTagType, vlanTagType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, isDefaultVlanTagType, vlanTagType));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosL2TrustModeVlanTagSelectGet function
* @endinternal
*
* @brief   Get Selected type of VLAN tag (either Tag0 or Tag1) for Trust L2 Qos mode.
*         APPLICABLE DEVICES:
*         xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*         INPUTS:
*         devNum   - device number.
*         portNum  - port number, CPU port.
*         In eArch devices portNum is default ePort for default VLAN Tag type.
*         isDefaultVlanTagType - indication that the vlanTagType is the default vlanTagType or
*         used for re-parse after TTI lookup:
*         1. Parse inner Layer2 after TT
*         2. Re-parse Layer2 after popping EVB/BPE tag
*         3. Parsing of passenger packet of non-TT packets (TRILL, IP-GRE, MPLS)
*         GT_TRUE - used as default vlanTagType
*         GT_FALSE - used for re-parse after TTI lookup.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort for default VLAN Tag type.
* @param[in] isDefaultVlanTagType     - indication that the vlanTagType is the default vlanTagType or
*                                      used for re-parse after TTI lookup:
*                                      1. Parse inner Layer2 after TT
*                                      2. Re-parse Layer2 after popping EVB/BPE tag
*                                      3. Parsing of passenger packet of non-TT packets (TRILL, IP-GRE, MPLS)
*                                      GT_TRUE  - used as default vlanTagType
*                                      GT_FALSE - used for re-parse after TTI lookup.
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[out] vlanTagTypePtr           - (pointer to)Vlan Tag Type (tag0 or tag1)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port or vlanTagType.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosL2TrustModeVlanTagSelectGet
(
    IN  GT_U8                     devNum,
    IN  GT_PORT_NUM               portNum,
    IN GT_BOOL                   isDefaultVlanTagType,
    OUT CPSS_VLAN_TAG_TYPE_ENT    *vlanTagTypePtr
)
{
    GT_U32 regData;     /* register data */
    GT_STATUS rc=GT_OK; /* return value */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    CPSS_NULL_PTR_CHECK_MAC(vlanTagTypePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(isDefaultVlanTagType == GT_TRUE)
        {
            PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

            /* read from pre-tti-lookup-ingress-eport table */
            rc = prvCpssDxChReadTableEntryField(devNum,
                                                CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                                portNum,
                                                PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                                SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRUST_L2_QOS_TAG0_OR_TAG1_E, /* field name */
                                                PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                                &regData);
        }
        else
        {
            PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
             /* read from post-tti-lookup-ingress-eport table */
            rc = prvCpssDxChReadTableEntryField(devNum,
                                                CPSS_DXCH_SIP5_TABLE_POST_TTI_LOOKUP_INGRESS_EPORT_E,
                                                portNum,
                                                PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                                SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_TRUST_L2_QOS_TAG0_OR_TAG1_E ,/* field name */
                                                PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                                &regData);
        }
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        /* read data to Ports VLAN and QoS Configuration Table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                            portNum,
                                            2 /*startWord*/,
                                            1 /*startBit*/,
                                            1,
                                            &regData);
    }
    if (rc != GT_OK)
    {
        return rc;
    }

    *vlanTagTypePtr =
        (regData == 0)
            ? CPSS_VLAN_TAG0_E
            : CPSS_VLAN_TAG1_E;

    return GT_OK;
}

/**
* @internal cpssDxChCosL2TrustModeVlanTagSelectGet function
* @endinternal
*
* @brief   Get Selected type of VLAN tag (either Tag0 or Tag1) for Trust L2 Qos mode.
*         APPLICABLE DEVICES:
*         xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*         INPUTS:
*         devNum   - device number.
*         portNum  - port number, CPU port.
*         In eArch devices portNum is default ePort for default VLAN Tag type.
*         isDefaultVlanTagType - indication that the vlanTagType is the default vlanTagType or
*         used for re-parse after TTI lookup:
*         1. Parse inner Layer2 after TT
*         2. Re-parse Layer2 after popping EVB/BPE tag
*         3. Parsing of passenger packet of non-TT packets (TRILL, IP-GRE, MPLS)
*         GT_TRUE - used as default vlanTagType
*         GT_FALSE - used for re-parse after TTI lookup.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort for default VLAN Tag type.
* @param[in] isDefaultVlanTagType     - indication that the vlanTagType is the default vlanTagType or
*                                      used for re-parse after TTI lookup:
*                                      1. Parse inner Layer2 after TT
*                                      2. Re-parse Layer2 after popping EVB/BPE tag
*                                      3. Parsing of passenger packet of non-TT packets (TRILL, IP-GRE, MPLS)
*                                      GT_TRUE  - used as default vlanTagType
*                                      GT_FALSE - used for re-parse after TTI lookup.
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[out] vlanTagTypePtr           - (pointer to)Vlan Tag Type (tag0 or tag1)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port or vlanTagType.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosL2TrustModeVlanTagSelectGet
(
    IN  GT_U8                     devNum,
    IN  GT_PORT_NUM               portNum,
    IN GT_BOOL                   isDefaultVlanTagType,
    OUT CPSS_VLAN_TAG_TYPE_ENT    *vlanTagTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosL2TrustModeVlanTagSelectGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, isDefaultVlanTagType, vlanTagTypePtr));

    rc = internal_cpssDxChCosL2TrustModeVlanTagSelectGet(devNum, portNum, isDefaultVlanTagType, vlanTagTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, isDefaultVlanTagType, vlanTagTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosPortUpProfileIndexSet function
* @endinternal
*
* @brief   Set per ingress port and per UP (user priority) the 'UP profile index' (table selector).
*         See also API cpssDxChCosUpCfiDeiToProfileMapSet
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ingress port number including CPU port.
* @param[in] up                       - user priority of a VLAN or DSA tagged packet (APPLICABLE RANGES: 0..7).
* @param[in] upProfileIndex           - the UP profile index (table selector) (APPLICABLE RANGES: 0..1).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, portNum or up
* @retval GT_OUT_OF_RANGE          - upProfileIndex > 1
* @retval GT_HW_ERROR              - on HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortUpProfileIndexSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U8                    up,
    IN GT_U32                   upProfileIndex
)
{
    GT_U32  regAddr;  /* the register address */
    GT_U32  bitIndex; /* bit index in register to write to */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;/* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E );

    PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);
    PRV_CPSS_DXCH_COS_CHECK_UP_MAC(up);
    PRV_CPSS_DXCH_CHECK_DATA_UP_PROFILE_INDEX_MAC(upProfileIndex);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

    /* Get the register address */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.
                qos.cfiUpToQoSProfileMappingTableSelector[localPort / 4];

    bitIndex =  ((localPort % 4) * 8) + up;

    return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr,
                    bitIndex, 1, upProfileIndex);

}

/**
* @internal cpssDxChCosPortUpProfileIndexSet function
* @endinternal
*
* @brief   Set per ingress port and per UP (user priority) the 'UP profile index' (table selector).
*         See also API cpssDxChCosUpCfiDeiToProfileMapSet
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ingress port number including CPU port.
* @param[in] up                       - user priority of a VLAN or DSA tagged packet (APPLICABLE RANGES: 0..7).
* @param[in] upProfileIndex           - the UP profile index (table selector) (APPLICABLE RANGES: 0..1).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, portNum or up
* @retval GT_OUT_OF_RANGE          - upProfileIndex > 1
* @retval GT_HW_ERROR              - on HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortUpProfileIndexSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U8                    up,
    IN GT_U32                   upProfileIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortUpProfileIndexSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, up, upProfileIndex));

    rc = internal_cpssDxChCosPortUpProfileIndexSet(devNum, portNum, up, upProfileIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, up, upProfileIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosPortUpProfileIndexGet function
* @endinternal
*
* @brief   Get per ingress port and per UP (user priority) the 'UP profile index' (table selector).
*         See also API cpssDxChCosUpCfiDeiToProfileMapSet
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ingress port number including CPU port.
* @param[in] up                       - user priority of a VLAN or DSA tagged packet (APPLICABLE RANGES: 0..7).
*
* @param[out] upProfileIndexPtr        - (pointer to)the UP profile index (table selector)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, portNum or up.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortUpProfileIndexGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U8                    up,
    OUT GT_U32                  *upProfileIndexPtr
)
{
    GT_U32  regAddr;  /* the register address */
    GT_U32  bitIndex; /* bit index in register to read from */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;/* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E );

    PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);
    PRV_CPSS_DXCH_COS_CHECK_UP_MAC(up);
    CPSS_NULL_PTR_CHECK_MAC(upProfileIndexPtr);

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

    /* Get the register address */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.
                qos.cfiUpToQoSProfileMappingTableSelector[localPort / 4];

    bitIndex =  ((localPort % 4) * 8) + up;

    return prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr,
                    bitIndex, 1, upProfileIndexPtr);
}

/**
* @internal cpssDxChCosPortUpProfileIndexGet function
* @endinternal
*
* @brief   Get per ingress port and per UP (user priority) the 'UP profile index' (table selector).
*         See also API cpssDxChCosUpCfiDeiToProfileMapSet
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ingress port number including CPU port.
* @param[in] up                       - user priority of a VLAN or DSA tagged packet (APPLICABLE RANGES: 0..7).
*
* @param[out] upProfileIndexPtr        - (pointer to)the UP profile index (table selector)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, portNum or up.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortUpProfileIndexGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U8                    up,
    OUT GT_U32                  *upProfileIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortUpProfileIndexGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, up, upProfileIndexPtr));

    rc = internal_cpssDxChCosPortUpProfileIndexGet(devNum, portNum, up, upProfileIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, up, upProfileIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChCosPortTrustQosMappingTableIndexSet function
* @endinternal
*
* @brief   Set per ingress port the 'Trust Qos Mapping Table Index' (table selector).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ingress port number including CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] useUpAsIndex             - indication if the L2 cos parameters using 'Trust Qos Mapping Table Index'
*                                      selected according to the UP (user priority) of the packet or
*                                      according the mappingTableIndex parameter.
*                                      GT_TRUE  - according to the UP (user priority) of the packet.
*                                      GT_FALSE - according the mappingTableIndex parameter.
* @param[in] mappingTableIndex        - the 'Trust Qos Mapping Table Index' (table selector).
*                                      Note: even when useUpAsIndex == GT_TRUE this param maybe used
*                                      for L3 cos parameters. (APPLICABLE RANGES : 0..11)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_OUT_OF_RANGE          - mappingTableIndex > 11
* @retval GT_HW_ERROR              - on HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortTrustQosMappingTableIndexSet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN GT_BOOL                  useUpAsIndex,
    IN GT_U32                   mappingTableIndex
)
{
    GT_STATUS  rc;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E );
    PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

    if(mappingTableIndex > 11)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* write entry to pre-tti-lookup-ingress-eport Table */
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                         CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                         portNum,
                                         PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                         SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRUST_QOS_MAPPING_TABLE_INDEX_E, /* field name */
                                         PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                         mappingTableIndex);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* write entry to pre-tti-lookup-ingress-eport Table */
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                         CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                         portNum,
                                         PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                         SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_UP_CFI_TO_QOS_TABLE_SELECT_MODE_E, /* field name */
                                         PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                         BOOL2BIT_MAC(useUpAsIndex));

    return rc;
}

/**
* @internal cpssDxChCosPortTrustQosMappingTableIndexSet function
* @endinternal
*
* @brief   Set per ingress port the 'Trust Qos Mapping Table Index' (table selector).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ingress port number including CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] useUpAsIndex             - indication if the L2 cos parameters using 'Trust Qos Mapping Table Index'
*                                      selected according to the UP (user priority) of the packet or
*                                      according the mappingTableIndex parameter.
*                                      GT_TRUE  - according to the UP (user priority) of the packet.
*                                      GT_FALSE - according the mappingTableIndex parameter.
* @param[in] mappingTableIndex        - the 'Trust Qos Mapping Table Index' (table selector).
*                                      Note: even when useUpAsIndex == GT_TRUE this param maybe used
*                                      for L3 cos parameters. (APPLICABLE RANGES : 0..11)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_OUT_OF_RANGE          - mappingTableIndex > 11
* @retval GT_HW_ERROR              - on HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortTrustQosMappingTableIndexSet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN GT_BOOL                  useUpAsIndex,
    IN GT_U32                   mappingTableIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortTrustQosMappingTableIndexSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, useUpAsIndex, mappingTableIndex));

    rc = internal_cpssDxChCosPortTrustQosMappingTableIndexSet(devNum, portNum, useUpAsIndex, mappingTableIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, useUpAsIndex, mappingTableIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosPortTrustQosMappingTableIndexGet function
* @endinternal
*
* @brief   Get per ingress port the 'Trust Qos Mapping Table Index' (table selector).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ingress port number including CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortTrustQosMappingTableIndexGet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    OUT GT_BOOL                 *useUpAsIndexPtr,
    OUT GT_U32                  *mappingTableIndexPtr
)
{
    GT_STATUS  rc;
    GT_U32  hwValue;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E );
    PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

    CPSS_NULL_PTR_CHECK_MAC(useUpAsIndexPtr);
    CPSS_NULL_PTR_CHECK_MAC(mappingTableIndexPtr);

    /* read entry from pre-tti-lookup-ingress-eport Table */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                         CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                         portNum,
                                         PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                         SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRUST_QOS_MAPPING_TABLE_INDEX_E, /* field name */
                                         PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                         mappingTableIndexPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* read entry from pre-tti-lookup-ingress-eport Table */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                         CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                         portNum,
                                         PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                         SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_UP_CFI_TO_QOS_TABLE_SELECT_MODE_E, /* field name */
                                         PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                         &hwValue);

    *useUpAsIndexPtr = BIT2BOOL_MAC(hwValue);

    return rc;
}

/**
* @internal cpssDxChCosPortTrustQosMappingTableIndexGet function
* @endinternal
*
* @brief   Get per ingress port the 'Trust Qos Mapping Table Index' (table selector).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ingress port number including CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortTrustQosMappingTableIndexGet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    OUT GT_BOOL                 *useUpAsIndexPtr,
    OUT GT_U32                  *mappingTableIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortTrustQosMappingTableIndexGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, useUpAsIndexPtr, mappingTableIndexPtr));

    rc = internal_cpssDxChCosPortTrustQosMappingTableIndexGet(devNum, portNum, useUpAsIndexPtr, mappingTableIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, useUpAsIndexPtr, mappingTableIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChCosPortEgressQosMappingTableIndexSet function
* @endinternal
*
* @brief   Set the table set index for egress port QoS remapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
* @param[in] mappingTableIndex        - Egress Qos Mapping Table Index (table set selector).
*                                      (APPLICABLE RANGES : 0..11).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, portNum or mappingTableIndex
* @retval GT_HW_ERROR              - on HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortEgressQosMappingTableIndexSet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN GT_U32                   mappingTableIndex
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    if(mappingTableIndex > 11)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* write entry to HA Egress ePort Attribute Table1 */

    return prvCpssDxChWriteTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP5_HA_EPORT_TABLE_1_FIELDS_QOS_MAPPING_TABLE_INDEX_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   mappingTableIndex);
}

/**
* @internal cpssDxChCosPortEgressQosMappingTableIndexSet function
* @endinternal
*
* @brief   Set the table set index for egress port QoS remapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
* @param[in] mappingTableIndex        - Egress Qos Mapping Table Index (table set selector).
*                                      (APPLICABLE RANGES : 0..11).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, portNum or mappingTableIndex
* @retval GT_HW_ERROR              - on HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortEgressQosMappingTableIndexSet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN GT_U32                   mappingTableIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortEgressQosMappingTableIndexSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, mappingTableIndex));

    rc = internal_cpssDxChCosPortEgressQosMappingTableIndexSet(devNum, portNum, mappingTableIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, mappingTableIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChCosPortEgressQosMappingTableIndexGet function
* @endinternal
*
* @brief   Get the table set index for egress port QoS remapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
*
* @param[out] mappingTableIndexPtr     - pointer to Egress Qos Mapping Table Index (table set selector).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, portNum or mappingTableIndex
* @retval GT_HW_ERROR              - on HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortEgressQosMappingTableIndexGet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    OUT GT_U32                  *mappingTableIndexPtr
)
{
    GT_U32      hwValue;    /* hardware value */
    GT_STATUS   rc;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(mappingTableIndexPtr);

    /* read field from  HA Egress ePort Attribute Table1 */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_EPORT_TABLE_1_FIELDS_QOS_MAPPING_TABLE_INDEX_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

   *mappingTableIndexPtr = hwValue;

    return rc;
}

/**
* @internal cpssDxChCosPortEgressQosMappingTableIndexGet function
* @endinternal
*
* @brief   Get the table set index for egress port QoS remapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
*
* @param[out] mappingTableIndexPtr     - pointer to Egress Qos Mapping Table Index (table set selector).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, portNum or mappingTableIndex
* @retval GT_HW_ERROR              - on HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortEgressQosMappingTableIndexGet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    OUT GT_U32                  *mappingTableIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortEgressQosMappingTableIndexGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, mappingTableIndexPtr));

    rc = internal_cpssDxChCosPortEgressQosMappingTableIndexGet(devNum, portNum, mappingTableIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, mappingTableIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosPortEgressQosExpMappingEnableSet function
* @endinternal
*
* @brief   Enable/disable Egress port EXP Mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
* @param[in] enable                   - enable/disable Egress port EXP Mapping.
*                                      GT_TRUE  - enable mapping.
*                                      GT_FALSE - disable mapping.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortEgressQosExpMappingEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_U32     value;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    value = BOOL2BIT_MAC(enable);

    /* write entry to HA Egress ePort Attribute Table1 */
    return prvCpssDxChWriteTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_EXP_MAPPING_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   value);
}

/**
* @internal cpssDxChCosPortEgressQosExpMappingEnableSet function
* @endinternal
*
* @brief   Enable/disable Egress port EXP Mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
* @param[in] enable                   - enable/disable Egress port EXP Mapping.
*                                      GT_TRUE  - enable mapping.
*                                      GT_FALSE - disable mapping.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortEgressQosExpMappingEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortEgressQosExpMappingEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChCosPortEgressQosExpMappingEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosPortEgressQosExpMappingEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of Egress port EXP Mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
*
* @param[out] enablePtr                - pointer to enable/disable Egress EXP Mapping status on the egress port.
*                                      GT_TRUE  -  mapping is enabled.
*                                      GT_FALSE -  mapping is disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortEgressQosExpMappingEnableGet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32      hwValue;    /* hardware value */
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* read field from  HA Egress ePort Attribute Table1 */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_EXP_MAPPING_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

   *enablePtr = BIT2BOOL_MAC(hwValue);
    return rc;
}

/**
* @internal cpssDxChCosPortEgressQosExpMappingEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of Egress port EXP Mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
*
* @param[out] enablePtr                - pointer to enable/disable Egress EXP Mapping status on the egress port.
*                                      GT_TRUE  -  mapping is enabled.
*                                      GT_FALSE -  mapping is disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortEgressQosExpMappingEnableGet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortEgressQosExpMappingEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChCosPortEgressQosExpMappingEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosPortEgressQosTcDpMappingEnableSet function
* @endinternal
*
* @brief   Enable/disable Egress port (TC, DP) mapping to (UP,EXP,DSCP).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
* @param[in] enable                   - enable/disable Egress port {TC, DP} mapping .
*                                      GT_TRUE  - enable mapping.
*                                      GT_FALSE - disable mapping.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortEgressQosTcDpMappingEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_U32     value;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    value = BOOL2BIT_MAC(enable);

    /* write entry to HA Egress ePort Attribute Table1 */
    return prvCpssDxChWriteTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_TC_DP_MAPPING_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   value);
}

/**
* @internal cpssDxChCosPortEgressQosTcDpMappingEnableSet function
* @endinternal
*
* @brief   Enable/disable Egress port (TC, DP) mapping to (UP,EXP,DSCP).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
* @param[in] enable                   - enable/disable Egress port {TC, DP} mapping .
*                                      GT_TRUE  - enable mapping.
*                                      GT_FALSE - disable mapping.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortEgressQosTcDpMappingEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortEgressQosTcDpMappingEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChCosPortEgressQosTcDpMappingEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosPortEgressQosTcDpMappingEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of Egress port (TC, DP) mapping to (UP,EXP,DSCP).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
*
* @param[out] enablePtr                - pointer to enable/disable Egress port {TC, DP} mapping status to {UP,EXP,DSCP} .
*                                      GT_TRUE  -   mapping  is enabled.
*                                      GT_FALSE -  mapping is disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortEgressQosTcDpMappingEnableGet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32      hwValue;    /* hardware value */
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* read field from  HA Egress ePort Attribute Table1 */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_TC_DP_MAPPING_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

   *enablePtr = BIT2BOOL_MAC(hwValue);;

    return rc;
}

/**
* @internal cpssDxChCosPortEgressQosTcDpMappingEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of Egress port (TC, DP) mapping to (UP,EXP,DSCP).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
*
* @param[out] enablePtr                - pointer to enable/disable Egress port {TC, DP} mapping status to {UP,EXP,DSCP} .
*                                      GT_TRUE  -   mapping  is enabled.
*                                      GT_FALSE -  mapping is disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortEgressQosTcDpMappingEnableGet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortEgressQosTcDpMappingEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChCosPortEgressQosTcDpMappingEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosPortEgressQosUpMappingEnableSet function
* @endinternal
*
* @brief   Enable/disable Egress port UP Mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
* @param[in] enable                   - enable/disable Egress port UP Mapping.
*                                      GT_TRUE  - enable mapping.
*                                      GT_FALSE - disable mapping.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortEgressQosUpMappingEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_U32     value;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    value = BOOL2BIT_MAC(enable);

    /* write entry to HA Egress ePort Attribute Table1 */
    return prvCpssDxChWriteTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_UP_MAPPING_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   value);
}

/**
* @internal cpssDxChCosPortEgressQosUpMappingEnableSet function
* @endinternal
*
* @brief   Enable/disable Egress port UP Mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
* @param[in] enable                   - enable/disable Egress port UP Mapping.
*                                      GT_TRUE  - enable mapping.
*                                      GT_FALSE - disable mapping.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortEgressQosUpMappingEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortEgressQosUpMappingEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChCosPortEgressQosUpMappingEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosPortEgressQosUpMappingEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of Egress UP Mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
*
* @param[out] enablePtr                - pointer to enable/disable Egress UP Mapping status.
*                                      GT_TRUE  -  mapping is enabled.
*                                      GT_FALSE -  mapping is disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortEgressQosUpMappingEnableGet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32      hwValue;    /* hardware value */
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* read field from  HA Egress ePort Attribute Table1 */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_UP_MAPPING_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

   *enablePtr = BIT2BOOL_MAC(hwValue);;

    return rc;
}

/**
* @internal cpssDxChCosPortEgressQosUpMappingEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of Egress UP Mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
*
* @param[out] enablePtr                - pointer to enable/disable Egress UP Mapping status.
*                                      GT_TRUE  -  mapping is enabled.
*                                      GT_FALSE -  mapping is disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortEgressQosUpMappingEnableGet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortEgressQosUpMappingEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChCosPortEgressQosUpMappingEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosPortEgressQosDscpMappingEnableSet function
* @endinternal
*
* @brief   Enable/disable Egress port DSCP Mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
* @param[in] enable                   - enable/disable Egress port DSCP Mapping.
*                                      GT_TRUE  - enable mapping.
*                                      GT_FALSE - disable mapping.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortEgressQosDscpMappingEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_U32     value;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    value = BOOL2BIT_MAC(enable);
    /* write entry to HA Egress ePort Attribute Table1 */
    return prvCpssDxChWriteTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_DSCP_MAPPING_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   value);
}

/**
* @internal cpssDxChCosPortEgressQosDscpMappingEnableSet function
* @endinternal
*
* @brief   Enable/disable Egress port DSCP Mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
* @param[in] enable                   - enable/disable Egress port DSCP Mapping.
*                                      GT_TRUE  - enable mapping.
*                                      GT_FALSE - disable mapping.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortEgressQosDscpMappingEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortEgressQosDscpMappingEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChCosPortEgressQosDscpMappingEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChCosPortEgressQosDscpMappingEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of Egress Port DSCP Mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
*
* @param[out] enablePtr                - pointer to enable/disable Egress port DSCP Mapping status.
*                                      GT_TRUE  -  mapping is enabled.
*                                      GT_FALSE -  mappingis disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCosPortEgressQosDscpMappingEnableGet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32      hwValue;    /* hardware value */
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);


    /* read field from  HA Egress ePort Attribute Table1 */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_DSCP_MAPPING_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

   *enablePtr = BIT2BOOL_MAC(hwValue);;

    return rc;
}

/**
* @internal cpssDxChCosPortEgressQosDscpMappingEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of Egress Port DSCP Mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
*
* @param[out] enablePtr                - pointer to enable/disable Egress port DSCP Mapping status.
*                                      GT_TRUE  -  mapping is enabled.
*                                      GT_FALSE -  mappingis disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortEgressQosDscpMappingEnableGet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosPortEgressQosDscpMappingEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChCosPortEgressQosDscpMappingEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosEgressDscp2DscpMappingEntrySet function
* @endinternal
*
* @brief   Egress mapping of current DSCP to a new DSCP value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
*                                      (APPLICABLE RANGES : 0..11)
* @param[in] dscp                     - current DSCP  (APPLICABLE RANGES: 0..63).
* @param[in] newDscp                  - new DSCP assigned to packet(APPLICABLE RANGES: 0..63).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or exp, or newExp, or mappingTableIndex .
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChCosEgressDscp2DscpMappingEntrySet
(
    IN GT_U8   devNum,
    IN GT_U32  mappingTableIndex,
    IN GT_U32  dscp,
    IN GT_U32  newDscp
)
{
    GT_U32      lineNumInSet; /* line number inside of maping table set */
    GT_U32      entryIndex;   /* entry index in table */
    GT_U32      fieldOffset;  /* field offset in entry */
    GT_U32      fieldLength;  /* field length */
    GT_U32      fieldValue;   /* field value  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if(mappingTableIndex > 11)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_COS_CHECK_DSCP_MAC(dscp);
    PRV_CPSS_DXCH_COS_CHECK_DSCP_MAC(newDscp);

    /* QoS Mapping Table line number is mappingTableIndex*16+N   */
    /* where N=0..15 represents the line number in set marked by */
    /*  mappingTableIndex of QoS Maping Table                     */
    lineNumInSet = dscp/4; /* 4 DCSP values in line */
    entryIndex = mappingTableIndex*16 + lineNumInSet;
    fieldOffset = (dscp%4)*6;
    fieldLength = 6;
    fieldValue = newDscp;
    return prvCpssDxChWriteTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_QOS_DSCP_MAP_E,
        entryIndex, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        fieldOffset , fieldLength, fieldValue);
}

/**
* @internal cpssDxChCosEgressDscp2DscpMappingEntrySet function
* @endinternal
*
* @brief   Egress mapping of current DSCP to a new DSCP value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
*                                      (APPLICABLE RANGES : 0..11)
* @param[in] dscp                     - current DSCP  (APPLICABLE RANGES: 0..63).
* @param[in] newDscp                  - new DSCP assigned to packet(APPLICABLE RANGES: 0..63).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or exp, or newExp, or mappingTableIndex .
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChCosEgressDscp2DscpMappingEntrySet
(
    IN GT_U8   devNum,
    IN GT_U32  mappingTableIndex,
    IN GT_U32  dscp,
    IN GT_U32  newDscp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosEgressDscp2DscpMappingEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mappingTableIndex, dscp, newDscp));

    rc = internal_cpssDxChCosEgressDscp2DscpMappingEntrySet(devNum, mappingTableIndex, dscp, newDscp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mappingTableIndex, dscp, newDscp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChCosEgressDscp2DscpMappingEntryGet function
* @endinternal
*
* @brief   Get egress mapped packet DSCP value for current dscp.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
*                                      (APPLICABLE RANGES : 0..11)
* @param[in] dscp                     - current  DSCP  (APPLICABLE RANGES: 0..63).
*
* @param[out] newDscpPtr               - pointer to mapped DSCP value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or exp, or newExp, or mappingTableIndex .
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChCosEgressDscp2DscpMappingEntryGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mappingTableIndex,
    IN  GT_U32  dscp,
    OUT GT_U32  *newDscpPtr
)
{
    GT_STATUS   rc;           /* return value   */
    GT_U32      lineNumInSet; /* line number inside of maping table set */
    GT_U32      entryIndex;   /* entry index in table */
    GT_U32      fieldOffset;  /* field offset in entry */
    GT_U32      fieldLength;  /* field length */
    GT_U32      fieldValue;   /* field value  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    if(mappingTableIndex > 11)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_COS_CHECK_DSCP_MAC(dscp);
    CPSS_NULL_PTR_CHECK_MAC(newDscpPtr);

    /* QoS Mapping Table line number is mappingTableIndex*16+N   */
    /* where N=0..15 represents the line number in set marked by */
    /* mappingTableIndex of QoS Maping Table                     */

    lineNumInSet = dscp/4; /* 4 DCSP values in line */
    entryIndex = mappingTableIndex*16 + lineNumInSet;
    fieldOffset = (dscp%4)*6;
    fieldLength = 6;

    rc =  prvCpssDxChReadTableEntryField(
            devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_QOS_DSCP_MAP_E,
            entryIndex, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            fieldOffset, fieldLength, &fieldValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    *newDscpPtr = fieldValue;
    return rc;
}

/**
* @internal cpssDxChCosEgressDscp2DscpMappingEntryGet function
* @endinternal
*
* @brief   Get egress mapped packet DSCP value for current dscp.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
*                                      (APPLICABLE RANGES : 0..11)
* @param[in] dscp                     - current  DSCP  (APPLICABLE RANGES: 0..63).
*
* @param[out] newDscpPtr               - pointer to mapped DSCP value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or exp, or newExp, or mappingTableIndex .
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChCosEgressDscp2DscpMappingEntryGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mappingTableIndex,
    IN  GT_U32  dscp,
    OUT GT_U32  *newDscpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosEgressDscp2DscpMappingEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mappingTableIndex, dscp, newDscpPtr));

    rc = internal_cpssDxChCosEgressDscp2DscpMappingEntryGet(devNum, mappingTableIndex, dscp, newDscpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mappingTableIndex, dscp, newDscpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChCosEgressExp2ExpMappingEntrySet function
* @endinternal
*
* @brief   Egress mapping of current packet Exp to a new Exp value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
*                                      (APPLICABLE RANGES : 0..11)
* @param[in] exp                      - current  (APPLICABLE RANGES: 0..7).
* @param[in] newExp                   - new Exp assigned to packet (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or exp, or newExp, or mappingTableIndex .
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChCosEgressExp2ExpMappingEntrySet
(
    IN GT_U8   devNum,
    IN GT_U32  mappingTableIndex,
    IN GT_U32  exp,
    IN GT_U32  newExp
)
{
    GT_STATUS   rc;           /* return value   */
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr; /* pointer to register db */
    GT_U32      regAddr;      /* register address */
    GT_U32      fieldOffset;  /* field offset in entry */
    GT_U32      fieldLength;  /* field length */
    GT_U32      fieldValue;   /* field value  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if(mappingTableIndex > 11)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_COS_CHECK_EXP_MAC(exp);
    PRV_CPSS_DXCH_COS_CHECK_EXP_MAC(newExp);
    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    regAddr = regsAddrPtr->ERMRK.ERMRKEXP2EXPMapTable[mappingTableIndex];
    fieldLength = 3; /* exp length*/
    fieldOffset = exp * fieldLength;
    fieldValue = newExp;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, fieldValue);
    return rc;
}

/**
* @internal cpssDxChCosEgressExp2ExpMappingEntrySet function
* @endinternal
*
* @brief   Egress mapping of current packet Exp to a new Exp value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
*                                      (APPLICABLE RANGES : 0..11)
* @param[in] exp                      - current  (APPLICABLE RANGES: 0..7).
* @param[in] newExp                   - new Exp assigned to packet (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or exp, or newExp, or mappingTableIndex .
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChCosEgressExp2ExpMappingEntrySet
(
    IN GT_U8   devNum,
    IN GT_U32  mappingTableIndex,
    IN GT_U32  exp,
    IN GT_U32  newExp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosEgressExp2ExpMappingEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mappingTableIndex, exp, newExp));

    rc = internal_cpssDxChCosEgressExp2ExpMappingEntrySet(devNum, mappingTableIndex, exp, newExp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mappingTableIndex, exp, newExp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosEgressExp2ExpMappingEntryGet function
* @endinternal
*
* @brief   Get egress mapped packet exp value for current exp.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
*                                      (APPLICABLE RANGES : 0..11)
* @param[in] exp                      - current packet    (APPLICABLE RANGES: 0..7).
*
* @param[out] newExpPtr                - pointer to mapped Exp value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or exp, or newExp, or mappingTableIndex .
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChCosEgressExp2ExpMappingEntryGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mappingTableIndex,
    IN  GT_U32  exp,
    OUT GT_U32  *newExpPtr
)
{
    GT_STATUS   rc;           /* return value   */
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr; /* pointer to register db */
    GT_U32      regAddr;      /* register address */
    GT_U32      fieldOffset;  /* field offset in entry */
    GT_U32      fieldLength;  /* field length */
    GT_U32      fieldValue;   /* field value  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if(mappingTableIndex > 11)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_COS_CHECK_EXP_MAC(exp);
    CPSS_NULL_PTR_CHECK_MAC(newExpPtr);
    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    regAddr = regsAddrPtr->ERMRK.ERMRKEXP2EXPMapTable[mappingTableIndex];
    fieldLength = 3; /* exp length*/
    fieldOffset = exp * fieldLength;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, &fieldValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    *newExpPtr = fieldValue;
    return rc;
}

/**
* @internal cpssDxChCosEgressExp2ExpMappingEntryGet function
* @endinternal
*
* @brief   Get egress mapped packet exp value for current exp.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
*                                      (APPLICABLE RANGES : 0..11)
* @param[in] exp                      - current packet    (APPLICABLE RANGES: 0..7).
*
* @param[out] newExpPtr                - pointer to mapped Exp value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or exp, or newExp, or mappingTableIndex .
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChCosEgressExp2ExpMappingEntryGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mappingTableIndex,
    IN  GT_U32  exp,
    OUT GT_U32  *newExpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosEgressExp2ExpMappingEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mappingTableIndex, exp, newExpPtr));

    rc = internal_cpssDxChCosEgressExp2ExpMappingEntryGet(devNum, mappingTableIndex, exp, newExpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mappingTableIndex, exp, newExpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosEgressUp2UpMappingEntrySet function
* @endinternal
*
* @brief   Egress mapping of current packet up to a new up value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
*                                      (APPLICABLE RANGES : 0..11)
* @param[in] up                       - current packet    (APPLICABLE RANGES: 0..7).
* @param[in] newUp                    - new Up assigned to packet (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or up, or newUp, or mappingTableIndex .
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChCosEgressUp2UpMappingEntrySet
(
    IN GT_U8   devNum,
    IN GT_U32  mappingTableIndex,
    IN GT_U32  up,
    IN GT_U32  newUp
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr; /* pointer to register db */
    GT_U32      regAddr;      /* register address */
    GT_U32      fieldOffset;  /* field offset in entry */
    GT_U32      fieldLength;  /* field length */
    GT_U32      fieldValue;   /* field value  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if(mappingTableIndex > 11)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_COS_CHECK_UP_MAC(up);
    PRV_CPSS_DXCH_COS_CHECK_UP_MAC(newUp);

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    regAddr = regsAddrPtr->ERMRK.ERMRKUP2UPMapTable[mappingTableIndex];
    fieldLength = 3; /* up length*/
    fieldOffset = up * fieldLength;
    fieldValue = newUp;
    return prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, fieldValue);
}

/**
* @internal cpssDxChCosEgressUp2UpMappingEntrySet function
* @endinternal
*
* @brief   Egress mapping of current packet up to a new up value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
*                                      (APPLICABLE RANGES : 0..11)
* @param[in] up                       - current packet    (APPLICABLE RANGES: 0..7).
* @param[in] newUp                    - new Up assigned to packet (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or up, or newUp, or mappingTableIndex .
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChCosEgressUp2UpMappingEntrySet
(
    IN GT_U8   devNum,
    IN GT_U32  mappingTableIndex,
    IN GT_U32  up,
    IN GT_U32  newUp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosEgressUp2UpMappingEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mappingTableIndex, up, newUp));

    rc = internal_cpssDxChCosEgressUp2UpMappingEntrySet(devNum, mappingTableIndex, up, newUp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mappingTableIndex, up, newUp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChCosEgressUp2UpMappingEntryGet function
* @endinternal
*
* @brief   Get egress mapped packet Up value for current Up.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
*                                      (APPLICABLE RANGES : 0..11)
* @param[in] up                       - current packet    (APPLICABLE RANGES: 0..7).
*
* @param[out] newUpPtr                 - pointer to mapped up value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or up, or newUp, or mappingTableIndex .
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChCosEgressUp2UpMappingEntryGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mappingTableIndex,
    IN  GT_U32  up,
    OUT GT_U32  *newUpPtr
)
{
    GT_STATUS   rc;           /* return value   */
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr; /* pointer to register db */
    GT_U32      regAddr;      /* register address */
    GT_U32      fieldOffset;  /* field offset in entry */
    GT_U32      fieldLength;  /* field length */
    GT_U32      fieldValue;   /* field value  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if(mappingTableIndex > 11)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_COS_CHECK_UP_MAC(up);
    CPSS_NULL_PTR_CHECK_MAC(newUpPtr);
    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    regAddr = regsAddrPtr->ERMRK.ERMRKUP2UPMapTable[mappingTableIndex];
    fieldLength = 3; /* up length*/
    fieldOffset = up * fieldLength;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, &fieldValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    *newUpPtr = fieldValue;
    return rc;
}

/**
* @internal cpssDxChCosEgressUp2UpMappingEntryGet function
* @endinternal
*
* @brief   Get egress mapped packet Up value for current Up.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
*                                      (APPLICABLE RANGES : 0..11)
* @param[in] up                       - current packet    (APPLICABLE RANGES: 0..7).
*
* @param[out] newUpPtr                 - pointer to mapped up value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or up, or newUp, or mappingTableIndex .
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChCosEgressUp2UpMappingEntryGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mappingTableIndex,
    IN  GT_U32  up,
    OUT GT_U32  *newUpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosEgressUp2UpMappingEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mappingTableIndex, up, newUpPtr));

    rc = internal_cpssDxChCosEgressUp2UpMappingEntryGet(devNum, mappingTableIndex, up, newUpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mappingTableIndex, up, newUpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCosEgressTcDp2UpExpDscpMappingEntrySet function
* @endinternal
*
* @brief   Set Egress (TC, DP) mapping to (UP,EXP,DSCP).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
*                                      (APPLICABLE RANGES : 0..11).
* @param[in] tc                       - traffic class assigned to packet  (APPLICABLE RANGES: 0..7).
* @param[in] dp                       - drop precedence.
* @param[in] up                       -  value (APPLICABLE RANGES: 0..7).
* @param[in] exp                      -  value (APPLICABLE RANGES: 0..7).
* @param[in] dscp                     -  value (APPLICABLE RANGES: 0..63).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or tc,dp, or up,exp,dscp or mappingTableIndex .
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChCosEgressTcDp2UpExpDscpMappingEntrySet
(
    IN GT_U8              devNum,
    IN GT_U32             mappingTableIndex,
    IN GT_U32             tc,
    IN CPSS_DP_LEVEL_ENT  dp,
    IN GT_U32             up,
    IN GT_U32             exp,
    IN GT_U32             dscp
)
{
    GT_U32      lineNumInSet;     /* line number inside of maping table set */
    GT_U32      entryIndex;       /* entry index in table */
    GT_U32      fieldOffset;      /* field offset in entry */
    GT_U32      fieldLength;      /* field length */
    GT_U32      fieldValue = 0;   /* field value  */
    GT_U32      dpHwValue;        /* dp in hw format */
    GT_U32      tcDpValue = 0;    /* tc_dp value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if(mappingTableIndex > 11)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_COS_DP_TO_HW_CHECK_AND_CONVERT_MAC(
        devNum, dp, dpHwValue);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);
    PRV_CPSS_DXCH_COS_CHECK_EXP_MAC(exp);
    PRV_CPSS_DXCH_COS_CHECK_DSCP_MAC(dscp);
    PRV_CPSS_DXCH_COS_CHECK_UP_MAC(up);

    /* set tcDp value */
    U32_SET_FIELD_IN_ENTRY_MAC(&tcDpValue,0,2,dpHwValue);
    U32_SET_FIELD_IN_ENTRY_MAC(&tcDpValue,2,3,tc);

    /* QoS Mapping Table line number is mappingTableIndex*16+N   */
    /* where N=0..15 represents the line number in set marked by */
    /*  mappingTableIndex of QoS Maping Table                     */
    lineNumInSet = tcDpValue/2; /* TC_DP mapings  in line */
    entryIndex = mappingTableIndex*16 + lineNumInSet;
    fieldOffset = (tcDpValue%2)*12; /* select needed half entry*/
    fieldLength = 12; /* half entry length */
    /* set half entry value */
     U32_SET_FIELD_IN_ENTRY_MAC(&fieldValue,0,6,dscp);
     U32_SET_FIELD_IN_ENTRY_MAC(&fieldValue,6,3,exp);
     U32_SET_FIELD_IN_ENTRY_MAC(&fieldValue,9,3,up);

    fieldValue = (dscp & 0x3F) | ((exp & 0x7) << 6) | ((up & 0x7) <<9);
    return prvCpssDxChWriteTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_QOS_TC_DP_MAP_E,
        entryIndex, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        fieldOffset , fieldLength, fieldValue);
}

/**
* @internal cpssDxChCosEgressTcDp2UpExpDscpMappingEntrySet function
* @endinternal
*
* @brief   Set Egress (TC, DP) mapping to (UP,EXP,DSCP).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
*                                      (APPLICABLE RANGES : 0..11).
* @param[in] tc                       - traffic class assigned to packet  (APPLICABLE RANGES: 0..7).
* @param[in] dp                       - drop precedence.
* @param[in] up                       -  value (APPLICABLE RANGES: 0..7).
* @param[in] exp                      -  value (APPLICABLE RANGES: 0..7).
* @param[in] dscp                     -  value (APPLICABLE RANGES: 0..63).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or tc,dp, or up,exp,dscp or mappingTableIndex .
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChCosEgressTcDp2UpExpDscpMappingEntrySet
(
    IN GT_U8              devNum,
    IN GT_U32             mappingTableIndex,
    IN GT_U32             tc,
    IN CPSS_DP_LEVEL_ENT  dp,
    IN GT_U32             up,
    IN GT_U32             exp,
    IN GT_U32             dscp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosEgressTcDp2UpExpDscpMappingEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mappingTableIndex, tc, dp, up, exp, dscp));

    rc = internal_cpssDxChCosEgressTcDp2UpExpDscpMappingEntrySet(devNum, mappingTableIndex, tc, dp, up, exp, dscp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mappingTableIndex, tc, dp, up, exp, dscp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChCosEgressTcDp2UpExpDscpMappingEntryGet function
* @endinternal
*
* @brief   Get Egress up,exp,dscp values mapped for current (tc, dp).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
*                                      (APPLICABLE RANGES : 0..11).
* @param[in] tc                       - traffic class assigned to packet  (APPLICABLE RANGES: 0..7).
* @param[in] dp                       - drop precedence.
*
* @param[out] upPtr                    - pointer to up value.
* @param[out] expPtr                   - pointer to exp value.
* @param[out] dscpPtr                  - pointer to dscp value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or tc, or dp, or mappingTableIndex .
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChCosEgressTcDp2UpExpDscpMappingEntryGet
(
    IN  GT_U8             devNum,
    IN  GT_U32            mappingTableIndex,
    IN  GT_U32            tc,
    IN  CPSS_DP_LEVEL_ENT dp,
    OUT GT_U32            *upPtr,
    OUT GT_U32            *expPtr,
    OUT GT_U32            *dscpPtr
)
{
    GT_STATUS   rc;                 /* return value   */
    GT_U32      lineNumInSet;       /* line number inside of maping table set */
    GT_U32      entryIndex;         /* entry index in table */
    GT_U32      fieldOffset;        /* field offset in entry */
    GT_U32      fieldLength;        /* field length */
    GT_U32      fieldValue = 0;     /* field value  */
    GT_U32      dpHwValue;          /* dp in hw format */
    GT_U32      tcDpValue = 0;      /* tc_dp value */
    GT_U32      value = 0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if(mappingTableIndex > 11)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_COS_DP_TO_HW_CHECK_AND_CONVERT_MAC(
        devNum, dp, dpHwValue);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);

    CPSS_NULL_PTR_CHECK_MAC(expPtr);
    CPSS_NULL_PTR_CHECK_MAC(upPtr);
    CPSS_NULL_PTR_CHECK_MAC(dscpPtr);

    /* set tcDp value */
    U32_SET_FIELD_IN_ENTRY_MAC(&tcDpValue,0,2,dpHwValue);
    U32_SET_FIELD_IN_ENTRY_MAC(&tcDpValue,2,3,tc);
    /* QoS Mapping Table line number is mappingTableIndex*16+N   */
    /* where N=0..15 represents the line number in set marked by */
    /*  mappingTableIndex of QoS Maping Table                     */
    lineNumInSet = tcDpValue/2; /* TC_DP mapings  in line */
    entryIndex = mappingTableIndex*16 + lineNumInSet;
    fieldOffset = (tcDpValue%2)*12; /* select needed half entry*/
    fieldLength = 12; /* half entry length */
    rc =  prvCpssDxChReadTableEntryField(
            devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_QOS_TC_DP_MAP_E,
            entryIndex, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            fieldOffset, fieldLength, &fieldValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    U32_GET_FIELD_IN_ENTRY_MAC(&fieldValue,0,6,value);
    *dscpPtr = value;
    U32_GET_FIELD_IN_ENTRY_MAC(&fieldValue,6,3,value);
    *expPtr = value;
    U32_GET_FIELD_IN_ENTRY_MAC(&fieldValue,9,3,value);
    *upPtr = value;

    return rc;
}

/**
* @internal cpssDxChCosEgressTcDp2UpExpDscpMappingEntryGet function
* @endinternal
*
* @brief   Get Egress up,exp,dscp values mapped for current (tc, dp).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
*                                      (APPLICABLE RANGES : 0..11).
* @param[in] tc                       - traffic class assigned to packet  (APPLICABLE RANGES: 0..7).
* @param[in] dp                       - drop precedence.
*
* @param[out] upPtr                    - pointer to up value.
* @param[out] expPtr                   - pointer to exp value.
* @param[out] dscpPtr                  - pointer to dscp value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or tc, or dp, or mappingTableIndex .
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChCosEgressTcDp2UpExpDscpMappingEntryGet
(
    IN  GT_U8             devNum,
    IN  GT_U32            mappingTableIndex,
    IN  GT_U32            tc,
    IN  CPSS_DP_LEVEL_ENT dp,
    OUT GT_U32            *upPtr,
    OUT GT_U32            *expPtr,
    OUT GT_U32            *dscpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCosEgressTcDp2UpExpDscpMappingEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mappingTableIndex, tc, dp, upPtr, expPtr, dscpPtr));

    rc = internal_cpssDxChCosEgressTcDp2UpExpDscpMappingEntryGet(devNum, mappingTableIndex, tc, dp, upPtr, expPtr, dscpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mappingTableIndex, tc, dp, upPtr, expPtr, dscpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

