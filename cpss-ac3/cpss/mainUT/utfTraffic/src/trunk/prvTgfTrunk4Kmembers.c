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
* @file prvTgfTrunk4Kmembers.c
*
* @brief CPSS trunk testing implementation , 4K members in trunk.
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/port/cpssPortStat.h>
#include <cpss/generic/cos/cpssCosTypes.h>
#include <common/tgfPortGen.h>
#include <common/tgfPclGen.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <trafficEngine/private/prvTgfTrafficParser.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTrunkGen.h>
#include <common/tgfCscdGen.h>
#include <common/tgfPclGen.h>
#include <trunk/prvTgfTrunk.h>
#include <common/tgfConfigGen.h>
#include <common/tgfBridgeL2EcmpGen.h>
#include <common/tgfCncGen.h>
#include <cnc/prvTgfCncGen.h>
#include <cnc/prvTgfCncVlanL2L3.h>

#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>

/* bookmark to state that we need last index in the L2 ECMP table to hold the NULL port */
/* see PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpIndex_NULL_port */
#define L2_ECMP_NULL_PORT_BOOKMARK

#define SWAP(X,Y) { GT_U32 T = X; X = Y; Y = T; }

extern void init_4k_members(IN GT_U8   dev);
/* 'orig' array to set members : initialized once */
extern CPSS_TRUNK_MEMBER_STC   membersArray_orig_4K_tests[/*CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS*/];
/* array to set members : may copy from 'orig' and do modifications */
extern CPSS_TRUNK_MEMBER_STC   membersArray_set_4K_tests[/*CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS*/];

static CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT origLocalPortsEgfLink[1024];/*PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC*/
static GT_U32   origLocalPortsTxqPort[1024];/*PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC*/

static GT_U32    orig_maxNumberOfTrunks;
static CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT orig_trunkMembersMode;
static GT_TRUNK_ID trunkId_for_4K_members;
static PRV_TGF_TRUNK_LBH_GLOBAL_MODE_ENT globalHashModeGet;
static PRV_TGF_TRUNK_LBH_CRC_MODE_ENT    crcHashModeGet;
static GT_U32                            crcSeedGet;
static PRV_TGF_PCL_PACKET_TYPE_ENT    packetType = PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E;
static GT_U32   burstCount;
static GT_U32   actualNumOfMembersInTrunk;
static GT_U32   maxExpectedMemberToGetTraffic;
static GT_U16   testedVid;
static GT_BOOL  orig_CncCounterClearByReadEnable;

static GT_U32   expectedEgressMemberCounterArr[CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS] = {0};
static GT_U32   expectedEgressMemberBmpArr[(CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS+31)/32] = {0};

#define FLOOD_MAX_MEMBERS_CNS   80
/* indication that the 'flood' test is running */
static GT_BOOL  floodTesting = GT_FALSE;
/* indication that the 'flood' test running on device 'B' (and not device 'A') */
static GT_BOOL  deviceB_running = GT_FALSE;
/* device 'B' hwDevNum */
static GT_HW_DEV_NUM    deviceB_hwDevNum = 0x1BF;
/* device 'A' hwDevNum */
static GT_HW_DEV_NUM    deviceA_hwDevNum = 0x10;
static GT_U32           deviceA_DevPortMask = 0xFF;
static GT_U32           deviceA_globalCounter = 0;

static GT_U32   tgfTrunkCrcHash_4K_members_known_UC_EPCL_forcePortToMaxSpeed =  0;
/* debug function to modify tgfTrunkCrcHash_4K_members_known_UC_EPCL_forcePortToMaxSpeed */
void tgfTrunkCrcHash_4K_members_known_UC_EPCL_forcePortToMaxSpeed_set(GT_U32   force)
{
    tgfTrunkCrcHash_4K_members_known_UC_EPCL_forcePortToMaxSpeed = force;
}

/*((maxNumOfMembers+15) / 16)*/
#define BURST_COUNT_FROM_MAX_MEMBERS_MAC(maxNumOfMembers)  \
    (maxNumOfMembers)

#define CONVERT_TRUNK_MEMBER_TO_CNC_INDEX_MAC(memberIndex)  \
    (((memberIndex)+1) % _4K)


/* make sure that the sender port and the cascade port are not trunk members */
#define COLLIDE_PORT_OFFSET  80


static GT_U32   cncBlockCntrs;
static GT_U32   cncBlocks;


/* size of L2 (mac SA,Da) + vlan TAG + ethertype */
#define L2_VLAN_TAG_ETHERETYPE_SIZE_CNS \
                    (TGF_L2_HEADER_SIZE_CNS +/*l2*/         \
                     TGF_VLAN_TAG_SIZE_CNS  +/*vlan tag*/   \
                     TGF_ETHERTYPE_SIZE_CNS )/*ethertype*/

/* the ingress port must be with the lowest speed */
/* runtime may change those indexes */
static GT_U32   cascadePortIndex = 2;
static GT_U32   ingressPortIndex = 3;
static GT_U32   otherPortIndex   = 1;

#define CASCADE_PORT_INDEX_CNS  cascadePortIndex
#define INGRESS_PORT_INDEX_CNS  ingressPortIndex
#define OTHER_PORT_INDEX_CNS    otherPortIndex

typedef enum{
    UDB_FIELD_TRG_DEV_E,
    UDB_FIELD_TRG_PORT_E,
    UDB_FIELD_IS_TRG_PHY_PORT_VALID_E,
    UDB_FIELD_IS_VIDX_E,


    UDB_FIELD___NOT_VALID__E
}UDB_FIELD_ENT;

typedef struct
{
    UDB_FIELD_ENT   udbFieldType;
    GT_U32  startBit;
    GT_U32  numOfBits;
}UDB_OFFSET_INFO_STC;

static UDB_OFFSET_INFO_STC  udbOffsetsArr[] = {
             {UDB_FIELD_TRG_DEV_E,              104,10}   /* Trg Dev */
            ,{UDB_FIELD_TRG_PORT_E,             80,8}     /* Trg Port */
            ,{UDB_FIELD_IS_TRG_PHY_PORT_VALID_E,88,1}     /* Is Trg PHY Port Valid*/
            ,{UDB_FIELD_IS_VIDX_E,              136,1}    /* Use VIDX*/
            };
static GT_U32   udbOffsets = sizeof(udbOffsetsArr)/sizeof(udbOffsetsArr[0]);

typedef struct
{
    UDB_FIELD_ENT   udbFieldType;
    GT_U32  value;
}FIELD_VALUE_STC;
/* in known UC : useVidx = 0 , IS_TRG_PHY_PORT_VALID = true */
static FIELD_VALUE_STC  fieldValuesArr[] = {
             {UDB_FIELD_TRG_DEV_E,              0/*set in runtime from membersArray_set_4K_tests[ii].hwDevice*/}   /* Trg Dev */
            ,{UDB_FIELD_TRG_PORT_E,             0/*set in runtime from membersArray_set_4K_tests[ii].port*/}     /* Trg Port */
            ,{UDB_FIELD_IS_TRG_PHY_PORT_VALID_E,1}     /* Is Trg PHY Port Valid*/
            ,{UDB_FIELD_IS_VIDX_E,              0}     /* Use VIDX*/
            };
/* in flood : useVidx = 1 , IS_TRG_PHY_PORT_VALID = false */
static FIELD_VALUE_STC  floodFieldValuesArr[] = {
             {UDB_FIELD_TRG_DEV_E,              0/*set in runtime from membersArray_set_4K_tests[ii].hwDevice*/}   /* Trg Dev */
            ,{UDB_FIELD_TRG_PORT_E,             0/*set in runtime from membersArray_set_4K_tests[ii].port*/}     /* Trg Port */
            ,{UDB_FIELD_IS_TRG_PHY_PORT_VALID_E,0}     /* Is Trg PHY Port Valid*/
            ,{UDB_FIELD_IS_VIDX_E,              1}     /* Use VIDX*/
            };

extern GT_STATUS prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortMapSet
(
    IN GT_U8                devNum,
    IN GT_U32               portGroupId,
    IN GT_PHYSICAL_PORT_NUM physPort,
    IN GT_U32               txqNum
);
extern GT_STATUS prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortMapGet
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 physPort,
   OUT GT_U32 *txqNumPtr
);

extern GT_STATUS appDemoDxHwDevNumChange
(
    IN  GT_U8                       dev,
    IN  GT_HW_DEV_NUM               hwDevNum
);


/**
* @internal epcl_egressPortsSet function
* @endinternal
*
* @brief   setup/invalidate EPCL configurations on egress ports to allow EPCL lookup
*         and configurations
* @param[in] enable                   - GT_TRUE to setup, GT_FALSE to invalidate
*                                       None
*/
static GT_VOID epcl_egressPortsSet
(
    IN GT_BOOL enable
)
{
    GT_STATUS    rc;
    GT_U32  ii;
    CPSS_INTERFACE_INFO_STC interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;
    GT_PORT_NUM         portNum;
    GT_HW_DEV_NUM                own_hwDevNum;

    if (enable == GT_TRUE)
    {
        /* init PCL */
        rc = prvTgfPclInit();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "prvTgfPclInit: failed");
    }

    /* AUTODOC: enables egress policy per devices */
    rc = prvTgfPclEgressPolicyEnable(enable);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "prvTgfPclEgressPolicyEnable: failed");

    /* set EPCL configuration table for ports */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = enable;
    lookupCfg.pclId                  = 0;
    lookupCfg.dualLookup             = GT_FALSE;
    lookupCfg.pclIdL01               = 0;
    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;


    own_hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    for (ii = 0; ii < (actualNumOfMembersInTrunk + 1); ii++)
    {
        if(ii == actualNumOfMembersInTrunk)
        {
            portNum = prvTgfPortsArray[CASCADE_PORT_INDEX_CNS];
        }
        else
        {
            if(membersArray_set_4K_tests[ii].hwDevice != own_hwDevNum)
            {
                continue;
            }

            portNum = membersArray_set_4K_tests[ii].port;
        }

        /* AUTODOC: enables egress policy for all ports and non-tunneled packets */
        rc = prvTgfPclEgressPclPacketTypesSet(
            prvTgfDevNum, portNum,
            PRV_TGF_PCL_EGRESS_PKT_NON_TS_E,
            enable);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "prvTgfPclEgressPclPacketTypesSet: failed");

        /* AUTODOC: configure lookup0 egress entry by Port area 0,1,2,3 */
        rc = prvTgfPclPortLookupCfgTabAccessModeSet(
                portNum, CPSS_PCL_DIRECTION_EGRESS_E,
                CPSS_PCL_LOOKUP_0_E, 0 /*sublookup*/,
                PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "prvTgfPclPortLookupCfgTabAccessModeSet: failed");

        interfaceInfo.devPort.portNum = portNum;

        /* AUTODOC: set EPCL config table for ports 0,1,2,3 lookup0 with: */
        /* AUTODOC:   nonIpKey = EGRESS_UDB_10 */
        /* AUTODOC:   ipv4Key  = EGRESS_UDB_10 */
        /* AUTODOC:   ipv6Key  = EGRESS_UDB_10 */
        rc = prvTgfPclCfgTblSet(
            &interfaceInfo,
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            &lookupCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "prvTgfPclCfgTblSet: failed");
    }

    if(floodTesting == GT_FALSE)
    {
        rc = prvTgfPclEgressTargetPortSelectionModeSet(
            enable == GT_TRUE ?
            PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_FINAL_E :
            PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_LOCAL_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                               "prvTgfPclEgressTargetPortSelectionModeSet: failed");
    }
    else
    {
        /* we need the 'default' which is 'LOCAL' so no need to modify the config */
    }

}

/**
* @internal epcl4K_members_Rules function
* @endinternal
*
* @brief   setup/invalidate EPCL rules : 4K rules that needed for 4K members in the trunk.
*         generating '10B_UDB' key with fields , on 'exact match':
*         UDB_FIELD_TRG_DEV_E - per trunk member
*         UDB_FIELD_TRG_PORT_E - per trunk member
*         for single destination :
*         UDB_FIELD_IS_TRG_PHY_PORT_VALID_E - must be 1 'true'
*         UDB_FIELD_IS_VIDX_E - must be 0 'false'
*         for multi destination (flood):
*         UDB_FIELD_IS_TRG_PHY_PORT_VALID_E - must be 0 'false'
*         UDB_FIELD_IS_VIDX_E - must be 1 'true'
*         the action is : bind to CNC counter , to count the HIT
*         also build metadata UDBs for 'IPV4_TCP' traffic (UDBs 30,31,32,33)
* @param[in] enable                   - GT_TRUE to setup, GT_FALSE to invalidate rules
*                                       None
*/
static GT_VOID epcl4K_members_Rules
(
    IN GT_BOOL enable
)
{
    GT_STATUS                            rc;
    GT_U32                                offset;
    GT_U32                               udbIndex;
    GT_U32                               ii,jj,kk;
    GT_U32      udbIndexBase = 30;/* EPCL : Only UDBs 30-49 may be configured with metadata anchor type*/
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT  ruleFormat =  PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    GT_U32  numOfRules = actualNumOfMembersInTrunk;
    GT_U32  maskValue;
    GT_U32  fieldValue;
    GT_U32  startBit;
    GT_U32  numOfBits;

    PRV_TGF_PCL_UDB_SELECT_STC  udbSelect;
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;      /* rule mask */
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;   /* rule pattern */
    PRV_TGF_PCL_ACTION_STC      action;    /* rule aciton */
    UDB_OFFSET_INFO_STC         *currUdbInfoPtr;

    if (GT_FALSE == enable)
    {
        for (ii=0; ii < numOfRules; ++ii)
        {
            rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_10_BYTES_E,
                prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(ii),
                                             GT_FALSE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "prvTgfPclRuleValidStatusSet: %d", prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(ii));
        }

        return;
    }

    TGF_RESET_PARAM_MAC(mask);
    TGF_RESET_PARAM_MAC(pattern);
    TGF_RESET_PARAM_MAC(action);

    TGF_RESET_PARAM_MAC(udbSelect);

    udbIndex = udbIndexBase;
    for (ii = 0; (ii < udbOffsets); ii++)
    {
        currUdbInfoPtr = &udbOffsetsArr[ii];

        if(floodTesting == GT_FALSE)
        {
            fieldValue = fieldValuesArr[ii].value;
        }
        else
        {
            fieldValue = floodFieldValuesArr[ii].value;
        }

        maskValue =  BIT_MASK_MAC(currUdbInfoPtr->numOfBits);

        for(jj = 0 ; jj < currUdbInfoPtr->numOfBits; jj += 8, udbIndex++)
        {
            offset = (currUdbInfoPtr->startBit + jj) / 8 ;

            rc = prvTgfPclUserDefinedByteSet(
                    ruleFormat,
                    packetType,
                    CPSS_PCL_DIRECTION_EGRESS_E,
                    udbIndex,
                    PRV_TGF_PCL_OFFSET_METADATA_E,
                    (GT_U8)offset);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                         "prvTgfPclUserDefinedByteSet: failed");

            startBit  = jj == 0 ? currUdbInfoPtr->startBit % 8 : 0;
            numOfBits = jj == 0 ?
                            currUdbInfoPtr->numOfBits :
                            currUdbInfoPtr->numOfBits -
                                (8-(currUdbInfoPtr->startBit % 8)) -/*those that given in iteration 0 */
                                (8*(jj-1));             /* those that given in next iterations */

            if((numOfBits+startBit) > 8)
            {
                numOfBits = 8 - startBit;
            }

            mask   .ruleEgrUdbOnly.udb[udbIndex-udbIndexBase] = (GT_U8)(maskValue  << startBit);
            pattern.ruleEgrUdbOnly.udb[udbIndex-udbIndexBase] = (GT_U8)(fieldValue << startBit);

            maskValue  >>= numOfBits;/* remove the bits that already used */
            fieldValue >>= numOfBits;/* remove the bits that already used */


            /* state that the UDB are from high range */
            udbSelect.udbSelectArr[udbIndex-udbIndexBase] = udbIndex;
        }
    }

    /* map the UDBs indexes to proper UDBs */
    rc = prvTgfPclUserDefinedBytesSelectSet(
        ruleFormat, packetType, CPSS_PCL_LOOKUP_0_E,
        &udbSelect);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclUserDefinedBytesSelectSet: failed");


    /* the action only need to count */
    action.egressPolicy          = GT_TRUE;
    action.pktCmd                = CPSS_PACKET_CMD_FORWARD_E;
    action.matchCounter.enableMatchCount = GT_TRUE;

    PRV_UTF_LOG1_MAC("======= start to set [%d] EPCL rules (one per trunk member) =======\n",
        numOfRules);

    for (ii=0; ii < numOfRules; ++ii)
    {
        action.matchCounter.matchCounterIndex =
            CONVERT_TRUNK_MEMBER_TO_CNC_INDEX_MAC(ii);

        /*
            set the runtime values for the fields:
            UDB_FIELD_TRG_DEV_E
            UDB_FIELD_TRG_PORT_E

        */

        udbIndex = 0;
        for(kk = 0 ; kk < 2 ; kk++)
        {
            if(kk == 0)
            {
                fieldValue = membersArray_set_4K_tests[ii].hwDevice;
                currUdbInfoPtr = &udbOffsetsArr[UDB_FIELD_TRG_DEV_E];
            }
            else  /* kk = 1 */
            {
                fieldValue = membersArray_set_4K_tests[ii].port;
                currUdbInfoPtr = &udbOffsetsArr[UDB_FIELD_TRG_PORT_E];
            }

            for(jj = 0 ; jj < currUdbInfoPtr->numOfBits; jj += 8, udbIndex++)
            {
                startBit  = jj == 0 ? currUdbInfoPtr->startBit % 8 : 0;
                numOfBits = jj == 0 ?
                                currUdbInfoPtr->numOfBits :
                                currUdbInfoPtr->numOfBits -
                                    (8-(currUdbInfoPtr->startBit % 8)) -/*those that given in iteration 0 */
                                    (8*(jj-1));             /* those that given in next iterations */

                if((numOfBits+startBit) > 8)
                {
                    numOfBits = 8 - startBit;
                }

                pattern.ruleEgrUdbOnly.udb[udbIndex] = (GT_U8)(fieldValue << startBit);

                fieldValue >>= numOfBits;/* remove the bits that already used */
            }
        }
/* --- EXCELLENT debug print --
        PRV_UTF_LOG4_MAC("Rule Index [%d] is for trunk member {dev=[%d],port=[%d]} cnc index[%d] \n",
            ii,
            membersArray_set_4K_tests[ii].hwDevice,
            membersArray_set_4K_tests[ii].port,
            action.matchCounter.matchCounterIndex);
*/
        rc = prvTgfPclRuleSet(ruleFormat,
                              prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(ii), &mask, &pattern, &action);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                     "prvTgfPclRuleSet: failed");
    }
}

/**
* @internal epcl_configInit function
* @endinternal
*
* @brief   init the EPCL config needed for 4K members
*/
static GT_VOID epcl_configInit
(
    GT_VOID
)
{
    epcl_egressPortsSet(GT_TRUE);
    epcl4K_members_Rules(GT_TRUE);
}


/**
* @internal cnc_configSet function
* @endinternal
*
* @brief   setup/invalidate CNC configurations for EPCL to use
*
* @param[in] enable                   - GT_TRUE to setup, GT_FALSE to invalidate
*                                       None
*/
static GT_VOID cnc_configSet
(
    IN GT_BOOL enable
)
{
    GT_STATUS   st;
    GT_U32                   ii;
    PRV_TGF_CNC_CLIENT_ENT   client = PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_0_E;
    GT_U64                   indexRangesBmp;
    GT_U32                   actualNeededCncBlocks;

    /* get CNC size info */
    prvTgfPclCncSizeGet(&cncBlockCntrs,&cncBlocks);

    /* the number of blocks are : 4K counter divided by num counters per block */
    actualNeededCncBlocks = (_4K+(cncBlockCntrs-1)) / cncBlockCntrs;

    indexRangesBmp.l[0] = 0x0;
    indexRangesBmp.l[1] = 0x0;

    for (ii = 0; ii < actualNeededCncBlocks; ii++)
    {
        indexRangesBmp.l[0] = 1 << ii;
        /* configure CNC */
        st = prvTgfCncTestCncBlockConfigure(ii,client,enable /*enable*/,
            indexRangesBmp.l,
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, st, "prvTgfCncTestCncBlockConfigure: block %d, client %d",
                ii,
                client);
    }

    if(enable == GT_TRUE)
    {
        st = prvTgfCncCounterClearByReadEnableGet(&orig_CncCounterClearByReadEnable);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvTgfCncCounterClearByReadEnableSet(GT_TRUE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    }
    else
    {
        st = prvTgfCncCounterClearByReadEnableSet(orig_CncCounterClearByReadEnable);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    }
}

static GT_U32 debug_burstCount = 0;
static GT_U32 debug_maxNumOfMembers = 0;
/* debug function to set 'max' num of members to use */
void tgfTrunkCrcHash_4K_members_known_UC_EPCL_debug_maxNumOfMembers(GT_U32  value)
{
    debug_maxNumOfMembers = value;
}

/* debug function to set 'max' num of packets to use */
void tgfTrunkCrcHash_4K_members_known_UC_EPCL_debug_burstCount(GT_U32  value)
{
    debug_burstCount = value;
}

/**
* @internal trunk_configInit function
* @endinternal
*
* @brief   init the trunk config needed for 4K members
*/
static GT_VOID trunk_configInit
(
    GT_VOID
)
{
    GT_STATUS   st;
    GT_U8   dev = prvTgfDevNum;

    GT_U32      l2EcmpNumMembers,maxNumOfMembers;
    GT_U32      maxTrunksInDev;
    GT_U32      l2EcmpStartIndex;
    PRV_TGF_TRUNK_LBH_CRC_MASK_ENTRY_STC maskEntry;
    CPSS_TRUNK_MEMBER_STC tempMember;
    GT_U32  ii;
    GT_U32  ttiHit0Index;
    GT_HW_DEV_NUM   hwDevNum;
    GT_HW_DEV_NUM   hwDevNum_checkColide[2];
    GT_U32          hwDevNum_checkColideNum = 0;
    GT_U32          colideIndex;
    GT_U32          devPortMask;

    if(floodTesting == GT_FALSE)
    {
        init_4k_members(dev);
    }

    l2EcmpNumMembers = PRV_CPSS_DXCH_PP_MAC(dev)->bridge.l2EcmpNumMembers;
    L2_ECMP_NULL_PORT_BOOKMARK
    l2EcmpNumMembers--;

    maxTrunksInDev   = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.trunksNum;

    st = cpssDxChTrunkDbInitInfoGet(dev,&orig_maxNumberOfTrunks,&orig_trunkMembersMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

    /* destroy the trunk LIB so we can set flex mode */
    st = cpssDxChTrunkDestroy(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    /*set (4K-1) trunks in flex mode */
    st = cpssDxChTrunkInit(dev , maxTrunksInDev,CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

    trunkId_for_4K_members = (GT_TRUNK_ID)maxTrunksInDev;

    if(floodTesting == GT_FALSE)
    {
        /* set to use the end of the 'L2 ECMP' */
        maxNumOfMembers  = (l2EcmpNumMembers > _4K) ? _4K : l2EcmpNumMembers;

        ttiHit0Index = appDemoDxChTcamTtiBaseIndexGet(dev,0/*TTI_HIT_0 is first one*/);

        if(maxNumOfMembers > ttiHit0Index)
        {
            /* the appDemo not allow so many rules for IPCL/EPCL !!! */
            PRV_UTF_LOG2_MAC("the appDemo not allow [%d] rules for IPCL/EPCL !!! , so limit trunk members to [%d] to get good load balance \n",
                maxNumOfMembers , ttiHit0Index);

            maxNumOfMembers = ttiHit0Index;
        }

        if(prvUtfIsGmCompilation() == GT_TRUE)
        {
            maxNumOfMembers = _1K;
        }
    }
    else
    {
        maxNumOfMembers = FLOOD_MAX_MEMBERS_CNS;
    }


    if(debug_maxNumOfMembers)/* allow debug to force any value */
    {
        maxNumOfMembers = debug_maxNumOfMembers;
    }

    l2EcmpStartIndex = l2EcmpNumMembers - maxNumOfMembers;

    if(floodTesting == GT_FALSE)
    {
        burstCount = _4K;/* because the CRC32 set with 12 bits selection ...
            we will get good LB only if sending 4K packets ! */
    }
    else
    {
        /* the flooding is going only to 64 ports ... no need more than factor 10
           on that number , to understand that behaves 'ok' */
        burstCount = 64*10;
    }

    if(debug_burstCount && burstCount > debug_burstCount)
    {
        burstCount = debug_burstCount;
    }

    if(prvUtfIsGmCompilation() == GT_TRUE)
    {   /* reduce to 64 packets */
        if(burstCount > 64)
        {
            burstCount = 64;
        }
    }

    /* set this trunk to be operational */
    st = cpssDxChTrunkFlexInfoSet(dev,trunkId_for_4K_members,l2EcmpStartIndex,maxNumOfMembers);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

    devPortMask = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(dev) - 1;
    if (PRV_CPSS_DXCH_PP_MAC(dev)->port.reservedPortsPtr)
    {
        /* Don't use two upper reserved ports as port members in trunk */
        devPortMask -= 2;
    }


    if(floodTesting == GT_FALSE)
    {
        /* copy to 'set' array from the 'orig' array */
        cpssOsMemCpy(membersArray_set_4K_tests,
                     membersArray_orig_4K_tests,
                     CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS * sizeof(CPSS_TRUNK_MEMBER_STC));

        deviceA_DevPortMask = devPortMask;
    }
    else
    {
        if(deviceB_running == GT_FALSE)
        {
            deviceA_DevPortMask = devPortMask;
            deviceA_hwDevNum    = PRV_CPSS_HW_DEV_NUM_MAC(dev);
        }
        else
        {
            /* do not modify deviceA_hwDevNum */

            goto membersArray_ready_lbl;
        }

        for(ii = 0 ; ii < maxNumOfMembers ; ii++)
        {
            if(ii < 25)
            {
                /* first 25 ports are from device A */
                hwDevNum = deviceA_hwDevNum;
            }
            else
            if(ii < 64)
            {
                /* next 39 ports are from device B */
                hwDevNum = deviceB_hwDevNum;
            }
            else
            if(ii < 72)
            {
                /* next 8 ports from device A */
                hwDevNum = deviceA_hwDevNum;
            }
            else
            {
                /* next 8 ports are from device B */
                hwDevNum = deviceB_hwDevNum;
            }

            membersArray_set_4K_tests[ii].port     = (deviceA_DevPortMask - ii);
            if(prvUtfIsGmCompilation() == GT_TRUE)
            {
                if(membersArray_set_4K_tests[ii].port)
                {
                    /* for some reason the GM not like last port number 'deviceA_DevPortMask' */
                    /* so we shift all ports by 1 */
                    membersArray_set_4K_tests[ii].port--;
                }
                else
                {
                    /* wraparound (should not get here because only 80 ports in trunk)
                       and the deviceA_DevPortMask is 255 or more */
                    membersArray_set_4K_tests[ii].port = deviceA_DevPortMask - 1;
                }
            }

            membersArray_set_4K_tests[ii].hwDevice = hwDevNum;
        }
    }

membersArray_ready_lbl:
    PRV_UTF_LOG2_MAC("======= start to set [%d] members into trunk [%d]=======\n",
        maxNumOfMembers,trunkId_for_4K_members);

    actualNumOfMembersInTrunk = maxNumOfMembers;
    /* set 4K members in the trunk */
    /* all ports are enabled */
    st = cpssDxChTrunkMembersSet(dev,trunkId_for_4K_members,
                maxNumOfMembers,
                &membersArray_set_4K_tests[0],/*enabled members in the start*/
                0,
                NULL);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

    if(floodTesting == GT_TRUE && (deviceB_running == GT_TRUE))
    {
        /* we not modify trunk members order */
        goto trunkGlobalConfig_lbl;
    }

    if(floodTesting == GT_FALSE)
    {
        hwDevNum_checkColideNum = 1;
        hwDevNum_checkColide[0] = PRV_CPSS_HW_DEV_NUM_MAC(dev);
    }
    else
    {
        hwDevNum_checkColideNum = 2;
        hwDevNum_checkColide[0] = deviceA_hwDevNum;
        hwDevNum_checkColide[1] = deviceB_hwDevNum;
    }

    /* make sure that the sender port and the cascade port are not trunk members */
    for(colideIndex = 0; colideIndex < hwDevNum_checkColideNum ; colideIndex++)
    {
        tempMember.hwDevice = hwDevNum_checkColide[colideIndex];
        for(ii = 0 ; ii < 2 ; ii++)
        {
            if(ii == 0)
            {
                tempMember.port     = prvTgfPortsArray[INGRESS_PORT_INDEX_CNS];
            }
            else
            {
                tempMember.port     = prvTgfPortsArray[CASCADE_PORT_INDEX_CNS];
            }

            /* make sure that the sender port and the cascade port are not trunk members */
            st = cpssDxChTrunkDbIsMemberOfTrunk(dev,&tempMember,NULL);
            if(st == GT_NOT_FOUND)
            {
                /* good ! not in the trunk */
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfTrunkHashGlobalModeGet: %d", prvTgfDevNum);

                /*we need to remove it from the trunk */
                st = cpssDxChTrunkMemberRemove(dev,trunkId_for_4K_members,&tempMember);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkMemberRemove: %d", prvTgfDevNum);

                tempMember.port     += COLLIDE_PORT_OFFSET;

                if((tempMember.port & deviceA_DevPortMask) == CPSS_NULL_PORT_NUM_CNS || /*null port*/
                   (tempMember.port & deviceA_DevPortMask) == CPSS_CPU_PORT_NUM_CNS)   /*cpu port*/
                {
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, GT_BAD_PARAM, "cpssDxChTrunkMemberRemove: tempMember.port [%d] + 80 cause"
                        " with mask[0x%x] unsupported portNum[%d]",
                        tempMember.port-COLLIDE_PORT_OFFSET,
                        deviceA_DevPortMask,
                        (tempMember.port & deviceA_DevPortMask));
                    return /* kill the test*/;
                }

                tempMember.port &= deviceA_DevPortMask;

                /*we need to replace it with other port */
                st = cpssDxChTrunkMemberAdd(dev,trunkId_for_4K_members,&tempMember);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkMemberAdd: %d", prvTgfDevNum);
            }
        }
    }

    if(floodTesting == GT_TRUE && (deviceB_running == GT_FALSE))
    {
        /* we need to update the trunk members array due to possible
           'replaced member' */
        st = cpssDxChTrunkDbEnabledMembersGet(dev,trunkId_for_4K_members,
            &maxNumOfMembers,
            membersArray_set_4K_tests);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkDbEnabledMembersGet: %d", prvTgfDevNum);
    }


trunkGlobalConfig_lbl:
    /* Get the general hashing mode of trunk hash generation as CRC Hash based on the packet's data*/
    st =  prvTgfTrunkHashGlobalModeGet(prvTgfDevNum,&globalHashModeGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfTrunkHashGlobalModeGet: %d", prvTgfDevNum);

    /* Set the general hashing mode of trunk hash generation as CRC Hash based on the packet's data*/
    st =  prvTgfTrunkHashGlobalModeSet(PRV_TGF_TRUNK_LBH_PACKETS_INFO_CRC_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfTrunkHashGlobalModeSet: %d", prvTgfDevNum);

    /* get crc hash parameters */
    st =  prvTgfTrunkHashCrcParametersGet(prvTgfDevNum, &crcHashModeGet, &crcSeedGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfTrunkHashCrcParametersGet: %d", prvTgfDevNum);

    /* set Enhanced CRC-Based Hash Mode */
    st =  prvTgfTrunkHashCrcParametersSet(PRV_TGF_TRUNK_LBH_CRC_32_MODE_E, 0/*crcSeed*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfTrunkHashCrcParametersSet: %d", prvTgfDevNum);

    cpssOsMemSet(&maskEntry,0,sizeof(maskEntry));

    maskEntry.macDaMaskBmp      = 0x3f;
    maskEntry.macSaMaskBmp      = 0x3f;
    maskEntry.ipDipMaskBmp      = 0xffff;
    maskEntry.ipSipMaskBmp      = 0xffff;
    maskEntry.l4DstPortMaskBmp = 0x3;
    maskEntry.l4SrcPortMaskBmp = 0x3;

    /* set HASH mask */
    st =  prvTgfTrunkHashMaskCrcEntrySet(GT_FALSE,0,packetType,&maskEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfTrunkHashMaskCrcEntrySet: %d", prvTgfDevNum);

    st = cpssDxChTrunkMemberSelectionModeSet(dev,CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkMemberSelectionModeSet: %d", prvTgfDevNum);


}


/**
* @internal remoteDevicesMappingSet function
* @endinternal
*
* @brief   setup/invalidate device map table to the remote devices.
*         NOTE: all remote devices are set to single cascade port.
* @param[in] enable                   - GT_TRUE to setup, GT_FALSE to invalidate
*                                       None
*/
static GT_VOID remoteDevicesMappingSet
(
    IN GT_BOOL enable
)
{
    GT_STATUS   rc;
    GT_U8   dev = prvTgfDevNum;
    GT_HW_DEV_NUM                targetHwDevNum;
    CPSS_CSCD_LINK_TYPE_STC      cascadeLink;

    if(floodTesting == GT_TRUE)
    {
        /* no need to modify the device map table , because no known UC is used */
        /* only flooding */
        return;
    }


    if(enable == GT_TRUE)
    {
        cascadeLink.linkNum  = prvTgfPortsArray[CASCADE_PORT_INDEX_CNS];
        cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    }
    else
    {
        cascadeLink.linkNum  = 0;
        cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    }

    for(targetHwDevNum = 0 ;
        targetHwDevNum <= PRV_CPSS_DEV_TRUNK_INFO_MAC(dev)->validityMask.hwDevice;
        targetHwDevNum++)
    {
        /*
            NOTE: can't use prvTgfCscdDevMapTableSet(...) because it consider
                parameter 'targetHwDevNum' as 'swDevNum' and convert it to 'HW devNum'

                but we have SW_devNum = 0 (own dev)
                and HW_devNum = 0 is 'other device'
                ... so can use prvTgfCscdDevMapTableSet(...)
        */

        rc = cpssDxChCscdDevMapTableSet(dev,targetHwDevNum,0/*sourceHwDevNum -- dont care*/,
            0/* targetPortNum -- dont care */,&cascadeLink,GT_FALSE,GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
    }
}

/**
* @internal fdb_configSet function
* @endinternal
*
* @brief   setup/invalidate fdb entries.
*
* @param[in] enable                   - GT_TRUE to setup, GT_FALSE to invalidate
*                                       None
*/
static GT_VOID fdb_configSet
(
    IN GT_BOOL enable
)
{
    GT_STATUS   rc;
    GT_U32      ii;
    TGF_PACKET_STC      *exampleTrafficPtr;
    TGF_PACKET_L2_STC*   packetL2Ptr;
    TGF_PACKET_VLAN_TAG_STC* vlanTagPtr;
    PRV_TGF_BRG_MAC_ENTRY_STC  brgMacEntry;

    if(enable == GT_FALSE)
    {
        /* flush FDB include static entries */
        rc = prvTgfBrgFdbFlush(GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfBrgFdbFlush: failed\n");

        return;
    }

    /*prvTgfIpv4TcpPacketInfo*/
    exampleTrafficPtr = prvTgfTrunkPacketGet(packetType);
    if(exampleTrafficPtr == NULL)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, GT_BAD_PTR, "prvTgfTrunkPacketGet: failed\n");
        return;
    }

    packetL2Ptr = (TGF_PACKET_L2_STC*)exampleTrafficPtr->partsArray[0].partPtr;
    vlanTagPtr = (TGF_PACKET_VLAN_TAG_STC*)exampleTrafficPtr->partsArray[1].partPtr;
    testedVid = vlanTagPtr->vid;

    if(floodTesting == GT_TRUE)
    {
        /* must not have FDB entry for the macDA */
        /* we got here to have the 'testedVid' value */
        return;
    }



    cpssOsMemSet(&brgMacEntry, 0, sizeof(brgMacEntry));
    brgMacEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    for (ii = 0; (ii < 6); ii++)
    {
        brgMacEntry.key.key.macVlan.macAddr.arEther[ii] =
            packetL2Ptr->daMac[ii];
    }
    brgMacEntry.key.key.macVlan.vlanId = testedVid;
    brgMacEntry.daCommand = PRV_TGF_PACKET_CMD_FORWARD_E;
    brgMacEntry.saCommand = PRV_TGF_PACKET_CMD_FORWARD_E;

    brgMacEntry.dstInterface.type    = CPSS_INTERFACE_TRUNK_E;
    brgMacEntry.dstInterface.trunkId = trunkId_for_4K_members;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(brgMacEntry.dstInterface.trunkId);
    brgMacEntry.dstOutlif.outlifType = PRV_TGF_OUTLIF_TYPE_LL_E;

    brgMacEntry.isStatic = GT_TRUE;

    rc = prvTgfBrgFdbMacEntrySet(&brgMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgFdbMacEntrySet: failed\n");
}


/**
* @internal vlan_configSet function
* @endinternal
*
* @brief   setup/invalidate vlan entry.
*
* @param[in] enable                   - GT_TRUE to setup, GT_FALSE to invalidate
*                                       None
*/
static GT_VOID vlan_configSet
(
    IN GT_BOOL enable
)
{
    GT_STATUS   rc;
    GT_U8   dev = prvTgfDevNum;
    GT_U32  ii;
    /* port bitmap VLAN members */
    CPSS_PORTS_BMP_STC localPortsVlanMembers;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_HW_DEV_NUM   own_hwDevNum;
    GT_U32          txqOfValidPort;
    GT_U32          portGroupId = 0;

    own_hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(dev);

    if(enable == GT_TRUE)
    {
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&localPortsVlanMembers);

        /* set all ports as VLAN Members */
        for (ii = 0; ii < prvTgfPortsNum; ii++)
        {
            CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[ii]);
        }

        /* get the TXQ of Valid port ... so it will have 'pizza credits'
           (and simulation will have MAC to it) */
        rc = prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortMapGet(dev,portGroupId,
            prvTgfPortsArray[OTHER_PORT_INDEX_CNS],
            &txqOfValidPort);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortMapGet: failed\n");

        for (ii = 0; ii < actualNumOfMembersInTrunk; ii++)
        {
            if(membersArray_set_4K_tests[ii].hwDevice != own_hwDevNum)
            {
                continue;
            }

            portNum = membersArray_set_4K_tests[ii].port;


            rc = cpssDxChBrgEgrFltPortLinkEnableGet(dev,portNum,
                &origLocalPortsEgfLink[portNum]);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "cpssDxChBrgEgrFltPortLinkEnableGet: failed\n");

            /* add the port to vlan and force link up */
            rc = cpssDxChBrgEgrFltPortLinkEnableSet(dev,portNum,
                CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "cpssDxChBrgEgrFltPortLinkEnableSet: failed\n");

            CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,portNum);

            rc = prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortMapGet(dev,portGroupId,
                portNum,
                &origLocalPortsTxqPort[portNum]);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortMapGet: failed\n");

            rc = prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortMapSet(dev,0,
                portNum,
                txqOfValidPort);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortMapSet: failed\n");
        }

        /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
        prvTgfTrunkVlanTestInit(testedVid, localPortsVlanMembers);
    }
    else
    {
        for (ii = 0; ii < actualNumOfMembersInTrunk ; ii++)
        {
            if(membersArray_set_4K_tests[ii].hwDevice != own_hwDevNum)
            {
                continue;
            }

            portNum = membersArray_set_4K_tests[ii].port;

            /* restore link state */
            rc = cpssDxChBrgEgrFltPortLinkEnableSet(dev,portNum,
                origLocalPortsEgfLink[portNum]);
            if(origLocalPortsEgfLink[portNum] == CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_LINK_STATE_BASED_E &&
                rc != GT_OK)
            {
                /* the port hold no MAC ... let's force it down */
                rc = cpssDxChBrgEgrFltPortLinkEnableSet(dev,portNum,
                    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E);
            }
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "cpssDxChBrgEgrFltPortLinkEnableSet: failed\n");

            rc = prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortMapSet(dev,portGroupId,
                portNum,
                origLocalPortsTxqPort[portNum]);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortMapSet: failed\n");


        }

        /* AUTODOC: invalidate VLAN 5 */
        prvTgfTrunkVlanRestore(testedVid);
    }

}

/* convert CPSS_PORT_SPEED_ENT to 'Mbps value' */
static GT_U32   speedGet(CPSS_PORT_SPEED_ENT speedEnum)
{
    GT_U32  speed;/*speed in Mbps !*/
    const GT_U32  Gbps = 1000;/*Gbps is 1000 Mbps */
    switch(speedEnum)
    {
        case CPSS_PORT_SPEED_10_E:     speed=10;        break;
        case CPSS_PORT_SPEED_100_E:    speed=100;       break;
        case CPSS_PORT_SPEED_1000_E:   speed=1*Gbps;    break;
        case CPSS_PORT_SPEED_10000_E:  speed=10*Gbps;   break;
        case CPSS_PORT_SPEED_12000_E:  speed=12*Gbps;   break;
        case CPSS_PORT_SPEED_2500_E:   speed=2500;      break;
        case CPSS_PORT_SPEED_5000_E:   speed=5*Gbps;    break;
        case CPSS_PORT_SPEED_13600_E:  speed=13600;     break;
        case CPSS_PORT_SPEED_20000_E:  speed=20*Gbps;   break;
        case CPSS_PORT_SPEED_40000_E:  speed=40*Gbps;   break;
        case CPSS_PORT_SPEED_16000_E:  speed=16*Gbps;   break;
        case CPSS_PORT_SPEED_15000_E:  speed=15*Gbps;   break;
        case CPSS_PORT_SPEED_75000_E:  speed=75*Gbps;   break;
        case CPSS_PORT_SPEED_100G_E:   speed=100*Gbps;  break;
        case CPSS_PORT_SPEED_50000_E:  speed=50*Gbps;   break;
        case CPSS_PORT_SPEED_140G_E:   speed=140*Gbps;  break;

        case CPSS_PORT_SPEED_11800_E:  speed=11800;     break;
        case CPSS_PORT_SPEED_47200_E:  speed=47200;     break;
        case CPSS_PORT_SPEED_22000_E:  speed=22*Gbps;   break;
        case CPSS_PORT_SPEED_23600_E:  speed=23600;     break;
        case CPSS_PORT_SPEED_12500_E:  speed=12500;     break;
        case CPSS_PORT_SPEED_25000_E:  speed=25*Gbps;   break;

        default:                       speed=0;         break;

    }

    return speed;
}

/**
* @internal selectPortsIndexes function
* @endinternal
*
* @brief   select ports indexes according to speed of ports
*/
static void selectPortsIndexes(IN GT_BOOL  enable)
{
    GT_STATUS   rc;
    GT_U8   dev = prvTgfDevNum;
    GT_U32  portNum;
    CPSS_PORT_SPEED_ENT speedArr[4] = {0};
    GT_U32  ii;

    /* some ports in 7 bits may not be OK when reduced bits in the test.
        the next function will remove those ports from the test.

        will be restored at the end of the test (by the generic engine)
    */
    if(enable == GT_TRUE)
    {
        /* the init_4k_members(...) hold logic of: (currentPortMask / 2) + 1 which equivalent to '7 bits' */
        prvTgfFilterLimitedBitsSpecialPortsArray(
                7/*7 bits*/,
                COLLIDE_PORT_OFFSET,
                prvTgfPortsArray,
                8);/* the test is using up to 4 ports , need place for next filter */

        /* run next after that we may have changed ports */
        PRV_UTF_LOG0_MAC("======= clear counters =======\n");
        /* clear also counters at end of test */
        clearAllTestedCounters();

        PRV_UTF_LOG0_MAC("======= force links up =======\n");
        /* force linkup on all ports involved */
        forceLinkUpOnAllTestedPorts();
    }


    /*  the defaults are :
        cascadePortIndex = 2
        ingressPortIndex = 3
        otherPortIndex   = 1 */

    for(ii = 0 ; ii <= 3; ii++)
    {
        portNum = prvTgfPortsArray[ii];

        if(tgfTrunkCrcHash_4K_members_known_UC_EPCL_forcePortToMaxSpeed)
        {
            /* force the port to run in max supported */
            rc = prvTgfPortMaxSpeedForce(dev, portNum,enable);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfPortMaxSpeedForce: failed on port [%d] \n",
                portNum);
        }


        rc = prvTgfPortSpeedGet(dev, portNum, &speedArr[ii]);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfPortSpeedGet: failed\n");

        PRV_UTF_LOG2_MAC("======= port [%d] with speed of [%d]Mbps =======\n",
            prvTgfPortsArray[ii],
            speedGet(speedArr[ii]));
    }

    if(enable == GT_FALSE)
    {
        /* restore the defaults */
        cascadePortIndex = 2;
        ingressPortIndex = 3;
        otherPortIndex   = 1;

        return;
    }

    /* we need the ingress port to be the slowest to not get tail drops on
       egress ports on the HW */
    if(speedGet(speedArr[ingressPortIndex]) >
       speedGet(speedArr[cascadePortIndex]))
    {
        SWAP(ingressPortIndex,cascadePortIndex);
    }

    if(speedGet(speedArr[ingressPortIndex]) >
       speedGet(speedArr[otherPortIndex]))
    {
        SWAP(ingressPortIndex,otherPortIndex);
    }

    PRV_UTF_LOG2_MAC("======= selected 'ingress' port [%d] with speed of [%d]Mbps =======\n",
        prvTgfPortsArray[ingressPortIndex],
        speedGet(speedArr[ingressPortIndex]));

    PRV_UTF_LOG2_MAC("======= selected 'cascade' port [%d] with speed of [%d]Mbps =======\n",
        prvTgfPortsArray[cascadePortIndex],
        speedGet(speedArr[cascadePortIndex]));

    PRV_UTF_LOG2_MAC("======= selected 'other' port [%d] with speed of [%d]Mbps =======\n",
        prvTgfPortsArray[otherPortIndex],
        speedGet(speedArr[otherPortIndex]));

}

/**
* @internal init_tgfTrunkCrcHash_4K_members_known_UC_EPCL function
* @endinternal
*
* @brief   init for test : tgfTrunkCrcHash_4K_members_known_UC_EPCL
*/
static void init_tgfTrunkCrcHash_4K_members_known_UC_EPCL
(
    void
)
{
    /* select ports indexes according to speed of ports */
    selectPortsIndexes(GT_TRUE);


    PRV_UTF_LOG0_MAC("======= trunk config =======\n");
    trunk_configInit();
    PRV_UTF_LOG0_MAC("======= EPCL config =======\n");
    epcl_configInit();
    PRV_UTF_LOG0_MAC("======= device map table config =======\n");
    remoteDevicesMappingSet(GT_TRUE);
    PRV_UTF_LOG0_MAC("======= CNC config =======\n");
    cnc_configSet(GT_TRUE);
    PRV_UTF_LOG0_MAC("======= FDB config =======\n");
    fdb_configSet(GT_TRUE);
    PRV_UTF_LOG0_MAC("======= VLAN config =======\n");
    vlan_configSet(GT_TRUE);
}

/* function duplicates traffic info .
NOTE:
    caller need to call freeDuplicatedTrafficInfo() to release the dynamic allocations;
*/
static TGF_PACKET_STC      *deepCopyTrafficInfo
(
    IN TGF_PACKET_STC      *trafficPtr
)
{
    TGF_PACKET_STC      *dupTrafficPtr;
    TGF_PACKET_PART_STC *currPartPtr;
    TGF_PACKET_PART_STC *dupPartPtr;
    GT_U32  ii;
    GT_U32  partSize;
    GT_U8              *dataPtr;

    dupTrafficPtr = cpssOsMalloc(sizeof(TGF_PACKET_STC));
    *dupTrafficPtr = *trafficPtr;

    dupTrafficPtr->partsArray =
        cpssOsMalloc(trafficPtr->numOfParts * sizeof(TGF_PACKET_PART_STC));

    currPartPtr = &trafficPtr->partsArray[0];

    for(ii = 0 ; ii < trafficPtr->numOfParts ; ii++,currPartPtr++)
    {
        dupTrafficPtr->partsArray[ii] = *currPartPtr;

        switch(currPartPtr->type)
        {
            case TGF_PACKET_PART_L2_E:
                partSize = sizeof(TGF_PACKET_L2_STC);
                break;
            case TGF_PACKET_PART_VLAN_TAG_E:
                partSize = sizeof(TGF_PACKET_VLAN_TAG_STC);
                break;
            case TGF_PACKET_PART_DSA_TAG_E:
                partSize = sizeof(TGF_PACKET_DSA_TAG_STC);
                break;
            case TGF_PACKET_PART_ETHERTYPE_E:
                partSize = sizeof(TGF_PACKET_ETHERTYPE_STC);
                break;
            case TGF_PACKET_PART_MPLS_E:
                partSize = sizeof(TGF_PACKET_MPLS_STC);
                break;
            case TGF_PACKET_PART_IPV4_E:
                partSize = sizeof(TGF_PACKET_IPV4_STC);
                break;
            case TGF_PACKET_PART_IPV6_E:
                partSize = sizeof(TGF_PACKET_IPV6_STC);
                break;
            case TGF_PACKET_PART_TCP_E:
                partSize = sizeof(TGF_PACKET_TCP_STC);
                break;
            case TGF_PACKET_PART_UDP_E:
                partSize = sizeof(TGF_PACKET_UDP_STC);
                break;
            case TGF_PACKET_PART_ARP_E:
                partSize = sizeof(TGF_PACKET_ARP_STC);
                break;
            case TGF_PACKET_PART_ICMP_E:
                partSize = sizeof(TGF_PACKET_ICMP_STC);
                break;
            case TGF_PACKET_PART_WILDCARD_E:
                partSize = sizeof(TGF_PACKET_WILDCARD_STC);
                break;
            case TGF_PACKET_PART_PAYLOAD_E:
                partSize = sizeof(TGF_PACKET_PAYLOAD_STC);
                break;
            case TGF_PACKET_PART_TRILL_E:
                partSize = sizeof(TGF_PACKET_TRILL_STC);
                break;
            case TGF_PACKET_PART_TRILL_FIRST_OPTION_E:
                partSize = sizeof(TGF_PACKET_TRILL_FIRST_OPTION_STC);
                break;
            case TGF_PACKET_PART_TRILL_GENERAL_OPTION_E:
                partSize = sizeof(TGF_PACKET_TRILL_GEN_OPTION_STC);
                break;
            case TGF_PACKET_PART_GRE_E:
                partSize = sizeof(TGF_PACKET_GRE_STC);
                break;
            case TGF_PACKET_PART_TUNNEL_HEADER_FORCE_TILL_PASSENGER_E:
                partSize = sizeof(TGF_PACKET_TUNNEL_HEADER_FORCE_TILL_PASSENGER_STC);
                break;
            case TGF_PACKET_PART_CRC_E:
                partSize = sizeof(TGF_PACKET_CRC_STC);
                break;
            case TGF_PACKET_PART_PTP_V2_E:
                partSize = sizeof(TGF_PACKET_PTP_V2_STC);
                break;
            case TGF_PACKET_PART_TEMPLATE_E:
                partSize = sizeof(TGF_PACKET_TEMPLATE16_STC);
                break;
            default:
            case TGF_PACKET_PART_SKIP_E:
                partSize = 0;
                break;
        }

        if(partSize)
        {
            dupPartPtr  = cpssOsMalloc(partSize);
            cpssOsMemCpy(dupPartPtr,currPartPtr->partPtr,partSize);
            dupTrafficPtr->partsArray[ii].partPtr = dupPartPtr;

            switch(currPartPtr->type)
            {
                case TGF_PACKET_PART_PAYLOAD_E :
                case TGF_PACKET_PART_WILDCARD_E:
                case TGF_PACKET_PART_TEMPLATE_E:
                case TGF_PACKET_PART_CRC_E:
                case TGF_PACKET_PART_TUNNEL_HEADER_FORCE_TILL_PASSENGER_E:
                    /****************************/
                    /* we also need the payload */
                    /****************************/

                    partSize = ((TGF_PACKET_PAYLOAD_STC*)dupPartPtr)->dataLength;
                    ((TGF_PACKET_PAYLOAD_STC*)dupPartPtr)->dataPtr  = cpssOsMalloc(partSize);

                    dataPtr = ((TGF_PACKET_PAYLOAD_STC*)currPartPtr->partPtr)->dataPtr;

                    cpssOsMemCpy(((TGF_PACKET_PAYLOAD_STC*)dupPartPtr)->dataPtr,dataPtr,partSize);

                    break;
                default:
                    break;
            }
        }
        else
        {
            dupTrafficPtr->partsArray[ii].partPtr = NULL;
        }
    }

    return dupTrafficPtr;
}

/* free dynamic allocations that done by deepCopyTrafficInfo(...) */
static void freeDuplicatedTrafficInfo(
    IN TGF_PACKET_STC      *dupTrafficPtr
)
{
    TGF_PACKET_PART_STC *currPartPtr;
    GT_VOID *partPtr;
    GT_U32  ii;


    currPartPtr = &dupTrafficPtr->partsArray[0];

    for(ii = 0 ; ii < dupTrafficPtr->numOfParts ; ii++,currPartPtr++)
    {
        switch(currPartPtr->type)
        {
            case TGF_PACKET_PART_L2_E:
            case TGF_PACKET_PART_VLAN_TAG_E:
            case TGF_PACKET_PART_DSA_TAG_E:
            case TGF_PACKET_PART_ETHERTYPE_E:
            case TGF_PACKET_PART_MPLS_E:
            case TGF_PACKET_PART_IPV4_E:
            case TGF_PACKET_PART_IPV6_E:
            case TGF_PACKET_PART_TCP_E:
            case TGF_PACKET_PART_UDP_E:
            case TGF_PACKET_PART_ARP_E:
            case TGF_PACKET_PART_ICMP_E:
            case TGF_PACKET_PART_WILDCARD_E:
            case TGF_PACKET_PART_PAYLOAD_E:
            case TGF_PACKET_PART_TRILL_E:
            case TGF_PACKET_PART_TRILL_FIRST_OPTION_E:
            case TGF_PACKET_PART_TRILL_GENERAL_OPTION_E:
            case TGF_PACKET_PART_GRE_E:
            case TGF_PACKET_PART_TUNNEL_HEADER_FORCE_TILL_PASSENGER_E:
            case TGF_PACKET_PART_CRC_E:
            case TGF_PACKET_PART_PTP_V2_E:
            case TGF_PACKET_PART_TEMPLATE_E:
                partPtr = currPartPtr->partPtr;
                break;
            default:
            case TGF_PACKET_PART_SKIP_E:
                partPtr = NULL;
                break;
        }

        if(partPtr)
        {
            switch(currPartPtr->type)
            {
                case TGF_PACKET_PART_PAYLOAD_E :
                case TGF_PACKET_PART_WILDCARD_E:
                case TGF_PACKET_PART_TEMPLATE_E:
                case TGF_PACKET_PART_CRC_E:
                case TGF_PACKET_PART_TUNNEL_HEADER_FORCE_TILL_PASSENGER_E:
                    /****************************/
                    /* we also need the payload */
                    /****************************/
                    cpssOsFree(((TGF_PACKET_PAYLOAD_STC*)partPtr)->dataPtr);
                    break;
                default:
                    break;
            }

            cpssOsFree(partPtr);
        }
    }

    cpssOsFree(dupTrafficPtr->partsArray);

    cpssOsFree(dupTrafficPtr);

    return;
}

/**
* @internal checkResults_tgfTrunkCrcHash_4K_members_known_UC_EPCL function
* @endinternal
*
* @brief   check results for test : tgfTrunkCrcHash_4K_members_known_UC_EPCL
*/
static void checkResults_tgfTrunkCrcHash_4K_members_known_UC_EPCL
(
    void
)
{
    GT_STATUS                         rc;
    GT_U32                            blockNum;
    GT_U32                            counterIdx;
    PRV_TGF_CNC_COUNTER_FORMAT_ENT    format=PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    PRV_TGF_CNC_COUNTER_STC           counter;
    GT_U32      expectedCounter;
    GT_U32      ii;
    GT_U32      index;
    GT_U32      globalCounter = 0;
    GT_U32      actualNumOfIndexes;
    GT_U32      maxTrunkMembers;
    GT_HW_DEV_NUM  own_hwDevNum;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCounters;


    if(floodTesting == GT_FALSE)
    {
        /* print the BMP of indexes that are expected */
        actualNumOfIndexes =
            printBitmapArr("trunk 4K members : expected Indexes : (bitmap)",
            expectedEgressMemberBmpArr , _4K,
            0xFFFFFFFF/*expected_numOfSetBits -- no compare*/);

        PRV_UTF_LOG3_MAC("The Load balance of the [%d] sent packets to[%d] members, reached [%d] different members \n",
            burstCount,
            actualNumOfMembersInTrunk ,
            actualNumOfIndexes);

        /* we need to check 'reasonable' load balance */
        /* lets say 1/16 of the ports may not get packet any */
        if((maxExpectedMemberToGetTraffic >= 16) &&
           (actualNumOfIndexes < actualNumOfMembersInTrunk))
        {
            if((actualNumOfIndexes + (maxExpectedMemberToGetTraffic/16)) > maxExpectedMemberToGetTraffic)
            {
                /* good ... less than 1/16 of the members not get any packet */
                PRV_UTF_LOG1_MAC("we expect good LBH less than 1/16 (%d) of the members not get any packet \n",
                    maxExpectedMemberToGetTraffic-actualNumOfIndexes);
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL,
                    "we expect BAD LBH : more than 1/16 (%d) of the members not get any packet \n",
                        maxExpectedMemberToGetTraffic-actualNumOfIndexes);
            }
        }

        maxTrunkMembers = _4K;
    }
    else
    {
        maxTrunkMembers = FLOOD_MAX_MEMBERS_CNS;
    }

    own_hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    for(ii = 0 ; ii < maxTrunkMembers ; ii++)
    {
        index = CONVERT_TRUNK_MEMBER_TO_CNC_INDEX_MAC(ii);

        /* use here 'index' */
        blockNum   = index / cncBlockCntrs;
        counterIdx = index % cncBlockCntrs;

        if(floodTesting == GT_FALSE)
        {
            /* use here 'ii' */
            expectedCounter = expectedEgressMemberCounterArr[ii];
        }
        else
        {
            expectedCounter = 0;/* dummy not used*/
        }

        rc = prvTgfCncCounterGet(blockNum, counterIdx, format, &counter);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterGet");

        if(floodTesting == GT_FALSE)
        {
            UTF_VERIFY_EQUAL5_STRING_MAC(
                expectedCounter, counter.packetCount.l[0],
                "Trunk Member at index [%d] {hwDev[%d],port[%d]} expected CNC value [%d] received [%d]",
                ii,
                membersArray_set_4K_tests[ii].hwDevice,
                membersArray_set_4K_tests[ii].port,
                expectedCounter, counter.packetCount.l[0]);
        }
        else
        {
            if(membersArray_set_4K_tests[ii].hwDevice != own_hwDevNum)
            {
                /* the flooding to remote ports is not expected at this device counters */
                expectedCounter = 0;
                UTF_VERIFY_EQUAL5_STRING_MAC(
                    expectedCounter , counter.packetCount.l[0],
                    "'Remote' Trunk Member at index [%d] {hwDev[%d],port[%d]} expected CNC value '0' received [%d]",
                    ii,
                    membersArray_set_4K_tests[ii].hwDevice,
                    membersArray_set_4K_tests[ii].port,
                    expectedCounter, counter.packetCount.l[0]);
            }
            else
            if(ii >= 64)
            {
                /* only first 64 members of the trunk may get traffic */

                expectedCounter = 0;
                UTF_VERIFY_EQUAL5_STRING_MAC(
                    expectedCounter , counter.packetCount.l[0],
                    "Trunk Member at index [%d] {hwDev[%d],port[%d]} expected CNC value '0' received [%d]",
                    ii,
                    membersArray_set_4K_tests[ii].hwDevice,
                    membersArray_set_4K_tests[ii].port,
                    expectedCounter, counter.packetCount.l[0]);
            }
            else
            {
                if(burstCount >= (64 * 8))
                {
                    expectedCounter = 0;/* NOT '0' ... we use MACRO 'NOT_EQUAL' */
                    /* since we send a large number of packets ... lets assume
                        that ALL 'first' ports got at least single packet */
                    UTF_VERIFY_NOT_EQUAL3_STRING_MAC(
                        expectedCounter , counter.packetCount.l[0],
                        "Trunk Member at index [%d] {hwDev[%d],port[%d]} expected CNC value NOT to be '0' received '0'",
                        ii,
                        membersArray_set_4K_tests[ii].hwDevice,
                        membersArray_set_4K_tests[ii].port);
                }
                else
                {
                    /*we not sent enough to be decisive */
                }
            }
        }

        globalCounter += counter.packetCount.l[0];


        if(prvUtfIsGmCompilation() == GT_TRUE &&
            counter.packetCount.l[0] != 0)
        {
            /* GM bug ! not clearing the CNC counter */
            counter.packetCount.l[0] = 0;
            rc = prvTgfCncCounterSet(blockNum, counterIdx, format, &counter);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterSet");
        }

    }

    if(floodTesting == GT_FALSE)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, globalCounter, "summary not match");
    }
    else
    {
        /* when flooding each packet also egress other ports in the vlan
           (that are not trunk members)

           so check that the 'cascade' port (was not really set as cascade) got all packets
        */
        /* read counters */
        rc = prvTgfReadPortCountersEth(
            prvTgfDevsArray[CASCADE_PORT_INDEX_CNS],
            prvTgfPortsArray[CASCADE_PORT_INDEX_CNS],
            GT_TRUE, &portCounters);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfReadPortCountersEth: failed\n");

        UTF_VERIFY_EQUAL2_STRING_MAC(
            burstCount,
            portCounters.goodPktsSent.l[0],
            "Expected all [%d] packets to flood to 'cascade' port , but got only [%d]\n",
            burstCount,
            portCounters.goodPktsSent.l[0]);



        if(deviceB_running == GT_FALSE)
        {
            deviceA_globalCounter = globalCounter;
        }
        else
        {
            /* the flooding is running on 2 devices ... so we can summary only after
               the second device finished */
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, globalCounter + deviceA_globalCounter, "summary not match");
        }
    }

}

/**
* @internal traffic_tgfTrunkCrcHash_4K_members_known_UC_EPCL function
* @endinternal
*
* @brief   traffic for test : tgfTrunkCrcHash_4K_members_known_UC_EPCL
*/
static void traffic_tgfTrunkCrcHash_4K_members_known_UC_EPCL
(
    void
)
{
    TGF_PACKET_STC      *exampleTrafficPtr = prvTgfTrunkPacketGet(packetType);/*prvTgfIpv4TcpPacketInfo*/
    TGF_PACKET_STC      *trafficPtr;
    GT_U32               numVfd = 1;/* number of VFDs in vfdArray */
    TGF_VFD_INFO_STC     vfdArray[1] = {{0}};/* vfd Array -- used for increment the tested bytes , and for vlan tag changing */
    TGF_VFD_INFO_STC     *vrfPtr = &vfdArray[0];
    GT_U32      l3Offset     = L2_VLAN_TAG_ETHERETYPE_SIZE_CNS;
    GT_U32      l4StartIndex = l3Offset + TGF_IPV4_HEADER_SIZE_CNS;/*ipv4 header*/
    GT_U32      offset;
    GT_U32  ingressPortNum;
    GT_U32  ii;
    GT_U32  memberIndex;
    GT_U8   dev = prvTgfDevNum;
    TGF_PACKET_TCP_STC *tcpPartPtr = NULL;
    GT_U32  startValue = 0xaa5555aa;
    GT_U32  currValue;

    if(exampleTrafficPtr == NULL)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, GT_BAD_PTR, "prvTgfTrunkPacketGet: failed\n");
        return;
    }

    /* copy the 'example' for our test that need to modify the packet */
    trafficPtr = deepCopyTrafficInfo(exampleTrafficPtr);

    /* the 2 fields of 'src/trg TCP ports' (2Bytes + 2 Bytes) */
    offset = l4StartIndex;

    vrfPtr->modeExtraInfo = 0;
    vrfPtr->offset = offset;
    cpssOsMemSet(vrfPtr->patternPtr,0,sizeof(TGF_MAC_ADDR));
    vrfPtr->patternPtr[0] = (GT_U8)(startValue>> 0);
    vrfPtr->patternPtr[1] = (GT_U8)(startValue>> 8);
    vrfPtr->patternPtr[2] = (GT_U8)(startValue>>16);
    vrfPtr->patternPtr[3] = (GT_U8)(startValue>>24);
    vrfPtr->cycleCount = 4;/* 4 bytes */

    vrfPtr->mode = TGF_VFD_MODE_INCREMENT_VALUE_E;

    for(ii = 0 ; ii < trafficPtr->numOfParts ; ii++)
    {
        if(trafficPtr->partsArray[ii].type == TGF_PACKET_PART_TCP_E)
        {
            tcpPartPtr = trafficPtr->partsArray[ii].partPtr;
            break;
        }
    }

    if(tcpPartPtr == NULL)
    {
        return;
    }

    /* those will get init value from startValue */
    tcpPartPtr->srcPort = 0;
    tcpPartPtr->dstPort = 0;

    vrfPtr->incValue = 0x1;

    ingressPortNum = prvTgfPortsArray[INGRESS_PORT_INDEX_CNS];

    maxExpectedMemberToGetTraffic = burstCount > actualNumOfMembersInTrunk ?
                                    actualNumOfMembersInTrunk :
                                    burstCount;


    /* send the burst of packets with incremental byte */
    prvTgfTrunkTestPacketSend(dev, ingressPortNum, trafficPtr ,burstCount ,numVfd ,vfdArray);

    if(floodTesting == GT_TRUE)
    {
        /* we are not calculating the expected ports */
        goto cleanExit_lbl;
    }

    cpssOsMemSet(expectedEgressMemberCounterArr,0,sizeof(expectedEgressMemberCounterArr));
    cpssOsMemSet(expectedEgressMemberBmpArr,0,sizeof(expectedEgressMemberBmpArr));

    /* we sent the traffic as 'single burst' ... but we need to calculate the
       egress ports one by one */

    currValue = startValue;
    for(ii = 0 ; ii < burstCount ; ii++, currValue += vrfPtr->incValue)
    {
        /* because we increment the bytes of TCP port , we need to emulate it
           in the packet */
        tcpPartPtr->srcPort = (GT_U16)((currValue)>>16);
        tcpPartPtr->dstPort = (GT_U16)((currValue)>> 0);

        /* do calc of calculatedHash_expectedTrunkMemberIndex */
        memberIndex = do_calculatedHash_expectedTrunkMemberIndex(trafficPtr,
            packetType,
            GT_FALSE/*usedUdb*/,
            0/*fieldByteIndex - not relevant when 'usedUdb = GT_FALSE' */,
            ingressPortNum/*localSrcPort*/,
            actualNumOfMembersInTrunk);

        if(ii < 64)
        {
            PRV_UTF_LOG2_MAC("Packet [%d] expected to egress memberIndex [%d] \n",
                ii,memberIndex);
        }

        /* increment the index of the expected member */
        /* will be used to check CNC counters */
        expectedEgressMemberCounterArr[memberIndex]++;

        /* set bmp bit for this index too */
        expectedEgressMemberBmpArr[memberIndex >> 5] |= 1 << (memberIndex & 0x1f);
    }

cleanExit_lbl:
    /* free all dynamic memories that allocated to duplicate our own copy for the test */
    freeDuplicatedTrafficInfo(trafficPtr);
    /* state that we are not using it any more ! */
    trafficPtr = NULL;
}

/**
* @internal epcl_configRestore function
* @endinternal
*
* @brief   restore the EPCL config needed for 4K members
*/
static GT_VOID epcl_configRestore
(
    GT_VOID
)
{
    epcl_egressPortsSet(GT_FALSE);
    epcl4K_members_Rules(GT_FALSE);
}

/**
* @internal trunk_configRestore function
* @endinternal
*
* @brief   restore the trunk config needed for 4K members
*/
static GT_VOID trunk_configRestore
(
    GT_VOID
)
{
    GT_STATUS   st;
    GT_U8   dev = prvTgfDevNum;

    /* reset trunk */
    st = cpssDxChTrunkMembersSet(dev,trunkId_for_4K_members,
                0,
                NULL,
                0,
                NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkMembersSet: %d", trunkId_for_4K_members);

    /* AUTODOC: restore enhanced crc hash data */
    /* restore global hash mode */
    st =  prvTgfTrunkHashGlobalModeSet(globalHashModeGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfTrunkHashGlobalModeSet: %d", prvTgfDevNum);

    /* restore crc hash parameters */
    st =  prvTgfTrunkHashCrcParametersSet(crcHashModeGet, crcSeedGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfTrunkHashCrcParametersSet: %d", prvTgfDevNum);

    st = cpssDxChTrunkMemberSelectionModeSet(dev,CPSS_DXCH_MEMBER_SELECTION_MODE_6_LSB_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkMemberSelectionModeSet: %d", prvTgfDevNum);

    /* now we can destroy the trunk LIB and generate new init with orig mode */
    st = cpssDxChTrunkDestroy(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

    st = cpssDxChTrunkInit(dev , orig_maxNumberOfTrunks,orig_trunkMembersMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
}

/**
* @internal restore_tgfTrunkCrcHash_4K_members_known_UC_EPCL function
* @endinternal
*
* @brief   restore for test : tgfTrunkCrcHash_4K_members_known_UC_EPCL
*/
static void restore_tgfTrunkCrcHash_4K_members_known_UC_EPCL
(
    void
)
{
    trunk_configRestore();
    epcl_configRestore();
    remoteDevicesMappingSet(GT_FALSE);
    cnc_configSet(GT_FALSE);
    fdb_configSet(GT_FALSE);
    vlan_configSet(GT_FALSE);

    /* restore ports indexes according to speed of ports */
    selectPortsIndexes(GT_FALSE);

}

/**
* @internal tgfTrunkCrcHash_4K_members_known_UC_EPCL function
* @endinternal
*
* @brief   set 4K members in a trunk
*         send 'known UC' traffic to it , check by EPCL that bound to CNC counters
*         that the members got traffic according to 'predicted' egress ports.
*/
void tgfTrunkCrcHash_4K_members_known_UC_EPCL
(
    void
)
{
    /* init */
    PRV_UTF_LOG0_MAC("======= INI config =======\n");
    init_tgfTrunkCrcHash_4K_members_known_UC_EPCL();

    PRV_UTF_LOG0_MAC("======= send traffic =======\n");
    /* send traffic */
    traffic_tgfTrunkCrcHash_4K_members_known_UC_EPCL();

    PRV_UTF_LOG0_MAC("======= check results =======\n");
    /* check results */
    checkResults_tgfTrunkCrcHash_4K_members_known_UC_EPCL();

    PRV_UTF_LOG0_MAC("======= restore config =======\n");
    /* restore */
    restore_tgfTrunkCrcHash_4K_members_known_UC_EPCL();

    PRV_UTF_LOG0_MAC("======= ended =======\n");
}

/**
* @internal tgfTrunk_80_members_HwDevNum_deviceB_set function
* @endinternal
*
* @brief   check that only first 64 members of the trunk get flooded traffic.
*         set 80 members in a trunk
*         send 4K flows of 'unknown' traffic to it , check by EPCL that bound to CNC counters
*         that only first 64 members of the trunk got traffic.
*         NOTE:
*         the test runs as '2 devices' : A and B
*         members in the trunk are ports from : A and B :
*         'first 64 ports : 25 ports from A , 39 ports from B
*         next 16 ports  : 8 ports from A , 8 ports from B
*         total 80 members
*         first run as device A :
*         send flood traffic , check that only those 25 ports got traffic and not
*         the other 8
*         second run as device B :
*         send flood traffic (the same traffic) , check that only those 39 ports got traffic and not
*         the other 8
*         NOTE: there is no use of DSA traffic , and the generating flood is network port
*         both on device A and on device B
*/
static void tgfTrunk_80_members_HwDevNum_deviceB_set(void)
{
    GT_STATUS   rc;
    GT_HW_DEV_NUM   hwDevNum;
    GT_U8   dev = prvTgfDevNum;

    if(deviceB_running == GT_TRUE)
    {
        hwDevNum = deviceB_hwDevNum;
    }
    else
    {
        /* restore to device A */
        hwDevNum = deviceA_hwDevNum;
    }

    rc = appDemoDxHwDevNumChange(dev,hwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDxHwDevNumChange");


}

/**
* @internal tgfTrunk_80_members_flood_EPCL function
* @endinternal
*
* @brief   set 'Device B' mode
*/
void tgfTrunk_80_members_flood_EPCL
(
    void
)
{
    floodTesting = GT_TRUE;
    deviceB_running = GT_FALSE;

    PRV_UTF_LOG0_MAC("======= Start Running on 'device A' =======\n");
    tgfTrunkCrcHash_4K_members_known_UC_EPCL();
    PRV_UTF_LOG0_MAC("======= Ended Running on 'device A' =======\n");

    deviceB_running = GT_TRUE;
    /* set 'Device B' mode */
    tgfTrunk_80_members_HwDevNum_deviceB_set();

    PRV_UTF_LOG0_MAC("======= Start Running on 'device B' =======\n");
    tgfTrunkCrcHash_4K_members_known_UC_EPCL();
    PRV_UTF_LOG0_MAC("======= Ended Running on 'device B' =======\n");

    floodTesting = GT_FALSE;
    deviceB_running = GT_FALSE;

    /* restore 'Device A' mode */
    tgfTrunk_80_members_HwDevNum_deviceB_set();
}


