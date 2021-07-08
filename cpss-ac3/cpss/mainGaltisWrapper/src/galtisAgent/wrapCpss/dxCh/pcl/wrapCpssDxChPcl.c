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
* @file wrapCpssDxChPcl.c
*
* @brief Wrapper functions for Pcl cpss.dxCh functions.
*
* @version   88
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

/* Feature specific includes. */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/generic/pcl/cpssPcl.h>
#include <cpss/dxCh/dxChxGen/pcl/private/prvCpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>

#include <galtisAgent/wrapUtil/dxCh/utils/cmdCpssDxChIncrEntry.h>

extern GT_U32 appDemoDxChTcamPclConvertedIndexGet
(
    IN     GT_U8                                devNum,
    IN     GT_U32                               index,
    IN     CPSS_DXCH_TCAM_RULE_SIZE_ENT         ruleSize
);

extern GT_VOID appDemoDxChNewPclTcamSupportSet
(
    IN  GT_BOOL             enableIndexConversion
);

extern GT_VOID appDemoDxChNewPclTcamSupportGet
(
    OUT GT_BOOL            *enableIndexConversion
);

extern GT_U32 appDemoDxChTcamIpclBaseIndexGet
(
    IN     GT_U8           devNum,
    IN     GT_U32          lookupId
);
extern GT_U32 appDemoDxChTcamIpclNumOfIndexsGet
(
    IN     GT_U8           devNum,
    IN     GT_U32          lookupId
);
extern GT_U32 appDemoDxChTcamEpclBaseIndexGet
(
    IN     GT_U8           devNum
);
extern GT_U32 appDemoDxChTcamEpclNumOfIndexsGet
(
    IN     GT_U8           devNum
);

#define PRV_CONVERT_RULE_PORT_MAC(_dev, _maskVar, _patVar, _srcPortPath)     \
    if ((_maskVar._srcPortPath & 0x3F) == 0x3F)                              \
    {                                                                        \
        GT_U8   __dummyDev = _dev;                                           \
        CONVERT_DEV_PHYSICAL_PORT_MAC(__dummyDev, _patVar._srcPortPath);     \
    }
/* check that the bytes array coming from the GALTIS/RDE are not larger then the
   field size

   in case of error --> command fail and return GT_BAD_PARAM
   */
#define CHECK_BYTE_ARRAY_LENGTH_MAC(fieldArrayName,galtisByteArray) \
    if(galtisByteArray.length > sizeof(fieldArrayName))             \
    {                                                               \
        /* pack output arguments to galtis string */                \
        galtisOutput(outArgs, GT_BAD_PARAM, "");                    \
        return CMD_OK;                                              \
    }

/**
* @struct PRV_DXCH_PCL_OVERRIDE_UDB_STC
 *
 * @brief Defines content of some User Defined Bytes in the keys.
 * Only for xCat and above devices.
*/
typedef struct{

    /** @brief Standard Not Ip Key, VRF
     *  (APPLICABLE DEVICES: xCat, Lion).
     */
    GT_BOOL vrfIdLsbEnableStdNotIp;

    /** @brief Standard Not Ip Key, VRF
     *  (APPLICABLE DEVICES: xCat, Lion).
     */
    GT_BOOL vrfIdMsbEnableStdNotIp;

    /** @brief Standard Ip L2 Qos Key, VRF
     *  (APPLICABLE DEVICES: xCat, Lion).
     */
    GT_BOOL vrfIdLsbEnableStdIpL2Qos;

    /** @brief Standard Ip L2 Qos Key, VRF
     *  (APPLICABLE DEVICES: xCat, Lion).
     */
    GT_BOOL vrfIdMsbEnableStdIpL2Qos;

    /** @brief Standard Ipv4 L4 Key, VRF
     *  (APPLICABLE DEVICES: xCat, Lion).
     */
    GT_BOOL vrfIdLsbEnableStdIpv4L4;

    /** @brief Standard Ipv4 L4 Key, VRF
     *  (APPLICABLE DEVICES: xCat, Lion).
     */
    GT_BOOL vrfIdMsbEnableStdIpv4L4;

    /** @brief Standard UDB Key, VRF
     *  (APPLICABLE DEVICES: xCat, Lion).
     */
    GT_BOOL vrfIdLsbEnableStdUdb;

    /** @brief Standard UDB Key, VRF
     *  (APPLICABLE DEVICES: xCat, Lion).
     */
    GT_BOOL vrfIdMsbEnableStdUdb;

    /** @brief Extended Not Ipv6 Key, VRF
     *  (APPLICABLE DEVICES: xCat, Lion).
     */
    GT_BOOL vrfIdLsbEnableExtNotIpv6;

    /** @brief Extended Not Ipv6 Key, VRF
     *  (APPLICABLE DEVICES: xCat, Lion).
     */
    GT_BOOL vrfIdMsbEnableExtNotIpv6;

    /** @brief Extended Ipv6 L2 Key, VRF
     *  (APPLICABLE DEVICES: xCat, Lion).
     */
    GT_BOOL vrfIdLsbEnableExtIpv6L2;

    /** @brief Extended Ipv6 L2 Key, VRF
     *  (APPLICABLE DEVICES: xCat, Lion).
     */
    GT_BOOL vrfIdMsbEnableExtIpv6L2;

    /** @brief Extended Ipv6 L4 Key, VRF
     *  (APPLICABLE DEVICES: xCat, Lion).
     */
    GT_BOOL vrfIdLsbEnableExtIpv6L4;

    /** @brief Extended Ipv6 L4 Key, VRF
     *  (APPLICABLE DEVICES: xCat, Lion).
     */
    GT_BOOL vrfIdMsbEnableExtIpv6L4;

    /** @brief Extended UDB Key, VRF
     *  (APPLICABLE DEVICES: xCat, Lion).
     */
    GT_BOOL vrfIdLsbEnableExtUdb;

    /** @brief Extended UDB Key, VRF
     *  (APPLICABLE DEVICES: xCat, Lion).
     */
    GT_BOOL vrfIdMsbEnableExtUdb;

    /** Standard UDB key, QoS profile in UDB2 */
    GT_BOOL qosProfileEnableStdUdb;

    /** @brief Extended UDB key, QoS profile in UDB5
     *  Comment:
     */
    GT_BOOL qosProfileEnableExtUdb;

} PRV_DXCH_PCL_OVERRIDE_UDB_STC;

/**
* @struct PRV_DXCH_PCL_OVERRIDE_UDB_TRUNK_HASH_STC
 *
 * @brief Defines content of some User Defined Bytes in the key
 * to be the packets trunk hash value.
*/
typedef struct{

    /** @brief Standard Not Ip Key,
     *  trunk hash in UDB17
     */
    GT_BOOL trunkHashEnableStdNotIp;

    /** @brief Standard Ipv4 L4 Key,
     *  trunk hash in UDB22
     */
    GT_BOOL trunkHashEnableStdIpv4L4;

    /** @brief Standard UDB Key,
     *  trunk hash in UDB3
     */
    GT_BOOL trunkHashEnableStdUdb;

    /** @brief Extended Not Ipv6 Key,
     *  trunk hash in UDB3
     */
    GT_BOOL trunkHashEnableExtNotIpv6;

    /** @brief Extended Ipv6 L2 Key,
     *  trunk hash in UDB7
     */
    GT_BOOL trunkHashEnableExtIpv6L2;

    /** @brief Extended Ipv6 L4 Key,
     *  trunk hash in UDB13
     */
    GT_BOOL trunkHashEnableExtIpv6L4;

    /** @brief Extended UDB Key,
     *  trunk hash in UDB3
     *  Comment:
     */
    GT_BOOL trunkHashEnableExtUdb;

} PRV_DXCH_PCL_OVERRIDE_UDB_TRUNK_HASH_STC;


/* forward declaration */
GT_STATUS utilCpssDxChPclRuleIncrementalSequenceSet
(
    IN GT_U8                              devNum,
    IN CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT checkRuleFormat,
    IN GT_U32                             ruleIndexBase,
    IN GT_U32                             ruleIndexIncrement,
    IN GT_U32                             rulesAmount,
    IN char                               *field_full_name,
    IN GT_U32                             elementStart, /* start array element */
    IN GT_U32                             increment
);

/**
* @internal wrCpssDxChPclGenericDymmyGetNext function
* @endinternal
*
*
* @note Generic
*
*/
static CMD_STATUS wrCpssDxChPclGenericDymmyGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    galtisOutput(outArgs, GT_OK, "%d", -1);

    return CMD_OK;
}

/* Rule Format for RuleParsedGet tables */
static CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT utilCpssDxChPclRuleParsedGet_RuleFormat = 0;
/* Rule index for RuleParsedGet tables */
static GT_U32 utilCpssDxChPclRuleParsedGet_RuleIndex = 0;
/* Lines Amount (to output) for RuleParsedGet tables */
static GT_U32 utilCpssDxChPclRuleParsedGet_LinesAmount = 0;

static    GT_BOOL                                extPcl= GT_FALSE;
/* if GT_TRUE == extPcl extended pcl Galtis table is used,
      GT_FALSE otherwise*/

static    GT_BOOL                                sip5Pcl= GT_FALSE;
/* if sip5Pcl==GT_TRUE lion3 pcl Galtis table is used,
      GT_FALSE otherwise*/

static CMD_STATUS  utilCpssDxChPclRuleParsedGetParametersSet
(
    IN CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN  GT_U32                            baseRuleIndex,
    IN  GT_U32                            ruleAmount
)
{
    utilCpssDxChPclRuleParsedGet_RuleFormat = ruleFormat;
    utilCpssDxChPclRuleParsedGet_RuleIndex = baseRuleIndex;
    utilCpssDxChPclRuleParsedGet_LinesAmount = (ruleAmount * 2);
    return CMD_OK;
}

static CMD_STATUS wrUtil_cpssDxChPclRuleParsedGetParametersSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    utilCpssDxChPclRuleParsedGetParametersSet(
        (CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT)inArgs[0],
        (GT_U32)inArgs[1], (GT_U32)inArgs[2]);

    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

/* support for multi port groups */

/**
* @internal pclMultiPortGroupsBmpGet function
* @endinternal
*
* @brief   Get the portGroupsBmp for multi port groups device.
*         when 'enabled' --> wrappers will use the APIs
*         with portGroupsBmp parameter
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)enable / disable the use of APIs with portGroupsBmp parameter.
* @param[out] portGroupsBmpPtr         - (pointer to)port groups bmp , relevant only when enable = GT_TRUE
*                                       NONE
*/
static void pclMultiPortGroupsBmpGet
(
    IN   GT_U8               devNum,
    OUT  GT_BOOL             *enablePtr,
    OUT  GT_PORT_GROUPS_BMP  *portGroupsBmpPtr
)
{
    /* default */
    *enablePtr  = GT_FALSE;
    *portGroupsBmpPtr = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    if (0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        return;
    }

    utilMultiPortGroupsBmpGet(devNum, enablePtr, portGroupsBmpPtr);
}

/* default mode - backward compatible     */
/* the new command  will set these values */
static GT_U32  ruleSet_ruleOptionsBmp = 0;
static CPSS_PORTS_BMP_STC  ruleSet_portListBmpMask    = {{0,0,0,0,0,0,0,0}};
static CPSS_PORTS_BMP_STC  ruleSet_portListBmpPattern = {{0,0,0,0,0,0,0,0}};

static void wrUtil_cpssDxChPclRuleOptionsSet_printHelp()
{
    cpssOsPrintf(" such ruleOptionsBmp parameter supported:\n");
    cpssOsPrintf(" ((ruleOptionsBmp & 0xFFF00000) != 0xFFF00000) - \n");
    cpssOsPrintf(" .........ruleOptionsBmp and portListBmp word0 assignment\n");
    cpssOsPrintf(" .........portListBmp words 1-7 cleared \n");
    cpssOsPrintf(" ruleOptionsBmp == 0xFFF1wwww - \n");
    cpssOsPrintf(" .........portListBmp word wwww assignment (without port conversion)\n");
    cpssOsPrintf(" ruleOptionsBmp == 0xFFF2ddww - \n");
    cpssOsPrintf(".........portListBmp mask word ww assignment (with port conversion)\n");
    cpssOsPrintf(".........dd - devnum used for port conversion\n");
    cpssOsPrintf(".........portListBmp pattern cleared (and must be spesified zero)\n");
    cpssOsPrintf(".........Note: ww - word index i.e. range of portNums before conversion \n");
}

/**
* @internal wrUtil_cpssDxChPclRuleOptionsSet function
* @endinternal
*
* @brief   Set Rule write options.
*/
static CMD_STATUS wrUtil_cpssDxChPclRuleOptionsSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32  wordIndex;
    GT_U8   devNum;
    GT_U8   copyDevNum;
    GT_PORT_NUM   portNum;
    GT_U32  i;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (((GT_U32)inArgs[0] & 0xFFF00000) != 0xFFF00000)
    {
        /* backward compatible mode */

        /* clear both mask and pattern */
        cmdOsMemSet(&ruleSet_portListBmpMask, 0, sizeof(CPSS_PORTS_BMP_STC));
        cmdOsMemSet(&ruleSet_portListBmpPattern, 0, sizeof(CPSS_PORTS_BMP_STC));

        ruleSet_ruleOptionsBmp              = (GT_U32)inArgs[0];
        ruleSet_portListBmpMask.ports[0]    = (GT_U32)inArgs[1];
        ruleSet_portListBmpPattern.ports[0] = (GT_U32)inArgs[2];
    }
    else if (((GT_U32)inArgs[0] & 0xFFFF0000) == 0xFFF10000)
    {
        /* copy without port conversion */
        wordIndex = ((GT_U32)inArgs[0] & 0xFFFF);
        if (wordIndex >= CPSS_MAX_PORTS_BMP_NUM_CNS)
        {
            wrUtil_cpssDxChPclRuleOptionsSet_printHelp();
            galtisOutput(outArgs, GT_BAD_PARAM, "");
            return CMD_OK;
        }
        ruleSet_portListBmpMask.ports[wordIndex]    = (GT_U32)inArgs[1];
        ruleSet_portListBmpPattern.ports[wordIndex] = (GT_U32)inArgs[2];
    }
    else if (((GT_U32)inArgs[0] & 0xFFFF0000) == 0xFFF20000)
    {
        /* copy mask with port conversion       */
        /* pattern must be zero                 */
        /* mask non-zero-bits indexes converted */
        devNum = (GT_U8)(((GT_U32)inArgs[0] >> 8) & 0xFF);
        wordIndex = ((GT_U32)inArgs[0] & 0xFF);
        if ((wordIndex >= CPSS_MAX_PORTS_BMP_NUM_CNS)
            || ((GT_U32)inArgs[2] != 0))
        {
            wrUtil_cpssDxChPclRuleOptionsSet_printHelp();
            galtisOutput(outArgs, GT_BAD_PARAM, "");
            return CMD_OK;
        }
        for (i = 0; (i < 32); i++)
        {
            if (((1 << i) & (GT_U32)inArgs[1]) == 0)
            {
                /* port is don't care */
                continue;
            }
            portNum = (GT_PORT_NUM)((wordIndex * 32) + i);
            /* Override Device and Port */
            copyDevNum = devNum;
            CONVERT_DEV_PORT_U32_MAC(copyDevNum, portNum);
            /* set mask bit for converted port */
            ruleSet_portListBmpMask.ports[portNum / 32] |= (1 << (portNum % 32));
        }
    }
    else
    {
        /* not supported: print help */
        wrUtil_cpssDxChPclRuleOptionsSet_printHelp();
        galtisOutput(outArgs, GT_BAD_PARAM, "");
        return CMD_OK;
    }

    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

/**
* @internal wrUtil_cpssDxChPclPortListBmpSet function
* @endinternal
*
* @brief   Set Rule port list bitmap.
*/
static CMD_STATUS wrUtil_cpssDxChPclPortListBmpSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cmdOsMemSet(&ruleSet_portListBmpMask, 0, sizeof(CPSS_PORTS_BMP_STC));
    cmdOsMemSet(&ruleSet_portListBmpPattern, 0, sizeof(CPSS_PORTS_BMP_STC));

    ruleSet_portListBmpMask.ports[0]    = (GT_U32)inArgs[0];
    ruleSet_portListBmpMask.ports[1]    = (GT_U32)inArgs[1];
    ruleSet_portListBmpMask.ports[2]    = (GT_U32)inArgs[2];
    ruleSet_portListBmpMask.ports[3]    = (GT_U32)inArgs[3];
    ruleSet_portListBmpMask.ports[4]    = (GT_U32)inArgs[4];
    ruleSet_portListBmpMask.ports[5]    = (GT_U32)inArgs[5];
    ruleSet_portListBmpMask.ports[6]    = (GT_U32)inArgs[6];
    ruleSet_portListBmpMask.ports[7]    = (GT_U32)inArgs[7];

    ruleSet_portListBmpPattern.ports[0] = (GT_U32)inArgs[8];
    ruleSet_portListBmpPattern.ports[1] = (GT_U32)inArgs[9];
    ruleSet_portListBmpPattern.ports[2] = (GT_U32)inArgs[10];
    ruleSet_portListBmpPattern.ports[3] = (GT_U32)inArgs[11];
    ruleSet_portListBmpPattern.ports[4] = (GT_U32)inArgs[12];
    ruleSet_portListBmpPattern.ports[5] = (GT_U32)inArgs[13];
    ruleSet_portListBmpPattern.ports[6] = (GT_U32)inArgs[14];
    ruleSet_portListBmpPattern.ports[7] = (GT_U32)inArgs[15];

    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

extern GT_U32          wrCpssDxChVirtualTcam_useVTcamApi;
extern GT_U32          wrCpssDxChVirtualTcam_vTcamMngId;
extern GT_U32          wrCpssDxChVirtualTcam_vTcamId;
extern GT_U32          wrCpssDxChVirtualTcam_vTcamRulePrio;

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPclRuleSet
(
    IN GT_U8                              devNum,
    IN CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN GT_U32                             ruleIndex,
    IN CPSS_DXCH_PCL_RULE_FORMAT_UNT      *maskPtr,
    IN CPSS_DXCH_PCL_RULE_FORMAT_UNT      *patternPtr,
    IN CPSS_DXCH_PCL_ACTION_STC           *actionPtr
)
{
    if (wrCpssDxChVirtualTcam_useVTcamApi)
    {
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC  ruleAttributes;
        CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        ruleType;
        CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        ruleData;

        ruleAttributes.priority = wrCpssDxChVirtualTcam_vTcamRulePrio;
        ruleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
        ruleType.rule.pcl.ruleFormat = ruleFormat;
        ruleData.valid = (ruleSet_ruleOptionsBmp & 0x1) ? GT_FALSE : GT_TRUE;
        ruleData.rule.pcl.actionPtr = actionPtr;
        ruleData.rule.pcl.patternPtr = patternPtr;
        ruleData.rule.pcl.maskPtr = maskPtr;

        return cpssDxChVirtualTcamRuleWrite(wrCpssDxChVirtualTcam_vTcamMngId,
                                            wrCpssDxChVirtualTcam_vTcamId,
                                            ruleIndex,
                                            &ruleAttributes,
                                            &ruleType,
                                            &ruleData);
    }
    else
    {
        GT_BOOL             pgEnable; /* multi port group  enable */
        GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */
        CPSS_DXCH_TCAM_RULE_SIZE_ENT tcamRuleSize;

        pclMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

        /* convert rule's format to rule's size */
        PRV_CPSS_DXCH_PCL_CONVERT_RULE_FORMAT_TO_TCAM_RULE_SIZE_VAL_MAC(tcamRuleSize,ruleFormat);

        ruleIndex = appDemoDxChTcamPclConvertedIndexGet(devNum,ruleIndex,tcamRuleSize);

        if (pgEnable == GT_FALSE)
        {
            return cpssDxChPclRuleSet(
                devNum, ruleFormat, ruleIndex,
                ruleSet_ruleOptionsBmp,
                maskPtr, patternPtr, actionPtr);
        }
        else
        {
            return cpssDxChPclPortGroupRuleSet(
                devNum, pgBmp, ruleFormat, ruleIndex,
                ruleSet_ruleOptionsBmp,
                maskPtr, patternPtr, actionPtr);
        }
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPclRuleActionUpdate
(
    IN GT_U8                              devNum,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleIndex,
    IN CPSS_DXCH_PCL_ACTION_STC           *actionPtr
)
{
    if (wrCpssDxChVirtualTcam_useVTcamApi)
    {
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC actionType;
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC actionData;

        actionType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
        actionType.action.pcl.direction = actionPtr->egressPolicy ? CPSS_PCL_DIRECTION_EGRESS_E : CPSS_PCL_DIRECTION_INGRESS_E;

        actionData.action.pcl.actionPtr = actionPtr;

        return cpssDxChVirtualTcamRuleActionUpdate(wrCpssDxChVirtualTcam_vTcamMngId,
                                            wrCpssDxChVirtualTcam_vTcamId,
                                            ruleIndex,
                                            &actionType,
                                            &actionData);
    }
    else
    {
        GT_BOOL             pgEnable; /* multi port group  enable */
        GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */
        CPSS_DXCH_TCAM_RULE_SIZE_ENT tcamRuleSize;

        pclMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

        /* convert pcl rule's size to tcam rule's size */
        PRV_CPSS_DXCH_PCL_CONVERT_RULE_SIZE_TO_TCAM_RULE_SIZE_VAL_MAC(tcamRuleSize, ruleSize);

        ruleIndex = appDemoDxChTcamPclConvertedIndexGet(devNum,ruleIndex,tcamRuleSize);

        if (pgEnable == GT_FALSE)
        {
            return cpssDxChPclRuleActionUpdate(
                devNum, ruleSize, ruleIndex, actionPtr);
        }
        else
        {
            return cpssDxChPclPortGroupRuleActionUpdate(
                devNum, pgBmp, ruleSize, ruleIndex, actionPtr);
        }
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPclRuleActionGet
(
    IN  GT_U8                              devNum,
    IN  CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN  GT_U32                             ruleIndex,
    IN  CPSS_PCL_DIRECTION_ENT             direction,
    OUT CPSS_DXCH_PCL_ACTION_STC           *actionPtr
)
{
    if (wrCpssDxChVirtualTcam_useVTcamApi)
    {
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC actionType;
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC actionData;

        actionType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
        actionType.action.pcl.direction = direction;

        actionData.action.pcl.actionPtr = actionPtr;

        return cpssDxChVirtualTcamRuleActionGet(wrCpssDxChVirtualTcam_vTcamMngId,
                                            wrCpssDxChVirtualTcam_vTcamId,
                                            ruleIndex,
                                            &actionType,
                                            &actionData);
    }
    else
    {

        GT_BOOL             pgEnable; /* multi port group  enable */
        GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */
        CPSS_DXCH_TCAM_RULE_SIZE_ENT tcamRuleSize;

        pclMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

        /* convert pcl rule's size to tcam rule's size */
        PRV_CPSS_DXCH_PCL_CONVERT_RULE_SIZE_TO_TCAM_RULE_SIZE_VAL_MAC(tcamRuleSize, ruleSize);

        ruleIndex = appDemoDxChTcamPclConvertedIndexGet(devNum,ruleIndex,tcamRuleSize);

        if (pgEnable == GT_FALSE)
        {
            return cpssDxChPclRuleActionGet(
                devNum, ruleSize, ruleIndex, direction, actionPtr);
        }
        else
        {
            return cpssDxChPclPortGroupRuleActionGet(
                devNum, pgBmp, ruleSize, ruleIndex, direction, actionPtr);
        }
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPclRuleInvalidate
(
    IN GT_U8                              devNum,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleIndex
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */
    CPSS_DXCH_TCAM_RULE_SIZE_ENT tcamRuleSize;

    pclMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    /* convert pcl rule's size to tcam rule's size */
    PRV_CPSS_DXCH_PCL_CONVERT_RULE_SIZE_TO_TCAM_RULE_SIZE_VAL_MAC(tcamRuleSize, ruleSize);

    ruleIndex = appDemoDxChTcamPclConvertedIndexGet(devNum,ruleIndex,tcamRuleSize);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChPclRuleInvalidate(
            devNum, ruleSize, ruleIndex);
    }
    else
    {
        return cpssDxChPclPortGroupRuleInvalidate(
            devNum, pgBmp, ruleSize, ruleIndex);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPclRuleValidStatusSet
(
    IN GT_U8                              devNum,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleIndex,
    IN GT_BOOL                            valid
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */
    CPSS_DXCH_TCAM_RULE_SIZE_ENT tcamRuleSize;

    pclMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    /* convert pcl rule's size to tcam rule's size */
    PRV_CPSS_DXCH_PCL_CONVERT_RULE_SIZE_TO_TCAM_RULE_SIZE_VAL_MAC(tcamRuleSize, ruleSize);

    ruleIndex = appDemoDxChTcamPclConvertedIndexGet(devNum,ruleIndex,tcamRuleSize);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChPclRuleValidStatusSet(
            devNum, ruleSize, ruleIndex, valid);
    }
    else
    {
        return cpssDxChPclPortGroupRuleValidStatusSet(
            devNum, pgBmp, ruleSize, ruleIndex, valid);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPclRuleStateGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        ruleIndex,
    OUT GT_BOOL                       *validPtr,
    OUT CPSS_PCL_RULE_SIZE_ENT        *ruleSizePtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */
    CPSS_DXCH_TCAM_RULE_SIZE_ENT tcamRuleSize;

    pclMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    /* convert pcl rule's size to tcam rule's size */
    PRV_CPSS_DXCH_PCL_CONVERT_RULE_SIZE_TO_TCAM_RULE_SIZE_VAL_MAC(tcamRuleSize, CPSS_PCL_RULE_SIZE_STD_E);

    ruleIndex = appDemoDxChTcamPclConvertedIndexGet(devNum,ruleIndex,tcamRuleSize);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChPclRuleStateGet(
            devNum, ruleIndex, validPtr, ruleSizePtr);
    }
    else
    {
        return cpssDxChPclPortGroupRuleAnyStateGet(
            devNum, pgBmp, CPSS_PCL_RULE_SIZE_STD_E,
            ruleIndex, validPtr, ruleSizePtr);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPclRuleCopy
(
    IN GT_U8                              devNum,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleSrcIndex,
    IN GT_U32                             ruleDstIndex
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */
    CPSS_DXCH_TCAM_RULE_SIZE_ENT tcamRuleSize;

    pclMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    /* convert pcl rule's size to tcam rule's size */
    PRV_CPSS_DXCH_PCL_CONVERT_RULE_SIZE_TO_TCAM_RULE_SIZE_VAL_MAC(tcamRuleSize, ruleSize);

    ruleSrcIndex = appDemoDxChTcamPclConvertedIndexGet(devNum,ruleSrcIndex,tcamRuleSize);
    ruleDstIndex = appDemoDxChTcamPclConvertedIndexGet(devNum,ruleDstIndex,tcamRuleSize);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChPclRuleCopy(
            devNum, ruleSize, ruleSrcIndex, ruleDstIndex);
    }
    else
    {
        return cpssDxChPclPortGroupRuleCopy(
            devNum, pgBmp, ruleSize, ruleSrcIndex, ruleDstIndex);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPclRuleGet
(
    IN  GT_U8                  devNum,
    IN  CPSS_PCL_RULE_SIZE_ENT ruleSize,
    IN  GT_U32                 ruleIndex,
    OUT GT_U32                 maskArr[],
    OUT GT_U32                 patternArr[],
    OUT GT_U32                 actionArr[]
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */
    CPSS_DXCH_TCAM_RULE_SIZE_ENT tcamRuleSize;

    pclMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    /* convert pcl rule's size to tcam rule's size */
    PRV_CPSS_DXCH_PCL_CONVERT_RULE_SIZE_TO_TCAM_RULE_SIZE_VAL_MAC(tcamRuleSize, ruleSize);

    ruleIndex = appDemoDxChTcamPclConvertedIndexGet(devNum,ruleIndex,tcamRuleSize);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChPclRuleGet(
            devNum, ruleSize, ruleIndex,
            maskArr, patternArr, actionArr);
    }
    else
    {
        return cpssDxChPclPortGroupRuleGet(
            devNum, pgBmp, ruleSize, ruleIndex,
            maskArr, patternArr, actionArr);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPclRuleParsedGet
(
    IN  GT_U8                                            devNum,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT               ruleFormat,
    IN  GT_U32                                           ruleIndex,
    IN  CPSS_DXCH_PCL_RULE_OPTION_ENT                    ruleOptionsBmp,
    OUT GT_BOOL                                          *isRuleValidPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *maskPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *patternPtr,
    OUT CPSS_DXCH_PCL_ACTION_STC                         *actionPtr
)
{
    if (wrCpssDxChVirtualTcam_useVTcamApi)
    {
        CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        ruleType;
        CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        ruleData;

        ruleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
        ruleType.rule.pcl.ruleFormat = ruleFormat;

        ruleData.rule.pcl.actionPtr = actionPtr;
        ruleData.rule.pcl.patternPtr = patternPtr;
        ruleData.rule.pcl.maskPtr = maskPtr;

        return cpssDxChVirtualTcamRuleRead(wrCpssDxChVirtualTcam_vTcamMngId,
                                            wrCpssDxChVirtualTcam_vTcamId,
                                            ruleIndex,
                                            &ruleType,
                                            &ruleData);
    }
    else
    {
        GT_BOOL             pgEnable; /* multi port group  enable */
        GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */
        CPSS_DXCH_TCAM_RULE_SIZE_ENT tcamRuleSize;

        pclMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

        /* convert rule's format to rule's size */
        PRV_CPSS_DXCH_PCL_CONVERT_RULE_FORMAT_TO_TCAM_RULE_SIZE_VAL_MAC(tcamRuleSize,ruleFormat);

        ruleIndex = appDemoDxChTcamPclConvertedIndexGet(devNum,ruleIndex,tcamRuleSize);

        if (pgEnable == GT_FALSE)
        {
            return cpssDxChPclRuleParsedGet(
                devNum, ruleFormat, ruleIndex, ruleOptionsBmp,
                isRuleValidPtr, maskPtr, patternPtr, actionPtr);
        }
        else
        {
            return cpssDxChPclPortGroupRuleParsedGet(
                devNum, pgBmp, ruleFormat, ruleIndex, ruleOptionsBmp,
                isRuleValidPtr, maskPtr, patternPtr, actionPtr);
        }
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPclCfgTblSet
(
    IN GT_U8                           devNum,
    IN CPSS_INTERFACE_INFO_STC        *interfaceInfoPtr,
    IN CPSS_PCL_DIRECTION_ENT          direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    IN CPSS_DXCH_PCL_LOOKUP_CFG_STC    *lookupCfgPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    pclMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChPclCfgTblSet(
            devNum, interfaceInfoPtr,
            direction, lookupNum, lookupCfgPtr);
    }
    else
    {
        return cpssDxChPclPortGroupCfgTblSet(
            devNum, pgBmp, interfaceInfoPtr,
            direction, lookupNum, lookupCfgPtr);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPclCfgTblGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_INTERFACE_INFO_STC        *interfaceInfoPtr,
    IN  CPSS_PCL_DIRECTION_ENT          direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    OUT CPSS_DXCH_PCL_LOOKUP_CFG_STC    *lookupCfgPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    pclMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChPclCfgTblGet(
            devNum, interfaceInfoPtr,
            direction, lookupNum, lookupCfgPtr);
    }
    else
    {
        return cpssDxChPclPortGroupCfgTblGet(
            devNum, pgBmp, interfaceInfoPtr,
            direction, lookupNum, lookupCfgPtr);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPclCfgTblEntryGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PCL_DIRECTION_ENT          direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    IN  GT_U32                          entryIndex,
    OUT GT_U32                          *pclCfgTblEntryPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    pclMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChPclCfgTblEntryGet(
            devNum, direction, lookupNum,
            entryIndex, pclCfgTblEntryPtr);
    }
    else
    {
        return cpssDxChPclPortGroupCfgTblEntryGet(
            devNum, pgBmp, direction, lookupNum,
            entryIndex, pclCfgTblEntryPtr);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPclTcpUdpPortComparatorSet
(
    IN GT_U8                             devNum,
    IN CPSS_PCL_DIRECTION_ENT            direction,
    IN CPSS_L4_PROTOCOL_ENT              l4Protocol,
    IN GT_U8                             entryIndex,
    IN CPSS_L4_PROTOCOL_PORT_TYPE_ENT    l4PortType,
    IN CPSS_COMPARE_OPERATOR_ENT         compareOperator,
    IN GT_U16                            value
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    pclMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxCh2PclTcpUdpPortComparatorSet(
            devNum, direction, l4Protocol,
            entryIndex, l4PortType, compareOperator, value);
    }
    else
    {
        return cpssDxChPclPortGroupTcpUdpPortComparatorSet(
            devNum, pgBmp, direction, l4Protocol,
            entryIndex, l4PortType, compareOperator, value);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPclTcpUdpPortComparatorGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_PCL_DIRECTION_ENT            direction,
    IN  CPSS_L4_PROTOCOL_ENT              l4Protocol,
    IN  GT_U8                             entryIndex,
    OUT CPSS_L4_PROTOCOL_PORT_TYPE_ENT    *l4PortTypePtr,
    OUT CPSS_COMPARE_OPERATOR_ENT         *compareOperatorPtr,
    OUT GT_U16                            *valuePtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    pclMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxCh2PclTcpUdpPortComparatorGet(
            devNum, direction, l4Protocol,
            entryIndex, l4PortTypePtr, compareOperatorPtr, valuePtr);
    }
    else
    {
        return cpssDxChPclPortGroupTcpUdpPortComparatorGet(
            devNum, pgBmp, direction, l4Protocol,
            entryIndex, l4PortTypePtr, compareOperatorPtr, valuePtr);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPclLookupCfgPortListEnableSet
(
    IN  GT_U8                         devNum,
    IN  CPSS_PCL_DIRECTION_ENT        direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT    lookupNum,
    IN  GT_U32                        subLookupNum,
    IN  GT_BOOL                       enable
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    pclMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChPclLookupCfgPortListEnableSet(
            devNum, direction, lookupNum,
            subLookupNum, enable);
    }
    else
    {
        return cpssDxChPclPortGroupLookupCfgPortListEnableSet(
            devNum, pgBmp, direction, lookupNum,
            subLookupNum, enable);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPclLookupCfgPortListEnableGet
(
    IN  GT_U8                         devNum,
    IN  CPSS_PCL_DIRECTION_ENT        direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT    lookupNum,
    IN  GT_U32                        subLookupNum,
    OUT GT_BOOL                       *enablePtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    pclMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChPclLookupCfgPortListEnableGet(
            devNum, direction, lookupNum,
            subLookupNum, enablePtr);
    }
    else
    {
        return cpssDxChPclPortGroupLookupCfgPortListEnableGet(
            devNum, pgBmp, direction, lookupNum,
            subLookupNum, enablePtr);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPclUserDefinedByteSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType,
    IN CPSS_PCL_DIRECTION_ENT               direction,
    IN GT_U32                               udbIndex,
    IN CPSS_DXCH_PCL_OFFSET_TYPE_ENT        offsetType,
    IN GT_U8                                offset
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    pclMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChPclUserDefinedByteSet(
            devNum, ruleFormat,
            packetType, direction,
            udbIndex, offsetType, offset);
    }
    else
    {
        return cpssDxChPclPortGroupUserDefinedByteSet(
            devNum, pgBmp, ruleFormat,
            packetType, direction,
            udbIndex, offsetType, offset);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPclUserDefinedByteGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType,
    IN  CPSS_PCL_DIRECTION_ENT               direction,
    IN  GT_U32                               udbIndex,
    OUT CPSS_DXCH_PCL_OFFSET_TYPE_ENT        *offsetTypePtr,
    OUT GT_U8                                *offsetPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    pclMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChPclUserDefinedByteGet(
            devNum, ruleFormat, packetType, direction,
            udbIndex, offsetTypePtr, offsetPtr);
    }
    else
    {
        return cpssDxChPclPortGroupUserDefinedByteGet(
            devNum, pgBmp, ruleFormat, packetType, direction,
            udbIndex, offsetTypePtr, offsetPtr);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN  CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_ENT  udbOverrideType,
    IN  GT_BOOL                              enable
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    pclMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChPclOverrideUserDefinedBytesEnableSet(
            devNum, ruleFormat, udbOverrideType, enable);
    }
    else
    {
        return cpssDxChPclPortGroupOverrideUserDefinedBytesEnableSet(
            devNum, pgBmp, ruleFormat, udbOverrideType, enable);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet
(
    IN  GT_U8                          devNum,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN  CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_ENT  udbOverrideType,
    OUT GT_BOOL                              *enablePtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    pclMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChPclOverrideUserDefinedBytesEnableGet(
            devNum, ruleFormat, udbOverrideType, enablePtr);
    }
    else
    {
        return cpssDxChPclPortGroupOverrideUserDefinedBytesEnableGet(
            devNum, pgBmp, ruleFormat, udbOverrideType, enablePtr);
    }
}

/* instead of obsolette function */
static GT_STATUS prvGaltisDxChPclOverrideUserDefinedBytesSet
(
    IN  GT_U8                          devNum,
    IN  PRV_DXCH_PCL_OVERRIDE_UDB_STC *udbOverridePtr,
    IN  GT_BOOL                        vplsMode
)
{
    GT_STATUS           rc;       /* return code              */

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        udbOverridePtr->vrfIdLsbEnableStdNotIp);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        udbOverridePtr->vrfIdMsbEnableStdNotIp);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        udbOverridePtr->vrfIdLsbEnableStdIpL2Qos);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        udbOverridePtr->vrfIdMsbEnableStdIpL2Qos);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        udbOverridePtr->vrfIdLsbEnableStdIpv4L4);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        udbOverridePtr->vrfIdMsbEnableStdIpv4L4);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        udbOverridePtr->vrfIdLsbEnableStdUdb);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        udbOverridePtr->vrfIdMsbEnableStdUdb);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        udbOverridePtr->vrfIdLsbEnableExtNotIpv6);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        udbOverridePtr->vrfIdMsbEnableExtNotIpv6);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        udbOverridePtr->vrfIdLsbEnableExtIpv6L2);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        udbOverridePtr->vrfIdMsbEnableExtIpv6L2);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        udbOverridePtr->vrfIdLsbEnableExtIpv6L4);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        udbOverridePtr->vrfIdMsbEnableExtIpv6L4);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        udbOverridePtr->vrfIdLsbEnableExtUdb);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        udbOverridePtr->vrfIdMsbEnableExtUdb);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_QOS_PROFILE_E,
        udbOverridePtr->qosProfileEnableStdUdb);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_QOS_PROFILE_E,
        udbOverridePtr->qosProfileEnableExtUdb);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (GT_TRUE == vplsMode)
    {
        rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
            devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
            CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MODE_E,
            udbOverridePtr->vrfIdLsbEnableStdNotIp);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/* instead of obsolette function */
static GT_STATUS prvGaltisDxChPclOverrideUserDefinedBytesGet
(
    IN   GT_U8                          devNum,
    OUT  PRV_DXCH_PCL_OVERRIDE_UDB_STC *udbOverridePtr,
    IN  GT_BOOL                         vplsMode
)
{
    GT_STATUS           rc;       /* return code              */

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        &(udbOverridePtr->vrfIdLsbEnableStdNotIp));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        &(udbOverridePtr->vrfIdMsbEnableStdNotIp));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        &(udbOverridePtr->vrfIdLsbEnableStdIpL2Qos));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        &(udbOverridePtr->vrfIdMsbEnableStdIpL2Qos));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        &(udbOverridePtr->vrfIdLsbEnableStdIpv4L4));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        &(udbOverridePtr->vrfIdMsbEnableStdIpv4L4));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        &(udbOverridePtr->vrfIdLsbEnableStdUdb));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        &(udbOverridePtr->vrfIdMsbEnableStdUdb));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        &(udbOverridePtr->vrfIdLsbEnableExtNotIpv6));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        &(udbOverridePtr->vrfIdMsbEnableExtNotIpv6));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        &(udbOverridePtr->vrfIdLsbEnableExtIpv6L2));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        &(udbOverridePtr->vrfIdMsbEnableExtIpv6L2));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        &(udbOverridePtr->vrfIdLsbEnableExtIpv6L4));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        &(udbOverridePtr->vrfIdMsbEnableExtIpv6L4));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,
        &(udbOverridePtr->vrfIdLsbEnableExtUdb));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,
        &(udbOverridePtr->vrfIdMsbEnableExtUdb));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_QOS_PROFILE_E,
        &(udbOverridePtr->qosProfileEnableStdUdb));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_QOS_PROFILE_E,
        &(udbOverridePtr->qosProfileEnableExtUdb));
    if (rc != GT_OK)
    {
        return rc;
    }

    if (GT_TRUE == vplsMode)
    {
        rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
            devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
            CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MODE_E,
            &(udbOverridePtr->vrfIdLsbEnableStdNotIp));
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/* instead of obsolette function */
static GT_STATUS prvGaltisDxChPclOverrideUserDefinedBytesByTrunkHashSet
(
    IN  GT_U8                                     devNum,
    IN  PRV_DXCH_PCL_OVERRIDE_UDB_TRUNK_HASH_STC *udbOverTrunkHashPtr
)
{
    GT_STATUS           rc;       /* return code              */

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,
        udbOverTrunkHashPtr->trunkHashEnableStdNotIp);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,
        udbOverTrunkHashPtr->trunkHashEnableStdIpv4L4);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,
        udbOverTrunkHashPtr->trunkHashEnableStdUdb);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,
        udbOverTrunkHashPtr->trunkHashEnableExtNotIpv6);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,
        udbOverTrunkHashPtr->trunkHashEnableExtIpv6L2);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,
        udbOverTrunkHashPtr->trunkHashEnableExtIpv6L4);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableSet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,
        udbOverTrunkHashPtr->trunkHashEnableExtUdb);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/* instead of obsolette function */
static GT_STATUS prvGaltisDxChPclOverrideUserDefinedBytesByTrunkHashGet
(
    IN  GT_U8                                     devNum,
    OUT PRV_DXCH_PCL_OVERRIDE_UDB_TRUNK_HASH_STC *udbOverTrunkHashPtr
)
{
    GT_STATUS           rc;       /* return code              */

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,
        &(udbOverTrunkHashPtr->trunkHashEnableStdNotIp));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,
        &(udbOverTrunkHashPtr->trunkHashEnableStdIpv4L4));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,
        &(udbOverTrunkHashPtr->trunkHashEnableStdUdb));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,
        &(udbOverTrunkHashPtr->trunkHashEnableExtNotIpv6));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,
        &(udbOverTrunkHashPtr->trunkHashEnableExtIpv6L2));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,
        &(udbOverTrunkHashPtr->trunkHashEnableExtIpv6L4));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = pg_wrap_cpssDxChPclOverrideUserDefinedBytesEnableGet(
        devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
        CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,
        &(udbOverTrunkHashPtr->trunkHashEnableExtUdb));
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPclEgressKeyFieldsVidUpModeSet
(
    IN  GT_U8                                      devNum,
    IN  CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_MODE_ENT   vidUpMode
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    pclMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChPclEgressKeyFieldsVidUpModeSet(
            devNum, vidUpMode);
    }
    else
    {
        return cpssDxChPclPortGroupEgressKeyFieldsVidUpModeSet(
            devNum, pgBmp, vidUpMode);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPclEgressKeyFieldsVidUpModeGet
(
    IN   GT_U8                                      devNum,
    OUT  CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_MODE_ENT   *vidUpModePtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    pclMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChPclEgressKeyFieldsVidUpModeGet(
            devNum, vidUpModePtr);
    }
    else
    {
        return cpssDxChPclPortGroupEgressKeyFieldsVidUpModeGet(
            devNum, pgBmp, vidUpModePtr);
    }
}

/**
* @internal wrCpssDxChPclInit function
* @endinternal
*
* @brief   The function initializes the device for following configuration
*         and using Policy engine
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*
* @note NONE
*
*/
static CMD_STATUS wrCpssDxChPclInit

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8    devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPclInit(devNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}



/**
* @internal prvCpssDxChPclUDBIndexBaseGet function
* @endinternal
*
* @brief   The function gets UDB index base by the key format
*
* @note   APPLICABLE DEVICES:      All DxCh devices
* @param[in] ruleFormat               - rule format
*
* @param[out] indexBasePtr             - (pointer to) UDB index base
*
* @retval GT_OK                    -   on success
* @retval GT_BAD_PARAM             -   otherwise
*/
static CMD_STATUS prvCpssDxChPclUDBIndexBaseGet
(
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat,
    OUT GT_U32                                *indexBasePtr
)
{
    switch (ruleFormat)
    {
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
            *indexBasePtr = 15;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
            *indexBasePtr = 18;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
            *indexBasePtr = 20;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
            *indexBasePtr = 0;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
            *indexBasePtr = 0;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
            *indexBasePtr = 6;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
            *indexBasePtr = 12;
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
            *indexBasePtr = 0;
            break;
        default: return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal wrCpssDxChPclUserDefinedByteSet function
* @endinternal
*
* @brief   The function configures the User Defined Byte (UDB)
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
*                                       GT_FAIL   otherwise
*
* @note See comments to CPSS_DXCH_PCL_UDB_CFG_STC
*
*/
static CMD_STATUS wrCpssDxChPclUserDefinedByteSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                             result;
    GT_U32                                i;
    GT_U8                                 devNum;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;
    GT_U32                                udbIndex;
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT         offsetType;
    GT_U8                                 offset;
    GT_U32                                udbIndexBase;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ruleFormat = (CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT)inArgs[1];
    udbIndex = (GT_U32)inArgs[2];
    offsetType = (CPSS_DXCH_PCL_OFFSET_TYPE_ENT)inArgs[3];
    offset = (GT_U8)inArgs[4];

    if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* Ch1-3                  */
        /* call cpss api function */
        result = pg_wrap_cpssDxChPclUserDefinedByteSet(
            devNum, ruleFormat,
            /* for DxCh1-3 parameter ignored     */
            /* for DxChXCat - another command    */
            CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
            CPSS_PCL_DIRECTION_INGRESS_E,
            udbIndex, offsetType, offset);
    }
    else
    {
        /* XCatA1 and above */

        result = prvCpssDxChPclUDBIndexBaseGet(
            ruleFormat, &udbIndexBase);
        if (result != GT_OK)
        {
            /* pack output arguments to galtis string */
            galtisOutput(outArgs, result, "");
            return CMD_OK;
        }

        /* configure UDB in all 8 templates */
        for (i = 8; (i > 0); i--)
        {
            result = pg_wrap_cpssDxChPclUserDefinedByteSet(
                devNum, ruleFormat,
                (CPSS_DXCH_PCL_PACKET_TYPE_ENT)(i - 1),
                CPSS_PCL_DIRECTION_INGRESS_E,
                (udbIndexBase + udbIndex), offsetType, offset);
        }
    }


    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPclUserDefinedByteGet function
* @endinternal
*
* @brief   The function gets the User Defined Byte configuration
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
*                                       GT_FAIL   otherwise
*
* @note See comments to CPSS_DXCH_PCL_UDB_CFG_STC
*
*/
static CMD_STATUS wrCpssDxChPclUserDefinedByteGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                             result;
    GT_U8                                 devNum;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT         pktType;
    GT_U32                                udbIndex;
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT         offsetType;
    GT_U8                                 offset;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[0];
    ruleFormat = (CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT)inArgs[1];
    pktType    = (CPSS_DXCH_PCL_PACKET_TYPE_ENT)inArgs[2];
    udbIndex   = (GT_U32)inArgs[3];
    offsetType = 0;
    offset     = 0;

    /* call cpss api function */
    result = pg_wrap_cpssDxChPclUserDefinedByteGet(
        devNum, ruleFormat, pktType, CPSS_PCL_DIRECTION_INGRESS_E,
        udbIndex, &offsetType, &offset);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", offsetType, offset);
    return CMD_OK;
}

/* Table: cpssDxChPclRule (Union Table)
*
* Description:
*     PCL Configuration table
*
* Fields:
*       maskData          - rule mask. The rule mask is AND styled one. Mask
*                          bit's 0 means don't care bit (corresponding bit in
*                          the pattern is not using in the TCAM lookup).
*                          Mask bit's 1 means that corresponding bit in the
*                          pattern is using in the TCAM lookup.
*                          The format of mask is defined by ruleFormat
*
*       patternData       - rule pattern.
*                          The format of pattern is defined by ruleFormat
*
*       actionPtr        - Policy rule action that applied on packet if packet's
*                          search key matched with masked pattern.
*
*
*       See include file cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h for more details.
*
*
* Comments:
*
*
*
*/


/* table cpssDxChPclRule global variables */

static    CPSS_DXCH_PCL_RULE_FORMAT_UNT      maskData;
static    CPSS_DXCH_PCL_RULE_FORMAT_UNT      patternData;
static    CPSS_DXCH_PCL_ACTION_STC           actionData;
static    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
/* last action stored by pg_wrap_cpssDxChPclRuleSet */
static    CPSS_DXCH_PCL_ACTION_STC           *lastActionPtr;

/* only words 10..20 should be used */
/* use GT_32 to be with same type as --> GT_32 inFields[CMD_MAX_FIELDS] */
static GT_UINTPTR   ruleDataWords[50];

/**
* @internal actionRetrieve function
* @endinternal
*
* @brief   Retrueves the RULE Action data from the table fields
*
* @param[in] inputFields[]            - source input fields array
* @param[in] baseFieldIndex           - index of first field related to the action
*
* @param[out] actionPtr                - he retrieved CPSS action
*                                       None
*
* @note NONE
*
*/
static void actionRetrieve

(
    IN  GT_U8                       devNum,
    IN  GT_UINTPTR                  inputFields[],
    IN  GT_32                       baseFieldIndex,
    OUT CPSS_DXCH_PCL_ACTION_STC    *actionPtr
)
{
    GT_UINTPTR *inFields;

    inFields = &(inputFields[baseFieldIndex]);

    /* retrieve command from Galtis Drop Down List */
    switch (inFields[0])
    {
        case 0:
            actionPtr->pktCmd = CPSS_PACKET_CMD_FORWARD_E;
            break;
        case 1:
            actionPtr->pktCmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
            break;
        case 2:
            actionPtr->pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            break;
        case 3:
            actionPtr->pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;
            break;
        case 4:
            actionPtr->pktCmd = CPSS_PACKET_CMD_DROP_SOFT_E;
            break;
        default:
            actionPtr->pktCmd = (CPSS_PACKET_CMD_ENT)0xFF;
            cmdOsPrintf(" *** WRONG PACKET COMMAND ***\n");
            break;
    }

    actionPtr->mirror.cpuCode = (GT_U32)inFields[1];
    actionPtr->mirror.mirrorToRxAnalyzerPort = (GT_BOOL)inFields[2];
    actionPtr->matchCounter.enableMatchCount = (GT_BOOL)inFields[3];
    actionPtr->matchCounter.matchCounterIndex = (GT_U32)inFields[4];
    actionPtr->egressPolicy = (GT_BOOL)inFields[5];

    switch (actionPtr->egressPolicy)
    {
        case GT_FALSE:
            actionPtr->qos.ingress.modifyDscp =
                            (CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[6];
            actionPtr->qos.ingress.modifyUp =
                            (CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[7];
            actionPtr->qos.ingress.profileIndex = (GT_U32)inFields[8];
            actionPtr->qos.ingress.profileAssignIndex = (GT_BOOL)inFields[9];
            actionPtr->qos.ingress.profilePrecedence =
                      (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[10];
        break;

        case GT_TRUE:
            actionPtr->qos.egress.dscp = (GT_U8)inFields[11];
            actionPtr->qos.egress.up = (GT_U8)inFields[12];
        break;

    default:
        break;
    }

    actionPtr->redirect.redirectCmd =
                            (CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ENT)inFields[13];

    if (inFields[13] == 1)
    {
        actionPtr->redirect.data.outIf.outInterface.type =
                                          (CPSS_INTERFACE_TYPE_ENT)inFields[14];

        switch (inFields[14])
        {
        case 0:
            actionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum =
                                                            (GT_HW_DEV_NUM)inFields[15];
            actionPtr->redirect.data.outIf.outInterface.devPort.portNum =
                                                            (GT_PORT_NUM)inFields[16];

            CONVERT_DEV_PORT_DATA_MAC(actionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum,
                                 actionPtr->redirect.data.outIf.outInterface.devPort.portNum);

            break;

        case 1:
            actionPtr->redirect.data.outIf.outInterface.trunkId =
                                                      (GT_TRUNK_ID)inFields[17];
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(actionPtr->redirect.data.outIf.outInterface.trunkId);
            break;

        case 2:
            actionPtr->redirect.data.outIf.outInterface.vidx =
                                                           (GT_U16)inFields[18];
            break;

        case 3:
            actionPtr->redirect.data.outIf.outInterface.vlanId =
                                                           (GT_U16)inFields[19];
            break;

        default:
            break;
        }

        actionPtr->redirect.data.outIf.vntL2Echo = (GT_BOOL)inFields[20];
        actionPtr->redirect.data.outIf.tunnelStart = (GT_BOOL)inFields[21];
        actionPtr->redirect.data.outIf.tunnelPtr = (GT_U32)inFields[22];
    }

    if (inFields[13] == 2)
    {
        actionPtr->redirect.data.routerLttIndex = (GT_U32)inFields[23];
    }

    actionPtr->policer.policerEnable = (GT_BOOL)inFields[24];
    actionPtr->policer.policerId = (GT_U32)inFields[25];

    /*actionPtr->vlan.egressTaggedModify = (GT_BOOL)inFields[26];*/

    switch (actionPtr->egressPolicy)
    {
        case GT_FALSE:
            actionPtr->vlan.ingress.modifyVlan =
                             (CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT)inFields[27];
            actionPtr->vlan.ingress.nestedVlan = (GT_BOOL)inFields[28];
            actionPtr->vlan.ingress.vlanId = (GT_U16)inFields[29];
            actionPtr->vlan.ingress.precedence =
                      (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[30];
            break;

        case GT_TRUE:
            actionPtr->vlan.egress.vlanCmd =
                ((inFields[27] == CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E)
                 || (inFields[27] == CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_UNTAGGED_E))
                ? CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E
                : CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
            actionPtr->vlan.egress.vlanId = (GT_U16)inFields[29];
            break;

        default:
            break;
    }

    actionPtr->ipUcRoute.doIpUcRoute = (GT_BOOL)inFields[31];
    actionPtr->ipUcRoute.arpDaIndex = (GT_U32)inFields[32];
    actionPtr->ipUcRoute.decrementTTL = (GT_BOOL)inFields[33];
    actionPtr->ipUcRoute.bypassTTLCheck = (GT_BOOL)inFields[34];
    actionPtr->ipUcRoute.icmpRedirectCheck = (GT_BOOL)inFields[35];


    /* The bypass Bridge or Bypass Ingress pipe should be set for redirect to Outlif action
       for SIP5 devices. For other devices CPSS configures flags by itself. */
    if ((actionPtr->redirect.redirectCmd == CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E) &&
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
    {
        actionPtr->bypassBridge = GT_TRUE;
    }
}

/**
* @internal actionRetrieve_1 function
* @endinternal
*
* @brief   Retrueves the RULE Action data from the table fields
*
* @param[in] inputFields[]            - source input fields array
* @param[in] baseFieldIndex           - index of first field related to the action
*
* @param[out] actionPtr                - he retrieved CPSS action
*                                       None
*
* @note NONE
*
*/
static void actionRetrieve_1

(
    IN  GT_U8                       devNum,
    IN  GT_UINTPTR                  inputFields[],
    IN  GT_32                       baseFieldIndex,
    OUT CPSS_DXCH_PCL_ACTION_STC    *actionPtr
)
{
    GT_UINTPTR *inFields;

    inFields = &(inputFields[baseFieldIndex]);

    /* retrieve command from Galtis Drop Down List */
    switch (inFields[0])
    {
        case 0:
            actionPtr->pktCmd = CPSS_PACKET_CMD_FORWARD_E;
            break;
        case 1:
            actionPtr->pktCmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
            break;
        case 2:
            actionPtr->pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            break;
        case 3:
            actionPtr->pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;
            break;
        case 4:
            actionPtr->pktCmd = CPSS_PACKET_CMD_DROP_SOFT_E;
            break;
        default:
            actionPtr->pktCmd = (CPSS_PACKET_CMD_ENT)0xFF;
            cmdOsPrintf(" *** WRONG PACKET COMMAND ***\n");
            break;
    }

    actionPtr->mirror.cpuCode = (GT_U32)inFields[1];
    actionPtr->mirror.mirrorToRxAnalyzerPort = (GT_BOOL)inFields[2];
    actionPtr->matchCounter.enableMatchCount = (GT_BOOL)inFields[3];
    actionPtr->matchCounter.matchCounterIndex = (GT_U32)inFields[4];
    actionPtr->egressPolicy = (GT_BOOL)inFields[5];

    switch (actionPtr->egressPolicy)
    {
        case GT_FALSE:
            actionPtr->qos.ingress.modifyDscp =
                            (CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[6];
            actionPtr->qos.ingress.modifyUp =
                            (CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[7];
            actionPtr->qos.ingress.profileIndex = (GT_U32)inFields[8];
            actionPtr->qos.ingress.profileAssignIndex = (GT_BOOL)inFields[9];
            actionPtr->qos.ingress.profilePrecedence =
                      (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[10];
        break;

        case GT_TRUE:
            actionPtr->qos.egress.dscp = (GT_U8)inFields[11];
            actionPtr->qos.egress.up = (GT_U8)inFields[12];
        break;

    default:
        break;
    }

    actionPtr->redirect.redirectCmd =
                            (CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ENT)inFields[13];

    if (PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_ENABLED_MAC(devNum) != GT_FALSE)
    {
        if (actionPtr->redirect.redirectCmd ==
            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E)
        {
            /* Galtis GUI for xCat VPLS ommites the VIRT_ROUTER value */
            actionPtr->redirect.redirectCmd =
                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_LOGICAL_PORT_ASSIGN_E;
        }
    }

    if (inFields[13] == 1)
    {
        actionPtr->redirect.data.outIf.outInterface.type =
                                          (CPSS_INTERFACE_TYPE_ENT)inFields[14];

        switch (inFields[14])
        {
        case 0:
            actionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum =
                                                            (GT_HW_DEV_NUM)inFields[15];
            actionPtr->redirect.data.outIf.outInterface.devPort.portNum =
                                                            (GT_PORT_NUM)inFields[16];

            CONVERT_DEV_PORT_DATA_MAC(actionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum,
                                 actionPtr->redirect.data.outIf.outInterface.devPort.portNum);

            break;

        case 1:
            actionPtr->redirect.data.outIf.outInterface.trunkId =
                                                      (GT_TRUNK_ID)inFields[17];
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(actionPtr->redirect.data.outIf.outInterface.trunkId);
            break;

        case 2:
            actionPtr->redirect.data.outIf.outInterface.vidx =
                                                           (GT_U16)inFields[18];
            break;

        case 3:
            actionPtr->redirect.data.outIf.outInterface.vlanId =
                                                           (GT_U16)inFields[19];
            break;

        default:
            break;
        }

        actionPtr->redirect.data.outIf.vntL2Echo = (GT_BOOL)inFields[20];
        actionPtr->redirect.data.outIf.tunnelStart = (GT_BOOL)inFields[21];
        actionPtr->redirect.data.outIf.tunnelPtr = (GT_U32)inFields[22];
    }

    if (inFields[13] == 2)
    {
        actionPtr->redirect.data.routerLttIndex = (GT_U32)inFields[23];
    }

    if (inFields[13] == 5)
    {
        actionPtr->redirect.data.logicalSourceInterface.logicalInterface.type =
                                          (CPSS_INTERFACE_TYPE_ENT)inFields[24];
        switch (inFields[24])
        {
        case 0:
            actionPtr->redirect.data.logicalSourceInterface.logicalInterface.devPort.hwDevNum =
                                                            (GT_HW_DEV_NUM)inFields[25];
            actionPtr->redirect.data.logicalSourceInterface.logicalInterface.devPort.portNum =
                                                            (GT_PORT_NUM)inFields[26];

            CONVERT_DEV_PORT_DATA_MAC(actionPtr->redirect.data.logicalSourceInterface.logicalInterface.devPort.hwDevNum,
                                 actionPtr->redirect.data.logicalSourceInterface.logicalInterface.devPort.portNum);

            break;

        default:
            break;
        }

        actionPtr->redirect.data.logicalSourceInterface.sourceMeshIdSetEnable = (GT_BOOL)inFields[27];
        actionPtr->redirect.data.logicalSourceInterface.sourceMeshId = (GT_U32)inFields[28];
        actionPtr->redirect.data.logicalSourceInterface.userTagAcEnable = (GT_BOOL)inFields[29];

    }

    actionPtr->policer.policerEnable = (GT_BOOL)inFields[30];
    actionPtr->policer.policerId = (GT_U32)inFields[31];

    /*actionPtr->vlan.egressTaggedModify = (GT_BOOL)inFields[32];*/

    switch (actionPtr->egressPolicy)
    {
        case GT_FALSE:
            actionPtr->vlan.ingress.modifyVlan =
                             (CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT)inFields[33];
            actionPtr->vlan.ingress.nestedVlan = (GT_BOOL)inFields[34];
            actionPtr->vlan.ingress.vlanId = (GT_U16)inFields[35];
            actionPtr->vlan.ingress.precedence =
                      (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[36];
            break;

        case GT_TRUE:
            actionPtr->vlan.egress.vlanCmd =
                ((inFields[33] == CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E)
                 || (inFields[33] == CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_UNTAGGED_E))
                ? CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E
                : CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
            actionPtr->vlan.egress.vlanId = (GT_U16)inFields[35];
            break;

        default:
            break;
    }

    actionPtr->ipUcRoute.doIpUcRoute = (GT_BOOL)inFields[37];
    actionPtr->ipUcRoute.arpDaIndex = (GT_U32)inFields[38];
    actionPtr->ipUcRoute.decrementTTL = (GT_BOOL)inFields[39];
    actionPtr->ipUcRoute.bypassTTLCheck = (GT_BOOL)inFields[40];
    actionPtr->ipUcRoute.icmpRedirectCheck = (GT_BOOL)inFields[41];


    /* The bypass Bridge or Bypass Ingress pipe should be set for redirect to Outlif action
       for SIP5 devices. For other devices CPSS configures flags by itself. */
    if ((actionPtr->redirect.redirectCmd == CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E) &&
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
    {
        actionPtr->bypassBridge = GT_TRUE;
    }
}

/**
* @internal wrCpssDxChPclRuleSet_STD_NOT_IP function
* @endinternal
*
* @brief   The function sets the Policy Rule Mask, Pattern and Action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*
* @note NONE
*
*/
static CMD_STATUS wrCpssDxChPclRuleSet_STD_NOT_IP
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    IN  GT_BOOL vplsFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                          result;
    GT_U8                              devNum;
    GT_U32                             ruleIndex;
    GT_BYTE_ARRY                       maskBArr, patternBArr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    ruleIndex = (GT_U32)inFields[0];
    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    cmdOsMemCpy(
        &maskData.ruleStdNotIp.common.portListBmp,
        &ruleSet_portListBmpMask,
        sizeof(CPSS_PORTS_BMP_STC));

    maskData.ruleStdNotIp.common.pclId = (GT_U16)inFields[2];
    CONVERT_PCL_MASK_PCL_ID_TEST_TO_CPSS_MAC(maskData.ruleStdNotIp.common.pclId);
    maskData.ruleStdNotIp.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[3];
    maskData.ruleStdNotIp.common.isTagged = (GT_U8)inFields[4];
    maskData.ruleStdNotIp.common.vid = (GT_U16)inFields[5];
    maskData.ruleStdNotIp.common.up = (GT_U8)inFields[6];
    maskData.ruleStdNotIp.common.qosProfile = (GT_U8)inFields[7];
    maskData.ruleStdNotIp.common.isIp = (GT_U8)inFields[8];
    maskData.ruleStdNotIp.common.isL2Valid = (GT_U8)inFields[9];
    maskData.ruleStdNotIp.common.isUdbValid = (GT_U8)inFields[10];
    maskData.ruleStdNotIp.isIpv4 = (GT_U8)inFields[11];
    maskData.ruleStdNotIp.etherType = (GT_U16)inFields[12];
    maskData.ruleStdNotIp.isArp = (GT_BOOL)inFields[13];
    maskData.ruleStdNotIp.l2Encap = (GT_BOOL)inFields[14];
    galtisMacAddr(&maskData.ruleStdNotIp.macDa, (GT_U8*)inFields[15]);
    galtisMacAddr(&maskData.ruleStdNotIp.macSa, (GT_U8*)inFields[16]);
    galtisBArray(&maskBArr,(GT_U8*)inFields[17]);

    cmdOsMemCpy(
        maskData.ruleStdNotIp.udb15_17, maskBArr.data,
        MIN(3, maskBArr.length));

    cmdOsMemCpy(
        &patternData.ruleStdNotIp.common.portListBmp,
        &ruleSet_portListBmpPattern,
        sizeof(CPSS_PORTS_BMP_STC));

    patternData.ruleStdNotIp.common.pclId = (GT_U16)inFields[18];
    patternData.ruleStdNotIp.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[19];
    patternData.ruleStdNotIp.common.isTagged = (GT_U8)inFields[20];
    patternData.ruleStdNotIp.common.vid = (GT_U16)inFields[21];
    patternData.ruleStdNotIp.common.up = (GT_U8)inFields[22];
    patternData.ruleStdNotIp.common.qosProfile = (GT_U8)inFields[23];
    patternData.ruleStdNotIp.common.isIp = (GT_U8)inFields[24];
    patternData.ruleStdNotIp.common.isL2Valid = (GT_U8)inFields[25];
    patternData.ruleStdNotIp.common.isUdbValid = (GT_U8)inFields[26];
    patternData.ruleStdNotIp.isIpv4 = (GT_U8)inFields[27];
    patternData.ruleStdNotIp.etherType = (GT_U16)inFields[28];
    patternData.ruleStdNotIp.isArp = (GT_BOOL)inFields[29];
    patternData.ruleStdNotIp.l2Encap = (GT_BOOL)inFields[30];
    galtisMacAddr(&patternData.ruleStdNotIp.macDa, (GT_U8*)inFields[31]);
    galtisMacAddr(&patternData.ruleStdNotIp.macSa, (GT_U8*)inFields[32]);
    galtisBArray(&patternBArr,(GT_U8*)inFields[33]);

    cmdOsMemCpy(
        patternData.ruleStdNotIp.udb15_17, patternBArr.data,
        MIN(3, patternBArr.length));

    if (GT_TRUE == vplsFields)
    {
        actionRetrieve_1(devNum, inFields, 34, &actionData);
    }
    else
    {
        actionRetrieve(devNum, inFields, 34, &actionData);
    }

    PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleStdNotIp.common.sourcePort);

    lastActionPtr = &actionData;

    /* call cpss api function */
    result = pg_wrap_cpssDxChPclRuleSet(devNum, ruleFormat, ruleIndex, &maskData,
                                                &patternData, &actionData);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/******************************************************************************/
static CMD_STATUS wrCpssDxChPclRuleSet_STD_IP_L2_QOS

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    IN  GT_BOOL vplsFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                          result;
    GT_U8                              devNum;
    GT_U32                             ruleIndex;
    GT_BYTE_ARRY                       maskBArr, patternBArr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    ruleIndex = (GT_U32)inFields[0];
    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    cmdOsMemCpy(
        &maskData.ruleStdIpL2Qos.common.portListBmp,
        &ruleSet_portListBmpMask,
        sizeof(CPSS_PORTS_BMP_STC));

    maskData.ruleStdIpL2Qos.common.pclId = (GT_U16)inFields[2];
    CONVERT_PCL_MASK_PCL_ID_TEST_TO_CPSS_MAC(maskData.ruleStdIpL2Qos.common.pclId);
    maskData.ruleStdIpL2Qos.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[3];
    maskData.ruleStdIpL2Qos.common.isTagged = (GT_U8)inFields[4];
    maskData.ruleStdIpL2Qos.common.vid = (GT_U16)inFields[5];
    maskData.ruleStdIpL2Qos.common.up = (GT_U8)inFields[6];
    maskData.ruleStdIpL2Qos.common.qosProfile = (GT_U8)inFields[7];
    maskData.ruleStdIpL2Qos.common.isIp = (GT_U8)inFields[8];
    maskData.ruleStdIpL2Qos.common.isL2Valid = (GT_U8)inFields[9];
    maskData.ruleStdIpL2Qos.common.isUdbValid = (GT_U8)inFields[10];
    maskData.ruleStdIpL2Qos.commonStdIp.isIpv4 = (GT_U8)inFields[11];
    maskData.ruleStdIpL2Qos.commonStdIp.ipProtocol = (GT_U8)inFields[12];
    maskData.ruleStdIpL2Qos.commonStdIp.dscp = (GT_U8)inFields[13];
    maskData.ruleStdIpL2Qos.commonStdIp.isL4Valid = (GT_U8)inFields[14];
    maskData.ruleStdIpL2Qos.commonStdIp.l4Byte2 = (GT_U8)inFields[15];
    maskData.ruleStdIpL2Qos.commonStdIp.l4Byte3 = (GT_U8)inFields[16];
    maskData.ruleStdIpL2Qos.commonStdIp.ipHeaderOk = (GT_U8)inFields[17];
    maskData.ruleStdIpL2Qos.commonStdIp.ipv4Fragmented = (GT_U8)inFields[18];
    maskData.ruleStdIpL2Qos.isArp = (GT_BOOL)inFields[19];
    maskData.ruleStdIpL2Qos.isIpv6ExtHdrExist = (GT_BOOL)inFields[20];
    maskData.ruleStdIpL2Qos.isIpv6HopByHop = (GT_BOOL)inFields[21];
    galtisMacAddr(&maskData.ruleStdIpL2Qos.macDa, (GT_U8*)inFields[22]);
    galtisMacAddr(&maskData.ruleStdIpL2Qos.macSa, (GT_U8*)inFields[23]);
    galtisBArray(&maskBArr,(GT_U8*)inFields[24]);

    cmdOsMemCpy(
        maskData.ruleStdIpL2Qos.udb18_19, maskBArr.data,
        MIN(2, maskBArr.length));

    cmdOsMemCpy(
        &patternData.ruleStdIpL2Qos.common.portListBmp,
        &ruleSet_portListBmpPattern,
        sizeof(CPSS_PORTS_BMP_STC));

    patternData.ruleStdIpL2Qos.common.pclId = (GT_U16)inFields[25];
    patternData.ruleStdIpL2Qos.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[26];
    patternData.ruleStdIpL2Qos.common.isTagged = (GT_U8)inFields[27];
    patternData.ruleStdIpL2Qos.common.vid = (GT_U16)inFields[28];
    patternData.ruleStdIpL2Qos.common.up = (GT_U8)inFields[29];
    patternData.ruleStdIpL2Qos.common.qosProfile = (GT_U8)inFields[30];
    patternData.ruleStdIpL2Qos.common.isIp = (GT_U8)inFields[31];
    patternData.ruleStdIpL2Qos.common.isL2Valid = (GT_U8)inFields[32];
    patternData.ruleStdIpL2Qos.common.isUdbValid = (GT_U8)inFields[33];
    patternData.ruleStdIpL2Qos.commonStdIp.isIpv4 = (GT_U8)inFields[34];
    patternData.ruleStdIpL2Qos.commonStdIp.ipProtocol = (GT_U8)inFields[35];
    patternData.ruleStdIpL2Qos.commonStdIp.dscp = (GT_U8)inFields[36];
    patternData.ruleStdIpL2Qos.commonStdIp.isL4Valid = (GT_U8)inFields[37];
    patternData.ruleStdIpL2Qos.commonStdIp.l4Byte2 = (GT_U8)inFields[38];
    patternData.ruleStdIpL2Qos.commonStdIp.l4Byte3 = (GT_U8)inFields[39];
    patternData.ruleStdIpL2Qos.commonStdIp.ipHeaderOk = (GT_U8)inFields[40];
    patternData.ruleStdIpL2Qos.commonStdIp.ipv4Fragmented = (GT_U8)inFields[41];
    patternData.ruleStdIpL2Qos.isArp = (GT_BOOL)inFields[42];
    patternData.ruleStdIpL2Qos.isIpv6ExtHdrExist = (GT_BOOL)inFields[43];
    patternData.ruleStdIpL2Qos.isIpv6HopByHop = (GT_BOOL)inFields[44];
    galtisMacAddr(&patternData.ruleStdIpL2Qos.macDa, (GT_U8*)inFields[45]);
    galtisMacAddr(&patternData.ruleStdIpL2Qos.macSa, (GT_U8*)inFields[46]);
    galtisBArray(&patternBArr,(GT_U8*)inFields[47]);

    cmdOsMemCpy(
        patternData.ruleStdIpL2Qos.udb18_19, patternBArr.data,
        MIN(2, patternBArr.length));

    if (GT_TRUE == vplsFields)
    {
        actionRetrieve_1(devNum, inFields, 48, &actionData);
    }
    else
    {
        actionRetrieve(devNum, inFields, 48, &actionData);
    }

    PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleStdIpL2Qos.common.sourcePort);

    lastActionPtr = &actionData;

    /* call cpss api function */
    result = pg_wrap_cpssDxChPclRuleSet(devNum, ruleFormat, ruleIndex, &maskData,
                                                &patternData, &actionData);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/******************************************************************************/
static CMD_STATUS wrCpssDxChPclRuleSet_EXT_NOT_IPV6

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    IN  GT_BOOL vplsFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                          result;
    GT_U8                              devNum;
    GT_U32                             ruleIndex;
    GT_BYTE_ARRY                       maskBArr, patternBArr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    ruleIndex = (GT_U32)inFields[0];
    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E;

    cmdOsMemCpy(
        &maskData.ruleExtNotIpv6.common.portListBmp,
        &ruleSet_portListBmpMask,
        sizeof(CPSS_PORTS_BMP_STC));

    maskData.ruleExtNotIpv6.common.pclId = (GT_U16)inFields[2];
    maskData.ruleExtNotIpv6.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[3];
    maskData.ruleExtNotIpv6.common.isTagged = (GT_U8)inFields[4];
    maskData.ruleExtNotIpv6.common.vid = (GT_U16)inFields[5];
    maskData.ruleExtNotIpv6.common.up = (GT_U8)inFields[6];
    maskData.ruleExtNotIpv6.common.qosProfile = (GT_U8)inFields[7];
    maskData.ruleExtNotIpv6.common.isIp = (GT_U8)inFields[8];
    maskData.ruleExtNotIpv6.common.isL2Valid = (GT_U8)inFields[9];
    maskData.ruleExtNotIpv6.common.isUdbValid = (GT_U8)inFields[10];
    maskData.ruleExtNotIpv6.commonExt.isIpv6 = (GT_U8)inFields[11];
    maskData.ruleExtNotIpv6.commonExt.ipProtocol = (GT_U8)inFields[12];
    maskData.ruleExtNotIpv6.commonExt.dscp = (GT_U8)inFields[13];
    maskData.ruleExtNotIpv6.commonExt.isL4Valid = (GT_U8)inFields[14];
    maskData.ruleExtNotIpv6.commonExt.l4Byte0 = (GT_U8)inFields[15];
    maskData.ruleExtNotIpv6.commonExt.l4Byte1 = (GT_U8)inFields[16];
    maskData.ruleExtNotIpv6.commonExt.l4Byte2 = (GT_U8)inFields[17];
    maskData.ruleExtNotIpv6.commonExt.l4Byte3 = (GT_U8)inFields[18];
    maskData.ruleExtNotIpv6.commonExt.l4Byte13 = (GT_U8)inFields[19];
    maskData.ruleExtNotIpv6.commonExt.ipHeaderOk = (GT_U8)inFields[20];
    galtisIpAddr(&maskData.ruleExtNotIpv6.sip, (GT_U8*)inFields[21]);
    galtisIpAddr(&maskData.ruleExtNotIpv6.dip, (GT_U8*)inFields[22]);
    maskData.ruleExtNotIpv6.etherType = (GT_U16)inFields[23];
    maskData.ruleExtNotIpv6.l2Encap = (GT_U8)inFields[24];
    galtisMacAddr(&maskData.ruleExtNotIpv6.macDa, (GT_U8*)inFields[25]);
    galtisMacAddr(&maskData.ruleExtNotIpv6.macSa, (GT_U8*)inFields[26]);
    maskData.ruleExtNotIpv6.ipv4Fragmented = (GT_BOOL)inFields[27];
    galtisBArray(&maskBArr,(GT_U8*)inFields[28]);

    cmdOsMemCpy(
        maskData.ruleExtNotIpv6.udb0_5, maskBArr.data,
        MIN(6, maskBArr.length));

    cmdOsMemCpy(
        &patternData.ruleExtNotIpv6.common.portListBmp,
        &ruleSet_portListBmpPattern,
        sizeof(CPSS_PORTS_BMP_STC));

    patternData.ruleExtNotIpv6.common.pclId = (GT_U16)inFields[29];
    patternData.ruleExtNotIpv6.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[30];
    patternData.ruleExtNotIpv6.common.isTagged = (GT_U8)inFields[31];
    patternData.ruleExtNotIpv6.common.vid = (GT_U16)inFields[32];
    patternData.ruleExtNotIpv6.common.up = (GT_U8)inFields[33];
    patternData.ruleExtNotIpv6.common.qosProfile = (GT_U8)inFields[34];
    patternData.ruleExtNotIpv6.common.isIp = (GT_U8)inFields[35];
    patternData.ruleExtNotIpv6.common.isL2Valid = (GT_U8)inFields[36];
    patternData.ruleExtNotIpv6.common.isUdbValid = (GT_U8)inFields[37];
    patternData.ruleExtNotIpv6.commonExt.isIpv6 = (GT_U8)inFields[38];
    patternData.ruleExtNotIpv6.commonExt.ipProtocol = (GT_U8)inFields[39];
    patternData.ruleExtNotIpv6.commonExt.dscp = (GT_U8)inFields[40];
    patternData.ruleExtNotIpv6.commonExt.isL4Valid = (GT_U8)inFields[40];
    patternData.ruleExtNotIpv6.commonExt.l4Byte0 = (GT_U8)inFields[42];
    patternData.ruleExtNotIpv6.commonExt.l4Byte1 = (GT_U8)inFields[43];
    patternData.ruleExtNotIpv6.commonExt.l4Byte2 = (GT_U8)inFields[44];
    patternData.ruleExtNotIpv6.commonExt.l4Byte3 = (GT_U8)inFields[45];
    patternData.ruleExtNotIpv6.commonExt.l4Byte13 = (GT_U8)inFields[46];
    patternData.ruleExtNotIpv6.commonExt.ipHeaderOk = (GT_U8)inFields[47];
    galtisIpAddr(&patternData.ruleExtNotIpv6.sip, (GT_U8*)inFields[48]);
    galtisIpAddr(&patternData.ruleExtNotIpv6.dip, (GT_U8*)inFields[49]);
    patternData.ruleExtNotIpv6.etherType = (GT_U16)inFields[50];
    patternData.ruleExtNotIpv6.l2Encap = (GT_U8)inFields[51];
    galtisMacAddr(&patternData.ruleExtNotIpv6.macDa, (GT_U8*)inFields[52]);
    galtisMacAddr(&patternData.ruleExtNotIpv6.macSa, (GT_U8*)inFields[53]);
    patternData.ruleExtNotIpv6.ipv4Fragmented = (GT_BOOL)inFields[54];
    galtisBArray(&patternBArr,(GT_U8*)inFields[55]);

    cmdOsMemCpy(
        patternData.ruleExtNotIpv6.udb0_5, patternBArr.data,
        MIN(6, patternBArr.length));

    if (GT_TRUE == vplsFields)
    {
        actionRetrieve_1(devNum, inFields, 56, &actionData);
    }
    else
    {
        actionRetrieve(devNum, inFields, 56, &actionData);
    }

    PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleExtNotIpv6.common.sourcePort);

    lastActionPtr = &actionData;

    /* call cpss api function */
    result = pg_wrap_cpssDxChPclRuleSet(devNum, ruleFormat, ruleIndex, &maskData,
                                                &patternData, &actionData);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/******************************************************************************/
static CMD_STATUS wrCpssDxChPclRuleSet_EXT_IPV6_L2

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    IN  GT_BOOL vplsFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                          result;
    GT_U8                              devNum;
    GT_U32                             ruleIndex;
    GT_BYTE_ARRY                       maskBArr, patternBArr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    ruleIndex = (GT_U32)inFields[0];
    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E;

    cmdOsMemCpy(
        &maskData.ruleExtIpv6L2.common.portListBmp,
        &ruleSet_portListBmpMask,
        sizeof(CPSS_PORTS_BMP_STC));

    maskData.ruleExtIpv6L2.common.pclId = (GT_U16)inFields[2];
    maskData.ruleExtIpv6L2.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[3];
    maskData.ruleExtIpv6L2.common.isTagged = (GT_U8)inFields[4];
    maskData.ruleExtIpv6L2.common.vid = (GT_U16)inFields[5];
    maskData.ruleExtIpv6L2.common.up = (GT_U8)inFields[6];
    maskData.ruleExtIpv6L2.common.qosProfile = (GT_U8)inFields[7];
    maskData.ruleExtIpv6L2.common.isIp = (GT_U8)inFields[8];
    maskData.ruleExtIpv6L2.common.isL2Valid = (GT_U8)inFields[9];
    maskData.ruleExtIpv6L2.common.isUdbValid = (GT_U8)inFields[10];
    maskData.ruleExtIpv6L2.commonExt.isIpv6 = (GT_U8)inFields[11];
    maskData.ruleExtIpv6L2.commonExt.ipProtocol = (GT_U8)inFields[12];
    maskData.ruleExtIpv6L2.commonExt.dscp = (GT_U8)inFields[13];
    maskData.ruleExtIpv6L2.commonExt.isL4Valid = (GT_U8)inFields[14];
    maskData.ruleExtIpv6L2.commonExt.l4Byte0 = (GT_U8)inFields[15];
    maskData.ruleExtIpv6L2.commonExt.l4Byte1 = (GT_U8)inFields[16];
    maskData.ruleExtIpv6L2.commonExt.l4Byte2 = (GT_U8)inFields[17];
    maskData.ruleExtIpv6L2.commonExt.l4Byte3 = (GT_U8)inFields[18];
    maskData.ruleExtIpv6L2.commonExt.l4Byte13 = (GT_U8)inFields[19];
    maskData.ruleExtIpv6L2.commonExt.ipHeaderOk = (GT_U8)inFields[20];
    galtisIpv6Addr(&maskData.ruleExtIpv6L2.sip, (GT_U8*)inFields[21]);
    maskData.ruleExtIpv6L2.dipBits127to120 = (GT_U8)inFields[22];
    maskData.ruleExtIpv6L2.isIpv6ExtHdrExist = (GT_BOOL)inFields[23];
    maskData.ruleExtIpv6L2.isIpv6HopByHop = (GT_BOOL)inFields[24];
    galtisMacAddr(&maskData.ruleExtIpv6L2.macDa, (GT_U8*)inFields[25]);
    galtisMacAddr(&maskData.ruleExtIpv6L2.macSa, (GT_U8*)inFields[26]);
    galtisBArray(&maskBArr,(GT_U8*)inFields[27]);

    cmdOsMemCpy(
        maskData.ruleExtIpv6L2.udb6_11, maskBArr.data,
        MIN(6, maskBArr.length));

    cmdOsMemCpy(
        &patternData.ruleExtIpv6L2.common.portListBmp,
        &ruleSet_portListBmpPattern,
        sizeof(CPSS_PORTS_BMP_STC));

    patternData.ruleExtIpv6L2.common.pclId = (GT_U16)inFields[28];
    patternData.ruleExtIpv6L2.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[29];
    patternData.ruleExtIpv6L2.common.isTagged = (GT_U8)inFields[30];
    patternData.ruleExtIpv6L2.common.vid = (GT_U16)inFields[31];
    patternData.ruleExtIpv6L2.common.up = (GT_U8)inFields[32];
    patternData.ruleExtIpv6L2.common.qosProfile = (GT_U8)inFields[33];
    patternData.ruleExtIpv6L2.common.isIp = (GT_U8)inFields[34];
    patternData.ruleExtIpv6L2.common.isL2Valid = (GT_U8)inFields[35];
    patternData.ruleExtIpv6L2.common.isUdbValid = (GT_U8)inFields[36];
    patternData.ruleExtIpv6L2.commonExt.isIpv6 = (GT_U8)inFields[37];
    patternData.ruleExtIpv6L2.commonExt.ipProtocol = (GT_U8)inFields[38];
    patternData.ruleExtIpv6L2.commonExt.dscp = (GT_U8)inFields[39];
    patternData.ruleExtIpv6L2.commonExt.isL4Valid = (GT_U8)inFields[40];
    patternData.ruleExtIpv6L2.commonExt.l4Byte0 = (GT_U8)inFields[41];
    patternData.ruleExtIpv6L2.commonExt.l4Byte1 = (GT_U8)inFields[42];
    patternData.ruleExtIpv6L2.commonExt.l4Byte2 = (GT_U8)inFields[43];
    patternData.ruleExtIpv6L2.commonExt.l4Byte3 = (GT_U8)inFields[44];
    patternData.ruleExtIpv6L2.commonExt.l4Byte13 = (GT_U8)inFields[45];
    patternData.ruleExtIpv6L2.commonExt.ipHeaderOk = (GT_U8)inFields[46];
    galtisIpv6Addr(&patternData.ruleExtIpv6L2.sip, (GT_U8*)inFields[47]);
    patternData.ruleExtIpv6L2.dipBits127to120 = (GT_U8)inFields[48];
    patternData.ruleExtIpv6L2.isIpv6ExtHdrExist = (GT_BOOL)inFields[49];
    patternData.ruleExtIpv6L2.isIpv6HopByHop = (GT_BOOL)inFields[50];
    galtisMacAddr(&patternData.ruleExtIpv6L2.macDa, (GT_U8*)inFields[51]);
    galtisMacAddr(&patternData.ruleExtIpv6L2.macSa, (GT_U8*)inFields[52]);
    galtisBArray(&patternBArr,(GT_U8*)inFields[53]);

    cmdOsMemCpy(
        patternData.ruleExtIpv6L2.udb6_11, patternBArr.data,
        MIN(6, patternBArr.length));

    if (GT_TRUE == vplsFields)
    {
        actionRetrieve_1(devNum, inFields, 54, &actionData);
    }
    else
    {
        actionRetrieve(devNum, inFields, 54, &actionData);
    }

    PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleExtIpv6L2.common.sourcePort);

    lastActionPtr = &actionData;

    /* call cpss api function */
    result = pg_wrap_cpssDxChPclRuleSet(devNum, ruleFormat, ruleIndex, &maskData,
                                                &patternData, &actionData);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/******************************************************************************/
static CMD_STATUS wrCpssDxChPclRuleSet_EXT_IPV6_L4

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    IN  GT_BOOL vplsFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                          result;
    GT_U8                              devNum;
    GT_U32                             ruleIndex;
    GT_BYTE_ARRY                       maskBArr, patternBArr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    ruleIndex = (GT_U32)inFields[0];
    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E;

    cmdOsMemCpy(
        &maskData.ruleExtIpv6L4.common.portListBmp,
        &ruleSet_portListBmpMask,
        sizeof(CPSS_PORTS_BMP_STC));

    maskData.ruleExtIpv6L4.common.pclId = (GT_U16)inFields[2];
    maskData.ruleExtIpv6L4.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[3];
    maskData.ruleExtIpv6L4.common.isTagged = (GT_U8)inFields[4];
    maskData.ruleExtIpv6L4.common.vid = (GT_U16)inFields[5];
    maskData.ruleExtIpv6L4.common.up = (GT_U8)inFields[6];
    maskData.ruleExtIpv6L4.common.qosProfile = (GT_U8)inFields[7];
    maskData.ruleExtIpv6L4.common.isIp = (GT_U8)inFields[8];
    maskData.ruleExtIpv6L4.common.isL2Valid = (GT_U8)inFields[9];
    maskData.ruleExtIpv6L4.common.isUdbValid = (GT_U8)inFields[10];
    maskData.ruleExtIpv6L4.commonExt.isIpv6 = (GT_U8)inFields[11];
    maskData.ruleExtIpv6L4.commonExt.ipProtocol = (GT_U8)inFields[12];
    maskData.ruleExtIpv6L4.commonExt.dscp = (GT_U8)inFields[13];
    maskData.ruleExtIpv6L4.commonExt.isL4Valid = (GT_U8)inFields[14];
    maskData.ruleExtIpv6L4.commonExt.l4Byte0 = (GT_U8)inFields[15];
    maskData.ruleExtIpv6L4.commonExt.l4Byte1 = (GT_U8)inFields[16];
    maskData.ruleExtIpv6L4.commonExt.l4Byte2 = (GT_U8)inFields[17];
    maskData.ruleExtIpv6L4.commonExt.l4Byte3 = (GT_U8)inFields[18];
    maskData.ruleExtIpv6L4.commonExt.l4Byte13 = (GT_U8)inFields[19];
    maskData.ruleExtIpv6L4.commonExt.ipHeaderOk = (GT_U8)inFields[20];
    galtisIpv6Addr(&maskData.ruleExtIpv6L4.sip, (GT_U8*)inFields[21]);
    galtisIpv6Addr(&maskData.ruleExtIpv6L4.dip, (GT_U8*)inFields[22]);
    maskData.ruleExtIpv6L4.isIpv6ExtHdrExist = (GT_U8)inFields[23];
    maskData.ruleExtIpv6L4.isIpv6HopByHop = (GT_U8)inFields[24];
    galtisBArray(&maskBArr,(GT_U8*)inFields[25]);

    cmdOsMemCpy(
        maskData.ruleExtIpv6L4.udb12_14, maskBArr.data,
        MIN(3, maskBArr.length));

    cmdOsMemCpy(
        &patternData.ruleExtIpv6L4.common.portListBmp,
        &ruleSet_portListBmpPattern,
        sizeof(CPSS_PORTS_BMP_STC));

    patternData.ruleExtIpv6L4.common.pclId = (GT_U16)inFields[26];
    patternData.ruleExtIpv6L4.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[27];
    patternData.ruleExtIpv6L4.common.isTagged = (GT_BOOL)inFields[28];
    patternData.ruleExtIpv6L4.common.vid = (GT_U16)inFields[29];
    patternData.ruleExtIpv6L4.common.up = (GT_U8)inFields[30];
    patternData.ruleExtIpv6L4.common.qosProfile = (GT_U8)inFields[31];
    patternData.ruleExtIpv6L4.common.isIp = (GT_BOOL)inFields[32];
    patternData.ruleExtIpv6L4.common.isL2Valid = (GT_BOOL)inFields[33];
    patternData.ruleExtIpv6L4.common.isUdbValid = (GT_BOOL)inFields[34];
    patternData.ruleExtIpv6L4.commonExt.isIpv6 = (GT_U8)inFields[35];
    patternData.ruleExtIpv6L4.commonExt.ipProtocol = (GT_U8)inFields[36];
    patternData.ruleExtIpv6L4.commonExt.dscp = (GT_U8)inFields[37];
    patternData.ruleExtIpv6L4.commonExt.isL4Valid = (GT_BOOL)inFields[38];
    patternData.ruleExtIpv6L4.commonExt.l4Byte0 = (GT_U8)inFields[39];
    patternData.ruleExtIpv6L4.commonExt.l4Byte1 = (GT_U8)inFields[40];
    patternData.ruleExtIpv6L4.commonExt.l4Byte2 = (GT_U8)inFields[41];
    patternData.ruleExtIpv6L4.commonExt.l4Byte3 = (GT_U8)inFields[42];
    patternData.ruleExtIpv6L4.commonExt.l4Byte13 = (GT_U8)inFields[43];
    patternData.ruleExtIpv6L4.commonExt.ipHeaderOk = (GT_BOOL)inFields[44];
    galtisIpv6Addr(&patternData.ruleExtIpv6L4.sip, (GT_U8*)inFields[45]);
    galtisIpv6Addr(&patternData.ruleExtIpv6L4.dip, (GT_U8*)inFields[46]);
    patternData.ruleExtIpv6L4.isIpv6ExtHdrExist = (GT_BOOL)inFields[47];
    patternData.ruleExtIpv6L4.isIpv6HopByHop = (GT_BOOL)inFields[48];
    galtisBArray(&patternBArr,(GT_U8*)inFields[49]);

    cmdOsMemCpy(
        patternData.ruleExtIpv6L4.udb12_14, patternBArr.data,
        MIN(3, patternBArr.length));

    if (GT_TRUE == vplsFields)
    {
        actionRetrieve_1(devNum, inFields, 50, &actionData);
    }
    else
    {
        actionRetrieve(devNum, inFields, 50, &actionData);
    }

    PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleExtIpv6L4.common.sourcePort);

    lastActionPtr = &actionData;

    /* call cpss api function */
    result = pg_wrap_cpssDxChPclRuleSet(devNum, ruleFormat, ruleIndex, &maskData,
                                                &patternData, &actionData);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}



/******************************************************************************/
static CMD_STATUS wrCpssDxChPclRuleSet_STD_IPV6_DIP

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    IN  GT_BOOL vplsFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                          result;
    GT_U8                              devNum;
    GT_U32                             ruleIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    ruleIndex = (GT_U32)inFields[0];

    cmdOsMemCpy(
        &maskData.ruleStdIpv6Dip.common.portListBmp,
        &ruleSet_portListBmpMask,
        sizeof(CPSS_PORTS_BMP_STC));

    maskData.ruleStdIpv6Dip.common.pclId = (GT_U16)inFields[2];
    CONVERT_PCL_MASK_PCL_ID_TEST_TO_CPSS_MAC(maskData.ruleStdIpv6Dip.common.pclId);
    maskData.ruleStdIpv6Dip.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[3];
    maskData.ruleStdIpv6Dip.common.isTagged = (GT_U8)inFields[4];
    maskData.ruleStdIpv6Dip.common.vid = (GT_U16)inFields[5];
    maskData.ruleStdIpv6Dip.common.up = (GT_U8)inFields[6];
    maskData.ruleStdIpv6Dip.common.qosProfile = (GT_U8)inFields[7];
    maskData.ruleStdIpv6Dip.common.isIp = (GT_U8)inFields[8];
    maskData.ruleStdIpv6Dip.common.isL2Valid = (GT_U8)inFields[9];
    maskData.ruleStdIpv6Dip.common.isUdbValid = (GT_U8)inFields[10];
    maskData.ruleStdIpv6Dip.commonStdIp.isIpv4 = (GT_U8)inFields[11];
    maskData.ruleStdIpv6Dip.commonStdIp.ipProtocol = (GT_U8)inFields[12];
    maskData.ruleStdIpv6Dip.commonStdIp.dscp = (GT_U8)inFields[13];
    maskData.ruleStdIpv6Dip.commonStdIp.isL4Valid = (GT_U8)inFields[14];
    maskData.ruleStdIpv6Dip.commonStdIp.l4Byte2 = (GT_U8)inFields[15];
    maskData.ruleStdIpv6Dip.commonStdIp.l4Byte3 = (GT_U8)inFields[16];
    maskData.ruleStdIpv6Dip.commonStdIp.ipHeaderOk = (GT_U8)inFields[17];
    maskData.ruleStdIpv6Dip.commonStdIp.ipv4Fragmented = (GT_U8)inFields[18];
    maskData.ruleStdIpv6Dip.isArp = (GT_BOOL)inFields[19];
    maskData.ruleStdIpv6Dip.isIpv6ExtHdrExist = (GT_U8)inFields[20];
    maskData.ruleStdIpv6Dip.isIpv6HopByHop = (GT_U8)inFields[21];
    galtisIpv6Addr(&maskData.ruleStdIpv6Dip.dip, (GT_U8*)inFields[22]);

    cmdOsMemCpy(
        &patternData.ruleStdIpv6Dip.common.portListBmp,
        &ruleSet_portListBmpPattern,
        sizeof(CPSS_PORTS_BMP_STC));

    patternData.ruleStdIpv6Dip.common.pclId = (GT_U16)inFields[23];
    patternData.ruleStdIpv6Dip.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[24];
    patternData.ruleStdIpv6Dip.common.isTagged = (GT_U8)inFields[25];
    patternData.ruleStdIpv6Dip.common.vid = (GT_U16)inFields[26];
    patternData.ruleStdIpv6Dip.common.up = (GT_U8)inFields[27];
    patternData.ruleStdIpv6Dip.common.qosProfile = (GT_U8)inFields[28];
    patternData.ruleStdIpv6Dip.common.isIp = (GT_U8)inFields[29];
    patternData.ruleStdIpv6Dip.common.isL2Valid = (GT_U8)inFields[30];
    patternData.ruleStdIpv6Dip.common.isUdbValid = (GT_U8)inFields[31];
    patternData.ruleStdIpv6Dip.commonStdIp.isIpv4 = (GT_U8)inFields[32];
    patternData.ruleStdIpv6Dip.commonStdIp.ipProtocol = (GT_U8)inFields[33];
    patternData.ruleStdIpv6Dip.commonStdIp.dscp = (GT_U8)inFields[34];
    patternData.ruleStdIpv6Dip.commonStdIp.isL4Valid = (GT_U8)inFields[35];
    patternData.ruleStdIpv6Dip.commonStdIp.l4Byte2 = (GT_U8)inFields[36];
    patternData.ruleStdIpv6Dip.commonStdIp.l4Byte3 = (GT_U8)inFields[37];
    patternData.ruleStdIpv6Dip.commonStdIp.ipHeaderOk = (GT_U8)inFields[38];
    patternData.ruleStdIpv6Dip.commonStdIp.ipv4Fragmented = (GT_U8)inFields[39];
    patternData.ruleStdIpv6Dip.isArp = (GT_BOOL)inFields[40];
    patternData.ruleStdIpv6Dip.isIpv6ExtHdrExist = (GT_U8)inFields[41];
    patternData.ruleStdIpv6Dip.isIpv6HopByHop = (GT_U8)inFields[42];
    galtisIpv6Addr(&patternData.ruleStdIpv6Dip.dip, (GT_U8*)inFields[43]);

    if (GT_TRUE == vplsFields)
    {
        actionRetrieve_1(devNum, inFields, 44, &actionData);
    }
    else
    {
        actionRetrieve(devNum, inFields, 44, &actionData);
    }

    PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleStdIpv6Dip.common.sourcePort);

    lastActionPtr = &actionData;

    /* call cpss api function */
    result = pg_wrap_cpssDxChPclRuleSet(devNum,
                              CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E,
                                  ruleIndex, &maskData, &patternData, &actionData);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/******************************************************************************/
static CMD_STATUS wrCpssDxChPclRuleSet_STD_IPV4_L4

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    IN  GT_BOOL vplsFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                          result;
    GT_U8                              devNum;
    GT_U32                             ruleIndex;
    GT_BYTE_ARRY                       maskBArr, patternBArr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    ruleIndex = (GT_U32)inFields[0];
    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E;

    cmdOsMemCpy(
        &maskData.ruleStdIpv4L4.common.portListBmp,
        &ruleSet_portListBmpMask,
        sizeof(CPSS_PORTS_BMP_STC));

    maskData.ruleStdIpv4L4.common.pclId = (GT_U16)inFields[2];
    CONVERT_PCL_MASK_PCL_ID_TEST_TO_CPSS_MAC(maskData.ruleStdIpv4L4.common.pclId);
    maskData.ruleStdIpv4L4.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[3];
    maskData.ruleStdIpv4L4.common.isTagged = (GT_U8)inFields[4];
    maskData.ruleStdIpv4L4.common.vid = (GT_U16)inFields[5];
    maskData.ruleStdIpv4L4.common.up = (GT_U8)inFields[6];
    maskData.ruleStdIpv4L4.common.qosProfile = (GT_U8)inFields[7];
    maskData.ruleStdIpv4L4.common.isIp = (GT_U8)inFields[8];
    maskData.ruleStdIpv4L4.common.isL2Valid = (GT_U8)inFields[9];
    maskData.ruleStdIpv4L4.common.isUdbValid = (GT_U8)inFields[10];
    maskData.ruleStdIpv4L4.commonStdIp.isIpv4 = (GT_U8)inFields[11];
    maskData.ruleStdIpv4L4.commonStdIp.ipProtocol = (GT_U8)inFields[12];
    maskData.ruleStdIpv4L4.commonStdIp.dscp = (GT_U8)inFields[13];
    maskData.ruleStdIpv4L4.commonStdIp.isL4Valid = (GT_U8)inFields[14];
    maskData.ruleStdIpv4L4.commonStdIp.l4Byte2 = (GT_U8)inFields[15];
    maskData.ruleStdIpv4L4.commonStdIp.l4Byte3 = (GT_U8)inFields[16];
    maskData.ruleStdIpv4L4.commonStdIp.ipHeaderOk = (GT_U8)inFields[17];
    maskData.ruleStdIpv4L4.commonStdIp.ipv4Fragmented = (GT_U8)inFields[18];
    maskData.ruleStdIpv4L4.isArp = (GT_BOOL)inFields[19];
    maskData.ruleStdIpv4L4.isBc = (GT_BOOL)inFields[20];
    galtisIpAddr(&maskData.ruleStdIpv4L4.sip, (GT_U8*)inFields[21]);
    galtisIpAddr(&maskData.ruleStdIpv4L4.dip, (GT_U8*)inFields[22]);
    maskData.ruleStdIpv4L4.l4Byte0 = (GT_U8)inFields[23];
    maskData.ruleStdIpv4L4.l4Byte1 = (GT_U8)inFields[24];
    maskData.ruleStdIpv4L4.l4Byte13 = (GT_U8)inFields[25];
    galtisBArray(&maskBArr,(GT_U8*)inFields[26]);

    cmdOsMemCpy(
        maskData.ruleStdIpv4L4.udb20_22, maskBArr.data,
        MIN(3, maskBArr.length));

    cmdOsMemCpy(
        &patternData.ruleStdIpv4L4.common.portListBmp,
        &ruleSet_portListBmpPattern,
        sizeof(CPSS_PORTS_BMP_STC));

    patternData.ruleStdIpv4L4.common.pclId = (GT_U16)inFields[27];
    patternData.ruleStdIpv4L4.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[28];
    patternData.ruleStdIpv4L4.common.isTagged = (GT_U8)inFields[29];
    patternData.ruleStdIpv4L4.common.vid = (GT_U16)inFields[30];
    patternData.ruleStdIpv4L4.common.up = (GT_U8)inFields[31];
    patternData.ruleStdIpv4L4.common.qosProfile = (GT_U8)inFields[32];
    patternData.ruleStdIpv4L4.common.isIp = (GT_U8)inFields[33];
    patternData.ruleStdIpv4L4.common.isL2Valid = (GT_U8)inFields[34];
    patternData.ruleStdIpv4L4.common.isUdbValid = (GT_U8)inFields[35];
    patternData.ruleStdIpv4L4.commonStdIp.isIpv4 = (GT_U8)inFields[36];
    patternData.ruleStdIpv4L4.commonStdIp.ipProtocol = (GT_U8)inFields[37];
    patternData.ruleStdIpv4L4.commonStdIp.dscp = (GT_U8)inFields[38];
    patternData.ruleStdIpv4L4.commonStdIp.isL4Valid = (GT_U8)inFields[39];
    patternData.ruleStdIpv4L4.commonStdIp.l4Byte2 = (GT_U8)inFields[40];
    patternData.ruleStdIpv4L4.commonStdIp.l4Byte3 = (GT_U8)inFields[41];
    patternData.ruleStdIpv4L4.commonStdIp.ipHeaderOk = (GT_U8)inFields[42];
    patternData.ruleStdIpv4L4.commonStdIp.ipv4Fragmented = (GT_U8)inFields[43];
    patternData.ruleStdIpv4L4.isArp = (GT_BOOL)inFields[44];
    patternData.ruleStdIpv4L4.isBc = (GT_BOOL)inFields[45];
    galtisIpAddr(&patternData.ruleStdIpv4L4.sip, (GT_U8*)inFields[46]);
    galtisIpAddr(&patternData.ruleStdIpv4L4.dip, (GT_U8*)inFields[47]);
    patternData.ruleStdIpv4L4.l4Byte0 = (GT_U8)inFields[48];
    patternData.ruleStdIpv4L4.l4Byte1 = (GT_U8)inFields[49];
    patternData.ruleStdIpv4L4.l4Byte13 = (GT_U8)inFields[50];
    galtisBArray(&patternBArr,(GT_U8*)inFields[51]);

    cmdOsMemCpy(
        patternData.ruleStdIpv4L4.udb20_22, patternBArr.data,
        MIN(3, patternBArr.length));

    if (GT_TRUE == vplsFields)
    {
        actionRetrieve_1(devNum, inFields, 52, &actionData);
    }
    else
    {
        actionRetrieve(devNum, inFields, 52, &actionData);
    }

    PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleStdIpv4L4.common.sourcePort);

    lastActionPtr = &actionData;

    /* call cpss api function */
    result = pg_wrap_cpssDxChPclRuleSet(devNum, ruleFormat, ruleIndex, &maskData,
                                                &patternData, &actionData);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}




/* Table: cpssDxChPclRuleAction
*
* Description:
*     PCL Configuration table
*
* Fields:
*      pktCmd       -  packet command (forward, mirror-to-cpu,
*                      hard-drop, soft-drop, or trap-to-cpu)
*      mirror       -  packet mirroring configuration
*           mirrorToRxAnalyzerPort
*                        - Enables mirroring the packet to
*                          the ingress analyzer port.
*                          GT_FALSE = Packet is not mirrored to ingress
*                                     analyzer port.
*                          GT_TRUE = Packet is mirrored to ingress analyzer port.
*           cpuCode      - The CPU code assigned to packets
*                          Mirrored to CPU or Trapped to CPU due
*                          to a match in the Policy rule entry
*      matchCounter -  match counter configuration
*           enableMatchCount
*                        - Enables the binding of this
*                          policy action entry to the Policy Rule Match
*                          Counter<n> (0<=n<32) indexed by matchCounterIndex
*                          GT_FALSE  = Match counter binding is disabled.
*                          GT_TRUE = Match counter binding is enabled.
*
*           matchCounterIndex
*                        - A index one of the 32 Policy Rule Match Counter<n>
*                          (0<=n<32) The counter is incremented for every packet
*                          satisfying both of the following conditions:
*                          - Matching this rule.
*                          - The previous packet command is not hard drop.
*      qos          -  packet QoS attributes modification configuration
*           egressPolicy - GT_TRUE  - Action is used for the Egress Policy
*                          GT_FALSE - Action is used for the Ingress Policy
*           modifyDscp   - For Ingress Policy:
*                          The Modify DSCP QoS attribute of the packet.
*                          Enables modification of the packet's DSCP field.
*                          Only relevant if QoS precedence of the previous
*                          QoS assignment mechanisms (Port, Protocol Based
*                          QoS, and previous matching rule) is Soft.
*                          Relevant for IPv4/IPv6 packets, only.
*                          ModifyDSCP enables the following:
*                          - Keep previous DSCP modification command.
*                          - Enable modification of the DSCP field in
*                            the packet.
*                          - Disable modification of the DSCP field in
*                            the packet.
*
*                          For Egress Policy:
*                          Enables modification of the IP Header DSCP field
*                          to egress.dscp of the transmitted packet.
*                          ModifyDSCP enables the following:
*                          - Keep previous packet DSCP setting.
*                          - Modify the Packet's DSCP to <egress.dscp>.
*
*           modifyUp     - For Ingress Policy:
*                          The Modify UP QoS attribute of the packet.
*                          Enables modification of the packet's
*                          802.1p User Priority field.
*                          Only relevant if QoS precedence of the previous
*                          QoS assignment mechanisms  (Port, Protocol Based
*                          QoS, and previous matching rule) is Soft.
*                          ModifyUP enables the following:
*                          - Keep previous QoS attribute <ModifyUP> setting.
*                          - Set the QoS attribute <modifyUP> to 1.
*                          - Set the QoS attribute <modifyUP> to 0.
*
*                          For Egress Policy:
*                          Enables the modification of the 802.1p User
*                          Priority field to egress.up of packet
*                          transmitted with a VLAN tagged.
*                          ModifyUP enables the following:
*                          - Keep previous QoS attribute <ModifyUP> setting.
*                          - Modify the Packet's UP to <egress.up>.
*
*           egress.dscp  - The DSCP field set to the transmitted packets.
*                          Relevant for Egress Policy only.
*           egress.up    - The 802.1p UP field set to the transmitted packets.
*                          Relevant for Egress Policy only.
*        ingress members:
*           profileIndex - The QoS Profile Attribute of the packet.
*                          Only relevant if the QoS precedence of the
*                          previous QoS Assignment Mechanisms (Port,
*                          Protocol Based QoS, and previous matching rule)
*                          is Soft and profileAssignIndex is set
*                          to GT_TRUE.the QoSProfile is used to index the
*                          QoSProfile to QoS Table Entry<n> (0<=n<72)
*                          and map the QoS Parameters for the packet,
*                          which are TC, DP, UP and DSCP
*                          Valid Range - 0 through 71
*           profileAssignIndex
*                        - Enable marking of QoS Profile Attribute of
*                          the packet.
*                          GT_TRUE - Assign <profileIndex> to the packet.
*                          GT_FALSE - Preserve previous QoS Profile setting.
*           profilePrecedence
*                        - Marking of the QoSProfile Precedence.
*                          Setting this bit locks the QoS parameters setting
*                          from being modified by subsequent QoS
*                          assignment engines in the ingress pipe.
*                          QoSPrecedence enables the following:
*                          - QoS precedence is soft and the packet's QoS
*                            parameters may be overridden by subsequent
*                            QoS assignment engines.
*                          - QoS precedence is hard and the packet's QoS
*                            parameters setting is performed until
*                            this stage is locked. It cannot be overridden
*                            by subsequent QoS assignment engines.
*      redirect     -  packet Policy redirection configuration
*           redirectCmd  - redirection command
*
*           data.outIf   - out interface redirection data
*                          relevant for CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_IF_E
*                          packet redirected to output interface
*           outInterface - output interface (port, trunk, VID, VIDX)
*           vntL2Echo    - Enables Virtual Network Tester Layer 2 Echo
*                          GT_TRUE - swap MAC SA and MAC DA in redirected packet
*                          GT_FALSE - don't swap MAC SA and MAC DA
*           tunnelStart  - Indicates this action is a Tunnel Start point
*                          GT_TRUE - Packet is redirected to an Egress Interface
*                                    and is tunneled as an  Ethernet-over-MPLS.
*                          GT_FALSE - Packet is redirected to an Egress
*                                     Interface and is not tunneled.
*           tunnelPtr    - the pointer to "Tunnel-start" entry
*                          (relevant only if tunnelStart == GT_TRUE)
*
*           data.routerLttIndex
*                        - index of IP router Lookup Translation Table entry
*                          relevant for CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E
*      policer      -  packet Policing configuration
*           policerEnable
*                        - GT_TRUE - policer enable,
*                          GT_FALSE - policer disable
*           policerId    - policers table entry index
*      vlan         -  packet VLAN modification configuration
*           egressTaggedModify - the field deleted
*                        - VLAN ID Modification either for Egress or Ingress
*                          Policy
*                        - GT_FALSE - Ingress Policy.
*                                     Use <modifyVlan> command,
*                        - GT_TRUE  - Egress Policy.
*                                     Use VLAN ID assignment to tagged packets.
*                                     (for Ingress Policy this is
*                                     ASSIGN_FOR_UNTAGGED_E command)
*           modifyVlan   - VLAN id modification command
*
*           nestedVlan   - When this field is set to GT_TRUE, this rule matching
*                          flow is defined as an access flow. The VID of all
*                          packets received on this flow is discarded and they
*                          are assigned with a VID using the device's VID
*                          assignment algorithms, as if they are untagged. When
*                          a packet received on an access flow is transmitted
*                          via a core port or a Cascading port, a VLAN tag is
*                          added to the packet (on top of the existing VLAN tag,
*                          if any). The VID field is the VID assigned to the
*                          packet as a result of all VLAN assignment algorithms.
*                          The 802.1p User Priority field of this added tag may
*                          be one of the following, depending on the ModifyUP
*                          QoS parameter set to the packet at the end of the
*                          Ingress pipe:
*                          - If ModifyUP is GT_TRUE, it is the UP extracted
*                            from the QoSProfile to QoS Table Entry<n>
*                          - If ModifyUP is GT_FALSE, it is the original packet
*                            802.1p User Priority field if it is tagged and is
*                            UP if the original packet is untagged.
*
*           vlanId       - VLAN id used for VLAN id modification if command
*                          not CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E or
*                          egressTaggedModify = GT_TRUE
*           precedence   - The VLAN Assignment precedence for the subsequent
*                          VLAN assignment mechanism, which is the Policy engine
*                          next policy-pass rule. Only relevant if the
*                          VID precedence set by the previous VID assignment
*                          mechanisms (Port, Protocol Based VLANs, and previous
*                          matching rule) is Soft.
*                          - Soft precedence The VID assignment can be
*                            overridden by the subsequent VLAN assignment
*                            mechanism, which is the Policy engine.
*                          - Hard precedence The VID assignment is locked to the last
*                            VLAN assigned to the packet and cannot be overridden.
*      ipUcRoute    -  special 98DX2X3 and 98DX1x7 Ip Unicast Route action
*           doIpUcRoute  - Configure IP Unicast Routing Actions
*                          GT_TRUE - the action used for IP unicast routing
*                          GT_FALSE - the action is not used for IP unicast
*                                     routing, all data of the structure
*                                      ignored.
*           arpDaIndex   - Route Entry ARP Index to the ARP Table (10 bit)
*           decrementTTL - Decrement IPv4 <TTL> or IPv6 <Hop Limit> enable
*                          GT_TRUE - TTL Decrement for routed packets is enabled
*                          GT_FALSE - TTL Decrement for routed packets is disabled
*           bypassTTLCheck
*                        - Bypass Router engine TTL and Options Check
*                          GT_TRUE - the router engine bypasses the
*                                    IPv4 TTL/Option check and the IPv6 Hop
*                                    Limit/Hop-by-Hop check. This is used for
*                                    IP-TO-ME host entries, where the packet
*                                    is destined to this device
*                          GT_FALSE - the check is not bypassed
*           icmpRedirectCheck
*                        - ICMP Redirect Check Enable
*                          GT_TRUE - the router engine checks if the next hop
*                                    VLAN is equal to the packet VLAN
*                                    assignment, and if so, the packet is
*                                    mirrored to the CPU, in order to send an
*                                    ICMP Redirect message back to the sender.
*                          GT_FALSE - the check disabled parameters
*                                     configuration.
*
*
* Comments:
*
*   - for 98DX2X5 - full support,
*   - for 98DX2x3,98DX2x0,98DX2x7, 98DX1x6, 98DX1x7 and so on supports
*     Ingress direction only with 32 ports per device and the
*     non-local-device-entries-segment-base == 0
*
*
*/
/**
* @internal wrCpssDxChPclRuleActionUpdate function
* @endinternal
*
* @brief   The function updates the RULE Action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*
* @note NONE
*
*/
static CMD_STATUS wrCpssDxChPclRuleActionUpdate

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PCL_RULE_SIZE_ENT      ruleSize;
    GT_U32                      ruleIndex;
    CPSS_DXCH_PCL_ACTION_STC    action;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* to support the not seted/geted members of structures */
    cpssOsMemSet(&action, 0, sizeof(action));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ruleSize = (CPSS_PCL_RULE_SIZE_ENT)inArgs[1];
    ruleIndex = (GT_U32)inArgs[2];


    actionRetrieve(devNum, inFields, 0, &action);

    /* call cpss api function */
    result = pg_wrap_cpssDxChPclRuleActionUpdate(devNum, ruleSize, ruleIndex,
                                                          &action);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclRuleActionUpdate_1 function
* @endinternal
*
* @brief   The function updates the RULE Action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*
* @note NONE
*
*/
static CMD_STATUS wrCpssDxChPclRuleActionUpdate_1

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PCL_RULE_SIZE_ENT      ruleSize;
    GT_U32                      ruleIndex;
    CPSS_DXCH_PCL_ACTION_STC    action;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* to support the not seted/geted members of structures */
    cpssOsMemSet(&action, 0, sizeof(action));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ruleSize = (CPSS_PCL_RULE_SIZE_ENT)inArgs[1];
    ruleIndex = (GT_U32)inArgs[2];


    actionRetrieve_1(devNum, inFields, 0, &action);

    /* call cpss api function */
    result = pg_wrap_cpssDxChPclRuleActionUpdate(devNum, ruleSize, ruleIndex,
                                                          &action);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChPclRuleActionGet function
* @endinternal
*
* @brief   The function gets the Rule Action
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*
* @note NONE
*
*/
static CMD_STATUS wrCpssDxChPclRuleActionGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PCL_RULE_SIZE_ENT      ruleSize;
    GT_U32                      ruleIndex;
    CPSS_PCL_DIRECTION_ENT      direction;
    CPSS_DXCH_PCL_ACTION_STC    action;
    GT_HW_DEV_NUM               __HwDev;  /* Dummy for converting. */
    GT_PORT_NUM                 __Port; /* Dummy for converting. */

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* to support the not seted/geted members of structures */
    cpssOsMemSet(&action, 0, sizeof(action));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ruleSize = (CPSS_PCL_RULE_SIZE_ENT)inArgs[1];
    ruleIndex = (CPSS_PCL_DIRECTION_ENT)inArgs[2];
    direction = (GT_U32)inArgs[3];


    result = pg_wrap_cpssDxChPclRuleActionGet(devNum, ruleSize, ruleIndex,
                                            direction, &action);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = action.pktCmd;
    inFields[1] = action.mirror.cpuCode;
    inFields[2] = action.mirror.mirrorToRxAnalyzerPort;
    inFields[3] = action.matchCounter.enableMatchCount;
    inFields[4] = action.matchCounter.matchCounterIndex;
    inFields[5] = action.egressPolicy;
    if (action.egressPolicy == GT_FALSE)
    {
        inFields[6] = action.qos.ingress.modifyDscp;
        inFields[7] = action.qos.ingress.modifyUp;
        inFields[8] = action.qos.ingress.profileIndex;
        inFields[9] = action.qos.ingress.profileAssignIndex;
        inFields[10] = action.qos.ingress.profilePrecedence;
    }
    else
    {
        inFields[6] =
            (action.qos.egress.modifyDscp == CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E)
                ? CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E
                : CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
        inFields[7] =
            (action.qos.egress.modifyUp ==
             CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E)
                ? CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E
                : CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
        inFields[11] = action.qos.egress.dscp;
        inFields[12] = action.qos.egress.up;
    }
    inFields[13] = action.redirect.data.outIf.outInterface.type;
    __HwDev  = action.redirect.data.outIf.outInterface.devPort.hwDevNum;
    __Port = action.redirect.data.outIf.outInterface.devPort.portNum;

    CONVERT_BACK_DEV_PORT_DATA_MAC(__HwDev, __Port) ;
    inFields[15] = __HwDev  ;
    inFields[16] = __Port ;

    CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(action.redirect.data.outIf.outInterface.trunkId);
    inFields[17] = action.redirect.data.outIf.outInterface.trunkId;
    inFields[18] = action.redirect.data.outIf.outInterface.vidx;
    inFields[19] = action.redirect.data.outIf.outInterface.vlanId;
    inFields[20] = action.redirect.data.outIf.vntL2Echo;
    inFields[21] = action.redirect.data.outIf.tunnelStart;
    inFields[22] = action.redirect.data.outIf.tunnelPtr;
    inFields[23] = action.redirect.data.routerLttIndex;
    inFields[24] = action.policer.policerEnable;
    inFields[25] = action.policer.policerId;
    inFields[26] = GT_FALSE; /* actionPtr.vlan.egressTaggedModify - this field does not exist anymore */
    if (action.egressPolicy == GT_FALSE)
    {
        inFields[27] = action.vlan.ingress.modifyVlan;
        inFields[28] = action.vlan.ingress.nestedVlan;
        inFields[29] = action.vlan.ingress.vlanId;
        inFields[30] = action.vlan.ingress.precedence;
    }
    else
    {
        inFields[27] =
            (action.vlan.egress.vlanCmd ==
             CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E)
                ? CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E
                : CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E;
        inFields[29] = action.vlan.egress.vlanId;
    }
    inFields[31] = action.ipUcRoute.doIpUcRoute;
    inFields[32] = action.ipUcRoute.arpDaIndex;
    inFields[33] = action.ipUcRoute.decrementTTL;
    inFields[34] = action.ipUcRoute.bypassTTLCheck;
    inFields[35] = action.ipUcRoute.icmpRedirectCheck;


    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0],  inFields[1],  inFields[2],
                inFields[3],  inFields[4],  inFields[5],  inFields[6],
                inFields[7],  inFields[8],  inFields[9],  inFields[10],
                inFields[11], inFields[12], inFields[13], inFields[14],
                inFields[15], inFields[16], inFields[17], inFields[18],
                inFields[19], inFields[20], inFields[21], inFields[22],
                inFields[23], inFields[24], inFields[25], inFields[26],
                inFields[27], inFields[28], inFields[29], inFields[30],
                inFields[31], inFields[32], inFields[33], inFields[34],
                inFields[35]);

    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclRuleInvalidate function
* @endinternal
*
* @brief   The function invalidates the Policy Rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChPclRuleInvalidate

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                        devNum;
    CPSS_PCL_RULE_SIZE_ENT       ruleSize;
    GT_U32                       ruleIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ruleSize = (CPSS_PCL_RULE_SIZE_ENT)inArgs[1];
    ruleIndex = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = pg_wrap_cpssDxChPclRuleInvalidate(devNum, ruleSize, ruleIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChPclRuleCopy function
* @endinternal
*
* @brief   The function copies the Rule's mask, pattern and action to new TCAM position.
*         The source Rule is not invalidated by the function. To implement move Policy
*         Rule from old position to new one at first pg_wrap_cpssDxChPclRuleCopy should be
*         called. And after this cpssExMxPclRuleInvalidate should be used to invalidate
*         Rule in old position
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChPclRuleCopy

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                        devNum;
    CPSS_PCL_RULE_SIZE_ENT       ruleSize;
    GT_U32                       ruleSrcIndex;
    GT_U32                       ruleDstIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ruleSize = (CPSS_PCL_RULE_SIZE_ENT)inArgs[1];
    ruleSrcIndex = (GT_U32)inArgs[2];
    ruleDstIndex = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = pg_wrap_cpssDxChPclRuleCopy(devNum, ruleSize, ruleSrcIndex, ruleDstIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChPclRuleMatchCounterGet function
* @endinternal
*
* @brief   Get or get and reset rule matching counters depending on the reset
*         parameter.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwize
*/
static CMD_STATUS wrCpssDxChPclRuleMatchCounterGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8     devNum;
    GT_U32    counterIndex;
    GT_U32    counterPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    counterIndex = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPclRuleMatchCounterGet(devNum, counterIndex, &counterPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", counterPtr);
    return CMD_OK;
}


/**
* @internal wrCpssDxChPclRuleMatchCounterSet function
* @endinternal
*
* @brief   Set rule matching counters.
*         To reset value of counter use counterValue = 0
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChPclRuleMatchCounterSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8     devNum;
    GT_U32    counterIndex;
    GT_U32    counterValue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    counterIndex = (GT_U32)inArgs[1];
    counterValue = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPclRuleMatchCounterSet(devNum, counterIndex, counterValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChPclPortIngressPolicyEnable function
* @endinternal
*
* @brief   Enables/disables ingress policy per port.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
*/
static CMD_STATUS wrCpssDxChPclPortIngressPolicyEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8      devNum;
    GT_PORT_NUM      port;
    GT_BOOL    enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChPclPortIngressPolicyEnable(devNum, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPclPortIngressPolicyEnableGet function
* @endinternal
*
* @brief   Get the Enable/Disable ingress policy status per port.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclPortIngressPolicyEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                 result;

    GT_U8                    devNum;
    GT_PORT_NUM                    port;
    GT_BOOL                  enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChPclPortIngressPolicyEnableGet(devNum, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPclPortLookupCfgTabAccessModeSet function
* @endinternal
*
* @brief   Configures VLAN/PORT access mode to Ingress or Egress PCL
*         configuration table perlookup.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
* @note   NOT APPLICABLE DEVICES:  Configures VLAN/PORT access mode to Ingress or Egress PCL
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
*
* @note - for 98DX2X5 - full support,
*       - for 98DX2x3,98DX2x0,98DX2x7, 98DX1x6, 98DX1x7 and so on supports
*       Ingress direction only and set same access type
*       for both lookups regardless the <lookupNum> parameter value
*
*/
static CMD_STATUS wrCpssDxChPclPortLookupCfgTabAccessModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                         result;

    GT_U8                                             devNum;
    GT_PORT_NUM                                             port;
    CPSS_PCL_DIRECTION_ENT                            direction;
    CPSS_PCL_LOOKUP_NUMBER_ENT                        lookupNum;
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT    mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];
    direction = (CPSS_PCL_DIRECTION_ENT)inArgs[2];
    lookupNum = (CPSS_PCL_LOOKUP_NUMBER_ENT)inArgs[3];
    mode = (CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT)inArgs[4];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChPclPortLookupCfgTabAccessModeSet(
        devNum, port, direction, lookupNum,
        0 /*subLookupNum*/, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPclPortLookupCfgTabAccessModeSet_V1 function
* @endinternal
*
* @brief   Configures VLAN/PORT access mode to Ingress or Egress PCL
*         configuration table perlookup.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
* @note   NOT APPLICABLE DEVICES:  Configures VLAN/PORT access mode to Ingress or Egress PCL
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
*
* @note - for CH2 and above- full support,
*       - for CH1 supports
*       Ingress direction only and set same access type
*       for both lookups regardless the <lookupNum> parameter value
*
*/
static CMD_STATUS wrCpssDxChPclPortLookupCfgTabAccessModeSet_V1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                         result;

    GT_U8                                             devNum;
    GT_PORT_NUM                                       port;
    CPSS_PCL_DIRECTION_ENT                            direction;
    CPSS_PCL_LOOKUP_NUMBER_ENT                        lookupNum;
    GT_U32                                            subLookupNum;
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT    mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum       = (GT_U8)inArgs[0];
    port         = (GT_PORT_NUM)inArgs[1];
    direction    = (CPSS_PCL_DIRECTION_ENT)inArgs[2];
    lookupNum    = (CPSS_PCL_LOOKUP_NUMBER_ENT)inArgs[3];
    subLookupNum = (GT_U32)inArgs[4];
    mode         = (CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT)inArgs[5];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChPclPortLookupCfgTabAccessModeSet(
        devNum, port, direction, lookupNum,
        subLookupNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/* Table: cpssDxChPclCfgTblAccessMode
*
* Description:
*     PCL Configuration table
*
* Fields:
*    ipclAccMode     - Ingress Policy local/non-local mode
*    ipclMaxDevPorts - Ingress Policy support 32/64 port remote devices
*    ipclDevPortBase - Ingress Policy Configuration table access base
*    epclAccMode     - Egress PCL local/non-local mode
*    epclMaxDevPorts - Egress PCL support 32/64 port remote devices
*    epclDevPortBase - Egress PCL Configuration table access base
*
*
* Comments:
*
*   - for 98DX2X5 - full support,
*   - for 98DX2x3,98DX2x0,98DX2x7, 98DX1x6, 98DX1x7 and so on supports
*     Ingress direction only with 32 ports per device and the
*     non-local-device-entries-segment-base == 0
*
*
*/

/**
* @internal wrCpssDxChPclCfgTblAccessModeSet function
* @endinternal
*
* @brief   Configures global access mode to Ingress or Egress PCL configuration tables.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
*/
static CMD_STATUS wrCpssDxChPclCfgTblAccessModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                                  result;
    GT_U8                                      devNum;
    CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_STC      accModePtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];


    accModePtr.ipclAccMode = (CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_ENT)inFields[0];
    accModePtr.ipclMaxDevPorts =
                           (CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_ENT)inFields[1];
    accModePtr.ipclDevPortBase =
                           (CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE_ENT)inFields[2];
    accModePtr.epclAccMode = (CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_ENT)inFields[3];
    accModePtr.epclMaxDevPorts =
                           (CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_ENT)inFields[4];
    accModePtr.epclDevPortBase =
                           (CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE_ENT)inFields[5];


    /* call cpss api function */
    result = cpssDxChPclCfgTblAccessModeSet(devNum, &accModePtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclCfgTblAccessModeGetFirst function
* @endinternal
*
* @brief   Get global access mode to Ingress or Egress PCL configuration tables.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
*/
static CMD_STATUS wrCpssDxChPclCfgTblAccessModeGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                                  result;
    GT_U8                                      devNum;
    CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_STC      accModePtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];


    result = cpssDxChPclCfgTblAccessModeGet(devNum, &accModePtr);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = accModePtr.ipclAccMode;
    inFields[1] = accModePtr.ipclMaxDevPorts;
    inFields[2] = accModePtr.ipclDevPortBase;
    inFields[3] = accModePtr.epclAccMode;
    inFields[4] = accModePtr.epclMaxDevPorts;
    inFields[5] = accModePtr.epclDevPortBase;


    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                                inFields[3], inFields[4], inFields[5]);

    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/* Table: cpssDxChPclCfgTbl
*
* Description:
*     PCL Configuration table
*
* Fields:
*    enableLookup           - Enable Lookup
*                             GT_TRUE - enable Lookup
*                             GT_FALSE - disable Lookup
*    pclId                  - PCL-ID bits in the TCAM search key.
*    groupKeyTypes.nonIpKey - type of TCAM search key for NON-IP packets.
*    groupKeyTypes.ipv4Key  - type of TCAM search key for IPV4 packets
*    groupKeyTypes.ipv6Key  - type of TCAM search key for IPV6 packets
*
*
* Comments:
*
*       For 98DX2X5 - full support (beside the Stadard IPV6 DIP key)
*       For 98DX2X3 - ingerss only, keys for not-IP, IPV4 and IPV6 must
*           be of the same size, Stadard IPV6 DIP key allowed only on lookup1
*
*
*/

/**
* @internal wrCpssDxChPclCfgTblSet function
* @endinternal
*
* @brief   PCL Configuration table entry's lookup configuration for interface.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
*/
static CMD_STATUS wrCpssDxChPclCfgTblSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                       result;
    GT_U8                           devNum;
    CPSS_INTERFACE_INFO_STC         interfaceInfoPtr;
    CPSS_PCL_DIRECTION_ENT          direction;
    CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum;
    CPSS_DXCH_PCL_LOOKUP_CFG_STC    lookupCfgPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* to support the not seted/geted members of structures */
    cpssOsMemSet(&interfaceInfoPtr, 0, sizeof(interfaceInfoPtr));
    cpssOsMemSet(&lookupCfgPtr, 0, sizeof(lookupCfgPtr));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    interfaceInfoPtr.type = (CPSS_INTERFACE_TYPE_ENT)inArgs[1];

    switch (interfaceInfoPtr.type)
    {
    case CPSS_INTERFACE_PORT_E:
        interfaceInfoPtr.devPort.hwDevNum = (GT_HW_DEV_NUM)inArgs[2];
        interfaceInfoPtr.devPort.portNum = (GT_PORT_NUM)inArgs[3];
        CONVERT_DEV_PORT_DATA_MAC(interfaceInfoPtr.devPort.hwDevNum ,
                             interfaceInfoPtr.devPort.portNum);
        break;

    case CPSS_INTERFACE_TRUNK_E:
        interfaceInfoPtr.trunkId = (GT_TRUNK_ID)inArgs[4];
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(interfaceInfoPtr.trunkId);
        break;

    case CPSS_INTERFACE_VIDX_E:
        interfaceInfoPtr.vidx = (GT_U16)inArgs[5];
        break;

    case CPSS_INTERFACE_VID_E:
        interfaceInfoPtr.vlanId = (GT_U16)inArgs[6];
        break;

    default:
        break;
    }

    direction = (CPSS_PCL_DIRECTION_ENT)inArgs[7];
    lookupNum = (CPSS_PCL_LOOKUP_NUMBER_ENT)inArgs[8];


    lookupCfgPtr.enableLookup = (GT_BOOL)inFields[0];
    lookupCfgPtr.pclId = (GT_U32)inFields[1];
    lookupCfgPtr.groupKeyTypes.nonIpKey =
                                (CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT)inFields[2];
    lookupCfgPtr.groupKeyTypes.ipv4Key =
                                (CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT)inFields[3];
    lookupCfgPtr.groupKeyTypes.ipv6Key =
                                (CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT)inFields[4];

    /* call cpss api function */
    result = pg_wrap_cpssDxChPclCfgTblSet(devNum, &interfaceInfoPtr, direction,
                                              lookupNum, &lookupCfgPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChPclCfgTblGet function
* @endinternal
*
* @brief   PCL Configuration table entry's lookup configuration for interface.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
*/
static CMD_STATUS wrCpssDxChPclCfgTblGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                       result;
    GT_U8                           devNum;
    CPSS_INTERFACE_INFO_STC         interfaceInfoPtr;
    CPSS_PCL_DIRECTION_ENT          direction;
    CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum;
    CPSS_DXCH_PCL_LOOKUP_CFG_STC    lookupCfgPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* to support the not seted/geted members of structures */
    cpssOsMemSet(&interfaceInfoPtr, 0, sizeof(interfaceInfoPtr));
    cpssOsMemSet(&lookupCfgPtr, 0, sizeof(lookupCfgPtr));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    interfaceInfoPtr.type = (CPSS_INTERFACE_TYPE_ENT)inArgs[1];

    switch (interfaceInfoPtr.type)
    {
        case CPSS_INTERFACE_PORT_E:
            interfaceInfoPtr.devPort.hwDevNum = (GT_HW_DEV_NUM)inArgs[2];
            interfaceInfoPtr.devPort.portNum = (GT_PORT_NUM)inArgs[3];
            CONVERT_DEV_PORT_DATA_MAC(interfaceInfoPtr.devPort.hwDevNum ,
                                 interfaceInfoPtr.devPort.portNum);
            break;

        case CPSS_INTERFACE_TRUNK_E:
            interfaceInfoPtr.trunkId = (GT_TRUNK_ID)inArgs[4];
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(interfaceInfoPtr.trunkId);
            break;

        case CPSS_INTERFACE_VIDX_E:
            interfaceInfoPtr.vidx = (GT_U16)inArgs[5];
            break;

        case CPSS_INTERFACE_VID_E:
            interfaceInfoPtr.vlanId = (GT_U16)inArgs[6];
            break;

        default:
            break;
    }

    direction = (CPSS_PCL_DIRECTION_ENT)inArgs[7];
    lookupNum = (CPSS_PCL_LOOKUP_NUMBER_ENT)inArgs[8];


    result = pg_wrap_cpssDxChPclCfgTblGet(devNum, &interfaceInfoPtr, direction,
                                              lookupNum, &lookupCfgPtr);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = lookupCfgPtr.enableLookup;
    inFields[1] = lookupCfgPtr.pclId;
    inFields[2] = lookupCfgPtr.groupKeyTypes.nonIpKey;
    inFields[3] = lookupCfgPtr.groupKeyTypes.ipv4Key;
    inFields[4] = lookupCfgPtr.groupKeyTypes.ipv6Key;



    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                                           inFields[3], inFields[4]);

    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}


/*****************Table:cpssDxCh3PclCfgTbl **************************************/

/**
* @internal wrCpssDxCh3PclCfgTblSet function
* @endinternal
*
* @brief   PCL Configuration table entry's lookup configuration for interface.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DxCh2 and above - full support (beside the Stadard IPV6 DIP key)
*       For DxCh1 - ingress only, keys for not-IP, IPV4 and IPV6 must
*       be of the same size, Stadard IPV6 DIP key allowed only on lookup1
*
*/
static CMD_STATUS wrCpssDxCh3PclCfgTblSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                       result;
    GT_U8                           devNum;
    CPSS_INTERFACE_INFO_STC         interfaceInfo;
    CPSS_PCL_DIRECTION_ENT          direction;
    CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum;
    CPSS_DXCH_PCL_LOOKUP_CFG_STC    lookupCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* to support the not seted/geted members of structures */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    interfaceInfo.type = (CPSS_INTERFACE_TYPE_ENT)inArgs[1];

    switch (interfaceInfo.type)
    {
        case CPSS_INTERFACE_PORT_E:
            interfaceInfo.devPort.hwDevNum = (GT_HW_DEV_NUM)inArgs[2];
            interfaceInfo.devPort.portNum = (GT_PORT_NUM)inArgs[3];
            CONVERT_DEV_PORT_DATA_MAC(interfaceInfo.devPort.hwDevNum ,
                                 interfaceInfo.devPort.portNum);
            break;

        case CPSS_INTERFACE_TRUNK_E:
            interfaceInfo.trunkId = (GT_TRUNK_ID)inArgs[4];
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(interfaceInfo.trunkId);
            break;

        case CPSS_INTERFACE_VIDX_E:
            interfaceInfo.vidx = (GT_U16)inArgs[5];
            break;

        case CPSS_INTERFACE_VID_E:
            interfaceInfo.vlanId = (GT_U16)inArgs[6];
            break;

        default:
            break;
    }

    direction = (CPSS_PCL_DIRECTION_ENT)inArgs[7];
    lookupNum = (CPSS_PCL_LOOKUP_NUMBER_ENT)inArgs[8];


    lookupCfg.enableLookup = (GT_BOOL)inFields[0];
    lookupCfg.dualLookup   = (GT_BOOL)inFields[1];
    lookupCfg.pclId = (GT_U32)inFields[2];
    lookupCfg.groupKeyTypes.nonIpKey =
                                (CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT)inFields[3];
    lookupCfg.groupKeyTypes.ipv4Key =
                                (CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT)inFields[4];
    lookupCfg.groupKeyTypes.ipv6Key =
                                (CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT)inFields[5];
    lookupCfg.exactMatchLookupSubProfileId =
                                (GT_U32)inFields[6];

    /* call cpss api function */
    result = pg_wrap_cpssDxChPclCfgTblSet(devNum, &interfaceInfo, direction,
                                              lookupNum, &lookupCfg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PclCfgTblGet function
* @endinternal
*
* @brief   PCL Configuration table entry's lookup configuration for interface.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DxCh2 and above - full support (beside the Standard IPV6 DIP key)
*       For DxCh1 - ingress only, keys for not-IP, IPV4 and IPV6 must
*       be the same size, Standard IPV6 DIP key allowed only on lookup1
*
*/
static CMD_STATUS wrCpssDxCh3PclCfgTblGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                       result;
    GT_U8                           devNum;
    CPSS_INTERFACE_INFO_STC         interfaceInfo;
    CPSS_PCL_DIRECTION_ENT          direction;
    CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum;
    CPSS_DXCH_PCL_LOOKUP_CFG_STC    lookupCfg;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* to support the not seted/geted members of structures */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    interfaceInfo.type = (CPSS_INTERFACE_TYPE_ENT)inArgs[1];

    switch (interfaceInfo.type)
    {
        case CPSS_INTERFACE_PORT_E:
            interfaceInfo.devPort.hwDevNum = (GT_HW_DEV_NUM)inArgs[2];
            interfaceInfo.devPort.portNum = (GT_PORT_NUM)inArgs[3];
            CONVERT_DEV_PORT_DATA_MAC(interfaceInfo.devPort.hwDevNum ,
                                 interfaceInfo.devPort.portNum);
            break;

        case CPSS_INTERFACE_TRUNK_E:
            interfaceInfo.trunkId = (GT_TRUNK_ID)inArgs[4];
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(interfaceInfo.trunkId);
            break;

        case CPSS_INTERFACE_VIDX_E:
            interfaceInfo.vidx = (GT_U16)inArgs[5];
            break;

        case CPSS_INTERFACE_VID_E:
            interfaceInfo.vlanId = (GT_U16)inArgs[6];
            break;

        default:
            break;
    }

    direction = (CPSS_PCL_DIRECTION_ENT)inArgs[7];
    lookupNum = (CPSS_PCL_LOOKUP_NUMBER_ENT)inArgs[8];


    result = pg_wrap_cpssDxChPclCfgTblGet(devNum, &interfaceInfo, direction,
                                              lookupNum, &lookupCfg);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = lookupCfg.enableLookup;
    inFields[1] = lookupCfg.dualLookup;
    inFields[2] = lookupCfg.pclId;
    inFields[3] = lookupCfg.groupKeyTypes.nonIpKey;
    inFields[4] = lookupCfg.groupKeyTypes.ipv4Key;
    inFields[5] = lookupCfg.groupKeyTypes.ipv6Key;
    inFields[6] = lookupCfg.exactMatchLookupSubProfileId;


    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                                           inFields[3], inFields[4],inFields[5],inFields[6]);

    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssDxChXCatPclCfgTblSet function
* @endinternal
*
* @brief   PCL Configuration table entry's lookup configuration for interface.
*
* @note   APPLICABLE DEVICES:      All XCAT devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DxCh2 and above - full support (beside the Stadard IPV6 DIP key)
*       For DxCh1 - ingress only, keys for not-IP, IPV4 and IPV6 must
*       be of the same size, Stadard IPV6 DIP key allowed only on lookup1
*
*/
static CMD_STATUS wrCpssDxChXCatPclCfgTblSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                       result;
    GT_U8                           devNum;
    CPSS_INTERFACE_INFO_STC         interfaceInfo;
    CPSS_PCL_DIRECTION_ENT          direction;
    CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum;
    CPSS_DXCH_PCL_LOOKUP_CFG_STC    lookupCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* to support the not seted/geted members of structures */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    interfaceInfo.type = (CPSS_INTERFACE_TYPE_ENT)inArgs[1];

    switch (interfaceInfo.type)
    {
        case CPSS_INTERFACE_PORT_E:
            interfaceInfo.devPort.hwDevNum = (GT_HW_DEV_NUM)inArgs[2];
            interfaceInfo.devPort.portNum = (GT_PORT_NUM)inArgs[3];
            CONVERT_DEV_PORT_DATA_MAC(interfaceInfo.devPort.hwDevNum ,
                                 interfaceInfo.devPort.portNum);
            break;

        case CPSS_INTERFACE_TRUNK_E:
            interfaceInfo.trunkId = (GT_TRUNK_ID)inArgs[4];
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(interfaceInfo.trunkId);
            break;

        case CPSS_INTERFACE_VIDX_E:
            interfaceInfo.vidx = (GT_U16)inArgs[5];
            break;

        /* vlanId and index use the same field in Galtis */
        case CPSS_INTERFACE_VID_E:
            interfaceInfo.vlanId = (GT_U16)inArgs[6];
            break;

        /* vlanId and index use the same field in Galtis */
        case CPSS_INTERFACE_INDEX_E:
            interfaceInfo.index = (GT_U16)inArgs[6];
            break;

        default:
            break;
    }

    direction = (CPSS_PCL_DIRECTION_ENT)inArgs[7];
    lookupNum = (CPSS_PCL_LOOKUP_NUMBER_ENT)inArgs[8];


    lookupCfg.enableLookup = (GT_BOOL)inFields[0];
    lookupCfg.dualLookup   = (GT_BOOL)inFields[1];
    lookupCfg.pclId        = (GT_U32)inFields[2];
    lookupCfg.pclIdL01      = (GT_U32)inFields[3];
    lookupCfg.groupKeyTypes.nonIpKey =
        (CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT)inFields[4];
    lookupCfg.groupKeyTypes.ipv4Key =
        (CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT)inFields[5];
    lookupCfg.groupKeyTypes.ipv6Key =
        (CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT)inFields[6];
    if (numFields > 7)
    {
        /* Bobcat2, Caelum, Bobcat3 and above devices. */
        lookupCfg.udbKeyBitmapEnable    = (GT_BOOL)inFields[7];
        lookupCfg.tcamSegmentMode       = (CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_ENT)inFields[8];
    }

    /* call cpss api function */
    result = pg_wrap_cpssDxChPclCfgTblSet(devNum, &interfaceInfo, direction,
                                              lookupNum, &lookupCfg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChXCatPclCfgTblGet_util function
* @endinternal
*
* @brief   PCL Configuration table entry's lookup configuration for interface.
*
* @note   APPLICABLE DEVICES:      All XCAT devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DxCh2 and above - full support (beside the Standard IPV6 DIP key)
*       For DxCh1 - ingress only, keys for not-IP, IPV4 and IPV6 must
*       be the same size, Standard IPV6 DIP key allowed only on lookup1
*
*/
static CMD_STATUS wrCpssDxChXCatPclCfgTblGet_util
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                       result;
    GT_U8                           devNum;
    CPSS_INTERFACE_INFO_STC         interfaceInfo;
    CPSS_PCL_DIRECTION_ENT          direction;
    CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum;
    CPSS_DXCH_PCL_LOOKUP_CFG_STC    lookupCfg;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* to support the not seted/geted members of structures */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    interfaceInfo.type = (CPSS_INTERFACE_TYPE_ENT)inArgs[1];

    switch (interfaceInfo.type)
    {
        case CPSS_INTERFACE_PORT_E:
            interfaceInfo.devPort.hwDevNum = (GT_HW_DEV_NUM)inArgs[2];
            interfaceInfo.devPort.portNum = (GT_PORT_NUM)inArgs[3];
            CONVERT_DEV_PORT_DATA_MAC(interfaceInfo.devPort.hwDevNum ,
                                 interfaceInfo.devPort.portNum);
            break;

        case CPSS_INTERFACE_TRUNK_E:
            interfaceInfo.trunkId = (GT_TRUNK_ID)inArgs[4];
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(interfaceInfo.trunkId);
            break;

        case CPSS_INTERFACE_VIDX_E:
            interfaceInfo.vidx = (GT_U16)inArgs[5];
            break;

        /* vlanId and index use the same field in Galtis */
        case CPSS_INTERFACE_VID_E:
            interfaceInfo.vlanId = (GT_U16)inArgs[6];
            break;

        /* vlanId and index use the same field in Galtis */
        case CPSS_INTERFACE_INDEX_E:
            interfaceInfo.index  = (GT_U32)inArgs[6];
            break;

        default:
            break;
    }

    direction = (CPSS_PCL_DIRECTION_ENT)inArgs[7];
    lookupNum = (CPSS_PCL_LOOKUP_NUMBER_ENT)inArgs[8];


    result = pg_wrap_cpssDxChPclCfgTblGet(devNum, &interfaceInfo, direction,
                                              lookupNum, &lookupCfg);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = lookupCfg.enableLookup;
    inFields[1] = lookupCfg.dualLookup;
    inFields[2] = lookupCfg.pclId;
    inFields[3] = lookupCfg.pclIdL01;
    inFields[4] = lookupCfg.groupKeyTypes.nonIpKey;
    inFields[5] = lookupCfg.groupKeyTypes.ipv4Key;
    inFields[6] = lookupCfg.groupKeyTypes.ipv6Key;


    /* pack and output table fields */
    fieldOutput(
        "%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
        inFields[3], inFields[4], inFields[5], inFields[6]);

    if (numFields > 7)
    {
        /* Bobcat2, Caelum, Bobcat3 and above devices. */
        fieldOutputSetAppendMode();
        fieldOutput(
            "%d%d", lookupCfg.udbKeyBitmapEnable, lookupCfg.tcamSegmentMode);
    }

    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChXCatPclCfgTblGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChXCatPclCfgTblGet_util(
        inArgs, inFields, 7 /*numFields*/, outArgs);
}

static CMD_STATUS wrCpssDxChPclLion3CfgTableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChXCatPclCfgTblGet_util(
        inArgs, inFields, 9 /*numFields*/, outArgs);
}

/**
* @internal wrCpssDxChPclIngressPolicyEnable function
* @endinternal
*
* @brief   Enables Ingress Policy.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
*/
static CMD_STATUS wrCpssDxChPclIngressPolicyEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8      devNum;
    GT_BOOL    enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPclIngressPolicyEnable(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPclIngressPolicyEnableGet function
* @endinternal
*
* @brief   Get Ingress Policy Enable state .
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on null pointer parameter
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
*/
static CMD_STATUS wrCpssDxChPclIngressPolicyEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8      devNum;
    GT_BOOL    enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPclIngressPolicyEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPclTcamRuleSizeModeSet function
* @endinternal
*
* @brief   Set TCAM Rules size mode.
*         The rules TCAM may be configured to contain short rules (24 bytes), long
*         rules (48 bytes), or a mix of short rules and long rules.
*         The rule size may be set by global configuration or may be defined
*         dynamical according to PCL Configuration table settings.
*
* @note   APPLICABLE DEVICES:      98DX2x3,98DX2x0,98DX2x7, 98DX1x6, 98DX1x7  devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
*/
static CMD_STATUS wrCpssDxChPclTcamRuleSizeModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8                     devNum;
    GT_BOOL                   mixedRuleSizeMode;
    CPSS_PCL_RULE_SIZE_ENT    ruleSize;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    mixedRuleSizeMode = (GT_BOOL)inArgs[1];
    ruleSize = (CPSS_PCL_RULE_SIZE_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPclTcamRuleSizeModeSet(devNum, mixedRuleSizeMode,
                                                             ruleSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPclTcamRuleSizeModeGet function
* @endinternal
*
* @brief   Get TCAM Rules size mode.
*         The rules TCAM may be configured to contain short rules (24 bytes), long
*         rules (48 bytes), or a mix of short rules and long rules.
*         The rule size may be set by global configuration or may be defined
*         dynamical according to PCL Configuration table settings.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond.
* @note   NOT APPLICABLE DEVICES:  DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PTR               - on NULL pointer
*/
static CMD_STATUS wrCpssDxChPclTcamRuleSizeModeGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8                     devNum;
    GT_BOOL                   mixedRuleSizeMode;
    CPSS_PCL_RULE_SIZE_ENT    ruleSize;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPclTcamRuleSizeModeGet(devNum, &mixedRuleSizeMode,
                                                             &ruleSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", mixedRuleSizeMode, ruleSize);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPclTwoLookupsCpuCodeResolution function
* @endinternal
*
* @brief   Resolve the result CPU Code if both lookups has action commands
*         are either both TRAP or both MIRROR To CPU
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
*/
static CMD_STATUS wrCpssDxChPclTwoLookupsCpuCodeResolution

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    CPSS_PCL_LOOKUP_NUMBER_ENT    lookupNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    lookupNum = (CPSS_PCL_LOOKUP_NUMBER_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPclTwoLookupsCpuCodeResolution(devNum, lookupNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChPclInvalidUdbCmdSet function
* @endinternal
*
* @brief   Set the command that is applied when the policy key <User-Defined>
*         field cannot be extracted from the packet due to lack of header
*         depth (i.e, the field resides deeper than 128 bytes into the packet).
*         This command is NOT applied when the policy key <User-Defined>
*         field cannot be extracted due to the offset being relative to a layer
*         start point that does not exist in the packet. (e.g. the offset is relative
*         to the IP header but the packet is a non-IP).
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
*/
static CMD_STATUS wrCpssDxChPclInvalidUdbCmdSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                      result;

    GT_U8                          devNum;
    CPSS_DXCH_UDB_ERROR_CMD_ENT    udbErrorCmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    udbErrorCmd = (CPSS_DXCH_UDB_ERROR_CMD_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPclInvalidUdbCmdSet(devNum, udbErrorCmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPclInvalidUdbCmdGet function
* @endinternal
*
* @brief   Get the command that is applied when the policy key <User-Defined>
*         field cannot be extracted from the packet due to lack of header
*         depth (i.e, the field resides deeper than 128 bytes into the packet).
*         This command is NOT applied when the policy key <User-Defined>
*         field cannot be extracted due to the offset being relative to a layer
*         start point that does not exist in the packet. (e.g. the offset is relative
*         to the IP header but the packet is a non-IP).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PTR               - on NULL pointer
*/
static CMD_STATUS wrCpssDxChPclInvalidUdbCmdGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                      result;

    GT_U8                          devNum;
    CPSS_DXCH_UDB_ERROR_CMD_ENT    udbErrorCmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPclInvalidUdbCmdGet(devNum, &udbErrorCmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", udbErrorCmd);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPclIpLengthCheckModeSet function
* @endinternal
*
* @brief   Set the mode of checking IP packet length.
*         To determine if an IP packet is a valid IP packet, one of the checks is a
*         length check to find out if the total IP length field reported in the
*         IP header is less or equal to the packet's length.
*         This function determines the check mode.
*         The results of this check sets the policy key <IP Header OK>.
*         The result is also used by the router engine to determine whether
*         to forward or trap/drop the packet.
*         There are two check modes:
*         1. For IPv4: ip_total_length <= packet's byte count
*         For IPv6: ip_total_length + 40 <= packet's byte count
*         2. For IPv4: ip_total_length + L3 Offset + 4 (CRC) <= packet's byte count,
*         For IPv6: ip_total_length + 40 +L3 Offset + 4 (CRC) <= packet's
*         byte count
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
*/
static CMD_STATUS wrCpssDxChPclIpLengthCheckModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                 result;

    GT_U8                                     devNum;
    CPSS_DXCH_PCL_IP_LENGTH_CHECK_MODE_ENT    mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    mode = (CPSS_DXCH_PCL_IP_LENGTH_CHECK_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPclIpLengthCheckModeSet(devNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPclIpLengthCheckModeGet function
* @endinternal
*
* @brief   Get the mode of checking IP packet length.
*         To determine if an IP packet is a valid IP packet, one of the checks is a
*         length check to find out if the total IP length field reported in the
*         IP header is less or equal to the packet's length.
*         This function determines the check mode.
*         The results of this check sets the policy key <IP Header OK>.
*         The result is also used by the router engine to determine whether
*         to forward or trap/drop the packet.
*         There are two check modes:
*         1. For IPv4: ip_total_length <= packet's byte count
*         For IPv6: ip_total_length + 40 <= packet's byte count
*         2. For IPv4: ip_total_length + L3 Offset + 4 (CRC) <= packet's byte count,
*         For IPv6: ip_total_length + 40 +L3 Offset + 4 (CRC) <= packet's
*         byte count
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PTR               - on NULL pointer
*/
static CMD_STATUS wrCpssDxChPclIpLengthCheckModeGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                 result;

    GT_U8                                     devNum;
    CPSS_DXCH_PCL_IP_LENGTH_CHECK_MODE_ENT    mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPclIpLengthCheckModeGet(devNum, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);
    return CMD_OK;
}


/**
* @internal wrCpssDxChPclIpPayloadMinSizeSet function
* @endinternal
*
* @brief   Set the IP payload minimum size. This value will determine the Layer 4
*         validity
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The L4_Valid in the packet descriptor will be cleared if:
*       For IPv4: total length - header length < IP payload minimum size
*       For IPv6: payload length < IP payload minimum size
*
*/
static CMD_STATUS wrCpssDxChPclIpPayloadMinSizeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    ipPayloadMinSize;


    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ipPayloadMinSize = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPclIpPayloadMinSizeSet(devNum, ipPayloadMinSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclIpPayloadMinSizeGet function
* @endinternal
*
* @brief   Get the IP payload minimum size. This value will determine the Layer 4
*         validity
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The L4_Valid in the packet descriptor will be cleared if:
*       For IPv4: total length - header length < IP payload minimum size
*       For IPv6: payload length < IP payload minimum size
*
*/
static CMD_STATUS wrCpssDxChPclIpPayloadMinSizeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    ipPayloadMinSize;


    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPclIpPayloadMinSizeGet(devNum, &ipPayloadMinSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", ipPayloadMinSize);

    return CMD_OK;
}


/**
* @internal wrCpssDxChPclRuleStateGet function
* @endinternal
*
* @brief   Get state (valid or not) of the rule and it's size
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
static CMD_STATUS wrCpssDxChPclRuleStateGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                 result;

    GT_U8                     devNum;
    GT_U32                    ruleIndex;
    GT_BOOL                   validPtr;
    CPSS_PCL_RULE_SIZE_ENT    ruleSizePtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ruleIndex = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = pg_wrap_cpssDxChPclRuleStateGet(devNum, ruleIndex, &validPtr,
                                                     &ruleSizePtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", validPtr, ruleSizePtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPclPolicyRuleGetFirst function
* @endinternal
*
* @brief   The function gets the Policy Rule Mask, Pattern and Action in in te rowIndex
*         in Hw format.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChPclPolicyRuleGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PCL_RULE_SIZE_ENT      ruleSize;
    GT_U32                      ruleIndex;
    GT_U32                      maskPtr[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS];
    GT_U32                      patternPtr[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS];
    GT_U32                      actionPtr[CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS];

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ruleSize = (CPSS_PCL_RULE_SIZE_ENT)inArgs[1];
    ruleIndex = (GT_U32)inArgs[2];

    /* clean pattern and mask */
    cmdOsMemSet(maskPtr, 0, sizeof(maskPtr));
    cmdOsMemSet(patternPtr, 0, sizeof(patternPtr));

    result = pg_wrap_cpssDxChPclRuleGet(devNum, ruleSize, ruleIndex,
                                maskPtr, patternPtr, actionPtr);
    inFields[0] = maskPtr[0];
    inFields[1] = maskPtr[1];
    inFields[2] = maskPtr[2];
    inFields[3] = maskPtr[3];
    inFields[4] = maskPtr[4];
    inFields[5] = maskPtr[5];
    inFields[6] = maskPtr[6];
    inFields[7] = maskPtr[7];
    inFields[8] = maskPtr[8];
    inFields[9] = maskPtr[9];
    inFields[10] = maskPtr[10];
    inFields[11] = maskPtr[11];

    inFields[12] = patternPtr[0];
    inFields[13] = patternPtr[1];
    inFields[14] = patternPtr[2];
    inFields[15] = patternPtr[3];
    inFields[16] = patternPtr[4];
    inFields[17] = patternPtr[5];
    inFields[18] = patternPtr[6];
    inFields[19] = patternPtr[7];
    inFields[20] = patternPtr[8];
    inFields[21] = patternPtr[9];
    inFields[22] = patternPtr[10];
    inFields[23] = patternPtr[11];

    inFields[24] = actionPtr[0];
    inFields[25] = actionPtr[1];
    inFields[26] = actionPtr[2];


    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0], inFields[1], inFields[2], inFields[3],
                inFields[4], inFields[5], inFields[6], inFields[7],
                inFields[8], inFields[9], inFields[10], inFields[11],
                inFields[12], inFields[13], inFields[14], inFields[15],
                inFields[16], inFields[17], inFields[18], inFields[19],
                inFields[20], inFields[21], inFields[22], inFields[23],
                inFields[24], inFields[25], inFields[26]);

    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclCfgTblEntryGet function
* @endinternal
*
* @brief   Gets the PCL configuration table entry in Hw format.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_STATE             - in case of already bound pclHwId with same
*                                       combination of slot/direction
*
* @note for 98DX2X3 ingess direction only
*
*/
static CMD_STATUS wrCpssDxChPclCfgTblEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                 result;

    GT_U8                     devNum;
    CPSS_PCL_DIRECTION_ENT    direction;
    GT_U32                    entryIndex;
    GT_U32                    pclCfgTblEntry[4];

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PCL_DIRECTION_ENT)inArgs[1];
    entryIndex = (GT_U32)inArgs[2];

    pclCfgTblEntry[0] = 0;
    pclCfgTblEntry[1] = 0;
    pclCfgTblEntry[2] = 0;
    pclCfgTblEntry[3] = 0;

    /* call cpss api function */
    result = pg_wrap_cpssDxChPclCfgTblEntryGet(
        devNum, direction,
        CPSS_PCL_LOOKUP_0_E,
        entryIndex,
        &(pclCfgTblEntry[0]));

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%X%X%X%X",
        pclCfgTblEntry[0], pclCfgTblEntry[1],
        pclCfgTblEntry[2], pclCfgTblEntry[3]);
    return CMD_OK;
}


/**
* @internal wrCpssDxChPclLookupCfgPortListEnableSet function
* @endinternal
*
* @brief   The function enables/disables using port-list in search keys.
*
* @note   APPLICABLE DEVICES:      xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChPclLookupCfgPortListEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                      devNum;
    CPSS_PCL_DIRECTION_ENT                     direction;
    CPSS_PCL_LOOKUP_NUMBER_ENT                 lookupNum;
    GT_U32                                     subLookupNum;
    GT_BOOL                                    enable;
    GT_STATUS                                  result;

    devNum       = (GT_U8)                      inArgs[0];
    direction    = (CPSS_PCL_DIRECTION_ENT)     inArgs[1];
    lookupNum    = (CPSS_PCL_LOOKUP_NUMBER_ENT) inArgs[2];
    subLookupNum = (GT_U32)                     inArgs[3];
    enable       = (GT_U32)                     inArgs[4];

    result = pg_wrap_cpssDxChPclLookupCfgPortListEnableSet(
        devNum, direction, lookupNum, subLookupNum, enable);

    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclLookupCfgPortListEnableGet function
* @endinternal
*
* @brief   The function gets enable/disable state of
*         using port-list in search keys.
*
* @note   APPLICABLE DEVICES:      xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChPclLookupCfgPortListEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                      devNum;
    CPSS_PCL_DIRECTION_ENT                     direction;
    CPSS_PCL_LOOKUP_NUMBER_ENT                 lookupNum;
    GT_U32                                     subLookupNum;
    GT_BOOL                                    enable;
    GT_STATUS                                  result;

    devNum       = (GT_U8)                      inArgs[0];
    direction    = (CPSS_PCL_DIRECTION_ENT)     inArgs[1];
    lookupNum    = (CPSS_PCL_LOOKUP_NUMBER_ENT) inArgs[2];
    subLookupNum = (GT_U32)                     inArgs[3];

    result = pg_wrap_cpssDxChPclLookupCfgPortListEnableGet(
        devNum, direction, lookupNum, subLookupNum, &enable);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    galtisOutput(
        outArgs, GT_OK, "%d", enable);

    return CMD_OK;
}

/* 98DX2x5 only API */

/**
* @internal wrCpssDxCh2PclEgressPolicyEnable function
* @endinternal
*
* @brief   Enables Egress Policy.
*
* @note   APPLICABLE DEVICES:      98DX2x5 devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
*/
static CMD_STATUS wrCpssDxCh2PclEgressPolicyEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8      devNum;
    GT_BOOL    enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxCh2PclEgressPolicyEnable(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxCh2PclEgressPolicyEnableGet function
* @endinternal
*
* @brief   Get status of Egress Policy.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PTR               - on NULL poinet
*/
static CMD_STATUS wrCpssDxCh2PclEgressPolicyEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8      devNum;
    GT_BOOL    enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxCh2PclEgressPolicyEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxCh2EgressPclPacketTypesSet function
* @endinternal
*
* @brief   Enables/disables Egress PCL (EPCL) for set of packet types on port
*
* @note   APPLICABLE DEVICES:      98DX2X5 devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
*
* @note - after reset EPCL disabled for all packet types on all ports
*
*/
static CMD_STATUS wrCpssDxCh2EgressPclPacketTypesSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8                                devNum;
    GT_PHYSICAL_PORT_NUM                                port;
    CPSS_DXCH_PCL_EGRESS_PKT_TYPE_ENT    pktType;
    GT_BOOL                              enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    pktType = (CPSS_DXCH_PCL_EGRESS_PKT_TYPE_ENT)inArgs[2];
    enable = (GT_BOOL)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxCh2EgressPclPacketTypesSet(devNum, port, pktType, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxCh2EgressPclPacketTypesGet function
* @endinternal
*
* @brief   Get status of Egress PCL (EPCL) for set of packet types on port
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PTR               - on NULL pointer
*/
static CMD_STATUS wrCpssDxCh2EgressPclPacketTypesGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8                                devNum;
    GT_PHYSICAL_PORT_NUM                 port;
    CPSS_DXCH_PCL_EGRESS_PKT_TYPE_ENT    pktType;
    GT_BOOL                              enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    pktType = (CPSS_DXCH_PCL_EGRESS_PKT_TYPE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxCh2EgressPclPacketTypesGet(devNum, port, pktType, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}


/**
* @internal wrCpssDxCh2PclTcpUdpPortComparatorSet function
* @endinternal
*
* @brief   Configure TCP or UDP Port Comparator entry
*
* @note   APPLICABLE DEVICES:      98DX2X5 devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
*/
static CMD_STATUS wrCpssDxCh2PclTcpUdpPortComparatorSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8                              devNum;
    CPSS_PCL_DIRECTION_ENT             direction;
    CPSS_L4_PROTOCOL_ENT               l4Protocol;
    GT_U8                              entryIndex;
    CPSS_L4_PROTOCOL_PORT_TYPE_ENT     l4PortType;
    CPSS_COMPARE_OPERATOR_ENT          compareOperator;
    GT_U16                             value;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PCL_DIRECTION_ENT)inArgs[1];
    l4Protocol = (CPSS_L4_PROTOCOL_ENT)inArgs[2];
    entryIndex = (GT_U8)inArgs[3];
    l4PortType = (CPSS_L4_PROTOCOL_PORT_TYPE_ENT)inArgs[4];
    compareOperator = (CPSS_COMPARE_OPERATOR_ENT)inArgs[5];
    value = (GT_U16)inArgs[6];

    /* call cpss api function */
    result = pg_wrap_cpssDxChPclTcpUdpPortComparatorSet(devNum, direction, l4Protocol,
                                                        entryIndex, l4PortType,
                                                        compareOperator, value);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxCh2PclTcpUdpPortComparatorGet function
* @endinternal
*
* @brief   Get TCP or UDP Port Comparator entry
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PTR               - on NULL pointer
*/
static CMD_STATUS wrCpssDxCh2PclTcpUdpPortComparatorGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8                              devNum;
    CPSS_PCL_DIRECTION_ENT             direction;
    CPSS_L4_PROTOCOL_ENT               l4Protocol;
    GT_U8                              entryIndex;
    CPSS_L4_PROTOCOL_PORT_TYPE_ENT     l4PortType;
    CPSS_COMPARE_OPERATOR_ENT          compareOperator;
    GT_U16                             value;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PCL_DIRECTION_ENT)inArgs[1];
    l4Protocol = (CPSS_L4_PROTOCOL_ENT)inArgs[2];
    entryIndex = (GT_U8)inArgs[3];

    /* call cpss api function */
    result = pg_wrap_cpssDxChPclTcpUdpPortComparatorGet(devNum, direction, l4Protocol,
                                                        entryIndex, &l4PortType,
                                                        &compareOperator, &value);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", l4PortType, compareOperator, value);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPclRuleSet function
* @endinternal
*
* @brief   The function sets the Policy Rule Mask, Pattern and Action
*         Switches rule-format specific functions according to
*         the field amonunt
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*
* @note NONE
*
*/
static CMD_STATUS wrCpssDxChPclRuleSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    cpssOsMemSet(&maskData , 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
    cpssOsMemSet(&patternData, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));

    /* tho old version of this function switched specific functions */
    /* by amount of fields, the table resides only in this comment  */
    /* don't remove it                                              */
    /* 70 - wrCpssDxChPclRuleSet_STD_NOT_IP                         */
    /* 84 - wrCpssDxChPclRuleSet_STD_IP_L2_QOS                      */
    /* 88 - wrCpssDxChPclRuleSet_STD_IPV4_L4                        */
    /* 80 - wrCpssDxChPclRuleSet_STD_IPV6_DIP                       */
    /* 92 - wrCpssDxChPclRuleSet_EXT_NOT_IPV6                       */
    /* 90 - wrCpssDxChPclRuleSet_EXT_IPV6_L2                        */
    /* 86 - wrCpssDxChPclRuleSet_EXT_IPV6_L4                        */

    /* switch by rule format in inFields[1]               */
    /* egress formats not supported, ingress used instead */
    switch (inFields[1])
    {
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E:
            return wrCpssDxChPclRuleSet_STD_NOT_IP(
                inArgs, inFields ,numFields, GT_FALSE ,outArgs);
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E:
            return wrCpssDxChPclRuleSet_STD_IP_L2_QOS(
                inArgs, inFields ,numFields, GT_FALSE ,outArgs);
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E:
            return wrCpssDxChPclRuleSet_STD_IPV4_L4(
                inArgs, inFields ,numFields, GT_FALSE ,outArgs);
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E:
            return wrCpssDxChPclRuleSet_STD_IPV6_DIP(
                inArgs, inFields ,numFields, GT_FALSE ,outArgs);
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E:
            return wrCpssDxChPclRuleSet_EXT_NOT_IPV6(
                inArgs, inFields ,numFields, GT_FALSE ,outArgs);
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E:
            return wrCpssDxChPclRuleSet_EXT_IPV6_L2(
                inArgs, inFields ,numFields, GT_FALSE ,outArgs);
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E:
            return wrCpssDxChPclRuleSet_EXT_IPV6_L4(
                inArgs, inFields ,numFields, GT_FALSE ,outArgs);
        default: return CMD_AGENT_ERROR;
    }
}

static CMD_STATUS wrCpssDxChPclRuleSet_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    cpssOsMemSet(&maskData , 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
    cpssOsMemSet(&patternData, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));

    /* tho old version of this function switched specific functions */
    /* by amount of fields, the table resides only in this comment  */
    /* don't remove it                                              */
    /* 70 - wrCpssDxChPclRuleSet_STD_NOT_IP                         */
    /* 84 - wrCpssDxChPclRuleSet_STD_IP_L2_QOS                      */
    /* 88 - wrCpssDxChPclRuleSet_STD_IPV4_L4                        */
    /* 80 - wrCpssDxChPclRuleSet_STD_IPV6_DIP                       */
    /* 92 - wrCpssDxChPclRuleSet_EXT_NOT_IPV6                       */
    /* 90 - wrCpssDxChPclRuleSet_EXT_IPV6_L2                        */
    /* 86 - wrCpssDxChPclRuleSet_EXT_IPV6_L4                        */

    /* switch by rule format in inFields[1]               */
    /* egress formats not supported, ingress used instead */
    switch (inFields[1])
    {
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E:
            return wrCpssDxChPclRuleSet_STD_NOT_IP(
                inArgs, inFields ,numFields, GT_TRUE ,outArgs);
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E:
            return wrCpssDxChPclRuleSet_STD_IP_L2_QOS(
                inArgs, inFields ,numFields, GT_TRUE ,outArgs);
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E:
            return wrCpssDxChPclRuleSet_STD_IPV4_L4(
                inArgs, inFields ,numFields, GT_TRUE ,outArgs);
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E:
            return wrCpssDxChPclRuleSet_STD_IPV6_DIP(
                inArgs, inFields ,numFields, GT_TRUE ,outArgs);
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E:
            return wrCpssDxChPclRuleSet_EXT_NOT_IPV6(
                inArgs, inFields ,numFields, GT_TRUE ,outArgs);
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E:
            return wrCpssDxChPclRuleSet_EXT_IPV6_L2(
                inArgs, inFields ,numFields, GT_TRUE ,outArgs);
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E:
            return wrCpssDxChPclRuleSet_EXT_IPV6_L4(
                inArgs, inFields ,numFields, GT_TRUE ,outArgs);
        default: return CMD_AGENT_ERROR;
    }
}

/**
* @internal wrCpssDxChPclRuleValidStatusSet function
* @endinternal
*
* @brief   Validates/Invalidates the Policy rule.
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
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - if in TCAM found rule of size different
*                                       from the specified
*/
static CMD_STATUS wrCpssDxChPclRuleValidStatusSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_PCL_RULE_SIZE_ENT ruleSize;
    GT_U32 ruleIndex;
    GT_BOOL valid;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    ruleSize = (CPSS_PCL_RULE_SIZE_ENT)inArgs[1];
    ruleIndex = (GT_U32)inArgs[2];
    valid = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = pg_wrap_cpssDxChPclRuleValidStatusSet(devNum, ruleSize, ruleIndex, valid);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChPclEgressForRemoteTunnelStartEnableSet function
* @endinternal
*
* @brief   Enable/Disable the Egress PCL processing for the packets,
*         whith Tunnel Start on another device.
*         These packets are ingessed whith DSA Tag contains source port 60.
*
* @note   APPLICABLE DEVICES:      DxCh2 devices only
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChPclEgressForRemoteTunnelStartEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPclEgressForRemoteTunnelStartEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChPclEgressForRemoteTunnelStartEnableGet function
* @endinternal
*
* @brief   Gets Enable/Disable of the Egress PCL processing for the packets,
*         which Tunnel Start on another device.
*         These packets are ingessed with DSA Tag contains source port 60.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChPclEgressForRemoteTunnelStartEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPclEgressForRemoteTunnelStartEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet function
* @endinternal
*
* @brief   Sets Egress Policy Configuration Table Access Mode for
*         Tunnel Start packets
*
* @note   APPLICABLE DEVICES:      DxCh3 and above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT cfgTabAccMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum        = (GT_U8)inArgs[0];
    cfgTabAccMode = (CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT)inArgs[1];

    /* call cpss api function */
        result = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet(
            devNum, cfgTabAccMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeGet function
* @endinternal
*
* @brief   Gets Egress Policy Configuration Table Access Mode for
*         Tunnel Start packets
*
* @note   APPLICABLE DEVICES:      DxCh3 and above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT cfgTabAccMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum        = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeGet(
        devNum, &cfgTabAccMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cfgTabAccMode);

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PclTunnelTermForceVlanModeEnableSet function
* @endinternal
*
* @brief   The function enables/disables forcing of the PCL ID configuration
*         for all TT packets according to the VLAN assignment.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxCh3PclTunnelTermForceVlanModeEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxCh3PclTunnelTermForceVlanModeEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PclTunnelTermForceVlanModeEnableGet function
* @endinternal
*
* @brief   The function gets enable/disable of the forcing of the PCL ID configuration
*         for all TT packets according to the VLAN assignment.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxCh3PclTunnelTermForceVlanModeEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable=GT_FALSE;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxCh3PclTunnelTermForceVlanModeEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/*************table cpssDxCh3PclRuleAction********************/

/* global variables */

/*actionTable-wrapper level table*/

#define ACTION_TAB_SIZE 64

static struct
{
    CPSS_DXCH_PCL_ACTION_STC  actionEntry;
    GT_U32                      ruleIndex;
    GT_BOOL                     valid;
    GT_U32                      nextFree;
}actionTable[ACTION_TAB_SIZE];

static GT_U32   firstFree = 0;
static GT_BOOL  firstRun = GT_TRUE;
#define INVALID_RULE_INDEX 0x7FFFFFF0
/**
* @internal wrCpssDxCh3PclActionClear function
* @endinternal
*
* @brief   Clears the wrapper level action table
*         The function initializes the actionTable entries and sets all entries invalid.
*         Done in wrapper level.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
*
* @note The use of this function is for first initialization and for clearing all
*       entries in the table.
*
*/
static CMD_STATUS wrCpssDxCh3PclActionClear
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32          index = 0;
    firstFree = 0;
    while (index < ACTION_TAB_SIZE)
    {
        actionTable[index].valid = GT_FALSE;
        actionTable[index].nextFree = index + 1;
        index++;
    }
    /* override*/
    actionTable[ACTION_TAB_SIZE - 1].nextFree = INVALID_RULE_INDEX;

    return CMD_OK;
}

/**
* @internal pclDxCh3ActionGet function
* @endinternal
*
* @brief   The function searches for action entry in internal wrapper table by rule index
*         and returns the actionEntry.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
* @param[in] ruleIndex                - index of the rule
*
* @param[out] tableIndex               - index of entry in actionTable(wrappper level table) that
*                                      contains ruleIndex
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
*/
void pclDxCh3ActionGet
(
    IN  GT_U32                  ruleIndex,
    OUT GT_U32                  *tableIndex
)
{
    GT_U32              index;

    for (index = 0; (index < ACTION_TAB_SIZE); index++)
    {
        if (actionTable[index].ruleIndex != ruleIndex)
            continue;

        *tableIndex = index;
        return;
    }

    *tableIndex = INVALID_RULE_INDEX;
 }

/*******************************************************************************
* cpssDxCh3PclActionDelete (table command)
*
* DESCRIPTION:
*  Delete from wrapper level table.
*  The function finds action table entry by rule index ,
*  deletes the actionEntry and updates the list.
*
* APPLICABLE DEVICES:  All DxCh devices
*
* INPUTS:
*       ruleIndex       - index of the rule
*
* OUTPUTS:
*       none
*
* RETURNS:
*       GT_OK          - on success
*       GT_BAD_PARAM   - on wrong parameters
*       GT_HW_ERROR    - on hardware error
*       GT_BAD_PTR     - on null pointer
*
* COMMENTS:
*
*******************************************************************************/
static CMD_STATUS wrCpssDxCh3PclActionDelete
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32                              ruleIndex;
    GT_U32                              index;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    ruleIndex = (GT_U32)inArgs[1];

    pclDxCh3ActionGet( ruleIndex, &index);

    /* the rule is not found */
    if (index == INVALID_RULE_INDEX)
    {
        galtisOutput(outArgs, GT_NO_SUCH, "%d", -1);
        return CMD_AGENT_ERROR;
    }

    actionTable[index].valid = GT_FALSE;
    actionTable[index].nextFree = firstFree;
    firstFree = index;

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PclActionSet function
* @endinternal
*
* @brief   The function sets action entry at the action table for the use of cpssDxCh3PclIngressRule/
*         cpssDxCh3PclEgressRule table.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
*
* @note if inFields[0]==1 then action enry is also written to hardware if not it written
*       only to wrapper level table
*
*/
static CMD_STATUS wrCpssDxCh3PclActionSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_PCL_ACTION_STC           *actionPtr;
    GT_U32                              ruleIndex;
    GT_U8                               devNum;
    GT_U32                              index;
    CPSS_PCL_RULE_SIZE_ENT              ruleSize;
    GT_STATUS                           result;
    GT_BOOL                             update;
    GT_U32                              i;
    GT_BOOL                             isXCatActionTable;
    GT_BOOL                             isLionActionTable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* CH3 Galtis table has 42 fields
       xCat Galtis table has 51 fields
       Lion Galtis table has 50 fields */
    isXCatActionTable = (numFields < 51) ? GT_FALSE : GT_TRUE;
    isLionActionTable = (numFields == 50) ? GT_TRUE : GT_FALSE;
    if (isLionActionTable != GT_FALSE)
    {
        /* lion table looks like xcat one */
        isXCatActionTable = GT_TRUE;
    }

    devNum  =       (GT_U8)inArgs[0];
    ruleIndex =     (GT_U32)inArgs[1];

    if (firstRun)
    {
        wrCpssDxCh3PclActionClear(inArgs,inFields,numFields,outArgs);
        firstRun = GT_FALSE;
    }
    if (firstFree == INVALID_RULE_INDEX)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_NO_RESOURCE, "%d", -1);
        return CMD_AGENT_ERROR;
    }
    pclDxCh3ActionGet( ruleIndex, &index);

    /* the rule wasn't previously in action table */
    if (index == INVALID_RULE_INDEX)
    {
        if (firstFree == INVALID_RULE_INDEX)
        {
            /* pack output arguments to galtis string */
            galtisOutput(outArgs, GT_NO_RESOURCE, "%d", -1);
            return CMD_AGENT_ERROR;
        }

        index = firstFree;
    }

    cpssOsMemSet(
        &(actionTable[index].actionEntry), 0,
        sizeof(actionTable[index].actionEntry));

    actionPtr = &(actionTable[index].actionEntry);

    /* field index */
    i = 0;

    update = (GT_BOOL)inFields[i++];

    actionPtr->pktCmd        = (CPSS_PACKET_CMD_ENT)inFields[i++];
    actionPtr->actionStop    =(GT_BOOL)inFields[i++];
    actionPtr->egressPolicy  = (GT_BOOL)inFields[i++];
    actionPtr->mirror.cpuCode=(CPSS_NET_RX_CPU_CODE_ENT)inFields[i++];
    actionPtr->mirror.mirrorToRxAnalyzerPort=(GT_BOOL)inFields[i++];
    actionPtr->matchCounter.enableMatchCount=(GT_BOOL)inFields[i++];
    actionPtr->matchCounter.matchCounterIndex=(GT_U32)inFields[i++];

    switch (actionPtr->egressPolicy)
    {
        case GT_FALSE: /* ingress */
    /* WA - GUI defines the field modifyDscp as BOOL instead of Enum */
            switch (inFields[i])
    {
        case (GT_32)GT_FALSE:
                            actionPtr->qos.ingress.modifyDscp =
                CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
            break;
        case (GT_32)GT_TRUE:
                            actionPtr->qos.ingress.modifyDscp =
                CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
            break;
        default:
            /* the case reserved for Galtis GUI that must     */
            /* be fixed by appending "DISABLE_MODIFY" to list */
            /* {"GT_FALSE", "GT_TRUE", "DISABLE_MODIFY"}      */
                            actionPtr->qos.ingress.modifyDscp =
                CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
            break;
    }

            actionPtr->qos.ingress.modifyUp =
                (CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[i+1];

            actionPtr->qos.ingress.profileIndex = (GT_U32)inFields[i+2];
            actionPtr->qos.ingress.profileAssignIndex = (GT_BOOL)inFields[i+3];
            actionPtr->qos.ingress.profilePrecedence =
                (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[i+4];
            break;

        default:
        case GT_TRUE: /* egress */
            /* GUI defines the field modifyDscp as BOOL instead of Enum */
            actionPtr->qos.egress.modifyDscp = (GT_BOOL)(inFields[i]);
            actionPtr->qos.egress.modifyUp =
                (inFields[i+1] == CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E)
                    ? CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E
                    : CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
            actionPtr->qos.egress.dscp = (GT_U32)inFields[i+5];
            actionPtr->qos.egress.up   = (GT_U32)inFields[i+6];
            break;

    }

    /* bypass the switch */
    i += 7;

    actionPtr->redirect.redirectCmd =
        (CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ENT)inFields[i++];

    if (actionPtr->redirect.redirectCmd ==
        CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E)
    {
        if (isLionActionTable != GT_FALSE)
        {
            /* fields retrieved from the same GUI that for OutLif */
            actionPtr->redirect.data.noRedirect.arpPtr = (GT_U32)inFields[i+10];
            actionPtr->redirect.data.noRedirect.modifyMacDa = (GT_BOOL)inFields[i+11];
        }
    }

    if (actionPtr->redirect.redirectCmd ==
        CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E)
    {
        actionPtr->redirect.data.outIf.outInterface.type =
            (CPSS_INTERFACE_TYPE_ENT)inFields[i];

        switch (actionPtr->redirect.data.outIf.outInterface.type)
        {
        case CPSS_INTERFACE_PORT_E:
            actionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum =
                (GT_HW_DEV_NUM)inFields[i+1];
            actionPtr->redirect.data.outIf.outInterface.devPort.portNum =
                (GT_PORT_NUM)inFields[i+2];

            CONVERT_DEV_PORT_DATA_MAC(
                actionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum,
                actionPtr->redirect.data.outIf.outInterface.devPort.portNum);

            break;

        case CPSS_INTERFACE_TRUNK_E:
            actionPtr->redirect.data.outIf.outInterface.trunkId =
                (GT_TRUNK_ID)inFields[i+3];
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(actionPtr->redirect.data.outIf.outInterface.trunkId);
            break;

        case CPSS_INTERFACE_VIDX_E:
            actionPtr->redirect.data.outIf.outInterface.vidx =
                (GT_U16)inFields[i+4];
            break;

        case CPSS_INTERFACE_VID_E:
            actionPtr->redirect.data.outIf.outInterface.vlanId =
                (GT_U16)inFields[i+5];
            break;

        default:
            break;
        }
        actionPtr->redirect.data.outIf.vntL2Echo   = (GT_BOOL)inFields[i+6];
        actionPtr->redirect.data.outIf.tunnelStart = (GT_BOOL)inFields[i+7];
        actionPtr->redirect.data.outIf.tunnelPtr   = (GT_U32)inFields[i+8];
        actionPtr->redirect.data.outIf.tunnelType  =
            (CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ENT)inFields[i+9];
        if (isLionActionTable != GT_FALSE)
        {
            actionPtr->redirect.data.outIf.arpPtr = (GT_U32)inFields[i+10];
            actionPtr->redirect.data.outIf.modifyMacDa = (GT_BOOL)inFields[i+11];
            actionPtr->redirect.data.outIf.modifyMacSa = (GT_BOOL)inFields[i+12];
        }
    }

    /* bypass the "if" body */
    i += 10;
    if (isLionActionTable != GT_FALSE)
    {
        /* arpPtr, modifyMacDa, modifyMacSa */
        i += 3;
    }
    else
    {
        /* not implemented "redirectInterfaceType" - skip it */
        i++;
        if (actionPtr->redirect.data.outIf.tunnelStart == GT_TRUE)
        {
            actionPtr->redirect.data.outIf.modifyMacDa = GT_FALSE;
            actionPtr->redirect.data.outIf.modifyMacSa = GT_FALSE;
        }
    }

    if (actionPtr->redirect.redirectCmd ==
        CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E)
    {
        actionPtr->redirect.data.routerLttIndex = (GT_U32)inFields[i++];
    }
    else
    {
        /* bypass the "if" body */
        i ++;
    }


    if (actionPtr->redirect.redirectCmd ==
        CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E)
    {
        actionPtr->redirect.data.vrfId = (GT_U32)inFields[i++];
    }
    else
    {
        /* bypass the "if" body */
        i ++;
    }

    actionPtr->policer.policerEnable = (CPSS_DXCH_PCL_POLICER_ENABLE_ENT)inFields[i++];
    actionPtr->policer.policerId = (GT_U32)inFields[i++];

    switch (actionPtr->egressPolicy)
    {
        case GT_FALSE:
            actionPtr->vlan.ingress.modifyVlan =
                (CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT)inFields[i];
            actionPtr->vlan.ingress.nestedVlan = (GT_BOOL)inFields[i+1];
            actionPtr->vlan.ingress.vlanId = (GT_U16)inFields[i+2];
            actionPtr->vlan.ingress.precedence =
                (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[i+3];
            break;

        case GT_TRUE:
            actionPtr->vlan.egress.vlanCmd =
                ((inFields[i] == CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E)
                 || (inFields[i] == CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_UNTAGGED_E))
                ? CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E
                : CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
            actionPtr->vlan.egress.vlanId = (GT_U16)inFields[i+2];
            break;

        default:
            break;
    }

    i += 4;

    actionPtr->ipUcRoute.doIpUcRoute = (GT_BOOL)inFields[i++];
    actionPtr->ipUcRoute.arpDaIndex = (GT_U32)inFields[i++];
    actionPtr->ipUcRoute.decrementTTL = (GT_BOOL)inFields[i++];
    actionPtr->ipUcRoute.bypassTTLCheck = (GT_BOOL)inFields[i++];
    actionPtr->ipUcRoute.icmpRedirectCheck = (GT_BOOL)inFields[i++];

    actionPtr->sourceId.assignSourceId = (GT_BOOL)inFields[i++];
    actionPtr->sourceId.sourceIdValue  = (GT_U32)inFields[i++];

    if (isXCatActionTable != GT_FALSE)
    {
        actionPtr->bypassBridge                           = (GT_BOOL)inFields[i++];
        actionPtr->bypassIngressPipe                      = (GT_BOOL)inFields[i++];
        actionPtr->lookupConfig.ipclConfigIndex           = (GT_U32)inFields[i++];
        actionPtr->lookupConfig.pcl0_1OverrideConfigIndex = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[i++];
        actionPtr->lookupConfig.pcl1OverrideConfigIndex   = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[i++];

        /* skip reserved fields in xCat table */
        if (isLionActionTable == GT_FALSE)
        {
            i++; /* reserved_2 [D] */
            i++; /* reserved_3 [D] */
        }

        actionPtr->mirror.mirrorTcpRstAndFinPacketsToCpu  = (GT_BOOL)inFields[i++];
        i++; /*redundant  policer.policerEnable */
    }
    else
    {
        /* The bypass Bridge or Bypass Ingress pipe should be set for redirect to Outlif action
           for SIP5 devices. For other devices CPSS configures flags by itself. */
        if ((actionPtr->redirect.redirectCmd == CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E) &&
            PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
        {
            actionPtr->bypassBridge = GT_TRUE;
        }
    }

    if (update != GT_FALSE) /*Write entry to hardware or not*/
    {
        ruleSize=(CPSS_PCL_RULE_SIZE_ENT)inArgs[2];
        /*call api function*/
        result = pg_wrap_cpssDxChPclRuleActionUpdate(devNum, ruleSize, ruleIndex, actionPtr);
    }
    else
    {
        actionTable[index].valid = GT_TRUE;
        actionTable[index].ruleIndex = ruleIndex;
        firstFree = actionTable[firstFree].nextFree;
        result = GT_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxCh3PclActionGet function
* @endinternal
*
* @brief   The function gets action entry from the action table in hardware
*         (not in wrapper level table)
*
* @note   APPLICABLE DEVICES:      All ExMxPm devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
*/
static CMD_STATUS wrCpssDxCh3PclActionGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PCL_RULE_SIZE_ENT      ruleSize;
    GT_U32                      ruleIndex;
    CPSS_PCL_DIRECTION_ENT      direction;
    CPSS_DXCH_PCL_ACTION_STC    action;
    GT_U32                      i;
    GT_BOOL                     isXCatActionTable;
    GT_BOOL                     isLionActionTable;
    GT_U32                      actualNumFields;
    GT_HW_DEV_NUM               __HwDev;  /* Dummy for converting. */
    GT_PORT_NUM                 __Port; /* Dummy for converting. */

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* CH3 Galtis table has 42 fields
       xCat Galtis table has 51 fields
       Lion Galtis table has 50 fields */
    isXCatActionTable = (numFields < 51) ? GT_FALSE : GT_TRUE;
    isLionActionTable = (numFields == 50) ? GT_TRUE : GT_FALSE;
    if (isLionActionTable != GT_FALSE)
    {
        /* lion table looks like xcat one */
        isXCatActionTable = GT_TRUE;
    }

    /* to support the not seted/geted members of structures */
    cpssOsMemSet(&action, 0, sizeof(action));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ruleIndex = (CPSS_PCL_DIRECTION_ENT)inArgs[1];
    ruleSize = (CPSS_PCL_RULE_SIZE_ENT)inArgs[2];
    direction = (GT_U32)inArgs[3];


    result = pg_wrap_cpssDxChPclRuleActionGet(devNum, ruleSize, ruleIndex,
                                            direction, &action);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    i = 0;

    inFields[i++]=GT_TRUE;
    inFields[i++]=action.pktCmd;
    inFields[i++]=action.actionStop ;
    inFields[i++]=action.egressPolicy ;
    inFields[i++]=action.mirror.cpuCode;
    inFields[i++]=action.mirror.mirrorToRxAnalyzerPort;
    inFields[i++]=action.matchCounter.enableMatchCount;
    inFields[i++]=action.matchCounter.matchCounterIndex;
    if (action.egressPolicy == GT_FALSE)
    {
        /* workaround caused by GUI: Values FALSE, TRUE, DISABLE */
        switch (action.qos.ingress.modifyDscp)
        {
            case CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E:
                inFields[i] = 0;
                break;
            case CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
                inFields[i] = 1;
                break;
            default:
            case CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:
                inFields[i] = 2;
                break;
        }
        inFields[i+1] = action.qos.ingress.modifyUp;
        inFields[i+2] = action.qos.ingress.profileIndex;
        inFields[i+3] = action.qos.ingress.profileAssignIndex;
        inFields[i+4] = action.qos.ingress.profilePrecedence;
        inFields[i+5] = 0;
        inFields[i+6] = 0;
    }
    else
    {
        inFields[i] = action.qos.egress.modifyDscp;
        inFields[i+1] =
            (action.qos.egress.modifyUp ==
             CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E)
                ? CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E
                : CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
        inFields[i+2] = 0;
        inFields[i+3] = 0;
        inFields[i+4] = 0;
        inFields[i+5] = action.qos.egress.dscp;
        inFields[i+6] = action.qos.egress.up;
    }
    i += 7;
    inFields[i++]=action.redirect.redirectCmd;
    inFields[i++]=action.redirect.data.outIf.outInterface.type;
    __HwDev  = action.redirect.data.outIf.outInterface.devPort.hwDevNum;
    __Port = action.redirect.data.outIf.outInterface.devPort.portNum;

    CONVERT_BACK_DEV_PORT_DATA_MAC(__HwDev, __Port) ;
    inFields[i++] = __HwDev  ;
    inFields[i++] = __Port ;
    CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(action.redirect.data.outIf.outInterface.trunkId);
    inFields[i++]=action.redirect.data.outIf.outInterface.trunkId;
    inFields[i++]=action.redirect.data.outIf.outInterface.vidx;
    inFields[i++]=action.redirect.data.outIf.outInterface.vlanId;
    inFields[i++]= action.redirect.data.outIf.vntL2Echo;
    inFields[i++]= action.redirect.data.outIf.tunnelStart;
    inFields[i++]= action.redirect.data.outIf.tunnelPtr;
    inFields[i++]= action.redirect.data.outIf.tunnelType;
    if (isLionActionTable != GT_FALSE)
    {
        /* used the same GUI fields */
        if (action.redirect.redirectCmd ==
            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E)
        {
            inFields[i++] = action.redirect.data.noRedirect.arpPtr;
            inFields[i++] = action.redirect.data.noRedirect.modifyMacDa;
            inFields[i++] = 0;
        }
        else
        {
            inFields[i++] = action.redirect.data.outIf.arpPtr;
            inFields[i++] = action.redirect.data.outIf.modifyMacDa;
            inFields[i++] = action.redirect.data.outIf.modifyMacSa;
        }
    }
    else
    {
        inFields[i++]= 0 /* not implemented "redirectInterfaceType" */;
    }
    inFields[i++]=action.redirect.data.routerLttIndex;
    inFields[i++]=action.redirect.data.vrfId ;
    inFields[i++]=action.policer.policerEnable;
    inFields[i++]=action.policer.policerId;

    if (action.egressPolicy == GT_FALSE)
    {
        inFields[i]   = action.vlan.ingress.modifyVlan;
        inFields[i+1] = action.vlan.ingress.nestedVlan;
        inFields[i+2] = action.vlan.ingress.vlanId;
        inFields[i+3] = action.vlan.ingress.precedence;
    }
    else
    {
        inFields[i] =
            (action.vlan.egress.vlanCmd ==
             CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E)
                ? CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E
                : CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E;
        inFields[i+1] = 0;
        inFields[i+2] = action.vlan.egress.vlanId;
        inFields[i+3] = 0;
    }

    i += 4;

    inFields[i++]=action.ipUcRoute.doIpUcRoute;
    inFields[i++]=action.ipUcRoute.arpDaIndex ;
    inFields[i++]=action.ipUcRoute.decrementTTL;
    inFields[i++]=action.ipUcRoute.bypassTTLCheck;
    inFields[i++]=action.ipUcRoute.icmpRedirectCheck;
    inFields[i++]=action.sourceId.assignSourceId;
    inFields[i++]=action.sourceId.sourceIdValue ;

    if (isXCatActionTable != GT_FALSE)
    {
        inFields[i++] = action.bypassBridge;
        inFields[i++] = action.bypassIngressPipe;
        inFields[i++] = action.lookupConfig.ipclConfigIndex;
        inFields[i++] = action.lookupConfig.pcl0_1OverrideConfigIndex;
        inFields[i++] = action.lookupConfig.pcl1OverrideConfigIndex;
        if (isLionActionTable == GT_FALSE)
        {
            inFields[i++] = 0; /*redundant mirror.CpuCode */
            inFields[i++] = 0; /*redundant  mirror.mirrorToRxAnalyzerPort */
        }
        inFields[i++] = action.mirror.mirrorTcpRstAndFinPacketsToCpu;
        if (isLionActionTable == GT_FALSE)
        {
            inFields[i++] = 0; /*redundant  policer.policerEnable */
        }
    }

    actualNumFields = i;

    /* pack and output table fields */
    for (i = 0; (i < actualNumFields); i++)
    {
        fieldOutput("%d", inFields[i]);
        fieldOutputSetAppendMode();
    }

    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

static CMD_STATUS wrCpssDxCh3PclActionGet_Ch3
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxCh3PclActionGet(
        inArgs,inFields, 42 /*numFields*/, outArgs);
}

static CMD_STATUS wrCpssDxCh3PclActionGet_XCat
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxCh3PclActionGet(
        inArgs,inFields, 51 /*numFields*/, outArgs);
}

static CMD_STATUS wrCpssDxCh3PclActionGet_Lion
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxCh3PclActionGet(
        inArgs,inFields, 50 /*numFields*/, outArgs);
}

/* Begin Lion3 Action Table */

/**
* @internal wrCpssDxChPclLion3ActionSet function
* @endinternal
*
* @brief   The function sets action entry at the action table for the use of Rule tables.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
*
* @note if inFields[0]==1 then action enry is also written to hardware if not it written
*       only to wrapper level table
*
*/
static CMD_STATUS wrCpssDxChPclLion3ActionSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_PCL_ACTION_STC           *actionPtr;
    GT_U32                              ruleIndex;
    GT_U8                               devNum;
    GT_U32                              index;
    CPSS_PCL_RULE_SIZE_ENT              ruleSize;
    GT_STATUS                           result;
    GT_BOOL                             update;
    GT_U32                              i;
    GT_U32                              saveArpPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum  =       (GT_U8)inArgs[0];
    ruleIndex =     (GT_U32)inArgs[1];

    if (firstRun)
    {
        wrCpssDxCh3PclActionClear(inArgs,inFields,numFields,outArgs);
        firstRun = GT_FALSE;
    }

    pclDxCh3ActionGet( ruleIndex, &index);

    /* the rule wasn't previously in action table */
    if (index == INVALID_RULE_INDEX)
    {
        if (firstFree == INVALID_RULE_INDEX)
        {
            /* pack output arguments to galtis string */
            galtisOutput(outArgs, GT_NO_RESOURCE, "%d", -1);
            return CMD_AGENT_ERROR;
        }

        index = firstFree;
    }

    cpssOsMemSet(
        &(actionTable[index].actionEntry), 0,
        sizeof(actionTable[index].actionEntry));

    actionPtr = &(actionTable[index].actionEntry);

    /* field index */
    i = 0;

    update = (GT_BOOL)inFields[i++];

    actionPtr->pktCmd        = (CPSS_PACKET_CMD_ENT)inFields[i++];
    actionPtr->actionStop    =(GT_BOOL)inFields[i++];
    actionPtr->egressPolicy  = (GT_BOOL)inFields[i++];
    actionPtr->mirror.cpuCode=(CPSS_NET_RX_CPU_CODE_ENT)inFields[i++];
    actionPtr->mirror.mirrorToRxAnalyzerPort=(GT_BOOL)inFields[i++];
    actionPtr->mirror.ingressMirrorToAnalyzerIndex=(GT_U32)inFields[i++];
    actionPtr->matchCounter.enableMatchCount=(GT_BOOL)inFields[i++];
    actionPtr->matchCounter.matchCounterIndex=(GT_U32)inFields[i++];

    switch (actionPtr->egressPolicy)
    {
        case GT_FALSE: /* ingress */
            actionPtr->qos.ingress.modifyDscp =
                (CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[i];

            actionPtr->qos.ingress.modifyUp =
                (CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[i+1];

            actionPtr->qos.ingress.profileIndex = (GT_U32)inFields[i+2];
            actionPtr->qos.ingress.profileAssignIndex = (GT_BOOL)inFields[i+3];
            actionPtr->qos.ingress.profilePrecedence =
                (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[i+4];
            actionPtr->qos.ingress.up1Cmd =
                (CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ENT)inFields[i+5];
            actionPtr->qos.ingress.up1 = (GT_U32)inFields[i+6];
            break;

        default:
        case GT_TRUE: /* egress */
            actionPtr->qos.egress.modifyDscp =
                (CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_ENT)(inFields[i+7]);
            actionPtr->qos.egress.dscp = (GT_U32)inFields[i+8];
            actionPtr->qos.egress.modifyUp =
                (CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT)(inFields[i+9]);
            actionPtr->qos.egress.up   = (GT_U32)inFields[i+10];
            actionPtr->qos.egress.up1ModifyEnable   = (GT_BOOL)inFields[i+11];
            actionPtr->qos.egress.up1   = (GT_U32)inFields[i+12];
            break;
    }

    /* bypass the switch */
    i += 13;

    actionPtr->redirect.redirectCmd =
        (CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ENT)inFields[i++];

    if (actionPtr->redirect.redirectCmd ==
        CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E)
    {
        /* used the same GUI fields as for redirect to outIf */
        actionPtr->redirect.data.noRedirect.arpPtr = (GT_U32)inFields[i+9];
        actionPtr->redirect.data.noRedirect.modifyMacDa = (GT_BOOL)inFields[i+10];
    }

    if (actionPtr->redirect.redirectCmd ==
        CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E)
    {
        actionPtr->redirect.data.outIf.outInterface.type =
            (CPSS_INTERFACE_TYPE_ENT)inFields[i];

        switch (actionPtr->redirect.data.outIf.outInterface.type)
        {
            case CPSS_INTERFACE_PORT_E:
                actionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum =
                    (GT_HW_DEV_NUM)inFields[i+1];
                actionPtr->redirect.data.outIf.outInterface.devPort.portNum =
                    (GT_PORT_NUM)inFields[i+2];
                CONVERT_DEV_PORT_DATA_MAC(
                    actionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum,
                    actionPtr->redirect.data.outIf.outInterface.devPort.portNum);
                break;

            case CPSS_INTERFACE_TRUNK_E:
                actionPtr->redirect.data.outIf.outInterface.trunkId =
                    (GT_TRUNK_ID)inFields[i+3];
                CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(
                   actionPtr->redirect.data.outIf.outInterface.trunkId);
                break;

            case CPSS_INTERFACE_VIDX_E:
                actionPtr->redirect.data.outIf.outInterface.vidx =
                    (GT_U16)inFields[i+4];
                break;

            default:
                break;
        }
        actionPtr->redirect.data.outIf.vntL2Echo   = (GT_BOOL)inFields[i+5];
        actionPtr->redirect.data.outIf.tunnelStart = (GT_BOOL)inFields[i+6];
        actionPtr->redirect.data.outIf.tunnelPtr   = (GT_U32)inFields[i+7];
        actionPtr->redirect.data.outIf.tunnelType  =
            (CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ENT)inFields[i+8];
        actionPtr->redirect.data.outIf.arpPtr = (GT_U32)inFields[i+9];
        actionPtr->redirect.data.outIf.modifyMacDa = (GT_BOOL)inFields[i+10];
        actionPtr->redirect.data.outIf.modifyMacSa = (GT_BOOL)inFields[i+11];
    }
    /* bypass redirect to IF */
    /* arvPtr relevant also to ...REDIRECT_CMD_REPLACE_MAC_SA_E */
    saveArpPtr = (GT_U32)inFields[i+9];
    i += 12;

    if (actionPtr->redirect.redirectCmd ==
        CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E)
    {
        actionPtr->redirect.data.routerLttIndex = (GT_U32)inFields[i];
    }
    /* bypass redirect to Next Hop */
    i ++;


    if (actionPtr->redirect.redirectCmd ==
        CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E)
    {
        actionPtr->redirect.data.vrfId = (GT_U32)inFields[i];
    }
    /* bypass redirect to Virtual Router */
    i ++;

    if (actionPtr->redirect.redirectCmd ==
        CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E)
    {
        galtisMacAddr(&(actionPtr->redirect.data.modifyMacSa.macSa), (GT_U8*)inFields[i]);
        /* arpPtr saved from ...REDIRECT_CMD_OUT_IF_E parameters */
        actionPtr->redirect.data.modifyMacSa.arpPtr = saveArpPtr;
    }
    /* bypass replace MAC_SA */
    i ++;

    actionPtr->policer.policerEnable = (CPSS_DXCH_PCL_POLICER_ENABLE_ENT)inFields[i++];
    actionPtr->policer.policerId = (GT_U32)inFields[i++];

    switch (actionPtr->egressPolicy)
    {
        case GT_FALSE:
            actionPtr->vlan.ingress.modifyVlan =
                (CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT)inFields[i];
            actionPtr->vlan.ingress.nestedVlan = (GT_BOOL)inFields[i+2];
            actionPtr->vlan.ingress.vlanId = (GT_U32)inFields[i+3];
            actionPtr->vlan.ingress.precedence =
                (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[i+4];
            actionPtr->vlan.ingress.vlanId1Cmd =
                (CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ENT)inFields[i+5];
            actionPtr->vlan.ingress.vlanId1 = (GT_U32)inFields[i+7];
            break;

        case GT_TRUE:
            actionPtr->vlan.egress.vlanCmd =
                (CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT)inFields[i+1];
            actionPtr->vlan.egress.vlanId = (GT_U32)inFields[i+3];
            actionPtr->vlan.egress.vlanId1ModifyEnable = (GT_BOOL)inFields[i+6];
            actionPtr->vlan.egress.vlanId1 = (GT_U32)inFields[i+7];
            break;

        default:
            break;
    }
    /* bypass VLAN */
    i += 8;

    actionPtr->sourceId.assignSourceId = (GT_BOOL)inFields[i++];
    actionPtr->sourceId.sourceIdValue  = (GT_U32)inFields[i++];

    actionPtr->bypassBridge                           = (GT_BOOL)inFields[i++];
    actionPtr->bypassIngressPipe                      = (GT_BOOL)inFields[i++];

    actionPtr->lookupConfig.ipclConfigIndex           = (GT_U32)inFields[i++];
    actionPtr->lookupConfig.pcl0_1OverrideConfigIndex =
        (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[i++];
    actionPtr->lookupConfig.pcl1OverrideConfigIndex   =
        (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[i++];
    actionPtr->mirror.mirrorTcpRstAndFinPacketsToCpu  = (GT_BOOL)inFields[i++];

    actionPtr->oam.timeStampEnable  = (GT_BOOL)inFields[i++];
    actionPtr->oam.offsetIndex      = (GT_U32)inFields[i++];
    actionPtr->oam.oamProcessEnable = (GT_BOOL)inFields[i++];
    actionPtr->oam.oamProfile       = (GT_U32)inFields[i++];

    actionPtr->flowId       = (GT_U32)inFields[i++];
    actionPtr->setMacToMe   = (GT_BOOL)inFields[i++];

    actionPtr->sourcePort.assignSourcePortEnable   = (GT_BOOL)inFields[i++];
    actionPtr->sourcePort.sourcePortValue          = (GT_U32)inFields[i++];

    actionPtr->channelTypeToOpcodeMapEnable   = (GT_BOOL)inFields[i++];
    actionPtr->tmQueueId                      = (GT_U32)inFields[i++];
    actionPtr->exactMatchOverPclEn            = (GT_BOOL)inFields[i++];

    /* falcon fields */
    actionPtr->unknownSaCommandEnable           = (GT_BOOL)inFields[i++];
    actionPtr->unknownSaCommand                 = (CPSS_PACKET_CMD_ENT)inFields[i++];
    actionPtr->terminateCutThroughMode          = (GT_BOOL)inFields[i++];
    actionPtr->latencyMonitor.monitoringEnable  = (GT_BOOL)inFields[i++];
    actionPtr->latencyMonitor.latencyProfile    = (GT_U32)inFields[i++];
    actionPtr->skipFdbSaLookup                  = (GT_BOOL)inFields[i++];
    actionPtr->triggerInterrupt                 = (GT_BOOL)inFields[i++];
    actionPtr->mirror.mirrorToTxAnalyzerPortEn  = (GT_BOOL)inFields[i++];
    actionPtr->mirror.egressMirrorToAnalyzerIndex = (GT_U32)inFields[i++];
    actionPtr->mirror.egressMirrorToAnalyzerMode = (CPSS_DXCH_MIRROR_EGRESS_MODE_ENT)inFields[i++];

    /* PHA related fields */
    {
        actionPtr->epclPhaInfo.phaThreadIdAssignmentMode =  (CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENT)inFields[i++];
        actionPtr->epclPhaInfo.phaThreadId   =  inFields[i++];
        actionPtr->epclPhaInfo.phaThreadType =  (CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_ENT)inFields[i++];
        if(actionPtr->epclPhaInfo.phaThreadType == CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_IOAM_INGRESS_SWITCH_E)
        {
            actionPtr->epclPhaInfo.phaThreadUnion.epclIoamIngressSwitch.ioamIncludesBothE2EOptionAndTraceOption   =  inFields[i+0];
        }
        else
        {
            actionPtr->epclPhaInfo.phaThreadUnion.notNeeded = inFields[i+1];
        }
        i+=2;/* 2 fields comes from the Galtis (NOT implemented as tabulator for the 'union')*/
    }

    if (update != GT_FALSE) /*Write entry to hardware or not*/
    {
        ruleSize=(CPSS_PCL_RULE_SIZE_ENT)inArgs[2];
        /*call api function*/
        result = pg_wrap_cpssDxChPclRuleActionUpdate(devNum, ruleSize, ruleIndex, actionPtr);
    }
    else
    {
        actionTable[index].valid = GT_TRUE;
        actionTable[index].ruleIndex = ruleIndex;
        firstFree = actionTable[firstFree].nextFree;
        result = GT_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChPclLion3ActionGetFirst function
* @endinternal
*
* @brief   The function gets action entry from the action table in hardware
*         (not in wrapper level table)
*
* @note   APPLICABLE DEVICES:      All ExMxPm devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
*/
static CMD_STATUS wrCpssDxChPclLion3ActionGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PCL_RULE_SIZE_ENT      ruleSize;
    GT_U32                      ruleIndex;
    CPSS_PCL_DIRECTION_ENT      direction;
    CPSS_DXCH_PCL_ACTION_STC    action;
    char                        workStr[32];
    GT_U8                       *workPtr;
    GT_HW_DEV_NUM               __HwDev;  /* Dummy for converting. */
    GT_PORT_NUM                 __Port; /* Dummy for converting. */

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ruleIndex = (CPSS_PCL_DIRECTION_ENT)inArgs[1];
    ruleSize = (CPSS_PCL_RULE_SIZE_ENT)inArgs[2];
    direction = (GT_U32)inArgs[3];

    if(direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        /*****************************************/
        /* next relevant only to SIP6 and 'EPCL' */
        /*****************************************/
        /*
            IMPORTANT : this field is input !!! although we are in 'Get' function !!!
            the application gives us hint how to parse the info from the HW.
        */
        action.epclPhaInfo.phaThreadType = (CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_ENT)inArgs[4];
    }

    result = pg_wrap_cpssDxChPclRuleActionGet(
        devNum, ruleSize, ruleIndex, direction, &action);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    fieldOutput(
        "%d%d%d%d%d%d%d%d%d",
        GT_TRUE,
        action.pktCmd,
        action.actionStop,
        action.egressPolicy,
        action.mirror.cpuCode,
        action.mirror.mirrorToRxAnalyzerPort,
        action.mirror.ingressMirrorToAnalyzerIndex,
        action.matchCounter.enableMatchCount,
        action.matchCounter.matchCounterIndex);

    fieldOutputSetAppendMode();

    if (action.egressPolicy == GT_FALSE)
    {
        fieldOutput(
            "%d%d%d%d%d%d%d%d%d%d%d%d%d",
            action.qos.ingress.modifyDscp,
            action.qos.ingress.modifyUp,
            action.qos.ingress.profileIndex,
            action.qos.ingress.profileAssignIndex,
            action.qos.ingress.profilePrecedence,
            action.qos.ingress.up1Cmd,
            action.qos.ingress.up1,
            0 /*action.qos.egress.modifyDscp*/,
            0 /*action.qos.egress.dscp*/,
            0 /*action.qos.egress.modifyUp*/,
            0 /*action.qos.egress.up*/,
            0 /*action.qos.egress.up1ModifyEnable*/,
            0 /*action.qos.egress.up1*/);
    }
    else
    {
        fieldOutput(
            "%d%d%d%d%d%d%d%d%d%d%d%d%d",
            0 /*action.qos.ingress.modifyDscp*/,
            0 /*action.qos.ingress.modifyUp*/,
            0 /*action.qos.ingress.profileIndex*/,
            0 /*action.qos.ingress.profileAssignIndex*/,
            0 /*action.qos.ingress.profilePrecedence*/,
            0 /*action.qos.ingress.up1Cmd*/,
            0 /*action.qos.ingress.up1*/,
            action.qos.egress.modifyDscp,
            action.qos.egress.dscp,
            action.qos.egress.modifyUp,
            action.qos.egress.up,
            action.qos.egress.up1ModifyEnable,
            action.qos.egress.up1);
    }

    fieldOutputSetAppendMode();

    fieldOutput("%d", action.redirect.redirectCmd);
    fieldOutputSetAppendMode();

    if (action.redirect.redirectCmd ==
        CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E)
    {
        fieldOutput(
            "%d%d%d%d%d%d%d%d%d%d%d%d",
            0, 0, 0, 0, 0, 0, 0, 0, 0,
            action.redirect.data.noRedirect.arpPtr,
            action.redirect.data.noRedirect.modifyMacDa,
            0);
    }
    else if (action.redirect.redirectCmd ==
        CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E)
    {
        __HwDev  = action.redirect.data.outIf.outInterface.devPort.hwDevNum;
        __Port = action.redirect.data.outIf.outInterface.devPort.portNum;
        CONVERT_BACK_DEV_PORT_DATA_MAC(__HwDev, __Port) ;

        fieldOutput(
            "%d%d%d%d%d%d%d%d%d%d%d%d",
            action.redirect.data.outIf.outInterface.type,
            __HwDev, __Port,
            action.redirect.data.outIf.outInterface.trunkId,
            action.redirect.data.outIf.outInterface.vidx,
            action.redirect.data.outIf.vntL2Echo,
            action.redirect.data.outIf.tunnelStart,
            action.redirect.data.outIf.tunnelPtr,
            action.redirect.data.outIf.tunnelType,
            action.redirect.data.outIf.arpPtr,
            action.redirect.data.outIf.modifyMacDa,
            action.redirect.data.outIf.modifyMacSa);
    }
    else if (action.redirect.redirectCmd ==
        CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E)
    {
        fieldOutput(
            "%d%d%d%d%d%d%d%d%d%d%d%d",
            0,0,0,0,0,0,0,0,0,action.redirect.data.modifyMacSa.arpPtr,0,0);
    }
    else
    {
        fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d", 0,0,0,0,0,0,0,0,0,0,0,0);
    }
    fieldOutputSetAppendMode();

    if (action.redirect.redirectCmd ==
        CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E)
    {
        fieldOutput("%d", action.redirect.data.routerLttIndex);
    }
    else
    {
        fieldOutput("%d", 0);
    }
    fieldOutputSetAppendMode();

    if (action.redirect.redirectCmd ==
        CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E)
    {
        fieldOutput("%d", action.redirect.data.vrfId);
    }
    else
    {
        fieldOutput("%d", 0);
    }
    fieldOutputSetAppendMode();

    if (action.redirect.redirectCmd ==
        CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E)
    {
        workPtr = &(action.redirect.data.modifyMacSa.macSa.arEther[0]);
        cmdOsSprintf(
            workStr, "%02X%02X%02X%02X%02X%02X",
            workPtr[0], workPtr[1], workPtr[2], workPtr[3], workPtr[4], workPtr[5]);

        fieldOutput("%s", workStr);
    }
    else
    {
        fieldOutput("%s", "000000000000");
    }
    fieldOutputSetAppendMode();

    fieldOutput(
        "%d%d",
        action.policer.policerEnable,
        action.policer.policerId);
    fieldOutputSetAppendMode();

    if (action.egressPolicy == GT_FALSE)
    {
        fieldOutput(
            "%d%d%d%d%d%d%d%d",
            action.vlan.ingress.modifyVlan,
            0 /*action.vlan.egress.vlanCmd*/,
            action.vlan.ingress.nestedVlan,
            action.vlan.ingress.vlanId,
            action.vlan.ingress.precedence,
            action.vlan.ingress.vlanId1Cmd,
            0 /*action.vlan.egress.vlanId1ModifyEnable*/,
            action.vlan.ingress.vlanId1);
    }
    else
    {
        fieldOutput(
            "%d%d%d%d%d%d%d%d",
            0 /*action.vlan.ingress.modifyVlan*/,
            action.vlan.egress.vlanCmd,
            0 /*action.vlan.ingress.nestedVlan*/,
            action.vlan.egress.vlanId,
            0 /*action.vlan.ingress.precedence*/,
            0 /*action.vlan.ingress.vlanId1Cmd*/,
            action.vlan.egress.vlanId1ModifyEnable,
            action.vlan.egress.vlanId1);
    }
    fieldOutputSetAppendMode();

    fieldOutput(
        "%d%d",
        action.sourceId.assignSourceId,
        action.sourceId.sourceIdValue);
    fieldOutputSetAppendMode();

    fieldOutput(
        "%d%d%d%d%d%d",
        action.bypassBridge,
        action.bypassIngressPipe,
        action.lookupConfig.ipclConfigIndex,
        action.lookupConfig.pcl0_1OverrideConfigIndex,
        action.lookupConfig.pcl1OverrideConfigIndex,
        action.mirror.mirrorTcpRstAndFinPacketsToCpu);
    fieldOutputSetAppendMode();

    fieldOutput(
        "%d%d%d%d",
        action.oam.timeStampEnable,
        action.oam.offsetIndex,
        action.oam.oamProcessEnable,
        action.oam.oamProfile);
    fieldOutputSetAppendMode();

    fieldOutput(
        "%d%d%d%d%d%d%d",
        action.flowId,
        action.setMacToMe,
        action.sourcePort.assignSourcePortEnable,
        action.sourcePort.sourcePortValue,
        action.channelTypeToOpcodeMapEnable,
        action.tmQueueId,
        action.exactMatchOverPclEn);
    fieldOutputSetAppendMode();

    /* falcon fields */
    fieldOutput(
        "%d%d%d%d%d%d%d%d%d%d",
        action.unknownSaCommandEnable,
        action.unknownSaCommand,
        action.terminateCutThroughMode,
        action.latencyMonitor.monitoringEnable,
        action.latencyMonitor.latencyProfile,
        action.skipFdbSaLookup,
        action.triggerInterrupt,
        action.mirror.mirrorToTxAnalyzerPortEn,
        action.mirror.egressMirrorToAnalyzerIndex,
        action.mirror.egressMirrorToAnalyzerMode);
    fieldOutputSetAppendMode();

    /* PHA related fields */
    {
        fieldOutput(
            "%d%d%d",
            action.epclPhaInfo.phaThreadIdAssignmentMode,
            action.epclPhaInfo.phaThreadId,
            action.epclPhaInfo.phaThreadType);
        fieldOutputSetAppendMode();

        /* 2 fields send to the Galtis (NOT implemented as tabulator for the 'union')*/
        if(action.epclPhaInfo.phaThreadType == CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_IOAM_INGRESS_SWITCH_E)
        {
            fieldOutput(
                "%d%d",
                action.epclPhaInfo.phaThreadUnion.epclIoamIngressSwitch.ioamIncludesBothE2EOptionAndTraceOption,
                0);
        }
        else
        {
            fieldOutput(
                "%d%d",
                0,
                action.epclPhaInfo.phaThreadUnion.notNeeded);
        }
    }

    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/* End of Lion3 Action Table */


/* tables cpssDxCh3PclIngressRule and cpssDxCh3PclEgressRule global variables */

static    GT_BOOL                              mask_set = GT_FALSE; /* is mask set*/
static    GT_BOOL                              pattern_set = GT_FALSE; /* is pattern set*/
static    GT_U32                               mask_ruleIndex = 0;
static    GT_U32                               pattern_ruleIndex = 0;
/********************cpssDxCh3PclIngressRule********************************/

static CMD_STATUS wrCpssDxCh3PclIngressRule_STD_NOT_IP_Write
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8           devNum;
    GT_BYTE_ARRY                       maskBArr, patternBArr;
    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;


    /* map input arguments to locals */
    /*
       inFields[1] = mask/pattern indication
       mask = 0
       pattern = 1
    */
    devNum=(GT_U8)inArgs[0];

    if(inFields[1] == 0) /* mask */
    {
    mask_ruleIndex = (GT_U32)inFields[0];

    cmdOsMemCpy(
        &maskData.ruleStdNotIp.common.portListBmp,
        &ruleSet_portListBmpMask,
        sizeof(CPSS_PORTS_BMP_STC));

    maskData.ruleStdNotIp.common.pclId = (GT_U16)inFields[2];
    CONVERT_PCL_MASK_PCL_ID_TEST_TO_CPSS_MAC(maskData.ruleStdNotIp.common.pclId);
    maskData.ruleStdNotIp.common.macToMe=(GT_U8)inFields[3];
    maskData.ruleStdNotIp.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[4];


    maskData.ruleStdNotIp.common.isTagged = (GT_U8)inFields[5];
    maskData.ruleStdNotIp.common.vid = (GT_U16)inFields[6];
    maskData.ruleStdNotIp.common.up = (GT_U8)inFields[7];
    maskData.ruleStdNotIp.common.qosProfile = (GT_U8)inFields[8];
    maskData.ruleStdNotIp.common.isIp = (GT_U8)inFields[9];
    maskData.ruleStdNotIp.common.isL2Valid = (GT_U8)inFields[10];
    maskData.ruleStdNotIp.common.isUdbValid = (GT_U8)inFields[11];
    maskData.ruleStdNotIp.isIpv4 = (GT_U8)inFields[12];
    maskData.ruleStdNotIp.etherType = (GT_U16)inFields[13];
    maskData.ruleStdNotIp.isArp = (GT_BOOL)inFields[14];
    maskData.ruleStdNotIp.l2Encap = (GT_BOOL)inFields[15];
    galtisMacAddr(&maskData.ruleStdNotIp.macDa, (GT_U8*)inFields[16]);
    galtisMacAddr(&maskData.ruleStdNotIp.macSa, (GT_U8*)inFields[17]);
    galtisBArray(&maskBArr,(GT_U8*)inFields[18]);

    cmdOsMemCpy(
            maskData.ruleStdNotIp.udb15_17, maskBArr.data,
            maskBArr.length);

    if( GT_TRUE == sip5Pcl )
    {
        maskData.ruleStdNotIp.common.sourceDevice = (GT_U16)inFields[19];
        maskData.ruleStdNotIp.tag1Exist = (GT_U8)inFields[20];
        maskData.ruleStdNotIp.vid1 = (GT_U16)inFields[21];
        maskData.ruleStdNotIp.cfi1 = (GT_U8)inFields[22];
        maskData.ruleStdNotIp.up1 = (GT_U8)inFields[23];
        maskData.ruleStdNotIp.vrfId = (GT_U16)inFields[24];
        maskData.ruleStdNotIp.trunkHash = (GT_U8)inFields[25];
        galtisBArray(&maskBArr,(GT_U8*)inFields[26]);

        cmdOsMemCpy(
            maskData.ruleStdNotIp.udb23_26, maskBArr.data,
            maskBArr.length);
    }

        mask_set = GT_TRUE;
    }
    else /* pattern */
    {
        pattern_ruleIndex = (GT_U32)inFields[0];

        cmdOsMemCpy(
            &patternData.ruleStdNotIp.common.portListBmp,
            &ruleSet_portListBmpPattern,
            sizeof(CPSS_PORTS_BMP_STC));

        patternData.ruleStdNotIp.common.pclId = (GT_U16)inFields[2];
        patternData.ruleStdNotIp.common.macToMe=(GT_U8)inFields[3];
        patternData.ruleStdNotIp.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[4];

        devNum=(GT_U8)inArgs[0];
        inArgs[0]=devNum;

        patternData.ruleStdNotIp.common.isTagged = (GT_U8)inFields[5];
        patternData.ruleStdNotIp.common.vid = (GT_U16)inFields[6];
        patternData.ruleStdNotIp.common.up = (GT_U8)inFields[7];
        patternData.ruleStdNotIp.common.qosProfile = (GT_U8)inFields[8];
        patternData.ruleStdNotIp.common.isIp = (GT_U8)inFields[9];
        patternData.ruleStdNotIp.common.isL2Valid = (GT_U8)inFields[10];
        patternData.ruleStdNotIp.common.isUdbValid = (GT_U8)inFields[11];
        patternData.ruleStdNotIp.isIpv4 = (GT_U8)inFields[12];
        patternData.ruleStdNotIp.etherType = (GT_U16)inFields[13];
        patternData.ruleStdNotIp.isArp = (GT_BOOL)inFields[14];
        patternData.ruleStdNotIp.l2Encap = (GT_BOOL)inFields[15];
        galtisMacAddr(&patternData.ruleStdNotIp.macDa, (GT_U8*)inFields[16]);
        galtisMacAddr(&patternData.ruleStdNotIp.macSa, (GT_U8*)inFields[17]);
        galtisBArray(&patternBArr,(GT_U8*)inFields[18]);

       cmdOsMemCpy(
        patternData.ruleStdNotIp.udb15_17, patternBArr.data,
        patternBArr.length);

       if( GT_TRUE == sip5Pcl )
       {
           patternData.ruleStdNotIp.common.sourceDevice = (GT_U16)inFields[19];
           patternData.ruleStdNotIp.tag1Exist = (GT_U8)inFields[20];
           patternData.ruleStdNotIp.vid1 = (GT_U16)inFields[21];
           patternData.ruleStdNotIp.cfi1 = (GT_U8)inFields[22];
           patternData.ruleStdNotIp.up1 = (GT_U8)inFields[23];
           patternData.ruleStdNotIp.vrfId = (GT_U16)inFields[24];
           patternData.ruleStdNotIp.trunkHash = (GT_U8)inFields[25];
           galtisBArray(&patternBArr,(GT_U8*)inFields[26]);

           cmdOsMemCpy(
            patternData.ruleStdNotIp.udb23_26, patternBArr.data,
            patternBArr.length);
       }

        pattern_set = GT_TRUE;
    }

    if ((mask_set == GT_TRUE) && (pattern_set == GT_TRUE))
    {
        PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleStdNotIp.common.sourcePort);
    }

    return CMD_OK;
}
/*********************************************************************************/

static CMD_STATUS wrCpssDxCh3PclIngressRule_STD_IP_L2_QOS_Write
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8           devNum;
    GT_BYTE_ARRY                       maskBArr, patternBArr;
    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;


    /* map input arguments to locals */
    /*
       inFields[1] = mask/pattern indication
       mask = 0
       pattern = 1
    */
    devNum=(GT_U8)inArgs[0];

    if(inFields[1] == 0) /* mask */
    {
    mask_ruleIndex = (GT_U32)inFields[0];

    cmdOsMemCpy(
        &maskData.ruleStdIpL2Qos.common.portListBmp,
        &ruleSet_portListBmpMask,
        sizeof(CPSS_PORTS_BMP_STC));

    maskData.ruleStdIpL2Qos.common.pclId = (GT_U16)inFields[2];
    CONVERT_PCL_MASK_PCL_ID_TEST_TO_CPSS_MAC(maskData.ruleStdIpL2Qos.common.pclId);
    maskData.ruleStdIpL2Qos.common.macToMe=(GT_U8)inFields[3];
    maskData.ruleStdIpL2Qos.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[4];
    maskData.ruleStdIpL2Qos.common.isTagged = (GT_U8)inFields[5];
    maskData.ruleStdIpL2Qos.common.vid = (GT_U16)inFields[6];
    maskData.ruleStdIpL2Qos.common.up = (GT_U8)inFields[7];
    maskData.ruleStdIpL2Qos.common.qosProfile = (GT_U8)inFields[8];
    maskData.ruleStdIpL2Qos.common.isIp = (GT_U8)inFields[9];
    maskData.ruleStdIpL2Qos.common.isL2Valid = (GT_U8)inFields[10];
    maskData.ruleStdIpL2Qos.common.isUdbValid = (GT_U8)inFields[11];
    maskData.ruleStdIpL2Qos.commonStdIp.isIpv4 = (GT_U8)inFields[12];
    maskData.ruleStdIpL2Qos.commonStdIp.ipProtocol = (GT_U8)inFields[13];
    maskData.ruleStdIpL2Qos.commonStdIp.dscp = (GT_U8)inFields[14];
    maskData.ruleStdIpL2Qos.commonStdIp.isL4Valid = (GT_U8)inFields[15];
    maskData.ruleStdIpL2Qos.commonStdIp.l4Byte2 = (GT_U8)inFields[16];
    maskData.ruleStdIpL2Qos.commonStdIp.l4Byte3 = (GT_U8)inFields[17];
    maskData.ruleStdIpL2Qos.commonStdIp.ipHeaderOk = (GT_U8)inFields[18];
    maskData.ruleStdIpL2Qos.commonStdIp.ipv4Fragmented = (GT_U8)inFields[19];
    maskData.ruleStdIpL2Qos.isArp = (GT_BOOL)inFields[20];
    maskData.ruleStdIpL2Qos.isIpv6ExtHdrExist = (GT_BOOL)inFields[21];
    maskData.ruleStdIpL2Qos.isIpv6HopByHop = (GT_BOOL)inFields[22];
    galtisMacAddr(&maskData.ruleStdIpL2Qos.macDa, (GT_U8*)inFields[23]);
    galtisMacAddr(&maskData.ruleStdIpL2Qos.macSa, (GT_U8*)inFields[24]);
    galtisBArray(&maskBArr,(GT_U8*)inFields[25]);

    if(maskBArr.length > 2)
    {
        maskBArr.length = 2;
    }

    cmdOsMemCpy(
            maskData.ruleStdIpL2Qos.udb18_19, maskBArr.data,
            maskBArr.length);

    if( GT_TRUE == sip5Pcl )
    {
        maskData.ruleStdIpL2Qos.common.sourceDevice = (GT_U16)inFields[26];
        maskData.ruleStdIpL2Qos.vrfId = (GT_U16)inFields[27];
        galtisBArray(&maskBArr,(GT_U8*)inFields[28]);

        cmdOsMemCpy(
            maskData.ruleStdIpL2Qos.udb27_30, maskBArr.data,
            maskBArr.length);
    }

        mask_set = GT_TRUE;
    }
    else /* pattern */
    {
        pattern_ruleIndex = (GT_U32)inFields[0];

        cmdOsMemCpy(
            &patternData.ruleStdIpL2Qos.common.portListBmp,
            &ruleSet_portListBmpPattern,
            sizeof(CPSS_PORTS_BMP_STC));

        patternData.ruleStdIpL2Qos.common.pclId = (GT_U16)inFields[2];
        patternData.ruleStdIpL2Qos.common.macToMe=(GT_U8)inFields[3];
        patternData.ruleStdIpL2Qos.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[4];
        patternData.ruleStdIpL2Qos.common.isTagged = (GT_U8)inFields[5];
        patternData.ruleStdIpL2Qos.common.vid = (GT_U16)inFields[6];
        patternData.ruleStdIpL2Qos.common.up = (GT_U8)inFields[7];
        patternData.ruleStdIpL2Qos.common.qosProfile = (GT_U8)inFields[8];
        patternData.ruleStdIpL2Qos.common.isIp = (GT_U8)inFields[9];
        patternData.ruleStdIpL2Qos.common.isL2Valid = (GT_U8)inFields[10];
        patternData.ruleStdIpL2Qos.common.isUdbValid = (GT_U8)inFields[11];
        patternData.ruleStdIpL2Qos.commonStdIp.isIpv4 = (GT_U8)inFields[12];
        patternData.ruleStdIpL2Qos.commonStdIp.ipProtocol = (GT_U8)inFields[13];
        patternData.ruleStdIpL2Qos.commonStdIp.dscp = (GT_U8)inFields[14];
        patternData.ruleStdIpL2Qos.commonStdIp.isL4Valid = (GT_U8)inFields[15];
        patternData.ruleStdIpL2Qos.commonStdIp.l4Byte2 = (GT_U8)inFields[16];
        patternData.ruleStdIpL2Qos.commonStdIp.l4Byte3 = (GT_U8)inFields[17];
        patternData.ruleStdIpL2Qos.commonStdIp.ipHeaderOk = (GT_U8)inFields[18];
        patternData.ruleStdIpL2Qos.commonStdIp.ipv4Fragmented = (GT_U8)inFields[19];
        patternData.ruleStdIpL2Qos.isArp = (GT_BOOL)inFields[20];
        patternData.ruleStdIpL2Qos.isIpv6ExtHdrExist = (GT_BOOL)inFields[21];
        patternData.ruleStdIpL2Qos.isIpv6HopByHop = (GT_BOOL)inFields[22];
        galtisMacAddr(&patternData.ruleStdIpL2Qos.macDa, (GT_U8*)inFields[23]);
        galtisMacAddr(&patternData.ruleStdIpL2Qos.macSa, (GT_U8*)inFields[24]);
        galtisBArray(&patternBArr,(GT_U8*)inFields[25]);

        if(patternBArr.length > 2)
        {
            patternBArr.length = 2;
        }

        cmdOsMemCpy(
            patternData.ruleStdIpL2Qos.udb18_19, patternBArr.data,
            patternBArr.length);

        if( GT_TRUE == sip5Pcl )
        {
            patternData.ruleStdIpL2Qos.common.sourceDevice = (GT_U16)inFields[26];
            patternData.ruleStdIpL2Qos.vrfId = (GT_U16)inFields[27];
            galtisBArray(&patternBArr,(GT_U8*)inFields[28]);

            cmdOsMemCpy(
                patternData.ruleStdIpL2Qos.udb27_30, patternBArr.data,
                patternBArr.length);
        }

        pattern_set = GT_TRUE;
    }

    if ((mask_set == GT_TRUE) && (pattern_set == GT_TRUE))
    {
        PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleStdIpL2Qos.common.sourcePort);
    }
    return CMD_OK;
}
/**********************************************************************************/

static CMD_STATUS wrCpssDxCh3PclIngressRule_STD_IPV4_L4_Write
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8           devNum;
    GT_BYTE_ARRY                       maskBArr, patternBArr;
    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* map input arguments to locals */
    /*
       inFields[1] = mask/pattern indication
       mask = 0
       pattern = 1
    */
    if(inFields[1] == 0) /* mask */
    {
    mask_ruleIndex = (GT_U32)inFields[0];

    cmdOsMemCpy(
        &maskData.ruleStdIpv4L4.common.portListBmp,
        &ruleSet_portListBmpMask,
        sizeof(CPSS_PORTS_BMP_STC));

    maskData.ruleStdIpv4L4.common.pclId = (GT_U16)inFields[2];
    CONVERT_PCL_MASK_PCL_ID_TEST_TO_CPSS_MAC(maskData.ruleStdIpv4L4.common.pclId);
    maskData.ruleStdIpv4L4.common.macToMe=(GT_U8)inFields[3];
    maskData.ruleStdIpv4L4.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[4];
    maskData.ruleStdIpv4L4.common.isTagged = (GT_U8)inFields[5];
    maskData.ruleStdIpv4L4.common.vid = (GT_U16)inFields[6];
    maskData.ruleStdIpv4L4.common.up = (GT_U8)inFields[7];
    maskData.ruleStdIpv4L4.common.qosProfile = (GT_U8)inFields[8];
    maskData.ruleStdIpv4L4.common.isIp = (GT_U8)inFields[9];
    maskData.ruleStdIpv4L4.common.isL2Valid = (GT_U8)inFields[10];
    maskData.ruleStdIpv4L4.common.isUdbValid = (GT_U8)inFields[11];
    maskData.ruleStdIpv4L4.commonStdIp.isIpv4 = (GT_U8)inFields[12];
    maskData.ruleStdIpv4L4.commonStdIp.ipProtocol = (GT_U8)inFields[13];
    maskData.ruleStdIpv4L4.commonStdIp.dscp = (GT_U8)inFields[14];
    maskData.ruleStdIpv4L4.commonStdIp.isL4Valid = (GT_U8)inFields[15];
    maskData.ruleStdIpv4L4.commonStdIp.l4Byte2 = (GT_U8)inFields[16];
    maskData.ruleStdIpv4L4.commonStdIp.l4Byte3 = (GT_U8)inFields[17];
    maskData.ruleStdIpv4L4.commonStdIp.ipHeaderOk = (GT_U8)inFields[18];
    maskData.ruleStdIpv4L4.commonStdIp.ipv4Fragmented = (GT_U8)inFields[19];
    maskData.ruleStdIpv4L4.isArp = (GT_BOOL)inFields[20];
    maskData.ruleStdIpv4L4.isBc = (GT_BOOL)inFields[21];
    galtisIpAddr(&maskData.ruleStdIpv4L4.sip, (GT_U8*)inFields[22]);
    galtisIpAddr(&maskData.ruleStdIpv4L4.dip, (GT_U8*)inFields[23]);
    maskData.ruleStdIpv4L4.l4Byte0 = (GT_U8)inFields[24];
    maskData.ruleStdIpv4L4.l4Byte1 = (GT_U8)inFields[25];
    maskData.ruleStdIpv4L4.l4Byte13 = (GT_U8)inFields[26];
    galtisBArray(&maskBArr,(GT_U8*)inFields[27]);
    cmdOsMemCpy(
            maskData.ruleStdIpv4L4.udb20_22, maskBArr.data,
            maskBArr.length);

    if( GT_TRUE == sip5Pcl )
    {
        maskData.ruleStdIpv4L4.common.sourceDevice = (GT_U16)inFields[28];
        maskData.ruleStdIpv4L4.vrfId = (GT_U16)inFields[29];
        maskData.ruleStdIpv4L4.trunkHash = (GT_U8)inFields[30];
        galtisBArray(&maskBArr,(GT_U8*)inFields[31]);

        cmdOsMemCpy(
            maskData.ruleStdIpv4L4.udb31_34, maskBArr.data,
            maskBArr.length);
    }

        mask_set = GT_TRUE;
    }
    else /* pattern */
    {
    pattern_ruleIndex = (GT_U32)inFields[0];

    cmdOsMemCpy(
        &patternData.ruleStdIpv4L4.common.portListBmp,
        &ruleSet_portListBmpPattern,
        sizeof(CPSS_PORTS_BMP_STC));

    patternData.ruleStdIpv4L4.common.pclId = (GT_U16)inFields[2];
    patternData.ruleStdIpv4L4.common.macToMe=(GT_U8)inFields[3];
    patternData.ruleStdIpv4L4.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[4];
    patternData.ruleStdIpv4L4.common.isTagged = (GT_U8)inFields[5];
    patternData.ruleStdIpv4L4.common.vid = (GT_U16)inFields[6];
    patternData.ruleStdIpv4L4.common.up = (GT_U8)inFields[7];
    patternData.ruleStdIpv4L4.common.qosProfile = (GT_U8)inFields[8];
    patternData.ruleStdIpv4L4.common.isIp = (GT_U8)inFields[9];
    patternData.ruleStdIpv4L4.common.isL2Valid = (GT_U8)inFields[10];
    patternData.ruleStdIpv4L4.common.isUdbValid = (GT_U8)inFields[11];
    patternData.ruleStdIpv4L4.commonStdIp.isIpv4 = (GT_U8)inFields[12];
    patternData.ruleStdIpv4L4.commonStdIp.ipProtocol = (GT_U8)inFields[13];
    patternData.ruleStdIpv4L4.commonStdIp.dscp = (GT_U8)inFields[14];
    patternData.ruleStdIpv4L4.commonStdIp.isL4Valid = (GT_U8)inFields[15];
    patternData.ruleStdIpv4L4.commonStdIp.l4Byte2 = (GT_U8)inFields[16];
    patternData.ruleStdIpv4L4.commonStdIp.l4Byte3 = (GT_U8)inFields[17];
    patternData.ruleStdIpv4L4.commonStdIp.ipHeaderOk = (GT_U8)inFields[18];
    patternData.ruleStdIpv4L4.commonStdIp.ipv4Fragmented = (GT_U8)inFields[19];
    patternData.ruleStdIpv4L4.isArp = (GT_BOOL)inFields[20];
    patternData.ruleStdIpv4L4.isBc = (GT_BOOL)inFields[21];
    galtisIpAddr(&patternData.ruleStdIpv4L4.sip, (GT_U8*)inFields[22]);
    galtisIpAddr(&patternData.ruleStdIpv4L4.dip, (GT_U8*)inFields[23]);
    patternData.ruleStdIpv4L4.l4Byte0 = (GT_U8)inFields[24];
    patternData.ruleStdIpv4L4.l4Byte1 = (GT_U8)inFields[25];
    patternData.ruleStdIpv4L4.l4Byte13 = (GT_U8)inFields[26];
    galtisBArray(&patternBArr,(GT_U8*)inFields[27]);

        cmdOsMemCpy(
            patternData.ruleStdIpv4L4.udb20_22, patternBArr.data,
            patternBArr.length);

    if( GT_TRUE == sip5Pcl )
    {
        patternData.ruleStdIpv4L4.common.sourceDevice = (GT_U16)inFields[28];
        patternData.ruleStdIpv4L4.vrfId = (GT_U16)inFields[29];
        patternData.ruleStdIpv4L4.trunkHash = (GT_U8)inFields[30];
        galtisBArray(&patternBArr,(GT_U8*)inFields[31]);

        cmdOsMemCpy(
            patternData.ruleStdIpv4L4.udb31_34, patternBArr.data,
            patternBArr.length);
    }

        pattern_set = GT_TRUE;
    }

    if ((mask_set == GT_TRUE) && (pattern_set == GT_TRUE))
    {
        PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleStdIpv4L4.common.sourcePort);
    }
    return CMD_OK;
}

/**********************************************************************************/

static CMD_STATUS wrCpssDxCh3PclIngressRule_STD_IPV6_DIP_Write
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_BYTE_ARRY    maskBArr, patternBArr;
    GT_U8           devNum;
    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* map input arguments to locals */
    /*
       inFields[1] = mask/pattern indication
       mask = 0
       pattern = 1
    */
    if(inFields[1] == 0) /* mask */
    {
    mask_ruleIndex = (GT_U32)inFields[0];

    cmdOsMemCpy(
        &maskData.ruleStdIpv6Dip.common.portListBmp,
        &ruleSet_portListBmpMask,
        sizeof(CPSS_PORTS_BMP_STC));

    maskData.ruleStdIpv6Dip.common.pclId = (GT_U16)inFields[2];
    CONVERT_PCL_MASK_PCL_ID_TEST_TO_CPSS_MAC(maskData.ruleStdIpv6Dip.common.pclId);
    maskData.ruleStdIpv6Dip.common.macToMe=(GT_U8)inFields[3];
    maskData.ruleStdIpv6Dip.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[4];
    maskData.ruleStdIpv6Dip.common.isTagged = (GT_U8)inFields[5];
    maskData.ruleStdIpv6Dip.common.vid = (GT_U16)inFields[6];
    maskData.ruleStdIpv6Dip.common.up = (GT_U8)inFields[7];
    maskData.ruleStdIpv6Dip.common.qosProfile = (GT_U8)inFields[8];
    maskData.ruleStdIpv6Dip.common.isIp = (GT_U8)inFields[9];
    maskData.ruleStdIpv6Dip.common.isL2Valid = (GT_U8)inFields[10];
    maskData.ruleStdIpv6Dip.common.isUdbValid = (GT_U8)inFields[11];
    maskData.ruleStdIpv6Dip.commonStdIp.isIpv4 = (GT_U8)inFields[12];
    maskData.ruleStdIpv6Dip.commonStdIp.ipProtocol = (GT_U8)inFields[13];
    maskData.ruleStdIpv6Dip.commonStdIp.dscp = (GT_U8)inFields[14];
    maskData.ruleStdIpv6Dip.commonStdIp.isL4Valid = (GT_U8)inFields[15];
    maskData.ruleStdIpv6Dip.commonStdIp.l4Byte2 = (GT_U8)inFields[16];
    maskData.ruleStdIpv6Dip.commonStdIp.l4Byte3 = (GT_U8)inFields[17];
    maskData.ruleStdIpv6Dip.commonStdIp.ipHeaderOk = (GT_U8)inFields[18];
    maskData.ruleStdIpv6Dip.commonStdIp.ipv4Fragmented = (GT_U8)inFields[19];
    maskData.ruleStdIpv6Dip.isArp = (GT_BOOL)inFields[20];
    maskData.ruleStdIpv6Dip.isIpv6ExtHdrExist = (GT_U8)inFields[21];
    maskData.ruleStdIpv6Dip.isIpv6HopByHop = (GT_U8)inFields[22];
    galtisIpv6Addr(&maskData.ruleStdIpv6Dip.dip, (GT_U8*)inFields[23]);

    if( GT_TRUE == sip5Pcl )
    {
        maskData.ruleStdIpv6Dip.common.sourceDevice = (GT_U16)inFields[24];
        galtisBArray(&maskBArr,(GT_U8*)inFields[25]);

        cmdOsMemCpy(
            maskData.ruleStdIpv6Dip.udb47_49, maskBArr.data,
            maskBArr.length);

        galtisBArray(&maskBArr,(GT_U8*)inFields[26]);

        if( maskBArr.length )
        {
            maskData.ruleStdIpv6Dip.udb0 = maskBArr.data[0];
        }
    }

    mask_set = GT_TRUE;
    }
    else /* pattern */
    {
    pattern_ruleIndex = (GT_U32)inFields[0];

    cmdOsMemCpy(
        &patternData.ruleStdIpv6Dip.common.portListBmp,
        &ruleSet_portListBmpPattern,
        sizeof(CPSS_PORTS_BMP_STC));

    patternData.ruleStdIpv6Dip.common.pclId = (GT_U16)inFields[2];
    patternData.ruleStdIpv6Dip.common.macToMe=(GT_U8)inFields[3];
    patternData.ruleStdIpv6Dip.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[4];
    patternData.ruleStdIpv6Dip.common.isTagged = (GT_U8)inFields[5];
    patternData.ruleStdIpv6Dip.common.vid = (GT_U16)inFields[6];
    patternData.ruleStdIpv6Dip.common.up = (GT_U8)inFields[7];
    patternData.ruleStdIpv6Dip.common.qosProfile = (GT_U8)inFields[8];
    patternData.ruleStdIpv6Dip.common.isIp = (GT_U8)inFields[9];
    patternData.ruleStdIpv6Dip.common.isL2Valid = (GT_U8)inFields[10];
    patternData.ruleStdIpv6Dip.common.isUdbValid = (GT_U8)inFields[11];
    patternData.ruleStdIpv6Dip.commonStdIp.isIpv4 = (GT_U8)inFields[12];
    patternData.ruleStdIpv6Dip.commonStdIp.ipProtocol = (GT_U8)inFields[13];
    patternData.ruleStdIpv6Dip.commonStdIp.dscp = (GT_U8)inFields[14];
    patternData.ruleStdIpv6Dip.commonStdIp.isL4Valid = (GT_U8)inFields[15];
    patternData.ruleStdIpv6Dip.commonStdIp.l4Byte2 = (GT_U8)inFields[16];
    patternData.ruleStdIpv6Dip.commonStdIp.l4Byte3 = (GT_U8)inFields[17];
    patternData.ruleStdIpv6Dip.commonStdIp.ipHeaderOk = (GT_U8)inFields[18];
    patternData.ruleStdIpv6Dip.commonStdIp.ipv4Fragmented = (GT_U8)inFields[19];
    patternData.ruleStdIpv6Dip.isArp = (GT_BOOL)inFields[20];
    patternData.ruleStdIpv6Dip.isIpv6ExtHdrExist = (GT_U8)inFields[21];
    patternData.ruleStdIpv6Dip.isIpv6HopByHop = (GT_U8)inFields[22];
    galtisIpv6Addr(&patternData.ruleStdIpv6Dip.dip, (GT_U8*)inFields[23]);

    if( GT_TRUE == sip5Pcl )
    {
        patternData.ruleStdIpv6Dip.common.sourceDevice = (GT_U16)inFields[24];
        galtisBArray(&patternBArr,(GT_U8*)inFields[25]);

        cmdOsMemCpy(
            patternData.ruleStdIpv6Dip.udb47_49, patternBArr.data,
            patternBArr.length);

        galtisBArray(&patternBArr,(GT_U8*)inFields[26]);

        if( patternBArr.length )
        {
            patternData.ruleStdIpv6Dip.udb0 = patternBArr.data[0];
        }
    }

    pattern_set = GT_TRUE;
    }

    if ((mask_set == GT_TRUE) && (pattern_set == GT_TRUE))
    {
        PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleStdIpv6Dip.common.sourcePort);
    }
    return CMD_OK;
}
/*********************************************************************************/

static CMD_STATUS wrCpssDxCh3PclIngressRule_EXT_NOT_IPV6_Write
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_BYTE_ARRY                       maskBArr, patternBArr;
    GT_U8           devNum;
    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* map input arguments to locals */
    /*
       inFields[1] = mask/pattern indication
       mask = 0
       pattern = 1
    */
    if(inFields[1] == 0) /* mask */
    {
    mask_ruleIndex = (GT_U32)inFields[0];

    cmdOsMemCpy(
        &maskData.ruleExtNotIpv6.common.portListBmp,
        &ruleSet_portListBmpMask,
        sizeof(CPSS_PORTS_BMP_STC));

    maskData.ruleExtNotIpv6.common.pclId = (GT_U16)inFields[2];
    maskData.ruleExtNotIpv6.common.macToMe=(GT_U8)inFields[3];
    maskData.ruleExtNotIpv6.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[4];
    maskData.ruleExtNotIpv6.common.isTagged = (GT_U8)inFields[5];
    maskData.ruleExtNotIpv6.common.vid = (GT_U16)inFields[6];
    maskData.ruleExtNotIpv6.common.up = (GT_U8)inFields[7];
    maskData.ruleExtNotIpv6.common.qosProfile = (GT_U8)inFields[8];
    maskData.ruleExtNotIpv6.common.isIp = (GT_U8)inFields[9];
    maskData.ruleExtNotIpv6.common.isL2Valid = (GT_U8)inFields[10];
    maskData.ruleExtNotIpv6.common.isUdbValid = (GT_U8)inFields[11];
    maskData.ruleExtNotIpv6.commonExt.isIpv6 = (GT_U8)inFields[12];
    maskData.ruleExtNotIpv6.commonExt.ipProtocol = (GT_U8)inFields[13];
    maskData.ruleExtNotIpv6.commonExt.dscp = (GT_U8)inFields[14];
    maskData.ruleExtNotIpv6.commonExt.isL4Valid = (GT_U8)inFields[15];
    maskData.ruleExtNotIpv6.commonExt.l4Byte0 = (GT_U8)inFields[16];
    maskData.ruleExtNotIpv6.commonExt.l4Byte1 = (GT_U8)inFields[17];
    maskData.ruleExtNotIpv6.commonExt.l4Byte2 = (GT_U8)inFields[18];
    maskData.ruleExtNotIpv6.commonExt.l4Byte3 = (GT_U8)inFields[19];
    maskData.ruleExtNotIpv6.commonExt.l4Byte13 = (GT_U8)inFields[20];
    maskData.ruleExtNotIpv6.commonExt.ipHeaderOk = (GT_U8)inFields[21];
    galtisIpAddr(&maskData.ruleExtNotIpv6.sip, (GT_U8*)inFields[22]);
    galtisIpAddr(&maskData.ruleExtNotIpv6.dip, (GT_U8*)inFields[23]);
    maskData.ruleExtNotIpv6.etherType = (GT_U16)inFields[24];
    maskData.ruleExtNotIpv6.l2Encap = (GT_U8)inFields[25];
    galtisMacAddr(&maskData.ruleExtNotIpv6.macDa, (GT_U8*)inFields[26]);
    galtisMacAddr(&maskData.ruleExtNotIpv6.macSa, (GT_U8*)inFields[27]);
    maskData.ruleExtNotIpv6.ipv4Fragmented = (GT_BOOL)inFields[28];
    galtisBArray(&maskBArr,(GT_U8*)inFields[29]);

    cmdOsMemCpy(
            maskData.ruleExtNotIpv6.udb0_5, maskBArr.data,
            maskBArr.length);

    if( GT_TRUE == sip5Pcl )
    {
        maskData.ruleExtNotIpv6.common.sourceDevice = (GT_U16)inFields[30];
        maskData.ruleExtNotIpv6.tag1Exist = (GT_U8)inFields[31];
        maskData.ruleExtNotIpv6.vid1 = (GT_U16)inFields[32];
        maskData.ruleExtNotIpv6.cfi1 = (GT_U8)inFields[33];
        maskData.ruleExtNotIpv6.up1 = (GT_U8)inFields[34];
        maskData.ruleExtNotIpv6.vrfId = (GT_U16)inFields[35];
        maskData.ruleExtNotIpv6.trunkHash = (GT_U8)inFields[36];
        galtisBArray(&maskBArr,(GT_U8*)inFields[37]);

        cmdOsMemCpy(
            maskData.ruleExtNotIpv6.udb39_46, maskBArr.data,
            maskBArr.length);
    }

    mask_set = GT_TRUE;
    }
    else /* pattern */
    {
    pattern_ruleIndex = (GT_U32)inFields[0];

    cmdOsMemCpy(
        &patternData.ruleExtNotIpv6.common.portListBmp,
        &ruleSet_portListBmpPattern,
        sizeof(CPSS_PORTS_BMP_STC));

    patternData.ruleExtNotIpv6.common.pclId = (GT_U16)inFields[2];
    patternData.ruleExtNotIpv6.common.macToMe=(GT_U8)inFields[3];
    patternData.ruleExtNotIpv6.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[4];
    patternData.ruleExtNotIpv6.common.isTagged = (GT_U8)inFields[5];
    patternData.ruleExtNotIpv6.common.vid = (GT_U16)inFields[6];
    patternData.ruleExtNotIpv6.common.up = (GT_U8)inFields[7];
    patternData.ruleExtNotIpv6.common.qosProfile = (GT_U8)inFields[8];
    patternData.ruleExtNotIpv6.common.isIp = (GT_U8)inFields[9];
    patternData.ruleExtNotIpv6.common.isL2Valid = (GT_U8)inFields[10];
    patternData.ruleExtNotIpv6.common.isUdbValid = (GT_U8)inFields[11];
    patternData.ruleExtNotIpv6.commonExt.isIpv6 = (GT_U8)inFields[12];
    patternData.ruleExtNotIpv6.commonExt.ipProtocol = (GT_U8)inFields[13];
    patternData.ruleExtNotIpv6.commonExt.dscp = (GT_U8)inFields[14];
    patternData.ruleExtNotIpv6.commonExt.isL4Valid = (GT_U8)inFields[15];
    patternData.ruleExtNotIpv6.commonExt.l4Byte0 = (GT_U8)inFields[16];
    patternData.ruleExtNotIpv6.commonExt.l4Byte1 = (GT_U8)inFields[17];
    patternData.ruleExtNotIpv6.commonExt.l4Byte2 = (GT_U8)inFields[18];
    patternData.ruleExtNotIpv6.commonExt.l4Byte3 = (GT_U8)inFields[19];
    patternData.ruleExtNotIpv6.commonExt.l4Byte13 = (GT_U8)inFields[20];
    patternData.ruleExtNotIpv6.commonExt.ipHeaderOk = (GT_U8)inFields[21];
    galtisIpAddr(&patternData.ruleExtNotIpv6.sip, (GT_U8*)inFields[22]);
    galtisIpAddr(&patternData.ruleExtNotIpv6.dip, (GT_U8*)inFields[23]);
    patternData.ruleExtNotIpv6.etherType = (GT_U16)inFields[24];
    patternData.ruleExtNotIpv6.l2Encap = (GT_U8)inFields[25];
    galtisMacAddr(&patternData.ruleExtNotIpv6.macDa, (GT_U8*)inFields[26]);
    galtisMacAddr(&patternData.ruleExtNotIpv6.macSa, (GT_U8*)inFields[27]);
    patternData.ruleExtNotIpv6.ipv4Fragmented = (GT_BOOL)inFields[28];
    galtisBArray(&patternBArr,(GT_U8*)inFields[29]);

    cmdOsMemCpy(
            patternData.ruleExtNotIpv6.udb0_5, patternBArr.data,
            patternBArr.length);

    if( GT_TRUE == sip5Pcl )
    {
        patternData.ruleExtNotIpv6.common.sourceDevice = (GT_U16)inFields[30];
        patternData.ruleExtNotIpv6.tag1Exist = (GT_U8)inFields[31];
        patternData.ruleExtNotIpv6.vid1 = (GT_U16)inFields[32];
        patternData.ruleExtNotIpv6.cfi1 = (GT_U8)inFields[33];
        patternData.ruleExtNotIpv6.up1 = (GT_U8)inFields[34];
        patternData.ruleExtNotIpv6.vrfId = (GT_U16)inFields[35];
        patternData.ruleExtNotIpv6.trunkHash = (GT_U8)inFields[36];
        galtisBArray(&patternBArr,(GT_U8*)inFields[37]);

        cmdOsMemCpy(
            patternData.ruleExtNotIpv6.udb39_46, patternBArr.data,
            patternBArr.length);
    }

    pattern_set = GT_TRUE;
    }

    if ((mask_set == GT_TRUE) && (pattern_set == GT_TRUE))
    {
        PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleExtNotIpv6.common.sourcePort);
    }
    return CMD_OK;
}
/**********************************************************************************/

static CMD_STATUS wrCpssDxCh3PclIngressRule_EXT_IPV6_L2_Write
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_BYTE_ARRY                       maskBArr, patternBArr;
    GT_U8           devNum;
    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* map input arguments to locals */
    /*
       inFields[1] = mask/pattern indication
       mask = 0
       pattern = 1
    */
    if(inFields[1] == 0) /* mask */
    {
    mask_ruleIndex = (GT_U32)inFields[0];

    cmdOsMemCpy(
        &maskData.ruleExtIpv6L2.common.portListBmp,
        &ruleSet_portListBmpMask,
        sizeof(CPSS_PORTS_BMP_STC));

    maskData.ruleExtIpv6L2.common.pclId = (GT_U16)inFields[2];
    maskData.ruleExtIpv6L2.common.macToMe=(GT_U8)inFields[3];
    maskData.ruleExtIpv6L2.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[4];
    maskData.ruleExtIpv6L2.common.isTagged = (GT_U8)inFields[5];
    maskData.ruleExtIpv6L2.common.vid = (GT_U16)inFields[6];
    maskData.ruleExtIpv6L2.common.up = (GT_U8)inFields[7];
    maskData.ruleExtIpv6L2.common.qosProfile = (GT_U8)inFields[8];
    maskData.ruleExtIpv6L2.common.isIp = (GT_U8)inFields[9];
    maskData.ruleExtIpv6L2.common.isL2Valid = (GT_U8)inFields[10];
    maskData.ruleExtIpv6L2.common.isUdbValid = (GT_U8)inFields[11];
    maskData.ruleExtIpv6L2.commonExt.isIpv6 = (GT_U8)inFields[12];
    maskData.ruleExtIpv6L2.commonExt.ipProtocol = (GT_U8)inFields[13];
    maskData.ruleExtIpv6L2.commonExt.dscp = (GT_U8)inFields[14];
    maskData.ruleExtIpv6L2.commonExt.isL4Valid = (GT_U8)inFields[15];
    maskData.ruleExtIpv6L2.commonExt.l4Byte0 = (GT_U8)inFields[16];
    maskData.ruleExtIpv6L2.commonExt.l4Byte1 = (GT_U8)inFields[17];
    maskData.ruleExtIpv6L2.commonExt.l4Byte2 = (GT_U8)inFields[18];
    maskData.ruleExtIpv6L2.commonExt.l4Byte3 = (GT_U8)inFields[19];
    maskData.ruleExtIpv6L2.commonExt.l4Byte13 = (GT_U8)inFields[20];
    maskData.ruleExtIpv6L2.commonExt.ipHeaderOk = (GT_U8)inFields[21];
    galtisIpv6Addr(&maskData.ruleExtIpv6L2.sip, (GT_U8*)inFields[22]);
    maskData.ruleExtIpv6L2.dipBits127to120 = (GT_U8)inFields[23];
    maskData.ruleExtIpv6L2.isIpv6ExtHdrExist = (GT_BOOL)inFields[24];
    maskData.ruleExtIpv6L2.isIpv6HopByHop = (GT_BOOL)inFields[25];
    galtisMacAddr(&maskData.ruleExtIpv6L2.macDa, (GT_U8*)inFields[26]);
    galtisMacAddr(&maskData.ruleExtIpv6L2.macSa, (GT_U8*)inFields[27]);
    galtisBArray(&maskBArr,(GT_U8*)inFields[28]);

    cmdOsMemCpy(
            maskData.ruleExtIpv6L2.udb6_11, maskBArr.data,
            maskBArr.length);

    if( GT_TRUE == sip5Pcl )
    {
        maskData.ruleExtIpv6L2.common.sourceDevice = (GT_U16)inFields[29];
        maskData.ruleExtIpv6L2.tag1Exist = (GT_U8)inFields[30];
        maskData.ruleExtIpv6L2.vid1 = (GT_U16)inFields[31];
        maskData.ruleExtIpv6L2.up1 = (GT_U8)inFields[32];
        maskData.ruleExtIpv6L2.vrfId = (GT_U16)inFields[33];
        maskData.ruleExtIpv6L2.trunkHash = (GT_U8)inFields[34];
        galtisBArray(&maskBArr,(GT_U8*)inFields[35]);

        cmdOsMemCpy(
            maskData.ruleExtIpv6L2.udb47_49, maskBArr.data,
            maskBArr.length);

        galtisBArray(&maskBArr,(GT_U8*)inFields[36]);

        cmdOsMemCpy(
            maskData.ruleExtIpv6L2.udb0_4, maskBArr.data,
            maskBArr.length);
    }

    mask_set = GT_TRUE;
    }
    else /* pattern */
    {
    pattern_ruleIndex = (GT_U32)inFields[0];

    cmdOsMemCpy(
        &patternData.ruleExtIpv6L2.common.portListBmp,
        &ruleSet_portListBmpPattern,
        sizeof(CPSS_PORTS_BMP_STC));

    patternData.ruleExtIpv6L2.common.pclId = (GT_U16)inFields[2];
    patternData.ruleExtIpv6L2.common.macToMe=(GT_U8)inFields[3];
    patternData.ruleExtIpv6L2.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[4];
    patternData.ruleExtIpv6L2.common.isTagged = (GT_U8)inFields[5];
    patternData.ruleExtIpv6L2.common.vid = (GT_U16)inFields[6];
    patternData.ruleExtIpv6L2.common.up = (GT_U8)inFields[7];
    patternData.ruleExtIpv6L2.common.qosProfile = (GT_U8)inFields[8];
    patternData.ruleExtIpv6L2.common.isIp = (GT_U8)inFields[9];
    patternData.ruleExtIpv6L2.common.isL2Valid = (GT_U8)inFields[10];
    patternData.ruleExtIpv6L2.common.isUdbValid = (GT_U8)inFields[11];
    patternData.ruleExtIpv6L2.commonExt.isIpv6 = (GT_U8)inFields[12];
    patternData.ruleExtIpv6L2.commonExt.ipProtocol = (GT_U8)inFields[13];
    patternData.ruleExtIpv6L2.commonExt.dscp = (GT_U8)inFields[14];
    patternData.ruleExtIpv6L2.commonExt.isL4Valid = (GT_U8)inFields[15];
    patternData.ruleExtIpv6L2.commonExt.l4Byte0 = (GT_U8)inFields[16];
    patternData.ruleExtIpv6L2.commonExt.l4Byte1 = (GT_U8)inFields[17];
    patternData.ruleExtIpv6L2.commonExt.l4Byte2 = (GT_U8)inFields[18];
    patternData.ruleExtIpv6L2.commonExt.l4Byte3 = (GT_U8)inFields[19];
    patternData.ruleExtIpv6L2.commonExt.l4Byte13 = (GT_U8)inFields[20];
    patternData.ruleExtIpv6L2.commonExt.ipHeaderOk = (GT_U8)inFields[21];
    galtisIpv6Addr(&patternData.ruleExtIpv6L2.sip, (GT_U8*)inFields[22]);
    patternData.ruleExtIpv6L2.dipBits127to120 = (GT_U8)inFields[23];
    patternData.ruleExtIpv6L2.isIpv6ExtHdrExist = (GT_BOOL)inFields[24];
    patternData.ruleExtIpv6L2.isIpv6HopByHop = (GT_BOOL)inFields[25];
    galtisMacAddr(&patternData.ruleExtIpv6L2.macDa, (GT_U8*)inFields[26]);
    galtisMacAddr(&patternData.ruleExtIpv6L2.macSa, (GT_U8*)inFields[27]);
    galtisBArray(&patternBArr,(GT_U8*)inFields[28]);

    cmdOsMemCpy(
            patternData.ruleExtIpv6L2.udb6_11, patternBArr.data,
            patternBArr.length);

    if( GT_TRUE == sip5Pcl )
    {
        patternData.ruleExtIpv6L2.common.sourceDevice = (GT_U16)inFields[29];
        patternData.ruleExtIpv6L2.tag1Exist = (GT_U8)inFields[30];
        patternData.ruleExtIpv6L2.vid1 = (GT_U16)inFields[31];
        patternData.ruleExtIpv6L2.up1 = (GT_U8)inFields[32];
        patternData.ruleExtIpv6L2.vrfId = (GT_U16)inFields[33];
        patternData.ruleExtIpv6L2.trunkHash = (GT_U8)inFields[34];
        galtisBArray(&patternBArr,(GT_U8*)inFields[35]);

        cmdOsMemCpy(
            patternData.ruleExtIpv6L2.udb47_49, patternBArr.data,
            patternBArr.length);

        galtisBArray(&patternBArr,(GT_U8*)inFields[36]);

        cmdOsMemCpy(
            patternData.ruleExtIpv6L2.udb0_4, patternBArr.data,
            patternBArr.length);
    }

    pattern_set = GT_TRUE;
    }

    if ((mask_set == GT_TRUE) && (pattern_set == GT_TRUE))
    {
        PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleExtIpv6L2.common.sourcePort);
    }
    return CMD_OK;
}
/**********************************************************************************/

static CMD_STATUS wrCpssDxCh3PclIngressRule_EXT_IPV6_L4_Write
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_BYTE_ARRY                       maskBArr, patternBArr;
    GT_U8           devNum;
    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* map input arguments to locals */
    /*
       inFields[1] = mask/pattern indication
       mask = 0
       pattern = 1
    */
    if(inFields[1] == 0) /* mask */
    {
    mask_ruleIndex = (GT_U32)inFields[0];

    cmdOsMemCpy(
        &maskData.ruleExtIpv6L4.common.portListBmp,
        &ruleSet_portListBmpMask,
        sizeof(CPSS_PORTS_BMP_STC));

    maskData.ruleExtIpv6L4.common.pclId = (GT_U16)inFields[2];
    maskData.ruleExtIpv6L4.common.macToMe=(GT_U8)inFields[3];
    maskData.ruleExtIpv6L4.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[4];
    maskData.ruleExtIpv6L4.common.isTagged = (GT_U8)inFields[5];
    maskData.ruleExtIpv6L4.common.vid = (GT_U16)inFields[6];
    maskData.ruleExtIpv6L4.common.up = (GT_U8)inFields[7];
    maskData.ruleExtIpv6L4.common.qosProfile = (GT_U8)inFields[8];
    maskData.ruleExtIpv6L4.common.isIp = (GT_U8)inFields[9];
    maskData.ruleExtIpv6L4.common.isL2Valid = (GT_U8)inFields[10];
    maskData.ruleExtIpv6L4.common.isUdbValid = (GT_U8)inFields[11];
    maskData.ruleExtIpv6L4.commonExt.isIpv6 = (GT_U8)inFields[12];
    maskData.ruleExtIpv6L4.commonExt.ipProtocol = (GT_U8)inFields[13];
    maskData.ruleExtIpv6L4.commonExt.dscp = (GT_U8)inFields[14];
    maskData.ruleExtIpv6L4.commonExt.isL4Valid = (GT_U8)inFields[15];
    maskData.ruleExtIpv6L4.commonExt.l4Byte0 = (GT_U8)inFields[16];
    maskData.ruleExtIpv6L4.commonExt.l4Byte1 = (GT_U8)inFields[17];
    maskData.ruleExtIpv6L4.commonExt.l4Byte2 = (GT_U8)inFields[18];
    maskData.ruleExtIpv6L4.commonExt.l4Byte3 = (GT_U8)inFields[19];
    maskData.ruleExtIpv6L4.commonExt.l4Byte13 = (GT_U8)inFields[20];
    maskData.ruleExtIpv6L4.commonExt.ipHeaderOk = (GT_U8)inFields[21];
    galtisIpv6Addr(&maskData.ruleExtIpv6L4.sip, (GT_U8*)inFields[22]);
    galtisIpv6Addr(&maskData.ruleExtIpv6L4.dip, (GT_U8*)inFields[23]);
    maskData.ruleExtIpv6L4.isIpv6ExtHdrExist = (GT_U8)inFields[24];
    maskData.ruleExtIpv6L4.isIpv6HopByHop = (GT_U8)inFields[25];
    galtisBArray(&maskBArr,(GT_U8*)inFields[26]);

    cmdOsMemCpy(
            maskData.ruleExtIpv6L4.udb12_14, maskBArr.data,
            maskBArr.length);

    if( GT_TRUE == sip5Pcl )
    {
        maskData.ruleExtIpv6L4.common.sourceDevice = (GT_U16)inFields[27];
        maskData.ruleExtIpv6L4.tag1Exist = (GT_U8)inFields[28];
        maskData.ruleExtIpv6L4.vid1 = (GT_U16)inFields[29];
        maskData.ruleExtIpv6L4.up1 = (GT_U8)inFields[30];
        maskData.ruleExtIpv6L4.vrfId = (GT_U16)inFields[31];
        maskData.ruleExtIpv6L4.trunkHash = (GT_U8)inFields[32];
        galtisBArray(&maskBArr,(GT_U8*)inFields[33]);

        cmdOsMemCpy(
            maskData.ruleExtIpv6L4.udb15_22, maskBArr.data,
            maskBArr.length);
    }

    mask_set = GT_TRUE;
    }
    else /* pattern */
    {
    pattern_ruleIndex = (GT_U32)inFields[0];

    cmdOsMemCpy(
        &patternData.ruleExtIpv6L4.common.portListBmp,
        &ruleSet_portListBmpPattern,
        sizeof(CPSS_PORTS_BMP_STC));

    patternData.ruleExtIpv6L4.common.pclId = (GT_U16)inFields[2];
    patternData.ruleExtIpv6L4.common.macToMe=(GT_U8)inFields[3];
    patternData.ruleExtIpv6L4.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[4];
    patternData.ruleExtIpv6L4.common.isTagged = (GT_U8)inFields[5];
    patternData.ruleExtIpv6L4.common.vid = (GT_U16)inFields[6];
    patternData.ruleExtIpv6L4.common.up = (GT_U8)inFields[7];
    patternData.ruleExtIpv6L4.common.qosProfile = (GT_U8)inFields[8];
    patternData.ruleExtIpv6L4.common.isIp = (GT_U8)inFields[9];
    patternData.ruleExtIpv6L4.common.isL2Valid = (GT_U8)inFields[10];
    patternData.ruleExtIpv6L4.common.isUdbValid = (GT_U8)inFields[11];
    patternData.ruleExtIpv6L4.commonExt.isIpv6 = (GT_U8)inFields[12];
    patternData.ruleExtIpv6L4.commonExt.ipProtocol = (GT_U8)inFields[13];
    patternData.ruleExtIpv6L4.commonExt.dscp = (GT_U8)inFields[14];
    patternData.ruleExtIpv6L4.commonExt.isL4Valid = (GT_U8)inFields[15];
    patternData.ruleExtIpv6L4.commonExt.l4Byte0 = (GT_U8)inFields[16];
    patternData.ruleExtIpv6L4.commonExt.l4Byte1 = (GT_U8)inFields[17];
    patternData.ruleExtIpv6L4.commonExt.l4Byte2 = (GT_U8)inFields[18];
    patternData.ruleExtIpv6L4.commonExt.l4Byte3 = (GT_U8)inFields[19];
    patternData.ruleExtIpv6L4.commonExt.l4Byte13 = (GT_U8)inFields[20];
    patternData.ruleExtIpv6L4.commonExt.ipHeaderOk = (GT_U8)inFields[21];
    galtisIpv6Addr(&patternData.ruleExtIpv6L4.sip, (GT_U8*)inFields[22]);
    galtisIpv6Addr(&patternData.ruleExtIpv6L4.dip, (GT_U8*)inFields[23]);
    patternData.ruleExtIpv6L4.isIpv6ExtHdrExist = (GT_U8)inFields[24];
    patternData.ruleExtIpv6L4.isIpv6HopByHop = (GT_U8)inFields[25];
    galtisBArray(&patternBArr,(GT_U8*)inFields[26]);


    cmdOsMemCpy(
            patternData.ruleExtIpv6L4.udb12_14, patternBArr.data,
            patternBArr.length);

    if( GT_TRUE == sip5Pcl )
    {
        patternData.ruleExtIpv6L4.common.sourceDevice = (GT_U16)inFields[27];
        patternData.ruleExtIpv6L4.tag1Exist = (GT_U8)inFields[28];
        patternData.ruleExtIpv6L4.vid1 = (GT_U16)inFields[29];
        patternData.ruleExtIpv6L4.up1 = (GT_U8)inFields[30];
        patternData.ruleExtIpv6L4.vrfId = (GT_U16)inFields[31];
        patternData.ruleExtIpv6L4.trunkHash = (GT_U8)inFields[32];
        galtisBArray(&patternBArr,(GT_U8*)inFields[33]);

        cmdOsMemCpy(
            patternData.ruleExtIpv6L4.udb15_22, patternBArr.data,
            patternBArr.length);
    }

    pattern_set = GT_TRUE;
    }

    if ((mask_set == GT_TRUE) && (pattern_set == GT_TRUE))
    {
        PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleExtIpv6L4.common.sourcePort);
    }
    return CMD_OK;
}
/*********************************************************************************/

static CMD_STATUS  wrCpssDxCh3PclIngressRuleWriteFirst

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&maskData , 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
    cpssOsMemSet(&patternData, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));


    mask_set    = GT_FALSE;
    pattern_set = GT_FALSE;

    switch (inArgs[1])
    {
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
            wrCpssDxCh3PclIngressRule_STD_NOT_IP_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
            wrCpssDxCh3PclIngressRule_STD_IP_L2_QOS_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
            wrCpssDxCh3PclIngressRule_STD_IPV4_L4_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E:
            wrCpssDxCh3PclIngressRule_STD_IPV6_DIP_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
            wrCpssDxCh3PclIngressRule_EXT_NOT_IPV6_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
            wrCpssDxCh3PclIngressRule_EXT_IPV6_L2_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
            wrCpssDxCh3PclIngressRule_EXT_IPV6_L4_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        default:
            galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong PCL RULE FORMAT.\n");
            return CMD_AGENT_ERROR;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}
/*************************************************************************/
static CMD_STATUS wrCpssDxCh3PclIngressRuleWriteNext

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result = GT_OK;
    GT_U8           devNum;
    CPSS_DXCH_PCL_ACTION_STC         *actionPtr;
    GT_U32                              actionIndex;
    GT_U32                              ruleIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;


    /* Check Validity */
    if(inFields[1] > 1)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong value for data_type (should be MASK or PATTERN).\n");
        return CMD_AGENT_ERROR;
    }


    if(inFields[1] == 0 && mask_set)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Must be subsequent pair of Ingress {Rule,Mask}.\n");
        return CMD_AGENT_ERROR;
    }

    if(inFields[1] == 1 && pattern_set)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Must be subsequent pair of Ingress {Rule,Mask}.\n");
        return CMD_AGENT_ERROR;
    }



    /*
       inFields[1] = mask/pattern indication
       mask = 0
       pattern = 1
    */
    switch (inArgs[1])
    {
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
            wrCpssDxCh3PclIngressRule_STD_NOT_IP_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
            wrCpssDxCh3PclIngressRule_STD_IP_L2_QOS_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
            wrCpssDxCh3PclIngressRule_STD_IPV4_L4_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E:
            wrCpssDxCh3PclIngressRule_STD_IPV6_DIP_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
            wrCpssDxCh3PclIngressRule_EXT_NOT_IPV6_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
            wrCpssDxCh3PclIngressRule_EXT_IPV6_L2_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
            wrCpssDxCh3PclIngressRule_EXT_IPV6_L4_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        default:
            galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong PCL RULE FORMAT.\n");
            return CMD_AGENT_ERROR;
    }
    if(mask_set && pattern_set && (mask_ruleIndex != pattern_ruleIndex))
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : mask ruleIndex doesn't match pattern ruleIndex.\n");
        return CMD_AGENT_ERROR;
    }

    /* Get the action */
    ruleIndex = (GT_U32)inFields[0];
    pclDxCh3ActionGet( ruleIndex, &actionIndex);

    if(actionIndex == INVALID_RULE_INDEX || !(actionTable[actionIndex].valid))
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Action table entry is not set.\n");
        return CMD_AGENT_ERROR;

    }

    if(mask_set && pattern_set)
    {
         /* map input arguments to locals */
        devNum = (GT_U8)inArgs[0];

        actionPtr = &(actionTable[actionIndex].actionEntry);
        lastActionPtr = actionPtr;

        /* call cpss api function */
        result = pg_wrap_cpssDxChPclRuleSet(devNum, ruleFormat, ruleIndex, &maskData,
                                                    &patternData, actionPtr);

        mask_set = GT_FALSE;
        pattern_set = GT_FALSE;

        /* don't clear pattern and mask - could be reused */

    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*************************************************************************/
static CMD_STATUS wrCpssDxCh3PclIngressRuleWriteEnd

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
  if(mask_set || pattern_set)
      /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_BAD_STATE, "");
    else
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

/*********************************************************************************/
static GT_VOID  wrCpssDxCh3PclIngressRuleReadParseFields
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  CPSS_DXCH_PCL_RULE_FORMAT_UNT *ruleDataPtr,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC          *commonPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC   *commonStdIpPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC      *commonExtPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC      *stdNotIpPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC   *stdIpL2QosPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_L4_STC     *stdIpv4L4Ptr;
    CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV6_DIP_STC    *stdIpv6DipPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC    *extNotIpv6Ptr;
    CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L2_STC     *extIpv6L2Ptr;
    CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L4_STC     *extIpv6L4Ptr;

    commonPtr       = NULL;
    commonStdIpPtr  = NULL;
    commonExtPtr    = NULL;
    stdNotIpPtr     = NULL;
    stdIpL2QosPtr   = NULL;
    stdIpv4L4Ptr    = NULL;
    stdIpv6DipPtr   = NULL;
    extNotIpv6Ptr   = NULL;
    extIpv6L2Ptr    = NULL;
    extIpv6L4Ptr    = NULL;

    switch (utilCpssDxChPclRuleParsedGet_RuleFormat)
    {
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
            commonPtr      = &(ruleDataPtr->ruleStdNotIp.common);
            stdNotIpPtr    = &(ruleDataPtr->ruleStdNotIp);
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
            commonPtr      = &(ruleDataPtr->ruleStdIpL2Qos.common);
            commonStdIpPtr = &(ruleDataPtr->ruleStdIpL2Qos.commonStdIp);
            stdIpL2QosPtr  = &(ruleDataPtr->ruleStdIpL2Qos);
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
            commonPtr      = &(ruleDataPtr->ruleStdIpv4L4.common);
            commonStdIpPtr = &(ruleDataPtr->ruleStdIpv4L4.commonStdIp);
            stdIpv4L4Ptr   = &(ruleDataPtr->ruleStdIpv4L4);
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E:
            commonPtr      = &(ruleDataPtr->ruleStdIpv6Dip.common);
            commonStdIpPtr = &(ruleDataPtr->ruleStdIpv6Dip.commonStdIp);
            stdIpv6DipPtr  = &(ruleDataPtr->ruleStdIpv6Dip);
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
            commonPtr     = &(ruleDataPtr->ruleExtNotIpv6.common);
            commonExtPtr  = &(ruleDataPtr->ruleExtNotIpv6.commonExt);
            extNotIpv6Ptr = &(ruleDataPtr->ruleExtNotIpv6);
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
            commonPtr    = &(ruleDataPtr->ruleExtIpv6L2.common);
            commonExtPtr = &(ruleDataPtr->ruleExtIpv6L2.commonExt);
            extIpv6L2Ptr = &(ruleDataPtr->ruleExtIpv6L2);
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
            commonPtr    = &(ruleDataPtr->ruleExtIpv6L4.common);
            commonExtPtr = &(ruleDataPtr->ruleExtIpv6L4.commonExt);
            extIpv6L4Ptr = &(ruleDataPtr->ruleExtIpv6L4);
            break;

        default:
            galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong PCL RULE FORMAT.\n");
            break;
    }

    if (commonPtr != NULL)
    {
        fieldOutput(
            "%d%d%d%d%d%d%d%d%d%d",
            commonPtr->pclId,      commonPtr->macToMe,
            commonPtr->sourcePort, commonPtr->isTagged,
            commonPtr->vid,        commonPtr->up,
            commonPtr->qosProfile, commonPtr->isIp,
            commonPtr->isL2Valid,  commonPtr->isUdbValid);
         fieldOutputSetAppendMode();
    }

    if (commonStdIpPtr != NULL)
    {
        fieldOutput(
            "%d%d%d%d%d%d%d%d",
            commonStdIpPtr->isIpv4,       commonStdIpPtr->ipProtocol,
            commonStdIpPtr->dscp,         commonStdIpPtr->isL4Valid,
            commonStdIpPtr->l4Byte2,      commonStdIpPtr->l4Byte3,
            commonStdIpPtr->ipHeaderOk,   commonStdIpPtr->ipv4Fragmented);
         fieldOutputSetAppendMode();
    }

    if (commonExtPtr != NULL)
    {
        fieldOutput(
            "%d%d%d%d%d%d%d%d%d%d",
            commonExtPtr->isIpv6,       commonExtPtr->ipProtocol,
            commonExtPtr->dscp,         commonExtPtr->isL4Valid,
            commonExtPtr->l4Byte0,      commonExtPtr->l4Byte1,
            commonExtPtr->l4Byte2,      commonExtPtr->l4Byte3,
            commonExtPtr->l4Byte13,     commonExtPtr->ipHeaderOk);
         fieldOutputSetAppendMode();
    }

    if (stdNotIpPtr != NULL)
    {
        fieldOutput(
            "%d%d%d%d%s%s%s",
            stdNotIpPtr->isIpv4,       stdNotIpPtr->etherType,
            stdNotIpPtr->isArp,         stdNotIpPtr->l2Encap,
            galtisByteArrayToString(stdNotIpPtr->macDa.arEther, 6),
            galtisByteArrayToString(stdNotIpPtr->macSa.arEther, 6),
            galtisByteArrayToString(stdNotIpPtr->udb15_17, 3));
    }

    if (stdIpL2QosPtr != NULL)
    {
        fieldOutput(
            "%d%d%d%s%s%s",
            stdIpL2QosPtr->isArp, stdIpL2QosPtr->isIpv6ExtHdrExist,
            stdIpL2QosPtr->isIpv6HopByHop,
            galtisByteArrayToString(stdIpL2QosPtr->macDa.arEther, 6),
            galtisByteArrayToString(stdIpL2QosPtr->macSa.arEther, 6),
            galtisByteArrayToString(stdIpL2QosPtr->udb18_19, 2));
    }

    if (stdIpv4L4Ptr != NULL)
    {
        fieldOutput(
            "%d%d%s%s%d%d%d%s",
            stdIpv4L4Ptr->isArp, stdIpv4L4Ptr->isBc,
            galtisByteArrayToString(stdIpv4L4Ptr->sip.arIP, 4),
            galtisByteArrayToString(stdIpv4L4Ptr->dip.arIP, 4),
            stdIpv4L4Ptr->l4Byte0, stdIpv4L4Ptr->l4Byte1,
            stdIpv4L4Ptr->l4Byte13,
            galtisByteArrayToString(stdIpv4L4Ptr->udb20_22, 3));
    }

    if (stdIpv6DipPtr != NULL)
    {
        fieldOutput(
            "%d%d%d%s",
            stdIpv6DipPtr->isArp, stdIpv6DipPtr->isIpv6ExtHdrExist,
            stdIpv6DipPtr->isIpv6HopByHop,
            galtisByteArrayToString(stdIpv6DipPtr->dip.arIP, 16));
    }

    if (extNotIpv6Ptr != NULL)
    {
        fieldOutput(
            "%s%s%d%d%s%s%d%s",
            galtisByteArrayToString(extNotIpv6Ptr->sip.arIP, 4),
            galtisByteArrayToString(extNotIpv6Ptr->dip.arIP, 4),
            extNotIpv6Ptr->etherType, extNotIpv6Ptr->l2Encap,
            galtisByteArrayToString(extNotIpv6Ptr->macDa.arEther, 6),
            galtisByteArrayToString(extNotIpv6Ptr->macSa.arEther, 6),
            extNotIpv6Ptr->ipv4Fragmented,
            galtisByteArrayToString(extNotIpv6Ptr->udb0_5, 6));
    }

    if (extIpv6L2Ptr != NULL)
    {
        fieldOutput(
            "%s%d%d%d%s%s%s",
            galtisByteArrayToString(extIpv6L2Ptr->sip.arIP, 16),
            extIpv6L2Ptr->dipBits127to120, extIpv6L2Ptr->isIpv6ExtHdrExist,
            extIpv6L2Ptr->isIpv6HopByHop,
            galtisByteArrayToString(extIpv6L2Ptr->macDa.arEther, 6),
            galtisByteArrayToString(extIpv6L2Ptr->macSa.arEther, 6),
            galtisByteArrayToString(extIpv6L2Ptr->udb6_11, 6));
    }

    if (extIpv6L4Ptr != NULL)
    {
        fieldOutput(
            "%s%s%d%d%s",
            galtisByteArrayToString(extIpv6L4Ptr->sip.arIP, 16),
            galtisByteArrayToString(extIpv6L4Ptr->dip.arIP, 16),
            extIpv6L4Ptr->isIpv6ExtHdrExist, extIpv6L4Ptr->isIpv6HopByHop,
            galtisByteArrayToString(extIpv6L4Ptr->udb12_14, 3));
    }
}

static CMD_STATUS  wrCpssDxCh3PclIngressRuleReadParse
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32                        rc;
    GT_BOOL                       isRuleValid;
    GT_U32                        dataType;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT *ruleDataPtr;

    if (utilCpssDxChPclRuleParsedGet_LinesAmount == 0)
    {
        /* end of required rules */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    if ((utilCpssDxChPclRuleParsedGet_LinesAmount & 1) == 0)
    {
        /* even line: - new rule */
        rc = pg_wrap_cpssDxChPclRuleParsedGet(
            (GT_U8)inArgs[0] /*devNum*/,
            utilCpssDxChPclRuleParsedGet_RuleFormat,
            utilCpssDxChPclRuleParsedGet_RuleIndex,
            0 /*ruleOptionsBmp*/,
            &isRuleValid, &maskData, &patternData, &actionData);
        if (rc != GT_OK)
        {
            galtisOutput(outArgs, rc, "%d", -1);
            return CMD_OK;
        }
        /* output mask */
        ruleDataPtr = &maskData;
        dataType = 0;
    }
    else
    {
        /* odd line: output pattern */
        ruleDataPtr = &patternData;
        dataType = 1;
    }

    /* Tab-number, ruleIndex and dataType(mask/pattern) before fields */
    fieldOutput(
        "%d%d",
        utilCpssDxChPclRuleParsedGet_RuleIndex, dataType);
    fieldOutputSetAppendMode();

    /* output rule fields to internal buffer */
    wrCpssDxCh3PclIngressRuleReadParseFields(
        inArgs, inFields, ruleDataPtr, outArgs);

    /* output all to GUI */
    galtisOutput(outArgs, GT_OK, "%d%f",
        (utilCpssDxChPclRuleParsedGet_RuleFormat
            - CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E)
        );

    if ((utilCpssDxChPclRuleParsedGet_LinesAmount & 1) != 0)
    {
        /*  odd line: increment rule index */
        utilCpssDxChPclRuleParsedGet_RuleIndex ++;
    }

    /* decrement amount of not outputed lines */
    utilCpssDxChPclRuleParsedGet_LinesAmount --;

    return CMD_OK;
}

/*********************************************************************************/
static CMD_STATUS  wrCpssDxCh3PclIngressRuleReadFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if ((utilCpssDxChPclRuleParsedGet_RuleFormat
        < CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E)
        || (utilCpssDxChPclRuleParsedGet_RuleFormat
        > CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E))
    {
        /* rule format for another table */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_AGENT_ERROR;
    }

     return wrCpssDxCh3PclIngressRuleReadParse(
        inArgs, inFields, outArgs);
}

/*************************************************************************/
static CMD_STATUS wrCpssDxCh3PclIngressRuleReadNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    return wrCpssDxCh3PclIngressRuleReadParse(
       inArgs, inFields, outArgs);
}

/****************************Ingress UDB stylesd keys Table*******************/

/* returns reached field index */
static GT_U32 prvWrCpssDxChXCatPclUdbCommonSet
(
    IN  GT_UINTPTR                                            inFields[],
    IN  GT_32                                            startFieldIndex,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_COMMON_STC *comPtr
)
{
    GT_U32 i; /* field index */

    i = startFieldIndex;

    comPtr->pclId           = (GT_U16) inFields[i++];
    comPtr->macToMe         = (GT_U8)  inFields[i++];
    comPtr->sourcePort      = (GT_PHYSICAL_PORT_NUM)  inFields[i++];
    comPtr->vid             = (GT_U16) inFields[i++];
    comPtr->up              = (GT_U8)  inFields[i++];
    comPtr->isIp            = (GT_U8)  inFields[i++];
    comPtr->dscpOrExp       = (GT_U8)  inFields[i++];
    comPtr->isL2Valid       = (GT_U8)  inFields[i++];
    comPtr->isUdbValid      = (GT_U8)  inFields[i++];
    comPtr->pktTagging      = (GT_U8)  inFields[i++];
    comPtr->l3OffsetInvalid = (GT_U8)  inFields[i++];
    comPtr->l4ProtocolType  = (GT_U8)  inFields[i++];
    comPtr->pktType         = (GT_U8)  inFields[i++];
    comPtr->ipHeaderOk      = (GT_U8)  inFields[i++];
    comPtr->macDaType       = (GT_U8)  inFields[i++];
    comPtr->l4OffsetInvalid = (GT_U8)  inFields[i++];
    comPtr->l2Encapsulation = (GT_U8)  inFields[i++];
    comPtr->isIpv6Eh        = (GT_U8)  inFields[i++];
    comPtr->isIpv6HopByHop  = (GT_U8)  inFields[i++];

    /* set port list */
    cmdOsMemCpy(
        &(comPtr->portListBmp),
        ((inFields[1] == 0)
            ? &ruleSet_portListBmpMask : &ruleSet_portListBmpPattern),
        sizeof(CPSS_PORTS_BMP_STC));

    return i;
}

/* returns reached field index */
static GT_U32 prvWrCpssDxChXCatPclUdbStdSet
(
    IN  GT_UINTPTR                                              inFields[],
    IN  GT_32                                              startFieldIndex,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC *keyPtr
)
{
    GT_U32 i;           /* field index */
    GT_BYTE_ARRY bArr;  /* binary data */

    i = prvWrCpssDxChXCatPclUdbCommonSet(
        inFields,
        startFieldIndex,
        &(keyPtr->commonIngrUdb));

    keyPtr->isIpv4 = (GT_BOOL)inFields[i++];

    galtisBArray(&bArr,(GT_U8*)inFields[i++]);
    cmdOsMemCpy(keyPtr->udb0_15, bArr.data, bArr.length);
    galtisBArrayFree(&bArr);

    if( GT_TRUE == sip5Pcl )
    {
        keyPtr->commonIngrUdb.sourceDevice = (GT_U16) inFields[i++];
        keyPtr->qosProfile = (GT_U8) inFields[i++];
        keyPtr->vrfId = (GT_U16) inFields[i++];
        keyPtr->trunkHash = (GT_U8) inFields[i++];

        galtisBArray(&bArr,(GT_U8*)inFields[i++]);
        cmdOsMemCpy(keyPtr->udb35_38, bArr.data, bArr.length);
        galtisBArrayFree(&bArr);
    }

    return i;
}

/* returns reached field index */
static GT_U32 prvWrCpssDxChXCatPclUdbExtSet
(
    IN  GT_UINTPTR                                              inFields[],
    IN  GT_32                                              startFieldIndex,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXTENDED_UDB_STC *keyPtr
)
{
    GT_U32 i;           /* field index */
    GT_BYTE_ARRY bArr;  /* binary data */
    GT_ETHERADDR mac;   /* mac address */
    GT_IPADDR    ipAddr;/* IP address  */

    i = prvWrCpssDxChXCatPclUdbCommonSet(
        inFields,
        startFieldIndex,
        &(keyPtr->commonIngrUdb));

    keyPtr->isIpv6 = (GT_BOOL)inFields[i++];

    keyPtr->ipProtocol  = (GT_U8)  inFields[i++];

    galtisIpAddr(&ipAddr, (GT_U8*)inFields[i++]);
    cmdOsMemCpy(keyPtr->sipBits31_0, ipAddr.arIP, 4);

    galtisMacAddr(&mac,  (GT_U8*)inFields[i++]);
    cmdOsMemCpy(keyPtr->sipBits79_32orMacSa, mac.arEther, 6);

    galtisMacAddr(&mac, (GT_U8*)inFields[i++]);
    cmdOsMemCpy(keyPtr->sipBits127_80orMacDa, mac.arEther, 6);

    galtisIpAddr(&ipAddr, (GT_U8*)inFields[i++]);
    cmdOsMemCpy(keyPtr->dipBits127_112, ipAddr.arIP, 2);

    galtisIpAddr(&ipAddr, (GT_U8*)inFields[i++]);
    cmdOsMemCpy(keyPtr->dipBits31_0, ipAddr.arIP, 4);

    galtisBArray(&bArr,(GT_U8*)inFields[i++]);
    cmdOsMemCpy(keyPtr->udb0_15, bArr.data, bArr.length);
    galtisBArrayFree(&bArr);

    if( GT_TRUE == sip5Pcl )
    {
        keyPtr->commonIngrUdb.sourceDevice = (GT_U16) inFields[i++];
        keyPtr->qosProfile = (GT_U8) inFields[i++];
        keyPtr->vrfId = (GT_U16) inFields[i++];
        keyPtr->trunkHash = (GT_U8) inFields[i++];
        keyPtr->tag1Exist = (GT_U8) inFields[i++];
        keyPtr->vid1 = (GT_U16) inFields[i++];
        keyPtr->cfi1 = (GT_U8) inFields[i++];
        keyPtr->up1 = (GT_U8) inFields[i++];

        galtisBArray(&bArr,(GT_U8*)inFields[i++]);
        cmdOsMemCpy(keyPtr->udb23_30, bArr.data, bArr.length);
        galtisBArrayFree(&bArr);
    }

    return i;
}

static CMD_STATUS  wrCpssDxChPclXCatUDBRuleWriteNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_PCL_RULE_FORMAT_UNT *untPtr;
    GT_STATUS                     result = GT_OK;
    GT_U8                         devNum;
    CPSS_DXCH_PCL_ACTION_STC      *actionPtr;
    GT_U32                        actionIndex;
    GT_U32                        ruleIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    /*
       inFields[1] = mask/pattern indication
       mask = 0
       pattern = 1
    */
    if(inFields[1] == 0) /* mask */
    {
        if (mask_set != GT_FALSE)
        {
            galtisOutput(outArgs, GT_BAD_STATE, "");
            return CMD_OK;
        }
        mask_ruleIndex = (GT_U32)inFields[0];
        mask_set    = GT_TRUE;
        untPtr = &maskData;
    }
    else
    {
        if (pattern_set != GT_FALSE)
        {
            galtisOutput(outArgs, GT_BAD_STATE, "");
            return CMD_OK;
        }
        pattern_ruleIndex = (GT_U32)inFields[0];
        pattern_set = GT_TRUE;
        untPtr = &patternData;
    }

    switch (inArgs[1])
    {
        case 0:
            ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E;
            prvWrCpssDxChXCatPclUdbStdSet(
                inFields, 2, &(untPtr->ruleIngrStdUdb));

            if(inFields[1] == 0) /* mask */
            {
                CONVERT_PCL_MASK_PCL_ID_TEST_TO_CPSS_MAC(maskData.ruleIngrStdUdb.commonIngrUdb.pclId);
            }

            break;

        case 1:
            ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E;
            prvWrCpssDxChXCatPclUdbExtSet(
                inFields, 2, &(untPtr->ruleIngrExtUdb));
            break;

        default:
            galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong PCL RULE FORMAT.\n");
            return CMD_AGENT_ERROR;
    }

    if (mask_set && pattern_set)
    {
        if (mask_ruleIndex != pattern_ruleIndex)
        {
            galtisOutput(outArgs, GT_BAD_STATE, "");
            return CMD_OK;
        }

        ruleIndex = mask_ruleIndex;

         /* map input arguments to locals */
        devNum  = (GT_U8)inArgs[0];

        pclDxCh3ActionGet(ruleIndex, &actionIndex);

        if(actionIndex == INVALID_RULE_INDEX || !(actionTable[actionIndex].valid))
        {
            /* pack output arguments to galtis string */
            galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Action table entry is not set.\n");
            return CMD_AGENT_ERROR;

        }

        actionPtr = &(actionTable[actionIndex].actionEntry);
        lastActionPtr = actionPtr;

        /* call cpss api function */
        result = pg_wrap_cpssDxChPclRuleSet(
            devNum, ruleFormat, ruleIndex,
            &maskData, &patternData, actionPtr);

        mask_set = GT_FALSE;
        pattern_set = GT_FALSE;

    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

static CMD_STATUS  wrCpssDxChPclXCatUDBRuleWriteFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&maskData , 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
    cpssOsMemSet(&patternData, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));


    mask_set    = GT_FALSE;
    pattern_set = GT_FALSE;

    return wrCpssDxChPclXCatUDBRuleWriteNext(
        inArgs, inFields, numFields, outArgs);

}

/*********************************************************************************/
static GT_VOID  wrCpssDxChPclXCatUDBRuleReadParseFields
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  CPSS_DXCH_PCL_RULE_FORMAT_UNT *ruleDataPtr,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_COMMON_STC     *commonUdbPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC   *stdUdbPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXTENDED_UDB_STC   *extUdbPtr;

    commonUdbPtr  = NULL;
    stdUdbPtr     = NULL;
    extUdbPtr     = NULL;

    switch (utilCpssDxChPclRuleParsedGet_RuleFormat)
    {
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
            commonUdbPtr = &(ruleDataPtr->ruleIngrStdUdb.commonIngrUdb);
            stdUdbPtr    = &(ruleDataPtr->ruleIngrStdUdb);
            break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
            commonUdbPtr = &(ruleDataPtr->ruleIngrExtUdb.commonIngrUdb);
            extUdbPtr    = &(ruleDataPtr->ruleIngrExtUdb);
            break;

        default:
            galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong PCL RULE FORMAT.\n");
            break;
    }

    if (commonUdbPtr != NULL)
    {
        fieldOutput(
            "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
            commonUdbPtr->pclId,           commonUdbPtr->macToMe,
            commonUdbPtr->sourcePort,      commonUdbPtr->vid,
            commonUdbPtr->up,              commonUdbPtr->isIp,
            commonUdbPtr->dscpOrExp,       commonUdbPtr->isL2Valid,
            commonUdbPtr->isUdbValid,      commonUdbPtr->pktTagging,
            commonUdbPtr->l3OffsetInvalid, commonUdbPtr->l4ProtocolType,
            commonUdbPtr->pktType,         commonUdbPtr->ipHeaderOk,
            commonUdbPtr->macDaType,       commonUdbPtr->l4OffsetInvalid,
            commonUdbPtr->l2Encapsulation, commonUdbPtr->isIpv6Eh,
            commonUdbPtr->isIpv6HopByHop);
         fieldOutputSetAppendMode();
    }

    if (stdUdbPtr != NULL)
    {
        fieldOutput(
            "%d%s",
            stdUdbPtr->isIpv4,
            galtisByteArrayToString(stdUdbPtr->udb0_15, 16));
    }

    if (extUdbPtr != NULL)
    {
        fieldOutput(
            "%d%d%s%s%s%s%s%s",
            extUdbPtr->isIpv6, extUdbPtr->ipProtocol,
            galtisByteArrayToString(extUdbPtr->sipBits31_0, 4),
            galtisByteArrayToString(extUdbPtr->sipBits79_32orMacSa, 6),
            galtisByteArrayToString(extUdbPtr->sipBits127_80orMacDa, 6),
            galtisByteArrayToString(extUdbPtr->dipBits127_112, 2),
            galtisByteArrayToString(extUdbPtr->dipBits31_0, 4),
            galtisByteArrayToString(extUdbPtr->udb0_15, 16));
    }
}

static CMD_STATUS  wrCpssDxChPclXCatUDBRuleReadParse
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32                        rc;
    GT_BOOL                       isRuleValid;
    GT_U32                        dataType;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT *ruleDataPtr;

    if (utilCpssDxChPclRuleParsedGet_LinesAmount == 0)
    {
        /* end of required rules */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    if ((utilCpssDxChPclRuleParsedGet_LinesAmount & 1) == 0)
    {
        /* even line: - new rule */
        rc = pg_wrap_cpssDxChPclRuleParsedGet(
            (GT_U8)inArgs[0] /*devNum*/,
            utilCpssDxChPclRuleParsedGet_RuleFormat,
            utilCpssDxChPclRuleParsedGet_RuleIndex,
            0 /*ruleOptionsBmp*/,
            &isRuleValid, &maskData, &patternData, &actionData);
        if (rc != GT_OK)
        {
            galtisOutput(outArgs, rc, "%d", -1);
            return CMD_OK;
        }
        /* output mask */
        ruleDataPtr = &maskData;
        dataType = 0;
    }
    else
    {
        /* odd line: output pattern */
        ruleDataPtr = &patternData;
        dataType = 1;
    }

    /* Tab-number, ruleIndex and dataType(mask/pattern) before fields */
    fieldOutput(
        "%d%d",
        utilCpssDxChPclRuleParsedGet_RuleIndex, dataType);
    fieldOutputSetAppendMode();

    /* output rule fields to internal buffer */
    wrCpssDxChPclXCatUDBRuleReadParseFields(
        inArgs, inFields, ruleDataPtr, outArgs);

    /* output all to GUI */
    galtisOutput(outArgs, GT_OK, "%d%f",
        (utilCpssDxChPclRuleParsedGet_RuleFormat
            - CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E)
        );

    if ((utilCpssDxChPclRuleParsedGet_LinesAmount & 1) != 0)
    {
        /*  odd line: increment rule index */
        utilCpssDxChPclRuleParsedGet_RuleIndex ++;
    }

    /* decrement amount of not outputed lines */
    utilCpssDxChPclRuleParsedGet_LinesAmount --;

    return CMD_OK;
}

/*********************************************************************************/
static CMD_STATUS  wrCpssDxChPclXCatUDBRuleReadFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if ((utilCpssDxChPclRuleParsedGet_RuleFormat
        < CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E)
        || (utilCpssDxChPclRuleParsedGet_RuleFormat
        > CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E))
    {
        /* rule format for another table */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_AGENT_ERROR;
    }

     return wrCpssDxChPclXCatUDBRuleReadParse(
        inArgs, inFields, outArgs);
}

/*************************************************************************/
static CMD_STATUS wrCpssDxChPclXCatUDBRuleReadNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    return wrCpssDxChPclXCatUDBRuleReadParse(
       inArgs, inFields, outArgs);
}

/****************************Egress Table*************************************/

static void epclCommonEgrPktTypeInfoSet
(
    INOUT CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC *commonPtr,
    IN GT_UINTPTR   fieldsArray[]/* relevant fields are 10..20*/
)
{
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC *patternCommonPtr =
        &patternData.ruleEgrStdNotIp.common;/* all egress rules starts with the 'common'*/
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC *maskCommonPtr =
        &maskData.ruleEgrStdNotIp.common;/* all egress rules starts with the 'common'*/
    GT_TRUNK_ID trunkId;

    commonPtr->toCpu.cpuCode=(GT_U8)fieldsArray[10];
    commonPtr->toCpu.srcTrg=(GT_U8)fieldsArray[11];
    commonPtr->fromCpu.tc=(GT_U8)fieldsArray[12];
    commonPtr->fromCpu.dp=(GT_U8)fieldsArray[13];
    commonPtr->fromCpu.egrFilterEnable=(GT_U8)fieldsArray[14];
    commonPtr->toAnalyzer.rxSniff=(GT_U8)fieldsArray[15];
    commonPtr->fwdData.qosProfile=(GT_U8)fieldsArray[16];
    commonPtr->fwdData.srcTrunkId=(GT_U8)fieldsArray[17];
    commonPtr->fwdData.srcIsTrunk=(GT_U8)fieldsArray[18];
    commonPtr->fwdData.isUnknown=(GT_U8)fieldsArray[19];
    commonPtr->fwdData.isRouted=(GT_U8)fieldsArray[20];

    trunkId = commonPtr->fwdData.srcTrunkId;
        if(commonPtr == maskCommonPtr)
        {
            /* we do the mask now */
            gtPclMaskTrunkConvertFromTestToCpss(
            maskCommonPtr->fwdData.srcIsTrunk,
            patternCommonPtr->fwdData.srcIsTrunk,
                &trunkId);
        }
        else
        {
            /* we do the pattern now */
            gtPclPatternTrunkConvertFromTestToCpss(
            maskCommonPtr->fwdData.srcIsTrunk,
            patternCommonPtr->fwdData.srcIsTrunk,
                &trunkId);
        }

    commonPtr->fwdData.srcTrunkId = (GT_U8)trunkId;
}

static CMD_STATUS wrCpssDxCh3PclEgressRule_EGRESS_STD_NOT_IP_E
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_BYTE_ARRY    maskBArr, patternBArr;
    GT_U8           devNum;
    GT_U32   ii;

    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;


    /* map input arguments to locals */
    /*
       inFields[1] = mask/pattern indication
       mask = 0
       pattern = 1
    */
    devNum=(GT_U8)inArgs[0];

    if(inFields[1] == 0) /* mask */
    {
    mask_ruleIndex = (GT_U32)inFields[0];

    cmdOsMemCpy(
        &maskData.ruleEgrStdNotIp.common.portListBmp,
        &ruleSet_portListBmpMask,
        sizeof(CPSS_PORTS_BMP_STC));

    /*start of common*/
    maskData.ruleEgrStdNotIp.common.pclId = (GT_U16)inFields[2];
    maskData.ruleEgrStdNotIp.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[3];

    maskData.ruleEgrStdNotIp.common.isTagged = (GT_U8)inFields[4];
    maskData.ruleEgrStdNotIp.common.vid = (GT_U16)inFields[5];
    maskData.ruleEgrStdNotIp.common.up = (GT_U8)inFields[6];

    maskData.ruleEgrStdNotIp.common.isIp = (GT_U8)inFields[7];
    maskData.ruleEgrStdNotIp.common.isL2Valid = (GT_U8)inFields[8];
    maskData.ruleEgrStdNotIp.common.egrPacketType=(GT_U8)inFields[9];

    /*egrPacketType
      0 - packet to CPU
      1 - packet from CPU
      2 - packet to ANALYZER
      3 - forward DATA packet
    */

    if(pattern_set==GT_TRUE)
    {
        epclCommonEgrPktTypeInfoSet(&maskData.ruleEgrStdNotIp.common,
            inFields);
        epclCommonEgrPktTypeInfoSet(&patternData.ruleEgrStdNotIp.common,
            ruleDataWords);
    }
    else
    {
        /* save the info from mask and set it properly when the pattern is set */
        for(ii = 10 ; ii <= 20 ; ii++)
        {
            ruleDataWords[ii] = inFields[ii];
        }
    }

    maskData.ruleEgrStdNotIp.common.srcHwDev= (GT_U8)inFields[21];
    maskData.ruleEgrStdNotIp.common.sourceId=(GT_U16)inFields[22];
    maskData.ruleEgrStdNotIp.common.isVidx=(GT_U8)inFields[23];

    /*end of common*/

    maskData.ruleEgrStdNotIp.isIpv4 = (GT_U8)inFields[24];
    maskData.ruleEgrStdNotIp.etherType = (GT_U16)inFields[25];
    maskData.ruleEgrStdNotIp.isArp = (GT_U8)inFields[26];
    maskData.ruleEgrStdNotIp.l2Encap = (GT_BOOL)inFields[27];
    galtisMacAddr(&maskData.ruleEgrStdNotIp.macDa, (GT_U8*)inFields[28]);
    galtisMacAddr(&maskData.ruleEgrStdNotIp.macSa, (GT_U8*)inFields[29]);

    if( GT_TRUE == sip5Pcl )
    {
    maskData.ruleEgrStdNotIp.common.tag1Exist = (GT_U8)inFields[30];
    maskData.ruleEgrStdNotIp.vid1 = (GT_U16)inFields[31];
    maskData.ruleEgrStdNotIp.cfi1 = (GT_U8)inFields[32];
    maskData.ruleEgrStdNotIp.up1 = (GT_U8)inFields[33];
    maskData.ruleEgrStdNotIp.common.isUdbValid = (GT_U8)inFields[34];
    galtisBArray(&maskBArr,(GT_U8*)inFields[35]);

    cmdOsMemCpy(
            maskData.ruleEgrStdNotIp.udb0_3, maskBArr.data,
            maskBArr.length);
    }

    mask_set = GT_TRUE;
    }
    else /* pattern */
    {
    pattern_ruleIndex = (GT_U32)inFields[0];

    cmdOsMemCpy(
        &patternData.ruleEgrStdNotIp.common.portListBmp,
        &ruleSet_portListBmpPattern,
        sizeof(CPSS_PORTS_BMP_STC));

    /*start of common*/
    patternData.ruleEgrStdNotIp.common.pclId = (GT_U16)inFields[2];
    patternData.ruleEgrStdNotIp.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[3];

    patternData.ruleEgrStdNotIp.common.isTagged = (GT_U8)inFields[4];
    patternData.ruleEgrStdNotIp.common.vid = (GT_U16)inFields[5];
    patternData.ruleEgrStdNotIp.common.up = (GT_U8)inFields[6];

    patternData.ruleEgrStdNotIp.common.isIp = (GT_U8)inFields[7];
    patternData.ruleEgrStdNotIp.common.isL2Valid = (GT_U8)inFields[8];
    patternData.ruleEgrStdNotIp.common.egrPacketType=(GT_U8)inFields[9];

    /*egrPacketType
      0 - packet to CPU
      1 - packet from CPU
      2 - packet to ANALYZER
      3 - forward DATA packet
    */

    if(mask_set == GT_TRUE)
    {
        epclCommonEgrPktTypeInfoSet(&maskData.ruleEgrStdNotIp.common,
            ruleDataWords);
        epclCommonEgrPktTypeInfoSet(&patternData.ruleEgrStdNotIp.common,
            inFields);
    }
    else
    {
        /* save the info from mask and set it properly when the pattern is set */
        for(ii = 10 ; ii <= 20 ; ii++)
        {
            ruleDataWords[ii] = inFields[ii];
        }
    }

    patternData.ruleEgrStdNotIp.common.srcHwDev= (GT_U8)inFields[21];
    patternData.ruleEgrStdNotIp.common.sourceId=(GT_U16)inFields[22];
    patternData.ruleEgrStdNotIp.common.isVidx=(GT_U8)inFields[23];

    /*end of common*/

    patternData.ruleEgrStdNotIp.isIpv4 = (GT_U8)inFields[24];
    patternData.ruleEgrStdNotIp.etherType = (GT_U16)inFields[25];
    patternData.ruleEgrStdNotIp.isArp = (GT_U8)inFields[26];
    patternData.ruleEgrStdNotIp.l2Encap = (GT_BOOL)inFields[27];
    galtisMacAddr(&patternData.ruleEgrStdNotIp.macDa, (GT_U8*)inFields[28]);
    galtisMacAddr(&patternData.ruleEgrStdNotIp.macSa, (GT_U8*)inFields[29]);

    if( GT_TRUE == sip5Pcl )
    {
    patternData.ruleEgrStdNotIp.common.tag1Exist = (GT_U8)inFields[30];
    patternData.ruleEgrStdNotIp.vid1 = (GT_U16)inFields[31];
    patternData.ruleEgrStdNotIp.cfi1 = (GT_U8)inFields[32];
    patternData.ruleEgrStdNotIp.up1 = (GT_U8)inFields[33];
    patternData.ruleEgrStdNotIp.common.isUdbValid = (GT_U8)inFields[34];
    galtisBArray(&patternBArr,(GT_U8*)inFields[35]);

    cmdOsMemCpy(
            patternData.ruleEgrStdNotIp.udb0_3, patternBArr.data,
            patternBArr.length);
    }

    pattern_set = GT_TRUE;
    }

    if ((mask_set == GT_TRUE) && (pattern_set == GT_TRUE))
    {
        PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleEgrStdNotIp.common.sourcePort);
    }
    return CMD_OK;
}
static CMD_STATUS wrCpssDxCh3PclEgressRule_EGRESS_STD_IP_L2_QOS_E
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8           devNum;
    GT_BYTE_ARRY                       maskBArr, patternBArr;
    GT_U32   ii;

    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;


    /* map input arguments to locals */
    /*
       inFields[1] = mask/pattern indication
       mask = 0
       pattern = 1
    */
    devNum=(GT_U8)inArgs[0];

    if(inFields[1] == 0) /* mask */
    {
    mask_ruleIndex = (GT_U32)inFields[0];

    cmdOsMemCpy(
        &maskData.ruleEgrStdIpL2Qos.common.portListBmp,
        &ruleSet_portListBmpMask,
        sizeof(CPSS_PORTS_BMP_STC));

    /*start of common*/
    maskData.ruleEgrStdIpL2Qos.common.pclId = (GT_U16)inFields[2];
    maskData.ruleEgrStdIpL2Qos.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[3];

    maskData.ruleEgrStdIpL2Qos.common.isTagged = (GT_U8)inFields[4];
    maskData.ruleEgrStdIpL2Qos.common.vid = (GT_U16)inFields[5];
    maskData.ruleEgrStdIpL2Qos.common.up = (GT_U8)inFields[6];

    maskData.ruleEgrStdIpL2Qos.common.isIp = (GT_U8)inFields[7];
    maskData.ruleEgrStdIpL2Qos.common.isL2Valid = (GT_U8)inFields[8];
    maskData.ruleEgrStdIpL2Qos.common.egrPacketType=(GT_U8)inFields[9];

    /*egrPacketType
      0 - packet to CPU
      1 - packet from CPU
      2 - packet to ANALYZER
      3 - forward DATA packet
    */

    if(pattern_set==GT_TRUE)
    {
        epclCommonEgrPktTypeInfoSet(&maskData.ruleEgrStdIpL2Qos.common,
            inFields);
        epclCommonEgrPktTypeInfoSet(&patternData.ruleEgrStdIpL2Qos.common,
            ruleDataWords);
    }
    else
    {
        /* save the info from mask and set it properly when the pattern is set */
        for(ii = 10 ; ii <= 20 ; ii++)
        {
            ruleDataWords[ii] = inFields[ii];
        }
    }

    maskData.ruleEgrStdIpL2Qos.common.srcHwDev= (GT_U8)inFields[21];
    maskData.ruleEgrStdIpL2Qos.common.sourceId=(GT_U16)inFields[22];
    maskData.ruleEgrStdIpL2Qos.common.isVidx=(GT_U8)inFields[23];

    /*end of common*/




    maskData.ruleEgrStdIpL2Qos.commonStdIp.isIpv4 = (GT_U8)inFields[24];
    maskData.ruleEgrStdIpL2Qos.commonStdIp.ipProtocol = (GT_U8)inFields[25];
    maskData.ruleEgrStdIpL2Qos.commonStdIp.dscp = (GT_U8)inFields[26];
    maskData.ruleEgrStdIpL2Qos.commonStdIp.isL4Valid = (GT_U8)inFields[27];
    maskData.ruleEgrStdIpL2Qos.commonStdIp.l4Byte2 = (GT_U8)inFields[28];
    maskData.ruleEgrStdIpL2Qos.commonStdIp.l4Byte3 = (GT_U8)inFields[29];
    maskData.ruleEgrStdIpL2Qos.commonStdIp.ipv4Fragmented = (GT_U8)inFields[30];
    maskData.ruleEgrStdIpL2Qos.commonStdIp.egrTcpUdpPortComparator= (GT_U8)inFields[31];
    maskData.ruleEgrStdIpL2Qos.isArp = (GT_U8)inFields[32];
    galtisMacAddr(&maskData.ruleEgrStdIpL2Qos.macDa, (GT_U8*)inFields[33]);
    maskData.ruleEgrStdIpL2Qos.l4Byte13=(GT_U8)inFields[34];
    galtisBArray(&maskBArr,(GT_U8*)inFields[35]);
    cmdOsMemCpy(
            maskData.ruleEgrStdIpL2Qos.dipBits0to31, maskBArr.data,
            maskBArr.length);

    if( GT_TRUE == sip5Pcl )
    {
    maskData.ruleEgrStdIpL2Qos.common.tag1Exist = (GT_U8)inFields[36];
    maskData.ruleEgrStdIpL2Qos.common.isUdbValid = (GT_U8)inFields[37];
    galtisBArray(&maskBArr,(GT_U8*)inFields[38]);

    cmdOsMemCpy(
            maskData.ruleEgrStdIpL2Qos.udb4_7, maskBArr.data,
            maskBArr.length);
    }

        mask_set = GT_TRUE;
    }
    else /* pattern */
    {
    pattern_ruleIndex = (GT_U32)inFields[0];

    cmdOsMemCpy(
        &patternData.ruleEgrStdIpL2Qos.common.portListBmp,
        &ruleSet_portListBmpPattern,
        sizeof(CPSS_PORTS_BMP_STC));

    /*start of common*/
    patternData.ruleEgrStdIpL2Qos.common.pclId = (GT_U16)inFields[2];
    patternData.ruleEgrStdIpL2Qos.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[3];

    patternData.ruleEgrStdIpL2Qos.common.isTagged = (GT_U8)inFields[4];
    patternData.ruleEgrStdIpL2Qos.common.vid = (GT_U16)inFields[5];
    patternData.ruleEgrStdIpL2Qos.common.up = (GT_U8)inFields[6];

    patternData.ruleEgrStdIpL2Qos.common.isIp = (GT_U8)inFields[7];
    patternData.ruleEgrStdIpL2Qos.common.isL2Valid = (GT_U8)inFields[8];
    patternData.ruleEgrStdIpL2Qos.common.egrPacketType=(GT_U8)inFields[9];

    /*egrPacketType
      0 - packet to CPU
      1 - packet from CPU
      2 - packet to ANALYZER
      3 - forward DATA packet
    */

    if(mask_set == GT_TRUE)
    {
        epclCommonEgrPktTypeInfoSet(&maskData.ruleEgrStdIpL2Qos.common,
            ruleDataWords);
        epclCommonEgrPktTypeInfoSet(&patternData.ruleEgrStdIpL2Qos.common,
            inFields);
    }
    else
    {
        /* save the info from mask and set it properly when the pattern is set */
        for(ii = 10 ; ii <= 20 ; ii++)
        {
            ruleDataWords[ii] = inFields[ii];
        }
    }

    patternData.ruleEgrStdIpL2Qos.common.srcHwDev= (GT_U8)inFields[21];
    patternData.ruleEgrStdIpL2Qos.common.sourceId=(GT_U16)inFields[22];
    patternData.ruleEgrStdIpL2Qos.common.isVidx=(GT_U8)inFields[23];

    /*end of common*/
    patternData.ruleEgrStdIpL2Qos.commonStdIp.isIpv4 = (GT_U8)inFields[24];
    patternData.ruleEgrStdIpL2Qos.commonStdIp.ipProtocol = (GT_U8)inFields[25];
    patternData.ruleEgrStdIpL2Qos.commonStdIp.dscp = (GT_U8)inFields[26];
    patternData.ruleEgrStdIpL2Qos.commonStdIp.isL4Valid = (GT_U8)inFields[27];
    patternData.ruleEgrStdIpL2Qos.commonStdIp.l4Byte2 = (GT_U8)inFields[28];
    patternData.ruleEgrStdIpL2Qos.commonStdIp.l4Byte3 = (GT_U8)inFields[29];
    patternData.ruleEgrStdIpL2Qos.commonStdIp.ipv4Fragmented = (GT_U8)inFields[30];
    patternData.ruleEgrStdIpL2Qos.commonStdIp.egrTcpUdpPortComparator= (GT_U8)inFields[31];
    patternData.ruleEgrStdIpL2Qos.isArp = (GT_U8)inFields[32];
    galtisMacAddr(&patternData.ruleEgrStdIpL2Qos.macDa, (GT_U8*)inFields[33]);
    patternData.ruleEgrStdIpL2Qos.l4Byte13=(GT_U8)inFields[34];
    galtisBArray(&patternBArr,(GT_U8*)inFields[35]);
    cmdOsMemCpy(
            patternData.ruleEgrStdIpL2Qos.dipBits0to31, patternBArr.data,
            patternBArr.length);

    if( GT_TRUE == sip5Pcl )
    {
    patternData.ruleEgrStdIpL2Qos.common.tag1Exist = (GT_U8)inFields[36];
    patternData.ruleEgrStdIpL2Qos.common.isUdbValid = (GT_U8)inFields[37];
    galtisBArray(&patternBArr,(GT_U8*)inFields[38]);

    cmdOsMemCpy(
            patternData.ruleEgrStdIpL2Qos.udb4_7, patternBArr.data,
            patternBArr.length);
    }

        pattern_set = GT_TRUE;
    }

    if ((mask_set == GT_TRUE) && (pattern_set == GT_TRUE))
    {
        PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleEgrStdIpL2Qos.common.sourcePort);
    }
    return CMD_OK;
}

static CMD_STATUS wrCpssDxCh3PclEgressRule_EGRESS_STD_IPV4_L4_E
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_BYTE_ARRY    maskBArr, patternBArr;
    GT_U8           devNum;
    GT_U32   ii;

    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* map input arguments to locals */
    /*
       inFields[1] = mask/pattern indication
       mask = 0
       pattern = 1
    */
    if(inFields[1] == 0) /* mask */
    {
    mask_ruleIndex = (GT_U32)inFields[0];

    cmdOsMemCpy(
        &maskData.ruleEgrStdIpv4L4.common.portListBmp,
        &ruleSet_portListBmpMask,
        sizeof(CPSS_PORTS_BMP_STC));

    /*start of common*/
    maskData.ruleEgrStdIpv4L4.common.pclId = (GT_U16)inFields[2];
    maskData.ruleEgrStdIpv4L4.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[3];

    maskData.ruleEgrStdIpv4L4.common.isTagged = (GT_U8)inFields[4];
    maskData.ruleEgrStdIpv4L4.common.vid = (GT_U16)inFields[5];
    maskData.ruleEgrStdIpv4L4.common.up = (GT_U8)inFields[6];

    maskData.ruleEgrStdIpv4L4.common.isIp = (GT_U8)inFields[7];
    maskData.ruleEgrStdIpv4L4.common.isL2Valid = (GT_U8)inFields[8];
    maskData.ruleEgrStdIpv4L4.common.egrPacketType=(GT_U8)inFields[9];

    /*egrPacketType
      0 - packet to CPU
      1 - packet from CPU
      2 - packet to ANALYZER
      3 - forward DATA packet
    */

    if(pattern_set==GT_TRUE)
    {
        epclCommonEgrPktTypeInfoSet(&maskData.ruleEgrStdIpv4L4.common,
            inFields);
        epclCommonEgrPktTypeInfoSet(&patternData.ruleEgrStdIpv4L4.common,
            ruleDataWords);
    }
    else
    {
        /* save the info from mask and set it properly when the pattern is set */
        for(ii = 10 ; ii <= 20 ; ii++)
        {
            ruleDataWords[ii] = inFields[ii];
        }
    }

    maskData.ruleEgrStdIpv4L4.common.srcHwDev= (GT_U8)inFields[21];
    maskData.ruleEgrStdIpv4L4.common.sourceId=(GT_U16)inFields[22];
    maskData.ruleEgrStdIpv4L4.common.isVidx=(GT_U8)inFields[23];

    /*end of common*/

    maskData.ruleEgrStdIpv4L4.commonStdIp.isIpv4 = (GT_U8)inFields[24];
    maskData.ruleEgrStdIpv4L4.commonStdIp.ipProtocol = (GT_U8)inFields[25];
    maskData.ruleEgrStdIpv4L4.commonStdIp.dscp = (GT_U8)inFields[26];
    maskData.ruleEgrStdIpv4L4.commonStdIp.isL4Valid = (GT_U8)inFields[27];
    maskData.ruleEgrStdIpv4L4.commonStdIp.l4Byte2 = (GT_U8)inFields[28];
    maskData.ruleEgrStdIpv4L4.commonStdIp.l4Byte3 = (GT_U8)inFields[29];
    maskData.ruleEgrStdIpv4L4.commonStdIp.ipv4Fragmented = (GT_U8)inFields[30];
    maskData.ruleEgrStdIpv4L4.commonStdIp.egrTcpUdpPortComparator= (GT_U8)inFields[31];
    maskData.ruleEgrStdIpv4L4.isArp = (GT_U8)inFields[32];
    maskData.ruleEgrStdIpv4L4.isBc = (GT_U8)inFields[33];
    galtisIpAddr(&maskData.ruleEgrStdIpv4L4.sip, (GT_U8*)inFields[34]);
    galtisIpAddr(&maskData.ruleEgrStdIpv4L4.dip, (GT_U8*)inFields[35]);
    maskData.ruleEgrStdIpv4L4.l4Byte0 = (GT_U8)inFields[36];
    maskData.ruleEgrStdIpv4L4.l4Byte1 = (GT_U8)inFields[37];
    maskData.ruleEgrStdIpv4L4.l4Byte13 = (GT_U8)inFields[38];

    if( GT_TRUE == sip5Pcl )
    {
    maskData.ruleEgrStdIpv4L4.common.tag1Exist = (GT_U8)inFields[39];
    maskData.ruleEgrStdIpv4L4.common.isUdbValid = (GT_U8)inFields[40];
    galtisBArray(&maskBArr,(GT_U8*)inFields[41]);

    cmdOsMemCpy(
            maskData.ruleEgrStdIpv4L4.udb8_11, maskBArr.data,
            maskBArr.length);
    }

    mask_set = GT_TRUE;
    }
    else /* pattern */
    {
   pattern_ruleIndex = (GT_U32)inFields[0];

   cmdOsMemCpy(
       &patternData.ruleEgrStdIpv4L4.common.portListBmp,
       &ruleSet_portListBmpPattern,
       sizeof(CPSS_PORTS_BMP_STC));

   /*start of common*/
    patternData.ruleEgrStdIpv4L4.common.pclId = (GT_U16)inFields[2];
    patternData.ruleEgrStdIpv4L4.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[3];

    patternData.ruleEgrStdIpv4L4.common.isTagged = (GT_U8)inFields[4];
    patternData.ruleEgrStdIpv4L4.common.vid = (GT_U16)inFields[5];
    patternData.ruleEgrStdIpv4L4.common.up = (GT_U8)inFields[6];

    patternData.ruleEgrStdIpv4L4.common.isIp = (GT_U8)inFields[7];
    patternData.ruleEgrStdIpv4L4.common.isL2Valid = (GT_U8)inFields[8];
    patternData.ruleEgrStdIpv4L4.common.egrPacketType=(GT_U8)inFields[9];

    /*egrPacketType
      0 - packet to CPU
      1 - packet from CPU
      2 - packet to ANALYZER
      3 - forward DATA packet
    */

    if(mask_set == GT_TRUE)
    {
        epclCommonEgrPktTypeInfoSet(&maskData.ruleEgrStdIpv4L4.common,
            ruleDataWords);
        epclCommonEgrPktTypeInfoSet(&patternData.ruleEgrStdIpv4L4.common,
            inFields);
    }
    else
    {
        /* save the info from mask and set it properly when the pattern is set */
        for(ii = 10 ; ii <= 20 ; ii++)
        {
            ruleDataWords[ii] = inFields[ii];
        }
    }


    patternData.ruleEgrStdIpv4L4.common.srcHwDev= (GT_U8)inFields[21];
    patternData.ruleEgrStdIpv4L4.common.sourceId=(GT_U16)inFields[22];
    patternData.ruleEgrStdIpv4L4.common.isVidx=(GT_U8)inFields[23];

    /*end of common*/

    patternData.ruleEgrStdIpv4L4.commonStdIp.isIpv4 = (GT_U8)inFields[24];
    patternData.ruleEgrStdIpv4L4.commonStdIp.ipProtocol = (GT_U8)inFields[25];
    patternData.ruleEgrStdIpv4L4.commonStdIp.dscp = (GT_U8)inFields[26];
    patternData.ruleEgrStdIpv4L4.commonStdIp.isL4Valid = (GT_U8)inFields[27];
    patternData.ruleEgrStdIpv4L4.commonStdIp.l4Byte2 = (GT_U8)inFields[28];
    patternData.ruleEgrStdIpv4L4.commonStdIp.l4Byte3 = (GT_U8)inFields[29];
    patternData.ruleEgrStdIpv4L4.commonStdIp.ipv4Fragmented = (GT_U8)inFields[30];
    patternData.ruleEgrStdIpv4L4.commonStdIp.egrTcpUdpPortComparator= (GT_U8)inFields[31];
    patternData.ruleEgrStdIpv4L4.isArp = (GT_U8)inFields[32];
    patternData.ruleEgrStdIpv4L4.isBc = (GT_U8)inFields[33];
    galtisIpAddr(&patternData.ruleEgrStdIpv4L4.sip, (GT_U8*)inFields[34]);
    galtisIpAddr(&patternData.ruleEgrStdIpv4L4.dip, (GT_U8*)inFields[35]);
    patternData.ruleEgrStdIpv4L4.l4Byte0 = (GT_U8)inFields[36];
    patternData.ruleEgrStdIpv4L4.l4Byte1 = (GT_U8)inFields[37];
    patternData.ruleEgrStdIpv4L4.l4Byte13 = (GT_U8)inFields[38];

    if( GT_TRUE == sip5Pcl )
    {
    patternData.ruleEgrStdIpv4L4.common.tag1Exist = (GT_U8)inFields[39];
    patternData.ruleEgrStdIpv4L4.common.isUdbValid = (GT_U8)inFields[40];
    galtisBArray(&patternBArr,(GT_U8*)inFields[41]);

    cmdOsMemCpy(
            patternData.ruleEgrStdIpv4L4.udb8_11, patternBArr.data,
            patternBArr.length);
    }

    pattern_set = GT_TRUE;
    }

    if ((mask_set == GT_TRUE) && (pattern_set == GT_TRUE))
    {
        PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleEgrStdIpv4L4.common.sourcePort);
    }
    return CMD_OK;
}
/**********************************************************************************/

static CMD_STATUS wrCpssDxCh3PclEgressRule_EGRESS_EXT_NOT_IPV6_E
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_BYTE_ARRY    maskBArr, patternBArr;
    GT_U8           devNum;
    GT_U32   ii;

    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* map input arguments to locals */
    /*
       inFields[1] = mask/pattern indication
       mask = 0
       pattern = 1
    */
    if(inFields[1] == 0) /* mask */
    {
    mask_ruleIndex = (GT_U32)inFields[0];

    cmdOsMemCpy(
        &maskData.ruleEgrExtNotIpv6.common.portListBmp,
        &ruleSet_portListBmpMask,
        sizeof(CPSS_PORTS_BMP_STC));

    /*start of common*/
    maskData.ruleEgrExtNotIpv6.common.pclId = (GT_U16)inFields[2];
    maskData.ruleEgrExtNotIpv6.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[3];

    maskData.ruleEgrExtNotIpv6.common.isTagged = (GT_U8)inFields[4];
    maskData.ruleEgrExtNotIpv6.common.vid = (GT_U16)inFields[5];
    maskData.ruleEgrExtNotIpv6.common.up = (GT_U8)inFields[6];

    maskData.ruleEgrExtNotIpv6.common.isIp = (GT_U8)inFields[7];
    maskData.ruleEgrExtNotIpv6.common.isL2Valid = (GT_U8)inFields[8];
    maskData.ruleEgrExtNotIpv6.common.egrPacketType=(GT_U8)inFields[9];

    /*egrPacketType
      0 - packet to CPU
      1 - packet from CPU
      2 - packet to ANALYZER
      3 - forward DATA packet
    */

    if(pattern_set==GT_TRUE)
    {
        epclCommonEgrPktTypeInfoSet(&maskData.ruleEgrExtNotIpv6.common,
            inFields);
        epclCommonEgrPktTypeInfoSet(&patternData.ruleEgrExtNotIpv6.common,
            ruleDataWords);
    }
    else
    {
        /* save the info from mask and set it properly when the pattern is set */
        for(ii = 10 ; ii <= 20 ; ii++)
        {
            ruleDataWords[ii] = inFields[ii];
        }
    }

    maskData.ruleEgrExtNotIpv6.common.srcHwDev= (GT_U8)inFields[21];
    maskData.ruleEgrExtNotIpv6.common.sourceId=(GT_U16)inFields[22];
    maskData.ruleEgrExtNotIpv6.common.isVidx=(GT_U8)inFields[23];

    /*end of common*/

    maskData.ruleEgrExtNotIpv6.commonExt.isIpv6 = (GT_U8)inFields[24];
    maskData.ruleEgrExtNotIpv6.commonExt.ipProtocol = (GT_U8)inFields[25];
    maskData.ruleEgrExtNotIpv6.commonExt.dscp = (GT_U8)inFields[26];
    maskData.ruleEgrExtNotIpv6.commonExt.isL4Valid = (GT_U8)inFields[27];
    maskData.ruleEgrExtNotIpv6.commonExt.l4Byte0 = (GT_U8)inFields[28];
    maskData.ruleEgrExtNotIpv6.commonExt.l4Byte1 = (GT_U8)inFields[29];
    maskData.ruleEgrExtNotIpv6.commonExt.l4Byte2 = (GT_U8)inFields[30];
    maskData.ruleEgrExtNotIpv6.commonExt.l4Byte3 = (GT_U8)inFields[31];
    maskData.ruleEgrExtNotIpv6.commonExt.l4Byte13 = (GT_U8)inFields[32];
    maskData.ruleEgrExtNotIpv6.commonExt.egrTcpUdpPortComparator = (GT_U8)inFields[33];
    galtisIpAddr(&maskData.ruleEgrExtNotIpv6.sip, (GT_U8*)inFields[34]);
    galtisIpAddr(&maskData.ruleEgrExtNotIpv6.dip, (GT_U8*)inFields[35]);
    maskData.ruleEgrExtNotIpv6.etherType = (GT_U16)inFields[36];
    maskData.ruleEgrExtNotIpv6.l2Encap = (GT_U8)inFields[37];
    galtisMacAddr(&maskData.ruleEgrExtNotIpv6.macDa, (GT_U8*)inFields[38]);
    galtisMacAddr(&maskData.ruleEgrExtNotIpv6.macSa, (GT_U8*)inFields[39]);
    maskData.ruleEgrExtNotIpv6.ipv4Fragmented = (GT_U8)inFields[40];

    if( GT_TRUE == sip5Pcl )
    {
    maskData.ruleEgrExtNotIpv6.common.tag1Exist = (GT_U8)inFields[41];
    maskData.ruleEgrExtNotIpv6.vid1 = (GT_U16)inFields[42];
    maskData.ruleEgrExtNotIpv6.cfi1 = (GT_U8)inFields[43];
    maskData.ruleEgrExtNotIpv6.up1 = (GT_U8)inFields[44];
    maskData.ruleEgrExtNotIpv6.isMpls = (GT_U8)inFields[45];
    maskData.ruleEgrExtNotIpv6.numOfMplsLabels = (GT_U8)inFields[46];
    maskData.ruleEgrExtNotIpv6.protocolTypeAfterMpls = (GT_U8)inFields[47];
    maskData.ruleEgrExtNotIpv6.mplsLabel0 = (GT_U32)inFields[48];
    maskData.ruleEgrExtNotIpv6.mplsExp0 = (GT_U8)inFields[49];
    maskData.ruleEgrExtNotIpv6.mplsLabel1 = (GT_U32)inFields[50];
    maskData.ruleEgrExtNotIpv6.mplsExp1 = (GT_U8)inFields[51];
    maskData.ruleEgrExtNotIpv6.commonExt.isUdbValid = (GT_U8)inFields[52];
    galtisBArray(&maskBArr,(GT_U8*)inFields[53]);

    cmdOsMemCpy(
            maskData.ruleEgrExtNotIpv6.udb12_19, maskBArr.data,
            maskBArr.length);
    }

    mask_set = GT_TRUE;
    }
    else /* pattern */
    {
    pattern_ruleIndex = (GT_U32)inFields[0];

    cmdOsMemCpy(
        &patternData.ruleEgrExtNotIpv6.common.portListBmp,
        &ruleSet_portListBmpPattern,
        sizeof(CPSS_PORTS_BMP_STC));

 /*start of common*/
    patternData.ruleEgrExtNotIpv6.common.pclId = (GT_U16)inFields[2];
    patternData.ruleEgrExtNotIpv6.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[3];

    patternData.ruleEgrExtNotIpv6.common.isTagged = (GT_U8)inFields[4];
    patternData.ruleEgrExtNotIpv6.common.vid = (GT_U16)inFields[5];
    patternData.ruleEgrExtNotIpv6.common.up = (GT_U8)inFields[6];

    patternData.ruleEgrExtNotIpv6.common.isIp = (GT_U8)inFields[7];
    patternData.ruleEgrExtNotIpv6.common.isL2Valid = (GT_U8)inFields[8];
    patternData.ruleEgrExtNotIpv6.common.egrPacketType=(GT_U8)inFields[9];

    /*egrPacketType
      0 - packet to CPU
      1 - packet from CPU
      2 - packet to ANALYZER
      3 - forward DATA packet
    */

    if(mask_set == GT_TRUE)
    {
        epclCommonEgrPktTypeInfoSet(&maskData.ruleEgrExtNotIpv6.common,
            ruleDataWords);
        epclCommonEgrPktTypeInfoSet(&patternData.ruleEgrExtNotIpv6.common,
            inFields);
    }
    else
    {
        /* save the info from mask and set it properly when the pattern is set */
        for(ii = 10 ; ii <= 20 ; ii++)
        {
            ruleDataWords[ii] = inFields[ii];
        }
    }

    patternData.ruleEgrExtNotIpv6.common.srcHwDev= (GT_U8)inFields[21];
    patternData.ruleEgrExtNotIpv6.common.sourceId=(GT_U16)inFields[22];
    patternData.ruleEgrExtNotIpv6.common.isVidx=(GT_U8)inFields[23];

    /*end of common*/

    patternData.ruleEgrExtNotIpv6.commonExt.isIpv6 = (GT_U8)inFields[24];
    patternData.ruleEgrExtNotIpv6.commonExt.ipProtocol = (GT_U8)inFields[25];
    patternData.ruleEgrExtNotIpv6.commonExt.dscp = (GT_U8)inFields[26];
    patternData.ruleEgrExtNotIpv6.commonExt.isL4Valid = (GT_U8)inFields[27];
    patternData.ruleEgrExtNotIpv6.commonExt.l4Byte0 = (GT_U8)inFields[28];
    patternData.ruleEgrExtNotIpv6.commonExt.l4Byte1 = (GT_U8)inFields[29];
    patternData.ruleEgrExtNotIpv6.commonExt.l4Byte2 = (GT_U8)inFields[30];
    patternData.ruleEgrExtNotIpv6.commonExt.l4Byte3 = (GT_U8)inFields[31];
    patternData.ruleEgrExtNotIpv6.commonExt.l4Byte13 = (GT_U8)inFields[32];
    patternData.ruleEgrExtNotIpv6.commonExt.egrTcpUdpPortComparator = (GT_U8)inFields[33];
    galtisIpAddr(&patternData.ruleEgrExtNotIpv6.sip, (GT_U8*)inFields[34]);
    galtisIpAddr(&patternData.ruleEgrExtNotIpv6.dip, (GT_U8*)inFields[35]);
    patternData.ruleEgrExtNotIpv6.etherType = (GT_U16)inFields[36];
    patternData.ruleEgrExtNotIpv6.l2Encap = (GT_U8)inFields[37];
    galtisMacAddr(&patternData.ruleEgrExtNotIpv6.macDa, (GT_U8*)inFields[38]);
    galtisMacAddr(&patternData.ruleEgrExtNotIpv6.macSa, (GT_U8*)inFields[39]);
    patternData.ruleEgrExtNotIpv6.ipv4Fragmented = (GT_U8)inFields[40];

    if( GT_TRUE == sip5Pcl )
    {
    patternData.ruleEgrExtNotIpv6.common.tag1Exist = (GT_U8)inFields[41];
    patternData.ruleEgrExtNotIpv6.vid1 = (GT_U16)inFields[42];
    patternData.ruleEgrExtNotIpv6.cfi1 = (GT_U8)inFields[43];
    patternData.ruleEgrExtNotIpv6.up1 = (GT_U8)inFields[44];
    patternData.ruleEgrExtNotIpv6.isMpls = (GT_U8)inFields[45];
    patternData.ruleEgrExtNotIpv6.numOfMplsLabels = (GT_U8)inFields[46];
    patternData.ruleEgrExtNotIpv6.protocolTypeAfterMpls = (GT_U8)inFields[47];
    patternData.ruleEgrExtNotIpv6.mplsLabel0 = (GT_U32)inFields[48];
    patternData.ruleEgrExtNotIpv6.mplsExp0 = (GT_U8)inFields[49];
    patternData.ruleEgrExtNotIpv6.mplsLabel1 = (GT_U32)inFields[50];
    patternData.ruleEgrExtNotIpv6.mplsExp1 = (GT_U8)inFields[51];
    patternData.ruleEgrExtNotIpv6.commonExt.isUdbValid = (GT_U8)inFields[52];
    galtisBArray(&patternBArr,(GT_U8*)inFields[53]);

    cmdOsMemCpy(
            patternData.ruleEgrExtNotIpv6.udb12_19, patternBArr.data,
            patternBArr.length);
    }

    pattern_set = GT_TRUE;
    }

    if ((mask_set == GT_TRUE) && (pattern_set == GT_TRUE))
    {
        PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleEgrExtNotIpv6.common.sourcePort);
    }
    return CMD_OK;
}

static CMD_STATUS wrCpssDxCh3PclEgressRule_EGRESS_EXT_IPV6_L2_E
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_BYTE_ARRY    maskBArr, patternBArr;
    GT_U8           devNum;
    GT_U32   ii;
    GT_IPV6ADDR    tmpIpv6;

    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* map input arguments to locals */
    /*
       inFields[1] = mask/pattern indication
       mask = 0
       pattern = 1
    */
    if(inFields[1] == 0) /* mask */
    {
        mask_ruleIndex = (GT_U32)inFields[0];

        cmdOsMemCpy(
            &maskData.ruleEgrExtIpv6L2.common.portListBmp,
            &ruleSet_portListBmpMask,
            sizeof(CPSS_PORTS_BMP_STC));

        /*start of common*/
        maskData.ruleEgrExtIpv6L2.common.pclId = (GT_U16)inFields[2];
        maskData.ruleEgrExtIpv6L2.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[3];

        maskData.ruleEgrExtIpv6L2.common.isTagged = (GT_U8)inFields[4];
        maskData.ruleEgrExtIpv6L2.common.vid = (GT_U16)inFields[5];
        maskData.ruleEgrExtIpv6L2.common.up = (GT_U8)inFields[6];

        maskData.ruleEgrExtIpv6L2.common.isIp = (GT_U8)inFields[7];
        maskData.ruleEgrExtIpv6L2.common.isL2Valid = (GT_U8)inFields[8];
        maskData.ruleEgrExtIpv6L2.common.egrPacketType=(GT_U8)inFields[9];

        /*egrPacketType
          0 - packet to CPU
          1 - packet from CPU
          2 - packet to ANALYZER
          3 - forward DATA packet
        */

        if(pattern_set==GT_TRUE)
        {
            epclCommonEgrPktTypeInfoSet(&maskData.ruleEgrExtIpv6L2.common,
                inFields);
            epclCommonEgrPktTypeInfoSet(&patternData.ruleEgrExtIpv6L2.common,
                ruleDataWords);
        }
        else
        {
            /* save the info from mask and set it properly when the pattern is set */
            for(ii = 10 ; ii <= 20 ; ii++)
            {
                ruleDataWords[ii] = inFields[ii];
            }
        }


        maskData.ruleEgrExtIpv6L2.common.srcHwDev= (GT_U8)inFields[21];
        maskData.ruleEgrExtIpv6L2.common.sourceId=(GT_U16)inFields[22];
        maskData.ruleEgrExtIpv6L2.common.isVidx=(GT_U8)inFields[23];

        /*end of common*/

        maskData.ruleEgrExtIpv6L2.commonExt.isIpv6 = (GT_U8)inFields[24];
        maskData.ruleEgrExtIpv6L2.commonExt.ipProtocol = (GT_U8)inFields[25];
        maskData.ruleEgrExtIpv6L2.commonExt.dscp = (GT_U8)inFields[26];
        maskData.ruleEgrExtIpv6L2.commonExt.isL4Valid = (GT_U8)inFields[27];
        maskData.ruleEgrExtIpv6L2.commonExt.l4Byte0 = (GT_U8)inFields[28];
        maskData.ruleEgrExtIpv6L2.commonExt.l4Byte1 = (GT_U8)inFields[29];
        maskData.ruleEgrExtIpv6L2.commonExt.l4Byte2 = (GT_U8)inFields[30];
        maskData.ruleEgrExtIpv6L2.commonExt.l4Byte3 = (GT_U8)inFields[31];
        maskData.ruleEgrExtIpv6L2.commonExt.l4Byte13 = (GT_U8)inFields[32];
        maskData.ruleEgrExtIpv6L2.commonExt.egrTcpUdpPortComparator = (GT_U8)inFields[33];
        galtisIpv6Addr(&maskData.ruleEgrExtIpv6L2.sip, (GT_U8*)inFields[34]);
        galtisIpv6Addr(&tmpIpv6, (GT_U8*)inFields[35]);
        maskData.ruleEgrExtIpv6L2.dipBits127to120 = tmpIpv6.arIP[0];

        galtisMacAddr(&maskData.ruleEgrExtIpv6L2.macDa, (GT_U8*)inFields[36]);
        galtisMacAddr(&maskData.ruleEgrExtIpv6L2.macSa, (GT_U8*)inFields[37]);

        if( GT_TRUE == sip5Pcl )
        {
            maskData.ruleEgrExtIpv6L2.common.tag1Exist = (GT_U8)inFields[38];
            maskData.ruleEgrExtIpv6L2.vid1 = (GT_U16)inFields[39];
            maskData.ruleEgrExtIpv6L2.cfi1 = (GT_U8)inFields[40];
            maskData.ruleEgrExtIpv6L2.up1 = (GT_U8)inFields[41];
            maskData.ruleEgrExtIpv6L2.isIpv6ExtHdrExist = (GT_U8)inFields[42];
            maskData.ruleEgrExtIpv6L2.isIpv6HopByHop = (GT_U8)inFields[43];
            maskData.ruleEgrExtIpv6L2.commonExt.isUdbValid = (GT_U8)inFields[44];
            galtisBArray(&maskBArr,(GT_U8*)inFields[45]);

            cmdOsMemCpy(
                maskData.ruleEgrExtIpv6L2.udb20_27, maskBArr.data,
                maskBArr.length);
        }

        mask_set = GT_TRUE;
    }
    else /* pattern */
    {
        pattern_ruleIndex = (GT_U32)inFields[0];

        cmdOsMemCpy(
            &patternData.ruleEgrExtIpv6L2.common.portListBmp,
            &ruleSet_portListBmpPattern,
            sizeof(CPSS_PORTS_BMP_STC));

        /*start of common*/
        patternData.ruleEgrExtIpv6L2.common.pclId = (GT_U16)inFields[2];
        patternData.ruleEgrExtIpv6L2.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[3];

        patternData.ruleEgrExtIpv6L2.common.isTagged = (GT_U8)inFields[4];
        patternData.ruleEgrExtIpv6L2.common.vid = (GT_U16)inFields[5];
        patternData.ruleEgrExtIpv6L2.common.up = (GT_U8)inFields[6];

        patternData.ruleEgrExtIpv6L2.common.isIp = (GT_U8)inFields[7];
        patternData.ruleEgrExtIpv6L2.common.isL2Valid = (GT_U8)inFields[8];
        patternData.ruleEgrExtIpv6L2.common.egrPacketType=(GT_U8)inFields[9];

        /*egrPacketType
          0 - packet to CPU
          1 - packet from CPU
          2 - packet to ANALYZER
          3 - forward DATA packet
        */

        if(mask_set == GT_TRUE)
        {
            epclCommonEgrPktTypeInfoSet(&maskData.ruleEgrExtIpv6L2.common,
                ruleDataWords);
            epclCommonEgrPktTypeInfoSet(&patternData.ruleEgrExtIpv6L2.common,
                inFields);
        }
        else
        {
            /* save the info from mask and set it properly when the pattern is set */
            for(ii = 10 ; ii <= 20 ; ii++)
            {
                ruleDataWords[ii] = inFields[ii];
            }
        }

        patternData.ruleEgrExtIpv6L2.common.srcHwDev= (GT_U8)inFields[21];
        patternData.ruleEgrExtIpv6L2.common.sourceId=(GT_U16)inFields[22];
        patternData.ruleEgrExtIpv6L2.common.isVidx=(GT_U8)inFields[23];

        /*end of common*/

        patternData.ruleEgrExtIpv6L2.commonExt.isIpv6 = (GT_U8)inFields[24];
        patternData.ruleEgrExtIpv6L2.commonExt.ipProtocol = (GT_U8)inFields[25];
        patternData.ruleEgrExtIpv6L2.commonExt.dscp = (GT_U8)inFields[26];
        patternData.ruleEgrExtIpv6L2.commonExt.isL4Valid = (GT_U8)inFields[27];
        patternData.ruleEgrExtIpv6L2.commonExt.l4Byte0 = (GT_U8)inFields[28];
        patternData.ruleEgrExtIpv6L2.commonExt.l4Byte1 = (GT_U8)inFields[29];
        patternData.ruleEgrExtIpv6L2.commonExt.l4Byte2 = (GT_U8)inFields[30];
        patternData.ruleEgrExtIpv6L2.commonExt.l4Byte3 = (GT_U8)inFields[31];
        patternData.ruleEgrExtIpv6L2.commonExt.l4Byte13 = (GT_U8)inFields[32];
        patternData.ruleEgrExtIpv6L2.commonExt.egrTcpUdpPortComparator = (GT_U8)inFields[33];
        galtisIpv6Addr(&patternData.ruleEgrExtIpv6L2.sip, (GT_U8*)inFields[34]);
        galtisIpv6Addr(&tmpIpv6, (GT_U8*)inFields[35]);
        patternData.ruleEgrExtIpv6L2.dipBits127to120 = tmpIpv6.arIP[0];

        galtisMacAddr(&patternData.ruleEgrExtIpv6L2.macDa, (GT_U8*)inFields[36]);
        galtisMacAddr(&patternData.ruleEgrExtIpv6L2.macSa, (GT_U8*)inFields[37]);

        if( GT_TRUE == sip5Pcl )
        {
            patternData.ruleEgrExtIpv6L2.common.tag1Exist = (GT_U8)inFields[38];
            patternData.ruleEgrExtIpv6L2.vid1 = (GT_U16)inFields[39];
            patternData.ruleEgrExtIpv6L2.cfi1 = (GT_U8)inFields[40];
            patternData.ruleEgrExtIpv6L2.up1 = (GT_U8)inFields[41];
            patternData.ruleEgrExtIpv6L2.isIpv6ExtHdrExist = (GT_U8)inFields[42];
            patternData.ruleEgrExtIpv6L2.isIpv6HopByHop = (GT_U8)inFields[43];
            patternData.ruleEgrExtIpv6L2.commonExt.isUdbValid = (GT_U8)inFields[44];
            galtisBArray(&patternBArr,(GT_U8*)inFields[45]);

            cmdOsMemCpy(
                patternData.ruleEgrExtIpv6L2.udb20_27, patternBArr.data,
                patternBArr.length);
        }

        pattern_set = GT_TRUE;
    }

    if ((mask_set == GT_TRUE) && (pattern_set == GT_TRUE))
    {
        PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleEgrExtIpv6L2.common.sourcePort);
    }
    return CMD_OK;
}
static CMD_STATUS wrCpssDxCh3PclEgressRule_EGRESS_EXT_IPV6_L4_E
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_BYTE_ARRY    maskBArr, patternBArr;
    GT_U8           devNum;
    GT_U32   ii;

    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* map input arguments to locals */
    /*
       inFields[1] = mask/pattern indication
       mask = 0
       pattern = 1
    */
    if(inFields[1] == 0) /* mask */
    {
    mask_ruleIndex = (GT_U32)inFields[0];

    cmdOsMemCpy(
        &maskData.ruleEgrExtIpv6L4.common.portListBmp,
        &ruleSet_portListBmpMask,
        sizeof(CPSS_PORTS_BMP_STC));

   /*start of common*/
    maskData.ruleEgrExtIpv6L4.common.pclId = (GT_U16)inFields[2];
    maskData.ruleEgrExtIpv6L4.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[3];

    maskData.ruleEgrExtIpv6L4.common.isTagged = (GT_U8)inFields[4];
    maskData.ruleEgrExtIpv6L4.common.vid = (GT_U16)inFields[5];
    maskData.ruleEgrExtIpv6L4.common.up = (GT_U8)inFields[6];

    maskData.ruleEgrExtIpv6L4.common.isIp = (GT_U8)inFields[7];
    maskData.ruleEgrExtIpv6L4.common.isL2Valid = (GT_U8)inFields[8];
    maskData.ruleEgrExtIpv6L4.common.egrPacketType=(GT_U8)inFields[9];

    /*egrPacketType
      0 - packet to CPU
      1 - packet from CPU
      2 - packet to ANALYZER
      3 - forward DATA packet
    */

    if(pattern_set==GT_TRUE)
    {
        epclCommonEgrPktTypeInfoSet(&maskData.ruleEgrExtIpv6L4.common,
            inFields);
        epclCommonEgrPktTypeInfoSet(&patternData.ruleEgrExtIpv6L4.common,
            ruleDataWords);
    }
    else
    {
        /* save the info from mask and set it properly when the pattern is set */
        for(ii = 10 ; ii <= 20 ; ii++)
        {
            ruleDataWords[ii] = inFields[ii];
        }
    }


    maskData.ruleEgrExtIpv6L4.common.srcHwDev= (GT_U8)inFields[21];
    maskData.ruleEgrExtIpv6L4.common.sourceId=(GT_U16)inFields[22];
    maskData.ruleEgrExtIpv6L4.common.isVidx=(GT_U8)inFields[23];

    /*end of common*/

    maskData.ruleEgrExtIpv6L4.commonExt.isIpv6 = (GT_U8)inFields[24];
    maskData.ruleEgrExtIpv6L4.commonExt.ipProtocol = (GT_U8)inFields[25];
    maskData.ruleEgrExtIpv6L4.commonExt.dscp = (GT_U8)inFields[26];
    maskData.ruleEgrExtIpv6L4.commonExt.isL4Valid = (GT_U8)inFields[27];
    maskData.ruleEgrExtIpv6L4.commonExt.l4Byte0 = (GT_U8)inFields[28];
    maskData.ruleEgrExtIpv6L4.commonExt.l4Byte1 = (GT_U8)inFields[29];
    maskData.ruleEgrExtIpv6L4.commonExt.l4Byte2 = (GT_U8)inFields[30];
    maskData.ruleEgrExtIpv6L4.commonExt.l4Byte3 = (GT_U8)inFields[31];
    maskData.ruleEgrExtIpv6L4.commonExt.l4Byte13 = (GT_U8)inFields[32];
    maskData.ruleEgrExtIpv6L4.commonExt.egrTcpUdpPortComparator = (GT_U8)inFields[33];

    galtisIpv6Addr(&maskData.ruleEgrExtIpv6L4.sip, (GT_U8*)inFields[34]);
    galtisIpv6Addr(&maskData.ruleEgrExtIpv6L4.dip, (GT_U8*)inFields[35]);

    if( GT_TRUE == sip5Pcl )
    {
        maskData.ruleEgrExtIpv6L4.common.tag1Exist = (GT_U8)inFields[36];
        maskData.ruleEgrExtIpv6L4.isIpv6ExtHdrExist = (GT_U8)inFields[37];
        maskData.ruleEgrExtIpv6L4.isIpv6HopByHop = (GT_U8)inFields[38];
        maskData.ruleEgrExtIpv6L4.commonExt.isUdbValid = (GT_U8)inFields[39];
        galtisBArray(&maskBArr,(GT_U8*)inFields[40]);

        cmdOsMemCpy(
            maskData.ruleEgrExtIpv6L4.udb28_35, maskBArr.data,
            maskBArr.length);
    }

    mask_set = GT_TRUE;
    }
    else /* pattern */
    {
    pattern_ruleIndex = (GT_U32)inFields[0];

    cmdOsMemCpy(
        &patternData.ruleEgrExtIpv6L4.common.portListBmp,
        &ruleSet_portListBmpPattern,
        sizeof(CPSS_PORTS_BMP_STC));

    /*start of common*/
    patternData.ruleEgrExtIpv6L4.common.pclId = (GT_U16)inFields[2];
    patternData.ruleEgrExtIpv6L4.common.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[3];

    patternData.ruleEgrExtIpv6L4.common.isTagged = (GT_U8)inFields[4];
    patternData.ruleEgrExtIpv6L4.common.vid = (GT_U16)inFields[5];
    patternData.ruleEgrExtIpv6L4.common.up = (GT_U8)inFields[6];

    patternData.ruleEgrExtIpv6L4.common.isIp = (GT_U8)inFields[7];
    patternData.ruleEgrExtIpv6L4.common.isL2Valid = (GT_U8)inFields[8];
    patternData.ruleEgrExtIpv6L4.common.egrPacketType=(GT_U8)inFields[9];

    /*egrPacketType
      0 - packet to CPU
      1 - packet from CPU
      2 - packet to ANALYZER
      3 - forward DATA packet
    */

    if(mask_set == GT_TRUE)
    {
        epclCommonEgrPktTypeInfoSet(&maskData.ruleEgrExtIpv6L4.common,
            ruleDataWords);
        epclCommonEgrPktTypeInfoSet(&patternData.ruleEgrExtIpv6L4.common,
            inFields);
    }
    else
    {
        /* save the info from mask and set it properly when the pattern is set */
        for(ii = 10 ; ii <= 20 ; ii++)
        {
            ruleDataWords[ii] = inFields[ii];
        }
    }


    patternData.ruleEgrExtIpv6L4.common.srcHwDev= (GT_U8)inFields[21];
    patternData.ruleEgrExtIpv6L4.common.sourceId=(GT_U16)inFields[22];
    patternData.ruleEgrExtIpv6L4.common.isVidx=(GT_U8)inFields[23];

    /*end of common*/

    patternData.ruleEgrExtIpv6L4.commonExt.isIpv6 = (GT_U8)inFields[24];
    patternData.ruleEgrExtIpv6L4.commonExt.ipProtocol = (GT_U8)inFields[25];
    patternData.ruleEgrExtIpv6L4.commonExt.dscp = (GT_U8)inFields[26];
    patternData.ruleEgrExtIpv6L4.commonExt.isL4Valid = (GT_U8)inFields[27];
    patternData.ruleEgrExtIpv6L4.commonExt.l4Byte0 = (GT_U8)inFields[28];
    patternData.ruleEgrExtIpv6L4.commonExt.l4Byte1 = (GT_U8)inFields[29];
    patternData.ruleEgrExtIpv6L4.commonExt.l4Byte2 = (GT_U8)inFields[30];
    patternData.ruleEgrExtIpv6L4.commonExt.l4Byte3 = (GT_U8)inFields[31];
    patternData.ruleEgrExtIpv6L4.commonExt.l4Byte13 = (GT_U8)inFields[32];
    patternData.ruleEgrExtIpv6L4.commonExt.egrTcpUdpPortComparator = (GT_U8)inFields[33];

    galtisIpv6Addr(&patternData.ruleEgrExtIpv6L4.sip, (GT_U8*)inFields[34]);
    galtisIpv6Addr(&patternData.ruleEgrExtIpv6L4.dip, (GT_U8*)inFields[35]);

    if( GT_TRUE == sip5Pcl )
    {
        patternData.ruleEgrExtIpv6L4.common.tag1Exist = (GT_U8)inFields[36];
        patternData.ruleEgrExtIpv6L4.isIpv6ExtHdrExist = (GT_U8)inFields[37];
        patternData.ruleEgrExtIpv6L4.isIpv6HopByHop = (GT_U8)inFields[38];
        patternData.ruleEgrExtIpv6L4.commonExt.isUdbValid = (GT_U8)inFields[39];
        galtisBArray(&patternBArr,(GT_U8*)inFields[40]);

        cmdOsMemCpy(
            patternData.ruleEgrExtIpv6L4.udb28_35, patternBArr.data,
            patternBArr.length);
    }

    pattern_set = GT_TRUE;
    }

    if ((mask_set == GT_TRUE) && (pattern_set == GT_TRUE))
    {
        PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleEgrExtIpv6L4.common.sourcePort);
    }
    return CMD_OK;
}
/*********************************************************************************/

static CMD_STATUS  wrCpssDxCh3PclEgressRuleWriteFirst

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cmdOsMemSet(&maskData , 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
    cmdOsMemSet(&patternData, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));


    mask_set    = GT_FALSE;
    pattern_set = GT_FALSE;
   switch (inArgs[1]+CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E)
    {
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E:
            wrCpssDxCh3PclEgressRule_EGRESS_STD_NOT_IP_E(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E:
            wrCpssDxCh3PclEgressRule_EGRESS_STD_IP_L2_QOS_E(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E:
            wrCpssDxCh3PclEgressRule_EGRESS_STD_IPV4_L4_E(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E:
            wrCpssDxCh3PclEgressRule_EGRESS_EXT_NOT_IPV6_E(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E:
            wrCpssDxCh3PclEgressRule_EGRESS_EXT_IPV6_L2_E(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E:
            wrCpssDxCh3PclEgressRule_EGRESS_EXT_IPV6_L4_E(
                inArgs, inFields ,numFields ,outArgs);
            break;

       default:
            galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong PCL RULE FORMAT.\n");
            return CMD_AGENT_ERROR;
    }
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}
/*************************************************************************/

static CMD_STATUS wrCpssDxCh3PclEgressRuleWriteNext

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result = GT_OK;
    GT_U8           devNum;
    CPSS_DXCH_PCL_ACTION_STC         *actionPtr;
    GT_U32                              actionIndex;
    GT_U32                              ruleIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;


    /* Check Validity */
    if(inFields[1] > 1)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong value for data_type (should be MASK or PATTERN).\n");
        return CMD_AGENT_ERROR;
    }


    if(inFields[1] == 0 && mask_set)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Must be subsequent pair of Ingress {Rule,Mask}.\n");
        return CMD_AGENT_ERROR;
    }

    if(inFields[1] == 1 && pattern_set)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Must be subsequent pair of Ingress {Rule,Mask}.\n");
        return CMD_AGENT_ERROR;
    }

    /*
       inFields[1] = mask/pattern indication
       mask = 0
       pattern = 1
    */
    switch (inArgs[1]+CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E)
    {
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E:
            wrCpssDxCh3PclEgressRule_EGRESS_STD_NOT_IP_E(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E:
            wrCpssDxCh3PclEgressRule_EGRESS_STD_IP_L2_QOS_E(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E:
            wrCpssDxCh3PclEgressRule_EGRESS_STD_IPV4_L4_E(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E:
            wrCpssDxCh3PclEgressRule_EGRESS_EXT_NOT_IPV6_E(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E:
            wrCpssDxCh3PclEgressRule_EGRESS_EXT_IPV6_L2_E(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E:
            wrCpssDxCh3PclEgressRule_EGRESS_EXT_IPV6_L4_E(
                inArgs, inFields ,numFields ,outArgs);
            break;

       default:
            galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong PCL RULE FORMAT.\n");
            return CMD_AGENT_ERROR;
    }
    if(mask_set && pattern_set && (mask_ruleIndex != pattern_ruleIndex))
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : mask ruleIndex doesn't match pattern ruleIndex.\n");
        return CMD_AGENT_ERROR;
    }

    /* Get the action */
    ruleIndex = (GT_U32)inFields[0];
    pclDxCh3ActionGet( ruleIndex, &actionIndex);

    if(actionIndex == INVALID_RULE_INDEX || !(actionTable[actionIndex].valid))
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Action table entry is not set.\n");
        return CMD_AGENT_ERROR;

    }

    if(mask_set && pattern_set)
    {
         /* map input arguments to locals */
        devNum = (GT_U8)inArgs[0];

        actionPtr = &(actionTable[actionIndex].actionEntry);
        lastActionPtr = actionPtr;

        /* call cpss api function */
        result = pg_wrap_cpssDxChPclRuleSet(devNum, ruleFormat, ruleIndex, &maskData,
                                                    &patternData, actionPtr);

        mask_set = GT_FALSE;
        pattern_set = GT_FALSE;

        /* maskData, patternData, lastActionPtr cleared only at */
        /* wrCpssDxCh3PclEgressRuleWriteFirst                   */
        /* Keeped for use in utilites                           */
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/*************************************************************************/

static CMD_STATUS wrCpssDxCh3PclEgressRuleWriteEnd
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
  if(mask_set || pattern_set)
      /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_BAD_STATE, "");
    else
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

/*********************************************************************************/
static GT_VOID  wrCpssDxCh3PclEgressRuleReadParseFields
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  CPSS_DXCH_PCL_RULE_FORMAT_UNT *ruleDataPtr,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC          *egrCommonPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STD_IP_STC   *egrCommonStdIpPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC      *egrCommonExtPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_STC      *egrStdNotIpPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_STC   *egrStdIpL2QosPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_STC     *egrStdIpv4L4Ptr;
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC    *egrExtNotIpv6Ptr;
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_STC     *egrExtIpv6L2Ptr;
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_STC     *egrExtIpv6L4Ptr;

    egrCommonPtr       = NULL;
    egrCommonStdIpPtr  = NULL;
    egrCommonExtPtr    = NULL;
    egrStdNotIpPtr     = NULL;
    egrStdIpL2QosPtr   = NULL;
    egrStdIpv4L4Ptr    = NULL;
    egrExtNotIpv6Ptr   = NULL;
    egrExtIpv6L2Ptr    = NULL;
    egrExtIpv6L4Ptr    = NULL;

    switch (utilCpssDxChPclRuleParsedGet_RuleFormat)
    {
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E:
            egrCommonPtr   = &(ruleDataPtr->ruleEgrStdNotIp.common);
            egrStdNotIpPtr = &(ruleDataPtr->ruleEgrStdNotIp);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E:
            egrCommonPtr = &(ruleDataPtr->ruleEgrStdIpL2Qos.common);
            egrCommonStdIpPtr = &(ruleDataPtr->ruleEgrStdIpL2Qos.commonStdIp);
            egrStdIpL2QosPtr  = &(ruleDataPtr->ruleEgrStdIpL2Qos);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E:
            egrCommonPtr      = &(ruleDataPtr->ruleEgrStdIpv4L4.common);
            egrCommonStdIpPtr = &(ruleDataPtr->ruleEgrStdIpv4L4.commonStdIp);
            egrStdIpv4L4Ptr   = &(ruleDataPtr->ruleEgrStdIpv4L4);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E:
            egrCommonPtr      = &(ruleDataPtr->ruleEgrExtNotIpv6.common);
            egrCommonExtPtr   = &(ruleDataPtr->ruleEgrExtNotIpv6.commonExt);
            egrExtNotIpv6Ptr  = &(ruleDataPtr->ruleEgrExtNotIpv6);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E:
            egrCommonPtr     = &(ruleDataPtr->ruleEgrExtIpv6L2.common);
            egrCommonExtPtr  = &(ruleDataPtr->ruleEgrExtIpv6L2.commonExt);
            egrExtIpv6L2Ptr  = &(ruleDataPtr->ruleEgrExtIpv6L2);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E:
            egrCommonPtr     = &(ruleDataPtr->ruleEgrExtIpv6L4.common);
            egrCommonExtPtr  = &(ruleDataPtr->ruleEgrExtIpv6L4.commonExt);
            egrExtIpv6L4Ptr  = &(ruleDataPtr->ruleEgrExtIpv6L4);
            break;

        default:
            galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong PCL RULE FORMAT.\n");
            break;
    }

    if (egrCommonPtr != NULL)
    {
        fieldOutput(
            "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
            egrCommonPtr->pclId, egrCommonPtr->sourcePort,
            egrCommonPtr->isTagged, egrCommonPtr->vid,
            egrCommonPtr->up, egrCommonPtr->isIp,
            egrCommonPtr->isL2Valid, egrCommonPtr->egrPacketType,
            egrCommonPtr->toCpu.cpuCode, egrCommonPtr->toCpu.srcTrg,
            egrCommonPtr->fromCpu.tc, egrCommonPtr->fromCpu.dp,
            egrCommonPtr->fromCpu.egrFilterEnable, egrCommonPtr->toAnalyzer.rxSniff,
            egrCommonPtr->fwdData.qosProfile, egrCommonPtr->fwdData.srcTrunkId,
            egrCommonPtr->fwdData.srcIsTrunk, egrCommonPtr->fwdData.isUnknown,
            egrCommonPtr->fwdData.isRouted, egrCommonPtr->srcHwDev,
            egrCommonPtr->sourceId, egrCommonPtr->isVidx);
         fieldOutputSetAppendMode();
    }

    if (egrCommonStdIpPtr != NULL)
    {
        fieldOutput(
            "%d%d%d%d%d%d%d%d",
            egrCommonStdIpPtr->isIpv4, egrCommonStdIpPtr->ipProtocol,
            egrCommonStdIpPtr->dscp, egrCommonStdIpPtr->isL4Valid,
            egrCommonStdIpPtr->l4Byte2, egrCommonStdIpPtr->l4Byte3,
            egrCommonStdIpPtr->ipv4Fragmented,
            egrCommonStdIpPtr->egrTcpUdpPortComparator);
         fieldOutputSetAppendMode();
    }

    if (egrCommonExtPtr != NULL)
    {
        fieldOutput(
            "%d%d%d%d%d%d%d%d%d%d",
            egrCommonExtPtr->isIpv6, egrCommonExtPtr->ipProtocol,
            egrCommonExtPtr->dscp, egrCommonExtPtr->isL4Valid,
            egrCommonExtPtr->l4Byte0, egrCommonExtPtr->l4Byte1,
            egrCommonExtPtr->l4Byte2, egrCommonExtPtr->l4Byte3,
            egrCommonExtPtr->l4Byte13,
            egrCommonExtPtr->egrTcpUdpPortComparator);
         fieldOutputSetAppendMode();
    }

    if (egrStdNotIpPtr != NULL)
    {
        fieldOutput(
            "%d%d%d%d%s%s",
            egrStdNotIpPtr->isIpv4, egrStdNotIpPtr->etherType,
            egrStdNotIpPtr->isArp, egrStdNotIpPtr->l2Encap,
            galtisByteArrayToString(egrStdNotIpPtr->macDa.arEther, 6),
            galtisByteArrayToString(egrStdNotIpPtr->macSa.arEther, 6));
    }

    if (egrStdIpL2QosPtr != NULL)
    {
        fieldOutput(
            "%d%s%d%s",
            egrStdIpL2QosPtr->isArp,
            galtisByteArrayToString(egrStdIpL2QosPtr->macDa.arEther, 6),
            egrStdIpL2QosPtr->l4Byte13,
            galtisByteArrayToString(egrStdIpL2QosPtr->dipBits0to31, 4));
    }

    if (egrStdIpv4L4Ptr != NULL)
    {
        fieldOutput(
            "%d%d%s%s%d%d%d",
            egrStdIpv4L4Ptr->isArp, egrStdIpv4L4Ptr->isArp,
            galtisByteArrayToString(egrStdIpv4L4Ptr->sip.arIP, 4),
            galtisByteArrayToString(egrStdIpv4L4Ptr->dip.arIP, 4),
            egrStdIpv4L4Ptr->l4Byte0, egrStdIpv4L4Ptr->l4Byte1,
            egrStdIpv4L4Ptr->l4Byte13);
    }

    if (egrExtNotIpv6Ptr != NULL)
    {
        fieldOutput(
            "%s%s%d%d%s%s%d",
            galtisByteArrayToString(egrExtNotIpv6Ptr->sip.arIP, 4),
            galtisByteArrayToString(egrExtNotIpv6Ptr->dip.arIP, 4),
            egrExtNotIpv6Ptr->etherType, egrExtNotIpv6Ptr->l2Encap,
            galtisByteArrayToString(egrExtNotIpv6Ptr->macDa.arEther, 6),
            galtisByteArrayToString(egrExtNotIpv6Ptr->macSa.arEther, 6),
            egrExtNotIpv6Ptr->ipv4Fragmented);
    }

    if (egrExtIpv6L2Ptr != NULL)
    {
        fieldOutput(
            "%s%d%s%s",
            galtisByteArrayToString(egrExtIpv6L2Ptr->sip.arIP, 16),
            egrExtIpv6L2Ptr->dipBits127to120,
            galtisByteArrayToString(egrExtIpv6L2Ptr->macDa.arEther, 6),
            galtisByteArrayToString(egrExtIpv6L2Ptr->macSa.arEther, 6));
    }

    if (egrExtIpv6L4Ptr != NULL)
    {
        fieldOutput(
            "%s%s",
            galtisByteArrayToString(egrExtIpv6L4Ptr->sip.arIP, 16),
            galtisByteArrayToString(egrExtIpv6L4Ptr->dip.arIP, 16));
    }
}

static CMD_STATUS  wrCpssDxCh3PclEgressRuleReadParse
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32                        rc;
    GT_BOOL                       isRuleValid;
    GT_U32                        dataType;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT *ruleDataPtr;

    if (utilCpssDxChPclRuleParsedGet_LinesAmount == 0)
    {
        /* end of required rules */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    if ((utilCpssDxChPclRuleParsedGet_LinesAmount & 1) == 0)
    {
        /* even line: - new rule */
        rc = pg_wrap_cpssDxChPclRuleParsedGet(
            (GT_U8)inArgs[0] /*devNum*/,
            utilCpssDxChPclRuleParsedGet_RuleFormat,
            utilCpssDxChPclRuleParsedGet_RuleIndex,
            0 /*ruleOptionsBmp*/,
            &isRuleValid, &maskData, &patternData, &actionData);
        if (rc != GT_OK)
        {
            galtisOutput(outArgs, rc, "%d", -1);
            return CMD_OK;
        }
        /* output mask */
        ruleDataPtr = &maskData;
        dataType = 0;
    }
    else
    {
        /* odd line: output pattern */
        ruleDataPtr = &patternData;
        dataType = 1;
    }

    /* Tab-number, ruleIndex and dataType(mask/pattern) before fields */
    fieldOutput(
        "%d%d",
        utilCpssDxChPclRuleParsedGet_RuleIndex, dataType);
    fieldOutputSetAppendMode();

    /* output rule fields to internal buffer */
    wrCpssDxCh3PclEgressRuleReadParseFields(
        inArgs, inFields, ruleDataPtr, outArgs);

    /* output all to GUI */
    galtisOutput(outArgs, GT_OK, "%d%f",
        (utilCpssDxChPclRuleParsedGet_RuleFormat
            - CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E)
        );

    if ((utilCpssDxChPclRuleParsedGet_LinesAmount & 1) != 0)
    {
        /*  odd line: increment rule index */
        utilCpssDxChPclRuleParsedGet_RuleIndex ++;
    }

    /* decrement amount of not outputed lines */
    utilCpssDxChPclRuleParsedGet_LinesAmount --;

    return CMD_OK;
}

/*********************************************************************************/
static CMD_STATUS  wrCpssDxCh3PclEgressRuleReadFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if ((utilCpssDxChPclRuleParsedGet_RuleFormat
        < CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E)
        || (utilCpssDxChPclRuleParsedGet_RuleFormat
        > CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E))
    {
        /* rule format for another table */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_AGENT_ERROR;
    }

     return wrCpssDxCh3PclEgressRuleReadParse(
        inArgs, inFields, outArgs);
}

/*************************************************************************/
static CMD_STATUS wrCpssDxCh3PclEgressRuleReadNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    return wrCpssDxCh3PclEgressRuleReadParse(
       inArgs, inFields, outArgs);
}

/****************************end of egress*****************************/

/* XCat UDB Table */

/* Extract Direction from Rule Format */
static CPSS_PCL_DIRECTION_ENT wrCpssDxChPclRuleFormatToDirection
(
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat
)
{
    switch (ruleFormat)
    {
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E          :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E       :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E         :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E        :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E         :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E         :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E  :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E:
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E              :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E              :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E              :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E              :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E              :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E              :
            return CPSS_PCL_DIRECTION_EGRESS_E;

        default: return CPSS_PCL_DIRECTION_INGRESS_E;
    }
}

/**
* @internal wrCpssDxChPclUserDefinedByteTableSet function
* @endinternal
*
* @brief   The function configures the User Defined Byte (UDB)
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_OUT_OF_RANGE          - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note See comments to CPSS_DXCH_PCL_OFFSET_TYPE_ENT
*
*/
static CMD_STATUS wrCpssDxChPclUserDefinedByteTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS rc;
    GT_U8                                devNum;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType;
    GT_U32                               udbIndex;
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT        offsetType;
    GT_U8                                offset;
    CPSS_PCL_DIRECTION_ENT               direction;

    devNum      = (GT_U8)inArgs[0];
    ruleFormat  = (CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT)inArgs[1];
    packetType  = (CPSS_DXCH_PCL_PACKET_TYPE_ENT)inArgs[2];
    udbIndex    = (GT_U32)inFields[0];
    offsetType  = (CPSS_DXCH_PCL_OFFSET_TYPE_ENT)inFields[1];
    offset      = (GT_U8)inFields[2];

    direction = wrCpssDxChPclRuleFormatToDirection(ruleFormat);

    rc =  pg_wrap_cpssDxChPclUserDefinedByteSet(
        devNum, ruleFormat, packetType, direction,
        udbIndex, offsetType, offset);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

/* index for Get first/next */
static GT_U32  udbTableIndex;

/**
* @internal wrCpssDxChPclUserDefinedByteTableGetNext function
* @endinternal
*
* @brief   The function gets the User Defined Byte (UDB) configuration
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note See comments to CPSS_DXCH_PCL_OFFSET_TYPE_ENT
*
*/
static CMD_STATUS wrCpssDxChPclUserDefinedByteTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS rc;
    GT_U8                                devNum;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType;
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT        offsetType;
    GT_U8                                offset;
    CPSS_PCL_DIRECTION_ENT               direction;

    devNum      = (GT_U8)inArgs[0];
    ruleFormat  = (CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT)inArgs[1];
    packetType  = (CPSS_DXCH_PCL_PACKET_TYPE_ENT)inArgs[2];
    offsetType  = (CPSS_DXCH_PCL_OFFSET_TYPE_ENT)0;
    offset      = (GT_U8)0;

    direction = wrCpssDxChPclRuleFormatToDirection(ruleFormat);

    rc =  pg_wrap_cpssDxChPclUserDefinedByteGet(
        devNum, ruleFormat, packetType, direction,
        udbTableIndex, &offsetType, &offset);

    if ((rc != GT_OK) && (udbTableIndex != 0))
    {
        /* notify "no records more" */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    fieldOutput("%d%d%d", udbTableIndex, offsetType, offset);
    galtisOutput(outArgs, rc, "%f");

    /* increment index */
    udbTableIndex ++;

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclUserDefinedByteTableGetFirst function
* @endinternal
*
* @brief   The function gets the User Defined Byte (UDB) configuration
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note See comments to CPSS_DXCH_PCL_OFFSET_TYPE_ENT
*
*/
static CMD_STATUS wrCpssDxChPclUserDefinedByteTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    udbTableIndex = 0;

    return wrCpssDxChPclUserDefinedByteTableGetNext(
        inArgs, inFields, numFields, outArgs);
}

/**
* @internal wrCpssDxChPclUserDefinedBytesSelectTableSet function
* @endinternal
*
* @brief   Set the User Defined Byte (UDB) Selection Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChPclUserDefinedBytesSelectTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            rc = GT_OK;
    GT_U8                                devNum;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType;
    CPSS_PCL_LOOKUP_NUMBER_ENT           lookupNum;
    CPSS_DXCH_PCL_UDB_SELECT_STC         udbSelect;
    GT_U8                                byteArr[128];
    GT_U32                               i;

    cpssOsMemSet(&udbSelect, 0, sizeof(udbSelect));

    devNum      = (GT_U8)inArgs[0];
    ruleFormat  = (CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT)inArgs[1];
    packetType  = (CPSS_DXCH_PCL_PACKET_TYPE_ENT)inArgs[2];
    lookupNum   = (CPSS_PCL_LOOKUP_NUMBER_ENT)inArgs[3];

    galtisBytesArrayDirect(byteArr, (GT_U8*)inFields[0], sizeof(byteArr));
    for (i = 0; (i < CPSS_DXCH_PCL_UDB_MAX_NUMBER_CNS); i++)
    {
        udbSelect.udbSelectArr[i] = (GT_U32)byteArr[i];
    }

    galtisBytesArrayDirect(byteArr, (GT_U8*)inFields[1], sizeof(byteArr));
    for (i = 0; (i < CPSS_DXCH_PCL_INGRESS_UDB_REPLACE_MAX_CNS); i++)
    {
        udbSelect.ingrUdbReplaceArr[i] = ((byteArr[i] == 0) ? GT_FALSE : GT_TRUE);
    }

    udbSelect.egrUdb01Replace = (GT_BOOL)inFields[2];
    udbSelect.egrUdbBit15Replace = (GT_BOOL)inFields[3];

    if ((PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum))
        && (packetType == CPSS_DXCH_PCL_PACKET_TYPE_IPV6_E))
    {
        /* backward compatibility , PACKET_TYPE_IPV6_E not relevant for E_ARCH */
        GT_U32                             _ii;
        static CPSS_DXCH_PCL_PACKET_TYPE_ENT _pktType[3] =
        {
            CPSS_DXCH_PCL_PACKET_TYPE_IPV6_TCP_E,
            CPSS_DXCH_PCL_PACKET_TYPE_IPV6_UDP_E,
            CPSS_DXCH_PCL_PACKET_TYPE_IPV6_OTHER_E
        };

        for (_ii = 0; (_ii < 3); _ii++)
        {
            rc =  cpssDxChPclUserDefinedBytesSelectSet(
                devNum, ruleFormat, _pktType[_ii], lookupNum, &udbSelect);
            if (rc != GT_OK)
            {
                break;
            }
        }
    }
    else
    {
        rc =  cpssDxChPclUserDefinedBytesSelectSet(
            devNum, ruleFormat, packetType, lookupNum, &udbSelect);
    }

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPclUserDefinedBytesSelectTableGetFirst function
* @endinternal
*
* @brief   Get the User Defined Byte (UDB) Selection Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on unsupported HW state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChPclUserDefinedBytesSelectTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS rc;
    GT_U8                                devNum;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType;
    CPSS_PCL_LOOKUP_NUMBER_ENT           lookupNum;
    CPSS_DXCH_PCL_UDB_SELECT_STC         udbSelect;
    GT_U8                                byteArr[128];
    GT_U32                               i;
    char*                                strPtr;
    GT_U32                               numUdbs;

    devNum      = (GT_U8)inArgs[0];
    ruleFormat  = (CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT)inArgs[1];
    packetType  = (CPSS_DXCH_PCL_PACKET_TYPE_ENT)inArgs[2];
    lookupNum   = (CPSS_PCL_LOOKUP_NUMBER_ENT)inArgs[3];

    if ((PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum))
        && (packetType == CPSS_DXCH_PCL_PACKET_TYPE_IPV6_E))
    {
        /* backward compatibility , PACKET_TYPE_IPV6_E not relevant for E_ARCH */
        packetType = CPSS_DXCH_PCL_PACKET_TYPE_IPV6_OTHER_E;
    }

    rc =  cpssDxChPclUserDefinedBytesSelectGet(
        devNum, ruleFormat, packetType, lookupNum, &udbSelect);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "%d", -1);
        return CMD_OK;
    }

    numUdbs = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)?70:50;

    for (i = 0; i < numUdbs; i++)
    {
        byteArr[i] = (GT_U8)udbSelect.udbSelectArr[i];
    }
    strPtr = galtisByteArrayToString(byteArr,
        PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)?70:50);
    fieldOutput("%s", strPtr);
    fieldOutputSetAppendMode();


    for (i = 0; (i < CPSS_DXCH_PCL_INGRESS_UDB_REPLACE_MAX_CNS); i++)
    {
        byteArr[i] = BOOL2BIT_MAC(udbSelect.ingrUdbReplaceArr[i]);
    }
    strPtr = galtisByteArrayToString(byteArr, CPSS_DXCH_PCL_INGRESS_UDB_REPLACE_MAX_CNS);
    fieldOutput("%s", strPtr);
    fieldOutputSetAppendMode();

    fieldOutput("%d%d", udbSelect.egrUdb01Replace, udbSelect.egrUdbBit15Replace);
    galtisOutput(outArgs, rc, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclOverrideUserDefinedBytesSet function
* @endinternal
*
* @brief   The function sets Override User Defined Bytes
*
* @note   APPLICABLE DEVICES:      All DxChXCat devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
*
* @note NONE
*
*/
static CMD_STATUS wrCpssDxChPclOverrideUserDefinedBytesSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                    devNum;
    PRV_DXCH_PCL_OVERRIDE_UDB_STC           udbOverride;
    GT_STATUS                                result;
    GT_U32                                   i;

    i = 0;
    cpssOsMemSet(&udbOverride, 0, sizeof(udbOverride));

    devNum = (GT_U8)inArgs[i++];
    udbOverride.vrfIdLsbEnableStdNotIp      = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdMsbEnableStdNotIp      = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdLsbEnableStdIpL2Qos    = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdMsbEnableStdIpL2Qos    = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdLsbEnableStdIpv4L4     = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdMsbEnableStdIpv4L4     = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdLsbEnableStdUdb        = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdMsbEnableStdUdb        = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdLsbEnableExtNotIpv6    = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdMsbEnableExtNotIpv6    = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdLsbEnableExtIpv6L2     = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdMsbEnableExtIpv6L2     = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdLsbEnableExtIpv6L4     = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdMsbEnableExtIpv6L4     = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdLsbEnableExtUdb        = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdMsbEnableExtUdb        = (GT_BOOL)inArgs[i++];
    udbOverride.qosProfileEnableStdUdb      = (GT_BOOL)inArgs[i++];
    udbOverride.qosProfileEnableExtUdb      = (GT_BOOL)inArgs[i++];

    result = prvGaltisDxChPclOverrideUserDefinedBytesSet(
        devNum, &udbOverride, GT_FALSE);

    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclOverrideUserDefinedBytesGet function
* @endinternal
*
* @brief   The function gets Override User Defined Bytes
*
* @note   APPLICABLE DEVICES:      All DxChXCat devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
*
* @note NONE
*
*/
static CMD_STATUS wrCpssDxChPclOverrideUserDefinedBytesGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                    devNum;
    PRV_DXCH_PCL_OVERRIDE_UDB_STC           udbOverride;
    GT_STATUS                                result;

    devNum = (GT_U8)inArgs[0];
    cpssOsMemSet(&udbOverride, 0, sizeof(udbOverride));

    result = prvGaltisDxChPclOverrideUserDefinedBytesGet(
        devNum, &udbOverride, GT_FALSE);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    galtisOutput(
        outArgs, GT_OK, "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
        udbOverride.vrfIdLsbEnableStdNotIp,
        udbOverride.vrfIdMsbEnableStdNotIp,
        udbOverride.vrfIdLsbEnableStdIpL2Qos,
        udbOverride.vrfIdMsbEnableStdIpL2Qos,
        udbOverride.vrfIdLsbEnableStdIpv4L4,
        udbOverride.vrfIdMsbEnableStdIpv4L4,
        udbOverride.vrfIdLsbEnableStdUdb,
        udbOverride.vrfIdMsbEnableStdUdb,
        udbOverride.vrfIdLsbEnableExtNotIpv6,
        udbOverride.vrfIdMsbEnableExtNotIpv6,
        udbOverride.vrfIdLsbEnableExtIpv6L2,
        udbOverride.vrfIdMsbEnableExtIpv6L2,
        udbOverride.vrfIdLsbEnableExtIpv6L4,
        udbOverride.vrfIdMsbEnableExtIpv6L4,
        udbOverride.vrfIdLsbEnableExtUdb,
        udbOverride.vrfIdMsbEnableExtUdb,
        udbOverride.qosProfileEnableStdUdb,
        udbOverride.qosProfileEnableExtUdb);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclOverrideUserDefinedBytesSet_1 function
* @endinternal
*
* @brief   The function sets Override User Defined Bytes
*
* @note   APPLICABLE DEVICES:      All DxChXCat devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
*
* @note NONE
*
*/
static CMD_STATUS wrCpssDxChPclOverrideUserDefinedBytesSet_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                    devNum;
    PRV_DXCH_PCL_OVERRIDE_UDB_STC           udbOverride;
    GT_STATUS                                result;
    GT_U32                                   i;

    i = 0;
    cpssOsMemSet(&udbOverride, 0, sizeof(udbOverride));

    devNum = (GT_U8)inArgs[i++];
    udbOverride.vrfIdLsbEnableStdNotIp      = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdMsbEnableStdNotIp      = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdLsbEnableStdIpL2Qos    = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdMsbEnableStdIpL2Qos    = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdLsbEnableStdIpv4L4     = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdMsbEnableStdIpv4L4     = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdLsbEnableStdUdb        = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdMsbEnableStdUdb        = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdLsbEnableExtNotIpv6    = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdMsbEnableExtNotIpv6    = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdLsbEnableExtIpv6L2     = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdMsbEnableExtIpv6L2     = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdLsbEnableExtIpv6L4     = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdMsbEnableExtIpv6L4     = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdLsbEnableExtUdb        = (GT_BOOL)inArgs[i++];
    udbOverride.vrfIdMsbEnableExtUdb        = (GT_BOOL)inArgs[i++];
    udbOverride.qosProfileEnableStdUdb      = (GT_BOOL)inArgs[i++];
    udbOverride.qosProfileEnableExtUdb      = (GT_BOOL)inArgs[i++];

    result = prvGaltisDxChPclOverrideUserDefinedBytesSet(
        devNum, &udbOverride, GT_TRUE);

    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclOverrideUserDefinedBytesGet_1 function
* @endinternal
*
* @brief   The function gets Override User Defined Bytes
*
* @note   APPLICABLE DEVICES:      All DxChXCat devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
*
* @note NONE
*
*/
static CMD_STATUS wrCpssDxChPclOverrideUserDefinedBytesGet_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                    devNum;
    PRV_DXCH_PCL_OVERRIDE_UDB_STC           udbOverride;
    GT_STATUS                                result;

    devNum = (GT_U8)inArgs[0];
    cpssOsMemSet(&udbOverride, 0, sizeof(udbOverride));

    result = prvGaltisDxChPclOverrideUserDefinedBytesGet(
        devNum, &udbOverride, GT_TRUE);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    galtisOutput(
        outArgs, GT_OK, "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
        udbOverride.vrfIdLsbEnableStdNotIp,
        udbOverride.vrfIdMsbEnableStdNotIp,
        udbOverride.vrfIdLsbEnableStdIpL2Qos,
        udbOverride.vrfIdMsbEnableStdIpL2Qos,
        udbOverride.vrfIdLsbEnableStdIpv4L4,
        udbOverride.vrfIdMsbEnableStdIpv4L4,
        udbOverride.vrfIdLsbEnableStdUdb,
        udbOverride.vrfIdMsbEnableStdUdb,
        udbOverride.vrfIdLsbEnableExtNotIpv6,
        udbOverride.vrfIdMsbEnableExtNotIpv6,
        udbOverride.vrfIdLsbEnableExtIpv6L2,
        udbOverride.vrfIdMsbEnableExtIpv6L2,
        udbOverride.vrfIdLsbEnableExtIpv6L4,
        udbOverride.vrfIdMsbEnableExtIpv6L4,
        udbOverride.vrfIdLsbEnableExtUdb,
        udbOverride.vrfIdMsbEnableExtUdb,
        udbOverride.qosProfileEnableStdUdb,
        udbOverride.qosProfileEnableExtUdb);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclOverrideUserDefinedBytesByTrunkHashSet function
* @endinternal
*
* @brief   The function sets overriding of User Defined Bytes
*         by packets Trunk Hash value.
*
* @note   APPLICABLE DEVICES:      Lion and above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChPclOverrideUserDefinedBytesByTrunkHashSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                      devNum;
    PRV_DXCH_PCL_OVERRIDE_UDB_TRUNK_HASH_STC  udbOverTrunkHash;
    GT_STATUS                                  result;
    GT_U32                                     i;

    i = 0;
    cpssOsMemSet(&udbOverTrunkHash, 0, sizeof(udbOverTrunkHash));

    devNum = (GT_U8)inArgs[i++];
    udbOverTrunkHash.trunkHashEnableStdNotIp      = (GT_BOOL)inArgs[i++];
    udbOverTrunkHash.trunkHashEnableStdIpv4L4     = (GT_BOOL)inArgs[i++];
    udbOverTrunkHash.trunkHashEnableStdUdb        = (GT_BOOL)inArgs[i++];
    udbOverTrunkHash.trunkHashEnableExtNotIpv6    = (GT_BOOL)inArgs[i++];
    udbOverTrunkHash.trunkHashEnableExtIpv6L2     = (GT_BOOL)inArgs[i++];
    udbOverTrunkHash.trunkHashEnableExtIpv6L4     = (GT_BOOL)inArgs[i++];
    udbOverTrunkHash.trunkHashEnableExtUdb        = (GT_BOOL)inArgs[i++];

    result = prvGaltisDxChPclOverrideUserDefinedBytesByTrunkHashSet(
        devNum, &udbOverTrunkHash);

    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclOverrideUserDefinedBytesByTrunkHashGet function
* @endinternal
*
* @brief   The function gets overriding of User Defined Bytes
*         by packets Trunk Hash value.
*
* @note   APPLICABLE DEVICES:      Lion and above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChPclOverrideUserDefinedBytesByTrunkHashGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                      devNum;
    PRV_DXCH_PCL_OVERRIDE_UDB_TRUNK_HASH_STC  udbOverTrunkHash;
    GT_STATUS                                  result;

    devNum = (GT_U8)inArgs[0];
    cpssOsMemSet(&udbOverTrunkHash, 0, sizeof(udbOverTrunkHash));

    result = prvGaltisDxChPclOverrideUserDefinedBytesByTrunkHashGet(
        devNum, &udbOverTrunkHash);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    galtisOutput(
        outArgs, GT_OK, "%d%d%d%d%d%d%d",
        udbOverTrunkHash.trunkHashEnableStdNotIp,
        udbOverTrunkHash.trunkHashEnableStdIpv4L4,
        udbOverTrunkHash.trunkHashEnableStdUdb,
        udbOverTrunkHash.trunkHashEnableExtNotIpv6,
        udbOverTrunkHash.trunkHashEnableExtIpv6L2,
        udbOverTrunkHash.trunkHashEnableExtIpv6L4,
        udbOverTrunkHash.trunkHashEnableExtUdb);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclUdeEtherTypeSet function
* @endinternal
*
* @brief   This function sets UDE Ethertype.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_RANGE          - on out of range parameter value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclUdeEtherTypeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8           devNum;
    GT_U32          index;
    GT_U32          ethType;
    GT_STATUS       result;

    devNum   = (GT_U8) inArgs[0];
    index    = (GT_U32)inArgs[1];
    ethType  = (GT_U32)inArgs[2];

    result = cpssDxChPclUdeEtherTypeSet(
        devNum, index, ethType);

    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclUdeEtherTypeGet function
* @endinternal
*
* @brief   This function gets the UDE Ethertype.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclUdeEtherTypeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8           devNum;
    GT_U32          index;
    GT_U32          ethType;
    GT_STATUS       result;

    devNum   = (GT_U8) inArgs[0];
    index    = (GT_U32)inArgs[1];

    result = cpssDxChPclUdeEtherTypeGet(
        devNum, index, &ethType);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    galtisOutput(
        outArgs, GT_OK, "%d", ethType);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclEgressKeyFieldsVidUpModeSet function
* @endinternal
*
* @brief   Sets Egress Policy VID and UP key fields content mode
*
* @note   APPLICABLE DEVICES:      Lion and above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChPclEgressKeyFieldsVidUpModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                      devNum;
    CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_MODE_ENT   vidUpMode;
    GT_STATUS                                  result;

    devNum    = (GT_U8) inArgs[0];
    vidUpMode = (CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_MODE_ENT)inArgs[1];

    result = pg_wrap_cpssDxChPclEgressKeyFieldsVidUpModeSet(
        devNum, vidUpMode);

    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclEgressKeyFieldsVidUpModeGet function
* @endinternal
*
* @brief   Gets Egress Policy VID and UP key fields content mode
*
* @note   APPLICABLE DEVICES:      Lion and above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChPclEgressKeyFieldsVidUpModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                      devNum;
    CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_MODE_ENT   vidUpMode;
    GT_STATUS                                  result;

    devNum = (GT_U8)inArgs[0];

    result = pg_wrap_cpssDxChPclEgressKeyFieldsVidUpModeGet(
        devNum, &vidUpMode);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    galtisOutput(
        outArgs, GT_OK, "%d", vidUpMode);

    return CMD_OK;
}

/*******************************************************************************
* cpssDxChPclRuleAnyStateGet
*
* DESCRIPTION:
*       Get state (valid or not) of the rule and it's size
*
* APPLICABLE DEVICES:  All DxCh Devices
*
* INPUTS:
*       devNum         - device number
*       ruleSize       - size of rule
*       ruleIndex      - index of rule
* OUTPUTS:
*       validPtr       -  rule's validity
*                         GT_TRUE  - rule valid
*                         GT_FALSE - rule invalid
*       ruleSizePtr    -  rule's size
*
* RETURNS :
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong parameter
*       GT_BAD_PTR               - null pointer
*       GT_BAD_STATE             - For DxCh3 and above if cannot determinate
*                                  the rule size by found X/Y bits of compare mode
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_HW_ERROR              - on hardware error
*       GT_FAIL                  - otherwise
*
* COMMENTS:
*
*******************************************************************************/
/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPclRuleAnyStateGet
(
    IN  GT_U8                         devNum,
    IN CPSS_PCL_RULE_SIZE_ENT         ruleSize,
    IN  GT_U32                        ruleIndex,
    OUT GT_BOOL                       *validPtr,
    OUT CPSS_PCL_RULE_SIZE_ENT        *ruleSizePtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */
    CPSS_DXCH_TCAM_RULE_SIZE_ENT tcamRuleSize;

    pclMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    /* convert pcl rule's size to tcam rule's size */
    PRV_CPSS_DXCH_PCL_CONVERT_RULE_SIZE_TO_TCAM_RULE_SIZE_VAL_MAC(tcamRuleSize, CPSS_PCL_RULE_SIZE_STD_E);

    ruleIndex = appDemoDxChTcamPclConvertedIndexGet(devNum,ruleIndex,tcamRuleSize);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChPclRuleAnyStateGet(
            devNum, ruleSize, ruleIndex, validPtr, ruleSizePtr);
    }
    else
    {
        return cpssDxChPclPortGroupRuleAnyStateGet(
            devNum, pgBmp, ruleSize, ruleIndex, validPtr, ruleSizePtr);
    }
}

/**
* @internal wrCpssDxChPclRuleAnyStateGet function
* @endinternal
*
* @brief   Get state (valid or not) of the rule and it's size
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_BAD_STATE             - For DxCh3 and above if cannot determinate
*                                       the rule size by found X/Y bits of compare mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChPclRuleAnyStateGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32      numFields,
    OUT GT_8       outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_PCL_RULE_SIZE_ENT ruleSizeIn;
    GT_U32 ruleIndex;
    GT_BOOL valid;
    CPSS_PCL_RULE_SIZE_ENT ruleSize;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ruleSizeIn = (CPSS_PCL_RULE_SIZE_ENT)inArgs[1];
    ruleIndex = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = pg_wrap_cpssDxChPclRuleAnyStateGet(devNum, ruleSizeIn, ruleIndex, &valid, &ruleSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", valid, ruleSize);

    return CMD_OK;
}

/**
* @internal wrUtilCpssDxChPclRuleIncrementalSequenceSet function
* @endinternal
*
* @brief   Writes sequence of rules.
*         The Rules calculated by incremental modification of last written rule
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note Only network ordered byte array supported
*       Example (start from 1-th element, carry of overflow bit)
*       checkRuleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E
*       field_full_name = macDa
*       elementStart  = 1
*       increment    = 0x01020080
*       MAC_DA states   00:00:00:00:00:00
*       00:01:02:00:80:00
*       00:02:04:01:00:00
*       00:03:06:01:80:00
*       00:04:08:02:00:00
*
*/
static CMD_STATUS wrUtilCpssDxChPclRuleIncrementalSequenceSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          rc;
    GT_U8                              devNum;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT checkRuleFormat;
    GT_U32                             ruleIndexBase;
    GT_U32                             ruleIndexIncrement;
    GT_U32                             rulesAmount;
    char                               *field_full_name;
    GT_U32                             elementStart;
    GT_U32                             increment;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum                = (GT_U8) inArgs[0];
    checkRuleFormat       = (CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT)inArgs[1];
    ruleIndexBase         = (GT_U32)inArgs[2];
    ruleIndexIncrement    = (GT_U32)inArgs[3];
    rulesAmount           = (GT_U32)inArgs[4];
    field_full_name       = (char*) inArgs[5];
    elementStart          = (GT_U32)inArgs[6];
    increment             = (GT_U32)inArgs[7];

    rc = utilCpssDxChPclRuleIncrementalSequenceSet(
        devNum, checkRuleFormat,
        ruleIndexBase, ruleIndexIncrement, rulesAmount,
        field_full_name, elementStart,  increment);

    galtisOutput(outArgs, rc, "");
    return GT_OK;
}

/**
* @internal wrCpssDxChPclL3L4ParsingOverMplsEnableSet function
* @endinternal
*
* @brief   If enabled, non-Tunnel-Terminated IPvx over MPLS packets are treated
*         by Ingress PCL as IP packets for key selection and UDB usage.
*         If disabled, non-Tunnel-Terminated IPvx over MPLS packets are treated
*         by Ingress PCL as MPLS packets for key selection and UDB usage.
*
* @note   APPLICABLE DEVICES:      All DxChXcat and above devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclL3L4ParsingOverMplsEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPclL3L4ParsingOverMplsEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclL3L4ParsingOverMplsEnableGet function
* @endinternal
*
* @brief   If enabled, non-Tunnel-Terminated IPvx over MPLS packets are treated
*         by Ingress PCL as IP packets for key selection and UDB usage.
*         If disabled, non-Tunnel-Terminated IPvx over MPLS packets are treated
*         by Ingress PCL as MPLS packets for key selection and UDB usage.
*
* @note   APPLICABLE DEVICES:      All DxChXcat and above devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclL3L4ParsingOverMplsEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPclL3L4ParsingOverMplsEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclLookup0ForRoutedPacketsEnableSet function
* @endinternal
*
* @brief   Enables/disables PCL lookup0 for routed packets.
*         For not routed packets lookup0 always enable.
*
* @note   APPLICABLE DEVICES:      All DxCh3 devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChPclLookup0ForRoutedPacketsEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_NUM portNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPclLookup0ForRoutedPacketsEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclLookup0ForRoutedPacketsEnableGet function
* @endinternal
*
* @brief   Gets Enable/disable status of PCL lookup0 for routed packets.
*         For not routed packets lookup0 always enable.
*
* @note   APPLICABLE DEVICES:      All DxCh3 devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
*/
static CMD_STATUS wrCpssDxChPclLookup0ForRoutedPacketsEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_NUM portNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPclLookup0ForRoutedPacketsEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclLookup1ForNotRoutedPacketsEnableSet function
* @endinternal
*
* @brief   Enables/disables PCL lookup1 for not routed packets.
*         For routed packets lookup1 always enable.
*
* @note   APPLICABLE DEVICES:      All DxCh3 devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChPclLookup1ForNotRoutedPacketsEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPclLookup1ForNotRoutedPacketsEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclLookup1ForNotRoutedPacketsEnableGet function
* @endinternal
*
* @brief   Gets Enable/disable status of PCL lookup1 for not routed packets.
*         For routed packets lookup1 always enable.
*
* @note   APPLICABLE DEVICES:      All DxCh3 devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
*/
static CMD_STATUS wrCpssDxChPclLookup1ForNotRoutedPacketsEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPclLookup1ForNotRoutedPacketsEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclTcamEccDaemonEnableSet function
* @endinternal
*
* @brief   Enables or disables PCL TCAM ECC Daemon.
*
* @note   APPLICABLE DEVICES:      All DxCh3 devices.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChPclTcamEccDaemonEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enalbe;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enalbe = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPclTcamEccDaemonEnableSet(devNum, enalbe);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclTcamEccDaemonEnableGet function
* @endinternal
*
* @brief   Gets status (Enabled/Disabled) of PCL TCAM ECC Daemon.
*
* @note   APPLICABLE DEVICES:      All DxCh3 devices.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChPclTcamEccDaemonEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enalbe;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPclTcamEccDaemonEnableGet(devNum, &enalbe);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enalbe);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclIpMinOffsetSet function
* @endinternal
*
* @brief   Set the IP Minimum Offset parameter.
*         If packet is IPV4 fragment and has non zero offset
*         and it's offset less then IP Minimum Offset
*         the packet is counted as small offset.
*         The ipHeaderInfo field in PCL search key is set to be 3 i.e. "Ip Small Offset".
*
* @note   APPLICABLE DEVICES:      All DxChXcat and above devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range minIpOffset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclIpMinOffsetSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 ipMinOffset;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ipMinOffset = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPclIpMinOffsetSet(devNum, ipMinOffset);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclIpMinOffsetGet function
* @endinternal
*
* @brief   Get the IP Minimum Offset parameter.
*         If packet is IPV4 fragment and has non zero offset
*         and it's offset less then IP Minimum Offset
*         the packet is counted as small offset.
*         The ipHeaderInfo field in PCL search key is set to be 3 i.e. "Ip Small Offset".
*
* @note   APPLICABLE DEVICES:      All DxChXcat and above devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range minIpOffset
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclIpMinOffsetGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 ipMinOffset;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPclIpMinOffsetGet(devNum, &ipMinOffset);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", ipMinOffset);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclPortIngressLookup0Sublookup1TypeSet function
* @endinternal
*
* @brief   Configures Ingress PCL Lookup0 sublookup1 type.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above Devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclPortIngressLookup0Sublookup1TypeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                         devNum;
    CPSS_DXCH_PCL_IPCL01_LOOKUP_TYPE_ENT     lookupType;
    GT_STATUS                                     result;

    devNum     = (GT_U8)inArgs[0];
    lookupType = (CPSS_DXCH_PCL_IPCL01_LOOKUP_TYPE_ENT)inArgs[1];

    result = cpssDxChPclPortIngressLookup0Sublookup1TypeSet(
        devNum, lookupType);

    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclPortIngressLookup0Sublookup1TypeGet function
* @endinternal
*
* @brief   Configures Ingress PCL Lookup0 sublookup1 type.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above Devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclPortIngressLookup0Sublookup1TypeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                         devNum;
    CPSS_DXCH_PCL_IPCL01_LOOKUP_TYPE_ENT          lookupType;
    GT_STATUS                                     result;

    devNum     = (GT_U8)inArgs[0];
    lookupType = CPSS_DXCH_PCL_IPCL01_LOOKUP_TYPE_PARTIAL_E;

    result = cpssDxChPclPortIngressLookup0Sublookup1TypeGet(
        devNum, &lookupType);

    galtisOutput(outArgs, result, "%d", lookupType);

    return CMD_OK;
}

/*********************************************************************************/

/* only words 7..8 should be used */
/* use GT_32 to be with same type as --> GT_32 inFields[CMD_MAX_FIELDS] */
static GT_UINTPTR   maskDataWords[10];

#define FIRST_NEW_FORMAT_CNS CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E

static void epclCommonEgrPktTrunkFieldSet
(
    INOUT CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC *commonPtr,
    IN GT_UINTPTR   fieldsArray[]/* relevant fields are 7..8*/
)
{
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC *patternCommonPtr =
        &patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon;/* all egress rules starts with the 'ingressIpCommon'*/
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC *maskCommonPtr =
        &maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon;/* all egress rules starts with the 'ingressIpCommon'*/
    GT_TRUNK_ID trunkId;

    commonPtr->isSrcTrunk       = (GT_U8) fieldsArray[7];
    commonPtr->srcDevOrTrunkId  = (GT_U8) fieldsArray[8];

    trunkId = (GT_TRUNK_ID)commonPtr->srcDevOrTrunkId;
    if(commonPtr == maskCommonPtr)
    {
        /* we do the mask now */
        gtPclMaskTrunkConvertFromTestToCpss(
            maskCommonPtr->isSrcTrunk,
            patternCommonPtr->isSrcTrunk,
            &trunkId);
    }
    else
    {
        /* we do the pattern now */
        gtPclPatternTrunkConvertFromTestToCpss(
            maskCommonPtr->isSrcTrunk,
            patternCommonPtr->isSrcTrunk,
            &trunkId);
    }

    commonPtr->srcDevOrTrunkId = (GT_U8)trunkId;

    return;
}

static CMD_STATUS wrCpssDxChPclIngressRule_IPCL1_IPV4_E_Write
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_BYTE_ARRY    maskBArr, patternBArr;
    GT_U8           devNum;

    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E;


    /* map input arguments to locals */
    /*
       inFields[1] = mask/pattern indication
       mask = 0
       pattern = 1
    */
    devNum=(GT_U8)inArgs[0];

    if(inFields[1] == 0) /* mask */
    {
    mask_ruleIndex = (GT_U32)inFields[0];
    maskData.ruleStdIpv4RoutedAclQos.ingressIpCommon.pclId= (GT_U16)inFields[2];
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&maskData.ruleStdIpv4RoutedAclQos.ingressIpCommon.portListBmp);
    maskData.ruleStdIpv4RoutedAclQos.ingressIpCommon.macToMe=(GT_U8)inFields[3];
    maskData.ruleStdIpv4RoutedAclQos.ingressIpCommon.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[4];

    maskData.ruleStdIpv4RoutedAclQos.ingressIpCommon.isTagged = (GT_U8)inFields[5];
    maskData.ruleStdIpv4RoutedAclQos.ingressIpCommon.vid = (GT_U16)inFields[6];
    maskData.ruleStdIpv4RoutedAclQos.ingressIpCommon.up = (GT_U8)inFields[7];
    maskData.ruleStdIpv4RoutedAclQos.ingressIpCommon.tos= (GT_U8)inFields[8];
    maskData.ruleStdIpv4RoutedAclQos.ingressIpCommon.ipProtocol=(GT_U8)inFields[9];
    maskData.ruleStdIpv4RoutedAclQos.pktType=(GT_U8)inFields[10];
    maskData.ruleStdIpv4RoutedAclQos.ipFragmented=(GT_U8)inFields[11];
    maskData.ruleStdIpv4RoutedAclQos.ipHeaderInfo=(GT_U8)inFields[12];
    maskData.ruleStdIpv4RoutedAclQos.ipPacketLength=(GT_U16)inFields[13];
    maskData.ruleStdIpv4RoutedAclQos.ttl=(GT_U8)inFields[14];
    galtisIpAddr(&maskData.ruleStdIpv4RoutedAclQos.sip,(GT_U8*)inFields[15]);
    galtisIpAddr(&maskData.ruleStdIpv4RoutedAclQos.dip,(GT_U8*)inFields[16]);
    maskData.ruleStdIpv4RoutedAclQos.ingressIpCommon.isL4Valid=(GT_U8)inFields[17];
    maskData.ruleStdIpv4RoutedAclQos.ingressIpCommon.l4Byte0=(GT_U8)inFields[18];
    maskData.ruleStdIpv4RoutedAclQos.ingressIpCommon.l4Byte1=(GT_U8)inFields[19];
    maskData.ruleStdIpv4RoutedAclQos.ingressIpCommon.l4Byte2=(GT_U8)inFields[20];
    maskData.ruleStdIpv4RoutedAclQos.ingressIpCommon.l4Byte3=(GT_U8)inFields[21];
    maskData.ruleStdIpv4RoutedAclQos.ingressIpCommon.l4Byte13=(GT_U8)inFields[22];
    /* tcpUdpPortComparators - CH3 only */
    maskData.ruleStdIpv4RoutedAclQos.tcpUdpPortComparators =
        (GT_U8)inFields[23];
    /* udb5 - XCat_A1 only */
    maskData.ruleStdIpv4RoutedAclQos.udb5 = (GT_U8)
        maskData.ruleStdIpv4RoutedAclQos.tcpUdpPortComparators;

    if( GT_TRUE == sip5Pcl )
    {
        maskData.ruleStdIpv4RoutedAclQos.ingressIpCommon.sourceDevice = (GT_U16)inFields[24];
        maskData.ruleStdIpv4RoutedAclQos.vrfId = (GT_U16)inFields[25];
        maskData.ruleStdIpv4RoutedAclQos.isUdbValid = (GT_U8)inFields[26];
        galtisBArray(&maskBArr,(GT_U8*)inFields[27]);

        cmdOsMemCpy(
            maskData.ruleStdIpv4RoutedAclQos.udb41_44, maskBArr.data,
            maskBArr.length);
    }

    mask_set = GT_TRUE;
    }
    else /* pattern */
    {
        pattern_ruleIndex = (GT_U32)inFields[0];
        patternData.ruleStdIpv4RoutedAclQos.ingressIpCommon.pclId= (GT_U16)inFields[2];
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&patternData.ruleStdIpv4RoutedAclQos.ingressIpCommon.portListBmp);
        patternData.ruleStdIpv4RoutedAclQos.ingressIpCommon.macToMe=(GT_U8)inFields[3];
        patternData.ruleStdIpv4RoutedAclQos.ingressIpCommon.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[4];

        patternData.ruleStdIpv4RoutedAclQos.ingressIpCommon.isTagged = (GT_U8)inFields[5];
        patternData.ruleStdIpv4RoutedAclQos.ingressIpCommon.vid = (GT_U16)inFields[6];
        patternData.ruleStdIpv4RoutedAclQos.ingressIpCommon.up = (GT_U8)inFields[7];
        patternData.ruleStdIpv4RoutedAclQos.ingressIpCommon.tos= (GT_U8)inFields[8];
        patternData.ruleStdIpv4RoutedAclQos.ingressIpCommon.ipProtocol=(GT_U8)inFields[9];
        patternData.ruleStdIpv4RoutedAclQos.pktType=(GT_U8)inFields[10];
        patternData.ruleStdIpv4RoutedAclQos.ipFragmented=(GT_U8)inFields[11];
        patternData.ruleStdIpv4RoutedAclQos.ipHeaderInfo=(GT_U8)inFields[12];
        patternData.ruleStdIpv4RoutedAclQos.ipPacketLength=(GT_U16)inFields[13];
        patternData.ruleStdIpv4RoutedAclQos.ttl=(GT_U8)inFields[14];
        galtisIpAddr(&patternData.ruleStdIpv4RoutedAclQos.sip,(GT_U8*)inFields[15]);
        galtisIpAddr(&patternData.ruleStdIpv4RoutedAclQos.dip,(GT_U8*)inFields[16]);
        patternData.ruleStdIpv4RoutedAclQos.ingressIpCommon.isL4Valid=(GT_U8)inFields[17];
        patternData.ruleStdIpv4RoutedAclQos.ingressIpCommon.l4Byte0=(GT_U8)inFields[18];
        patternData.ruleStdIpv4RoutedAclQos.ingressIpCommon.l4Byte1=(GT_U8)inFields[19];
        patternData.ruleStdIpv4RoutedAclQos.ingressIpCommon.l4Byte2=(GT_U8)inFields[20];
        patternData.ruleStdIpv4RoutedAclQos.ingressIpCommon.l4Byte3=(GT_U8)inFields[21];
        patternData.ruleStdIpv4RoutedAclQos.ingressIpCommon.l4Byte13=(GT_U8)inFields[22];

        /* tcpUdpPortComparators - CH3 only */
        patternData.ruleStdIpv4RoutedAclQos.tcpUdpPortComparators =
            (GT_U8)inFields[23];
        /* udb5 - XCat_A1 only */
        patternData.ruleStdIpv4RoutedAclQos.udb5 = (GT_U8)
            patternData.ruleStdIpv4RoutedAclQos.tcpUdpPortComparators;

        if( GT_TRUE == sip5Pcl )
        {
            patternData.ruleStdIpv4RoutedAclQos.ingressIpCommon.sourceDevice = (GT_U16)inFields[24];
            patternData.ruleStdIpv4RoutedAclQos.vrfId = (GT_U16)inFields[25];
            patternData.ruleStdIpv4RoutedAclQos.isUdbValid = (GT_U8)inFields[26];
            galtisBArray(&patternBArr,(GT_U8*)inFields[27]);

            cmdOsMemCpy(
                patternData.ruleStdIpv4RoutedAclQos.udb41_44, patternBArr.data,
                patternBArr.length);
        }

        pattern_set = GT_TRUE;
    }

    if ((mask_set == GT_TRUE) && (pattern_set == GT_TRUE))
    {
        PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleStdIpv4RoutedAclQos.ingressIpCommon.sourcePort);
    }

    return CMD_OK;

}
/***********************************************************************************/
static CMD_STATUS wrCpssDxChPclIngressRule_EXT_IPCL0_IPV4_Write
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8           devNum;
    GT_BYTE_ARRY   maskBL2Qos, maskBIpV4L4,maskBIpv6L2,maskBIpv6L4;
    GT_BYTE_ARRY   patternBL2Qos, patternBIpV4L4,patternBIpv6L2,patternBIpv6L4;


    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E;


    /* map input arguments to locals */
    /*
       inFields[1] = mask/pattern indication
       mask = 0
       pattern = 1
    */
    devNum=(GT_U8)inArgs[0];

    if(inFields[1] == 0) /* mask */
    {
        mask_ruleIndex = (GT_U32)inFields[0];

        maskData.ruleExtIpv4PortVlanQos.ingressIpCommon.pclId= (GT_U16)inFields[2];
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&maskData.ruleExtIpv4PortVlanQos.ingressIpCommon.portListBmp);
        maskData.ruleExtIpv4PortVlanQos.ingressIpCommon.macToMe=(GT_U8)inFields[3];
        maskData.ruleExtIpv4PortVlanQos.ingressIpCommon.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[4];

        maskData.ruleExtIpv4PortVlanQos.ingressIpCommon.isTagged = (GT_U8)inFields[5];
        maskData.ruleExtIpv4PortVlanQos.ingressIpCommon.vid = (GT_U16)inFields[6];
        maskData.ruleExtIpv4PortVlanQos.ingressIpCommon.up = (GT_U8)inFields[7];
        maskData.ruleExtIpv4PortVlanQos.ingressIpCommon.tos= (GT_U8)inFields[8];
        maskData.ruleExtIpv4PortVlanQos.ingressIpCommon.ipProtocol=(GT_U8)inFields[9];

        maskData.ruleExtIpv4PortVlanQos.isL2Valid=(GT_U8)inFields[10];
        maskData.ruleExtIpv4PortVlanQos.isBc=(GT_U8)inFields[11];
        maskData.ruleExtIpv4PortVlanQos.isIp=(GT_U8)inFields[12];
        maskData.ruleExtIpv4PortVlanQos.isArp=(GT_U8)inFields[13];
        maskData.ruleExtIpv4PortVlanQos.l2Encap=(GT_U8)inFields[14];
        maskData.ruleExtIpv4PortVlanQos.etherType=(GT_U16)inFields[15];
        galtisMacAddr(&maskData.ruleExtIpv4PortVlanQos.macDa,(GT_U8*)inFields[16]);
        galtisMacAddr(&maskData.ruleExtIpv4PortVlanQos.macSa,(GT_U8*)inFields[17]);
        maskData.ruleExtIpv4PortVlanQos.ipFragmented=(GT_U8)inFields[18];
        maskData.ruleExtIpv4PortVlanQos.ipHeaderInfo=(GT_U8)inFields[19];
        galtisIpAddr(&maskData.ruleExtIpv4PortVlanQos.sip,(GT_U8*)inFields[20]);
        galtisIpAddr(&maskData.ruleExtIpv4PortVlanQos.dip,(GT_U8*)inFields[21]);
        maskData.ruleExtIpv4PortVlanQos.ingressIpCommon.l4Byte0=(GT_U8)inFields[22];
        maskData.ruleExtIpv4PortVlanQos.ingressIpCommon.l4Byte1=(GT_U8)inFields[23];
        maskData.ruleExtIpv4PortVlanQos.ingressIpCommon.l4Byte2=(GT_U8)inFields[24];
        maskData.ruleExtIpv4PortVlanQos.ingressIpCommon.l4Byte3=(GT_U8)inFields[25];
        maskData.ruleExtIpv4PortVlanQos.ingressIpCommon.l4Byte13=(GT_U8)inFields[26];
        maskData.ruleExtIpv4PortVlanQos.isUdbValid=(GT_U8)inFields[27];

        if( GT_FALSE == sip5Pcl )
        {
        galtisBArray(&maskBL2Qos,(GT_U8*)inFields[28]);
        galtisBArray(&maskBIpV4L4,(GT_U8*)inFields[29]);
        galtisBArray(&maskBIpv6L2,(GT_U8*)inFields[30]);
        galtisBArray(&maskBIpv6L4,(GT_U8*)inFields[31]);

        if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            CHECK_BYTE_ARRAY_LENGTH_MAC(maskData.ruleExtIpv4PortVlanQos.UdbStdIpL2Qos,
                                        maskBL2Qos);
            cmdOsMemCpy(
                    maskData.ruleExtIpv4PortVlanQos.UdbStdIpL2Qos, maskBL2Qos.data,
                    maskBL2Qos.length);

            CHECK_BYTE_ARRAY_LENGTH_MAC(maskData.ruleExtIpv4PortVlanQos.UdbStdIpV4L4,
                                maskBIpV4L4);
            cmdOsMemCpy(
                    maskData.ruleExtIpv4PortVlanQos.UdbStdIpV4L4, maskBIpV4L4.data,
                    maskBIpV4L4.length);

            CHECK_BYTE_ARRAY_LENGTH_MAC(maskData.ruleExtIpv4PortVlanQos.UdbExtIpv6L2,
                                maskBIpv6L2);
            cmdOsMemCpy(
                    maskData.ruleExtIpv4PortVlanQos.UdbExtIpv6L2, maskBIpv6L2.data,
                    maskBIpv6L2.length);

            CHECK_BYTE_ARRAY_LENGTH_MAC(maskData.ruleExtIpv4PortVlanQos.UdbExtIpv6L4,
                                maskBIpv6L4);
            cmdOsMemCpy(
                    maskData.ruleExtIpv4PortVlanQos.UdbExtIpv6L4, maskBIpv6L4.data,
                    maskBIpv6L4.length);
        }
        else
        {
            /*xcat a1 and above*/
            CHECK_BYTE_ARRAY_LENGTH_MAC(maskData.ruleExtIpv4PortVlanQos.udb5_16,
                                maskBL2Qos);
            cmdOsMemCpy(
                    maskData.ruleExtIpv4PortVlanQos.udb5_16,
                    maskBL2Qos.data,/* take all 12 UDBs from the first
                                       UDB field of the wrapper rule */
                    maskBL2Qos.length);
        }
        }
        else /* GT_TRUE == sip5Pcl */
        {
            maskData.ruleExtIpv4PortVlanQos.ingressIpCommon.isL4Valid = (GT_U8)inFields[28];
            maskData.ruleExtIpv4PortVlanQos.ingressIpCommon.sourceDevice = (GT_U16)inFields[29];
            maskData.ruleExtIpv4PortVlanQos.tag1Exist = (GT_U8)inFields[30];
            maskData.ruleExtIpv4PortVlanQos.vid1 = (GT_U16)inFields[31];
            maskData.ruleExtIpv4PortVlanQos.cfi1 = (GT_U8)inFields[32];
            maskData.ruleExtIpv4PortVlanQos.up1 = (GT_U8)inFields[33];
            maskData.ruleExtIpv4PortVlanQos.vrfId = (GT_U16)inFields[34];
            maskData.ruleExtIpv4PortVlanQos.trunkHash = (GT_U8)inFields[35];
            galtisBArray(&maskBL2Qos,(GT_U8*)inFields[36]);

            cmdOsMemCpy(
                maskData.ruleExtIpv4PortVlanQos.udb5_16, maskBL2Qos.data,
                maskBL2Qos.length);

            galtisBArray(&maskBL2Qos,(GT_U8*)inFields[37]);

            cmdOsMemCpy(
                maskData.ruleExtIpv4PortVlanQos.udb31_38, maskBL2Qos.data,
                maskBL2Qos.length);
        }

        mask_set = GT_TRUE;
    }
    else /* pattern */
    {
        pattern_ruleIndex = (GT_U32)inFields[0];

        patternData.ruleExtIpv4PortVlanQos.ingressIpCommon.pclId= (GT_U16)inFields[2];
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&patternData.ruleExtIpv4PortVlanQos.ingressIpCommon.portListBmp);
        patternData.ruleExtIpv4PortVlanQos.ingressIpCommon.macToMe=(GT_U8)inFields[3];
        patternData.ruleExtIpv4PortVlanQos.ingressIpCommon.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[4];

        patternData.ruleExtIpv4PortVlanQos.ingressIpCommon.isTagged = (GT_U8)inFields[5];
        patternData.ruleExtIpv4PortVlanQos.ingressIpCommon.vid = (GT_U16)inFields[6];
        patternData.ruleExtIpv4PortVlanQos.ingressIpCommon.up = (GT_U8)inFields[7];
        patternData.ruleExtIpv4PortVlanQos.ingressIpCommon.tos= (GT_U8)inFields[8];
        patternData.ruleExtIpv4PortVlanQos.ingressIpCommon.ipProtocol=(GT_U8)inFields[9];

        patternData.ruleExtIpv4PortVlanQos.isL2Valid=(GT_U8)inFields[10];
        patternData.ruleExtIpv4PortVlanQos.isBc=(GT_U8)inFields[11];
        patternData.ruleExtIpv4PortVlanQos.isIp=(GT_U8)inFields[12];
        patternData.ruleExtIpv4PortVlanQos.isArp=(GT_U8)inFields[13];
        patternData.ruleExtIpv4PortVlanQos.l2Encap=(GT_U8)inFields[14];
        patternData.ruleExtIpv4PortVlanQos.etherType=(GT_U16)inFields[15];
        galtisMacAddr(&patternData.ruleExtIpv4PortVlanQos.macDa,(GT_U8*)inFields[16]);
        galtisMacAddr(&patternData.ruleExtIpv4PortVlanQos.macSa,(GT_U8*)inFields[17]);
        patternData.ruleExtIpv4PortVlanQos.ipFragmented=(GT_U8)inFields[18];
        patternData.ruleExtIpv4PortVlanQos.ipHeaderInfo=(GT_U8)inFields[19];
        galtisIpAddr(&patternData.ruleExtIpv4PortVlanQos.sip,(GT_U8*)inFields[20]);
        galtisIpAddr(&patternData.ruleExtIpv4PortVlanQos.dip,(GT_U8*)inFields[21]);
        patternData.ruleExtIpv4PortVlanQos.ingressIpCommon.l4Byte0=(GT_U8)inFields[22];
        patternData.ruleExtIpv4PortVlanQos.ingressIpCommon.l4Byte1=(GT_U8)inFields[23];
        patternData.ruleExtIpv4PortVlanQos.ingressIpCommon.l4Byte2=(GT_U8)inFields[24];
        patternData.ruleExtIpv4PortVlanQos.ingressIpCommon.l4Byte3=(GT_U8)inFields[25];
        patternData.ruleExtIpv4PortVlanQos.ingressIpCommon.l4Byte13=(GT_U8)inFields[26];
        patternData.ruleExtIpv4PortVlanQos.isUdbValid=(GT_U8)inFields[27];

        if( GT_FALSE == sip5Pcl )
        {
        galtisBArray(&patternBL2Qos,(GT_U8*)inFields[28]);
        galtisBArray(&patternBIpV4L4,(GT_U8*)inFields[29]);
        galtisBArray(&patternBIpv6L2,(GT_U8*)inFields[30]);
        galtisBArray(&patternBIpv6L4,(GT_U8*)inFields[31]);

        if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            CHECK_BYTE_ARRAY_LENGTH_MAC(patternData.ruleExtIpv4PortVlanQos.UdbStdIpL2Qos,
                                patternBL2Qos);
            cmdOsMemCpy(
                    patternData.ruleExtIpv4PortVlanQos.UdbStdIpL2Qos, patternBL2Qos.data,
                    patternBL2Qos.length);

            CHECK_BYTE_ARRAY_LENGTH_MAC(patternData.ruleExtIpv4PortVlanQos.UdbStdIpV4L4,
                                patternBIpV4L4);
            cmdOsMemCpy(
                    patternData.ruleExtIpv4PortVlanQos.UdbStdIpV4L4, patternBIpV4L4.data,
                    patternBIpV4L4.length);

            CHECK_BYTE_ARRAY_LENGTH_MAC(patternData.ruleExtIpv4PortVlanQos.UdbExtIpv6L2,
                                patternBIpv6L2);
            cmdOsMemCpy(
                    patternData.ruleExtIpv4PortVlanQos.UdbExtIpv6L2, patternBIpv6L2.data,
                    patternBIpv6L2.length);

            CHECK_BYTE_ARRAY_LENGTH_MAC(patternData.ruleExtIpv4PortVlanQos.UdbExtIpv6L4,
                                patternBIpv6L4);
            cmdOsMemCpy(
                    patternData.ruleExtIpv4PortVlanQos.UdbExtIpv6L4, patternBIpv6L4.data,
                    patternBIpv6L4.length);
        }
        else
        {
            /*xcat and above*/
            CHECK_BYTE_ARRAY_LENGTH_MAC(patternData.ruleExtIpv4PortVlanQos.udb5_16,
                                patternBL2Qos);
            cmdOsMemCpy(
                    patternData.ruleExtIpv4PortVlanQos.udb5_16,
                    patternBL2Qos.data,/* take all 12 UDBs from the first
                                       UDB field of the wrapper rule */
                    patternBL2Qos.length);
        }
        }
        else /* GT_TRUE == sip5Pcl */
        {
            patternData.ruleExtIpv4PortVlanQos.ingressIpCommon.isL4Valid = (GT_U8)inFields[28];
            patternData.ruleExtIpv4PortVlanQos.ingressIpCommon.sourceDevice = (GT_U16)inFields[29];
            patternData.ruleExtIpv4PortVlanQos.tag1Exist = (GT_U8)inFields[30];
            patternData.ruleExtIpv4PortVlanQos.vid1 = (GT_U16)inFields[31];
            patternData.ruleExtIpv4PortVlanQos.cfi1 = (GT_U8)inFields[32];
            patternData.ruleExtIpv4PortVlanQos.up1 = (GT_U8)inFields[33];
            patternData.ruleExtIpv4PortVlanQos.vrfId = (GT_U16)inFields[34];
            patternData.ruleExtIpv4PortVlanQos.trunkHash = (GT_U8)inFields[35];
            galtisBArray(&patternBL2Qos,(GT_U8*)inFields[36]);

            cmdOsMemCpy(
                patternData.ruleExtIpv4PortVlanQos.udb5_16, patternBL2Qos.data,
                patternBL2Qos.length);

            galtisBArray(&patternBL2Qos,(GT_U8*)inFields[37]);

            cmdOsMemCpy(
                patternData.ruleExtIpv4PortVlanQos.udb31_38, patternBL2Qos.data,
                patternBL2Qos.length);
        }

        pattern_set = GT_TRUE;
    }

    if ((mask_set == GT_TRUE) && (pattern_set == GT_TRUE))
    {
        PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleExtIpv4PortVlanQos.ingressIpCommon.sourcePort);
    }

    return CMD_OK;

}
/*********************************************************************************/
static CMD_STATUS wrCpssDxChPclEgressRule_EXT_NOT_IPV6_Write
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_BYTE_ARRY    maskBArr, patternBArr;
    GT_U8           devNum;
    GT_U32          ii;

    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E;


    /* map input arguments to locals */
    /*
       inFields[1] = mask/pattern indication
       mask = 0
       pattern = 1
    */
    devNum=(GT_U8)inArgs[0];

    if(inFields[1] == 0) /* mask */
    {
    mask_ruleIndex = (GT_U32)inFields[0];
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.pclId= (GT_U16)inFields[2];
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.sourcePort= (GT_PHYSICAL_PORT_NUM)inFields[3];

    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.qosProfile= (GT_U8)inFields[4];
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.isL2Valid=(GT_U8)inFields[5];
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.originalVid=(GT_U16)inFields[6];
    if(pattern_set==GT_TRUE)
    {
        epclCommonEgrPktTrunkFieldSet(&maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon,
            inFields);
    }
    else
    {
        /* save the info from mask and set it properly when the pattern is set */
        for(ii = 7 ; ii <= 8 ; ii++)
        {
            maskDataWords[ii] = inFields[ii];
        }
    }
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.isIp=(GT_U8)inFields[9];
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.isArp=(GT_U8)inFields[10];
    galtisMacAddr(&maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.macDa,(GT_U8*)inFields[11]);
    galtisMacAddr(&maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.macSa,(GT_U8*)inFields[12]);
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.tos=(GT_U8)inFields[13];
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.ipProtocol=(GT_U8)inFields[14];
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.ttl=(GT_U8)inFields[15];
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.isL4Valid=(GT_U8)inFields[16];
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.l4Byte0=(GT_U8)inFields[17];
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.l4Byte1=(GT_U8)inFields[18];
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.l4Byte2=(GT_U8)inFields[19];
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.l4Byte3=(GT_U8)inFields[20];
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.l4Byte13=(GT_U8)inFields[21];
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.tcpUdpPortComparators=(GT_U8)inFields[22];
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.tc=(GT_U8)inFields[23];
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.dp=(GT_U8)inFields[24];
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.egrPacketType=(GT_U8)inFields[25];
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.srcTrgOrTxMirror=(GT_U8)inFields[26];
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.assignedUp=(GT_U8)inFields[27];
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.rxSniff=(GT_U8)inFields[28];
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.isRouted=(GT_U8)inFields[29];
    maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.isIpv6=(GT_U8)inFields[30];
    galtisIpAddr(&maskData.ruleEgrExtIpv4RaclVacl.sip,(GT_U8*)inFields[31]);
    galtisIpAddr(&maskData.ruleEgrExtIpv4RaclVacl.dip,(GT_U8*)inFields[32]);
    maskData.ruleEgrExtIpv4RaclVacl.ipv4Options=(GT_U8)inFields[33];
    /*add on for extended pcl table*/
    if(GT_TRUE == extPcl)
       maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.trgPhysicalPort =(GT_U8)inFields[34];

    if(GT_TRUE == sip5Pcl)
    {
        maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.cpuCode = (GT_U8)inFields[35];
        maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.srcHwDev = (GT_U8)inFields[36];
        maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.srcTrg = (GT_U8)inFields[37];
        maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.egrFilterEnable = (GT_U8)inFields[38];
        maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.sourceId = (GT_U16)inFields[39];
        maskData.ruleEgrExtIpv4RaclVacl.isVidx = (GT_U8)inFields[40];
        maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon.tag1Exist = (GT_U8)inFields[41];
        maskData.ruleEgrExtIpv4RaclVacl.vid1 = (GT_U16)inFields[42];
        maskData.ruleEgrExtIpv4RaclVacl.cfi1 = (GT_U8)inFields[43];
        maskData.ruleEgrExtIpv4RaclVacl.up1 = (GT_U8)inFields[44];

        maskData.ruleEgrExtIpv4RaclVacl.isUdbValid = (GT_U8)inFields[45];
        galtisBArray(&maskBArr,(GT_U8*)inFields[46]);

        cmdOsMemCpy(
            maskData.ruleEgrExtIpv4RaclVacl.udb36_49, maskBArr.data,
            maskBArr.length);

        galtisBArray(&maskBArr,(GT_U8*)inFields[47]);

        if( maskBArr.length )
        {
            maskData.ruleEgrExtIpv4RaclVacl.udb0 = maskBArr.data[0];
        }
    }

    mask_set = GT_TRUE;
    }
    else /* pattern */
    {
        pattern_ruleIndex = (GT_U32)inFields[0];
        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.pclId= (GT_U16)inFields[2];
        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.sourcePort= (GT_PHYSICAL_PORT_NUM)inFields[3];


        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.qosProfile= (GT_U8)inFields[4];
        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.isL2Valid=(GT_U8)inFields[5];
        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.originalVid=(GT_U16)inFields[6];

        epclCommonEgrPktTrunkFieldSet(&patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon,
            inFields);

        if(mask_set == GT_TRUE)
        {
            epclCommonEgrPktTrunkFieldSet(&maskData.ruleEgrExtIpv4RaclVacl.egressIpCommon,
                maskDataWords);
        }

        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.isIp=(GT_U8)inFields[9];
        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.isArp=(GT_U8)inFields[10];
        galtisMacAddr(&patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.macDa,(GT_U8*)inFields[11]);
        galtisMacAddr(&patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.macSa,(GT_U8*)inFields[12]);
        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.tos=(GT_U8)inFields[13];
        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.ipProtocol=(GT_U8)inFields[14];
        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.ttl=(GT_U8)inFields[15];
        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.isL4Valid=(GT_U8)inFields[16];
        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.l4Byte0=(GT_U8)inFields[17];
        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.l4Byte1=(GT_U8)inFields[18];
        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.l4Byte2=(GT_U8)inFields[19];
        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.l4Byte3=(GT_U8)inFields[20];
        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.l4Byte13=(GT_U8)inFields[21];
        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.tcpUdpPortComparators=(GT_U8)inFields[22];
        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.tc=(GT_U8)inFields[23];
        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.dp=(GT_U8)inFields[24];
        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.egrPacketType=(GT_U8)inFields[25];
        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.srcTrgOrTxMirror=(GT_U8)inFields[26];
        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.assignedUp=(GT_U8)inFields[27];
        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.rxSniff=(GT_U8)inFields[28];
        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.isRouted=(GT_U8)inFields[29];
        patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.isIpv6 =(GT_U8)inFields[30];
        galtisIpAddr(&patternData.ruleEgrExtIpv4RaclVacl.sip,(GT_U8*)inFields[31]);
        galtisIpAddr(&patternData.ruleEgrExtIpv4RaclVacl.dip,(GT_U8*)inFields[32]);
        patternData.ruleEgrExtIpv4RaclVacl.ipv4Options=(GT_U8)inFields[33];

        /*add on for extended pcl table*/
        if(GT_TRUE == extPcl)
            patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.trgPhysicalPort=(GT_U8)inFields[34];

        if(GT_TRUE == sip5Pcl)
        {
            patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.cpuCode = (GT_U8)inFields[35];
            patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.srcHwDev = (GT_U8)inFields[36];
            patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.srcTrg = (GT_U8)inFields[37];
            patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.egrFilterEnable = (GT_U8)inFields[38];
            patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.sourceId = (GT_U16)inFields[39];
            patternData.ruleEgrExtIpv4RaclVacl.isVidx = (GT_U8)inFields[40];
            patternData.ruleEgrExtIpv4RaclVacl.egressIpCommon.tag1Exist = (GT_U8)inFields[41];
            patternData.ruleEgrExtIpv4RaclVacl.vid1 = (GT_U16)inFields[42];
            patternData.ruleEgrExtIpv4RaclVacl.cfi1 = (GT_U8)inFields[43];
            patternData.ruleEgrExtIpv4RaclVacl.up1 = (GT_U8)inFields[44];

            patternData.ruleEgrExtIpv4RaclVacl.isUdbValid = (GT_U8)inFields[45];
            galtisBArray(&patternBArr,(GT_U8*)inFields[46]);

            cmdOsMemCpy(
                patternData.ruleEgrExtIpv4RaclVacl.udb36_49, patternBArr.data,
                patternBArr.length);

            galtisBArray(&patternBArr,(GT_U8*)inFields[47]);

            if( patternBArr.length )
            {
                patternData.ruleEgrExtIpv4RaclVacl.udb0 = patternBArr.data[0];
            }
        }

        pattern_set = GT_TRUE;
    }


    if ((mask_set == GT_TRUE) && (pattern_set == GT_TRUE))
    {
        PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleEgrExtIpv4RaclVacl.egressIpCommon.sourcePort);
    }

    return CMD_OK;
}

/*********************************************************************************/
static CMD_STATUS wrCpssDxCh3PclEgressRule_ULTRA_IPV6_Write
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_BYTE_ARRY    maskBArr, patternBArr;
    GT_U8           devNum;
    GT_U32          ii;

    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E;


    /* map input arguments to locals */
    /*
       inFields[1] = mask/pattern indication
       mask = 0
       pattern = 1
    */
    devNum=(GT_U8)inArgs[0];

    if(inFields[1] == 0) /* mask */
    {
    mask_ruleIndex = (GT_U32)inFields[0];
    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.pclId= (GT_U16)inFields[2];
    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.sourcePort= (GT_PHYSICAL_PORT_NUM)inFields[3];


    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.qosProfile= (GT_U8)inFields[4];
    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.isL2Valid=(GT_U8)inFields[5];
    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.originalVid=(GT_U16)inFields[6];

    if(pattern_set==GT_TRUE)
    {
        epclCommonEgrPktTrunkFieldSet(&maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon,
            inFields);
    }
    else
    {
        /* save the info from mask and set it properly when the pattern is set */
        for(ii = 7 ; ii <= 8 ; ii++)
        {
            maskDataWords[ii] = inFields[ii];
        }
    }

    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.isIp=(GT_U8)inFields[9];
    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.isArp=(GT_U8)inFields[10];
    galtisMacAddr(&maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.macDa,(GT_U8*)inFields[11]);
    galtisMacAddr(&maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.macSa,(GT_U8*)inFields[12]);
    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.tos=(GT_U8)inFields[13];
    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.ipProtocol=(GT_U8)inFields[14];
    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.ttl=(GT_U8)inFields[15];
    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.isL4Valid=(GT_U8)inFields[16];
    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.l4Byte0=(GT_U8)inFields[17];
    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.l4Byte1=(GT_U8)inFields[18];
    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.l4Byte2=(GT_U8)inFields[19];
    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.l4Byte3=(GT_U8)inFields[20];
    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.l4Byte13=(GT_U8)inFields[21];
    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.tcpUdpPortComparators=(GT_U8)inFields[22];
    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.tc=(GT_U8)inFields[23];
    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.dp=(GT_U8)inFields[24];
    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.egrPacketType=(GT_U8)inFields[25];
    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.srcTrgOrTxMirror=(GT_U8)inFields[26];
    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.assignedUp=(GT_U8)inFields[27];
    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.rxSniff=(GT_U8)inFields[28];
    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.isRouted=(GT_U8)inFields[29];
    maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.isIpv6=(GT_U8)inFields[30];
    galtisIpv6Addr(&maskData.ruleEgrUltraIpv6RaclVacl.sip,(GT_U8*)inFields[31]);
    galtisIpv6Addr(&maskData.ruleEgrUltraIpv6RaclVacl.dip,(GT_U8*)inFields[32]);
    maskData.ruleEgrUltraIpv6RaclVacl.isNd=(GT_U8)inFields[33];
    maskData.ruleEgrUltraIpv6RaclVacl.isIpv6ExtHdrExist=(GT_U8)inFields[34];
    maskData.ruleEgrUltraIpv6RaclVacl.isIpv6HopByHop=(GT_U8)inFields[35];

    /*add on for extended pcl table*/
    if(GT_TRUE == extPcl)
      maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.trgPhysicalPort=(GT_U8)inFields[36];

    if(GT_TRUE == sip5Pcl)
    {
        maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.cpuCode = (GT_U8)inFields[37];
        maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.srcHwDev = (GT_U8)inFields[38];
        maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.srcTrg = (GT_U8)inFields[39];
        maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.egrFilterEnable = (GT_U8)inFields[40];
        maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.sourceId = (GT_U16)inFields[41];
        maskData.ruleEgrUltraIpv6RaclVacl.isVidx = (GT_U8)inFields[42];
        maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.tag1Exist = (GT_U8)inFields[43];
        maskData.ruleEgrUltraIpv6RaclVacl.vid1 = (GT_U16)inFields[44];
        maskData.ruleEgrUltraIpv6RaclVacl.cfi1 = (GT_U8)inFields[45];
        maskData.ruleEgrUltraIpv6RaclVacl.up1 = (GT_U8)inFields[46];

        maskData.ruleEgrUltraIpv6RaclVacl.srcPort = (GT_U16)inFields[47];
        maskData.ruleEgrUltraIpv6RaclVacl.trgPort = (GT_U16)inFields[48];

        maskData.ruleEgrUltraIpv6RaclVacl.isUdbValid = (GT_U8)inFields[49];
        galtisBArray(&maskBArr,(GT_U8*)inFields[50]);

        cmdOsMemCpy(
            maskData.ruleEgrUltraIpv6RaclVacl.udb1_4, maskBArr.data,
            maskBArr.length);
    }

    mask_set = GT_TRUE;
    }
    else /* pattern */
    {
        pattern_ruleIndex = (GT_U32)inFields[0];
        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.pclId= (GT_U16)inFields[2];
        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.sourcePort= (GT_PHYSICAL_PORT_NUM)inFields[3];


        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.qosProfile= (GT_U8)inFields[4];
        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.isL2Valid=(GT_U8)inFields[5];
        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.originalVid=(GT_U16)inFields[6];

        epclCommonEgrPktTrunkFieldSet(&patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon,
            inFields);

        if(mask_set == GT_TRUE)
        {
            epclCommonEgrPktTrunkFieldSet(&maskData.ruleEgrUltraIpv6RaclVacl.egressIpCommon,
                maskDataWords);
        }

        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.isIp=(GT_U8)inFields[9];
        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.isArp=(GT_U8)inFields[10];
        galtisMacAddr(&patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.macDa,(GT_U8*)inFields[11]);
        galtisMacAddr(&patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.macSa,(GT_U8*)inFields[12]);
        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.tos=(GT_U8)inFields[13];
        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.ipProtocol=(GT_U8)inFields[14];
        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.ttl=(GT_U8)inFields[15];
        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.isL4Valid=(GT_U8)inFields[16];
        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.l4Byte0=(GT_U8)inFields[17];
        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.l4Byte1=(GT_U8)inFields[18];
        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.l4Byte2=(GT_U8)inFields[19];
        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.l4Byte3=(GT_U8)inFields[20];
        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.l4Byte13=(GT_U8)inFields[21];
        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.tcpUdpPortComparators=(GT_U8)inFields[22];
        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.tc=(GT_U8)inFields[23];
        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.dp=(GT_U8)inFields[24];
        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.egrPacketType=(GT_U8)inFields[25];
        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.srcTrgOrTxMirror=(GT_U8)inFields[26];
        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.assignedUp=(GT_U8)inFields[27];
        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.rxSniff=(GT_U8)inFields[28];
        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.isRouted=(GT_U8)inFields[29];
        patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.isIpv6=(GT_U8)inFields[30];
        galtisIpv6Addr(&patternData.ruleEgrUltraIpv6RaclVacl.sip,(GT_U8*)inFields[31]);
        galtisIpv6Addr(&patternData.ruleEgrUltraIpv6RaclVacl.dip,(GT_U8*)inFields[32]);
        patternData.ruleEgrUltraIpv6RaclVacl.isNd=(GT_U8)inFields[33];
        patternData.ruleEgrUltraIpv6RaclVacl.isIpv6ExtHdrExist=(GT_U8)inFields[34];
        patternData.ruleEgrUltraIpv6RaclVacl.isIpv6HopByHop=(GT_U8)inFields[35];

        /*add on for extended pcl table*/
        if(GT_TRUE == extPcl)
          patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.trgPhysicalPort=(GT_U8)inFields[36];

        if(GT_TRUE == sip5Pcl)
        {
            patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.cpuCode = (GT_U8)inFields[37];
            patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.srcHwDev = (GT_U8)inFields[38];
            patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.srcTrg = (GT_U8)inFields[39];
            patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.egrFilterEnable = (GT_U8)inFields[40];
            patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.sourceId = (GT_U16)inFields[41];
            patternData.ruleEgrUltraIpv6RaclVacl.isVidx = (GT_U8)inFields[42];
            patternData.ruleEgrUltraIpv6RaclVacl.egressIpCommon.tag1Exist = (GT_U8)inFields[43];
            patternData.ruleEgrUltraIpv6RaclVacl.vid1 = (GT_U16)inFields[44];
            patternData.ruleEgrUltraIpv6RaclVacl.cfi1 = (GT_U8)inFields[45];
            patternData.ruleEgrUltraIpv6RaclVacl.up1 = (GT_U8)inFields[46];

            patternData.ruleEgrUltraIpv6RaclVacl.srcPort = (GT_U16)inFields[47];
            patternData.ruleEgrUltraIpv6RaclVacl.trgPort = (GT_U16)inFields[48];

            patternData.ruleEgrUltraIpv6RaclVacl.isUdbValid = (GT_U8)inFields[49];
            galtisBArray(&patternBArr,(GT_U8*)inFields[50]);

            cmdOsMemCpy(
                patternData.ruleEgrUltraIpv6RaclVacl.udb1_4, patternBArr.data,
                patternBArr.length);
        }

        pattern_set = GT_TRUE;
    }


    if ((mask_set == GT_TRUE) && (pattern_set == GT_TRUE))
    {
        PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleEgrUltraIpv6RaclVacl.egressIpCommon.sourcePort);
    }

    return CMD_OK;
}

/***********************************************************************************/
static CMD_STATUS wrCpssDxChPclIngressRule_ULTRA_IPCL0_IPV6_Write
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8           devNum;
    GT_BYTE_ARRY   maskBExtNotIpv6, maskBExtIpv6L2,maskBExtIpv6L4;
    GT_BYTE_ARRY   patternBExtNotIpv6, patternBExtIpv6L2,patternBExtIpv6L4;


    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E;


    /* map input arguments to locals */
    /*
       inFields[1] = mask/pattern indication
       mask = 0
       pattern = 1
    */
    devNum=(GT_U8)inArgs[0];

    if(inFields[1] == 0) /* mask */
    {
        mask_ruleIndex = (GT_U32)inFields[0];
        maskData.ruleUltraIpv6PortVlanQos.ingressIpCommon.pclId= (GT_U16)inFields[2];
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&maskData.ruleUltraIpv6PortVlanQos.ingressIpCommon.portListBmp);
        maskData.ruleUltraIpv6PortVlanQos.ingressIpCommon.macToMe=(GT_U8)inFields[3];
        maskData.ruleUltraIpv6PortVlanQos.ingressIpCommon.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[4];


        maskData.ruleUltraIpv6PortVlanQos.ingressIpCommon.isTagged = (GT_U8)inFields[5];
        maskData.ruleUltraIpv6PortVlanQos.ingressIpCommon.vid = (GT_U16)inFields[6];
        maskData.ruleUltraIpv6PortVlanQos.ingressIpCommon.up = (GT_U8)inFields[7];
        maskData.ruleUltraIpv6PortVlanQos.ingressIpCommon.tos= (GT_U8)inFields[8];
        maskData.ruleUltraIpv6PortVlanQos.ingressIpCommon.ipProtocol=(GT_U8)inFields[9];

        maskData.ruleUltraIpv6PortVlanQos.isL2Valid=(GT_U8)inFields[10];
        maskData.ruleUltraIpv6PortVlanQos.isNd=(GT_U8)inFields[11];
        maskData.ruleUltraIpv6PortVlanQos.isBc=(GT_U8)inFields[12];
        maskData.ruleUltraIpv6PortVlanQos.isIp=(GT_U8)inFields[13];

        maskData.ruleUltraIpv6PortVlanQos.l2Encap=(GT_U8)inFields[14];
        maskData.ruleUltraIpv6PortVlanQos.etherType=(GT_U16)inFields[15];
        galtisMacAddr(&maskData.ruleUltraIpv6PortVlanQos.macDa,(GT_U8*)inFields[16]);
        galtisMacAddr(&maskData.ruleUltraIpv6PortVlanQos.macSa,(GT_U8*)inFields[17]);

        galtisIpv6Addr(&maskData.ruleUltraIpv6PortVlanQos.sip,(GT_U8*)inFields[18]);
        galtisIpv6Addr(&maskData.ruleUltraIpv6PortVlanQos.dip,(GT_U8*)inFields[19]);

        maskData.ruleUltraIpv6PortVlanQos.isIpv6ExtHdrExist=(GT_U8)inFields[20];
        maskData.ruleUltraIpv6PortVlanQos.isIpv6HopByHop=(GT_U8)inFields[21];
        maskData.ruleUltraIpv6PortVlanQos.ipHeaderOk=(GT_U8)inFields[22];

        maskData.ruleUltraIpv6PortVlanQos.ingressIpCommon.l4Byte0=(GT_U8)inFields[23];
        maskData.ruleUltraIpv6PortVlanQos.ingressIpCommon.l4Byte1=(GT_U8)inFields[24];
        maskData.ruleUltraIpv6PortVlanQos.ingressIpCommon.l4Byte2=(GT_U8)inFields[25];
        maskData.ruleUltraIpv6PortVlanQos.ingressIpCommon.l4Byte3=(GT_U8)inFields[26];
        maskData.ruleUltraIpv6PortVlanQos.ingressIpCommon.l4Byte13=(GT_U8)inFields[27];
        maskData.ruleUltraIpv6PortVlanQos.isUdbValid=(GT_U8)inFields[28];

        if( GT_FALSE == sip5Pcl )
        {
        galtisBArray(&maskBExtNotIpv6,(GT_U8*)inFields[29]);
        galtisBArray(&maskBExtIpv6L2,(GT_U8*)inFields[30]);
        galtisBArray(&maskBExtIpv6L4,(GT_U8*)inFields[31]);

        if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            CHECK_BYTE_ARRAY_LENGTH_MAC(maskData.ruleUltraIpv6PortVlanQos.UdbExtNotIpv6,
                                maskBExtNotIpv6);
            cmdOsMemCpy(
                    maskData.ruleUltraIpv6PortVlanQos.UdbExtNotIpv6, maskBExtNotIpv6.data,
                    maskBExtNotIpv6.length);

            CHECK_BYTE_ARRAY_LENGTH_MAC(maskData.ruleUltraIpv6PortVlanQos.UdbExtIpv6L2,
                                maskBExtIpv6L2);
            cmdOsMemCpy(
                    maskData.ruleUltraIpv6PortVlanQos.UdbExtIpv6L2, maskBExtIpv6L2.data,
                    maskBExtIpv6L2.length);

            CHECK_BYTE_ARRAY_LENGTH_MAC(maskData.ruleUltraIpv6PortVlanQos.UdbExtIpv6L4,
                                maskBExtIpv6L4);
            cmdOsMemCpy(
                    maskData.ruleUltraIpv6PortVlanQos.UdbExtIpv6L4, maskBExtIpv6L4.data,
                    maskBExtIpv6L4.length);
        }
        else
        {
            CHECK_BYTE_ARRAY_LENGTH_MAC(maskData.ruleUltraIpv6PortVlanQos.udb0_11,
                                maskBExtNotIpv6);
            cmdOsMemCpy(
                    maskData.ruleUltraIpv6PortVlanQos.udb0_11,
                    maskBExtNotIpv6.data,/* take all 12 UDBs from the first
                                            UDB field of the wrapper rule */
                    maskBExtNotIpv6.length);
        }
        }
        else /* GT_TRUE == sip5Pcl */
        {
            maskData.ruleUltraIpv6PortVlanQos.ingressIpCommon.isL4Valid = (GT_U8)inFields[29];
            maskData.ruleUltraIpv6PortVlanQos.ingressIpCommon.sourceDevice = (GT_U16)inFields[30];
            maskData.ruleUltraIpv6PortVlanQos.tag1Exist = (GT_U8)inFields[31];
            maskData.ruleUltraIpv6PortVlanQos.vid1 = (GT_U16)inFields[32];
            maskData.ruleUltraIpv6PortVlanQos.cfi1 = (GT_U8)inFields[33];
            maskData.ruleUltraIpv6PortVlanQos.up1 = (GT_U8)inFields[34];
            maskData.ruleUltraIpv6PortVlanQos.vrfId = (GT_U16)inFields[35];
            maskData.ruleUltraIpv6PortVlanQos.trunkHash = (GT_U8)inFields[36];
            maskData.ruleUltraIpv6PortVlanQos.srcPortOrTrunk = (GT_U16)inFields[37];
            maskData.ruleUltraIpv6PortVlanQos.srcIsTrunk = (GT_U8)inFields[38];
            galtisBArray(&maskBExtNotIpv6,(GT_U8*)inFields[39]);

            cmdOsMemCpy(
                maskData.ruleUltraIpv6PortVlanQos.udb0_11, maskBExtNotIpv6.data,
                maskBExtNotIpv6.length);

            galtisBArray(&maskBExtNotIpv6,(GT_U8*)inFields[40]);

            if( maskBExtNotIpv6.length )
            {
                maskData.ruleUltraIpv6PortVlanQos.udb12 = maskBExtNotIpv6.data[0];
            }

            galtisBArray(&maskBExtNotIpv6,(GT_U8*)inFields[41]);

            cmdOsMemCpy(
                maskData.ruleUltraIpv6PortVlanQos.udb39_40, maskBExtNotIpv6.data,
                maskBExtNotIpv6.length);
        }

        mask_set = GT_TRUE;
    }
    else /* pattern */
    {
        pattern_ruleIndex = (GT_U32)inFields[0];
        patternData.ruleUltraIpv6PortVlanQos.ingressIpCommon.pclId= (GT_U16)inFields[2];
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&patternData.ruleUltraIpv6PortVlanQos.ingressIpCommon.portListBmp);
        patternData.ruleUltraIpv6PortVlanQos.ingressIpCommon.macToMe=(GT_U8)inFields[3];
        patternData.ruleUltraIpv6PortVlanQos.ingressIpCommon.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[4];


        patternData.ruleUltraIpv6PortVlanQos.ingressIpCommon.isTagged = (GT_U8)inFields[5];
        patternData.ruleUltraIpv6PortVlanQos.ingressIpCommon.vid = (GT_U16)inFields[6];
        patternData.ruleUltraIpv6PortVlanQos.ingressIpCommon.up = (GT_U8)inFields[7];
        patternData.ruleUltraIpv6PortVlanQos.ingressIpCommon.tos= (GT_U8)inFields[8];
        patternData.ruleUltraIpv6PortVlanQos.ingressIpCommon.ipProtocol=(GT_U8)inFields[9];

        patternData.ruleUltraIpv6PortVlanQos.isL2Valid=(GT_U8)inFields[10];
        patternData.ruleUltraIpv6PortVlanQos.isNd=(GT_U8)inFields[11];
        patternData.ruleUltraIpv6PortVlanQos.isBc=(GT_U8)inFields[12];
        patternData.ruleUltraIpv6PortVlanQos.isIp=(GT_U8)inFields[13];

        patternData.ruleUltraIpv6PortVlanQos.l2Encap=(GT_U8)inFields[14];
        patternData.ruleUltraIpv6PortVlanQos.etherType=(GT_U16)inFields[15];
        galtisMacAddr(&patternData.ruleUltraIpv6PortVlanQos.macDa,(GT_U8*)inFields[16]);
        galtisMacAddr(&patternData.ruleUltraIpv6PortVlanQos.macSa,(GT_U8*)inFields[17]);

        galtisIpv6Addr(&patternData.ruleUltraIpv6PortVlanQos.sip,(GT_U8*)inFields[18]);
        galtisIpv6Addr(&patternData.ruleUltraIpv6PortVlanQos.dip,(GT_U8*)inFields[19]);

        patternData.ruleUltraIpv6PortVlanQos.isIpv6ExtHdrExist=(GT_U8)inFields[20];
        patternData.ruleUltraIpv6PortVlanQos.isIpv6HopByHop=(GT_U8)inFields[21];
        patternData.ruleUltraIpv6PortVlanQos.ipHeaderOk=(GT_U8)inFields[22];

        patternData.ruleUltraIpv6PortVlanQos.ingressIpCommon.l4Byte0=(GT_U8)inFields[23];
        patternData.ruleUltraIpv6PortVlanQos.ingressIpCommon.l4Byte1=(GT_U8)inFields[24];
        patternData.ruleUltraIpv6PortVlanQos.ingressIpCommon.l4Byte2=(GT_U8)inFields[25];
        patternData.ruleUltraIpv6PortVlanQos.ingressIpCommon.l4Byte3=(GT_U8)inFields[26];
        patternData.ruleUltraIpv6PortVlanQos.ingressIpCommon.l4Byte13=(GT_U8)inFields[27];
        patternData.ruleUltraIpv6PortVlanQos.isUdbValid=(GT_U8)inFields[28];

        if( GT_FALSE == sip5Pcl )
        {
        galtisBArray(&patternBExtNotIpv6,(GT_U8*)inFields[29]);
        galtisBArray(&patternBExtIpv6L2,(GT_U8*)inFields[30]);
        galtisBArray(&patternBExtIpv6L4,(GT_U8*)inFields[31]);


        if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            CHECK_BYTE_ARRAY_LENGTH_MAC(patternData.ruleUltraIpv6PortVlanQos.UdbExtNotIpv6,
                            patternBExtNotIpv6);
            cmdOsMemCpy(
                    patternData.ruleUltraIpv6PortVlanQos.UdbExtNotIpv6, patternBExtNotIpv6.data,
                    patternBExtNotIpv6.length);

            CHECK_BYTE_ARRAY_LENGTH_MAC(patternData.ruleUltraIpv6PortVlanQos.UdbExtIpv6L2,
                            patternBExtIpv6L2);
            cmdOsMemCpy(
                    patternData.ruleUltraIpv6PortVlanQos.UdbExtIpv6L2, patternBExtIpv6L2.data,
                    patternBExtIpv6L2.length);

            CHECK_BYTE_ARRAY_LENGTH_MAC(patternData.ruleUltraIpv6PortVlanQos.UdbExtIpv6L4,
                            patternBExtIpv6L4);
            cmdOsMemCpy(
                    patternData.ruleUltraIpv6PortVlanQos.UdbExtIpv6L4, patternBExtIpv6L4.data,
                    patternBExtIpv6L4.length);
        }
        else
        {
            CHECK_BYTE_ARRAY_LENGTH_MAC(patternData.ruleUltraIpv6PortVlanQos.udb0_11,
                            patternBExtNotIpv6);
            cmdOsMemCpy(
                    patternData.ruleUltraIpv6PortVlanQos.udb0_11,
                    patternBExtNotIpv6.data,/* take all 12 UDBs from the first
                                            UDB field of the wrapper rule */
                    patternBExtNotIpv6.length);
        }
        }
        else /* GT_TRUE == sip5Pcl */
        {
            patternData.ruleUltraIpv6PortVlanQos.ingressIpCommon.isL4Valid = (GT_U8)inFields[29];
            patternData.ruleUltraIpv6PortVlanQos.ingressIpCommon.sourceDevice = (GT_U16)inFields[30];
            patternData.ruleUltraIpv6PortVlanQos.tag1Exist = (GT_U8)inFields[31];
            patternData.ruleUltraIpv6PortVlanQos.vid1 = (GT_U16)inFields[32];
            patternData.ruleUltraIpv6PortVlanQos.cfi1 = (GT_U8)inFields[33];
            patternData.ruleUltraIpv6PortVlanQos.up1 = (GT_U8)inFields[34];
            patternData.ruleUltraIpv6PortVlanQos.vrfId = (GT_U16)inFields[35];
            patternData.ruleUltraIpv6PortVlanQos.trunkHash = (GT_U8)inFields[36];
            patternData.ruleUltraIpv6PortVlanQos.srcPortOrTrunk = (GT_U16)inFields[37];
            patternData.ruleUltraIpv6PortVlanQos.srcIsTrunk = (GT_U8)inFields[38];
            galtisBArray(&patternBExtNotIpv6,(GT_U8*)inFields[39]);

            cmdOsMemCpy(
                patternData.ruleUltraIpv6PortVlanQos.udb0_11, patternBExtNotIpv6.data,
                patternBExtNotIpv6.length);

            galtisBArray(&patternBExtNotIpv6,(GT_U8*)inFields[40]);

            if( patternBExtNotIpv6.length )
            {
                patternData.ruleUltraIpv6PortVlanQos.udb12 = patternBExtNotIpv6.data[0];
            }

            galtisBArray(&patternBExtNotIpv6,(GT_U8*)inFields[41]);

            cmdOsMemCpy(
                patternData.ruleUltraIpv6PortVlanQos.udb39_40, patternBExtNotIpv6.data,
                patternBExtNotIpv6.length);
        }

        pattern_set = GT_TRUE;
    }


    if ((mask_set == GT_TRUE) && (pattern_set == GT_TRUE))
    {
        PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleUltraIpv6PortVlanQos.ingressIpCommon.sourcePort);
    }

    return CMD_OK;

}
/**********************************************************************************/
static CMD_STATUS wrCpssDxChPclIngressRule_ULTRA_IPCL1_IPV6_Write
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8           devNum;

    GT_BYTE_ARRY   maskBStdNotIp, maskBStdIpL2Qos,maskBStdIpV4L4;
    GT_BYTE_ARRY   maskBExtNotIpv6, maskBExtIpv6L2,maskBExtIpv6L4;
    GT_BYTE_ARRY   patternBStdNotIp, patternBStdIpL2Qos,patternBStdIpV4L4;
    GT_BYTE_ARRY   patternBExtNotIpv6, patternBExtIpv6L2,patternBExtIpv6L4;

    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E;


    /* map input arguments to locals */
    /*
       inFields[1] = mask/pattern indication
       mask = 0
       pattern = 1
    */
    devNum=(GT_U8)inArgs[0];

    if(inFields[1] == 0) /* mask */
    {
        mask_ruleIndex = (GT_U32)inFields[0];
        maskData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.pclId= (GT_U16)inFields[2];
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&maskData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.portListBmp);
        maskData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.macToMe=(GT_U8)inFields[3];
        maskData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[4];


        maskData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.isTagged = (GT_U8)inFields[5];
        maskData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.vid = (GT_U16)inFields[6];
        maskData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.up = (GT_U8)inFields[7];
        maskData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.tos= (GT_U8)inFields[8];
        maskData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.ipProtocol=(GT_U8)inFields[9];
        maskData.ruleUltraIpv6RoutedAclQos.pktType=(GT_U8)inFields[10];
        maskData.ruleUltraIpv6RoutedAclQos.isNd=(GT_U8)inFields[11];
        maskData.ruleUltraIpv6RoutedAclQos.mplsOuterLabel=(GT_U32)inFields[12];
        maskData.ruleUltraIpv6RoutedAclQos.mplsOuterLabExp=(GT_U8)inFields[13];
        maskData.ruleUltraIpv6RoutedAclQos.mplsOuterLabSBit=(GT_U8)inFields[14];

        maskData.ruleUltraIpv6RoutedAclQos.ipPacketLength=(GT_U16)inFields[15];
        maskData.ruleUltraIpv6RoutedAclQos.ipv6HdrFlowLabel=(GT_U16)inFields[16];
        maskData.ruleUltraIpv6RoutedAclQos.ttl=(GT_U8)inFields[17];
        galtisIpv6Addr(&maskData.ruleUltraIpv6RoutedAclQos.sip,(GT_U8*)inFields[18]);
        galtisIpv6Addr(&maskData.ruleUltraIpv6RoutedAclQos.dip,(GT_U8*)inFields[19]);
        maskData.ruleUltraIpv6RoutedAclQos.isIpv6ExtHdrExist=(GT_U8)inFields[20];
        maskData.ruleUltraIpv6RoutedAclQos.isIpv6HopByHop=(GT_U8)inFields[21];
        maskData.ruleUltraIpv6RoutedAclQos.isIpv6LinkLocal=(GT_U8)inFields[22];
        maskData.ruleUltraIpv6RoutedAclQos.isIpv6Mld=(GT_U8)inFields[23];
        maskData.ruleUltraIpv6RoutedAclQos.ipHeaderOk=(GT_U8)inFields[24];
        maskData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.isL4Valid=(GT_U8)inFields[25];
        maskData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.l4Byte0=(GT_U8)inFields[26];
        maskData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.l4Byte1=(GT_U8)inFields[27];
        maskData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.l4Byte2=(GT_U8)inFields[28];
        maskData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.l4Byte3=(GT_U8)inFields[29];
        maskData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.l4Byte13=(GT_U8)inFields[30];
        maskData.ruleUltraIpv6RoutedAclQos.isUdbValid=(GT_U8)inFields[31];

        if( GT_FALSE == sip5Pcl )
        {
        galtisBArray(&maskBStdNotIp,(GT_U8*)inFields[32]);
        galtisBArray(&maskBStdIpL2Qos,(GT_U8*)inFields[33]);
        galtisBArray(&maskBStdIpV4L4,(GT_U8*)inFields[34]);
        galtisBArray(&maskBExtNotIpv6,(GT_U8*)inFields[35]);
        galtisBArray(&maskBExtIpv6L2,(GT_U8*)inFields[36]);
        galtisBArray(&maskBExtIpv6L4,(GT_U8*)inFields[37]);


        if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            CHECK_BYTE_ARRAY_LENGTH_MAC(maskData.ruleUltraIpv6RoutedAclQos.UdbStdNotIp,
                            maskBStdNotIp);
            cmdOsMemCpy(
                     maskData.ruleUltraIpv6RoutedAclQos.UdbStdNotIp, maskBStdNotIp.data,
                     maskBStdNotIp.length);

            CHECK_BYTE_ARRAY_LENGTH_MAC(maskData.ruleUltraIpv6RoutedAclQos.UdbStdIpL2Qos,
                            maskBStdIpL2Qos);
            cmdOsMemCpy(
                    maskData.ruleUltraIpv6RoutedAclQos.UdbStdIpL2Qos, maskBStdIpL2Qos.data,
                    maskBStdIpL2Qos.length);

            CHECK_BYTE_ARRAY_LENGTH_MAC(maskData.ruleUltraIpv6RoutedAclQos.UdbStdIpV4L4,
                            maskBStdIpV4L4);
            cmdOsMemCpy(
                     maskData.ruleUltraIpv6RoutedAclQos.UdbStdIpV4L4, maskBStdIpV4L4.data,
                     maskBStdIpV4L4.length);

            CHECK_BYTE_ARRAY_LENGTH_MAC(maskData.ruleUltraIpv6RoutedAclQos.UdbExtNotIpv6,
                            maskBExtNotIpv6);
            cmdOsMemCpy(
                     maskData.ruleUltraIpv6RoutedAclQos.UdbExtNotIpv6, maskBExtNotIpv6.data,
                     maskBExtNotIpv6.length);

            CHECK_BYTE_ARRAY_LENGTH_MAC(maskData.ruleUltraIpv6RoutedAclQos.UdbExtIpv6L2,
                            maskBExtIpv6L2);
            cmdOsMemCpy(
                    maskData.ruleUltraIpv6RoutedAclQos.UdbExtIpv6L2, maskBExtIpv6L2.data,
                    maskBExtIpv6L2.length);

            CHECK_BYTE_ARRAY_LENGTH_MAC(maskData.ruleUltraIpv6RoutedAclQos.UdbExtIpv6L4,
                            maskBExtIpv6L4);
            cmdOsMemCpy(
                     maskData.ruleUltraIpv6RoutedAclQos.UdbExtIpv6L4, maskBExtIpv6L4.data,
                     maskBExtIpv6L4.length);
        }
        else
        {
            /*xcat a1 and above*/
            CHECK_BYTE_ARRAY_LENGTH_MAC(maskData.ruleUltraIpv6RoutedAclQos.udb0_11,
                                maskBStdNotIp);
            cmdOsMemCpy(
                    maskData.ruleUltraIpv6RoutedAclQos.udb0_11,
                    maskBStdNotIp.data,/* take all 12 UDBs from the first
                                       UDB field of the wrapper rule */
                    maskBStdNotIp.length);

            CHECK_BYTE_ARRAY_LENGTH_MAC(maskData.ruleUltraIpv6RoutedAclQos.udb17_22,
                                maskBStdIpL2Qos);
            cmdOsMemCpy(
                    maskData.ruleUltraIpv6RoutedAclQos.udb17_22,
                    maskBStdIpL2Qos.data,/* take all 6 UDBs from the second
                                       UDB field of the wrapper rule */
                    maskBStdIpL2Qos.length);
        }
        }
        else /* GT_TRUE == sip5Pcl */
        {
            maskData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.sourceDevice = (GT_U16)inFields[32];
            maskData.ruleUltraIpv6RoutedAclQos.tag1Exist = (GT_U8)inFields[33];
            maskData.ruleUltraIpv6RoutedAclQos.vid1 = (GT_U16)inFields[34];
            maskData.ruleUltraIpv6RoutedAclQos.cfi1 = (GT_U8)inFields[35];
            maskData.ruleUltraIpv6RoutedAclQos.up1 = (GT_U8)inFields[36];
            maskData.ruleUltraIpv6RoutedAclQos.vrfId = (GT_U16)inFields[37];
            maskData.ruleUltraIpv6RoutedAclQos.trunkHash = (GT_U8)inFields[38];
            maskData.ruleUltraIpv6RoutedAclQos.srcPortOrTrunk = (GT_U16)inFields[39];
            maskData.ruleUltraIpv6RoutedAclQos.srcIsTrunk = (GT_U8)inFields[40];
            galtisBArray(&maskBStdNotIp,(GT_U8*)inFields[41]);

            cmdOsMemCpy(
                maskData.ruleUltraIpv6RoutedAclQos.udb0_11, maskBStdNotIp.data,
                maskBStdNotIp.length);

            galtisBArray(&maskBStdNotIp,(GT_U8*)inFields[42]);

            cmdOsMemCpy(
                maskData.ruleUltraIpv6RoutedAclQos.udb17_22, maskBStdNotIp.data,
                maskBStdNotIp.length);

            galtisBArray(&maskBStdNotIp,(GT_U8*)inFields[43]);

            cmdOsMemCpy(
                maskData.ruleUltraIpv6RoutedAclQos.udb45_46, maskBStdNotIp.data,
                maskBStdNotIp.length);
        }

        mask_set = GT_TRUE;
    }
    else /* pattern */
    {
        pattern_ruleIndex = (GT_U32)inFields[0];
        patternData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.pclId= (GT_U16)inFields[2];
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&patternData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.portListBmp);
        patternData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.macToMe=(GT_U8)inFields[3];
        patternData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.sourcePort = (GT_PHYSICAL_PORT_NUM)inFields[4];


        patternData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.isTagged = (GT_U8)inFields[5];
        patternData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.vid = (GT_U16)inFields[6];
        patternData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.up = (GT_U8)inFields[7];
        patternData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.tos= (GT_U8)inFields[8];
        patternData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.ipProtocol=(GT_U8)inFields[9];
        patternData.ruleUltraIpv6RoutedAclQos.pktType=(GT_U8)inFields[10];
        patternData.ruleUltraIpv6RoutedAclQos.isNd=(GT_U8)inFields[11];
        patternData.ruleUltraIpv6RoutedAclQos.mplsOuterLabel=(GT_U32)inFields[12];
        patternData.ruleUltraIpv6RoutedAclQos.mplsOuterLabExp=(GT_U8)inFields[13];
        patternData.ruleUltraIpv6RoutedAclQos.mplsOuterLabSBit=(GT_U8)inFields[14];

        patternData.ruleUltraIpv6RoutedAclQos.ipPacketLength=(GT_U16)inFields[15];
        patternData.ruleUltraIpv6RoutedAclQos.ipv6HdrFlowLabel=(GT_U16)inFields[16];
        patternData.ruleUltraIpv6RoutedAclQos.ttl=(GT_U8)inFields[17];
        galtisIpv6Addr(&patternData.ruleUltraIpv6RoutedAclQos.sip,(GT_U8*)inFields[18]);
        galtisIpv6Addr(&patternData.ruleUltraIpv6RoutedAclQos.dip,(GT_U8*)inFields[19]);
        patternData.ruleUltraIpv6RoutedAclQos.isIpv6ExtHdrExist=(GT_U8)inFields[20];
        patternData.ruleUltraIpv6RoutedAclQos.isIpv6HopByHop=(GT_U8)inFields[21];
        patternData.ruleUltraIpv6RoutedAclQos.isIpv6LinkLocal=(GT_U8)inFields[22];
        patternData.ruleUltraIpv6RoutedAclQos.isIpv6Mld=(GT_U8)inFields[23];
        patternData.ruleUltraIpv6RoutedAclQos.ipHeaderOk=(GT_U8)inFields[24];
        patternData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.isL4Valid=(GT_U8)inFields[25];
        patternData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.l4Byte0=(GT_U8)inFields[26];
        patternData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.l4Byte1=(GT_U8)inFields[27];
        patternData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.l4Byte2=(GT_U8)inFields[28];
        patternData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.l4Byte3=(GT_U8)inFields[29];
        patternData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.l4Byte13=(GT_U8)inFields[30];
        patternData.ruleUltraIpv6RoutedAclQos.isUdbValid=(GT_U8)inFields[31];

        if( GT_FALSE == sip5Pcl )
        {
        galtisBArray(&patternBStdNotIp,(GT_U8*)inFields[32]);
        galtisBArray(&patternBStdIpL2Qos,(GT_U8*)inFields[33]);
        galtisBArray(&patternBStdIpV4L4,(GT_U8*)inFields[34]);
        galtisBArray(&patternBExtNotIpv6,(GT_U8*)inFields[35]);
        galtisBArray(&patternBExtIpv6L2,(GT_U8*)inFields[36]);
        galtisBArray(&patternBExtIpv6L4,(GT_U8*)inFields[37]);

        if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            CHECK_BYTE_ARRAY_LENGTH_MAC(patternData.ruleUltraIpv6RoutedAclQos.UdbStdNotIp,
                            patternBStdNotIp);
            cmdOsMemCpy(
                    patternData.ruleUltraIpv6RoutedAclQos.UdbStdNotIp, patternBStdNotIp.data,
                    patternBStdNotIp.length);

            CHECK_BYTE_ARRAY_LENGTH_MAC(patternData.ruleUltraIpv6RoutedAclQos.UdbStdIpL2Qos,
                            patternBStdIpL2Qos);
            cmdOsMemCpy(
                    patternData.ruleUltraIpv6RoutedAclQos.UdbStdIpL2Qos, patternBStdIpL2Qos.data,
                    patternBStdIpL2Qos.length);

            CHECK_BYTE_ARRAY_LENGTH_MAC(patternData.ruleUltraIpv6RoutedAclQos.UdbStdIpV4L4,
                            patternBStdIpV4L4);
            cmdOsMemCpy(
                    patternData.ruleUltraIpv6RoutedAclQos.UdbStdIpV4L4, patternBStdIpV4L4.data,
                    patternBStdIpV4L4.length);

            CHECK_BYTE_ARRAY_LENGTH_MAC(patternData.ruleUltraIpv6RoutedAclQos.UdbExtNotIpv6,
                            patternBExtNotIpv6);
            cmdOsMemCpy(
                    patternData.ruleUltraIpv6RoutedAclQos.UdbExtNotIpv6, patternBExtNotIpv6.data,
                    patternBExtNotIpv6.length);

            CHECK_BYTE_ARRAY_LENGTH_MAC(patternData.ruleUltraIpv6RoutedAclQos.UdbExtIpv6L2,
                            patternBExtIpv6L2);
            cmdOsMemCpy(
                    patternData.ruleUltraIpv6RoutedAclQos.UdbExtIpv6L2, patternBExtIpv6L2.data,
                    patternBExtIpv6L2.length);

            CHECK_BYTE_ARRAY_LENGTH_MAC(patternData.ruleUltraIpv6RoutedAclQos.UdbExtIpv6L4,
                            patternBExtIpv6L4);
            cmdOsMemCpy(
                    patternData.ruleUltraIpv6RoutedAclQos.UdbExtIpv6L4, patternBExtIpv6L4.data,
                    patternBExtIpv6L4.length);
        }
        else
        {
            /*xcat and above*/
            CHECK_BYTE_ARRAY_LENGTH_MAC(patternData.ruleUltraIpv6RoutedAclQos.udb0_11,
                                patternBStdNotIp);
            cmdOsMemCpy(
                    patternData.ruleUltraIpv6RoutedAclQos.udb0_11,
                    patternBStdNotIp.data,/* take all 12 UDBs from the first
                                       UDB field of the wrapper rule */
                    patternBStdNotIp.length);

            CHECK_BYTE_ARRAY_LENGTH_MAC(patternData.ruleUltraIpv6RoutedAclQos.udb17_22,
                                patternBStdIpL2Qos);
            cmdOsMemCpy(
                    patternData.ruleUltraIpv6RoutedAclQos.udb17_22,
                    patternBStdIpL2Qos.data,/* take all 6 UDBs from the second
                                       UDB field of the wrapper rule */
                    patternBStdIpL2Qos.length);
        }
        }
        else /* GT_TRUE == sip5Pcl */
        {
            patternData.ruleUltraIpv6RoutedAclQos.ingressIpCommon.sourceDevice = (GT_U16)inFields[32];
            patternData.ruleUltraIpv6RoutedAclQos.tag1Exist = (GT_U8)inFields[33];
            patternData.ruleUltraIpv6RoutedAclQos.vid1 = (GT_U16)inFields[34];
            patternData.ruleUltraIpv6RoutedAclQos.cfi1 = (GT_U8)inFields[35];
            patternData.ruleUltraIpv6RoutedAclQos.up1 = (GT_U8)inFields[36];
            patternData.ruleUltraIpv6RoutedAclQos.vrfId = (GT_U16)inFields[37];
            patternData.ruleUltraIpv6RoutedAclQos.trunkHash = (GT_U8)inFields[38];
            patternData.ruleUltraIpv6RoutedAclQos.srcPortOrTrunk = (GT_U16)inFields[39];
            patternData.ruleUltraIpv6RoutedAclQos.srcIsTrunk = (GT_U8)inFields[40];
            galtisBArray(&patternBStdNotIp,(GT_U8*)inFields[41]);

            cmdOsMemCpy(
                patternData.ruleUltraIpv6RoutedAclQos.udb0_11, patternBStdNotIp.data,
                patternBStdNotIp.length);

            galtisBArray(&patternBStdNotIp,(GT_U8*)inFields[42]);

            cmdOsMemCpy(
                patternData.ruleUltraIpv6RoutedAclQos.udb17_22, patternBStdNotIp.data,
                patternBStdNotIp.length);

            galtisBArray(&patternBStdNotIp,(GT_U8*)inFields[43]);

            cmdOsMemCpy(
                patternData.ruleUltraIpv6RoutedAclQos.udb45_46, patternBStdNotIp.data,
                patternBStdNotIp.length);
        }

        pattern_set = GT_TRUE;
    }

    if ((mask_set == GT_TRUE) && (pattern_set == GT_TRUE))
    {
        PRV_CONVERT_RULE_PORT_MAC(devNum, maskData, patternData, ruleUltraIpv6RoutedAclQos.ingressIpCommon.sourcePort);
    }

    return CMD_OK;

}
/*********************************************************************************/
static CMD_STATUS  wrCpssDxChPclRuleWriteFirst

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&maskData , 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
    cpssOsMemSet(&patternData, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));


    mask_set    = GT_FALSE;
    pattern_set = GT_FALSE;


    switch (inArgs[1]+FIRST_NEW_FORMAT_CNS)
    {
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E:
            wrCpssDxChPclIngressRule_IPCL1_IPV4_E_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E:
            wrCpssDxChPclIngressRule_EXT_IPCL0_IPV4_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E:
            wrCpssDxChPclIngressRule_ULTRA_IPCL0_IPV6_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E:
            wrCpssDxChPclIngressRule_ULTRA_IPCL1_IPV6_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E:
            wrCpssDxChPclEgressRule_EXT_NOT_IPV6_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E:
            wrCpssDxCh3PclEgressRule_ULTRA_IPV6_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;


        default:
            galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong PCL RULE FORMAT.\n");
            return CMD_AGENT_ERROR;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}
/*************************************************************************/
static CMD_STATUS wrCpssDxChPclRuleWriteNext

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result = GT_OK;
    GT_U8           devNum;
    CPSS_DXCH_PCL_ACTION_STC            *actionPtr;
    GT_U32                              actionIndex;
    GT_U32                              ruleIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;


    /* Check Validity */
    if(inFields[1] > 1)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong value for data_type (should be MASK or PATTERN).\n");
        return CMD_AGENT_ERROR;
    }


    if(inFields[1] == 0 && mask_set)
    {
        extPcl=GT_FALSE;

        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Must be subsequent pair of Ingress {Rule,Mask}.\n");
        return CMD_AGENT_ERROR;
    }

    if(inFields[1] == 1 && pattern_set)
    {
        extPcl=GT_FALSE;

        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Must be subsequent pair of Ingress {Rule,Mask}.\n");
        return CMD_AGENT_ERROR;
    }



    /*
       inFields[1] = mask/pattern indication
       mask = 0
       pattern = 1
    */


    switch (inArgs[1]+FIRST_NEW_FORMAT_CNS)
    {
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E:
            wrCpssDxChPclIngressRule_IPCL1_IPV4_E_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E:
            wrCpssDxChPclIngressRule_EXT_IPCL0_IPV4_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E:
            wrCpssDxChPclIngressRule_ULTRA_IPCL0_IPV6_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E:
            wrCpssDxChPclIngressRule_ULTRA_IPCL1_IPV6_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E:
            wrCpssDxChPclEgressRule_EXT_NOT_IPV6_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E:
            wrCpssDxCh3PclEgressRule_ULTRA_IPV6_Write(
                inArgs, inFields ,numFields ,outArgs);
            break;


        default:
            galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong PCL RULE FORMAT.\n");
            return CMD_AGENT_ERROR;
    }
    if(mask_set && pattern_set && (mask_ruleIndex != pattern_ruleIndex))
    {
        extPcl=GT_FALSE;

        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : mask ruleIndex doesn't match pattern ruleIndex.\n");
        return CMD_AGENT_ERROR;
    }

    /* Get the action */
    ruleIndex = (GT_U32)inFields[0];
    pclDxCh3ActionGet( ruleIndex, &actionIndex);
    if(actionIndex == INVALID_RULE_INDEX || !(actionTable[actionIndex].valid))
    {
        extPcl=GT_FALSE;

        /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Action table entry is not set.\n");
        return CMD_AGENT_ERROR;

    }

    if(mask_set && pattern_set)
    {
         /* map input arguments to locals */
        devNum = (GT_U8)inArgs[0];

        actionPtr = &(actionTable[actionIndex].actionEntry);

        /* call cpss api function */
        result = pg_wrap_cpssDxChPclRuleSet(devNum, ruleFormat, ruleIndex, &maskData,
                                                    &patternData, actionPtr);

        mask_set = GT_FALSE;
        pattern_set = GT_FALSE;

        cpssOsMemSet(&maskData , 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&patternData, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));



    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/*************************************************************************/
static CMD_STATUS wrCpssDxChPclRuleWriteEnd

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
  if(mask_set || pattern_set)
      /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_BAD_STATE, "");
    else
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}


/*************************************************************************/
static CMD_STATUS wrCpssDxChPclRuleCancelSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    /* pack output arguments to galtis string */
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : result is not GT_OK\n");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclRuleExtWriteNext function
* @endinternal
*
* @brief   The function sets the Policy Rule Mask, Pattern and Action
*
* @note   APPLICABLE DEVICES:      All DxCh3 devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
*
* @note NONE
*
*/
static CMD_STATUS wrCpssDxChPclRuleExtWriteNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{   extPcl=GT_TRUE;
    return wrCpssDxChPclRuleWriteNext(inArgs,inFields,numFields,outArgs);
}
/**
* @internal wrCpssDxChPclRuleExtWriteFirst function
* @endinternal
*
* @brief   The function sets the Policy Rule Mask, Pattern and Action
*
* @note   APPLICABLE DEVICES:      All DxCh3 devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
*
* @note NONE
*
*/
static CMD_STATUS wrCpssDxChPclRuleExtWriteFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    extPcl=GT_TRUE;
    return wrCpssDxChPclRuleWriteFirst(inArgs,inFields,numFields,outArgs);
}
/**
* @internal wrCpssDxChPclRuleExtWriteEnd function
* @endinternal
*
* @brief   The function sets the Policy Rule Mask, Pattern and Action
*
* @note   APPLICABLE DEVICES:      All DxCh3 devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
*
* @note NONE
*
*/
static CMD_STATUS wrCpssDxChPclRuleExtWriteEnd
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{   extPcl=GT_FALSE;
    return wrCpssDxChPclRuleWriteEnd(inArgs,inFields,numFields,outArgs);
}

/*************************************************************************/
static CMD_STATUS  wrCpssDxChPclLion3IngressRuleWriteFirst

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CMD_STATUS  cmdStatus;

    sip5Pcl = GT_TRUE;

    switch (inArgs[1])
    {
        case 0 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E*/:
        case 1 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E*/:
        case 2 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E*/:
        case 3 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E*/:
        case 4 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E*/:
        case 5 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E*/:
        case 6 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E*/:
            cmdStatus = wrCpssDxCh3PclIngressRuleWriteFirst(
                            inArgs, inFields ,numFields ,outArgs);
            break;

        case 7 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E*/:
        case 8 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E*/:
        case 9 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E*/:
        case 10 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E*/:
            /* update identation to use former wrapper */
            inArgs[1]= inArgs[1]-7;
            cmdStatus = wrCpssDxChPclRuleExtWriteFirst(
                            inArgs, inFields ,numFields ,outArgs);
            extPcl=GT_FALSE;
            break;

        case 11 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E*/:
        case 12 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E*/:
            /* update identation to use former wrapper */
            inArgs[1] = inArgs[1]-11;
            cmdStatus = wrCpssDxChPclXCatUDBRuleWriteFirst(
                            inArgs, inFields ,numFields ,outArgs);
            break;

        default:
            galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong PCL RULE FORMAT.\n");
            cmdStatus = CMD_AGENT_ERROR;
    }

    sip5Pcl = GT_FALSE;

    return cmdStatus;
}
/*************************************************************************/

static CMD_STATUS wrCpssDxChPclLion3IngressRuleWriteNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CMD_STATUS  cmdStatus;

    sip5Pcl = GT_TRUE;

    switch (inArgs[1])
    {
        case 0 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E*/:
        case 1 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E*/:
        case 2 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E*/:
        case 3 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E*/:
        case 4 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E*/:
        case 5 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E*/:
        case 6 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E*/:
            cmdStatus = wrCpssDxCh3PclIngressRuleWriteNext(
                            inArgs, inFields ,numFields ,outArgs);
            break;

        case 7 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E*/:
        case 8 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E*/:
        case 9 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E*/:
        case 10 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E*/:
            /* update identation to use former wrapper */
            inArgs[1]= inArgs[1]-7;
            cmdStatus = wrCpssDxChPclRuleExtWriteNext(
                            inArgs, inFields ,numFields ,outArgs);
            extPcl=GT_FALSE;
            break;

        case 11 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E*/:
        case 12 /*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E*/:
            /* update identation to use former wrapper */
            inArgs[1] = inArgs[1]-11;
            cmdStatus = wrCpssDxChPclXCatUDBRuleWriteNext(
                            inArgs, inFields ,numFields ,outArgs);
            break;

        default:
            galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong PCL RULE FORMAT.\n");
            cmdStatus = CMD_AGENT_ERROR;
    }

    sip5Pcl = GT_FALSE;

    return cmdStatus;
}

/*************************************************************************/
static CMD_STATUS wrCpssDxChPclLion3IngressRuleWriteEnd
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    if(mask_set || pattern_set)
      /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_BAD_STATE, "");
    else
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_OK, "");

    sip5Pcl = GT_FALSE;
    extPcl = GT_FALSE;

    return CMD_OK;
}

/*********************************************************************************/

static CMD_STATUS  wrCpssDxChPclLion3EgressRuleWriteFirst

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CMD_STATUS cmdStatus;

    sip5Pcl = GT_TRUE;

    switch (inArgs[1])
    {
        case 0 /*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E*/:
        case 1 /*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E*/:
        case 2 /*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E*/:
        case 3 /*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E*/:
        case 4 /*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E*/:
        case 5 /*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E*/:
            cmdStatus = wrCpssDxCh3PclEgressRuleWriteFirst(
                            inArgs, inFields ,numFields ,outArgs);
            break;

        case 6 /*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E*/:
        case 7 /*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E*/:
            /* update identation to use former wrapper */
            inArgs[1]= inArgs[1]-2;
            cmdStatus = wrCpssDxChPclRuleExtWriteFirst(
                            inArgs, inFields ,numFields ,outArgs);
            extPcl = GT_FALSE;
            break;
        default:
            galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong PCL RULE FORMAT.\n");
            cmdStatus = CMD_AGENT_ERROR;
    }

    sip5Pcl = GT_FALSE;

    return cmdStatus;
}
/*************************************************************************/

static CMD_STATUS wrCpssDxChPclLion3EgressRuleWriteNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CMD_STATUS cmdStatus;

    sip5Pcl = GT_TRUE;

    switch (inArgs[1])
    {
        case 0 /*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E*/:
        case 1 /*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E*/:
        case 2 /*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E*/:
        case 3 /*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E*/:
        case 4 /*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E*/:
        case 5 /*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E*/:
            cmdStatus = wrCpssDxCh3PclEgressRuleWriteNext(
                        inArgs, inFields ,numFields ,outArgs);
            break;

        case 6 /*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E*/:
        case 7 /*CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E*/:
            /* update identation to use former wrapper */
            inArgs[1]= inArgs[1]-2;
            cmdStatus = wrCpssDxChPclRuleExtWriteNext(
                            inArgs, inFields ,numFields ,outArgs);
            extPcl = GT_FALSE;
            break;
        default:
            galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong PCL RULE FORMAT.\n");
            cmdStatus = CMD_AGENT_ERROR;
    }

    sip5Pcl = GT_FALSE;

    return cmdStatus;
}

/*************************************************************************/
static CMD_STATUS wrCpssDxChPclLion3EgressRuleWriteEnd

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    if(mask_set || pattern_set)
      /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_BAD_STATE, "");
    else
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_OK, "");

    sip5Pcl = GT_FALSE;
    extPcl = GT_FALSE;

    return CMD_OK;
}

/* UDB obly SIP5 rules */
static GT_U32 lineIndexOfPclRuleUdbOnly;

/*************************************************************************/
static CMD_STATUS wrCpssDxChPclRuleUdbOnlyIngressSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    static GT_U32 lineIndexOfTable = 0;

    GT_STATUS                                        result = GT_OK;
    GT_U8                                            devNum;
    CPSS_DXCH_PCL_ACTION_STC                         *actionPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *rulePtr;
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_ONLY_STC   *rPtr;
    GT_U32                                           actionIndex;
    GT_U32                                           ruleIndex;
    GT_U32                                           ii;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

     /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ruleIndex = (GT_U32)inArgs[1];
    ruleFormat = (CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT)
        (CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E + inArgs[2]);

    rulePtr = (lineIndexOfTable == 0) ? &maskData : &patternData;
    cpssOsMemSet(rulePtr, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
    rPtr = &(rulePtr->ruleIngrUdbOnly);

    /* fields */

    galtisBytesArrayDirect(
        rPtr->udb, (GT_U8*)inFields[0], sizeof(rPtr->udb));

    ii = 1;

    /*rPtr->replacedFld*/
    rPtr->replacedFld.pclId       = (GT_U32)inFields[ii++];
    rPtr->replacedFld.isUdbValid  = (GT_U8)inFields[ii++];
    rPtr->replacedFld.vid         = (GT_U32)inFields[ii++];
    rPtr->replacedFld.srcPort     = (GT_U32)inFields[ii++];
    rPtr->replacedFld.srcDevIsOwn = (GT_U8)inFields[ii++];
    rPtr->replacedFld.vid1        = (GT_U32)inFields[ii++];
    rPtr->replacedFld.up1         = (GT_U32)inFields[ii++];
    rPtr->replacedFld.macToMe     = (GT_U8)inFields[ii++];
    rPtr->replacedFld.qosProfile  = (GT_U32)inFields[ii++];
    rPtr->replacedFld.flowId      = (GT_U32)inFields[ii++];

    /*rPtr->udb60FixedFld*/
    rPtr->udb60FixedFld.isUdbValid   = (GT_U8)inFields[ii++];
    rPtr->udb60FixedFld.pclId        = (GT_U32)inFields[ii++];
    rPtr->udb60FixedFld.vid          = (GT_U32)inFields[ii++];
    rPtr->udb60FixedFld.srcPort      = (GT_U32)inFields[ii++];
    rPtr->udb60FixedFld.srcDevIsOwn  = (GT_U8)inFields[ii++];
    rPtr->udb60FixedFld.vid1         = (GT_U32)inFields[ii++];
    rPtr->udb60FixedFld.up1          = (GT_U32)inFields[ii++];
    rPtr->udb60FixedFld.macToMe      = (GT_U8)inFields[ii++];
    rPtr->udb60FixedFld.qosProfile   = (GT_U32)inFields[ii++];
    rPtr->udb60FixedFld.flowId       = (GT_U32)inFields[ii++];

    if (lineIndexOfTable == 1)
    {
        lineIndexOfTable = 0;
        /* Get the action */
        pclDxCh3ActionGet( ruleIndex, &actionIndex);
        if ((actionIndex == INVALID_RULE_INDEX)
            || (actionTable[actionIndex].valid == GT_FALSE))
        {
            /* pack output arguments to galtis string */
            galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Action table entry is not set.\n");
            return CMD_AGENT_ERROR;
        }

        actionPtr = &(actionTable[actionIndex].actionEntry);

        /* call cpss api function */
        result = pg_wrap_cpssDxChPclRuleSet(
            devNum, ruleFormat, ruleIndex,
            &maskData, &patternData, actionPtr);
    }
    else
    {
        lineIndexOfTable = 1;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*************************************************************************/
static CMD_STATUS wrCpssDxChPclRuleUdbOnlyIngressGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                        result = GT_OK;
    GT_U8                                            devNum;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *rulePtr;
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_ONLY_STC   *rPtr;
    GT_U32                                           ruleIndex;
    char                                             *strPtr;
    GT_BOOL                                          isRuleValid;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (lineIndexOfPclRuleUdbOnly > 1)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

     /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ruleIndex = (GT_U32)inArgs[1];
    ruleFormat = (CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT)
        (CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E + inArgs[2]);

    if (lineIndexOfPclRuleUdbOnly == 0)
    {
        /* call cpss api function */
        result = pg_wrap_cpssDxChPclRuleParsedGet(
            devNum, ruleFormat, ruleIndex, 0 /*ruleOptionsBmp*/,
            &isRuleValid, &maskData, &patternData, &actionData);
        if (result != 0)
        {
            galtisOutput(outArgs, result, "%d", -1);
            return CMD_OK;
        }
    }

    rulePtr = (lineIndexOfPclRuleUdbOnly == 0) ? &maskData : &patternData;
    rPtr = &(rulePtr->ruleIngrUdbOnly);

    lineIndexOfPclRuleUdbOnly++;

    /* fields */
    strPtr = galtisByteArrayToString(rPtr->udb, sizeof(rPtr->udb));
    fieldOutput("%s", strPtr);
    fieldOutputSetAppendMode();

    fieldOutput(
        "%d%d%d%d",
        rPtr->replacedFld.pclId, rPtr->replacedFld.isUdbValid,
        rPtr->replacedFld.vid, rPtr->replacedFld.srcPort);
    fieldOutputSetAppendMode();

    fieldOutput(
        "%d%d%d%d%d%d", rPtr->replacedFld.srcDevIsOwn,
        rPtr->replacedFld.vid1, rPtr->replacedFld.up1, rPtr->replacedFld.macToMe,
        rPtr->replacedFld.qosProfile, rPtr->replacedFld.flowId);
    fieldOutputSetAppendMode();

    fieldOutput(
        "%d%d%d%d%d",
        rPtr->udb60FixedFld.isUdbValid, rPtr->udb60FixedFld.pclId, rPtr->udb60FixedFld.vid,
        rPtr->udb60FixedFld.srcPort, rPtr->udb60FixedFld.srcDevIsOwn);
    fieldOutputSetAppendMode();

    fieldOutput(
        "%d%d%d%d%d",
        rPtr->udb60FixedFld.vid1, rPtr->udb60FixedFld.up1, rPtr->udb60FixedFld.macToMe,
        rPtr->udb60FixedFld.qosProfile, rPtr->udb60FixedFld.flowId);
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChPclRuleUdbOnlyIngressGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    lineIndexOfPclRuleUdbOnly = 0;

    return wrCpssDxChPclRuleUdbOnlyIngressGetNext(
        inArgs, inFields, numFields,outArgs);
}

/*************************************************************************/
static CMD_STATUS wrCpssDxChPclRuleUdbOnlyEgressSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    static GT_U32 lineIndexOfTable = 0;

    GT_STATUS                                        result = GT_OK;
    GT_U8                                            devNum;
    CPSS_DXCH_PCL_ACTION_STC                         *actionPtr;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *rulePtr;
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_ONLY_STC    *rPtr;
    GT_U32                                           actionIndex;
    GT_U32                                           ruleIndex;
    GT_U32                                           ii;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

     /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ruleIndex = (GT_U32)inArgs[1];
    ruleFormat = (CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT)
        (CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E + inArgs[2]);

    rulePtr = (lineIndexOfTable == 0) ? &maskData : &patternData;
    cpssOsMemSet(rulePtr, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
    rPtr = &(rulePtr->ruleEgrUdbOnly);

    /* fields */

    galtisBytesArrayDirect(
        rPtr->udb, (GT_U8*)inFields[0], sizeof(rPtr->udb));

    ii = 1;

    /*rPtr->replacedFld*/
    rPtr->replacedFld.pclId       = (GT_U32)inFields[ii++];
    rPtr->replacedFld.isUdbValid  = (GT_U8)inFields[ii++];

    /*rPtr->udb60FixedFld*/
    rPtr->udb60FixedFld.pclId               = (GT_U32)inFields[ii++];
    rPtr->udb60FixedFld.isUdbValid          = (GT_U8)inFields[ii++];
    rPtr->udb60FixedFld.vid                 = (GT_U32)inFields[ii++];
    rPtr->udb60FixedFld.srcPort             = (GT_U32)inFields[ii++];
    rPtr->udb60FixedFld.trgPort             = (GT_U32)inFields[ii++];
    rPtr->udb60FixedFld.srcDev              = (GT_U32)inFields[ii++];
    rPtr->udb60FixedFld.trgDev              = (GT_U32)inFields[ii++];
    rPtr->udb60FixedFld.localDevTrgPhyPort  = (GT_PHYSICAL_PORT_NUM)inFields[ii++];

    if (lineIndexOfTable == 1)
    {
        lineIndexOfTable = 0;

        /* Get the action */
        pclDxCh3ActionGet( ruleIndex, &actionIndex);
        if ((actionIndex == INVALID_RULE_INDEX)
            || (actionTable[actionIndex].valid == GT_FALSE))
        {
            /* pack output arguments to galtis string */
            galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Action table entry is not set.\n");
            return CMD_AGENT_ERROR;
        }

        actionPtr = &(actionTable[actionIndex].actionEntry);

        /* call cpss api function */
        result = pg_wrap_cpssDxChPclRuleSet(
            devNum, ruleFormat, ruleIndex,
            &maskData, &patternData, actionPtr);
    }
    else
    {
        lineIndexOfTable = 1;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*************************************************************************/
static CMD_STATUS wrCpssDxChPclRuleUdbOnlyEgressGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                        result = GT_OK;
    GT_U8                                            devNum;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *rulePtr;
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_ONLY_STC    *rPtr;
    GT_U32                                           ruleIndex;
    char                                             *strPtr;
    GT_BOOL                                          isRuleValid;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (lineIndexOfPclRuleUdbOnly > 1)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

     /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ruleIndex = (GT_U32)inArgs[1];
    ruleFormat = (CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT)
        (CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E + inArgs[2]);

    if (lineIndexOfPclRuleUdbOnly == 0)
    {
        /* call cpss api function */
        result = pg_wrap_cpssDxChPclRuleParsedGet(
            devNum, ruleFormat, ruleIndex, 0 /*ruleOptionsBmp*/,
            &isRuleValid, &maskData, &patternData, &actionData);
        if (result != 0)
        {
            galtisOutput(outArgs, result, "%d", -1);
            return CMD_OK;
        }
    }

    rulePtr = (lineIndexOfPclRuleUdbOnly == 0) ? &maskData : &patternData;
    rPtr = &(rulePtr->ruleEgrUdbOnly);

    lineIndexOfPclRuleUdbOnly++;

    /* fields */
    strPtr = galtisByteArrayToString(rPtr->udb, sizeof(rPtr->udb));
    fieldOutput("%s", strPtr);
    fieldOutputSetAppendMode();

    fieldOutput(
        "%d%d",rPtr->replacedFld.pclId, rPtr->replacedFld.isUdbValid);
    fieldOutputSetAppendMode();


    fieldOutput(
        "%d%d%d%d%d",
        rPtr->udb60FixedFld.pclId, rPtr->udb60FixedFld.isUdbValid, rPtr->udb60FixedFld.vid,
        rPtr->udb60FixedFld.srcPort, rPtr->udb60FixedFld.trgDev);
    fieldOutputSetAppendMode();

    fieldOutput(
        "%d%d%d", rPtr->udb60FixedFld.srcDev,
        rPtr->udb60FixedFld.trgDev, rPtr->udb60FixedFld.localDevTrgPhyPort);
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChPclRuleUdbOnlyEgressGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    lineIndexOfPclRuleUdbOnly = 0;

    return wrCpssDxChPclRuleUdbOnlyEgressGetNext(
        inArgs, inFields, numFields,outArgs);
}

/**********************************************************************************/

#define MASK_SIZE_WORDS 36
#define PATTERN_SIZE_WORDS 36
#define ACTION_SIZE_WORDS 4

static GT_U32 maskPclBtm[MASK_SIZE_WORDS];
static GT_U32 patternPclBtm[PATTERN_SIZE_WORDS];
static GT_U32 actionPclBtm[ACTION_SIZE_WORDS];
static GT_U8  state;
/*
state==0-print Mask
state==1-print Pattern
state==2-print action
*/
/**
* @internal wrCpssDxChPclPolicyRuleRawExtGetNext function
* @endinternal
*
* @brief   The function gets the Policy Rule Mask, Pattern and Action in Hw format
*
* @note   APPLICABLE DEVICES:      All DxCh3 devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_STATE             - on ruleSize differs from actual one for ruleIndex
*/
static CMD_STATUS wrCpssDxChPclPolicyRuleRawExtGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8 i;

    cmdOsMemSet(inFields,0,CMD_MAX_FIELDS);

    if(state>2)
     {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
     }
    switch(state){
    case 0:
        for(i=0;i<MASK_SIZE_WORDS;i++)
         inFields[i+1]=maskPclBtm[i];
        inFields[0]=0;
        break;
    case 1:
        for(i=0;i<PATTERN_SIZE_WORDS;i++)
         inFields[i+1]=patternPclBtm[i];
        inFields[0]=1;
        break;
    case 2:
        for(i=0;i<ACTION_SIZE_WORDS;i++)
         inFields[i+1]=actionPclBtm[i];
        inFields[0]=2;
        break;
    default:
        break;
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0],  inFields[1],  inFields[2],
                inFields[3],  inFields[4],  inFields[5],  inFields[6],
                inFields[7],  inFields[8],  inFields[9],  inFields[10],
                inFields[11], inFields[12], inFields[13], inFields[14],
                inFields[15], inFields[16], inFields[17], inFields[18],
                inFields[19], inFields[20], inFields[21], inFields[22],
                inFields[23], inFields[24], inFields[25], inFields[26],
                inFields[27], inFields[28], inFields[29], inFields[30],
                inFields[31], inFields[32], inFields[33], inFields[34],
                inFields[35],inFields[36]);


    galtisOutput(outArgs, GT_OK, "%f");


    state++;
    return CMD_OK;
}
/**
* @internal wrCpssDxChPclPolicyRuleRawExtGetFirst function
* @endinternal
*
* @brief   The function gets the Policy Rule Mask, Pattern and Action in Hw format
*
* @note   APPLICABLE DEVICES:      All DxCh3 devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_STATE             - on ruleSize differs from actual one for ruleIndex
*/
static CMD_STATUS wrCpssDxChPclPolicyRuleRawExtGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8 devNum;
    CPSS_PCL_RULE_SIZE_ENT ruleSize;
    GT_U32 ruleIndex;
    GT_STATUS result;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    ruleSize=(CPSS_PCL_RULE_SIZE_ENT)inArgs[1];
    ruleIndex=(GT_U32)inArgs[2];

    /* call cpss api function */
    result = pg_wrap_cpssDxChPclRuleGet(devNum, ruleSize, ruleIndex,maskPclBtm,patternPclBtm,actionPclBtm);


    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    state=0;/*init state*/

    return wrCpssDxChPclPolicyRuleRawExtGetNext(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChPclPortListGroupingEnableSet function
* @endinternal
*
* @brief   Enable/disable port grouping mode per direction. When enabled, the 4 MSB
*         bits of the <Port List> field in the PCL keys are replaced by a 4-bit
*         <Port Group>.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclPortListGroupingEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_PCL_DIRECTION_ENT  direction;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PCL_DIRECTION_ENT)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPclPortListGroupingEnableSet(devNum, direction, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclPortListGroupingEnableGet function
* @endinternal
*
* @brief   Get the port grouping mode enabling status per direction. When enabled,
*         the 4 MSB bits of the <Port List> field in the PCL keys are replaced by a
*         4-bit <Port Group>.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclPortListGroupingEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_PCL_DIRECTION_ENT  direction;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PCL_DIRECTION_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPclPortListGroupingEnableGet(devNum, direction, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclPortListPortMappingSet function
* @endinternal
*
* @brief   Set port group mapping and offset in port list for PCL working in Physical
*         Port List.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When working in PCL Port List mode with port grouping enabled, <offset>
*       value in the range 23..27 has no influence.
*
*/
static CMD_STATUS wrCpssDxChPclPortListPortMappingSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_PCL_DIRECTION_ENT  direction;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL                 enable;
    GT_U32                  group;
    GT_U32                  offset;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PCL_DIRECTION_ENT)inArgs[1];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[2];
    enable = (GT_BOOL)inArgs[3];
    group = (GT_U32)inArgs[4];
    offset = (GT_U32)inArgs[5];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPclPortListPortMappingSet(devNum, direction, portNum,
                                               enable, group, offset);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclPortListPortMappingGet function
* @endinternal
*
* @brief   Get port group mapping and offset in port list for PCL working in Physical
*         Port List.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclPortListPortMappingGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_PCL_DIRECTION_ENT  direction;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL                 enable;
    GT_U32                  group;
    GT_U32                  offset;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PCL_DIRECTION_ENT)inArgs[1];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPclPortListPortMappingGet(devNum, direction, portNum,
                                               &enable, &group, &offset);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", enable, group, offset);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclOamChannelTypeProfileToOpcodeMappingSet function
* @endinternal
*
* @brief   Set Mapping of MPLS Channel Type Profile to OAM opcode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - for out-of-range parameter values.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclOamChannelTypeProfileToOpcodeMappingSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    channelTypeProfile;
    GT_U32    opcode;
    GT_BOOL   RBitAssignmentEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                    = (GT_U8)inArgs[0];
    channelTypeProfile        = (GT_U32)inArgs[1];
    opcode                    = (GT_U32)inArgs[2];
    RBitAssignmentEnable      = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPclOamChannelTypeProfileToOpcodeMappingSet(
        devNum, channelTypeProfile, opcode, RBitAssignmentEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclOamChannelTypeProfileToOpcodeMappingGet function
* @endinternal
*
* @brief   Get Mapping of MPLS Channel Type Profile to OAM opcode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclOamChannelTypeProfileToOpcodeMappingGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    channelTypeProfile;
    GT_U32    opcode;
    GT_BOOL   RBitAssignmentEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                    = (GT_U8)inArgs[0];
    channelTypeProfile        = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPclOamChannelTypeProfileToOpcodeMappingGet(
        devNum, channelTypeProfile, &opcode, &RBitAssignmentEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", opcode, RBitAssignmentEnable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclOamMegLevelModeSet function
* @endinternal
*
* @brief   Set OAM MEG Level Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - for out-of-range parameter values.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclOamMegLevelModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                 result;
    GT_U8                                     devNum;
    CPSS_PCL_DIRECTION_ENT                    direction;
    GT_U32                                    oamProfile;
    CPSS_DXCH_PCL_OAM_PKT_MEG_LEVEL_MODE_ENT  megLevelMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum           = (GT_U8)inArgs[0];
    direction        = (CPSS_PCL_DIRECTION_ENT)inArgs[1];
    oamProfile       = (GT_U32)inArgs[2];
    megLevelMode     = (CPSS_DXCH_PCL_OAM_PKT_MEG_LEVEL_MODE_ENT)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPclOamMegLevelModeSet(
        devNum, direction, oamProfile, megLevelMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclOamMegLevelModeGet function
* @endinternal
*
* @brief   Get OAM MEG Level Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_STATE             - on unexpected value in HW
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclOamMegLevelModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                 result;
    GT_U8                                     devNum;
    CPSS_PCL_DIRECTION_ENT                    direction;
    GT_U32                                    oamProfile;
    CPSS_DXCH_PCL_OAM_PKT_MEG_LEVEL_MODE_ENT  megLevelMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum           = (GT_U8)inArgs[0];
    direction        = (CPSS_PCL_DIRECTION_ENT)inArgs[1];
    oamProfile       = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPclOamMegLevelModeGet(
        devNum, direction, oamProfile, &megLevelMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", megLevelMode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclOamRdiMatchingSet function
* @endinternal
*
* @brief   Set OAM RDI(Remote Direct Indicator) Matching parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - for out-of-range parameter values.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclOamRdiMatchingSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                    result;
    GT_U8                        devNum;
    CPSS_PCL_DIRECTION_ENT       direction;
    GT_U32                       oamProfile;
    GT_U32                       rdiMask;
    GT_U32                       rdiPattern;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum           = (GT_U8)inArgs[0];
    direction        = (CPSS_PCL_DIRECTION_ENT)inArgs[1];
    oamProfile       = (GT_U32)inArgs[2];
    rdiMask          = (GT_U32)inArgs[3];
    rdiPattern       = (GT_U32)inArgs[4];

    /* call cpss api function */
    result = cpssDxChPclOamRdiMatchingSet(
        devNum, direction, oamProfile, rdiMask, rdiPattern);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclOamRdiMatchingGet function
* @endinternal
*
* @brief   Get OAM RDI(Remote Direct Indicator) Matching parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclOamRdiMatchingGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                    result;
    GT_U8                        devNum;
    CPSS_PCL_DIRECTION_ENT       direction;
    GT_U32                       oamProfile;
    GT_U32                       rdiMask;
    GT_U32                       rdiPattern;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum           = (GT_U8)inArgs[0];
    direction        = (CPSS_PCL_DIRECTION_ENT)inArgs[1];
    oamProfile       = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPclOamRdiMatchingGet(
        devNum, direction, oamProfile, &rdiMask, &rdiPattern);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", rdiMask, rdiPattern);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclOamEgressPacketDetectionSet function
* @endinternal
*
* @brief   Set OAM Egress Packet Detection parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - for out-of-range parameter values.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclOamEgressPacketDetectionSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                 result;
    GT_U8                                     devNum;
    CPSS_DXCH_PCL_OAM_EGRESS_DETECT_MODE_ENT  mode;
    GT_U32                                    sourceId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum           = (GT_U8)inArgs[0];
    mode             = (CPSS_DXCH_PCL_OAM_EGRESS_DETECT_MODE_ENT)inArgs[1];
    sourceId         = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPclOamEgressPacketDetectionSet(
        devNum, mode, sourceId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclOamEgressPacketDetectionGet function
* @endinternal
*
* @brief   Get OAM Egress Packet Detection parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclOamEgressPacketDetectionGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                 result;
    GT_U8                                     devNum;
    CPSS_DXCH_PCL_OAM_EGRESS_DETECT_MODE_ENT  mode;
    GT_U32                                    sourceId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum           = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPclOamEgressPacketDetectionGet(
        devNum, &mode, &sourceId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", mode, sourceId);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclOamRFlagMatchingSet function
* @endinternal
*
* @brief   Set OAM R-Flag Matching parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - for out-of-range parameter values.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclOamRFlagMatchingSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS     result;
    GT_U8         devNum;
    GT_U32        oamProfile;
    GT_U32        rFlagMask;
    GT_U32        rFlagPattern;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum           = (GT_U8)inArgs[0];
    oamProfile       = (GT_U32)inArgs[1];
    rFlagMask        = (GT_U32)inArgs[2];
    rFlagPattern     = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPclOamRFlagMatchingSet(
        devNum, oamProfile, rFlagMask, rFlagPattern);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclOamRFlagMatchingGet function
* @endinternal
*
* @brief   Get OAM R-Flag Matching parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclOamRFlagMatchingGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS     result;
    GT_U8         devNum;
    GT_U32        oamProfile;
    GT_U32        rFlagMask;
    GT_U32        rFlagPattern;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum           = (GT_U8)inArgs[0];
    oamProfile       = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPclOamRFlagMatchingGet(
        devNum, oamProfile, &rFlagMask, &rFlagPattern);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", rFlagMask, rFlagPattern);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclEgressSourcePortSelectionModeSet function
* @endinternal
*
* @brief   Set EPCL source port field selection mode for DSA tagged packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note EPCL key <SrcPort> always reflects the local device source physical
*       ingress port, even for DSA-tagged packets. To maintain backward
*       compatibility for DSA-tagged packets, a global mode allows working in
*       the backward compatible mode where <SrcPort> is taken from the DSA tag
*       if packet was received DSA tagged.
*
*/
static CMD_STATUS wrCpssDxChPclEgressSourcePortSelectionModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                           result;
    GT_U8                                               devNum;
    CPSS_DXCH_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ENT portSelectionMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              = (GT_U8)inArgs[0];
    portSelectionMode   =
                (CPSS_DXCH_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPclEgressSourcePortSelectionModeSet(
                                        devNum, portSelectionMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclEgressSourcePortSelectionModeGet function
* @endinternal
*
* @brief   Get EPCL source port field selection mode for DSA tagged packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note EPCL key <SrcPort> always reflects the local device source physical
*       ingress port, even for DSA-tagged packets. To maintain backward
*       compatibility for DSA-tagged packets, a global mode allows working in
*       the backward compatible mode where <SrcPort> is taken from the DSA tag
*       if packet was received DSA tagged.
*
*/
static CMD_STATUS wrCpssDxChPclEgressSourcePortSelectionModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                           result;
    GT_U8                                               devNum;
    CPSS_DXCH_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ENT portSelectionMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum           = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPclEgressSourcePortSelectionModeGet(
                                                    devNum, &portSelectionMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", portSelectionMode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclEgressTargetPortSelectionModeSet function
* @endinternal
*
* @brief   Set EPCL target port field selection mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note EPCL key <TrgPort> always reflects the local device target physical
*       ingress port, even for DSA-tagged packets. To maintain backward
*       compatibility for DSA-tagged packets, a global mode allows working in
*       the backward compatible mode where <TrgPort> is taken from the DSA tag
*       if packet is transmitted DSA tagged.
*
*/
static CMD_STATUS wrCpssDxChPclEgressTargetPortSelectionModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                           result;
    GT_U8                                               devNum;
    CPSS_DXCH_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_ENT portSelectionMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              = (GT_U8)inArgs[0];
    portSelectionMode   =
                (CPSS_DXCH_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPclEgressTargetPortSelectionModeSet(
                                        devNum, portSelectionMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclEgressTargetPortSelectionModeGet function
* @endinternal
*
* @brief   Get EPCL target port field selection mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note EPCL key <TrgPort> always reflects the local device target physical
*       ingress port, even for DSA-tagged packets. To maintain backward
*       compatibility for DSA-tagged packets, a global mode allows working in
*       the backward compatible mode where <TrgPort> is taken from the DSA tag
*       if packet is transmitted DSA tagged.
*
*/
static CMD_STATUS wrCpssDxChPclEgressTargetPortSelectionModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                           result;
    GT_U8                                               devNum;
    CPSS_DXCH_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_ENT portSelectionMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum           = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPclEgressTargetPortSelectionModeGet(
                                                    devNum, &portSelectionMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", portSelectionMode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclSourceIdMaskSet function
* @endinternal
*
* @brief   Set mask so only certain bits in the source ID will be modified due to PCL
*         action.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclSourceIdMaskSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PCL_LOOKUP_NUMBER_ENT  lookupNum;
    GT_U32                      mask;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_U8)inArgs[0];
    lookupNum   = (CPSS_PCL_LOOKUP_NUMBER_ENT)inArgs[1];
    mask        = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPclSourceIdMaskSet(devNum, lookupNum, mask);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclSourceIdMaskGet function
* @endinternal
*
* @brief   Get mask used for source ID modify due to PCL action.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclSourceIdMaskGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PCL_LOOKUP_NUMBER_ENT  lookupNum;
    GT_U32                      mask;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_U8)inArgs[0];
    lookupNum   = (CPSS_PCL_LOOKUP_NUMBER_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPclSourceIdMaskGet(devNum, lookupNum, &mask);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mask);

    return CMD_OK;
}
/**
* @internal wrCpssDxChPclTcamIndexConversionEnableSet function
* @endinternal
*
* @brief   Set the appDemoPclTcamUseIndexConversion for new PCL TCAM mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
*/
static CMD_STATUS wrCpssDxChPclTcamIndexConversionEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_BOOL       enableIndexConversion;
    enableIndexConversion  = (GT_BOOL) inArgs[0];
    appDemoDxChNewPclTcamSupportSet (enableIndexConversion);
    galtisOutput(outArgs, GT_OK, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPclTcamIndexConversionEnableGet function
* @endinternal
*
* @brief   Get the appDemoPclTcamUseIndexConversion for new PCL TCAM mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
*/
static CMD_STATUS wrCpssDxChPclTcamIndexConversionEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_BOOL       enableIndexConversion;
    enableIndexConversion  = (GT_BOOL) inArgs[0];
    appDemoDxChNewPclTcamSupportGet (&enableIndexConversion);
    galtisOutput( outArgs, GT_OK, "%d", enableIndexConversion );
    return CMD_OK;
}

/**
* @internal wrCpssDxChPclEgressRxAnalyzerUseOrigVidEnableSet function
* @endinternal
*
* @brief   Function enables/disables for ingress analyzed packet the using of VID from
*         the incoming packet to access the EPCL Configuration table and for lookup
*         keys generation.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChPclEgressRxAnalyzerUseOrigVidEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPclEgressRxAnalyzerUseOrigVidEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclEgressRxAnalyzerUseOrigVidEnableGet function
* @endinternal
*
* @brief   Get state of the flag in charge of the using of VID of the incoming packet
*         for ingress analyzed packet to access the EPCL Configuration table and for
*         lookup keys generation.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChPclEgressRxAnalyzerUseOrigVidEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   enable;
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPclEgressRxAnalyzerUseOrigVidEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCmdCpssDxChPclRuleSequenceWrite function
* @endinternal
*
* @brief   Write sequence of incremented PclRules.
*         The rule mask copied unchanged, pattern and action incremented.
*
* @retval GT_OK                    - on sucess, other on error.
*/
static CMD_STATUS wrCmdCpssDxChPclRuleSequenceWrite
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          result;
    GT_U8                              devNum;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    GT_U32                             baseRuleIndex;
    GT_U32                             incrRuleIndex;
    GT_U32                             amountToWrite;
    GT_32                              writeIndexIncr;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT       tcamRuleSize;
    GT_U32                             saveBaseRuleIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum         = (GT_U8)inArgs[0];
    ruleFormat     = (CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT)inArgs[1];
    baseRuleIndex  = (GT_U32)inArgs[2];
    incrRuleIndex  = (GT_U32)inArgs[3];
    amountToWrite  = (GT_U32)inArgs[4];
    writeIndexIncr = (GT_32)inArgs[5];

    /* convert rule's format to rule's size */
    PRV_CPSS_DXCH_PCL_CONVERT_RULE_FORMAT_TO_TCAM_RULE_SIZE_VAL_MAC(tcamRuleSize,ruleFormat);
    saveBaseRuleIndex = baseRuleIndex;
    baseRuleIndex = appDemoDxChTcamPclConvertedIndexGet(devNum,baseRuleIndex,tcamRuleSize);
    incrRuleIndex = appDemoDxChTcamPclConvertedIndexGet(devNum,incrRuleIndex,tcamRuleSize);
    writeIndexIncr =
        appDemoDxChTcamPclConvertedIndexGet(
            devNum,(saveBaseRuleIndex + writeIndexIncr),tcamRuleSize)
        - baseRuleIndex;

    /* call cpss api function */
    result = cmdCpssDxChPclRuleSequenceWrite(
    devNum, ruleFormat, baseRuleIndex, incrRuleIndex,
        amountToWrite, writeIndexIncr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPclTcamClientIndexInfoGet function
* @endinternal
*
* @brief   Gets TCAM base index and total number of indexes for client rules
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPclTcamClientIndexInfoGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                   devNum;
    CPSS_DXCH_TCAM_CLIENT_ENT               client;
    GT_U32                                  baseRuleIndex;
    GT_U32                                  totalRules;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    client  = (CPSS_DXCH_TCAM_CLIENT_ENT)inArgs[1];

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        galtisOutput(outArgs, GT_NOT_APPLICABLE_DEVICE, "");
        return CMD_OK;
    }

    switch (client)
    {
        case CPSS_DXCH_TCAM_IPCL_0_E:
        case CPSS_DXCH_TCAM_IPCL_1_E:
        case CPSS_DXCH_TCAM_IPCL_2_E:
            /* call appdemo functions */
            baseRuleIndex = appDemoDxChTcamIpclBaseIndexGet(devNum, client);
            totalRules = appDemoDxChTcamIpclNumOfIndexsGet(devNum, client);
            break;
        case CPSS_DXCH_TCAM_EPCL_E:
            /* call appdemo functions */
            baseRuleIndex = appDemoDxChTcamEpclBaseIndexGet(devNum);
            totalRules = appDemoDxChTcamEpclNumOfIndexsGet(devNum);
            break;
        default:
            galtisOutput(outArgs, GT_BAD_PARAM, "");
            return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%d%d", baseRuleIndex, totalRules);

    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChPclInit",
        &wrCpssDxChPclInit,
        1, 0},

    {"cpssDxChPclUserDefinedByteSet",
        &wrCpssDxChPclUserDefinedByteSet,
        5, 0},

    {"cpssDxChPclUserDefinedByteGet",
        &wrCpssDxChPclUserDefinedByteGet,
        4, 0},

    {"cpssDxChPclL3L4ParsingOverMplsEnableSet",
        &wrCpssDxChPclL3L4ParsingOverMplsEnableSet,
        2, 0},

    {"cpssDxChPclL3L4ParsingOverMplsEnableGet",
        &wrCpssDxChPclL3L4ParsingOverMplsEnableGet,
        1, 0},

    /* call for union table DxChPclRule */

    {"cpssDxChPclRuleSet",
        &wrCpssDxChPclRuleSet,
        2, 100}, /* the fields number is variable (see the function) */

    /* end call for union table DxChPclRule */

    {"cpssDxChPclRuleActionSet",
        &wrCpssDxChPclRuleActionUpdate,
        3, 36},

    /* call for union table DxChPclRule */

    {"cpssDxChPclRuleSet1",
        &wrCpssDxChPclRuleSet_1,
        2, 100}, /* the fields number is variable (see the function) */

    /* end call for union table DxChPclRule */

    {"cpssDxChPclRuleActionSet1",
        &wrCpssDxChPclRuleActionUpdate_1,
        3, 36},

    {"cpssDxChPclRuleActionGetFirst",
        &wrCpssDxChPclRuleActionGet,
        4, 0},

    {"cpssDxChPclRuleActionGetNext",
        &genericTableGetLast,
        4, 0},

    {"cpssDxChPclRuleInvalidate",
        &wrCpssDxChPclRuleInvalidate,
        3, 0},

    {"cpssDxChPclRuleCopy",
        &wrCpssDxChPclRuleCopy,
        4, 0},

    {"cpssDxChPclRuleMatchCounterGet",
        &wrCpssDxChPclRuleMatchCounterGet,
        2, 0},

    {"cpssDxChPclRuleMatchCounterSet",
        &wrCpssDxChPclRuleMatchCounterSet,
        3, 0},

    {"cpssDxChPclPortIngressPolicyEnable",
        &wrCpssDxChPclPortIngressPolicyEnable,
        3, 0},

    {"cpssDxChPclPortIngressPolicyEnableGet",
        &wrCpssDxChPclPortIngressPolicyEnableGet,
        2, 0},

    {"cpssDxChPclPortLookupCfgTabAccessModeSet",
        &wrCpssDxChPclPortLookupCfgTabAccessModeSet,
        5, 0},

    {"cpssDxChPclPortLookupCfgTabAccessModeSet_v1",
        &wrCpssDxChPclPortLookupCfgTabAccessModeSet_V1,
        6, 0},

    {"cpssDxChPclCfgTblAccessModeSet",
        &wrCpssDxChPclCfgTblAccessModeSet,
        1, 5},

    {"cpssDxChPclCfgTblAccessModeGetFirst",
        &wrCpssDxChPclCfgTblAccessModeGetFirst,
        1, 0},

    {"cpssDxChPclCfgTblAccessModeGetNext",
        &wrCpssDxChPclGenericDymmyGetNext,
        1, 0},

    {"cpssDxChPclCfgTblSet",
        &wrCpssDxChPclCfgTblSet,
        9, 5},

    {"cpssDxChPclCfgTblGetFirst",
        &wrCpssDxChPclCfgTblGet,
        9, 0},

    {"cpssDxChPclCfgTblGetNext",
        &wrCpssDxChPclGenericDymmyGetNext,
        9, 0},

    {"cpssDxCh3PclCfgTblSet",
        &wrCpssDxCh3PclCfgTblSet,
        9, 6},

    {"cpssDxCh3PclCfgTblGetFirst",
        &wrCpssDxCh3PclCfgTblGet,
        9, 0},

    {"cpssDxCh3PclCfgTblGetNext",
        &wrCpssDxChPclGenericDymmyGetNext,
        9, 0},

    {"cpssDxChXCatPclCfgTblSet",
        &wrCpssDxChXCatPclCfgTblSet,
        9, 7},

    {"cpssDxChXCatPclCfgTblGetFirst",
        &wrCpssDxChXCatPclCfgTblGet,
        9, 0},

    {"cpssDxChXCatPclCfgTblGetNext",
        &wrCpssDxChPclGenericDymmyGetNext,
        9, 0},

    {"cpssDxChPclLion3CfgTableSet",
        &wrCpssDxChXCatPclCfgTblSet,
        9, 9},

    {"cpssDxChPclLion3CfgTableGetFirst",
        &wrCpssDxChPclLion3CfgTableGet,
        9, 0},

    {"cpssDxChPclLion3CfgTableGetNext",
        &wrCpssDxChPclGenericDymmyGetNext,
        9, 0},

    {"cpssDxChPclIngressPolicyEnable",
        &wrCpssDxChPclIngressPolicyEnable,
        2, 0},

    {"cpssDxChPclIngressPolicyEnableGet",
        &wrCpssDxChPclIngressPolicyEnableGet,
        1, 0},

    {"cpssDxChPclTcamRuleSizeModeSet",
        &wrCpssDxChPclTcamRuleSizeModeSet,
        3, 0},

    {"cpssDxChPclTcamRuleSizeModeGet",
        &wrCpssDxChPclTcamRuleSizeModeGet,
        1, 0},

    {"cpssDxChPclTwoLookupsCpuCodeResolution",
        &wrCpssDxChPclTwoLookupsCpuCodeResolution,
        2, 0},

    {"cpssDxChPclInvalidUdbCmdSet",
        &wrCpssDxChPclInvalidUdbCmdSet,
        2, 0},

    {"cpssDxChPclInvalidUdbCmdGet",
        &wrCpssDxChPclInvalidUdbCmdGet,
        1, 0},

    {"cpssDxChPclIpLengthCheckModeSet",
        &wrCpssDxChPclIpLengthCheckModeSet,
        2, 0},

    {"cpssDxChPclIpLengthCheckModeGet",
        &wrCpssDxChPclIpLengthCheckModeGet,
        1, 0},

    {"cpssDxChPclIpPayloadMinSizeSet",
        &wrCpssDxChPclIpPayloadMinSizeSet,
        2, 0},
    {"cpssDxChPclIpPayloadMinSizeGet",
        &wrCpssDxChPclIpPayloadMinSizeGet,
        1, 0},

    {"cpssDxChPclRuleStateGet",
        &wrCpssDxChPclRuleStateGet,
        2, 0},

    {"cpssDxChPclCfgTblEntryGet",
        &wrCpssDxChPclCfgTblEntryGet,
        3, 0},

    {"cpssDxChPclLookupCfgPortListEnableSet",
        &wrCpssDxChPclLookupCfgPortListEnableSet,
        5, 0},

    {"cpssDxChPclLookupCfgPortListEnableGet",
        &wrCpssDxChPclLookupCfgPortListEnableGet,
        4, 0},

    {"util_cpssDxChPclRuleOptionsSet",
        &wrUtil_cpssDxChPclRuleOptionsSet,
        3, 0},

    {"util_cpssDxChPclPortListBmpSet",
        &wrUtil_cpssDxChPclPortListBmpSet,
        16, 0},

    {"cpssDxCh2PclEgressPolicyEnable",
        &wrCpssDxCh2PclEgressPolicyEnable,
        2, 0},

    {"cpssDxCh2PclEgressPolicyEnableGet",
        &wrCpssDxCh2PclEgressPolicyEnableGet,
        1, 0},

    {"cpssDxCh2EgressPclPacketTypesSet",
        &wrCpssDxCh2EgressPclPacketTypesSet,
        4, 0},

    {"cpssDxCh2EgressPclPacketTypesGet",
        &wrCpssDxCh2EgressPclPacketTypesGet,
        3, 0},

    {"cpssDxCh2PclTcpUdpPortComparatorSet",
        &wrCpssDxCh2PclTcpUdpPortComparatorSet,
        7, 0},

    {"cpssDxCh2PclTcpUdpPortComparatorGet",
        &wrCpssDxCh2PclTcpUdpPortComparatorGet,
        4, 0},

    {"cpssDxChPclPolicyRuleGetFirst",
        &wrCpssDxChPclPolicyRuleGetFirst,
        3, 0},

    {"cpssDxChPclPolicyRuleGetNext",
        &wrCpssDxChPclGenericDymmyGetNext,
        3, 0},


    {"cpssDxChPclRuleValidStatusSet",
         &wrCpssDxChPclRuleValidStatusSet,
         4, 0},

    {"cpssDxChPclEgressForRemoteTunnelStartEnableSet",
         &wrCpssDxChPclEgressForRemoteTunnelStartEnableSet,
         2, 0},

    {"cpssDxChPclEgressForRemoteTunnelStartEnableGet",
         &wrCpssDxChPclEgressForRemoteTunnelStartEnableGet,
         1, 0},

    {"cpssDxChPclEgressTunnelStartPktCfgTabAccessModeSet",
         &wrCpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet,
         2, 0},

    {"cpssDxChPclEgressTunnelStartPktCfgTabAccesModeGet",
         &wrCpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeGet,
         1, 0},

    {"cpssDxCh3PclTunnelTermForceVlanModeEnableSet",
         &wrCpssDxCh3PclTunnelTermForceVlanModeEnableSet,
         2, 0},

    {"cpssDxCh3PclTunnelTermForceVlanModeEnableGet",
         &wrCpssDxCh3PclTunnelTermForceVlanModeEnableGet,
         1, 0},

    {"cpssDxChPclOverrideUserDefinedBytesSet",
         &wrCpssDxChPclOverrideUserDefinedBytesSet,
         19, 0},

    {"cpssDxChPclOverrideUserDefinedBytesSet_1",
         &wrCpssDxChPclOverrideUserDefinedBytesSet_1,
         19, 0},

    {"cpssDxChPclOverrideUserDefinedBytesGet",
         &wrCpssDxChPclOverrideUserDefinedBytesGet,
         1, 0},

    {"cpssDxChPclOverrideUserDefinedBytesGet_1",
         &wrCpssDxChPclOverrideUserDefinedBytesGet_1,
         1, 0},

    {"cpssDxChPclOverUDB_ByTrunkHashSet",
         &wrCpssDxChPclOverrideUserDefinedBytesByTrunkHashSet,
         8, 0},

    {"cpssDxChPclOverUDB_ByTrunkHashGet",
         &wrCpssDxChPclOverrideUserDefinedBytesByTrunkHashGet,
         1, 0},

    {"cpssDxChPclUdeEtherTypeSet",
         &wrCpssDxChPclUdeEtherTypeSet,
         3, 0},

    {"cpssDxChPclUdeEtherTypeGet",
         &wrCpssDxChPclUdeEtherTypeGet,
         2, 0},

    {"cpssDxChPclEgrKeyVidUpModeSet",
         &wrCpssDxChPclEgressKeyFieldsVidUpModeSet,
         2, 0},

    {"cpssDxChPclEgrKeyVidUpModeGet",
         &wrCpssDxChPclEgressKeyFieldsVidUpModeGet,
         1, 0},

    {"cpssDxCh3PclActionSet",
        &wrCpssDxCh3PclActionSet,
        4, 42},

    {"cpssDxCh3PclActionGetFirst",
        &wrCpssDxCh3PclActionGet_Ch3,
        4, 0},

    {"cpssDxCh3PclActionGetNext",
        &genericTableGetLast,
        0, 0},

    {"cpssDxCh3PclActionClear",
        &wrCpssDxCh3PclActionClear,
        0, 0},

    {"cpssDxCh3PclActionDelete",
        &wrCpssDxCh3PclActionDelete,
        2, 0},
     /* --- cpssDxChXCatActionTable --- */

    {"cpssDxChXCatActionTableSet",
        &wrCpssDxCh3PclActionSet,
        4, 51},

    {"cpssDxChXCatActionTableGetFirst",
        &wrCpssDxCh3PclActionGet_XCat,
        4, 0},

    {"cpssDxChXCatActionTableGetNext",
        &genericTableGetLast,
        0, 0},

    {"cpssDxChXCatActionTableClear",
        &wrCpssDxCh3PclActionClear,
        0, 0},

    {"cpssDxChXCatActionTableDelete",
        &wrCpssDxCh3PclActionDelete,
        2, 0},

     /* --- cpssDxChLionActionTable --- */

    {"cpssDxChLionActionTableSet",
        &wrCpssDxCh3PclActionSet,
        4, 50},

    {"cpssDxChLionActionTableGetFirst",
        &wrCpssDxCh3PclActionGet_Lion,
        4, 0},

    {"cpssDxChLionActionTableGetNext",
        &genericTableGetLast,
        0, 0},

    {"cpssDxChLionctionTableClear",
        &wrCpssDxCh3PclActionClear,
        0, 0},

    {"cpssDxChLionActionTableDelete",
        &wrCpssDxCh3PclActionDelete,
        2, 0},

     /* --- cpssDxChPclLion3ActionTable --- */

    {"cpssDxChLion3ActionTableSet",
        &wrCpssDxChPclLion3ActionSet,
        5, 81},

    {"cpssDxChLion3ActionTableGetFirst",
        &wrCpssDxChPclLion3ActionGetFirst,
        5, 0},

    {"cpssDxChLion3ActionTableGetNext",
        &genericTableGetLast,
        0, 0},

    /* --- cpssDxCh3PclIngressRule Table --- */

    {"cpssDxCh3PclIngressRuleSetFirst",
        &wrCpssDxCh3PclIngressRuleWriteFirst,
        2, 30},

    {"cpssDxCh3PclIngressRuleSetNext",
        &wrCpssDxCh3PclIngressRuleWriteNext,
        2, 30},

    {"cpssDxCh3PclIngressRuleEndSet",
        &wrCpssDxCh3PclIngressRuleWriteEnd,
        2, 0},
    /**/

   /* --- cpssDxChPclLion3IngressRule Table --- */

    {"cpssDxChPclLion3IngressRuleSetFirst",
        &wrCpssDxChPclLion3IngressRuleWriteFirst,
        2, 44},

    {"cpssDxChPclLion3IngressRuleSetNext",
        &wrCpssDxChPclLion3IngressRuleWriteNext,
        2, 44},

    {"cpssDxChPclLion3IngressRuleEndSet",
        &wrCpssDxChPclLion3IngressRuleWriteEnd,
        2, 0},
    /**/

    {"cpssDxChPclXCatUDBRuleSetFirst",
        &wrCpssDxChPclXCatUDBRuleWriteFirst,
        2, 30},

    {"cpssDxChPclXCatUDBRuleSetNext",
        &wrCpssDxChPclXCatUDBRuleWriteNext,
        2, 30},

    {"cpssDxChPclXCatUDBRuleEndSet",
        &wrCpssDxCh3PclIngressRuleWriteEnd,
        2, 0},

    /* --- cpssDxCh3PclEgressRule Table --- */

    {"cpssDxCh3PclEgressRuleSetFirst",
        &wrCpssDxCh3PclEgressRuleWriteFirst,
        2, 41},

    {"cpssDxCh3PclEgressRuleSetNext",
        &wrCpssDxCh3PclEgressRuleWriteNext,
        2, 41},

    {"cpssDxCh3PclEgressRuleEndSet",
        &wrCpssDxCh3PclEgressRuleWriteEnd,
        2, 0},

    /* --- cpssDxChPclLion3EgressRule Table --- */

    {"cpssDxChPclLion3EgressRuleSetFirst",
        &wrCpssDxChPclLion3EgressRuleWriteFirst,
        2, 54},

    {"cpssDxChPclLion3EgressRuleSetNext",
        &wrCpssDxChPclLion3EgressRuleWriteNext,
        2, 54},

    {"cpssDxChPclLion3EgressRuleEndSet",
        &wrCpssDxChPclLion3EgressRuleWriteEnd,
        2, 0},
    /**/

    {"utilCpssDxChPclRuleIncrementalSequenceSet",
        &wrUtilCpssDxChPclRuleIncrementalSequenceSet,
        8, 0},

    {"cpssDxChPclUserDefinedByteTableSet",
        &wrCpssDxChPclUserDefinedByteTableSet,
        3, 3},

    {"cpssDxChPclUserDefinedByteTableGetNext",
        &wrCpssDxChPclUserDefinedByteTableGetNext,
        3, 0},

    {"cpssDxChPclUserDefinedByteTableGetFirst",
        &wrCpssDxChPclUserDefinedByteTableGetFirst,
        3, 0},

    {"cpssDxChPclUserDefinedByteTableLion3Set",
        &wrCpssDxChPclUserDefinedByteTableSet,
        3, 3},

    {"cpssDxChPclUserDefinedByteTableLion3GetNext",
        &wrCpssDxChPclUserDefinedByteTableGetNext,
        3, 0},

    {"cpssDxChPclUserDefinedByteTableLion3GetFirst",
        &wrCpssDxChPclUserDefinedByteTableGetFirst,
        3, 0},

    {"cpssDxChPclUserDefinedByteSelectTableSet",
        &wrCpssDxChPclUserDefinedBytesSelectTableSet,
        4, 4},

    {"cpssDxChPclUserDefinedByteSelectTableGetFirst",
        &wrCpssDxChPclUserDefinedBytesSelectTableGetFirst,
        4, 0},

    {"cpssDxChPclUserDefinedByteSelectTableGetNext",
        &wrCpssDxChPclGenericDymmyGetNext,
        4, 0},

    {"cpssDxChPclRuleAnyStateGet",
         &wrCpssDxChPclRuleAnyStateGet,
         3, 0},

    {"utilCpssDxChPclRuleParsedGetParametersSet",
         &wrUtil_cpssDxChPclRuleParsedGetParametersSet,
         3, 0},
    /* copy for temporary tables =========================== */

    {"cpssDxCh3PclEgressRule1GetFirst",
        &wrCpssDxCh3PclEgressRuleReadFirst,
        2, 0},

    {"cpssDxCh3PclEgressRule1GetNext",
        &wrCpssDxCh3PclEgressRuleReadNext,
        2, 0},

    {"cpssDxCh3PclIngressRule1GetFirst",
        &wrCpssDxCh3PclIngressRuleReadFirst,
        2, 0},

    {"cpssDxCh3PclIngressRule1GetNext",
        &wrCpssDxCh3PclIngressRuleReadNext,
        2, 0},

    {"cpssDxChPclXCatUDBRule1GetFirst",
        &wrCpssDxChPclXCatUDBRuleReadFirst,
        2, 0},

    {"cpssDxChPclXCatUDBRule1GetNext",
        &wrCpssDxChPclXCatUDBRuleReadNext,
        2, 0},

    /***********************/
        {"cpssDxChPclLookup0ForRoutedPacketsEnableSet",
         &wrCpssDxChPclLookup0ForRoutedPacketsEnableSet,
         3, 0},
        {"cpssDxChPclLookup0ForRoutedPacketsEnableGet",
         &wrCpssDxChPclLookup0ForRoutedPacketsEnableGet,
         2, 0},
        {"cpssDxChPclLookup1ForNotRoutedPacketsEnbSet",
         &wrCpssDxChPclLookup1ForNotRoutedPacketsEnableSet,
         2, 0},
        {"cpssDxChPclLookup1ForNotRoutedPacketsEnbGet",
         &wrCpssDxChPclLookup1ForNotRoutedPacketsEnableGet,
         1, 0},

        {"cpssDxChPclPortIngressLookup0Sublookup1TypeSet",
             &wrCpssDxChPclPortIngressLookup0Sublookup1TypeSet,
             2, 0},

        {"cpssDxChPclPortIngressLookup0Sublookup1TypeGet",
             &wrCpssDxChPclPortIngressLookup0Sublookup1TypeGet,
             1, 0},
        {"cpssDxChPclIpMinOffsetSet",
             &wrCpssDxChPclIpMinOffsetSet,
             2, 0},
        {"cpssDxChPclIpMinOffsetGet",
             &wrCpssDxChPclIpMinOffsetGet,
             1, 0},
        {"cpssDxChPclTcamEccDaemonEnableSet",
             &wrCpssDxChPclTcamEccDaemonEnableSet,
             2, 0},
        {"cpssDxChPclTcamEccDaemonEnableGet",
             &wrCpssDxChPclTcamEccDaemonEnableGet,
             1, 0},

        /* --- cpssDxChPclRule Table --- */

        {"cpssDxChPclRuleSetFirst",
          &wrCpssDxChPclRuleWriteFirst,
          2, 38},

        {"cpssDxChPclRuleSetNext",
          &wrCpssDxChPclRuleWriteNext,
           2, 38},

        {"cpssDxChPclRuleEndSet",
          &wrCpssDxChPclRuleWriteEnd,
           2, 0},

        {"cpssDxChPclRuleCancelSet",
          &wrCpssDxChPclRuleCancelSet,
           0, 0},


        /* --- cpssDxChPclRuleExt Table --- */

         {"cpssDxChPclRuleExtSetFirst",
          &wrCpssDxChPclRuleExtWriteFirst,
          2, 38},

        {"cpssDxChPclRuleExtSetNext",
          &wrCpssDxChPclRuleExtWriteNext,
           2, 38},

        {"cpssDxChPclRuleExtEndSet",
          &wrCpssDxChPclRuleExtWriteEnd,
           2, 0},

        /*--------------------------------------*/
        {"cpssDxChPclPolicyRuleRawExtGetFirst",
            &wrCpssDxChPclPolicyRuleRawExtGetFirst,
            3, 0},

        {"cpssDxChPclPolicyRuleRawExtGetNext",
            &wrCpssDxChPclPolicyRuleRawExtGetNext,
            3, 0},


        /* --- cpssDxChPclLion3RuleIngressUdbOnlyTable --- */

         {"cpssDxChPclLion3RuleIngressUdbOnlyTableSet",
          &wrCpssDxChPclRuleUdbOnlyIngressSet,
          3, 20 /*21*/},

        /*--------------------------------------*/
        {"cpssDxChPclLion3RuleIngressUdbOnlyTableGetFirst",
            &wrCpssDxChPclRuleUdbOnlyIngressGetFirst,
            3, 0},

        {"cpssDxChPclLion3RuleIngressUdbOnlyTableGetNext",
            &wrCpssDxChPclRuleUdbOnlyIngressGetNext,
            3, 0},

        /* --- cpssDxChPclLion3RuleEgressUdbOnlyTable --- */

         {"cpssDxChPclLion3RuleEgressUdbOnlyTableSet",
          &wrCpssDxChPclRuleUdbOnlyEgressSet,
          3, 11},

        /*--------------------------------------*/
        {"cpssDxChPclLion3RuleEgressUdbOnlyTableGetFirst",
            &wrCpssDxChPclRuleUdbOnlyEgressGetFirst,
            3, 0},

        {"cpssDxChPclLion3RuleEgressUdbOnlyTableGetNext",
            &wrCpssDxChPclRuleUdbOnlyEgressGetNext,
            3, 0},

    /***********************/

    {"cpssDxChPclPortListGroupingEnableSet",
        &wrCpssDxChPclPortListGroupingEnableSet,
        3, 0},

    {"cpssDxChPclPortListGroupingEnableGet",
        &wrCpssDxChPclPortListGroupingEnableGet,
        2, 0},

    {"cpssDxChPclPortListPortMappingSet",
        &wrCpssDxChPclPortListPortMappingSet,
        6, 0},

    {"cpssDxChPclPortListPortMappingGet",
        &wrCpssDxChPclPortListPortMappingGet,
        3, 0},

    {"cpssDxChPclOamChannelTypeToOpcodeMappingSet",
        &wrCpssDxChPclOamChannelTypeProfileToOpcodeMappingSet,
        4, 0},

    {"cpssDxChPclOamChannelTypeToOpcodeMappingGet",
        &wrCpssDxChPclOamChannelTypeProfileToOpcodeMappingGet,
        2, 0},

    {"cpssDxChPclOamMegLevelModeSet",
        &wrCpssDxChPclOamMegLevelModeSet,
        4, 0},

    {"cpssDxChPclOamMegLevelModeGet",
        &wrCpssDxChPclOamMegLevelModeGet,
        3, 0},

    {"cpssDxChPclOamRdiMatchingSet",
        &wrCpssDxChPclOamRdiMatchingSet,
        5, 0},

    {"cpssDxChPclOamRdiMatchingGet",
        &wrCpssDxChPclOamRdiMatchingGet,
        3, 0},

    {"cpssDxChPclOamEgressPacketDetectionSet",
        &wrCpssDxChPclOamEgressPacketDetectionSet,
        3, 0},

    {"cpssDxChPclOamEgressPacketDetectionGet",
        &wrCpssDxChPclOamEgressPacketDetectionGet,
        1, 0},

    {"cpssDxChPclOamRFlagMatchingSet",
        &wrCpssDxChPclOamRFlagMatchingSet,
        4, 0},

    {"cpssDxChPclOamRFlagMatchingGet",
        &wrCpssDxChPclOamRFlagMatchingGet,
        2, 0},

    {"cpssDxChPclEgressSourcePortSelectionModeSet",
        &wrCpssDxChPclEgressSourcePortSelectionModeSet,
        2, 0},

    {"cpssDxChPclEgressSourcePortSelectionModeGet",
        &wrCpssDxChPclEgressSourcePortSelectionModeGet,
        1, 0},

    {"cpssDxChPclEgressTargetPortSelectionModeSet",
        &wrCpssDxChPclEgressTargetPortSelectionModeSet,
        2, 0},

    {"cpssDxChPclEgressTargetPortSelectionModeGet",
        &wrCpssDxChPclEgressTargetPortSelectionModeGet,
        1, 0},

    {"cpssDxChPclSourceIdMaskSet",
        &wrCpssDxChPclSourceIdMaskSet,
        3, 0},

    {"cpssDxChPclSourceIdMaskGet",
        &wrCpssDxChPclSourceIdMaskGet,
        2, 0},

    {"cpssDxChPclTcamIndexConversionEnableSet",
        &wrCpssDxChPclTcamIndexConversionEnableSet,
          1, 0},
    {"cpssDxChPclTcamIndexConversionEnableGet",
         &wrCpssDxChPclTcamIndexConversionEnableGet,
          0, 0},

    {"cpssDxChPclEgressRxAnalyzerUseOrigVidEnableSet",
         &wrCpssDxChPclEgressRxAnalyzerUseOrigVidEnableSet,
         2, 0},

    {"cpssDxChPclEgressRxAnalyzerUseOrigVidEnableGet",
         &wrCpssDxChPclEgressRxAnalyzerUseOrigVidEnableGet,
         1, 0},

    {"cmdCpssDxChPclRuleSequenceWrite",
         &wrCmdCpssDxChPclRuleSequenceWrite,
         6, 0},

    {"appDemoDxChPclTcamClientIndexInfoGet",
         &wrCpssDxChPclTcamClientIndexInfoGet,
         2, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibResetCpssDxChPcl function
* @endinternal
*
* @brief   Library database reset function.
*
* @note none
*
*/
static GT_VOID cmdLibResetCpssDxChPcl
(
    GT_VOID
)
{
    /* Rule Format for RuleParsedGet tables */
    utilCpssDxChPclRuleParsedGet_RuleFormat = 0;
    /* Rule index for RuleParsedGet tables */
    utilCpssDxChPclRuleParsedGet_RuleIndex = 0;
    /* Lines Amount (to output) for RuleParsedGet tables */
    utilCpssDxChPclRuleParsedGet_LinesAmount = 0;

    extPcl= GT_FALSE;
    /* if GT_TRUE == extPcl extended pcl Galtis table is used,
          GT_FALSE otherwise*/

    sip5Pcl= GT_FALSE;
    /* if sip5Pcl==GT_TRUE lion3 pcl Galtis table is used,
          GT_FALSE otherwise*/
    /* default mode - backward compatible     */
    /* the new command  will set these values */
    ruleSet_ruleOptionsBmp = 0;

    MEMSET_0_ELEMENT_MAC(ruleSet_portListBmpMask);
    MEMSET_0_ELEMENT_MAC(ruleSet_portListBmpPattern);

    firstFree = 0;
    firstRun = GT_TRUE;


    mask_set = GT_FALSE; /* is mask set*/
    pattern_set = GT_FALSE; /* is pattern set*/
    mask_ruleIndex = 0;
    pattern_ruleIndex = 0;
}

/**
* @internal cmdLibInitCpssDxChPcl function
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
GT_STATUS cmdLibInitCpssDxChPcl
(
    GT_VOID
)
{
    GT_STATUS rc;

    rc = wrCpssRegisterResetCb(cmdLibResetCpssDxChPcl);
    if(rc != GT_OK)
    {
        return rc;
    }

    return cmdInitLibrary(dbCommands, numCommands);
}


/********************************* UTILITES *************************************/

/*
 * typedef: struct SUBFIELD_DATA_STC
 *
 * Description:
 *      Rule Subfield descriptor
 *
 * Fields:
 *      name           -  subfield name
 *      subfieldOffset -  offset of subfield from the structure origin
 *      subfieldSize   -  size of subfield
 *      elementsAmount -  amount of elements (for not-array - 1)
 *      nodePtr        -  address of substructure table
 *                        (for not-substructure NULL)
 *
 * Comment:
 */
typedef struct __SUBFIELD_DATA
{
    char             *name;            /* name in structure         */
    GT_U32           subfieldOffset;   /* offset in bytes           */
    GT_U32           subfieldSize;     /* size in bytes             */
    GT_U32           elementsAmount;   /* Not 1 means array         */
    struct __SUBFIELD_DATA  *nodePtr;  /* descpiptor of subsructure */
} SUBFIELD_DATA_STC;

#define SUBFIELD_NULL_BASED(_struct, _member) ((_struct*)0)->_member

#define SUBFIELD_OFFSET(_struct, _member) \
    (int)((char*)&(SUBFIELD_NULL_BASED(_struct, _member)) - (char*)0)

#define SUBFIELD_SIZE(_struct, _member) \
    sizeof(SUBFIELD_NULL_BASED(_struct, _member))

/* for array */
#define SUBFIELD_ELEMENT_AMOUNT(_struct, _member) \
    (SUBFIELD_SIZE(_struct, _member) / SUBFIELD_SIZE(_struct, _member[0]))

#define SUBFIELD_END {(char*)0, 0, 0, 0, (SUBFIELD_DATA_STC*)0}

#define SUBFIELD_SCALAR(_struct, _member)   \
    {                                       \
        #_member,                           \
        SUBFIELD_OFFSET(_struct, _member),  \
        SUBFIELD_SIZE(_struct, _member),    \
        1,                                  \
        (SUBFIELD_DATA_STC*)0               \
    }

#define SUBFIELD_SCALAR_DEEP(_struct, _branch, _member)    \
    {                                                      \
        #_member,                                          \
        SUBFIELD_OFFSET(_struct, _branch._member),        \
        SUBFIELD_SIZE(_struct, _branch._member),          \
        1,                                                 \
        (SUBFIELD_DATA_STC*)0                              \
    }

#define SUBFIELD_NODE(_struct, _member, _scriptPtr)     \
    {                                                   \
        #_member,                                       \
        SUBFIELD_OFFSET(_struct, _member),              \
        SUBFIELD_SIZE(_struct, _member),                \
        1,                                              \
        _scriptPtr                                      \
    }

/* for GT_IPV6ADDR member      _hidden_path = ".arIP"    */
/* for GT_ETHERADDR member     _hidden_path = ".arEther" */
#define SUBFIELD_ARRAY(_struct, _member, _hidden_path)            \
    {                                                             \
        #_member,                                                 \
        SUBFIELD_OFFSET(_struct, _member),          \
        SUBFIELD_SIZE(_struct, _member),            \
        SUBFIELD_ELEMENT_AMOUNT(_struct, _member._hidden_path),  \
        (SUBFIELD_DATA_STC*)0                                     \
    }

#define SUBFIELD_ARRAY_PURE(_struct, _member)       \
    {                                               \
        #_member,                                   \
        SUBFIELD_OFFSET(_struct, _member),          \
        SUBFIELD_SIZE(_struct, _member),            \
        SUBFIELD_ELEMENT_AMOUNT(_struct, _member),  \
        (SUBFIELD_DATA_STC*)0                       \
    }

#undef _STR
#define _STR CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC
static SUBFIELD_DATA_STC subfields_RULE_FORMAT_COMMON_STC[] =
{
    SUBFIELD_SCALAR(_STR, pclId),
    SUBFIELD_SCALAR(_STR, sourcePort),
    SUBFIELD_SCALAR(_STR, vid),
    SUBFIELD_SCALAR(_STR, up),
    SUBFIELD_SCALAR(_STR, qosProfile),
    SUBFIELD_END
};

#undef _STR
#define _STR CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC
static SUBFIELD_DATA_STC subfields_RULE_FORMAT_COMMON_EXT_STC[] =
{
    SUBFIELD_SCALAR(_STR, ipProtocol),
    SUBFIELD_SCALAR(_STR, dscp),
    SUBFIELD_SCALAR(_STR, l4Byte0),
    SUBFIELD_SCALAR(_STR, l4Byte1),
    SUBFIELD_SCALAR(_STR, l4Byte2),
    SUBFIELD_SCALAR(_STR, l4Byte3),
    SUBFIELD_SCALAR(_STR, l4Byte13),
    SUBFIELD_END
};

#undef _STR
#define _STR CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC
static SUBFIELD_DATA_STC subfields_RULE_FORMAT_COMMON_STD_IP_STC[] =
{
    SUBFIELD_SCALAR(_STR, ipProtocol),
    SUBFIELD_SCALAR(_STR, dscp),
    SUBFIELD_SCALAR(_STR, l4Byte2),
    SUBFIELD_SCALAR(_STR, l4Byte3),
    SUBFIELD_END
};

#undef _STR
#define _STR CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC
static SUBFIELD_DATA_STC subfields_RULE_FORMAT_STD_NOT_IP_STC[] =
{
    SUBFIELD_NODE(_STR, common, subfields_RULE_FORMAT_COMMON_STC),
    SUBFIELD_SCALAR(_STR, etherType),
    SUBFIELD_ARRAY(_STR, macDa, arEther),
    SUBFIELD_ARRAY(_STR, macSa, arEther),
    SUBFIELD_SCALAR(_STR, udb15_17[0]),
    SUBFIELD_SCALAR(_STR, udb15_17[1]),
    SUBFIELD_SCALAR(_STR, udb15_17[2]),
    SUBFIELD_END
};

#undef _STR
#define _STR CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC
static SUBFIELD_DATA_STC subfields_RULE_FORMAT_STD_IP_L2_QOS_STC[] =
{
    SUBFIELD_NODE(_STR, common, subfields_RULE_FORMAT_COMMON_STC),
    SUBFIELD_NODE(_STR, commonStdIp, subfields_RULE_FORMAT_COMMON_STD_IP_STC),
    SUBFIELD_ARRAY(_STR, macDa, arEther),
    SUBFIELD_ARRAY(_STR, macSa, arEther),
    SUBFIELD_SCALAR(_STR, udb18_19[0]),
    SUBFIELD_SCALAR(_STR, udb18_19[1]),
    SUBFIELD_END
};

#undef _STR
#define _STR CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_L4_STC
static SUBFIELD_DATA_STC subfields_RULE_FORMAT_STD_IPV4_L4_STC[] =
{
    SUBFIELD_NODE(_STR, common, subfields_RULE_FORMAT_COMMON_STC),
    SUBFIELD_NODE(_STR, commonStdIp, subfields_RULE_FORMAT_COMMON_STD_IP_STC),
    SUBFIELD_ARRAY(_STR, sip, arIP),
    SUBFIELD_ARRAY(_STR, dip, arIP),
    SUBFIELD_SCALAR(_STR, l4Byte0),
    SUBFIELD_SCALAR(_STR, l4Byte1),
    SUBFIELD_SCALAR(_STR, l4Byte13),
    SUBFIELD_SCALAR(_STR, udb20_22[0]),
    SUBFIELD_SCALAR(_STR, udb20_22[1]),
    SUBFIELD_SCALAR(_STR, udb20_22[2]),
    SUBFIELD_END
};

#undef _STR
#define _STR CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC
static SUBFIELD_DATA_STC subfields_RULE_FORMAT_EXT_NOT_IPV6_STC[] =
{
    SUBFIELD_NODE(_STR, common, subfields_RULE_FORMAT_COMMON_STC),
    SUBFIELD_NODE(_STR, commonExt, subfields_RULE_FORMAT_COMMON_EXT_STC),
    SUBFIELD_ARRAY(_STR, sip, arIP),
    SUBFIELD_ARRAY(_STR, dip, arIP),
    SUBFIELD_SCALAR(_STR, etherType),
    SUBFIELD_ARRAY(_STR, macDa, arEther),
    SUBFIELD_ARRAY(_STR, macSa, arEther),
    SUBFIELD_SCALAR(_STR, udb0_5[0]),
    SUBFIELD_SCALAR(_STR, udb0_5[1]),
    SUBFIELD_SCALAR(_STR, udb0_5[2]),
    SUBFIELD_SCALAR(_STR, udb0_5[3]),
    SUBFIELD_SCALAR(_STR, udb0_5[4]),
    SUBFIELD_SCALAR(_STR, udb0_5[5]),
    SUBFIELD_END
};

#undef _STR
#define _STR CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L2_STC
static SUBFIELD_DATA_STC subfields_RULE_FORMAT_EXT_IPV6_L2_STC[] =
{
    SUBFIELD_NODE(_STR, common, subfields_RULE_FORMAT_COMMON_STC),
    SUBFIELD_NODE(_STR, commonExt, subfields_RULE_FORMAT_COMMON_EXT_STC),
    SUBFIELD_ARRAY(_STR, sip, arIP),
    SUBFIELD_SCALAR(_STR, dipBits127to120),
    SUBFIELD_ARRAY(_STR, macDa, arEther),
    SUBFIELD_ARRAY(_STR, macSa, arEther),
    SUBFIELD_SCALAR(_STR, udb6_11[0]),
    SUBFIELD_SCALAR(_STR, udb6_11[1]),
    SUBFIELD_SCALAR(_STR, udb6_11[2]),
    SUBFIELD_SCALAR(_STR, udb6_11[3]),
    SUBFIELD_SCALAR(_STR, udb6_11[4]),
    SUBFIELD_SCALAR(_STR, udb6_11[5]),
    SUBFIELD_END
};

#undef _STR
#define _STR CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L4_STC
static SUBFIELD_DATA_STC subfields_RULE_FORMAT_EXT_IPV6_L4_STC[] =
{
    SUBFIELD_NODE(_STR, common, subfields_RULE_FORMAT_COMMON_STC),
    SUBFIELD_NODE(_STR, commonExt, subfields_RULE_FORMAT_COMMON_EXT_STC),
    SUBFIELD_ARRAY(_STR, sip, arIP),
    SUBFIELD_ARRAY(_STR, dip, arIP),
    SUBFIELD_SCALAR(_STR, udb12_14[0]),
    SUBFIELD_SCALAR(_STR, udb12_14[1]),
    SUBFIELD_SCALAR(_STR, udb12_14[2]),
    SUBFIELD_END
};

#undef _STR
#define _STR CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC
static SUBFIELD_DATA_STC subfields_RULE_FORMAT_EGRESS_COMMON_STC[] =
{
    SUBFIELD_SCALAR(_STR, pclId),
    SUBFIELD_SCALAR(_STR, sourcePort),
    SUBFIELD_SCALAR(_STR, vid),
    SUBFIELD_SCALAR(_STR, up),
    SUBFIELD_SCALAR(_STR, egrPacketType),
    SUBFIELD_SCALAR_DEEP(_STR, toCpu, cpuCode),
    SUBFIELD_SCALAR_DEEP(_STR, fromCpu, tc),
    SUBFIELD_SCALAR_DEEP(_STR, fromCpu, dp),
    SUBFIELD_SCALAR_DEEP(_STR, fwdData, qosProfile),
    SUBFIELD_SCALAR_DEEP(_STR, fwdData, srcTrunkId),
    SUBFIELD_SCALAR(_STR, srcHwDev),
    SUBFIELD_SCALAR(_STR, sourceId),
    SUBFIELD_END
};

#undef _STR
#define _STR CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC
static SUBFIELD_DATA_STC subfields_RULE_FORMAT_EGRESS_COMMON_EXT_STC[] =
{
    SUBFIELD_SCALAR(_STR, ipProtocol),
    SUBFIELD_SCALAR(_STR, dscp),
    SUBFIELD_SCALAR(_STR, l4Byte0),
    SUBFIELD_SCALAR(_STR, l4Byte1),
    SUBFIELD_SCALAR(_STR, l4Byte2),
    SUBFIELD_SCALAR(_STR, l4Byte3),
    SUBFIELD_SCALAR(_STR, l4Byte13),
    SUBFIELD_SCALAR(_STR, egrTcpUdpPortComparator),
    SUBFIELD_END
};

#undef _STR
#define _STR CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STD_IP_STC
static SUBFIELD_DATA_STC subfields_RULE_FORMAT_EGRESS_COMMON_STD_IP_STC[] =
{
    SUBFIELD_SCALAR(_STR, ipProtocol),
    SUBFIELD_SCALAR(_STR, dscp),
    SUBFIELD_SCALAR(_STR, l4Byte2),
    SUBFIELD_SCALAR(_STR, l4Byte3),
    SUBFIELD_SCALAR(_STR, egrTcpUdpPortComparator),
    SUBFIELD_END
};

#undef _STR
#define _STR CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_STC
static SUBFIELD_DATA_STC subfields_RULE_FORMAT_EGRESS_STD_NOT_IP_STC[] =
{
    SUBFIELD_NODE(_STR, common, subfields_RULE_FORMAT_EGRESS_COMMON_STC),
    SUBFIELD_SCALAR(_STR, etherType),
    SUBFIELD_ARRAY(_STR, macDa, arEther),
    SUBFIELD_ARRAY(_STR, macSa, arEther),
    SUBFIELD_END
};

#undef _STR
#define _STR CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_STC
static SUBFIELD_DATA_STC subfields_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_STC[] =
{
    SUBFIELD_NODE(_STR, common, subfields_RULE_FORMAT_EGRESS_COMMON_STC),
    SUBFIELD_NODE(_STR, commonStdIp, subfields_RULE_FORMAT_EGRESS_COMMON_STD_IP_STC),
    SUBFIELD_ARRAY_PURE(_STR, dipBits0to31),
    SUBFIELD_SCALAR(_STR, l4Byte13),
    SUBFIELD_ARRAY(_STR, macDa, arEther),
    SUBFIELD_END
};

#undef _STR
#define _STR CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_STC
static SUBFIELD_DATA_STC subfields_RULE_FORMAT_EGRESS_STD_IPV4_L4_STC[] =
{
    SUBFIELD_NODE(_STR, common, subfields_RULE_FORMAT_EGRESS_COMMON_STC),
    SUBFIELD_NODE(_STR, commonStdIp, subfields_RULE_FORMAT_EGRESS_COMMON_STD_IP_STC),
    SUBFIELD_ARRAY(_STR, sip, arIP),
    SUBFIELD_ARRAY(_STR, dip, arIP),
    SUBFIELD_SCALAR(_STR, l4Byte0),
    SUBFIELD_SCALAR(_STR, l4Byte1),
    SUBFIELD_SCALAR(_STR, l4Byte13),
    SUBFIELD_END
};

#undef _STR
#define _STR CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC
static SUBFIELD_DATA_STC subfields_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC[] =
{
    SUBFIELD_NODE(_STR, common, subfields_RULE_FORMAT_EGRESS_COMMON_STC),
    SUBFIELD_NODE(_STR, commonExt, subfields_RULE_FORMAT_EGRESS_COMMON_EXT_STC),
    SUBFIELD_ARRAY(_STR, sip, arIP),
    SUBFIELD_ARRAY(_STR, dip, arIP),
    SUBFIELD_SCALAR(_STR, etherType),
    SUBFIELD_ARRAY(_STR, macDa, arEther),
    SUBFIELD_ARRAY(_STR, macSa, arEther),
    SUBFIELD_END
};

#undef _STR
#define _STR CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_STC
static SUBFIELD_DATA_STC subfields_RULE_FORMAT_EGRESS_EXT_IPV6_L2_STC[] =
{
    SUBFIELD_NODE(_STR, common, subfields_RULE_FORMAT_EGRESS_COMMON_STC),
    SUBFIELD_NODE(_STR, commonExt, subfields_RULE_FORMAT_EGRESS_COMMON_EXT_STC),
    SUBFIELD_ARRAY(_STR, sip, arIP),
    SUBFIELD_SCALAR(_STR, dipBits127to120),
    SUBFIELD_ARRAY(_STR, macDa, arEther),
    SUBFIELD_ARRAY(_STR, macSa, arEther),
    SUBFIELD_END
};

#undef _STR
#define _STR CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_STC
static SUBFIELD_DATA_STC subfields_RULE_FORMAT_EGRESS_EXT_IPV6_L4_STC[] =
{
    SUBFIELD_NODE(_STR, common, subfields_RULE_FORMAT_EGRESS_COMMON_STC),
    SUBFIELD_NODE(_STR, commonExt, subfields_RULE_FORMAT_EGRESS_COMMON_EXT_STC),
    SUBFIELD_ARRAY(_STR, sip, arIP),
    SUBFIELD_ARRAY(_STR, dip, arIP),
    SUBFIELD_END
};

/*******************************************************************************
* prvUtilSubfieldRootGet
*
* DESCRIPTION:
*     Retrieves root subfield descriptors table by rule format
*
* INPUTS:
*     checkRuleFormat    - rule format
*
* OUTPUTS:
*     None
*
*
* RETURNS:
*     subfield descriptor address - on success.
*     NULL                        - on failure.
*
* COMMENTS:
*
*******************************************************************************/
static SUBFIELD_DATA_STC* prvUtilSubfieldRootGet
(
    IN CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT checkRuleFormat
)
{
    switch (checkRuleFormat)
    {
        default: break;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
            return subfields_RULE_FORMAT_STD_NOT_IP_STC;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
            return subfields_RULE_FORMAT_STD_IP_L2_QOS_STC;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
            return subfields_RULE_FORMAT_STD_IPV4_L4_STC;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
            return subfields_RULE_FORMAT_EXT_NOT_IPV6_STC;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
            return subfields_RULE_FORMAT_EXT_IPV6_L2_STC;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
            return subfields_RULE_FORMAT_EXT_IPV6_L4_STC;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E:
            return subfields_RULE_FORMAT_EGRESS_STD_NOT_IP_STC;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E:
            return subfields_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_STC;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E:
            return subfields_RULE_FORMAT_EGRESS_STD_IPV4_L4_STC;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E:
            return subfields_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E:
            return subfields_RULE_FORMAT_EGRESS_EXT_IPV6_L2_STC;

        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E:
            return subfields_RULE_FORMAT_EGRESS_EXT_IPV6_L4_STC;
    }

    return (SUBFIELD_DATA_STC*)0;
}

/*******************************************************************************
* prvUtilSubfieldGet
*
* DESCRIPTION:
*     Retrieves subfield descriptor and calculates total offset
*
* INPUTS:
*     checkRuleFormat    - rule format
*     field_full_name    - the composed name of incremented field
*                          names used in CPSS_DXCH_PCL_RULE_FORMAT_..._STC
*                          sructure separated by '.'.For example:
*                         "common.sourcePort" or "macDa"
*
* OUTPUTS:
*     offsetPtr         - field offset from the rule origin
*
*
* RETURNS:
*     subfield descriptor address - on success.
*     NULL                        - on failure.
*
* COMMENTS:
*
*******************************************************************************/
static SUBFIELD_DATA_STC* prvUtilSubfieldGet
(
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT checkRuleFormat,
    IN  char                               *full_name,
    OUT GT_U32                              *offsetPtr
)
{
    SUBFIELD_DATA_STC* sfPtr;
    char*              path = full_name;
    GT_U32             name_len;

    *offsetPtr = 0;

    sfPtr = prvUtilSubfieldRootGet(checkRuleFormat);

    while (sfPtr != (SUBFIELD_DATA_STC*)0)
    {
        /* measure length of the current name in the path */
        for (name_len = 0;
              ((path[name_len] != '.') && (path[name_len] != 0));
              name_len ++) {};

        /* lookm for name from the path in the fields list */
        for (; (sfPtr->name != 0); sfPtr ++)
        {
            /* another length */
            if (sfPtr->name[name_len] != 0)
                continue;

            /* another contents */
            if (cmdOsMemCmp(sfPtr->name, path, name_len) != 0)
                continue;

            /* found */
            break;
        }

        /* not found */
        if (sfPtr->name == 0)
        {
            /* end of list reached */
            return (SUBFIELD_DATA_STC*)0;
        }

        /* accomulate offset */
        *offsetPtr += sfPtr->subfieldOffset;

        /* end of path */
        if (path[name_len] == 0)
        {
            return sfPtr;
        }

        /* down to substructure */
        sfPtr = sfPtr->nodePtr;

        /* to the next name in the path */
        path += (name_len + 1);
    }

    return (SUBFIELD_DATA_STC*)0;
}

/**
* @internal prvUtilSubfieldIncrement function
* @endinternal
*
* @brief   Increments the field
*
* @param[in,out] fieldPtr                 - field address
* @param[in] fieldSizeof              - size of field in bytes
* @param[in] elementAmount            - amount of elements in array, 1 for scalar
* @param[in] elementStart             - index of array element to increment from
* @param[in] increment                - increment
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note Supported only network oredered byte array
*       The elementStart is offset from the last array element
*       see testUtilCpssDxChPclRuleIncrementalSequenceSet's comment
*
*/
static GT_STATUS prvUtilSubfieldIncrement
(
    INOUT GT_U8              *fieldPtr,
    IN    GT_U32             fieldSizeof,
    IN    GT_U32             elementAmount,
    IN    GT_U32             elementStart,
    IN    GT_U32             increment
)
{
    GT_U32 index;
    GT_U32 maxIndex;
    GT_U32 realIndex;
    GT_U32 data;
    GT_U32 carryIndex;

    /* array */
    if (elementAmount != 1)
    {
        if (elementAmount != fieldSizeof)
        {
            /* only byte network ordered array supported */
            return GT_FAIL;
        }

        if (elementStart >= elementAmount)
        {
            /* start byte out of array */
            return GT_FAIL;
        }

        /* increment is 32 bits == 4 bytes */
        maxIndex = 3;
        if ((elementStart + maxIndex) >= elementAmount)
        {
            maxIndex = elementAmount - elementStart - 1;
        }

        for (index = 0; (index <= maxIndex); index ++)
        {
            /* network ordered array */
            realIndex = elementAmount - elementStart - 1 - index;

            data = ((increment >> (index * 8)) & 0xFF)
                + (GT_U32)(fieldPtr[realIndex]);

            fieldPtr[realIndex] = (GT_U8)data;

            /* carry */
            for (carryIndex = realIndex;
                  ((data & 0x0100) && (carryIndex > 0));
                  carryIndex --)
            {
                data = (GT_U32)(fieldPtr[carryIndex - 1]) + 1;
                fieldPtr[carryIndex - 1] = (GT_U8)data;
            }
        }

        return GT_OK;
    }

    /* SCALARS 1,2,4 byte-sized */
    if (elementStart)
    {
        return GT_FAIL;
    }

    switch (fieldSizeof)
    {
        case 1:
            *(GT_U8*)fieldPtr  += (GT_U8)increment;
            break;
        case 2:
            *(GT_U16*)fieldPtr += (GT_U16)increment;
            break;
        case 4:
            *(GT_U32*)fieldPtr += (GT_U32)increment;
            break;
        default: return GT_FAIL;
    }

    return GT_OK;
}

/**
* @internal utilCpssDxChPclRuleIncrementalSequenceSet function
* @endinternal
*
* @brief   Writes sequence of rules.
*         The Rules calculated by incremental modification of last written rule
* @param[in] devNum                   - device number
* @param[in] checkRuleFormat          - the format of last written rule
* @param[in] ruleIndexBase            - rule Index Base
* @param[in] ruleIndexIncrement       - rule Index Increment
* @param[in] rulesAmount              - rules Amount
*                                      the i-th rule index is
* @param[in] ruleIndexBase            + (i  ruleIndexIncrement)
* @param[in] field_full_name          - the composed name of incremented field
*                                      names used in CPSS_DXCH_PCL_RULE_FORMAT_..._STC
*                                      sructure separated by '.'.For example:
*                                      "common.sourcePort" or "macDa"
* @param[in] elementStart             - for scalar field - 0; for array the index of
* @param[in] increment                array element
* @param[in] increment                - value
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note Only network ordered byte array supported
*       Example (start from 1-th element, carry of overflow bit)
*       checkRuleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E
*       field_full_name = macDa
*       elementStart  = 1
*       increment    = 0x01020080
*       MAC_DA states   00:00:00:00:00:00
*       00:01:02:00:80:00
*       00:02:04:01:00:00
*       00:03:06:01:80:00
*       00:04:08:02:00:00
*
*/
GT_STATUS utilCpssDxChPclRuleIncrementalSequenceSet
(
    IN GT_U8                              devNum,
    IN CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT checkRuleFormat,
    IN GT_U32                             ruleIndexBase,
    IN GT_U32                             ruleIndexIncrement,
    IN GT_U32                             rulesAmount,
    IN char                               *field_full_name,
    IN GT_U32                             elementStart, /* start array element */
    IN GT_U32                             increment
)
{
    GT_STATUS          result;
    SUBFIELD_DATA_STC* sfPtr;
    GT_U32             offset;
    GT_U32             i;
    GT_U8              *fieldPtr;

    if (checkRuleFormat != ruleFormat)
    {
        return GT_BAD_STATE;
    }

    sfPtr = prvUtilSubfieldGet(
        checkRuleFormat, field_full_name, &offset);
    if (sfPtr == 0)
    {
        return GT_FAIL;
    }

    fieldPtr = (GT_U8*)&patternData + offset;

    for (i = 0; (i < rulesAmount); i++)
    {
         /* call cpss api function */
        result = pg_wrap_cpssDxChPclRuleSet(
            devNum, checkRuleFormat,
            (ruleIndexBase + (i * ruleIndexIncrement)),
            &maskData, &patternData, lastActionPtr);
        if (result != GT_OK)
        {
            return result;
        }

        result = prvUtilSubfieldIncrement(
            fieldPtr,
            sfPtr->subfieldSize, sfPtr->elementsAmount,
            elementStart, increment);
        if (result != GT_OK)
        {
            return result;
        }
   }

    return GT_OK;
}
/**
* @internal utilCpssDxChPclRuleDump function
* @endinternal
*
* @brief   Dump PCL rule
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] ruleSize                 - size of rule
* @param[in] ruleIndex                - index of rule
* @param[in] maskArr[]                - mask words array
* @param[in] patternArr[]             - pattern words array
* @param[in] actionArr[]              - action words array
*                                       NONE
*/
static void utilCpssDxChPclRuleDump
(
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleIndex,
    IN GT_U32                             maskArr[],
    IN GT_U32                             patternArr[],
    IN GT_U32                             actionArr[]
)
{
    GT_U32 wordsNum;
    GT_U32 i;
    static GT_U32 firstLineWords = 8;
    static GT_U32 nextLineWords = 10;
    /* strings with the same printed length */
    static char maskHdr[]   =   "# %04X M ";
    static char pattHdr[]   = "\n       P ";
    static char actnHdr[]   = "\n       A ";
    static char lineBreak[] = "\n         ";


    /* ULTRA rule for XCAT2 hal unused high halfwords in words 6,13,20 */
    /* for SIP5 it is 80 bytes and 20 words is enough                 */
    switch (ruleSize)
    {
        case CPSS_PCL_RULE_SIZE_STD_E:       wordsNum =   8;  break;
        case CPSS_PCL_RULE_SIZE_EXT_E:       wordsNum =  15;  break;
        case CPSS_PCL_RULE_SIZE_ULTRA_E:     wordsNum =  21;  break;
        case CPSS_PCL_RULE_SIZE_10_BYTES_E:  wordsNum =   3;  break;
        case CPSS_PCL_RULE_SIZE_20_BYTES_E:  wordsNum =   5;  break;
        case CPSS_PCL_RULE_SIZE_40_BYTES_E:  wordsNum =  10;  break;
        case CPSS_PCL_RULE_SIZE_50_BYTES_E:  wordsNum =  13;  break;
        default:                             wordsNum =  20;  break;
    }

    /* mask */
    cmdOsPrintf(maskHdr, ruleIndex);
    for (i = 0; (i < wordsNum); i++)
    {
        if ((i % nextLineWords) == firstLineWords)
        {
            cmdOsPrintf(lineBreak);
        }
        cmdOsPrintf("%08X ", maskArr[i]);
    }

    /* pattern */
    cmdOsPrintf(pattHdr);
    for (i = 0; (i < wordsNum); i++)
    {
        if ((i % nextLineWords) == firstLineWords)
        {
            cmdOsPrintf(lineBreak);
        }
        cmdOsPrintf("%08X ", patternArr[i]);
    }

    /* action */
    cmdOsPrintf(actnHdr);
    for (i = 0; (i < CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS); i++)
    {
        cmdOsPrintf("%08X ", actionArr[i]);
    }
    cmdOsPrintf("\n");
}

/**
* @internal utilCpssDxChPclRulesDump function
* @endinternal
*
* @brief   Dump all valid PCL rules of specified size.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group
* @param[in] ruleSize                 - size of rule
* @param[in] startIndex               - index of first rule
* @param[in] rulesAmount              - number of rules to scan and dump
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_BAD_STATE             - For DxCh3 and above if cannot determinate
*                                       the rule size by found X/Y bits of compare mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS utilCpssDxChPclRulesDump
(
    IN GT_U8                              devNum,
    IN GT_U32                             portGroup,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             startIndex,
    IN GT_U32                             rulesAmount
)
{
    GT_U32                  tcamRuleIndex;
    GT_PORT_GROUPS_BMP      portGroupsBmp = (1 << portGroup);
    CPSS_DXCH_TCAM_RULE_SIZE_ENT tcamRuleSize;

    /* convert pcl rule's size to tcam rule's size */
    PRV_CPSS_DXCH_PCL_CONVERT_RULE_SIZE_TO_TCAM_RULE_SIZE_VAL_MAC(tcamRuleSize, ruleSize);
    tcamRuleIndex = appDemoDxChTcamPclConvertedIndexGet(devNum, startIndex, tcamRuleSize);
    return cpssDxChPclRulesDump(devNum, portGroupsBmp, ruleSize, tcamRuleIndex,
        rulesAmount);
}


GT_STATUS utilCpssDxChPclRulesDump_raw
(
    IN GT_U8                              devNum,
    IN GT_U32                             portGroup,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             startIndex,
    IN GT_U32                             rulesAmount
)
{
    GT_U32                  ruleIndex;
    GT_U32                  tcamRuleIndex;
    CPSS_PCL_RULE_SIZE_ENT  actualSize;
    GT_BOOL                 isValid;
    GT_U32                  actionArr[CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS];
    GT_U32                  maskArr[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS];
    GT_U32                  patternArr[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS];
    GT_PORT_GROUPS_BMP      portGroupsBmp = (1 << portGroup);
    GT_STATUS               rc;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT tcamRuleSize;

    /* convert pcl rule's size to tcam rule's size */
    PRV_CPSS_DXCH_PCL_CONVERT_RULE_SIZE_TO_TCAM_RULE_SIZE_VAL_MAC(tcamRuleSize, ruleSize);

    for (ruleIndex = startIndex; ruleIndex <= (startIndex + rulesAmount); ruleIndex++)
    {
        switch(tcamRuleSize)
        {
            case CPSS_DXCH_TCAM_RULE_SIZE_10_B_E:
                tcamRuleIndex = ruleIndex;
                break;
            case CPSS_DXCH_TCAM_RULE_SIZE_20_B_E:
                tcamRuleIndex =  (ruleIndex*2);
                break;
            case CPSS_DXCH_TCAM_RULE_SIZE_40_B_E:
            case CPSS_DXCH_TCAM_RULE_SIZE_50_B_E:
            case CPSS_DXCH_TCAM_RULE_SIZE_60_B_E:
                tcamRuleIndex =  (ruleIndex*6);
                break;
            case CPSS_DXCH_TCAM_RULE_SIZE_80_B_E:
                tcamRuleIndex =  (ruleIndex*12);
                break;
            case CPSS_DXCH_TCAM_RULE_SIZE_30_B_E:
            default:
                tcamRuleIndex =  (ruleIndex*3);
                break;
        }


        rc = cpssDxChPclPortGroupRuleAnyStateGet(
            devNum, portGroupsBmp, ruleSize, tcamRuleIndex,
            &isValid, &actualSize);
        if ((rc != GT_OK)  && (rc != GT_BAD_STATE))
        {
            cmdOsPrintf("cpssDxChPclPortGroupRuleAnyStateGet error: index %d rc %d\n", tcamRuleIndex, rc);
            return rc;
        }

        if ((isValid == GT_FALSE) || (rc != GT_OK) || (actualSize != ruleSize))
        {
            continue;
        }

        rc = cpssDxChPclPortGroupRuleGet(
            devNum, portGroupsBmp, ruleSize, tcamRuleIndex,
                                         maskArr, patternArr, actionArr);
        if (rc != GT_OK)
        {
            cmdOsPrintf("cpssDxChPclRuleGet error: index %d rc %d\n", tcamRuleIndex, rc);
            return rc;
        }

        /* dump rule */
        utilCpssDxChPclRuleDump(ruleSize, tcamRuleIndex, maskArr, patternArr, actionArr);
    }

    return GT_OK;
}


GT_STATUS utilCpssDxChPclRulesValidateSet
(
    IN GT_U8                              devNum,
    IN GT_U32                             portGroup,
    IN GT_U32                             startIndex,
    IN GT_U32                             rulesAmount,
    IN GT_BOOL                            valid
)
{
    GT_U32                  ruleIndex;
    GT_PORT_GROUPS_BMP      portGroupsBmp = (1 << portGroup);
    GT_STATUS               rc;

    for (ruleIndex = startIndex; ruleIndex < (startIndex + rulesAmount); ruleIndex++)
    {
        rc = cpssDxChTcamPortGroupRuleValidStatusSet(devNum,portGroupsBmp,ruleIndex, valid);
        if (rc != GT_OK)
        {
            cmdOsPrintf("cpssDxChTcamPortGroupRuleValidStatusSet error: index %d rc %d\n", ruleIndex, rc);
            return rc;
        }
    }

    return GT_OK;
}



/**
* @internal wrUtil_cpssDxChPclRuleMultiCopy function
* @endinternal
*
* @brief   The function copies the Rule's mask, pattern and action to new TCAM positions.
*         The source Rule is not invalidated by the function.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleSize                 - size of Rule.
* @param[in] ruleSrcIndex             - index of the rule in the TCAM from which pattern,
*                                      mask and action are taken.
* @param[in] ruleDstStartIndex        - first index of the rule in the TCAM to which pattern,
*                                      mask and action are placed
* @param[in] numOfRules               - number of time the source rule is copied to
*                                      sequential destination rules
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS wrUtil_cpssDxChPclRuleMultiCopy
(
    IN GT_U8                              devNum,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleSrcIndex,
    IN GT_U32                             ruleDstStartIndex,
    IN GT_U32                             numOfRules
)
{
    GT_U32      ii;
    GT_U32      dstIdx;
    GT_STATUS   result;
    GT_BOOL     enableIndexConversion;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT tcamRuleSize;
    GT_U32      indexFactor = 1;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        appDemoDxChNewPclTcamSupportGet(&enableIndexConversion);

        if (enableIndexConversion == GT_FALSE)
        {
            /* convert pcl rule's size to tcam rule's size */
            PRV_CPSS_DXCH_PCL_CONVERT_RULE_SIZE_TO_TCAM_RULE_SIZE_VAL_MAC(tcamRuleSize, ruleSize);

            switch(tcamRuleSize)
            {
                case CPSS_DXCH_TCAM_RULE_SIZE_10_B_E:
                    indexFactor = 1;
                    break;
                case CPSS_DXCH_TCAM_RULE_SIZE_20_B_E:
                    indexFactor = 2;
                    break;
                case CPSS_DXCH_TCAM_RULE_SIZE_40_B_E:
                case CPSS_DXCH_TCAM_RULE_SIZE_50_B_E:
                case CPSS_DXCH_TCAM_RULE_SIZE_60_B_E:
                    indexFactor = 6;
                    break;
                case CPSS_DXCH_TCAM_RULE_SIZE_80_B_E:
                    indexFactor = 12;
                    break;
                case CPSS_DXCH_TCAM_RULE_SIZE_30_B_E:
                default:
                    indexFactor = 3;
                    break;
            }
        }
    }

    for (ii = 0; ii < numOfRules; ii++)
    {
        dstIdx = ruleDstStartIndex + (ii * indexFactor);
        result = pg_wrap_cpssDxChPclRuleCopy(devNum, ruleSize, ruleSrcIndex, dstIdx);
        if (result != GT_OK)
        {
             cmdOsPrintf("Pcl Rule Multi Copy fail: dstIdx %d rc %d\n", dstIdx, result);
             return result;
        }
    }
    return GT_OK;
}


