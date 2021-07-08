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
* @file tgfTcamManagerGen.h
*
* @brief This file contains TCAM manager module.
*
* @version   1
********************************************************************************
*/
#ifndef __tgfTcamManagerGenh
#define __tgfTcamManagerGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/resourceManager/cpssDxChTcamManager.h>
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
#endif /* EXMXPM_FAMILY */


/******************************************************************************\
 *                          Private type definitions                          *
\******************************************************************************/

/**
* @enum PRV_TGF_TCAM_MANAGER_TCAM_TYPE_ENT
 *
 * @brief TCAM types that can be managed by the TCAM manager.
*/
typedef enum{

    /** @brief xCat and above router TCAM
     *  Has 4 columns per line, row based search order,
     */
    PRV_TGF_TCAM_MANAGER_XCAT_AND_ABOVE_ROUTER_TCAM_E = 0,

    /** @brief xCat and above PCL TCAM
     *  Has 4 columns per line, row based search order,
     */
    PRV_TGF_TCAM_MANAGER_XCAT_AND_ABOVE_PCL_TCAM_E    = 1

} PRV_TGF_TCAM_MANAGER_TCAM_TYPE_ENT;

/**
* @enum PRV_TGF_TCAM_MANAGER_RANGE_UPDATE_METHOD_ENT
 *
 * @brief Defines how to rearrange the already allocated entries when updating
 * the TCAM range.
*/
typedef enum{

    /** Don't move existing entries at all. */
    PRV_TGF_TCAM_MANAGER_DO_NOT_MOVE_RANGE_UPDATE_METHOD_E = 0,

    /** Align entries to the top of the new range and compress */
    PRV_TGF_TCAM_MANAGER_MOVE_TOP_AND_COMPRESS_RANGE_UPDATE_METHOD_E,

    /** Align entries to bottom of the new range and compress */
    PRV_TGF_TCAM_MANAGER_MOVE_BOTTOM_AND_COMPRESS_RANGE_UPDATE_METHOD_E,

    /** @brief Align entries to middle of the new range and compress
     *  around the middle
     */
    PRV_TGF_TCAM_MANAGER_MOVE_MIDDLE_AND_COMPRESS_RANGE_UPDATE_METHOD_E,

    /** @brief Keeps the exact offsets from the top of the range, meaning
     *  that for each entry, the offset from the top of the new
     *  range will be the same as the offset form the top of the
     *  old range.
     */
    PRV_TGF_TCAM_MANAGER_MOVE_TOP_AND_KEEP_OFFSETS_RANGE_UPDATE_METHOD_E

} PRV_TGF_TCAM_MANAGER_RANGE_UPDATE_METHOD_ENT;

/**
* @struct PRV_TGF_TCAM_MANAGER_RANGE_STC
 *
 * @brief Holds physical TCAM range.
*/
typedef struct{

    /** first line;       range 0..(2^18 */
    GT_U32 firstLine;

    /** last line (including); range 0..(2^18 */
    GT_U32 lastLine;

} PRV_TGF_TCAM_MANAGER_RANGE_STC;


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
);

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
);

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
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfTcamManagerGenh */


