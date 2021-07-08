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
* @file prvTgfExactMatchDefaultActionUdbPcl.c
*
* @brief Test Exact Match Default Action functionality with PCL Action Type
*        and UDB key
*
* @version 1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <exactMatch/prvTgfExactMatchUdbPcl.h>
#include <exactMatch/prvTgfExactMatchDefaultActionUdbPcl.h>

#define PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS 5

/* parameters that is needed to be restored */
static struct
{
    GT_BOOL                             profileIdEn;
    GT_U32                              profileId;
    GT_BOOL                             defaultActionEn;
    PRV_TGF_EXACT_MATCH_ACTION_UNT      defaultAction;
    PRV_TGF_EXACT_MATCH_CLIENT_ENT      firstLookupClientType;
} prvTgfExactMatchRestoreCfg;


/* PCL rule index */
#define PRV_TGF_PCL_RULE_IDX_CNS             8

static GT_U32   currentRuleIndex = 0;

static void setRuleIndex(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum
)
{
    GT_U32  lookupId;

    if(direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        currentRuleIndex =  prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(PRV_TGF_PCL_RULE_IDX_CNS); ;
    }
    else
    {
        lookupId = lookupNum == CPSS_PCL_LOOKUP_NUMBER_0_E ? 0 :
                   lookupNum == CPSS_PCL_LOOKUP_NUMBER_1_E ? 1 :
                   lookupNum == CPSS_PCL_LOOKUP_NUMBER_2_E ? 2 :
                                                             0 ;/*CPSS_PCL_LOOKUP_0_E*/

        currentRuleIndex =  prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(lookupId,PRV_TGF_PCL_RULE_IDX_CNS);
    }
}

/**
* @internal prvTgfExactMatchDefaultActionUdbPclInvalidateRule function
* @endinternal
*
* @brief   Invalidate PCL Rule
*/
GT_VOID prvTgfExactMatchDefaultActionUdbPclInvalidateRule
(
    GT_VOID
)
{
    GT_STATUS   rc;
    CPSS_PCL_RULE_SIZE_ENT         ruleSize;
    GT_U32                         ruleIndex;
    GT_BOOL                        valid;

    setRuleIndex(CPSS_PCL_DIRECTION_INGRESS_E,CPSS_PCL_LOOKUP_0_0_E);

    ruleSize = CPSS_PCL_RULE_SIZE_40_BYTES_E;
    ruleIndex = currentRuleIndex;
    valid = GT_FALSE;

    /* invalidate the rule configured in setRuleIndex  */
    rc =  prvTgfPclRuleValidStatusSet(ruleSize,ruleIndex,valid);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet");
}
/**
* @internal GT_VOID prvTgfExactMatchDefaultActionUdbPclConfigSet function
* @endinternal
*
* @brief   Set PCL test configuration related to Exact Match Default Action
*/
GT_VOID prvTgfExactMatchDefaultActionUdbPclConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc;

    PRV_TGF_EXACT_MATCH_CLIENT_ENT      firstLookupClientType;
    CPSS_PCL_DIRECTION_ENT              direction;
    PRV_TGF_PCL_PACKET_TYPE_ENT         pclPacketType;
    GT_U32                              subProfileId;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT      pclLookupNum;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT      exactMatchLookupNum;

    GT_BOOL                             enableExactMatchLookup;
    GT_U32                              profileId;

    GT_BOOL                             enableDefaultAction;
    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT defaultActionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT      defaultActionData;

    /* AUTODOC: keep clientType configured for first lookup , for restore */
    exactMatchLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    rc = prvTgfExactMatchClientLookupGet(prvTgfDevNum,exactMatchLookupNum,&prvTgfExactMatchRestoreCfg.firstLookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set first lookup client type to be PRV_TGF_EXACT_MATCH_CLIENT_EPCL_E */
    firstLookupClientType = PRV_TGF_EXACT_MATCH_CLIENT_IPCL_0_E;
    rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum,exactMatchLookupNum, firstLookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: keep profileId configured for first lookup , for restore */
    pclLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    direction = CPSS_PCL_DIRECTION_INGRESS_E;
    pclPacketType = PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E;
    subProfileId = 0;/* by default if not configured this field is 0 */
    pclLookupNum=PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    rc = prvTgfExactMatchPclProfileIdGet(prvTgfDevNum, direction, pclPacketType, subProfileId, pclLookupNum,
                                         &prvTgfExactMatchRestoreCfg.profileIdEn , &prvTgfExactMatchRestoreCfg.profileId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPclProfileIdGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set profileId=5 for PCL first lookup */
    enableExactMatchLookup = GT_TRUE;
    profileId = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;
    rc = prvTgfExactMatchPclProfileIdSet(prvTgfDevNum, direction, pclPacketType, subProfileId,
                                         pclLookupNum , enableExactMatchLookup , profileId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPclProfileIdSet FAILED: %d", prvTgfDevNum);


    /* clear entry */
    cpssOsMemSet((GT_VOID*) &defaultActionData, 0, sizeof(defaultActionData));

    defaultActionData.pclAction.pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;
    /* For SIP6 and above for the egress control pipe the CPU code is also 'drop code' so relevant for drops too. */
    defaultActionData.pclAction.mirror.cpuCode=CPSS_NET_FIRST_USER_DEFINED_E+1;
    defaultActionData.pclAction.egressPolicy = GT_FALSE;
    defaultActionType = PRV_TGF_EXACT_MATCH_ACTION_PCL_E;
    enableDefaultAction = GT_TRUE ;

    /* AUTODOC: save Default Action , for restore */
    rc = prvTgfExactMatchProfileDefaultActionGet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS ,
                                                 defaultActionType,&prvTgfExactMatchRestoreCfg.defaultAction,
                                                 &prvTgfExactMatchRestoreCfg.defaultActionEn);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileDefaultActionGet FAILED: %d", prvTgfDevNum);



    /* AUTODOC: set Default Action for PCL lookup, profileId=5, ActionType=EPCL, enableDefault=GT_TRUE, packet Command = DROP */
     rc = prvTgfExactMatchProfileDefaultActionSet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS,
                                                  defaultActionType,&defaultActionData,enableDefaultAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileDefaultActionSet FAILED: %d", prvTgfDevNum);

    return;
}
/**
* @internal GT_VOID prvTgfExactMatchDefaultActionUdbPclConfigRestore function
* @endinternal
*
* @brief   Restore PCL test configuration related to Exact Match Default Action
*/
GT_VOID prvTgfExactMatchDefaultActionUdbPclConfigRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    CPSS_PCL_DIRECTION_ENT              direction;
    PRV_TGF_PCL_PACKET_TYPE_ENT         pclPacketType;
    GT_U32                              subProfileId;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT      pclLookupNum;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT      exactMatchLookupNum;
    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT defaultActionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT      zeroAction;

    cpssOsMemSet(&zeroAction, 0, sizeof(zeroAction));

    /* AUTODOC: Restore profileId configured for first lookup
       need to be restored profileId before restore of client type */
    pclLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    direction = CPSS_PCL_DIRECTION_INGRESS_E;
    pclPacketType = PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E;
    subProfileId = 0;/* by default if not configured this field is 0 */
    pclLookupNum=PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    rc = prvTgfExactMatchPclProfileIdSet(prvTgfDevNum, direction, pclPacketType, subProfileId, pclLookupNum,
                                         prvTgfExactMatchRestoreCfg.profileIdEn , prvTgfExactMatchRestoreCfg.profileId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPclProfileIdSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: Restore first lookup client type */
    exactMatchLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum,exactMatchLookupNum, prvTgfExactMatchRestoreCfg.firstLookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);

    defaultActionType = PRV_TGF_EXACT_MATCH_ACTION_PCL_E;

    /* AUTODOC: Restore Default Action */
    if (prvTgfExactMatchRestoreCfg.defaultActionEn == GT_TRUE )
    {
        /* AUTODOC: restore - enable Exact Match default Action */
        rc = prvTgfExactMatchProfileDefaultActionSet(prvTgfDevNum,
                                                     PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS,
                                                     defaultActionType,
                                                     &prvTgfExactMatchRestoreCfg.defaultAction,
                                                     GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileDefaultActionSet FAILED: %d", prvTgfDevNum);
    }
    else
    {
        /* AUTODOC: restore - disable Exact Match default Action */
        rc = prvTgfExactMatchProfileDefaultActionSet(prvTgfDevNum,
                                                     PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS,
                                                     defaultActionType,
                                                     &zeroAction,
                                                     GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileDefaultActionSet FAILED: %d", prvTgfDevNum);
    }
    return;
}

