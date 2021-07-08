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
* @file cpssDxChTcam.c
*
* @brief CPSS TCAM implementation.
*
* @version   11
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/private/routerTunnelTermTcam/prvCpssDxChRouterTunnelTermTcam.h>
#include <cpss/dxCh/dxChxGen/tcam/private/prvCpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/tcam/private/prvCpssDxChTcamLog.h>

/* there are 3K entries in floor
   NOTE: currently this number is not 'per device'
*/
#define NUM_RULES_IN_FLOOR_MAC(_devNum)     CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS

/* macro to roundup a number that is divided so :
    for example :
    3 / 2 is round up to 2 and not round down to 1
*/
#define ROUNDUP_DIV_MAC(_number , _divider)             \
    (((_number) + ((_divider)-1)) / (_divider))

/* get the number of floors needed for the number of rules in the tcam */
#define CPSS_DXCH_TCAM_MAX_NUM_FLOORS_MAC(_devNum)                          \
    ROUNDUP_DIV_MAC(                                                        \
        PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.tunnelTerm,     \
        NUM_RULES_IN_FLOOR_MAC(_devNum))

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
)
{
    GT_U32  floorNum;           /* floor number in TCAM         */
    GT_U32  bankNum;            /* bank number in TCAM          */
    GT_U32  lineIndex;          /* index in bank                */
    GT_U32  mod;
    GT_U32  entryNumber;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_TCAM_CHECK_LINE_INDEX_MAC(devNum,index);
    CPSS_NULL_PTR_CHECK_MAC(entryNumberXPtr);
    CPSS_NULL_PTR_CHECK_MAC(entryNumberYPtr);

    floorNum = (GT_U32)(index / CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS);
    mod = (GT_U32)(index % CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS);
    lineIndex = (GT_U32)(mod / CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS);
    bankNum = (GT_U32)(mod % CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS);

    /* Entry Number = {floor_num[3:0], bank_index[7:0], bank_num[3:0]} */
    entryNumber = ((floorNum & 0xF) << 12) | ((lineIndex & 0xFF) << 4) | (bankNum & 0xF);

    /* Entry Number X = {floor_num[3:0], bank_index[7:0], bank_num[3:0], 0} */
    *entryNumberXPtr  =  entryNumber << 1;

    /* Entry Number Y = {floor_num[3:0], bank_index[7:0], bank_num[3:0], 1} */
    *entryNumberYPtr  =  (entryNumber << 1) | 1;

    return GT_OK;
}

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
)
{
    GT_U32  floorNum;           /* floor number in TCAM         */
    GT_U32  bankNum;            /* bank number in TCAM          */
    GT_U32  lineIndex;          /* index in bank                */
    GT_U32  mod;
    GT_U32  entryNumber;

    floorNum = (GT_U32)(index / CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS);
    mod = (GT_U32)(index % CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS);
    lineIndex = (GT_U32)(mod / CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS);
    bankNum = (GT_U32)(mod % CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS);

    /* Entry Number = {floor_num[3:0], bank_index[7:0], bank_num[3:0]} */
    entryNumber = ((floorNum) << 12) | ((lineIndex & 0xFF) << 4) | (bankNum & 0xF);

    /* Entry Number X = {floor_num[3:0], bank_index[7:0], bank_num[3:0], 0} */
    *entryNumberXPtr  =  entryNumber << 1;

    return GT_OK;
}

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
)
{
    GT_U32      tcamBank;  /* TCAM bank number */
    GT_U32      tcamRow;   /* TCAM row number */
    GT_U32      tcamFloor; /* TCAM floor number */

    /* HW Entry Number = {floor_num[3:0], array_addr[7:0], bank[3:0], XY} */
    tcamBank = ((hwIndex >> 1) & 0xF);
    tcamRow =  ((hwIndex >> 5) & 0xFF);
    tcamFloor = hwIndex >> 13;

    *ruleIndexPtr = CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS * tcamFloor +
                    CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS * tcamRow + tcamBank;

    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    GT_U32    nextHwEntryIndex; /* index of next after range HW entry */
    GT_U32    tmp; /* TMP variable */

    /* each logical entry has two HW (one X one Y). Some holes are in HW range also.
      HW Entry Number = {floor_num[3:0], array_addr[7:0], bank[3:0], XY},
      where the banks 12-15 at each floor do not exists. */

    /* convert to X HW entry index */
    rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum, firstRuleIndex, firstHwEntryIndexPtr, &tmp);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* use special function without rule index check to calculate number of HW X/Y entires */
    rc = prvCpssDxChTcamRuleIndexToSip5HwEntryNumber(numOfRules + firstRuleIndex, &nextHwEntryIndex);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* HW Entries number may be more then TCAM real size.
       the caller function must to take care of index wraparound. */
    *hwEntriesNumberPtr = nextHwEntryIndex - (*firstHwEntryIndexPtr);
    return GT_OK;
}

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
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT            ruleSize
)
{
    GT_U32 ruleColumn;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_TCAM_CHECK_LINE_INDEX_MAC(devNum,index);

    ruleColumn = (GT_U32)(index % CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS);

    switch (ruleSize)
    {
    case CPSS_DXCH_TCAM_RULE_SIZE_10_B_E:
        /* do nothing - rule can start in each bank */
        break;
    case CPSS_DXCH_TCAM_RULE_SIZE_20_B_E:
        /* ruleColumn must divide by 2 */
        if ((ruleColumn % 2) != 0)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "index[%d] , ruleSize[20B] , ruleColumn[%d] must divide by 2",index,ruleColumn);
        break;
    case CPSS_DXCH_TCAM_RULE_SIZE_30_B_E:
        /* ruleColumn must divide by 3 */
        if ((ruleColumn % 3) != 0)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "index[%d] , ruleSize[30B] , ruleColumn[%d] must divide by 3",index,ruleColumn);
        break;
    case CPSS_DXCH_TCAM_RULE_SIZE_40_B_E:
    case CPSS_DXCH_TCAM_RULE_SIZE_50_B_E:
    case CPSS_DXCH_TCAM_RULE_SIZE_60_B_E:
        /* ruleColumn must divide by 6 */
        if ((ruleColumn % 6) != 0)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "index[%d] , ruleSize[40B/50B/60B] , ruleColumn[%d] must divide by 6",index,ruleColumn);
        break;
    case CPSS_DXCH_TCAM_RULE_SIZE_80_B_E:
        /* ruleColumn must divide by 12 */
        if ((ruleColumn % 12) != 0)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "index[%d] , ruleSize[80B] , ruleColumn[%d] must divide by 12",index,ruleColumn);
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

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
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
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
)
{
    GT_STATUS rc;               /* function return code         */
    GT_U32  portGroupId;        /* the port group Id            */
    GT_U32  entryKey[3];        /* entry to write (key)         */
    GT_U32  entryMask[3];       /* entry to write (mask)        */
    GT_U32  entryDataX[3];      /* entry to write (dataX)       */
    GT_U32  entryDataY[3];      /* entry to write (dataY)       */
    GT_U32  ii;                 /* iteration index              */
    GT_U32  numOfBanks;         /* number of banks to write     */
    GT_U32  keySize;            /* key size values              */
    GT_U32  entryNumberX;       /* index in hw for X format     */
    GT_U32  entryNumberY;       /* index in hw for Y format     */
    GT_U32  value;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
    PRV_CPSS_DXCH_TCAM_CHECK_LINE_INDEX_MAC(devNum,index);
    CPSS_NULL_PTR_CHECK_MAC(validPtr);
    CPSS_NULL_PTR_CHECK_MAC(ruleSizePtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamEntryPatternPtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamEntryMaskPtr);

    rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum,index,&entryNumberX,&entryNumberY);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* clear data */
    cpssOsMemSet(&entryKey , 0, sizeof(entryKey));
    cpssOsMemSet(&entryMask , 0, sizeof(entryMask));
    cpssOsMemSet(&entryDataX , 0, sizeof(entryDataX));
    cpssOsMemSet(&entryDataY , 0, sizeof(entryDataY));

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    /* read first bank to get the rule's length */
    rc =  prvCpssDxChPortGroupReadTableEntry(devNum,
                                             portGroupId,
                                             CPSS_DXCH_SIP5_TABLE_TCAM_E,
                                             entryNumberX,
                                             entryDataX);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc =  prvCpssDxChPortGroupReadTableEntry(devNum,
                                             portGroupId,
                                             CPSS_DXCH_SIP5_TABLE_TCAM_E,
                                             entryNumberY,
                                             entryDataY);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* first bank size is used as validity for the whole rule.
       when X=0 and Y=1 results--> Match 0,
       when X=1 and Y=1 results--> Always Miss. */
    U32_GET_FIELD_IN_ENTRY_MAC(entryDataX,3,1,value);
    *validPtr = !(BIT2BOOL_MAC(value));

    /* convert data from X/Y format to pattern/mask */
    for (ii = 0 ; ii <= 2 ; ii++)
    {
        entryKey[ii]  = PRV_BIT_X_Y_TO_PATTERN_MAC(entryDataX[ii],entryDataY[ii]);
        entryMask[ii] = PRV_BIT_X_Y_TO_MASK_MAC(entryDataX[ii],entryDataY[ii]);
    }

    U32_GET_FIELD_IN_ENTRY_MAC(entryKey,0,3,keySize);
    if (keySize == 7)
        numOfBanks = 8;
    else
        numOfBanks = keySize + 1;
    PRV_CPSS_DXCH_TCAM_CONVERT_NUM_OF_BANKS_TO_RULE_SIZE_VAL_MAC(*ruleSizePtr,numOfBanks);

    rc = prvCpssDxChTcamRuleIndexCheck(devNum,index,*ruleSizePtr);
    if(rc != GT_OK)
    {
        *validPtr = GT_FALSE;
        return GT_OK;
    }

    /* copy first 64 bits of pattern/mask */
    for(ii = 0 ; ii < 2 ; ii++)
    {
        U32_GET_FIELD_IN_ENTRY_MAC(entryKey,4+(32*ii),32,value);
        U32_SET_FIELD_IN_ENTRY_MAC(tcamEntryPatternPtr,32*ii,32,value);
        U32_GET_FIELD_IN_ENTRY_MAC(entryMask,4+(32*ii),32,value);
        U32_SET_FIELD_IN_ENTRY_MAC(tcamEntryMaskPtr,32*ii,32,value);
    }
    ii = 2;
    /* copy next 16 bits of pattern/mask (total of 80 bits)*/
    U32_GET_FIELD_IN_ENTRY_MAC(entryKey,4+(32*ii),16,value);
    U32_SET_FIELD_IN_ENTRY_MAC(tcamEntryPatternPtr,32*ii,16,value);
    U32_GET_FIELD_IN_ENTRY_MAC(entryMask,4+(32*ii),16,value);
    U32_SET_FIELD_IN_ENTRY_MAC(tcamEntryMaskPtr,32*ii,16,value);

    return GT_OK;
}

/**
* @internal prvCpssDxChTcamPortGroupRuleWriteSingleEntry function
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
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] valid                    - whether TCAM entry is  or not
* @param[in] ruleSize                 - size of rule
* @param[in] tcamEntryPatternPtr      - points to the TCAM rule's pattern.
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
static GT_STATUS prvCpssDxChTcamPortGroupRuleWriteSingleEntry
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize,
    IN  GT_U32                              *tcamEntryPatternPtr,
    IN  GT_U32                              *tcamEntryMaskPtr
)
{
    GT_STATUS rc;               /* function return code         */
    GT_U32  portGroupId;        /* the port group Id            */
    GT_U32  entryKey[3];        /* entry to write (key)         */
    GT_U32  entryMask[3];       /* entry to write (mask)        */
    GT_U32  entryDataX[3];      /* entry to write (dataX)       */
    GT_U32  entryDataY[3];      /* entry to write (dataY)       */
    GT_U32  ii;                 /* iteration index              */
    GT_U32  keySize;            /* key size values              */
    GT_U32  entryNumberX;       /* index in hw for X format     */
    GT_U32  entryNumberY;       /* index in hw for Y format     */
    GT_U32  value;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
    CPSS_NULL_PTR_CHECK_MAC(tcamEntryPatternPtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamEntryMaskPtr);

    rc = prvCpssDxChTcamRuleIndexCheck(devNum,index,ruleSize);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* clear data */
    cpssOsMemSet(&entryKey , 0, sizeof(entryKey));
    cpssOsMemSet(&entryMask , 0, sizeof(entryMask));
    cpssOsMemSet(&entryDataX , 0, sizeof(entryDataX));
    cpssOsMemSet(&entryDataY , 0, sizeof(entryDataY));

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    PRV_CPSS_DXCH_TCAM_CONVERT_RULE_SIZE_TO_KEY_SIZE_VAL_MAC(keySize,ruleSize);

    rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum,index,&entryNumberX,&entryNumberY);
    if(rc != GT_OK)
    {
        return rc;
    }

    U32_SET_FIELD_IN_ENTRY_MAC(entryKey,0,3,keySize);
    U32_SET_FIELD_IN_ENTRY_MAC(entryMask,0,3,7);

    /* copy first 64 bits of pattern/mask */
    for(ii = 0 ; ii < 2 ; ii++)
    {
        U32_GET_FIELD_IN_ENTRY_MAC(tcamEntryPatternPtr,(32*ii),32,value);
        U32_SET_FIELD_IN_ENTRY_MAC(entryKey,4+(32*ii),32,value);
        U32_GET_FIELD_IN_ENTRY_MAC(tcamEntryMaskPtr,(32*ii),32,value);
        U32_SET_FIELD_IN_ENTRY_MAC(entryMask,4+(32*ii),32,value);
    }
    ii = 2;
    /* copy next 16 bits of pattern/mask (total of 80 bits)*/
    U32_GET_FIELD_IN_ENTRY_MAC(tcamEntryPatternPtr,(32*ii),16,value);
    U32_SET_FIELD_IN_ENTRY_MAC(entryKey,4+(32*ii),16,value);
    U32_GET_FIELD_IN_ENTRY_MAC(tcamEntryMaskPtr,(32*ii),16,value);
    U32_SET_FIELD_IN_ENTRY_MAC(entryMask,4+(32*ii),16,value);

    /* convert pattern/mask to X/Y format */
    for (ii = 0; ii <= 2; ii++)
    {
        entryDataX[ii] = PRV_BIT_PATTERN_MASK_TO_X_MAC(entryKey[ii], entryMask[ii]);
        entryDataY[ii] = PRV_BIT_PATTERN_MASK_TO_Y_MAC(entryKey[ii], entryMask[ii]);
    }

    /* first bank size is used as validity for the whole rule.
       when X=0 and Y=1 results--> Match 0,
       when X=1 and Y=1 results--> Always Miss. */
    U32_SET_FIELD_IN_ENTRY_MAC(entryDataX,3,1,(1-BOOL2BIT_MAC(valid)));
    U32_SET_FIELD_IN_ENTRY_MAC(entryDataY,3,1,1); /* Y=1*/

    /* The bit#3 is used to manage validity of TCAM entry.
       The Y entry has constant bit #3 that is always 1. The content of X entry
       bit #3 defines if entry valid or not. The X entry must be written after Y
       entry to ensure consistency of the whole X/Y entry.  */
    rc =  prvCpssDxChPortGroupWriteTableEntry(devNum,
                                             portGroupId,
                                             CPSS_DXCH_SIP5_TABLE_TCAM_E,
                                             entryNumberY,
                                             entryDataY);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc =  prvCpssDxChPortGroupWriteTableEntry(devNum,
                                             portGroupId,
                                             CPSS_DXCH_SIP5_TABLE_TCAM_E,
                                             entryNumberX,
                                             entryDataX);
    return rc;
}

/**
* @internal prvCpssDxChTcamWriteDaemonWaWait function
* @endinternal
*
* @brief   Wait some time for daemon disable/enable WA.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] waitCount                - number of register reads count for wait operation
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChTcamWriteDaemonWaWait
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              waitCount
)
{
    GT_U32      regAddr;  /* register address  */
    GT_U32      regData;  /* register data     */
    GT_U32      ii;       /* iterator          */
    GT_STATUS   rc;

    /* use readable register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.tcamInterrupts.TCAMInterruptsSummaryMask;

    /* read register several times to make delay */
    for (ii = 0; ii < waitCount; ii++)
    {
        /*  read register */
        rc = prvCpssHwPpReadRegister (devNum,regAddr,&regData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/* Number of register read operation to wait completion of all internal
   processes in packet processor before enable parity daemon.
   The WA requires 120 PP core clocks delay.
   The read register procedure takes at least 114
   (number was get by tests on BC2 device) Core Clocks for code run in internal
   CPU. Two read register procedures guaranties 120 PP core clock delay.
   The WA algorithm uses prvCpssHwPpSetRegField function that already includes
   one read. So need only
   (2 - 1) = 1 additional read operations. */
static GT_U32 prvTcamWaEnableCount = 1;

/* Number of register read operation to wait completion of all internal
   processes in packet processor after disable parity daemon and before write
   TCAM entry. The delay is calculated same as above. */
static GT_U32 prvTcamWaDisableCount = 1;

/**
* @internal debug_cpssDxChTcamWaWaitCountSet function
* @endinternal
*
* @brief   Debug function to set WA wait counts.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] waEnableCount            - count for enable daemon wait
* @param[in] waDisableCount           - count for disable daemon wait
*                                       none
*/
GT_VOID debug_cpssDxChTcamWaWaitCountSet
(
    IN  GT_U32 waEnableCount,
    IN  GT_U32 waDisableCount
)
{
    prvTcamWaEnableCount = waEnableCount;
    prvTcamWaDisableCount = waDisableCount;
    return;
}

/* count for enable daemon wait operation in parity daemon WA */
#define PRV_CPSS_DXCH_TCAM_WA_ENABLE_COUNT_CNS prvTcamWaEnableCount

/* count for disable daemon wait operation in parity daemon WA */
#define PRV_CPSS_DXCH_TCAM_WA_DISABLE_COUNT_CNS prvTcamWaDisableCount

/**
* @internal prvCpssDxChTcamWriteDaemonWaExecute function
* @endinternal
*
* @brief   Execute disable/enable parity daemon WA for TCAM write operation.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] disableDaemon            - 1 - disable daemon
*                                      0 - enable daemon
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChTcamWriteDaemonWaExecute
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              disableDaemon
)
{
    GT_U32      regAddr;  /* register address  */
    GT_STATUS   rc;

    if (disableDaemon == 0)
    {
        /* wait some time to guranty that TCAM write updated parity DB also
           before enable daemon. */
        rc = prvCpssDxChTcamWriteDaemonWaWait(devNum, PRV_CPSS_DXCH_TCAM_WA_ENABLE_COUNT_CNS);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.parityDaemonRegs.parityDaemonCtrl;

    /*  write to daemon control bit: 0 - enable, 1 - disable */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, disableDaemon);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (disableDaemon)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.tcamGlobalConfig;

        /*  increase daemon's priority  */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 2, 1, 1);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* wait some time to guranty that all daemon request were executed by TCAM */
        rc = prvCpssDxChTcamWriteDaemonWaWait(devNum, PRV_CPSS_DXCH_TCAM_WA_DISABLE_COUNT_CNS);
        if (rc != GT_OK)
        {
            return rc;
        }

        /*  restore daemon's priority  */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 2, 1, 0);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

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
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] valid                    - whether TCAM entry is  or not
* @param[in] ruleSize                 - size of rule
* @param[in] tcamEntryPatternPtr      - points to the TCAM rule's pattern.
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
)
{
    GT_STATUS rc;               /* function return code         */
    GT_STATUS rc1;              /* function return code         */
    GT_BOOL   doWriteParityWa;  /* execute write TCAM WA        */

    doWriteParityWa = PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                            PRV_CPSS_DXCH_BOBCAT2_TCAM_WRITE_PARITY_ERROR_WA_E);

    if (doWriteParityWa)
    {
        /* disable parity daemon before write to TCAM */
        rc = prvCpssDxChTcamWriteDaemonWaExecute(devNum, 1);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = prvCpssDxChTcamPortGroupRuleWriteSingleEntry(devNum, portGroupsBmp,
                                                      index, valid, ruleSize,
                                                      tcamEntryPatternPtr,
                                                      tcamEntryMaskPtr);

    if (doWriteParityWa)
    {
        /* enable parity daemon after write to TCAM */
        rc1 = prvCpssDxChTcamWriteDaemonWaExecute(devNum, 0);
        if (rc1 != GT_OK)
        {
            /* return rc if it's not OK */
            return (rc != GT_OK)? rc : rc1;
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChTcamPortGroupRuleWrite function
* @endinternal
*
* @brief   Writes the TCAM mask and key. Writing operation is preformed on all
*         bits in the line.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] valid                    - whether TCAM entry is  or not
* @param[in] ruleSize                 - size of rule
* @param[in] tcamPatternPtr           - points to the TCAM rule's pattern.
* @param[in] tcamMaskPtr              - points to the TCAM rule's mask.
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
static GT_STATUS prvCpssDxChTcamPortGroupRuleWrite
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize,
    IN  GT_U32                              *tcamPatternPtr,
    IN  GT_U32                              *tcamMaskPtr
)
{
    GT_STATUS rc;               /* function return code         */
    GT_U32  portGroupId;        /* the port group Id            */
    GT_U32  entryKey[3];        /* entry to write (key)         */
    GT_U32  entryMask[3];       /* entry to write (mask)        */
    GT_U32  entryDataX[3];      /* entry to write (dataX)       */
    GT_U32  entryDataY[3];      /* entry to write (dataY)       */
    GT_U32  bank;               /* current bank written         */
    GT_U32  ii;                 /* iteration index              */
    GT_U32  numOfBanks;         /* number of banks to write     */
    GT_U32  keySize;            /* key size values              */
    GT_U32  entryNumberX;       /* index in hw for X format     */
    GT_U32  entryNumberY;       /* index in hw for Y format     */
    GT_U32  bankIdx;            /* bank iterator                */
    GT_U32  value;
    GT_U32  startBit;

    PRV_CPSS_DXCH_TCAM_CONVERT_RULE_SIZE_TO_KEY_SIZE_VAL_MAC(keySize,ruleSize);
    PRV_CPSS_DXCH_TCAM_CONVERT_RULE_SIZE_TO_NUM_OF_BANKS_VAL_MAC(numOfBanks,ruleSize);

    rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum,index,&entryNumberX,&entryNumberY);
    if(rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        /* bank#0 validates whole entry. Therefore write banks in reverse
           order to ensure that bank#0 will be last. */
        bank = numOfBanks - 1;
        for (bankIdx = 0; bankIdx < numOfBanks; bankIdx++, bank--)
        {
             /* clear data */
            cpssOsMemSet(&entryKey , 0, sizeof(entryKey));
            cpssOsMemSet(&entryMask , 0, sizeof(entryMask));
            cpssOsMemSet(&entryDataX , 0, sizeof(entryDataX));
            cpssOsMemSet(&entryDataY , 0, sizeof(entryDataY));

            startBit = bank * 80;

            U32_SET_FIELD_IN_ENTRY_MAC(entryKey,0,3,keySize);
            U32_SET_FIELD_IN_ENTRY_MAC(entryMask,0,3,7); /* excet match */

            /* copy first 64 bits of pattern/mask */
            for(ii = 0 ; ii < 2 ; ii++)
            {
                U32_GET_FIELD_IN_ENTRY_MAC(tcamPatternPtr,startBit+(32*ii),32,value);
                U32_SET_FIELD_IN_ENTRY_MAC(entryKey,4+(32*ii),32,value);
                U32_GET_FIELD_IN_ENTRY_MAC(tcamMaskPtr,startBit+(32*ii),32,value);
                U32_SET_FIELD_IN_ENTRY_MAC(entryMask,4+(32*ii),32,value);
            }
            ii = 2;
            /* copy next 16 bits of pattern/mask (total of 80 bits)*/
            U32_GET_FIELD_IN_ENTRY_MAC(tcamPatternPtr,startBit+(32*ii),16,value);
            U32_SET_FIELD_IN_ENTRY_MAC(entryKey,4+(32*ii),16,value);
            U32_GET_FIELD_IN_ENTRY_MAC(tcamMaskPtr,startBit+(32*ii),16,value);
            U32_SET_FIELD_IN_ENTRY_MAC(entryMask,4+(32*ii),16,value);

            /* convert pattern/mask to X/Y format */
            for (ii = 0; ii <= 2; ii++)
            {
                entryDataX[ii] = PRV_BIT_PATTERN_MASK_TO_X_MAC(entryKey[ii], entryMask[ii]);
                entryDataY[ii] = PRV_BIT_PATTERN_MASK_TO_Y_MAC(entryKey[ii], entryMask[ii]);
            }

            /* first bank size is used as validity for the whole rule.
               when X=0 and Y=1 results--> Match 0,
               when X=1 and Y=1 results--> Always Miss. */
            if(bank == 0)
            {
                U32_SET_FIELD_IN_ENTRY_MAC(entryDataX,3,1,(1-BOOL2BIT_MAC(valid)));
            }
            else
            {
                U32_SET_FIELD_IN_ENTRY_MAC(entryDataX,3,1,0); /* X=0*/
            }

            U32_SET_FIELD_IN_ENTRY_MAC(entryDataY,3,1,1); /* Y=1*/

            /* The bit#3 is used to manage validity of TCAM entry.
               The Y entry has constant bit #3 that is always 1. The content of X entry
               bit #3 defines if entry valid or not. The X entry must be written after Y
               entry to ensure consistency of the whole X/Y entry.  */
            rc =  prvCpssDxChPortGroupWriteTableEntry(devNum,
                                                     portGroupId,
                                                     CPSS_DXCH_SIP5_TABLE_TCAM_E,
                                                     entryNumberY + (2*bank),
                                                     entryDataY);
            if(rc != GT_OK)
            {
                return rc;
            }

            rc =  prvCpssDxChPortGroupWriteTableEntry(devNum,
                                                     portGroupId,
                                                     CPSS_DXCH_SIP5_TABLE_TCAM_E,
                                                     entryNumberX + (2*bank),
                                                     entryDataX);
            if(rc != GT_OK)
            {
                return rc;
            }
       }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    return GT_OK;
}

/**
* @internal internal_cpssDxChTcamPortGroupRuleWrite function
* @endinternal
*
* @brief   Writes the TCAM mask and key. Writing operation is preformed on all
*         bits in the line.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] valid                    - whether TCAM entry is  or not
* @param[in] ruleSize                 - size of rule
* @param[in] tcamPatternPtr           - points to the TCAM rule's pattern.
* @param[in] tcamMaskPtr              - points to the TCAM rule's mask.
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
static GT_STATUS internal_cpssDxChTcamPortGroupRuleWrite
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize,
    IN  GT_U32                              *tcamPatternPtr,
    IN  GT_U32                              *tcamMaskPtr
)
{
    GT_STATUS rc;               /* function return code         */
    GT_STATUS rc1;              /* function return code         */
    GT_BOOL   doWriteParityWa;  /* execute write TCAM WA        */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
    CPSS_NULL_PTR_CHECK_MAC(tcamPatternPtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamMaskPtr);

    rc = prvCpssDxChTcamRuleIndexCheck(devNum,index,ruleSize);
    if(rc != GT_OK)
    {
        return rc;
    }

    doWriteParityWa = PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                            PRV_CPSS_DXCH_BOBCAT2_TCAM_WRITE_PARITY_ERROR_WA_E);

    if (doWriteParityWa)
    {
        /* disable parity daemon before write to TCAM */
        rc = prvCpssDxChTcamWriteDaemonWaExecute(devNum, 1);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = prvCpssDxChTcamPortGroupRuleWrite(devNum, portGroupsBmp, index, valid,
                                           ruleSize, tcamPatternPtr, tcamMaskPtr);

    if (doWriteParityWa)
    {
        /* enable parity daemon after write to TCAM */
        rc1 = prvCpssDxChTcamWriteDaemonWaExecute(devNum, 0);
        if (rc1 != GT_OK)
        {
            /* return rc if it's not OK */
            return (rc != GT_OK)? rc : rc1;
        }
    }

    return rc;
}

/**
* @internal cpssDxChTcamPortGroupRuleWrite function
* @endinternal
*
* @brief   Writes the TCAM mask and key. Writing operation is preformed on all
*         bits in the line.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] valid                    - whether TCAM entry is  or not
* @param[in] ruleSize                 - size of rule
* @param[in] tcamPatternPtr           - points to the TCAM rule's pattern.
* @param[in] tcamMaskPtr              - points to the TCAM rule's mask.
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
GT_STATUS cpssDxChTcamPortGroupRuleWrite
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize,
    IN  GT_U32                              *tcamPatternPtr,
    IN  GT_U32                              *tcamMaskPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamPortGroupRuleWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, index, valid, ruleSize, tcamPatternPtr, tcamMaskPtr));

    rc = internal_cpssDxChTcamPortGroupRuleWrite(devNum, portGroupsBmp, index, valid, ruleSize, tcamPatternPtr, tcamMaskPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, index, valid, ruleSize, tcamPatternPtr, tcamMaskPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamPortGroupRuleRead function
* @endinternal
*
* @brief   Reads the TCAM mask and key.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
*
* @param[out] validPtr                 - (pointer to) whether TCAM entry is valid or not
* @param[out] ruleSizePtr              - (pointer to) size of rule
* @param[out] tcamPatternPtr           - points to the TCAM rule's pattern.
* @param[out] tcamMaskPtr              - points to the TCAM rule's mask.
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
static GT_STATUS internal_cpssDxChTcamPortGroupRuleRead
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    OUT GT_BOOL                             *validPtr,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT        *ruleSizePtr,
    OUT GT_U32                              *tcamPatternPtr,
    OUT GT_U32                              *tcamMaskPtr
)
{
    GT_STATUS rc;               /* function return code         */
    GT_U32  portGroupId;        /* the port group Id            */
    GT_U32  entryKey[3];        /* entry to write (key)         */
    GT_U32  entryMask[3];       /* entry to write (mask)        */
    GT_U32  entryDataX[3];      /* entry to write (dataX)       */
    GT_U32  entryDataY[3];      /* entry to write (dataY)       */
    GT_U32  bank;               /* current bank written         */
    GT_U32  ii;                 /* iteration index              */
    GT_U32  numOfBanks;         /* number of banks to write     */
    GT_U32  entryNumberX;       /* index in hw for X format     */
    GT_U32  entryNumberY;       /* index in hw for Y format     */
    GT_U32  value;
    GT_U32  startBit;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
    PRV_CPSS_DXCH_TCAM_CHECK_LINE_INDEX_MAC(devNum,index);
    CPSS_NULL_PTR_CHECK_MAC(validPtr);
    CPSS_NULL_PTR_CHECK_MAC(ruleSizePtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamPatternPtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamMaskPtr);

    rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum,index,&entryNumberX,&entryNumberY);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* clear data */
    cpssOsMemSet(&entryKey , 0, sizeof(entryKey));
    cpssOsMemSet(&entryMask , 0, sizeof(entryMask));

    rc = prvCpssDxChTcamPortGroupRuleReadEntry(devNum,portGroupsBmp,index,validPtr,ruleSizePtr,tcamPatternPtr,tcamMaskPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChTcamRuleIndexCheck(devNum,index,*ruleSizePtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_DXCH_TCAM_CONVERT_RULE_SIZE_TO_NUM_OF_BANKS_VAL_MAC(numOfBanks,*ruleSizePtr);


    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        for (bank = 0; bank < numOfBanks; bank++)
        {
            /* clear data */
            cpssOsMemSet(&entryKey , 0, sizeof(entryKey));
            cpssOsMemSet(&entryMask , 0, sizeof(entryMask));
            cpssOsMemSet(&entryDataX , 0, sizeof(entryDataX));
            cpssOsMemSet(&entryDataY , 0, sizeof(entryDataY));

            rc =  prvCpssDxChPortGroupReadTableEntry(devNum,
                                                     portGroupId,
                                                     CPSS_DXCH_SIP5_TABLE_TCAM_E,
                                                     entryNumberX + (bank*2),
                                                     entryDataX);
            if(rc != GT_OK)
            {
                return rc;
            }

            rc =  prvCpssDxChPortGroupReadTableEntry(devNum,
                                                     portGroupId,
                                                     CPSS_DXCH_SIP5_TABLE_TCAM_E,
                                                     entryNumberY + (bank*2),
                                                     entryDataY);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* convert data from X/Y format to pattern/mask */
            for (ii = 0 ; ii <= 2 ; ii++)
            {
                entryKey[ii]  = PRV_BIT_X_Y_TO_PATTERN_MAC(entryDataX[ii],entryDataY[ii]);
                entryMask[ii] = PRV_BIT_X_Y_TO_MASK_MAC(entryDataX[ii],entryDataY[ii]);
            }

            startBit = bank * 80;

            /* copy first 64 bits of pattern/mask */
            for(ii = 0 ; ii < 2 ; ii++)
            {
                U32_GET_FIELD_IN_ENTRY_MAC(entryKey,4+(32*ii),32,value);
                U32_SET_FIELD_IN_ENTRY_MAC(tcamPatternPtr,startBit+(32*ii),32,value);
                U32_GET_FIELD_IN_ENTRY_MAC(entryMask,4+(32*ii),32,value);
                U32_SET_FIELD_IN_ENTRY_MAC(tcamMaskPtr,startBit+(32*ii),32,value);
            }
            ii = 2;
            /* copy next 16 bits of pattern/mask (total of 80 bits)*/
            U32_GET_FIELD_IN_ENTRY_MAC(entryKey,4+(32*ii),16,value);
            U32_SET_FIELD_IN_ENTRY_MAC(tcamPatternPtr,startBit+(32*ii),16,value);
            U32_GET_FIELD_IN_ENTRY_MAC(entryMask,4+(32*ii),16,value);
            U32_SET_FIELD_IN_ENTRY_MAC(tcamMaskPtr,startBit+(32*ii),16,value);
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChTcamPortGroupRuleRead function
* @endinternal
*
* @brief   Reads the TCAM mask and key.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
*
* @param[out] validPtr                 - (pointer to) whether TCAM entry is valid or not
* @param[out] ruleSizePtr              - (pointer to) size of rule
* @param[out] tcamPatternPtr           - points to the TCAM rule's pattern.
* @param[out] tcamMaskPtr              - points to the TCAM rule's mask.
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
GT_STATUS cpssDxChTcamPortGroupRuleRead
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    OUT GT_BOOL                             *validPtr,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT        *ruleSizePtr,
    OUT GT_U32                              *tcamPatternPtr,
    OUT GT_U32                              *tcamMaskPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamPortGroupRuleRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, index, validPtr, ruleSizePtr, tcamPatternPtr, tcamMaskPtr));

    rc = internal_cpssDxChTcamPortGroupRuleRead(devNum, portGroupsBmp, index, validPtr, ruleSizePtr, tcamPatternPtr, tcamMaskPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, index, validPtr, ruleSizePtr, tcamPatternPtr, tcamMaskPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamPortGroupRuleValidStatusSet function
* @endinternal
*
* @brief   Validates/Invalidates the TCAM rule.
*         The validation of the rule is performed by next steps:
*         1. Retrieve the content of the rule from PP TCAM
*         2. Write content back to TCAM with Valid indication set.
*         The function does not check content of the rule before
*         write it back to TCAM
*         The invalidation of the rule is performed by next steps:
*         1. Retrieve the content of the rule from PP TCAM
*         2. Write content back to TCAM with Invalid indication set.
*         If the given the rule found already in needed valid state
*         no write done. If the given the rule found with size
*         different from the given rule-size an error code returned.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] valid                    - whether TCAM entry is  or not
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_STATE             - if in TCAM found rule of size different from the specified
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTcamPortGroupRuleValidStatusSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid
)
{
    GT_U32  portGroupId;        /* the port group Id            */
    GT_STATUS rc;               /* function return code         */
    GT_U32  entryKey[3];        /* entry to write (key)         */
    GT_U32  entryMask[3];       /* entry to write (mask)        */
    GT_BOOL validFound;         /* is rule found valid          */
    CPSS_DXCH_TCAM_RULE_SIZE_ENT  ruleSizeFound; /* rule size found */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);

    PRV_CPSS_DXCH_TCAM_CHECK_LINE_INDEX_MAC(devNum,index);

    /* clear data */
    cpssOsMemSet(&entryKey , 0, sizeof(entryKey));
    cpssOsMemSet(&entryMask , 0, sizeof(entryMask));

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {

        rc = prvCpssDxChTcamPortGroupRuleReadEntry(devNum,1<<portGroupId,index,&validFound,&ruleSizeFound,entryKey,entryMask);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (valid == validFound)
        {
            /* the rule is already in required valid state */
            return GT_OK;
        }

        rc = prvCpssDxChTcamPortGroupRuleWriteEntry(devNum,1<<portGroupId,index,valid,ruleSizeFound,entryKey,entryMask);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChTcamPortGroupRuleValidStatusSet function
* @endinternal
*
* @brief   Validates/Invalidates the TCAM rule.
*         The validation of the rule is performed by next steps:
*         1. Retrieve the content of the rule from PP TCAM
*         2. Write content back to TCAM with Valid indication set.
*         The function does not check content of the rule before
*         write it back to TCAM
*         The invalidation of the rule is performed by next steps:
*         1. Retrieve the content of the rule from PP TCAM
*         2. Write content back to TCAM with Invalid indication set.
*         If the given the rule found already in needed valid state
*         no write done. If the given the rule found with size
*         different from the given rule-size an error code returned.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] valid                    - whether TCAM entry is  or not
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_STATE             - if in TCAM found rule of size different from the specified
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTcamPortGroupRuleValidStatusSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamPortGroupRuleValidStatusSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, index, valid));

    rc = internal_cpssDxChTcamPortGroupRuleValidStatusSet(devNum, portGroupsBmp, index, valid);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, index, valid));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamPortGroupRuleValidStatusGet function
* @endinternal
*
* @brief   Gets validity of the TCAM rule.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
*
* @param[out] validPtr                 - (pointer to) whether TCAM entry is valid or not
* @param[out] ruleSizePtr              - (pointer to) size of rule
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_STATE             - if in TCAM found rule of size different from the specified
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTcamPortGroupRuleValidStatusGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    OUT GT_BOOL                             *validPtr,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT        *ruleSizePtr
)
{
    GT_STATUS rc;               /* function return code         */
    GT_U32  portGroupId;        /* the port group Id            */
    GT_U32  entryKey[3];        /* entry to write (key)         */
    GT_U32  entryMask[3];       /* entry to write (mask)        */
    GT_U32  entryDataX[3];      /* entry to write (dataX)       */
    GT_U32  entryDataY[3];      /* entry to write (dataY)       */
    GT_U32  ii;                 /* iteration index              */
    GT_U32  numOfBanks;         /* number of banks to write     */
    GT_U32  keySize;            /* key size values              */
    GT_U32  entryNumberX;       /* index in hw for X format     */
    GT_U32  entryNumberY;       /* index in hw for Y format     */
    GT_U32  value;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);

    PRV_CPSS_DXCH_TCAM_CHECK_LINE_INDEX_MAC(devNum,index);
    CPSS_NULL_PTR_CHECK_MAC(validPtr);
    CPSS_NULL_PTR_CHECK_MAC(ruleSizePtr);

    rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum,index,&entryNumberX,&entryNumberY);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* clear data */
    cpssOsMemSet(&entryKey , 0, sizeof(entryKey));
    cpssOsMemSet(&entryMask , 0, sizeof(entryMask));
    cpssOsMemSet(&entryDataX , 0, sizeof(entryDataX));
    cpssOsMemSet(&entryDataY , 0, sizeof(entryDataY));

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    /* read first bank to get the rule's length */
    rc =  prvCpssDxChPortGroupReadTableEntry(devNum,
                                             portGroupId,
                                             CPSS_DXCH_SIP5_TABLE_TCAM_E,
                                             entryNumberX,
                                             entryDataX);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc =  prvCpssDxChPortGroupReadTableEntry(devNum,
                                             portGroupId,
                                             CPSS_DXCH_SIP5_TABLE_TCAM_E,
                                             entryNumberY,
                                             entryDataY);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* first bank size is used as validity for the whole rule.
       when X=0 and Y=1 results--> Match 0,
       when X=1 and Y=1 results--> Always Miss. */
    U32_GET_FIELD_IN_ENTRY_MAC(entryDataX,3,1,value);
    *validPtr = !(BIT2BOOL_MAC(value));

    /* convert data from X/Y format to pattern/mask */
    for (ii = 0 ; ii <= 2 ; ii++)
    {
        entryKey[ii]  = PRV_BIT_X_Y_TO_PATTERN_MAC(entryDataX[ii],entryDataY[ii]);
        entryMask[ii] = PRV_BIT_X_Y_TO_MASK_MAC(entryDataX[ii],entryDataY[ii]);
    }

    U32_GET_FIELD_IN_ENTRY_MAC(entryKey,0,3,keySize);
    if (keySize == 7)
        numOfBanks = 8;
    else
        numOfBanks = keySize + 1;
    PRV_CPSS_DXCH_TCAM_CONVERT_NUM_OF_BANKS_TO_RULE_SIZE_VAL_MAC(*ruleSizePtr,numOfBanks);

    rc = prvCpssDxChTcamRuleIndexCheck(devNum,index,*ruleSizePtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChTcamPortGroupRuleValidStatusGet function
* @endinternal
*
* @brief   Gets validity of the TCAM rule.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
*
* @param[out] validPtr                 - (pointer to) whether TCAM entry is valid or not
* @param[out] ruleSizePtr              - (pointer to) size of rule
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_STATE             - if in TCAM found rule of size different from the specified
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTcamPortGroupRuleValidStatusGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    OUT GT_BOOL                             *validPtr,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT        *ruleSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamPortGroupRuleValidStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, index, validPtr, ruleSizePtr));

    rc = internal_cpssDxChTcamPortGroupRuleValidStatusGet(devNum, portGroupsBmp, index, validPtr, ruleSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, index, validPtr, ruleSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/*******************************************************************************
* clientGroupForRepresentativePortGroupSet
*
* DESCRIPTION:
*       Enable/Disable client per TCAM group for Representative Port Group.
*
* APPLICABLE DEVICES:
*        Bobcat2; Caelum; Aldrin; AC3X.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon.
*
*       NOTE: Bobcat3 : The Tcam groups can not be configured as it is 'Hard Wired'
*
* INPUTS:
*       devNum            - the device number
*       portGroupsBmp     - bitmap of Port Groups.
*                        NOTEs:
*                         1. for non multi-port groups device this parameter is IGNORED.
*                         2. for multi-port groups device :
*                         (APPLICABLE DEVICES Falcon)
*                         Bitmap must be set with at least one bit representing
*                         valid port group(s). If a bit of non valid port group
*                         is set then function returns GT_BAD_PARAM.
*                         value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                         3. see general note about portGroupsBmp in TCAM APIs for Falcon
*       hwClientId        - HW index for tcamClient.
*       tcamGroup         - TCAM group id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Falcon: 0..3)
*       representativePortGroupId - the representative port group Id (0,2,4,6).
*                               every 2 pipes share a TCAM
*       enable            - GT_TRUE: TCAM client is enabled
*                           GT_FALSE: TCAM client is disabled
*
* OUTPUTS:
*       none.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on bad parameter.
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_OUT_OF_RANGE          - parameter not in valid range.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
/*static*/ GT_STATUS clientGroupForRepresentativePortGroupSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              hwClientId,
    IN  GT_U32                              tcamGroup,
    IN  GT_U32                              representativePortGroupId,
    IN  GT_BOOL                             enable
)
{
    GT_U32    ii;
    GT_STATUS rc;
    GT_U32    value;
    GT_U32    regAddr;          /* register address          */
    GT_U32    currentValueMask;
    GT_U32    currentPortGroupsBmp;
    GT_U32    numClientIds;     /* number of TCAM clients */
    GT_U32    valueMask;        /* support 2 pipes (Falcon) */
    GT_U32    numOfPipeOnTcam = 2;
    GT_U32    numOfClientGroups = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.maxClientGroups;

    if(representativePortGroupId & 1)/* every 2 pipes share a TCAM */
    {
        /* only representatives port groups (0,2,4,6) supported */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* set the mask to proper client */
    numClientIds = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass ? 4 : 5;
    valueMask = BIT_0 | (1 << numClientIds);
    currentValueMask = valueMask << hwClientId;
    /* caller already '>>' the portGroupsBmp */
    /* get proper port groups */
    currentPortGroupsBmp = portGroupsBmp & ((1<<numOfPipeOnTcam)-1);


    for(ii = 0 ; ii < numOfPipeOnTcam; ii++)
    {
        if(0 == (currentPortGroupsBmp & (1 << ii)))
        {
            /* remove the bit of this port group from the mask */
            currentValueMask &= ~(((1 << numClientIds) - 1) << (numClientIds*ii));
        }
    }

    /* first : disable the client on the rest of 4 tcam groups */
    value = 0;

    for(ii = 0 ; ii < numOfClientGroups ;ii++)
    {
        if(ii == tcamGroup)
        {
            /* will be configured after this loop */
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.groupClientEnable[ii];
        rc = prvCpssHwPpPortGroupWriteRegBitMask(devNum,representativePortGroupId,regAddr,currentValueMask,value);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* enable/disable the client on the chosen tcam group */
    value = (enable == GT_TRUE) ? 0xFFFFFFFF : 0;
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.groupClientEnable[tcamGroup];
    rc = prvCpssHwPpPortGroupWriteRegBitMask(devNum,representativePortGroupId,regAddr,currentValueMask,value);
    if(rc != GT_OK)
    {
        return rc;
    }

    return rc;
}
/**
* @internal internal_cpssDxChTcamPortGroupClientGroupSet function
* @endinternal
*
* @brief   Enable/Disable client per TCAM group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] tcamClient               - TCAM client.
* @param[in] tcamGroup                - TCAM group id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Falcon: 0..3)
* @param[in] enable                   - GT_TRUE: TCAM client is enabled
*                                      GT_FALSE: TCAM client is disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
static GT_STATUS internal_cpssDxChTcamPortGroupClientGroupSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_TCAM_CLIENT_ENT           tcamClient,
    IN  GT_U32                              tcamGroup,
    IN  GT_BOOL                             enable
)
{
    GT_STATUS rc;
    GT_U32    hwClientId;                   /* HW index for tcamClient   */
    GT_PORT_GROUPS_BMP client_portGroupsBmp = portGroupsBmp;/* the BMP of pipes that need clients */
    GT_U32  portGroupId;                    /* the port group Id            */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E |
        CPSS_BOBCAT3_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    /* TCAM we have for every 2 pipes */
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
    /* clients we have per pipe we have for every 2 pipes */
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,client_portGroupsBmp);
    PRV_CPSS_DXCH_TCAM_GROUP_ID_DEV_CHECK_MAC(devNum,tcamGroup);

    PRV_CPSS_DXCH_TCAM_CONVERT_CLIENT_TO_CLIENT_ID_VAL_MAC(devNum, hwClientId, tcamClient);

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        rc = clientGroupForRepresentativePortGroupSet(devNum,
            client_portGroupsBmp >> portGroupId,/* the BMP of clients */
            hwClientId,
                        tcamGroup,
            portGroupId,/* the representative port group */
            enable);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    return GT_OK;

}

/**
* @internal cpssDxChTcamPortGroupClientGroupSet function
* @endinternal
*
* @brief   Enable/Disable client per TCAM group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] tcamClient               - TCAM client.
* @param[in] tcamGroup                - TCAM group id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Falcon: 0..3)
* @param[in] enable                   - GT_TRUE: TCAM client is enabled
*                                      GT_FALSE: TCAM client is disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTcamPortGroupClientGroupSet

(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_TCAM_CLIENT_ENT           tcamClient,
    IN  GT_U32                              tcamGroup,
    IN  GT_BOOL                             enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamPortGroupClientGroupSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, tcamClient, tcamGroup, enable));

    rc = internal_cpssDxChTcamPortGroupClientGroupSet(devNum, portGroupsBmp, tcamClient, tcamGroup, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, tcamClient, tcamGroup, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal clientGroupForRepresentativePortGroupGet function
* @endinternal
*
* @brief   Get Enable/Disable TCAM group per client for Representative Port Group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] hwClientId               - HW index for tcamClient.
* @param[in] representativePortGroupId - the representative port group Id (0,2,4,6).
*                                      every 2 pipes share a TCAM
*
* @param[out] tcamGroupPtr             - TCAM group id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Falcon: 0..3)
*                                      relevant only when (enablePtr == GT_TRUE)
* @param[out] enablePtr                - GT_TRUE: TCAM client is enabled
*                                      GT_FALSE: TCAM client is disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
static GT_STATUS clientGroupForRepresentativePortGroupGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              hwClientId,
    IN  GT_U32                              representativePortGroupId,
    OUT GT_U32                              *tcamGroupPtr,
    OUT  GT_BOOL                            *enablePtr
)
{
    GT_U32    ii;
    GT_STATUS rc;
    GT_U32    value;
    GT_U32    regAddr;          /* register address          */
    GT_U32    currentValueMask;
    GT_U32    currentPortGroupsBmp;
    GT_U32    numClientIds;     /* number of TCAM clients */
    GT_U32    valueMask;        /* support 2 pipes (Falcon) */
    GT_U32    numOfHits = 0;
    GT_U32    numOfPipeOnTcam = 2;
    GT_U32    numOfClientGroups = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.maxClientGroups;

    if(representativePortGroupId & 1)/* every 2 pipes share a TCAM */
    {
        /* only representatives port groups (0,2,4,6) supported */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *enablePtr = GT_FALSE;


    /* set the mask to proper client */
    numClientIds = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass ? 4 : 5;
    valueMask = BIT_0 | (1 << numClientIds);
    currentValueMask = valueMask << hwClientId;
    /* caller already '>>' the portGroupsBmp */
    /* get proper port groups */
    currentPortGroupsBmp = portGroupsBmp & ((1<<numOfPipeOnTcam)-1);


    for(ii = 0 ; ii < numOfPipeOnTcam; ii++)
    {
        if(0 == (currentPortGroupsBmp & (1 << ii)))
        {
            /* remove the bit of this port group from the mask */
            currentValueMask &= ~(((1 << numClientIds) - 1) << (numClientIds*ii));
        }
    }

    for(ii = 0 ; ii < numOfClientGroups ; ii++)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.groupClientEnable[ii];
        rc = prvCpssHwPpPortGroupReadRegBitMask(devNum,representativePortGroupId,regAddr,currentValueMask,&value);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(value == 0)
        {
            /* 'disabled' on all port groups */
        }
        else
        if(value == currentValueMask)
        {
            /* all the bits are '1'*/
            numOfHits ++;
            if(numOfHits > 1)
            {
                /* more then one hit ?! */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

            *tcamGroupPtr = ii;
            *enablePtr = GT_TRUE;
        }
        else
        {
            /* no all port groups 'agree' that this is the tcam group */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

    }

    return GT_OK;
}


/**
* @internal internal_cpssDxChTcamPortGroupClientGroupGet function
* @endinternal
*
* @brief   Gets status (Enable/Disable) for client per TCAM group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] tcamClient               - TCAM client.
*
* @param[out] tcamGroupPtr             - (pointer to) TCAM group id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Falcon: 0..3)
* @param[out] enablePtr                - (pointer to) TCAM client status.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssDxChTcamPortGroupClientGroupGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_TCAM_CLIENT_ENT           tcamClient,
    OUT GT_U32                              *tcamGroupPtr,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS rc;
    GT_U32    hwClientId;                   /* HW index for tcamClient   */
    GT_U32    curr_tcamGroup=0;             /* tcam group from the current representative port group */
    GT_BOOL   curr_enable=GT_FALSE;         /* enable/disable from the current representative port group */
    GT_U32    numValuesGet = 0;             /* number of values read from the registers */
    GT_PORT_GROUPS_BMP client_portGroupsBmp = portGroupsBmp;/* the BMP of pipes that need clients */
    GT_U32  portGroupId;                    /* the port group Id            */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E |
        CPSS_BOBCAT3_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    /* TCAM we have for every 2 pipes */
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
    /* clients we have per pipe we have for every 2 pipes */
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,client_portGroupsBmp);

    CPSS_NULL_PTR_CHECK_MAC(tcamGroupPtr);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass)
    {
        /* Bypassed client is not enabled */
        if (tcamClient == CPSS_DXCH_TCAM_IPCL_2_E)
        {
            *tcamGroupPtr = 0;
            *enablePtr = GT_FALSE;
            return GT_OK;
        }
    }
    PRV_CPSS_DXCH_TCAM_CONVERT_CLIENT_TO_CLIENT_ID_VAL_MAC(devNum, hwClientId, tcamClient);

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        rc = clientGroupForRepresentativePortGroupGet(devNum,
            client_portGroupsBmp >> portGroupId,/* the BMP of clients */
            hwClientId,
            portGroupId,/* the representative port group */
            &curr_tcamGroup,&curr_enable);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(numValuesGet != 0)
        {
            /* we can compare current result with previous one */
            if(curr_tcamGroup != (*tcamGroupPtr))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Getting curr_tcamGroup[%d] (from port group[%d]) that differ from previous one[%d]",
                    curr_tcamGroup,portGroupId,(*tcamGroupPtr));
            }

            if(curr_enable != (*enablePtr))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Getting curr_enable[%d] (from port group[%d]) that differ from previous one[%d]",
                    curr_enable,portGroupId,(*enablePtr));
            }
        }

        *tcamGroupPtr = curr_tcamGroup;
        *enablePtr = curr_enable;

        numValuesGet++;
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChTcamPortGroupClientGroupGet function
* @endinternal
*
* @brief   Gets status (Enable/Disable) for client per TCAM group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] tcamClient               - TCAM client.
*
* @param[out] tcamGroupPtr             - (pointer to) TCAM group id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Falcon: 0..3)
* @param[out] enablePtr                - (pointer to) TCAM client status.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChTcamPortGroupClientGroupGet

(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_TCAM_CLIENT_ENT           tcamClient,
    OUT GT_U32                              *tcamGroupPtr,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamPortGroupClientGroupGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, tcamClient, tcamGroupPtr, enablePtr));

    rc = internal_cpssDxChTcamPortGroupClientGroupGet(devNum, portGroupsBmp, tcamClient, tcamGroupPtr, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, tcamClient, tcamGroupPtr, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamIndexRangeHitNumAndGroupSet function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
*         APPLICABLE DEVICES:
*         Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*         INPUTS:
*         devNum       - the device number
*         floorIndex     - floor index (APPLICABLE RANGES: 0..11)
*         floorInfoArr    - holds group id and lookup Number for each half floor.
*         Index 0 represent banks 0-5 and index 1 represent banks 6-11.
*         APPLICABLE DEVICES: Bobcat2, Bobcat3, Aldrin2, Falcon.
*         Index 0 represent banks 0-1, index 1 represent banks 2-3,
*         index 2 represent banks 4-5, index 3 represent banks 6-7,
*         index 4 represent banks 8-9, index 5 represent banks 10-11.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] floorIndex               - floor index (APPLICABLE RANGES: 0..11)
* @param[in] floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS] - holds group id and lookup Number for each half floor.
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
static GT_STATUS internal_cpssDxChTcamIndexRangeHitNumAndGroupSet
(
    IN GT_U8            devNum,
    IN GT_U32           floorIndex,
    IN CPSS_DXCH_TCAM_BLOCK_INFO_STC          floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS]
)
{

    GT_U32    regAddr;      /* register address  */
    GT_U32  ii;             /* index */
    GT_U32  numOfBlocks;    /* number of blocks in TCAM, 2 in Bobcat2, 6 in Caelum */
    GT_U32  blockId;        /* blockId */

    GT_STATUS rc;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    numOfBlocks = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.numBanksForHitNumGranularity;

    for (ii = 0; ii < numOfBlocks; ii++)
    {
        PRV_CPSS_DXCH_TCAM_GROUP_ID_DEV_CHECK_MAC(devNum,floorInfoArr[ii].group);
        PRV_CPSS_DXCH_TCAM_HIT_NUM_CHECK_MAC((GT_32)(floorInfoArr[ii].hitNum));
    }

    if (floorIndex>=CPSS_DXCH_TCAM_MAX_NUM_FLOORS_MAC(devNum)) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.tcamHitNumAndGroupSelFloor[floorIndex];
    if(!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        /*  index 1 - means all banks at range 6-11 */
        rc = prvCpssHwPpSetRegField(devNum,regAddr,3,3,floorInfoArr[1].group);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssHwPpSetRegField(devNum,regAddr,8,2,floorInfoArr[1].hitNum);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* Bank0 - means all banks at range 0-5 */

        rc = prvCpssHwPpSetRegField(devNum,regAddr,0,3,floorInfoArr[0].group);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssHwPpSetRegField(devNum,regAddr,6,2,floorInfoArr[0].hitNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        for(ii = 0 ; ii < numOfBlocks; ii++)
        {
            blockId = ii * (6 / numOfBlocks);

            rc = prvCpssHwPpSetRegField(devNum,regAddr,blockId*3,3,floorInfoArr[ii].group);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = prvCpssHwPpSetRegField(devNum,regAddr,18 + blockId*2,2,floorInfoArr[ii].hitNum);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;

}

/**
* @internal cpssDxChTcamIndexRangeHitNumAndGroupSet function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
*         APPLICABLE DEVICES:
*         Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*         INPUTS:
*         devNum       - the device number
*         floorIndex     - floor index (APPLICABLE RANGES: 0..11)
*         floorInfoArr    - holds group id and lookup Number for each half floor.
*         Index 0 represent banks 0-5 and index 1 represent banks 6-11.
*         APPLICABLE DEVICES: Bobcat2, Bobcat3, Aldrin2, Falcon.
*         Index 0 represent banks 0-1, index 1 represent banks 2-3,
*         index 2 represent banks 4-5, index 3 represent banks 6-7,
*         index 4 represent banks 8-9, index 5 represent banks 10-11.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] floorIndex               - floor index (APPLICABLE RANGES: 0..11)
* @param[in] floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS] - holds group id and lookup Number for each half floor.
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
GT_STATUS cpssDxChTcamIndexRangeHitNumAndGroupSet
(
    IN GT_U8            devNum,
    IN GT_U32           floorIndex,
    IN CPSS_DXCH_TCAM_BLOCK_INFO_STC          floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamIndexRangeHitNumAndGroupSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, floorIndex, floorInfoArr));

    rc = internal_cpssDxChTcamIndexRangeHitNumAndGroupSet(devNum, floorIndex, floorInfoArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, floorIndex, floorInfoArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamIndexRangeHitNumAndGroupGet function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] floorIndex               - floor index (APPLICABLE RANGES: 0..11)
*
* @param[out] floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS] - group id and lookup Number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/

static GT_STATUS internal_cpssDxChTcamIndexRangeHitNumAndGroupGet
(
    IN GT_U8            devNum,
    IN GT_U32           floorIndex,
    OUT CPSS_DXCH_TCAM_BLOCK_INFO_STC          floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS]
)
{

    GT_U32    regAddr;  /* register address  */
    GT_STATUS rc;
    GT_U32  ii;
    GT_U32  numOfBlocks;    /* number of blocks in TCAM, 2 in Bobcat2, 6 in Caelum */
    GT_U32  blockId;        /* blockId */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(floorInfoArr);

    if (floorIndex>=CPSS_DXCH_TCAM_MAX_NUM_FLOORS_MAC(devNum)) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    numOfBlocks = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.numBanksForHitNumGranularity;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.tcamHitNumAndGroupSelFloor[floorIndex];
    if(!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        rc = prvCpssHwPpGetRegField(devNum,regAddr,3,3,&floorInfoArr[1].group);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssHwPpGetRegField(devNum,regAddr,8,2,&floorInfoArr[1].hitNum);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* Bank0 - means all banks at range 0-5 */

        rc = prvCpssHwPpGetRegField(devNum,regAddr,0,3,&floorInfoArr[0].group);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssHwPpGetRegField(devNum,regAddr,6,2,&floorInfoArr[0].hitNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        for(ii = 0 ; ii < numOfBlocks; ii++)
        {
            blockId = ii * (6 / numOfBlocks);

            rc = prvCpssHwPpGetRegField(devNum,regAddr,blockId*3,3,&floorInfoArr[ii].group);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = prvCpssHwPpGetRegField(devNum,regAddr,18 + blockId*2,2,&floorInfoArr[ii].hitNum);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;

}

/**
* @internal cpssDxChTcamIndexRangeHitNumAndGroupGet function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] floorIndex               - floor index (APPLICABLE RANGES: 0..11)
*
* @param[out] floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS] - group id and lookup Number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTcamIndexRangeHitNumAndGroupGet
(
    IN GT_U8            devNum,
    IN GT_U32           floorIndex,
    OUT CPSS_DXCH_TCAM_BLOCK_INFO_STC          floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamIndexRangeHitNumAndGroupGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, floorIndex, floorInfoArr));

    rc = internal_cpssDxChTcamIndexRangeHitNumAndGroupGet(devNum, floorIndex, floorInfoArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, floorIndex, floorInfoArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamActiveFloorsSet function
* @endinternal
*
* @brief   Function sets number of active floors at TCAM array. The non-active
*         floors will be the upper floors and will be at power down mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] numOfActiveFloors        - number of active floors (APPLICABLE RANGES: 0..12)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
static GT_STATUS internal_cpssDxChTcamActiveFloorsSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          numOfActiveFloors
)
{
    GT_U32    regAddr;  /* register address  */
    GT_STATUS rc;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    if (numOfActiveFloors > CPSS_DXCH_TCAM_MAX_NUM_FLOORS_MAC(devNum)) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.tcamArrayConf.tcamActiveFloors;

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 8, numOfActiveFloors);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChTcamActiveFloorsSet function
* @endinternal
*
* @brief   Function sets number of active floors at TCAM array. The non-active
*         floors will be the upper floors and will be at power down mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] numOfActiveFloors        - number of active floors (APPLICABLE RANGES: 0..12)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTcamActiveFloorsSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          numOfActiveFloors
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamActiveFloorsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numOfActiveFloors));

    rc = internal_cpssDxChTcamActiveFloorsSet(devNum, numOfActiveFloors);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numOfActiveFloors));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamActiveFloorsGet function
* @endinternal
*
* @brief   Function gets number of active floors at TCAM array. The non-active
*         floors will be the upper floors and will be at power down mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] numOfActiveFloorsPtr     - number of active floors
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
static GT_STATUS internal_cpssDxChTcamActiveFloorsGet
(
    IN  GT_U8           devNum,
    OUT  GT_U32         *numOfActiveFloorsPtr
)
{
    GT_U32    regAddr;  /* register address  */
    GT_STATUS rc;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(numOfActiveFloorsPtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.tcamArrayConf.tcamActiveFloors;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 8, numOfActiveFloorsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChTcamActiveFloorsGet function
* @endinternal
*
* @brief   Function gets number of active floors at TCAM array. The non-active
*         floors will be the upper floors and will be at power down mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] numOfActiveFloorsPtr     - number of active floors
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTcamActiveFloorsGet
(
    IN  GT_U8           devNum,
    OUT  GT_U32         *numOfActiveFloorsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamActiveFloorsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numOfActiveFloorsPtr));

    rc = internal_cpssDxChTcamActiveFloorsGet(devNum, numOfActiveFloorsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numOfActiveFloorsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamPortGroupCpuLookupTriggerSet function
* @endinternal
*
* @brief   Sets lookup data and triggers CPU lookup in the TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] group                    - TCAM  id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4)
* @param[in] size                     - key  in TCAM
* @param[in] tcamKeyPtr               - (pointer to) key array in the TCAM
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssDxChTcamPortGroupCpuLookupTriggerSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              group,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        size,
    IN  GT_U32                             *tcamKeyPtr
)
{

    GT_STATUS rc;               /* function return code         */
    GT_U32  portGroupId;        /* the port group Id            */
    GT_U32  regAddr;            /* register address             */
    GT_U32  ii;                 /* iteration index              */
    GT_U32  keySize;            /* key size values              */
    GT_U32  offset;             /* offset in chunk register     */
    GT_U32  value;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(tcamKeyPtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
    PRV_CPSS_DXCH_TCAM_GROUP_ID_DEV_CHECK_MAC(devNum,group);

    PRV_CPSS_DXCH_TCAM_CONVERT_RULE_SIZE_TO_KEY_SIZE_VAL_MAC(keySize,size);

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        /* write key size to register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.mgAccess.mgKeySizeAndTypeGroup[group];
        rc = prvCpssHwPpPortGroupSetRegField(devNum,portGroupId,regAddr,4,3,keySize);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* write key size to key array (key size is 4 bits at the begining of each 80 bits of key array)  */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.mgAccess.mgKeyGroupChunk[group][0];
        for (ii=0, offset=0; ii<keySize+1; ii++)
        {
            rc = prvCpssHwPpPortGroupSetRegField(devNum,portGroupId,regAddr,offset+0,3,keySize);
            if(rc != GT_OK)
            {
                return rc;
            }
            /* copy first 64 bits of key */
            for(ii = 0 ; ii < 2 ; ii++)
            {
                U32_GET_FIELD_IN_ENTRY_MAC(tcamKeyPtr,offset+32*ii,32,value);
                rc = prvCpssHwPpPortGroupSetRegField(devNum,portGroupId,regAddr,offset+4+(32*ii),32,value);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
            ii = 2;
            /* copy next 16 bits of key (total of 80 bits)*/
            U32_GET_FIELD_IN_ENTRY_MAC(tcamKeyPtr,offset+32*ii,16,value);
            rc = prvCpssHwPpPortGroupSetRegField(devNum,portGroupId,regAddr,offset+4+(32*ii),16,value);
            if(rc != GT_OK)
            {
                return rc;
            }
            offset += 80;
        }

        /* trigger request*/
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.mgAccess.mgCompRequest;
        rc = prvCpssHwPpPortGroupSetRegField(devNum,portGroupId,regAddr,group,1,1);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChTcamPortGroupCpuLookupTriggerSet function
* @endinternal
*
* @brief   Sets lookup data and triggers CPU lookup in the TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] group                    - TCAM  id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4)
* @param[in] size                     - key  in TCAM
* @param[in] tcamKeyPtr               - (pointer to) key array in the TCAM
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChTcamPortGroupCpuLookupTriggerSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              group,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        size,
    IN  GT_U32                             *tcamKeyPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamPortGroupCpuLookupTriggerSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, group, size, tcamKeyPtr));

    rc = internal_cpssDxChTcamPortGroupCpuLookupTriggerSet(devNum, portGroupsBmp, group, size, tcamKeyPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, group, size, tcamKeyPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamPortGroupCpuLookupTriggerGet function
* @endinternal
*
* @brief   Gets lookup data and key size from CPU lookup in the TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] group                    - TCAM  id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4)
*
* @param[out] sizePtr                  - (pointer to) key size in TCAM
* @param[out] tcamKeyPtr               - (pointer to) key array in the TCAM
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_STATE             - if in TCAM found rule with different sizes
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssDxChTcamPortGroupCpuLookupTriggerGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              group,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT       *sizePtr,
    OUT GT_U32                             *tcamKeyPtr
)
{

    GT_STATUS rc;               /* function return code         */
    GT_U32  portGroupId;        /* the port group Id            */
    GT_U32  regAddr;            /* register address             */
    GT_U32  ii;                 /* iteration index              */
    GT_U32  currKeySize;        /* current key size value       */
    GT_U32  keySize;            /* key size value               */
    GT_U32  offset;             /* offset in chunk register     */
    GT_U32  value;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(sizePtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamKeyPtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
    PRV_CPSS_DXCH_TCAM_GROUP_ID_DEV_CHECK_MAC(devNum,group);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    /* read first 4 bits to get key size */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.mgAccess.mgKeyGroupChunk[group][0];
    rc = prvCpssHwPpGetRegField(devNum,regAddr,0,3,&currKeySize);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* read rest of 80 bits of key from the register */
    /* copy first 64 bits of key */
    for(ii = 0 ; ii < 2 ; ii++)
    {
        rc = prvCpssHwPpGetRegField(devNum,regAddr,4+(32*ii),32,&value);
        if(rc != GT_OK)
        {
            return rc;
        }
        U32_SET_FIELD_IN_ENTRY_MAC(tcamKeyPtr,32*ii,32,value);
    }
    ii = 2;
    /* copy next 16 bits of key (total of 80 bits)*/
    rc = prvCpssHwPpGetRegField(devNum,regAddr,4+(32*ii),16,&value);
    if(rc != GT_OK)
    {
        return rc;
    }
    U32_SET_FIELD_IN_ENTRY_MAC(tcamKeyPtr,32*ii,16,value);

    /* rule size is 10 bytes */
    if (currKeySize == 0)
    {
        *sizePtr = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E;
        return GT_OK;
    }

    keySize = currKeySize;

    /* read the rest of the key */
    for (ii=1, offset=84; ii<keySize+1; ii++)
    {
        rc = prvCpssHwPpGetRegField(devNum,regAddr,offset+0,3,&currKeySize);
        if(rc != GT_OK)
        {
            return rc;
        }
        if (currKeySize != keySize)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        /* read rest of 80 bits of key from the register */
        /* copy first 64 bits of key */
        for(ii = 0 ; ii < 2 ; ii++)
        {
            rc = prvCpssHwPpGetRegField(devNum,regAddr,offset+(32*ii),32,&value);
            if(rc != GT_OK)
            {
                return rc;
            }
            U32_SET_FIELD_IN_ENTRY_MAC(tcamKeyPtr,offset-(4*ii)+(32*ii),32,value);
        }
        ii = 2;
        /* copy next 16 bits of key (total of 80 bits)*/
        rc = prvCpssHwPpGetRegField(devNum,regAddr,offset+(32*ii),16,&value);
        if(rc != GT_OK)
        {
            return rc;
        }
        U32_SET_FIELD_IN_ENTRY_MAC(tcamKeyPtr,offset-(4*ii)+(32*ii),16,value);
    }

    PRV_CPSS_DXCH_TCAM_CONVERT_KEY_SIZE_TO_RULE_SIZE_VAL_MAC(*sizePtr,keySize);

    return GT_OK;
}

/**
* @internal cpssDxChTcamPortGroupCpuLookupTriggerGet function
* @endinternal
*
* @brief   Gets lookup data and key size from CPU lookup in the TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] group                    - TCAM  id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4)
*
* @param[out] sizePtr                  - (pointer to) key size in TCAM
* @param[out] tcamKeyPtr               - (pointer to) key array in the TCAM
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_STATE             - if in TCAM found rule with different sizes
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChTcamPortGroupCpuLookupTriggerGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              group,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT       *sizePtr,
    OUT GT_U32                             *tcamKeyPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamPortGroupCpuLookupTriggerGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, group, sizePtr, tcamKeyPtr));

    rc = internal_cpssDxChTcamPortGroupCpuLookupTriggerGet(devNum, portGroupsBmp, group, sizePtr, tcamKeyPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, group, sizePtr, tcamKeyPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTcamCpuLookupResultsGet function
* @endinternal
*
* @brief   Gets CPU lookup results.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] group                    - TCAM  id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4)
* @param[in] hitNum                   - hit number in TCAM (APPLICABLE RANGES: 0..3)
* @param[in] portGroupsBmp            -
*
* @param[out] isValidPtr               - (pointer to) whether results are valid for prev trigger set.
* @param[out] isHitPtr                 - (pointer to) whether there was hit in the TCAM. Valid only when isValidPtr == GT_TRUE
* @param[out] hitIndexPtr              - (pointer to) global hit index in the TCAM. Valid only when isHitPtr == GT_TRUE
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - if CPU lookup results are not valid
*/
static GT_STATUS internal_cpssDxChTcamCpuLookupResultsGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              group,
    IN  GT_U32                              hitNum,
    OUT GT_BOOL                            *isValidPtr,
    OUT GT_BOOL                            *isHitPtr,
    OUT GT_U32                             *hitIndexPtr
)
{
    GT_STATUS rc;               /* function return code         */
    GT_U32  portGroupId;        /* the port group Id            */
    GT_U32  regAddr;            /* register address             */
    GT_U32  value;
    GT_U32  floorNum;           /* floor number in TCAM         */
    GT_U32  bankNum;            /* bank number in TCAM          */
    GT_U32  bankIndex;          /* index in bank                */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(isValidPtr);
    CPSS_NULL_PTR_CHECK_MAC(isHitPtr);
    CPSS_NULL_PTR_CHECK_MAC(hitIndexPtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
    PRV_CPSS_DXCH_TCAM_GROUP_ID_DEV_CHECK_MAC(devNum,group);
    PRV_CPSS_DXCH_TCAM_HIT_NUM_CHECK_MAC((GT_32)hitNum);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    /* check trigget request*/
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.mgAccess.mgCompRequest;
    rc = prvCpssHwPpGetRegField(devNum,regAddr,group,1,&value);
    if(rc != GT_OK)
    {
        return rc;
    }
    *isValidPtr = (value == 1)? GT_FALSE : GT_TRUE;
    if (*isValidPtr == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.mgAccess.mgHitGroupHitNum[group][hitNum];
    rc = prvCpssHwPpGetRegField(devNum,regAddr,0,1,&value);
    if(rc != GT_OK)
    {
        return rc;
    }
    *isHitPtr = (BIT2BOOL_MAC(value));

    rc = prvCpssHwPpGetRegField(devNum,regAddr,1,16,&value);
    if(rc != GT_OK)
    {
        return rc;
    }

    U32_GET_FIELD_IN_ENTRY_MAC(&value,12,4,floorNum);
    U32_GET_FIELD_IN_ENTRY_MAC(&value,4,8,bankIndex);
    U32_GET_FIELD_IN_ENTRY_MAC(&value,0,4,bankNum);

    *hitIndexPtr = floorNum*CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS + bankIndex * CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS + bankNum;

    return GT_OK;
}

/**
* @internal cpssDxChTcamCpuLookupResultsGet function
* @endinternal
*
* @brief   Gets CPU lookup results.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] group                    - TCAM  id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4)
* @param[in] hitNum                   - hit number in TCAM (APPLICABLE RANGES: 0..3)
* @param[in] portGroupsBmp            -
*
* @param[out] isValidPtr               - (pointer to) whether results are valid for prev trigger set.
* @param[out] isHitPtr                 - (pointer to) whether there was hit in the TCAM. Valid only when isValidPtr == GT_TRUE
* @param[out] hitIndexPtr              - (pointer to) global hit index in the TCAM. Valid only when isHitPtr == GT_TRUE
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - if CPU lookup results are not valid
*/
GT_STATUS cpssDxChTcamCpuLookupResultsGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              group,
    IN  GT_U32                              hitNum,
    OUT GT_BOOL                            *isValidPtr,
    OUT GT_BOOL                            *isHitPtr,
    OUT GT_U32                             *hitIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTcamCpuLookupResultsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, group, hitNum, isValidPtr, isHitPtr, hitIndexPtr));

    rc = internal_cpssDxChTcamCpuLookupResultsGet(devNum, portGroupsBmp, group, hitNum, isValidPtr, isHitPtr, hitIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, group, hitNum, isValidPtr, isHitPtr, hitIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

