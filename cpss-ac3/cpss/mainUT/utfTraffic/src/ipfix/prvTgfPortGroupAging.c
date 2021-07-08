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
* @file prvTgfPortGroupAging.c
*
* @brief IPFIX Aging per port group test for IPFIX
*
* @version   2
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
#include <ipfix/prvTgfPortGroupEntry.h>

/**
* @internal prvTgfIpfixPortGroupAgingCheck function
* @endinternal
*
* @brief   Check that the 1st bit in a requested aging bitmap is the same as an
*         expected one.
* @param[in] enPortGroupsBmp          - state if to uses port groups bmp.
* @param[in] portGroupsBmp            - bmp of port groups to use in test.
* @param[in] reset                    -  bmp value enabling after read.
* @param[in] expectedBmp              - the expected value.
* @param[in] stepNum                  - printing mark auxilery.
* @param[in] bookmark                 - second printing mark auxilery.
*                                       None
*/
GT_VOID prvTgfIpfixPortGroupAgingCheck
(
    GT_BOOL     enPortGroupsBmp,
    GT_PORT_GROUPS_BMP  portGroupsBmp,
    GT_BOOL     reset,
    GT_U32      expectedBmp,
    GT_U32      stepNum,
    GT_U32      bookmark
)
{
    GT_STATUS rc;
    GT_U32    bmp[1];

    /* set PortGroupsBmp mode */
    usePortGroupsBmp  = enPortGroupsBmp;
    currPortGroupsBmp = portGroupsBmp;

    /* check aging bit map */
    rc = prvTgfIpfixAgingStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   reset, bmp);
    UTF_VERIFY_EQUAL7_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingStatusGet: %d, %d, %d, %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[2].ipfixIndex,
                                 reset, stepNum, bookmark);

    UTF_VERIFY_EQUAL2_STRING_MAC(expectedBmp, (bmp[0] & 0x1), "Aging indication expected: %d, %d",
                                 stepNum, bookmark);
}

/**
* @internal prvTgfIpfixPortGroupAgingTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixPortGroupAgingTestTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter, portIterJ;
    GT_BOOL     enable;
    GT_U32      bmp[1];
    GT_U32      stepNum;

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

    /* clear aging bit map */
    rc = prvTgfIpfixAgingStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   GT_TRUE, bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[2].ipfixIndex,
                                 GT_TRUE);

    /* enabling aging */
    rc = prvTgfIpfixAgingEnableSet(prvTgfDevNum, testedStage, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingEnableSet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, GT_TRUE);

    rc = prvTgfIpfixAgingEnableGet(prvTgfDevNum, testedStage, &enable);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingEnableGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage);

    UTF_VERIFY_EQUAL0_STRING_MAC(enable, GT_TRUE,
                                 "Aging is expected to be enabled but disabled");

    prvTgfIpfixPortGroupAgingCheck(GT_FALSE, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                   GT_FALSE, 0, 0, /* bookmark */1);

    for( portIter = 0 ; portIter < prvTgfPortsNum ; portIter++ )
    {
        prvTgfIpfixPortGroupAgingCheck(GT_TRUE,
                                       (1 << (PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(prvTgfDevsArray[portIter], prvTgfPortsArray[portIter]))),
                                       GT_FALSE, 0, portIter, /* bookmark */2);
    }

    for( portIter = 0 ; portIter < prvTgfPortsNum ; portIter++ )
    {
        /* send packet */
        prvTgfIpfixTestPacketSend(prvTgfPortsArray[portIter], &prvTgfPacketInfo, 1);

        cpssOsTimerWkAfter(400);

        prvTgfIpfixPortGroupAgingCheck(GT_FALSE, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                       GT_FALSE, 1, portIter, /* bookmark */3);

        for( portIterJ = 0 ; portIterJ < prvTgfPortsNum ; portIterJ++ )
        {
            /* use PortGroupsBmp API in the test */
            /* set <currPortGroupsBmp> */
            TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(prvTgfPortsArray[portIterJ]);

            /* check aging bit map */
            rc = prvTgfIpfixAgingStatusGet(prvTgfDevNum, testedStage,
                                           basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                           GT_FALSE, bmp);
            UTF_VERIFY_EQUAL7_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingStatusGet: %d, %d, %d, %d, %d, %d, %d",
                                         prvTgfDevNum, testedStage,
                                         basicTestConf[0].ipfixIndex,basicTestConf[2].ipfixIndex,
                                         GT_FALSE, portIter, portIterJ);

            UTF_VERIFY_EQUAL2_STRING_MAC((( portIterJ == portIter ) ? 1 : 0),
                                         (bmp[0] & 0x1),
                                         "Aging indication expected: %d, %d", portIter, portIterJ);
        }

        prvTgfIpfixPortGroupAgingCheck(GT_FALSE, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                       GT_TRUE, 1, portIter, /* bookmark */4);

        prvTgfIpfixPortGroupAgingCheck(GT_TRUE,
                                       (1 << (PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(prvTgfDevsArray[portIter], prvTgfPortsArray[portIter]))),
                                       GT_FALSE, 0, portIter, /* bookmark */5);
    }

    /* send packet on port 0 & 36 */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[0], &prvTgfPacketInfo, 1);
    cpssOsTimerWkAfter(400);
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[2], &prvTgfPacketInfo, 1);
    cpssOsTimerWkAfter(400);

    /* step 1 */
    stepNum = 1;
    prvTgfIpfixPortGroupAgingCheck(GT_FALSE, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                   GT_FALSE, 1, stepNum++, /* bookmark */0);

    /* step 2 */
    prvTgfIpfixPortGroupAgingCheck(GT_TRUE, (1 << (PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(prvTgfDevsArray[0], prvTgfPortsArray[0]))), GT_FALSE, 1, stepNum++, /* bookmark */0);

    /* step 3 */
    prvTgfIpfixPortGroupAgingCheck(GT_TRUE, (1 << (PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(prvTgfDevsArray[1], prvTgfPortsArray[1]))), GT_FALSE, 0, stepNum++, /* bookmark */0);

    /* step 4 */
    prvTgfIpfixPortGroupAgingCheck(GT_TRUE, (1 << (PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(prvTgfDevsArray[2], prvTgfPortsArray[2]))), GT_FALSE, 1, stepNum++, /* bookmark */0);

    /* step 5 */
    prvTgfIpfixPortGroupAgingCheck(GT_TRUE, (1 << (PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(prvTgfDevsArray[3], prvTgfPortsArray[3]))), GT_FALSE, 0, stepNum++, /* bookmark */0);

    /* step 6 */
    prvTgfIpfixPortGroupAgingCheck(GT_TRUE, (1 << (PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(prvTgfDevsArray[0], prvTgfPortsArray[0]))), GT_TRUE, 1, stepNum++, /* bookmark */0);

    /* step 7 */
    prvTgfIpfixPortGroupAgingCheck(GT_TRUE, (1 << (PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(prvTgfDevsArray[1], prvTgfPortsArray[1]))), GT_FALSE, 0, stepNum++, /* bookmark */0);

    /* step 8 */
    prvTgfIpfixPortGroupAgingCheck(GT_TRUE, (1 << (PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(prvTgfDevsArray[2], prvTgfPortsArray[2]))), GT_FALSE, 1, stepNum++, /* bookmark */0);

    /* step 9 */
    prvTgfIpfixPortGroupAgingCheck(GT_TRUE, (1 << (PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(prvTgfDevsArray[3], prvTgfPortsArray[3]))), GT_FALSE, 0, stepNum++, /* bookmark */0);

    /* step 10 */
    prvTgfIpfixPortGroupAgingCheck(GT_FALSE, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                   GT_FALSE, 1, stepNum++, /* bookmark */0);
    /* step 11 */
    prvTgfIpfixPortGroupAgingCheck(GT_TRUE, (1 << (PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(prvTgfDevsArray[2], prvTgfPortsArray[2]))), GT_TRUE, 1, stepNum++, /* bookmark */0);

    /* step 12 */
    prvTgfIpfixPortGroupAgingCheck(GT_TRUE, (1 << (PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(prvTgfDevsArray[0], prvTgfPortsArray[0]))), GT_FALSE, 0, stepNum++, /* bookmark */0);

    /* step 13 */
    prvTgfIpfixPortGroupAgingCheck(GT_TRUE, (1 << (PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(prvTgfDevsArray[1], prvTgfPortsArray[1]))), GT_FALSE, 0, stepNum++, /* bookmark */0);

    /* step 14 */
    prvTgfIpfixPortGroupAgingCheck(GT_TRUE, (1 << (PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(prvTgfDevsArray[2], prvTgfPortsArray[2]))), GT_FALSE, 0, stepNum++, /* bookmark */0);

    /* step 15 */
    prvTgfIpfixPortGroupAgingCheck(GT_TRUE, (1 << (PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(prvTgfDevsArray[3], prvTgfPortsArray[3]))), GT_FALSE, 0, stepNum++, /* bookmark */0);

    /* step 16 */
    prvTgfIpfixPortGroupAgingCheck(GT_FALSE, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                   GT_FALSE, 0, stepNum++, /* bookmark */0);

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

