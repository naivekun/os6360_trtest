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
* @file prvCpssDxChLpmHw.c
*
* @brief LPM HW releated internal functions
*
* @version   3
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/private/lpm/hw/prvCpssDxChLpmHw.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRam.h>
#include <cpss/dxCh/dxChxGen/ip/private/prvCpssDxChIp.h>

/* LPM bucket - fields offsets in bits */
#define PRV_CPSS_DXCH_LPM_RAM_POINTER_TYPE_OFFSET_CNS                       0
#define PRV_CPSS_DXCH_LPM_RAM_POINT_TO_LOOKUP_1_OFFSET_CNS                  2
#define PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_GROUP_SCOPE_LEVEL_OFFSET_CNS          2
#define PRV_CPSS_DXCH_LPM_RAM_UC_RPF_CHECK_OFFSET_CNS                       4
#define PRV_CPSS_DXCH_LPM_RAM_UC_SIP_SA_CHECK_MISMATCH_OFFSET_CNS           5
#define PRV_CPSS_DXCH_LPM_RAM_ACTIVITY_STATE_OFFSET_CNS                     6
#define PRV_CPSS_DXCH_LPM_RAM_NEXT_BUCKET_TYPE_OFFSET_CNS                   3
#define PRV_CPSS_DXCH_LPM_RAM_FIFTH_RANGE_OFFSET_CNS                        5
#define PRV_CPSS_DXCH_LPM_RAM_NEXT_BUCKET_POINTER_OFFSET_CNS                5
#define PRV_CPSS_DXCH_LPM_RAM_NEXTHOP_OR_ECMP_POINTER_OFFSET_CNS            7
#define PRV_CPSS_DXCH_LPM_RAM_COMPRESSED_2_NEXT_BUCKET_POINTER_OFFSET_CNS   13

/* LPM bucket - fields length in bits */
#define PRV_CPSS_DXCH_LPM_RAM_POINTER_TYPE_LENGTH_CNS                       2
#define PRV_CPSS_DXCH_LPM_RAM_POINT_TO_LOOKUP_1_LENGTH_CNS                  1
#define PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_GROUP_SCOPE_LEVEL_LENGTH_CNS          2
#define PRV_CPSS_DXCH_LPM_RAM_UC_RPF_CHECK_LENGTH_CNS                       1
#define PRV_CPSS_DXCH_LPM_RAM_UC_SIP_SA_CHECK_MISMATCH_LENGTH_CNS           1
#define PRV_CPSS_DXCH_LPM_RAM_ACTIVITY_STATE_LENGTH_CNS                     1
#define PRV_CPSS_DXCH_LPM_RAM_NEXT_BUCKET_TYPE_LENGTH_CNS                   2
#define PRV_CPSS_DXCH_LPM_RAM_FIFTH_RANGE_LENGTH_CNS                        8
#define PRV_CPSS_DXCH_LPM_RAM_NEXT_BUCKET_POINTER_LENGTH_CNS                24
#define PRV_CPSS_DXCH_LPM_RAM_NEXTHOP_OR_ECMP_POINTER_LENGTH_CNS            15
#define PRV_CPSS_DXCH_LPM_RAM_COMPRESSED_2_NEXT_BUCKET_POINTER_LENGTH_CNS   19

/**
* @internal prvCpssDxChLpmHwVrfEntryWrite function
* @endinternal
*
* @brief   Write a VRF table entry
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] shadowType               - the shadow type
* @param[in] vrId                     - the virtual router id
* @param[in] protocol                 - the protocol
* @param[in] nodeType                 - the type of the node
* @param[in] rootNodeAddr             - the line number of the root node
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_OUT_OF_RANGE          - rootNodeAddr is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
* @note VRF root nodeType can be only CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E or
*       CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E.
*
*/
GT_STATUS prvCpssDxChLpmHwVrfEntryWrite
(
    IN  GT_U8                                   devNum,
    IN  PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT       shadowType,
    IN  GT_U32                                  vrId,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol,
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT     nodeType,
    IN  GT_U32                                  rootNodeAddr
)
{
    CPSS_DXCH_TABLE_ENT         tableType;
    GT_U32                      bucketTypeValue, data;
    GT_STATUS                   rc;
    GT_U32                      bucketTypeOffset;
    /*Lock the access to per device data base in order to avoid corruption*/
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E);

    /*Unlock the access to per device data base*/
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    switch (protocol)
    {
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
            tableType = CPSS_DXCH_SIP5_TABLE_IPV4_VRF_E;
            break;

        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
            tableType = CPSS_DXCH_SIP5_TABLE_IPV6_VRF_E;
            break;

        case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
            tableType = CPSS_DXCH_SIP5_TABLE_FCOE_VRF_E;
            break;

        default:

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (nodeType)
    {
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
            bucketTypeValue = 0;

            break;

        case CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E:
            bucketTypeValue = 1;
            break;

        default:

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
    {
        bucketTypeOffset = 20;
    }
    else
    {
        bucketTypeOffset = 24;
    }
    data = ((rootNodeAddr & BIT_MASK_MAC(bucketTypeOffset)) | (bucketTypeValue << bucketTypeOffset));

    /*Lock the access to per device data base in order to avoid corruption*/
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    rc = prvCpssDxChWriteTableEntry(devNum,
                                    tableType,
                                    vrId,
                                    &data);

    /*Unlock the access to per device data base*/
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChLpmHwVrfEntryRead function
* @endinternal
*
* @brief   Read a VRF table entry
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] vrId                     - the virtual router id
* @param[in] protocol                 - the protocol
*
* @param[out] nodeTypePtr              - the type of the node
* @param[out] rootNodeAddrPtr          - the line number of the root node
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
* @note VRF root nodeTypePtr can be only CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E
*       or CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E.
*
*/
GT_STATUS prvCpssDxChLpmHwVrfEntryRead
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  vrId,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol,
    OUT CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT     *nodeTypePtr,
    OUT GT_U32                                  *rootNodeAddrPtr
)
{
    CPSS_DXCH_TABLE_ENT tableType;
    GT_U32                  data;
    GT_STATUS               rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(nodeTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(rootNodeAddrPtr);

    switch (protocol)
    {
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
            tableType = CPSS_DXCH_SIP5_TABLE_IPV4_VRF_E;
            break;

        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
            tableType = CPSS_DXCH_SIP5_TABLE_IPV6_VRF_E;
            break;

        case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
            tableType = CPSS_DXCH_SIP5_TABLE_FCOE_VRF_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChReadTableEntry(devNum,
                                   tableType,
                                   vrId,
                                   &data);
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        *rootNodeAddrPtr = (data & 0x00FFFF);/* 20 bits */

        if ((data >> 20) & 0x1)
        {
            *nodeTypePtr = CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E;
        }
        else
        {
            *nodeTypePtr = CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E;
        }

        return rc;
    }

    /* the bucket type bit */
    if ((data >> 24) & 0x1)
    {
        *nodeTypePtr = CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E;
    }
    else
    {
        *nodeTypePtr = CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E;
    }
    *rootNodeAddrPtr = (data & 0xFFFFFF);

    return rc;
}

/**
* @internal prvCpssDxChLpmHwVrfEntryReadSip6 function
* @endinternal
 *
* @brief   Read a VRF table entry
 *
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2;
*         DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum;
*         Aldrin; AC3X; Bobcat3.
 *
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
* @note VRF root nodeTypePtr can be only CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E
*       CPSS_DXCH_LPM_LEAFE_NODE_PTR_TYPE_E,
*       or CPSS_DXCH_LPM_ONE_COMPRESSED_NODE_PTR_TYPE_E.
*
* @param[in] devNum                   - the device number
* @param[in] vrId                     - the virtual router id
* @param[in] protocol                 - the
* @param[out] nodeTypePtr              - the type of the node
* @param[out] rootNodeAddrPtr          - the line number of the root node
*/
GT_STATUS prvCpssDxChLpmHwVrfEntryReadSip6
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  vrId,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol,
    OUT PRV_CPSS_DXCH_LPM_CHILD_TYPE_ENT        *nodeTypePtr,
    OUT GT_U32                                  *rootNodeAddrPtr
)
{
    CPSS_DXCH_TABLE_ENT tableType;
    GT_U32                  data;
    GT_STATUS               rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E |
          CPSS_BOBCAT2_E  | CPSS_CAELUM_E   | CPSS_ALDRIN_E   | CPSS_BOBCAT3_E  | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(nodeTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(rootNodeAddrPtr);

    switch (protocol)
    {
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
            tableType = CPSS_DXCH_SIP5_TABLE_IPV4_VRF_E;
            break;

        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
            tableType = CPSS_DXCH_SIP5_TABLE_IPV6_VRF_E;
            break;

        case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
            tableType = CPSS_DXCH_SIP5_TABLE_FCOE_VRF_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChReadTableEntry(devNum,
                                   tableType,
                                   vrId,
                                   &data);

    /* the bucket type bit 20 */
    switch ((data >> 20) & 0x1)
    {
        case 0:/* regular */
            *nodeTypePtr = PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E;
            break;
        case 1:/* one compress  */
            *nodeTypePtr = PRV_CPSS_DXCH_LPM_CHILD_COMPRESSED_TYPE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
    }
    *rootNodeAddrPtr = (data & 0xFFFFF);/* 0-19 bits */

    return rc;
}

/**
* @internal prvCpssDxChLpmHwEcmpEntryWrite function
* @endinternal
*
* @brief   Write an ECMP entry
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] ecmpEntryIndex           - the index of the entry in the ECMP table
* @param[in] ecmpEntryPtr             - (pointer to) the ECMP entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
*/
GT_STATUS prvCpssDxChLpmHwEcmpEntryWrite
(
    IN GT_U8                                devNum,
    IN GT_U32                               ecmpEntryIndex,
    IN PRV_CPSS_DXCH_LPM_ECMP_ENTRY_STC     *ecmpEntryPtr
)
{
    GT_U32  data = 0;
    GT_U32  numOfNextHopEntries;
    GT_U32  maximalIndex;
    GT_U32  value;
    CPSS_DXCH_TABLE_ENT         tableType;

    if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum) == GT_TRUE)
    {
        /*ECMP table point to indirect access table*/
        maximalIndex = prvCpssDxchTableIpvxRouterEcmpPointerNumEntriesGet(devNum);
    }
    else
    {
        maximalIndex = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerNextHop;
    }

    /* there are only 15 bits in the base index field*/
     numOfNextHopEntries = MIN(BIT_15, maximalIndex);

    if ((ecmpEntryPtr->numOfPaths == 0) || (ecmpEntryPtr->numOfPaths > BIT_12))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (ecmpEntryPtr->nexthopBaseIndex >= numOfNextHopEntries)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((ecmpEntryPtr->nexthopBaseIndex + ecmpEntryPtr->numOfPaths) > numOfNextHopEntries)
    {
        /* last entry must not be out of the NH table */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
    {
        tableType = CPSS_DXCH_SIP6_TABLE_IPVX_ECMP_E;
        switch (ecmpEntryPtr->multiPathMode) {
        case PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E:
            value = 0;
            break;
        case PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E:
            value = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        data = ((BOOL2BIT_MAC(ecmpEntryPtr->randomEnable) << 15)|
                ((ecmpEntryPtr->numOfPaths - 1) << 16) |
                (value  << 28) |
                (ecmpEntryPtr->nexthopBaseIndex));
    }
    else
    {
        tableType = CPSS_DXCH_SIP5_TABLE_ECMP_E;

        data = ((BOOL2BIT_MAC(ecmpEntryPtr->randomEnable) << 15)|
                ((ecmpEntryPtr->numOfPaths - 1) << 16) |
                (ecmpEntryPtr->nexthopBaseIndex));
    }
    return prvCpssDxChWriteTableEntry(devNum,
                                      tableType,
                                      ecmpEntryIndex,
                                      &data);
}

/**
* @internal prvCpssDxChLpmHwEcmpEntryRead function
* @endinternal
*
* @brief   Read an ECMP entry
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] ecmpEntryIndex           - the index of the entry in the ECMP table
*
* @param[out] ecmpEntryPtr             - (pointer to) the ECMP entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
*/
GT_STATUS prvCpssDxChLpmHwEcmpEntryRead
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              ecmpEntryIndex,
    OUT PRV_CPSS_DXCH_LPM_ECMP_ENTRY_STC    *ecmpEntryPtr
)
{
    GT_U32      data = 0;
    GT_STATUS   rc;
    GT_U32      value = 0;
    CPSS_DXCH_TABLE_ENT         tableType;
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
    {
        tableType = CPSS_DXCH_SIP6_TABLE_IPVX_ECMP_E;
    }
    else
    {
        tableType = CPSS_DXCH_SIP5_TABLE_ECMP_E;
    }
    rc = prvCpssDxChReadTableEntry(devNum,
                                   tableType,
                                   ecmpEntryIndex,
                                   &data);
    if (rc == GT_OK)
    {
        ecmpEntryPtr->randomEnable = BIT2BOOL_MAC((data >> 15) & 0x1);
        ecmpEntryPtr->numOfPaths = ((data >> 16) & 0xfff) + 1;
        ecmpEntryPtr->nexthopBaseIndex = (data & 0x7fff);

        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
        {
            value = (data >> 28) & 0x1;
            switch (value) {
            case 0:
                ecmpEntryPtr->multiPathMode = PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E;
                break;
            case 1:
                ecmpEntryPtr->multiPathMode = PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
    }

    return rc;
}


/**
* @internal prvCpssDxChLpmLeafEntryCalcLpmHwIndexFromSwIndex function
* @endinternal
*
* @brief   Calculate HW Index for an LPM leaf entry
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] leafIndex                - leaf index within the LPM PBR block
*
* @param[out] lpmIndexPtr              - (pointer to) leaf index within the LPM PBR block in HW representation
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChLpmLeafEntryCalcLpmHwIndexFromSwIndex
(
    IN GT_U8            devNum,
    IN GT_U32           leafIndex,
    OUT GT_U32          *lpmIndexPtr
)
{
    GT_U32              pbrBase_k;                   /* the base address of PBR - SW address */
    GT_U32              numOfLinesInFirstPbrBlock_m; /* number of lines occupied by PBR in the first PBR block,
                                                        rest of the lines are used for IP LPM*/
    GT_U32              numOfLinesInBlock_n;         /* number of legal lines in the memory block*/
    GT_U32              totalNumOfLinesInBlockIncludingGap_h;/* number of lines in the memory block
                                                                including gap of lines that cannot be used */
    GT_U32              blockIdFromStartPbr_e;      /* block id from the start of PBR blocks.
                                                       if we have 3 blocks for example then the
                                                       blocks ID will be 0, 1, 2 */
    GT_U32              addrOfLastPbrBlock;         /* address of the last PBR found that
                                                       is suitable for adding the needed PBR */
    GT_U32              offsetInLastPbrBlock;       /* offset of the new PBR in the last PBR block found */


    /* check that leafIndex is a legal PBR index: 0...(maxNumOfPbrEntries-1) */
    if(leafIndex>=PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.maxNumOfPbrEntries)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "leafIndex[%d] must be less than maxNumOfPbrEntries[%d]",
            leafIndex,
            PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.maxNumOfPbrEntries);
    }
    pbrBase_k=PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.indexForPbr;
    numOfLinesInBlock_n = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
    {
       numOfLinesInBlock_n/=2;
    }
    totalNumOfLinesInBlockIncludingGap_h = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap;
    numOfLinesInFirstPbrBlock_m = numOfLinesInBlock_n-(pbrBase_k%totalNumOfLinesInBlockIncludingGap_h);

    if (leafIndex<numOfLinesInFirstPbrBlock_m)
    {
        *lpmIndexPtr = pbrBase_k+leafIndex;
    }
    else
    {

        /* LPM with PBR blocks in case numOfLinesInBlock_n < totalNumOfLinesInBlockIncludingGap_h

           blocks:  0       1               17          18          19 --> total 20 blocks
                    -----   -----   .....   -----  e--> ----- e-->  -----  --
                    |   |   |   |           |n-m|       |   |       |   |  ^
                    | n |   | n |      k--> |---|       | n |       | n |  ^
                    |   |   |   |           | m |       |   |       |   |  ^
                    |---|   |---|           |---|       |---|       |---|  ^ h
                    |   |   |   |           |   |       |   |       |   |  ^
                    |   |   |   |           |   |       |   |       |   |  ^
                    -----   -----           -----       -----       -----  --
        */
        /* the formula for finding the lpmIndex (hw index)calculated from leafIndex (sw index ) is:
           lpmIndex = (e+(k/h))*h + ((x-m)%n)
           e = (leafIndex - m)/n + 1 --> when the leafIndex is bigger the size allocated for the
           PBR in the first PBR block (m), we need to deduct the size in the first PBR block (m) from the leafIndex,
           then we need to divide it by the size of the block (n) in order to get the block index where it should be set.
           since we get values from 0... we need to add + 1 in order to get to the correct block index that can be
           used in the next formula calculation */
        blockIdFromStartPbr_e = ((leafIndex-numOfLinesInFirstPbrBlock_m)/numOfLinesInBlock_n)+1;
        /* addrOfLastPbrBlock = (e+(k/h))* h --> k is the base address of PBR (SW address),
           h is the total size of a block including the gap, k/h gives the block index where the PBR starts,
           then we need to progress to the block calculated above (e) where our index will reside.
           in the end we have the PBR block index from the beginning of the LPM blocks where we will set the data.
           now we want to translate it to an address so we multiply it by the total size of the block including the gap (h)*/
        addrOfLastPbrBlock = (blockIdFromStartPbr_e+(pbrBase_k/totalNumOfLinesInBlockIncludingGap_h))*totalNumOfLinesInBlockIncludingGap_h;
        /* offsetInLastPbrBlock = (leafIndex - m) % n --> in order to get the offset of the PBR in the
           last suitable block we found, we need to deduct from leafIndex the size in the first PBR block,
           and to do modulo n in order to get the remaining that will fit the last suitable PBR block */
        offsetInLastPbrBlock = ((leafIndex-numOfLinesInFirstPbrBlock_m)%numOfLinesInBlock_n);
        *lpmIndexPtr = addrOfLastPbrBlock + offsetInLastPbrBlock;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChLpmLeafEntryCalcLpmSwIndexFromHwIndex function
* @endinternal
*
* @brief   Calculate SW Index for an LPM leaf HW index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] leafHwIndex              - leaf HW index within the LPM PBR block in HW representation
*
* @param[out] leafSwIndexPtr           - (pointer to)leaf index within the LPM PBR block SW representation
*
* @retval GT_OK                    - on success
*/
GT_VOID prvCpssDxChLpmLeafEntryCalcLpmSwIndexFromHwIndex
(
    IN GT_U8            devNum,
    IN GT_U32           leafHwIndex,
    OUT GT_U32          *leafSwIndexPtr
)
{
    GT_U32              pbrBase_k;                   /* the base address of PBR - SW address */
    GT_U32              numOfLinesInFirstPbrBlock_m; /* number of lines occupied by PBR in the first PBR block,
                                                        rest of the lines are used for IP LPM*/
    GT_U32              numOfLinesInBlock_n;         /* number of legal lines in the memory block*/
    GT_U32              totalNumOfLinesInBlockIncludingGap_h;/* number of lines in the memory block
                                                                including gap of lines that cannot be used */
    GT_U32              numberOfGapsFromBeginnigOfPbrBase; /* number of "holes" from the beggining of the
                                                              PBR to the checked HW Index */
    GT_U32              gapSize_w;                         /* the size of the "hole" in each LPM block */

    pbrBase_k=PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.indexForPbr;
    numOfLinesInBlock_n = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
    {
       numOfLinesInBlock_n/=2;
    }

    totalNumOfLinesInBlockIncludingGap_h = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap;
    numOfLinesInFirstPbrBlock_m = numOfLinesInBlock_n-(pbrBase_k%totalNumOfLinesInBlockIncludingGap_h);

    if (leafHwIndex<numOfLinesInFirstPbrBlock_m)
    {
        *leafSwIndexPtr = leafHwIndex;
    }
    else
    {

        /* LPM with PBR blocks in case numOfLinesInBlock_n < totalNumOfLinesInBlockIncludingGap_h

           blocks:  0       1               17          18          19 --> total 20 blocks
                    -----   -----   .....   -----  e--> ----- e-->  -----  --
                    |   |   |   |           |n-m|       |   |       |   |  ^
                    | n |   | n |      k--> |---|       | n |       | n |  ^
                    |   |   |   |           | m |       |   |       |   |  ^
                    |---|   |---|           |---|       |---|       |---|  ^ h
                    |   |   |   |           |   |       |   |       |   |  ^
                    | w |   | w |           | w |       | w |       | w |  ^
                    -----   -----           -----       -----       -----  --
        */

        numberOfGapsFromBeginnigOfPbrBase = (leafHwIndex/totalNumOfLinesInBlockIncludingGap_h);
        if (numberOfGapsFromBeginnigOfPbrBase==0)
        {
            /* in case the HW Index value is in bigger the numOfLinesInFirstPbrBlock_m
               but smaller then the size of one block (h)--> the index resize in the first
               full block after the block partialy occupied by m */
            numberOfGapsFromBeginnigOfPbrBase=1;
        }
        gapSize_w = totalNumOfLinesInBlockIncludingGap_h - numOfLinesInBlock_n;
        *leafSwIndexPtr = (leafHwIndex - (numberOfGapsFromBeginnigOfPbrBase * gapSize_w));
    }

    return;
}

/**
* @internal prvCpssDxChLpmNextPointerDataBuild function
* @endinternal
*
* @brief   Local function for LPM bucket next pointer HW format build.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2.
*
* @param[in] dev                      -  number
* @param[in] nextPointerData          - the next pointer data.
*
* @param[out] hwDataPtr                - node data in HW format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if any parameter is wrong
*/
GT_STATUS prvCpssDxChLpmNextPointerDataBuild
(
    IN  GT_U8                                       dev,
    IN  CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC         nextPointerData,
    OUT GT_U32                                      *hwDataPtr
)
{
    GT_U32      data = 0, maxMemSize;
    GT_U32      entryIndex;

    switch (nextPointerData.pointerType)
    {
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E:
            if (nextPointerData.pointerType == CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E)
            {
                if ((nextPointerData.pointerData.nextNodePointer.range5Index < 1)
                     || (nextPointerData.pointerData.nextNodePointer.range5Index > 255))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                /* convert to bytes */
                maxMemSize = PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS *
                    (1 << PRV_CPSS_DXCH_LPM_RAM_COMPRESSED_2_NEXT_BUCKET_POINTER_LENGTH_CNS);

                if (nextPointerData.pointerData.nextNodePointer.nextPointer >= maxMemSize)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }

            /* convert to bytes */
            maxMemSize =
                PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS *
                PRV_CPSS_DXCH_PP_MAC(dev)->accessTableInfoPtr[CPSS_DXCH_SIP5_TABLE_LPM_MEM_E].maxNumOfEntries;


            if (nextPointerData.pointerData.nextNodePointer.nextPointer >= maxMemSize)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            if (nextPointerData.pointerType == CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E)
            {
                U32_SET_FIELD_MAC(data,
                                  PRV_CPSS_DXCH_LPM_RAM_FIFTH_RANGE_OFFSET_CNS,
                                  PRV_CPSS_DXCH_LPM_RAM_FIFTH_RANGE_LENGTH_CNS,
                                  nextPointerData.pointerData.nextNodePointer.range5Index);
                U32_SET_FIELD_MAC(data,
                                  PRV_CPSS_DXCH_LPM_RAM_COMPRESSED_2_NEXT_BUCKET_POINTER_OFFSET_CNS,
                                  PRV_CPSS_DXCH_LPM_RAM_COMPRESSED_2_NEXT_BUCKET_POINTER_LENGTH_CNS,
                                  nextPointerData.pointerData.nextNodePointer.nextPointer);
            }
            else
            {
                U32_SET_FIELD_MAC(data,
                                  PRV_CPSS_DXCH_LPM_RAM_NEXT_BUCKET_POINTER_OFFSET_CNS,
                                  PRV_CPSS_DXCH_LPM_RAM_NEXT_BUCKET_POINTER_LENGTH_CNS,
                                  nextPointerData.pointerData.nextNodePointer.nextPointer);
            }
            U32_SET_FIELD_MAC(data,
                              PRV_CPSS_DXCH_LPM_RAM_POINT_TO_LOOKUP_1_OFFSET_CNS,
                              PRV_CPSS_DXCH_LPM_RAM_POINT_TO_LOOKUP_1_LENGTH_CNS,
                              nextPointerData.pointerData.nextNodePointer.pointToSipTree);
            U32_SET_FIELD_MAC(data,
                              PRV_CPSS_DXCH_LPM_RAM_POINTER_TYPE_OFFSET_CNS,
                              PRV_CPSS_DXCH_LPM_RAM_POINTER_TYPE_LENGTH_CNS,
                              0);
            U32_SET_FIELD_MAC(data,
                              PRV_CPSS_DXCH_LPM_RAM_NEXT_BUCKET_TYPE_OFFSET_CNS,
                              PRV_CPSS_DXCH_LPM_RAM_NEXT_BUCKET_TYPE_LENGTH_CNS,
                              nextPointerData.pointerType);
            break;

        case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
        case CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E:
        case CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E:
            entryIndex = nextPointerData.pointerData.nextHopOrEcmpPointer.entryIndex;
            if (nextPointerData.pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
            {
                maxMemSize = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.routerNextHop;
            }
            else
            {
                maxMemSize = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.ecmpQos;
            }
            if (entryIndex >= maxMemSize)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            U32_SET_FIELD_MAC(data,
                              PRV_CPSS_DXCH_LPM_RAM_NEXTHOP_OR_ECMP_POINTER_OFFSET_CNS,
                              PRV_CPSS_DXCH_LPM_RAM_NEXTHOP_OR_ECMP_POINTER_LENGTH_CNS,
                              entryIndex);
            U32_SET_FIELD_MAC(data,
                              PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_GROUP_SCOPE_LEVEL_OFFSET_CNS,
                              PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_GROUP_SCOPE_LEVEL_LENGTH_CNS,
                              nextPointerData.pointerData.nextHopOrEcmpPointer.ipv6McGroupScopeLevel);
            U32_SET_FIELD_MAC(data,
                              PRV_CPSS_DXCH_LPM_RAM_UC_RPF_CHECK_OFFSET_CNS,
                              PRV_CPSS_DXCH_LPM_RAM_UC_RPF_CHECK_LENGTH_CNS,
                              BOOL2BIT_MAC(nextPointerData.pointerData.nextHopOrEcmpPointer.ucRpfCheckEnable));
            U32_SET_FIELD_MAC(data,
                              PRV_CPSS_DXCH_LPM_RAM_UC_SIP_SA_CHECK_MISMATCH_OFFSET_CNS,
                              PRV_CPSS_DXCH_LPM_RAM_UC_SIP_SA_CHECK_MISMATCH_LENGTH_CNS,
                              BOOL2BIT_MAC(nextPointerData.pointerData.nextHopOrEcmpPointer.srcAddrCheckMismatchEnable));
            U32_SET_FIELD_MAC(data,
                              PRV_CPSS_DXCH_LPM_RAM_ACTIVITY_STATE_OFFSET_CNS,
                              PRV_CPSS_DXCH_LPM_RAM_ACTIVITY_STATE_LENGTH_CNS,
                              BOOL2BIT_MAC(nextPointerData.pointerData.nextHopOrEcmpPointer.activityState));
            switch (nextPointerData.pointerType)
            {
                case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
                    U32_SET_FIELD_MAC(data,
                                      PRV_CPSS_DXCH_LPM_RAM_POINTER_TYPE_OFFSET_CNS,
                                      PRV_CPSS_DXCH_LPM_RAM_POINTER_TYPE_LENGTH_CNS,
                                      1);
                    break;
                case CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E:
                    U32_SET_FIELD_MAC(data,
                                      PRV_CPSS_DXCH_LPM_RAM_POINTER_TYPE_OFFSET_CNS,
                                      PRV_CPSS_DXCH_LPM_RAM_POINTER_TYPE_LENGTH_CNS,
                                      2);
                    break;
                case CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E:
                    U32_SET_FIELD_MAC(data,
                                      PRV_CPSS_DXCH_LPM_RAM_POINTER_TYPE_OFFSET_CNS,
                                      PRV_CPSS_DXCH_LPM_RAM_POINTER_TYPE_LENGTH_CNS,
                                      3);
                    break;
                default:
                    break;
            }
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    *hwDataPtr = data;

    return GT_OK;
}

/**
* @internal prvCpssDxChLpmNextPointerDataDecode function
* @endinternal
*
* @brief   Local function for LPM bucket next pointer HW format build
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; Lion2; Falcon.
*
* @param[in] hwDataPtr                - node data in HW format
*
* @param[out] nextPointerDataPtr       - (pointer to) next pointer data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on any parameter is wrong
*/
GT_STATUS prvCpssDxChLpmNextPointerDataDecode
(
    IN  GT_U32                                      *hwDataPtr,
    OUT CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC         *nextPointerDataPtr
)
{
    GT_U32  nextBucketTypeField;
    GT_U32  pointerTypeField;

    pointerTypeField = U32_GET_FIELD_MAC(*hwDataPtr,
                                         PRV_CPSS_DXCH_LPM_RAM_POINTER_TYPE_OFFSET_CNS,
                                         PRV_CPSS_DXCH_LPM_RAM_POINTER_TYPE_LENGTH_CNS);
    switch (pointerTypeField)
    {
        case 0:
            nextBucketTypeField = U32_GET_FIELD_MAC(*hwDataPtr,
                                                    PRV_CPSS_DXCH_LPM_RAM_NEXT_BUCKET_TYPE_OFFSET_CNS,
                                                    PRV_CPSS_DXCH_LPM_RAM_NEXT_BUCKET_TYPE_LENGTH_CNS);
            switch (nextBucketTypeField)
            {
                case 0:
                    nextPointerDataPtr->pointerType = CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E;
                    break;
                case 1:
                    nextPointerDataPtr->pointerType = CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E;
                    break;
                case 2:
                    nextPointerDataPtr->pointerType = CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case 1:
            nextPointerDataPtr->pointerType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
            break;
        case 2:
            nextPointerDataPtr->pointerType = CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E;
            break;
        case 3:
            nextPointerDataPtr->pointerType = CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (nextPointerDataPtr->pointerType)
    {
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E:
            if (nextPointerDataPtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E)
            {
                nextPointerDataPtr->pointerData.nextNodePointer.range5Index =
                            U32_GET_FIELD_MAC(*hwDataPtr,
                                              PRV_CPSS_DXCH_LPM_RAM_FIFTH_RANGE_OFFSET_CNS,
                                              PRV_CPSS_DXCH_LPM_RAM_FIFTH_RANGE_LENGTH_CNS);
                nextPointerDataPtr->pointerData.nextNodePointer.nextPointer =
                            U32_GET_FIELD_MAC(*hwDataPtr,
                                              PRV_CPSS_DXCH_LPM_RAM_COMPRESSED_2_NEXT_BUCKET_POINTER_OFFSET_CNS,
                                              PRV_CPSS_DXCH_LPM_RAM_COMPRESSED_2_NEXT_BUCKET_POINTER_LENGTH_CNS);
            }
            else
            {
                nextPointerDataPtr->pointerData.nextNodePointer.nextPointer =
                            U32_GET_FIELD_MAC(*hwDataPtr,
                                              PRV_CPSS_DXCH_LPM_RAM_NEXT_BUCKET_POINTER_OFFSET_CNS,
                                              PRV_CPSS_DXCH_LPM_RAM_NEXT_BUCKET_POINTER_LENGTH_CNS);
            }
            nextPointerDataPtr->pointerData.nextNodePointer.pointToSipTree =
                        U32_GET_FIELD_MAC(*hwDataPtr,
                                          PRV_CPSS_DXCH_LPM_RAM_POINT_TO_LOOKUP_1_OFFSET_CNS,
                                          PRV_CPSS_DXCH_LPM_RAM_POINT_TO_LOOKUP_1_LENGTH_CNS);
            break;

        case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
        case CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E:
        case CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E:
            nextPointerDataPtr->pointerData.nextHopOrEcmpPointer.entryIndex =
                        U32_GET_FIELD_MAC(*hwDataPtr,
                                          PRV_CPSS_DXCH_LPM_RAM_NEXTHOP_OR_ECMP_POINTER_OFFSET_CNS,
                                          PRV_CPSS_DXCH_LPM_RAM_NEXTHOP_OR_ECMP_POINTER_LENGTH_CNS);
            nextPointerDataPtr->pointerData.nextHopOrEcmpPointer.ipv6McGroupScopeLevel =
                        U32_GET_FIELD_MAC(*hwDataPtr,
                                          PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_GROUP_SCOPE_LEVEL_OFFSET_CNS,
                                          PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_GROUP_SCOPE_LEVEL_LENGTH_CNS);
            nextPointerDataPtr->pointerData.nextHopOrEcmpPointer.ucRpfCheckEnable =
                        BIT2BOOL_MAC(U32_GET_FIELD_MAC(*hwDataPtr,
                                          PRV_CPSS_DXCH_LPM_RAM_UC_RPF_CHECK_OFFSET_CNS,
                                          PRV_CPSS_DXCH_LPM_RAM_UC_RPF_CHECK_LENGTH_CNS));
            nextPointerDataPtr->pointerData.nextHopOrEcmpPointer.srcAddrCheckMismatchEnable =
                        BIT2BOOL_MAC(U32_GET_FIELD_MAC(*hwDataPtr,
                                          PRV_CPSS_DXCH_LPM_RAM_UC_SIP_SA_CHECK_MISMATCH_OFFSET_CNS,
                                          PRV_CPSS_DXCH_LPM_RAM_UC_SIP_SA_CHECK_MISMATCH_LENGTH_CNS));
            nextPointerDataPtr->pointerData.nextHopOrEcmpPointer.activityState =
                        BIT2BOOL_MAC(U32_GET_FIELD_MAC(*hwDataPtr,
                                          PRV_CPSS_DXCH_LPM_RAM_ACTIVITY_STATE_OFFSET_CNS,
                                          PRV_CPSS_DXCH_LPM_RAM_ACTIVITY_STATE_LENGTH_CNS));
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


/**
* @internal prvCpssLpmFalconFillPrvLeafValue function
* @endinternal
*
* @brief   translate CPSS_DXCH_LPM_LEAF_ENTRY_STC leaf into PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC leaf entry
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] leafNodePtr            - leaf data
*
* @param[out] valuePtr              - pointer to leaf data in hw format.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on any parameter is wrong
*/
GT_VOID prvCpssLpmFalconFillPrvLeafValue
(
    IN  CPSS_DXCH_LPM_LEAF_ENTRY_STC      *leafEntryPtr,
    OUT PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC  *prvLeafEntryPtr
)
{
    cpssOsMemSet(prvLeafEntryPtr,0,sizeof(PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC));
    prvLeafEntryPtr->entryType = leafEntryPtr->entryType;
    prvLeafEntryPtr->index = leafEntryPtr->index;
    prvLeafEntryPtr->ipv6MCGroupScopeLevel = leafEntryPtr->ipv6MCGroupScopeLevel;
    prvLeafEntryPtr->sipSaCheckMismatchEnable = leafEntryPtr->sipSaCheckMismatchEnable;
    prvLeafEntryPtr->ucRPFCheckEnable = leafEntryPtr->ucRPFCheckEnable;
    prvLeafEntryPtr->priority = leafEntryPtr->priority;
}


/**
* @internal prvCpssLpmFalconBuildLeafValue function
* @endinternal
*
* @brief   Build leaf data for single leaf
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] leafNodePtr            - leaf data
*
* @param[out] valuePtr              - pointer to leaf data in hw format.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on any parameter is wrong
*/
GT_STATUS prvCpssLpmFalconBuildLeafValue
(
    IN  PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC  *leafNodePtr,
    OUT GT_U32                            *valuePtr
)
{
    GT_U32  value1 = 0;

    *valuePtr = 0;
    /* Set Leaf priority */
    switch (leafNodePtr->priority)
    {
        case CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_FDB_E:
            value1 = 0;
            break;
        case CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_LPM_E:
            value1 = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "bad priority value");
    }

    U32_SET_FIELD_MASKED_MAC(*valuePtr, 0, 1, value1);
    if (leafNodePtr->pointToSip == GT_TRUE) 
    {
        /* fill fields that relevant for pointing to src tree*/
        U32_SET_FIELD_MASKED_MAC(*valuePtr, 1, 1, 1); /* entry type: 1:1 */
        /* pointer to next bucket : 2...21*/
        U32_SET_FIELD_MASKED_MAC(*valuePtr, 2, 20, leafNodePtr->pointerToSip);
        switch (leafNodePtr->nextNodeType) 
        {
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
            value1 = 0;
            break;
        case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
            value1 = 1;
            break;
        default: 
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "BAD next node type"); 
        }
        U32_SET_FIELD_MASKED_MAC(*valuePtr, 22, 1, value1);

        return GT_OK;
    }
    switch (leafNodePtr->entryType) 
    {
    case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
        value1 = 0;
        break;
    case CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E:
        value1 = 1;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
   
    U32_SET_FIELD_MASKED_MAC(*valuePtr, 1, 1, 0); /* entry type */

    U32_SET_FIELD_MASKED_MAC(*valuePtr, 2, 1, value1); /* regular leaf */

    U32_SET_FIELD_MASKED_MAC(*valuePtr, 3, 1,BOOL2BIT_MAC(leafNodePtr->ucRPFCheckEnable));
    U32_SET_FIELD_MASKED_MAC(*valuePtr, 4, 1,BOOL2BIT_MAC(leafNodePtr->sipSaCheckMismatchEnable));
    U32_SET_FIELD_MASKED_MAC(*valuePtr, 5, 2, leafNodePtr->ipv6MCGroupScopeLevel);


    U32_SET_FIELD_MASKED_MAC(*valuePtr, 7, 15, leafNodePtr->index);


    return GT_OK;
}

/**
* @internal prvCpssDxChLpmFalconLeafNodeDataBuild function
* @endinternal
*
* @brief   Build hw format for LPM leaves nodes
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] compressedNodePtr          - node data
* @param[in] leavesCounter              - number of leaves so far
* @param[in] startNewGonLeaves          - leaves from the new GON started to process
* @param[in] groupOfNodesArrayPtrPtr    - pointer to adress of current data in hw format
*
* @param[out] groupOfNodesArrayPtrPtr   - (pointer to pointer) to data in hw format.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on any parameter is wrong
*/
GT_STATUS prvCpssDxChLpmFalconLeafNodeDataBuild
(
    IN  PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC            *leafNodePtr,
    IN  GT_U32                                      leavesCounter,
    IN  GT_BOOL                                     startNewGonLeaves,
    INOUT GT_U32                                    **groupOfNodesArrayPtrPtr
)
{
    static int k = 0;/* offset inside word */
    GT_U32      value = 0;
    GT_U32      bitMask = 0;
    GT_U32      firstPartFieldBitsNum = 0;
    GT_U32      restFieldBitsNum = 0;
    GT_STATUS   rc = GT_OK;
    static GT_U32 *tempGonArrayPtr = NULL;


    rc = prvCpssLpmFalconBuildLeafValue(leafNodePtr,&value);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* one lpm line contain 5 leaves: 23*5 = 115 */
    /* for each group of nodes leavesCounter is started from 0*/
    if ((leavesCounter % MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS) == 0)
    {
        k = 19;/* first leaf in entry started from bit 114 -> bit 18 in word */
        if (startNewGonLeaves == GT_TRUE)
        {
            tempGonArrayPtr = *groupOfNodesArrayPtrPtr + 4; /* points to next LPM line */
            *groupOfNodesArrayPtrPtr = *groupOfNodesArrayPtrPtr+3; /* start from the last word */
        }
        else
        {
            /* another LPM entry */
            *groupOfNodesArrayPtrPtr = tempGonArrayPtr+3;
            tempGonArrayPtr = tempGonArrayPtr + 4; /* points to next LPM line */
        }
    }

    if ((k - 23) >= 0)
    {
        /* there is enough place to set full leaf in word */
        k = k-23;
        U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, 23, value & 0x7fffff);

    }
    else
    {
        firstPartFieldBitsNum = k; /* how much bits free till word bound */
        restFieldBitsNum = 23 - firstPartFieldBitsNum; /* number of bits for next word */
        if (firstPartFieldBitsNum != 0)
        {
            k = 0;
            bitMask = BIT_MASK_MAC(firstPartFieldBitsNum);
            U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, firstPartFieldBitsNum,
                                      (value>>restFieldBitsNum) & bitMask);
        }
        (*groupOfNodesArrayPtrPtr)--;
        /* new word in use */
        k = 32 - restFieldBitsNum;
        if (restFieldBitsNum !=0)
        {
            bitMask = BIT_MASK_MAC(restFieldBitsNum);
            U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, restFieldBitsNum, value & bitMask);
        }

    }

    return GT_OK;
}

/**
* @internal prvCpssDxChLpmFalconCompressedNodeDataBuild function
* @endinternal
*
* @brief   Build hw format for LPM compressed nodes
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] devNum                     - the device number
* @param[in] compressedNodePtr          - node data
* @param[in] groupOfNodesArrayPtrPtr    - pointer to adress of current data in hw format
*
* @param[out] groupOfNodesArrayPtrPtr   - (pointer to pointer) to data in hw format.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on any parameter is wrong
*/
GT_STATUS prvCpssDxChLpmFalconCompressedNodeDataBuild
(
    IN    PRV_CPSS_DXCH_LPM_COMPRESSED_STC          *compressedNodePtr,
    INOUT GT_U32                                    **groupOfNodesArrayPtrPtr
)
{
    int k;
    GT_U32 i;
    GT_U32      childType = 0;
    GT_U32      offset = 0;
    GT_U32      value = 0;
    GT_U32      bitMask = 0;
    GT_U32      firstPartFieldBitsNum = 0;
    GT_U32      restFieldBitsNum = 0;
    GT_U32      *tempGonArrayPtr = NULL;
    GT_U32      bankNumber = 0;
    GT_U32      offsetInBank = 0;


    tempGonArrayPtr = *groupOfNodesArrayPtrPtr;
    *groupOfNodesArrayPtrPtr = *groupOfNodesArrayPtrPtr+3;
    if (compressedNodePtr->compressedType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)
    {
        /* the number of leaves for standart compress is 0 */
        U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 16, 2, 0);
        k = 14;
        for (i = 0; i < 10; i++)
        {
            childType = compressedNodePtr->childNodeTypes0_9[i];
            offset = compressedNodePtr->ranges1_9[i];
            if (i == 0)
            {
                U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, 2,childType);
                continue;
            }
            value = ((offset << 2) | childType)& 0x3ff;
            if ((k - 10) > 0)
            {
                k = k -10;
                U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, 10, value);
            }
            else
            {
                firstPartFieldBitsNum = k; /* how much bits free till word bound */
                restFieldBitsNum = 10 - firstPartFieldBitsNum; /* number of bits in next word */
                if (firstPartFieldBitsNum != 0)
                {
                    k = 0;
                    bitMask = BIT_MASK_MAC(firstPartFieldBitsNum);
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, firstPartFieldBitsNum,
                                             (value>>restFieldBitsNum) & bitMask);
                }
                (*groupOfNodesArrayPtrPtr)--;
                k = 32-restFieldBitsNum;
                if (restFieldBitsNum != 0)
                {
                    bitMask = BIT_MASK_MAC(restFieldBitsNum);
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, restFieldBitsNum, value & bitMask);
                }
            }
        }
    }
    else
        if (compressedNodePtr->compressedType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)
        {
            /* the number of leaves for this type of compress is 1 */
            U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 16, 2, 1);
            prvCpssLpmFalconBuildLeafValue(&compressedNodePtr->embLeavesArray[0], &value);
            /* leaf occupies bits 89-111*/
            /* In current word there is place for 16 bits: from 96 - 111*/
            U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 0, 16, value>>7);
            (*groupOfNodesArrayPtrPtr)--;
            /* The last 7 bits from 89 - 95*/
            U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 25, 7, value);
            /* 82 -88 are reserved */
            k = 16;
            value = 0;
            for (i = 0; i < 7; i++)
            {
                childType = compressedNodePtr->childNodeTypes0_9[i];
                offset = compressedNodePtr->ranges1_9[i];
                if (i == 0)
                {
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, 2,childType);
                    continue;
                }
                value = ((offset << 2) | childType)& 0x3ff;
                if ((k - 10) > 0)
                {
                    k = k -10;
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, 10, value);
                }
                else
                {
                    firstPartFieldBitsNum = k; /* how much bits free till word bound */
                    restFieldBitsNum = 10 - firstPartFieldBitsNum; /* number of bits in next word */
                    if (firstPartFieldBitsNum != 0)
                    {
                        k = 0;
                        bitMask = BIT_MASK_MAC(firstPartFieldBitsNum);
                        U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, firstPartFieldBitsNum,
                                                 (value>>restFieldBitsNum) & bitMask);
                    }
                    (*groupOfNodesArrayPtrPtr)--;
                    k = 32-restFieldBitsNum;
                    bitMask = BIT_MASK_MAC(restFieldBitsNum);
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, restFieldBitsNum, value & bitMask);
                }
            }
        }
        else
            if (compressedNodePtr->compressedType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E)
            {
                /* the number of leaves for this type of compress is 2 */
                U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 16, 2, 2);
                prvCpssLpmFalconBuildLeafValue(&compressedNodePtr->embLeavesArray[0], &value);
                /* leaf occupies bits 89-111*/
                /* In current word there is place for 16 bits: from 96 - 111*/
                U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 0, 16, value>>7);
                (*groupOfNodesArrayPtrPtr)--;
                /* The last 7 bits from 89 - 95*/
                U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 25, 7, value);
                /* The second leaf is 66-88 */
                prvCpssLpmFalconBuildLeafValue( &compressedNodePtr->embLeavesArray[1], &value);
                U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 2, 23, value & 0x7fffff);
                /* the next 4 bits are reserved : bits 65,64 in this word and bits 63,62 in next word */
                (*groupOfNodesArrayPtrPtr)--;
                /* starting from bit61*/

                k = 28;
                value = 0;
                for (i = 0; i < 5; i++)
                {
                    childType = compressedNodePtr->childNodeTypes0_9[i];
                    offset = compressedNodePtr->ranges1_9[i];
                    if (i == 0)
                    {
                        U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, 2,childType);
                        continue;
                    }
                    value = ((offset << 2) | childType)& 0x3ff;
                    if ((k - 10) > 0)
                    {
                        k = k -10;
                        U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, 10, value);
                    }
                    else
                    {
                        firstPartFieldBitsNum = k; /* how much bits free till word bound */
                        restFieldBitsNum = 10 - firstPartFieldBitsNum; /* number of bits in next word */
                        if (firstPartFieldBitsNum != 0)
                        {
                            k = 0;
                            bitMask = BIT_MASK_MAC(firstPartFieldBitsNum);
                            U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, firstPartFieldBitsNum,
                                                     (value>>restFieldBitsNum) & bitMask);
                        }
                        (*groupOfNodesArrayPtrPtr)--;
                        k = 32-restFieldBitsNum;
                        bitMask = BIT_MASK_MAC(restFieldBitsNum);
                        U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, restFieldBitsNum, value & bitMask);
                    }
                }

            }
            else
                if (compressedNodePtr->compressedType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E)
                {
                    /* the number of leaves for this type of compress is 3 */
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 16, 2, 3);
                    prvCpssLpmFalconBuildLeafValue(&compressedNodePtr->embLeavesArray[0], &value);
                    /* leaf0 occupies bits 89-111*/
                    /* In current word there is place for 16 bits: from 96 - 111*/
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 0, 16, value>>7);
                    (*groupOfNodesArrayPtrPtr)--;
                    /* The last 7 bits from 89 - 95*/
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 25, 7, value);
                    /* The second leaf1 is 66-88 */
                    prvCpssLpmFalconBuildLeafValue(&compressedNodePtr->embLeavesArray[1], &value);
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 2, 23, value & 0x7fffff);
                    /* The 3 leaf is 43-65*/
                    prvCpssLpmFalconBuildLeafValue(&compressedNodePtr->embLeavesArray[2], &value);
                    /* In current word there is place for 2 bits: from 65,64*/
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 0, 2, (value>>21)& 3);
                    (*groupOfNodesArrayPtrPtr)--;
                    /* In this word 21 bits 43-63 */
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 11, 21, value & 0x1fffff);
                    /* bit 42 is reserved */
                    /* so fill ranges starting from bit 41*/
                    k = 8;
                    value = 0;
                    for (i = 0; i < 3; i++)
                    {
                        childType = compressedNodePtr->childNodeTypes0_9[i];
                        offset = compressedNodePtr->ranges1_9[i];
                        if (i == 0)
                        {
                            U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, 2,childType);
                            continue;
                        }
                        value = ((offset << 2) | childType)& 0x3ff;
                        if ((k - 10) > 0)
                        {
                            k = k -10;
                            U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, 10, value);
                        }
                        else
                        {
                            firstPartFieldBitsNum = k; /* how much bits free till word bound */
                            restFieldBitsNum = 10 - firstPartFieldBitsNum; /* number of bits in next word */
                            if (firstPartFieldBitsNum != 0)
                            {
                                k = 0;
                                bitMask = BIT_MASK_MAC(firstPartFieldBitsNum);
                                U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, firstPartFieldBitsNum,
                                                         (value>>restFieldBitsNum) & bitMask);
                            }
                            (*groupOfNodesArrayPtrPtr)--;
                            k = 32-restFieldBitsNum;
                            bitMask = BIT_MASK_MAC(restFieldBitsNum);
                            U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, restFieldBitsNum, value & bitMask);
                        }
                    }
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                if (*groupOfNodesArrayPtrPtr != tempGonArrayPtr)
                {
                    cpssOsPrintf("prvCpssDxChLpmGroupOfNodesWrite: wrong compr GON ptr\n ");
                }
                offsetInBank = compressedNodePtr->lpmOffset & 0x7fff;
                bankNumber = (compressedNodePtr->lpmOffset >> 15) & 0x1f;
                U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr,15,5,bankNumber & 0x1f);
                U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr,0,15,offsetInBank & 0x7fff);
                *groupOfNodesArrayPtrPtr = *groupOfNodesArrayPtrPtr+4;


   return GT_OK;

}

/**
* @internal prvCpssLpmFalconDecodeLeafValue function
* @endinternal
*
* @brief   Decode leaf entry from HW value
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] value                  - leaf data in HW format
*
* @param[out] leafNodePtr           - pointer to leaf entry.
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - on any parameter is wrong
*/
GT_STATUS prvCpssLpmFalconDecodeLeafValue
(
    IN  GT_U32                          value,
    OUT CPSS_DXCH_LPM_LEAF_ENTRY_STC    *leafNodePtr
)
{
    GT_U32 value1 = 0;
    GT_BOOL mcLeafTrigerMcSgLookup = GT_FALSE;

    leafNodePtr->priority = U32_GET_FIELD_MAC(value, 0, 1);
    mcLeafTrigerMcSgLookup = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value , 1, 1));
    value1 = U32_GET_FIELD_MAC(value, 2, 1);
    switch(value1)
    {
        case 0:
            leafNodePtr->entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
            break;
        case 1:
            leafNodePtr->entryType = CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (mcLeafTrigerMcSgLookup == GT_TRUE)
    {
        /* not implemented yet */
        /* in bits 2...21 is intended for pointer to the next bucket on hte S,G lookup */
        /* it will be source tree root bucket */
    }

    leafNodePtr->ucRPFCheckEnable = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value , 3, 1));
    leafNodePtr->sipSaCheckMismatchEnable = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value, 4, 1));
    leafNodePtr->ipv6MCGroupScopeLevel = U32_GET_FIELD_MAC(value, 5, 2);
    leafNodePtr->index = U32_GET_FIELD_MAC(value, 7, 15);

    if (mcLeafTrigerMcSgLookup == GT_TRUE)
    {
        /* next node type : regular or compressed : not implemented yet */
    }
    return GT_OK;
}
