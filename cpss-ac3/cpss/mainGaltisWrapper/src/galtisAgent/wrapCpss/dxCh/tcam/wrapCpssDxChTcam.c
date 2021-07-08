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
* @file wrapCpssDxChTcam.c
*
* @brief Wrapper functions for Cpss Dx Tcam
*
* @version   34
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

/**
* @internal wrCpssDxChTcamPortGroupRuleWrite function
* @endinternal
*
* @brief   Writes the TCAM mask and key. Writing operation is preformed on all
*         bits in the line.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTcamPortGroupRuleWrite
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;

    GT_U8           devNum;
    GT_PORT_GROUPS_BMP              portGroupsBmp;
    GT_U32                          index;
    GT_BOOL                         valid;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT    ruleSize;
    GT_U32                          tcamPattern;
    GT_U32                          tcamMask;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portGroupsBmp = (GT_PORT_GROUPS_BMP)inArgs[1];
    index = (GT_U32)inArgs[2];
    valid = (GT_BOOL)inArgs[3];
    ruleSize = (CPSS_DXCH_TCAM_RULE_SIZE_ENT)inArgs[4];
    tcamPattern = (GT_U32)inArgs[5];
    tcamMask = (GT_U32)inArgs[6];

    /* call cpss api function */
    result = cpssDxChTcamPortGroupRuleWrite(devNum, portGroupsBmp,index,valid,ruleSize,&tcamPattern, &tcamMask);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChTcamPortGroupRuleRead function
* @endinternal
*
* @brief   Reads the TCAM mask and key.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTcamPortGroupRuleRead
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;

    GT_U8           devNum;
    GT_PORT_GROUPS_BMP              portGroupsBmp;
    GT_U32                          index;
    GT_BOOL                         valid;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT    ruleSize;
    GT_U32                          tcamPattern[20];
    GT_U32                          tcamMask[20];

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portGroupsBmp = (GT_PORT_GROUPS_BMP)inArgs[1];
    index = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTcamPortGroupRuleRead(devNum, portGroupsBmp,index,&valid,&ruleSize,&tcamPattern[0], &tcamMask[0]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d", valid,ruleSize,tcamPattern[0],tcamMask[0]);
    return CMD_OK; 
}

/**
* @internal wrCpssDxChTcamPortGroupRuleValidStatusSet function
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
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_STATE             - if in TCAM found rule of size different from the specified
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTcamPortGroupRuleValidStatusSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;

    GT_U8           devNum;
    GT_PORT_GROUPS_BMP              portGroupsBmp;
    GT_U32                          index;
    GT_BOOL                         valid;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portGroupsBmp = (GT_PORT_GROUPS_BMP)inArgs[1];
    index = (GT_U32)inArgs[2];
    valid = (GT_BOOL)inArgs[3];



    /* call cpss api function */
    result = cpssDxChTcamPortGroupRuleValidStatusSet(devNum, portGroupsBmp,index,valid);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChTcamPortGroupRuleValidStatusGet function
* @endinternal
*
* @brief   Gets validity of the TCAM rule.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_STATE             - if in TCAM found rule of size different from the specified
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTcamPortGroupRuleValidStatusGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;

    GT_U8           devNum;
    GT_PORT_GROUPS_BMP              portGroupsBmp;
    GT_U32                          index;
    GT_BOOL                         valid;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT    ruleSize;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portGroupsBmp = (GT_PORT_GROUPS_BMP)inArgs[1];
    index = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTcamPortGroupRuleValidStatusGet(devNum, portGroupsBmp,index,&valid,&ruleSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", valid,ruleSize);
    return CMD_OK;
}

/**
* @internal wrCpssDxChTcamPortGroupClientGroupSet function
* @endinternal
*
* @brief   Enable/Disable client per TCAM group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
static CMD_STATUS wrCpssDxChTcamPortGroupClientGroupSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;

    GT_U8           devNum;
    GT_PORT_GROUPS_BMP                  portGroupsBmp;
    CPSS_DXCH_TCAM_CLIENT_ENT           tcamClient;
    GT_U32                              tcamGroup;
    GT_BOOL                             enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portGroupsBmp = (GT_PORT_GROUPS_BMP)inArgs[1];
    tcamClient = (CPSS_DXCH_TCAM_CLIENT_ENT)inArgs[2];
    tcamGroup = (GT_U32)inArgs[3];
    enable = (GT_BOOL)inArgs[4];


    /* call cpss api function */
    result = cpssDxChTcamPortGroupClientGroupSet(devNum, portGroupsBmp,tcamClient,tcamGroup,enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChTcamPortGroupClientGroupGet function
* @endinternal
*
* @brief   Gets status (Enable/Disable) for client per TCAM group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static CMD_STATUS wrCpssDxChTcamPortGroupClientGroupGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;

    GT_U8           devNum;
    GT_PORT_GROUPS_BMP          portGroupsBmp;
    CPSS_DXCH_TCAM_CLIENT_ENT   tcamClient;
    GT_U32                      tcamGroup;
    GT_BOOL                     enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portGroupsBmp = (GT_PORT_GROUPS_BMP)inArgs[1];
    tcamClient = (CPSS_DXCH_TCAM_CLIENT_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTcamPortGroupClientGroupGet(devNum, portGroupsBmp,tcamClient,&tcamGroup,&enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", tcamGroup,enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChTcamIndexRangeHitNumAndGroupSet function
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
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
static CMD_STATUS wrCpssDxChTcamIndexRangeHitNumAndGroupSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    GT_U32          floorIndex;
    CPSS_DXCH_TCAM_BLOCK_INFO_STC          floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    floorIndex = (GT_U32)inArgs[1];
    if ( PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.numBanksForHitNumGranularity == 6)
    {
        floorInfoArr[0].group = (GT_U32)inArgs[2];
        floorInfoArr[0].hitNum = (GT_U32)inArgs[3];
        floorInfoArr[1].group = (GT_U32)inArgs[2];
        floorInfoArr[1].hitNum = (GT_U32)inArgs[3];
        floorInfoArr[2].group = (GT_U32)inArgs[2];
        floorInfoArr[2].hitNum = (GT_U32)inArgs[3];
        floorInfoArr[3].group = (GT_U32)inArgs[4];
        floorInfoArr[3].hitNum = (GT_U32)inArgs[5];
        floorInfoArr[4].group = (GT_U32)inArgs[4];
        floorInfoArr[4].hitNum = (GT_U32)inArgs[5];
        floorInfoArr[5].group = (GT_U32)inArgs[4];
        floorInfoArr[5].hitNum = (GT_U32)inArgs[5];
    }
    else
    {
        floorInfoArr[0].group = (GT_U32)inArgs[2];
        floorInfoArr[0].hitNum = (GT_U32)inArgs[3];
        floorInfoArr[1].group = (GT_U32)inArgs[4];
        floorInfoArr[1].hitNum = (GT_U32)inArgs[5];
    }

    /* call cpss api function */
    result = cpssDxChTcamIndexRangeHitNumAndGroupSet(devNum, floorIndex,floorInfoArr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/****************cpssDxChTcamIndexRangeHitNumAndGroupSet Table************/
#define CPSS_DXCH_TCAM_BLOCK_TEMP_VALUE_CNS 10
/*set*/
static CPSS_DXCH_TCAM_BLOCK_INFO_STC    floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];

/*get*/
static GT_U32          floorInfoEntryLastIndx;


/**
* @internal wrCpssDxChTcamIndexRangeHitNumAndGroupSetNext function
* @endinternal
*
* @brief   Write an array of TCAM block info entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
static CMD_STATUS wrCpssDxChTcamIndexRangeHitNumAndGroupSetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
     GT_U32   index;

     /* check for valid arguments */
     if(!inArgs || !outArgs)
         return CMD_AGENT_ERROR;

     /* map input arguments to locals */
     index = (GT_U32) inFields[0];

     if (index > 5)
     {
        galtisOutput(outArgs, GT_NO_RESOURCE, "\nIndex range is between 0-5\n");
        return CMD_OK;
     }
     floorInfoArr[index].group = (GT_U32)inFields[1];
     floorInfoArr[index].hitNum = (GT_U32)inFields[2];

     /* pack output arguments to galtis string */
     galtisOutput(outArgs, GT_OK, "");

     return CMD_OK;
}

/**
* @internal wrCpssDxChTcamIndexRangeHitNumAndGroupSetFirst function
* @endinternal
*
* @brief   Write an array of TCAM block info entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
static CMD_STATUS wrCpssDxChTcamIndexRangeHitNumAndGroupSetFirst
(
 IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
 IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
 IN  GT_32 numFields,
 OUT GT_8  outArgs[CMD_MAX_BUFFER]
 )
{
    GT_U32   ii=0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* initializtion */
    for (ii = 0; ii < CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS; ii++)
    {
        floorInfoArr[ii].group = CPSS_DXCH_TCAM_BLOCK_TEMP_VALUE_CNS;
        floorInfoArr[ii].hitNum = CPSS_DXCH_TCAM_BLOCK_TEMP_VALUE_CNS;
    }

    return wrCpssDxChTcamIndexRangeHitNumAndGroupSetNext(inArgs, inFields, numFields, outArgs);
}

/**
* @internal wrCpssDxChTcamIndexRangeHitNumAndGroupEndSet function
* @endinternal
*
* @brief   Write an array of TCAM block info entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
 static CMD_STATUS wrCpssDxChTcamIndexRangeHitNumAndGroupEndSet
     (
     IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
     IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
     IN  GT_32 numFields,
     OUT GT_8  outArgs[CMD_MAX_BUFFER]
     )
 {

     GT_STATUS      status;
     GT_U32         ii;
     GT_U8          dev;
     GT_U32         floorIndex;
     CPSS_DXCH_TCAM_BLOCK_INFO_STC    floorInfoArrGet[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];

     /* check for valid arguments */
     if(!inArgs || !outArgs)
         return CMD_AGENT_ERROR;

     dev = (GT_U8)inArgs[0];
     floorIndex = (GT_U32)inArgs[1];

     status = cpssDxChTcamIndexRangeHitNumAndGroupGet(dev,floorIndex,floorInfoArrGet);
     for (ii = 0; ii < CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS; ii++)
     {
         if (floorInfoArr[ii].group == CPSS_DXCH_TCAM_BLOCK_TEMP_VALUE_CNS
             && floorInfoArr[ii].hitNum == CPSS_DXCH_TCAM_BLOCK_TEMP_VALUE_CNS)
         {
             floorInfoArr[ii].group = floorInfoArrGet[ii].group;
             floorInfoArr[ii].hitNum = floorInfoArrGet[ii].hitNum;
         }
     }
     /* call cpss api function */
     status = cpssDxChTcamIndexRangeHitNumAndGroupSet(dev,floorIndex,floorInfoArr);

     /* pack output arguments to galtis string */
     galtisOutput(outArgs, status, "");

     return CMD_OK;
 }


/*******************************************************************************
* wrCpssDxChTcamIndexRangeHitNumAndGroupGetNext
*
* DESCRIPTION:
*       Select for each TCAM index which TCAM group and lookup number
*       is served by TCAM.
*
* APPLICABLE DEVICES:
*        Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum                  - the device number
*       floorIndex              - floor index (0..11)
*       floorInfoArr               - group id and lookup Number
*
* OUTPUTS:
*       floorInfoArr               - group id and lookup Number
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
static CMD_STATUS wrCpssDxChTcamIndexRangeHitNumAndGroupGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    if (floorInfoEntryLastIndx == CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }
    inFields[0] = floorInfoEntryLastIndx;
    inFields[1] = floorInfoArr[floorInfoEntryLastIndx].group;
    inFields[2] = floorInfoArr[floorInfoEntryLastIndx].hitNum;

    /* pack and output table fields */
    fieldOutput("%d%d%d", inFields[0], inFields[1], inFields[2]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%d%d%d",floorInfoEntryLastIndx, floorInfoArr[floorInfoEntryLastIndx].group, floorInfoArr[floorInfoEntryLastIndx].hitNum);

    floorInfoEntryLastIndx++;

    return CMD_OK;
}

/**
* @internal wrCpssDxChTcamIndexRangeHitNumAndGroupGetFirst function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
static CMD_STATUS wrCpssDxChTcamIndexRangeHitNumAndGroupGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
     GT_U8          dev;
     GT_U32         floorIndex;
     GT_STATUS      rc;

     /* check for valid arguments */
     if(!inArgs || !outArgs)
         return CMD_AGENT_ERROR;

     dev = (GT_U8)inArgs[0];
     floorIndex = (GT_U32)inArgs[1];

     /* clean information */
     cpssOsBzero((GT_CHAR *)floorInfoArr, sizeof(floorInfoArr));
     floorInfoEntryLastIndx = 0;
     rc = cpssDxChTcamIndexRangeHitNumAndGroupGet(dev,floorIndex,floorInfoArr);
     if (rc != GT_OK)
     {
         galtisOutput(outArgs, rc, "%d", -1);
         return CMD_OK;
     }

     return wrCpssDxChTcamIndexRangeHitNumAndGroupGetNext(inArgs, inFields, numFields, outArgs);

}


/**
* @internal wrCpssDxChTcamIndexRangeHitNumAndGroupGet function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
static CMD_STATUS wrCpssDxChTcamIndexRangeHitNumAndGroupGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;

    GT_U8            devNum;
    GT_U32           floorIndex;
    CPSS_DXCH_TCAM_BLOCK_INFO_STC          floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    floorIndex = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTcamIndexRangeHitNumAndGroupGet(devNum, floorIndex,floorInfoArr);

    /* pack output arguments to galtis string */
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.numBanksForHitNumGranularity == 2)
    {
        galtisOutput(outArgs, result, "%d%d%d%d", floorInfoArr[0].group, floorInfoArr[0].hitNum, floorInfoArr[1].group, floorInfoArr[1].hitNum);
    }
    else
    {
       galtisOutput(outArgs, result, "%d%d%d%d", floorInfoArr[0].group, floorInfoArr[0].hitNum, floorInfoArr[3].group, floorInfoArr[3].hitNum);
    }
    return CMD_OK;
}
/**
* @internal wrCpssDxChTcamActiveFloorsSet function
* @endinternal
*
* @brief   Function sets number of active floors at TCAM array. The non-active
*         floors will be the upper floors and will be at power down mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
static CMD_STATUS wrCpssDxChTcamActiveFloorsSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;

    GT_U8           devNum;
    GT_U32          numOfActiveFloors;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    numOfActiveFloors = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTcamActiveFloorsSet(devNum, numOfActiveFloors);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}
/**
* @internal wrCpssDxChTcamActiveFloorsGet function
* @endinternal
*
* @brief   Function gets number of active floors at TCAM array. The non-active
*         floors will be the upper floors and will be at power down mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
static CMD_STATUS wrCpssDxChTcamActiveFloorsGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;

    GT_U8           devNum;
    GT_U32          tcamActiveFloors;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChTcamActiveFloorsGet(devNum, &tcamActiveFloors);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", tcamActiveFloors);
    return CMD_OK;
}
/**
* @internal wrCpssDxChTcamPortGroupCpuLookupTriggerSet function
* @endinternal
*
* @brief   Sets lookup data and triggers CPU lookup in the TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static CMD_STATUS wrCpssDxChTcamPortGroupCpuLookupTriggerSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;

    GT_U8                           devNum;
    GT_PORT_GROUPS_BMP              portGroupsBmp;
    GT_U32                          group;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT    size;
    GT_U32                          tcamKey;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portGroupsBmp = (GT_PORT_GROUPS_BMP)inArgs[1];
    group = (GT_U32)inArgs[2];
    size = (CPSS_DXCH_TCAM_RULE_SIZE_ENT)inArgs[3];
    tcamKey = (GT_U32)inArgs[43];


    /* call cpss api function */
    result = cpssDxChTcamPortGroupCpuLookupTriggerSet(devNum, portGroupsBmp,group,size, &tcamKey );

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}
/**
* @internal wrCpssDxChTcamPortGroupCpuLookupTriggerGet function
* @endinternal
*
* @brief   Gets lookup data and key size from CPU lookup in the TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_STATE             - if in TCAM found rule with different sizes
* @retval GT_BAD_PTR               - on NULL pointer
*/
static CMD_STATUS wrCpssDxChTcamPortGroupCpuLookupTriggerGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;

    GT_U8                           devNum;
    GT_PORT_GROUPS_BMP              portGroupsBmp;
    GT_U32                          group;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT    size;
    GT_U32                          tcamKey[18];


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portGroupsBmp = (GT_PORT_GROUPS_BMP)inArgs[1];
    group = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTcamPortGroupCpuLookupTriggerGet(devNum, portGroupsBmp,group,&size, &tcamKey[0]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", size, tcamKey[0]);
    return CMD_OK;
}

/**
* @internal wrCpssDxChTcamCpuLookupResultsGet function
* @endinternal
*
* @brief   Gets CPU lookup results.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - if CPU lookup results are not valid
*/
static CMD_STATUS wrCpssDxChTcamCpuLookupResultsGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;

    GT_U8                   devNum;
    GT_PORT_GROUPS_BMP      portGroupsBmp;
    GT_U32                  group;
    GT_U32                  hitNum;
    GT_BOOL                 isValid;
    GT_BOOL                 isHit;
    GT_U32                  hitIndex;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portGroupsBmp = (GT_PORT_GROUPS_BMP)inArgs[1];
    group = (GT_U32)inArgs[2];
    hitNum = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChTcamCpuLookupResultsGet(devNum, portGroupsBmp,group,hitNum,&isValid, &isHit, &hitIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", isValid, isHit, hitIndex);
    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChTcamPortGroupRuleWrite",
        &wrCpssDxChTcamPortGroupRuleWrite,
        7, 0},
    {"cpssDxChTcamPortGroupRuleRead",
        &wrCpssDxChTcamPortGroupRuleRead,
        3, 0},
    {"cpssDxChTcamPortGroupRuleValidStatusSet",
        &wrCpssDxChTcamPortGroupRuleValidStatusSet,
        5, 0},
    {"cpssDxChTcamPortGroupRuleValidStatusGet",
        &wrCpssDxChTcamPortGroupRuleValidStatusGet,
        3, 0},
    {"cpssDxChTcamPortGroupClientGroupSet",
        &wrCpssDxChTcamPortGroupClientGroupSet,
        5, 0},
    {"cpssDxChTcamPortGroupClientGroupGet",
        &wrCpssDxChTcamPortGroupClientGroupGet,
        3, 0},
    {"cpssDxChTcamIndexRangeHitNumAndGroupSet",
        &wrCpssDxChTcamIndexRangeHitNumAndGroupSet,
        6, 0},
    {"cpssDxChTcamIndexRangeHitNumAndGroup_1SetFirst",
        &wrCpssDxChTcamIndexRangeHitNumAndGroupSetFirst,
        0, 3},
    {"cpssDxChTcamIndexRangeHitNumAndGroup_1SetNext",
        &wrCpssDxChTcamIndexRangeHitNumAndGroupSetNext,
        0, 3},
    {"cpssDxChTcamIndexRangeHitNumAndGroup_1EndSet",
        &wrCpssDxChTcamIndexRangeHitNumAndGroupEndSet,
        2, 0},
    {"cpssDxChTcamIndexRangeHitNumAndGroupGet",
        &wrCpssDxChTcamIndexRangeHitNumAndGroupGet,
        2, 0},
    {"cpssDxChTcamIndexRangeHitNumAndGroup_1GetFirst",
        &wrCpssDxChTcamIndexRangeHitNumAndGroupGetFirst,
        2, 0},

    {"cpssDxChTcamIndexRangeHitNumAndGroup_1GetNext",
        &wrCpssDxChTcamIndexRangeHitNumAndGroupGetNext,
        2, 0},
    {"cpssDxChTcamActiveFloorsSet",
        &wrCpssDxChTcamActiveFloorsSet,
        2, 0},

    {"cpssDxChTcamActiveFloorsGet",
        &wrCpssDxChTcamActiveFloorsGet,
        1, 0},
    {"cpssDxChTcamPortGroupCpuLookupTriggerSet",
        &wrCpssDxChTcamPortGroupCpuLookupTriggerSet,
        5, 0},
    {"cpssDxChTcamPortGroupCpuLookupTriggerGet",
        &wrCpssDxChTcamPortGroupCpuLookupTriggerGet,
        3, 0},
    {"cpssDxChTcamCpuLookupResultsGet",
        &wrCpssDxChTcamCpuLookupResultsGet,
        3, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChTcam function
* @endinternal
*
* @brief   Library database initialization function.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note none
*
*/
GT_STATUS cmdLibInitCpssDxChTcam
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

