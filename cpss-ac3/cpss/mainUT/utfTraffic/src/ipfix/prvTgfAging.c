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
* @file prvTgfAging.c
*
* @brief Aging test for IPFIX
*
* @version   3
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
#include <ipfix/prvTgfAging.h>

/**
* @internal prvTgfIpfixAgingTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixAgingTestInit
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
* @internal prvTgfIpfixAgingTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixAgingTestTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter = 0;
    /* CPSS_PORT_MAC_COUNTER_SET_STC portCntrs; */
    GT_U32      burstCount;
    GT_BOOL     enable;
    GT_U32      bmp[1];
    GT_U32      bmpMask;
    GT_U32      expectedBmp;

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

    /* disable aging */
    rc = prvTgfIpfixAgingEnableSet(prvTgfDevNum, testedStage, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingEnableSet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, GT_FALSE);

    rc = prvTgfIpfixAgingEnableGet(prvTgfDevNum, testedStage, &enable);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingEnableGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage);

    UTF_VERIFY_EQUAL0_STRING_MAC(enable, GT_FALSE,
                                 "Aging is expected to be disabled but enabled");

    /* clear aging bit map */
    rc = prvTgfIpfixAgingStatusGet(prvTgfDevNum, testedStage, 
                                   basicTestConf[0].ipfixIndex,basicTestConf[2].ipfixIndex,
                                   GT_TRUE,bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage, 
                                 basicTestConf[0].ipfixIndex,basicTestConf[2].ipfixIndex,
                                 GT_TRUE);

    bmpMask = 1 | ( 1 << (basicTestConf[1].ipfixIndex - basicTestConf[0].ipfixIndex) ) |
                  ( 1 << (basicTestConf[2].ipfixIndex - basicTestConf[0].ipfixIndex) );
    expectedBmp = 1 | ( 1 << (basicTestConf[2].ipfixIndex - basicTestConf[0].ipfixIndex) );

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

    /* Set ethertype to value 0x1002 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x02;

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, burstCount);

    cpssOsTimerWkAfter(200);

    /* check bit map is still clear */
    rc = prvTgfIpfixAgingStatusGet(prvTgfDevNum, testedStage, 
                                   basicTestConf[0].ipfixIndex,basicTestConf[2].ipfixIndex,
                                   GT_FALSE, bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage, 
                                 basicTestConf[0].ipfixIndex,basicTestConf[2].ipfixIndex,
                                 GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC((bmp[0] & bmpMask), 0,
                                 "No aging indication expected");

    /* enabling aging */
    rc = prvTgfIpfixAgingEnableSet(prvTgfDevNum, testedStage, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingEnableSet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, GT_TRUE);

    rc = prvTgfIpfixAgingEnableGet(prvTgfDevNum, testedStage, &enable);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingEnableGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage);

    UTF_VERIFY_EQUAL0_STRING_MAC(enable, GT_TRUE,
                                 "Aging is expected to be enabled but disabled");
    
    /* Set ethertype to value 0x1000 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x00;

    burstCount = basicTestConf[0].burstCount;

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, burstCount);

    cpssOsTimerWkAfter(200);

    /* Set ethertype to value 0x1002 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x02;

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, burstCount);

    cpssOsTimerWkAfter(200);

    /* check aging bit map updated correctly */
    rc = prvTgfIpfixAgingStatusGet(prvTgfDevNum, testedStage, 
                                   basicTestConf[0].ipfixIndex,basicTestConf[2].ipfixIndex,
                                   GT_FALSE, bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage, 
                                 basicTestConf[0].ipfixIndex,basicTestConf[2].ipfixIndex,
                                 GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC((bmp[0] & bmpMask), expectedBmp,
                                 "Aging indication expected");

    /* read aging bit map again with reset flag enabled */
    rc = prvTgfIpfixAgingStatusGet(prvTgfDevNum, testedStage, 
                                   basicTestConf[0].ipfixIndex,basicTestConf[2].ipfixIndex,
                                   GT_TRUE, bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage, 
                                 basicTestConf[0].ipfixIndex,basicTestConf[2].ipfixIndex,
                                 GT_TRUE);

    UTF_VERIFY_EQUAL0_STRING_MAC((bmp[0] & bmpMask), expectedBmp,
                                 "Aging indication expected");

    /* verify bit map is cleared */
    rc = prvTgfIpfixAgingStatusGet(prvTgfDevNum, testedStage, 
                                   basicTestConf[0].ipfixIndex,basicTestConf[2].ipfixIndex,
                                   GT_TRUE, bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage, 
                                 basicTestConf[0].ipfixIndex,basicTestConf[2].ipfixIndex,
                                 GT_TRUE);

    UTF_VERIFY_EQUAL0_STRING_MAC((bmp[0] & bmpMask), 0,
                                 "No aging indication expected");
    
    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}


