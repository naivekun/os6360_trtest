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
* @file gtDbDxAc3x_trunk_phy1690_WA_C.c
*
* @brief Trunk WA 'C' related to phy1690 on cascade system (eDSA).
* This WA assumes that cascade port will NOT do FDB learning (not auto learning and
* not controlled learning).
* Disable NA to CPU on the cascade port.(otherwise we get storming of NA
* when FDB hold entry in trunkId but DSA came on same (MAC,vlan) with
* (remoteDe,port) of trunk member !)
* The trunk source filtering from cascade port originated from 'Other device'
* trunk members , using SRC-ID assignment (srcId = trunkId).
* (The trunk source filtering from local ports (non-cascade) done in native
* trunk mechanism --> the 'Non-trunk' entry)
* WA 'C' is much more simple for application than WA 'B'.
*
*
* @version   1
********************************************************************************
*/

/***********************************************************/
/***************** trunk phy1690_WA_C **********************/
/***********************************************************/
/*

***********************
    Next is the 'WA init' function
***********************

GT_STATUS cpssDxChTrunkInit_phy1690_WA_C
(
    IN GT_U8                    devNum
);

***********************
    Next is 'wrapper' for cascade port manipulation:
***********************
GT_STATUS cpssDxChCscdPortTypeSet_phy1690_WA_C
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN CPSS_PORT_DIRECTION_ENT      portDirection,
    IN CPSS_CSCD_PORT_TYPE_ENT      portType
);

***********************
    Next are the 'wrappers' for trunk members manipulations:
***********************

GT_STATUS cpssDxChTrunkMemberRemove_phy1690_WA_C
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);
GT_STATUS cpssDxChTrunkMemberAdd_phy1690_WA_C
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);
GT_STATUS cpssDxChTrunkMembersSet_phy1690_WA_C
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
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSrcId.h>

typedef enum
{
    GT_REMOVE = 0,
    GT_ADD    = 1
} GT_ADD_OR_REMOVE;

/* convert the trunkId to the SRC-ID that represents it */
GT_STATUS trunkSrcIdGet_phy1690_WA_C(
    IN GT_TRUNK_ID          trunkId,
    OUT GT_U32              *srcIdPtr
)
{
    *srcIdPtr = trunkId;
    return GT_OK;
}

typedef struct{
    GT_U32                               udbIndex;
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT        offsetType;
    GT_U8                                offset;
    GT_U8                                byteMask;
}IPCL_UDB_STC;

/* WA 'C'
    Rule per 'not-on-my-device' trunk member

    the UDBs of the IPCL :
    1. Only UDBs 30-49 may be configured with metadata anchor type.

    Need to check:
    1. 587:586 - Marvell Tagged Extended - must be '3' (16B eDSA)
        BYTE 73 bits 2..3
    2. 585     - Ingress Marvell Tagged - must be '1'
        BYTE 73 bit 1
    3. 341      - Routed - must be '0'
        BYTE 42 bit 5
    5. 79:67    - Orig Src ePort or Trunk ID - must be the physical port of the trunk member
        BYTE 9 , bits 0..7
        BYTE 8 , bits 3..7
    6. 66       - Orig Src Is Trunk - must be '0'
        BYTE 8 , bit 2
    7. 65:56    - Orig Src Dev  - must be the hwDevNum of the trunk member
        BYTE 8 , bits 0..1
        BYTE 7 , bits 0..7

    =====================
    used bytes summary  : 7,8,9,42,73 --> 5 bytes
*/
#define BYTE_7    7
#define BYTE_8    8
#define BYTE_9    9
#define BYTE_42  42
#define BYTE_73  73

/*1. Only UDBs 30-49 may be configured with metadata anchor type.*/
#define IPCL_META_DATA_FIRST_UDB_CNS    30
static IPCL_UDB_STC ipclUdbInfo[] = {
      {IPCL_META_DATA_FIRST_UDB_CNS+0, CPSS_DXCH_PCL_OFFSET_METADATA_E , BYTE_73,0x0E}
     ,{IPCL_META_DATA_FIRST_UDB_CNS+1, CPSS_DXCH_PCL_OFFSET_METADATA_E , BYTE_42,BIT_5}
     ,{IPCL_META_DATA_FIRST_UDB_CNS+2, CPSS_DXCH_PCL_OFFSET_METADATA_E , BYTE_9,0xFF}
     ,{IPCL_META_DATA_FIRST_UDB_CNS+3, CPSS_DXCH_PCL_OFFSET_METADATA_E , BYTE_8,0xFF}
     ,{IPCL_META_DATA_FIRST_UDB_CNS+4, CPSS_DXCH_PCL_OFFSET_METADATA_E , BYTE_7,0xFF}

    /* must be last */
    ,{0, CPSS_DXCH_PCL_OFFSET_INVALID_E , 0}
};

static CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ingressRuleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E;
static CPSS_PCL_DIRECTION_ENT      ipclDirection = CPSS_PCL_DIRECTION_INGRESS_E;

/* set during initialization */
static CPSS_DXCH_PCL_ACTION_STC      ipcl_actionInfo;
/* set during initialization */
static CPSS_DXCH_PCL_RULE_FORMAT_UNT ipcl_maskInfo;


static GT_U32  ipcl_lookupId = CPSS_DXCH_TCAM_IPCL_2_E;

extern GT_U32 appDemoDxChTcamIpclBaseIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           lookupId
);

extern GT_U32 appDemoDxChTcamIpclNumOfIndexsGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           lookupId
);

#define ERROR_TABLE_FULL_CNS 0xFFFF0000
#define INDEX_NOT_VALID_CNS  0xFFFFFFFF
/* allow WA DB up to 256 total ports in trunks (all ports in all trunks) */
/* this DB actually hold only 'non-local' ports that are trunk members */
/* it is needed to know the TCAM index that it occupy */
#define MAX_RULES_CNS   256
typedef struct{
    /* initialized to hold {port = 0xFFFFFFFF , hwDevice = 0}*/
    CPSS_TRUNK_MEMBER_STC usedIpclRules[MAX_RULES_CNS];
}PER_DEV_IPCL_RULES_STC;
/*dynamic allocated DBs for the devices managed by current CPU */
static PER_DEV_IPCL_RULES_STC* usedIpclRules_devsArr[PRV_CPSS_MAX_PP_DEVICES_CNS];

static CPSS_TRUNK_MEMBER_STC* getDevDb(IN GT_U8 devNum)
{
    return &usedIpclRules_devsArr[devNum]->usedIpclRules[0];
}

/* return index of found/free place .
   returns ERROR_TABLE_FULL_CNS --> when table is FULL.
*/
static GT_U32  findMemberInDb(
    IN GT_U8                    devNum,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr)
{
    GT_U32  ii;
    CPSS_TRUNK_MEMBER_STC *usedIpclRulePtr;
    GT_U32  firstFreeIndex = ERROR_TABLE_FULL_CNS;

    usedIpclRulePtr = getDevDb(devNum);
    for(ii = 0 ; ii < MAX_RULES_CNS;ii++,usedIpclRulePtr++)
    {
        if(usedIpclRulePtr->port == memberPtr->port &&
           usedIpclRulePtr->hwDevice == memberPtr->hwDevice)
        {
            return ii;
        }

        if(firstFreeIndex == ERROR_TABLE_FULL_CNS &&
           usedIpclRulePtr->port == INDEX_NOT_VALID_CNS)
        {
            firstFreeIndex = ii;
        }
    }

    if(firstFreeIndex == ERROR_TABLE_FULL_CNS)
    {
        cpssOsPrintf("ERROR: usedIpclRules[%s] : DB is not large enough \n",
            MAX_RULES_CNS);
    }

    /* not found in DB but this is free index */
    return firstFreeIndex;
}
/* initialize the members in the DB */
static GT_STATUS initMembersInDb(IN GT_U8   devNum)
{
    GT_U32  ii;
    PER_DEV_IPCL_RULES_STC  *myDevDbPtr;
    CPSS_TRUNK_MEMBER_STC *usedIpclRulePtr;

    if(NULL == usedIpclRules_devsArr[devNum])
    {
        usedIpclRules_devsArr[devNum] = cpssOsMalloc(sizeof(*usedIpclRulePtr));
        myDevDbPtr = usedIpclRules_devsArr[devNum];
        if(myDevDbPtr == NULL)
        {
            /* the malloc failed */
            return GT_OUT_OF_CPU_MEM;
        }
    }

    usedIpclRulePtr = getDevDb(devNum);
    for(ii = 0 ; ii < MAX_RULES_CNS;ii++,usedIpclRulePtr++)
    {
        usedIpclRulePtr->port = INDEX_NOT_VALID_CNS;
    }

    return GT_OK;
}

/* add(save) member to the DB */
static void addMemberToDb(
    IN GT_U8   devNum,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr)
{
    GT_U32 dbIndex;
    CPSS_TRUNK_MEMBER_STC *usedIpclRulePtr = getDevDb(devNum);

    dbIndex = findMemberInDb(devNum,memberPtr);

    usedIpclRulePtr[dbIndex] = *memberPtr;

    return;
}
/* remove member from the DB */
static void removeMemberFromDb(
    IN GT_U8   devNum,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    GT_U32 dbIndex;
    CPSS_TRUNK_MEMBER_STC *usedIpclRulePtr = getDevDb(devNum);

    dbIndex = findMemberInDb(devNum,memberPtr);

    usedIpclRulePtr[dbIndex].port = INDEX_NOT_VALID_CNS;

    return;
}

/*
*       Get absolute IPCL rules index bases in TCAM for IPCL
*       the 'relative index' from end of entries according to
*       appDemoDxChTcamIpclNumOfIndexsGet()
*/
static GT_U32 getIpclRuleIndex(
    IN GT_U8                    devNum,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    GT_U32 numIndexGet = appDemoDxChTcamIpclNumOfIndexsGet(devNum,ipcl_lookupId);
    GT_U32 baseIndex   = appDemoDxChTcamIpclBaseIndexGet(devNum,ipcl_lookupId);
    GT_U32 relativeTcamEntryIndex;
    GT_U32 dbIndex;

    dbIndex = findMemberInDb(devNum,memberPtr);
    if(dbIndex >= MAX_RULES_CNS)
    {
        /* ERROR */
        return dbIndex;
    }

    /* each port with it's rule ...set reverse order from last index */
    relativeTcamEntryIndex = (numIndexGet -1) - dbIndex;

    return  baseIndex + (relativeTcamEntryIndex % numIndexGet);
}

/* IPCL : ADD/REMOVE non-local port configurations  */
static GT_STATUS trunkMemberIpcl_phy1690_WA_C(
    IN GT_U8                    devNum,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr,
    IN GT_U32                   srcId,
    IN GT_ADD_OR_REMOVE         addOrRemove
)
{
    GT_STATUS   rc;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT ipcl_patternInfo;
    GT_U32 ruleIndex;

    /* get the unique index for this member */
    ruleIndex = getIpclRuleIndex(devNum,memberPtr);

    if(addOrRemove == GT_REMOVE)
    {
        /* remove member from the DB */
        removeMemberFromDb(devNum,memberPtr);

        /* the member removed from the trunk .. not need the rule */
        rc = cpssDxChPclRuleInvalidate(devNum,0/*ruleSize --sip5 not used*/,
            ruleIndex);
        if(rc != GT_OK)
        {
            return rc;
        }

    }
    else
    {
        /* add member to the DB */
        addMemberToDb(devNum,memberPtr);

        cpssOsMemSet(&ipcl_patternInfo, 0, sizeof(ipcl_patternInfo));

        /*
            1. 587:586 - Marvell Tagged Extended - must be '3' (16B eDSA)
                BYTE 73 bits 2..3
            2. 585     - Ingress Marvell Tagged - must be '1'
                BYTE 73 bit 1
        */
        ipcl_patternInfo.ruleEgrUdbOnly.udb[0] = 0xE;
        /*
            3. 341      - Routed - must be '0'
                BYTE 42 bit 5
        */
        ipcl_patternInfo.ruleEgrUdbOnly.udb[1] = 0;
        /*
            5. 79:67    - Orig Src ePort or Trunk ID - must be the physical port of the trunk member
                BYTE 9 , bits 0..7
                BYTE 8 , bits 3..7
            6. 66       - Orig Src Is Trunk - must be '0'
                BYTE 8 , bit 2
        */
        ipcl_patternInfo.ruleEgrUdbOnly.udb[2] = (GT_U8)(memberPtr->port >> 5);
        ipcl_patternInfo.ruleEgrUdbOnly.udb[3] = (GT_U8)(memberPtr->port << 3);

        /*
            7. 65:56    - Orig Src Dev  - must be the hwDevNum of the trunk member
                BYTE 8 , bits 0..1
                BYTE 7 , bits 0..7
        */
        ipcl_patternInfo.ruleEgrUdbOnly.udb[3]|= (GT_U8)(memberPtr->hwDevice >> 8);
        ipcl_patternInfo.ruleEgrUdbOnly.udb[4] = (GT_U8)(memberPtr->hwDevice);


        ipcl_actionInfo.sourceId.assignSourceId = GT_TRUE;
        ipcl_actionInfo.sourceId.sourceIdValue = srcId;

        /* the port added to the trunk .. need a rule for egress filtering
            (replace the 'non-trunk' filter) */
        rc = cpssDxChPclRuleSet(devNum,ingressRuleFormat,ruleIndex,
            0,/*ruleOptionsBmp,*/
            &ipcl_maskInfo,
            &ipcl_patternInfo,
            &ipcl_actionInfo);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/* SRC-ID : ADD/REMOVE local port configurations  */
static GT_STATUS trunkMemberSrcId_phy1690_WA_C(
    IN GT_U8                    devNum,
    IN GT_U32                   srcId,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_ADD_OR_REMOVE         addOrRemove
)
{
    /*
        NOTE: the HW initialization mode is that ALL ports are members in ALL SRC-ID
        meaning that we not need to initialize the SRC-ID.

        so only on runtime we add/delete member from it.
    */
    if(addOrRemove == GT_REMOVE)
    {
        /* remove local member from trunk ... so add it to the SRC-ID group */
        return cpssDxChBrgSrcIdGroupPortAdd(devNum,srcId,portNum);
    }
    else
    {
        /* add local member to trunk ... so remove it from the SRC-ID group
           to NOT allow packets from the cascade port (originated from
           this trunk) to egress this port (replacing the non-trunk table filter) */
        return cpssDxChBrgSrcIdGroupPortDelete(devNum,srcId,portNum);
    }
}


/* ADD/REMOVE member configurations */
static GT_STATUS trunkMember_phy1690_WA_C(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr,
    IN GT_ADD_OR_REMOVE         addOrRemove
)
{
    GT_STATUS rc;
    GT_U32      srcId;
    GT_HW_DEV_NUM   localDev_hwDevNum;

    localDev_hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);

    /* get the srcId that used for the src filtering */
    rc = trunkSrcIdGet_phy1690_WA_C(trunkId,&srcId);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(memberPtr->hwDevice != localDev_hwDevNum)
    {
        /* call CPSS for IPCL rules */
        rc = trunkMemberIpcl_phy1690_WA_C(devNum,memberPtr ,srcId , addOrRemove);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* call CPSS for SRC-ID management */
        rc = trunkMemberSrcId_phy1690_WA_C(devNum,srcId, memberPtr->port , addOrRemove);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChTrunkMembersSet_phy1690_WA_C function
* @endinternal
*
* @brief   part of trunk WA 'C' due to phy1690.
*         set trunk members including relevant WA configurations.
*         Add an associated IPCL rules in case of new non-local trunk members (for source filtering).
*         Remove the associated IPCL rules in case of removed non-local trunk members.
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
GT_STATUS cpssDxChTrunkMembersSet_phy1690_WA_C
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
    CPSS_TRUNK_MEMBER_STC   *currentMemberPtr;
    GT_U32                 old_numOfEnabledMembers;
    static CPSS_TRUNK_MEMBER_STC  old_membersArray[CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS];
    GT_U32                 old_numOfDisabledMembers;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    old_numOfEnabledMembers = CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS;
    rc = cpssDxChTrunkDbEnabledMembersGet(devNum,trunkId,
        &old_numOfEnabledMembers,
        &old_membersArray[0]);
    if(rc != GT_OK)
    {
        return rc;
    }

    old_numOfDisabledMembers = CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS - old_numOfEnabledMembers;
    rc = cpssDxChTrunkDbEnabledMembersGet(devNum,trunkId,
        &old_numOfDisabledMembers,
        &old_membersArray[old_numOfEnabledMembers]);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* loop on enabled ports */
    for(ii = 0 ; ii < numOfEnabledMembers; ii++)
    {
        currentMemberPtr = &enabledMembersArray[ii];

        /* set needed rules/SRC-ID for the added member to the trunk */
        rc = trunkMember_phy1690_WA_C(devNum,trunkId,currentMemberPtr,GT_ADD);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* loop on disabled ports */
    for(ii = 0 ; ii < numOfDisabledMembers; ii++)
    {
        currentMemberPtr = &disabledMembersArray[ii];

        /* set needed rules/SRC-ID for the added member to the trunk */
        rc = trunkMember_phy1690_WA_C(devNum,trunkId,currentMemberPtr,GT_ADD);
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


    /* remove ports from old that not exists any more */
    for(ii = 0 ; ii < (old_numOfEnabledMembers + old_numOfDisabledMembers); ii++)
    {
        currentMemberPtr = &old_membersArray[ii];

        rc = cpssDxChTrunkDbIsMemberOfTrunk(devNum,currentMemberPtr,NULL);
        if(rc == GT_OK)
        {
            /* the member is still in the trunk */
            continue;
        }

        /* unset needed rules/SRC-ID for the removed member from the trunk */
        rc = trunkMember_phy1690_WA_C(devNum,trunkId,currentMemberPtr,GT_REMOVE);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChTrunkMemberAdd_phy1690_WA_C function
* @endinternal
*
* @brief   part of trunk WA 'C' due to phy1690.
*         add trunk member including relevant WA configurations.
*         Add an associated IPCL rule in case of non-local trunk member (for source filtering).
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
GT_STATUS cpssDxChTrunkMemberAdd_phy1690_WA_C
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    GT_STATUS       rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* set needed rules/SRC-ID for the added member to the trunk */
    rc = trunkMember_phy1690_WA_C(devNum,trunkId,memberPtr,GT_ADD);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* call the CPSS for the trunk member */
    rc = cpssDxChTrunkMemberAdd(devNum,trunkId,memberPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChTrunkMemberRemove_phy1690_WA_C function
* @endinternal
*
* @brief   part of trunk WA 'C' due to phy1690.
*         remove trunk member including relevant WA configurations.
*         Remove the associated IPCL rule in case of non-local trunk member.
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
GT_STATUS cpssDxChTrunkMemberRemove_phy1690_WA_C
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    GT_STATUS       rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* unset needed rules/SRC-ID for the removed member from the trunk */
    rc = trunkMember_phy1690_WA_C(devNum,trunkId,memberPtr,GT_REMOVE);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* call the CPSS for the trunk member */
    rc = cpssDxChTrunkMemberRemove(devNum,trunkId,memberPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChCscdPortTypeSet_phy1690_WA_C function
* @endinternal
*
* @brief   part of trunk WA 'C' due to phy1690.
*         Configure a PP port to be a cascade port, including relevant WA configurations.
*         Enable IPCL-2 lookup on the cascade port.
*         Disable NA to CPU on the cascade port.(otherwise we get storming of NA
*         when FDB hold entry in trunkId but DSA came on same (MAC,vlan) with
*         (remoteDev,port) of trunk member !)
*         call CPSS API : cpssDxChCscdPortTypeSet(...)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - The port to be configured as cascade
* @param[in] portDirection            - port's direction.
* @param[in] portType                 - cascade  type DSA tag port or network port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdPortTypeSet_phy1690_WA_C
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN CPSS_PORT_DIRECTION_ENT      portDirection,
    IN CPSS_CSCD_PORT_TYPE_ENT      portType
)
{
    GT_STATUS                   rc;
    CPSS_PCL_LOOKUP_NUMBER_ENT  ipclLookupNum = CPSS_PCL_LOOKUP_NUMBER_2_E;
    GT_HW_DEV_NUM   localDev_hwDevNum;
    CPSS_DXCH_PCL_LOOKUP_CFG_STC    lookupCfg;
    CPSS_INTERFACE_INFO_STC   interfaceInfo;
    GT_BOOL                   enableIpcl;
    GT_BOOL                   naToCpuEnable;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    enableIpcl    = (portType != CPSS_CSCD_PORT_NETWORK_E) ? GT_TRUE : GT_FALSE;
    naToCpuEnable = (portType == CPSS_CSCD_PORT_NETWORK_E) ? GT_TRUE : GT_FALSE;

    /* ingress PCL configuration*/
    /* enable pcl on ingress port */
    rc = cpssDxChPclPortIngressPolicyEnable(devNum, portNum, enableIpcl);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(enableIpcl == GT_TRUE)
    {
        localDev_hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);

        /* enable IPCL2 on port */
        /* define that the accesss to PCL configuration table index done via port */
        rc = cpssDxChPclPortLookupCfgTabAccessModeSet(devNum,
                          portNum,
                          ipclDirection,
                          ipclLookupNum,
                          0, /* subLookupNum */
                          CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
        if(rc != GT_OK)
        {
            return rc;
        }

        cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

        lookupCfg.enableLookup           = GT_TRUE;
        lookupCfg.pclId                  = 0;
        lookupCfg.groupKeyTypes.nonIpKey = ingressRuleFormat;
        lookupCfg.groupKeyTypes.ipv4Key  = ingressRuleFormat;
        lookupCfg.groupKeyTypes.ipv6Key  = ingressRuleFormat;

        interfaceInfo.type = CPSS_INTERFACE_PORT_E;
        interfaceInfo.devPort.hwDevNum = localDev_hwDevNum;
        interfaceInfo.devPort.portNum  = portNum;

        /* set the IPCL2 configuration table */
        rc = cpssDxChPclCfgTblSet(devNum,&interfaceInfo,ipclDirection,ipclLookupNum,
                                  &lookupCfg);
        if(rc != GT_OK)
        {
            return rc;
        }

    }

    /* DONOT allow the NA messages to go to CPU for cascade port */
    rc = cpssDxChBrgFdbNaToCpuPerPortSet(devNum,portNum,naToCpuEnable);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* call the CPSS for the cascade port */
    rc = cpssDxChCscdPortTypeSet(devNum,portNum,portDirection,portType);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}


/* init the IPCL */
static GT_STATUS trunkIpclInit_phy1690_WA_C
(
    IN GT_U8                    devNum
)
{
    GT_STATUS   rc;
    CPSS_PCL_LOOKUP_NUMBER_ENT  ipclLookupNum = CPSS_PCL_LOOKUP_NUMBER_2_E;
    CPSS_DXCH_PCL_UDB_SELECT_STC    udbSelect;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT   packetType;
    IPCL_UDB_STC *ipclUdbPtr;
    GT_U32  ii;

    /*****************/
    /* set IPCL UDBs */
    /*****************/
    for(packetType = 0 ; packetType < CPSS_DXCH_PCL_PACKET_TYPE_LAST_E ;packetType++)
    {
        if(packetType == CPSS_DXCH_PCL_PACKET_TYPE_IPV6_E)
        {
            /* no such for SIP5 devices */
            continue;
        }
        for(ii = 0 ; ipclUdbInfo[ii].offsetType != CPSS_DXCH_PCL_OFFSET_INVALID_E ;ii++)
        {
            ipclUdbPtr = &ipclUdbInfo[ii];

            rc = cpssDxChPclUserDefinedByteSet(devNum,
                0,/*ruleFormat - ignored sip5 */
                packetType,/*packetType */
                ipclDirection,/*direction*/
                ipclUdbPtr->udbIndex,
                ipclUdbPtr->offsetType,
                ipclUdbPtr->offset
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
        udbSelect.udbSelectArr[ii] = IPCL_META_DATA_FIRST_UDB_CNS + ii;
    }

    /********************************************/
    /* map the IPCL UDBs indexes to proper UDBs */
    /********************************************/
    for(packetType = 0 ; packetType < CPSS_DXCH_PCL_PACKET_TYPE_LAST_E ;packetType++)
    {
        if(packetType == CPSS_DXCH_PCL_PACKET_TYPE_IPV6_E)
        {
            /* no such for SIP5 devices */
            continue;
        }

        rc = cpssDxChPclUserDefinedBytesSelectSet(devNum,
            ingressRuleFormat, packetType, ipclLookupNum,
            &udbSelect);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /************************/
    /* init the IPCL action */
    /************************/
    cpssOsMemSet(&ipcl_actionInfo, 0, sizeof(ipcl_actionInfo));


    /**********************/
    /* init the IPCL mask */
    /**********************/
    cpssOsMemSet(&ipcl_maskInfo, 0, sizeof(ipcl_maskInfo));

    for(ii = 0 ; ipclUdbInfo[ii].offsetType != CPSS_DXCH_PCL_OFFSET_INVALID_E ;ii++)
    {
        ipclUdbPtr = &ipclUdbInfo[ii];

        ipcl_maskInfo.ruleEgrUdbOnly.udb[ii] = ipclUdbPtr->byteMask;
    }

    return GT_OK;
}

/* init the SRC-ID */
static GT_STATUS trunkSrcIdInit_phy1690_WA_C
(
    IN GT_U8                    devNum
)
{
    devNum = devNum;

    /*
        NOTE: the HW initialization mode is that ALL ports are members in ALL SRC-ID
        meaning that we not need to initialize the SRC-ID.

        so only on runtime we add/delete member to/from it.
    */

    return GT_OK;
}

/**
* @internal cpssDxChTrunkInit_phy1690_WA_C function
* @endinternal
*
* @brief   part of trunk WA 'C' due to phy1690.
*         init the trunk WA
*         set IPCL-2 needed UDBs.
*         assuming :
*         1. application must call cpssDxChTrunkInit(...)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkInit_phy1690_WA_C
(
    IN GT_U8                    devNum
)
{
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    rc = initMembersInDb(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*************************/
    /* init the IPCL related */
    /*************************/
    rc = trunkIpclInit_phy1690_WA_C(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /***************************/
    /* init the SRC-ID related */
    /***************************/
    rc = trunkSrcIdInit_phy1690_WA_C(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}




