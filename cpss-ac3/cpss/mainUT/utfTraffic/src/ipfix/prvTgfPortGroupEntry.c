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
* @file prvTgfPortGroupEntry.c
*
* @brief IPFIX Entry per port group test for IPFIX
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

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

static GT_U32 prvTgfIPfixPortGroupSavePorts[PRV_TGF_MAX_PORTS_NUM_CNS];
static GT_U8 prvTgfIPfixPortGroupSavePortsNum;

/**
* @internal prvTgfIpfixPortGroupEntryTestInit function
* @endinternal
*
* @brief   IPFIX per port group entry manipulation test configuration set.
*/
GT_VOID prvTgfIpfixPortGroupEntryTestInit
(
    GT_VOID
)
{
    PRV_TGF_PCL_LOOKUP_CFG_STC lookupCfg;
    GT_STATUS               rc = GT_OK;
    GT_U32                           ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    GT_U32                           ipfixIndex;
    PRV_TGF_IPFIX_ENTRY_STC          ipfixEntry;
    PRV_TGF_IPFIX_WRAPAROUND_CFG_STC wraparounfConf;
    GT_U8                            portIter;

    prvTgfIPfixPortGroupSavePortsNum = prvTgfPortsNum;
    for( portIter = 0 ; portIter < prvTgfPortsNum ; portIter ++ )
    {
        prvTgfIPfixPortGroupSavePorts[portIter] = prvTgfPortsArray[portIter];
    }

    cpssOsMemSet(basicTestConf, 0, sizeof(basicTestConf));

    basicTestConf[0].ruleIndex  = 0;
    basicTestConf[0].ipfixIndex = 0;
    basicTestConf[0].burstCount = 1;
    basicTestConf[0].payloadLen = 44;

    testedStage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;

    prvTgfPortsNum = 4;
    if (prvTgfDevicePortMode == PRV_TGF_DEVICE_PORT_MODE_XLG_E)
    {
        prvTgfPortsArray[0] = 4;
        prvTgfPortsArray[1] = 16;
        prvTgfPortsArray[2] = 88;
        prvTgfPortsArray[3] = 105;
    }
    else
    {
        prvTgfPortsArray[0] = 0;
        prvTgfPortsArray[1] = 18;
        prvTgfPortsArray[2] = 36;
        prvTgfPortsArray[3] = 58;
    }

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* ports 0, 18, 36, 58 are VLAN Members */
    localPortsVlanMembers.ports[0] = BIT_0 | BIT_18;
    localPortsVlanMembers.ports[1] = BIT_4 | BIT_26;

    /* set VLAN entry */
    prvTgfIpfixVlanTestInit(PRV_TGF_VLANID_CNS, localPortsVlanMembers);

     /* Init PCL. */
    rc = prvTgfPclInit();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"prvTgfPclInit");

    /* Enable PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_TRUE);

    /* Set PCL configuration table */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;

    lookupCfg.enableLookup        = GT_TRUE;
    lookupCfg.enableLookup = GT_TRUE;
    lookupCfg.pclId = 0;

    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    /* enables ingress policy for ports 0, 18, 36, 58 */
    for( portIter = 0 ; portIter < prvTgfPortsNum ; portIter++ )
    {
        rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[portIter], GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                     portIter,
                                     GT_TRUE);

        rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[portIter],
                                                    CPSS_PCL_DIRECTION_INGRESS_E,
                                                    CPSS_PCL_LOOKUP_0_E,
                                                    0,
                                                    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet: %d, %d, %d, %d, %d",
                                     prvTgfPortsArray[portIter],
                                     CPSS_PCL_DIRECTION_INGRESS_E,
                                     CPSS_PCL_LOOKUP_0_E,
                                     0,
                                     PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);

        interfaceInfo.devPort.portNum = prvTgfPortsArray[portIter];

        rc = prvTgfPclCfgTblSet(&interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
                                CPSS_PCL_LOOKUP_0_E, &lookupCfg);

        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet: %d %d %d",
                                     prvTgfPortsArray[portIter],
                                     CPSS_PCL_DIRECTION_INGRESS_E,
                                     CPSS_PCL_LOOKUP_0_E);
    }

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
    rc = prvTgfIpfixEntrySet(prvTgfDevNum, testedStage,
                             ipfixIndex, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);
}

/**
* @internal prvTgfIpfixPortGroupEntryTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixPortGroupEntryTestTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter, portIterJ;
    PRV_TGF_IPFIX_ENTRY_STC ipfixEntry;
    GT_U32      sendPacketsPerPort[4] = {1,2,4,8};
    GT_U32      sendPacketsSum = 0;

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);

    /* 64 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[0].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1000 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x00;

    for( portIter = 0 ; portIter < prvTgfPortsNum ; portIter++ )
    {
        /* send packet */
        prvTgfIpfixTestPacketSend(prvTgfPortsArray[portIter], &prvTgfPacketInfo, sendPacketsPerPort[portIter]);
        sendPacketsSum += sendPacketsPerPort[portIter];

        cpssOsTimerWkAfter(400);

        for( portIterJ = 0 ; portIterJ < prvTgfPortsNum ; portIterJ++ )
        {
            /* set <currPortGroupsBmp> , <usePortGroupsBmp> */
            TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(prvTgfPortsArray[portIterJ]);

            rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                                     basicTestConf[0].ipfixIndex, GT_FALSE, &ipfixEntry);
            UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d, %d, %d",
                                         prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex,
                                         portIter, portIterJ);

            if( portIterJ <= portIter )
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(sendPacketsPerPort[portIterJ], ipfixEntry.packetCount,
                                             "packetCount different then expected: %d %d",
                                             portIter, portIterJ);
            }
            else
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(0, ipfixEntry.packetCount,
                                             "packetCount different then expected: %d %d",
                                             portIter, portIterJ);
            }
        }

        /* restore PortGroupsBmp mode */
        usePortGroupsBmp  = GT_FALSE;
        currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex, GT_FALSE, &ipfixEntry);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d, %d, %d",
                                     prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex,
                                     portIter, portIterJ);

        UTF_VERIFY_EQUAL2_STRING_MAC(sendPacketsSum, ipfixEntry.packetCount,
                                     "packetCount different then expected: %d %d",
                                     portIter, portIterJ);
    }

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");
}

/**
* @internal prvTgfIpfixPortGroupEntryTestRestore function
* @endinternal
*
* @brief   IPFIX per port group entry manipulation test configuration restore.
*/
GT_VOID prvTgfIpfixPortGroupEntryTestRestore
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       ruleIndex;
    GT_U32       i;
    CPSS_PCL_RULE_SIZE_ENT  ruleSize;
    GT_U8        portIter;

    prvTgfPayloadDataArr[0] = 0;
    prvTgfPayloadDataArr[1] = 0;

    ruleSize = CPSS_PCL_RULE_SIZE_STD_E;

    prvTgfIpfixStagesParamsRestore();

    for( i = 0 ; i < 5 ; i++ )
    {
        ruleIndex = basicTestConf[i].ruleIndex;

        rc = prvTgfPclRuleValidStatusSet(ruleSize, ruleIndex, GT_FALSE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                     ruleSize, ruleIndex, GT_FALSE);
    }

    /* Disables ingress policy for ports 0, 18, 36, 58 */
    for( portIter = 0 ; portIter < prvTgfPortsNum ; portIter++ )
    {
        rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[portIter], GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                     portIter,
                                     GT_FALSE);
    }

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

    prvTgfPortsNum = prvTgfIPfixPortGroupSavePortsNum;
    for( portIter = 0 ; portIter < prvTgfPortsNum ; portIter ++ )
    {
        prvTgfPortsArray[portIter] = prvTgfIPfixPortGroupSavePorts[portIter];
    }
}


