/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* tgfBridgeGen.c
*
* DESCRIPTION:
*       Generic API implementation for TCAM
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*
*******************************************************************************/
#ifdef COW_INTERNAL_COMPILE
    /* those 3 are defined , so remove them */
    #undef EXMXPM_FAMILY
    #undef EX_FAMILY
    #undef CHX_FAMILY

    /* define only the tested one */
    #define CHX_FAMILY

#endif /*COW_INTERNAL_COMPILE*/

/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
/*#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfTcamGen.h>



/******************************************************************************\
 *                           Private declarations                             *
\******************************************************************************/



/******************************************************************************\
 *                     Private function's implementation                      *
\******************************************************************************/

#ifdef CHX_FAMILY

/**
* @internal prvTgfConvertGenericToDxChTcamClient function
* @endinternal
*
* @brief   Convert generic TCAM client into device specific mode.
*
* @param[in] tcamClient               - generic TCAM clint
*
* @param[out] dxChTcamClientPtr        - (pointer to) DxCh TCAM client
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChTcamClient
(
    IN  PRV_TGF_TCAM_CLIENT_ENT     tcamClient,
    OUT CPSS_DXCH_TCAM_CLIENT_ENT   *dxChTcamClientPtr
)
{
    switch (tcamClient)
    {
        PRV_TGF_SWITCH_CASE_MAC(*dxChTcamClientPtr, PRV_TGF_TCAM_IPCL_0_E,
                                CPSS_DXCH_TCAM_IPCL_0_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChTcamClientPtr, PRV_TGF_TCAM_IPCL_1_E,
                                CPSS_DXCH_TCAM_IPCL_1_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChTcamClientPtr, PRV_TGF_TCAM_IPCL_2_E,
                                CPSS_DXCH_TCAM_IPCL_2_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChTcamClientPtr, PRV_TGF_TCAM_EPCL_E,
                                CPSS_DXCH_TCAM_EPCL_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChTcamClientPtr, PRV_TGF_TCAM_TTI_E,
                                CPSS_DXCH_TCAM_TTI_E);
        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChTcamBlockInfo function
* @endinternal
*
* @brief   Convert generic into device specific TCAM block info
*
* @param[in] floorInfoPtr             - (pointer to) TCAM block info
*
* @param[out] dxChFloorInfoPtr         - (pointer to) DxCh TCAM block info
*                                       None
*/
static GT_VOID prvTgfConvertGenericToDxChTcamBlockInfo
(
    IN  PRV_TGF_TCAM_BLOCK_INFO_STC         *floorInfoPtr,
    OUT CPSS_DXCH_TCAM_BLOCK_INFO_STC       *dxChFloorInfoPtr
)
{
    /* convert common part into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChFloorInfoPtr, floorInfoPtr, group);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChFloorInfoPtr, floorInfoPtr, hitNum);
}

/**
* @internal prvTgfConvertDxChToGenericTcamBlockInfo function
* @endinternal
*
* @brief   Convert device specific into generic TCAM block info
*
* @param[in] dxChFloorInfoPtr         - (pointer to) DxCh TCAM block info
*
* @param[out] floorInfoPtr             - (pointer to) TCAM block info
*                                       None
*/
static GT_VOID prvTgfConvertDxChToGenericTcamBlockInfo
(
    IN  CPSS_DXCH_TCAM_BLOCK_INFO_STC   *dxChFloorInfoPtr,
    OUT PRV_TGF_TCAM_BLOCK_INFO_STC     *floorInfoPtr
)
{
    /* convert common part into device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChFloorInfoPtr, floorInfoPtr, group);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChFloorInfoPtr, floorInfoPtr, hitNum);
}

#endif
/**
* @internal prvTgfTcamClientGroupSet function
* @endinternal
*
* @brief   Enable/Disable client per TCAM group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] tcamClient               - TCAM client.
* @param[in] tcamGroup                - TCAM group id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4)
* @param[in] enable                   - GT_TRUE: TCAM client is enabled
*                                      GT_FALSE: TCAM client is disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS prvTgfTcamClientGroupSet
(
    IN  PRV_TGF_TCAM_CLIENT_ENT tcamClient,
    IN  GT_U32                  tcamGroup,
    IN  GT_BOOL                 enable
)
{
#ifdef CHX_FAMILY
    GT_U8                       devNum  = 0;
    GT_STATUS                   rc, rc1 = GT_OK;
    GT_PORT_GROUPS_BMP          portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    CPSS_DXCH_TCAM_CLIENT_ENT   dxChTcamClient;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    if( GT_FALSE == usePortGroupsBmp )
    {
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    }
    else
    {
        portGroupsBmp = currPortGroupsBmp;
    }

    rc = prvTgfConvertGenericToDxChTcamClient(tcamClient, &dxChTcamClient);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChTcamClient FAILED, rc = [%d]", rc);
        return rc;
    }

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChTcamPortGroupClientGroupSet(devNum,
                                                 portGroupsBmp,
                                                 dxChTcamClient,
                                                 tcamGroup,
                                                 enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTcamPortGroupClientGroupSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tcamClient);
    TGF_PARAM_NOT_USED(tcamGroup);
    TGF_PARAM_NOT_USED(enable);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTcamClientGroupGet function
* @endinternal
*
* @brief   Gets status (Enable/Disable) for client per TCAM group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] tcamClient               - TCAM client.
*
* @param[out] tcamGroupPtr             - (pointer to) TCAM group id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4)
* @param[out] enablePtr                - (pointer to) TCAM client status.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfTcamClientGroupGet
(
    IN  PRV_TGF_TCAM_CLIENT_ENT tcamClient,
    OUT GT_U32                  *tcamGroupPtr,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_U8       devNum  = prvTgfDevNum;

#ifdef CHX_FAMILY
    GT_STATUS                   rc;
    GT_PORT_GROUPS_BMP          portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    CPSS_DXCH_TCAM_CLIENT_ENT   dxChTcamClient;

    if( GT_FALSE == usePortGroupsBmp )
    {
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    }
    else
    {
        portGroupsBmp = currPortGroupsBmp;
    }

    rc = prvTgfConvertGenericToDxChTcamClient(tcamClient, &dxChTcamClient);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChTcamClient FAILED, rc = [%d]", rc);
        return rc;
    }

    rc = cpssDxChTcamPortGroupClientGroupGet(devNum,
                                             portGroupsBmp,
                                             dxChTcamClient,
                                             tcamGroupPtr,
                                             enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTcamPortGroupClientGroupGet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(tcamClient);
    TGF_PARAM_NOT_USED(tcamGroupPtr);
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTcamIndexRangeHitNumAndGroupSet function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
*         APPLICABLE DEVICES:
*         Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*         INPUTS:
*         floorIndex     - floor index (APPLICABLE RANGES: 0..11)
*         floorInfoArr    - array of group ID and lookup number for all TCAM blocks in floor.
*         Index 0 represent banks 0-5 and index 1 represent banks 6-11.
*         APPLICABLE DEVICES: Bobcat2, Bobcat3, Aldrin2, Falcon.
*         Index 0 represent banks 0-1, index 1 represent banks 2-3,
*         index 2 represent banks 4-5, index 3 represent banks 6-7,
*         index 4 represent banks 8-9, index 5 represent banks 10-11.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] floorIndex               - floor index (APPLICABLE RANGES: 0..11)
* @param[in] floorInfoArr[PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS] - array of group ID and lookup number for all TCAM blocks in floor.
*                                      Index 0 represent banks 0-5 and index 1 represent  banks  6-11.
*                                      APPLICABLE DEVICES: Bobcat2, Bobcat3; Aldrin2; Falcon.
*                                      Index 0 represent banks 0-1, index 1 represent  banks  2-3,
*                                      index 2 represent banks 4-5, index 3 represent  banks  6-7,
*                                      index 4 represent banks 8-9, index 5 represent  banks  10-11.
*                                      APPLICABLE DEVICES: Caelum, Aldrin, AC3X.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS prvTgfTcamIndexRangeHitNumAndGroupSet
(
    IN GT_U32                           floorIndex,
    IN PRV_TGF_TCAM_BLOCK_INFO_STC      floorInfoArr[PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS]
)
{
#ifdef CHX_FAMILY
    GT_U8                       devNum  = 0;
    GT_STATUS                   rc, rc1 = GT_OK;
    CPSS_DXCH_TCAM_BLOCK_INFO_STC  dxChFloorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];
    GT_U32                      blockNumber;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    for (blockNumber = 0; blockNumber < PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS; blockNumber++)
    {
        prvTgfConvertGenericToDxChTcamBlockInfo(&floorInfoArr[blockNumber], &dxChFloorInfoArr[blockNumber]);
    }
    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChTcamIndexRangeHitNumAndGroupSet(devNum,
                                                     floorIndex,
                                                     dxChFloorInfoArr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTcamIndexRangeHitNumAndGroupSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(floorIndex);
    TGF_PARAM_NOT_USED(floorInfoArr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTcamIndexRangeHitNumAndGroupGet function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
*         APPLICABLE DEVICES:
*         Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*         INPUTS:
*         floorIndex     - floor index (APPLICABLE RANGES: 0..11)
*         OUTPUTS:
*         floorInfoArr    - array of group ID and lookup number for all TCAM blocks in floor.
*         Index 0 represent banks 0-5 and index 1 represent banks 6-11.
*         APPLICABLE DEVICES: Bobcat2, Bobcat3, Aldrin2, Falcon.
*         Index 0 represent banks 0-1, index 1 represent banks 2-3,
*         index 2 represent banks 4-5, index 3 represent banks 6-7,
*         index 4 represent banks 8-9, index 5 represent banks 10-11.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] floorIndex               - floor index (APPLICABLE RANGES: 0..11)
*
* @param[out] floorInfoArr[PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS] - array of group ID and lookup number for all TCAM blocks in floor.
*                                      Index 0 represent banks 0-5 and index 1 represent  banks  6-11.
*                                      APPLICABLE DEVICES: Bobcat2, Bobcat3; Aldrin2; Falcon.
*                                      Index 0 represent banks 0-1, index 1 represent  banks  2-3,
*                                      index 2 represent banks 4-5, index 3 represent  banks  6-7,
*                                      index 4 represent banks 8-9, index 5 represent  banks  10-11.
*                                      APPLICABLE DEVICES: Caelum, Aldrin, AC3X.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS prvTgfTcamIndexRangeHitNumAndGroupGet
(
    IN GT_U32                               floorIndex,
    OUT PRV_TGF_TCAM_BLOCK_INFO_STC         floorInfoArr[PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS]
)
{
    GT_U8       devNum  = prvTgfDevNum;
    GT_U32      blockNumber;

#ifdef CHX_FAMILY
    GT_STATUS                   rc;
    CPSS_DXCH_TCAM_BLOCK_INFO_STC  dxChFloorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];

    rc = cpssDxChTcamIndexRangeHitNumAndGroupGet(devNum,
                                                 floorIndex,
                                                 dxChFloorInfoArr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTcamIndexRangeHitNumAndGroupGet FAILED, rc = [%d]", rc);
        return rc;
    }

    for (blockNumber = 0; blockNumber < PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS; blockNumber++)
    {
        prvTgfConvertDxChToGenericTcamBlockInfo(&dxChFloorInfoArr[blockNumber], &floorInfoArr[blockNumber]);
    }
    return GT_OK;
#endif

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(floorIndex);
    TGF_PARAM_NOT_USED(floorInfoArr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTcamFloorsNumGet function
* @endinternal
*
* @brief   Return number of TCAM floors
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*                                       number of TCAM floors
*/
GT_U32 prvTgfTcamFloorsNumGet
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    tcamEntriesNum; /* number of entries in TCAM */

    rc = cpssDxChCfgTableNumEntriesGet(prvTgfDevNum, CPSS_DXCH_CFG_TABLE_TTI_TCAM_E, &tcamEntriesNum);
    if (rc != GT_OK)
    {
        return 0;
    }

    return tcamEntriesNum / CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS;
}

