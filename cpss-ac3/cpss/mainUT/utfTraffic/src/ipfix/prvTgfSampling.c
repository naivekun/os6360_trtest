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
* @file prvTgfSampling.c
*
* @brief Sampling to CPU tests for IPFIX
*
* @version   15
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
* @internal prvTgfIpfixSamplingAndPclTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
static GT_VOID prvTgfIpfixSamplingAndPclTestInit
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
    CPSS_NET_RX_CPU_CODE_ENT         cpuCode;
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

    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
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
    ipfixEntry.samplingAction = PRV_TGF_IPFIX_SAMPLING_ACTION_MIRROR_E;
    ipfixEntry.samplingMode = PRV_TGF_IPFIX_SAMPLING_MODE_PACKET_E;
    ipfixEntry.randomFlag = PRV_TGF_IPFIX_SAMPLING_DIST_DETERMINISTIC_E;
    ipfixEntry.samplingWindow.l[0] = 1;
    ipfixEntry.samplingWindow.l[1] = 0;
    ipfixEntry.cpuSubCode = 1;

    rc = prvTgfIpfixEntrySet(prvTgfDevNum, testedStage,
                             ipfixIndex, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    rc = prvTgfIpfixCpuCodeSet(prvTgfDevNum, testedStage,
                               CPSS_NET_FIRST_USER_DEFINED_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 CPSS_NET_FIRST_USER_DEFINED_E);

    rc = prvTgfIpfixCpuCodeGet(prvTgfDevNum, testedStage, &cpuCode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d",
                                 prvTgfDevNum, testedStage);

    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E,
                                 cpuCode,
                                 "CPU code retrieved value diffrent then set");

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");
}

/**
* @internal prvTgfIpfixSamplingTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
static GT_VOID prvTgfIpfixSamplingTestInit
(
    GT_VOID
)
{
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;
    GT_STATUS                        rc = GT_OK;
    GT_U32                           i;
    GT_U32                           portIter = 0;
    GT_U32                           ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    GT_U32                           ipfixIndex;
    PRV_TGF_IPFIX_ENTRY_STC          ipfixEntry;
    CPSS_NET_RX_CPU_CODE_ENT         cpuCode;
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

    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    prvTgfIpfixStagesParamsSaveAndReset();

    rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, testedStage,
                                      PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);

    for( i = 0 ; i < 5 ; i++ )
    {
        ruleIndex  = basicTestConf[i].ruleIndex;
        ipfixIndex = basicTestConf[i].ipfixIndex;

        cpssOsMemSet(&mask, 0, sizeof(mask));
        mask.ruleStdNotIp.etherType = 0xFFFF;

        cpssOsMemSet(&pattern, 0, sizeof(pattern));
        pattern.ruleStdNotIp.etherType = (GT_U16)(0x1000 + i);

        cpssOsMemSet(&action, 0, sizeof(action));
        action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
        action.policer.policerEnable = PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
        action.policer.policerId = ipfixIndex;

        rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                     ruleFormat, ruleIndex);

        cpssOsMemSet(&ipfixEntry, 0, sizeof(ipfixEntry));
        ipfixEntry.samplingAction = basicTestConf[i].samplingAction;
        ipfixEntry.samplingMode = basicTestConf[i].samplingMode;
        ipfixEntry.randomFlag = basicTestConf[i].randomFlag;
        ipfixEntry.samplingWindow.l[0] = basicTestConf[i].samplingWindow.l[0];
        ipfixEntry.samplingWindow.l[1] = basicTestConf[i].samplingWindow.l[1];
        ipfixEntry.cpuSubCode = basicTestConf[i].cpuSubCode;


        rc = prvTgfIpfixEntrySet(prvTgfDevNum, testedStage,
                                 ipfixIndex, &ipfixEntry);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                     prvTgfDevNum, testedStage, ipfixIndex);
    }

    wraparounfConf.action = PRV_TGF_IPFIX_WRAPAROUND_ACTION_NONE_E;
    wraparounfConf.dropThreshold = 0x3FFFFFFF;
    wraparounfConf.packetThreshold = 0x3FFFFFFF;
    wraparounfConf.byteThreshold.l[0] = 0xFFFFFFFF;
    wraparounfConf.byteThreshold.l[1] = 0xF;

    /* set wraparound configuration */
    rc = prvTgfIpfixWraparoundConfSet(prvTgfDevNum, testedStage, &wraparounfConf);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundConfSet: %d, %d",
                                 prvTgfDevNum, testedStage);

    rc = prvTgfIpfixCpuCodeSet(prvTgfDevNum, testedStage,
                               CPSS_NET_FIRST_USER_DEFINED_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 CPSS_NET_FIRST_USER_DEFINED_E);

    rc = prvTgfIpfixCpuCodeGet(prvTgfDevNum, testedStage, &cpuCode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d",
                                 prvTgfDevNum, testedStage);

    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E,
                                 cpuCode,
                                 "CPU code retrieved value diffrent then set");

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");
}

/**
* @internal prvTgfIpfixSamplingSimpleTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixSamplingSimpleTestInit
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

    prvTgfIpfixSamplingAndPclTestInit();
}

/* Timestamp Sampling window - 1 second */
#define PRV_TGF_TS_MODE_WINDOW_CNS 0x100

/* number of packets in test */
#define PRV_TGF_TS_BURST_SIZE_CNS 5

/**
* @internal prvTgfIpfixSamplingToCpuTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixSamplingToCpuTestInit
(
    GT_VOID
)
{
    GT_U32  i;

    cpssOsMemSet(basicTestConf, 0, sizeof(basicTestConf));

    for( i = 0 ; i < 5 ; i++ )
    {
        basicTestConf[i].ruleIndex  = i;
        basicTestConf[i].ipfixIndex = i;
        basicTestConf[i].burstCount = PRV_TGF_TS_BURST_SIZE_CNS;
        basicTestConf[i].payloadLen = 44;
        basicTestConf[i].samplingAction = PRV_TGF_IPFIX_SAMPLING_ACTION_MIRROR_E;
        basicTestConf[i].randomFlag = PRV_TGF_IPFIX_SAMPLING_DIST_DETERMINISTIC_E;
        basicTestConf[i].samplingWindow.l[1] = 0;
    }

    basicTestConf[0].samplingMode = PRV_TGF_IPFIX_SAMPLING_MODE_DISABLE_E;
    basicTestConf[0].samplingWindow.l[0] = 1;
    basicTestConf[0].cpuSubCode = 0;

    basicTestConf[1].samplingMode = PRV_TGF_IPFIX_SAMPLING_MODE_PACKET_E;
    basicTestConf[1].samplingWindow.l[0] = 1;
    basicTestConf[1].cpuSubCode = 0;

    basicTestConf[2].samplingMode = PRV_TGF_IPFIX_SAMPLING_MODE_PACKET_E;
    basicTestConf[2].samplingWindow.l[0] = 2;
    basicTestConf[2].cpuSubCode = 1;

    basicTestConf[3].samplingMode = PRV_TGF_IPFIX_SAMPLING_MODE_BYTE_E;
    basicTestConf[3].samplingWindow.l[0] = 84*3;
    basicTestConf[3].cpuSubCode = 2;

    basicTestConf[4].samplingMode = PRV_TGF_IPFIX_SAMPLING_MODE_TIME_E;
    basicTestConf[4].samplingWindow.l[0] = PRV_TGF_TS_MODE_WINDOW_CNS;
    basicTestConf[4].cpuSubCode = 3;

    testedStage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;

    prvTgfIpfixSamplingTestInit();
}

/**
* @internal prvTgfIpfixAlarmEventsTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixAlarmEventsTestInit
(
    GT_VOID
)
{
    GT_U32  i;

    cpssOsMemSet(basicTestConf, 0, sizeof(basicTestConf));

    for( i = 0 ; i < 5 ; i++ )
    {
        basicTestConf[i].ruleIndex  = i;
        basicTestConf[i].ipfixIndex = i;
        basicTestConf[i].burstCount = 5;
        basicTestConf[i].payloadLen = 44;
        basicTestConf[i].samplingAction = PRV_TGF_IPFIX_SAMPLING_ACTION_ALARM_E;
        basicTestConf[i].randomFlag = PRV_TGF_IPFIX_SAMPLING_DIST_DETERMINISTIC_E;
        basicTestConf[0].samplingWindow.l[1] = 0;
    }

    basicTestConf[0].samplingMode = PRV_TGF_IPFIX_SAMPLING_MODE_DISABLE_E;
    basicTestConf[0].samplingWindow.l[0] = 1;
    basicTestConf[0].cpuSubCode = 0;

    basicTestConf[1].samplingMode = PRV_TGF_IPFIX_SAMPLING_MODE_PACKET_E;
    basicTestConf[1].samplingWindow.l[0] = 1;
    basicTestConf[1].cpuSubCode = 0;

    basicTestConf[2].samplingMode = PRV_TGF_IPFIX_SAMPLING_MODE_PACKET_E;
    basicTestConf[2].samplingWindow.l[0] = 2;
    basicTestConf[2].cpuSubCode = 1;

    basicTestConf[3].samplingMode = PRV_TGF_IPFIX_SAMPLING_MODE_BYTE_E;
    basicTestConf[3].samplingWindow.l[0] = 84*3;
    basicTestConf[3].cpuSubCode = 2;

    basicTestConf[4].samplingMode = PRV_TGF_IPFIX_SAMPLING_MODE_TIME_E;
    basicTestConf[4].samplingWindow.l[0] = PRV_TGF_TS_MODE_WINDOW_CNS;
    basicTestConf[4].cpuSubCode = 3;

    testedStage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;

    prvTgfIpfixSamplingTestInit();
}

/**
* @internal prvTgfIpfixTimestampToCpuTestInit function
* @endinternal
*
* @brief   Set test configuration:
*/
GT_VOID prvTgfIpfixTimestampToCpuTestInit
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
    ruleIndex  = 0;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    mask.ruleStdNotIp.etherType = 0xFFFF;

    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    pattern.ruleStdNotIp.etherType = 0x1234;

    cpssOsMemSet(&action, 0, sizeof(action));

    action.pktCmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
    action.mirror.cpuCode = CPSS_NET_CPU_TO_CPU_E;
    action.policer.policerEnable = PRV_TGF_PCL_POLICER_DISABLE_ALL_E;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                  ruleFormat, ruleIndex);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");
}

/**
* @internal prvTgfIpfixSamplingSimpleTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixSamplingSimpleTestTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    PRV_TGF_IPFIX_ENTRY_STC ipfixEntry;
    GT_U32      burstCount;
    GT_U8                trigPacketBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32               buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32               packetActualLength = 0;
    GT_U8                devNum;
    GT_U8                queue;
    TGF_NET_DSA_STC      rxParam;
    GT_U32      expectedCnt = 0;

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

    cpssOsTimerWkAfter(100);

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
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[0].ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex);

    if(testedStage == PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        expectedCnt = burstCount + 1;/* to cpu packets */
        /* check IPFIX counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(expectedCnt, ipfixEntry.packetCount,
                                     "packetCount different then expected");
    }
    else
    {
        /* check IPFIX counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                     "packetCount different then expected");
    }

    /* Get first entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                  GT_TRUE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &devNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E + 1,
                                 rxParam.cpuCode,
                                 "samples packet cpu code different then expected");

    /* Get next entry from captured packet's table - no entry expected*/
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                  GT_FALSE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &devNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, rc, "prvTgfIpfixEntryGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}


static GT_VOID prvTgfLogIpfixEntries(PRV_TGF_IPFIX_ENTRY_STC *ipfixEntryArr, GT_U32 arrSize)
{
    GT_U32 ii;
    for (ii = 0; ii < arrSize; ii++)
    {
        PRV_UTF_LOG1_MAC("\n++++++++++++++IPFIX entry %u\n",ii);
        PRV_UTF_LOG1_MAC("IPFIX entry: packetCount           %u\n",ipfixEntryArr[ii].packetCount);
        PRV_UTF_LOG1_MAC("IPFIX entry: dropCounter           %u\n",ipfixEntryArr[ii].dropCounter);
        PRV_UTF_LOG1_MAC("IPFIX entry: timeStamp           0x%X\n",ipfixEntryArr[ii].timeStamp);
        PRV_UTF_LOG1_MAC("IPFIX entry: lastSampledValue[0] 0x%X\n",ipfixEntryArr[ii].lastSampledValue.l[0]);
        PRV_UTF_LOG1_MAC("IPFIX entry: lastSampledValue[1] 0x%X\n",ipfixEntryArr[ii].lastSampledValue.l[1]);
        PRV_UTF_LOG1_MAC("IPFIX entry: samplingWindow[0]   0x%X\n",ipfixEntryArr[ii].samplingWindow.l[0]);
        PRV_UTF_LOG1_MAC("IPFIX entry: samplingWindow[1]   0x%X\n",ipfixEntryArr[ii].samplingWindow.l[1]);
    }
}

/**
* @internal prvTgfIpfixSamplingToCpuTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixSamplingToCpuTestTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i;
    GT_U32      portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    PRV_TGF_IPFIX_ENTRY_STC ipfixEntry[PRV_TGF_TS_BURST_SIZE_CNS];
    GT_U32               burstCount;
    GT_U8                trigPacketBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32               buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32               packetActualLength = 0;
    GT_U8                devNum;
    GT_U8                queue;
    TGF_NET_DSA_STC      rxParam;

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

    /* Stage 1: no sampling */

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* 64 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[0].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1000 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x00;

    burstCount = basicTestConf[0].burstCount;

    /* send packets */
    for( i = 0 ; i < burstCount ; i++ )
    {
        prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, 1);

        cpssOsTimerWkAfter(100);
    }

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
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[0].ipfixIndex, GT_FALSE, &ipfixEntry[0]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry[0].packetCount,
                                 "packetCount different then expected");

    /* Get entry from captured packet's table - no entry expected*/
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                  GT_FALSE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &devNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* Stage 2: sampling based on packets */

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* 64 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[1].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1001 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x01;

    burstCount = basicTestConf[1].burstCount;

    /* send packets */
    for( i = 0 ; i < burstCount ; i++ )
    {
        prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, 1);

        cpssOsTimerWkAfter(100);
    }

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
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[1].ipfixIndex, GT_FALSE, &ipfixEntry[0]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[1].ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry[0].packetCount,
                                 "packetCount different then expected");

    /* Get first entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                  GT_TRUE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &devNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E + 0,
                                 rxParam.cpuCode,
                                 "sampled packet cpu code different then expected");

    /* get next 3 sampled packets */
    for (i = 1 ; i <= 3 ; i++ )
    {
        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                           GT_FALSE, GT_TRUE, trigPacketBuff,
                                           &buffLen, &packetActualLength,
                                           &devNum, &queue,
                                           &rxParam);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                     TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

        UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E + 0,
                                     rxParam.cpuCode,
                                     "sampled packet cpu code different then expected");
    }

    /* Get next entry from captured packet's table - no entry expected*/
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, trigPacketBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue,
                                       &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);


    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* Stage 3: sampling based on packets */

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* 64 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[2].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1002 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x02;

    burstCount = basicTestConf[2].burstCount;

    /* send packets */
    for( i = 0 ; i < burstCount ; i++ )
    {
        prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, 1);

        cpssOsTimerWkAfter(100);
    }

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
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[2].ipfixIndex, GT_FALSE, &ipfixEntry[0]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[2].ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry[0].packetCount,
                                 "packetCount different then expected");

    /* Get first entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                  GT_TRUE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &devNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E + 1,
                                 rxParam.cpuCode,
                                 "sampled packet cpu code different then expected");

    /* Get second entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, trigPacketBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue,
                                       &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                     TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E + 1,
                                     rxParam.cpuCode,
                                     "sampled packet cpu code different then expected");

    /* Get next entry from captured packet's table - no entry expected*/
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, trigPacketBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue,
                                       &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* Stage 4: sampling based on bytes */

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* 64 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[3].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1003 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x03;

    burstCount = basicTestConf[3].burstCount;

    /* send packets */
    for( i = 0 ; i < burstCount ; i++ )
    {
        prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, 1);

        cpssOsTimerWkAfter(100);
    }

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
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[3].ipfixIndex, GT_FALSE, &ipfixEntry[0]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[3].ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry[0].packetCount,
                                 "packetCount different then expected");

    /* Get first entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                  GT_TRUE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &devNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E + 2,
                                 rxParam.cpuCode,
                                 "sampled packet cpu code different then expected");

    /* Get next entry from captured packet's table - no entry expected*/
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, trigPacketBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue,
                                       &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* Stage 5: sampling based on timestamp */

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* 64 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[4].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1004 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x04;

    burstCount = basicTestConf[4].burstCount;

    /* send packets */
    for( i = 0 ; i < burstCount ; i++ )
    {
        prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, 1);

        /* save entry for failure analysis */
        rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                                 basicTestConf[4].ipfixIndex, GT_FALSE, &ipfixEntry[i]);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                     prvTgfDevNum, testedStage, basicTestConf[4].ipfixIndex);

        if (i == 0)
        {
            if ((ipfixEntry[i].timeStamp < PRV_TGF_TS_MODE_WINDOW_CNS) && (ipfixEntry[i].lastSampledValue.l[0] == 0))
            {
                PRV_UTF_LOG0_MAC("Timestamp is low for test, go sleep 600 millisecond\n");
                cpssOsTimerWkAfter(600);
            }
        }

        cpssOsTimerWkAfter(100);
    }

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
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[4].ipfixIndex, GT_FALSE, &ipfixEntry[burstCount-1]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[4].ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry[burstCount-1].packetCount,
                                 "packetCount different then expected");

    /* The test may result in two samples in case that current time for first packet between 0x100 and 0x200.
       e.g. current time of first packet is 0x177. Packet is sampled and <Last Sampled Value> became 0x100
       (incremented by <Sampling Window>). Last packet#5 is came in current time 0x204 and sampled again.
    */

    /* Get entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_TRUE, GT_TRUE, trigPacketBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue,
                                       &rxParam);

    if (rc != GT_OK)
    {
        prvTgfLogIpfixEntries(ipfixEntry,PRV_TGF_TS_BURST_SIZE_CNS);
    }

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E + 3,
                                 rxParam.cpuCode,
                                 "sampled packet cpu code different then expected");

    /* Get next entry from captured packet's table - no entry expected */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, trigPacketBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue,
                                       &rxParam);

    if (rc == GT_OK)
    {
        if (ipfixEntry[0].lastSampledValue.l[0] == PRV_TGF_TS_MODE_WINDOW_CNS)
        {
            /* it's valid case where two samples are */
        }
        else
        {
            prvTgfLogIpfixEntries(ipfixEntry,PRV_TGF_TS_BURST_SIZE_CNS);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                         TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);
        }
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfIpfixAlarmEventsTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixAlarmEventsTestTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i;
    GT_U32      portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    PRV_TGF_IPFIX_ENTRY_STC ipfixEntry;
    GT_U32               burstCount;
    GT_U8                trigPacketBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32               buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32               packetActualLength = 0;
    GT_U8                devNum;
    GT_U8                queue;
    TGF_NET_DSA_STC      rxParam;
    GT_U32               eventsArr[PRV_TGF_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS];
    GT_U32               eventsNum;
    GT_U32               countArr[2];
    GT_BOOL              resetUsePortGroupsBmp = GT_FALSE;

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

    /* Stage 1: no sampling */

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* 64 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[0].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1000 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x00;

    burstCount = basicTestConf[0].burstCount;

    /* Clear events */
    rc = prvTgfIpfixAlarmEventsGet(prvTgfDevNum, testedStage, eventsArr, &eventsNum);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixAlarmEventsGet: %d, %d",
                                 prvTgfDevNum, testedStage);

    /* send packets */
    for( i = 0 ; i < burstCount ; i++ )
    {
        prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, 1);

        cpssOsTimerWkAfter(100);
    }

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
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[0].ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* Get alarm events -  no event expecterd */
    rc = prvTgfIpfixAlarmEventsGet(prvTgfDevNum, testedStage, eventsArr, &eventsNum);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixAlarmEventsGet: %d, %d",
                                 prvTgfDevNum, testedStage);

    UTF_VERIFY_EQUAL0_STRING_MAC(0, eventsNum,
                                 "more alarm events then expexted");

    /* Get entry from captured packet's table - no entry expected*/
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                  GT_FALSE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &devNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* Stage 2: sampling based on packets */

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* 64 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[1].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1001 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x01;

    burstCount = basicTestConf[1].burstCount;

    /* send packets */
    for( i = 0 ; i < burstCount ; i++ )
    {
        prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, 1);

        cpssOsTimerWkAfter(100);
    }

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
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[1].ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[1].ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* Get alarm events -  4 event expecterd */
    rc = prvTgfIpfixAlarmEventsGet(prvTgfDevNum, testedStage, eventsArr, &eventsNum);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixAlarmEventsGet: %d, %d",
                                 prvTgfDevNum, testedStage);

    UTF_VERIFY_EQUAL0_STRING_MAC(4, eventsNum,
                                 "different number of alarm events then expexted");

    for( i = 0 ; i < 4 ; i++ )
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(basicTestConf[1].ipfixIndex, eventsArr[i],
                                     "different IPFIX index then expexted");
    }

    /* Get entry from captured packet's table - no entry expected*/
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                  GT_FALSE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &devNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);


    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* Stage 3: sampling based on packets */

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* 64 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[2].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1002 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x02;

    burstCount = basicTestConf[2].burstCount;

    /* send packets */
    for( i = 0 ; i < burstCount ; i++ )
    {
        prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, 1);

        cpssOsTimerWkAfter(100);
    }

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
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[2].ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[2].ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* Stage 4: sampling based on bytes */

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* 64 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[3].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1003 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x03;

    burstCount = basicTestConf[3].burstCount;

    /* send packets */
    for( i = 0 ; i < burstCount ; i++ )
    {
        prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, 1);

        cpssOsTimerWkAfter(100);
    }

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
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[3].ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[3].ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* Get alarm events -  3 event expecterd */
    rc = prvTgfIpfixAlarmEventsGet(prvTgfDevNum, testedStage, eventsArr, &eventsNum);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixAlarmEventsGet: %d, %d",
                                 prvTgfDevNum, testedStage);

    UTF_VERIFY_EQUAL0_STRING_MAC(3, eventsNum,
                                 "different number of alarm events then expexted");
    countArr[0] = countArr[1] = 0;
    for( i = 0 ; i < 3 ; i++ )
    {
        if( eventsArr[i] == basicTestConf[2].ipfixIndex )
        {
            countArr[0]++;
        }
        else if ( eventsArr[i] == basicTestConf[3].ipfixIndex )
        {
            countArr[1]++;
        }
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(2, countArr[0],
                                 "different IPFIX index then expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(1, countArr[1],
                                 "different IPFIX index then expexted");

    /* Get entry from captured packet's table - no entry expected*/
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                  GT_FALSE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &devNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* Stage 5: sampling based on timestamp */

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* 64 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[4].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1004 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x04;

    burstCount = basicTestConf[4].burstCount;

    /* send packets */
    for( i = 0 ; i < burstCount ; i++ )
    {
        prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, 1);

        if (usePortGroupsBmp == GT_FALSE)
        {
            /* use ingress port processing pipe to get true values */
            TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
            resetUsePortGroupsBmp = GT_TRUE;
        }

        /* save entry for failure analysis */
        rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                                 basicTestConf[4].ipfixIndex, GT_FALSE, &ipfixEntry);

        if (resetUsePortGroupsBmp)
        {
            /* restore PortGroupsBmp mode */
            usePortGroupsBmp  = GT_FALSE;
            currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        }

        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                     prvTgfDevNum, testedStage, basicTestConf[4].ipfixIndex);

        if (i == 0)
        {
            if ((ipfixEntry.timeStamp < PRV_TGF_TS_MODE_WINDOW_CNS) && (ipfixEntry.lastSampledValue.l[0] == 0))
            {
                PRV_UTF_LOG0_MAC("Timestamp is low for test, go sleep 600 millisecond\n");
                cpssOsTimerWkAfter(600);
            }
        }

        cpssOsTimerWkAfter(100);
    }

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
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    /* Get alarm events -  1 event expected */
    rc = prvTgfIpfixAlarmEventsGet(prvTgfDevNum, testedStage, eventsArr, &eventsNum);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixAlarmEventsGet: %d, %d",
                                 prvTgfDevNum, testedStage);

    UTF_VERIFY_EQUAL0_STRING_MAC(1, eventsNum,
                                 "different number of alarm events then expexted");

    UTF_VERIFY_EQUAL0_STRING_MAC(basicTestConf[4].ipfixIndex, eventsArr[0],
                                 "different IPFIX index then expexted");

    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[4].ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[3].ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* Get entry from captured packet's table - no entry expected*/
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                  GT_FALSE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &devNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfIpfixTimestampToCpuTestTrafficGenerate function
* @endinternal
*
* @brief   1. Disable time stamp to CPU and check that time stamp is not changing.
*         2. Enable time stamp to CPU and check that time stamp is changing
*         according to wait time.
*/
GT_VOID prvTgfIpfixTimestampToCpuTestTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U8                trigPacketBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32               buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32               packetActualLength = 0;
    GT_U8                devNum;
    GT_U8                queue;
    TGF_NET_DSA_STC      rxParam;
    GT_BOOL              enable;
    GT_U32               timestampSeconds, timestampSeconds1;

    cpssOsBzero((char *)&rxParam, sizeof(rxParam));
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

    /* 64 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = 44;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1234 */
    prvTgfPayloadDataArr[0] = 0x12;
    prvTgfPayloadDataArr[1] = 0x34;

    rc = prvTgfIpfixTimestampToCpuEnableSet(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixDropCountModeSet: %d, %d",
                                 prvTgfDevNum, GT_FALSE);

    rc = prvTgfIpfixTimestampToCpuEnableGet(prvTgfDevNum, &enable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpfixDropCountModeSet: %d",
                                 prvTgfDevNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_FALSE, enable, "timestamp to CPU is expected to be disabled");

    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, 1);

    /* 2 seconds wait. The time stamp to CPU is disabled.
      The 2 seconds is sufficient time for understanding
      that time stamp is changing or not. */
    cpssOsTimerWkAfter(2000);

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
            UTF_VERIFY_EQUAL0_STRING_MAC(1, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(1, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    /* Get entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                  GT_TRUE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &devNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    timestampSeconds = rxParam.originByteCount;

    /* Get next entry from captured packet's table - no entry expected*/
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, trigPacketBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue,
                                       &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, 1);

    cpssOsTimerWkAfter(100);

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
            UTF_VERIFY_EQUAL0_STRING_MAC(1, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(1, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    /* Get entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                  GT_TRUE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &devNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                  GT_FALSE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &devNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC(timestampSeconds,
                                 rxParam.originByteCount,
                                 "timestamp field value is different then expected");

    /* Get next entry from captured packet's table - no entry expected*/
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, trigPacketBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue,
                                       &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    rc = prvTgfIpfixTimestampToCpuEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixDropCountModeSet: %d, %d",
                                 prvTgfDevNum, GT_FALSE);

    rc = prvTgfIpfixTimestampToCpuEnableGet(prvTgfDevNum, &enable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpfixDropCountModeSet: %d",
                                 prvTgfDevNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, enable, "timestamp to CPU is expected to be enabled");

    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, 1);

    /* 5 seconds wait */
    cpssOsTimerWkAfter(5000);

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
            UTF_VERIFY_EQUAL0_STRING_MAC(1, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(1, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    /* Get entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                  GT_TRUE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &devNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                  GT_FALSE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &devNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_FALSE);

    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                  GT_FALSE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &devNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_FALSE);

    timestampSeconds = (rxParam.originByteCount >> 7);

    /* Get next entry from captured packet's table - no entry expected*/
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, trigPacketBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue,
                                       &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, 1);

    cpssOsTimerWkAfter(100);

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
            UTF_VERIFY_EQUAL0_STRING_MAC(1, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(1, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    /* Get entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                  GT_TRUE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &devNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                  GT_FALSE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &devNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_FALSE);

    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                  GT_FALSE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &devNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_FALSE);

    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                  GT_FALSE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &devNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_FALSE);

    timestampSeconds1 = (rxParam.originByteCount >> 7);

    if( (((timestampSeconds + 5) & 0x7F) != timestampSeconds1) &&
        (((timestampSeconds + 6) & 0x7F) != timestampSeconds1) )
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(0, 1,
                                     "timestamps value difference is different the expected: %d %d",
                                     timestampSeconds, timestampSeconds1);
    }

    /* Get next entry from captured packet's table - no entry expected*/
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, trigPacketBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue,
                                       &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_REGULAR_E, GT_TRUE);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfIpfixSamplingTestRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfIpfixSamplingTestRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    prvTgfIpfixIngressTestRestore();
}

/**
* @internal prvTgfIpfixTimestampToCpuTestRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfIpfixTimestampToCpuTestRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    prvTgfIpfixVlanRestore(PRV_TGF_VLANID_CNS, localPortsVlanMembers);

    /* restore IPFIX and PCL config */
    prvTgfIpfixIngressTestRestore();
}

/**
* @internal prvTgfIpfixEgressSamplingTestInit function
* @endinternal
*
* @brief    Set test configuration:
*           Enable the Egress Policy Engine set Pcl rule.
*/
static GT_VOID prvTgfIpfixEgressSamplingTestInit
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    GT_U32                           portIter = 0;
    GT_U32                           ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    GT_U32                           ipfixIndex;
    PRV_TGF_IPFIX_ENTRY_STC          ipfixEntry;
    CPSS_NET_RX_CPU_CODE_ENT         cpuCode;
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

    /* PCL Configuration */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* init PCL Engine for Egress PCL */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[2],
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    ipfixIndex = basicTestConf[0].ipfixIndex;

    ruleIndex                                    = 0;
    ruleFormat                                   = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    action.pktCmd                                = CPSS_PACKET_CMD_FORWARD_E;
    action.policer.policerEnable                 = PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
    action.egressPolicy                          = GT_TRUE;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d", prvTgfDevNum, ruleFormat, ruleIndex);

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
    ipfixEntry.samplingAction = PRV_TGF_IPFIX_SAMPLING_ACTION_MIRROR_E;
    ipfixEntry.samplingMode = PRV_TGF_IPFIX_SAMPLING_MODE_PACKET_E;
    ipfixEntry.randomFlag = PRV_TGF_IPFIX_SAMPLING_DIST_DETERMINISTIC_E;
    ipfixEntry.samplingWindow.l[0] = 1;
    ipfixEntry.samplingWindow.l[1] = 0;
    ipfixEntry.cpuSubCode = 1;

    rc = prvTgfIpfixEntrySet(prvTgfDevNum, testedStage,
                             ipfixIndex, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    rc = prvTgfIpfixCpuCodeSet(prvTgfDevNum, testedStage,
                               CPSS_NET_FIRST_USER_DEFINED_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 CPSS_NET_FIRST_USER_DEFINED_E);

    rc = prvTgfIpfixCpuCodeGet(prvTgfDevNum, testedStage, &cpuCode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d",
                                 prvTgfDevNum, testedStage);

    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E,
                                 cpuCode,
                                 "CPU code retrieved value diffrent then set");
    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");
}

/**
* @internal prvTgfIpfixEgressSamplingToCpuTestInit function
* @endinternal
*
* @brief    Set test configuration:
*           Enable the Egress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixEgressSamplingToCpuTestInit
(
    GT_VOID
)
{

    cpssOsMemSet(basicTestConf, 0, sizeof(basicTestConf));

    basicTestConf[0].ruleIndex  = 0;
    basicTestConf[0].ipfixIndex = 0;
    basicTestConf[0].burstCount = 2;
    basicTestConf[0].payloadLen = 44;

    testedStage = PRV_TGF_POLICER_STAGE_EGRESS_E;

    prvTgfIpfixEgressSamplingTestInit();
}

/**
* @internal prvTgfIpfixEgressTestRestore function
* @endinternal
*
* @brief    Restore configuration:
*/

GT_VOID prvTgfIpfixEgressTestRestore
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       ruleIndex;
    GT_U32       i;
    CPSS_PCL_RULE_SIZE_ENT  ruleSize;

    prvTgfPayloadDataArr[0] = 0;
    prvTgfPayloadDataArr[1] = 0;

    if(GT_FALSE ==  prvTgfPolicerStageCheck(testedStage))
    {
        return ;
    }

    ruleSize = CPSS_PCL_RULE_SIZE_STD_E;

    prvTgfIpfixStagesParamsRestore();

    for( i = 0 ; i < 5 ; i++ )
    {
        ruleIndex = basicTestConf[i].ruleIndex;

        rc = prvTgfPclRuleValidStatusSet(ruleSize, ruleIndex, GT_FALSE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                     ruleSize, ruleIndex, GT_FALSE);
    }

    /* Disables PCL ingress Policy */
    rc = prvTgfPclEgressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    rc = prvTgfPolicerMemorySizeModeSet(prvTgfDevNum,
                                        PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_PLR1_LOWER_E,
                                        0,0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeModeSet: %d, %d",
                                 prvTgfDevNum,
                                 PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_PLR1_LOWER_E);

    prvTgfIpfixVlanRestore(PRV_TGF_VLANID_CNS, localPortsVlanMembers);

}

/**
* @internal prvTgfIpfixEgressTestRestore function
* @endinternal
*
* @brief    Restore configuration:
*/
GT_VOID prvTgfIpfixEgressSamplingTestRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    prvTgfIpfixEgressTestRestore();
}
