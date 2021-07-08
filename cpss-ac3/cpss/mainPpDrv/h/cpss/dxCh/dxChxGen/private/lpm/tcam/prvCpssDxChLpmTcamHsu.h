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
* @file prvCpssDxChLpmTcamHsu.h
*
* @brief internal HSU functions for TCAM shadow.
*
* @version   2
********************************************************************************
*/
#ifndef __prvCpssDxChLpmTcamHsuh
#define __prvCpssDxChLpmTcamHsuh

#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamCommonTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvCpssDxChLpmTcamDbGet function
* @endinternal
*
* @brief   Retrieves a specific shadow's Table memory Size needed and info
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in,out] iterationSizePtr         - (pointer to) data size in bytes supposed to be processed
*                                      in current iteration.
* @param[in,out] iterPtr                  - the iterator, to start - set to 0.
*
* @param[out] tableSizePtr             - (pointer to) the table size calculated (in bytes)
* @param[out] tablePtr                 - (pointer to) the table size info block
* @param[in,out] iterationSizePtr         - (pointer to) data size in bytes left after iteration .
* @param[in,out] iterPtr                  - the iterator, if = 0 then the operation is done.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In case the LPM DB uses TCAM manager and creates the TCAM manager
*       internally, then the LPM DB is also responsible for exporting and
*       importing the TCAM manager data. The TCAM manager must be imported
*       before VR, UC and MC entries.
*       If the LPM uses TCAM manager (internally or externally) then the LPM DB
*       must update the TCAM manager client callback functions.
*       Data is arranged in entries of different types in the following order:
*       - TCAM manager entries, if needed. Each TCAM manager entry is 1k
*       of the TCAM manager HSU data (last entry is up to 1k).
*       - VR entry per virtual router in the LPM DB. If the LPM DB contains
*       no VR, then this section is empty.
*       - Dummy VR entry. Note that the dummy VR entry will always exists,
*       even if there are no VR in the LPM DB
*       - If the LPM DB supports IPv4 protocol then for each virtual router in
*       the LPM DB the following section will appear. The order of the
*       virtual routers is according to the virtual router ID. In case
*       there are no virtual routers, this section will be empty.
*       - UC entry for each IPv4 UC entry other than the default.
*       - Dummy UC entry.
*       - MC entry for each IPv4 MC entry other than the default.
*       - Dummy MC entry
*       - If the LPM DB supports IPv6 protocol then for each virtual router in
*       the LPM DB the following section will appear. The order of the
*       virtual routers is according to the virtual router ID. In case
*       there are no virtual routers, this section will be empty.
*       - UC entry for each IPv6 UC entry other than the default.
*       - Dummy UC entry.
*       - MC entry for each IPv6 MC entry other than the default.
*       - Dummy MC entry.
*       Note that if the LPM DB doesn't support a protocol stack then the
*       sections related to this protocol stack will be empty (will not include
*       the dummy entries as well).
*       If a VR doesn't support one of the prefix types then the section for
*       this prefix type will be empty but will include dummy.
*
*/
GT_STATUS prvCpssDxChLpmTcamDbGet
(
    IN    PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC   *lpmDbPtr,
    OUT   GT_U32                              *tableSizePtr,
    OUT   GT_VOID                             *tablePtr,
    INOUT GT_U32                              *iterationSizePtr,
    INOUT GT_UINTPTR                          *iterPtr
);

/**
* @internal prvCpssDxChLpmTcamDbSet function
* @endinternal
*
* @brief   Set the data needed for core IP shadow reconstruction used after HSU.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] tablePtr                 - (pointer to)the table size info block.
* @param[in,out] iterationSizePtr         - (pointer to) data size in bytes supposed to be processed.
*                                      in current iteration.
* @param[in,out] iterPtr                  - the iterator, to start - set to 0.
* @param[in,out] iterationSizePtr         - (pointer to) data size in bytes left after iteration .
* @param[in,out] iterPtr                  - the iterator, if = 0 then the operation is done.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
*/
GT_STATUS prvCpssDxChLpmTcamDbSet
(
    IN     PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC  *lpmDbPtr,
    IN     GT_VOID                            *tablePtr,
    INOUT  GT_U32                             *iterationSizePtr,
    INOUT  GT_UINTPTR                         *iterPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChLpmTcamHsuh */


