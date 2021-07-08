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
* @file prvTgfBridgeFdb_ForMultiCoreFdbLookupUT.h
*
* @brief enhanced UT for Bridge FDB - for multi port group fdb lookup
*
* tests according to design doc in documentum :
* http://docil.marvell.com/webtop/drl/objectId/0900dd88800a0077
*
* @version   3
********************************************************************************
*/
#ifndef __prvTgfBridgeFdb_ForMultiCoreFdbLookupUT
#define __prvTgfBridgeFdb_ForMultiCoreFdbLookupUT

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/bridge/cpssGenBrgFdb.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCscdGen.h>

/**
* @internal prvTgfMultiPortGroupInit_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   function init the DB of the multi-port groups
*/
GT_VOID prvTgfMultiPortGroupInit_ForMultiCoreFdbLookup
(
    GT_VOID
);


/**
* @internal tgfBridgeFdbAddDeleteEntryRun_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   4.1    Test 3: check add/delete (by index/message) of FDB entry
*         (Covered API: FDB :
*         1.    cpssDxChBrgFdbPortGroupMacEntrySet
*         a.    cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet
*         2.    cpssDxChBrgFdbPortGroupMacEntryDelete
*         a.    cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet
*         3.    cpssDxChBrgFdbPortGroupMacEntryWrite
*         4.    cpssDxChBrgFdbPortGroupMacEntryRead
*         5.    cpssDxChBrgFdbPortGroupMacEntryInvalidate
*         )
*         1.    check add/delete (by index/message) of FDB entry :
*         a.    add the same FDB entry (mac + vlan) , but with different destination port to each port group , meaning:
*         i.    port group 0 - FDB entry , on port N1
*         ii.    port group 1 - FDB entry , on port N2
*         iii.    port group 2 - FDB entry , on port N3
*         iv.    port group 3 - FDB entry , on port N4
*         b.    Refresh FDB to check that add is ok.
*         c.    NOTEs:
*         i.    when add/delete 'by message' need to call cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet to check that operation succeeded on the needed port groups
*         b.    send traffic from all uplinks to this mac address :
*         a.    In 128K mode : check that egress N1
*         b.    In 64K mode :
*         i.    when Uplink is U1,U2
*         1.    check that egress N1
*         ii.    else
*         1.    check that egress N3
*         c.    delete the entry from port group 0
*         d.    Refresh FDB to check that delete is ok.
*         e.    send traffic from all uplinks to this mac address :
*         a.    In 128K mode : check that egress N2
*         b.    In 64K mode :
*         i.    when Uplink is U1,U2
*         1.    check that egress N2
*         ii.    else
*         1.    check that egress N3
*         f.    delete the entry from port group 1
*         g.    Refresh FDB to check that delete is ok.
*         h.    send traffic from all uplinks to this mac address :
*         a.    In 128K mode : check that egress N3
*         b.    In 64K mode :
*         i.    when Uplink is U1,U2
*         1.    check that flooding
*         ii.    else
*         1.    check that egress N3
*         i.    delete the entry from port group 2
*         j.    Refresh FDB to check that delete is ok.
*         k.    send traffic from all uplinks to this mac address :
*         a.    In 128K mode : check that egress N4
*         b.    In 64K mode :
*         i.    when Uplink is U1,U2
*         1.    check that flooding
*         ii.    else
*         1.    check that egress N4
*         l.    delete the entry from port group 3
*         m.    Refresh FDB to check that delete is ok.
*         n.    send multi-destination traffic from all uplinks to this mac address , check flooding
*/
GT_VOID tgfBridgeFdbAddDeleteEntryRun_ForMultiCoreFdbLookup
(
    GT_VOID
);


/**
* @internal tgfBridgeFdbAddDeleteEntryRestore_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID tgfBridgeFdbAddDeleteEntryRestore_ForMultiCoreFdbLookup
(
    GT_VOID
);

/**
* @internal tgfBridgeFdbAgingAppDemoRun_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   4.2    Test 4: check aging of mac addresses - AppDemo behavior
*         (Covered API: FDB :
*         1.    cpssDxChBrgFdbPortGroupMacEntrySet
*         a.    cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet
*         2.    cpssDxChBrgFdbPortGroupMacEntryDelete
*         b.    cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet
*         3.    cpssDxChBrgFdbPortGroupMacEntryWrite
*         4.    cpssDxChBrgFdbPortGroupMacEntryRead
*         )
*         1.    check aging of mac addresses.
*         a.    Set aging to 'trigger mode'
*         b.    Set aging timeout to 10 seconds (2 loops are 20 seconds)
*         c.    Flush FDB (trigger action 'delete') ' done on all port groups
*         d.    Add (by message /index) FDB entry on port N1 to all 4 port groups
*         i.    BUT on port group 0 (where N1 is 'local port') , set as 'dynamic' (not 'static')
*         ii.    On ALL other port groups , set as 'static' !!!
*         e.    Add another 3 FDB entries to each port group that associated with ports N2,N3,N4 , with same logic as done from N1 :
*         iii.    on port N2 to all 4 port groups
*         1.    BUT on port group 1 (where N2 is 'local port') , set as 'dynamic' (not 'static')
*         2.    On ALL other port groups , set as 'static' !!!
*         iv.    on port N3 to all 4 port groups
*         3.    BUT on port group 2 (where N3 is 'local port') , set as 'dynamic' (not 'static')
*         4.    On ALL other port groups , set as 'static' !!!
*         v.    on port N4 to all 4 port groups
*         5.    BUT on port group 3 (where N4 is 'local port') , set as 'dynamic' (not 'static')
*         6.    On ALL other port groups , set as 'static' !!!
*         f.    Set age without removal
*         g.    Set aging to 'auto aging'
*         h.    Wait 22 seconds ((2 age time) + 10%)
*         i.    Check that entry deleted from all port groups !!! (even the 3 port groups that hold 'static') ' because the AppDemo delete 'aged out' entry from all port groups !
*         j.    Repeat the test with next change :
*         a.    The entries that should be set as 'static' are ignored. (Not to set as dynamic or static)
* @param[in] useStatic                - when GT_TRUE --> Run_ForMultiCoreFdbLookup test as described above.
*                                      when GT_FALSE -->
*                                      j.        Run_ForMultiCoreFdbLookup the test with next change :
*                                      a.        The entries that should be set as 'static' are ignored. (Not to set as dynamic or static)
* @param[in] forceLessThenHalfSleepTime - when GT_FALSE - do full aging wait of 22 seconds
*                                      when GT_TRUE  - do less then half sleep time of 9 seconds
*                                      check that entries not deleted.
*                                       None
*/
GT_VOID tgfBridgeFdbAgingAppDemoRun_ForMultiCoreFdbLookup
(
    IN  GT_BOOL useStatic,
    IN  GT_BOOL forceLessThenHalfSleepTime
);


/**
* @internal tgfBridgeFdbAgingAppDemoRestore_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID tgfBridgeFdbAgingAppDemoRestore_ForMultiCoreFdbLookup
(
    GT_VOID
);


/**
* @internal tgfBridgeFdbLearningFromNetworkPortsRun_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   4.3    Test 5: check learning of 128K/64K from network ports
*         (Covered API: FDB :
*         1. cpssDxChBrgFdbPortGroupMacEntrySet
*         c. cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet
*         2. cpssDxChBrgFdbPortGroupMacEntryRead
*         )
*         1. check learning of 128K/64K from network ports
*         a. Do learning from network ports (N1,N2,N3,N4).
*         a. In 128K mode --> 32K from each port
*         b. In 64K mode --> 16K from each port
*         b. Check that ALL uplinks see 128K/64K addresses. (send traffic to those learned macs)
*         c. Check FDB capacity (FDB should be full on all port groups)
* @param[in] learnByTraffic           - learning by traffic or by 'set entry'
*                                      GT_TRUE - by traffic
*                                      GT_FALSE - by 'set entry'
*                                       None
*/
GT_VOID tgfBridgeFdbLearningFromNetworkPortsRun_ForMultiCoreFdbLookup
(
    IN GT_BOOL  learnByTraffic
);

/**
* @internal tgfBridgeFdbLearningFromNetworkPortsRestore_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID tgfBridgeFdbLearningFromNetworkPortsRestore_ForMultiCoreFdbLookup
(
    GT_VOID
);

/**
* @internal tgfBridgeFdbLearningFromNetworkTrunksRun_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   4.4    Test 6: check learning of 128K/64K from trunk ports
*         (Covered API: FDB :
*         1. cpssDxChBrgFdbPortGroupMacEntrySet
*         d. cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet
*         2. cpssDxChBrgFdbPortGroupMacEntryRead
*         )
*         1. check learning of 128K/64K from trunk ports
*         a. Add N1,N3 to trunk Tr1 and N2,N4 to trunk Tr2.
*         b. Do learning from those ports (N1,N2,N3,N4).
*         a. In 128K mode --> 32K from each port
*         b. In 64K mode --> 16K from each port
*         c. Check that ALL uplinks see 128K/64K addresses. (send traffic to those learned macs)
*         d. Check FDB capacity (FDB should be full on all port groups)
* @param[in] learnByTraffic           - learning by traffic or by 'set entry'
*                                      GT_TRUE - by traffic
*                                      GT_FALSE - by 'set entry'
*                                      learnMixedOnTrunkPorts - do we learn addresses from single port in trunk
*                                       None
*/
GT_VOID tgfBridgeFdbLearningFromNetworkTrunksRun_ForMultiCoreFdbLookup
(
    IN GT_BOOL  learnByTraffic,
    IN GT_BOOL  learnFromSinglePortInTrunk
);

/**
* @internal tgfBridgeFdbLearningFromNetworkTrunksRestore_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID tgfBridgeFdbLearningFromNetworkTrunksRestore_ForMultiCoreFdbLookup
(
    GT_VOID
);

/**
* @internal tgfBridgeFdbStationMovementRun_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   4.5    Test 7: check station movement
*         (Covered API: FDB :
*         1. cpssDxChBrgFdbPortGroupMacEntrySet
*         e. cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet
*         2. cpssDxChBrgFdbPortGroupMacEntryRead
*         )
*         1. check station movement (port to port)
*         a. learn mac A from port N1 in port group 0
*         b. In 128K mode:
*         a. check that FDB of port group 0 has it , other FDBs don't
*         c. In 64 K mode:
*         a. check that FDB of port group 0,2 has it , other FDBs don't
*         d. learn same mac A from port N2 in port group 1
*         e. In 128K mode:
*         a. check that FDB of port group 1 has it , other FDBs don't
*         f. In 64 K mode:
*         a. check that FDB of port group 1,3 has it , other FDBs don't
*         g. learn same mac A from port N3 in port group 2
*         h. In 128K mode:
*         a. check that FDB of port group 2 has it , other FDBs don't
*         i. In 64 K mode:
*         a. check that FDB of port group 0,2 has it , other FDBs don't
*         j. learn same mac A from port N4 in port group 3
*         k. In 128K mode:
*         a. check that FDB of port group 3 has it , other FDBs don't
*         l. In 64 K mode:
*         a. check that FDB of port group 1,3 has it , other FDBs don't
*         2. check address movement on trunk ports (in same trunk)
*         a. Add the network ports to trunk.
*         b. learn mac A from port N1 in port group 0
*         c. In 128K mode:
*         a. check that FDB of port group 0 has it , other FDBs don't
*         d. In 64 K mode:
*         a. check that FDB of port group 0,2 has it , other FDBs don't
*         e. learn same mac A from port N2 in port group 1
*         f. In 128K mode:
*         a. check that FDB of port group 1 has it , other FDBs don't
*         g. In 64 K mode:
*         a. check that FDB of port group 1,3 has it , other FDBs don't
*         h. learn same mac A from port N3 in port group 2
*         i. In 128K mode:
*         a. check that FDB of port group 2 has it , other FDBs don't
*         j. In 64 K mode:
*         a. check that FDB of port group 0,2 has it , other FDBs don't
*         k. learn same mac A from port N4 in port group 3
*         l. In 128K mode:
*         a. check that FDB of port group 3 has it , other FDBs don't
*         m. In 64 K mode:
*         a. check that FDB of port group 1,3 has it , other FDBs don't
*         3. check station movement from trunk to trunk
*         a. set N1,N3 in trunk A
*         b. set N2,N4 in trunk B
*         c. learn mac A from port N1 in port group 0
*         d. In 128K mode:
*         a. check that FDB of port group 0 has it , other FDBs don't' on Tr_A
*         e. In 64 K mode:
*         a. check that FDB of port group 0,2 has it , other FDBs don't' on Tr_A
*         f. learn same mac A from port N2 in port group 1
*         g. In 128K mode:
*         a. check that FDB of port group 1 has it , other FDBs don't' on Tr_B
*         h. In 64 K mode:
*         a. check that FDB of port group 1,3 has it , other FDBs don't' on Tr_B
*         i. learn same mac A from port N3 in port group 2
*         j. In 128K mode:
*         a. check that FDB of port group 2 has it , other FDBs don't' on Tr_A
*         k. In 64 K mode:
*         a. check that FDB of port group 0,2 has it , other FDBs don't' on Tr_A
*         l. learn same mac A from port N4 in port group 3
*         m. In 128K mode:
*         a. check that FDB of port group 3 has it , other FDBs don't' on Tr_B
*         n. In 64 K mode:
*         a. check that FDB of port group 1,3 has it , other FDBs don't' on Tr_B
*         4. check station movement from trunk to port , and port to trunk
*         a. set N1,N3 in trunk A
*         b. learn mac A from port N1 in port group 0
*         c. In 128K mode:
*         a. check that FDB of port group 0 has it , other FDBs don't' on Tr_A
*         d. In 64 K mode:
*         a. check that FDB of port group 0,2 has it , other FDBs don't' on Tr_A
*         e. learn same mac A from port N2 in port group 1
*         f. In 128K mode:
*         a. check that FDB of port group 1 has it , other FDBs don't' on port N2
*         g. In 64 K mode:
*         a. check that FDB of port group 1,3 has it , other FDBs don't' on port N2
*         h. learn same mac A from port N3 in port group 2
*         i. In 128K mode:
*         a. check that FDB of port group 2 has it , other FDBs don't' on Tr_A
*         j. In 64 K mode:
*         a. check that FDB of port group 0,2 has it , other FDBs don't' on Tr_A
*         k. learn same mac A from port N4 in port group 3
*         l. In 128K mode:
*         a. check that FDB of port group 3 has it , other FDBs don't' on port N4
*         m. In 64 K mode:
*         a. check that FDB of port group 1,3 has it , other FDBs don't' on port N4
*/
GT_VOID tgfBridgeFdbStationMovementRun_ForMultiCoreFdbLookup
(
    IN PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_ENT     mode
);

/**
* @internal tgfBridgeFdbStationMovementRestore_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID tgfBridgeFdbStationMovementRestore_ForMultiCoreFdbLookup
(
    GT_VOID
);


/**
* @internal tgfBridgeFdbAgeBitRun_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   4.7    Test 9: check FDB mac entry's 'Age bit' on specific port group
*         (Covered API: FDB :
*         1. cpssDxChBrgFdbPortGroupMacEntrySet
*         a. cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet
*         2. cpssDxChBrgFdbPortGroupMacEntryAgeBitSet
*         )
*         1. check FDB mac entry's 'Age bit' on specific port group
*         a. Add (by message /index) FDB entry (not static) on port N1 to all 4 port groups
*         b. set age bit to GT_FALSE on port groups 1,2,3 (not 0)
*         c. read the FDB and check that the entry on port group 0 with GT_TRUE , and others with GT_FALSE
*         d. trigger aging loop by HW
*         e. read the FDB and check that the entry on ALL port groups with 'Age bit' = 'GT_FALSE
*         f. trigger aging loop by HW
*         g. read the FDB and check that the entry removed from all port groups
*         h. ...
*         i. Add (by message /index) FDB entry (not static) on port N1 to all 4 port groups
*         j. set age bit to GT_FALSE on port groups 0,1,3 (not 2)
*         k. read the FDB and check that the entry on port group 2 with 'Age bit' = GT_TRUE , and others with 'Age bit' = GT_FALSE
*         l. trigger aging loop by HW
*         m. read the FDB and check that the entry removed from all port groups
*         i. because the port N1 associated with port group 0 , and the age bit was GT_FALSE prior to the trigger aging , the AA was sent to the AppDemo , that in turn deleted it from all the port groups.
*/
GT_VOID tgfBridgeFdbAgeBitRun_ForMultiCoreFdbLookup
(
    GT_VOID
);

/**
* @internal tgfBridgeFdbAgeBitRestore_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID tgfBridgeFdbAgeBitRestore_ForMultiCoreFdbLookup
(
    GT_VOID
);


/**
* @internal tgfBridgeFdbValidAndSkipBitsRun_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   4.8    Test 10: check FDB mac entry Get valid bit , skip bit on specific port group
*         (Covered API: FDB :
*         1. cpssDxChBrgFdbPortGroupMacEntrySet
*         a. cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet
*         2. cpssDxChBrgFdbPortGroupMacEntryStatusGet
*         )
*         1. check FDB mac entry Get valid bit , skip bit on specific port group
*         a. Add by (message / by index) FDB entry on port N1 to all port groups
*         b. Check status of entry on each port group : cpssDxChBrgFdbPortGroupMacEntryStatusGet , should be:
*         i. Valid = GT_TRUE
*         ii. Skip = GT_FALSE
*         c. Delete by message the entry from port group 0
*         d. Check status of entry on port group 0: cpssDxChBrgFdbPortGroupMacEntryStatusGet , should be:
*         i. Valid = GT_TRUE
*         ii. Skip = GT_TRUE
*         e. Delete by message the entry from port group 3
*         f. Check status of entry on port group 3: cpssDxChBrgFdbPortGroupMacEntryStatusGet , should be:
*         i. Valid = GT_TRUE
*         ii. Skip = GT_TRUE
*         g. Delete by index the entry from port group 2
*         h. Check status of entry on port group 2: cpssDxChBrgFdbPortGroupMacEntryStatusGet , should be:
*         i. Valid = GT_FALSE
*         ii. Skip = GT_FALSE
*         i. Check status of entry on port group 1 (no change done on it): cpssDxChBrgFdbPortGroupMacEntryStatusGet , should be:
*         i. Valid = GT_TRUE
*         ii. Skip = GT_FALSE
*/
GT_VOID tgfBridgeFdbValidAndSkipBitsRun_ForMultiCoreFdbLookup
(
    GT_VOID
);

/**
* @internal tgfBridgeFdbValidAndSkipBitsRestore_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID tgfBridgeFdbValidAndSkipBitsRestore_ForMultiCoreFdbLookup
(
    GT_VOID
);

/**
* @internal tgfBridgeFdbDebugBurstNumSet function
* @endinternal
*
* @brief   debug function to run test of learn 128K/64K with smaller burst -->
*         less number of entries to the FDB.
*         to to allow fast debug of error in test and not wait till end of 128K/64K
*         learn...
*/
GT_STATUS tgfBridgeFdbDebugBurstNumSet
(
    IN GT_U32   burstNum
);

/**
* @internal tgfBridgeFdbDeviceMapTableModeSet_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   Set lookup mode for accessing the Device Map table.
*         and call to adjust the device map table.
* @param[in] mode                     - device Map lookup mode
*                                       none
*/
GT_VOID tgfBridgeFdbDeviceMapTableModeSet_ForMultiCoreFdbLookup
(
    IN PRV_TGF_DEV_MAP_LOOKUP_MODE_ENT   mode
);

/**
* @internal tgfBridgeFdbDeviceMapTableModeRestore_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   Restore_ForMultiCoreFdbLookup test configuration
*/
GT_VOID tgfBridgeFdbDeviceMapTableModeRestore_ForMultiCoreFdbLookup
(
    GT_VOID
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBridgeFdb_ForMultiCoreFdbLookupUT */



