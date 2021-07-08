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
* @file prvTgfDropMode.c
*
* @brief Drop Count Mode test for IPFIX
*
* @version   7
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPolicerGen.h>
#include <common/tgfIpfixGen.h>

#include <ipfix/prvTgfBasicIngressPclKeys.h>
#include <ipfix/prvTgfBasicTests.h>

/**
* @internal prvTgfIpfixDropModeTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
static GT_VOID prvTgfIpfixDropModeTestInit
(
    GT_VOID
)
{
    PRV_TGF_PCL_LOOKUP_CFG_STC lookupCfg;
    GT_STATUS               rc = GT_OK;
    GT_U32                  portIter = 0;
    GT_U32                           ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    GT_U32                           ipfixIndex;
    PRV_TGF_IPFIX_ENTRY_STC          ipfixEntry;
    PRV_TGF_IPFIX_WRAPAROUND_CFG_STC wraparounfConf;
    
    /* clear entry */
    cpssOsMemSet(&localPortsVlanMembers, 0, sizeof(localPortsVlanMembers));

    /* ports 0, 8, 18, 23 are VLAN Members */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        localPortsVlanMembers.ports[prvTgfPortsArray[portIter] >> 5] |= 1 << (prvTgfPortsArray[portIter] & 0x1f);
    }

    /* set VLAN entry */
     prvTgfIpfixVlanTestInit(PRV_TGF_VLANID_CNS, localPortsVlanMembers); 

     /* Init PCL. */
    rc = prvTgfPclInit();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"prvTgfPclInit");

    /* Enable PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_TRUE);

    /* enables ingress policy for port 8 */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], 
                                 GT_TRUE);

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                CPSS_PCL_DIRECTION_INGRESS_E,
                                                CPSS_PCL_LOOKUP_0_E,
                                                0,
                                                PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet: %d, %d, %d, %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 CPSS_PCL_DIRECTION_INGRESS_E, 
                                 CPSS_PCL_LOOKUP_0_E,
                                 0,
                                 PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);

    /* Set PCL configuration table */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    interfaceInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    lookupCfg.enableLookup        = GT_TRUE;

    lookupCfg.enableLookup = GT_TRUE;
    lookupCfg.pclId = 0;

    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    rc = prvTgfPclCfgTblSet(&interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
                            CPSS_PCL_LOOKUP_0_E, &lookupCfg);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

    /* set PCL rule */
    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    ruleIndex  = basicTestConf[0].ruleIndex;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    mask.ruleStdNotIp.etherType = 0xFFFF;
    
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    pattern.ruleStdNotIp.etherType = 0x1000;
    
    cpssOsMemSet(&action, 0, sizeof(action));

    ipfixIndex = basicTestConf[0].ipfixIndex;

    action.pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;
    action.policer.policerEnable = PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
    action.policer.policerId = ipfixIndex;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                  ruleFormat, ruleIndex);

    prvTgfIpfixStagesParamsSaveAndReset();
    
    wraparounfConf.action = PRV_TGF_IPFIX_WRAPAROUND_ACTION_NONE_E;
    wraparounfConf.dropThreshold = 0x3FFFFFFF;
    wraparounfConf.packetThreshold = 0x3FFFFFFF;
    wraparounfConf.byteThreshold.l[0] = 0xFFFFFFFF;
    wraparounfConf.byteThreshold.l[1] = 0xF;

    /* set wraparound configuration */
    rc = prvTgfIpfixWraparoundConfSet(prvTgfDevNum, testedStage, &wraparounfConf);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundConfSet: %d, %d",
                                 prvTgfDevNum, testedStage);

    rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, testedStage,
                                      PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);

    cpssOsMemSet(&ipfixEntry, 0, sizeof(ipfixEntry));
    rc = prvTgfIpfixEntrySet(prvTgfDevNum, testedStage,
                             ipfixIndex, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);
}

/**
* @internal prvTgfIpfixDropModeSimpleTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixDropModeSimpleTestInit
(
    GT_VOID
)
{
    cpssOsMemSet(basicTestConf, 0, sizeof(basicTestConf));

    basicTestConf[0].ruleIndex  = 0;
    basicTestConf[0].ipfixIndex = 0;
    basicTestConf[0].burstCount = 2;
    basicTestConf[0].payloadLen = 44;

    testedStage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;

    prvTgfIpfixDropModeTestInit();
}

/**
* @internal prvTgfIpfixDropModeSimpleTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixDropModeSimpleTestTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    PRV_TGF_IPFIX_DROP_COUNT_MODE_ENT   mode;
    PRV_TGF_IPFIX_ENTRY_STC             ipfixEntry;
    GT_U32      burstCount;
    

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* set drop mode all */
    rc = prvTgfIpfixDropCountModeSet(prvTgfDevNum, testedStage, 
                                     PRV_TGF_IPFIX_DROP_COUNT_MODE_ALL_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixDropCountModeSet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, 
                                 PRV_TGF_IPFIX_DROP_COUNT_MODE_ALL_E);

    rc = prvTgfIpfixDropCountModeGet(prvTgfDevNum, testedStage, &mode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixDropCountModeGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage);

    UTF_VERIFY_EQUAL0_STRING_MAC(mode, PRV_TGF_IPFIX_DROP_COUNT_MODE_ALL_E,
                                 "Metering only drop mode is expected");

    /* 64 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[0].payloadLen;
    prvTgfPacketInfo.totalLen = 
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;
    
    /* Set ethertype to value 0x1000 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x00;

    burstCount = basicTestConf[0].burstCount;

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, burstCount);

    cpssOsTimerWkAfter(400);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

     
        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[0].ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.dropCounter,
                                 "dropCount different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* set drop mode meter only */
    rc = prvTgfIpfixDropCountModeSet(prvTgfDevNum, testedStage, 
                                     PRV_TGF_IPFIX_DROP_COUNT_MODE_METER_ONLY_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixDropCountModeSet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, 
                                 PRV_TGF_IPFIX_DROP_COUNT_MODE_METER_ONLY_E);

    rc = prvTgfIpfixDropCountModeGet(prvTgfDevNum, testedStage, &mode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixDropCountModeGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage);

    UTF_VERIFY_EQUAL0_STRING_MAC(mode, PRV_TGF_IPFIX_DROP_COUNT_MODE_METER_ONLY_E,
                                 "Metering only drop mode is expected");

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, burstCount);

    cpssOsTimerWkAfter(400);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

     
        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[0].ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex);

    /* check IPFIX counters */
    if (!prvTgfIpfixIsResetOnReadErratum(prvTgfDevNum))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.dropCounter,
                                 "dropCount different then expected");
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.dropCounter,
                                 "dropCount different then expected");
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}


