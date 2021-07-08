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
* @file cpssTmUtDefs.h
*
* @brief common TM UTs declarations and APIs.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssTmUtDefs
#define __cpssTmUtDefs

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/tm/cpssTmPublicDefs.h>

#define UT_TM_MAX_LEVELS_CNS (CPSS_TM_LEVEL_P_E+1)

#define UT_TM_MIN_PORT_QUANTUM_CNS 40 /* 40 * 64*/


/**
* @internal cpssTmTestTmCtlLibInit function
* @endinternal
*
* @brief   common test API to open TM LIB after verify TM lib is closed
*
* @param[in] devNum                   - Device number.
*                                       GT_STATUS
*/
extern GT_STATUS cpssTmTestTmCtlLibInit
(
    IN GT_U8 devNum
);


/**
* @internal cpssTmTestPeriodicSchemeConfiguration function
* @endinternal
*
* @brief   common test API to configure periodic scheme
*
* @param[in] devNum                   - Device number.
*                                       GT_STATUS
*/
extern GT_STATUS cpssTmTestPeriodicSchemeConfiguration
(
    IN GT_U8 devNum
);

/**
* @internal cpssTmTestTreeBuild function
* @endinternal
*
* @brief   common test API to create single thread tree:
*         port(1)
*         \
*         cnode
*         \
*         bnode
*         \
*         anode
*         \
*         queue
* @param[in] devNum                   - Device number.
*
* @param[out] queueIndPtr              - (pointer of) queue index.
* @param[out] aNodeIndPtr              - (pointer of) anode index.
* @param[out] bNodeIndPtr              - (pointer of) bnode index.
* @param[out] cNodeIndPtr              - (pointer of) cnode index.
*                                       GT_STATUS
*/
GT_STATUS cpssTmTestTreeBuild
(
    IN  GT_U8   devNum,
    OUT GT_U32  *queueIndPtr,
    OUT GT_U32  *aNodeIndPtr,
    OUT GT_U32  *bNodeIndPtr,
    OUT GT_U32  *cNodeIndPtr
);

/**
* @internal cpssTmTestTmClose function
* @endinternal
*
* @brief   common test API to Close TM LIB
*
* @param[in] devNum                   - Device number.
*                                       GT_STATUS
*/
extern GT_STATUS cpssTmTestTmClose
(
    IN GT_U8 devNum
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __utfMainh */


