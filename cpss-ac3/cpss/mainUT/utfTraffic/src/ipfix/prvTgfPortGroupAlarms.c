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
* @file prvTgfPortGroupAlarms.c
*
* @brief IPFIX Alarms per port group test for IPFIX
*
* @version   5
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
* @internal prvTgfIpfixPortGroupAlarmsTestInit function
* @endinternal
*
* @brief   IPFIX per port group entry manipulation test configuration set.
*/
GT_VOID prvTgfIpfixPortGroupAlarmsTestInit
(
    GT_VOID
)
{
    PRV_TGF_PCL_LOOKUP_CFG_STC lookupCfg;
    GT_STATUS               rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    GT_U32                           ipfixIndex;
    PRV_TGF_IPFIX_ENTRY_STC          ipfixEntry;
    PRV_TGF_IPFIX_WRAPAROUND_CFG_STC wraparounfConf;
    GT_U8                            portIter, ruleIndexIter;

    prvTgfIPfixPortGroupSavePortsNum = prvTgfPortsNum;
    for( portIter = 0 ; portIter < prvTgfPortsNum ; portIter ++ )
    {
        prvTgfIPfixPortGroupSavePorts[portIter] = prvTgfPortsArray[portIter];
    }

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

    cpssOsMemSet(&mask, 0, sizeof(mask));
    mask.ruleStdNotIp.etherType = 0xFFFF;

    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    cpssOsMemSet(&action, 0, sizeof(action));

    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.policer.policerEnable = PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E;

    prvTgfIpfixStagesParamsSaveAndReset();

    rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, testedStage,
                                      PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);

    cpssOsMemSet(&ipfixEntry, 0, sizeof(ipfixEntry));

    ipfixEntry.samplingAction = PRV_TGF_IPFIX_SAMPLING_ACTION_ALARM_E;
    ipfixEntry.randomFlag = PRV_TGF_IPFIX_SAMPLING_DIST_DETERMINISTIC_E;
    ipfixEntry.samplingMode = PRV_TGF_IPFIX_SAMPLING_MODE_PACKET_E;
    ipfixEntry.samplingWindow.l[1] = 0;
    ipfixEntry.samplingWindow.l[0] = 1;

    for( portIter = 0 ; portIter < prvTgfPortsNum ; portIter++ )
    {
        /* set <currPortGroupsBmp> , <usePortGroupsBmp> */
        TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(prvTgfPortsArray[portIter]);

        for( ruleIndexIter = 0 ; ruleIndexIter < 4 ; ruleIndexIter++ )
        {
            ipfixIndex = portIter * 4 + ruleIndexIter;
            pattern.ruleStdNotIp.etherType = (GT_U16) (0x1000 + ruleIndexIter);
            action.policer.policerId = ipfixIndex;

            rc = prvTgfPclRuleSet(ruleFormat, ruleIndexIter, &mask, &pattern, &action);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
                                         ruleFormat, portIter, ruleIndexIter);

            rc = prvTgfIpfixEntrySet(prvTgfDevNum, testedStage,
                                     ipfixIndex, &ipfixEntry);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                         prvTgfDevNum, testedStage, ipfixIndex);
        }
    }

    usePortGroupsBmp  = GT_FALSE;
    currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    wraparounfConf.action = PRV_TGF_IPFIX_WRAPAROUND_ACTION_NONE_E;
    wraparounfConf.dropThreshold = 0x3FFFFFFF;
    wraparounfConf.packetThreshold = 0x3FFFFFFF;
    wraparounfConf.byteThreshold.l[0] = 0xFFFFFFFF;
    wraparounfConf.byteThreshold.l[1] = 0xF;

    /* set wraparound configuration */
    rc = prvTgfIpfixWraparoundConfSet(prvTgfDevNum, testedStage, &wraparounfConf);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundConfSet: %d, %d",
                                 prvTgfDevNum, testedStage);

    /* 64 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = 44;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1000 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x00;
}

/**
* @internal prvTgfIpfixPortGroupAlarmsTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixPortGroupAlarmsTestTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter, portIterJ;
    GT_U8       i;
    GT_U32      eventsArr[PRV_TGF_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS];
    GT_U32      eventsNum;

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);

    /*make sure all ports are 'link UP'*/
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in prvTgfEthCountersReset %d", prvTgfDevNum);


    for( portIter = 0 ; portIter < prvTgfPortsNum ; portIter++ )
    {
        usePortGroupsBmp  = GT_FALSE;
        currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        /* send packet */
        for( i = 0 ; i < 4 ; i++ )
        {
            prvTgfPayloadDataArr[1] = i;
            prvTgfIpfixTestPacketSend(prvTgfPortsArray[portIter], &prvTgfPacketInfo, 1);
            cpssOsTimerWkAfter(100);
        }

        /* Clear events */
        rc = prvTgfIpfixAlarmEventsGet(prvTgfDevNum, testedStage, eventsArr, &eventsNum);

        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixAlarmEventsGet: %d, %d, %d",
                                     prvTgfDevNum, testedStage, portIter);

        /* send packet */
        for( i = 0 ; i < 4 ; i++ )
        {
            prvTgfPayloadDataArr[1] = i;
            prvTgfIpfixTestPacketSend(prvTgfPortsArray[portIter], &prvTgfPacketInfo, 1);
            cpssOsTimerWkAfter(100);
        }

        /* Get alarm events -  4 events expecterd */
        rc = prvTgfIpfixAlarmEventsGet(prvTgfDevNum, testedStage, eventsArr, &eventsNum);

        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixAlarmEventsGet: %d, %d, %d",
                                     prvTgfDevNum, testedStage, portIter);

        UTF_VERIFY_EQUAL1_STRING_MAC(4, eventsNum,
                                     "different number of alarm events then expexted: %d",
                                     portIter);

        for( i = 0 ; i < 4 ; i++ )
        {
            UTF_VERIFY_EQUAL2_STRING_MAC((portIter*4 + i), eventsArr[(i>1)?(i-2):(i+2)],
                                         "different IPFIX index then expexted: %d, %d",
                                         portIter, i);
        }

        /* send packet */
        for( i = 0 ; i < 4 ; i++ )
        {
            prvTgfPayloadDataArr[1] = i;
            prvTgfIpfixTestPacketSend(prvTgfPortsArray[portIter], &prvTgfPacketInfo, 1);
            cpssOsTimerWkAfter(100);
        }

        for( portIterJ = 0 ; portIterJ < prvTgfPortsNum ; portIterJ++ )
        {
            /* set <currPortGroupsBmp> , <usePortGroupsBmp> */
            TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(prvTgfPortsArray[portIterJ]);

            rc = prvTgfIpfixAlarmEventsGet(prvTgfDevNum, testedStage, eventsArr, &eventsNum);

            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfIpfixAlarmEventsGet: %d, %d, %d, %d",
                                         prvTgfDevNum, testedStage, portIter, portIterJ);

            if( portIterJ == portIter )
            {
                /* 4 events expecterd */
                UTF_VERIFY_EQUAL2_STRING_MAC(4, eventsNum,
                                             "different number of alarm events then expexted: %d, %d",
                                             portIter, portIterJ);

                for( i = 0 ; i < 4 ; i++ )
                {
                    UTF_VERIFY_EQUAL3_STRING_MAC((portIter*4 + i), eventsArr[(i>1)?(i-2):(i+2)],
                                                 "different IPFIX index then expexted: %d, %d, %d",
                                                 portIter, portIterJ, i);
                }
            }
            else
            {
                /* no event expected */
                UTF_VERIFY_EQUAL2_STRING_MAC(0, eventsNum,
                                             "different number of alarm events then expexted: %d, %d",
                                             portIter, portIterJ);
            }
        }

        /* restore PortGroupsBmp mode */
        usePortGroupsBmp  = GT_FALSE;
        currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    }

    for( portIter = 0 ; portIter < prvTgfPortsNum ; portIter++ )
    {
        /* send packet */
        for( i = 0 ; i < 4 ; i++ )
        {
            prvTgfPayloadDataArr[1] = i;
            prvTgfIpfixTestPacketSend(prvTgfPortsArray[portIter], &prvTgfPacketInfo, 1);
            cpssOsTimerWkAfter(100);
        }
    }

    usePortGroupsBmp  = GT_FALSE;
    currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* Get alarm events -  16 events expecterd */
    rc = prvTgfIpfixAlarmEventsGet(prvTgfDevNum, testedStage, eventsArr, &eventsNum);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixAlarmEventsGet: %d, %d",
                                 prvTgfDevNum, testedStage);

    UTF_VERIFY_EQUAL0_STRING_MAC(16, eventsNum, "different number of alarm events then expexted: %d");

    for( portIter = 0 ; portIter < prvTgfPortsNum ; portIter++ )
    {
        for( i = 0 ; i < 4 ; i++ )
        {
            UTF_VERIFY_EQUAL2_STRING_MAC((portIter*4 + i), eventsArr[portIter*4 + ((i>1)?(i-2):(i+2))],
                                         "different IPFIX index then expexted: %d, %d",
                                         portIter, i);
        }
    }

    for( portIter = 0 ; portIter < prvTgfPortsNum ; portIter++ )
    {
        /* send packet */
        for( i = 0 ; i < 4 ; i++ )
        {
            prvTgfPayloadDataArr[1] = i;
            prvTgfIpfixTestPacketSend(prvTgfPortsArray[portIter], &prvTgfPacketInfo, 1);
            cpssOsTimerWkAfter(100);
        }
    }

    for( portIter = 0 ; portIter < prvTgfPortsNum ; portIter++ )
    {
        /* set <currPortGroupsBmp> , <usePortGroupsBmp> */
        TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(prvTgfPortsArray[portIter]);

        rc = prvTgfIpfixAlarmEventsGet(prvTgfDevNum, testedStage, eventsArr, &eventsNum);

        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixAlarmEventsGet: %d, %d, %d",
                                     prvTgfDevNum, testedStage, portIter);

        /* 4 events expecterd */
        UTF_VERIFY_EQUAL1_STRING_MAC(4, eventsNum,
                                     "different number of alarm events then expexted: %d",
                                      portIter);

        for( i = 0 ; i < 4 ; i++ )
        {
            UTF_VERIFY_EQUAL2_STRING_MAC((portIter*4 + i), eventsArr[(i>1)?(i-2):(i+2)],
                                         "different IPFIX index then expexted: %d, %d",
                                         portIter, i);
        }
    }

    /* restore PortGroupsBmp mode */
    usePortGroupsBmp  = GT_FALSE;
    currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfIpfixPortGroupAlarmsTestRestore function
* @endinternal
*
* @brief   IPFIX per port group entry manipulation test configuration restore.
*/
GT_VOID prvTgfIpfixPortGroupAlarmsTestRestore
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       ruleIndex;
    CPSS_PCL_RULE_SIZE_ENT  ruleSize;
    GT_U8        portIter;

    prvTgfPayloadDataArr[0] = 0;
    prvTgfPayloadDataArr[1] = 0;

    ruleSize = CPSS_PCL_RULE_SIZE_STD_E;

    prvTgfIpfixStagesParamsRestore();

    /* restore PortGroupsBmp mode */
    usePortGroupsBmp  = GT_FALSE;
    currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    for( ruleIndex = 0 ; ruleIndex < 4 ; ruleIndex++ )
    {
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


