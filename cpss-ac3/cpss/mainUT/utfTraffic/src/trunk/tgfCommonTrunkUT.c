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
* @file tgfCommonTrunkUT.c
*
* @brief Enhanced UTs for CPSS trunk
*
* @version   29
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <trunk/prvTgfTrunk.h>
#include <common/tgfTrunkGen.h>
#include <common/tgfCommon.h>
#include <common/tgfCscdGen.h>

/* sorting mode */
static GT_BOOL  sortMode = GT_FALSE;

#define DECLARE_START_OF_TEST_MAC       \
    sortMode = GT_FALSE;                \
    cpssOsPrintf("[%s] running with NO sorted trunks \n",utfTestNameGet());\
    restartTest_lbl:/* must be after sortMode = GT_FALSE;*/

/* macro that will redo the test with sorting mode enabled (only for supporting devices)*/
#define REDO_TEST_WITH_SORTING_MAC                                                          \
    /* no need to run second case when reducing GM tests */                                 \
    if(prvUtfIsGmCompilation() == GT_TRUE)                                                  \
    {                                                                                       \
        return;                                                                             \
    }                                                                                       \
    /* no need to run second case when reducing long tests */                               \
    if(prvUtfSkipLongTestsFlagGet(UTF_ALL_FAMILY_E) != GT_FALSE)                            \
    {                                                                                       \
        return;                                                                             \
    }                                                                                       \
    /*enable sorting and then re-trigger the common section */                              \
    if(GT_NOT_SUPPORTED == prvTgfTrunkDbMembersSortingEnableSet(prvTgfDevNum, GT_TRUE))     \
    {                                                                                       \
        cpssOsPrintf("[%s] running with sorted trunks - not supported \n",utfTestNameGet());\
        /* the device not trunk support sorting of the trunk members */                     \
        return;                                                                             \
    }                                                                                       \
                                                                                            \
    if(sortMode == GT_FALSE)                                                                \
    {                                                                                       \
        sortMode = GT_TRUE;                                                                 \
        cpssOsPrintf("[%s] running with sorted trunks - started \n",utfTestNameGet());      \
        goto restartTest_lbl;                                                               \
    }                                                                                       \
                                                                                            \
    cpssOsPrintf("[%s] running with sorted trunks - ended \n",utfTestNameGet());            \
    /* restore defaults */                                                                  \
    prvTgfTrunkDbMembersSortingEnableSet(prvTgfDevNum, GT_FALSE)


/*----------------------------------------------------------------------------*/
/*
    tgfTrunkHashMplsModeEnableSet
*/
UTF_TEST_CASE_MAC(tgfTrunkHashMplsModeEnableSet)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    /* Set configuration */
    prvTgfTrunkForHashTestConfigure();

    /* Generate traffic */
    prvTgfTrunkHashTestTrafficMplsModeEnableSet(GT_FALSE);

    /* Restore configuration */
    prvTgfTrunkForHashTestConfigurationReset();
}

/*----------------------------------------------------------------------------*/
/*
    tgfTrunkHashL3L4OverMplsParseEnable
*/
UTF_TEST_CASE_MAC(tgfTrunkHashL3L4OverMplsParseEnable)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* test is not ready for GM */
    GM_SKIP_TEST_DUE_TO_POOR_PERFORMANCE_MAC;/* the BC2 GM not support the L3L4OverMplsParseEnable anyway */

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfTrunkForHashTestConfigure();

    /* Generate traffic */
    prvTgfTrunkHashTestTrafficMplsModeEnableSet(GT_TRUE);

    /* Restore configuration */
    prvTgfTrunkForHashTestConfigurationReset();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test trunk hash mask:
    configure TRUNK with 2 ports;
    configure general trunk hashing mode;
    mask\unmask SA field for trunk hash calc and
    send Ethernet traffic and verify expected traffic on trunk ports.
*/
UTF_TEST_CASE_MAC(tgfTrunkHashMaskSet)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    /* Set configuration */
    prvTgfTrunkForHashTestConfigure();

    /* Generate traffic */
    prvTgfTrunkHashTestTrafficMaskSet();

    /* Restore configuration */
    prvTgfTrunkForHashTestConfigurationReset();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test trunk hash IP shift:
    configure TRUNK with 2 ports;
    configure general trunk hashing mode;
    configure 2 shift values to IPv4 src addresses;
    send IPv4 traffic and verify expected traffic on trunk ports.
*/
UTF_TEST_CASE_MAC(tgfTrunkHashIpShiftSet)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    /* Set configuration */
    prvTgfTrunkForHashTestConfigure();

    /* Generate traffic */
    prvTgfTrunkHashTestTrafficIpShiftSet();

    /* Restore configuration */
    prvTgfTrunkForHashTestConfigurationReset();
}

/*----------------------------------------------------------------------------*/
/*
    tgfTrunkDesignatedPorts
*/
UTF_TEST_CASE_MAC(tgfTrunkDesignatedPorts)
{
    /* must be first line */
    DECLARE_START_OF_TEST_MAC;

    /* the Puma APIs not support the 'Designated member' */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_DXCH_E);

    /* for GM : the test will reduce number of iterations to 1 */

    /* run test */
    prvTgfTrunkDesignatedPorts();

    /* call to redo the test for sorting mode enabled (only for supporting devices) */
    REDO_TEST_WITH_SORTING_MAC;

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test trunk hash IP shift:
    configure VLAN, FDB entries;
    configure TRUNK with 4 ports in non-sorted order;
    send unknown UC traffic from each port;
    verify no Tx traffic on ports and verify traffic on CPU.
*/
UTF_TEST_CASE_MAC(tgfTrunkToCpu)
{
    /* must be first line */
    DECLARE_START_OF_TEST_MAC;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);
    /* Set configuration */
    prvTgfTrunkAndVlanTestConfigure();

    /* Generate traffic */
    prvTgfTrunkToCpuTraffic();

    /* Restore configuration */
    prvTgfTrunkAndVlanTestConfigurationReset();

    /* call to redo the test for sorting mode enabled (only for supporting devices) */
    REDO_TEST_WITH_SORTING_MAC;
}

/*----------------------------------------------------------------------------*/
/*
    tgfTrunkCrcHash
*/
UTF_TEST_CASE_MAC(tgfTrunkCrcHash)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_DXCH_E);

    /* for GM : the test will reduce number of iterations to 1 */

    /* Set configuration */
    prvTgfTrunkCrcHashTestInit(GT_FALSE);

    /* Generate traffic */
    prvTgfTrunkCrcHashTest(GT_FALSE);

    /* Restore configuration */
    prvTgfTrunkCrcHashTestRestore();
}

extern void prvTgfTrunkCrcHash_cpssPredictEgressPortEnableSet(IN GT_BOOL enable);
/*----------------------------------------------------------------------------*/
/*
    tgfTrunkCrcHash_liteMode_cpssPredictEgressPort
    run test tgfTrunkCrcHash in lite mode , send limited number of packets to
    check that the CPSS api : cpssDxChTrunkHashIndexCalculate predict properly
    the actual egress port.
*/
UTF_TEST_CASE_MAC(tgfTrunkCrcHash_liteMode_CRC32_cpssPredictEgressPort)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* NOTE: flag restored by the 'Restore' function */
    prvTgfTrunkCrcHash_cpssPredictEgressPortEnableSet(GT_TRUE);

    /* Set configuration */
    prvTgfTrunkCrcHashTestInit(GT_FALSE);

    /* Generate traffic */
    prvTgfTrunkCrcHashTest(GT_FALSE);

    /* Restore configuration */
    prvTgfTrunkCrcHashTestRestore();
}

extern void prvTgfTrunkCrcHash_symmetricModeEnable(IN GT_BOOL enable);
UTF_TEST_CASE_MAC(tgfTrunkCrcHash_liteMode_CRC32_cpssPredictEgressPort_symmetricHash)
{
    /* NOTE: flag restored by the 'Restore' function */
    prvTgfTrunkCrcHash_symmetricModeEnable(GT_TRUE);
    UTF_TEST_CALL_MAC(tgfTrunkCrcHash_liteMode_CRC32_cpssPredictEgressPort);
    prvTgfTrunkCrcHash_symmetricModeEnable(GT_FALSE);/* must be here for GM that skip the test !!!*/
}

/*----------------------------------------------------------------------------*/
/*
    tgfTrunkCrcHashL3L4OverMplsParseEnable
*/
UTF_TEST_CASE_MAC(tgfTrunkCrcHashL3L4OverMplsParseEnable)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_DXCH_E);

    /* for GM : the test will reduce number of iterations to 1 */

    /* Set configuration */
    prvTgfTrunkCrcHashTestInit(GT_TRUE);

    /* Generate traffic */
    prvTgfTrunkCrcHashTest(GT_TRUE);

    /* Restore configuration */
    prvTgfTrunkCrcHashTestRestore();
}


/*----------------------------------------------------------------------------*/
/*
    tgfTrunkCrcHashMaskPriority
*/
UTF_TEST_CASE_MAC(tgfTrunkCrcHashMaskPriority)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* test is not ready for GM */
    /* for GM : the test will reduce number of iterations to 1 */

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);



    /* Set configuration */
    prvTgfTrunkCrcHashTestInit(GT_FALSE);

    /* Generate traffic */
    prvTgfTrunkCrcHashMaskPriorityTest();

    /* Restore configuration */
    prvTgfTrunkCrcHashTestRestore();
}

/*----------------------------------------------------------------------------*/
/*
    tgfTrunkCascadeTrunk
*/
UTF_TEST_CASE_MAC(tgfTrunkCascadeTrunk)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* test is not ready for GM */

    /* for GM : the test will reduce number of iterations to 2 */

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);


    /* Set configuration */
    tgfTrunkCascadeTrunkTestInit(GT_FALSE);

    /* Generate traffic */
    tgfTrunkCascadeTrunkTest();

    /* Restore configuration */
    tgfTrunkCascadeTrunkTestRestore();
}

/*----------------------------------------------------------------------------*/
/*
    tgfTrunkDesignatedTableModes
*/
UTF_TEST_CASE_MAC(tgfTrunkDesignatedTableModes)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_PUMA_E);

    /* for GM : the test will reduce number of iterations to 1 */

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);


    /* Set configuration */
    tgfTrunkDesignatedTableModesTestInit();

    /* Generate traffic */
    tgfTrunkDesignatedTableModesTest();

    /* Restore configuration */
    tgfTrunkDesignatedTableModesTestRestore();
}

/*----------------------------------------------------------------------------*/
/*
    tgfTrunkMcLocalSwitching
*/
UTF_TEST_CASE_MAC(tgfTrunkMcLocalSwitching)
{
    /* must be first line */
    DECLARE_START_OF_TEST_MAC;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_DXCH_E | UTF_PUMA_E);

    if(CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily))
    {
        /* since the test cause 'send back to ingress port' the system is stuck
           because the packets are sent in the context of the test */
        GM_SKIP_TEST_DUE_TO_POOR_PERFORMANCE_MAC;
    }

    /* Set configuration */
    tgfTrunkMcLocalSwitchingTestInit();

    /* Generate traffic */
    tgfTrunkMcLocalSwitchingTest();

    /* Restore configuration */
    tgfTrunkMcLocalSwitchingTestRestore();

    /* call to redo the test for sorting mode enabled (only for supporting devices) */
    REDO_TEST_WITH_SORTING_MAC;
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: tgfTrunkSimpleHashMacSaDa - check Simple hash for Mac SA DA (targeting known UC to the trunk).
*
2.1.1.1        Mac DA LBH

1.        Define trunk-Id 255 with 12 members. (global eport for the 'global EPort that represents the trunk')
2.        add the FDB 64 incremental macs for associate it with the trunkID (global eport for the 'global EPort that represents the trunk')
3.        from port that is not in trunk , send 1K packets with incremental DA (constant SA) that associated with the trunk (DA = incremental mac A , mac A+1 ...)
    a.        do the same from 3 other ports that not in trunk
4.        check that each port of the trunk got ~64/<num of members> packets
    a.        check that total packets received at the trunk is 64 (no packet loss)
5.        check that ports that are not in the trunk got no packets (no flooding)
6.        If we left with 1 port in the trunk ' Stop test.
7.        Remove one port from the trunk.
8.        go to #3


2.1.1.2        Mac SA LBH

9.        Define trunk-Id 255 with 12 members. (global eport for the 'global EPort that represents the trunk')
10.
11.        from port that is not in trunk , send 64 packets with constant DA and incremental SA that associated with the trunk (DA = constant mac A)
    a.        do the same from 3 other ports that not in trunk
12.        check that each port of the trunk got ~64/<num of members> packets
    a.        check that total packets received at the trunk is 1K (no packet loss)
13.        check that ports that are not in the trunk got no packets (no flooding)
14.        If we left with 1 port in the trunk ' Stop test.
15.        Remove one port from the trunk.
16.        go to #3
*/
UTF_TEST_CASE_MAC(tgfTrunkSimpleHashMacSaDa)
{
    /* must be first line */
    DECLARE_START_OF_TEST_MAC;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    /* allow 'long' test on DXCH , as it reduce the iterations inside prvTgfTrunkSimpleHashMacSaDaTest */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_PUMA_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfTrunkSimpleHashMacSaDaTestInit();

    /* Generate traffic */
    prvTgfTrunkSimpleHashMacSaDaTest();

    /* Restore configuration */
    prvTgfTrunkSimpleHashMacSaDaTestRestore();

    /* call to redo the test for sorting mode enabled (only for supporting devices) */
    REDO_TEST_WITH_SORTING_MAC;
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: tgfTrunkSimpleHashMacSaDa_globalEPortRepresentTrunk - check Simple hash for Mac SA DA
            when the 'trunk' is represented in the ingress pipe by 'global EPort'
            this global ePort is mapped to trunkId at the end of the ingress pipe
            by the E2PHY mechanism.
            (targeting known UC to the global EPort that represents the trunk),

            NOTE : this test based on tgfTrunkSimpleHashMacSaDa.

*/
UTF_TEST_CASE_MAC(tgfTrunkSimpleHashMacSaDa_globalEPortRepresentTrunk)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    tgfTrunkGlobalEPortRepresentTrunk = 1;
    UTF_TEST_CALL_MAC(tgfTrunkSimpleHashMacSaDa);
    tgfTrunkGlobalEPortRepresentTrunk = 0;
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: tgfTrunkWithRemoteMembers

    design according to : http://docil.marvell.com/webtop/drl/objectId/0900dd88801254fd
    Purpose:
    Define enh-UT / RDE test for LBH (load balance hash) on trunk members with "remote" members.
    The test require single device

    Configuration 1:
    FDB : Set 64 mac addresses related to trunk A
    Trunk A "regular" with 8 members : 2 locals + 6 remote
    Trunk B "cascade" with 2 ports
    Trunk C "cascade" with 2 ports
    Dev 2 "Remote" reached via trunk B (3 members in trunk A) --> device map table
    Dev 3 "Remote" reached via trunk C (2 members in trunk A) --> device map table
    Dev 4 "Remote" reached via port (1 member in trunk A) --> device map table

    Configuration 2:
    Same as configuration 1 + next:
    Trunk B is "weighted cascade" the 2 ports weights are (2,6)

    Configuration 3:
    Same as configuration 1 or 2 + next:
    Case 1 : Remove one of the ports from Cascade trunk A or B
    Case 2 : Remove one of the members from Trunk A , a member of Dev 2 or Dev 3

    Configuration 4:
    After configuration 3 : restore the removed port

    Traffic generated:
    Send traffic from a port that is not relate to any trunk / remote device:
    1. known UC - 64 mac DA
    2. unknown UC - 64 mac DA (other then the known)

    Check:
    1. a. when 3 members of Dev 2 in trunk A : the ports of trunk B got 3/numMembersOf_trunkA of traffic
        b. when 2 members of Dev 2 in trunk A : the ports of trunk B got 2/numMembersOf_trunkA of traffic
        c. when "flood" trunk B get FULL traffic

    2. a. when 2 members of Dev 3 in trunk A : the ports of trunk C got 2/numMembersOf_trunkA of traffic
        b. when 1 members of Dev 3 in trunk A : the ports of trunk C got 1/numMembersOf_trunkA of traffic
        c. when "flood" trunk C get FULL traffic

    3. a. when 2 ports in trunk B : the ports share traffic according to weight (when no weight --> equal)
        b. when 1 port in trunk B : the port get all traffic
    4. Local Dev 1 Member 1 : get 1/numMembersOf_trunkA
    5. Local Dev 1 Member 5 : get 1/numMembersOf_trunkA
    6. a. Local Dev 1 Member 24 : get 1/numMembersOf_trunkA
        b. when "flood"  Local Dev 1 Member 24 get FULL traffic

    Test 1:
    1. Configuration 1 ,traffic generate ,check
    2. Configuration 3 ,traffic generate ,check
    3. Configuration 4 ,traffic generate ,check

    Test 2: (weighted cascade trunk)
    Like test 1 , use configuration 2 instead of configuration 1

*/
UTF_TEST_CASE_MAC(tgfTrunkWithRemoteMembers)
{
    /* must be first line */
    DECLARE_START_OF_TEST_MAC;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    utfGeneralStateMessageSave(0,"sortMode = %d",sortMode);

    tgfTrunkWithRemoteMembersTest();

    /* call to redo the test for sorting mode enabled (only for supporting devices) */
    REDO_TEST_WITH_SORTING_MAC;
}


/*----------------------------------------------------------------------------*/
/*
    AUTODOC: tgfTrunkWithRemoteMembers_globalEPortRepresentTrunk - test trunk members with "remote" members.
            when the 'trunk' is represented in the ingress pipe by 'global EPort'
            this global ePort is mapped to trunkId at the end of the ingress pipe
            by the E2PHY mechanism.

            NOTE : this test based on tgfTrunkWithRemoteMembers.
*/
UTF_TEST_CASE_MAC(tgfTrunkWithRemoteMembers_globalEPortRepresentTrunk)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    tgfTrunkGlobalEPortRepresentTrunk = 1;
    UTF_TEST_CALL_MAC(tgfTrunkWithRemoteMembers);
    tgfTrunkGlobalEPortRepresentTrunk = 0;
}


/*----------------------------------------------------------------------------*/
/*
    tgfTrunkCascadeTrunkWithWeights
*/
UTF_TEST_CASE_MAC(tgfTrunkCascadeTrunkWithWeights)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* the 'Weight' is SW manipulation that we not need to check in GM , due to poor performance */
    GM_SKIP_TEST_DUE_TO_POOR_PERFORMANCE_MAC;

    /* Set configuration */
    tgfTrunkCascadeTrunkTestInit(GT_TRUE);

    /* Generate traffic */
    tgfTrunkCascadeTrunkTest();

    /* Restore configuration */
    tgfTrunkCascadeTrunkTestRestore();
}

/*----------------------------------------------------------------------------*/
/*
    tgfTrunkCascadeTrunkWithWeights
*/
UTF_TEST_CASE_MAC(tgfTrunkSortMode)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* the 'sort' is SW manipulation that we not need to check in GM , due to poor performance */
    GM_SKIP_TEST_DUE_TO_POOR_PERFORMANCE_MAC;

    tgfTrunkSortModeTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test CRC_32 hash using L2ECMP (global eport point to 128 secondary eports)
*/
UTF_TEST_CASE_MAC(tgfTrunkL2EcmpBasicCrc32HashTest)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    tgfL2EcmpBasicCrc32HashTest();
}
/*----------------------------------------------------------------------------*/
/* AUTODOC: Test CRC_32 hash with 'SALT' using L2ECMP (global eport point to 128 secondary eports)
*           the tests uses SALT A and SALT B on the specific byte and check that
*           is produce different hash value (by select different secondary eport)
*/
UTF_TEST_CASE_MAC(tgfTrunkL2EcmpBasicCrc32HashSaltTest)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E)

    tgfL2EcmpBasicCrc32HashSaltTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test L2ECMP for SA learning and source filtering.
*           1. define the L2ECMP as global EPorts in the Bridge(to ignore srcDev,trgDev
*               in SA learning and source filtering) ,
*           2. in FDB set the MAC SA with 'ownDev',eport=L2ECMP .
*           3. send traffic from cascade port ,
*             from 'reomte' srcDev and with 'srcEPort' of the L2ECMP.
*           4. check that no SA relearning due to change of source device.
*               4.a remove L2ECMP eport from 'global eports'
*                   and check that there is re-learning.
*                   (restore L2ECMP eport to 'global eports')
*           5. disable 'bride bypass' on cascade port
*           6. send traffic from cascade port ,
*             from 'reomte' srcDev and with 'srcEPort' of the L2ECMP , with DA
*             that associated with 'ownDev',eport=L2ECMP
*           7. check that packet is filtered although <'ownDev'> !=  <'reomte' srcDev>
*               7.a remove L2ECMP eport from 'global eports'
*                   and check that packet not filtered.
*
*/
UTF_TEST_CASE_MAC(tgfTrunkL2EcmpSaLearningAndFilter)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    tgfL2EcmpSaLearningAndFilterTest();
}
/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfTrunkSaLearning -
*   Test mac SA learning from ports that are trunk members in local device,
*   and from from ports that are trunk members in remote device (via the DSA tag)
*/
UTF_TEST_CASE_MAC(tgfTrunkSaLearning)
{
    /*check if the device support setting cascade on port for ingress different then for the egress */
    if(GT_FALSE == prvTgfCscdIngressPortEnableCheck(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }
    /* this test use 12 port. falcon GM have only 8 ports*/
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_FALCON_E);

    tgfTrunkSaLearningTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfTrunkSaLearning_globalEPortRepresentTrunk -
*   based on tgfTrunkSaLearning :
*   Test mac SA learning from ports that are members in global eport that represents a trunk in local device,
*   and from from ports that are members in global eport that represents a trunk  in remote device (via the DSA tag)
*/
UTF_TEST_CASE_MAC(tgfTrunkSaLearning_globalEPortRepresentTrunk)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    tgfTrunkGlobalEPortRepresentTrunk = 1;
    UTF_TEST_CALL_MAC(tgfTrunkSaLearning);
    tgfTrunkGlobalEPortRepresentTrunk = 0;
}

extern GT_BOOL appDemoIsTmEnabled(void);     /* SW */

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfTrunkCrcHash_4K_members_known_UC_EPCL -
*   set 4K members in a trunk
*   send 'known UC' traffic to it , check by EPCL that bound to CNC counters
*       that the members got traffic according to 'predicted' egress ports.
*/
UTF_TEST_CASE_MAC(tgfTrunkCrcHash_4K_members_known_UC_EPCL)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(appDemoIsTmEnabled())
    {
        /* the trunk members use high range of ports that collide with TM mapped ports */
        SKIP_TEST_MAC;
    }

    PRV_TGF_SKIP_PASS_OK_LONG_TEST_GM_MAC(UTF_CPSS_PP_ALL_SIP6_CNS);

    tgfTrunkCrcHash_4K_members_known_UC_EPCL();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfTrunk_80_members_flood_EPCL -
*   check that only first 64 members of the trunk get flooded traffic.
*   set 80 members in a trunk
*   send 4K flows of 'unknown' traffic to it , check by EPCL that bound to CNC counters
*       that only first 64 members of the trunk got traffic.
*   NOTE:
*   the test runs as '2 devices' : A and B
*   members in the trunk are ports from : A and B :
*       'first 64 ports : 25 ports from A , 39 ports from B
*       next 16 ports   : 8  ports from A , 8  ports from B
*   total 80 members
*
*   first run as device A :
*       send flood traffic , check that only those 25 ports got traffic and not
*       the other 8
*   second run as device B :
*       send flood traffic (the same traffic) , check that only those 39 ports got traffic and not
*       the other 8
*
*   NOTE: there is no use of DSA traffic , and the generating flood is network port
*           both on device A and on device B
*/
UTF_TEST_CASE_MAC(tgfTrunk_80_members_flood_EPCL)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(appDemoIsTmEnabled())
    {
        /* the trunk members use high range of ports that collide with TM mapped ports */
        SKIP_TEST_MAC;
    }

    PRV_TGF_SKIP_PASS_OK_LONG_TEST_GM_MAC(UTF_CPSS_PP_ALL_SIP6_CNS);

    if(prvUtfIsGmCompilation() == GT_TRUE)
    {
        /* BC2,BOBK devices not support mapping of physical port to mac port */
        /* and it cause the test to crash because of accessing to MAC numbers
           more than actual max number */
        /* was fixed in GM of BC3 only */
        PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_BOBCAT2_E | UTF_CAELUM_E);

    }



    tgfTrunk_80_members_flood_EPCL();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IPCL Load balancing hash
*  0. Configure hash bits offset and num
*  1. Packet sent to ingress.
*  2. One of 64 IPCL rules selected
*  3. Rule action selects CNC counter index to increment. Counter index is
*     is chosen as index of the most significanthash bit which is non-zero in
*     the hash. Hash calculation mode is Simple(XOR) so we can easily predict
*     what hash to expect.
*  4. CNC counter incremented
*  5. Check expected CNC counter
*/
UTF_TEST_CASE_MAC(tgfTrunkIpclLoadBalancingHash)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    tgfTrunkIpclLoadBalancingHashConfig(GT_TRUE);
    tgfTrunkIpclLoadBalancingHashExecute();
    tgfTrunkIpclLoadBalancingHashConfig(GT_FALSE);
}

/*
 * Configuration of tgfTrunk suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfTrunk)
    /* L2ECMP related */
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkL2EcmpBasicCrc32HashTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkL2EcmpBasicCrc32HashSaltTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkL2EcmpSaLearningAndFilter)

    /* trunk related */
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkSaLearning)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkSaLearning_globalEPortRepresentTrunk)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkSimpleHashMacSaDa)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkSimpleHashMacSaDa_globalEPortRepresentTrunk)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkWithRemoteMembers)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkWithRemoteMembers_globalEPortRepresentTrunk)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkCrcHash)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkCrcHashL3L4OverMplsParseEnable)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkCrcHashMaskPriority)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkCascadeTrunk)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkDesignatedTableModes)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkHashMplsModeEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkHashL3L4OverMplsParseEnable)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkHashMaskSet)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkHashIpShiftSet)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkDesignatedPorts)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkToCpu)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkMcLocalSwitching)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkCascadeTrunkWithWeights)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkSortMode)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkCrcHash_liteMode_CRC32_cpssPredictEgressPort)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkCrcHash_liteMode_CRC32_cpssPredictEgressPort_symmetricHash)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkCrcHash_4K_members_known_UC_EPCL)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunk_80_members_flood_EPCL)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkIpclLoadBalancingHash)

UTF_SUIT_END_TESTS_MAC(tgfTrunk)


