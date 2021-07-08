/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtDbDxAc3x_trunk_phy1690_WA_B.c
*
* @brief Trunk WA 'B' related to phy1690 on cascade system (eDSA).
* This WA based on reassign by TTI action the trunkId to globalEPort that
* represents it.
* the trunk source filtering is based on EPCL on the trunk members.
* the application responsible to not use the trunkId interface and used
* the globalEPort in it's place.
*
* @version   1
********************************************************************************
*/

/***********************************************************/
/***************** trunk phy1690_WA_B **********************/
/***********************************************************/
/*

***********************
    Next is the 'WA init' function
***********************

GT_STATUS cpssDxChTrunkInit_phy1690_WA_B
(
    IN GT_U8                    devNum,
    IN GT_U32                   firstGlobalEportForTrunkId1,
    IN GT_U32                   numEports
);

***********************
    Next are the 'wrappers for trunk members manipulations:
***********************

GT_STATUS cpssDxChTrunkMemberRemove_phy1690_WA_B
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);
GT_STATUS cpssDxChTrunkMemberAdd_phy1690_WA_B
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);
GT_STATUS cpssDxChTrunkMembersSet_phy1690_WA_B
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfEnabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    enabledMembersArray[],
    IN GT_U32                   numOfDisabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    disabledMembersArray[]
);

*/

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>

static GT_U32   firstGlobalEportForTrunkId1_phy1690_WA_B = 513;
static GT_U32   numEports_phy1690_WA_B = 63;

typedef enum
{
    GT_REMOVE = 0,
    GT_ADD    = 1
} GT_ADD_OR_REMOVE;

/* convert the trunkId to the global EPort that represents it */
GT_STATUS trunkEPortGet_phy1690_WA_B(
    IN GT_TRUNK_ID              trunkId,
    OUT GT_PORT_NUM              *globalEPortPtr
)
{
    if(trunkId == 0)
    {
        cpssOsPrintf("ERROR : trunkId[%d] not valid \n",
            trunkId);
        return GT_BAD_PARAM;
    }

    if(trunkId > numEports_phy1690_WA_B)
    {
        cpssOsPrintf("ERROR : trunkId[%d] > numEports_phy1690_WA_B [%d] \n",
            trunkId , numEports_phy1690_WA_B);
        return GT_OUT_OF_RANGE;
    }

    *globalEPortPtr = firstGlobalEportForTrunkId1_phy1690_WA_B + (trunkId-1);
    return GT_OK;
}



typedef struct{
    GT_U32                               udbIndex;
    CPSS_DXCH_TTI_OFFSET_TYPE_ENT        offsetType;
    GT_U8                                offset;
    GT_U8                                byteMask;
}TTI_UDB_STC;
#define TTI_BIT_IS_TRUNK_CNS                   181
#define TTI_BITS_SRC_TRUNK_SECOND_BYTE_CNS     216
#define TTI_BITS_SRC_TRUNK_FIRST_BYTE_CNS      208
/*
    the UDBs of the TTI :
    Need to check:
    1. 181      --> Local Device Source is Trunk --> must be '1'
       BYTE 22 , bit 5
    2. 222:208 Local Device Source ePort/TrunkID --> must be 'trunkId' of the ingress port
       BYTE 27 , bit 0..6
       BYTE 26 , bit 0..7
*/
static TTI_UDB_STC ttiUdbInfo[] = {
     {0, CPSS_DXCH_TTI_OFFSET_METADATA_E , TTI_BIT_IS_TRUNK_CNS               / 8 , BIT_5}
    ,{1, CPSS_DXCH_TTI_OFFSET_METADATA_E , TTI_BITS_SRC_TRUNK_SECOND_BYTE_CNS / 8 , 0x7F}
    ,{2, CPSS_DXCH_TTI_OFFSET_METADATA_E , TTI_BITS_SRC_TRUNK_FIRST_BYTE_CNS  / 8 , 0xFF}
    /* must be last */
    ,{0, CPSS_DXCH_TTI_OFFSET_INVALID_E , 0}
};

/* set during initialization */
CPSS_DXCH_TTI_RULE_UNT tti_maskInfo;
/* set during initialization */
CPSS_DXCH_TTI_ACTION_UNT tti_actionInfo;


typedef struct{
    GT_U32                               udbIndex;
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT        offsetType;
    GT_U8                                offset;
    GT_U8                                byteMask;
}EPCL_UDB_STC;

#define EPCL_BIT_ROUTED_CNS                          320
#define EPCL_BITS_ORIG_SRC_EPORT_SECOND_BYTE_CNS     72
#define EPCL_BITS_ORIG_SRC_EPORT_FIRST_BYTE_CNS      64
#define EPCL_BITS_LOCAL_TRG_PORT_CNS                 96
/*
    the UDBs of the EPCL :
    1. Only UDBs 30-49 may be configured with metadata anchor type.

    Need to check:

    320     --> Routed --> must be 0 (meaning 'not routed')
                BYTE 40 , bit 0
    79:67   --> Orig Src ePort or Trunk ID --> must be the global eport of the trunk that the port is member in
                BYTE 9 , bits 0..7
                BYTE 8 , bits 3..7
    66      --> Orig Src Is Trunk --> must be 0 (meaning 'not from trunk' ... since we check 'as eport')
                BYTE 8 , bit  2

    103:96  --> Local Dev Trg PHY Port --> must be the 'egress port'
                BYTE 16 , bit  0..7
*/
/*1. Only UDBs 30-49 may be configured with metadata anchor type.*/
#define EPCL_META_DATA_FIRST_UDB_CNS    30
static EPCL_UDB_STC epclUdbInfo[] = {
     {EPCL_META_DATA_FIRST_UDB_CNS+0, CPSS_DXCH_PCL_OFFSET_METADATA_E , EPCL_BIT_ROUTED_CNS                      / 8,BIT_0}
    ,{EPCL_META_DATA_FIRST_UDB_CNS+1, CPSS_DXCH_PCL_OFFSET_METADATA_E , EPCL_BITS_ORIG_SRC_EPORT_SECOND_BYTE_CNS / 8,0xFF}
    ,{EPCL_META_DATA_FIRST_UDB_CNS+2, CPSS_DXCH_PCL_OFFSET_METADATA_E , EPCL_BITS_ORIG_SRC_EPORT_FIRST_BYTE_CNS  / 8,0xFC}/*include bit 2 of 'Orig Src Is Trunk'*/
    ,{EPCL_META_DATA_FIRST_UDB_CNS+3, CPSS_DXCH_PCL_OFFSET_METADATA_E , EPCL_BITS_LOCAL_TRG_PORT_CNS             / 8,0xFF}

    /* must be last */
    ,{0, CPSS_DXCH_PCL_OFFSET_INVALID_E , 0}
};

static CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT egressRuleFormat = CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E;

/* set during initialization */
static CPSS_DXCH_PCL_ACTION_STC      epcl_actionInfo;
/* set during initialization */
static CPSS_DXCH_PCL_RULE_FORMAT_UNT epcl_maskInfo;


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
/*
*       Get absolute TTI rules index bases in TCAM for TTI
*       the 'relative index' from end of entries according to
*       appDemoDxChTcamTtiNumOfIndexsGet()
*/
static GT_U32 getTtiRuleIndex(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId
)
{
    GT_U32 numIndexGet = appDemoDxChTcamTtiNumOfIndexsGet(devNum,0);
    GT_U32 baseIndex   = appDemoDxChTcamTtiBaseIndexGet(devNum,0);
    GT_U32 relativeTcamEntryIndex;
    GT_U32 tmpIndex;

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E ||
        PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E ||
        PRV_CPSS_PP_MAC(devNum)->devType   == CPSS_98DX8332_Z0_CNS)
    {
        /* Aldrin/xCat3x has 3 floors those have default partition:
           Floor 0 - PCL
           Floor 1, blocks 0..2 TTI_0, blocks 3..5 TTI_1
           Floor 2, blocks 0..2 TTI_2, blocks 3..5 TTI_3
           TCAM block is two TCAM arrays.
           Index must be aligned with proper block.
           e.g. Indexes for TTI_0 are 0..5, 12..17, 24..29 ...
           baseIndex points to first entry of TTI lookup */
        /* there are 6 '10B' rules per line in the tcam for HIT '0' */
        tmpIndex = (trunkId / 6) * 12 + (trunkId % 6);

        /* since we go in reverse order need to remove 6 from the size of 'numIndexGet' */
        numIndexGet -= 6;
    }
    else
    {
        tmpIndex = trunkId;
    }

    /* each trunk with it's rule ...set reverse order from last index */
    relativeTcamEntryIndex = (numIndexGet -1) - tmpIndex;

    return  baseIndex + (relativeTcamEntryIndex % numIndexGet);
}

/* TTI : add the TTI rules for specific trunk */
static GT_STATUS trunkTtiRulesInit_phy1690_WA_B(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_PORT_NUM              globalEPort
)
{
    GT_STATUS   rc;
    CPSS_DXCH_TTI_RULE_UNT tti_patternInfo;
    GT_U32 ruleIndex;

    /* get the unique index for the ingress trunkId */
    ruleIndex = getTtiRuleIndex(devNum,trunkId);

    cpssOsMemSet(&tti_patternInfo, 0, sizeof(tti_patternInfo));

    /*
        1. 181      --> Local Device Source is Trunk --> must be '1'
           BYTE 22 , bit 5
    */
    tti_patternInfo.udbArray[0] = BIT_5;
    /*
        2. 222:208 Local Device Source ePort/TrunkID --> must be 'trunkId' of the ingress port
           BYTE 27 , bit 0..6
           BYTE 26 , bit 0..7
    */
    tti_patternInfo.udbArray[1] = (GT_U8)(trunkId >> 8);
    tti_patternInfo.udbArray[2] = (GT_U8)(trunkId);

    /* the action need to reassign the global eport for those that cam
       from the trunk */
    tti_actionInfo.type2.sourceEPort = globalEPort;

    /* the port added to the trunk .. need a rule for src EPort re-assign */
    rc = cpssDxChTtiRuleSet(devNum,ruleIndex,CPSS_DXCH_TTI_RULE_UDB_10_E,
            &tti_patternInfo,
            &tti_maskInfo,
            CPSS_DXCH_TTI_ACTION_TYPE2_ENT,
            &tti_actionInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

extern GT_U32 appDemoDxChTcamEpclBaseIndexGet
(
    IN     GT_U8           devNum
);
extern GT_U32 appDemoDxChTcamEpclNumOfIndexsGet
(
    IN     GT_U8           devNum
);

/*
*       Get absolute EPCL rules index bases in TCAM for EPCL
*       the 'relative index' from end of entries according to
*       appDemoDxChTcamEpclNumOfIndexsGet()
*/
static GT_U32 getEpclRuleIndex(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     localPortNum
)
{
    GT_U32 numIndexGet = appDemoDxChTcamEpclNumOfIndexsGet(devNum);
    GT_U32 baseIndex   = appDemoDxChTcamEpclBaseIndexGet(devNum);
    GT_U32 relativeTcamEntryIndex;

    /* each port with it's rule ...set reverse order from last index */
    relativeTcamEntryIndex = (numIndexGet -1) - localPortNum;

    return  baseIndex + (relativeTcamEntryIndex % numIndexGet);
}

/* EPCL : ADD/REMOVE local port configurations  */
static GT_STATUS trunkEpclLocalMember_phy1690_WA_B(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              globalEPort,
    IN GT_PHYSICAL_PORT_NUM     localPortNum,
    IN GT_ADD_OR_REMOVE         addOrRemove
)
{
    GT_STATUS   rc;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT epcl_patternInfo;
    GT_U32 ruleIndex;

    /* get the unique index for the egress port */
    ruleIndex = getEpclRuleIndex(devNum,localPortNum);

    if(addOrRemove == GT_REMOVE)
    {
        /* the port removed from the trunk .. not need the rule */
        rc = cpssDxChPclRuleInvalidate(devNum,0/*ruleSize --sip5 not used*/,
            ruleIndex);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        cpssOsMemSet(&epcl_patternInfo, 0, sizeof(epcl_patternInfo));

        /*
            320     --> Routed --> must be 0 (meaning 'not routed')
                        BYTE 40 , bit 0
        */
        epcl_patternInfo.ruleEgrUdbOnly.udb[0] = 0;
        /*
            71:67   --> Orig Src ePort or Trunk ID --> must be the global eport of the trunk that the port is member in
                        BYTE 9 , bits 0..7
                        BYTE 8 , bits 3..7
            66      --> Orig Src Is Trunk --> must be 0 (meaning 'not from trunk' ... since we check 'as eport')
                        BYTE 8 , bit  2
        */
        epcl_patternInfo.ruleEgrUdbOnly.udb[1] = (GT_U8)(globalEPort >> 5);
        epcl_patternInfo.ruleEgrUdbOnly.udb[2] = (GT_U8)(globalEPort << 3);
        /*
            103:96  --> Local Dev Trg PHY Port --> must be the 'egress port'
                        BYTE 16 , bit  0..7
        */
        epcl_patternInfo.ruleEgrUdbOnly.udb[3] = (GT_U8)(localPortNum);


        /* the port added to the trunk .. need a rule for egress filtering
            (replace the 'non-trunk' filter) */
        rc = cpssDxChPclRuleSet(devNum,egressRuleFormat,ruleIndex,
            0,/*ruleOptionsBmp,*/
            &epcl_maskInfo,
            &epcl_patternInfo,
            &epcl_actionInfo);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/* ADD/REMOVE local port configurations */
static GT_STATUS trunkLocalPortMember_phy1690_WA_B(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              globalEPort,
    IN GT_PHYSICAL_PORT_NUM     localPortNum,
    IN GT_ADD_OR_REMOVE         addOrRemove
)
{
    GT_STATUS rc;

    /* call CPSS for EPCL rules (filter egress on trunk member for non-routed
       that ingress from the global eport (of 'this' trunk) ---
       replace the 'non-trunk' filter )*/
    rc = trunkEpclLocalMember_phy1690_WA_B(devNum,globalEPort,localPortNum ,addOrRemove);
    if(rc != GT_OK)
    {
        return rc;
    }

    return rc;
}

/**
* @internal cpssDxChTrunkMembersSet_phy1690_WA_B function
* @endinternal
*
* @brief   part of trunk WA 'B' due to phy1690.
*         set trunk members including relevant WA configurations.
*         Add an associated EPCL rules in case of new local trunk members (for source filtering).
*         Remove the associated EPCL rules in case of removed local trunk members.
*         call CPSS API : cpssDxChTrunkMembersSet(...)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] trunkId                  - trunk id
* @param[in] enabledMembersArray[]    - (array of) members to set in this trunk as enabled
*                                      members .
*                                      (this parameter ignored if numOfEnabledMembers = 0)
* @param[in] numOfEnabledMembers      - number of enabled members in the array.
* @param[in] disabledMembersArray[]   - (array of) members to set in this trunk as disabled
*                                      members .
*                                      (this parameter ignored if numOfDisabledMembers = 0)
* @param[in] numOfDisabledMembers     - number of disabled members in the array.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkMembersSet_phy1690_WA_B
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfEnabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    enabledMembersArray[],
    IN GT_U32                   numOfDisabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    disabledMembersArray[]
)
{
    GT_STATUS   rc;
    GT_U32  ii;
    GT_PORT_NUM  globalEPort;
    GT_HW_DEV_NUM   localDev_hwDevNum;
    CPSS_TRUNK_MEMBER_STC   *currentMemberPtr;
    CPSS_PORTS_BMP_STC      old_nonTrunkPorts,old_TrunkPortsBmp;
    CPSS_PORTS_BMP_STC      new_nonTrunkPorts,new_TrunkPortsBmp;
    CPSS_PORTS_BMP_STC      fullLocalPortsBmp;
    CPSS_PORTS_BMP_STC      removedLocalPortsBmp;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    localDev_hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);

    /* get the 'old' 'non-ports' of the trunk */
    /* used later to remove configurations from local ports that are not in the
       trunk any more */
    rc = cpssDxChTrunkNonTrunkPortsEntryGet(devNum,trunkId,&old_nonTrunkPorts);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* get the globalEport that represents the trunkId*/
    rc = trunkEPortGet_phy1690_WA_B(trunkId,&globalEPort);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* loop on enabled ports */
    for(ii = 0 ; ii < numOfEnabledMembers; ii++)
    {
        currentMemberPtr = &enabledMembersArray[ii];
        if(currentMemberPtr->hwDevice != localDev_hwDevNum)
        {
            /* the WA not relevant to non local device members */
            continue;
        }

        /* set needed rules for the added local port to the trunk */
        rc = trunkLocalPortMember_phy1690_WA_B(devNum,globalEPort,
            currentMemberPtr->port,
            GT_ADD);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* loop on disabled ports */
    for(ii = 0 ; ii < numOfDisabledMembers; ii++)
    {
        currentMemberPtr = &disabledMembersArray[ii];
        if(currentMemberPtr->hwDevice != localDev_hwDevNum)
        {
            /* the WA not relevant to non local device members */
            continue;
        }

        /* set needed rules for the added local port to the trunk */
        rc = trunkLocalPortMember_phy1690_WA_B(devNum,globalEPort,
            currentMemberPtr->port,
            GT_ADD);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* call CPSS for the trunk members */
    rc = cpssDxChTrunkMembersSet(devNum,trunkId,
        numOfEnabledMembers,enabledMembersArray,
        numOfDisabledMembers,disabledMembersArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* get the 'new' 'non-ports' of the trunk */
    rc = cpssDxChTrunkNonTrunkPortsEntryGet(devNum,trunkId,&new_nonTrunkPorts);
    if(rc != GT_OK)
    {
        return rc;
    }


    CPSS_PORTS_BMP_PORT_SET_ALL_MAC(&fullLocalPortsBmp);
    /* adjusted physical ports of BMP to hold only bmp of existing ports*/
    rc = prvCpssDxChTablePortsBmpAdjustToDevice(devNum , &fullLocalPortsBmp , &fullLocalPortsBmp);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* new_TrunkPortsBmp = fullLocalPortsBmp & ~(new_nonTrunkPorts) */
    CPSS_PORTS_BMP_BITWISE_AND_NOT_MAC(&new_TrunkPortsBmp,
                                       &fullLocalPortsBmp,
                                       &new_nonTrunkPorts);
    /* old_TrunkPortsBmp = fullLocalPortsBmp & ~(new_nonTrunkPorts) */
    CPSS_PORTS_BMP_BITWISE_AND_NOT_MAC(&old_TrunkPortsBmp,
                                       &fullLocalPortsBmp,
                                       &old_nonTrunkPorts);

    /* removedLocalPortsBmp = old_TrunkPortsBmp & ~(new_TrunkPortsBmp) */
    CPSS_PORTS_BMP_BITWISE_AND_NOT_MAC(&removedLocalPortsBmp,
                                       &old_TrunkPortsBmp,
                                       &new_TrunkPortsBmp);

    /* remove ports from old that not exists any more */
    for(ii = 0 ; ii < PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum); ii++)
    {
        if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&removedLocalPortsBmp,ii))
        {
            continue;
        }

        /* unset needed rules for the removed local port from the trunk */
        rc = trunkLocalPortMember_phy1690_WA_B(devNum,globalEPort,ii,
            GT_REMOVE);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChTrunkMemberAdd_phy1690_WA_B function
* @endinternal
*
* @brief   part of trunk WA 'B' due to phy1690.
*         add trunk member including relevant WA configurations.
*         Add an associated EPCL rule in case of local trunk member (for source filtering).
*         call CPSS API : cpssDxChTrunkMemberAdd(...)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - the trunkId
* @param[in] memberPtr                - (pointer to) the trunk member
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkMemberAdd_phy1690_WA_B
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    GT_STATUS       rc;
    GT_PORT_NUM  globalEPort;
    GT_HW_DEV_NUM   localDev_hwDevNum;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    localDev_hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);

    if(memberPtr->hwDevice == localDev_hwDevNum)
    {
        /* get the globalEport that represents the trunkId*/
        rc = trunkEPortGet_phy1690_WA_B(trunkId,&globalEPort);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* set needed rules for the added local port to the trunk */
        rc = trunkLocalPortMember_phy1690_WA_B(devNum,globalEPort,memberPtr->port,
            GT_ADD);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    rc = cpssDxChTrunkMemberAdd(devNum,trunkId,memberPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChTrunkMemberRemove_phy1690_WA_B function
* @endinternal
*
* @brief   part of trunk WA 'B' due to phy1690.
*         remove trunk member including relevant WA configurations.
*         invalidate the associated EPCL rule in case of local trunk member.
*         call CPSS API : cpssDxChTrunkMemberRemove(...)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - the trunkId
* @param[in] memberPtr                - (pointer to) the trunk member
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkMemberRemove_phy1690_WA_B
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    GT_STATUS       rc;
    GT_PORT_NUM  globalEPort;
    GT_HW_DEV_NUM   localDev_hwDevNum;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    localDev_hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);

    if(memberPtr->hwDevice == localDev_hwDevNum)
    {
        /* get the globalEport that represents the trunkId*/
        rc = trunkEPortGet_phy1690_WA_B(trunkId,&globalEPort);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* unset needed rules for the removed local port from the trunk */
        rc = trunkLocalPortMember_phy1690_WA_B(devNum,globalEPort,memberPtr->port,
            GT_REMOVE);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    rc = cpssDxChTrunkMemberRemove(devNum,trunkId,memberPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}


/* init the TTI */
static GT_STATUS trunkTtiInit_phy1690_WA_B
(
    IN GT_U8                    devNum
)
{
    GT_STATUS   rc;
    GT_PORT_NUM             globalEport;
    GT_TRUNK_ID             trunkId;
    GT_PHYSICAL_PORT_NUM      portNum;
    CPSS_DXCH_TTI_KEY_TYPE_ENT ttiKey;
    TTI_UDB_STC *ttiUdbPtr;
    CPSS_DXCH_TTI_KEY_SIZE_ENT  ttiKeySize = CPSS_DXCH_TTI_KEY_SIZE_10_B_E;
    GT_U32  ii;

    /************************************************/
    /* allow all ports to do TTI lookup on all keys */
    /************************************************/
    for(portNum = 0; portNum < PRV_CPSS_DXCH_MAX_DEFAULT_EPORT_NUMBER_MAC(devNum); portNum++)
    {
        for(ttiKey = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E ; ttiKey <= CPSS_DXCH_TTI_KEY_UDB_UDE6_E ; ttiKey++)
        {
            rc = cpssDxChTtiPortLookupEnableSet(devNum,portNum,ttiKey,GT_TRUE);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }


    /****************/
    /* set TTI UDBs */
    /****************/
    for(ii = 0 ; ttiUdbInfo[ii].offsetType != CPSS_DXCH_TTI_OFFSET_INVALID_E ;ii++)
    {
        ttiUdbPtr = &ttiUdbInfo[ii];
        for(ttiKey = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E ; ttiKey <= CPSS_DXCH_TTI_KEY_UDB_UDE6_E ; ttiKey++)
        {
            rc = cpssDxChTtiUserDefinedByteSet(devNum,ttiKey,
                ttiUdbPtr->udbIndex,
                ttiUdbPtr->offsetType,
                ttiUdbPtr->offset);
            if(rc != GT_OK)
            {
                return rc;
            }

        }
    }

    /* set the TTI to do 10B key lookups (on all keys) */
    for(ttiKey = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E ; ttiKey <= CPSS_DXCH_TTI_KEY_UDB_UDE6_E ; ttiKey++)
    {
        rc = cpssDxChTtiPacketTypeKeySizeSet(devNum,ttiKey,ttiKeySize);
        if(rc != GT_OK)
        {
            return rc;
        }
    }


    /***********************/
    /* init the TTI action */
    /***********************/
    cpssOsMemSet(&tti_actionInfo, 0, sizeof(tti_actionInfo));

    tti_actionInfo.type2.command = CPSS_PACKET_CMD_FORWARD_E ;
    tti_actionInfo.type2.sourceEPortAssignmentEnable  = GT_TRUE;
    tti_actionInfo.type2.keepPreviousQoS              = GT_TRUE;
    tti_actionInfo.type2.tag1VlanCmd                  = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;

    /**********************/
    /* init the TTI mask */
    /**********************/
    cpssOsMemSet(&tti_maskInfo, 0, sizeof(tti_maskInfo));

    for(ii = 0 ; ttiUdbInfo[ii].offsetType != CPSS_DXCH_TTI_OFFSET_INVALID_E ;ii++)
    {
        ttiUdbPtr = &ttiUdbInfo[ii];

        tti_maskInfo.udbArray[ii] = ttiUdbPtr->byteMask;
    }

    trunkId = 1;

    for(globalEport = firstGlobalEportForTrunkId1_phy1690_WA_B;
        globalEport < (firstGlobalEportForTrunkId1_phy1690_WA_B + numEports_phy1690_WA_B);
        globalEport++,trunkId++)
    {
        rc = trunkTtiRulesInit_phy1690_WA_B(devNum,trunkId,globalEport);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/* init the EPCL */
static GT_STATUS trunkEpclInit_phy1690_WA_B
(
    IN GT_U8                    devNum
)
{
    GT_STATUS   rc;
    GT_HW_DEV_NUM   localDev_hwDevNum;
    GT_PHYSICAL_PORT_NUM      portNum;
    CPSS_INTERFACE_INFO_STC   interfaceInfo;
    CPSS_PCL_DIRECTION_ENT    epclDirection = CPSS_PCL_DIRECTION_EGRESS_E;
    CPSS_DXCH_PCL_LOOKUP_CFG_STC    lookupCfg;
    CPSS_DXCH_PCL_UDB_SELECT_STC    udbSelect;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT   packetType;
    EPCL_UDB_STC *epclUdbPtr;
    GT_U32  ii;

    localDev_hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);
    /*********************************************************/
    /* allow all ports to do EPCL lookup on 'TS' and non-TS' */
    /*********************************************************/
    rc = cpssDxCh2PclEgressPolicyEnable(devNum, GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.pclId                  = 0;
    lookupCfg.dualLookup             = GT_FALSE;
    lookupCfg.pclIdL01               = 0;
    lookupCfg.groupKeyTypes.nonIpKey = egressRuleFormat;
    lookupCfg.groupKeyTypes.ipv4Key  = egressRuleFormat;
    lookupCfg.groupKeyTypes.ipv6Key  = egressRuleFormat;

    interfaceInfo.type = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum = localDev_hwDevNum;

    for(portNum = 0; portNum < PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum); portNum++)
    {
        rc = cpssDxCh2EgressPclPacketTypesSet(devNum, portNum,
            CPSS_DXCH_PCL_EGRESS_PKT_TS_E, GT_TRUE);
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxCh2EgressPclPacketTypesSet(devNum, portNum,
            CPSS_DXCH_PCL_EGRESS_PKT_NON_TS_E, GT_TRUE);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* configure egress configuration table to be accessed by 'Port mode' */
        rc = cpssDxChPclPortLookupCfgTabAccessModeSet(
            devNum, portNum, epclDirection,
            CPSS_PCL_LOOKUP_0_E, 0/*sublookup*/,
            CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
        if(rc != GT_OK)
        {
            return rc;
        }

        interfaceInfo.devPort.portNum = portNum;

        /* set the egress configuration table entry to allow 10B key lookups */
        rc = cpssDxChPclCfgTblSet(devNum, &interfaceInfo, epclDirection,
            CPSS_PCL_LOOKUP_0_E,
            &lookupCfg);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /*****************/
    /* set EPCL UDBs */
    /*****************/
    for(packetType = 0 ; packetType < CPSS_DXCH_PCL_PACKET_TYPE_LAST_E ;packetType++)
    {
        if(packetType == CPSS_DXCH_PCL_PACKET_TYPE_IPV6_E)
        {
            /* no such for SIP5 devices */
            continue;
        }
        for(ii = 0 ; epclUdbInfo[ii].offsetType != CPSS_DXCH_PCL_OFFSET_INVALID_E ;ii++)
        {
            epclUdbPtr = &epclUdbInfo[ii];

            rc = cpssDxChPclUserDefinedByteSet(devNum,
                0,/*ruleFormat - ignored sip5 */
                packetType,/*packetType */
                epclDirection,/*direction*/
                epclUdbPtr->udbIndex,
                epclUdbPtr->offsetType,
                epclUdbPtr->offset
            );

            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    cpssOsMemSet(&udbSelect, 0, sizeof(udbSelect));
    for(ii = 0 ; ii < 10; ii++)
    {
        udbSelect.udbSelectArr[ii] = EPCL_META_DATA_FIRST_UDB_CNS + ii;
    }

    /********************************************/
    /* map the EPCL UDBs indexes to proper UDBs */
    /********************************************/
    for(packetType = 0 ; packetType < CPSS_DXCH_PCL_PACKET_TYPE_LAST_E ;packetType++)
    {
        if(packetType == CPSS_DXCH_PCL_PACKET_TYPE_IPV6_E)
        {
            /* no such for SIP5 devices */
            continue;
        }

        rc = cpssDxChPclUserDefinedBytesSelectSet(devNum,
            egressRuleFormat, packetType, CPSS_PCL_LOOKUP_0_E,
            &udbSelect);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /************************/
    /* init the EPCL action */
    /************************/
    cpssOsMemSet(&epcl_actionInfo, 0, sizeof(epcl_actionInfo));

    epcl_actionInfo.egressPolicy          = GT_TRUE;
    epcl_actionInfo.pktCmd                = CPSS_PACKET_CMD_DROP_HARD_E;

    /**********************/
    /* init the EPCL mask */
    /**********************/
    cpssOsMemSet(&epcl_maskInfo, 0, sizeof(epcl_maskInfo));

    for(ii = 0 ; epclUdbInfo[ii].offsetType != CPSS_DXCH_PCL_OFFSET_INVALID_E ;ii++)
    {
        epclUdbPtr = &epclUdbInfo[ii];

        epcl_maskInfo.ruleEgrUdbOnly.udb[ii] = epclUdbPtr->byteMask;
    }

    return GT_OK;
}
/* init the E2PHY */
static GT_STATUS trunkE2PhyInit_phy1690_WA_B
(
    IN GT_U8                    devNum
)
{
    GT_STATUS   rc;
    GT_PORT_NUM portNum;
    CPSS_INTERFACE_INFO_STC physicalInfo;
    GT_TRUNK_ID             trunkId;

    trunkId = 1;
    physicalInfo.type = CPSS_INTERFACE_TRUNK_E;

    /***********************************************/
    /* map the Global EPort's to physical trunkIds */
    /***********************************************/

    for(portNum = firstGlobalEportForTrunkId1_phy1690_WA_B;
        portNum < (firstGlobalEportForTrunkId1_phy1690_WA_B + numEports_phy1690_WA_B);
        portNum++,trunkId++)
    {
        physicalInfo.trunkId = trunkId;

        rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,portNum,
            &physicalInfo);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}
/* init defaults on the global eports */
static GT_STATUS trunkEportInit_phy1690_WA_B
(
    IN GT_U8                    devNum
)
{
    GT_STATUS   rc;
    GT_PORT_NUM portNum;

    /***********************************/
    /* set defaults on Global EPort's  */
    /***********************************/
    for(portNum = firstGlobalEportForTrunkId1_phy1690_WA_B;
        portNum < (firstGlobalEportForTrunkId1_phy1690_WA_B + numEports_phy1690_WA_B);
        portNum++)
    {
        /* NOTE: not needed on 'auto learning' */
        /* allow the NA messages to go to CPU for controlled learning */
        rc = cpssDxChBrgFdbNaToCpuPerPortSet(devNum,portNum,GT_TRUE);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChTrunkInit_phy1690_WA_B function
* @endinternal
*
* @brief   part of trunk WA 'B' due to phy1690.
*         init the trunk WA
*         set the first global EPort that will represent trunkId = 1
*         set the number of global eports that are allocated for those trunks.
*         set TTI,EPCL lookup enable,UDBs.
*         set TTI rules - one per trunk.
*         set E2PHY - mapping the needed global EPorts to 'physical' trunkId.
*         assuming :
*         1. application set range for 'GLOBAL' EPORTs
*         2. application must call cpssDxChTrunkInit(...)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] firstGlobalEportForTrunkId1 - the first global EPort that will represent trunkId = 1
* @param[in] numEports                - the number of global eports that are allocated for those trunks.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkInit_phy1690_WA_B
(
    IN GT_U8                    devNum,
    IN GT_U32                   firstGlobalEportForTrunkId1,
    IN GT_U32                   numEports
)
{
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    firstGlobalEportForTrunkId1_phy1690_WA_B = firstGlobalEportForTrunkId1;
    numEports_phy1690_WA_B = numEports;

    /************************/
    /* init the TTI related */
    /************************/
    rc = trunkTtiInit_phy1690_WA_B(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*************************/
    /* init the EPCL related */
    /*************************/
    rc = trunkEpclInit_phy1690_WA_B(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /**************************/
    /* init the E2PHY mapping */
    /**************************/
    rc = trunkE2PhyInit_phy1690_WA_B(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /**************************************/
    /* init defaults on the global eports */
    /**************************************/
    rc = trunkEportInit_phy1690_WA_B(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }


    return GT_OK;
}




