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
* @file wrapCpssDxChExactMatch.c
*
* @brief Wrapper functions for Cpss Falcon Exact Match
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/exactMatch/cpssDxChExactMatch.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>

/* support for multi port groups */
static GT_BOOL             multiPortGroupsBmpEnable = GT_FALSE;
static GT_PORT_GROUPS_BMP  multiPortGroupsBmp;

/* current port group for the 'refresh' Exact Match */
static GT_BOOL useRefreshExactMatchPerPortGroupId = GT_FALSE;
/* current port group for the 'refresh' Exact Match */
static GT_U32  refreshExactMatchCurrentPortGroupId = 0;
/* first port group for the 'refresh' Exact Match */
static GT_U32  refreshExactMatchFirstPortGroupId = 0;

static  GT_U32     exactMatchEntryIndex;      /* first index to get from Exact Match Table */
static  GT_U32     exactMatchEntryNumOfEntries;/* for the refresh of Exact Match Table */
static  GT_U32     exactMatchEntryIndexMaxGet;/* for the refresh of Exact Match Table */


/*
   !!! for Exact Match purposes ONLY !!!
   Get the portGroupsBmp for multi port groups device.
   when 'enabled' --> wrappers will use the APIs with portGroupsBmp parameter*/
#define EXACT_MATCH_MULTI_PORT_GROUPS_BMP_GET(device,enable,portGroupBmp)  \
    exactMatchMultiPortGroupsBmpGet(device,&enable,&portGroupBmp);


/**
* @internal exactMatchMultiPortGroupsBmpGet function
* @endinternal
*
* @brief   !!! for Exact Match purposes ONLY !!!
*         Get the portGroupsBmp for multi port groups device.
*         when 'enabled' --> wrappers will use the APIs with portGroupsBmp parameter
*         with port group bmp parameter  |   NO port group bmp parameter
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)enable / disable the use of APIs with portGroupsBmp parameter.
* @param[out] portGroupsBmpPtr         - (pointer to)port groups bmp , relevant only when enable = GT_TRUE
*                                       NONE
*/
static void exactMatchMultiPortGroupsBmpGet
(
    IN  GT_U8                devNum,
    OUT  GT_BOOL             *enablePtr,
    OUT  GT_PORT_GROUPS_BMP  *portGroupsBmpPtr
)
{
    GT_STATUS   rc;

    *enablePtr  = GT_FALSE;

    if((0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum)) ||
       (0 == PRV_CPSS_DXCH_FAMILY_CHECK_MAC(devNum)))
    {
        return;
    }

    rc = utilMultiPortGroupsBmpGet(devNum,enablePtr,portGroupsBmpPtr);
    if(rc != GT_OK)
    {
        return;
    }

    return;
}

/**
* @internal wrCpssDxChExactMatchTtiProfileIdSet function
* @endinternal
*
* @brief  Sets the Exact Match Profile Id for TTI keyType
*
* NOTE: Client lookup for given ttiLookupNum MUST be configured
*       before this API is called (cpssDxChExactMatchClientLookupSet).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid
*         range.
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChExactMatchTtiProfileIdSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    CPSS_DXCH_TTI_KEY_TYPE_ENT      keyType;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT ttiLookupNum;
    GT_U32                          profileId;
    GT_BOOL                         enableExactMatchLookup;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    keyType = (CPSS_DXCH_TTI_KEY_TYPE_ENT)inArgs[1];
    ttiLookupNum = (CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)inArgs[2];
    enableExactMatchLookup=(GT_BOOL)inArgs[3];
    profileId = (GT_U32)inArgs[4];

    /* call cpss api function */
    result = cpssDxChExactMatchTtiProfileIdSet(devNum, keyType, ttiLookupNum, enableExactMatchLookup, profileId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChExactMatchTtiProfileIdGet function
* @endinternal
*
* @brief   Gets the  Exact Match Profile Id form TTI keyType
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid
*         range.
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChExactMatchTtiProfileIdGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    CPSS_DXCH_TTI_KEY_TYPE_ENT      keyType;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT ttiLookupNum;
    GT_U32                          profileId=0;
    GT_BOOL                         enableExactMatchLookup=GT_FALSE;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    keyType = (CPSS_DXCH_TTI_KEY_TYPE_ENT)inArgs[1];
    ttiLookupNum = (CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChExactMatchTtiProfileIdGet(devNum, keyType, ttiLookupNum, &enableExactMatchLookup, &profileId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", enableExactMatchLookup, profileId);

    return CMD_OK;
}

/**
* @internal wrCpssDxChExactMatchPclProfileIdSet function
* @endinternal
*
* @brief  Sets the Exact Match Profile Id for PCL packet type
*         and Sub Profile Id
*
* NOTE: Client lookup for given pclLookupNum MUST be configured
*       before this API is called (cpssDxChExactMatchClientLookupSet).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                               Aldrin; AC3X; Lion2; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChExactMatchPclProfileIdSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT       packetType;
    CPSS_PCL_DIRECTION_ENT              direction;
    GT_U32                              subProfileId;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    pclLookupNum;
    GT_BOOL                             enableExactMatchLookup;
    GT_U32                              profileId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum       = (GT_U8)inArgs[0];
    direction    = (CPSS_PCL_DIRECTION_ENT)inArgs[1];
    packetType   = (CPSS_DXCH_PCL_PACKET_TYPE_ENT)inArgs[2];
    subProfileId = (GT_U32)inArgs[3];
    pclLookupNum    = (CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)inArgs[4];
    enableExactMatchLookup = (GT_BOOL)inArgs[5];
    profileId    = (GT_U32)inArgs[6];

    /* call cpss api function */
    result = cpssDxChExactMatchPclProfileIdSet(devNum, direction ,packetType, subProfileId, pclLookupNum, enableExactMatchLookup, profileId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChExactMatchPclProfileIdGet function
* @endinternal
*
* @brief  Gets the Exact Match Profile Id form PCL packet type
*         and Sub Profile Id
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                               Aldrin; AC3X; Lion2; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChExactMatchPclProfileIdGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT       packetType;
    CPSS_PCL_DIRECTION_ENT              direction;
    GT_U32                              subProfileId;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    pclLookupNum;
    GT_BOOL                             enableExactMatchLookup;
    GT_U32                              profileId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum       = (GT_U8)inArgs[0];
    packetType   = (CPSS_DXCH_PCL_PACKET_TYPE_ENT)inArgs[1];
    direction    = (CPSS_PCL_DIRECTION_ENT)inArgs[2];
    subProfileId = (GT_U32)inArgs[3];
    pclLookupNum = (CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)inArgs[4];

    /* call cpss api function */
    result = cpssDxChExactMatchPclProfileIdGet(devNum, direction, packetType, subProfileId, pclLookupNum,&enableExactMatchLookup, &profileId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", enableExactMatchLookup, profileId);

    return CMD_OK;
}

/**
* @internal wrCpssDxChExactMatchClientLookupSet function
* @endinternal
*
* @brief   Set global configuration to determine the client of
*          the first EM lookup and the client of the second EM
*          lookup.
*
*   NOTE: this API should be called before configuring
*      Exact Match Profile Id for TTI keyType (cpssDxChExactMatchTtiProfileIdSet)or
*      Exact Match Profile Id for PCL/EPCL packet type (cpssDxChExactMatchPclProfileIdSet)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChExactMatchClientLookupSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            exactMatchLookupNum;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            clientType;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    exactMatchLookupNum = (CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)inArgs[1];
    clientType = (CPSS_DXCH_EXACT_MATCH_CLIENT_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChExactMatchClientLookupSet(devNum, exactMatchLookupNum, clientType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChExactMatchClientLookupGet function
* @endinternal
*
* @brief   Get global configuration that determine the client of
*          the first EM lookup and the client of the second EM
*          lookup.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

static CMD_STATUS wrCpssDxChExactMatchClientLookupGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            exactMatchLookupNum;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            clientType;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    exactMatchLookupNum = (CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChExactMatchClientLookupGet(devNum, exactMatchLookupNum, &clientType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d" , clientType);
    return CMD_OK;
}

/**
* @internal cpssDxChExactMatchActivityBitEnableSet function
* @endinternal
*
* @brief   Enables/disables the refresh of the Exact Match
*          activity bit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum           - device number
* @param[in] exactMatchActivityBit - GT_TRUE - enable refreshing
*                                  GT_FALSE - disable refreshing
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static CMD_STATUS wrCpssDxChExactMatchActivityBitEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    GT_BOOL         exactMatchActivityBitEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    exactMatchActivityBitEnable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChExactMatchActivityBitEnableSet(devNum, exactMatchActivityBitEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal cpssDxChExactMatchActivityBitEnableGet function
* @endinternal
*
* @brief   Enables/disables the refresh of the Exact Match
*          activity bit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum             - device number
* @param[in] enablePtr          - (pointer to)
* @param[in] exactMatchActivityBitPtr - (pointer to)
*                               GT_TRUE - enable refreshing
*                               GT_FALSE - disable refreshing
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static CMD_STATUS wrCpssDxChExactMatchActivityBitEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    GT_BOOL         exactMatchActivityBit;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChExactMatchActivityBitEnableGet(devNum, &exactMatchActivityBit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d" , exactMatchActivityBit);
    return CMD_OK;
}


/**
* @internal wrCpssDxChExactMatchActivityStatusGet function
* @endinternal
*
* @brief   Return the Exact Match activity bit for a given
*          entry.The bit is set by the device when the entry is
*          matched in the EM Lookup. The bit is reset by the CPU
*          as part of the aging process.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in]  devNum             - device number
* @param[in] portGroupsBmp       - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in]  exactMatchEntryIndex - EM entry index
* @param[in]  exactMatchClearActivity - set activity bit to 0
* @param[out] exactMatchActivityStatusPtr  - (pointer to)
*                  GT_FALSE = Not Refreshed; next_age_pass;
*                  Entry was matched since the last reset;
*                  GT_TRUE = Refreshed; two_age_pass; Entry was
*                  not matched since the last reset;
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static CMD_STATUS wrCpssDxChExactMatchActivityStatusGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_BOOL                 enable;
    GT_PORT_GROUPS_BMP      portGroupsBmp;
    GT_U32                  exactMatchEntryIndex;
    GT_BOOL                 exactMatchClearActivity;
    GT_BOOL                 exactMatchActivityStatus;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    exactMatchEntryIndex = (GT_U32)inArgs[1];
    exactMatchClearActivity = (GT_BOOL)inArgs[2];

    exactMatchMultiPortGroupsBmpGet(devNum,&enable,&portGroupsBmp);

    if(multiPortGroupsBmpEnable == GT_FALSE)
    {
        /* call cpss api function */
        result = cpssDxChExactMatchActivityStatusGet(devNum,
                                             CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                             exactMatchEntryIndex,
                                             exactMatchClearActivity,
                                             &exactMatchActivityStatus);
    }
    else
    {
        /* call cpss api function */
        result = cpssDxChExactMatchActivityStatusGet(devNum,
                                             portGroupsBmp,
                                             exactMatchEntryIndex,
                                             exactMatchClearActivity,
                                             &exactMatchActivityStatus);
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d" , exactMatchActivityStatus);
    return CMD_OK;
}

/*************************************************/
/* cpssDxChExactMatchProfileDefaultAction Table  */
/*************************************************/

/* table cpssDxChExactMatchProfileDefaultAction global variables */
#define DEFAULT_ACTION_TABLE_SIZE 15

/* Database to hold default actions that has been set.
   Non valid entries maintain link list of free table entries */
typedef struct
{
    GT_BOOL                                 valid;                  /* weather this entry is valid           */
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT   actionType;             /* action type                           */
    GT_U32                                  exactMatchProfileIndex; /* index associated with the action      */
    GT_U8                                   devNum;                 /* device associated with the action     */
    GT_U32                                  nextFree;               /* in case not valid, next free entry    */
}defaultActionDB;

static defaultActionDB defaultActionDatabase[DEFAULT_ACTION_TABLE_SIZE];

/* index of first free default action entry in the default action table */
static GT_U32   firstFree = 0;
/* weather default action was used before or not (used to invalidate all entries
   on first time the table is used) */
static GT_BOOL  firstRun = GT_TRUE;
/* indicated invalid rule entry */
#define INVALID_EXACT_MATCH_PROFILE_INDEX 16 /* valid profileID 1-15 */

/**
* @internal defaultActionDatabaseInit function
* @endinternal
*
*/
static CMD_STATUS defaultActionDatabaseInit(void)
{
    GT_U32      index = 0;

    firstFree = 0;

    while (index < DEFAULT_ACTION_TABLE_SIZE)
    {
        /* init all 15 profileId entries as valid */
        defaultActionDatabase[index].valid = GT_TRUE;
        defaultActionDatabase[index].exactMatchProfileIndex = index + 1;
        defaultActionDatabase[index].nextFree = index + 1;
        if (index==(DEFAULT_ACTION_TABLE_SIZE-1))
        {
            defaultActionDatabase[index].nextFree = INVALID_EXACT_MATCH_PROFILE_INDEX;
        }
        index++;
    }
    return CMD_OK;
}

/**
* @internal defaultActionDatabaseGet function
* @endinternal
*
*/
static void defaultActionDatabaseGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  exactMatchProfileIndex,
    OUT GT_U32                  *tableIndex
)
{
    GT_U32              index = 0;
    GT_BOOL             entryFound = GT_FALSE;

    while ((!entryFound) && (index < DEFAULT_ACTION_TABLE_SIZE))
    {
        /* get next valid entry */
        while ((defaultActionDatabase[index].valid == GT_FALSE) && (index < DEFAULT_ACTION_TABLE_SIZE))
            index++;
        /* check if the entry match the device number and rule index */
        if ((index < DEFAULT_ACTION_TABLE_SIZE) &&
            (defaultActionDatabase[index].exactMatchProfileIndex == exactMatchProfileIndex) &&
            (defaultActionDatabase[index].devNum == devNum))
        {
            entryFound = GT_TRUE;
        }
        else
        {
            index++;
        }
    }

    if (entryFound)
    {
        *tableIndex = index;
    }
    else
    {
        *tableIndex = INVALID_EXACT_MATCH_PROFILE_INDEX;
    }
}

/**
* @internal wrCpssDxChExactMatchProfileDefaultActionUpdate
*           function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChExactMatchProfileDefaultActionUpdate
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result = GT_OK;
    GT_U8                                   devNum;
    GT_U32                                  exactMatchProfileIndex;
    GT_U32                                  index;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT   actionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT        defaultActionData;
    GT_BOOL                                 defActionEn;
    GT_BOOL                                 isNewIndex = GT_FALSE;
    defaultActionDB                         defaultTempAction = {0};
    GT_U32                                  i=0;
    GT_U32                                  saveArpPtr;


    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    actionType  = (CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT)inArgs[1];

    exactMatchProfileIndex = (GT_U32)inFields[0];
    defActionEn = (GT_BOOL)inFields[1];
    i=2;

    /* in case it is first usage of default action database, initialize it */
    if (firstRun)
    {
        defaultActionDatabaseInit();
        firstRun = GT_FALSE;
    }

    /* look for the action in the default action database */
    defaultActionDatabaseGet(devNum, exactMatchProfileIndex, &index);

    /* the action wasn't in the default action database */
    if (index == INVALID_EXACT_MATCH_PROFILE_INDEX)
    {
        if (firstFree == INVALID_EXACT_MATCH_PROFILE_INDEX)
        {
            /* pack output arguments to galtis string */
            galtisOutput(outArgs, GT_NO_RESOURCE, "%d", -1);
            return CMD_AGENT_ERROR;
        }

        index = firstFree;
        firstFree = defaultActionDatabase[firstFree].nextFree;
        isNewIndex = GT_TRUE;
    }

    /* this is update of an existing action:
       save the old action parameters */
    if (isNewIndex == GT_FALSE)
    {
        cpssOsMemCpy(&defaultTempAction,&defaultActionDatabase[index],sizeof(defaultTempAction));
    }

    /* set action entry in default action database as valid */
    defaultActionDatabase[index].valid                      = GT_TRUE;
    defaultActionDatabase[index].exactMatchProfileIndex     = exactMatchProfileIndex;
    defaultActionDatabase[index].devNum                     = devNum;
    defaultActionDatabase[index].actionType                 = (CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT)inArgs[0];

    cpssOsMemSet(&defaultActionData, 0, sizeof(defaultActionData));

    /* fill action fields */
    switch (actionType)
    {
    case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:

        defaultActionData.ttiAction.tunnelTerminate                 = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.ttPassengerPacketType           = (CPSS_DXCH_TTI_PASSENGER_TYPE_ENT)inFields[i++];
        defaultActionData.ttiAction.tsPassengerPacketType           = (CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT)inFields[i++];

        defaultActionData.ttiAction.ttHeaderLength                  = (GT_U32)inFields[i++];
        defaultActionData.ttiAction.continueToNextTtiLookup         = (GT_BOOL)inFields[i++];

        defaultActionData.ttiAction.copyTtlExpFromTunnelHeader      = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.mplsCommand                     = (CPSS_DXCH_TTI_MPLS_CMD_ENT)inFields[i++];
        defaultActionData.ttiAction.mplsTtl                         = (GT_U32)inFields[i++];
        defaultActionData.ttiAction.enableDecrementTtl              = (GT_BOOL)inFields[i++];

        defaultActionData.ttiAction.passengerParsingOfTransitMplsTunnelMode = (CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_TYPE_ENT)inFields[i++];
        defaultActionData.ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable = (GT_BOOL)inFields[i++];

        defaultActionData.ttiAction.command                         = (CPSS_PACKET_CMD_ENT)inFields[i++];
        defaultActionData.ttiAction.redirectCommand                 = (CPSS_DXCH_TTI_REDIRECT_COMMAND_ENT)inFields[i++];
        defaultActionData.ttiAction.egressInterface.type            = (CPSS_INTERFACE_TYPE_ENT)inFields[i++];
        defaultActionData.ttiAction.egressInterface.devPort.hwDevNum  = (GT_HW_DEV_NUM)inFields[i++];
        defaultActionData.ttiAction.egressInterface.devPort.portNum = (GT_PORT_NUM)inFields[i++];
        /* Override Device and Port */
        CONVERT_DEV_PORT_DATA_MAC(defaultActionData.ttiAction.egressInterface.devPort.hwDevNum, defaultActionData.ttiAction.egressInterface.devPort.portNum);
        defaultActionData.ttiAction.egressInterface.trunkId         = (GT_TRUNK_ID)inFields[i++];
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(defaultActionData.ttiAction.egressInterface.trunkId);
        defaultActionData.ttiAction.egressInterface.vidx            = (GT_U16)inFields[i++];
        defaultActionData.ttiAction.egressInterface.vlanId          = (GT_U16)inFields[i++];
        defaultActionData.ttiAction.arpPtr                          = (GT_U32)inFields[i++];
        defaultActionData.ttiAction.tunnelStart                     = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.tunnelStartPtr                  = (GT_U32)inFields[i++];
        defaultActionData.ttiAction.routerLttPtr                    = (GT_U32)inFields[i++];
        defaultActionData.ttiAction.vrfId                           = (GT_U32)inFields[i++];
        defaultActionData.ttiAction.sourceIdSetEnable               = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.sourceId                        = (GT_U32)inFields[i++];
        defaultActionData.ttiAction.tag0VlanCmd                     = (CPSS_DXCH_TTI_VLAN_COMMAND_ENT)inFields[i++];
        defaultActionData.ttiAction.tag0VlanId                      = (GT_U16)inFields[i++];
        defaultActionData.ttiAction.tag1VlanCmd                     = (CPSS_DXCH_TTI_VLAN_COMMAND_ENT)inFields[i++];
        defaultActionData.ttiAction.tag1VlanId                      = (GT_U16)inFields[i++];
        defaultActionData.ttiAction.tag0VlanPrecedence              = (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[i++];
        defaultActionData.ttiAction.nestedVlanEnable                = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.bindToPolicerMeter              = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.bindToPolicer                   = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.policerIndex                    = (GT_U32)inFields[i++];
        defaultActionData.ttiAction.qosPrecedence                   = (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[i++];
        defaultActionData.ttiAction.keepPreviousQoS                 = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.trustUp                         = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.trustDscp                       = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.trustExp                        = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.qosProfile                      = (GT_U32)inFields[i++];
        defaultActionData.ttiAction.modifyTag0Up                    = (CPSS_DXCH_TTI_MODIFY_UP_ENT)inFields[i++];
        defaultActionData.ttiAction.tag1UpCommand                   = (CPSS_DXCH_TTI_TAG1_UP_COMMAND_ENT)inFields[i++];
        defaultActionData.ttiAction.modifyDscp                      = (CPSS_DXCH_TTI_MODIFY_DSCP_ENT)inFields[i++];
        defaultActionData.ttiAction.tag0Up                          = (GT_U32)inFields[i++];
        defaultActionData.ttiAction.tag1Up                          = (GT_U32)inFields[i++];
        defaultActionData.ttiAction.remapDSCP                       = (GT_BOOL)inFields[i++];

        defaultActionData.ttiAction.qosUseUpAsIndexEnable           = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.qosMappingTableIndex            = (GT_U32)inFields[i++];
        defaultActionData.ttiAction.mplsLLspQoSProfileEnable        = (GT_BOOL)inFields[i++];

        defaultActionData.ttiAction.pcl0OverrideConfigIndex         = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[i++];
        defaultActionData.ttiAction.pcl0_1OverrideConfigIndex       = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[i++];
        defaultActionData.ttiAction.pcl1OverrideConfigIndex         = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[i++];
        defaultActionData.ttiAction.iPclConfigIndex                 = (GT_U32)inFields[i++];

        defaultActionData.ttiAction.iPclUdbConfigTableEnable        = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.iPclUdbConfigTableIndex         = (CPSS_DXCH_PCL_PACKET_TYPE_ENT)inFields[i++];

        defaultActionData.ttiAction.mirrorToIngressAnalyzerEnable   = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.mirrorToIngressAnalyzerIndex    = (GT_U32)inFields[i++];

        defaultActionData.ttiAction.userDefinedCpuCode              = (CPSS_NET_RX_CPU_CODE_ENT)inFields[i++];
        defaultActionData.ttiAction.bindToCentralCounter            = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.centralCounterIndex             = (GT_U32)inFields[i++];
        defaultActionData.ttiAction.vntl2Echo                       = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.bridgeBypass                    = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.ingressPipeBypass               = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.actionStop                      = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.hashMaskIndex                   = (GT_U32)inFields[i++];
        defaultActionData.ttiAction.modifyMacSa                     = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.modifyMacDa                     = (GT_BOOL)inFields[i++];

        defaultActionData.ttiAction.sourceEPortAssignmentEnable     = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.sourceEPort                     = (GT_PORT_NUM)inFields[i++];
        defaultActionData.ttiAction.flowId                          = (GT_U32)inFields[i++];
        defaultActionData.ttiAction.setMacToMe                      = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.rxProtectionSwitchEnable        = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.rxIsProtectionPath              = (GT_BOOL)inFields[i++];

        defaultActionData.ttiAction.oamTimeStampEnable              = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.oamOffsetIndex                  = (GT_U32)inFields[i++];
        defaultActionData.ttiAction.oamProcessEnable                = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.oamProfile                      = (GT_U32)inFields[i++];
        defaultActionData.ttiAction.oamChannelTypeToOpcodeMappingEnable = (GT_BOOL)inFields[i++];

        defaultActionData.ttiAction.isPtpPacket                     = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.ptpTriggerType                  = (CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_ENT)inFields[i++];
        defaultActionData.ttiAction.ptpOffset                       = (GT_U32)inFields[i++];

        defaultActionData.ttiAction.cwBasedPw                       = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.ttlExpiryVccvEnable             = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.pwe3FlowLabelExist              = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.pwCwBasedETreeEnable            = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.applyNonDataCwCommand           = (GT_BOOL)inFields[i++];

        defaultActionData.ttiAction.tunnelHeaderStartL4Enable       = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.skipFdbSaLookupEnable           = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.ipv6SegmentRoutingEndNodeEnable = (GT_BOOL)inFields[i++];
        defaultActionData.ttiAction.exactMatchOverTtiEn             = (GT_BOOL)inFields[i++];
        break;

    case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
    case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
        /* field index */
        i = 2;

        defaultActionData.pclAction.pktCmd = (CPSS_PACKET_CMD_ENT)inFields[i++];
        defaultActionData.pclAction.actionStop    =(GT_BOOL)inFields[i++];
        defaultActionData.pclAction.egressPolicy  = (GT_BOOL)inFields[i++];
        defaultActionData.pclAction.mirror.cpuCode=(CPSS_NET_RX_CPU_CODE_ENT)inFields[i++];
        defaultActionData.pclAction.mirror.mirrorToRxAnalyzerPort               =(GT_BOOL)inFields[i++];
        defaultActionData.pclAction.mirror.ingressMirrorToAnalyzerIndex         =(GT_U32)inFields[i++];
        defaultActionData.pclAction.mirror.mirrorToTxAnalyzerPortEn             = (GT_U32)inFields[i++];
        defaultActionData.pclAction.mirror.egressMirrorToAnalyzerIndex          = (GT_U32)inFields[i++];
        defaultActionData.pclAction.mirror.egressMirrorToAnalyzerMode           = (CPSS_DXCH_MIRROR_EGRESS_MODE_ENT)inFields[i++];
        defaultActionData.pclAction.matchCounter.enableMatchCount=(GT_BOOL)inFields[i++];
        defaultActionData.pclAction.matchCounter.matchCounterIndex=(GT_U32)inFields[i++];

        switch (defaultActionData.pclAction.egressPolicy)
        {
            case GT_FALSE: /* ingress */
                defaultActionDatabase[index].actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;

                defaultActionData.pclAction.qos.ingress.modifyDscp =
                    (CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[i];

                defaultActionData.pclAction.qos.ingress.modifyUp =
                    (CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[i+1];

                defaultActionData.pclAction.qos.ingress.profileIndex = (GT_U32)inFields[i+2];
                defaultActionData.pclAction.qos.ingress.profileAssignIndex = (GT_BOOL)inFields[i+3];
                defaultActionData.pclAction.qos.ingress.profilePrecedence =
                    (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[i+4];
                defaultActionData.pclAction.qos.ingress.up1Cmd =
                    (CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ENT)inFields[i+5];
                defaultActionData.pclAction.qos.ingress.up1 = (GT_U32)inFields[i+6];
                break;

            default:
            case GT_TRUE: /* egress */
                defaultActionDatabase[index].actionType = CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E;

                defaultActionData.pclAction.qos.egress.modifyDscp =
                    (CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_ENT)(inFields[i+7]);
                defaultActionData.pclAction.qos.egress.dscp = (GT_U32)inFields[i+8];
                defaultActionData.pclAction.qos.egress.modifyUp =
                    (CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT)(inFields[i+9]);
                defaultActionData.pclAction.qos.egress.up   = (GT_U32)inFields[i+10];
                defaultActionData.pclAction.qos.egress.up1ModifyEnable   = (GT_BOOL)inFields[i+11];
                defaultActionData.pclAction.qos.egress.up1   = (GT_U32)inFields[i+12];
                break;
        }

        /* bypass the switch */
        i += 13;

        defaultActionData.pclAction.redirect.redirectCmd =
            (CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ENT)inFields[i++];

        if (defaultActionData.pclAction.redirect.redirectCmd ==
            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E)
        {
            /* used the same GUI fields as for redirect to outIf */
            defaultActionData.pclAction.redirect.data.noRedirect.arpPtr = (GT_U32)inFields[i+9];
            defaultActionData.pclAction.redirect.data.noRedirect.modifyMacDa = (GT_BOOL)inFields[i+10];
        }

        if (defaultActionData.pclAction.redirect.redirectCmd ==
            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E)
        {
            defaultActionData.pclAction.redirect.data.outIf.outInterface.type =
                (CPSS_INTERFACE_TYPE_ENT)inFields[i];

            switch (defaultActionData.pclAction.redirect.data.outIf.outInterface.type)
            {
                case CPSS_INTERFACE_PORT_E:
                    defaultActionData.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum =
                        (GT_HW_DEV_NUM)inFields[i+1];
                    defaultActionData.pclAction.redirect.data.outIf.outInterface.devPort.portNum =
                        (GT_PORT_NUM)inFields[i+2];
                    CONVERT_DEV_PORT_DATA_MAC(
                        defaultActionData.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum,
                        defaultActionData.pclAction.redirect.data.outIf.outInterface.devPort.portNum);
                    break;

                case CPSS_INTERFACE_TRUNK_E:
                    defaultActionData.pclAction.redirect.data.outIf.outInterface.trunkId =
                        (GT_TRUNK_ID)inFields[i+3];
                    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(
                       defaultActionData.pclAction.redirect.data.outIf.outInterface.trunkId);
                    break;

                case CPSS_INTERFACE_VIDX_E:
                    defaultActionData.pclAction.redirect.data.outIf.outInterface.vidx =
                        (GT_U16)inFields[i+4];
                    break;

                default:
                    break;
            }
            defaultActionData.pclAction.redirect.data.outIf.vntL2Echo   = (GT_BOOL)inFields[i+5];
            defaultActionData.pclAction.redirect.data.outIf.tunnelStart = (GT_BOOL)inFields[i+6];
            defaultActionData.pclAction.redirect.data.outIf.tunnelPtr   = (GT_U32)inFields[i+7];
            defaultActionData.pclAction.redirect.data.outIf.tunnelType  =
                (CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ENT)inFields[i+8];
            defaultActionData.pclAction.redirect.data.outIf.arpPtr = (GT_U32)inFields[i+9];
            defaultActionData.pclAction.redirect.data.outIf.modifyMacDa = (GT_BOOL)inFields[i+10];
            defaultActionData.pclAction.redirect.data.outIf.modifyMacSa = (GT_BOOL)inFields[i+11];
        }
        /* bypass redirect to IF */
        /* arpPtr relevant also to ...REDIRECT_CMD_REPLACE_MAC_SA_E */
        saveArpPtr = (GT_U32)inFields[i+9];
        i += 12;

        if (defaultActionData.pclAction.redirect.redirectCmd ==
            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E)
        {
            defaultActionData.pclAction.redirect.data.routerLttIndex = (GT_U32)inFields[i];
        }
        /* bypass redirect to Next Hop */
        i ++;


        if (defaultActionData.pclAction.redirect.redirectCmd ==
            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E)
        {
            defaultActionData.pclAction.redirect.data.vrfId = (GT_U32)inFields[i];
        }
        /* bypass redirect to Virtual Router */
        i ++;

        if (defaultActionData.pclAction.redirect.redirectCmd ==
            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E)
        {
            galtisMacAddr(&(defaultActionData.pclAction.redirect.data.modifyMacSa.macSa), (GT_U8*)inFields[i]);
            /* arpPtr saved from ...REDIRECT_CMD_OUT_IF_E parameters */
            defaultActionData.pclAction.redirect.data.modifyMacSa.arpPtr = saveArpPtr;
        }
        /* bypass replace MAC_SA */
        i ++;

        defaultActionData.pclAction.policer.policerEnable = (CPSS_DXCH_PCL_POLICER_ENABLE_ENT)inFields[i++];
        defaultActionData.pclAction.policer.policerId = (GT_U32)inFields[i++];

        switch (defaultActionData.pclAction.egressPolicy)
        {
            case GT_FALSE:
                defaultActionData.pclAction.vlan.ingress.modifyVlan =
                    (CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT)inFields[i];
                defaultActionData.pclAction.vlan.ingress.nestedVlan = (GT_BOOL)inFields[i+2];
                defaultActionData.pclAction.vlan.ingress.vlanId = (GT_U32)inFields[i+3];
                defaultActionData.pclAction.vlan.ingress.precedence =
                    (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[i+4];
                defaultActionData.pclAction.vlan.ingress.vlanId1Cmd =
                    (CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ENT)inFields[i+5];
                defaultActionData.pclAction.vlan.ingress.vlanId1 = (GT_U32)inFields[i+7];
                break;

            case GT_TRUE:
                defaultActionData.pclAction.vlan.egress.vlanCmd =
                    (CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT)inFields[i+1];
                defaultActionData.pclAction.vlan.egress.vlanId = (GT_U32)inFields[i+3];
                defaultActionData.pclAction.vlan.egress.vlanId1ModifyEnable = (GT_BOOL)inFields[i+6];
                defaultActionData.pclAction.vlan.egress.vlanId1 = (GT_U32)inFields[i+7];
                break;

            default:
                break;
        }
        /* bypass VLAN */
        i += 8;

        defaultActionData.pclAction.sourceId.assignSourceId = (GT_BOOL)inFields[i++];
        defaultActionData.pclAction.sourceId.sourceIdValue  = (GT_U32)inFields[i++];

        defaultActionData.pclAction.bypassBridge                           = (GT_BOOL)inFields[i++];
        defaultActionData.pclAction.bypassIngressPipe                      = (GT_BOOL)inFields[i++];

        defaultActionData.pclAction.lookupConfig.ipclConfigIndex           = (GT_U32)inFields[i++];
        defaultActionData.pclAction.lookupConfig.pcl0_1OverrideConfigIndex =
            (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[i++];
        defaultActionData.pclAction.lookupConfig.pcl1OverrideConfigIndex   =
            (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[i++];
        defaultActionData.pclAction.mirror.mirrorTcpRstAndFinPacketsToCpu  = (GT_BOOL)inFields[i++];

        defaultActionData.pclAction.oam.timeStampEnable  = (GT_BOOL)inFields[i++];
        defaultActionData.pclAction.oam.offsetIndex      = (GT_U32)inFields[i++];
        defaultActionData.pclAction.oam.oamProcessEnable = (GT_BOOL)inFields[i++];
        defaultActionData.pclAction.oam.oamProfile       = (GT_U32)inFields[i++];

        defaultActionData.pclAction.flowId       = (GT_U32)inFields[i++];
        defaultActionData.pclAction.setMacToMe   = (GT_BOOL)inFields[i++];

        defaultActionData.pclAction.sourcePort.assignSourcePortEnable   = (GT_BOOL)inFields[i++];
        defaultActionData.pclAction.sourcePort.sourcePortValue          = (GT_U32)inFields[i++];

        defaultActionData.pclAction.channelTypeToOpcodeMapEnable   = (GT_BOOL)inFields[i++];
        defaultActionData.pclAction.exactMatchOverPclEn            = (GT_BOOL)inFields[i++];

        /* falcon fields */
        defaultActionData.pclAction.terminateCutThroughMode          = (GT_BOOL)inFields[i++];
        defaultActionData.pclAction.latencyMonitor.monitoringEnable  = (GT_BOOL)inFields[i++];
        defaultActionData.pclAction.latencyMonitor.latencyProfile    = (GT_U32)inFields[i++];
        defaultActionData.pclAction.skipFdbSaLookup                  = (GT_BOOL)inFields[i++];

        defaultActionData.pclAction.triggerInterrupt                 = (GT_BOOL)inFields[i++];

        /* TBD pha fields should be updated after implementation will be added
        defaultActionData.epclPhaInfo.phaThreadType   = (GT_BOOL)inFields[i++];
        defaultActionData.epclPhaInfo.phaThreadUnion   = (GT_BOOL)inFields[i++];
        defaultActionData.epclPhaInfo.phaThreadIdAssignmentMode   = (GT_BOOL)inFields[i++];
        defaultActionData.epclPhaInfo.phaThreadId   = (GT_BOOL)inFields[i++];*/
        break;

    default:
        galtisOutput(outArgs, GT_BAD_PARAM, "");
        return CMD_AGENT_ERROR;
    }

    /* call cpss api function */
    result = cpssDxChExactMatchProfileDefaultActionSet(devNum,exactMatchProfileIndex,actionType,&defaultActionData,defActionEn);


    /* if the rule action didn't succeed:  */
    if (result != GT_OK)
    {
        /* if this is a new action remove it from the database */
        if (isNewIndex)
        {
            defaultActionDatabase[index].valid = GT_FALSE;
            defaultActionDatabase[index].nextFree = firstFree;
            firstFree = index;
        }
        /* if this is an update for existing action restore previous data */
        else
        {
            cpssOsMemCpy(&defaultActionDatabase[index],&defaultTempAction,sizeof(defaultTempAction));
        }
    }


    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/*  table defaultActionDbIndexGet global variable   */

static GT_U32    defaultActionDbIndexGet;

/******************************************************************************/
static CMD_STATUS wrCpssDxChExactMatchProfileDefaultActionGetNext

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result = GT_OK;
    GT_U8                                   devNum;
    GT_U32                                  exactMatchProfileIndex;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT   actionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT        defaultActionData;
    GT_BOOL                                 defActionEn;
    GT_U32                                  i=0;
    GT_HW_DEV_NUM                           tempHwDev;  /* used for port,dev converting */
    GT_PORT_NUM                             tempPort;   /* used for port,dev converting */
    char                                    workStr[32];
    GT_U8                                   *workPtr;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* get next valid TTI action entry */
    while ((defaultActionDbIndexGet < DEFAULT_ACTION_TABLE_SIZE) &&
           (defaultActionDatabase[defaultActionDbIndexGet].valid == GT_FALSE))
        defaultActionDbIndexGet++;

    if (defaultActionDbIndexGet == DEFAULT_ACTION_TABLE_SIZE)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }


    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    actionType  = defaultActionDatabase[defaultActionDbIndexGet].actionType;
    exactMatchProfileIndex= defaultActionDatabase[defaultActionDbIndexGet].exactMatchProfileIndex;

    cmdOsMemSet(&defaultActionData, 0, sizeof(defaultActionData));

    result = cpssDxChExactMatchProfileDefaultActionGet(devNum, exactMatchProfileIndex, actionType, &defaultActionData, &defActionEn);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    switch (actionType)
    {
    case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
        inArgs[1] = actionType;
        i=0;
        inFields[i++]  = exactMatchProfileIndex;
        inFields[i++]  = defActionEn;
        inFields[i++]  = defaultActionData.ttiAction.tunnelTerminate;
        inFields[i++]  = defaultActionData.ttiAction.ttPassengerPacketType;
        inFields[i++]  = defaultActionData.ttiAction.tsPassengerPacketType;
        inFields[i++]  = defaultActionData.ttiAction.ttHeaderLength;
        inFields[i++]  = defaultActionData.ttiAction.continueToNextTtiLookup;
        inFields[i++]  = defaultActionData.ttiAction.copyTtlExpFromTunnelHeader;
        inFields[i++]  = defaultActionData.ttiAction.mplsCommand;
        inFields[i++]  = defaultActionData.ttiAction.mplsTtl;
        inFields[i++]  = defaultActionData.ttiAction.enableDecrementTtl;
        inFields[i++]  = defaultActionData.ttiAction.passengerParsingOfTransitMplsTunnelMode;
        inFields[i++]  = defaultActionData.ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable;
        inFields[i++]  = defaultActionData.ttiAction.command;
        inFields[i++] = defaultActionData.ttiAction.redirectCommand;
        inFields[i++] = defaultActionData.ttiAction.egressInterface.type;
        tempHwDev      = defaultActionData.ttiAction.egressInterface.devPort.hwDevNum;
        tempPort     = defaultActionData.ttiAction.egressInterface.devPort.portNum;
        CONVERT_BACK_DEV_PORT_DATA_MAC(tempHwDev,tempPort);
        inFields[i++] = tempHwDev;
        inFields[i++] = tempPort;
        CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(defaultActionData.ttiAction.egressInterface.trunkId);

        inFields[i++] = defaultActionData.ttiAction.egressInterface.trunkId;
        inFields[i++] = defaultActionData.ttiAction.egressInterface.vidx;
        inFields[i++] = defaultActionData.ttiAction.egressInterface.vlanId;
        inFields[i++] = defaultActionData.ttiAction.arpPtr;
        inFields[i++] = defaultActionData.ttiAction.tunnelStart;
        inFields[i++] = defaultActionData.ttiAction.tunnelStartPtr;
        inFields[i++] = defaultActionData.ttiAction.routerLttPtr;
        inFields[i++] = defaultActionData.ttiAction.vrfId;
        inFields[i++] = defaultActionData.ttiAction.sourceIdSetEnable;
        inFields[i++] = defaultActionData.ttiAction.sourceId;
        inFields[i++] = defaultActionData.ttiAction.tag0VlanCmd;
        inFields[i++] = defaultActionData.ttiAction.tag0VlanId;
        inFields[i++] = defaultActionData.ttiAction.tag1VlanCmd;
        inFields[i++] = defaultActionData.ttiAction.tag1VlanId;
        inFields[i++] = defaultActionData.ttiAction.tag0VlanPrecedence;
        inFields[i++] = defaultActionData.ttiAction.nestedVlanEnable;
        inFields[i++] = defaultActionData.ttiAction.bindToPolicerMeter;
        inFields[i++] = defaultActionData.ttiAction.bindToPolicer;
        inFields[i++] = defaultActionData.ttiAction.policerIndex;
        inFields[i++] = defaultActionData.ttiAction.qosPrecedence;
        inFields[i++] = defaultActionData.ttiAction.keepPreviousQoS;
        inFields[i++] = defaultActionData.ttiAction.trustUp;
        inFields[i++] = defaultActionData.ttiAction.trustDscp;
        inFields[i++] = defaultActionData.ttiAction.trustExp;
        inFields[i++] = defaultActionData.ttiAction.qosProfile;
        inFields[i++] = defaultActionData.ttiAction.modifyTag0Up;
        inFields[i++] = defaultActionData.ttiAction.tag1UpCommand;
        inFields[i++] = defaultActionData.ttiAction.modifyDscp;
        inFields[i++] = defaultActionData.ttiAction.tag0Up;
        inFields[i++] = defaultActionData.ttiAction.tag1Up;
        inFields[i++] = defaultActionData.ttiAction.remapDSCP;
        inFields[i++] = defaultActionData.ttiAction.qosUseUpAsIndexEnable;
        inFields[i++] = defaultActionData.ttiAction.qosMappingTableIndex;
        inFields[i++] = defaultActionData.ttiAction.mplsLLspQoSProfileEnable;
        inFields[i++] = defaultActionData.ttiAction.pcl0OverrideConfigIndex;
        inFields[i++] = defaultActionData.ttiAction.pcl0_1OverrideConfigIndex;
        inFields[i++] = defaultActionData.ttiAction.pcl1OverrideConfigIndex;
        inFields[i++] = defaultActionData.ttiAction.iPclConfigIndex;
        inFields[i++] = defaultActionData.ttiAction.iPclUdbConfigTableEnable;
        inFields[i++] = defaultActionData.ttiAction.iPclUdbConfigTableIndex;
        inFields[i++] = defaultActionData.ttiAction.mirrorToIngressAnalyzerEnable;
        inFields[i++] = defaultActionData.ttiAction.mirrorToIngressAnalyzerIndex;
        inFields[i++] = defaultActionData.ttiAction.userDefinedCpuCode;
        inFields[i++] = defaultActionData.ttiAction.bindToCentralCounter;
        inFields[i++] = defaultActionData.ttiAction.centralCounterIndex;
        inFields[i++] = defaultActionData.ttiAction.vntl2Echo;
        inFields[i++] = defaultActionData.ttiAction.bridgeBypass;
        inFields[i++] = defaultActionData.ttiAction.ingressPipeBypass;
        inFields[i++] = defaultActionData.ttiAction.actionStop;
        inFields[i++] = defaultActionData.ttiAction.hashMaskIndex;
        inFields[i++] = defaultActionData.ttiAction.modifyMacSa;
        inFields[i++] = defaultActionData.ttiAction.modifyMacDa;
        inFields[i++] = defaultActionData.ttiAction.sourceEPortAssignmentEnable;
        inFields[i++] = defaultActionData.ttiAction.sourceEPort;
        inFields[i++] = defaultActionData.ttiAction.flowId;
        inFields[i++] = defaultActionData.ttiAction.setMacToMe;
        inFields[i++] = defaultActionData.ttiAction.rxProtectionSwitchEnable;
        inFields[i++] = defaultActionData.ttiAction.rxIsProtectionPath;
        inFields[i++] = defaultActionData.ttiAction.oamTimeStampEnable;
        inFields[i++] = defaultActionData.ttiAction.oamOffsetIndex;
        inFields[i++] = defaultActionData.ttiAction.oamProcessEnable;
        inFields[i++] = defaultActionData.ttiAction.oamProfile;
        inFields[i++] = defaultActionData.ttiAction.oamChannelTypeToOpcodeMappingEnable;
        inFields[i++] = defaultActionData.ttiAction.isPtpPacket;
        inFields[i++] = defaultActionData.ttiAction.ptpTriggerType;
        inFields[i++] = defaultActionData.ttiAction.ptpOffset;
        inFields[i++] = defaultActionData.ttiAction.cwBasedPw;
        inFields[i++] = defaultActionData.ttiAction.ttlExpiryVccvEnable;
        inFields[i++] = defaultActionData.ttiAction.pwe3FlowLabelExist;
        inFields[i++] = defaultActionData.ttiAction.pwCwBasedETreeEnable;
        inFields[i++] = defaultActionData.ttiAction.applyNonDataCwCommand;

        inFields[i++] = defaultActionData.ttiAction.tunnelHeaderStartL4Enable;
        inFields[i++] = defaultActionData.ttiAction.skipFdbSaLookupEnable;
        inFields[i++] = defaultActionData.ttiAction.ipv6SegmentRoutingEndNodeEnable;
        inFields[i++] = defaultActionData.ttiAction.exactMatchOverTtiEn;

        fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                    "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                    "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                    "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                    "%d%d%d%d%d%d%d%d%d%d%d%d%d",
                    inFields[0],  inFields[1],   inFields[2],   inFields[3],
                    inFields[4],  inFields[5],   inFields[6],   inFields[7],
                    inFields[8],  inFields[9],   inFields[10],  inFields[11],
                    inFields[12], inFields[13],  inFields[14],  inFields[15],
                    inFields[16], inFields[17],  inFields[18],  inFields[19],
                    inFields[20], inFields[21],  inFields[22],  inFields[23],
                    inFields[24], inFields[25],  inFields[26],  inFields[27],
                    inFields[28], inFields[29],  inFields[30],  inFields[31],
                    inFields[32], inFields[33],  inFields[34],  inFields[35],
                    inFields[36], inFields[37],  inFields[38],  inFields[39],
                    inFields[40], inFields[41],  inFields[42],  inFields[43],
                    inFields[44], inFields[45],  inFields[46],  inFields[47],
                    inFields[48], inFields[49],  inFields[50],  inFields[51],
                    inFields[52], inFields[53],  inFields[54],  inFields[55],
                    inFields[56], inFields[57],  inFields[58],  inFields[59],
                    inFields[60], inFields[61],  inFields[62],  inFields[63],
                    inFields[64], inFields[65],  inFields[66],  inFields[67],
                    inFields[68], inFields[69],  inFields[70],  inFields[71],
                    inFields[72], inFields[73],  inFields[74],  inFields[75],
                    inFields[76], inFields[77],  inFields[78],  inFields[79],
                    inFields[80], inFields[81],  inFields[82],  inFields[83],
                    inFields[84], inFields[85],  inFields[86],  inFields[87],
                    inFields[88], inFields[89],  inFields[90],  inFields[91],
                    inFields[92]);

        /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_OK, "%d%f", 0);
        break;

    case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
    case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
        fieldOutput(
            "%d%d%d%d%d%d%d%d%d%d%d%d%d",
            exactMatchProfileIndex,
            defActionEn,
            defaultActionData.pclAction.pktCmd,
            defaultActionData.pclAction.actionStop,
            defaultActionData.pclAction.egressPolicy,
            defaultActionData.pclAction.mirror.cpuCode,
            defaultActionData.pclAction.mirror.mirrorToRxAnalyzerPort,
            defaultActionData.pclAction.mirror.ingressMirrorToAnalyzerIndex,
            defaultActionData.pclAction.mirror.mirrorToTxAnalyzerPortEn,
            defaultActionData.pclAction.mirror.egressMirrorToAnalyzerIndex,
            defaultActionData.pclAction.mirror.egressMirrorToAnalyzerMode,
            defaultActionData.pclAction.matchCounter.enableMatchCount,
            defaultActionData.pclAction.matchCounter.matchCounterIndex);

        fieldOutputSetAppendMode();

        if (defaultActionData.pclAction.egressPolicy == GT_FALSE)
        {
            fieldOutput(
                "%d%d%d%d%d%d%d%d%d%d%d%d%d",
                defaultActionData.pclAction.qos.ingress.modifyDscp,
                defaultActionData.pclAction.qos.ingress.modifyUp,
                defaultActionData.pclAction.qos.ingress.profileIndex,
                defaultActionData.pclAction.qos.ingress.profileAssignIndex,
                defaultActionData.pclAction.qos.ingress.profilePrecedence,
                defaultActionData.pclAction.qos.ingress.up1Cmd,
                defaultActionData.pclAction.qos.ingress.up1,
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
                defaultActionData.pclAction.qos.egress.modifyDscp,
                defaultActionData.pclAction.qos.egress.dscp,
                defaultActionData.pclAction.qos.egress.modifyUp,
                defaultActionData.pclAction.qos.egress.up,
                defaultActionData.pclAction.qos.egress.up1ModifyEnable,
                defaultActionData.pclAction.qos.egress.up1);
        }

        fieldOutputSetAppendMode();

        fieldOutput("%d", defaultActionData.pclAction.redirect.redirectCmd);
        fieldOutputSetAppendMode();

        if (defaultActionData.pclAction.redirect.redirectCmd ==
            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E)
        {
            fieldOutput(
                "%d%d%d%d%d%d%d%d%d%d%d%d",
                0, 0, 0, 0, 0, 0, 0, 0, 0,
                defaultActionData.pclAction.redirect.data.noRedirect.arpPtr,
                defaultActionData.pclAction.redirect.data.noRedirect.modifyMacDa,
                0);
        }
        else if (defaultActionData.pclAction.redirect.redirectCmd ==
            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E)
        {
            tempHwDev  = defaultActionData.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum;
            tempPort = defaultActionData.pclAction.redirect.data.outIf.outInterface.devPort.portNum;
            CONVERT_BACK_DEV_PORT_DATA_MAC(tempHwDev, tempPort) ;

            fieldOutput(
                "%d%d%d%d%d%d%d%d%d%d%d%d",
                defaultActionData.pclAction.redirect.data.outIf.outInterface.type,
                tempHwDev, tempPort,
                defaultActionData.pclAction.redirect.data.outIf.outInterface.trunkId,
                defaultActionData.pclAction.redirect.data.outIf.outInterface.vidx,
                defaultActionData.pclAction.redirect.data.outIf.vntL2Echo,
                defaultActionData.pclAction.redirect.data.outIf.tunnelStart,
                defaultActionData.pclAction.redirect.data.outIf.tunnelPtr,
                defaultActionData.pclAction.redirect.data.outIf.tunnelType,
                defaultActionData.pclAction.redirect.data.outIf.arpPtr,
                defaultActionData.pclAction.redirect.data.outIf.modifyMacDa,
                defaultActionData.pclAction.redirect.data.outIf.modifyMacSa);
        }
        else if (defaultActionData.pclAction.redirect.redirectCmd ==
            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E)
        {
            fieldOutput(
                "%d%d%d%d%d%d%d%d%d%d%d%d",
                0,0,0,0,0,0,0,0,0,defaultActionData.pclAction.redirect.data.modifyMacSa.arpPtr,0,0);
        }
        else
        {
            fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d", 0,0,0,0,0,0,0,0,0,0,0,0);
        }
        fieldOutputSetAppendMode();

        if (defaultActionData.pclAction.redirect.redirectCmd ==
            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E)
        {
            fieldOutput("%d", defaultActionData.pclAction.redirect.data.routerLttIndex);
        }
        else
        {
            fieldOutput("%d", 0);
        }
        fieldOutputSetAppendMode();

        if (defaultActionData.pclAction.redirect.redirectCmd ==
            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E)
        {
            fieldOutput("%d", defaultActionData.pclAction.redirect.data.vrfId);
        }
        else
        {
            fieldOutput("%d", 0);
        }
        fieldOutputSetAppendMode();

        if (defaultActionData.pclAction.redirect.redirectCmd ==
            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E)
        {
            workPtr = &(defaultActionData.pclAction.redirect.data.modifyMacSa.macSa.arEther[0]);
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
            defaultActionData.pclAction.policer.policerEnable,
            defaultActionData.pclAction.policer.policerId);
        fieldOutputSetAppendMode();

        if (defaultActionData.pclAction.egressPolicy == GT_FALSE)
        {
            fieldOutput(
                "%d%d%d%d%d%d%d%d",
                defaultActionData.pclAction.vlan.ingress.modifyVlan,
                0 /*action.vlan.egress.vlanCmd*/,
                defaultActionData.pclAction.vlan.ingress.nestedVlan,
                defaultActionData.pclAction.vlan.ingress.vlanId,
                defaultActionData.pclAction.vlan.ingress.precedence,
                defaultActionData.pclAction.vlan.ingress.vlanId1Cmd,
                0 /*action.vlan.egress.vlanId1ModifyEnable*/,
                defaultActionData.pclAction.vlan.ingress.vlanId1);
        }
        else
        {
            fieldOutput(
                "%d%d%d%d%d%d%d%d",
                0 /*action.vlan.ingress.modifyVlan*/,
                defaultActionData.pclAction.vlan.egress.vlanCmd,
                0 /*action.vlan.ingress.nestedVlan*/,
                defaultActionData.pclAction.vlan.egress.vlanId,
                0 /*action.vlan.ingress.precedence*/,
                0 /*action.vlan.ingress.vlanId1Cmd*/,
                defaultActionData.pclAction.vlan.egress.vlanId1ModifyEnable,
                defaultActionData.pclAction.vlan.egress.vlanId1);
        }
        fieldOutputSetAppendMode();

        fieldOutput(
            "%d%d",
            defaultActionData.pclAction.sourceId.assignSourceId,
            defaultActionData.pclAction.sourceId.sourceIdValue);
        fieldOutputSetAppendMode();

        fieldOutput(
            "%d%d%d%d%d%d",
            defaultActionData.pclAction.bypassBridge,
            defaultActionData.pclAction.bypassIngressPipe,
            defaultActionData.pclAction.lookupConfig.ipclConfigIndex,
            defaultActionData.pclAction.lookupConfig.pcl0_1OverrideConfigIndex,
            defaultActionData.pclAction.lookupConfig.pcl1OverrideConfigIndex,
            defaultActionData.pclAction.mirror.mirrorTcpRstAndFinPacketsToCpu);
        fieldOutputSetAppendMode();

        fieldOutput(
            "%d%d%d%d",
            defaultActionData.pclAction.oam.timeStampEnable,
            defaultActionData.pclAction.oam.offsetIndex,
            defaultActionData.pclAction.oam.oamProcessEnable,
            defaultActionData.pclAction.oam.oamProfile);
        fieldOutputSetAppendMode();

        fieldOutput(
            "%d%d%d%d%d%d",
            defaultActionData.pclAction.flowId,
            defaultActionData.pclAction.setMacToMe,
            defaultActionData.pclAction.sourcePort.assignSourcePortEnable,
            defaultActionData.pclAction.sourcePort.sourcePortValue,
            defaultActionData.pclAction.channelTypeToOpcodeMapEnable,
            defaultActionData.pclAction.exactMatchOverPclEn);
        fieldOutputSetAppendMode();

        /* falcon fields */
        fieldOutput(
            "%d%d%d%d%d",
            defaultActionData.pclAction.terminateCutThroughMode,
            defaultActionData.pclAction.latencyMonitor.monitoringEnable,
            defaultActionData.pclAction.latencyMonitor.latencyProfile,
            defaultActionData.pclAction.skipFdbSaLookup,
            defaultActionData.pclAction.triggerInterrupt);

        /* TBD pha fields should be updated after implementation will be added
          "%d%d%d%d%d%d%d%d%d",

         defaultActionData.epclPhaInfo.phaThreadType,
         defaultActionData.epclPhaInfo.phaThreadUnion,
         defaultActionData.epclPhaInfo.phaThreadIdAssignmentMode,
         defaultActionData.epclPhaInfo.phaThreadId);*/

         /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_OK, "%d%f", 1);
        break;
        break;
    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong Exact Match DEFAULT ACTION FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

    defaultActionDbIndexGet++;

    return CMD_OK;
}

/**
* @internal wrCpssDxChExactMatchProfileDefaultActionGetFirst
*           function
* @endinternal
*/
static CMD_STATUS wrCpssDxChExactMatchProfileDefaultActionGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* in case it is first usage of default action database, initialize it */
    if (firstRun)
    {
        defaultActionDatabaseInit();
        firstRun = GT_FALSE;
    }
    defaultActionDbIndexGet = 0;

    return wrCpssDxChExactMatchProfileDefaultActionGetNext(inArgs,inFields,numFields,outArgs);
}

/*************************************************/
/* cpssDxChExactMatchProfileKeyParamsSet Table  */
/*************************************************/
/* table cpssDxChExactMatchProfileKeyParamsSet global variables */
#define PROFILE_CONTROL_TABLE_SIZE 15

/* Database to hold profile control entries that has been set */
typedef struct
{
    GT_BOOL                                 valid;                  /* weather this entry is valid           */
    GT_U32                                  exactMatchProfileIndex; /* index associated with the action      */
    GT_U8                                   devNum;                 /* device associated with the action     */
    GT_U32                                  nextFree;               /* in case not valid, next free entry    */
}profileControlDB;

static profileControlDB profileControlDatabase[PROFILE_CONTROL_TABLE_SIZE];

/* index of first free profile control entry in the profile control table */
static GT_U32   firstFree_1 = 0;
/* wether  profile control was used before or not (used to invalidate all entries
   on first time the table is used) */
static GT_BOOL  firstRun_1 = GT_TRUE;
/* indicated invalid rule entry */
#define INVALID_EXACT_MATCH_PROFILE_INDEX 16 /* valid profileID 1-15 */
/* index to go over Profile Control */

/**
* @internal profileControlDatabaseInit function
* @endinternal
*
*/
static CMD_STATUS profileControlDatabaseInit(void)
{
    GT_U32      index = 0;

    firstFree_1 = 0;

    while (index < PROFILE_CONTROL_TABLE_SIZE)
    {
        /* init all 15 profileId entries as valid */
        profileControlDatabase[index].valid = GT_TRUE;
        profileControlDatabase[index].exactMatchProfileIndex = index + 1;
        profileControlDatabase[index].nextFree = index + 1;
        if (index==(PROFILE_CONTROL_TABLE_SIZE-1))
        {
            profileControlDatabase[index].nextFree = INVALID_EXACT_MATCH_PROFILE_INDEX;
        }
        index++;
    }
    return CMD_OK;
}

/**
* @internal profileControlDatabaseGet function
* @endinternal
*
*/
static void profileControlDatabaseGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  exactMatchProfileIndex,
    OUT GT_U32                  *tableIndex
)
{
    GT_U32              index = 0;
    GT_BOOL             entryFound = GT_FALSE;

    while ((!entryFound) && (index < PROFILE_CONTROL_TABLE_SIZE))
    {
        /* get next valid entry */
        while ((profileControlDatabase[index].valid == GT_FALSE) && (index < PROFILE_CONTROL_TABLE_SIZE))
            index++;
        /* check if the entry match the device number and rule index */
        if ((index < PROFILE_CONTROL_TABLE_SIZE) &&
            (profileControlDatabase[index].exactMatchProfileIndex == exactMatchProfileIndex) &&
            (profileControlDatabase[index].devNum == devNum))
        {
            entryFound = GT_TRUE;
        }
        else
        {
            index++;
        }
    }

    if (entryFound)
    {
        *tableIndex = index;
    }
    else
    {
        *tableIndex = INVALID_EXACT_MATCH_PROFILE_INDEX;
    }
}

/**
* @internal cpssDxChExactMatchProfileKeyParamsSet function
* @endinternal
*
* @brief   Sets the Exact Match Profile key parameters
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum            - device number
* @param[in] exactMatchProfileIndex - Exact Match
*            profile identifier (APPLICABLE RANGES: 1..15)
* @param[in] keyParamsPtr      - (pointer to)Exact Match
*                                profile key parameters
*
* @retval GT_OK                - on success
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_BAD_PARAM         - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static CMD_STATUS wrCpssDxChExactMatchProfileKeyParamsSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                       result = GT_OK;
    GT_U8                                           devNum;
    GT_U32                                          exactMatchProfileIndex;
    GT_U32                                          index;
    CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    keyParams;
    GT_BOOL                                         isNewIndex = GT_FALSE;
    profileControlDB                                profileControlTemp = {0};
    GT_U32                                          i=0;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    exactMatchProfileIndex = (GT_U32)inFields[0];

    keyParams.keySize = (CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT)inFields[1];
    keyParams.keyStart = (GT_U32)inFields[2];
    for (i=0;i<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS;i++)
    {
        keyParams.mask[i] = (GT_U32)inFields[i+3];
    }


    /* in case it is first usage of default action database, initialize it */
    if (firstRun_1)
    {
       profileControlDatabaseInit();
        firstRun_1 = GT_FALSE;
    }

    /* look for the Profile Control in the Profile Control database */
    profileControlDatabaseGet(devNum, exactMatchProfileIndex, &index);

    /* the action wasn't in the default action database */
    if (index == INVALID_EXACT_MATCH_PROFILE_INDEX)
    {
        if (firstFree_1 == INVALID_EXACT_MATCH_PROFILE_INDEX)
        {
            /* pack output arguments to galtis string */
            galtisOutput(outArgs, GT_NO_RESOURCE, "%d", -1);
            return CMD_AGENT_ERROR;
        }

        index = firstFree_1;
        firstFree_1 = profileControlDatabase[firstFree_1].nextFree;
        isNewIndex = GT_TRUE;
    }

    /* this is update of an existing action:
       save the old action parameters */
    if (isNewIndex == GT_FALSE)
    {
        cpssOsMemCpy(&profileControlTemp,&profileControlDatabase[index],sizeof(profileControlTemp));
    }

    /* set action entry in default action database as valid */
    profileControlDatabase[index].valid                      = GT_TRUE;
    profileControlDatabase[index].exactMatchProfileIndex     = exactMatchProfileIndex;
    profileControlDatabase[index].devNum                     = devNum;

    /* call cpss api function */
    result = cpssDxChExactMatchProfileKeyParamsSet(devNum, exactMatchProfileIndex, &keyParams);


    /* if the set didn't succeed:  */
    if (result != GT_OK)
    {
        /* if this is a new action remove it from the database */
        if (isNewIndex)
        {
            profileControlDatabase[index].valid = GT_FALSE;
            profileControlDatabase[index].nextFree = firstFree_1;
            firstFree_1 = index;
        }
        /* if this is an update for existing action restore previous data */
        else
        {
            cpssOsMemCpy(&profileControlDatabase[index],&profileControlTemp,sizeof(profileControlTemp));
        }
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/*  table profileControlDbIndexGet global variable   */

static GT_U32    profileControlDbIndexGet;

/******************************************************************************/
static CMD_STATUS wrCpssDxChExactMatchProfileKeyParamsGetNext

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                       result = GT_OK;
    GT_U8                                           devNum;
    GT_U32                                          exactMatchProfileIndex;
    CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    keyParams;
    GT_U32                                          i=0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* get next valid TTI action entry */
    while ((profileControlDbIndexGet < PROFILE_CONTROL_TABLE_SIZE) &&
           (profileControlDatabase[profileControlDbIndexGet].valid == GT_FALSE))
        profileControlDbIndexGet++;

    if (profileControlDbIndexGet == DEFAULT_ACTION_TABLE_SIZE)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }


    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    exactMatchProfileIndex= profileControlDatabase[profileControlDbIndexGet].exactMatchProfileIndex;
    cmdOsMemSet(&keyParams, 0, sizeof(keyParams));
    result = cpssDxChExactMatchProfileKeyParamsGet(devNum, exactMatchProfileIndex,&keyParams);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = exactMatchProfileIndex;
    inFields[1] = keyParams.keySize;
    inFields[2] = keyParams.keyStart;
    for (i=0;i<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS;i++)
    {
        inFields[i+3]=keyParams.mask[i];
    }

    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                "%d%d%d%d%d%d%d%d%d%d",
                inFields[0],  inFields[1],   inFields[2],   inFields[3],
                inFields[4],  inFields[5],   inFields[6],   inFields[7],
                inFields[8],  inFields[9],   inFields[10],  inFields[11],
                inFields[12], inFields[13],  inFields[14],  inFields[15],
                inFields[16], inFields[17],  inFields[18],  inFields[19],
                inFields[20], inFields[21],  inFields[22],  inFields[23],
                inFields[24], inFields[25],  inFields[26],  inFields[27],
                inFields[28], inFields[29],  inFields[30],  inFields[31],
                inFields[32], inFields[33],  inFields[34],  inFields[35],
                inFields[36], inFields[37],  inFields[38],  inFields[39],
                inFields[40], inFields[41],  inFields[42],  inFields[43],
                inFields[44], inFields[45],  inFields[46],  inFields[47],
                inFields[48], inFields[49]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%f");

    profileControlDbIndexGet++;

    return CMD_OK;
}

/**
* @internal cpssDxChExactMatchProfileKeyParamsGet function
* @endinternal
*
* @brief   Gets the Exact Match Profile key parameters
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum            - device number
* @param[in] exactMatchProfileIndex - Exact Match profile
*                 identifier (APPLICABLE RANGES:1..15)
* @param[in] keyParamsPtr      - (pointer to)Exact Match
*                                profile key parameters
*
* @retval GT_OK                - on success
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_BAD_PARAM         - on wrong param
* @retval GT_BAD_PTR           - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static CMD_STATUS wrCpssDxChExactMatchProfileKeyParamsGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* in case it is first usage of default action database, initialize it */
    if (firstRun_1)
    {
       profileControlDatabaseInit();
        firstRun_1 = GT_FALSE;
    }

    profileControlDbIndexGet = 0;

    return wrCpssDxChExactMatchProfileKeyParamsGetNext(inArgs,inFields,numFields,outArgs);
}

/*************************************************/
/* cpssDxChExactMatchExpandedAction Table        */
/*************************************************/

/* table cpssDxChExactMatchExpandedAction global variables */
#define EXPANDED_ACTION_TABLE_SIZE 16

/* Database to hold Expanded actions that has been set.
   Non valid entries maintain link list of free table entries */
typedef struct
{
    GT_BOOL                                 valid;                  /* weather this entry is valid           */
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT   actionType;             /* action type                           */
    GT_U32                                  expandedActionIndex;    /* index associated with the action      */
    GT_U8                                   devNum;                 /* device associated with the action     */
    GT_U32                                  nextFree;               /* in case not valid, next free entry    */
}expandedActionDB;

static expandedActionDB expandedActionDatabase[EXPANDED_ACTION_TABLE_SIZE];

/* weather expanded action was used before or not (used to invalidate all entries
   on first time the table is used) */
static GT_BOOL  firstRunExpanded = GT_TRUE;
/* indicated invalid rule entry */
#define EXPANDED_INVALID_EXACT_MATCH_PROFILE_INDEX 16 /* valid profileID 1-15 */

/**
* @internal expandedActionDatabaseInit function
* @endinternal
*
*/
static CMD_STATUS expandedActionDatabaseInit(void)
{
    GT_U32      index = 1;

    while (index < EXPANDED_ACTION_TABLE_SIZE)
    {
        /* init all 15 profileId entries as valid */
        expandedActionDatabase[index].valid = GT_FALSE;
        expandedActionDatabase[index].expandedActionIndex = index;
        index++;
    }
    return CMD_OK;
}

/**
* @internal expandedActionDatabaseGet function
* @endinternal
*
*/
static void expandedActionDatabaseGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  expandedActionIndex,
    OUT GT_U32                  *tableIndex
)
{
    GT_U32              index = 1;
    GT_BOOL             entryFound = GT_FALSE;

    while ((!entryFound) && (index < EXPANDED_ACTION_TABLE_SIZE))
    {
        /* get next valid entry */
        while ((expandedActionDatabase[index].valid == GT_FALSE) && (index < EXPANDED_ACTION_TABLE_SIZE))
            index++;
        /* check if the entry match the device number and rule index */
        if ((index < EXPANDED_ACTION_TABLE_SIZE) &&
            (expandedActionDatabase[index].expandedActionIndex == expandedActionIndex) &&
            (expandedActionDatabase[index].devNum == devNum))
        {
            entryFound = GT_TRUE;
        }
        else
        {
            index++;
        }
    }

    if (entryFound)
    {
        *tableIndex = expandedActionDatabase[index].expandedActionIndex;
    }
    else
    {
        *tableIndex = EXPANDED_INVALID_EXACT_MATCH_PROFILE_INDEX;
    }
}

/**
* @internal wrCpssDxChExactMatchExpandedActionUpdate function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChExactMatchExpandedActionUpdate
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                           result = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              expandedActionIndex;
    GT_U32                                              index;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT               actionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    actionData;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    expandedActionOriginData;

    GT_BOOL                                             isNewIndex = GT_FALSE;
    expandedActionDB                                    expandedTempAction = {0};
    GT_U32                                              i=0;
    GT_U32                                              saveArpPtr;


    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    actionType  = (CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT)inArgs[1];

    expandedActionIndex = (GT_U32)inFields[0];
    if (expandedActionIndex<1 || expandedActionIndex>15)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_NO_RESOURCE, "%d", -1);
        return CMD_AGENT_ERROR;
    }
    i = 1;

    /* in case it is first usage of expanded action database, initialize it */
    if (firstRunExpanded)
    {
        expandedActionDatabaseInit();
        firstRunExpanded = GT_FALSE;
    }

    /* look for the action in the expanded action database */
    expandedActionDatabaseGet(devNum, expandedActionIndex, &index);

    /* the action wasn't in the expanded action database */
    if (index == EXPANDED_INVALID_EXACT_MATCH_PROFILE_INDEX)
    {
        /* the DB was initialized in a way that in
           index i we hold the data for expandedAction i */
        index = expandedActionDatabase[expandedActionIndex].expandedActionIndex;
        isNewIndex = GT_TRUE;
    }

    /* this is update of an existing action:
       save the old action parameters */
    if (isNewIndex == GT_FALSE)
    {
        cpssOsMemCpy(&expandedTempAction,&expandedActionDatabase[index],sizeof(expandedTempAction));
    }

    /* set action entry in expanded action database as valid */
    expandedActionDatabase[index].valid                      = GT_TRUE;
    expandedActionDatabase[index].expandedActionIndex        = expandedActionIndex;
    expandedActionDatabase[index].devNum                     = devNum;
    expandedActionDatabase[index].actionType                 = (CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT)inArgs[0];

    cmdOsMemSet(&actionData, 0, sizeof(actionData));
    cmdOsMemSet(&expandedActionOriginData, 0, sizeof(expandedActionOriginData));

    /* fill action fields */
    switch (actionType)
    {
    case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTunnelTerminate = (GT_BOOL)inFields[i++];
        actionData.ttiAction.tunnelTerminate                 = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTtPassengerPacketType = (GT_BOOL)inFields[i++];
        actionData.ttiAction.ttPassengerPacketType           = (CPSS_DXCH_TTI_PASSENGER_TYPE_ENT)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType = (GT_BOOL)inFields[i++];
        actionData.ttiAction.tsPassengerPacketType           = (CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT)inFields[i++];

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTtHeaderLength = (GT_BOOL)inFields[i++];
        actionData.ttiAction.ttHeaderLength                  = (GT_U32)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionContinueToNextTtiLookup = (GT_BOOL)inFields[i++];
        actionData.ttiAction.continueToNextTtiLookup         = (GT_BOOL)inFields[i++];

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionCopyTtlExpFromTunnelHeader = (GT_BOOL)inFields[i++];
        actionData.ttiAction.copyTtlExpFromTunnelHeader      = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionMplsCommand = (GT_BOOL)inFields[i++];
        actionData.ttiAction.mplsCommand                     = (CPSS_DXCH_TTI_MPLS_CMD_ENT)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionMplsTtl = (GT_BOOL)inFields[i++];
        actionData.ttiAction.mplsTtl                         = (GT_U32)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionEnableDecrementTtl = (GT_BOOL)inFields[i++];
        actionData.ttiAction.enableDecrementTtl              = (GT_BOOL)inFields[i++];

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitMplsTunnelMode = (GT_BOOL)inFields[i++];
        actionData.ttiAction.passengerParsingOfTransitMplsTunnelMode = (CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_TYPE_ENT)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitNonMplsTransitTunnelEnable = (GT_BOOL)inFields[i++];
        actionData.ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable = (GT_BOOL)inFields[i++];

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionCommand = (GT_BOOL)inFields[i++];
        actionData.ttiAction.command                         = (CPSS_PACKET_CMD_ENT)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand = (GT_BOOL)inFields[i++];
        actionData.ttiAction.redirectCommand                 = (CPSS_DXCH_TTI_REDIRECT_COMMAND_ENT)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface = (GT_BOOL)inFields[i++];
        actionData.ttiAction.egressInterface.type            = (CPSS_INTERFACE_TYPE_ENT)inFields[i++];
        actionData.ttiAction.egressInterface.devPort.hwDevNum  = (GT_HW_DEV_NUM)inFields[i++];
        actionData.ttiAction.egressInterface.devPort.portNum = (GT_PORT_NUM)inFields[i++];
        /* Override Device and Port */
        CONVERT_DEV_PORT_DATA_MAC(actionData.ttiAction.egressInterface.devPort.hwDevNum, actionData.ttiAction.egressInterface.devPort.portNum);
        actionData.ttiAction.egressInterface.trunkId         = (GT_TRUNK_ID)inFields[i++];
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(actionData.ttiAction.egressInterface.trunkId);
        actionData.ttiAction.egressInterface.vidx            = (GT_U16)inFields[i++];
        actionData.ttiAction.egressInterface.vlanId          = (GT_U16)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionArpPtr = (GT_BOOL)inFields[i++];
        actionData.ttiAction.arpPtr                          = (GT_U32)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTunnelStart = (GT_BOOL)inFields[i++];
        actionData.ttiAction.tunnelStart                     = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTunnelIndex = (GT_BOOL)inFields[i++];
        actionData.ttiAction.tunnelStartPtr                  = (GT_U32)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRouterLttPtr = (GT_BOOL)inFields[i++];
        actionData.ttiAction.routerLttPtr                    = (GT_U32)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionVrfId = (GT_BOOL)inFields[i++];
        actionData.ttiAction.vrfId                           = (GT_U32)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceIdSetEnable = (GT_BOOL)inFields[i++];
        actionData.ttiAction.sourceIdSetEnable               = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceId = (GT_BOOL)inFields[i++];
        actionData.ttiAction.sourceId                        = (GT_U32)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanCmd = (GT_BOOL)inFields[i++];
        actionData.ttiAction.tag0VlanCmd                     = (CPSS_DXCH_TTI_VLAN_COMMAND_ENT)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId= (GT_BOOL)inFields[i++];
        actionData.ttiAction.tag0VlanId                      = (GT_U16)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanCmd = (GT_BOOL)inFields[i++];
        actionData.ttiAction.tag1VlanCmd                     = (CPSS_DXCH_TTI_VLAN_COMMAND_ENT)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanId = (GT_BOOL)inFields[i++];
        actionData.ttiAction.tag1VlanId                      = (GT_U16)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanPrecedence = (GT_BOOL)inFields[i++];
        actionData.ttiAction.tag0VlanPrecedence              = (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable = (GT_BOOL)inFields[i++];
        actionData.ttiAction.nestedVlanEnable                = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicerMeter = (GT_BOOL)inFields[i++];
        actionData.ttiAction.bindToPolicerMeter              = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicer = (GT_BOOL)inFields[i++];
        actionData.ttiAction.bindToPolicer                   = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionPolicerIndex = (GT_BOOL)inFields[i++];
        actionData.ttiAction.policerIndex                    = (GT_U32)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionQosPrecedence = (GT_BOOL)inFields[i++];
        actionData.ttiAction.qosPrecedence                   = (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionKeepPreviousQoS = (GT_BOOL)inFields[i++];
        actionData.ttiAction.keepPreviousQoS                 = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTrustUp = (GT_BOOL)inFields[i++];
        actionData.ttiAction.trustUp                         = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTrustDscp = (GT_BOOL)inFields[i++];
        actionData.ttiAction.trustDscp                       = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTrustExp = (GT_BOOL)inFields[i++];
        actionData.ttiAction.trustExp                        = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionQosProfile = (GT_BOOL)inFields[i++];
        actionData.ttiAction.qosProfile                      = (GT_U32)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionModifyTag0 = (GT_BOOL)inFields[i++];
        actionData.ttiAction.modifyTag0Up                    = (CPSS_DXCH_TTI_MODIFY_UP_ENT)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag1UpCommand = (GT_BOOL)inFields[i++];
        actionData.ttiAction.tag1UpCommand                   = (CPSS_DXCH_TTI_TAG1_UP_COMMAND_ENT)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionModifyDscp = (GT_BOOL)inFields[i++];
        actionData.ttiAction.modifyDscp                      = (CPSS_DXCH_TTI_MODIFY_DSCP_ENT)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0Up = (GT_BOOL)inFields[i++];
        actionData.ttiAction.tag0Up                          = (GT_U32)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag1Up = (GT_BOOL)inFields[i++];
        actionData.ttiAction.tag1Up                          = (GT_U32)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRemapDscp = (GT_BOOL)inFields[i++];
        actionData.ttiAction.remapDSCP                       = (GT_BOOL)inFields[i++];

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionQosUseUpAsIndexEnable = (GT_BOOL)inFields[i++];
        actionData.ttiAction.qosUseUpAsIndexEnable           = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionQosMappingTableIndex = (GT_BOOL)inFields[i++];
        actionData.ttiAction.qosMappingTableIndex            = (GT_U32)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionMplsLLspQoSProfileEnable = (GT_BOOL)inFields[i++];
        actionData.ttiAction.mplsLLspQoSProfileEnable        = (GT_BOOL)inFields[i++];

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionPcl0OverrideConfigIndex = (GT_BOOL)inFields[i++];
        actionData.ttiAction.pcl0OverrideConfigIndex         = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionPcl1OverrideConfigIndex = (GT_BOOL)inFields[i++];
        actionData.ttiAction.pcl0_1OverrideConfigIndex       = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionPcl2OverrideConfigIndex = (GT_BOOL)inFields[i++];
        actionData.ttiAction.pcl1OverrideConfigIndex         = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionIpclConfigIndex = (GT_BOOL)inFields[i++];
        actionData.ttiAction.iPclConfigIndex                 = (GT_U32)inFields[i++];

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionIpclUdbConfigTable = (GT_BOOL)inFields[i++];
        actionData.ttiAction.iPclUdbConfigTableEnable        = (GT_BOOL)inFields[i++];
        actionData.ttiAction.iPclUdbConfigTableIndex         = (CPSS_DXCH_PCL_PACKET_TYPE_ENT)inFields[i++];

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionMirrorToIngressAnalyzerIndex = (GT_BOOL)inFields[i++];
        actionData.ttiAction.mirrorToIngressAnalyzerEnable   = (GT_BOOL)inFields[i++];
        actionData.ttiAction.mirrorToIngressAnalyzerIndex    = (GT_U32)inFields[i++];

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode = (GT_BOOL)inFields[i++];
        actionData.ttiAction.userDefinedCpuCode              = (CPSS_NET_RX_CPU_CODE_ENT)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionBindToCentralCounter = (GT_BOOL)inFields[i++];
        actionData.ttiAction.bindToCentralCounter            = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionCentralCounterIndex = (GT_BOOL)inFields[i++];
        actionData.ttiAction.centralCounterIndex             = (GT_U32)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionVntl2Echo = (GT_BOOL)inFields[i++];
        actionData.ttiAction.vntl2Echo                       = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionBridgeBypass = (GT_BOOL)inFields[i++];
        actionData.ttiAction.bridgeBypass                    = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionIngressPipeBypass = (GT_BOOL)inFields[i++];
        actionData.ttiAction.ingressPipeBypass               = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionActionStop = (GT_BOOL)inFields[i++];
        actionData.ttiAction.actionStop                      = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionHashMaskIndex = (GT_BOOL)inFields[i++];
        actionData.ttiAction.hashMaskIndex                   = (GT_U32)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionModifyMacSa = (GT_BOOL)inFields[i++];
        actionData.ttiAction.modifyMacSa                     = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionModifyMacDa = (GT_BOOL)inFields[i++];
        actionData.ttiAction.modifyMacDa                     = (GT_BOOL)inFields[i++];

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable = (GT_BOOL)inFields[i++];
        actionData.ttiAction.sourceEPortAssignmentEnable     = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort = (GT_BOOL)inFields[i++];
        actionData.ttiAction.sourceEPort                     = (GT_PORT_NUM)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionFlowId = (GT_BOOL)inFields[i++];
        actionData.ttiAction.flowId                          = (GT_U32)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSetMacToMe = (GT_BOOL)inFields[i++];
        actionData.ttiAction.setMacToMe                      = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRxProtectionSwitchEnable = (GT_BOOL)inFields[i++];
        actionData.ttiAction.rxProtectionSwitchEnable        = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRxIsProtectionPath = (GT_BOOL)inFields[i++];
        actionData.ttiAction.rxIsProtectionPath              = (GT_BOOL)inFields[i++];

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionOamTimeStampEnable = (GT_BOOL)inFields[i++];
        actionData.ttiAction.oamTimeStampEnable              = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex = (GT_BOOL)inFields[i++];
        actionData.ttiAction.oamOffsetIndex                  = (GT_U32)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionOamProcessEnable = (GT_BOOL)inFields[i++];
        actionData.ttiAction.oamProcessEnable                = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionOamProfile = (GT_BOOL)inFields[i++];
        actionData.ttiAction.oamProfile                      = (GT_U32)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable = (GT_BOOL)inFields[i++];
        actionData.ttiAction.oamChannelTypeToOpcodeMappingEnable = (GT_BOOL)inFields[i++];

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionIsPtpPacket = (GT_BOOL)inFields[i++];
        actionData.ttiAction.isPtpPacket                     = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionPtpTriggerType = (GT_BOOL)inFields[i++];
        actionData.ttiAction.ptpTriggerType                  = (CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_ENT)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionPtpOffset = (GT_BOOL)inFields[i++];
        actionData.ttiAction.ptpOffset                       = (GT_U32)inFields[i++];

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionCwBasedPw = (GT_BOOL)inFields[i++];
        actionData.ttiAction.cwBasedPw                       = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTtlExpiryVccvEnable = (GT_BOOL)inFields[i++];
        actionData.ttiAction.ttlExpiryVccvEnable             = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionPwe3FlowLabelExist = (GT_BOOL)inFields[i++];
        actionData.ttiAction.pwe3FlowLabelExist              = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionPwCwBasedETreeEnable = (GT_BOOL)inFields[i++];
        actionData.ttiAction.pwCwBasedETreeEnable            = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionApplyNonDataCwCommand = (GT_BOOL)inFields[i++];
        actionData.ttiAction.applyNonDataCwCommand           = (GT_BOOL)inFields[i++];

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTunnelHeaderStartL4Enable = (GT_BOOL)inFields[i++];
        actionData.ttiAction.tunnelHeaderStartL4Enable       = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookupEnable = (GT_BOOL)inFields[i++];
        actionData.ttiAction.skipFdbSaLookupEnable           = (GT_BOOL)inFields[i++];
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionIpv6SegmentRoutingEndNodeEnable = (GT_BOOL)inFields[i++];
        actionData.ttiAction.ipv6SegmentRoutingEndNodeEnable = (GT_BOOL)inFields[i++];

        break;

    case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
    case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
        /* field index */
        i = 1;

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd = (GT_BOOL)inFields[i++];
        actionData.pclAction.pktCmd = (CPSS_PACKET_CMD_ENT)inFields[i++];
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionActionStop = (GT_BOOL)inFields[i++];
        actionData.pclAction.actionStop    =(GT_BOOL)inFields[i++];

        actionData.pclAction.egressPolicy  = (GT_BOOL)inFields[i++];

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode = (GT_BOOL)inFields[i++];
        actionData.pclAction.mirror.cpuCode=(CPSS_NET_RX_CPU_CODE_ENT)inFields[i++];

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMirror            = (GT_BOOL)inFields[i++];
        actionData.pclAction.mirror.mirrorToRxAnalyzerPort              =(GT_BOOL)inFields[i++];
        actionData.pclAction.mirror.ingressMirrorToAnalyzerIndex        =(GT_U32)inFields[i++];
        actionData.pclAction.mirror.mirrorToTxAnalyzerPortEn            = (GT_U32)inFields[i++];
        actionData.pclAction.mirror.egressMirrorToAnalyzerIndex         = (GT_U32)inFields[i++];

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMirrorMode        = (GT_BOOL)inFields[i++];
        actionData.pclAction.mirror.egressMirrorToAnalyzerMode          = (CPSS_DXCH_MIRROR_EGRESS_MODE_ENT)inFields[i++];


        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMatchCounterEnable = (GT_BOOL)inFields[i++];
        actionData.pclAction.matchCounter.enableMatchCount=(GT_BOOL)inFields[i++];
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMatchCounterIndex = (GT_BOOL)inFields[i++];
        actionData.pclAction.matchCounter.matchCounterIndex=(GT_U32)inFields[i++];

        switch (actionData.pclAction.egressPolicy)
        {
            case GT_FALSE: /* ingress */
                expandedActionDatabase[index].actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;

                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionQoSModifyDSCP = (GT_BOOL)inFields[i];
                actionData.pclAction.qos.ingress.modifyDscp =(CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[i+1];

                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyUp0 = (GT_BOOL)inFields[i+2];
                actionData.pclAction.qos.ingress.modifyUp =(CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[i+3];

                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionQoSProfile = (GT_BOOL)inFields[i+4];
                actionData.pclAction.qos.ingress.profileIndex = (GT_U32)inFields[i+5];
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionQosProfileMakingEnable = (GT_BOOL)inFields[i+6];
                actionData.pclAction.qos.ingress.profileAssignIndex = (GT_BOOL)inFields[i+7];

                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionQosPrecedence = (GT_BOOL)inFields[i+8];
                actionData.pclAction.qos.ingress.profilePrecedence =(CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[i+9];

                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUp1Command = (GT_BOOL)inFields[i+10];
                actionData.pclAction.qos.ingress.up1Cmd =(CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ENT)inFields[i+11];
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUp1 = (GT_BOOL)inFields[i+12];
                actionData.pclAction.qos.ingress.up1 = (GT_U32)inFields[i+13];
                break;

            default:
            case GT_TRUE: /* egress */
                expandedActionDatabase[index].actionType = CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E;

                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionQoSModifyDSCP = (GT_BOOL)inFields[i+14];
                actionData.pclAction.qos.egress.modifyDscp =(CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_ENT)(inFields[i+15]);
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionDscpExp = (GT_BOOL)inFields[i+16];
                actionData.pclAction.qos.egress.dscp = (GT_U32)inFields[i+17];
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyUp0 = (GT_BOOL)inFields[i+18];
                actionData.pclAction.qos.egress.modifyUp =(CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT)(inFields[i+19]);
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUp0 = (GT_BOOL)inFields[i+20];
                actionData.pclAction.qos.egress.up   = (GT_U32)inFields[i+21];
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyUp1 = (GT_BOOL)inFields[i+22];
                actionData.pclAction.qos.egress.up1ModifyEnable   = (GT_BOOL)inFields[i+23];
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUp1 = (GT_BOOL)inFields[i+24];
                actionData.pclAction.qos.egress.up1   = (GT_U32)inFields[i+25];
                break;
        }

        /* bypass the switch */
        i += 26;

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand = (GT_BOOL)inFields[i++];
        actionData.pclAction.redirect.redirectCmd = (CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ENT)inFields[i++];
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionEgressInterface = (GT_BOOL)inFields[i];

        if (actionData.pclAction.redirect.redirectCmd == CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E)
        {
            actionData.pclAction.redirect.data.outIf.outInterface.type =
                (CPSS_INTERFACE_TYPE_ENT)inFields[i+1];

            switch (actionData.pclAction.redirect.data.outIf.outInterface.type)
            {
                case CPSS_INTERFACE_PORT_E:
                    actionData.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum =
                        (GT_HW_DEV_NUM)inFields[i+2];
                    actionData.pclAction.redirect.data.outIf.outInterface.devPort.portNum =
                        (GT_PORT_NUM)inFields[i+3];
                    CONVERT_DEV_PORT_DATA_MAC(
                        actionData.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum,
                        actionData.pclAction.redirect.data.outIf.outInterface.devPort.portNum);
                    break;

                case CPSS_INTERFACE_TRUNK_E:
                    actionData.pclAction.redirect.data.outIf.outInterface.trunkId =
                        (GT_TRUNK_ID)inFields[i+4];
                    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(
                       actionData.pclAction.redirect.data.outIf.outInterface.trunkId);
                    break;

                case CPSS_INTERFACE_VIDX_E:
                    actionData.pclAction.redirect.data.outIf.outInterface.vidx =
                        (GT_U16)inFields[i+5];
                    break;

                default:
                    break;
            }
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVntl2Echo = (GT_BOOL)inFields[i+6];
            actionData.pclAction.redirect.data.outIf.vntL2Echo   = (GT_BOOL)inFields[i+7];
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionTunnelStart = (GT_BOOL)inFields[i+8];
            actionData.pclAction.redirect.data.outIf.tunnelStart = (GT_BOOL)inFields[i+9];
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionTunnelIndex = (GT_BOOL)inFields[i+10];
            actionData.pclAction.redirect.data.outIf.tunnelPtr   = (GT_U32)inFields[i+11];
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType = (GT_BOOL)inFields[i+12];
            actionData.pclAction.redirect.data.outIf.tunnelType = (CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ENT)inFields[i+13];
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionArpPtr = (GT_BOOL)inFields[i+14];
            actionData.pclAction.redirect.data.outIf.arpPtr = (GT_U32)inFields[i+15];
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyMacDA = (GT_BOOL)inFields[i+16];
            actionData.pclAction.redirect.data.outIf.modifyMacDa = (GT_BOOL)inFields[i+17];
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyMacSA = (GT_BOOL)inFields[i+18];
            actionData.pclAction.redirect.data.outIf.modifyMacSa = (GT_BOOL)inFields[i+19];
        }
        else
        {
            if (actionData.pclAction.redirect.redirectCmd == CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E)

            {
                /* used the same GUI fields as for redirect to outIf */
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionArpPtr = (GT_BOOL)inFields[i+14];
                actionData.pclAction.redirect.data.noRedirect.arpPtr = (GT_U32)inFields[i+15];
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyMacDA = (GT_BOOL)inFields[i+16];
                actionData.pclAction.redirect.data.noRedirect.modifyMacDa = (GT_BOOL)inFields[i+17];
            }
        }


        /* bypass redirect to IF */
        /* arpPtr relevant also to ...REDIRECT_CMD_REPLACE_MAC_SA_E */
        saveArpPtr = (GT_U32)inFields[i+15];
        i += 20;

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRouterLttPtr = (GT_BOOL)inFields[i];
        if (actionData.pclAction.redirect.redirectCmd == CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E)
        {
            actionData.pclAction.redirect.data.routerLttIndex = (GT_U32)inFields[i+1];
        }
        /* bypass redirect to Next Hop */
        i ++;
        i ++;

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVrfId = (GT_BOOL)inFields[i];
        if (actionData.pclAction.redirect.redirectCmd == CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E)
        {
            actionData.pclAction.redirect.data.vrfId = (GT_U32)inFields[i+1];
        }
        /* bypass redirect to Virtual Router */
        i ++;
        i ++;

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMacSa = (GT_BOOL)inFields[i];
        if (actionData.pclAction.redirect.redirectCmd == CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E)
        {
            galtisMacAddr(&(actionData.pclAction.redirect.data.modifyMacSa.macSa), (GT_U8*)inFields[i+1]);
            /* arpPtr saved from ...REDIRECT_CMD_OUT_IF_E parameters */
            actionData.pclAction.redirect.data.modifyMacSa.arpPtr = saveArpPtr;
        }
        /* bypass replace MAC_SA */
        i ++;
        i ++;

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPolicerEnable = (GT_BOOL)inFields[i++];
        actionData.pclAction.policer.policerEnable = (CPSS_DXCH_PCL_POLICER_ENABLE_ENT)inFields[i++];
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPolicerIndex = (GT_BOOL)inFields[i++];
        actionData.pclAction.policer.policerId = (GT_U32)inFields[i++];

        switch (actionData.pclAction.egressPolicy)
        {
        case GT_FALSE:
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0Command = (GT_BOOL)inFields[i];
                actionData.pclAction.vlan.ingress.modifyVlan = (CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT)inFields[i+1];
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable = (GT_BOOL)inFields[i+3];
                actionData.pclAction.vlan.ingress.nestedVlan = (GT_BOOL)inFields[i+4];
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0 = (GT_BOOL)inFields[i+5];
                actionData.pclAction.vlan.ingress.vlanId = (GT_U32)inFields[i+6];
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlanPrecedence = (GT_BOOL)inFields[i+7];
                actionData.pclAction.vlan.ingress.precedence = (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[i+8];
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan1Command = (GT_BOOL)inFields[i+9];
                actionData.pclAction.vlan.ingress.vlanId1Cmd = (CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ENT)inFields[i+10];
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan1 = (GT_BOOL)inFields[i+12];
                actionData.pclAction.vlan.ingress.vlanId1 = (GT_U32)inFields[i+13];
                break;

            case GT_TRUE:
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0Command = (GT_BOOL)inFields[i];
                actionData.pclAction.vlan.egress.vlanCmd = (CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT)inFields[i+2];
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0 = (GT_BOOL)inFields[i+5];
                actionData.pclAction.vlan.egress.vlanId = (GT_U32)inFields[i+6];
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan1Command = (GT_BOOL)inFields[i+9];
                actionData.pclAction.vlan.egress.vlanId1ModifyEnable = (GT_BOOL)inFields[i+11];
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan1 = (GT_BOOL)inFields[i+12];
                actionData.pclAction.vlan.egress.vlanId1 = (GT_U32)inFields[i+13];
                break;

            default:
                break;
        }
        /* bypass VLAN */
        i += 14;

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourceIdEnable = (GT_BOOL)inFields[i++];
        actionData.pclAction.sourceId.assignSourceId = (GT_BOOL)inFields[i++];
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourceId = (GT_BOOL)inFields[i++];
        actionData.pclAction.sourceId.sourceIdValue  = (GT_U32)inFields[i++];

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionBypassBridge = (GT_BOOL)inFields[i++];
        actionData.pclAction.bypassBridge                           = (GT_BOOL)inFields[i++];
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionBypassIngressPipe = (GT_BOOL)inFields[i++];
        actionData.pclAction.bypassIngressPipe                      = (GT_BOOL)inFields[i++];

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionIPCLConfigurationIndex = (GT_BOOL)inFields[i++];
        actionData.pclAction.lookupConfig.ipclConfigIndex           = (GT_U32)inFields[i++];
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPCL1OverrideConfigIndex = (GT_BOOL)inFields[i++];
        actionData.pclAction.lookupConfig.pcl0_1OverrideConfigIndex =
            (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[i++];
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPCL2OverrideConfigIndex = (GT_BOOL)inFields[i++];
        actionData.pclAction.lookupConfig.pcl1OverrideConfigIndex   =
            (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[i++];
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMirrorTcpRstAndFinPacketsToCpu = (GT_BOOL)inFields[i++];
        actionData.pclAction.mirror.mirrorTcpRstAndFinPacketsToCpu  = (GT_BOOL)inFields[i++];

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionOamTimestampEnable = (GT_BOOL)inFields[i++];
        actionData.pclAction.oam.timeStampEnable  = (GT_BOOL)inFields[i++];
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex = (GT_BOOL)inFields[i++];
        actionData.pclAction.oam.offsetIndex      = (GT_U32)inFields[i++];
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionOamProcessingEnable = (GT_BOOL)inFields[i++];
        actionData.pclAction.oam.oamProcessEnable = (GT_BOOL)inFields[i++];
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionOamProfile = (GT_BOOL)inFields[i++];
        actionData.pclAction.oam.oamProfile       = (GT_U32)inFields[i++];

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionFlowId = (GT_BOOL)inFields[i++];
        actionData.pclAction.flowId       = (GT_U32)inFields[i++];
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSetMacToMe = (GT_BOOL)inFields[i++];
        actionData.pclAction.setMacToMe   = (GT_BOOL)inFields[i++];

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourcePortEnable = (GT_BOOL)inFields[i++];
        actionData.pclAction.sourcePort.assignSourcePortEnable   = (GT_BOOL)inFields[i++];
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourcePort = (GT_BOOL)inFields[i++];
        actionData.pclAction.sourcePort.sourcePortValue          = (GT_U32)inFields[i++];

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable = (GT_BOOL)inFields[i++];
        actionData.pclAction.channelTypeToOpcodeMapEnable   = (GT_BOOL)inFields[i++];

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId = (GT_BOOL)inFields[i++];
        actionData.pclAction.terminateCutThroughMode          = (GT_BOOL)inFields[i++];
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitorEnable = (GT_BOOL)inFields[i++];
        actionData.pclAction.latencyMonitor.monitoringEnable  = (GT_BOOL)inFields[i++];
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitor = (GT_BOOL)inFields[i++];
        actionData.pclAction.latencyMonitor.latencyProfile    = (GT_U32)inFields[i++];
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup = (GT_BOOL)inFields[i++];
        actionData.pclAction.skipFdbSaLookup                  = (GT_BOOL)inFields[i++];
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionTriggerInterrupt = (GT_BOOL)inFields[i++];
        actionData.pclAction.triggerInterrupt                 = (GT_BOOL)inFields[i++];

        /* TBD pha fields should be updated after implementation will be added */

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPhaMetadataAssignEnable = (GT_BOOL)inFields[i];
        /*actionData.epclPhaInfo.phaThreadType   = (GT_BOOL)inFields[i+1];*/
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPhaMetadata = (GT_BOOL)inFields[i+2];
         /*actionData.epclPhaInfo.phaThreadUnion   = (GT_BOOL)inFields[i+3];*/
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPhaThreadNumberAssignEnable = (GT_BOOL)inFields[i+4];
         /*actionData.epclPhaInfo.phaThreadIdAssignmentMode   = (GT_BOOL)inFields[i+5];*/
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPhaThreadNumber = (GT_BOOL)inFields[i+6];
         /*actionData.epclPhaInfo.phaThreadId   = (GT_BOOL)inFields[i+7];*/


        break;

    default:
        galtisOutput(outArgs, GT_BAD_PARAM, "");
        return CMD_AGENT_ERROR;
    }

    /* call cpss api function */
    result = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);


    /* if the rule action didn't succeed:  */
    if (result != GT_OK)
    {
        /* if this is a new action remove it from the database */
        if (isNewIndex)
        {
            expandedActionDatabase[index].valid = GT_FALSE;
        }
        /* if this is an update for existing action restore previous data */
        else
        {
            cpssOsMemCpy(&expandedActionDatabase[index],&expandedTempAction,sizeof(expandedTempAction));
        }
    }


    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/*  table expandedActionDbIndexGet global variable   */

static GT_U32    expandedActionDbIndexGet;

/******************************************************************************/
static CMD_STATUS wrCpssDxChExactMatchExpandedActionGetNext

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                           result = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              expandedActionIndex;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT               actionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    actionData;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    expandedActionOriginData;
    GT_U32                                              i=0;
    GT_HW_DEV_NUM                                       tempHwDev;  /* used for port,dev converting */
    GT_PORT_NUM                                         tempPort;   /* used for port,dev converting */
    char                                                workStr[32];
    GT_U8                                               *workPtr;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* get next valid TTI action entry */
    while ((expandedActionDbIndexGet < EXPANDED_ACTION_TABLE_SIZE) &&
           (expandedActionDatabase[expandedActionDbIndexGet].valid == GT_FALSE))
        expandedActionDbIndexGet++;

    if (expandedActionDbIndexGet == EXPANDED_ACTION_TABLE_SIZE)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }


    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    actionType  = expandedActionDatabase[expandedActionDbIndexGet].actionType;
    expandedActionIndex= expandedActionDatabase[expandedActionDbIndexGet].expandedActionIndex;

    cmdOsMemSet(&actionData, 0, sizeof(actionData));
    cmdOsMemSet(&expandedActionOriginData, 0x1, sizeof(expandedActionOriginData));

    result = cpssDxChExactMatchExpandedActionGet(devNum, expandedActionIndex, actionType, &actionData, &expandedActionOriginData);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    switch (actionType)
    {
    case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
        inArgs[1] = actionType;
        i=0;
        inFields[i++]  = expandedActionIndex;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTunnelTerminate;
        inFields[i++]  = actionData.ttiAction.tunnelTerminate;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTtPassengerPacketType;
        inFields[i++]  = actionData.ttiAction.ttPassengerPacketType;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType;
        inFields[i++]  = actionData.ttiAction.tsPassengerPacketType;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTtHeaderLength;
        inFields[i++]  = actionData.ttiAction.ttHeaderLength;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionContinueToNextTtiLookup;
        inFields[i++]  = actionData.ttiAction.continueToNextTtiLookup;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionCopyTtlExpFromTunnelHeader;
        inFields[i++]  = actionData.ttiAction.copyTtlExpFromTunnelHeader;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionMplsCommand;
        inFields[i++]  = actionData.ttiAction.mplsCommand;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionMplsTtl;
        inFields[i++]  = actionData.ttiAction.mplsTtl;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionEnableDecrementTtl;
        inFields[i++]  = actionData.ttiAction.enableDecrementTtl;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitMplsTunnelMode;
        inFields[i++]  = actionData.ttiAction.passengerParsingOfTransitMplsTunnelMode;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitNonMplsTransitTunnelEnable;
        inFields[i++]  = actionData.ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionCommand;
        inFields[i++]  = actionData.ttiAction.command;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand;
        inFields[i++] = actionData.ttiAction.redirectCommand;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface;
        inFields[i++] = actionData.ttiAction.egressInterface.type;
        tempHwDev      = actionData.ttiAction.egressInterface.devPort.hwDevNum;
        tempPort     = actionData.ttiAction.egressInterface.devPort.portNum;
        CONVERT_BACK_DEV_PORT_DATA_MAC(tempHwDev,tempPort);
        inFields[i++] = tempHwDev;
        inFields[i++] = tempPort;
        CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(actionData.ttiAction.egressInterface.trunkId);

        inFields[i++] = actionData.ttiAction.egressInterface.trunkId;
        inFields[i++] = actionData.ttiAction.egressInterface.vidx;
        inFields[i++] = actionData.ttiAction.egressInterface.vlanId;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionArpPtr;
        inFields[i++] = actionData.ttiAction.arpPtr;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTunnelStart;
        inFields[i++] = actionData.ttiAction.tunnelStart;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTunnelIndex;
        inFields[i++] = actionData.ttiAction.tunnelStartPtr;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRouterLttPtr;
        inFields[i++] = actionData.ttiAction.routerLttPtr;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionVrfId;
        inFields[i++] = actionData.ttiAction.vrfId;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceIdSetEnable;
        inFields[i++] = actionData.ttiAction.sourceIdSetEnable;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceId;
        inFields[i++] = actionData.ttiAction.sourceId;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanCmd;
        inFields[i++] = actionData.ttiAction.tag0VlanCmd;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId;
        inFields[i++] = actionData.ttiAction.tag0VlanId;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanCmd;
        inFields[i++] = actionData.ttiAction.tag1VlanCmd;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanId;
        inFields[i++] = actionData.ttiAction.tag1VlanId;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanPrecedence;
        inFields[i++] = actionData.ttiAction.tag0VlanPrecedence;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable;
        inFields[i++] = actionData.ttiAction.nestedVlanEnable;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicerMeter;
        inFields[i++] = actionData.ttiAction.bindToPolicerMeter;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicer;
        inFields[i++] = actionData.ttiAction.bindToPolicer;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionPolicerIndex;
        inFields[i++] = actionData.ttiAction.policerIndex;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionQosPrecedence;
        inFields[i++] = actionData.ttiAction.qosPrecedence;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionKeepPreviousQoS;
        inFields[i++] = actionData.ttiAction.keepPreviousQoS;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTrustUp;
        inFields[i++] = actionData.ttiAction.trustUp;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTrustDscp;
        inFields[i++] = actionData.ttiAction.trustDscp;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTrustExp;
        inFields[i++] = actionData.ttiAction.trustExp;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionQosProfile;
        inFields[i++] = actionData.ttiAction.qosProfile;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionModifyTag0;
        inFields[i++] = actionData.ttiAction.modifyTag0Up;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag1UpCommand;
        inFields[i++] = actionData.ttiAction.tag1UpCommand;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionModifyDscp;
        inFields[i++] = actionData.ttiAction.modifyDscp;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0Up;
        inFields[i++] = actionData.ttiAction.tag0Up;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag1Up;
        inFields[i++] = actionData.ttiAction.tag1Up;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRemapDscp;
        inFields[i++] = actionData.ttiAction.remapDSCP;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionQosUseUpAsIndexEnable;
        inFields[i++] = actionData.ttiAction.qosUseUpAsIndexEnable;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionQosMappingTableIndex;
        inFields[i++] = actionData.ttiAction.qosMappingTableIndex;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionMplsLLspQoSProfileEnable;
        inFields[i++] = actionData.ttiAction.mplsLLspQoSProfileEnable;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionPcl0OverrideConfigIndex;
        inFields[i++] = actionData.ttiAction.pcl0OverrideConfigIndex;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionPcl1OverrideConfigIndex;
        inFields[i++] = actionData.ttiAction.pcl0_1OverrideConfigIndex;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionPcl2OverrideConfigIndex;
        inFields[i++] = actionData.ttiAction.pcl1OverrideConfigIndex;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionIpclConfigIndex;
        inFields[i++] = actionData.ttiAction.iPclConfigIndex;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionIpclUdbConfigTable;
        inFields[i++] = actionData.ttiAction.iPclUdbConfigTableEnable;
        inFields[i++] = actionData.ttiAction.iPclUdbConfigTableIndex;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionMirrorToIngressAnalyzerIndex;
        inFields[i++] = actionData.ttiAction.mirrorToIngressAnalyzerEnable;
        inFields[i++] = actionData.ttiAction.mirrorToIngressAnalyzerIndex;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode;
        inFields[i++] = actionData.ttiAction.userDefinedCpuCode;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionBindToCentralCounter;
        inFields[i++] = actionData.ttiAction.bindToCentralCounter;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionCentralCounterIndex;
        inFields[i++] = actionData.ttiAction.centralCounterIndex;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionVntl2Echo;
        inFields[i++] = actionData.ttiAction.vntl2Echo;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionBridgeBypass;
        inFields[i++] = actionData.ttiAction.bridgeBypass;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionIngressPipeBypass;
        inFields[i++] = actionData.ttiAction.ingressPipeBypass;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionActionStop;
        inFields[i++] = actionData.ttiAction.actionStop;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionHashMaskIndex;
        inFields[i++] = actionData.ttiAction.hashMaskIndex;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionModifyMacSa;
        inFields[i++] = actionData.ttiAction.modifyMacSa;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionModifyMacDa;
        inFields[i++] = actionData.ttiAction.modifyMacDa;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable;
        inFields[i++] = actionData.ttiAction.sourceEPortAssignmentEnable;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort;
        inFields[i++] = actionData.ttiAction.sourceEPort;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionFlowId;
        inFields[i++] = actionData.ttiAction.flowId;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSetMacToMe;
        inFields[i++] = actionData.ttiAction.setMacToMe;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRxProtectionSwitchEnable;
        inFields[i++] = actionData.ttiAction.rxProtectionSwitchEnable;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRxIsProtectionPath;
        inFields[i++] = actionData.ttiAction.rxIsProtectionPath;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionOamTimeStampEnable;
        inFields[i++] = actionData.ttiAction.oamTimeStampEnable;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex;
        inFields[i++] = actionData.ttiAction.oamOffsetIndex;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionOamProcessEnable;
        inFields[i++] = actionData.ttiAction.oamProcessEnable;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionOamProfile;
        inFields[i++] = actionData.ttiAction.oamProfile;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable;
        inFields[i++] = actionData.ttiAction.oamChannelTypeToOpcodeMappingEnable;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionIsPtpPacket;
        inFields[i++] = actionData.ttiAction.isPtpPacket;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionPtpTriggerType;
        inFields[i++] = actionData.ttiAction.ptpTriggerType;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionPtpOffset;
        inFields[i++] = actionData.ttiAction.ptpOffset;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionCwBasedPw;
        inFields[i++] = actionData.ttiAction.cwBasedPw;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTtlExpiryVccvEnable;
        inFields[i++] = actionData.ttiAction.ttlExpiryVccvEnable;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionPwe3FlowLabelExist;
        inFields[i++] = actionData.ttiAction.pwe3FlowLabelExist;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionPwCwBasedETreeEnable;
        inFields[i++] = actionData.ttiAction.pwCwBasedETreeEnable;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionApplyNonDataCwCommand;
        inFields[i++] = actionData.ttiAction.applyNonDataCwCommand;

        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTunnelHeaderStartL4Enable;
        inFields[i++] = actionData.ttiAction.tunnelHeaderStartL4Enable;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookupEnable;
        inFields[i++] = actionData.ttiAction.skipFdbSaLookupEnable;
        inFields[i++]  = expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionIpv6SegmentRoutingEndNodeEnable;
        inFields[i++] = actionData.ttiAction.ipv6SegmentRoutingEndNodeEnable;

        fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                    "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                    "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                    "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                    "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                    "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                    "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                    "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                    "%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                    inFields[0],  inFields[1],   inFields[2],   inFields[3],
                    inFields[4],  inFields[5],   inFields[6],   inFields[7],
                    inFields[8],  inFields[9],   inFields[10],  inFields[11],
                    inFields[12], inFields[13],  inFields[14],  inFields[15],
                    inFields[16], inFields[17],  inFields[18],  inFields[19],
                    inFields[20], inFields[21],  inFields[22],  inFields[23],
                    inFields[24], inFields[25],  inFields[26],  inFields[27],
                    inFields[28], inFields[29],  inFields[30],  inFields[31],
                    inFields[32], inFields[33],  inFields[34],  inFields[35],
                    inFields[36], inFields[37],  inFields[38],  inFields[39],
                    inFields[40], inFields[41],  inFields[42],  inFields[43],
                    inFields[44], inFields[45],  inFields[46],  inFields[47],
                    inFields[48], inFields[49],  inFields[50],  inFields[51],
                    inFields[52], inFields[53],  inFields[54],  inFields[55],
                    inFields[56], inFields[57],  inFields[58],  inFields[59],
                    inFields[60], inFields[61],  inFields[62],  inFields[63],
                    inFields[64], inFields[65],  inFields[66],  inFields[67],
                    inFields[68], inFields[69],  inFields[70],  inFields[71],
                    inFields[72], inFields[73],  inFields[74],  inFields[75],
                    inFields[76], inFields[77],  inFields[78],  inFields[79],
                    inFields[80], inFields[81],  inFields[82],  inFields[83],
                    inFields[84], inFields[85],  inFields[86],  inFields[87],
                    inFields[88], inFields[89],  inFields[90],  inFields[91],
                    inFields[92], inFields[93],  inFields[94],  inFields[95],
                    inFields[96], inFields[97],  inFields[98],  inFields[99],
                    inFields[100], inFields[101],  inFields[102],  inFields[103],
                    inFields[104], inFields[105],  inFields[106],  inFields[107],
                    inFields[108], inFields[109],  inFields[110],  inFields[111],
                    inFields[112], inFields[113],  inFields[114],  inFields[115],
                    inFields[116], inFields[117],  inFields[118],  inFields[119],
                    inFields[120], inFields[121],  inFields[122],  inFields[123],
                    inFields[124], inFields[125],  inFields[126],  inFields[127],
                    inFields[128], inFields[129],  inFields[130],  inFields[131],
                    inFields[132], inFields[133],  inFields[134],  inFields[135],
                    inFields[136], inFields[137],  inFields[138],  inFields[139],
                    inFields[140], inFields[141],  inFields[142],  inFields[143],
                    inFields[144], inFields[145],  inFields[146],  inFields[147],
                    inFields[148], inFields[149],  inFields[150],  inFields[151],
                    inFields[152], inFields[153],  inFields[154],  inFields[155],
                    inFields[156], inFields[157],  inFields[158],  inFields[159],
                    inFields[160], inFields[161],  inFields[162],  inFields[163],
                    inFields[164], inFields[165],  inFields[166],  inFields[167],
                    inFields[168], inFields[169],  inFields[170],  inFields[171],
                    inFields[172], inFields[173]);

        /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_OK, "%d%f", 0);
        break;

    case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
    case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
        fieldOutput(
            "%d%d%d%d%d%d%d%d%d%d%d%d"
            "%d%d%d%d%d%d%d",
            expandedActionIndex,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd,
            actionData.pclAction.pktCmd,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionActionStop,
            actionData.pclAction.actionStop,
            actionData.pclAction.egressPolicy,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode,
            actionData.pclAction.mirror.cpuCode,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMirrorMode,
            actionData.pclAction.mirror.mirrorToRxAnalyzerPort,
            actionData.pclAction.mirror.ingressMirrorToAnalyzerIndex,
            actionData.pclAction.mirror.mirrorToTxAnalyzerPortEn,
            actionData.pclAction.mirror.egressMirrorToAnalyzerIndex,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMirrorMode,
            actionData.pclAction.mirror.egressMirrorToAnalyzerMode,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMatchCounterEnable,
            actionData.pclAction.matchCounter.enableMatchCount,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMatchCounterIndex,
            actionData.pclAction.matchCounter.matchCounterIndex);

        fieldOutputSetAppendMode();

        if (actionData.pclAction.egressPolicy == GT_FALSE)
        {
            fieldOutput(
                "%d%d%d%d%d%d%d%d%d%d%d%d%d"
                "%d%d%d%d%d%d%d%d%d%d%d%d%d",
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionQoSModifyDSCP,
                actionData.pclAction.qos.ingress.modifyDscp,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyUp0,
                actionData.pclAction.qos.ingress.modifyUp,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionQoSProfile,
                actionData.pclAction.qos.ingress.profileIndex,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionQosProfileMakingEnable,
                actionData.pclAction.qos.ingress.profileAssignIndex,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionQosPrecedence,
                actionData.pclAction.qos.ingress.profilePrecedence,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyUp1,
                actionData.pclAction.qos.ingress.up1Cmd,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUp1,
                actionData.pclAction.qos.ingress.up1,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionQoSModifyDSCP,
                0 /*actionData.qos.egress.modifyDscp*/,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionDscpExp,
                0 /*actionData.qos.egress.dscp*/,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyUp0,
                0 /*actionData.qos.egress.modifyUp*/,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUp0,
                0 /*actionData.qos.egress.up*/,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyUp1,
                0 /*actionData.qos.egress.up1ModifyEnable*/,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUp1,
                0 /*actionData.qos.egress.up1*/);
        }
        else
        {
            fieldOutput(
                "%d%d%d%d%d%d%d%d%d%d%d%d%d"
                "%d%d%d%d%d%d%d%d%d%d%d%d%d",
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionQoSModifyDSCP,
                0 /*action.qos.ingress.modifyDscp*/,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyUp0,
                0 /*action.qos.ingress.modifyUp*/,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionQoSProfile,
                0 /*action.qos.ingress.profileIndex*/,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionQosProfileMakingEnable,
                0 /*action.qos.ingress.profileAssignIndex*/,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionQosPrecedence,
                0 /*action.qos.ingress.profilePrecedence*/,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyUp1,
                0 /*action.qos.ingress.up1Cmd*/,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUp1,
                0 /*action.qos.ingress.up1*/,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionQoSModifyDSCP,
                actionData.pclAction.qos.egress.modifyDscp,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionDscpExp,
                actionData.pclAction.qos.egress.dscp,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyUp0,
                actionData.pclAction.qos.egress.modifyUp,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUp0,
                actionData.pclAction.qos.egress.up,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyUp1,
                actionData.pclAction.qos.egress.up1ModifyEnable,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUp1,
                actionData.pclAction.qos.egress.up1);
        }

        fieldOutputSetAppendMode();

        fieldOutput("%d%d",
                    expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand,
                    actionData.pclAction.redirect.redirectCmd);
        fieldOutputSetAppendMode();

        if (actionData.pclAction.redirect.redirectCmd ==
            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E)
        {
            fieldOutput(
                "%d%d%d%d%d%d%d%d%d%d%d%d"
                "%d%d%d%d%d%d%d%d",
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionEgressInterface,
                0, 0, 0, 0, 0,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVntl2Echo,
                0,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionTunnelStart,
                0,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionTunnelIndex,
                0,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType,
                0,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionArpPtr,
                actionData.pclAction.redirect.data.noRedirect.arpPtr,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyMacDA,
                actionData.pclAction.redirect.data.noRedirect.modifyMacDa,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyMacSA,
                0);
        }
        else if (actionData.pclAction.redirect.redirectCmd ==
            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E)
        {
            tempHwDev  = actionData.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum;
            tempPort = actionData.pclAction.redirect.data.outIf.outInterface.devPort.portNum;
            CONVERT_BACK_DEV_PORT_DATA_MAC(tempHwDev, tempPort) ;

            fieldOutput(
                "%d%d%d%d%d%d%d%d%d%d%d%d"
                "%d%d%d%d%d%d%d%d",
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionEgressInterface,
                actionData.pclAction.redirect.data.outIf.outInterface.type,
                tempHwDev, tempPort,
                actionData.pclAction.redirect.data.outIf.outInterface.trunkId,
                actionData.pclAction.redirect.data.outIf.outInterface.vidx,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVntl2Echo,
                actionData.pclAction.redirect.data.outIf.vntL2Echo,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionTunnelStart,
                actionData.pclAction.redirect.data.outIf.tunnelStart,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionTunnelIndex,
                actionData.pclAction.redirect.data.outIf.tunnelPtr,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType,
                actionData.pclAction.redirect.data.outIf.tunnelType,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionArpPtr,
                actionData.pclAction.redirect.data.outIf.arpPtr,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyMacDA,
                actionData.pclAction.redirect.data.outIf.modifyMacDa,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyMacSA,
                actionData.pclAction.redirect.data.outIf.modifyMacSa);
        }
        else if (actionData.pclAction.redirect.redirectCmd ==
            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E)
        {
            fieldOutput(
                "%d%d%d%d%d%d%d%d%d%d%d%d"
                 "%d%d%d%d%d%d%d%d",
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionEgressInterface,
                0,0,0,0,0,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVntl2Echo,
                0,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionTunnelStart,
                0,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionTunnelIndex,
                0,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType,
                0,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionArpPtr,
                actionData.pclAction.redirect.data.modifyMacSa.arpPtr,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyMacDA,
                0,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyMacSA,
                0);
        }
        else
        {
            fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d"
                        "%d%d%d%d%d%d%d%d",
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionEgressInterface,
                0,0,0,0,0,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVntl2Echo,
                0,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionTunnelStart,
                0,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionTunnelIndex,
                0,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType,
                0,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionArpPtr,
                0,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyMacDA,
                0,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyMacSA,
                0);
        }
        fieldOutputSetAppendMode();

        if (actionData.pclAction.redirect.redirectCmd ==
            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E)
        {
            fieldOutput("%d%d",
                        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRouterLttPtr,
                        actionData.pclAction.redirect.data.routerLttIndex);
        }
        else
        {
            fieldOutput("%d%d",
                        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRouterLttPtr,
                        0);
        }
        fieldOutputSetAppendMode();

        if (actionData.pclAction.redirect.redirectCmd ==
            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E)
        {
            fieldOutput("%d%d",
                        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVrfId,
                        actionData.pclAction.redirect.data.vrfId);
        }
        else
        {
            fieldOutput("%d%d",
                        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVrfId,
                        0);
        }
        fieldOutputSetAppendMode();

        if (actionData.pclAction.redirect.redirectCmd ==
            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E)
        {
            workPtr = &(actionData.pclAction.redirect.data.modifyMacSa.macSa.arEther[0]);
            cmdOsSprintf(
                workStr, "%02X%02X%02X%02X%02X%02X",
                workPtr[0], workPtr[1], workPtr[2], workPtr[3], workPtr[4], workPtr[5]);

            fieldOutput("%d"
                        "%s",
                        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMacSa,
                        workStr);
        }
        else
        {
            fieldOutput("%d"
                        "%s",
                        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMacSa,
                        "000000000000");
        }
        fieldOutputSetAppendMode();

        fieldOutput(
            "%d%d"
            "%d%d",
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPolicerEnable,
            actionData.pclAction.policer.policerEnable,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPolicerIndex,
            actionData.pclAction.policer.policerId);
        fieldOutputSetAppendMode();

        if (actionData.pclAction.egressPolicy == GT_FALSE)
        {
            fieldOutput(
                "%d%d%d%d%d%d%d%d"
                "%d%d%d%d%d%d",
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0Command,
                actionData.pclAction.vlan.ingress.modifyVlan,
                0 /*action.vlan.egress.vlanCmd*/,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable,
                actionData.pclAction.vlan.ingress.nestedVlan,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0,
                actionData.pclAction.vlan.ingress.vlanId,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlanPrecedence,
                actionData.pclAction.vlan.ingress.precedence,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan1Command,
                actionData.pclAction.vlan.ingress.vlanId1Cmd,
                0 /*action.vlan.egress.vlanId1ModifyEnable*/,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan1,
                actionData.pclAction.vlan.ingress.vlanId1);
        }
        else
        {
            fieldOutput(
                "%d%d%d%d%d%d%d%d"
                "%d%d%d%d%d%d",
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0Command,
                0 /*action.vlan.ingress.modifyVlan*/,
                actionData.pclAction.vlan.egress.vlanCmd,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable,
                0 /*action.vlan.ingress.nestedVlan*/,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0,
                actionData.pclAction.vlan.egress.vlanId,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlanPrecedence,
                0 /*action.vlan.ingress.precedence*/,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan1Command,
                0 /*action.vlan.ingress.vlanId1Cmd*/,
                actionData.pclAction.vlan.egress.vlanId1ModifyEnable,
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan1,
                actionData.pclAction.vlan.egress.vlanId1);
        }
        fieldOutputSetAppendMode();

        fieldOutput(
            "%d%d"
            "%d%d",
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourceIdEnable,
            actionData.pclAction.sourceId.assignSourceId,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourceId,
            actionData.pclAction.sourceId.sourceIdValue);
        fieldOutputSetAppendMode();

        fieldOutput(
            "%d%d%d%d%d%d"
            "%d%d%d%d%d%d",
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionBypassBridge,
            actionData.pclAction.bypassBridge,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionBypassIngressPipe,
            actionData.pclAction.bypassIngressPipe,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionIPCLConfigurationIndex,
            actionData.pclAction.lookupConfig.ipclConfigIndex,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPCL1OverrideConfigIndex,
            actionData.pclAction.lookupConfig.pcl0_1OverrideConfigIndex,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPCL2OverrideConfigIndex,
            actionData.pclAction.lookupConfig.pcl1OverrideConfigIndex,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMirrorTcpRstAndFinPacketsToCpu,
            actionData.pclAction.mirror.mirrorTcpRstAndFinPacketsToCpu);
        fieldOutputSetAppendMode();

       fieldOutput(
            "%d%d%d%d"
            "%d%d%d%d",
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionOamTimestampEnable,
            actionData.pclAction.oam.timeStampEnable,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex,
            actionData.pclAction.oam.offsetIndex,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionOamProcessingEnable,
            actionData.pclAction.oam.oamProcessEnable,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionOamProfile,
            actionData.pclAction.oam.oamProfile);
        fieldOutputSetAppendMode();

        fieldOutput(
            "%d%d%d%d%d%d"
            "%d%d%d%d%d",
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionFlowId,
            actionData.pclAction.flowId,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSetMacToMe,
            actionData.pclAction.setMacToMe,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourcePortEnable,
            actionData.pclAction.sourcePort.assignSourcePortEnable,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourcePort,
            actionData.pclAction.sourcePort.sourcePortValue,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable,
            actionData.pclAction.channelTypeToOpcodeMapEnable,
            actionData.pclAction.exactMatchOverPclEn);
        fieldOutputSetAppendMode();

        /* falcon fields */
        fieldOutput(
            "%d%d%d%d%d"
            "%d%d%d%d%d",
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId,
            actionData.pclAction.terminateCutThroughMode,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitorEnable,
            actionData.pclAction.latencyMonitor.monitoringEnable,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitor,
            actionData.pclAction.latencyMonitor.latencyProfile,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup,
            actionData.pclAction.skipFdbSaLookup,
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionTriggerInterrupt,
            actionData.pclAction.triggerInterrupt);

         /* TBD pha fields should be updated after implementation will be added
           "%d%d%d%d%d"
           "%d%d%d%d%d"
           "%d%d%d%d"
           "%d%d%d%d",

         expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPhaMetadataAssignEnable,
         defaultActionData.epclPhaInfo.phaThreadType,
         expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPhaMetadata,
         defaultActionData.epclPhaInfo.phaThreadUnion,
         expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPhaThreadNumberAssignEnable,
         defaultActionData.epclPhaInfo.phaThreadIdAssignmentMode,
         expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPhaThreadNumber,
         defaultActionData.epclPhaInfo.phaThreadId);*/


         /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_OK, "%d%f", 1);
        break;
        break;
    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong Exact Match Expanded FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

    expandedActionDbIndexGet++;

    return CMD_OK;
}

/**
* @internal wrCpssDxChExactMatchExpandedActionGetFirst function
* @endinternal
*/
static CMD_STATUS wrCpssDxChExactMatchExpandedActionGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* in case it is first usage of expanded action database, initialize it */
    if (firstRunExpanded)
    {
        expandedActionDatabaseInit();
        firstRunExpanded = GT_FALSE;
    }
    expandedActionDbIndexGet = 1;

    return wrCpssDxChExactMatchExpandedActionGetNext(inArgs,inFields,numFields,outArgs);
}



/* cpssDxChExactMatch Table */

/**
* @internal wrCpssDxChExactMatchTtiEntrySet function
* @endinternal
*
* @brief   Write the new entry in Hardware TTI Format in
*          specified index.
*
* @note   APPLICABLE DEVICES:      Falcon devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong params
* @retval GT_OUT_OF_RANGE          - index out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChExactMatchTtiEntrySet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                result;

    GT_U8                                    devNum;
    GT_U32                                   i,k;
    GT_U32                                   exactMatchIndex;
    GT_U32                                   expandedActionIndex;
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC          exactMatchEntry;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT         exactMatchAction;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&exactMatchEntry, 0, sizeof(exactMatchEntry));
    cpssOsMemSet(&exactMatchAction, 0, sizeof(exactMatchAction));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    actionType  = (CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT)inArgs[3];

    i=0;
    exactMatchIndex = (GT_U32)inFields[i++];
    expandedActionIndex = (GT_U32)inFields[i++];
    exactMatchEntry.lookupNum = (CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)inFields[i++];
    exactMatchEntry.key.keySize = (CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT)inFields[i++];
    for (k=0; k<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS; k++)
    {
        exactMatchEntry.key.pattern[k]=(GT_U8)inFields[i++];
    }

    exactMatchAction.ttiAction.tunnelTerminate                 = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.ttPassengerPacketType           = (CPSS_DXCH_TTI_PASSENGER_TYPE_ENT)inFields[i++];
    exactMatchAction.ttiAction.tsPassengerPacketType           = (CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT)inFields[i++];

    exactMatchAction.ttiAction.ttHeaderLength                  = (GT_U32)inFields[i++];
    exactMatchAction.ttiAction.continueToNextTtiLookup         = (GT_BOOL)inFields[i++];

    exactMatchAction.ttiAction.copyTtlExpFromTunnelHeader      = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.mplsCommand                     = (CPSS_DXCH_TTI_MPLS_CMD_ENT)inFields[i++];
    exactMatchAction.ttiAction.mplsTtl                         = (GT_U32)inFields[i++];
    exactMatchAction.ttiAction.enableDecrementTtl              = (GT_BOOL)inFields[i++];

    exactMatchAction.ttiAction.passengerParsingOfTransitMplsTunnelMode = (CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_TYPE_ENT)inFields[i++];
    exactMatchAction.ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable = (GT_BOOL)inFields[i++];

    exactMatchAction.ttiAction.command                         = (CPSS_PACKET_CMD_ENT)inFields[i++];
    exactMatchAction.ttiAction.redirectCommand                 = (CPSS_DXCH_TTI_REDIRECT_COMMAND_ENT)inFields[i++];
    exactMatchAction.ttiAction.egressInterface.type            = (CPSS_INTERFACE_TYPE_ENT)inFields[i++];
    exactMatchAction.ttiAction.egressInterface.devPort.hwDevNum  = (GT_HW_DEV_NUM)inFields[i++];
    exactMatchAction.ttiAction.egressInterface.devPort.portNum = (GT_PORT_NUM)inFields[i++];
    /* Override Device and Port */
    CONVERT_DEV_PORT_DATA_MAC(exactMatchAction.ttiAction.egressInterface.devPort.hwDevNum, exactMatchAction.ttiAction.egressInterface.devPort.portNum);
    exactMatchAction.ttiAction.egressInterface.trunkId         = (GT_TRUNK_ID)inFields[i++];
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(exactMatchAction.ttiAction.egressInterface.trunkId);
    exactMatchAction.ttiAction.egressInterface.vidx            = (GT_U16)inFields[i++];
    exactMatchAction.ttiAction.egressInterface.vlanId          = (GT_U16)inFields[i++];
    exactMatchAction.ttiAction.arpPtr                          = (GT_U32)inFields[i++];
    exactMatchAction.ttiAction.tunnelStart                     = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.tunnelStartPtr                  = (GT_U32)inFields[i++];
    exactMatchAction.ttiAction.routerLttPtr                    = (GT_U32)inFields[i++];
    exactMatchAction.ttiAction.vrfId                           = (GT_U32)inFields[i++];
    exactMatchAction.ttiAction.sourceIdSetEnable               = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.sourceId                        = (GT_U32)inFields[i++];
    exactMatchAction.ttiAction.tag0VlanCmd                     = (CPSS_DXCH_TTI_VLAN_COMMAND_ENT)inFields[i++];
    exactMatchAction.ttiAction.tag0VlanId                      = (GT_U16)inFields[i++];
    exactMatchAction.ttiAction.tag1VlanCmd                     = (CPSS_DXCH_TTI_VLAN_COMMAND_ENT)inFields[i++];
    exactMatchAction.ttiAction.tag1VlanId                      = (GT_U16)inFields[i++];
    exactMatchAction.ttiAction.tag0VlanPrecedence              = (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[i++];
    exactMatchAction.ttiAction.nestedVlanEnable                = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.bindToPolicerMeter              = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.bindToPolicer                   = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.policerIndex                    = (GT_U32)inFields[i++];
    exactMatchAction.ttiAction.qosPrecedence                   = (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[i++];
    exactMatchAction.ttiAction.keepPreviousQoS                 = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.trustUp                         = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.trustDscp                       = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.trustExp                        = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.qosProfile                      = (GT_U32)inFields[i++];
    exactMatchAction.ttiAction.modifyTag0Up                    = (CPSS_DXCH_TTI_MODIFY_UP_ENT)inFields[i++];
    exactMatchAction.ttiAction.tag1UpCommand                   = (CPSS_DXCH_TTI_TAG1_UP_COMMAND_ENT)inFields[i++];
    exactMatchAction.ttiAction.modifyDscp                      = (CPSS_DXCH_TTI_MODIFY_DSCP_ENT)inFields[i++];
    exactMatchAction.ttiAction.tag0Up                          = (GT_U32)inFields[i++];
    exactMatchAction.ttiAction.tag1Up                          = (GT_U32)inFields[i++];
    exactMatchAction.ttiAction.remapDSCP                       = (GT_BOOL)inFields[i++];

    exactMatchAction.ttiAction.qosUseUpAsIndexEnable           = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.qosMappingTableIndex            = (GT_U32)inFields[i++];
    exactMatchAction.ttiAction.mplsLLspQoSProfileEnable        = (GT_BOOL)inFields[i++];

    exactMatchAction.ttiAction.pcl0OverrideConfigIndex         = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[i++];
    exactMatchAction.ttiAction.pcl0_1OverrideConfigIndex       = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[i++];
    exactMatchAction.ttiAction.pcl1OverrideConfigIndex         = (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[i++];
    exactMatchAction.ttiAction.iPclConfigIndex                 = (GT_U32)inFields[i++];

    exactMatchAction.ttiAction.iPclUdbConfigTableEnable        = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.iPclUdbConfigTableIndex         = (CPSS_DXCH_PCL_PACKET_TYPE_ENT)inFields[i++];

    exactMatchAction.ttiAction.mirrorToIngressAnalyzerEnable   = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.mirrorToIngressAnalyzerIndex    = (GT_U32)inFields[i++];

    exactMatchAction.ttiAction.userDefinedCpuCode              = (CPSS_NET_RX_CPU_CODE_ENT)inFields[i++];
    exactMatchAction.ttiAction.bindToCentralCounter            = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.centralCounterIndex             = (GT_U32)inFields[i++];
    exactMatchAction.ttiAction.vntl2Echo                       = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.bridgeBypass                    = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.ingressPipeBypass               = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.actionStop                      = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.hashMaskIndex                   = (GT_U32)inFields[i++];
    exactMatchAction.ttiAction.modifyMacSa                     = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.modifyMacDa                     = (GT_BOOL)inFields[i++];

    exactMatchAction.ttiAction.sourceEPortAssignmentEnable     = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.sourceEPort                     = (GT_PORT_NUM)inFields[i++];
    exactMatchAction.ttiAction.flowId                          = (GT_U32)inFields[i++];
    exactMatchAction.ttiAction.setMacToMe                      = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.rxProtectionSwitchEnable        = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.rxIsProtectionPath              = (GT_BOOL)inFields[i++];

    exactMatchAction.ttiAction.oamTimeStampEnable              = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.oamOffsetIndex                  = (GT_U32)inFields[i++];
    exactMatchAction.ttiAction.oamProcessEnable                = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.oamProfile                      = (GT_U32)inFields[i++];
    exactMatchAction.ttiAction.oamChannelTypeToOpcodeMappingEnable = (GT_BOOL)inFields[i++];

    exactMatchAction.ttiAction.isPtpPacket                     = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.ptpTriggerType                  = (CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_ENT)inFields[i++];
    exactMatchAction.ttiAction.ptpOffset                       = (GT_U32)inFields[i++];

    exactMatchAction.ttiAction.cwBasedPw                       = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.ttlExpiryVccvEnable             = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.pwe3FlowLabelExist              = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.pwCwBasedETreeEnable            = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.applyNonDataCwCommand           = (GT_BOOL)inFields[i++];

    exactMatchAction.ttiAction.tunnelHeaderStartL4Enable       = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.skipFdbSaLookupEnable           = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.ipv6SegmentRoutingEndNodeEnable = (GT_BOOL)inFields[i++];
    exactMatchAction.ttiAction.exactMatchOverTtiEn             = (GT_BOOL)inFields[i++];

    /* call cpss api function */
    exactMatchMultiPortGroupsBmpGet(devNum,&multiPortGroupsBmpEnable, &multiPortGroupsBmp);

    if(multiPortGroupsBmpEnable == GT_FALSE)
    {
        result = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,exactMatchIndex,expandedActionIndex,
                                                   &exactMatchEntry,actionType,&exactMatchAction);
    }
    else
    {
        result = cpssDxChExactMatchPortGroupEntrySet(devNum,multiPortGroupsBmp,exactMatchIndex,expandedActionIndex,
                                                   &exactMatchEntry,actionType,&exactMatchAction);
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChExactMatchPclEntrySet function
* @endinternal
*
* @brief   Write the new entry in Hardware PCL Format in
*          specified index.
*
* @note   APPLICABLE DEVICES:      Falcon devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong params
* @retval GT_OUT_OF_RANGE          - index out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChExactMatchPclEntrySet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                result;

    GT_U8                                    devNum;
    GT_U32                                   i,k;
    GT_U32                                   exactMatchIndex;
    GT_U32                                   expandedActionIndex;
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC          exactMatchEntry;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT         exactMatchAction;
    GT_U32                                   saveArpPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&exactMatchEntry, 0, sizeof(exactMatchEntry));
    cpssOsMemSet(&exactMatchAction, 0, sizeof(exactMatchAction));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    actionType  = (CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT)inArgs[3];

    i=0;
    exactMatchIndex = (GT_U32)inFields[i++];
    expandedActionIndex = (GT_U32)inFields[i++];
    exactMatchEntry.lookupNum = (CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)inFields[i++];
    exactMatchEntry.key.keySize = (CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT)inFields[i++];
    for (k=0; k<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS; k++)
    {
        exactMatchEntry.key.pattern[k]=(GT_U8)inFields[i++];
    }

    exactMatchAction.pclAction.pktCmd = (CPSS_PACKET_CMD_ENT)inFields[i++];
    exactMatchAction.pclAction.actionStop    =(GT_BOOL)inFields[i++];
    exactMatchAction.pclAction.egressPolicy  = (GT_BOOL)inFields[i++];
    exactMatchAction.pclAction.mirror.cpuCode=(CPSS_NET_RX_CPU_CODE_ENT)inFields[i++];
    exactMatchAction.pclAction.mirror.mirrorToRxAnalyzerPort               =(GT_BOOL)inFields[i++];
    exactMatchAction.pclAction.mirror.ingressMirrorToAnalyzerIndex         =(GT_U32)inFields[i++];
    exactMatchAction.pclAction.mirror.mirrorToTxAnalyzerPortEn             = (GT_U32)inFields[i++];
    exactMatchAction.pclAction.mirror.egressMirrorToAnalyzerIndex          = (GT_U32)inFields[i++];
    exactMatchAction.pclAction.mirror.egressMirrorToAnalyzerMode           = (CPSS_DXCH_MIRROR_EGRESS_MODE_ENT)inFields[i++];
    exactMatchAction.pclAction.matchCounter.enableMatchCount=(GT_BOOL)inFields[i++];
    exactMatchAction.pclAction.matchCounter.matchCounterIndex=(GT_U32)inFields[i++];

    switch (exactMatchAction.pclAction.egressPolicy)
    {
        case GT_FALSE: /* ingress */
            exactMatchAction.pclAction.qos.ingress.modifyDscp =
                (CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[i];

            exactMatchAction.pclAction.qos.ingress.modifyUp =
                (CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[i+1];

            exactMatchAction.pclAction.qos.ingress.profileIndex = (GT_U32)inFields[i+2];
            exactMatchAction.pclAction.qos.ingress.profileAssignIndex = (GT_BOOL)inFields[i+3];
            exactMatchAction.pclAction.qos.ingress.profilePrecedence =
                (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[i+4];
            exactMatchAction.pclAction.qos.ingress.up1Cmd =
                (CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ENT)inFields[i+5];
            exactMatchAction.pclAction.qos.ingress.up1 = (GT_U32)inFields[i+6];
            break;

        default:
        case GT_TRUE: /* egress */
            exactMatchAction.pclAction.qos.egress.modifyDscp =
                (CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_ENT)(inFields[i+7]);
            exactMatchAction.pclAction.qos.egress.dscp = (GT_U32)inFields[i+8];
            exactMatchAction.pclAction.qos.egress.modifyUp =
                (CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT)(inFields[i+9]);
            exactMatchAction.pclAction.qos.egress.up   = (GT_U32)inFields[i+10];
            exactMatchAction.pclAction.qos.egress.up1ModifyEnable   = (GT_BOOL)inFields[i+11];
            exactMatchAction.pclAction.qos.egress.up1   = (GT_U32)inFields[i+12];
            break;
    }

    /* bypass the switch */
    i += 13;

    exactMatchAction.pclAction.redirect.redirectCmd =
        (CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ENT)inFields[i++];

    if (exactMatchAction.pclAction.redirect.redirectCmd ==
        CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E)
    {
        /* used the same GUI fields as for redirect to outIf */
        exactMatchAction.pclAction.redirect.data.noRedirect.arpPtr = (GT_U32)inFields[i+9];
        exactMatchAction.pclAction.redirect.data.noRedirect.modifyMacDa = (GT_BOOL)inFields[i+10];
    }

    if (exactMatchAction.pclAction.redirect.redirectCmd ==
        CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E)
    {
        exactMatchAction.pclAction.redirect.data.outIf.outInterface.type =
            (CPSS_INTERFACE_TYPE_ENT)inFields[i];

        switch (exactMatchAction.pclAction.redirect.data.outIf.outInterface.type)
        {
            case CPSS_INTERFACE_PORT_E:
                exactMatchAction.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum =
                    (GT_HW_DEV_NUM)inFields[i+1];
                exactMatchAction.pclAction.redirect.data.outIf.outInterface.devPort.portNum =
                    (GT_PORT_NUM)inFields[i+2];
                CONVERT_DEV_PORT_DATA_MAC(
                    exactMatchAction.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum,
                    exactMatchAction.pclAction.redirect.data.outIf.outInterface.devPort.portNum);
                break;

            case CPSS_INTERFACE_TRUNK_E:
                exactMatchAction.pclAction.redirect.data.outIf.outInterface.trunkId =
                    (GT_TRUNK_ID)inFields[i+3];
                CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(
                   exactMatchAction.pclAction.redirect.data.outIf.outInterface.trunkId);
                break;

            case CPSS_INTERFACE_VIDX_E:
                exactMatchAction.pclAction.redirect.data.outIf.outInterface.vidx =
                    (GT_U16)inFields[i+4];
                break;

            default:
                break;
        }
        exactMatchAction.pclAction.redirect.data.outIf.vntL2Echo   = (GT_BOOL)inFields[i+5];
        exactMatchAction.pclAction.redirect.data.outIf.tunnelStart = (GT_BOOL)inFields[i+6];
        exactMatchAction.pclAction.redirect.data.outIf.tunnelPtr   = (GT_U32)inFields[i+7];
        exactMatchAction.pclAction.redirect.data.outIf.tunnelType  =
            (CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ENT)inFields[i+8];
        exactMatchAction.pclAction.redirect.data.outIf.arpPtr = (GT_U32)inFields[i+9];
        exactMatchAction.pclAction.redirect.data.outIf.modifyMacDa = (GT_BOOL)inFields[i+10];
        exactMatchAction.pclAction.redirect.data.outIf.modifyMacSa = (GT_BOOL)inFields[i+11];
    }
    /* bypass redirect to IF */
    /* arpPtr relevant also to ...REDIRECT_CMD_REPLACE_MAC_SA_E */
    saveArpPtr = (GT_U32)inFields[i+9];
    i += 12;

    if (exactMatchAction.pclAction.redirect.redirectCmd ==
        CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E)
    {
        exactMatchAction.pclAction.redirect.data.routerLttIndex = (GT_U32)inFields[i];
    }
    /* bypass redirect to Next Hop */
    i ++;


    if (exactMatchAction.pclAction.redirect.redirectCmd ==
        CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E)
    {
        exactMatchAction.pclAction.redirect.data.vrfId = (GT_U32)inFields[i];
    }
    /* bypass redirect to Virtual Router */
    i ++;

    if (exactMatchAction.pclAction.redirect.redirectCmd ==
        CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E)
    {
        galtisMacAddr(&(exactMatchAction.pclAction.redirect.data.modifyMacSa.macSa), (GT_U8*)inFields[i]);
        /* arpPtr saved from ...REDIRECT_CMD_OUT_IF_E parameters */
        exactMatchAction.pclAction.redirect.data.modifyMacSa.arpPtr = saveArpPtr;
    }
    /* bypass replace MAC_SA */
    i ++;

    exactMatchAction.pclAction.policer.policerEnable = (CPSS_DXCH_PCL_POLICER_ENABLE_ENT)inFields[i++];
    exactMatchAction.pclAction.policer.policerId = (GT_U32)inFields[i++];

    switch (exactMatchAction.pclAction.egressPolicy)
    {
        case GT_FALSE:
            exactMatchAction.pclAction.vlan.ingress.modifyVlan =
                (CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT)inFields[i];
            exactMatchAction.pclAction.vlan.ingress.nestedVlan = (GT_BOOL)inFields[i+2];
            exactMatchAction.pclAction.vlan.ingress.vlanId = (GT_U32)inFields[i+3];
            exactMatchAction.pclAction.vlan.ingress.precedence =
                (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[i+4];
            exactMatchAction.pclAction.vlan.ingress.vlanId1Cmd =
                (CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ENT)inFields[i+5];
            exactMatchAction.pclAction.vlan.ingress.vlanId1 = (GT_U32)inFields[i+7];
            break;

        case GT_TRUE:
            exactMatchAction.pclAction.vlan.egress.vlanCmd =
                (CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT)inFields[i+1];
            exactMatchAction.pclAction.vlan.egress.vlanId = (GT_U32)inFields[i+3];
            exactMatchAction.pclAction.vlan.egress.vlanId1ModifyEnable = (GT_BOOL)inFields[i+6];
            exactMatchAction.pclAction.vlan.egress.vlanId1 = (GT_U32)inFields[i+7];
            break;

        default:
            break;
    }
    /* bypass VLAN */
    i += 8;

    exactMatchAction.pclAction.sourceId.assignSourceId = (GT_BOOL)inFields[i++];
    exactMatchAction.pclAction.sourceId.sourceIdValue  = (GT_U32)inFields[i++];

    exactMatchAction.pclAction.bypassBridge                           = (GT_BOOL)inFields[i++];
    exactMatchAction.pclAction.bypassIngressPipe                      = (GT_BOOL)inFields[i++];

    exactMatchAction.pclAction.lookupConfig.ipclConfigIndex           = (GT_U32)inFields[i++];
    exactMatchAction.pclAction.lookupConfig.pcl0_1OverrideConfigIndex =
        (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[i++];
    exactMatchAction.pclAction.lookupConfig.pcl1OverrideConfigIndex   =
        (CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT)inFields[i++];
    exactMatchAction.pclAction.mirror.mirrorTcpRstAndFinPacketsToCpu  = (GT_BOOL)inFields[i++];

    exactMatchAction.pclAction.oam.timeStampEnable  = (GT_BOOL)inFields[i++];
    exactMatchAction.pclAction.oam.offsetIndex      = (GT_U32)inFields[i++];
    exactMatchAction.pclAction.oam.oamProcessEnable = (GT_BOOL)inFields[i++];
    exactMatchAction.pclAction.oam.oamProfile       = (GT_U32)inFields[i++];

    exactMatchAction.pclAction.flowId       = (GT_U32)inFields[i++];
    exactMatchAction.pclAction.setMacToMe   = (GT_BOOL)inFields[i++];

    exactMatchAction.pclAction.sourcePort.assignSourcePortEnable   = (GT_BOOL)inFields[i++];
    exactMatchAction.pclAction.sourcePort.sourcePortValue          = (GT_U32)inFields[i++];

    exactMatchAction.pclAction.channelTypeToOpcodeMapEnable   = (GT_BOOL)inFields[i++];
    exactMatchAction.pclAction.exactMatchOverPclEn            = (GT_BOOL)inFields[i++];

    /* falcon fields */
    exactMatchAction.pclAction.terminateCutThroughMode          = (GT_BOOL)inFields[i++];
    exactMatchAction.pclAction.latencyMonitor.monitoringEnable  = (GT_BOOL)inFields[i++];
    exactMatchAction.pclAction.latencyMonitor.latencyProfile    = (GT_U32)inFields[i++];
    exactMatchAction.pclAction.skipFdbSaLookup                  = (GT_BOOL)inFields[i++];

    exactMatchAction.pclAction.triggerInterrupt                 = (GT_BOOL)inFields[i++];

    /* TBD pha fields should be updated after implementation will be added
    exactMatchAction.epclPhaInfo.phaThreadType   = (GT_BOOL)inFields[i++];
    exactMatchAction.epclPhaInfo.phaThreadUnion   = (GT_BOOL)inFields[i++];
    exactMatchAction.epclPhaInfo.phaThreadIdAssignmentMode   = (GT_BOOL)inFields[i++];
    exactMatchAction.epclPhaInfo.phaThreadId   = (GT_BOOL)inFields[i++];*/

        /* call cpss api function */
    exactMatchMultiPortGroupsBmpGet(devNum,&multiPortGroupsBmpEnable, &multiPortGroupsBmp);

    if(multiPortGroupsBmpEnable == GT_FALSE)
    {
        result = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,exactMatchIndex,expandedActionIndex,
                                                   &exactMatchEntry,actionType,&exactMatchAction);
    }
    else
    {
        result = cpssDxChExactMatchPortGroupEntrySet(devNum,multiPortGroupsBmp,exactMatchIndex,expandedActionIndex,
                                                   &exactMatchEntry,actionType,&exactMatchAction);
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChExactMatchEntrySet function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChExactMatchEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    switch (inArgs[3]) /* Exact Match entry type tab */
    {
    case 0: /* TTI */
        return wrCpssDxChExactMatchTtiEntrySet(inArgs,inFields,numFields,outArgs);

    case 1:/* PCL */
        return wrCpssDxChExactMatchPclEntrySet(inArgs,inFields,numFields,outArgs);

    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong Exact Match ENTRY FORMAT.\n");
        return CMD_AGENT_ERROR;
    }
}

/**
* @internal wrCpssDxChExactMatchEntryEndSet function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChExactMatchEntryEndSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChExactMatchEntryCancelSet function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChExactMatchEntryCancelSet

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


static GT_STATUS wrPrvCpssDxChExactMatchEntryRead
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   index,
    OUT GT_BOOL                                 *validPtr,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT   *actionTypePtr,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT        *actionPtr,
    OUT CPSS_DXCH_EXACT_MATCH_ENTRY_STC         *entryPtr,
    OUT GT_U32                                  *expandedActionIndexPtr
)
{
    /*  a call to exactMatchMultiPortGroupsBmpGet(devNum,&multiPortGroupsBmpEnable, &multiPortGroupsBmp);
    is done on the 'getFirst' entry */

    if(useRefreshExactMatchPerPortGroupId == GT_TRUE)
    {
        return cpssDxChExactMatchPortGroupEntryGet(devNum,
            (1 << refreshExactMatchCurrentPortGroupId),/* bmp of explicit port group */
            index,validPtr, actionTypePtr, actionPtr,entryPtr,expandedActionIndexPtr);
    }
    return cpssDxChExactMatchPortGroupEntryGet(devNum,multiPortGroupsBmp,index,
                                               validPtr,actionTypePtr,actionPtr,entryPtr,expandedActionIndexPtr);
}

/**
* @internal internalDxChExactMatchEntryReadNext function
* @endinternal
*
* @brief  Reads the new entry in Hardware Exact Match table from
*         specified index. This action do direct read access to
*         RAM .
*
* @note   APPLICABLE DEVICES:      Falcom devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - index out of range
*/
static CMD_STATUS internalDxChExactMatchEntryReadNext
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;

    GT_U8                                   devNum;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT   actionType;
    GT_BOOL                                 valid;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT        exactMatchAction;
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC         exactMatchEntry;
    GT_U32                                  expandedActionIndex;

    GT_U32                                  portGroupId;/* port group Id iterator */
    GT_U32                                  i,k;
    GT_HW_DEV_NUM                           tempHwDev;  /* used for port,dev converting */
    GT_PORT_NUM                             tempPort;   /* used for port,dev converting */



    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    do{
        if(exactMatchEntryIndex > exactMatchEntryIndexMaxGet)
        {
            /* the result is ok, this is the last entry wanted */
            result = GT_OK;

            galtisOutput(outArgs, result, "%d", -1);
            return CMD_OK;
        }

        /* call cpss api function */
        result = wrPrvCpssDxChExactMatchEntryRead(devNum, exactMatchEntryIndex, &valid,&actionType,
                                                  &exactMatchAction, &exactMatchEntry,&expandedActionIndex);

        if (result != GT_OK)
        {
             /* check end of table return code */
             if(GT_OUT_OF_RANGE == result)
             {
                 /* the result is ok, this is end of table */
                 result = GT_OK;
             }

             galtisOutput(outArgs, result, "%d", -1);
             return CMD_OK;
        }

        if(useRefreshExactMatchPerPortGroupId == GT_TRUE)
        {
            /* find first port group in the bmp */

            /* access to read from next port group */
            PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,multiPortGroupsBmp,portGroupId)
            {
                if(portGroupId <= refreshExactMatchCurrentPortGroupId)
                {
                    /* skip port groups we already been to */
                    continue;
                }

                refreshExactMatchCurrentPortGroupId = portGroupId;
                break;
            }
            PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,multiPortGroupsBmp,portGroupId)

            if(portGroupId > refreshExactMatchCurrentPortGroupId)
            {
                /* indication that we finished loop on all port groups */
                exactMatchEntryIndex++;

                /* state that the next read come from first port group again */
                refreshExactMatchCurrentPortGroupId = refreshExactMatchFirstPortGroupId;
            }
        }
        else
        {
            exactMatchEntryIndex++;
        }
    }while (!valid);

    i=0;

    if(useRefreshExactMatchPerPortGroupId == GT_TRUE &&
       refreshExactMatchCurrentPortGroupId != refreshExactMatchFirstPortGroupId)
    {
        /* index was not incremented */
        inFields[i++] = exactMatchEntryIndex;
    }
    else
    {
        inFields[i++] = exactMatchEntryIndex - 1;
    }

    inFields[i++] = expandedActionIndex;
    inFields[i++] = exactMatchEntry.lookupNum;
    inFields[i++] = exactMatchEntry.key.keySize;
    for (k=0; k<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS; k++)
    {
        inFields[i++]=exactMatchEntry.key.pattern[k];
    }

    switch (actionType) /* entry type tab */
    {
        case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:

            inFields[i++]  = exactMatchAction.ttiAction.tunnelTerminate;
            inFields[i++]  = exactMatchAction.ttiAction.ttPassengerPacketType;
            inFields[i++]  = exactMatchAction.ttiAction.tsPassengerPacketType;
            inFields[i++]  = exactMatchAction.ttiAction.ttHeaderLength;
            inFields[i++]  = exactMatchAction.ttiAction.continueToNextTtiLookup;
            inFields[i++]  = exactMatchAction.ttiAction.copyTtlExpFromTunnelHeader;
            inFields[i++]  = exactMatchAction.ttiAction.mplsCommand;
            inFields[i++]  = exactMatchAction.ttiAction.mplsTtl;
            inFields[i++]  = exactMatchAction.ttiAction.enableDecrementTtl;
            inFields[i++]  = exactMatchAction.ttiAction.passengerParsingOfTransitMplsTunnelMode;
            inFields[i++]  = exactMatchAction.ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable;
            inFields[i++]  = exactMatchAction.ttiAction.command;
            inFields[i++] = exactMatchAction.ttiAction.redirectCommand;
            inFields[i++] = exactMatchAction.ttiAction.egressInterface.type;
            tempHwDev      = exactMatchAction.ttiAction.egressInterface.devPort.hwDevNum;
            tempPort     = exactMatchAction.ttiAction.egressInterface.devPort.portNum;
            CONVERT_BACK_DEV_PORT_DATA_MAC(tempHwDev,tempPort);
            inFields[i++] = tempHwDev;
            inFields[i++] = tempPort;
            CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(exactMatchAction.ttiAction.egressInterface.trunkId);

            inFields[i++] = exactMatchAction.ttiAction.egressInterface.trunkId;
            inFields[i++] = exactMatchAction.ttiAction.egressInterface.vidx;
            inFields[i++] = exactMatchAction.ttiAction.egressInterface.vlanId;
            inFields[i++] = exactMatchAction.ttiAction.arpPtr;
            inFields[i++] = exactMatchAction.ttiAction.tunnelStart;
            inFields[i++] = exactMatchAction.ttiAction.tunnelStartPtr;
            inFields[i++] = exactMatchAction.ttiAction.routerLttPtr;
            inFields[i++] = exactMatchAction.ttiAction.vrfId;
            inFields[i++] = exactMatchAction.ttiAction.sourceIdSetEnable;
            inFields[i++] = exactMatchAction.ttiAction.sourceId;
            inFields[i++] = exactMatchAction.ttiAction.tag0VlanCmd;
            inFields[i++] = exactMatchAction.ttiAction.tag0VlanId;
            inFields[i++] = exactMatchAction.ttiAction.tag1VlanCmd;
            inFields[i++] = exactMatchAction.ttiAction.tag1VlanId;
            inFields[i++] = exactMatchAction.ttiAction.tag0VlanPrecedence;
            inFields[i++] = exactMatchAction.ttiAction.nestedVlanEnable;
            inFields[i++] = exactMatchAction.ttiAction.bindToPolicerMeter;
            inFields[i++] = exactMatchAction.ttiAction.bindToPolicer;
            inFields[i++] = exactMatchAction.ttiAction.policerIndex;
            inFields[i++] = exactMatchAction.ttiAction.qosPrecedence;
            inFields[i++] = exactMatchAction.ttiAction.keepPreviousQoS;
            inFields[i++] = exactMatchAction.ttiAction.trustUp;
            inFields[i++] = exactMatchAction.ttiAction.trustDscp;
            inFields[i++] = exactMatchAction.ttiAction.trustExp;
            inFields[i++] = exactMatchAction.ttiAction.qosProfile;
            inFields[i++] = exactMatchAction.ttiAction.modifyTag0Up;
            inFields[i++] = exactMatchAction.ttiAction.tag1UpCommand;
            inFields[i++] = exactMatchAction.ttiAction.modifyDscp;
            inFields[i++] = exactMatchAction.ttiAction.tag0Up;
            inFields[i++] = exactMatchAction.ttiAction.tag1Up;
            inFields[i++] = exactMatchAction.ttiAction.remapDSCP;
            inFields[i++] = exactMatchAction.ttiAction.qosUseUpAsIndexEnable;
            inFields[i++] = exactMatchAction.ttiAction.qosMappingTableIndex;
            inFields[i++] = exactMatchAction.ttiAction.mplsLLspQoSProfileEnable;
            inFields[i++] = exactMatchAction.ttiAction.pcl0OverrideConfigIndex;
            inFields[i++] = exactMatchAction.ttiAction.pcl0_1OverrideConfigIndex;
            inFields[i++] = exactMatchAction.ttiAction.pcl1OverrideConfigIndex;
            inFields[i++] = exactMatchAction.ttiAction.iPclConfigIndex;
            inFields[i++] = exactMatchAction.ttiAction.iPclUdbConfigTableEnable;
            inFields[i++] = exactMatchAction.ttiAction.iPclUdbConfigTableIndex;
            inFields[i++] = exactMatchAction.ttiAction.mirrorToIngressAnalyzerEnable;
            inFields[i++] = exactMatchAction.ttiAction.mirrorToIngressAnalyzerIndex;
            inFields[i++] = exactMatchAction.ttiAction.userDefinedCpuCode;
            inFields[i++] = exactMatchAction.ttiAction.bindToCentralCounter;
            inFields[i++] = exactMatchAction.ttiAction.centralCounterIndex;
            inFields[i++] = exactMatchAction.ttiAction.vntl2Echo;
            inFields[i++] = exactMatchAction.ttiAction.bridgeBypass;
            inFields[i++] = exactMatchAction.ttiAction.ingressPipeBypass;
            inFields[i++] = exactMatchAction.ttiAction.actionStop;
            inFields[i++] = exactMatchAction.ttiAction.hashMaskIndex;
            inFields[i++] = exactMatchAction.ttiAction.modifyMacSa;
            inFields[i++] = exactMatchAction.ttiAction.modifyMacDa;
            inFields[i++] = exactMatchAction.ttiAction.sourceEPortAssignmentEnable;
            inFields[i++] = exactMatchAction.ttiAction.sourceEPort;
            inFields[i++] = exactMatchAction.ttiAction.flowId;
            inFields[i++] = exactMatchAction.ttiAction.setMacToMe;
            inFields[i++] = exactMatchAction.ttiAction.rxProtectionSwitchEnable;
            inFields[i++] = exactMatchAction.ttiAction.rxIsProtectionPath;
            inFields[i++] = exactMatchAction.ttiAction.oamTimeStampEnable;
            inFields[i++] = exactMatchAction.ttiAction.oamOffsetIndex;
            inFields[i++] = exactMatchAction.ttiAction.oamProcessEnable;
            inFields[i++] = exactMatchAction.ttiAction.oamProfile;
            inFields[i++] = exactMatchAction.ttiAction.oamChannelTypeToOpcodeMappingEnable;
            inFields[i++] = exactMatchAction.ttiAction.isPtpPacket;
            inFields[i++] = exactMatchAction.ttiAction.ptpTriggerType;
            inFields[i++] = exactMatchAction.ttiAction.ptpOffset;
            inFields[i++] = exactMatchAction.ttiAction.cwBasedPw;
            inFields[i++] = exactMatchAction.ttiAction.ttlExpiryVccvEnable;
            inFields[i++] = exactMatchAction.ttiAction.pwe3FlowLabelExist;
            inFields[i++] = exactMatchAction.ttiAction.pwCwBasedETreeEnable;
            inFields[i++] = exactMatchAction.ttiAction.applyNonDataCwCommand;

            inFields[i++] = exactMatchAction.ttiAction.tunnelHeaderStartL4Enable;
            inFields[i++] = exactMatchAction.ttiAction.skipFdbSaLookupEnable;
            inFields[i++] = exactMatchAction.ttiAction.ipv6SegmentRoutingEndNodeEnable;
            inFields[i++] = exactMatchAction.ttiAction.exactMatchOverTtiEn;

            fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                        "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                        "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                        "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                        "%d%d%d%d%d%d%d%d%d%d%d"
                        "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                        "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                        "%d%d%d%d%d%d%d%d%d%d%d",
                        inFields[0],  inFields[1],   inFields[2],   inFields[3],
                        inFields[4],  inFields[5],   inFields[6],   inFields[7],
                        inFields[8],  inFields[9],   inFields[10],  inFields[11],
                        inFields[12], inFields[13],  inFields[14],  inFields[15],
                        inFields[16], inFields[17],  inFields[18],  inFields[19],
                        inFields[20], inFields[21],  inFields[22],  inFields[23],
                        inFields[24], inFields[25],  inFields[26],  inFields[27],
                        inFields[28], inFields[29],  inFields[30],  inFields[31],
                        inFields[32], inFields[33],  inFields[34],  inFields[35],
                        inFields[36], inFields[37],  inFields[38],  inFields[39],
                        inFields[40], inFields[41],  inFields[42],  inFields[43],
                        inFields[44], inFields[45],  inFields[46],  inFields[47],
                        inFields[48], inFields[49],  inFields[50],  inFields[51],
                        inFields[52], inFields[53],  inFields[54],  inFields[55],
                        inFields[56], inFields[57],  inFields[58],  inFields[59],
                        inFields[60], inFields[61],  inFields[62],  inFields[63],
                        inFields[64], inFields[65],  inFields[66],  inFields[67],
                        inFields[68], inFields[69],  inFields[70],  inFields[71],
                        inFields[72], inFields[73],  inFields[74],  inFields[75],
                        inFields[76], inFields[77],  inFields[78],  inFields[79],
                        inFields[80], inFields[81],  inFields[82],  inFields[83],
                        inFields[84], inFields[85],  inFields[86],  inFields[87],
                        inFields[88], inFields[89],  inFields[90],  inFields[91],
                        inFields[92], inFields[93],  inFields[94],  inFields[95],
                        inFields[96], inFields[97],  inFields[98],  inFields[99],
                        inFields[100], inFields[101],  inFields[102],  inFields[103],
                        inFields[104], inFields[105],  inFields[106],  inFields[107],
                        inFields[108], inFields[109],  inFields[110],  inFields[111],
                        inFields[112], inFields[113],  inFields[114],  inFields[115],
                        inFields[116], inFields[117],  inFields[118],  inFields[119],
                        inFields[120], inFields[121],  inFields[122],  inFields[123],
                        inFields[124], inFields[125],  inFields[126],  inFields[127],
                        inFields[128], inFields[129],  inFields[130],  inFields[131],
                        inFields[132], inFields[133],  inFields[134],  inFields[135],
                        inFields[136], inFields[137],  inFields[138],  inFields[139],
                        inFields[140], inFields[141],  inFields[142],  inFields[143],
                        inFields[144], inFields[145],  inFields[146],  inFields[147],
                        inFields[148], inFields[149]);

            galtisOutput(outArgs, GT_OK, "%d%f", 0);

            break;

        case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
        case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:

            inFields[i++]  = exactMatchAction.ttiAction.tunnelTerminate;
            inFields[i++]  = exactMatchAction.ttiAction.ttPassengerPacketType;
            inFields[i++]  = exactMatchAction.ttiAction.tsPassengerPacketType;
            inFields[i++]  = exactMatchAction.ttiAction.ttHeaderLength;
            inFields[i++]  = exactMatchAction.ttiAction.continueToNextTtiLookup;
            inFields[i++]  = exactMatchAction.ttiAction.copyTtlExpFromTunnelHeader;
            inFields[i++]  = exactMatchAction.ttiAction.mplsCommand;
            inFields[i++]  = exactMatchAction.ttiAction.mplsTtl;
            inFields[i++]  = exactMatchAction.ttiAction.enableDecrementTtl;
            inFields[i++]  = exactMatchAction.ttiAction.passengerParsingOfTransitMplsTunnelMode;
            inFields[i++]  = exactMatchAction.ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable;
            inFields[i++]  = exactMatchAction.ttiAction.command;
            inFields[i++] = exactMatchAction.ttiAction.redirectCommand;
            inFields[i++] = exactMatchAction.ttiAction.egressInterface.type;
            tempHwDev      = exactMatchAction.ttiAction.egressInterface.devPort.hwDevNum;
            tempPort     = exactMatchAction.ttiAction.egressInterface.devPort.portNum;
            CONVERT_BACK_DEV_PORT_DATA_MAC(tempHwDev,tempPort);
            inFields[i++] = tempHwDev;
            inFields[i++] = tempPort;
            CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(exactMatchAction.ttiAction.egressInterface.trunkId);

            inFields[i++] = exactMatchAction.ttiAction.egressInterface.trunkId;
            inFields[i++] = exactMatchAction.ttiAction.egressInterface.vidx;
            inFields[i++] = exactMatchAction.ttiAction.egressInterface.vlanId;
            inFields[i++] = exactMatchAction.ttiAction.arpPtr;
            inFields[i++] = exactMatchAction.ttiAction.tunnelStart;
            inFields[i++] = exactMatchAction.ttiAction.tunnelStartPtr;
            inFields[i++] = exactMatchAction.ttiAction.routerLttPtr;
            inFields[i++] = exactMatchAction.ttiAction.vrfId;
            inFields[i++] = exactMatchAction.ttiAction.sourceIdSetEnable;
            inFields[i++] = exactMatchAction.ttiAction.sourceId;
            inFields[i++] = exactMatchAction.ttiAction.tag0VlanCmd;
            inFields[i++] = exactMatchAction.ttiAction.tag0VlanId;
            inFields[i++] = exactMatchAction.ttiAction.tag1VlanCmd;
            inFields[i++] = exactMatchAction.ttiAction.tag1VlanId;
            inFields[i++] = exactMatchAction.ttiAction.tag0VlanPrecedence;
            inFields[i++] = exactMatchAction.ttiAction.nestedVlanEnable;
            inFields[i++] = exactMatchAction.ttiAction.bindToPolicerMeter;
            inFields[i++] = exactMatchAction.ttiAction.bindToPolicer;
            inFields[i++] = exactMatchAction.ttiAction.policerIndex;
            inFields[i++] = exactMatchAction.ttiAction.qosPrecedence;
            inFields[i++] = exactMatchAction.ttiAction.keepPreviousQoS;
            inFields[i++] = exactMatchAction.ttiAction.trustUp;
            inFields[i++] = exactMatchAction.ttiAction.trustDscp;
            inFields[i++] = exactMatchAction.ttiAction.trustExp;
            inFields[i++] = exactMatchAction.ttiAction.qosProfile;
            inFields[i++] = exactMatchAction.ttiAction.modifyTag0Up;
            inFields[i++] = exactMatchAction.ttiAction.tag1UpCommand;
            inFields[i++] = exactMatchAction.ttiAction.modifyDscp;
            inFields[i++] = exactMatchAction.ttiAction.tag0Up;
            inFields[i++] = exactMatchAction.ttiAction.tag1Up;
            inFields[i++] = exactMatchAction.ttiAction.remapDSCP;
            inFields[i++] = exactMatchAction.ttiAction.qosUseUpAsIndexEnable;
            inFields[i++] = exactMatchAction.ttiAction.qosMappingTableIndex;
            inFields[i++] = exactMatchAction.ttiAction.mplsLLspQoSProfileEnable;
            inFields[i++] = exactMatchAction.ttiAction.pcl0OverrideConfigIndex;
            inFields[i++] = exactMatchAction.ttiAction.pcl0_1OverrideConfigIndex;
            inFields[i++] = exactMatchAction.ttiAction.pcl1OverrideConfigIndex;
            inFields[i++] = exactMatchAction.ttiAction.iPclConfigIndex;
            inFields[i++] = exactMatchAction.ttiAction.iPclUdbConfigTableEnable;
            inFields[i++] = exactMatchAction.ttiAction.iPclUdbConfigTableIndex;
            inFields[i++] = exactMatchAction.ttiAction.mirrorToIngressAnalyzerEnable;
            inFields[i++] = exactMatchAction.ttiAction.mirrorToIngressAnalyzerIndex;
            inFields[i++] = exactMatchAction.ttiAction.userDefinedCpuCode;
            inFields[i++] = exactMatchAction.ttiAction.bindToCentralCounter;
            inFields[i++] = exactMatchAction.ttiAction.centralCounterIndex;
            inFields[i++] = exactMatchAction.ttiAction.vntl2Echo;
            inFields[i++] = exactMatchAction.ttiAction.bridgeBypass;
            inFields[i++] = exactMatchAction.ttiAction.ingressPipeBypass;
            inFields[i++] = exactMatchAction.ttiAction.actionStop;
            inFields[i++] = exactMatchAction.ttiAction.hashMaskIndex;
            inFields[i++] = exactMatchAction.ttiAction.modifyMacSa;
            inFields[i++] = exactMatchAction.ttiAction.modifyMacDa;
            inFields[i++] = exactMatchAction.ttiAction.sourceEPortAssignmentEnable;
            inFields[i++] = exactMatchAction.ttiAction.sourceEPort;
            inFields[i++] = exactMatchAction.ttiAction.flowId;
            inFields[i++] = exactMatchAction.ttiAction.setMacToMe;
            inFields[i++] = exactMatchAction.ttiAction.rxProtectionSwitchEnable;
            inFields[i++] = exactMatchAction.ttiAction.rxIsProtectionPath;
            inFields[i++] = exactMatchAction.ttiAction.oamTimeStampEnable;
            inFields[i++] = exactMatchAction.ttiAction.oamOffsetIndex;
            inFields[i++] = exactMatchAction.ttiAction.oamProcessEnable;
            inFields[i++] = exactMatchAction.ttiAction.oamProfile;
            inFields[i++] = exactMatchAction.ttiAction.oamChannelTypeToOpcodeMappingEnable;
            inFields[i++] = exactMatchAction.ttiAction.isPtpPacket;
            inFields[i++] = exactMatchAction.ttiAction.ptpTriggerType;
            inFields[i++] = exactMatchAction.ttiAction.ptpOffset;
            inFields[i++] = exactMatchAction.ttiAction.cwBasedPw;
            inFields[i++] = exactMatchAction.ttiAction.ttlExpiryVccvEnable;
            inFields[i++] = exactMatchAction.ttiAction.pwe3FlowLabelExist;
            inFields[i++] = exactMatchAction.ttiAction.pwCwBasedETreeEnable;
            inFields[i++] = exactMatchAction.ttiAction.applyNonDataCwCommand;

            inFields[i++] = exactMatchAction.ttiAction.tunnelHeaderStartL4Enable;
            inFields[i++] = exactMatchAction.ttiAction.skipFdbSaLookupEnable;
            inFields[i++] = exactMatchAction.ttiAction.ipv6SegmentRoutingEndNodeEnable;
            inFields[i++] = exactMatchAction.ttiAction.exactMatchOverTtiEn;

            fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                        "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                        "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                        "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                        "%d%d%d%d%d%d%d%d%d%d%d"
                        "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                        "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
                        "%d%d%d%d%d%d%d%d%d%d%d",
                        inFields[0],  inFields[1],   inFields[2],   inFields[3],
                        inFields[4],  inFields[5],   inFields[6],   inFields[7],
                        inFields[8],  inFields[9],   inFields[10],  inFields[11],
                        inFields[12], inFields[13],  inFields[14],  inFields[15],
                        inFields[16], inFields[17],  inFields[18],  inFields[19],
                        inFields[20], inFields[21],  inFields[22],  inFields[23],
                        inFields[24], inFields[25],  inFields[26],  inFields[27],
                        inFields[28], inFields[29],  inFields[30],  inFields[31],
                        inFields[32], inFields[33],  inFields[34],  inFields[35],
                        inFields[36], inFields[37],  inFields[38],  inFields[39],
                        inFields[40], inFields[41],  inFields[42],  inFields[43],
                        inFields[44], inFields[45],  inFields[46],  inFields[47],
                        inFields[48], inFields[49],  inFields[50],  inFields[51],
                        inFields[52], inFields[53],  inFields[54],  inFields[55],
                        inFields[56], inFields[57],  inFields[58],  inFields[59],
                        inFields[60], inFields[61],  inFields[62],  inFields[63],
                        inFields[64], inFields[65],  inFields[66],  inFields[67],
                        inFields[68], inFields[69],  inFields[70],  inFields[71],
                        inFields[72], inFields[73],  inFields[74],  inFields[75],
                        inFields[76], inFields[77],  inFields[78],  inFields[79],
                        inFields[80], inFields[81],  inFields[82],  inFields[83],
                        inFields[84], inFields[85],  inFields[86],  inFields[87],
                        inFields[88], inFields[89],  inFields[90], inFields[91],
                        inFields[92],  inFields[93],  inFields[94],inFields[95],
                        inFields[96],  inFields[97],  inFields[98],inFields[99],
                        inFields[100], inFields[101],  inFields[102],  inFields[103],
                        inFields[104], inFields[105],  inFields[106],  inFields[107],
                        inFields[108], inFields[109],  inFields[110],  inFields[111],
                        inFields[112], inFields[113],  inFields[114],  inFields[115],
                        inFields[116], inFields[117],  inFields[118],  inFields[119],
                        inFields[120], inFields[121],  inFields[122],  inFields[123],
                        inFields[124], inFields[125],  inFields[126],  inFields[127],
                        inFields[128], inFields[129],  inFields[130],  inFields[131],
                        inFields[132], inFields[133],  inFields[134],  inFields[135],
                        inFields[136], inFields[137],  inFields[138],  inFields[139],
                        inFields[140],inFields[141]);

            galtisOutput(outArgs, GT_OK, "%d%f", 1);
        break;

    default:
        galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong EXACT MATCH ENTRY FORMAT.\n");
        return CMD_AGENT_ERROR;
    }

    return CMD_OK;
}

/**
* @internal wrCpssDxChExactMatchEntryReadFirst function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChExactMatchEntryReadFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8       devNum;
    GT_U32      portGroupId;/* port group Id iterator */

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];
    exactMatchEntryIndex = (GT_32)inArgs[1];
    exactMatchEntryNumOfEntries= (GT_32)inArgs[2];

    /* check if requested refresh range is legal */
    if (exactMatchEntryIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* check if requested refresh is all */
    if ((exactMatchEntryIndex == 0) && (exactMatchEntryNumOfEntries==0))
    {
        exactMatchEntryNumOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum;
    }
    else
    {
        /* check if the application request 0 entries but not all */
        if (exactMatchEntryNumOfEntries == 0)
        {
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }
    }

    exactMatchEntryIndexMaxGet = exactMatchEntryIndex + exactMatchEntryNumOfEntries - 1;


    exactMatchMultiPortGroupsBmpGet(devNum,&multiPortGroupsBmpEnable, &multiPortGroupsBmp);

    if(PRV_CPSS_IS_DEV_EXISTS_MAC(devNum) &&
       PRV_CPSS_DXCH_FAMILY_CHECK_MAC(devNum))
    {
        if(multiPortGroupsBmpEnable == GT_FALSE)
        {
            multiPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        }

        /* get first port group to access */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,multiPortGroupsBmp,portGroupId)
        {
            refreshExactMatchFirstPortGroupId = portGroupId;
            break;
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,multiPortGroupsBmp,portGroupId)

        refreshExactMatchCurrentPortGroupId = refreshExactMatchFirstPortGroupId;
        useRefreshExactMatchPerPortGroupId = GT_TRUE;
    }
    else
    {
        useRefreshExactMatchPerPortGroupId = GT_FALSE;
    }


    return internalDxChExactMatchEntryReadNext(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChBrgFdbEntryReadNext function
* @endinternal
*
*/
static CMD_STATUS wrCpssDxChExactMatchEntryReadNext
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

    return internalDxChExactMatchEntryReadNext(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChExactMatchEntryDelete function
* @endinternal
*
* @brief   The function delete an entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*
* @note NONE
*
*/
static CMD_STATUS wrCpssDxChExactMatchEntryDelete
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                 result;

    GT_U8                     devNum;
    GT_U32                    index;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    index = (GT_U32)inFields[0];

    exactMatchMultiPortGroupsBmpGet(devNum,&multiPortGroupsBmpEnable, &multiPortGroupsBmp);
    if(multiPortGroupsBmpEnable == GT_FALSE)
    {
        /* call cpss api function */
        result = cpssDxChExactMatchPortGroupEntryInvalidate(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index);
    }
    else
    {
        /* call cpss api function */
        result = cpssDxChExactMatchPortGroupEntryInvalidate(devNum,multiPortGroupsBmp,index);
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChExactMatchHashCrcMultiResultsByParamsCalc function
* @endinternal
*
* @brief   This function calculates the CRC multiple hash results
*         (indexes into the Exact Match table).
*         NOTE: the function do not access the HW , and do only SW calculations.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameters in entryKeyPtr.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChExactMatchHashCrcMultiResultsByParamsCalc
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                       result = GT_OK;
    GT_U8                                           devNum;
    CPSS_DXCH_EXACT_MATCH_KEY_STC                   keyParams;
    GT_U32                                          numberOfElemInCrcMultiHash;
    GT_U32                                          exactMatchCrcMultiHashArr[CPSS_DXCH_EXACT_MATCH_MAX_NUM_BANKS_CNS];
    GT_U32                                          ii=0;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    keyParams.keySize = (CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT)inArgs[1];
    for (ii=0;ii<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS;ii++)
    {
        keyParams.pattern[ii] = (GT_U32)inArgs[ii+2];
    }

    /* call cpss api function */
    result = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &keyParams, &numberOfElemInCrcMultiHash,&exactMatchCrcMultiHashArr[0]);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x",
                 numberOfElemInCrcMultiHash,exactMatchCrcMultiHashArr[0] , exactMatchCrcMultiHashArr[1]  , exactMatchCrcMultiHashArr[2]  ,
                 exactMatchCrcMultiHashArr[3]  ,exactMatchCrcMultiHashArr[4] , exactMatchCrcMultiHashArr[5]  , exactMatchCrcMultiHashArr[6]  ,
                 exactMatchCrcMultiHashArr[7]  ,exactMatchCrcMultiHashArr[8] , exactMatchCrcMultiHashArr[9]  , exactMatchCrcMultiHashArr[10] ,
                 exactMatchCrcMultiHashArr[11] ,exactMatchCrcMultiHashArr[12], exactMatchCrcMultiHashArr[13] , exactMatchCrcMultiHashArr[14] ,
                 exactMatchCrcMultiHashArr[15]);

    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{

    {"cpssDxChExactMatchTtiProfileIdSet",
      &wrCpssDxChExactMatchTtiProfileIdSet,
      5, 0},
    {"cpssDxChExactMatchTtiProfileIdGet",
      &wrCpssDxChExactMatchTtiProfileIdGet,
      3, 0},

    {"cpssDxChExactMatchPclProfileIdSet",
    &wrCpssDxChExactMatchPclProfileIdSet,
    7, 0},
    {"cpssDxChExactMatchPclProfileIdGet",
    &wrCpssDxChExactMatchPclProfileIdGet,
    5, 0},

    {"cpssDxChExactMatchClientLookupSet",
      &wrCpssDxChExactMatchClientLookupSet,
      3,0},

    {"cpssDxChExactMatchClientLookupGet",
      &wrCpssDxChExactMatchClientLookupGet,
      2,0},

    {"cpssDxChExactMatchActivityBitEnableSet",
      &wrCpssDxChExactMatchActivityBitEnableSet,
      2,0},

    {"cpssDxChExactMatchActivityBitEnableGet",
      &wrCpssDxChExactMatchActivityBitEnableGet,
      1,0},

    {"cpssDxChExactMatchActivityStatusGet",
      &wrCpssDxChExactMatchActivityStatusGet,
      3,0},

    {"cpssDxChExactMatchProfileKeyParamsSet",
     &wrCpssDxChExactMatchProfileKeyParamsSet,
     1, 50},
    {"cpssDxChExactMatchProfileKeyParamsGetFirst",
     &wrCpssDxChExactMatchProfileKeyParamsGetFirst,
     1, 0},
    {"cpssDxChExactMatchProfileKeyParamsGetNext",
     &wrCpssDxChExactMatchProfileKeyParamsGetNext,
     1, 0},

    {"cpssDxChExactMatchProfileDefaultActionSet",
     &wrCpssDxChExactMatchProfileDefaultActionUpdate,
     2, 94},
    {"cpssDxChExactMatchProfileDefaultActionGetFirst",
     &wrCpssDxChExactMatchProfileDefaultActionGetFirst,
     2, 0},
    {"cpssDxChExactMatchProfileDefaultActionGetNext",
     &wrCpssDxChExactMatchProfileDefaultActionGetNext,
     2, 0},

    {"cpssDxChExactMatchExpandedActionSet",
     &wrCpssDxChExactMatchExpandedActionUpdate,
     2, 175},
    {"cpssDxChExactMatchExpandedActionGetFirst",
     &wrCpssDxChExactMatchExpandedActionGetFirst,
     2, 0},
    {"cpssDxChExactMatchExpandedActionGetNext",
     &wrCpssDxChExactMatchExpandedActionGetNext,
     2, 0},

    {"cpssDxChExactMatchEntrySet",
      &wrCpssDxChExactMatchEntrySet,
     4, 150},

    {"cpssDxChExactMatchEntryEndSet",
      &wrCpssDxChExactMatchEntryEndSet,
     4, 0},

    {"cpssDxChExactMatchEntryCancelSet",
     &wrCpssDxChExactMatchEntryCancelSet,
     0, 0},

    {"cpssDxChExactMatchEntryGetFirst",
     &wrCpssDxChExactMatchEntryReadFirst,
     4, 0},

    {"cpssDxChExactMatchEntryGetNext",
     &wrCpssDxChExactMatchEntryReadNext,
     4, 0},

    {"cpssDxChExactMatchEntryDelete",
     &wrCpssDxChExactMatchEntryDelete,
     4, 150},

    {"cpssDxChExactMatchHashCrcMultiResultsByParamsCalc",
     &wrCpssDxChExactMatchHashCrcMultiResultsByParamsCalc,
     49, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibResetCpssDxChExactMatch function
* @endinternal
*
* @brief   Library database reset function.
*
* @note none
*
*/
static GT_VOID cmdLibResetCpssDxChExactMatch
(
    GT_VOID
)
{
    /* support for multi port groups */
    multiPortGroupsBmpEnable = GT_FALSE;

    /* current port group for the 'refresh' Exact Match */
    useRefreshExactMatchPerPortGroupId = GT_FALSE;
    /* current port group for the 'refresh' Exact Match */
    refreshExactMatchCurrentPortGroupId = 0;
    /* first port group for the 'refresh' Exact Match */
    refreshExactMatchFirstPortGroupId = 0;
}

/**
* @internal cmdLibInitCpssDxChExactMatch function
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
GT_STATUS cmdLibInitCpssDxChExactMatch
(
 GT_VOID
 )
{
    GT_STATUS rc;

    rc = wrCpssRegisterResetCb(cmdLibResetCpssDxChExactMatch);
    if(rc != GT_OK)
    {
        return rc;
    }

    return cmdInitLibrary(dbCommands, numCommands);
}

