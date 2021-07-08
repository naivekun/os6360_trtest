/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file prvCpssDxChTcam.h
*
* @brief Common private TCAM declarations.
*
* @version   2
********************************************************************************
*/
#ifndef __prvCpssDxChTcamh
#define __prvCpssDxChTcamh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>

/**
* @internal prvCpssDxChTcamRuleIndexToEntryNumber function
* @endinternal
*
* @brief   Converts TCAM rule global index to entry number.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] index                    - global line  in TCAM to write to.
*
* @param[out] entryNumberXPtr          - (pointer to) entry number in TCAM for X format
* @param[out] entryNumberYPtr          - (pointer to) entry number in TCAM for Y format
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTcamRuleIndexToEntryNumber
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    OUT GT_U32                              *entryNumberXPtr,
    OUT GT_U32                              *entryNumberYPtr
);

/**
* @internal prvCpssDxChTcamRuleIndexToSip5HwEntryNumber function
* @endinternal
*
* @brief   Converts TCAM rule global index to HW X entry number.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] index                    - global line  in TCAM. Index may be more then
*                                      real TCAM size.
*
* @param[out] entryNumberXPtr          - (pointer to) entry number in TCAM for X format
*
* @retval GT_OK                    - on success.
*/
GT_STATUS prvCpssDxChTcamRuleIndexToSip5HwEntryNumber
(
    IN  GT_U32                              index,
    OUT GT_U32                              *entryNumberXPtr
);

/**
* @internal prvCpssDxChTcamSip5HwEntryToRuleIndexConvert function
* @endinternal
*
* @brief   Converts SIP5 TCAM HW entry index to global rule index.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] hwIndex                  - entry HW number in TCAM
*
* @param[out] ruleIndexPtr             - (pointer to) global rule index in TCAM.
*
* @retval GT_OK                    - on success.
*/
GT_STATUS prvCpssDxChTcamSip5HwEntryToRuleIndexConvert
(
    IN  GT_U32                              hwIndex,
    OUT GT_U32                              *ruleIndexPtr
);

/**
* @internal prvCpssDxChTcamRuleRangeToHwSpaceConvert function
* @endinternal
*
* @brief   Converts range of TCAM rules to HW space range.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] firstRuleIndex           - global first rule index in TCAM.
* @param[in] numOfRules               - number of TCAM rules
*
* @param[out] firstHwEntryIndexPtr     - (pointer to) first HW entry number in TCAM for X format
* @param[out] hwEntriesNumberPtr       - (pointer to) number of HW entries. HW Entries number
*                                      may be more then TCAM real size. The caller function
*                                      must to take care of index wraparound.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTcamRuleRangeToHwSpaceConvert
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              firstRuleIndex,
    IN  GT_U32                              numOfRules,
    OUT GT_U32                              *firstHwEntryIndexPtr,
    OUT GT_U32                              *hwEntriesNumberPtr
);

/**
* @internal prvCpssDxChTcamRuleIndexCheck function
* @endinternal
*
* @brief   Checks if an gloabl TCAM index is valid for a TCAM rule's size.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] ruleSize                 - size of rule
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTcamRuleIndexCheck
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    CPSS_DXCH_TCAM_RULE_SIZE_ENT            ruleSize
);

/**
* @internal prvCpssDxChTcamTtiRuleBaseOffsetGet function
* @endinternal
*
* @brief   Gets TCAM base index for client TTI
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*                                       The offset to add to the TTI rule index in TCAM
*/
GT_U32 prvCpssDxChTcamTtiRuleBaseOffsetGet
(
    IN     GT_U8               devNum
);

/**
* @internal prvCpssDxChTcamPortGroupRuleReadEntry function
* @endinternal
*
* @brief   Reads the TCAM pattern/mask of single bank.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] index                    - global line  in TCAM to write to.
*
* @param[out] validPtr                 - (pointer to) whether TCAM entry is valid or not
* @param[out] ruleSizePtr              - (pointer to) size of rule
* @param[out] tcamEntryPatternPtr      - points to the TCAM rule's pattern.
* @param[out] tcamEntryMaskPtr         - points to the TCAM rule's mask.
*                                      The rule mask is "AND STYLED ONE".
*                                      Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_STATE             - if in TCAM found rule of size different from the specified
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTcamPortGroupRuleReadEntry
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    OUT GT_BOOL                             *validPtr,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT        *ruleSizePtr,
    OUT GT_U32                              *tcamEntryPatternPtr,
    OUT GT_U32                              *tcamEntryMaskPtr
);

/**
* @internal prvCpssDxChTcamPortGroupRuleWriteEntry function
* @endinternal
*
* @brief   Writes the TCAM mask and key of a single bank.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
* @param[in] valid                    port group(s). If a bit of non  port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] valid                    - whether TCAM entry is  or not
* @param[in] ruleSize                 - size of rule
*                                      tcamEmtryPatternPtr - points to the TCAM rule's pattern.
* @param[in] tcamEntryMaskPtr         - points to the TCAM rule's mask.
*                                      The rule mask is "AND STYLED ONE".
*                                      Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTcamPortGroupRuleWriteEntry
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize,
    IN  GT_U32                              *tcamEntryPatternPtr,
    IN  GT_U32                              *tcamEntryMaskPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChTcamh */

