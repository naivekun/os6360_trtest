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
* @file cpssDxChLpm.c
*
* @brief LPM APIs
*
* @version   12
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpm.h>
#include <cpss/dxCh/dxChxGen/lpm/private/prvCpssDxChLpmLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/hw/prvCpssDxChLpmHw.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRam.h>


#define LPM_DEBUG_PRINT
#undef LPM_DEBUG_PRINT
#ifdef LPM_DEBUG_PRINT
extern int cpssOsPrintf(char*, ...);
#define PRV_CPSS_DXCH_LPM_DBG_INFO_MAC(x) cpssOsPrintf x
#else
#define PRV_CPSS_DXCH_LPM_DBG_INFO_MAC(x)
#endif

/* 2 LPM per pipe (4 pre device) */
#define PRV_CPSS_DXCH_LPM_BC3_ENGINE_NUM_CNS    4
/* 2 LPMs per device */
#define PRV_CPSS_DXCH_LPM_ALDRIN2_ENGINE_NUM_CNS   2


/**
* @internal internal_cpssDxChLpmNodeWrite function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Write an LPM Node (Bucket) to the HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] lpmLineOffset            - offset from the LPM base in LPM lines
* @param[in] nodeType                 - type of node (bucket)
* @param[in] rangeSelectSecPtr        - the range selection (bitmap) section
* @param[in] numOfRanges              - number of next pointers in nextPointerArrayPtr
* @param[in] nextPointerArrayPtr      - next pointers array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
static GT_STATUS internal_cpssDxChLpmNodeWrite
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   lpmLineOffset,
    IN CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT      nodeType,
    IN CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT   *rangeSelectSecPtr,
    IN GT_U32                                   numOfRanges,
    IN CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC      *nextPointerArrayPtr
)
{
    GT_STATUS   rc;
    GT_U32      i;
    GT_U32      bucket[PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS];
    GT_U32      *tmpBucketPtr;
    GT_U32      bucketSize;         /* in LPM lines */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(rangeSelectSecPtr);
    CPSS_NULL_PTR_CHECK_MAC(nextPointerArrayPtr);

    tmpBucketPtr = bucket;

    switch (nodeType)
    {
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
            for (i = 0; i < 11; i++, tmpBucketPtr++)
            {
                *tmpBucketPtr = 0;
                U32_SET_FIELD_MASKED_MAC(*tmpBucketPtr, 0, 24,
                    rangeSelectSecPtr->regularBitVector.bitVectorEntry[i].bitMap);
                U32_SET_FIELD_MASKED_MAC(*tmpBucketPtr, 24, 8,
                    rangeSelectSecPtr->regularBitVector.bitVectorEntry[i].rangeCounter);
            }

            for(i = 0; i < numOfRanges; i++, tmpBucketPtr++)
            {
                if ((rc = prvCpssDxChLpmNextPointerDataBuild(devNum,
                                                             nextPointerArrayPtr[i],
                                                             tmpBucketPtr)) != GT_OK)
                {
                    return rc;
                }
            }

            bucketSize = 11 + numOfRanges;
            break;

        case CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E:
            *tmpBucketPtr = 0;
            for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_NUM_OF_RANGES_IN_LPM_LINE_CNS; i++)
            {
                U32_SET_FIELD_MAC(*tmpBucketPtr, (i << 3), 8,
                          rangeSelectSecPtr->compressed1BitVector.ranges1_4[i]);
            }

            tmpBucketPtr++;
            *tmpBucketPtr = 0;

            for (i = 0; i < numOfRanges; i++, tmpBucketPtr++)
            {
                if((rc = prvCpssDxChLpmNextPointerDataBuild(devNum,
                                                            nextPointerArrayPtr[i],
                                                            tmpBucketPtr)) != GT_OK)
                {
                    return rc;
                }
            }

            bucketSize = 1 + numOfRanges;
            break;

        case CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E:
            *tmpBucketPtr = 0;
            for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_NUM_OF_RANGES_IN_LPM_LINE_CNS; i++)
            {
                U32_SET_FIELD_MAC(*tmpBucketPtr, (i << 3), 8,
                          rangeSelectSecPtr->compressed2BitVector.ranges1_4[i]);
            }

            tmpBucketPtr++;
            *tmpBucketPtr = 0;

            for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_NUM_OF_RANGES_IN_LPM_LINE_CNS; i++)
            {
                U32_SET_FIELD_MAC(*tmpBucketPtr, (i << 3), 8,
                          rangeSelectSecPtr->compressed2BitVector.ranges6_9[i]);
            }

            tmpBucketPtr++;

            for (i = 0; i < numOfRanges; i++, tmpBucketPtr++)
            {
                if ((rc = prvCpssDxChLpmNextPointerDataBuild(devNum,
                                                             nextPointerArrayPtr[i],
                                                             tmpBucketPtr)) != GT_OK)
                {
                    return rc;
                }
            }
            bucketSize = 2 + numOfRanges;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Write the node */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
    {
        /* check that the lpmLineOffset is in range of half mode
           meanning half of the memory lines */
        if ((lpmLineOffset%PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap)>=
            (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock/2))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        /* check that the lpmLineOffset is in range of full mode
           meanning can not be more than numOfLinesInBlock */
        if ((lpmLineOffset%PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap)>=
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }


    rc = prvCpssDxChWriteTableMultiEntry(devNum,
                                         CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                         lpmLineOffset,
                                         bucketSize,
                                         &bucket[0]);

    if(rc == GT_OK)
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
        {
            /* Write the node  to the second half*/
            rc = prvCpssDxChWriteTableMultiEntry(devNum,
                                         CPSS_DXCH_SIP5_20_TABLE_LPM_MEM_PIPE_1_E,
                                         lpmLineOffset,
                                         bucketSize,
                                         &bucket[0]);
        }
    }
    return rc;
}

/**
* @internal cpssDxChLpmNodeWrite function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Write an LPM Node (Bucket) to the HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] lpmLineOffset            - offset from the LPM base in LPM lines
* @param[in] nodeType                 - type of node (bucket)
* @param[in] rangeSelectSecPtr        - the range selection (bitmap) section
* @param[in] numOfRanges              - number of next pointers in nextPointerArrayPtr
* @param[in] nextPointerArrayPtr      - next pointers array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
GT_STATUS cpssDxChLpmNodeWrite
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   lpmLineOffset,
    IN CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT      nodeType,
    IN CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT   *rangeSelectSecPtr,
    IN GT_U32                                   numOfRanges,
    IN CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC      *nextPointerArrayPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLpmNodeWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, lpmLineOffset, nodeType, rangeSelectSecPtr, numOfRanges, nextPointerArrayPtr));

    rc = internal_cpssDxChLpmNodeWrite(devNum, lpmLineOffset, nodeType, rangeSelectSecPtr, numOfRanges, nextPointerArrayPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, lpmLineOffset, nodeType, rangeSelectSecPtr, numOfRanges, nextPointerArrayPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/*******************************************************************************
* internal_cpssDxChLpmNodeRead
*
* DESCRIPTION:
*       Function Relevant mode : Low Level API modes
*       Read an LPM Node (Bucket) from the HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] lpmLineOffset            - offset from the LPM base in LPM lines
* @param[in] nodeType                 - type of node (bucket)
*
* @param[out] rangeSelectSecPtr        - the range selection (bitmap) section
* @param[out] numOfRangesPtr           - number of next pointers in nextPointerArrayPtr
* @param[out] nextPointerArrayPtr      - next pointers array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
static GT_STATUS internal_cpssDxChLpmNodeRead
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  lpmLineOffset,
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT     nodeType,
    OUT CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT  *rangeSelectSecPtr,
    OUT GT_U32                                  *numOfRangesPtr,
    OUT CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC     *nextPointerArrayPtr
)
{
    GT_U32      baseAddr, lastAddr;
    GT_STATUS   rc;
    GT_U32      bucket[PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS];
    GT_U32      i, counter, prevCount, tmp, fromWhere;
    GT_U32      *tmpBucketPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(rangeSelectSecPtr);
    CPSS_NULL_PTR_CHECK_MAC(numOfRangesPtr);
    CPSS_NULL_PTR_CHECK_MAC(nextPointerArrayPtr);

    tmpBucketPtr = bucket;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
    {
        /* check that the lpmLineOffset is in range of half mode
           meanning half of the memory lines */
        if ((lpmLineOffset%PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap)>=
            (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock/2))

        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        /* check that the lpmLineOffset is in range of full mode
           meanning can not be more than numOfLinesInBlock */
        if ((lpmLineOffset%PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap)>=
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    switch (nodeType)
    {
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
            rc = prvCpssDxChReadTableMultiEntry(devNum,
                                                CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                lpmLineOffset,
                                                11,
                                                tmpBucketPtr);
            if (rc != GT_OK)
                return rc;

            counter = 0;
            prevCount = bucket[(255 / 24)] >> 24;
            tmp = bucket[(255 / 24)];
            fromWhere = 255%24;
            for (i = 0; i <= fromWhere; i++)
            {
                counter += (tmp & 0x01);
                tmp >>= 1;
            }
            *numOfRangesPtr = prevCount + counter;

            for(i = 0; i < 11; i++, tmpBucketPtr++)
            {
                rangeSelectSecPtr->regularBitVector.bitVectorEntry[i].bitMap =
                                            U32_GET_FIELD_MAC(*tmpBucketPtr, 0, 24);
                rangeSelectSecPtr->regularBitVector.bitVectorEntry[i].rangeCounter =
                                            (GT_U8)U32_GET_FIELD_MAC(*tmpBucketPtr, 24, 8);
            }

            /* address after bitmap */
            rc = prvCpssDxChReadTableMultiEntry(devNum,
                                                CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                lpmLineOffset + 11,
                                                *numOfRangesPtr,
                                                tmpBucketPtr);
            if (rc != GT_OK)
                return rc;

            for(i = 0; i < *numOfRangesPtr; i++, tmpBucketPtr++, nextPointerArrayPtr++)
            {
                if((rc = prvCpssDxChLpmNextPointerDataDecode(tmpBucketPtr,
                                                             nextPointerArrayPtr)) != GT_OK)
                {
                    return rc;
                }
            }

        break;

        case CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E:
            /* read ranges description bitmap and calculate number of ranges in bucket */
            rc = prvCpssDxChReadTableMultiEntry(devNum,
                                                CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                lpmLineOffset,
                                                1,
                                                tmpBucketPtr);
            if (rc != GT_OK)
                return rc;

            lastAddr = baseAddr = 0;
            for(i = 0; i < PRV_CPSS_DXCH_LPM_RAM_NUM_OF_RANGES_IN_LPM_LINE_CNS; i++)
            {
                baseAddr = (bucket[0] >> (8 * i)) & 0xFF;
                if ((baseAddr > 0xFF) || (lastAddr >= baseAddr))
                {
                    break;
                }
                else
                {
                    lastAddr = baseAddr;
                }
            }
            *numOfRangesPtr = i + 1;

            for(i = 0; i < PRV_CPSS_DXCH_LPM_RAM_NUM_OF_RANGES_IN_LPM_LINE_CNS; i++)
            {
                rangeSelectSecPtr->compressed1BitVector.ranges1_4[i] =
                                        (GT_U8)U32_GET_FIELD_MAC(*tmpBucketPtr, (i<<3), 8);
            }

            tmpBucketPtr++;

            /* address after bitmap */
            rc = prvCpssDxChReadTableMultiEntry(devNum,
                                                CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                lpmLineOffset + 1,
                                                *numOfRangesPtr,
                                                tmpBucketPtr);
            if (rc != GT_OK)
                return rc;


            for(i = 0; i < *numOfRangesPtr; i++, tmpBucketPtr++, nextPointerArrayPtr++)
            {
                if((rc = prvCpssDxChLpmNextPointerDataDecode(tmpBucketPtr,
                                                             nextPointerArrayPtr)) != GT_OK)
                {
                    return rc;
                }
            }

        break;

        case CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E:
            /* read ranges description bitmap and calculate number of ranges in bucket */
            rc = prvCpssDxChReadTableMultiEntry(devNum,
                                                CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                lpmLineOffset,
                                                2,
                                                tmpBucketPtr);
            if (rc != GT_OK)
            {
               return rc;
            }
            tmpBucketPtr += 2;

            /* read bit index 5 - 24:31 */
            baseAddr = ((bucket[0] >> 24) & 0xFF);
            lastAddr = baseAddr;
            if (baseAddr > 0xFF)
            {
                prevCount = 1;
                lastAddr = 0;
                tmp = bucket[0];
            }
            else
            {
                prevCount = 6;
                /* switch to the next bucket */
                tmp = bucket[1];
            }

            for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_NUM_OF_RANGES_IN_LPM_LINE_CNS; i++)
            {
                baseAddr = (tmp >> (8 * i)) & 0xFF;

                if ((baseAddr > 0xFF) || (lastAddr >= baseAddr))
                {
                    break;
                }
                else
                {
                    lastAddr = baseAddr;
                }
            }

            *numOfRangesPtr = i + prevCount;

            for(i = 0; i < PRV_CPSS_DXCH_LPM_RAM_NUM_OF_RANGES_IN_LPM_LINE_CNS; i++)
            {
                rangeSelectSecPtr->compressed2BitVector.ranges1_4[i] =
                                        (GT_U8)U32_GET_FIELD_MAC(bucket[0], (i<<3), 8);
            }

            for(i = 0; i < PRV_CPSS_DXCH_LPM_RAM_NUM_OF_RANGES_IN_LPM_LINE_CNS; i++)
            {
                rangeSelectSecPtr->compressed2BitVector.ranges6_9[i] =
                                        (GT_U8)U32_GET_FIELD_MAC(bucket[1], (i<<3), 8);
            }

            /* address after bitmap */

            rc = prvCpssDxChReadTableMultiEntry(devNum,
                                                CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                lpmLineOffset+2,
                                                *numOfRangesPtr,
                                                tmpBucketPtr);
            if (rc != GT_OK)
            {
               return rc;
            }


            for(i = 0; i < *numOfRangesPtr; i++, tmpBucketPtr++, nextPointerArrayPtr++)
            {
                if((rc = prvCpssDxChLpmNextPointerDataDecode(tmpBucketPtr,
                                                             nextPointerArrayPtr)) != GT_OK)
                {
                    return rc;
                }
            }

        break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssDxChLpmNodeRead function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Read an LPM Node (Bucket) from the HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] lpmLineOffset            - offset from the LPM base in LPM lines
* @param[in] nodeType                 - type of node (bucket)
*
* @param[out] rangeSelectSecPtr        - the range selection (bitmap) section
* @param[out] numOfRangesPtr           - number of next pointers in nextPointerArrayPtr
* @param[out] nextPointerArrayPtr      - next pointers array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
GT_STATUS cpssDxChLpmNodeRead
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  lpmLineOffset,
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT     nodeType,
    OUT CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT  *rangeSelectSecPtr,
    OUT GT_U32                                  *numOfRangesPtr,
    OUT CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC     *nextPointerArrayPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLpmNodeRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, lpmLineOffset, nodeType, rangeSelectSecPtr, numOfRangesPtr, nextPointerArrayPtr));

    rc = internal_cpssDxChLpmNodeRead(devNum, lpmLineOffset, nodeType, rangeSelectSecPtr, numOfRangesPtr, nextPointerArrayPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, lpmLineOffset, nodeType, rangeSelectSecPtr, numOfRangesPtr, nextPointerArrayPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLpmNodeNextPointersWrite function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Write an LPM Node's (Bucket) Next Pointer entry (range) to the HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] lpmLineOffset            - offset from the LPM base in LPM lines
* @param[in] numOfNextPointers        - number of next pointers in nextPointerArrayPtr
* @param[in] nextPointerArrayPtr      - next pointers array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
static GT_STATUS internal_cpssDxChLpmNodeNextPointersWrite
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   lpmLineOffset,
    IN GT_U32                                   numOfNextPointers,
    IN CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC      *nextPointerArrayPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      data[16];
    GT_U32      i = 0, j;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(nextPointerArrayPtr);

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
    {
        /* check that the lpmLineOffset is in range of half mode
           meanning half of the memory lines */
        if ((lpmLineOffset%PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap)>=
            (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock/2))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        /* check that the lpmLineOffset is in range of full mode
           meanning can not be more than numOfLinesInBlock */
        if ((lpmLineOffset%PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap)>=
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    cpssOsMemSet(data, 0x00, sizeof(data));

    PRV_CPSS_DXCH_LPM_DBG_INFO_MAC(("[Vr]Writing to -> [0x%08X] : " , lpmLineOffset));

    while (i < numOfNextPointers)
    {
        /* lets write in bulks of 16 entries - I don't want to write
         * all at once because it requires allocate up to 4K to
         * create node SW copy
         */
        for (j = 0; (i < numOfNextPointers) && (j < 16); i++, j++)
        {
            if ((rc = prvCpssDxChLpmNextPointerDataBuild(devNum,
                                                         nextPointerArrayPtr[i],
                                                         &(data[j]))) != GT_OK)
            {
                return rc;
            }
            PRV_CPSS_DXCH_LPM_DBG_INFO_MAC(("[0x%08X] " , data[j]));
        }

        rc = prvCpssDxChWriteTableMultiEntry(devNum,
                                             CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                             lpmLineOffset,
                                             j,
                                             &data[0]);
        if (rc != GT_OK)
            return rc;


        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
        {
            /* Write the node  to the second half*/
            rc = prvCpssDxChWriteTableMultiEntry(devNum,
                                         CPSS_DXCH_SIP5_20_TABLE_LPM_MEM_PIPE_1_E,
                                         lpmLineOffset,
                                         j,
                                         &data[0]);
        }


        /* increase address by size of entries in bulk */
        lpmLineOffset += j;
    }
    PRV_CPSS_DXCH_LPM_DBG_INFO_MAC(("\n"));

    return rc;
}

/**
* @internal cpssDxChLpmNodeNextPointersWrite function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Write an LPM Node's (Bucket) Next Pointer entry (range) to the HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] lpmLineOffset            - offset from the LPM base in LPM lines
* @param[in] numOfNextPointers        - number of next pointers in nextPointerArrayPtr
* @param[in] nextPointerArrayPtr      - next pointers array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
GT_STATUS cpssDxChLpmNodeNextPointersWrite
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   lpmLineOffset,
    IN GT_U32                                   numOfNextPointers,
    IN CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC      *nextPointerArrayPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLpmNodeNextPointersWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, lpmLineOffset, numOfNextPointers, nextPointerArrayPtr));

    rc = internal_cpssDxChLpmNodeNextPointersWrite(devNum, lpmLineOffset, numOfNextPointers, nextPointerArrayPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, lpmLineOffset, numOfNextPointers, nextPointerArrayPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLpmLastLookupStagesBypassEnableSet function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Set the enabling status of bypassing lookup stages 8-31.
*         bypassing the last lookup stages is recommended when IPv6 routing is not
*         used in the system, to decrease the latency caused by the LPM engine.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] bypassEnabled            - the bypass enabling status:
*                                      GT_TRUE  - enable bypassing of lookup stages 8-31
*                                      GT_FALSE - disable bypassing of lookup stages 8-31
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
static GT_STATUS internal_cpssDxChLpmLastLookupStagesBypassEnableSet
(
    IN GT_U8                                    devNum,
    IN GT_BOOL                                  bypassEnabled
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    return prvCpssHwPpSetRegField(devNum,
                                PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->LPM.globalConfig,
                                1, 1, bypassEnabled);
}

/**
* @internal cpssDxChLpmLastLookupStagesBypassEnableSet function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Set the enabling status of bypassing lookup stages 8-31.
*         bypassing the last lookup stages is recommended when IPv6 routing is not
*         used in the system, to decrease the latency caused by the LPM engine.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] bypassEnabled            - the bypass enabling status:
*                                      GT_TRUE  - enable bypassing of lookup stages 8-31
*                                      GT_FALSE - disable bypassing of lookup stages 8-31
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
GT_STATUS cpssDxChLpmLastLookupStagesBypassEnableSet
(
    IN GT_U8                                    devNum,
    IN GT_BOOL                                  bypassEnabled
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLpmLastLookupStagesBypassEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, bypassEnabled));

    rc = internal_cpssDxChLpmLastLookupStagesBypassEnableSet(devNum, bypassEnabled);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, bypassEnabled));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLpmLastLookupStagesBypassEnableGet function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Get the enabling status of bypassing lookup stages 8-31.
*         bypassing the last lookup stages is recommended when IPv6 routing is not
*         used in the system, to decrease the latency caused by the LPM engine.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] bypassEnabledPtr         - (pointer to) the bypass enabling status:
*                                      GT_TRUE  - enable bypassing of lookup stages 8-31
*                                      GT_FALSE - disable bypassing of lookup stages 8-31
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
static GT_STATUS internal_cpssDxChLpmLastLookupStagesBypassEnableGet
(
    IN  GT_U8                                   devNum,
    OUT GT_BOOL                                 *bypassEnabledPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32   value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(bypassEnabledPtr);


    rc = prvCpssHwPpGetRegField(devNum,
                                PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->LPM.globalConfig,
                                1, 1, &value);
    if(rc != GT_OK)
        return rc;

    *bypassEnabledPtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChLpmLastLookupStagesBypassEnableGet function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Get the enabling status of bypassing lookup stages 8-31.
*         bypassing the last lookup stages is recommended when IPv6 routing is not
*         used in the system, to decrease the latency caused by the LPM engine.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] bypassEnabledPtr         - (pointer to) the bypass enabling status:
*                                      GT_TRUE  - enable bypassing of lookup stages 8-31
*                                      GT_FALSE - disable bypassing of lookup stages 8-31
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
GT_STATUS cpssDxChLpmLastLookupStagesBypassEnableGet
(
    IN  GT_U8                                   devNum,
    OUT GT_BOOL                                 *bypassEnabledPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLpmLastLookupStagesBypassEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, bypassEnabledPtr));

    rc = internal_cpssDxChLpmLastLookupStagesBypassEnableGet(devNum, bypassEnabledPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, bypassEnabledPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLpmPortSipLookupEnableSet function
* @endinternal
*
* @brief   Enable/Disable SIP lookup for packets received from the given port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number
* @param[in] enable                   - GT_TRUE:    SIP Lookup on the port
*                                      GT_FALSE:  disable SIP Lookup on the port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on out of range
* @retval GT_BAD_STATE             - on invalid hardware value set
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When this attribute is set to "False", SIP Lookup is never performed for
*       the source ePort, regardless of other SIP related configurations
*       (i.e SIP/SA check, SIP filtering etc')
*
*/
static GT_STATUS internal_cpssDxChLpmPortSipLookupEnableSet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  GT_BOOL                      enable
)
{
    GT_STATUS   rc = GT_OK; /* return code */
    GT_U32      data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
     PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if(enable==GT_TRUE)
        data=0;/* disable SIP Lookup bit in hw is false */
    else
        data=1;/* disable SIP Lookup bit in hw is true */

    rc = prvCpssDxChWriteTableEntryField(devNum,
                                         CPSS_DXCH_SIP5_TABLE_IPVX_INGRESS_EPORT_E,
                                         portNum,
                                         PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                         SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_DISABLE_SIP_LOOKUP,
                                         PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                         data);
    return rc;
}

/**
* @internal cpssDxChLpmPortSipLookupEnableSet function
* @endinternal
*
* @brief   Enable/Disable SIP lookup for packets received from the given port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number
* @param[in] enable                   - GT_TRUE:    SIP Lookup on the port
*                                      GT_FALSE:  disable SIP Lookup on the port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on out of range
* @retval GT_BAD_STATE             - on invalid hardware value set
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When this attribute is set to "False", SIP Lookup is never performed for
*       the source ePort, regardless of other SIP related configurations
*       (i.e SIP/SA check, SIP filtering etc')
*
*/
GT_STATUS cpssDxChLpmPortSipLookupEnableSet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  GT_BOOL                      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLpmPortSipLookupEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChLpmPortSipLookupEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLpmPortSipLookupEnableGet function
* @endinternal
*
* @brief   Return the SIP Lookup status for packets received from the given port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE:   enable SIP Lookup on the port
*                                      GT_FALSE:  disable SIP Lookup on the port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on out of range
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note When this attribute is set to "False", SIP Lookup is never performed for
*       the source ePort, regardless of other SIP related configurations
*       (i.e SIP/SA check, SIP filtering etc')
*
*/
static GT_STATUS internal_cpssDxChLpmPortSipLookupEnableGet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    OUT GT_BOOL                      *enablePtr
)
{
    GT_STATUS   rc = GT_OK; /* return code */
    GT_U32      data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_IPVX_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_DISABLE_SIP_LOOKUP,
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &data);

    if(data==0)
        *enablePtr = GT_TRUE;/* SIP Lookup is enabled */
    else
        *enablePtr = GT_FALSE;/* SIP Lookup is disabled */

    return rc;
}

/**
* @internal cpssDxChLpmPortSipLookupEnableGet function
* @endinternal
*
* @brief   Return the SIP Lookup status for packets received from the given port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE:   enable SIP Lookup on the port
*                                      GT_FALSE:  disable SIP Lookup on the port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on out of range
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note When this attribute is set to "False", SIP Lookup is never performed for
*       the source ePort, regardless of other SIP related configurations
*       (i.e SIP/SA check, SIP filtering etc')
*
*/
GT_STATUS cpssDxChLpmPortSipLookupEnableGet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    OUT GT_BOOL                      *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLpmPortSipLookupEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChLpmPortSipLookupEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLpmLeafEntryWrite function
* @endinternal
*
* @brief   Write an LPM leaf entry, which is used for policy based routing, to the
*         HW
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] leafIndex                - leaf index within the LPM PBR block
* @param[in] leafPtr                  - (pointer to) the leaf entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
* @note The PBR base address is set in cpssDxChCfgPpLogicalInit according to the
*       maxNumOfPbrEntries parameter in CPSS_DXCH_PP_CONFIG_INIT_STC.
*       See the datasheet for maximal leafIndex.
*
*/
static GT_STATUS internal_cpssDxChLpmLeafEntryWrite
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   leafIndex,
    IN CPSS_DXCH_LPM_LEAF_ENTRY_STC             *leafPtr
)
{
    GT_U32                              hwData;
    CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC nextPointerData;
    GT_U32                              lpmIndex;
    GT_STATUS                           rc;
    GT_U32                              maxMemSize = 0;
    PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC    leafPrvData = {0};

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(leafPtr);

    if (leafIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.maxNumOfPbrEntries)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (leafPtr->entryType)
    {
    case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            maxMemSize = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerNextHop;
        }
        break;
        case CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E:
        case CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E:
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            break; 
        case CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E:
            if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            maxMemSize = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.ecmpQos;
            break; 
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (leafPtr->ipv6MCGroupScopeLevel)
    {
        case CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E:
        case CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E:
        case CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E:
        case CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if (leafPtr->index >= maxMemSize)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        /* translate CPSS_DXCH_LPM_LEAF_ENTRY_STC into PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC*/
        prvCpssLpmFalconFillPrvLeafValue(leafPtr, &leafPrvData);
        /* Calculate LPM leaf value */
        rc = prvCpssLpmFalconBuildLeafValue(&leafPrvData, &hwData);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* Write leaf value to HW */
        rc = prvCpssDxChWriteTableEntry(devNum,
                            CPSS_DXCH_SIP6_TABLE_PBR_E,
                            leafIndex,
                            &hwData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        cpssOsMemSet(&nextPointerData, 0x0, sizeof(nextPointerData));

        nextPointerData.pointerType = leafPtr->entryType;
        nextPointerData.pointerData.nextHopOrEcmpPointer.entryIndex = leafPtr->index;
        nextPointerData.pointerData.nextHopOrEcmpPointer.ucRpfCheckEnable =
            leafPtr->ucRPFCheckEnable;
        nextPointerData.pointerData.nextHopOrEcmpPointer.srcAddrCheckMismatchEnable =
            leafPtr->sipSaCheckMismatchEnable;
        nextPointerData.pointerData.nextHopOrEcmpPointer.ipv6McGroupScopeLevel =
            leafPtr->ipv6MCGroupScopeLevel;

        rc = prvCpssDxChLpmNextPointerDataBuild(devNum, nextPointerData, &hwData);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* calculate HW index */
        rc = prvCpssDxChLpmLeafEntryCalcLpmHwIndexFromSwIndex(devNum,leafIndex,&lpmIndex);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
        {
            /* check that the lpmLineOffset is in range of half mode
            meanning half of the memory lines */
            if ((lpmIndex%PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap)>=
                (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock/2))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            /* check that the leafIndex is in range of full mode
            meanning can not be more than numOfLinesInBlock */
            if ((lpmIndex%PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap)>=
                PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }

        rc = prvCpssDxChWriteTableEntry(devNum,
                            CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                            lpmIndex,
                            &hwData);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
        {
            /* Write the node  to the second half*/
        rc = prvCpssDxChWriteTableEntry(devNum,
                                        CPSS_DXCH_SIP5_20_TABLE_LPM_MEM_PIPE_1_E,
                                        lpmIndex,
                                        &hwData);
        }
    }  
    
    return rc;
}

/**
* @internal cpssDxChLpmLeafEntryWrite function
* @endinternal
*
* @brief   Write an LPM leaf entry, which is used for policy based routing, to the
*         HW
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] leafIndex                - leaf index within the LPM PBR block
* @param[in] leafPtr                  - (pointer to) the leaf entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
* @note The PBR base address is set in cpssDxChCfgPpLogicalInit according to the
*       maxNumOfPbrEntries parameter in CPSS_DXCH_PP_CONFIG_INIT_STC.
*       See the datasheet for maximal leafIndex.
*
*/
GT_STATUS cpssDxChLpmLeafEntryWrite
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   leafIndex,
    IN CPSS_DXCH_LPM_LEAF_ENTRY_STC             *leafPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLpmLeafEntryWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, leafIndex, leafPtr));

    rc = internal_cpssDxChLpmLeafEntryWrite(devNum, leafIndex, leafPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, leafIndex, leafPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLpmLeafEntryRead function
* @endinternal
*
* @brief   Read an LPM leaf entry, which is used for policy based routing, from the
*         HW
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] leafIndex                - leaf index within the LPM PBR block
*
* @param[out] leafPtr                  - (pointer to) the leaf entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
* @note The PBR base address is set in cpssDxChCfgPpLogicalInit according to the
*       maxNumOfPbrEntries parameter in CPSS_DXCH_PP_CONFIG_INIT_STC.
*       See the datasheet for maximal leafIndex.
*
*/
static GT_STATUS internal_cpssDxChLpmLeafEntryRead
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  leafIndex,
    OUT CPSS_DXCH_LPM_LEAF_ENTRY_STC            *leafPtr
)
{
    GT_U32                              hwData;
    CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC nextPointerData;
    GT_U32                              lpmIndex;
    GT_STATUS                           rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(leafPtr);

    if (leafIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.maxNumOfPbrEntries)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* Read LPM leaf HW value */
        rc = prvCpssDxChReadTableEntry(devNum,
                                       CPSS_DXCH_SIP6_TABLE_PBR_E,
                                       leafIndex,
                                       &hwData);

        /* Convert HW value to LPM leaf SW format */
        rc =  prvCpssLpmFalconDecodeLeafValue(hwData, leafPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {        
        /* calculate HW index */
        rc = prvCpssDxChLpmLeafEntryCalcLpmHwIndexFromSwIndex(devNum,leafIndex,&lpmIndex);
        if (rc != GT_OK)
        {
            return rc;
        }
    
        rc = prvCpssDxChReadTableEntry(devNum,
                                   CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                   lpmIndex,
                                   &hwData);
        if (rc != GT_OK)
        {
            return rc;
        }

        cpssOsMemSet(&nextPointerData, 0x0, sizeof(nextPointerData));

        rc = prvCpssDxChLpmNextPointerDataDecode(&hwData, &nextPointerData);
        if (rc != GT_OK)
        {
            return rc;
        }

        leafPtr->entryType = nextPointerData.pointerType;
        leafPtr->index = nextPointerData.pointerData.nextHopOrEcmpPointer.entryIndex;
        leafPtr->ucRPFCheckEnable =
            nextPointerData.pointerData.nextHopOrEcmpPointer.ucRpfCheckEnable;
        leafPtr->sipSaCheckMismatchEnable =
            nextPointerData.pointerData.nextHopOrEcmpPointer.srcAddrCheckMismatchEnable;
        leafPtr->ipv6MCGroupScopeLevel =
            nextPointerData.pointerData.nextHopOrEcmpPointer.ipv6McGroupScopeLevel;
    }

    return GT_OK;
}

/**
* @internal cpssDxChLpmLeafEntryRead function
* @endinternal
*
* @brief   Read an LPM leaf entry, which is used for policy based routing, from the
*         HW
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] leafIndex                - leaf index within the LPM PBR block
*
* @param[out] leafPtr                  - (pointer to) the leaf entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
* @note The PBR base address is set in cpssDxChCfgPpLogicalInit according to the
*       maxNumOfPbrEntries parameter in CPSS_DXCH_PP_CONFIG_INIT_STC.
*       See the datasheet for maximal leafIndex.
*
*/
GT_STATUS cpssDxChLpmLeafEntryRead
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  leafIndex,
    OUT CPSS_DXCH_LPM_LEAF_ENTRY_STC            *leafPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLpmLeafEntryRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, leafIndex, leafPtr));

    rc = internal_cpssDxChLpmLeafEntryRead(devNum, leafIndex, leafPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, leafIndex, leafPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLpmPbrBaseAndSizeGet function
* @endinternal
*
* @brief   Return the base address and size of the policy based routing area within
*         the LPM memory
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] baseOffsetPtr            - (pointer to) PBR base offset within the LPM memory
* @param[out] sizePtr                  - (pointer to) number of PBR leaf entries
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
static GT_STATUS internal_cpssDxChLpmPbrBaseAndSizeGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *baseOffsetPtr,
    OUT GT_U32  *sizePtr
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(baseOffsetPtr);
    CPSS_NULL_PTR_CHECK_MAC(sizePtr);

    *sizePtr = PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.maxNumOfPbrEntries;

    rc = prvCpssHwPpGetRegField(devNum,
                                PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->LPM.directAccessMode,
                                0, PRV_CPSS_SIP_6_CHECK_MAC(devNum) ? 20 : 19,
                                baseOffsetPtr);
    return rc;
}

/**
* @internal cpssDxChLpmPbrBaseAndSizeGet function
* @endinternal
*
* @brief   Return the base address and size of the policy based routing area within
*         the LPM memory
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] baseOffsetPtr            - (pointer to) PBR base offset within the LPM memory
* @param[out] sizePtr                  - (pointer to) number of PBR leaf entries
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
GT_STATUS cpssDxChLpmPbrBaseAndSizeGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *baseOffsetPtr,
    OUT GT_U32  *sizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLpmPbrBaseAndSizeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, baseOffsetPtr, sizePtr));

    rc = internal_cpssDxChLpmPbrBaseAndSizeGet(devNum, baseOffsetPtr, sizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, baseOffsetPtr, sizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLpmExceptionStatusGet function
* @endinternal
*
* @brief   Return the Lpm Exception Status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] lpmEngine                - LPM engine index 
*       (APPLICABLE DEVICES Bobcat3; Aldrin2; Falcon )
*       (APPLICABLE RANGES: Bobcat3: 0..3, Aldrin2: 0..1, Falcon: 0..7)
*
* @param[out] hitExceptionPtr          - (pointer to) hit Exception happened in LPM Stages
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_STATE             - wrong value returned from hw
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/

static GT_STATUS internal_cpssDxChLpmExceptionStatusGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              lpmEngine,
    OUT CPSS_DXCH_LPM_EXCEPTION_STATUS_ENT  *hitExceptionPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32   value;
    GT_U32   regAddr;
    GT_U32   maxLpmEngine;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(hitExceptionPtr);

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        maxLpmEngine = PRV_CPSS_DXCH_LPM_BC3_ENGINE_NUM_CNS;
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        maxLpmEngine = PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.numOfPortGroups;
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        maxLpmEngine = PRV_CPSS_DXCH_LPM_ALDRIN2_ENGINE_NUM_CNS;
    }
    else
    {
        maxLpmEngine = 0;/* lpmEngine is dont care */
    }

    if(maxLpmEngine && (lpmEngine >= maxLpmEngine))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "lpmEngine must be [0..%d] but got[%d]",
            maxLpmEngine - 1,
            lpmEngine);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->LPM.exceptionStatus;

    if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {   
        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))  
            regAddr += (lpmEngine<<24);

        rc = prvCpssHwPpGetRegField(devNum,
                                    regAddr,
                                    0, 4, &value); 
    }
    else
    {
        rc = prvCpssHwPpPortGroupGetRegField(devNum,
                                             1<<lpmEngine,
                                             regAddr,0,4,&value);
    }
    if(rc != GT_OK)
        return rc;

    switch(value)
    {
        case 0:
            *hitExceptionPtr = CPSS_DXCH_LPM_EXCEPTION_HIT_E;
            break;
        case 1:
             *hitExceptionPtr = CPSS_DXCH_LPM_EXCEPTION_LPM_ECC_E;
            break;
        case 2:
             *hitExceptionPtr = CPSS_DXCH_LPM_EXCEPTION_ECMP_ECC_E;
            break;
        case 3:
             *hitExceptionPtr = CPSS_DXCH_LPM_EXCEPTION_PBR_BUCKET_E;
            break;
        case 4:
             *hitExceptionPtr = CPSS_DXCH_LPM_EXCEPTION_CONTINUE_TO_LOOKUP_E;
            break;
        case 5:
             *hitExceptionPtr = CPSS_DXCH_LPM_EXCEPTION_UNICAST_LOOKUP_0_E;
            break;
        case 6:
             *hitExceptionPtr = CPSS_DXCH_LPM_EXCEPTION_UNICAST_LOOKUP_1_E;
            break;
        case 7:
             *hitExceptionPtr = CPSS_DXCH_LPM_EXCEPTION_DST_G_IPV4_PACKETS_E;
            break;
        case 8:
             *hitExceptionPtr = CPSS_DXCH_LPM_EXCEPTION_SRC_G_IPV4_PACKETS_E;
            break;
        case 9:
             *hitExceptionPtr = CPSS_DXCH_LPM_EXCEPTION_UNICAST_LOOKUP_0_IPV6_PACKETS_E;
            break;
        case 10:
             *hitExceptionPtr = CPSS_DXCH_LPM_EXCEPTION_UNICAST_LOOKUP_1_IPV6_PACKETS_E;
            break;
        case 11:
             *hitExceptionPtr = CPSS_DXCH_LPM_EXCEPTION_DST_G_IPV6_PACKETS_E;
            break;
        case 12:
             *hitExceptionPtr = CPSS_DXCH_LPM_EXCEPTION_SRC_G_IPV6_PACKETS_E;
            break;
        case 13:
             *hitExceptionPtr = CPSS_DXCH_LPM_EXCEPTION_FCOE_D_ID_LOOKUP_E;
            break;
        case 14:
             *hitExceptionPtr = CPSS_DXCH_LPM_EXCEPTION_FCOE_S_ID_LOOKUP_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            break;
    }

    return GT_OK;
}

/**
* @internal cpssDxChLpmExceptionStatusGet function
* @endinternal
*
* @brief   Return the Lpm Exception Status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] lpmEngine                - LPM engine index 
*       (APPLICABLE DEVICES Bobcat3; Aldrin2; Falcon )
*       (APPLICABLE RANGES: Bobcat3: 0..3, Aldrin2: 0..1, Falcon: 0..7) 
*
* @param[out] hitExceptionPtr          - (pointer to) hit Exception happened in LPM Stages
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_STATE             - wrong value returned from hw
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
GT_STATUS cpssDxChLpmExceptionStatusGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              lpmEngine,
    OUT CPSS_DXCH_LPM_EXCEPTION_STATUS_ENT  *hitExceptionPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLpmExceptionStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, lpmEngine, hitExceptionPtr));

    rc = internal_cpssDxChLpmExceptionStatusGet(devNum, lpmEngine, hitExceptionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, lpmEngine, hitExceptionPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLpmMemoryModeGet function
* @endinternal
*
* @brief   This function get LPM memory mode
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; xCat3; Lion2.
*
* @param[in] devNum                   -  the device number
*                                      lpmMemMode     - lpm memory mode
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_NOT_SUPPORTED         - in case DFX unit is not supported
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
static GT_STATUS internal_cpssDxChLpmMemoryModeGet
(
    IN    GT_U8                              devNum,
    OUT    CPSS_DXCH_LPM_RAM_MEM_MODE_ENT    *lpmMemoryModePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32   value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(lpmMemoryModePtr);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) != GT_FALSE)
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE)
        {
            rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum,
                                    PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl15,
                                    25, 1, &value);
            if(value == 0)
            {
               *lpmMemoryModePtr = CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E;
            }
            else
            {
               *lpmMemoryModePtr = CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;
            }
        }
        else
        {
           *lpmMemoryModePtr = CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;
        }
    }
    else
    {
        rc = GT_NOT_SUPPORTED;
    }

    return rc;
}


/**
* @internal cpssDxChLpmMemoryModeGet function
* @endinternal
*
* @brief   This function gets LPM memory mode
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; xCat3; Lion2.
*
* @param[in] devNum                   -  the device number
*                                      lpmMemMode     - lpm memory mode
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_NOT_SUPPORTED         - in case DFX unit is not supported
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS cpssDxChLpmMemoryModeGet
(
    IN    GT_U8                              devNum,
    OUT    CPSS_DXCH_LPM_RAM_MEM_MODE_ENT    *lpmMemoryModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLpmMemoryModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, lpmMemoryModePtr));

    rc = internal_cpssDxChLpmMemoryModeGet(devNum, lpmMemoryModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, lpmMemoryModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}






