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
* @file wrapCpssDxChPortLoopback.c
*
* @brief Wrapper functions for CPSS DXCH API relevant to forwarding to
* loopback/service port.
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortLoopback.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>


/**
* @internal wrCpssDxChPortLoopbackEnableSet function
* @endinternal
*
* @brief   Enable forwarding-to-loopback-port for specified combination of
*         source/target loopback profiles and traffic class.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Assign source/target physical ports with loopback profiles
*       You need to enable forwarding to loopback per desirable packet types
*       additionally. See
*       cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet
*       cpssDxChPortLoopbackPktTypeFromCpuEnableSet
*       cpssDxChPortLoopbackPktTypeToCpuSet
*       cpssDxChPortLoopbackPktTypeToAnalyzerSet
*
*/
static CMD_STATUS wrCpssDxChPortLoopbackEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    srcProfile;
    GT_U32    trgProfile;
    GT_U32    tc;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)  inArgs[0];
    srcProfile = (GT_U32) inArgs[1];
    trgProfile = (GT_U32) inArgs[2];
    tc         = (GT_U32) inArgs[3];
    enable     = (GT_BOOL)inArgs[4];
    result = cpssDxChPortLoopbackEnableSet(
        devNum, srcProfile, trgProfile, tc, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;

}
/**
* @internal wrCpssDxChPortLoopbackEnableGet function
* @endinternal
*
* @brief   Get a forwarding-to-loopback-port status for specified combination of
*         source/target loopback profiles and traffic class.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortLoopbackEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    srcProfile;
    GT_U32    trgProfile;
    GT_U32    tc;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =     (GT_U8)  inArgs[0];
    srcProfile = (GT_U32) inArgs[1];
    trgProfile = (GT_U32) inArgs[2];
    tc         = (GT_U32) inArgs[3];

    result = cpssDxChPortLoopbackEnableGet(
        devNum, srcProfile, trgProfile, tc, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet function
* @endinternal
*
* @brief   Enable forwarding-to-loopback-port for FORWARD and FROM_CPU packet type.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   fromCpuEn;
    GT_BOOL   singleTargetEn;
    GT_BOOL   multiTargetEn;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum         = (GT_U8)  inArgs[0];
    fromCpuEn      = (GT_BOOL)inArgs[1];
    singleTargetEn = (GT_BOOL)inArgs[2];
    multiTargetEn  = (GT_BOOL)inArgs[3];

    result = cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet(
        devNum, fromCpuEn, singleTargetEn, multiTargetEn);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet function
* @endinternal
*
* @brief   Get status of loopback for FORWARD and FROM_CPU packet type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   fromCpuEn;
    GT_BOOL   singleTargetEn;
    GT_BOOL   multiTargetEn;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    result = cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet(
        devNum, &fromCpuEn, &singleTargetEn, &multiTargetEn);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d,%d,%d",
                 fromCpuEn, singleTargetEn, multiTargetEn);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortLoopbackPktTypeForwardAndFromCpuSet function
* @endinternal
*
* @brief   Specify loopback port for physical port number. This loopback port is
*         used for FORWARD and FROM_CPU packet type physical port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Loopback port is only assigned (i.e. marked as 'loopback port for
*       specified packet type') not configured.
*
*/
static CMD_STATUS wrCpssDxChPortLoopbackPktTypeForwardAndFromCpuSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_PHYSICAL_PORT_NUM loopbackPortNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum         = (GT_U8)inArgs[0];
    portNum        = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    loopbackPortNum = (GT_PHYSICAL_PORT_NUM)inArgs[2];

    result = cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet(
        devNum, portNum, loopbackPortNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortLoopbackPktTypeForwardAndFromCpuGet function
* @endinternal
*
* @brief   Get loopback port assigned to physical port number. The loopback port is
*         used for by FORWARD and FROM_CPU packet targeted to this physical port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Loopback port is only assigned (i.e. marked as 'loopback port for
*       specified packet type') not configured.
*
*/
static CMD_STATUS wrCpssDxChPortLoopbackPktTypeForwardAndFromCpuGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_PHYSICAL_PORT_NUM loopbackPortNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum         = (GT_U8)inArgs[0];
    portNum        = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    result = cpssDxChPortLoopbackPktTypeForwardAndFromCpuGet(
        devNum, portNum, &loopbackPortNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", loopbackPortNum);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortLoopbackPktTypeToCpuSet function
* @endinternal
*
* @brief   Enable forwarding-to-loopback-port and assign a loopback port for
*         TO_CPU packet type with specified CPU code
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Loopback port is only assigned (i.e. marked as 'loopback port for
*       specified packet type') not configured.
*
*/
static CMD_STATUS wrCpssDxChPortLoopbackPktTypeToCpuSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;
    GT_U8                    devNum;
    CPSS_NET_RX_CPU_CODE_ENT cpuCode;
    GT_PHYSICAL_PORT_NUM     loopbackPortNum;
    GT_BOOL                  enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)                   inArgs[0];
    cpuCode         = (CPSS_NET_RX_CPU_CODE_ENT)inArgs[1];
    loopbackPortNum = (GT_PHYSICAL_PORT_NUM)    inArgs[2];
    enable          = (GT_BOOL)                 inArgs[3];

    result = cpssDxChPortLoopbackPktTypeToCpuSet(
        devNum, cpuCode, loopbackPortNum, enable);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortLoopbackPktTypeToCpuGet function
* @endinternal
*
* @brief   Get a forwarding-to-loopback-port status and a loopback port number for
*         TO_CPU packet type with specified CPU code
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortLoopbackPktTypeToCpuGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;
    GT_U8                    devNum;
    CPSS_NET_RX_CPU_CODE_ENT cpuCode;
    GT_PHYSICAL_PORT_NUM     loopbackPortNum;
    GT_BOOL                  enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)                   inArgs[0];
    cpuCode         = (CPSS_NET_RX_CPU_CODE_ENT)inArgs[1];

    result = cpssDxChPortLoopbackPktTypeToCpuGet(
        devNum, cpuCode, &loopbackPortNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d,%d", loopbackPortNum, enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortLoopbackPktTypeToAnalyzerSet function
* @endinternal
*
* @brief   Enable forwarding-to-loopback-port and assign a loopback port for
*         TO_ANALYZER packet mirrored to specified analyzer index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Loopback port is only assigned (i.e. marked as 'loopback port for
*       specified packet type') not configured.
*
*/
static CMD_STATUS wrCpssDxChPortLoopbackPktTypeToAnalyzerSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_U32               analyzerIndex;
    GT_PHYSICAL_PORT_NUM loopbackPortNum;
    GT_BOOL              enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)               inArgs[0];
    analyzerIndex   = (GT_U32)              inArgs[1];
    loopbackPortNum = (GT_PHYSICAL_PORT_NUM)inArgs[2];
    enable          = (GT_BOOL)             inArgs[3];

    result = cpssDxChPortLoopbackPktTypeToAnalyzerSet(
        devNum, analyzerIndex, loopbackPortNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortLoopbackPktTypeToAnalyzerGet function
* @endinternal
*
* @brief   Get a forwarding-to-loopback-port status and a loopback port number for
*         TO_ANALYZER packet mirrored to specified analyzer index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortLoopbackPktTypeToAnalyzerGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_U32               analyzerIndex;
    GT_PHYSICAL_PORT_NUM loopbackPortNum;
    GT_BOOL              enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)               inArgs[0];
    analyzerIndex   = (GT_U32)              inArgs[1];

    result = cpssDxChPortLoopbackPktTypeToAnalyzerGet(
        devNum, analyzerIndex, &loopbackPortNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d,%d", loopbackPortNum, enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortLoopbackProfileSet function
* @endinternal
*
* @brief   Bind source or target loopback profiles to specified physical port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortLoopbackProfileSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_DIRECTION_ENT   direction;
    GT_U32               profile;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)               inArgs[0];
    portNum    = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    direction  = (CPSS_DIRECTION_ENT)  inArgs[2];
    profile    = (GT_U32)              inArgs[3];

    result = cpssDxChPortLoopbackProfileSet(
        devNum, portNum, direction, profile);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortLoopbackProfileGet function
* @endinternal
*
* @brief   Get source or target loopback profile of specified physical port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortLoopbackProfileGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_DIRECTION_ENT   direction;
    GT_U32               profile;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)               inArgs[0];
    portNum    = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    direction  = (CPSS_DIRECTION_ENT)  inArgs[2];

    result = cpssDxChPortLoopbackProfileGet(
        devNum, portNum, direction, &profile);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", profile);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortLoopbackEvidxMappingSet function
* @endinternal
*
* @brief   Enable eVIDX mapping for multi-target traffic forwarded to loopback and
*         specify Loopback eVIDX offset.
*         If enabled multi-target packet forwarded to loopback (this is done after
*         the replication to the port distribution list) will be assigned with
*         new eVIDX=<Loopback eVIDX offset> + <target port>.
*         The plan is to allow you configure every eVIDX in the all target ports
*         range as physical-port-distribution-list that includes only the
*         corresponding physical port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is usually used in conjunction with
*       cpssDxChPortLoopbackEnableEgressMirroringSet
*
*/
static CMD_STATUS wrCpssDxChPortLoopbackEvidxMappingSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_U32               vidxOffset;
    GT_BOOL              enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)   inArgs[0];
    vidxOffset = (GT_U32)  inArgs[1];
    enable     = (GT_BOOL) inArgs[2];

    result = cpssDxChPortLoopbackEvidxMappingSet(
        devNum, vidxOffset, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortLoopbackEvidxMappingGet function
* @endinternal
*
* @brief   Get status and eVIDX mapping for multi-target traffic forwarded to loopback
*         and get specified specify Loopback eVIDX offset.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortLoopbackEvidxMappingGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_U32               vidxOffset;
    GT_BOOL              enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)   inArgs[0];
    vidxOffset = (GT_U32)  inArgs[1];
    enable     = (GT_BOOL) inArgs[2];

    result = cpssDxChPortLoopbackEvidxMappingGet(
        devNum, &vidxOffset, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d,%d", vidxOffset, enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortLoopbackEnableEgressMirroringSet function
* @endinternal
*
* @brief   Enables egress mirroring for packet forwarded to loopback.
*         Useful if you don't want the packet was mirrored twice: before
*         loopback and after loopback.
*         If this option is set a packet will be mirrored after loopback only
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is usually used in conjunction with
*       cpssDxChPortLoopbackEvidxMappingSet
*
*/
static CMD_STATUS wrCpssDxChPortLoopbackEnableEgressMirroringSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_BOOL              enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)   inArgs[0];
    enable = (GT_BOOL) inArgs[1];

    result = cpssDxChPortLoopbackEnableEgressMirroringSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortLoopbackEnableEgressMirroringGet function
* @endinternal
*
* @brief   Get status of egress mirroring for packets forwarded to loopback.
*         Useful if you don't want the packet was mirrored twice: before
*         loopback and after loopback.
*         If this option is set a packet will be mirrored after loopback only
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortLoopbackEnableEgressMirroringGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_BOOL              enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)   inArgs[0];
    enable = (GT_BOOL) inArgs[1];

    result = cpssDxChPortLoopbackEnableEgressMirroringGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**** database initialization **************************************/
static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChPortLoopbackEnableSet",
        &wrCpssDxChPortLoopbackEnableSet,
        5, 0},
    {"cpssDxChPortLoopbackEnableGet",
        &wrCpssDxChPortLoopbackEnableGet,
        4, 0},
    {"cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet",
        &wrCpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet,
        4, 0},
    {"cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet",
        &wrCpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet,
        1, 0},
    {"cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet",
        &wrCpssDxChPortLoopbackPktTypeForwardAndFromCpuSet,
        3, 0},
    {"cpssDxChPortLoopbackPktTypeForwardAndFromCpuGet",
        &wrCpssDxChPortLoopbackPktTypeForwardAndFromCpuGet,
        2, 0},
    {"cpssDxChPortLoopbackPktTypeToCpuSet",
        &wrCpssDxChPortLoopbackPktTypeToCpuSet,
        4, 0},
    {"cpssDxChPortLoopbackPktTypeToCpuGet",
        &wrCpssDxChPortLoopbackPktTypeToCpuGet,
        2, 0},
    {"cpssDxChPortLoopbackPktTypeToAnalyzerSet",
        &wrCpssDxChPortLoopbackPktTypeToAnalyzerSet,
        4, 0},
    {"cpssDxChPortLoopbackPktTypeToAnalyzerGet",
        &wrCpssDxChPortLoopbackPktTypeToAnalyzerGet,
        2, 0},
    {"cpssDxChPortLoopbackProfileSet",
        &wrCpssDxChPortLoopbackProfileSet,
        4, 0},
    {"cpssDxChPortLoopbackProfileGet",
        &wrCpssDxChPortLoopbackProfileGet,
        3, 0},
    {"cpssDxChPortLoopbackEvidxMappingSet",
        &wrCpssDxChPortLoopbackEvidxMappingSet,
        3, 0},
    {"cpssDxChPortLoopbackEvidxMappingGet",
        &wrCpssDxChPortLoopbackEvidxMappingGet,
        1, 0},
    {"cpssDxChPortLoopbackEnableEgressMirroringSet",
        &wrCpssDxChPortLoopbackEnableEgressMirroringSet,
        2, 0},
    {"cpssDxChPortLoopbackEnableEgressMirroringGet",
        &wrCpssDxChPortLoopbackEnableEgressMirroringGet,
        1, 0},
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChPortLoopback function
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
GT_STATUS cmdLibInitCpssDxChPortLoopback
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

