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
* @file tgfCommonBridge_ForMultiCoreFdbLookupUT.c
*
* @brief Enhanced UTs for Bridge For Multi Core Fdb Lookup
*
* tests according to design doc in documentum :
* http://docil.marvell.com/webtop/drl/objectId/0900dd88800a0077
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <bridge/prvTgfBridgeFdb_ForMultiCoreFdbLookupUT.h>

#define INDEX_0_CNS     0
#define INDEX_1_CNS     1
#define INDEX_4_CNS     4

typedef void (*TEST_FUNC)(void);

/* the learn by traffic takes too long , so usually test it 'by message'*/
static GT_BOOL learnByTraffic = GT_FALSE;

/* run same test twice :
first time with 'by message' FDB access
second time with 'by index' FDB accesss
*/
static void testByIndexAndByMessage(
    IN TEST_FUNC testFunc
)
{
    PRV_TGF_BRG_FDB_ACCESS_MODE_ENT oldMode;

    /* state that the FDB accessing is 'by message' */
    oldMode = prvTgfBrgFdbAccessModeSet(PRV_TGF_BRG_FDB_ACCESS_MODE_BY_MESSAGE_E);

    utfGeneralStateMessageSave(INDEX_0_CNS,"FdbAccessMode=[%s]","PRV_TGF_BRG_FDB_ACCESS_MODE_BY_MESSAGE_E");

    testFunc();

    /* state that the FDB accessing is 'by index' */
    prvTgfBrgFdbAccessModeSet(PRV_TGF_BRG_FDB_ACCESS_MODE_BY_INDEX_E);

    utfGeneralStateMessageSave(INDEX_0_CNS,"FdbAccessMode=[%s]","PRV_TGF_BRG_FDB_ACCESS_MODE_BY_INDEX_E");

    testFunc();

    /* Restore the FDB accessing mode */
    prvTgfBrgFdbAccessModeSet(oldMode);
}

/*----------------------------------------------------------------------------*/
/*
4.1        Test 3: check add/delete (by index/message) of FDB entry
*/
static void tgfBridgeFdbAddDeleteEntry_ForMultiCoreFdbLookup(void)
{
/*
    1. Run_ForMultiCoreFdbLookup test
    2. Restore_ForMultiCoreFdbLookup configuration
*/

    /* do init for multi port groups */
    prvTgfMultiPortGroupInit_ForMultiCoreFdbLookup();

    /* Run_ForMultiCoreFdbLookup test */
    tgfBridgeFdbAddDeleteEntryRun_ForMultiCoreFdbLookup();

    /* Restore_ForMultiCoreFdbLookup configuration */
    tgfBridgeFdbAddDeleteEntryRestore_ForMultiCoreFdbLookup();

    /* restore port configuration according to device family */
    prvTgfPortsArrayByDevTypeSet();
}

/*----------------------------------------------------------------------------*/
/*
4.1        Test 3: check add/delete (by index/message) of FDB entry
*/
UTF_TEST_CASE_MAC(tgfBridgeFdbAddDeleteEntry_ForMultiCoreFdbLookup)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);
    testByIndexAndByMessage(tgfBridgeFdbAddDeleteEntry_ForMultiCoreFdbLookup);
}

/*----------------------------------------------------------------------------*/
/*
4.2        Test 4: check aging of mac addresses -- AppDemo behavior
*/
static void tgfBridgeFdbAgingAppDemo_ForMultiCoreFdbLookup(void)
{
/*
    1. Run_ForMultiCoreFdbLookup test
    2. Restore_ForMultiCoreFdbLookup configuration
*/
    GT_BOOL useStatic[2] = {GT_TRUE,GT_FALSE};
    GT_BOOL forceLessThenHalfSleepTime[2] = {GT_TRUE,GT_FALSE};
    GT_U32  ii,jj;

/*
NOTE:
    this test takes (at least) : 2*22 + 2*9 = 62 seconds
    explanation  :
        when forceLessThenHalfSleepTime[jj] = GT_TRUE    test takes(at least)  9 seconds
        when forceLessThenHalfSleepTime[jj] = GT_FALSE   test takes(at least) 22 seconds
*/


    /* do init for multi port groups */
    prvTgfMultiPortGroupInit_ForMultiCoreFdbLookup();

    for(ii = 0 ; ii < 2; ii++)
    {
        for(jj = 0 ; jj < 2; jj++)
        {
            /* Run_ForMultiCoreFdbLookup test */
            tgfBridgeFdbAgingAppDemoRun_ForMultiCoreFdbLookup(useStatic[ii] , forceLessThenHalfSleepTime[jj]);

            /* Restore_ForMultiCoreFdbLookup configuration */
            tgfBridgeFdbAgingAppDemoRestore_ForMultiCoreFdbLookup();
        }
    }

    /* restore port configuration according to device family */
    prvTgfPortsArrayByDevTypeSet();
}

/*----------------------------------------------------------------------------*/
/*
4.2        Test 4: check aging of mac addresses -- AppDemo behavior
*/
UTF_TEST_CASE_MAC(tgfBridgeFdbAgingAppDemo_ForMultiCoreFdbLookup)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);
    testByIndexAndByMessage(tgfBridgeFdbAgingAppDemo_ForMultiCoreFdbLookup);
}

/*----------------------------------------------------------------------------*/
/*
4.3        Test 5: check learning of 128K/64K from network ports
*/
UTF_TEST_CASE_MAC(tgfBridgeFdbLearningFromNetworkPorts_ForMultiCoreFdbLookup)
{
/*
    1. Run_ForMultiCoreFdbLookup test
    2. Restore_ForMultiCoreFdbLookup configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* do init for multi port groups */
    prvTgfMultiPortGroupInit_ForMultiCoreFdbLookup();

    /* Run_ForMultiCoreFdbLookup test */
    tgfBridgeFdbLearningFromNetworkPortsRun_ForMultiCoreFdbLookup(learnByTraffic);

    /* Restore_ForMultiCoreFdbLookup configuration */
    tgfBridgeFdbLearningFromNetworkPortsRestore_ForMultiCoreFdbLookup();

    /* restore port configuration according to device family */
    prvTgfPortsArrayByDevTypeSet();
}

/*----------------------------------------------------------------------------*/
/*
4.4        Test 6: check learning of 128K/64K from trunk ports
*/
UTF_TEST_CASE_MAC(tgfBridgeFdbLearningFromNetworkTrunks_ForMultiCoreFdbLookup)
{
/*
    1. Run_ForMultiCoreFdbLookup test
    2. Restore_ForMultiCoreFdbLookup configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* do init for multi port groups */
    prvTgfMultiPortGroupInit_ForMultiCoreFdbLookup();

    /* Run_ForMultiCoreFdbLookup test - learn from 2 ports in each trunk  */
    tgfBridgeFdbLearningFromNetworkTrunksRun_ForMultiCoreFdbLookup(learnByTraffic,GT_FALSE);

    /* Restore_ForMultiCoreFdbLookup configuration */
    tgfBridgeFdbLearningFromNetworkTrunksRestore_ForMultiCoreFdbLookup();

    /* Run_ForMultiCoreFdbLookup test - learn from single port in each trunk */
    tgfBridgeFdbLearningFromNetworkTrunksRun_ForMultiCoreFdbLookup(learnByTraffic,GT_TRUE);

    /* Restore_ForMultiCoreFdbLookup configuration */
    tgfBridgeFdbLearningFromNetworkTrunksRestore_ForMultiCoreFdbLookup();

    /* restore port configuration according to device family */
    prvTgfPortsArrayByDevTypeSet();

}

/*----------------------------------------------------------------------------*/
/*
4.5        Test 7: check station movement
*/
UTF_TEST_CASE_MAC(tgfBridgeFdbStationMovement_ForMultiCoreFdbLookup)
{
/*
    1. Run_ForMultiCoreFdbLookup test
    2. Restore_ForMultiCoreFdbLookup configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* do init for multi port groups */
    prvTgfMultiPortGroupInit_ForMultiCoreFdbLookup();

    /* Run_ForMultiCoreFdbLookup test - station movement : port to port */
    tgfBridgeFdbStationMovementRun_ForMultiCoreFdbLookup(
        PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_PORT_TO_PORT_E);

    /* Restore_ForMultiCoreFdbLookup configuration */
    tgfBridgeFdbStationMovementRestore_ForMultiCoreFdbLookup();

    /* Run_ForMultiCoreFdbLookup test - station movement : port to trunk ,
        NOTE: this also test 'trunk to port' :
        PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_trunk_TO_PORT_E
    */
    tgfBridgeFdbStationMovementRun_ForMultiCoreFdbLookup(
        PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_PORT_TO_TRUNK_E);

    /* Restore_ForMultiCoreFdbLookup configuration */
    tgfBridgeFdbStationMovementRestore_ForMultiCoreFdbLookup();

    /* Run_ForMultiCoreFdbLookup test - station movement : trunk to trunk */
    tgfBridgeFdbStationMovementRun_ForMultiCoreFdbLookup(
        PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_TRUNK_TO_TRUNK_E);

    /* Run_ForMultiCoreFdbLookup test - station movement : in the trunk */
    tgfBridgeFdbStationMovementRun_ForMultiCoreFdbLookup(
        PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_IN_THE_TRUNK_E);

    /* Restore_ForMultiCoreFdbLookup configuration */
    tgfBridgeFdbStationMovementRestore_ForMultiCoreFdbLookup();

    /* restore port configuration according to device family */
    prvTgfPortsArrayByDevTypeSet();
}

/*----------------------------------------------------------------------------*/
/*
4.6        Test 8: check QA (query address message) send to specific port group
(Covered API: FDB :
    1. cpssDxChBrgFdbPortGroupQaSend
)
    1. check QA (query address message) send to specific port group
        a. Take test 7 (check station movement) ,and after every time that test
            change the port group of the FDB entry send 4 QA messages
            (one for each port group) and check that the response from the
            'real' port group state 'found' and other port groups state 'no found'
            i. In 128K mode : one port group should replay 'found' other 3 'not found'
            ii. In 64K mode : 2 port group should replay 'found' other 2 'not found'
*/
UTF_TEST_CASE_MAC(tgfBridgeFdbQaSend_ForMultiCoreFdbLookup)
{
/*
    1. Run_ForMultiCoreFdbLookup test
    2. Restore_ForMultiCoreFdbLookup configuration
*/
    PRV_TGF_BRG_FDB_ACCESS_MODE_ENT oldMode;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* do init for multi port groups */
    prvTgfMultiPortGroupInit_ForMultiCoreFdbLookup();

    /* state that the FDB accessing is 'by message' */
    oldMode = prvTgfBrgFdbAccessModeSet(PRV_TGF_BRG_FDB_ACCESS_MODE_BY_MESSAGE_E);

    /* call other test (of station movement) */
    UTF_TEST_CALL_MAC(tgfBridgeFdbStationMovement_ForMultiCoreFdbLookup);

    /* Restore the FDB accessing mode */
    prvTgfBrgFdbAccessModeSet(oldMode);

    /* restore port configuration according to device family */
    prvTgfPortsArrayByDevTypeSet();
}

/*----------------------------------------------------------------------------*/
/*
4.7        Test 9: check FDB mac entry's 'Age bit' on specific port group
*/
static void tgfBridgeFdbAgeBit_ForMultiCoreFdbLookup(void)
{
/*
    1. Run_ForMultiCoreFdbLookup test
    2. Restore_ForMultiCoreFdbLookup configuration
*/

    /* do init for multi port groups */
    prvTgfMultiPortGroupInit_ForMultiCoreFdbLookup();

    /* Run_ForMultiCoreFdbLookup test */
    tgfBridgeFdbAgeBitRun_ForMultiCoreFdbLookup();

    /* Restore_ForMultiCoreFdbLookup configuration */
    tgfBridgeFdbAgeBitRestore_ForMultiCoreFdbLookup();

    /* restore port configuration according to device family */
    prvTgfPortsArrayByDevTypeSet();
}

/*----------------------------------------------------------------------------*/
/*
4.7        Test 9: check FDB mac entry's 'Age bit' on specific port group
*/
UTF_TEST_CASE_MAC(tgfBridgeFdbAgeBit_ForMultiCoreFdbLookup)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    testByIndexAndByMessage(tgfBridgeFdbAgeBit_ForMultiCoreFdbLookup);
}

/*----------------------------------------------------------------------------*/
/*
4.8        Test 10: check FDB mac entry Get valid bit , skip bit on specific port group
*/
static void tgfBridgeFdbValidAndSkipBits_ForMultiCoreFdbLookup(void)
{
/*
    1. Run_ForMultiCoreFdbLookup test
    2. Restore_ForMultiCoreFdbLookup configuration
*/

    /* do init for multi port groups */
    prvTgfMultiPortGroupInit_ForMultiCoreFdbLookup();

    /* Run_ForMultiCoreFdbLookup test */
    tgfBridgeFdbValidAndSkipBitsRun_ForMultiCoreFdbLookup();

    /* Restore_ForMultiCoreFdbLookup configuration */
    tgfBridgeFdbValidAndSkipBitsRestore_ForMultiCoreFdbLookup();

    /* restore port configuration according to device family */
    prvTgfPortsArrayByDevTypeSet();
}

/*----------------------------------------------------------------------------*/
/*
4.8        Test 10: check FDB mac entry Get valid bit , skip bit on specific port group
*/
UTF_TEST_CASE_MAC(tgfBridgeFdbValidAndSkipBits_ForMultiCoreFdbLookup)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    testByIndexAndByMessage(tgfBridgeFdbValidAndSkipBits_ForMultiCoreFdbLookup);
}

/*----------------------------------------------------------------------------*/
/*
7.1        Test 17 : Device map table modes
*/
UTF_TEST_CASE_MAC(tgfBridgeFdbdeviceMapTableModes_ForMultiCoreFdbLookup)
{
    PRV_TGF_DEV_MAP_LOOKUP_MODE_ENT mode;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    for(mode = PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_E ;
        mode <= PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E;
        mode ++)
    {
        utfGeneralStateMessageSave(INDEX_4_CNS,"device map table mode=[%s]",
            (mode == PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_E) ?
                "PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_E" :
            (mode == PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E) ?
                "PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E" :
                " unknown ");
        if(mode == PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_E)
        {
            /* skip this mode since it was tested as part of the 'default' run */
            continue;
        }

        tgfBridgeFdbDeviceMapTableModeSet_ForMultiCoreFdbLookup(mode);

        utfGeneralStateMessageSave(INDEX_1_CNS,"tgfBridgeFdbAddDeleteEntry_ForMultiCoreFdbLookup");
        UTF_TEST_CALL_MAC(tgfBridgeFdbAddDeleteEntry_ForMultiCoreFdbLookup);

        utfGeneralStateMessageSave(INDEX_1_CNS,"tgfBridgeFdbAgingAppDemo_ForMultiCoreFdbLookup");
        UTF_TEST_CALL_MAC(tgfBridgeFdbAgingAppDemo_ForMultiCoreFdbLookup);

        tgfBridgeFdbDebugBurstNumSet(1000);/* make next 2 tests shorter */
        utfGeneralStateMessageSave(INDEX_1_CNS,"tgfBridgeFdbLearningFromNetworkPorts_ForMultiCoreFdbLookup");
        UTF_TEST_CALL_MAC(tgfBridgeFdbLearningFromNetworkPorts_ForMultiCoreFdbLookup);

        utfGeneralStateMessageSave(INDEX_1_CNS,"tgfBridgeFdbLearningFromNetworkTrunks_ForMultiCoreFdbLookup");
        UTF_TEST_CALL_MAC(tgfBridgeFdbLearningFromNetworkTrunks_ForMultiCoreFdbLookup);
        tgfBridgeFdbDebugBurstNumSet(0);

        utfGeneralStateMessageSave(INDEX_1_CNS,"tgfBridgeFdbStationMovement_ForMultiCoreFdbLookup");
        UTF_TEST_CALL_MAC(tgfBridgeFdbStationMovement_ForMultiCoreFdbLookup);

        utfGeneralStateMessageSave(INDEX_1_CNS,"tgfBridgeFdbQaSend_ForMultiCoreFdbLookup");
        UTF_TEST_CALL_MAC(tgfBridgeFdbQaSend_ForMultiCoreFdbLookup);

        utfGeneralStateMessageSave(INDEX_1_CNS,"tgfBridgeFdbAgeBit_ForMultiCoreFdbLookup");
        UTF_TEST_CALL_MAC(tgfBridgeFdbAgeBit_ForMultiCoreFdbLookup);

        utfGeneralStateMessageSave(INDEX_1_CNS,"tgfBridgeFdbValidAndSkipBits_ForMultiCoreFdbLookup");
        UTF_TEST_CALL_MAC(tgfBridgeFdbValidAndSkipBits_ForMultiCoreFdbLookup);
    }

    /* restore configuration */
    tgfBridgeFdbDeviceMapTableModeRestore_ForMultiCoreFdbLookup();

    utfGeneralStateMessageSave(INDEX_4_CNS,NULL);
    utfGeneralStateMessageSave(INDEX_1_CNS,NULL);
}
/*
 * Configuration of tgfBridge_ForMultiCoreFdbLookup suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfBridge_ForMultiCoreFdbLookup)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBridgeFdbAddDeleteEntry_ForMultiCoreFdbLookup)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBridgeFdbAgingAppDemo_ForMultiCoreFdbLookup)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBridgeFdbLearningFromNetworkPorts_ForMultiCoreFdbLookup)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBridgeFdbLearningFromNetworkTrunks_ForMultiCoreFdbLookup)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBridgeFdbStationMovement_ForMultiCoreFdbLookup)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBridgeFdbQaSend_ForMultiCoreFdbLookup)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBridgeFdbAgeBit_ForMultiCoreFdbLookup)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBridgeFdbValidAndSkipBits_ForMultiCoreFdbLookup)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBridgeFdbdeviceMapTableModes_ForMultiCoreFdbLookup)
UTF_SUIT_END_TESTS_MAC(tgfBridge_ForMultiCoreFdbLookup)



