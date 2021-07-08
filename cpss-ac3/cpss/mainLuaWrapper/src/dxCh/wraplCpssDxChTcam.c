/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplCpssDxChTcam.c
*
* DESCRIPTION:
*       A lua wrapper for TCAM functions
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*******************************************************************************/

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>


#define QUIT_IF_NOT_OK(value)           \
    if ((value) != GT_OK)                       \
    {                                           \
        lua_pushnumber(L, (value));             \
        return 1;                               \
    }

#define NUM_RULES_IN_FLOOR_MAC(_devNum)     CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS

/* macro to roundup a number that is divided so :
    for example :
    3 / 2 is round up to 2 and not round down to 1 */
#define ROUNDUP_DIV_MAC(_number , _divider)             \
    (((_number) + ((_divider)-1)) / (_divider))

/* get the number of floors needed for the number of rules in the tcam */
#define CPSS_DXCH_TCAM_MAX_NUM_FLOORS_MAC(_devNum)                          \
    ROUNDUP_DIV_MAC(                                                        \
        PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.tunnelTerm,     \
        NUM_RULES_IN_FLOOR_MAC(_devNum))

/***** declarations ********/

use_prv_enum(CPSS_DXCH_TCAM_CLIENT_ENT);
use_prv_struct(CPSS_DXCH_TCAM_BLOCK_INFO_STC);

/***** declarations ********/


/*******************************************************************************
* wrlCpssDxChTcamNumBanksGet
*
* DESCRIPTION:
*   The function gets
*
* APPLICABLE DEVICES:
*        Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        dxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* INPUTS:
*       L                   - lua state
*           1:GT_U8                     devNum      device number
*           2.CPSS_DXCH_TCAM_CLIENT_ENT tcamClient  TCAM client
*
* OUTPUTS:
*       None
*
* RETURNS:
*       status, tcamGroup, entriesTotal, entriesUsed
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChTcamUtilizationGet
(
    IN lua_State *L
)
{
#define CHECK_STATUS_OK() \
    if (status != GT_OK) \
    { \
        lua_pushinteger(L, (lua_Integer)status); \
        return 1; \
    }

    GT_STATUS                   status = GT_OK;
    GT_U8                       devNum = 0;
    GT_PORT_GROUPS_BMP          portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    CPSS_DXCH_TCAM_CLIENT_ENT   tcamClient = CPSS_DXCH_TCAM_IPCL_0_E;
    GT_U32                      tcamGroup;
    GT_BOOL                     tcamClientEnabled;
    GT_U32                      floorIndex, line, bank, index;
    GT_U32                      entriesTotal = 0;
    GT_U32                      entriesUsed = 0;
    CPSS_DXCH_TCAM_BLOCK_INFO_STC  floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];
    GT_U32                      i;
    GT_BOOL                     entryValid;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT ruleSize;
    GT_U32                      numOfActiveFloors;
    GT_U32                      numOfBlocks;
    GT_U32                      numOfRulesPerBlock;
    GT_U32                      numOfBanksInBlock;

    PARAM_NUMBER(status, devNum, 1, GT_U8);
    PARAM_ENUM(status, tcamClient, 2, CPSS_DXCH_TCAM_CLIENT_ENT);
    CHECK_STATUS_OK();

    status = cpssDxChTcamPortGroupClientGroupGet(devNum,
            portGroupsBmp, tcamClient,
            &tcamGroup, &tcamClientEnabled);
    CHECK_STATUS_OK();

    status = cpssDxChTcamActiveFloorsGet(devNum, &numOfActiveFloors);
    CHECK_STATUS_OK();
    numOfBlocks = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.numBanksForHitNumGranularity;
    numOfRulesPerBlock = CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS / numOfBlocks;
    numOfBanksInBlock = CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS / numOfBlocks;
    for (floorIndex = 0; floorIndex < numOfActiveFloors; floorIndex++)
    {
        status = cpssDxChTcamIndexRangeHitNumAndGroupGet(devNum, floorIndex, floorInfoArr);
        CHECK_STATUS_OK();

        for (i = 0; i < numOfBlocks; i++)
        {
            if (floorInfoArr[i].group != tcamGroup)
                continue;
            entriesTotal += numOfRulesPerBlock;
        }
        if (numOfBlocks == 6)
        {
            if ((floorInfoArr[0].group != tcamGroup) && (floorInfoArr[1].group != tcamGroup) && (floorInfoArr[2].group != tcamGroup)
                && (floorInfoArr[3].group != tcamGroup) && (floorInfoArr[4].group != tcamGroup) && (floorInfoArr[5].group != tcamGroup))
                continue;
        }
        else
        {
            if ((floorInfoArr[0].group != tcamGroup) && (floorInfoArr[1].group != tcamGroup))
                continue;
        }
        if (tcamClientEnabled != GT_TRUE)
            continue;
        for (line = 0; line < CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS; line++)
        {
            for (bank = 0; bank < CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS; bank++)
            {
                if (floorInfoArr[bank/numOfBanksInBlock].group != tcamGroup)
                    continue;
                index = floorIndex * CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS
                        + line * CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS
                        + bank;
                status = cpssDxChTcamPortGroupRuleValidStatusGet(
                        devNum, portGroupsBmp, index,
                        &entryValid, &ruleSize);
                if (status != GT_OK)
                    continue;
                if (entryValid != GT_TRUE)
                    continue;

                switch (ruleSize)
                {
                    case CPSS_DXCH_TCAM_RULE_SIZE_10_B_E:
                        entriesUsed += 1;
                        break;
                    case CPSS_DXCH_TCAM_RULE_SIZE_20_B_E:
                        entriesUsed += 2;
                        bank += 1;
                        break;
                    case CPSS_DXCH_TCAM_RULE_SIZE_30_B_E:
                        entriesUsed += 3;
                        bank += 2;
                        break;
                    case CPSS_DXCH_TCAM_RULE_SIZE_40_B_E:
                        entriesUsed += 4;
                        bank += 3;
                        break;
                    case CPSS_DXCH_TCAM_RULE_SIZE_50_B_E:
                        entriesUsed += 5;
                        bank += 4;
                        break;
                    case CPSS_DXCH_TCAM_RULE_SIZE_60_B_E:
                        entriesUsed += 6;
                        bank += 5;
                        break;
                    case CPSS_DXCH_TCAM_RULE_SIZE_80_B_E:
                        entriesUsed += 8;
                        bank += 7;
                        break;
                }

            }
        }
    }
    lua_pushinteger(L, (lua_Integer)status);
    lua_pushinteger(L, (lua_Integer)tcamGroup);
    lua_pushinteger(L, (lua_Integer)entriesTotal);
    lua_pushinteger(L, (lua_Integer)entriesUsed);
    return 4;
#undef CHECK_STATUS_OK
}

extern GT_U32 appDemoDxChTcamTtiBaseIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           hitNum
);
extern GT_U32 appDemoDxChTcamTtiNumOfIndexsGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           hitNum
);
/*******************************************************************************
* wrlCpssDxChTcamTtiHitInfoGet
*
* DESCRIPTION:
*   The function gets TCAM info for TTI hit sections
*
* APPLICABLE DEVICES:
*        Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        dxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* INPUTS:
*       L                   - lua state
*           1:GT_U8                     devNum      device number
*
* OUTPUTS:
*       None
*
* RETURNS:
*       status, hit0_base , hit0_size , hit1_base , hit1_size ,
*               hit2_base , hit2_size , hit3_base , hit3_size
*
* COMMENTS:
*
********************************************************************************/
int wrlCpssDxChTcamTtiHitInfoGet
(
    IN lua_State *L
)
{
    GT_U32      index = 0;/* index of out parameter */
#ifndef CLI_STANDALONE
    GT_STATUS   rc = GT_OK;
    GT_U8       devNum = 0;
    GT_U32      ii;

    PARAM_NUMBER(rc, devNum, 1, GT_U8);
    if (rc != GT_OK)
    {
        /* illegal input parameter */
        return 0; /* return nil */
    }
#endif

    lua_pushinteger(L, (lua_Integer)GT_OK);
    index++;

#ifndef CLI_STANDALONE
    for(ii = 0 ; ii < 4 ; ii++)
    {
        lua_pushinteger(L, (lua_Integer)appDemoDxChTcamTtiBaseIndexGet(devNum,ii));
        index++;
        lua_pushinteger(L, (lua_Integer)appDemoDxChTcamTtiNumOfIndexsGet(devNum,ii));
        index++;
    }
#endif

    return index;
}

/*******************************************************************************
* wrlCpssDxChTcamGeomGet
*
* DESCRIPTION:
*   Returns TCAM information for specified device :
*   number of floors, number of all items, number of blocks per floor
*
* APPLICABLE DEVICES:
*        dxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       L                   - lua state
*       L[1]                - device number
*
* OUTPUTS:
*       None
*
* RETURNS:
*       nil -  if an integer input parameter (device number) is not specified.
*       else return 3 parameters:
*         floorsNum - number of TCAM floors
*         itemsNum  - number of all TCAM items
*         blocksNum - number of block on a floor.
*
* COMMENTS:
*
********************************************************************************/
int wrlCpssDxChTcamGeomGet
(
    IN lua_State *L
)
{
    GT_U8 devNum;
    GT_U32 n;
    GT_STATUS rc = GT_OK;

    PARAM_NUMBER(rc, devNum, 1, GT_U8);
    if (rc != GT_OK)
    {
        return 0; /* return nil */
    }

    /* number of TCAM floors */
    n = CPSS_DXCH_TCAM_MAX_NUM_FLOORS_MAC(devNum);
    lua_pushinteger(L, (lua_Integer)n);

    /* number of blocks per floor */
    n = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.numBanksForHitNumGranularity;
    lua_pushinteger(L, (lua_Integer)n);

    /* total number of TCAM items */
    n = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelTerm;
    lua_pushinteger(L, (lua_Integer)n);

    return 3;
}


/*******************************************************************************
* wrlCpssDxChTcamIndexRangeHitNumAndGroupGet
*
* DESCRIPTION:
*       Select for each TCAM block a TCAM group and lookup number (hitNum)
*
* APPLICABLE DEVICES:
*        Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* INPUTS:
*       L       - lua state
*       l[1]    - the device number
*       l[2]    - floor index (APPLICABLE RANGES: 0..11)
*
*
* OUTPUTS:
*       None
*
* RETURNS:
*       if failed - an error code
*       if ok- two values: a GT_OK and a table (indexed from 0, not 1!)
*              appropriate to an out-paramter floorInfoArr  of underlying
*              cpss API cpssDxChTcamIndexRangeHitNumAndGroupGet.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
int wrlCpssDxChTcamIndexRangeHitNumAndGroupGet
(
    IN lua_State *L
)
{
    GT_STATUS                     rc = GT_OK;
    GT_U8                         devNum;
    GT_U32                        floorIndex;
    GT_U32                        numOfBlocks;
    GT_U32                        i;
    GT_U32                        t;
    CPSS_DXCH_TCAM_BLOCK_INFO_STC floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];



    PARAM_NUMBER(rc, devNum, 1, GT_U8);
    QUIT_IF_NOT_OK(rc);

    PARAM_NUMBER(rc, floorIndex, 2, GT_U32);
    QUIT_IF_NOT_OK(rc);

    rc = cpssDxChTcamIndexRangeHitNumAndGroupGet(devNum, floorIndex, floorInfoArr);
    QUIT_IF_NOT_OK(rc);

    lua_pushnumber(L, rc);      /* first returned value */
    /* get a number of blocks on floor */
    numOfBlocks = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.numBanksForHitNumGranularity;

    lua_createtable(L, numOfBlocks-1, 1); /* second returned value */
    t = lua_gettop(L);
    for (i=0; i < numOfBlocks; i++)
    {
        prv_c_to_lua_CPSS_DXCH_TCAM_BLOCK_INFO_STC(L, &floorInfoArr[i]);
        lua_rawseti(L, t, i);
    }
    return 2;
}
