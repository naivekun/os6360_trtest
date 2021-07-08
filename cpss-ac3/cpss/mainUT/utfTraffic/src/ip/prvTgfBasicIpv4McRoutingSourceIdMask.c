/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfBasicIpv4McRouting.c
*
* DESCRIPTION:
*       Basic IPV4 MC Routing: source id and mask testing
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfL2MllGen.h>
#include <common/tgfIpGen.h>
#include <ip/prvTgfBasicIpv4McRouting.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/


/* PCL rule & action index */
#define EPCL_MATCH_INDEX_CNS      prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(108)

/* router sourceId value */
#define PRV_TGF_ROUTER_SOURCE_ID_CNS        (0xFF >> 5) /* 0b111 */

/* router sourceId mask */
#define PRV_TGF_ROUTER_SOURCE_ID_MASK_CNS   (PRV_TGF_ROUTER_SOURCE_ID_CNS >> 1) /* 0b011 */

/* config restore variables */
static GT_U32 sourceIdMaskOrig;
static GT_U32 sourceIdOrig;


/**
* @internal prvTgfEPclSourceIsDrop function
* @endinternal
*
* @brief   build Pcl rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_VOID prvTgfEPclSourceIsDrop
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    portIter    = 0;

    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat     = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;

    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
    PRV_TGF_PCL_ACTION_STC      action;

    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));


    PRV_UTF_LOG0_MAC("======= Setting Pcl Configuration =======\n");


    for (portIter = 0; portIter < 4; portIter++)
    {
        /* AUTODOC: Init EPCL Engine */
        rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[portIter],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            ruleFormat  /*nonIpKey*/,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
        UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_OK, rc, "prvTgfPclDefPortInit: %d, %d, %d",
                prvTgfDevNum,
                prvTgfPortsArray[portIter],
                CPSS_PCL_DIRECTION_INGRESS_E);


        /* AUTODOC: EPCL Pattern for Source ID */
        pattern.ruleEgrStdNotIp.common.sourceId = PRV_TGF_ROUTER_SOURCE_ID_MASK_CNS;

        /* AUTODOC: EPCL Mask for Source ID */
        mask.ruleEgrStdNotIp.common.sourceId = 0x1F /*ePcl supports up to 5 bits*/;


        /* AUTODOC: EPCL action - drop the packet */
        action.pktCmd  = CPSS_PACKET_CMD_DROP_HARD_E;
        action.egressPolicy = GT_TRUE;

        rc = prvTgfPclRuleSet(ruleFormat, EPCL_MATCH_INDEX_CNS, &mask, &pattern, &action);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                            ruleFormat, EPCL_MATCH_INDEX_CNS);
    }

}


/* restore configuration function */
static GT_VOID prvTgfBasicIpv4McRoutingSourceIdConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: restore source id mask and value */
    prvTgfIpRouterSourceIdSet(CPSS_IP_MULTICAST_E, sourceIdMaskOrig, sourceIdOrig);

    /* AUTODOC: invalidate EPCL rule */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, EPCL_MATCH_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E, EPCL_MATCH_INDEX_CNS, GT_FALSE);

    /* AUTODOC: Clear EPCL Engine configuration tables */
    prvTgfPclPortsRestore(CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E);

    /* AUTODOC: Disables PCL egress Policy */
    rc = prvTgfPclEgressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPolicyEnable: %d", GT_FALSE);

    prvTgfBasicIpv4McRoutingConfigurationRestore();
}


/**
* @internal prvTgfBasicIpv4McRoutingSourceId function
* @endinternal
*
* @brief   Main test function of tgfBasicIpv4McRoutingSourceId
*/
GT_VOID prvTgfBasicIpv4McRoutingSourceId
(
    GT_VOID
)
{
    prvTgfBasicIpv4McRoutingBaseConfigurationSet();
    prvTgfBasicIpv4McRoutingRouteConfigurationSet();

    /* AUTODOC: save source id mask and value for config restore */
    prvTgfIpRouterSourceIdGet(CPSS_IP_MULTICAST_E, &sourceIdMaskOrig, &sourceIdOrig);

    /* AUTODOC: set the ePcl Rule for the packets matching srcId   */
    /* AUTODOC: ePcl rule action: drop packets with src id value */
    prvTgfEPclSourceIsDrop();

    /* --------------------------------------------------------------- */
    /* AUTODOC: check traffic pass with default source id value and mask   */
    prvTgfBasicIpv4McRoutingTrafficGenerate(GT_TRUE);

    /* --------------------------------------------------------------- */

    /* AUTODOC: set router source id mask (less than value) and source id value   */
    prvTgfIpRouterSourceIdSet(CPSS_IP_MULTICAST_E,
                              PRV_TGF_ROUTER_SOURCE_ID_MASK_CNS,   /* source id mask (less than value) */
                              PRV_TGF_ROUTER_SOURCE_ID_CNS);       /* source id value */
    /* AUTODOC: send packet and expect ePcl match, and no traffic (dropped by ePcl) */
    prvTgfBasicIpv4McRoutingTrafficGenerate(GT_FALSE);

    /* --------------------------------------------------------------- */
    /* AUTODOC: set router source id mask (less than previous mask) and same source id value   */
    prvTgfIpRouterSourceIdSet(CPSS_IP_MULTICAST_E,
                              PRV_TGF_ROUTER_SOURCE_ID_MASK_CNS >> 1, /* source id mask (less than orig mask) */
                              PRV_TGF_ROUTER_SOURCE_ID_CNS);          /* source id value */
    /* AUTODOC: send packet and expect no ePcl match, and traffic (no drop) */
    prvTgfBasicIpv4McRoutingTrafficGenerate(GT_TRUE);

    /* --------------------------------------------------------------- */
    /* AUTODOC: configur restore */
    prvTgfBasicIpv4McRoutingSourceIdConfigurationRestore();

}



