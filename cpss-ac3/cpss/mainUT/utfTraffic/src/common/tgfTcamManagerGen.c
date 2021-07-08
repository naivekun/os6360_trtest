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
* @file tgfTcamManagerGen.c
*
* @brief Generic API implementation for TCAM Manager
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfTcamManagerGen.h>


/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfTcamManagerCreate function
* @endinternal
*
* @brief   Creates TCAM manager and returns handler for the created TCAM manager.
*
* @param[in] tcamType                 - the TCAM type that the TCAM manager will handle
* @param[in] rangePtr                 - (pointer to) the TCAM range allocated for this TCAM manager
*
* @param[out] tcamManagerHandlerPtrPtr - (pointer to) pointer of the created TCAM manager object
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - if no CPU memory for memory allocation
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfTcamManagerCreate
(
    IN  PRV_TGF_TCAM_MANAGER_TCAM_TYPE_ENT    tcamType,
    IN  PRV_TGF_TCAM_MANAGER_RANGE_STC       *rangePtr,
    OUT GT_VOID                             **tcamManagerHandlerPtrPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT    dxChTcamType;
    CPSS_DXCH_TCAM_MANAGER_RANGE_STC        dxChRange;


    /* convert tcamType into device specific format */
    switch (tcamType)
    {
        case PRV_TGF_TCAM_MANAGER_XCAT_AND_ABOVE_ROUTER_TCAM_E:
            dxChTcamType = CPSS_DXCH_TCAM_MANAGER_XCAT_AND_ABOVE_ROUTER_TCAM_E;
            break;

        case PRV_TGF_TCAM_MANAGER_XCAT_AND_ABOVE_PCL_TCAM_E:
            dxChTcamType = CPSS_DXCH_TCAM_MANAGER_XCAT_AND_ABOVE_PCL_TCAM_E;
            break;

        default:
            return GT_BAD_PARAM;
    }
    
    /* convert rangePtr into device specific format */
    dxChRange.firstLine = rangePtr->firstLine;
    dxChRange.lastLine  = rangePtr->lastLine;
    
    /* call device specific API */
    return cpssDxChTcamManagerCreate(dxChTcamType, &dxChRange, tcamManagerHandlerPtrPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tcamType);
    TGF_PARAM_NOT_USED(rangePtr);
    TGF_PARAM_NOT_USED(tcamManagerHandlerPtrPtr);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfTcamManagerDelete function
* @endinternal
*
* @brief   Deletes the TCAM manager.
*
* @param[in] tcamManagerHandlerPtr    - the TCAM manager handler
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on error
*
* @note All entries allocated by the TCAM manager must be freed prior of calling
*       this API. Clients may be registered upon calling the API.
*
*/
GT_STATUS prvTgfTcamManagerDelete
(
    IN  GT_VOID         *tcamManagerHandlerPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChTcamManagerDelete(tcamManagerHandlerPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tcamManagerHandlerPtr);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfTcamManagerRangeUpdate function
* @endinternal
*
* @brief   Updates the range allocated for the TCAM manager.
*
* @param[in] tcamManagerHandlerPtr    - (pointer to) the TCAM manager handler
* @param[in] rangePtr                 - (pointer to) the new TCAM range
* @param[in] rangeUpdateMethod        - update method to use during the range update
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfTcamManagerRangeUpdate
(
    IN  GT_VOID                                      *tcamManagerHandlerPtr,
    IN  PRV_TGF_TCAM_MANAGER_RANGE_STC               *rangePtr,
    IN  PRV_TGF_TCAM_MANAGER_RANGE_UPDATE_METHOD_ENT  rangeUpdateMethod
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_TCAM_MANAGER_RANGE_STC                dxChRange;
    CPSS_DXCH_TCAM_MANAGER_RANGE_UPDATE_METHOD_ENT  dxChRangeUpdateMethod;


    /* convert rangePtr into device specific format */
    dxChRange.firstLine = rangePtr->firstLine;
    dxChRange.lastLine  = rangePtr->lastLine;

    /* convert rangeUpdateMethod into device specific format */
    switch (rangeUpdateMethod)
    {
        case PRV_TGF_TCAM_MANAGER_DO_NOT_MOVE_RANGE_UPDATE_METHOD_E:
            dxChRangeUpdateMethod = CPSS_DXCH_TCAM_MANAGER_DO_NOT_MOVE_RANGE_UPDATE_METHOD_E;
            break;

        case PRV_TGF_TCAM_MANAGER_MOVE_TOP_AND_COMPRESS_RANGE_UPDATE_METHOD_E:
            dxChRangeUpdateMethod = CPSS_DXCH_TCAM_MANAGER_MOVE_TOP_AND_COMPRESS_RANGE_UPDATE_METHOD_E;
            break;

        case PRV_TGF_TCAM_MANAGER_MOVE_BOTTOM_AND_COMPRESS_RANGE_UPDATE_METHOD_E:
            dxChRangeUpdateMethod = CPSS_DXCH_TCAM_MANAGER_MOVE_BOTTOM_AND_COMPRESS_RANGE_UPDATE_METHOD_E;
            break;

        case PRV_TGF_TCAM_MANAGER_MOVE_MIDDLE_AND_COMPRESS_RANGE_UPDATE_METHOD_E:
            dxChRangeUpdateMethod = CPSS_DXCH_TCAM_MANAGER_MOVE_MIDDLE_AND_COMPRESS_RANGE_UPDATE_METHOD_E;
            break;

        case PRV_TGF_TCAM_MANAGER_MOVE_TOP_AND_KEEP_OFFSETS_RANGE_UPDATE_METHOD_E:
            dxChRangeUpdateMethod = CPSS_DXCH_TCAM_MANAGER_MOVE_TOP_AND_KEEP_OFFSETS_RANGE_UPDATE_METHOD_E;
            break;

        default:
            return GT_BAD_PARAM;
    }
    
    /* call device specific API */
    return cpssDxChTcamManagerRangeUpdate(tcamManagerHandlerPtr, &dxChRange, dxChRangeUpdateMethod);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tcamManagerHandlerPtr);
    TGF_PARAM_NOT_USED(rangePtr);
    TGF_PARAM_NOT_USED(rangeUpdateMethod);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}


