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
* @file cpssDxChTmGlueDrop.c
*
* @brief Traffic Manager Glue - TM drop and statistics.
*
* @version   6
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/tmGlue/private/prvCpssDxChTmGlueLog.h>

#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueDrop.h>

/* number of Profile Ids */
#define PRV_CPSS_TM_GLUE_PROFILE_NUM_MAX_CNS     8
/* number of Queue Ids */
#define PRV_CPSS_TM_GLUE_TM_TC_NUM_MAX_CNS       16
/* number of QueueMax Ids */
#define PRV_CPSS_TM_GLUE_QUEUE_MAX_NUM_MAX_CNS   16*1024

/* check that the profileId is in applicable ranges 0..7
   return GT_OUT_OF_RANGE on error
*/
#define PRV_CPSS_TM_GLUE_DROP_PROFILE_ID_CHECK_MAC(profileId)    \
    if(profileId >= PRV_CPSS_TM_GLUE_PROFILE_NUM_MAX_CNS)   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG)

/* check that the profileId as index is in applicable ranges 0..7
   return GT_BAD_PARAM on error
*/
#define PRV_CPSS_TM_GLUE_DROP_PROFILE_IDX_CHECK_MAC(profileId)    \
    if(profileId >= PRV_CPSS_TM_GLUE_PROFILE_NUM_MAX_CNS)   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

/* check that the queueId is in applicable ranges 0..16383
   return GT_BAD_PARAM on error
*/
#define PRV_CPSS_TM_GLUE_DROP_QUEUE_ID_MAX_CHECK_MAC(queueId)    \
    if(queueId >= PRV_CPSS_TM_GLUE_QUEUE_MAX_NUM_MAX_CNS)   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

/* check that the TM traffic class is in applicable ranges 0..7
   return GT_BAD_PARAM on error
*/
#define PRV_CPSS_TM_GLUE_DROP_TM_TC_CHECK_MAC(tmTc)    \
    if(tmTc >= PRV_CPSS_TM_GLUE_TM_TC_NUM_MAX_CNS)   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

/**
* @internal internal_cpssDxChTmGlueDropQueueProfileIdSet function
* @endinternal
*
* @brief   The function maps every Queue Id to its Profile Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] queueId                  - TM queue id
*                                      (APPLICABLE RANGES: 0..16383).
* @param[in] profileId                - per queue profile Id
*                                      (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on wrong profile id
*/
static GT_STATUS internal_cpssDxChTmGlueDropQueueProfileIdSet
(
    IN  GT_U8     devNum,
    IN  GT_U32    queueId,
    IN  GT_U32    profileId
)
{
    GT_U32    entryIndex;       /* index to table */
    GT_U32    fieldOffset;      /* field offset   */
    GT_U32    fieldWordNum;     /* field word num */
    GT_STATUS rc;               /* return code    */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_TM_GLUE_DROP_QUEUE_ID_MAX_CHECK_MAC(queueId);
    PRV_CPSS_TM_GLUE_DROP_PROFILE_ID_CHECK_MAC(profileId);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    fieldWordNum = 0;
    entryIndex = queueId/16;
    fieldOffset = (queueId%16)*3;
    if(30 == fieldOffset)
    {
        rc = prvCpssDxChWriteTableEntryField( devNum,
                                              CPSS_DXCH_SIP5_TABLE_TM_DROP_QUEUE_PROFILE_ID_E,
                                              entryIndex,
                                              0,
                                              fieldOffset,
                                              2,
                                              (profileId&0x3));
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChWriteTableEntryField( devNum,
                                              CPSS_DXCH_SIP5_TABLE_TM_DROP_QUEUE_PROFILE_ID_E,
                                              entryIndex,
                                              1,
                                              0,
                                              1,
                                              ((profileId>>2)&0x1));
        return rc;
    }
    else if(fieldOffset > 31)
    {
        fieldWordNum = 1;
        fieldOffset -= 32;
    }

    rc = prvCpssDxChWriteTableEntryField( devNum,
                                          CPSS_DXCH_SIP5_TABLE_TM_DROP_QUEUE_PROFILE_ID_E,
                                          entryIndex,
                                          fieldWordNum,
                                          fieldOffset,
                                          3,
                                          profileId);
    return rc;
}

/**
* @internal cpssDxChTmGlueDropQueueProfileIdSet function
* @endinternal
*
* @brief   The function maps every Queue Id to its Profile Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] queueId                  - TM queue id
*                                      (APPLICABLE RANGES: 0..16383).
* @param[in] profileId                - per queue profile Id
*                                      (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on wrong profile id
*/
GT_STATUS cpssDxChTmGlueDropQueueProfileIdSet
(
    IN  GT_U8     devNum,
    IN  GT_U32    queueId,
    IN  GT_U32    profileId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueDropQueueProfileIdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, queueId, profileId));

    rc = internal_cpssDxChTmGlueDropQueueProfileIdSet(devNum, queueId, profileId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, queueId, profileId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGlueDropQueueProfileIdGet function
* @endinternal
*
* @brief   The function gets the Profile Id of a given QueueId.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] queueId                  - TM queue id
*                                      (APPLICABLE RANGES: 0..16383).
*
* @param[out] profileIdPtr             - (pointer to) profile Id per Queue
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTmGlueDropQueueProfileIdGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    queueId,
    OUT GT_U32   *profileIdPtr
)
{
    GT_U32    entryIndex;       /* index to table */
    GT_U32    fieldOffset;      /* field offset   */
    GT_U32    fieldWordNum=0;   /* field word num */
    GT_STATUS rc;               /* return code    */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_TM_GLUE_DROP_QUEUE_ID_MAX_CHECK_MAC(queueId);
    CPSS_NULL_PTR_CHECK_MAC(profileIdPtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    entryIndex = queueId/16;
    fieldOffset = (queueId%16)*3;
    if(30 == fieldOffset)
    {
        GT_U32 field0, field1;

        rc = prvCpssDxChReadTableEntryField( devNum,
                                              CPSS_DXCH_SIP5_TABLE_TM_DROP_QUEUE_PROFILE_ID_E,
                                              entryIndex,
                                              0,
                                              fieldOffset,
                                              2,
                                              &field0);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChReadTableEntryField( devNum,
                                              CPSS_DXCH_SIP5_TABLE_TM_DROP_QUEUE_PROFILE_ID_E,
                                              entryIndex,
                                              1,
                                              0,
                                              1,
                                              &field1);
        *profileIdPtr = (field1<<2) | field0;

        return rc;
    }
    else if(fieldOffset > 31)
    {
        fieldWordNum = 1;
        fieldOffset -= 32;
    }

    rc = prvCpssDxChReadTableEntryField(  devNum,
                                          CPSS_DXCH_SIP5_TABLE_TM_DROP_QUEUE_PROFILE_ID_E,
                                          entryIndex,
                                          fieldWordNum,
                                          fieldOffset,
                                          3,
                                          profileIdPtr);
    return rc;
}

/**
* @internal cpssDxChTmGlueDropQueueProfileIdGet function
* @endinternal
*
* @brief   The function gets the Profile Id of a given QueueId.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] queueId                  - TM queue id
*                                      (APPLICABLE RANGES: 0..16383).
*
* @param[out] profileIdPtr             - (pointer to) profile Id per Queue
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueDropQueueProfileIdGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    queueId,
    OUT GT_U32   *profileIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueDropQueueProfileIdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, queueId, profileIdPtr));

    rc = internal_cpssDxChTmGlueDropQueueProfileIdGet(devNum, queueId, profileIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, queueId, profileIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGlueDropProfileDropMaskSet function
* @endinternal
*
* @brief   The function configures the Drop Masking.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] profileId                - configuration per profile Id
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] tc                       - TM traffic class
*                                      (APPLICABLE RANGES: 0..15).
* @param[in] dropMaskCfgPtr           - (pointer to) drop mask configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTmGlueDropProfileDropMaskSet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   profileId,
    IN  GT_U32                                   tc,
    IN  CPSS_DXCH_TM_GLUE_DROP_MASK_STC         *dropMaskCfgPtr
)
{
    GT_U32    entryIndex;  /* index to the table    */
    GT_U32    value = 0;   /* value write to  table */
    GT_U32    hwValue;     /* hw value              */
    GT_STATUS rc;          /* return code           */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_TM_GLUE_DROP_PROFILE_IDX_CHECK_MAC(profileId);
    PRV_CPSS_TM_GLUE_DROP_TM_TC_CHECK_MAC(tc);
    CPSS_NULL_PTR_CHECK_MAC(dropMaskCfgPtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    entryIndex = profileId*PRV_CPSS_TM_GLUE_TM_TC_NUM_MAX_CNS + tc;

    switch (dropMaskCfgPtr->redPacketsDropMode)
    {
        case CPSS_DXCH_TM_GLUE_DROP_RED_PACKET_DROP_MODE_DROP_ALL_RED_E:
                hwValue = 0;
                break;

        case CPSS_DXCH_TM_GLUE_DROP_RED_PACKET_DROP_MODE_ASK_TM_RESP_E:
            hwValue = 1;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    U32_SET_FIELD_MAC(value,0,1,hwValue);
    U32_SET_FIELD_MAC(value,1,1,BOOL2BIT_MAC(dropMaskCfgPtr->outOfResourceDropUnmask));
    U32_SET_FIELD_MAC(value,2,1,BOOL2BIT_MAC(dropMaskCfgPtr->portWredDropUnmask));
    U32_SET_FIELD_MAC(value,3,1,BOOL2BIT_MAC(dropMaskCfgPtr->portTailDropUnmask));
    U32_SET_FIELD_MAC(value,4,1,BOOL2BIT_MAC(dropMaskCfgPtr->cWredDropUnmask));
    U32_SET_FIELD_MAC(value,5,1,BOOL2BIT_MAC(dropMaskCfgPtr->cTailDropUnmask));
    U32_SET_FIELD_MAC(value,6,1,BOOL2BIT_MAC(dropMaskCfgPtr->bWredDropUnmask));
    U32_SET_FIELD_MAC(value,7,1,BOOL2BIT_MAC(dropMaskCfgPtr->bTailDropUnmask));
    U32_SET_FIELD_MAC(value,8,1,BOOL2BIT_MAC(dropMaskCfgPtr->aWredDropUnmask));
    U32_SET_FIELD_MAC(value,9,1,BOOL2BIT_MAC(dropMaskCfgPtr->aTailDropUnmask));
    U32_SET_FIELD_MAC(value,10,1,BOOL2BIT_MAC(dropMaskCfgPtr->qWredDropUnmask));
    U32_SET_FIELD_MAC(value,11,1,BOOL2BIT_MAC(dropMaskCfgPtr->qTailDropUnmask));

    rc = prvCpssDxChWriteTableEntryField( devNum,
                                          CPSS_DXCH_SIP5_TABLE_TM_DROP_DROP_MASKING_E,
                                          entryIndex,
                                          0,
                                          0,
                                          12,
                                          value);
    return rc;
}

/**
* @internal cpssDxChTmGlueDropProfileDropMaskSet function
* @endinternal
*
* @brief   The function configures the Drop Masking.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] profileId                - configuration per profile Id
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] tc                       - TM traffic class
*                                      (APPLICABLE RANGES: 0..15).
* @param[in] dropMaskCfgPtr           - (pointer to) drop mask configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueDropProfileDropMaskSet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   profileId,
    IN  GT_U32                                   tc,
    IN  CPSS_DXCH_TM_GLUE_DROP_MASK_STC         *dropMaskCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueDropProfileDropMaskSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileId, tc, dropMaskCfgPtr));

    rc = internal_cpssDxChTmGlueDropProfileDropMaskSet(devNum, profileId, tc, dropMaskCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileId, tc, dropMaskCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGlueDropProfileDropMaskGet function
* @endinternal
*
* @brief   The function gets the Drop Mask configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] profileId                - configuration per profile Id
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] tc                       - TM traffic class
*                                      (APPLICABLE RANGES: 0..15).
*
* @param[out] dropMaskCfgPtr           - (pointer to) structure for drop mask configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTmGlueDropProfileDropMaskGet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   profileId,
    IN  GT_U32                                   tc,
    OUT CPSS_DXCH_TM_GLUE_DROP_MASK_STC         *dropMaskCfgPtr
)
{
    GT_U32    entryIndex;   /* index to the table    */
    GT_U32    value;        /* value read from table */
    GT_STATUS rc;           /* return code           */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_TM_GLUE_DROP_PROFILE_IDX_CHECK_MAC(profileId);
    PRV_CPSS_TM_GLUE_DROP_TM_TC_CHECK_MAC(tc);

    CPSS_NULL_PTR_CHECK_MAC(dropMaskCfgPtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    entryIndex = profileId*PRV_CPSS_TM_GLUE_TM_TC_NUM_MAX_CNS + tc;

    rc = prvCpssDxChReadTableEntryField(  devNum,
                                          CPSS_DXCH_SIP5_TABLE_TM_DROP_DROP_MASKING_E,
                                          entryIndex,
                                          0,
                                          0,
                                          12,
                                          &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    dropMaskCfgPtr->redPacketsDropMode = ((U32_GET_FIELD_MAC(value, 0, 1) == 0) ? CPSS_DXCH_TM_GLUE_DROP_RED_PACKET_DROP_MODE_DROP_ALL_RED_E :
                                          CPSS_DXCH_TM_GLUE_DROP_RED_PACKET_DROP_MODE_ASK_TM_RESP_E);
    dropMaskCfgPtr->outOfResourceDropUnmask = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value, 1, 1));
    dropMaskCfgPtr->portWredDropUnmask      = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value, 2, 1));
    dropMaskCfgPtr->portTailDropUnmask      = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value, 3, 1));
    dropMaskCfgPtr->cWredDropUnmask         = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value, 4, 1));
    dropMaskCfgPtr->cTailDropUnmask         = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value, 5, 1));
    dropMaskCfgPtr->bWredDropUnmask         = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value, 6, 1));
    dropMaskCfgPtr->bTailDropUnmask         = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value, 7, 1));
    dropMaskCfgPtr->aWredDropUnmask         = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value, 8, 1));
    dropMaskCfgPtr->aTailDropUnmask         = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value, 9, 1));
    dropMaskCfgPtr->qWredDropUnmask         = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value, 10, 1));
    dropMaskCfgPtr->qTailDropUnmask         = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value, 11, 1));

    return GT_OK;
}

/**
* @internal cpssDxChTmGlueDropProfileDropMaskGet function
* @endinternal
*
* @brief   The function gets the Drop Mask configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] profileId                - configuration per profile Id
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] tc                       - TM traffic class
*                                      (APPLICABLE RANGES: 0..15).
*
* @param[out] dropMaskCfgPtr           - (pointer to) structure for drop mask configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueDropProfileDropMaskGet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   profileId,
    IN  GT_U32                                   tc,
    OUT CPSS_DXCH_TM_GLUE_DROP_MASK_STC         *dropMaskCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueDropProfileDropMaskGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileId, tc, dropMaskCfgPtr));

    rc = internal_cpssDxChTmGlueDropProfileDropMaskGet(devNum, profileId, tc, dropMaskCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileId, tc, dropMaskCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGlueDropTcToCosSet function
* @endinternal
*
* @brief   The function configures TM TC to COS mapping table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] tmTc                     - TM traffic class
*                                      (APPLICABLE RANGES: 0..15).
* @param[in] cos                      - mapped COS value for TM-TC
*                                      (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on wrong cos
*/
static GT_STATUS internal_cpssDxChTmGlueDropTcToCosSet
(
    IN  GT_U8     devNum,
    IN  GT_U32    tmTc,
    IN  GT_U32    cos
)
{
    GT_U32    regAddr;     /* register address   */
    GT_U32    entryIndex;  /* index to the table */
    GT_U32    fieldOffset; /* field offset       */
    GT_STATUS rc;          /* return code        */
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_TM_GLUE_DROP_TM_TC_CHECK_MAC(tmTc);
    PRV_CPSS_TM_GLUE_DROP_PROFILE_ID_CHECK_MAC(cos);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);


    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    entryIndex = tmTc/8;
    fieldOffset = (tmTc%8)*3;
    regAddr = regsAddrPtr->TMDROP.TMDROPTMTCMap8To87[entryIndex];

    rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 3, cos);
    return rc;
}

/**
* @internal cpssDxChTmGlueDropTcToCosSet function
* @endinternal
*
* @brief   The function configures TM TC to COS mapping table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] tmTc                     - TM traffic class
*                                      (APPLICABLE RANGES: 0..15).
* @param[in] cos                      - mapped COS value for TM-TC
*                                      (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on wrong cos
*/
GT_STATUS cpssDxChTmGlueDropTcToCosSet
(
    IN  GT_U8     devNum,
    IN  GT_U32    tmTc,
    IN  GT_U32    cos
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueDropTcToCosSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tmTc, cos));

    rc = internal_cpssDxChTmGlueDropTcToCosSet(devNum, tmTc, cos);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tmTc, cos));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGlueDropTcToCosGet function
* @endinternal
*
* @brief   The function retrieves the TM TC to COS mapping COS value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] tmTc                     - TM traffic class
*                                      (APPLICABLE RANGES: 0..15).
*
* @param[out] cosPtr                   - (pointer to) mapped COS value for TM-TC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTmGlueDropTcToCosGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    tmTc,
    OUT GT_U32   *cosPtr
)
{
    GT_U32    regAddr;          /* register address   */
    GT_U32    entryIndex;       /* index to the table */
    GT_U32    fieldOffset;      /* field offset       */
    GT_STATUS rc;               /* return code        */
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_TM_GLUE_DROP_TM_TC_CHECK_MAC(tmTc);
    CPSS_NULL_PTR_CHECK_MAC(cosPtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    entryIndex = tmTc/8;
    fieldOffset = (tmTc%8)*3;
    regAddr = regsAddrPtr->TMDROP.TMDROPTMTCMap8To87[entryIndex];

    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, 3, cosPtr);

    return rc;
}

/**
* @internal cpssDxChTmGlueDropTcToCosGet function
* @endinternal
*
* @brief   The function retrieves the TM TC to COS mapping COS value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] tmTc                     - TM traffic class
*                                      (APPLICABLE RANGES: 0..15).
*
* @param[out] cosPtr                   - (pointer to) mapped COS value for TM-TC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueDropTcToCosGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    tmTc,
    OUT GT_U32   *cosPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueDropTcToCosGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tmTc, cosPtr));

    rc = internal_cpssDxChTmGlueDropTcToCosGet(devNum, tmTc, cosPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tmTc, cosPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

