/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfAging.c
*
* DESCRIPTION:
*       Wraparound tests for IPFIX
*
* FILE REVISION NUMBER:
*       $Revision: 7 $
*
*******************************************************************************/

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
#include <ipfix/prvTgfWraparound.h>

#define PRV_TGF_BYTE_WA_THRESHOLD 0x40000

/**
* @internal prvTgfIpfixWraparoundTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixWraparoundTestInit
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
* @internal prvTgfIpfixWraparoundFreezeTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixWraparoundFreezeTestTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter = 0;
    /* CPSS_PORT_MAC_COUNTER_SET_STC portCntrs; */
    GT_U32      burstCount;
    GT_U32      bmp;
    PRV_TGF_IPFIX_WRAPAROUND_CFG_STC       wraparounfConfSet, wraparounfConfGet;
    PRV_TGF_IPFIX_ENTRY_STC ipfixEntry;

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

    /* set <currPortGroupsBmp> , <usePortGroupsBmp> */
    TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* clear wraparound bit map */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                        basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                        GT_TRUE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_TRUE);

    wraparounfConfSet.action = PRV_TGF_IPFIX_WRAPAROUND_ACTION_NONE_E;
    wraparounfConfSet.dropThreshold = 0xFFFF;
    wraparounfConfSet.packetThreshold = 2;
    wraparounfConfSet.byteThreshold.l[0] = PRV_TGF_BYTE_WA_THRESHOLD;
    wraparounfConfSet.byteThreshold.l[1] = 0;

    /* set wraparound configuration */
    rc = prvTgfIpfixWraparoundConfSet(prvTgfDevNum, testedStage, &wraparounfConfSet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundConfSet: %d, %d, %d",
                                 prvTgfDevNum, testedStage);

    rc = prvTgfIpfixWraparoundConfGet(prvTgfDevNum, testedStage, &wraparounfConfGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundConfSet: %d, %d, %d",
                                 prvTgfDevNum, testedStage);

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.action, wraparounfConfGet.action,
                                 "Get wraparound action deifferent then set");

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.dropThreshold, wraparounfConfGet.dropThreshold,
                                 "Get wraparound dropThreshold deifferent then set");

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.packetThreshold, wraparounfConfGet.packetThreshold,
                                 "Get wraparound packetThreshold deifferent then set");

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.byteThreshold.l[0], wraparounfConfGet.byteThreshold.l[0],
                                 "Get wraparound byteThreshold.l[0] deifferent then set");

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.byteThreshold.l[1], wraparounfConfGet.byteThreshold.l[1],
                                 "Get wraparound byteThreshold.l[1] deifferent then set");

    /* 64 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[0].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1000 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x00;

    burstCount = 2;

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, burstCount);

    cpssOsTimerWkAfter(400);

    /* check IPFIX counters */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[0].ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex);

    /* check IPFIX counters */

    /* Due to errata, when working in wraparound freeze mode, if one counter reach its limit,   */
    /* the other counters stop counting as well, but does not count the last increment that was */
    /* the reason for the freeze. */
    UTF_VERIFY_EQUAL0_STRING_MAC((wraparounfConfSet.packetThreshold - 1) *
                                 (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[1] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.packetThreshold, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* check bit map with reset flag disabled */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   GT_FALSE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC(1, (bmp & 1),
                                 "Wraparound indication expected");

    /* check bit map with reset flag enabled */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   GT_TRUE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC(1, (bmp & 1),
                                 "Wraparound indication expected");

    /* check bit map is cleared */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   GT_TRUE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC(0, (bmp & 1),
                                 "Wraparound indication expected");

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, 1);

    /* check IPFIX counters */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[0].ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex);

    /* check IPFIX counters */
    /* Due to errata, when working in wraparound freeze mode, if one counter reach its limit,   */
    /* the other counters stop counting as well, but does not count the last increment that was */
    /* the reason for the freeze. */
    UTF_VERIFY_EQUAL0_STRING_MAC((wraparounfConfSet.packetThreshold - 1) *
                                 (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[1] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.packetThreshold, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* check bit map with reset flag enabled */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   GT_TRUE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_FALSE);

#ifndef ASIC_SIMULATION
    UTF_VERIFY_EQUAL0_STRING_MAC(1, (bmp & 1),
                                 "Wraparound indication expected");
#endif /* ASIC_SIMULATION */

    /* check bit map is cleared */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   GT_TRUE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC(0, (bmp & 1),
                                 "Wraparound indication expected");

    /* read entry with reset flag enabled for clear */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[0].ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex);

    /* read entry to verify clear counters */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[0].ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex);

    if (!prvTgfIpfixIsResetOnReadErratum(prvTgfDevNum))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                     "btyeCount.l[1] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.packetCount,
                                     "packetCount different then expected");
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC((wraparounfConfSet.packetThreshold - 1) *
                                     (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                     ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                     "btyeCount.l[1] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.packetThreshold, ipfixEntry.packetCount,
                                     "packetCount different then expected");

        /* reset counters for the following entry set */
        ipfixEntry.byteCount.l[0] = ipfixEntry.byteCount.l[1] = ipfixEntry.packetCount = 0;
    }



    /* clear WA bit */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   GT_TRUE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_FALSE);

    ipfixEntry.byteCount.l[0] = PRV_TGF_BYTE_WA_THRESHOLD - 200;
    rc = prvTgfIpfixEntrySet(prvTgfDevNum, testedStage,
                             basicTestConf[0].ipfixIndex, &ipfixEntry);

    /* 200 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[0].payloadLen = 200;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, burstCount);

    cpssOsTimerWkAfter(400);

    /* check IPFIX counters */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[0].ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.byteThreshold.l[0],
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[1] different then expected");
    /* Due to errata, when working in wraparound freeze mode, if one counter reach its limit,   */
    /* the other counters stop counting as well, but does not count the last increment that was */
    /* the reason for the freeze. */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount - 2, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* check bit map with reset flag disabled */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   GT_FALSE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC(1, (bmp & 1),
                                 "Wraparound indication expected");

    /* check bit map with reset flag enabled */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   GT_TRUE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC(1, (bmp & 1),
                                 "Wraparound indication expected");

    /* check bit map is cleared */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   GT_TRUE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC(0, (bmp & 1),
                                 "Wraparound indication expected");

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, 1);

    /* check IPFIX counters */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[0].ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.byteThreshold.l[0],
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[1] different then expected");
    /* Due to errata, when working in wraparound freeze mode, if one counter reach its limit,   */
    /* the other counters stop counting as well, but does not count the last increment that was */
    /* the reason for the freeze. */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount - 2, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* check bit map with reset flag enabled */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   GT_TRUE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_FALSE);

#ifndef ASIC_SIMULATION
    UTF_VERIFY_EQUAL0_STRING_MAC(1, (bmp & 1),
                                 "Wraparound indication expected");
#endif /* ASIC_SIMULATION */

    /* check bit map is cleared */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   GT_TRUE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC(0, (bmp & 1),
                                 "Wraparound indication expected");

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
* @internal prvTgfIpfixWraparoundClearTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixWraparoundClearTestTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter = 0;
    /* CPSS_PORT_MAC_COUNTER_SET_STC portCntrs; */
    GT_U32      burstCount;
    GT_U32      bmp;
    PRV_TGF_IPFIX_WRAPAROUND_CFG_STC       wraparounfConfSet, wraparounfConfGet;
    PRV_TGF_IPFIX_ENTRY_STC ipfixEntry;

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

    /* set <currPortGroupsBmp> , <usePortGroupsBmp> */
    TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* clear wraparound bit map */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                        basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                        GT_TRUE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_TRUE);

    wraparounfConfSet.action = PRV_TGF_IPFIX_WRAPAROUND_ACTION_CLEAR_E;
    wraparounfConfSet.dropThreshold = 0xFFFF;
    wraparounfConfSet.packetThreshold = 3;
    wraparounfConfSet.byteThreshold.l[0] = PRV_TGF_BYTE_WA_THRESHOLD;
    wraparounfConfSet.byteThreshold.l[1] = 0;

    /* set wraparound configuration */
    rc = prvTgfIpfixWraparoundConfSet(prvTgfDevNum, testedStage, &wraparounfConfSet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundConfSet: %d, %d, %d",
                                 prvTgfDevNum, testedStage);

    rc = prvTgfIpfixWraparoundConfGet(prvTgfDevNum, testedStage, &wraparounfConfGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundConfSet: %d, %d, %d",
                                 prvTgfDevNum, testedStage);

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.action, wraparounfConfGet.action,
                                 "Get wraparound action deifferent then set");

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.dropThreshold, wraparounfConfGet.dropThreshold,
                                 "Get wraparound dropThreshold deifferent then set");

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.packetThreshold, wraparounfConfGet.packetThreshold,
                                 "Get wraparound packetThreshold deifferent then set");

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.byteThreshold.l[0], wraparounfConfGet.byteThreshold.l[0],
                                 "Get wraparound byteThreshold.l[0] deifferent then set");

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.byteThreshold.l[1], wraparounfConfGet.byteThreshold.l[1],
                                 "Get wraparound byteThreshold.l[1] deifferent then set");

    /* 64 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[0].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1000 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x00;

    burstCount = 4;

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, burstCount);

    cpssOsTimerWkAfter(500);

    /* check IPFIX counters */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[0].ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[1] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount - wraparounfConfSet.packetThreshold, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* check bit map with reset flag disabled */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   GT_FALSE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC((bmp & 1), 1,
                                 "Wraparound indication expected");

    /* check bit map with reset flag enabled */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   GT_TRUE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC((bmp & 1), 1,
                                 "Wraparound indication expected");

    /* check bit map is cleared */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   GT_TRUE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_TRUE);

    UTF_VERIFY_EQUAL0_STRING_MAC((bmp & 1), 0,
                                 "Wraparound indication expected");

    /* read entry with reset flag enabled for clear */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[0].ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex);

    /* read entry to verify clear counters */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[0].ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex);

    if (!prvTgfIpfixIsResetOnReadErratum(prvTgfDevNum))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                     "btyeCount.l[1] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.packetCount,
                                     "packetCount different then expected");
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                     ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                     "btyeCount.l[1] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount - wraparounfConfSet.packetThreshold, ipfixEntry.packetCount,
                                     "packetCount different then expected");

        ipfixEntry.byteCount.l[0] = ipfixEntry.byteCount.l[1] = ipfixEntry.packetCount = 0;
    }

    ipfixEntry.byteCount.l[0] = PRV_TGF_BYTE_WA_THRESHOLD - 400;
    rc = prvTgfIpfixEntrySet(prvTgfDevNum, testedStage,
                             basicTestConf[0].ipfixIndex, &ipfixEntry);

    /* 200 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[0].payloadLen = 200;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    burstCount = 2;

    /* clear WA bit */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   GT_TRUE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_TRUE);

    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, burstCount);

    cpssOsTimerWkAfter(400);

    /* check IPFIX counters */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[0].ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC((burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS))
                                 - 400,
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* check bit map with reset flag disabled */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   GT_FALSE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC((bmp & 1), 1,
                                 "Wraparound indication expected");

    /* check bit map with reset flag enabled */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   GT_TRUE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC((bmp & 1), 1,
                                 "Wraparound indication expected");

    /* check bit map is cleared */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   GT_TRUE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC((bmp & 1), 0,
                                 "Wraparound indication expected");

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
* @internal prvTgfIpfixWraparoundMaxBytesTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixWraparoundMaxBytesTestTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter = 0;
    /* CPSS_PORT_MAC_COUNTER_SET_STC portCntrs; */
    GT_U32      burstCount;
    GT_U32      bmp;
    PRV_TGF_IPFIX_WRAPAROUND_CFG_STC       wraparounfConfSet, wraparounfConfGet;
    PRV_TGF_IPFIX_ENTRY_STC ipfixEntry;

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

    /* set <currPortGroupsBmp> , <usePortGroupsBmp> */
    TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* clear wraparound bit map */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                        basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                        GT_TRUE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_TRUE);

    wraparounfConfSet.action = PRV_TGF_IPFIX_WRAPAROUND_ACTION_CLEAR_E;
    wraparounfConfSet.dropThreshold = 0xF;
    wraparounfConfSet.packetThreshold = 0xF;
    wraparounfConfSet.byteThreshold.l[0] = 0xFFFC0000;
    wraparounfConfSet.byteThreshold.l[1] = 0xF;

    /* set wraparound configuration */
    rc = prvTgfIpfixWraparoundConfSet(prvTgfDevNum, testedStage, &wraparounfConfSet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundConfSet: %d, %d, %d",
                                 prvTgfDevNum, testedStage);

    rc = prvTgfIpfixWraparoundConfGet(prvTgfDevNum, testedStage, &wraparounfConfGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundConfSet: %d, %d, %d",
                                 prvTgfDevNum, testedStage);

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.action, wraparounfConfGet.action,
                                 "Get wraparound action deifferent then set");

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.dropThreshold, wraparounfConfGet.dropThreshold,
                                 "Get wraparound dropThreshold deifferent then set");

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.packetThreshold, wraparounfConfGet.packetThreshold,
                                 "Get wraparound packetThreshold deifferent then set");

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.byteThreshold.l[0], wraparounfConfGet.byteThreshold.l[0],
                                 "Get wraparound byteThreshold.l[0] deifferent then set");

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.byteThreshold.l[1], wraparounfConfGet.byteThreshold.l[1],
                                 "Get wraparound byteThreshold.l[1] deifferent then set");

    /* simple wraparound test */
    /* set IPFIX counters */
    cpssOsMemSet(&ipfixEntry, 0, sizeof(ipfixEntry));
    ipfixEntry.byteCount.l[0] = 0xFFFC0000;
    ipfixEntry.byteCount.l[1] = 0xF;
    rc = prvTgfIpfixEntrySet(prvTgfDevNum, testedStage,
                             basicTestConf[0].ipfixIndex, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex);

    /* 64 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[0].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1000 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x00;

    burstCount = 1;

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, burstCount);

    cpssOsTimerWkAfter(200);

    /* get IPFIX counters */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[0].ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS,
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[1] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(1, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* wraparound test - MAX BYTES value threshold and freeze mode */
    wraparounfConfSet.action = PRV_TGF_IPFIX_WRAPAROUND_ACTION_NONE_E;

    /* set wraparound configuration */
    rc = prvTgfIpfixWraparoundConfSet(prvTgfDevNum, testedStage, &wraparounfConfSet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundConfSet: %d, %d, %d",
                                 prvTgfDevNum, testedStage);

    rc = prvTgfIpfixWraparoundConfGet(prvTgfDevNum, testedStage, &wraparounfConfGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundConfSet: %d, %d, %d",
                                 prvTgfDevNum, testedStage);

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.action, wraparounfConfGet.action,
                                 "Get wraparound action deifferent then set");

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.dropThreshold, wraparounfConfGet.dropThreshold,
                                 "Get wraparound dropThreshold deifferent then set");

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.packetThreshold, wraparounfConfGet.packetThreshold,
                                 "Get wraparound packetThreshold deifferent then set");

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.byteThreshold.l[0], wraparounfConfGet.byteThreshold.l[0],
                                 "Get wraparound byteThreshold.l[0] deifferent then set");

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.byteThreshold.l[1], wraparounfConfGet.byteThreshold.l[1],
                                 "Get wraparound byteThreshold.l[1] deifferent then set");

    /* set IPFIX counters */
    cpssOsMemSet(&ipfixEntry, 0, sizeof(ipfixEntry));
    ipfixEntry.byteCount.l[0] = 0xFFFBFFFF;
    ipfixEntry.byteCount.l[1] = 0xF;
    rc = prvTgfIpfixEntrySet(prvTgfDevNum, testedStage,
                             basicTestConf[0].ipfixIndex, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex);

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, burstCount);

    cpssOsTimerWkAfter(200);

    /* get IPFIX counters */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[0].ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.byteThreshold.l[0],
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.byteThreshold.l[1],
                                 ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[1] different then expected");

    /* Due to errata, when working in wraparound freeze mode, if one counter reach its limit,   */
    /* the other counters stop counting as well, but does not count the last increment that was */
    /* the reason for the freeze. */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount - 1, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* check bit map with reset flag disabled */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   GT_TRUE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC(1, (bmp & 1),
                                 "Wraparound indication expected");

    /* check bit map with reset flag enabled */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   GT_TRUE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC(0, (bmp & 1),
                                 "No wraparound indication expected");

    /* wraparound test - MAX BYTES value threshold and clear mode */
    wraparounfConfSet.action = PRV_TGF_IPFIX_WRAPAROUND_ACTION_CLEAR_E;

    /* set wraparound configuration */
    rc = prvTgfIpfixWraparoundConfSet(prvTgfDevNum, testedStage, &wraparounfConfSet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundConfSet: %d, %d, %d",
                                 prvTgfDevNum, testedStage);

    rc = prvTgfIpfixWraparoundConfGet(prvTgfDevNum, testedStage, &wraparounfConfGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundConfSet: %d, %d, %d",
                                 prvTgfDevNum, testedStage);

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.action, wraparounfConfGet.action,
                                 "Get wraparound action deifferent then set");

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.dropThreshold, wraparounfConfGet.dropThreshold,
                                 "Get wraparound dropThreshold deifferent then set");

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.packetThreshold, wraparounfConfGet.packetThreshold,
                                 "Get wraparound packetThreshold deifferent then set");

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.byteThreshold.l[0], wraparounfConfGet.byteThreshold.l[0],
                                 "Get wraparound byteThreshold.l[0] deifferent then set");

    UTF_VERIFY_EQUAL0_STRING_MAC(wraparounfConfSet.byteThreshold.l[1], wraparounfConfGet.byteThreshold.l[1],
                                 "Get wraparound byteThreshold.l[1] deifferent then set");

    /* set IPFIX counters */
    cpssOsMemSet(&ipfixEntry, 0, sizeof(ipfixEntry));
    ipfixEntry.byteCount.l[0] = 0xFFFBFFFF;
    ipfixEntry.byteCount.l[1] = 0xF;
    rc = prvTgfIpfixEntrySet(prvTgfDevNum, testedStage,
                             basicTestConf[0].ipfixIndex, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex);

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, burstCount);

    cpssOsTimerWkAfter(200);

    /* get IPFIX counters */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             basicTestConf[0].ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS - 1,
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[1] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(1, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* check bit map with reset flag disabled */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   GT_TRUE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC(1, (bmp & 1),
                                 "Wraparound indication expected");

    /* check bit map with reset flag enabled */
    rc = prvTgfIpfixWraparoundStatusGet(prvTgfDevNum, testedStage,
                                   basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                   GT_TRUE, &bmp);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 basicTestConf[0].ipfixIndex,basicTestConf[0].ipfixIndex,
                                 GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC(0, (bmp & 1),
                                 "No wraparound indication expected");

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
* @internal prvTgfIpfixWraparoundTestRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfIpfixWraparoundTestRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    PRV_TGF_IPFIX_WRAPAROUND_CFG_STC       wraparounfConf;

    wraparounfConf.action = PRV_TGF_IPFIX_WRAPAROUND_ACTION_NONE_E;
    wraparounfConf.dropThreshold = 0;
    wraparounfConf.packetThreshold = 0;
    wraparounfConf.byteThreshold.l[0] = 0x40000;
    wraparounfConf.byteThreshold.l[1] = 0;

    /* set wraparound configuration */
    rc = prvTgfIpfixWraparoundConfSet(prvTgfDevNum, testedStage, &wraparounfConf);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundConfSet: %d, %d, %d",
                                 prvTgfDevNum, testedStage);

    prvTgfIpfixIngressTestRestore();
}


