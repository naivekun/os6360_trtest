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
* @file prvTgfBasicTests.c
*
* @brief Basic tests for IPFIX
*
* @version   13
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPolicerGen.h>
#include <common/tgfIpfixGen.h>

#include <ipfix/prvTgfBasicIngressPclKeys.h>

#define PRV_TGF_TEST_DEFINITIONS

#include <ipfix/prvTgfBasicTests.h>

/* port bitmap VLAN members */
CPSS_PORTS_BMP_STC localPortsVlanMembers = {{0, 0}};

PRV_TGF_TEST_CONFIG_STC basicTestConf[5];

PRV_TGF_POLICER_STAGE_TYPE_ENT testedStage;

PRV_TGF_POLICER_COUNTING_MODE_ENT stagesCountingModeSave[PRV_TGF_POLICER_STAGE_NUM];
GT_BOOL                           stagesMeterEnableSave[PRV_TGF_POLICER_STAGE_NUM];
static PRV_TGF_POLICER_STAGE_METER_MODE_ENT stagesModeSave[PRV_TGF_POLICER_STAGE_NUM];

/******************************* Test packet **********************************/

/* L2 part of packet */
TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x99},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}                /* srcMac */
};

/* First VLAN_TAG part */
TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};
/* use 933 instead of 1004 to avoid problems in xcat-A1 that total packet length
   was into the issue of :
         add 8 byte padding (another buffer is added, numOfAddedBuffs = 3) should
         be done only if packets length is <byteblock>*n+k (1<=k<=8) (including
         DSA tag), where n and k are integers and <byteblock> is 256 for xCat
         and 512 for DxChLion,
         otherwise (numOfAddedBuffs = 2) and previous flow is preserved.
*/
#define LONG_PAYLOAD_LEN_CNS     933

/* DATA of packet */
GT_U8 prvTgfPayloadDataArr[1004];

/* PAYLOAD part */
TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    0,                       /* dataLength */
    prvTgfPayloadDataArr     /* dataPtr */
};

/* PARTS of packet */
TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* PACKET to send */
TGF_PACKET_STC prvTgfPacketInfo = {
    0,                                                           /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/**
* @internal prvTgfIpfixStagesParamsSaveAndReset function
* @endinternal
*
* @brief   Save stages counting mode and enable metering state for later restore
*         and set stages to disable counting and metering.
*/
GT_VOID prvTgfIpfixStagesParamsSaveAndReset
(
    GT_VOID
)
{
    GT_U32      i;
    GT_STATUS   rc;

    for( i = 0 ; i < PRV_TGF_POLICER_STAGE_NUM ; i++ )
    {
        if(GT_FALSE ==  prvTgfPolicerStageCheck(i))
        {
            continue ;
        }

        rc = prvTgfPolicerCountingModeGet(prvTgfDevNum, i, &stagesCountingModeSave[i]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeGet: %d, %d",
                                     prvTgfDevNum, i);
        rc = prvTgfPolicerMeteringEnableGet(prvTgfDevNum, i, &stagesMeterEnableSave[i]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerMeteringEnableGet: %d, %d",
                                     prvTgfDevNum, i);
        rc = prvTgfPolicerStageMeterModeGet(prvTgfDevNum, i, &stagesModeSave[i]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeGet: %d, %d",
                                     prvTgfDevNum, i);
        rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, i, PRV_TGF_POLICER_COUNTING_DISABLE_E);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d, %d, %d",
                                     prvTgfDevNum, i, PRV_TGF_POLICER_COUNTING_DISABLE_E);
        rc = prvTgfPolicerMeteringEnableSet(i, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerMeteringEnableSet: %d, %d",
                                     i, GT_FALSE);
        rc = prvTgfPolicerStageMeterModeSet(i, PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d, %d",
                                     i, PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);
    }
}

/**
* @internal prvTgfIpfixStagesParamsRestore function
* @endinternal
*
* @brief   Restore stages counting mode and enable metering state.
*/
GT_VOID prvTgfIpfixStagesParamsRestore
(
    GT_VOID
)
{
    GT_U32      i;
    GT_STATUS   rc;

    for( i = 0 ; i < PRV_TGF_POLICER_STAGE_NUM ; i++ )
    {
        if(GT_FALSE ==  prvTgfPolicerStageCheck(i))
        {
            continue ;
        }

        rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, i, stagesCountingModeSave[i]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d, %d",
                                     i, stagesCountingModeSave[i]);
        rc = prvTgfPolicerMeteringEnableSet(i, stagesMeterEnableSave[i]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerMeteringEnableSet: %d, %d",
                                     i, stagesMeterEnableSave[i]);
        rc = prvTgfPolicerStageMeterModeSet(i, stagesModeSave[i]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d, %d",
                                     i, stagesModeSave[i]);
    }
}

/**
* @internal prvTgfIpfixIngressTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixIngressTestInit
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
        CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[portIter]);
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

    /* IPFIX works in Flow mode */
    rc = prvTgfPolicerStageMeterModeSet(testedStage, PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d",
                                 testedStage);

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

    ruleIndex  = basicTestConf[1].ruleIndex;
    pattern.ruleStdNotIp.etherType = 0x1001;
    ipfixIndex = basicTestConf[1].ipfixIndex;
    action.policer.policerId = ipfixIndex;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                  ruleFormat, ruleIndex);

    rc = prvTgfIpfixEntrySet(prvTgfDevNum, testedStage,
                             ipfixIndex, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    ruleIndex  = basicTestConf[2].ruleIndex;
    pattern.ruleStdNotIp.etherType = 0x1002;
    ipfixIndex = basicTestConf[2].ipfixIndex;
    action.policer.policerId = ipfixIndex;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                  ruleFormat, ruleIndex);

    rc = prvTgfIpfixEntrySet(prvTgfDevNum, testedStage,
                             ipfixIndex, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);
}

/**
* @internal prvTgfIpfixFirstIngressTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixFirstIngressTestInit
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32 maxCountersNum;
    GT_U32  numPhyPorts = MAX(256,PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.maxPhysicalPorts);

    rc = prvUtfCfgTableNumEntriesGet(prvTgfDevNum,
                                     PRV_TGF_CFG_TABLE_POLICER_COUNTERS_E,
                                     &maxCountersNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvUtfCfgTableNumEntriesGet: %d, %d",
                                 prvTgfDevNum,
                                 PRV_TGF_CFG_TABLE_POLICER_COUNTERS_E);

    cpssOsMemSet(basicTestConf, 0, sizeof(basicTestConf));

    basicTestConf[0].ruleIndex  = 0;
    basicTestConf[0].ipfixIndex = 0;
    basicTestConf[0].burstCount = 1;
    basicTestConf[0].payloadLen = 44;

    basicTestConf[1].ruleIndex  = 1;
    basicTestConf[1].ipfixIndex = maxCountersNum/2 - 1;
    basicTestConf[1].burstCount = 2;
    basicTestConf[1].payloadLen = 480;

    basicTestConf[2].ruleIndex  = 2;
    basicTestConf[2].ipfixIndex = (maxCountersNum > numPhyPorts) ? (maxCountersNum - 1 - numPhyPorts) : maxCountersNum/4 - 1;
    basicTestConf[2].burstCount = 3;
    basicTestConf[2].payloadLen = LONG_PAYLOAD_LEN_CNS;

    testedStage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;

    prvTgfIpfixIngressTestInit();
}

/**
* @internal prvTgfIpfixFirstUseAllIngressTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixFirstUseAllIngressTestInit
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32 maxCountersNum;

    rc = prvUtfCfgTableNumEntriesGet(prvTgfDevNum,
                                     PRV_TGF_CFG_TABLE_POLICER_COUNTERS_E,
                                     &maxCountersNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvUtfCfgTableNumEntriesGet: %d, %d",
                                 prvTgfDevNum,
                                 PRV_TGF_CFG_TABLE_POLICER_COUNTERS_E);

    cpssOsMemSet(basicTestConf, 0, sizeof(basicTestConf));

    basicTestConf[0].ruleIndex  = 0;
    basicTestConf[0].ipfixIndex = 0;
    basicTestConf[0].burstCount = 1;
    basicTestConf[0].payloadLen = 44;

    basicTestConf[1].ruleIndex  = 1;
    basicTestConf[1].ipfixIndex = maxCountersNum/2;
    basicTestConf[1].burstCount = 2;
    basicTestConf[1].payloadLen = 480;

    basicTestConf[2].ruleIndex  = 2;
    basicTestConf[2].ipfixIndex = maxCountersNum - 1;
    basicTestConf[2].burstCount = 3;
    basicTestConf[2].payloadLen = LONG_PAYLOAD_LEN_CNS;

    testedStage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;

    rc = prvTgfPolicerMemorySizeModeSet(prvTgfDevNum,
                                        PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_AND_LOWER_E,
                                        0,0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeModeSet: %d, %d",
                                 prvTgfDevNum,
                                 PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_AND_LOWER_E);

    prvTgfIpfixIngressTestInit();
}

/**
* @internal prvTgfIpfixSecondUseAllIngressTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixSecondUseAllIngressTestInit
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32 maxCountersNum;

    rc = prvUtfCfgTableNumEntriesGet(prvTgfDevNum,
                                     PRV_TGF_CFG_TABLE_POLICER_COUNTERS_E,
                                     &maxCountersNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvUtfCfgTableNumEntriesGet: %d, %d",
                                 prvTgfDevNum,
                                 PRV_TGF_CFG_TABLE_POLICER_COUNTERS_E);

    cpssOsMemSet(basicTestConf, 0, sizeof(basicTestConf));

    basicTestConf[0].ruleIndex  = 0;
    basicTestConf[0].ipfixIndex = 0;
    basicTestConf[0].burstCount = 1;
    basicTestConf[0].payloadLen = 44;

    basicTestConf[1].ruleIndex  = 1;
    basicTestConf[1].ipfixIndex = maxCountersNum/2;
    basicTestConf[1].burstCount = 2;
    basicTestConf[1].payloadLen = 480;

    basicTestConf[2].ruleIndex  = 2;
    basicTestConf[2].ipfixIndex = maxCountersNum - 1;
    basicTestConf[2].burstCount = 1;
    basicTestConf[2].payloadLen = LONG_PAYLOAD_LEN_CNS;

    testedStage = PRV_TGF_POLICER_STAGE_INGRESS_1_E;

    if(GT_FALSE ==  prvTgfPolicerStageCheck(testedStage))
    {
        return ;
    }

    rc = prvTgfPolicerMemorySizeModeSet(prvTgfDevNum,
                                        PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_AND_LOWER_E,
                                        0,0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeModeSet: %d, %d",
                                 prvTgfDevNum,
                                 PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_AND_LOWER_E);

    prvTgfIpfixIngressTestInit();

    /* switch IPLR0 to be in Port mode in order to avoid influence of IPLR0 on the test.
       this is MUST be done for SIP5 devices with hierarchical policing. */
    rc = prvTgfPolicerStageMeterModeSet(PRV_TGF_POLICER_STAGE_INGRESS_0_E, PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: INGRESS_0, MODE_PORT");
}

/**
* @internal prvTgfIpfixTimestampVerificationTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixTimestampVerificationTestInit
(
    GT_VOID
)
{
    cpssOsMemSet(basicTestConf, 0, sizeof(basicTestConf));

    basicTestConf[0].ruleIndex  = 0;
    basicTestConf[0].ipfixIndex = 0;
    basicTestConf[0].burstCount = 1;
    basicTestConf[0].payloadLen = 44;

    basicTestConf[1].ruleIndex  = 1;
    basicTestConf[1].ipfixIndex = 1;
    basicTestConf[1].burstCount = 1;
    basicTestConf[1].payloadLen = 44;

    basicTestConf[2].ruleIndex  = 2;
    basicTestConf[2].ipfixIndex = 2;
    basicTestConf[2].burstCount = 1;
    basicTestConf[2].payloadLen = 44;

    testedStage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;

    prvTgfIpfixIngressTestInit();
}

/**
* @internal prvTgfIpfixIngressTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixIngressTestTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32                  ipfixIndex;
    PRV_TGF_IPFIX_ENTRY_STC ipfixEntry;
    GT_U32                  burstCount;

    if(GT_FALSE ==  prvTgfPolicerStageCheck(testedStage))
    {
        return ;
    }

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

    cpssOsTimerWkAfter(200);

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

    ipfixIndex = basicTestConf[0].ipfixIndex;
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* check IPFIX counters again with reset flag enabled */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* check IPFIX counters again and verify all counters are zero */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    if (!prvTgfIpfixIsResetOnReadErratum(prvTgfDevNum))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.packetCount,
                                     "packetCount different then expected");
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                     ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                     "packetCount different then expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* 500 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[1].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1001 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x01;

    burstCount = basicTestConf[1].burstCount;

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, burstCount);

    cpssOsTimerWkAfter(200);

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

    ipfixIndex = basicTestConf[1].ipfixIndex;
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* check IPFIX counters again with reset flag enabled */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* check IPFIX counters again and verify all counters are zero */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);
    if (!prvTgfIpfixIsResetOnReadErratum(prvTgfDevNum))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.packetCount,
                                     "packetCount different then expected");
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                     ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                     "packetCount different then expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* 1024 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[2].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1002 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x02;

    burstCount = basicTestConf[2].burstCount;

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, burstCount);

    cpssOsTimerWkAfter(200);

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

    ipfixIndex = basicTestConf[2].ipfixIndex;
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* check IPFIX counters again with reset flag enabled */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* check IPFIX counters again and verify all counters are zero */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    if (!prvTgfIpfixIsResetOnReadErratum(prvTgfDevNum))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.packetCount,
                                     "packetCount different then expected");
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                     ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                     "packetCount different then expected");
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
* @internal prvTgfIpfixTimestampVerificationTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixTimestampVerificationTestTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    /*CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;*/
    GT_U32                  ipfixIndex;
    PRV_TGF_IPFIX_ENTRY_STC ipfixEntry;
    GT_U32                  burstCount;
    GT_U32                  secondsFromTimerToCompare;
    GT_U32                  secondsFromEntryToCompare;
    PRV_TGF_IPFIX_TIMER_STC timer;

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

    /* get IPFIX timer */
    rc = prvTgfIpfixTimerGet(prvTgfDevNum, testedStage, &timer);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d",
                                 prvTgfDevNum, testedStage);

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, burstCount);

    cpssOsTimerWkAfter(5000);

    ipfixIndex = basicTestConf[0].ipfixIndex;

    /* check IPFIX timestamp */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    secondsFromEntryToCompare = ((ipfixEntry.timeStamp & 0xFF00 ) >> 8);

    secondsFromTimerToCompare = (timer.secondTimer.l[0] & 0xFF);

    if( (secondsFromEntryToCompare != secondsFromTimerToCompare) &&
        (secondsFromEntryToCompare != (secondsFromTimerToCompare + 1)) &&
        ((secondsFromEntryToCompare + 1) != secondsFromTimerToCompare) )
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(secondsFromEntryToCompare,
                                     secondsFromTimerToCompare,
                                     "ipfix timestamp too different then timer");
    }

    /* check IPFIX timestamp after read with reset flag enabled */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    secondsFromEntryToCompare = ((ipfixEntry.timeStamp & 0xFF00 ) >> 8);

    secondsFromTimerToCompare = ((timer.secondTimer.l[0] + 5) & 0xFF);

    if( (secondsFromEntryToCompare != secondsFromTimerToCompare) &&
        (secondsFromEntryToCompare != (secondsFromTimerToCompare + 1)) &&
        ((secondsFromEntryToCompare + 1) != secondsFromTimerToCompare) )
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(secondsFromEntryToCompare,
                                     secondsFromTimerToCompare,
                                     "ipfix timestamp after reset too different then timer");
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
* @internal prvTgfIpfixIngressTestRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfIpfixIngressTestRestore
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

    /* Disables ingress policy for port 8 */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 GT_FALSE);

    /* Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
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
* @internal prvTgfIpfixTrafficGenManager function
* @endinternal
*
* @brief   Perform the test traffic generation and checks for all port groups
*         (unaware mode) and again for first core.
*/
GT_VOID prvTgfIpfixTrafficGenManager
(
    GT_VOID (*initFuncPtr)(GT_VOID),
    GT_VOID (*trafficGenFuncPtr)(GT_VOID),
    GT_VOID (*restoreFuncPtr)(GT_VOID)
)
{
    initFuncPtr();
    trafficGenFuncPtr();
    restoreFuncPtr();

    /* set <currPortGroupsBmp> , <usePortGroupsBmp> */
    TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    initFuncPtr();
    trafficGenFuncPtr();
    restoreFuncPtr();

    /* restore PortGroupsBmp mode */
    usePortGroupsBmp  = GT_FALSE;
    currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
}


