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
* @file tgfCommonIpUT.c
*
* @brief Enhanced UTs for CPSS IPv4 and IPv6
*
* @version   94
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <ip/prvTgfBasicIpv4UcRouting.h>
#include <ip/prvTgfBasicIpv4UcRoutingTrapToCpu.h>
#include <ip/prvTgfBasicIpv4UcEcmpRouting.h>
#include <ip/prvTgfBasicIpv6UcRouting.h>
#include <ip/prvTgfBasicIpv4UcLowLevelRouting.h>
#include <ip/prvTgfBasicIpv4McRouting.h>
#include <ip/prvTgfBasicIpv6McRouting.h>
#include <ip/prvTgfBasicIpv4Uc2VirtualRouter.h>
#include <ip/prvTgfBasicIpv42SharedVirtualRouter.h>
#include <ip/prvTgfIpv4Uc2MultipleVr.h>
#include <ip/prvTgfDefaultIpv4McRouting.h>
#include <ip/prvTgfDefaultIpv6McRouting.h>
#include <ip/prvTgfComplicatedIpv4McRouting.h>
#include <ip/prvTgfIpLpmDBCapacityUpdate.h>
#include <ip/prvTgfIpNonExactMatchRoutingIpv4Uc.h>
#include <ip/prvTgfIpNonExactMatchRoutingIpv6Uc.h>
#include <ip/prvTgfIpNonExactMatchRoutingIpv4Mc.h>
#include <ip/prvTgfIpNonExactMatchRoutingIpv6Mc.h>
#include <ip/prvTgfComplicatedPbrIpResize.h>
#include <ip/prvTgfComplicatedPbrIpPclResize.h>
#include <ip/prvTgfIpResizeDefSmallerBigger.h>
#include <ip/prvTgfIpPclResizeDefSmallerBigger.h>
#include <ip/prvTgfTablesFillingLtt.h>
#include <ip/prvTgfTablesFillingLpm.h>
#include <ip/prvTgfIpMtuExceedExceptionCheck.h>
#include <ip/prvTgfIpv4UcRoutingHiddenPrefixes.h>
#include <ip/prvTgfIpv4UcRoutingAddMany.h>
#include <ip/prvTgfIpv4UcRoutingAddManyIpLpmMng.h>
#include <ip/prvTgfIpv6UcRoutingAddManyIpLpmMng.h>
#include <ip/prvTgfIpv4UcRoutingAddRealPrefixes.h>
#include <ip/prvTgfIpv4UcRoutingAddDeleteAddRealPrefixes.h>
#include <ip/prvTgfIpv4Ipv6UcRoutingAddManyIpLpmMng.h>
#include <ip/prvTgfIpv6UcRoutingAddMany.h>
#include <ip/prvTgfIpv4McRoutingAddMany.h>
#include <ip/prvTgfIpv4McRoutingAddRealPrefixes.h>
#include <ip/prvTgfIpv6McRoutingAddMany.h>
#include <ip/prvTgfDynamicPartitionIpLpmUcRouting.h>
#include <ip/prvTgfDynamicPartitionIpLpmMcRouting.h>
#include <ip/prvTgfIpv6UcSaMismatchCheck.h>
#include <ip/prvTgfIpv6McRpfFail.h>
#include <ip/prvTgfIpv4Ipv6UcMcRoutingAddDelAndFillLpm.h>
#include <ip/prvTgfIpv4UcRoutingPriotiry.h>
#include <ip/prvTgfIpv4UcPbrRoutingPriotiry.h>
#include <common/tgfIpGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfL2MllGen.h>
#include <trunk/prvTgfTrunk.h>
#include <ip/prvTgfIpLpmIpUcPrefixBulkAdd.h>
#include <common/tgfCscdGen.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#define RUN_LONG_CAPACITY_TESTS 0

/* macro for unstable test */
#define TGF_UNSTABLE_TEST_FORCE_FAIL_MAC    \
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, GT_FALSE, "force fail for unstable test");

/* macro to check if the device supports L2 MLL */
#define PRV_TGF_NOT_SUPPORTED_L2_MLL_CHECK_MAC(_devNum)         \
    if(GT_FALSE == prvTgfL2MllCheck(_devNum))                   \
    {                                                           \
        /* set skip flag and update skip tests num */           \
        prvUtfSkipTestsSet();                                   \
                                                                \
        return ;                                                \
    }

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0


static PRV_TGF_IPV4_STACKING_SYSTEM_ENT   stackingSystemType = PRV_TGF_IPV4_STACKING_SYSTEM_NONE_E;

GT_VOID prvTgfBasicIpv4McRoutingSourceId(GT_VOID);
GT_VOID prvTgfIpv4UcNatTcpRouting(GT_VOID);
GT_VOID prvTgfIpv4UcNatUdpWithOptions(GT_VOID);

void prvTgfCaptureTpidWithZeroBmpDisable(void);
void prvTgfCaptureTpidWithZeroBmpRestore(void);



/*In Kilobytes*/
static GT_U32 allowedPbrSizes [] = {8,0};

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv4 unicast routing:
    configure VLAN, FDB entries;
    configure UC Routing entries;
    add IPv4 UC prefix to LPM DB with exact packet's DIP match;
    send IPv4 UC traffic and verify expected routed traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfBasicIpv4UcRouting)
{
/*
    1. Set Base configuration
    2. Set Route configuration for vrfId[0]
    3. Generate traffic
    4. Restore configuration
    5. Select random vrfId
    6. Set Base configuration
    7. Set Route configuration for vrfId[random]
    8. Generate traffic
    9. Restore configuration
*/

    GT_U32      prvUtfVrfId = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    if(stackingSystemType == PRV_TGF_IPV4_STACKING_SYSTEM_NONE_E)
    {
        /* Set Base configuration */
        prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

        /* Set Route configuration for vrfId[0] */
        prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                      prvTgfPortsArray[3], GT_FALSE,
                                                      GT_FALSE, GT_FALSE);

        /* Generate traffic */
        prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS);

        /* Restore configuration */
        prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);
    }

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

    /* Set Route configuration for vrfId[random] */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                  prvTgfPortsArray[3], GT_FALSE,
                                                  GT_FALSE, GT_FALSE);

    /* Generate traffic */
    prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS);

    /* Restore configuration */
    prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, 3, GT_FALSE);

    if (prvUtfIsPbrModeUsed() == GT_FALSE)
    {
        /*Repeat the test with enabled IPv4 MC routing and disabled IPv6 MC routing */
        /* Set Base configuration */
        prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

        /* Set Route configuration for vrfId[random] */
        prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                      prvTgfPortsArray[3], GT_FALSE,
                                                      GT_TRUE, GT_FALSE);

        /* Generate traffic */
        prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS);

        /* Restore configuration */
        prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, 3, GT_FALSE);

        /*Repeat the test with disabled IPv4 MC routing and enabled IPv6 MC routing */
        /* Set Base configuration */
        prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

        /* Set Route configuration for vrfId[random] */
        prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                      prvTgfPortsArray[3], GT_FALSE,
                                                      GT_FALSE, GT_TRUE);

        /* Generate traffic */
        prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS);

        /* Restore configuration */
        prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, 3, GT_FALSE);

        /*Repeat the test with enabled IPv4 MC routing and enabled IPv6 MC routing */
        /* Set Base configuration */
        prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

        /* Set Route configuration for vrfId[random] */
        prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                      prvTgfPortsArray[3], GT_FALSE,
                                                      GT_TRUE, GT_TRUE);

        /* Generate traffic */
        prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS);

        /* Restore configuration */
        prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, 3, GT_FALSE);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv4 unicast routing:
    configure VLAN, FDB entries;
    configure UC Routing entries;
    add IPv4 UC prefix to LPM DB with exact packet's DIP match;
    add EPCL rule to match packets with DSA tag command TO_CPU and
    vlanId = PRV_TGF_NEXTHOPE_VLANID_CNS.(metadata key - supporte only for eArch)
    EPCL Action will be DROP
    We expect the vlanId in the DSA tag to be the original vlan of the packet
    send IPv4 UC traffic and verify expected routed traffic trapped to CPU
*/
UTF_TEST_CASE_MAC(tgfBasicIpv4UcRoutingTrapToCpu)
{
/*
    1. Set Base configuration
    2. Set Route configuration for vrfId[0]
    3. Generate traffic
    4. Restore configuration
*/

    GT_U32      notAppFamilyBmp;
    GT_U32      prvUtfVrfId = 0;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

    /* Set Route configuration */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0, prvTgfPortsArray[3], GT_FALSE, GT_FALSE, GT_FALSE);

    /* Set Trap To CPU configuration */
    prvTgfBasicIpv4UcRoutingTrapToCpuConfigurationSet(prvTgfPortsArray[0]);

    /* Set regularEntryPtr->cmd to CPSS_PACKET_CMD_TRAP_TO_CPU_E , expect traffic to be trapped */
    prvTgfBasicIpv4UcRoutingTrapToCpuTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3]);

    /* Restore configuration */
    prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);

    /* Restore Trap To CPU configuration */
    prvTgfBasicIpv4UcRoutingTrapToCpuConfigurationRestore(prvTgfPortsArray[0]);

}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv4 unicast routing:
    configure VLAN, FDB entries;
    configure UC Routing entries;
    enable bypassing lookup stages 8-31
    add IPv4 UC prefix to LPM DB with exact packet's DIP match;
    send IPv4 UC traffic and verify expected routed traffic on nextHop port.
*/

UTF_TEST_CASE_MAC(tgfBasicIpv4UcRouting4BytesMode)
{
/*
    1. Set Base configuration
    2. Set Route configuration for vrfId[0]
    3. Generate traffic
    4. Restore configuration
    5. Select random vrfId
    6. Set Base configuration
    7. Set Route configuration for vrfId[random]
    8. Generate traffic
    9. Restore configuration
*/

    GT_U32      prvUtfVrfId = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(stackingSystemType == PRV_TGF_IPV4_STACKING_SYSTEM_NONE_E)
    {
        /* Set Base configuration */
        prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

        /* Set Route configuration for vrfId[0] */
        prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                      prvTgfPortsArray[3], GT_FALSE,
                                                      GT_FALSE, GT_FALSE);

        prvTgfBasicIpv4LpmConfigurationSet(prvTgfPortsArray[0],GT_TRUE,GT_TRUE);

        /* Generate traffic */
        prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS);

        /* Restore configuration */
        prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);
    }

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

    /* Set Route configuration for vrfId[random] */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                  prvTgfPortsArray[3], GT_FALSE,
                                                  GT_FALSE, GT_FALSE);

    prvTgfBasicIpv4LpmConfigurationSet(prvTgfPortsArray[0],GT_TRUE,GT_TRUE);

    /* Generate traffic */
    prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS);

    /* Restore configuration */
    prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, 3, GT_FALSE);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv4 unicast routing when Sip Lookup is Disable:
    configure VLAN, FDB entries;
    configure UC Routing entries;
    enable bypassing lookup stages 8-31
    add IPv4 UC prefix to LPM DB with exact packet's DIP match;
    send IPv4 UC traffic and verify expected routed traffic on nextHop port.
*/

UTF_TEST_CASE_MAC(tgfBasicIpv4UcRoutingSipLookupDisable)
{
/*
    1. Set Base configuration
    2. Set Route configuration for vrfId[0]
    3. Generate traffic
    4. Restore configuration
    5. Select random vrfId
    6. Set Base configuration
    7. Set Route configuration for vrfId[random]
    8. Generate traffic
    9. Restore configuration
*/

    GT_U32      prvUtfVrfId = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E );
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(stackingSystemType == PRV_TGF_IPV4_STACKING_SYSTEM_NONE_E)
    {
        /* Set Base configuration */
        prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

        /* Set Route configuration for vrfId[0] */
        prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                      prvTgfPortsArray[3], GT_FALSE,
                                                      GT_FALSE, GT_FALSE);
        /*  disable SIP Lookup */
        prvTgfBasicIpv4LpmConfigurationSet(prvTgfPortsArray[0], GT_FALSE, GT_FALSE);

        prvTgfBasicIpv4UcRoutingRouteSipLookupConfigurationSet(prvUtfVrfId);

        /* Generate traffic - expected traffic */
        prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, GT_FALSE);

        /*  enable SIP Lookup */
        prvTgfBasicIpv4LpmConfigurationSet(prvTgfPortsArray[0], GT_FALSE, GT_TRUE);

        /* Generate traffic - expected NO traffic */
        prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, GT_TRUE);

        /* Restore configuration */
        prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);
    }

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

    /* Set Route configuration for vrfId[random] */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                  prvTgfPortsArray[3], GT_FALSE,
                                                  GT_FALSE, GT_FALSE);

    /*  disable SIP Lookup */
    prvTgfBasicIpv4LpmConfigurationSet(prvTgfPortsArray[0],GT_FALSE, GT_FALSE);

    prvTgfBasicIpv4UcRoutingRouteSipLookupConfigurationSet(prvUtfVrfId);

    /* Generate traffic - expected traffic */
    prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, GT_FALSE);

    /*  enable SIP Lookup */
    prvTgfBasicIpv4LpmConfigurationSet(prvTgfPortsArray[0], GT_FALSE, GT_TRUE);

    /* Generate traffic - expected NO traffic */
    prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, GT_TRUE);

    /* Restore configuration */
    prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, 3, GT_FALSE);
}



/*----------------------------------------------------------------------------*/
/* AUTODOC: run test tgfBasicIpv4UcRouting. in the next system.
    current device Y get DSA packet from srcDev X , and device Y need to do routing.
    the target port of the route is on device X.
    the test checks the DSA tag that device Y send towards device X.
    need to see that the 'srcDev' field in the DSA changed from 'X' to 'Y' so
    device X will not do filter of packet 'from me from cascade port'

    NOTE: work with extended DSA tag
*/
UTF_TEST_CASE_MAC(tgfBasicIpv4UcRouting_DSA_fromRemoteDeviceX_toRemoteDeviceX)
{
    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /*check if the device support setting cascade on port for ingress different then for the egress */
    if(GT_FALSE == prvTgfCscdIngressPortEnableCheck(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        GT_STATUS   rc;
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   0, 3,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }



    stackingSystemType = PRV_TGF_IPV4_STACKING_SYSTEM_ROUTING_DEVICE_E;
    prvTgfBasicIpv4UcRoutingInStackingSystemEnable(stackingSystemType);
    UTF_TEST_CALL_MAC(tgfBasicIpv4UcRouting);
    stackingSystemType = PRV_TGF_IPV4_STACKING_SYSTEM_NONE_E;
    prvTgfBasicIpv4UcRoutingInStackingSystemEnable(stackingSystemType);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Emulate device in 'stacking' system.
    current device X get DSA packet from srcDev X , after device Y did routing.
    the target port of the route is on device X.
    the test send the ingress packet with DSA tag that device Y send towards device X.

    this test is the complement of test tgfBasicIpv4UcRouting_DSA_fromRemoteDeviceX_toRemoteDeviceX

    need to see that device 'X' will filter (due to 'loops prevention in stacking system')
    the packet that came with srcDev 'X' ,
    but will redirect (to egress port) when srcDev is 'Y'.

    NOTE: in this system device X is not set with any 'Routing' configurations.
        as it needs to apply to the destination interface in the DSA tag.

    NOTE: work with extended DSA tag
*/
UTF_TEST_CASE_MAC(tgfBasicIpv4UcRouting_DsaRouted_backToSrcDev)
{
    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /*check if the device support setting cascade on port for ingress different then for the egress */
    if(GT_FALSE == prvTgfCscdIngressPortEnableCheck(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    if( GT_TRUE == prvTgfCommonIsDeviceForce(prvTgfDevNum))
    {
        /* the test use the <DSATag_SrcDevIsLocal_FilterDis> , but in this system
           the filter is disabled */
        prvUtfSkipTestsSet();
        return ;
    }

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        GT_STATUS   rc;
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   0,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }



    stackingSystemType = PRV_TGF_IPV4_STACKING_SYSTEM_AFTER_ROUTING_DEVICE_E;
    prvTgfBasicIpv4UcRoutingInStackingSystemEnable(stackingSystemType);
    UTF_TEST_CALL_MAC(tgfBasicIpv4UcRouting);
    stackingSystemType = PRV_TGF_IPV4_STACKING_SYSTEM_NONE_E;
    prvTgfBasicIpv4UcRoutingInStackingSystemEnable(stackingSystemType);
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv6 unicast routing:
    configure VLAN, FDB entries;
    configure UC Routing entries;
    add IPv6 UC prefix to LPM DB with exact packet's DIP match;
    send IPv6 UC traffic and verify expected routed traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfBasicIpv6UcRouting)
{
/*
    1. Set Base configuration
    2. Set Route configuration for vrfId[0]
    3. Generate traffic
    4. Restore configuration
    5. Select random vrfId
    6. Set Base configuration
    7. Set Route configuration for vrfId[random]
    8. Generate traffic
    9. Restore configuration
*/

    GT_U32      prvUtfVrfId = 0;

    PRV_TGF_SKIP_PASS_OK_ON_FULL_LPM_MEM_IN_GM_MAC(UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Set Base configuration */
    prvTgfBasicIpv6UcRoutingBaseConfigurationSet();

    /* Set Route configuration for vrfId[0]*/
    prvTgfBasicIpv6UcRoutingRouteConfigurationSet(prvUtfVrfId, GT_FALSE);

    /* Generate traffic */
    prvTgfBasicIpv6UcRoutingTrafficGenerate(GT_FALSE,GT_TRUE,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* Restore configuration */
    prvTgfBasicIpv6UcRoutingConfigurationRestore(prvUtfVrfId, GT_FALSE);

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    prvTgfBasicIpv6UcRoutingBaseConfigurationSet();

    /* Set Route configuration for vrfId[random] */
    prvTgfBasicIpv6UcRoutingRouteConfigurationSet(prvUtfVrfId, GT_FALSE);

    /* Generate traffic */
    prvTgfBasicIpv6UcRoutingTrafficGenerate(GT_FALSE,GT_TRUE,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* Restore configuration */
    prvTgfBasicIpv6UcRoutingConfigurationRestore(prvUtfVrfId, GT_FALSE);

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv6 unicast routing:
    configure VLAN, FDB entries;
    configure UC Routing entries;
    enable bypassing lookup stages 8-31
    add IPv6 UC prefix to LPM DB with exact packet's DIP match;
    send IPv6 UC traffic and verify no hit on the first 4 bytes --> get an exception
    disable bypassing lookup stages 8-31
    send IPv6 UC traffic and verify expected routed traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfBasicIpv6UcRouting4BytesMode)
{
/*
    1. Set Base configuration
    2. Set Route configuration for vrfId[0]
    3. Generate traffic
    4. Restore configuration
    5. Select random vrfId
    6. Set Base configuration
    7. Set Route configuration for vrfId[random]
    8. Generate traffic
    9. Restore configuration
*/

    GT_U32      prvUtfVrfId = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_FALCON_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /*  enable bypassing lookup stages 8-31 */
    prvTgfBasicIpv6LpmConfigurationSet(prvTgfPortsArray[0],GT_TRUE,GT_TRUE);

    /* Set Base configuration */
    prvTgfBasicIpv6UcRoutingBaseConfigurationSet();

    /* Set Route configuration for vrfId[0]*/
    prvTgfBasicIpv6UcRoutingRouteConfigurationSet(prvUtfVrfId, GT_FALSE);

    /* Generate traffic */
    prvTgfBasicIpv6UcRoutingTrafficGenerate4BytesMode();

    /* Restore configuration */
    prvTgfBasicIpv6UcRoutingConfigurationRestore(prvUtfVrfId, GT_FALSE);


    /*  disable bypassing lookup stages 8-31 */
    prvTgfBasicIpv6LpmConfigurationSet(prvTgfPortsArray[0],GT_FALSE,GT_TRUE);

    /* Set Base configuration */
    prvTgfBasicIpv6UcRoutingBaseConfigurationSet();

    /* Set Route configuration for vrfId[0]*/
    prvTgfBasicIpv6UcRoutingRouteConfigurationSet(prvUtfVrfId, GT_FALSE);

    /* Generate traffic */
    prvTgfBasicIpv6UcRoutingTrafficGenerate(GT_FALSE,GT_TRUE,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* Restore configuration */
    prvTgfBasicIpv6UcRoutingConfigurationRestore(prvUtfVrfId, GT_FALSE);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv6 unicast routing when Sip Lookup is Disable:
    configure VLAN, FDB entries;
    configure UC Routing entries;
    add IPv6 UC prefix to LPM DB with exact packet's DIP match;
    send IPv6 UC traffic and verify expected routed traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfBasicIpv6UcRoutingSipLookupDisable)
{
/*
    1. Set Base configuration
    2. Set Route configuration for vrfId[0]
    3. Generate traffic
    4. Restore configuration
    5. Select random vrfId
    6. Set Base configuration
    7. Set Route configuration for vrfId[random]
    8. Generate traffic
    9. Restore configuration
*/

    GT_U32      prvUtfVrfId = 0;


    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_LION2_E );

    PRV_TGF_SKIP_PASS_OK_ON_FULL_LPM_MEM_IN_GM_MAC(UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Set Base configuration */
    prvTgfBasicIpv6UcRoutingBaseConfigurationSet();

    /* Set Route configuration for vrfId[0]*/
    prvTgfBasicIpv6UcRoutingRouteConfigurationSet(prvUtfVrfId, GT_FALSE);

    /*  disable SIP Lookup */
    prvTgfBasicIpv6LpmConfigurationSet(prvTgfPortsArray[0], GT_FALSE, GT_FALSE);

    prvTgfBasicIpv6UcRoutingRouteSipLookupConfigurationSet(prvUtfVrfId);

    /* Generate traffic - expected traffic */
    prvTgfBasicIpv6UcRoutingTrafficGenerate(GT_FALSE,GT_TRUE,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /*  enable SIP Lookup */
    prvTgfBasicIpv6LpmConfigurationSet(prvTgfPortsArray[0], GT_FALSE, GT_TRUE);

    /* Generate traffic - expected NO traffic */
    prvTgfBasicIpv6UcRoutingTrafficGenerate(GT_FALSE,GT_FALSE,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* Restore configuration */
    prvTgfBasicIpv6UcRoutingConfigurationRestore(prvUtfVrfId, GT_FALSE);



    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    prvTgfBasicIpv6UcRoutingBaseConfigurationSet();

    /* Set Route configuration for vrfId[random] */
    prvTgfBasicIpv6UcRoutingRouteConfigurationSet(prvUtfVrfId, GT_FALSE);

    /*  disable SIP Lookup */
    prvTgfBasicIpv6LpmConfigurationSet(prvTgfPortsArray[0], GT_FALSE, GT_FALSE);

    prvTgfBasicIpv6UcRoutingRouteSipLookupConfigurationSet(prvUtfVrfId);

    /* Generate traffic - expected traffic */
    prvTgfBasicIpv6UcRoutingTrafficGenerate(GT_FALSE,GT_TRUE,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /*  enable SIP Lookup */
    prvTgfBasicIpv6LpmConfigurationSet(prvTgfPortsArray[0], GT_FALSE, GT_TRUE);

    /* Generate traffic - expected NO traffic */
    prvTgfBasicIpv6UcRoutingTrafficGenerate(GT_FALSE,GT_FALSE,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* Restore configuration */
    prvTgfBasicIpv6UcRoutingConfigurationRestore(prvUtfVrfId, GT_FALSE);

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv6 unicast routing:
    configure VLAN, FDB entries;
    configure UC Routing entries;
    add IPv6 UC prefix to LPM DB with exact packet's DIP match;
    set egress port to be DSA/eDSA tagged;
    send IPv6 UC traffic and verify expected routed traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfBasicIpv6UcRoutingEgrCascadePort)
{
/*
    For egress ports set to be with DSA/eDSA tag:
    1. Set Base configuration
    2. Set Route configuration for vrfId[0]
    3. Generate traffic
    4. Restore configuration
*/
    GT_U32      prvUtfVrfId = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_LION2_E);

    PRV_TGF_SKIP_PASS_OK_ON_FULL_LPM_MEM_IN_GM_MAC(UTF_CPSS_PP_ALL_SIP6_CNS);

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        GT_STATUS   rc;
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   3,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }


        /* enable substitution of VLAN tag with DSA tag cascade egress port */
    prvTgfBasicIpv6UcRoutingTestAsDsa(TGF_DSA_2_WORD_TYPE_E);
    /* Set Base configuration */
    prvTgfBasicIpv6UcRoutingBaseConfigurationSet();

    /* Set Route configuration for vrfId[0]*/
    prvTgfBasicIpv6UcRoutingRouteConfigurationSet(prvUtfVrfId, GT_FALSE);

    /* Generate traffic */
    prvTgfBasicIpv6UcRoutingTrafficGenerate(GT_FALSE,GT_TRUE,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* enable substitution of VLAN tag with eDSA tag cascade egress port */
    prvTgfBasicIpv6UcRoutingTestAsDsa(TGF_DSA_4_WORD_TYPE_E);

    /* Generate traffic */
    prvTgfBasicIpv6UcRoutingTrafficGenerate(GT_FALSE,GT_TRUE,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* Restore configuration */
    prvTgfBasicIpv6UcRoutingConfigurationRestore(prvUtfVrfId, GT_FALSE);

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv4 multicast routing:
    configure VLAN, FDB entries;
    configure MC Routing entries and prefixes;
    send IPv4 MC traffic and verify to get expected routed traffic.
*/
UTF_TEST_CASE_MAC(tgfBasicIpv4McRouting)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    prvTgfBasicIpv4McRoutingBaseConfigurationSet();

    prvTgfBasicIpv4McRoutingRouteConfigurationSet();

    prvTgfBasicIpv4McRoutingTrafficGenerate(GT_TRUE);

    prvTgfBasicIpv4McRoutingConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv4 multicast routing:
    configure VLAN, FDB entries;
    configure MC Routing entries and prefixes;
    send IPv4 MC traffic and verify packets are silent drop .
*/
UTF_TEST_CASE_MAC(tgfBasicIpv4McRoutingMllSilentDrop)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_L2_MLL_CHECK_MAC(prvTgfDevNum);

    prvTgfBasicIpv4McRoutingBaseConfigurationSet();

    prvTgfBasicIpv4McRoutingRouteConfigurationMllSilentDropSet(GT_FALSE,65);

    prvTgfBasicIpv4McRoutingMllSilentDropTrafficGenerate(GT_TRUE);

    prvTgfBasicIpv4McRoutingRouteConfigurationMllSilentDropSet(GT_TRUE, 64);

    prvTgfBasicIpv4McRoutingMllSilentDropTrafficGenerate(GT_FALSE);

    prvTgfBasicIpv4McRoutingConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv4 multicast routing:
    configure VLAN, FDB entries;
    configure MC Routing entries and prefixes;
    send IPv4 MC traffic and verify to get expected routed traffic.

    Set LPM prefixes with prefixLen=15 (for creating bucket compress #2 on second octet)
    224.170.x.x, 224.172.x.x, 224.174.x.x, 224.176.x.x, 224.178.x.x, 224.180.x.x, 224.182.x.x, 224.184.x.x
    Transmit IPv4 MC with prefix match
*/
UTF_TEST_CASE_MAC(tgfBasicIpv4McRoutingDifferentPrefixLength)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E | UTF_CPSS_PP_E_ARCH_CNS);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    prvTgfBasicIpv4McRoutingBaseConfigurationSet();

    prvTgfBasicIpv4McRoutingRouteConfigurationSet();

    prvTgfBasicIpv4McRoutingRouteConfigurationDifferentPrefixLengthSet();

    prvTgfBasicIpv4McRoutingTrafficDifferentPrefixLengthGenerate();

    prvTgfBasicIpv4McRoutingConfigurationDifferentPrefixLengthRestore();

    prvTgfBasicIpv4McRoutingConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test default IPv4 multicast routing for multi hemisphere devices:
    define ports for multi hemisphere devices - tx\rx ports on diff hemispheres;
    configure VLAN, FDB entries;
    configure MC Routing entries and prefixes;
    send IPv4 MC traffic and verify to get expected routed traffic
    and expected flooded traffic.
*/
UTF_TEST_CASE_MAC(tgfBasicIpv4McRoutingForDualHwDev)
{
    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        prvUtfSkipTestsSet();
        return;
    }

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_CPSS_PP_E_ARCH_CNS | UTF_PUMA_E);

    /* this test is supported only for Lion2_B0 and above */
    if((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)&&
       (PRV_CPSS_PP_MAC(prvTgfDevNum)->revision == 0))
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfBasicIpv4McRoutingForDualHwDevConfigurationInit();

    prvTgfBasicIpv4McRoutingBaseConfigurationSet();

    prvTgfBasicIpv4McRoutingRouteConfigurationSet();

    prvTgfBasicIpv4McRoutingTrafficGenerate(GT_TRUE);

    prvTgfBasicIpv4McRoutingConfigurationRestore();

    prvTgfBasicIpv4McRoutingForDualHwDevConfigurationRestore();

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv6 multicast routing:
    configure VLAN, FDB entries;
    configure MC Routing entries and prefixes;
    send IPv6 MC traffic and verify to get expected routed traffic.
*/

UTF_TEST_CASE_MAC(tgfBasicIpv6McRouting)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    UTF_SIP6_GM_NOT_READY_SKIP_MAC("SIP6 ipv6 mc full address yet not supported in GM");

    prvTgfBasicIpv6McRoutingBaseConfigurationSet();

    prvTgfBasicIpv6McRoutingRouteConfigurationSet();

    prvTgfBasicIpv6McRoutingTrafficGenerate(PRV_TGF_DEFAULT_BOOLEAN_CNS);

    prvTgfBasicIpv6McRoutingConfigurationRestore();
}

/* AUTODOC: IPv6 MC Scope Checking:
        - IPv6 Routers are required to perform address scope checking and filter
          local traffic from crossing over to external interfaces.
        - IPv6 Scope checking can be enabled/disabled on a per-Route entry basis by configuring the Unicast
          or Multicast Route entry <Enable IPv6 Scope Checking> field accordingly.
        - Scope checking involves assigning to the packet Source and Destination Address "Scope Level"
          and "Site ID", and then using these assignments to form an index into the IPv6 Unicast or Multicast
          Scope Command table whose command enforces the scope check.
*/

UTF_TEST_CASE_MAC(tgfBasicIpv6McRoutingScopeChecking)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);
    UTF_SIP6_GM_NOT_READY_SKIP_MAC("SIP6 ipv6 mc full address yet not supported in GM");

    prvTgfBasicIpv6McRoutingBaseConfigurationSet();

    prvTgfBasicIpv6McRoutingRouteConfigurationSet();

    prvTgfBasicIpv6McRoutingScopeCheckingConfigurationSave();

    /* AUTODOC: basic IPv6 MC to verify the board configuration with traffic */
    /* expect traffic */
    prvTgfBasicIpv6McRoutingTrafficGenerate(GT_FALSE);

    /* AUTODOC: "Scope Checking" is Enabled.
                The Group (Destination) SiteId and the Source SiteId are both Internals.
                As a result,  the traffic is NOT Crossing between different types of SiteId's.

                The Command for this case is Route under the following 3 conditions:
                1. The Address Source Scope is Global.
                2. The Address Dest. Scope is Global.
                3. The traffic does Not Crossed between 2 diff. SiteId's (BorderCross=False) */
    prvTgfBasicIpv6McRoutingScopeCheckingConfigurationSet(CPSS_IP_SITE_ID_INTERNAL_E, CPSS_IP_SITE_ID_INTERNAL_E,
                                                          CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E, GT_FALSE, CPSS_PACKET_CMD_ROUTE_E);
    /* expect traffic - command is ROUTE */
    prvTgfBasicIpv6McRoutingTrafficGenerate(GT_FALSE);

    /* AUTODOC: "Scope Checking" is Enabled.
                The Group (Destination) SiteId and the Source SiteId are both Internals.
                As a result,  the traffic is NOT Crossing between different types of SiteId's.

                The Command for this case is Drop under the following 3 conditions:
                1. The Address Source Scope is Global.
                2. The Address Dest. Scope is Global.
                3. The traffic does Not Crossed between 2 diff. SiteId's (BorderCross=False) */
    prvTgfBasicIpv6McRoutingScopeCheckingConfigurationSet(CPSS_IP_SITE_ID_INTERNAL_E, CPSS_IP_SITE_ID_INTERNAL_E,
                                                          CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E, GT_FALSE, CPSS_PACKET_CMD_DROP_HARD_E);
    /* expect No traffic - command is DROP */
    prvTgfBasicIpv6McRoutingTrafficGenerate(GT_TRUE);


    /* AUTODOC: "Scope Checking" is Enabled.
                The Group (Destination) SiteId and the Source SiteId are different.
                As a result,  the traffic is Crossing between different types of SiteId's.

                The Command for this case is Route under the following 3 conditions:
                1. The Address Source Scope is Global.
                2. The Address Dest. Scope is Global.
                3. The traffic Crossed between 2 diff. SiteId's (BorderCross=True) */
    prvTgfBasicIpv6McRoutingScopeCheckingConfigurationSet(CPSS_IP_SITE_ID_EXTERNAL_E, CPSS_IP_SITE_ID_INTERNAL_E,
                                                          CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E, GT_TRUE, CPSS_PACKET_CMD_ROUTE_E);
    /* expect traffic - command is ROUTE */
    prvTgfBasicIpv6McRoutingTrafficGenerate(GT_FALSE);

    prvTgfBasicIpv6McRoutingConfigurationRestore();

    prvTgfBasicIpv6McRoutingScopeCheckingConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv4 unicast routing and Age Bit updates:
    configure VLAN, FDB entries;
    configure UC Routing entries;
    add IPv4 UC prefix to LPM DB with exact packet's DIP match;
    send IPv4 UC traffic and verify to get expected routed traffic;
    read nextHop table age bits entry and verify Age Bit is set to 1.
*/
UTF_TEST_CASE_MAC(tgfBasicIpv4UcRoutingCheckAgeBit)
{
/*
    1. set port configuration if needed
    2. Set Base configuration
    3. Set Route configuration
    4. Generate traffic
    5. Check Age Bit updated
    6. Restore port configuration if needed
    7. Restore configuration
*/
    GT_U32   prvUtfVrfId = 0;

    /* select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);

    if(prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_PUMA_E))
    {
        /* Set Base configuration */
        prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[0]);

        /* Set Route configuration */
        prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 1,
                                                      prvTgfPortsArray[2], GT_FALSE,
                                                      GT_FALSE, GT_FALSE);

        /* Generate traffic from port 18 to port 36 */
        prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[1], prvTgfPortsArray[2], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS);

             /* Check age bit was updated */
        prvTgfBasicIpv4UcRoutingAgeBitGet();

        /* Restore configuration */
        prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 1, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);
    }
    else
    {
        /* Set Base configuration */
        prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

        /* Set Route configuration */
        prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                      prvTgfPortsArray[3], GT_FALSE,
                                                      GT_FALSE, GT_FALSE);

        /* Generate traffic */
        prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS);

        /* Check age bit was updated */
        prvTgfBasicIpv4UcRoutingAgeBitGet();

        /* Restore configuration */
        prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);

    }
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv4 unicast routing to destination ePort:
    configure VLAN, FDB entries;
    configure UC Routing entries with ePort;
    add IPv4 UC prefix to LPM DB with exact packet's DIP match;
    map ePort to physical nextHopPort;
    send IPv4 UC traffic and verify expected routed traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfBasicIpv4UcRoutingUseEPort)
{
    GT_U32      notAppFamilyBmp;
    GT_U32      prvUtfVrfId = 0;

    /* select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* enable ePort use */
    prvTgfBasicIpv4UcRoutingUseEPortEnableSet(GT_TRUE);

    /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

    /* Set Route configuration */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                  prvTgfPortsArray[3], GT_FALSE,
                                                  GT_FALSE, GT_FALSE);

    /* Generate traffic */
    prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS);

    /* Restore configuration */
    prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);

    /* disable ePort use */
    prvTgfBasicIpv4UcRoutingUseEPortEnableSet(GT_FALSE);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Unicast Reverse Path Forwarding (uRPF) Check
    The uRPF feature is a security mechanism to detect Unicast packets with malformed or spoofed IP
    source address.
    The uRPF check ensures that the router forwards a Unicast packet only if it is received on the
    correct upstream interface. This is done by comparing selected fields (according to the uRPF mode)
    of the ingress interface the packet was received on, with the same fields in the SIP address next-hop
    interface.
    The RPF check can be performed in one of the following modes:
        . eVLAN-Based uRPF mode. In this mode, the packet assigned eVLAN is compared to the SIP
          next-hop eVLAN assignment.
        . ePort-Based uRPF mode. In this mode, the packet assigned source {Device, ePort} is
          compared to the SIP next-hop {Device, ePort} assignment.
        . Loose uRPF mode. In this mode, the SIP next-hop packet command assignment is checked to
          be different than a DROP command
*/
UTF_TEST_CASE_MAC(tgfBasicIpv4UcRoutingUnicastRPF)
{
    GT_U32      notAppFamilyBmp;
    GT_U32      prvUtfVrfId = 0;

    /* select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

    /* Set Route configuration */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                  prvTgfPortsArray[3], GT_FALSE,
                                                  GT_FALSE, GT_FALSE);
    /* enable uRPF */
    prvTgfBasicIpv4UcRoutingUnicastRpfEnableSet(prvUtfVrfId, PRV_TGF_EVLAN_BASED_URPF_MODE_E);  /* eVLAN-Based uRPF */

    prvTgfBasicIpv4UcRoutingUnicastRpfEnableSet(prvUtfVrfId, PRV_TGF_EPORT_BASED_URPF_MODE_E);  /* ePort-Based uRPF */

    prvTgfBasicIpv4UcRoutingUnicastRpfEnableSet(prvUtfVrfId, PRV_TGF_LOOSE_URPF_MODE_E);        /* Loose uRPF */

    /* Restore configuration */
    prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Loose Mode Types for uRPF check
*/
UTF_TEST_CASE_MAC(tgfBasicIpv4UcRoutingUnicastRPFLooseModeType)
{
    GT_U32      notAppFamilyBmp;
    GT_U32      prvUtfVrfId = 0;

    /* select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Exclude non eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* Supported by eArch SIP 5.25 devices */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_FALCON_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

    /* Set Route configuration */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                  prvTgfPortsArray[3], GT_FALSE,
                                                  GT_FALSE, GT_FALSE);

    /* Test PRV_TGF_URPF_LOOSE_MODE_TYPE_0_E - uRPF check fails if the SIP Next Hop <Packet Command> is HARD/SOFT DROP, TRAP */
    prvTgfBasicIpv4UcRoutingUnicastRpfLooseModeTypeSet(prvUtfVrfId, PRV_TGF_URPF_LOOSE_MODE_TYPE_0_E);

    /* Test PRV_TGF_URPF_LOOSE_MODE_TYPE_1_E - uRPF check fails if the SIP Next Hop <Packet Command> is HARD/SOFT DROP */
    prvTgfBasicIpv4UcRoutingUnicastRpfLooseModeTypeSet(prvUtfVrfId, PRV_TGF_URPF_LOOSE_MODE_TYPE_1_E);

    /* Restore configuration */
    prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Unicast SIP/SA Check
    An additional counter measure to prevent source IP spoofing or ARP spoofing
    is to verify that the packet Layer 2 MAC Source Address (SA) corresponds
    to the ARP MAC address of the packet Layer 3 Source IP (SIP).
*/
UTF_TEST_CASE_MAC(tgfBasicIpv4UcRoutingUnicastSipSa)
{
    GT_U32      notAppFamilyBmp;
    GT_U32      prvUtfVrfId = 0;

    /* select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

    /* Set Route configuration */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                  prvTgfPortsArray[3], GT_FALSE,
                                                  GT_FALSE, GT_FALSE);
    /* enable uSIP SA */
    prvTgfBasicIpv4UcRoutingUnicastSipSaEnableSet(prvUtfVrfId);

    /* Restore configuration */
    prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IpLpmDBCapacityUpdate:
    Add IPv4 exact match prefix (like 1.1.1.3 with prefix length 32)
    Verify traffic to this prefix and also to the default prefix
    Update LPM DB capacity with the following params:
        indexesRange: 0 - 9
        tcamLpmManagerCapcityCfg:
            numOfIpv4Prefixes: 4
            numOfIpv4McSourcePrefixes: 4
            numOfIpv6Prefixes: 4
    This should initiate moving of all existing prefixes.
    Now verify traffic again to both prefixes.
*/
UTF_TEST_CASE_MAC(tgfIpLpmDBCapacityUpdate)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);

    /* currently only PBR mode is supported */
    if (GT_FALSE == prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfIpLpmDBCapacityUpdateBaseConfigurationSet();

    prvTgfIpLpmDBCapacityUpdateRouteConfigurationSet();

    PRV_UTF_LOG0_MAC("======= Generating Traffic - matched prefix =======\n");
    prvTgfIpLpmDBCapacityUpdateTrafficGenerate(GT_TRUE);

    PRV_UTF_LOG0_MAC("======= Generating Traffic - default prefix =======\n");
    prvTgfIpLpmDBCapacityUpdateTrafficGenerate(GT_FALSE);

    PRV_UTF_LOG0_MAC("======= IpLpmDBCapacityUpdate =======\n");
    prvTgfIpLpmDBCapacityUpdateRouteConfigurationUpdate();

    PRV_UTF_LOG0_MAC("======= Generating Traffic - matched prefix =======\n");
    prvTgfIpLpmDBCapacityUpdateTrafficGenerate(GT_TRUE);

    PRV_UTF_LOG0_MAC("======= Generating Traffic - default prefix =======\n");
    prvTgfIpLpmDBCapacityUpdateTrafficGenerate(GT_FALSE);

    prvTgfIpLpmDBCapacityUpdateConfigurationRestore();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv4 unicast ECMP routing:
    configure VLAN, FDB, ARP entries;
    configure UC Routing entries:
       The rules are following:
         pathIndex          = [0..numOfEntries - 1]
         baseRouteEntryIndex = routeEntryBaseIdx + pathIndex
         nextHopARPPointer   = arpEntryBaseIdx   + pathIndex
         countSet            = pathIndex % 4
       IP packets will use following rules:
         sip   = sipBase   + pathIndex;
         chsum = chsumBase - pathIndex;
         daMac = daMacBase + pathIndex;
    add IPv4 UC prefix to to LPM DB exact match packet's DIP;
    send IPv4 UC traffic with the following modifications:
        increase SIP with step calculating;
        packetHash - 3 last bits of SIP (when DIP is the same).
    and verify to get expected routed traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfBasicIpv4UcEcmpRouting)
{

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);

/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/

    /* Set Base configuration */
    prvTgfBasicIpv4UcEcmpRoutingBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfBasicIpv4UcEcmpRoutingRouteConfigurationSet(GT_FALSE,GT_FALSE);

    /* Generate traffic */
    prvTgfBasicIpv4UcEcmpRoutingTrafficGenerate(GT_FALSE);

    /* Restore configuration */
    prvTgfBasicIpv4UcEcmpRoutingConfigurationRestore(PRV_TGF_DEFAULT_BOOLEAN_CNS);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv4 unicast ECMP routing:
    configure VLAN, FDB, ARP entries;
    configure UC Routing entries:
       The rules are following:
         pathIndex          = [0..numOfEntries - 1]
         baseRouteEntryIndex = routeEntryBaseIdx + pathIndex
         nextHopARPPointer   = arpEntryBaseIdx   + pathIndex
         countSet            = pathIndex % 4
       IP packets will use following rules:
         sip   = sipBase   + pathIndex;
         chsum = chsumBase - pathIndex;
         daMac = daMacBase + pathIndex;
    add IPv4 UC prefix to to LPM DB exact match packet's DIP;
    send IPv4 UC traffic with the following modifications:
        increase SIP with step calculating;
        packetHash - 3 last bits of SIP (when DIP is the same).
    and verify to get expected routed traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfBasicIpv4UcEcmpRoutingRandomEnable)
{
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/

    /* Set Base configuration */
    prvTgfBasicIpv4UcEcmpRoutingBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfBasicIpv4UcEcmpRoutingRouteConfigurationSet(GT_TRUE,GT_FALSE);

    /* Generate traffic */
    prvTgfBasicIpv4UcEcmpRoutingTrafficGenerate(GT_TRUE);

    /* Restore configuration */
    prvTgfBasicIpv4UcEcmpRoutingConfigurationRestore(PRV_TGF_DEFAULT_BOOLEAN_CNS);
}

UTF_TEST_CASE_MAC(tgfBasicIpv4UcEcmpRoutingIndirectAccess)
{


    /* Supported by Aldrin 2  device only */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (GT_U32)(~ (UTF_ALDRIN2_E | UTF_FALCON_E)));

/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_SKIP_PASS_OK_LONG_TEST_GM_MAC(UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Set Base configuration */
    prvTgfBasicIpv4UcEcmpRoutingBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfBasicIpv4UcEcmpRoutingRouteConfigurationSet(GT_FALSE,GT_TRUE);

    /* Generate traffic */
    prvTgfBasicIpv4UcEcmpRoutingTrafficGenerate(GT_TRUE);

    /* Restore configuration */
    prvTgfBasicIpv4UcEcmpRoutingConfigurationRestore(PRV_TGF_DEFAULT_BOOLEAN_CNS);
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv4 unicast ECMP routing with expected hashing index match:
    configure VLAN, FDB, ARP entries;
    configure UC Routing entries:
       The rules are following:
         pathIndex          = [0..numOfEntries - 1]
         baseRouteEntryIndex = routeEntryBaseIdx + pathIndex
         nextHopARPPointer   = arpEntryBaseIdx   + pathIndex
         countSet            = pathIndex % 4
       IP packets will use following rules:
         sip   = sipBase   + pathIndex;
         chsum = chsumBase - pathIndex;
         daMac = daMacBase + pathIndex;
    add IPv4 UC prefix to to LPM DB exact match packet's DIP;
    send IPv4 UC traffic with the following modifications:
        increase SIP with step calculating;
        packetHash - 3 last bits of SIP (when DIP is the same).
    and verify to get expected routed traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfBasicIpv4UcEcmpRoutingHashCheck)
{
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/

    /* Set Base configuration */
    prvTgfBasicIpv4UcEcmpRoutingBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfBasicIpv4UcEcmpRoutingHashCheckRouteConfigurationSet(GT_FALSE);

    /* Generate traffic */
    prvTgfBasicIpv4UcEcmpRoutingHashCheckTrafficGenerate(GT_FALSE);

    /* Restore configuration */
    prvTgfBasicIpv4UcEcmpRoutingConfigurationRestore(GT_TRUE);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv4 unicast low level routing:
    configure VLAN, FDB entries;
    configure UC Routing entries and LTT Entries with ECMP route type;
    add IPv4 UC prefix to Router Tcam to TRAP default packets to CPU;
    add IPv4 UC prefix to Router Tcam to ROUTE packets with exact DIP match;
    send IPv4 UC traffic;
    vefify to get expected routed traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfBasicIpv4UcLowLevelRouting)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/
    GT_U32      appFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_SUPPORTED_FAMILY_GET_MAC(&appFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, appFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);

    /* Set Base configuration */
    prvTgfBasicIpv4UcLowLevelRoutingBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfBasicIpv4UcLowLevelRoutingRouteConfigurationSet();

    /* Generate traffic */
    prvTgfBasicIpv4UcLowLevelRoutingTrafficGenerate();

    /* Restore configuration */
    prvTgfBasicIpv4UcLowLevelRoutingConfigurationRestore();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test default IPv4 multicast routing:
    configure VLAN, FDB entries;
    sends MC IPv4 packet without adding prefix;
    check that packet sent according to defaults config;
    add IPv4 MC prefix to LPM DB;
    send MC IPv4 packets;
    check that packet sent accordingly created config;
    send MC IPv4 packets with different Src;
    check that packet sent according to defaults config.
*/
UTF_TEST_CASE_MAC(tgfDefaultIpv4McRouting)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    prvTgfDefaultIpv4McRoutingBaseConfigurationSet();

    prvTgfDefaultIpv4McRoutingDefaultLttRouteConfigurationSet();

    prvTgfDefaultIpv4McRoutingDefaultTrafficGenerate();

    prvTgfDefaultIpv4McRoutingAdditionalRouteConfigurationSet(0);

    prvTgfDefaultIpv4McRoutingAdditionalTrafficGenerate(0);

    prvTgfDefaultIpv4McRoutingMllBridgeEnableTraffic();

    prvTgfDefaultIpv4McRoutingConfigurationRestore();
}



/*----------------------------------------------------------------------------*/
/* AUTODOC: Test default IPv4 multicast routing:
    configure VLAN, FDB entries;
    rewrite default mc nexthop entry with ROUTE cmd, pointed to MLL 0;
    create MLL 0 and configure it with output interface on port 58;
    sends MC IPv4 packet without adding prefix;
    check that packet sent according to defaults config;
    create new mc route entry with index 10 in nextHop table pointed to new MLL 5;
    create new MLL 5 with utput interface on port 18;
    add new IPv4 MC default prefix to LPM DB : 224/4 pointed to mc route entry with index 10;
    send MC IPv4 packets;
    check that packet sent according to defaults config.
*/
UTF_TEST_CASE_MAC(tgfIpv4McRoutingDefaultOverride)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    prvTgfDefaultIpv4McRoutingBaseConfigurationSet();

    prvTgfDefaultIpv4McRoutingDefaultLttRouteConfigurationSet();

    prvTgfDefaultIpv4McRoutingDefaultTrafficGenerate();

    prvTgfDefaultIpv4McRoutingAdditionalRouteConfigurationSet(1);

    prvTgfDefaultIpv4McRoutingAdditionalTrafficGenerate(1);

    prvTgfDefaultIpv4McRoutingConfigurationRestore();
}




/*----------------------------------------------------------------------------*/
/* AUTODOC: Test default IPv4 multicast routing for multi hemisphere devices:
    define ports for multi hemisphere devices - tx\rx ports on diff hemispheres;
    configure VLAN, FDB entries;
    sends MC IPv4 packet without adding prefix;
    check that packet sent according to defaults config;
    add IPv4 MC prefix to LPM DB;
    send MC IPv4 packets;
    check that packet sent accordingly created config;
    send MC IPv4 packets with different Src;
    check that packet sent according to defaults config;
    restore default test ports.
*/
UTF_TEST_CASE_MAC(tgfDefaultIpv4McRoutingForDualHwDev)
{
    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        prvUtfSkipTestsSet();
        return;
    }

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                        UTF_LION_E | UTF_PUMA_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    prvTgfDefaultIpv4McRoutingForDualHwDevConfigurationInit();

    prvTgfDefaultIpv4McRoutingBaseConfigurationSet();

    prvTgfDefaultIpv4McRoutingDefaultLttRouteConfigurationSet();

    prvTgfDefaultIpv4McRoutingDefaultTrafficGenerate();

    prvTgfDefaultIpv4McRoutingAdditionalRouteConfigurationSet(0);

    prvTgfDefaultIpv4McRoutingAdditionalTrafficGenerate(0);

    prvTgfDefaultIpv4McRoutingConfigurationRestore();

    prvTgfDefaultIpv4McRoutingForDualHwDevConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test default IPv6 multicast routing:
    configure VLAN, FDB entries;
    sends MC IPv6 packet without adding prefix;
    check that packet sent according to defaults config;
    add IPv6 MC prefix to LPM DB;
    send MC IPv6 packets;
    check that packet sent accordingly created config;
    send MC IPv6 packets with different Src;
    check that packet sent according to defaults config.
*/
UTF_TEST_CASE_MAC(tgfDefaultIpv6McRouting)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    UTF_SIP6_GM_NOT_READY_SKIP_MAC("SIP6 ipv6 mc full address yet not supported in GM");

    prvTgfDefaultIpv6McRoutingBaseConfigurationSet();

    prvTgfDefaultIpv6McRoutingDefaultLttRouteConfigurationSet();

    prvTgfDefaultIpv6McRoutingDefaultTrafficGenerate();

    prvTgfDefaultIpv6McRoutingAdditionalRouteConfigurationSet(0);

    prvTgfDefaultIpv6McRoutingAdditionalTrafficGenerate(0);

    prvTgfDefaultIpv6McRoutingConfigurationRestore();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test default IPv6 multicast routing:
    configure VLAN, FDB entries;
    rewrite default mc nexthop entry with ROUTE cmd, pointed to MLL 0;
    create MLL 0 and configure it with output interface on port 58;
    sends MC IPv6 packet without adding prefix;
    check that packet sent according to defaults config;
    create new mc route entry with index 10 in nextHop table pointed to new MLL 5;
    create new MLL 5 with utput interface on port 18;
    add new IPv6 MC default prefix to LPM DB : FF/8 pointed to mc route entry with index 10;
    send MC IPv6 packets;
    check that packet sent according to defaults config.
*/

UTF_TEST_CASE_MAC(tgfIpv6McRoutingDefaultOverride)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    prvTgfDefaultIpv6McRoutingBaseConfigurationSet();

    prvTgfDefaultIpv6McRoutingDefaultLttRouteConfigurationSet();

    prvTgfDefaultIpv6McRoutingDefaultTrafficGenerate();

    prvTgfDefaultIpv6McRoutingAdditionalRouteConfigurationSet(1);

    prvTgfDefaultIpv6McRoutingAdditionalTrafficGenerate(1);

    prvTgfDefaultIpv6McRoutingConfigurationRestore();
}



/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv4 unicast to virtual router:
    configure VLAN, FDB entries;
    enable IPv4 UC routing on port and VLAN;
    create UC Route Entry with CMD_TRAP_TO_CPU as default IPv4 UC;
    create UC Route Entry with CMD_TRAP_TO_CPU as default IPv4 MC;
    create Virtual Router with created defaults;
    create UC Route Entry;
    add IPv4 UC prefix to LPM DB with exact packet's DIP match;
    send IPv4 UC traffic;
    vefify to get expected routed traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfBasicIpv4Uc2VirtualRouter)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/

    GT_U32      prvUtfVrfId = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    prvTgfBasicIpv4Uc2VirtualRouterBaseConfigurationSet(prvUtfVrfId);

    /* Set Route configuration */
    prvTgfBasicIpv4Uc2VirtualRouterRouteConfigurationSet();

    /* Generate traffic */
    prvTgfBasicIpv4Uc2VirtualRouterTrafficGenerate();

    /* Restore configuration */
    prvTgfBasicIpv4Uc2VirtualRouterConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IPv4 unicast to multiple virtual routers:
    configure VLAN, FDB entries;
    enable IPv4 UC routing on port and VLAN;
    create 1959 of Virtual Routers;
    create one prefix with exact packet's DIP match per VR in the LPM trie;
    create route entry for every prefix in the Next_Hop Table.
    create route entry for default route of every VR in the Next_Hop Table.
    send IPv4 UC traffic to device: two packets for every VR_ID, first packet
    to be processed by default route, 2nd - by a route with matching prefix;
    vefify to get expected routed traffic on appropriate port depending of VR.
*/
UTF_TEST_CASE_MAC(tgfIpv4Uc2MultipleVr)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* skip this test for all families. */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /* skip test if VPLS mode enabled for  xCat3 devices */
    if (prvUtfIsVplsModeUsed(prvTgfDevNum))
    {
        PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E, CPSS-6203);
    }


    /* Set Base configuration */
    prvTgfIpv4Uc2MultipleVrBaseConfigurationSet();

    /* Set Route configuration - add prefixes only after creation of all VRs */
    prvTgfIpv4Uc2MultipleVrRouteConfigurationSet(GT_TRUE);

    /* Generate traffic */
    prvTgfIpv4Uc2MultipleVrTrafficGenerate();

    /* Restore configuration */
    prvTgfIpv4Uc2MultipleVrConfigurationRestore();


    /* Set Base configuration */
    prvTgfIpv4Uc2MultipleVrBaseConfigurationSet();

    /* Set Route configuration - add prefixes within creation of all VRs */
    prvTgfIpv4Uc2MultipleVrRouteConfigurationSet(GT_FALSE);

    /* Generate traffic */
    prvTgfIpv4Uc2MultipleVrTrafficGenerate();

    /* Restore configuration */
    prvTgfIpv4Uc2MultipleVrConfigurationRestore();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPV4 2 shared virtual Router:
    configure VLAN, FDB, VIDX entries;
    enable IPv4 UC\MC routing on port and VLAN;
    create UC Route Entry with CMD_TRAP_TO_CPU as default IPv4 UC;
    create UC Route Entry with CMD_TRAP_TO_CPU as default IPv4 MC;
    create Shared Virtual Router with created defaults;
    create 2 non-Shared Virtual Routers;
    bind VLANs into created Virtual Routers;
    create UC Route Entry;
    add IPv4 UC prefix to LPM DB with exact packet's DIP match;
    create 2 MC Route Entries and MLL;
    add 2 IPv4 MC prefixes (S, G) with created VRs to LPM DB;
    send 3 IPv4 UC packets with modified VLAN Tag;
    vefify to get expected routed traffic on nextHop port;
    send 2 IPv4 MC packets with modified VLAN Tag;
    vefify to get expected routed traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfBasicIpv42SharedVirtualRouter)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E |
                                         UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfBasicIpv42SharedVirtualRouterBaseConfigSet();

    /* Set Route configuration */
    prvTgfBasicIpv42SharedVirtualRouterRouteConfigSet();

    /* Set UC IP Configuration */
    prvTgfBasicIpv42SharedVirtualRouterUcIpConfigSet();

    /* Set MC IP Configuration */
    prvTgfBasicIpv42NonSharedVirtualRouterMcIpConfigSet();

    /* Enable routing mechanism */
    prvTgfBasicIpv42SharedVirtualRouterEnable();

    /* Generate traffic */
    prvTgfBasicIpv42SharedVirtualRouterTrafficGenerate();

    /* Restore configuration */
    prvTgfBasicIpv4Uc2SharedVRConfigRestore();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test complicated IPv4 multicast routing:
    configure VLAN, FDB, VIDX entries;
    send MC IPv4 packet without adding prefix;
    verify that packet is sent according to defaults config;
    add two MC entries;
    send 2 MC IPv4 packets with prefix;
    verify that packets are sent accordingly created config;
    delete second MC entry;
    send 2 MC IPv4 packets with one prefix;
    verify that packets are sent accordingly created config and to the default;
    restore second MC entry;
    send 2 MC IPv4 packets with prefix;
    verify that packets are sent accordingly created config.
*/
UTF_TEST_CASE_MAC(tgfComplicatedIpv4McRouting)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);

    prvTgfComplicatedIpv4McRoutingBaseConfigurationSet();

    prvTgfComplicatedIpv4McRoutingDefaultLttRouteConfigurationSet();

    prvTgfComplicatedIpv4McRoutingDefaultTrafficGenerate();

    prvTgfComplicatedIpv4McRoutingAdditionalRouteConfigurationSet();

    prvTgfComplicatedIpv4McRoutingAdditionalTrafficGenerate();

    prvTgfComplicatedIpv4McRoutingConfigurationRestore();
}



/*----------------------------------------------------------------------------*/
/* AUTODOC: Test UC IPv4 non-exact match prefix:
    configure VLAN, FDB entries;
    set base IP address = 10.153.102.170;
    generate IP address array in the following way:
      IP address is made from Base IP address by inverting one bit;
      inverted bit index is equal to index in IP address array;
    Start loop where prefixLen is incremented from 1 to 31;
    add UC prefix to the LPM DB, where IP prefix is equal to
    base IP and Prefix Length is equal to current iteration number.
    transmit burst of 32 packets, where:
      dst IP addresses are got from generated IP address array;
      payload Data contain modified bit index value;
    check success criteria:
      if modified bit index < prefixLen, packet is expected on port
      if modified bit index >= prefixLen packet is not expected
    delete prefix from the LPM DB.
    End loop.
*/
UTF_TEST_CASE_MAC(prvTgfIpv4NonExactMatchUcRouting)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* Set Base configuration */
    prvTgfIpNonExactMatchRoutingIpv4UcBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfIpNonExactMatchRoutingIpv4UcRouteConfigurationSet();

    /* Generate traffic */
    prvTgfIpNonExactMatchRoutingIpv4UcTrafficGenerate();

    /* Restore configuration */
    prvTgfIpNonExactMatchRoutingIpv4UcConfigurationRestore();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test UC IPv6 non-exact match prefix:
    configure VLAN, FDB entries;
    set base IP address = 0x6545:...:0x3212;
    generate IP address array in the following way:
      IP address is made from Base IP address by inverting one bit;
      inverted bit index is equal to index in IP address array;
    Start loop where prefixLen is incremented from 1 to 127;
    add UC prefix to the LPM DB, where IP prefix is equal to
    base IP and Prefix Length is equal to current iteration number.
    transmit burst of 128 packets, where:
      dst IP addresses are got from generated IP address array;
      payload Data contain modified bit index value;
    check success criteria:
      if modified bit index < prefixLen, packet is expected on port
      if modified bit index >= prefixLen packet is not expected
    delete prefix from the LPM DB.
    End loop.
*/
UTF_TEST_CASE_MAC(prvTgfIpv6NonExactMatchUcRouting)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_DXCH_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E );

    /* Set Base configuration */
    prvTgfIpNonExactMatchRoutingIpv6UcBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfIpNonExactMatchRoutingIpv6UcRouteConfigurationSet();

    /* Generate traffic */
    prvTgfIpNonExactMatchRoutingIpv6UcTrafficGenerate();

    /* Restore configuration */
    prvTgfIpNonExactMatchRoutingIpv6UcConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test MC IPv4 non-exact match prefix:
    configure VLAN, FDB entries;
    set SRC IP address = 10.153.102.170;
    set GRP IP address = 244.1.1.1;
    generate IP address array in the following way:
      IP address is made from Base IP address by inverting one bit;
      inverted bit index is equal to index in IP address array;
    Start loop where prefixLen is incremented from 1 to 31;
    add MC prefix to the LPM DB, where Src IP prefix is equal to
    base IP and Prefix Length is equal to current iteration number;
    GRP IP prefix is equal to base GRP IP address with length 32;
    transmit burst of 32 packets, where:
      SRC IP addresses are got from generated IP address array;
      GRP IP addresses is equal to base GRP IP address;
      payload Data contain modified bit index value;
    check success criteria:
      if modified bit index < prefixLen, packet is expected on port
      if modified bit index >= prefixLen packet is not expected
    delete prefix from the LPM DB.
    End loop.
*/
UTF_TEST_CASE_MAC(prvTgfIpv4NonExactMatchMcRouting)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);

    /* Set Base configuration */
    prvTgfIpNonExactMatchRoutingIpv4McBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfIpNonExactMatchRoutingIpv4McRouteConfigurationSet();

    /* Generate traffic */
    prvTgfIpNonExactMatchRoutingIpv4McTrafficGenerate();

    /* Restore configuration */
    prvTgfIpNonExactMatchRoutingIpv4McConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test MC IPv6 non-exact match prefix:
    configure VLAN, FDB entries;
    set SRC IP address = 0x6545:...:0x3212;
    set GRP IP address = 0xFF01:...:0xEEFF;
    generate IP address array in the following way:
      IP address is made from Base IP address by inverting one bit;
      inverted bit index is equal to index in IP address array;
    Start loop where prefixLen is incremented from 1 to 128;
    add MC prefix to the LPM DB, where Src IP prefix is equal to
    base IP and Prefix Length is equal to current iteration number;
    GRP IP prefix is equal to base GRP IP address with length 32;
    transmit burst of 128 packets, where:
      SRC IP addresses are got from generated IP address array;
      GRP IP addresses is equal to base GRP IP address;
      payload Data contain modified bit index value;
    check success criteria:
      if modified bit index < prefixLen, packet is expected on port
      if modified bit index >= prefixLen packet is not expected
    delete prefix from the LPM DB.
    End loop.
*/
UTF_TEST_CASE_MAC(prvTgfIpv6NonExactMatchMcRouting)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/
    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        return;
    }

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_DXCH_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);

    /* Set Base configuration */
    prvTgfIpNonExactMatchRoutingIpv6McBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfIpNonExactMatchRoutingIpv6McRouteConfigurationSet();

    /* Generate traffic */
    prvTgfIpNonExactMatchRoutingIpv6McTrafficGenerate();

    /* Restore configuration */
    prvTgfIpNonExactMatchRoutingIpv6McConfigurationRestore();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test LPM DB resize (from default --> smaller size):
    Verify basic LPM DB resize functionality by checking the number of
    IPv4 and IPv6 entries that can be added under dynamic resizing (reducing)
    of the IP TCAM size.

    Create external TCAM manager;
    Configure VLAN, FDB entries;
    Update LPM DB and TCAM table to reduce size;
    Enable IPv4\IPv6 UC routing;
    Try to add max possible IPv4\IPv6 UC prefix to LPM DB to fill whole LPM DB;
    Verify that not possible to add more;
    Send IPv4\IPv6 UC traffic and vefify that added prefixes are valid.
*/
UTF_TEST_CASE_MAC(tgfComplicatedPbrIpResizeDefToSmaller)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);

    /* currently only PBR mode is supported */
    if (GT_FALSE == prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfComplicatedPbrIpResizeBaseConfigSet();

    /* Set Route configuration */
    prvTgfComplicatedPbrIpResizeRouteConfigSet();

    /* Generate traffic */
    prvTgfComplicatedPbrIpResizeTrafficGenerate();

    /* Restore configuration */
    prvTgfComplicatedPbrIpResizeConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test LPM DB resize (from Default --> Smaller --> Bigger size):
    Verify basic LPM DB resize functionality by checking the number of IPv4 and
    IPv6 entries that can be added under dynamic resizing
    (reducing / enlarging / reducing) of the IP TCAM size.

    Create external TCAM manager;
    Configure VLAN, FDB entries;
    Enable IPv4\IPv6 UC routing;
    Update LPM DB and TCAM table to reduce size;
    Try to add max possible IPv4\IPv6 UC prefix to LPM DB to fill whole LPM DB;
    Verify that not possible to add more;
    Send IPv4\IPv6 UC traffic and vefify that added prefixes are valid;
    Update LPM DB and TCAM table to enlarge size;
    Try to add additional IPv4\IPv6 UC prefix to LPM DB into increased space;
    Send IPv4\IPv6 UC traffic and vefify that added prefixes are valid.
*/
UTF_TEST_CASE_MAC(tgfIpResizeDefSmallerBigger)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);

    /* currently only PBR mode is supported */
    if (GT_FALSE == prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfIpResizeDefSmallerBiggerBaseConfigSet();

    /* Set Route configuration */
    prvTgfIpResizeDefSmallerBiggerRouteConfigSet(0);

    /* Generate traffic */
    prvTgfIpResizeDefSmallerBiggerTrafficGenerate(0);

    /* Set Route additional configuration */
    prvTgfIpResizeDefSmallerBiggerRouteConfigSet(1);

    /* Generate additional traffic */
    prvTgfIpResizeDefSmallerBiggerTrafficGenerate(1);

    /* Restore configuration */
    prvTgfIpResizeDefSmallerBiggerConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test LPM DB resize (from Default --> Smaller --> Bigger --> Smaller size):
    Verify basic LPM DB resize functionality by checking the number of IPv4 and
    IPv6 entries that can be added under dynamic resizing
    (reducing / enlarging / reducing) of the IP TCAM size.

    Create external TCAM manager;
    Configure VLAN, FDB entries;
    Enable IPv4\IPv6 UC routing;
    Update LPM DB and TCAM table to reduce size;
    Try to add max possible IPv4\IPv6 UC prefix to LPM DB to fill whole LPM DB;
    Verify that not possible to add more;
    Send IPv4\IPv6 UC traffic and vefify that added prefixes are valid;
    Update LPM DB and TCAM table to enlarge size;
    Try to add additional IPv4\IPv6 UC prefix to LPM DB into increased space;
    Send IPv4\IPv6 UC traffic and vefify that added prefixes are valid;
    Delete several prefixes and update LPM DB and TCAM table to reduce size;
    Send IPv4\IPv6 UC traffic and vefify that all prefixes are valid.
*/
UTF_TEST_CASE_MAC(tgfIpResizeDefSmallerBiggerSmaller)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/
    GT_U32  stageIter = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);

    /* currently only PBR mode is supported */
    if (GT_FALSE == prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfIpResizeDefSmallerBiggerBaseConfigSet();

    /* Iterate thru 3 stages */
    for (stageIter = 0; stageIter < 3; stageIter++)
    {
        /* Set Route configuration */
        prvTgfIpResizeDefSmallerBiggerRouteConfigSet(stageIter);

        /* Generate traffic */
        prvTgfIpResizeDefSmallerBiggerTrafficGenerate(stageIter);
    }

    /* Restore configuration */
    prvTgfIpResizeDefSmallerBiggerConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test LPM DB resize (from Default --> Bigger --> Smaller --> Smaller size):
    Verify basic LPM DB resize functionality by checking the number of IPv4 and
    IPv6 entries that can be added under dynamic resizing
    (enlarging / reducing / reducing) of the IP TCAM size.

    Create external TCAM manager;
    Configure VLAN, FDB entries;
    Enable IPv4\IPv6 UC routing;
    Update LPM DB and TCAM table with specific size;
    Try to add max possible IPv4\IPv6 UC prefix to LPM DB to fill whole LPM DB;
    Send IPv4\IPv6 UC traffic and vefify that added prefixes are valid;
    Delete several prefixes and update LPM DB and TCAM table to reduce size;
    Send IPv4\IPv6 UC traffic and vefify that all prefixes are valid;
    Delete several prefixes and update LPM DB and TCAM table to reduce size;
    Send IPv4\IPv6 UC traffic and vefify that all prefixes are valid.
*/
UTF_TEST_CASE_MAC(tgfIpResizeDefBiggerSmallerSmaller)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/
    GT_U32  stageIter = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);

    /* currently only PBR mode is supported */
    if (GT_FALSE == prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfIpResizeDefSmallerBiggerBaseConfigSet();

    /* Iterate thru 3 stages */
    for (stageIter = 1; stageIter < 4; stageIter++)
    {
        /* Set Route configuration */
        prvTgfIpResizeDefSmallerBiggerRouteConfigSet(stageIter);

        /* Generate traffic */
        prvTgfIpResizeDefSmallerBiggerTrafficGenerate(stageIter);
    }

    /* Restore configuration */
    prvTgfIpResizeDefSmallerBiggerConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test LPM DB resize with PCL rules:
    Verify LPM DB resize functionality by checking that old IPv4 and IPv6
    entries and PCL rules do not lost under dynamic resizing of the IP TCAM.

    Create external TCAM manager;
    Configure VLAN, FDB entries;
    Enable IPv4\IPv6 UC routing;
    Update LPM DB and TCAM table to reduce size;
    Try to add max possible IPv4\IPv6 UC prefix to LPM DB to fill whole LPM DB;
    Verify that not possible to add more;
    Send IPv4\IPv6 UC traffic and vefify that added prefixes are valid;
    Add PCL rule to DROP any Not Ipv4 packet with specific DA;
    Update LPM DB and TCAM table to enlarge size;
    Try to add additional IPv4\IPv6 UC prefix to LPM DB into increased space;
    Send IPv4\IPv6 UC traffic and vefify that added prefixes are valid;
    Send ethernet traffic on portIdx 3 and vefify that PCL rule is valid.
*/
UTF_TEST_CASE_MAC(tgfComplicatedPbrIpPclResizeDefToSmaller)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Set Additional Route configuration
    5. Generate Additional traffic
    6. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);

    /* currently only PBR mode is supported */
    if (GT_FALSE == prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfComplicatedPbrIpPclResizeBaseConfigSet();

    /* Set Route configuration */
    prvTgfComplicatedPbrIpPclResizeRouteConfigSet();

    /* Generate traffic */
    prvTgfComplicatedPbrIpPclResizeTrafficGenerate(0);

    /* Set Additional Route configuration */
    prvTgfComplicatedPbrIpPclResizeAdditionalRouteConfigSet();

    /* Generate Additional traffic */
    prvTgfComplicatedPbrIpPclResizeTrafficGenerate(1);

    /* Restore configuration */
    prvTgfComplicatedPbrIpPclResizeConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test LPM DB resize (from Default --> Smaller --> Smaller --> Bigger size) with PCL rules:
    Verify LPM DB resize functionality by checking the number of IPv4 and IPv6
    entries that can be added under dynamic resizing (reducing / reducing / enlarging)
    of the IP TCAM size and added PCL rules are valid.

    Create external TCAM manager;
    Configure VLAN, FDB entries;
    Enable IPv4\IPv6 UC routing;
    Update LPM DB and TCAM table to reduce size;
    Try to add max possible IPv4\IPv6 UC prefix to LPM DB to fill whole LPM DB;
    Verify that not possible to add more;
    Send IPv4\IPv6 UC traffic and vefify that added prefixes are valid;
    Add PCL rule to DROP any Not Ipv4 packet with specific DA;
    Delete several prefixes and update LPM DB and TCAM table to reduce size;
    Send IPv4\IPv6 UC traffic and vefify that all prefixes are valid;
    Send ethernet traffic and vefify that PCL rule is valid;
    Update LPM DB and TCAM table to enlarge size;
    Try to add additional IPv4\IPv6 UC prefix to LPM DB into increased space;
    Send IPv4\IPv6 UC traffic and vefify that all prefixes are valid;
    Send ethernet traffic and vefify that PCL rule is valid.
*/
UTF_TEST_CASE_MAC(tgfIpPclResizeDefSmallerSmallerBigger)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/
    GT_U32  stageIter = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);

    /* currently only PBR mode is supported */
    if (GT_FALSE == prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfIpPclResizeDefSmallerBiggerBaseConfigSet();

    /* Iterate thru 3 stages */
    for (stageIter = 0; stageIter < 3; stageIter++)
    {
        /* Set Route configuration */
        prvTgfIpPclResizeDefSmallerBiggerRouteConfigSet(stageIter);

        /* Generate traffic */
        prvTgfIpPclResizeDefSmallerBiggerTrafficGenerate(stageIter);
    }
    /* Restore configuration */
    prvTgfIpPclResizeDefSmallerBiggerConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test LPM DB resize (from Default --> Bigger --> Smaller --> Smaller size) with PCL rules:
    Verify LPM DB resize functionality by checking the number of IPv4 and IPv6
    entries that can be added under dynamic resizing (enlarging / reducing / reducing)
    of the IP TCAM size and added PCL rules are valid.

    Create external TCAM manager;
    Configure VLAN, FDB entries;
    Enable IPv4\IPv6 UC routing;
    Update LPM DB and TCAM table to specific size;
    Try to add max possible IPv4\IPv6 UC prefix to LPM DB to fill whole LPM DB;
    Add PCL rule to DROP any Not Ipv4 packet;
    Send IPv4\IPv6 UC traffic and vefify that added prefixes are valid;
    Send ethernet traffic and vefify that PCL rule is valid;
    Update LPM DB and TCAM table to enlarge size;
    Try to add additional IPv4\IPv6 UC prefix to LPM DB into increased space;
    Send IPv4\IPv6 UC traffic and vefify that all prefixes are valid;
    Send ethernet traffic and vefify that PCL rule is valid
    Delete several prefixes and update LPM DB and TCAM table to reduce size;
    Send IPv4\IPv6 UC traffic and vefify that all prefixes are valid;
    Send ethernet traffic and vefify that PCL rule is valid;
    Delete several prefixes and update LPM DB and TCAM table to reduce size;
    Add another PCL rule to DROP any Not Ipv4 packet with specific DA;
    Send IPv4\IPv6 UC traffic and vefify that all prefixes are valid;
    Send 2 ethernet packets and vefify that 2 PCL rules are valid.
*/
UTF_TEST_CASE_MAC(tgfIpPclResizeDefBiggerSmallerSmaller)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/
    GT_U32  stageIter = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);

    /* currently only PBR mode is supported */
    if (GT_FALSE == prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfIpPclResizeDefSmallerBiggerBaseConfigSet();

    /* Iterate thru 3 stages */
    for (stageIter = 1; stageIter < 4; stageIter++)
    {
        /* Set Route configuration */
        prvTgfIpPclResizeDefSmallerBiggerRouteConfigSet(stageIter);

        /* Generate traffic */
        prvTgfIpPclResizeDefSmallerBiggerTrafficGenerate(stageIter);
    }

    /* Restore configuration */
    prvTgfIpPclResizeDefSmallerBiggerConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IPv4 UC Low Level:
    Check behavior under complex conditions when there are several libraries
    initialized and used simultaneously under IPv4 traffic.

    The main goal of IP UC Table Filling tests is to check:
      - Low Level prefix creating mode;
      - whether the table's limits can be reachable;
      - proper CPSS behavior when the table's limit has been reached;
      - proper table's collaboration in entire pipeline under IPv4 UC traffic;
      - each prefix entry in the table;
      - IPv4 UC prefixes;
      - as many different prefix lengths as possible;
      - whether the tables can be cleaned properly;
      - whether there are no memory leakages after huge tasks.

    Table Filling Tests use the following workflow:
      - saving default values;
      - fill tables with generated prefixes (increment and shift) and verify
        each packet separately to check:
          - each matched entry in all tables;
          - non-matched entries;
          - matching with default prefix;
      - deleting all entries;
      - verifying each packet separately to check:
          - each matched entry was successfully deleted in all tables;
          - several non-matched entries to matching with default prefix;
      - process repeating in a few loops to check memory leakages and
        to check whether shadow/PP is cleaned properly after deletion;
      - restoring default values.
*/
UTF_TEST_CASE_MAC(tgfTablesFillingLttCheckIpv4)
{
/*
    1. Set Base configuration
    2. Generate traffic
    3. Restore configuration
*/
    GT_BOOL prevState;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT2_E | UTF_CPSS_PP_E_ARCH_CNS | UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_DXCH_E);

    /* Set Base Configuration */
    prevState = prvTgfTrafficPrintPacketTxEnableSet(GT_FALSE);
    prvTgfTablesFillingLttConfigurationSet();

    /* Generate Traffic */
    prvTgfTablesFillingLttTrafficGenerate(CPSS_IP_PROTOCOL_IPV4_E, 1 /*number of Loops*/);

    /* Restore Base Configuration */
    prvTgfTablesFillingLttConfigurationRestore();
    (void)prvTgfTrafficPrintPacketTxEnableSet(prevState);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IPv6 UC Low Level:
    Check behavior under complex conditions when there are several libraries
    initialized and used simultaneously under IPv6 traffic.

    The main goal of IP UC Table Filling tests is to check:
      - Low Level prefix creating mode;
      - whether the table's limits can be reachable;
      - proper CPSS behavior when the table's limit has been reached;
      - proper table's collaboration in entire pipeline under IPv6 UC traffic;
      - each prefix entry in the table;
      - IPv6 UC prefixes;
      - as many different prefix lengths as possible;
      - whether the tables can be cleaned properly;
      - whether there are no memory leakages after huge tasks.

    Table Filling Tests use the following workflow:
      - saving default values;
      - fill tables with generated prefixes (increment and shift) and verify
        each packet separately to check:
          - each matched entry in all tables;
          - non-matched entries;
          - matching with default prefix;
      - deleting all entries;
      - verifying each packet separately to check:
          - each matched entry was successfully deleted in all tables;
          - several non-matched entries to matching with default prefix;
      - process repeating in a few loops to check memory leakages and
        to check whether shadow/PP is cleaned properly after deletion;
      - restoring default values.
*/
UTF_TEST_CASE_MAC(tgfTablesFillingLttCheckIpv6)
{
/*
    1. Set Base configuration
    2. Generate traffic
    3. Restore configuration
*/
    GT_BOOL prevState;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT2_E | UTF_CPSS_PP_E_ARCH_CNS | UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_DXCH_E);

    /* Set Base Configuration */
    prevState = prvTgfTrafficPrintPacketTxEnableSet(GT_FALSE);
    prvTgfTablesFillingLttConfigurationSet();

    /* Generate Traffic */
    prvTgfTablesFillingLttTrafficGenerate(CPSS_IP_PROTOCOL_IPV6_E, 1 /*number of Loops*/);

    /* Restore Base Configuration */
    prvTgfTablesFillingLttConfigurationRestore();
    (void)prvTgfTrafficPrintPacketTxEnableSet(prevState);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IPv4 UC High Level with LPM DB logic checking:
    Check behavior under complex conditions when there are several libraries
    initialized and used simultaneously under IPv4 traffic.

    The main goal of IP UC Table Filling tests is to check:
      - High Level prefix creating mode;
      - whether the table's limits can be reachable;
      - proper CPSS behavior when the table's limit has been reached;
      - proper table's collaboration in entire pipeline under IPv4 UC traffic;
      - each prefix entry in the table;
      - IPv4 UC prefixes;
      - as many different prefix lengths as possible;
      - whether the tables can be cleaned properly;
      - whether there are no memory leakages after huge tasks.

    Table Filling Tests use the following workflow:
      - saving default values;
      - fill tables with generated prefixes (increment, shift and random)
        and verify each packet separately to check:
          - each matched entry in all tables;
          - non-matched entries;
          - matching with default prefix;
      - deleting all entries;
      - verifying each packet separately to check:
          - each matched entry was successfully deleted in all tables;
          - several non-matched entries to matching with default prefix;
      - process repeating in a few loops to check memory leakages and
        to check whether shadow/PP is cleaned properly after deletion;
      - restoring default values.
*/
UTF_TEST_CASE_MAC(tgfTablesFillingLpmCheckIpv4)
{
/*
    1. Set Base configuration
    2. Generate traffic
    3. Restore configuration
*/
    GT_BOOL prevState;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT2_E | UTF_CPSS_PP_E_ARCH_CNS);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_DXCH_E);

    GM_SKIP_TEST_DUE_TO_POOR_PERFORMANCE_MAC

    /* Set Base Configuration */
    prevState = prvTgfTrafficPrintPacketTxEnableSet(GT_FALSE);
    prvTgfTablesFillingLpmConfigurationSet();

    /* Generate Traffic */
    prvTgfTablesFillingLpmTrafficGenerate(CPSS_IP_PROTOCOL_IPV4_E, 0, 1 /*number of Loops*/);

    /* Restore Base Configuration */
    prvTgfTablesFillingLpmConfigurationRestore();
    (void)prvTgfTrafficPrintPacketTxEnableSet(prevState);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IPv6 UC High Level with LPM DB logic checking:
    Check behavior under complex conditions when there are several libraries
    initialized and used simultaneously under IPv6 traffic.

    The main goal of IP UC Table Filling tests is to check:
      - High Level prefix creating mode;
      - whether the table's limits can be reachable;
      - proper CPSS behavior when the table's limit has been reached;
      - proper table's collaboration in entire pipeline under IPv6 UC traffic;
      - each prefix entry in the table;
      - IPv6 UC prefixes;
      - as many different prefix lengths as possible;
      - whether the tables can be cleaned properly;
      - whether there are no memory leakages after huge tasks.

    Table Filling Tests use the following workflow:
      - saving default values;
      - fill tables with generated prefixes (increment, shift and random)
        and verify each packet separately to check:
          - each matched entry in all tables;
          - non-matched entries;
          - matching with default prefix;
      - deleting all entries;
      - verifying each packet separately to check:
          - each matched entry was successfully deleted in all tables;
          - several non-matched entries to matching with default prefix;
      - process repeating in a few loops to check memory leakages and
        to check whether shadow/PP is cleaned properly after deletion;
      - restoring default values.
*/
UTF_TEST_CASE_MAC(tgfTablesFillingLpmCheckIpv6)
{
/*
    1. Set Base configuration
    2. Generate traffic
    3. Restore configuration
*/
    GT_BOOL prevState;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT2_E | UTF_CPSS_PP_E_ARCH_CNS);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_DXCH_E);

    GM_SKIP_TEST_DUE_TO_POOR_PERFORMANCE_MAC

    /* Set Base Configuration */
    prevState = prvTgfTrafficPrintPacketTxEnableSet(GT_FALSE);
    prvTgfTablesFillingLpmConfigurationSet();

    /* Generate Traffic */
    prvTgfTablesFillingLpmTrafficGenerate(CPSS_IP_PROTOCOL_IPV6_E, 0, 3 /*number of Loops*/);

    /* Restore Base Configuration */
    prvTgfTablesFillingLpmConfigurationRestore();
    (void)prvTgfTrafficPrintPacketTxEnableSet(prevState);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IPv4\IPv6 UC High Level with LPM DB logic checking:
    Check behavior under complex conditions when there are several libraries
    initialized and used simultaneously under mixed IPv4\IPv6 traffic.

    The main goal of IP UC Table Filling tests is to check:
      - High Level prefix creating mode;
      - whether the table's limits can be reachable;
      - proper CPSS behavior when the table's limit has been reached;
      - proper table's collaboration in entire pipeline under IPv4\IPv6 UC traffic;
      - each prefix entry in the table;
      - IPv4\IPv6 UC prefixes;
      - as many different prefix lengths as possible;
      - whether the tables can be cleaned properly;
      - whether there are no memory leakages after huge tasks.

    Table Filling Tests use the following workflow:
      - saving default values;
      - fill tables with generated prefixes (increment, shift and random)
        and verify each packet separately to check:
          - each matched entry in all tables;
          - non-matched entries;
          - matching with default prefix;
      - deleting all entries;
      - verifying each packet separately to check:
          - each matched entry was successfully deleted in all tables;
          - several non-matched entries to matching with default prefix;
      - process repeating in a few loops to check memory leakages and
        to check whether shadow/PP is cleaned properly after deletion;
      - restoring default values.
*/
UTF_TEST_CASE_MAC(tgfTablesFillingLpmCheckIpv4v6)
{
/*
    1. Set Base configuration
    2. Generate traffic
    3. Restore configuration
*/
    GT_BOOL prevState;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT2_E | UTF_CPSS_PP_E_ARCH_CNS);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_DXCH_E);

    GM_SKIP_TEST_DUE_TO_POOR_PERFORMANCE_MAC

    /* Set Base Configuration */
    prevState = prvTgfTrafficPrintPacketTxEnableSet(GT_FALSE);
    prvTgfTablesFillingLpmConfigurationSet();

    /* Generate Traffic */
    prvTgfTablesFillingLpmTrafficGenerate(CPSS_IP_PROTOCOL_IPV4V6_E, 0, 1 /*number of Loops*/);

    /* Restore Base Configuration */
    prvTgfTablesFillingLpmConfigurationRestore();
    (void)prvTgfTrafficPrintPacketTxEnableSet(prevState);
}

/**
* @internal tgfTrunkBasicIpv4UcRouting function
* @endinternal
*
* @brief   IPv4 routing with trunk(s)
*         applicable devices: ALL
* @param[in] senderIsTrunk            - send is trunk ?
*                                      GT_FALSE - the sender is port (index 0)
*                                      GT_FALSE - the sender is trunk (trunk A)
* @param[in] nextHopIsTrunk           - next hop is trunk ?
*                                      GT_FALSE - the NH is port (index 0)
*                                      GT_FALSE - the NH is trunk (trunk B)
* @param[in] useSameTrunk             - trunk A and trunk B are the same trunk
*                                      relevant when   senderIsTrunk = GT_TRUE and nextHopIsTrunk = GT_TRUE
*                                       None
*/
static void tgfTrunkBasicIpv4UcRouting
(
    GT_BOOL     senderIsTrunk,
    GT_BOOL     nextHopIsTrunk,
    GT_BOOL     useSameTrunk
)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/
    GT_U32      prvUtfVrfId = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    if (GT_TRUE == prvUtfIsGmCompilation())
    {
        tgfTrafficGeneratorGmTimeFactorSet(15);
    }

    tgfTrunkBasicIpv4UcRoutingTestInit(senderIsTrunk,/*trunk A*/
                                       nextHopIsTrunk,/*trunk B*/
                                       useSameTrunk);/*trunk A != trunk B */

    /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[0]);/* portIdx 0 member of trunk A */

    /* Set Route configuration */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                  prvTgfPortsArray[3], GT_FALSE,
                                                  GT_FALSE, GT_FALSE);/* portIdx 0 member of trunk A, portIdx 3 member of trunk B */

    /* Generate traffic */
    prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS);/* portIdx 0 member of trunk A  , portIdx 3 member of trunk B */

    /* Restore configuration */
    prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);/* portIdx 0 member of trunk A */

    tgfTrunkBasicIpv4UcRoutingTestRestore();

    /* restore defaults */
    if (GT_TRUE == prvUtfIsGmCompilation())
    {
        tgfTrafficGeneratorGmTimeFactorSet(0);/* value 0 ... restore to defaults ! */
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv4 unicast routing. between trunks:
    create trunk A : with portIdx [0,1]
    create trunk B : with portIdx [2,3]

    Create 2 VLANs[portIdx]: 5[0, 1], 6[2, 3];
    Create FDB entry on portIdx 0, vlan 5;
    Enable UC routing on ports 0,1 and VLAN 5;
    Create UC Route Entry with nextHopVlanId = 6 and (trunk type) trunk B;
    Add IPv4 UC prefix to LPM DB with exact packet's DIP match;
    Send IPv4 UC traffic from port in trunk A ports and verify to get expected routed traffic
    on trunk B --> check LBH.
*/
UTF_TEST_CASE_MAC(tgfTrunkBasicIpv4UcRoutingTrunkAToTrunkB)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    tgfTrunkBasicIpv4UcRouting(GT_TRUE,/*trunk A*/
                               GT_TRUE,/*trunk B*/
                               GT_FALSE);/*trunk A != trunk B */
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv4 unicast routing. between port and trunk:
    port : portIdx 0
    create trunk B : with portIdx [2,3]

    Create 2 VLANs[portIdx]: 5[0, 1], 6[2, 3];
    Create FDB entry on portIdx 0, vlan 5;
    Enable UC routing on ports 0,1 and VLAN 5;
    Create UC Route Entry with nextHopVlanId = 6 and (trunk type) trunk B;
    Add IPv4 UC prefix to LPM DB with exact packet's DIP match;
    Send IPv4 UC traffic from port and verify to get expected routed traffic
    on trunk B --> check LBH.
*/
UTF_TEST_CASE_MAC(tgfTrunkBasicIpv4UcRoutingPortToTrunkB)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    tgfTrunkBasicIpv4UcRouting(GT_FALSE,/*port*/
                               GT_TRUE,/*trunk B*/
                               GT_FALSE);/*NA */
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv4 unicast routing. between trunk and port:
    create trunk A : with portIdx [0,1]
    port : portIdx 3

    Create 2 VLANs[portIdx]: 5[0, 1], 6[2, 3];
    Create FDB entry on portIdx 0, vlan 5;
    Enable UC routing on ports 0,1 and VLAN 5;
    Create UC Route Entry with nextHopVlanId = 6 and (port type) portIdx 3;
    Add IPv4 UC prefix to LPM DB with exact packet's DIP match;
    Send IPv4 UC traffic from port in trunk A and verify to get expected routed traffic
    on portIdx 3.
*/
UTF_TEST_CASE_MAC(tgfTrunkBasicIpv4UcRoutingTrunkAToPort)
{
    tgfTrunkBasicIpv4UcRouting(GT_TRUE,/*trunk A*/
                               GT_FALSE,/*port*/
                               GT_FALSE);/*NA*/
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv4 unicast routing. between Trunk A and Trunk A (routing in the same trunk):
    create trunk A : with portIdx [0,1]

    Create 2 VLANs[portIdx]: 5[0, 1], 6[2, 3];
    Create FDB entry on portIdx 0, vlan 5;
    Enable UC routing on ports 0,1 and VLAN 5;
    Create UC Route Entry with nextHopVlanId = 6 and (trunk type) trunk A;
    Add IPv4 UC prefix to LPM DB with exact packet's DIP match;
    Send IPv4 UC traffic from port in trunk A and verify to get expected routed traffic
    on trunk A --> check LBH.
*/
UTF_TEST_CASE_MAC(tgfTrunkBasicIpv4UcRoutingTrunkAToTrunkA)
{
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_PUMA_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    prvTgfCaptureTpidWithZeroBmpDisable();
    tgfTrunkBasicIpv4UcRouting(GT_TRUE,/*trunk A*/
                               GT_TRUE,/*trunk A*/
                               GT_TRUE);/*use the same trunk for source and NH */
    prvTgfCaptureTpidWithZeroBmpRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv4 unicast routing. between trunk member and EPort represent VIDX with the same trunk.
    create trunk A : with portIdx [0,1]
    eport : 1000 -->E2PHY : VIDX 100 : ports : portIdx [0,1]
    NOTE: portIdx [2,3] is not in 'routing path'

    Create 2 VLANs[portIdx]: 5[0, 1], 6[0,1, 2,3];

    Create FDB entry on portIdx 0, vlan 5;
    Enable UC routing on ports 0,1 and VLAN 5;
    Create UC Route Entry with nextHopVlanId = 6 and (port type) EPORT 1000;
    Add IPv4 UC prefix to LPM DB with exact packet's DIP match;
    map E2PHY eport 1000 to VIDX 100
    set VIDX 100 with portIdx [0,1] (trunk members)
    Send IPv4 UC traffic from port in trunk A and verify to get expected routed traffic
    on trunk A --> check LBH --> check routed packet
    on portIdx [2] --> check NO packets
    on portIdx [3] --> check NO packets
*/
UTF_TEST_CASE_MAC(tgfVidxBasicIpv4UcRoutingTrunkAToVidxWithTrunkA)
{
    GT_U32      notAppFamilyBmp;
    GT_U32      prvUtfVrfId = 0;
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    if (GT_TRUE == prvUtfIsGmCompilation())
    {
        tgfTrafficGeneratorGmTimeFactorSet(15);
    }

    prvTgfCaptureTpidWithZeroBmpDisable();

    tgfTrunkBasicIpv4UcRoutingTestInit(GT_TRUE,/*trunk A*/
                                       GT_FALSE,/* NO need for trunk B */
                                       GT_FALSE);/*trunk A != trunk B */

    /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[0]);/* portIdx 0 member of trunk A */

    /* set EPort for vidx with the next port indexes [0,1] (only trunk members) */
    /* Set Route configuration on this EPort */
    prvTgfBasicIpv4EPortToVidxRouteConfigurationSet(
                                   BIT_0 |/*portIdx [0]*/
                                   BIT_1 /*portIdx [1]*/);

    /* Set Route configuration */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                  prvTgfPortsArray[0], GT_FALSE,
                                                  GT_FALSE, GT_FALSE);

    /* Generate traffic */
    prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[0], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS);/* portIdx 0 member of trunk A  , portIdx 3 member of trunk B */

    /* Restore configuration */
    prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);/* portIdx 0 member of trunk A */

    tgfTrunkBasicIpv4UcRoutingTestRestore();

    /* restore defaults */
    if (GT_TRUE == prvUtfIsGmCompilation())
    {
        tgfTrafficGeneratorGmTimeFactorSet(0);/* value 0 ... restore to defaults ! */
    }

    prvTgfCaptureTpidWithZeroBmpRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test MTU exceed exception:*/
/* AUTODOC:
    Configure VLAN, FDB entries;
    configure UC Routing entries;
    cunfiguge MTU;
    configure exception type: MTU_EXCEEDED_FOR_NON_DF_E
    configure exception command: CMD_DROP_HARD_
    send IPv4 UC traffic and verify to get expected routed traffic on nextHop port
    Update Mtu to smaller size and excption command to throw the packet
    send IPv4 UC traffic and verify that the packet is droped
    change the exception command to "trap to cpu"
    send IPv4 UC traffic and verify that the was traped in cpu

*/
UTF_TEST_CASE_MAC(tgfIpMtuExceedExceptionCheck)
{
    /*
        1. Set Base configuration
        2. Set Route configuration
        3. Set valid mtu
        3. Generate traffic
        5. set small mtu
        4. Restore configuration
    */

    GT_U32      prvUtfVrfId = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);

   /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

    /* Set Route configuration */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0, prvTgfPortsArray[3], GT_FALSE, GT_FALSE, GT_FALSE);

    /*start capture to the CPU configuration set*/
    prvTgfIpv4UcCaptureToCpuConfigurationSet();

    /* set valid mtu configuration */
    prvTgfIpv4UcMtuConfigurationSet(1000);

    /*set exception command & exception type*/
    prvTgfIpv4UcExceptionConfigurationSet(PRV_TGF_IP_EXCP_MTU_EXCEEDED_E,PRV_TGF_IP_EXC_CMD_NONE_E);

    /* Generate traffic */
    prvTgfIpv4UcMtuRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3],PRV_TGF_IP_EXC_CMD_NONE_E);

    /* set small mtu configuration */
    prvTgfIpv4UcMtuConfigurationSet(40);

    /*set exception command to "trap to cpu"*/
    prvTgfIpv4UcExceptionConfigurationSet(PRV_TGF_IP_EXCP_MTU_EXCEEDED_E,PRV_TGF_IP_EXC_CMD_TRAP_TO_CPU_E);

    /* Generate traffic */
    prvTgfIpv4UcMtuRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3],PRV_TGF_IP_EXC_CMD_TRAP_TO_CPU_E );

    /*setexception command to "drop hard"*/
    prvTgfIpv4UcExceptionConfigurationSet(PRV_TGF_IP_EXCP_MTU_EXCEEDED_E,PRV_TGF_IP_EXC_CMD_DROP_HARD_E);

    /* Generate traffic */
    prvTgfIpv4UcMtuRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3],PRV_TGF_IP_EXC_CMD_DROP_HARD_E );

    /* Restore configuration */
    prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId ,0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);

    /* restore MTU configuration */
    prvTgfIpv4UcMtuConfigurationRestore(PRV_TGF_IP_EXCP_MTU_EXCEEDED_E, CPSS_IP_PROTOCOL_IPV4_E,CPSS_IP_UNICAST_E);

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IPv4 unicast routing with prefixes manipulation to fit compress_1,
            compress_2 and regular buckets:
    configure VLAN, FDB entries;
    configure UC Routing entries;
    add IPv4 UC prefix to LPM DB with exact packet's DIP match;
    send IPv4 UC traffic and verify expected routed traffic on nextHop port.
    on each prefix added, traffic will be send.
    in the end of the test traffic will be send in a loop on all added prefixes.
*/
UTF_TEST_CASE_MAC(tgfIpv4UcRoutingAddMany)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/
    GT_U32      prvUtfVrfId = 0;

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

    /* Set Route configuration */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0, prvTgfPortsArray[3], GT_FALSE, GT_FALSE, GT_FALSE);

    /* add prefixes to lpm and send traffic to each prefix */
    prvTgfIpv4UcRoutingAddManyConfigurationAndTrafficGenerate();

    /* Restore configuration */
    prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);

    prvTgfIpv4UcRoutingAddManyConfigurationRestore();
}

UTF_TEST_CASE_MAC(tgfIpv4UcRoutingAddManyActivityState)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/
    GT_U32      prvUtfVrfId = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_FALCON_E);
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

    /* Set Route configuration */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0, prvTgfPortsArray[3], GT_FALSE, GT_FALSE, GT_FALSE);

    /* add prefixes to lpm and send traffic to each prefix */
    prvTgfIpv4UcRoutingAddManyConfigurationAndTrafficGenerateAndCheckActivityState();

    /* Restore configuration */
    prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);

    prvTgfIpv4UcRoutingAddManyConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IPv4 unicast routing with prefixes manipulation when filling
   the Lpm using cpssDxChIpLpmIpv4UcPrefixAddManyByOctet.
    configure VLAN, FDB entries;
    configure UC Routing entries;
    add IPv4 UC prefix to LPM DB with exact packet's DIP match;
    send IPv4 UC traffic and verify expected routed traffic on nextHop port.
    on first and last prefix added, traffic will be send.
*/
UTF_TEST_CASE_MAC(tgfIpv4UcRoutingAddManyIpLpmMng)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/
    GT_U32      notAppFamilyBmp;
    GT_U32      prvUtfVrfId = 0;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_FALCON_E) ;
    /* skip long test */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

    /* Set Route configuration */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0, prvTgfPortsArray[3], GT_FALSE, GT_FALSE, GT_FALSE);

    /* add prefixes to lpm and send traffic to each prefix */
    prvTgfIpLpmMngIpv4UcRoutingAddManyConfigurationAndTrafficGenerate();

    /* change blocks allocation method and reset LPM DB */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E);

    /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

    /* Set Route configuration */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0, prvTgfPortsArray[3], GT_FALSE, GT_FALSE, GT_FALSE);

    /* add prefixes to lpm and send traffic to each prefix */
    prvTgfIpLpmMngIpv4UcRoutingAddManyConfigurationAndTrafficGenerate();

    /* Restore configuration */
    prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);

    prvTgfIpLpmMngIpv4UcRoutingAddManyConfigurationRestore();
}

/*******************************************************************************
* tgfIpv4UcRoutingAddDeleteAddRealPrefixes
*
* DESCRIPTION:
*   Test IPv4 unicast routing with real prefixes manipulation when filling
*    the Lpm.
*   configure VLAN, FDB entries;
*   configure UC Routing entries;
*   add IPv4 UC prefix to LPM DB according to prefixes predefined in a file.
*   send IPv4 UC traffic and verify expected routed traffic on nextHop port.
*   delete IPv4 UC prefix from LPM DB according to prefixes predefined in a file
*       in order TOP to BOTTOM.
*   add IPv4 UC prefix to LPM DB according to prefixes predefined in a file.
*   send IPv4 UC traffic and verify expected routed traffic on nextHop port.
*   delete IPv4 UC prefix from LPM DB according to prefixes predefined in a file
*       in order BOTTOM to TOP.
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
UTF_TEST_CASE_MAC(tgfIpv4UcRoutingAddDeleteAddRealPrefixes)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/
    GT_U32      notAppFamilyBmp;
    GT_U32      prvUtfVrfId = 0;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_FALCON_E) ; /* need to addapt to correct falcon memory configuration - ramDbCfg.blocksSizeArray */

    if (prvUtfBaselineTestsExecutionFlagGet()==GT_TRUE)
    {
        /* Set Base configuration */
        prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

        /* Set Route configuration */
        prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0, prvTgfPortsArray[3], GT_FALSE, GT_FALSE, GT_FALSE);

        /* add prefixes to lpm and send traffic to each prefix */
        prvTgfIpLpmMngIpv4UcRoutingAddDeleteAddRealPrefixesConfigurationAndTrafficGenerate();

        /* delete prefixes from top and send traffic to each prefix */
        prvTgfIpLpmMngIpv4UcRoutingAddDeleteAddRealPrefixesConfigurationRestore(GT_TRUE);

        /* add prefixes to lpm and send traffic to each prefix */
        prvTgfIpLpmMngIpv4UcRoutingAddDeleteAddRealPrefixesConfigurationAndTrafficGenerate();

        /* delete prefixes from bottom and send traffic to each prefix */
        prvTgfIpLpmMngIpv4UcRoutingAddDeleteAddRealPrefixesConfigurationRestore(GT_FALSE);

        prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);
    }
}

/*******************************************************************************
* tgfIpv4UcRoutingHiddenPrefixes
*
* DESCRIPTION:
*   Test traffic to hidden prefixes.
*   Configure VLAN, FDB entries;
*   Configure UC Routing entries;
*   Add prefix 4/8 as NH1
*   Check traffic, all should match NH1
*   Add prefix 4.0/9 as NH2
*   Check traffic, first 2 should match NH2 and last 2 should match NH1
*   Add prefix 4.128/9 as NH3
*   Check traffic, first 2 should match NH2 and last 2 should match NH3
*   Delete prefix 4/8
*   Check traffic remain the same
*   Re-add prefix 4/8 as NH1
*   Check traffic remains the same (last 2 steps shows that deleting and re-adding the �default 4/8� doesn�t affect the other prefixes)
*   Delete 4.0/9
*   Check traffic, first 2 should match NH1 and last 2 should match NH3
*   Delete 4.128/9
*   All traffic should match NH1
*   Delete the Ipv4 prefix 4/8
*   Expect no traffic
*
*   Add prefix 4.0.0/24 as NH1
*   Check traffic, all should match NH1
*   Add prefix 4.0.0/25 as NH2
*   Check traffic, first 2 should match NH2 and last 2 should match NH1
*   Add prefix 4.0.0.128/25 as NH3
*   Check traffic, first 2 should match NH2 and last 2 should match NH3
*   Delete the Ipv4 prefix 4.0.0/24
*   Check traffic remain the same
*   Re-add prefix 4.0.0/24 as NH1
*   Check traffic remain the same (last 2 steps shows that deleting and re-adding the �default 4.0.0/24� doesn�t affect the other prefixes)
*   Delete the Ipv4 prefix 4.0.0/25
*   Check traffic, first 2 should match NH1 and last 2 should match NH3
*   Delete the Ipv4 prefix 4.0.0128/25
*   All traffic should match NH1
*   Delete the Ipv4 prefix 4.0.0/24
*   Expect no traffic
*
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
UTF_TEST_CASE_MAC(tgfIpv4UcRoutingHiddenPrefixes)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    PRV_TGF_SKIP_PASS_OK_LONG_TEST_GM_MAC(UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Set Base configuration */
    prvTgfIpv4UcRoutingHiddenPrefixesBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfIpv4UcRoutingHiddenPrefixesLttRouteConfigurationSet();

    /* add prefixes to lpm and send traffic to each prefix */
    prvTgfIpv4UcRoutingHiddenPrefixesConfigurationAndTrafficGenerate();

    /* Restore configuration */
    prvTgfIpv4UcRoutingHiddenPrefixesConfigurationRestore();
}


/**
* @internal tgfIpv4UcRoutingAddRealPrefixesSharingAndPbrDifferentModes function
* @endinternal
*
* @brief   Test IPv4 unicast routing with real prefixes manipulation and with different
*         sharing and PBR configuration when filling the Lpm.
*         configure VLAN, FDB entries;
*         configure UC Routing entries;
*         add IPv4 UC prefix to LPM DB according to prefixes predefined in a file.
*         send IPv4 UC traffic and verify expected routed traffic on nextHop port.
*/
static void tgfIpv4UcRoutingAddRealPrefixesSharingAndPbrDifferentModes
(
    GT_VOID
)
{   /*
        1. Set Base configuration
        2. Set Route configuration
        3. Generate traffic
        4. Restore configuration
    */
    GT_U32      prvUtfVrfId = 0;


    /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

    /* Set Route configuration */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0, prvTgfPortsArray[3], GT_FALSE, GT_FALSE, GT_FALSE);

    /* add prefixes to lpm and send traffic to each prefix */
    prvTgfIpLpmMngIpv4UcRoutingAddRealPrefixesConfigurationAndTrafficGenerate();

    /* Restore configuration */
    prvTgfIpLpmMngIpv4UcRoutingAddRealPrefixesConfigurationRestore();

    prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IPv4 unicast routing with real prefixes manipulation when filling
    the Lpm.
    configure VLAN, FDB entries;
    configure UC Routing entries;
    add IPv4 UC prefix to LPM DB according to prefixes predefined in a file.
    send IPv4 UC traffic and verify expected routed traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfIpv4UcRoutingAddRealPrefixes)
{
    GT_U32      notAppFamilyBmp;
    GT_U8       pbrSizeIterator,lpmBlockAllocationMethodIterator,lpmMemModeIterator;
    GT_U32      currentPbrSize = PRV_TGF_IP_ADD_MANY_DEFAULT_PBR_SIZE_CNS;
    PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT currentBlockAllocationMethod = PRV_TGF_IP_ADD_MANY_DEFAULT_LPM_RAM_BLOCKS_ALLOCATION_METHOD;
    PRV_TGF_LPM_RAM_MEM_MODE_ENT currentLpmMemoryMode = PRV_TGF_IP_ADD_MANY_DEFAULT_LPM_RAM_MEMORY_MODE;
    GT_BOOL iterationValid = GT_TRUE;

    /* skip long test */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    CPSS_TBD_BOOKMARK_BOBCAT2   /* the test fails on TM, temporarily skip it */
        PRV_TGF_SKIP_TEST_WHEN_TM_USED_MAC;

    for(lpmMemModeIterator = 0;lpmMemModeIterator < PRV_TGF_LPM_RAM_MEM_MODE_LAST_E;lpmMemModeIterator++)
    {
        if((PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum) == GT_FALSE) ||
            (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)))
        {   /*half memory mode is tested only for SIP_5_20*/
            if((PRV_TGF_LPM_RAM_MEM_MODE_ENT)lpmMemModeIterator != PRV_TGF_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E)
            {
                continue;

            }
        }
        for(pbrSizeIterator = 0; pbrSizeIterator < sizeof(allowedPbrSizes)/sizeof(allowedPbrSizes[0]);pbrSizeIterator++)
        {
            for(lpmBlockAllocationMethodIterator = 0;lpmBlockAllocationMethodIterator < PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_LAST_E;
                lpmBlockAllocationMethodIterator++)
            {

                if((allowedPbrSizes[pbrSizeIterator]*_1K!=PRV_TGF_IP_ADD_MANY_DEFAULT_PBR_SIZE_CNS)
                    ||((PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT)lpmBlockAllocationMethodIterator!=PRV_TGF_IP_ADD_MANY_DEFAULT_LPM_RAM_BLOCKS_ALLOCATION_METHOD))
                {
                    if(prvUtfBaselineTestsExecutionFlagGet()== GT_FALSE)
                    {   /*perform only 8K PBR and non-sharing for non baseline execution*/
                        continue;
                    }
                }

                cpssOsPrintf("\n\n\nTest configuration: PBR size = %d K ,  block allocation method %s , lpm memory mode %s \n",
                    allowedPbrSizes[pbrSizeIterator],lpmBlockAllocationMethodIterator?"WITH_BLOCK_SHARING":"WITHOUT_BLOCK_SHARING",
                    lpmMemModeIterator?"LPM_RAM_MEM_MODE_HALF_MEM":"LPM_RAM_MEM_MODE_FULL_MEM");

                iterationValid = GT_TRUE;
                prvTgfIpLpmMngRoutingAddManyPrepareIteration(&currentBlockAllocationMethod,&currentLpmMemoryMode,&currentPbrSize,
                    (PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT)lpmBlockAllocationMethodIterator,(PRV_TGF_LPM_RAM_MEM_MODE_ENT)lpmMemModeIterator,allowedPbrSizes[pbrSizeIterator]*_1K,
                    &iterationValid);


                if(iterationValid == GT_FALSE)
                {
                    /*reset not supported,skip*/
                    cpssOsPrintf("Skipping scenario.Reset is not supported\n");
                    continue;
                 }

                tgfIpv4UcRoutingAddRealPrefixesSharingAndPbrDifferentModes();
            }
        }
    }

    /*Now  restore to default*/
    prvTgfIpLpmMngRoutingAddManyRestoreToDefault(currentBlockAllocationMethod,currentLpmMemoryMode,currentPbrSize);

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IPv6 unicast routing with prefixes manipulation to fit compress_1,
            compress_2 and regular buckets:
    configure VLAN, FDB entries;
    configure UC Routing entries;
    add IPv6 UC prefix to LPM DB with exact packet's DIP match;
    send IPv6 UC traffic and verify expected routed traffic on nextHop port.
    on each prefix added, traffic will be send.
    in the end of the test traffic will be send in a loop on all added prefixes.
*/
UTF_TEST_CASE_MAC(tgfIpv6UcRoutingAddMany)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/
    GT_U32      prvUtfVrfId = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /* Set Base configuration */
    prvTgfBasicIpv6UcRoutingBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfBasicIpv6UcRoutingRouteConfigurationSet(prvUtfVrfId,GT_FALSE);

    /* add prefixes to lpm and send traffic to each prefix */
    prvTgfIpv6UcRoutingAddManyConfigurationAndTrafficGenerate();

    /* Restore configuration */
    prvTgfBasicIpv6UcRoutingConfigurationRestore(prvUtfVrfId,GT_FALSE);

    prvTgfIpv6UcRoutingAddManyConfigurationRestore();
}

UTF_TEST_CASE_MAC(tgfIpv6UcRoutingAddManyActivityState)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/
    GT_U32      prvUtfVrfId = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_FALCON_E);
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /* Set Base configuration */
    prvTgfBasicIpv6UcRoutingBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfBasicIpv6UcRoutingRouteConfigurationSet(prvUtfVrfId,GT_FALSE);

    /* add prefixes to lpm and send traffic to each prefix */
    prvTgfIpv6UcRoutingAddManyConfigurationAndTrafficGenerateAndCheckActivityState();

    /* Restore configuration */
    prvTgfBasicIpv6UcRoutingConfigurationRestore(prvUtfVrfId,GT_FALSE);

    prvTgfIpv6UcRoutingAddManyConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IPv6 unicast routing with prefixes manipulation when filling
   the Lpm using cpssDxChIpLpmIpv6UcPrefixAddManyByOctet.
    configure VLAN, FDB entries;
    configure UC Routing entries;
    add IPv6 UC prefix to LPM DB with exact packet's DIP match;
    send IPv6 UC traffic and verify expected routed traffic on nextHop port.
    on first and last prefix added, traffic will be send.
*/
UTF_TEST_CASE_MAC(tgfIpv6UcRoutingAddManyIpLpmMng)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/
    GT_U32      notAppFamilyBmp;
    GT_U32      prvUtfVrfId = 0;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
     PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_FALCON_E) ;

     /* skip long test */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /* Set Base configuration */
    prvTgfBasicIpv6UcRoutingBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfBasicIpv6UcRoutingRouteConfigurationSet(prvUtfVrfId,GT_FALSE);

    /* add prefixes to lpm and send traffic to each prefix */
    prvTgfIpLpmMngIpv6UcRoutingAddManyConfigurationAndTrafficGenerate();

    /* change blocks allocation method and reset LPM DB */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E);

    /* Set Base configuration */
    prvTgfBasicIpv6UcRoutingBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfBasicIpv6UcRoutingRouteConfigurationSet(prvUtfVrfId,GT_FALSE);

    /* add prefixes to lpm and send traffic to each prefix */
    prvTgfIpLpmMngIpv6UcRoutingAddManyConfigurationAndTrafficGenerate();

    /* Restore configuration */
    prvTgfBasicIpv6UcRoutingConfigurationRestore(prvUtfVrfId,GT_FALSE);

    prvTgfIpLpmMngIpv6UcRoutingAddManyConfigurationRestore();
}

/* AUTODOC: Test Ipv4 & IPv6 unicast routing with prefixes manipulation when filling
   the Lpm using cpssDxChIpLpmIpv4UcPrefixAddManyByOctet & cpssDxChIpLpmIpv6UcPrefixAddManyByOctet.
    configure VLAN, FDB entries;
    configure UC Routing entries;
    add IPv4 UC prefix to LPM DB with exact packet's DIP match;
    send IPv4 UC traffic and verify expected routed traffic on nextHop port.
    on first and last prefix added, traffic will be send.
    add IPv6 UC prefix to LPM DB with exact packet's DIP match;
    send IPv6 UC traffic and verify expected routed traffic on nextHop port.
    on first and last prefix added, traffic will be send.
*/
UTF_TEST_CASE_MAC(tgfIpv4Ipv6UcRoutingAddManyIpLpmMng)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/
    GT_U32      notAppFamilyBmp;
    GT_U32      prvUtfVrfId = 0;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_FALCON_E) ;

    /* skip long test */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

    /* Set Base configuration */
    prvTgfBasicIpv6UcRoutingBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0, prvTgfPortsArray[3], GT_FALSE, GT_FALSE, GT_FALSE);

    /* Set Route configuration */
    prvTgfBasicIpv6UcRoutingRouteConfigurationSet(prvUtfVrfId,GT_FALSE);

    /* add prefixes to lpm and send traffic to each prefix */
    prvTgfIpLpmMngIpv4Ipv6UcRoutingAddManyConfigurationAndTrafficGenerate();

    /* change blocks allocation method and reset LPM DB */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E);

    /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

    /* Set Base configuration */
    prvTgfBasicIpv6UcRoutingBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0, prvTgfPortsArray[3], GT_FALSE, GT_FALSE, GT_FALSE);

    /* Set Route configuration */
    prvTgfBasicIpv6UcRoutingRouteConfigurationSet(prvUtfVrfId,GT_FALSE);

    /* add prefixes to lpm and send traffic to each prefix */
    prvTgfIpLpmMngIpv4Ipv6UcRoutingAddManyConfigurationAndTrafficGenerate();

    /* Restore configuration */
    prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);

    prvTgfBasicIpv6UcRoutingConfigurationRestore(prvUtfVrfId,GT_FALSE);

    prvTgfIpLpmMngIpv4Ipv6UcRoutingAddManyConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IPv4 multicast routing with prefixes manipulation to fit compress_1,
            compress_2 and regular buckets:
    configure VLAN, FDB entries;
    configure MC Routing entries and prefixes;
    send IPv4 MC traffic and verify to get expected routed traffic.
*/
UTF_TEST_CASE_MAC(tgfIpv4McRoutingAddMany)
{
    /*PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_DXCH_E);*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    prvTgfBasicIpv4McRoutingBaseConfigurationSet();

    prvTgfBasicIpv4McRoutingRouteConfigurationSet();

    /* add prefixes to lpm and send traffic to each prefix */
    prvTgfIpv4McRoutingAddManyConfigurationAndTrafficGenerate();

    /* Restore configuration */
    prvTgfBasicIpv4McRoutingConfigurationRestore();

}

UTF_TEST_CASE_MAC(tgfIpv4McRoutingAddManyActivityState)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_FALCON_E);
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    prvTgfBasicIpv4McRoutingBaseConfigurationSet();

    prvTgfBasicIpv4McRoutingRouteConfigurationSet();

    /* add prefixes to lpm and send traffic to each prefix */
    prvTgfIpv4McRoutingAddManyConfigurationAndTrafficGenerateAndCheckActivityState();

    /* Restore configuration */
    prvTgfBasicIpv4McRoutingConfigurationRestore();

}


/**
* @internal tgfIpv4McRoutingAddRealPrefixesSharingAndPbrDifferentModes function
* @endinternal
*
* @brief   Test IPv4 multicast routing with real prefixes manipulation and with different
*         sharing and PBR configuration when filling the Lpm.
*         configure VLAN, FDB entries;
*         configure MC Routing entries;
*         add IPv4 MC prefix to LPM DB according to prefixes predefined in a file.
*         send IPv4 MC traffic and verify expected routed traffic.
*/
static void tgfIpv4McRoutingAddRealPrefixesSharingAndPbrDifferentModes
(
    GT_VOID
)
{   /*
        1. Set Base configuration
        2. Set Route configuration
        3. Generate traffic
        4. Restore configuration
    */
    /* Set Base configuration */
    prvTgfBasicIpv4McRoutingBaseConfigurationSet();
    /* Set Route configuration */
    prvTgfBasicIpv4McRoutingRouteConfigurationSet();
    /* add prefixes to lpm and send traffic to each prefix */
    /* prvTgfBasicIpv4McRoutingRouteConfigurationDifferentPrefixLengthSet();
       prvTgfBasicIpv4McRoutingTrafficDifferentPrefixLengthGenerate();
       prvTgfBasicIpv4McRoutingConfigurationDifferentPrefixLengthRestore();*/
    prvTgfIpLpmMngIpv4McRoutingAddRealPrefixesConfigurationAndTrafficGenerate();

    /* Restore configuration */
    prvTgfIpLpmMngIpv4McRoutingAddRealPrefixesConfigurationRestore();

    prvTgfBasicIpv4McRoutingConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IPv4 multicast routing with real prefixes manipulation when filling
    the Lpm.
    configure VLAN, FDB entries;
    configure MC Routing entries and prefixes;
    add IPv4 MC prefix to LPM DB according to prefixes predefined in a file.
    send IPv4 MC traffic and verify expected routed traffic.
*/
UTF_TEST_CASE_MAC(tgfIpv4McRoutingAddRealPrefixes)
{
    GT_U32      notAppFamilyBmp;
    GT_STATUS   rc=GT_OK;

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_BOBCAT3_E , "JIRA:6787");
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_FALCON_E) ;

    /* run the test with the default configuration of the system:
       do not allow sharing and num of PBR entries = 8K */
    cpssOsPrintf("do not allow sharing, PBR=8K \n");
    tgfIpv4McRoutingAddRealPrefixesSharingAndPbrDifferentModes();

    if (prvUtfBaselineTestsExecutionFlagGet()==GT_TRUE)
    {
        /* cleans CPSS appDemo default configurations -
           LPM DB id 0 with VR id 0
           changes the LPM RAM blocks allocation method to DYNAMIC with sharing
           then restores the CPSS appDemo  default configurations */
        prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E);

        /* run the test with:
           allow sharing and num of PBR entries = 8K */
        cpssOsPrintf("allow sharing, PBR=8K \n");
        tgfIpv4McRoutingAddRealPrefixesSharingAndPbrDifferentModes();

        if(prvTgfResetModeGet() == GT_TRUE)
        {
            /*  set the max Num of PBR entries memory allocated for RAM based LPM shadow to be 0 --> all LPM is used for IP */
            rc = appDemoDbEntryAdd("maxNumOfPbrEntries", 0);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

             /* reset the system and run the test again with:
                allow sharing and num of PBR entries = 0 */
            prvTgfResetAndInitSystem();
            cpssOsPrintf("allow sharing, PBR=0 \n");
            tgfIpv4McRoutingAddRealPrefixesSharingAndPbrDifferentModes();

            /* cleans CPSS appDemo default configurations -
            LPM DB id 0 with VR id 0
            changes the LPM RAM blocks allocation method to DYNAMIC with no sharing
            then restores the CPSS appDemo  default configurations */
            prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E);

            /* reset the system and  run the test again with:
                do not allow sharing and num of PBR entries = 0 */
            cpssOsPrintf("do not allow sharing, PBR=0 \n");
            tgfIpv4McRoutingAddRealPrefixesSharingAndPbrDifferentModes();

            /* return default configuration of the system */
            rc = appDemoDbEntryAdd("maxNumOfPbrEntries", _8K);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

            /* change blocks allocation method and reset system */
            prvTgfResetAndInitSystem();
        }
        else
        {
           /* cleans CPSS appDemo default configurations -
           LPM DB id 0 with VR id 0
           changes the LPM RAM blocks allocation method to DYNAMIC with no sharing
           then restores the CPSS appDemo  default configurations */
           prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E);
        }
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv6 multicast routing with prefixes manipulation to fit compress_1,
            compress_2 and regular buckets:
    configure VLAN, FDB entries;
    configure MC Routing entries and prefixes;
    send IPv6 MC traffic and verify to get expected routed traffic.
*/
UTF_TEST_CASE_MAC(tgfIpv6McRoutingAddMany)
{
    /*PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_DXCH_E);*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    prvTgfBasicIpv6McRoutingBaseConfigurationSet();

    prvTgfBasicIpv6McRoutingRouteConfigurationSet();

      /* add prefixes to lpm and send traffic to each prefix */
    prvTgfIpv6McRoutingAddManyConfigurationAndTrafficGenerate();

    prvTgfBasicIpv6McRoutingConfigurationRestore();
}


UTF_TEST_CASE_MAC(tgfIpv6McRoutingAddManyActivityState)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_FALCON_E);
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    prvTgfBasicIpv6McRoutingBaseConfigurationSet();

    prvTgfBasicIpv6McRoutingRouteConfigurationSet();

      /* add prefixes to lpm and send traffic to each prefix */
    prvTgfIpv6McRoutingAddManyConfigurationAndTrafficGenerateAndCheckActivityState();

    prvTgfBasicIpv6McRoutingConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test adding of IPv4 bulk:
    configure VLAN, FDB entries;
    configure UC Routing entries;
    add bulk of IPv4 UC prefix to LPM DB with exact packet's DIP match;
    send IPv4 UC traffic and verify expected routed traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfBasicIpv4RoutingBulk)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/

    GT_U32   prvUtfVrfId = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);

    /* select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

    /* Set Route configuration */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                  prvTgfPortsArray[3], GT_TRUE,
                                                  GT_FALSE, GT_FALSE);

    /* Generate traffic */
    prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], GT_FALSE, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS);
    prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], GT_TRUE, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS);

    /* Restore configuration */
    prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_TRUE);

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test adding of IPv6 bulk:
    configure VLAN, FDB entries;
    configure UC Routing entries;
    add bulk of IPv6 UC prefix to LPM DB with exact packet's DIP match;
    send IPv6 UC traffic and verify expected routed traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfBasicIpv6RoutingBulk)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/

    GT_U32      prvUtfVrfId = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);

    /* Set Base configuration */
    prvTgfBasicIpv6UcRoutingBaseConfigurationSet();

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Route configuration */
    prvTgfBasicIpv6UcRoutingRouteConfigurationSet(prvUtfVrfId, GT_TRUE);

    /* Generate traffic */
    prvTgfBasicIpv6UcRoutingTrafficGenerate(GT_FALSE,GT_TRUE,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    prvTgfBasicIpv6UcRoutingTrafficGenerate(GT_TRUE,GT_TRUE,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* Restore configuration */
    prvTgfBasicIpv6UcRoutingConfigurationRestore(prvUtfVrfId, GT_FALSE);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test bulk adding of prefixes for IP unicast routing:
    Scenario:
    1. Set Max number of levels M in prefix:
       For IPv4 tests: M=4 (0..3);
       For IPv6 tests: M=16(0..15);
    2. Select a bucket level for the test randomly in range
       [ 1<= L < M(L can't be the VR level) ];
    3. Fill octets for the rest of the levels [M-1];
    4. Select a prefix length PL randomly in range [8*(L+1) <= P <=8*M];
    5. Select random number X in range [0 < X < 253];
    6. Select random number Y in range [1 < Y < (255-X)];
    7. Add a bulk of 3 IP prefixes (to force the bucket at selected level L
       to be changed from compressed 1 to compressed 2):
       1) for 1st IP address use X for the octet at level L,
          point prefix to -> NH = 1.
       2) for 2nd IP address use X+Y for the octet at level L,
          point prefix to -> NH = 2.
       3) for 3rd IP address use (X+Y+1) for the octet at level L,
          point prefix to -> NH = 3.
    8. Send traffic for the 3 prefixes, check that it reaches its destination
    9. Bulk delete 3rd prefix (to force octet at level L to
                               be changed back to compressed 1).
    10. Send traffic for 1st and 2nd prefixes, check that it reaches
        its destinations. Send traffic for 3rd prefix and check
        that it is dropped.
    11. Select randomly a different octet value for level 0.
    12. Add a bulk of 3 IP prefixes (to force the bucket at selected level L
        to be changed from compressed 1 to compressed 2):
       1) use 250 for the octet at level L,
          point prefix to -> NH = 1.
       2) use 252 for the octet at level L,
          point prefix to -> NH = 2.
       3) use 254 for the octet at level L
          point prefix to -> NH = 3.
    13. Select random number X in range [0 < X < 247];
    14. Select random number Y: Y=X+2;
    15. Add bulk of 2 IP prefixes (to force octet at level L to be changed
                                   from compressed 2 to regular):
       1) use X for the octet at level L,
          point prefix to -> NH = 4.
       2) use Y for the octet at level L
          point prefix to -> NH = 5.
    16. Send traffic for the 5 prefixes and check that it reaches
        its destination.
    17. Bulk delete the prefix from 15.1 (to force the bucket at level L
                                          to be changed back to compressed 2).
    18. Send traffic again for the 5 prefixes.
        Check that traffic to the prefix at 15.1 is dropped
        while traffic to the other 4 prefixes reaches its destination.
*/
UTF_TEST_CASE_MAC(tgfIpLpmIpUcPrefixBulkAdd)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /* Set Base configuration */
    prvTgfIpLpmIpUcPrefixBulkAddBaseConfigurationSet();

    /* Set Route configuration with 5 max possible route entries */
    prvTgfIpLpmIpUcPrefixBulkAddRouteConfigurationSet(5);

    /* Run test scenario for Ipv4 */
    prvTgfIpLpmIpvxUcPrefixBulkTest(CPSS_IP_PROTOCOL_IPV4_E);
    /* Run test scenario for Ipv6 */
    prvTgfIpLpmIpvxUcPrefixBulkTest(CPSS_IP_PROTOCOL_IPV6_E);

    /* Restore configuration */
    prvTgfIpLpmIpUcPrefixBulkAddConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv4 unicast routing with Mac sa mode modification:
    configure VLAN, FDB entries;
    configure UC Routing entries;
    add IPv4 UC prefix to LPM DB with exact packet's DIP match;
    configure MAC SA configuration
    send IPv4 UC traffic and verify expected routed traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfBasicIpv4UcMacSaModesRouting)
{
/*
    1.  configure MAC SA assignment mode to 8 bit VLAN mode
        1.1. Set Base configuration
        1.2. Set Route configuration for vrfId[0]
        1.3. Set Mac sa configuration
        1.4. Generate traffic
        1.5. Restore configuration
    2.  configure per egress- physical-port MAC SA assignment mode,
        to global MAC SA mode and configure the global MAC SA
        2.1. Set Base configuration
        2.2. Set Route configuration for vrfId[0]
        2.3. Set Mac sa configuration
        2.4. Generate traffic
        2.5. Restore configuration
    3.  configure MAC SA assignment mode to 8 bit per port mode
        3.1. Set Base configuration
        3.2. Set Route configuration for vrfId[0]
        3.3. Set Mac sa configuration
        3.4. Generate traffic
        3.5. Restore configuration
    4.  configure MAC SA assignment mode to least significant 12-bits according to the packet's VLAN ID.
        4.1. Set Base configuration
        4.2. Set Route configuration for vrfId[0]
        4.3. Set Mac sa configuration
        4.4. Generate traffic
        4.5. Restore configuration
*/
    GT_U32      prvUtfVrfId = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /*configure MAC SA assignment mode to 8 bit VLAN mode */

     /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

    /* Set Route configuration for vrfId[0] */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                  prvTgfPortsArray[3], GT_FALSE,
                                                  GT_FALSE, GT_FALSE);
    /*Set Mac sa configuration*/
    prvTgfBasicIpv4UcMacSaRoutingConfigurationSet(CPSS_SA_LSB_PER_VLAN_E);

    /* Generate traffic */
    prvTgfBasicIpv4UcMacSaRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], GT_FALSE, CPSS_SA_LSB_PER_VLAN_E);

    /* Restore configuration */
    prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);




    /*configure MAC SA assignment mode to least significant 12-bits
    according to the packet's VLAN ID*/

    if(!(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)))
    {
         /* Set Base configuration */
        prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

        /* Set Route configuration for vrfId[0] */
        prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                      prvTgfPortsArray[3], GT_FALSE,
                                                      GT_FALSE, GT_FALSE);
        /*Set Mac sa configuration*/
        prvTgfBasicIpv4UcMacSaRoutingConfigurationSet(CPSS_SA_LSB_PER_PKT_VID_E);

        /* Generate traffic */
         prvTgfBasicIpv4UcMacSaRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], GT_FALSE, CPSS_SA_LSB_PER_PKT_VID_E);

        /* Restore configuration */
        prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);
    }

    /*configure per egress- physical-port MAC SA assignment mode,
    to global MAC SA mode and configure the global MAC SA */

    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        /* Set Base configuration */
        prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

        /* Set Route configuration for vrfId[0] */
        prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                      prvTgfPortsArray[3], GT_FALSE,
                                                      GT_FALSE, GT_FALSE);
        /*Set Mac sa configuration*/
        prvTgfBasicIpv4UcMacSaRoutingConfigurationSet(CPSS_SA_LSB_FULL_48_BIT_GLOBAL);

        /* Generate traffic */
        prvTgfBasicIpv4UcMacSaRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], GT_FALSE, CPSS_SA_LSB_FULL_48_BIT_GLOBAL);

        /* Restore configuration */
        prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);
    }
    else
    {

         /*configure MAC SA assignment mode to 8 bit per port mode */

         /* Set Base configuration */
        prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

        /* Set Route configuration for vrfId[0] */
        prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                      prvTgfPortsArray[3], GT_FALSE,
                                                      GT_FALSE, GT_FALSE);
        /*Set Mac sa configuration*/
        prvTgfBasicIpv4UcMacSaRoutingConfigurationSet(CPSS_SA_LSB_PER_PORT_E);

        /* Generate traffic */
        prvTgfBasicIpv4UcMacSaRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], GT_FALSE, CPSS_SA_LSB_PER_PORT_E);

        /* Restore configuration */
        prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);
    }

}



/*----------------------------------------------------------------------------*/
/* AUTODOC: Test  IPv4 unicast routing default override:
    configure VLAN, FDB entries;
    configure IPv4 UC default rout entry;
    send IPv4 UC traffic and verify expected routed traffic on default nextHop port.
    add prefix 240/4
    send IPv4 UC traffic and verify expected routed traffic on default nextHop port.
*/
UTF_TEST_CASE_MAC(tgfIpv4UcRoutingDefaultOverride)
{
/*
    1. Set Base configuration
    2. Set Route configuration for vrfId[0]
    3. Generate traffic
    4. Restore configuration
    5. Select random vrfId
    6. Set Base configuration
    7. Set Route configuration for vrfId[random]
    8. Generate traffic
    9. Restore configuration
*/
    GT_U32      notAppFamilyBmp;
    GT_U32      prvUtfVrfId = 0;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;


        /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

    /* Set Route configuration for vrfId[0] */
    prvTgfIpv4UcRoutingDefaultOverrideRouteConfigurationSet(prvUtfVrfId, 0,
                                                  prvTgfPortsArray[3],GT_TRUE);
    /* Generate traffic */
    prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS);

    prvTgfIpv4UcRoutingDefaultOverrideRouteConfigurationSet(prvUtfVrfId, 0,
                                                  prvTgfPortsArray[3],GT_FALSE);
    /* Generate traffic */
    prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS);

    /* Restore configuration */
    prvTgfIpv4UcRoutingDefaultOverrideConfigurationRestore(prvUtfVrfId, 0);
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv4 multicast routing:
    configure VLAN, FDB entries;
    configure MC Routing entries and prefixes;
    send IPv4 MC traffic and verify to get expected routed traffic.


    set legal IPV4 Address: group 224.1.1.1/32, source 1.1.1.1/32, expect traffic
    set IPv4 MC Illegal DIP Address - Multicast class D
    group 224.0.0.0/4, source 1.0.0.0/0, expect traffic
    set IPv4 MC Illegal SIP Address - 0/0 - illegal SIP
    group 224.2.3.4/32, source 0.0.0.0/0, expect no traffic (dropped)
    set IPv4 MC entry: group 224.2.3.4/32, source 1.2.3.4/32, expect traffic
    set legal IPV4 Address: group 224.1.1.1/32, source 1.2.3.4/8, expect traffic
    set legal IPV4 Address: group 224.1.1.1/8, source 1.2.3.4/8, expect traffic
    set legal IPV4 Address: group 224.0.0.0/4, source 0.0.0.1/8, expect traffic
    set legal IPV4 Address: group 224.0.0.0/4, source 1.2.3.4/32, expect traffic
    set legal IPV4 Address: group 224.0.0.0/8, source 1.2.3.4/32, expect traffic

   restore configuration
*/
UTF_TEST_CASE_MAC(tgfBasicIpv4McRoutingDifferentMcEntries)
{
    /*GT_U32      notAppFamilyBmp;*/

    /* Supported by eArch devices */
    /*UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E | UTF_CPSS_PP_E_ARCH_CNS);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);

    PRV_TGF_NOT_SUPPORTED_L2_MLL_CHECK_MAC(prvTgfDevNum);

    prvTgfBasicIpv4McRoutingDifferentMcEntries();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic IPv4 multicast routing: source Id Mask and source Id
    configure VLAN, FDB entries;
    configure MC Routing entries and prefixes;
    send IPv4 MC traffic and verify to get expected routed traffic.

    set source Id Mask and source Id
    set ePcl rule to drop traffic with final sourceId
    send IPv4 MC traffic and verify no traffic

    change sourceId Mask
    send IPv4 MC traffic and verify traffic

    configuration restore
*/
UTF_TEST_CASE_MAC(tgfBasicIpv4McRoutingSourceId)
{
    GT_U32      notAppFamilyBmp;

    /* bug in egress counters for drop packets in GM therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC


    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);
    prvTgfBasicIpv4McRoutingSourceId();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test is for IPv4 UC rollback
    configure VLAN, FDB entries;
    configure UC Routing entry and uc prefixes;
    insert another ipv4 uc prefix with same first octet and
    calculate how many hw write and malloc operations are consumed
    during insertion;
    delete this prefix;
    by random manner set hw write sequence fail number and insert previous
    prefix;
    it should fail on predicted hw write operation;
    send IPv4 UC traffic and verify to get expected routed traffic;
    verify that LPM hw data and software shadow are OK.
    by random manner set malloc sequence fail number and insert previous
    prefix;
    it should fail on predicted malloc operation;
    send IPv4 UC traffic and verify to get expected routed traffic;
    verify that LPM hw data and software shadow are OK.
*/

UTF_TEST_CASE_MAC(tgfIpv4UcRollBack)
{
    GT_U32      notAppFamilyBmp;
    GT_U32      prvUtfVrfId = 0;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_FALCON_E) ;

#ifdef SHARED_MEMORY
    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_ALL_FAMILY_E , "JIRA:7457");
#endif

    /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

    /* Set Route configuration for vrfId[0] */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                  prvTgfPortsArray[3], GT_FALSE,
                                                  GT_FALSE, GT_FALSE);
    prvTgfIpv4UcRollBackCheck();
    /* Restore configuration */
    prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test is for IPv6 UC rollback
    configure VLAN, FDB entries;
    configure UC Routing entry and uc prefixes;
    insert another IPv6 uc prefix with same first octet and
    calculate how many hw write and malloc operations are consumed
    during insertion;
    delete this prefix;
    by random manner set hw write sequence fail number and insert previous
    prefix;
    it should fail on predicted hw write operation;
    send IPv6 UC traffic and verify to get expected routed traffic;
    verify that LPM hw data and software shadow are OK.
    by random manner set malloc sequence fail number and insert previous
    prefix;
    it should fail on predicted malloc operation;
    send IPv6 UC traffic and verify to get expected routed traffic;
    verify that LPM hw data and software shadow are OK.
*/

UTF_TEST_CASE_MAC(tgfIpv6UcRollBack)
{
    GT_U32      notAppFamilyBmp;
    GT_U32      prvUtfVrfId = 0;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_FALCON_E) ;

#ifdef SHARED_MEMORY
    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_ALL_FAMILY_E , "JIRA:7457");
#endif

    /* Set Base configuration */
    prvTgfBasicIpv6UcRoutingBaseConfigurationSet();

    /* Set Route configuration for vrfId[0]*/
    prvTgfBasicIpv6UcRoutingRouteConfigurationSet(prvUtfVrfId, GT_FALSE);

    prvTgfIpv6UcRollBackCheck();
    /* Restore configuration */
    /* Restore configuration */
    prvTgfBasicIpv6UcRoutingConfigurationRestore(prvUtfVrfId, GT_FALSE);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test is for IPv4 MC rollback
    configure VLAN, FDB entries;
    configure MC Routing entry and mc prefix;
    insert another ipv4 mc prefix with same first octet and
    calculate how many hw write and malloc operations are consumed
    during insertion;
    delete this prefix;
    by random manner set hw write sequence fail number and insert previous
    prefix;
    it should fail on predicted hw write operation;
    send IPv4 MC traffic and verify to get expected routed traffic;
    verify that LPM hw data and software shadow are OK.
    by random manner set malloc sequence fail number and insert previous
    prefix;
    it should fail on predicted malloc operation;
    send IPv4 MC traffic and verify to get expected routed traffic;
    verify that LPM hw data and software shadow are OK.
*/
UTF_TEST_CASE_MAC(tgfIpv4McRollBack)
{
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_FALCON_E) ;

#ifdef SHARED_MEMORY
    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_ALL_FAMILY_E , "JIRA:7457");
#endif

    prvTgfBasicIpv4McRoutingBaseConfigurationSet();

    prvTgfBasicIpv4McRoutingRouteConfigurationSet();
    prvTgfIpv4McRollBackCheck();
    prvTgfBasicIpv4McRoutingConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test is for IPv6 MC rollback
    configure VLAN, FDB entries;
    configure MC Routing entry and mc prefix;
    insert another IPv6 mc prefix with same first octet and
    calculate how many hw write and malloc operations are consumed
    during insertion;
    delete this prefix;
    by random manner set hw write sequence fail number and insert previous
    prefix;
    it should fail on predicted hw write operation;
    send IPv6 MC traffic and verify to get expected routed traffic;
    verify that LPM hw data and software shadow are OK.
    by random manner set malloc sequence fail number and insert previous
    prefix;
    it should fail on predicted malloc operation;
    send IPv6 MC traffic and verify to get expected routed traffic;
    verify that LPM hw data and software shadow are OK.
*/

UTF_TEST_CASE_MAC(tgfIpv6McRollBack)
{
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_FALCON_E) ;

#ifdef SHARED_MEMORY
    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_ALL_FAMILY_E , "JIRA:7457");
#endif

    prvTgfBasicIpv6McRoutingBaseConfigurationSet();

    prvTgfBasicIpv6McRoutingRouteConfigurationSet();

    prvTgfIpv6McRollBackCheck();

    prvTgfBasicIpv6McRoutingConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - LTT entry points to NAT, TCP packet:
    configure VLAN, FDB entries;
    configure PCL entries and create PCL rules to redirect to LTT entry;
    configure UC Routing entry and LTT entry;
    configure NAT entry: macDa, sip, dip, src port, dst port
    send IPv4 traffic to match rule;
    verify traffic on port 3 with NAT fields
*/

UTF_TEST_CASE_MAC(tgfIpv4UcNatTcpRouting)
{
    GT_U32      notAppFamilyBmp;
    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_FALCON_E) ;

    /* this test is supported only for BobCat2_B0 and above */
    if(GT_FALSE == PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfIpv4UcNatTcpRouting();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - LTT entry points to NAT, TCP packet:
    configure VLAN, FDB entries;
    configure PCL entries and create PCL rules to redirect to LTT entry;
    configure UC Routing entry and LTT entry;
    configure NAT entry: macDa, sip, dip, src port, dst port
    send IPv4 traffic to match rule;
    verify traffic on port 3 with NAT fields
*/

UTF_TEST_CASE_MAC(tgfIpv4UcNatUdpWithOptions)
{
    GT_U32      notAppFamilyBmp;
    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_FALCON_E) ;

    /* this test is supported only for BobCat2_B0 and above */
    if(GT_FALSE == PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfIpv4UcNatUdpWithOptions();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test lpm algorithm (push down) and unicast routing under dynamic partition:
    configure VLAN, FDB entries;
    configure 12 UC route and ARP entries;
    add 10 IPv4 and 2 IPv6 UC prefix to LPM DB;
    delete exact match IPv4 prefix and add new IPv4 12 len prefix;
    send IPv4\IPv6 UC traffic and verify expected routed traffic on nextHop ports.
*/
UTF_TEST_CASE_MAC(tgfDynamicPartitionIpLpmPushDownUcRouting)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* NOTE: Test is not written for PBR mode !!! */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* create new lpm DB */
    prvTgfDynamicPartitionIpLpmDbConfigurationSet(7);

    /* Set Base configuration */
    prvTgfDynamicPartitionIpLpmPushDownUcBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfDynamicPartitionIpLpmPushDownUcRouteConfigurationSet();

    /* Generate traffic */
    prvTgfDynamicPartitionIpLpmPushDownUcRoutingTrafficGenerate();

    if(prvTgfResetModeGet() == GT_TRUE &&/* this reset cause crash in 'parallell'
            running from LUA test : deadlock_check_zero_level_mtx */
        (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E) &&
        (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E))
    {

        prvTgfResetAndInitSystem();
    }
    else
    {
        /* Restore configuration */
        prvTgfDynamicPartitionIpLpmPushDownUcRoutingConfigurationRestore();

         /* clean new lpm DB */
        prvTgfDynamicPartitionIpLpmDbConfigurationRestore();
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test lpm algorithm (push up) and unicast routing under dynamic partition:
    configure VLAN, FDB entries;
    configure 12 UC route and ARP entries;
    add 10 IPv4 and 2 IPv6 UC prefix to LPM DB;
    send IPv4\IPv6 UC traffic and verify expected routed traffic on nextHop ports.
*/
UTF_TEST_CASE_MAC(tgfDynamicPartitionIpLpmPushUpUcRouting)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* NOTE: Test is not written for PBR mode !!! */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }
    /* create new lpm DB */
    prvTgfDynamicPartitionIpLpmDbConfigurationSet(7);

    /* Set Base configuration */
    prvTgfDynamicPartitionIpLpmPushUpUcBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfDynamicPartitionIpLpmPushUpUcRouteConfigurationSet();

    /* Generate traffic */
    prvTgfDynamicPartitionIpLpmPushUpUcRoutingTrafficGenerate();

    /* Restore configuration */
    prvTgfDynamicPartitionIpLpmPushUpUcRoutingConfigurationRestore();

    /* clean new lpm DB */
    prvTgfDynamicPartitionIpLpmDbConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test lpm algorithm (defrag enable) and unicast routing under dynamic partition:
    configure VLAN, FDB entries;
    configure 12 UC route and ARP entries;
    add 10 IPv4 and 2 IPv6 UC prefix to LPM DB;
    delete 4 IPv4 prefix and add new IPv6 12 len prefix;
    send IPv4\IPv6 UC traffic and verify expected routed traffic on nextHop ports.
*/
UTF_TEST_CASE_MAC(tgfDynamicPartitionIpLpmDefragUcRouting)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* NOTE: Test is not written for PBR mode !!! */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* create new lpm DB */
    prvTgfDynamicPartitionIpLpmDbConfigurationSet(7);

    /* Set Base configuration */
    prvTgfDynamicPartitionIpLpmDefragUcBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfDynamicPartitionIpLpmDefragUcRouteConfigurationSet();

    /* Generate traffic */
    prvTgfDynamicPartitionIpLpmDefragUcRoutingTrafficGenerate();

    /* Restore configuration */
    prvTgfDynamicPartitionIpLpmDefragUcRoutingConfigurationRestore();

    /* clean new lpm DB */
    prvTgfDynamicPartitionIpLpmDbConfigurationRestore();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test lpm algorithm (defrag enable) and unicast routing under dynamic partition:
    Test scenario from costumer is a specific test that fit a 8 lines TCAM
    So in order for the test to check what we want we need to configure before cpssInitSystem:
    lpmDbPartitionEnable=0x0
    lpmDbFirstTcamLine=0
    lpmDbLastTcamLine=7
    maxNumOfIpv4Prefixes=1
    maxNumOfIpv4McEntries=1
    maxNumOfIpv6Prefixes=1
*/
UTF_TEST_CASE_MAC(tgfDynamicPartitionIpLpmDefragUcRoutingSpecificCostumerTestScenario)
{
/*
    1. Set configuration and check there is no fails
    4. Restore configuration - delete all prefixes
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* NOTE: Test is not written for PBR mode !!! */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* create new lpm DB */
    prvTgfDynamicPartitionIpLpmDbConfigurationSet(7);

    /* Set Base configuration */
    prvTgfDynamicPartitionIpLpmDefragUcBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfDynamicPartitionIpLpmDefragUcRouteConfigurationCostumerTestScenarioSet();

    /* Costumer scenario configuration */
    prvTgfDynamicPartitionIpLpmDefragUcRoutingSpecificCostumerTestScenario();

    /* Generate traffic */
    prvTgfDynamicPartitionIpLpmDefragUcRoutingSpecificCostumerTestScenarioTrafficGenerate();

    /* Clean Costumer scenario configuration */
    prvTgfDynamicPartitionIpLpmDefragUcRoutingCostumerTestScenarioRestore();

    /* clean new lpm DB */
    prvTgfDynamicPartitionIpLpmDbConfigurationRestore();

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test lpm algorithm (defrag enable) and unicast routing under dynamic partition:
    Test scenario from costumer is a specific test that fit a 1001 lines TCAM
    So in order for the test to check what we want we need to configure before cpssInitSystem:
    lpmDbPartitionEnable=0x0
    lpmDbFirstTcamLine=0
    lpmDbLastTcamLine=1001
    maxNumOfIpv4Prefixes=1
    maxNumOfIpv4McEntries=1
    maxNumOfIpv6Prefixes=1
*/
UTF_TEST_CASE_MAC(tgfDynamicPartitionIpLpmDefragUcRoutingRandomCostumerTestScenario)
{
/*
    1. Set configuration and check there is no fails
    4. Restore configuration - delete all prefixes
*/

    /* skip this test when 'prvUtfSkipLongTestsFlagSet' */
   PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);
    /* Skip this test for GM - performance reason */
    GM_SKIP_TEST_DUE_TO_POOR_PERFORMANCE_MAC;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* NOTE: Test is not written for PBR mode !!! */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* create new lpm DB */
    prvTgfDynamicPartitionIpLpmDbConfigurationSet(1000);

    /* Set Base configuration */
    prvTgfDynamicPartitionIpLpmDefragUcBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfDynamicPartitionIpLpmDefragUcRouteConfigurationCostumerTestScenarioSet();

    /* Costumer scenario configuration */
    prvTgfDynamicPartitionIpLpmDefragUcRoutingRandomCostumerTestScenario();

    /* Generate traffic - this test do not generate traffic,
       in case traffic is needed we will need to re-write the trafficGenerate function */
    /*prvTgfDynamicPartitionIpLpmDefragUcRoutingCostumerTestScenarioTrafficGenerate();*/

    /* Clean Costumer scenario configuration */
    prvTgfDynamicPartitionIpLpmDefragUcRoutingCostumerTestScenarioRestore();

    /* clean new lpm DB */
    prvTgfDynamicPartitionIpLpmDbConfigurationRestore();

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test lpm algorithm (defrag enable) and unicast routing under dynamic partition:
    Test scenario is a specific test that fit a 1001 lines TCAM
    So in order for the test to check what we want we need to configure before cpssInitSystem:
    lpmDbPartitionEnable=0x0
    lpmDbFirstTcamLine=0
    lpmDbLastTcamLine=1000
    maxNumOfIpv4Prefixes=1
    maxNumOfIpv4McEntries=1
    maxNumOfIpv6Prefixes=1
*/
UTF_TEST_CASE_MAC(tgfDynamicPartitionIpLpmDefragUcRoutingRandomTestScenario)
{
/*
    1. Set configuration and check there is no fails
    4. Restore configuration - delete all prefixes
*/

    /* skip this test when 'prvUtfSkipLongTestsFlagSet' */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);
    /* Skip this test for GM - performance reason */
    GM_SKIP_TEST_DUE_TO_POOR_PERFORMANCE_MAC;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* NOTE: Test is not written for PBR mode !!! */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* create new lpm DB */
    prvTgfDynamicPartitionIpLpmDbConfigurationSet(1000);

    /* Set Base configuration */
    prvTgfDynamicPartitionIpLpmDefragUcBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfDynamicPartitionIpLpmDefragUcRouteConfigurationCostumerTestScenarioSet();

    /* Costumer scenario configuration */
    prvTgfDynamicPartitionIpLpmDefragUcRoutingRandomTestScenario();

    /* Generate traffic - this test do not generate traffic,
       in case traffic is needed we will need to re-write the trafficGenerate function */
    /*prvTgfDynamicPartitionIpLpmDefragUcRoutingTestScenarioTrafficGenerate();*/

    /* Clean Costumer scenario configuration */
    prvTgfDynamicPartitionIpLpmDefragUcRoutingCostumerTestScenarioRestore();

    /* clean new lpm DB */
    prvTgfDynamicPartitionIpLpmDbConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test lpm algorithm (defrag enable) and multicast routing under dynamic partition:
   Test scenario is a specific test that fit a 6 lines TCAM
   So in order for the test to check what we want we need to configure before cpssInitSystem:
   lpmDbPartitionEnable=0x0
   lpmDbFirstTcamLine=0
   lpmDbLastTcamLine=5
   maxNumOfIpv4Prefixes=1
   maxNumOfIpv4McEntries=1
   maxNumOfIpv6Prefixes=1
*/
UTF_TEST_CASE_MAC(tgfDynamicPartitionIpLpmDefragIpv4McRouting)
{
/*
    1. Set configuration and check there is no fails
    4. Restore configuration - delete all prefixes
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* NOTE: Test is not written for PBR mode !!! */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* create new lpm DB */
    prvTgfDynamicPartitionIpLpmDbMcConfigurationSet(5);

    /* Set Base configuration */
    prvTgfDynamicPartitionIpLpmDefragMcBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfDynamicPartitionIpLpmDefragMcRouteConfigurationSet(CPSS_IP_PROTOCOL_IPV4_E);

    /* Costumer scenario configuration */
    prvTgfDynamicPartitionIpLpmDefragIpv4McRouting();

    /* Generate traffic */
    prvTgfDynamicPartitionIpLpmDefragIpv4McRoutingTrafficGenerate();

    /* Clean Costumer scenario configuration */
    prvTgfDynamicPartitionIpLpmDefragMcRoutingRestore();

    /* clean new lpm DB */
    prvTgfDynamicPartitionIpLpmDbMcConfigurationRestore();

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test lpm algorithm (defrag enable) and multicast routing under dynamic partition,
   the test want to check a scenario of moving the default MC entry:
   Test scenario is a specific test that fit a 5 lines TCAM
   So in order for the test to check what we want we need to configure before cpssInitSystem:
   lpmDbPartitionEnable=0x0
   lpmDbFirstTcamLine=0
   lpmDbLastTcamLine=4
   maxNumOfIpv4Prefixes=1
   maxNumOfIpv4McEntries=1
   maxNumOfIpv6Prefixes=1
*/
UTF_TEST_CASE_MAC(tgfDynamicPartitionIpLpmDefragIpv4McRoutingMovingDefault)
{
/*
    1. Set configuration and check there is no fails
    4. Restore configuration - delete all prefixes
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* due to a bug in moving default MC entry this test will be skiped.
       when the bug is fixed the "#if 0" should be removed */
    prvUtfSkipTestsSet();

#if 0
    /* NOTE: Test is not written for PBR mode !!! */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* create new lpm DB */
    prvTgfDynamicPartitionIpLpmDbMcConfigurationSet(4);

    /* Set Base configuration */
    prvTgfDynamicPartitionIpLpmDefragMcBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfDynamicPartitionIpLpmDefragMcRouteConfigurationSet(CPSS_IP_PROTOCOL_IPV4_E);

    /* Costumer scenario configuration */
    prvTgfDynamicPartitionIpLpmDefragIpv4McRoutingMovingDefault();

    /* Generate traffic */
    prvTgfDynamicPartitionIpLpmDefragIpv4McRoutingTrafficGenerate();

    /* Clean Costumer scenario configuration */
    prvTgfDynamicPartitionIpLpmDefragMcRoutingRestore();

    /* clean new lpm DB */
    prvTgfDynamicPartitionIpLpmDbIpv4McConfigurationRestore();
#endif
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test lpm algorithm (defrag enable) and multicast routing under dynamic partition:
   Test scenario is a specific test that fit a 7 lines TCAM
   So in order for the test to check what we want we need to configure before cpssInitSystem:
   lpmDbPartitionEnable=0x0
   lpmDbFirstTcamLine=0
   lpmDbLastTcamLine=6
   maxNumOfIpv4Prefixes=1
   maxNumOfIpv4McEntries=1
   maxNumOfIpv6Prefixes=1
*/
UTF_TEST_CASE_MAC(tgfDynamicPartitionIpLpmDefragIpv6McRouting)
{
/*
    1. Set configuration and check there is no fails
    4. Restore configuration - delete all prefixes
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* NOTE: Test is not written for PBR mode !!! */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* create new lpm DB */
    prvTgfDynamicPartitionIpLpmDbMcConfigurationSet(6);

    /* Set Base configuration */
    prvTgfDynamicPartitionIpLpmDefragMcBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfDynamicPartitionIpLpmDefragMcRouteConfigurationSet(CPSS_IP_PROTOCOL_IPV6_E);

    /* Costumer scenario configuration */
    prvTgfDynamicPartitionIpLpmDefragIpv6McRouting();

    /* Generate traffic */
    prvTgfDynamicPartitionIpLpmDefragIpv6McRoutingTrafficGenerate();

    /* Clean Costumer scenario configuration */
    prvTgfDynamicPartitionIpLpmDefragMcRoutingRestore();

    /* clean new lpm DB */
    prvTgfDynamicPartitionIpLpmDbMcConfigurationRestore();

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test lpm algorithm (defrag enable) and unicast routing under dynamic partition:
    Test scenario from costumer is a specific test that fit a 2048 lines TCAM
    So in order for the test to check what we want we need to configure before cpssInitSystem:
    lpmDbPartitionEnable=0x0
    lpmDbFirstTcamLine=0
    lpmDbLastTcamLine=2047
    maxNumOfIpv4Prefixes=1
    maxNumOfIpv4McEntries=1
    maxNumOfIpv6Prefixes=1
*/
UTF_TEST_CASE_MAC(tgfDynamicPartitionIpLpmDefragMcRoutingRandomCostumerTestScenario)
{
/*
    1. Set configuration and check there is no fails
    4. Restore configuration - delete all prefixes
*/
    /* skip this test when 'prvUtfSkipLongTestsFlagSet' */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);
    /* Skip this test for GM - performance reason */
    GM_SKIP_TEST_DUE_TO_POOR_PERFORMANCE_MAC;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* due to a bug in moving default MC entry this test will be skiped.
       when the bug is fixed the "#if 0" should be removed */
    prvUtfSkipTestsSet();

#if 0
    /* NOTE: Test is not written for PBR mode !!! */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* create new lpm DB */
    prvTgfDynamicPartitionIpLpmDbMcConfigurationSet(2047);

    /* Set Base configuration */
    prvTgfDynamicPartitionIpLpmDefragMcBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfDynamicPartitionIpLpmDefragMcRouteConfigurationSet(CPSS_IP_PROTOCOL_IPV6_E);

     /* Costumer scenario configuration */
    prvTgfDynamicPartitionIpLpmDefragMcRoutingRandomCostumerTestScenario();

    /* Generate traffic - this test do not generate traffic,
       in case traffic is needed we will need to re-write the trafficGenerate function */
    /*prvTgfDynamicPartitionIpLpmDefragIpv6McRoutingTrafficGenerate();*/

    /* Clean Costumer scenario configuration */
    prvTgfDynamicPartitionIpLpmDefragMcRoutingRestore();

    /* clean new lpm DB */
    prvTgfDynamicPartitionIpLpmDbMcConfigurationRestore();
#endif
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test lpm algorithm (defrag enable) and unicast routing under dynamic partition:
    Test scenario from costumer is a specific test that fit a 1001 lines TCAM
    So in order for the test to check what we want we need to configure before cpssInitSystem:
    lpmDbPartitionEnable=0x0
    lpmDbFirstTcamLine=0
    lpmDbLastTcamLine=1000
    maxNumOfIpv4Prefixes=1
    maxNumOfIpv4McEntries=1
    maxNumOfIpv6Prefixes=1
*/
UTF_TEST_CASE_MAC(tgfDynamicPartitionIpLpmDefragIpv4UcIpv6McRoutingRandomCostumerTestScenario)
{
/*
    1. Set configuration and check there is no fails
    4. Restore configuration - delete all prefixes
*/

    /* skip this test when 'prvUtfSkipLongTestsFlagSet' */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);
    /* Skip this test for GM - performance reason */
    GM_SKIP_TEST_DUE_TO_POOR_PERFORMANCE_MAC;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* NOTE: Test is not written for PBR mode !!! */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* create new lpm DB */
    prvTgfDynamicPartitionIpLpmDbMcConfigurationSet(1000);

    /* Set Base configuration */
    prvTgfDynamicPartitionIpLpmDefragMcBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfDynamicPartitionIpLpmDefragMcRouteConfigurationSet(CPSS_IP_PROTOCOL_IPV6_E);

     /* Costumer scenario configuration */
    prvTgfDynamicPartitionIpLpmDefragIpv4UcIpv6McRoutingRandomCostumerTestScenario();

    /* Generate traffic - this test do not generate traffic,
       in case traffic is needed we will need to re-write the trafficGenerate function */
    /*prvTgfDynamicPartitionIpLpmDefragIpv6McRoutingTrafficGenerate();*/

    /* Clean Costumer scenario configuration */
    prvTgfDynamicPartitionIpLpmDefragMcRoutingRestore();

    /* clean new lpm DB */
    prvTgfDynamicPartitionIpLpmDbMcConfigurationRestore();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IPv4 unicast routing when LPM is configured to work in the half mode
1.Configure LPM to work in half mode
2.Add one prefix to LPM
3.Send same paket one time to port that belong to PIPE0 and second time to port that belong to PIPE1
4.Expected result that the packet will egress same port in both cases
*/
UTF_TEST_CASE_MAC(tgfIpv4UcRoutingLpmHalfModeCheck)
{
    GT_STATUS rc;
    GT_U32      prvUtfVrfId = 0;
    GT_U8       pipe0PortIdx;
    GT_U8       pipe1PortIdx;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (GT_U32)(~UTF_BOBCAT3_E));
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_FALCON_E) ;

    if(GT_TRUE == prvTgfResetModeGet())
    {

        /* LPM RAM Configuration for  half LPM Memory mode */
        rc = appDemoDbEntryAdd("lpmMemMode", 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd");

        prvTgfResetAndInitSystem();

        /* Set Base configuration */
        prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

        /*Pick a port from pipe 0*/
        rc = prvTgfBasicIpPickPortFromPipe(0,3,&pipe0PortIdx);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBasicIpPickPortFromPipe");

        /*Add port from PIPE0 to default sending VLAN and enable routing on this port - may be already done at prvTgfBasicIpv4UcRoutingBaseConfigurationSet*/
        prvTgfBasicIpAddToDefaultSendVlanAndEnableRouting(pipe0PortIdx,CPSS_IP_PROTOCOL_IPV4_E);

        /* Set Route configuration */
        prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, pipe0PortIdx, prvTgfPortsArray[3], GT_FALSE, GT_FALSE, GT_FALSE);

        /*Pick a port from pipe 1*/
        rc = prvTgfBasicIpPickPortFromPipe(1,3,&pipe1PortIdx);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBasicIpPickPortFromPipe");


        /*Add port from PIPE1 to default sending VLAN and enable routing on this port*/
        prvTgfBasicIpAddToDefaultSendVlanAndEnableRouting(pipe1PortIdx,CPSS_IP_PROTOCOL_IPV4_E);


        /* Generate traffic  from port that  belong to PIPE0*/
        prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[pipe0PortIdx], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS);


        /* Generate traffic  from port that  belong to PIPE1*/
        prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[pipe1PortIdx], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS);

        /* LPM RAM Configuration restore*/
        rc = appDemoDbEntryAdd("lpmMemMode", 0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd");

        prvTgfResetAndInitSystem();

        prvTgfPortsNum = PRV_TGF_DEFAULT_NUM_PORTS_IN_TEST_CNS;
    }
    else
    {
        SKIP_TEST_MAC
    }

}


UTF_TEST_CASE_MAC(tgfIpv6UcRoutingLpmHalfModeCheck)
{
    GT_STATUS rc;
    GT_U32      prvUtfVrfId = 0;
    GT_U8       pipe0PortIdx;
    GT_U8       pipe1PortIdx;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (GT_U32)(~UTF_BOBCAT3_E));
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_FALCON_E) ;

    if(GT_TRUE == prvTgfResetModeGet())
    {

        /* LPM RAM Configuration for  half LPM Memory mode */
        rc = appDemoDbEntryAdd("lpmMemMode", 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd");

        prvTgfResetAndInitSystem();

        /* Set Base configuration */
        prvTgfBasicIpv6UcRoutingBaseConfigurationSet();

        /*Pick a port from pipe 0*/
        rc = prvTgfBasicIpPickPortFromPipe(0,3,&pipe0PortIdx);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBasicIpPickPortFromPipe");

        /*Add port from PIPE0 to default sending VLAN and enable routing on this port - may be already done at prvTgfBasicIpv4UcRoutingBaseConfigurationSet*/
        prvTgfBasicIpAddToDefaultSendVlanAndEnableRouting(pipe0PortIdx,CPSS_IP_PROTOCOL_IPV6_E);

        /* Set Route configuration */
        prvTgfBasicIpv6UcRoutingRouteConfigurationSet(prvUtfVrfId, GT_FALSE);

        /*Pick a port from pipe 1*/
        rc = prvTgfBasicIpPickPortFromPipe(1,3,&pipe1PortIdx);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBasicIpPickPortFromPipe");


        /*Add port from PIPE1 to default sending VLAN and enable routing on this port*/
        prvTgfBasicIpAddToDefaultSendVlanAndEnableRouting(pipe1PortIdx,CPSS_IP_PROTOCOL_IPV6_E);


        /* Generate traffic  from port that  belong to PIPE0*/
        prvTgfBasicIpv6UcRoutingTrafficGenerate(GT_FALSE,GT_TRUE,prvTgfPortsArray[pipe0PortIdx]);


        /* Generate traffic  from port that  belong to PIPE1*/
        prvTgfBasicIpv6UcRoutingTrafficGenerate(GT_FALSE,GT_TRUE,prvTgfPortsArray[pipe1PortIdx]);

        /* LPM RAM Configuration restore*/
        rc = appDemoDbEntryAdd("lpmMemMode", 0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd");

        prvTgfResetAndInitSystem();

        prvTgfPortsNum = PRV_TGF_DEFAULT_NUM_PORTS_IN_TEST_CNS;
    }
    else
    {
        SKIP_TEST_MAC
    }

}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IPv6 unicast routing SA mismatch check
1. Setup routing between two VLANs
2. Setup ARP and FDB entries
3. Check following cases when packet passes:
3.1 SA check enabled, ARP entry correct
3.2 SA check disabled, ARP entry correct
3.3 SA check disabled, ARP entry incorrect
4. Check if packet dropped in cases when:
4.1. SA check enabled, ARP entry incorrect
*/
UTF_TEST_CASE_MAC(tgfIpv6UcSaMismatchCheck)
{
    GT_U32 notAppFamilyBmp;
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_FALCON_E) ;

    prvTgfIpv6UcSaMismatchCheckConfig();
    prvTgfIpv6UcSaMismatchCheckGenerate();
    prvTgfIpv6UcSaMismatchCheckRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test RPF fail:
    configure VLAN, FDB entries;
    configure MC Routing entries and prefixes;
    send IPv6 MC traffic and verify to get expected routed traffic.
*/

UTF_TEST_CASE_MAC(tgfIpv6McRpfFail)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_FALCON_E);

    prvTgfIpv6McRpfFailBaseConfigurationSet();

    prvTgfIpv6McRpfFailRouteConfigurationSet();

    prvTgfIpv6McRpfFailTrafficGenerate(PRV_TGF_DEFAULT_BOOLEAN_CNS);

    prvTgfIpv6McRpfFailConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IPv4 unicast routing priority:
    configure VLAN, FDB entries;
    configure UC Routing entries;
    add IPv4 UC prefix to LPM DB with exact packet's DIP match;
    add FDB foute entry;
    send IPv4 UC traffic and verify expected packet has ARP MAC configured for FDB entry;
    set LPM lookup priority;
    send IPv4 UC traffic and verify expected packet has ARP MAC configured for LPM entry.
*/
UTF_TEST_CASE_MAC(tgfIpv4UcRoutingPriority)
{
/*
    1. Set Base configuration
    2. Set Route configuration for vrfId[0]
    3. Set Generic FDB Route configuration
    4. Set FDB Route configuration
    5. Generate traffic
    6. Set routing lookup priority
    7. Generate traffic
    8. Restore configuration
*/

    GT_U32      prvUtfVrfId = 0;
    GT_U32      notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_ALDRIN2_E | UTF_AC3X_E);

    /* Test is not applicable in PBR mode */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    prvTgfIpv4UcRoutingPriorityBaseConfigurationSet(prvTgfPortsArray[1]);

    /* Set Route configuration for vrfId[random] */
    prvTgfIpv4UcRoutingPriorityBasicRoutingConfigurationSet(prvUtfVrfId, 0, prvTgfPortsArray[3]);

    /* Set Generic FDB Route configuration */
    prvTgfIpv4UcRoutingPriorityFdbRoutingGenericConfigurationSet(0);

    /* Set FDB Route configuration */
    prvTgfIpv4UcRoutingPriorityFdbRoutingConfigurationSet(prvUtfVrfId);

    /* Genereate traffic */
    prvTgfIpv4UcRoutingPriorityTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], GT_TRUE);

    /* Set routing lookup priority */
    prvTgfIpv4UcRoutingPrioritySetLeafPriority(prvUtfVrfId, CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_LPM_E);

    /* Generate traffic */
    prvTgfIpv4UcRoutingPriorityTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], GT_FALSE);

    /* Restore configuration */
    prvTgfIpv4UcRoutingPriorityConfigurationRestore(prvUtfVrfId, 0);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IPv4 unicast PBR routing priority:
    configure VLAN, FDB entries;
    configure UC Routing entries;
    add IPv4 UC prefix to LPM DB with exact packet's DIP match;
    add FDB foute entry;
    send IPv4 UC traffic and verify expected packet has ARP MAC configured for FDB entry;
    set LPM lookup priority;
    send IPv4 UC traffic and verify expected packet has ARP MAC configured for LPM entry.
*/
UTF_TEST_CASE_MAC(tgfIpv4UcPbrRoutingPriority)
{
/*
    1. Set Base configuration
    2. Set Route configuration for vrfId[0]
    3. Set Generic FDB Route configuration
    4. Set FDB Route configuration
    5. Generate traffic
    6. Set routing lookup priority
    7. Generate traffic
    8. Restore configuration
*/

    GT_U32      prvUtfVrfId = 0;
    GT_U32      notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_ALDRIN2_E | UTF_AC3X_E);

    /* Test is not applicable in PBR mode */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    prvTgfIpv4UcPbrRoutingPriorityBaseConfigurationSet(prvUtfVrfId);

    /* Set PCL configuration */
    prvTgfIpv4UcPbrRoutingPriorityPclConfigurationSet();

    /* Set Generic FDB Route configuration */
    prvTgfIpv4UcPbrRoutingPriorityFdbRoutingGenericConfigurationSet();

    /* Set FDB Route configuration */
    prvTgfIpv4UcPbrRoutingPriorityFdbRoutingConfigurationSet(prvUtfVrfId);

    /* Genereate traffic */
    prvTgfIpv4UcPbrRoutingPriorityTrafficGenerate(GT_TRUE, GT_TRUE);

    /* Set routing LPM lookup priority */
    prvTgfIpv4UcPbrRoutingPriorityLeafEntrySet(CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_LPM_E);

    /* Generate traffic */
    prvTgfIpv4UcPbrRoutingPriorityTrafficGenerate(GT_TRUE, GT_FALSE);

    /* Set routing FDB lookup priority */
    prvTgfIpv4UcPbrRoutingPriorityLeafEntrySet(CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_FDB_E);

    /* Generate traffic */
    prvTgfIpv4UcPbrRoutingPriorityTrafficGenerate(GT_FALSE, GT_FALSE);

    /* Set routing LPM lookup priority */
    prvTgfIpv4UcPbrRoutingPriorityLeafEntrySet(CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_LPM_E);

    /* Generate traffic */
    prvTgfIpv4UcPbrRoutingPriorityTrafficGenerate(GT_FALSE, GT_FALSE);

    /* Disable FDB routing for PBR */
    prvTgfIpv4UcPbrRoutingUnicastRouteForPbrEnable(GT_FALSE);

    /* Set routing FDB lookup priority */
    prvTgfIpv4UcPbrRoutingPriorityLeafEntrySet(CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_FDB_E);

    /* Generate traffic */
    prvTgfIpv4UcPbrRoutingPriorityTrafficGenerate(GT_TRUE, GT_FALSE);

    /* Set routing LPM lookup priority */
    prvTgfIpv4UcPbrRoutingPriorityLeafEntrySet(CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_LPM_E);

    /* Generate traffic */
    prvTgfIpv4UcPbrRoutingPriorityTrafficGenerate(GT_TRUE, GT_FALSE);

    /* Restore configuration */
    prvTgfIpv4UcPbrRoutingPriorityConfigurationRestore(prvUtfVrfId, 0);
}

#if RUN_LONG_CAPACITY_TESTS
/* AUTODOC: Test Ipv4 & IPv6 unicast routing.
   adding and deleting prefixes untill the LPM is full.
   Adding 8 known prefixes
   Fill the LPM with ipv4 UC prefixes
   Delete all ipv4 prefixes
   Fill the LPM with ipv6 UC prefixes
   Delete all ipv6 prefixes
   Fill the LPM with ipv4 UC prefixes
   Delete all ipv4 prefixes
*/
UTF_TEST_CASE_MAC(prvTgfIpv4Ipv6UcRoutingAddDelAndFillLpm)
{
/*
    1. Add 8 pre defined prefixes
    2. Fill the LPM with ipv4 UC prefixes
    3. Delete all ipv4 prefixes
    4. Fill the LPM with ipv6 UC prefixes
    5. Delete all ipv6 prefixes
    6. Fill the LPM with ipv4 UC prefixes
    7. Delete all ipv4 prefixes

*/
    GT_STATUS   rc=GT_OK;

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /* Supported only for Aldrin with 2K lines in each block */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E | UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_XCAT3_E|UTF_BOBCAT3_E | UTF_CAELUM_E|UTF_AC3X_E|UTF_ALDRIN2_E);

            /* skip long test */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /*  set the max Num of PBR entries memory allocated for RAM based LPM shadow to be 0 --> all LPM is used for IP */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");
        if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
                rc = appDemoDbEntryAdd("sharedTableMode", 1);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(sharedTableMode)");
        }

     /* reset the system and run the test with:
        num of PBR entries = 0 */
    prvTgfResetAndInitSystem();
    cpssOsPrintf("PBR=0 WITHOUT_SHARING \n");

    prvTgfIpLpmIpv4Ipv6UcPrefixAddSomePrefix();

    /* add 4000+4000 prefixes with prefixlen=128 then delete them */
    prvTgfIpLpmIpv6UcPrefix128(4000,4000,14,GT_TRUE,1,GT_FALSE);

    /* add prefixes with prefixlen=64 (as much as possible acording to the device)*/
    prvTgfIpLpmIpv6UcPrefix64(60000,6,GT_FALSE,1,GT_FALSE);

    /* delete prefixes with prefixlen=64 then delete them (as much as possible acording to the device)*/
    prvTgfIpLpmIpv6UcPrefix64(60000,6,GT_TRUE,1,GT_FALSE);

    /* add 4000+4000 prefixes with prefixlen=128 then delete them */
    prvTgfIpLpmIpv6UcPrefix128(4000,4000,14,GT_TRUE,1,GT_FALSE);

    /* cleans CPSS appDemo default configurations -
       LPM DB id 0 with VR id 0
       changes the LPM RAM blocks allocation method to DYNAMIC with sharing
       then restores the CPSS appDemo  default configurations */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E);

    /*  set the max Num of PBR entries memory allocated for RAM based LPM shadow to be 0 --> all LPM is used for IP */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

     /* reset the system and run the test with:
        num of PBR entries = 0 */
    prvTgfResetAndInitSystem();
    cpssOsPrintf("PBR=0 WITH_SHARING\n");

    /* add prefixes with prefixlen=64 (as much as possible acording to the device)*/
    prvTgfIpLpmIpv6UcPrefix64(60000,6,GT_FALSE,1,GT_FALSE);

    /* delete prefixes with prefixlen=64 then delete them (as much as possible acording to the device)*/
    prvTgfIpLpmIpv6UcPrefix64(60000,6,GT_TRUE,1,GT_FALSE);

    /* cleans CPSS appDemo default configurations -
    LPM DB id 0 with VR id 0
    changes the LPM RAM blocks allocation method to DYNAMIC with sharing
    then restores the CPSS appDemo  default configurations */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E);

     /* return default configuration of the system */
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
            rc = appDemoDbEntryAdd("sharedTableMode", 2);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(sharedTableMode)");
                rc = appDemoDbEntryAdd("maxNumOfPbrEntries", _1K);
    }
    else
    {
        rc = appDemoDbEntryAdd("maxNumOfPbrEntries", _8K);
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

    /* change blocks allocation mapping and reset system */
    prvTgfResetAndInitSystem();
}

/* AUTODOC: Test IPv6 unicast routing.
   adding prefixes untill the LPM is full.
   Adding 8 known prefixes
   Fill the LPM with ipv6 UC prefixes
   Delete all ipv6 prefixes
   Fill the LPM with ipv6 UC prefixes - disordered + defragEnable=GT_FALSE
   Delete all ipv6 prefixes
   Fill the LPM with ipv6 UC prefixes - disordered + defragEnable=GT_TRUE
   Delete all ipv6 prefixes
*/
UTF_TEST_CASE_MAC(prvTgfIpv6UcRoutingFillLpmOrderAndDisorder)
{
/*
   adding prefixes untill the LPM is full.
   Adding 8 known prefixes
   1.Fill the LPM with ipv6 UC prefixes
   2.Delete all ipv6 prefixes
   3.Fill the LPM with ipv6 UC prefixes - disordered + defragEnable=GT_FALSE
   4.Delete all ipv6 prefixes
   5.Fill the LPM with ipv6 UC prefixes - disordered + defragEnable=GT_TRUE
   6.Delete all ipv6 prefixes

*/
    GT_STATUS   rc=GT_OK;

   /* Supported only for Aldrin with 2K lines in each block */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E | UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_XCAT3_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|UTF_ALDRIN2_E|UTF_FALCON_E);

    /* skip long test */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /*  set the max Num of PBR entries memory allocated for RAM based LPM shadow to be 0 --> all LPM is used for IP */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

     /* reset the system and run the test with:
        num of PBR entries = 0 */
    prvTgfResetAndInitSystem();
    cpssOsPrintf("PBR=0 WITHOUT_SHARING \n");

    /* flush memory then add 8072 prefixes with prefixlen=128 random=false,defrag=false */
    prvTgfIpLpmIpv6UcOrderAndDisorder(9128, GT_FALSE, GT_FALSE);

    /* flush memory then add 9126 prefixes with prefixlen=128 random=false,defrag=true */
    prvTgfIpLpmIpv6UcOrderAndDisorder(9128, GT_FALSE, GT_TRUE);

    /* flush memory then add ~4000 prefixes with prefixlen=128 random=true,defrag=false
       number of prefixes addded will be between 3000 to 6000 acording to the order added */
    prvTgfIpLpmIpv6UcOrderAndDisorder(9128, GT_TRUE, GT_FALSE);

    /* flush memory then add ~8000 prefixes with prefixlen=128 random=true,defrag=true
       number of prefixes addded will be between 6000 to 9128 acording to the order added */
    prvTgfIpLpmIpv6UcOrderAndDisorder(9128, GT_TRUE, GT_TRUE);


    /* cleans CPSS appDemo default configurations -
    LPM DB id 0 with VR id 0
    changes the LPM RAM blocks allocation method to DYNAMIC with sharing
    then restores the CPSS appDemo  default configurations */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E);

     /* return default configuration of the system */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", _8K);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

    /* change blocks allocation mapping and reset system */
    prvTgfResetAndInitSystem();
}
/* AUTODOC: Test IPv4 unicast routing.
   adding prefixes untill the LPM is full.
   Adding 8 known prefixes
   Fill the LPM with ipv4 UC prefixes
   Delete all ipv4 prefixes
   Fill the LPM with ipv4 UC prefixes - disordered + defragEnable=GT_FALSE
   Delete all ipv6 prefixes
   Fill the LPM with ipv4 UC prefixes - disordered + defragEnable=GT_TRUE
   Delete all ipv6 prefixes
*/
UTF_TEST_CASE_MAC(prvTgfIpv4UcRoutingFillLpmOrderAndDisorder)
{
/*
   adding prefixes untill the LPM is full.
   Adding 8 known prefixes
   1.Fill the LPM with ipv4 UC prefixes
   2.Delete all ipv4 prefixes
   3.Fill the LPM with ipv4 UC prefixes - disordered + defragEnable=GT_FALSE
   4.Delete all ipv4 prefixes
   5.Fill the LPM with ipv4 UC prefixes - disordered + defragEnable=GT_TRUE
   6.Delete all ipv4 prefixes

*/
    GT_STATUS   rc=GT_OK;

   /* Supported only for Aldrin with 2K lines in each block */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E | UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_XCAT3_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|UTF_ALDRIN2_E|UTF_FALCON_E);

    /* skip long test */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /*  set the max Num of PBR entries memory allocated for RAM based LPM shadow to be 0 --> all LPM is used for IP */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

     /* reset the system and run the test with:
        num of PBR entries = 0 */
    prvTgfResetAndInitSystem();
    cpssOsPrintf("PBR=0 WITHOUT_SHARING \n");

    /* flush memory then add 30205 prefixes with prefixlen=32 random=false,defrag=false */
    prvTgfIpLpmIpv4UcOrderAndDisorder(30631, GT_FALSE, GT_FALSE);

    /* flush memory then add 30630 prefixes with prefixlen=32 random=false,defrag=true */
    prvTgfIpLpmIpv4UcOrderAndDisorder(30631, GT_FALSE, GT_TRUE);

    /* flush memory then add prefixes with prefixlen=32 random=true,defrag=false
       number of prefixes addded will be between 15000 to 19000 acording to the order added */
    prvTgfIpLpmIpv4UcOrderAndDisorder(30631, GT_TRUE, GT_FALSE);

    /* flush memory then add prefixes with prefixlen=32 random=true,defrag=true
       number of prefixes addded will be between 19000 to 30631 acording to the order added */
    prvTgfIpLpmIpv4UcOrderAndDisorder(30631, GT_TRUE, GT_TRUE);


    /* cleans CPSS appDemo default configurations -
    LPM DB id 0 with VR id 0
    changes the LPM RAM blocks allocation method to DYNAMIC with sharing
    then restores the CPSS appDemo  default configurations */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E);

     /* return default configuration of the system */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", _8K);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

    /* change blocks allocation mapping and reset system */
    prvTgfResetAndInitSystem();
}

/* AUTODOC: Test Ipv4 and IPv6 unicast routing LPM filling
   Adding ipv4 UC prefixes untill the LPM is full when defragEnable=false.
   Adding ipv6 UC entry with prefixLen = 64 and defragEnable=false,
   we get a FAIL that the LPM is full.
   Adding the same ipv6 UC entry with prefixLen = 64 and defragEnable=true,
   the operation pass OK.
*/
UTF_TEST_CASE_MAC(prvTgfIpv4Ip6UcRoutingFillLpmDisorder)
{
/*
    1.Set system with maxNumOfPbrEntries = 0
    2.Adding ipv4 UC prefixes until the LPM is full when defragEnable=false.
    3.Adding one ipv6 UC prefix with prefixLen = 64 and defragEnable=false,
      --> we get a FAIL: the LPM is full.
    4.Adding the same ipv6 UC entry with prefixLen = 64 and defragEnable=true,
      --> the operation pass OK, defrag is done in blocks associated with ipv4
      and the ipv6 entry is successfully added.
*/
    GT_STATUS   rc=GT_OK;

   /* Supported only for Aldrin with 2K lines in each block */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E | UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_XCAT3_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|UTF_ALDRIN2_E|UTF_FALCON_E);

    /* skip long test */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /*  set the max Num of PBR entries memory allocated for RAM based LPM shadow to be 0 --> all LPM is used for IP */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

     /* reset the system and run the test with:
        num of PBR entries = 0 */
    prvTgfResetAndInitSystem();
    cpssOsPrintf("PBR=0 WITHOUT_SHARING \n");

    /* flush memory then add prefixes with prefixlen=32 random=false,defrag=false */
    prvTgfIpLpmIpv4UcDisorder(100000, GT_FALSE);

    /* add specific ipv6Uc/64 --> check Fail */
    prvTgfIpLpmIpv6UcAddSpecificPrefix(GT_FALSE);

    /* add specific ipv6Uc/64 --> check ok */
    prvTgfIpLpmIpv6UcAddSpecificPrefix(GT_TRUE);

    /* cleans CPSS appDemo default configurations -
    LPM DB id 0 with VR id 0
    changes the LPM RAM blocks allocation method to DYNAMIC with sharing
    then restores the CPSS appDemo  default configurations */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E);

     /* return default configuration of the system */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", _8K);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

    /* change blocks allocation mapping and reset system */
    prvTgfResetAndInitSystem();
}

/* AUTODOC: Test IPv6 Multicast routing when Src is changed.
   adding and deleting prefixes untill the LPM is full.
   Fill the LPM with ipv6 MC prefixes
   Delete all ipv4 prefixes
   Fill the LPM with ipv6 MC prefixes
   Delete all ipv6 prefixes
   Fill the LPM with ipv6 MC prefixes
   Delete all ipv4 prefixes
*/
UTF_TEST_CASE_MAC(prvTgfIpv6McRoutingAddDelAndFillLpmSrcChange)
{
/*
    1. Add 8 pre defined prefixes
    2. Fill the LPM with ipv6 MC prefixes
    3. Delete all ipv4 prefixes
    4. Fill the LPM with ipv6 MC prefixes
    5. Delete all ipv6 prefixes
    6. Fill the LPM with ipv6 MC prefixes
    7. Delete all ipv4 prefixes

*/
    GT_STATUS   rc=GT_OK;

    /* Supported only for Aldrin with 2K lines in each block */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E | UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_XCAT3_E|UTF_BOBCAT3_E | UTF_FALCON_E|UTF_CAELUM_E|UTF_AC3X_E|UTF_ALDRIN2_E|UTF_FALCON_E);
    /* skip long test */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /*  set the max Num of PBR entries memory allocated for RAM based LPM shadow to be 0 --> all LPM is used for IP */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

    /* reset the system and run the test with: num of PBR entries = 0 */
    prvTgfResetAndInitSystem();
    cpssOsPrintf("PBR=0 WITHOUT_SHARING\n");

    /* fill lpm when octet 14, 15 is changed */
    prvTgfIpLpmIpv6McPrefixSrcChange(60000,15,14,128,128,GT_FALSE,GT_FALSE);
    /* delete lpm when octet 14, 15 is changed */
    prvTgfIpLpmIpv6McPrefixSrcChange(60000,15,14,128,128,GT_TRUE, GT_FALSE);

    /* fill lpm when octet 12, 13 is changed */
    prvTgfIpLpmIpv6McPrefixSrcChange(60000,3,4,128,128,GT_FALSE,GT_TRUE);
    /* delete lpm when octet 12, 13 is changed */
    prvTgfIpLpmIpv6McPrefixSrcChange(60000,3,4,128,128,GT_TRUE, GT_TRUE);

    /* cleans CPSS appDemo default configurations -
       LPM DB id 0 with VR id 0
       changes the LPM RAM blocks allocation method to DYNAMIC with sharing
       then restores the CPSS appDemo  default configurations */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E);

    /*  set the max Num of PBR entries memory allocated for RAM based LPM shadow to be 0 --> all LPM is used for IP */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

     /* reset the system and run the test with:
        num of PBR entries = 0 */
    prvTgfResetAndInitSystem();
    cpssOsPrintf("PBR=0 WITH_SHARING\n");

    /* fill lpm when octet 14, 15 is changed */
    prvTgfIpLpmIpv6McPrefixSrcChange(60000,15,14,128,128,GT_FALSE,GT_FALSE);
    /* delete lpm when octet 14, 15 is changed */
    prvTgfIpLpmIpv6McPrefixSrcChange(60000,15,14,128,128,GT_TRUE, GT_FALSE);

    /* fill lpm when octet 12, 13 is changed */
    prvTgfIpLpmIpv6McPrefixSrcChange(60000,3,4,128,128,GT_FALSE,GT_TRUE);
    /* delete lpm when octet 12, 13 is changed */
    prvTgfIpLpmIpv6McPrefixSrcChange(60000,3,4,128,128,GT_TRUE, GT_TRUE);

    /* cleans CPSS appDemo default configurations -
    LPM DB id 0 with VR id 0
    changes the LPM RAM blocks allocation method to DYNAMIC with sharing
    then restores the CPSS appDemo  default configurations */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E);

     /* return default configuration of the system */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", _8K);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

    /* change blocks allocation mapping and reset system */
    prvTgfResetAndInitSystem();
}

/* AUTODOC: Test IPv6 Multicast routing when Group is changed.
   adding and deleting prefixes untill the LPM is full.
   Fill the LPM with ipv6 MC prefixes
   Delete all ipv4 prefixes
   Fill the LPM with ipv6 MC prefixes
   Delete all ipv6 prefixes
   Fill the LPM with ipv6 MC prefixes
   Delete all ipv4 prefixes
*/
UTF_TEST_CASE_MAC(prvTgfIpv6McRoutingAddDelAndFillLpmGroupChange)
{
/*
    1. Add 8 pre defined prefixes
    2. Fill the LPM with ipv6 MC prefixes
    3. Delete all ipv6 prefixes
    4. Fill the LPM with ipv6 MC prefixes
    5. Delete all ipv6 prefixes
    6. Fill the LPM with ipv6 MC prefixes
    7. Delete all ipv6 prefixes

*/
    GT_STATUS   rc=GT_OK;

    /* Supported only for Aldrin with 2K lines in each block */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E | UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_XCAT3_E|UTF_BOBCAT3_E | UTF_FALCON_E|UTF_CAELUM_E|UTF_AC3X_E|UTF_ALDRIN2_E);

    /* skip long test */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /*  set the max Num of PBR entries memory allocated for RAM based LPM shadow to be 0 --> all LPM is used for IP */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

    /* reset the system and run the test with: num of PBR entries = 0 */
    prvTgfResetAndInitSystem();
    cpssOsPrintf("PBR=0 WITHOUT_SHARING\n");

    /* fill lpm when octet 14, 15 is changed */
    prvTgfIpLpmIpv6McPrefixGroupChange(60000,15,14,128,128,GT_FALSE,GT_FALSE);
    /* delete lpm when octet 14, 15 is changed */
    prvTgfIpLpmIpv6McPrefixGroupChange(60000,15,14,128,128,GT_TRUE, GT_FALSE);

    /* fill lpm when octet 12, 13 is changed */
    prvTgfIpLpmIpv6McPrefixGroupChange(60000,3,4,128,128,GT_FALSE,GT_TRUE);
    /* delete lpm when octet 12, 13 is changed */
    prvTgfIpLpmIpv6McPrefixGroupChange(60000,3,4,128,128,GT_TRUE, GT_TRUE);

    /* cleans CPSS appDemo default configurations -
       LPM DB id 0 with VR id 0
       changes the LPM RAM blocks allocation method to DYNAMIC with sharing
       then restores the CPSS appDemo  default configurations */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E);

    /*  set the max Num of PBR entries memory allocated for RAM based LPM shadow to be 0 --> all LPM is used for IP */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

     /* reset the system and run the test with:
        num of PBR entries = 0 */
    prvTgfResetAndInitSystem();
    cpssOsPrintf("PBR=0 WITH_SHARING\n");

    /* fill lpm when octet 14, 15 is changed */
    prvTgfIpLpmIpv6McPrefixGroupChange(60000,15,14,128,128,GT_FALSE,GT_FALSE);
    /* delete lpm when octet 14, 15 is changed */
    prvTgfIpLpmIpv6McPrefixGroupChange(60000,15,14,128,128,GT_TRUE, GT_FALSE);

    /* fill lpm when octet 12, 13 is changed */
    prvTgfIpLpmIpv6McPrefixGroupChange(60000,3,4,128,128,GT_FALSE,GT_TRUE);
    /* delete lpm when octet 12, 13 is changed */
    prvTgfIpLpmIpv6McPrefixGroupChange(60000,3,4,128,128,GT_TRUE, GT_TRUE);

    /* cleans CPSS appDemo default configurations -
    LPM DB id 0 with VR id 0
    changes the LPM RAM blocks allocation method to DYNAMIC with sharing
    then restores the CPSS appDemo  default configurations */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E);

     /* return default configuration of the system */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", _8K);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

    /* change blocks allocation mapping and reset system */
    prvTgfResetAndInitSystem();
}

/* AUTODOC: Test IPv6 Unicast routing when adding and deleting prefixes in a mixed maner
*/
UTF_TEST_CASE_MAC(prvTgfIpv6UcRoutingAddDelMix)
{
    GT_STATUS   rc=GT_OK;

    /* Supported only for Aldrin with 2K lines in each block */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E | UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_XCAT3_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|UTF_ALDRIN2_E);
    /* skip long test */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /*  set the max Num of PBR entries memory allocated for RAM based LPM shadow to be 0 --> all LPM is used for IP */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");
        if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
                rc = appDemoDbEntryAdd("sharedTableMode", 1);
                 UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(sharedTableMode)");
        }

    /* reset the system and run the test with: num of PBR entries = 0 */
    prvTgfResetAndInitSystem();
    cpssOsPrintf("PBR=0 WITHOUT_SHARING\n");

    prvTgfIpLpmIpv6UcPrefix128AddDelMix(9000,8,14,1000,GT_FALSE);


    /* cleans CPSS appDemo default configurations -
       LPM DB id 0 with VR id 0
       changes the LPM RAM blocks allocation method to DYNAMIC with sharing
       then restores the CPSS appDemo  default configurations */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E);

    /*  set the max Num of PBR entries memory allocated for RAM based LPM shadow to be 0 --> all LPM is used for IP */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

     /* reset the system and run the test with:
        num of PBR entries = 0 */
    prvTgfResetAndInitSystem();
    cpssOsPrintf("PBR=0 WITH_SHARING\n");

    prvTgfIpLpmIpv6UcPrefix128AddDelMix(9000,9,14,1000,GT_FALSE);

    /* cleans CPSS appDemo default configurations -
    LPM DB id 0 with VR id 0
    changes the LPM RAM blocks allocation method to DYNAMIC with sharing
    then restores the CPSS appDemo  default configurations */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E);

     /* return default configuration of the system */
        if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
                rc = appDemoDbEntryAdd("sharedTableMode", 2);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(sharedTableMode)");
        rc = appDemoDbEntryAdd("maxNumOfPbrEntries", _1K);
    }
    else
    {
        rc = appDemoDbEntryAdd("maxNumOfPbrEntries", _8K);
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");


    /* change blocks allocation mapping and reset system */
    prvTgfResetAndInitSystem();
}

/* AUTODOC: Test IPv4 Multicast routing when Src is changed.
   adding and deleting prefixes untill the LPM is full.
   Fill the LPM with ipv4 MC prefixes
   Delete all ipv4 prefixes
   Fill the LPM with ipv4 MC prefixes
   Delete all ipv4 prefixes
   Fill the LPM with ipv4 MC prefixes
   Delete all ipv4 prefixes
*/
UTF_TEST_CASE_MAC(prvTgfIpv4McRoutingAddDelAndFillLpmSrcChange)
{
/*
    1. Add 8 pre defined prefixes
    2. Fill the LPM with ipv4 MC prefixes
    3. Delete all ipv4 prefixes
    4. Fill the LPM with ipv4 MC prefixes
    5. Delete all ipv4 prefixes
    6. Fill the LPM with ipv4 MC prefixes
    7. Delete all ipv4 prefixes

*/
    GT_STATUS   rc=GT_OK;

    /* Supported only for Aldrin with 2K lines in each block */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E | UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_XCAT3_E|UTF_BOBCAT3_E | UTF_FALCON_E|UTF_CAELUM_E|UTF_AC3X_E|UTF_ALDRIN2_E);
    /* skip long test */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /*  set the max Num of PBR entries memory allocated for RAM based LPM shadow to be 0 --> all LPM is used for IP */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

    /* reset the system and run the test with: num of PBR entries = 0 */
    prvTgfResetAndInitSystem();
    cpssOsPrintf("PBR=0 WITHOUT_SHARING\n");

    /* fill lpm when octet 3, 2 is changed */
    prvTgfIpLpmIpv4McPrefixSrcChange(60000,3,2,32,32,GT_FALSE,GT_FALSE);
    /* delete lpm when octet 3, 2 is changed */
    prvTgfIpLpmIpv4McPrefixSrcChange(60000,3,2,32,32,GT_TRUE, GT_FALSE);

    /* fill lpm when octet 0, 1 is changed */
    prvTgfIpLpmIpv4McPrefixSrcChange(60000,0,1,32,32,GT_FALSE,GT_TRUE);
    /* delete lpm when octet 0, 1 is changed */
    prvTgfIpLpmIpv4McPrefixSrcChange(60000,0,1,32,32,GT_TRUE, GT_TRUE);

    /* cleans CPSS appDemo default configurations -
       LPM DB id 0 with VR id 0
       changes the LPM RAM blocks allocation method to DYNAMIC with sharing
       then restores the CPSS appDemo  default configurations */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E);

    /*  set the max Num of PBR entries memory allocated for RAM based LPM shadow to be 0 --> all LPM is used for IP */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

     /* reset the system and run the test with:
        num of PBR entries = 0 */
    prvTgfResetAndInitSystem();
    cpssOsPrintf("PBR=0 WITH_SHARING\n");

    /* fill lpm when octet 3, 2 is changed */
    prvTgfIpLpmIpv4McPrefixSrcChange(60000,3,2,32,32,GT_FALSE,GT_FALSE);
    /* delete lpm when octet 3, 2 is changed */
    prvTgfIpLpmIpv4McPrefixSrcChange(60000,3,2,32,32,GT_TRUE, GT_FALSE);

    /* fill lpm when octet 0, 1 is changed */
    prvTgfIpLpmIpv4McPrefixSrcChange(60000,0,1,32,32,GT_FALSE,GT_TRUE);
    /* delete lpm when octet 0, 1 is changed */
    prvTgfIpLpmIpv4McPrefixSrcChange(60000,0,1,32,32,GT_TRUE, GT_TRUE);

    /* cleans CPSS appDemo default configurations -
    LPM DB id 0 with VR id 0
    changes the LPM RAM blocks allocation method to DYNAMIC with sharing
    then restores the CPSS appDemo  default configurations */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E);

     /* return default configuration of the system */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", _8K);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

    /* change blocks allocation mapping and reset system */
    prvTgfResetAndInitSystem();
}

/* AUTODOC: Test IPv4 Multicast routing when Group is changed.
   adding and deleting prefixes untill the LPM is full.
   Fill the LPM with ipv4 MC prefixes
   Delete all ipv4 prefixes
   Fill the LPM with ipv4 MC prefixes
   Delete all ipv4 prefixes
   Fill the LPM with ipv4 MC prefixes
   Delete all ipv4 prefixes
*/
UTF_TEST_CASE_MAC(prvTgfIpv4McRoutingAddDelAndFillLpmGroupChange)
{
/*
    1. Add 8 pre defined prefixes
    2. Fill the LPM with ipv4 MC prefixes
    3. Delete all ipv4 prefixes
    4. Fill the LPM with ipv4 MC prefixes
    5. Delete all ipv4 prefixes
    6. Fill the LPM with ipv4 MC prefixes
    7. Delete all ipv4 prefixes

*/
    GT_STATUS   rc=GT_OK;

    /* Supported only for Aldrin with 2K lines in each block */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E | UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_XCAT3_E|UTF_BOBCAT3_E | UTF_FALCON_E|UTF_CAELUM_E|UTF_AC3X_E|UTF_ALDRIN2_E);
    /* skip long test */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /*  set the max Num of PBR entries memory allocated for RAM based LPM shadow to be 0 --> all LPM is used for IP */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

    /* reset the system and run the test with: num of PBR entries = 0 */
    prvTgfResetAndInitSystem();
    cpssOsPrintf("PBR=0 WITHOUT_SHARING\n");

    /* fill lpm when octet 3, 2 is changed */
    prvTgfIpLpmIpv4McPrefixGroupChange(60000,3,2,32,32,GT_FALSE,GT_FALSE);
    /* delete lpm when octet 3, 2 is changed */
    prvTgfIpLpmIpv4McPrefixGroupChange(60000,3,2,32,32,GT_TRUE, GT_FALSE);

    /* fill lpm when octet 0, 1 is changed */
    prvTgfIpLpmIpv4McPrefixGroupChange(60000,0,1,32,32,GT_FALSE,GT_TRUE);
    /* delete lpm when octet 0, 1 is changed */
    prvTgfIpLpmIpv4McPrefixGroupChange(60000,0,1,32,32,GT_TRUE, GT_TRUE);

    /* cleans CPSS appDemo default configurations -
       LPM DB id 0 with VR id 0
       changes the LPM RAM blocks allocation method to DYNAMIC with sharing
       then restores the CPSS appDemo  default configurations */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E);

    /*  set the max Num of PBR entries memory allocated for RAM based LPM shadow to be 0 --> all LPM is used for IP */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

     /* reset the system and run the test with:
        num of PBR entries = 0 */
    prvTgfResetAndInitSystem();
    cpssOsPrintf("PBR=0 WITH_SHARING\n");

    /* fill lpm when octet 3, 2 is changed */
    prvTgfIpLpmIpv4McPrefixGroupChange(60000,3,2,32,32,GT_FALSE,GT_FALSE);
    /* delete lpm when octet 3, 2 is changed */
    prvTgfIpLpmIpv4McPrefixGroupChange(60000,3,2,32,32,GT_TRUE, GT_FALSE);

    /* fill lpm when octet 0, 1 is changed */
    prvTgfIpLpmIpv4McPrefixGroupChange(60000,0,1,32,32,GT_FALSE,GT_TRUE);
    /* delete lpm when octet 0, 1 is changed */
    prvTgfIpLpmIpv4McPrefixGroupChange(60000,0,1,32,32,GT_TRUE, GT_TRUE);

    /* cleans CPSS appDemo default configurations -
    LPM DB id 0 with VR id 0
    changes the LPM RAM blocks allocation method to DYNAMIC with sharing
    then restores the CPSS appDemo  default configurations */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E);

     /* return default configuration of the system */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", _8K);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

    /* change blocks allocation mapping and reset system */
    prvTgfResetAndInitSystem();
}


/* AUTODOC: Test IPv4,IPv6 Multicast routing when Source is changed.
   adding and deleting prefixes untill the LPM is full.
   Fill the LPM with ipv4 MC prefixes
   Delete all ipv4 prefixes
   Fill the LPM with ipv6 MC prefixes
   Delete all ipv6 prefixes
   Fill the LPM with ipv4 MC prefixes
   Delete all ipv4 prefixes
*/
UTF_TEST_CASE_MAC(prvTgfIpv4Ipv6McRoutingAddDelAndFillLpmSrcChange)
{
/*
    1. Add 8 pre defined prefixes
    2. Fill the LPM with ipv4 MC prefixes
    3. Delete all ipv4 prefixes
    4. Fill the LPM with ipv6 MC prefixes
    5. Delete all ipv6 prefixes
    6. Fill the LPM with ipv4 MC prefixes
    7. Delete all ipv4 prefixes

*/
    GT_STATUS   rc=GT_OK;

    /* Supported only for Aldrin with 2K lines in each block */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E | UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_XCAT3_E|UTF_BOBCAT3_E | UTF_FALCON_E|UTF_CAELUM_E|UTF_AC3X_E|UTF_ALDRIN2_E);
    /* skip long test */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /*  set the max Num of PBR entries memory allocated for RAM based LPM shadow to be 0 --> all LPM is used for IP */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

    /* reset the system and run the test with: num of PBR entries = 0 */
    prvTgfResetAndInitSystem();
    cpssOsPrintf("PBR=0 WITHOUT_SHARING\n");

    /* fill lpm when octet 3, 2 is changed */
    prvTgfIpLpmIpv4McPrefixSrcChange(60000,3,2,32,32,GT_FALSE,GT_FALSE);
    /* delete lpm when octet 3, 2 is changed */
    prvTgfIpLpmIpv4McPrefixSrcChange(60000,3,2,32,32,GT_TRUE, GT_FALSE);

    /* fill lpm when octet 0, 1 is changed */
    prvTgfIpLpmIpv4McPrefixSrcChange(60000,0,1,32,32,GT_FALSE,GT_TRUE);
    /* delete lpm when octet 0, 1 is changed */
    prvTgfIpLpmIpv4McPrefixSrcChange(60000,0,1,32,32,GT_TRUE, GT_TRUE);

    /* fill lpm when octet 14, 15 is changed */
    prvTgfIpLpmIpv6McPrefixSrcChange(60000,15,14,128,128,GT_FALSE,GT_FALSE);
    /* delete lpm when octet 14, 15 is changed */
    prvTgfIpLpmIpv6McPrefixSrcChange(60000,15,14,128,128,GT_TRUE, GT_FALSE);

    /* fill lpm when octet 12, 13 is changed */
    prvTgfIpLpmIpv6McPrefixSrcChange(60000,3,4,128,128,GT_FALSE,GT_TRUE);
    /* delete lpm when octet 12, 13 is changed */
    prvTgfIpLpmIpv6McPrefixSrcChange(60000,3,4,128,128,GT_TRUE, GT_TRUE);

    /* cleans CPSS appDemo default configurations -
    LPM DB id 0 with VR id 0
    changes the LPM RAM blocks allocation method to DYNAMIC with sharing
    then restores the CPSS appDemo  default configurations */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E);

     /* return default configuration of the system */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", _8K);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

    /* change blocks allocation mapping and reset system */
    prvTgfResetAndInitSystem();
}

/* AUTODOC: Test IPv4,IPv6 Multicast routing when Group is changed.
   adding and deleting prefixes untill the LPM is full.
   Fill the LPM with ipv4 MC prefixes
   Delete all ipv4 prefixes
   Fill the LPM with ipv6 MC prefixes
   Delete all ipv6 prefixes
   Fill the LPM with ipv4 MC prefixes
   Delete all ipv4 prefixes
*/
UTF_TEST_CASE_MAC(prvTgfIpv4Ipv6McRoutingAddDelAndFillLpmGroupChange)
{
/*
    1. Add 8 pre defined prefixes
    2. Fill the LPM with ipv4 MC prefixes
    3. Delete all ipv4 prefixes
    4. Fill the LPM with ipv6 MC prefixes
    5. Delete all ipv6 prefixes
    6. Fill the LPM with ipv4 MC prefixes
    7. Delete all ipv4 prefixes

*/
    GT_STATUS   rc=GT_OK;

    /* Supported only for Aldrin with 2K lines in each block */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E | UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_XCAT3_E|UTF_BOBCAT3_E | UTF_FALCON_E|UTF_CAELUM_E|UTF_AC3X_E|UTF_ALDRIN2_E);
    /* skip long test */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /*  set the max Num of PBR entries memory allocated for RAM based LPM shadow to be 0 --> all LPM is used for IP */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

    /* reset the system and run the test with: num of PBR entries = 0 */
    prvTgfResetAndInitSystem();
    cpssOsPrintf("PBR=0 WITHOUT_SHARING\n");

    /* fill lpm when octet 3, 2 is changed */
    prvTgfIpLpmIpv4McPrefixGroupChange(60000,3,2,32,32,GT_FALSE,GT_FALSE);
    /* delete lpm when octet 3, 2 is changed */
    prvTgfIpLpmIpv4McPrefixGroupChange(60000,3,2,32,32,GT_TRUE, GT_FALSE);

    /* fill lpm when octet 0, 1 is changed */
    prvTgfIpLpmIpv4McPrefixGroupChange(60000,0,1,32,32,GT_FALSE,GT_TRUE);
    /* delete lpm when octet 0, 1 is changed */
    prvTgfIpLpmIpv4McPrefixGroupChange(60000,0,1,32,32,GT_TRUE, GT_TRUE);

    /* fill lpm when octet 14, 15 is changed */
    prvTgfIpLpmIpv6McPrefixGroupChange(60000,15,14,128,128,GT_FALSE,GT_FALSE);
    /* delete lpm when octet 14, 15 is changed */
    prvTgfIpLpmIpv6McPrefixGroupChange(60000,15,14,128,128,GT_TRUE, GT_FALSE);

    /* fill lpm when octet 12, 13 is changed */
    prvTgfIpLpmIpv6McPrefixGroupChange(60000,3,4,128,128,GT_FALSE,GT_TRUE);
    /* delete lpm when octet 12, 13 is changed */
    prvTgfIpLpmIpv6McPrefixGroupChange(60000,3,4,128,128,GT_TRUE, GT_TRUE);

    /* cleans CPSS appDemo default configurations -
    LPM DB id 0 with VR id 0
    changes the LPM RAM blocks allocation method to DYNAMIC with sharing
    then restores the CPSS appDemo  default configurations */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E);

     /* return default configuration of the system */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", _8K);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

    /* change blocks allocation mapping and reset system */
    prvTgfResetAndInitSystem();
}

/* AUTODOC: Test IPv4 unicast routing capacity with shrink and merge between banks .
   adding prefixes untill the LPM is full.
   Add a predefined prefix 3.3.3.3
   Fill the LPM with sequential ipv4 UC prefixes(0.0.0.1...) ~ 0 to 1500 entries (changing octet 2 then octet 3)
   Fill the LPM with sequential ipv4 UC prefixes (4.4.4.4...)~500 entries (changing octet 2 then octet 3)
   Fill the LPM with sequential ipv4 UC prefixes(0.0.0.1...) ~ 1500 to 3300 entries (changing octet 2 then octet 3)
   Fill the LPM with sequential ipv4 UC prefixes (5.5.5.5...)~500 entries (changing octet 2 then octet 3)
   Fill the LPM with sequential ipv4 UC prefixes(0.0.0.1...) ~ 3300 to 28925 entries (changing octet 2 then octet 3)
   Delete all ipv4 prefixes (0.0.0.1...) 0 to 3300 entries
   Add prefixes (0.0.12.13 ...) 0 to 1689(changing octet 0 then octet 1) get memory is full
   change defrag=true and succeed in adding 5542 more prefixes.
*/
UTF_TEST_CASE_MAC(prvTgfIpv4UcRoutingCapacityMergeBanks)
{
/*
   1.Add a predefined prefix 3.3.3.3
   2.Fill the LPM with sequential ipv4 UC prefixes(0.0.0.1...) ~ 0 to 1500 entries (changing octet 2 then octet 3)
   3.Fill the LPM with sequential ipv4 UC prefixes (4.4.4.4...)~500 entries (changing octet 2 then octet 3)
   4.Fill the LPM with sequential ipv4 UC prefixes(0.0.0.1...) ~ 1500 to 3300 entries (changing octet 2 then octet 3)
   5.Fill the LPM with sequential ipv4 UC prefixes (5.5.5.5...)~500 entries (changing octet 2 then octet 3)
   6.Fill the LPM with sequential ipv4 UC prefixes(0.0.0.1...) ~ 3300 to 28925 entries (changing octet 2 then octet 3)
   7.Delete all ipv4 prefixes (0.0.0.1...) 0 to 3300 entries
   8.Add prefixes (0.0.12.13 ...) 0 to 1689(changing octet 0 then octet 1) get memory is full
   9.change defrag=true and succeed in adding 5542 more prefixes.
*/
    GT_STATUS   rc=GT_OK;

   /* Supported only for Aldrin with 2K lines in each block */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E | UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_XCAT3_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|UTF_ALDRIN2_E|UTF_FALCON_E);

    /* skip long test */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /*  set the max Num of PBR entries memory allocated for RAM based LPM shadow to be 0 --> all LPM is used for IP */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

     /* reset the system and run the test with:
        num of PBR entries = 0 */
    prvTgfResetAndInitSystem();
    cpssOsPrintf("PBR=0 WITHOUT_SHARING \n");

    /* flush memory then add prefixes with defrag = false,
       after no more space then try again with defrag= true  */
    prvTgfIpLpmIpv4UcOrderAndPredefined(500000,GT_TRUE);

    /* cleans CPSS appDemo default configurations -
    LPM DB id 0 with VR id 0
    changes the LPM RAM blocks allocation method to DYNAMIC with sharing
    then restores the CPSS appDemo  default configurations */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E);

     /* return default configuration of the system */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", _8K);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

    /* change blocks allocation mapping and reset system */
    prvTgfResetAndInitSystem();
}

/* AUTODOC: Test IPv4 unicast routing capacity with shrink and merge between banks .
   adding prefixes untill the LPM is full.
   Add a predefined prefix 11.12.13.14 with next hop to port 1 (octet 3 defined in bank3)
   Send traffic and check packet arrive to port 1
   Add a predefined prefix 3.3.3.3
   Fill the LPM with sequential ipv4 UC prefixes(0.0.0.1...) ~ 0 to 1500 entries (changing octet 2 then octet 3)
   Fill the LPM with sequential ipv4 UC prefixes (4.4.4.4...)~500 entries (changing octet 2 then octet 3)
   Fill the LPM with sequential ipv4 UC prefixes(0.0.0.1...) ~ 1500 to 3300 entries (changing octet 2 then octet 3)
   Fill the LPM with sequential ipv4 UC prefixes (5.5.5.5...)~500 entries (changing octet 2 then octet 3)
   Fill the LPM with sequential ipv4 UC prefixes(0.0.0.1...) ~ 3300 to 28925 entries (changing octet 2 then octet 3)
   Delete all ipv4 prefixes (0.0.0.1...) 0 to 3300 entries
   Add prefixes (0.0.12.13 ...) 0 to 1689(changing octet 0 then octet 1) get memory is full
   change defrag=true and succeed in adding 5542 more prefixes.
   octet 3 of prefix 11.12.13.14 was moved to a different bank.
   Send traffic and check packet arrive to port 1
*/
UTF_TEST_CASE_MAC(prvTgfIpv4UcRoutingCapacityMergeBanksWithTraffic)
{
/*
   Add a predefined prefix 1.1.1.3/32 with next hop to port 1 (octet 3 defined in bank3)
   Send traffic and check packet arrive to port 1

   1.Add a predefined prefix 3.3.3.3
   2.Fill the LPM with sequential ipv4 UC prefixes(0.0.0.1...) ~ 0 to 1500 entries (changing octet 2 then octet 3)
   3.Fill the LPM with sequential ipv4 UC prefixes (4.4.4.4...)~500 entries (changing octet 2 then octet 3)
   4.Fill the LPM with sequential ipv4 UC prefixes(0.0.0.1...) ~ 1500 to 3300 entries (changing octet 2 then octet 3)
   5.Fill the LPM with sequential ipv4 UC prefixes (5.5.5.5...)~500 entries (changing octet 2 then octet 3)
   6.Fill the LPM with sequential ipv4 UC prefixes(0.0.0.1...) ~ 3300 to 28925 entries (changing octet 2 then octet 3)
   7.Delete all ipv4 prefixes (0.0.0.1...) 0 to 3300 entries
   8.Add prefixes (0.0.12.13 ...) 0 to 1689(changing octet 0 then octet 1) get memory is full
   9.change defrag=true and succeed in adding 5542 more prefixes.

   octet 3 of prefix 1.1.1.3 was moved to a different bank.
   Send traffic and check packet arrive to port 1
*/
    GT_STATUS   rc=GT_OK;
    GT_U32      prvUtfVrfId = 0;

   /* Supported only for Aldrin with 2K lines in each block */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E | UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_XCAT3_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|UTF_ALDRIN2_E|UTF_FALCON_E);

    /* skip long test */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /*  set the max Num of PBR entries memory allocated for RAM based LPM shadow to be 0 --> all LPM is used for IP */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

     /* reset the system and run the test with:
        num of PBR entries = 0 */
    prvTgfResetAndInitSystem();
    cpssOsPrintf("PBR=0 WITHOUT_SHARING \n");

    /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

    /* Set Route configuration for vrfId[0] */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                  prvTgfPortsArray[3], GT_FALSE,
                                                  GT_FALSE, GT_FALSE);

    /* Generate traffic */
    prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS);

    /* add prefixes with defrag = false,
       after no more space then try again with defrag= true  */
    prvTgfIpLpmIpv4UcOrderAndPredefined(500000,GT_FALSE);

    /* Generate traffic */
    prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS);

    /* Restore configuration */
    prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, 3, GT_FALSE);

    /* cleans CPSS appDemo default configurations -
    LPM DB id 0 with VR id 0
    changes the LPM RAM blocks allocation method to DYNAMIC with sharing
    then restores the CPSS appDemo  default configurations */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E);

     /* return default configuration of the system */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", _8K);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

    /* change blocks allocation mapping and reset system */
    prvTgfResetAndInitSystem();
}

/* AUTODOC: Test IPv4 unicast routing capacity with shrink and merge between banks .
   adding prefixes untill the LPM is full.
   Add a predefined prefix 3.3.3.3
   Fill the LPM with sequential ipv4 UC prefixes(0.0.0.1...) ~ 0 to 1500 entries (changing octet 3 then octet 2)
   Fill the LPM with sequential ipv4 UC prefixes (4.4.4.4...)~500 entries (changing octet 3 then octet 2)
   Fill the LPM with sequential ipv4 UC prefixes(0.0.0.1...) ~ 1500 to 3000 entries (changing octet 3 then octet 2)
   Fill the LPM with sequential ipv4 UC prefixes (5.5.5.5...)~500 entries (changing octet 3 then octet 3)
   Fill the LPM with sequential ipv4 UC prefixes(0.0.0.1...) ~ 3000 to 4500 entries (changing octet 3 then octet 2)
   and so on in a loop
   Delete all ipv4 prefixes (0.0.0.1...) 0 to the max added entries
   Add prefixes (20.20.12.13/16 ...) 0 to 1533(changing octet 1 then octet 0) get memory is full
   change defrag=true and succeed in adding 21929 more prefixes.

*/
UTF_TEST_CASE_MAC(prvTgfIpv4UcRoutingExtendedCapacityMergeBanks)
{
/*
   1.Add a predefined prefix 3.3.3.3
   2.Fill the LPM with sequential ipv4 UC prefixes(0.0.0.1...) ~ 0 to 1500 entries (changing octet 3 then octet 2)
   3.Fill the LPM with sequential ipv4 UC prefixes (4.4.4.4...)~500 entries (changing octet 3 then octet 2)
   4.Fill the LPM with sequential ipv4 UC prefixes(0.0.0.1...) ~ 1500 to 3000 entries (changing octet 3 then octet 2)
   5.Fill the LPM with sequential ipv4 UC prefixes (5.5.5.5...)~500 entries (changing octet 3 then octet 3)
   6.Fill the LPM with sequential ipv4 UC prefixes(0.0.0.1...) ~ 3000 to 4500 entries (changing octet 3 then octet 2)
   7. and so on in a loop
   8.Delete all ipv4 prefixes (0.0.0.1...) 0 to the max added entries
   8.Add prefixes (20.20.12.13/16 ...) 0 to 1533(changing octet 1 then octet 0) get memory is full
   10.change defrag=true and succeed in adding 21929 more prefixes.

*/
    GT_STATUS   rc=GT_OK;

   /* Supported only for Aldrin with 2K lines in each block */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E | UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_XCAT3_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|UTF_ALDRIN2_E|UTF_FALCON_E);

    /* skip long test */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /*  set the max Num of PBR entries memory allocated for RAM based LPM shadow to be 0 --> all LPM is used for IP */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

     /* reset the system and run the test with:
        num of PBR entries = 0 */
    prvTgfResetAndInitSystem();
    cpssOsPrintf("PBR=0 WITHOUT_SHARING \n");

    /* flush memory then add prefixes with defrag = false,
       after no more space then try again with defrag= true  */
    prvTgfIpLpmIpv4UcCheckMergeBanks(29181);

    /* cleans CPSS appDemo default configurations -
    LPM DB id 0 with VR id 0
    changes the LPM RAM blocks allocation method to DYNAMIC with sharing
    then restores the CPSS appDemo  default configurations */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E);

     /* return default configuration of the system */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", _8K);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

    /* change blocks allocation mapping and reset system */
    prvTgfResetAndInitSystem();
}

/* AUTODOC: Test IPv6 unicast routing with prefixLength=64.
   adding prefixes untill the LPM is full.
   Fill the LPM with ipv6 UC prefixes when octet 0 to 6 is changed
   Run Validity
   Delete all ipv6 prefixes
*/
UTF_TEST_CASE_MAC(prvTgfIpv6UcRoutingFillLpmOrderOctets0To6With64PrefixLength)
{
/*
   adding prefixes untill the LPM is full.
   Fill the LPM with ipv6 UC prefixes when octet 0 to 6 is changed
   Run Validity
   Delete all ipv6 prefixes
*/
    GT_STATUS   rc=GT_OK;

   /* Supported only for Aldrin with 2K lines in each block */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E | UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_XCAT3_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|UTF_ALDRIN2_E|UTF_FALCON_E);

    /* skip long test */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /*  set the max Num of PBR entries memory allocated for RAM based LPM shadow to be 0 --> all LPM is used for IP */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

     /* reset the system and run the test with:
        num of PBR entries = 0 */
    prvTgfResetAndInitSystem();
    cpssOsPrintf("PBR=0 WITHOUT_SHARING \n");

    /* flush memory then add 1364 ipv6 UC prefixes with prefixlen=64 defrag=true */
    prvTgfIpLpmIpUcOrderOctet0To6(9128,CPSS_IP_PROTOCOL_IPV6_E,CPSS_UNICAST_E,64,0);

    /* return default configuration of the system */
    rc = appDemoDbEntryAdd("maxNumOfPbrEntries", _8K);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

    /* change blocks allocation mapping and reset system */
    prvTgfResetAndInitSystem();
}
#endif
/*
 * Configuration of tgfIp suit
 */

UTF_SUIT_PBR_BEGIN_TESTS_MAC(tgfIp)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4UcRouting)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4UcRoutingTrapToCpu)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4UcRouting4BytesMode)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4UcRoutingSipLookupDisable)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4UcRouting_DSA_fromRemoteDeviceX_toRemoteDeviceX)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4UcRouting_DsaRouted_backToSrcDev)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv6UcRouting)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv6UcRouting4BytesMode)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv6UcRoutingSipLookupDisable)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv6UcRoutingEgrCascadePort)

    /*start basic routing for trunks*/
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkBasicIpv4UcRoutingTrunkAToTrunkB)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkBasicIpv4UcRoutingPortToTrunkB)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkBasicIpv4UcRoutingTrunkAToPort)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTrunkBasicIpv4UcRoutingTrunkAToTrunkA)
    /*end basic routing for trunks*/

    /*start basic routing for EPorts mapped to VIDX */
    UTF_SUIT_DECLARE_TEST_MAC(tgfVidxBasicIpv4UcRoutingTrunkAToVidxWithTrunkA)
    /*end basic routing for EPorts mapped to VIDX */

    UTF_SUIT_DECLARE_TEST_MAC(tgfIpLpmDBCapacityUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(tgfComplicatedPbrIpResizeDefToSmaller)
    UTF_SUIT_DECLARE_TEST_MAC(tgfComplicatedPbrIpPclResizeDefToSmaller)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpResizeDefSmallerBigger)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpResizeDefSmallerBiggerSmaller)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpResizeDefBiggerSmallerSmaller)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpPclResizeDefSmallerSmallerBigger)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpPclResizeDefBiggerSmallerSmaller)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpMtuExceedExceptionCheck)
    /*********** end of tests that are relevant to PBR ************************/

    /* all test above this line are relevant to PBR and to Router */
    /* NOTE: for every test that you add above this line you MUST increment by 1
       the number in the line of : UTF_SUIT_PBR_END_TESTS_MAC(tgfIp,XXX) */


    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4UcRoutingCheckAgeBit)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4UcRoutingUseEPort)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4UcRoutingUnicastRPF)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4UcRoutingUnicastRPFLooseModeType)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4UcRoutingUnicastSipSa)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4UcEcmpRouting)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4UcEcmpRoutingRandomEnable)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4UcEcmpRoutingIndirectAccess)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4McRouting)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4McRoutingMllSilentDrop)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4McRoutingForDualHwDev)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4McRoutingDifferentPrefixLength)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv6McRouting)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv6McRoutingScopeChecking)
    UTF_SUIT_DECLARE_TEST_MAC(tgfDefaultIpv4McRouting)
    UTF_SUIT_DECLARE_TEST_MAC(tgfDefaultIpv4McRoutingForDualHwDev)
    UTF_SUIT_DECLARE_TEST_MAC(tgfDefaultIpv6McRouting)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4UcLowLevelRouting)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4Uc2VirtualRouter)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv42SharedVirtualRouter)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv4Uc2MultipleVr)
    UTF_SUIT_DECLARE_TEST_MAC(tgfComplicatedIpv4McRouting)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIpv4NonExactMatchUcRouting)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIpv6NonExactMatchUcRouting)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIpv4NonExactMatchMcRouting)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIpv6NonExactMatchMcRouting)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTablesFillingLttCheckIpv4)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTablesFillingLttCheckIpv6)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTablesFillingLpmCheckIpv4)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTablesFillingLpmCheckIpv6)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTablesFillingLpmCheckIpv4v6)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv4UcRoutingAddMany)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv4UcRoutingAddManyIpLpmMng)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv4UcRoutingAddRealPrefixes)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv4UcRoutingAddDeleteAddRealPrefixes)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv4UcRoutingHiddenPrefixes)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv6UcRoutingAddMany)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv6UcRoutingAddManyIpLpmMng)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv4Ipv6UcRoutingAddManyIpLpmMng)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv4McRoutingAddMany)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv4McRoutingAddRealPrefixes)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv6McRoutingAddMany)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv4UcRoutingAddManyActivityState)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv4McRoutingAddManyActivityState)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv6UcRoutingAddManyActivityState)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv6McRoutingAddManyActivityState)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4RoutingBulk)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv6RoutingBulk)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpLpmIpUcPrefixBulkAdd)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4UcMacSaModesRouting)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv4UcRoutingDefaultOverride)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4McRoutingDifferentMcEntries)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv4McRoutingDefaultOverride)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv6McRoutingDefaultOverride)

    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4McRoutingSourceId)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv4UcRollBack)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv6UcRollBack)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv4McRollBack)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv6McRollBack)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicIpv4UcEcmpRoutingHashCheck)

    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv4UcNatTcpRouting)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv4UcNatUdpWithOptions)

    UTF_SUIT_DECLARE_TEST_MAC(tgfDynamicPartitionIpLpmPushUpUcRouting)
    UTF_SUIT_DECLARE_TEST_MAC(tgfDynamicPartitionIpLpmPushDownUcRouting)
    UTF_SUIT_DECLARE_TEST_MAC(tgfDynamicPartitionIpLpmDefragUcRouting)
    UTF_SUIT_DECLARE_TEST_MAC(tgfDynamicPartitionIpLpmDefragUcRoutingSpecificCostumerTestScenario)
    UTF_SUIT_DECLARE_TEST_MAC(tgfDynamicPartitionIpLpmDefragUcRoutingRandomCostumerTestScenario)
    UTF_SUIT_DECLARE_TEST_MAC(tgfDynamicPartitionIpLpmDefragUcRoutingRandomTestScenario)
    UTF_SUIT_DECLARE_TEST_MAC(tgfDynamicPartitionIpLpmDefragIpv4UcIpv6McRoutingRandomCostumerTestScenario)
    UTF_SUIT_DECLARE_TEST_MAC(tgfDynamicPartitionIpLpmDefragIpv4McRouting)
    UTF_SUIT_DECLARE_TEST_MAC(tgfDynamicPartitionIpLpmDefragIpv4McRoutingMovingDefault)
    UTF_SUIT_DECLARE_TEST_MAC(tgfDynamicPartitionIpLpmDefragIpv6McRouting)
    UTF_SUIT_DECLARE_TEST_MAC(tgfDynamicPartitionIpLpmDefragMcRoutingRandomCostumerTestScenario)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv4UcRoutingLpmHalfModeCheck)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv6UcRoutingLpmHalfModeCheck)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv6UcSaMismatchCheck)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv6McRpfFail)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv4UcRoutingPriority)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpv4UcPbrRoutingPriority)

    /* Capacity tests */
    #if RUN_LONG_CAPACITY_TESTS
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIpv4Ipv6UcRoutingAddDelAndFillLpm)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIpv6UcRoutingFillLpmOrderAndDisorder)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIpv4UcRoutingFillLpmOrderAndDisorder)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIpv4Ip6UcRoutingFillLpmDisorder)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIpv6McRoutingAddDelAndFillLpmSrcChange)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIpv6McRoutingAddDelAndFillLpmGroupChange)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIpv4McRoutingAddDelAndFillLpmSrcChange)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIpv4McRoutingAddDelAndFillLpmGroupChange)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIpv4Ipv6McRoutingAddDelAndFillLpmSrcChange)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIpv4Ipv6McRoutingAddDelAndFillLpmGroupChange)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIpv6UcRoutingAddDelMix)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIpv4UcRoutingCapacityMergeBanks)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIpv4UcRoutingCapacityMergeBanksWithTraffic)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIpv4UcRoutingExtendedCapacityMergeBanks)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIpv6UcRoutingFillLpmOrderOctets0To6With64PrefixLength)
    #endif


UTF_SUIT_PBR_END_TESTS_MAC(tgfIp, 0)


