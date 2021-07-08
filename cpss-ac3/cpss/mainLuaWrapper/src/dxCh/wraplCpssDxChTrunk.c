/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplCpssDxChTrunk.c
*
* DESCRIPTION:
*       A lua wrapper for trunk functions
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 17 $
*******************************************************************************/

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/cpssTypes.h>
#include <generic/private/prvWraplGeneral.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <cpss/dxCh/dxChxGen/trunk/private/prvCpssDxChTrunk.h>
#include <cpssCommon/wraplCpssExtras.h>
#include <cpssCommon/wraplCpssDeviceInfo.h>
#include <cpssCommon/wraplCpssTrunk.h>
#include <dxCh/wraplCpssDxChTrunk.h>

#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>

/*******************************************************************************
*   Trunk wrapper warnings                                                     *
*******************************************************************************/
GT_CHAR portOccurrenceInOthersTrunksWarningMessage[]            =
                                            "Port occures in port-channel %d";
GT_CHAR xcatDeviceAndTrunkIDOfWATrafficToCPUWarningMessage[]    =
                                                "Trunk WA CPU traffic problem";
GT_CHAR maximumTrunkMembersCountExceedingWarningMessage[]       =
                                    "Exceeding the maximum trunk members count";

/*******************************************************************************
*   Trunk wrapper errors                                                       *
*******************************************************************************/
GT_CHAR trunkCreationErrorMessage[]         =
                            "Could not create trunk with given configuration";
GT_CHAR trunkPortCountOveflowErrorMessage[] =
                                                        "To many trunk members";
GT_CHAR trunkGettingErrorMessage[]          =
                                "Could not read requested trunk information";


/***** declarations ********/

use_prv_print_struct_array(CPSS_TRUNK_MEMBER_STC);

typedef struct{
    GT_U32                numMembers;
    CPSS_TRUNK_MEMBER_STC membersArray[CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS];
}CPSS_TRUNK_MEMBERS_AND_NUM_STC;
extern void prv_lua_to_c_CPSS_TRUNK_MEMBERS_AND_NUM_STC(
    lua_State *L,
    CPSS_TRUNK_MEMBERS_AND_NUM_STC *val
);
extern void prv_c_to_lua_CPSS_TRUNK_MEMBERS_AND_NUM_STC(
    lua_State *L,
    CPSS_TRUNK_MEMBERS_AND_NUM_STC *val
);
extern enumDescr enumDescr_CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT[];
/***** declarations ********/


/*******************************************************************************
* pvrCpssTrunkMembersArrayToLuaPush
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Push dev/port array to lua stack.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       status              - caller status
*       L                   - lua state
*       L_index             - entry lua stack index
*
* OUTPUTS:
*
* RETURNS:
*       1 if array was pushed to stack otherwise 0
*
* COMMENTS:
*
*******************************************************************************/
static int pvrCpssTrunkMembersArrayToLuaPush
(
    IN GT_STATUS                status,
    IN lua_State                *L,
    IN GT_U32                   numMembers,
    IN CPSS_TRUNK_MEMBER_STC    membersArray[]
)
{
    GT_U32                      index;

    if (GT_OK == status)
    {
        lua_newtable(L);
        for (index = 0; index < numMembers; index++)
        {
            lua_pushnumber(L, membersArray[index].hwDevice);
            lua_rawget(L, -2);
            if (lua_isnil(L, -1))
            {
                lua_pop(L, 1);
                lua_pushnumber(L, membersArray[index].hwDevice);
                lua_newtable(L);
                lua_rawset(L, -3);
                lua_pushnumber(L, membersArray[index].hwDevice);
                lua_rawget(L, -2);
            }
            lua_pushnumber(L, membersArray[index].port);
            lua_rawseti(L, -2, (int)lua_objlen(L, -2) + 1);
            lua_pop(L, 1);
        }

        return 1;
    }
    else
    {
        return 0;
    }
}

/*******************************************************************************
* pvrCpssTrunkMembersArrayToLuaPush_getRealOrder
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Push dev/port array to lua stack - keep the order of the members
*       in format of : CPSS_TRUNK_MEMBERS_AND_NUM_STC
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       status              - caller status
*       L                   - lua state
*       L_index             - entry lua stack index
*
* OUTPUTS:
*
* RETURNS:
*       1 if array was pushed to stack otherwise 0
*
* COMMENTS:
*
*******************************************************************************/
static int pvrCpssTrunkMembersArrayToLuaPush_getRealOrder
(
    IN GT_STATUS                status,
    IN lua_State                *L,
    IN CPSS_TRUNK_MEMBERS_AND_NUM_STC    *membersArrayAndNumPtr
)
{
    if (GT_OK != status)
    {
        return 0;
    }

    prv_c_to_lua_CPSS_TRUNK_MEMBERS_AND_NUM_STC(L,membersArrayAndNumPtr);

    return 1;
}

/*******************************************************************************
* prvCpssDxChDoesTrunkExist
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Check's that trunk exists.
*
* APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*       None.
*
* INPUTS:
*       devNum                - device number
*       trunkId               - trunk id
*       P_CALLING_FORMAL_DATA - general debug information (environment variables
*                               etc); could be empty
*
* OUTPUTS:
*       doesVlanExist         - vlan existence
*       errorMessagePtr       - error message
*
* RETURNS:
*       operation execution status
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChDoesTrunkExist
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId,
    OUT GT_BOOL                 *doesTrunkValid,
    OUT GT_CHAR_PTR             *errorMessagePtr
    P_CALLING_FORMAL_DATA
)
{
    GT_STATUS                   status          = GT_OK;
    GT_U32                      maxNumberOfTrunks = 0;
    CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT trunkMembersMode = 0;
    *doesTrunkValid      = GT_TRUE;
    *errorMessagePtr    = NULL;

    status = cpssDxChTrunkDbInitInfoGet(devNum, &maxNumberOfTrunks, &trunkMembersMode);
    P_CALLING_API(
        cpssDxChTrunkDbInitInfoGet,
        PC_NUMBER(devNum,                       devNum,                  GT_U8,
        PC_NUMBER(maxNumberOfTrunks,            maxNumberOfTrunks,           GT_U32,
        PC_ENUM  (trunkMembersMode,            trunkMembersMode,           CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT,
        PC_LAST_PARAM))),
        PC_STATUS);
    if (GT_OK != status)
    {
        maxNumberOfTrunks = 0;
    }

    *doesTrunkValid  = trunkId > maxNumberOfTrunks ? GT_FALSE : GT_TRUE;

    return status;
}


/*******************************************************************************
* prvCpssDxChTrunkMembersCount
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of trunk members (dev/ports) count.
*
* APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*       None.
*
* INPUTS:
*       devNum              - device number
*       trunkId             - trunk id
*       P_CALLING_FORMAL_DATA
*                           - general debug information (environment variables
*                             etc); could be empty
*
* OUTPUTS:
*       numMembersInTrunkPtr
*                           - number of trunk members
*       errorMessagePtr     - possible error message, relevant
*                                   if status != GT_OK
*
* RETURNS:
*       operation execution status
*
* COMMENTS:
*
*******************************************************************************/
static GT_STATUS prvCpssDxChTrunkMembersCount
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId,
    OUT GT_U32_PTR              numMembersInTrunkPtr,
    OUT GT_CHAR_PTR             *errorMessagePtr
    P_CALLING_FORMAL_DATA
)
{
    GT_STATUS       status              = GT_OK;
    GT_U32  numOfEnabledMembers = 0;

    *errorMessagePtr   = NULL;

    status = cpssDxChTrunkDbEnabledMembersGet(devNum, trunkId, &numOfEnabledMembers, NULL);
    P_CALLING_API(
        cpssDxChTrunkDbEnabledMembersGet,
        PC_NUMBER(devNum,                       devNum,             GT_U8,
        PC_NUMBER(trunkId,                      trunkId,            GT_TRUNK_ID,
        PC_NUMBER(numOfEnabledMembers,        numOfEnabledMembers,  GT_U32,
        PC_ARRAY (0,                       NULL,  NULL , CPSS_TRUNK_MEMBER_STC,
        PC_LAST_PARAM)))),
        PC_STATUS);
    if (GT_OK != status)
    {
        *errorMessagePtr    = trunkGettingErrorMessage;
    }

    *numMembersInTrunkPtr = numOfEnabledMembers;

    return status;
}

/*******************************************************************************
* prvCpssDxChMaximumTrunkMembersCount
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of maximum trunk members (dev/ports) count.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum              - device number
*
* OUTPUTS:
*       maximumNumberMembersInTrunkPtr
*                           - maximum number of trunk members
*       errorMessage        - possible error message, relevant
*                                   if status != GT_OK
*
* RETURNS:
*       GT_OK and maximum member counts pused to lua stack
*
* COMMENTS:
*
*******************************************************************************/
static GT_STATUS prvCpssDxChMaximumTrunkMembersCount
(
    IN  GT_U8                   devNum,
    OUT GT_U32_PTR              maximumNumberMembersInTrunkPtr,
    OUT GT_CHAR_PTR             *errorMessage
)
{
    GT_STATUS       status      = GT_OK;

    *errorMessage   = NULL;

    *maximumNumberMembersInTrunkPtr =
        PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->maxNumMembersInTrunk;

    return status;
}


/*******************************************************************************
* prvXcatDeviceAndTrunkIDOfWATrafficToCPUCheck
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Checking, that device need the WA but the trunkId not according to the
*       rules.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum              - device number
*       trunkId             - trunk id
*
* OUTPUTS:
*       chekingResultPtr    - checking result
*       errorMessage        - possible error message, relevant
*                                   if status != GT_OK
*
* RETURNS:
*       operation execution status
*
* COMMENTS:
*
*******************************************************************************/
static GT_STATUS prvXcatDeviceAndTrunkIDOfWATrafficToCPUCheck
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId,
    OUT GT_BOOL                 *chekingResultPtr,
    OUT GT_CHAR_PTR             *errorMessage
)
{
    GT_STATUS                   status          = GT_OK;

    *errorMessage       = NULL;

    *chekingResultPtr   =
        GT_FALSE == WRL_PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->initDone;

    if ((GT_OK == status) && (GT_FALSE== *chekingResultPtr))
    {
        *chekingResultPtr   =
            0 == WRL_PRV_CPSS_DXCH_CHECK_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_MAC(devNum);
    }

    if ((GT_OK == status) && (GT_FALSE == *chekingResultPtr))
    {
        *chekingResultPtr   =
            1 == WRL_XCAT_TRUNKID_CHECK_FOR_WA_WRONG_SOURCE_PORT_TO_CPU_MAC(trunkId);
    }

    if ((GT_OK == status) && (GT_FALSE == *chekingResultPtr))
    {
        *chekingResultPtr   = (((trunkId) & 0x7e) == 0x7e) || ((trunkId) == 0);
    }

    return status;
}

/*******************************************************************************
* wrlDxChTrunkMemberAddingConditionChecking
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Checking of condition for trunk member adding.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       L                   - lua state
*       L[1]                - device number
*       L[2]                - trunk id
*       L[3]                - hardware device number
*       L[4]                - hardware port number
*
* OUTPUTS:
*
* RETURNS:
*       GT_OK and true are pused to lua stack if no errors occurs and checking
*       was successful
*       GT_OK, true and warning message are pused to lua stack if no errors
*       occurs and checking was not successful
*       error status and error message it error occures
*
* COMMENTS:
*
*******************************************************************************/
int wrlDxChTrunkMemberAddingConditionChecking
(
    IN lua_State *L
)
{
    GT_U8                       devNum                  =
                                                    (GT_U8) lua_tonumber(L, 1);
                                                                    /* devId */
    GT_TRUNK_ID                 trunkId                 =
                                            (GT_TRUNK_ID) lua_tonumber(L, 2);
                                                                 /* trunkId */
    GT_HW_DEV_NUM               hwDevNum                =
                                            (GT_HW_DEV_NUM) lua_tonumber(L, 3);
                                                                /* hwDevNum */
    GT_PHYSICAL_PORT_NUM        hwPortNum               =
                                    (GT_PHYSICAL_PORT_NUM) lua_tonumber(L, 4);
                                                                /* hwPortNum */
    GT_BOOL                     chekingResult           = GT_TRUE;
    GT_BOOL                     isPortAlreadyInTrunk    = GT_FALSE;
    GT_U32                      numMembersInTrunk       = 0;
    GT_U32                      maximumNumberMembersInTrunk
                                                        = 0;
    GT_TRUNK_ID                 temporaryTrunkId        = 0;
    GT_CHAR_PTR                 conditionWarningString  = NULL;
    GT_STATUS                   status                  = GT_OK;
    GT_CHAR_PTR                 error_message           = NULL;
    int                         returned_param_count    = 0;
    CPSS_TRUNK_MEMBER_STC       trunkMember;
    P_CALLING_CHECKING;

    trunkMember.port     = hwPortNum;
    trunkMember.hwDevice = hwDevNum;

    status = cpssDxChTrunkDbIsMemberOfTrunk(devNum, &trunkMember, &temporaryTrunkId);
    if(status == GT_OK)
    {
        /* already in trunk */
        if (trunkId != temporaryTrunkId)
        {
            /* member of other trunk */
            cpssOsSprintf(wraplWarningMessageString,
                          portOccurrenceInOthersTrunksWarningMessage,
                          temporaryTrunkId);
            conditionWarningString  = wraplWarningMessageString;
        }
        else
        {
            /* member of the needed trunk */
            isPortAlreadyInTrunk = GT_TRUE;
        }

        chekingResult = GT_FALSE;
    }
    else if(status == GT_NOT_FOUND)
    {
        /* the port is not member of any trunk */
        status = GT_OK;
    }

    if ((GT_OK == status) && (GT_TRUE == chekingResult))
    {
        status = prvXcatDeviceAndTrunkIDOfWATrafficToCPUCheck(devNum, trunkId,
                                                              &chekingResult,
                                                              &error_message);
        if ((GT_OK == status) && (GT_FALSE == chekingResult))
        {
            conditionWarningString  =
                xcatDeviceAndTrunkIDOfWATrafficToCPUWarningMessage;
        }
    }

    if ((GT_OK == status) && (GT_TRUE == chekingResult))
    {
        status = prvCpssDxChTrunkMembersCount(devNum, trunkId,
                                              &numMembersInTrunk,
                                              &error_message
                                              P_CALLING_ACTUAL_DATA);
    }

    if ((GT_OK == status) && (GT_TRUE == chekingResult))
    {
        status = prvCpssDxChMaximumTrunkMembersCount(
                     devNum, &maximumNumberMembersInTrunk, &error_message);
    }

    if ((GT_OK == status) && (GT_TRUE == chekingResult))
    {
        chekingResult   = maximumNumberMembersInTrunk >= numMembersInTrunk + 1;
        if (GT_FALSE == chekingResult)
        {
            conditionWarningString  =
                maximumTrunkMembersCountExceedingWarningMessage;
        }
    }

    returned_param_count    +=
        prvWrlCpssStatusToLuaPush(status, L, error_message);
    returned_param_count    +=
        pvrCpssBooleanToLuaPush(status, L, chekingResult);
    returned_param_count    +=
        pvrCpssBooleanToLuaPush(status, L, isPortAlreadyInTrunk);
    returned_param_count    +=
           pvrCpssStringToLuaPush(status, L, conditionWarningString);
    return returned_param_count;
}

/*******************************************************************************
* wrlCpssDxChTrunkDevicesAndPortsList
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of trunk devices/ports list.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*       devices/ports list pused to lua stack
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChTrunkDevicesAndPortsList
(
    IN lua_State *L
)
{
    GT_U32                      paramIndex = 1;
    GT_U8                   devNum                  =
                                            (GT_U8)       lua_tonumber(L, paramIndex++);
                                                                    /* devId */
    GT_TRUNK_ID             trunkId                 =
                                            (GT_TRUNK_ID) lua_tonumber(L, paramIndex++);
                                                                  /* trunkId */
    GT_U32                  getRealOrder                 =  0;
                                                                  /* trunkId */
    GT_U32                  numMembers;
    CPSS_TRUNK_MEMBER_STC   *membersArray;
    static CPSS_TRUNK_MEMBERS_AND_NUM_STC       membersArrayAndNum;
    GT_BOOL                 isInitializedMember;
    GT_STATUS               status                  = GT_OK;
    GT_CHAR_PTR             error_message           = NULL;
    int                     returned_param_count    = 0;
    P_CALLING_CHECKING;

    if (! lua_isnil(L, paramIndex))
    {
        getRealOrder = (GT_TRUNK_ID) lua_tonumber(L, paramIndex++);
    }

    membersArray = &membersArrayAndNum.membersArray[0];

    status = cpssDxChTrunkTableEntryGet(devNum, trunkId, &membersArrayAndNum.numMembers,
                                        membersArray);

    numMembers = membersArrayAndNum.numMembers;


    P_CALLING_API(
        cpssDxChTrunkTableEntryGet,
        PC_NUMBER(devNum,                       devNum,                  GT_U8,
        PC_NUMBER(trunkId,                      trunkId,           GT_TRUNK_ID,
        PC_NUMBER(numMembers,                   numMembers,             GT_U32,
        PC_ARRAY (numMembers, membersArray,    membersArray,
                                                         CPSS_TRUNK_MEMBER_STC,
        PC_LAST_PARAM)))),
        PC_STATUS);
    if (GT_OK != status)
    {
        error_message   = trunkGettingErrorMessage;
    }

    if ((GT_OK == status) && (1 == numMembers))
    {
        status = prvCpssIsHwDevicePortExist(
                     membersArray[0].hwDevice, membersArray[0].port,
                     &isInitializedMember, &error_message);

        if ((GT_OK == status) && (GT_FALSE == isInitializedMember))
        {
            numMembers = 0;
        }
    }

    returned_param_count    +=
        prvWrlCpssStatusToLuaPush(status, L, error_message);
    if(getRealOrder == 0)
    {
        returned_param_count    +=
            pvrCpssTrunkMembersArrayToLuaPush(status, L, numMembers, membersArray);
    }
    else
    {
        /* put 'real members order' on stack in format of : CPSS_TRUNK_MEMBERS_AND_NUM_STC */
        returned_param_count    +=
            pvrCpssTrunkMembersArrayToLuaPush_getRealOrder(status, L, &membersArrayAndNum);
    }

    return returned_param_count;
}

/*******************************************************************************
* wrlCpssDxChTrunkTableEntrySet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Creation of new trunk.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       L                   - lua state
*       L[1]                - device number
*       L[2]                - trunk id
*       L[2]                - dev/port array
*                               sample: {[0] = {1, 2}}
*
* OUTPUTS:
*
* RETURNS:
*       GT_OK is pused to lua stack if no errors occurs otherwise
*       error code and error message
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChTrunkTableEntrySet
(
    IN lua_State *L
)
{
    GT_U32                      paramIndex = 1;
    GT_U8                       devNum =  (GT_U8)       lua_tonumber(L, paramIndex++);
                                                                    /* devId */
    GT_TRUNK_ID                 trunkId = (GT_TRUNK_ID) lua_tonumber(L, paramIndex++);
                                                                  /* trunkId */
    static CPSS_TRUNK_MEMBERS_AND_NUM_STC       membersArrayAndNum;
    GT_STATUS                   status = GT_OK;
    GT_CHAR_PTR                 error_message = NULL;
    GT_U32                numMembers;
    CPSS_TRUNK_MEMBER_STC *membersArray;
    P_CALLING_CHECKING;

    /* call parser of 'members + num of members' */
    prv_lua_to_c_CPSS_TRUNK_MEMBERS_AND_NUM_STC(L,&membersArrayAndNum);

    numMembers = membersArrayAndNum.numMembers;
    membersArray = &membersArrayAndNum.membersArray[0];

    status = cpssDxChTrunkTableEntrySet(devNum, trunkId, numMembers, membersArray);
    P_CALLING_API(
        cpssDxChTrunkTableEntrySet,
        PC_NUMBER(devNum,                   devNum,                  GT_U8,

        PC_NUMBER(trunkId,                  trunkId,           GT_TRUNK_ID,
        PC_NUMBER(numMembers,               numMembers,             GT_U32,
        PC_ARRAY (numMembers, membersArray,membersArray,
                                                     CPSS_TRUNK_MEMBER_STC,
        PC_LAST_PARAM)))),
        PC_STATUS);
    if (GT_OK != status)
    {
        error_message   = trunkCreationErrorMessage;
    }

    return prvWrlCpssStatusToLuaPush(status, L, error_message);
}

/*
GT_STATUS prvCpssDxChTrunkHashBitsSelectionGet
(
    IN GT_U8            devNum,
    IN CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT hashClient,
    OUT GT_U32           *startBitPtr,
    OUT GT_U32           *numOfBitsPtr
)

*/

static CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT getHashClientParam
(
    IN lua_State *L,
    IN GT_U32    paramIndex
)
{
    const char* str;
    size_t      len;
    if (lua_isstring(L, paramIndex))
    {
        str = lua_tolstring(L, paramIndex, &len);
        if (0 == cpssOsStrCmp(str, "CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E"))
        {
            return CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E;
        }
        if (0 == cpssOsStrCmp(str, "CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E"))
        {
            return CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E;
        }
        if (0 == cpssOsStrCmp(str, "CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E"))
        {
            return CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E;
        }
        if (0 == cpssOsStrCmp(str, "CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_DESIGNATED_TABLE_E"))
        {
            return CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_DESIGNATED_TABLE_E;
        }
        if (0 == cpssOsStrCmp(str, "CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_IPCL_E"))
        {
            return CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_IPCL_E;
        }
        if (0 == cpssOsStrCmp(str, "CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_INGRESS_OAM_E"))
        {
            return CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_INGRESS_OAM_E;
        }
        if (0 == cpssOsStrCmp(str, "CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E"))
        {
            return CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E;
        }
        return CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE___LAST___E;
    }
    return (CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT)lua_tonumber(L, paramIndex);
}

/*******************************************************************************
* wrlPrvCpssDxChTrunkHashBitsSelectionSet
*
* DESCRIPTION:
*       Function Relevant mode : ALL modes
*
*       set the number of bits for client.
*
* APPLICABLE DEVICES:
*        Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* INPUTS:
*       L                   - lua state
*       L[1]                - device number
*       L[2]                - hashClient
*       L[3]                - startBit
*       L[4]                - numOfBits
*
* OUTPUTS:
*
* RETURNS:
*       GT_OK is pused to lua stack if no errors occurs otherwise
*       error code and error message
*
* COMMENTS:
*
*******************************************************************************/
int wrlPrvCpssDxChTrunkHashBitsSelectionSet
(
    IN lua_State *L
)
{
    GT_U8                                    devNum;
    CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT hashClient; /* parameter2 */
    GT_U32                                   startBit;
    GT_U32                                   numOfBits;
    GT_STATUS                                status = GT_OK;

    devNum     =  (GT_U8)       lua_tonumber(L, 1);
    hashClient =                getHashClientParam(L, 2);
    startBit   =  (GT_U32)      lua_tonumber(L, 3);
    numOfBits  =  (GT_U32)      lua_tonumber(L, 4);


    /* wrapper name different from API name becase */
    /* wrapper name is like Get-API                */
    status = cpssDxChTrunkHashNumBitsSet(
        devNum, hashClient, startBit, numOfBits);

    lua_pushinteger(L, status);
    return 1;
}

/*******************************************************************************
* wrlPrvCpssDxChTrunkHashBitsSelectionGet
*
* DESCRIPTION:
*       Function Relevant mode : ALL modes
*
*       set the number of bits for client.
*
* APPLICABLE DEVICES:
*        Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* INPUTS:
*       L                   - lua state
*       L[1]                - device number
*       L[2]                - hashClient
*
* OUTPUTS:
*
* RETURNS:
*       GT_OK is pused to lua stack if no errors occurs otherwise
*       error code and error message
*
* COMMENTS:
*
*******************************************************************************/
int wrlPrvCpssDxChTrunkHashBitsSelectionGet
(
    IN lua_State *L
)
{
    GT_U8                                    devNum;
    CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT hashClient; /* parameter2 */
    GT_U32                                   startBit;
    GT_U32                                   numOfBits;
    GT_STATUS                                status = GT_OK;

    devNum     =  (GT_U8)       lua_tonumber(L, 1);
    hashClient =                getHashClientParam(L, 2);

    status = prvCpssDxChTrunkHashBitsSelectionGet(
        devNum, hashClient, &startBit, &numOfBits);

    if (status != GT_OK)
    {
        lua_pushinteger(L, status);
        return 1;
    }

    lua_pushinteger(L, status);
    lua_pushinteger(L, startBit);
    lua_pushinteger(L, numOfBits);
    return 3;
}


