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
* @file prvTgfBasicDynamicLearning.h
*
* @brief Basic dynamic learning
*
* @version   10
********************************************************************************
*/
#ifndef __prvTgfBasicDynamicLearningh
#define __prvTgfBasicDynamicLearningh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void utfCpssDxChBrgFdbFromCpuAuMsgStatusGet
(
    IN GT_U8    devNum,
    IN GT_BOOL  mustSucceed
);

GT_STATUS utfCpssDxChBrgFdbMacEntrySet
(
    IN GT_U8                        devNum,
    IN CPSS_MAC_ENTRY_EXT_STC       *macEntryPtr
);

GT_STATUS utfCpssDxChBrgFdbMacEntryInvalidate
(
    IN GT_U8         devNum,
    IN GT_U32        index
);

GT_STATUS utfCpssDxChBrgFdbMacEntryRead
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    OUT GT_BOOL                 *validPtr,
    OUT GT_BOOL                 *skipPtr,
    OUT GT_BOOL                 *agedPtr,
    OUT GT_HW_DEV_NUM           *associatedHwDevNumPtr,
    OUT CPSS_MAC_ENTRY_EXT_STC  *entryPtr
);

GT_STATUS utfCpssDxChBrgFdbQaSend
(
    IN  GT_U8                       devNum,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC   *macEntryKeyPtr
);

GT_STATUS utfCpssDxChBrgFdbMacEntryDelete
(
    IN GT_U8                        devNum,
    IN CPSS_MAC_ENTRY_EXT_KEY_STC   *macEntryKeyPtr
);

GT_STATUS utfCpssDxChBrgFdbMacEntryWrite
(
    IN GT_U8                        devNum,
    IN GT_U32                       index,
    IN GT_BOOL                      skip,
    IN CPSS_MAC_ENTRY_EXT_STC       *macEntryPtr
);

GT_STATUS utfCpssDxChBrgFdbMacEntryStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    OUT GT_BOOL                 *validPtr,
    OUT GT_BOOL                 *skipPtr
);

GT_STATUS utfCpssDxChBrgFdbMacEntryAgeBitSet
(
    IN GT_U8        devNum,
    IN GT_U32       index,
    IN GT_BOOL      age
);

/**
* @internal prvTgfBrgBdlConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgBdlConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgBdlTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgBdlTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgBdlConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgBdlConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfFdbLearning128KAddressesTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbLearning128KAddressesTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgBdlConfigurationDsaTagEnableSet function
* @endinternal
*
* @brief   1. enable/disable running the test in DSA tag mode.
*         2. set bridge bypass mode
*         3. set DSA tag type
* @param[in] dsaTagEnable             - enable/disable running the test in DSA tag mode.
* @param[in] dsaBypassBridge          - relevant when dsaTagEnable = TRUE
*                                      indicate that the traffic with 'forward' DSA tag do or not
*                                      'bypass' to the bridge.
* @param[in] dsaType                  - DSA type.
*
* @retval GT_TRUE                  - test can start - parameters are valid
* @retval GT_FALSE                 - test can't start - parameters are NOT valid
*/
GT_BOOL prvTgfBrgBdlConfigurationDsaTagEnableSet
(
    IN GT_BOOL                  dsaTagEnable,
    IN GT_BOOL                  dsaBypassBridge,
    IN TGF_DSA_TYPE_ENT         dsaType
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBasicDynamicLearningh */


