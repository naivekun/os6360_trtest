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
* @file cpssDxChVirtualTcamCliUtil.c
*
* @brief The utilities for fast reconstruct bugs of Virtual TCAM Manager
*        from command line
*
* @version   1
********************************************************************************
*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcamDb.h>
#include <gtUtil/gtStringUtil.h>

void cliWrapCpssDxChVirtualTcamHelp()
{
    cpssOsPrintf("Any ...Inc parameter is signed\n");
    cpssOsPrintf("Enum values are quoted strings\n");
    cpssOsPrintf("cpssDxChVirtualTcamManagerCreate vTcamMngId\n");
    cpssOsPrintf("cpssDxChVirtualTcamManagerDelete vTcamMngId\n");
    cpssOsPrintf("cpssDxChVirtualTcamRemove vTcamMngId, vTcamId\n");
    cpssOsPrintf("cpssDxChVirtualTcamResize TcamMngId, vTcamId, rulePlace, toInsert, sizeInRules\n");
    cpssOsPrintf("cpssDxChVirtualTcamAutoResizeEnable vTcamMngId, vTcamId, enable\n");
    cpssOsPrintf("cpssDxChVirtualTcamAutoResizeGranularitySet vTcamMngId, granularity\n");
    cpssOsPrintf("cliWrapCpssDxChVirtualTcamManagerDevAdd vTcamMngId, devNum\n");
    cpssOsPrintf("cliWrapCpssDxChVirtualTcamManagerDevRemove vTcamMngId, devNum\n");
    cpssOsPrintf("cliWrapCpssDxChVirtualTcamCreate vTcamMngId, vTcamId,\n");
    cpssOsPrintf("---- clientGroup, hitNumber, ruleSize<std,ext,ultra,10b,20b,30b,40b,50b,60b>,\n");
    cpssOsPrintf("---- autoResize, guaranteedNumOfRules, ruleAdditionMethod<index or priority>\n");
    cpssOsPrintf("cliWrapCpssDxChVirtualTcamInfoGet vTcamMngId, vTcamId\n");
    cpssOsPrintf("cliWrapCpssDxChVirtualTcamUsageGet vTcamMngId, vTcamId\n");
    cpssOsPrintf("cliWrapCpssDxChVirtualTcamRuleWrite vTcamMngId, vTcamId, numOfRules,\n");
    cpssOsPrintf("---- ruleIdBase, ruleIdInc, priorityBase, priorityInc, ruleType <pcl or tti>\n");
    cpssOsPrintf("---- ruleStampBase, ruleStampInc, actionStampBase, actionStampInc\n");
    cpssOsPrintf("cliWrapCpssDxChVirtualTcamRuleRead vTcamMngId, vTcamId, numOfRules,\n");
    cpssOsPrintf("---- ruleIdBase, ruleIdInc, ruleType <pcl or tti>\n");
    cpssOsPrintf("cliWrapCpssDxChVirtualTcamRuleActionUpdate vTcamMngId, vTcamId, numOfRules,\n");
    cpssOsPrintf("---- ruleIdBase, ruleIdInc, ruleType <pcl or tti>\n");
    cpssOsPrintf("---- actionStampBase, actionStampInc\n");
    cpssOsPrintf("cliWrapCpssDxChVirtualTcamRuleActionGet vTcamMngId, vTcamId, numOfRules,\n");
    cpssOsPrintf("---- ruleIdBase, ruleIdInc, ruleType <pcl or tti>\n");
    cpssOsPrintf("cliWrapCpssDxChVirtualTcamRuleDelete vTcamMngId, vTcamId, numOfRules,\n");
    cpssOsPrintf("---- ruleIdBase, ruleIdInc\n");
    cpssOsPrintf("cliWrapCpssDxChVirtualTcamRuleMove vTcamMngId, vTcamId, numOfRules,\n");
    cpssOsPrintf("---- srcRuleIdBase, srcRuleIdInc, dstRuleIdBase, dstRuleIdInc\n");
    cpssOsPrintf("cliWrapCpssDxChVirtualTcamRulePriorityUpdate vTcamMngId, vTcamId, numOfRules,\n");
    cpssOsPrintf("---- ruleIdBase, ruleIdInc, priorityBase, priorityInc, position<first,last>\n");
    cpssOsPrintf("cliWrapCpssDxChVirtualTcamRuleValidStatusSet vTcamMngId, vTcamId, numOfRules,\n");
    cpssOsPrintf("---- ruleIdBase, ruleIdInc, validStatus\n");
    cpssOsPrintf("cliWrapCpssDxChVirtualTcamRuleValidStatusGet vTcamMngId, vTcamId, numOfRules\n");
    cpssOsPrintf("cliWrapCpssDxChVirtualTcamNextRuleIdGet vTcamMngId, vTcamId, numOfRules\n");
    cpssOsPrintf("cliWrapCpssDxChVirtualTcamRulePriorityGet vTcamMngId, vTcamId, numOfRules,\n");
    cpssOsPrintf("---- ruleIdBase, ruleIdInc\n");
    cpssOsPrintf("cliWrapCpssDxChVirtualTcamDbRuleIdToHwIndexConvert vTcamMngId, vTcamId, numOfRules,\n");
    cpssOsPrintf("---- ruleIdBase, ruleIdInc\n");
}

typedef const char *ENUM_STR;

typedef struct
{
    ENUM_STR string; /*NULL an the end of array*/
    GT_U32   value;
} ENUM_STR_TO_VAL_STC;

#define ENUM_STR_TO_VAL_END_MAC {NULL, 0}

#define INVALID_ENUM_VALUE_CNS 0xFF

static int isValidAddr(const char* s)
{
    /* prevent to use little numbers as addresses and get exception */
    GT_U32 addr = (GT_U32)(s - ((const char*)0));

    return ((addr & 0xFFFF0000) ? 1 : 0);
}

static GT_U32 stringToEnum(ENUM_STR_TO_VAL_STC *tablePtr, ENUM_STR string)
{
    if (isValidAddr(string) == 0)
    {
        cpssOsPrintf("Invalid address of string parameter\n");
        return INVALID_ENUM_VALUE_CNS;
    }
    for (; (tablePtr->string); tablePtr++)
    {
        if (cpssOsStrCmp(tablePtr->string, string) == 0)
        {
            return tablePtr->value;
        }
    }
    cpssOsPrintf("Not found enum value for string parameter\n");
    return INVALID_ENUM_VALUE_CNS;
}

static ENUM_STR enumToString(ENUM_STR_TO_VAL_STC *tablePtr, GT_U32 enumVal)
{
    for ( ; (tablePtr->string); tablePtr++)
    {
        if (tablePtr->value == enumVal)
        {
            return tablePtr->string;
        }
    }
    cpssOsPrintf("Not found string for enum value\n");
    return "Unknown";
}


static ENUM_STR_TO_VAL_STC stringToValTabRuleAdditionMethod[] =
{
    {"index", CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E},
    {"priority", CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E},
    ENUM_STR_TO_VAL_END_MAC
};

static CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_ENT stringToRuleAdditionMethod(ENUM_STR string)
{
    return stringToEnum(stringToValTabRuleAdditionMethod, string);
}

static ENUM_STR stringFromRuleAdditionMethod(GT_U32 enumVal)
{
    return enumToString(stringToValTabRuleAdditionMethod, enumVal);
}

static ENUM_STR_TO_VAL_STC stringToValTabTcamRuleSize[] =
{
    {"std",   CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_STD_E},
    {"ext",   CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_EXT_E},
    {"ultra", CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ULTRA_E},
    {"10b",   CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E},
    {"20b",   CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E},
    {"30b",   CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E},
    {"40b",   CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E},
    {"50b",   CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E},
    {"60b",   CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E},
    ENUM_STR_TO_VAL_END_MAC
};

static CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT stringToTcamRuleSize(ENUM_STR string)
{
    return stringToEnum(stringToValTabTcamRuleSize, string);
}

static ENUM_STR stringFromTcamRuleSize(GT_U32 enumVal)
{
    return enumToString(stringToValTabTcamRuleSize, enumVal);
}

static ENUM_STR_TO_VAL_STC stringToValTabRuleFormatType[] =
{
    {"pcl", CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E},
    {"tti", CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E},
    ENUM_STR_TO_VAL_END_MAC
};

static CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_ENT stringToRuleFormatType(ENUM_STR string)
{
    return stringToEnum(stringToValTabRuleFormatType, string);
}

static ENUM_STR_TO_VAL_STC stringToValTabRulePriorityPosition[] =
{
    {"first", CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E},
    {"last", CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E},
    ENUM_STR_TO_VAL_END_MAC
};

static CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT stringToRulePriorityPosition(ENUM_STR string)
{
    return stringToEnum(stringToValTabRulePriorityPosition, string);
}

GT_VOID cliWrapCpssDxChVirtualTcamManagerDevAdd
(
    IN GT_U32   vTcamMngId,
    IN GT_U8    devNum
)
{
    GT_STATUS rc;
    IN GT_U8    devListArr[1];

    devListArr[0] = devNum;
    rc = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId, devListArr, 1);
    cpssOsPrintf("returned %d\n", rc);
}

GT_VOID cliWrapCpssDxChVirtualTcamManagerDevRemove
(
    IN GT_U32   vTcamMngId,
    IN GT_U8    devNum
)
{
    GT_STATUS rc;
    IN GT_U8    devListArr[1];

    devListArr[0] = devNum;
    rc = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, 1);
    cpssOsPrintf("returned %d\n", rc);
}

GT_VOID cliWrapCpssDxChVirtualTcamCreate
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    clientGroup,
    IN  GT_U32    hitNumber,
    IN  ENUM_STR  ruleSize,
    IN  GT_BOOL   autoResize,
    IN  GT_U32    guaranteedNumOfRules,
    IN  ENUM_STR  ruleAdditionMethod
)
{
    GT_STATUS                            rc;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC      vTcamInfo;

    vTcamInfo.clientGroup          = clientGroup;
    vTcamInfo.hitNumber            = hitNumber;
    vTcamInfo.ruleSize             = stringToTcamRuleSize(ruleSize);
    if (vTcamInfo.ruleSize == INVALID_ENUM_VALUE_CNS)
    {
        /* message already printed */
        return;
    }
    vTcamInfo.autoResize           = autoResize;
    vTcamInfo.guaranteedNumOfRules = guaranteedNumOfRules;
    vTcamInfo.ruleAdditionMethod   = stringToRuleAdditionMethod(ruleAdditionMethod);
    if (vTcamInfo.ruleAdditionMethod == INVALID_ENUM_VALUE_CNS)
    {
        /* meesage already printed */
        return;
    }

    rc = cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
    cpssOsPrintf("returned %d\n", rc);
}

GT_VOID cliWrapCpssDxChVirtualTcamInfoGet
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId
)
{
    GT_STATUS                            rc;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC      vTcamInfo;

    rc = cpssDxChVirtualTcamInfoGet(vTcamMngId,vTcamId, &vTcamInfo);
    cpssOsPrintf("returned %d\n", rc);
    if (rc != GT_OK)
    {
        return;
    }

    cpssOsPrintf(
        "clientGroup %d hitNumber %d ruleSize %s autoResize %d\n"
        "guaranteedNumOfRules %d ruleAdditionMethod %s\n",
        vTcamInfo.clientGroup,
        vTcamInfo.hitNumber,
        stringFromTcamRuleSize(vTcamInfo.ruleSize),
        vTcamInfo.autoResize,
        vTcamInfo.guaranteedNumOfRules,
        stringFromRuleAdditionMethod(vTcamInfo.ruleAdditionMethod));
}

GT_VOID cliWrapCpssDxChVirtualTcamUsageGet
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId
)
{
    GT_STATUS                            rc;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC     vTcamUsage;

    rc = cpssDxChVirtualTcamUsageGet(vTcamMngId,vTcamId, &vTcamUsage);
    cpssOsPrintf("returned %d\n", rc);
    if (rc != GT_OK)
    {
        return;
    }

    cpssOsPrintf(
        "rulesUsed %d rulesFree %d\n",
        vTcamUsage.rulesUsed, vTcamUsage.rulesFree);
}

static GT_STATUS ruleFormatTypeGet
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  ENUM_STR                                    ruleType, /*PCL or TTI*/
    OUT CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        *ruleTcamTypePtr
)
{
    GT_STATUS                            rc;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC      vTcamInfo;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_ENT ruleTcamType;

    ruleTcamType = stringToRuleFormatType(ruleType);
    if (ruleTcamType == INVALID_ENUM_VALUE_CNS)
    {
        /* message already printed */
        return GT_BAD_PARAM;
    }
    rc = cpssDxChVirtualTcamInfoGet(vTcamMngId, vTcamId, &vTcamInfo);
    if (rc != GT_OK)
    {
        cpssOsPrintf("cpssDxChVirtualTcamInfoGet returned %d\n", rc);
        return rc;
    }
    ruleTcamTypePtr->ruleType = ruleTcamType;
    switch (ruleTcamType)
    {
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E:
            switch (vTcamInfo.ruleSize)
            {
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E:
                    ruleTcamTypePtr->rule.pcl.ruleFormat =
                        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E;
                    break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E:
                    ruleTcamTypePtr->rule.pcl.ruleFormat =
                        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E;
                    break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E: /*CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_STD_E*/
                    ruleTcamTypePtr->rule.pcl.ruleFormat =
                        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
                    break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E:
                    ruleTcamTypePtr->rule.pcl.ruleFormat =
                        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E;
                    break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E:
                    ruleTcamTypePtr->rule.pcl.ruleFormat =
                        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E;
                    break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E: /*CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_EXT_E*/
                    ruleTcamTypePtr->rule.pcl.ruleFormat =
                        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E;
                   break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E: /*CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ULTRA_E*/
                    ruleTcamTypePtr->rule.pcl.ruleFormat =
                        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E;
                    break;
                default: return GT_FAIL; /*should not occur*/
            }
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E:
            switch (vTcamInfo.ruleSize)
            {
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E:
                    ruleTcamTypePtr->rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_UDB_10_E;
                    break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E:
                    ruleTcamTypePtr->rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_UDB_20_E;
                    break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E: /*CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_STD_E*/
                    ruleTcamTypePtr->rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_ETH_E;
                    break;
                default:
                    cpssOsPrintf(
                        "There is no TTI format for vTcam rule size %s\n",
                        stringFromTcamRuleSize(vTcamInfo.ruleSize));
                    return GT_BAD_PARAM;
            }
            break;
        default: return GT_FAIL; /*should not occur*/
    }
    return GT_OK;
}

static GT_STATUS ruleFormatDataFill
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  ENUM_STR                                    ruleType, /*PCL or TTI*/
    IN  GT_U32                                      ruleStamp,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT               *rulePclPtr,
    OUT CPSS_DXCH_TTI_RULE_UNT                      *ruleTtiPtr
)
{
    GT_STATUS                            rc;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC      vTcamInfo;
    GT_U8                                stampByteArr[4];
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_ENT ruleTcamType;

    stampByteArr[0] = (GT_U8)(ruleStamp & 0xFF);
    stampByteArr[1] = (GT_U8)((ruleStamp >> 8) & 0xFF);
    stampByteArr[2] = (GT_U8)((ruleStamp >> 16) & 0xFF);
    stampByteArr[3] = (GT_U8)((ruleStamp >> 24) & 0xFF);


    ruleTcamType = stringToRuleFormatType(ruleType);
    if (ruleTcamType == INVALID_ENUM_VALUE_CNS)
    {
        /* message already printed */
        return GT_BAD_PARAM;
    }
    rc = cpssDxChVirtualTcamInfoGet(vTcamMngId,vTcamId, &vTcamInfo);
    if (rc != GT_OK)
    {
        cpssOsPrintf("cpssDxChVirtualTcamInfoGet returned %d\n", rc);
        return rc;
    }
    switch (ruleTcamType)
    {
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E:
            cpssOsMemSet(rulePclPtr, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
            switch (vTcamInfo.ruleSize)
            {
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E:
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E:
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E:
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E:
                    rulePclPtr->ruleIngrUdbOnly.udb[0] = stampByteArr[0];
                    rulePclPtr->ruleIngrUdbOnly.udb[1] = stampByteArr[1];
                    rulePclPtr->ruleIngrUdbOnly.udb[2] = stampByteArr[2];
                    rulePclPtr->ruleIngrUdbOnly.udb[3] = stampByteArr[3];
                    break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E: /*CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_STD_E*/
                    rulePclPtr->ruleStdNotIp.macDa.arEther[0] = stampByteArr[0];
                    rulePclPtr->ruleStdNotIp.macDa.arEther[1] = stampByteArr[1];
                    rulePclPtr->ruleStdNotIp.macDa.arEther[2] = stampByteArr[2];
                    rulePclPtr->ruleStdNotIp.macDa.arEther[3] = stampByteArr[3];
                    break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E: /*CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_EXT_E*/
                    rulePclPtr->ruleExtNotIpv6.macDa.arEther[0] = stampByteArr[0];
                    rulePclPtr->ruleExtNotIpv6.macDa.arEther[1] = stampByteArr[1];
                    rulePclPtr->ruleExtNotIpv6.macDa.arEther[2] = stampByteArr[2];
                    rulePclPtr->ruleExtNotIpv6.macDa.arEther[3] = stampByteArr[3];
                    break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E: /*CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ULTRA_E*/
                    rulePclPtr->ruleUltraIpv6PortVlanQos.macDa.arEther[0] = stampByteArr[0];
                    rulePclPtr->ruleUltraIpv6PortVlanQos.macDa.arEther[1] = stampByteArr[1];
                    rulePclPtr->ruleUltraIpv6PortVlanQos.macDa.arEther[2] = stampByteArr[2];
                    rulePclPtr->ruleUltraIpv6PortVlanQos.macDa.arEther[3] = stampByteArr[3];
                    break;
                default: return GT_FAIL; /*should not occur*/
            }
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E:
            cpssOsMemSet(ruleTtiPtr, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
            switch (vTcamInfo.ruleSize)
            {
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E:
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E:
                    ruleTtiPtr->udbArray[0] = stampByteArr[0];
                    ruleTtiPtr->udbArray[1] = stampByteArr[1];
                    ruleTtiPtr->udbArray[2] = stampByteArr[2];
                    ruleTtiPtr->udbArray[3] = stampByteArr[3];
                    break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E: /*CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_STD_E*/
                    ruleTtiPtr->eth.common.mac.arEther[0] = stampByteArr[0];
                    ruleTtiPtr->eth.common.mac.arEther[1] = stampByteArr[1];
                    ruleTtiPtr->eth.common.mac.arEther[2] = stampByteArr[2];
                    ruleTtiPtr->eth.common.mac.arEther[3] = stampByteArr[3];
                    break;
                default:
                    cpssOsPrintf(
                        "There is no TTI format for vTcam rule size %s\n",
                        stringFromTcamRuleSize(vTcamInfo.ruleSize));
                    return GT_BAD_PARAM;
            }
            break;
        default: return GT_FAIL; /*should not occur*/
    }
    return GT_OK;
}

static GT_STATUS ruleFormatDataRuleStampGet
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  ENUM_STR                                    ruleType, /*PCL or TTI*/
    IN  CPSS_DXCH_PCL_RULE_FORMAT_UNT               *rulePclPtr,
    IN  CPSS_DXCH_TTI_RULE_UNT                      *ruleTtiPtr,
    OUT GT_U32                                      *ruleStampPtr
)
{
    GT_STATUS                            rc;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC      vTcamInfo;
    GT_U8                                stampByteArr[4];
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_ENT ruleTcamType;

    ruleTcamType = stringToRuleFormatType(ruleType);
    if (ruleTcamType == INVALID_ENUM_VALUE_CNS)
    {
        /* message already printed */
        return GT_BAD_PARAM;
    }
    rc = cpssDxChVirtualTcamInfoGet(vTcamMngId,vTcamId, &vTcamInfo);
    if (rc != GT_OK)
    {
        cpssOsPrintf("cpssDxChVirtualTcamInfoGet returned %d\n", rc);
        return rc;
    }
    switch (ruleTcamType)
    {
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E:
            switch (vTcamInfo.ruleSize)
            {
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E:
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E:
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E:
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E:
                    stampByteArr[0] = rulePclPtr->ruleIngrUdbOnly.udb[0];
                    stampByteArr[1] = rulePclPtr->ruleIngrUdbOnly.udb[1];
                    stampByteArr[2] = rulePclPtr->ruleIngrUdbOnly.udb[2];
                    stampByteArr[3] = rulePclPtr->ruleIngrUdbOnly.udb[3];
                    break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E: /*CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_STD_E*/
                    stampByteArr[0] = rulePclPtr->ruleStdNotIp.macDa.arEther[0];
                    stampByteArr[1] = rulePclPtr->ruleStdNotIp.macDa.arEther[1];
                    stampByteArr[2] = rulePclPtr->ruleStdNotIp.macDa.arEther[2];
                    stampByteArr[3] = rulePclPtr->ruleStdNotIp.macDa.arEther[3];
                    break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E: /*CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_EXT_E*/
                    stampByteArr[0] = rulePclPtr->ruleExtNotIpv6.macDa.arEther[0];
                    stampByteArr[1] = rulePclPtr->ruleExtNotIpv6.macDa.arEther[1];
                    stampByteArr[2] = rulePclPtr->ruleExtNotIpv6.macDa.arEther[2];
                    stampByteArr[3] = rulePclPtr->ruleExtNotIpv6.macDa.arEther[3];
                    break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E: /*CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ULTRA_E*/
                    stampByteArr[0] = rulePclPtr->ruleUltraIpv6PortVlanQos.macDa.arEther[0];
                    stampByteArr[1] = rulePclPtr->ruleUltraIpv6PortVlanQos.macDa.arEther[1];
                    stampByteArr[2] = rulePclPtr->ruleUltraIpv6PortVlanQos.macDa.arEther[2];
                    stampByteArr[3] = rulePclPtr->ruleUltraIpv6PortVlanQos.macDa.arEther[3];
                    break;
                default: return GT_FAIL; /*should not occur*/
            }
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E:
            switch (vTcamInfo.ruleSize)
            {
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E:
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E:
                    stampByteArr[0] = ruleTtiPtr->udbArray[0];
                    stampByteArr[1] = ruleTtiPtr->udbArray[1];
                    stampByteArr[2] = ruleTtiPtr->udbArray[2];
                    stampByteArr[3] = ruleTtiPtr->udbArray[3];
                    break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E: /*CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_STD_E*/
                    stampByteArr[0] = ruleTtiPtr->eth.common.mac.arEther[0];
                    stampByteArr[1] = ruleTtiPtr->eth.common.mac.arEther[1];
                    stampByteArr[2] = ruleTtiPtr->eth.common.mac.arEther[2];
                    stampByteArr[3] = ruleTtiPtr->eth.common.mac.arEther[3];
                    break;
                default:
                    cpssOsPrintf(
                        "There is no TTI format for vTcam rule size %s\n",
                        stringFromTcamRuleSize(vTcamInfo.ruleSize));
                    return GT_BAD_PARAM;
            }
            break;
        default: return GT_FAIL; /*should not occur*/
    }
    *ruleStampPtr =
        ((GT_U32)stampByteArr[0] | ((GT_U32)stampByteArr[1] << 8)
         | ((GT_U32)stampByteArr[2] << 16) | ((GT_U32)stampByteArr[3] << 24));
    return GT_OK;
}


static GT_STATUS actionFormatTypeGet
(
    IN  ENUM_STR                                      ruleType, /*PCL or TTI*/
    OUT CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC   *actionTypePtr
)
{
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_ENT ruleTcamType;

    ruleTcamType = stringToRuleFormatType(ruleType);
    if (ruleTcamType == INVALID_ENUM_VALUE_CNS)
    {
        /* message already printed */
        return GT_BAD_PARAM;
    }

    actionTypePtr->ruleType = ruleTcamType;
    switch (ruleTcamType)
    {
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E:
            actionTypePtr->action.pcl.direction = CPSS_PCL_DIRECTION_INGRESS_E;
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E:
            break;
        default: return GT_FAIL; /*should not occur*/
    }
    return GT_OK;
}


static GT_STATUS ruleFormatActionFill
(
    IN  ENUM_STR                               ruleType, /*PCL or TTI*/
    IN  GT_U32                                 actionStampPtr,
    OUT CPSS_DXCH_PCL_ACTION_STC               *actionPclPtr,
    OUT CPSS_DXCH_TTI_ACTION_UNT               *actionTtiPtr
)
{
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_ENT ruleTcamType;

    ruleTcamType = stringToRuleFormatType(ruleType);
    if (ruleTcamType == INVALID_ENUM_VALUE_CNS)
    {
        /* message already printed */
        return GT_BAD_PARAM;
    }

    switch (ruleTcamType)
    {
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E:
            cpssOsMemSet(actionPclPtr, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));
            actionPclPtr->vlan.ingress.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E;
            actionPclPtr->vlan.ingress.vlanId = (actionStampPtr & 0xFFF);
            actionPclPtr->matchCounter.enableMatchCount = GT_TRUE;
            actionPclPtr->matchCounter.matchCounterIndex = ((actionStampPtr >> 12) & 0xFFF);
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E:
            cpssOsMemSet(actionTtiPtr, 0, sizeof(CPSS_DXCH_TTI_ACTION_UNT));
            actionTtiPtr->type2.tag0VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_TAGGED_E;
            actionTtiPtr->type2.tag0VlanId  = (actionStampPtr & 0xFFF);
            actionTtiPtr->type2.bindToCentralCounter = GT_TRUE;
            actionTtiPtr->type2.centralCounterIndex = ((actionStampPtr >> 12) & 0xFFF);
            break;
        default: return GT_FAIL; /*should not occur*/
    }
    return GT_OK;
}

static GT_STATUS ruleFormatActionStampGet
(
    IN  ENUM_STR                               ruleType, /*PCL or TTI*/
    IN  CPSS_DXCH_PCL_ACTION_STC               *actionPclPtr,
    IN  CPSS_DXCH_TTI_ACTION_UNT               *actionTtiPtr,
    OUT GT_U32                                 *actionStampPtr
)
{
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_ENT ruleTcamType;

    ruleTcamType = stringToRuleFormatType(ruleType);
    if (ruleTcamType == INVALID_ENUM_VALUE_CNS)
    {
        /* message already printed */
        return GT_BAD_PARAM;
    }

    switch (ruleTcamType)
    {
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E:
            *actionStampPtr =
                actionPclPtr->vlan.ingress.vlanId
                | (actionPclPtr->matchCounter.matchCounterIndex << 12);
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E:
            *actionStampPtr =
               actionTtiPtr->type2.tag0VlanId
                | (actionTtiPtr->type2.centralCounterIndex << 12);
        default: return GT_FAIL; /*should not occur*/
    }
    return GT_OK;
}

GT_VOID cliWrapCpssDxChVirtualTcamRuleWrite
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    numOfRules,
    IN  GT_U32    ruleIdBase,
    IN  GT_32     ruleIdInc,
    IN  GT_U32    priorityBase,
    IN  GT_32     priorityInc,
    IN  ENUM_STR  ruleType,
    IN  GT_U32    ruleStampBase,
    IN  GT_32     ruleStampInc,
    IN  GT_U32    actionStampBase,
    IN  GT_32     actionStampInc
)
{
    GT_STATUS                                   rc;
    GT_U32                                      index;
    GT_U32                                      ruleId;
    GT_U32                                      priority;
    GT_U32                                      ruleStamp;
    GT_U32                                      actionStamp;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC  ruleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        ruleTcamType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        ruleData;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT               pclMask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT               pclPattern;
    CPSS_DXCH_PCL_ACTION_STC                    pclAction;
    CPSS_DXCH_TTI_RULE_UNT                      ttiMask;
    CPSS_DXCH_TTI_RULE_UNT                      ttiPattern;
    CPSS_DXCH_TTI_ACTION_UNT                    ttiAction;

    rc = ruleFormatTypeGet(
        vTcamMngId, vTcamId, ruleType, &ruleTcamType);
    if (rc != GT_OK)
    {
        /* message already printed */
        return;
    }

    ruleData.valid = GT_TRUE;
    switch (ruleTcamType.ruleType)
    {
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E:
            ruleData.rule.pcl.maskPtr = &pclMask;
            ruleData.rule.pcl.patternPtr = &pclPattern;
            ruleData.rule.pcl.actionPtr = &pclAction;
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E:
            ruleData.rule.tti.maskPtr    = &ttiMask;
            ruleData.rule.tti.patternPtr = &ttiPattern;
            ruleData.rule.tti.actionPtr  = &ttiAction;
            break;
        default: return; /*should not occur*/
    }

    for (index = 0; (index < numOfRules); index++)
    {
        ruleId        = (GT_U32)(ruleIdBase      + (index * ruleIdInc));
        priority      = (GT_U32)(priorityBase    + (index * priorityInc));
        ruleStamp     = (GT_U32)(ruleStampBase   + (index * ruleStampInc));
        actionStamp   = (GT_U32)(actionStampBase + (index * actionStampInc));

        ruleAttributes.priority = priority;
        rc = ruleFormatDataFill(
            vTcamMngId, vTcamId, ruleType, 0xFFFFFFFF /*ruleStamp*/,
            &pclMask, &ttiMask);
        if (rc != GT_OK)
        {
            /* message already printed */
            return;
        }
        rc = ruleFormatDataFill(
            vTcamMngId, vTcamId, ruleType, ruleStamp,
            &pclPattern, &ttiPattern);
        if (rc != GT_OK)
        {
            /* message already printed */
            return;
        }
        rc = ruleFormatActionFill(
            ruleType, actionStamp,
            &pclAction, &ttiAction);
        if (rc != GT_OK)
        {
            /* message already printed */
            return;
        }

        rc = cpssDxChVirtualTcamRuleWrite(
            vTcamMngId, vTcamId, (CPSS_DXCH_VIRTUAL_TCAM_RULE_ID)ruleId,
            &ruleAttributes, &ruleTcamType, &ruleData);
        if (rc != GT_OK)
        {
            cpssOsPrintf("cpssDxChVirtualTcamRuleWrite failed rc = %d\n", rc);
            return;
        }
    }
    cpssOsPrintf("cpssDxChVirtualTcamRuleWrite all rules written\n");
}

GT_VOID cliWrapCpssDxChVirtualTcamRuleRead
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    numOfRules,
    IN  GT_U32    ruleIdBase,
    IN  GT_32     ruleIdInc,
    IN  ENUM_STR  ruleType
)
{
    GT_STATUS                                   rc;
    GT_U32                                      index;
    GT_U32                                      ruleId;
    GT_U32                                      ruleStamp;
    GT_U32                                      actionStamp;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        ruleTcamType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        ruleData;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT               pclMask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT               pclPattern;
    CPSS_DXCH_PCL_ACTION_STC                    pclAction;
    CPSS_DXCH_TTI_RULE_UNT                      ttiMask;
    CPSS_DXCH_TTI_RULE_UNT                      ttiPattern;
    CPSS_DXCH_TTI_ACTION_UNT                    ttiAction;

    rc = ruleFormatTypeGet(
        vTcamMngId, vTcamId, ruleType, &ruleTcamType);
    if (rc != GT_OK)
    {
        /* message already printed */
        return;
    }

    ruleData.valid = GT_FALSE; /*clear*/
    switch (ruleTcamType.ruleType)
    {
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E:
            ruleData.rule.pcl.maskPtr = &pclMask;
            ruleData.rule.pcl.patternPtr = &pclPattern;
            ruleData.rule.pcl.actionPtr = &pclAction;
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E:
            ruleData.rule.tti.maskPtr    = &ttiMask;
            ruleData.rule.tti.patternPtr = &ttiPattern;
            ruleData.rule.tti.actionPtr  = &ttiAction;
            break;
        default: return; /*should not occur*/
    }

    for (index = 0; (index < numOfRules); index++)
    {
        ruleId  = (GT_U32)(ruleIdBase + (index * ruleIdInc));

        rc = cpssDxChVirtualTcamRuleRead(
            vTcamMngId, vTcamId, (CPSS_DXCH_VIRTUAL_TCAM_RULE_ID)ruleId,
            &ruleTcamType, &ruleData);
        if (rc == GT_NOT_FOUND)
        {
            cpssOsPrintf("Rule Id %d not exists\n", ruleId);
            continue;
        }
        if (rc == GT_BAD_STATE)
        {
            cpssOsPrintf("Rule Id %d contains garbage(GT_BAD_STATE)\n", ruleId);
            continue;
        }
        if (rc != GT_OK)
        {
            cpssOsPrintf("cpssDxChVirtualTcamRuleRead failed rc = %d\n", rc);
            return;
        }
        if (ruleData.valid == GT_FALSE)
        {
            cpssOsPrintf("Rule Id %d is invalid\n", ruleId);
            continue;
        }

        rc = ruleFormatDataRuleStampGet(
            vTcamMngId, vTcamId, ruleType,
            &pclPattern, &ttiPattern, &ruleStamp);
        if (rc != GT_OK)
        {
            /* message already printed */
            return;
        }
        rc = ruleFormatActionStampGet(
            ruleType, &pclAction, &ttiAction, &actionStamp);
        if (rc != GT_OK)
        {
            /* message already printed */
            return;
        }
        cpssOsPrintf(
            "Rule Id %d ruleStamp %d actionStamp %d\n",
            ruleId, ruleStamp, actionStamp);
    }
}

GT_VOID cliWrapCpssDxChVirtualTcamActionUpdate
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    numOfRules,
    IN  GT_U32    ruleIdBase,
    IN  GT_32     ruleIdInc,
    IN  ENUM_STR  ruleType,
    IN  GT_U32    actionStampBase,
    IN  GT_32     actionStampInc
)
{
    GT_STATUS                                   rc;
    GT_U32                                      index;
    GT_U32                                      ruleId;
    GT_U32                                      actionStamp;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC actionType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC actionData;
    CPSS_DXCH_PCL_ACTION_STC                    pclAction;
    CPSS_DXCH_TTI_ACTION_UNT                    ttiAction;

    rc = actionFormatTypeGet(ruleType, &actionType);
    if (rc != GT_OK)
    {
        /* message already printed */
        return;
    }

    switch (actionType.ruleType)
    {
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E:
            actionData.action.pcl.actionPtr = &pclAction;
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E:
            actionData.action.tti.actionPtr = &ttiAction;
            break;
        default: return; /*should not occur*/
    }

    for (index = 0; (index < numOfRules); index++)
    {
        ruleId        = (GT_U32)(ruleIdBase      + (index * ruleIdInc));
        actionStamp   = (GT_U32)(actionStampBase + (index * actionStampInc));

        rc = ruleFormatActionFill(
            ruleType, actionStamp,
            &pclAction, &ttiAction);
        if (rc != GT_OK)
        {
            /* message already printed */
            return;
        }

        rc = cpssDxChVirtualTcamRuleActionUpdate(
            vTcamMngId, vTcamId, (CPSS_DXCH_VIRTUAL_TCAM_RULE_ID)ruleId,
            &actionType, &actionData);
        if (rc != GT_OK)
        {
            cpssOsPrintf(
                "cpssDxChVirtualTcamRuleActionUpdate failed rc %d ruleId %d\n",
                rc, ruleId);
        }
    }
    cpssOsPrintf("cpssDxChVirtualTcamRuleActionUpdate all rules written\n");
}

GT_VOID cliWrapCpssDxChVirtualTcamActionGet
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    numOfRules,
    IN  GT_U32    ruleIdBase,
    IN  GT_32     ruleIdInc,
    IN  ENUM_STR  ruleType
)
{
    GT_STATUS                                   rc;
    GT_U32                                      index;
    GT_U32                                      ruleId;
    GT_U32                                      actionStamp;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC actionType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC actionData;
    CPSS_DXCH_PCL_ACTION_STC                    pclAction;
    CPSS_DXCH_TTI_ACTION_UNT                    ttiAction;

    rc = actionFormatTypeGet(ruleType, &actionType);
    if (rc != GT_OK)
    {
        /* message already printed */
        return;
    }

    switch (actionType.ruleType)
    {
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E:
            actionData.action.pcl.actionPtr = &pclAction;
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E:
            actionData.action.tti.actionPtr = &ttiAction;
            break;
        default: return; /*should not occur*/
    }

    for (index = 0; (index < numOfRules); index++)
    {
        ruleId        = (GT_U32)(ruleIdBase      + (index * ruleIdInc));

        rc = cpssDxChVirtualTcamRuleActionGet(
            vTcamMngId, vTcamId, (CPSS_DXCH_VIRTUAL_TCAM_RULE_ID)ruleId,
            &actionType, &actionData);
        if (rc != GT_OK)
        {
            cpssOsPrintf(
                "cpssDxChVirtualTcamRuleActionUpdate failed rc %d ruleId %d\n",
                rc, ruleId);
        }

        rc = ruleFormatActionStampGet(
            ruleType, &pclAction, &ttiAction, &actionStamp);
        if (rc != GT_OK)
        {
            /* message already printed */
            return;
        }
        cpssOsPrintf(
            "Rule Id %d actionStamp %d\n",
            ruleId, actionStamp);
    }
}

GT_VOID cliWrapCpssDxChVirtualTcamRuleDelete
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    numOfRules,
    IN  GT_U32    ruleIdBase,
    IN  GT_32     ruleIdInc
)
{
    GT_STATUS                                   rc;
    GT_U32                                      index;
    GT_U32                                      ruleId;

    for (index = 0; (index < numOfRules); index++)
    {
        ruleId        = (GT_U32)(ruleIdBase      + (index * ruleIdInc));

        rc = cpssDxChVirtualTcamRuleDelete(
            vTcamMngId, vTcamId, (CPSS_DXCH_VIRTUAL_TCAM_RULE_ID)ruleId);
        if (rc != GT_OK)
        {
            cpssOsPrintf(
                "cpssDxChVirtualTcamRuleDelete failed rc %d ruleId %d\n",
                rc, ruleId);
        }
    }
    cpssOsPrintf("cliWrapCpssDxChVirtualTcamRuleDelete all rules deleted\n");
}

GT_VOID cliWrapCpssDxChVirtualTcamRuleMove
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    numOfRules,
    IN  GT_U32    srcRuleIdBase,
    IN  GT_32     srcRuleIdInc,
    IN  GT_U32    dstRuleIdBase,
    IN  GT_32     dstRuleIdInc
)
{
    GT_STATUS                                   rc;
    GT_U32                                      index;
    GT_U32                                      srcRuleId;
    GT_U32                                      dstRuleId;

    for (index = 0; (index < numOfRules); index++)
    {
        srcRuleId        = (GT_U32)(srcRuleIdBase      + (index * srcRuleIdInc));
        dstRuleId        = (GT_U32)(dstRuleIdBase      + (index * dstRuleIdInc));

        rc = cpssDxChVirtualTcamRuleMove(
            vTcamMngId, vTcamId,
            (CPSS_DXCH_VIRTUAL_TCAM_RULE_ID)srcRuleId,
            (CPSS_DXCH_VIRTUAL_TCAM_RULE_ID)dstRuleId);
        if (rc != GT_OK)
        {
            cpssOsPrintf(
                "cpssDxChVirtualTcamRuleMove failed rc = %d srcRuleId %d dstRuleId %d\n",
                 rc, srcRuleId, dstRuleId);
        }
    }
    cpssOsPrintf("cpssDxChVirtualTcamRuleMove all rules deleted\n");
}

GT_VOID cliWrapCpssDxChVirtualTcamRulePriorityUpdate
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    numOfRules,
    IN  GT_U32    ruleIdBase,
    IN  GT_32     ruleIdInc,
    IN  GT_U32    priorityBase,
    IN  GT_32     priorityInc,
    IN  ENUM_STR  rulePosition
)
{
    GT_STATUS                                                 rc;
    GT_U32                                                    index;
    GT_U32                                                    ruleId;
    GT_U32                                                    priority;
    CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT   position;

    position = stringToRulePriorityPosition(rulePosition);
    if (position == INVALID_ENUM_VALUE_CNS)
    {
        /* message already printed */
        return;
    }

    for (index = 0; (index < numOfRules); index++)
    {
        ruleId        = (GT_U32)(ruleIdBase      + (index * ruleIdInc));
        priority      = (GT_U32)(priorityBase    + (index * priorityInc));

        rc = cpssDxChVirtualTcamRulePriorityUpdate(
            vTcamMngId, vTcamId, (CPSS_DXCH_VIRTUAL_TCAM_RULE_ID)ruleId,
            priority, position);
        if (rc != GT_OK)
        {
            cpssOsPrintf(
                "cpssDxChVirtualTcamRulePriorityUpdate failed rc %d ruleId %d\n",
                rc, ruleId);
        }
    }
    cpssOsPrintf(
        "cliWrapCpssDxChVirtualTcamRulePriorityUpdate all rules priorities updated\n");
}

GT_VOID cliWrapCpssDxChVirtualTcamRuleValidStatusSet
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    numOfRules,
    IN  GT_U32    ruleIdBase,
    IN  GT_32     ruleIdInc,
    IN  GT_BOOL   validStatus
)
{
    GT_STATUS                                   rc;
    GT_U32                                      index;
    GT_U32                                      ruleId;

    for (index = 0; (index < numOfRules); index++)
    {
        ruleId        = (GT_U32)(ruleIdBase      + (index * ruleIdInc));

        rc = cpssDxChVirtualTcamRuleValidStatusSet(
            vTcamMngId, vTcamId, (CPSS_DXCH_VIRTUAL_TCAM_RULE_ID)ruleId, validStatus);
        if (rc != GT_OK)
        {
            cpssOsPrintf(
                "cpssDxChVirtualTcamRuleValidStatusSet failed rc %d ruleId %d\n",
                rc, ruleId);
        }
    }
    cpssOsPrintf("cliWrapCpssDxChVirtualTcamRuleValidStatusSet all rules updated\n");
}

GT_VOID cliWrapCpssDxChVirtualTcamRuleValidStatusGet
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    numOfRules,
    IN  GT_U32    ruleIdBase,
    IN  GT_32     ruleIdInc
)
{
    GT_STATUS                                   rc;
    GT_U32                                      index;
    GT_U32                                      ruleId;
    GT_BOOL                                     validStatus;

    for (index = 0; (index < numOfRules); index++)
    {
        ruleId        = (GT_U32)(ruleIdBase      + (index * ruleIdInc));

        rc = cpssDxChVirtualTcamRuleValidStatusGet(
            vTcamMngId, vTcamId, (CPSS_DXCH_VIRTUAL_TCAM_RULE_ID)ruleId, &validStatus);
        if (rc != GT_OK)
        {
            cpssOsPrintf(
                "cpssDxChVirtualTcamRuleValidStatusGet failed rc %d ruleId %d\n",
                rc, ruleId);
        }
        cpssOsPrintf(
            "Rule Id %d validStatus %d\n",
            ruleId, validStatus);
    }
}

GT_VOID cliWrapCpssDxChVirtualTcamNextRuleIdGet
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    ruleId,
    IN  GT_U32    numOfRules
)
{
    GT_STATUS     rc;
    GT_U32        index;
    GT_U32        nextRuleId;

    for (index = 0; (index < numOfRules); index++)
    {
        rc = cpssDxChVirtualTcamNextRuleIdGet(
            vTcamMngId, vTcamId, ruleId, &nextRuleId);
        if (rc != GT_OK)
        {
            cpssOsPrintf("cpssDxChVirtualTcamNextRuleIdGet failed rc = %d\n", rc);
            return;
        }
        cpssOsPrintf(
            "Rule Id %d nextRuleId %d\n",
            ruleId, nextRuleId);
        ruleId = nextRuleId;
    }
}

GT_VOID cliWrapCpssDxChVirtualTcamRulePriorityGet
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    numOfRules,
    IN  GT_U32    ruleIdBase,
    IN  GT_32     ruleIdInc
)
{
    GT_STATUS                                   rc;
    GT_U32                                      index;
    GT_U32                                      ruleId;
    GT_U32                                      priority;

    for (index = 0; (index < numOfRules); index++)
    {
        ruleId        = (GT_U32)(ruleIdBase      + (index * ruleIdInc));

        rc = cpssDxChVirtualTcamRulePriorityGet(
            vTcamMngId, vTcamId, (CPSS_DXCH_VIRTUAL_TCAM_RULE_ID)ruleId, &priority);
        if (rc != GT_OK)
        {
            cpssOsPrintf(
                "cpssDxChVirtualTcamRulePriorityGet failed rc %d  %d\n",
                rc, ruleId);
        }
        cpssOsPrintf(
            "Rule Id %d priority %d\n",
            ruleId, priority);
    }
}

GT_VOID cliWrapCpssDxChVirtualTcamDbRuleIdToHwIndexConvert
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    numOfRules,
    IN  GT_U32    ruleIdBase,
    IN  GT_32     ruleIdInc
)
{
    GT_STATUS         rc;
    GT_U32            index;
    GT_U32            ruleId;
    GT_U32            logicalIndex;
    GT_U32            hwIndex;

    for (index = 0; (index < numOfRules); index++)
    {
        ruleId        = (GT_U32)(ruleIdBase      + (index * ruleIdInc));

        rc = cpssDxChVirtualTcamDbRuleIdToHwIndexConvert(
            vTcamMngId, vTcamId, (CPSS_DXCH_VIRTUAL_TCAM_RULE_ID)ruleId,
            &logicalIndex, &hwIndex);
        if (rc != GT_OK)
        {
            cpssOsPrintf(
                "cpssDxChVirtualTcamDbRuleIdToHwIndexConvert failed rc %d ruleId %d\n",
                rc, ruleId);
        }
        cpssOsPrintf(
            "Rule Id %d logicalIndex %d hwIndex %d\n",
            ruleId, logicalIndex, hwIndex);
    }
}

