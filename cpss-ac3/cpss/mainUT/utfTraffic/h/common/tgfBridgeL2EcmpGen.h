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
* @file tgfBridgeL2EcmpGen.h
*
* @brief Generic API for Bridge L2 ECMP
*
* @version   5
********************************************************************************
*/
#ifndef __tgfBridgeL2EcmpGenh
#define __tgfBridgeL2EcmpGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @struct PRV_TGF_BRG_L2_ECMP_LTT_ENTRY_STC
 *
 * @brief L2 ECMP LTT Entry
*/
typedef struct{

    /** Determines the start index of the L2 ECMP block. */
    GT_U32 ecmpStartIndex;

    /** @brief The number of paths in the ECMP block: (1
     *  0x1 = 1 path, 0x2 = 2 paths and so on.
     */
    GT_U32 ecmpNumOfPaths;

    /** Determines whether this packet is load balanced over an ECMP group. */
    GT_BOOL ecmpEnable;

} PRV_TGF_BRG_L2_ECMP_LTT_ENTRY_STC;

/**
* @struct PRV_TGF_BRG_L2_ECMP_ENTRY_STC
 *
 * @brief L2 ECMP Entry
*/
typedef struct{

    IN GT_PORT_NUM                          targetEport;

    IN GT_HW_DEV_NUM                        targetHwDevice;

} PRV_TGF_BRG_L2_ECMP_ENTRY_STC;

/**
* @internal prvTgfBrgL2EcmpEnableSet function
* @endinternal
*
* @brief   Globally enable/disable L2 ECMP (AKA ePort ECMP)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - L2 ECMP status:
*                                      GT_TRUE: enable L2 ECMP
*                                      GT_FALSE: disable L2 ECMP
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgL2EcmpEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);

/**
* @internal prvTgfBrgL2EcmpEnableGet function
* @endinternal
*
* @brief   Get the global enable/disable L2 ECMP (AKA ePort ECMP)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) L2 ECMP  status:
*                                      GT_TRUE : L2 ECMP is enabled
*                                      GT_FALSE: L2 ECMP is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgL2EcmpEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);


/**
* @internal prvTgfBrgL2EcmpIndexBaseEportSet function
* @endinternal
*
* @brief   Defines the first ePort number in the L2 ECMP ePort number range.
*         The index to the L2 ECMP LTT is <Target ePort>-<Base ePort>.
* @param[in] devNum                   - device number
* @param[in] ecmpIndexBaseEport       - the ePort number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgL2EcmpIndexBaseEportSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      ecmpIndexBaseEport
);

/**
* @internal prvTgfBrgL2EcmpIndexBaseEportGet function
* @endinternal
*
* @brief   Return the first ePort number in the L2 ECMP ePort number range.
*         The index to the L2 ECMP LTT is <Target ePort>-<Base ePort>.
* @param[in] devNum                   - device number
*
* @param[out] ecmpIndexBaseEportPtr    - (pointer to) the ePort number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgL2EcmpIndexBaseEportGet
(
    IN  GT_U8           devNum,
    OUT GT_PORT_NUM     *ecmpIndexBaseEportPtr
);

/**
* @internal prvTgfBrgL2EcmpLttTableSet function
* @endinternal
*
* @brief   Set L2 ECMP LTT entry info.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the LTT index
* @param[in] ecmpLttInfoPtr           - (pointer to) L2 ECMP LTT Information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The index to the LTT is the result of InDesc<TRGePort>-<Base ePort>.
*
*/
GT_STATUS prvTgfBrgL2EcmpLttTableSet
(
    IN GT_U8                                devNum,
    IN GT_U32                               index,
    IN PRV_TGF_BRG_L2_ECMP_LTT_ENTRY_STC   *ecmpLttInfoPtr
);

/**
* @internal prvTgfBrgL2EcmpLttTableGet function
* @endinternal
*
* @brief   Get L2 ECMP LTT entry info.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the LTT index
*
* @param[out] ecmpLttInfoPtr           - (pointer to) L2 ECMP LTT Information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The index to the LTT is the result of InDesc<TRGePort>-<Base ePort>.
*
*/
GT_STATUS prvTgfBrgL2EcmpLttTableGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               index,
    OUT PRV_TGF_BRG_L2_ECMP_LTT_ENTRY_STC   *ecmpLttInfoPtr
);

/**
* @internal prvTgfBrgL2EcmpTableSet function
* @endinternal
*
* @brief   Set L2 ECMP entry info.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the ECMP entry index
* @param[in] ecmpEntryPtr             - (pointer to) L2 ECMP entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Index according to index from L2 ECMP LTT
*
*/
GT_STATUS prvTgfBrgL2EcmpTableSet
(
    IN GT_U8                                devNum,
    IN GT_U32                               index,
    IN PRV_TGF_BRG_L2_ECMP_ENTRY_STC       *ecmpEntryPtr
);

/**
* @internal prvTgfBrgL2EcmpTableGet function
* @endinternal
*
* @brief   Get L2 ECMP entry info.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the ECMP entry index
*
* @param[out] ecmpEntryPtr             - (pointer to) L2 ECMP entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Index according to index from L2 ECMP LTT
*
*/
GT_STATUS prvTgfBrgL2EcmpTableGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               index,
    OUT PRV_TGF_BRG_L2_ECMP_ENTRY_STC       *ecmpEntryPtr
);

/**
* @internal prvTgfBrgL2EcmpMemberSelectionModeSet function
* @endinternal
*
* @brief   Set mode used to calculate the Index of the secondary ePort (L2 ECMP member)
*
* @param[in] devNum                   - device number
* @param[in] selectionMode            - member selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgL2EcmpMemberSelectionModeSet
(
    IN GT_U8                                    devNum,
    IN PRV_TGF_TRUNK_MEMBER_SELECTION_MODE_ENT  selectionMode
);

/**
* @internal prvTgfBrgL2EcmpMemberSelectionModeGet function
* @endinternal
*
* @brief   Get mode used to calculate the Index of the secondary ePort (L2 ECMP member)
*
* @param[in] devNum                   - device number
*
* @param[out] selectionModePtr         - (pointer to) member selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgL2EcmpMemberSelectionModeGet
(
    IN  GT_U8                                   devNum,
    OUT PRV_TGF_TRUNK_MEMBER_SELECTION_MODE_ENT *selectionModePtr
);

/**
* @internal prvTgfBrgL2EcmpAndLttEntriesClear function
* @endinternal
*
* @brief   Clear ECMP MLL & LTT entries.
*
* @param[in] lttIndexArr              - Array of LTT indexes to clear.
* @param[in] numOfLttIndexes          - Number of valid entries in <lttIndexArr>.
* @param[in] ecmpIndexArr             - Array of ECMP indexes to clear.
* @param[in] numOfEcmpIndexes         - Number of valid entries in <ecmpIndexArr>.
*                                       None
*/
GT_VOID prvTgfBrgL2EcmpAndLttEntriesClear
(
    IN GT_U32 *lttIndexArr,
    IN GT_U32  numOfLttIndexes,
    IN GT_U32 *ecmpIndexArr,
    IN GT_U32  numOfEcmpIndexes
);

/**
* @internal prvTgfBrgL2EcmpEntryClear function
* @endinternal
*
* @brief   Clear ECMP MLL & LTT entries.
*
* @param[in] lttIndex                 - LTT indexes to clear.
* @param[in] resetMemebers            - indication to clear all the members that are pointed
*                                      from the LTT in the ECMP table.
*                                      GT_TRUE  - clear members too.
*                                      GT_FALSE - not clear members (only LTT entry).
*                                       None
*/
GT_VOID prvTgfBrgL2EcmpEntryClear
(
    IN GT_U32  lttIndex,
    IN GT_BOOL resetMemebers
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfBridgeL2EcmpGenh */


