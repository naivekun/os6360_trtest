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
* @file prvTgfBrgGenVid1InLearnFDBCheck.c
*
* @brief Check correctness of Vid1 field filling in the learned FDB entry
*
* @version   2
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfAutoFlow.h>

#include <bridge/prvTgfBrgGenVid1LearnFDBCheck.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
GT_BOOL                fdbVid1AssignmentEnableRestore;
GT_BOOL                fdbPortVid1LearningEnableRestore;
GT_BOOL                fdbPortLearnStatusRestore;
GT_U32                 ingressPortIdx;
CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT     muxingModeRestore = CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_TAG_1_VID_E;


/******************************* Inner functions ******************************/

/**
* @internal prvTgfBrgGenVidInLearnFDBCheckTrafficGeneratorAf function
* @endinternal
*
* @brief   Generate traffic and compare counters
*/
GT_VOID prvTgfBrgGenVidInLearnFDBCheckTrafficGeneratorAf
(
    GT_VOID
)
{
    GT_STATUS                     rc;
    TGF_MAC_ADDR                  prvTgfSaMac;
    PRV_TGF_BRG_MAC_ENTRY_STC     macEntry;
    PRV_TGF_MAC_ENTRY_KEY_STC     macEntryKey;
    TGF_PACKET_VLAN_TAG_STC       prvTgfVlanTag0;
    TGF_PACKET_VLAN_TAG_STC       prvTgfVlanTag1;
    PRV_TGF_AF_TRAFFIC_UNT             trafficUn;
    PRV_TGF_AF_BRIDGE_UNT              bridgeUn;

    /* AUTODOC:   set packets to double tagged */
    trafficUn.vlanTagType=PRV_TGF_AF_VLAN_TAG_TYPE_DOUBLE_TAGGED_E;
    tgfAutoFlowTrafficPreset(PRV_TGF_AF_TRAFFIC_PARAM_VLAN_TAG_TYPE_E,&trafficUn);
    tgfAutoFlowCreate();

    /* setup Packet */
    tgfAutoFlowTxSetup();

    tgfAutoFlowTrafficGet(PRV_TGF_AF_TRAFFIC_PARAM_MAC_SA_E, &trafficUn);
    cpssOsMemCpy(&prvTgfSaMac, &trafficUn.saMac, sizeof(TGF_MAC_ADDR));

    tgfAutoFlowTrafficGet(PRV_TGF_AF_TRAFFIC_PARAM_VLAN_TAG_0_E, &trafficUn);
    prvTgfVlanTag0 = trafficUn.vlanTag0;
    tgfAutoFlowTrafficGet(PRV_TGF_AF_TRAFFIC_PARAM_VLAN_TAG_1_E, &trafficUn);
    prvTgfVlanTag1 = trafficUn.vlanTag1;
    tgfAutoFlowBridgeGet(PRV_TGF_AF_BRIDGE_PARAM_INGRESS_PORT_IDX_E,&bridgeUn);
    ingressPortIdx = bridgeUn.ingressPortIdx;

    /* AUTODOC:   Phase 1: Check control FDB learning*/

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* SIP_6 devices require VID1 mode enabled in FDB entry */
        rc = prvTgfBrgFdbMacEntryMuxingModeGet(&muxingModeRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryMuxingModeGet: %d", prvTgfDevNum);
        rc = prvTgfBrgFdbMacEntryMuxingModeSet(CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_TAG_1_VID_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryMuxingModeSet: %d", prvTgfDevNum);
    }
    else
    {
        rc = prvTgfBrgFdbVid1AssignmentEnableGet(&fdbVid1AssignmentEnableRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbVid1AssignmentEnableGet: %d", prvTgfDevNum);
        rc = prvTgfBrgFdbVid1AssignmentEnableSet(GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbVid1AssignmentEnableSet: %d", prvTgfDevNum);
    }

    rc = prvTgfBrgFdbPortVid1LearningEnableGet(prvTgfDevNum, prvTgfPortsArray[ingressPortIdx], &fdbPortVid1LearningEnableRestore);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbPortVid1LearningEnableGet: %d", prvTgfDevNum);
    rc = prvTgfBrgFdbPortVid1LearningEnableSet(prvTgfDevNum, prvTgfPortsArray[ingressPortIdx], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbPortVid1LearningEnableSet: %d", prvTgfDevNum);

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* AUTODOC:   Send Packet */
    tgfAutoFlowTxStart();

    /* AUTODOC:   Check FDB entries phase - read FDB entries and verify MACs, VLANs and VIDs */
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = prvTgfVlanTag0.vid;
    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, &prvTgfSaMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

    /* get each FDB entry */
    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC:  Check vid1 field in FDB data */
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.vid1, prvTgfVlanTag1.vid,
                                     "macEntry.key.vid1: %d", prvTgfDevNum);
    }
    else
    {
        /* AUTODOC:  Check vid1 field in FDB data is zero */
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.vid1, 0, "macEntry.key.vid1: %d", prvTgfDevNum);
    }

    /* AUTODOC:  Phase 2: Check FDB autolearning*/
    /* AUTODOC:  Delete FDB entry */
    rc = prvTgfBrgFdbMacEntryDelete(&macEntryKey);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryDelete");

     rc = prvTgfBrgFdbPortAutoLearnEnableGet(prvTgfDevNum, prvTgfPortsArray[ingressPortIdx], &fdbPortLearnStatusRestore);
     UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbPortLearnStatusGet: %d", prvTgfDevNum);

     /*AUTODOC:  Enable auto learning on ingress ports and VLAN */
     rc = prvTgfBrgFdbPortAutoLearnEnableSet  (prvTgfDevNum, prvTgfPortsArray[ingressPortIdx],  GT_TRUE);
     UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbPortAutoLearnEnableSet  : %d", prvTgfDevNum);

     rc = prvTgfBrgVlanLearningStateSet(prvTgfVlanTag0.vid, GT_TRUE);
     UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

     /* reset ETH counters */
     rc = prvTgfEthCountersReset(prvTgfDevNum);
     UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

     /* AUTODOC:  send Packet */
     tgfAutoFlowTxStart();

     cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

     /* AUTODOC:   Check FDB entries phase - read FDB entries and verify MACs, VLANs and VID */
     rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
     UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);

     /*Check vid1 field in FDB data*/
     UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfVlanTag1.vid, macEntry.key.vid1, "macEntry.key.vid1: %d", prvTgfDevNum);
}

/**
* @internal prvTgfBrgGenVidInLearnFDBCheckRestoreAf function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfBrgGenVidInLearnFDBCheckRestoreAf
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* SIP_6 devices require VID1 mode enabled in FDB entry */
        rc = prvTgfBrgFdbMacEntryMuxingModeSet(muxingModeRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryMuxingModeSet: %d", prvTgfDevNum);
    }
    else
    {
        rc = prvTgfBrgFdbVid1AssignmentEnableSet(fdbVid1AssignmentEnableRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbVid1AssignmentEnableSet: %d", prvTgfDevNum);
    }

    rc = prvTgfBrgFdbPortVid1LearningEnableSet(prvTgfDevNum, prvTgfPortsArray[ingressPortIdx], fdbPortVid1LearningEnableRestore);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbPortVid1LearningEnableSet: %d", prvTgfDevNum);

    rc = prvTgfBrgFdbPortAutoLearnEnableSet  (prvTgfDevNum, prvTgfPortsArray[ingressPortIdx],  fdbPortLearnStatusRestore);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbPortAutoLearnEnableSet  : %d", prvTgfDevNum);

    tgfAutoFlowDel();

}

