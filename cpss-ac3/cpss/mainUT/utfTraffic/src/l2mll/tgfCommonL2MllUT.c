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
* @file tgfCommonL2MllUT.c
*
* @brief Enhanced UTs for CPSS L2 MLL
*
* @version   22
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <common/tgfL2MllGen.h>
#include <l2mll/prvTgfLttEvidxRange.h>
#include <l2mll/prvTgfTtlException.h>
#include <l2mll/prvTgfMaskProfileSkipAndCounters.h>
#include <l2mll/prvTgfUnucUnregmcBcFiltering.h>
#include <l2mll/prvTgfMcLocalSwitching.h>
#include <l2mll/prvTgfMaxHopCount.h>
#include <l2mll/prvTgfEgressInterface.h>
#include <l2mll/prvTgfFullMll.h>
#include <l2mll/prvTgfReplicateToCpu.h>
#include <l2mll/prvTgfTaggedUntaggedComb.h>
#include <l2mll/prvTgfGlobalEport.h>

/* macro to check if the device supports L2 MLL */
#define PRV_TGF_NOT_SUPPORTED_L2_MLL_CHECK_MAC(_devNum)         \
    if(GT_FALSE == prvTgfL2MllCheck(_devNum))                   \
    {                                                           \
        /* set skip flag and update skip tests num */           \
        prvUtfSkipTestsSet();                                   \
                                                                \
        return ;                                                \
    }

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test LTT eVIDX range:
    configure VLAN, FDB entries;
    configure L2 MLL configuration and partition mode;
    map eVIDX to local port;
    create L2 MLL entries with different ports;
    enable\disable MLL lookup for all multi-target packets;
    send Ethernet traffic with different DAs and verify expected traffic.
*/
UTF_TEST_CASE_MAC(prvTgfLttEvidxRange)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_L2_MLL_CHECK_MAC(prvTgfDevNum);

    /* Set configuration */
    prvTgfLttEvidxRangeConfigurationSet();

    /* Generate traffic */
    prvTgfLttEvidxRangeTrafficGenerate();

    /* Restore configuration */
    prvTgfLttEvidxRangeConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test TTL exceptions:
    configure VLAN, FDB entries;
    configure L2 MLL configuration and partition mode;
    enable MLL lookup for all multi-target packets;
    create L2 MLL and LTT entries and enable TTL exception trap to CPU;
    send IPv4 traffic with different TTLs and L2 MLL TTL treshold;
    verify correct L2 MLL TTL exception behavior.
*/
UTF_TEST_CASE_MAC(prvTgfTtlException)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_L2_MLL_CHECK_MAC(prvTgfDevNum);

    /* Set configuration */
    prvTgfTtlExceptionConfigurationSet();

    /* Generate traffic */
    prvTgfTtlExceptionTrafficGenerate();

    /* Restore configuration */
    prvTgfTtlExceptionConfigurationRestore();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test TTL exceptions:
  There is unknown unicast traffic that is received on port 18 on VLAN 3
  port 0 is part of the VLAN 3, only physical ports are members of VLAN.
  VLAN 3 is also "eVlan" 3, meaning switching on ePorts is done on VLAN 3
  on eVLAN 3 we define a floodVIDX 5000, ePorts 100 and 101 are members of
  this VIDX, which is actually L2MLL group
  all above ePorts are mapped to physical port 1
  On ePort 101 we define vlan translation of VLAN 1 to VLAN 3
  Port 0 is also a member of VLAN 4, actually we define VLAN 4 filtering
  table as including ePort 2 or disable egress VLAN filtering altogether
  All the above will lead to duplication of ingress packet and both copies
  will egress on physical port 0, but one of the copies will have VLAN4 tag,
  since port 0 is tagged and because of VLAN translation on egress
*/
UTF_TEST_CASE_MAC(prvTgfTtlExceptionVlanPortTranslation)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_L2_MLL_CHECK_MAC(prvTgfDevNum);

    /* Set configuration */
    prvTgfTtlExceptionVlanPortTranslationConfigurationSet(65);

    /* Generate traffic */
    prvTgfTtlExceptionVlanPortTranslationTrafficGenerate();

    /* Restore configuration */
     prvTgfTtlExceptionVlanPortTranslationConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test MLL mask profile and skip couners:
    configure VLAN, FDB entries;
    configure L2 MLL configuration and partition mode;
    create 3 L2 MLL entries with different ports, maskBitmap and mllCounterIndex;
    add 3 L2 MLL LTT entries with different mllMaskProfile;
    for each L2 MLL LTT entry send Ethernet traffic;
    verify expected traffic on port from appropriate L2 MLL entries;
    verify L2 MLL exception Skip counter to be 2;
    disable MLL Mask Profile;
        send Ethernet traffic and verify expected traffic on all ports;
        verify L2 MLL exception Skip counter to be 0.
*/
UTF_TEST_CASE_MAC(prvTgfMaskProfileSkipAndCounters)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_L2_MLL_CHECK_MAC(prvTgfDevNum);

    /* Set configuration */
    prvTgfMaskProfileSkipAndCountersConfigurationSet();

    /* Generate traffic */
    prvTgfMaskProfileSkipAndCountersTrafficGenerate();

    /* Restore configuration */
    prvTgfMaskProfileSkipAndCountersConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Unknown UC\Unregister MC\BC traffic filterring:
    configure VLAN, FDB entries;
    configure L2 MLL configuration and partition mode;
    create 3 L2 MLL entries with different ports;
    send Unknown UC\Unregister MC\BC traffic;
    verify expected traffic on all ports;
    modify L2 MLL entries to enable unknown UC\unreg MC\BC Filterring;
    for each modification send Unknown UC\Unregister MC\BC traffic;
    verify that appropriate type of traffic is filtered.
*/
UTF_TEST_CASE_MAC(prvTgfUnucUnregmcBcFiltering)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_L2_MLL_CHECK_MAC(prvTgfDevNum);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfUnucUnregmcBcFilteringConfigurationSet();

    /* Generate traffic */
    prvTgfUnucUnregmcBcFilteringTrafficGenerate();

    /* Restore configuration */
    prvTgfUnucUnregmcBcFilteringConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test MC local switching:
    configure VLAN, FDB entries;
    configure L2 MLL configuration and partition mode;
    create 2 L2 MLL entries with different ports;
    send Unregister MC Ethernet traffic verify expected traffic on eVidx MLL port;
        enable PE filtering\disable PE filtering\disable MC local switching, enable PE filtering;
        for each modification send Unregistered MC traffic;
        verify that appropriate traffic is filtered correctly.
*/
UTF_TEST_CASE_MAC(prvTgfMcLocalSwitching)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_L2_MLL_CHECK_MAC(prvTgfDevNum);

    /* Skip this test for GM - performance reason */
    GM_SKIP_TEST_DUE_TO_POOR_PERFORMANCE_MAC;

    /* Set configuration */
    prvTgfMcLocalSwitchingConfigurationSet();

    /* Generate traffic */
    prvTgfMcLocalSwitchingTrafficGenerate();

    /* Restore configuration */
    prvTgfMcLocalSwitchingConfigurationRestore();
}

#if 0 /*trill tests are not operational*/
/*----------------------------------------------------------------------------*/
/* AUTODOC: Test max Hot count update:
    configure VLAN, FDB entries;
    configure L2 MLL configuration and partition mode;
    create L2 MLL entry and enable Max HOP count;
    send Trill traffic and verify expected traffic with updated Hop Count.
*/
UTF_TEST_CASE_MAC(prvTgfMaxHopCount)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_L2_MLL_CHECK_MAC(prvTgfDevNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(1,0,"trill tests are not operational \n");

    /* Set configuration */
    prvTgfMaxHopCountConfigurationSet();

    /* Generate traffic */
    prvTgfMaxHopCountTrafficGenerate();

    /* Restore configuration */
    prvTgfMaxHopCountConfigurationRestore();
}
#endif

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test L2 MLL Egress Interface:
    update default ports list to test specific;
    configure VLAN, FDB, eVIDX, Trunk entries;
    configure L2 MLL configuration and partition mode;
    create L2 MLL entries on ports, trunk, eVidx;
    update L2 MLL LTT entry and mask different MLL profiles;
    for each modification send Unregister MC Ethernet traffic;
    verify expected traffic on correct interface (port, trunk or eVidx).
*/
UTF_TEST_CASE_MAC(prvTgfEgressInterface)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* this test use 10 port. falcon GM have only 8 ports*/
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_FALCON_E);

    PRV_TGF_NOT_SUPPORTED_L2_MLL_CHECK_MAC(prvTgfDevNum);

    /* Set configuration */
    prvTgfEgressInterfaceConfigurationSet();

    /* Generate traffic */
    prvTgfEgressInterfaceTrafficGenerate();

    /* Restore configuration */
    prvTgfEgressInterfaceConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test L2 MLL Egress Interface:
    update default ports list to test specific;
    configure VLAN, FDB, eVIDX, Trunk entries;
    configure L2 MLL configuration and partition mode;
    configure L2 MLL entrySelector configuration;
    create L2 MLL entries on ports, trunk, eVidx;
    update L2 MLL LTT entry and mask different MLL profiles;
    for each modification send Unregister MC Ethernet traffic;
    verify expected traffic on correct interface (port, trunk or eVidx).
*/
UTF_TEST_CASE_MAC(prvTgfEgressInterfaceEntrySelector)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* this test use 10 port. falcon GM have only 8 ports*/
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_FALCON_E);

    PRV_TGF_NOT_SUPPORTED_L2_MLL_CHECK_MAC(prvTgfDevNum);

    /* Set configuration */
    prvTgfEgressInterfaceConfigurationSet();

    /* Generate traffic */
    prvTgfEgressInterfaceEntrySelectorTrafficGenerate();

    /* Restore configuration */
    prvTgfEgressInterfaceConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test L2 MLL full MLL table:
    configure VLAN, FDB entries;
    configure L2 MLL configuration and partition mode;
    create all possible L2 MLL entries with different two ports;
    send Unregister MC Ethernet traffic;
    verify expected traffic on configured ports (each of 2 ports has totalNum/2 packets).
*/
UTF_TEST_CASE_MAC(prvTgfFullMll)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_L2_MLL_CHECK_MAC(prvTgfDevNum);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /* Skip this test for GM - performance reason */
    GM_SKIP_TEST_DUE_TO_POOR_PERFORMANCE_MAC;

    /* Set configuration */
    prvTgfFullMllConfigurationSet();

    /* Generate traffic */
    prvTgfFullMllTrafficGenerate();

    /* Restore configuration */
    prvTgfFullMllConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test L2 MLL full MLL table in one Mll list chunk:
    configure VLAN, FDB entries;
    configure L2 MLL configuration and partition mode;
    create all possible L2 MLL entries with different two ports;
    send Unregister MC Ethernet traffic;
    verify expected traffic on configured ports (each of 2 ports has totalNum/2 packets).
*/
UTF_TEST_CASE_MAC(prvTgfFullMllOneList)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_L2_MLL_CHECK_MAC(prvTgfDevNum);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /* Skip this test for GM - performance reason */
    GM_SKIP_TEST_DUE_TO_POOR_PERFORMANCE_MAC;

        /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    /* Set configuration */
    prvTgfFullMllOneListConfigurationSet();

    /* Generate traffic */
    prvTgfFullMllOneListTrafficGenerate();

    /* Restore configuration */
    prvTgfFullMllOneListConfigurationRestore();
#endif
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test L2 MLL replicate to CPU:
    configure VLAN, FDB on eVIDX entries;
    configure L2 MLL configuration and partition mode;
    create L2 MLL entry on port;
    send Unregister MC Ethernet traffic verify expected traffic on eVidx MLL port;
    create L2 MLL entry on CPU port;
    send Unregister MC Ethernet traffic verify expected traffic on eVidx MLL port;
        verify to capture packet with Forward cpuCode.
*/
UTF_TEST_CASE_MAC(prvTgfReplicateToCpu)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_L2_MLL_CHECK_MAC(prvTgfDevNum);

    /* Set configuration */
    prvTgfReplicateToCpuConfigurationSet();

    /* Generate traffic */
    prvTgfReplicateToCpuTrafficGenerate();

    /* Restore configuration */
    prvTgfReplicateToCpuConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test L2 MLL Tagged\Untagged for Ingress\Egress combinations:
    configure VLAN, FDB on eVIDX entries;
    configure L2 MLL configuration and partition mode;
    create L2 MLL entry on port;
    send Tagged Ethernet traffic verify expected UnTagged traffic on eVidx MLL port;
    send UnTagged Ethernet traffic verify expected UnTagged traffic on eVidx MLL port;
    Update VLAN entry and add tagged port;
    send Tagged Ethernet traffic verify expected Tagged traffic on eVidx MLL port;
    send UnTagged Ethernet traffic verify expected Tagged traffic on eVidx MLL port.
*/
UTF_TEST_CASE_MAC(prvTgfTaggedUntaggedComb)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_L2_MLL_CHECK_MAC(prvTgfDevNum);

    /* Set configuration */
    prvTgfTaggedUntaggedCombConfigurationSet();

    /* Generate traffic */
    prvTgfTaggedUntaggedCombTrafficGenerate();

    /* Restore configuration */
    prvTgfTaggedUntaggedCombConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test L2 MLL Global ePort:
    configure VLAN, FDB on eVIDX entries;
    configure L2 MLL configuration and partition mode;
    create L2 MLL entry on port;
    clear Global ePorts configuration;
    send Unregister MC Ethernet traffic verify expected traffic on eVidx MLL port;
    check L2 MLL exception Skip counter is 0;
    configure Global ePorts - ECMP primary ePorts;
    send Unregister MC Ethernet traffic verify expected traffic on eVidx MLL port;
    check L2 MLL exception Skip counter is 1.
*/
UTF_TEST_CASE_MAC(prvTgfGlobalEport)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_L2_MLL_CHECK_MAC(prvTgfDevNum);

    /* Set configuration */
    prvTgfGlobalEportConfigurationSet();

    /* Generate traffic */
    prvTgfGlobalEportTrafficGenerate();

    /* Restore configuration */
    prvTgfGlobalEportConfigurationRestore();
}

/*
 * Configuration of tgfL2Mll suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfL2Mll)
        UTF_SUIT_DECLARE_TEST_MAC(prvTgfLttEvidxRange)
        UTF_SUIT_DECLARE_TEST_MAC(prvTgfTtlException)
        UTF_SUIT_DECLARE_TEST_MAC(prvTgfTtlExceptionVlanPortTranslation)
        UTF_SUIT_DECLARE_TEST_MAC(prvTgfMaskProfileSkipAndCounters)
        UTF_SUIT_DECLARE_TEST_MAC(prvTgfUnucUnregmcBcFiltering)
        UTF_SUIT_DECLARE_TEST_MAC(prvTgfMcLocalSwitching)
        CPSS_TBD_BOOKMARK_EARCH
#if 0
        /* Max hop count test will fail - wait for APIs support for TRILL. */
        UTF_SUIT_DECLARE_TEST_MAC(prvTgfMaxHopCount)
#endif
        UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressInterface)
        UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressInterfaceEntrySelector)
        UTF_SUIT_DECLARE_TEST_MAC(prvTgfFullMll)
        UTF_SUIT_DECLARE_TEST_MAC(prvTgfFullMllOneList)
        UTF_SUIT_DECLARE_TEST_MAC(prvTgfReplicateToCpu)
        UTF_SUIT_DECLARE_TEST_MAC(prvTgfTaggedUntaggedComb)
        UTF_SUIT_DECLARE_TEST_MAC(prvTgfGlobalEport)

UTF_SUIT_END_TESTS_MAC(tgfL2Mll)


