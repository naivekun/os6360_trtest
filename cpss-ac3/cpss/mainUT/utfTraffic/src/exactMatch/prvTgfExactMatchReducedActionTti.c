/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *2
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvTgfExactMatchReducedActionTti.c
*
* @brief Configuration for Exact Match
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfBridgeGen.h>
#include <common/tgfCommon.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <exactMatch/prvTgfExactMatchReducedActionTti.h>
#include <tunnel/prvTgfTunnelTermEtherType.h>


/******************************* Test packet **********************************/

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS             1

/* port to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number to receive traffic from  */
#define PRV_TGF_EGR1_PORT_IDX_CNS       1
#define PRV_TGF_EGR2_PORT_IDX_CNS       2
#define PRV_TGF_EGR3_PORT_IDX_CNS       3

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

#define PRV_TGF_CPU_PORT_IDX_CNS        3

/* profileId number to set */
#define PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS 9

#define PRV_TGF_NUM_OF_UDB_CNS        7

/* cpu code to set */
#define PRV_TGF_EXACT_MATCH_CPU_CODE_CNS    CPSS_NET_FIRST_USER_DEFINED_E + 3

/* default tunnel term entry index */
#define PRV_TGF_TTI_INDEX_CNS       1

/* default number of packets to send */
static GT_U32       prvTgfBurstCount = 1;



/* parameters that is needed to be restored */
static struct
{
    GT_U32                              sendPort;
} prvTgfRestoreCfg;

static struct
{
    GT_BOOL                                           profileIdEn;
    GT_U32                                            profileId;
    PRV_TGF_EXACT_MATCH_CLIENT_ENT                    firstLookupClientType;
    PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC        keyParams;

    GT_BOOL                                           exactMatchEntryIndex;
    GT_BOOL                                           exactMatchEntryValid;
    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT               exactMatchEntryActionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT                    exactMatchEntryAction;
    PRV_TGF_EXACT_MATCH_ENTRY_STC                     exactMatchEntry;
    GT_U32                                            exactMatchEntryExpandedActionIndex;

} prvTgfExactMatchRestoreCfg;

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x44, 0x33, 0x22, 0x11},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                /* srcMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};
/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x01, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* packet to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,         /* totalLen   */
    (sizeof(prvTgfPacketPartArray)
        / sizeof(prvTgfPacketPartArray[0])), /* numOfParts */
    prvTgfPacketPartArray                    /* partsArray */
};

static GT_BOOL prvTgfActionTrapToCpuCheck[] = {
    GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE, GT_FALSE, GT_FALSE
};
/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/****************************************************************************************************************************************************/
/****************************************************************************************************************************************************/

/*Reset the port counters */
static GT_STATUS prvTgfCountersEthReset
(
    GT_VOID
)
{
    GT_U32    portIter;
    GT_STATUS rc, rc1 = GT_OK;

    for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfResetCountersEth");
    }

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    return rc1;
};

/**
* @internal prvTgfTunnelTermEtherTypeOtherTestInit function
* @endinternal
*
* @brief   Set TTI test settings:
*         - Enable port 0 for Eth lookup
*         - Set Eth key lookup MAC mode to Mac DA
*         - Set TTI rule action
*         - Set TTI rule
* @param[in] sendPortNum              - port number to send packet
* @param[in] vrfId                    - assign VRF ID for packet
*                                      GT_TRUE  - assign VRF ID for packet
*                                      GT_FALSE - not assign VRF ID for packet
* @param[in] vrfId                    - VRF ID to set
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Used 7 first bytes of anchor 'L3 offset - 2'.
*
*/
static GT_VOID prvTgfExactMatchReducedActionTtiGenericConfigInit
(
    IN  GT_U32                        sendPortNum,
    IN  GT_BOOL                       vrfIdAssign,
    IN  GT_U32                        vrfId
)
{
    GT_STATUS                   rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC    ttiAction ;
    PRV_TGF_TTI_RULE_UNT        ttiPattern;
    PRV_TGF_TTI_RULE_UNT        ttiMask;
    GT_U32 ii;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* AUTODOC: enable TTI lookup for port 0, key TTI_KEY_UDB_ETHERNET_OTHER_E */
    rc = prvTgfTtiPortLookupEnableSet(sendPortNum,
                                PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);


    /* set the TTI Rule Pattern, Mask and Action for Ether Type Key */

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet((GT_VOID*) &ttiPattern,0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,   0, sizeof(ttiMask));

    /* set TTI Action */
    if (vrfIdAssign == GT_FALSE)
    {
        ttiAction.command                       = CPSS_PACKET_CMD_DROP_HARD_E;
        ttiAction.redirectCommand               = PRV_TGF_TTI_NO_REDIRECT_E;
        ttiAction.tunnelTerminate               = GT_TRUE;
        ttiAction.vrfId                         = 0;
    }
    else
    {
        ttiAction.command                       = CPSS_PACKET_CMD_FORWARD_E;
        ttiAction.redirectCommand               = PRV_TGF_TTI_VRF_ID_ASSIGN_E;
        ttiAction.tunnelTerminate               = GT_FALSE;
        ttiAction.vrfId                         = vrfId;
    }

    if (vrfIdAssign == GT_FALSE)
    {
        ttiPattern.eth.common.vid           = PRV_TGF_VLANID_CNS;
        ttiPattern.eth.common.isTagged      = GT_TRUE;
    }
    else
    {
        /* don't care of VLAN in the packet */
        ttiPattern.eth.common.vid           = 0;
        ttiPattern.eth.common.isTagged      = GT_FALSE;
    }

    for(ii = 0; ii < PRV_TGF_NUM_OF_UDB_CNS ; ii++)
    {
        ttiPattern.udbArray[ii] = prvTgfPayloadDataArr[ii+3];
        ttiMask.udbArray[ii] = 0xff;
    }

    PRV_UTF_LOG0_MAC("======= Setting UDB Configuration =======\n");

    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum,
                            PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
                            PRV_TGF_TTI_KEY_SIZE_10_B_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);
    /* AUTODOC: set TTI rule */

    rc = prvTgfTtiRuleUdbSet(PRV_TGF_TTI_INDEX_CNS,PRV_TGF_TTI_RULE_UDB_10_E,
                             &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleUdbSet");

    for(ii = 0; ii < PRV_TGF_NUM_OF_UDB_CNS ; ii++)
    {
        rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                        PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E, ii,
                        PRV_TGF_TTI_OFFSET_L3_MINUS_2_E,(GT_U8) ii);
    }

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d", prvTgfDevNum, GT_FALSE);

}

/**
* @internal prvTgfExactMatchReducedActionTtiGenericConfigSet function
* @endinternal
*
* @brief   Set TTI generic test configuration, not related to Exact Match
*
* @note Used 7 first bytes of anchor 'L3 offset - 2'.
*
*/
GT_VOID prvTgfExactMatchReducedActionTtiGenericConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    prvTgfRestoreCfg.sendPort = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    if(MUST_USE_ORIG_PORTS_MAC(prvTgfDevNum))
    {
        /* do not modify the 12 ports in prvTgfPortsArray[] !!! */
    }
    else
    {
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] = 16;
    }
    /* init configuration */
    prvTgfExactMatchReducedActionTtiGenericConfigInit(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],GT_TRUE, 0);
}

/**
* @internal prvTgfExactMatchReducedActionTtiConfigSet function
* @endinternal
*
* @brief Set TTI test configuration related to Exact Match Reduced Action
*
*/
GT_VOID prvTgfExactMatchReducedActionTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc;

    PRV_TGF_EXACT_MATCH_CLIENT_ENT                  firstLookupClientType;
    PRV_TGF_TTI_KEY_TYPE_ENT                        packetType;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT                  lookupNum;

    GT_BOOL                                         enableExactMatchLookup;
    GT_U32                                          profileId;

    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT             actionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT                  actionData;

    PRV_TGF_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT  expandedActionOrigin;
    PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC      keyParams;

    GT_PORT_GROUPS_BMP                              portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U32                                          expandedActionIndex;
    PRV_TGF_EXACT_MATCH_ENTRY_STC                   exactMatchEntry;

    GT_U32                                          index=0;
    GT_U32                                          numberOfElemInCrcMultiHashArr;
    GT_U32                                          exactMatchCrcMultiHashArr[16];



    /* clear entry */
    cpssOsMemSet((GT_VOID*) &actionData, 0, sizeof(actionData));

    /* take all fields from Expanded Action: all fields overwriteExpande=GT_FALSE*/
    cpssOsMemSet((GT_VOID*) &expandedActionOrigin, 0, sizeof(expandedActionOrigin));


    lookupNum                   = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    firstLookupClientType       = PRV_TGF_EXACT_MATCH_CLIENT_TTI_E;
    packetType                  = PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E;

    enableExactMatchLookup      = GT_TRUE;
    profileId                   = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;
    expandedActionIndex         = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;
    actionType                  = PRV_TGF_EXACT_MATCH_ACTION_TTI_E ;

    actionData.ttiAction.command            = PRV_TGF_PACKET_CMD_TRAP_TO_CPU_E ;
    actionData.ttiAction.userDefinedCpuCode = PRV_TGF_EXACT_MATCH_CPU_CODE_CNS;

    keyParams.keySize           =CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
    keyParams.keyStart          =0;
    cpssOsMemSet((GT_VOID*) &keyParams.mask[0], 0, sizeof(keyParams.mask));

    exactMatchEntry.key.keySize = PRV_TGF_EXACT_MATCH_KEY_SIZE_33B_E;
    exactMatchEntry.lookupNum   = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    /* here we check exactly the same pattern set for the TTI match in TCAM - all zero */
    cpssOsMemSet((GT_VOID*) &exactMatchEntry.key.pattern[0], 0, sizeof(exactMatchEntry.key.pattern));


    /* AUTODOC: calculate index */
    rc =  prvTgfExactMatchHashCrcMultiResultsByParamsCalc(prvTgfDevNum,
                                                          &exactMatchEntry.key,
                                                          &numberOfElemInCrcMultiHashArr,
                                                          exactMatchCrcMultiHashArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchHashCrcMultiResultsByParamsCalc FAILED: %d", prvTgfDevNum);

    if (numberOfElemInCrcMultiHashArr!=0)
    {
        index = exactMatchCrcMultiHashArr[numberOfElemInCrcMultiHashArr-3];
    }
    else
    {
        rc = GT_BAD_VALUE;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "numberOfElemInCrcMultiHashArr is 0  - illegal - FAILED");
    }
    prvTgfExactMatchRestoreCfg.exactMatchEntryIndex=index;


    /*save config */

    /* AUTODOC: keep clientType configured for first lookup , for restore */
    rc = prvTgfExactMatchClientLookupGet(prvTgfDevNum,lookupNum,&prvTgfExactMatchRestoreCfg.firstLookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: keep profileId configured for first lookup , for restore */
    rc = prvTgfExactMatchTtiProfileIdGet(prvTgfDevNum, packetType, lookupNum , &prvTgfExactMatchRestoreCfg.profileIdEn ,
                                         &prvTgfExactMatchRestoreCfg.profileId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiExactMatchProfileIdGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: save Profile Key Params , for restore */
    rc =  prvTgfExactMatchProfileKeyParamsGet(prvTgfDevNum,profileId ,
                                           &prvTgfExactMatchRestoreCfg.keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchProfileKeyParamsGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: save Exact Match Entry , for restore */
    rc =  prvTgfExactMatchPortGroupEntryGet(prvTgfDevNum,portGroupsBmp,index,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryValid,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryActionType,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryAction,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntry,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryExpandedActionIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntryGet FAILED: %d", prvTgfDevNum);

    /*set */

    /* AUTODOC: set first lookup client type to be PRV_TGF_EXACT_MATCH_CLIENT_TTI_E */
    rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum,lookupNum, firstLookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set profileId= 9  for TTI first lookup */
    rc = prvTgfExactMatchTtiProfileIdSet(prvTgfDevNum, packetType, lookupNum , enableExactMatchLookup , profileId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiExactMatchProfileIdSet FAILED: %d", prvTgfDevNum);


    /* AUTODOC: set Expanded Action for TTI lookup, profileId=9, ActionType=TTI, packet Command = TRAP */
    rc = prvTgfExactMatchExpandedActionSet(prvTgfDevNum,profileId,
                                            actionType,&actionData,&expandedActionOrigin);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchExpandedActionSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set Profile Key Params */
    rc =  prvTgfExactMatchProfileKeyParamsSet(prvTgfDevNum,profileId, &keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileKeyParamsSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set Exact Match Entry */
    rc =  prvTgfExactMatchPortGroupEntrySet(prvTgfDevNum,portGroupsBmp,index,
                                            expandedActionIndex,
                                            &exactMatchEntry,
                                            actionType,/* same a sconfigured in Extpanded Entry */
                                            &actionData);/* same a sconfigured in Extpanded Entry */
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntrySet FAILED: %d", prvTgfDevNum);

    return;
}

/**
* @internal prvTgfExactMatchReducedActionTtiExactMatchPriorityConfig function
* @endinternal
*
* @brief   Set TCAM Priority over Exact Match
* @param[in] exactMatchOverTtiEn - GT_FALSE: TCAM priority over Exact Match
*                                - GT_TRUE : Exact Match priority over TCAM
*/
GT_VOID prvTgfExactMatchReducedActionTtiExactMatchPriorityConfig
(
    GT_BOOL exactMatchOverTtiEn
)
{
    GT_STATUS                         rc;
    GT_U32                            ii;
    GT_U32                            ruleIndex;
    PRV_TGF_TTI_RULE_UNT              mask;
    PRV_TGF_TTI_RULE_UNT              pattern;
    PRV_TGF_TTI_ACTION_2_STC          action;
    PRV_TGF_TTI_RULE_TYPE_ENT         ruleType;

    /* TTI Configuration */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    ruleIndex                                    = PRV_TGF_TTI_INDEX_CNS;
    ruleType                                     = PRV_TGF_TTI_RULE_UDB_10_E;
    action.command                               = CPSS_PACKET_CMD_FORWARD_E;
    action.redirectCommand                       = PRV_TGF_TTI_VRF_ID_ASSIGN_E;
    action.tunnelTerminate                       = GT_FALSE;
    action.vrfId                                 = 0;
    action.exactMatchOverTtiEn                   = exactMatchOverTtiEn;


    for(ii = 0; ii < PRV_TGF_NUM_OF_UDB_CNS ; ii++)
    {
        pattern.udbArray[ii] = prvTgfPayloadDataArr[ii+3];
        mask.udbArray[ii] = 0xff;
    }
    rc = prvTgfTtiRuleUdbSet(ruleIndex,ruleType,&pattern, &mask, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleUdbSet FAILED: %d, %d, %d", prvTgfDevNum ,ruleIndex ,ruleType );
}

/**
* @internal prvTgfExactMatchReducedActionTtiReducedActionSet function
* @endinternal
*
* @brief Set to Reduced Action diffrent vaule from expanded Action
*        -set command PRV_TGF_PACKET_CMD_TRAP_TO_CPU_E
*        -set userDefinedCpuCode to 505
*/
GT_VOID prvTgfExactMatchReducedActionTtiReducedActionSet
(
    GT_VOID
)
{
    GT_STATUS   rc;

    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT             actionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT                  actionData;
    PRV_TGF_EXACT_MATCH_ACTION_UNT                  zeroAction;
    PRV_TGF_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT  expandedActionOrigin;
    GT_PORT_GROUPS_BMP                              portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U32                                          expandedActionIndex;
    PRV_TGF_EXACT_MATCH_ENTRY_STC                   exactMatchEntry;
    GT_U32                                          index=0;

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &actionData, 0, sizeof(actionData));
    cpssOsMemSet((GT_VOID*) &zeroAction, 0, sizeof(zeroAction));
    cpssOsMemSet((GT_VOID*) &expandedActionOrigin, 0, sizeof(expandedActionOrigin));

    expandedActionIndex         = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;
    actionType                  = PRV_TGF_EXACT_MATCH_ACTION_TTI_E ;

    zeroAction.ttiAction.command =     PRV_TGF_PACKET_CMD_FORWARD_E ;
    /*same as set on init*/
    actionData.ttiAction.command  = PRV_TGF_PACKET_CMD_TRAP_TO_CPU_E ;
    expandedActionOrigin.ttiExpandedActionOrigin.overwriteExpandedActionCommand = GT_TRUE ;

    /* set diffrent cpu code */
    actionData.ttiAction.userDefinedCpuCode = PRV_TGF_EXACT_MATCH_CPU_CODE_CNS + 2 ; /*505*/
    expandedActionOrigin.ttiExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode = GT_TRUE ;


    exactMatchEntry.key.keySize = PRV_TGF_EXACT_MATCH_KEY_SIZE_33B_E;
    exactMatchEntry.lookupNum   = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    /* here we check exactly the same pattern set for the TTI match in TCAM - all zero */
    cpssOsMemSet((GT_VOID*) &exactMatchEntry.key.pattern[0], 0, sizeof(exactMatchEntry.key.pattern));

    index = prvTgfExactMatchRestoreCfg.exactMatchEntryIndex ;

    rc = prvTgfExactMatchExpandedActionSet(prvTgfDevNum,expandedActionIndex,
                                            actionType,&zeroAction,&expandedActionOrigin);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntrySet FAILED: %d", prvTgfDevNum);
    rc =  prvTgfExactMatchPortGroupEntrySet(prvTgfDevNum, portGroupsBmp, index,
                                            expandedActionIndex,
                                            &exactMatchEntry,
                                            actionType, /* same a sconfigured in Extpanded Entry */
                                            &actionData); /* same a sconfigured in Extpanded Entry */
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntrySet FAILED: %d", prvTgfDevNum);

}

/**
* @internal prvTgfExactMatchReducedActionTtiTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] expectForwardTraffic - GT_FALSE: expect no traffic
*                                 - GT_TRUE: expect traffic to be forwarded
* @param[in] expectTrapTraffic    - GT_FALSE: expect no traffic
*                                 - GT_TRUE: expect traffic to be trapped
* @param[in] expectCPUcode        - if expectForwardTraffic set to GT_FALSE
*                                   Compar expectCPUcode to what we got
*/
GT_VOID prvTgfExactMatchReducedActionTtiTrafficGenerate
(
    GT_BOOL     expectForwardTraffic,
    GT_BOOL     expectTrapTraffic,
    GT_U32      expectCPUcode
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic prvTgfTunnelTermEtherTypeTrafficGenerateExpectTraffic =======\n");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */

    /* Reset the port Counter */
    rc = prvTgfCountersEthReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCountersEthReset");

    /* setup portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;

    PRV_UTF_LOG0_MAC("======= set capture on all ports =======\n");
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS];
    /* enable capture on port 1 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS]);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS];
    /* enable capture on port 2 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS]);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS];
    /* enable capture on port 3 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=00:00:44:33:22:11, SA=00:00:00:00:00:33, VID=1 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* ======= disable capture on all ports ======= */
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS];
    /* disable capture on port 3 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS]);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS];
    /* disable capture on port 2 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS]);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS];
    /* disable capture on port 1 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify to get no traffic - dropped */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Tx and Rx counters */
        switch (portIter) {
            case PRV_TGF_SEND_PORT_IDX_CNS:

                /* packetSize is not changed */
                expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;

                break;

        default:
             if (expectForwardTraffic == GT_TRUE)
             {
                /* port has both Rx and Tx counters because of capture */
                expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
             }
             else
             {
                expectedCntrs.goodOctetsSent.l[0] = 0;
                expectedCntrs.goodPktsSent.l[0]    = 0;
                expectedCntrs.ucPktsSent.l[0]      = 0;
                expectedCntrs.brdcPktsSent.l[0]    = 0;
                expectedCntrs.mcPktsSent.l[0]      = 0;
                expectedCntrs.goodOctetsRcv.l[0]   = 0;
                expectedCntrs.goodPktsRcv.l[0]     = 0;
                expectedCntrs.ucPktsRcv.l[0]       = 0;
                expectedCntrs.brdcPktsRcv.l[0]     = 0;
                expectedCntrs.mcPktsRcv.l[0]       = 0;
             }
        }

        isOk =
            portCntrs.goodOctetsSent.l[0] == expectedCntrs.goodOctetsSent.l[0] &&
            portCntrs.goodPktsSent.l[0]   == expectedCntrs.goodPktsSent.l[0] &&
            portCntrs.ucPktsSent.l[0]     == expectedCntrs.ucPktsSent.l[0] &&
            portCntrs.brdcPktsSent.l[0]   == expectedCntrs.brdcPktsSent.l[0] &&
            portCntrs.mcPktsSent.l[0]     == expectedCntrs.mcPktsSent.l[0] &&
            portCntrs.goodOctetsRcv.l[0]  == expectedCntrs.goodOctetsRcv.l[0] &&
            portCntrs.goodPktsRcv.l[0]    == expectedCntrs.goodPktsRcv.l[0] &&
            portCntrs.ucPktsRcv.l[0]      == expectedCntrs.ucPktsRcv.l[0] &&
            portCntrs.brdcPktsRcv.l[0]    == expectedCntrs.brdcPktsRcv.l[0] &&
            portCntrs.mcPktsRcv.l[0]      == expectedCntrs.mcPktsRcv.l[0];

        UTF_VERIFY_EQUAL0_STRING_MAC(isOk, GT_TRUE, "get another counters values.");

        /* print expected values if bug */
        if (isOk != GT_TRUE) {
            PRV_UTF_LOG0_MAC("Expected values:\n");
            PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");

            PRV_UTF_LOG0_MAC("Current values:\n");
            PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", portCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", portCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsSent = %d\n", portCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsSent = %d\n", portCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsSent = %d\n", portCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", portCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", portCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsRcv = %d\n", portCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsRcv = %d\n", portCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsRcv = %d\n", portCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }

    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    /* get trigger counters */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    /* ======= check on all ports ======= */
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS];

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    if (expectForwardTraffic==GT_TRUE)
    {
        /* check if there are captured packets at all */
        UTF_VERIFY_EQUAL0_STRING_MAC(1, numTriggers, "the test expected traffic to be forwarded\n");
    }
    else
    {
        /* check if no captured packets at all */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, numTriggers, "the test expected traffic NOT to be forwarded\n");

        if (GT_TRUE == prvTgfActionTrapToCpuCheck[PRV_TGF_CPU_PORT_IDX_CNS])
        {
            GT_U8                           packetBufPtr[64] = {0};
            GT_U32                          packetBufLen = 64;
            GT_U32                          packetLen;
            GT_U8                           devNum;
            GT_U8                           queueCpu;
            TGF_NET_DSA_STC                 rxParams;

            PRV_UTF_LOG0_MAC("CPU port capturing:\n");

            cpssOsBzero((GT_VOID*)&rxParams, sizeof(TGF_NET_DSA_STC));
            rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                               GT_TRUE, GT_TRUE,
                                               packetBufPtr,
                                               &packetBufLen,
                                               &packetLen,
                                               &devNum,
                                               &queueCpu,
                                               &rxParams);

            if(expectTrapTraffic==GT_FALSE)
            {
                /* we want to verify no packet was trapped to CPU
                   the command in the Exact Match Expanded Action was DROP */
                UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_TRUE,rxParams.dsaCmdIsToCpu);
                UTF_VERIFY_EQUAL0_PARAM_MAC(expectCPUcode,rxParams.cpuCode);

                PRV_UTF_LOG1_MAC("Source Port :%d\n",rxParams.portNum);
                PRV_UTF_LOG0_MAC("Packet is NOT Trapped to  CPU. Packet is dropped\n\n");
            }
            else
            {
                 /* we want to verify packet was trapped to CPU */
                 UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE,rxParams.dsaCmdIsToCpu);
                 UTF_VERIFY_EQUAL0_PARAM_MAC(expectCPUcode,rxParams.cpuCode);

                 PRV_UTF_LOG1_MAC("Source Port :%d\n",rxParams.portNum);
                 PRV_UTF_LOG1_MAC("Packet is Trapped to  CPU with CPU Code  :%d\n\n",rxParams.cpuCode);
            }
        }
    }

    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

}

/**
* @internal prvTgfExactMatchBasicExpandedActionGenericConfigRestore function
* @endinternal
*
* @brief   Restore TTI generic test configuration, not related to Exact Match
*/
GT_VOID prvTgfExactMatchReducedActionTtiGenericConfigRestore
(
    GT_VOID
)
{

    GT_STATUS   rc = GT_OK;
    GT_U32 ii;

    PRV_UTF_LOG0_MAC("======= Restore Configuration =======\n");

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

       /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_UDB_ETHERNET_OTHER_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d", prvTgfDevNum, GT_FALSE);

    /* AUTODOC: restore send port */
    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] = prvTgfRestoreCfg.sendPort;

    /* AUTODOC: restore UDBs */

    for (ii = 0; ii < PRV_TGF_NUM_OF_UDB_CNS; ii++) {
        rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                                         PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E, ii,
                                         PRV_TGFH_TTI_OFFSET_INVALID_E , 0);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    }

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);
}

/**
* @internal prvTgfExactMatchReducedActionTtiConfigRestore function
* @endinternal
*
* @brief   Restore TTI test configuration related to Exact Match
*          Reduced Action
*/
GT_VOID prvTgfExactMatchReducedActionTtiConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    PRV_TGF_TTI_KEY_TYPE_ENT                keyType;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT          ttiLookupNum;
    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT     actionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT          zeroAction;
    GT_PORT_GROUPS_BMP                      portGroupsBmp;
    PRV_TGF_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT  zeroActionOrigin;



    cpssOsMemSet(&zeroAction, 0, sizeof(zeroAction));
    cpssOsMemSet(&zeroActionOrigin, 0, sizeof(zeroActionOrigin));

    /* AUTODOC: Restore profileId configured for first lookup
       need to be restored profileId before restore of client type */
    portGroupsBmp   = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    keyType         = PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E;
    ttiLookupNum    = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    actionType      = PRV_TGF_EXACT_MATCH_ACTION_TTI_E;

    /* AUTODOC: restore Exact Match profileId */
    rc= prvTgfExactMatchTtiProfileIdSet(prvTgfDevNum,keyType,ttiLookupNum,
                                        prvTgfExactMatchRestoreCfg.profileIdEn,prvTgfExactMatchRestoreCfg.profileId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiExactMatchProfileIdSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: Restore first lookup client type */
    rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum,ttiLookupNum, prvTgfExactMatchRestoreCfg.firstLookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);


    if (prvTgfExactMatchRestoreCfg.exactMatchEntryValid == GT_TRUE)
    {
        /* AUTODOC: restore Exact Match Entry */
        rc =  prvTgfExactMatchPortGroupEntrySet(prvTgfDevNum,portGroupsBmp,
                                                prvTgfExactMatchRestoreCfg.exactMatchEntryIndex,
                                                prvTgfExactMatchRestoreCfg.exactMatchEntryExpandedActionIndex,
                                                &prvTgfExactMatchRestoreCfg.exactMatchEntry,
                                                prvTgfExactMatchRestoreCfg.exactMatchEntryActionType,/* same as configured in Extpanded Entry */
                                                &prvTgfExactMatchRestoreCfg.exactMatchEntryAction);/* same a sconfigured in Extpanded Entry */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntrySet FAILED: %d", prvTgfDevNum);
    }
    else
    {
        /* invalidate the entry */
        rc = prvTgfExactMatchPortGroupEntryInvalidate(prvTgfDevNum,portGroupsBmp,prvTgfExactMatchRestoreCfg.exactMatchEntryIndex);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);
    }

    /* AUTODOC: restore Expanded Action for TTI lookup */
    rc = prvTgfExactMatchExpandedActionSet(prvTgfDevNum, PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS,
                                           actionType, &zeroAction,
                                           &zeroActionOrigin);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchExpandedActionSet FAILED: %d", prvTgfDevNum);


    /* AUTODOC: restore Profile Key Params */
    rc =  prvTgfExactMatchProfileKeyParamsSet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS, &prvTgfExactMatchRestoreCfg.keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileKeyParamsSet FAILED: %d", prvTgfDevNum);

    return;
}


