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
* @file prvTgfExactMatchExpandedActionUdbPcl.c
*
* @brief Test Exact Match functionality with PCL Action Type
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
#include <exactMatch/prvTgfExactMatchExpandedActionUdbPcl.h>

#define PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS 5

/* parameters that is needed to be restored */
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

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS            1

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS         2

/* PCL rule index */
#define PRV_TGF_PCL_RULE_IDX_CNS             8

#define PRV_TGF_PCL_RULE_INDEX_CNS           0 /* shiri - see if can remove PRV_TGF_PCL_RULE_IDX_CNS*/

/* TTI rule index */
#define PRV_TGF_TTI_RULE_IDX_CNS             2

/* ARP Entry index */
#define PRV_TGF_ARP_ENTRY_IDX_CNS            3

/* TTI ID */
#define PRV_TGF_TTI_ID_CNS                   1

/* PCL ID */
#define PRV_TGF_PCL_ID_CNS                   0x55

/* original VLAN Id */
#define PRV_TGF_PASSENGER_TAG0_VLAN_ID_CNS   0x10

/* original VLAN Id */
#define PRV_TGF_PASSENGER_TAG1_VLAN_ID_CNS   0x11

/* original VLAN Id */
#define PRV_TGF_TUNNEL_TAG0_VLAN_ID_CNS      0x21

/* original VLAN Id */
#define PRV_TGF_TUNNEL_TAG1_VLAN_ID_CNS      0x22

/* lookup1 VLAN Id */
#define PRV_TGF_PCL_NEW_VLAN_ID_CNS          0x31

/* Action Flow Id  */
#define PRV_TGF_TTI_ACTION_FLOW_ID_CNS       0x25

/* Action Flow Id  */
#define PRV_TGF_PCL_ACTION_FLOW_ID_CNS       0x26

/* Router Source MAC_SA index  */
#define PRV_TGF_PCL_ROUTER_MAC_SA_INDEX_CNS  0x12

/* TCP Sorce port  */
#define PRV_TGF_PCL_TCP_SRC_PORT_CNS         0x7654

/* TCP Destination port  */
#define PRV_TGF_PCL_TCP_DST_PORT_CNS         0xFECB

/* UDP Sorce port  */
#define PRV_TGF_PCL_UDP_SRC_PORT_CNS         0x4567

/* UDP Destination port  */
#define PRV_TGF_PCL_UDP_DST_PORT_CNS         0xBCEF

/* IPV6 Sorce IP  */
#define PRV_TGF_PCL_IPV6_SRC_IP_CNS          {0x23, 0x45, 0x67, 0x89, 0x87, 0x65, 0x43, 0x21}

/* IPV6 Destination IP  */
#define PRV_TGF_PCL_IPV6_DST_IP_CNS          {0x01, 0x2E, 0x3D, 0x4C, 0x5B, 0x6A, 0x79, 0x88}


/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketPassengerL2Part =
{
    { 0x00, 0x12, 0x34, 0x56, 0x78, 0x9A },                /* daMac */
    { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 }                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketPassengerVlanTag0Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,              /* etherType */
    0, 0, PRV_TGF_PASSENGER_TAG0_VLAN_ID_CNS      /* pri, cfi, VlanId */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketPassengerVlanTag1Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,              /* etherType */
    0, 0, PRV_TGF_PASSENGER_TAG1_VLAN_ID_CNS      /* pri, cfi, VlanId */
};

/* Ethernet Type part */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEthOth1EthertypePart =
{
    0x3456
};

/* Payload of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPacketPayloadDataArr),            /* dataLength */
    prvTgfPacketPayloadDataArr                     /* dataPtr */
};

/* Double Tagged Ethernet Other Packet Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketNotTunneled2tagsEthOthPartsArray[] = {
    { TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part },  /* type, partPtr */
    { TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part },
    { TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part },
    { TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthOth1EthertypePart },
    { TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart }
};

/* Double Tagged Ethernet Other Packet Info */
static TGF_PACKET_STC prvTgfPacketNotTunneled2tagsEthOthInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + sizeof(prvTgfPacketPayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacketNotTunneled2tagsEthOthPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketNotTunneled2tagsEthOthPartsArray                                        /* partsArray */
};


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
* @internal prvTgfExactMatchBasicExpandedDefaultActionPclInvalidateRule function
* @endinternal
*
* @brief   Invalidate PCL Rule
*/
GT_VOID prvTgfExactMatchExpandedActionUdbPclInvalidateRule
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
* @internal GT_VOID prvTgfExactMatchExpandedActionUdbPclConfigSet function
* @endinternal
*
* @brief   Set PCL test configuration related to Exact Match Expanded Action
*/
GT_VOID prvTgfExactMatchExpandedActionUdbPclConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc;

    PRV_TGF_EXACT_MATCH_CLIENT_ENT                  firstLookupClientType;
    CPSS_PCL_DIRECTION_ENT                          direction;
    PRV_TGF_PCL_PACKET_TYPE_ENT                     pclPacketType;
    GT_U32                                          subProfileId;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT                  pclLookupNum;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT                  exactMatchLookupNum;

    GT_BOOL                                         enableExactMatchLookup;
    GT_U32                                          profileId;

    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT             actionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT                  actionData;

    PRV_TGF_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT  expandedActionOrigin;
    PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC      keyParams;
    GT_U32                                          udbAmount;

    GT_PORT_GROUPS_BMP                              portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U32                                          expandedActionIndex;
    PRV_TGF_EXACT_MATCH_ENTRY_STC                   exactMatchEntry;

    GT_U32                                          index=0;
    GT_U32                                          numberOfElemInCrcMultiHashArr;
    GT_U32                                          exactMatchCrcMultiHashArr[16];

    PRV_TGF_PCL_RULE_FORMAT_UNT                     mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT                     pattern;
    GT_U8                                           *udbMaskPtr;
    GT_U8                                           *udbPatternPtr;


    /* AUTODOC: keep clientType configured for first lookup , for restore */
    exactMatchLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    rc = prvTgfExactMatchClientLookupGet(prvTgfDevNum,exactMatchLookupNum,&prvTgfExactMatchRestoreCfg.firstLookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set first lookup client type to be PRV_TGF_EXACT_MATCH_CLIENT_IPCL_1_E */
    firstLookupClientType = PRV_TGF_EXACT_MATCH_CLIENT_IPCL_0_E;
    rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum,exactMatchLookupNum, firstLookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: keep profileId configured for first lookup , for restore */
    pclLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    direction = CPSS_PCL_DIRECTION_INGRESS_E;
    pclPacketType = PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E;
    subProfileId = 0;/* by default if not configured this field is 0 */
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
    cpssOsMemSet((GT_VOID*) &actionData, 0, sizeof(actionData));

    /* take all fields from Expanded Action: all fields overwriteExpande=GT_FALSE*/
    cpssOsMemSet((GT_VOID*) &expandedActionOrigin, 0, sizeof(expandedActionOrigin));

    actionData.pclAction.pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;
    /* For SIP6 and above for the egress control pipe the CPU code is also 'drop code' so relevant for drops too. */
    actionData.pclAction.mirror.cpuCode=CPSS_NET_FIRST_USER_DEFINED_E+1;
    actionData.pclAction.egressPolicy = GT_FALSE;
    actionType = PRV_TGF_EXACT_MATCH_ACTION_PCL_E;

    /* AUTODOC: set Expanded Action for PCL lookup, profileId=5, ActionType=PCL, packet Command = DROP */
    rc = prvTgfExactMatchExpandedActionSet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS,
                                            actionType,&actionData,&expandedActionOrigin);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchExpandedActionSet FAILED: %d", prvTgfDevNum);

    udbAmount = 33;
    keyParams.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
    keyParams.keyStart=0;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    udbMaskPtr    = mask.ruleIngrUdbOnly.udb;
    udbPatternPtr = pattern.ruleIngrUdbOnly.udb;

    prvTgfExactMatchPclUdbMaskPatternBuild(udbAmount,
                                           &prvTgfPacketNotTunneled2tagsEthOthInfo,/* packetPtr      */
                                           0,                                      /* offsetInPacket */
                                           udbMaskPtr,
                                           udbPatternPtr);

    cpssOsMemCpy((GT_VOID*) &keyParams.mask[0], udbMaskPtr, sizeof(keyParams.mask));

    /* AUTODOC: save Profile Key Params , for restore */
    rc =  prvTgfExactMatchProfileKeyParamsGet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS ,
                                           &prvTgfExactMatchRestoreCfg.keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchProfileKeyParamsGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set Profile Key Params */
    rc =  prvTgfExactMatchProfileKeyParamsSet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS, &keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchProfileKeyParamsSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: calculate index */

    exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;

    /* here we check exactly the same pattern set for the PCL match in TCAM */

    cpssOsMemCpy((GT_VOID*) &exactMatchEntry.key.pattern[0], udbPatternPtr, sizeof(exactMatchEntry.key.pattern));
    exactMatchEntry.lookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;

    rc =  prvTgfExactMatchHashCrcMultiResultsByParamsCalc(prvTgfDevNum,
                                                          &exactMatchEntry.key,
                                                          &numberOfElemInCrcMultiHashArr,
                                                          exactMatchCrcMultiHashArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchHashCrcMultiResultsByParamsCalc FAILED: %d", prvTgfDevNum);

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

    /* AUTODOC: save Exact Match Entry , for restore */
    rc =  prvTgfExactMatchPortGroupEntryGet(prvTgfDevNum,portGroupsBmp,index,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryValid,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryActionType,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryAction,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntry,
                                            &prvTgfExactMatchRestoreCfg.exactMatchEntryExpandedActionIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntryGet FAILED: %d", prvTgfDevNum);

    expandedActionIndex = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;
    actionType = PRV_TGF_EXACT_MATCH_ACTION_PCL_E;

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
* @internal GT_VOID prvTgfExactMatchExpandedActionUdbPclConfigRestore function
* @endinternal
*
* @brief   Restore PCL test configuration related to Exact Match Expanded Action
*/
GT_VOID prvTgfExactMatchExpandedActionUdbPclConfigRestore
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
    PRV_TGF_EXACT_MATCH_ACTION_UNT      zeroAction;
    GT_PORT_GROUPS_BMP                  portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    cpssOsMemSet(&zeroAction, 0, sizeof(zeroAction));

    /* AUTODOC: Restore profileId configured for first lookup
       need to be restored profileId before restore of client type */
    pclLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    direction = CPSS_PCL_DIRECTION_INGRESS_E;
    pclPacketType = PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E;
    subProfileId = 0;/* by default if not configured this field is 0 */
    rc = prvTgfExactMatchPclProfileIdSet(prvTgfDevNum, direction, pclPacketType, subProfileId, pclLookupNum,
                                         prvTgfExactMatchRestoreCfg.profileIdEn , prvTgfExactMatchRestoreCfg.profileId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPclProfileIdSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: Restore first lookup client type */
    exactMatchLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum,exactMatchLookupNum, prvTgfExactMatchRestoreCfg.firstLookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);

    if (prvTgfExactMatchRestoreCfg.exactMatchEntryValid==GT_TRUE)
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

    /* AUTODOC: restore Profile Key Params */
    rc =  prvTgfExactMatchProfileKeyParamsSet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS, &prvTgfExactMatchRestoreCfg.keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchProfileKeyParamsSet FAILED: %d", prvTgfDevNum);

    return;
}

/**
* @internal prvTgfExactMatchExpandedActionUdbPclInvalidateEmEntry function
* @endinternal
*
* @brief   Invalidate Exact Match Entry
*/
GT_VOID prvTgfExactMatchExpandedActionUdbPclInvalidateEmEntry
(
    GT_VOID
)
{
    GT_STATUS           rc;
    GT_PORT_GROUPS_BMP  portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* invalidate the entry */
    rc = prvTgfExactMatchPortGroupEntryInvalidate(prvTgfDevNum,portGroupsBmp,prvTgfExactMatchRestoreCfg.exactMatchEntryIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);
}
/**
* @internal prvTgfExactMatchExpandedActionUdbPclExactMatchPriorityConfig function
* @endinternal
*
* @brief   Set TCAM Priority over Exact Match
* @param[in] exactMatchOverPclEn - GT_FALSE: TCAM priority over Exact Match
*                                - GT_TRUE: Exact Match priority over TCAM
*/
GT_VOID prvTgfExactMatchExpandedActionUdbPclExactMatchPriorityConfig
(
    GT_BOOL exactMatchOverPclEn
)
{
    GT_STATUS                         rc;
    GT_U32                            ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_UNT       mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT       pattern;
    PRV_TGF_PCL_ACTION_STC            action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT  ruleFormat;

    GT_U8                            *udbMaskPtr;
    GT_U8                            *udbPatternPtr;
    GT_U32                            udbAmount;

    setRuleIndex(CPSS_PCL_DIRECTION_INGRESS_E,CPSS_PCL_LOOKUP_0_0_E);
    ruleIndex = currentRuleIndex;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    udbMaskPtr    = mask.ruleIngrUdbOnly.udb;
    udbPatternPtr = pattern.ruleIngrUdbOnly.udb;
    udbAmount     = 33;

    prvTgfExactMatchPclUdbMaskPatternBuild(udbAmount,
                                           &prvTgfPacketNotTunneled2tagsEthOthInfo,/* packetPtr      */
                                           0,                                      /* offsetInPacket */
                                           udbMaskPtr,
                                           udbPatternPtr);

    cpssOsMemSet(&action, 0, sizeof(action));

    action.pktCmd               = CPSS_PACKET_CMD_FORWARD_E;
    action.egressPolicy         = GT_FALSE;
    action.vlan.vlanCmd         = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    action.vlan.modifyVlan      = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    action.vlan.vlanId          = PRV_TGF_PCL_NEW_VLAN_ID_CNS;
    action.exactMatchOverPclEn  = exactMatchOverPclEn;

    ruleFormat                  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_40_E;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d", prvTgfDevNum, ruleFormat, ruleIndex);

}

